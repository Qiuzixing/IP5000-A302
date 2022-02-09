#!/bin/sh

#TO_CHECK='images/logo.bmp'
#MAX_SIZE='131072'
TO_CHECK="$1"
MAX_SIZE="$2"

fsize=`stat -c %s ${TO_CHECK}`
#set -- `ls -l $TO_CHECK`
#fsize="$5"
#$5 will be the size of the file

if [ "$fsize" -gt "$MAX_SIZE" ]; then
	echo "ERROR!! $TO_CHECK over $MAX_SIZE Bytes"
	exit 1
fi
exit 0