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
ldflags=-g -lrt


default: build
	@echo "Done"

build : heartbeat
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f heartbeat $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f heartbeat /tftpboot

heartbeat: heartbeat.o uds_ipc.o astnetwork.o multi_timer.o ipc.o
	$(cc) $(ldflags) -o heartbeat heartbeat.o uds_ipc.o astnetwork.o multi_timer.o ipc.o

%.o : %.c Makefile *.h
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f heartbeat
