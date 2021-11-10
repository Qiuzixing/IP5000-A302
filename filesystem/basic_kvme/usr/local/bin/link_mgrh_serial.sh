#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

S_MODULE=s
#S_CONTROL=s_ctrl
S_IPC_SET=s_lm_set
S_IPC_GET=s_lm_get
S_IPC_QUERY=s_lm_query
S_IPC_REPLY=s_lm_reply
S_PATCH=patch_s_lmh.sh
S_VAR=/var/s_lm_var
#S_HB_PORT=59005

. ip_mapping.sh

load_drv()
{
	return
}

init()
{
	return
}

watch_dog()
{
	# Bruce170505. Guest mode doesn't do multicast.
	# At this stage, multicast IP is unknown.
	while true; do
		$*
		if [ $? -eq 138 ]; then
			break
		fi
	done
}

load_soip_h()
{
	SOIP_GUEST_ON=`astparam g soip_guest_on`
	SOIP_PORT=`astparam g soip_port`
	SOIP_TYPE=`astparam g soip_type`
	S0_BAUDRATE=`astparam g s0_baudrate`

	echo "load_soip_h SOIP_GUEST_ON($SOIP_GUEST_ON) SOIP_PORT($SOIP_PORT) SOIP_TYPE($SOIP_TYPE) S0_BAUDRATE($S0_BAUDRATE)"

	case "$SOIP_TYPE" in
		1)
			if [ "$SOIP_GUEST_ON" = 'y' ]; then
				soip -c -d /dev/ttyS0
			else
				soip -h -d /dev/ttyS0 -b $S0_BAUDRATE -p "${HOSTNAME_PREFIX}${HOSTNAME_RX_MIDDLE}"
			fi
		;;
		2)
			if [ "$SOIP_GUEST_ON" = 'y' ]; then
				soip2 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT -p $SOIP_PORT
			else
				soip2 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT -p 6752
			fi
		;;
		3)
			# Bruce170505. Guest mode doesn't do multicast.
			if [ "$MULTICAST_ON" = 'y' ] && [ "$SOIP_GUEST_ON" = 'n' ]; then
				soip3 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT -m $S_MULTICAST_IP
			else
				soip3 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT
			fi
		;;
		*)
			echo "ERROR! Unsupported SOIP_TYPE($SOIP_TYPE)"
		;;
	esac
}

unload_soip_h()
{
	pkill soip
	# case "$SOIP_TYPE" in
	# 	1)
	# 		pkill soip 2>/dev/null
	# 	;;
	# 	2)
	# 		pkill soip2 2>/dev/null
	# 	;;
	# 	3)
	# 		pkill soip3 2>/dev/null
	# 	;;
	# 	*)
	# 		echo "ERROR! Unsupported SOIP_TYPE($SOIP_TYPE)"
	# 	;;
	# esac
}

start_service()
{
	load_soip_h &
}

stop_service()
{
	unload_soip_h
}
################################################################################

update_session_id()
{
	S_SESSION_ID=$(( ($S_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$S_SESSION_ID" -lt 2 ]; then
		S_SESSION_ID='2'
	fi
}

to_s_srv_on()
{
	S_STATE='s_srv_on'
}

to_s_idle()
{
	S_STATE='s_idle'
}

to_s_stop_srv()
{
	S_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:$S_MODULE:$S_SESSION_ID

	if [ $NO_SOIP = 'n' ]; then
		stop_service
	fi
}

to_s_start_srv()
{
	S_STATE='s_start_srv'

	if [ "$NO_SOIP" = 'n' ]; then
		start_service
	fi

	to_s_srv_on
	# return as "S_STATE='s_srv_on'"
}


to_s_pre_start_srv()
{
	# Start HB and wait for client.
	S_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. S_MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$u_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:$S_MODULE:$S_SESSION_ID:uc:$MY_IP:$S_HB_PORT
	else
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:$S_MODULE:$S_SESSION_ID:mc:$S_MULTICAST_IP:$S_HB_PORT
	fi
}

handle_se_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	S_CLIENT_IP="$2"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ue_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$S_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $S_SESSION_ID"

		ipc @$S_IPC_REPLY s ignore
		return
	fi

	case "$S_STATE" in
		s_pre_start_srv)
			to_s_start_srv $S_SESSION_ID $S_CH_SELECT $S_MULTICAST_IP $S_CLIENT_IP
			# return as "S_STATE='s_srv_on'"

			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @$S_IPC_REPLY s ok
		;;
		*)
			ipc @$S_IPC_REPLY s ignore
		;;
	esac
}

