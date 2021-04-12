/*
**
** Copyright (c) 2004-2015
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#ifndef _HDMI_DEF_H_
#define _HDMI_DEF_H_

#include <linux/types.h>

/******************************************************************************
  VIC Code Reference: (Get from Wikipedia "E-EDID")

   Code     Picture Pixel
   Short    Aspect  Aspect
   Name     Ratio   Ratio                HxV @ F

  1 DMT0659    4:3                  640x480p @ 59.94/60 Hz
  2 480p       4:3     8:9          720x480p @ 59.94/60 Hz
  3 480pH     16:9    32:37         720x480p @ 59.94/60 Hz
  4 720p      16:9     1:1         1280x720p @ 59.94/60 Hz
  5 1080i     16:9     1:1        1920x1080i @ 59.94/60 Hz
  6 480i       4:3     8:9    720(1440)x480i @ 59.94/60 Hz
  7 480iH     16:9    32:37   720(1440)x480i @ 59.94/60 Hz
  8 240p       4:3     8:9    720(1440)x240p @ 59.94/60 Hz
  9 240pH     16:9    32:37   720(1440)x240p @ 59.94/60 Hz
 10 480i4x     4:3     8:9       (2880)x480i @ 59.94/60 Hz
 11 480i4xH   16:9    32:37      (2880)x480i @ 59.94/60 Hz
 12 240p4x     4:3     8:9       (2880)x240p @ 59.94/60 Hz
 13 240p4xH   16:9    32:37      (2880)x240p @ 59.94/60 Hz
 14 480p2x     4:3     8:9         1440x480p @ 59.94/60 Hz
 15 480p2xH   16:9    32:37        1440x480p @ 59.94/60 Hz
 16 1080p     16:9     1:1        1920x1080p @ 59.94/60 Hz
 17 576p       4:3    16:15         720x576p @ 50 Hz
 18 576pH     16:9    64:45         720x576p @ 50 Hz
 19 720p50    16:9     1:1        1280x720p @ 50 Hz
 20 1080i25   16:9     1:1        1920x1080i @ 50 Hz*
 21 576i       4:3    16:15   720(1440)x576i @ 50 Hz
 22 576iH     16:9    64:45   720(1440)x576i @ 50 Hz
 23 288p       4:3    16:15   720(1440)x288p @ 50 Hz
 24 288pH     16:9    64:45   720(1440)x288p @ 50 Hz
 25 576i4x     4:3    16:15      (2880)x576i @ 50 Hz
 26 576i4xH   16:9    64:45      (2880)x576i @ 50 Hz
 27 288p4x     4:3    16:15      (2880)x288p @ 50 Hz
 28 288p4xH   16:9    64:45      (2880)x288p @ 50 Hz
 29 576p2x     4:3    16:15        1440x576p @ 50 Hz
 30 576p2xH   16:9    64:45        1440x576p @ 50 Hz
 31 1080p50   16:9     1:1        1920x1080p @ 50 Hz
 32 1080p24   16:9     1:1        1920x1080p @ 23.98/24 Hz
 33 1080p25   16:9     1:1        1920x1080p @ 25 Hz
 34 1080p30   16:9     1:1        1920x1080p @ 29.97/30 Hz
 35 480p4x     4:3     8:9       (2880)x480p @ 59.94/60 Hz
 36 480p4xH   16:9    32:37      (2880)x480p @ 59.94/60 Hz
 37 576p4x    4:3     16:15      (2880)x576p @ 50 Hz
 38 576p4xH   16:9    64:45      (2880)x576p @ 50 Hz
 39 1080i25   16:9     1:1        1920x1080i @ 50 Hz* (1250 Total)
 40 1080i50   16:9     1:1        1920x1080i @ 100 Hz
 41 720p100   16:9     1:1         1280x720p @ 100 Hz
 42 576p100    4:3     8:9          720x576p @ 100 Hz
 43 576p100H  16:9    32:37         720x576p @ 100 Hz
 44 576i50     4:3    16:15   720(1440)x576i @ 100 Hz
 45 576i50H   16:9    64:45   720(1440)x576i @ 100 Hz
 46 1080i60   16:9     1:1        1920x1080i @ 119.88/120 Hz
 47 720p120   16:9     1:1         1280x720p @ 119.88/120 Hz
 48 480p119    4:3    16:15         720x480p @ 119.88/120 Hz
 49 480p119H  16:9    64:45         720x480p @ 119.88/120 Hz
 50 480i59     4:3     8:9    720(1440)x480i @ 119.88/120 Hz
 51 480i59H   16:9    32:37   720(1440)x480i @ 119.88/120 Hz
 52 576p200    4:3    16:15         720x576p @ 200 Hz
 53 576p200H  16:9    64:45         720x576p @ 200 Hz
 54 576i100    4:3    16:15   720(1440)x576i @ 200 Hz
 55 576i100H  16:9    64:45   720(1440)x576i @ 200 Hz
 56 480p239    4:3     8:9          720x480p @ 239.76/240 Hz
 57 480p239H  16:9    32:37         720x480p @ 239.76/240 Hz
 58 480i119    4:3     8:9    720(1440)x480i @ 239.76/240 Hz
 59 480i119H  16:9    32:37   720(1440)x480i @ 239.76/240 Hz
 60 720p24    16:9     1:1         1280x720p @ 23.98/24 Hz
 61 720p25    16:9     1:1         1280x720p @ 25Hz
 62 720p30    16:9     1:1         1280x720p @ 29.97/30 Hz
 63 1080p120  16:9     1:1        1920x1080p @ 119.88/120 Hz
 64 1080p100  16:9     1:1        1920x1080p @ 100 Hz
 65 720p24    64:27    4:3         1280x720p @ 23.98/24 Hz
 66 720p25    64:27    4:3         1280x720p @ 25Hz
 67 720p30    64:27    4:3         1280x720p @ 29.97/30 Hz
 68 720p50    64:27    4:3         1280x720p @ 50 Hz
 69 720p      64:27    4:3         1280x720p @ 59.94/60 Hz
 70 720p100   64:27    4:3         1280x720p @ 100 Hz
 71 720p120   64:27    4:3         1280x720p @ 119.88/120 Hz
 72 1080p24   64:27    4:3        1920x1080p @ 23.98/24 Hz
 73 1080p25   64:27    4:3        1920x1080p @ 25Hz
 74 1080p30   64:27    4:3        1920x1080p @ 29.97/30 Hz
 75 1080p50   64:27    4:3        1920x1080p @ 50 Hz
 76 1080p     64:27    4:3        1920x1080p @ 59.94/60 Hz
 77 1080p100  64:27    4:3        1920x1080p @ 100 Hz
 78 1080p120  64:27    4:3        1920x1080p @ 119.88/120 Hz
 79  720p24   64:27   64:63        1680x720p @ 23.98/24 Hz
 80  720p25   64:27   64:63        1680x720p @ 25Hz
 81  720p30   64:27   64:63        1680x720p @ 29.97/30 Hz
 82  720p50   64:27   64:63        1680x720p @ 50 Hz
 83  720p     64:27   64:63        1680x720p @ 59.94/60 Hz
 84  720p100  64:27   64:63        1680x720p @ 100 Hz
 85  720p120  64:27   64:63        1680x720p @ 119.88/120 Hz
 86 1080p24   64:27    1:1        2560x1080p @ 23.98/24 Hz
 87 1080p25   64:27    1:1        2560x1080p @ 25Hz
 88 1080p30   64:27    1:1        2560x1080p @ 29.97/30 Hz
 89 1080p50   64:27    1:1        2560x1080p @ 50 Hz
 90 1080p     64:27    1:1        2560x1080p @ 59.94/60 Hz
 91 1080p100  64:27    1:1        2560x1080p @ 100 Hz
 92 1080p120  64:27    1:1        2560x1080p @ 119.88/120 Hz
 93 2160p24   16:9     1:1        3840x2160p @ 23.98/24 Hz
 94 2160p25   16:9     1:1        3840x2160p @ 25Hz
 95 2160p30   16:9     1:1        3840x2160p @ 29.97/30 Hz
 96 2160p50   16:9     1:1        3840x2160p @ 50 Hz
 97 2160p     16:9     1:1        3840x2160p @ 59.94/60 Hz
 98 2160p24  256:135   1:1        4096x2160p @ 23.98/24 Hz
 99 2160p25  256:135   1:1        4096x2160p @ 25Hz
100 2160p30  256:135   1:1        4096x2160p @ 29.97/30 Hz
101 2160p50  256:135   1:1        4096x2160p @ 50 Hz
102 2160p    256:135   1:1        4096x2160p @ 59.94/60 Hz
103 2160p24   64:27    4:3        3840x2160p @ 23.98/24 Hz
104 2160p25   64:27    4:3        3840x2160p @ 25Hz
105 2160p30   64:27    4:3        3840x2160p @ 29.97/30 Hz
106 2160p50   64:27    4:3        3840x2160p @ 50 Hz
107 2160p     64:27    4:3        3840x2160p @ 59.94/60 Hz

 0, 108 - 127   Reserved
*******************************************************************************/

