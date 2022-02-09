/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <malloc.h>
#include <post.h>

#include "slt.h"
#define WIN_GLOBALS
#include "type.h"
#include "vreg.h"
#define VESA_GLOBALS
#include "vesa.h"
#include "vfun.h"
#include "vdef.h"
#include "vhace.h"
#include "crt.h"
#include "videotest.h"

#define VHASH_ALIGNMENT			16
#define VHASH_MAX_DST			(32+VHASH_ALIGNMENT)


#if ((CFG_CMD_SLT & CFG_CMD_VIDEOTEST) && defined(CONFIG_SLT))
#include "videotest.h"

#define RAND_MAX 32767 //2^16-1

ULONG randSeed = 1;

void srand(ULONG seed)
{
  randSeed = seed;
}

int rand(void)
{
  randSeed = randSeed * 214013 + 2531011;
  return (int)(randSeed >> 17); //32 -15 = 17
}

//static unsigned char CaptureVideo1Buf1Addr[VIDEO_SOURCE_SIZE], CaptureVideo1Buf2Addr[VIDEO_SOURCE_SIZE], Video1CompressBufAddr[CRYPTO_MAX_CONTEXT];
ULONG pCaptureVideo1Buf1Addr[VIDEO_SOURCE_SIZE/4], pCaptureVideo1Buf2Addr[VIDEO_SOURCE_SIZE/4], pVideo1CompressBufAddr[VIDEO_MAX_STREAM_SIZE/4], pVideo1FlagBufAddr[VIDEO_FLAG_SIZE];
ULONG pCaptureVideo2Buf1Addr[VIDEO_SOURCE_SIZE/4], pCaptureVideo2Buf2Addr[VIDEO_SOURCE_SIZE/4], pVideo2CompressBufAddr[VIDEO_MAX_STREAM_SIZE/4], pVideo2FlagBufAddr[VIDEO_FLAG_SIZE];

ULONG pVHashDstBuffer[VHASH_MAX_DST/4];

ULONG pVideo1DecAddr[VIDEO_SOURCE_SIZE/4];
ULONG pCrt1Addr[VIDEO_SOURCE_SIZE/4];
//ULONG pCap1Addr[VIDEO_SOURCE_SIZE/4];

