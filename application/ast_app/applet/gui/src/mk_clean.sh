#!/bin/sh

if [ -f Makefile ]; then
	make clean &&
	rm -f GUI || exit 1
fi
