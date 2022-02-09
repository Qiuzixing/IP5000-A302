#!/bin/sh

# $1: SCUD8 value

source calc_ring.sh

if [ -z "$1" ]; then
	calc_oscclk dly32
else
	calc_oscclk dly32_check "$1"
fi
printf "Ring SCU10[29:16] for 0x%X(0x%X): 0x%X, $RING_HZ Hz, $RING_NS ns\n" $SCUD8 $DLYSTAGE $RING
