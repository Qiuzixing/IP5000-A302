#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy lib files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib

cp -af ${PREFIX}/lib/libsysf* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }



