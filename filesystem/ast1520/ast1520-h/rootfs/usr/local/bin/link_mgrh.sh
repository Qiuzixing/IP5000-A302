#!/bin/sh

##
## Copyright (c) 2017
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##

#config file path
device_setting="/data/configs/kds-7/device/device_setting.json"
edid_setting="/data/configs/kds-7/edid/edid_setting.json"
av_setting="/data/configs/kds-7/av_setting/av_setting.json"
gateway_setting="/data/configs/kds-7/gateway/gateway.json"
network_setting="/data/configs/kds-7/network/network_setting.json"
time_setting="/data/configs/kds-7/time/time_setting.json"
av_signal="/data/configs/kds-7/av_signal/av_signal.json"
audio_setting="/data/configs/kds-7/audio/audio_setting.json"
rx_tcp_port='8888'
analog_out_volum='87'
execute_once_flag='on'
stop_all_service()
{
	## A7 TBD
	echo "TBD"
}

# NOTICE: This function run on different process scope. (not in state_machine scope)
# Bruce130123. Now moved into state_machine scope.
do_s_init()
{
	echo "Memory Controller Setting:"
	echo "0x1e6e2040:"
	/usr/local/bin/io 0 0x1e6e2040
	echo "0x1e6e0068:"
	/usr/local/bin/io 0 0x1e6e0068

	case $SOC_VER in
		1)
			# If CPU frequency down to 266MHz, the board's memory is bad.
			if { /usr/local/bin/io 0 0x1e6e2024 | grep -iq "44120"; }; then
				echo "ERROR!!!!!! Board Memory Test FAIL!"
				if [ "$STOP_ON_BOOT_TEST_FAIL" = 'y' ]; then
					ast_send_event -1 e_kill
					return 1
				fi
			fi
		;;
		2)
			if ! boot_test_v2.sh; then
				if [ "$STOP_ON_BOOT_TEST_FAIL" = 'y' ]; then
					ast_send_event -1 e_kill
					return 1
				fi
			fi
		;;
		3)
			source calc_ring.sh
			echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
			echo "MCLK Ring"
			calc_oscclk mclk
			printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
			echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
			echo "DLY32 Ring"
			calc_oscclk dly32
			printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
			echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
		;;
		*)
			echo "" > /dev/null
		;;
	esac

	if [ "$EN_LOG" = 'n' ]; then
		/usr/local/bin/log.sh off
	fi
	# Indicate that system is stable.
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		led_off $LED_PWR
	else
		led_on $LED_PWR
	fi
	# 802.1X should start firstly before configure IP.
	handle_e_802_1x
	#ast_send_event "$EM_PID" "e_sys_init_ok"
	post_ip_addr
}

warn()
{
	echo "!!!!! $1 !!!!!"
	#to_s_error
}

to_s_error()
{
	echo "Last state=$STATE event=$event"
	STATE='s_error'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
	handle_e_kill
}

_do_start_srv()
{
	if [ "$NO_VIDEO" = 'n' ]; then
		ipc @v_lm_set s ve_start:$CH_SELECT_V
	fi
	if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
		USB_CLIENT_IP='0.0.0.0'
		ipc @u_lm_set s ue_start:$CH_SELECT_U
		if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
			led_on $LED_PWR
		fi
	fi
	if [ "$NO_I2S" = 'n' ]; then
		ipc @a_lm_set s ae_start:$CH_SELECT_A
	fi
	if [ "$NO_IR" = 'n' ]; then
		ipc @r_lm_set s re_start:$CH_SELECT_R
	fi
	if [ "$NO_SOIP" = 'n' ]; then
		ipc @s_lm_set s se_start:$CH_SELECT_S
	fi
	if [ "$NO_PWRBTN" = 'n' ]; then
		ipc @p_lm_set s pe_start:$CH_SELECT_P
	fi
	if [ "$NO_CEC" = 'n' -a "$CEC_GATWAY" = 'off' ]; then
		ipc @c_lm_set s ce_start:$CH_SELECT_C
	fi
}

do_start_srv()
{
	led_blink $LED_LINK

	_do_start_srv

	to_s_srv_on
}

to_s_srv_on()
{
	STATE='s_srv_on'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
	led_on $LED_LINK

	if [ $NO_VIDEO = 'n' ]; then
		# vLM may not load VE driver yet.
		if ! [ -f "${VIDEO_SYS_PATH}/State" ]; then
			to_s_attaching
		elif ! { cat ${VIDEO_SYS_PATH}/State | grep -q "OPERATING"; }; then
			to_s_attaching
		fi
	fi
}

to_s_attaching()
{
	STATE='s_attaching'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
	led_blink $LED_LINK

	if [ $NO_VIDEO = 'y' ]; then
		led_on $LED_LINK
	fi
}

do_stop_srv()
{
	led_blink $LED_LINK

	if [ "$NO_VIDEO" = 'n' ]; then
		ipc @v_lm_set s ve_stop
	fi
	if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
		ipc @u_lm_set s ue_stop
		if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
			led_off $LED_PWR
		fi
	fi
	if [ "$NO_I2S" = 'n' ]; then
		ipc @a_lm_set s ae_stop
	fi
	if [ "$NO_IR" = 'n' ]; then
		ipc @r_lm_set s re_stop
	fi
	if [ "$NO_SOIP" = 'n' ]; then
		ipc @s_lm_set s se_stop
	fi
	if [ "$NO_PWRBTN" = 'n' ]; then
		ipc @p_lm_set s pe_stop
	fi
	if [ "$NO_CEC" = 'n' ]; then
		ipc @c_lm_set s ce_stop
	fi

	do_ready_to_go
}

to_s_idle()
{
	STATE='s_idle'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE

	led_off $LED_LINK

	if [ $NO_VIDEO = 'n' ]; then
		ipc @v_lm_set s ve_start_loopback
	fi
}

do_ready_to_go()
{
	if eth_link_is_off ; then
		echo "Network link is down"
		to_s_idle
	elif [ "$ACCESS_ON" = 'n' ]; then
		echo "Press the link button to connect"
		to_s_idle
	else
		# for "$ACCESS_ON" == 'y'
		do_start_srv
	fi
}

handle_e_sys_init_ok()
{
	if [ "$STATE" = 's_init' ]; then
		# Init variables.
		USB_CLIENT_IP='0.0.0.0'
		V_LOOPBACK_ENABLED=$LOOPBACK_DEFAULT_ON

		# A7 disable watchdog after sys_init_ok
		#disable_watchdog
		handle_e_log
		if [ "$P3KCFG_NTP_SRV_MODE" = 'on' ]; then
			handle_e_p3k_ntp_on e_p3k_ntp_enable_on::$P3KCFG_NTP_SRV_ADDR::$P3KCFG_NTP_SYNC_HOUR
		fi
		# Export LM params to /var/lm. So that sub-LM can import.
		handle_e_var

		update_node_info
		# start node_responser
		node_responser --mac $MY_MAC &

		# Start heartbeat service
		heartbeat &

		if [ "$NO_VIDEO" = 'n' ]; then
			# display-related (crt) configuration for video
			set_display_config
			# Start vLM
			link_mgrh_video.sh
			# Start of apply_profile_config() related init.
			# Note, apply_profile_config() must be called after VE driver loaded.
			if [ "$SOC_VER" -ge '3' ]; then
				if [ "$V_DUAL_INPUT" = 'undefined' ]; then
					V_DUAL_INPUT=`cat $VIDEO_SYS_PATH/has_dual_input`
				fi
				# Note sysfs 'scu board_info' only available on SoC Ver >= 3
				case `cat  $SCU_SYS_PATH/board_info | grep "Video Type"` in
					*VGA*)
						V_TYPE='0'
					;;
					*Disable*)
						V_TYPE='1'
					;;
					*HDMI*)
						V_TYPE='2'
					;;
					*DVI*)
						V_TYPE='3'
					;;
					*)
						V_TYPE='2'
					;;
				esac
			fi
			apply_profile_config `select_v_input refresh`
		fi
		if [ "$NO_I2S" = 'n' ]; then
			# Start aLM
			link_mgrh_audio.sh
			select_audio_input `select_a_input`
		fi
		if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
			# Start uLM
			link_mgrh_usb.sh
		fi
		if [ "$NO_IR" = 'n' ]; then
			# Start rLM
			link_mgrh_ir.sh
		fi
		if [ "$NO_SOIP" = 'n' ]; then
			# Start sLM
			link_mgrh_serial.sh
		fi
		if [ "$NO_PWRBTN" = 'n' ]; then
			# Start pLM
			link_mgrh_pushbutton.sh
		fi
		if [ "$NO_CEC" = 'n' ]; then
			# Start pLM
			link_mgrh_cec.sh
		fi

		if [ "$P3KCFG_AV_ACTION" = 'play' ];then
			do_ready_to_go
		else
			to_s_idle
		fi

		set_igmp_leave_force
	fi

	#qzx 2022.1.11:When the power is on, turn on the black screen switch when the video is ready
	if [ "$UGP_FLAG" = 'success' -a "$execute_once_flag" = 'on' ];then
		case "$MODEL_NUMBER" in
			KDS-EN7)
				ipc @m_lm_set s set_video_control:17:1:0
				execute_once_flag='off'
			;;
			KDS-SW3-EN7)
			;;
			*)
			;;
		esac
	fi
}

handle_e_button_link_1()
{
	if [ "$IGNORE_E_BUTTON_LINK_1" = 'y' ]; then
		return
	fi

	if [ "$STATE" = 's_idle' ] && eth_link_is_off && [ "$BTN1_LONG_ON_ETH_OFF" != 'e_btn_ignore' ]; then
		handle_"$BTN1_LONG_ON_ETH_OFF"
		return
	fi

	handle_"$BTN1_LONG"
}

# when ip changed, maybe from autoip to dhcp or dhcp to autoip, something need be did
handle_e_sys_ip_chg()
{
	route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
	echo 2 > /proc/sys/net/ipv4/conf/eth0/force_igmp_version

	#avahi-daemon -k
	pkill -9 heartbeat
	pkill -9 node_responser
	pkill -9 name_service
	#pkill -9 httpd

	pkill -9 telnetd
	pkill -9 ldap_daemon

	#avahi-daemon -D
	name_service -thost
	#httpd -h /www &
	start_telnetd

	update_node_info

	node_responser --mac $MY_MAC &
	heartbeat &

	ulmparam s RELOAD_KMOIP 1
	ast_send_event -1 e_reconnect

	set_igmp_leave_force
}

_link_on_off()
{
	# Save the state into flash
	if [ "$ACCESS_ON" = 'y' ]; then
		ACCESS_ON='n'
		#astparam s astaccess n
	else
		ACCESS_ON='y'
		#astparam s astaccess y
	fi
	update_node_info ACCESS_ON $ACCESS_ON

	case "$STATE" in
		s_idle)
			do_ready_to_go
		;;
		s_srv_on | s_attaching)
			echo "Stop link STATE=$STATE"
			do_stop_srv
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac
}

handle_e_button_link()
{
	if [ "$IGNORE_E_BUTTON_LINK" = 'y' ]; then
		return
	fi

	if [ "$STATE" = 's_idle' ] && eth_link_is_off && [ "$BTN1_SHORT_ON_ETH_OFF" != 'e_btn_ignore' ]; then
		handle_"$BTN1_SHORT_ON_ETH_OFF"
		return
	fi

	handle_"$BTN1_SHORT"
}


handle_e_button_pairing()
{
	if [ "$STATE" = 's_idle' ] && eth_link_is_off && [ "$BTN2_SHORT_ON_ETH_OFF" != 'e_btn_ignore' ]; then
		handle_"$BTN2_SHORT_ON_ETH_OFF"
		return
	fi

	handle_"$BTN2_SHORT"
}

# Used to save the host's position layout for WebUI. Set once when performing basic setup.
handle_e_vw_pos_layout()
{
	# Parse e_vw_pos_layout_${_VW_VAR_POS_MAX_ROW}_${_VW_VAR_POS_MAX_COL}
	#_VW_VAR_POS_MAX_ROW=`expr "$1" : 'e_vw_pos_layout_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_POS_MAX_COL=`expr "$1" : 'e_vw_pos_layout_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 4;IFS="$_IFS"
	_VW_VAR_POS_MAX_ROW=$1
	_VW_VAR_POS_MAX_COL=$2

	if [ -z "$_VW_VAR_POS_MAX_ROW" ]; then
		_VW_VAR_POS_MAX_ROW='x'
	fi
	if [ -z "$_VW_VAR_POS_MAX_COL" ]; then
		_VW_VAR_POS_MAX_COL='x'
	fi

	if [ "$_VW_VAR_POS_MAX_ROW" = 'd' ]; then
		VW_VAR_POS_MAX_ROW="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_POS_MAX_ROW" != 'x' ]; then
		VW_VAR_POS_MAX_ROW="$_VW_VAR_POS_MAX_ROW"
	fi

	if [ "$_VW_VAR_POS_MAX_COL" = 'd' ]; then
		VW_VAR_POS_MAX_COL="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_POS_MAX_COL" != 'x' ]; then
		VW_VAR_POS_MAX_COL="$_VW_VAR_POS_MAX_COL"
	fi

	astparam s vw_pos_max_row "$VW_VAR_POS_MAX_ROW"
	astparam s vw_pos_max_col "$VW_VAR_POS_MAX_COL"
	astparam save
}

