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
av_setting="/data/configs/kds-7/av_setting/av_setting.json"
gateway_setting="/data/configs/kds-7/gateway/gateway.json"
auto_switch_setting="/data/configs/kds-7/switch/auto_switch_setting.json"
# 0: This value does not exist.
# 1: GUI screen
# 2: Decode screen
CURRENT_SCREEN='1'
GUI_PICTURE_NAME='default.jpg'
GUI_SHOW_TEXT_INIT=""
GUI_SHOW_TEXT_RUNTIME=""
GUI_PICTURE_DAEMON=""

# OSD globals ##################################################################
# OSD_STR_UUID: a token for tracking OSD_STR session.
OSD_STR_UUID=''
# OSD_PIC_UUID: a token for tracking OSD_PIC session.
OSD_PIC_UUID=''
# OSDSTR_XXX current OSD_STR session setting for CRT
OSDSTR_OSD_Y_START=''
OSDSTR_OSD_MAX_Y_SIZE=''
OSDSTR_OSD_TRANSPARENT=''
OSDSTR_OSD_BKGRND_MASK_EN=''
OSDSTR_OSD_OFF_TIMER=''
OSDSTR_OSD_STR_FILE=''
OSDSTR_OSD_FONT_SIZE=''
OSDSTR_OSD_FONT_COLOR=''
# OSDPIC_XXX current OSD_PIC session setting for CRT
OSDPIC_OSD_Y_START=''
OSDPIC_OSD_MAX_Y_SIZE=''
OSDPIC_OSD_TRANSPARENT=''
OSDPIC_OSD_BKGRND_MASK_EN=''
OSDPIC_OSD_OFF_TIMER=''
OSDPIC_OSD_PIC_FILE=''
OSDPIC_OSD_PIC_OPTION=''
OSDPIC_OSD_POSITION_ALIGN='0'
OSDPIC_OSD_POSITION_X_START_OFFSET='0'
OSDPIC_OSD_POSITION_Y_START_OFFSET='0'
################################################################################

# Construct scriptlet
######################################################
# eth_link_off_timer.sh $LM_LINK_OFF_TIMEOUT
echo "#!/bin/sh

sleep \"\$1\"
ast_send_event -1 e_link_off_time_up
" > /usr/local/bin/eth_link_off_timer.sh
chmod a+x /usr/local/bin/eth_link_off_timer.sh

start_eth_link_off_timer()
{
	eth_link_off_timer.sh "$1" &
}
stop_eth_link_off_timer()
{
	pkill eth_link_off_timer 2>/dev/null
}
######################################################

stop_all_service()
{
	## A7 TBD
	echo "TBD"
}

# NOTICE: This function run on different process scope. (not in state_machine scope)
# Bruce130123. Now moved into state_machine scope.
do_s_init()
{
	# Bruce171122. We won't show_dialog until ip_got.
	# Bruce150820. Call _switch_to_GUI_screen to init GUI's GUI_SHOW_TEXT_RUNTIME behavior.
	switch_to_GUI_screen hide_dialog

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
	## A7 TBD
	inform_gui_echo "Last state=$STATE event=$event"
	STATE='s_error'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
	handle_e_kill
}

handle_e_link_off_time_up()
{
	local LM_V_STATE=`ipc @v_lm_query q ve_param_query:V_STATE`

	case "$LM_V_STATE" in
		s_srv_on)
			# ignore.
			local _t='dummy'
		;;
		s_search | s_start_hb)
			a30_led_blink  $LINK_ON_G 500 500
			inform_gui_echo "Finding transmitter..."
			switch_to_GUI_screen hide_dialog
		;;
		s_idle)
			# VE stopped. For eth link off case.
			# I assume this case happens ONLY when:
			# STATE=='s_idle' and eth_link_is_off
			if [ "`cat $DISPLAY_SYS_PATH/screen`" = '[2] decode screen' ]; then
				# Switch to GUI(console) screen
				switch_to_GUI_screen show_dialog
			fi
		;;
		*)
			warn "Wrong state?! LM_V_STATE=$LM_V_STATE"
		;;
	esac
}

handle_e_encoder_ip_got()
{
	local LM_V_ENCODER_IP=`ipc @v_lm_query q ve_param_query:V_ENCODER_IP`

	if [ "$ETH_LINK_STATE" = 'on' ]; then
		case "$LM_V_ENCODER_IP" in
			Unknown)
				inform_gui_echo "Trying to find the transmitter..."
			;;
			*)
				inform_gui_echo "Transmitter found. Connecting..."
			;;
		esac
	fi

	# GWIP for backward compatible with A6.x API
	GWIP="$LM_V_ENCODER_IP"
	inform_gui "e_ip_remote_got::$LM_V_ENCODER_IP"
}

do_stop_srv_ex()
{
	# STOP_LIST may contain duplicate items. Use X_DONE to track.
	local v_done=0 u_done=0 a_done=0 r_done=0 s_done=0 p_done=0 c_done=0
	local w

	echo "STOP_LIST (${STOP_LIST[@]})"
	for w in ${STOP_LIST[@]}; do
		case $w in
			v)
				if [ $v_done -eq 1 ]; then
					continue
				fi
				a30_led_blink  $LINK_ON_G 500 500
				if [ "$NO_VIDEO" = 'n' ]; then
					# unloading videoIP driver will switch back to console screen unexpectly. Turn osd off here.
					osd_off 0
					ipc @v_lm_set s ve_stop
				fi
				a30_led_off $LINK_ON_G
				# Switch to GUI(console) screen
				switch_to_GUI_screen show_dialog
				v_done=1
			;;
			u)
				if [ $u_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
					ipc @u_lm_set s ue_stop
				fi
				u_done=1
			;;
			a)
				if [ $a_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_I2S" = 'n' ]; then
					ipc @a_lm_set s ae_stop
				fi
				a_done=1
			;;
			r)
				if [ $r_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_IR" = 'n' ]; then
					ipc @r_lm_set s re_stop
				fi
				r_done=1
			;;
			s)
				if [ $s_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_SOIP" = 'n' ]; then
					ipc @s_lm_set s se_stop
				fi
				s_done=1
			;;
			p)
				if [ $p_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_PWRBTN" = 'n' ]; then
					ipc @p_lm_set s pe_stop
				fi
				p_done=1
			;;
			c)
				if [ $c_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_CEC" = 'n' ]; then
					ipc @c_lm_set s ce_stop
				fi
				c_done=1
			;;

			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done
	# Done. Clear entire STOP_LIST
	#unset STOP_LIST
	STOP_LIST=()
}

do_stop_srv()
{
	if [ $NO_VIDEO = 'n' ]; then
		a30_led_blink  $LINK_ON_G 500 500
		# unloading videoIP driver will switch back to console screen unexpectly. Turn osd off here.
		osd_off 0
		ipc @v_lm_set s ve_stop
		a30_led_off $LINK_ON_G
		# Bruce171226. To be eth off/on in a short time friendly,
		# We don't switch back to GUI screen immediately.
		# Instead, we start a eth_link_off_timer(). See handle_e_eth_link_off()
		# Switch to GUI(console) screen
		##switch_to_GUI_screen show_dialog
	fi

	if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
		ipc @u_lm_set s ue_stop
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
}

do_start_srv_ex()
{
	# CHG_LIST may contain duplicate items. Use X_DONE to track.
	local v_done=0 u_done=0 a_done=0 r_done=0 s_done=0 p_done=0 c_done=0
	local w

	for w in ${CHG_LIST[@]}; do
		case $w in
			v)
				if [ $v_done -eq 1 ]; then
					continue
				fi
				## inform_gui_ui_feature_action "GUI_hide_dialog"
				## inform_gui_echo "Start Services..."
				if [ "$NO_VIDEO" = 'n' ]; then
					ipc @v_lm_set s ve_start:$CH_SELECT_V
					update_node_info CH_SELECT_V $CH_SELECT_V
				fi
				# Bruce170828. Try to reduce reconnect time. There is only about 50ms impact here.
				{
					a30_led_blink  $LINK_ON_G 500 500
					inform_gui_echo "Services started..."
					# Why hide again here?
					inform_gui_ui_feature_action "GUI_hide_dialog"
				} &
				v_done=1
			;;
			u)
				if [ $u_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
					ipc @u_lm_set s ue_start:$CH_SELECT_U
					update_node_info CH_SELECT_U $CH_SELECT_U
					# Node change notification is currently used only for KMoIP roaming.
					notify_node_info_chg
				fi
				u_done=1
			;;
			a)
				if [ $a_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_I2S" = 'n' ]; then
					ipc @a_lm_set s ae_start:$CH_SELECT_A
					update_node_info CH_SELECT_A $CH_SELECT_A
				fi
				a_done=1
			;;
			r)
				if [ $r_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_IR" = 'n' ]; then
					ipc @r_lm_set s re_start:$CH_SELECT_R
					update_node_info CH_SELECT_R $CH_SELECT_R
				fi
				r_done=1
			;;
			s)
				if [ $s_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_SOIP" = 'n' ]; then
					ipc @s_lm_set s se_start:$CH_SELECT_S
					update_node_info CH_SELECT_S $CH_SELECT_S
				fi
				s_done=1
			;;
			p)
				if [ $p_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_PWRBTN" = 'n' ]; then
					ipc @p_lm_set s pe_start:$CH_SELECT_P
					update_node_info CH_SELECT_P $CH_SELECT_P
				fi
				p_done=1
			;;
			c)
				if [ $c_done -eq 1 ]; then
					continue
				fi
				if [ "$NO_CEC" = 'n' -a "$CEC_GATWAY" = 'off' ]; then
					ipc @c_lm_set s ce_start:$CH_SELECT_C
					update_node_info CH_SELECT_C $CH_SELECT_C
				fi
				c_done=1
			;;

			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done
	# Done. Clear entire CHG_LIST
	#unset CHG_LIST
	CHG_LIST=()
	to_s_srv_on
}

to_s_srv_on()
{
	STATE='s_srv_on'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
}

to_s_idle()
{
	STATE='s_idle'
	echo $STATE > $DEVICE_STATUS_FILE
	update_node_info STATE $STATE
}

do_ready_to_go()
{
	if eth_link_is_off ; then
		inform_gui_echo "Network link is down"
		to_s_idle
	elif [ "$ACCESS_ON" = 'n' ]; then
		inform_gui "Press the link button to connect"
		to_s_idle
	else
		# for "$ACCESS_ON" == 'y'
		#do_start_srv
		do_start_srv_ex
	fi
}

