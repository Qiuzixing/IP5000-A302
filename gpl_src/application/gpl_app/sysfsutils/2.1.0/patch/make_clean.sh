#!/bin/sh

. make_define.sh

if [ -f Makefile ]; then
	make distclean &&
	rm -f bruce.cache || exit 1
fi