handle_e_button_pairing_1()
{
	if [ "$STATE" = 's_idle' ] && eth_link_is_off && [ "$BTN2_LONG_ON_ETH_OFF" != 'e_btn_ignore' ]; then
		handle_"$BTN2_LONG_ON_ETH_OFF"
		return
	fi

	handle_"$BTN2_LONG"
}


handle_e_kill()
{
	## A7 TBD.
	kill "$EM_PID"

	stop_all_service
	led_off $LED_LINK

	disable_watchdog
	to_mfg_mode

	exit 0;
}

_chg_hostname()
{
	#/sbin/avahi-daemon -k 2>/dev/null

	local _HOSTNAME_ID="$1"

	if [ -z "$HOSTNAME_CUSTOMIZED" ]; then
		HOSTNAME="${MODEL_NUMBER}-${_HOSTNAME_ID}"
	else
		HOSTNAME="$HOSTNAME_CUSTOMIZED"
	fi

	echo "HOSTNAME=$HOSTNAME"
	astsetname $HOSTNAME
	echo "${MODEL_NUMBER}-${_HOSTNAME_ID}" > /etc/device_name
	echo $HOSTNAME > /etc/hostname
	hostname -F /etc/hostname

	#/sbin/avahi-daemon -D 2>/dev/null

	update_node_info HOSTNAME $HOSTNAME
}

handle_e_chg_hostname()
{
	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	local _host_name=$1
	astparam s hostname_customized $_host_name
	astparam save
	#start avahi-daemon
	# The $HOSTNAME_ID is now decided in refresh_hostname_params()
	refresh_4bits_ch
	refresh_ch_params
	refresh_hostname_params

	if [ "$HOSTNAMEBYDIPSWITCH" = 'y' ]; then
		ast_send_event -1 e_reconnect::"$HOSTNAME_ID"
	else
		_chg_hostname "$HOSTNAME_ID"
	fi
}

handle_e_stop_link()
{
	ACCESS_ON='n'
	update_node_info ACCESS_ON $ACCESS_ON

	case "$STATE" in
		s_idle)
			echo "Already stopped"
		;;
		s_srv_on | s_attaching)
			echo "Stop link STATE=$STATE"
			do_stop_srv
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac
}

do_e_reconnect()
{
	ACCESS_ON='y'
	update_node_info ACCESS_ON $ACCESS_ON
	# Update CH_SELECT node_info on e_reconnect
	# Should update before stop link so that next time client node_query will get the new CH_SELECT value.
	update_node_info CH_SELECT $CH_SELECT

	case "$STATE" in
		s_idle)
			do_ready_to_go
		;;
		s_srv_on | s_attaching)
			do_stop_srv
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac
}

handle_e_reconnect_refresh()
{
	refresh_4bits_ch
	refresh_ch_params

	# Bruce171024. In case user is still using legacy A6 API,
	# we call refresh_hostname_params() to refresh CH_SELECT.
	refresh_hostname_params

	do_e_reconnect
}

handle_e_reconnect()
{
	#
	# e_reconnect::$ch_select
	#
	# $ch_select can be:
	#  0000~9999: channel format
	#

	case "$STATE" in
		s_idle | s_srv_on | s_attaching)
			# typical case
		;;
		*)
			# s_init
			warn "Wrong state?! $STATE"
			return
		;;
	esac

	# if there is no $who, it means legacy command without parameters.
	#
	if [ "$*" = 'e_reconnect' ]; then
		handle_e_reconnect_refresh
		return
	fi

	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	local _ch_select=$1

	_ch_select=`foolproof_ch_select $_ch_select`
	CH_SELECT=$_ch_select
	# Still need to set astparam here because handle_e_reconnect_refresh()::refresh_ch_params() need it.
	astparam s ch_select $CH_SELECT
	LED_DISPLAY_CH_SELECT_V=$CH_SELECT
	led_display_num
	# Optimize astparam access. Do save astparam if RESET_CH_ON_BOOT is y.
	if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
		astparam save
	fi
	CH_SELECT_V=$CH_SELECT
	CH_SELECT_U=$CH_SELECT
	CH_SELECT_A=$CH_SELECT
	CH_SELECT_S=$CH_SELECT
	CH_SELECT_R=$CH_SELECT
	CH_SELECT_P=$CH_SELECT
	CH_SELECT_C=$CH_SELECT

	do_e_reconnect
	# Host update hostname.
	if [ "$HOSTNAMEBYDIPSWITCH" = 'y' ]; then
		HOSTNAME_ID="$CH_SELECT"
		_chg_hostname "$HOSTNAME_ID"
	fi
}

handle_e_video_start_working()
{
	case "$STATE" in
		s_srv_on | s_start_srv)
			echo "Video start capture"
			# We stay in s_srv_on state, but stop blinking the LED_LINK
			#led_on $LED_LINK
		;;
		s_attaching)
			echo "Video start capture"
			# We change state to s_srv_on state, but stop blinking the LED_LINK
			to_s_srv_on
			if [ $MODEL_NUMBER = 'WP-SW2-EN7' ];then
				return
			fi
			a30_led_on $LINK_ON_G
		;;
		s_stop_srv)
			# Firmware >= A6.2.0, VE will restart immediately after suspended.
			echo "Video restarted"
		;;
		*)
			warn "Wrong state?! ($STATE)"
		;;
	esac
	ipc @v_lm_set s ve_post_config
}

handle_e_video_stop_working()
{
	case "$STATE" in
		s_srv_on)
			echo "Video stop capture"
			# We change to s_attaching state, and start blinking the LED_LINK
			to_s_attaching
		;;
		s_attaching)
			echo "Ignore this case" > /dev/null
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac
}

handle_e_ip_got()
{
	# Parse 'e_ip_got::xxx.xxx.xxx.xx'
	#MY_IP=`expr "$*" : 'e_ip_got::\(.*\)'`
	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	MY_IP=$1

	case "$IP_MODE" in
	autoip)
		# I statically set the value here.
		MY_NETMASK='255.255.0.0'
		MY_GATEWAYIP='169.254.0.254'
		route add default gw '169.254.0.254'
	;;
	dhcp)
		#MY_NETMASK and $MY_GATEWAYIP will be assigned in /usr/share/udhcpc/default.script
		;;
	static)
		astparam s sec_priority_net_status $IP_VALID
		astparam s third_priority_board_status
		control_net_and_board_led_status
		MY_NETMASK="$NETMASK"
		MY_GATEWAYIP="$GATEWAYIP"
	;;
	*)
	;;
	esac

	if [ "$STATE" = 's_init' ]; then
		## A7 TBD. tickle_watchdog here?
		#tickle_watchdog
		if [ "$SOC_OP_MODE" -ge '2' ]; then
			tcp.sh
			set_mtu
		else
			tcp.sh
		fi
		# The $HOSTNAME_ID is now decided in init_share_param_from_flash()
		if [ -z "$HOSTNAME_CUSTOMIZED" ]; then
			HOSTNAME="${MODEL_NUMBER}-${HOSTNAME_ID}"
		else
			HOSTNAME="$HOSTNAME_CUSTOMIZED"
		fi

		echo "HOSTNAME=$HOSTNAME"
		astsetname $HOSTNAME
		echo "${MODEL_NUMBER}-${HOSTNAME_ID}" > /etc/device_name
		echo $HOSTNAME > /etc/hostname
		hostname -F /etc/hostname

		route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
		# Force IGMP version to Version 2
		echo 2 > /proc/sys/net/ipv4/conf/eth0/force_igmp_version

		#avahi-daemon -D
		name_service -thost
		#httpd -h /www &
		# Start telnetd
		start_telnetd
		p3ktcp &
		usleep 10000
		web &
		case $MODEL_NUMBER in
			KDS-SW3-EN7)
					if [ $P3KCFG_FP_LOCK_ON = 'off' ];then
						lcd_display IPE5000P &
					fi
				;;
			KDS-EN7)
					if [ $P3KCFG_FP_LOCK_ON = 'off' ];then
						lcd_display IPE5000 &
					fi
				;;
			*)
			;;
		esac

		ast_send_event -1 "e_sys_init_ok"
	else
		ast_send_event -1 "e_sys_ip_chg"
	fi

	set_igmp_report
}

handle_e_debug()
{
	dump_parameters

	# set -x is annoying. Disable it.
	return

	if [ "$DBG" = '0' ]; then
		DBG='1'
		set -x
	else
		DBG='0'
		set +x
	fi
}

handle_e_debug_json()
{
	dump_parameters_json
}

handle_e_eth_link_off()
{
	set_mtu

	case "$STATE" in
		s_init | s_idle)
			#  Actually, eth_link_off event can be ignored under s_init state.
			return
		;;
		s_srv_on | s_attaching)
			echo "Network link is down"
			# Link off triggers reconnect (stop and start link).
			do_stop_srv
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac
}

handle_e_eth_link_on()
{
	# Double check
	if eth_link_is_off ; then
		echo "Network link is down again"
		return
	fi
	init_param_from_p3k_cfg
	set_mtu

	case "$STATE" in
		s_init)
			#  Actually, eth_link_off event can be ignored under s_init state.
			return
		;;
		s_idle)
			# Bug. avahi-daemon may not respond to astresname if eth link on/off frequently.
			# To resolve this issue, we try to reload avahi-daemon on each eth link on.
			# avahi-daemon -k
			# From my test, wait for 1 second after link on and load avahi-daemon can
			# resolve the problem that "can't find the host after link down for 10 minutes (SMC switch)".
			# { avahi-daemon -k 2>/dev/null; sleep 1; avahi-daemon -D; } &

			# Eth link mode may change. Should update profile based on new Eth link mode.
			apply_profile_config `select_v_input refresh`
			if [ "$ACCESS_ON" = 'y' ]; then
				if [ "$P3KCFG_AV_ACTION" = 'play' ];then
					handle_e_reconnect_refresh
				fi
			fi
		;;
		*)
			warn "Wrong state?! $STATE"
		;;
	esac

	set_igmp_leave_force

	set_igmp_report
}

handle_e_vw_enable()
{
	#_VW_VAR_MAX_ROW=`expr "$1" : 'e_vw_enable_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}'`
	#_VW_VAR_MAX_COLUMN=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}'`
	#_VW_VAR_ROW=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_COLUMN=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 3;IFS="$_IFS"
	_VW_VAR_MAX_ROW=$1
	_VW_VAR_MAX_COLUMN=$2
	_VW_VAR_ROW=$3
	_VW_VAR_COLUMN=$4

	if [ -z "$_VW_VAR_MAX_ROW" ]; then
		_VW_VAR_MAX_ROW='x'
	fi
	if [ -z "$_VW_VAR_MAX_COLUMN" ]; then
		_VW_VAR_MAX_COLUMN='x'
	fi
	if [ -z "$_VW_VAR_ROW" ]; then
		_VW_VAR_ROW='x'
	fi
	if [ -z "$_VW_VAR_COLUMN" ]; then
		_VW_VAR_COLUMN='x'
	fi
	if [ "$_VW_VAR_MAX_ROW" = 'd' ]; then
		VW_VAR_MAX_ROW="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_MAX_ROW" != 'x' ]; then
		VW_VAR_MAX_ROW="$_VW_VAR_MAX_ROW"
	fi
	if [ "$_VW_VAR_MAX_COLUMN" = 'd' ]; then
		VW_VAR_MAX_COLUMN="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_MAX_COLUMN" != 'x' ]; then
		VW_VAR_MAX_COLUMN="$_VW_VAR_MAX_COLUMN"
	fi

	if [ "$_VW_VAR_ROW" = 'd' ]; then
		VW_VAR_ROW="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_ROW" != 'x' ]; then
		VW_VAR_ROW="$_VW_VAR_ROW"
		#VW_VAR_ROW=`expr $VW_VAR_ROW % \( $VW_VAR_MAX_ROW + 1 \)`
		VW_VAR_ROW=$(( $VW_VAR_ROW % $(( $VW_VAR_MAX_ROW + 1 )) ))
		if [ -z "$VW_VAR_ROW" ]; then
			VW_VAR_ROW='0'
		fi
	fi
	if [ "$_VW_VAR_COLUMN" = 'd' ]; then
		VW_VAR_COLUMN='0'
	elif [ "$_VW_VAR_COLUMN" != 'x' ]; then
		VW_VAR_COLUMN="$_VW_VAR_COLUMN"
		#VW_VAR_COLUMN=`expr $VW_VAR_COLUMN % \( $VW_VAR_MAX_COLUMN + 1 \)`
		VW_VAR_COLUMN=$(( $VW_VAR_COLUMN % $(( $VW_VAR_MAX_COLUMN + 1 )) ))
		if [ -z "$VW_VAR_COLUMN" ]; then
			VW_VAR_COLUMN='0'
		fi
	fi
	#VW_VAR_LAYOUT=`expr $VW_VAR_MAX_ROW + 1`x`expr $VW_VAR_MAX_COLUMN + 1`
	VW_VAR_LAYOUT=$(( $VW_VAR_MAX_ROW + 1 ))x$(( $VW_VAR_MAX_COLUMN + 1 ))

	vw_enable
	./astparam s vw_max_row ${VW_VAR_MAX_ROW}
	./astparam s vw_max_column ${VW_VAR_MAX_COLUMN}
	./astparam s vw_row ${VW_VAR_ROW}
	./astparam s vw_column ${VW_VAR_COLUMN}
	./astparam save
}

