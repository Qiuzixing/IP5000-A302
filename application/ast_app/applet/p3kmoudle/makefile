include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
STRIP := /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-strip
AR = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

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