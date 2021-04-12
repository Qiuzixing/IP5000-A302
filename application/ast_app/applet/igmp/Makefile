

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ = igmp.o

default: build
	@echo "Done"

build: igmp
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f igmp $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f igmp /tftpboot/

igmp: $(OBJ)
	$(cc) $(ldflags) -o igmp $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f igmp

