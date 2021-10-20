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
rx_tcp_port='8888'

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
	if [ "$NO_CEC" = 'n' ]; then
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
		disable_watchdog

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

		do_ready_to_go

		set_igmp_leave_force
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

	avahi-daemon -k
	pkill -9 heartbeat
	pkill -9 node_responser
	pkill -9 name_service
	#pkill -9 httpd
	pkill -9 lcd_display
	pkill -9 web
	pkill -9 telnetd
	pkill -9 ldap_daemon
	pkill -9 ieee802dot1x
	pkill -9 wpa
	pkill -9 p3ktcp

	avahi-daemon -D
	name_service -thost
	#httpd -h /www &
	start_telnetd

	update_node_info

	node_responser --mac $MY_MAC &
	heartbeat &
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
	/sbin/avahi-daemon -k 2>/dev/null

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

	/sbin/avahi-daemon -D 2>/dev/null

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

		avahi-daemon -D
		name_service -thost
		#httpd -h /www &
		# Start telnetd
		start_telnetd
		p3ktcp &
		usleep 10000
		web &
		
		if [ "$P3KCFG_AV_ACTION" = 'stop' ];then
			e e_stop_link
		fi
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
			{ avahi-daemon -k 2>/dev/null; sleep 1; avahi-daemon -D; } &

			# Eth link mode may change. Should update profile based on new Eth link mode.
			apply_profile_config `select_v_input refresh`
			if [ "$ACCESS_ON" = 'y' ]; then
				handle_e_reconnect_refresh
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
		echo "$_type" > $I2S_SYS_PATH/io_select
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
		;;
		e_key_up_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
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
		;;
		e_key_down_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
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
		;;
		e_key_left_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
				return
			fi
			send_key_info 4
		;;
		e_key_right_pressed)
			if [ "$MODEL_NUMBER" = 'WP-SW2-EN7' ];then
				return
			fi
			send_key_info 5
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
	_pri_3="$1"

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
		e_p3k_video_edid_default?*)
			handle_edid -s default
		;;
		e_p3k_video_edid_passthru::?*)
			echo "e_p3k_video_edid_passthru ($event) received"
			shift 2
			_para1="$1"
			handle_edid -c $_para1
		;;
		e_p3k_video_edid_custom::?*)
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
			ipc @m_lm_set s set_hdcp_cap:0-2:$_para2
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

	echo "set p3k switch input!!! $_switch_input"
	case "$MODEL_NUMBER" in
		KDS-EN7)
			case "$_switch_input" in
				hdmi)
					ipc @m_lm_set s set_gpio_val:1:72:1
					echo hdmi > /sys/devices/platform/1500_i2s/io_select
					echo out_analog > /sys/devices/platform/1500_i2s/io_select
				;;
				analog)
					ipc @m_lm_set s set_gpio_val:1:72:0
					echo analog > /sys/devices/platform/1500_i2s/io_select
				;;
				*)
				;;
			esac
		;;
		KDS-SW3-EN7)
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
		e_p3k_audio_dante_name::?*)
			echo "e_p3k_audio_dante_name ($event) received"
			ipc @m_lm_set s set_dante:0:$_para1
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
			ipc @a_lm_set s ae_level:$_para1
		;;
		e_p3k_audio_dir::?*)
			#ipc @a_lm_set s ae_dir:$_para1
			case "$_para1" in
				in)
					ipc @m_lm_set s set_gpio_val:1:70:1
					case "$MODEL_NUMBER" in
						KDS-EN7)
							ipc @m_lm_set s set_gpio_val:1:72:0
						;;
					esac
				;;
				out)
					ipc @m_lm_set s set_gpio_val:1:70:0
					case "$MODEL_NUMBER" in
						KDS-EN7)
							ipc @m_lm_set s set_gpio_val:1:72:1
						;;
					esac
				;;
				*)
				;;
			esac
		;;
		e_p3k_audio_mute::?*)
			ipc @a_lm_set s ae_mute:$_para1
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

handle_e_p3k_cec()
{
	echo "handle_e_p3k_cec."
	local _para1

	#e_p3k_ir_dir::dir
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_cec_gw::?*)
			ipc @c_lm_set s ce_gw:$_para1
		;;
		e_p3k_cec_send::?*)
			ipc @c_lm_set s ce_send:$_para1
		;;
		*)
		;;
	esac
}

