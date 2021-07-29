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
    mv images/ast1520-h/all.bin images/IPE5000-A30_${ver}.bin
    mv images/ast1520-h/webfwh.bin images/IPE5000-A30_upgrade_${ver}.bin
    rm -rf images/ast1520-h
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
    mv images/ast1520-c/allc.bin images/IPD5000-A30_${ver}.bin
    mv images/ast1520-c/webfwc.bin images/IPD5000-A30_upgrade_${ver}.bin
    rm -rf images/ast1520-c
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


