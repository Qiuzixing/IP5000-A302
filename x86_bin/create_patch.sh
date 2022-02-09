#!/bin/bash

###################################################################################################################
# This script will create a <patch_dir> directory and put all modified files from <modified_dir> directory.
# The difference is created by comparing with <original_dir>.
# Blame Bruce for this script.
#
# Usage:
#       create_patch.sh <original_dir> <modified_dir> <patch_dir>
#
# Note:
#       - Empty folder will be ignored.
#       - Empty file will be ignored.
#       - Binary file will be ignored.
#       - Folder name with space characters works OK.
#       - File name with space characters works OK.
#       - Symbolic file works OK.
#       - Hiden files (file name starts with '.') works.
###################################################################################################################

# uncomment for debug
#set -x

ORIGINAL_DIR="$1"
MODIFIED_DIR="$2"
PATCH_DIR="$3"
DIR_DEPTH=${4:-'1'} #directory depth used for patch program. default value is '1'.
MY_PWD=`pwd`
_rtn='0'

echo "Creating patch from $ORIGINAL_DIR vs $MODIFIED_DIR to $PATCH_DIR"
cd "$MY_PWD"

# First cleanup the patch folder
rm -rf "$PATCH_DIR"
mkdir -p "$PATCH_DIR"

# To seperate output one by one line, set IFS to '\n'
OLD_IFS="$IFS"
IFS="
"
# Use diff to create patch and 'dry-applying' the patch to get a 'easy to parse' diff file list one by one line.
echo "Comparing diff..."
rm -f "$MY_PWD"/p.patch
diff -Nur "$ORIGINAL_DIR" "$MODIFIED_DIR" > "$MY_PWD"/p.patch
for line_out in `patch -d "$ORIGINAL_DIR" -p$DIR_DEPTH --dry-run -i "$MY_PWD"/p.patch`; do
	#echo "$line_out"
	#Each $line_out will looks like "patching file include/configs/ast1520c.h"
	case "$line_out" in
		"patching file "*)
			# Now, parse the $line_out and get the diff file name and folder.
			# patching file include/configs/ast1520c.h
			the_diff=`expr "$line_out" : 'patching file \(.*\)'` || exit 1
			# Use tar to copy the file to the PATCH_DIR
			if ! [ -e "$MODIFIED_DIR"/`eval echo -n "${the_diff}"` ]; then echo "!!!!! not exists. Could be a file only exists on GPL source. Ignore. ";continue; fi
			echo "Copying $MODIFIED_DIR/$the_diff"
			# The `eval echo -n "${the_diff}"` tricky is used to support file name with space.
			tar c -C "$MODIFIED_DIR" `eval echo -n "${the_diff}"` | tar x -C "$PATCH_DIR"
			if [ "${PIPESTATUS[0]}" != '0' ] || ([ -n "${PIPESTATUS[1]}" ] && [ "${PIPESTATUS[1]}" != '0' ]);then
				echo "!!!!!ERROR!!!!"
				_rtn='1'
				break
			fi
			
		;;
		"checking file "*)
			# Now, parse the $line_out and get the diff file name and folder.
			# patching file include/configs/ast1520c.h
			the_diff=`expr "$line_out" : 'checking file \(.*\)'` || exit 1
			# Use tar to copy the file to the PATCH_DIR
			if ! [ -e "$MODIFIED_DIR"/`eval echo -n "${the_diff}"` ]; then echo "!!!!! not exists. Could be a file only exists on GPL source. Ignore. ";continue; fi
			echo "Copying $MODIFIED_DIR/$the_diff"
			# The `eval echo -n "${the_diff}"` tricky is used to support file name with space.
			tar c -C "$MODIFIED_DIR" `eval echo -n "${the_diff}"` | tar x -C "$PATCH_DIR"
			if [ "${PIPESTATUS[0]}" != '0' ] || ([ -n "${PIPESTATUS[1]}" ] && [ "${PIPESTATUS[1]}" != '0' ]);then
				echo "!!!!!ERROR!!!!"
				_rtn='1'
				break
			fi
		;;
		*"is not a regular file"*)
			echo "ignore $line_out"
		;;
		"1 out of 1 hunk ignored")
			echo "ignore $line_out"
		;;
		*)
			echo "!!!!!!! WTF !!!!!!!!!!"
			_rtn='1'
			break
		;;
	esac
done

IFS="$OLD_IFS"
rm -f "$MY_PWD"/p.patch

exit "$_rtn"

