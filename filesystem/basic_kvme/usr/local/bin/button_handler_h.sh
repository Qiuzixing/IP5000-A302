#!/bin/sh

##########################################
#btn1_short
#btn1_long
#btn1_short_on_boot
#btn1_long_on_boot
#btn1_short_on_eth_off
#btn1_long_on_eth_off
#
#btn2_short
#btn2_long
#btn2_short_on_boot
#btn2_long_on_boot
#btn2_short_on_eth_off
#btn2_long_on_eth_off
#
#btn_init: will be called once before LM start.
#btn1_delay: in ms. Used to control the response time of button press
#btn2_delay: in ms. Used to control the response time of button press
#
#handle_e_btn_xxxxx()
#{
#	# $1 will be "e_btn_xxxx"
#	p1="$1"
#}
###########################################

###########################################
# Default behavior. Can be overwritten by astparam
BTN1_SHORT="e_btn_toggle_link"
BTN1_LONG="e_btn_toggle_snoop"
BTN1_SHORT_ON_BOOT="e_btn_to_mfg_mode"
BTN1_LONG_ON_BOOT="e_btn_to_mfg_mode_n_default"
BTN1_SHORT_ON_ETH_OFF="e_btn_ignore"
BTN1_LONG_ON_ETH_OFF="e_btn_ignore"

BTN2_SHORT="e_btn_toggle_video_profile"
BTN2_LONG="e_btn_toggle_video_anti_dither"
BTN2_SHORT_ON_BOOT="e_btn_enable_edid_update"
BTN2_LONG_ON_BOOT="e_btn_ignore"
BTN2_SHORT_ON_ETH_OFF="e_btn_ignore"
BTN2_LONG_ON_ETH_OFF="e_btn_toggle_eth_jumbo"

BTN_INIT="e_btn_ignore"
BTN1_DELAY="200"
BTN2_DELAY="200"

if [ "$SOC_OP_MODE" -eq '1' ]; then
	BTN2_LONG_ON_ETH_OFF="e_btn_ignore"
fi
# Bruce150622. Rarely used and confused current user. Remove it.
#if [ "$MULTICAST_ON" = 'y' ] && [ "$EN_VIDEO_WALL" = 'y' ]; then
#	BTN1_SHORT="e_btn_vw_select_client_same_row"
#	BTN1_LONG="e_btn_vw_select_client_next_row"
#	BTN2_SHORT="e_btn_vw_toggle_client_column"
#	BTN2_LONG="e_btn_vw_toggle_basic_setup"
#	BTN_INIT="e_btn_init_vw"
#fi
###########################################

handle_e_btn_ignore()
{
	return
}

###############################################
##### Start of Video Wall implementation. (For RS232 chain)
####handle_e_btn_init_vw()
####{
####	if [ "$EN_VIDEO_WALL" = 'y' ]; then
####		# reduce the response time of long press to 0.5 sec (default is 2 sec)
####		echo 50 > "$GPIO_SYS_PATH"/button_link/delay
####		echo 50 > "$GPIO_SYS_PATH"/button_pairing/delay
####		stty 115200 -F /dev/ttyS0
####	fi
####}
####
#####btn2 long press
####handle_e_btn_vw_toggle_basic_setup()
####{
####	if [ "$EN_VIDEO_WALL" = 'y' ]; then
####		if [ "$VW_VAR_INSTALL_STATE" = 'off' ]; then
####			#VW_VAR_INSTALL_STATE='on'
####			#ast_send_event -1 r99_e_vw_enable_${MY_ROW_ID}_${MY_ROW_ID}_d_x
####			#ast_send_event -1 r99_msg_toFFFFFFFF_e_vw_enable_${MY_ROW_ID}_${MY_ROW_ID}_d_x
####			#ast_send_event -1 e_vw_pos_layout_${MY_ROW_ID}_${MY_ROW_ID}
####			#_vw_config_osd_on
####			ast_send_event -1 e_vw_refresh_pos_idx_start_d_d
####		else
####			#VW_VAR_INSTALL_STATE='off'
####			_vw_config_osd_off
####		fi
####		return
####	fi
####}
#####btn2 short press
####handle_e_btn_vw_toggle_client_column()
####{
####	# Toggle column value
####	if [ "$EN_VIDEO_WALL" = 'y' ]; then
####		if [ "$VW_VAR_INSTALL_STATE" = 'off' ]; then
####			return
####		fi
####		if [ "$THE_ROW_ID" = "$VW_VAR_POS_IDX" ]; then
####			if [ -n "$THE_CLIENT_IP" ] && [ "$THE_CLIENT_IP" != '00000000' ]; then
####				ast_send_event -1 msg_to${THE_CLIENT_IP}_e_vwbtn_toggle
####			fi
####		else
####			ast_send_event -1 "r${THE_ROW_ID}_e_btn_vw_toggle_client_column"
####		fi
####		return
####	fi
####}
#####btn1 short press. other client in the same row
####handle_e_btn_vw_select_client_same_row()
####{
####	# to next client
####	if [ "$EN_VIDEO_WALL" = 'y' ]; then
####		if [ "$VW_VAR_INSTALL_STATE" = 'off' ]; then
####			return
####		fi
####		if [ "$THE_ROW_ID" = "$VW_VAR_POS_IDX" ]; then
####			if [ -n "$THE_CLIENT_IP" ] && [ "$THE_CLIENT_IP" != '00000000' ]; then
####				ast_send_event -1 msg_to${THE_CLIENT_IP}_e_vwbtn_menu_off
####			fi
####			next_client_idx
####			THE_CLIENT_IP=`the_client_ip_hex`
####			if [ "$THE_CLIENT_IP" != '00000000' ]; then
####				ast_send_event -1 msg_to${THE_CLIENT_IP}_e_vwbtn_menu_on
####			fi
####		else
####			ast_send_event -1 "r${THE_ROW_ID}_e_btn_vw_select_client_same_row"
####		fi
####		return
####	fi
####}
#####btn1 long press. other client next row
####handle_e_btn_vw_select_client_next_row()
####{
####	# To next row
####	if [ "$EN_VIDEO_WALL" = 'y' ]; then
####		if [ "$VW_VAR_INSTALL_STATE" = 'off' ]; then
####			return
####		fi
####		# Turn off all OSDs when changing row
####		ast_send_event -1 r99_msg_toFFFFFFFF_e_vwbtn_menu_off
####		next_row_id
####		echo "Next Row ID: $THE_ROW_ID"
####		if [ "$THE_ROW_ID" = "$VW_VAR_POS_IDX" ]; then
####			# next_client_idx must be called here, otherwise $CLIENT_LIST won't have a chance to init
####			# when user started with "button_link_1".
####			next_client_idx
####			THE_CLIENT_IP=`the_client_ip_hex`
####			if [ "$THE_CLIENT_IP" != '00000000' ]; then
####				ast_send_event -1 r${VW_VAR_POS_IDX}_msg_to${THE_CLIENT_IP}_e_vwbtn_menu_on
####			fi
####		else
####			# Try to toggle next row's menu on
####			ast_send_event -1 "r${THE_ROW_ID}_e_btn_vw_select_client_same_row"
####		fi
####		return
####	fi
####}
##### End of Video Wall implementation.
###############################################


