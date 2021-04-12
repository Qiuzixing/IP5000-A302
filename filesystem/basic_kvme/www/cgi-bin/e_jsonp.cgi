#!/bin/sh

LOG='/dev/shm/query.log'

callback=`expr "${QUERY_STRING}" : 'callback=\(.\{1,\}\)&e_sh_.*'`
event=`expr "${QUERY_STRING}" : 'callback=.\{1,\}&\(e_sh_.*\)'`

# The CGI response header
echo "Content-type: application/json"
echo ""

#echo '============================='

if [ -n "$callback" ]; then
	echo "$callback ("
fi

if [ -n "${event}" ]; then 
	#
	# Bruce110520. Important!! the $event MUST be the first key.
	#              Link Manager can't handle the case which $event is not the first key.
	#              We just ignore the other keys which is not the first key.
	# Replace "&" with "\&"
	#QUERY_STRING=$(echo "${QUERY_STRING}" | sed 's/&/\\&/g')
	# Replace ";" with "\;"
	#QUERY_STRING=$(echo "${QUERY_STRING}" | sed 's/;/\\;/g')

	eval "e ${event}"
fi

if [ -n "$callback" ]; then
	echo ");"
fi
