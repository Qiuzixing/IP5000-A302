#include "ast_def.h"
#ifdef CONFIG_AST1500_CAT6613
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >cat6613_sys.c<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/

///////////////////////////////////////////////////////////////////////////////
// This is the sample program for CAT6611 driver usage.
///////////////////////////////////////////////////////////////////////////////

/*
 * ASPEED Options
 */
#define CAP_HDMI	0x01
#define CAP_HDCP	0x02
#define CAP_AUDIO	0x04
#if	defined(Linux_KernelMode)
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <asm/arch/gpio.h>
#if SUPPORT_HDCP_REPEATER
#include <aspeed/hdcp1.h>
static void start_hdcp1_auth(int enable);
#endif

#include "typedef_hdmitx.h"
#include "cat6613_ioctl.h"

#if	(!defined(TestMode))
extern int SetI2CReg(u32 Channel, u32 Address, u32 RegisterIndex, u8 RegisterValue);
extern int GetI2CReg(u32 Channel, u32 Address, u32 RegisterIndex, u8 *RegisterValue);
#endif

#elif	defined(Linux_UserMode)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdarg.h>

#include "ioaccess.h"
#endif
//#ifdef CONFIG_AST1500_CAT6613
//#include "typedef_hdmitx.h"
//#else
//#include "hdmitx.h"
//#endif
#include <linux/platform_device.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include "cat6613_sys.h"
#include "cat6613_drv.h"
#include <asm/arch/drivers/crt.h>

#define PARAM_DOUBLE_COPY 1

#if HANDLE_INT_IN_THREAD
	#include <linux/workqueue.h>
	static struct workqueue_struct *wq = NULL;
	static struct semaphore drv_lock;
	static struct work_struct hotplug_task;
	static struct work_struct setup_video_task;
#ifdef CONFIG_ARCH_AST1500_HOST
	static struct work_struct setup_loopback_video_task;
#endif
	static struct work_struct setup_audio_task;
	static struct work_struct disable_task;
	static struct work_struct disable_audio_task;
#if SUPPORT_HDCP_REPEATER
	static struct work_struct Hdcp_start_auth_work;
	static struct work_struct Hdcp_stop_auth_work;
	static struct work_struct Hdcp_disable_work;
#endif

#if PARAM_DOUBLE_COPY
	static struct ws_setup_video {
//		BYTE InputColorMode;
//		BYTE OutputColorMode;
//		HDMI_Video_Type OutputVideoTiming;
		ULONG VCLK;
		BYTE bCapibility;
		BYTE ColorDepth;
		VIDEOPCLKLEVEL level;
		unsigned int is_HD; //Is it an HD timing. VActive >= 720
		InfoFrame	AVI_Info;
		InfoFrame	HDMI_Info;
	} ws_video;

	static struct ws_setup_audio {
		//BYTE Audio_On;
		BYTE SampleFrequency;
		BYTE ValidCh;
		BYTE SampleSize;
		BYTE Audio_Type;
		BYTE bSPDIF;
		Audio_InfoFrame audio_infoframe;
		BYTE ucIEC60958ChStat[5];
	} ws_audio;
#endif //#if PARAM_DOUBLE_COPY
#else
	struct tasklet_struct hotplug_task;
#endif

static UCHAR jVersion[] = "v1.09";
extern INSTANCE Instance[];

#if 0
void ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep);
#endif
void ConfigAudioInfoFrm(void);

////////////////////////////////////////////////////////////////////////////////
// Function Body.
////////////////////////////////////////////////////////////////////////////////

extern BYTE HPDStatus;
static unsigned int bcat6613_exist = 0;

#if (CONFIG_AST1500_SOC_VER >= 2)
#define SYNC_REGEN 0
#else //#if (CONFIG_AST1500_SOC_VER >= 2)
	#ifdef CONFIG_ARCH_AST1500_CLIENT
		#define SYNC_REGEN 1
	#else
		#define SYNC_REGEN 0
	#endif
#endif //#if (CONFIG_AST1500_SOC_VER >= 2)

#if SYNC_REGEN
struct SyncRegenTable {
	BYTE VIC; // video timing code
	BYTE Reg90;
	BYTE Reg91;
	BYTE Reg92;
	BYTE Reg93;
	BYTE Reg94;
	BYTE Reg95;
	BYTE Reg96;
	BYTE Reg97;
	BYTE Reg98;
	BYTE Reg99;
	BYTE Reg9A;
	BYTE Reg9B;
	BYTE Reg9C;
	BYTE Reg9D;
	BYTE Reg9E;
	BYTE Reg9F;
	BYTE RegA0;
	BYTE RegA1;
	BYTE RegA2;
	BYTE RegA3;
};

/*
** AST1500 doesn't support interlace mode. Ignore it.
*/
struct SyncRegenTable SRT[] = 
{
	{HDMI_Unkown,     0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3}, //dummy value
	{HDMI_640x480p60, 0xf9, 0x31, 0x8e, 0x0e, 0x30, 0x1e, 0x5e, 0x03, 0xff, 0xff, 0x21, 0x01, 0x20, 0xFF, 0xFF, 0xFF, 0x0b, 0x02, 0xff, 0xff}, //golden
	{HDMI_480p60,     0x99, 0x35, 0x78, 0x48, 0x30, 0x58, 0x3c, 0x03, 0x0d, 0x02, 0x22, 0x02, 0x20, 0xff, 0xff, 0xff, 0x0b, 0x42, 0xff, 0xff}, //golden
	{HDMI_480p60_16x9,0x99, 0x35, 0x78, 0x48, 0x30, 0x58, 0x3c, 0x03, 0x0d, 0x02, 0x22, 0x02, 0x20, 0xff, 0xff, 0xff, 0x0b, 0x42, 0xff, 0xff}, //golden
	{HDMI_720p60,     0x1f, 0x67, 0x02, 0x02, 0x61, 0x70, 0x26, 0x06, 0xed, 0x02, 0x17, 0xe7, 0x20, 0x23, 0x36, 0x00, 0xec, 0x32, 0x1f, 0x1f}, //golden
	//{HDMI_1080i60,    90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_480i60,     90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_480i60_16x9,90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{HDMI_1080p60,    0x7f, 0x89, 0xbe, 0x3e, 0x80, 0x96, 0x2a, 0x08, 0xFF, 0xFF, 0x27, 0x5f, 0x40, 0xFF, 0xFF, 0xFF, 0x63, 0x34, 0xff, 0xff}, //golden
	{HDMI_576p50,     0xf9, 0x35, 0x82, 0x52, 0x30, 0x5e, 0x3e, 0x03, 0xFF, 0xFF, 0x2a, 0x6a, 0x20, 0xFF, 0xFF, 0xFF, 0x6f, 0x32, 0xFF, 0xFF}, //golden
	{HDMI_576p50_16x9,0xf9, 0x35, 0x82, 0x52, 0x30, 0x5e, 0x3e, 0x03, 0xFF, 0xFF, 0x2a, 0x6a, 0x20, 0xFF, 0xFF, 0xFF, 0x6f, 0x32, 0xFF, 0xFF}, //golden
	{HDMI_720p50,     0x1f, 0x67, 0x02, 0x02, 0x61, 0xba, 0x26, 0x07, 0xFF, 0xFF, 0x17, 0xe7, 0x20, 0xFF, 0xFF, 0xFF, 0xec, 0x32, 0xFF, 0xFF}, //golden
	//{HDMI_1080i50,    90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_576i50,     90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_576i50_16x9,90, 91, 92, 93, 94, 95, 96, 97, 9A, 9B, 9C, A0, A1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{HDMI_1080p50,    0xff, 0xa4, 0xbe, 0x3e, 0x80, 0x4e, 0x2a, 0x0a, 0xff, 0xff, 0x27, 0x5f, 0x40, 0xFF, 0xFF, 0xFF, 0x63, 0x34, 0xFF, 0xFF}, //golden
	//{HDMI_1080p24,    0x0F, 0xFF, 92, 93, 94, 95, 96, 97, 9, 9, 9, 0, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_1080p25,    0x0F, 0xFF, 92, 93, 94, 95, 96, 97, 9, 9, 9, 0, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	//{HDMI_1080p30,    0x0F, 0xFF, 92, 93, 94, 95, 96, 97, 9, 9, 9, 0, 1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF,            90, 91, 92, 93, 94, 95, 96, 97, 9, 9, 9, 0xA0, 0xA1, 0xA2, 0xA3, 0xFF, 0xFF, 0xFF},
};


