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

update_session_id()
{
	V_SESSION_ID=$(( ($V_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$V_SESSION_ID" -lt 2 ]; then
		V_SESSION_ID='2'
	fi
}

start_videoip_remote()
{
	# TODO. avoid kill/re-load asthost. How? IPC?
	# Following asthost call will never return until killed.
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @v_ctrl q start:$V_SESSION_ID:uc:$V_CLIENT_IP:$V_TX_PORT:$MY_IP:$V_CTRL_PORT
	else
		ipc @v_ctrl q start:$V_SESSION_ID:mc:$V_MULTICAST_IP:$V_TX_PORT:$V_MULTICAST_IP:$V_CTRL_PORT
	fi
}

attach_client()
{
	if [ "$MULTICAST_ON" = 'n' ]; then
		ipc @v_ctrl q attach:$V_SESSION_ID:uc:$MY_IP:$V_CTRL_PORT
	else
		ipc @v_ctrl q attach:$V_SESSION_ID:mc:$V_MULTICAST_IP:$V_CTRL_PORT
	fi

	to_s_srv_on
}

# Copied from include.sh
stop_videoip_remote()
{
#	pkill -9 asthost 2>/dev/null

	ast_send_event -1 e_video_stop_working
	#disable remote will also disconnect all connected clients
	echo disable > "$VIDEO_SYS_PATH"/Remote
}

to_s_srv_on()
{
	V_STATE='s_srv_on'

	# TBD? for LED?
}

to_s_idle()
{
	V_STATE='s_idle'
}

to_s_stop_srv()
{
	V_STATE='s_stop_srv'

	# TODO.
	##if [ "$MULTICAST_ON" = 'y' ]; then
	##	pkill -9 igmp_daemon 2> /dev/null
	##	igmp_daemon.sh &
	##fi

	# stop heartbeat
	ipc @hb_ctrl s stop_acker:v:$V_SESSION_ID

	# Bruce170301. I see no need to execute in background.
	##stop_videoip_remote &
	stop_videoip_remote
}

to_s_start_srv()
{
	V_STATE='s_start_srv'

	start_videoip_remote

	# to_s_srv_on only when client attached
#	to_s_srv_on
	# return as "V_STATE='s_srv_on'"
}

handle_ve_attaching()
{
	# ve_attaching:session_id:client_ip
	# ==> $1: session_id
	#     $2: client_ip
	local _session_id="$1"
	V_CLIENT_IP="$2"

	# TODO:
	# Both unicast and multicast only reply one client.
	# Under multicast mode, another ve_attaching will be triggered when all
	# clients leaves and a new one comes in.
	#

	if [ "$V_SESSION_ID" != "$_session_id" ]; then
		echo "Ignore. session ID doesn't match $_session_id vs $V_SESSION_ID"

		ipc @v_lm_reply s ignore
		return
	fi

	case "$V_STATE" in
		s_pre_start_srv)
			to_s_start_srv $V_SESSION_ID $V_CH_SELECT $V_MULTICAST_IP $V_CLIENT_IP
			# return as "V_STATE='s_srv_on'"
		;& # fall-through
		s_start_srv)
			attach_client # start already just do procedure for attach

			# ve_attaching is triggered by hb through 'query'
			# query/reply must be called in pair and better in the same event handler.
			ipc @v_lm_reply s ok
		;;
		*)
			ipc @v_lm_reply s ignore
		;;
	esac
}

to_s_pre_start_srv()
{
	# Start HB and wait for client.
	V_STATE='s_pre_start_srv'

	# FIXME. all service uses the same CH_SELECT in encoder side. MULTICAST_IP
	# is some how generated according to CH_SELECT. So, I see no need to refresh
	# CH_SELECT here.
	## Update essential parameters.
	##node_query --if essential --of essential --set_key CH_SELECT=$V_CH_SELECT

	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_acker:type:session_id:uc:acker_ip:acker_target_and_listen_port
		ipc @hb_ctrl s start_acker:v:$V_SESSION_ID:uc:$MY_IP:$V_HB_PORT
	else
		# TBD: HB's multicast ip and port number.
		# start_acker:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_acker:v:$V_SESSION_ID:mc:$V_MULTICAST_IP:$V_HB_PORT

		# be the same as A6,
		# start streaming even if no client in multicast case
		# this can be disabled by videoip driver option, DRVO_H_MULTICAST_STOP_TX_NO_CLIENT
		to_s_start_srv
	fi
}

