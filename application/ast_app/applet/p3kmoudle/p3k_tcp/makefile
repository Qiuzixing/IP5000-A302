ROOT_DIR := $(shell pwd)

#GPULS = arm-linux-gnueabihf-gcc
#STRIP := arm-linux-gnueabihf-strip
#AR := arm-linux-gnueabihf-ar

#GPULS = /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
#STRIP := /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-strip
#AR := /home/longao/P3K/crosstool/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

CC = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
STRIP := /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-strip
AR = /opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

#GPULS = gcc
#STRIP := strip
#AR := ar
SRC_DIR += $(ROOT_DIR)/\
			$(ROOT_DIR)/debugtool\
			$(ROOT_DIR)/socketapi\
			$(ROOT_DIR)/handlelist\
			$(ROOT_DIR)/list\
		
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
LIB = libp3kapi.a
LIB_PWD = ../p3klib/

COMPILE         =  $(CC) $(INCLUDES) -c "$<" -o "$@"
#$(LIB)



TARGET := p3ktcp
.PHONY:all

all: $(TARGET)
$(TARGET):$(OBJS)
	cd $(ROOTDIR) 
	make -C $(LIB_PWD)
	cp -f $(LIB_PWD)$(LIB) ./
	$(CC)  -o $@ $^  -lpthread -lm $(LIB)
	$(STRIP) $(TARGET)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
#	make -C $(LIB_PWD) clean
#	rm $(LIB)
	$(RM)  $(OBJS) $(TARGET) $(LIB_TARGET)