#!/bin/sh

# Usage: 
# Use 'export' command to setup environment variables and call this script.
# ./osd_pic_on.sh [OSD_PIC_FILE] [OSD_OFF_TIMER] [OSD_BKGRND_MASK_EN] [OSD_TRANSPARENT]
# For example:
# (export OSD_PIC_OPTION="0"; osd_pic_on.sh test.jpg)
###########################################################
# Leave following settings as default should be fine for most of cases.
# Customize them carefully
###########################################################
# Extra options.
#OSD_PIC_OPTION="0"
# The vertical start of OSD source image. (GUI's range in pixels. It is hard coded as 640x480) 0 ~ 480.
#OSD_Y_START="0"
# The maximum height of OSD source image. TBD
#OSD_MAX_Y_SIZE="9999"
# From 1 to 31.  31 means no transparent.
#OSD_TRANSPARENT="31"
# Will use first pixel as background color and make it transparent. 0 or 1
#OSD_BKGRND_MASK_EN="1"
# Turn off OSD after OSD_OFF_TIMER seconds. 'n' means never turn off.
#OSD_OFF_TIMER="n"

MY_OSD_PIC_FILE=${OSD_PIC_FILE:-'default.jpg'}
MY_OSD_PIC_OPTION=${OSD_PIC_OPTION:-'0'}
MY_OSD_Y_START=${OSD_Y_START:-'0'}
MY_OSD_MAX_Y_SIZE=${OSD_MAX_Y_SIZE:-'9999'}
MY_OSD_TRANSPARENT=${OSD_TRANSPARENT:-'31'}
MY_OSD_BKGRND_MASK_EN=${OSD_BKGRND_MASK_EN:-'1'}
MY_OSD_OFF_TIMER=${OSD_OFF_TIMER:-'n'}

case "$#" in
	1)
		MY_OSD_PIC_FILE="$1"
	;;
	2)
		MY_OSD_PIC_FILE="$1"
		MY_OSD_OFF_TIMER="$2"
	;;
	3)
		MY_OSD_PIC_FILE="$1"
		MY_OSD_OFF_TIMER="$2"
		MY_OSD_BKGRND_MASK_EN="$3"
	;;
	4)
		MY_OSD_PIC_FILE="$1"
		MY_OSD_OFF_TIMER="$2"
		MY_OSD_BKGRND_MASK_EN="$3"
		MY_OSD_TRANSPARENT="$4"
	;;
	*)
		echo "ERR: Unexpected parameters: $*"
		exit
	;;
esac

#echo "MY_OSD_PIC_FILE=$MY_OSD_PIC_FILE"
#echo "MY_OSD_PIC_OPTION=$MY_OSD_PIC_OPTION"
#echo "MY_OSD_Y_START=$MY_OSD_Y_START"
#echo "MY_OSD_MAX_Y_SIZE=$MY_OSD_MAX_Y_SIZE"
#echo "MY_OSD_TRANSPARENT=$MY_OSD_TRANSPARENT"
#echo "MY_OSD_BKGRND_MASK_EN=$MY_OSD_BKGRND_MASK_EN"
#echo "MY_OSD_OFF_TIMER=$MY_OSD_OFF_TIMER"

# Bruce150831. It is important to add " to event message, in case there are 'space' characters in $MY_OSD_PIC_FILE
ast_send_event -1 "e_osd_on_pic::$MY_OSD_Y_START::$MY_OSD_MAX_Y_SIZE::$MY_OSD_TRANSPARENT::$MY_OSD_BKGRND_MASK_EN::$MY_OSD_OFF_TIMER::$MY_OSD_PIC_FILE::$MY_OSD_PIC_OPTION"
