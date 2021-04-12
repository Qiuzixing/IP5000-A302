#!/bin/sh

# Usage:
# Setup environment variables and call this script.
# ./osd_on_tl.sh
###########################################################
# Leave following settings as default should be fine for most of cases.
# Customize them carefully
###########################################################
#OSD_FONT_SIZE="30"
# 32bits RGB color. 0xFFRRGGBB
#OSD_FONT_COLOR="0xFF00FF00"
# The maximum height of OSD messages. 200 pixel is default good value. Don't change it.
#OSD_MAX_Y_SIZE="200"
# From 1 to 31.  31 means no transparent. SoC <= V2 supports only up to 15.
#OSD_TRANSPARENT="15"

DISPLAY_SYS_PATH='/sys/devices/platform/display'

MY_OSD_STR=${OSD_STR:-' '}
MY_OSD_FONT_SIZE=${OSD_FONT_SIZE:-'16'}
MY_OSD_FONT_COLOR=${OSD_FONT_COLOR:-'0xF000FF00'}
MY_OSD_TRANSPARENT=${OSD_TRANSPARENT:-'31'}
MY_OSD_MAX_Y_SIZE=${OSD_MAX_Y_SIZE:-'250'}
MY_OSD_OFF_TIMER=${OSD_OFF_TIMER:-'n'}

if [ "$#" -gt 0 ]; then
	MY_OSD_STR="$*"
fi

#echo "MY_OSD_STR=$MY_OSD_STR"
#echo "MY_OSD_FONT_SIZE=$MY_OSD_FONT_SIZE"
#echo "MY_OSD_FONT_COLOR=$MY_OSD_FONT_COLOR"
#echo "MY_OSD_TRANSPARENT=$MY_OSD_TRANSPARENT"
#echo "MY_OSD_MAX_Y_SIZE=$MY_OSD_MAX_Y_SIZE"

_calc_font_size()
{
	#timing=[3] 1280x720p@60Hz H+ V+
	_width=`cat $DISPLAY_SYS_PATH/timing_info | grep 'timing='`
	_width=`expr "$_width" : 'timing=.\{1,\} \([[:digit:]]\{1,\}\)x.*'`
	# The number '10' is get from the max string length of OSD string
	#_width=`expr \( "$_width" / \( "$VW_VAR_MAX_COLUMN" + 1 \) \) / 10`
	_width=`expr \( "$_width" / \( 0 + 1 \) \) / 10`
	if [ -z "$_width" ]; then
		_width='16'
	fi
	if [ "$_width" -lt '8' ]; then
		_width='8'
	fi
	if [ "$_width" -gt '96' ]; then
		_width='96'
	fi
	MY_OSD_FONT_SIZE="$_width"
	echo "MY_OSD_FONT_SIZE=$MY_OSD_FONT_SIZE"
}

_calc_font_size

# Bruce150831. It is important to add " to event message, in case there are 'space' characters in $MY_OSD_STR
ast_send_event -1 "e_osd_on_str::-1::$MY_OSD_MAX_Y_SIZE::$MY_OSD_TRANSPARENT::1::$MY_OSD_OFF_TIMER::$MY_OSD_STR::$MY_OSD_FONT_SIZE::$MY_OSD_FONT_COLOR"

### Start of Legacy code #######################################################
legacy_code()
{
	PIPE_INFO_OSD='/var/info_osd'

	#printf "$MY_OSD_STR, $MY_OSD_FONT_SIZE, ${MY_OSD_FONT_COLOR}\0" > "$PIPE_INFO_OSD"
	# Bruce150716. Instead of direct printf, we use 'cat' to cat a whole file to pipe to avoid Bash printf EOF problem.
	__tmp=/var/osd_$RANDOM
	printf "$MY_OSD_STR, $MY_OSD_FONT_SIZE, ${MY_OSD_FONT_COLOR}\0" > $__tmp
	cat $__tmp > "$PIPE_INFO_OSD"
	rm -f $__tmp

	echo "1 $MY_OSD_TRANSPARENT 1 -1 $MY_OSD_MAX_Y_SIZE" > "$DISPLAY_SYS_PATH"/osd_from_gui
}
### End of Legacy code #######################################################
