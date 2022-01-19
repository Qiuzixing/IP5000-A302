#!/bin/bash
#
# Copyright (c) 2017
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.

#U_HB_PORT=59007

. ip_mapping.sh

update_session_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	U_SESSION_ID=$(( ($U_SESSION_ID + 1) & 0xFFFFFFFF ))

	# reserved ID for special usage: 0, 1
	if [ "$U_SESSION_ID" -lt 2 ]; then
		U_SESSION_ID='2'
	fi
}

update_request_id()
{
	# Limit session id to 32bits long. C code saves it using unsigned int type.
	U_REQUEST_ID=$(( ($U_REQUEST_ID + 1) & 0xFFFFFFFF ))
}

_get_node_info()
{
	local _ch_select=$1
	local _node_info=""
	local _match_key

	# The $CH_SELECT should always be valid at this stage.
	case "$_ch_select" in
		?*.?*.?*.?*)
			#if there's a dot ".", it's definitely ip address, no name resolution needed
			# If failed to ping the host, just return empty string
			if ! { ping "$_ch_select" | grep -q " is alive!"; }; then
				echo ""
				echo "ping fail?! ($_ch_select)" >&2
				return
			fi
			_match_key='MY_IP'
		;;
		*)
			#no dot ".", assume friendly CH_SELECT name
			_match_key='CH_SELECT'
		;;
	esac
	_node_info=`node_query --dump --match_key $_match_key=$_ch_select --match_key IS_HOST=y --match_key HOSTNAME_PREFIX=$HOSTNAME_PREFIX --max 1`
	echo "$_node_info"
}

_parse_node_info()
{
	local _info

	# NOTE! DO NOT "" $_PEER_NODE_INFO, otherwise parsing will be wrong.
	for _info in $_PEER_NODE_INFO; do
		case "$_info" in
		MY_IP=*)
			_PEER_IP=${_info#MY_IP=}
			#echo "_PEER_IP=$_PEER_IP" >&2
		;;
		CH_SELECT=*)
			_PEER_CH_SELECT=${_info#CH_SELECT=}
			#echo "_PEER_CH_SELECT=$_PEER_CH_SELECT" >&2
		;;
		NO_USB=*)
			_PEER_NO_USB=${_info#NO_USB=}
			#echo "_PEER_NO_USB=$_PEER_NO_USB" >&2
		;;
		NO_KMOIP=*)
			_PEER_NO_KMOIP=${_info#NO_KMOIP=}
			#echo "_PEER_NO_KMOIP=$_PEER_NO_KMOIP" >&2
		;;
		*)
		;;
		esac
	done

	# Validate result
	case "$_PEER_IP" in
		?*.?*.?*.?*)
			# valid peer id
			return
		;;
		*)
			_PEER_IP='Unknown'
			echo "Unknown host ($_ip)" >&2
		;;
	esac
}

handle_ue_kmr_slave_chg()
{
	local _who="$1"
	_IFS="$IFS";IFS=':';set -- $KMOIP_ROAMING_LAYOUT;IFS="$_IFS"
	local _slaves=$*
	local _slave
	echo "Restart KMR $_who."
	for _slave in $_slaves; do
		_IFS="$IFS";IFS=',';set -- $_slave;IFS="$_IFS"
		local _smac=$1
		local _sx=$2
		local _sy=$3

		if [ "$_who" = "$_smac" ]; then
			echo "Found $_smac, $_sx, $_sy"
			start_kmr_slave $_smac $_sx $_sy
			# Slave change notification is already registered. No need here.
		fi
	done
}

