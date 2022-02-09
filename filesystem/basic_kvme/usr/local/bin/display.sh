#!/bin/sh

######  PIPE
PIPE_INFO_LOCAL="/var/info_local"
PIPE_INFO_REMOTE="/var/info_remote"
PIPE_INFO_OSD="/var/info_osd"
PIPE_INFO_GUI_ACTION="/var/info_gui_action"

. ./osd_str.sh
. ./vw_osd.sh

_osd_position()
{
	if [ "$SOC_VER" -ge '3' ]; then
		#_osd_position "$OSDPIC_OSD_POSITION_ALIGN" "$OSDPIC_OSD_POSITION_X_START_OFFSET" "$OSDPIC_OSD_POSITION_Y_START_OFFSET"
		echo "$1" "$2" "$3" > "$DISPLAY_SYS_PATH"/osd_position
	fi
}

osd_position()
{
	#osd_position "$_OSD_POSITION_ALIGN" "$_OSD_POSITION_X_START_OFFSET" "$_OSD_POSITION_Y_START_OFFSET"
	OSDPIC_OSD_POSITION_ALIGN="$1"
	OSDPIC_OSD_POSITION_X_START_OFFSET="$2"
	OSDPIC_OSD_POSITION_Y_START_OFFSET="$3"

	_osd_position "$OSDPIC_OSD_POSITION_ALIGN" "$OSDPIC_OSD_POSITION_X_START_OFFSET" "$OSDPIC_OSD_POSITION_Y_START_OFFSET"
}

_osd_off_pic()
{
	if [ ${HAS_GUI} = 'n' ]; then
		return
	fi

	# Turn off OSD first before change GUI
	echo 0 > "$DISPLAY_SYS_PATH"/osd_from_gui

	if [ "$CURRENT_SCREEN" = '2' ]; then
		# In decode screen
		inform_gui_action "GUI_show_picture::../usr/local/bin/mono.jpg::$GUI_SHOW_TEXT_RUNTIME"

		# if OSD_STR session exists, we don't turn off CRT's OSD. Just remove GUI's pic
		if ! [ -z "$OSD_STR_UUID" ]; then
			echo "1 $OSDSTR_OSD_TRANSPARENT $OSDSTR_OSD_BKGRND_MASK_EN $OSDSTR_OSD_Y_START $OSDSTR_OSD_MAX_Y_SIZE" > "$DISPLAY_SYS_PATH"/osd_from_gui
		fi
	else
		# In GUI screen
		#echo "GUI_show_picture::$GUI_PICTURE_NAME::$GUI_SHOW_TEXT_RUNTIME"
		inform_gui_action "GUI_show_picture::$GUI_PICTURE_NAME::$GUI_SHOW_TEXT_RUNTIME"
	fi

	# Clear OSD_PIC_UUID to indicate current OSD_PIC session is done
	OSD_PIC_UUID=''
}

osd_off_pic()
{
	#e_osd_off_pic::OSD_PIC_UUID

	case "$*" in
		e_osd_off_pic::"$OSD_PIC_UUID")
			# off
			#echo "OSD PIC OFF!!$OSD_PIC_UUID"
			_osd_off_pic
		;;
		e_osd_off_pic::now)
			# force off
			#echo "Force OSD PIC OFF!!"
			_osd_off_pic
		;;
		#*)
		#	# do nothing
		#	echo "aged OSD off"
		#;;
	esac
}

_osd_on_pic()
{
	#osd_on_pic "$_OSD_Y_START" "$_OSD_MAX_Y_SIZE" "$_OSD_TRANSPARENT" "$_OSD_BKGRND_MASK_EN" "$_OFF_TIMER" "$_OSD_PIC_FILE" "$_OSD_PIC_OPTION"
	inform_gui_action "GUI_show_picture::$6::n"
	_osd_position "$OSDPIC_OSD_POSITION_ALIGN" "$OSDPIC_OSD_POSITION_X_START_OFFSET" "$OSDPIC_OSD_POSITION_Y_START_OFFSET"
	echo "1 $3 $4 $1 $2" > "$DISPLAY_SYS_PATH"/osd_from_gui
}

