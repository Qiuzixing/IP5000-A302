#!/bin/sh

SHARE_USB_STATE='s_idle'
SHARE_USBIP_RDY='n'
SHARE_USBIP_USER_RDY='n'
USB_ACCESS_ON='n'

usb_is_all_started()
{
	if [ "$SHARE_USBIP_RDY" = 'y' ] && [ "$SHARE_USBIP_USER_RDY" = 'y' ]; then
		return 0
	fi
	return 1
}

usb_is_all_stopped()
{
	if [ "$SHARE_USBIP_RDY" = 'n' ] && [ "$SHARE_USBIP_USER_RDY" = 'n' ]; then
		return 0
	fi
	return 1
}

usb_handle_e_usb_all_init_ok()
{
	SHARE_USB_STATE='s_srv_on'
}

usb_handle_e_usbip_init_ok()
{
	SHARE_USBIP_RDY='y'
	if [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
		echo "USBIP init OK!"
		if usb_is_all_started ; then 
			ast_send_event "$EM_PID" 'e_usb_all_init_ok'
		fi
	else
		warn "Wrong state?!"
	fi
}

usb_handle_e_usbip_user_init_ok()
{
	SHARE_USBIP_USER_RDY='y'
	if [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
		echo "USBIP_USER init OK!"
		if usb_is_all_started ; then 
			ast_send_event "$EM_PID" 'e_usb_all_init_ok'
		fi
	else
		warn "Wrong state?!"
	fi
}

usb_to_s_search()
{
	SHARE_USB_STATE='s_search'
	# unset all required event to be checked.
	SHARE_USBIP_RDY='n'
	SHARE_USBIP_USER_RDY='n'

	# Start and Wait for vhub_heartbeat
	vhub_heartbeat -c -d "$GWIP" &
}

usb_to_s_start_srv()
{
	SHARE_USB_STATE='s_start_srv'
	vhubc.sh $GWIP &
}

usb_to_s_idle()
{
	SHARE_USB_STATE='s_idle'

	if [ "$USB_ACCESS_ON" = 'y' ]; then
		usb_to_s_search
	fi
}

usb_to_s_stop_srv()
{
	SHARE_USB_STATE='s_stop_srv'
	
	if usb_is_all_stopped ; then
		echo "usb already all stopped"
		usb_to_s_idle
		return 0
	fi

	pkill -9 vhub_heartbeat 2>/dev/null
	rmvhubc.sh "$GWIP" &
}

usb_handle_e_usbip_stopped()
{
	SHARE_USBIP_RDY='n'
	if [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
		echo "USBIP stopped!"
		if usb_is_all_stopped ; then 
			ast_send_event "$EM_PID" 'e_usb_all_stopped'
		fi
	elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
		echo "ERROR!! USBIP goes wrong?! Stop the link"
		usb_to_s_stop_srv
	else
		warn "Wrong state?!"
	fi
}

usb_handle_e_usbip_user_stopped()
{
	SHARE_USBIP_USER_RDY='n'
	if [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
		echo "USBIP_USER stopped!"
		if usb_is_all_stopped ; then 
			ast_send_event "$EM_PID" 'e_usb_all_stopped'
		fi
	elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
		echo "ERROR!! USBIP_USER goes wrong?! Stop the link"
		usb_to_s_stop_srv
	else
		warn "Wrong state?!"
	fi
}

usb_handle_e_all_stopped()
{
	echo "All USB services are stopped"
	usb_to_s_idle
}

usb_handle_e_vhub_heartbeat_init_ok()
{
	if [ "$SHARE_USB_STATE" = 's_search' ]; then
		echo "Found VHUB($GWIP), start link.."
		usb_to_s_start_srv
	else
		warn "Wrong state?!"
	fi

}

usb_link_on_off()
{
	# share usb only available under s_srv_on state.
	if [ "$STATE" = 's_srv_on' ]; then
		if [ "$SHARE_USB_STATE" = 's_idle' ]; then
			echo "Start usb link"
			USB_ACCESS_ON='y'
			usb_to_s_search
			#usb_to_s_start_srv
		elif [ "$SHARE_USB_STATE" = 's_search' ]; then
			echo "Stop usb link"
			USB_ACCESS_ON='n'
			usb_to_s_stop_srv
		elif [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
			echo "Try to stop USB on USB::s_start_srv state! Re-stop latter"
			{ sleep 1 && ast_send_event -1 e_stop_usb; } &
		elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
			echo "Stop usb link"
			USB_ACCESS_ON='n'
			usb_to_s_stop_srv
		elif [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
			USB_ACCESS_ON='n'
		else
			warn "unknonw usb state"
		fi	
	else
		USB_ACCESS_ON='n'
		warn "Wrong state?!"
	fi
}

usb_link_on()
{
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		led_on $LED_PWR
	fi
	
	if [ "$SHARE_USB_STATE" = 's_idle' ]; then
		#echo "Start usb link"
		osd_set_to 'START_USB' 5
		USB_ACCESS_ON='y'
		usb_to_s_search
	elif [ "$SHARE_USB_STATE" = 's_search' ]; then
		osd_set_to 'START_USB' 5
		# do nothing
	elif [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
		osd_set_to 'START_USB' 5
		# do nothing
	elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
		osd_set_to 'START_USB' 5
		# already on. do nothing
	elif [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
		osd_set_to 'START_USB' 5
		# set USB_ACCESS_ON='y' so that state machine will restart after s_idle
		USB_ACCESS_ON='y'
	else
		warn "unknonw usb state"
	fi
}

usb_link_off()
{
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		led_off $LED_PWR
	fi

	if [ "$SHARE_USB_STATE" = 's_idle' ]; then
		# do nothing
		return
	elif [ "$SHARE_USB_STATE" = 's_search' ]; then
		#echo "Stop usb link"
		osd_set_to 'STOP_USB' 5
		USB_ACCESS_ON='n'
		usb_to_s_stop_srv
	elif [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
		# stop later
		echo "Try to stop USB on USB::s_start_srv state! Re-stop latter"
		{ sleep 1 && ast_send_event -1 e_stop_usb; } &
	elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
		#echo "Stop usb link"
		osd_set_to 'STOP_USB' 5
		USB_ACCESS_ON='n'
		usb_to_s_stop_srv
	elif [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
		osd_set_to 'STOP_USB' 5
		USB_ACCESS_ON='n'
	else
		warn "unknonw usb state"
	fi	
}

usb_request_on_off()
{
	# share usb only available under s_srv_on state.
	if [ "$STATE" = 's_srv_on' ]; then
		if [ "$SHARE_USB_STATE" = 's_idle' ]; then
			ast_send_event -1 "msg_e_request_usb"
			#echo "Request usb link"
			osd_set_to 'REQUEST_USB' 5
		elif [ "$SHARE_USB_STATE" = 's_search' ]; then
			usb_link_off
		elif [ "$SHARE_USB_STATE" = 's_start_srv' ]; then
			usb_link_off
		elif [ "$SHARE_USB_STATE" = 's_srv_on' ]; then
			usb_link_off
		elif [ "$SHARE_USB_STATE" = 's_stop_srv' ]; then
			usb_link_off
		else
			warn "unknonw usb state"
		fi	
	else
		USB_ACCESS_ON='n'
		warn "Wrong state?!"
	fi
}
