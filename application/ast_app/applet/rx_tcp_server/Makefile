
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ = rx_tcp_server.o create_socket.o

default: build
	@echo "Done"

build: rx_tcp_server
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f rx_tcp_server $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@#cp -f rx_tcp_server /tftpboot/

rx_tcp_server: $(OBJ)
	$(cc) $(ldflags) -o rx_tcp_server $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f rx_tcp_server

