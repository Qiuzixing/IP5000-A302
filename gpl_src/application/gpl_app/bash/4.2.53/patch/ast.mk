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
	make -f ast.mk build_
	make -f ast.mk install

config:
	## Error due to script can't test some parameters while cross compiling
	## See http://library.gnome.org/devel/glib/unstable/glib-cross-compiling.html
	## Let's create the "cache file"
	## [root@localhost glib-2.12.12]# vi bruce.cache
	## glib_cv_stack_grows=no
	## glib_cv_uscore=no
	## ac_cv_func_posix_getpwuid_r=no
	./configure --host=arm-linux --build=i686-linux --prefix=$(PREFIX) --enable-minimal-config --enable-alias --enable-array-variables --enable-prompt-string-decoding --enable-readline --without-bash-malloc --enable-dparen-arithmetic --enable-arith-for-command --enable-cond-command --enable-extended-glob

build_:
	make
	make -C examples/loadables sleep cat
	make install

install:
	arm-linux-strip --strip-unneeded bash
	# Now copy files to rootfs
	@echo 'copy files...'
	cp -af bash $(ROOTFS)/bin/
	rm -f $(ROOTFS)/bin/sh
	ln -s bash $(ROOTFS)/bin/sh
	@echo 'copy loadables...'
	mkdir -p $(ROOTFS)/usr/local/bin/bash/
	cp -af examples/loadables/sleep $(ROOTFS)/usr/local/bin/bash/
	cp -af examples/loadables/cat $(ROOTFS)/usr/local/bin/bash/

clean:
	@[ -f Makefile ] && make distclean || echo "Nothing to clean"
