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
	cd build && sed -i '982 s/strip/\/opt\/crosstool\/gcc-3.4.2-glibc-2.3.3\/arm-aspeed-linux-gnu\/bin\/arm-aspeed-linux-gnu-strip/'  shtool
	cd build && sed -i '982 s/\/opt\/crosstool\/gcc-3.4.2-glibc-2.3.3\/arm-aspeed-linux-gnu\/bin\/arm-aspeed-linux-gnu-//2' shtool
	cd build && sed -i '984 s/strip/\/opt\/crosstool\/gcc-3.4.2-glibc-2.3.3\/arm-aspeed-linux-gnu\/bin\/arm-aspeed-linux-gnu-strip/' shtool
	cd build && sed -i '984 s/\/opt\/crosstool\/gcc-3.4.2-glibc-2.3.3\/arm-aspeed-linux-gnu\/bin\/arm-aspeed-linux-gnu-//2' shtool
	sed -i '23562  s/#define NEED_MEMCMP_REPLACEMENT/#define NEED_MEMCMP_REPLACEMENT_NONE/' configure 
	cd libraries/liblmdb/ && sed -i '95c #define SSIZE_MAX 2147483647' mdb.c 
	# CROSS_PREFIX=$(CROSS_COMPILE) ./configure --prefix=$(PREFIX) --host=arm-aspeed-linux-gnu --with-libs --with-zlib=$(PREFIX) --with-ssl-dir=$(PREFIX) -with-yielding-select=yes --enable-bdb
	CC=$(CROSS_COMPILE)gcc \
	CXX=$(CROSS_COMPILE)g++ \
	AR=$(CROSS_COMPILE)ar \
	RANLIB=$(CROSS_COMPILE)ranlib \
	LINK=$(CROSS_COMPILE)gcc \
	LD=$(CROSS_COMPILE)ld \
	STRIP=$(CROSS_COMPILE)strip \
	./configure --host=arm-linux  --prefix=$(PREFIX) LDFLAGS="-L$(PREFIX)/lib" CPPFLAGS="-I$(PREFIX)/include" -with-yielding-select=yes --enable-static=no --enable-slapd=no

	make depend 
	make

build:
	@#make install

install:
	# Now copy files to rootfs
	make install
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib
	cp -af ./libraries/libldap/.libs/libldap* $(ROOTFS)/lib
	cp -af ./libraries/liblber/.libs/liblber* $(ROOTFS)/lib
	cp -af ./libraries/libldap_r/.libs/libldap_r* $(ROOTFS)/lib

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"

