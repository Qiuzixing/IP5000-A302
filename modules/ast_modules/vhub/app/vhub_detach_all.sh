#!/bin/sh

vhub_driver_path="/sys/devices/platform/vhci_hcd"

echo 0 > "$vhub_driver_path"/detach 2> /dev/null
echo 1 > "$vhub_driver_path"/detach 2> /dev/null
echo 2 > "$vhub_driver_path"/detach 2> /dev/null
echo 3 > "$vhub_driver_path"/detach 2> /dev/null
echo 4 > "$vhub_driver_path"/detach 2> /dev/null
echo 5 > "$vhub_driver_path"/detach 2> /dev/null
echo 6 > "$vhub_driver_path"/detach 2> /dev/null

#ast_send_event -1 'e_usbip_user_stopped'
exit 0