handle_e_sys_init_ok()
{
	if [ "$STATE" = 's_init' ]; then
		# Init default values
		#U_USBIP_STATE='off' #use ulmparam g U_USBIP_STATE instead.

		# A7 disable watchdog after sys_init_ok
                #disable_watchdog

		# Bruce170427. Already done in astparam init. No need here.
		# Refresh 4-bits dip switch here, so that ch_select get updated under cases like "e_reconnect"
		#refresh_4bits_ch
		#refresh_ch_params

		handle_e_log
		# Export LM params to /var/lm. So that sub-LM can import.
		handle_e_var

		# disable logo
		display_logo 0

		update_node_info
		# start node_responser
		node_responser --mac $MY_MAC &

		heartbeat &

		if [ "$NO_VIDEO" = 'n' ]; then
			# display-related (crt) configuration for video
			set_display_config
			# Start vLM
			link_mgrc_video.sh
		fi

		if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
			# Start uLM
			link_mgrc_usb.sh
		fi

		if [ "$NO_I2S" = 'n' ]; then
			# Start aLM
			link_mgrc_audio.sh
		fi

		if [ "$NO_IR" = 'n' ]; then
			# Start rLM
			link_mgrc_ir.sh
		fi

		if [ "$NO_SOIP" = 'n' ]; then
			# Start sLM
			link_mgrc_serial.sh
		fi

		if [ "$NO_PWRBTN" = 'n' ]; then
			# Start pLM
			link_mgrc_pushbutton.sh
		fi

		if [ "$NO_CEC" = 'n' ]; then
			# Start cLM
			link_mgrc_cec.sh
		fi

		handle_e_reconnect_on_boot

		set_igmp_leave_force
	fi
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
	pkill -9 inetd
	#pkill -9 httpd
	pkill -9 lcd_display
	pkill -9 web
	pkill -9 telnetd
	pkill -9 ldap_daemon
	pkill -9 p3ktcp

	avahi-daemon -D
	name_service -tclient
	inetd &
	#httpd -h /www &
	start_telnetd
	# it is harmless to send igmp leave in background because the switch will query immediately after a leave received.
	inform_gui_ui_feature_action "GUI_refresh_node"

	update_node_info

	node_responser --mac $MY_MAC &
	heartbeat &
	p3ktcp &
	usleep 10000
	web &

	case $MODEL_NUMBER in
		KDS-DEC7)
			if [ $P3KCFG_FP_LOCK_ON = 'off' ];then
				lcd_display IPD5000 &
			fi
		;;
		*)
		;;
	esac

	ulmparam s MY_IP $MY_IP
	ast_send_event -1 e_reconnect

	set_igmp_leave_force
}

_link_on_off()
{
	if [ "$ACCESS_ON" = 'y' ]; then
		handle_e_stop_link_all
	else
		handle_e_reconnect_all
	fi
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

handle_e_button_pairing_1()
{
	if [ "$STATE" = 's_idle' ] && eth_link_is_off && [ "$BTN2_LONG_ON_ETH_OFF" != 'e_btn_ignore' ]; then
		handle_"$BTN2_LONG_ON_ETH_OFF"
		return
	fi

	handle_"$BTN2_LONG"
}


handle_e_vw_delay_kick()
{
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	# Parse e_vw_delay_kick_XXXX
	VW_VAR_DELAY_KICK=$5

	#VW_VAR_DELAY_KICK=`expr "$*" : 'e_vw_delay_kick_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_DELAY_KICK" ]; then
		VW_VAR_DELAY_KICK='0'
	fi
	echo "$VW_VAR_DELAY_KICK" > "$VIDEO_SYS_PATH"/delay_dequeue
	astparam s vw_delay_kick "$VW_VAR_DELAY_KICK"
	astparam save
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

handle_e_usb_request_on_off()
{
	if [ $NO_USB = 'n' ]; then
		local LM_U_USBIP_STATE=`ipc @u_lm_query q ue_param_query:U_USBIP_STATE`
		case "$LM_U_USBIP_STATE" in
			'off')
				ipc @u_lm_set s ue_request_usbip
			;;
			'on')
				ipc @u_lm_set s ue_stop_usbip
			;;
			'request')
				ipc @u_lm_set s ue_request_usbip
			;;
			*)
				warn "Unknown state?! LM_U_USBIP_STATE=$LM_U_USBIP_STATE"
			;;
		esac
	fi
}

handle_e_kill()
{
	## A7 TBD
	kill "$EM_PID"

	stop_all_service
	a30_led_off $LINK_ON_G

	disable_watchdog
	to_mfg_mode

	exit 0;
}

do_e_reconnect()
{
	echo "CHG_LIST (${CHG_LIST[@]})"
	# Update ACCESS_ON
	if [ "${#CHG_LIST[@]}" -gt 0 ]; then
		ACCESS_ON='y'
	else
		ACCESS_ON='n'
	fi

	#igmp_leave_immediately
	case "$STATE" in
		s_idle|s_init)
			#ACCESS_ON='y'
			do_ready_to_go
		;;
		s_srv_on)
			#_switch_to_decode_screen
			#ACCESS_ON='y'
			# uLM SHARE_USB=y case behave differently if we call do_stop_srv()
			# instead of do_ready_to_go().
			# Calling do_stop_srv() will drop 'requested_usb' access.
			# Bruce think keeping 'requested_usb' access is more user friendly.
			# So, call do_ready_to_go().
			# If user don't like this behavior, call e_stop_link first before
			# calling e_reconnect.
			do_ready_to_go
		;;
		*)
			warn "Wrong state?! ($STATE)"
		;;
	esac
}

_update_access_on_x()
{
	# $1: y|n
	# $2: the list array
	local w
	local -a _skey_list=()
	local s=$1
	# shift 1 to remove first argument so that we can get _chg_list[].
	shift 1
	local -a _chg_list
	# Bruce171228. Bash bug!!. DO NOT assign value to local array on declare.
	# Otherwise, you get memory leak.
	# NOTE. From my test, assign empty array is fine.
	_chg_list=("$@")

	for w in ${_chg_list[@]}; do
		case $w in
			v)
				ACCESS_ON_V="$s"
				_skey_list+=("--set_key ACCESS_ON_V=$ACCESS_ON_V")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_v $ACCESS_ON_V
				#fi
			;;
			u)
				ACCESS_ON_U="$s"
				_skey_list+=("--set_key ACCESS_ON_U=$ACCESS_ON_U")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_u $ACCESS_ON_U
				#fi
			;;
			a)
				ACCESS_ON_A="$s"
				_skey_list+=("--set_key ACCESS_ON_A=$ACCESS_ON_A")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_a $ACCESS_ON_A
				#fi
			;;
			r)
				ACCESS_ON_R="$s"
				_skey_list+=("--set_key ACCESS_ON_R=$ACCESS_ON_R")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_r $ACCESS_ON_R
				#fi
			;;
			s)
				ACCESS_ON_S="$s"
				_skey_list+=("--set_key ACCESS_ON_S=$ACCESS_ON_S")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_s $ACCESS_ON_S
				#fi
			;;
			p)
				ACCESS_ON_P="$s"
				_skey_list+=("--set_key ACCESS_ON_P=$ACCESS_ON_P")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_p $ACCESS_ON_P
				#fi
			;;

			c)
				ACCESS_ON_C="$s"
				_skey_list+=("--set_key ACCESS_ON_C=$ACCESS_ON_C")
				#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
				#	astparam s access_on_c $ACCESS_ON_C
				#fi
			;;
			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done
	if [ "${#_skey_list[@]}" -gt 0 ]; then
		node_query --if essential --of essential ${_skey_list[@]}
	fi
	# Optimize astparam access. Do save astparam if RESET_CH_ON_BOOT is y.
	#if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
	#	astparam save &
	#fi
}

refresh_enabled_service_list()
{
	local -a _new_list=()
	local w

	# Check ACCESS_ON_X for all 'enabled' service.
	for w in ${DEFAULT_CHG_LIST[@]}; do
		case "$w" in
			v)
				if [ "$ACCESS_ON_V" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			u)
				if [ "$ACCESS_ON_U" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			a)
				if [ "$ACCESS_ON_A" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			r)
				if [ "$ACCESS_ON_R" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			s)
				if [ "$ACCESS_ON_S" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			p)
				if [ "$ACCESS_ON_P" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;

			c)
				if [ "$ACCESS_ON_C" = 'n' ]; then
					continue
				fi
				_new_list+=("$w")
			;;
			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done

	CHG_LIST=("${_new_list[@]}")
}

handle_e_reconnect_on_boot()
{
	refresh_enabled_service_list

	do_e_reconnect
}

handle_e_reconnect_all()
{
	# Just enable all services.
	ACCESS_ON_V='y'
	ACCESS_ON_U='y'
	ACCESS_ON_A='y'
	ACCESS_ON_R='y'
	ACCESS_ON_S='y'
	ACCESS_ON_P='y'
	ACCESS_ON_C='y'

	# This function may triggered by API command. So, we need to update ACCESS_ON_X here.
	_update_access_on_x y "${DEFAULT_CHG_LIST[@]}"

	handle_e_reconnect_refresh
}

handle_e_reconnect_refresh()
{
	# Refresh 4-bits dip switch here, so that ch_select get updated under cases like "e_reconnect"
	refresh_4bits_ch
	refresh_ch_params
	update_node_info ch_select

	refresh_enabled_service_list

	do_e_reconnect
}

handle_e_reconnect()
{
	#
	# e_reconnect::$ch_select::$who
	#
	# $who: [optional]
	#  z: all features == vausrpc.
	#  Z: all features except video. == ausrpc
	#  v: video
	#  u: usb
	#  a: audio
	#  r: IRoIP
	#  s: SerialoIP
	#  p: GPIOoIP
	#  c: CECoIP
	#
	# $ch_select can be:
	#  0000~9999: channel format
	#  xxx.xxx.xxx.xxx: IP format
	#

	case "$STATE" in
		s_idle | s_srv_on)
			# typical case
		;;
		*)
			# s_init
			warn "Wrong state?! ($STATE)"
			return
		;;
	esac

	# if there is no $ch_select, it means legacy command without parameters.
	#
	if [ "$*" = 'e_reconnect' ]; then
		handle_e_reconnect_all
		return
	fi

	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	local _ch_select=$1
	local _who=$3
	local i w
	local -a _new_list=()

	# If there is no $_who, it implies to all.
	if [ -z $_who ]; then
		_who='z'
		# Bruce170828. Apply speedy V switching trick as default behavior.
		{
			ast_send_event -1 e_reconnect::$_ch_select::v
			sleep 1
			ast_send_event -1 e_reconnect::$_ch_select::Z
		} &
		return
	fi
	if [ ${#_who} -eq 1 ]; then
		case $_who in
			z)
				_who='vuasrpc'
			;;
			Z)
				_who='uasrpc'
			;;
			*)
				# ignore
			;;
		esac
	fi
	_ch_select=`foolproof_ch_select $_ch_select`

	for (( i=0 ; i < ${#_who} ; i++ )); do
		w=${_who:$i:1}
		case "$w" in
			v)
				_new_list[$i]="$w"
				CH_SELECT_V=$_ch_select
				astparam s ch_select_v $CH_SELECT_V
				if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
					LED_DISPLAY_CH_SELECT_V=$CH_SELECT_V
					led_display_num
				fi
			;;
			u)
				_new_list[$i]="$w"
				CH_SELECT_U=$_ch_select
				astparam s ch_select_u $CH_SELECT_U
			;;
			a)
				_new_list[$i]="$w"
				CH_SELECT_A=$_ch_select
				astparam s ch_select_a $CH_SELECT_A
			;;
			r)
				_new_list[$i]="$w"
				CH_SELECT_R=$_ch_select
				astparam s ch_select_r $CH_SELECT_R
			;;
			s)
				_new_list[$i]="$w"
				CH_SELECT_S=$_ch_select
				astparam s ch_select_s $CH_SELECT_S
			;;
			p)
				_new_list[$i]="$w"
				CH_SELECT_P=$_ch_select
				astparam s ch_select_p $CH_SELECT_P
			;;
			c)
				_new_list[$i]="$w"
				CH_SELECT_C=$_ch_select
				astparam s ch_select_c $CH_SELECT_C
			;;

			*)
				warn "Unsupported type?! ($w)"
				# How to handle it? Ignore the whole command?
				return
			;;
		esac
	done

	_update_access_on_x y "${_new_list[@]}"
	# Bruce170731. Fix RctBug#2017073100.
	# Calling _update_access_on_x() will NOT do "astparam save" anymore.
	# Optimize astparam access. Do save astparam if RESET_CH_ON_BOOT is y.
	if [ "$RESET_CH_ON_BOOT" = 'n' ]; then
		astparam save
	fi

	CHG_LIST=("${_new_list[@]}")

	do_e_reconnect
}