/*
** Bruce.101220. This is for passing HDMI 1.4a certification Test ID 7-25 Video Format Timing.
** AST1500's CRT has 1T of HVSync offset which is not allowed in HDMI certification test.
** To resolve this issue, we need CAT6613 to re-generate the HSync, VSync and DE signals.
** The side effect is the Video Data will shift some how.
** This is a patch for HDMI certification only. It is not necessary in production FW.
**
** To re-generate Sync signal, simply configure Reg 91~A3, tell CAT6613 the position of each
** signal, then kick Reg 90 to apply. This can be done while displaying, but a good place to do
** do it in driver is before configuring the Video timing.
*/
void sync_regen(HDMI_Video_Type OutputVideoTiming)
{
	int i;

	if (OutputVideoTiming == HDMI_Unkown)
		goto regen_off;

	for (i = 0; SRT[i].VIC != 0xFF; i++) {
		if (OutputVideoTiming == SRT[i].VIC)
			break;
	}
	if (SRT[i].VIC == 0xFF)
		goto regen_off;
	
	HDMITX_WriteI2C_Byte(REG_TX_HfPixel,SRT[i].Reg91); // Reg91 //for interlace only. 0xFF for progresive

	HDMITX_WriteI2C_Byte(REG_TX_HDES_L,SRT[i].Reg92); // Reg92
	HDMITX_WriteI2C_Byte(REG_TX_HDEE_L,SRT[i].Reg93); // Reg93
	HDMITX_WriteI2C_Byte(REG_TX_HDE_H,SRT[i].Reg94); // Reg94
	
	HDMITX_WriteI2C_Byte(REG_TX_HSSL,SRT[i].Reg95); // Reg95
	HDMITX_WriteI2C_Byte(REG_TX_HSEL,SRT[i].Reg96); // Reg96
	HDMITX_WriteI2C_Byte(REG_TX_HSH,SRT[i].Reg97); // Reg97

	HDMITX_WriteI2C_Byte(REG_TX_VTOTAL_L,SRT[i].Reg98); // Reg98
	HDMITX_WriteI2C_Byte(REG_TX_VTOTAL_H,SRT[i].Reg99); // Reg99
	HDMITX_WriteI2C_Byte(REG_TX_VDES_L,SRT[i].Reg9A); // Reg9A
	HDMITX_WriteI2C_Byte(REG_TX_VDEE_L,SRT[i].Reg9B); // Reg9B
	HDMITX_WriteI2C_Byte(REG_TX_VDE_H,SRT[i].Reg9C); // Reg9C

	HDMITX_WriteI2C_Byte(REG_TX_VDES2_L,SRT[i].Reg9D); // Reg9D
	HDMITX_WriteI2C_Byte(REG_TX_VDEE2_L,SRT[i].Reg9E); // Reg9E
	HDMITX_WriteI2C_Byte(REG_TX_VDE2_H,SRT[i].Reg9F); // Reg9F

	HDMITX_WriteI2C_Byte(REG_TX_VSS1,SRT[i].RegA0); // RegA0
	HDMITX_WriteI2C_Byte(REG_TX_VSE1,SRT[i].RegA1); // RegA1
	
	HDMITX_WriteI2C_Byte(REG_TX_VSS2,SRT[i].RegA2); // RegA2 //for interlace only. 0xFF for progressive
	HDMITX_WriteI2C_Byte(REG_TX_VSE2,SRT[i].RegA3); // RegA3 //for interlace only. 0xFF for progressive
	barrier();
	HDMITX_WriteI2C_Byte(REG_TX_HVPol,SRT[i].Reg90); // Reg90
	return;

regen_off:
	HDMITX_WriteI2C_Byte(REG_TX_HVPol,0); // Reg90

}
#endif //#if SYNC_REGEN


static void select_YUV_colorimetry(HDMI_Video_Type OutputVideoTiming)
{
    switch(OutputVideoTiming)
	{
	case HDMI_640x480p60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480p60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480p60_16x9:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_720p60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080i60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_480i60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480i60_16x9:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_1080p60:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_576p50:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_576p50_16x9:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_720p50:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080i50:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_576i50:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_576i50_16x9:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_1080p50:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p24:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p25:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p30:
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	default:
		printk("cannot decide colorimetry!!!\n");
		BUG();
    }
}

#if 0
static void select_RGB_quantization_range(HDMI_Video_Type OutputVideoTiming)
{
    switch(OutputVideoTiming)
	{
    case HDMI_480p60:
    case HDMI_480p60_16x9:
    case HDMI_720p60:
    case HDMI_1080i60:
    case HDMI_480i60:
    case HDMI_480i60_16x9:
    case HDMI_1080p60:
    case HDMI_576p50:
    case HDMI_576p50_16x9:
    case HDMI_720p50:
    case HDMI_1080i50:
    case HDMI_576i50:
    case HDMI_576i50_16x9:
    case HDMI_1080p50:
    case HDMI_1080p24:
    case HDMI_1080p25:
    case HDMI_1080p30:
		//limited quantization range
		Instance[0].outputparam.bInputColorMode |= F_MODE_16_235 ;
        break ;
	//IT video formats
    default:
		//full quantization range
		Instance[0].outputparam.bInputColorMode &= ~F_MODE_16_235 ;
        return ;
    }
}
#endif

static BOOL HDMITX_DisplayOutput(/*BYTE InputColorMode,BYTE OutputColorMode,*/
						  unsigned int is_HD, ULONG VCLK,
						  BYTE	bCapibility, BYTE ColorDepth, LpInfoFrame pAVI_InfoFrame, LpInfoFrame pHDMI_InfoFrame)
{
	BYTE bHDMIMode;
	BYTE bHDCP;
	VIDEOPCLKLEVEL level;
	BYTE VIC = HDMI_Unkown;
	
    HDMITX_DEBUG_PRINTF("HDMITX_DisplayOutput() \n");
	printk("VCLK = %ld\n", VCLK);
	printk("bCapibility = %x\n", bCapibility);
	printk("ColorDepth = %d\n", ColorDepth);
    
    /* Set Capability */
    bHDMIMode = bCapibility & CAP_HDMI;
    bHDCP = bCapibility & CAP_HDCP;

#if PARAM_DOUBLE_COPY
//    Instance[0].outputparam.bInputColorMode = InputColorMode;

	if (bHDMIMode)
	{
		if (pAVI_InfoFrame->number > 0)
		{
			unsigned int i;
			printk("AVI InfoFrame:\n");
			for (i = 4; i <= 16; i++)
				printk("%02X ", pAVI_InfoFrame->data[i]);
			printk("\n");
			Instance[0].outputparam.VIC = VIC = pAVI_InfoFrame->data[7] & 0x7F;
			Instance[0].outputparam.VideoPixelClock = VCLK;
			Instance[0].outputparam.pixelrep = pAVI_InfoFrame->data[8] & 0xF;
			printk("VIC = %d\n", Instance[0].outputparam.VIC);
			printk("VideoPixelClock = %lu\n", Instance[0].outputparam.VideoPixelClock);
			printk("PR = %d\n", Instance[0].outputparam.pixelrep);
			Instance[0].outputparam.bOutputColorMode = (pAVI_InfoFrame->data[4] & 0x60) >> 5;
			printk("Y = %d\n", Instance[0].outputparam.bOutputColorMode);
			if (Instance[0].outputparam.bOutputColorMode == F_MODE_RGB444)
			{
			}
			else if ((Instance[0].outputparam.bOutputColorMode == F_MODE_YUV422) || (Instance[0].outputparam.bOutputColorMode == F_MODE_YUV444))
			{
				unsigned char colorimetry = (pAVI_InfoFrame->data[5] & 0xC0) >> 6;
				printk("C = %d\n", colorimetry);
				if (colorimetry == 0)
					//use default colorimetry
					//select_YUV_colorimetry(VIC);
					if (is_HD)
						Instance[0].outputparam.Colorimetry = HDMI_ITU709;
					else
						Instance[0].outputparam.Colorimetry = HDMI_ITU601;

				else if (colorimetry == 1)
					Instance[0].outputparam.Colorimetry = HDMI_ITU601;
				else if (colorimetry == 2)
					Instance[0].outputparam.Colorimetry = HDMI_ITU709;
				else
				{
					printk("got extended colorimetry!!!\n");
					BUG();
				}
				if(Instance[0].outputparam.Colorimetry == HDMI_ITU709 )
				{
					Instance[0].outputparam.bInputColorMode |= F_MODE_ITU709;
				}
				else
				{
					Instance[0].outputparam.bInputColorMode &= ~F_MODE_ITU709;
				}
			}
			else
			{
				BUG();
			}
		}
		else
		{
			printk("AVI InfoFrame unavailable!!!\n");
			Instance[0].outputparam.VideoPixelClock = VCLK;
			Instance[0].outputparam.pixelrep = 0 ;
			Instance[0].outputparam.bOutputColorMode = F_MODE_RGB444 ;
		}
	}
	else
	{
		Instance[0].outputparam.VideoPixelClock = VCLK;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.bOutputColorMode = F_MODE_RGB444 ;
	}
	
    /* save output param */
    //Instance[0].outputparam.VideoSet = 1; //Only set VideoSet after outputparam really set to the HW.
    //Instance[0].outputparam.OutputVideoTiming = OutputVideoTiming;
//    Instance[0].outputparam.VCLK = VCLK;
    Instance[0].outputparam.bCapibility = (Instance[0].outputparam.bCapibility & ~(CAP_HDMI | CAP_HDCP)) | bCapibility;
	Instance[0].outputparam.OutputColorDepth = ColorDepth;
#endif //#if PARAM_DOUBLE_COPY

    /* Update to Instance */
    Instance[0].TMDSClock = 
        Instance[0].outputparam.VideoPixelClock * (Instance[0].outputparam.pixelrep + 1);

    if(!HPDStatus)
    {
    	printk("sink not plugged yet\n");
		return TRUE ;
    }

    Instance[0].outputparam.VideoSet = 1;

#if !HANDLE_INT_IN_THREAD
	DISABLE_INT();
#endif

#if SYNC_REGEN
	sync_regen(VIC);
#endif

#if 0
    if( Instance[0].TMDSClock > 80000000 ) {
        level = PCLK_HIGH ;
    } else if(Instance[0].TMDSClock > 20000000) {
        level = PCLK_MEDIUM ;
    } else {
        level = PCLK_LOW ;
    }
#else//level should be decided according to input video frequency
    if( Instance[0].outputparam.VideoPixelClock > 80000000 ) {
        level = PCLK_HIGH ;
    } else if(Instance[0].outputparam.VideoPixelClock > 20000000) {
        level = PCLK_MEDIUM ;
    } else {
        level = PCLK_LOW ;
    }
#endif

	//Bruce110315. Ported from HDMITX_SetOutput().
	EnableHDCP(FALSE);
	/*
	** Bruce110315. SetOutputColorDepthPhase() should be called before
	** FireAFE() according to ITE. 
	** This is the cause of non-stoppable "audio overflow" and "6613 no picture".
	*/
	SetOutputColorDepthPhase(ColorDepth,0);

    /* Enable Video */    
    EnableVideoOutput(level, Instance[0].outputparam.bInputColorMode, Instance[0].outputparam.bOutputColorMode, bHDMIMode) ;
    
    /* Enable HDMI */
    if( bHDMIMode )
    {
		if (pAVI_InfoFrame->number > 0)
			CopyAVIInfoFrame(&pAVI_InfoFrame->data[3]);
		else
			EnableAVIInfoFrame(FALSE ,NULL);
		if (pHDMI_InfoFrame->number > 0)
		{
			printk("enable HDMI InfoFrame\n");
			{
				unsigned int i;
				printk("HDMI InfoFrame:\n");
				for (i = 0; i < 31; i++)
					printk("%02X ", pHDMI_InfoFrame->data[i]);
				printk("\n");
			}
			EnableVendorSpecificInfoFrame(TRUE, pHDMI_InfoFrame->data);
		}
		else
		{
			printk("disable HDMI InfoFrame\n");
			EnableVendorSpecificInfoFrame(FALSE, NULL);
		}
    }
	else
	{
    	printk("disable AVIInfo\n");
		EnableAVIInfoFrame(FALSE ,NULL);
	}

#if !SUPPORT_HDCP_REPEATER
	if (bHDCP)
	{
		printk("enable HDCP\n");
		EnableHDCP(TRUE) ;
	}
	else
	{
		printk("disable HDCP\n");
		EnableHDCP(FALSE) ;
	}
#endif

	//SetOutputColorDepthPhase(ColorDepth,0);

    /* Enable Audio */
	if (bHDMIMode && Instance[0].outputparam.AudioSet)
    {
#if defined(CODE202)
		EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
		ConfigAudioInfoFrm() ;
#else
        EnableAudioOutput(Instance[0].TMDSClock, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.bAudSWL, Instance[0].outputparam.bSPDIF);            
        ConfigAudioInfoFrm() ;
//        SetupAudioChannel() ;
        EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
#endif
    }

#if SUPPORT_HDCP_REPEATER
	/*
	** Bruce151211. Re-orginize HDCP code flow.
	** - CRT call 'xHal_setup_xmiter()' and 'xHal_hdcp1_auth()' in a pair.
	** - xmiter driver don't need to callback here.
	** - so that 'callback()' is only called when there is something wrong.
	**
	** TODO. Is there is better way to enable HDCP right here instead of
	** calling xHal_hdcp1_auth() from CRT? (Calling xHal_hdcp1_auth() from
	** CRT is a quick solution to cover SiI9678 case.)
	*/
	//CRT_HDCP1_Downstream_Port_Auth_Status_Callback(2, NULL, 0, 0, NULL);
#endif
        
    SetAVMute(FALSE) ;
#if !HANDLE_INT_IN_THREAD
	ENABLE_INT();
#endif
    return TRUE;
    
} /* HDMITX_DisplayOutput */

