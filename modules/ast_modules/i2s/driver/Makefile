 
# Please modify here or set environments.
# KOBJDIR should be pointed to the build directory of your kernel.
# 
#DEBUG ?= n 
include $(AST_SDK_ROOT)/ast.cfg


TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
KOBJDIR = $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel

PWD       := $(shell pwd)

%.x:%.c
	arm-aspeed-linux-gnu-gcc -o $@ $<

KBUILD_VERBOSE:=0

1500_i2s-objs 	  := i2s_main.o i2s_task.o i2s_network.o i2s_event.o  ast1500i2s.o ast1510i2s.o alc5624.o alc5640.o wm8903.o codec_hal.o i2s_clk.o

obj-m += 1500_i2s.o

ifdef CONFIG_ARCH_AST1500_CLIENT
	EXTRA_CFLAGS += -DI2S_CLIENT
endif

default:
	$(MAKE) clean
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE}  M=$(PWD) modules
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f 1500_i2s.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f 1500_i2s.ko /tftpboot

check:
	#$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules checkstack namespacecheck headers_check
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules

.PHONY: cscope
cscope:
	cscope -b -k -R

.PHONY: clean
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

