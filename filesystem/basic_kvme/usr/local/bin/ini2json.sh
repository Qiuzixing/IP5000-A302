#!/bin/sh
INI=`cat /proc/self/fd/0`

_IFS=$IFS
IFS='
'
echo "{"
for str in $INI; do
	key=`expr "$str" :  '\(.\{1,\}\)=.*'`
	key=`expr "$key" :  '\([^ =]\{1,\}\)'`
	echo -n "\"$key\":"
	value=`expr "$str" :  '[^=]\{1,\}=\(.*\)'`
	echo "\"$value\","
done
echo "\"_result\":\"pass\""
echo "}"
IFS=$_IFS
