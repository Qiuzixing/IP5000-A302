# Makefile for the VIDEO/IP driver
#

ifeq ($(KERNELRELEASE),)
########################################################################################################################
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

# The current directory is passed to sub-makes as argument
PWD := $(shell pwd)

# Please modify here or set environments.
# KOBJDIR should be pointed to the build directory of your kernel.
KOBJDIR = $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/kernel

KBUILD_VERBOSE:=0


default:
	$(MAKE) clean
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	@[ -f "$(PWD)/release/$(TARGET_FOLDER_NAME)/videoip.ko" ] || { echo;echo "ERROR!!! Need $(TARGET_FOLDER_NAME)/videoip.ko! Wrong SDK?!"; exit 1;}
	cp -f $(PWD)/release/$(TARGET_FOLDER_NAME)/videoip.ko $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	
check:
	#make -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules checkstack namespacecheck headers_check
	$(MAKE) -C $(KOBJDIR) LANG=C KBUILD_VERBOSE=${KBUILD_VERBOSE} C=1 M=`pwd` modules

sdk:
	rm -rf *.c
	cp -f Makefile.sdk Makefile
	cp -f .gitignore.sdk .gitignore

.PHONY: cscope
cscope:
	cscope -b -k -R

.PHONY: clean
clean:
	#rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions polarssl/*.o polarssl/.*.cmd
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions polarssl/*.o polarssl/.*.cmd

else #ifeq ($(KERNELRELEASE),)
########################################################################################################################

DEBUG ?= n 

%.x:%.c
	arm-aspeed-linux-gnu-gcc -o $@ $<

ifeq ($(DEBUG),y)
	EXTRA_CFLAGS += -DCONFIG_VIDEO_DEBUG
endif

#EXTRA_CFLAGS += -DCONFIG_AS_AST2100
EXTRA_CFLAGS += -DAST_MULTICAST

videoip-objs 	  := videoip_main.o videoip_interrupt.o rc4.o videoIP_network.o client_mgnt.o frame_buffer_snapshot.o aes.o gen_lock.o
videoip-objs += hdcp_handler.o hdcp1_sm.o hdcp2_sm.o polarssl/bignum.o polarssl/aes.o polarssl/sha2.o polarssl/rsa.o polarssl/md_wrap.o polarssl/md.o
videoip-objs += stream_mode_encode.o stream_mode_decode.o desc_mode.o
videoip-objs += ve_encode.o encode.o decode.o common.o ve_common.o delay_dequeue.o
videoip-objs += hw_quality_profile.o sw_quality_profile.o dynamic_quality.o
videoip-objs += cat9883.o vga_rx_drv.o
videoip-objs += mode_detection.o
videoip-objs += ddc_ctrl.o
videoip-objs += videoip_devfs.o videoip_sysfs.o
videoip-objs += videoip_event.o encode_worker.o decode_worker.o

ifdef CONFIG_CRT_ASPEED
	obj-m += videoip.o
endif


endif #ifeq ($(KERNELRELEASE),)