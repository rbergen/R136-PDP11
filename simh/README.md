# Running R136 on 2.11BSD under SIMH

You don't need a PDP-11 on your desk to build and play R136. [SIMH](https://github.com/open-simh/simh)
emulates one well enough to run 2.11BSD at a very respectable speed, and everything in this
directory is what you need to get from nothing to a running game.

> ### Read [Shutting down](#shutting-down) before you start
>
> 2.11BSD has to be halted properly, and the account you log in as cannot do it directly. Quit
> the simulator with the machine still running and the disk image is silently damaged: files come
> back with the right size and nothing in them but NUL bytes. It is the easiest way to lose an
> afternoon here, and it does not look like damage when it happens.

This is also a good place to reproduce bugs. The emulated machine is a real 2.11BSD system, so
K&R C, 16-bit pointers, a 64 KB address space and the original BSD curses all behave exactly as
they do on the real hardware.

## Quick start

On your own machine:

```sh
cd simh
./setup.sh
./run.sh
```

`setup.sh` installs the simulator and downloads a 2.11BSD disk image, skipping whichever of the
two you already have. It only needs to be run once. `run.sh` is what you use from then on: it
rebuilds the tape image whenever the sources have changed, and starts the machine.

The simulator counts down and boots by itself; press Enter to skip the wait. At `login:`, log in
as **`user`**, which has no password. Then, on the PDP-11:

```csh
cd
tar xvf /dev/rmt0
cd r136
./build.csh
./r136 -l en
```

