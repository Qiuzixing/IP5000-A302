export ROOT_DIR:=$(shell pwd)

RM = rm -rf
#GPULS = arm-linux-gnueabihf-gcc
#STRIP := arm-linux-gnueabihf-strip
#AR := arm-linux-gnueabihf-ar

#GPULS = /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
#STRIP := /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-strip
#AR := /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

GPULS = gcc
STRIP := strip
AR := ar
SRC_DIR += $(ROOT_DIR)/\
			$(ROOT_DIR)/debugtool\
			$(ROOT_DIR)/p3kmsgqueue\
			$(ROOT_DIR)/list\
			$(ROOT_DIR)/handlelist\
			$(ROOT_DIR)/p3ksrc\
#STRIP :=  arm-linux-gnueabihf-strip

SRCS                :=  $(shell find $(SRC_DIR) -name '*.c')
SRCS                :=  $(sort $(SRCS))
OBJS                :=  $(SRCS:%.c=%.o)

INCLUDE_DIR         :=  $(ROOT_DIR)
INCLUDE_DIR         +=  $(SRC_DIR)
INCLUDE_DIR         +=  $(ROOT_DIR)/include
INCLUDES            :=  $(shell find $(INCLUDE_DIR) -name '*.h*')
INCLUDE_DIR         :=  $(sort $(dir $(INCLUDES)))
INCLUDES       :=  $(foreach dir,$(INCLUDE_DIR), -I$(dir))


COMPILE         =  $(GPULS) $(INCLUDES) -c "$<" -o "$@" 

LIB_TARGET = libp3kapi.a

TARGET := p3kapitest
.PHONY:all

all:$(LIB_TARGET) $(TARGET)
$(LIB_TARGET):$(OBJS)
	$(AR) rcs $(LIB_TARGET) $(OBJS)
$(TARGET):$(OBJS)
	$(GPULS)  -o $@ $^  -lpthread -lm
	$(STRIP) $(TARGET)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
	$(RM)  $(OBJS) $(TARGET) $(LIB_TARGET)