

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


OBJ = soip3.o


default: build
	@echo "Done"

build: $(OBJ)
	$(cc) $(ldflags) -o soip3 $(OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f soip3 $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -f *.o
	rm -f soip3 2>/dev/null || exit 0

%.o : %.c Makefile
	${cc} ${cflags} -c $<