BOOL HDMITX_AudioOutput(
    BYTE bAudioSampleFreq,
    BYTE ChannelNumber,
    BYTE bAudSWL,
    BYTE bSPDIF,
    BYTE *AUD_DB,
    BYTE *ucIEC60958ChStat,
    BYTE Audio_Type
    )
{
	int i = 0;
    BYTE bHDMIMode = Instance[0].outputparam.bCapibility & CAP_HDMI;
    HDMITX_DEBUG_PRINTF("HDMITX_AudioOutput() \n");
	printk("bAudioSampleFreq = %d\n", bAudioSampleFreq);
	printk("ChannelNumber = %d\n", ChannelNumber);
	printk("bAudSWL = %d\n", bAudSWL);
	printk("bSPDIF = %d\n", bSPDIF);

#if PARAM_DOUBLE_COPY
	/* Bruce101130.Add
	** Add a new feature to disable audio and clear AudioSet by
	** calling HDMITX_AudioOutput() with bAudioSampleFreq and ChannelNumber == 0.
	*/
	if (bAudioSampleFreq == 0 && ChannelNumber == 0) {
		Instance[0].outputparam.AudioSet = 0;
		Instance[0].outputparam.bCapibility &= ~CAP_AUDIO;
		DisableAudioOutput();
		return TRUE;
	}
	/* save output param */
	Instance[0].outputparam.AudioSet = 1;
	Instance[0].outputparam.bAudioSampleFreq = bAudioSampleFreq;
	Instance[0].outputparam.ChannelNumber = ChannelNumber;
	Instance[0].outputparam.bAudSWL = bAudSWL;
	Instance[0].outputparam.bSPDIF = bSPDIF;
	Instance[0].outputparam.bCapibility = Instance[0].outputparam.bCapibility | CAP_AUDIO;
	Instance[0].outputparam.Audio_Type = Audio_Type;
	for(i = 0; i < 5; i++)
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[i] = AUD_DB[i];
	for(i = 0; i < 5; i++)
		Instance[0].outputparam.ucIEC60958ChStat[i] = ucIEC60958ChStat[i];

#else //#if PARAM_DOUBLE_COPY
	if (bAudioSampleFreq == 0) {
		DisableAudioOutput();
		return TRUE;
	}
#endif //#if PARAM_DOUBLE_COPY

    if(!HPDStatus)
    {
    	printk("sink not plugged yet\n");
		return TRUE ;
    }
	
	if (bHDMIMode && Instance[0].outputparam.VideoSet)
	{
#if !HANDLE_INT_IN_THREAD
		DISABLE_INT();
#endif

#ifdef I2S_DBG//steven???
	/*
	** Enabling video causes screen flashing. It is not necessary.
	*/
    EnableVideoOutput(VideoPixelClock, 
                                    Instance[0].outputparam.bInputColorMode, 
                                    Instance[0].outputparam.bOutputColorMode,
                                    (Instance[0].outputparam.bCapibility & CAP_HDMI));

    ConfigAVIInfoFrame(Instance[0].outputparam.bOutputColorMode, VIC, pixelrep) ;

	EnableHDCP(FALSE) ;
	SetOutputColorDepthPhase(Instance[0].outputparam.ColorDepth,0);
#else
#if 0
#if SUPPORT_HDCP_REPEATER
	EnableHDCP(FALSE);
#endif
#endif
#endif //#if I2S_DBG

#if defined(CODE202)
		EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
		ConfigAudioInfoFrm() ;
#else
        EnableAudioOutput(Instance[0].TMDSClock, bAudioSampleFreq, ChannelNumber, bAudSWL, bSPDIF);            
        ConfigAudioInfoFrm() ;
//        SetupAudioChannel() ;
        EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
#endif

#if 0
#if SUPPORT_HDCP_REPEATER
	if (!Instance[0].Hdcp_mode)
	{
		if (Instance[0].outputparam.bCapibility & CAP_HDCP)
		{
			printk("enable HDCP\n");
			HDCP_Reset();
			EnableHDCP(TRUE) ;
		}
		else
		{
			printk("disable HDCP\n");
			EnableHDCP(FALSE) ;
		}
	}
	else
	{
		if (Instance[0].bAuthenticated)
		{
			printk("reenable HDCP authentication\n");
			HDCP_Reset();
			if (EnableHDCP(TRUE) && (Instance[0].Hdcp_encrypt))
			{
				printk("reenable HDCP encryption\n");
#if 0//useless according to JJ
				HDCP_EnableEncryption();
#endif
			}
		}
	}
#endif
#endif

		SetAVMute(FALSE) ;
#if !HANDLE_INT_IN_THREAD
		ENABLE_INT();
#endif
	}
    
    return TRUE;
    
} /* HDMITX_AudioOutput */

static unsigned int cat6613_check_hotplug(struct s_crt_drv *crt)
{
	if(!HPDStatus)
	{
		//printk("sink not plugged yet\n");
		return 0;
	}
	if(!Instance[0].EDIDisReady)
	{
		return 0;
	}
	return 1;
}