typedef enum {
	VIC_unknown    =   0     ,
	VIC_DMT0659    =   1     ,//4:3                  640x480p @ 59.94/60 Hz
	VIC_480p       =   2     ,//4:3     8:9          720x480p @ 59.94/60 Hz
	VIC_480pH      =   3     ,//16:9    32:37         720x480p @ 59.94/60 Hz
	VIC_720p       =   4     ,//16:9     1:1         1280x720p @ 59.94/60 Hz
	VIC_1080i      =   5     ,//16:9     1:1        1920x1080i @ 59.94/60 Hz
	VIC_480i       =   6     ,// 4:3     8:9    720(1440)x480i @ 59.94/60 Hz
	VIC_480iH      =   7     ,//16:9    32:37   720(1440)x480i @ 59.94/60 Hz
	VIC_240p       =   8     ,// 4:3     8:9    720(1440)x240p @ 59.94/60 Hz
	VIC_240pH      =   9     ,//16:9    32:37   720(1440)x240p @ 59.94/60 Hz
	VIC_480i4x     =  10     ,// 4:3     8:9       (2880)x480i @ 59.94/60 Hz
	VIC_480i4xH    =  11     ,//16:9    32:37      (2880)x480i @ 59.94/60 Hz
	VIC_240p4x     =  12     ,// 4:3     8:9       (2880)x240p @ 59.94/60 Hz
	VIC_240p4xH    =  13     ,//16:9    32:37      (2880)x240p @ 59.94/60 Hz
	VIC_480p2x     =  14     ,// 4:3     8:9         1440x480p @ 59.94/60 Hz
	VIC_480p2xH    =  15     ,//16:9    32:37        1440x480p @ 59.94/60 Hz
	VIC_1080p      =  16     ,//16:9     1:1        1920x1080p @ 59.94/60 Hz
	VIC_576p       =  17     ,// 4:3    16:15         720x576p @ 50 Hz
	VIC_576pH      =  18     ,//16:9    64:45         720x576p @ 50 Hz
	VIC_720p50     =  19     ,//16:9     1:1        1280x720p @ 50 Hz
	VIC_1080i25    =  20     ,//16:9     1:1        1920x1080i @ 50 Hz*
	VIC_576i       =  21     ,// 4:3    16:15   720(1440)x576i @ 50 Hz
	VIC_576iH      =  22     ,//16:9    64:45   720(1440)x576i @ 50 Hz
	VIC_288p       =  23     ,// 4:3    16:15   720(1440)x288p @ 50 Hz
	VIC_288pH      =  24     ,//16:9    64:45   720(1440)x288p @ 50 Hz
	VIC_576i4x     =  25     ,// 4:3    16:15      (2880)x576i @ 50 Hz
	VIC_576i4xH    =  26     ,//16:9    64:45      (2880)x576i @ 50 Hz
	VIC_288p4x     =  27     ,// 4:3    16:15      (2880)x288p @ 50 Hz
	VIC_288p4xH    =  28     ,//16:9    64:45      (2880)x288p @ 50 Hz
	VIC_576p2x     =  29     ,// 4:3    16:15        1440x576p @ 50 Hz
	VIC_576p2xH    =  30     ,//16:9    64:45        1440x576p @ 50 Hz
	VIC_1080p50    =  31     ,//16:9     1:1        1920x1080p @ 50 Hz
	VIC_1080p24    =  32     ,//16:9     1:1        1920x1080p @ 23.98/24 Hz
	VIC_1080p25    =  33     ,//16:9     1:1        1920x1080p @ 25 Hz
	VIC_1080p30    =  34     ,//16:9     1:1        1920x1080p @ 29.97/30 Hz
	VIC_480p4x     =  35     ,// 4:3     8:9       (2880)x480p @ 59.94/60 Hz
	VIC_480p4xH    =  36     ,//16:9    32:37      (2880)x480p @ 59.94/60 Hz
	VIC_576p4x     =  37     ,//4:3     16:15      (2880)x576p @ 50 Hz
	VIC_576p4xH    =  38     ,//16:9    64:45      (2880)x576p @ 50 Hz
	VIC_1080i25A   =  39     ,//16:9     1:1        1920x1080i @ 50 Hz* (1250 Total)
	VIC_1080i50    =  40     ,//16:9     1:1        1920x1080i @ 100 Hz
	VIC_720p100    =  41     ,//16:9     1:1         1280x720p @ 100 Hz
	VIC_576p100    =  42     ,// 4:3     8:9          720x576p @ 100 Hz
	VIC_576p100H   =  43     ,//16:9    32:37         720x576p @ 100 Hz
	VIC_576i50     =  44     ,// 4:3    16:15   720(1440)x576i @ 100 Hz
	VIC_576i50H    =  45     ,//16:9    64:45   720(1440)x576i @ 100 Hz
	VIC_1080i60    =  46     ,//16:9     1:1        1920x1080i @ 119.88/120 Hz
	VIC_720p120    =  47     ,//16:9     1:1         1280x720p @ 119.88/120 Hz
	VIC_480p119    =  48     ,// 4:3    16:15         720x480p @ 119.88/120 Hz
	VIC_480p119H   =  49     ,//16:9    64:45         720x480p @ 119.88/120 Hz
	VIC_480i59     =  50     ,// 4:3     8:9    720(1440)x480i @ 119.88/120 Hz
	VIC_480i59H    =  51     ,//16:9    32:37   720(1440)x480i @ 119.88/120 Hz
	VIC_576p200    =  52     ,// 4:3    16:15         720x576p @ 200 Hz
	VIC_576p200H   =  53     ,//16:9    64:45         720x576p @ 200 Hz
	VIC_576i100    =  54     ,// 4:3    16:15   720(1440)x576i @ 200 Hz
	VIC_576i100H   =  55     ,//16:9    64:45   720(1440)x576i @ 200 Hz
	VIC_480p239    =  56     ,// 4:3     8:9          720x480p @ 239.76/240 Hz
	VIC_480p239H   =  57     ,//16:9    32:37         720x480p @ 239.76/240 Hz
	VIC_480i119    =  58     ,// 4:3     8:9    720(1440)x480i @ 239.76/240 Hz
	VIC_480i119H   =  59     ,//16:9    32:37   720(1440)x480i @ 239.76/240 Hz
	VIC_720p24     =  60     ,//16:9     1:1         1280x720p @ 23.98/24 Hz
	VIC_720p25     =  61     ,//16:9     1:1         1280x720p @ 25Hz
	VIC_720p30     =  62     ,//16:9     1:1         1280x720p @ 29.97/30 Hz
	VIC_1080p120   =  63     ,//16:9     1:1        1920x1080p @ 119.88/120 Hz
	VIC_1080p100   =  64     ,//16:9     1:1        1920x1080p @ 100 Hz
	VIC_720p24W1   =  65     ,//64:27    4:3         1280x720p @ 23.98/24 Hz
	VIC_720p25W1   =  66     ,//64:27    4:3         1280x720p @ 25Hz
	VIC_720p30W1   =  67     ,//64:27    4:3         1280x720p @ 29.97/30 Hz
	VIC_720p50W1   =  68     ,//64:27    4:3         1280x720p @ 50 Hz
	VIC_720pW1     =  69     ,//64:27    4:3         1280x720p @ 59.94/60 Hz
	VIC_720p100W1  =  70     ,//64:27    4:3         1280x720p @ 100 Hz
	VIC_720p120W1  =  71     ,//64:27    4:3         1280x720p @ 119.88/120 Hz
	VIC_1080p24W1  =  72     ,//64:27    4:3        1920x1080p @ 23.98/24 Hz
	VIC_1080p25W1  =  73     ,//64:27    4:3        1920x1080p @ 25Hz
	VIC_1080p30W1  =  74     ,//64:27    4:3        1920x1080p @ 29.97/30 Hz
	VIC_1080p50W1  =  75     ,//64:27    4:3        1920x1080p @ 50 Hz
	VIC_1080pW1    =  76     ,//64:27    4:3        1920x1080p @ 59.94/60 Hz
	VIC_1080p100W1 =  77     ,//64:27    4:3        1920x1080p @ 100 Hz
	VIC_1080p120W1 =  78     ,//64:27    4:3        1920x1080p @ 119.88/120 Hz
	VIC_720p24W2   =  79     ,//64:27   64:63        1680x720p @ 23.98/24 Hz
	VIC_720p25W2   =  80     ,//64:27   64:63        1680x720p @ 25Hz
	VIC_720p30W2   =  81     ,//64:27   64:63        1680x720p @ 29.97/30 Hz
	VIC_720p50W2   =  82     ,//64:27   64:63        1680x720p @ 50 Hz
	VIC_720pW2     =  83     ,//64:27   64:63        1680x720p @ 59.94/60 Hz
	VIC_720p100W2  =  84     ,//64:27   64:63        1680x720p @ 100 Hz
	VIC_720p120W2  =  85     ,//64:27   64:63        1680x720p @ 119.88/120 Hz
	VIC_1080p24W3  =  86     ,//64:27    1:1        2560x1080p @ 23.98/24 Hz
	VIC_1080p25W3  =  87     ,//64:27    1:1        2560x1080p @ 25Hz
	VIC_1080p30W3  =  88     ,//64:27    1:1        2560x1080p @ 29.97/30 Hz
	VIC_1080p50W3  =  89     ,//64:27    1:1        2560x1080p @ 50 Hz
	VIC_1080pW3    =  90     ,//64:27    1:1        2560x1080p @ 59.94/60 Hz
	VIC_1080p100W3 =  91     ,//64:27    1:1        2560x1080p @ 100 Hz
	VIC_1080p120W3 =  92     ,//64:27    1:1        2560x1080p @ 119.88/120 Hz
	VIC_2160p24    =  93     ,//16:9     1:1        3840x2160p @ 23.98/24 Hz
	VIC_2160p25    =  94     ,//16:9     1:1        3840x2160p @ 25Hz
	VIC_2160p30    =  95     ,//16:9     1:1        3840x2160p @ 29.97/30 Hz
	VIC_2160p50    =  96     ,//16:9     1:1        3840x2160p @ 50 Hz
	VIC_2160p      =  97     ,//16:9     1:1        3840x2160p @ 59.94/60 Hz
	VIC_2160p24E   =  98     ,//256:135   1:1        4096x2160p @ 23.98/24 Hz
	VIC_2160p25E   =  99     ,//256:135   1:1        4096x2160p @ 25Hz
	VIC_2160p30E   = 100     ,//256:135   1:1        4096x2160p @ 29.97/30 Hz
	VIC_2160p50E   = 101     ,//256:135   1:1        4096x2160p @ 50 Hz
	VIC_2160pE     = 102     ,//256:135   1:1        4096x2160p @ 59.94/60 Hz
	VIC_2160p24W1  = 103     ,// 64:27    4:3        3840x2160p @ 23.98/24 Hz
	VIC_2160p25W1  = 104     ,// 64:27    4:3        3840x2160p @ 25Hz
	VIC_2160p30W1  = 105     ,// 64:27    4:3        3840x2160p @ 29.97/30 Hz
	VIC_2160p50W1  = 106     ,// 64:27    4:3        3840x2160p @ 50 Hz
	VIC_2160pW1    = 107     ,// 64:27    4:3        3840x2160p @ 59.94/60 Hz
	VIC_MAX
} e_hdmi_vic;

