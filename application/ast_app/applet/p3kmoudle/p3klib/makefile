#include conf.mak

export ROOT_DIR:=$(shell pwd)

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar

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


COMPILE         =  $(CC) $(INCLUDES) -c "$<" -o "$@" 

LIB_TARGET = libp3kapi.a

TARGET := p3kapitest
.PHONY:all

all:$(LIB_TARGET) $(TARGET)
$(LIB_TARGET):$(OBJS)
	$(AR) rcs $(LIB_TARGET) $(OBJS)
$(TARGET):$(OBJS)
	$(CC)  -o $@ $^  -lpthread -lm
	$(STRIP) $(TARGET)
#
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
	$(RM)  $(OBJS) $(TARGET) $(LIB_TARGET)