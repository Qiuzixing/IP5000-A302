#!/bin/bash

####################################################################
# Blame Bruce for creating this script.
#
# Environment variables required from caller:
#
# FLASH_SIZE="16"               # in MB
# ALL_IMAGE="all.bin"           # image file name
# IMAGE_FOLDER="${AST_SDK_ROOT}/images/ast1510-h"  # Where source files come from
# IMG_PARTITION= see below. Define the image partition and source files
#
# IMG_PARTITION Define:
#                Start_From(K)   Block_Cnt(K)   File_Name
#-------------------------------------------------------------
#IMG_PARTITION="\
#                0               512            u-boot.bin, \
#                512             3584           uImage, \
#                4096            10240          $(INITRDM_IMAGE), \
#                14336           1856           $(ROOTFS_PATCH_IMAGE), \
#                16128           128            $(LOGO_IMAGE) \
#"
#
# Return:
#    TRUE on success. Resulting image ($ALL_IMAGE) will be moved back to $IMAGE_FOLDER
#    FALSE on fail.

function do_fail()
{
	rm -f ${ALL_IMAGE}
	exit 1
}

# Validate input first
[ -n "${FLASH_SIZE}" ] && \
[ -n "${ALL_IMAGE}" ] && \
[ -n "${IMAGE_FOLDER}" ] && \
[ -n "${IMG_PARTITION}" ] || \
{ echo "ERROR!! Missing input?!";do_fail; }

# Initialization
max_count=`expr ${FLASH_SIZE} \* 1024`
rm -f ${ALL_IMAGE}
dd if=/dev/zero of=${ALL_IMAGE} bs=1K count=${max_count} 2>/dev/null

# Parse IMG_PARTITION into shell variables $1,$2,... line by line
_IFS="$IFS";IFS=',';set -- ${IMG_PARTITION};IFS="$_IFS"

# Create image line by line
for partition do
	# Parse one line of the partition table
	base_addr=`expr "$partition" : '[[:space:]]\{0,\}\([[:digit:]]\{1,\}\)[[:space:]]\{1,\}[[:digit:]]\{1,\}[[:space:]]\{1,\}[[:graph:]]\{1,\}[[:space:]]\{0,\}'`
	block_cnt=`expr "$partition" : '[[:space:]]\{0,\}[[:digit:]]\{1,\}[[:space:]]\{1,\}\([[:digit:]]\{1,\}\)[[:space:]]\{1,\}[[:graph:]]\{1,\}[[:space:]]\{0,\}'`
	file_name=`expr "$partition" : '[[:space:]]\{0,\}[[:digit:]]\{1,\}[[:space:]]\{1,\}[[:digit:]]\{1,\}[[:space:]]\{1,\}\([[:graph:]]\{1,\}\)[[:space:]]\{0,\}'`

	echo "START_FROM=$base_addr,			BLOCK_CNT=$block_cnt,				FILE_NAME=$file_name"

	# Validate Partition items
	[ -n "${base_addr}" ] || { echo "ERROR!! Partition table (IMG_PARTITION) has invalid 'Start_From' ?!";do_fail; }
	[ -n "${block_cnt}" ] || { echo "ERROR!! Partition table (IMG_PARTITION) has invalid 'Block_Cnt' ?!";do_fail; }
	[ -n "${file_name}" ] || { echo "ERROR!! Partition table (IMG_PARTITION) has invalid 'File_Name' ?!";do_fail; }

	# Check file existense
	if ! [ -f ${IMAGE_FOLDER}/${file_name} ]; then
		echo "ERROR!! ${IMAGE_FOLDER}/${file_name} doesn't exists!!"
		do_fail
	fi
	# Validate file size. Not sure filename =?= image file?
	max_size=`expr ${block_cnt} \* 1024`
	if ! check_size.sh ${IMAGE_FOLDER}/${file_name} ${max_size}; then
		do_fail
	fi

	dd if=${IMAGE_FOLDER}/${file_name} of=${ALL_IMAGE} bs=1K seek=${base_addr} count=${block_cnt} conv=notrunc || { echo "ERROR!! dd error?!";do_fail; }
done

# Done. Move $ALL_IMAGE back to $IMAGE_FOLDER
mv -f ${ALL_IMAGE} ${IMAGE_FOLDER}/ || { echo "ERROR!! Move ${ALL_IMAGE} to ${IMAGE_FOLDER}/ error?!";do_fail; }

