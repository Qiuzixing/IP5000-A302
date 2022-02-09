#!/bin/sh
module="1500_i2s"
device="i2s"

# kill applications
pkill host_i2s 2>/dev/null
#pkill client_i2s 2>/dev/null

# invoke rmmod with all arguments we got
rmmod $module $* || exit 1
#rmmod usbip_common_mod.ko 2>/dev/null

# Remove stale nodes
rm -f /dev/${device} 