BOOL AllocateEncodeBufHost(ULONG        MMIOBase,
                    int             nVideo)
{
    //ULONG     Addr;
    //ULONG     dwRegOffset = nVideo * 0x100;

    if (VIDEO1 == nVideo)
    {
        
        //Addr = (ULONG)malloc(pVideoInfo->SrcWidth * pVideoInfo->SrcHeight * 4);
        //pCaptureVideo1Buf1Addr = malloc(VIDEO_SOURCE_SIZE);

        g_CAPTURE_VIDEO1_BUF1_ADDR = vBufAlign((ULONG)pCaptureVideo1Buf1Addr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_1_ADDR_REG, g_CAPTURE_VIDEO1_BUF1_ADDR, BUF_1_ADDR_MASK);
        
        //Addr = (ULONG)malloc(pVideoInfo->SrcWidth * pVideoInfo->SrcHeight * 4);
        //pCaptureVideo1Buf2Addr = malloc(VIDEO_SOURCE_SIZE);

        g_CAPTURE_VIDEO1_BUF2_ADDR = vBufAlign((ULONG)pCaptureVideo1Buf2Addr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_2_ADDR_REG, g_CAPTURE_VIDEO1_BUF2_ADDR, BUF_2_ADDR_MASK);
        
        //Addr = (ULONG)malloc(pVideoInfo->uStreamBufSize.StreamBufSize.RingBufNum * pVideoInfo->uStreamBufSize.StreamBufSize.PacketSize)
        //pVideo1CompressBufAddr = malloc(VIDEO_MAX_STREAM_SIZE);
        g_VIDEO1_COMPRESS_BUF_ADDR = vBufAlign((ULONG)pVideo1CompressBufAddr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_ADDR_REG, g_VIDEO1_COMPRESS_BUF_ADDR, BUF_2_ADDR_MASK);
        
        //Addr = (ULONG)malloc((pVideoInfo->SrcHeigh/64) * pVideoInfo->SrcWidth * 8);
        //g_VIDEO1_CRC_BUF_ADDR = vBufAlign((ULONG)malloc(VIDEO_MAX_STREAM_SIZE));
        //WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_CRC_BUF_ADDR_REG, g_VIDEO1_CRC_BUF_ADDR, BUF_2_ADDR_MASK);
        

        //Addr = (ULONG)malloc(pVideoInfo->SrcHeigh * pVideoInfo->SrcWidth  / 128 (/64*4/8));
        //pVideo1FlagBufAddr = malloc(VIDEO_FLAG_SIZE);
        g_VIDEO1_FLAG_BUF_ADDR = vBufAlign((ULONG)pVideo1FlagBufAddr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_FLAG_BUF_ADDR_REG, g_VIDEO1_FLAG_BUF_ADDR, BUF_2_ADDR_MASK);
    }
    else if (VIDEO2 == nVideo)
    {
        //Addr = (ULONG)malloc(pVideoInfo->SrcWidth * pVideoInfo->SrcHeight * 4);
        //pCaptureVideo2Buf1Addr = malloc(VIDEO_SOURCE_SIZE);
        g_CAPTURE_VIDEO2_BUF1_ADDR = vBufAlign((ULONG)pCaptureVideo2Buf1Addr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO2_BUF_1_ADDR_REG, g_CAPTURE_VIDEO2_BUF1_ADDR, BUF_1_ADDR_MASK);
        
        //Addr = (ULONG)malloc(pVideoInfo->SrcWidth * pVideoInfo->SrcHeight * 4);
        //pCaptureVideo2Buf2Addr = malloc(VIDEO_SOURCE_SIZE);
        g_CAPTURE_VIDEO2_BUF2_ADDR = vBufAlign((ULONG)pCaptureVideo2Buf2Addr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO2_BUF_2_ADDR_REG, g_CAPTURE_VIDEO2_BUF2_ADDR, BUF_2_ADDR_MASK);
        
        //Addr = (ULONG)malloc(pVideoInfo->uStreamBufSize.StreamBufSize.RingBufNum * pVideoInfo->uStreamBufSize.StreamBufSize.PacketSize)
        //pVideo2CompressBufAddr = malloc(VIDEO_MAX_STREAM_SIZE);
        g_VIDEO2_COMPRESS_BUF_ADDR = vBufAlign((ULONG)pVideo2CompressBufAddr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO2_COMPRESS_BUF_ADDR_REG, g_VIDEO2_COMPRESS_BUF_ADDR, BUF_2_ADDR_MASK);
        
        //Addr = (ULONG)malloc((pVideoInfo->SrcHeigh/64) * pVideoInfo->SrcWidth * 8);
        //g_VIDEO1_CRC_BUF_ADDR = vBufAlign((ULONG)malloc(VIDEO_MAX_STREAM_SIZE));
        //WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_CRC_BUF_ADDR_REG, g_VIDEO1_CRC_BUF_ADDR, BUF_2_ADDR_MASK);
        

        //Addr = (ULONG)malloc(pVideoInfo->SrcHeigh * pVideoInfo->SrcWidth  / 128 (/64*4/8));
        //pVideo2FlagBufAddr = malloc(VIDEO_FLAG_SIZE);
        g_VIDEO2_FLAG_BUF_ADDR = vBufAlign((ULONG)pVideo2FlagBufAddr);
        WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO2_FLAG_BUF_ADDR_REG, g_VIDEO2_FLAG_BUF_ADDR, BUF_2_ADDR_MASK);
    }
  
}

