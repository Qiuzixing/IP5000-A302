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

#ifdef CONFIG_AST1500_CAT6613
/*
 * ASPEED Options
 */
#define CAP_HDMI	0x01
#define CAP_HDCP	0x02
#define CAP_AUDIO	0x04

#include <linux/string.h>
#include <linux/interrupt.h>
#include <asm/arch/gpio.h>
#include "typedef_hdmitx.h"
#include "cat6613_sys.h"
#endif
#include "hdmitx.h"
#ifndef CONFIG_AST1500_CAT6613
#include "cat6613_sys.h"
#endif



#ifdef CONFIG_AST1500_CAT6613
void crt_hotplug_tx(int plug);
#endif
#define INPUT_SIGNAL_TYPE 0 // 24 bit sync seperate
//#define INPUT_SIGNAL_TYPE ( T_MODE_DEGEN )
//#define INPUT_SIGNAL_TYPE ( T_MODE_INDDR)
//#define INPUT_SIGNAL_TYPE ( T_MODE_SYNCEMB)
//#define INPUT_SIGNAL_TYPE ( T_MODE_CCIR656 | T_MODE_SYNCEMB )
#define I2S 0
#define SPDIF 1
#define INPUT_SAMPLE_FREQ AUDFS_48KHz
#define OUTPUT_CHANNEL 2

INSTANCE InstanceData =
{
#ifndef CONFIG_AST1500_CAT6613
    0,      // BYTE I2C_DEV ;
    0x98,    // BYTE I2C_ADDR ;
#endif
    /////////////////////////////////////////////////
    // Interrupt Type
    /////////////////////////////////////////////////
    0x40,      // BYTE bIntType ; // = 0 ;
    /////////////////////////////////////////////////
    // Video Property
    /////////////////////////////////////////////////
    INPUT_SIGNAL_TYPE ,// BYTE bInputVideoSignalType ; // for Sync Embedded,CCIR656,InputDDR

    /////////////////////////////////////////////////
    // Audio Property
    /////////////////////////////////////////////////
    I2S, // BYTE bOutputAudioMode ; // = 0 ;
    FALSE , // BYTE bAudioChannelSwap ; // = 0 ;
    0x01, // BYTE bAudioChannelEnable ;
    AUDFS_48KHz ,// BYTE bAudFs ;
    0, // unsigned long TMDSClock ;
    FALSE, // BYTE bAuthenticated:1 ;
#ifndef CONFIG_AST1500_CAT6613
    FALSE, // BYTE bHDMIMode: 1;
#endif 
    FALSE, // BYTE bIntPOL:1 ; // 0 = Low Active
#ifndef CONFIG_AST1500_CAT6613
    FALSE, // BYTE bHPD:1 ;
#endif

#ifdef CONFIG_AST1500_CAT6613
	EDIDisReady:FALSE
#endif
};

////////////////////////////////////////////////////////////////////////////////
// EDID
////////////////////////////////////////////////////////////////////////////////
static _XDATA unsigned char EDID_Buf[128] ;
static RX_CAP _XDATA RxCapability ;
#ifndef CONFIG_AST1500_CAT6613
static BOOL bChangeMode = FALSE ;
#endif
_IDATA AVI_InfoFrame AviInfo;
_IDATA Audio_InfoFrame AudioInfo ;

////////////////////////////////////////////////////////////////////////////////
// Program utility.
////////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_AST1500_CAT6613
BYTE ParseEDID(int *);
#else
BYTE ParseEDID();
static BOOL ParseCEAEDID(BYTE *pCEAEDID);
#endif
void ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep);
void ConfigAudioInfoFrm(void);
#if defined(CODE202)
void Config_GeneralPurpose_Infoframe(BYTE *p3DInfoFrame);
#endif


#ifdef CONFIG_AST1500_CAT6613
extern INSTANCE Instance[];
#endif
#ifndef CONFIG_AST1500_CAT6613
_IDATA BYTE bInputColorMode = F_MODE_RGB444;
// _IDATA BYTE bInputColorMode = F_MODE_YUV422 ;
// _IDATA BYTE bInputColorMode = F_MODE_YUV444 ;
_XDATA BYTE OutputColorDepth = 24 ;
// _IDATA BYTE bOutputColorMode = F_MODE_YUV422 ;
// _IDATA BYTE bOutputColorMode = F_MODE_YUV444 ;
_IDATA BYTE bOutputColorMode = F_MODE_RGB444 ;

_IDATA BYTE iVideoModeSelect=0 ;//steven:useless

