#!/bin/sh

# The CGI response header
echo "Content-type: text/html"
echo ""

# The CGI response content below...
echo '<html><body>'
echo '<pre>'

# this sources bashlib into your current environment
#. /www/cgi-bin/bashlib

#echo '============================='
echo '<div id="e_stdout">'

if [ -n "${QUERY_STRING}" ]; then 
	#
	# Bruce110520. Important!! the $event MUST be the first key.
	#              Link Manager can't handle the case which $event is not the first key.
	#              We just ignore the other keys which is not the first key.
	# Replace "&" with "\&"
	#QUERY_STRING=$(echo "${QUERY_STRING}" | sed 's/&/\\&/g')
	# Replace ";" with "\;"
	#QUERY_STRING=$(echo "${QUERY_STRING}" | sed 's/;/\\;/g')

	eval "e ${QUERY_STRING}"
fi

echo '</div>'
#echo '============================='
echo '</pre>'
echo '</body></html>'
