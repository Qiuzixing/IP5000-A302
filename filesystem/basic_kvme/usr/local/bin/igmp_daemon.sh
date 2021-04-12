#!/bin/sh

echo "start igmp_report_daemon"
echo "MULTICAST_IP = $1"
while true; do
	sleep 5
	if [ -n "$1" ]; then
		#send AV streaming group membership report
		igmp -t 0x16 -m $1 2>&1 1>/dev/null
		sleep 5
	fi
	#send name_service group membership report
	igmp -t 0x16 -m 225.1.0.0 2>&1 1>/dev/null
	sleep 5
	#send avahi mDNS group membership report
	igmp -t 0x16 -m 224.0.0.251 2>&1 1>/dev/null
done