_video_quality_set()
{
	# Range from -1 to 3. -1 means auto
	echo "video quality mode set to $1"
	echo "$1" > "$VIDEO_SYS_PATH"/QualityMode
}

_v_osd_set_to()
{
	# $1: OSD TYPE STRING
	# $2: Time out
	ipc @hb_ctrl s msg:v:pinger:255.255.255.255:ve_osd_set_to:"$1":"$2"
}

handle_ve_toggle_video_profile()
{
	# Syntax:
	# ve_toggle_video_profile[:mode]
	#

	# Bruce100525. A1 change button define.
	if [ "$SOC_OP_MODE" -eq '1' ]; then
		_t=`cat $VIDEO_SYS_PATH/rx`
		if [ "$_t" = "analog" ]; then
			echo "Analog input doesn't support quality mode change"
			return
		fi
	fi

	if [ "$NO_VIDEO" = 'y' ] || [ "$V_STATE" != 's_srv_on' ]; then
		echo "Unexpected state! Ignore."
		return
	fi

	if [ $# -ge 1 ]; then
		local _mode=$1
		case "$_mode" in
		'-1')
			# to switch to auto mode
			V_QUALITY_MODE='-1'
			_video_quality_set "$V_QUALITY_MODE"
			# set to client OSD
			_v_osd_set_to "VIDEO_VIDEO_MODE" 5
		;;
		'0')
			V_QUALITY_MODE='0'
			_video_quality_set "$V_QUALITY_MODE"
			# set to client OSD
			_v_osd_set_to "VIDEO_GRAPHIC_MODE" 5
		;;
		[1-5])
			_video_quality_set "$_mode"
		;;
		*)
			echo "Unexpected mode($_mode)! Ignore."
		;;
		esac
		ast_send_event -1 e_vlm_notify_quality_mode_chg
		return
	fi

	# To switch between Text mode and Auto mode
	if cat $VIDEO_SYS_PATH/QualityMode | grep "Quality Mode set" | grep -q "\-1"; then
		# to switch to text mode
		V_QUALITY_MODE='0'
		_video_quality_set "$V_QUALITY_MODE"
		# set to client OSD
		_v_osd_set_to "VIDEO_GRAPHIC_MODE" 5
	else
		# to switch to auto mode
		V_QUALITY_MODE='-1'
		_video_quality_set "$V_QUALITY_MODE"
		# set to client OSD
		_v_osd_set_to "VIDEO_VIDEO_MODE" 5
	fi
	ast_send_event -1 e_vlm_notify_quality_mode_chg
}

handle_ve_toggle_video_anti_dither()
{
	local _t=`cat $VIDEO_SYS_PATH/rx`
	if [ "$_t" = "analog" ]; then
		echo "Analog input don't care dithering."
		return
	fi

	if [ "$NO_VIDEO" = 'y' ] || [ "$V_STATE" != 's_srv_on' ]; then
		echo "Unexpected state! Ignore."
		return
	fi

	if [ $# -ge 1 ]; then
		local _mode=$1
		case "$_mode" in
		'0')
			# to disable anti-dithering
			V_BCD_THRESHOLD='0'
			echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
			_v_osd_set_to "VIDEO_ANTIDITHER_OFF" 5
		;;
		'1')
			# to enable anti-dithering 1
			V_BCD_THRESHOLD='1'
			echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
			_v_osd_set_to "VIDEO_ANTIDITHER_1" 5
		;;
		'2')
			# to enable anti-dithering 2
			V_BCD_THRESHOLD='2'
			echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
			_v_osd_set_to "VIDEO_ANTIDITHER_2" 5
		;;
		*)
			echo "Unexpected mode($_mode)! Ignore."
		;;
		esac
		ast_send_event -1 e_vlm_notify_anti_dither_mode_chg
		return
	fi

	local thresh=`cat $VIDEO_SYS_PATH/bcd_threshold`
	if [ "$thresh" = '0' ]; then
		# to enable anti-dithering 1
		V_BCD_THRESHOLD='1'
		echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
		_v_osd_set_to "VIDEO_ANTIDITHER_1" 5
	elif [ "$thresh" = '1' ]; then
		# to enable anti-dithering 2
		V_BCD_THRESHOLD='2'
		echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
		_v_osd_set_to "VIDEO_ANTIDITHER_2" 5
	else
		# to disable anti-dithering
		V_BCD_THRESHOLD='0'
		echo $V_BCD_THRESHOLD > $VIDEO_SYS_PATH/bcd_threshold
		_v_osd_set_to "VIDEO_ANTIDITHER_OFF" 5
	fi
	ast_send_event -1 e_vlm_notify_anti_dither_mode_chg
}

