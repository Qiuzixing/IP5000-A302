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
A_PATCH=patch_a_lmh.sh
A_VAR=/var/a_lm_var
A_DRV_NAME=1500_i2s
A_DRV_DEVICE=i2s
A_DRV_MODE=777
A_APP=host_i2s
#A_HB_PORT=59003
#A_TX_PORT=1234
#A_RX_PORT=1235

. ip_mapping.sh

load_drv()
{
	# I2S also depends on HDMI Rx driver
	if ! { lsmod | grep -q "$V_RX_DRV"; }; then
		insmod $V_RX_DRV.ko drv_param=$V_RX_DRV_OPTION
	fi

	insmod ./$A_DRV_NAME.ko $*

	rm -f /dev/$A_DRV_DEVICE

	major=`cat /proc/devices | awk "\\$2==\"$A_DRV_NAME\" {print \\$1}"`
	mknod /dev/$A_DRV_DEVICE c $major 0
}

init()
{
	echo $A_ANALOG_IN_VOL > $I2S_SYS_PATH/analog_in_vol
	echo $A_ANALOG_OUT_VOL > $I2S_SYS_PATH/analog_out_vol

	if [ "$SOC_OP_MODE" -ne 1 ]; then
		if [ "$EN_AES" = 'y' ]; then
			echo 1 > $I2S_SYS_PATH/enable_aes_encryption 2>/dev/null
		else
			echo 0 > $I2S_SYS_PATH/enable_aes_encryption 2>/dev/null
		fi
	fi
}

start_service()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @$A_CONTROL q start:$A_SESSION_ID:uc:$A_CLIENT_IP:$A_TX_PORT:$MY_IP:$A_TX_PORT
	else
		ipc @$A_CONTROL q start:$A_SESSION_ID:mc:$A_MULTICAST_IP:$A_TX_PORT:$A_MULTICAST_IP:$A_TX_PORT
	fi
}

stop_service()
{
	# TODO use IPC instead
	echo 1 > $I2S_SYS_PATH/stop
}
################################################################################
update_session_id()
{
	A_SESSION_ID=$(( ($A_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$A_SESSION_ID" -lt 2 ]; then
		A_SESSION_ID='2'
	fi
}

to_s_srv_on()
{
	A_STATE='s_srv_on'
}

to_s_idle()
{
	A_STATE='s_idle'
}

to_s_stop_srv()
{
	A_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:$A_MODULE:$A_SESSION_ID

	if [ $NO_I2S = 'n' ]; then
		stop_service
	fi
}

to_s_start_srv()
{
	A_STATE='s_start_srv'

	if [ "$NO_I2S" = 'n' ]; then
		start_service
	fi

	to_s_srv_on
	# return as "A_STATE='s_srv_on'"
}


to_s_pre_start_srv()
{
	# Start HB and wait for client.
	A_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. A_MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$u_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:$A_MODULE:$A_SESSION_ID:uc:$MY_IP:$A_HB_PORT
	else
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:$A_MODULE:$A_SESSION_ID:mc:$A_MULTICAST_IP:$A_HB_PORT
	fi
}

handle_ae_attaching()
{
	# ue_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	A_CLIENT_IP="$2"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ue_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$A_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $A_SESSION_ID"

		ipc @$A_IPC_REPLY s ignore
		return
	fi

	case "$A_STATE" in
		s_pre_start_srv)
			to_s_start_srv $A_SESSION_ID $A_CH_SELECT $A_MULTICAST_IP $A_CLIENT_IP
			# return as "A_STATE='s_srv_on'"

			# ue_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @$A_IPC_REPLY s ok
		;;
		*)
			ipc @$A_IPC_REPLY s ignore
		;;
	esac
}


handle_ae_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_ae_stop
	# return as A_STATE='s_idle'

	#update_session_id updated in handle_ue_stop()

	# get channel param
	A_CH_SELECT="$1"
	A_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX a $A_CH_SELECT`

	to_s_pre_start_srv $A_SESSION_ID $A_CH_SELECT $A_MULTICAST_IP
	# return as "A_STATE='s_pre_start_srv'"
}

handle_ae_stop()
{
	case "$A_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as A_STATE='s_idle'
		;;
	esac
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

handle_ae_lm_destroy()
{
	kill $PID_IPC_SVR

	# TODO

	exit 0
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

	sconfig --audio-analog "$_dir"
	echo "ae_dir!!! $_dir"
}

handle_ae_mute()
{
	local _para1="$1"
	echo "handle_ae_mute.($_para1)"
	#web_mute_slider_handle $_para1
}

handle_ae_test()
{
	return
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

	# NOTE:
	# There can be ONLY one thread calling @u_lm_get and @u_lm_query/@u_lm_reply
	# There can be multiple thread calling @u_lm_set
	while true; do
		event=`ipc @a_lm_get g 2>/dev/null`
		echo -e "\naaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n[$event]\n"
		event_loop_handler "$event"
	done
}

adc_pin_mux_gpio()
{
	#qzx20210722:they(ADC PIN) are invalid in leds driver,So need to set it manually
	reg_value=`io 0 0x1E6E20A0 | awk '{printf $3}'`
	reg_value=0x$reg_value
	bit_clear=0xf7ffffff
	bit_set=0x08000000
	reg_value=$((reg_value&bit_clear))
	reg_value=`printf "%x" $((reg_value|bit_set))`
	io 1 0x1E6E20A0 $reg_value
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
	else
		echo 0 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 0 > /sys/class/leds/linein_mute/brightness
		echo 0 > /sys/class/leds/lineout_mute/brightness
	fi

	if [ $UGP_FLAG = 'success' ];then
		ipc @m_lm_set s open_report
		#set lineio_sel pin to default to line_out;0:line_out;1:line_in
		case "$MODEL_NUMBER" in
			KDS-EN7)
				audioswitch &
				echo out_analog > /sys/devices/platform/1500_i2s/io_select
			;;
			KDS-SW3-EN7)
				audioswitch &
				ipc @m_lm_set s audio_out:0:1:2:3
				ipc @m_lm_set s get_link_status:0
				ipc @m_lm_set s get_link_status:1
				ipc @m_lm_set s get_link_status:2
			;;
			WP-SW2-EN7)
				ipc @m_lm_set s get_link_status:0
				ipc @m_lm_set s get_link_status:1
			;;
			*)
			;;
		esac
	fi

	if [ $UGP_FLAG = 'success' ];then
		case "$MODEL_NUMBER" in
			KDS-EN7)
				audio_detect &
				usleep 100
				echo 500 > /sys/class/leds/audio_detect/delay
				adc_pin_mux_gpio
			;;
			KDS-SW3-EN7)
				audio_detect &
				usleep 100
				echo 500 > /sys/class/leds/audio_detect/delay
				adc_pin_mux_gpio
			;;
			*)
			;;
		esac
	fi
}

start_alm
