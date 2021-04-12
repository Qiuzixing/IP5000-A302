#!/bin/sh

. make_define.sh

./configure --host=arm-linux --build=i686-linux CC=${CROSS_COMPILE}gcc CXX=${CROSS_COMPILE}g++ RANLIB=${CROSS_COMPILE}ranlib LD=${CROSS_COMPILE}ld --prefix=${PREFIX} CFLAGS="-I${XTOOL_PREFIX}/include" LDFLAGS="-L${XTOOL_PREFIX}/lib" || exit 1

