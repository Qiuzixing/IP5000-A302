#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.


#V_HB_PORT=59002

. ip_mapping.sh

# Construct scriptlet
######################################################
# link_off_timer.sh $LM_LINK_OFF_TIMEOUT
echo "#!/bin/sh

sleep \"\$1\"
ast_send_event -1 e_link_off_time_up
" > /usr/local/bin/link_off_timer.sh
chmod a+x /usr/local/bin/link_off_timer.sh

start_link_off_timer()
{
	link_off_timer.sh "$1" &
}
stop_link_off_timer()
{
	pkill link_off_timer 2>/dev/null
}
######################################################

update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	V_SESSION_ID=$(( ($V_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$V_SESSION_ID" -lt 2 ]; then
		V_SESSION_ID='2'
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
	local _get_key='NO_VIDEO'
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
		NO_VIDEO=*)
			_is_disabled=${_info#NO_VIDEO=}
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
					echo "ERROR! Tx's NO_VIDEO=$_is_disabled" >&2
				;;
				*)
					echo "Disabled"
					echo "ERROR! Tx's NO_VIDEO=$_is_disabled" >&2
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
		NO_VIDEO=*)
			_PEER_NO_VIDEO=${_info#NO_VIDEO=}
			#echo "_PEER_NO_VIDEO=$_PEER_NO_VIDEO" >&2
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
	local _V_SESSION_ID="$1"
	local _V_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_V_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_VIDEO=$NO_VIDEO

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @v_lm_set s "ve_no_encoder_ip:$_V_SESSION_ID"
			# Refresh GUI V_ENCODER_IP
			ast_send_event -1 e_encoder_ip_got
		;;
		*)
			# Found
			if [ "$_PEER_NO_VIDEO" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @v_lm_set s ve_stop
				# Refresh GUI V_ENCODER_IP
				ast_send_event -1 e_encoder_ip_got
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX v $_PEER_CH_SELECT`
				ipc @v_lm_set s "ve_encoder_ip_got:$_V_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP"
			fi
		;;
	esac
}

to_s_search()
{
	V_STATE='s_search'
	V_ENCODER_IP="Unknown"
	# Bruce170508. Move into do_search_encoder(). Try to optimize v reconnect flow.
	# Refresh GUI V_ENCODER_IP
	#ast_send_event -1 e_encoder_ip_got
	do_search_encoder $V_SESSION_ID $V_CH_SELECT &
}

to_s_start_hb()
{
	V_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:v:$V_SESSION_ID:uc:$V_ENCODER_IP:$V_HB_PORT
	else
		# start_pinger:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:v:$V_SESSION_ID:mc:$V_MULTICAST_IP:$V_HB_PORT
	fi
}

to_s_srv_on()
{
	V_STATE='s_srv_on'
}

to_s_start_srv()
{
	V_STATE='s_start_srv'
	# Copied from include.sh::load_videoip_c()
	# astclient should wait for the videoip.ko initialized
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @v_ctrl q start:$V_SESSION_ID:uc:$V_ENCODER_IP:$V_CTRL_PORT:$MY_IP:$V_TX_PORT
	else
		ipc @v_ctrl q start:$V_SESSION_ID:mc:$V_MULTICAST_IP:$V_CTRL_PORT:$V_MULTICAST_IP:$V_TX_PORT
	fi
}

to_s_idle()
{
	V_STATE='s_idle'
}

do_stop_srv()
{
	# V_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:v:$V_SESSION_ID

	# Bruce151225. We call unload_videoip_c in major thread instead background.
	# There is a bug which somehow causing VIDEOIP_RDY never be 'n'.
	# This bug is difficult to duplicate. It occurs when doing e_reconnect stress test.
	# To avoid this issue, we call unload_videoip_c() in forground so that
	# e_videoip_stopped event is no more necessary.
	echo 1 > $VIDEO_SYS_PATH/unlink
}

handle_ve_heartbeat_init_ok()
{
	#Parse ve_heartbeat_init_ok:$V_SESSION_ID
	local _t="$1"

	if [ "$V_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $V_SESSION_ID"
		return
	fi
	stop_link_off_timer
	to_s_start_srv $V_SESSION_ID $V_ENCODER_IP $V_MULTICAST_IP
	# return as V_STATE='s_start_srv'
	to_s_srv_on
	# return as V_STATE='s_srv_on'
}

handle_ve_no_heartbeat()
{
	#Parse ve_no_heartbeat:$V_SESSION_ID
	local _t="$1"

	if [ "$V_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $V_SESSION_ID"
		return
	fi
	case "$V_STATE" in
		s_start_hb)
			# can't connect to found encoder.
			local _tt='dummy'
		;;
		s_srv_on)
			# heartbeat lost!
			start_link_off_timer $LM_LINK_OFF_TIMEOUT
		;;
		*)
			echo "Wrong state?! V_STATE=$V_STATE"
		;;
	esac
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	do_restart	"$V_CH_SELECT"
}

handle_ve_encoder_ip_got()
{
	#Parse ve_encoder_ip_got:$V_SESSION_ID:$_encoder_ip:$_multicast_ip
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$V_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $V_SESSION_ID"
		return
	fi

	V_ENCODER_IP="$_encoder_ip"
	V_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $V_SESSION_ID $V_ENCODER_IP $V_MULTICAST_IP
	# Refresh GUI V_ENCODER_IP
	ast_send_event -1 e_encoder_ip_got
}

handle_ve_no_encoder_ip()
{
	#Parse ve_no_encoder_ip:$V_SESSION_ID
	local _t="$1"

	if [ "$V_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $V_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $V_SESSION_ID $V_CH_SELECT
}

do_restart()
{
	if [ "$V_STATE" != 's_idle' ]; then
		do_stop_srv $V_SESSION_ID
	fi

	update_session_id

	# get channel param
	V_CH_SELECT="$1"
	V_MULTICAST_IP='0.0.0.0'

	to_s_search $V_SESSION_ID $V_CH_SELECT
}

handle_ve_toggle_video_profile()
{
	ipc @hb_ctrl s msg:v:acker:0.0.0.0:ve_toggle_video_profile
}

handle_ve_toggle_video_anti_dither()
{
	ipc @hb_ctrl s msg:v:acker:0.0.0.0:ve_toggle_video_anti_dither
}

handle_ve_osd_set_to()
{
	# $1: OSD type string
	# $2: Time out
	ast_send_event -1 e_osd_set_to::"$1"::"$2"
}

handle_ve_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	start_link_off_timer $LM_LINK_OFF_TIMEOUT
	do_restart $*
}

handle_ve_stop()
{
	stop_link_off_timer

	do_stop_srv $V_SESSION_ID
	update_session_id
	to_s_idle
}

handle_ve_lm_destroy()
{
	kill $PID_IPC_SVR
	rmmod videoip
	exit 0
}

handle_ve_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_v_lmc.sh ]; then
		source ./patch_v_lmc.sh
	fi
}

handle_ve_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @v_lm_query q ve_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @v_lm_reply s "$_value"
}

handle_ve_param_set()
{
	# ipc @v_lm_set s ve_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	local _value=$2

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_ve_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > /var/v_lm_var
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < /var/v_lm_var |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> /var/v_lm_var
		}
	) &
	wait $!
}

handle_ve_param_dump()
{
	handle_ve_var
	local _dump=`cat /var/v_lm_var`
	ipc @v_lm_reply s "$_dump"
}

handle_ve_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
}

handle_ve_rgb()
{
	local _para1=$1
	echo "handle_ve_rgb.($_para1)" 
	
	astparam s v_hdmi_force_rgb_output $_para1
	astparam save
}

handle_ve_scale()
{
	local _para1=$1
	echo "handle_ve_scale.($_para1)"

	case "$_para1" in
		pass)
			astparam s v_output_timing_convert 0
			echo 0 > /sys/devices/platform/videoip/output_timing_convert
		;;
		2160p30)
			astparam s v_output_timing_convert 8000005F
			echo 8000005F > /sys/devices/platform/videoip/output_timing_convert
		;;
		2160p25)
			astparam s v_output_timing_convert 8000005E
			echo 8000005E > /sys/devices/platform/videoip/output_timing_convert
		;;
		1080p60)
			astparam s v_output_timing_convert 80000010
			echo 80000010 > /sys/devices/platform/videoip/output_timing_convert
		;;
		1080p50)
			astparam s v_output_timing_convert 8000001F
			echo 8000001F > /sys/devices/platform/videoip/output_timing_convert
		;;
		720p60)
			astparam s v_output_timing_convert 80000004
			echo 80000004 > /sys/devices/platform/videoip/output_timing_convert
		;;
		*)
			echo "ERROR: Unknown param ($1)!?"
		;;
	esac
	
	astparam save
}

# TODO. Need an include.sh for sub-LMs.
load_sii9678_drv()
{
	# FIXME. This is actually platform dependent
	if [ "$SOC_VER" -lt '3' ]; then
		return
	fi

	# SiI9678 driver is built-in together with CRT driver. So, only make node here.
	local _major=`cat /proc/devices | awk "\\$2==\"sii9678\" {print \\$1}"`
	if [ -n "$_major" ]; then
		mknod /dev/sii9678 c $_major 0
	fi
}

v_lm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @v_lm_query" > /var/lock/@v_lm_query.lck
	echo "lock file for @v_ctrl" > /var/lock/@v_ctrl.lck

	ipc_server @v_lm_set @v_lm_get @v_lm_query @v_lm_reply &
	PID_IPC_SVR=$!

	case "$MODEL_NUMBER" in
		WP-DEC7)
			load_sii9678_drv
		;;
		*)
		;;
	esac

	#load_videoip_driver_c
	if [ "$HDCP_ALWAYS_ON_22" = 'y' ]; then
		HDCP_MODE=$(( $HDCP_MODE | 0x100000 ))
	elif [ "$HDCP_ALWAYS_ON" = 'y' ]; then
		HDCP_MODE=$(( $HDCP_MODE | 0x40000 ))
	fi

	insmod videoip.ko init_hdcp_mode=$HDCP_MODE hdcp2_version=$HDCP2_VERSION drv_option=$V_ENG_DRV_OPTION hdmi_force_rgb_out=$V_HDMI_FORCE_RGB_OUTPUT

	if [ -f "$VIDEO_SYS_PATH/gen_lock_cfg" ]; then
		echo $V_GEN_LOCK_CFG > $VIDEO_SYS_PATH/gen_lock_cfg
	fi
	if [ "$HDCP2_GLOBAL_CONSTANT" != 'unavailable' ]; then
		echo "$HDCP2_GLOBAL_CONSTANT" > $VIDEO_SYS_PATH/hdcp2_global_constant
	fi
	if [ "$HDCP2_RX_ID" != 'unavailable' ]; then
		echo "$HDCP2_RX_ID" > $VIDEO_SYS_PATH/receiver_id
	fi
#			if [ "$HDCP2_RX_CERT" != 'unavailable' ]; then
#				echo "$HDCP2_RX_CERT" > $VIDEO_SYS_PATH/hdcp2_rx_cert
#			fi
	if [ "$HDCP2_RX_CERT_DATA" != 'unavailable' ]; then
		echo "$HDCP2_RX_CERT_DATA" > $VIDEO_SYS_PATH/hdcp2_rx_cert_data
	fi
	if [ "$HDCP2_RX_CERT_SIG" != 'unavailable' ]; then
		echo "$HDCP2_RX_CERT_SIG" > $VIDEO_SYS_PATH/hdcp2_rx_cert_sig
	fi
	if [ "$HDCP2_RX_PRIVATE_KEY_CRT" != 'unavailable' ]; then
		echo "$HDCP2_RX_PRIVATE_KEY_CRT" > $VIDEO_SYS_PATH/hdcp2_rx_private_key_crt
	fi
	if [ "$SOC_OP_MODE" -ge "3" ]; then
		echo "$V_OUTPUT_TIMING_CONVERT" > $VIDEO_SYS_PATH/output_timing_convert
		echo "$V_HDMI_HDR_MODE" > $VIDEO_SYS_PATH/hdmi_hdr_mode
	fi
	echo "$EDID_USE" > /sys/devices/platform/videoip/edid_use
	echo "$V_1080I_TO_720P" > $VIDEO_SYS_PATH/force_1080i_to_720p
	echo "$V_SRC_UNAVAILABLE_TIMEOUT" > $VIDEO_SYS_PATH/src_unavailable_timeout
	echo "$V_PWR_SAVE_TIMEOUT" > $VIDEO_SYS_PATH/pwr_save_timeout
	echo "$V_DBG_MODE" > $VIDEO_SYS_PATH/videoip_debug

	# Copy profiles
	cp -f ./V$SOC_OP_MODE/* . 2>/dev/null

	astclient &

	to_s_idle
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! vLM received signal!!!!!! Ignore it."
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
		ve_?*)
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

	# To ignore previous pending events, we init V_SESSION_ID at the beginning of event loop.
	V_SESSION_ID="$RANDOM"

	update_session_id
	# NOTE:
	# There can be ONLY one thread calling @v_lm_get and @v_lm_query/@v_lm_reply
	# There can be multiple thread calling @v_lm_set
	while true; do
		event=`ipc @v_lm_get g 2>>/ipc_fail.log`
		echo -e "\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: vLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
}

start_vlm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi
	if [ -f ./patch_v_lmc.sh ]; then
		source ./patch_v_lmc.sh
	fi

	v_lm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &
}

start_vlm
