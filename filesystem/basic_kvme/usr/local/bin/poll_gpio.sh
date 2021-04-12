#!/bin/sh

PRE_STATE='N'
GPIO_SYSFS_PATH="/sys/class/leds/gpio_pwr_status_in"
GPIO_VALUE="${GPIO_SYSFS_PATH}/brightness"
GPIO_READ="$GPIO_VALUE"

get_gpio_value()
{
	# Bruce160513. Fix.
	# Write 0 to brightness means read. Write 1 to brightness means read and invert.
	# We should write 0 here instead of 1.
	echo 0 > $GPIO_READ
	cat $GPIO_VALUE
}

if ! [ -f "$GPIO_VALUE" ]; then
	echo "gpio_pwr_status_in is not available!!"
	exit
fi

while sleep 1; do
	current_value=`get_gpio_value`
	if [ "$current_value" != "$PRE_STATE" ]; then
		PRE_STATE=$current_value
		ast_send_event -1 "e_pwr_status_changed::$current_value"
	fi
done