handle_e_chg_hostname()
{
	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	local _host_name=$1
	astparam s hostname_customized $_host_name
	astparam save
	/sbin/avahi-daemon -k 2>/dev/null
	#start avahi-daemon
	# The $HOSTNAME_ID is now decided in refresh_hostname_params()
	refresh_4bits_ch
	refresh_ch_params
	refresh_hostname_params
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

	/sbin/avahi-daemon -D
	update_node_info HOSTNAME $HOSTNAME
	#update_node_info CH_SELECT $CH_SELECT
}

_is_all_access_off()
{
	local w
	# Only return TRUE when all ACCESS_ON_X are 'n'.
	for w in ${DEFAULT_CHG_LIST[@]}; do
		case "$w" in
			v)
				if [ "$ACCESS_ON_V" = 'y' ]; then
					return 1
				fi
			;;
			u)
				if [ "$ACCESS_ON_U" = 'y' ]; then
					return 1
				fi
			;;
			a)
				if [ "$ACCESS_ON_A" = 'y' ]; then
					return 1
				fi
			;;
			r)
				if [ "$ACCESS_ON_R" = 'y' ]; then
					return 1
				fi
			;;
			s)
				if [ "$ACCESS_ON_S" = 'y' ]; then
					return 1
				fi
			;;
			p)
				if [ "$ACCESS_ON_P" = 'y' ]; then
					return 1
				fi
			;;
			c)
				if [ "$ACCESS_ON_C" = 'y' ]; then
					return 1
				fi
			;;

			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done
	# Return TRUE
	return 0
}

do_e_stop_link()
{
	# stop service based on STOP_LIST
	do_stop_srv_ex

	if _is_all_access_off; then
		case "$STATE" in
			s_idle)
				ACCESS_ON='n'
			;;
			s_srv_on)
				ACCESS_ON='n'
				do_ready_to_go
			;;
			*)
				warn "Wrong state?! ($STATE)"
			;;
		esac
	fi
}

handle_e_stop_link_all()
{
	# reset STOP_LIST so that next e_reconnect can start All services.
	STOP_LIST=("${DEFAULT_CHG_LIST[@]}")
	_update_access_on_x n "${STOP_LIST[@]}"

	do_e_stop_link
}

handle_e_stop_link()
{
	#
	# e_stop_link::$who
	#
	# $who: [optional]
	#  z: all features == vausrpc.
	#  Z: all features except video. == ausrpc
	#  v: video
	#  u: usb
	#  a: audio
	#  r: IRoIP
	#  s: SerialoIP
	#  p: GPIOoIP
	#  c: CECoIP

	case "$STATE" in
		s_idle | s_srv_on)
			# typical case
		;;
		*)
			# s_init
			warn "Wrong state?! ($STATE)"
			return
		;;
	esac

	# if there is no $ch_select, it means legacy command without parameters.
	#
	if [ "$*" = 'e_stop_link' ]; then
		handle_e_stop_link_all
		return
	fi

	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"
	local _who=$1
	local i w
	local -a _new_list=()

	# If there is no $_who, it implies to all.
	if [ -z $_who ]; then
		_who='z'
	fi
	if [ ${#_who} -eq 1 ]; then
		case $_who in
			z)
				_who='vuasrpc'
			;;
			Z)
				_who='uasrpc'
			;;
			*)
				# ignore
			;;
		esac
	fi

	for (( i=0 ; i < ${#_who} ; i++ )); do
		w=${_who:$i:1}
		case "$w" in
			v)
				_new_list[$i]="$w"
			;;
			u)
				_new_list[$i]="$w"
			;;
			a)
				_new_list[$i]="$w"
			;;
			r)
				_new_list[$i]="$w"
			;;
			s)
				_new_list[$i]="$w"
			;;
			p)
				_new_list[$i]="$w"
			;;

			c)
				_new_list[$i]="$w"
			;;
			*)
				warn "Unsupported type?! ($w)"
			;;
		esac
	done

	STOP_LIST=("${_new_list[@]}")
	_update_access_on_x n "${STOP_LIST[@]}"
	do_e_stop_link
}

post_config()
{
	if [ "$EN_VIDEO_WALL" = 'y' ]; then
		vw_enable
	fi

	switch_to_decode_screen
}

handle_e_video_start_working()
{
	# A7 new. FIXME. Do we really need to check V_STATE here?
	local LM_V_STATE=`ipc @v_lm_query q ve_param_query:V_STATE`

	case "$LM_V_STATE" in
		s_start_srv | s_srv_on)
			# Turn off "Wait for Video Input" msg. Should delay 2 sec so that the up coming OSD has chance to cancel it.
			osd_set_to "VIDEO_START_DECODE" 8
			# We stay in s_srv_on state, but stop blinking the LED_LINK
			a30_led_on $LINK_ON_G
			post_config
		;;
		*)
			warn "Wrong state?! LM_V_STATE=$LM_V_STATE"
		;;
	esac
}

handle_e_video_stop_working()
{
	# FIXME. Do we really need to check V_STATE here?
	# Triggered by VE driver, do_CLIENT_EVENT_STOP_VE(). Typically on VE reset.
	local LM_V_STATE=`ipc @v_lm_query q ve_param_query:V_STATE`

	case "$LM_V_STATE" in
		s_start_srv | s_srv_on)
			# We stay in s_srv_on state, but start blinking the LED_LINK
			a30_led_blink  $LINK_ON_G 500 500
			a30_led_off $LINK_ON_G
		;;
		*)
			warn "Wrong state?! LM_V_STATE=$LM_V_STATE"
		;;
	esac
}

handle_e_video_hdcp_fail()
{
	osd_set_to "VIDEO_HDCP_FAIL" 3
}

handle_e_video_unsupported_mode()
{
	osd_set_to "VIDEO_UNSUPPORTED_MODE" 10
}

handle_e_video_unsupported_refresh_rate()
{
	osd_set_to "VIDEO_UNSUPPORTED_REFRESH_RATE" 10
}


handle_e_video_mode_changed()
{
	# It is unnecessary now.
	return
	osd_set_to "VIDEO_MODE_CHANGED" 25
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

handle_e_video_stat()
{
	case "$1" in
		e_video_stat_client_read_edid)
			#osd_set_to "VIDEO_CLIENT_READ_EDID"
		;;
		e_video_stat_client_wait_host_info)
			# FIXME. Do we really need to check V_STATE here?
			#osd_set_to "VIDEO_CLIENT_WAIT_HOST_INFO"
			local LM_V_STATE=`ipc @v_lm_query q ve_param_query:V_STATE`

			case "$LM_V_STATE" in
				s_start_srv | s_srv_on)
					inform_gui_echo "Waiting for video source - standby"
					a30_led_blink $LINK_ON_G 500 500
					astparam s fourth_priority_board_status $STANDBY
					control_board_led_status			
				;;
				s_idle|s_search|s_start_hb)
					# First time vLM loaded VE driver will trigger a e_video_stat_client_wait_host_info event.
				;;
				*)
					warn "Unexpected LM_V_STATE ($LM_V_STATE)?!"
				;;
			esac
		;;
		e_video_stat_mode_1080p)
			VIDEO_MODE='V_MODE_1080P'
			astparam s fourth_priority_board_status $POWER_ON
			control_board_led_status
		;;
		e_video_stat_mode_non_1080p)
			VIDEO_MODE='V_MODE_NON_1080P'
			astparam s fourth_priority_board_status $POWER_ON
			control_board_led_status
		;;
		*)
		;;
	esac
}

handle_e_ip_got()
{
	# Parse e_ip_got::MY_IP
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	MY_IP=$3

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
	#MY_IP=`expr "$*" : 'e_ip_got::\(.*\)'`

	if [ "$STATE" = 's_init' ]; then
		## A7 TBD. tickle_watchdog()?
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
		ast_send_event "$EM_PID" "e_name_id::$HOSTNAME_ID"

		echo "HOSTNAME:$HOSTNAME"
		astsetname $HOSTNAME
		echo "${MODEL_NUMBER}-${HOSTNAME_ID}" > /etc/device_name
		echo $HOSTNAME > /etc/hostname
		hostname -F /etc/hostname

		mknod /dev/videoip c 250 0

		route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
		# Force IGMP version to Version 2
		echo 2 > /proc/sys/net/ipv4/conf/eth0/force_igmp_version

		#avahi-daemon will be restarted in to_s_idle(). Skip here.
		#Bruce120718. Now use -SIGUSR2 to flush mDNS, so load avahi-d here.
		avahi-daemon -D
		name_service -tclient

		# Start inetd
		inetd &
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
			KDS-DEC7)
				if [ $P3KCFG_FP_LOCK_ON = 'off' ];then
					lcd_display IPD5000 &
				fi
			;;
			*)
			;;
		esac
		# it is harmless to send igmp leave in background because the switch will query immediately after a leave received.
		inform_gui_ui_feature_action "GUI_refresh_node"
		# Bruce171122. console screen is ready but dialog not yet. show/hide dialog at this point.
		if [ "$ACCESS_ON" = 'n' ]; then
			echo "GUI_show_dialog"
			inform_gui_ui_feature_action "GUI_show_dialog"
		else
			echo "GUI_hide_dialog"
			inform_gui_ui_feature_action "GUI_hide_dialog"
		fi

		ast_send_event -1 "e_sys_init_ok"
	else
		ast_send_event -1 "e_sys_ip_chg"
	fi

	set_igmp_report
}

handle_e_button_remote_ctrl()
{
	# event from pb_interrupt()/pb_pressed() of drivers/leds/ledtrig-pushbutton.c
	if [ "$1" = "e_button_remote_ctrl_pressed" ]; then
		echo "e_button_remote_ctrl_pressed"
		ipc @p_lm_set s pe_pwr_ctrl_press
	elif [ "$1" = "e_button_remote_ctrl_released" ]; then
		echo "e_button_remote_ctrl_released"
		ipc @p_lm_set s pe_pwr_ctrl_release
	fi
}

handle_e_button_chg_ch()
{
	refresh_4bits_ch
	# re-connect is not enough. We need to save the ch_select in case of board reboot.
	astparam s ch_select "${CH0}${CH1}${CH2}${CH3}"

	astparam save
	ast_send_event -1 'e_reconnect'
}

handle_e_start_usb()
{
	# Allow user to start/stop usbip by his will.
	if [ $NO_USB = 'n' ]; then
		#osd_set_to 'REQUEST_USB' 5
		ipc @u_lm_set s ue_request_usbip
	fi
}

handle_e_stop_usb()
{
	# Allow user to start/stop usbip by his will.
	if [ $NO_USB = 'n' ]; then
		#osd_set_to 'STOP_USB' 5
		ipc @u_lm_set s ue_stop_usbip
	fi
}

handle_e_start_kmoip()
{
	# Allow user to start/stop kmoip by his will.
	if [ $NO_KMOIP = 'n' ]; then
		ipc @u_lm_set s ue_start_kmoip
	fi
}

handle_e_stop_kmoip()
{
	# Allow user to start/stop kmoip by his will.
	if [ $NO_KMOIP = 'n' ]; then
		ipc @u_lm_set s ue_stop_kmoip
	fi
}

handle_e_usb_requesting()
{
	if [ "$MULTICAST_ON" = 'y' ] && [ "$SHARE_USB" = 'y' ]; then
		osd_set_to 'REQUEST_USB' 5
	fi
}

handle_e_usb_stopping()
{
	if [ "$MULTICAST_ON" = 'y' ] && [ "$SHARE_USB" = 'y' ]; then
		osd_set_to 'STOP_USB' 5
	fi
}

