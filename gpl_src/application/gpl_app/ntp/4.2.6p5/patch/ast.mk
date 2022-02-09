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
	make -f ast.mk installxx

config:
	./configure --prefix=$(PREFIX) --exec-prefix=$(PREFIX) --host=arm-linux --build=i686-linux CC=$(CROSS_COMPILE)gcc

build:
	make

installxx:
	make install
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/sbin
	@cp -f $(PREFIX)/bin/ntpdate $(ROOTFS)/sbin
	@$(CROSS_COMPILE)strip $(ROOTFS)/sbin/ntpdate
	@echo ntp 123/tcp >> $(ROOTFS)/etc/services
	@echo ntp 123/udp >> $(ROOTFS)/etc/services

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"
