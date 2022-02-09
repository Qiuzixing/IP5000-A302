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
	autoreconf --install --force
	autoconf -i
	./configure --target=arm-linux --host=arm-aspeed-linux-gnu --prefix=$(PREFIX) CC=$(CROSS_COMPILE)gcc RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld LDFLAGS=-lpthread

build:
	make
	#make install

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/usr/local/bin

	@cp -af jq $(ROOTFS)/usr/local/bin

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"

.PHONY: default config build install clean
