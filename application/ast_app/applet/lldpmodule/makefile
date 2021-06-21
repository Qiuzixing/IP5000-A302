include conf.mak

ROOTDIR = $(shell pwd)
LIBDIR := ./libdir
THIRDLIBNAME := lldpd-1.0.11
USRCTLNAME := lldpusrctl
BUILDDIR := ./lldpd-1.0.11/build
OUTINCLUDEDIR := ./out/include
$(shell if [ ! -d $(BUILDDIR) ] ;then mkdir $(BUILDDIR);fi)
$(shell if [ ! -d $(OUTINCLUDEDIR) ] ;then mkdir $(OUTINCLUDEDIR);fi)

.PHONY:all
all:
ifeq ($(EXPORT_PLATFORM),X86)
	cd $(BUILDDIR) && ../configure --prefix=$(PREFIX)  --with-privsep-user=nobody --with-privsep-group=nogroup \
			  --enable-lldpmed \
			  --enable-cdp \
			  --enable-fdp \
			  --enable-edp \
			  --enable-sonmp
else 
	cd $(BUILDDIR) && ../configure --prefix=$(PREFIX) CC=$(CC) --host=$(HOST) --with-privsep-user=root --with-privsep-group=root \
			  --enable-lldpmed \
			  --enable-cdp \
			  --enable-fdp \
			  --enable-edp \
			  --enable-sonmp
endif
	cd $(ROOTDIR) 

	make -C  $(THIRDLIBNAME)/build
	cp $(THIRDLIBNAME)/build/src/daemon/lldpd ./out
	cp $(THIRDLIBNAME)/src/lib/lldpctl.h $(LIBDIR)
	cp $(THIRDLIBNAME)/src/lldp-const.h $(LIBDIR)
	cp $(THIRDLIBNAME)/build/src/lib/.libs/liblldpctl.a $(LIBDIR)

	make -C  $(USRCTLNAME)
clean:
	make -C  $(USRCTLNAME) clean
	make -C  $(THIRDLIBNAME)/build distclean
	rm -rf $(BUILDDIR)
	rm -rf out/*
	rm -rf libdir/*

