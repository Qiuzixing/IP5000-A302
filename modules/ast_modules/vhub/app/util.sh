#!/bin/sh

#
# Used tools:
# echo -n, kill, chmod, ping, grep, sleep, set, ps, expr, kill, shift?, nc, pkill
# bind_driver
#


HOST_RDY_PORT=6001
CLIENT_RDY_PORT=6002

VHUB_HOST_IP='/etc/vhub_host_ip'

ENTER='
'


#
# Some scripts uses STDOUT to return strings back to the caller.
# So, we use msg()  instead of echo to print out messages to STDERR
#
msg()
{
	echo "$*" >&2
}

record_pid()
{
	if [ -z "$#" ]; then
		msg "no input for record?!"
		return 1
	fi
	if [ -z "$1" ]; then
		msg "pid doesn't exist?!"
		return 1
	fi
	msg "reocrd pid: $1 for killing in the future."
	# record the pid to to_kill.sh
	echo "kill $1" >> to_kill.sh
	chmod a+x ./to_kill.sh
	return 0
}

wait_for_remote_alive()
{
	# ping $1 -c 1 | grep -q " 0%" # for linux
	# ping $1 | grep -q " is alive!"
	msg "Wait for remote ${1} available..."
	until ping $1 | grep -q " is alive!" ; do
		sleep 2
	done
	
	# check the result of grep
	if ping $1 | grep -q " is alive!" ; then
		msg $1 is alive
		return 0
	else
		msg $1 is not alive
		return 1
	fi
}

check_if_remote_alive()
{
	# ping $1 -c 1 | grep -q " 0%" # for linux
	# ping $1 | grep -q " is alive!"
	#echo "Check if remote ${1} available..."

	# check the result of grep
	if ping $1 | grep -q " is alive!" ; then
		#echo $1 is alive
		return 0
	else
		#echo $1 is not alive
		if ping $1 | grep -q " is alive!" ; then
			#echo $1 is alive
			return 0
		else
			#echo $1 is not alive
			return 1
		fi
	fi
}

# This function is not reliable, for somtimes, ping failed. DON'T use or modify it.
wait_for_remote_die()
{
	while check_if_remote_alive $1; do
		sleep 1
	done
}

# seperated by '|'
get_first_col()
{
	local _IFS="$IFS"
	#use "space" as seperator
	IFS='|'
	#seperate $1 by "space"
	set -- $1
	#restore IFS
	IFS="$_IFS"
	#return the first word
	echo "$1"
	return 0
}


get_first_word()
{
	local _IFS="$IFS"
	#use "space" as seperator
	IFS=' '
	#seperate $1 by "space"
	set -- $1
	#restore IFS
	IFS="$_IFS"
	#return the first word
	echo "$1"
	return 0
}

get_second_word()
{
	local _IFS="$IFS"
	#use "space" as seperator
	IFS=' '
	#seperate $1 by "space"
	set -- $1
	#restore IFS
	IFS="$_IFS"
	#return the first word
	echo "$2"
	return 0
}


kill_process()
{
	local _IFS
	local list
	local name
	local each
	local pid
	
	name="$1"
	
	# print all process -> remove the 1st line (non-process line) -> try to match the $name
	list=`ps -A | sed -n '2,$p' | grep ${name}`
	if [ -z "$list" ]; then
		return 1
	fi

	# seperate line by line
	_IFS=$IFS
	IFS=$ENTER

	for each in $list; do
		# check if the $each contains "grep", do only it DOESN'T contain 'grep'
		if ! expr "$each" : '.*grep' > /dev/null ; then
			# get the pid
			pid=`get_first_word "${each}"`
			if [ -n "$pid" ]; then
				kill "$pid"
				msg "process $name($pid) killed"
			fi
		fi
	done

	IFS=$_IFS
	return 0
}

list_used_busid()
{
	local _IFS
	local busid_list
	local busid
	
	busid_list=`./bind_driver --scriptlist`
	if [ -z "$busid_list" ]; then
		return 1
	fi
	_IFS=$IFS
	IFS=$ENTER
	for each_line in $busid_list; do
		#Bruce180613. pattern actually looks like "xxx-yyy.zzz.aaa|usbip|"
		# find a pattern like "xxx-yyy|usbip" where xxx and yyy are 1~3 digits
		#expr "$each_line" : '\([[:digit:].]\{1,3\}-[[:digit:].]\{1,3\}\)|usbip' > /dev/null
		if echo "$each_line" | grep '|usbip|' > /dev/null; then # if found then echo the busid
			# get the pattern
			busid=`expr "$each_line" : '\(.*\)|usbip|'`
			echo -n "$busid "
		fi
	done
	IFS=$_IFS
	return 0
}