start_kmr_slave()
{
	local _mac=$1
	local _idx_x=$2
	local _idx_y=$3
	local _slave
	local _ch_select_v
	local _multicast_ip

	# Find specified MAC's CH_SELECT_V value.
	# We use V instead of U because we believe user want to KM control
	# what it 'see' on the screen. Plus, a KMR slave can free routing
	# USB to other host.
	_slave=$( node_query --get_key CH_SELECT_V --match_key MY_MAC=$_mac --match_key IS_HOST=n --match_key HOSTNAME_PREFIX=$HOSTNAME_PREFIX --max 1 )
	# Remove 'CH_SELECT_V=' prefix
	_ch_select_v=${_slave#CH_SELECT_V=}

	# foolproof ch_select_v
	# Validate number format first.
	#if [ "$_ch_select" -eq "$_ch_select" ] 2>/dev/null; then
	# =~ regex match
	# ^: beginning of pattern
	# $: end of pattern
	if [[ $_ch_select_v =~ ^[0-9][0-9][0-9][0-9]$ ]]; then
		# get target's multicast ip
		_multicast_ip=$( map_multicast_ip $MULTICAST_IP_PREFIX u $_ch_select_v )
		kmoip -m $_multicast_ip -c -r $_mac -x $_idx_x -y $_idx_y
	else
		# not found?! Hope add_chg_notify will handle this.
		echo "KMoIP Roaming Slave $_mac,$_idx_x,$_idx_y not found?!"
	fi
}

start_kmoip_roaming_c()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _slaves=$*
	local _slave

	for _slave in $_slaves; do
		_IFS="$IFS";IFS=',';set -- $_slave;IFS="$_IFS"
		local _smac=$1
		local _sx=$2
		local _sy=$3
		start_kmr_slave $_smac $_sx $_sy
		# Register change notification
		ipc @nq_ctrl s add_chg_notify:$_smac
	done
}

stop_kmoip_roaming_c()
{
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _slaves=$*
	local _slave

	for _slave in $_slaves; do
		_IFS="$IFS";IFS=',';set -- $_slave;IFS="$_IFS"
		local _smac=$1
		local _sx=$2
		local _sy=$3
		# sockets are detached in one sysfs call. No need here.
		# de-register change notification
		ipc @nq_ctrl s del_chg_notify:$_smac
	done
}

start_kmoip_c()
{
	#KMOIP_ROAMING_LAYOUT="82D0194B1276,1,0:82D0194B1277,-1,0:82D0194B1276,1,1"
	if [ "$MULTICAST_ON" = 'y' ]; then
		kmoip -m $U_MULTICAST_IP -c -r $MY_MAC -x 0 -y 0
		start_kmoip_roaming_c $KMOIP_ROAMING_LAYOUT
	else
		kmoip -u $U_ENCODER_IP -c -r $MY_MAC -x 0 -y 0
	fi
	U_KMOIP_STATE='on'
}

stop_kmoip_c()
{
	# MAC idx_x idx_y upstream_fd downstream_fd
	#echo $MY_MAC 0 0 0 0 > $KMOIP_SYS_PATH/attach
	# MAC use FFFFFFFFFFFF means detach all including roaming slaves.
	echo FFFFFFFFFFFF 0 0 0 0 > $KMOIP_SYS_PATH/attach
	stop_kmoip_roaming_c $KMOIP_ROAMING_LAYOUT
	U_KMOIP_STATE='off'
}

do_search_encoder()
{
	local _U_SESSION_ID="$1"
	local _U_CH_SELECT="$2"

	local _PEER_NODE_INFO=`_get_node_info $_U_CH_SELECT`
	local _PEER_IP='Unknown'
	local _PEER_MULTICAST_IP='0.0.0.0'
	local _PEER_CH_SELECT='0000'
	local _PEER_NO_USB=$NO_USB
	local _PEER_NO_KMOIP=$NO_KMOIP

	_parse_node_info

	case "$_PEER_IP" in
		Unknown)
			# Retry
			ipc @u_lm_set s "ue_no_encoder_ip:$_U_SESSION_ID"
		;;
		*)
			# Found
			if [ "$_PEER_NO_USB" = 'y' ] && [ "$_PEER_NO_KMOIP" = 'y' ]; then
				# Peer doesn't enable this service.
				ipc @u_lm_set s ue_stop
			else
				_PEER_MULTICAST_IP=`map_multicast_ip $MULTICAST_IP_PREFIX u $_PEER_CH_SELECT`
				ipc @u_lm_set s "ue_encoder_ip_got:$_U_SESSION_ID:$_PEER_IP:$_PEER_MULTICAST_IP:$_PEER_NO_USB:$_PEER_NO_KMOIP"
			fi
		;;
	esac
}

to_s_search()
{
	U_STATE='s_search'
	do_search_encoder $U_SESSION_ID $U_CH_SELECT &
}

to_s_start_hb()
{
	U_STATE='s_start_hb'
	if [ "$MULTICAST_ON" = 'n' ]; then
		# start_pinger:type:session_id:uc:target_acker_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:u:$U_SESSION_ID:uc:$U_ENCODER_IP:$U_HB_PORT
	else
		# start_pinger:type:session_id:mc:hb_multicast_group_ip:target_and_listen_port
		ipc @hb_ctrl s start_pinger:u:$U_SESSION_ID:mc:$U_MULTICAST_IP:$U_HB_PORT
	fi
}

