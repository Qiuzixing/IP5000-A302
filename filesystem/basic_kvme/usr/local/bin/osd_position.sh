#!/bin/sh

# Usage: 
# ./osd_position.sh [OSD_POSITION_ALIGN] [OSD_POSITION_X_START_OFFSET] [OSD_POSITION_Y_START_OFFSET]
# For example: top alignment with right shift 100 pixels and down shift 500 pixels.
# osd_position.sh 2 100 500
#
# OSD_POSITION_ALIGN:
#    0: Disable alignment overwrite. Use default behavior.
#    1: Top-Left
#    2: Top
#    3: Top-Right
#    4: Left
#    5: Center
#    6: Right
#    7: Bottom-Left
#    8: Bottom
#    9: Bottom-Right
#
# OSD_POSITION_X_START_OFFSET: X offset in pixels
#
# OSD_POSITION_Y_START_OFFSET: Y offset in pixels
#

case "$#" in
	0)
		OSD_POSITION_ALIGN="0"
		OSD_POSITION_X_START_OFFSET="0"
		OSD_POSITION_Y_START_OFFSET="0"
	;;
	1)
		OSD_POSITION_ALIGN="$1"
		OSD_POSITION_X_START_OFFSET="0"
		OSD_POSITION_Y_START_OFFSET="0"
	;;
	3)
		OSD_POSITION_ALIGN="$1"
		OSD_POSITION_X_START_OFFSET="$2"
		OSD_POSITION_Y_START_OFFSET="$3"
	;;
	*)
		echo "ERR: Unexpected parameters: $*"
		exit
	;;
esac

ast_send_event -1 "e_osd_position::$OSD_POSITION_ALIGN::$OSD_POSITION_X_START_OFFSET::$OSD_POSITION_Y_START_OFFSET"
