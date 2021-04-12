#!/bin/sh

read_reg()
{
	# $1: reg.
	local r=`printf "0x%X" $1`
	local reg=`io 0 $r`
	reg=${reg#*=} #remove prefix ending with '='
	reg=${reg:1}  #remove first ' ' space
	echo "0x$reg"
}

write_reg()
{
	# $1: reg.
	# $2: value.
	local r=`printf "0x%X" $1`
	local v=`printf "0x%X" $2`
	#echo "write_reg $r to $v"
	io 1 $r $v
}

mod_reg()
{
	# $1: reg
	# $2: value
	# $3: mask
	local reg=`printf "0x%X" $1`
	local value=$2
	local mask=$3
	local r=`read_reg $reg`
	r=$(( ($r & ~$mask) | ($value & $mask) ))
	#printf "mod_reg r=0x%X\n" $r
	write_reg $reg $r
}

get_bits()
{
	# $1: value
	# $2: shift
	# $3: mask
	echo "$(( ($1 >> $2) & $3 ))"
}

calc_oscclk()
{
	# $1: Test Type: dly32, mclk
	# $2: SCUD8 value. Used to get SCUD8[6:11] delay value.

	# Return value:
	# $SCUD8: Value of SCUD8. 4K is 0xdbd. 
	# $DLYSTAGE: Value from SCUD8[6:11]. 4K is 0xdbd ==> 0x2F. 
	# $RING: SCU10[29:16] reg value
	# $RING_HZ: ring value in Hz
	# $RING_NS: ring value in ns

	local SELECT=0x3
	local MULTI=4

	case "$1" in
		dly32_check)
			SELECT=0x3
			MULTI=4
			SCUD8=`cat /sys/devices/platform/scu/v_clk_delay`
			# Value from SCUD8[6:11]. 4K is 0xdbd ==> 0x36.
			DLYSTAGE=${2:-$SCUD8}
			SCUD8=$DLYSTAGE
			DLYSTAGE=$(( ($DLYSTAGE >> 6) & 0x3F ))
			#printf "DLYSTAGE=0x%X\n" $DLYSTAGE
		;;
		dly32)
			SELECT=0x3
			MULTI=4
			# Value from SCUD8[6:11]. 4K is 0xdbd ==> 0x2F.
			DLYSTAGE=`read_reg 0x1e6e20d8`
			SCUD8=$DLYSTAGE
			DLYSTAGE=$(( ($DLYSTAGE >> 6) & 0x3F ))
			#printf "DLYSTAGE=0x%X\n" $DLYSTAGE
		;;
		mclk)
			# will get around 233MHz result in AST152X SoC
			SELECT=0x8
			MULTI=1
			DLYSTAGE=0 # ??
			SCUD8=$DLYSTAGE
		;;
		*)
			echo "TBD"
			return
		;;
	esac
	
	local r=0

	write_reg 0x1e6e2010 0x30

	# Wait until SCU10[29:16] = 0
	while true; do
		r=`read_reg 0x1e6e2010`
		r=`get_bits $r 16 0x3FFF`
		if [ $r = 0 ]; then
			break;
		fi
		sleep 1
	done

	# Set SCU10[0] = 1 and SCU10[5:2] = clock for measurement
	# SCU10[5:2] Set to 0011 (0x3) for DLY32 clock.
	# SCU10[5:2] Set to 1000 (0x8) for MCLK clock.
	# SCU10[9:14] Set to delay stage to test. NOTE:SCU10[14] should be ignored and be 0.
	r=$(( (($DLYSTAGE & 0x1F) << 9) | ($SELECT << 2) | 0x1 ))
	write_reg 0x1e6e2010 $r

	# delay 1ms
	usleep 100000

	# Set SCU10[1] = 1
	mod_reg 0x1e6e2010 0x2 0x2

	# Wait until SCU10[6] = 1
	while true; do
		r=`read_reg 0x1e6e2010`
		r=`get_bits $r 6 0x1`
		if [ $r = 1 ]; then
			break;
		fi
		sleep 1
	done

	# Read SCU10[29:16] for the result. And calculate OSCCLK frequency.
	r=`read_reg 0x1e6e2010`
	r=`get_bits $r 16 0x3FFF`
	RING=$r
	#printf "SCU10[29:16] Ring value=0x%X\n" $RING

	# OSCCLK_freq = ( CLK24M / 512 * (SCU10[29:16] + 1) ) * DLY32_Multiply
	# CLK24M = 24000000
	# SCU10[29:16] = $RING
	# DLY32_Multiply = 4
	local CLK24M=24000000
	RING_HZ=$(( $MULTI * $CLK24M / 512 * ( $RING + 1 ) ))
	RING_NS="$(( 1000000*1000 / $RING_HZ )).$(( ( 1000000*1000*1000 / $RING_HZ ) % 1000 ))"
	
	#printf "OSCCLK freq=$RING_HZ Hz, delay=$RING_NS ns\n"
	
	# Return value:
	# $DLYSTAGE: Value from SCUD8[6:11]. 4K is 0xdbd ==> 0x2F. 
	# $RING: SCU10[29:16] reg value
	# $RING_HZ: ring value in Hz
	# $RING_NS: ring value in ns
}

#echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
#echo "MCLK Ring"
#calc_oscclk mclk
#printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
#echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
#echo "DLY32 Ring with FW default 4K value"
#calc_oscclk dly32_check
#printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
#echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
#echo "DLY32 Ring with specifid SCUD8 value"
#calc_oscclk dly32_check 0xdbd
#printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
#echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"
#echo "DLY32 Ring with current SCUD8 value"
#calc_oscclk dly32
#printf "Ring SCU10[29:16]: 0x%X, $RING_HZ Hz, $RING_NS ns\n" $RING
#echo "RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR"