handle_e_vw_enable()
{
	# Parse e_vw_enable_${VW_VAR_MAX_ROW}_${VW_VAR_MAX_COLUMN}_${VW_VAR_ROW}_${VW_VAR_COLUMN}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_VW_VAR_MAX_ROW=$4
	_VW_VAR_MAX_COLUMN=$5
	_VW_VAR_ROW=$6
	_VW_VAR_COLUMN=$7
	_VW_VAR_VER=$8
	_VW_JUST_DO=$_VW_VAR_VER

	#_VW_VAR_MAX_ROW=`expr "$1" : 'e_vw_enable_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}'`
	#_VW_VAR_MAX_COLUMN=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}'`
	#_VW_VAR_ROW=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_COLUMN=`expr "$1" : 'e_vw_enable_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`

	if [ "$SOC_OP_MODE" -ge '3' ]; then
		if [ -n "$_VW_VAR_VER" ] && [ "$_VW_VAR_VER" = '2' ]; then
			VW_VAR_VER=$_VW_VAR_VER
			VW_VAR_V2_X1=$_VW_VAR_MAX_ROW
			VW_VAR_V2_Y1=$_VW_VAR_MAX_COLUMN
			VW_VAR_V2_X2=$_VW_VAR_ROW
			VW_VAR_V2_Y2=$_VW_VAR_COLUMN

			vw_enable

			./astparam s vw_ver ${VW_VAR_VER}
			./astparam s vw_v2_x1 ${VW_VAR_V2_X1}
			./astparam s vw_v2_y1 ${VW_VAR_V2_Y1}
			./astparam s vw_v2_x2 ${VW_VAR_V2_X2}
			./astparam s vw_v2_y2 ${VW_VAR_V2_Y2}
			./astparam save
			return
		fi
	fi

	VW_VAR_VER='1'

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
		VW_VAR_MAX_ROW="$MY_ROW_ID"
	elif [ "$_VW_VAR_MAX_ROW" != 'x' ]; then
		VW_VAR_MAX_ROW="$_VW_VAR_MAX_ROW"
	fi
	if [ "$_VW_VAR_MAX_COLUMN" = 'd' ]; then
		VW_VAR_MAX_COLUMN="$MY_ROW_ID"
	elif [ "$_VW_VAR_MAX_COLUMN" != 'x' ]; then
		VW_VAR_MAX_COLUMN="$_VW_VAR_MAX_COLUMN"
	fi

	if [ "$_VW_VAR_ROW" = 'd' ]; then
		VW_VAR_ROW='0'
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
	VW_VAR_LAYOUT="$(( $VW_VAR_MAX_ROW + 1 ))x$(( $VW_VAR_MAX_COLUMN + 1 ))"

	if [ -z "$_VW_JUST_DO" ]; then
		vw_enable
	fi

	./astparam s vw_max_row ${VW_VAR_MAX_ROW}
	./astparam s vw_max_column ${VW_VAR_MAX_COLUMN}
	./astparam s vw_row ${VW_VAR_ROW}
	./astparam s vw_column ${VW_VAR_COLUMN}
	./astparam s vw_ver ${VW_VAR_VER}
	./astparam save
}

handle_e_vw_moninfo()
{
	# Parse e_vw_moninfo_${VW_VAR_MONINFO_HA}_${VW_VAR_MONINFO_HT}_${VW_VAR_MONINFO_VA}_${VW_VAR_MONINFO_VT}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_MONINFO_HA=$4
	VW_VAR_MONINFO_HT=$5
	VW_VAR_MONINFO_VA=$6
	VW_VAR_MONINFO_VT=$7
	VW_VAR_MONINFO_DO_IT=$8

	#VW_VAR_MONINFO_HA=`expr "$1" : 'e_vw_moninfo_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_HT=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_VA=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)_[[:digit:]]\{1,\}'`
	#VW_VAR_MONINFO_VT=`expr "$1" : 'e_vw_moninfo_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_[[:digit:]]\{1,\}_\([[:digit:]]\{1,\}\)'`
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

	if [ "$VW_VAR_MONINFO_DO_IT" != '0' ]; then
		vw_update_moninfo
	fi

	astparam s vw_moninfo_ha "$VW_VAR_MONINFO_HA"
	astparam s vw_moninfo_ht "$VW_VAR_MONINFO_HT"
	astparam s vw_moninfo_va "$VW_VAR_MONINFO_VA"
	astparam s vw_moninfo_vt "$VW_VAR_MONINFO_VT"
	astparam save
}

handle_e_eth_link_off()
{
	set_mtu

	case "$STATE" in
		s_init)
			#  Actually, eth_link_off event can be ignored under s_init state.
			return
		;;
		*)
			# In client, link off triggers reconnect (stop and start link).
			# link manager will double check eth link state in "s_search" state.
			# If eth link is off in "s_search" state, LM should try not sending out packets and go to "s_idle" state.
			#if [ "$ACCESS_ON" = 'y' ]; then
			#	handle_e_reconnect_refresh
			#else
			#	handle_e_stop_link
			#fi
			do_stop_srv

			start_eth_link_off_timer $LM_LINK_OFF_TIMEOUT

			# Bruce170927. A eth_link_off/on event race bug.
			# Scenario:
			# 1. e_eth_link_off
			# 2 handle_e_eth_link_off() calls do_ready_to_go()
			# 3 do_ready_to_go() check mac sysfs for link on/off
			# 4 Eth link is on (the unexpected state)
			# 5 do_ready_to_go() calls do_start_srv_ex(). (expect: call to_s_idle())
			# 6 do_start_srv_ex() tries to start service. But unfortunately, CHG_LIST is empty at this stage.
			# 7 LM stays in s_srv_on, but NO services started.
			# 8 eventhough LM got e_eth_link_on event later, still won't work because LM state is not as expected s_idle.
			# Solution:
			# Don't call do_ready_to_go(). Just do to_s_idle atomically here.
			#do_ready_to_go
			inform_gui_echo "Network link is down"
			to_s_idle
		;;
	esac
}

handle_e_eth_link_on()
{
	# Double check
	if eth_link_is_off ; then
		inform_gui_echo "Network link is down again"
		return
	fi

	set_mtu
	stop_eth_link_off_timer
	case "$STATE" in
		s_init)
			# Bug. avahi-daemon is loaded when to_s_init(). We should not kill avahi-daemon under this state.
			#  Actually, eth_link_on event can be ignored under s_init state.
			return
		;;
		s_idle)
			# Bug. avahi-daemon may not respond to astresname if eth link on/off frequently.
			# To resolve this issue, we try to reload avahi-daemon on each eth link on.
			# avahi-daemon -k
			# From my test, wait for 1 second after link on and load avahi-daemon can
			# resolve the problem that "can't find the host after link down for 10 minutes (SMC switch)".
			{ avahi-daemon -k 2>/dev/null; sleep 1; avahi-daemon -D; } &

			if [ "$ACCESS_ON" = 'y' ]; then
				handle_e_reconnect_refresh
			else
				inform_gui "Press the link button to connect"
			fi
		;;
		*)
			# Current AST1520 platform MAC driver indicate eth_link_on event twice.
			warn "FIXME: Wrong state?! $STATE"
		;;
	esac

	set_igmp_leave_force

	set_igmp_report
}

handle_e_osd_test()
{
	for _num in '0' '1' '2' '3' '4' '5' '6' '7' '8' '9' '10'; do
		osd_on $_num 3
	done
}

