ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

NAME = libjsoncpp

CFLAGS +=  -Wall -fPIC 
CFLAGS += -O2

CFLAGS += 	-I ./include
			
CXXFLAGS = $(CFLAGS)
CPPFLAGS = $(CXXFLAGS)

CXXFILES =	$(shell ls src/*.cpp)
HEADERS := $(shell ls include/json/*.h)

#LDFLAGS = 	-L../../prefix/lib -lIpControlLib -lstdc++ -lpthread -lrt 
default: clean
	make install
	
clean:
	rm -f $(NAME) $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) $(NAME).a	 $(NAME).so
	
build: $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
	$(AR) -r $(NAME).a $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) 
	$(RANLIB) $(NAME).a	
	$(LINK) -o $(NAME).so $(CFILES:.c=.o) $(CXXFILES:.cpp=.o) -shared $(LDFLAGS)

install: build
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	cp $(NAME).a $(PREFIX)/lib/.
#	cp $(NAME).so $(PREFIX)/lib/.
	cp $(HEADERS) $(PREFIX)/include/.

