
include $(AST_SDK_ROOT)/ast.cfg


TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc
cflags=-O3 -g -W -Wstrict-prototypes -std=gnu99 -I.

OBJ=ctrl.o uds_ipc.o
ifeq ($(BUILD_TYPE),host)
BIN=host_i2s
else
BIN=client_i2s
cflags+=-DAST_VIDEO_APP_CLIENT
endif

default: build
	@echo "Done"


build: clean $(OBJ)
	$(CC) -o $(BIN) $(OBJ) -lm
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(BIN) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -rf *.o
	rm -f host_i2s client_i2s 2>/dev/null || exit 0

%.o : %.c Makefile
	$(CC) $(cflags) -c $<

