include $(AST_SDK_ROOT)/ast.cfg

#CROSS_COMPILE=arm-aspeed-linux-gnu-
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD  = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip

TARGET_APP = ntp_client
OBJ_PATH = objs
TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
PREFIX_BIN =$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/


SRC = source

LIBS =
CFLAGS =-Wall -Werror -O2 
CFLAGS += -std=gnu99

CPPFLAGS = -Wall -Werror -O2

LINKFLAGS = -lrt

INCLUDES = -I ./$(SRC)/include

INCLUDES += -I./$(SRC)/app
INCLUDES += -I./$(SRC)/app/Global

SRCDIR = $(SRC)/app
SRCDIR += $(SRC)/app/Global

#C_SOURCES = $(wildcard *.c)
C_SRCDIR = $(SRCDIR)
C_SOURCES = $(foreach d,$(C_SRCDIR),$(wildcard  $(d)/*.c) )
C_OBJS = $(patsubst %.c, $(OBJ_PATH)/%.o, $(C_SOURCES))

#CPP_SOURCES = $(wildcard *.cpp)
CPP_SRCDIR = $(SRCDIR)
CPP_SOURCES = $(foreach d,$(CPP_SRCDIR),$(wildcard  $(d)/*.cpp) )
CPP_OBJS = $(patsubst %.cpp, $(OBJ_PATH)/%.o, $(CPP_SOURCES))

default:init compile install
	@echo "Done"

$(C_OBJS):$(OBJ_PATH)/%.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(CPP_OBJS):$(OBJ_PATH)/%.o:%.cpp
	$(CPP) -c $(CPPFLAGS) $(INCLUDES) $< -o $@

init:
	$(foreach d,$(SRCDIR), mkdir -p $(OBJ_PATH)/$(d);)

test:
	@echo "C_SOURCES: $(C_SOURCES)"
	@echo "C_OBJS: $(C_OBJS)"
	@echo "CPP_SOURCES: $(CPP_SOURCES)"
	@echo "CPP_OBJS: $(CPP_OBJS)"

compile:$(C_OBJS) $(CPP_OBJS)
	$(CPP)  $^ -o $(TARGET_APP)  $(LINKFLAGS) $(LIBS)
	$(STRIP) $(TARGET_APP)

clean:
	rm -rf $(OBJ_PATH)
	rm -f $(TARGET_APP)

install: $(TARGET_APP)
	mkdir -p $(PREFIX_BIN)
	cp $(TARGET_APP) $(PREFIX_BIN)

uninstall:
	rm -f $(PREFIX_BIN)/$(TARGET_APP)

rebuild: clean init compile