unbind_usbip_all()
{
	local busid_list
	local idx
	local _IFS

	busid_list=`list_used_busid`
	if [ -z "$busid_list" ]; then
		return 1
	fi

	for idx in ${busid_list}; do
		./bind_driver --other "${idx}"
		sleep 1
	done
	return 0
}


keep_ready()
{
	local port
	local rdy
	port=$1

	while nc -l -p "$port" > /dev/null; do
		nc -l -p "$port" > /dev/null
	done
	msg "FAIL!! nc returns $?"
	return 1
}

im_ready()
{
#	local port
#	port=$1
#
#	# open an port to listen in background
#	keep_ready $port &
#	# record the pid for killing later
#	echo "kill $!" > ./im_not_ready.sh
#	chmod a+x ./im_not_ready.sh
#	return 0

	echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_all
	return 0
}

im_not_ready()
{
#	./im_not_ready.sh
#	pkill nc
#	echo "" > ./im_not_ready.sh
#	return 0

	echo 1 > /proc/sys/net/ipv4/icmp_echo_ignore_all
	return 0
}

wait_for_ready()
{
	local remote_ip
	local port
	remote_ip=$1
	port=$2
	
	# the script will block here until the remote port is opend
	while pscan -p $port -P $port -t 500 $remote_ip | grep "0 open" > /dev/null; do
		sleep 2
	done
	# remote is ready now, return
	return 0
}

wait_for_not_ready()
{
	local remote_ip
	local port
	remote_ip=$1
	port=$2

	# the script will block here until the remote port is opend
	while pscan -p $port -P $port -t 500 $remote_ip | grep "1 open" > /dev/null; do
		sleep 2
	done
	# Check more times to make sure it is not ready.
	while pscan -p $port -P $port -t 500 $remote_ip | grep "1 open" > /dev/null; do
		sleep 2
	done
	while pscan -p $port -P $port -t 500 $remote_ip | grep "1 open" > /dev/null; do
		sleep 2
	done
	# remote is not ready now, return
	return 0
}

ru_ready()
{
	local remote_ip
	local port
	remote_ip=$1
	port=$2
	
	if pscan -p $port -P $port -t 500 $remote_ip | grep "1 open" > /dev/null ; then
		return 0
	fi
	# I do the check twice to make sure it is not ready
	if pscan -p $port -P $port -t 500 $remote_ip | grep "1 open" > /dev/null ; then
		return 0
	fi
	
	return 1
}

host_is_ready()
{
	im_ready $HOST_RDY_PORT
	msg "Set host as ready"
	return 0
}

host_is_not_ready()
{
	im_not_ready
	msg "Set host as NOT ready"
	return 0
}

is_host_ready()
{
	local host_ip
	host_ip=$1
	return ru_ready $host_ip $HOST_RDY_PORT
}

wait_for_host_ready()
{
	local host_ip
	host_ip=$1
	wait_for_ready $host_ip $HOST_RDY_PORT
	msg "Host($host_ip) is ready"
	return 0
}

wait_for_host_die()
{
	local host_ip
	host_ip=$1
	wait_for_not_ready $host_ip $HOST_RDY_PORT
	msg "Host($host_ip) die!"
	return 0
}

client_is_ready()
{
	im_ready $CLIENT_RDY_PORT
	msg "Set client as ready"
	return 0
}

client_is_not_ready()
{
	im_not_ready
	msg "Set client as NOT ready"
	return 0
}

wait_for_client_ready()
{
	local client_ip
	client_ip=$1
	wait_for_ready $client_ip $CLIENT_RDY_PORT
	msg "Client($client_ip) is ready"
	return 0
}

wait_for_client_die()
{
	local client_ip
	client_ip=$1
	wait_for_not_ready $client_ip $CLIENT_RDY_PORT
	msg "Client($client_ip) die!"
	return 0
}

stop_hotplug()
{
	rm "$VHUB_HOST_IP" 2>/dev/null
	return 0
}

start_hotplug()
{
	local host_ip
	host_ip=$1
	echo "$host_ip" > $VHUB_HOST_IP
	return 0
}
