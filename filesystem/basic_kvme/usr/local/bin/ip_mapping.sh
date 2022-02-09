#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.
#
################################################################################
# Service Type ID:
# V: video over IP
# A: audio over ip (I2S)
# R: IR over IP
# S: serial over IP
# P: push button over IP (GPIO)
# U: USB over IP
# C: CEC over IP
################################################################################
# Heartbeat IP Mapping:
# Always use the same service's multicast IP
# V: 225.2.xxx.xxx
# A: 225.3.xxx.xxx
# R: 225.4.xxx.xxx
# S: 225.5.xxx.xxx
# P: 225.6.xxx.xxx
# U: 225.7.xxx.xxx
# C: 225.8.xxx.xxx
################################################################################
# Heartbeat Port Mapping. Both pinger and acker listen to this port number.
V_HB_PORT=59002
A_HB_PORT=59003
R_HB_PORT=59004
S_HB_PORT=59005
P_HB_PORT=59006
U_HB_PORT=59007
C_HB_PORT=59008
################################################################################
# Video over IP Port Mapping
# V_TX_PORT: encoder stream data to V_TX_PORT destination port.
# V_CTRL_PORT: encoder receive control/feedback on this port.
V_TX_PORT=59200
V_CTRL_PORT=59201
################################################################################
# Audio over IP Port Mapping
# A_TX_PORT: encoder stream audio to A_TX_PORT destination port.
A_TX_PORT=59300
################################################################################
# IR over IP Port Mapping
# R_TX_PORT: encoder stream data to R_TX_PORT destination port.
R_TX_PORT=59400
################################################################################
# serial over IP Port Mapping. For Type 3 only. Type 1 & 2 uses TCP 6752 port.
# S_TX_PORT: encoder stream data to S_TX_PORT destination port.
# TODO. hard coded in soip.c
#S_TX_PORT=59500
################################################################################
# Push button over IP Port Mapping
# P_TX_PORT: encoder stream data to P_TX_PORT destination port.
# Re-use msg_channel, so, no extra port needed.
#P_TX_PORT=
################################################################################
# KM over IP Port Mapping
# U_TX_PORT: KMoIP up/downstream data to U_TX_PORT destination port.
# TODO: Hard coded. Modify vhubc.sh, kmoip.c.
#U_USBIP_PORT=59700
#U_KM_DS_PORT=59702
#U_KM_US_PORT=59703
################################################################################
# CEC over IP Port Mapping
# C_TX_PORT: encoder stream data to C_TX_PORT destination port.
# C_TX_TOPOLOGY_PORT: encoder topology data to C_TX_TOPOLOGY_PORT destination port.
C_TX_PORT=59800
C_TOPOLOPY_TX_PORT=59801

# Old version which is difficult for human mapping.
#map_multicast_ip_by_4digit_v1()
#{
#	#
#	# Channel to IP mapping:
#	# 225.y.((Ch >> 8) & 0xFF).(Ch & 0xFF)
#	#
#	# Output:
#	# IP format: aaa.bbb.ccc.ddd
#	#
#	# Input:
#	# $1: The "aaa" part. $MULTICAST_IP_PREFIX
#	# $2: Service Type ID. In lower case.
#	# $3: $CH_SELECT from 0000 to 9999
#	#
#	local aaa=$1
#	local type=$2
#	local ch=$3
#	local bbb
#	local ccc
#	local ddd
#	# Split 4 digit $ch one by one to calculate its dec value.
#	local ch0=${ch:0:1}
#	local ch1=${ch:1:1}
#	local ch2=${ch:2:1}
#	local ch3=${ch:3:1}
#	# convert $ch to dec value
#	ch=$(( $ch0 * 1000 + $ch1 * 100 + $ch2 * 10 + $ch3 ))
#	# Channel to IP mapping:
#	# 225.y.((Ch >> 8) & 0xFF).(Ch & 0xFF)
#	case "$type" in
#		v)
#			bbb=2
#		;;
#		a)
#			bbb=3
#		;;
#		r)
#			bbb=4
#		;;
#		s)
#			bbb=5
#		;;
#		p)
#			bbb=6
#		;;
#		u)
#			bbb=7
#		;;
#		*)
#			echo "0.0.0.0"
#			echo "ip_mapping Error! Unknown Type ID ($type)" >&2
#			return
#		;;
#	esac
#	ccc=$(( ($ch >> 8) & 0xFF ))
#	ddd=$(( $ch & 0xFF ))
#	echo "${aaa}.${bbb}.${ccc}.${ddd}"
#}

