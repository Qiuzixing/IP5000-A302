ROOT_DIR := $(shell pwd)

CC = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar


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
	$(CC)  -o $@ $^  -lpthread -lm $(LIB) -lstdc++
	$(STRIP) $(TARGET)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
#	make -C $(LIB_PWD) clean
#	rm $(LIB)
	$(RM)  $(OBJS) $(TARGET) $(LIB_TARGET)