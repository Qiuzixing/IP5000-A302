
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
#libcivetweb.aÂ·¾¶
LIB_CIVETWEB = $(PREFIX)/lib
TOP = civetweb

CIVETWEB_LIB = libcivetweb.a
NAME = civetweb
include ./Makefile.in-os

ifeq ($(TARGET_OS),LINUX) 
	LIBS += -ldl
endif

default:
	#make unpack
	#make dopatch
	make build 
	make install
	
clean:
	rm -rf $(NAME)
	rm -f $(TOP)/$(CIVETWEB_LIB)
	rm -f $(LIB_CIVETWEB)/$(CIVETWEB_LIB)
unpack:
	@echo "unpacking" ; tar -xf $(NAME).tar.gz

dopatch:
	@echo "dopatch" ;

build:unpack
	$(MAKE) -C $(TOP) COPT='-DNO_SSL_DL=1' WITH_CPP=1 WITH_WEBSOCKET=1 clean lib
	
install:
	@echo "install"
	test -z $(PREFIX) || mkdir -p $(PREFIX)
	cp $(TOP)/$(CIVETWEB_LIB) $(LIB_CIVETWEB)
	test -z $(PREFIX)/include/civetweb || mkdir -p $(PREFIX)/include/civetweb
	cp -rf civetweb/include/* $(PREFIX)/include/civetweb
	
.PHONY: default clean build install
#.PHONY: all
