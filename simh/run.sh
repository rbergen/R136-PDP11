#!/bin/sh

# Starts 2.11BSD under SIMH, rebuilding the tape image that carries R136
# onto the emulated machine whenever the sources have moved on.
#
# Anything you pass to this script is handed straight to the simulator.
# Run setup.sh once before the first use.

set -e

. "$(dirname "$0")/common.sh"

if simulator=$(find_simulator); then
    :
else
    echo "The PDP-11 simulator isn't installed. Run ./setup.sh first." >&2
    exit 1
fi

if [ ! -f "$disk_image" ]; then
    echo "$disk_image is missing. Run ./setup.sh first." >&2
    exit 1
fi

if simulator_is_running; then
    echo "A simulator is already running against $disk_image, as pid $running." >&2
    echo "Two of them sharing one disk image will corrupt it beyond repair, so" >&2
    echo "shut that one down first. See 'Shutting down' in README.md." >&2
    exit 1
fi

# Bring the tape up to date. mktape.py already knows exactly which files belong
# on it, so it decides whether the one we have is still current: keeping a
# second copy of that list here is how the two would quietly drift apart.
if have_python; then
    "$tape_builder" --if-needed -o "$tape_image" || exit 1
    echo
elif [ ! -f "$tape_image" ]; then
    echo "There is no tape image yet, and python3 isn't installed to build one." >&2
    echo "Run ./setup.sh, which checks for it." >&2
    exit 1
else
    echo "python3 isn't installed, so I can't tell whether the tape image is" >&2
    echo "still current. Carrying on with the one that is already there." >&2
    echo >&2
fi

cat <<'EOF'
*** Starting 2.11BSD

    It counts down and boots by itself; press Enter to skip the wait. Log in
    as "root", which has no password. To copy R136 onto the machine:

        mkdir -p /usr/src/r136 && cd /usr/src/r136
        dd if=/dev/rmt0 of=r136.tar bs=512
        tar xf r136.tar && sync
        cd r136 && ./build.csh

    When you are done, shut down with "sync; sync; halt" and only then quit the
    simulator with Ctrl-E followed by q. Killing it costs you recent writes.

EOF

# Record our own pid before handing this process over to the simulator: exec
# keeps the pid, so the file ends up naming the simulator itself.
echo $$ > "$pid_file"

# The configuration refers to the disk and tape images by relative name.
cd "$simh_directory"
exec "$simulator" "$configuration" "$@"
