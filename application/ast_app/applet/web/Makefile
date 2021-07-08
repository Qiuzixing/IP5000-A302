ifeq ($(PARAM_FILE), )
	PARAM_FILE:=./Makefile.param
	include $(PARAM_FILE)
endif

default:distclean prepare build_all bin

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
	rm -rf ${DIR_NAME}
	mkdir -p ${DIR_NAME}
	cp -r $(PREFIX)/* $(DIR_NAME)
	#cp -r patch/* "$(DIR_NAME)"
	#cp -r brntserver/*  "$(DIR_NAME)/usr/local/bin/"
	mv $(DIR_NAME)/ssl ${DIR_NAME}/etc/
	test -d ${DIR_NAME}/bin && chmod +x ${DIR_NAME}/bin/* || true
	test -d ${DIR_NAME}/sbin && chmod +x ${DIR_NAME}/sbin/* || true
	test -d ${DIR_NAME}/usr/bin && chmod +x ${DIR_NAME}/usr/bin/* || true
	test -d ${DIR_NAME}/usr/sbin && chmod +x ${DIR_NAME}/usr/sbin/* || true
	test -d ${DIR_NAME}/usr/local/bin && chmod +x ${DIR_NAME}/usr/local/bin/* || true
	test -d ${DIR_NAME}/usr/local/sbin && chmod +x ${DIR_NAME}/usr/local/sbin/* || true
	test -d ${DIR_NAME}/etc && chmod +x ${DIR_NAME}/etc/* || true
	#test -d ${DIR_NAME}/etc/init.d && chmod +x ${DIR_NAME}/etc/init.d/* || true	
	echo $(DEV_NAME) > ${DIR_NAME}/etc/version
	echo $(VERSION) >> ${DIR_NAME}/etc/version
	echo $(BUILD_DATE) >> ${DIR_NAME}/etc/version
	cd $(DIR_NAME) && tar cvzf update.data *
	#cat setup.sh $(DIR_NAME)/update.data > $(BIN_NAME)
	#chmod +x $(BIN_NAME)

clean:
	make -C lib clean
	make -C src clean
	rm -rf $(IMAGES_DIR)

distclean:clean
	rm -rf $(PREFIX)
	rm -rf $(IMAGES_DIR)
	rm -rf $(DIR_NAME)