_XDATA ULONG VideoPixelClock ;//determined by OutputVideoTiming
_XDATA BYTE VIC ;//determined by OutputVideoTiming
_XDATA BYTE pixelrep ;//determined by OutputVideoTiming
_XDATA HDMI_Aspec aspec ;//determined by OutputVideoTiming
_XDATA HDMI_Colorimetry Colorimetry ;//determined by OutputVideoTiming

_XDATA BYTE bAudioSampleFreq = INPUT_SAMPLE_FREQ ;
BOOL bHDMIMode, bAudioEnable ;
#endif

////////////////////////////////////////////////////////////////////////////////
// Function Body.
////////////////////////////////////////////////////////////////////////////////
void InitCAT6613_Instance(void);
#ifdef CONFIG_AST1500_CAT6613
#if 0
BOOL HDMITX_ChangeDisplayOption(HDMI_Video_Type VideoMode, HDMI_OutputColorMode OutputColorMode);
#endif
#else
void HDMITX_ChangeDisplayOption(HDMI_Video_Type VideoMode, HDMI_OutputColorMode OutputColorMode);
#endif
#ifndef CONFIG_AST1500_CAT6613
void HDMITX_SetOutput();
void HDMITX_DevLoopProc();
#endif

BYTE HPDStatus = FALSE, HPDChangeStatus = FALSE;
#ifdef CONFIG_AST1500_CAT6613
extern struct tasklet_struct hotplug_task;
#endif


void InitCAT6613_Instance(void)
{
    HDMITX_InitInstance(&InstanceData);
	HPDStatus = FALSE;
	HPDChangeStatus = FALSE;

}

#ifndef CONFIG_AST1500_CAT6613
void
HDMITX_SetOutput()
{
    VIDEOPCLKLEVEL level ;
#ifdef CONFIG_AST1500_CAT6613
    unsigned long TMDSClock = Instance[0].TMDSClock;
	BYTE bHDMIMode = Instance[0].outputparam.bCapibility & CAP_HDMI;
	BYTE bHDCPMode = Instance[0].outputparam.bCapibility & CAP_HDCP;
#else
	unsigned long TMDSClock = VideoPixelClock*(pixelrep+1);
#endif

	printk("HDMITX_SetOutput()\n");

#ifdef CONFIG_AST1500_CAT6613
	if(!Instance[0].outputparam.VideoSet)
	{
		printk("video parameters not set yet\n");
		return;
	}
#endif

#if defined(CODE202)
    DisableAudioOutput();
#else
    #ifdef SUPPORT_SYNCEMB
#ifdef CONFIG_AST1500_CAT6613
    ProgramSyncEmbeddedVideoMode(Instance[0].outputparam.VIC, InstanceData.bInputVideoSignalType); // inf CCIR656 input
#else
    ProgramSyncEmbeddedVideoMode(VIC, InstanceData.bInputVideoSignalType); // inf CCIR656 input
#endif 
    #endif
#endif


    if( TMDSClock>80000000 )
    {
        level = PCLK_HIGH ;
    }
    else if(TMDSClock>20000000)
    {
        level = PCLK_MEDIUM ;
    }
    else
    {
        level = PCLK_LOW ;
    }

	EnableHDCP(FALSE);
#ifdef CONFIG_AST1500_CAT6613
	SetOutputColorDepthPhase(Instance[0].outputparam.OutputColorDepth,0);
#else
#if defined(CODE202)
    if(RxCapability.dc.uc & (HDMI_DC_SUPPORT_36|HDMI_DC_SUPPORT_30))
    {
	    //SetOutputColorDepthPhase(OutputColorDepth,0);
    }
    else
    {
        OutputColorDepth = B_CD_NODEF;
    }
#endif
    SetOutputColorDepthPhase(OutputColorDepth,0);
#endif
	SetupVideoInputSignal(InstanceData.bInputVideoSignalType);

    #ifdef SUPPORT_SYNCEMB
	if(inputSignalType & T_MODE_SYNCEMB)
	{
#ifdef CONFIG_AST1500_CAT6613
		ProgramSyncEmbeddedVideoMode(Instance[0].outputparam.VIC, InstanceData.bInputVideoSignalType);
#else
		ProgramSyncEmbeddedVideoMode(VIC,InstanceData.bInputVideoSignalType);
#endif
	}
    #endif

#ifdef CONFIG_AST1500_CAT6613
	EnableVideoOutput(level, Instance[0].outputparam.bInputColorMode, Instance[0].outputparam.bOutputColorMode, bHDMIMode);
#else
	EnableVideoOutput(level,bInputColorMode,bOutputColorMode ,bHDMIMode);
#endif



    if( bHDMIMode )
    {
#ifdef CONFIG_AST1500_CAT6613
        ConfigAVIInfoFrame(Instance[0].outputparam.VIC, Instance[0].outputparam.pixelrep);
#else
        ConfigAVIInfoFrame(VIC, pixelrep);
#endif

		if (bHDCPMode)
			EnableHDCP(TRUE);
#ifdef CONFIG_AST1500_CAT6613
		if(Instance[0].outputparam.AudioSet)
#else
		if( bAudioEnable )
#endif
		{
#ifdef CONFIG_AST1500_CAT6613
#if defined(CODE202)
			EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
#else
            EnableAudioOutput(TMDSClock,
            Instance[0].outputparam.bAudioSampleFreq,
            Instance[0].outputparam.ChannelNumber,
            Instance[0].outputparam.bAudSWL,
            Instance[0].outputparam.bSPDIF);
#endif
#else
#if defined(CODE202)
#ifdef SUPPORT_HBR_AUDIO
            EnableHDMIAudio(T_AUDIO_HBR, FALSE, 768000L,OUTPUT_CHANNEL,NULL,TMDSClock);
#else
            EnableHDMIAudio(T_AUDIO_LPCM, TRUE, 48000L,OUTPUT_CHANNEL,NULL,TMDSClock);
#endif
#else
            EnableAudioOutput(TMDSClock, bAudioSampleFreq, OUTPUT_CHANNEL,24, FALSE);
#endif
#endif
            ConfigAudioInfoFrm();
#if !defined(CODE202)
#ifdef CONFIG_AST1500_CAT6613//steven:this must be done
//			SetupAudioChannel() ;
			EnableHDMIAudio(Instance[0].outputparam.Audio_Type, Instance[0].outputparam.bSPDIF, Instance[0].outputparam.bAudioSampleFreq, Instance[0].outputparam.ChannelNumber, Instance[0].outputparam.ucIEC60958ChStat, Instance[0].TMDSClock);
#endif
#endif
		}
    }
	else
	{
		EnableAVIInfoFrame(FALSE ,NULL);
	}
    SetAVMute(FALSE);
#ifndef CONFIG_AST1500_CAT6613
	bChangeMode = FALSE ;
#endif
}
#endif

