#!/bin/sh

qmake -config release || exit 1
make clean
make || exit 1

#Now copy files

STRIP=${CROSS_COMPILE}strip

if file GUI | grep -q 'not stripped' ; then
	echo "not stripped! strip it..."
	${STRIP} -F elf32-littlearm GUI || exit 1
fi	

#mv GUI GUI1 || exit 1
#/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/arm-aspeed-linux-gnu/bin/strip -F elf32-littlearm -o GUI GUI1 || exit 1
#arm-aspeed-linux-gnu-strip -F elf32-littlearm -o GUI GUI1 || exit 1
#rm -f GUI1 || exit 1
upx -qqq -v GUI
