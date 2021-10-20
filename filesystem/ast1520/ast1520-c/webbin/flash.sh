#!/bin/sh

HTML_INFO="/www/update_fw_info.txt"
HTML_UPGRADE_STATUS="/www/fw_status.txt"

FW_PATH="/dev/shm"
FW_KERNEL="mtdblkkernel"
FW_ROOTFS="mtdblkrootfs"
FW_KERNEL_DEV="mtdblkkernel2"
FW_ROOTFS_DEV="mtdblkrootfs2"
FILE_THIS="flash.sh"
BLOCKSIZE=65536

cursys=`astparam misc g cursys`
if [ "$cursys" == "b" ]; then
    FW_KERNEL_DEV="mtdblkkernel"
    FW_ROOTFS_DEV="mtdblkrootfs"
else 
    FW_KERNEL_DEV="mtdblkkernel2"
    FW_ROOTFS_DEV="mtdblkrootfs2"
fi
echo "FW_KERNEL_DEV=$FW_KERNEL_DEV"
echo "FW_ROOTFS_DEV=$FW_ROOTFS_DEV"

total_fw_size()
{
	fsize='0'
	if [ -f "$FW_PATH/$FW_KERNEL" ]; then
		set -- `ls -l $FW_PATH/$FW_KERNEL`
		fsize=`expr $fsize + $5`
	fi
	if [ -f "$FW_PATH/$FW_ROOTFS" ]; then
		set -- `ls -l $FW_PATH/$FW_ROOTFS`
		fsize=`expr $fsize + $5`
	fi
	echo "$fsize"
}

html_info()
{
	echo "$1" >> $HTML_INFO
}

update_fw_status()
{
	local _status=$1
	local _remain_size=$2
	local _errno=$3
	local _progress=0
	if [ $fw_total_size -lt 0 ]; then
		echo "err,$_progress,2" > $HTML_UPGRADE_STATUS.tmp
		mv $HTML_UPGRADE_STATUS.tmp $HTML_UPGRADE_STATUS
		exit 2
	fi
	_progress=$(( ($fw_total_size - $_remain_size) * 100 / $fw_total_size))
	echo "$_status,$_progress,$_errno" > $HTML_UPGRADE_STATUS.tmp
	mv $HTML_UPGRADE_STATUS.tmp $HTML_UPGRADE_STATUS
}

fail_out()
{
	local _errno=$1
	# Remove temp files
	rm -f $FW_PATH/$FW_KERNEL 2> /dev/null
	rm -f $FW_PATH/$FW_ROOTFS 2> /dev/null
	rm -f $FW_PATH/$FILE_THIS 2> /dev/null

	html_info "Programming FAILED!"
	update_fw_status "err" `total_fw_size` $_errno
	exit 1
}

copy_file()
{
	# options: if, of, fw_size_remain
	local if=$1
	local of=$2
	local fw_size_remain=$3
	set -- `ls -l $FW_PATH/$if`
	local fsize=$5
	local count=0
	local total_count=$(( ($fsize + $BLOCKSIZE - 1) / $BLOCKSIZE ))
	while [ $count -lt $total_count ]; do
		if ! dd if=$if of=$of bs=$BLOCKSIZE seek=$count skip=$count count=1; then
			fail_out $?
		else
			count=$(($count +1))
			if [ $count -lt $total_count ]; then
				# last block maybe not same as $BLOCKSIZE, so update it outside when checking all
				fw_size_remain=$(($fw_size_remain - $BLOCKSIZE))
				update_fw_status "ongoing" $fw_size_remain 0
			fi
		fi
	done
}

_p=`/usr/local/bin/io 0 1e6e207c`
_IFS="$IFS";IFS=' ';set -- $_p;IFS="$_IFS"
_p=$3
case "$_p" in
	302)
		# AST1500
		SOC_VER='1'
	;;
	1000003 | 1010303 | 1010203 | 2010303)
		#1000003  AST1510 A0
		#1010303  AST1510 A1
		#1010203  AST1510 A1
		#2010303  AST1510 FPGA
		SOC_VER='2'
	;;
	2010003 | 4000003 | 4000303 | 4010303 | 4??1003)
		#2010003  AST1520 FPGA
		#4000003  AST1520 FPGA
		#4000303  AST1520 A0
		#4010303  AST2500 A1
		#4011003  AST1520 A1
		#4031003  AST1520 A2
		SOC_VER='3'
	;;
	*)
		html_info "Error!!! Unknown SoC version? $_p"
		fail_out 1
	;;
esac

if ! [ "$SOC_VER" = '3' ]; then
	html_info "Error! SoC version mismatch! ($SOC_VER)"
	fail_out 1
fi

# Check Platform
case "$AST_PLATFORM" in
	#New firmware uses 'ast1520-c-v1'.
	#We have to support both and check 'host'/'client'.
	ast1520-c-v?)
		html_info "Platform matched."
	;;
	*)
		html_info "Error! Platform mismatch!"
		fail_out 1
esac

remain_fw_size=`total_fw_size`
fw_total_size=$remain_fw_size
# create json file
update_fw_status "ongoing" $remain_fw_size 0

# ToDo. Check version.
# Start update
html_info "Start programming flash..."
if [ -f "$FW_KERNEL" ]; then
	html_info "programming kernel..."
	if ! [ -e /dev/"$FW_KERNEL_DEV" ]; then
		mknod /dev/"$FW_KERNEL_DEV" b 31 1
	fi

	if ! copy_file "$FW_KERNEL" /dev/"$FW_KERNEL_DEV" "$remain_fw_size"; then
		fail_out 255
	else
		rm -f "$FW_KERNEL"
	fi
fi
remain_fw_size=`total_fw_size`
update_fw_status "ongoing" $remain_fw_size 0
if [ -f "$FW_ROOTFS" ]; then
	html_info "programming rootfs..."
	if ! [ -e /dev/"$FW_ROOTFS_DEV" ]; then
		mknod /dev/"$FW_ROOTFS_DEV" b 31 2
	fi

	if ! copy_file "$FW_ROOTFS" /dev/"$FW_ROOTFS" "$remain_fw_size"; then
		fail_out 255
	else
		rm -f "$FW_ROOTFS"
	fi
fi
remain_fw_size=`total_fw_size`
update_fw_status "ongoing" $remain_fw_size 0

if [ "$cursys" == "b" ]; then
    astparam misc s cursys a
else
    astparam misc s cursys b
fi

update_fw_status "ok" `total_fw_size` 0

html_info "Programming completed"
exit 0;

