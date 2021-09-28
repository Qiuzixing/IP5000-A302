#/bin/sh
cd /usr/local/bin

echo "Reset to factory default..."
./astparam flush
./astparam save

# reset Encoder EDID to default HDMI
if [ -e /sys/devices/platform/videoip/eeprom_content ]; then
    cat /sys/devices/platform/display/default_edid_hdmi > /sys/devices/platform/videoip/eeprom_content
fi

rm /data/configs/kds-7/ -rf
sync
echo "done"
