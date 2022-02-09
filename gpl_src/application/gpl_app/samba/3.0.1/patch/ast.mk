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
	cd ./source && ./configure --host=arm-linux --build=i686-linux CC=$(CROSS_COMPILE)gcc STRIP=$(CROSS_COMPILE)strip --enable-static=yes --disable-shared  samba_cv_CC_NEGATIVE_ENUM_VALUES=yes

build:
	cd ./source && make

installxx:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/usr/local/samba
	@mkdir -p $(ROOTFS)/usr/local/samba/bin
	@mkdir -p $(ROOTFS)/usr/local/samba/lib
	@mkdir -p $(ROOTFS)/usr/local/samba/var
	@mkdir -p $(ROOTFS)/usr/local/samba/share
	@mkdir -p $(ROOTFS)/usr/local/samba/private

	@cp -f ./source/bin/nmbd $(ROOTFS)/usr/local/samba/bin/
#	@cp -f ./source/bin/smbd $(ROOTFS)/usr/local/samba/bin/
	@cp -f ./smb.conf $(ROOTFS)/usr/local/samba/lib
	@$(CROSS_COMPILE)strip $(ROOTFS)/usr/local/samba/bin/nmbd
#	@$(CROSS_COMPILE)strip $(ROOTFS)/usr/local/samba/bin/smbd

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"
