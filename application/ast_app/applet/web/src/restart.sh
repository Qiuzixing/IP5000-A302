#!/bin/sh

help()
{
	echo "Usage: $0 <process_name>"
		exit 0;
}

# 参数范围检查
if [ "$#" != 1 ]; 
then   
    help
fi

while [ 1 ]; do
	sleep 1
	exit 0;
done
