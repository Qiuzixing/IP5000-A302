#!/bin/sh

. make_define.sh

make || { echo "make failed"; exit 1; }
make install || { echo "make install failed"; exit 2; }

# Now copy files to rootfs
echo 'copy files...'
mkdir -p ${ROOTFS}/lib
mkdir -p ${ROOTFS}/etc/usbip/server.d
mkdir -p ${ROOTFS}/etc/hotplug.d/usb
cp -af ${PREFIX}/lib/libusbip* ${ROOTFS}/lib || { echo "copy library failed"; exit 3; }
cp -f ${PREFIX}/bin/bind_driver ${ROOTFS}/usr/local/bin || { echo "copy file failed"; exit 3; }
cp -f ${PREFIX}/bin/usbaid ${ROOTFS}/usr/local/bin || { echo "copy file failed"; exit 3; }

cp -f hotplug/usbip/usbip_export.conf ${ROOTFS}/etc/usbip || { echo "copy file failed"; exit 3; }
chmod a+x hotplug/usbip.hotplug
cp -f hotplug/usbip.hotplug ${ROOTFS}/etc/hotplug.d/usb || { echo "copy file failed"; exit 3; }
# This is the file from "hotplug" open source code
chmod a+x hotplug/sbin/hotplug
cp -f hotplug/sbin/hotplug ${ROOTFS}/sbin || { echo "copy file failed"; exit 3; }

