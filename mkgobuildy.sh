#!/bin/bash
set -e

PROJ_NAME="linux-driver-management"
JOBCOUNT=$(getconf _NPROCESSORS_ONLN)

# Brain-dead default library directory
LIBDIR="/usr/lib"
DISTRO_ID=""
DISTRO_LIKE=""

# Attempt OS detection
# Based on Neal's work on budgie-rd mkgobuildy.sh
detect_os()
{
    echo "Performing OS detection"
    if [[ -e /etc/os-release ]]; then
        source /etc/os-release
    elif [[ -e /usr/lib/os-release ]]; then
        source /usr/lib/os-release
    fi

    DISTRO_ID="${ID}"
    DISTRO_LIKE="${ID_LIKE}"

    # Let's deal with the Debian family special case, since they like being different...
    if [[ "$ID_LIKE" == "debian" ]]; then
        # We'll just ask dpkg for the architecture
        LIBDIR="/usr/lib/`dpkg-architecture -qDEB_HOST_MULTIARCH`"
        echo "Multiarch host"
    else
        # Everyone but Debian follows LSB FHS, so we'll just check if it's a known 64-bit arch and /usr/lib64 is in use...
        if ([[ `uname -m` == "x86_64" ]] || [[ `uname -m` == "aarch64" ]] || [[ `uname -m` =~ "ppc64" ]]) && [[ -e "/usr/lib64" ]]; then
            LIBDIR="/usr/lib64"
        fi
    fi
}

# Stock configure which accepts additional arguments
configure_meson()
{
    echo "Configuring ${PROJ_NAME}"
    meson build  --buildtype debugoptimized --prefix=/usr --libdir="${LIBDIR}" --sysconfdir=/etc $*
}

MESON_OPTIONS="-Dwith-tests=yes"

# Configure with our requirements
if [[ ! -d build ]]; then
    detect_os
    if [[ "${DISTRO_ID}" == "solus" ]]; then
        MESON_OPTIONS+=" -Dwith-gl-driver-switch=true -Dwith-autostart-dir=/usr/share/xdg/autostart"
    fi
    configure_meson $MESON_OPTIONS
fi

echo "Building ${PROJ_NAME}"
ninja -C build -j${JOBCOUNT}
