
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../Makefile.param
	include $(PARAM_FILE)
endif

PROG = bin/web
BMP2JPG = bin/bmp2jpg
LOG_INC = $(PREFIX)/include/log4cpp
INCPATH = $(PREFIX)/include
PREFIX_LIB = $(PREFIX)/lib

JPG_PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix
JPG_INCLUDE=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix/include

#cc=$(CROSS_COMPILE)gcc

TOP = civetweb
LOG_PATH = log4cpp
CIVETWEB_LIB = $(PREFIX_LIB)/libcivetweb.a
LOG4CPP_LIB = $(PREFIX_LIB)/liblog4cpp.a

LIBS =  -lpthread -lrt  \
		-lstdc++ -ldl
	

#LDAP_LIB = -L../../prefix/openldap-arm/lib -lldap -llber
#LDAP_INC = -I ../ldap -I ../../prefix/openldap-arm/include -I ../../prefix/include/openssl


CXXFILES = 	  src/Civetweb_API.cpp \
			  src/main.cpp \
			  src/webapp.cpp	\
			  src/process_json.cpp \
			  src/operation.cpp \
			  src/stringsplit.cpp \
			  src/mutex.cpp \
			  src/cond.cpp \
			  src/thread.cpp \
			  src/astparam.cpp 
			  
CFILES = src/bmp2jpg.c



CPPFLAGS = -I$(PREFIX)/include/civetweb -I./src -I$(LOG_INC) -I$(INCPATH) \
		  $(COPT) -DUSE_WEBSOCKET
CFLAGS = -I$(JPG_INCLUDE)




LDFLAGS = $(CIVETWEB_LIB) $(LOG4CPP_LIB)  -L$(PREFIX_LIB) -L$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix/lib\
			-L./ -lssl -lcrypt -lcrypto  -ljsoncpp $(LIBS)

	


include ./Makefile.in-os



default:clean build install

build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) 
	$(CXX) -o $(PROG) $(CXXFILES:.cpp=.o) $(LDFLAGS)
	$(CC) -o $(BMP2JPG) $(CFILES:.c=.o) -L ${JPG_PREFIX}/lib -ljpeg

clean:
#	rm -f $(PROG)
	rm -f $(PROG) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	
install:
	@echo "install"
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ./bin/bmp2jpg $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ./bin/web $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f ./bin/gbwebserver.json $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/
	cp -f ./bin/log4cpp.conf $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/
	cp -f ./bin/server.pem $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/www/
	
.PHONY: default clean build install
#.PHONY: all
