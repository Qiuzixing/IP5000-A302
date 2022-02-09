
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ_QUERY = node_query.o astparam.o astnetwork.o
OBJ_RESPONSER = node_responser.o astparam.o astnetwork.o ast_send_event.o uds_ipc.o

default: build
	@echo "Done"

build: clean node_query node_responser
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f node_query $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f node_responser $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@#cp -f node_query /tftpboot
	@#cp -f node_responser /tftpboot
	@#cp -f responser_emu.sh /tftpboot

node_query: $(OBJ_QUERY)
	$(cc) $(ldflags) -o node_query $(OBJ_QUERY)

node_responser: $(OBJ_RESPONSER)
	$(cc) $(ldflags) -o node_responser $(OBJ_RESPONSER)

%.o: %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f node_query
	rm -f node_responser
