#!/bin/sh

echo "*********************************************************"
echo "                     Building ntp_client            "
echo "*********************************************************"

make clean
make
make install