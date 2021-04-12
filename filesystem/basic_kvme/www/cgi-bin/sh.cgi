#!/bin/sh

# The CGI response header
echo "Content-type: text/html"
echo ""

# The CGI response content below...
echo '<html><body>'
echo '<pre>'

# this sources bashlib into your current environment
. /www/cgi-bin/bashlib

#for p in `param`; do
#	echo "$p:$(param $p)"
#done

#echo '============================='
echo '<div id="sh_stdout">'
dbg=`param dbg`
cmd=`param cmd`
while [ -n "$cmd" ]; do
	
	# Replace "+" with space
	cmd=$(echo ${cmd} | tr "+" "\x20")
	if [ "$dbg" = 'y' ]; then
		echo "cmd:$cmd"
	fi
	eval "$cmd"
	break

done
echo '</div>'
#echo '============================='
echo '</pre>'
echo '</body></html>'
