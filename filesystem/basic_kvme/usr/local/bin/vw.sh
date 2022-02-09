#!/bin/sh
cd /usr/local/bin

if [ "$1" = '0_0' ]; then
	echo 1 1 0 0 > /sys/devices/platform/display/vw_layout
elif [ "$1" = '0_1' ]; then
	echo 1 1 0 1 > /sys/devices/platform/display/vw_layout
elif [ "$1" = '1_0' ]; then
	echo 1 1 1 0 > /sys/devices/platform/display/vw_layout
elif [ "$1" = '1_1' ]; then
	echo 1 1 1 1 > /sys/devices/platform/display/vw_layout
elif [ "$1" = '0' ]; then
	echo 0 0 0 0 > /sys/devices/platform/display/vw_layout
else
	echo "Usage: vw.sh screen_num"
	echo "screen_num: 0 or 0_0 or 0_1 or 1_0 or 1_1"
fi

exit 0

if [ "$1" = '0_0' ]; then
	./io 1 1e6e608c 80008000
	./io 1 1e6e6084 1681e00
	./io 1 1e6e6080 6400000
	./io 1 1e6e60c0 7654321f
	./io 1 1e6e60c4 fedcba98
	./io 1 1e6e60c8 7654321f
	./io 1 1e6e60cc fedcba98
	./io 1 1e6e6060 30000231
	#echo 14000 > /sys/devices/platform/videoip/delay_dequeue
elif [ "$1" = '0_1' ]; then
	./io 1 1e6e608c 80008000
	./io 1 1e6e6084 1681e00
	./io 1 1e6e6080 6407800
	./io 1 1e6e60c0 7654321f
	./io 1 1e6e60c4 fedcba98
	./io 1 1e6e60c8 7654321f
	./io 1 1e6e60cc fedcba98
	./io 1 1e6e6060 30000231
	#echo 14000 > /sys/devices/platform/videoip/delay_dequeue
elif [ "$1" = '1_0' ]; then
	./io 1 1e6e608c 80008000
	./io 1 1e6e6084 1681e00
	#./io 1 1e6e6080 67fc000
	./io 1 1e6e6080 6400000
	./io 1 1e6e60c0 7654321f
	./io 1 1e6e60c4 fedcba98
	./io 1 1e6e60c8 7654321f
	./io 1 1e6e60cc fedcba98
	./io 1 1e6e6060 30000231
	#echo 8000 > /sys/devices/platform/videoip/delay_dequeue
elif [ "$1" = '1_1' ]; then
	./io 1 1e6e608c 80008000
	./io 1 1e6e6084 1681e00
	#./io 1 1e6e6080 6803800
	./io 1 1e6e6080 6407800
	./io 1 1e6e60c0 7654321f
	./io 1 1e6e60c4 fedcba98
	./io 1 1e6e60c8 7654321f
	./io 1 1e6e60cc fedcba98
	./io 1 1e6e6060 30000231
	#echo 8000 > /sys/devices/platform/videoip/delay_dequeue
elif [ "$1" = '0' ]; then
	./io 1 1e6e608c 0
	./io 1 1e6e6084 2d01e00
	./io 1 1e6e6080 6400000
	./io 1 1e6e60c0 0
	./io 1 1e6e60c4 0
	./io 1 1e6e60c8 0
	./io 1 1e6e60cc 0
	./io 1 1e6e6060 30000201
	echo 0 > /sys/devices/platform/videoip/delay_dequeue
else
	echo "Usage: vw.sh screen_num"
	echo "screen_num: 0 or 0_0 or 0_1 or 1_0 or 1_1"
fi


