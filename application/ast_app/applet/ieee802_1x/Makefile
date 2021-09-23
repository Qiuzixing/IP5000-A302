include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix
ROOTFS=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon

CC=$(CROSS_COMPILE)gcc
CPP=$(CROSS_COMPILE)g++
CXX=$(CROSS_COMPILE)g++

NAME=ieee802dot1x


CFLAGS += -I ./common -I $(PREFIX)/include -Wall
CXXFLAGS = $(CFLAGS)
CPPFLAGS = $(CXXFLAGS)
LDFLAGS = -L$(PREFIX)/lib -lwpa_client -l802_1x_commn -lpthread -lrt 

CFILES=

CXXFILES = ./main.cpp\
           ./IEEE_802_1x_Verify.cpp
           


default: build
	@echo "Done"
	
build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	make -C common
	$(CXX) -o $(NAME) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(LDFLAGS)
	make -C daemon
	mkdir -p $(PREFIX)/usr/local/bin
	cp $(NAME) $(PREFIX)/usr/local/bin
	cp $(NAME) $(ROOTFS)/usr/local/bin

clean:
	rm -rf $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(NAME)
	make -C common clean
	make -C daemon clean
	
