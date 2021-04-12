#!/bin/sh

. make_define.sh

if [ -f Makefile ]; then
	make distclean || exit 1
fi