Compiling all nine source files takes roughly twenty to thirty minutes of emulated time, so put
the kettle on. When you're finished, shut the machine down as described under
[Shutting down](#shutting-down).

Everything below assumes you are logged in as `user`, whose shell is `tcsh`.

## What is in this directory

| File | What it does |
| --- | --- |
| `setup.sh` | Installs the simulator and fetches the disk image, if you don't have them |
| `run.sh` | Rebuilds the tape image when needed and starts the machine |
| `r136.ini` | SIMH configuration: an 11/70, the system disk, and a tape drive |
| `mktape.py` | Packs the game's sources into a tape image that 2.11BSD's `tar` can read |
| `common.sh` | Paths and helpers the two scripts share; not run on its own |

The disk image and the tape image are both generated, and are ignored by git.

## What setup.sh does

It looks for SIMH's `pdp11` binary under the various names and locations distributions use for
it, and installs the `simh` package if it can't find one. `apt-get`, Homebrew, `dnf` and `pacman`
are all tried, though only the `apt-get` path has actually been exercised. On anything else, if
the install doesn't produce a working binary the script says so and leaves you to it. Installing
packages needs `sudo`, so expect to be asked for your password.

The disk image is Chase Covello's [PiDP-11 distribution](https://github.com/chasecovello/211bsd-pidp11)
at **patch level 482**, which is about as modern as 2.11BSD gets. It is about 24 MB to download
and roughly 1 GB once unpacked, although most of that is empty space your filesystem will happily
leave as a hole. If you already own a PiDP-11 you have this image already, in
`/opt/pidp11/systems/211bsd/`, and can drop it in here instead.

## Putting R136 on a tape

2.11BSD has no shared folders, and setting up networking is more trouble than it is worth for
this. A tape is the path of least resistance: `mktape.py` writes a tar archive wrapped in SIMH's
`.tap` container, and `r136.ini` attaches it as `/dev/rmt0`.

What goes on it is what the PDP-11 needs to build and play the game: the sources, `texts`, `tools`
and `build.csh`. This directory stays behind, along with the top level `README.md`, since both are
about getting to the PDP-11 rather than anything you do once you are there.

`run.sh` builds the tape for you. It rebuilds whenever one of those files is newer than the tape
image, and tells you which one triggered it:

```
*** Building the tape image, because init.c is newer
Wrote .../r136.tap: 239212 bytes, 460 records of 512 bytes
The tar it holds is 235520 bytes; "sum" on 2.11BSD should report: 15274 230
```

Keep that last number: it is what `sum` should say on the other side. You can also invoke
`./mktape.py` yourself; `--help` lists what it takes.

## Getting R136 off the tape

The tape drive is `/dev/rmt0`, and it is readable by anyone, so you do not need to be root.

Logged in as `user` your home directory is `/home/user`, and that is where R136 should go.
`/usr/src` is the traditional home for sources on a Unix system, but it belongs to root and you
cannot write there:

```
% mkdir /usr/src/r136
mkdir: /usr/src/r136: Permission denied
```

So change to your home directory and unpack the tape into it:

```csh
cd
tar xvf /dev/rmt0
```

That is all there is to it. `tar` reads the tape directly, with no `dd` first and no blocking
factor to get right, and creates an `r136` directory in the current one:

```
x r136/build.csh, 297 bytes, 1 tape blocks
x r136/cmd.c, 20945 bytes, 41 tape blocks
x r136/cmd.h, 278 bytes, 1 tape blocks
...
```

### Checking the transfer first

If you would rather be sure before you unpack, copy the tape to a file and compare its checksum
against the one `run.sh` printed:

```csh
cd
dd if=/dev/rmt0 of=r136.tar bs=512
sum r136.tar
tar xf r136.tar
```

`sum` prints a checksum and a block count, and both should match. This is worth doing whenever
something inexplicable happens later; see [Troubleshooting](#troubleshooting).

### Reading the tape more than once

`/dev/rmt0` rewinds when it is closed, so you can read it as often as you like in one session.
`/dev/nrmt0` is the same drive without the rewind. If a read ever stops early, `mt rew` winds
back to the start and `mt status` reports what the drive thinks is going on.

### Picking up newer sources

The tape is attached when the simulator starts, so it is a snapshot of your sources at that
moment. After editing on your own machine, shut the PDP-11 down and run `./run.sh` again. It
will notice and rebuild the tape, ready for you to unpack once more.

## Building and running

`build.csh` compiles the game and the `gendata` tool, generates the data files from `texts/`, and
rolls a tarball:

```csh
cd ~/r136
./build.csh
```

The game finds its texts through the relative path `data/<language>/`, so **run it from the
directory that contains `data/`**. If you don't, it tells you so and exits:

```
Error opening file data/nl/p0!
Could not read the game texts from data/nl.
Run this game from the directory that holds the data directory.
```

It defaults to Dutch. `./r136 -l en` gives you English, and `./r136 -h` lists what is available.
<kbd>Ctrl</kbd>-<kbd>C</kbd> quits at any point and puts your terminal back the way it found it.

For reference, on patch level 482 the result is a comfortable fit in the PDP-11's 64 KB:

```
% size r136
text    data    bss     dec     hex
35650   3068    3854    42572   a64c
```

## Logging in

The image ships three accounts, none of which have a password:

| Account | Home | Shell | Use it for |
| --- | --- | --- | --- |
| `user` | `/home/user` | `tcsh` | Everything in this guide |
| `root` | `/` | `sh` | Shutting the machine down, and creating accounts |
| `tektronix` | `/home/tektronix` | `tcsh` | Tektronix 4010 graphics demos |

`user` is uid 100, in the group `users`, and is deliberately not in `wheel`, so it cannot become
root:

```
% su root
You do not have permission to su root
```

To do anything that needs root, log out with `exit` and log back in as `root`.

### Creating your own account

Changes to the disk image are permanent, so an account you create now will still be there
tomorrow, as long as you shut the machine down properly. There is no `adduser` on 2.11BSD; the
tool is `vipw`, which edits `/etc/master.passwd` and regenerates `/etc/passwd` from it. **As
`root`:**

```sh
vipw
```

Add a line in the ten-field format `name:password:uid:gid:class:change:expire:gecos:home:shell`.
An empty password field means no password, which is how the shipped accounts are set up:

```
rutger::1001:100::0:0:Rutger van Bergen,,,:/home/rutger:/bin/csh
```

> **`vipw` decides whether you changed anything by comparing modification times, and the clock
> here ticks once per second.** If you save within the same second that `vipw` started your
> editor, it prints `vipw: no changes made` and quietly throws your edit away. Take your time.

Then give the account somewhere to live, and, if you want it to be able to shut the machine down
without logging out, a place in the `wheel` group:

```sh
mkdir /home/rutger
cp /home/user/.login /home/user/.cshrc /home/rutger
chown -R rutger /home/rutger && chgrp -R users /home/rutger
chmod 755 /home/rutger && chmod 644 /home/rutger/.cshrc /home/rutger/.login

vi /etc/group          # wheel:*:0:root  ->  wheel:*:0:root,rutger
sync; sync
```

`passwd rutger` sets a password if you want one. `/etc/shells` allows `/bin/sh`, `/bin/csh` and
`/bin/tcsh`. `/home` is its own filesystem with plenty of room, so it is a good place to keep
R136.

## Shutting down

**This is the one thing you have to get right.**

`halt` belongs to root, and is not even on your path as `user`:

```
% halt
halt: Command not found.
% su root
You do not have permission to su root
```

So shutting down means logging out first. From your `user` session:

```
% exit

login: root
# sync
# sync
# halt
```

Wait for `halted by root`, and only then press <kbd>Ctrl</kbd>-<kbd>E</kbd> to reach the `sim>`
prompt and type `q` to quit the simulator.

If you skip this and kill the simulator instead, its buffer cache goes with it. `fsck` cleans up
on the next boot and reports things like `PARTIALLY TRUNCATED INODE (SALVAGED)`, but files you
recently wrote come back **with the correct size and filled with NUL bytes**, including, if you
are unlucky, the sources you were about to compile.

If you would rather not log out every time, put your own account in the `wheel` group as
described above; `su root` then works and you can halt from there.

## Simulator control

<kbd>Ctrl</kbd>-<kbd>E</kbd> drops you from the emulated console to SIMH's own `sim>` prompt.
From there `c` continues, `q` quits, and `help` lists the rest.

Anything you pass to `run.sh` is handed on to the simulator, so `./run.sh -q` starts it quietly.

By default the machine has no serial lines and no networking: everything happens on the console.
If you would rather use a real terminal emulator, `r136.ini` has a commented-out DZ11 section;
enable it and `telnet localhost 4000` once the simulator is running.

## Gotchas

**Only ever run one simulator against the disk image.** Two of them sharing it will destroy the
filesystem, and `fsck` will not be able to put it back together. `run.sh` refuses to start a
second one, but it cannot stop you from launching `pdp11 r136.ini` by hand.

**Always halt cleanly.** See [Shutting down](#shutting-down).

**`user` runs tcsh, not sh.** `$status` rather than `$?`, and `|&` rather than `2>&1`. Typing
Bourne shell syntax gets you `Illegal variable name.` and a command that never ran.

**Tape records must be 512 bytes.** `tar`'s default blocking factor of 20 gives 10240-byte
records, which SIMH and the 2.11BSD tape driver both accept without complaint, and the files
then arrive with correct sizes and corrupted contents. `mktape.py` writes 512-byte records for this
reason; do not raise `--record-size` unless you enjoy debugging phantoms. Reading is not fussy:
plain `tar xvf /dev/rmt0` copes with whatever it finds.

**The terminal is 24x80 and does not resize.** `$TERM` is `vt100` and `stty size` reports
`24 0` on the console; curses falls back to termcap's 80 columns. `/.profile` and `/home/*/.login`
set `stty rows 24` by hand, so adjust those if your terminal is a different height.

## Troubleshooting

| Symptom | Cause |
| --- | --- |
| `cc` reports things like `KEY_ESCAPE undefined` or `FIL undefined` for identifiers that are plainly there | A header is a run of NUL bytes after an unclean shutdown. Check it with `sum`, unpack the tape again, `sync`. |
| Files have the right size but the wrong contents | Either an unclean shutdown, or a tape written with records larger than 512 bytes. |
| `halt: Command not found.` | Also `user`. Log out and log back in as `root` to shut down. |
| `Error opening file data/nl/p0!` | You are not in the directory that holds `data/`, or `build.csh` never got as far as running `tools/gendata`. |
| `A simulator is already running` from `run.sh` | You have another one open somewhere. Shut it down properly before starting a second. |
| Your edits aren't on the machine | The tape is a snapshot taken at boot. Halt, `./run.sh` again, unpack again. |
| `setup.sh` installs a package but still can't find `pdp11` | Your distribution puts it somewhere `common.sh` doesn't look. Add the path there, or put the binary on your `PATH`. |