enum hdmi_infoframe_type {
	HDMI_INFOFRAME_TYPE_VENDOR	= 0x81,
	HDMI_INFOFRAME_TYPE_AVI		= 0x82,
	HDMI_INFOFRAME_TYPE_SPD		= 0x83,
	HDMI_INFOFRAME_TYPE_AUDIO	= 0x84,
	HDMI_INFOFRAME_TYPE_DRM		= 0x87,
};

#define HDMI_VENDORSPEC_INFOFRAME_TYPE 0x01
#define HDMI_AVI_INFOFRAME_TYPE  0x02
#define HDMI_SPD_INFOFRAME_TYPE 0x03
#define HDMI_AUDIO_INFOFRAME_TYPE 0x04
#define HDMI_MPEG_INFOFRAME_TYPE 0x05

#define HDMI_VENDORSPEC_INFOFRAME_VER 0x01
#define HDMI_AVI_INFOFRAME_VER  0x02
#define HDMI_SPD_INFOFRAME_VER 0x01
#define HDMI_AUDIO_INFOFRAME_VER 0x01
#define HDMI_MPEG_INFOFRAME_VER 0x01

#define HDMI_H14B_VSIF_LEN 8
#define HDMI_HF_VSIF_LEN 27
#define HDMI_MAX_VSIF_LEN HDMI_HF_VSIF_LEN
#define HDMI_AVI_INFOFRAME_LEN 13
#define HDMI_SPD_INFOFRAME_LEN 25
#define HDMI_AUDIO_INFOFRAME_LEN 10
#define HDMI_MPEG_INFOFRAME_LEN 10
#define HDMI_HDR_LEN 26


