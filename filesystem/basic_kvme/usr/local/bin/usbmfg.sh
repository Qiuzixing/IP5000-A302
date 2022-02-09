echo 0 > /sys/devices/platform/vhci_hcd/attach_mfg
rmmod vhub.ko
insmod usbip_common_mod.ko
insmod vhub.ko
echo $1 > /sys/devices/platform/vhci_hcd/attach_mfg