osd_on_pic()
{
	#osd_on_pic "$_OSD_Y_START" "$_OSD_MAX_Y_SIZE" "$_OSD_TRANSPARENT" "$_OSD_BKGRND_MASK_EN" "$_OFF_TIMER" "$_OSD_PIC_FILE" "$_OSD_PIC_OPTION"
	# TODO. Validate input parameters.
	if [ "$#" != '7' ]; then
		echo "ERROR!! wrong osd_on_pic()"
		return
	fi

	local _OFF_TIMER="$5"
	# Create a new OSD_STR session
	OSD_PIC_UUID="PIC$RANDOM"
	OSDPIC_OSD_Y_START="$1"
	OSDPIC_OSD_MAX_Y_SIZE="$2"
	OSDPIC_OSD_TRANSPARENT="$3"
	OSDPIC_OSD_BKGRND_MASK_EN="$4"
	OSDPIC_OSD_OFF_TIMER="$5"
	OSDPIC_OSD_PIC_FILE="$6"
	OSDPIC_OSD_PIC_OPTION="$7"

	# Start OSD off timer
	if [ "$_OFF_TIMER" != 'n' ]; then
		{ sleep $_OFF_TIMER; ast_send_event -1 e_osd_off_pic::$OSD_PIC_UUID; } &
	fi

	if [ "$CURRENT_SCREEN" = '1' ]; then
		# Don't do it under console screen. osd_recover() will take effect.
		return
	fi
	# Run in background in case GUI runs slow
	_osd_on_pic "$@"
}

_osd_off_str()
{
	if [ ${HAS_GUI} = 'n' ]; then
		return
	fi

	# Bruce: Don't run in background. It causes race condition.
	# Bruce160302. Hope new CRT driver resolves the race condition. And we put PIPE access to background to avoid LM hang and enhance LM performance(?).
	{ printf ", 30, 0xFF00FF00\0"; } > "$PIPE_INFO_OSD" &

	# if OSD_PIC session exists, we don't turn off CRT's OSD. Just remove GUI's string.
	if [ -z "$OSD_PIC_UUID" ]; then
		echo 0 > "$DISPLAY_SYS_PATH"/osd_from_gui
	#else
	#	# This line should be redundent. OSD_PIC has higher CRT OSD setting priority.
	#	# So, there should have no overwrite and hence no recover needed.
	#	echo "1 $OSDPIC_OSD_TRANSPARENT $OSDPIC_OSD_BKGRND_MASK_EN $OSDPIC_OSD_Y_START $OSDPIC_OSD_MAX_Y_SIZE" > "$DISPLAY_SYS_PATH"/osd_from_gui
	fi

	# Clear OSD_STR_UUID to indicate current OSD_STR session is done
	OSD_STR_UUID=''
}

osd_off_str()
{
	#e_osd_off_str::OSD_STR_UUID

	case "$*" in
		e_osd_off_str::"$OSD_STR_UUID")
			# off
			#echo "OSD OFF!!$OSD_STR_UUID"
			_osd_off_str
		;;
		e_osd_off_str::now)
			# force off
			#echo "Force OSD OFF!!"
			_osd_off_str
		;;
		#*)
		#	# do nothing
		#	echo "aged OSD off"
		#;;
	esac
}

_osd_on_str()
{
	#osd_on_str "$OSD_Y_START" "$OSD_MAX_Y_SIZE" "$OSD_TRANSPARENT" "$OSD_BKGRND_MASK_EN" "$OFF_TIMER" "$OSD_STR_FILE" "$OSD_FONT_SIZE" "$OSD_FONT_COLOR"
	{ printf "$6, $7, ${8}\0"; } > "$PIPE_INFO_OSD" &

	# Use OSD_PIC session's CRT OSD setting is preferred.
	if [ -z "$OSD_PIC_UUID" ]; then
		_osd_position "0" "0" "0"
		echo "1 $3 $4 $1 $2" > "$DISPLAY_SYS_PATH"/osd_from_gui
	fi
}

