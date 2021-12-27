#!/bin/sh

##
## Copyright (c) 2017
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##

######  GPIO & LED defines
LED_LINK="led_link"
LED_PWR="led_pwr"

#######	A30 LED defines
LINK_ON_G="led_link_g"
NET_ON_G="led_status_g"
NET_ON_R="led_status_r"
BOARD_ON_R="led_on_r"
BOARD_ON_G="led_on_g"
IPE5000W_INPUT_SOURCE_LED="hdmi"
IPD5000W_BOARD_ON_R="on_red"
IPD5000W_BOARD_ON_G="on_green"
IPD5000W_HDMI_LED="led_hdmi"
NET_IP_FALLBACK_BLINK_ON='500'
NET_IP_FALLBACK_BLINK_OFF='10000'
NET_FLAG_ME_BLINK_ON='250'
NET_FLAG_ME_BLINK_OFF='250'
BOARD_DOWNLOAD_BLINK_ON='250'
BOARD_DOWNLOAD_BLINK_OFF='1000'
BOARD_STANDBY_BLINK_ON='500'
BOARD_STANDBY_BLINK_OFF='4000'
#######	A30 LED STATUS									net_led_status_index	board_led_status_index
FLAG_ME_STATUS="flag_me"								#first_priority-1		#second_priority-2
DHCP_FAIL="ip_fallback"									#second_priority-2		#third_priority-3
IP_VALID="ip_valid"										#second_priority-3		#NULL
SECURITY_PROHIBIT_IP="security_prohibit_ip"				#second_priority-4		#third_priority-4
GET_IP_FAIL="get_ip_fial"								#second_priority-5		#NULL
STANDBY="standby"										#NULL					#fourth_priority-6
FIRMWARE_DOWNLOAD="firmware_background_download"		#NULL					#first_priority-1
POWER_ON="power_on"										#NULL					#fourth_priority-5
######  VideoIP defines
VIDEO_SYS_PATH="/sys/devices/platform/videoip"
HDMIRX_SYS_PATH="/sys/devices/platform/$V_RX_DRV"
DISPLAY_SYS_PATH="/sys/devices/platform/display"
I2S_SYS_PATH="/sys/devices/platform/1500_i2s"
IR_SYS_PATH="/sys/devices/platform/IRoIP"
MAC_SYS_PATH="/sys/devices/platform/ftgmac"
GPIO_SYS_PATH="/sys/class/leds"
KMOIP_SYS_PATH="/sys/devices/platform/kmoip"
VHUB_SYS_PATH="/sys/devices/platform/vhci_hcd"
SCU_SYS_PATH="/sys/devices/platform/scu"
CEC_SYS_PATH="/sys/devices/platform/cec"
#watchdog defines & functions
WATCHDOG_AVAILABLE='n'

DEVICE_STATUS_FILE="/var/ast_device_status"
OSD_FROM_GUI='y'
JUMBO_MTU='8000'
IEEE8021X_PATH='/data/configs/kds-7/secure'
IEEE8021X_JSON_PATH="$IEEE8021X_PATH/ieee802_1x_setting.json"
LOG_JSON_PATH="/data/configs/kds-7/log/log_setting.json"

#. ip_mapping.sh
. bash/utilities.sh

a30_led_on()
{
	case $MODEL_NUMBER in
		WP-DEC7)
			case $1 in
				$BOARD_ON_G)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/trigger
					echo 0 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/brightness
				;;
				$BOARD_ON_R)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/trigger
					echo 0 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/brightness
				;;
				$LINK_ON_G)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/trigger
					echo 0 > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/brightness
				;;
				*)
				;;
			esac
			return
		;;
		WP-SW2-EN7)
			case $1 in
				$LINK_ON_G)
					if [ $IPE5000W_INPUT_SOURCE_LED = 'hdmi' ];then
						ipc @m_lm_set s set_led_control:0:1
					else
						ipc @m_lm_set s set_led_control:1:1
					fi
					return
				;;
				*)
				;;
			esac
		;;
		*)
		;;
	esac
	echo none > ${GPIO_SYS_PATH}/$1/trigger
	echo 0 > ${GPIO_SYS_PATH}/$1/brightness
}

a30_led_off()
{
	case $MODEL_NUMBER in
		WP-DEC7)
			case $1 in
				$BOARD_ON_G)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/trigger
					echo 1 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/brightness
				;;
				$BOARD_ON_R)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/trigger
					echo 1 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/brightness
				;;
				$LINK_ON_G)
					echo none > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/trigger
					echo 1 > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/brightness
				;;
				*)
				;;
			esac
			return
		;;
		*)
		;;
	esac
	echo none > ${GPIO_SYS_PATH}/$1/trigger
	echo 1 > ${GPIO_SYS_PATH}/$1/brightness
}

a30_led_blink()
{
	case $MODEL_NUMBER in
		WP-DEC7)
			case $1 in
				$BOARD_ON_G)
					echo $2 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/delay_off
					echo $3 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/delay_on
					echo timer > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_G/trigger
				;;
				$BOARD_ON_R)
					echo $2 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/delay_off
					echo $3 > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/delay_on
					echo timer > ${GPIO_SYS_PATH}/$IPD5000W_BOARD_ON_R/trigger
				;;
				$LINK_ON_G)
					echo $2 > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/delay_off
					echo $3 > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/delay_on
					echo timer > ${GPIO_SYS_PATH}/$IPD5000W_HDMI_LED/trigger
				;;
				*)
				;;
			esac
			return
		;;
		*)
		;;
	esac
	echo $2 > ${GPIO_SYS_PATH}/$1/delay_off
	echo $3 > ${GPIO_SYS_PATH}/$1/delay_on
	echo timer > ${GPIO_SYS_PATH}/$1/trigger
}

net_status_detect()
{
	FIR_PRIORITY_NET_STATUS=`astparam g fir_priority_net_status`
	repeat_net_lighting_flag=`astparam g repeat_net_lighting_flag`
	if [ "$FIR_PRIORITY_NET_STATUS" = "$FLAG_ME_STATUS" ];then
		if [ $repeat_net_lighting_flag -eq 1 ];then
			return 0
		fi
		astparam s repeat_net_lighting_flag 1
		return 1
	fi

	SEC_PRIORITY_NET_STATUS=`astparam g sec_priority_net_status`
	case $SEC_PRIORITY_NET_STATUS in
		$DHCP_FAIL)
			if [ $repeat_net_lighting_flag -eq 2 ];then
				return	0
			fi
			astparam s repeat_net_lighting_flag 2
			return 2
		;;
		$IP_VALID)
			if [ $repeat_net_lighting_flag -eq 3 ];then
				return 0
			fi
			astparam s repeat_net_lighting_flag 3
			return 3
		;;
		$SECURITY_PROHIBIT_IP)
			if [ $repeat_net_lighting_flag -eq 4 ];then
				return 0
			fi
			astparam s repeat_net_lighting_flag 4
			return 4
		;;
		$GET_IP_FAIL)
			if [ $repeat_net_lighting_flag -eq 5 ];then
				return 0
			fi
			astparam s repeat_net_lighting_flag 5
			return 5
		;;
		*)
		;;
	esac
}

board_status_detect()
{
	FIR_PRIORITY_STATUS=`astparam g fir_priority_board_status`
	repeat_board_lighting_flag=`astparam g repeat_board_lighting_flag`
	if [ "$FIR_PRIORITY_STATUS" = "$FIRMWARE_DOWNLOAD" ];then
		if [ $repeat_board_lighting_flag -eq 1 ];then
			return 0
		fi
		astparam s repeat_board_lighting_flag 1
		return 1
	fi

	SEC_PRIORITY_STATUS=`astparam g sec_priority_board_status`
	if [ "$SEC_PRIORITY_STATUS" = "$FLAG_ME_STATUS" ];then
		if [ $repeat_board_lighting_flag -eq 2 ];then
			return 0
		fi
		astparam s repeat_board_lighting_flag 2
		return 2
	fi

	THIRD_PRIORITY_STATUS=`astparam g third_priority_board_status`
	case $THIRD_PRIORITY_STATUS in
		$DHCP_FAIL)
			if [ $repeat_board_lighting_flag -eq 3 ];then
				return	0
			fi
			astparam s repeat_board_lighting_flag 3
			return 3
		;;
		$SECURITY_PROHIBIT_IP)
			if [ $repeat_board_lighting_flag -eq 4 ];then
				return 0
			fi
			astparam s repeat_board_lighting_flag 4
			return 4
		;;
		*)
		;;
	esac

	FOURTH_PRIORITY_STATUS=`astparam g fourth_priority_board_status`
	case $FOURTH_PRIORITY_STATUS in
		$POWER_ON)
			if [ $repeat_board_lighting_flag -eq 5 ];then
				return 0
			fi
			astparam s repeat_board_lighting_flag 5
			return 5
		;;
		$STANDBY)
			if [ $repeat_board_lighting_flag -eq 6 ];then
				return	0
			fi
			astparam s repeat_board_lighting_flag 6
			return	6
		;;
		*)
		;;
	esac
}

status_light_up()
{
	case $1 in
		0)
		;;
		1)
			a30_led_off $NET_ON_R
			a30_led_off $NET_ON_G
			a30_led_blink $NET_ON_G $NET_FLAG_ME_BLINK_ON $NET_FLAG_ME_BLINK_OFF
		;;
		2)
			a30_led_off $NET_ON_R
			a30_led_off $NET_ON_G
			a30_led_blink $NET_ON_G $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF
			a30_led_blink $NET_ON_R $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF

		;;
		3)
			a30_led_off $NET_ON_R
			a30_led_on $NET_ON_G
		;;
		4)
			a30_led_off $NET_ON_G
			a30_led_blink $NET_ON_R $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF
			a30_led_on $NET_ON_R
		;;
		5)
			a30_led_off $NET_ON_G
			a30_led_off $NET_ON_R
		;;
		*)
		;;
	esac

	case $2 in
		0)
		;;
		1)
			a30_led_off $BOARD_ON_R
			a30_led_off $BOARD_ON_G
			a30_led_blink $BOARD_ON_G $BOARD_DOWNLOAD_BLINK_ON $BOARD_DOWNLOAD_BLINK_OFF
		;;
		2)
			a30_led_off $BOARD_ON_R
			a30_led_off $BOARD_ON_G
			a30_led_blink $BOARD_ON_G $NET_FLAG_ME_BLINK_ON $NET_FLAG_ME_BLINK_OFF
		;;
		3)
			a30_led_off $BOARD_ON_R
			a30_led_off $BOARD_ON_G
			a30_led_blink $BOARD_ON_G $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF
			a30_led_blink $BOARD_ON_R $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF
		;;
		4)
			a30_led_off $BOARD_ON_R
			a30_led_off $BOARD_ON_G
			a30_led_blink $BOARD_ON_R $NET_IP_FALLBACK_BLINK_ON $NET_IP_FALLBACK_BLINK_OFF
		;;
		5)
			a30_led_on $BOARD_ON_R
			a30_led_on $BOARD_ON_G
		;;
		6)
			a30_led_off $BOARD_ON_R
			a30_led_off $BOARD_ON_G
			a30_led_blink $BOARD_ON_G $BOARD_STANDBY_BLINK_ON $BOARD_STANDBY_BLINK_OFF
		;;
		*)
		;;
	esac
}

control_net_led_status()
{
	net_status_detect
	status_light_up $? 0
}

control_board_led_status()
{
	board_status_detect
	status_light_up 0 $?
}

control_net_and_board_led_status()
{
	net_status_detect
	tmp=$?
	board_status_detect
	status_light_up $tmp $?
}

ipe5000w_led_tmp_define()
{
	BOARD_ON_R="led_on_g"
	BOARD_ON_G="led_on_r"
}

kill_process()
{
	#2021.10.12 qzx:If the process starts with parameters, the pkill -9 process_name cannot be killed.You must use kill -9 to kill the process started with parameters
	PROCESS_PID=`ps | grep $1 | grep -v grep | awk '{print $1}'`
	if [ $PROCESS_PID ];then
		kill -9 $PROCESS_PID
	fi
}

led_display_num()
{
	kill_process led_display
	#2021.10.12 qzx:Because the read value is a string such as 0001, you need to remove the previous 0, otherwise it will not be a normal number
	tmp=`echo -e $LED_DISPLAY_CH_SELECT_V | sed -r 's/0*([0-9])/\1/'`
	case "$MODEL_NUMBER" in
		WP-SW2-EN7)
			ipc @m_lm_set s set_lcd_content:$tmp
		;;
		WP-DEC7)
			led_display -n $tmp &
		;;
		*)
		;;
	esac
}

init_watchdog()
{
	if [ -d /sys/devices/platform/watchdog ]; then
		echo "watchdog available"
		WATCHDOG_AVAILABLE='y'
		# set timeout will enable the watchdog, but won't touch
		# the enable flag. So, we should enable the flag here.
		echo 'yes' > /sys/devices/platform/watchdog/enable
		# Set watchdog timeout to 50 secs
		echo 50 > /sys/devices/platform/watchdog/timeout
	else
		echo "watchdog unavailable"
		WATCHDOG_AVAILABLE='n'
	fi
}

#enable_watchdog()
#{
#	if [ "$WATCHDOG_AVAILABLE" = 'y' ]; then
#		echo yes > /sys/devices/platform/watchdog/enable
#	fi
#}

disable_watchdog()
{
	if [ "$WATCHDOG_AVAILABLE" = 'y' ]; then
		echo no > /sys/devices/platform/watchdog/enable
	fi
}

tickle_watchdog()
{
	if [ "$WATCHDOG_AVAILABLE" = 'y' ]; then
		echo > /sys/devices/platform/watchdog/tickle
	fi
}

start_watchdog_daemon()
{
	if [ "$WATCHDOG_AVAILABLE" = 'y' ]; then
		_t="$1"
		if [ -z "$1" ]; then
			# tickle every 10 sec by default
			_t='10'
		fi
		watchdog_daemon.sh "$_t" &
		# Set watchdog timeout to shorter 35 secs
		echo 35 > /sys/devices/platform/watchdog/timeout
	fi
}

stop_watchdog_daemon()
{
	if [ "$WATCHDOG_AVAILABLE" = 'y' ]; then
		# Set watchdog timeout back to 50 secs
		echo 50 > /sys/devices/platform/watchdog/timeout
		pkill watchdog_daemon 2> /dev/null
	fi
}

###### GPIO & LED functions
led_on()
{
	if [ "$FPGA_TEST" = '1' ]; then
		return
	fi

	# $1 is the led name to be access. Ex: $LED_LINK
	#echo none > ${GPIO_SYS_PATH}/$1/trigger
	#echo 1 > ${GPIO_SYS_PATH}/$1/brightness
}
led_off()
{
	if [ "$FPGA_TEST" = '1' ]; then
		return
	fi

	# $1 is the led name to be access. Ex: $LED_LINK
	#echo none > ${GPIO_SYS_PATH}/$1/trigger
	#echo 0 > ${GPIO_SYS_PATH}/$1/brightness
}
led_blink()
{
	if [ "$FPGA_TEST" = '1' ]; then
		return
	fi
	# $1 is the led name to be access. Ex: $LED_LINK
	#echo timer > ${GPIO_SYS_PATH}/$1/trigger
}

select_v_input_by_current()
{
	echo `cat $VIDEO_SYS_PATH/rx`
}

select_v_input_by_swap_current()
{
	# Default 'current' value is decided in SCU and VE driver,
	# which uses similar decision flow as select_v_input().
	local _t=`cat $VIDEO_SYS_PATH/rx`
	case "$_t" in
		digital)
			echo "analog"
		;;
		analog)
			echo "digital"
		;;
		*)
			echo "digital"
		;;
	esac
}

select_v_input_from_gpio_old()
{
	# Request GPIO value
	echo 0 > ${GPIO_SYS_PATH}/v_input/brightness 2>/dev/null
	# Get the value now
	V_INPUT_FROM_GPIO=`cat ${GPIO_SYS_PATH}/v_input/brightness 2>/dev/null`
	# GPIO num mapping is based on SW port# define
	case "$V_INPUT_FROM_GPIO" in
		0)
			echo 'digital'
		;;
		1)
			echo 'analog'
		;;
		*)
			echo 'digital'
		;;
	esac
}

select_v_input_from_gpio()
{
	local _read_done=0
	local _cnt_zero=0
	local _cnt_one=0

	usleep 200000
	while [ $_read_done -lt 3 ]; do
		# Request GPIO value
		echo 0 > ${GPIO_SYS_PATH}/v_input/brightness 2>/dev/null
		# Get the value now
		V_INPUT_FROM_GPIO=`cat ${GPIO_SYS_PATH}/v_input/brightness 2>/dev/null`
		case "$V_INPUT_FROM_GPIO" in
			0)
				_cnt_zero="$(( $_cnt_zero + 1 ))"
				_read_done="$(( $_read_done + 1 ))"
			;;
			1)
				_cnt_one="$(( $_cnt_one + 1 ))"
				_read_done="$(( $_read_done + 1 ))"
			;;
		esac
	done

	# GPIO num mapping 0:digital 1:analog
	if [ $_cnt_zero -gt $_cnt_one ]; then
		echo 'digital'
	else
		echo 'analog'
	fi
}

select_v_input_from_v_type()
{
	# 0: VGA
	# 1: disable
	# 2: HDMI
	# 3: DVI
	case "$V_TYPE" in
		0)
			echo "analog"
		;;
		*)
			echo "digital"
		;;
	esac
}

select_v_input_detect_sync()
{
	# $1: swap, gpio, refresh
	local _type="$1"

	if [ "$V_DUAL_INPUT" = "n" ]; then
		select_v_input_from_v_type
		return
	fi

	# For dual input auto selection:
	case "$_type" in
		swap) # swap current port
			select_v_input_by_swap_current
		;;
		gpio) # select from GPIO, for GPIO changes
			select_v_input_from_gpio
		;;
		refresh) # as it is, means current.
			# May not be equal to GPIO setting when user used e_v_select_input_xxxx to runtime change input port.
			select_v_input_by_current
		;;
		*)
			select_v_input_by_current
		;;
	esac
}

select_v_input()
{
	# $1: swap, gpio, refresh
	local _type="$1"

	# SOC_VER < 3 doesn't support dual port input.
	if [ "$SOC_VER" -lt '3' ]; then
		select_v_input_by_current
		return
	fi

	# Specific v_input_select has highest priority.
	case "$V_INPUT_SELECT" in
		fixed)
			select_v_input_from_v_type
		;;
		auto) # for user don't like auto swap port.
			select_v_input_from_gpio
		;;
		detect_sync)
			select_v_input_detect_sync "$_type"
		;;
		*)
			echo "digital"
		;;
	esac
}

select_a_input_from_gpio_old()
{
	# Request GPIO value
	echo 0 > ${GPIO_SYS_PATH}/a_input/brightness 2>/dev/null
	# Get the value now
	A_INPUT_FROM_GPIO=`cat ${GPIO_SYS_PATH}/a_input/brightness 2>/dev/null`
	# GPIO num mapping 0:digital 1:analog codec
	case "$A_INPUT_FROM_GPIO" in
		0)
			echo 'hdmi'
		;;
		1)
			echo 'analog'
		;;
		*)
			echo 'hdmi'
		;;
	esac
}

select_a_input_from_gpio()
{
	local _read_done=0
	local _cnt_zero=0
	local _cnt_one=0

	usleep 200000
	while [ $_read_done -lt 3 ]; do
		# Request GPIO value
		echo 0 > ${GPIO_SYS_PATH}/a_input/brightness 2>/dev/null
		# Get the value now
		A_INPUT_FROM_GPIO=`cat ${GPIO_SYS_PATH}/a_input/brightness 2>/dev/null`
		case "$A_INPUT_FROM_GPIO" in
			0)
				_cnt_zero="$(( $_cnt_zero + 1 ))"
				_read_done="$(( $_read_done + 1 ))"
			;;
			1)
				_cnt_one="$(( $_cnt_one + 1 ))"
				_read_done="$(( $_read_done + 1 ))"
			;;
		esac
	done

	# GPIO num mapping 0:digital 1:analog codec
	if [ $_cnt_zero -gt $_cnt_one ]; then
		echo 'hdmi'
	else
		echo 'analog'
	fi
}

select_a_input_per_v_input()
{
	local _t=`cat $VIDEO_SYS_PATH/rx`
	case "$_t" in
		digital)
			echo "hdmi"
		;;
		analog)
			echo "analog"
		;;
		*)
			#echo "Unsupported video input type?! ($_t)"
			select_a_input_from_gpio
		;;
	esac
}

