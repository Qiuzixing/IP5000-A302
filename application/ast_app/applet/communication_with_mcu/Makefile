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

cflags=-O3 -g	\
		-I ./dante_example_code\
		-I ./dante_example_code/common\
		-I ./dante_example_code/common/audinate\
		-I ./dante_example_code/libs/lib_ddp/include\
		-I ./dante_example_code/libs/lib_ddp/include/ddp\
		-I ./dante_example_code/libs/lib_serial/include\
		-I ./dante_example_code/libs/lib_cobs/include\
		-I ./dante_example_code/libs/lib_uhip/include\
		-I ./dante_example_code/libs/interface\
		-I ./dante_example_code/app/linux\
		-I ./dante_example_code/app/example\
		-I ./dante_example_code/dante
ldflags=-g -lpthread -lrt -ldl
 

default: build
	@echo "Done"

build : communication_with_mcu ipc_server_listen_one
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f communication_with_mcu $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/share/ch6001/
	cp -f ./*.bin $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/share/ch6001/
	cp -f ipc_server_listen_one $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

communication_with_mcu : gb_commun_with_mcu.o uds_ipc.o ipc.o auto_swtich_socket.o dante_cmd_packet.o\
					./commun_with_mcu/command_protocol.o \
					./commun_with_mcu/command.o		\
					./commun_with_mcu/uart_mcu_cmd.o \
					./commun_with_mcu/ymodem.o		\
					./dante_example_code/app/example/example_uhip_main.o\
					./dante_example_code/app/example/example_rx_uhip.o\
					./dante_example_code/app/example/example_rx_ddp.o\
					./dante_example_code/app/example/example_tx_uhip.o\
					./dante_example_code/app/example/example_tx_ddp.o\
					./dante_example_code/app/example/example_tx_pb.o\
					./dante_example_code/app/example/example_uhip_common.o\
					./dante_example_code/app/linux/rx_timer.o\
					./dante_example_code/app/linux/tx_timer.o\
					./dante_example_code/app/linux/timer_common.o\
					./dante_example_code/app/linux/uart_transport.o\
					./dante_example_code/app/linux/aardvark.o\
					./dante_example_code/libs/lib_uhip/src/uhip_packet_read.o\
					./dante_example_code/libs/lib_uhip/src/uhip_util.o\
					./dante_example_code/libs/lib_uhip/src/uhip_packet_write.o\
					./dante_example_code/libs/lib_cobs/src/cobs.o\
					./dante_example_code/libs/lib_serial/src/frame.o\
					./dante_example_code/libs/lib_ddp/src/ddp_packet_read.o\
					./dante_example_code/libs/lib_ddp/src/ddp_util_opcode.o\
					./dante_example_code/libs/lib_ddp/src/ddp_audio_write_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_network_read_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_routing_read_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_routing_write_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_clocking_util.o\
					./dante_example_code/libs/lib_ddp/src/ddp_clocking_read_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_local_read_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_clocking_write_client.o\
					./dante_example_code/libs/lib_ddp/src/routing_support.o\
					./dante_example_code/libs/lib_ddp/src/ddp_util.o\
					./dante_example_code/libs/lib_ddp/src/ddp_packet_write.o\
					./dante_example_code/libs/lib_ddp/src/ddp_routing.o\
					./dante_example_code/libs/lib_ddp/src/ddp_network_write_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_device_write_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_shared.o\
					./dante_example_code/libs/lib_ddp/src/ddp_device_read_client.o\
					./dante_example_code/libs/lib_ddp/src/ddp_audio_read_client.o\
					./dante_example_code/common/audinate/strcasecmp.o\
					./dante_example_code/common/audinate/dante_common_util.o
	$(cc) $(ldflags) -o communication_with_mcu gb_commun_with_mcu.o auto_swtich_socket.o uds_ipc.o ipc.o command_protocol.o command.o uart_mcu_cmd.o ymodem.o dante_cmd_packet.o	\
						example_uhip_main.o example_rx_uhip.o example_rx_ddp.o example_tx_uhip.o example_tx_ddp.o example_tx_pb.o example_uhip_common.o rx_timer.o tx_timer.o timer_common.o uart_transport.o \
						aardvark.o uhip_packet_read.o  uhip_util.o uhip_packet_write.o cobs.o frame.o ddp_packet_read.o ddp_util_opcode.o ddp_audio_write_client.o ddp_network_read_client.o ddp_routing_read_client.o \
						ddp_routing_write_client.o ddp_clocking_util.o ddp_clocking_read_client.o ddp_local_read_client.o ddp_clocking_write_client.o routing_support.o ddp_util.o ddp_packet_write.o ddp_routing.o \
						ddp_network_write_client.o ddp_device_write_client.o ddp_shared.o ddp_device_read_client.o ddp_audio_read_client.o strcasecmp.o dante_common_util.o

ipc_server_listen_one : ipc_server.o uds_ipc.o
	$(cc) $(ldflags) -o ipc_server_listen_one ipc_server.o uds_ipc.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f communication_with_mcu
	rm -f ipc_server_listen_one