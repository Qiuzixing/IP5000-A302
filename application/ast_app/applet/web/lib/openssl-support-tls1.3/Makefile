ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

# special handle for Makefile.param env
#export CC := gcc

export CROSS_COMPILE=



NAME = openssl-1.1.1c

default: clean unpack dopatch config build install

clean:
	@echo "cleanning" ; rm -rf $(NAME)

unpack:
	@echo "unpacking" ; tar xzvf $(NAME).tar.gz

dopatch:
	@echo "dopatch" ;

config:
#	cd $(NAME) && ./Configure --prefix=$(PREFIX) no-asm --cross-compile-prefix=$(CROSS_COMPILE) shared zlib-dynamic -I$(PREFIX)/include -L$(PREFIX)/lib -lz linux-armv4
#	cd $(NAME) && ./Configure dist --prefix=$(PREFIX) --cross-compile-prefix=$(CROSS_COMPILE) shared -fPIC
#	cd $(NAME) && ./config --prefix=$(PREFIX)  shared -fPIC
#	cd $(NAME) && ./Configure --prefix=$(PREFIX) no-asm shared  enable-tls1_3   linux-aarch64
	cd $(NAME) && ./Configure --prefix=$(PREFIX) no-asm shared  enable-tls1_3   linux-armv4
build:
	make -C $(NAME)
	
install:
	test -z $(PREFIX) || mkdir -p $(PREFIX)
	make -C $(NAME) install
	rm -rf $(PREFIX)/lib/libssl.so*
	rm -rf $(PREFIX)/lib/libcrypto.so*
