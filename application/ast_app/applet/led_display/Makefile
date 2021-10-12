
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g -lrt -ldl

OBJ = led_display.o

default: build
	@echo "Done"

build: led_display
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f led_display $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@#cp -f led_display /tftpboot/

led_display: $(OBJ)
	$(cc) $(ldflags) -o led_display $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f led_display