# Used to save the host's position layout for WebUI. Set once when performing basic setup.
handle_e_vw_pos_layout()
{
	# Parse e_vw_pos_layout_${_VW_VAR_POS_MAX_ROW}_${_VW_VAR_POS_MAX_COL}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_VW_VAR_POS_MAX_ROW=$5
	_VW_VAR_POS_MAX_COL=$6

	#_VW_VAR_POS_MAX_ROW=`expr "$1" : 'e_vw_pos_layout_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_POS_MAX_COL=`expr "$1" : 'e_vw_pos_layout_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
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

handle_e_vw_refresh_pos_idx()
{
	# Parse e_vw_refresh_pos_idx_${_VW_VAR_POS_R}_${_VW_VAR_POS_C}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_VW_VAR_POS_R=$6
	_VW_VAR_POS_C=$7

	#_VW_VAR_POS_R=`expr "$*" : 'e_vw_refresh_pos_idx_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_POS_C=`expr "$*" : 'e_vw_refresh_pos_idx_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	if [ -z "$_VW_VAR_POS_R" ]; then
		_VW_VAR_POS_R='x'
	fi
	if [ -z "$_VW_VAR_POS_C" ]; then
		_VW_VAR_POS_C='x'
	fi
	if [ "$_VW_VAR_POS_R" = 'd' ]; then
		VW_VAR_POS_R="$MY_ROW_ID"
	elif [ "$_VW_VAR_POS_R" != 'x' ]; then
		VW_VAR_POS_R="$_VW_VAR_POS_R"
	fi
	if [ "$_VW_VAR_POS_C" = 'd' ]; then
		VW_VAR_POS_C='VW_VAR_COLUMN'
	elif [ "$_VW_VAR_POS_C" != 'x' ]; then
		VW_VAR_POS_C="$_VW_VAR_POS_C"
	fi

	VW_VAR_POS_IDX="r${VW_VAR_POS_R}c${VW_VAR_POS_C}"
	./astparam s vw_pos_idx ${VW_VAR_POS_IDX}
	./astparam save
}

## A7 to be removed
##handle_rXcX()
##{
##	# Seperate rXcX and from event
##	_IFS="$IFS";IFS='_';set -- $*;
##	_idx="$1"
##	shift
##	_e="$*"
##	IFS="$_IFS"
##	# Parse rXcX
##	_IFS="$IFS";IFS='rc';set -- $_idx;IFS="$_IFS"
##	_R="$2"
##	_C="$3"
##	#_R=`expr "$*" : 'r\([[:digit:]x]\{1,\}\)c[[:digit:]x]\{1,\}_.*'`
##	#_C=`expr "$*" : 'r[[:digit:]x]\{1,\}c\([[:digit:]x]\{1,\}\)_.*'`
##	if [ "$_R" = 'x' ]; then
##		_R="$VW_VAR_POS_R"
##	fi
##	if [ "$_C" = 'x' ]; then
##		_C="$VW_VAR_POS_C"
##	fi
##	_idx="r${_R}c${_C}"
##	if [ "$_idx" = "$VW_VAR_POS_IDX" ]; then
##		#_e=`expr "$*" : 'r[[:digit:]x]\{1,\}c[[:digit:]x]\{1,\}_\(.*\)'`
##		ast_send_event -1 "$_e"
##	fi
##}

handle_e_vw_v_shift_d()
{
	# Parse e_vw_v_shift_d_${VW_VAR_V_SHIFT}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_V_SHIFT=$6

	#VW_VAR_V_SHIFT=`expr "$*" : 'e_vw_v_shift_d_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_V_SHIFT" ]; then
		VW_VAR_V_SHIFT='0'
	fi

	vw_update_v_shift

	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam save
}
handle_e_vw_v_shift_u()
{
	# Parse e_vw_v_shift_u_${VW_VAR_V_SHIFT}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_V_SHIFT=$6

	#VW_VAR_V_SHIFT=`expr "$*" : 'e_vw_v_shift_u_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_V_SHIFT" ]; then
		VW_VAR_V_SHIFT='0'
	fi
	if [ "$VW_VAR_V_SHIFT" != '0' ]; then
		VW_VAR_V_SHIFT="-$VW_VAR_V_SHIFT"
	fi

	vw_update_v_shift

	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam save
}
handle_e_vw_h_shift_r()
{
	# Parse e_vw_h_shift_r_${VW_VAR_H_SHIFT}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_H_SHIFT=$6

	#VW_VAR_H_SHIFT=`expr "$*" : 'e_vw_h_shift_r_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_H_SHIFT" ]; then
		VW_VAR_H_SHIFT='0'
	fi

	vw_update_h_shift

	astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam save
}
handle_e_vw_h_shift_l()
{
	# Parse e_vw_h_shift_l_${VW_VAR_H_SHIFT}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_H_SHIFT=$6

	#VW_VAR_H_SHIFT=`expr "$*" : 'e_vw_h_shift_l_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_H_SHIFT" ]; then
		VW_VAR_H_SHIFT='0'
	fi
	if [ "$VW_VAR_H_SHIFT" != '0' ]; then
		VW_VAR_H_SHIFT="-$VW_VAR_H_SHIFT"
	fi

	vw_update_h_shift

	astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam save
}
handle_e_vw_h_scale()
{
	# Parse e_vw_h_scale_${VW_VAR_H_SCALE}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_H_SCALE=$5

	#VW_VAR_H_SCALE=`expr "$*" : 'e_vw_h_scale_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_H_SCALE" ]; then
		VW_VAR_H_SCALE='0'
	fi

	vw_update_scale_up

	astparam s vw_h_scale "$VW_VAR_H_SCALE"
	astparam save
}
handle_e_vw_v_scale()
{
	# Parse e_vw_v_scale_${VW_VAR_V_SCALE}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_V_SCALE=$5

	#VW_VAR_V_SCALE=`expr "$*" : 'e_vw_v_scale_\([[:digit:]]\{1,\}\)'`
	if [ -z "$VW_VAR_V_SCALE" ]; then
		VW_VAR_V_SCALE='0'
	fi

	vw_update_scale_up

	astparam s vw_v_scale "$VW_VAR_V_SCALE"
	astparam save
}

handle_e_vw_stretch_type()
{
	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi
	# Parse e_vw_stretch_type_${VW_VAR_STRETCH_TYPE}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_STRETCH_TYPE=$5

	if [ -z "$VW_VAR_STRETCH_TYPE" ]; then
		VW_VAR_STRETCH_TYPE='2'
	fi

	vw_update_stretch_type

	astparam s vw_stretch_type "$VW_VAR_STRETCH_TYPE"
	astparam save
}

handle_e_vw_rotate()
{
	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi
	# Parse e_vw_rotate_${VW_VAR_ROTATE}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	VW_VAR_ROTATE=$4

	if [ -z "$VW_VAR_ROTATE" ]; then
		VW_VAR_ROTATE='0'
	fi

	vw_update_rotate

	astparam s vw_rotate "$VW_VAR_ROTATE"
	astparam save
}

handle_e_vw_reset_to_pos()
{
	# Parse e_vw_reset_to_pos_${_VW_VAR_MAX_ROW}_${_VW_VAR_MAX_COLUMN}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_VW_VAR_MAX_ROW=$6
	_VW_VAR_MAX_COLUMN=$7

	#_VW_VAR_MAX_ROW=`expr "$*" : 'e_vw_reset_to_pos_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_MAX_COLUMN=`expr "$*" : 'e_vw_reset_to_pos_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	if [ -z "$_VW_VAR_MAX_ROW" ]; then
		_VW_VAR_MAX_ROW='x'
	fi
	if [ -z "$_VW_VAR_MAX_COLUMN" ]; then
		_VW_VAR_MAX_COLUMN='x'
	fi

	if [ "$_VW_VAR_MAX_ROW" = 'd' ]; then
		VW_VAR_MAX_ROW="$MY_ROW_ID"
	elif [ "$_VW_VAR_MAX_ROW" != 'x' ]; then
		VW_VAR_MAX_ROW="$_VW_VAR_MAX_ROW"
	fi
	if [ "$_VW_VAR_MAX_COLUMN" = 'd' ]; then
		VW_VAR_MAX_COLUMN="$MY_ROW_ID"
	elif [ "$_VW_VAR_MAX_COLUMN" != 'x' ]; then
		VW_VAR_MAX_COLUMN="$_VW_VAR_MAX_COLUMN"
	fi

	VW_VAR_ROW='0' # multiple host video wall always 0
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
	VW_VAR_ROTATE='0'
	VW_VAR_STRETCH_TYPE='2'
	#echo "$VW_VAR_H_SHIFT" > "$DISPLAY_SYS_PATH"/vw_h_shift
	#echo "$VW_VAR_V_SHIFT" > "$DISPLAY_SYS_PATH"/vw_v_shift
	#echo "$VW_VAR_H_SCALE $VW_VAR_V_SCALE" > "$DISPLAY_SYS_PATH"/vw_scale_up
	#echo "$VW_VAR_DELAY_KICK" > "$VIDEO_SYS_PATH"/delay_dequeue

	vw_enable

	astparam s vw_max_row "$VW_VAR_MAX_ROW"
	astparam s vw_max_column "$VW_VAR_MAX_COLUMN"
	astparam s vw_row "$VW_VAR_ROW"
	astparam s vw_column "$VW_VAR_COLUMN"
	astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam s vw_h_scale "$VW_VAR_H_SCALE"
	astparam s vw_v_scale "$VW_VAR_V_SCALE"
	astparam s vw_delay_kick "$VW_VAR_DELAY_KICK"
	astparam s vw_rotate "$VW_VAR_ROTATE"
	astparam s vw_stretch_type "$VW_VAR_STRETCH_TYPE"
	astparam save
}

# For single host mode
handle_e_vw_reset_to_pos_s()
{
	# Parse e_vw_reset_to_pos_s_${_VW_VAR_MAX_ROW}_${_VW_VAR_MAX_COLUMN}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_VW_VAR_MAX_ROW=$7
	_VW_VAR_MAX_COLUMN=$8

	#_VW_VAR_MAX_ROW=`expr "$*" : 'e_vw_reset_to_pos_s_\([[:alnum:]]\{1,\}\)_[[:alnum:]]\{1,\}'`
	#_VW_VAR_MAX_COLUMN=`expr "$*" : 'e_vw_reset_to_pos_s_[[:alnum:]]\{1,\}_\([[:alnum:]]\{1,\}\)'`
	if [ -z "$_VW_VAR_MAX_ROW" ]; then
		_VW_VAR_MAX_ROW='x'
	fi
	if [ -z "$_VW_VAR_MAX_COLUMN" ]; then
		_VW_VAR_MAX_COLUMN='x'
	fi

	if [ "$_VW_VAR_MAX_ROW" = 'd' ]; then
		VW_VAR_MAX_ROW="$MY_ROW_ID"
	elif [ "$_VW_VAR_MAX_ROW" != 'x' ]; then
		VW_VAR_MAX_ROW="$_VW_VAR_MAX_ROW"
	fi
	if [ "$_VW_VAR_MAX_COLUMN" = 'd' ]; then
		VW_VAR_MAX_COLUMN="$MY_ROW_ID"
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
	VW_VAR_ROTATE='0'
	VW_VAR_STRETCH_TYPE='2'
	#echo "$VW_VAR_H_SHIFT" > "$DISPLAY_SYS_PATH"/vw_h_shift
	#echo "$VW_VAR_V_SHIFT" > "$DISPLAY_SYS_PATH"/vw_v_shift
	#echo "$VW_VAR_H_SCALE $VW_VAR_V_SCALE" > "$DISPLAY_SYS_PATH"/vw_scale_up
	#echo "$VW_VAR_DELAY_KICK" > "$VIDEO_SYS_PATH"/delay_dequeue

	vw_enable

	astparam s vw_max_row "$VW_VAR_MAX_ROW"
	astparam s vw_max_column "$VW_VAR_MAX_COLUMN"
	astparam s vw_row "$VW_VAR_ROW"
	astparam s vw_column "$VW_VAR_COLUMN"
	astparam s vw_h_shift "$VW_VAR_H_SHIFT"
	astparam s vw_v_shift "$VW_VAR_V_SHIFT"
	astparam s vw_h_scale "$VW_VAR_H_SCALE"
	astparam s vw_v_scale "$VW_VAR_V_SCALE"
	astparam s vw_delay_kick "$VW_VAR_DELAY_KICK"
	astparam s vw_rotate "$VW_VAR_ROTATE"
	astparam s vw_stretch_type "$VW_VAR_STRETCH_TYPE"
	astparam save
}

handle_e_video_enter_pwr_save()
{
	inform_gui_echo "Waiting for video source - standby"
	if [ "$V_TURN_OFF_SCREEN_ON_PWR_SAVE" = 'n' ]; then
		# Turn off power save
		display_power_save 0

		# Switch to GUI screen for displaying information.
		switch_to_GUI_screen hide_dialog

		# in case of switching screen racing, we poll the video state again
		if [ `cat "$VIDEO_SYS_PATH"/State` = 'OPERATING' ]; then
			# to decode screen
			display_screen_force 2
		fi
	fi
}

handle_e_ulm_notify_chg()
{
	local U_USBIP_STATE=`ipc @u_lm_query q ue_param_query:U_USBIP_STATE`

	## Move into uLM to reduce LM overhead.
	##if [ "$PWR_LED_TYPE" != 'share_usb' ]; then
	##	return
	##fi
	case "$U_USBIP_STATE" in
		on)
			led_on $LED_PWR
		;;
		off)
			led_off $LED_PWR
		;;
		request)
			led_blink $LED_PWR
		;;
		*)
			echo "ERROR: Unknown U_USBIP_STATE($U_USBIP_STATE)!?"
		;;
	esac
}

handle_e_nq_notify_chg()
{
	#e_nq_notify_chg::82D0194B1276::essential
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _who="$3"
	local _what="$5"
	ipc @u_lm_set s ue_kmr_slave_chg:$_who
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

handle_e_pwr_status_chg()
{
	# Parse e_pwr_status_chg_${_on}
	_IFS="$IFS";IFS='_';set -- $*;IFS="$_IFS"
	_on=$5

	#_on=`expr "$*" : 'e_pwr_status_chg_\(.\)'`
	echo "$_on" > "${GPIO_SYS_PATH}"/gpio_pwr_status_out/brightness
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

handle_e_vw()
{
	case "$*" in
		e_vw_refresh_pos_idx_?*)
			handle_e_vw_refresh_pos_idx "$*"
		;;
		e_vw_pos_layout_?*)
			handle_e_vw_pos_layout "$*"
		;;
		e_vw_v_shift_u_?*)
			handle_e_vw_v_shift_u "$*"
		;;
		e_vw_v_shift_d_?*)
			handle_e_vw_v_shift_d "$*"
		;;
		e_vw_h_shift_r_?*)
			handle_e_vw_h_shift_r "$*"
		;;
		e_vw_h_shift_l_?*)
			handle_e_vw_h_shift_l "$*"
		;;
		e_vw_h_scale_?*)
			handle_e_vw_h_scale "$*"
		;;
		e_vw_v_scale_?*)
			handle_e_vw_v_scale "$*"
		;;
		e_vw_delay_kick_?*)
			handle_e_vw_delay_kick "$*"
		;;
		e_vw_reset_to_pos_s_?*)
			handle_e_vw_reset_to_pos_s "$*"
		;;
		e_vw_reset_to_pos_?*)
			handle_e_vw_reset_to_pos "$*"
		;;
		e_vw_moninfo_?*)
			handle_e_vw_moninfo "$*"
		;;
		e_vw_enable_?*)
			handle_e_vw_enable "$*"
		;;
		e_vw_stretch_type_?*)
			handle_e_vw_stretch_type "$*"
		;;
		e_vw_rotate_?*)
			handle_e_vw_rotate "$*"
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

