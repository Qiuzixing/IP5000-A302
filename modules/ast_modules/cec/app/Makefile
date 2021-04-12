
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc
cflags=-O3 -g -W -Wstrict-prototypes -std=gnu99 -I.
ifeq ($(BUILD_TYPE),host)
cflags+=
else
cflags+=-DAST_CEC_APP_CLIENT
endif

OBJ=ctrl.o uds_ipc.o
BIN=cec
UTI=cec_send cec_topology cec_topology_raw cec_pa_poll cec_power_on_tv cec_power_off_tv

default: build
	@echo "Done"


build: clean $(OBJ)
	$(CC) -o $(BIN) $(OBJ) -lm
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(BIN) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(UTI) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -rf *.o
	rm -f $(BIN) 2>/dev/null || exit 0

%.o : %.c Makefile
	$(CC) $(cflags) -c $<

