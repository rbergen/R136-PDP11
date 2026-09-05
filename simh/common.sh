# Paths and helpers shared by setup.sh and run.sh. This file is sourced by
# those scripts, not run on its own.

simh_directory=$(cd "$(dirname "$0")" && pwd)

disk_image="$simh_directory/2.11BSD_rq.dsk"
tape_image="$simh_directory/r136.tap"
configuration="$simh_directory/r136.ini"
tape_builder="$simh_directory/mktape.py"
pid_file="$simh_directory/.simulator.pid"

image_url=https://github.com/chasecovello/211bsd-pidp11/raw/master/2.11BSD_rq.dsk.xz

# Prints the path to the PDP-11 simulator and returns 0, or returns 1 if it
# isn't installed. Distributions don't agree on where to put it, or even on
# what to call it.
find_simulator()
{
    for candidate in pdp11 simh-pdp11
    do
        path=$(command -v "$candidate" 2>/dev/null)
        if [ -n "$path" ]; then
            echo "$path"
            return 0
        fi
    done

    for candidate in /usr/lib/simh/pdp11 /usr/local/lib/simh/pdp11 \
                     /usr/share/simh/pdp11 /opt/simh/pdp11 /opt/pidp11/bin/pdp11
    do
        if [ -x "$candidate" ]; then
            echo "$candidate"
            return 0
        fi
    done

    return 1
}

# mktape.py needs Python 3, which is the only thing here that does.
have_python()
{
    [ -n "$(command -v python3 2>/dev/null)" ]
}

# Returns 0 if a simulator started from this directory is still running, and
# sets $running to its pid. Two simulators sharing one disk image will quietly
# destroy it, so run.sh checks before starting another.
simulator_is_running()
{
    [ -f "$pid_file" ] || return 1

    running=$(cat "$pid_file" 2>/dev/null)
    [ -n "$running" ] || return 1

    kill -0 "$running" 2>/dev/null
}
