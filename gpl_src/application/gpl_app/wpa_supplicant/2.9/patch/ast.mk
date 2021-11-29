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
	sed -i '17i\CFLAGS += -I $(PREFIX)\/include' wpa_supplicant/.config  
	sed -i '19i\LIBS += -L $(PREFIX)\/lib' wpa_supplicant/.config
	sed -i '20i\LIBS_p += -L $(PREFIX)\/lib' wpa_supplicant/.config
	sed -i '21i\CC = $(CROSS_COMPILE)gcc' wpa_supplicant/.config
	sed -i '22i\AR = $(CROSS_COMPILE)ar' wpa_supplicant/.config
	sed -i '23i\LD = $(CROSS_COMPILE)ld' wpa_supplicant/.config
	
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

