#!/bin/sh

. make_define.sh

cp -f mk.cache bruce.cache
./configure --host=arm-linux --build=i686-linux CC=${CROSS_COMPILE}gcc RANLIB=${CROSS_COMPILE}ranlib LD=${CROSS_COMPILE}ld --prefix=${PREFIX} CFLAGS="-I${XTOOL_PREFIX}/include -L${XTOOL_PREFIX}/lib" --cache-file=bruce.cache || exit 1

## Error due to script can't test some parameters while cross compiling
## See http://library.gnome.org/devel/glib/unstable/glib-cross-compiling.html
## Let's create the "cache file"
## [root@localhost glib-2.12.12]# vi bruce.cache
## glib_cv_stack_grows=no
## glib_cv_uscore=no
## ac_cv_func_posix_getpwuid_r=no

