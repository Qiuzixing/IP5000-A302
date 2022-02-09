#!/bin/sh

if [ "$1" = 'on' ]; then
	dmesg -n 8 && setconsole -r
else
	dmesg -n 1 && setconsole /dev/ttyS1
fi