#define DD 0
#if DD
void EDID_test(void)
{
	static unsigned int tt = 0;
	int sinkchanged = 0, i = 0;
	
	if ((++tt) % 5) {
		return;
	}
	printk("++++++ Start EDID Test +++++++\n");
	while (++i) {
	//for (i = 0; i < 100; i++) {
		printk("Run %d Start\n", i);
		if(!ParseEDID(&sinkchanged)) {
			printk("!!!!! ParseEDID failed!!\n");
			BUG();
		}
		if (sinkchanged)
		{
			printk("new sink EDID:\n");
			for (i = 0; i < 256; i++)
			{
				printk("%02X", Instance[0].SinkEDID[i]);
				if ((i % 16) == 15)
					printk("\n");
				else
					printk(" ");
			}
	//				memset(&Instance[0].outputparam, 0, sizeof(Output_Param));
		}
		else
		{
			printk("EDID not changed\n");
		}
	}
	printk("++++++ EDID Test Completed +++++++\n");

}
#endif

#ifdef CONFIG_AST1500_CAT6613
void
HDMITX_DevLoopProc(void)
#else
void
HDMITX_DevLoopProc()
#endif
{
    CheckHDMITX(&HPDStatus,&HPDChangeStatus);

    if( HPDChangeStatus )
    {
        if( HPDStatus )
        {
#ifdef CONFIG_AST1500_CAT6613
			int i, sinkchanged = 0;
			printk("sink plugged\n");
		#if DD
			EDID_test();
		#endif
			ParseEDID(&sinkchanged);
			if (sinkchanged)
			{
				printk("new sink EDID:\n");
				for (i = 0; i < 256; i++)
				{
					printk("%02X", Instance[0].SinkEDID[i]);
					if ((i % 16) == 15)
						printk("\n");
					else
						printk(" ");
				}
//				memset(&Instance[0].outputparam, 0, sizeof(Output_Param));
			}
			else
			{
				printk("EDID not changed\n");
			}
			//HDMITX_SetOutput(); Bruce100720. I think crt driver will handle this.
			//HDMITX_SetOutput();//Added back by Steven. CRT and upper layers will then only need to set up Tx when any setting is changed.
			Instance[0].EDIDisReady = TRUE;
			crt_hotplug_tx(1);
#else
            ParseEDID();
            bOutputColorMode = F_MODE_RGB444;

			if( RxCapability.ValidHDMI )
			{
				bHDMIMode = TRUE ;

				if(RxCapability.VideoMode & (1<<6))
				{
					bAudioEnable = TRUE ;
				}

				if( RxCapability.VideoMode & (1<<5))
				{
					bOutputColorMode &= ~F_MODE_CLRMOD_MASK ;
					bOutputColorMode |= F_MODE_YUV444;
				}
				else if (RxCapability.VideoMode & (1<<4))
				{
					bOutputColorMode &= ~F_MODE_CLRMOD_MASK ;
					bOutputColorMode |= F_MODE_YUV422 ;
				}
			}
			else
			{
				bHDMIMode = FALSE ;
				bAudioEnable = FALSE ;
#if defined(CODE202)
                DisableAudioOutput();
                DisableVideoOutput();
#ifdef SUPPORT_HDCP
                EnableHDCP(FALSE);
#endif
#endif
			}
#ifdef Printf
    		HDMITX_DEBUG_PRINTF(("HPD change HDMITX_SetOutput();\n"));
#endif
            HDMITX_SetOutput();
#endif

        }
        else
        {
			Instance[0].EDIDisReady = FALSE;
            // unplug mode, ...
			printk("sink unplugged\n");
#ifdef Printf
    		HDMITX_DEBUG_PRINTF(("HPD OFF DisableVideoOutput()\n"));
#endif
#ifdef CONFIG_AST1500_CAT6613
			crt_hotplug_tx(0);
			DisableAudioOutput();
			/* clear Vieoset and transmitter will get re-configure after plug */
			Instance[0].outputparam.VideoSet = 0;
#endif
            DisableVideoOutput();

        }
    }
#ifndef CONFIG_AST1500_CAT6613
    else // no stable but need to process mode change procedure
    {
        if(bChangeMode && HPDStatus)
        {
#ifdef Printf
    		HDMITX_DEBUG_PRINTF(("Mode change HDMITX_SetOutput();\n"));
#endif
            HDMITX_SetOutput();
        }
    }
#endif
#ifdef CONFIG_AST1500_CAT6613
	//enable interrupt
	ENABLE_INT();
#endif
}


