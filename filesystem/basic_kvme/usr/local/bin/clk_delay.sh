#!/bin/sh

#CFG="03d 07d 0bd 0fd 13d 17d 1bd 1fd 23d 27d 2bd 2fd 33d 37d 3bd 3fd 43d 47d 4bd 4fd 53d 57d 5bd 5fd 63d 67d 6bd 6fd 73d 77d 7bd 7fd 83d 87d 8bd 8fd 93d 97d 9bd 9fd a3d a7d abd afd b3d b7d bbd bfd c3d c7d cbd cfd d3d d7d dbd dfd e3d e7d ebd efd f3d f7d fbd ffd"
loop="1 2 3 4 5 6 7"
REG=0x1e6e20d8
TEST_CMD="3840 2160 30 0 0 4" # Default white/black two tone
#TEST_CMD="3840 2160 30 0 0 4 0xFF800000 0xFF000000" # Red MSB/Black two tone in pixel
#TEST_CMD="3840 2160 30 0 0 4 0xFF008000 0xFF000000" # Green MSB/Black two tone in pixel
#TEST_CMD="3840 2160 30 0 0 4 0xFF000080 0xFF000000" # Blue MSB/Black two tone in pixel
TEST_PATH="/sys/devices/platform/display/test_mode"
CLK_EDGE_PATH="/sys/devices/platform/SiI9136/clk_edge_select"
I2C_PATH=/sys/devices/platform/i2c
I2C_ID=5
I2C_ADDR=72
I2C_OFFSET=40



dmesg -n 1

. /usr/local/bin/bash/utilities.sh

# Show ring test result first
source calc_ring.sh
echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
echo "MCLK Ring"
calc_oscclk mclk
printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
echo "DLY32 Ring"
calc_oscclk dly32
printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"

e e_stop_link
sleep 3
# Make sure LM got e_stop_link by calling lmparam.
echo `lmparam g MY_MAC`

echo $I2C_ID $I2C_ADDR > $I2C_PATH/io_select
echo $I2C_OFFSET 1 > $I2C_PATH/i_range
echo 1 > /sys/devices/platform/SiI9136/no_tmds_stable_check


SCUD8_ORI=`read_reg $REG`
SII9136_CLK_EDGE_ORI=`cat /sys/devices/platform/SiI9136/clk_edge_select`
dly_value=0

# [1:0]: clock edge select: 1: rising, 0: falling
# [1]: 4K clock edge select
# [0]: 1080 clock edge select
echo 3 > $CLK_EDGE_PATH
echo HDMI Tx Edge select: rising
while [ $dly_value -le 63 ]; do # 0x3F == 63
	echo $TEST_CMD > $TEST_PATH
	# Modify tx dely: SCUD8[6:11]
	mod_reg "$REG" "$(( ($dly_value & 0x3F) << 6 ))" "0xFC0"
	i=`read_reg $REG`
	sleep 3
	RESULT=0
	for j in $loop; do
		tx=$(cat $I2C_PATH/io_value  | sed -e "1d" | sed 's/40: //g')
		if [ "$tx" != "C0" ]; then
			RESULT=1
			break
		fi
	done
	# calculate current ring value
	calc_oscclk dly32
	if [ $RESULT == '0' ]; then
		#echo "Clock delay setting: "$i "OK"
		printf "CLK_Delay_SCUD8: $i, OK; Ring_SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
	else
		#echo "Clock delay setting: "$i "FAIL"
		printf "CLK_Delay_SCUD8: $i, FAIL; Ring_SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
	fi
	dly_value="$(( $dly_value + 1 ))"
done

dly_value=0

echo 0 > $CLK_EDGE_PATH
echo HDMI Tx Edge select: falling
while [ $dly_value -le 63 ]; do # 0x3F == 63
	echo $TEST_CMD > $TEST_PATH
	# Modify tx dely: SCUD8[6:11]
	mod_reg "$REG" "$(( ($dly_value & 0x3F) << 6 ))" "0xFC0"
	i=`read_reg $REG`
	sleep 3
	RESULT=0
	for j in $loop;	do
		tx=$(cat $I2C_PATH/io_value  | sed -e "1d" | sed 's/40: //g')
		if [ "$tx" != "C0" ]; then
			RESULT=1
			break
		fi
	done
	# calculate current ring value
	calc_oscclk dly32
	if [ $RESULT == '0' ]; then
		#echo "Clock delay setting: "$i "OK"
		printf "CLK_Delay_SCUD8: $i, OK; Ring_SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
	else
		#echo "Clock delay setting: "$i "FAIL"
		printf "CLK_Delay_SCUD8: $i, FAIL; Ring_SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
	fi
	dly_value="$(( $dly_value + 1 ))"
done

# Change register back to FW default value: dbd
io 1 $REG $SCUD8_ORI

# Change HDMI Tx clock edge select back to FW default: falling
echo $SII9136_CLK_EDGE_ORI > $CLK_EDGE_PATH

echo $TEST_CMD > $TEST_PATH

dmesg -n 7