to_s_srv_ready()
{
	U_STATE='s_srv_ready'
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		ast_send_event -1 e_ulm_notify_chg
	fi
}

to_s_srv_on()
{
	U_STATE='s_srv_on'
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		ast_send_event -1 e_ulm_notify_chg
	fi
}

to_s_start_srv()
{
	# handle_ue_request_usbip checks U_STATE
	U_STATE='s_start_srv'

	if [ "$PEER_NO_USB" = 'n' ]; then
		if [ "$SHARE_USB" = 'n' ] || [ "$U_USBIP_STATE" = 'request' ]; then
			handle_ue_request_usbip
		fi
	fi

	if [ "$PEER_NO_KMOIP" = 'n' ]; then
		if [ "$NO_KMOIP" = 'n' ]; then
			start_kmoip_c
		fi
	fi
}

to_s_idle()
{
	U_STATE='s_idle'
	if [ "$PWR_LED_TYPE" = 'share_usb' ]; then
		ast_send_event -1 e_ulm_notify_chg
	fi
}

do_stop_srv()
{
	# U_SESSION_ID is changed before calling to_s_idle(). So, stop_pinger's ve_no_heartbeat will be ignored.
	# Stop video HB
	ipc @hb_ctrl s stop_pinger:u:$U_SESSION_ID

	if { lsmod | grep -q usbip; }; then
		rmvhubc.sh
	fi

	if [ "$PEER_NO_KMOIP" = 'n' ]; then
		if [ "$NO_KMOIP" = 'n' ]; then
			stop_kmoip_c
		fi
	fi
}

handle_ue_heartbeat_init_ok()
{
	#Parse ue_heartbeat_init_ok:$U_SESSION_ID
	local _t="$1"

	if [ "$U_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $U_SESSION_ID"
		return
	fi
	to_s_start_srv $U_SESSION_ID $U_ENCODER_IP $U_MULTICAST_IP
	# return as U_STATE='s_start_srv'
	to_s_srv_ready
	# return as U_STATE='s_srv_ready'
}

handle_ue_no_heartbeat()
{
	#Parse ue_no_heartbeat:$U_SESSION_ID
	local _t="$1"

	if [ "$U_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $U_SESSION_ID"
		return
	fi

	# Restart a new session.
	# Note: Send a start event may causes race condition if there is an start/stop event already pending.
	#       So, we call it directly.
	handle_ue_start	"$U_CH_SELECT"
}

handle_ue_encoder_ip_got()
{
	#Parse ue_encoder_ip_got:$U_SESSION_ID:$_encoder_ip:$_multicast_ip:$PEER_NO_USB:$PEER_NO_KMOIP
	local _t="$1"
	local _encoder_ip="$2"
	local _multicast_ip="$3"

	if [ "$U_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $U_SESSION_ID"
		return
	fi

	PEER_NO_USB="$4"
	PEER_NO_KMOIP="$5"
	U_ENCODER_IP="$_encoder_ip"
	U_MULTICAST_IP="$_multicast_ip"
	to_s_start_hb $U_SESSION_ID $U_ENCODER_IP $U_MULTICAST_IP
}

handle_ue_no_encoder_ip()
{
	#Parse ue_no_encoder_ip:$U_SESSION_ID
	local _t="$1"

	if [ "$U_SESSION_ID" != "$_t" ]; then
		echo "Ignore. session ID doesn't match $_t vs $U_SESSION_ID"
		return
	fi
	# retry again
	to_s_search $U_SESSION_ID $U_CH_SELECT
}

handle_ue_start_kmoip()
{
	case "$U_STATE" in
		's_srv_ready' | 's_start_srv' | 's_srv_on')
			# do start kmoip if not started yet.
			if [ "$U_KMOIP_STATE" != 'on' ] && [ "$PEER_NO_KMOIP" = 'n' ]; then
				start_kmoip_c
			fi
		;;
		*)
			# Ignore.
			echo "Ignore KMoIP request. Not ready yet."
		;;
	esac
}

handle_ue_stop_kmoip()
{
	# stop kmoip no matter how.
	if [ "$U_KMOIP_STATE" = 'on' ]; then
		stop_kmoip_c
	fi
}

handle_ue_request_usbip()
{
	if [ "$NO_USB" = 'y' ] || [ "$PEER_NO_USB" = 'y' ] ; then
		return
	fi
	# We ack user immediately no matter what current uLM state is.
	ast_send_event -1 e_usb_requesting

	case "$U_STATE" in
		's_srv_ready' | 's_start_srv')
			# Since we double checked usbip.ko in handle_ue_start_usbip().
			# So, it is fine to request twice. (in case request lost)
			U_USBIP_STATE='request'
			update_request_id
			ipc @hb_ctrl s msg:u:acker:0.0.0.0:ue_request_usbip:"$U_REQUEST_ID":"$MY_IP"
		;;
		's_srv_on')
			echo "Ignore USBoIP request. Already ON"
		;;
		*)
			# Save it and process later.
			U_USBIP_STATE='request'
		;;
	esac
}

