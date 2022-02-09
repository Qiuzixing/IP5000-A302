#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

#U_HB_PORT=59007

. ip_mapping.sh

update_session_id()
{
	U_SESSION_ID=$(( ($U_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$U_SESSION_ID" -lt 2 ]; then
		U_SESSION_ID='2'
	fi
}

start_kmoip_h()
{
	if [ "$MULTICAST_ON" = 'y' ]; then
		kmoip -m $U_MULTICAST_IP
	else
		kmoip -u $U_CLIENT_IP
	fi
}

stop_kmoip_h()
{
	echo 0 0 > $VHUB_SYS_PATH/attach_kmoip
}

to_s_srv_on()
{
	U_STATE='s_srv_on'
}

to_s_idle()
{
	U_USBIP_CLIENT='0.0.0.0'
	ast_send_event -1 e_ulm_notify_chg
	U_STATE='s_idle'
}

to_s_stop_srv()
{
	U_STATE='s_stop_srv'

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:u:$U_SESSION_ID

	# We are not going to remove vhub.ko anymore.
	#rmvhubh.sh &
	vhub_detach_all.sh

	# Always keep kmoip alive
	if [ "$NO_KMOIP" = 'n' ] && [ "$RELOAD_KMOIP" -eq 1 ]; then
		stop_kmoip_h
	fi
}

to_s_start_srv()
{
	U_STATE='s_start_srv'

	# Unicast must re-start KMoIP every time becuase unicast IP changed.
	if [ "$MULTICAST_ON" = 'n' ]; then
		# Multicast is now started in to_s_pre_start_srv() stage.
		if [ "$NO_KMOIP" = 'n' ] && [ "$RELOAD_KMOIP" -eq 1 ]; then
			start_kmoip_h
			RELOAD_KMOIP=0
		fi
	fi

	to_s_srv_on
	# return as "U_STATE='s_srv_on'"
}

handle_ue_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	U_CLIENT_IP="$2"

	#
	# Both unicast and multicast only reply one client.
	#
	if [ "$U_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $U_SESSION_ID"

		ipc @u_lm_reply s ignore
		return
	fi

	case "$U_STATE" in
		s_pre_start_srv)
			to_s_start_srv $U_SESSION_ID $U_CH_SELECT $U_MULTICAST_IP $U_CLIENT_IP

			# Bruce170705. @hb_ctrl may block in ipc.c if too many request at the same time.
			# If we don't replay ue_attaching back to HB before next "ipc @hb_ctrl s",
			# we will dead-lock HB forever.
			#
			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @u_lm_reply s ok

			# return as "U_STATE='s_srv_on'"
			# First attached client grant USBIP
			if [ "$MULTICAST_ON" = 'y' ] && [ "$SHARE_USB" = 'y' ]; then
				if [ "$SHARE_USB_ON_FIRST_PEER" = 'y' ]; then
					ipc @hb_ctrl s msg:u:pinger:$U_CLIENT_IP:ue_request_usbip
				fi
			fi
		;;
		## Bruce170405. HB always notify no_heartbeat in current design.
		#s_srv_on)
		#	# First attached client grant USBIP
		#	if [ "$MULTICAST_ON" = 'y' ] && [ "$SHARE_USB" = 'y' ]; then
		#		if [ "$SHARE_USB_ON_FIRST_PEER" = 'y' ]; then
		#			ipc @hb_ctrl s msg:u:pinger:$U_CLIENT_IP:ue_request_usbip
		#		fi
		#	fi
		#	# ue_attaching is triggered by hb through 'query'
		#	# query/reply must be called in pair and better in the same event handler.
		#	ipc @u_lm_reply s ok
		#;;
		*)
			ipc @u_lm_reply s ignore
		;;
	esac
}

handle_ue_request_usbip()
{
	# This event is typically triggered by client through msg_channel.
	# ue_request_usbip:U_USBIP_CLIENT_IP

	# $1: requester's sequence ID
	# $2: requester's IP
	local _request_id="$1"
	local _request_ip="$2"

	# Stop linked client under share_usb=y case.
	if [ "$MULTICAST_ON" = 'y' ] && [ "$SHARE_USB" = 'y' ]; then
		if [ "$U_USBIP_CLIENT" != '0.0.0.0' ] && [ "$U_USBIP_CLIENT" != "$_request_ip" ]; then
			ipc @hb_ctrl s msg:u:pinger:$U_USBIP_CLIENT:ue_stop_usbip
			# Bruce170425. Under some unknown case, host still have
			# socket connected with disconnected client.
			# Bruce guess it is caused by TCP socket disconnect problem.
			# To avoid this issue, we explicitly detach client.
			vhub_detach_all.sh
		fi
	fi
	U_USBIP_CLIENT="$_request_ip"
	ipc @hb_ctrl s msg:u:pinger:$U_USBIP_CLIENT:ue_start_usbip:"$_request_id"
	ast_send_event -1 e_ulm_notify_chg
}

to_s_pre_start_srv()
{
	# Start HB and wait for client.
	U_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$U_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:u:$U_SESSION_ID:uc:$MY_IP:$U_HB_PORT
	else
		# For multicast, there is no need for KMoIP to wait for client connect.
		# Start KMoIP here so that KMoIP roaming can work when there is only
		# one client (the KMR master) enabled KMoIP.
		if [ "$NO_KMOIP" = 'n' ] && [ "$RELOAD_KMOIP" -eq 1 ]; then
			start_kmoip_h
			RELOAD_KMOIP=0
		fi
		# TBD: HB's multicast ip and port number.
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:u:$U_SESSION_ID:mc:$U_MULTICAST_IP:$U_HB_PORT
	fi
}

handle_ue_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$U_CH_SELECT" != "$1" ]; then
		RELOAD_KMOIP=1
	fi

	# stop service first if necessary.
	handle_ue_stop
	# return as U_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()

	# get channel param
	U_CH_SELECT="$1"
	U_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX u $U_CH_SELECT`

	to_s_pre_start_srv $U_SESSION_ID $U_CH_SELECT $U_MULTICAST_IP
	# return as "U_STATE='s_pre_start_srv'"
}

handle_ue_stop()
{
	case "$U_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			# Bruce170726. Unicast's KMoIP uses unicast socket for both up and down stream.
			# We need to reload KMoIP and restart new unicast socket on reconnect.
			# Fixed RctBug#2017072400.
			if [ "$MULTICAST_ON" = 'n' ]; then
				RELOAD_KMOIP=1
			fi
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as U_STATE='s_idle'
		;;
	esac
}

handle_ue_no_heartbeat()
{
	#Parse ue_no_heartbeat:$U_SESSION_ID
	local _t="$1"

	if [ "$U_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $U_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_ue_start	"$U_CH_SELECT"
}

handle_ue_kmr_chg()
{
	local x=$1
	local y=$2
	# ue_kmr_chg:x:y
	if [ "$NO_KMOIP" = 'n' ]; then
		echo "$x" "$y" > $VHUB_SYS_PATH/kmr_resolution
	fi
}

handle_ue_lm_destroy()
{
	kill $PID_IPC_SVR

	# TODO

	exit 0
}

handle_ue_test()
{
	return
}

handle_ue_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_u_lmh.sh ]; then
		source ./patch_u_lmh.sh
	fi
}

handle_ue_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @u_lm_query q ue_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @u_lm_reply s "$_value"
}

handle_ue_param_set()
{
	# ipc @u_lm_set s ue_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	shift
	local _value=$(echo $@ | sed 's/ /:/g')

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_ue_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > /var/u_lm_var
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < /var/u_lm_var |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> /var/u_lm_var
		}
	) &
	wait $!
}

handle_ue_param_dump()
{
	handle_ue_var
	local _dump=`cat /var/u_lm_var`
	ipc @u_lm_reply s "$_dump"
}

ulm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @u_lm_query" > /var/lock/@u_lm_query.lck

	ipc_server @u_lm_set @u_lm_get @u_lm_query @u_lm_reply &
	PID_IPC_SVR=$!

	vhubh.sh
	echo $USB_SET_ADDR_HACK > $VHUB_SYS_PATH/set_addr_hack
	echo $USB_HID_URB_INTERVAL > $VHUB_SYS_PATH/hid_urb_interval
	echo $USB_QUIRK > $VHUB_SYS_PATH/usb_quirk
	if [ "$NO_KMOIP" = 'n' ]; then
		# FIXME. from astparam.
		echo 1920 1080 > $VHUB_SYS_PATH/kmr_resolution
	fi
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! uLM received signal!!!!!! Ignore it."
	echo ""
	echo ""
	echo ""
}

event_loop_handler()
{
	#Parse $event. Firt param is event, the others are parameters.
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _e=$1
	shift 1
	local _p=$@

	#echo "event=$_e; param=$_p"

	case "$_e" in
		ue_?*)
			handle_"$_e" $_p
		;;
		*)
			echo "ERROR!!!! Invalid event ($_e) received"
		;;
	esac
}

event_loop()
{
	# Try to ignore all TERM signals.
	trap signal_handler SIGTERM SIGPIPE SIGHUP SIGINT SIGALRM SIGUSR1 SIGUSR2 SIGPROF SIGVTALRM

	# To ignore previous pending events, we init U_SESSION_ID at the beginning of event loop.
	U_SESSION_ID="$RANDOM"
	RELOAD_KMOIP=1

	update_session_id

	to_s_idle

	# NOTE:
	# There can be ONLY one thread calling @u_lm_get and @u_lm_query/@u_lm_reply
	# There can be multiple thread calling @u_lm_set
	while true; do
		event=`ipc @u_lm_get g 2>/dev/null`
		echo -e "\nuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\n[$event]\n"
		event_loop_handler "$event"
	done
}

start_ulm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi
	if [ -f ./patch_u_lmh.sh ]; then
		source ./patch_u_lmh.sh
	fi

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	ulm_init
	# start event loop
	event_loop &
}

start_ulm
