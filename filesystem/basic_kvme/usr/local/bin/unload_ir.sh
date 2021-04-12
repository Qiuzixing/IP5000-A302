#!/bin/sh
module="IRoIP"
device="iroip"

# kill applications
pkill host_ir 2>/dev/null
pkill client_ir 2>/dev/null

# invoke rmmod with all arguments we got
rmmod $module $* || exit 1

# Remove stale nodes
rm -f /dev/${device} 

