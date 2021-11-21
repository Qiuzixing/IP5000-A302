#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.
A_MODULE=a
A_CONTROL=a_ctrl
A_IPC_SET=a_lm_set
A_IPC_GET=a_lm_get
A_IPC_QUERY=a_lm_query
A_IPC_REPLY=a_lm_reply
A_PATCH=patch_a_lmc.sh
A_VAR=/var/a_lm_var
A_DRV_NAME=1500_i2s
A_DRV_DEVICE=i2s
A_DRV_MODE=777
A_APP=client_i2s
#A_HB_PORT=59003
#A_TX_PORT=1235
#A_RX_PORT=1234
###############################################################################

. ip_mapping.sh

load_drv()
{
	insmod ./$A_DRV_NAME.ko $*

	rm -f /dev/$A_DRV_DEVICE

	major=`cat /proc/devices | awk "\\$2==\"$A_DRV_NAME\" {print \\$1}"`
	mknod /dev/$A_DRV_DEVICE c $major 0
}

init()
{
	# I2S_CLOCK_LOCK_MODE for SoC >= v2 client only.
	if [ "$SOC_VER" -ge 2 ]; then
		echo $I2S_CLOCK_LOCK_MODE > $I2S_SYS_PATH/clock_lock
	fi

	# Only SoC >= V3 support client audio hybrid
	if [ "$SOC_OP_MODE" -ge 3 ]; then
		echo $A_IO_SELECT > $I2S_SYS_PATH/io_select
		echo $A_OUTPUT_DELAY > $I2S_SYS_PATH/tx_delay_ms
	fi
	echo $A_ANALOG_IN_VOL > $I2S_SYS_PATH/analog_in_vol
	echo $A_ANALOG_OUT_VOL > $I2S_SYS_PATH/analog_out_vol
}

start_service()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @$A_CONTROL q start:$A_SESSION_ID:uc:$A_ENCODER_IP:$A_TX_PORT:$MY_IP:$A_TX_PORT
	else
		ipc @$A_CONTROL q start:$A_SESSION_ID:mc:$A_MULTICAST_IP:$A_TX_PORT:$A_MULTICAST_IP:$A_TX_PORT
	fi
}

stop_service()
{
	# TODO use IPC instead
	echo 1 > $I2S_SYS_PATH/stop
	return
}

