#!/bin/sh

cd /sys/devices/platform/i2c
echo 1 9a > io_select
echo 0 100 > i_range
echo "Bank 0"
cat io_value

echo "Bank 1"
echo 0xf 1 > io_value
cat io_value
echo 0xf 0 > io_value


