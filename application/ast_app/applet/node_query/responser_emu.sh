#!/bin/bash


EMU_NODE_SIZE=200
MAC_PREFIX=0030F1


start_resp()
{
	# $1 MAC
	# $2 Serail Number
	local FILE="$1"
	local MAC=$1
	local SN=$2

	echo "start $FILE $MAC $SN"

	# multi key pair set for 'essential' reply_type
	./node_query --if $FILE --of $FILE --set_key MY_MAC=$MAC
	./node_query --if $FILE --of $FILE --set_key MY_IP=169.254.4.195
	./node_query --if $FILE --of $FILE --set_key IS_HOST=n
	./node_query --if $FILE --of $FILE --set_key HOSTNAME="Im_$MAC"
	./node_query --if $FILE --of $FILE --set_key MULTICAST_ON=y
	./node_query --if $FILE --of $FILE --set_key JUMBO_MTU=8000
	./node_query --if $FILE --of $FILE --set_key CH_SELECT=$SN

	# start node_responser
	./node_responser --mac $MAC &
}

for (( i=1 ; i <= $EMU_NODE_SIZE ; i++ )); do
	MAC="$MAC_PREFIX"`printf '%06d' $i`
	#echo $MAC
	start_resp $MAC $i
done
