#!/bin/sh

HTML_INFO="/www/update_fw_info.txt"
HTML_FW_SIZE_REMAIN="/www/fw_size_remain.js"

FW_PATH="/dev/shm"
FW_BOOT="mtdblkboot"
FW_KERNEL="mtdblkkernel"
FW_ROOTFS="mtdblkrootfs"
FW_KERNEL_DEV="mtdblkkernel2"
FW_ROOTFS_DEV="mtdblkrootfs2"
FW_LOGO="mtdblklogo"
FILE_FLAGS="flags.sh"
FILE_THIS="flash.sh"

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
	# if [ -f "$FW_PATH/$FW_BOOT" ]; then
	# 	set -- `ls -l $FW_PATH/$FW_BOOT`
	# 	#echo "$FW_BOOT size $5 B"
	# 	fsize=`expr $fsize + $5`
	# fi
	if [ -f "$FW_PATH/$FW_KERNEL" ]; then
		set -- `ls -l $FW_PATH/$FW_KERNEL`
		#echo "$FW_KERNEL size $5 B"
		fsize=`expr $fsize + $5`
	fi
	if [ -f "$FW_PATH/$FW_ROOTFS" ]; then
		set -- `ls -l $FW_PATH/$FW_ROOTFS`
		#echo "$FW_ROOTFS size $5 B"
		fsize=`expr $fsize + $5`
	fi
	# if [ -f "$FW_PATH/$FW_LOGO" ]; then
	# 	set -- `ls -l $FW_PATH/$FW_LOGO`
	# 	#echo "$FW_LOGO size $5 B"
	# 	fsize=`expr $fsize + $5`
	# fi
	echo "$fsize"
}

html_info()
{
	echo "$1" >> $HTML_INFO
}

html_set_fw_size_remain()
{
	#print "Content-type: application/x-javascript\n\n" > $HTML_FW_SIZE_REMAIN
	echo "FWSizeRemain = $1;" > $HTML_FW_SIZE_REMAIN
}

fail_out()
{
	# Remove temp files
	rm -f $FW_PATH/$FW_BOOT 2> /dev/null
	rm -f $FW_PATH/$FW_KERNEL 2> /dev/null
	rm -f $FW_PATH/$FW_ROOTFS 2> /dev/null
	rm -f $FW_PATH/$FW_LOGO 2> /dev/null
	rm -f $FW_PATH/$FILE_FLAGS 2> /dev/null
	rm -f $FW_PATH/$FILE_THIS 2> /dev/null

	html_info "Programming FAILED!"
	exit 1
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
		fail_out
	;;
esac

if ! [ "$SOC_VER" = '3' ]; then
	html_info "Error! SoC version mismatch! ($SOC_VER)"
	fail_out
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
		fail_out
esac

# ToDo. Check version.
# Start update
html_info "Start programming flash..."
# if [ -f "$FW_BOOT" ]; then
# 	html_info "programming bootloader..."
# 	if ! [ -e /dev/"$FW_BOOT" ]; then
# 		mknod /dev/"$FW_BOOT" b 31 0
# 	fi
# 	if ! dd if="$FW_BOOT" of=/dev/"$FW_BOOT" bs=64k; then
# 		fail_out
# 	else
# 		rm -f "$FW_BOOT"
# 	fi
# fi
# html_set_fw_size_remain `total_fw_size`
if [ -f "$FW_KERNEL" ]; then
	html_info "programming kernel..."
	if ! [ -e /dev/"$FW_KERNEL_DEV" ]; then
		mknod /dev/"$FW_KERNEL_DEV" b 31 1
	fi
	if ! dd if="$FW_KERNEL" of=/dev/"$FW_KERNEL_DEV" bs=64k; then
		fail_out
	else
		rm -f "$FW_KERNEL"
	fi
fi
html_set_fw_size_remain `total_fw_size`
if [ -f "$FW_ROOTFS" ]; then
	html_info "programming rootfs..."
	if ! [ -e /dev/"$FW_ROOTFS_DEV" ]; then
		mknod /dev/"$FW_ROOTFS_DEV" b 31 2
	fi
	if ! dd if="$FW_ROOTFS" of=/dev/"$FW_ROOTFS_DEV" bs=64k; then
		fail_out
	else
		rm -f "$FW_ROOTFS"
	fi
fi
html_set_fw_size_remain `total_fw_size`
# if [ -f "$FW_LOGO" ]; then
# 	html_info "programming logo..."
# 	if ! [ -e /dev/"$FW_LOGO" ]; then
# 		mknod /dev/"$FW_LOGO" b 31 4
# 	fi
# 	if ! dd if="$FW_LOGO" of=/dev/"$FW_LOGO" bs=64k; then
# 		fail_out
# 	else
# 		rm -f "$FW_LOGO"
# 	fi
# fi
# if [ -f "$FILE_FLAGS" ]; then
# 	html_info "programming parameters..."
# 	chmod a+x "$FILE_FLAGS"
# 	./"$FILE_FLAGS"
# 	rm -f "$FILE_FLAGS"
# fi
# html_set_fw_size_remain `total_fw_size`

if [ "$cursys" == "b" ]; then
    astparam misc s cursys a
else
    astparam misc s cursys b
fi

html_info "Programming completed"
exit 0;

