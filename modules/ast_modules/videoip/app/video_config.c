/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <getopt.h>
#include <signal.h>

#include "vdef.h"
#include "getinf.h"
#include "videoip_ioctl.h"

#define ASTHOST_GLOBALS
#include "global.h"
#include "astdebug.h"

#define dbg_info(fmt, args...)	do { \
	if (0) { \
		syslog(LOG_DEBUG, fmt, ##args); \
	} \
	if (0) { \
		fprintf(stderr, fmt "\n",  ##args); \
	} \
} while (0)

static char *soc_version = NULL;

static const FILE_SECTION Sections[] = {
{"[general]", 0xFF},
{"[text]", QUALITY_TEXT},
{"[high]", QUALITY_HIGH},
{"[med]", QUALITY_MED},
{"[low]", QUALITY_LOW},
};
static int Section_Num = sizeof(Sections) / sizeof(FILE_SECTION);
static const FILE_SECTION Sections_V2[] = {
{"[general]", 0xFF},
{"[quality0]", 0},
{"[quality1]", 1},
{"[quality2]", 2},
{"[quality3]", 3},
{"[quality4]", 4},
{"[quality5]", 5},
};
static int Section_Num_V2 = sizeof(Sections_V2) / sizeof(FILE_SECTION);


int  GetCompressCrt(char *filename, PCTRL_VIDEO pCtrlVideo)
{

    unsigned char          string[81],name[81];
    unsigned char          StringToken[256];
    long          temp;
	int			i, quality_level = 0xFF;
    FILE          *fp;

    fp = fopen(filename, "rb");

	if (!fp)
		return -1;

//	printf("Section_Num = %d\n", Section_Num);
    while (fgets ((char *)string, 80, fp) != NULL) {
		if (string[0] == '['){//section title
//			printf("section \"%s\"\n", string);
			for (i = 0; i < Section_Num; i++){
//				printf("section #%d %s\n", i, Sections[i].SectionName);
				if (!strncmp(string, Sections[i].SectionName, strlen(Sections[i].SectionName))){
//					printf("section #%d matched\n", i);
					break;
				}
			}
			if (i < Section_Num){
				quality_level = Sections[i].QualityLevel;
				//printf("quality level = %d\n", quality_level);
			}
			else
				quality_level = 0xFFFF;
		}
		else{
			if (quality_level < 0xFF){//setting of a specific quality
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "NormalLumaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("NormalLumaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalLumaTableSelection = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "NormalChromaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("NormalChromaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalChromaTableSelection = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HighQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HighQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighQualityEnable = temp;
					continue;			 
				}		 
				
				if (strcmp ((char *)name, "BestQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("BestQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestQualityEnable = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "HighDeValueOrHighLumaJpeg") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HighDeValueOrHighLumaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighDeValueOrHighLumaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "BestDeValueOrHighChromaJpeg") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("BestDeValueOrHighChromaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestDeValueOrHighChromaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "FrameRateControl") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("FrameRateControl = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].FrameRate = temp; 
					continue;
				}
				if (strcmp ((char *)name, "CodecMode") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("CodecMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].CodecMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HorScale") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HorScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HorScale = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "VerScale") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("VerScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].VerScale = temp;
					continue;			 
				}		 
			}
			else if (quality_level == 0xFF){//general setting
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "DynamicQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("DynamicQualityEnable = %d\n", temp);
					pCtrlVideo->DynamicQualityEnable = temp;
					continue;
				}
				if (strcmp ((char *)name, "DefaultQualityMode") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("DefaultQualityMode = %d\n", temp);
					pCtrlVideo->DefaultQualityMode = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktSize") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("StreamBufPktSize = %d\n", temp);
					pCtrlVideo->StreamBufPktSize = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktNum") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("StreamBufPktNum = %d\n", temp);
					pCtrlVideo->StreamBufPktNum = temp;
					continue;
				}
				if (strcmp ((char *)name, "VideoBitRateLimit") == 0) {
					temp = atol((char *)StringToken);
					pCtrlVideo->VideoBitRateLimit = temp;
					continue;
				}
			}
			else{
				//printf("should not happen!!!\n");
			}
		}
    }

    fclose (fp);

	return 0;
}