#define AVIINFO_COLOR_MODE_RGB444  0
#define AVIINFO_COLOR_MODE_YUV422  1
#define AVIINFO_COLOR_MODE_YUV444  2
#define AVIINFO_COLOR_MODE_YUV420  3
#define AVIINFO_CODED_FRAME_ASPECT_NA    0
#define AVIINFO_CODED_FRAME_ASPECT_4x3   1
#define AVIINFO_CODED_FRAME_ASPECT_16x9  2
#define AVIINFO_COLORIMETRY_NA     0
#define AVIINFO_COLORIMETRY_ITU601 1
#define AVIINFO_COLORIMETRY_ITU709 2
#define AVIINFO_COLORIMETRY_EXTEND 3

#define AVIINFO_EXTENDED_COLORIMETRY_XV_YCC_601		0
#define AVIINFO_EXTENDED_COLORIMETRY_XV_YCC_709		1
#define AVIINFO_EXTENDED_COLORIMETRY_S_YCC_601		2
#define AVIINFO_EXTENDED_COLORIMETRY_ADOBE_YCC_601	3
#define AVIINFO_EXTENDED_COLORIMETRY_ADOBE_RGB		4
#define AVIINFO_EXTENDED_COLORIMETRY_BT2020_CONST_LUM	5
#define AVIINFO_EXTENDED_COLORIMETRY_BT2020		6
#define AVIINFO_EXTENDED_COLORIMETRY_RESERVED		7

