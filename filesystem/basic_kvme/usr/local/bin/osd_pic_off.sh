#!/bin/sh

#osd_pic_off.sh $1 &

if [ "$#" = '1' ]; then
	_T=$1
else
	echo "ERR: required timeout value"
	exit
fi

sleep "$_T"
ast_send_event -1 "e_osd_off_pic::now"
