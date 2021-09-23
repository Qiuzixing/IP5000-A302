ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../Makefile.param
	include $(PARAM_FILE)
endif

dirs =  log jsoncpp-0.y.z libcivetweb

SUBDIRS = $(dirs)

.PHONY: default $(SUBDIRS) subdirs clean

default:clean
	make prepare
	make subdirs

$(SUBDIRS):
	make -C $@

subdirs: $(SUBDIRS)

prepare:
	test -z $(PREFIX) || mkdir -p $(PREFIX)
	test -L ../prefix || ln -f -s $(PREFIX) ../prefix

clean:
	for dir in $(SUBDIRS) ;\
	do \
		make -C $$dir clean || exit 1;\
	done

install:
	for dir in $(SUBDIRS) ;\
	do \
		make -C $$dir install || exit 1;\
	done