handle_ve_toggle_snoop()
{
	local _t
	# on/off code path
	if [ $# -ge 1 ]; then
		local _mode=$1
		case "$_mode" in
		'n')
			echo "Snoop Disable"
			echo 0 > $VIDEO_SYS_PATH/LoopbackEnable
			echo 0 > $HDMIRX_SYS_PATH/LoopbackEnable
			V_LOOPBACK_ENABLED='n'
		;;
		'y')
			echo "Snoop Enable"
			echo 1 > $VIDEO_SYS_PATH/LoopbackEnable
			echo 1 > $HDMIRX_SYS_PATH/LoopbackEnable
			V_LOOPBACK_ENABLED='y'
		;;
		*)
			echo "Unexpected mode($_mode)! Ignore."
		;;
		esac
		ast_send_event -1 e_vlm_notify_snoop_chg
		return
	fi
	# toggle code path
	_t=`cat $VIDEO_SYS_PATH/LoopbackEnable`
	if [ "$_t" = '1' ]; then
		echo "Snoop Disable"
		echo 0 > $VIDEO_SYS_PATH/LoopbackEnable
		echo 0 > $HDMIRX_SYS_PATH/LoopbackEnable
		V_LOOPBACK_ENABLED='n'
	else
		echo "Snoop Enable"
		echo 1 > $VIDEO_SYS_PATH/LoopbackEnable
		echo 1 > $HDMIRX_SYS_PATH/LoopbackEnable
		V_LOOPBACK_ENABLED='y'
	fi
	ast_send_event -1 e_vlm_notify_snoop_chg
}

handle_ve_post_config()
{
	if [ "$NO_VIDEO" = 'n' ]; then
		# Bruce100525. A1 change button behavior
		# Only apply under multicast mode.
		# if [ "$MULTICAST_ON" = 'n' ]; then
		# 	return
		# fi

		_video_quality_set "$V_QUALITY_MODE"

		local _t=`cat $VIDEO_SYS_PATH/rx`
		if [ "$_t" != "analog" ]; then
			# Set bcd_threshold
			echo "$V_BCD_THRESHOLD" > $VIDEO_SYS_PATH/bcd_threshold
		fi
	fi
}

handle_ve_start_loopback()
{
	echo "To loopback mode"
	if [ "$AST_PLATFORM" = "ast1500hv1" ]; then
		# do nothing
		echo ""
	else
		#insmod videoip.ko
		echo $EDID_USE > $VIDEO_SYS_PATH/edid_use
		handle_ve_toggle_snoop $V_LOOPBACK_ENABLED
	fi
}