osd_on_str()
{
	#osd_on_str "$OSD_Y_START" "$OSD_MAX_Y_SIZE" "$OSD_TRANSPARENT" "$OSD_BKGRND_MASK_EN" "$OFF_TIMER" "$OSD_STR_FILE" "$OSD_FONT_SIZE" "$OSD_FONT_COLOR"
	# TODO. Validate input parameters.
	if [ "$#" != '8' ]; then
		echo "ERROR!! wrong osd_on_str()"
		return
	fi

	local _OFF_TIMER="$5"
	# Create a new OSD_STR session
	OSD_STR_UUID="STR$RANDOM"
	OSDSTR_OSD_Y_START="$1"
	OSDSTR_OSD_MAX_Y_SIZE="$2"
	OSDSTR_OSD_TRANSPARENT="$3"
	OSDSTR_OSD_BKGRND_MASK_EN="$4"
	OSDSTR_OSD_OFF_TIMER="$5"
	OSDSTR_OSD_STR_FILE="$6"
	OSDSTR_OSD_FONT_SIZE="$7"
	OSDSTR_OSD_FONT_COLOR="$8"

	# Start OSD off timer
	if [ "$_OFF_TIMER" != 'n' ]; then
		{ sleep $_OFF_TIMER; ast_send_event -1 e_osd_off_str::$OSD_STR_UUID; } &
	fi

	# Run in background in case GUI runs slow
	_osd_on_str "$@"
}

_osd_recover()
{
	if [ -n "$OSD_PIC_UUID" ]; then
		osd_on_pic  "$OSDPIC_OSD_Y_START" "$OSDPIC_OSD_MAX_Y_SIZE" "$OSDPIC_OSD_TRANSPARENT" "$OSDPIC_OSD_BKGRND_MASK_EN" "$OSDPIC_OSD_OFF_TIMER" "$OSDPIC_OSD_PIC_FILE" "$OSDPIC_OSD_PIC_OPTION"
	fi
	#if [ -n "$OSD_STR_UUID" ]; then
	#	osd_on_str "$OSDSTR_OSD_Y_START" "$OSDSTR_OSD_MAX_Y_SIZE" "$OSDSTR_OSD_TRANSPARENT" "$OSDSTR_OSD_BKGRND_MASK_EN" "$OSDSTR_OSD_OFF_TIMER" "$OSDSTR_OSD_STR_FILE" "$OSDSTR_OSD_FONT_SIZE" "$OSDSTR_OSD_FONT_COLOR"
	#fi
}

osd_recover()
{
	if [ "$SOC_OP_MODE" -lt "3" ]; then
		return
	fi

	if [ -n "$OSD_PIC_UUID" ]; then
		_osd_position "$OSDPIC_OSD_POSITION_ALIGN" "$OSDPIC_OSD_POSITION_X_START_OFFSET" "$OSDPIC_OSD_POSITION_Y_START_OFFSET"
	fi
	# Bruce150901. CRT may run faster then GUI and causes background mask color incorrect.
	# Bruce can't find an easy way to resolve this issue. So, add some more re-try
	# here to workaround this issue.
	_osd_recover
	# Bruce151221. Following retry is buggy because $OSD_PIC_UUID won't reflect the realtime value.
	#              I'm going to rely on CRT driver retry to check GUI's update.
	#{ usleep 50000; _osd_recover; } &
	#{ usleep 100000; _osd_recover; } &
	#{ usleep 500000; _osd_recover; } &
}

