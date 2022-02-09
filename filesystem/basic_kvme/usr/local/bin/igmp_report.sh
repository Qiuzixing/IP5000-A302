#!/bin/sh
#
# $1 is source IP address in X.X.X.X format
# 225.1.0.0:	name service
# 225.1.0.1:	node query
# 224.0.0.251:	Bonjour/mDNS/Avahi

IP_LIST="225.1.0.0 225.1.0.1 224.0.0.251"
AGGRESSIVE_CNT=15

igmp_report_send()
{
	for ip in $IP_LIST; do
		igmp -t 0x16 -s $1 -m $ip 2>&1 1>/dev/null
	done
}

# no idea why switch doesn't get packets which we sent at first
# add additonal packet sending before main loop
# 1 report packet/2secs for 30 seconds

for (( i=0 ; i < $AGGRESSIVE_CNT ; i++ )); do
	igmp_report_send $1
	sleep 2
done

while true; do
	igmp_report_send $1
	sleep 30
done
