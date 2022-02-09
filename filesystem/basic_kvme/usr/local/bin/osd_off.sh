#!/bin/sh

#osd_off.sh $1 "$OSD_FROM_GUI" "$DISPLAY_SYS_PATH" "$PIPE_INFO_OSD" &

if [ $# = '1' ];then
	_T=$1
	OSD_FROM_GUI='y'
	DISPLAY_SYS_PATH='/sys/devices/platform/display'
	PIPE_INFO_OSD='/var/info_osd'
elif [ $# != '4' ]; then
	echo "ERR: required timeout value"
	exit
else
	_T=$1
	OSD_FROM_GUI=$2
	DISPLAY_SYS_PATH=$3
	PIPE_INFO_OSD=$4
fi

sleep "$_T"
if [ "$OSD_FROM_GUI" = 'y' ]; then
	## Start of legacy code ####################################################
	#echo 0 > "$DISPLAY_SYS_PATH"/osd_from_gui
	#printf ", 30, 0xFF00FF00\0" > "$PIPE_INFO_OSD"
	## End of legacy code ######################################################
	ast_send_event -1 "e_osd_off_str::now"
else
	echo "0" > "$DISPLAY_SYS_PATH"/osd
fi
