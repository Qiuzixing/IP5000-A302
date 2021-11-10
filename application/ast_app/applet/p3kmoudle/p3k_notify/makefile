ROOT_DIR := $(shell pwd)

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar


SRC_DIR += $(ROOT_DIR)/\

		
#STRIP :=  arm-linux-gnueabihf-strip

SRCS                :=  $(shell find $(SRC_DIR) -name '*.c')
SRCS                :=  $(sort $(SRCS))
OBJS                :=  $(SRCS:%.c=%.o)

INCLUDE_DIR         :=  $(ROOT_DIR)
INCLUDE_DIR         +=  $(SRC_DIR)
INCLUDE_DIR         +=  $(ROOT_DIR)
INCLUDES            :=  $(shell find $(INCLUDE_DIR) -name '*.h*')
INCLUDE_DIR         :=  $(sort $(dir $(INCLUDES)))
INCLUDES       :=  $(foreach dir,$(INCLUDE_DIR), -I$(dir))
LIB = libp3kapi.a
LIB_PWD = ../p3klib/

COMPILE         =  $(CC) $(INCLUDES) -c "$<" -o "$@"
#$(LIB)



TARGET := p3k_notify
.PHONY:all

all: $(TARGET)
$(TARGET):$(OBJS)

	$(CC)  -o $@ $^  -lpthread -lm -lstdc++
	$(STRIP) $(TARGET)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
	$(RM)  $(OBJS) $(TARGET)