/********************************************************/
/* 1. product random data to encode                     */
/* 2. use hash to verify encode function                */
/* 3. use encode stream to decompress original data     */
/********************************************************/
int CodecTest(void)
{
	int		num, i=0, j=0;
	ULONG	ulTemp = 0, ulTemp2;
    int     dwValue;
	ULONG	ulHWWp;
    ULONG   ulHWPt;

	//max size
	ULONG tArray[32/4];
	//mode detection
	BOOL	bExternal = TRUE;
	BOOL	bAnalog = TRUE;
	ULONG	Status;	    

	ULONG ulHashSha1[5] = {0x2a19e99f,0x99b1bb2d,0x9ac82862,0x49205e43,0x6bc4b4d7};
	ULONG aHashDecode[5] = {0x2907a827,0xaf337079,0x47817f1f,0xb0b7cd68,0x8d33bd2};

    //Load pattern to src1 & src2 buffer
    srand(1);

	//Total size : DefWidth*DeHeight*4
	//rand function: 16 bits one time is equal to 2 bytes
	//OutdwmBankMode: 32 bits one time is equal to 4 bytes
	for (i=0; i<g_DefWidth*g_DefHeight*2; i++)
	{
		if (i%2)
		{
		  ulTemp2 = rand();
			ulTemp = (ulTemp2 << 16) | ulTemp;
			//WriteMemoryLongHost(DRAM_BASE, g_CAPTURE_VIDEO1_BUF1_ADDR + ((i-1)/2)*4, ulTemp);
			*(((ULONG *)g_CAPTURE_VIDEO1_BUF1_ADDR) + (i-1)/2) = ulTemp;
			ulTemp = 0;
		}
		else
		{
		  ulTemp = rand();
		}
	}

    /* init encoder engine */
    InitializeVideoEngineHost (0,
			       VIDEO1,
				vModeTable[2].HorPolarity,
				vModeTable[2].VerPolarity);

	/* reset offset pointer register*/    
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, 0, VIDEO_CODEC_TRIGGER | VIDEO_CAPTURE_TRIGGER);

	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_READ_OFFSET_REG, 0, COMPRESS_BUF_READ_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_CODEC_OFFSET_READ, 0, BUF_CODEC_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_PROCESS_OFFSET_REG, 0, COMPRESS_BUF_PROCESS_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_FRAME_END_READ, 0, COMPRESS_FRAME_END_READ_MASK);

    /* start compress stream */
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, MODE_DETECTION_TRIGGER, MODE_DETECTION_TRIGGER);
	//Stream mode: set CODEC trigger first
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, VIDEO_CODEC_TRIGGER, VIDEO_CODEC_TRIGGER);

	//Stream mode: start trigger (only trigger capture bit)
	StartVideoCaptureTriggerHost(0, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG);

    //stop engine
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, 0, 0xFF);

	//read 30 times to get HW write pointer
	for (i=0; i<30000; i++)
		ulHWWp = ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BUF_CODEC_OFFSET_READ);

	g_HashSrcBuffer = g_VIDEO1_COMPRESS_BUF_ADDR + 128; //skip encode data 128 byte
	
    g_HashDstBuffer = v16byteAlign((unsigned long)pVHashDstBuffer);
	ulTemp = 300;


	//Enable hash clock
	WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, EN_HACE, STOP_HACE_MASK);
	g_HashMethod.HashMode = VHASHMODE_SHA1;
	g_HashMethod.DigestLen = 20;
	HashAst(ulTemp, tArray, g_HashMethod.HashMode);
    
	if (strncmp((const char *)tArray, (const char *)ulHashSha1, g_HashMethod.DigestLen))
	{
		printf("[VIDEO] Encoder Test: Wrong\n");
        //ExitVideoTest();
        return VIDEO_ENCODE_FAIL;
	}
	else
	{
		printf("[VIDEO] Encoder Test: Pass\n");
	}

	/******** prepare for next decoding test at the same chip ***********/
	ResetVideoHost();

	dwValue=0;

	do
	{
		dwValue = UnlockVideoRegHost(0, VIDEO_UNLOCK_KEY);
		i++;
	}
	while ((VIDEO_UNLOCK != dwValue) && (i<10));

	if (VIDEO_UNLOCK != dwValue)
	{
		printf("[VIDEO] Video register is locked");
		return VIDEO_UNLOCK_FAIL;
	}

	// allocate decoding buffer
	//Use Compress buffer last time, and Don't need to allocate
    g_VIDEO1_DECODE_BUF_1_ADDR = g_VIDEO1_COMPRESS_BUF_ADDR;
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_1_ADDR_REG, g_VIDEO1_DECODE_BUF_1_ADDR, BUF_1_ADDR_MASK);
    
    //	pVideo1DecAddr = malloc(VIDEO_SOURCE_SIZE);
    g_VIDEO1_DECOMPRESS_BUF_ADDR = vBufAlign((unsigned long)pVideo1DecAddr);    
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_ADDR_REG, g_VIDEO1_DECOMPRESS_BUF_ADDR, COMPRESS_BUF_ADDR_MASK);
    
    //Addr = (ULONG)malloc(256);
    //g_VIDEO1_RC4_BUF_ADDR = Addr;
    //g_VIDEO1_DECODE_RC4_BUF_ADDR = g_VIDEO1_DECOMPRESS_BUF_ADDR + 0x800000; //assume video size is 8MB for umcompressed buf;
    //WriteMemoryLongWithMASKClient(VIDEO_REG_BASE, VIDEO1_RC4_TABLE_ADDR, g_VIDEO1_DECODE_RC4_BUF_ADDR, RC4_TABLE_ADDR_MASK);

	//HW recommanded value
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_2_ADDR_REG, g_VIDEO1_DECOMPRESS_BUF_ADDR, BUF_2_ADDR_MASK);
	//WriteMemoryLongWithMASKClient(VIDEO_REG_BASE, VIDEO1_BUF_2_ADDR_REG, 0, BUF_2_ADDR_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_CRC_BUF_ADDR_REG, 0, BUF_2_ADDR_MASK);            

    /* init encoder engine */
    InitializeVideoEngineClient (0,
				 VIDEO1);

	/* reset offset pointer register*/    
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, 0, VIDEO_CODEC_TRIGGER | VIDEO_CAPTURE_TRIGGER);

	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_READ_OFFSET_REG, 0, COMPRESS_BUF_READ_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_CODEC_OFFSET_READ, 0, BUF_CODEC_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_BUF_PROCESS_OFFSET_REG, 0, COMPRESS_BUF_PROCESS_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_COMPRESS_FRAME_END_READ, 0, COMPRESS_FRAME_END_READ_MASK);

	StartVideoCodecTriggerHost(0, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG);

	//wrtie SW write pointer
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_DECOMPRESS_BUF_PROCESS_OFFSET_REG, ulHWWp, COMPRESS_BUF_READ_OFFSET_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_DECOMPRESS_BUF_WRITE_OFFSET_REG, ulHWWp, COMPRESS_BUF_READ_OFFSET_MASK);

    i = 0;

    do
    {
        ulHWPt = ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO1_BUF_CODEC_OFFSET_READ);
        i++;
    }while((ulHWPt != ulHWWp) && (i<10000));

    if (10000 > i)
    {
 		printf("[VIDEO] Decoder Pointer cannot move!!! /n");
        //ExitVideoTest();
		return VIDEO_DECODE_FAIL;       
    }
        
	//8*8 YUVA block
	for (i=24; i<g_DefWidth*g_DefHeight; i=i+32)
	{
	  *(((ULONG *)g_VIDEO1_DECOMPRESS_BUF_ADDR) + i) = 0x0;
	  *(((ULONG *)g_VIDEO1_DECOMPRESS_BUF_ADDR) + i+1) = 0x0;
	}

	g_HashSrcBuffer = g_VIDEO1_DECOMPRESS_BUF_ADDR;
	//g_HashDstBuffer = VIDEO1_EN_BASE + VIDEO_ALL_SIZE; //config forward

	ulTemp = 300;


	//Enable hash clock
	WriteMemoryLongWithMASKHost(SCU_BASE, SCU_CLOCK_STOP_REG, EN_HACE, STOP_HACE_MASK);
	g_HashMethod.HashMode = VHASHMODE_SHA1;
	g_HashMethod.DigestLen = 20;
	HashAst(ulTemp, tArray, g_HashMethod.HashMode);
	
	if (strncmp((const char *)tArray, (const char *)aHashDecode, g_HashMethod.DigestLen))
	{
		printf("[VIDEO] Decoder Test: Wrong\n");
        //ExitVideoTest();
        return VIDEO_DECODE_FAIL;
	}
	else
	{
		printf("[VIDEO] Decoder Test: Pass\n");
	}

    return VIDEO_TEST_OK;
}

