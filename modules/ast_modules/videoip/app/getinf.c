/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include <math.h>
#include <time.h>

#include "vdef.h"
#include "global.h"
#include "astdebug.h"

static int  GetGeneralINFData (char *filename, PGENERAL_INFO pGeneralInfo)
{

    unsigned char          string[81],name[81];
    unsigned char          StringToken[256];
    __u32          temp;
    
    FILE          *fp;

    fp = fopen(filename, "rb");
    
	if (!fp)
		return -1;

    while (fgets ((char *)string, 80, fp) != NULL) {
    	sscanf ((char *)string, "%[^=] = %s", name, StringToken);

    	if (strcmp ((char *)name, "EnableVideoM") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->EnableVideoM = (__u8)temp;
    	    continue;
    	}        
    	if (strcmp ((char *)name, "CenterMode") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->CenterMode = (__u8)temp;
    	    continue;
    	}
        /* Contrl reg */
    	if (strcmp ((char *)name, "SkipEmptyFrame") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->uCtlReg.CtlReg.SkipEmptyFrame = temp;
    	    continue;
    	}
    	if (strcmp ((char *)name, "MemBurstLength") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->uCtlReg.CtlReg.MemBurstLen = temp;
    	    continue;
    	}
		/* 0: Disable  1: Video1  2: Video2  3: VideoM */
    	if (strcmp ((char *)name, "VerLineBufferEN") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->uCtlReg.CtlReg.LineBufEn = temp;
    	    continue;
    	}
        /* CRC reg */        
    	if (strcmp ((char *)name, "CRCEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pGeneralInfo->uCRCPriParam.CRCPriParam.Enable = (__u8)temp;
    	    continue;
    	}
    	if (strcmp ((char *)name, "CRCPrimaryPolynomial") == 0) {
            temp = atol((char *)StringToken);
            
    	    pGeneralInfo->uCRCPriParam.CRCPriParam.PolyLow   = ((__u32)temp & 0xFF);
            pGeneralInfo->uCRCPriParam.CRCPriParam.PolyHigh  = (((__u32)temp >> 8) & 0xFFFF);
    	    continue;
    	}
    	if (strcmp ((char *)name, "CRCHighOnly") == 0) {
            temp = atol((char *)StringToken);
            pGeneralInfo->uCRCPriParam.CRCPriParam.HighBitOnly = (__u8)temp; 
    	    continue;
    	}
    	if (strcmp ((char *)name, "CRCMaxCount") == 0) {
    	    temp = atol((char *)StringToken);
            pGeneralInfo->uCRCPriParam.CRCPriParam.SkipCountMax = (__u8)temp; 
    	    continue;
    	}        
    	if (strcmp ((char *)name, "CRCSecondaryPolynomial") == 0) {    	    
            temp = atol((char *)StringToken);
    	    pGeneralInfo->uCRCSecParam.CRCSecParam.PolyLow   = ((__u32)temp & 0xFF);
            pGeneralInfo->uCRCSecParam.CRCSecParam.PolyHigh  = (((__u32)temp >> 8) & 0xFFFF);
    	    continue;
    	}
    	if (strcmp ((char *)name, "RC4_NO_RESET_FRAME") == 0) {    	    
            temp = atol((char *)StringToken);
    	    pGeneralInfo->RC4NoResetFrame = (__u8)temp;
    	    continue;
    	}
    	if (strcmp ((char *)name, "RC4_TEST_MODE") == 0) {    	    
            temp = atol((char *)StringToken);
    	    pGeneralInfo->RC4TestMode = (__u8)temp;
    	    continue;
    	}
    }
    
    fclose (fp);

	return 0;
}

static int  GetVIDEOINFData (char *filename, PVIDEO_INFO pVideoInfo)
{

    unsigned char          string[81],name[81];
    unsigned char          StringToken[256];
    long          temp;
    
    FILE          *fp;

    fp = fopen(filename, "rb");

	if (!fp)
		return -1;

    while (fgets ((char *)string, 80, fp) != NULL) {
    	sscanf ((char *)string, "%[^=] = %s", name, StringToken);

    	if (strcmp ((char *)name, "DOWN_SCALING_METHOD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DownScalingMethod = (__u8)temp;
            continue;            
    	}
#if 0        
    	if (strcmp ((char *)name, "DestinationWidth") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DstWidth = (__u16)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "DestinationHeight") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DstHeight = (__u16)(atol((char *)StringToken));
            continue;            
    	}
#endif
    	if (strcmp ((char *)name, "HighLumaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighLumaTable = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighChromaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighChromaTable = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighDeQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighDeQuantiValue = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "BestDeQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->BestDequantiValue = (__u8)temp;
            continue;            
        }
        
        /* Sequence Control reg */
    	if (strcmp ((char *)name, "CaptureAutoMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CaptureAutoMode = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CodecAutoMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CodecAutoMode = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CRTSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CRTSel = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "AntiTearingEn") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.AntiTearing = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "DataType") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.DataType = temp;
            continue;
    	}
        /* Video Control reg */
    	if (strcmp ((char *)name, "ExternalSignal") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ExtSrc = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "InternalCaptureDataFrom") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.IntDataFrom = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "WriteFormat") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.WriteFmt = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ExternalPortSlection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ExtPort = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ClockDelay") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ClockDelay = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CCIR565SrcMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.CCIR656Src = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "PortClockMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.PortClock= temp;
            continue;
    	}
    	if (strcmp ((char *)name, "FrameRateControl") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.FrameRate= temp;
            continue;
    	}
        /* BCD Setting*/
    	if (strcmp ((char *)name, "BCDEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBCDCtlReg.BCDCtlReg.Enable = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ANALOG_DIFFERENTIAL_THRESHOLD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->AnalogDifferentialThreshold = (__u16)temp;
			printf("analog [%d]\n", pVideoInfo->AnalogDifferentialThreshold);
            continue;            
    	}

    	if (strcmp ((char *)name, "DIGITAL_DIFFERENTIAL_THRESHOLD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DigitalDifferentialThreshold = (__u16)temp;
            continue;            
    	}
        /* Packet Size */
    	if (strcmp ((char *)name, "PacketSize") == 0) {
            __u8 PacketRegister = 0;
            
            temp = atol((char *)StringToken);
            temp = temp / 1024;
            
            while (temp != 0) {
        	    PacketRegister++;
        	    temp /= 2;
            }

    	    pVideoInfo->uStreamBufSize.StreamBufSize.PacketSize = PacketRegister - 1;
            continue;
    	}
    	if (strcmp ((char *)name, "PacketNumber") == 0) {
            __u8 NumberRegister = 0;
            temp = atol((char *)StringToken);

            temp = temp / 4;
            while (temp != 0) {
            	NumberRegister++;
            	temp /= 2;
            }

    	    pVideoInfo->uStreamBufSize.StreamBufSize.RingBufNum = NumberRegister - 1;
            continue;
    	}
    	if (strcmp ((char *)name, "SkipHighMBThreshold") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uStreamBufSize.StreamBufSize.SkipHighMBThres = temp;
            continue;
    	}
        /* Compression Setting */
        if (strcmp ((char *)name, "JpegOnlyMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.JPEGOnly = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "VQ4ColorMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.En4VQ = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "CodecMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.CodecMode = temp;
            continue;            
    	}        
     	if (strcmp ((char *)name, "JPEGDualQuality") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.DualQuality = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "BestQualityEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.EnBest = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "RC4Enable") == 0) {
            temp = atol((char *)StringToken);
			pVideoInfo->uCompressCtlReg.CompressCtlReg.EnRC4 = (__u8)temp;
    	    pVideoInfo->EnableRC4 = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "NormalLumaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.NorLumaDCTTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "NormalChromaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.NorChromaDCTTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighQualityEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.EnHigh = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "JPEGHufTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.HufTable = temp;
            continue;            
    	}
        /* Quantization register */
    	if (strcmp ((char *)name, "LowChromaDCTTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQTableLowReg.QTableLowReg.ChromaTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "LowLumaDCTTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQTableLowReg.QTableLowReg.LumaTable = temp;
            continue;            
    	}
        /* Quantization value */
    	if (strcmp ((char *)name, "HighQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQValueReg.QValueReg.High = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "BestQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQValueReg.QValueReg.Best = temp;
            continue;            
    	}
        /* BSD parameter */
        if (strcmp ((char *)name, "BSDHighThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.HighThres = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDLowThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.LowThres = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDHighCountThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.HighCount = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDLowCountThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.LowCount = temp;
            continue;            
    	}
		
        /* RC4 keys */
		//Todo:
		/*
        if (strcmp ((char *)name, "RC4_KEYS") == 0) {
            if (pVideoInfo->EnableRC4)
            {
                int i;

                memset(EncodeKeys, 0, 256);
                for (i = 0; i < (int)strlen ((char *)StringToken); i++) {
                    EncodeKeys[i] = StringToken[i];
                }
            }
        }
        */

    }

    fclose (fp);

	return 0;
}

static int  GetVIDEOINFData_V2 (char *filename, PVIDEO_INFO_V2 pVideoInfo)
{

    unsigned char          string[81],name[81];
    unsigned char          StringToken[256];
    long          temp;
    
    FILE          *fp;

    fp = fopen(filename, "rb");

	if (!fp)
		return -1;

    while (fgets ((char *)string, 80, fp) != NULL) {
    	sscanf ((char *)string, "%[^=] = %s", name, StringToken);

    	if (strcmp ((char *)name, "DOWN_SCALING_METHOD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DownScalingMethod = (__u8)temp;
            continue;            
    	}
#if 0        
    	if (strcmp ((char *)name, "DestinationWidth") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DstWidth = (__u16)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "DestinationHeight") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DstHeight = (__u16)(atol((char *)StringToken));
            continue;            
    	}
#endif
    	if (strcmp ((char *)name, "HighLumaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighLumaTable = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighChromaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighChromaTable = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighDeQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->HighDeQuantiValue = (__u8)temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "BestDeQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->BestDequantiValue = (__u8)temp;
            continue;            
        }
        
        /* Sequence Control reg */
    	if (strcmp ((char *)name, "CaptureAutoMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CaptureAutoMode = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CodecAutoMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CodecAutoMode = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CRTSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.CRTSel = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "AntiTearingEn") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.AntiTearing = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "DataType") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uSeqCtlReg.SeqCtlReg.DataType = temp;
            continue;
    	}
        /* Video Control reg */
    	if (strcmp ((char *)name, "ExternalSignal") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ExtSrc = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "InternalCaptureDataFrom") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.IntDataFrom = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "WriteFormat") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.WriteFmt = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ExternalPortSlection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ExtPort = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ClockDelay") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.ClockDelay = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "CCIR565SrcMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.CCIR656Src = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "PortClockMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.PortClock= temp;
            continue;
    	}
    	if (strcmp ((char *)name, "FrameRateControl") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCtlReg.CtlReg.FrameRate= temp;
            continue;
    	}
        /* BCD Setting*/
    	if (strcmp ((char *)name, "BCDEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBCDCtlReg.BCDCtlReg.Enable = temp;
            continue;
    	}
    	if (strcmp ((char *)name, "ANALOG_DIFFERENTIAL_THRESHOLD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->AnalogDifferentialThreshold = (__u16)temp;
			printf("analog [%d]\n", pVideoInfo->AnalogDifferentialThreshold);
            continue;            
    	}

    	if (strcmp ((char *)name, "DIGITAL_DIFFERENTIAL_THRESHOLD") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->DigitalDifferentialThreshold = (__u16)temp;
            continue;            
    	}
        /* Packet Size */
    	if (strcmp ((char *)name, "PacketSize") == 0) {
            __u8 PacketRegister = 0;
            
            temp = atol((char *)StringToken);
            temp = temp / 1024;
            
            while (temp != 0) {
        	    PacketRegister++;
        	    temp /= 2;
            }

    	    pVideoInfo->uStreamBufSize.StreamBufSize.PacketSize = PacketRegister - 1;
            continue;
    	}
    	if (strcmp ((char *)name, "PacketNumber") == 0) {
            __u8 NumberRegister = 0;
            temp = atol((char *)StringToken);

            temp = temp / 4;
            while (temp != 0) {
            	NumberRegister++;
            	temp /= 2;
            }

    	    pVideoInfo->uStreamBufSize.StreamBufSize.RingBufNum = NumberRegister - 1;
            continue;
    	}
    	if (strcmp ((char *)name, "SkipHighMBThreshold") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uStreamBufSize.StreamBufSize.SkipHighMBThres = temp;
            continue;
    	}
        /* Compression Setting */
        if (strcmp ((char *)name, "JpegOnlyMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.JPEGOnly = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "VQ4ColorMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.En4VQ = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "CodecMode") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.CodecMode = temp;
            continue;            
    	}        
     	if (strcmp ((char *)name, "JPEGDualQuality") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.DualQuality = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "BestQualityEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.EnBest = temp;
            continue;            
    	}
#if 0
    	if (strcmp ((char *)name, "RC4Enable") == 0) {
            temp = atol((char *)StringToken);
			pVideoInfo->uCompressCtlReg.CompressCtlReg.EnRC4 = (__u8)temp;
    	    pVideoInfo->EnableRC4 = temp;
            continue;            
    	}
#endif
		if (strcmp ((char *)name, "CryptoMode") == 0) {
			temp = atol((char *)StringToken);
#if 0
			if (temp)
				pVideoInfo->uCompressCtlReg.CompressCtlReg.EnRC4 = 1;
			else
				pVideoInfo->uCompressCtlReg.CompressCtlReg.EnRC4 = 0;
#endif
			pVideoInfo->CryptoMode = temp;
			continue;
		}
		if (strcmp ((char *)name, "FastCrypto") == 0) {
			temp = atol((char *)StringToken);
			pVideoInfo->FastCrypto = temp;
			continue;
		}
    	if (strcmp ((char *)name, "NormalLumaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.NorLumaDCTTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "NormalChromaTableSelection") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.NorChromaDCTTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "HighQualityEnable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.EnHigh = temp;
            continue;            
    	}        
    	if (strcmp ((char *)name, "JPEGHufTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uCompressCtlReg.CompressCtlReg.HufTable = temp;
            continue;            
    	}
        /* Quantization register */
    	if (strcmp ((char *)name, "LowChromaDCTTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQTableLowReg.QTableLowReg.ChromaTable = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "LowLumaDCTTable") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQTableLowReg.QTableLowReg.LumaTable = temp;
            continue;            
    	}
        /* Quantization value */
    	if (strcmp ((char *)name, "HighQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQValueReg.QValueReg.High = temp;
            continue;            
    	}
    	if (strcmp ((char *)name, "BestQuantizationValue") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uQValueReg.QValueReg.Best = temp;
            continue;            
    	}
        /* BSD parameter */
        if (strcmp ((char *)name, "BSDHighThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.HighThres = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDLowThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.LowThres = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDHighCountThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.HighCount = temp;
            continue;            
    	}
        if (strcmp ((char *)name, "BSDLowCountThres") == 0) {
            temp = atol((char *)StringToken);
    	    pVideoInfo->uBSDParamReg.BSDParamReg.LowCount = temp;
            continue;            
    	}
		
        /* RC4 keys */
		//Todo:
		/*
        if (strcmp ((char *)name, "RC4_KEYS") == 0) {
            if (pVideoInfo->EnableRC4)
            {
                int i;

                memset(EncodeKeys, 0, 256);
                for (i = 0; i < (int)strlen ((char *)StringToken); i++) {
                    EncodeKeys[i] = StringToken[i];
                }
            }
        }
        */

    }

    fclose (fp);

	return 0;
}


void LoadInf(PENGINE_CONFIG pEngConfig)
{

	memset(&pEngConfig->GenInfo, 0, sizeof(GENERAL_INFO));
	if (GetGeneralINFData(BASE_DIR"General.inf", &pEngConfig->GenInfo))
	{
		err("Loading General.inf fail\n");
		return;
	}

	memset(&pEngConfig->V1Info, 0, sizeof(VIDEO_INFO));
	if (GetVIDEOINFData (BASE_DIR"Video1.inf", &pEngConfig->V1Info))
	{
		err("Loading Video1.inf fail\n");
		return;
	}
}

void LoadInf_V2(PENGINE_CONFIG_V2 pEngConfig)
{

	memset(&pEngConfig->GenInfo, 0, sizeof(GENERAL_INFO));
	if (GetGeneralINFData(BASE_DIR"General.inf", &pEngConfig->GenInfo))
	{
		err("Loading General.inf fail\n");
		return;
	}

	memset(&pEngConfig->V1Info, 0, sizeof(VIDEO_INFO_V2));
	if (GetVIDEOINFData_V2(BASE_DIR"Video1.inf", &pEngConfig->V1Info))
	{
		err("Loading Video1.inf fail\n");
		return;
	}
}

