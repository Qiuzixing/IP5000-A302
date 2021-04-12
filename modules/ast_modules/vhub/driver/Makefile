
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.

# Assume the source tree is where the running kernel was built
# You should set KERNELDIR in the environment if it's elsewhere
#    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#KERNELDIR = ../../../kernel/linux-2.6.15.7

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

KOBJDIR = $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel
#KSOURCE = $(AST_SDK_ROOT)/kernel/linux-2.6.15.7-unpacked
#
#HCD_HEADER:=$(KSOURCE)/drivers/usb/core/hcd.h
#HUB_HEADER:=$(KSOURCE)/drivers/usb/core/hub.h
#
#EXTRA_CFLAGS += -DHCD_HEADER=\"$(HCD_HEADER)\"    
#EXTRA_CFLAGS += -DHUB_HEADER=\"$(HUB_HEADER)\"

HCD_HEADER:=../drivers/usb/core/hcd.h
HUB_HEADER:=../drivers/usb/core/hub.h

EXTRA_CFLAGS += -DHCD_HEADER=\<$(HCD_HEADER)\>
EXTRA_CFLAGS += -DHUB_HEADER=\<$(HUB_HEADER)\>

debug = n
mfg = y

ifeq ($(debug),y)
EXTRA_CFLAGS += -DCONFIG_USBIP_DEBUG=1 -Winline -g3 -O0
endif

ifeq ($(mfg),y)
EXTRA_CFLAGS += -DMFG=1
else
EXTRA_CFLAGS += -DMFG=0
endif

ifeq ($(KERNELRELEASE),)

    # The current directory is passed to sub-makes as argument
    PWD := $(shell pwd)

default:
	$(MAKE) clean
	$(MAKE) -C $(KOBJDIR) M=$(PWD) modules
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@if [ $(BUILD_TYPE) = 'host' ]; then \
		cp -f usbip_common_mod.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
		if ! [ -f "$(PWD)/release/$(TARGET_FOLDER_NAME)/vhub.ko" ]; then \
			echo; echo "ERROR!!! Need $(TARGET_FOLDER_NAME)/vhub.ko! Wrong SDK?!"; exit 1; \
		fi; \
		cp -f $(PWD)/release/$(TARGET_FOLDER_NAME)/vhub.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
	else \
		cp -f usbip_common_mod.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
		cp -f usbip.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
	fi || exit 1


clean:
	#rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions usbip_common_mod.ko usbip.ko

sdk:
	rm -rf vhub_sysfs.c vhub_hcd.c vhub_usbip_rx.c vhub_usbip_tx.c vhub_hal.c vhub_ep.c vhub_config.c vhub_mfg.c vhub_port_state.c vhub_kmoip.c
	cp -f Makefile.sdk Makefile
	cp -f .gitignore.sdk .gitignore

else
    usbip_common_mod-objs := usbip_common.o usbip_event.o
    #vhub-objs	  := vhub_sysfs.o vhub_hcd.o vhub_usbip_rx.o vhub_usbip_tx.o vhub_hal.o vhub_ep.o vhub_config.o vhub_mfg.o vhub_port_state.o
    usbip-objs    := stub_dev.o stub_main.o stub_rx.o stub_tx.o
    
    # called from kernel build system: just declare what our modules are
    #obj-m += vhub.o
    obj-m += usbip_common_mod.o
    obj-m += usbip.o

endif


