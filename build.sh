#!/bin/sh

set -e

ver=`sed -n 1p version`

build_IPE5000()
{
    echo "***************************"
    echo "**** BUILDING IPE5000 *****"
    echo "***************************"
    make ast1520-h_cfg
    source ./setenv.sh
    make clean
    make clean_unpacked
    make

    cat images/ast1520-h/version-h
}

build_IPD5000()
{
    echo "***************************"
    echo "**** BUILDING IPD5000 *****"
    echo "***************************"
    make ast1520-c_cfg
    source ./setenv.sh
    make clean
    make clean_unpacked
    make

    cat images/ast1520-c/version-c
}

if [ -z $* ] ; then
    build_IPE5000
    build_IPD5000
else
    for dev in $*
    do
        build_$dev
    done
fi

echo "***************************"
echo "********** DONE ***********"
echo "***************************"