#ifdef CONFIG_AST1500_CAT6613
#if 0
BOOL
HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_OutputColorMode OutputColorMode)
{
	switch(OutputVideoTiming)
	{
	case HDMI_640x480p60:
		Instance[0].outputparam.VIC = 1 ;
		Instance[0].outputparam.VideoPixelClock = 25000000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_4x3 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480p60:
		Instance[0].outputparam.VIC = 2 ;
		Instance[0].outputparam.VideoPixelClock = 27000000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_4x3 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480p60_16x9:
		Instance[0].outputparam.VIC = 3 ;
		Instance[0].outputparam.VideoPixelClock = 27000000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_720p60:
		Instance[0].outputparam.VIC = 4 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080i60:
		Instance[0].outputparam.VIC = 5 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_480i60:
		Instance[0].outputparam.VIC = 6 ;
		Instance[0].outputparam.VideoPixelClock = 13500000 ;
		Instance[0].outputparam.pixelrep = 1 ;
		Instance[0].outputparam.aspec = HDMI_4x3 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_480i60_16x9:
		Instance[0].outputparam.VIC = 7 ;
		Instance[0].outputparam.VideoPixelClock = 13500000 ;
		Instance[0].outputparam.pixelrep = 1 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_1080p60:
		Instance[0].outputparam.VIC = 16 ;
		Instance[0].outputparam.VideoPixelClock = 148500000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_576p50:
		Instance[0].outputparam.VIC = 17 ;
		Instance[0].outputparam.VideoPixelClock = 27000000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_4x3 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_576p50_16x9:
		Instance[0].outputparam.VIC = 18 ;
		Instance[0].outputparam.VideoPixelClock = 27000000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_720p50:
		Instance[0].outputparam.VIC = 19 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080i50:
		Instance[0].outputparam.VIC = 20 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_576i50:
		Instance[0].outputparam.VIC = 21 ;
		Instance[0].outputparam.VideoPixelClock = 13500000 ;
		Instance[0].outputparam.pixelrep = 1 ;
		Instance[0].outputparam.aspec = HDMI_4x3 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_576i50_16x9:
		Instance[0].outputparam.VIC = 22 ;
		Instance[0].outputparam.VideoPixelClock = 13500000 ;
		Instance[0].outputparam.pixelrep = 1 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;
		break ;
	case HDMI_1080p50:
		Instance[0].outputparam.VIC = 31 ;
		Instance[0].outputparam.VideoPixelClock = 148500000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p24:
		Instance[0].outputparam.VIC = 32 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p25:
		Instance[0].outputparam.VIC = 33 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	case HDMI_1080p30:
		Instance[0].outputparam.VIC = 34 ;
		Instance[0].outputparam.VideoPixelClock = 74250000 ;
		Instance[0].outputparam.pixelrep = 0 ;
		Instance[0].outputparam.aspec = HDMI_16x9 ;
		Instance[0].outputparam.Colorimetry = HDMI_ITU709 ;
		break ;
	default:
		Instance[0].outputparam.Colorimetry = HDMI_ITU601 ;    
		return FALSE;
	}

	switch(OutputColorMode)
	{
	case HDMI_YUV444:
		Instance[0].outputparam.bOutputColorMode = F_MODE_YUV444 ;
		break ;
	case HDMI_YUV422:
		Instance[0].outputparam.bOutputColorMode = F_MODE_YUV422 ;
		break ;
	case HDMI_RGB444:
	default:
		Instance[0].outputparam.bOutputColorMode = F_MODE_RGB444 ;
		break ;
	}

	if( Instance[0].outputparam.Colorimetry == HDMI_ITU709 )
	{
		Instance[0].outputparam.bInputColorMode |= F_MODE_ITU709 ;
	}
	else
	{
		Instance[0].outputparam.bInputColorMode &= ~F_MODE_ITU709 ;
	}

	if( Instance[0].outputparam.Colorimetry != HDMI_640x480p60)
	{
		Instance[0].outputparam.bInputColorMode |= F_MODE_16_235 ;
	}
	else
	{
		Instance[0].outputparam.bInputColorMode &= ~F_MODE_16_235 ;
	}

	return TRUE ;
}
#endif
#else
void
HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming, HDMI_OutputColorMode OutputColorMode)
{
   //HDMI_Video_Type  t=HDMI_480i60_16x9;
#if defined(CODE202)
   if((F_MODE_RGB444)==(bOutputColorMode&F_MODE_CLRMOD_MASK))//Force output RGB in RGB only case
   {
	   OutputColorMode=F_MODE_RGB444;
   }
   else if ((F_MODE_YUV422)==(bOutputColorMode&F_MODE_CLRMOD_MASK))//YUV422 only
   {
	   if(OutputColorMode==HDMI_YUV444){OutputColorMode=F_MODE_YUV422;}
   }
   else if ((F_MODE_YUV444)==(bOutputColorMode&F_MODE_CLRMOD_MASK))//YUV444 only
   {
	   if(OutputColorMode==HDMI_YUV422){OutputColorMode=F_MODE_YUV444;}
   }
#endif
    switch(OutputVideoTiming)
	{
    case HDMI_640x480p60:
        VIC = 1 ;
        VideoPixelClock = 25000000 ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60:
        VIC = 2 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480p60_16x9:
        VIC = 3 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p60:
        VIC = 4 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080i60:
        VIC = 5 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_480i60:
        VIC = 6 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_480i60_16x9:
        VIC = 7 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p60:
        VIC = 16 ;
        VideoPixelClock = 148500000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576p50:
        VIC = 17 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576p50_16x9:
        VIC = 18 ;
        VideoPixelClock = 27000000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_720p50:
        VIC = 19 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080i50:
        VIC = 20 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_576i50:
        VIC = 21 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_576i50_16x9:
        VIC = 22 ;
        VideoPixelClock = 13500000 ;
        pixelrep = 1 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU601 ;
        break ;
    case HDMI_1080p50:
        VIC = 31 ;
        VideoPixelClock = 148500000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p24:
        VIC = 32 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p25:
        VIC = 33 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    case HDMI_1080p30:
        VIC = 34 ;
        VideoPixelClock = 74250000 ;
        pixelrep = 0 ;
        aspec = HDMI_16x9 ;
        Colorimetry = HDMI_ITU709 ;
        break ;
    default:
        bChangeMode = FALSE ;
        return ;
    }

    switch(OutputColorMode)
    {
    case HDMI_YUV444:
        bOutputColorMode = F_MODE_YUV444 ;
        break ;
    case HDMI_YUV422:
        bOutputColorMode = F_MODE_YUV422 ;
        break ;
    case HDMI_RGB444:
    default:
        bOutputColorMode = F_MODE_RGB444 ;
        break ;
    }

    if( Colorimetry == HDMI_ITU709 )
    {
        bInputColorMode |= F_MODE_ITU709 ;
    }
    else
    {
        bInputColorMode &= ~F_MODE_ITU709 ;
    }

    if( Colorimetry != HDMI_640x480p60)
    {
        bInputColorMode |= F_MODE_16_235 ;
    }
    else
    {
        bInputColorMode &= ~F_MODE_16_235 ;
    }

    bChangeMode = TRUE ;
}
#endif


