#/bin/sh
cd /usr/local/bin

echo "Reset to factory default..."
MAC=`./astparam g ethaddr`
if echo "$MAC" | grep -q "not defined" ; then
	MAC=''
fi
./astparam flush
./astparam s ethaddr "$MAC"
./astparam save

SII9678_SYS_PATH="/sys/devices/platform/sii9678"
if [ -f "$SII9678_SYS_PATH/fw" ]; then
	echo "Erase SiI9678 flash"
	echo 2 > $SII9678_SYS_PATH/fw
fi

SII9679_SYS_PATH="/sys/devices/platform/sii9679"
if [ -f "$SII9679_SYS_PATH/fw" ]; then
	echo "Erase SiI9679 flash"
	echo 2 > $SII9679_SYS_PATH/fw
fi

echo "done"