handle_e_p3k_upgrade()
{
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

handle_e_p3k_fp_lock()
{
	case "$event" in
		e_p3k_fp_lock_off)
			P3KCFG_FP_LOCK_ON='off'
			echo 0 > /sys/class/leds/lcd_power/brightness
			usleep 500000
			case $MODEL_NUMBER in
				KDS-SW3-EN7)
					lcd_display IPE5000P &
				;;
				KDS-EN7)
					lcd_display IPE5000 &
				;;
				*)
					echo "error param"
				;;
			esac
		;;
		e_p3k_fp_lock_on)
			P3KCFG_FP_LOCK_ON='on'
			echo 1 > /sys/class/leds/lcd_power/brightness
			pkill -9 lcd_display
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
		e_p3k_net_gw_port::?*)
			handle_e_p3k_net_gw_port "$event"
		;;
		e_p3k_net_hostname::?*)
			handle_e_p3k_net_hostname "$event"
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
			e_ipe5000w_led*)
				handle_e_ipe5000w_led_chose "$event"
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

	astparam s fourth_priority_board_status $POWER_ON
	astparam s sec_priority_net_status $GET_IP_FAIL
	astparam s repeat_net_lighting_flag 0
	astparam s repeat_board_lighting_flag 0
	# Print the final parameters
	echo_parameters

	init_param_from_p3k_cfg
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
	else
		P3KCFG_HDCP_1_ON='on'
		P3KCFG_HDCP_2_ON='on'
		P3KCFG_HDCP_3_ON='on'
		P3KCFG_AV_MUTE='off'
		P3KCFG_AV_ACTION='play'
	fi
	echo "P3KCFG_HDCP_1_ON=$P3KCFG_HDCP_1_ON"
	echo "P3KCFG_HDCP_2_ON=$P3KCFG_HDCP_2_ON"
	echo "P3KCFG_HDCP_3_ON=$P3KCFG_HDCP_3_ON"
	echo "P3KCFG_AV_MUTE=$P3KCFG_AV_MUTE"
	echo "P3KCFG_AV_ACTION=$P3KCFG_AV_ACTION"

	if [ -f "$gateway_setting" ];then
		P3KCFG_IR_DIR=`jq -r '.gateway.ir_direction' $gateway_setting`
		if echo "$P3KCFG_IR_DIR" | grep -q "null" ; then
			P3KCFG_IR_DIR='out'
		fi
	else
		P3KCFG_IR_DIR='out'
	fi
	echo "P3KCFG_IR_DIR=$P3KCFG_IR_DIR"
}

set_variable_power_on_status()
{
	if [ $P3KCFG_HDCP_1_ON = 'on' ];then
		ipc @m_lm_set s set_hdcp_cap:0:1
	else
		ipc @m_lm_set s set_hdcp_cap:0:0
	fi
	if [ $P3KCFG_HDCP_2_ON = 'on' ];then
		ipc @m_lm_set s set_hdcp_cap:1:1
	else
		ipc @m_lm_set s set_hdcp_cap:1:0
	fi
	if [ $P3KCFG_HDCP_3_ON = 'on' ];then
		ipc @m_lm_set s set_hdcp_cap:2:1
	else
		ipc @m_lm_set s set_hdcp_cap:2:0
	fi

	if [ $P3KCFG_AV_MUTE = 'off' ];then
		echo 1 > /sys/class/leds/linein_mute/brightness
		echo 1 > /sys/class/leds/lineout_mute/brightness
	else
		echo 0 > /sys/class/leds/linein_mute/brightness
		echo 0 > /sys/class/leds/lineout_mute/brightness
	fi
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
. ./ir_decode.sh
# Used to patch link_mgrX.sh itself.
if [ -f ./patch_lmh.sh ]; then
	. ./patch_lmh.sh
fi

init_watchdog

#mknod $PIPE_INFO_LOCAL p    TBD
#mknod $PIPE_INFO_REMOTE p   TBD

ifconfig lo up
# initialize parameters
init_param_from_flash

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

# $AST_PLATFORM = ast1500hv4 or ptv1500hv2 or pce1500hv3
echo ""
echo "#### platform info:$AST_PLATFORM ####"
if [ -z "$AST_PLATFORM" ]; then
	echo "ERROR! no AST_PLATFORM info."
	exit 1;
fi

handle_button_on_boot

case "$MODEL_NUMBER" in
	WP-SW2-EN7)
		if communication_with_mcu -u -b 4; then
			UGP_FLAG="success"
		else
			UGP_FLAG="fail"
		fi
	;;
	*)
		if communication_with_mcu -u; then
			UGP_FLAG="success"
		else
			UGP_FLAG="fail"
		fi
	;;
esac

if [ $UGP_FLAG = 'success' ];then
	echo "mainswitch start."
	case "$MODEL_NUMBER" in
		KDS-EN7)
			audioswitch &
		;;
		KDS-SW3-EN7)
			mainswitch &
			audioswitch &
		;;
		WP-SW2-EN7)
			mainswitch &
		;;
		*)
		;;
	esac

fi

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

#configure the default configuration of MCU pin
if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-EN7)
			#set lineio_sel(70) pin to default to line_out;0:line_out;1:line_in
			#set i2s_sel(72) pin to default to 6802;0:ast1520 ; 1:it6802
			#set lcd_power(65) pin to default to no useful,it is useful in ast1520
			#set type_b or type_c(66) pin to default to type_b;0:type_c ; 1:type_b
			ipc @m_lm_set s set_gpio_config:3:70:1:65:1:72:1
			ipc @m_lm_set s set_gpio_val:3:70:0:65:0:72:1
		;;
		KDS-SW3-EN7)
			#set lineio_sel pin to default to line_out;0:line_out;1:line_in
			ipc @m_lm_set s set_gpio_config:3:70:1:65:1:66:1
			ipc @m_lm_set s set_gpio_val:3:70:0:65:0:66:0
			ipc @m_lm_set s set_gpio_config:9:15:1:35:1:8:1:36:1:37:1:32:1:33:1:11:1:12:1
			ipc @m_lm_set s set_gpio_val:9:15:1:35:1:8:1:36:1:37:1:32:1:33:1:11:1:12:1
		;;
		WP-SW2-EN7)
			#enable led_display	set_lcd_control--cmd;0--led_type 1--lcd_type;1--enable  0--disenable;
			ipc @m_lm_set s set_lcd_control:0:1
			led_display_num
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

# Start state machine in another process scope
state_machine &

# Bruce130123. Moved to state_machine. Avoid parameter scope problem.
#start_network 2
#do_s_init

# A7 to be removed. I stopped watchdog for development A7
# disable watchdog after 'sys_init_ok', let system reboot if it cannot get IP address from DHCP
#disable_watchdog
