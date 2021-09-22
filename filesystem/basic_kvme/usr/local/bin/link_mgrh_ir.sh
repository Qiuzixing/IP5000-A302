#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

R_MODULE=r
R_CONTROL=r_ctrl
R_IPC_SET=r_lm_set
R_IPC_GET=r_lm_get
R_IPC_QUERY=r_lm_query
R_IPC_REPLY=r_lm_reply
R_PATCH=patch_r_lmh.sh
R_VAR=/var/r_lm_var
R_APP=host_ir2
R_GUEST_IRRD=irrd
R_GUEST_IRSD=irsd

#R_HB_PORT=59004
#R_TX_PORT=18770
#R_RX_PORT=18770

. ip_mapping.sh

load_drv()
{
	load_ir2.sh
}

guest_irrd()
{
	_IFS="$IFS";IFS='_';set -- $IR_SW_DECODE_NEC_CFG;shift 0;IFS="$_IFS"

	local _addr=$1

	while true; do
		$R_GUEST_IRRD $_addr
		sleep 2
	done
}

guest_irsd()
{
	while true; do
		$R_GUEST_IRSD
		sleep 2
	done
}

ir_guest_mode()
{
	guest_irsd &
}

ir_decode()
{
	guest_irrd &
}

init()
{
	return
}

init_post()
{
	if [ "$IR_GUEST_ON" = 'y' ]; then
		ipc @$R_CONTROL q guest:0:uc:0.0.0.0:$R_TX_PORT
		ir_guest_mode
	fi
	if [ "$IR_SW_DECODE_ON" = 'y' ]; then
		ipc @$R_CONTROL q decode
		ir_decode
	fi
}

start_service()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @$R_CONTROL q start:$R_SESSION_ID:uc:$R_CLIENT_IP:$R_TX_PORT:$MY_IP:$R_TX_PORT
	else
		ipc @$R_CONTROL q start:$R_SESSION_ID:mc:$R_MULTICAST_IP:$R_TX_PORT:$R_MULTICAST_IP:$R_TX_PORT
	fi
}

stop_service()
{
	echo 1 > $IR_SYS_PATH/stop

	if [ "$IR_GUEST_ON" = 'y' ]; then
		ipc @$R_CONTROL q guest:0:uc:0.0.0.0:$R_TX_PORT
	fi
	if [ "$IR_SW_DECODE_ON" = 'y' ]; then
		ipc @$R_CONTROL q decode
		# local socket got deleted in driver, re-init by delete irrd and run it again
		killall $R_GUEST_IRRD
	fi
}
################################################################################

update_session_id()
{
	R_SESSION_ID=$(( ($R_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$R_SESSION_ID" -lt 2 ]; then
		R_SESSION_ID='2'
	fi
}

to_s_srv_on()
{
	R_STATE='s_srv_on'
}

to_s_idle()
{
	R_STATE='s_idle'
}

to_s_stop_srv()
{
	R_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:$R_MODULE:$R_SESSION_ID

	if [ $NO_IR = 'n' ]; then
		stop_service
	fi
}

to_s_start_srv()
{
	R_STATE='s_start_srv'

	if [ "$NO_IR" = 'n' ]; then
		start_service
	fi

	to_s_srv_on
	# return as "R_STATE='s_srv_on'"
}


to_s_pre_start_srv()
{
	# Start HB and wait for client.
	R_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. R_MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$u_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:$R_MODULE:$R_SESSION_ID:uc:$MY_IP:$R_HB_PORT
	else
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:$R_MODULE:$R_SESSION_ID:mc:$R_MULTICAST_IP:$R_HB_PORT
	fi
}

handle_re_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	R_CLIENT_IP="$2"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ue_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$R_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $R_SESSION_ID"

		ipc @$R_IPC_REPLY s ignore
		return
	fi

	case "$R_STATE" in
		s_pre_start_srv)
			to_s_start_srv $R_SESSION_ID $R_CH_SELECT $R_MULTICAST_IP $R_CLIENT_IP
			# return as "R_STATE='s_srv_on'"

			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @$R_IPC_REPLY s ok
		;;
		*)
			ipc @$R_IPC_REPLY s ignore
		;;
	esac
}

handle_re_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_re_stop
	# return as R_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()

	# get channel param
	R_CH_SELECT="$1"
	R_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX r $R_CH_SELECT`

	to_s_pre_start_srv $R_SESSION_ID $R_CH_SELECT $R_MULTICAST_IP
	# return as "R_STATE='s_pre_start_srv'"
}

handle_re_stop()
{
	case "$R_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as R_STATE='s_idle'
		;;
	esac
}

handle_re_no_heartbeat()
{
	#Parse ue_no_heartbeat:$R_SESSION_ID
	local _t="$1"

	if [ "$R_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $R_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_re_start	"$R_CH_SELECT"
}

handle_re_lm_destroy()
{
	kill $PID_IPC_SVR

	# TODO

	exit 0
}

handle_re_test()
{
	return
}

handle_re_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./$R_PATCH ]; then
		source ./$R_PATCH
	fi
}

handle_re_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @r_lm_query q re_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @$R_IPC_REPLY s "$_value"
}

handle_re_param_set()
{
	# ipc @r_lm_set s re_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_re_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > $R_VAR
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < $R_VAR |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> $R_VAR
		}
	) &
	wait $!
}

handle_re_param_dump()
{
	handle_re_var
	local _dump=`cat $R_VAR`
	ipc @$R_IPC_REPLY s "$_dump"
}

handle_re_dir()
{
	#Parse re_dir:$DIR
	echo "re_dir!!! $1"

	case "$1" in
		in)
			ipc @m_lm_set s set_gpio_config:1:71:1
			ipc @m_lm_set s set_gpio_val:1:71:1
			echo 1 > $IR_SYS_PATH/type
		;;
		out)
			ipc @m_lm_set s set_gpio_config:1:71:1
			ipc @m_lm_set s set_gpio_val:1:71:0
			echo 0 > $IR_SYS_PATH/type			
		;;		
		*)
			echo "ERROR!!!! Invalid event ($_1) received"
		;;
	esac	

}

handle_re_gw()
{
	local _para1=$1

	echo "handle_re_gw.($_para1)" 
}

handle_re_send()
{
	local _para1=$1

	echo "handle_re_send.($_para1)" 
}

rlm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @$R_IPC_QUERY" > /var/lock/@$R_IPC_QUERY.lck
	echo "lock file for @$R_CONTROL" > /var/lock/@$R_CONTROL.lck

	ipc_server @$R_IPC_SET @$R_IPC_GET @$R_IPC_QUERY @$R_IPC_REPLY &
	PID_IPC_SVR=$!

	load_drv

	init

	$R_APP &

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
		re_?*)
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

	# To ignore previous pending events, we init R_SESSION_ID at the beginning of event loop.
	R_SESSION_ID="$RANDOM"

	update_session_id

	while true; do
		event=`ipc @r_lm_get g 2>/dev/null`
		echo -e "\nrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n[$event]\n"
		event_loop_handler "$event"
	done
}

start_rlm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi

	handle_re_patch

	rlm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &

	if [ $P3KCFG_IR_DIR = 'out' ];then
		e e_p3k_ir_dir::out
	else
		e e_p3k_ir_dir::in
	fi
}

start_rlm