int GetCompressCrt_V2(char *filename, PCTRL_VIDEO_V2 pCtrlVideo)
{
	unsigned char          string[81],name[81];
	unsigned char          StringToken[256];
	long          temp;
	int			i, quality_level = 0xFF;
	FILE          *fp;

	fp = fopen(filename, "rb");

	if (!fp) {
		err("unable to open %s\n", filename);
		return -1;
	}

//	printf("%s:\n", filename);
	pCtrlVideo->QualityNum = 0;

	/* data structure initialize before file parsing */
	memset(pCtrlVideo, 0 , sizeof(CTRL_VIDEO_V2));

	while (fgets ((char *)string, 80, fp) != NULL) {
		if (string[0] == '['){//section title
//			printf("section \"%s\"\n", string);
			for (i = 0; i < Section_Num_V2; i++){
//				printf("section #%d %s\n", i, Sections[i].SectionName);
				if (!strncmp(string, Sections_V2[i].SectionName, strlen(Sections_V2[i].SectionName))){
//					printf("section #%d matched\n", i);
					break;
				}
			}
			if (i < Section_Num_V2){
				quality_level = Sections_V2[i].QualityLevel;
				dbg_info("quality level = %d\n", quality_level);
				if (quality_level != 0xFF)
				{
					pCtrlVideo->QualityNum++;
				}
			}
			else
				quality_level = 0xFFFF;
		}
		else{
			if (quality_level < 0xFF){//setting of a specific quality
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "NormalLumaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("NormalLumaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalLumaTableSelection = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "NormalChromaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("NormalChromaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalChromaTableSelection = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HighQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HighQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighQualityEnable = temp;
					continue;			 
				}		 
				
				if (strcmp ((char *)name, "BestQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("BestQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestQualityEnable = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "HighDeValueOrHighLumaJpeg") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HighDeValueOrHighLumaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighDeValueOrHighLumaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "BestDeValueOrHighChromaJpeg") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("BestDeValueOrHighChromaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestDeValueOrHighChromaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "FrameRateControl") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("FrameRateControl = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].FrameRate = temp; 
					continue;
				}
				if (strcmp ((char *)name, "CodecMode") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("CodecMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].CodecMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HorScale") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("HorScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HorScale = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "VerScale") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("VerScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].VerScale = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "YUV420") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("YUV420 = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].YUV420 = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "JpegOnlyMode") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("JpegOnlyMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].JpegOnlyMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "VQ4ColorMode") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("VQ4ColorMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].VQ4ColorMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableBCD") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("EnableBCD = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableBCD = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "BCDThreshold") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("BCDThreshold = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BCDThreshold = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableABCD") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("EnableABCD = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableABCD = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "ABCDThreshold") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("ABCDThreshold = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].ABCDThreshold = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "Pass2BCDDelay") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("Pass2BCDDelay = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].Pass2BCDDelay = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "Pass3BCDDelay") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("Pass3BCDDelay = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].Pass3BCDDelay = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "TruncatedBits") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("TruncatedBits = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].TruncatedBits = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableDithering") == 0) {
					temp = atol((char *)StringToken);
					dbg_info("EnableDithering = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableDithering = temp;
					continue;			 
				}		 
			}
			else if (quality_level == 0xFF){//general setting
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "DynamicQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("DynamicQualityEnable = %d\n", temp);
					pCtrlVideo->DynamicQualityEnable = temp;
					continue;
				}
				if (strcmp ((char *)name, "DefaultQualityMode") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("DefaultQualityMode = %d\n", temp);
					pCtrlVideo->DefaultQualityMode = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktSize") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("StreamBufPktSize = %d\n", temp);
					pCtrlVideo->StreamBufPktSize = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktNum") == 0) {
					temp = atol((char *)StringToken);
					//dbg_info("StreamBufPktNum = %d\n", temp);
					pCtrlVideo->StreamBufPktNum = temp;
					continue;
				}
				if (strcmp ((char *)name, "VideoBitRateLimit") == 0) {
					temp = atol((char *)StringToken);
					pCtrlVideo->VideoBitRateLimit = temp;
					continue;
				}
			}
			else{
				//dbg_info("should not happen!!!\n");
			}
		}
	}

	fclose (fp);

	return 0;
}

#if 0
static void LoadInf(PIOCTL_HOST_INFO pHostInfo)
{

	memset(&pHostInfo->GenInfo, 0, sizeof(GENERAL_INFO));
    if (!GetGeneralINFData("General.inf", &pHostInfo->GenInfo))
    {
        err("Loading General.inf fail\n");
		return;
    }

	memset(&pHostInfo->V1Info, 0, sizeof(VIDEO_INFO));
    if (!GetVIDEOINFData ("Video1.inf", &pHostInfo->V1Info))
    {
       err("Loading Video1.inf fail\n");
       return;
    }
    
	memset(&pHostInfo->V2Info, 0, sizeof(VIDEO_INFO));
    if (!GetVIDEOINFData ("Video2.inf", &pHostInfo->V2Info))
    {
       err("Loading Video2.inf fail\n");
       return;
    }

	memset(&pHostInfo->VMInfo, 0, sizeof(VIDEO_INFO));
    if (!GetVIDEOMINFData("VideoM.inf", &pHostInfo->VMInfo))
    {
      err("Loading VideoM.inf fail\n");
      return;
    }
    
}
#endif