osd_off()
{
	VWOSD_STATE='off'
	if [ ${HAS_GUI} = 'n' ]; then
		return
	fi
	pkill -9 osd_off 2> /dev/null
	osd_off.sh $1 "$OSD_FROM_GUI" "$DISPLAY_SYS_PATH" "$PIPE_INFO_OSD" &
	return

	sleep $1
	if [ "$OSD_FROM_GUI" = 'y' ]; then
		echo 0 > "$DISPLAY_SYS_PATH"/osd_from_gui
		{ printf ", 30, 0xFF00FF00\0"; } > "$PIPE_INFO_OSD"
	else
		echo "0" > "$DISPLAY_SYS_PATH"/osd
	fi
}

# This function is only used for LM's own OSD string messages
_osd_on()
{
	#_osd_on osd_msg, osd_font_size, osd_font_color, osd_transparent, osd_max_y_size, off_timer
	if [ "$#" != '6' ]; then
		echo "ERROR!! wrong _osd_on()"
		return
	fi

	#osd_on_str "$OSD_Y_START" "$OSD_MAX_Y_SIZE" "$OSD_TRANSPARENT" "$OSD_BKGRND_MASK_EN" "$OFF_TIMER" "$OSD_STR_FILE" "$OSD_FONT_SIZE" "$OSD_FONT_COLOR"
	osd_on_str "0" "$5" "$4" "1" "$6" "$1" "$2" "$3"
	return

	################ OLD CODE ####################
	#printf "$1, $2, $3\0" > "$PIPE_INFO_OSD"
	# Bruce150716. Instead of direct printf, we use 'cat' to cat a whole file to pipe to avoid Bash printf EOF problem.
	__tmp=/var/osd_$RANDOM
	printf "$1, $2, $3\0" > $__tmp
	cat $__tmp > "$PIPE_INFO_OSD"
	rm -f $__tmp

	echo "1 $4 1 0 $5" > "$DISPLAY_SYS_PATH"/osd_from_gui
}

###########################################################
# Following script is used for further customization.
# Don't uncomment it unless you understand what to do.
###########################################################
_osd_customize()
{
	local _osd_msg
	case "$1" in
		0)
			_osd_msg="$OSD_MSG0"
		;;
		1)
			_osd_msg="$OSD_MSG1"
		;;
		2)
			_osd_msg="$OSD_MSG2"
		;;
		3)
			_osd_msg="$OSD_MSG3"
		;;
		4)
			_osd_msg="$OSD_MSG4"
		;;
		5)
			_osd_msg="$OSD_MSG5"
		;;
		6)
			_osd_msg="$OSD_MSG6"
		;;
		7)
			_osd_msg="$OSD_MSG7"
		;;
		8)
			_osd_msg="$OSD_MSG8"
		;;
		9)
			_osd_msg="$OSD_MSG9"
		;;
		10)
			_osd_msg="$OSD_MSG10"
		;;
		11)
			_osd_msg="$OSD_MSG11"
		;;
		*)
			_osd_msg=" "
		;;
	esac

	#echo "_osd_customize $_osd_msg '$2'"
	_osd_on "$_osd_msg" "$OSD_FONT_SIZE" "$OSD_FONT_COLOR" "$OSD_TRANSPARENT" "$OSD_MAX_Y_SIZE" "$2"
}

osd_on()
{
	if [ "$IS_HOST" = 'n' ]; then
		# Client
		if [ ${HAS_GUI} = 'n' ]; then
			return
		fi
		case `cat "$DISPLAY_SYS_PATH"/screen` in
			*'message screen'* | *'no screen'*)
				# We are not going to show OSDs under non-decode screen
				return
			;;
			*)
			;;
		esac
		if [ "$OSD_FROM_GUI" = 'y' ]; then
			_osd_customize "$1" "$2"
		else
			echo "1 15 $1" > "$DISPLAY_SYS_PATH"/osd
		fi
	else
		# Host need to broadcast msg to clients
		#ast_send_event -1 "msg_osd_on_$1"
		#ipc @hb_ctrl s msg:v:pinger:255.255.255.255:ve_notify_lm_osd:$1
		# A7 TBD. Different service has different msg_channel.
		echo "osd_on() TBD"
	fi
}

