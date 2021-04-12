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
	@cp -f mk.cache bruce.cache
	./configure --host=arm-linux --build=i686-linux CC=$(CROSS_COMPILE)gcc RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld --prefix=$(PREFIX) CFLAGS="-I$(XTOOL_PREFIX)/include -L$(XTOOL_PREFIX)/lib" --cache-file=bruce.cache


build_:
	make
	make install

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib

	cp -af $(PREFIX)/lib/libgthread* $(ROOTFS)/lib
	cp -af $(PREFIX)/lib/libgmodule* $(ROOTFS)/lib
	cp -af $(PREFIX)/lib/libgobject* $(ROOTFS)/lib
	cp -af $(PREFIX)/lib/libglib* $(ROOTFS)/lib

clean:
	@[ -f Makefile ] && make distclean && rm -f bruce.cache || echo "Nothing to clean"