static unsigned int cat6613_exist(struct s_crt_drv *crt)
{
	return bcat6613_exist;
}

static unsigned int cat6613_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	if (blk_num > 1)
	{
		uerr("blk_num(%d) not supported\n", blk_num);
		return 1;
	}
	if(!HPDStatus)
	{
		uinfo("sink not plugged yet\n");
		return 2;
	}
	if (!Instance[0].EDIDisReady) {
		uinfo("EDID is not ready yet\n");
		return 3;
	}
	memcpy(pEdid, Instance[0].SinkEDID + (blk_num * 128), 128);
	return 0;
}

static unsigned int cat6613_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum)
{
	if(!HPDStatus)
	{
		uinfo("sink not plugged yet\n");
		return 2;
	}
	if (!Instance[0].EDIDisReady) {
		uinfo("EDID is not ready yet\n");
		return 3;
	}
	*pChksum = Instance[0].SinkEDID[EDID_CHECKSUM_OFFSET];
	return 0;
}

#if 0
unsigned int xmiter_init_cat6613(struct s_crt_drv *crt)
{
	if (!InitCAT6613()){
		uerr("init CAT6613 failed\n");
		return -ENODEV;
	}
	return 0;
}

#if HANDLE_INT_IN_THREAD
void disable_xmiter(void)
{
	DisableCAT6613();
}
#endif
#endif

static unsigned int cat6613_disable(struct s_crt_drv *crt)
{
#if HANDLE_INT_IN_THREAD
	cancel_delayed_work(&disable_task);
	flush_workqueue(wq);
	queue_work(wq, &disable_task);
	flush_workqueue(wq);
#else
	DisableCAT6613();
#endif
	return 0;
}

static unsigned int cat6613_init(struct s_crt_drv *crt)
{
	return cat6613_disable(crt);
}

#if HANDLE_INT_IN_THREAD
void setup_video(
#if PARAM_DOUBLE_COPY
struct ws_setup_video *ws_video
#endif
)
{
	//set up CAT6613
#if PARAM_DOUBLE_COPY
	if (!HDMITX_DisplayOutput(
//		ws_video->InputColorMode, 
//		ws_video->OutputColorMode, 
		ws_video->is_HD,
		ws_video->VCLK, 
		ws_video->bCapibility, 
		ws_video->ColorDepth,
		&ws_video->AVI_Info,
		&ws_video->HDMI_Info))
#endif //#if PARAM_DOUBLE_COPY
	{
		uerr("set up CAT6613 failed\n");
	}
}
#endif

static int _is_same_timing(
	u32 vclk, 
	u8 capability, 
	u8 color_depth, 
	InfoFrame *AVI_Info, 
	InfoFrame *HDMI_Info)
{
	if (Instance[0].outputparam.VideoSet == 0)
		return 0;

	if (ws_video.VCLK != vclk)
		return 0;

	if (ws_video.bCapibility != capability)
		return 0;
	
	if (ws_video.ColorDepth != color_depth)
		return 0;

	if (memcmp(&ws_video.AVI_Info, AVI_Info, sizeof(InfoFrame)))
		return 0;

	if (memcmp(&ws_video.HDMI_Info, HDMI_Info, sizeof(InfoFrame)))
		return 0;
	
	return 1;
}

static inline unsigned int is_HD_timing(MODE_ITEM *mt, int i)
{
	unsigned int VActive = mt[i].VActive;

	if (mt[i].ScanMode == Interl)
		VActive <<= 1;

	if (VActive >= 720)
		return 1;

	return 0;
}

static unsigned int cat6613_setup_video(struct s_crt_drv *crt, struct s_crt_info *info)
{
	u32 vclk;
	u8 capability = 0;
	int i;
	MODE_ITEM *mt = crt->mode_table;
	struct s_xmiter_mode *xmiter_mode = &info->xmiter_mode;

#if 0//redundant
	/* xmiter is for digital output only. */
	if (OUTPUT_SELECT_ANALOG == crt->disp_select)
		return 0;
#endif

	if (info->EnHDCP)
		capability |= CAP_HDCP;

	//look up mode
	//DstWidth,DstHight is CRT resolution
	i = lookupModeTable(crt, info);

	vclk = mt[i].DCLK10000 * 100;

	/* Following code are used to configure the xmiter's AVInfoFrame. (Digital Only). */
	switch (xmiter_mode->HDMI_Mode)
	{
	case s_RGB:
		//Means default mode from non-VE (OWNER_CON).
		uinfo("DVI mode\n");
		break;
	case s_DVI:
		uinfo("DVI mode\n");
		break;
	case s_HDMI_4x3:
		uinfo("HDMI 4:3 mode\n");
		capability |= CAP_HDMI;
		break;
	case s_HDMI_16x9:
		uinfo("HDMI 16:9 mode\n");
		capability |= CAP_HDMI;
		break;
	default:
		uerr("Unknown HDMI type!?(%d)\n", xmiter_mode->HDMI_Mode);
		BUG();
		break;
	}

	/* Bruce140905. Some PC output wrong timing but using HDMI format.
	** Some ot these timings in our vesa table has HDMI_Unkown video type.
	** It will confuse it6613 driver when AVIinfo frame's colorimetry code is unknown.
	** And the stupid select_YUV_colorimetry() can't find a proper colorimetry code.
	** Bruce141231. We use 'is_HD' to replace select_YUV_colorimetry()
		 */

	if ((info->crt_output_format == XRGB8888_FORMAT) && (xmiter_mode->AVI_Info.number)) {
		if (xmiter_mode->AVI_Info.data[4] & (0x3 << 5)) { /* NOT RGB */
			int i;
			/* modify to RGB */
			xmiter_mode->AVI_Info.data[4] &= ~(0x3 << 5);

			/* Calculate AVI InfoFrame Checksum */
			xmiter_mode->AVI_Info.data[3] = 0x82 + 0x02 + 0x0D;

			/* AVI_Info data[0] ~ [2] => header; data[3] ~ [16] => contents */
			for (i = 4; i < 17; i++)
				xmiter_mode->AVI_Info.data[3] += xmiter_mode->AVI_Info.data[i];

			xmiter_mode->AVI_Info.data[3] = 0x100 - xmiter_mode->AVI_Info.data[3];
		}
	}

#if HANDLE_INT_IN_THREAD
	cancel_delayed_work(&setup_video_task);
	flush_workqueue(wq);

	/* BugFix.RctBug#2013081500.
	** We have to flush_workqueue() to make sure there is no other work items queued (Ex:disable_task)
	** before checking _is_same_timing().
	** Otherwise, a race will happen when crt calls cat6613_disable(), then cat6613_setup_video().
	*/
	if (_is_same_timing(vclk, capability, xmiter_mode->color_depth, &xmiter_mode->AVI_Info, &xmiter_mode->HDMI_Info)) {
		uinfo("Same video timing. Ignore.\n");
		return 0;
	}

	/* ws_video must be protected from two context calling cat6613_setup_video(). */
	down(&drv_lock);
//	ws_video.InputColorMode = HDMI_RGB444;
//	ws_video.OutputColorMode = HDMI_RGB444;
//	ws_video.OutputVideoTiming = videotype;
	ws_video.VCLK = vclk;
	ws_video.bCapibility = capability;
	ws_video.ColorDepth = xmiter_mode->color_depth;
	ws_video.is_HD = is_HD_timing(mt, i);
	memcpy(&ws_video.AVI_Info, &xmiter_mode->AVI_Info, sizeof(InfoFrame));
	memcpy(&ws_video.HDMI_Info, &xmiter_mode->HDMI_Info, sizeof(InfoFrame));
	queue_work(wq, &setup_video_task);
	up(&drv_lock);

	flush_workqueue(wq);
#endif /* #if HANDLE_INT_IN_THREAD */
	return 0;
}

#ifdef CONFIG_ARCH_AST1500_HOST
static void setup_loopback_video(struct ws_setup_video *ws_video)
{
    BYTE bHDMIMode = ws_video->bCapibility & CAP_HDMI;

    EnableVideoOutput(ws_video->level, HDMI_RGB444, F_MODE_RGB444, bHDMIMode) ;
    
    /* Enable HDMI */
    if( bHDMIMode )
    {
		if (ws_video->AVI_Info.number > 0)
			CopyAVIInfoFrame(&ws_video->AVI_Info.data[3]);
		else
			EnableAVIInfoFrame(FALSE ,NULL);
		if (ws_video->HDMI_Info.number > 0)
		{
			EnableVendorSpecificInfoFrame(TRUE, ws_video->HDMI_Info.data);
		}
		else
		{
			EnableVendorSpecificInfoFrame(FALSE, NULL);
		}
    }
	else
	{
		EnableAVIInfoFrame(FALSE ,NULL);
	}

	Instance[0].outputparam.VideoSet = 1;
}
void cat6613_setup_loopback_video(VIDEOPCLKLEVEL level, unsigned char bHDMI, unsigned char *pAVI_InfoFrame, unsigned char *pHDMI_InfoFrame)
{
	cancel_delayed_work(&setup_loopback_video_task);
	flush_workqueue(wq);

	/* ws_video must be protected from two context calling cat6613_setup_loopback_video(). */
	down(&drv_lock);

	ws_video.level = level;
	if (bHDMI)
		ws_video.bCapibility |= CAP_HDMI;
	else
		ws_video.bCapibility &= CAP_HDMI;
	if (pAVI_InfoFrame)
	{
		ws_video.AVI_Info.number = 1;
		memcpy(ws_video.AVI_Info.data, pAVI_InfoFrame, sizeof(InfoFrame));
	}
	else
		ws_video.AVI_Info.number = 0;
	if (pHDMI_InfoFrame)
	{
		ws_video.HDMI_Info.number = 1;
		memcpy(ws_video.HDMI_Info.data, pHDMI_InfoFrame, sizeof(InfoFrame));
	}
	else
		ws_video.HDMI_Info.number = 0;

	queue_work(wq, &setup_loopback_video_task);

	up(&drv_lock);

	flush_workqueue(wq);
}
EXPORT_SYMBOL(cat6613_setup_loopback_video);
#endif

