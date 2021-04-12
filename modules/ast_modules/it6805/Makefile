# Makefile for the IT6805 driver
#
# Copyright (c) 2019 ASPEED Technology Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#


include $(AST_SDK_ROOT)/ast.cfg


TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


# Please modify here or set environments.
# KOBJDIR should be pointed to the build directory of your kernel.
#
DEBUG ?= n
KOBJDIR = $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel
PWD       := $(shell pwd)
DRIVER_NAME = it6805

%.x:%.c
	arm-aspeed-linux-gnu-gcc -o $@ $<

KBUILD_VERBOSE:=0

$(DRIVER_NAME)-objs := ast_it6805.o ast_wrap.o ast_io.o iTE6805_SYS.o iTE6805_DRV.o iTE6805_EQ.o iTE6805_I2C_RDWR.o \
	iTE6805_CEC_SYS.o iTE6805_CEC_DRV.o iTE6805_CEC_FETURE.o #iTE6805_EDID.o

obj-m += $(DRIVER_NAME).o

#ifeq ($(DEBUG),y)
#	EXTRA_CFLAGS += -DCONFIG_CAT_DEBUG
#	EXTRA_CFLAGS += -DLinux_KernelMode
#endif

default:
ifeq ($(BUILD_TYPE),host)
	$(MAKE) clean
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE}  M=$(PWD) modules
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(DRIVER_NAME).ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(DRIVER_NAME).ko /tftpboot
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



