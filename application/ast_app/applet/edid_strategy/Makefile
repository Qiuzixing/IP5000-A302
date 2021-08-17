include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CXX=$(CROSS_COMPILE)g++
PROG=handle_edid
CXXFILES=main.cpp jsoncpp.cpp jsonparse.cpp
KCFLAGS=-I ./json

default: clean
	make build

build: $(PROG)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(PROG) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

$(PROG): $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	$(CXX) $(KCFLAGS) -o $(PROG) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(LDFLAGS)

clean:
	rm -rf $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	rm -rf $(PROG)