handle_e_vw_moninfo()
{
	#VW_VAR_MONINFO_HA=`expr "$1" : 'e_vw_moninfo_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_HT=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_VA=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_VT=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 3;IFS="$_IFS"
	VW_VAR_MONINFO_HA=$1
	VW_VAR_MONINFO_HT=$2
	VW_VAR_MONINFO_VA=$3
	VW_VAR_MONINFO_VT=$4

	echo "HA=$VW_VAR_MONINFO_HA HT=$VW_VAR_MONINFO_HT VA=$VW_VAR_MONINFO_VA VT=$VW_VAR_MONINFO_VT"
	if [ -z "$VW_VAR_MONINFO_HA" ]; then
		VW_VAR_MONINFO_HA='0'
	fi
	if [ -z "$VW_VAR_MONINFO_HT" ]; then
		VW_VAR_MONINFO_HT='0'
	fi
	if [ -z "$VW_VAR_MONINFO_VA" ]; then
		VW_VAR_MONINFO_VA='0'
	fi
	if [ -z "$VW_VAR_MONINFO_VT" ]; then
		VW_VAR_MONINFO_VT='0'
	fi
	echo "$VW_VAR_MONINFO_HA $VW_VAR_MONINFO_HT $VW_VAR_MONINFO_VA $VW_VAR_MONINFO_VT" > "$VIDEO_SYS_PATH"/vw_frame_comp

	astparam s vw_moninfo_ha "$VW_VAR_MONINFO_HA"
	astparam s vw_moninfo_ht "$VW_VAR_MONINFO_HT"
	astparam s vw_moninfo_va "$VW_VAR_MONINFO_VA"
	astparam s vw_moninfo_vt "$VW_VAR_MONINFO_VT"
	astparam save
}

#
# e_vw_refresh_pos_idx_start_$RowCnt_$ColCnt
# e_vw_refresh_pos_idx_head_$TheMac_$PosIdx
# e_vw_refresh_pos_idx_tail_$TheMac_$TailIdx
#
# e_vw_refresh_pos_idx_force_$PosIdx_$TailIdx
#
handle_e_vw_refresh_pos_idx()
{
	_cmd=$*

	case "$_cmd" in
		e_vw_refresh_pos_idx_start_?*)
			## A7 to be removed. Drop rs232 chain support
			return
		;;
		e_vw_refresh_pos_idx_force_?*)
			#VW_VAR_POS_IDX=`expr $* : 'e_vw_refresh_pos_idx_force_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
			#VW_VAR_TAIL_IDX=`expr $* : 'e_vw_refresh_pos_idx_force_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
			_IFS="$IFS";IFS='_';set -- $_cmd;shift 6;IFS="$_IFS"
			VW_VAR_POS_IDX=$1
			VW_VAR_TAIL_IDX=$2
			VW_VAR_THE_MAX_ROW="$VW_VAR_TAIL_IDX"
			VW_VAR_POS_R="$VW_VAR_POS_IDX"
			VW_VAR_POS_C='0'
			THE_ROW_ID="$VW_VAR_POS_IDX"
			astparam s vw_pos_idx "$VW_VAR_POS_IDX"
			astparam s vw_tail_idx "$VW_VAR_TAIL_IDX"
			astparam save
			return
		;;
		e_vw_refresh_pos_idx_head_?* | e_vw_refresh_pos_idx_tail_?*)
			## A7 to be removed. Drop rs232 chain support
			return
		;;
		*)
			return
		;;
	esac
}

handle_e_vw_v_shift_d()
{
	#VW_VAR_V_SHIFT=`expr "$*" : 'e_vw_v_shift_d_\([[:digit:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 5;IFS="$_IFS"
	VW_VAR_V_SHIFT=$1

	if [ -z "$VW_VAR_V_SHIFT" ]; then
		VW_VAR_V_SHIFT='0'
	fi
	echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam save
}
handle_e_vw_v_shift_u()
{
	#VW_VAR_V_SHIFT=`expr "$*" : 'e_vw_v_shift_u_\([[:digit:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 5;IFS="$_IFS"
	VW_VAR_V_SHIFT=$1

	if [ -z "$VW_VAR_V_SHIFT" ]; then
		VW_VAR_V_SHIFT='0'
	fi
	if [ "$VW_VAR_V_SHIFT" != '0' ]; then
		VW_VAR_V_SHIFT="-$VW_VAR_V_SHIFT"
	fi
	echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam save
}

handle_e_vw_reset_to_pos()
{
	#_VW_VAR_MAX_ROW=`expr "$*" : 'e_vw_reset_to_pos_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_MAX_COLUMN=`expr "$*" : 'e_vw_reset_to_pos_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 5;IFS="$_IFS"
	_VW_VAR_MAX_ROW=$1
	_VW_VAR_MAX_COLUMN=$2

	if [ -z "$_VW_VAR_MAX_ROW" ]; then
		_VW_VAR_MAX_ROW='x'
	fi
	if [ -z "$_VW_VAR_MAX_COLUMN" ]; then
		_VW_VAR_MAX_COLUMN='x'
	fi

	if [ "$_VW_VAR_MAX_ROW" = 'd' ]; then
		VW_VAR_MAX_ROW="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_MAX_ROW" != 'x' ]; then
		VW_VAR_MAX_ROW="$_VW_VAR_MAX_ROW"
	fi
	if [ "$_VW_VAR_MAX_COLUMN" = 'd' ]; then
		VW_VAR_MAX_COLUMN="$VW_VAR_POS_IDX"
	elif [ "$_VW_VAR_MAX_COLUMN" != 'x' ]; then
		VW_VAR_MAX_COLUMN="$_VW_VAR_MAX_COLUMN"
	fi

	VW_VAR_ROW="$VW_VAR_POS_R"
	VW_VAR_COLUMN="$VW_VAR_POS_C"
	#VW_VAR_MONINFO_HA='0'
	#VW_VAR_MONINFO_HT='0'
	#VW_VAR_MONINFO_VA='0'
	#VW_VAR_MONINFO_VT='0'
	VW_VAR_H_SHIFT='0'
	VW_VAR_V_SHIFT='0'
	VW_VAR_H_SCALE='0'
	VW_VAR_V_SCALE='0'
	VW_VAR_DELAY_KICK='0'
	echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
	vw_enable
	astparam s vw_max_row "$VW_VAR_MAX_ROW"
	astparam s vw_max_column "$VW_VAR_MAX_COLUMN"
	astparam s vw_row "$VW_VAR_ROW"
	astparam s vw_column "$VW_VAR_COLUMN"
	#astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	#astparam s vw_h_scale "$VW_VAR_H_SCALE"
	#astparam s vw_v_scale "$VW_VAR_V_SCALE"
	#astparam s vw_delay_kick "$VW_VAR_DELAY_KICK"
	astparam save
}

handle_e_vw_reset_to_pos_s()
{
	VW_VAR_MAX_ROW='0'
	VW_VAR_MAX_COLUMN='0'
	VW_VAR_ROW='0'
	VW_VAR_COLUMN='0'
	#VW_VAR_MONINFO_HA='0'
	#VW_VAR_MONINFO_HT='0'
	#VW_VAR_MONINFO_VA='0'
	#VW_VAR_MONINFO_VT='0'
	VW_VAR_H_SHIFT='0'
	VW_VAR_V_SHIFT='0'
	VW_VAR_H_SCALE='0'
	VW_VAR_V_SCALE='0'
	VW_VAR_DELAY_KICK='0'
	echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
	vw_enable
	astparam s vw_max_row "$VW_VAR_MAX_ROW"
	astparam s vw_max_column "$VW_VAR_MAX_COLUMN"
	astparam s vw_row "$VW_VAR_ROW"
	astparam s vw_column "$VW_VAR_COLUMN"
	#astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	#astparam s vw_h_scale "$VW_VAR_H_SCALE"
	#astparam s vw_v_scale "$VW_VAR_V_SCALE"
	#astparam s vw_delay_kick "$VW_VAR_DELAY_KICK"
	astparam save
}

select_audio_input()
{
	# $1: hdmi, analog
	local _type="$1"

	if [ -f "$I2S_SYS_PATH/io_select" ]; then
		#echo "$_type" > $I2S_SYS_PATH/io_select
		echo "no switch"
	fi
}

handle_e_v_input_digital()
{
	apply_profile_config 'digital'
	select_audio_input `select_a_input`
}

handle_e_v_input_analog()
{
	apply_profile_config 'analog'
	select_audio_input `select_a_input`
}

handle_e_v_input_0()
{
	# Triggered by 'v_input' GPIO interrupt event.
	# Digital port selected.

	# For the design that astparam has higher priority than GPIO.
	apply_profile_config `select_v_input gpio`

	# User toggle GPIO has higher priority than astparam specific setting.
	#apply_profile_config 'digital'

	select_audio_input `select_a_input`
}

handle_e_v_input_1()
{
	# Triggered by 'v_input' GPIO interrupt event.
	# Analog port selected.

	# For the design that astparam has higher priority than GPIO.
	apply_profile_config `select_v_input gpio`

	# User toggle GPIO has higher priority than astparam specific setting.
	#apply_profile_config 'analog'

	select_audio_input `select_a_input`
}

handle_e_v_src_unavailable()
{
	apply_profile_config `select_v_input swap`
	select_audio_input `select_a_input`
}

handle_e_a_input_0()
{
	# e_a_input_0 event is triggered by host line in hotplug GPIO
	# Select a_input port 0 (digital)
	select_audio_input `select_a_input`
}

handle_e_a_input_1()
{
	# e_a_input_1 event is triggered by host line in hotplug GPIO
	# Select a_input port 1 (analog)
	select_audio_input `select_a_input`
}

handle_e_vlm_notify_quality_mode_chg()
{
	# notified by vLM
	V_QUALITY_MODE=`ipc @v_lm_query q ve_param_query:V_QUALITY_MODE`
	astparam s ast_video_quality_mode "$V_QUALITY_MODE"
	astparam save
}

handle_e_vlm_notify_anti_dither_mode_chg()
{
	# notified by vLM
	V_BCD_THRESHOLD=`ipc @v_lm_query q ve_param_query:V_BCD_THRESHOLD`
	astparam s v_bcd_threshold "$V_BCD_THRESHOLD"
	astparam save
}

handle_e_vlm_notify_snoop_chg()
{
	# notified by vLM
	V_LOOPBACK_ENABLED=`ipc @v_lm_query q ve_param_query:V_LOOPBACK_ENABLED`
}

handle_e_ulm_notify_chg()
{
	USB_CLIENT_IP=`ipc @u_lm_query q ue_param_query:U_USBIP_CLIENT`
}

handle_e_video_stat()
{
	case "$1" in
		e_video_stat_mode:?*)
			if [ "$NO_KMOIP" = 'y' ]; then
				# break is used to terminate the execution of entire loop
				# place 'break' here will break the loop in state_machine()
				# use return instead
				return
			fi
			# e_video_stat_mode::x::y
			_IFS="$IFS";IFS=':';set -- $1;IFS="$_IFS"
			local x=$3
			local y=$5
			echo "Video Resolution: ${x}x${y}"
			ipc @u_lm_set s ue_kmr_chg:${x}:${y}
		;;
		*)
		;;
	esac
}

handle_e_sh()
{
	#_str=`expr "$*" : 'e_sh_\(.*\)'`
	_IFS="$IFS";IFS='_';set -- $*;shift 2;_str="$*";IFS="$_IFS"
	parse_n_exec "$_str"
}

handle_e_sh_jsonp()
{
	#_str=`expr "$*" : 'callback=.+&e_sh_\(.*\)'`
	_IFS="$IFS";IFS='&';set -- $*;shift;_str="$*";IFS="$_IFS"
	_IFS="$IFS";IFS='_';set -- $_str;shift 2;_str="$*";IFS="$_IFS"
	parse_n_exec "$_str"
}

