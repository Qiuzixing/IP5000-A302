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
ldflags=-g


default: build
	@echo "Done"

build : clean ipc ipc_server
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ipc $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ipc /tftpboot
	cp -f ipc_server $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ipc_server /tftpboot

ipc : ipc.o uds_ipc.o
	$(cc) $(ldflags) -o ipc ipc.o uds_ipc.o

ipc_server: ipc_server.o uds_ipc.o
	$(cc) $(ldflags) -o ipc_server ipc_server.o uds_ipc.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f ipc
	rm -f ipc_server
