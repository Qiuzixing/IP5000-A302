#/bin/sh

gcc bin2hex.c -o bin2hex
./bin2hex osd0_xrgb1555.bmp osd_0.h
./bin2hex osd1_xrgb1555.bmp osd_1.h
./bin2hex osd2_xrgb1555.bmp osd_2.h
./bin2hex osd3_xrgb1555.bmp osd_3.h
./bin2hex osd4_xrgb1555.bmp osd_4.h
./bin2hex osd5_xrgb1555.bmp osd_5.h
./bin2hex osd6_xrgb1555.bmp osd_6.h
./bin2hex osd7_xrgb1555.bmp osd_7.h
./bin2hex osd8_xrgb1555.bmp osd_8.h
./bin2hex osd9_xrgb1555.bmp osd_9.h

cp -f osd_0.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_1.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_2.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_3.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_4.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_5.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_6.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_7.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_8.h ../../kernel/linux-2.6.15.7/drivers/video/crt/
cp -f osd_9.h ../../kernel/linux-2.6.15.7/drivers/video/crt/

