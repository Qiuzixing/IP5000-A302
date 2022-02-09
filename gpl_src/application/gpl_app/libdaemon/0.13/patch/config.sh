#!/bin/sh

. make_define.sh

./configure --host=arm-linux --build=i686-linux --prefix=$PREFIX --disable-lynx CC=${CROSS_COMPILE}gcc RANLIB=${CROSS_COMPILE}ranlib LD=${CROSS_COMPILE}ld CXX=${CROSS_COMPILE}g++ || exit 1