/********************************************************/
/*  Only used in the host                               */
/*  test function: Mode detection and Capture pattern   */
/********************************************************/
int CapTest(void)
{
    ULONG   dwValue, Status;
    int     i;
    BOOL    bAnalog;
	ULONG	HStart, HEnd, VStart, VEnd;
	USHORT	usCRTHor, usCRTVer;
	ULONG   ulHor, ulVer;
	ULONG   ulVGABaseAddr, ulCapAddr;
	ULONG   ulFlag;
    
    printf("\n --------- Mode Detection Test --------- \n");
	//clear clear register
	WriteMemoryLongHost(VIDEO_REG_BASE, VIDEO1_CONTROL_REG, 0);
	dwValue = ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO1_CONTROL_REG);

	//  Note: Current mode detection procedure has to set signal input 1st
	//Note: Clear and enable interrupt Encode
    ClearVideoInterruptHost(0, VIDEO1_MODE_DETECTION_READY_CLEAR);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_INT_CONTROL_EN_REG, VIDEO1_MODE_DETECTION_READY_INT_EN, VIDEO1_MODE_DETECTION_READY_INT_EN);
	//  Set input signal
    dwValue |= EXTERNAL_VGA_SOURCE;
    
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_CONTROL_REG, (dwValue <<EXTERNAL_SOURCE_BIT), EXTERNAL_SOURCE_MASK);

