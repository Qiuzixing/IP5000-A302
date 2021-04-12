#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib
cp -af ${PREFIX}/lib/libnss_mdns* ${ROOTFS}/lib || { echo "copy library to /lib failed"; exit 3; }
