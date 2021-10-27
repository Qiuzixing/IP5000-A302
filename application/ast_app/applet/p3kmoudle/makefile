include $(AST_SDK_ROOT)/ast-current.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar

LIB = libp3kapi.a
LIB_PWD = ./p3klib/
P3KTCP = ./p3k_tcp/

default: build
	@echo "Done"
	
build : p3ktcp
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/ 
	cp -f $(P3KTCP)p3ktcp $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	
p3ktcp:
	make -C $(P3KTCP)


clean:
	make -C $(LIB_PWD) clean
	make -C $(P3KTCP) clean
