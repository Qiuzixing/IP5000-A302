include $(AST_SDK_ROOT)/ast-current.cfg
export ROOT_DIR:=$(shell pwd)

GPULS = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip
AR = $(CROSS_COMPILE)ar
GPULSCXX = $(CROSS_COMPILE)g++
RM = rm -rf

SRC_DIR += $(ROOT_DIR)/\
			$(ROOT_DIR)/debugtool\
			$(ROOT_DIR)/p3kmsgqueue\
			$(ROOT_DIR)/list\
			$(ROOT_DIR)/handlelist\
			$(ROOT_DIR)/p3ksrc\
#STRIP :=  arm-linux-gnueabihf-strip


SRCS               :=  $(shell find $(SRC_DIR) -name '*.c')

SRCS                :=  $(sort $(SRCS))
OBJS                :=  $(SRCS:%.c=%.o)

SRCSCXX               :=  $(shell find $(SRC_DIR) -name '*.cpp')
SRCSCXX                :=  $(sort $(SRCSCXX))
OBJSCXX                +=  $(SRCSCXX:%.cpp=%.o)


INCLUDE_DIR         :=  $(ROOT_DIR)
INCLUDE_DIR         +=  $(SRC_DIR)
INCLUDE_DIR         +=  $(ROOT_DIR)/include
INCLUDES            :=  $(shell find $(INCLUDE_DIR) -name '*.h*')
INCLUDE_DIR         :=  $(sort $(dir $(INCLUDES)))
INCLUDES       :=  $(foreach dir,$(INCLUDE_DIR), -I$(dir))

CFLAGS = -Wall  -O2 -Wunused-but-set-variable
CXXFLAGS = -Wall -O2 -Wunused-but-set-variable

ifeq ($(BUILD_TYPE),host)
CFLAGS += -DCONFIG_P3K_HOST
CXXFLAGS  += -DCONFIG_P3K_HOST
else
CFLAGS += -DCONFIG_P3K_CLIENT
CXXFLAGS  += -DCONFIG_P3K_CLIENT
endif

COMPILE         =  $(GPULS) $(CFLAGS) $(INCLUDES) -c "$<" -o "$@" 
COMPILECXX      =  $(GPULSCXX) $(CXXFLAGS) $(INCLUDES) -c "$<" -o "$@" 
LIB_TARGET = libp3kapi.a

#TARGET := p3kapitest
.PHONY:all

all:$(LIB_TARGET)
$(LIB_TARGET):$(OBJS) $(OBJSCXX)
	$(AR) rcs $(LIB_TARGET) $(OBJS) $(OBJSCXX)
#$(TARGET):$(OBJS)
#	$(GPULS)  -o $@ $^  -lpthread -lm
#	$(STRIP) $(TARGET)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)
$(OBJSCXX) :%.o : %.cpp
	$(COMPILECXX)
clean:
	$(RM)  $(OBJS) $(TARGET) $(LIB_TARGET)