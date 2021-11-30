#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.
S_MODULE=s
S_CONTROL=s_ctrl
S_IPC_SET=s_lm_set
S_IPC_GET=s_lm_get
S_IPC_QUERY=s_lm_query
S_IPC_REPLY=s_lm_reply
S_PATCH=patch_s_lmc.sh
S_VAR=/var/s_lm_var
#S_HB_PORT=59005

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

load_soip_c()
{
	SOIP_TYPE=`astparam g soip_type`
	if echo "$SOIP_TYPE" | grep -q "not defined" ; then
		SOIP_TYPE=`astparam r soip_type`
		if echo "$SOIP_TYPE" | grep -q "not defined" ; then
			# Check legacy soip_type2
			SOIP_TYPE2=`astparam g soip_type2`
			if echo "$SOIP_TYPE2" | grep -q "not defined" ; then
				SOIP_TYPE2=`astparam r soip_type2`
			fi
			# 1, 2 or 3. Default type 2.
			case "$SOIP_TYPE2" in
				y)
					SOIP_TYPE='2'
				;;
				n)
					SOIP_TYPE='1'
				;;
				*)
					SOIP_TYPE='2'
					SOIP_TYPE2='y'
				;;
			esac
		fi
	fi

	SOIP_GUEST_ON=`astparam g soip_guest_on`
	if echo "$SOIP_GUEST_ON" | grep -q "not defined" ; then
		SOIP_GUEST_ON=`astparam r soip_guest_on`
		if echo "$SOIP_GUEST_ON" | grep -q "not defined" ; then
			SOIP_GUEST_ON='y'
		fi
	fi

	SOIP_PORT=`astparam g soip_port`
	if echo "$SOIP_PORT" | grep -q "not defined" ; then
		SOIP_PORT=`astparam r soip_port`
		if echo "$SOIP_PORT" | grep -q "not defined" ; then
			SOIP_PORT='5001'
		fi
	fi

	S0_BAUDRATE=`astparam g s0_baudrate`
	if echo "$S0_BAUDRATE" | grep -q "not defined" ; then
		S0_BAUDRATE=`astparam r s0_baudrate`
		if echo "$S0_BAUDRATE" | grep -q "not defined" ; then
			S0_BAUDRATE='115200-8n1'
		fi
	fi

	echo "load_soip_c SOIP_GUEST_ON($SOIP_GUEST_ON) SOIP_PORT($SOIP_PORT) SOIP_TYPE($SOIP_TYPE) S0_BAUDRATE($S0_BAUDRATE)"

	case "$SOIP_TYPE" in
		1)
			soip -c -d /dev/ttyS0
		;;
		2)
			if [ "$SOIP_GUEST_ON" = 'y' ]; then
				soip2 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT -p $SOIP_PORT
			else
				soip2 -c -f /dev/ttyS0 -d $S_ENCODER_IP -b $S0_BAUDRATE -p 6752
			fi
		;;
		3)
			if [ "$SOIP_GUEST_ON" = 'y' ]; then
				soip3 -h -f /dev/ttyS0 -b $S0_BAUDRATE -o $SOIP_TOKEN_TIMEOUT
			else
				if [ "$MULTICAST_ON" = 'y' ]; then
					soip3 -c -f /dev/ttyS0 -d $S_ENCODER_IP -b $S0_BAUDRATE -m $S_MULTICAST_IP
				else
					soip3 -c -f /dev/ttyS0 -d $S_ENCODER_IP -b $S0_BAUDRATE
				fi
			fi
		;;
		*)
			echo "ERROR! Unsupported SOIP_TYPE($SOIP_TYPE)"
		;;
	esac
}

unload_soip_c()
{
	pkill soip
}

start_service()
{
	# soip will not return, put it in background
	load_soip_c &
}

stop_service()
{
	unload_soip_c
}