_ch_hex_to_dec()
{
	# Split 4 digit $ch one by one to calculate its dec value.
	local ch0=${CH_SELECT:0:1}
	local ch1=${CH_SELECT:1:1}
	local ch2=${CH_SELECT:2:1}
	local ch3=${CH_SELECT:3:1}

	# convert $ch to dec value (base 10)
	local ch=$(( $ch0 * 1000 + $ch1 * 100 + $ch2 * 10 + $ch3 ))
	# convert $ch to dec value (base 2)
	#local ch=$(( $ch0 * 8 + $ch1 * 4 + $ch2 * 2 + $ch3 + 1))
	printf "%d" $ch
}

osd_set_to()
{
	case "$1" in
		VIDEO_INITIALIZED)
			return
		;;
		VIDEO_CLIENT_READ_EDID)
			return
		;;
		VIDEO_CLIENT_WAIT_HOST_INFO)
			return
		;;
		VIDEO_START_DECODE)
			if [ "$SHOW_CH_OSD" = 'y' ]; then
				( # Use subshell, (), to export OSD_OFF_TIMER
					export OSD_OFF_TIMER='3'
					osd_on_tl.sh "\n    "`_ch_hex_to_dec`
				)
			fi
			return
		;;
		VIDEO_GRAPHIC_MODE)
			osd_on 0 $2
		;;
		VIDEO_VIDEO_MODE)
			osd_on 1 $2
		;;
		VIDEO_ANTIDITHER_OFF)
			osd_on 2 $2
		;;
		VIDEO_ANTIDITHER_1)
			osd_on 3 $2
		;;
		VIDEO_ANTIDITHER_2)
			osd_on 4 $2
		;;
		VIDEO_UNSUPPORTED_MODE)
			osd_on 5 $2
		;;
		VIDEO_MODE_CHANGED)
			osd_on 6 $2
		;;
		REQUEST_USB)
			osd_on 7 $2
		;;
		START_USB)
			osd_on 8 $2
		;;
		STOP_USB)
			osd_on 9 $2
		;;
		VIDEO_UNSUPPORTED_REFRESH_RATE)
			osd_on 10 $2
		;;
		VIDEO_HDCP_FAIL)
			osd_on 11 $2
		;;
		OSD_OFF)
			_osd_off_str
			return
		;;
		*)
		;;
	esac

}


inform_gui()
{
	if [ "$HAS_GUI" = 'y' ]; then
		# Bruce171018.
		# gui_action may be called several times in a single handler.
		# for example: switch_to_decode_screen() clears osd_pic then recover osd_pic.
		# so, we should wait in order to keep code flow in sequence.

		#echo "$1" > "$PIPE_INFO_LOCAL"
		{ printf "%s\0" "$1"; } > "$PIPE_INFO_LOCAL" &
		wait ${!}
	fi
}

inform_gui_echo()
{
	inform_gui "$1"
	echo "$1"
}

gui_action()
{
	if [ "$HAS_GUI" = 'y' ]; then
		# Bruce171018.
		# gui_action may be called several times in a single handler.
		# for example: switch_to_decode_screen() clears osd_pic then recover osd_pic.
		# so, we should wait in order to keep code flow in sequence.
		{ printf "$1\0"; } > "$PIPE_INFO_GUI_ACTION" &
		wait ${!}
	fi
}

inform_gui_ui_feature_action()
{
	if [ "$UI_FEATURE" = 'y' ]; then
		gui_action "$1"
	fi
}

inform_gui_action()
{
	gui_action "$1"
}