handle_ve_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	# stop service first if necessary.
	handle_ve_stop
	# return as V_STATE='s_idle'

	#update_session_id updated in handle_ve_stop()

	# get channel param
	V_CH_SELECT="$1"
	V_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX v $V_CH_SELECT`

	# TODO.
	##if eth_link_is_off ; then
	##	echo "Network link is down"
	##	return 0
	##fi
	to_s_pre_start_srv $V_SESSION_ID $V_CH_SELECT $V_MULTICAST_IP
	# return as "V_STATE='s_pre_start_srv'"
}

handle_ve_stop()
{
	case "$V_STATE" in
		s_idle)
			echo "Stopped. Do nothing."
		;;
		*)
			to_s_stop_srv
			update_session_id
			to_s_idle
			# return as V_STATE='s_idle'
		;;
	esac
}

handle_ve_no_heartbeat()
{
	#Parse ve_no_heartbeat:$V_SESSION_ID
	local _t="$1"

	if [ "$V_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $V_SESSION_ID"
		return
	fi
	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_ve_start	"$V_CH_SELECT"
}

handle_ve_lm_destroy()
{
	kill $PID_IPC_SVR
	exit 0
}

handle_ve_test()
{
	return
## Moved to LM
#	update_session_id
#	MAC='02C7C324D7E3'
#	MY_IP='169.254.10.133'
#	CH_SELECT='0011'
#	# multi key pair set for 'essential' reply_type
#	node_query --if essential --of essential --set_key MY_MAC=$MAC
#	node_query --if essential --of essential --set_key MY_IP=$MY_IP
#	node_query --if essential --of essential --set_key IS_HOST=y
#	node_query --if essential --of essential --set_key HOSTNAME="ast3-gateway0011"
#	node_query --if essential --of essential --set_key MULTICAST_ON=y
#	node_query --if essential --of essential --set_key JUMBO_MTU=8000
#	node_query --if essential --of essential --set_key CH_SELECT=$CH_SELECT
#
#	# start node_responser
#	node_responser --mac $MAC &
#
#	heartbeat &
}

handle_ve_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_v_lmh.sh ]; then
		source ./patch_v_lmh.sh
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

# Copied from include.sh
start_videoip_h()
{
	if [ "$HDCP_ALWAYS_ON_22" = 'y' ]; then
		HDCP_MODE=$(( $HDCP_MODE | 0x100000 ))
	elif [ "$HDCP_ALWAYS_ON" = 'y' ]; then
		HDCP_MODE=$(( $HDCP_MODE | 0x40000 ))
	fi

	rmmod videoip 2>/dev/null
	insmod videoip.ko init_hdcp_mode=$HDCP_MODE hdcp2_version=$HDCP2_VERSION drv_option=$V_ENG_DRV_OPTION analog_edge_detect=$V_ANALOG_EDGE_DETECT
	mknod /dev/videoip c 250 0
	echo "$LOOPBACK_EDID_PATCH" > $VIDEO_SYS_PATH/loopback_edid_patch
	echo "$REMOTE_EDID_PATCH" > $VIDEO_SYS_PATH/remote_edid_patch
	# Config edid_use here so that edid_use value will always be as expected even when boot up with Ethernet link OFF.
	if [ "$MULTICAST_ON" = 'n' ]; then
		# Unicast mode always use remote EDID as primary.
		echo 'secondary' > $VIDEO_SYS_PATH/edid_use
	else
		# Init edid_use per $EDID_USE under multicast mode.
		echo $EDID_USE > $VIDEO_SYS_PATH/edid_use
	fi
	if [ "$LOOPBACK_DEFAULT_ON" = 'y' ]; then
		echo 1 > $VIDEO_SYS_PATH/LoopbackEnable
	else
		echo 0 > $VIDEO_SYS_PATH/LoopbackEnable
	fi
	echo "$V_1080I_TO_720P" > $VIDEO_SYS_PATH/force_1080i_to_720p
	echo "$V_CHUNK_SIZE" > $VIDEO_SYS_PATH/chunk_size
	echo "$V_FRAME_RATE" > $VIDEO_SYS_PATH/frame_rate_control
	echo "$V_DBG_MODE" > $VIDEO_SYS_PATH/videoip_debug

#	if [ "$DEFAULT_KSV" != 'unavailable' ]; then
#		echo "$DEFAULT_KSV" > $VIDEO_SYS_PATH/default_ksv
#	fi
	if [ "$HDCP2_GLOBAL_CONSTANT" != 'unavailable' ]; then
		echo "$HDCP2_GLOBAL_CONSTANT" > $VIDEO_SYS_PATH/hdcp2_global_constant
	fi
	if [ "$HDCP2_DCP_PUBLIC_KEY" != 'unavailable' ]; then
		echo "$HDCP2_DCP_PUBLIC_KEY" > $VIDEO_SYS_PATH/hdcp2_dcp_public_key
	fi

	cp -f ./V$SOC_OP_MODE/* . 2>/dev/null

	if [ "$EN_VIDEO_WALL" = 'y' ] && [ "$SOC_OP_MODE" = '1' ]; then
		# Only apply to host and SoC op mode == 1
		echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
		echo "$VW_VAR_MONINFO_HA $VW_VAR_MONINFO_HT $VW_VAR_MONINFO_VA $VW_VAR_MONINFO_VT" > "$VIDEO_SYS_PATH"/vw_frame_comp
		echo "${VW_VAR_MAX_ROW} ${VW_VAR_MAX_COLUMN} ${VW_VAR_ROW} ${VW_VAR_COLUMN}" > "$VIDEO_SYS_PATH"/vw_layout
	fi
}

# Copied from include.sh
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

# Copied from include.sh
load_sii9679_drv()
{
	# FIXME. This is actually platform dependent
	if [ "$SOC_VER" -lt '3' ]; then
		return
	fi


	if [ "$V_RX_DRV" != 'it6802' ]; then
		return
	fi

	insmod sii9679.ko
	local _major=`cat /proc/devices | awk "\\$2==\"sii9679\" {print \\$1}"`
	if [ -n "$_major" ]; then
		mknod /dev/sii9679 c $_major 0
	fi
}

v_lm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @v_lm_query" > /var/lock/@v_lm_query.lck
	echo "lock file for @v_ctrl" > /var/lock/@v_ctrl.lck

	ipc_server @v_lm_set @v_lm_get @v_lm_query @v_lm_reply &
	PID_IPC_SVR=$!

	#load_sii9678_drv

	# Copied from link_mgrh.sh::handle_e_sys_init_ok()
	# I2S also depends on cat6023.ko
	if ! { lsmod | grep -q "$V_RX_DRV"; }; then
		insmod $V_RX_DRV.ko drv_param=$V_RX_DRV_OPTION
		if [ "$LOOPBACK_DEFAULT_ON" = 'y' ]; then
			echo 1 > $HDMIRX_SYS_PATH/LoopbackEnable
		else
			echo 0 > $HDMIRX_SYS_PATH/LoopbackEnable
		fi
		#load_sii9679_drv
	fi

	start_videoip_h

	asthost &

	# TODO. How to implement a better approach to resolve the problem that
	#       igmp_daemon.sh is trying to resolve?
	#igmp_daemon.sh &
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
		event=`ipc @v_lm_get g 2>/dev/null`
		echo -e "\nvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n[$event]\n"
		event_loop_handler "$event"
	done
}

start_vlm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi
	if [ -f ./patch_v_lmh.sh ]; then
		source ./patch_v_lmh.sh
	fi

	v_lm_init

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	# start event loop
	event_loop &

	if [ "$NO_I2S" = 'y' ]; then
		case "$MODEL_NUMBER" in
			KDS-EN7)
			;;
			KDS-SW3-EN7)
				ipc @m_lm_set s open_report
				ipc @m_lm_set s get_link_status:0
				ipc @m_lm_set s get_link_status:1
				ipc @m_lm_set s get_link_status:2
			;;
			WP-SW2-EN7)
				ipc @m_lm_set s open_report
				ipc @m_lm_set s get_link_status:0
				ipc @m_lm_set s get_link_status:1
			;;
			*)
			;;
		esac
	fi

	if [ $P3KCFG_EDID_MODE = 'passthru' ];then
		handle_edid -c $P3KCFG_EDID_NET_SRC -t $rx_tcp_port -m 1
	elif [ $P3KCFG_EDID_MODE = 'custom' ];then
		handle_edid -s custom -i $P3KCFG_EDID_ACTIVE
	elif [ $P3KCFG_EDID_MODE = 'default' ];then
		handle_edid -s default
	fi
}

start_vlm