handle_e_btn_toggle_link()
{
	_link_on_off
}

handle_e_btn_toggle_video_profile()
{
	ipc @v_lm_set s ve_toggle_video_profile
}

handle_e_btn_toggle_video_profile_video()
{
	ipc @v_lm_set s ve_toggle_video_profile:-1
}

handle_e_btn_toggle_video_profile_graphic()
{
	ipc @v_lm_set s ve_toggle_video_profile:0
}

handle_e_btn_toggle_video_anti_dither()
{
	ipc @v_lm_set s ve_toggle_video_anti_dither
}

handle_e_btn_toggle_video_anti_dither_0()
{
	ipc @v_lm_set s ve_toggle_video_anti_dither:0
}

handle_e_btn_toggle_video_anti_dither_1()
{
	ipc @v_lm_set s ve_toggle_video_anti_dither:1
}

handle_e_btn_toggle_video_anti_dither_2()
{
	ipc @v_lm_set s ve_toggle_video_anti_dither:2
}

handle_e_btn_toggle_eth_jumbo()
{
	# Bruce120601. For MAC jumbo frame configure.
	# Steps: Unplug ethernet cable -> wait for s_idle -> long press button 2 to toggle.
	if [ "$SOC_OP_MODE" -ge '2' ]; then
		if [ "$JUMBO_MTU" = '8000' ]; then
			led_blink "$LED_LINK"
			JUMBO_MTU='1500'
			astparam s jumbo_mtu 1500
			astparam save
			echo "jumbo mtu=$JUMBO_MTU"
		else
			led_on "$LED_LINK"
			JUMBO_MTU='8000'
			astparam s jumbo_mtu 8000
			astparam save
			echo "jumbo mtu=$JUMBO_MTU"
		fi
	fi
}

handle_e_btn_to_mfg_mode()
{
	if [ "$SCENARIO" = "usb_only" ]; then
		MFG_MODE='Off'
	else
		MFG_MODE='On'
	fi

	if [ "$MFG_MODE" = 'On' ]; then
		#disable watchdog
		disable_watchdog
		to_mfg_mode 'keep_rw'
		# Stop link manager
		exit 0
	fi
}

handle_e_btn_to_mfg_mode_n_default()
{
	if [ "$SCENARIO" = "usb_only" ]; then
		MFG_MODE='Off'
	else
		MFG_MODE='On'
	fi

	if [ "$MFG_MODE" = 'On' ]; then
		#disable watchdog
		disable_watchdog
		to_mfg_mode 'clear_rw'
		# Stop link manager
		exit 0
	fi
}

handle_e_btn_enable_edid_update()
{
	if [ "$MULTICAST_ON" = 'y' ]; then
		EDID_USE='primary'
	fi
}

handle_e_btn_toggle_snoop()
{
	ipc @v_lm_set s ve_toggle_snoop
}

handle_e_btn_toggle_snoop_on()
{
	ipc @v_lm_set s ve_toggle_snoop:y
}

handle_e_btn_toggle_snoop_off()
{
	ipc @v_lm_set s ve_toggle_snoop:n
}