#define AVIINFO_RGB_RANGE_DFLT    0
#define AVIINFO_RGB_RANGE_LMTD    1
#define AVIINFO_RGB_RANGE_FULL    2

#define AVIINFO_YUV_RANGE_LMTD    0
#define AVIINFO_YUV_RANGE_FULL    1

/* AVI InfoFrame */
typedef union
{
	struct {
		u8 Type;
		u8 Ver;
		u8 Len;

		u8 Checksum;

		u8 Scan:2,
		   BarInfo:2,
		   ActiveFmtInfoPresent:1,
		   ColorMode:2,
		   FU1:1;

		u8 ActiveFormatAspectRatio:4,
		   PictureAspectRatio:2,
		   Colorimetry:2;

		u8 Scaling:2,
		   Quantization:2,
		   ExtendedColorimetry:3,
		   ITContent:1;

		u8 VIC:7,
		   FU3:1;

		u8 PixelRepetition:4,
		   ContentType:2,
		   YccQuantization:2;

		u16 Ln_End_Top;
		u16 Ln_Start_Bottom;
		u16 Pix_End_Left;
		u16 Pix_Start_Right;
	}__attribute__ ((packed)) info;
	struct {
		u8 AVI_HB[3] ;
		u8 AVI_PB[1 + HDMI_AVI_INFOFRAME_LEN] ; // one more byte for checksum
	} __attribute__ ((packed)) pktbyte ;
} __attribute__ ((packed)) hdmi_avi_infoframe;


