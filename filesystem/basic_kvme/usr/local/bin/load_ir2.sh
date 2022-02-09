#!/bin/sh
module="IRoIP2"
device="iroip"
mode="777"

insmod ./$module.ko $* || exit 1

rm -f /dev/${device}

major=`cat /proc/devices | awk "\\$2==\"$module\" {print \\$1}"`
mknod /dev/${device} c $major 0
