#/bin/sh

gcc hex2bin.c -o hex2bin
# $1 is source, $2 is output
./hex2bin "$1" "$2"