switch_to_GUI_screen()
{
	local _show="$1"
	# do not check HAS_CRT
	# if [ "$HAS_CRT" = 'n' ]; then
	#	return
	# fi

	# Switch to GUI(console) screen
	# Bruce121214. Should always fire the switch command in case of power saving.
	#if ! { cat "$DISPLAY_SYS_PATH"/screen | grep -q 'message screen'; }; then
	echo "Switch to GUI screen"
	osd_off 0
	echo 1 > "$DISPLAY_SYS_PATH"/screen
	#fi

	# Bruce171122. ACCESS_ON is now can't represent the state of showing or hiding dialog.
	# Check do_e_stop_link(), where do_stop_srv_ex()::switch_to_GUI_screen() is called
	# before toggle ACCESS_ON to 'n'. (Due to free routing, we only toggle ACCESS_ON when all services are off.)
	# So, when switch_to_GUI_screen() is called, ACCESS_ON is still 'y' but we
	# want to show the dialog at that stage.
	# Solution:
	# Whoever calling switch_to_GUI_screen() is now responsible to decide showing or hiding the dialog.
	# Note: We treat system boot up procedure as special case and handle it manually.
	#if [ "$ACCESS_ON" = 'n' ]; then
	if [ "$_show" = 'show_dialog' ]; then
		echo "GUI_show_dialog"
		inform_gui_ui_feature_action "GUI_show_dialog"
	else
		echo "GUI_hide_dialog"
		inform_gui_ui_feature_action "GUI_hide_dialog"
	fi

	inform_gui_action "GUI_show_picture::$GUI_PICTURE_NAME::$GUI_SHOW_TEXT_RUNTIME"
	CURRENT_SCREEN='1'

	if [ "$GUI_PICTURE_DAEMON" != '' ]; then
		echo "Restart the scritp : $GUI_PICTURE_DAEMON"
		$GUI_PICTURE_DAEMON &
	fi

	if [ "$(( $HDCP_CTS_OPTION & 0x8 ))" -eq '8' ]; then
		echo "Start HDCP 2.2 Testing Mode"
		# Overwrite ACCESS_ON so that test mode won't auto link on boot up.
		ACCESS_ON='n'
		# According to Jerry, enabling dual port causes HDCP 2.2 CTS fail.
		# There is no root cause found yet. It is something related to CRT driver's
		# crt_get_display_edid() behavior. The quick fix is disabling dual output.
		echo 0 > "$DISPLAY_SYS_PATH"/dual_port
		# Enable 1920x1080 60Hz, HDMI, HDCP 2.2, Color Bar
		(
			# sleep 2 in order to avoid GUI garbage.
			sleep 2
			echo 1920 1080 60 1 2 6 > "$DISPLAY_SYS_PATH"/test_mode
		) &
	fi
}

_switch_to_decode_screen()
{
	if [ "$GUI_PICTURE_DAEMON" != '' ]; then
		echo "Kill the script : $GUI_PICTURE_DAEMON"
		ps -A | grep $GUI_PICTURE_DAEMON | kill `awk '{print $1}'`
	fi

	inform_gui_action "GUI_show_picture::../usr/local/bin/mono.jpg::$GUI_SHOW_TEXT_RUNTIME"

	osd_recover
}

switch_to_decode_screen()
{
	# do not check HAS_CRT
	# if [ "$HAS_CRT" = 'n' ]; then
	#	return
	# fi

	CURRENT_SCREEN='2'
	echo 2 > "$DISPLAY_SYS_PATH"/screen

	_switch_to_decode_screen &
}

switch_console_screen()
{
	# do not check HAS_CRT
	# if [ "$HAS_CRT" = 'n' ]; then
	#	return
	# fi

	SCREEN=`cat $DISPLAY_SYS_PATH/screen`
	if [ "$SCREEN" = '[2] decode screen' ]; then
		switch_to_GUI_screen show_dialog
		echo 1 > '/sys/devices/platform/ftgmac/net_debug'
	else
		echo 2 > "$DISPLAY_SYS_PATH"/screen
		echo 0 > '/sys/devices/platform/ftgmac/net_debug'
	fi
}

display_power_save()
{
	echo $1 > "$DISPLAY_SYS_PATH"/power_save
}

display_logo()
{
	if [ -f "$DISPLAY_SYS_PATH"/logo ]; then
		echo $1 > $DISPLAY_SYS_PATH/logo
	fi
}