//  Set H/V stable maximum
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_MODE_DETECTION_PARAM_REG, (MODEDETECTION_VERTICAL_STABLE_MAXIMUM << VER_STABLE_MAX_BIT), 0x000F0000);
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_MODE_DETECTION_PARAM_REG, (MODEDETECTION_HORIZONTAL_STABLE_MAXIMUM << HOR_STABLE_MAX_BIT), HOR_STABLE_MAX_BIT_MASK);
//  Set H/V stable threshold
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_MODE_DETECTION_PARAM_REG, (MODEDETECTION_VERTICAL_STABLE_THRESHOLD << VER_STABLE_THRES_BIT), VER_STABLE_THRES_BIT_MASK);
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO_MODE_DETECTION_PARAM_REG, (MODEDETECTION_HORIZONTAL_STABLE_THRESHOLD << HOR_STABLE_THRES_BIT), HOR_STABLE_THRES_BIT_MASK);
    
    //Trigger mode detection
    // turn off WATCH_DOG first
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG, (WATCH_DOG_OFF << WATCH_DOG_ENABLE_BIT), WATCH_DOG_EN_MASK);
        
    //  Note: Clear mode detection ready interrupt
    //ClearVideoInterrupt(MMIOBase, MODE_DETECTION_READY_CLEAR);
    StartModeDetectionTriggerHost(0, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG);


