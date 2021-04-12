
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g


default: build
	@echo "Done"

build: astsetname
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f astsetname $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f astsetname /tftpboot

astsetname: astsetname.o
	$(cc) $(ldflags) -o astsetname astsetname.o

%.o: %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f astsetname
