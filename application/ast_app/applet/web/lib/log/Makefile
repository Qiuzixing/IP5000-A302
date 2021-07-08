ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

NAME = log4cpp-1.1

default: clean unpack dopatch config build install

clean:
	@echo "cleanning" ; rm -rf $(NAME)

unpack:
	@echo "unpacking" ; tar xzvf $(NAME).tar.gz

dopatch:
	patch -f -d log4cpp-1.1 -p1 < patch/log4cpp-1.1.patch
	patch -f -d log4cpp-1.1 -p1 < patch/log4cpp-1.1-gethostbyname.patch
	patch -f -d log4cpp-1.1 -p1 < patch/log4cpp-1.1-new.patch

config:
	echo "ac_cv_func_snprintf=yes" > log4cpp-1.1/config.cache
	cd log4cpp-1.1 && ./configure\
		--host=arm-linux-gnueabihf\
		--config-cache\
		--prefix=$(PREFIX)

build:
	make -C $(NAME)
	
install:
	test -z $(PREFIX) || mkdir -p $(PREFIX)
	make -C $(NAME) install
	@cp log4cpp-1.1/include/log4cpp/log4cpp.hh $(PREFIX)/include/log4cpp
