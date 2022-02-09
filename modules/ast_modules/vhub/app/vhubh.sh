#!/bin/sh

#
# Used tools:
# sed, cat, expr, echo, sleep, insmod, rmmod,
# usbaid
#


. ./util.sh

vhub_driver_path="/sys/devices/platform/vhci_hcd"

clean_up()
{
	./rmvhubh.sh
}

list_used_port()
{
	local list_status
	local _IFS
	local port_idx
	local state
	local each_line
	
	#cat the status and print out from line 2 to the end ($)...
	list_status=`cat /sys/devices/platform/vhci_hcd/status | sed -n '2,$p'`
	if [ -z "$list_status" ]; then
		return 1
	fi
	_IFS=$IFS
	IFS=$ENTER
	
	port_idx='0'
	# for each line in the result of 'list_status', do...
	for each_line in $list_status; do
		# the 'state' is in the second word of status
		state=`get_second_word "${each_line}"`
		#if port state is not equal to VDEV_ST_NULL(004), then...
		if [ "$state" != '004' ]; then
			echo -n "$port_idx "  # echo the port_idx without 'enter'
		fi
		port_idx=`expr $port_idx + 1`
	done
	
	IFS=$_IFS
	return 0
}


#
# Check if remote client alive. If not, rmmod vhub.ko.
# Using rmmod vhub.ko instead of detach port one bye one, get quick response and aviod the "port reset" issue.
#
client_watchdog()
{
	local client_ip=$1
	local used_port_list
	local idx
	
	wait_for_remote_alive $client_ip
	
	while wait_for_remote_die $client_ip; do
		msg "lost client $client_ip !!"
		# set host to not ready
		host_is_not_ready
		rmmod vhub.ko
		sleep 2
		insmod vhub.ko
		sleep 3
		# set host to ready
		host_is_ready
		wait_for_remote_alive $client_ip
	done
}

client_watchdog_ori()
{
	local client_ip=$1
	local used_port_list
	local idx
	
	wait_for_remote_alive $client_ip
	wait_for_client_ready $client_ip
	
	while wait_for_client_die $client_ip; do
		msg "lost client $client_ip !!"
		# set host to not ready
		host_is_not_ready
		rmmod vhub.ko
		sleep 2
		insmod vhub.ko
		sleep 3
		# set host to ready
		host_is_ready
		wait_for_remote_alive $client_ip
		wait_for_client_ready $client_ip
	done
}

#main()
#{
#	local client_ip=$1

#	if [ -z "${client_ip}" ]; then
#		msg "NO client_ip!!"
#		msg "USAGE: vhubc.sh CLIENT_IP"
#		msg "ex: ./vhubc.sh 192.168.0.127"
#		exit 1
#	fi
	# set host to not ready
#	host_is_not_ready

	#clean_up
	#install driver modules
	insmod usbip_common_mod.ko
	insmod vhub.ko
	#sleep 2
	#start usbaid
	./usbaid &
	
	#client_watchdog $client_ip &
	#record_pid $!
	# set host to ready state
#	host_is_ready
	##A7 removed
	##ast_send_event -1 'e_usbip_user_init_ok'
	#return 0
#}

### Start of script ##########
#main $1
