# Makefile for the IT6802 driver
#
#
# Copyright (c) 2004-2012
# ASPEED Technology Inc. All Rights Reserved
# Proprietary and Confidential
#
# By using this code you acknowledge that you have signed and accepted
# the terms of the ASPEED SDK license agreement.
#

include $(AST_SDK_ROOT)/ast.cfg


TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


# Please modify here or set environments.
# KOBJDIR should be pointed to the build directory of your kernel.
#
DEBUG ?= n
KOBJDIR = $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel


PWD       := $(shell pwd)

%.x:%.c
	arm-aspeed-linux-gnu-gcc -o $@ $<

KBUILD_VERBOSE:=0

it6802-objs 	  := ast_main.o ast_wrap.o IO.o it680x.o it680x_cec.o

obj-m += it6802.o

#ifeq ($(DEBUG),y)
#	EXTRA_CFLAGS += -DCONFIG_CAT_DEBUG
#	EXTRA_CFLAGS += -DLinux_KernelMode
#endif

default:
ifeq ($(BUILD_TYPE),host)
	$(MAKE) clean
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE}  M=$(PWD) modules
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f it6802.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f it6802.ko /tftpboot
else
	@echo "Not supported"
endif

check:
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules

.PHONY: cscope
cscope:
	cscope -b -k -R

.PHONY: clean
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions



