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

# Work out whether the tape still reflects the sources. Everything git ignores
# is skipped, or the tape would always look out of date compared to itself.
if [ ! -f "$tape_image" ]; then
    tape_reason="it doesn't exist yet"
else
    changed=$(find "$project_directory" \
                   \( -name .git -o -name data -o -name __pycache__ \
                      -o -name '*.tap' -o -name '*.tar' -o -name '*.o' \
                      -o -name '*.dsk' -o -name '*.xz' -o -name '*.gz' \) -prune \
                   -o -type f -newer "$tape_image" -print 2>/dev/null | head -1)

    if [ -n "$changed" ]; then
        tape_reason="$(basename "$changed") is newer"
    else
        tape_reason=""
    fi
fi

if [ -n "$tape_reason" ]; then
    if have_python; then
        echo "*** Building the tape image, because $tape_reason"
        "$tape_builder" -o "$tape_image"
        echo
    else
        echo "The tape image needs rebuilding, but python3 isn't installed." >&2
        exit 1
    fi
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