handle_se_start()
{
	SOIP_GUEST_ON=`astparam g soip_guest_on`
	SOIP_TYPE=`astparam g soip_type`

	echo "handle_se_start SOIP_GUEST_ON($SOIP_GUEST_ON) SOIP_TYPE($SOIP_TYPE)"

	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_se_stop
	# return as S_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()
	if [ "$SOIP_GUEST_ON" = 'y' ]; then
		start_service
		to_s_srv_on
	elif [ "$SOIP_TYPE" -eq 1 ]; then
		start_service
		to_s_srv_on
	else
		# get channel param
		S_CH_SELECT="$1"
		S_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX s $S_CH_SELECT`

		to_s_pre_start_srv $S_SESSION_ID $S_CH_SELECT $S_MULTICAST_IP
		# return as "S_STATE='s_pre_start_srv'"
	fi
}

handle_se_stop()
{
#	if [ "$SOIP_GUEST_ON" = 'y' ] || [ "$SOIP_TYPE" -eq 1 ]; then
#		return
#	fi

	case "$S_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as S_STATE='s_idle'
		;;
	esac
}

handle_se_no_heartbeat()
{
	#Parse ue_no_heartbeat:$S_SESSION_ID
	local _t="$1"

	if [ "$S_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $S_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_se_start	"$S_CH_SELECT"
}

handle_se_lm_destroy()
{
	kill $PID_IPC_SVR

	# TODO

	exit 0
}

handle_se_test()
{
	return
}

handle_se_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./$S_PATCH ]; then
		source ./$S_PATCH
	fi
}

handle_se_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @s_lm_query q se_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @$S_IPC_REPLY s "$_value"
}

handle_se_param_set()
{
	# ipc @s_lm_set s se_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_se_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > $S_VAR
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < $S_VAR |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> $S_VAR
		}
	) &
	wait $!
}

handle_se_param_dump()
{
	handle_se_var
	local _dump=`cat $S_VAR`
	ipc @$S_IPC_REPLY s "$_dump"
}

slm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @$S_IPC_QUERY" > /var/lock/@$S_IPC_QUERY.lck

	ipc_server @$S_IPC_SET @$S_IPC_GET @$S_IPC_QUERY @$S_IPC_REPLY &
	PID_IPC_SVR=$!

	load_drv

	init

	to_s_idle
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! sLM received signal!!!!!! Ignore it."
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
		se_?*)
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

	# To ignore previous pending events, we init S_SESSION_ID at the beginning of event loop.
	S_SESSION_ID="$RANDOM"

	update_session_id

	while true; do
		event=`ipc @s_lm_get g 2>/dev/null`
		echo -e "\nssssssssssssssssssssssssssssssssssssssssss\n[$event]\n"
		event_loop_handler "$event"
	done
}

s_guest_mode()
{
	# Bruce170505. Guest mode doesn't do multicast.
	# At this stage, multicast IP is unknown.
	while true; do
		load_soip_h
	done
}

s_type_1()
{
	while true; do
		load_soip_h
	done
}

start_slm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi

	handle_se_patch

	slm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# if [ "$SOIP_GUEST_ON" = 'y' ]; then
	# 	s_guest_mode &
	# 	to_s_srv_on
	# elif [ "$SOIP_TYPE" -eq 1 ]; then
	# 	s_type_1 &
	# 	to_s_srv_on
	# fi

	# start event loop
	event_loop &
}

start_slm