#if HANDLE_INT_IN_THREAD
void setup_audio(
#if PARAM_DOUBLE_COPY
struct ws_setup_audio *ws_audio
#endif
)
{
#if PARAM_DOUBLE_COPY
	HDMITX_AudioOutput(
		ws_audio->SampleFrequency,
		ws_audio->ValidCh,
		ws_audio->SampleSize,
		ws_audio->bSPDIF,
		ws_audio->audio_infoframe.pktbyte.AUD_DB,
		ws_audio->ucIEC60958ChStat,
		ws_audio->Audio_Type);
#else //#if PARAM_DOUBLE_COPY
	HDMITX_AudioOutput(
		Instance[0].outputparam.bAudioSampleFreq,
		Instance[0].outputparam.ChannelNumber,
		Instance[0].outputparam.bAudSWL,
		Instance[0].outputparam.bSPDIF,
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB,
		Instance[0].outputparam.ucIEC60958ChStat,
		Instance[0].outputparam.Audio_Type);
#endif //#if PARAM_DOUBLE_COPY
}
#endif

static unsigned int cat6613_setup_audio(Audio_Info *audio_info)
{
	int i = 0;

#if HANDLE_INT_IN_THREAD
	cancel_delayed_work(&setup_audio_task);
	flush_workqueue(wq);

	/* ws_audio must be protected from two context calling cat6613_setup_audio(). */
	down(&drv_lock);

#if PARAM_DOUBLE_COPY
	ws_audio.SampleFrequency = audio_info->SampleFrequency;
	ws_audio.ValidCh = audio_info->ValidCh;
	ws_audio.SampleSize = audio_info->SampleSize;

#if (API_VER < 2)
	//SPDIF is never enabled in AST1500
	ws_audio.bSPDIF = 0;
	//For backward compatibility, AST1500 only supports LPCM.
	ws_audio.Audio_Type = T_AUDIO_LPCM;
#else
	ws_audio.bSPDIF = audio_info->bSPDIF;
	for(i = 0; i < 5; i++)
		ws_audio.audio_infoframe.pktbyte.AUD_DB[i] = audio_info->AUD_DB[i];
	for(i = 0; i < 5; i++)
		ws_audio.ucIEC60958ChStat[i] = audio_info->ucIEC60958ChStat[i];
	ws_audio.Audio_Type = audio_info->Audio_Type;
#endif

#else //#if PARAM_DOUBLE_COPY
	/* save output param */
	if (crt->audio_info.SampleFrequency == 0) {
		Instance[0].outputparam.AudioSet = 0;
		Instance[0].outputparam.bAudioSampleFreq = 0;
		Instance[0].outputparam.bCapibility &= ~CAP_AUDIO;
	} else {
		Instance[0].outputparam.AudioSet = 1;
		Instance[0].outputparam.bAudioSampleFreq = crt->audio_info.SampleFrequency;
		Instance[0].outputparam.ChannelNumber = crt->audio_info.ValidCh;
		Instance[0].outputparam.bAudSWL = crt->audio_info.SampleSize;
		Instance[0].outputparam.bSPDIF = crt->audio_info.bSPDIF;
		Instance[0].outputparam.bCapibility = Instance[0].outputparam.bCapibility | CAP_AUDIO;
		for(i = 0; i < 5; i++)
			Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[i] = crt->audio_info.AUD_DB[i];
		for(i = 0; i < 5; i++)
			Instance[0].outputparam.ucIEC60958ChStat[i] = crt->audio_info.ucIEC60958ChStat[i];
		Instance[0].outputparam.Audio_Type = crt->audio_info.Audio_Type;
	}
#endif //#if PARAM_DOUBLE_COPY

	queue_work(wq, &setup_audio_task);
	up(&drv_lock);

	flush_workqueue(wq);
#else //#if HANDLE_INT_IN_THREAD
	HDMITX_AudioOutput(crt->audio_info.SampleFrequency,
		crt->audio_info.ValidCh,
		crt->audio_info.SampleSize,
		crt->audio_info.bSPDIF,
		crt->audio_info.AUD_DB,
		crt->audio_info.ucIEC60958ChStat,
		crt->audio_info.Audio_Type);
#endif

	return 0;
}

static void cat6613_disable_audio(void)
{
#if HANDLE_INT_IN_THREAD
	cancel_delayed_work(&disable_audio_task);
	flush_workqueue(wq);
	queue_work(wq, &disable_audio_task);
#else
	DisableAudioOutput();
#endif
}

#if SUPPORT_HDCP_REPEATER

void disable_hdcp1(void)
{
	EnableHDCP(FALSE);
}

static void CAT6613_HDCP1_Downstream_Port_Set_Mode(unsigned char repeater)
{
	printk("CAT6613_HDCP1_Downstream_Port_Set_Mode(%d)\n", repeater);
	if (repeater)
	{
		if (!Instance[0].Hdcp_mode)
		{
			Instance[0].Hdcp_mode = 1;
			cancel_delayed_work(&Hdcp_disable_work);
			flush_workqueue(wq);
			queue_work(wq, &Hdcp_disable_work);
		}
		else
			printk("HDCP1 downstream port is already in repeater mode.\n");
	}
	else
	{
		if (Instance[0].Hdcp_mode)
		{
			Instance[0].Hdcp_mode = 0;
			//to do
		}
		else
			printk("HDCP1 downstream port is already in transmitter mode.\n");
	}
}

static void start_hdcp1_auth(int enable)
{
	if (!enable) {
		Instance[0].bAuthenticated = FALSE;
		HDCP_Reset();
		return;
	}
	// enable HDCP
	if(!Instance[0].outputparam.VideoSet)
	{
		printk("video parameters not set yet\n");
		//Bruce121224. It will cause infinite loop of retry. Ignore this case should be fine.
		//CRT_HDCP1_Downstream_Port_Auth_Status_Callback(0, NULL, Instance[0].Bcaps, 0, NULL);
		return;
	}
	{
		int ret;

		HDCP_Reset();
#ifdef FLASH_SCREEN
		SetAVMute(TRUE);
#endif
		ret = EnableHDCP(TRUE);
#ifdef FLASH_SCREEN
		if (!ret) {
			//This delay is needed for user can see the screen flash when HDCP failed.
			msleep(32);
		}
		SetAVMute(FALSE);
#endif
		if (ret) {
			if (HDCP_V_1X == enable) {
				CRT_HDCP1_Downstream_Port_Auth_Status_Callback(1, Instance[0].Bksv,  Instance[0].Bcaps, Instance[0].Bstatus, Instance[0].KSVList);
				return;
			}
		}

		CRT_HDCP1_Downstream_Port_Auth_Status_Callback(0, NULL, Instance[0].Bcaps, 0, NULL);

	}
}

static void hdcp1_auth_start(void)
{
	start_hdcp1_auth(1);
}

static void hdcp1_auth_stop(void)
{
	start_hdcp1_auth(0);
}

static void CAT6613_HDCP1_Downstream_Port_Auth(int enable)
{
	printk("CAT6613_HDCP1_Downstream_Port_Auth (%s)\n", (enable)?("enable"):("disable"));

	flush_workqueue(wq);

	if (enable) {
		queue_work(wq, &Hdcp_start_auth_work);
	} else {
		queue_work(wq, &Hdcp_stop_auth_work);
	}

	flush_workqueue(wq);
}

#if 0
void Get_HDCP_Downstream_Port_Auth_Status(unsigned char *is_authenticated, unsigned char *is_repeater, void **pksv_list, unsigned char *bstatus)
{
	if (Instance[0].bAuthenticated)
	{
		*is_authenticated = 1;
		*is_repeater = Instance[0].Rx_is_Repeater;
		if (*is_repeater)
			*pksv_list = KSVList;
		else
			*pksv_list = NULL;
		*bstatus = Instance[0].Bstatus;
	}
	else
	{
		*is_authenticated = 0;
		*is_repeater = 0;
		*pksv_list = NULL;
		*bstatus = 0;
	}
}
EXPORT_SYMBOL(Get_HDCP_Downstream_Port_Auth_Status);
#endif

