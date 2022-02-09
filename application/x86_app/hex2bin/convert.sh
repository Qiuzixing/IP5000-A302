#/bin/sh

gcc bin2hex.c -o bin2hex
./bin2hex logo_xrgb8888.bmp logo_bmp.h
./bin2hex ver_xrgb8888.bmp ver_bmp.h

cp -f logo_bmp.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f ver_bmp.h ../../kernel/linux-2.6.15.7/drivers/video/crt/