handle_e_button()
{
	case "$*" in
		e_button_link_5)
			p3k_notify button::long-reset
			./reset_to_default.sh
			reboot
			#handle_e_button_link
		;;
		*)
		;;
	esac
}

handle_e_pwr_status_changed()
{
	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	ipc @p_lm_set s pe_pwr_status_changed:$1
}

handle_e_vw()
{
	case "$*" in
		e_vw_pos_layout_?*)
			handle_e_vw_pos_layout "$event"
		;;
		e_vw_enable_?*)
			handle_e_vw_enable "$event"
		;;
		e_vw_moninfo_?*)
			handle_e_vw_moninfo "$event"
		;;
		e_vw_v_shift_u_?*)
			handle_e_vw_v_shift_u "$event"
		;;
		e_vw_v_shift_d_?*)
			handle_e_vw_v_shift_d "$event"
		;;
		e_vw_reset_to_pos_s_?*)
			handle_e_vw_reset_to_pos_s "$event"
		;;
		e_vw_reset_to_pos_?*)
			handle_e_vw_reset_to_pos "$event"
		;;
		e_vw_refresh_pos_idx_?*)
			handle_e_vw_refresh_pos_idx "$event"
		;;
		*)
			echo "Unknown Video Wall event?! ($*)"
		;;
	esac
}

handle_e_var_dump()
{
	_var_dump "$*"
}

handle_e_var_get()
{
	_var_get "$*"
}

handle_e_var_set()
{
	_var_set "$*"
}

handle_e_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_lmh.sh ]; then
		source ./patch_lmh.sh
	fi
}

handle_e_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > /var/lm_var
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < /var/lm_var |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> /var/lm_var
		}
	) &
	wait $!
}

handle_e_key()
{
	case "$*" in
		e_key_enter_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
				return
			fi
			send_key_info 1
			p3k_notify button::enter
		;;
		e_key_up_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' -a "$P3KCFG_FP_LOCK_ON" = 'off' ];then
				LED_DISPLAY_CH_SELECT_V=`echo -e $LED_DISPLAY_CH_SELECT_V | sed -r 's/0*([0-9])/\1/'`
				LED_DISPLAY_CH_SELECT_V=$[ $LED_DISPLAY_CH_SELECT_V + 1 ]
				if [ $LED_DISPLAY_CH_SELECT_V -gt 999 ];then
					LED_DISPLAY_CH_SELECT_V=1
				fi
				kill_process wp_dec7_key_timer.sh
				./wp_dec7_key_timer.sh $LED_DISPLAY_CH_SELECT_V &
				return
			fi
			send_key_info 2
			p3k_notify button::up
		;;
		e_key_down_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' -a "$P3KCFG_FP_LOCK_ON" = 'off' ];then
				LED_DISPLAY_CH_SELECT_V=`echo -e $LED_DISPLAY_CH_SELECT_V | sed -r 's/0*([0-9])/\1/'`
				LED_DISPLAY_CH_SELECT_V=$[ $LED_DISPLAY_CH_SELECT_V - 1 ]
				if [ $LED_DISPLAY_CH_SELECT_V -lt 1 ];then
					LED_DISPLAY_CH_SELECT_V=999
				fi
				kill_process wp_dec7_key_timer.sh
				./wp_dec7_key_timer.sh $LED_DISPLAY_CH_SELECT_V &
				return
			fi
			send_key_info 3
			p3k_notify button::down
		;;
		e_key_left_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
				return
			fi
			send_key_info 4
			p3k_notify button::left
		;;
		e_key_right_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
				return
			fi
			send_key_info 5
			p3k_notify button::right
		;;
		*)
		;;
	esac
}

handle_e_p3k_switch_mode()
{
	local _switch_mode

	#e_p3k_switch_mode::switch_mode
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_switch_mode="$1"

	echo "set p3k switch mode!!! $_switch_mode"
	sconfig --mode "$_switch_mode"
}

handle_e_p3k_switch_pri()
{
	echo "handle_e_p3k_switch_pri."
	local _pri_1
	local _pri_2
	local _pri_3

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_pri_1="$1"

	shift 2
	_pri_2="$1"

	shift 2

	case "$MODEL_NUMBER" in
		WP-SW2-EN7)
			_pri_3=HDMI3
		;;
		KDS-SW3-EN7)
			_pri_3="$1"
		;;
		*)
			echo "error switch_pri parameter"
			return
		;;
	esac

	echo "set p3k switch pri!!! $_pri_1 $_pri_2 $_pri_3"
	sconfig --priority "$_pri_1" "$_pri_2" "$_pri_3"
}

handle_e_p3k_switch_in()
{
	echo "handle_e_p3k_switch_in."
	local _switch_input

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_switch_input="$1"

	echo "set p3k switch input!!! $_switch_input"
	sconfig --input "$_switch_input"
}

handle_e_p3k_switch()
{
	echo "handle_e_p3k_switch."
	case "$*" in
		e_p3k_switch_mode::?*)
			handle_e_p3k_switch_mode "$event"
		;;
		e_p3k_switch_pri::?*)
			handle_e_p3k_switch_pri "$event"
		;;
		e_p3k_switch_in::?*)
			handle_e_p3k_switch_in "$event"
		;;
		*)
		;;
	esac
}

handle_e_p3k_edid()
{
	echo "handle_e_p3k_edid."
	local _para1
	local _para2

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	case "$event" in
		e_p3k_video_edid_add::?*)
			echo "e_p3k_video_edid_add ($event) received"
			shift 2
			_para1="$1"
			_para2="$3"
			handle_edid -a $_para2 -i $_para1
		;;
		e_p3k_video_edid_remove::?*)
			shift 2
			_para1="$1"
			handle_edid -d -i $_para1
		;;
		e_p3k_video_edid_default)
			kill_process handle_edid
			handle_edid -s default
		;;
		e_p3k_video_edid_passthru::?*)
			echo "e_p3k_video_edid_passthru ($event) received"
			shift 2
			_para1="$1"
			kill_process handle_edid
			handle_edid -c $_para1 -t $rx_tcp_port -m 1
			handle_edid -c $_para1 -t $rx_tcp_port &
		;;
		e_p3k_video_edid_custom::?*)
			kill_process handle_edid
			echo "e_p3k_video_edid_custom ($event) received"
			shift 2
			_para1="$1"
			handle_edid -s custom -i $_para1
		;;
		*)
		;;
	esac
}

handle_e_p3k_video()
{
	echo "handle_e_p3k_video."
	local _para1
	local _para2

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_video_edid?*)
			handle_e_p3k_edid "$event"
		;;
		e_p3k_video_hdcp_mode::?*)
			echo "e_p3k_video_hdcp_mode ($event) received"
			_para2="$3"
			if [ $_para2 = '0' ];then
				P3KCFG_HDCP_1_ON='off'
				P3KCFG_HDCP_2_ON='off'
				P3KCFG_HDCP_3_ON='off'
			else
				P3KCFG_HDCP_1_ON='on'
				P3KCFG_HDCP_2_ON='on'
				P3KCFG_HDCP_3_ON='on'
			fi
			set_hdcp_status
		;;
		*)
		echo "ERROR!!!! Invalid event ($event) received"
		;;
	esac
}

handle_e_p3k_audio_src()
{
	echo "handle_e_p3k_audio_src."
	local _switch_input

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_switch_input="$1"

	if [ $_switch_input = 'no' ];then
		echo 0 > /sys/devices/platform/1500_i2s/analog_in_vol
	else
		echo 100 > /sys/devices/platform/1500_i2s/analog_in_vol
	fi

	echo "set p3k switch input!!! $_switch_input"
	case "$MODEL_NUMBER" in
		KDS-EN7)
			sconfig --audio-input "$_switch_input"
		;;
		KDS-SW3-EN7)
			sconfig --audio-input "$_switch_input"
		;;
		WP-SW2-EN7)
			sconfig --audio-input "$_switch_input"
		;;
		*)
		;;
	esac
	
}

handle_e_p3k_audio_dst()
{
	echo "handle_e_p3k_audio_dst."
	local _dst_num
	local _dst_1
	local _dst_2
	local _dst_3
	local _dst_4

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_dst_num="$1"
	_dst_1="$3"
	_dst_2="$5"
	_dst_3="$7"
	_dst_4="$9"

	case "$_dst_num" in
		0)
			sconfig --audio-output no
			echo "sconfig --audio-output no"
		;;
		1)
			sconfig --audio-output "$_dst_1"
			echo "sconfig --audio-output $_dst_1"
		;;
		2)
			sconfig --audio-output "$_dst_1" "$_dst_2"
			echo "sconfig --audio-output $_dst_1 $_dst_2"
		;;
		3)
			sconfig --audio-output "$_dst_1" "$_dst_2" "$_dst_3"
			echo "sconfig --audio-output $_dst_1 $_dst_2 $_dst_3"
		;;
		4)
			sconfig --audio-output "$_dst_1" "$_dst_2" "$_dst_3" "$_dst_4"
			echo "sconfig --audio-output $_dst_1 $_dst_2 $_dst_3 $_dst_4"
		;;
		*)
		echo "ERROR!!!! Invalid dst_num ($_dst_num) received"
		;;
	esac

}

handle_e_p3k_audio_pri()
{
	echo "handle_e_p3k_audio_pri."
	local _pri_1
	local _pri_2
	local _pri_3

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_pri_1="$1"

	shift 2
	_pri_2="$1"

	shift 2
	_pri_3="$1"

	echo "set p3k switch pri!!! $_pri_1 $_pri_2 $_pri_3"
	sconfig --audio-priority "$_pri_1" "$_pri_2" "$_pri_3"
}

handle_e_p3k_audio_mode()
{
	echo "handle_e_p3k_audio_mode."
	local _switch_mode

	#e_p3k_switch_mode::switch_mode
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_switch_mode="$1"

	echo "set p3k switch mode!!! $_switch_mode"
	sconfig --audio-mode "$_switch_mode"
}

handle_e_p3k_audio_switch()
{
	echo "handle_e_p3k_audio_switch."
	local _para1
	local _para2

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_audio_src?*)
			handle_e_p3k_audio_src "$event"
		;;
		e_p3k_audio_dst?*)
			handle_e_p3k_audio_dst "$event"
		;;
		e_p3k_audio_pri::?*)
			handle_e_p3k_audio_pri "$event"
		;;
		e_p3k_audio_mode?*)
			handle_e_p3k_audio_mode "$event"
		;;
		*)
		echo "ERROR!!!! Invalid event ($event) received"
		;;
	esac
}

web_mute_slider_handle()
{
	if [ $1 != '1' ];then
		#unmute
		echo 100 > /sys/devices/platform/1500_i2s/analog_in_vol
		if [ $P3KCFG_ANALOG_DIR = 'in' ];then
			echo 1 > /sys/class/leds/linein_mute/brightness
		fi
	    echo 1 > /sys/class/leds/lineout_mute/brightness
		#Because KDS-EN7 use gsv2001 chip,it just have one core,therefore, mute outputs one way and follows mute later
		#but KDS-SW3-EN7 use gsv2008 will be no such problem,because it have two core
		ipc @m_lm_set s set_hdmi_mute:16:1:0
		if [ $MODEL_NUMBER = 'KDS-SW3-EN7' ];then
			ipc @m_lm_set s set_hdmi_mute:17:1:0
			echo 0 > /sys/class/leds/dante_mute/brightness
		fi
	else
		#mute,Because the linein mute sound is still a little, you can directly turn the volume to 0 as mute
		echo 0 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 0 > /sys/class/leds/linein_mute/brightness
	    echo 0 > /sys/class/leds/lineout_mute/brightness
		ipc @m_lm_set s set_hdmi_mute:16:1:1
		if [ $MODEL_NUMBER = 'KDS-SW3-EN7' ];then
			ipc @m_lm_set s set_hdmi_mute:17:1:1
			echo 1 > /sys/class/leds/dante_mute/brightness
		fi
	fi
}

handle_e_p3k_audio()
{
	echo "handle_e_p3k_audio."
	local _para1

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_audio_level::?*)
			analog_out_volum=$_para1
			ipc @a_lm_set s ae_level:$_para1
		;;
		e_p3k_audio_dir::?*)
			#ipc @a_lm_set s ae_dir:$_para1
			case "$_para1" in
				in)
					P3KCFG_ANALOG_DIR='in'
					ipc @m_lm_set s set_gpio_val:1:70:1
				;;
				out)
					P3KCFG_ANALOG_DIR='out'
					ipc @m_lm_set s set_gpio_val:1:70:0
				;;
				*)
				;;
			esac
		;;
		e_p3k_audio_mute::?*)
			web_mute_slider_handle $_para1
		;;
		e_p3k_audio_dante_name::?*)
			ipc @m_lm_set s set_dante:0:$_para1
		;;
		e_p3k_audio_?*)
			handle_e_p3k_audio_switch "$event"
		;;
		*)
		echo "ERROR!!!! Invalid event ($event) received"
		;;
	esac

}