static void CAT6613_HDCP1_Downstream_Port_Encrypt(unsigned char enable)
{
	printk("CAT6613_HDCP1_Downstream_Port_Encrypt(%d)\n", enable);
	if (!Instance[0].Hdcp_mode)
	{
		printk("HDMI Tx not in REPEATER mode!!!\n");
		return;
	}
}
#endif//#if SUPPORT_HDCP_REPEATER

#if	defined(Linux_KernelMode)
MODULE_LICENSE ("GPL");

int  ioaccess_major = 0;

int  ioaccess_ioctl (struct inode *inode, struct file *filp, unsigned cmd, unsigned long arg);

struct file_operations ioaccess_fops = {
#if 0
    ioctl:    ioaccess_ioctl,
#endif
};

#if 0
int ioaccess_ioctl(struct inode *inode, struct file *filp,
                   unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    IO_CAT6613_DATA io_reg;
    EDID_DATA edid_data;
    OUTPUT_OPTION output_option;
    
    switch (cmd) {
    case IOCTL_CAT6613_IO_READ:
        io_reg = *(IO_CAT6613_DATA*)arg;
        
        Switch_HDMITX_Bank(io_reg.Bank) ;    
        io_reg.Data = HDMITX_ReadI2C_Byte(io_reg.Index);
        
        if (copy_to_user((void *)arg, &io_reg, sizeof(IO_CAT6613_DATA)))
        {
	     ret = -1;
	}
        
    	break;

    case IOCTL_CAT6613_IO_WRITE:
        io_reg = *(IO_CAT6613_DATA*)arg;
        
        Switch_HDMITX_Bank(io_reg.Bank) ;    
        io_reg.Data = HDMITX_WriteI2C_Byte(io_reg.Index, io_reg.Data);
                
    	break;    	
    	
    case IOCTL_CAT6613_GETEDID:
        edid_data = *(EDID_DATA*)arg;
        
        if (GetEDIDData(edid_data.Blk, edid_data.Data) == FALSE)
        {
            ret = -1;	
        }
        else	
        {
            if (copy_to_user((void *)arg, &edid_data, sizeof(EDID_DATA)))
            {
	        ret = -1;
	    }
        }
    	break;

    case IOCTL_CAT6613_SETOUTPUT:
        output_option = *(OUTPUT_OPTION*)arg;
		if (HDMITX_DisplayOutput(output_option.bInputColorMode,
			output_option.bOutputColorMode,
			output_option.OutputVideoTiming,
			output_option.VCLK,
			output_option.bCapibility,
			output_option.OutputColorDepth) == FALSE)
		{
            ret = -1;
			break;
		}
		if (
	#if 0 //Bruce don't want to maintain this
			HDMITX_AudioOutput(output_option.bAudioSampleFreq,
			output_option.ChannelNumber,
			output_option.bAudSWL,
			output_option.bSPDIF,
			output_option.AUD_DB) == FALSE
	#else
			FALSE
	#endif
			)
		{
            ret = -1;
		}
		break;
            
    default:
        ret = -1;    	
    }
    
    return ret;
}
#endif

static irqreturn_t cat6613_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
//    ULONG uldata;
	
    /* Check if the interrupt is triggered by CAT6613 */
    //check share interrupt
    if (!gpio_get_int_stat(GPIO_CAT6613_INT)) {
		return IRQ_NONE;
    }
    HDMITX_DEBUG_PRINTF("In IRQ routine .... \n");
#if 0
    //ack interrupt
    gpio_ack_int_stat(AST1500_GPD4);

    /* IRQ Handle */
    CheckHDMITX(&HPDStatus,&HPDChangeStatus);
#else
	/*Disable & ack interrupt here.
	Enable interrupt after tasklet has finished interrupt processing*/
	DISABLE_INT();

#if HANDLE_INT_IN_THREAD
	queue_work(wq, &hotplug_task);
#else
	tasklet_schedule(&hotplug_task);
#endif

#endif
    
    HDMITX_DEBUG_PRINTF("exit IRQ routine\n");
    return IRQ_HANDLED;
} /* cat6613_interrupt */



static ssize_t show_ctrl(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf,
"\n-Control Commands-----------------------------------\n\
shutdown\n\
setup_video\n\
setup_audio\n\
hdcp_off\n\
hdcp_on\n\
hdcp_resume\n\
hdcp_reset\n\
int_off\n\
int_on\n\
avmute_on\n\
avmute_off\n\
"
);

	sprintf(buf+strlen(buf), "-Video Info-----------------------------------------\n");
//	sprintf(buf+strlen(buf), "ws_video.InputColorMode=%d,\n", ws_video.InputColorMode);
//	sprintf(buf+strlen(buf), "ws_video.OutputColorMode=%d,\n", ws_video.OutputColorMode);
//	sprintf(buf+strlen(buf), "ws_video.OutputVideoTiming=%d,\n", ws_video.OutputVideoTiming);
	sprintf(buf+strlen(buf), "ws_video.VCLK=%ld,\n", ws_video.VCLK);
	sprintf(buf+strlen(buf), "ws_video.bCapibility=%d,\n", ws_video.bCapibility);
	sprintf(buf+strlen(buf), "ws_video.ColorDepth=%d,\n", ws_video.ColorDepth);
	sprintf(buf+strlen(buf), "ws_video.is_HD=%d,\n", ws_video.is_HD);
	sprintf(buf+strlen(buf), "ws_video.AVI_Info=%d,\n", ws_video.AVI_Info.number);
	sprintf(buf+strlen(buf), "ws_video.HDMI_Info=%d,\n", ws_video.HDMI_Info.number);

	sprintf(buf+strlen(buf), "-Audio Info-----------------------------------------\n");
	sprintf(buf+strlen(buf), "ws_audio.SampleFrequency=%d,\n", ws_audio.SampleFrequency);
	sprintf(buf+strlen(buf), "ws_audio.ValidCh=%d,\n", ws_audio.ValidCh);
	sprintf(buf+strlen(buf), "ws_audio.SampleSize=%d,\n", ws_audio.SampleSize);
	sprintf(buf+strlen(buf), "ws_audio.bSPDIF=%d,\n", ws_audio.bSPDIF);
	sprintf(buf+strlen(buf), "ws_audio.Audio_Type=%02X,\n", ws_audio.Audio_Type);
	sprintf(buf+strlen(buf), "ws_audio.audio_infoframe=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X,\n", 
		ws_audio.audio_infoframe.pktbyte.AUD_DB[0],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[1],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[2],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[3],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[4],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[5],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[6],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[7],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[8],
		ws_audio.audio_infoframe.pktbyte.AUD_DB[9]);
	sprintf(buf+strlen(buf), "ws_audio.ChStat=%02X %02X %02X %02X %02X,\n", 
		ws_audio.ucIEC60958ChStat[0], ws_audio.ucIEC60958ChStat[1],
		ws_audio.ucIEC60958ChStat[2], ws_audio.ucIEC60958ChStat[3],
		ws_audio.ucIEC60958ChStat[4]);

	sprintf(buf+strlen(buf), "-Output Parameters----------------------------------\n");
	sprintf(buf+strlen(buf), "bHDMIMode=%d,\n", Instance[0].outputparam.bHDMIMode);
	sprintf(buf+strlen(buf), "VideoSet=%d,\n", Instance[0].outputparam.VideoSet);
	sprintf(buf+strlen(buf), "AudioSet=%d,\n", Instance[0].outputparam.AudioSet);
	sprintf(buf+strlen(buf), "bInputColorMode=%d,\n", Instance[0].outputparam.bInputColorMode);
	sprintf(buf+strlen(buf), "bOutputColorMode=%d,\n", Instance[0].outputparam.bOutputColorMode);
//	sprintf(buf+strlen(buf), "OutputVideoTiming=%d,\n", Instance[0].outputparam.OutputVideoTiming);
//	sprintf(buf+strlen(buf), "VCLK=%ld,\n", Instance[0].outputparam.VCLK);
	sprintf(buf+strlen(buf), "bAudioSampleFreq=%d,\n", Instance[0].outputparam.bAudioSampleFreq);
	sprintf(buf+strlen(buf), "ChannelNumber=%d,\n", Instance[0].outputparam.ChannelNumber);
	sprintf(buf+strlen(buf), "bAudSWL=%d,\n", Instance[0].outputparam.bAudSWL);
	sprintf(buf+strlen(buf), "bSPDIF=%d,\n", Instance[0].outputparam.bSPDIF);
	sprintf(buf+strlen(buf), "bCapibility=%d,\n", Instance[0].outputparam.bCapibility);
	sprintf(buf+strlen(buf), "OutputColorDepth=%d,\n", Instance[0].outputparam.OutputColorDepth);
	sprintf(buf+strlen(buf), "pixelrep=%d,\n", Instance[0].outputparam.pixelrep);
	sprintf(buf+strlen(buf), "VIC=%d,\n", Instance[0].outputparam.VIC);
	sprintf(buf+strlen(buf), "Audio_Type=%02X,\n", Instance[0].outputparam.Audio_Type);
	sprintf(buf+strlen(buf), "VideoPixelClock=%ld,\n", Instance[0].outputparam.VideoPixelClock);
