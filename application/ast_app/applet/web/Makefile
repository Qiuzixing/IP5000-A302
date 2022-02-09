ifeq ($(PARAM_FILE), )
	PARAM_FILE:=./Makefile.param
	include $(PARAM_FILE)
endif

default:distclean prepare build_all

prepare:
	mkdir -p $(IMAGES_DIR)

build_all:
	make -C lib
	make -C src

build_app:	
	make -C src

build_lib:	
	make -C lib

strip:
	@$(STRIP) --strip-unneeded --strip-debug $(PREFIX)/lib/* 2>/dev/null || echo "Ignore lib strip error"
	@$(STRIP) --strip-all $(PREFIX)/usr/bin/* 2>/dev/null || echo "Ignore bin strip error"
	@$(STRIP) --strip-all $(PREFIX)/sbin/* 2>/dev/null || echo "Ignore strip sbin error"
	@$(STRIP) --strip-all $(PREFIX)/usr/local/bin/* 2>/dev/null || echo "Ignore strip usr/local/bin error"
	@$(STRIP) --strip-debug $(PREFIX)/lib/* 2>/dev/null || echo "Ignore strip usr/local/bin error"

bin:
	rm -rf $(PREFIX)/include $(PREFIX)/man $(PREFIX)/share $(PREFIX)/lib/*.a $(PREFIX)/lib/*.la	$(PREFIX)/ssl/man

clean:
	make -C lib clean
	make -C src clean
	rm -rf $(IMAGES_DIR)

distclean:clean
	rm -rf $(PREFIX)
	rm -rf $(IMAGES_DIR)
	rm -rf $(DIR_NAME)

.PHONY: default prepare distclean clean build_all build_lib build_app