handle_e_p3k_ir()
{
	echo "handle_e_p3k_ir."
	local _para1

	#e_p3k_ir_dir::dir
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_ir_dir::?*)
			ipc @r_lm_set s re_dir:$_para1
		;;
		e_p3k_ir_gw::?*)
			ipc @r_lm_set s re_gw:$_para1
		;;
		e_p3k_ir_send::?*)
			ipc @r_lm_set s re_send:$_para1
		;;
		*)
		;;
	esac
}

open_cec_over_ip()
{
	NO_CEC='n'
	CEC_GATWAY='off'
	ipc @c_lm_set s ce_start:$CH_SELECT_C
}

ban_cec_over_ip()
{
	NO_CEC='y'
	CEC_GATWAY='on'
	ipc @c_lm_set s ce_stop
}

enable_hdmi_in_cec_report()
{
	echo 1 > /sys/devices/platform/cec/cec_report
	ipc @m_lm_set s cec_report:0
}

enable_hdmi_out_cec_report()
{
	echo 0 > /sys/devices/platform/cec/cec_report
	ipc @m_lm_set s cec_report:1
}

handle_ce_gw()
{
	local _para1=$1
	
	case "$MODEL_NUMBER" in
		KDS-EN7)
			case $1 in
				over_ip)
					ipc @m_lm_set s set_gpio_val:1:68:1
					open_cec_over_ip
				;;
				hdmi_in)
					ipc @m_lm_set s set_gpio_val:1:68:1
					ban_cec_over_ip
					enable_hdmi_in_cec_report
					CEC_SEND_DIR='hdmi_in'
				;;
				hdmi_out)
					ban_cec_over_ip
					enable_hdmi_out_cec_report
					CEC_SEND_DIR='hdmi_out'
				;;
				*)
				;;
			esac
		;;
		KDS-SW3-EN7)
			case $1 in
				over_ip)
					open_cec_over_ip
				;;
				#The switching of CEC varies with the switching of video source(in commun_with_mcu process)
				hdmi_in)
					ban_cec_over_ip
					enable_hdmi_in_cec_report
					CEC_SEND_DIR='hdmi_in'
				;;
				hdmi_out)
					ban_cec_over_ip
					enable_hdmi_out_cec_report
					CEC_SEND_DIR='hdmi_out'
				;;
				*)
				;;
			esac
		;;
		WP-SW2-EN7)
			case $1 in
				over_ip)
					open_cec_over_ip
				;;
				#The switching of CEC varies with the switching of video source(in commun_with_mcu process)
				hdmi_in)
					ban_cec_over_ip
					enable_hdmi_in_cec_report
				;;
				*)
				;;
			esac
		;;
		*)
		;;
	esac

	echo "handle_ce_gw.($_para1)" 
}

handle_ce_send()
{
	echo "$1"
	local _para1=$1
	case $CEC_SEND_DIR in
		hdmi_in)
			cec_send $1
		;;
		hdmi_out)
			array=(${_para1//:/ })
			ipc @m_lm_set s cec_send:${#array[@]}:$1
		;;
		*)
		;;
	esac
	echo "handle_ce_send.($_para1)" 
}

handle_e_p3k_cec()
{
	echo "handle_e_p3k_cec."
	local _para1

	_para1=${event#*::}

	case "$event" in
		e_p3k_cec_gw::?*)
			handle_ce_gw $_para1
		;;
		e_p3k_cec_send::?*)
			handle_ce_send $_para1
		;;
		*)
		;;
	esac
}

handle_e_p3k_upgrade()
{
	rm /www/tmp.*
	cd /dev/shm/
	if ! tar -zxvf ./IP5000-A30_upgrade.tar.gz; then
		echo "err,0,2" > "/www/fw_status.txt"
		return
	fi
	if [ -x flash.sh ]; then
		./flash.sh
		sleep 5 # wait client query result
		reboot
	else
		echo "err,0,2" > "/www/fw_status.txt"
	fi
}

handle_e_p3k_net_dhcp()
{
	echo "handle_e_p3k_net_dhcp."
	local _para1

	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$3"

	astparam s ip_mode $_para1
	astparam save
}

handle_e_p3k_net_conf()
{
	echo "handle_e_p3k_net_conf."
	local _para_ip
	local _para_mask
	local _para_gw
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para_ip="$3"
	_para_mask="$5"
	_para_gw="$7"

	astparam s ipaddr $_para_ip
	astparam s netmask $_para_mask
	astparam s gatewayip $_para_gw
	astparam save
}

handle_e_p3k_net_daisychain()
{
	echo "handle_e_p3k_net_daisychain."
}

handle_e_p3k_net_method()
{
	local _para1

	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$_para1" in
		1)
			astparam s multicast_on n
		;;
		2)
			astparam s multicast_on y
		;;
		*)
		;;
	esac

	astparam save
	echo "handle_e_p3k_net_method."
}

handle_e_p3k_net_multicast()
{
	echo "handle_e_p3k_net_multicast."
}

handle_e_p3k_net_vlan()
{
	echo "handle_e_p3k_net_vlan."
}

handle_e_p3k_net_gw_port()
{
	echo "handle_e_p3k_net_gw_port."
}

handle_e_p3k_net_hostname()
{
	echo "handle_e_p3k_net_hostname."
}

handle_e_p3k_net_dante_name()
{
	echo "handle_e_p3k_net_dante_name."
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	ipc @m_lm_set s set_dante:0:$_para1
}

handle_e_p3k_fp_lock()
{
	case "$event" in
		e_p3k_fp_lock_off)
			P3KCFG_FP_LOCK_ON='off'
			case $MODEL_NUMBER in
				KDS-SW3-EN7)
					echo 0 > /sys/class/leds/lcd_power/brightness
					usleep 500000
					lcd_display IPE5000P &
				;;
				KDS-EN7)
					echo 0 > /sys/class/leds/lcd_power/brightness
					usleep 500000
					lcd_display IPE5000 &
				;;
				WP-SW2-EN7)
					ipc @m_lm_set s set_lcd_control:0:1
					led_display_num
				;;
				*)
					echo "error param"
				;;
			esac
		;;
		e_p3k_fp_lock_on)
			P3KCFG_FP_LOCK_ON='on'
			case $MODEL_NUMBER in
				WP-SW2-EN7)
					ipc @m_lm_set s set_lcd_control:0:0
				;;
				KDS-EN7 | KDS-SW3-EN7)
					echo 1 > /sys/class/leds/lcd_power/brightness
					pkill -9 lcd_display
				;;
				*)
					echo "error param"
				;;
			esac
		;;
		*)
			echo "error param" 
		;;
	esac
}

handle_e_p3k_flagme()
{
	if [ `ps | grep -c flag_me.sh` = '2' ];then
		pkill -9 flag_me.sh
	fi
	astparam s fir_priority_net_status $FLAG_ME_STATUS
	astparam s sec_priority_board_status $FLAG_ME_STATUS
	control_net_and_board_led_status
	./flag_me.sh &
}

handle_e_p3k_flagme_timeout()
{
	astparam s fir_priority_net_status
	astparam s sec_priority_board_status
	control_net_and_board_led_status
}

handle_e_p3k_download_fw_start()
{
	astparam s fir_priority_board_status $FIRMWARE_DOWNLOAD
	control_board_led_status
}

init_p3k_net_vlan()
{
	echo "init_p3k_net_vlan"
	local _media_control_vlan='0'
	local _media_dante_vlan='0'
	local _service_control_vlan='0'
	local _service_dante_vlan='0'

	local _control_vlan_tag='0'

	case "$P3KCFG_CONTROL_PORT" in
		eth0)
			case "$P3KCFG_CONTROL_VLAN" in
				1)
					_media_control_vlan='1'
					_control_vlan_tag='0'
				;;
				*)
					_media_control_vlan="$P3KCFG_CONTROL_VLAN"
					_control_vlan_tag="$P3KCFG_CONTROL_VLAN"
				;;
			esac
		;;
		eth1)
			case "$P3KCFG_CONTROL_VLAN" in
				1)
					_service_control_vlan='1'
					_control_vlan_tag='4094'
				;;
				*)
					_service_control_vlan="$P3KCFG_CONTROL_VLAN"
					_control_vlan_tag="$P3KCFG_CONTROL_VLAN"
				;;
			esac
		;;
		*)
			echo "P3KCFG_CONTROL_PORT error param"
		;;
	esac

	case "$MODEL_NUMBER" in
		KDS-SW3-EN7)
			case "$P3KCFG_DANTE_PORT" in
				eth0)
					case "$P3KCFG_DANTE_VLAN" in
						1)
							_media_dante_vlan='1'
						;;
						*)
							_media_dante_vlan="$P3KCFG_CONTROL_VLAN"
						;;
					esac
				;;
				eth1)
					case "$P3KCFG_DANTE_VLAN" in
						1)
							_service_dante_vlan='1'
						;;
						*)
							_service_dante_vlan="$P3KCFG_CONTROL_VLAN"
						;;
					esac
				;;
				*)
					echo "P3KCFG_DANTE_PORT error param"
				;;
			esac
		;;
		*)
		;;
	esac

	handle_e_p3k_vlan_set_rtl_chip e_p3k_net_vlan_set_rtl_chip::"$_media_control_vlan":"$_media_dante_vlan":"$_service_control_vlan":"$_service_dante_vlan"

	case "$_control_vlan_tag" in
		0)
			echo "init_p3k_net_vlan control no vlan"
		;;
		*)
			case "$P3KCFG_CONTROL_MODE" in
				dhcp)
					handle_e_p3k_net_vlan_dhcp_on e_p3k_net_vlan_dhcp_on::"$_control_vlan_tag"
				;;
				static)
					handle_e_p3k_net_vlan_dhcp_off e_p3k_net_vlan_dhcp_off::"$_control_vlan_tag":"$P3KCFG_CONTROL_IP":"$P3KCFG_CONTROL_MASK"
				;;
				*)
					echo "P3KCFG_CONTROL_MODE error param"
				;;
			esac
		;;
	esac

}
#e e_p3k_net_vlan_del::VID
handle_e_p3k_net_vlan_del()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	ifconfig eth0.$1 down
	vconfig rem eth0.$1
}

#e e_p3k_net_vlan_dhcp_on::vlan_tag
handle_e_p3k_net_vlan_dhcp_on()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	echo "handle_e_p3k_net_vlan_dhcp_on. $1 "

	shift 2
	vconfig add eth0 $1
	ifconfig eth0.$1 up
	astparam s network_card eth0.$1

	udhcpc -i eth0.$1 -f &
}

#e e_p3k_net_vlan_dhcp_off::vlan_tag:ip:netmask
handle_e_p3k_net_vlan_dhcp_off()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	echo "handle_e_p3k_net_vlan_dhcp_off. $1 $2 $3"

	shift 2
	vconfig add eth0 $1
	ifconfig eth0.$1 $2 netmask $3 up
	astparam s network_card
}

handle_e_p3k_vlan_set_rtl_chip()
{

	#kill rs232_process
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	# $1--media_p3k&rs232_vlan_tag
	# $2--media_dante_vlan_tag
	# $3--service_p3k&rs232_vlan_tag
	# $4--service_dante_vlan_tag
	# eth_settings: p3k_port_tag rs232_gateway_tag dante_port_tag pvid

	echo "handle_e_p3k_vlan_set_rtl_chip. $1 $2 $3 $4"

	echo $1 0 $2 0 > /sys/devices/platform/ftgmac/eth0_settings
	echo $3 0 $4 0 > /sys/devices/platform/ftgmac/eth1_settings
	case "$MODEL_NUMBER" in
		WP-SW2-EN7 | KDS-EN7)
			echo 2 > /sys/devices/platform/ftgmac/set_vlan
		;;
		KDS-SW3-EN7)
			echo 1 > /sys/devices/platform/ftgmac/set_vlan
		;;
		*)
		;;
	esac
}

handle_e_p3k_net()
{
	echo "handle_e_p3k_net."

	case "$event" in
		e_p3k_net_dhcp::?*)
			handle_e_p3k_net_dhcp "$event"
		;;
		e_p3k_net_conf::?*)
			handle_e_p3k_net_conf "$event"
		;;
		e_p3k_net_daisychain::?*)
			handle_e_p3k_net_daisychain "$event"
		;;
		e_p3k_net_method::?*)
			handle_e_p3k_net_method "$event"
		;;
		e_p3k_net_multicast::?*)
			handle_e_p3k_net_multicast "$event"
		;;
		e_p3k_net_vlan::?*)
			handle_e_p3k_net_vlan "$event"
		;;
		e_p3k_net_vlan_del::?*)
			handle_e_p3k_net_vlan_del "$event"
		;;
		e_p3k_net_vlan_set_rtl_chip::?*)
			handle_e_p3k_vlan_set_rtl_chip "$event"
		;;
		e_p3k_net_vlan_dhcp_on::?*)
			handle_e_p3k_net_vlan_dhcp_on "$event"
		;;
		e_p3k_net_vlan_dhcp_off::?*)
			handle_e_p3k_net_vlan_dhcp_off "$event"
		;;
		e_p3k_net_gw_port::?*)
			handle_e_p3k_net_gw_port "$event"
		;;
		e_p3k_net_hostname::?*)
			handle_e_p3k_net_hostname "$event"
		;;
		e_p3k_net_dante_name::?*)
			handle_e_p3k_net_dante_name "$event"
		;;
		*)
		;;
	esac
}

handle_e_p3k_ntp_on()
{
	echo "handle_e_p3k_ntp_on."

	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2

	echo "ntp_client $1 $3"

	pkill -9 ntp_client
	ntp_client "$1" "$3" &
}

handle_e_p3k_ntp_off()
{
	echo "handle_e_p3k_ntp_off."

	echo "pkill -9 ntp_client"

	pkill -9 ntp_client
}

handle_e_p3k_ntp()
{
	echo "handle_e_p3k_ntp."

	case "$event" in
		e_p3k_ntp_enable_on?*)
			handle_e_p3k_ntp_on "$event"
		;;
		e_p3k_ntp_enable_off)
			handle_e_p3k_ntp_off
		;;
		*)
			echo "error param"
		;;
	esac
}

handle_e_soip_param()
{
	echo "handle_e_soip_param."
	local _para1

	#e_p3k_soip_param::param
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	astparam s s0_baudrate $_para1
	astparam save

	ipc @s_lm_set s se_start:$CH_SELECT_S


}

handle_e_soip_gw_on()
{
	echo "handle_e_soip_gw_on."
	local _para1

	#e_p3k_soip_gw_on::port
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	SOIP_GUEST_ON='y'
	update_node_info SOIP_GUEST_ON $SOIP_GUEST_ON
	astparam s soip_guest_on y
	astparam s soip_type 2
	astparam s soip_port $_para1
	astparam save

	ipc @s_lm_set s se_start:$CH_SELECT_S

}

handle_e_soip_gw_off()
{
	echo "handle_e_soip_gw_off."

	SOIP_GUEST_ON='n'
	update_node_info SOIP_GUEST_ON $SOIP_GUEST_ON
	astparam s soip_guest_on n
	astparam save

	ipc @s_lm_set s se_start:$CH_SELECT_S


}

handle_e_p3k_soip()
{
	echo "handle_e_p3k_soip."
	case "$event" in
		e_p3k_soip_param?*)
			handle_e_soip_param "$event"
		;;
		e_p3k_soip_gw_on?*)
			handle_e_soip_gw_on "$event"
		;;
		e_p3k_soip_gw_off)
			handle_e_soip_gw_off
		;;
		*)
			echo "error param"
		;;
	esac

}

handle_e_p3k_reset()
{
	echo "handle_e_p3k_reset."

	sleep 5

	echo "handle_e_p3k_reset.reboot"
	reboot
}

handle_e_p3k()
{
	echo "handle_e_p3k."
	case "$*" in
		e_p3k_switch_?*)
			handle_e_p3k_switch "$event"
		;;
		e_p3k_video_?*)
			handle_e_p3k_video "$event"
		;;
		e_p3k_audio_?*)
			handle_e_p3k_audio "$event"
		;;
		e_p3k_ir_?*)
			handle_e_p3k_ir "$event"
		;;
		e_p3k_cec_?*)
			handle_e_p3k_cec "$event"
		;;
		e_p3k_net_?*)
			handle_e_p3k_net "$event"
		;;
		e_p3k_upgrade_fw*)
			handle_e_p3k_upgrade
		;;
		e_p3k_fp_lock_?*)
			handle_e_p3k_fp_lock "$event"
		;;
		e_p3k_flag_me)
			handle_e_p3k_flagme
		;;
		e_p3k_flag_me_timeout)
			handle_e_p3k_flagme_timeout
		;;
		e_p3k_download_fw_start)
			handle_e_p3k_download_fw_start
		;;
		e_p3k_download_fw_stop)
			handle_e_p3k_download_fw_stop
		;;
		e_p3k_ntp_?*)
			handle_e_p3k_ntp "$event"
		;;
		e_p3k_soip_?*)
			handle_e_p3k_soip "$event"
		;;
		e_p3k_reset)
			handle_e_p3k_reset
		;;
		*)
		;;
	esac
}

handle_e_ipe5000w_led_chose()
{
	case "$*" in
		e_ipe5000w_led_hdmi)
			IPE5000W_INPUT_SOURCE_LED="hdmi"
		;;
		e_ipe5000w_led_typec)
			IPE5000W_INPUT_SOURCE_LED="typec"
		;;
		*)
		;;
	esac
}

handle_e_cec_cmd_report()
{
	local _para1

	_para1=${event#*::}

	echo "cec_cmd=$_para1"
	p3k_notify cec_msg::$_para1
}

handle_e_analog_control()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2

	if [ $1 = '0' ];then
		echo 0 > /sys/devices/platform/1500_i2s/analog_out_vol
	else
		echo 1 > /sys/class/leds/lineout_mute/brightness
		echo $analog_out_volum > /sys/devices/platform/1500_i2s/analog_out_vol
	fi
}

handle_e_set_ttl()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	#After modifying this value, ping our IP TTL will change to our modified value
	echo $1 > /proc/sys/net/ipv4/ip_default_ttl
}

#e e_audio_detect_time::10
handle_e_audio_detect_time()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2

	kill_process audio_detect
	audio_detect -t $1 &
}

state_machine()
{
	# Bruce160308. Try to ignore all TERM signals.
	trap signal_handler SIGTERM SIGPIPE SIGHUP SIGINT SIGALRM SIGUSR1 SIGUSR2 SIGPROF SIGVTALRM

	start_network 2
	do_s_init

	while true; do
		event=`lm_get_event 2>/dev/null`
		echo "Receive $event event on $STATE state "`cat /proc/uptime`
		case "$event" in
			tick)
				sleep 0.001
				tickle_watchdog
				sleep 0.001
			;;
			e_reconnect*)
				handle_e_reconnect "$event"
			;;
			e_button_?*)
				handle_e_button "$event"
			;;
			e_key_?*)
				handle_e_key "$event"
			;;
			e_p3k_?*)
				handle_e_p3k "$event"
			;;
			e_ip_got::?*)
				handle_e_ip_got "$event"
			;;
			e_vw_?*)
				handle_e_vw "$event"
			;;
			# Bruce130110. shell 'case' will see 'r1_r1c2_' as rXcX case. Which is wrong.
			#r?*c?*_?*)
			#	handle_rXcX "$event"
			#;;
			# A7 to be removed.
			#r[0-9x]*_?*)
			#	if echo "$event" | grep -q -e "^r[[:digit:]x]\{1,\}c[[:digit:]x]\{1,\}_.*" ; then
			#		handle_rXcX "$event"
			#	else
			#		handle_RIDX "$event"
			#	fi
			#;;
			e_sh_?*)
				handle_e_sh "$event"
			;;
			callback=*)
				handle_e_sh_jsonp "$event"
			;;
			e_btn_?*)
				handle_"$event" "$event"
			;;
			e_var_dump::?*::?*)
				sleep 0.001
				handle_e_var_dump "$event"
			;;
			e_var_get::?*)
				sleep 0.001
				handle_e_var_get "$event"
			;;
			e_var_set::?*::?*)
				sleep 0.001
				handle_e_var_set "$event"
			;;
			e_pwr_status_changed::?*)
				handle_e_pwr_status_changed "$event"
			;;
			e_osd?*)
				# host do nothing, this is for ir decode osd
			;;
			e_ir_decoded*)
				handle_e_ir_decoded "$event"
			;;
			e_video_stat_?*)
				handle_e_video_stat "$event"
			;;
			e_chg_hostname*)
				handle_e_chg_hostname "$event"
			;;
			e_cec_cmd_report*)
				handle_e_cec_cmd_report "$event"
			;;
			e_ipe5000w_led*)
				handle_e_ipe5000w_led_chose "$event"
			;;
			e_analog_control*)
				handle_e_analog_control "$event"
			;;
			e_set_ttl*)
				handle_e_set_ttl "$event"
			;;
			e_log*)
				handle_e_log "$event"
			;;
			e_audio_detect_time*)
				handle_e_audio_detect_time "$event"
			;;
			e_?*)
				tickle_watchdog
				handle_"$event"
			;;
			*)
				echo "ERROR!!!! Invalid event ($event) received"
				# Bruce160308. If the $event is an empty string, is should be some kind of error when reading from event pipe.
				# Ignoring this event, not ack the pipe should be good to go.
			;;
		esac
		#echo "End of $event event on $STATE state "`cat /proc/uptime`
	done
}

init_param_from_flash()
{
	init_share_param_from_flash

	LOOPBACK_DEFAULT_ON=`astparam g loopback_default_on`
	if echo "$LOOPBACK_DEFAULT_ON" | grep -q "not defined" ; then
		LOOPBACK_DEFAULT_ON=`astparam r loopback_default_on`
		if echo "$LOOPBACK_DEFAULT_ON" | grep -q "not defined" ; then
			LOOPBACK_DEFAULT_ON='y'
		fi
	fi
	echo "loopback_default_on: $LOOPBACK_DEFAULT_ON"

	LOOPBACK_EDID_PATCH=`astparam g loopback_edid_patch`
	if echo "$LOOPBACK_EDID_PATCH" | grep -q "not defined" ; then
		LOOPBACK_EDID_PATCH=`astparam r loopback_edid_patch`
		if echo "$LOOPBACK_EDID_PATCH" | grep -q "not defined" ; then
			if [ "$SOC_OP_MODE" = '1' ]; then
				LOOPBACK_EDID_PATCH='00000000'
			else
				LOOPBACK_EDID_PATCH='00000000'
			fi
		fi
	fi
	echo "LOOPBACK_EDID_PATCH=$LOOPBACK_EDID_PATCH"

	REMOTE_EDID_PATCH=`astparam g remote_edid_patch`
	if echo "$REMOTE_EDID_PATCH" | grep -q "not defined" ; then
		REMOTE_EDID_PATCH=`astparam r remote_edid_patch`
		if echo "$REMOTE_EDID_PATCH" | grep -q "not defined" ; then
			case "$SOC_OP_MODE" in
			1)
				REMOTE_EDID_PATCH='00230017'
			;;
			2)
				REMOTE_EDID_PATCH='00000005'
			;;
			3)
				REMOTE_EDID_PATCH='00000000'
			;;
			*)
				REMOTE_EDID_PATCH='00000001'
			;;
			esac
		fi
	fi
	echo "REMOTE_EDID_PATCH=$REMOTE_EDID_PATCH"

