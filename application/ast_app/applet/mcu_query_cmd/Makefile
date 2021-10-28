##
## Copyright (c) 2004-2016
## ASPEED Technology Inc. All Rights Reserved
## Proprietary and Confidential
##
## By using this code you acknowledge that you have signed and accepted
## the terms of the ASPEED SDK license agreement.
##
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g -lpthread -lrt -ldl


default: build
	@echo "Done"

build : clean mcu_query
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f mcu_query $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@#cp -f mcu_query /tftpboot

mcu_query : main.o	ipc.o uds_ipc.o
	$(cc) $(ldflags) -o mcu_query main.o ipc.o uds_ipc.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f mcu_query
