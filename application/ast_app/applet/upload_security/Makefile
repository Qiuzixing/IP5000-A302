
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


cc=$(CROSS_COMPILE)gcc

cflags=-Os -g -fdata-sections -ffunction-sections
ldflags=-g -Wl

ifeq ($(BUILD_TYPE), host)
	cflags += -DAST1500_HOST
else
	cflags += -DAST1500_CLIENT
endif

default: build
	@echo "Done"

build: clean upload_security
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/
	cp -f upload_security $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/.

upload_security : cgic.o upload_security.o
	$(cc) $(ldflags) -o upload_security cgic.o upload_security.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f upload_security
