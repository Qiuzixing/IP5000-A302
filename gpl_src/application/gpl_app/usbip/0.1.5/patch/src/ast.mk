##
## Copyright (c) 2004-2012
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix
ROOTFS=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon


default:
	make -f ast.mk clean
	make -f ast.mk config
	make -f ast.mk build
	make -f ast.mk install

config:
	./autogen.sh
	cp -f mk.cache bruce.cache
	./configure --host=arm-linux --build=i686-linux CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld --prefix=$(PREFIX) CFLAGS="-I$(XTOOL_PREFIX)/include -I$(PREFIX)/include" LDFLAGS="-L$(XTOOL_PREFIX)/lib -L$(PREFIX)/lib" --cache-file=bruce.cache

build:
	make
	make install

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib
	@mkdir -p $(ROOTFS)/etc/usbip/server.d
	@mkdir -p $(ROOTFS)/etc/hotplug.d/usb
	@mkdir -p $(ROOTFS)/usr/local/bin
	@mkdir -p $(ROOTFS)/sbin

	cp -af $(PREFIX)/lib/libusbip* $(ROOTFS)/lib
	cp -f $(PREFIX)/bin/bind_driver $(ROOTFS)/usr/local/bin
	cp -f $(PREFIX)/bin/usbaid $(ROOTFS)/usr/local/bin

	cp -f hotplug/usbip/usbip_export.conf $(ROOTFS)/etc/usbip
	chmod a+x hotplug/usbip.hotplug
	cp -f hotplug/usbip.hotplug $(ROOTFS)/etc/hotplug.d/usb
	# This is the file from "hotplug" open source code
	chmod a+x hotplug/sbin/hotplug
	cp -f hotplug/sbin/hotplug $(ROOTFS)/sbin

clean:
	@[ -f Makefile ] && make maintainer-clean && rm -f bruce.cache || echo "Nothing to clean"
