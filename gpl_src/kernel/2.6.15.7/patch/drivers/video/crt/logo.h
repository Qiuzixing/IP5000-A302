
#ifndef __LOGO_H__
#define __LOGO_H__

#define NO_LOGO 0

/*
** Some of the defines are referenced from 
** http://www.cnblogs.com/oomusou/archive/2008/05/06/1184240.html
*/

struct s_bmp_file_hdr {
  unsigned short identity;        // 2 byte : "BM"則為BMP
  unsigned int   file_size;       // 4 byte : 檔案size
  unsigned short reserved1;       // 2 byte : 保留欄位,設為0
  unsigned short reserved2;       // 2 byte : 保留欄位,設為0
  unsigned int   data_offset;     // 4 byte : RGB資料開始之前的資料偏移量
} __attribute__ ((packed));

struct s_bmp_dib_hdr {
  unsigned int   header_size;      // 4 byte : struct BITMAPINFOHEADER的size
  int            width;            // 4 byte : 影像寬度(pixel)
  int            height;           // 4 byte : 影像高度(pixel)
  unsigned short planes;           // 2 byte : 設為1
  unsigned short bit_per_pixel;    // 2 byte : 每個pixel所需的位元數(1/4/8/16/24/32)
  unsigned int   compression;      // 4 byte : 壓縮方式, 0 : 未壓縮
  unsigned int   data_size;        // 4 byte : 影像大小,設為0
  int            hresolution;      // 4 byte : pixel/m
  int            vresolution;      // 4 byte : pixel/m
  unsigned int   used_colors;      // 4 byte : 使用調色盤顏色數,0表使用調色盤所有顏色
  unsigned int   important_colors; // 4 byte : 重要顏色數,當等於0或used_colors時,表全部都重要
} __attribute__ ((packed));

struct s_bmp_palette {
  char blue;                       // 1 byte : 調色盤藍色
  char green;                      // 1 byte : 調色盤綠色
  char red;                        // 1 byte : 調色盤紅色
  char reserved;                   // 1 byte : 保留欄位,設為0
} __attribute__ ((packed));

/* This is bmp file structure for color depth >= 16bits ONLY. */
struct s_bmp_file {
	struct s_bmp_file_hdr file_hdr;
	struct s_bmp_dib_hdr dib_hdr;
	unsigned char data[];
} __attribute__ ((packed));

#define LOGO_FROM_FLASH 1
#define MEMORY_TEST 0

#if LOGO_FROM_FLASH
#define LOGO_OFFSET (ASPEED_SMC_FLASH_BASE+0xFC0000)
#define LOGO_SIZE (0x20000) //128KB

static unsigned char logo_file[] = {

#include "logo_black_bmp.h"

};
#else //#if LOGO_FROM_FLASH
static unsigned char logo_file[] = {

#include "logo_bmp.h"

};
#endif //#if LOGO_FROM_FLASH

#if 0
static unsigned char ver_file[] = {

#include "ver_bmp.h"

};
#endif
#endif

