#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy lib files to rootfs
#echo 'copy files...'

cp -af ./_install/. ${ROOTFS}/ || { echo "copy file failed"; exit 3; }



