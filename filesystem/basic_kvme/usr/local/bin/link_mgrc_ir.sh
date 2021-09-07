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
R_PATCH=patch_r_lmc.sh
R_VAR=/var/r_lm_var
R_APP=client_ir2
R_GUEST_IRRD=irrd
R_GUEST_IRSD=irsd
#R_HB_PORT=59004
#R_TX_PORT=18770
#R_RX_PORT=18770

. ip_mapping.sh

###############################################################################

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
		ipc @$R_CONTROL q start:$R_SESSION_ID:uc:$R_ENCODER_IP:$R_TX_PORT:$MY_IP:$R_TX_PORT
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

###############################################################################
update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	R_SESSION_ID=$(( ($R_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$R_SESSION_ID" -lt 2 ]; then
		R_SESSION_ID='2'
	fi
}

_get_ip_addr()
{
	local _ch_select=$1
	local _ip=""
	local _node_info=""
	local _info
	local _is_disabled=""
	local _match_key
	local _get_key='NO_IR'
	# The $CH_SELECT should always be valid at this stage.
	case "$_ch_select" in
		?*.?*.?*.?*)
			#if there's a dot ".", it's definitely ip address, no name resolution needed
			_ip=$_ch_select
			# If failed to ping the host, just return 'unknown host'.
			if ! { ping "$_ip" | grep -q " is alive!"; }; then
				_ip="Unknown"
				echo "$_ip"
				return
			fi
			_match_key='MY_IP'
			#echo "$_ip"
		;;
		*)
			#no dot ".", assume friendly CH_SELECT name
			#_ip=`node_query --get_key MY_IP --match_key CH_SELECT=$_ch_select --match_key IS_HOST=y --max 1 | grep "MY_IP=" | sed 's/^MY_IP=//'`
			_match_key='CH_SELECT'
		;;
	esac
	_node_info=`node_query --get_key MY_IP --get_key $_get_key --match_key $_match_key=$_ch_select --match_key IS_HOST=y --max 1`
	for _info in $_node_info; do
		case "$_info" in
		MY_IP=*)
			_ip=${_info#MY_IP=}
		;;
		NO_IR=*)
			_is_disabled=${_info#NO_IR=}
		;;
		*)
		;;
		esac
	done
	# Validate result
	case "$_ip" in
		?*.?*.?*.?*)
			case "$_is_disabled" in
				n)
					echo "$_ip"
				;;
				y)
					echo "Disabled"
					echo "ERROR! Tx's NO_IR=$_is_disabled" >&2
				;;
				*)
					echo "Disabled"
					echo "ERROR! Tx's NO_IR=$_is_disabled" >&2
				;;
			esac
		;;
		*)
			echo "Unknown"
			echo "Unknown host ($_ip)" >&2
		;;
	esac
}

_get_node_info()
{
	local _ch_select=$1
	local _node_info=""
	local _match_key

	# The $CH_SELECT should always be valid at this stage.
	case "$_ch_select" in
		?*.?*.?*.?*)
			#if there's a dot ".", it's definitely ip address, no name resolution needed
			# If failed to ping the host, just return empty string
			if ! { ping "$_ch_select" | grep -q " is alive!"; }; then
				echo ""
				echo "ping fail?! ($_ch_select)" >&2
				return
			fi
			_match_key='MY_IP'
		;;
		*)
			#no dot ".", assume friendly CH_SELECT name
			_match_key='CH_SELECT'
		;;
	esac
	_node_info=`node_query --dump --match_key $_match_key=$_ch_select --match_key IS_HOST=y --match_key HOSTNAME_PREFIX=$HOSTNAME_PREFIX --max 1`
	echo "$_node_info"
}

_parse_node_info()
{
	local _info

	#echo -e "try to parse node_info:[[\n$_PEER_NODE_INFO\n]]" >&2

	# NOTE! DO NOT "" $_PEER_NODE_INFO, otherwise parsing will be wrong.
	for _info in $_PEER_NODE_INFO; do
		case "$_info" in
		MY_IP=*)
			_PEER_IP=${_info#MY_IP=}
			#echo "_PEER_IP=$_PEER_IP" >&2
		;;
		CH_SELECT=*)
			_PEER_CH_SELECT=${_info#CH_SELECT=}
			#echo "_PEER_CH_SELECT=$_PEER_CH_SELECT" >&2
		;;
		NO_IR=*)
			_PEER_NO_IR=${_info#NO_IR=}
			#echo "_PEER_NO_IR=$_PEER_NO_IR" >&2
		;;
		*)
		;;
		esac
	done

	# Validate result
	case "$_PEER_IP" in
		?*.?*.?*.?*)
			# valid peer id
			return
		;;
		*)
			_PEER_IP='Unknown'
			echo "Unknown host ($_ip)" >&2
		;;
	esac
}

do_search_encoder()
{
	local _R_SESSION_ID="$1"
	local _R_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_R_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_IR=$NO_IR

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @$R_IPC_SET s "re_no_encoder_ip:$_R_SESSION_ID"
		;;
		*)
			# Found
			if [ "$_PEER_NO_IR" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @$R_IPC_SET s re_stop
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX r $_PEER_CH_SELECT`
				ipc @$R_IPC_SET s "re_encoder_ip_got:$R_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	R_STATE='s_search'
	do_search_encoder $R_SESSION_ID $R_CH_SELECT &
}

to_s_start_hb()
{
	R_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:$R_MODULE:$R_SESSION_ID:uc:$R_ENCODER_IP:$R_HB_PORT
	else
		ipc @hb_ctrl s start_pinger:$R_MODULE:$R_SESSION_ID:mc:$R_MULTICAST_IP:$R_HB_PORT
	fi
}

to_s_srv_on()
{
	R_STATE='s_srv_on'
}

to_s_start_srv()
{
	R_STATE='s_start_srv'

	if [ "$NO_IR" = 'n' ]; then
		start_service
	fi
}

to_s_idle()
{
	R_STATE='s_idle'
}

do_stop_srv()
{
	# R_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:$R_MODULE:$R_SESSION_ID


	if [ "$NO_IR" = 'n' ]; then
		stop_service
	fi
}

handle_re_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$R_SESSION_ID
	local _t="$1"

	if [ "$R_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $R_SESSION_ID"
		return
	fi
	to_s_start_srv $R_SESSION_ID $R_ENCODER_IP $R_MULTICAST_IP
	# return as R_STATE='s_start_srv'
	to_s_srv_on
	# return as R_STATE='s_srv_on'
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

handle_re_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$R_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$R_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $R_SESSION_ID"
		return
	fi

	R_ENCODER_IP="$_encoder_ip"
	R_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $R_SESSION_ID $R_ENCODER_IP $R_MULTICAST_IP
}

handle_re_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$R_SESSION_ID
	local _t="$1"

	if [ "$R_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $R_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $R_SESSION_ID $R_CH_SELECT
}

handle_re_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$R_STATE" != 's_idle' ]; then
		do_stop_srv $R_SESSION_ID
	fi

	update_session_id

	# get channel param
	R_CH_SELECT="$1"
	R_MULTICAST_IP='0.0.0.0'

	to_s_search $R_SESSION_ID $R_CH_SELECT
}

handle_re_stop()
{
	do_stop_srv $R_SESSION_ID
	update_session_id
	to_s_idle
}

handle_re_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
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

handle_re_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
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
		event=`ipc @r_lm_get g 2>>/ipc_fail.log`
		echo -e "\nrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: rLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
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
}

start_rlm
