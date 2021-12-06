

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


cc=$(CROSS_COMPILE)gcc

# O3 is error!!!
cflags=-O2 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


OBJ = soip2.o


default: build
	@echo "Done"

build: $(OBJ)
	$(cc) $(ldflags) -o soip2 $(OBJ)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f soip2 $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

clean:
	rm -f *.o
	rm -f soip2 2>/dev/null || exit 0

%.o : %.c Makefile
	${cc} ${cflags} -c $<
