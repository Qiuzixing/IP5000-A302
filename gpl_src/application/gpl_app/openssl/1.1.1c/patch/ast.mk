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
	make -f ast.mk Config
	make -f ast.mk build
	make -f ast.mk install

Config:
	./Configure --prefix=$(PREFIX) no-asm shared  enable-tls1_3 linux-armv4

build:
	make
	make install_sw

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib
	@mkdir -p $(ROOTFS)/bin

	@cp -af $(PREFIX)/lib/libssl.so.* $(ROOTFS)/lib
	@cp -af $(PREFIX)/lib/libcrypto.so.* $(ROOTFS)/lib

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"

