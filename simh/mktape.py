#!/usr/bin/env python3

"""Builds a SIMH tape image containing R136, so that it can be copied onto a
2.11BSD system running under the SIMH pdp11 simulator.

The tape holds a single tar archive in old (v7/ustar) format. Attach the result
in your SIMH configuration:

    set tq ena
    att tq0 r136.tap

and then, on the 2.11BSD side:

    mkdir -p /usr/src/r136 && cd /usr/src/r136
    dd if=/dev/rmt0 of=r136.tar bs=512
    sum r136.tar                 # must match the checksum printed below
    tar xf r136.tar && sync

Note the record size: SIMH and the 2.11BSD tape driver will happily accept
10240 byte records (tar's default blocking factor of 20), but the files then
arrive with the right sizes and corrupted contents. 512 byte records are
reliable, which is why they are the default here.

run.sh calls this with --if-needed, which rebuilds the tape only when one of
the files that belongs on it has changed. The lists below are the only place
that decides what does belong on it.
"""

import argparse
import io
import os
import struct
import sys
import tarfile

# Never worth shipping to the PDP-11: version control, editor state, the
# generated data directory (build.csh regenerates it), earlier tape images, and
# this directory, which is only of use on the machine running the simulator.
SKIP_NAMES = ('.git', '.vscode', 'data', '__pycache__', 'simh')
SKIP_SUFFIXES = ('.tap', '.tar', '.o', '.dsk', '.xz', '.gz')
# Binaries built on the host, since the PDP-11 builds its own, and the top level
# README, which is about getting to the PDP-11 rather than anything done on it.
SKIP_PATHS = ('r136', 'tools/gendata', 'README.md')


def should_skip(relative_path, is_dir):
    parts = relative_path.split('/')

    if any(part in SKIP_NAMES for part in parts):
        return True
    if not is_dir and relative_path.endswith(SKIP_SUFFIXES):
        return True
    if not is_dir and relative_path in SKIP_PATHS:
        return True

    return False


def collect(source_dir):
    """Returns the files to archive, as (absolute path, path relative to source)."""
    collected = []

    for directory, subdirs, files in os.walk(source_dir):
        relative_dir = os.path.relpath(directory, source_dir)
        if relative_dir == '.':
            relative_dir = ''

        subdirs[:] = sorted(d for d in subdirs
                            if not should_skip(os.path.join(relative_dir, d).lstrip('/'), True))

        for name in sorted(files):
            relative_path = os.path.join(relative_dir, name).lstrip('/')
            full_path = os.path.join(directory, name)

            if should_skip(relative_path, False) or not os.path.isfile(full_path):
                continue

            collected.append((full_path, relative_path))

    return collected


def newest(files, than):
    """Returns the relative path of the most recently changed file that is newer
    than the given timestamp, or None if none of them are."""
    newer = [(os.path.getmtime(full_path), relative_path)
             for full_path, relative_path in files
             if os.path.getmtime(full_path) > than]

    return max(newer)[1] if newer else None


def build_tar(files, member_dir):
    """Builds a tar archive in memory, with every path under member_dir."""
    buffer = io.BytesIO()

    # ustar is what 2.11BSD's tar understands; it ignores the newer header
    # fields, and every path in R136 is far shorter than the 100
    # character limit that old tar imposes.
    with tarfile.open(fileobj=buffer, mode='w', format=tarfile.USTAR_FORMAT) as archive:
        for full_path, relative_path in files:
            info = archive.gettarinfo(full_path, arcname='%s/%s' % (member_dir, relative_path))
            # 2.11BSD has no idea who we are, so don't confuse it.
            info.uid = info.gid = 0
            info.uname = info.gname = 'root'

            with open(full_path, 'rb') as handle:
                archive.addfile(info, handle)

    return buffer.getvalue()


def wrap_in_tape(data, record_size):
    """Wraps the data in the SIMH .tap container format: every record is
    preceded and followed by its length as a 32 bit little endian value. Two
    tape marks (a zero length) mean end of tape, and 0xFFFFFFFF ends the
    medium."""
    tape = bytearray()
    header = struct.pack('<I', record_size)

    for offset in range(0, len(data), record_size):
        record = data[offset:offset + record_size]
        tape += header + record.ljust(record_size, b'\0') + header

    tape += struct.pack('<I', 0) * 2
    tape += struct.pack('<I', 0xFFFFFFFF)

    return bytes(tape)


def bsd_sum(data):
    """The checksum that 2.11BSD's sum(1) prints: a 16 bit value, rotated right
    before every byte is added. Returned with the block count sum(1) reports."""
    checksum = 0

    for byte in data:
        checksum = (checksum >> 1) | ((checksum & 1) << 15)
        checksum = (checksum + byte) & 0xFFFF

    return checksum, (len(data) + 1023) // 1024


def main():
    project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    parser = argparse.ArgumentParser(
        description='Build a SIMH tape image holding a tar of R136.',
        epilog='On 2.11BSD: dd if=/dev/rmt0 of=r136.tar bs=512 && tar xf r136.tar && sync')
    parser.add_argument('-o', '--output', default='r136.tap',
                        help='tape image to write (default: r136.tap)')
    parser.add_argument('-s', '--source', default=project_dir,
                        help='directory to archive (default: where R136 lives)')
    parser.add_argument('-d', '--dir-name', default='r136',
                        help='directory name inside the archive (default: r136)')
    parser.add_argument('-r', '--record-size', type=int, default=512,
                        help='tape record size in bytes (default: 512, and see the '
                             'note at the top of this script before raising it)')
    parser.add_argument('-n', '--if-needed', action='store_true',
                        help='do nothing unless a file that belongs on the tape is '
                             'newer than the tape image, and say which one it was')
    arguments = parser.parse_args()

    if not os.path.isdir(arguments.source):
        sys.exit('%s is not a directory' % arguments.source)

    files = collect(arguments.source)

    if arguments.if_needed:
        if not os.path.exists(arguments.output):
            reason = "it doesn't exist yet"
        else:
            reason = newest(files, os.path.getmtime(arguments.output))
            if reason is None:
                print('%s is up to date.' % arguments.output)
                return
            reason = '%s is newer' % reason

        print('Building %s, because %s' % (arguments.output, reason))

    data = build_tar(files, arguments.dir_name)
    tape = wrap_in_tape(data, arguments.record_size)

    with open(arguments.output, 'wb') as handle:
        handle.write(tape)

    checksum, blocks = bsd_sum(data)

    print('Wrote %s: %d bytes, %d records of %d bytes'
          % (arguments.output, len(tape),
             (len(data) + arguments.record_size - 1) // arguments.record_size,
             arguments.record_size))
    print('The tar it holds is %d bytes; "sum" on 2.11BSD should report: %05d %d'
          % (len(data), checksum, blocks))


if __name__ == '__main__':
    main()