handle_ue_start_usbip()
{
	# Never call this function from LM or shell.
	# Should call handle_ue_request_usbip() instead.
	# Otherwise, uLM don't know how to handle "[new]ue_stop_usb, [old]ue_start_usb" sequence.
	if [ "$NO_USB" = 'y' ] || [ "$PEER_NO_USB" = 'y' ] ; then
		return
	fi
	local _t="$1"
	if [ "$U_REQUEST_ID" != "$_t" ]; then
		echo "Ignore mismatched REQUEST_ID ($U_REQUEST_ID vs $_t)"
		return
	fi

	# Check U_STATE. in case user do 'e e_start_usb' from nowhere.
	# - is s_srv_ready, then start
	# - is not s_srv_ready yet, then record U_USBIP_STATE as 'on' and request usbip later.
	case "$U_STATE" in
		s_srv_ready)
			# So, it is fine to start twice.
			if { lsmod | grep -q usbip; }; then
				echo "usbip.ko loaded?!"
				return
			fi
			U_USBIP_STATE='on'
			vhubc.sh $U_ENCODER_IP
			to_s_srv_on
		;;
		*)
			echo "Got ue_start_usbip under U_STATE=$U_STATE !?"
		;;
	esac
}

handle_ue_stop_usbip()
{
	if [ "$NO_USB" = 'y' ] || [ "$PEER_NO_USB" = 'y' ] ; then
		return
	fi
	# Ignore previous requests buy pump up U_REQUEST_ID number.
	update_request_id
	# We ack user immediately no matter what current uLM state is.
	ast_send_event -1 e_usb_stopping

	# Check U_STATE. in case user do 'e e_stop_usb' from nowhere.
	# - is s_srv_on, then stop
	# - is not s_srv_on yet, then save it and process it later.
	case "$U_STATE" in
		s_srv_on)
			if ! { lsmod | grep -q usbip; }; then
				echo "usbip.ko unloaded?!"
				return
			fi
			U_USBIP_STATE='off'
			rmvhubc.sh
			to_s_srv_ready
		;;
		*)
			U_USBIP_STATE='off'
		;;
	esac
}

handle_ue_start()
{
	# validata input
	if [ "$#" != '1' ]; then
		return
	fi

	if [ "$U_STATE" != 's_idle' ]; then
		do_stop_srv $U_SESSION_ID
		# in e_reconnect case, We re-request USBoIP if already requested.
		case "$U_USBIP_STATE" in
			on | request)
				U_USBIP_STATE='request'
			;;
			*)
				# ignore 'off' state
			;;
		esac
	fi

	foolproof_kmr_layout "$KMOIP_ROAMING_LAYOUT"

	update_session_id

	# get channel param
	U_CH_SELECT="$1"
	U_MULTICAST_IP='0.0.0.0'

	to_s_search $U_SESSION_ID $U_CH_SELECT
}

handle_ue_stop()
{
	do_stop_srv $U_SESSION_ID
	# By design, USBoIP state reset back to 'off' after USB service stopped by user.
	case "$U_USBIP_STATE" in
		on | request)
			U_USBIP_STATE='off'
		;;
		*)
			# ignore 'off' state
		;;
	esac

	update_session_id
	update_request_id
	to_s_idle
}

handle_ue_lm_destroy()
{
	kill $PID_IPC_SVR
	# TODO
	exit 0
}

handle_ue_patch()
{
	# Used to patch link_mgrX.sh itself.
	if [ -f ./patch_u_lmc.sh ]; then
		source ./patch_u_lmc.sh
	fi
}

