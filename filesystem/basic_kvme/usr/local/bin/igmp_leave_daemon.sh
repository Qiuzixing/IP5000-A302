#!/bin/sh

igmp -t 0x17 -m $1 2>&1 1>/dev/null
sleep 3
#send again
igmp -t 0x17 -m $1 2>&1 1>/dev/null