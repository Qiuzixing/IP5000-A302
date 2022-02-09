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
C_PATCH=patch_c_lmc.sh
C_VAR=/var/c_lm_var
C_APP=cec

. ip_mapping.sh

###############################################################################

load_drv()
{
	insmod /usr/local/bin/cec.ko drv_option=$CEC_DRV_OPTION
}

init()
{
	return
}

init_post()
{
	return
}

start_service()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @$C_CONTROL q start:$C_SESSION_ID:uc:$C_ENCODER_IP:$C_TX_PORT:$C_TOPOLOPY_TX_PORT
	else
		ipc @$C_CONTROL q start:$C_SESSION_ID:mc:$C_MULTICAST_IP:$C_TX_PORT:$C_TOPOLOPY_TX_PORT
	fi
}

stop_service()
{
	echo 1 > $CEC_SYS_PATH/stop
}

###############################################################################
update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	C_SESSION_ID=$(( ($C_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$C_SESSION_ID" -lt 2 ]; then
		C_SESSION_ID='2'
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
	local _get_key='NO_CEC'
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
		NO_CEC=*)
			_is_disabled=${_info#NO_CEC=}
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
					echo "ERROR! Tx's NO_CEC=$_is_disabled" >&2
				;;
				*)
					echo "Disabled"
					echo "ERROR! Tx's NO_CEC=$_is_disabled" >&2
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
		NO_CEC=*)
			_PEER_NO_CEC=${_info#NO_CEC=}
			#echo "_PEER_NO_CEC=$_PEER_NO_CEC" >&2
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
	local _C_SESSION_ID="$1"
	local _C_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_C_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEEC_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_CEC=$NO_CEC

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @$C_IPC_SET s "ce_no_encoder_ip:$_C_SESSION_ID"
		;;
		*)
			# Found
			if [ "$_PEER_NO_CEC" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @$C_IPC_SET s ce_stop
			else
				_PEEC_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX r $_PEER_CH_SELECT`
				ipc @$C_IPC_SET s "ce_encoder_ip_got:$C_SESSION_ID:$_PEER_IP:$_PEEC_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	C_STATE='s_search'
	do_search_encoder $C_SESSION_ID $C_CH_SELECT &
}

to_s_start_hb()
{
	C_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:$C_MODULE:$C_SESSION_ID:uc:$C_ENCODER_IP:$C_HB_PORT
	else
		ipc @hb_ctrl s start_pinger:$C_MODULE:$C_SESSION_ID:mc:$C_MULTICAST_IP:$C_HB_PORT
	fi
}

to_s_srv_on()
{
	C_STATE='s_srv_on'
}

to_s_start_srv()
{
	C_STATE='s_start_srv'

	if [ "$NO_CEC" = 'n' ]; then
		start_service
	fi
}

to_s_idle()
{
	C_STATE='s_idle'
}

do_stop_srv()
{
	# C_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:$C_MODULE:$C_SESSION_ID


	if [ "$NO_CEC" = 'n' ]; then
		stop_service
	fi
}

handle_ce_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$C_SESSION_ID
	local _t="$1"

	if [ "$C_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $C_SESSION_ID"
		return
	fi
	to_s_start_srv $C_SESSION_ID $C_ENCODER_IP $C_MULTICAST_IP
	# return as C_STATE='s_start_srv'
	to_s_srv_on
	# return as C_STATE='s_srv_on'
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

handle_ce_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$C_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$C_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $C_SESSION_ID"
		return
	fi

	C_ENCODER_IP="$_encoder_ip"
	C_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $C_SESSION_ID $C_ENCODER_IP $C_MULTICAST_IP
}

handle_ce_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$C_SESSION_ID
	local _t="$1"

	if [ "$C_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $C_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $C_SESSION_ID $C_CH_SELECT
}

handle_ce_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$C_STATE" != 's_idle' ]; then
		do_stop_srv $C_SESSION_ID
	fi

	update_session_id

	# get channel param
	C_CH_SELECT="$1"
	C_MULTICAST_IP='0.0.0.0'

	if [ "$C_CH_SELECT" = "0000" ]; then
		to_s_idle
	else
		to_s_search $C_SESSION_ID $C_CH_SELECT
	fi
}

handle_ce_stop()
{
	do_stop_srv $C_SESSION_ID
	update_session_id
	to_s_idle
}

handle_ce_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
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

handle_ce_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
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
	echo "ERROR!!!! cLM received signal!!!!!! Ignore it."
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
		event=`ipc @c_lm_get g 2>>/ipc_fail.log`
		echo -e "\ncccccccccccccccccccccccccccccccccccccccccc\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: cLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
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

	if [ $CEC_SEND_DIR = 'hdmi_in' ];then
		echo 0 > /sys/devices/platform/cec/cec_report
		if [ $UGP_FLAG = 'success' ];then
			ipc @m_lm_set s cec_report:1
		fi
	elif [ $CEC_SEND_DIR = 'hdmi_out' ];then
		echo 1 > /sys/devices/platform/cec/cec_report
		if [ $UGP_FLAG = 'success' ];then
			ipc @m_lm_set s cec_report:0
		fi
	fi

}

start_clm
