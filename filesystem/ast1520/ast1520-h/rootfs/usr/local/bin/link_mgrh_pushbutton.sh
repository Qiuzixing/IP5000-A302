#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

P_MODULE=p
P_CONTROL=p_ctrl
P_IPC_SET=p_lm_set
P_IPC_GET=p_lm_get
P_IPC_QUERY=p_lm_query
P_IPC_REPLY=p_lm_reply
P_PATCH=patch_p_lmh.sh
P_VAR=/var/p_lm_var
#P_HB_PORT=59006

. ip_mapping.sh

load_drv()
{
	return
}

init()
{
	return
}


start_service()
{
	poll_gpio.sh &
}

stop_service()
{
	pkill poll_gpio
}
################################################################################

update_session_id()
{
	P_SESSION_ID=$(( ($P_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$P_SESSION_ID" -lt 2 ]; then
		P_SESSION_ID='2'
	fi
}

to_s_srv_on()
{
	P_STATE='s_srv_on'
}

to_s_idle()
{
	P_STATE='s_idle'
}

to_s_stop_srv()
{
	P_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:$P_MODULE:$P_SESSION_ID

	if [ $NO_PWRBTN = 'n' ]; then
		stop_service
	fi
}

to_s_start_srv()
{
	P_STATE='s_start_srv'

	if [ "$NO_PWRBTN" = 'n' ]; then
		start_service
	fi

	to_s_srv_on
	# return as "P_STATE='s_srv_on'"
}


to_s_pre_start_srv()
{
	# Start HB and wait for client.
	P_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. P_MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$u_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:$P_MODULE:$P_SESSION_ID:uc:$MY_IP:$P_HB_PORT
	else
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:$P_MODULE:$P_SESSION_ID:mc:$P_MULTICAST_IP:$P_HB_PORT
	fi
}

handle_pe_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	P_CLIENT_IP="$2"

	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[attaching]\n"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ue_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$P_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $P_SESSION_ID"

		ipc @$P_IPC_REPLY s ignore
		return
	fi

	case "$P_STATE" in
		s_pre_start_srv)
			to_s_start_srv $P_SESSION_ID $P_CH_SELECT $P_MULTICAST_IP $P_CLIENT_IP
			# return as "P_STATE='s_srv_on'"

			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @$P_IPC_REPLY s ok
		;;
		*)
			ipc @$P_IPC_REPLY s ignore
		;;
	esac
}

handle_pe_start()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[start]\n"

	# validate input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_pe_stop
	# return as P_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()

	# get channel param
	P_CH_SELECT="$1"
	P_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX p $P_CH_SELECT`

	to_s_pre_start_srv $P_SESSION_ID $P_CH_SELECT $P_MULTICAST_IP
	# return as "P_STATE='s_pre_start_srv'"
}

handle_pe_stop()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[stop]\n"

	case "$P_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as P_STATE='s_idle'
		;;
	esac
}

handle_pe_no_heartbeat()
{
	#Parse ue_no_heartbeat:$P_SESSION_ID
	local _t="$1"

	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[no_heartbeat]\n"

	if [ "$P_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $P_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_pe_start	"$P_CH_SELECT"
}

handle_pe_lm_destroy()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[lm_destroy]\n"

	kill $PID_IPC_SVR

	# TODO

	exit 0
}

handle_pe_test()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[test]\n"
	return
}

handle_pe_patch()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[patch]\n"
	# Used to patch link_mgrX.sh itself.
	if [ -f ./$P_PATCH ]; then
		source ./$P_PATCH
	fi
}

handle_pe_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @p_lm_query q pe_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @$P_IPC_REPLY s "$_value"
}

handle_pe_param_set()
{
	# ipc @p_lm_set s pe_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_pe_var()
{
	echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[var]\n"
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > $P_VAR
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < $P_VAR |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> $P_VAR
		}
	) &
	wait $!
}

handle_pe_param_dump()
{
	handle_pe_var
	local _dump=`cat $P_VAR`
	ipc @$P_IPC_REPLY s "$_dump"
}

handle_pe_pwr_status_changed()
{
	PWR_STATUS=$1

	ipc @hb_ctrl s msg:p:piner:255.255.255.255:pe_status_changed:$1
}

handle_pe_query_pwr_status()
{
	if [ "$PWR_STATUS" = "N" ]; then
		return
	fi

	#echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[query_pwr_status]\n"
	local _request_ip="$1"
	ipc @hb_ctrl s msg:p:piner:$_request_ip:pe_status_changed:$PWR_STATUS
}

handle_pe_pwr_ctrl_press()
{
	echo -e "handle_pe_pwr_ctrl_press"
	echo none > ${GPIO_SYS_PATH}/pwr_ctrl/trigger
	echo 1 > ${GPIO_SYS_PATH}/pwr_ctrl/brightness
}

handle_pe_pwr_ctrl_release()
{
	echo -e "handle_pe_pwr_ctrl_release"
	echo none > ${GPIO_SYS_PATH}/pwr_ctrl/trigger
	echo 0 > ${GPIO_SYS_PATH}/pwr_ctrl/brightness
}

plm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @$P_IPC_QUERY" > /var/lock/@$P_IPC_QUERY.lck

	ipc_server @$P_IPC_SET @$P_IPC_GET @$P_IPC_QUERY @$P_IPC_REPLY &
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
	echo "ERROR!!!! pLM received signal!!!!!! Ignore it."
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
		pe_?*)
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

	# To ignore previous pending events, we init P_SESSION_ID at the beginning of event loop.
	P_SESSION_ID="$RANDOM"
	PWR_STATUS='N'

	update_session_id

	while true; do
		event=`ipc @p_lm_get g 2>/dev/null`
		# will get too many messages when many clients query power status
		# => move message to each handler
		# echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[$event]\n"
		event_loop_handler "$event"
	done
}

start_plm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi

	handle_pe_patch

	plm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &
}

start_plm
