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


CC=$(CROSS_COMPILE)gcc

default: build
	@echo "Done"

build: io
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f io $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f io_scratch $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

io: io.o io_scratch.o
	$(CC) -o io io.o -lm
	$(CC) -o io_scratch io_scratch.o -lm

clean:
	rm -rf *.o io io_scratch

