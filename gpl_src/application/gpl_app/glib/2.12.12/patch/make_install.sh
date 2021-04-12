#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy lib files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib

cp -af ${PREFIX}/lib/libgthread* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }
cp -af ${PREFIX}/lib/libgmodule* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }
cp -af ${PREFIX}/lib/libgobject* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }
cp -af ${PREFIX}/lib/libglib* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }

