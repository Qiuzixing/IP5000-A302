#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib/pkgconfig
cp -af ${PREFIX}/lib/libdaemon* ${ROOTFS}/lib || { echo "copy library to /lib failed"; exit 3; }
cp -af ${PREFIX}/lib/pkgconfig/libdaemon* ${ROOTFS}/lib/pkgconfig || { echo "copy library to /lib/pkgconfig library failed"; exit 4; }