handle_ue_param_query()
{
	#
	# We assume query/reply happens in pair and in sequence.
	# User is allowed to set/get in between query and reply.
	# ==> There can be only one queryer and replyer must reply in its own context.
	#
	# ipc @u_lm_query q ue_param_query:param_to_query
	# $1 is param_to_query
	# FIXME: This line is expensive. Is there a better way?
	local _value=`eval echo "\\$$1"`
	echo "[$1=$_value]"
	ipc @u_lm_reply s "$_value"
}

handle_ue_param_set()
{
	# ipc @u_lm_set s ue_param_set:key:value
	# $1: key to set
	# $2: value to set
	local _key=$1
	shift
	local _value=$(echo $@ | sed 's/ /:/g')

	eval "$_key=\"$_value\""
	echo "$_key=$_value"
}

handle_ue_var()
{
	# running in subshell to prevent my variables been modified.
	(
		# Unset specal variables which may cause parsing problem.
		unset _IFS
		unset IFS
		# Save all internal variables (lmparam) into /var/lm_var
		set > /var/u_lm_var
		# Remove shell built-in read only variables. Otherwise other shell can't "source" it.
		{
			sed '/^BASH*=*/d' < /var/u_lm_var |
			sed '/^UID=*/d' |
			sed '/^EUID=*/d' |
			sed '/^PPID=*/d' |
			sed '/^SHELL*=*/d' |
			sed '/^_.*=*/d'	> /var/u_lm_var
		}
	) &
	wait $!
}

handle_ue_param_dump()
{
	handle_ue_var
	local _dump=`cat /var/u_lm_var`
	ipc @u_lm_reply s "$_dump"
}

handle_ue_test()
{
	# Start HB. This should be done by link_mgrc.sh.
	#heartbeat &
	return
}

handle_ue_kmoip_token()
{
	if [ "$#" != '1' ]; then
		return
	fi
	KMOIP_TOKEN_INTERVAL=$1
	echo "$KMOIP_TOKEN_INTERVAL" > $KMOIP_SYS_PATH/token_interval
}

set_kmoip_export_conf()
{
	local CONFIG_FILE="/etc/usbip/usbip_export.conf"
	local _busid
	local _port

	if [ "$KMOIP_PORTS" = 'all' ]; then
		echo "kmoip all" >> $CONFIG_FILE
		echo ""
		return
	fi

	for _port in $KMOIP_PORTS; do
		case $_port in
			1)
				for _busid in $USB_P1_BUSID; do
					echo "kmoip $_busid" >> $CONFIG_FILE
				done
			;;
			2)
				for _busid in $USB_P2_BUSID; do
					echo "kmoip $_busid" >> $CONFIG_FILE
				done
			;;
			3)
				for _busid in $USB_P3_BUSID; do
					echo "kmoip $_busid" >> $CONFIG_FILE
				done
			;;
			4)
				for _busid in $USB_P4_BUSID; do
					echo "kmoip $_busid" >> $CONFIG_FILE
				done
			;;
			5)
				for _busid in $USB_P5_BUSID; do
					echo "kmoip $_busid" >> $CONFIG_FILE
				done
			;;
			*)
				echo "not supported"
			;;
		esac
	done

	echo "" >> $CONFIG_FILE
}

set_usbip_export_conf()
{
	local CONFIG_FILE="/etc/usbip/usbip_export.conf"
	local _tmp

	echo "enable_auto_export=yes" > $CONFIG_FILE
	echo "server file /etc/vhub_host_ip" >> $CONFIG_FILE
	# auto_export or no_auto_export
	echo "policy=$USB_DEFAULT_POLICY" >> $CONFIG_FILE
	# auto_export or no_auto_export
	echo "if_interface_conflict=$USB_CONFLICT_POLICY" >> $CONFIG_FILE

	for _tmp in $USB_DISABLE_CLASSES; do
		echo "class $_tmp no_auto_export" >> $CONFIG_FILE
	done

	for _tmp in $USB_ENABLE_CLASSES; do
		echo "class $_tmp auto_export" >> $CONFIG_FILE
	done

	for _tmp in $USB_DISABLE_DEVICES; do
		echo "device $_tmp no_auto_export" >> $CONFIG_FILE
	done

	for _tmp in $USB_ENABLE_DEVICES; do
		echo "device $_tmp auto_export" >> $CONFIG_FILE
	done

	echo "" >> $CONFIG_FILE
}