display_screen_force()
{
	echo $1 > "$DISPLAY_SYS_PATH"/screen
}
##############################################################################

vw_enable_v2()
{
	echo "$VW_VAR_ROTATE" > "$DISPLAY_SYS_PATH"/vw_rotate
	echo "${VW_VAR_V2_X1} ${VW_VAR_V2_Y1} ${VW_VAR_V2_X2} ${VW_VAR_V2_Y2}" > "$DISPLAY_SYS_PATH"/vw_layout_v2
}

vw_enable()
{
	if [ "$VW_VAR_VER" = '2' ]; then
		vw_enable_v2
		return
	fi

	if [ "$IS_HOST" = 'n' ]; then
		if [ "$SOC_OP_MODE" -ge "3" ]; then
			echo "$VW_VAR_STRETCH_TYPE" > "$DISPLAY_SYS_PATH"/vw_stretch_type
			echo "$VW_VAR_ROTATE" > "$DISPLAY_SYS_PATH"/vw_rotate
		fi
		echo "${VW_VAR_MAX_ROW} ${VW_VAR_MAX_COLUMN} ${VW_VAR_ROW} ${VW_VAR_COLUMN}" > "$DISPLAY_SYS_PATH"/vw_layout
		echo "${VW_VAR_MONINFO_HA} ${VW_VAR_MONINFO_HT} ${VW_VAR_MONINFO_VA} ${VW_VAR_MONINFO_VT}" > "$DISPLAY_SYS_PATH"/vw_frame_comp
		echo "$VW_VAR_H_SHIFT" > "$DISPLAY_SYS_PATH"/vw_h_shift
		echo "$VW_VAR_V_SHIFT" > "$DISPLAY_SYS_PATH"/vw_v_shift
		echo "$VW_VAR_H_SCALE $VW_VAR_V_SCALE" > "$DISPLAY_SYS_PATH"/vw_scale_up
		set_video_dequeue_delay $VW_VAR_DELAY_KICK
	elif [ "$SOC_OP_MODE" = '1' ]; then
		# TBD we'll not support 1500/1510
		# Only apply to host and SoC op mode == 1
		echo "$VW_VAR_V_SHIFT" > "$VIDEO_SYS_PATH"/vw_v_shift
		echo "$VW_VAR_MONINFO_HA $VW_VAR_MONINFO_HT $VW_VAR_MONINFO_VA $VW_VAR_MONINFO_VT" > "$VIDEO_SYS_PATH"/vw_frame_comp
		echo "${VW_VAR_MAX_ROW} ${VW_VAR_MAX_COLUMN} ${VW_VAR_ROW} ${VW_VAR_COLUMN}" > "$VIDEO_SYS_PATH"/vw_layout
		apply_profile_config `select_v_input refresh`
	fi
}

vw_update_moninfo()
{
	echo "$VW_VAR_MONINFO_HA $VW_VAR_MONINFO_HT $VW_VAR_MONINFO_VA $VW_VAR_MONINFO_VT" > "$DISPLAY_SYS_PATH"/vw_frame_comp
}

vw_update_v_shift()
{
	echo "$VW_VAR_V_SHIFT" > "$DISPLAY_SYS_PATH"/vw_v_shift
}

vw_update_h_shift()
{
	echo "$VW_VAR_H_SHIFT" > "$DISPLAY_SYS_PATH"/vw_h_shift
}

vw_update_scale_up()
{
	echo "$VW_VAR_H_SCALE $VW_VAR_V_SCALE" > "$DISPLAY_SYS_PATH"/vw_scale_up
}

vw_update_stretch_type()
{
	echo "$VW_VAR_STRETCH_TYPE" > "$DISPLAY_SYS_PATH"/vw_stretch_type
}

vw_update_rotate()
{
	echo "$VW_VAR_ROTATE" > "$DISPLAY_SYS_PATH"/vw_rotate
}
