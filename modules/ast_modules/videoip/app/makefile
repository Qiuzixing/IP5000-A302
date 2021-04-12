

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc


#cflags=-O3 -g -nostdinc -iprefix $(PREFIX) -isystem $(SYSTEM_INCLUDE_DIR) -isystem $(LINUX_INCLUDE_DIR)
cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


HOSTOBJ = ctrl.o uds_ipc.o \
	astnetwork.o \
	ast_send_event.o

VIDEO_CONFIG_OBJ = video_config.o \
	getinf.o

ClIENTOBJ = ctrl.o uds_ipc.o \
	astnetwork.o \
	getinf.o \
	astcommon.o

ifeq ($(BUILD_TYPE),host)
	BUILD_TARGET=ASTHOST
else
	BUILD_TARGET=ASTCLIENT
	cflags+=-DAST_VIDEO_APP_CLIENT
endif

default: $(BUILD_TARGET)
	@echo "Done"

clean:
	rm -f *.o
	rm -f astclient asthost video_config 2>/dev/null || exit 0

ASTHOST: clean $(HOSTOBJ) $(VIDEO_CONFIG_OBJ)
	$(cc) $(ldflags) -o asthost $(HOSTOBJ)
	$(cc) $(ldflags) -o video_config $(VIDEO_CONFIG_OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f asthost $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f video_config $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	 
ASTCLIENT: clean $(ClIENTOBJ)
	$(cc) $(ldflags) -o astclient $(ClIENTOBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f astclient $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

%.o : %.c makefile
	${cc} ${cflags} -c $<
