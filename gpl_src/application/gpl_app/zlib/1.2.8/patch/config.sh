#!/bin/sh

. make_define.sh

CROSS_PREFIX=$(CROSS_COMPILE) ./configure --prefix=$(PREFIX) || exit 1
