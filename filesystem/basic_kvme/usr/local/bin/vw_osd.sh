#!/bin/sh

###########################################################
# Leave following settings as default should be fine for most of cases.
# Customize them carefully
###########################################################
#"30"
VW_OSD_FONT_SIZE="16"
# 32bits RGB color. 0xFFRRGGBB
VW_OSD_FONT_COLOR="0xF000FF00"
# The maximum height of OSD messages. 200 pixel is default good value. Don't change it.
VW_OSD_MAX_Y_SIZE="250"
# From 1 to 15.  15 means no transparent.
VW_OSD_TRANSPARENT="15"


# Root menu select
OSDS_RT_LAYOUT='Layout'
OSDS_RT_ROW='Row Pos.'
OSDS_RT_COLUMN='Column Pos.'
OSDS_RT_APPLY='Apply'
OSDS_RT_CANCEL='Cancel'

OSDS_LAYOUT='Layout_value'
OSDS_ROW='Row_value'
OSDS_COLUMN='Column_value'

VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_LAYOUT"
VWOSD_STATE='off'
VW_VAR_LAYOUT='1x1'
VW_VAR_ROW='0'
VW_VAR_COLUMN='0'
# These max value depends on layout setting
VW_VAR_MAX_ROW='0'
VW_VAR_MAX_COLUMN='0'
VW_VAR_H_SHIFT="0"
VW_VAR_H_SCALE="0"
VW_VAR_V_SCALE="0"
VW_VAR_INSTALL_STATE='off'
VW_VAR_POS_R="0"
VW_VAR_POS_C="0"

vwosd_select_prompt()
{
	if [ "$VWOSD_VAR_CURRENT_SELECT" = "$*" ]; then
		echo -n '>'
	else
		echo -n ' '
	fi	
}

vwosd_create_menu_str()
{
	_ret=""
	for _item in "$OSDS_RT_LAYOUT" "$OSDS_RT_ROW" "$OSDS_RT_COLUMN" "$OSDS_RT_APPLY" "$OSDS_RT_CANCEL"; do
		_s=`vwosd_select_prompt $_item`
		_ret="$_ret$_s$_item"
		
		if [ "$_item" = "$OSDS_RT_LAYOUT" ]; then
			# append the layout value
			_s=`vwosd_select_prompt $OSDS_LAYOUT`
			_ret="$_ret\t\t$_s$VW_VAR_LAYOUT\n"
		elif [ "$_item" = "$OSDS_RT_ROW" ]; then
			# append the row value
			_s=`vwosd_select_prompt $OSDS_ROW`
			_ret="$_ret\t$_s$VW_VAR_ROW\n"
		elif [ "$_item" = "$OSDS_RT_COLUMN" ]; then
			# append the column value
			_s=`vwosd_select_prompt $OSDS_COLUMN`
			_ret="$_ret\t$_s$VW_VAR_COLUMN\n"
		elif [ "$_item" = "$OSDS_RT_APPLY" ]; then
			_ret="$_ret\n"
		elif [ "$_item" = "$OSDS_RT_CANCEL" ]; then
			_ret="$_ret\n"
		else
			_ret="$_ret\n"
		fi
	done
	echo "$_ret"
}

vwosd_redraw_menu_simple()
{
	_menu_str="Column Pos.\n    $VW_VAR_COLUMN"
	ast_send_event -1 "e_osd_on_str::0::$VW_OSD_MAX_Y_SIZE::$VW_OSD_TRANSPARENT::1::n::$_menu_str::$VW_OSD_FONT_SIZE::$VW_OSD_FONT_COLOR"
	return
}

vwosd_redraw_menu()
{
	# ToDo. menu color customization
	_menu_str=`vwosd_create_menu_str`
	ast_send_event -1 "e_osd_on_str::0::$VW_OSD_MAX_Y_SIZE::$VW_OSD_TRANSPARENT::1::n::$_menu_str::$VW_OSD_FONT_SIZE::$VW_OSD_FONT_COLOR"
	return
}