handle_e_gui_show_picture_daemon()
{
	echo "handle_e_gui_show_picture_daemon"

	 _IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	if [ "$3" != '' ]; then
		chmod a+x $3
	fi

	if [ $CURRENT_SCREEN = '2' ]; then
		echo "Set GUI_PICTURE_DAEMON=$3"
		GUI_PICTURE_DAEMON="$3"
	else
		if [ "$GUI_PICTURE_DAEMON" != '' ]; then
			ps -A | grep $GUI_PICTURE_DAEMON | kill `awk '{print $1}'`
		fi

		GUI_PICTURE_DAEMON="$3"

		if [ "$3" != '' ]; then
			$GUI_PICTURE_DAEMON &
		fi
	fi
}

handle_e_gui_show_picture()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	if [ "$5" = '' ]; then
		GUI_SHOW_TEXT_RUNTIME="$GUI_SHOW_TEXT_INIT"
	else
		GUI_SHOW_TEXT_RUNTIME="$5"
	fi

	GUI_PICTURE_NAME="$3"

	if [ $CURRENT_SCREEN = '2' ]; then
		echo "Set GUI_PICTURE_NAME=$GUI_PICTURE_NAME"
	else
		echo "GUI_show_picture::$GUI_PICTURE_NAME::$GUI_SHOW_TEXT_RUNTIME"
		inform_gui_action "GUI_show_picture::$GUI_PICTURE_NAME::$GUI_SHOW_TEXT_RUNTIME"
	fi
}

handle_e_osd_off_str()
{
	#e_osd_off_str::OSD_STR_UUID
	osd_off_str "$*"
}

handle_e_osd_on_str()
{
	local _OSD_Y_START
	local _OSD_MAX_Y_SIZE
	local _OSD_TRANSPARENT
	local _OSD_BKGRND_MASK_EN
	local _OFF_TIMER
	local _OSD_STR_FILE
	local _OSD_FONT_SIZE
	local _OSD_FONT_COLOR

	#e_osd_on_str::OSD_Y_START::OSD_MAX_Y_SIZE::OSD_TRANSPARENT::OSD_BKGRND_MASK_EN::OFF_TIMER::OSD_STR_FILE::OSD_FONT_SIZE::OSD_FONT_COLOR
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_OSD_Y_START="$1"
	shift 2
	_OSD_MAX_Y_SIZE="$1"
	shift 2
	_OSD_TRANSPARENT="$1"
	shift 2
	_OSD_BKGRND_MASK_EN="$1"
	shift 2
	_OFF_TIMER="$1"
	shift 2
	_OSD_STR_FILE="$1"
	shift 2
	_OSD_FONT_SIZE="$1"
	shift 2
	_OSD_FONT_COLOR="$1"

	# TODO. Validate input parameters.
	#echo "OSD string!!! $_OSD_Y_START $_OSD_MAX_Y_SIZE $_OSD_TRANSPARENT $_OSD_BKGRND_MASK_EN $_OFF_TIMER $_OSD_STR_FILE $_OSD_FONT_SIZE $_OSD_FONT_COLOR"
	osd_on_str "$_OSD_Y_START" "$_OSD_MAX_Y_SIZE" "$_OSD_TRANSPARENT" "$_OSD_BKGRND_MASK_EN" "$_OFF_TIMER" "$_OSD_STR_FILE" "$_OSD_FONT_SIZE" "$_OSD_FONT_COLOR"
}

handle_e_osd_off_pic()
{
	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi

	#e_osd_off_pic::OSD_PIC_UUID or e_osd_off_pic::now
	osd_off_pic "$*"
}

handle_e_osd_on_pic()
{
	local _OSD_Y_START
	local _OSD_MAX_Y_SIZE
	local _OSD_TRANSPARENT
	local _OSD_BKGRND_MASK_EN
	local _OFF_TIMER
	local _OSD_PIC_FILE
	local _OSD_PIC_OPTION

	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi

	#e_osd_on_pic::OSD_Y_START::OSD_MAX_Y_SIZE::OSD_TRANSPARENT::OSD_BKGRND_MASK_EN::OFF_TIMER::OSD_PIC_FILE::OSD_PIC_OPTION
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_OSD_Y_START="$1"
	shift 2
	_OSD_MAX_Y_SIZE="$1"
	shift 2
	_OSD_TRANSPARENT="$1"
	shift 2
	_OSD_BKGRND_MASK_EN="$1"
	shift 2
	_OFF_TIMER="$1"
	shift 2
	_OSD_PIC_FILE="$1"
	shift 2
	_OSD_PIC_OPTION="$1"

	# TODO. Validate input parameters.
	#echo "OSD PIC!!! $_OSD_Y_START $_OSD_MAX_Y_SIZE $_OSD_TRANSPARENT $_OSD_BKGRND_MASK_EN $_OFF_TIMER $_OSD_PIC_FILE $_OSD_PIC_OPTION"
	osd_on_pic "$_OSD_Y_START" "$_OSD_MAX_Y_SIZE" "$_OSD_TRANSPARENT" "$_OSD_BKGRND_MASK_EN" "$_OFF_TIMER" "$_OSD_PIC_FILE" "$_OSD_PIC_OPTION"
}

handle_e_osd_position()
{
	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi
	#e_osd_position::OSD_POSITION_ALIGN::OSD_POSITION_X_START_OFFSET::OSD_POSITION_Y_START_OFFSET
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_OSD_POSITION_ALIGN="$1"
	shift 2
	_OSD_POSITION_X_START_OFFSET="$1"
	shift 2
	_OSD_POSITION_Y_START_OFFSET="$1"

	osd_position "$_OSD_POSITION_ALIGN" "$_OSD_POSITION_X_START_OFFSET" "$_OSD_POSITION_Y_START_OFFSET"
}

handle_e_osd_set_to()
{
	#e_osd_set_to::OSD_TYPE_STR::TIMEOUT
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	local _type="$1"
	shift 2
	local _timeout="$1"

	osd_set_to "$_type" "$_timeout"
}

handle_e_osd()
{
	case "$*" in
		e_osd_position?*)
			handle_e_osd_position "$*"
		;;
		e_osd_on_str?*)
			handle_e_osd_on_str "$*"
		;;
		e_osd_off_str?*)
			handle_e_osd_off_str "$*"
		;;
		e_osd_on_pic?*)
			handle_e_osd_on_pic "$*"
		;;
		e_osd_off_pic?*)
			handle_e_osd_off_pic "$*"
		;;
		e_osd_set_to?*)
			handle_e_osd_set_to "$*"
		;;
		*)
			echo "Unknown OSD event?! ($*)"
		;;
	esac
}

handle_e_wrong_fw()
{
	(# Use subshell, (), to export OSD_OFF_TIMER
		export OSD_OFF_TIMER='3'
		osd_on.sh "Unsupported Video Stream"
	)
}

handle_e_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_lmc.sh ]; then
		source ./patch_lmc.sh
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

construct_default_chg_list()
{
	if [ "$NO_VIDEO" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "v")
	fi
	if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "u")
	fi
	if [ "$NO_I2S" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "a")
	fi
	if [ "$NO_IR" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "r")
	fi
	if [ "$NO_SOIP" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "s")
	fi
	if [ "$NO_PWRBTN" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "p")
	fi
	if [ "$NO_CEC" = 'n' ]; then
		DEFAULT_CHG_LIST=("${DEFAULT_CHG_LIST[@]}" "c")
	fi
}

handel_e_p3k_vw()
{
	echo "handle_e_p3k_vw."
	local _para1

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"

	case "$event" in
		e_p3k_video_vw_mode::?*)
			echo "e_p3k_video_vw_mode ($event) received"
		;;
		e_p3k_video_vw_id::?*)
			echo "e_p3k_video_vw_id ($event) received"
		;;
		e_p3k_video_vw_stretch::?*)
			echo "e_p3k_video_vw_stretch ($event) received"
		;;
		e_p3k_video_vw_bezel::?*)
			echo "e_p3k_video_vw_bezel ($event) received"
		;;
		*)
		echo "ERROR!!!! Invalid event ($event) received"
		;;
	esac
}

handle_e_p3k_video()
{
	echo "handle_e_p3k_video."
	local _para1

	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_para1="$1"


	case "$event" in
		e_p3k_video_rgb::?*)
			ipc @v_lm_set s ve_rgb:$_para1
		;;
		e_p3k_video_scale::?*)
			ipc @v_lm_set s ve_scale:$_para1
		;;
		e_p3k_video_vw_?*)
			handel_e_p3k_vw "$event"
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
		P3KCFG_AV_MUTE='off'
		echo 100 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 1 > /sys/class/leds/linein_mute/brightness
	    echo 1 > /sys/class/leds/lineout_mute/brightness
		ipc @m_lm_set s set_hdmi_mute:16:1:0
	else
		#mute,Because the linein mute sound is still a little, you can directly turn the volume to 0 as mute
		P3KCFG_AV_MUTE='on'
		echo 0 > /sys/devices/platform/1500_i2s/analog_in_vol
		echo 0 > /sys/class/leds/linein_mute/brightness
	    echo 0 > /sys/class/leds/lineout_mute/brightness
		ipc @m_lm_set s set_hdmi_mute:16:1:1
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
			ipc @a_lm_set s ae_level:$_para1
		;;
		e_p3k_audio_dir::?*)
			ipc @a_lm_set s ae_dir:$_para1
		;;
		e_p3k_audio_mute::?*)
			web_mute_slider_handle $_para1
		;;
		*)
		echo "ERROR!!!! Invalid event ($event) received"
		;;
	esac

}

handle_e_key()
{
	case "$*" in
		e_key_enter_pressed)
			if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
				return
			fi
			send_key_info 1
			p3k_notify button::enter
		;;
		e_key_up_pressed)
			if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
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
			if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
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
			if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
				return
			fi
			send_key_info 4
			p3k_notify button::left
		;;
		e_key_right_pressed)
			if [ "$MODEL_NUMBER" = 'WP-DEC7' ];then
				return
			fi
			send_key_info 5
			p3k_notify button::right
		;;
		*)
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

enable_hdmi_in_cec_report()
{
	echo 0 > /sys/devices/platform/cec/cec_report
	#ipc @m_lm_set s close_cec_report
}

enable_hdmi_out_cec_report()
{
	echo 1 > /sys/devices/platform/cec/cec_report
	#ipc @m_lm_set s open_cec_report
}

handle_ce_gw()
{
	local _para1=$1
	
	case "$MODEL_NUMBER" in
		KDS-DEC7)
			case $1 in
				over_ip)
					ipc @m_lm_set s set_gpio_val:1:68:1
					NO_CEC='n'
					CEC_GATWAY='off'
					ipc @c_lm_set s ce_start:$CH_SELECT_C
				;;
				hdmi_out)
					ipc @m_lm_set s set_gpio_val:1:68:1
					NO_CEC='y'
					CEC_GATWAY='on'
					CEC_SEND_DIR='hdmi_out'
					enable_hdmi_out_cec_report
					ipc @c_lm_set s ce_stop
				;;
				hdmi_in)
					NO_CEC='y'
					CEC_GATWAY='on'
					CEC_SEND_DIR='hdmi_in'
					enable_hdmi_in_cec_report
					ipc @c_lm_set s ce_stop
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
			echo "To be done"
		;;
		hdmi_out)
			cec_send $1
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
}

