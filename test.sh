#!/bin/bash
set -e
set -x

if [[ -e build ]] ; then
    rm -rf build
fi

jobCount=-j$(getconf _NPROCESSORS_ONLN)

meson build --buildtype debugoptimized -Dwith-tests=yes
ninja -C build $jobCount

# Normal
meson test -C build
