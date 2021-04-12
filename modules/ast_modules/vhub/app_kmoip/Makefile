
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc

default: clean build
	@echo "Done"

build: kmoip.o
	$(CC) -o kmoip kmoip.o -lm
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f kmoip $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -rf *.o kmoip


