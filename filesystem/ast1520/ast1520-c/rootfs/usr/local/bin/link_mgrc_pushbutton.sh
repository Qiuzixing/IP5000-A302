#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.
P_MODULE=p
#P_CONTROL=p_ctrl
P_IPC_SET=p_lm_set
P_IPC_GET=p_lm_get
P_IPC_QUERY=p_lm_query
P_IPC_REPLY=p_lm_reply
P_PATCH=patch_p_lmc.sh
P_VAR=/var/p_lm_var
#P_HB_PORT=59006

. ip_mapping.sh

###############################################################################

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
	echo 'pushbutton' > "${GPIO_SYS_PATH}"/button_remote_ctrl/trigger
}

stop_service()
{
	echo 'none' > "${GPIO_SYS_PATH}"/button_remote_ctrl/trigger
}

###############################################################################
update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	P_SESSION_ID=$(( ($P_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$P_SESSION_ID" -lt 2 ]; then
		P_SESSION_ID='2'
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
	local _get_key='NO_PWRBTN'
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
		NO_PWRBTN=*)
			_is_disabled=${_info#NO_PWRBTN=}
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
					echo "ERROR! Tx's NO_PWRBTN=$_is_disabled" >&2
				;;
				*)
					echo "Disabled"
					echo "ERROR! Tx's NO_PWRBTN=$_is_disabled" >&2
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
		NO_PWRBTN=*)
			_PEER_NO_PWRBTN=${_info#NO_PWRBTN=}
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
	local _P_SESSION_ID="$1"
	local _P_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_P_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_PWRBTN=$NO_PWRBTN

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @$P_IPC_SET s "pe_no_encoder_ip:$_P_SESSION_ID"
		;;
		*)
			# Found
			if [ "$_PEER_NO_PWRBTN" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @$P_IPC_SET s pe_stop
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX p $_PEER_CH_SELECT`
				ipc @$P_IPC_SET s "pe_encoder_ip_got:$_P_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	S_STATE='s_search'
	do_search_encoder $P_SESSION_ID $P_CH_SELECT &
}

to_s_start_hb()
{
	S_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:$P_MODULE:$P_SESSION_ID:uc:$P_ENCODER_IP:$P_HB_PORT
	else
		ipc @hb_ctrl s start_pinger:$P_MODULE:$P_SESSION_ID:mc:$P_MULTICAST_IP:$P_HB_PORT
	fi
}

to_s_srv_on()
{
	S_STATE='s_srv_on'
}

to_s_start_srv()
{
	S_STATE='s_start_srv'

	if [ "$NO_PWRBTN" = 'n' ]; then
		ipc @hb_ctrl s msg:p:acker:0.0.0.0:pe_query_pwr_status:"$MY_IP"
		start_service
	fi
}

to_s_idle()
{
	S_STATE='s_idle'
}

do_stop_srv()
{
	# P_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:$P_MODULE:$P_SESSION_ID


	if [ "$NO_PWRBTN" = 'n' ]; then
		stop_service
	fi
}

handle_pe_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$P_SESSION_ID
	local _t="$1"

	if [ "$P_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $P_SESSION_ID"
		return
	fi
	to_s_start_srv $P_SESSION_ID $P_ENCODER_IP $P_MULTICAST_IP
	# return as S_STATE='s_start_srv'
	to_s_srv_on
	# return as S_STATE='s_srv_on'
}

handle_pe_no_heartbeat()
{
	#Parse ue_no_heartbeat:$P_SESSION_ID
	local _t="$1"

	if [ "$P_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $P_SESSION_ID"
		return
	fi

	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_pe_start	"$P_CH_SELECT"
}

handle_pe_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$P_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$P_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $P_SESSION_ID"
		return
	fi

	P_ENCODER_IP="$_encoder_ip"
	P_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $P_SESSION_ID $P_ENCODER_IP $P_MULTICAST_IP
}

handle_pe_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$P_SESSION_ID
	local _t="$1"

	if [ "$P_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $P_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $P_SESSION_ID $P_CH_SELECT
}

handle_pe_start()
{
	# validate input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$S_STATE" != 's_idle' ]; then
		do_stop_srv $P_SESSION_ID
	fi

	update_session_id

	# get channel param
	P_CH_SELECT="$1"
	P_MULTICAST_IP='0.0.0.0'
	if [ "$P_CH_SELECT" = "0000" ]; then
		to_s_idle
	else
		to_s_search $P_SESSION_ID $P_CH_SELECT
	fi
}

handle_pe_stop()
{
	do_stop_srv $P_SESSION_ID
	update_session_id
	to_s_idle
}

handle_pe_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
}

handle_pe_patch()
{
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

handle_pe_status_changed()
{
	echo "$1" > "${GPIO_SYS_PATH}"/gpio_pwr_status_out/brightness
}

handle_pe_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
}

handle_pe_pwr_ctrl_press()
{
	ipc @hb_ctrl s msg:p:acker:0.0.0.0:pe_pwr_ctrl_press
}

handle_pe_pwr_ctrl_release()
{

	ipc @hb_ctrl s msg:p:acker:0.0.0.0:pe_pwr_ctrl_release
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

	update_session_id

	while true; do
		event=`ipc @p_lm_get g 2>>/ipc_fail.log`
		echo -e "\npppppppppppppppppppppppppppppppppppppppppp\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: pLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
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
