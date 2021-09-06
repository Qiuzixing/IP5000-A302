/****************************************************************************
  名称：   bmp2jpg.c
  功能：   linux下bmp转化为jpeg程序源代码
  日期：   2016.01.08
  注意：   编译时加“-ljpeg”(gcc -o bmp2jpg jpeg.c -ljpeg)
 *****************************************************************************/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/timeb.h>
#include <string.h>
#include <errno.h>
#include "jpeglib.h"

#define JPEG_QUALITY    95  //图片质量

typedef unsigned char   UInt8;
typedef signed short    SInt16;
typedef signed int      SInt32;
typedef unsigned int    UInt32;
typedef float           Float32;

#pragma pack (1)
typedef struct
{
    SInt16 s16Type;         // 位图文件的类型，必须为BM(1-2字节）
    SInt32 s32Size;         // 位图文件的大小，以字节为单位（3-6字节，低位在前）
    SInt16 s16Reserved1;    // 位图文件保留字，必须为0(7-8字节）
    SInt16 s16Reserved2;    // 位图文件保留字，必须为0(9-10字节）
    SInt32 s32OffBits;      // 位图数据的起始位置，以相对于位图（11-14字节，低位在前）文件头的偏移量表示，以字节为单位
} T_BitmapFileHeader;

typedef struct
{
    SInt32 s32Size;         // 本结构所占用字节数（15-18字节）
    SInt32 s32Width;        // 位图的宽度，以像素为单位（19-22字节）
    SInt32 s32Height;       // 位图的高度，以像素为单位（23-26字节）
    SInt16 s16Planes;       // 目标设备的级别，必须为1(27-28字节）
    SInt16 s16BitCount;     // 每个像素所需的位数，（29-30字节），必须是1（双色），4(16色），8(256色）16(高彩色)或24（真彩色）之一
    SInt32 s32Compression;  // 位图压缩类型，（31-34字节），必须是0（不压缩），1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一
    SInt32 s32SizeImage;    // 位图的大小(其中包含了为了补齐行数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
    SInt32 s32XPelsPerMeter;// 位图水平分辨率，每米像素数（39-42字节）
    SInt32 s32YPelsPerMeter;// 位图垂直分辨率，每米像素数（43-46字节)
    SInt32 s32ClrUsed;      // 位图实际使用的颜色表中的颜色数（47-50字节）
    SInt32 s32ClrImportant; // 位图显示过程中重要的颜色数（51-54字节）
} T_BitmapInfoHeader;

typedef struct
{
    T_BitmapFileHeader  tFileHeader;
    T_BitmapInfoHeader  tInfoHeader;
    UInt32              u32Width;
    UInt32              u32Height;
    UInt8*              pRGBData;
} T_Bitmap;
#pragma pack ()