###############################################################################
update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	A_SESSION_ID=$(( ($A_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$A_SESSION_ID" -lt 2 ]; then
		A_SESSION_ID='2'
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
	local _get_key='NO_I2S'
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
		NO_I2S=*)
			_is_disabled=${_info#NO_I2S=}
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
					echo "ERROR! Tx's NO_I2S=$_is_disabled" >&2
				;;
				*)
					echo "Disabled"
					echo "ERROR! Tx's NO_I2S=$_is_disabled" >&2
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
		NO_I2S=*)
			_PEER_NO_I2S=${_info#NO_I2S=}
			#echo "_PEER_NO_I2S=$_PEER_NO_I2S" >&2
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
	local _A_SESSION_ID="$1"
	local _A_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_A_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_I2S=$NO_I2S

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @$A_IPC_SET s "ae_no_encoder_ip:$_A_SESSION_ID"
		;;
		*)
			# Found
			if [ "$_PEER_NO_I2S" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @$A_IPC_SET s ae_stop
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX a $_PEER_CH_SELECT`
				ipc @$A_IPC_SET s "ae_encoder_ip_got:$_A_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	A_STATE='s_search'
	do_search_encoder $A_SESSION_ID $A_CH_SELECT &
}

to_s_start_hb()
{
	A_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:$A_MODULE:$A_SESSION_ID:uc:$A_ENCODER_IP:$A_HB_PORT
	else
		ipc @hb_ctrl s start_pinger:$A_MODULE:$A_SESSION_ID:mc:$A_MULTICAST_IP:$A_HB_PORT
	fi
}

to_s_srv_on()
{
	A_STATE='s_srv_on'
}

to_s_start_srv()
{
	A_STATE='s_start_srv'

	if [ "$NO_I2S" = 'n' ]; then
		start_service
	fi
}

to_s_idle()
{
	A_STATE='s_idle'
}

do_stop_srv()
{
	# A_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:$A_MODULE:$A_SESSION_ID


	if [ "$NO_I2S" = 'n' ]; then
		stop_service
	fi
}

handle_ae_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$A_SESSION_ID
	local _t="$1"

	if [ "$A_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $A_SESSION_ID"
		return
	fi
	to_s_start_srv $A_SESSION_ID $A_ENCODER_IP $A_MULTICAST_IP
	# return as A_STATE='s_start_srv'
	to_s_srv_on
	# return as A_STATE='s_srv_on'
}

handle_ae_no_heartbeat()
{
	#Parse ue_no_heartbeat:$A_SESSION_ID
	local _t="$1"

	if [ "$A_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $A_SESSION_ID"
		return
	fi

	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_ae_start	"$A_CH_SELECT"
}

handle_ae_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$A_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$A_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $A_SESSION_ID"
		return
	fi

	A_ENCODER_IP="$_encoder_ip"
	A_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $A_SESSION_ID $A_ENCODER_IP $A_MULTICAST_IP
}

handle_ae_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$A_SESSION_ID
	local _t="$1"

	if [ "$A_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $A_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $A_SESSION_ID $A_CH_SELECT
}

handle_ae_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$A_STATE" != 's_idle' ]; then
		do_stop_srv $A_SESSION_ID
	fi

	update_session_id

	# get channel param
	A_CH_SELECT="$1"
	A_MULTICAST_IP='0.0.0.0'

	to_s_search $A_SESSION_ID $A_CH_SELECT
}

handle_ae_stop()
{
	do_stop_srv $A_SESSION_ID
	update_session_id
	to_s_idle
}

handle_ae_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
}

handle_ae_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./$A_PATCH ]; then
		source ./$A_PATCH
	fi
}

handle_ae_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @a_lm_query q ae_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @$A_IPC_REPLY s "$_value"
}

handle_ae_param_set()
{
	# ipc @a_lm_set s ae_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}


handle_ae_level()
{
	#Parse ae_level:$LEVEL
	local _level="$1"

	echo $_level > /sys/devices/platform/1500_i2s/analog_out_vol
	echo "ae_level!!! $_level"
}

handle_ae_dir()
{
	#Parse ae_dir:$DIR
	local _dir="$1"

	#sconfig --audio-analog "$_dir"
	echo "ae_dir!!! $_dir"
}

handle_ae_mute()
{
	local _para1=$1
	echo "handle_ae_mute.($_para1)"

	if [ $_para1 != '1' ]; then
	    echo 1 > /sys/class/leds/lineout_mute/brightness
	else
	    echo 0 > /sys/class/leds/lineout_mute/brightness
	fi
}

handle_ae_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > $A_VAR
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < $A_VAR |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> $A_VAR
		}
	) &
	wait $!
}

handle_ae_param_dump()
{
	handle_ae_var
	local _dump=`cat $A_VAR`
	ipc @$A_IPC_REPLY s "$_dump"
}

handle_ae_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
}

a_lm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @$A_IPC_QUERY" > /var/lock/@$A_IPC_QUERY.lck
	echo "lock file for @$A_CONTROL" > /var/lock/@$A_CONTROL.lck

	ipc_server @$A_IPC_SET @$A_IPC_GET @$A_IPC_QUERY @$A_IPC_REPLY &
	PID_IPC_SVR=$!

	load_drv

	init

	$A_APP &

	to_s_idle
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! aLM received signal!!!!!! Ignore it."
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
		ae_?*)
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

	# To ignore previous pending events, we init A_SESSION_ID at the beginning of event loop.
	A_SESSION_ID="$RANDOM"

	update_session_id

	while true; do
		event=`ipc @a_lm_get g 2>>/ipc_fail.log`
		echo -e "\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: aLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
}

start_alm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi

	handle_ae_patch

	a_lm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &

	if [ $P3KCFG_AV_MUTE = 'off' ];then
		echo 100 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 1 > /sys/class/leds/linein_mute/brightness
		echo 1 > /sys/class/leds/lineout_mute/brightness
		ipc @m_lm_set s set_hdmi_mute:16:1:0
	else
		echo 0 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 0 > /sys/class/leds/linein_mute/brightness
		echo 0 > /sys/class/leds/lineout_mute/brightness
		ipc @m_lm_set s set_hdmi_mute:16:1:1
	fi
}

start_alm
