#!/bin/sh

. ./util.sh

#main()
#{
	# set host to not ready
#	host_is_not_ready

	./to_kill.sh 2>/dev/null
	kill_process "usbaid" 2>/dev/null
	rmmod vhub.ko 2>/dev/null
	rmmod usbip_common_mod.ko 2>/dev/null
	echo "#!/bin/sh" > to_kill.sh
	chmod a+x to_kill.sh
	##A7 removed
	##ast_send_event -1 'e_usbip_user_stopped'
#	return 0
#}

### Start of script ##########
#main