//  Note: Polling mode detection ready interrupt
	//it sometime take a long time, especially during change mode, 
	//so the loop count must be big, or you can't pull it by timer
	i = 0;

    do {

		Status = ReadVideoInterruptHost(0, VIDEO1_MODE_DETECTION_READY_READ);
		i++;
    } while ((!Status) & (i<500000));

	if (!Status)
	{
		printf("[VIDEO] Mode detection error\n");
        //ExitVideoTest();
		return VIDEO_TEST_FAIL;
	}

	HStart	= (ReadMemoryLongHost(VIDEO_REG_BASE, VIDE1_MODE_DETECTION_EDGE_H_REG) & LEFT_EDGE_LOCATION_MASK) >> LEFT_EDGE_LOCATION_BIT;
	HEnd	= (ReadMemoryLongHost(VIDEO_REG_BASE, VIDE1_MODE_DETECTION_EDGE_H_REG) & RIGHT_EDGE_LOCATION_MASK) >> RIGHT_EDGE_LOCATION_BIT;

	VStart	= (ReadMemoryLongHost(VIDEO_REG_BASE, VIDE1_MODE_DETECTION_EDGE_V_REG) & TOP_EDGE_LOCATION_MASK) >> TOP_EDGE_LOCATION_BIT;
	VEnd	= (ReadMemoryLongHost(VIDEO_REG_BASE, VIDE1_MODE_DETECTION_EDGE_V_REG) & BOTTOM_EDGE_LOCATION_MASK) >> BOTTOM_EDGE_LOCATION_BIT;

	ulHor = HEnd-HStart+1;
	ulVer = VEnd-VStart+1;

	printf("[VIDEO] Resolution: H[%d] * V[%d]\n", ulHor, ulVer);

	if ((g_DefWidth == ulHor) & (g_DefHeight == ulVer))
	{
	  printf("[VIDEO] Mode detection PASS\n");
	}
	else
	{
	  printf("[VIDEO] Mode detection FAIL\n");
	  return VIDEO_TEST_FAIL;
	}

	if(!((ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO1_MODE_DETECTION_STATUS_READ_REG) & ANALONG_DIGITAL_READ) >> ANALONG_DIGITAL_READ_BIT))
		bAnalog = FALSE;
	else
		bAnalog = TRUE;

    //  Note: Clear mode detection ready interrupt
    ClearVideoInterruptHost(0, VIDEO1_MODE_DETECTION_READY_CLEAR);

    printf("\n --------- Capture Test --------- \n");

	//capture engine
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_TIMEING_GEN_HOR_REG, (HEnd << VIDEO_HDE_END_BIT), VIDEO_HDE_END_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_TIMEING_GEN_HOR_REG, (HStart << VIDEO_HDE_START_BIT), VIDEO_HDE_START_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_TIMEING_GEN_V_REG, (VEnd << VIDEO_VDE_END_BIT), VIDEO_VDE_END_MASK);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_TIMEING_GEN_V_REG, (VStart << VIDEO_VDE_START_BIT), VIDEO_VDE_START_MASK);
	
	ulCapAddr = vBufAlign((ULONG)pCaptureVideo1Buf1Addr);    
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_1_ADDR_REG, ulCapAddr, BUF_1_ADDR_MASK);
    
    WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_BUF_2_ADDR_REG, 0, BUF_2_ADDR_MASK);

	InitializeVideoEngineHost (0,
			       VIDEO1,
				vModeTable[2].HorPolarity,
				vModeTable[2].VerPolarity);

	WriteMemoryLongHost(VIDEO_REG_BASE, 0x04, 0x01);
	WriteMemoryLongHost(VIDEO_REG_BASE, 0x300, 0x0);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, 0x8, 0x2800, 0x2800);
	WriteMemoryLongWithMASKHost(VIDEO_REG_BASE, VIDEO1_TIMEING_GEN_HOR_REG, 0xa0000000, 0xa0000000);
   
	StartVideoCaptureTriggerHost(0, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG);

	i = 0;

    do {

	  Status = ReadMemoryLongHost(VIDEO_REG_BASE, VIDEO1_ENGINE_SEQUENCE_CONTROL_REG) & CAPTURE_READY_MASK;
		i++;
    } while ((!Status) & (i<500000));

	if (!Status)
	{
		printf("[VIDEO] Capture is not READY\n");
		return VIDEO_TEST_FAIL;
	}  
	
	ulVGABaseAddr = ulCapAddr + 0x1000;

	/* check pattern */
	ulFlag = 0; //no 0 is error

	for (i=0; i<100; i++)
	{
	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32);

	   if (0x32323232 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("[VIDEO]1 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }

	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32 + 4);

	   if (0x32323232 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("[VIDEO]2 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }

	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32 + 8);

	   if (0x80808080 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("[VIDEO]3 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }

	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32 + 12);

	   if (0x80808080 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("4 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }

	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32 + 16);

	   if (0x80808080 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("5 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }

	   dwValue = *(ULONG *)(ulVGABaseAddr + i*32 + 20);

	   if (0x80808080 != dwValue)
	   {
		   printf("[VIDEO] Capture Test fail -- capture data doesn't match source \n");
		   printf("6 i=%d value=%x\n", i, dwValue);
		   ulFlag = 1;
		   break;
	   }
	}

	if (!ulFlag)
	{
	  printf("[VIDEO] Capture Test OK\n");
	}
	else
	{
	    printf("[VIDEO] Capture Test FAIL\n");
	    return VIDEO_TEST_FAIL;
	}
	return VIDEO_TEST_OK;
}

/********************************************************/
/*  Only used in the station                            */
/********************************************************/
int CRTTest(void)
{
    ULONG   ulVGABaseAddr;
    BYTE	btCRTCenterMode, btCRTColorFmt;
    USHORT	usCRTHor, usCRTVer;
    ULONG   ulData;

    int i,j;

    printf("\n --------- Turn on CRT --------- \n");
    
 	//Enable CRT1 first
    ulVGABaseAddr = vBufAlign((unsigned long)pCrt1Addr);
    
	btCRTCenterMode = 0;
	btCRTColorFmt	= YUV_444;
	usCRTHor		= g_DefWidth;
	usCRTVer		= g_DefHeight;	

	CheckOnStartClient();

	for (i=0; i<usCRTHor; i++)
	{
		for (j=0; j<usCRTVer/8; j++)
		{
			//Y
			//ulData = 0x10101010;
			ulData = 0x32323232;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32)) = ulData;
			//ulData = 0x10101010;
			ulData = 0x32323232;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +4) = ulData;
			//U
			ulData = 0x80808080;
			//ulData = 0x77777777;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +8) = ulData;
			ulData = 0x80808080;
			//ulData = 0x77777777;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +12) = ulData;
			//V
			ulData = 0x80808080;
			//ulData = 0x11111111;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +16) = ulData;
			ulData = 0x80808080;
			//ulData = 0x11111111;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +20) = ulData;
			//A
			ulData = 0xFFFFFFFFF;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +24) = ulData;
			ulData = 0xFFFFFFFFF;
			*(ULONG *)(ulVGABaseAddr + (i*usCRTVer*4+j*32) +28) = ulData;
		}
	}
    
	if(!ASTSetModeV (0, CRT_1, ulVGABaseAddr, usCRTHor, usCRTVer, btCRTColorFmt, btCRTCenterMode))
    {   
        printf("[VIDEO] The resolution is not in mode table list\n");
        return CRT_FAIL;
    }
    
	printf("[VIDEO] CRT turn on\n");

    return VIDEO_TEST_OK;
}

