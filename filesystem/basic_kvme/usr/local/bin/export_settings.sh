#!/bin/sh

if [ $# -ne 1 ]; then
    exit 1
fi
PATH_PREFIX=/data/configs/kds-7
SAVED_PREFIX=/dev/shm/settings
METHOD=$1

rm -rf "$SAVED_PREFIX"
mkdir -p "$SAVED_PREFIX"
cd /dev/shm
rm -f settings.*

case $METHOD in
    '0') # All without IP
        cp -ra $PATH_PREFIX/* $SAVED_PREFIX/
        rm -rf $SAVED_PREFIX/network
    ;;
    '1') # Streams
        cp -ra $PATH_PREFIX/channel $SAVED_PREFIX/
    ;;
    '2') # AV Settings Only
        cp -ra $PATH_PREFIX/audio $SAVED_PREFIX/
        cp -ra $PATH_PREFIX/av_setting $SAVED_PREFIX/
        cp -ra $PATH_PREFIX/av_signal $SAVED_PREFIX/
    ;;
    '3')
        cp -ra $PATH_PREFIX/* $SAVED_PREFIX/
    ;;
    *)
        echo > /dev/null
    ;;
esac

tar cvf settings.tar settings
gzip settings.tar
