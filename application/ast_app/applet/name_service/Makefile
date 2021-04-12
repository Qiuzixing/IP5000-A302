
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ = name_service.o astnetwork.o

default: build
	@echo "Done"

build: name_service
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f name_service $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

name_service: $(OBJ)
	$(cc) $(ldflags) -o name_service $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f name_service