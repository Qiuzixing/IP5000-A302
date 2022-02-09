# Makefile for the I2C driver
#
# $Id: Makefile 14 2007-06-18 12:20:03Z hirofuchi $
#
# Copyright (C) 2003-2007 Takahiro Hirofuchi
#
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
# USA.
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

cat6023-objs 	  := hdmirx_main.o csc.o hdmirx.o hdmirx_repeater.o io.o sha1.o

obj-m += cat6023.o

ifeq ($(DEBUG),y)
	EXTRA_CFLAGS += -DCONFIG_CAT_DEBUG
	EXTRA_CFLAGS += -DLinux_KernelMode
endif

default:
ifeq ($(BUILD_TYPE),host)
	$(MAKE) clean
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE}  M=$(PWD) modules
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f cat6023.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@#cp -f cat6023.ko /tftpboot
else
	@echo "Not supported"
endif

#host:
#	$(MAKE) T=host default
#
#client:
#	#$(MAKE) KOBJDIR=../../kernel/client default
#	#cp -f cat6023.ko ../drivers/host
#	#cp -f cat6613.ko ../drivers/client

check:
	#$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules checkstack namespacecheck headers_check
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules

.PHONY: cscope
cscope:
	cscope -b -k -R

.PHONY: clean
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions



