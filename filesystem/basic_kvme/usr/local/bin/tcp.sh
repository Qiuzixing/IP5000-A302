#!/bin/sh

echo 1 > /proc/sys/net/ipv4/tcp_timestamps
echo 20 > /proc/sys/net/ipv4/tcp_reordering

echo 28896000 > /proc/sys/net/core/wmem_max
echo 28896000 > /proc/sys/net/core/rmem_max
echo 4194304 > /proc/sys/net/core/rmem_default
echo 6201344 > /proc/sys/net/core/wmem_default

echo 4096 2796202 28896000 > /proc/sys/net/ipv4/tcp_wmem
echo 4096 5592405 28896000 > /proc/sys/net/ipv4/tcp_rmem

# $1 is the MTU size
if [ -n "$1" ]; then
	ifconfig eth0 mtu $1
fi

