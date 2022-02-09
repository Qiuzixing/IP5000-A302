#!/bin/sh

while true; do
	sleep "$1"
	#to do: check subsystems
	#tickle watchdog
	#echo > /sys/devices/platform/watchdog/tickle
	ast_send_event -1 tick
done
