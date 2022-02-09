#!/bin/sh

# Test code for "$IMAGE_FOLDER" not defined.
#TARGET="ast1520"
#CONFIG="c"
#TARGET_FOLDER_NAME="$TARGET"-"$CONFIG"
#IMAGE_FOLDER="$AST_SDK_ROOT"/images/"$TARGET_FOLDER_NAME"

cd "$IMAGE_FOLDER"

rm -f roparam
rm -f rwparam
rm -f roparam_sh
rm -f rwparam_sh
rm -f mtdblkroparam
rm -f mtdblkparam

dd if=/dev/zero of=mtdblkroparam bs=64k count=1
dd if=/dev/zero of=mtdblkparam bs=64k count=1

astparam w bootargs root=/dev/ram rw
astparam w bootcmd bootm 20080000 20400000
astparam w baudrate 115200
astparam w eeprom n
astparam w ethact faradaynic#0
astparam w bootdelay 1
astparam w filesize 1000000
astparam w fileaddr 83000000
astparam w gatewayip 192.168.0.1
astparam w netmask 255.255.255.0
astparam w ipaddr 192.168.0.60
astparam w serverip 192.168.0.116
astparam w stdin serial
astparam w stdout serial
astparam w stderr serial
astparam w eth1addr 00:C0:A8:12:34:56
astparam w ethaddr 00:15:25:35:00:11
astparam w bootfile \"all.bin\"
# Start of AST specific parameters
#astparam w scenario usb_only
# End of AST specific parameters
astparam save ro


