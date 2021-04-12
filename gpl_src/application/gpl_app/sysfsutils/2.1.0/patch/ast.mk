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
	./configure --host=arm-linux --build=i686-linux CC=$(CROSS_COMPILE)gcc CXX=$(CROSS_COMPILE)g++ RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld --prefix=$(PREFIX) CFLAGS="-I$(XTOOL_PREFIX)/include" LDFLAGS="-L$(XTOOL_PREFIX)/lib"

build:
	make
	make install

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib

	cp -af $(PREFIX)/lib/libsysf* $(ROOTFS)/lib

clean:
	@[ -f Makefile ] && make distclean || echo "Nothing to clean"