SInt32 Bmp2Jpg(const char* i_pBmpFileName, const char* i_pJpegFileName)
{
    SInt32 s32Err   = 0;
    FILE* pBMPFile  = NULL;
    FILE* pJPEGFile = NULL;
    UInt8* pDataBuf = NULL;
    UInt32 u32ImageSize = 0;
    SInt32 s32Depth     = 3;
    UInt32 u32RGBIdx    = 0;
    JSAMPROW * pRowPtr  = NULL;
    struct jpeg_compress_struct tCompressInfo;
    struct jpeg_error_mgr tJpgErr;
    SInt32 i = 0;

    do
    {
        if((pBMPFile = fopen(i_pBmpFileName, "rb")) == NULL)
        {
            fprintf(stderr, " ** open bmp file failed: %s\n", strerror(errno));
            break;
        }

        T_Bitmap objBitMap;
        if(fread(&objBitMap.tFileHeader.s16Type, sizeof(objBitMap.tFileHeader.s16Type), 1, pBMPFile) != 1)
        {
            fprintf(stderr, " ** read bmp file failed: %s\n", strerror(errno));
            break;
        }

        if (objBitMap.tFileHeader.s16Type != 0x4D42)
        {
            fprintf(stderr, " ** bmp file format error\n");
            break;
        }

        fseek(pBMPFile, 10, SEEK_SET);  // aspeed cross compiler can not align on byte

        if(fread(&objBitMap.tFileHeader.s32OffBits, sizeof(objBitMap.tFileHeader.s32OffBits), 1, pBMPFile) != 1)
        {
            fprintf(stderr, " ** read bmp file failed: %s\n", strerror(errno));
            break;
        }

        if(fread(&objBitMap.tInfoHeader, sizeof(T_BitmapInfoHeader), 1, pBMPFile) != 1)
        {
            fprintf(stderr, " ** read bmp file failed: %s\n", strerror(errno));
            break;
        }

        objBitMap.u32Width = abs(objBitMap.tInfoHeader.s32Width);
        objBitMap.u32Height = abs(objBitMap.tInfoHeader.s32Height);

        if (24 != objBitMap.tInfoHeader.s16BitCount && 32 != objBitMap.tInfoHeader.s16BitCount)
        {
            fprintf(stderr, " ## bmp file is not 24 bits or 32 bits file, not support, bit count is %d\n", objBitMap.tInfoHeader.s16BitCount);
        }

        fseek(pBMPFile, objBitMap.tFileHeader.s32OffBits, SEEK_SET);  // aspeed cross compiler can not align on byte

        // Read bmp image data
        u32ImageSize = objBitMap.tInfoHeader.s32Width * objBitMap.tInfoHeader.s32Height * (objBitMap.tInfoHeader.s16BitCount >> 3);
        pDataBuf = (UInt8*)malloc(u32ImageSize);

#if 1
        SInt32 s32Off = 0;
        SInt32 s32Left = u32ImageSize;
        while((s32Err = fread(pDataBuf + s32Off, 1, s32Left > 10*1024 ? 10*1024 : s32Left, pBMPFile)) > 0)
        {
            s32Off += s32Err;
            s32Left -= s32Err;
            usleep(0);
        }
#else
        s32Err = fread(pDataBuf, 1, u32ImageSize, pBMPFile);
#endif
        if(s32Err == 0)
        {
            if(ferror(pBMPFile))
            {
                fprintf(stderr, " ** read bmp file failed: %s\n", strerror(errno));
                free(pDataBuf);
                fclose(pBMPFile);
                break;
            }
        }

        //Convert BMP to JPG
        tCompressInfo.err = jpeg_std_error(&tJpgErr);
        //* Now we can initialize the JPEG compression object.
        jpeg_create_compress(&tCompressInfo);

        if ((pJPEGFile = fopen(i_pJpegFileName, "w+b")) == NULL)
        {
            fprintf(stderr, " ** open jpg file failed: %s\n", strerror(errno));
            break;
        }
        jpeg_stdio_dest(&tCompressInfo, pJPEGFile);

        int s32Width = objBitMap.tInfoHeader.s32Width;
        int s32Height = objBitMap.tInfoHeader.s32Height;
        int s32HeightDouble = 0;
        int s32WidthDouble = 0;
        if (s32Width < s32Height)
        {
            s32Width *= 2;                              // 4:2:0 sub sampling video
            s32WidthDouble = 1;
        }
        else if (s32Width > s32Height * 2)
        {
            s32Height *= 2;                             // interlaced video
            s32HeightDouble = 1;
        }
        tCompressInfo.image_width = s32Width;           //* image i_s32Width and i_s32Height, in pixels
        tCompressInfo.image_height = s32Height;
        tCompressInfo.input_components = s32Depth;      //* # of color components per pixel
        tCompressInfo.in_color_space = JCS_RGB;         //* colorspace of input image
        jpeg_set_defaults(&tCompressInfo);

        // Now you can set any non-default parameters you wish to.
        // Here we just illustrate the use of quality (quantization table) scaling:

        jpeg_set_quality(&tCompressInfo, JPEG_QUALITY, TRUE ); //* limit to baseline-JPEG values
        jpeg_start_compress(&tCompressInfo, TRUE);

        // 一次写入
        SInt32 j=0;
        pRowPtr = (JSAMPROW *)malloc(s32Height * s32Width * 3);
        UInt8 ** pDataCache = (UInt8**)malloc(objBitMap.tInfoHeader.s32Height * sizeof(UInt8*));

        for(i = 0; i < objBitMap.tInfoHeader.s32Height; i++)
        {
            UInt8 * pLineData = NULL;
            pLineData = (UInt8*)malloc(s32Width*3);
            pDataCache[i]=pLineData;
            for(j=0;j<objBitMap.tInfoHeader.s32Width;j++)
            {
                if (s32WidthDouble)
                {
                    pLineData[j*6+5] = pDataBuf[u32RGBIdx];
                    pLineData[j*6+2] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                    pLineData[j*6+4] = pDataBuf[u32RGBIdx];
                    pLineData[j*6+1] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                    pLineData[j*6+3] = pDataBuf[u32RGBIdx];
                    pLineData[j*6+0] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                }
                else
                {
                    pLineData[j*3+2] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                    pLineData[j*3+1] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                    pLineData[j*3+0] = pDataBuf[u32RGBIdx];
                    u32RGBIdx ++;
                }
                if (32 == objBitMap.tInfoHeader.s16BitCount)
                {
                    u32RGBIdx++;
                }
            }
            if (s32HeightDouble)
            {
                pRowPtr[s32Height - 2*i - 1] = pLineData;                
                pRowPtr[s32Height - 2*i - 2] = pLineData;                
            }
            else
            {
                pRowPtr[s32Height - i - 1] = pLineData;                
            }
        }
        jpeg_write_scanlines(&tCompressInfo, pRowPtr, s32Height);
        jpeg_finish_compress(&tCompressInfo);
        jpeg_destroy_compress(&tCompressInfo);
        for (i = 0; i < objBitMap.tInfoHeader.s32Height; i++)
        {
            free(pDataCache[i]);
        }
        free(pDataCache);

    }
    while (0);
    if (NULL != pRowPtr)
    {
        free(pRowPtr);
    }
    if (NULL != pDataBuf)
    {
        free(pDataBuf);
    }
    if (NULL != pBMPFile)
    {
        fclose(pBMPFile);
    }
    if (NULL != pJPEGFile)
    {
        fclose(pJPEGFile);
    }

    return 0;
}

#if 1
int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, " ** arguments error\n");
        return 1;
    }

    return Bmp2Jpg(argv[1], argv[2]);
}
#endif

