#EXPORT_PLATFORM ?= X86
#EXPORT_PLATFORM ?= RX3328
EXPORT_PLATFORM ?= ASBEED5000
ifeq ($(EXPORT_PLATFORM),X86)
PREFIX = /home/longzhang/lldpmodule/install
HOST =  
CC = gcc
AR = ar
endif

ifeq ($(EXPORT_PLATFORM),RX3328)
PREFIX = /
HOST = aarch64-linux-gnu
CC = aarch64-linux-gnu-gcc
AR = aarch64-linux-gnu-ar
endif

ifeq ($(EXPORT_PLATFORM),ASBEED5000)
PREFIX = /
HOST = arm-aspeed-linux-gnu
CC = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
AR = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar
endif


