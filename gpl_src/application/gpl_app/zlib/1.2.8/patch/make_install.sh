#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib
mkdir -p ${ROOTFS}/bin

cp -af ${PREFIX}/lib/libz.so.* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }
#cp -f ${PREFIX}/bin/xmlwf ${ROOTFS}/bin || { echo "copy binary failed"; exit 4; }
