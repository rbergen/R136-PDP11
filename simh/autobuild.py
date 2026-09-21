#!/usr/bin/env python3
"""Builds R136 on the emulated PDP-11 with nobody at the console.

Starts the machine through run.sh, so the tape is current and the guard against
a second simulator applies, then logs in as "user", unpacks the tape, runs
build.csh, checks what came out of it, shuts the machine down properly and
exits 0 if all of that went well. Everything the console shows is streamed to
stdout, and to a log file as well if you ask for one.

This is what CI runs, and it's just as usable by hand:

    ./autobuild.py

Commands sent to the machine are tcsh, since that is what "user" gets.
"""

import argparse
import os
import pty
import re
import select
import signal
import sys
import time

SIMH_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIRECTORY = os.path.dirname(SIMH_DIRECTORY)

# Appears in the machine's output, never in the echo of what we typed: the
# shell only joins the two halves when it runs the echo.
MARKER = 'R136DONE'
MARKER_COMMAND = 'echo R136""DONE=$status'

# What a failed build looks like on the console. pcc reports as "file.c:12:",
# gendata's own complaints start with "!", and build.csh reports the rest.
DIAGNOSTIC = re.compile(r'\.[ch]:\s*\d+|^!|undefined|not found|Aborting|cannot|Error')

CONTROL_SEQUENCE = re.compile(rb'\x1b\[[0-9;?]*[a-zA-Z]|\x1b[()][A-Z0-9]|\x1b[=>]')


class Console:
    """The simulator's console, on a pseudo-terminal."""

    def __init__(self, command, cwd, log):
        self.log = log
        self.buffer = b''
        self.consumed = b''
        self.line = b''
        self.pid, self.fd = pty.fork()
        if self.pid == 0:
            os.chdir(cwd)
            os.execvp(command[0], command)
            os._exit(1)

    def read(self, seconds):
        """Collects output for the given number of seconds, echoing it."""
        end = time.time() + seconds
        while time.time() < end:
            ready, _, _ = select.select([self.fd], [], [], 0.2)
            if not ready:
                continue
            try:
                data = os.read(self.fd, 65536)
            except OSError:
                break
            if not data:
                break
            self.buffer += data
            if self.log:
                self.log.write(data)
                self.log.flush()
            self.echo(data)

    def echo(self, data):
        """Prints console output as plain lines."""
        self.line += data
        *lines, self.line = self.line.split(b'\n')
        for line in lines:
            line = CONTROL_SEQUENCE.sub(b'', line).replace(b'\r', b'')
            print('    ' + line.decode('ascii', 'replace').rstrip(), flush=True)

    def expect(self, pattern, seconds):
        """Waits until the output matches, and returns the match or None. What
        came before the match is kept in self.consumed."""
        expression = re.compile(pattern.encode() if isinstance(pattern, str) else pattern)
        end = time.time() + seconds
        while True:
            match = expression.search(self.buffer)
            if match:
                self.consumed = self.buffer[:match.start()]
                self.buffer = self.buffer[match.end():]
                return match
            if time.time() >= end:
                return None
            self.read(0.5)

    def send(self, text, delay=0.01):
        """Types text, slowly enough for the machine to keep up."""
        for byte in text.encode():
            os.write(self.fd, bytes([byte]))
            time.sleep(delay)

    def run(self, command, seconds):
        """Runs a shell command and returns (exit status, output)."""
        start = len(self.buffer)
        self.send(command + '; ' + MARKER_COMMAND + '\r')
        match = self.expect(MARKER + r'=(\d+)', seconds)
        if match is None:
            return None, ''
        return int(match.group(1)), self.consumed[start:].decode('ascii', 'replace')

    def finished(self, seconds):
        """Waits for the simulator to exit, and returns whether it did."""
        end = time.time() + seconds
        while time.time() < end:
            pid, _ = os.waitpid(self.pid, os.WNOHANG)
            if pid:
                return True
            self.read(0.5)
        return False

    def kill(self):
        try:
            os.kill(self.pid, signal.SIGKILL)
            os.waitpid(self.pid, 0)
        except OSError:
            pass


def step(title):
    print('\n*** ' + title, flush=True)


def fail(message):
    print('\n!!! ' + message, file=sys.stderr, flush=True)


def text_file_count(language):
    """How many text files a language has, and so how many data files to expect."""
    directory = os.path.join(PROJECT_DIRECTORY, 'texts', language)
    return len([name for name in os.listdir(directory) if name.endswith('.txt')])


