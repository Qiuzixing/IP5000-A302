
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc

default: build
	@echo "Done"

build: client_i2s.o
ifeq ($(BUILD_TYPE),client)
	$(CC) -o client_i2s client_i2s.o -lm
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f client_i2s $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
else
	@echo "Client only app"
endif

clean:
	rm -rf *.o client_i2s