//	sprintf(buf+strlen(buf), "aspec=%d,\n", Instance[0].outputparam.aspec);
	sprintf(buf+strlen(buf), "Colorimetry=%d,\n", Instance[0].outputparam.Colorimetry);
	sprintf(buf+strlen(buf), "audio_infoframe=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X,\n", 
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[0],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[1],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[2],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[3],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[4],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[5],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[6],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[7],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[8],
		Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[9]);
	sprintf(buf+strlen(buf), "ucIEC60958ChStat=%02X %02X %02X %02X %02X,\n", 
		Instance[0].outputparam.ucIEC60958ChStat[0],
		Instance[0].outputparam.ucIEC60958ChStat[1],
		Instance[0].outputparam.ucIEC60958ChStat[2],
		Instance[0].outputparam.ucIEC60958ChStat[3],
		Instance[0].outputparam.ucIEC60958ChStat[4]);

	return strlen(buf);
}

static ssize_t store_ctrl(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	static unsigned char cmd[64];

	memset(cmd, 0, 64);
	sscanf(buf, "%s", cmd);

	if (!strncmp(cmd, "shutdown", strlen("shutdown"))) {
		DisableCAT6613();
		goto out;
	}
	if (!strncmp(cmd, "setup_video", strlen("setup_video"))) {
		queue_work(wq, &setup_video_task);
		goto out;
	}
	if (!strncmp(cmd, "setup_audio", strlen("setup_audio"))) {
		queue_work(wq, &setup_audio_task);
		goto out;
	}
	if (!strncmp(cmd, "hdcp_off", strlen("hdcp_off"))) {
		EnableHDCP(FALSE);
		goto out;
	}
	if (!strncmp(cmd, "hdcp_on", strlen("hdcp_on"))) {
		EnableHDCP(TRUE);
		goto out;
	}
	if (!strncmp(cmd, "hdcp_resume", strlen("hdcp_resume"))) {
		HDCP_ResumeAuthentication();
		goto out;
	}
	if (!strncmp(cmd, "hdcp_reset", strlen("hdcp_reset"))) {
		HDCP_Reset();
		goto out;
	}
	if (!strncmp(cmd, "int_off", strlen("int_off"))) {
		DISABLE_INT();
		goto out;
	}
	if (!strncmp(cmd, "int_on", strlen("int_on"))) {
		ENABLE_INT();
		goto out;
	}
	if (!strncmp(cmd, "avmute_on", strlen("avmute_on"))) {
		SetAVMute(TRUE);
		goto out;
	}
	if (!strncmp(cmd, "avmute_off", strlen("avmute_off"))) {
		SetAVMute(FALSE);
		goto out;
	}
	uerr("Unknown command!\n");
	return -1;
out:
	return count;
}
static DEVICE_ATTR(ctrl, (S_IRUGO | S_IWUSR), show_ctrl, store_ctrl);

#if 0
static ssize_t show_reg_d(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int i;
    Switch_HDMITX_Bank(0);
	printk("bank 0:");
	for (i = 0x00; i < 0x100; i++)
	{
		if ((i % 16) == 0)
			printk("\n%02X	", i);
		else
			printk(" ");
		printk("%02X", HDMITX_ReadI2C_Byte(i));
	}
    Switch_HDMITX_Bank(1);
	printk("\nbank 1:");
	for (i = 0x30; i < 0x100; i++)
	{
		if ((i % 16) == 0)
			printk("\n%02X	", i);
		else
			printk(" ");
		printk("%02X", HDMITX_ReadI2C_Byte(i));
	}
    Switch_HDMITX_Bank(0);
	return 0;
}
DEVICE_ATTR(reg_d, S_IRUGO, show_reg_d, NULL);
#else
static ssize_t show_register(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int i;
	printk("HDMI Tx register dump:\n");
	printk("bank0\n");
    Switch_HDMITX_Bank(0);
	for (i = 0; i < 0x100; i++)
	{
		if ((i & 0xF) == 0)
			printk("%02X: %02X ", i, HDMITX_ReadI2C_Byte(i));
		else if ((i & 0xF) == 0xF)
			printk("%02X\n", HDMITX_ReadI2C_Byte(i));
		else
			printk("%02X ", HDMITX_ReadI2C_Byte(i));
	}
	printk("bank1\n");
    Switch_HDMITX_Bank(1);
	for (i = 0x30; i < 0x100; i++)
	{
		if ((i & 0xF) == 0)
			printk("%02X: %02X ", i, HDMITX_ReadI2C_Byte(i));
		else if ((i & 0xF) == 0xF)
			printk("%02X\n", HDMITX_ReadI2C_Byte(i));
		else
			printk("%02X ", HDMITX_ReadI2C_Byte(i));
	}
    Switch_HDMITX_Bank(0);
	return 0;
}

static ssize_t store_register(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int bank, offset, value;
	unsigned int c;

	c = sscanf(buf, "%x %x %x", &bank, &offset, &value);
	if (c == 2)
	{
		if (bank == 0)
		{
			Switch_HDMITX_Bank(0);
			value = HDMITX_ReadI2C_Byte(offset);
		}
		else if (bank == 1)
		{
			Switch_HDMITX_Bank(1);
			value = HDMITX_ReadI2C_Byte(offset);
			Switch_HDMITX_Bank(0);
		}
		printk("HDMI Tx bank%X register%02X is %02X.\n", bank, offset, value);
	}
	else if (c == 3)
	{
		printk("Set HDMI Tx bank%X register%02X to %02X.\n", bank, offset, value);
		if (bank == 0)
		{
			Switch_HDMITX_Bank(0);
			HDMITX_WriteI2C_Byte(offset, value);
		}
		else if (bank == 1)
		{
			Switch_HDMITX_Bank(1);
			HDMITX_WriteI2C_Byte(offset, value);
			Switch_HDMITX_Bank(0);
		}
	}
	else
	{
		printk("Usage:\nBank Offset [Value]\n");
	}
	return count;
}
static DEVICE_ATTR(register, (S_IRUGO | S_IWUSR), show_register, store_register);
#endif


static const char driver_name[] = "it6613";


static int dummy_probe(struct platform_device *pdev)
{
	return 0;
}
static int dummy_remove(struct platform_device *pdev)
{
	return 0;
}
static void the_pdev_release(struct device *dev)
{
	return;
}

static struct attribute *dev_attrs[] = {
	&dev_attr_ctrl.attr,
#if 0
	&dev_attr_reg_d.attr,
#else
	&dev_attr_register.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) driver_name,
	.id = -1,
	.dev = {
		.release = the_pdev_release,
		.driver_data = NULL,
	},
};

static struct platform_driver cat6613_driver = {
	.probe	       = dummy_probe,
	.remove	= __devexit_p(dummy_remove),
	.suspend    = NULL,
	.resume	= NULL,
	.driver	= {
		.name = (char *) driver_name,
		.owner = THIS_MODULE,
	},
};

int cat6613_sysfs_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&cat6613_driver);
	if (ret < 0) {
		uerr("platform_driver_register err\n");
		goto err_driver_register;
	}

	ret = platform_device_register(&pdev);
	if (ret < 0) {
		uerr("platform_device_register err\n");
		goto err_platform_device_register;
	}

	ret = sysfs_create_group(&pdev.dev.kobj, &dev_attr_group);
	if (ret) {
		uerr("can't create sysfs files\n");
		goto err_sysfs_create;
	}

	uinfo("cat6613 sysfs initialized\n");
	return 0;

err_sysfs_create:
	platform_device_unregister(&pdev);	
err_platform_device_register:
	platform_driver_unregister(&cat6613_driver);
err_driver_register:
	return ret;

}

int cat6613_sysfs_free(void)
{
	platform_device_unregister(&pdev);
	platform_driver_unregister(&cat6613_driver);
	return 0;
}

#if NO_INT_MODE
void __HDMITX_DevLoopProc(void)
{
	HDMITX_DevLoopProc();

	queue_delayed_work(wq, &hotplug_task, msecs_to_jiffies(1000));
}
#endif


extern struct s_xmiter_info xmiter_info[];

static void xmiter_info_init(void)
{
	struct s_xmiter_info *pinfo;

#if defined(CONFIG_ARCH_AST1500_HOST)
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
	pinfo = xmiter_info + XIDX_HOST_D;
#endif
#elif defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#else
	return ;
#endif
	memset(pinfo, 0, sizeof(struct s_xmiter_info));

	pinfo->chip		= xmiter_cat6613;
	pinfo->cap		= (xCap_DVI | xCap_HDMI);
	pinfo->exist		= cat6613_exist;
	pinfo->setup		= cat6613_setup_video;
	pinfo->setup_audio	= cat6613_setup_audio;
	pinfo->disable		= cat6613_disable;
	pinfo->disable_audio	= cat6613_disable_audio;
	pinfo->init		= cat6613_init;
	pinfo->check_hotplug	= cat6613_check_hotplug;
	pinfo->rd_edid_chksum	= cat6613_rd_edid_chksum;
	pinfo->rd_edid		= cat6613_rd_edid;
	pinfo->poll_hotplug	= NULL;
#if SUPPORT_HDCP_REPEATER
	pinfo->Hdcp1_set_mode	= CAT6613_HDCP1_Downstream_Port_Set_Mode;
	pinfo->Hdcp1_auth	= CAT6613_HDCP1_Downstream_Port_Auth;
	pinfo->Hdcp1_encrypt	= CAT6613_HDCP1_Downstream_Port_Encrypt;
#endif
}

