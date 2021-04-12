#!/bin/sh

UDP_ON='y'
SEAMLESS_SWITCH='n'
MULTICAST_ON='y'
S_SEARCH_TOKEN='0' #not important

GWIP='169.254.10.58'
MULTICAST_IP='225.0.100.011'

# Make sure IP is configured right.
# Setup jumbo packet?

# avahi daemon start

# load_videoip_driver_c

# _resolve_host_ip_addr

if [ "$SEAMLESS_SWITCH" = 'y' ]; then
	#ast_send_event -1 "e_heartbeat_init_ok"
	ast_heartbeat_no_tcp -c -m -d "$MULTICAST_IP" -t "$S_SEARCH_TOKEN" &
else
	#ast_heartbeat -c -m -d "$MULTICAST_IP" &
	ast_heartbeat -c -m -d "$GWIP" -t "$S_SEARCH_TOKEN" &
fi

## to_s_start_srv_hp
# load_videoip_c
if [ "$MULTICAST_ON" = 'n' ]; then
	if [ "$UDP_ON" = 'n' ]; then
		astclient -0 $GWIP -d -s V2 &
	else
		astclient -0 $GWIP -m udp -d -s V2 &
	fi
else
	if [ "$SEAMLESS_SWITCH" = 'y' ]; then
		astclient -0 $GWIP -m $MULTICAST_IP -d -s V2 -t no_tcp &
	else
		astclient -0 $GWIP -m $MULTICAST_IP -d -s V2 &
	fi
fi

# start_i2s_c
if [ "$MULTICAST_ON" = 'n' ]; then
	if [ "$UDP_ON" = 'n' ]; then
		client_i2s -t tcp -a $_GWIP &
	else
		client_i2s -a $_GWIP &
	fi
else
	if [ "$SEAMLESS_SWITCH" = 'y' ]; then
		client_i2s -m $_MULTICAST_IP -a $_GWIP -t no_tcp &
	else
		client_i2s -m $_MULTICAST_IP -a $_GWIP &
	fi
fi

