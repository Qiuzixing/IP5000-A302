

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


cc=$(CROSS_COMPILE)gcc


cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


OBJ = soip.o


default: build
	@echo "Done"

build: $(OBJ)
	$(cc) $(ldflags) -o soip $(OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f soip $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm soip 2>/dev/null || exit 0
	rm -f *.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