select_a_input_auto()
{
	# decide audio port. auto mode.
	# When video port is HDMI/DVI, select audio port per GPIO.
	# When video port is VGA, always select analog codec audio port.
	local _t=`cat $VIDEO_SYS_PATH/rx`
	case "$_t" in
		digital)
			select_a_input_from_gpio
		;;
		analog)
			echo "analog"
		;;
		*)
			#echo "Unsupported video input type?! ($_t)"
			select_a_input_from_gpio
		;;
	esac
}

select_a_input()
{
	# SOC_VER < 2 doesn't support dual port input.
	if [ "$SOC_VER" -lt '2' ]; then
		echo "hdmi"
		return
	fi

	case "$A_IO_SELECT" in
		hdmi)
			echo "hdmi"
		;;
		analog)
			echo "analog"
		;;
		auto) # HDMI per GPIO, VGA fixed codec.
			select_a_input_auto
		;;
		auto_1) # always per GPIO.
			select_a_input_from_gpio
		;;
		auto_2) # always per video port. HDMI/DVI use HDMI audio, VGA use codec audio.
			select_a_input_per_v_input
		;;
		*)
			select_a_input_from_gpio
		;;
	esac
}

apply_profile_config()
{
	local _old_name="${PROFILE_FOLDER}/${PROFILE_NAME}"
	local _port='0'
	PROFILE_FOLDER="digital"

	case "$1" in
		digital)
			echo "select digital source"
			PROFILE_FOLDER='digital'
			_port='0'
		;;
		analog)
			echo "select analog source"
			PROFILE_FOLDER='analog'
			_port='1'
		;;
		*)
			echo "source undetermined"
			PROFILE_FOLDER='digital'
			_port='0'
		;;
	esac

	if [ "$PROFILE" = 'auto' ]; then
		# Auto choose based on link mode.
		# Assume $ETH_LINK_MODE is valid and correct at this point.
		case "$ETH_LINK_MODE" in
			'10M')
				PROFILE_NAME='10M'
			;;
			'100M')
				PROFILE_NAME='100M'
			;;
			*)
				# 1G and unknown
				PROFILE_NAME='default'
			;;
		esac
	else
		PROFILE_NAME="$PROFILE"
	fi

	# Video Wall profile applies ONLY to SoC OP MODE 1 and must overwrite PROFILE setting.
	if [ "$SOC_OP_MODE" = '1' ]; then
		if [ "$EN_VIDEO_WALL" = 'y' ] && [ "$VW_VAR_MAX_ROW" != '0' ]; then
			echo "select video wall profile"
			if [ "$VW_VAR_MAX_ROW" -gt '3' ]; then
				PROFILE_NAME="video_wall_3"
			else
				PROFILE_NAME="video_wall_$VW_VAR_MAX_ROW"
			fi
		fi
	fi

	if ! [ -d ./V$SOC_OP_MODE/profile/"${PROFILE_FOLDER}/${PROFILE_NAME}" ]; then
		echo "Profile (V${SOC_OP_MODE}/profile/${PROFILE_FOLDER}/${PROFILE_NAME}) not exists!? Use default profile instead."
		PROFILE_NAME='default'
	fi

	if [ "${PROFILE_FOLDER}/${PROFILE_NAME}" != "$_old_name" ]; then
		printf "\n\nProfile=${PROFILE_FOLDER}/${PROFILE_NAME} vs $_old_name\n\n"
		cp -f ./V$SOC_OP_MODE/profile/"${PROFILE_FOLDER}/${PROFILE_NAME}"/* . 2>/dev/null
		# video_config should wait for the videoip.ko initialized
		video_config -s V2 -p "$_port"
		# video driver starts mode detection after this point.
	fi
}

post_ip_addr()
{
	case "$MY_IP" in
		Unknown)
			return 0
		;;
		e_ip_got*)
			# We already got the IP address, send it again.
			ast_send_event -1 "$MY_IP"
		;;
		*)
			ast_send_event -1 "e_ip_got::$MY_IP"
		;;
	esac
}

config_netmask_n_gw()
{
	ifconfig eth0:stat netmask "$NETMASK"
	route add default gw "$GATEWAYIP"
}

config_ip_addr()
{
	var="$1"
	local ipaddr=''

	MY_IP='Unknown'
	echo "IP_MODE=$IP_MODE"
	if [ "$IP_MODE" = 'autoip' ]; then
		ipaddr=`astparam g asthostip`
		if [ "$ipaddr" = "\"asthostip\" not defined" ]; then
			/sbin/avahi-autoipd -D --force-bind eth0 --script=/etc/avahi/avahi-autoipd.action
		else
			/sbin/avahi-autoipd -D --force-bind --start=$ipaddr eth0 --script=/etc/avahi/avahi-autoipd.action
		fi
	elif [ "$IP_MODE" = 'dhcp' ]; then
		#turn on udhcp will stop autoip from setting ip. Use -w in autoipd if you want to turn on udhcpc
		udhcpc -f &
	elif [ "$IP_MODE" = 'static' ]; then
		ifconfig eth0:stat "$IPADDR"
		config_netmask_n_gw
		MY_IP="$IPADDR"
	fi
}

# 0.06 seconds
#multicast_ip_from_dipswitch()
#{
#	#Query the gpio value by writing 0 to it
#	echo 0 > ${GPIO_SYS_PATH}/ch0/brightness 2>/dev/null
#	echo 0 > ${GPIO_SYS_PATH}/ch1/brightness 2>/dev/null
#	echo 0 > ${GPIO_SYS_PATH}/ch2/brightness 2>/dev/null
#	echo 0 > ${GPIO_SYS_PATH}/ch3/brightness 2>/dev/null
#	#Get the value now
#	ch0=`cat ${GPIO_SYS_PATH}/ch0/brightness 2>/dev/null`
#	ch1=`cat ${GPIO_SYS_PATH}/ch1/brightness 2>/dev/null`
#	ch2=`cat ${GPIO_SYS_PATH}/ch2/brightness 2>/dev/null`
#	ch3=`cat ${GPIO_SYS_PATH}/ch3/brightness 2>/dev/null`
#
#	echo "${MULTICAST_IP_PREFIX}$ch0.$ch1$ch2$ch3"
#}

# 0.06 seconds. sysfs read takes time
refresh_4bits_ch()
{
	if [ "$FPGA_TEST" = '1' ]; then
		CH0='0'
		CH1='0'
		CH2='0'
		CH3='0'
		return
	fi

	#Get the value now
	CH0=0
	CH1=0
	CH2=0
	CH3=0
}

foolproof_multicast_ip_prefix()
{
	# Multicast addresses are identified by the pattern “1110”
	# in the first four bits, which corresponds to a first octet of 224 to 239.
	# So, the full range of multicast addresses is from 224.0.0.0 to 239.255.255.255.
	local _prefix=$1
	# Is it a valid integer number?
	if [ "$_prefix" -eq "$_prefix" ] 2>/dev/null; then
		if [ "$_prefix" -ge 224 ] && [ "$_prefix" -le 239 ]; then
			echo "$_prefix"
			return
		fi
	fi
	echo "225"
	echo "ERROR: Invalid MULTICAST_IP_PREFIX ($1)!! Use 225 instead." >&2
}

foolproof_ch_select()
{
	local _ch_select="$1"

	case "$_ch_select" in
		?*.?*.?*.?*)
			# Host can't accept IP format as CH_SELECT
			if [ "$IS_HOST" = 'y' ]; then
				echo "0001"
				echo "ERROR: CH_SELECT ($_ch_select) must be 0000~9999 format" >&2
				return
			fi
			echo "$_ch_select"
		;;
		*)
			# Correct length first.
			case "${#_ch_select}" in
				1)
					_ch_select="000$_ch_select"
				;;
				2)
					_ch_select="00$_ch_select"
				;;
				3)
					_ch_select="0$_ch_select"
				;;
				4)
					# good
				;;
				*)
					echo "0001"
					echo "ERROR: CH_SELECT ($_ch_select) is not in 0000~9999 format" >&2
					return
				;;
			esac
			# Validate number format first.
			#if [ "$_ch_select" -eq "$_ch_select" ] 2>/dev/null; then
			# =~ regex match
			# ^: beginning of pattern
			# $: end of pattern
			if [[ $_ch_select =~ ^[0-9][0-9][0-9][0-9]$ ]]; then
				echo "$_ch_select"
			else
				echo "0001"
				echo "ERROR: CH_SELECT ($_ch_select) is not a number" >&2
			fi
		;;
	esac
}

refresh_ch_params_ex()
{
	if [ "$NO_VIDEO" = 'n' ]; then
		CH_SELECT_V=`astparam g ch_select_v`
		case "$CH_SELECT_V" in
			*'not defined')
				CH_SELECT_V=`astparam r ch_select_v`
				case "$CH_SELECT_V" in
					*'not defined')
						CH_SELECT_V=$CH_SELECT
					;;
					*)
						CH_SELECT_V=`foolproof_ch_select $CH_SELECT_V`
					;;
				esac
			;;
			*)
				CH_SELECT_V=`foolproof_ch_select $CH_SELECT_V`
			;;
		esac
	fi

	if [ "$NO_I2S" = 'n' ]; then
		# CH_SELECT_I2S is legacy astparam
		CH_SELECT_I2S=`astparam g ch_select_i2s`
		if echo "$CH_SELECT_I2S" | grep -q "not defined" ; then
			CH_SELECT_I2S=`astparam r ch_select_i2s`
			if echo "$CH_SELECT_I2S" | grep -q "not defined" ; then
				CH_SELECT_I2S=''
			else
				CH_SELECT_I2S=`foolproof_ch_select $CH_SELECT_I2S`
			fi
		else
			CH_SELECT_I2S=`foolproof_ch_select $CH_SELECT_I2S`
		fi
		CH_SELECT_A=`astparam g ch_select_a`
		case "$CH_SELECT_A" in
			*'not defined')
				CH_SELECT_A=`astparam r ch_select_a`
				case "$CH_SELECT_A" in
					*'not defined')
						if [ -z "$CH_SELECT_I2S" ]; then
							CH_SELECT_A=$CH_SELECT
						else
							CH_SELECT_A=$CH_SELECT_I2S
						fi
					;;
					*)
						CH_SELECT_A=`foolproof_ch_select $CH_SELECT_A`
					;;
				esac
			;;
			*)
				CH_SELECT_A=`foolproof_ch_select $CH_SELECT_A`
			;;
		esac
	fi

	if [ "$NO_USB" = 'n' ] || [ "$NO_KMOIP" = 'n' ]; then
		CH_SELECT_U=`astparam g ch_select_u`
		case "$CH_SELECT_U" in
			*'not defined')
				CH_SELECT_U=`astparam r ch_select_u`
				case "$CH_SELECT_U" in
					*'not defined')
						CH_SELECT_U=$CH_SELECT
					;;
					*)
						CH_SELECT_U=`foolproof_ch_select $CH_SELECT_U`
					;;
				esac
			;;
			*)
				CH_SELECT_U=`foolproof_ch_select $CH_SELECT_U`
			;;
		esac
	fi

	if [ "$NO_SOIP" = 'n' ]; then
		CH_SELECT_SOIP2=`astparam g ch_select_soip2`
		if echo "$CH_SELECT_SOIP2" | grep -q "not defined" ; then
			CH_SELECT_SOIP2=`astparam r ch_select_soip2`
			if echo "$CH_SELECT_SOIP2" | grep -q "not defined" ; then
				CH_SELECT_SOIP2=''
			else
				CH_SELECT_SOIP2=`foolproof_ch_select $CH_SELECT_SOIP2`
			fi
		else
				CH_SELECT_SOIP2=`foolproof_ch_select $CH_SELECT_SOIP2`
		fi
		CH_SELECT_S=`astparam g ch_select_s`
		case "$CH_SELECT_S" in
			*'not defined')
				CH_SELECT_S=`astparam r ch_select_s`
				case "$CH_SELECT_S" in
					*'not defined')
						if [ -z "$CH_SELECT_SOIP2" ]; then
							CH_SELECT_S=$CH_SELECT
						else
							CH_SELECT_S=$CH_SELECT_SOIP2
						fi
					;;
					*)
						CH_SELECT_S=`foolproof_ch_select $CH_SELECT_S`
					;;
				esac
			;;
			*)
				CH_SELECT_S=`foolproof_ch_select $CH_SELECT_S`
			;;
		esac
	fi

	if [ "$NO_IR" = 'n' ]; then
		CH_SELECT_R=`astparam g ch_select_r`
		case "$CH_SELECT_R" in
			*'not defined')
				CH_SELECT_R=`astparam r ch_select_r`
				case "$CH_SELECT_R" in
					*'not defined')
						CH_SELECT_R=$CH_SELECT
					;;
					*)
						CH_SELECT_R=`foolproof_ch_select $CH_SELECT_R`
					;;
				esac
			;;
			*)
				CH_SELECT_R=`foolproof_ch_select $CH_SELECT_R`
			;;
		esac
	fi

	if [ "$NO_PWRBTN" = 'n' ]; then
		CH_SELECT_P=`astparam g ch_select_p`
		case "$CH_SELECT_P" in
			*'not defined')
				CH_SELECT_P=`astparam r ch_select_p`
				case "$CH_SELECT_P" in
					*'not defined')
						CH_SELECT_P=$CH_SELECT
					;;
					*)
						CH_SELECT_P=`foolproof_ch_select $CH_SELECT_P`
					;;
				esac
			;;
			*)
				CH_SELECT_P=`foolproof_ch_select $CH_SELECT_P`
			;;
		esac
	fi
	if [ "$NO_CEC" = 'n' ]; then
		CH_SELECT_C=`astparam g ch_select_c`
		case "$CH_SELECT_C" in
			*'not defined')
				CH_SELECT_C=`astparam r ch_select_c`
				case "$CH_SELECT_C" in
					*'not defined')
						CH_SELECT_C=$CH_SELECT
					;;
					*)
						CH_SELECT_C=`foolproof_ch_select $CH_SELECT_C`
					;;
				esac
			;;
			*)
				CH_SELECT_C=`foolproof_ch_select $CH_SELECT_C`
			;;
		esac
	fi

}

# 0.16 s
refresh_ch_params()
{
	# CH_SELECT
	#    client: for default value only. CAN NOT share with video. Otherwise, hit below bug#CH_SELECT.
	#    host: apply to all CH_SELECT_X
	#
	# Bug#CH_SELECT:
	# 1. boot up client as default 0000 channel.
	# 2. e e_reconnect::0001::v   ==> Expect only V change channel, others remains the same 0000.
	# 3. eth link off, then on
	# 4. handle_e_reconnect_refresh() will refresh_ch_params().
	# 5. bug: If CH_SELECT === CH_SELECT_V, then,
	#    when other CH_SELECT_X will use default CH_SELECT as default value,
	#    CH_SELECT_X will be 0001, not expected 0000.
	# ==> We need separate CH_SELECT_V from CH_SELECT in client.
	_CH_SELECT_SPECIFIED='y'
	CH_SELECT=`astparam g ch_select`
	case "$CH_SELECT" in
		*'not defined')
			CH_SELECT=`astparam r ch_select`
			case "$CH_SELECT" in
				*'not defined')
					#CH_SELECT=`astconfig channel`
					# CH_SELECT="${CH0}${CH1}${CH2}${CH3}"
					CH_SELECT="0001"
					_CH_SELECT_SPECIFIED='n'
				;;
				*)
					CH_SELECT=`foolproof_ch_select $CH_SELECT`
				;;
			esac
		;;
		*)
			CH_SELECT=`foolproof_ch_select $CH_SELECT`
		;;
	esac

	# in case Web UI need it. Init it CH_SELECT_X anyway.
	CH_SELECT_V=$CH_SELECT
	CH_SELECT_U=$CH_SELECT
	CH_SELECT_A=$CH_SELECT
	CH_SELECT_S=$CH_SELECT
	CH_SELECT_R=$CH_SELECT
	CH_SELECT_P=$CH_SELECT
	CH_SELECT_C=$CH_SELECT
	# Client is free to route to any channel per service.
	if [ "$IS_HOST" = 'n' ]; then
		unset _CH_SELECT_SPECIFIED
		refresh_ch_params_ex
	fi

	## A7 can't specify multicast_ip
	##MULTICAST_IP=`astparam g multicast_ip`
	##case "$MULTICAST_IP" in
	##	*'not defined')
	##		MULTICAST_IP=`astparam r multicast_ip`
	##		case "$MULTICAST_IP" in
	##			*'not defined')
	##				#MULTICAST_IP=`multicast_ip_from_dipswitch`
	##				#MULTICAST_IP="${MULTICAST_IP_PREFIX}${CH0}.${CH1}${CH2}${CH3}"
	##				# Use video's channel as default channel.
	##				MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX v $CH_SELECT`
	##			;;
	##			*)
	##			;;
	##		esac
	##	;;
	##	*)
	##	;;
	##esac

	if [ "$IS_HOST" = 'n' ]; then
		# calculate THE_ROW_ID
		#_b3=`expr "$CH_SELECT" : '\(.\)...'`
		#_b2=`expr "$CH_SELECT" : '.\(.\)..'`
		#_b1=`expr "$CH_SELECT" : '..\(.\).'`
		#_b0=`expr "$CH_SELECT" : '...\(.\)'`
		#MY_ROW_ID=`expr $CH0 \* 8 + $CH1 \* 4 + $CH2 \* 2 + $CH3`
		MY_ROW_ID=$(( $CH0 * 8 + $CH1 * 4 + $CH2 * 2 + $CH3 ))
		THE_ROW_ID="$MY_ROW_ID"
		echo "MY_ROW_ID=$MY_ROW_ID"
	fi
	echo "CH_SELECT=$CH_SELECT"
}

refresh_hostname_params()
{
	echo "Refresh hostname params"
	HOSTNAME_ID=`astparam g hostname_id`
	case "$HOSTNAME_ID" in
		*'not defined')
			HOSTNAME_ID=`astparam r hostname_id`
			case "$HOSTNAME_ID" in
				*'not defined')
					if [ "$HOSTNAMEBYDIPSWITCH" = 'y' ]; then
						#HOSTNAME_ID=`astconfig channel`
						HOSTNAME_ID="$CH_SELECT"
					else
						HOSTNAME_ID="$MY_MAC"
					fi
				;;
				*)
				;;
			esac
		;;
		*)
		;;
	esac

	HOSTNAME_CUSTOMIZED=`astparam g hostname_customized`
	case "$HOSTNAME_CUSTOMIZED" in
		*'not defined')
			HOSTNAME_CUSTOMIZED=""
		;;
		*)
		;;
	esac

	if [ "$IS_HOST" = 'y' ]; then
		# For backward compatible with A6.
		# For the case where the host box already has hostname_id configurred.
		if [ "$HOSTNAME_ID" != "$CH_SELECT" ] && [ "$HOSTNAMEBYDIPSWITCH" = 'y' ]; then
			if [ "$_CH_SELECT_SPECIFIED" = 'y' ]; then
				# User uses new A7 API. remove 'hostname_id' for good.
				astparam s hostname_id
				astparam save
				HOSTNAME_ID="$CH_SELECT"
			else
				# User uses old A6 API.
				CH_SELECT=`foolproof_ch_select $HOSTNAME_ID`
				CH_SELECT_V=$CH_SELECT
				CH_SELECT_U=$CH_SELECT
				CH_SELECT_A=$CH_SELECT
				CH_SELECT_S=$CH_SELECT
				CH_SELECT_R=$CH_SELECT
				CH_SELECT_P=$CH_SELECT
				CH_SELECT_C=$CH_SELECT
			fi
			unset _CH_SELECT_SPECIFIED
		fi

		# calculate THE_ROW_ID
		#_b3=`expr "$HOSTNAME_ID" : '\(.\)...'`
		#_b2=`expr "$HOSTNAME_ID" : '.\(.\)..'`
		#_b1=`expr "$HOSTNAME_ID" : '..\(.\).'`
		#_b0=`expr "$HOSTNAME_ID" : '...\(.\)'`
		#MY_ROW_ID=`expr $_b3 \* 8 + $_b2 \* 4 + $_b1 \* 2 + $_b0`
		#MY_ROW_ID=`expr $CH0 \* 8 + $CH1 \* 4 + $CH2 \* 2 + $CH3`
		MY_ROW_ID=$(( $CH0 * 8 + $CH1 * 4 + $CH2 * 2 + $CH3 ))
		#THE_ROW_ID="$MY_ROW_ID" in host side the row id should be VW_VAR_POS_IDX
		echo "MY_ROW_ID=$MY_ROW_ID"
	fi
	echo "HOSTNAME_ID=$HOSTNAME_ID"
}

init_video_wall_params_v2()
{
	VW_VAR_V2_X1=`astparam g vw_v2_x1`
	if echo "$VW_VAR_V2_X1" | grep -q "not defined" ; then
		VW_VAR_V2_X1=`astparam r vw_v2_x1`
		if echo "$VW_VAR_V2_X1" | grep -q "not defined" ; then
			VW_VAR_V2_X1='0'
		fi
	fi

	VW_VAR_V2_Y1=`astparam g vw_v2_y1`
	if echo "$VW_VAR_V2_Y1" | grep -q "not defined" ; then
		VW_VAR_V2_Y1=`astparam r vw_v2_y1`
		if echo "$VW_VAR_V2_Y1" | grep -q "not defined" ; then
			VW_VAR_V2_Y1='0'
		fi
	fi

	VW_VAR_V2_X2=`astparam g vw_v2_x2`
	if echo "$VW_VAR_V2_X2" | grep -q "not defined" ; then
		VW_VAR_V2_X2=`astparam r vw_v2_x2`
		if echo "$VW_VAR_V2_X2" | grep -q "not defined" ; then
			VW_VAR_V2_X2='10000'
		fi
	fi

	VW_VAR_V2_Y2=`astparam g vw_v2_y2`
	if echo "$VW_VAR_V2_Y2" | grep -q "not defined" ; then
		VW_VAR_V2_Y2=`astparam r vw_v2_y2`
		if echo "$VW_VAR_V2_Y2" | grep -q "not defined" ; then
			VW_VAR_V2_Y2='10000'
		fi
	fi
}

video_wall_params_display_v2()
{
	echo "VW_VAR_VER=$VW_VAR_VER"
	echo "VW_VAR_V2_X1=$VW_VAR_V2_X1"
	echo "VW_VAR_V2_Y1=$VW_VAR_V2_Y1"
	echo "VW_VAR_V2_X2=$VW_VAR_V2_X2"
	echo "VW_VAR_V2_Y2=$VW_VAR_V2_Y2"
}

init_video_wall_params()
{
	# MY_ROW_ID must be ready before this call.

	VW_VAR_MAX_ROW=`astparam g vw_max_row`
	if echo "$VW_VAR_MAX_ROW" | grep -q "not defined" ; then
		VW_VAR_MAX_ROW=`astparam r vw_max_row`
		if echo "$VW_VAR_MAX_ROW" | grep -q "not defined" ; then
			VW_VAR_MAX_ROW='0'
		fi
	fi
	VW_VAR_MAX_COLUMN=`astparam g vw_max_column`
	if echo "$VW_VAR_MAX_COLUMN" | grep -q "not defined" ; then
		VW_VAR_MAX_COLUMN=`astparam r vw_max_column`
		if echo "$VW_VAR_MAX_COLUMN" | grep -q "not defined" ; then
			VW_VAR_MAX_COLUMN='0'
		fi
	fi
	VW_VAR_ROW=`astparam g vw_row`
	if echo "$VW_VAR_ROW" | grep -q "not defined" ; then
		VW_VAR_ROW=`astparam r vw_row`
		if echo "$VW_VAR_ROW" | grep -q "not defined" ; then
			VW_VAR_ROW="unknown"
		fi
	fi
	VW_VAR_COLUMN=`astparam g vw_column`
	if echo "$VW_VAR_COLUMN" | grep -q "not defined" ; then
		VW_VAR_COLUMN=`astparam r vw_column`
		if echo "$VW_VAR_COLUMN" | grep -q "not defined" ; then
			VW_VAR_COLUMN="0"
		fi
	fi
	VW_VAR_MONINFO_HA=`astparam g vw_moninfo_ha`
	if echo "$VW_VAR_MONINFO_HA" | grep -q "not defined" ; then
		VW_VAR_MONINFO_HA=`astparam r vw_moninfo_ha`
		if echo "$VW_VAR_MONINFO_HA" | grep -q "not defined" ; then
			VW_VAR_MONINFO_HA="1"
		fi
	fi
	VW_VAR_MONINFO_HT=`astparam g vw_moninfo_ht`
	if echo "$VW_VAR_MONINFO_HT" | grep -q "not defined" ; then
		VW_VAR_MONINFO_HT=`astparam r vw_moninfo_ht`
		if echo "$VW_VAR_MONINFO_HT" | grep -q "not defined" ; then
			VW_VAR_MONINFO_HT="1"
		fi
	fi
	VW_VAR_MONINFO_VA=`astparam g vw_moninfo_va`
	if echo "$VW_VAR_MONINFO_VA" | grep -q "not defined" ; then
		VW_VAR_MONINFO_VA=`astparam r vw_moninfo_va`
		if echo "$VW_VAR_MONINFO_VA" | grep -q "not defined" ; then
			VW_VAR_MONINFO_VA="1"
		fi
	fi
	VW_VAR_MONINFO_VT=`astparam g vw_moninfo_vt`
	if echo "$VW_VAR_MONINFO_VT" | grep -q "not defined" ; then
		VW_VAR_MONINFO_VT=`astparam r vw_moninfo_vt`
		if echo "$VW_VAR_MONINFO_VT" | grep -q "not defined" ; then
			VW_VAR_MONINFO_VT="1"
		fi
	fi
	VW_VAR_POS_IDX=`astparam g vw_pos_idx`
	if echo "$VW_VAR_POS_IDX" | grep -q "not defined" ; then
		VW_VAR_POS_IDX=`astparam r vw_pos_idx`
		if echo "$VW_VAR_POS_IDX" | grep -q "not defined" ; then
			VW_VAR_POS_IDX='unknown'
		fi
	fi
	VW_VAR_H_SHIFT=`astparam g vw_h_shift`
	if echo "$VW_VAR_H_SHIFT" | grep -q "not defined" ; then
		VW_VAR_H_SHIFT=`astparam r vw_h_shift`
		if echo "$VW_VAR_H_SHIFT" | grep -q "not defined" ; then
			VW_VAR_H_SHIFT='0'
		fi
	fi
	VW_VAR_V_SHIFT=`astparam g vw_v_shift`
	if echo "$VW_VAR_V_SHIFT" | grep -q "not defined" ; then
		VW_VAR_V_SHIFT=`astparam r vw_v_shift`
		if echo "$VW_VAR_V_SHIFT" | grep -q "not defined" ; then
			VW_VAR_V_SHIFT='0'
		fi
	fi
	VW_VAR_H_SCALE=`astparam g vw_h_scale`
	if echo "$VW_VAR_H_SCALE" | grep -q "not defined" ; then
		VW_VAR_H_SCALE=`astparam r vw_h_scale`
		if echo "$VW_VAR_H_SCALE" | grep -q "not defined" ; then
			VW_VAR_H_SCALE='0'
		fi
	fi
	VW_VAR_V_SCALE=`astparam g vw_v_scale`
	if echo "$VW_VAR_V_SCALE" | grep -q "not defined" ; then
		VW_VAR_V_SCALE=`astparam r vw_v_scale`
		if echo "$VW_VAR_V_SCALE" | grep -q "not defined" ; then
			VW_VAR_V_SCALE='0'
		fi
	fi
	VW_VAR_POS_MAX_ROW=`astparam g vw_pos_max_row`
	if echo "$VW_VAR_POS_MAX_ROW" | grep -q "not defined" ; then
		VW_VAR_POS_MAX_ROW=`astparam r vw_pos_max_row`
		if echo "$VW_VAR_POS_MAX_ROW" | grep -q "not defined" ; then
			VW_VAR_POS_MAX_ROW="$MY_ROW_ID"
		fi
	fi
	VW_VAR_POS_MAX_COL=`astparam g vw_pos_max_col`
	if echo "$VW_VAR_POS_MAX_COL" | grep -q "not defined" ; then
		VW_VAR_POS_MAX_COL=`astparam r vw_pos_max_col`
		if echo "$VW_VAR_POS_MAX_COL" | grep -q "not defined" ; then
			VW_VAR_POS_MAX_COL="$MY_ROW_ID"
		fi
	fi
	VW_VAR_DELAY_KICK=`astparam g vw_delay_kick`
	if echo "$VW_VAR_DELAY_KICK" | grep -q "not defined" ; then
		VW_VAR_DELAY_KICK=`astparam r vw_delay_kick`
		if echo "$VW_VAR_DELAY_KICK" | grep -q "not defined" ; then
			VW_VAR_DELAY_KICK='0'
		fi
	fi
	VW_VAR_TAIL_IDX=`astparam g vw_tail_idx`
	if echo "$VW_VAR_TAIL_IDX" | grep -q "not defined" ; then
		VW_VAR_TAIL_IDX=`astparam r vw_tail_idx`
		if echo "$VW_VAR_TAIL_IDX" | grep -q "not defined" ; then
			VW_VAR_TAIL_IDX='unknown'
		fi
	fi
	VW_VAR_STRETCH_TYPE=`astparam g vw_stretch_type`
	if echo "$VW_VAR_STRETCH_TYPE" | grep -q "not defined" ; then
		VW_VAR_STRETCH_TYPE=`astparam r vw_stretch_type`
		if echo "$VW_VAR_STRETCH_TYPE" | grep -q "not defined" ; then
			# 0:auto 1:Stretch out 2:Fit in
			VW_VAR_STRETCH_TYPE='2'
		fi
	fi
	VW_VAR_ROTATE=`astparam g vw_rotate`
	if echo "$VW_VAR_ROTATE" | grep -q "not defined" ; then
		VW_VAR_ROTATE=`astparam r vw_rotate`
		if echo "$VW_VAR_ROTATE" | grep -q "not defined" ; then
			# 5:90 3:180 6:270
			VW_VAR_ROTATE='0'
		fi
	fi

	#VW_VAR_LAYOUT=`expr $VW_VAR_MAX_ROW + 1`x`expr $VW_VAR_MAX_COLUMN + 1`
	VW_VAR_LAYOUT="$(( $VW_VAR_MAX_ROW + 1 ))x$(( $VW_VAR_MAX_COLUMN + 1 ))"

	if [ "$VW_VAR_ROW" = 'unknown' ]; then
		VW_VAR_ROW="$MY_ROW_ID"
	fi

	#Bug#2011090901. We should not mod VW_VAR_COLUMN. The driver can handle it.
	#VW_VAR_COLUMN=`expr $VW_VAR_COLUMN % \( $VW_VAR_MAX_COLUMN + 1 \)`

	if [ "$VW_VAR_POS_IDX" = 'unknown' ]; then
		if [ "$IS_HOST" = 'y' ]; then
			VW_VAR_POS_IDX="${MY_ROW_ID}"
		else
			VW_VAR_POS_IDX="r${VW_VAR_ROW}c${VW_VAR_COLUMN}"
		fi
	fi
	if [ "$IS_HOST" = 'y' ]; then
		VW_VAR_POS_R="$VW_VAR_POS_IDX"
		VW_VAR_POS_C="0"
		THE_ROW_ID="$VW_VAR_POS_IDX" #This flag will be the same as $VW_VAR_POS_R except the host box which is the controller.
	else
		_IFS="$IFS";IFS='rc';set -- $VW_VAR_POS_IDX;IFS="$_IFS"
		VW_VAR_POS_R="$2"
		VW_VAR_POS_C="$3"
		#VW_VAR_POS_R=`expr "$VW_VAR_POS_IDX" : 'r\([[:alnum:]]\{1,\}\)c[[:alnum:]]\{1,\}'`
		#VW_VAR_POS_C=`expr "$VW_VAR_POS_IDX" : 'r[[:alnum:]]\{1,\}c\([[:alnum:]]\{1,\}\)'`
	fi
	if [ "$VW_VAR_TAIL_IDX" = 'unknown' ]; then
		VW_VAR_TAIL_IDX="${VW_VAR_POS_R}"
	fi
	# In case of error
	if [ -z "$VW_VAR_POS_R" ]; then
		VW_VAR_POS_R="0"
	fi
	if [ -z "$VW_VAR_POS_C" ]; then
		VW_VAR_POS_C="0"
	fi
	VW_VAR_THE_MAX_ROW='0' #This is a temp value for host to save max row count

	if [ "$SOC_OP_MODE" -ge '3' ]; then
		VW_VAR_VER=`astparam g vw_ver`
		if echo "$VW_VAR_VER" | grep -q "not defined" ; then
			VW_VAR_VER=`astparam r vw_ver`
			if echo "$VW_VAR_VER" | grep -q "not defined" ; then
				VW_VAR_VER='1'
			fi
		fi
	else
		VW_VAR_VER='1'
	fi

	echo "VW_VAR_POS_IDX=$VW_VAR_POS_IDX"
	echo "VW_VAR_MAX_ROW=$VW_VAR_MAX_ROW"
	echo "VW_VAR_MAX_COLUMN=$VW_VAR_MAX_COLUMN"
	echo "VW_VAR_ROW=$VW_VAR_ROW"
	echo "VW_VAR_COLUMN=$VW_VAR_COLUMN"
	echo "VW_VAR_H_SHIFT=$VW_VAR_H_SHIFT"
	echo "VW_VAR_V_SHIFT=$VW_VAR_V_SHIFT"
	echo "VW_VAR_H_SCALE=$VW_VAR_H_SCALE"
	echo "VW_VAR_V_SCALE=$VW_VAR_V_SCALE"
	echo "VW_VAR_MONINFO_HA=$VW_VAR_MONINFO_HA"
	echo "VW_VAR_MONINFO_HT=$VW_VAR_MONINFO_HT"
	echo "VW_VAR_MONINFO_VA=$VW_VAR_MONINFO_VA"
	echo "VW_VAR_MONINFO_VT=$VW_VAR_MONINFO_VT"
	echo "VW_VAR_POS_MAX_ROW=$VW_VAR_POS_MAX_ROW"
	echo "VW_VAR_POS_MAX_COL=$VW_VAR_POS_MAX_COL"
	echo "VW_VAR_POS_R=$VW_VAR_POS_R"
	echo "VW_VAR_POS_C=$VW_VAR_POS_C"
	echo "VW_VAR_DELAY_KICK=$VW_VAR_DELAY_KICK"

	if [ "$VW_VAR_VER" = '2' ]; then
		init_video_wall_params_v2
		video_wall_params_display_v2
	fi
}

parse_usb_busid_map()
{
	## USB_BUSID_MAP='2-1 1-1,3-1 1-2,3-2 1-4,2-3 1-5,2-2 1-3'
	_IFS="$IFS";IFS=',';set -- $USB_BUSID_MAP;IFS="$_IFS"

	USB_P1_BUSID="$1"
	USB_P2_BUSID="$2"
	USB_P3_BUSID="$3"
	USB_P4_BUSID="$4"
	USB_P5_BUSID="$5"

	echo "USB_P1_BUSID=($USB_P1_BUSID)"
	echo "USB_P2_BUSID=($USB_P2_BUSID)"
	echo "USB_P3_BUSID=($USB_P3_BUSID)"
	echo "USB_P4_BUSID=($USB_P4_BUSID)"
	echo "USB_P5_BUSID=($USB_P5_BUSID)"
}

clear_ch_select_ex()
{
	astparam s ch_select
	astparam s ch_select_v
	astparam s ch_select_a
	astparam s ch_select_u
	astparam s ch_select_s
	astparam s ch_select_r
	astparam s ch_select_p
	astparam s ch_select_c
}

clear_access_on_x()
{
	# This is client only astparam.
	if [ "$IS_HOST" = 'y' ]; then
		return
	fi
	astparam s access_on_v
	astparam s access_on_u
	astparam s access_on_a
	astparam s access_on_r
	astparam s access_on_s
	astparam s access_on_p
	astparam s access_on_c
}

refresh_access_on_x()
{
	# This is client only astparam.
	if [ "$IS_HOST" = 'y' ]; then
		return
	fi
	ACCESS_ON_V=`astparam g access_on_v`
	if echo "$ACCESS_ON_V" | grep -q "not defined" ; then
		ACCESS_ON_V=`astparam r access_on_v`
		if echo "$ACCESS_ON_V" | grep -q "not defined" ; then
			ACCESS_ON_V="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_U=`astparam g access_on_u`
	if echo "$ACCESS_ON_U" | grep -q "not defined" ; then
		ACCESS_ON_U=`astparam r access_on_u`
		if echo "$ACCESS_ON_U" | grep -q "not defined" ; then
			ACCESS_ON_U="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_A=`astparam g access_on_a`
	if echo "$ACCESS_ON_A" | grep -q "not defined" ; then
		ACCESS_ON_A=`astparam r access_on_a`
		if echo "$ACCESS_ON_A" | grep -q "not defined" ; then
			ACCESS_ON_A="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_R=`astparam g access_on_r`
	if echo "$ACCESS_ON_R" | grep -q "not defined" ; then
		ACCESS_ON_R=`astparam r access_on_r`
		if echo "$ACCESS_ON_R" | grep -q "not defined" ; then
			ACCESS_ON_R="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_S=`astparam g access_on_s`
	if echo "$ACCESS_ON_S" | grep -q "not defined" ; then
		ACCESS_ON_S=`astparam r access_on_s`
		if echo "$ACCESS_ON_S" | grep -q "not defined" ; then
			ACCESS_ON_S="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_P=`astparam g access_on_p`
	if echo "$ACCESS_ON_P" | grep -q "not defined" ; then
		ACCESS_ON_P=`astparam r access_on_p`
		if echo "$ACCESS_ON_P" | grep -q "not defined" ; then
			ACCESS_ON_P="$ACCESS_ON"
		fi
	fi
	ACCESS_ON_C=`astparam g access_on_c`
	if echo "$ACCESS_ON_C" | grep -q "not defined" ; then
		ACCESS_ON_C=`astparam r access_on_c`
		if echo "$ACCESS_ON_C" | grep -q "not defined" ; then
			ACCESS_ON_C="$ACCESS_ON"
		fi
	fi
}

_map_mcip_srv_map()
{
	_IFS="$IFS";IFS=':';set -- $1;IFS="$_IFS"

	if [ "$FREE_ROUTING" = 'y' ]; then
		MCIP_SRV_MAP_V=${1:-'2'}
		MCIP_SRV_MAP_A=${2:-'3'}
		MCIP_SRV_MAP_R=${3:-'4'}
		MCIP_SRV_MAP_S=${4:-'5'}
		MCIP_SRV_MAP_P=${5:-'6'}
		MCIP_SRV_MAP_U=${6:-'7'}
	else
		# Always use the same and first channel specified.
		MCIP_SRV_MAP_V=${1:-'2'}
		MCIP_SRV_MAP_A=$MCIP_SRV_MAP_V
		MCIP_SRV_MAP_R=$MCIP_SRV_MAP_V
		MCIP_SRV_MAP_S=$MCIP_SRV_MAP_V
		MCIP_SRV_MAP_P=$MCIP_SRV_MAP_V
		MCIP_SRV_MAP_U=$MCIP_SRV_MAP_V
	fi

	#echo "MCIP_SRV_MAP_V=$MCIP_SRV_MAP_V"
	#echo "MCIP_SRV_MAP_A=$MCIP_SRV_MAP_A"
	#echo "MCIP_SRV_MAP_R=$MCIP_SRV_MAP_R"
	#echo "MCIP_SRV_MAP_S=$MCIP_SRV_MAP_S"
	#echo "MCIP_SRV_MAP_P=$MCIP_SRV_MAP_P"
	#echo "MCIP_SRV_MAP_U=$MCIP_SRV_MAP_U"
}

refresh_ieee802_1x_params()
{
	IEEE802_1X_MODE=$(jq -r .ieee802_1x_setting.mode $IEEE8021X_JSON_PATH | tr A-Z a-z)
	IEEE802_1X_AUTH_METHOD=$(jq -r .ieee802_1x_setting.default_authentication $IEEE8021X_JSON_PATH | tr A-Z a-z)

	IEEE802_1X_MSCHAPV2_USER=$(jq -r .ieee802_1x_setting.eap_mschap_setting.mschap_username $IEEE8021X_JSON_PATH | tr A-Z a-z)
	IEEE802_1X_MSCHAPV2_PASSWORD=$(jq -r .ieee802_1x_setting.eap_mschap_setting.mschap_password $IEEE8021X_JSON_PATH | tr A-Z a-z)

	IEEE802_1X_TLS_USER=$(jq -r .ieee802_1x_setting.eap_tls_setting.tls_username $IEEE8021X_JSON_PATH | tr A-Z a-z)
	IEEE802_1X_TLS_CA_CERT_PATH="$IEEE8021X_PATH/tls_ca_certificate/$(jq -r .ieee802_1x_setting.eap_tls_setting.tls_ca_certificate $IEEE8021X_JSON_PATH | tr A-Z a-z)"
	IEEE802_1X_TLS_CLIENT_CERT_PATH="$IEEE8021X_PATH/tls_client_certificate/$(jq -r .ieee802_1x_setting.eap_tls_setting.tls_client_certificate $IEEE8021X_JSON_PATH | tr A-Z a-z)"
	IEEE802_1X_TLS_PRIVATE_KEY_PATH="$IEEE8021X_PATH/tls_private_key/$(jq -r .ieee802_1x_setting.eap_tls_setting.tls_private_key $IEEE8021X_JSON_PATH | tr A-Z a-z)"
	IEEE802_1X_TLS_PRIVATE_KEY_PASSWORD=$(jq -r .ieee802_1x_setting.eap_tls_setting.tls_private_password $IEEE8021X_JSON_PATH | tr A-Z a-z)
}

init_ldap_params()
{
	LDAP_ENABLE=`astparam g ldap_enable`
	if echo "$LDAP_ENABLE" | grep -q "not defined" ; then
		LDAP_ENABLE=`astparam r ldap_enable`
		if echo "$LDAP_ENABLE" | grep -q "not defined" ; then
			LDAP_ENABLE='n'
		fi
	fi
	LDAP_MODE=`astparam g ldap_mode`
	if echo "$LDAP_MODE" | grep -q "not defined" ; then
		LDAP_MODE=`astparam r ldap_mode`
		if echo "$LDAP_MODE" | grep -q "not defined" ; then
			LDAP_MODE='dn'
		fi
	fi
	LDAP_URI=`astparam g ldap_uri`
	if echo "$LDAP_URI" | grep -q "not defined" ; then
		LDAP_URI=`astparam r ldap_uri`
		if echo "$LDAP_URI" | grep -q "not defined" ; then
			LDAP_URI='ldap://'
		fi
	fi
	LDAP_UID=`astparam g ldap_uid`
	if echo "$LDAP_UID" | grep -q "not defined" ; then
		LDAP_UID=`astparam r ldap_uid`
		if echo "$LDAP_UID" | grep -q "not defined" ; then
			LDAP_UID='0'
		fi
	fi
	LDAP_BASE_DN=`astparam g ldap_base_dn`
	if echo "$LDAP_BASE_DN" | grep -q "not defined" ; then
		LDAP_BASE_DN=`astparam r ldap_base_dn`
		if echo "$LDAP_BASE_DN" | grep -q "not defined" ; then
			LDAP_BASE_DN='none'
		fi
	fi
	LDAP_BIND_DN=`astparam g ldap_bind_dn`
	if echo "$LDAP_BIND_DN" | grep -q "not defined" ; then
		LDAP_BIND_DN=`astparam r ldap_bind_dn`
		if echo "$LDAP_BIND_DN" | grep -q "not defined" ; then
			LDAP_BIND_DN='none'
		fi
	fi
	LDAP_USER_ATTR=`astparam g ldap_user_attr`
	if echo "$LDAP_USER_ATTR" | grep -q "not defined" ; then
		LDAP_USER_ATTR=`astparam r ldap_user_attr`
		if echo "$LDAP_USER_ATTR" | grep -q "not defined" ; then
			LDAP_USER_ATTR='none'
		fi
	fi
	LDAP_PASSWORD=`astparam g ldap_password`
	if echo "$LDAP_PASSWORD" | grep -q "not defined" ; then
		LDAP_PASSWORD=`astparam r ldap_password`
		if echo "$LDAP_PASSWORD" | grep -q "not defined" ; then
			LDAP_PASSWORD='none'
		fi
	fi
}

query_board_type()
{
	MODEL_NUMBER=$(astparam r model_number)
	if echo "$MODEL_NUMBER" | grep -q "not defined"; then
		MODEL_NUMBER='UNKNOWN'
	fi
}

init_temperature_sensor()
{
	echo 3 100000 > /sys/devices/platform/i2c/bus_init
	echo 3 0x90 0x00 > /sys/devices/platform/i2c/io_word
	VALUE=$(cat /sys/devices/platform/i2c/io_word)
	if echo "$VALUE" | grep -q "fail"; then
		echo "Temperature sensor not found,quit"
		return
	fi

	echo "#!/bin/sh

TEMPERATURE=\`cat /sys/devices/platform/i2c/io_word\`
((TEMPERATURE=16#\${TEMPERATURE:0:2}))
if [ \$TEMPERATURE -le 128 ];then
	echo \"\$TEMPERATURE\"
else
	echo \"TEMPERATURE too high\"
fi
	" > /usr/local/bin/get_temperature
	chmod +x /usr/local/bin/get_temperature
}

init_share_param_from_flash()
{
	DBG='0'
	MFG_MODE='Off'
	EDID_USE='primary'

	_p=`/usr/local/bin/io 0 1e6e207c`
	_IFS="$IFS";IFS=' ';set -- $_p;IFS="$_IFS"
	_p=$3
	case "$_p" in
		302)
			# AST1500
			SOC_VER='1'
		;;
		1000003 | 1010303 | 1010203 | 2010303)
			#1000003  AST1510 A0
			#1010303  AST1510 A1
			#1010203  AST1510 A1
			#2010303  AST1510 FPGA
			SOC_VER='2'
		;;
		2010003 | 4000003 | 4000303 | 4010303 | 4??1003)
			#2010003  AST1520 FPGA
			#4000003  AST1520 FPGA
			#4000303  AST1520 A0
			#4010303  AST2500 A1
			#4011003  AST1520 A1
			#4031003  AST1520 A2
			SOC_VER='3'
		;;
		*)
			echo "ERROR!!! Unknown SoC version? $_p"
			exit
		;;
	esac

	SOC_OP_MODE=`astparam g soc_op_mode`
	if echo "$SOC_OP_MODE" | grep -q "not defined" ; then
		SOC_OP_MODE=`astparam r soc_op_mode`
		if echo "$SOC_OP_MODE" | grep -q "not defined" ; then
			SOC_OP_MODE=$SOC_VER
		fi
	fi

	# fool-proof $SOC_OP_MODE setting
	case $SOC_OP_MODE in
		1)
			if [ "$SOC_VER" -lt 2 ]; then
				SOC_OP_MODE=$SOC_VER
			fi
		;;
		2)
			if [ "$SOC_VER" -lt 3 ]; then
				SOC_OP_MODE=$SOC_VER
			fi
		;;
		3)
			if [ "$SOC_VER" -lt 4 ]; then
				SOC_OP_MODE=$SOC_VER
			fi
		;;
		*)
			SOC_OP_MODE=$SOC_VER
		;;
	esac

	if echo "$AST_PLATFORM" | grep "c" ; then
		#Client code
		IS_HOST='n'
	else
		#Host code
		IS_HOST='y'
	fi

	# BruceToDo. 'astconfig mac' may be slow. Check it.
	MY_MAC=`astconfig mac`
	echo "MY_MAC=$MY_MAC"

	if [ "$SOC_OP_MODE" -ge '2' ]; then
		IR_SYS_PATH="/sys/devices/platform/IRoIP2"
	fi

	# List of scenario: usb_only, pc2tv
	SCENARIO=`astparam g scenario`
	if echo "$SCENARIO" | grep -q "not defined" ; then
		SCENARIO=`astparam r scenario`
		if echo "$SCENARIO" | grep -q "not defined" ; then
			SCENARIO='pc2tv'
		fi
	fi

	SEAMLESS_SWITCH=`astparam g seamless_switch`
	if echo "$SEAMLESS_SWITCH" | grep -q "not defined" ; then
		SEAMLESS_SWITCH=`astparam r seamless_switch`
		if echo "$SEAMLESS_SWITCH" | grep -q "not defined" ; then
			SEAMLESS_SWITCH='n'
		fi
	fi

	EN_VIDEO_WALL=`astparam g en_video_wall`
	if echo "$EN_VIDEO_WALL" | grep -q "not defined" ; then
		EN_VIDEO_WALL=`astparam r en_video_wall`
		if echo "$EN_VIDEO_WALL" | grep -q "not defined" ; then
			EN_VIDEO_WALL='y'
		fi
	fi

	# AST1510(SoCv2 or SoCv3@soc_op_mode == 2) doesn't support video wall feature.
	if [ "$SOC_OP_MODE" = '2' ]; then
		EN_VIDEO_WALL='n'
	fi

	# Reset channel on boot first
	RESET_CH_ON_BOOT=`astparam g reset_ch_on_boot`
	if echo "$RESET_CH_ON_BOOT" | grep -q "not defined" ; then
		RESET_CH_ON_BOOT=`astparam r reset_ch_on_boot`
		if echo "$RESET_CH_ON_BOOT" | grep -q "not defined" ; then
			RESET_CH_ON_BOOT='n'
		fi
	fi

	NO_VIDEO=`astparam g no_video`
	if echo "$NO_VIDEO" | grep -q "not defined" ; then
		NO_VIDEO=`astparam r no_video`
		if echo "$NO_VIDEO" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ]; then
				NO_VIDEO='y'
			else
				NO_VIDEO='n'
			fi
		fi
	fi

	V_DUAL_PORT=`astparam g v_dual_port`
	if echo "$V_DUAL_PORT" | grep -q "not defined" ; then
		V_DUAL_PORT=`astparam r v_dual_port`
		if echo "$V_DUAL_PORT" | grep -q "not defined" ; then
			case $SOC_OP_MODE in
				3)
					if [ "$IS_HOST" = 'y' ]; then
						V_DUAL_PORT='0'
					else
						V_DUAL_PORT='1'
					fi
				;;
				*)
					V_DUAL_PORT='0'
				;;
			esac
		fi
	fi

	V_ENG_DRV_OPTION=`astparam g v_eng_drv_option`
	if echo "$V_ENG_DRV_OPTION" | grep -q "not defined" ; then
		V_ENG_DRV_OPTION=`astparam r v_eng_drv_option`
		if echo "$V_ENG_DRV_OPTION" | grep -q "not defined" ; then
			case $SOC_VER in
				*)
					V_ENG_DRV_OPTION='0'
				;;
			esac
		fi
	fi

	V_GEN_LOCK_CFG=`astparam g v_gen_lock_cfg`
	if echo "$V_GEN_LOCK_CFG=" | grep -q "not defined" ; then
		V_GEN_LOCK_CFG=`astparam r v_gen_lock_cfg`
		if echo "$V_GEN_LOCK_CFG" | grep -q "not defined" ; then
			case $SOC_OP_MODE in
				3)
					# gen_lock_cfg[31:24]: location
					#      the location user prefer to lock at (in percetage)
					# gen_lock_cfg[22:8] ppm limit
					#      acceptable PPM variation of monitor
					# gen_lock_cfg[7:4] factor
					#      number of swith between host and client
					# gen_lock_cfg[3]: ppm limit 4k patch:
					#      use 10% ppm limit for 4K resolution
					# gen_lock_cfg[2]: ppm strict mode:
					#      limit PPM even if in JUMP state
					# gen_lock_cfg[0]: enable:
					#      enable/disable gen-lock
					V_GEN_LOCK_CFG='55028aa9'
				;;
				*)
					V_GEN_LOCK_CFG='0'
				;;
			esac
		fi
	fi

	NO_USB=`astparam g no_usb`
	if echo "$NO_USB" | grep -q "not defined" ; then
		NO_USB=`astparam r no_usb`
		if echo "$NO_USB" | grep -q "not defined" ; then
			NO_USB='n'
		fi
	fi

	NO_KMOIP=`astparam g no_kmoip`
	if echo "$NO_KMOIP" | grep -q "not defined" ; then
		NO_KMOIP=`astparam r no_kmoip`
		if echo "$NO_KMOIP" | grep -q "not defined" ; then
			NO_KMOIP='n'
		fi
	fi

	USB_BUSID_MAP=`astparam g usb_busid_map`
	if echo "$USB_BUSID_MAP" | grep -q "not defined" ; then
		USB_BUSID_MAP=`astparam r usb_busid_map`
		if echo "$USB_BUSID_MAP" | grep -q "not defined" ; then
			case $SOC_VER in
				1) # default for AST1500 NEC 5 ports
					USB_BUSID_MAP='2-1 1-1,3-1 1-2,3-2 1-4,2-3 1-5,2-2 1-3'
				;;
				2) # for AST1510
					USB_BUSID_MAP='1-1,3-1 2-1,3-2 2-2,3-3 2-3'
				;;
				3) # for AST1520
					USB_BUSID_MAP='3-3,3-4,3-1 1-1,3-2 2-1'
				;;
				*)
					USB_BUSID_MAP='2-1 1-1,3-1 1-2,3-2 1-4,2-3 1-5,2-2 1-3'
				;;
			esac
		fi
	fi
	parse_usb_busid_map

	KMOIP_PORTS=`astparam g kmoip_ports`
	if echo "$KMOIP_PORTS" | grep -q "not defined" ; then
		KMOIP_PORTS=`astparam r kmoip_ports`
		if echo "$KMOIP_PORTS" | grep -q "not defined" ; then
			# default export all ports. Valid options example: '1 3'
			KMOIP_PORTS='all'
			#KMOIP_PORTS='1 2'
		fi
	fi

	KMOIP_HOTKEYS=`astparam g kmoip_hotkeys`
	if echo "$KMOIP_HOTKEYS" | grep -q "not defined" ; then
		KMOIP_HOTKEYS=`astparam r kmoip_hotkeys`
		if echo "$KMOIP_HOTKEYS" | grep -q "not defined" ; then
			# Default:
			# hotkey 0: scroll lock
			# hotkey 1: pause/break
			# hotkey 2: insert
			#KMOIP_HOTKEYS='00 47 00 48 00 49 00 00 00 00 00 00 00 00 00 00'
			KMOIP_HOTKEYS='00 40 00 41 00 42 00 39 00 00 00 00 00 00 00 00'
		fi
	fi

	KMOIP_TOKEN_INTERVAL=`astparam g kmoip_token_interval`
	if echo "$KMOIP_TOKEN_INTERVAL" | grep -q "not defined" ; then
		KMOIP_TOKEN_INTERVAL=`astparam r kmoip_token_interval`
		if echo "$KMOIP_TOKEN_INTERVAL" | grep -q "not defined" ; then
			# default 100 ms
			# KMOIP_TOKEN_INTERVAL=100
			KMOIP_TOKEN_INTERVAL=10000
		fi
	fi

	# A7 no more needed.
	#USB_FAST_SWITCH=`astparam g usb_fast_switch`
	#if echo "$USB_FAST_SWITCH" | grep -q "not defined" ; then
	#	USB_FAST_SWITCH=`astparam r usb_fast_switch`
	#	if echo "$USB_FAST_SWITCH" | grep -q "not defined" ; then
	#		USB_FAST_SWITCH='n'
	#	fi
	#fi

	NO_I2S=`astparam g no_i2s`
	if echo "$NO_I2S" | grep -q "not defined" ; then
		NO_I2S=`astparam r no_i2s`
		if echo "$NO_I2S" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ]; then
				NO_I2S='y'
			else
				NO_I2S='n'
			fi
		fi
	fi

	ACCESS_ON=`astparam g astaccess`
	if echo "$ACCESS_ON" | grep -q "not defined" ; then
		ACCESS_ON=`astparam r astaccess`
		if echo "$ACCESS_ON" | grep -q "not defined" ; then
			ACCESS_ON='y'
		fi
	fi
	# Are we going to use Multicast mode?
	MULTICAST_ON=`astparam g multicast_on`
	if echo "$MULTICAST_ON" | grep -q "not defined" ; then
		MULTICAST_ON=`astparam r multicast_on`
		if echo "$MULTICAST_ON" | grep -q "not defined" ; then
			MULTICAST_ON='y'
		fi
	fi
	# Read after MULTICAST_ON
	EDID_USE=`astparam g edid_use`
	if echo "$EDID_USE" | grep -q "not defined" ; then
		EDID_USE=`astparam r edid_use`
		if echo "$EDID_USE" | grep -q "not defined" ; then
			if [ "$MULTICAST_ON" = 'y' ]; then
				EDID_USE='secondary'
			else
				EDID_USE='primary'
			fi
		fi
	fi
	# Read after MULTICAST_ON
	KMOIP_ROAMING_LAYOUT=`astparam g kmoip_roaming_layout`
	if echo "$KMOIP_ROAMING_LAYOUT" | grep -q "not defined" ; then
		KMOIP_ROAMING_LAYOUT=`astparam r kmoip_roaming_layout`
		if echo "$KMOIP_ROAMING_LAYOUT" | grep -q "not defined" ; then
			KMOIP_ROAMING_LAYOUT=""
		fi
	fi
	# Unicast mode doesn't support KMoIP roaming. Overwrite it.
	if [ "$MULTICAST_ON" = 'n' ]; then
		# Empty string means disable.
		KMOIP_ROAMING_LAYOUT=""
	fi

	# Are we going to use UDP for unicast mode?
	UDP_ON=`astparam g udp_on`
	if echo "$UDP_ON" | grep -q "not defined" ; then
		UDP_ON=`astparam r udp_on`
		if echo "$UDP_ON" | grep -q "not defined" ; then
			UDP_ON='y'
		fi
	fi
	# selections: fix, static, dhcp, autoip
	# fix: use 169.254.xxx.xxx without mDNS feature
	# static: use ip setting from flash (asthostip).
	# dhcp: use dhcp client
	# autoip: use 169.254.xxx.xxx with mDNS feature
	IP_MODE=`astparam g ip_mode`
	if echo "$IP_MODE" | grep -q "not defined" ; then
		IP_MODE=`astparam r ip_mode`
		if echo "$IP_MODE" | grep -q "not defined" ; then
			IP_MODE='dhcp'
		fi
	fi
	# Are we going to use Serial over IP ?
	NO_SOIP=`astparam g no_soip`
	if echo "$NO_SOIP" | grep -q "not defined" ; then
		NO_SOIP=`astparam r no_soip`
		if echo "$NO_SOIP" | grep -q "not defined" ; then
			NO_SOIP='n'
		fi
	fi

	# SOIP_TYPE > SOIP_TYPE2
	SOIP_TYPE2='y'
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

	if [ "$SOIP_TYPE" -ge 2 ]; then
		SOIP_TYPE2_TOKEN_TIMEOUT='1'
		SOIP_TOKEN_TIMEOUT=`astparam g soip_token_timeout`
		if echo "$SOIP_TOKEN_TIMEOUT" | grep -q "not defined" ; then
			SOIP_TOKEN_TIMEOUT=`astparam r soip_token_timeout`
			if echo "$SOIP_TOKEN_TIMEOUT" | grep -q "not defined" ; then
				# Check legacy soip_type2_token_timeout
				SOIP_TYPE2_TOKEN_TIMEOUT=`astparam g soip_type2_token_timeout`
				if echo "$SOIP_TYPE2_TOKEN_TIMEOUT" | grep -q "not defined" ; then
					SOIP_TYPE2_TOKEN_TIMEOUT=`astparam r soip_type2_token_timeout`
				fi
				case "$SOIP_TYPE2_TOKEN_TIMEOUT" in
					*'not defined')
						SOIP_TOKEN_TIMEOUT='1'
						SOIP_TYPE2_TOKEN_TIMEOUT=$SOIP_TOKEN_TIMEOUT
					;;
					*)
						SOIP_TOKEN_TIMEOUT=$SOIP_TYPE2_TOKEN_TIMEOUT
					;;
				esac
			fi
		fi
	fi

	PROFILE=`astparam g profile`
	if echo "$PROFILE" | grep -q "not defined" ; then
		PROFILE=`astparam r profile`
		if echo "$PROFILE" | grep -q "not defined" ; then
			PROFILE='auto'
		fi
	fi

	SHARE_USB_AUTO_MODE=`astparam g share_usb_auto_mode`
	if echo "$SHARE_USB_AUTO_MODE" | grep -q "not defined" ; then
		SHARE_USB_AUTO_MODE=`astparam r share_usb_auto_mode`
		if echo "$SHARE_USB_AUTO_MODE" | grep -q "not defined" ; then
			SHARE_USB_AUTO_MODE='n'
		fi
	fi

	# MULTICAST_ON MUST be available here
	if [ "$SHARE_USB_AUTO_MODE" = 'y' ]; then
		if [ "$MULTICAST_ON" = 'y' ]; then
			SHARE_USB='y'
		else
			SHARE_USB='n'
		fi
	else
		SHARE_USB=`astparam g share_usb`
		if echo "$SHARE_USB" | grep -q "not defined" ; then
			SHARE_USB=`astparam r share_usb`
			if echo "$SHARE_USB" | grep -q "not defined" ; then
				SHARE_USB='n'
			fi
		fi
	fi

	SHARE_USB_ON_FIRST_PEER=`astparam g share_usb_on_first_peer`
	if echo "$SHARE_USB_ON_FIRST_PEER" | grep -q "not defined" ; then
		SHARE_USB_ON_FIRST_PEER=`astparam r share_usb_on_first_peer`
		if echo "$SHARE_USB_ON_FIRST_PEER" | grep -q "not defined" ; then
			SHARE_USB_ON_FIRST_PEER='y'
		fi
	fi

	NO_IR=`astparam g no_ir`
	if echo "$NO_IR" | grep -q "not defined" ; then
		NO_IR=`astparam r no_ir`
		if echo "$NO_IR" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ]; then
				NO_IR='y'
			else
				NO_IR='n'
			fi
		fi
	fi

	IR_GUEST_ON=`astparam g ir_guest_on`
	if echo "$IR_GUEST_ON" | grep -q "not defined" ; then
		IR_GUEST_ON=`astparam r ir_guest_on`
		if echo "$IR_GUEST_ON" | grep -q "not defined" ; then
			IR_GUEST_ON='y'
		fi
	fi

	IR_SW_DECODE_ON=`astparam g ir_sw_decode_on`
	if echo "$IR_SW_DECODE_ON" | grep -q "not defined" ; then
		IR_SW_DECODE_ON=`astparam r ir_sw_decode_on`
		if echo "$IR_SW_DECODE_ON" | grep -q "not defined" ; then
			IR_SW_DECODE_ON='n'
		fi
	fi

	IR_SW_DECODE_NEC_CFG=`astparam g ir_sw_decode_nec_cfg`
	if echo "$IR_SW_DECODE_NEC_CFG" | grep -q "not defined" ; then
		IR_SW_DECODE_NEC_CFG=`astparam r ir_sw_decode_nec_cfg`
		if echo "$IR_SW_DECODE_NEC_CFG" | grep -q "not defined" ; then
			# [dev addr]_[0 code]_[1 code]_[2 code]_..._[8 code]_[9 code]
			IR_SW_DECODE_NEC_CFG='65535_0_1_2_3_4_5_6_7_8_9'
		fi
	fi

	# Used for remote PC power on/off
	NO_PWRBTN=`astparam g no_pwrbtn`
	if echo "$NO_PWRBTN" | grep -q "not defined" ; then
		NO_PWRBTN=`astparam r no_pwrbtn`
		if echo "$NO_PWRBTN" | grep -q "not defined" ; then
			NO_PWRBTN='y'
		fi
	fi

	NO_CEC=`astparam g no_cec`
	if echo "$NO_CEC" | grep -q "not defined" ; then
		NO_CEC=`astparam r no_cec`
		if echo "$NO_CEC" | grep -q "not defined" ; then
			NO_CEC='n'
		fi
	fi

	CEC_DRV_OPTION=`astparam g cec_drv_option`
	if echo "$CEC_DRV_OPTION" | grep -q "not defined" ; then
		CEC_DRV_OPTION=`astparam r cec_drv_option`
		if echo "$CEC_DRV_OPTION" | grep -q "not defined" ; then
			CEC_DRV_OPTION='0'
		fi
	fi

	V_QUALITY_MODE=`astparam g ast_video_quality_mode`
	if echo "$V_QUALITY_MODE" | grep -q "not defined" ; then
		V_QUALITY_MODE=`astparam r ast_video_quality_mode`
		if echo "$V_QUALITY_MODE" | grep -q "not defined" ; then
			# default auto (-1)
			V_QUALITY_MODE='-1'
		fi
	fi

	V_DBG_MODE=`astparam g v_dbg_mode`
	if echo "$V_DBG_MODE" | grep -q "not defined" ; then
		V_DBG_MODE=`astparam r v_dbg_mode`
		if echo "$V_DBG_MODE" | grep -q "not defined" ; then
			V_DBG_MODE='0'
		fi
	fi

	IPADDR=`astparam g ipaddr`
	if echo "$IPADDR" | grep -q "not defined" ; then
		IPADDR=`astparam r ipaddr`
		if echo "$IPADDR" | grep -q "not defined" ; then
			if [ "$IS_HOST" = 'n' ]; then
				#Client code
				IPADDR="169.254.0.111"
			else
				#Host code
				IPADDR="169.254.0.222"
			fi
		fi
	fi

	NETMASK=`astparam g netmask`
	if echo "$NETMASK" | grep -q "not defined" ; then
		NETMASK=`astparam r netmask`
		if echo "$NETMASK" | grep -q "not defined" ; then
			NETMASK='255.255.0.0'
		fi
	fi

	GATEWAYIP=`astparam g gatewayip`
	if echo "$GATEWAYIP" | grep -q "not defined" ; then
		GATEWAYIP=`astparam r gatewayip`
		if echo "$GATEWAYIP" | grep -q "not defined" ; then
			GATEWAYIP='169.254.0.254'
		fi
	fi

	MULTICAST_LEAVE_FORCE=`astparam g multicast_leave_force`
	if echo "$MULTICAST_LEAVE_FORCE" | grep -q "not defined" ; then
		MULTICAST_LEAVE_FORCE=`astparam r multicast_leave_force`
		if echo "$MULTICAST_LEAVE_FORCE" | grep -q "not defined" ; then
			if [ "$MULTICAST_ON" = 'y' ]; then
				MULTICAST_LEAVE_FORCE='8000'
			else
				MULTICAST_LEAVE_FORCE='0'
			fi
		fi
	fi

	V_BCD_THRESHOLD=`astparam g v_bcd_threshold`
	if echo "$V_BCD_THRESHOLD" | grep -q "not defined" ; then
		V_BCD_THRESHOLD=`astparam r v_bcd_threshold`
		if echo "$V_BCD_THRESHOLD" | grep -q "not defined" ; then
			if [ "$SOC_VER" != '1' ]; then
				#V_BCD_THRESHOLD='1' Resolve RctBug#2012071200
				V_BCD_THRESHOLD='0'
			else
				V_BCD_THRESHOLD='0'
			fi
		fi
	fi

	UI_FEATURE=`astparam g ui_feature`
	if echo "$UI_FEATURE" | grep -q "not defined" ; then
		UI_FEATURE=`astparam r ui_feature`
		if echo "$UI_FEATURE" | grep -q "not defined" ; then
			UI_FEATURE='n'
		fi
	fi

	UI_OPTION=`astparam g ui_option`
	if echo "$UI_OPTION" | grep -q "not defined" ; then
		UI_OPTION=`astparam r ui_option`
		if echo "$UI_OPTION" | grep -q "not defined" ; then
			#UIO_CONVERT_TO_HEX   (1 << 0)
			UI_OPTION='0'
		fi
	fi

	_p=`astparam g debug`
	if ! { echo "$_p" | grep -q "not defined"; }; then
		DBG=$_p
	fi

	# Used to construct $MULTICAST_IP. MUST read before using $MULTICAST_IP.
	MULTICAST_IP_PREFIX=`astparam g multicast_ip_prefix`
	if echo "$MULTICAST_IP_PREFIX" | grep -q "not defined" ; then
		MULTICAST_IP_PREFIX=`astparam r multicast_ip_prefix`
		if echo "$MULTICAST_IP_PREFIX" | grep -q "not defined" ; then
			# MULTICAST_IP="${MULTICAST_IP_PREFIX}${CH0}.${CH1}${CH2}${CH3}"
			# 225.1.0.0 is used by name_service
			#MULTICAST_IP_PREFIX='225.0.'
			#MULTICAST_IP_PREFIX='225.0.10'
			# A7 changed to 225
			MULTICAST_IP_PREFIX='225'
		else
			MULTICAST_IP_PREFIX=`foolproof_multicast_ip_prefix $MULTICAST_IP_PREFIX`
		fi
	else
		MULTICAST_IP_PREFIX=`foolproof_multicast_ip_prefix $MULTICAST_IP_PREFIX`
	fi

	HOSTNAMEBYDIPSWITCH=`astparam g hostnamebydipswitch`
	case "$HOSTNAMEBYDIPSWITCH" in
		*'not defined')
			HOSTNAMEBYDIPSWITCH=`astparam r hostnamebydipswitch`
			case "$HOSTNAMEBYDIPSWITCH" in
				*'not defined')
					if [ "$IS_HOST" = 'n' ]; then
						HOSTNAMEBYDIPSWITCH='n'
						# USB only solution doesn't have button (for mfg mode) or
						# even dip switch, which is difficult to connect
						# to it's web page to update FW.
						# For USB only solution to easily connect to the web page,
						# we use 4-bits dip switch as hostname_id.
						if [ "$SCENARIO" = 'usb_only' ] && [ "$MULTICAST_ON" = 'n' ]; then
							HOSTNAMEBYDIPSWITCH='n'
						fi
					else
						HOSTNAMEBYDIPSWITCH='n'
					fi
				;;
				*)
				;;
			esac
		;;
		*)
		;;
	esac

	CS_GPIO_INV=`astparam g cs_gpio_inv`
	if echo "$CS_GPIO_INV" | grep -q "not defined" ; then
		CS_GPIO_INV=`astparam r cs_gpio_inv`
		if echo "$CS_GPIO_INV" | grep -q "not defined" ; then
			# CH0,CH1,CH2,CH3
			# 0 means normal read, 1 means invert
			CS_GPIO_INV="0000"
		fi
	fi
	_CHSLGP0INV=${CS_GPIO_INV:0:1}
	_CHSLGP1INV=${CS_GPIO_INV:1:1}
	_CHSLGP2INV=${CS_GPIO_INV:2:1}
	_CHSLGP3INV=${CS_GPIO_INV:3:1}

	CS_GPIO_ORDER=`astparam g cs_gpio_order`
	if echo "$CS_GPIO_ORDER" | grep -q "not defined" ; then
		CS_GPIO_ORDER=`astparam r cs_gpio_order`
		if echo "$CS_GPIO_ORDER" | grep -q "not defined" ; then
			CS_GPIO_ORDER="0123"
		fi
	fi
	_CHSLGP0=ch${CS_GPIO_ORDER:0:1}
	_CHSLGP1=ch${CS_GPIO_ORDER:1:1}
	_CHSLGP2=ch${CS_GPIO_ORDER:2:1}
	_CHSLGP3=ch${CS_GPIO_ORDER:3:1}

	if [ "$RESET_CH_ON_BOOT" = 'y' ]; then
		clear_ch_select_ex
		clear_access_on_x
		astparam s multicast_ip
		if [ "$IS_HOST" = 'y' ] && [ "$HOSTNAMEBYDIPSWITCH" = 'y' ]; then
			astparam s hostname_id
		fi
	fi

	FREE_ROUTING=`astparam g free_routing`
	if echo "$FREE_ROUTING" | grep -q "not defined" ; then
		FREE_ROUTING=`astparam r free_routing`
		if echo "$FREE_ROUTING" | grep -q "not defined" ; then
			FREE_ROUTING='y'
		fi
	fi
	# Requires $FREE_ROUTING
	MCIP_SRV_MAP=`astparam g mcip_srv_map`
	if echo "$MCIP_SRV_MAP" | grep -q "not defined" ; then
		MCIP_SRV_MAP=`astparam r mcip_srv_map`
		if echo "$MCIP_SRV_MAP" | grep -q "not defined" ; then
			if [ "$FREE_ROUTING" = 'y' ]; then
				MCIP_SRV_MAP="2:3:4:5:6:7"
			else
				MCIP_SRV_MAP="2:2:2:2:2:2"
			fi
		fi
	fi
	_map_mcip_srv_map $MCIP_SRV_MAP

	# NOTE: _CHSLGP0..3, _CHSLGPINV0..3 MUST be ready before calling refresh_4bits_ch()
	refresh_4bits_ch
	refresh_ch_params
	refresh_hostname_params
	# NOTE: need $ACCESS_ON and $IS_HOST ready before calling refresh_access_on_x
	refresh_access_on_x

	##RELOAD_AVAHID='n'

	## A7 moved to refresh_ch_params()
	##CH_SELECT_I2S=`astparam g ch_select_i2s`
	##if echo "$CH_SELECT_I2S" | grep -q "not defined" ; then
	##	CH_SELECT_I2S=`astparam r ch_select_i2s`
	##	if echo "$CH_SELECT_I2S" | grep -q "not defined" ; then
	##		CH_SELECT_I2S=''
	##	fi
	##fi

	## A7 to be removed
	##MULTICAST_IP_I2S=`astparam g multicast_ip_i2s`
	##if echo "$MULTICAST_IP_I2S" | grep -q "not defined" ; then
	##	MULTICAST_IP_I2S=`astparam r multicast_ip_i2s`
	##	if echo "$MULTICAST_IP_I2S" | grep -q "not defined" ; then
	##		MULTICAST_IP_I2S=''
	##	fi
	##fi

	# We need both Tx and Rx hostname middle because:
	# - Web UI node_list -j
	# - GUI node_list
	# - SoIP type 1 need Rx's hostname_prefix
	HOSTNAME_TX_MIDDLE=`astparam g hostname_tx_middle`
	if echo "$HOSTNAME_TX_MIDDLE" | grep -q "not defined" ; then
		HOSTNAME_TX_MIDDLE=`astparam r hostname_tx_middle`
		if echo "$HOSTNAME_TX_MIDDLE" | grep -q "not defined" ; then
			HOSTNAME_TX_MIDDLE='UNKNOWN'
			#HOSTNAME_TX_MIDDLE='-Tx-'
		fi
	fi

	HOSTNAME_RX_MIDDLE=`astparam g hostname_rx_middle`
	if echo "$HOSTNAME_RX_MIDDLE" | grep -q "not defined" ; then
		HOSTNAME_RX_MIDDLE=`astparam r hostname_rx_middle`
		if echo "$HOSTNAME_RX_MIDDLE" | grep -q "not defined" ; then
			HOSTNAME_RX_MIDDLE='UNKNOWN'
			#HOSTNAME_RX_MIDDLE='-Rx-'
		fi
	fi

	HOSTNAME_PREFIX=`astparam g hostname_prefix`
	if echo "$HOSTNAME_PREFIX" | grep -q "not defined" ; then
		HOSTNAME_PREFIX=`astparam r hostname_prefix`
		if echo "$HOSTNAME_PREFIX" | grep -q "not defined" ; then
			case "$SOC_OP_MODE" in
				1)
					HOSTNAME_PREFIX=''
				;;
				*)
					HOSTNAME_PREFIX=""
				;;
			esac
		fi
	fi

	V_1080I_TO_720P=`astparam g v_1080i_to_720p`
	if echo "$V_1080I_TO_720P" | grep -q "not defined" ; then
		V_1080I_TO_720P=`astparam r v_1080i_to_720p`
		if echo "$V_1080I_TO_720P" | grep -q "not defined" ; then
			V_1080I_TO_720P='n'
		fi
	fi

	IGNORE_E_BUTTON_LINK=`astparam g ignore_e_button_link`
	if echo "$IGNORE_E_BUTTON_LINK" | grep -q "not defined" ; then
		IGNORE_E_BUTTON_LINK=`astparam r ignore_e_button_link`
		if echo "$IGNORE_E_BUTTON_LINK" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ]; then
				IGNORE_E_BUTTON_LINK='y'
			else
				IGNORE_E_BUTTON_LINK='n'
			fi
		fi
	fi

	IGNORE_E_BUTTON_LINK_1=`astparam g ignore_e_button_link_1`
	if echo "$IGNORE_E_BUTTON_LINK_1" | grep -q "not defined" ; then
		IGNORE_E_BUTTON_LINK_1=`astparam r ignore_e_button_link_1`
		if echo "$IGNORE_E_BUTTON_LINK_1" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ] && [ "$IS_HOST" = 'y' ]; then
				IGNORE_E_BUTTON_LINK_1='y'
			else
				IGNORE_E_BUTTON_LINK_1='n'
			fi
		fi
	fi

	PWR_LED_TYPE=`astparam g pwr_led_type`
	if echo "$PWR_LED_TYPE" | grep -q "not defined" ; then
		PWR_LED_TYPE=`astparam r pwr_led_type`
		if echo "$PWR_LED_TYPE" | grep -q "not defined" ; then
			if [ "$SCENARIO" = 'usb_only' ] && [ "$SHARE_USB" = 'y' ]; then
				PWR_LED_TYPE='share_usb'
			else
				PWR_LED_TYPE='default'
			fi

		fi
	fi

	if [ "$EN_VIDEO_WALL" = 'y' ]; then
		# Considering non-RS232 chain mode, we can support SoIP
		#if [ "$IS_HOST" = 'y' ]; then
		#	NO_SOIP='y'
		#else
		#	SOIP_GUEST_ON='y'
		#fi
		init_video_wall_params
	fi

	EN_LOG=`astparam g en_log`
	if echo "$EN_LOG" | grep -q "not defined" ; then
		EN_LOG=`astparam r en_log`
		if echo "$EN_LOG" | grep -q "not defined" ; then
			EN_LOG='n'
		fi
	fi

	JUMBO_MTU=`astparam g jumbo_mtu`
	if echo "$JUMBO_MTU" | grep -q "not defined" ; then
		JUMBO_MTU=`astparam r jumbo_mtu`
		if echo "$JUMBO_MTU" | grep -q "not defined" ; then
			JUMBO_MTU='8000'
		fi
	fi

	STOP_ON_BOOT_TEST_FAIL=`astparam g stop_on_boot_test_fail`
	if echo "$STOP_ON_BOOT_TEST_FAIL" | grep -q "not defined" ; then
		STOP_ON_BOOT_TEST_FAIL=`astparam r stop_on_boot_test_fail`
		if echo "$STOP_ON_BOOT_TEST_FAIL" | grep -q "not defined" ; then
			STOP_ON_BOOT_TEST_FAIL='n'
		fi
	fi

	TELNETD_PARAM=`astparam g telnetd_param`
	if echo "$TELNETD_PARAM" | grep -q "not defined" ; then
		TELNETD_PARAM=`astparam r telnetd_param`
		if echo "$TELNETD_PARAM" | grep -q "not defined" ; then
			TELNETD_PARAM='-p 24'
		fi
	fi

	LM_LINK_OFF_TIMEOUT=`astparam g lm_link_off_timeout`
	if echo "$LM_LINK_OFF_TIMEOUT" | grep -q "not defined" ; then
		LM_LINK_OFF_TIMEOUT=`astparam r lm_link_off_timeout`
		if echo "$LM_LINK_OFF_TIMEOUT" | grep -q "not defined" ; then
			LM_LINK_OFF_TIMEOUT='8'
		fi
	fi

	HDCP_ALWAYS_ON=`astparam g hdcp_always_on`
	if echo "$HDCP_ALWAYS_ON" | grep -q "not defined" ; then
		HDCP_ALWAYS_ON=`astparam r hdcp_always_on`
		if echo "$HDCP_ALWAYS_ON" | grep -q "not defined" ; then
			HDCP_ALWAYS_ON='n'
		fi
	fi

	HDCP_ALWAYS_ON_22=`astparam g hdcp_always_on_22`
	if echo "$HDCP_ALWAYS_ON_22" | grep -q "not defined" ; then
		HDCP_ALWAYS_ON_22=`astparam r hdcp_always_on_22`
		if echo "$HDCP_ALWAYS_ON_22" | grep -q "not defined" ; then
			HDCP_ALWAYS_ON_22='n'
		fi
	fi

	HDCP_CTS_OPTION=`astparam g hdcp_cts_option`
	if echo "$HDCP_CTS_OPTION" | grep -q "not defined" ; then
		HDCP_CTS_OPTION=`astparam r hdcp_cts_option`
		if echo "$HDCP_CTS_OPTION" | grep -q "not defined" ; then
			HDCP_CTS_OPTION='0'
		fi
	fi

	HDCP_MODE=`astparam g hdcp_mode`
	if echo "$HDCP_MODE" | grep -q "not defined" ; then
		HDCP_MODE=`astparam r hdcp_mode`
		if echo "$HDCP_MODE" | grep -q "not defined" ; then
			# 0: legency HDCP
			# 1: standard HDCP 2.0
			# 2: fast HDCP 2.0
			HDCP_MODE='0'
		fi
	fi

	HDCP2_VERSION=`astparam g hdcp2_version`
	if echo "$HDCP2_VERSION" | grep -q "not defined" ; then
		HDCP2_VERSION=`astparam r hdcp2_version`
		if echo "$HDCP2_VERSION" | grep -q "not defined" ; then
			HDCP2_VERSION='0'
		fi
	fi

	EN_AES=`astparam g en_aes`
	if echo "$EN_AES" | grep -q "not defined" ; then
		EN_AES=`astparam r en_aes`
		if echo "$EN_AES" | grep -q "not defined" ; then
			EN_AES='n'
		fi
	fi

	HDCP2_GLOBAL_CONSTANT=`astparam g hdcp2_global_constant`
	if echo "$HDCP2_GLOBAL_CONSTANT" | grep -q "not defined" ; then
		HDCP2_GLOBAL_CONSTANT=`astparam r hdcp2_global_constant`
		if echo "$HDCP2_GLOBAL_CONSTANT" | grep -q "not defined" ; then
			HDCP2_GLOBAL_CONSTANT='unavailable'
		fi
	fi

	V_RX_DRV_OPTION=`astparam g v_rx_drv_option`
	if echo "$V_RX_DRV_OPTION" | grep -q "not defined" ; then
		V_RX_DRV_OPTION=`astparam r v_rx_drv_option`
		if echo "$V_RX_DRV_OPTION" | grep -q "not defined" ; then
			V_RX_DRV_OPTION='0'
		fi
	fi

	V_REJECT_HDCP=`astparam g v_reject_hdcp`
	if echo "$V_REJECT_HDCP" | grep -q "not defined" ; then
		V_REJECT_HDCP=`astparam r v_reject_hdcp`
		if echo "$V_REJECT_HDCP" | grep -q "not defined" ; then
			V_REJECT_HDCP='n'
		fi
	fi
	if [ "$V_REJECT_HDCP" = 'y' ]; then
		V_RX_DRV_OPTION=$(( $V_RX_DRV_OPTION | 1 ))
	fi

	V_IGNORE_CTS7_33=`astparam g v_ignore_cts7_33`
	if echo "$V_IGNORE_CTS7_33" | grep -q "not defined" ; then
		V_IGNORE_CTS7_33=`astparam r v_ignore_cts7_33`
		if echo "$V_IGNORE_CTS7_33" | grep -q "not defined" ; then
			V_IGNORE_CTS7_33='n'
		fi
	fi

	V_CHUNK_SIZE=`astparam g v_chunk_size`
	if echo "$V_CHUNK_SIZE" | grep -q "not defined" ; then
		V_CHUNK_SIZE=`astparam r v_chunk_size`
		if echo "$V_CHUNK_SIZE" | grep -q "not defined" ; then
			case "$SOC_VER" in
				1)
					V_CHUNK_SIZE='64512' # Use '7376' to smooth network burst
				;;
				2)
					V_CHUNK_SIZE='64512' #63x1024
				;;
				*)
					V_CHUNK_SIZE='64512' #63x1024
				;;
			esac
		fi
	fi

	A_IO_SELECT=`astparam g a_io_select`
	if echo "$A_IO_SELECT" | grep -q "not defined" ; then
		A_IO_SELECT=`astparam r a_io_select`
		if echo "$A_IO_SELECT" | grep -q "not defined" ; then
			# 'auto', 'auto_1', 'auto_2', 'hdmi' or 'analog'
			#auto: HDMI per GPIO, VGA fixed codec.
			#auto_1: always per GPIO.
			#auto_2: always per video port. HDMI/DVI use HDMI audio, VGA use codec audio.
			A_IO_SELECT='auto'
		fi
	fi

	A_OUTPUT_DELAY=`astparam g a_output_delay`
	if echo "$A_OUTPUT_DELAY" | grep -q "not defined" ; then
		A_OUTPUT_DELAY=`astparam r a_output_delay`
		if echo "$A_OUTPUT_DELAY" | grep -q "not defined" ; then
			# in ms. From 0 to 100 ms. For SoC >= V3
			A_OUTPUT_DELAY='0'
		fi
	fi

	A_ANALOG_IN_VOL=`astparam g a_analog_in_vol`
	if echo "$A_ANALOG_IN_VOL" | grep -q "not defined" ; then
		A_ANALOG_IN_VOL=`astparam r a_analog_in_vol`
		if echo "$A_ANALOG_IN_VOL" | grep -q "not defined" ; then
			A_ANALOG_IN_VOL='-1'
		fi
	fi

	A_ANALOG_OUT_VOL=`astparam g a_analog_out_vol`
	if echo "$A_ANALOG_OUT_VOL" | grep -q "not defined" ; then
		A_ANALOG_OUT_VOL=`astparam r a_analog_out_vol`
		if echo "$A_ANALOG_OUT_VOL" | grep -q "not defined" ; then
			A_ANALOG_OUT_VOL='-1'
		fi
	fi

	V_HDMI_FORCE_RGB_OUTPUT=`astparam g v_hdmi_force_rgb_output`
	if echo "$V_HDMI_FORCE_RGB_OUTPUT=" | grep -q "not defined" ; then
		V_HDMI_FORCE_RGB_OUTPUT=`astparam r v_hdmi_force_rgb_output`
		if echo "$V_HDMI_FORCE_RGB_OUTPUT" | grep -q "not defined" ; then
			# 5: all output RGB. (OF_DE_RGB | OF_SE_RGB)
			# 1: non-4K RGB, 4K YUV. (OF_DE_YUV | OF_SE_RGB)
			V_HDMI_FORCE_RGB_OUTPUT='5'
		fi
	fi

	V_OUTPUT_TIMING_CONVERT=`astparam g v_output_timing_convert`
	if echo "$V_OUTPUT_TIMING_CONVERT=" | grep -q "not defined" ; then
		V_OUTPUT_TIMING_CONVERT=`astparam r v_output_timing_convert`
		if echo "$V_OUTPUT_TIMING_CONVERT" | grep -q "not defined" ; then
			V_OUTPUT_TIMING_CONVERT='0'
		fi
	fi

	V_HDMI_HDR_MODE=`astparam g v_hdmi_hdr_mode`
	if echo "$V_HDMI_HDR_MODE=" | grep -q "not defined" ; then
		V_HDMI_HDR_MODE=`astparam r v_hdmi_hdr_mode`
		if echo "$V_HDMI_HDR_MODE" | grep -q "not defined" ; then
			# 0: HDR passthrough 1: Force HDR off
			V_HDMI_HDR_MODE='0'
		fi
	fi

	BOARD_REVISION=`astparam g board_revision`
	if echo "$BOARD_REVISION=" | grep -q "not defined" ; then
		BOARD_REVISION=`astparam r board_revision`
		if echo "$BOARD_REVISION" | grep -q "not defined" ; then
			BOARD_REVISION=`printf "%03x" $(($SOC_VER<<8))`
		fi
	fi

	MODEL_NUMBER=$(astparam r model_number)
	if echo "$MODEL_NUMBER" | grep -q "not defined"; then
		MODEL_NUMBER='UNKNOWN'
	fi

	BOARD_VERSION=$(astparam r board_version)
	if echo "$BOARD_VERSION" | grep -q "not defined"; then
		BOARD_VERSION='UNKNOWN'
	fi

	SERIAL_NUMBER=$(astparam r serial_number)
	if echo "$SERIAL_NUMBER" | grep -q "not defined"; then
		SERIAL_NUMBER='UNKNOWN'
	fi

	ETH_ADDR=`astparam r ethaddr`
	if echo "$ETH_ADDR" | grep -q "not defined"; then
		ETH_ADDR='RANDOM'
	fi
	init_ldap_params

	######### Always buttom ##############################################
	WEB_UI_CFG=`astparam g web_ui_cfg`
	if echo "$WEB_UI_CFG" | grep -q "not defined" ; then
		WEB_UI_CFG=`astparam r web_ui_cfg`
		if echo "$WEB_UI_CFG" | grep -q "not defined" ; then
			# e: essential. The dummy keyword to keep only essential web UI.
			# n: network
			# v: video
			# w: video wall
			# a: audio (i2s)
			# u: usb
			# s: serial over ip
			# r: IR
			# g: god mode. TBD
			WEB_UI_CFG='ne'
			if [ "$NO_VIDEO" = 'n' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}v"
			fi
			if [ "$EN_VIDEO_WALL" = 'y' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}w"
			fi
			if [ "$NO_I2S" = 'n' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}a"
			fi
			if [ "$NO_USB" = 'n' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}u"
			fi
			if [ "$NO_SOIP" = 'n' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}s"
			fi
			if [ "$NO_IR" = 'n' ]; then
				WEB_UI_CFG="${WEB_UI_CFG}r"
			fi
			# Initial WEB_UI_CFG only once and use the setting forever.
			# Otherwise, when user disable XX function, WEB UI won't display the enable option anymore,
			# since WEB_UI_CFG is changed after xx function disabled.
			astparam s web_ui_cfg $WEB_UI_CFG
			astparam save
		fi
	fi

	if [ "$IS_HOST" = 'y' ]; then
		. ./button_handler_h.sh
	else
		. ./button_handler_c.sh
	fi
	_t=`astparam g btn1_short`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_short`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_SHORT="$_t"
		fi
	else
		BTN1_SHORT="$_t"
	fi
	_t=`astparam g btn1_long`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_long`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_LONG="$_t"
		fi
	else
		BTN1_LONG="$_t"
	fi
	_t=`astparam g btn1_short_on_boot`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_short_on_boot`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_SHORT_ON_BOOT="$_t"
		fi
	else
		BTN1_SHORT_ON_BOOT="$_t"
	fi
	_t=`astparam g btn1_long_on_boot`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_long_on_boot`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_LONG_ON_BOOT="$_t"
		fi
	else
		BTN1_LONG_ON_BOOT="$_t"
	fi
	_t=`astparam g btn1_short_on_eth_off`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_short_on_eth_off`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_SHORT_ON_ETH_OFF="$_t"
		fi
	else
		BTN1_SHORT_ON_ETH_OFF="$_t"
	fi
	_t=`astparam g btn1_long_on_eth_off`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_long_on_eth_off`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_LONG_ON_ETH_OFF="$_t"
		fi
	else
		BTN1_LONG_ON_ETH_OFF="$_t"
	fi
	_t=`astparam g btn2_short`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_short`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_SHORT="$_t"
		fi
	else
		BTN2_SHORT="$_t"
	fi
	_t=`astparam g btn2_long`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_long`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_LONG="$_t"
		fi
	else
		BTN2_LONG="$_t"
	fi
	_t=`astparam g btn2_short_on_boot`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_short_on_boot`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_SHORT_ON_BOOT="$_t"
		fi
	else
		BTN2_SHORT_ON_BOOT="$_t"
	fi
	_t=`astparam g btn2_long_on_boot`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_long_on_boot`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_LONG_ON_BOOT="$_t"
		fi
	else
		BTN2_LONG_ON_BOOT="$_t"
	fi
	_t=`astparam g btn2_short_on_eth_off`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_short_on_eth_off`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_SHORT_ON_ETH_OFF="$_t"
		fi
	else
		BTN2_SHORT_ON_ETH_OFF="$_t"
	fi
	_t=`astparam g btn2_long_on_eth_off`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_long_on_eth_off`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_LONG_ON_ETH_OFF="$_t"
		fi
	else
		BTN2_LONG_ON_ETH_OFF="$_t"
	fi
	_t=`astparam g btn_init`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn_init`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN_INIT="$_t"
		fi
	else
		BTN_INIT="$_t"
	fi
	_t=`astparam g btn1_delay`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn1_delay`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN1_DELAY="$_t"
		fi
	else
		BTN1_DELAY="$_t"
	fi
	_t=`astparam g btn2_delay`
	if echo "$_t" | grep -q "not defined" ; then
		_t=`astparam r btn2_delay`
		if ! { echo "$_t" | grep -q "not defined"; }; then
			BTN2_DELAY="$_t"
		fi
	else
		BTN2_DELAY="$_t"
	fi
}

start_network()
{
	if [ "$PROFILE" = "wifi" ]; then
		if [ "$IS_HOST" = 'y' ]; then
			# For WiFi host, we force to 100Mbps to avoid traffic jam.
			echo 1 > /sys/devices/platform/ftgmac/link_mode
			echo "Force MAC to 100Mbps"
		fi
	fi
	#ifconfig lo up
	config_ip_addr "$1"
}

to_mfg_mode()
{
	# Check the link button again. If it is still pressed, then reset to factory default.
	if [ "$1" = 'clear_rw' ]; then
		echo "Reset to factory default"
		astparam flush
		astparam save
	fi

	# Set static ip address
	local ipaddr=`astparam g asthostip`
	if [ "$ipaddr" = "\"asthostip\" not defined" ]; then
		ipaddr="192.168.0.88"
	fi
	ifconfig eth0:stat "$ipaddr"
	# Start web server
	httpd -h /www &
	# Start telnet server
	start_telnetd
	# Set LEDs
	led_blink "$LED_PWR"
	led_blink "$LED_LINK"
	inform_gui_echo "In MFG mode. IP:$ipaddr"
}

handle_button_on_boot()
{
	if [ "$FPGA_TEST" = '1' ]; then
		return
	fi
	echo "$BTN1_DELAY" > "$GPIO_SYS_PATH"/button_link/delay

	handle_"$BTN_INIT"

	if [ `cat "$GPIO_SYS_PATH"/button_link/state_on_active` = 'On' ]; then
		# Check the link button again. If it is still pressed, then execute BTN1_LONG_ON_BOOT
		echo 4 > "$GPIO_SYS_PATH"/button_link/brightness
		_bs=`cat "$GPIO_SYS_PATH"/button_link/brightness`
		if [ "$_bs" = '1' ] && [ "$BTN1_LONG_ON_BOOT" != 'e_btn_ignore' ]; then
			handle_"$BTN1_LONG_ON_BOOT"
		else
			handle_"$BTN1_SHORT_ON_BOOT"
		fi
	fi

}

start_telnetd()
{
	# Start telnetd
	mount -t devpts devpts /dev/pts
	chown root.root /bin/busybox
	chmod 4755 /bin/busybox
	telnetd $TELNETD_PARAM
}

eth_link_is_off()
{
	# return 0 (true) if link off, otherwise return 1(false)
	ETH_LINK_STATE=`cat $MAC_SYS_PATH/link_state`
	if [ "$ETH_LINK_STATE" = "on" ]; then
		return 1
	fi

	return 0
}

to_readable_ip()
{
	# This function convert LM used IP format (ex:BA04FEA9) to human readable IP format.
	if [ -z "$1" ]; then
		echo 'N/A'
		return
	fi
	_d4='0x'`expr "$1" : '\(..\)......'`
	_d3='0x'`expr "$1" : '..\(..\)....'`
	sleep 0.01
	_d2='0x'`expr "$1" : '....\(..\)..'`
	_d1='0x'`expr "$1" : '......\(..\)'`
	sleep 0.01
	_ip=`printf "%d.%d.%d.%d" $_d1 $_d2 $_d3 $_d4`
	echo "$_ip"

}

get_current_time()
{
	#expr "`cat /proc/uptime`" : '\([[:digit:]]\{1,\}\).*'
	_IFS="$IFS";IFS='. ';set -- $(cat /proc/uptime);IFS="$_IFS"
	echo "$1"
}

_echo_parameters_client()
{
	echo "USB_DISABLE_DEVICES=$USB_DISABLE_DEVICES"
	echo "USB_ENABLE_DEVICES=$USB_ENABLE_DEVICES"
	echo "USB_DISABLE_CLASSES=$USB_DISABLE_CLASSES"
	echo "USB_ENABLE_CLASSES=$USB_ENABLE_CLASSES"
	echo "USB_CONFLICT_POLICY=$USB_CONFLICT_POLICY"
	sleep 0.01
	echo "USB_DEFAULT_POLICY=$USB_DEFAULT_POLICY"
	echo "GUI_SHOW_TEXT_INIT=$GUI_SHOW_TEXT_INIT"
	echo "SHOW_CH_OSD=$SHOW_CH_OSD"
	echo "V_TURN_OFF_SCREEN_ON_PWR_SAVE=$V_TURN_OFF_SCREEN_ON_PWR_SAVE"
	echo "V_SRC_UNAVAILABLE_TIMEOUT=$V_SRC_UNAVAILABLE_TIMEOUT"
	echo "I2S_CLOCK_LOCK_MODE=$I2S_CLOCK_LOCK_MODE"
	echo "LED_DISPLAY_CH_SELECT_V=$LED_DISPLAY_CH_SELECT_V"
	sleep 0.01
}

_echo_parameters_host()
{
	echo "USB_SET_ADDR_HACK=$USB_SET_ADDR_HACK"
	echo "USB_HID_URB_INTERVAL=$USB_HID_URB_INTERVAL"
	echo "USB_QUIRK=$USB_QUIRK"
	echo "REMOTE_EDID_PATCH=$REMOTE_EDID_PATCH"
	echo "LOOPBACK_EDID_PATCH=$LOOPBACK_EDID_PATCH"
	echo "LOOPBACK_DEFAULT_ON=$LOOPBACK_DEFAULT_ON"
	echo "V_RX_DRV=$V_RX_DRV"
	echo "V_FRAME_RATE=$V_FRAME_RATE"
	sleep 0.01
}

echo_parameters()
{
	sleep 0.01
	echo "IS_HOST=$IS_HOST"
	echo "STATE=$STATE"
	echo "ACCESS_ON=$ACCESS_ON"
	sleep 0.01
	echo "DBG=$DBG"
	echo "SHARE_USB=$SHARE_USB"
	echo "SHARE_USB_AUTO_MODE=$SHARE_USB_AUTO_MODE"
	echo "SHARE_USB_ON_FIRST_PEER=$SHARE_USB_ON_FIRST_PEER"
	sleep 0.01
	echo "CH_SELECT=$CH_SELECT"
	echo "HOSTNAMEBYDIPSWITCH=$HOSTNAMEBYDIPSWITCH"
	echo "RESET_CH_ON_BOOT=$RESET_CH_ON_BOOT"
	echo "HOSTNAME_ID=$HOSTNAME_ID"
	echo "IP_MODE=$IP_MODE"
	echo "IPADDR=$IPADDR"
	echo "NETMASK=$NETMASK"
	echo "GATEWAYIP=$GATEWAYIP"
	echo "MULTICAST_LEAVE_FORCE=$MULTICAST_LEAVE_FORCE"
	sleep 0.01
	echo "MULTICAST_ON=$MULTICAST_ON"
	echo "UI_FEATURE=$UI_FEATURE"
	echo "HOSTNAME_PREFIX=$HOSTNAME_PREFIX"
	echo "HOSTNAME_TX_MIDDLE=$HOSTNAME_TX_MIDDLE"
	echo "HOSTNAME_RX_MIDDLE=$HOSTNAME_RX_MIDDLE"
	echo "SCENARIO=$SCENARIO"
	echo "SOC_VER=$SOC_VER"
	echo "SOC_OP_MODE=$SOC_OP_MODE"
	echo "AST_PLATFORM=$AST_PLATFORM"
	echo "MY_MAC=$MY_MAC"
	echo "MY_IP=$MY_IP"
	echo "MY_NETMASK=$MY_NETMASK"
	echo "MY_GATEWAYIP=$MY_GATEWAYIP"
	echo "GWIP=$GWIP"
	sleep 0.01
	echo "ETH_LINK_STATE="`cat $MAC_SYS_PATH/link_state`
	sleep 0.01
	echo "ETH_LINK_MODE="`cat $MAC_SYS_PATH/link_mode`
	sleep 0.01
	echo "LM_LINK_OFF_TIMEOUT=$LM_LINK_OFF_TIMEOUT"
	echo "EDID_USE=$EDID_USE"
	echo "WEB_UI_CFG=$WEB_UI_CFG"
	echo "SEAMLESS_SWITCH=$SEAMLESS_SWITCH"
	echo "JUMBO_MTU=$JUMBO_MTU"
	echo "V_1080I_TO_720P=$V_1080I_TO_720P"
	echo "V_BCD_THRESHOLD=$V_BCD_THRESHOLD"
	echo "V_QUALITY_MODE=$V_QUALITY_MODE"
	echo "NO_PWRBTN=$NO_PWRBTN"
	echo "NO_CEC=$NO_CEC"
	echo "CEC_DRV_OPTION=$CEC_DRV_OPTION"
	echo "MFG_MODE=$MFG_MODE"
	sleep 0.01
	echo "PROFILE=$PROFILE"
	echo "SOIP_GUEST_ON=$SOIP_GUEST_ON"
	echo "SOIP_TYPE2=$SOIP_TYPE2"
	echo "SOIP_TYPE=$SOIP_TYPE"
	echo "SOIP_PORT=$SOIP_PORT"
	echo "S0_BAUDRATE=$S0_BAUDRATE"
	echo "NO_SOIP=$NO_SOIP"
	echo "NO_VIDEO=$NO_VIDEO"
	echo "NO_USB=$NO_USB"
	echo "NO_KMOIP=$NO_KMOIP"
	echo "NO_IR=$NO_IR"
	echo "IR_GUEST_ON=$IR_GUEST_ON"
	echo "IR_SW_DECODE_ON=$IR_SW_DECODE_ON"
	echo "IR_SW_DECODE_NEC_CFG=$IR_SW_DECODE_NEC_CFG"
	echo "NO_I2S=$NO_I2S"
	sleep 0.01
	echo "UDP_ON=$UDP_ON"
	echo "EN_LOG=$EN_LOG"
	echo "V_REJECT_HDCP=$V_REJECT_HDCP"
	echo "V_IGNORE_CTS7_33=$V_IGNORE_CTS7_33"
	echo "V_CHUNK_SIZE=$V_CHUNK_SIZE"
	echo "V_OUTPUT_TIMING_CONVERT=$V_OUTPUT_TIMING_CONVERT"
	echo "V_HDMI_HDR_MODE=$V_HDMI_HDR_MODE"
	echo "A_IO_SELECT=$A_IO_SELECT"
	echo "A_OUTPUT_DELAY=$A_OUTPUT_DELAY"
	echo "A_ANALOG_IN_VOL=$A_ANALOG_IN_VOL"
	echo "A_ANALOG_OUT_VOL=$A_ANALOG_OUT_VOL"
	sleep 0.01
	# Video Wall Stuff
	echo "EN_VIDEO_WALL=$EN_VIDEO_WALL"
	echo "VW_VAR_MAX_ROW=$VW_VAR_MAX_ROW"
	echo "VW_VAR_MAX_COLUMN=$VW_VAR_MAX_COLUMN"
	echo "VW_VAR_ROW=$VW_VAR_ROW"
	echo "VW_VAR_COLUMN=$VW_VAR_COLUMN"
	echo "VW_VAR_MONINFO_HA=$VW_VAR_MONINFO_HA"
	echo "VW_VAR_MONINFO_HT=$VW_VAR_MONINFO_HT"
	echo "VW_VAR_MONINFO_VA=$VW_VAR_MONINFO_VA"
	echo "VW_VAR_MONINFO_VT=$VW_VAR_MONINFO_VT"
	echo "VW_VAR_POS_IDX=$VW_VAR_POS_IDX"
	sleep 0.01
	echo "VW_VAR_H_SHIFT=$VW_VAR_H_SHIFT"
	echo "VW_VAR_V_SHIFT=$VW_VAR_V_SHIFT"
	echo "VW_VAR_H_SCALE=$VW_VAR_H_SCALE"
	echo "VW_VAR_V_SCALE=$VW_VAR_V_SCALE"
	echo "MY_ROW_ID=$MY_ROW_ID"
	echo "THE_ROW_ID=$THE_ROW_ID"
	echo "VW_VAR_POS_MAX_ROW=$VW_VAR_POS_MAX_ROW"
	echo "VW_VAR_POS_MAX_COL=$VW_VAR_POS_MAX_COL"
	sleep 0.01
	echo "VW_VAR_POS_R=$VW_VAR_POS_R"
	echo "VW_VAR_POS_C=$VW_VAR_POS_C"
	echo "VW_VAR_DELAY_KICK=$VW_VAR_DELAY_KICK"
	echo "VW_VAR_TAIL_IDX=$VW_VAR_TAIL_IDX"
	echo "VW_VAR_STRETCH_TYPE=$VW_VAR_STRETCH_TYPE"
	echo "VW_VAR_ROTATE=$VW_VAR_ROTATE"
	# Button handlers
	echo "BTN1_SHORT=$BTN1_SHORT"
	echo "BTN1_LONG=$BTN1_LONG"
	echo "BTN1_SHORT_ON_BOOT=$BTN1_SHORT_ON_BOOT"
	echo "BTN1_LONG_ON_BOOT=$BTN1_LONG_ON_BOOT"
	echo "BTN1_SHORT_ON_ETH_OFF=$BTN1_SHORT_ON_ETH_OFF"
	echo "BTN1_LONG_ON_ETH_OFF=$BTN1_LONG_ON_ETH_OFF"
	sleep 0.01
	echo "BTN2_SHORT=$BTN2_SHORT"
	echo "BTN2_LONG=$BTN2_LONG"
	echo "BTN2_SHORT_ON_BOOT=$BTN2_SHORT_ON_BOOT"
	echo "BTN2_LONG_ON_BOOT=$BTN2_LONG_ON_BOOT"
	echo "BTN2_SHORT_ON_ETH_OFF=$BTN2_SHORT_ON_ETH_OFF"
	echo "BTN2_LONG_ON_ETH_OFF=$BTN2_LONG_ON_ETH_OFF"
	echo "BTN_INIT=$BTN_INIT"
	echo "BTN1_DELAY=$BTN1_DELAY"
	echo "BTN2_DELAY=$BTN2_DELAY"
	sleep 0.01

	if [ "$IS_HOST" = 'y' ]; then
		_echo_parameters_host
	else
		_echo_parameters_client
	fi
}

_echo_parameters_json_client()
{
	echo "\"USB_DISABLE_DEVICES\":\"$USB_DISABLE_DEVICES\","
	echo "\"USB_ENABLE_DEVICES\":\"$USB_ENABLE_DEVICES\","
	echo "\"USB_DISABLE_CLASSES\":\"$USB_DISABLE_CLASSES\","
	echo "\"USB_ENABLE_CLASSES\":\"$USB_ENABLE_CLASSES\","
	echo "\"USB_CONFLICT_POLICY\":\"$USB_CONFLICT_POLICY\","
	sleep 0.01
	echo "\"USB_DEFAULT_POLICY\":\"$USB_DEFAULT_POLICY\","
	echo "\"GUI_SHOW_TEXT_INIT\":\"$GUI_SHOW_TEXT_INIT\","
	echo "\"SHOW_CH_OSD\":\"$SHOW_CH_OSD\","
	echo "\"V_TURN_OFF_SCREEN_ON_PWR_SAVE\":\"$V_TURN_OFF_SCREEN_ON_PWR_SAVE\","
	echo "\"V_SRC_UNAVAILABLE_TIMEOUT\":\"$V_SRC_UNAVAILABLE_TIMEOUT\","
	echo "\"I2S_CLOCK_LOCK_MODE\":\"$I2S_CLOCK_LOCK_MODE\","
	sleep 0.01
}

_echo_parameters_json_host()
{
	echo "\"USB_SET_ADDR_HACK\":\"$USB_SET_ADDR_HACK\","
	echo "\"USB_HID_URB_INTERVAL\":\"$USB_HID_URB_INTERVAL\","
	echo "\"USB_QUIRK\":\"$USB_QUIRK\","
	echo "\"REMOTE_EDID_PATCH\":\"$REMOTE_EDID_PATCH\","
	echo "\"LOOPBACK_EDID_PATCH\":\"$LOOPBACK_EDID_PATCH\","
	echo "\"LOOPBACK_DEFAULT_ON\":\"$LOOPBACK_DEFAULT_ON\","
	echo "\"V_RX_DRV\":\"$V_RX_DRV\","
	echo "\"V_FRAME_RATE\":\"$V_FRAME_RATE\","
	sleep 0.01
}

echo_parameters_json()
{
	sleep 0.01
	echo "{"
	echo "\"IS_HOST\":\"$IS_HOST\","
	echo "\"STATE\":\"$STATE\","
	echo "\"ACCESS_ON\":\"$ACCESS_ON\","
	sleep 0.01
	echo "\"DBG\":\"$DBG\","
	echo "\"SHARE_USB\":\"$SHARE_USB\","
	echo "\"SHARE_USB_AUTO_MODE\":\"$SHARE_USB_AUTO_MODE\","
	echo "\"SHARE_USB_ON_FIRST_PEER\":\"$SHARE_USB_ON_FIRST_PEER\","
	sleep 0.01
	echo "\"CH_SELECT\":\"$CH_SELECT\","
	echo "\"HOSTNAMEBYDIPSWITCH\":\"$HOSTNAMEBYDIPSWITCH\","
	echo "\"RESET_CH_ON_BOOT\":\"$RESET_CH_ON_BOOT\","
	echo "\"HOSTNAME_ID\":\"$HOSTNAME_ID\","
	echo "\"IP_MODE\":\"$IP_MODE\","
	echo "\"IPADDR\":\"$IPADDR\","
	echo "\"NETMASK\":\"$NETMASK\","
	echo "\"GATEWAYIP\":\"$GATEWAYIP\","
	echo "\"MULTICAST_LEAVE_FORCE\":\"$MULTICAST_LEAVE_FORCE\","
	sleep 0.01
	echo "\"MULTICAST_ON\":\"$MULTICAST_ON\","
	echo "\"UI_FEATURE\":\"$UI_FEATURE\","
	echo "\"HOSTNAME_PREFIX\":\"$HOSTNAME_PREFIX\","
	echo "\"HOSTNAME_TX_MIDDLE\":\"$HOSTNAME_TX_MIDDLE\","
	echo "\"HOSTNAME_RX_MIDDLE\":\"$HOSTNAME_RX_MIDDLE\","
	echo "\"SCENARIO\":\"$SCENARIO\","
	echo "\"SOC_VER\":\"$SOC_VER\","
	echo "\"SOC_OP_MODE\":\"$SOC_OP_MODE\","
	echo "\"AST_PLATFORM\":\"$AST_PLATFORM\","
	echo "\"MY_MAC\":\"$MY_MAC\","
	echo "\"MY_IP\":\"$MY_IP\","
	echo "\"MY_NETMASK\":\"$MY_NETMASK\","
	echo "\"MY_GATEWAYIP\":\"$MY_GATEWAYIP\","
	echo "\"GWIP\":\"$GWIP\","
	sleep 0.01
	echo "\"ETH_LINK_STATE\":\""`cat $MAC_SYS_PATH/link_state`"\","
	sleep 0.01
	echo "\"ETH_LINK_MODE\":\""`cat $MAC_SYS_PATH/link_mode`"\","
	sleep 0.01
	echo "\"LM_LINK_OFF_TIMEOUT\":\"$LM_LINK_OFF_TIMEOUT\","
	echo "\"EDID_USE\":\"$EDID_USE\","
	echo "\"WEB_UI_CFG\":\"$WEB_UI_CFG\","
	echo "\"SEAMLESS_SWITCH\":\"$SEAMLESS_SWITCH\","
	echo "\"JUMBO_MTU\":\"$JUMBO_MTU\","
	echo "\"V_1080I_TO_720P\":\"$V_1080I_TO_720P\","
	echo "\"V_BCD_THRESHOLD\":\"$V_BCD_THRESHOLD\","
	echo "\"V_QUALITY_MODE\":\"$V_QUALITY_MODE\","
	echo "\"NO_PWRBTN\":\"$NO_PWRBTN\","
	echo "\"NO_CEC\":\"$NO_CEC\","
	echo "\"CEC_DRV_OPTION\":\"$CEC_DRV_OPTION\","
	echo "\"MFG_MODE\":\"$MFG_MODE\","
	echo "\"PROFILE\":\"$PROFILE\","
	sleep 0.01
	echo "\"SOIP_GUEST_ON\":\"$SOIP_GUEST_ON\","
	echo "\"SOIP_TYPE2\":\"$SOIP_TYPE2\","
	echo "\"SOIP_TYPE\":\"$SOIP_TYPE\","
	echo "\"SOIP_PORT\":\"$SOIP_PORT\","
	echo "\"NO_SOIP\":\"$NO_SOIP\","
	echo "\"S0_BAUDRATE\":\"$S0_BAUDRATE\","
	echo "\"NO_VIDEO\":\"$NO_VIDEO\","
	echo "\"NO_USB\":\"$NO_USB\","
	echo "\"NO_KMOIP\":\"$NO_KMOIP\","
	echo "\"NO_IR\":\"$NO_IR\","
	echo "\"IR_GUEST_ON\":\"$IR_GUEST_ON\","
	echo "\"IR_SW_DECODE_ON\":\"$IR_SW_DECODE_ON\","
	echo "\"IR_SW_DECODE_NEC_CFG\":\"$IR_SW_DECODE_NEC_CFG\","
	echo "\"NO_I2S\":\"$NO_I2S\","
	echo "\"UDP_ON\":\"$UDP_ON\","
	echo "\"EN_LOG\":\"$EN_LOG\","
	sleep 0.01
	echo "\"V_REJECT_HDCP\":\"$V_REJECT_HDCP\","
	echo "\"V_IGNORE_CTS7_33\":\"$V_IGNORE_CTS7_33\","
	echo "\"V_CHUNK_SIZE\":\"$V_CHUNK_SIZE\","
	echo "\"V_OUTPUT_TIMING_CONVERT\":\"$V_OUTPUT_TIMING_CONVERT\","
	echo "\"V_HDMI_HDR_MODE\":\"$V_HDMI_HDR_MODE\","
	echo "\"A_IO_SELECT\":\"$A_IO_SELECT\","
	echo "\"A_OUTPUT_DELAY\":\"$A_OUTPUT_DELAY\","
	echo "\"A_ANALOG_IN_VOL\":\"$A_ANALOG_IN_VOL\","
	echo "\"A_ANALOG_OUT_VOL\":\"$A_ANALOG_OUT_VOL\","
	sleep 0.01
	# Video Wall Stuff
	echo "\"EN_VIDEO_WALL\":\"$EN_VIDEO_WALL\","
	echo "\"VW_VAR_MAX_ROW\":\"$VW_VAR_MAX_ROW\","
	echo "\"VW_VAR_MAX_COLUMN\":\"$VW_VAR_MAX_COLUMN\","
	echo "\"VW_VAR_ROW\":\"$VW_VAR_ROW\","
	echo "\"VW_VAR_COLUMN\":\"$VW_VAR_COLUMN\","
	echo "\"VW_VAR_MONINFO_HA\":\"$VW_VAR_MONINFO_HA\","
	echo "\"VW_VAR_MONINFO_HT\":\"$VW_VAR_MONINFO_HT\","
	echo "\"VW_VAR_MONINFO_VA\":\"$VW_VAR_MONINFO_VA\","
	echo "\"VW_VAR_MONINFO_VT\":\"$VW_VAR_MONINFO_VT\","
	sleep 0.01
	echo "\"VW_VAR_POS_IDX\":\"$VW_VAR_POS_IDX\","
	echo "\"VW_VAR_H_SHIFT\":\"$VW_VAR_H_SHIFT\","
	echo "\"VW_VAR_V_SHIFT\":\"$VW_VAR_V_SHIFT\","
	echo "\"VW_VAR_H_SCALE\":\"$VW_VAR_H_SCALE\","
	echo "\"VW_VAR_V_SCALE\":\"$VW_VAR_V_SCALE\","
	echo "\"MY_ROW_ID\":\"$MY_ROW_ID\","
	echo "\"THE_ROW_ID\":\"$THE_ROW_ID\","
	echo "\"VW_VAR_POS_MAX_ROW\":\"$VW_VAR_POS_MAX_ROW\","
	echo "\"VW_VAR_POS_MAX_COL\":\"$VW_VAR_POS_MAX_COL\","
	echo "\"VW_VAR_POS_R\":\"$VW_VAR_POS_R\","
	echo "\"VW_VAR_POS_C\":\"$VW_VAR_POS_C\","
	sleep 0.01
	echo "\"VW_VAR_DELAY_KICK\":\"$VW_VAR_DELAY_KICK\","
	echo "\"VW_VAR_TAIL_IDX\":\"$VW_VAR_TAIL_IDX\","
	echo "\"VW_VAR_STRETCH_TYPE\":\"$VW_VAR_STRETCH_TYPE\","
	echo "\"VW_VAR_ROTATE\":\"$VW_VAR_ROTATE\","
	# Button handlers
	echo "\"BTN1_SHORT\":\"$BTN1_SHORT\","
	echo "\"BTN1_LONG\":\"$BTN1_LONG\","
	echo "\"BTN1_SHORT_ON_BOOT\":\"$BTN1_SHORT_ON_BOOT\","
	echo "\"BTN1_LONG_ON_BOOT\":\"$BTN1_LONG_ON_BOOT\","
	echo "\"BTN1_SHORT_ON_ETH_OFF\":\"$BTN1_SHORT_ON_ETH_OFF\","
	echo "\"BTN1_LONG_ON_ETH_OFF\":\"$BTN1_LONG_ON_ETH_OFF\","
	sleep 0.01
	echo "\"BTN2_SHORT\":\"$BTN2_SHORT\","
	echo "\"BTN2_LONG\":\"$BTN2_LONG\","
	echo "\"BTN2_SHORT_ON_BOOT\":\"$BTN2_SHORT_ON_BOOT\","
	echo "\"BTN2_LONG_ON_BOOT\":\"$BTN2_LONG_ON_BOOT\","
	echo "\"BTN2_SHORT_ON_ETH_OFF\":\"$BTN2_SHORT_ON_ETH_OFF\","
	echo "\"BTN2_LONG_ON_ETH_OFF\":\"$BTN2_LONG_ON_ETH_OFF\","
	echo "\"BTN_INIT\":\"$BTN_INIT\","
	echo "\"BTN1_DELAY\":\"$BTN1_DELAY\","
	echo "\"BTN2_DELAY\":\"$BTN2_DELAY\","
	sleep 0.01

	if [ "$IS_HOST" = 'y' ]; then
		_echo_parameters_json_host
	else
		_echo_parameters_json_client
	fi

	# End
	echo "\"_result\":\"pass\""
	echo "}"
	sleep 0.01
}

dump_parameters_json()
{
	echo_parameters_json > lm_params_json
}

dump_parameters()
{
	echo_parameters
	echo_parameters > lm_params
}


# Reference from bashlib
parse_n_exec()
{
	QUERY_STRING="$*"
	if [ -n "${QUERY_STRING}" ]; then
		# name=value params, separated by either '&' or ';'
		if echo ${QUERY_STRING} | grep '=' >/dev/null ; then
			for Q in $(echo ${QUERY_STRING} | tr ";&" "\012") ; do
				#
				# Clear our local variables
				#
				unset name
				unset value
				unset tmpvalue

				#
				# get the name of the key, and decode it
				#
				name=${Q%%=*}
				#name=$(echo ${name} | \
				#	 sed -e 's/%\(\)/\\\x/g' | \
				#	 tr "+" " ")
				#name=$(echo ${name} | \
				#	 tr -d ".-")
				#name=$(printf ${name})
				name=$(httpd -d ${name})

				#
				# get the value and decode it. This is tricky... printf chokes on
				# hex values in the form \xNN when there is another hex-ish value
				# (i.e., a-fA-F) immediately after the first two. My (horrible)
				# solution is to put a space aftet the \xNN, give the value to
				# printf, and then remove it.
				#
				tmpvalue=${Q#*=}
				#tmpvalue=$(echo ${tmpvalue} | \
				#		 sed -e 's/%\(..\)/\\\x\1 /g')
				# Replace '-' as '\x2D', otherwise following printf won't recongize it.
				#tmpvalue=$(echo ${tmpvalue} | \
				#		 sed -e 's/\(-\)/\\\x2D /g')
				#echo "Intermediate \$value: ${tmpvalue}" 1>&2

				#
				# Iterate through tmpvalue and printf each string, and append it to
				# value
				#
				#for i in ${tmpvalue}; do
				#	g=$(printf ${i})
				#	value="${value}${g}"
				#done
				#value=$(echo ${value})
				value=$(httpd -d ${tmpvalue})

				#eval "export FORM_${name}='${value}'"
				if [ "$name" = 'cmd' ]; then
					# Replace "+" with space
					#cmd=$(echo ${value} | tr "+" "\x20")
					cmd=${value}
					echo "cmd:$cmd"
					#Bruce120510.
					#Invalid shell $cmd will cause "eval" exit itself and hence kills LM.
					#Run "eval" in background protects LM.
					{ eval "$cmd"; } &
				fi
			done
		else # keywords: foo.cgi?a+b+c
			Q=$(echo ${QUERY_STRING} | tr '+' ' ')
			{ eval "export KEYWORDS='${Q}'"; } &
		fi
	fi

}

set_mtu()
{
	if [ "$SOC_OP_MODE" = '1' ]; then
		return
	fi
	# For SoC supporting Jumbo frame
	if eth_link_is_off ; then
		ifconfig eth0 mtu 1500
	else
		ETH_LINK_MODE=`cat ${MAC_SYS_PATH}/link_mode`
		if [ "$ETH_LINK_MODE" = "1G" ] ; then
			ifconfig eth0 mtu "$JUMBO_MTU"
		fi
	fi
}

set_display_config()
{
	# Overwrite default EDID
	if [ -f "/share/edid_hdmi.txt" ]; then
		cat /share/edid_hdmi.txt > $DISPLAY_SYS_PATH/default_edid_hdmi
	fi
	if [ -f "/share/edid_dvi.txt" ]; then
		cat /share/edid_dvi.txt > $DISPLAY_SYS_PATH/default_edid_dvi
	fi
	if [ -f "/share/edid_vga.txt" ]; then
		cat /share/edid_vga.txt > $DISPLAY_SYS_PATH/default_edid_vga
	fi

	if [ "$SOC_OP_MODE" -ge '2' ]; then
		echo "$V_DUAL_PORT" > $DISPLAY_SYS_PATH/dual_port
	fi

	if [ "$IS_HOST" = 'n' ]; then
		if [ "$V_IGNORE_CTS7_33" = 'y' ]; then
			echo 1 > $DISPLAY_SYS_PATH/ignore_cts7_33
		fi
	fi
}

set_video_dequeue_delay()
{
	echo "$1" > "$VIDEO_SYS_PATH"/delay_dequeue
}

set_igmp_report()
{
	pkill igmp_report
	igmp_report.sh $MY_IP &
}

set_igmp_leave_force()
{
	if [ "$MULTICAST_LEAVE_FORCE" -gt '0' ]; then
		echo $MULTICAST_LEAVE_FORCE > /proc/net/igmp_leave_force
		ifconfig eth0 allmulti
	fi
}
# Construct scriptlet
######################################################
# vlmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @v_lm_query q ve_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @v_lm_set s ve_param_set:\$key:\$value
	;;
	dump)
		ipc @v_lm_query q ve_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/vlmparam
chmod a+x /usr/local/bin/vlmparam
######################################################
# ulmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @u_lm_query q ue_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @u_lm_set s ue_param_set:\$key:\$value
	;;
	dump)
		ipc @u_lm_query q ue_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/ulmparam
chmod a+x /usr/local/bin/ulmparam
######################################################
# almparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @a_lm_query q ae_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @a_lm_set s ae_param_set:\$key:\$value
	;;
	dump)
		ipc @a_lm_query q ae_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/almparam
chmod a+x /usr/local/bin/almparam
######################################################
# rlmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @r_lm_query q re_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @r_lm_set s re_param_set:\$key:\$value
	;;
	dump)
		ipc @r_lm_query q re_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/rlmparam
chmod a+x /usr/local/bin/rlmparam
######################################################
# slmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @s_lm_query q se_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @s_lm_set s se_param_set:\$key:\$value
	;;
	dump)
		ipc @s_lm_query q se_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/slmparam
chmod a+x /usr/local/bin/slmparam
######################################################
# plmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @p_lm_query q pe_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @p_lm_set s pe_param_set:\$key:\$value
	;;
	dump)
		ipc @p_lm_query q pe_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/plmparam
chmod a+x /usr/local/bin/plmparam
######################################################
# clmparam {g|s|dump} {param_to_query} [value_to_set]
echo "#!/bin/sh

cmd=\$1
key=\$2
value=\$3
case "\$cmd" in
	g)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @c_lm_query q ce_param_query:\$key
	;;
	s)
		if [ \$# -lt 2 ]; then
			exit 1
		fi
		ipc @c_lm_set s ce_param_set:\$key:\$value
	;;
	dump)
		ipc @c_lm_query q ce_param_dump
	;;
	*)
		echo \"Unsupported command!\" >&2
	;;
esac
" > /usr/local/bin/clmparam
chmod a+x /usr/local/bin/clmparam

######################################################
# lmparam {g|s|dump|jdump} [arg1] [arg2]
echo "#!/bin/sh

. /usr/local/bin/bash/utilities.sh

cmd=\"\$1\"
shift
arg1=\"\$1\"
shift
arg2=\"\$*\"

case \"\$cmd\" in
	g)
		__token=\$RANDOM
		if [ -f /var/lm_var\$__token ]; then
			rm -f /var/lm_var\$__token
			sleep 0.01
		fi
		mknod /var/lm_var\$__token p
		sleep 0.01
		( ast_send_event -1 e_var_get::\$arg1::\$__token ) &
		sleep 0.01
		cat /var/lm_var\$__token
		sleep 0.01
		rm -f /var/lm_var\$__token
	;;
	s)
		e e_var_set::\$arg1::\$arg2
	;;
	dump)
		__token=\$RANDOM
		if [ -f /var/lmparams\$__token ]; then
			rm -f /var/lmparams\$__token
			sleep 0.01
		fi
		mknod /var/lmparams\$__token p
		sleep 0.01
		(  ast_send_event -1 e_var_dump::ini::\$__token ) &
		sleep 0.01
		cat /var/lmparams\$__token
		sleep 0.01
		rm -f /var/lmparams\$__token
	;;
	jdump)
		__token=\$RANDOM
		if [ -f /var/lmparams\$__token ]; then
			rm -f /var/lmparams\$__token
			sleep 0.01
		fi
		mknod /var/lmparams\$__token p
		sleep 0.01
		( ast_send_event -1 e_var_dump::json::\$__token ) &
		sleep 0.01
		cat /var/lmparams\$__token
		sleep 0.01
		rm -f /var/lmparams\$__token
	;;
	*)
		echo \"invalid command\"
	;;
esac
" > /usr/local/bin/lmparam
chmod a+x /usr/local/bin/lmparam

_var_dump()
{
	# Parse e_var_dump::${type}::${_token}
	_IFS="$IFS";IFS='::';set -- $*;IFS="$_IFS"
	_type="$3"
	_token="$5"

	# Use PIPE here takes risk to block LM forever.
	if [ -p /var/lmparams$_token ]; then
		case "$_type" in
			json)
				# Bruce151021. Multiple Web UI lmparam jdump refresh kills LM.
				# For unknown reason, above sequence may causes below write
				# into pipe command block forever. Even I cat the pipe manually, LM still blocked.
				# To avoid LM block forever, I put below command into background process.
				# The bonus is the problem disappeared after put this command into background. (!!?)
				# Bruce160310. I believe it is caused by 'broken pipe' that will terminate LM.
				# We MUST put every PIPE write into background to avoid LM crash.
				sleep 0.01
				{ echo_parameters_json; } > /var/lmparams$_token &
				sleep 0.01
			;;
			*)
				# default 'ini'
				sleep 0.01
				{ echo_parameters; } > /var/lmparams$_token &
				sleep 0.01
			;;
		esac
	else
		case "$_type" in
			json)
				echo_parameters_json
			;;
			*)
				# default 'ini'
				echo_parameters
			;;
		esac
	fi
}

_var_get()
{
	# Parse e_var_get::${_var}::${_token}
	_IFS="$IFS";IFS='::';set -- $*;IFS="$_IFS"
	_var="$3"
	_token="$5"
	_value=`eval echo "\\$$_var"`

	# Use PIPE here takes risk to block LM forever.
	if [ -p /var/lm_var$_token ]; then
		sleep 0.01
		#echo "$_var=$_value" > /var/lm_var
		#echo "$_value" > /var/lm_var$_token &
		{ printf "%s" "$_value"; } > /var/lm_var$_token &
		sleep 0.01
	else
		#echo "$_var=$_value"
		echo "$_value"
		if [ "$_var" != "" ]; then
			#echo "$_value" > /var/$_var
			{ printf "%s" "$_value"; } > /var/$_var &
		fi
	fi
}

_var_set()
{
	# Parse e_var_set::${_var}::${_value}
	_IFS="$IFS";IFS='::';set -- $*;IFS="$_IFS"
	_var="$3"
	_value="$5"
	eval "$_var=\"$_value\""
	echo "$_var=$_value"
}
######################################################

update_node_info()
{
	case "$#" in
		1)
			case "$1" in
				ch_select)
					if [ "$IS_HOST" = 'n' ]; then
						node_query --if essential --of essential \
							--set_key CH_SELECT_V=$CH_SELECT_V \
							--set_key CH_SELECT_U=$CH_SELECT_U \
							--set_key CH_SELECT_A=$CH_SELECT_A \
							--set_key CH_SELECT_R=$CH_SELECT_R \
							--set_key CH_SELECT_S=$CH_SELECT_S \
							--set_key CH_SELECT_P=$CH_SELECT_P \
							--set_key CH_SELECT_C=$CH_SELECT_C
					else
						node_query --if essential --of essential \
							--set_key CH_SELECT=$CH_SELECT
					fi
				;;
				*)
					echo "update_node_info() wrong arg ($1)!"
				;;
			esac
		;;
		2)
			local _key=$1 _value=$2
			node_query --if essential --of essential --set_key $1=$2
		;;
		*)
			# multi key pair set for 'essential' reply_type
			node_query --if essential --of essential --set_key MY_MAC=$MY_MAC
			node_query --if essential --of essential --set_key MY_IP=$MY_IP
			node_query --if essential --of essential --set_key IS_HOST=$IS_HOST
			node_query --if essential --of essential --set_key HOSTNAME=$HOSTNAME
			node_query --if essential --of essential --set_key HOSTNAME_PREFIX=$HOSTNAME_PREFIX
			node_query --if essential --of essential --set_key MULTICAST_ON=$MULTICAST_ON
			#node_query --if essential --of essential --set_key JUMBO_MTU=$JUMBO_MTU
			node_query --if essential --of essential --set_key NO_VIDEO=$NO_VIDEO
			node_query --if essential --of essential --set_key NO_USB=$NO_USB
			node_query --if essential --of essential --set_key NO_KMOIP=$NO_KMOIP
			node_query --if essential --of essential --set_key NO_I2S=$NO_I2S
			node_query --if essential --of essential --set_key NO_SOIP=$NO_SOIP
			node_query --if essential --of essential --set_key SOIP_GUEST_ON=$SOIP_GUEST_ON
			node_query --if essential --of essential --set_key SOIP_TYPE=$SOIP_TYPE
			node_query --if essential --of essential --set_key NO_IR=$NO_IR
			node_query --if essential --of essential --set_key IR_GUEST_ON=$IR_GUEST_ON
			node_query --if essential --of essential --set_key NO_PWRBTN=$NO_PWRBTN
			node_query --if essential --of essential --set_key NO_CEC=$NO_CEC
			if [ "$IS_HOST" = 'n' ]; then
				node_query --if essential --of essential \
					--set_key CH_SELECT_V=$CH_SELECT_V \
					--set_key CH_SELECT_U=$CH_SELECT_U \
					--set_key CH_SELECT_A=$CH_SELECT_A \
					--set_key CH_SELECT_R=$CH_SELECT_R \
					--set_key CH_SELECT_S=$CH_SELECT_S \
					--set_key CH_SELECT_P=$CH_SELECT_P \
					--set_key CH_SELECT_C=$CH_SELECT_C
				node_query --if essential --of essential \
					--set_key ACCESS_ON_V=$ACCESS_ON_V \
					--set_key ACCESS_ON_U=$ACCESS_ON_U \
					--set_key ACCESS_ON_A=$ACCESS_ON_A \
					--set_key ACCESS_ON_R=$ACCESS_ON_R \
					--set_key ACCESS_ON_S=$ACCESS_ON_S \
					--set_key ACCESS_ON_P=$ACCESS_ON_P \
					--set_key ACCESS_ON_C=$ACCESS_ON_C
			else
				node_query --if essential --of essential \
					--set_key CH_SELECT=$CH_SELECT
				node_query --if essential --of essential \
					--set_key ACCESS_ON=$ACCESS_ON
			fi
		;;
	esac
}

notify_node_info_chg()
{
	node_query --reply_type NQ_NOTIFY_CHG --match_key FROM_MAC=$MY_MAC --match_key SESSION=$RANDOM --match_key CHG_INFO=essential &
}

init_version_file() {
	sed -i "1 c $MODEL_NUMBER" /etc/version
	echo "==============================="
	cat /etc/version
	echo "==============================="
}

init_info_file() {
	FIRMWARE_VERSION=`sed -n 2p /etc/version`
	echo "{" > /etc/board_info.json
	echo "    \"model\": \"$MODEL_NUMBER\"," >> /etc/board_info.json
	echo "    \"serial number\": \"$SERIAL_NUMBER\"," >> /etc/board_info.json
	echo "    \"mac address\": \"$ETH_ADDR\"," >> /etc/board_info.json
	echo "    \"board version\": \"$BOARD_VERSION\"," >> /etc/board_info.json
	echo "    \"firmware version\": \"$FIRMWARE_VERSION\"" >> /etc/board_info.json
	echo "}" >> /etc/board_info.json
}

init_p3k_cfg_file() {
	echo "init_p3k_cfg_file"

	MY_MAC=`astconfig mac`

	HOSTNAME_ID=`astparam g hostname_id`
	case "$HOSTNAME_ID" in
		*'not defined')
			HOSTNAME_ID=`astparam r hostname_id`
			case "$HOSTNAME_ID" in
				*'not defined')
						HOSTNAME_ID="$MY_MAC"
				;;
				*)
				;;
			esac
		;;
		*)
		;;
	esac

	MODEL_NUMBER=$(astparam r model_number)
	if echo "$MODEL_NUMBER" | grep -q "not defined"; then
		MODEL_NUMBER='UNKNOWN'
	fi

	HOSTNAME_CUSTOMIZED=`astparam g hostname_customized`
	case "$HOSTNAME_CUSTOMIZED" in
		*'not defined')
			HOSTNAME_CUSTOMIZED=""
		;;
		*)
		;;
	esac

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

	p3ktcp -c
}

handle_e_ldap() {
	init_ldap_params
	pkill -9 ldap_daemon
	start_ldap_daemon
}

handle_e_802_1x() {
	refresh_ieee802_1x_params
	stop_ieee802dot1x_daemon
	start_ieee802dot1x_daemon
}


start_ldap_daemon()
{
	if [ '$LDAP_ENABLE' = 'y' ]; then
		case "$LDAP_MODE" in
			dn)
				ldap_daemon --uri=$LDAP_URI --dn=$LDAP_BIND_DN --password=$LDAP_PASSWORD --ca_cert=/data/ldap_ca.pem -n 1
				;;
			uid)
				ldap_daemon --uri=$LDAP_URI --base_dn=$LDAP_BASE_DN --password=$LDAP_PASSWORD --uid=$LDAP_UID --ca_cert=/data/ldap_ca.pem  -n 2
				;;
		esac
	fi
}

stop_ieee802dot1x_daemon()
{
	pkill -9 wpa_supplicant
}

init_wpa_supplicant_conf_eap_tls()
{
	local EAP_METOHD
	cat > /etc/wpa_supplicant.conf << EOF
network={
	key_mgmt=IEEE8021X
	eap=TLS
	identity="$IEEE802_1X_TLS_USER"
	ca_cert="$IEEE802_1X_TLS_CA_CERT_PATH"
	client_cert="$IEEE802_1X_TLS_CLIENT_CERT_PATH"
	private_key="$IEEE802_1X_TLS_PRIVATE_KEY_PATH"
	private_key_passwd="$IEEE802_1X_TLS_PRIVATE_KEY_PASSWORD"
}
EOF
}

init_wpa_supplicant_conf_eap_mschapv2()
{
	cat > /etc/wpa_supplicant.conf << EOF
network={
	key_mgmt=IEEE8021X
	eap=PEAP
	identity="$IEEE802_1X_MSCHAPV2_USER"
	password="$IEEE802_1X_MSCHAPV2_PASSWORD"
	phase2="auth=MSCHAPV2"
}
EOF
}

start_ieee802dot1x_daemon()
{
	if [ "$IEEE802_1X_MODE" = "on" ]; then
		case "$IEEE802_1X_AUTH_METHOD" in
			eap_mschapv2|eap_mschap)
				init_wpa_supplicant_conf_eap_mschapv2
				;;
			eap_tls)
				init_wpa_supplicant_conf_eap_tls
				;;
			*)
				echo "#ERROR: not support the 8021x authentication method: $IEEE802_1X_AUTH_METHOD"
				return
				;;
		esac
		wpa_supplicant_802_1x -B -i eth0 -c /etc/wpa_supplicant.conf -D wired
	fi
}


##########################################################################################
#syslogd Options:
#
#        -n              Run in foreground
#        -O FILE         Log to given file (default:/var/log/messages)
#        -l n            Set local log level
#        -S              Smaller logging output
#        -s SIZE         Max size (KB) before rotate (default:200KB, 0=off)
#        -b NUM          Number of rotated logs to keep (default:1, max=99, 0=purge)
#        -R HOST[:PORT]  Log to IP or hostname on PORT (default PORT=514/UDP)
#        -L              Log locally and via network (default is network only if -R)
#        -D              Drop duplicates
#        -C[size(KiB)]   Log to shared mem buffer (read it using logread)
##########################################################################################

handle_e_log()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	LOG_ACTION=$3
	LOG_PERIOD=$5
	echo "LOG_ACTION=$LOG_ACTION"
	echo "LOG_PERIOD=$LOG_PERIOD"
	if [ -z "$LOG_ACTION" ]; then
		LOG_ACTION=$(jq -r .log_setting.active $LOG_JSON_PATH | tr A-Z a-z)
	fi
	if [ -z "$LOG_ACTION" ]; then
		LOG_ACTION='pause'
	fi
	case $LOG_ACTION in
		start|on)
			pkill -9 syslogd
			pkill -9 klogd
			syslogd -l 7 -s 1000 -b 1
			klogd
			;;
		pause|stop|off)
			pkill -9 syslogd
			pkill -9 klogd
			;;
		resume)
			pkill -9 syslogd
			pkill -9 klogd
			syslogd -l 7 -s 1000 -b 1
			klogd
			;;
		reset)
			pkill -9 syslogd
			pkill -9 klogd
			rm -f /var/log/messages*
			syslogd -l 7 -s 1000 -b 1
			klogd
			;;
	esac
}

init_json_cfg_path()
{
	mkdir -p $IEEE8021X_PATH/certificate_file_name
	mkdir -p $IEEE8021X_PATH/tls_ca_certificate
	mkdir -p $IEEE8021X_PATH/tls_client_certificate
	mkdir -p $IEEE8021X_PATH/tls_private_key
	mkdir -p $IEEE8021X_PATH/webconfig
}

remove_sbin_reboot()
{
	rm /sbin/reboot
}


init_time_zone()
{
	echo "time_zone"
	
	if [ -f "$time_setting" ];then
		P3KCFG_TIME_ZONE=`jq -r '.time_setting.time_zone' $time_setting`
	else
		P3KCFG_TIME_ZONE='0'
	fi

	echo "P3KCFG_TIME_ZONE=$P3KCFG_TIME_ZONE"

	if  echo $P3KCFG_TIME_ZONE | grep -q '[-][^0-9]' ; then
	    echo "this is not a num,please input num"
	    P3KCFG_TIME_ZONE='0'
	fi

	if [ $P3KCFG_TIME_ZONE -gt 12 ]; then
		P3KCFG_TIME_ZONE='0'
	fi

	if [ $P3KCFG_TIME_ZONE -lt -12 ]; then
		P3KCFG_TIME_ZONE='0'
	fi

	abs=$P3KCFG_TIME_ZONE;

	if [ $abs -lt 0 ]; then
	  let abs=0-$abs;
	  export TZ=UTC+$abs:00
	  echo export TZ=UTC+$abs:00 >> /root/.profile
	else
	  export TZ=UTC-$abs:00
	  echo export TZ=UTC-$abs:00 >> /root/.profile
	fi

	echo "P3KCFG_TIME_ZONE=$P3KCFG_TIME_ZONE"
	
}