#ifndef CONFIG_AST1500_CAT6613
void
ConfigAVIInfoFrame(BYTE VIC, BYTE pixelrep)
{
//     AVI_InfoFrame AviInfo;

    AviInfo.pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80 ;
    AviInfo.pktbyte.AVI_HB[1] = AVI_INFOFRAME_VER ;
    AviInfo.pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN ;

#ifdef CONFIG_AST1500_CAT6613
	switch(Instance[0].outputparam.bOutputColorMode)
#else
    switch(bOutputColorMode)
#endif
    {
    case F_MODE_YUV444:
        // AviInfo.info.ColorMode = 2 ;
        AviInfo.pktbyte.AVI_DB[0] = (2<<5)|(1<<4);
        break ;
    case F_MODE_YUV422:
        // AviInfo.info.ColorMode = 1 ;
        AviInfo.pktbyte.AVI_DB[0] = (1<<5)|(1<<4);
        break ;
    case F_MODE_RGB444:
    default:
        // AviInfo.info.ColorMode = 0 ;
        AviInfo.pktbyte.AVI_DB[0] = (0<<5)|(1<<4);
        break ;
    }
    AviInfo.pktbyte.AVI_DB[1] = 8 ;
#ifdef CONFIG_AST1500_CAT6613
    AviInfo.pktbyte.AVI_DB[1] |= (Instance[0].outputparam.aspec != HDMI_16x9)?(1<<4):(2<<4); // 4:3 or 16:9
    AviInfo.pktbyte.AVI_DB[1] |= (Instance[0].outputparam.Colorimetry != HDMI_ITU709)?(1<<6):(2<<6); // 4:3 or 16:9
#else
    AviInfo.pktbyte.AVI_DB[1] |= (aspec != HDMI_16x9)?(1<<4):(2<<4); // 4:3 or 16:9
    AviInfo.pktbyte.AVI_DB[1] |= (Colorimetry != HDMI_ITU709)?(1<<6):(2<<6); // 4:3 or 16:9
#endif 
    AviInfo.pktbyte.AVI_DB[2] = 0 ;
    AviInfo.pktbyte.AVI_DB[3] = VIC ;
    AviInfo.pktbyte.AVI_DB[4] =  pixelrep & 3 ;
    AviInfo.pktbyte.AVI_DB[5] = 0 ;
    AviInfo.pktbyte.AVI_DB[6] = 0 ;
    AviInfo.pktbyte.AVI_DB[7] = 0 ;
    AviInfo.pktbyte.AVI_DB[8] = 0 ;
    AviInfo.pktbyte.AVI_DB[9] = 0 ;
    AviInfo.pktbyte.AVI_DB[10] = 0 ;
    AviInfo.pktbyte.AVI_DB[11] = 0 ;
    AviInfo.pktbyte.AVI_DB[12] = 0 ;

    EnableAVIInfoFrame(TRUE, (unsigned char *)&AviInfo);
}
#endif