handle_e_p3k_net_multicast()
{
	echo "handle_e_p3k_net_multicast."
	local _ip

	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	_ip  = "$1"

	astparam s multicast_ip $_ip
	astparam save
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

handle_e_p3k_net_dante_hostname()
{
	echo "handle_e_p3k_net_dante_hostname."
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

handle_e_p3k_switch_in()
{
	echo "handle_e_p3k_switch_in."
	
	#e_p3k_switch_in::switch_input
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2

	case "$1" in
		HDMI)
			ipc @m_lm_set s set_input_source:16:1
		;;
		STREAM)
			ipc @m_lm_set s set_input_source:16:0
		;;
		*)
			echo "ERROR: Unknown param ($1)!?"
		;;
	esac
	
	echo "set p3k switch input!!! $1"
	
}

handle_e_p3k_fp_lock()
{
	case "$event" in
		e_p3k_fp_lock_off)
			P3KCFG_FP_LOCK_ON='off'
			echo 0 > /sys/class/leds/lcd_power/brightness
			usleep 500000
			case $MODEL_NUMBER in
				KDS-DEC7)
					lcd_display IPD5000 &
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

	astparam s soip_guest_on y
	astparam s soip_type 2
	astparam s soip_port $_para1
	astparam save

	ipc @s_lm_set s se_start:$CH_SELECT_S

}

handle_e_soip_gw_off()
{
	echo "handle_e_soip_gw_off."

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

handle_e_p3k()
{
	echo "handle_e_p3k."
	case "$*" in
		e_p3k_switch_in?*)
			handle_e_p3k_switch_in "$event"
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
		*)
		;;
	esac
}

handle_e_hdcp()
{
	case "$1" in
		e_hdcp_no_hdcp)
			ipc @m_lm_set s set_hdcp_mode:16:0
		;;
		e_hdcp_1.x)
			ipc @m_lm_set s set_hdcp_mode:16:1
		;;
		e_hdcp_2.x)
			ipc @m_lm_set s set_hdcp_mode:16:2
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

handle_e_set_ttl()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"

	shift 2
	#After modifying this value, ping our IP TTL will change to our modified value
	echo $1 > /proc/sys/net/ipv4/ip_default_ttl
}

handle_e_update_idle_screen()
{
	case `cat $DISPLAY_SYS_PATH/screen_off` in
		disable)
			SCREEN=`cat $DISPLAY_SYS_PATH/screen`
			if [ "$SCREEN" = '[1] message screen' ]; then
				echo 1 > $DISPLAY_SYS_PATH/screen_off
				cp -f /data/configs/kds-7/display/sleep_image.jpg /share/default.jpg
				switch_to_GUI_screen show_dialog
				echo 0 > $DISPLAY_SYS_PATH/screen_off
			fi
			;;
		*)
			;;
	esac
}

handle_set_up_alc5640()
{
	if [ $P3KCFG_AV_MUTE = 'off' ];then
		echo 1 > /sys/class/leds/lineout_mute/brightness
	else
		echo 0 > /sys/class/leds/lineout_mute/brightness
	fi
}

# Worst case 0.05s message loop without handling any event.
state_machine()
{
	# Bruce160308. Try to ignore all TERM signals.
	trap signal_handler SIGTERM SIGPIPE SIGHUP SIGINT SIGALRM SIGUSR1 SIGUSR2 SIGPROF SIGVTALRM

	# Declare an array for tracking channel change.
	declare -a DEFAULT_CHG_LIST=()
	declare -a CHG_LIST=()
	declare -a STOP_LIST=()
	GWIP="Unknown"

	construct_default_chg_list
	CHG_LIST=("${DEFAULT_CHG_LIST[@]}")

	start_network 1

	do_s_init

	while true; do
		event=`lm_get_event 2>/dev/null`
		#echo "Receive $event event on $STATE state "`cat /proc/uptime`
		case "$event" in
			e_reconnect*)
				handle_e_reconnect "$event"
			;;
			e_stop_link*)
				handle_e_stop_link "$event"
			;;
			e_button_?*)
				handle_e_button "$event"
			;;
			e_btn_?*)
				handle_"$event" "$event"
			;;
			e_p3k_?*)
				handle_e_p3k "$event"
			;;
			e_vw_?*)
				handle_e_vw "$event"
			;;
			e_sh_?*)
				handle_e_sh "$event"
			;;
			callback=*)
				handle_e_sh_jsonp "$event"
			;;
			e_hdcp_?*)
				handle_e_hdcp "$event"
			;;
			e_pwr_status_chg_?*)
				tickle_watchdog
				handle_e_pwr_status_chg "$event"
			;;
			e_video_stat_?*)
				handle_e_video_stat "$event"
			;;
			e_key_?*)
				handle_e_key "$event"
			;;
			## A7 to be removed? WebUI?
			##r?*c?*_?*)
			##	handle_rXcX "$event"
			##;;
			e_ip_got::?*)
				#echo "Pass event ($event) to GUI"
				inform_gui "$event::$STATE"
				handle_e_ip_got "$event"
			;;
			e_var_dump::?*::?*)
				handle_e_var_dump "$event"
			;;
			e_var_get::?*)
				handle_e_var_get "$event"
			;;
			e_var_set::?*::?*)
				handle_e_var_set "$event"
			;;
			e_ip_del::?* | e_name_id::?*)
				#echo "Pass event ($event) to GUI"
				inform_gui "$event::$STATE"
			;;
			e_gui_show_picture_daemon?*)
				tickle_watchdog
				handle_e_gui_show_picture_daemon "$event"
			;;
			e_gui_show_picture?*)
				tickle_watchdog
				handle_e_gui_show_picture "$event"
			;;
			e_osd?*)
				tickle_watchdog
				handle_e_osd "$event"
			;;
			e_nq_notify_chg?*)
				handle_e_nq_notify_chg "$event"
			;;
			e_ir_decoded*)
				handle_e_ir_decoded "$event"
			;;
			e_chg_hostname*)
				handle_e_chg_hostname "$event"
			;;
			e_set_ttl*)
				handle_e_set_ttl "$event"
			;;
			e_cec_cmd_report*)
				handle_e_cec_cmd_report "$event"
			;;
			e_set_up_alc5640*)
				handle_set_up_alc5640 "$event"
			;;
			e_log*)
				handle_e_log "$event"
			;;
			e_?*)
				tickle_watchdog
				handle_"$event"
			;;
			tick)
				tickle_watchdog
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

	# I2S_CLOCK_LOCK_MODE for SoC v2 client only.
	# 0x80: OFF
	# 0: Auto (default)
	# 1: Normal
	# 2: Low
	# 4: High
	I2S_CLOCK_LOCK_MODE=`astparam g i2s_clock_lock_mode`
	if echo "$I2S_CLOCK_LOCK_MODE" | grep -q "not defined" ; then
		I2S_CLOCK_LOCK_MODE=`astparam r i2s_clock_lock_mode`
		if echo "$I2S_CLOCK_LOCK_MODE" | grep -q "not defined" ; then
			I2S_CLOCK_LOCK_MODE='0'
		fi
	fi
	VIDEO_SHRINK=`astparam g ast_video_shrink`
	if [ "$VIDEO_SHRINK" = "\"ast_video_shrink\" not defined" ]; then
		VIDEO_SHRINK='y'
	fi
	echo "ast_video_shrink=$VIDEO_SHRINK  (abandon)"
	#Bruce130204. Overwrite SHARE_USB will break web UI code.
	# NO_USB should act as the main ON/OFF switch
	#if [ "$NO_USB" = 'y' ]; then
	#	echo "Client force SHARE_USB=n when NO_USB=y"
	#	SHARE_USB='n'
	#fi

	#Bruce130107. Overwrite NO_USB will break web UI code.
	#if [ "$SHARE_USB" = 'y' ]; then
	#	echo "Client force NO_USB=y under SHARE_USB mode"
	#	NO_USB='y'
	#fi

	HDCP2_RX_ID=`astparam g hdcp2_rx_id`
	if echo "$HDCP2_RX_ID" | grep -q "not defined" ; then
		HDCP2_RX_ID=`astparam r hdcp2_rx_id`
		if echo "$HDCP2_RX_ID" | grep -q "not defined" ; then
			HDCP2_RX_ID='unavailable'
		fi
	fi
