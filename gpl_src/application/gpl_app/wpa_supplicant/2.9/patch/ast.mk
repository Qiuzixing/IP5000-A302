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
	cp wpa_supplicant/defconfig wpa_supplicant/.config 
	sed -i '29 s/CONFIG_DRIVER_WEXT=y/#CONFIG_DRIVER_WEXT=y/' wpa_supplicant/.config
	sed -i '32 s/CONFIG_DRIVER_NL80211=y/#CONFIG_DRIVER_NL80211=y/' wpa_supplicant/.config
	sed -i '370 s/CONFIG_CTRL_IFACE_DBUS_NEW/#CONFIG_CTRL_IFACE_DBUS_NEW/' wpa_supplicant/.config
	sed -i '373 s/CONFIG_CTRL_IFACE_DBUS_INTRO=y/#CONFIG_CTRL_IFACE_DBUS_INTRO=y/' wpa_supplicant/.config
	sed -i '509 s/CONFIG_AP=y/#CONFIG_AP=y/' wpa_supplicant/.config
	sed -i '514 s/CONFIG_P2P=y/#CONFIG_P2P=y/' wpa_supplicant/.config 
	sed -i '522 s/CONFIG_WIFI_DISPLAY=y/#CONFIG_WIFI_DISPLAY=y/' wpa_supplicant/.config

	sed -i '17i\CFLAGS += -I $(PREFIX)\/include' wpa_supplicant/.config  
	sed -i '18i\CFLAGS += -I $(PREFIX)\/include\/libnl3\/netlink' wpa_supplicant/.config
	sed -i '19i\LIBS += -L $(PREFIX)\/lib' wpa_supplicant/.config
	sed -i '20i\LIBS_p += -L $(PREFIX)\/lib' wpa_supplicant/.config
	sed -i '21i\CC = $(CROSS_COMPILE)gcc' wpa_supplicant/.config
	sed -i '22i\AR = $(CROSS_COMPILE)ar' wpa_supplicant/.config
	sed -i '23i\LD = $(CROSS_COMPILE)ld' wpa_supplicant/.config
	
	#cd $(PREFIX)/include/libnl3/netlink/ && (mv errno.h errno2.h || cd $(PREFIX)/include/libnl3/netlink/)
	#cd $(PREFIX)/include/libnl3/netlink/ && sed -i '31 s/errno.h/errno2.h/' netlink.h
	#cd $(PREFIX)/include/libnl3/netlink/ && sed -i '31 s/errno2.h//2' netlink.h

build:
	make -C wpa_supplicant
	make -C wpa_supplicant install DESTDIR=$(PREFIX)
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/lib
	cp src/common/wpa_ctrl.h  $(PREFIX)/include 
	cp -af wpa_supplicant/libwpa_client.so $(PREFIX)/lib

install:
	# Now copy files to rootfs
	@echo 'copy files...'
	@mkdir -p $(ROOTFS)/lib
	@mkdir -p $(ROOTFS)/usr/local/bin
	cp -af wpa_supplicant/libwpa_client.so $(ROOTFS)/lib
	cp -af wpa_supplicant/wpa_supplicant_802_1x $(ROOTFS)/usr/local/bin

clean:
	@[ -f Makefile ] && make distclean || echo "Nothing to clean"

