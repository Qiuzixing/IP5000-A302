#!/bin/sh

. make_define.sh

if [ -f Makefile ]; then
	make distclean &&
	rm -f .config || exit 1
fi
