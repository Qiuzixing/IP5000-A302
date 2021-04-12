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
	# avahi config is buggy on 'install to prefix'. Do 'make clean' per build can resolve.
	make -f ast.mk clean
	make -f ast.mk config
	make -f ast.mk build
	make -f ast.mk install

config:
	( \
		export PKG_CONFIG_PATH="$(PREFIX)/lib/pkgconfig"; \
		./configure --host=arm-linux --build=i686-linux --prefix=$(PREFIX) --disable-dbus --disable-gdbm --disable-glib --disable-gobject --disable-gtk --disable-manpages --disable-monodoc --disable-python --disable-qt3 --disable-qt4 --enable-libdaemon --with-distro=none --with-xml=expat --with-avahi-user=root --with-avahi-group=root --with-autoipd-user=root --with-autoipd-group=root CC=$(CROSS_COMPILE)gcc RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld CXX=$(CROSS_COMPILE)c++ CPPFLAGS="-I$(XTOOL_PREFIX)/include" LDFLAGS="-L$(XTOOL_PREFIX)/lib" LIBDAEMON_CFLAGs="-I$(PREFIX)/include/libdaemon" || exit 1; \
	) || exit 1
	#./configure --host=arm-linux --build=i686-linux --prefix=$(PREFIX) --sysconfdir=/etc --localstatedir=/var --disable-dbus --disable-gdbm --disable-glib --disable-gobject --disable-gtk --disable-manpages --disable-monodoc --disable-python --disable-qt3 --disable-qt4 --enable-libdaemon --with-distro=none --with-xml=expat --with-avahi-user=root --with-avahi-group=root --with-autoipd-user=root --with-autoipd-group=root CC=$(CROSS_COMPILE)gcc RANLIB=$(CROSS_COMPILE)ranlib LD=$(CROSS_COMPILE)ld CXX=$(CROSS_COMPILE)c++ CPPFLAGS="-I$(XTOOL_PREFIX)/include" LDFLAGS="-L$(XTOOL_PREFIX)/lib" LIBDAEMON_CFLAGs="-I$(PREFIX)/include/libdaemon" || exit 1; \

build:
	make prefix=""
	make install

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib/pkgconfig
	@mkdir -p $(ROOTFS)/sbin
	@mkdir -p $(ROOTFS)/share

	cp -af $(PREFIX)/lib/libavahi* $(ROOTFS)/lib
	#cp -af $(PREFIX)/lib/pkgconfig/avahi* $(ROOTFS)/lib/pkgconfig
	cp -arf $(PREFIX)/lib/avahi $(ROOTFS)/lib
	cp -f $(PREFIX)/sbin/avahi* $(ROOTFS)/sbin
	#cp -rf /etc/avahi $(rootfs)/etc
	cp -rf $(PREFIX)/share/avahi $(ROOTFS)/share

clean:
	[ -f Makefile ] && make distclean || echo "Nothing to clean"

