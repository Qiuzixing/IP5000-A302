
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)


cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

ifeq ($(BUILD_TYPE), host)
	cflags += -DAST1500_HOST
else
	cflags += -DAST1500_CLIENT
endif

default: build
	@echo "Done"

build: clean astfwup
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/cgi-bin/
	cp -f astfwup $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/cgi-bin/astfwup.cgi

astfwup : cgic.o cgictest.o
	$(cc) $(ldflags) -o astfwup cgic.o cgictest.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f astfwup