#	HDCP2_RX_CERT=`astparam g hdcp2_rx_cert`
#	if echo "$HDCP2_RX_CERT" | grep -q "not defined" ; then
#		HDCP2_RX_CERT=`astparam r hdcp2_rx_cert`
#		if echo "$HDCP2_RX_CERT" | grep -q "not defined" ; then
#			HDCP2_RX_CERT='unavailable'
#		fi
#	fi
	HDCP2_RX_CERT_DATA=`astparam g hdcp2_rx_cert_data`
	if echo "$HDCP2_RX_CERT_DATA" | grep -q "not defined" ; then
		HDCP2_RX_CERT_DATA=`astparam r hdcp2_rx_cert_data`
		if echo "$HDCP2_RX_CERT_DATA" | grep -q "not defined" ; then
			HDCP2_RX_CERT_DATA='unavailable'
		fi
	fi
	HDCP2_RX_CERT_SIG=`astparam g hdcp2_rx_cert_sig`
	if echo "$HDCP2_RX_CERT_SIG" | grep -q "not defined" ; then
		HDCP2_RX_CERT_SIG=`astparam r hdcp2_rx_cert_sig`
		if echo "$HDCP2_RX_CERT_SIG" | grep -q "not defined" ; then
			HDCP2_RX_CERT_SIG='unavailable'
		fi
	fi
	HDCP2_RX_PRIVATE_KEY_CRT=`astparam g hdcp2_rx_private_key_crt`
	if echo "$HDCP2_RX_PRIVATE_KEY_CRT" | grep -q "not defined" ; then
		HDCP2_RX_PRIVATE_KEY_CRT=`astparam r hdcp2_rx_private_key_crt`
		if echo "$HDCP2_RX_PRIVATE_KEY_CRT" | grep -q "not defined" ; then
			HDCP2_RX_PRIVATE_KEY_CRT='unavailable'
		fi
	fi

	V_SRC_UNAVAILABLE_TIMEOUT=`astparam g v_src_unavailable_timeout`
	if echo "$V_SRC_UNAVAILABLE_TIMEOUT" | grep -q "not defined" ; then
		V_SRC_UNAVAILABLE_TIMEOUT=`astparam r v_src_unavailable_timeout`
		if echo "$V_SRC_UNAVAILABLE_TIMEOUT" | grep -q "not defined" ; then
			# Use 10s as default. (Driver is 5 sec by default, but VGA requires longer value.)
			# Set to '-1' to disable power save feature
			V_SRC_UNAVAILABLE_TIMEOUT='10000'
		fi
	fi

	V_PWR_SAVE_TIMEOUT=`astparam g v_pwr_save_timeout`
	if echo "$V_PWR_SAVE_TIMEOUT" | grep -q "not defined" ; then
		V_PWR_SAVE_TIMEOUT=`astparam r v_pwr_save_timeout`
		if echo "$V_PWR_SAVE_TIMEOUT" | grep -q "not defined" ; then
			V_PWR_SAVE_TIMEOUT='8000'
		fi
	fi

	V_TURN_OFF_SCREEN_ON_PWR_SAVE=`astparam g v_turn_off_screen_on_pwr_save`
	if echo "$V_TURN_OFF_SCREEN_ON_PWR_SAVE" | grep -q "not defined" ; then
		V_TURN_OFF_SCREEN_ON_PWR_SAVE=`astparam r v_turn_off_screen_on_pwr_save`
		if echo "$V_TURN_OFF_SCREEN_ON_PWR_SAVE" | grep -q "not defined" ; then
			# For easier field trouble shooting, we don't turn off screen on power save
			V_TURN_OFF_SCREEN_ON_PWR_SAVE='n'
		fi
	fi

	SHOW_CH_OSD=`astparam g show_ch_osd`
	if echo "$SHOW_CH_OSD" | grep -q "not defined" ; then
		SHOW_CH_OSD=`astparam r show_ch_osd`
		if echo "$SHOW_CH_OSD" | grep -q "not defined" ; then
			SHOW_CH_OSD='n'
		fi
	fi

	GUI_SHOW_TEXT_INIT=`astparam g ui_show_text`
	if echo "$GUI_SHOW_TEXT_INIT" | grep -q "not defined" ; then
		GUI_SHOW_TEXT_INIT=`astparam r ui_show_text`
		if echo "$GUI_SHOW_TEXT_INIT" | grep -q "not defined" ; then
			GUI_SHOW_TEXT_INIT='y'
		fi
	fi
	GUI_SHOW_TEXT_RUNTIME="$GUI_SHOW_TEXT_INIT"

	USB_DEFAULT_POLICY=`astparam g usb_default_policy`
	if echo "$USB_DEFAULT_POLICY" | grep -q "not defined" ; then
		USB_DEFAULT_POLICY=`astparam r usb_default_policy`
		if echo "$USB_DEFAULT_POLICY" | grep -q "not defined" ; then
			# auto_export or no_auto_export
			USB_DEFAULT_POLICY='auto_export'
		fi
	fi

	USB_CONFLICT_POLICY=`astparam g usb_conflict_policy`
	if echo "$USB_CONFLICT_POLICY" | grep -q "not defined" ; then
		USB_CONFLICT_POLICY=`astparam r usb_conflict_policy`
		if echo "$USB_CONFLICT_POLICY" | grep -q "not defined" ; then
			# auto_export or no_auto_export
			USB_CONFLICT_POLICY='no_auto_export'
		fi
	fi

	USB_ENABLE_CLASSES=`astparam g usb_enable_classes`
	if echo "$USB_ENABLE_CLASSES" | grep -q "not defined" ; then
		USB_ENABLE_CLASSES=`astparam r usb_enable_classes`
		if echo "$USB_ENABLE_CLASSES" | grep -q "not defined" ; then
			# To enable C-media USB audio, set to 'audio'
			# See /etc/usbip/usbip_export.conf for all valid options
			USB_ENABLE_CLASSES=''
		fi
	fi

	USB_DISABLE_CLASSES=`astparam g usb_disable_classes`
	if echo "$USB_DISABLE_CLASSES" | grep -q "not defined" ; then
		USB_DISABLE_CLASSES=`astparam r usb_disable_classes`
		if echo "$USB_DISABLE_CLASSES" | grep -q "not defined" ; then
			# To disable C-media USB audio, set to 'audio'
			# See /etc/usbip/usbip_export.conf for all valid options
			USB_DISABLE_CLASSES=''
		fi
	fi

	USB_ENABLE_DEVICES=`astparam g usb_enable_devices`
	if echo "$USB_ENABLE_DEVICES" | grep -q "not defined" ; then
		USB_ENABLE_DEVICES=`astparam r usb_enable_devices`
		if echo "$USB_ENABLE_DEVICES" | grep -q "not defined" ; then
			# To enable C-media USB audio, set to 'VID:PID'
			# See /etc/usbip/usbip_export.conf for all valid options
			USB_ENABLE_DEVICES=''
		fi
	fi

	USB_DISABLE_DEVICES=`astparam g usb_disable_devices`
	if echo "$USB_DISABLE_DEVICES" | grep -q "not defined" ; then
		USB_DISABLE_DEVICES=`astparam r usb_disable_devices`
		if echo "$USB_DISABLE_DEVICES" | grep -q "not defined" ; then
			# To disable C-media USB audio, set to 'VID:PID'
			# See /etc/usbip/usbip_export.conf for all valid options
			USB_DISABLE_DEVICES=''
		fi
	fi

	LED_DISPLAY_CH_SELECT_V=`astparam g ch_select_v`
	if echo "$LED_DISPLAY_CH_SELECT_V" | grep -q "not defined" ; then
		LED_DISPLAY_CH_SELECT_V=`astparam r ch_select_v`
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

	if [ -f "$av_setting" ];then
		P3KCFG_AV_MUTE=`jq -r '.av_setting.mute' $av_setting`
		if echo "$P3KCFG_AV_MUTE" | grep -q "null" ; then
			P3KCFG_AV_MUTE='off'
		fi

		P3KCFG_AV_ACTION=`jq -r '.av_setting.action' $av_setting`
		if echo "$P3KCFG_AV_ACTION" | grep -q "null" ; then
			P3KCFG_AV_ACTION='play'
		fi
	else
		P3KCFG_AV_MUTE='off'
		P3KCFG_AV_ACTION='play'
	fi
	echo "P3KCFG_AV_MUTE=$P3KCFG_AV_MUTE"
	echo "P3KCFG_AV_ACTION=$P3KCFG_AV_ACTION"

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
			CEC_SEND_DIR='hdmi_out'
		elif [ "$P3KCFG_CEC_DIR" = 'hdmi_in' ]; then
			CEC_SEND_DIR='hdmi_in'
		elif [ "$P3KCFG_CEC_DIR" = 'hdmi_loop' ]; then
			CEC_SEND_DIR='hdmi_in'
		else
			CEC_SEND_DIR='hdmi_out'
		fi
	else
		P3KCFG_IR_DIR='out'
		CEC_GATWAY='on'
		CEC_SEND_DIR='hdmi_out'
	fi
	echo "P3KCFG_IR_DIR=$P3KCFG_IR_DIR"
	echo "CEC_GATWAY=$CEC_GATWAY"
	echo "CEC_SEND_DIR=$CEC_SEND_DIR"

	if [ -f "$auto_switch_setting" ];then
		P3KCFG_SWITCH_IN=`jq -r '.gateway.ir_direction' $auto_switch_setting`
		if echo "$P3KCFG_SWITCH_IN" | grep -q "null" ; then
			P3KCFG_SWITCH_IN='stream'
		fi
	else
		P3KCFG_SWITCH_IN='stream'
	fi
	echo "P3KCFG_SWITCH_IN=$P3KCFG_SWITCH_IN"
}

set_variable_power_on_status()
{
	if [ $P3KCFG_AV_MUTE = 'off' ];then
		ipc @m_lm_set s set_hdmi_mute:16:1:0
	else
		ipc @m_lm_set s set_hdmi_mute:16:1:1
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
export PATH="$PATH":/usr/local/bin
cd /usr/local/bin
. ./display.sh
. ./include.sh
. ./ir_decode.sh

# Used to patch link_mgrX.sh itself.
if [ -f ./patch_lmc.sh ]; then
	. ./patch_lmc.sh
fi

#qzx 2021.11.4:It's useless to turn off the watchdog. If the MCU needs to be upgraded, it will lead to restart
#init_watchdog
echo no > /sys/devices/platform/watchdog/enable

mknod $PIPE_INFO_LOCAL p
mknod $PIPE_INFO_REMOTE p
mknod $PIPE_INFO_OSD p
mknod $PIPE_INFO_GUI_ACTION p
# msgd need "lo"
ifconfig lo up
msgd -h &

# initialize parameters
init_param_from_flash

# overwrite parameters from pssed in parameters
while [ -n "$1" ]; do
	if [ "$1" = "no_video" ]; then
		#inform_gui_echo "NO_VIDEO"
		NO_VIDEO='y'
	elif [ "$1" = "no_usb" ]; then
		#inform_gui_echo "NO_USB"
		NO_USB='y'
	elif [ "$1" = "no_i2s" ]; then
		#inform_gui_echo "NO_I2S"
		NO_I2S='y'
	elif [ "$1" = "no_ir" ]; then
		#inform_gui_echo "NO_IR"
		NO_IR='y'
	elif [ "$1" = "debug" ]; then
		#inform_gui_echo "DBG"
		DBG='1'
		set -x
	elif [ "$1" = "video_quality_mode_fix" ]; then
		#inform_gui_echo "fix video quality mode"
		V_QUALITY_MODE='1'
	fi
	shift 1
done

init_version_file
init_info_file
init_p3k_cfg_file
init_param_from_p3k_cfg

# $AST_PLATFORM = ast1500cv4 or ptv1500cv2 or pce1500cv3
echo ""
echo "#### platform info:$AST_PLATFORM ####"
if [ -z "$AST_PLATFORM" ]; then
	inform_gui_echo "ERROR! no AST_PLATFORM info."
	exit 1;
fi

rx_tcp_server &
handle_button_on_boot

case "$MODEL_NUMBER" in
	KDS-DEC7)
		if communication_with_mcu -u -b 2; then
			UGP_FLAG="success"
		else
			UGP_FLAG="fail"
		fi
	;;
	WP-DEC7)
		UGP_FLAG="fail"
		led_display_num
	;;
	*)
		UGP_FLAG="fail"
	;;
esac

if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-DEC7)
			echo "lock file for @m_lm_query" > /var/lock/@m_lm_query.lck
			ipc_server_listen_one @m_lm_set @m_lm_get @m_lm_query @m_lm_reply &
			usleep 1000
			#-b:select board_type 0:IPE5000 1:IPE5000P 2:IPD5000 3:IPD5000W
			communication_with_mcu -c -b 2 &
			usleep 10000
		;;
		*)
		;;
	esac
fi

if [ $UGP_FLAG = 'success' ];then
	case "$MODEL_NUMBER" in
		KDS-DEC7)
			#set lineio_sel pin to default to line_out;0:line_out;1:line_in
			#set cec_sel pin(68) to default to sii9316;1:sii9136-hdmi_out;0:hdmi_in-hdmi_out
			ipc @m_lm_set s set_gpio_config:3:65:1:70:1:68:1
			ipc @m_lm_set s set_gpio_val:3:70:0:65:0:68:1
			if [ $P3KCFG_SWITCH_IN = 'hdmi_in1' ];then
				ipc @m_lm_set s set_input_source:16:1
			fi
		;;
		*)
		;;
	esac
fi

#Initialize I2C bus3(temperature sensor)
init_temperature_sensor

if [ $UGP_FLAG = 'success' ];then
	set_variable_power_on_status
fi

# TBD remove? screen switch doesn't check HAS_CRT anymore
#if [ -f "$DISPLAY_SYS_PATH"/screen ]; then
#	HAS_CRT='y'
#else
#	HAS_CRT='n'
#fi

if [ -f /usr/local/bin/GUI ]; then
	if [ "$UI_FEATURE" = 'y' ]; then
		GUI -qws -keyboard -ui_feature -hostname_prefix "${HOSTNAME_PREFIX}${HOSTNAME_TX_MIDDLE}" -multicast_ip_prefix "$MULTICAST_IP_PREFIX" -option "$UI_OPTION" &
	else
		GUI -qws -option "$UI_OPTION" &
	fi
	HAS_GUI='y'
else
	HAS_GUI='n'
fi

# start event_monitor
ast_event_monitor &
EM_PID=$!

STATE='s_init'
echo $STATE > $DEVICE_STATUS_FILE
update_node_info STATE $STATE

# Disable OOM Killer
echo 1 > /proc/sys/vm/overcommit_memory

VIDEO_MODE='V_MODE_UNKNOWN'
# Start state machine in another process scope
state_machine &

# Bruce130123. Moved to state_machine. Avoid parameter scope problem.
#start_network 1
#do_s_init

# A7 to be removed. I stopped watchdog for development A7
# disable watchdog after 'sys_init_ok', let system reboot if it cannot get IP address from DHCP
#disable_watchdog
