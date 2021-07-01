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
ldflags=-g -lpthread -lrt
 

default: build
	@echo "Done"

build : communication_with_mcu ipc_server_listen_one
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f communication_with_mcu $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/share/ch6001/
	cp -f ./*.bin $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/share/ch6001/
	cp -f ipc_server_listen_one $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

communication_with_mcu : gb_commun_with_mcu.o uds_ipc.o ipc.o \
					./commun_with_mcu/command_protocol.o \
					./commun_with_mcu/command.o		\
					./commun_with_mcu/uart_mcu_cmd.o \
					./commun_with_mcu/ymodem.o		
	$(cc) $(ldflags) -o communication_with_mcu gb_commun_with_mcu.o uds_ipc.o ipc.o command_protocol.o command.o uart_mcu_cmd.o ymodem.o

ipc_server_listen_one : ipc_server.o uds_ipc.o
	$(cc) $(ldflags) -o ipc_server_listen_one ipc_server.o uds_ipc.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f communication_with_mcu
	rm -f ipc_server_listen_one