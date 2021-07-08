
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../Makefile.param
	include $(PARAM_FILE)
endif


PROG = bin/web
LOG_INC = $(PREFIX)/include/log4cpp
INCPATH = $(PREFIX)/include
PREFIX_LIB = $(PREFIX)/lib

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
			  src/thread.cpp
			  
#CFILES = ../ldap/ldap_funcs.c



CPPFLAGS = -I$(PREFIX)/include/civetweb -I./src -I$(LOG_INC) -I$(INCPATH) \
		  $(COPT) -DUSE_WEBSOCKET
CFLAGS = $(LDAP_INC)




LDFLAGS = $(LDAP_LIB) $(CIVETWEB_LIB) $(LOG4CPP_LIB)  -L$(PREFIX_LIB) \
			-L./ -lssl -lcrypt -lcrypto  -ljsoncpp $(LIBS)

	





include ./Makefile.in-os



default: clean 
	make build 
	make install


build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) 
	$(CXX) -o $(PROG) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(LDFLAGS)


clean:
#	rm -f $(PROG)
	rm -f $(PROG) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	
install:
	@echo "install"
	test -z $(PREFIX)/usr/local/bin || mkdir -p $(PREFIX)/usr/local/bin
	cp $(PROG) $(PREFIX)/usr/local/bin
	mkdir -p $(PREFIX)/etc/www
	cp -r ./bin/www/* $(PREFIX)/etc/www

	
.PHONY: default clean build install
#.PHONY: all
