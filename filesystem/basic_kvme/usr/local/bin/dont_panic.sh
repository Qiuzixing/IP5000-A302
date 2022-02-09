#!/bin/sh

echo no > /sys/devices/platform/watchdog/enable
echo 999999 > /proc/sys/kernel/panic

# This is the script to print date time
{ cnt='0'; while { echo -n "CNT=$cnt "; date; }; do sleep 60; cnt=`expr $cnt + 1`; done; } &