#	DEFAULT_KSV=`astparam g default_ksv`
#	if echo "$DEFAULT_KSV" | grep -q "not defined" ; then
#		DEFAULT_KSV=`astparam r default_ksv`
#		if echo "$DEFAULT_KSV" | grep -q "not defined" ; then
#			DEFAULT_KSV='unavailable'
#		fi
#	fi

	HDCP2_DCP_PUBLIC_KEY=`astparam g hdcp2_dcp_public_key`
	if echo "$HDCP2_DCP_PUBLIC_KEY" | grep -q "not defined" ; then
		HDCP2_DCP_PUBLIC_KEY=`astparam r hdcp2_dcp_public_key`
		if echo "$HDCP2_DCP_PUBLIC_KEY" | grep -q "not defined" ; then
			HDCP2_DCP_PUBLIC_KEY='unavailable'
		fi
	fi

	# Bruce120712. NO_USB should act as the main ON/OFF switch
	#if [ "$SHARE_USB" = 'y' ]; then
	#	echo "Host force NO_USB=n under SHARE_USB mode"
	#	NO_USB='n'
	#fi

	USB_SET_ADDR_HACK=`astparam g usb_set_addr_hack`
	if echo "$USB_SET_ADDR_HACK" | grep -q "not defined" ; then
		USB_SET_ADDR_HACK=`astparam r usb_set_addr_hack`
		if echo "$USB_SET_ADDR_HACK" | grep -q "not defined" ; then
			USB_SET_ADDR_HACK='0'
		fi
	fi

	USB_HID_URB_INTERVAL=`astparam g usb_hid_urb_interval`
	if echo "$USB_HID_URB_INTERVAL" | grep -q "not defined" ; then
		USB_HID_URB_INTERVAL=`astparam r usb_hid_urb_interval`
		if echo "$USB_HID_URB_INTERVAL" | grep -q "not defined" ; then
			#USB_HID_URB_INTERVAL='35' # Set to 35 to resolve some USB HID long latency issue
			USB_HID_URB_INTERVAL='0'
		fi
	fi

	USB_QUIRK=`astparam g usb_quirk`
	if echo "$USB_QUIRK" | grep -q "not defined" ; then
		USB_QUIRK=`astparam r usb_quirk`
		if echo "$USB_QUIRK" | grep -q "not defined" ; then
			# VHUB_QUIRK_BULK_SSPLIT (0x1UL << 0)
			USB_QUIRK='0'
		fi
	fi

	V_RX_DRV=`astparam g v_rx_drv`
	if echo "$V_RX_DRV" | grep -q "not defined" ; then
		V_RX_DRV=`astparam r v_rx_drv`
		if echo "$V_RX_DRV" | grep -q "not defined" ; then
			if [ "$SOC_VER" = '1' ] || [ "$SOC_VER" = '2' ]; then
				# FIXME. This is actually platform dependent
				V_RX_DRV='cat6023'
			else
				case "$BOARD_REVISION" in
				31*)
					V_RX_DRV='it6805'
				;;
				*)
					V_RX_DRV='it6802'
				;;
				esac
			fi
		fi
	fi
	# Update $HDMIRX_SYS_PATH
	HDMIRX_SYS_PATH="/sys/devices/platform/$V_RX_DRV"

	V_FRAME_RATE=`astparam g v_frame_rate`
	if echo "$V_FRAME_RATE" | grep -q "not defined" ; then
		V_FRAME_RATE=`astparam r v_frame_rate`
		if echo "$V_FRAME_RATE" | grep -q "not defined" ; then
			#V_FRAME_RATE='0' means off. Range: 0~60
			V_FRAME_RATE='0'
		fi
	fi

	V_ANALOG_EDGE_DETECT=`astparam g v_analog_edge_detect`
	if echo "$V_ANALOG_EDGE_DETECT" | grep -q "not defined" ; then
		V_ANALOG_EDGE_DETECT=`astparam r v_analog_edge_detect`
		if echo "$V_ANALOG_EDGE_DETECT" | grep -q "not defined" ; then
			#V_ANALOG_EDGE_DETECT Range: 0~255
			V_ANALOG_EDGE_DETECT='0x25'
		fi
	fi

	V_INPUT_SELECT=`astparam g v_input_select`
	if echo "$V_INPUT_SELECT" | grep -q "not defined" ; then
		V_INPUT_SELECT=`astparam r v_input_select`
		if echo "$V_INPUT_SELECT" | grep -q "not defined" ; then
			# Options: auto, detect_sync, fixed
			V_INPUT_SELECT='auto'
		fi
	fi

	V_DUAL_INPUT=`astparam g v_dual_input`
	if echo "$V_DUAL_INPUT" | grep -q "not defined" ; then
		V_DUAL_INPUT=`astparam r v_dual_input`
		if echo "$V_DUAL_INPUT" | grep -q "not defined" ; then
			V_DUAL_INPUT='undefined'
		fi
	fi

	LED_DISPLAY_CH_SELECT_V=`astparam g ch_select`
	if echo "$LED_DISPLAY_CH_SELECT_V" | grep -q "not defined" ; then
		LED_DISPLAY_CH_SELECT_V=`astparam r ch_select`
		if echo "$LED_DISPLAY_CH_SELECT_V" | grep -q "not defined" ; then
			LED_DISPLAY_CH_SELECT_V='001'
		fi
	fi

	astparam s network_card
	astparam s tv_access
	#Before the last power failure, it is possible to call save to clear these variables
	astparam s fir_priority_board_status
	astparam s sec_priority_board_status
	astparam s third_priority_board_status
	astparam s fourth_priority_board_status
	astparam s repeat_board_lighting_flag
	astparam s fir_priority_net_status
	astparam s sec_priority_net_status
	astparam s repeat_net_lighting_flag

	astparam s fourth_priority_board_status $POWER_ON
	astparam s sec_priority_net_status $GET_IP_FAIL
	astparam s repeat_net_lighting_flag 0
	astparam s repeat_board_lighting_flag 0
	# Print the final parameters
	echo_parameters
}

init_param_from_p3k_cfg()
{
	if [ -f "$device_setting" ];then
		P3KCFG_FP_LOCK_ON=`jq -r '.device_setting.fp_lock' $device_setting`
		if echo "$P3KCFG_FP_LOCK_ON" | grep -q "null" ; then
			P3KCFG_FP_LOCK_ON='off'
		fi
	else
		P3KCFG_FP_LOCK_ON='off'
	fi
	echo "P3KCFG_FP_LOCK_ON=$P3KCFG_FP_LOCK_ON"

	if [ -f "$edid_setting" ];then
		P3KCFG_EDID_MODE=`jq -r '.edid_setting.edid_mode' $edid_setting`
		if echo "$P3KCFG_EDID_MODE" | grep -q "null" ; then
			P3KCFG_EDID_MODE='default'
		fi

		P3KCFG_EDID_ACTIVE=`jq -r '.edid_setting.active_edid' $edid_setting`
		if echo "$P3KCFG_EDID_ACTIVE" | grep -q "null" ; then
			P3KCFG_EDID_ACTIVE='0'
		fi

		P3KCFG_EDID_NET_SRC=`jq -r '.edid_setting.net_src' $edid_setting`
		if echo "$P3KCFG_EDID_NET_SRC" | grep -q "null" ; then
			P3KCFG_EDID_NET_SRC='0.0.0.0'
		fi
	else
		P3KCFG_EDID_MODE='default'
		P3KCFG_EDID_ACTIVE='0'
		P3KCFG_EDID_NET_SRC='0.0.0.0'
	fi
	echo "P3KCFG_EDID_MODE=$P3KCFG_EDID_MODE"
	echo "P3KCFG_EDID_ACTIVE=$P3KCFG_EDID_ACTIVE"
	echo "P3KCFG_EDID_NET_SRC=$P3KCFG_EDID_NET_SRC"

	if [ -f "$av_setting" ];then
		P3KCFG_HDCP_1_ON=`jq -r '.av_setting.hdcp_mode.hdmi_in1' $av_setting`
		if echo "$P3KCFG_HDCP_1_ON" | grep -q "null" ; then
			P3KCFG_HDCP_1_ON='on'
		fi

		P3KCFG_HDCP_2_ON=`jq -r '.av_setting.hdcp_mode.hdmi_in2' $av_setting`
		if echo "$P3KCFG_HDCP_2_ON" | grep -q "null" ; then
			P3KCFG_HDCP_2_ON='on'
		fi

		P3KCFG_HDCP_3_ON=`jq -r '.av_setting.hdcp_mode.usb_in3' $av_setting`
		if echo "$P3KCFG_HDCP_3_ON" | grep -q "null" ; then
			P3KCFG_HDCP_3_ON='on'
		fi

		P3KCFG_AV_MUTE=`jq -r '.av_setting.mute' $av_setting`
		if echo "$P3KCFG_AV_MUTE" | grep -q "null" ; then
			P3KCFG_AV_MUTE='off'
		fi

		P3KCFG_AV_ACTION=`jq -r '.av_setting.action' $av_setting`
		if echo "$P3KCFG_AV_ACTION" | grep -q "null" ; then
			P3KCFG_AV_ACTION='play'
		fi

		P3KCFG_ANAOUT_VOLUME=`jq -r '.av_setting.volume' $av_setting`
		if echo "$P3KCFG_ANAOUT_VOLUME" | grep -q "null" ; then
			P3KCFG_ANAOUT_VOLUME='80'
		fi
	else
		P3KCFG_HDCP_1_ON='on'
		P3KCFG_HDCP_2_ON='on'
		P3KCFG_HDCP_3_ON='on'
		P3KCFG_AV_MUTE='off'
		P3KCFG_AV_ACTION='play'
		P3KCFG_ANAOUT_VOLUME='80'
	fi
	analog_out_volum=$P3KCFG_ANAOUT_VOLUME
	echo "P3KCFG_HDCP_1_ON=$P3KCFG_HDCP_1_ON"
	echo "P3KCFG_HDCP_2_ON=$P3KCFG_HDCP_2_ON"
	echo "P3KCFG_HDCP_3_ON=$P3KCFG_HDCP_3_ON"
	echo "P3KCFG_AV_MUTE=$P3KCFG_AV_MUTE"
	echo "P3KCFG_AV_ACTION=$P3KCFG_AV_ACTION"
	echo "P3KCFG_ANAOUT_VOLUME=$P3KCFG_ANAOUT_VOLUME"

	if [ -f "$gateway_setting" ];then
		P3KCFG_IR_DIR=`jq -r '.gateway.ir_direction' $gateway_setting`
		if echo "$P3KCFG_IR_DIR" | grep -q "null" ; then
			P3KCFG_IR_DIR='out'
		fi

		CEC_GATWAY=`jq -r '.gateway.cec_mode' $gateway_setting`
		if echo "CEC_GATWAY" | grep -q "null" ; then
			CEC_GATWAY='on'
		fi

		P3KCFG_CEC_DIR=`jq -r '.gateway.cec_destination' $gateway_setting`

		echo "P3KCFG_CEC_DIR=$P3KCFG_CEC_DIR"

		if echo "P3KCFG_CEC_DIR" | grep -q "null" ; then
			CEC_SEND_DIR='hdmi_in'
		elif [ "$P3KCFG_CEC_DIR" = 'hdmi_out' ]; then
			CEC_SEND_DIR='hdmi_out'
		elif [ "$P3KCFG_CEC_DIR" = 'hdmi_loop' ]; then
			CEC_SEND_DIR='hdmi_out'
		else
			CEC_SEND_DIR='hdmi_in'
		fi
	else
		P3KCFG_IR_DIR='out'
		CEC_GATWAY='on'
		CEC_SEND_DIR='hdmi_in'
	fi
	echo "P3KCFG_IR_DIR=$P3KCFG_IR_DIR"
	echo "CEC_GATWAY=$CEC_GATWAY"
	echo "CEC_SEND_DIR=$CEC_SEND_DIR"

	if [ -f "$network_setting" ];then
		P3KCFG_TTL=`jq -r '.network_setting.multicast.ttl' $network_setting`
		if echo "$P3KCFG_TTL" | grep -q "null" ; then
			P3KCFG_TTL='64'
		fi

		P3KCFG_CONTROL_MODE=`jq -r '.network_setting.control.mode' $network_setting`
		if echo "$P3KCFG_CONTROL_MODE" | grep -q "null" ; then
			P3KCFG_CONTROL_MODE='dhcp'
		fi

		P3KCFG_CONTROL_IP=`jq -r '.network_setting.control.ip_address' $network_setting`
		if echo "$P3KCFG_CONTROL_IP" | grep -q "null" ; then
			P3KCFG_CONTROL_IP='0.0.0.0'
		fi

		P3KCFG_CONTROL_MASK=`jq -r '.network_setting.control.mask_address' $network_setting`
		if echo "$P3KCFG_CONTROL_MASK" | grep -q "null" ; then
			P3KCFG_CONTROL_MASK='255.255.0.0'
		fi

		P3KCFG_CONTROL_PORT=`jq -r '.network_setting.port_setting.control.port' $network_setting`
		if echo "$P3KCFG_CONTROL_PORT" | grep -q "null" ; then
			P3KCFG_CONTROL_PORT='eth0'
		fi

		P3KCFG_CONTROL_VLAN=`jq -r '.network_setting.port_setting.control.vlan_tag' $network_setting`
		if echo "$P3KCFG_CONTROL_VLAN" | grep -q "null" ; then
			P3KCFG_CONTROL_VLAN='1'
		fi

		case "$MODEL_NUMBER" in
			KDS-SW3-EN7)
				P3KCFG_DANTE_PORT=`jq -r '.network_setting.port_setting.dante.port' $network_setting`
				if echo "$P3KCFG_DANTE_PORT" | grep -q "null" ; then
					P3KCFG_DANTE_PORT='eth0'
				fi

				P3KCFG_DANTE_VLAN=`jq -r '.network_setting.port_setting.dante.vlan_tag' $network_setting`
				if echo "$P3KCFG_DANTE_VLAN" | grep -q "null" ; then
					P3KCFG_DANTE_VLAN='1'
				fi
			;;
			*)
			;;
		esac
	else
		P3KCFG_TTL='64'
			P3KCFG_CONTROL_MODE='dhcp'
			P3KCFG_CONTROL_IP='0.0.0.0'
			P3KCFG_CONTROL_MASK='255.255.0.0'
			P3KCFG_CONTROL_PORT='eth0'
			P3KCFG_CONTROL_VLAN='1'

		case "$MODEL_NUMBER" in
			KDS-SW3-EN7)
				P3KCFG_DANTE_PORT='eth0'
				P3KCFG_DANTE_VLAN='1'
			;;
			*)
			;;
		esac
	fi
	echo "P3KCFG_TTL=$P3KCFG_TTL"
	echo "P3KCFG_CONTROL_MODE=$P3KCFG_CONTROL_MODE"
	echo "P3KCFG_CONTROL_IP=$P3KCFG_CONTROL_IP"
	echo "P3KCFG_CONTROL_MASK=$P3KCFG_CONTROL_MASK"
	echo "P3KCFG_CONTROL_PORT=$P3KCFG_CONTROL_PORT"
	echo "P3KCFG_CONTROL_VLAN=$P3KCFG_CONTROL_VLAN"

	case "$MODEL_NUMBER" in
		KDS-SW3-EN7)
			echo "P3KCFG_DANTE_PORT=$P3KCFG_DANTE_PORT"
			echo "P3KCFG_DANTE_VLAN=$P3KCFG_DANTE_VLAN"
		;;
		*)
		;;
	esac

	if [ -f "$time_setting" ];then
		P3KCFG_NTP_SRV_MODE=`jq -r '.time_setting.ntp_server.mode' $time_setting`
		P3KCFG_NTP_SRV_ADDR=`jq -r '.time_setting.ntp_server.ip_address' $time_setting`
		P3KCFG_NTP_SYNC_HOUR=`jq -r '.time_setting.ntp_server.daily_sync_hour' $time_setting`
	else
		P3KCFG_NTP_SRV_MODE='off'
		P3KCFG_NTP_SRV_ADDR='0.0.0.0'
		P3KCFG_NTP_SYNC_HOUR='0'
	fi
	echo "P3KCFG_NTP_SRV_MODE=$P3KCFG_NTP_SRV_MODE"
	echo "P3KCFG_NTP_SRV_ADDR=$P3KCFG_NTP_SRV_ADDR"
	echo "P3KCFG_NTP_SYNC_HOUR=$P3KCFG_NTP_SYNC_HOUR"

	if [ -f "$av_signal" ];then
		P3KCFG_GUARD_TIME=`jq -r '.av_signal.audio_connection_guard_time_sec' $av_signal`
		if echo "$P3KCFG_GUARD_TIME" | grep -q "null" ; then
			P3KCFG_GUARD_TIME='10'
		fi
	else
		P3KCFG_GUARD_TIME='10'
	fi
	echo "P3KCFG_GUARD_TIME=$P3KCFG_GUARD_TIME"

	if [ -f "$audio_setting" ];then
		P3KCFG_ANALOG_DIR=`jq -r '.audio_setting.analog_direction' $audio_setting`
		if echo "$P3KCFG_ANALOG_DIR" | grep -q "null" ; then
			P3KCFG_ANALOG_DIR='in'
		fi
	else
		P3KCFG_ANALOG_DIR='in'
	fi
	echo "P3KCFG_ANALOG_DIR=$P3KCFG_ANALOG_DIR"
}

