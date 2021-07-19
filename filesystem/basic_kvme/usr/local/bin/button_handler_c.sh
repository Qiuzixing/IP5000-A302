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
BTN1_LONG="e_btn_request_usb"
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
if [ "$SHARE_USB" = 'n' ]; then
	BTN1_LONG="e_btn_ignore"
fi
###########################################

read_reg()
{
	# $1: reg.
	local r=`printf "0x%X" $1`
	local reg=`io 0 $r`
	reg=${reg#*=} #remove prefix ending with '='
	reg=${reg:1}  #remove first ' ' space
	echo "0x$reg"
}

handle_e_astparam_overwrite()
{
	# Following is a sample code for AST152X client. Assign BTN_INIT="e_astparam_overwrite" to take effect.
	# Uses GPIOM0 to overwrite V_OUTPUT_TIMING_CONVERT setting.
	# If GPIOM0 is 0, then uses the value from astparam v_output_timing_convert_overwrite.
	# If GPIOM0 is 1, then don't touch V_OUTPUT_TIMING_CONVERT which will use v_output_timing_convert.
	#
	#Client:
	# 1.Make sure SCU84[24] is 0 [FIXME]
	# 2.Make sure GPIO07C[0] is 0 (input mode) [FIXME]
	# 3.Read GPIO078[0] check the value. 1:From astparam. 0: force 1080p60
	local _skip=$(( `read_reg 0x1E780078` & 0x1 ))
	local _V_OUTPUT_TIMING_CONVERT="$V_OUTPUT_TIMING_CONVERT"

	if [ "$_skip" = '1' ]; then
		return
	fi

	V_OUTPUT_TIMING_CONVERT=`astparam g v_output_timing_convert_overwrite`
	if echo "$V_OUTPUT_TIMING_CONVERT=" | grep -q "not defined" ; then
		V_OUTPUT_TIMING_CONVERT=`astparam r v_output_timing_convert_overwrite`
		if echo "$V_OUTPUT_TIMING_CONVERT" | grep -q "not defined" ; then
			V_OUTPUT_TIMING_CONVERT="$_V_OUTPUT_TIMING_CONVERT"
		fi
	fi
}

handle_e_btn_ignore()
{
	return
}

handle_e_btn_toggle_link()
{
	# When UI_FEATURE is ON, we ignore the "connect" button request, to avoid state confusing
	# between GUI and LM. We shouldn't ignore 'disconnect" button request.
	if [ "$UI_FEATURE" = 'y' ] && [ "$STATE" = 's_idle' ] && [ "$ACCESS_ON" = 'n' ]; then
		echo "Ignore connect request when UI_FEATURE is ON"
		return
	fi

	_link_on_off
}

handle_e_btn_request_usb()
{
	handle_e_usb_request_on_off
}

handle_e_btn_toggle_video_profile()
{
	ipc @v_lm_set s ve_toggle_video_profile
}

handle_e_btn_toggle_video_anti_dither()
{
	ipc @v_lm_set s ve_toggle_video_anti_dither
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

###########################################
#
# This function will toggle the ch_select and multicast_ip in sequence.
# User press the button one at a time to toggle to next channel in circle.
# Idle for 2 seconds to confirm.

# state: off, selecting, idle,
BTN_CH_SELECT_STATE='off'
BTN_CH_SELECT_CURRENT_CH=`astconfig channel`
#_btnto()
#{
#	sleep $1
#	ast_send_event -1 e_btn_ch_select_4bits_timeout
#}
echo '#!/bin/sh' > _btnto.sh
echo '' >> _btnto.sh
echo 'sleep $1' >> _btnto.sh
echo 'ast_send_event -1 e_btn_ch_select_4bits_timeout' >> _btnto.sh
chmod a+x _btnto.sh

_refire_ch_select_timer()
{
	pkill _btnto
	_btnto.sh $1 &
}

_ch_hex2dec()
{
	local _ch="$1"

	if [ "$_ch" = '0000' ]; then
		echo "00"
	elif [ "$_ch" = '0001' ]; then
		echo "01"
	elif [ "$_ch" = '0010' ]; then
		echo "02"
	elif [ "$_ch" = '0011' ]; then
		echo "03"
	elif [ "$_ch" = '0100' ]; then
		echo "04"
	elif [ "$_ch" = '0101' ]; then
		echo "05"
	elif [ "$_ch" = '0110' ]; then
		echo "06"
	elif [ "$_ch" = '0111' ]; then
		echo "07"
	elif [ "$_ch" = '1000' ]; then
		echo "08"
	elif [ "$_ch" = '1001' ]; then
		echo "09"
	elif [ "$_ch" = '1010' ]; then
		echo "10"
	elif [ "$_ch" = '1011' ]; then
		echo "11"
	elif [ "$_ch" = '1100' ]; then
		echo "12"
	elif [ "$_ch" = '1101' ]; then
		echo "13"
	elif [ "$_ch" = '1110' ]; then
		echo "14"
	elif [ "$_ch" = '1111' ]; then
		echo "15"
	fi
}

_ch_select_next()
{
	local _ch="$1"

	if [ "$_ch" = '0000' ]; then
		echo "0001"
	elif [ "$_ch" = '0001' ]; then
		echo "0010"
	elif [ "$_ch" = '0010' ]; then
		echo "0011"
	elif [ "$_ch" = '0011' ]; then
		echo "0100"
	elif [ "$_ch" = '0100' ]; then
		echo "0101"
	elif [ "$_ch" = '0101' ]; then
		echo "0110"
	elif [ "$_ch" = '0110' ]; then
		echo "0111"
	elif [ "$_ch" = '0111' ]; then
		echo "1000"
	elif [ "$_ch" = '1000' ]; then
		echo "1001"
	elif [ "$_ch" = '1001' ]; then
		echo "1010"
	elif [ "$_ch" = '1010' ]; then
		echo "1011"
	elif [ "$_ch" = '1011' ]; then
		echo "1100"
	elif [ "$_ch" = '1100' ]; then
		echo "1101"
	elif [ "$_ch" = '1101' ]; then
		echo "1110"
	elif [ "$_ch" = '1110' ]; then
		echo "1111"
	elif [ "$_ch" = '1111' ]; then
		echo "0000"
	fi
}
handle_e_btn_ch_select_4bits()
{
	if [ "$BTN_CH_SELECT_STATE" = 'off' ]; then
		# To "selecting" state and shows current channel
		BTN_CH_SELECT_STATE='selecting'
		BTN_CH_SELECT_CURRENT_CH="$CH_SELECT"
		osd_on_tl.sh Channel:`_ch_hex2dec $BTN_CH_SELECT_CURRENT_CH`
		echo "current ch=$BTN_CH_SELECT_CURRENT_CH"
		_refire_ch_select_timer 2
	elif [ "$BTN_CH_SELECT_STATE" = 'selecting' ] || [ "$BTN_CH_SELECT_STATE" = 'idle' ] ; then
		# select to next channel
		BTN_CH_SELECT_STATE='selecting'
		BTN_CH_SELECT_CURRENT_CH=`_ch_select_next $BTN_CH_SELECT_CURRENT_CH`
		osd_on_tl.sh Channel:`_ch_hex2dec $BTN_CH_SELECT_CURRENT_CH`
		echo "current ch=$BTN_CH_SELECT_CURRENT_CH"
		_refire_ch_select_timer 2
	else
		echo "Unknown BTN_CH_SELECT_STATE($BTN_CH_SELECT_STATE)??"
	fi
}

handle_e_btn_ch_select_4bits_timeout()
{
	if [ "$BTN_CH_SELECT_STATE" = 'off' ]; then
		echo "Do nothing"
	elif [ "$BTN_CH_SELECT_STATE" = 'selecting' ]; then
		# To "idle" state.
		BTN_CH_SELECT_STATE='idle'
		_refire_ch_select_timer 1
	elif [ "$BTN_CH_SELECT_STATE" = 'idle' ]; then
		# Confirm change and go to "off" state
		BTN_CH_SELECT_STATE='off'
		osd_off.sh 0
		if [ "$BTN_CH_SELECT_CURRENT_CH" != "$CH_SELECT" ]; then
			ast_send_event -1 e_reconnect::$BTN_CH_SELECT_CURRENT_CH
		fi
	fi
}
###########################################

handle_e_hotkey_0()
{
	handle_e_btn_toggle_link
}

handle_e_hotkey_1()
{
	handle_e_btn_request_usb
}

handle_e_hotkey_2()
{
	handle_e_btn_toggle_video_profile
}

handle_e_hotkey_3()
{
	echo "Hotkey3" > /var/info_local
	e e_stop_kmoip	
	return
}

handle_e_hotkey_4()
{
	echo "Hotkey4" > /var/info_local
	e e_start_kmoip
	return
}

handle_e_hotkey_5()
{
	return
}

handle_e_hotkey_6()
{
	return
}

handle_e_hotkey_7()
{
	return
}
