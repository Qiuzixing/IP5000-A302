
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99

OBJ = remote.o \
	astnetwork.o


default: build
	@echo "Done"

clean:
	rm -f *.o
	rm -f remote 2>/dev/null || exit 0

build: $(OBJ)
	$(cc) $(ldflags) -o remote $(OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f remote $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

%.o: %.c Makefile
	${cc} ${cflags} -c $<
