#!/bin/bash
set -e

# Attempt to emit full umockdev recording for a USB device to tmp.umockdev
# This takes care of the annoying nature of subdevices and should result in
# a complete tree, with required hid nodes too

BUS="$1"
DEVICE="$2"

if [[ -z "$BUS" ]]; then
    echo "Must provide bus and device"
    exit 1
fi

if [[ -z "$DEVICE" ]]; then
    echo "Must provide bus and device"
    exit 1
fi

if [[ ! -e "/dev/bus/usb/${BUS}/${DEVICE}" ]] ; then
    echo "Specified USB device does not exist"
    exit 1
fi

# Figure out the sysfs node for the device
SYSFS_NODE=$(echo "/sys`udevadm info /dev/bus/usb/${BUS}/${DEVICE}|head -n1|awk '{print $2}'`")

if [[ ! -e "${SYSFS_NODE}" ]]; then
    echo "Cannot find sysfs node"
    exit 1
fi

# Find the device ID to perform a split for ports
devID=`basename "${SYSFS_NODE}"`

# Find all directories containing an exposed device and merge them
NODES=""
for item in `find ${SYSFS_NODE}/${devID}:* -maxdepth 2 -name modalias -type f`; do
    NODES+="`dirname $item` "
done

# Record it.
sudo umockdev-record $NODES > tmp.umockdev