###############################################################################
update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	S_SESSION_ID=$(( ($S_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$S_SESSION_ID" -lt 2 ]; then
		S_SESSION_ID='2'
	fi
}

_get_ip_addr()
{
	local _ch_select=$1
	local _ip=""
	local _node_info=""
	local _info
	local _is_disabled="" _is_guest_on="n" _type=$SOIP_TYPE
	local _match_key
	local _get_key1='NO_SOIP'
	local _get_key2='SOIP_GUEST_ON'
	local _get_key3='SOIP_TYPE' # 1, 2, or 3
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
	_node_info=`node_query --get_key MY_IP --get_key $_get_key1 --get_key $_get_key2 --get_key $_get_key3 --match_key $_match_key=$_ch_select --match_key IS_HOST=y --max 1`
	for _info in $_node_info; do
		case "$_info" in
		MY_IP=*)
			_ip=${_info#MY_IP=}
		;;
		NO_SOIP=*)
			_is_disabled=${_info#NO_SOIP=}
		;;
		SOIP_GUEST_ON=*)
			_is_guest_on=${_info#SOIP_GUEST_ON=}
		;;
		SOIP_TYPE=*)
			_type=${_info#SOIP_TYPE=}
		;;
		*)
		;;
		esac
	done

	# Validate result
	case "$_ip" in
		?*.?*.?*.?*)
			# Don't start service if
			# - peer is disabled. (NO_SOIP == y)
			# - peer is guest mode
			# - peer's type != my SOIP_TYPE
			if [ $_is_disabled != 'n' ] || [ $_is_guest_on != 'n' ] || [ $_type != $SOIP_TYPE ]; then
				echo "Disabled"
				echo "ERROR! Tx's NO_SOIP=$_is_disabled SOIP_GUEST_ON=$_is_guest_on SOIP_TYPE=$_type" >&2
			else
				echo "$_ip"
			fi
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
		NO_SOIP=*)
			_PEER_NO_SOIP=${_info#NO_SOIP=}
			#echo "_PEER_NO_SOIP=$_PEER_NO_SOIP" >&2
		;;
		SOIP_GUEST_ON=*)
			_PEER_SOIP_GUEST_ON=${_info#SOIP_GUEST_ON=}
		;;
		SOIP_TYPE=*)
			_PEER_SOIP_TYPE=${_info#SOIP_TYPE=}
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
	local _S_SESSION_ID="$1"
	local _S_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_S_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_SOIP=$NO_SOIP
	local _PEER_SOIP_GUEST_ON=$SOIP_GUEST_ON
	local _PEER_SOIP_TYPE=$SOIP_TYPE

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @$S_IPC_SET s "se_no_encoder_ip:$_S_SESSION_ID"
		;;
		*)
			# Found
			# Don't start service if
			# - peer is disabled. (NO_SOIP == y)
			# - peer is guest mode
			# - peer's type != my SOIP_TYPE
			if [ "$_PEER_NO_SOIP" != 'n' ] || [ "$_PEER_SOIP_GUEST_ON" != 'n' ] || [ "$_PEER_SOIP_TYPE" != "$SOIP_TYPE" ]; then
				# Peer doesn't enable this service.
				ipc @$S_IPC_SET s se_stop
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX s $_PEER_CH_SELECT`
				ipc @$S_IPC_SET s "se_encoder_ip_got:$_S_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	S_STATE='s_search'
	do_search_encoder $S_SESSION_ID $S_CH_SELECT &
}

to_s_start_hb()
{
	S_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:$S_MODULE:$S_SESSION_ID:uc:$S_ENCODER_IP:$S_HB_PORT
	else
		ipc @hb_ctrl s start_pinger:$S_MODULE:$S_SESSION_ID:mc:$S_MULTICAST_IP:$S_HB_PORT
	fi
}

to_s_srv_on()
{
	S_STATE='s_srv_on'
}

to_s_start_srv()
{
	S_STATE='s_start_srv'

	if [ "$NO_SOIP" = 'n' ]; then
		start_service
	fi
}

to_s_idle()
{
	S_STATE='s_idle'
}

do_stop_srv()
{
	# S_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:$S_MODULE:$S_SESSION_ID


	if [ "$NO_SOIP" = 'n' ]; then
		stop_service
	fi
}

handle_se_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$S_SESSION_ID
	local _t="$1"

	if [ "$S_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $S_SESSION_ID"
		return
	fi
	to_s_start_srv $S_SESSION_ID $S_ENCODER_IP $S_MULTICAST_IP
	# return as S_STATE='s_start_srv'
	to_s_srv_on
	# return as S_STATE='s_srv_on'
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

handle_se_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$S_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$S_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $S_SESSION_ID"
		return
	fi

	S_ENCODER_IP="$_encoder_ip"
	S_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $S_SESSION_ID $S_ENCODER_IP $S_MULTICAST_IP
}

handle_se_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$S_SESSION_ID
	local _t="$1"

	if [ "$S_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $S_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $S_SESSION_ID $S_CH_SELECT
}

handle_se_start()
{
	SOIP_TYPE=`astparam g soip_type`
	if echo "$SOIP_TYPE" | grep -q "not defined" ; then
		SOIP_TYPE=`astparam r soip_type`
		if echo "$SOIP_TYPE" | grep -q "not defined" ; then
			# Check legacy soip_type2
			SOIP_TYPE2=`astparam g soip_type2`
			if echo "$SOIP_TYPE2" | grep -q "not defined" ; then
				SOIP_TYPE2=`astparam r soip_type2`
			fi
			# 1, 2 or 3. Default type 2.
			case "$SOIP_TYPE2" in
				y)
					SOIP_TYPE='2'
				;;
				n)
					SOIP_TYPE='1'
				;;
				*)
					SOIP_TYPE='2'
					SOIP_TYPE2='y'
				;;
			esac
		fi
	fi

	SOIP_GUEST_ON=`astparam g soip_guest_on`
	if echo "$SOIP_GUEST_ON" | grep -q "not defined" ; then
		SOIP_GUEST_ON=`astparam r soip_guest_on`
		if echo "$SOIP_GUEST_ON" | grep -q "not defined" ; then
			SOIP_GUEST_ON='y'
		fi
	fi

	echo "handle_se_start SOIP_GUEST_ON($SOIP_GUEST_ON) SOIP_TYPE($SOIP_TYPE)"

	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$S_STATE" != 's_idle' ]; then
		do_stop_srv $S_SESSION_ID
	fi

	update_session_id

	if [ "$SOIP_GUEST_ON" = 'y' ]; then
		start_service
		to_s_srv_on
	elif [ "$SOIP_TYPE" -eq 1 ]; then
		start_service
		to_s_srv_on
	else
		# get channel param
		S_CH_SELECT="$1"
		S_MULTICAST_IP='0.0.0.0'

		to_s_search $S_SESSION_ID $S_CH_SELECT
	fi
}

handle_se_stop()
{
	do_stop_srv $S_SESSION_ID
	update_session_id
	to_s_idle
}

handle_se_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
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

handle_se_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
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
		event=`ipc @s_lm_get g 2>>/ipc_fail.log`
		echo -e "\nssssssssssssssssssssssssssssssssssssssssss\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: sLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
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

	# start event loop
	event_loop &
}

start_slm