/* Vendor Specific InfoFrame */
typedef union
{
	struct {
		u8 Type;
		u8 Ver;
		u8 Len;

		u8 Checksum;

		u8 OUI_03;
		u8 OUI_0C;
		u8 OUI_00;

		u8 FU:5,
		   VideoFormat:3; /* 001 */

		u8 HDMI_VIC;
	}__attribute__ ((packed)) H14b4K;
	struct {
		u8 Type;
		u8 Ver;
		u8 Len;

		u8 Checksum;

		u8 OUI_03;
		u8 OUI_0C;
		u8 OUI_00;

		u8 FU1:5,
		   VideoFormat:3; /* 010 */

		u8 FU2:4,
		   ThreeDStruct:4;

		u8 FU3:4,
		   ThreeDExtData:4;
	}__attribute__ ((packed)) H14b3D;
	struct {
		u8 Type;
		u8 Ver;
		u8 Len;

		u8 Checksum;

		u8 OUI_D8;
		u8 OUI_5D;
		u8 OUI_C4;

		u8 Version; /* 1 */

		u8 ThreeDValid:1,
		   FU:7;

		//TODO;
	} __attribute__((packed)) HF;
	struct {
		u8 VSIF_HB[3] ;
		u8 VSIF_PB[1 + HDMI_MAX_VSIF_LEN] ; // one more byte for checksum
	} __attribute__ ((packed)) pktbyte ;
} __attribute__ ((packed)) hdmi_vsif;