static void xmiter_info_fini(void)
{
	struct s_xmiter_info *pinfo;

#if defined(CONFIG_ARCH_AST1500_HOST)
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
	pinfo = xmiter_info + XIDX_HOST_D;
#endif
#elif defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#else
	return ;
#endif
	memset(pinfo, 0, sizeof(struct s_xmiter_info));
}

static u8 is_xmiter_info_registered(void)
{
	struct s_xmiter_info *pinfo;

#if defined(CONFIG_ARCH_AST1500_HOST)
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
	pinfo = xmiter_info + XIDX_HOST_D;
#endif
#elif defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#else
	return 1; /* exception, return 1 to skip some configuration */
#endif

	if (xmiter_none == pinfo->chip)
		return 0;

	return 1;
}


#define CAT6613_DETECT_RETRY_LIMIT 3
int my_init(void)
{
    int    result, retval;
    USHORT VendorID, DeviceID; 	
    BYTE pData[4];

#if 0 //Bruce101202. Loopback Tx will be handled by crt
	unsigned char init_AUD_DB[5];
#endif

//    ULONG uldata;
	int i;

#if 0 //Bruce101202. Loopback Tx will be handled by crt
	for(i = 0; i < 4; i++)
		init_AUD_DB[i] = 0;
#endif

	HDMITX_DEBUG_PRINTF("Load CAT6613 Driver %s\n", jVersion);

	/* FIXME
	 * this is software workaround for multiple transmitters on a single board
	 */
	if (is_xmiter_info_registered()) {
		uinfo("Already have another transmitter registered !\n");
		return 0;
	}

	/* Bruce170322. Instead of reset 6613 in uboot. We reset here for easier maintenance. */
	gpio_direction_output(GPIO_CAT6613_RST, 0);
	msleep(10);
	//release CAT6613 from reset state
	gpio_direction_output(GPIO_CAT6613_RST, 1);

    /* I2C init */
#if 0
#ifdef	TestMode
    spin_lock_init(&cat_s_lock_i2c);
    I2CInit(HDMI_I2C_CHANNEL);
#endif
#else
    I2CInit(I2C_HDMI_TX, I2C_CAT6613_SPEED);
#endif

	//detect CAT6613
	for (i = 0; i < CAT6613_DETECT_RETRY_LIMIT; i++)
	{
	    if (Switch_HDMITX_Bank(0) == ER_SUCCESS){
	        break;	
		};
	}
	if (i == CAT6613_DETECT_RETRY_LIMIT){
		HDMITX_DEBUG_PRINTF("HDMI_TX_I2C_SLAVE_ADDR access failed\n");
		return -1;	
	}
	
	/* Check ID */
    HDMITX_ReadI2C_ByteN(0x00, pData, 4);
    VendorID = *(USHORT *)(pData) & 0xFF00;
    DeviceID = *(USHORT *)(pData + 2) & 0x0FFF;
    if ((VendorID != 0xCA00) || (DeviceID != 0x0613))
    {
        HDMITX_DEBUG_PRINTF("Can't find CAT6613 \n");
        HDMITX_DEBUG_PRINTF("Get VendorID:%x, DeviceID:%x \n", VendorID, DeviceID);
        return -1;	
    }	
	HDMITX_DEBUG_PRINTF("CAT6613 found\n");

    result = register_chrdev (ioaccess_major, "cat6613", &ioaccess_fops);
    if (result < 0) {
        HDMITX_DEBUG_PRINTF("Can't get major.\n");
        return result;
    }
    
    if (ioaccess_major == 0) {
        ioaccess_major = result;
        HDMITX_DEBUG_PRINTF("A ioaccess_major = %d\n", ioaccess_major);
    }

#if HANDLE_INT_IN_THREAD
    wq = create_singlethread_workqueue("cat6613_wq");
    if (!wq) {
        uerr("Failed to allocate wq?!\n");
        result = -ENOMEM;
        return result;
    }
    BUG_ON(wq == NULL);

	init_MUTEX(&drv_lock);
    INIT_WORK(&hotplug_task, (void (*)(void *))HDMITX_DevLoopProc, NULL);
#if PARAM_DOUBLE_COPY
    INIT_WORK(&setup_video_task, (void (*)(void *))setup_video, (void*)&ws_video);
#ifdef CONFIG_ARCH_AST1500_HOST
    INIT_WORK(&setup_loopback_video_task, (void (*)(void *))setup_loopback_video, (void*)&ws_video);
#endif
    INIT_WORK(&setup_audio_task, (void (*)(void *))setup_audio, (void*)&ws_audio);
#else //#if PARAM_DOUBLE_COPY
    INIT_WORK(&setup_video_task, (void (*)(void *))setup_video, NULL);
    INIT_WORK(&setup_audio_task, (void (*)(void *))setup_audio, NULL);
#endif //#if PARAM_DOUBLE_COPY
#if 0
    INIT_WORK(&disable_task, (void (*)(void *))disable_xmiter, NULL);
#else
	INIT_WORK(&disable_task, (void (*)(void *))DisableCAT6613, NULL);
#endif
	INIT_WORK(&disable_audio_task, (void (*)(void *))DisableAudioOutput, (void *)NULL);
#if SUPPORT_HDCP_REPEATER
	INIT_WORK(&Hdcp_start_auth_work, (void (*)(void *))hdcp1_auth_start, NULL);
	INIT_WORK(&Hdcp_stop_auth_work, (void (*)(void *))hdcp1_auth_stop, NULL);
	INIT_WORK(&Hdcp_disable_work, (void (*)(void *))disable_hdcp1, NULL);
#endif
#else
	tasklet_init(&hotplug_task, (void (*)(unsigned long))HDMITX_DevLoopProc, (unsigned long)NULL);
#endif

    /* Init */
	InitCAT6613_Instance();
    InitCAT6613() ;
#if 0
	CheckHDMITX(&HPDStatus, &HPDChangeStatus);
#endif

    /* init irq: GPIOD4 low-level trigger */
#if 0    
    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x10));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x10)) = uldata | 0x10000000;

    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x0c));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x0c)) = uldata & 0xEFFFFFFF;
     
    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x08));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x08)) = uldata | 0x10000000;
#endif

	/* request IRQ */
	retval = request_irq (20, &cat6613_interrupt, SA_SHIRQ, "cat6613", "cat6613");
	if (retval) {
		HDMITX_DEBUG_PRINTF("Unable to get IRQ");
		result = -1;
		goto free_task;
	}
	//enable interrupt after registering ISR
	ENABLE_INT();

#if 0 //Bruce101202. Loopback Tx will be handled by crt
#ifdef CONFIG_ARCH_AST1500_HOST
	//enable loopback audio Tx
	HDMITX_AudioOutput(2, 2, 24, 1, init_AUD_DB);
	//disable loopback Tx by default
	SetAVMute(1);
#endif
#endif

	cat6613_sysfs_init();

	bcat6613_exist = 1;

#if NO_INT_MODE
	INIT_WORK(&hotplug_task, (void (*)(void *))__HDMITX_DevLoopProc, NULL);

	queue_work(wq, &hotplug_task);
#endif
	xmiter_info_init();

    return 0;

free_task:

#if HANDLE_INT_IN_THREAD
    if (wq) {
        flush_workqueue(wq);
        destroy_workqueue(wq);
        wq = NULL;
    }
#else
	tasklet_disable(&hotplug_task);
	tasklet_kill(&hotplug_task);
#endif

	return result;

}
                                                                                
void my_cleanup(void)
{
    HDMITX_DEBUG_PRINTF("UnLoad CAT6613 \n");

	if (bcat6613_exist) {
		xmiter_info_fini();

	cat6613_sysfs_free();

    /* Disable interrupt */
    DISABLE_INT();

#if HANDLE_INT_IN_THREAD
    if (wq) {
        cancel_delayed_work(&hotplug_task);
        flush_workqueue(wq);
        destroy_workqueue(wq);
        wq = NULL;
    }
#else
	tasklet_disable(&hotplug_task);
	tasklet_kill(&hotplug_task);
#endif

    DisableCAT6613();
    
    unregister_chrdev (ioaccess_major, "cat6613");
    free_irq (20, "cat6613");

		bcat6613_exist = 0;
	}
    
    return;
}

#ifndef MODULE
arch_initcall(my_init);
#else
module_init (my_init);
#endif
module_exit (my_cleanup);


#elif	defined(Linux_UserMode)

#ifdef	TestMode

int   fd;

int  main (int argc, char **argv)
{

    fd = open ("/dev/i2c", O_RDONLY);       
    if (fd < 0)
    {
        printf("Can't Find the I2C Driver \n");
        exit(1);	
    }

    InitCAT6613() ;

    close (fd);

    return 1;
}

#endif

#endif	/* Linux_UserMode */

#endif
