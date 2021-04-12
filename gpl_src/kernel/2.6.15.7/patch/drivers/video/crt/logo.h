
#ifndef __LOGO_H__
#define __LOGO_H__

#define NO_LOGO 0

/*
** Some of the defines are referenced from 
** http://www.cnblogs.com/oomusou/archive/2008/05/06/1184240.html
*/

struct s_bmp_file_hdr {
  unsigned short identity;        // 2 byte : "BM"�h��BMP
  unsigned int   file_size;       // 4 byte : �ɮ�size
  unsigned short reserved1;       // 2 byte : �O�d���,�]��0
  unsigned short reserved2;       // 2 byte : �O�d���,�]��0
  unsigned int   data_offset;     // 4 byte : RGB��ƶ}�l���e����ư����q
} __attribute__ ((packed));

struct s_bmp_dib_hdr {
  unsigned int   header_size;      // 4 byte : struct BITMAPINFOHEADER��size
  int            width;            // 4 byte : �v���e��(pixel)
  int            height;           // 4 byte : �v������(pixel)
  unsigned short planes;           // 2 byte : �]��1
  unsigned short bit_per_pixel;    // 2 byte : �C��pixel�һݪ��줸��(1/4/8/16/24/32)
  unsigned int   compression;      // 4 byte : ���Y�覡, 0 : �����Y
  unsigned int   data_size;        // 4 byte : �v���j�p,�]��0
  int            hresolution;      // 4 byte : pixel/m
  int            vresolution;      // 4 byte : pixel/m
  unsigned int   used_colors;      // 4 byte : �ϥνզ�L�C���,0��ϥνզ�L�Ҧ��C��
  unsigned int   important_colors; // 4 byte : ���n�C���,����0��used_colors��,����������n
} __attribute__ ((packed));

struct s_bmp_palette {
  char blue;                       // 1 byte : �զ�L�Ŧ�
  char green;                      // 1 byte : �զ�L���
  char red;                        // 1 byte : �զ�L����
  char reserved;                   // 1 byte : �O�d���,�]��0
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

