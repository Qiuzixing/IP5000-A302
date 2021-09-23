include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix
ROOTFS=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon

CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
CXX=$(CROSS_COMPILE)g++

NAME = lib802_1x_commn

CFLAGS += -Wall -fPIC
CFLAGS += -I$(PREFIX)/include

CXXFLAGS = $(CFLAGS)
CPPFLAGS = $(CXXFLAGS)

LDFLAGS = -L$(PREFIX)/lib

CFILES=

CXXFILES = Exec.cpp \
		   file.cpp \
		   MsgQueue.cpp \
		   Timers.cpp 

default: build
	@echo "Done"

clean:
	rm -f $(NAME).so  $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
			    
build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	$(CXX) -o $(NAME).so $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) -shared $(LDFLAGS)
	cp $(NAME).so $(PREFIX)/lib		   
	cp $(NAME).so $(ROOTFS)/lib		   