set_hdcp_status()
{
	case "$MODEL_NUMBER" in
		KDS-EN7)
			if [ $P3KCFG_HDCP_1_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:0:3
			else
				ipc @m_lm_set s set_hdcp_cap:0:0
			fi
		;;
		KDS-SW3-EN7)
			if [ $P3KCFG_HDCP_1_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:0:3
			else
				ipc @m_lm_set s set_hdcp_cap:0:0
			fi
			if [ $P3KCFG_HDCP_2_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:1:3
			else
				ipc @m_lm_set s set_hdcp_cap:1:0
			fi
			if [ $P3KCFG_HDCP_3_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:2:3
			else
				ipc @m_lm_set s set_hdcp_cap:2:0
			fi
		;;
		WP-SW2-EN7)
			if [ $P3KCFG_HDCP_1_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:0:3
			else
				ipc @m_lm_set s set_hdcp_cap:0:0
			fi
			if [ $P3KCFG_HDCP_2_ON = 'on' ];then
				ipc @m_lm_set s set_hdcp_cap:1:3
			else
				ipc @m_lm_set s set_hdcp_cap:1:0
			fi
		;;
		*)
		;;
	esac
}

set_variable_power_on_status()
{
	set_hdcp_status

	if [ $P3KCFG_AV_MUTE = 'off' ];then
		ipc @m_lm_set s set_hdmi_mute:16:1:0
		if [ $MODEL_NUMBER = 'KDS-SW3-EN7' ];then
			ipc @m_lm_set s set_hdmi_mute:17:1:0
			echo 0 > /sys/class/leds/dante_mute/brightness
		fi
	else
		ipc @m_lm_set s set_hdmi_mute:16:1:1
		if [ $MODEL_NUMBER = 'KDS-SW3-EN7' ];then
			ipc @m_lm_set s set_hdmi_mute:17:1:1
			echo 1 > /sys/class/leds/dante_mute/brightness
		fi
	fi

	echo $P3KCFG_TTL > /proc/sys/net/ipv4/ip_default_ttl
}

clear_unused_files()
{
	rm -rf /sbin/e2fsck
	rm -rf /usr/share/ip5000_a30
	rm -rf /usr/share/ipe5000w_a30
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! LM received signal!!!!!! Ignore it."
	echo ""
	echo ""
	echo ""
}

#set -x
#### main #####
export PATH="${PATH}":/usr/local/bin
cd /usr/local/bin
. ./include.sh
. bash/utilities.sh
. ./ir_decode.sh
# Used to patch link_mgrX.sh itself.
if [ -f ./patch_lmh.sh ]; then
	. ./patch_lmh.sh
fi

#qzx 2021.11.4:It's useless to turn off the watchdog. If the MCU needs to be upgraded, it will lead to restart
#init_watchdog
echo no > /sys/devices/platform/watchdog/enable
#mknod $PIPE_INFO_LOCAL p    TBD
#mknod $PIPE_INFO_REMOTE p   TBD
#qzx 2021.12.3:Delete the /sbin/reboot command and let it call /usr/local/bin/reboot default
remove_sbin_reboot


ifconfig lo up

#init_p3k_cfg_file
init_p3k_cfg_file

# initialize parameters
init_param_from_flash

init_time_zone

# overwrite parameters from pssed in parameters
while [ -n "$1" ]; do
	if [ "$1" = "no_video" ]; then
		echo "NO_VIDEO"
		NO_VIDEO='y'
	elif [ "$1" = "no_usb" ]; then
		echo "NO_USB"
		NO_USB='y'
	elif [ "$1" = "no_i2s" ]; then
		echo "NO_I2S"
		NO_I2S='y'
	elif [ "$1" = "no_ir" ]; then
		echo "NO_IR"
		NO_IR='y'
	elif [ "$1" = "debug" ]; then
		echo "DBG"
		DBG='1'
		set -x
	fi
	shift 1
done
init_version_file
init_info_file
#init_p3k_cfg_file
init_param_from_p3k_cfg
init_json_cfg_path
init_p3k_net_vlan
# $AST_PLATFORM = ast1500hv4 or ptv1500hv2 or pce1500hv3
echo ""
echo "#### platform info:$AST_PLATFORM ####"
if [ -z "$AST_PLATFORM" ]; then
	echo "ERROR! no AST_PLATFORM info."
	exit 1;
fi

handle_button_on_boot

#ipe5000p -- Check whether the typec chip needs to be upgraded
#if [ $MODEL_NUMBER = 'KDS-SW3-EN7' ];then
	#typec_update.sh
#fi

case "$MODEL_NUMBER" in
	WP-SW2-EN7)
		if communication_with_mcu -u -b 4; then
			UGP_FLAG="success"
		else
			UGP_FLAG="fail"
		fi
	;;
	KDS-SW3-EN7 | KDS-EN7)
		if communication_with_mcu -u -b 5; then
			UGP_FLAG="success"
		else
			UGP_FLAG="fail"
		fi
	;;
	*)
		echo "Please set MODEL_NUMBER parameters >_<"
	;;
esac

if [ $UGP_FLAG = 'success' ];then
	echo "lock file for @m_lm_query" > /var/lock/@m_lm_query.lck
	ipc_server_listen_one @m_lm_set @m_lm_get @m_lm_query @m_lm_reply &
	usleep 1000
	case "$MODEL_NUMBER" in
		#-b Indicates the board type:0-IPE5000;1-IPE5000P;4-IPE5000W;
		KDS-EN7)
			communication_with_mcu -c -b 0 &
		;;
		KDS-SW3-EN7)
			communication_with_mcu -c -b 1 &
		;;
		WP-SW2-EN7)
			communication_with_mcu -c -b 4 &
		;;
		*)
		;;
	esac
fi

if [ $UGP_FLAG = 'success' ];then
	start_time=$(date +%s)
	while [ ! -f "/tmp/socket_ready" ];
	do
		end_time=$(date +%s)
		time_diff=$(( $end_time - $start_time ))
		if [ $time_diff -ge 2 ];then
			UGP_FLAG="fail"
			break
		fi
	done
	if [ -f "/tmp/socket_ready" ];then
		rm /tmp/socket_ready
	fi
fi

if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-SW3-EN7)
			mainswitch &
		;;
		WP-SW2-EN7)
			mainswitch &
		;;
		*)
		;;
	esac
fi

#configure the default configuration of MCU pin
if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-EN7)
			ipc @m_lm_set s powerup_cec_report:1
			#set lineio_sel(70) pin to default to line_out;0:line_out;1:line_in
			#set i2s_sel(72) pin to default to 6802;0:ast1520 ; 1:it6802
			#set lcd_power(65) pin to default to no useful,it is useful in ast1520
			#set type_b or type_c(66) pin to default to type_b;0:type_c ; 1:type_b
			#set cec_sel(68) pin to default to it6802;0:hdmi_out-hdmi_in; 1:it6802-hdmi_in
			ipc @m_lm_set s set_gpio_config:4:70:1:65:1:72:1:68:1
			ipc @m_lm_set s set_gpio_val:4:70:1:65:0:72:1:68:1
		;;
		KDS-SW3-EN7)
			ipc @m_lm_set s powerup_cec_report:1
			#set lineio_sel pin to default to line_out;0:line_out;1:line_in
			ipc @m_lm_set s set_gpio_config:4:70:1:65:1:66:1:78:0
			ipc @m_lm_set s set_gpio_val:3:70:0:65:0:66:0
			ipc @m_lm_set s set_gpio_config:9:15:1:35:1:8:1:36:1:37:1:32:1:33:1:11:1:12:1
			ipc @m_lm_set s set_gpio_val:9:15:1:35:1:8:1:36:1:37:1:32:1:33:1:11:1:12:1
			#set cec_switch2(67) pin to default to it6802_cec;0:hdmiout-hdmi_in;1:it6802-hdmi_in
			#cec_switch0(68) and cec_swicth1(69):0/0 - hdmi1;1/0 - hdmi2;0/1 - typec
			#HUB_VBUSM(1) and USB_5V_EN(73):The levels of these two pins can be consistent at present
			ipc @m_lm_set s set_gpio_config:5:67:1:68:1:69:1:1:1:73:1
			ipc @m_lm_set s set_gpio_val:5:67:1:68:0:69:0:1:0:73:0
		;;
		WP-SW2-EN7)
			#enable led_display	set_lcd_control--cmd;0--led_type 1--lcd_type;1--enable  0--disenable;
			#set cec_switch(77) pin to default to hdmi_in;0:hdmi_in - it6802;typec - it6802;
			#Pull up this pin to indicate that the power on is complete
			echo 1 > /sys/class/leds/i2c_mux_gpio/brightness
			ipc @m_lm_set s set_gpio_config:1:77:1
			ipc @m_lm_set s set_gpio_val:1:77:0
			if [ $P3KCFG_FP_LOCK_ON = 'off' ];then
				ipc @m_lm_set s set_lcd_control:0:1
				led_display_num
			else
				ipc @m_lm_set s set_lcd_control:0:0
			fi
		;;
		*)
		;;
	esac
fi

#Initialize I2C bus3(temperature sensor)
init_temperature_sensor

#configuration function power on timing
if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-EN7)
			set_variable_power_on_status
		;;
		KDS-SW3-EN7)
			set_variable_power_on_status
		;;
		WP-SW2-EN7)
			set_variable_power_on_status
			ipe5000w_led_tmp_define
		;;
		*)
		;;
	esac
fi

# start event_monitor
ast_event_monitor &
EM_PID=$!

STATE='s_init'
echo $STATE > $DEVICE_STATUS_FILE
update_node_info STATE $STATE

# Disable OOM Killer
echo 1 > /proc/sys/vm/overcommit_memory

clear_unused_files

# Start state machine in another process scope
state_machine &

# Bruce130123. Moved to state_machine. Avoid parameter scope problem.
#start_network 2
#do_s_init

# A7 to be removed. I stopped watchdog for development A7
# disable watchdog after 'sys_init_ok', let system reboot if it cannot get IP address from DHCP
#disable_watchdog