# Bruce151112.
# We re-orginized 'profile folder' in A6.x code. Following code need revised.
# Since it is not used anymore. I simply ____dead_code____ it.
# FIXME. Remove the whole vw_osd.sh file in the future.
____dead_code____vw_enable()
{
	if [ "$IS_HOST" = 'n' ]; then
		echo "$VW_VAR_STRETCH_TYPE" > "$DISPLAY_SYS_PATH"/vw_stretch_type
		echo "$VW_VAR_ROTATE" > "$DISPLAY_SYS_PATH"/vw_rotate
		echo "${VW_VAR_MAX_ROW} ${VW_VAR_MAX_COLUMN} ${VW_VAR_ROW} ${VW_VAR_COLUMN}" > "$DISPLAY_SYS_PATH"/vw_layout
		echo "${VW_VAR_MONINFO_HA} ${VW_VAR_MONINFO_HT} ${VW_VAR_MONINFO_VA} ${VW_VAR_MONINFO_VT}" > "$DISPLAY_SYS_PATH"/vw_frame_comp
		echo "$VW_VAR_H_SHIFT" > "$DISPLAY_SYS_PATH"/vw_h_shift
		echo "$VW_VAR_V_SHIFT" > "$DISPLAY_SYS_PATH"/vw_v_shift
		echo "$VW_VAR_H_SCALE $VW_VAR_V_SCALE" > "$DISPLAY_SYS_PATH"/vw_scale_up
		echo "$VW_VAR_DELAY_KICK" > "$VIDEO_SYS_PATH"/delay_dequeue
	else
		cp -f ./V$SOC_VER/* .
		echo "${VW_VAR_MAX_ROW} ${VW_VAR_MAX_COLUMN} ${VW_VAR_ROW} ${VW_VAR_COLUMN}" > "$VIDEO_SYS_PATH"/vw_layout
		if [ "$VW_VAR_MAX_ROW" != '0' ]; then
			if [ "$VW_VAR_MAX_ROW" -gt '3' ]; then
				PROFILE="video_wall_3"
			else
				PROFILE="video_wall_$VW_VAR_MAX_ROW"
			fi
		else
			_t=`cat $VIDEO_SYS_PATH/rx`
			if [ "$_t" = "digital" ]; then
				echo "select digital source"
				PROFILE='giga'
			elif [ "$_t" = "analog" ]; then
				echo "select analog source"
				PROFILE='analog'
			else
				echo "source undetermined"
			fi
		fi
		echo "profile=$PROFILE"
		cp -f ./V$SOC_VER/profile/"$PROFILE"/* .
		# video_config should wait for the videoip.ko initialized
		#video_config -s V$SOC_VER
		video_config -s V2
	fi
	return

	case "$VW_VAR_LAYOUT" in
		'1x1')
		./vw.sh 0
		;;
		'2x2')
		./vw.sh "${VW_VAR_ROW}_${VW_VAR_COLUMN}"
		;;
		'3x3')
		./vw_3x3.sh "${VW_VAR_ROW}_${VW_VAR_COLUMN}"
		;;
		'4x4')
		./vw_4x4.sh "${VW_VAR_ROW}_${VW_VAR_COLUMN}"
		;;
	esac
}

vwosd_on_select_rt_apply()
{
	# Apply changes
	echo "To Apply all changes"
	____dead_code____vw_enable
	./astparam s vw_column ${VW_VAR_COLUMN}
	./astparam save
	# Turn of OSD menu
	#VWOSD_STATE='off' # is done in osd_off
	osd_off 0
}

vwosd_on_select_rt_cancel()
{
	# Cancel all changes.
	#VWOSD_STATE='off' # is done in osd_off
	osd_off 0
}

vwosd_on_btn_select()
{
	if [ "$VWOSD_STATE" = 'off' ]; then
		# initialize OSD menu and variables
		#./vw.sh 0
		# ToDo
		VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
		# show osd menu
		vwosd_redraw_menu
		VWOSD_STATE='on'
		return
	fi
	# The case under VWOSD_STATE = on
	case "$VWOSD_VAR_CURRENT_SELECT" in
		"$OSDS_RT_LAYOUT")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_LAYOUT"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_ROW")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_ROW"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_COLUMN")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_COLUMN"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_APPLY")
			vwosd_on_select_rt_apply
		;;
		"$OSDS_RT_CANCEL")
			vwosd_on_select_rt_cancel
		;;
		"$OSDS_LAYOUT")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_LAYOUT"
			vwosd_redraw_menu
		;;
		"$OSDS_ROW")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_ROW"
			vwosd_redraw_menu
		;;
		"$OSDS_COLUMN")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
			vwosd_redraw_menu
		;;
		*)
		;;
	esac

}

vwosd_on_pre_layout()
{
	case "$VW_VAR_LAYOUT" in 
		'1x1')
			VW_VAR_LAYOUT='4x4'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='3'
			VW_VAR_MAX_COLUMN='3'
			vwosd_redraw_menu
		;;
		'2x2')
			VW_VAR_LAYOUT='1x1'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='0'
			VW_VAR_MAX_COLUMN='0'
			vwosd_redraw_menu
		;;
		'3x3')
			VW_VAR_LAYOUT='2x2'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='1'
			VW_VAR_MAX_COLUMN='1'
			vwosd_redraw_menu
		;;
		'4x4')
			VW_VAR_LAYOUT='3x3'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='2'
			VW_VAR_MAX_COLUMN='2'
			vwosd_redraw_menu
		;;
	esac
}

vwosd_on_next_layout()
{
	case "$VW_VAR_LAYOUT" in 
		'1x1')
			VW_VAR_LAYOUT='2x2'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='1'
			VW_VAR_MAX_COLUMN='1'
			vwosd_redraw_menu
		;;
		'2x2')
			VW_VAR_LAYOUT='3x3'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='2'
			VW_VAR_MAX_COLUMN='2'
			vwosd_redraw_menu
		;;
		'3x3')
			VW_VAR_LAYOUT='4x4'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='3'
			VW_VAR_MAX_COLUMN='3'
			vwosd_redraw_menu
		;;
		'4x4')
			VW_VAR_LAYOUT='1x1'
			VW_VAR_ROW='0'
			VW_VAR_COLUMN='0'
			VW_VAR_MAX_ROW='1'
			VW_VAR_MAX_COLUMN='1'
			vwosd_redraw_menu
		;;
	esac
}

vwosd_on_pre_row()
{
	if [ "$VW_VAR_ROW" -eq '0' ]; then
		VW_VAR_ROW="$VW_VAR_MAX_ROW"
	else
		VW_VAR_ROW=`expr $VW_VAR_ROW - 1`
	fi
	vwosd_redraw_menu
}

vwosd_on_next_row()
{
	if [ "$VW_VAR_ROW" -eq "$VW_VAR_MAX_ROW" ]; then
		VW_VAR_ROW='0'
	else
		VW_VAR_ROW=`expr $VW_VAR_ROW + 1`
	fi
	vwosd_redraw_menu
}

vwosd_on_pre_column()
{
	if [ "$VW_VAR_COLUMN" -eq '0' ]; then
		VW_VAR_COLUMN="$VW_VAR_MAX_COLUMN"
	else
		VW_VAR_COLUMN=`expr $VW_VAR_COLUMN - 1`
	fi
	vwosd_redraw_menu
}

vwosd_on_next_column()
{
	if [ "$VW_VAR_COLUMN" -eq "$VW_VAR_MAX_COLUMN" ]; then
		VW_VAR_COLUMN='0'
	else
		VW_VAR_COLUMN=`expr $VW_VAR_COLUMN + 1`
	fi
	vwosd_redraw_menu
}

vwosd_on_btn_previous()
{
	case "$VWOSD_VAR_CURRENT_SELECT" in
		"$OSDS_RT_LAYOUT")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_CANCEL"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_ROW")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_LAYOUT"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_COLUMN")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_CANCEL"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_APPLY")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_CANCEL")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_APPLY"
			vwosd_redraw_menu
		;;
		"$OSDS_LAYOUT")
			vwosd_on_pre_layout
		;;
		"$OSDS_ROW")
			vwosd_on_pre_row
		;;
		"$OSDS_COLUMN")
			vwosd_on_pre_column
		;;
		*)
			echo "Unknown select?! $VWOSD_VAR_CURRENT_SELECT"
		;;
	esac
}

vwosd_on_btn_next()
{
	case "$VWOSD_VAR_CURRENT_SELECT" in
		"$OSDS_RT_LAYOUT")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_ROW"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_ROW")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_COLUMN")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_APPLY"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_APPLY")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_CANCEL"
			vwosd_redraw_menu
		;;
		"$OSDS_RT_CANCEL")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
			vwosd_redraw_menu
		;;
		"$OSDS_LAYOUT")
			vwosd_on_next_layout
		;;
		"$OSDS_ROW")
			vwosd_on_next_row
		;;
		"$OSDS_COLUMN")
			vwosd_on_next_column
		;;
		*)
			echo "Unknown select?! $VWOSD_VAR_CURRENT_SELECT"
		;;
	esac
}

vwosd_on_btn_back()
{
	case "$VWOSD_VAR_CURRENT_SELECT" in
		"$OSDS_RT_LAYOUT")
			vwosd_redraw_menu
		;;
		"$OSDS_RT_ROW")
			vwosd_redraw_menu
		;;
		"$OSDS_RT_COLUMN")
			vwosd_redraw_menu
		;;
		"$OSDS_RT_APPLY")
			vwosd_redraw_menu
		;;
		"$OSDS_RT_CANCEL")
			vwosd_redraw_menu
		;;
		"$OSDS_LAYOUT")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_LAYOUT"
			vwosd_redraw_menu
		;;
		"$OSDS_ROW")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_ROW"
			vwosd_redraw_menu
		;;
		"$OSDS_COLUMN")
			VWOSD_VAR_CURRENT_SELECT="$OSDS_RT_COLUMN"
			vwosd_redraw_menu
		;;
		*)
			echo "Unknown select?! $VWOSD_VAR_CURRENT_SELECT"
		;;
	esac
}

_vwosd_font_size()
{
	#timing=[3] 1280x720p@60Hz H+ V+
	_width=`cat $DISPLAY_SYS_PATH/timing_info | grep 'timing='`
	_width=`expr "$_width" : 'timing=.\{1,\} \([[:digit:]]\{1,\}\)x.*'`
	# The number '10' is get from the max string length of OSD string
	_width=`expr \( "$_width" / \( "$VW_VAR_MAX_COLUMN" + 1 \) \) / 10`
	if [ -z "$_width" ]; then
		_width='16'
	fi
	if [ "$_width" -lt '8' ]; then
		_width='8'
	fi
	if [ "$_width" -gt '96' ]; then
		_width='96'
	fi
	VW_OSD_FONT_SIZE="$_width"
	echo "VW_OSD_FONT_SIZE=$VW_OSD_FONT_SIZE"
}

vwosd_on_btn_menu_on()
{
	if [ "$VWOSD_STATE" = 'off' ]; then
		pkill -9 osd_off 2> /dev/null
		_vwosd_font_size
		# initialize OSD menu and variables
		VWOSD_VAR_CURRENT_SELECT="simple_menu"
		# show osd menu
		vwosd_redraw_menu_simple
		VWOSD_STATE='on'
	fi
}

vwosd_on_btn_menu_cancel()
{
	if [ "$VWOSD_STATE" = 'on' ]; then
		# Turn of OSD menu
		#VWOSD_STATE='off' # is done in osd_off
		osd_off 0		
	fi
}

vwosd_on_btn_menu_off()
{
	if [ "$VWOSD_STATE" = 'on' ]; then
		# Apply changes
		echo "To Apply all changes"
		____dead_code____vw_enable
		#VW_VAR_ROW="$MY_ROW_ID"
		VW_VAR_POS_C="$VW_VAR_COLUMN"
		VW_VAR_POS_IDX="r${VW_VAR_POS_R}c${VW_VAR_POS_C}"
		./astparam s vw_row ${VW_VAR_ROW}
		./astparam s vw_column ${VW_VAR_COLUMN}
		./astparam s vw_pos_idx ${VW_VAR_POS_IDX}
		./astparam save
		# Turn of OSD menu
		#VWOSD_STATE='off' # is done in osd_off
		osd_off 0		
	fi
}

vwosd_on_btn_menu()
{
	if [ "$VWOSD_STATE" = 'off' ]; then
		vwosd_on_btn_menu_on
	else
		vwosd_on_btn_menu_off
	fi
}

vwosd_on_btn_menu_recover()
{
	if [ "$VWOSD_STATE" = 'on' ]; then
		VWOSD_STATE='off'
		vwosd_on_btn_menu_on
	fi
}

vwosd_on_btn_toggle()
{
	if [ "$VWOSD_STATE" = 'off' ]; then
		return
	fi

	case "$VWOSD_VAR_CURRENT_SELECT" in
		"simple_menu")
		VW_VAR_COLUMN=`expr \( $VW_VAR_COLUMN + 1 \) % \( $VW_VAR_MAX_COLUMN + 1 \)`
		#if [ "$VW_VAR_COLUMN" -eq "$VW_VAR_MAX_COLUMN" ]; then
		#	VW_VAR_COLUMN='0'
		#else
		#	VW_VAR_COLUMN=`expr $VW_VAR_COLUMN + 1`
		#fi
		if [ -z "$VW_VAR_COLUMN" ]; then
			VW_VAR_COLUMN='0'
		fi
		____dead_code____vw_enable
		vwosd_redraw_menu_simple
		;;
	esac
}

vw_osd()
{
	if [ "$VWOSD_STATE" = 'off' ] && [ "$1" != "btn_select" ] && [ "$1" != "btn_menu" ] && [ "$1" != "btn_menu_on" ]; then
		return
	fi

	if [ "$1" = "btn_select" ]; then
		vwosd_on_btn_select
	elif [ "$1" = "btn_back" ]; then
		vwosd_on_btn_back
	elif [ "$1" = "btn_next" ]; then
		vwosd_on_btn_next
	elif [ "$1" = "btn_previous" ]; then
		vwosd_on_btn_previous
	elif [ "$1" = "btn_menu" ]; then
		vwosd_on_btn_menu
	elif [ "$1" = "btn_menu_on" ]; then
		vwosd_on_btn_menu_on
	elif [ "$1" = "btn_menu_off" ]; then
		vwosd_on_btn_menu_off
	elif [ "$1" = "btn_menu_cancel" ]; then
		vwosd_on_btn_menu_cancel
	elif [ "$1" = "btn_toggle" ]; then
		vwosd_on_btn_toggle
	elif [ "$1" = "btn_menu_recover" ]; then
		vwosd_on_btn_menu_recover
	else
		echo "ERROR! Unknow command:$1"
	fi
}
