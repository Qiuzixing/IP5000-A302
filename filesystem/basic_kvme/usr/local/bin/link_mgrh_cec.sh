#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

C_MODULE=c
C_CONTROL=c_ctrl
C_IPC_SET=c_lm_set
C_IPC_GET=c_lm_get
C_IPC_QUERY=c_lm_query
C_IPC_REPLY=c_lm_reply
C_PATCH=patch_c_lmh.sh
C_VAR=/var/c_lm_var
C_APP=cec

. ip_mapping.sh

load_drv()
{
	insmod /usr/local/bin/cec.ko drv_option=$CEC_DRV_OPTION
}

init()
{
	# trigger edid patch
	cat /sys/devices/platform/videoip/edid_cache > /sys/devices/platform/videoip/eeprom_content_ex
	return
}

init_post()
{
	return
}

start_service()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @$C_CONTROL q start:$C_SESSION_ID:uc:$C_CLIENT_IP:$C_TX_PORT:$C_TOPOLOPY_TX_PORT
	else
		ipc @$C_CONTROL q start:$C_SESSION_ID:mc:$C_MULTICAST_IP:$C_TX_PORT:$C_TOPOLOPY_TX_PORT
	fi
}

stop_service()
{
	echo 1 > $CEC_SYS_PATH/stop
}
################################################################################

update_session_id()
{
	C_SESSION_ID=$(( ($C_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$C_SESSION_ID" -lt 2 ]; then
		C_SESSION_ID='2'
	fi
}

to_s_srv_on()
{
	C_STATE='s_srv_on'
}

to_s_idle()
{
	C_STATE='s_idle'
}

to_s_stop_srv()
{
	C_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:$C_MODULE:$C_SESSION_ID

	if [ $NO_CEC = 'n' ]; then
		stop_service
	fi
}

to_s_start_srv()
{
	C_STATE='s_start_srv'

	if [ "$NO_CEC" = 'n' ]; then
		start_service
	fi

	to_s_srv_on
	# return as "C_STATE='s_srv_on'"
}


to_s_pre_start_srv()
{
	# Start HB and wait for client.
	C_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. C_MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$u_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:$C_MODULE:$C_SESSION_ID:uc:$MY_IP:$C_HB_PORT
	else
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:$C_MODULE:$C_SESSION_ID:mc:$C_MULTICAST_IP:$C_HB_PORT
	fi
}

handle_ce_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	C_CLIENT_IP="$2"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ue_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$C_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $C_SESSION_ID"

		ipc @$C_IPC_REPLY s ignore
		return
	fi

	case "$C_STATE" in
		s_pre_start_srv)
			to_s_start_srv $C_SESSION_ID $C_CH_SELECT $C_MULTICAST_IP $C_CLIENT_IP
			# return as "C_STATE='s_srv_on'"

			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @$C_IPC_REPLY s ok
		;;
		*)
			ipc @$C_IPC_REPLY s ignore
		;;
	esac
}

handle_ce_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_ce_stop
	# return as C_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()

	# get channel param
	C_CH_SELECT="$1"
	C_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX r $C_CH_SELECT`

	to_s_pre_start_srv $C_SESSION_ID $C_CH_SELECT $C_MULTICAST_IP
	# return as "C_STATE='s_pre_start_srv'"
}

handle_ce_stop()
{
	case "$C_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as C_STATE='s_idle'
		;;
	esac
}

handle_ce_no_heartbeat()
{
	#Parse ue_no_heartbeat:$C_SESSION_ID
	local _t="$1"

	if [ "$C_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $C_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_ce_start	"$C_CH_SELECT"
}

handle_ce_lm_destroy()
{
	kill $PID_IPC_SVR

	# TODO

	exit 0
}

handle_ce_test()
{
	return
}

handle_ce_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./$C_PATCH ]; then
		source ./$C_PATCH
	fi
}

handle_ce_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @c_lm_query q ce_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @$C_IPC_REPLY s "$_value"
}

handle_ce_param_set()
{
	# ipc @c_lm_set s ce_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_ce_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > $C_VAR
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < $C_VAR |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> $C_VAR
		}
	) &
	wait $!
}

handle_ce_param_dump()
{
	handle_ce_var
	local _dump=`cat $C_VAR`
	ipc @$C_IPC_REPLY s "$_dump"
}

handle_ce_gw()
{
	local _para1=$1

	echo "handle_ce_gw.($_para1)" 
}

handle_ce_send()
{
	local _para1=$1

	echo "handle_ce_send.($_para1)" 
}

clm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @$C_IPC_QUERY" > /var/lock/@$C_IPC_QUERY.lck
	echo "lock file for @$C_CONTROL" > /var/lock/@$C_CONTROL.lck

	ipc_server @$C_IPC_SET @$C_IPC_GET @$C_IPC_QUERY @$C_IPC_REPLY &
	PID_IPC_SVR=$!

	load_drv

	init

	$C_APP &

	to_s_idle

	init_post
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! rLM received signal!!!!!! Ignore it."
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
		ce_?*)
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

	# To ignore previous pending events, we init C_SESSION_ID at the beginning of event loop.
	C_SESSION_ID="$RANDOM"

	update_session_id

	while true; do
		event=`ipc @c_lm_get g 2>/dev/null`
		echo -e "\ncccccccccccccccccccccccccccccccccccccccccc\n[$event]\n"
		event_loop_handler "$event"
	done
}

start_clm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi

	handle_ce_patch

	clm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &
}

start_clm