////////////////////////////////////////////////////////////////////////////////
// Function: ConfigAudioInfoFrm
// Parameter: NumChannel, number from 1 to 8
// Return: ER_SUCCESS for successfull.
// Remark: Evaluate. The speakerplacement is only for reference.
//         For production, the caller of SetAudioInfoFrame should program
//         Speaker placement by actual status.
// Side-Effect:
////////////////////////////////////////////////////////////////////////////////

void
ConfigAudioInfoFrm(void)
{
    int i ;
	
	// refrence cat6023 0xde,0xe1	  
	AudioInfo.pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE ;
	AudioInfo.pktbyte.AUD_HB[1] = 1 ;
	AudioInfo.pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN ;
#if 0
	AudioInfo.pktbyte.AUD_DB[0] = Instance[0].outputparam.AUD_DB[0]&0x7/*7*//*1*/ ;
#else
	AudioInfo.pktbyte.AUD_DB[0] = Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[0]&0x7/*7*//*1*/ ;
#endif
	for( i = 1 ;i < AUDIO_INFOFRAME_LEN ; i++ )
	{
		AudioInfo.pktbyte.AUD_DB[i] = 0 ;
	}
#if 0
	AudioInfo.pktbyte.AUD_DB[3] = Instance[0].outputparam.AUD_DB[3]/*0x1f*/;
	AudioInfo.pktbyte.AUD_DB[4] = Instance[0].outputparam.AUD_DB[4];
#else
	AudioInfo.pktbyte.AUD_DB[3] = Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[3]/*0x1f*/;
	AudioInfo.pktbyte.AUD_DB[4] = Instance[0].outputparam.audio_infoframe.pktbyte.AUD_DB[4];
#endif
	
	EnableAudioInfoFrame(TRUE, (unsigned char *)&AudioInfo) ;

/*
    int i ;
    HDMITX_DEBUG_PRINTF("ConfigAudioInfoFrm(%d)\n",2);

    AudioInfo.pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE ;
    AudioInfo.pktbyte.AUD_HB[1] = 1 ;
    AudioInfo.pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN ;
    AudioInfo.pktbyte.AUD_DB[0] = 1 ;
    for( i = 1 ;i < AUDIO_INFOFRAME_LEN ; i++ )
    {
        AudioInfo.pktbyte.AUD_DB[i] = 0 ;
    }
    EnableAudioInfoFrame(TRUE, (unsigned char *)&AudioInfo);
*/
}


