
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc
CFLAGS=-O3 -g -W -Wstrict-prototypes -std=gnu99 -I.

HOST_BIN=host_ir2
CLIENT_BIN=client_ir2

OBJ=ctrl.o uds_ipc.o

ifeq ($(BUILD_TYPE),host)
BIN=$(HOST_BIN)
else
BIN=$(CLIENT_BIN)
CFLAGS+=-DAST_IR_APP_CLIENT
endif

BIN_IRR=irr
OBJ_IRR=irr.o
BIN_IRRD=irrd
OBJ_IRRD=irrd.o ir_nec.o
BIN_IRS=irs
OBJ_IRS=irs.o
BIN_IRSD=irsd
OBJ_IRSD=irsd.o

default: build
	@echo "Done"

build: clean $(OBJ) $(OBJ_IRR) $(OBJ_IRRD) $(OBJ_IRS) $(OBJ_IRSD)
	$(CC) -o $(BIN) $(OBJ) -lm
	$(CC) -o $(BIN_IRR) $(OBJ_IRR) -lm
	$(CC) -o $(BIN_IRRD) $(OBJ_IRRD) -lm
	$(CC) -o $(BIN_IRS) $(OBJ_IRS) -lm
	$(CC) -o $(BIN_IRSD) $(OBJ_IRSD) -lm
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(BIN) $(BIN_IRR) $(BIN_IRRD) $(BIN_IRS) $(BIN_IRSD) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -rf *.o $(HOST_BIN) $(CLIENT_BIN) $(BIN_IRR) $(BIN_IRRD) $(BIN_IRS) $(BIN_IRSD)

%.o : %.c Makefile
	$(CC) $(CFLAGS) -c $<

