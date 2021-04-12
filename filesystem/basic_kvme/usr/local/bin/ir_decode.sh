#!/bin/sh

IR_CH_SELECT_STATE='off'
IR_CH_SELECT_CH=
IR_CH_SELECT_VALID=0
IR_CH_SELECT_TIMEOUT=3000000
IR_CH_SELECT_SCRIPT_NAME=_irto
IR_CH_SELECT_SCRIPT=$IR_CH_SELECT_SCRIPT_NAME.sh

echo '#!/bin/sh' > $IR_CH_SELECT_SCRIPT
echo '' >> $IR_CH_SELECT_SCRIPT
echo 'usleep $1' >> $IR_CH_SELECT_SCRIPT
echo 'ast_send_event -1 e_ir_decoded::timeout' >> $IR_CH_SELECT_SCRIPT

chmod a+x $IR_CH_SELECT_SCRIPT

_stop_ir_ch_select_timer()
{
	pkill $IR_CH_SELECT_SCRIPT_NAME
}

_refire_ir_ch_select_timer()
{
	_stop_ir_ch_select_timer
	$IR_CH_SELECT_SCRIPT $1 &
}

_ch_normalize()
{
	printf "%0${IR_CH_SELECT_VALID}d" $IR_CH_SELECT_CH
}

channel_set()
{
	local _addr=$1
	local _code=$2
	local _input=0

	_IFS="$IFS";IFS='_';set -- $IR_SW_DECODE_NEC_CFG;shift 0;IFS="$_IFS"

	addr=$1

	if [ $addr = 65535 ]; then
		# device address is default, just report decoded result
		echo "NEC decode result: addr $_addr code $_code"
		return
	fi

	b0=$2
	b1=$3
	b2=$4
	b3=$5
	b4=$6
	b5=$7
	b6=$8
	b7=$9
	b8=${10}
	b9=${11}

	case "$_code" in
		$b1)
			_input=1
		;;
		$b2)
			_input=2
		;;
		$b3)
			_input=3
		;;
		$b4)
			_input=4
		;;
		$b5)
			_input=5
		;;
		$b6)
			_input=6
		;;
		$b7)
			_input=7
		;;
		$b8)
			_input=8
		;;
		$b9)
			_input=9
		;;
		$b0)
			_input=0
		;;
		*)
			return
		;;
	esac

	if [ "$IR_CH_SELECT_STATE" = 'idle' ]; then
		# idle state is about to issue command, block incoming code
		return
	fi

	if [ "$IR_CH_SELECT_STATE" = 'off' ]; then
		IR_CH_SELECT_CH=$_input
		IR_CH_SELECT_STATE='selecting'
	else
		IR_CH_SELECT_CH=$(( $IR_CH_SELECT_CH*10+$_input ))
	fi

	IR_CH_SELECT_VALID=$(( $IR_CH_SELECT_VALID+1 ))
	osd_on_tl.sh `_ch_normalize`

	_refire_ir_ch_select_timer $IR_CH_SELECT_TIMEOUT

	if [ $IR_CH_SELECT_VALID == 4 ] ; then
		_stop_ir_ch_select_timer
		ir_timeout
	fi
}

handle_e_ir_decoded()
{
	#
	# e_ir_decoded::$type::$addr::$code
	#  To select channel by 4-digit value
	#  the codes for 0~9 are defined in IR_SW_DECODE_NEC_CFG
	#  IR_SW_DECODE_NEC_CFG format is:
	#	[devaddr]_[0 code]_[1 code]_[2 code]_..._[8 code]_[9 code]
	#
	# $type:
	#  nec:
	#  timeout:
	# $addr: remote control device address, be required when $type is nec
	# $code: remote control button code, be required when $type is nec
	#
	# for example:
	# e_ir_decoded::nec::0::26
	# e_ir_decoded::timeout
	#
	_IFS="$IFS";IFS=':';set -- $*;shift 2;IFS="$_IFS"

	local _type=$1

	case "$_type" in
	nec)
		local _addr=$3
		local _code=$5
		channel_set $_addr $_code
	;;
	timeout)
		ir_timeout
	;;
	esac
}

ir_timeout()
{
	if [ "$IR_CH_SELECT_STATE" = 'off' ]; then
		echo "Do nothing"
	elif [ "$IR_CH_SELECT_STATE" = 'selecting' ]; then
		# To "idle" state.
		IR_CH_SELECT_STATE='idle'
		_refire_ir_ch_select_timer 500000
	elif [ "$IR_CH_SELECT_STATE" = 'idle' ]; then
		# Confirm change and go to "off" state
		osd_off.sh 0
		IR_CH_SELECT_VALID=4
		IR_CH_SELECT_CH=`_ch_normalize`
		# FIXME free routing not support yet => choose video channel
		if [ "$IR_CH_SELECT_CH" != "$CH_SELECT_V" ]; then
			ast_send_event -1 e_reconnect::$IR_CH_SELECT_CH
		elif [ $STATE != 's_srv_on' ]; then
			ast_send_event -1 e_reconnect::$IR_CH_SELECT_CH
		fi
		IR_CH_SELECT_STATE='off'
		IR_CH_SELECT_VALID=0
	fi
}
