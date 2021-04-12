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

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ = ast_heartbeat.o astnetwork.o

default: build
	@echo "Done"

build: ast_heartbeat
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ast_heartbeat $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ast_heartbeat_no_tcp
	@#cp -f ast_heartbeat /tftpboot/ast_heartbeat_no_tcp

ast_heartbeat : $(OBJ)
	$(cc) $(ldflags) -o ast_heartbeat $(OBJ)

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f ast_heartbeat