static void UseOnlyV1Host(int devfd, PIOCTL_CONFIG_INFO pHostInfo)
{
	info("UseOnlyV1Host");

	memset(pHostInfo, 0, sizeof(IOCTL_CONFIG_INFO));
	LoadInf(&pHostInfo->e_cfg);
	
	//Load Compress crtl
	GetCompressCrt(BASE_DIR"Config_1920.inf", &pHostInfo->Ctrl1920);
    //printf("1920 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl1920);
    
    GetCompressCrt(BASE_DIR"Config_1280.inf", &pHostInfo->Ctrl1280);
    //printf("1280 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl1280);

    GetCompressCrt(BASE_DIR"Config_800.inf", &pHostInfo->Ctrl800);
    //printf("800 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl800);
    
	ioctl (devfd, IOCTL_HOST_ONLY_V1, pHostInfo);	

	return;
}

static void create_misc_cfg(struct misc_cfg *cfg, unsigned int port_select)
{
	cfg->magic_num = MISC_CFG_MAGIC_NUM;
	cfg->port_select = port_select;
}

static void UseOnlyV2Host(int devfd, PIOCTL_CONFIG_INFO_V2 pHostInfo, unsigned int port_select)
{
	info("UseOnlyV2Host");

	/* If port_select not specified, use old e_cfg. */
	if (port_select == (unsigned int)-1)
		LoadInf_V2(&pHostInfo->cfg.e_cfg);
	else
		create_misc_cfg(&pHostInfo->cfg.misc_cfg, port_select);

	//Load Compress crtl
	GetCompressCrt_V2(BASE_DIR"Config_1920.inf", &pHostInfo->Ctrl1920);
    //printf("1920 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl1920);
	GetCompressCrt_V2("Config_1920_low_frame_rate.inf", &pHostInfo->Ctrl1920_low_frame_rate);
    
    GetCompressCrt_V2(BASE_DIR"Config_1280.inf", &pHostInfo->Ctrl1280);
    //printf("1280 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl1280);
    GetCompressCrt_V2(BASE_DIR"Config_1280_low_frame_rate.inf", &pHostInfo->Ctrl1280_low_frame_rate);

    GetCompressCrt_V2(BASE_DIR"Config_800.inf", &pHostInfo->Ctrl800);
    //printf("800 setting [%x]\n", *(ULONG *)&pHostInfo->Ctrl800);

	GetCompressCrt_V2(BASE_DIR"Config_4K.inf", &pHostInfo->Ctrl4K);
	GetCompressCrt_V2(BASE_DIR"Config_4K_low_frame_rate.inf", &pHostInfo->Ctrl4K_low_frame_rate);

	ioctl (devfd, IOCTL_HOST_ONLY_V2, pHostInfo);

	return;
}


static const struct option longopts[] = {
	{"soc_ver",	required_argument,	NULL, 's'},
	{"port_select",	required_argument,	NULL, 'p'},
	{NULL,		0,		NULL,  0}
};

int main(int argc, char *argv[])
{
	int devfd;
	unsigned int port_select = -1;

	videoip_use_stderr = 0;
	videoip_use_syslog = 0;

	dbg_info("--->video_config\n");
	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "s:p:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 's':
				soc_version = optarg;
				dbg_info("SOC version = %s\n", soc_version);
				break;
			case 'p':
				port_select = (unsigned int)strtoul(optarg, NULL, 0);
				break;
			case '?':
				break;
			default:
				err("getopt");
		}
	}

	devfd = open ("/dev/videoip", O_RDONLY);
	if (!strncmp(soc_version, "V1", 2))
	{
		IOCTL_CONFIG_INFO HostInfo;
		info("SOC V1");
		UseOnlyV1Host(devfd, &HostInfo);
	}
	else if (!strncmp(soc_version, "V2", 2))
	{
		IOCTL_CONFIG_INFO_V2 HostInfo;
		info("SOC V2");
		UseOnlyV2Host(devfd, &HostInfo, port_select);
	}
	else
		err("unsupported SOC version: %s\n", soc_version);
	close(devfd);
	dbg_info("video_config<---\n");

	return 0;
}