map_multicast_ip_by_4digit()
{
	#
	# Channel to IP mapping:
	# 225.y.(Ch / 100).(Ch % 100)
	#
	# Output:
	# IP format: aaa.bbb.ccc.ddd
	#
	# Input:
	# $1: The "aaa" part. $MULTICAST_IP_PREFIX
	# $2: Service Type ID. In lower case.
	# $3: $CH_SELECT from 0000 to 9999
	#
	local aaa=$1
	local type=$2
	local ch=$3
	local bbb
	local ccc
	local ddd
	# Split 4 digit $ch one by one to calculate its dec value.
	local ch0=${ch:0:1}
	local ch1=${ch:1:1}
	local ch2=${ch:2:1}
	local ch3=${ch:3:1}
	# Channel to IP mapping:
	# 225.y.((Ch >> 8) & 0xFF).(Ch & 0xFF)
	case "$type" in
		v)
			# Default value is 2
			bbb=$MCIP_SRV_MAP_V
		;;
		a)
			# Default value is 3
			bbb=$MCIP_SRV_MAP_A
		;;
		r)
			# Default value is 4
			bbb=$MCIP_SRV_MAP_R
		;;
		s)
			# Default value is 5
			bbb=$MCIP_SRV_MAP_S
		;;
		p)
			# Default value is 6
			bbb=$MCIP_SRV_MAP_P
		;;
		u)
			# Default value is 7
			bbb=$MCIP_SRV_MAP_U
		;;
		*)
			echo "0.0.0.0"
			echo "ip_mapping Error! Unknown Type ID ($type)" >&2
			return
		;;
	esac
	ccc=$(( $ch0 * 10 + $ch1 ))
	ddd=$(( $ch2 * 10 + $ch3 ))
	echo "${aaa}.${bbb}.${ccc}.${ddd}"
}

# Never used
#map_multicast_ip_by_ip()
#{
#	#
#	# Channel to IP mapping:
#	# 225.y.CCC.DDD
#	# where CCC.DDD is from AAA.BBB.CCC.DDD CH_SELECT
#	#
#	# Output:
#	# IP format: aaa.bbb.ccc.ddd
#	#
#	# Input:
#	# $1: The "aaa" part. $MULTICAST_IP_PREFIX
#	# $2: Service Type ID. In lower case.
#	# $3: $CH_SELECT in AAA.BBB.CCC.DDD format
#	#
#	local aaa=$1
#	local type=$2
#	local ch_select=$3
#	_IFS="$IFS";IFS='.';set -- $ch_select;IFS="$_IFS"
#	local bbb
#	local ccc=$3
#	local ddd=$4
#
#	case "$type" in
#		v)
#			bbb=2
#		;;
#		a)
#			bbb=3
#		;;
#		r)
#			bbb=4
#		;;
#		s)
#			bbb=5
#		;;
#		p)
#			bbb=6
#		;;
#		u)
#			bbb=7
#		;;
#		*)
#			echo "0.0.0.0"
#			echo "ip_mapping Error! Unknown Type ID ($type)" >&2
#			return
#		;;
#	esac
#	echo "${aaa}.${bbb}.${ccc}.${ddd}"
#}

map_multicast_ip()
{
	# Input:
	# $1: The "aaa" part. $MULTICAST_IP_PREFIX
	# $2: Service Type ID. In lower case.
	# $3: $CH_SELECT in 0000~9999 format
	local _prefix=$1
	local _type=$2
	local _ch_select=$3

	map_multicast_ip_by_4digit $_prefix $_type $_ch_select

	return

	case "$_ch_select" in
		[[:digit:]][[:digit:]][[:digit:]][[:digit:]])
			map_multicast_ip_by_4digit $_prefix $_type $_ch_select
		;;
		*)
			#map_multicast_ip_by_ip $_prefix $_type $_ch_select
			echo "!!!! Error: Wrong CH_SELECT format ($_ch_select)" >&2
		;;
	esac
}
