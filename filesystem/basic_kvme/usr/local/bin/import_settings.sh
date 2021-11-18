#!/bin/sh

if [ $# -ne 1 ]; then
    exit 1
fi
PATH_PREFIX=/data/configs/kds-7
IMPORT_PREFIX=/dev/shm
SETTINGS_TAR_FILE=$1

mkdir -p "$PATH_PREFIX"

rm -rf settings
tar zvxf $SETTINGS_TAR_FILE 
cp -rf settings/* $PATH_PREFIX/
rm -rf settings
rm -rf $SETTINGS_TAR_FILE

# TODO Get json back to astparam?