int do_videotest (void)
{
	//unlock video register
	int dwValue=0;
    int i;
    int iRet;
    
    BOOL    bClient;


    //InitVideoTest();

	g_DefWidth = 640; 
	g_DefHeight= 480;

    printf("\n --------- Codec Test --------- \n");

    CheckOnStartHost();
    

	do
	{
		dwValue = UnlockVideoRegHost(0, VIDEO_UNLOCK_KEY);
		i++;
	}
	while ((VIDEO_UNLOCK != dwValue) && (i<1000));

	if (VIDEO_UNLOCK != dwValue)
	{
		printf("[VIDEO] Video register is locked \n");
		return VIDEO_TEST_FAIL;
	}
    AllocateEncodeBufHost(0,
                    VIDEO1);
   
	iRet = CodecTest();
	
    if (iRet)
        return VIDEO_TEST_FAIL;

    //Host or Client
    bClient = ((ReadMemoryLong(SCU_BASE, SCU_HW_TRAPPING_REG) & CLIENT_MODE_EN_MASK)?TRUE:FALSE);

    if (!bClient)
    {
        //To turn on crt, must be the client mode
        WriteMemoryLongWithMASKHost(SCU_BASE, SCU_HW_TRAPPING_REG, (BE_CLIENT_CHIP << CLIENT_MODE_EN_BIT), CLIENT_MODE_EN_MASK);
            
        iRet = CRTTest();

        if (iRet)
            return VIDEO_TEST_FAIL;
        
        iRet = CapTest();
		
		WriteMemoryLongWithMASKHost(SCU_BASE, SCU_HW_TRAPPING_REG, (BE_HOST_CHIP << CLIENT_MODE_EN_BIT), CLIENT_MODE_EN_MASK);
        if (iRet)
            return VIDEO_TEST_FAIL;

		//WriteMemoryLongWithMASKHost(SCU_BASE, SCU_HW_TRAPPING_REG, (BE_HOST_CHIP << CLIENT_MODE_EN_BIT), CLIENT_MODE_EN_MASK);
    }

	return VIDEO_TEST_OK;
	
}

#endif /* CONFIG_SLT */
