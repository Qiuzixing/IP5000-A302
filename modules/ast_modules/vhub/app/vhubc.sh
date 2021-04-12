#!/bin/sh

#
# Used tools:
# echo, sleep, sed, cat, insmod
# bind_driver, event_handler
#


#
# Usage: vhubc.sh host_ip
#

#set -x             for debug
. ./util.sh


# Bug. This function will block for a long time if the remote is disconnected.
wait_for_usbaid_alive()
{
	#should use nc instead of ping in the future.
	# ping $1 -c 1 | grep -q " 0%" # for linux
	# ping $1 | grep -q " is alive!"
	msg "Wait for usbaid ${1} available..."
	
	until ./bind_driver --check-host $1 2>/dev/null 1>/dev/null; do
		sleep 2
	done
	
	# check the result of grep
	if $? ; then
		msg usbaid is alive
		return 0
	else
		msg usbaid is not alive
		return 1
	fi
}


clean_up()
{
	./rmvhubc.sh
}

auto_export()
{
	local host_ip=$1
	
	#wait for host alive
	#wait_for_remote_alive $host_ip
	#wait_for_host_ready $host_ip

	# Bruce110407. If ethernet link disconnect while exporting, the bind_driver's
	# tcp socket will blocking and causes e_usbip_user_init_ok never being sent.
	# Since LM handles ethernet link off case, we send e_usbip_user_init_ok
	# earlier before export.
	##A7 removed
	##ast_send_event -1 'e_usbip_user_init_ok'
	#first export-all to host
	msg "export all devices to host..."
	if ./bind_driver --export-all $host_ip ;then
		#ast_send_event -1 'e_usbip_user_init_ok'
		msg "USBIP export OK"
	else
		# Handle the case when ethernet link is disconnect at this moment.
		#ast_send_event -1 'e_usbip_user_init_ok'
		msg "USBIP export Failed"
	fi
	# hot plug is handled in new hot plug framework. Just return here.
	return 0
	# Run the wait_for_hot_plug in the background.
	while ./wait_for_hot_plug; do
		#export to host if there are hot plug events
		wait_for_remote_alive $host_ip
		#wait_for_host_ready $host_ip
		sleep 1
		#wait for host alive./
		msg "auto-exporting"
		./bind_driver --export-all $host_ip
	done

	msg "something wrong in wait_for_hot_plug"
	return 1
}

# BruceToDo. If host is un-reachable, it "unexport" will hang...
handle_driver_event()
{
	local host_ip=$1
	#Wait for driver events
	sed_str='s/host_ip/'${host_ip}'/g' #replace "host_ip" to ${host_ip}
	while ./event_handler > res; do
		cmd=`cat res | sed "${sed_str}"`
		wait_for_remote_alive $host_ip
		sleep 1
		msg ${cmd}
		${cmd}
	done
	return 1
}

#
# Check if remote client alive. If not, detach all vhub ports.
#
host_watchdog()
{
	local host_ip=$1

	#wait for host alive
	wait_for_remote_alive $host_ip

	while wait_for_remote_die $host_ip; do
		msg "lost host $host_ip !!"
		stop_hotplug
		unbind_usbip_all
		wait_for_remote_alive $host_ip
		auto_export $host_ip
		start_hotplug $host_ip
	done
	
	return 1
}

host_watchdog_ori()
{
	local host_ip=$1

	#wait for host alive
	wait_for_remote_alive $host_ip
	wait_for_host_ready $host_ip

	while wait_for_host_die $host_ip; do
		msg "lost host $host_ip !!"
		unbind_usbip_all
		wait_for_remote_alive $host_ip
		wait_for_host_ready $host_ip
	done
	
	return 1
}


#main()
#{
	_host_ip=$1

	if [ -z "${_host_ip}" ]; then
		msg "NO host_ip!!"
		msg "USAGE: vhubc.sh HOST_IP"
		msg "ex: ./vhubc.sh 192.168.0.123"
		exit 1
	fi
	
#	client_is_not_ready
	#clean_up
	#insmod driver modules
	insmod usbip_common_mod.ko
	insmod usbip.ko

	#start export-all in background
	auto_export $_host_ip &
	#record_pid $!

	start_hotplug $_host_ip

	#handle driver events
	#handle_driver_event $host_ip &
	#record_pid $!
	
	#start host watch-dog
#	host_watchdog $host_ip &
#	record_pid $!
	
#	client_is_ready
	exit 0
#}

### Start of script ##########
#main $1



