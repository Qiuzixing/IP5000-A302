#!/bin/sh

# Bruce120829. Fix bug.
# "bind_driver --unexport-all" will hang forever when ethernet link is off
# We use the old method to avoid this issue.

#stop_hotplug
#rm /etc/vhub_host_ip 2>/dev/null
#if [ -n "$1" ]; then
#	bind_driver --unexport-all "$1"
#fi
#rmmod usbip.ko 2>/dev/null
#rmmod usbip_common_mod.ko 2>/dev/null
#ast_send_event -1 'e_usbip_user_stopped'
#return 0

####################################################
. ./util.sh

#main()
#{
	stop_hotplug
#	client_is_not_ready
	#./to_kill.sh 2>/dev/null
	#kill_process "event_handler" 2>/dev/null
	#kill_process "wait_for_hot_plug" 2>/dev/null
	unbind_usbip_all
	rmmod usbip.ko 2>/dev/null
	rmmod usbip_common_mod.ko 2>/dev/null
	#echo "#!/bin/sh" > to_kill.sh
	#chmod a+x to_kill.sh
	##A7 removed
	##ast_send_event -1 'e_usbip_user_stopped'
#	return 0
#}

### Start of script ##########
#main
