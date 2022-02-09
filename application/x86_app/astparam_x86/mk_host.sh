#!/bin/sh

rm -f roparam
rm -f rwparam
rm -f roparam_sh
rm -f rwparam_sh
rm -f mtdblkroparam
rm -f mtdblkparam

dd if=/dev/zero of=mtdblkroparam bs=64k count=1
dd if=/dev/zero of=mtdblkparam bs=64k count=1

./astparam w bootargs root=/dev/ram rw
./astparam w bootcmd bootm 14080000 14400000
./astparam w baudrate 115200
./astparam w eeprom n
./astparam w ethact faradaynic#0
./astparam w bootdelay 1
./astparam w filesize 1000000
./astparam w fileaddr 43000000
./astparam w gatewayip 192.168.0.1
./astparam w netmask 255.255.255.0
./astparam w ipaddr 192.168.0.86
./astparam w serverip 192.168.0.116
./astparam w stdin serial
./astparam w stdout serial
./astparam w stderr serial
./astparam w eth1addr 00:C0:A8:12:34:56
./astparam w ethaddr 00:15:25:35:00:11
./astparam w bootfile \"all.bin\"
# Start of AST specific parameters
./astparam w scenario usb_only
# End of AST specific parameters
./astparam save ro

ROOT_DIR="../../../"
IMAGE_DIR="images"

cp -f ./mtdblkroparam "${ROOT_DIR}${IMAGE_DIR}/"

cd ${ROOT_DIR}
dd if=${IMAGE_DIR}/mtdblkroparam of=all.bin bs=64k seek=255 count=1 conv=notrunc