#define CAT_EDID_HDMI_IEEE_OUI 0x000c03
#define CAT_EDID_HDMI_IEEE_OUI_FORUM 0xC45DD8 /* HDMI Forum VSDB, HF-VSDB, defined in 2.0 spec. */

#ifdef CONFIG_AST1500_CAT6613
unsigned char edid_block_checksum(unsigned char *data)
{
	int i;
	unsigned char checksum = 0;

	for (i = 0; i < 128; i++) {
		checksum += data[i];
		checksum &= 0xFF;
	}

	return checksum;
}
#endif

/////////////////////////////////////////////////////////////////////
// ParseEDID()
// Check EDID check sum and EDID 1.3 extended segment.
/////////////////////////////////////////////////////////////////////
#ifdef CONFIG_AST1500_CAT6613
BYTE ParseEDID(int *psinkchanged)
#else
BYTE ParseEDID(void)
#endif
{
	// collect the EDID ucdata of segment 0
#ifndef CONFIG_AST1500_CAT6613
	BYTE CheckSum;
	BYTE bValidCEA = FALSE;
#endif
	BYTE BlockCount;
	BYTE err = FALSE;
	int i;

#ifdef CONFIG_AST1500_CAT6613
	EnableHDCP(FALSE);
#endif
	RxCapability.ValidCEA = FALSE;
	RxCapability.ValidHDMI = FALSE;
#if defined(CODE202)
	RxCapability.dc.uc = 0;
#endif

#ifdef CONFIG_AST1500_CAT6613
	if (!GetEDIDData(0, EDID_Buf)) {
		HDMITX_DEBUG_PRINTF("Read EDID block 0 failed\n");
		//return FALSE ;
		goto fail_and_clear_edid;
	}
#else

	GetEDIDData(0, EDID_Buf);


	for (i = 0, CheckSum = 0; i < 128; i++) {

		CheckSum += EDID_Buf[i];
		CheckSum &= 0xFF;
	}

	if (CheckSum != 0)
		return FALSE;
#endif

	if (EDID_Buf[0] != 0x00 ||
		EDID_Buf[1] != 0xFF ||
		EDID_Buf[2] != 0xFF ||
		EDID_Buf[3] != 0xFF ||
		EDID_Buf[4] != 0xFF ||
		EDID_Buf[5] != 0xFF ||
		EDID_Buf[6] != 0xFF ||
		EDID_Buf[7] != 0x00) {
		HDMITX_DEBUG_PRINTF("Invalid EDID header\n");
		goto fail_and_clear_edid;
	}

#ifdef CONFIG_AST1500_CAT6613
	if (memcmp(EDID_Buf, Instance[0].SinkEDID, 128)) {
		*psinkchanged = 1;
		memcpy(Instance[0].SinkEDID, EDID_Buf, 128);
	}
	Instance[0].HDMISink = 0;
#endif
	BlockCount = EDID_Buf[0x7E];

	if (BlockCount == 0)
		return TRUE ; // do nothing.
	else if (BlockCount > 4)
		BlockCount = 4;

	// read all segment for test
	for (i = 1; i <= BlockCount; i++) {
		err = GetEDIDData(i, EDID_Buf);

		if (err) {
#ifdef CONFIG_AST1500_CAT6613
			/*
			 * we suppose 1st extension block exists and be valid => support HDMI
			 * do not care about VSDB
			 */
			if ((i == 1) && (edid_block_checksum(EDID_Buf) == 0)) {
				if (memcmp(EDID_Buf, Instance[0].SinkEDID + 128, 128)) {
					*psinkchanged = 1;
					memcpy(Instance[0].SinkEDID + 128, EDID_Buf, 128);
				}
				Instance[0].HDMISink = 1;
			}
#else
			if (!bValidCEA && EDID_Buf[0] == 0x2 && EDID_Buf[1] == 0x3) {
				err = ParseCEAEDID(EDID_Buf);
				if (err) {
					bValidCEA = TRUE; /* bValidCEA should get set no matter whether HDMI is */
					/*
					 * HDMI VSDB and HF-VSDB have the same tag code (0x3)
					 *
					 * ParseCEAEDID() will overwrite RxCapability.IEEEOUI by last data block with tag code 0x3
					 * if there is VSDB and HF-VSDB in a single block
					 */
					if ((RxCapability.IEEEOUI == CAT_EDID_HDMI_IEEE_OUI_FORUM)
						|| (RxCapability.IEEEOUI == CAT_EDID_HDMI_IEEE_OUI_FORUM)) {
						RxCapability.ValidHDMI = TRUE;
					} else {
						RxCapability.ValidHDMI = FALSE;
					}
				}
			}
#endif
		}
	}

	return err;

fail_and_clear_edid:
	memset(Instance[0].SinkEDID, 0, 256);
	return FALSE;
}

