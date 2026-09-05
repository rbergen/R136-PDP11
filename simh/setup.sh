#!/bin/sh

# Gets this machine ready to run 2.11BSD: installs SIMH's PDP-11 simulator and
# downloads a 2.11BSD disk image, skipping whichever of the two you already
# have. Run this once, then use run.sh from then on.

set -e

. "$(dirname "$0")/common.sh"

install_simulator()
{
    if [ -n "$(command -v apt-get 2>/dev/null)" ]; then
        echo "Installing the simh package with apt-get. This needs sudo."
        sudo apt-get update
        sudo apt-get install -y simh
    elif [ -n "$(command -v brew 2>/dev/null)" ]; then
        echo "Installing simh with Homebrew."
        brew install simh
    elif [ -n "$(command -v dnf 2>/dev/null)" ]; then
        echo "Installing simh with dnf. This needs sudo."
        sudo dnf install -y simh
    elif [ -n "$(command -v pacman 2>/dev/null)" ]; then
        echo "Installing simh with pacman. This needs sudo."
        sudo pacman -S --noconfirm simh
    else
        cat <<'EOF'
I don't recognise this system's package manager, so you'll have to install the
simulator yourself. You need SIMH's "pdp11" binary; either your package manager
has a "simh" package, or you can build it from https://github.com/open-simh/simh
(the pdp11 target is the only one you need).

Once it is on your PATH, run this script again.
EOF
        exit 1
    fi
}

echo "*** Checking for the PDP-11 simulator"

if simulator=$(find_simulator); then
    echo "    Found $simulator"
else
    install_simulator

    if simulator=$(find_simulator); then
        echo "    Found $simulator"
    else
        echo
        echo "The package installed, but I still can't find a pdp11 binary." >&2
        echo "If it went somewhere unusual, put it on your PATH and try again." >&2
        exit 1
    fi
fi

echo
echo "*** Checking for the 2.11BSD disk image"

if [ -f "$disk_image" ]; then
    echo "    Found $disk_image"
else
    echo "    Downloading (about 24 MB, and roughly 1 GB once unpacked)"

    # Download to a scratch name first, so an interrupted transfer never leaves
    # something behind that looks like a complete image.
    curl -L --fail -o "$disk_image.part" "$image_url"
    mv "$disk_image.part" "$disk_image.xz"

    echo "    Decompressing"
    unxz "$disk_image.xz"
    echo "    Wrote $disk_image"
fi

echo
echo "*** Checking for Python 3"

if have_python; then
    echo "    Found $(command -v python3)"
else
    echo "    Not found. mktape.py needs it to build the tape image that gets" >&2
    echo "    R136 onto the emulated machine. Install Python 3 and run" >&2
    echo "    this script again." >&2
    exit 1
fi

echo
echo "*** Ready. Start the machine with: ./run.sh"
