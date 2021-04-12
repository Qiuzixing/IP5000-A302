

#include $(AST_SDK_ROOT)/ast.cfg

#TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=gcc


#cflags=-O3 -g -nostdinc -iprefix $(PREFIX) -isystem $(SYSTEM_INCLUDE_DIR) -isystem $(LINUX_INCLUDE_DIR)
cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


HOSTOBJ = asthost.o \
	astnetwork.o \
	ast_send_event.o

VIDEO_CONFIG_OBJ = video_config.o \
	getinf.o

ClIENTOBJ = astclient.o \
	astnetwork.o \
	getinf.o \
	astcommon.o

ifeq ($(BUILD_TYPE),host)
	BUILD_TARGET=ASTHOST
else
	BUILD_TARGET=ASTCLIENT
endif

BUILD_TARGET=ASTCLIENT

default: $(BUILD_TARGET)
	@echo "Done"

clean:
	rm -f *.o

ASTHOST: clean $(HOSTOBJ) $(VIDEO_CONFIG_OBJ)
	$(cc) $(ldflags) -o asthost $(HOSTOBJ)
	$(cc) $(ldflags) -o video_config $(VIDEO_CONFIG_OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f asthost $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f video_config $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	 
ASTCLIENT: clean $(ClIENTOBJ)
	$(cc) $(ldflags) -o astclient $(ClIENTOBJ)
	cp -f astclient ../

%.o : %.c makefile
	${cc} ${cflags} -c $<