#ifndef CONFIG_AST1500_CAT6613
static BOOL
ParseCEAEDID(BYTE *pCEAEDID)
{
    BYTE offset,End ;
    BYTE count ;
    BYTE tag ;
    int i ;

    if( pCEAEDID[0] != 0x02 || pCEAEDID[1] != 0x03 ) return ER_SUCCESS ; // not a CEA BLOCK.
    End = pCEAEDID[2]  ; // CEA description.
    RxCapability.VideoMode = pCEAEDID[3] ;

	RxCapability.VDOModeCount = 0 ;
    RxCapability.idxNativeVDOMode = 0xff ;

    for( offset = 4 ; offset < End ; )
    {
        tag = pCEAEDID[offset] >> 5 ;
        count = pCEAEDID[offset] & 0x1f ;
        switch( tag )
        {
        case 0x01: // Audio Data Block ;
            RxCapability.AUDDesCount = count/3 ;
            offset++ ;
            for( i = 0 ; i < RxCapability.AUDDesCount ; i++ )
            {
                RxCapability.AUDDes[i].uc[0] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[1] = pCEAEDID[offset++] ;
                RxCapability.AUDDes[i].uc[2] = pCEAEDID[offset++] ;
            }

            break ;

        case 0x02: // Video Data Block ;
            //RxCapability.VDOModeCount = 0 ;
            offset ++ ;
            for( i = 0,RxCapability.idxNativeVDOMode = 0xff ; i < count ; i++, offset++ )
            {
            	BYTE VIC ;
            	VIC = pCEAEDID[offset] & (~0x80);
            	// if( FindModeTableEntryByVIC(VIC) != -1 )
            	{
	                RxCapability.VDOMode[RxCapability.VDOModeCount] = VIC ;
	                if( pCEAEDID[offset] & 0x80 )
	                {
	                    RxCapability.idxNativeVDOMode = (BYTE)RxCapability.VDOModeCount ;
#ifndef CONFIG_AST1500_CAT6613
	                    iVideoModeSelect = RxCapability.VDOModeCount ;
#endif
	                }

	                RxCapability.VDOModeCount++ ;
            	}
            }
            break ;

        case 0x03: // Vendor Specific Data Block ;
            offset ++ ;
            RxCapability.IEEEOUI = (ULONG)pCEAEDID[offset+2] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset+1] ;
            RxCapability.IEEEOUI <<= 8 ;
            RxCapability.IEEEOUI += (ULONG)pCEAEDID[offset] ;
#if defined(CODE202)
            if(count>5)
            {
                RxCapability.dc.uc = pCEAEDID[offset+5]&0x70;
            }
#endif
           offset += count ; // ignore the remaind.

            break ;

        case 0x04: // Speaker Data Block ;
            offset ++ ;
            RxCapability.SpeakerAllocBlk.uc[0] = pCEAEDID[offset] ;
            RxCapability.SpeakerAllocBlk.uc[1] = pCEAEDID[offset+1] ;
            RxCapability.SpeakerAllocBlk.uc[2] = pCEAEDID[offset+2] ;
            offset += 3 ;
            break ;
        case 0x05: // VESA Data Block ;
            offset += count+1 ;
            break ;
        case 0x07: // Extended Data Block ;
            offset += count+1 ; //ignore
            break ;
        default:
            offset += count+1 ; // ignore
        }
    }
    RxCapability.ValidCEA = TRUE ;
    return TRUE ;
}
#endif

#if defined(CODE202)
void Config_GeneralPurpose_Infoframe(BYTE *p3DInfoFrame)
{
    // please fill HDMI 1.4 Infoframe here.
    EnableVendorSpecificInfoFrame(TRUE, p3DInfoFrame) ;

}
#endif

#endif//CONFIG_AST1500_CAT6613