def build(console, arguments):
    """Runs the build and the checks, and returns a list of what went wrong."""
    problems = []

    step('Waiting for the machine to boot')
    if console.expect(r'login:', arguments.boot_timeout) is None:
        return ['The machine did not get to a login prompt in %d seconds' % arguments.boot_timeout]

    step('Logging in as user')
    time.sleep(1)
    console.send('user\r')
    if console.expect(r'user\[\d+\]', 120) is None:
        return ['No shell prompt after logging in']

    step('Unpacking the tape')
    status, output = console.run('cd ; rm -rf r136 ; tar xf /dev/rmt0', arguments.tape_timeout)
    if status != 0:
        return ['Unpacking the tape failed (status %s)' % status]

    step('Building')
    started = time.time()
    status, output = console.run('cd ~/r136 ; ./build.csh', arguments.build_timeout)
    if status is None:
        return ['The build did not finish in %d seconds' % arguments.build_timeout]
    print('    (%d seconds)' % (time.time() - started), flush=True)

    if status != 0:
        problems.append('build.csh exited with status %d' % status)

    diagnostics = [line.strip() for line in output.splitlines() if DIAGNOSTIC.search(line)]
    if diagnostics:
        problems.append('The build reported problems:\n' + '\n'.join('    ' + d for d in diagnostics))

    step('Checking what the build produced')
    # ls exits 0 here whether or not the files exist; test is the one that knows.
    console.run('cd ~/r136 ; ls -l r136 tools/gendata r136.tar', 120)
    for name, kind in (('r136', 'x'), ('tools/gendata', 'x'), ('r136.tar', 'f')):
        status, output = console.run('test -%s ~/r136/%s' % (kind, name), 120)
        if status != 0:
            problems.append('The build did not produce %s' % name)

    for language in ('en', 'nl'):
        expected = text_file_count(language)
        status, output = console.run('ls ~/r136/data/%s | wc -l' % language, 120)
        found = output.split()[-1] if output.split() else '?'
        if status != 0 or found != str(expected):
            problems.append('data/%s holds %s files, expected %d' % (language, found, expected))

    step('Checking that the game runs')
    status, output = console.run('cd ~/r136 ; ./r136 -h', 120)
    if status != 0 or '* en' not in output or '* nl' not in output:
        problems.append('r136 -h did not list both languages')

    return problems


def shut_down(console):
    """Halts 2.11BSD the proper way and quits the simulator."""
    step('Shutting down')
    console.send('exit\r')
    if console.expect(r'login:', 120) is None:
        return False
    time.sleep(2)
    console.send('root\r')
    if console.expect(r'# ', 120) is None:
        return False
    for command in ('sync', 'sync', 'halt'):
        console.send(command + '\r')
        time.sleep(2)
    if console.expect(r'halted by root', 120) is None:
        return False
    # 2.11BSD halts the CPU and the simulator drops to its own prompt.
    if console.expect(r'sim>', 60) is None:
        return False
    console.send('quit\r')
    return console.finished(30)


def main():
    parser = argparse.ArgumentParser(description='Builds R136 on the emulated PDP-11, unattended.')
    parser.add_argument('--log', metavar='FILE', help='also write the raw console output here')
    parser.add_argument('--boot-timeout', type=int, default=300, metavar='SECONDS',
                        help='how long to wait for the login prompt (default: 300)')
    parser.add_argument('--tape-timeout', type=int, default=300, metavar='SECONDS',
                        help='how long to wait for the tape to unpack (default: 300)')
    parser.add_argument('--build-timeout', type=int, default=900, metavar='SECONDS',
                        help='how long to wait for build.csh (default: 900)')
    arguments = parser.parse_args()

    log = open(arguments.log, 'wb') if arguments.log else None
    console = Console(['./run.sh'], SIMH_DIRECTORY, log)

    try:
        problems = build(console, arguments)
        if not shut_down(console):
            fail('The machine did not shut down cleanly; killing the simulator')
            console.kill()
            problems.append('The machine did not shut down cleanly')
    except KeyboardInterrupt:
        console.kill()
        raise
    finally:
        if log:
            log.close()

    if problems:
        for problem in problems:
            fail(problem)
        return 1

    print('\n*** R136 built on 2.11BSD', flush=True)
    return 0


if __name__ == '__main__':
    sys.exit(main())