/* Audio InfoFrame */
typedef union {
	struct {
		u8 Type;
		u8 Ver;
		u8 Len;

		u8 Checksum;

		u8 ChannelCount:3,
		   R0:1,
		   CodingType:4;

		u8 SampleSize:2,
		   SampleFrequency:3,
		   R1:3;

		u8 R2; /* data byte 3 is reserved and shall be zero */

		u8 ChannelAllocation;

		u8 LFEPlaybackLevelInformation:2,
		   R3:1,
		   LevelShiftValue:4,
		   DownmixInhibit:1;
		u8 reserved[5];
	} __attribute__ ((packed)) info;
	struct {
		u8 AUDIO_HB[3];
		u8 AUDIO_PB[1 + HDMI_AUDIO_INFOFRAME_LEN] ; /* one more byte for checksum */
	} __attribute__ ((packed)) pktbyte;
} __attribute__ ((packed)) hdmi_audio_infoframe;


/* HDR InfoFrame */
typedef union {
	struct {
		u8 type;
		u8 version;
		u8 length;

		u8 checksum;

		u8 eotf:3,	/* EOTF of stream */
		   reserved_0:5; /* shall be zero */

		u8 id:3, /* static metadata descriptor id */
		   reserved_1:5; /* shall be zero */

		u16 dp_x_0; /* display primary x[0] */
		u16 dp_y_0; /* display primary y[0] */

		u16 dp_x_1; /* display primary x[1] */
		u16 dp_y_1; /* display primary y[1] */

		u16 dp_x_2; /* display primary x[2] */
		u16 dp_y_2; /* display primary y[2] */

		u16 wp_x; /* while point x */
		u16 wp_y; /* while point y */

		u16 max_luminance; /* maximum display luminance */
		u16 min_luminance; /* minimum display luminance */

		u16 max_cll; /* MaxCLL, Maximum Content Light Level */
		u16 max_fall; /* MaxFALL, Maximum Frame-average Light Level */
	} __attribute__ ((packed)) info;

	struct {
		u8 HDR_HB[3];
		u8 HDR_PB[1 + HDMI_HDR_LEN] ; /* one more byte for checksum */
	} __attribute__ ((packed)) pktbyte;
} __attribute__ ((packed)) hdmi_hdr_infoframe;


/* IEC60958 channel status, mode 0 for consumer use */
typedef union {
	struct {

		u8 Use:1,	/* be 0 when mode 0, 0:consumer; 1:professional */
		   LinearPCM:1,	/* be 0 when mode 0, 0:PCM; 1:other */
		   CopyrightInformation:1,
		   AdditionalFormatInformation:3,
		   Mode:2;		/* 0: mode 0*/

		u8 CategoryCode;

		u8 SourceNumber:4,
		   ChannelNumber:2,
		   R0:2;

		u8 SamplingFrequency:4,
		   ClockAccuracy:4;

		u8 WordLength:4,
		   OriginalSamplingFrequency:4;
	} __attribute__ ((packed)) info;

	struct {
		u8 data[5];
	} __attribute__ ((packed)) pktbyte;
} __attribute__ ((packed)) iec60958_channel_status_mode_0;

#endif /* #ifndef _HDMI_DEF_H_ */
