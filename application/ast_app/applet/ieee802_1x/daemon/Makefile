include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix
ROOTFS=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon

CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
CXX=$(CROSS_COMPILE)g++

NAME=wpa_daemon

CFLAGS = -I ../common -I $(PREFIX)/include
CXXFLAGS = $(CFLAGS)
CPPFLAGS = $(CXXFLAGS)

LDFLAGS = -L$(PREFIX)/lib -l802_1x_commn  -lwpa_client -lpthread -lrt 

CFILES=
CXXFILES = daemon.cpp 

default: build
	@echo "Done"

clean:
	rm -f $(NAME) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)

build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	$(CXX) -o $(NAME) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(LDFLAGS)
	mkdir -p $(PREFIX)/usr/local/bin
	cp $(NAME) $(PREFIX)/usr/local/bin
	cp $(NAME) $(ROOTFS)/usr/local/bin