ulm_init()
{
	# create lock file before starting ipc_server
	echo "lock file for @u_lm_query" > /var/lock/@u_lm_query.lck

	ipc_server @u_lm_set @u_lm_get @u_lm_query @u_lm_reply &
	PID_IPC_SVR=$!

	# TODO
	# Bruce140313. usbip export config may be used by both USBoIP and KMoIP
	set_usbip_export_conf
	if [ "$NO_KMOIP" = 'n' ]; then
		# Bruce140401. We should only set kmoip_ports export policy under KMoIP enabled case.
		# Otherwise, HID won't export to USBoIP when KMoIP is disabled (due to the setting.)
		set_kmoip_export_conf
		echo $KMOIP_HOTKEYS > $KMOIP_SYS_PATH/hotkey
		echo $KMOIP_TOKEN_INTERVAL > $KMOIP_SYS_PATH/token_interval
	fi
}

signal_handler()
{
	echo ""
	echo ""
	echo ""
	echo "ERROR!!!! uLM received signal!!!!!! Ignore it."
	echo ""
	echo ""
	echo ""
}

event_loop_handler()
{
	#Parse $event. Firt param is event, the others are parameters.
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _e=$1
	shift 1
	local _p=$@

	#echo "event=$_e; param=$_p"

	case "$_e" in
		ue_?*)
			handle_"$_e" $_p
		;;
		*)
			echo "ERROR!!!! Invalid event ($_e) received"
		;;
	esac
}

foolproof_kmr_layout()
{
	#
	# KMOIP_ROAMING_LAYOUT rules:
	# - MAC must be upper case
	# - Skip 0,0 which should always be master
	#
	_IFS="$IFS";IFS=':';set -- $*;IFS="$_IFS"
	local _slaves=$*
	local _slave
	declare -i count=0
	echo "KMR Parsing:$_slaves"
	# First convert to upper case
	_slaves="$(tr '[a-z]' '[A-Z]' <<< "$_slaves")"
	# iterate the list and skip 0,0 (master)
	for _slave in $_slaves; do
		_IFS="$IFS";IFS=',';set -- $_slave;IFS="$_IFS"
		local _smac=$1
		local _sx=$2
		local _sy=$3
		# Skip master which is always 0,0
		if [ "$_sx" = '0' ] && [ "$_sy" = '0' ]; then
			continue
		fi
		if [ "$count" -eq 0 ]; then
			KMOIP_ROAMING_LAYOUT="$_smac,$_sx,$_sy"
		else
			KMOIP_ROAMING_LAYOUT="$KMOIP_ROAMING_LAYOUT:$_smac,$_sx,$_sy"
		fi
		count=$(( $count + 1 ))
	done
	echo "KMOIP_ROAMING_LAYOUT=$KMOIP_ROAMING_LAYOUT"
}

event_loop()
{
	# Try to ignore all TERM signals.
	trap signal_handler SIGTERM SIGPIPE SIGHUP SIGINT SIGALRM SIGUSR1 SIGUSR2 SIGPROF SIGVTALRM

	# To ignore previous pending events, we init U_SESSION_ID at the beginning of event loop.
	U_SESSION_ID="$RANDOM"
	U_REQUEST_ID="$RANDOM"
	U_USBIP_STATE='off'
	U_KMOIP_STATE='off'
	PEER_NO_USB="$NO_USB"
	PEER_NO_KMOIP="$NO_KMOIP"
	foolproof_kmr_layout "$KMOIP_ROAMING_LAYOUT"

	update_session_id

	to_s_idle
	# NOTE:
	# There can be ONLY one thread calling @u_lm_get and @u_lm_query/@u_lm_reply
	# There can be multiple thread calling @u_lm_set
	while true; do
		event=`ipc @u_lm_get g 2>>/ipc_fail.log`
		echo -e "\nuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\n[$event]\n"
		event_loop_handler "$event"
	done

	echo ""
	echo ""
	echo ""
	echo "!!!!! ERROR: uLM IPC FAILED !!!!!"
	echo ""
	echo ""
	echo ""
}

start_ulm()
{
	cd /usr/local/bin

	# import variables from link_mgrX.sh
	if [ -f /var/lm_var ]; then
		source /var/lm_var
	fi
	if [ -f ./patch_u_lmc.sh ]; then
		source ./patch_u_lmc.sh
	fi

	# ${string##substring}: Longest Substring Match
	# /usr/local/bin/xxx.sh to xxx.sh
	echo "[${0##/*/}] hello world"

	ulm_init
	# start event loop
	event_loop &
}

start_ulm
