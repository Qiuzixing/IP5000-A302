#include "ast_def.h"
#ifdef CONFIG_AST1500_CAT6613
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >cat6613_drv.c<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/

/////////////////////////////////////////////////////////////////////
// CAT6613.C
// Driver code for platform independent
/////////////////////////////////////////////////////////////////////
#ifdef CONFIG_AST1500_CAT6613
#include "typedef_hdmitx.h"
#include "cat6613_sys.h"
#endif
#include "hdmitx.h"
#ifdef CONFIG_AST1500_CAT6613
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#if SUPPORT_HDCP_REPEATER
#include <aspeed/hdcp1.h>
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif

//Bruce110401. The whole 6613 driver is a crap. I do this for this crapping reason.
#define CAP_HDMI	0x01
#define CAP_HDCP	0x02
#define CAP_AUDIO	0x04

#endif
#define FALLING_EDGE_TRIGGER
//#define USE_SPDIF_CHSTAT
#if defined(CONFIG_AST1520_BOARD_FPGA_ARM9) && (CONFIG_AST1500_SOC_DRAM_MAPPING_TYPE == 2)
#undef FALLING_EDGE_TRIGGER
#endif

#define MSCOUNT 1000
#define LOADING_UPDATE_TIMEOUT (3000/32)    // 3sec
// USHORT u8msTimer = 0 ;
// USHORT TimerServF = TRUE ;


//////////////////////////////////////////////////////////////////////
// Authentication status
//////////////////////////////////////////////////////////////////////

// #define TIMEOUT_WAIT_AUTH MS(2000)

#define Switch_HDMITX_Bank(x)   HDMITX_WriteI2C_Byte(0x0f,(x)&1)

#define HDMITX_OrREG_Byte(reg,ormask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) | (ormask)))
#define HDMITX_AndREG_Byte(reg,andmask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) & (andmask)))
#define HDMITX_SetREG_Byte(reg,andmask,ormask) HDMITX_WriteI2C_Byte(reg,((HDMITX_ReadI2C_Byte(reg) & (andmask))|(ormask)))



#ifdef CONFIG_AST1500_CAT6613
INSTANCE Instance[1] =
{
	{
		0, /* bIntType */
		0,/* | T_MODE_CCIR656 | T_MODE_SYNCEMB | T_MODE_INDDR */ // bInputVideoSignalType
		0x01,// bOutputAudioMode
		0,// bAudioChannelSwap
                0, // BYTE bAudioChannelEnable ;
                0, //BYTE bAudFs ;
                0, // unsigned long TMDSClock ;
		FALSE,//bAuthenticated
//		FALSE,// bHDMIMode
		FALSE,// bIntPOL
//		FALSE,// bHPD
//		FALSE // bMuteRequested
	},
} ;
#else
#ifdef _MCU_8051_
INSTANCE Instance[HDMITX_INSTANCE_MAX] _at_ 0x0000; // for 8051 , avoid a variable set in offset 0 to make a NULL pointer.
#else
INSTANCE Instance[HDMITX_INSTANCE_MAX] ;
#endif
#if defined(CODE202)
BOOL bForceCTS = FALSE;
#endif
#endif
#ifdef CONFIG_AST1500_CAT6613
extern struct tasklet_struct hotplug_task;
#endif

//////////////////////////////////////////////////////////////////////
// Function Prototype
//////////////////////////////////////////////////////////////////////

// static BOOL IsRxSense();

static void WaitTxVidStable(void);
static void SetInputMode(BYTE InputMode,BYTE bInputSignalType);
static void SetCSCScale(BYTE bInputMode,BYTE bOutputMode);
// static void SetupAFE(BYTE ucFreqInMHz);
static void SetupAFE(VIDEOPCLKLEVEL PCLKLevel);
static void FireAFE(void);


#if !defined(CODE202)
static SYS_STATUS SetAudioFormat(BYTE NumChannel,BYTE AudioEnable,BYTE bSampleFreq,BYTE AudSWL,BYTE AudioCatCode);
#endif
static SYS_STATUS SetNCTS(ULONG PCLK,BYTE Fs);

static void AutoAdjustAudio(void);
#ifdef CONFIG_AST1500_CAT6613
void SetupAudioChannel(void);
#else
static void SetupAudioChannel();
#endif

static SYS_STATUS SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame);
static SYS_STATUS SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame);
static SYS_STATUS SetSPDInfoFrame(SPD_InfoFrame *pSPDInfoFrame);
static SYS_STATUS SetMPEGInfoFrame(MPEG_InfoFrame *pMPGInfoFrame);
SYS_STATUS ReadEDID(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count);
static void AbortDDC(void);
static void ClearDDCFIFO(void);
static void ClearDDCFIFO(void);
static void GenerateDDCSCLK(void);
#ifdef SUPPORT_HDCP
#ifdef CONFIG_AST1500_CAT6613
SYS_STATUS HDCP_EnableEncryption(void);
#else
static SYS_STATUS HDCP_EnableEncryption(void);
#endif
static void HDCP_ResetAuth(void);
static void HDCP_Auth_Fire(void);
static void HDCP_StartAnCipher(void);
static void HDCP_StopAnCipher(void);
static void HDCP_GenerateAn(void);
static SYS_STATUS HDCP_GetVr(BYTE *pVr);
static SYS_STATUS HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus);
static SYS_STATUS HDCP_GetBKSV(BYTE *pBKSV);
static SYS_STATUS HDCP_Authenticate(void);
static SYS_STATUS HDCP_Authenticate_Repeater(void);
static SYS_STATUS HDCP_VerifyIntegration(void);
static SYS_STATUS HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream);
static SYS_STATUS HDCP_CheckSHA(BYTE M0[],USHORT BStatus,BYTE KSVList[],int devno,BYTE Vr[]);
void HDCP_ResumeAuthentication(void);
void HDCP_Reset(void);
#endif



static void ENABLE_NULL_PKT(void);
static void ENABLE_ACP_PKT(void);
static void ENABLE_ISRC1_PKT(void);
static void ENABLE_ISRC2_PKT(void);
static void ENABLE_AVI_INFOFRM_PKT(void);
static void ENABLE_AUD_INFOFRM_PKT(void);
static void ENABLE_SPD_INFOFRM_PKT(void);
static void ENABLE_MPG_INFOFRM_PKT(void);
#if defined(CODE202)
static void ENABLE_GeneralPurpose_PKT(void);
#endif

static void DISABLE_NULL_PKT(void);
static void DISABLE_ACP_PKT(void);
static void DISABLE_ISRC1_PKT(void);
static void DISABLE_ISRC2_PKT(void);
static void DISABLE_AVI_INFOFRM_PKT(void);
static void DISABLE_AUD_INFOFRM_PKT(void);
static void DISABLE_SPD_INFOFRM_PKT(void);
static void DISABLE_MPG_INFOFRM_PKT(void);
#if defined(CODE202)
static void DISABLE_GeneralPurpose_PKT(void);
#endif
static BYTE countbit(BYTE b);

void DumpCatHDMITXReg(void);





//////////////////////////////////////////////////////////////////////
// Function Body.
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// utility function for main..
//////////////////////////////////////////////////////////////////////


#ifndef DISABLE_TX_CSC
#if defined(CODE202)
#ifdef CONFIG_AST1500_CAT6613
BYTE _CODE bCSCOffset_16_235[] =
{
    0x00,0x80,0x00
};

BYTE _CODE bCSCOffset_0_255[] =
{
    0x10,0x80,0x10
};


#if (defined (SUPPORT_OUTPUTYUV)) && (defined (SUPPORT_INPUTRGB))
    BYTE _CODE bCSCMtx_RGB2YUV_ITU601_16_235[] =
    {
        0xB2,0x04,0x64,0x02,0xE9,0x00,
        0x93,0x3C,0x18,0x04,0x56,0x3F,
        0x49,0x3D,0x9F,0x3E,0x18,0x04
    } ;

    BYTE _CODE bCSCMtx_RGB2YUV_ITU601_0_255[] =
    {
        0x09,0x04,0x0E,0x02,0xC8,0x00,
        0x0E,0x3D,0x84,0x03,0x6E,0x3F,
        0xAC,0x3D,0xD0,0x3E,0x84,0x03
    } ;

    BYTE _CODE bCSCMtx_RGB2YUV_ITU709_16_235[] =
    {
        0xB8,0x05,0xB4,0x01,0x93,0x00,
        0x49,0x3C,0x18,0x04,0x9F,0x3F,
        0xD9,0x3C,0x10,0x3F,0x18,0x04
    } ;

    BYTE _CODE bCSCMtx_RGB2YUV_ITU709_0_255[] =
    {
        0xE5,0x04,0x78,0x01,0x81,0x00,
        0xCE,0x3C,0x84,0x03,0xAE,0x3F,
        0x49,0x3D,0x33,0x3F,0x84,0x03
    } ;
#endif

#if (defined (SUPPORT_OUTPUTRGB)) && (defined (SUPPORT_INPUTYUV))
    BYTE _CODE bCSCMtx_YUV2RGB_ITU601_16_235[] =
    {
        0x00,0x08,0x6A,0x3A,0x4F,0x3D,
        0x00,0x08,0xF7,0x0A,0x00,0x00,
        0x00,0x08,0x00,0x00,0xDB,0x0D
    } ;

    BYTE _CODE bCSCMtx_YUV2RGB_ITU601_0_255[] =
    {
        0x4F,0x09,0x81,0x39,0xDF,0x3C,
        0x4F,0x09,0xC2,0x0C,0x00,0x00,
        0x4F,0x09,0x00,0x00,0x1E,0x10
    } ;

    BYTE _CODE bCSCMtx_YUV2RGB_ITU709_16_235[] =
    {
        0x00,0x08,0x53,0x3C,0x89,0x3E,
        0x00,0x08,0x51,0x0C,0x00,0x00,
        0x00,0x08,0x00,0x00,0x87,0x0E
    } ;

    BYTE _CODE bCSCMtx_YUV2RGB_ITU709_0_255[] =
    {
        0x4F,0x09,0xBA,0x3B,0x4B,0x3E,
        0x4F,0x09,0x56,0x0E,0x00,0x00,
        0x4F,0x09,0x00,0x00,0xE7,0x10
    } ;
#endif
#else
    extern _CODE BYTE bCSCOffset_16_235[] ;
    extern _CODE BYTE bCSCOffset_0_255[] ;
    #if (defined (SUPPORT_OUTPUTYUV)) && (defined (SUPPORT_INPUTRGB))

        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] ;
    #endif

    #if (defined (SUPPORT_OUTPUTRGB)) && (defined (SUPPORT_INPUTYUV))

        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] ;

    #endif
#endif
#else
    // int aiNonCEAVIC[] = { 2 } ;
    #ifndef EXTERN_CSC_TABLE
        // Y,C,RGB offset
        // for register 73~75
        _CODE BYTE bCSCOffset_16_235[] =
        {
            0x00,0x80,0x00
        };

        _CODE BYTE bCSCOffset_0_255[] =
        {
            0x10,0x80,0x10
        };


        #ifdef SUPPORT_INPUTRGB
            _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] =
            {
                0xB2,0x04,0x64,0x02,0xE9,0x00,
                0x93,0x3C,0x18,0x04,0x56,0x3F,
                0x49,0x3D,0x9F,0x3E,0x18,0x04
            } ;

            _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] =
            {
                0x09,0x04,0x0E,0x02,0xC8,0x00,
                0x0E,0x3D,0x84,0x03,0x6E,0x3F,
                0xAC,0x3D,0xD0,0x3E,0x84,0x03
            } ;

            _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] =
            {
                0xB8,0x05,0xB4,0x01,0x93,0x00,
                0x49,0x3C,0x18,0x04,0x9F,0x3F,
                0xD9,0x3C,0x10,0x3F,0x18,0x04
            } ;

            _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] =
            {
                0xE5,0x04,0x78,0x01,0x81,0x00,
                0xCE,0x3C,0x84,0x03,0xAE,0x3F,
                0x49,0x3D,0x33,0x3F,0x84,0x03
            } ;
        #endif

        #ifdef SUPPORT_INPUTYUV

            _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] =
            {
                0x00,0x08,0x6A,0x3A,0x4F,0x3D,
                0x00,0x08,0xF7,0x0A,0x00,0x00,
                0x00,0x08,0x00,0x00,0xDB,0x0D
            } ;

            _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] =
            {
                0x4F,0x09,0x81,0x39,0xDF,0x3C,
                0x4F,0x09,0xC2,0x0C,0x00,0x00,
                0x4F,0x09,0x00,0x00,0x1E,0x10
            } ;

            _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] =
            {
                0x00,0x08,0x53,0x3C,0x89,0x3E,
                0x00,0x08,0x51,0x0C,0x00,0x00,
                0x00,0x08,0x00,0x00,0x87,0x0E
            } ;

            _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] =
            {
                0x4F,0x09,0xBA,0x3B,0x4B,0x3E,
                0x4F,0x09,0x56,0x0E,0x00,0x00,
                0x4F,0x09,0x00,0x00,0xE7,0x10
            } ;
        #endif
    #else
        extern _CODE BYTE bCSCOffset_16_235[] ;
        extern _CODE BYTE bCSCOffset_0_255[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] ;
        extern _CODE BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] ;
        extern _CODE BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] ;

    #endif // extern table
#endif
#endif// DISABLE_TX_CSC


//////////////////////////////////////////////////////////////////////
// external Interface                                                         //
//////////////////////////////////////////////////////////////////////

void
HDMITX_InitInstance(INSTANCE *pInstance)
{
	if(pInstance && 0 < HDMITX_INSTANCE_MAX)
	{
		Instance[0] = *pInstance ;
	}
#ifdef CONFIG_AST1500_CAT6613
	Instance[0].outputparam.bInputColorMode = HDMI_RGB444;
#endif
}

static void C6613_Check_EMEM_sts(void)
{
#if defined(CODE202)
    byte timeout ;
    byte uc ;
#endif

    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(0xF8,0xC3);//unlock register
    HDMITX_WriteI2C_Byte(0xF8,0xA5);//unlock register

    HDMITX_WriteI2C_Byte(0x22,0x00);


    HDMITX_WriteI2C_Byte(0x10,0x03);//

    HDMITX_WriteI2C_Byte(0x11,0xA0);
    HDMITX_WriteI2C_Byte(0x12,0x00);//
    HDMITX_WriteI2C_Byte(0x13,0x08);
    HDMITX_WriteI2C_Byte(0x14,0x00);//
    HDMITX_WriteI2C_Byte(0x15,0x00);//
#if defined(CODE202)
    Instance[0].TxEMEMStatus=TRUE;


    for( timeout = 0 ; timeout < 250 ; timeout ++ )
    {
        DelayMS(1);
        uc = HDMITX_ReadI2C_Byte(0x1c) ;
        if( 0 != (0x80 & uc))
        {
            Instance[0].TxEMEMStatus=FALSE;
            break ;
        }
        if( 0 != (0x38 & uc))
        {
            Instance[0].TxEMEMStatus=TRUE;
            break ;
        }
    }
	// HDMITX_DEBUG_PRINTF(("timeout = %d, uc = %02X\n",(int)timeout,(int)uc)) ;
#else
#ifdef CONFIG_AST1500_CAT6613
	DelayMS(200);
#else
    delay1ms(200);
#endif
    if((0x80 & HDMITX_ReadI2C_Byte(0x1c))){    //if 0x1c[7]==1 EXT_ROM
          Instance[0].TxEMEMStatus=FALSE;
#ifdef _PRINT_HDMI_TX_
          //HDMITX_DEBUG_PRINT(("==Dev %X is ExtROM==\n",I2CDEV));
#endif
    }
    else
    {                                    //if 0x1c[1] !=1 EMEM
          Instance[0].TxEMEMStatus=TRUE;
#ifdef _PRINT_HDMI_TX_
          //HDMITX_DEBUG_PRINT(("==Dev %X is EMEM==\n",I2CDEV));
#endif
    }
    HDMITX_WriteI2C_Byte(0x04,0x3F);     //reset all reg
#endif
    HDMITX_WriteI2C_Byte(0xF8,0xFF);//lock register
}

static void InitCAT6613_HDCPROM(void)
{

    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(0xF8,0xC3);//unlock register
    HDMITX_WriteI2C_Byte(0xF8,0xA5);//unlock register
    if(Instance[0].TxEMEMStatus==TRUE){
            // with internal eMem
            HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xE0);
            HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0x48);
    }else{
        // with external ROM
        HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xA0);
        HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0x00);
    }
    HDMITX_WriteI2C_Byte(0xF8,0xFF);//lock register
}

void LowerDDCClock(void)
{
	volatile unsigned char uc;

	uc = HDMITX_ReadI2C_Byte(REG_TX_INT_CTRL);
	uc &= ~(M_DDC_SPD | M_LOW_SPD_DDC_EN);
	uc |= (B_LOW_SPD_DDC_EN | B_DDC_SPD_1);
	HDMITX_WriteI2C_Byte(REG_TX_INT_CTRL, uc);
}

void NormalDDCClock(void)
{
	volatile unsigned char uc;

	uc = HDMITX_ReadI2C_Byte(REG_TX_INT_CTRL);
	uc &= ~(M_DDC_SPD | M_LOW_SPD_DDC_EN);
	HDMITX_WriteI2C_Byte(REG_TX_INT_CTRL, uc);
}

#ifdef CONFIG_AST1500_CAT6613
BOOL InitCAT6613(void)
#else
void InitCAT6613()
#endif
{
    BYTE intclr;


    HDMITX_WriteI2C_Byte(REG_TX_INT_CTRL,Instance[0].bIntType);
    Instance[0].bIntPOL = (Instance[0].bIntType&B_INTPOL_ACTH)?TRUE:FALSE ;

    C6613_Check_EMEM_sts();

    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_REF_RST_HDMITX|B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);
    DelayMS(1);
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);

    // Avoid power loading in un play status.
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
	//////////////////////////////////////////////////////////////////
	// Setup HDCP ROM
	//////////////////////////////////////////////////////////////////
	InitCAT6613_HDCPROM();

    // set interrupt mask,mask value 0 is interrupt available.
    //HDMITX_WriteI2C_Byte(REG_TX_INT_MASK1,0xB2);
	HDMITX_WriteI2C_Byte(REG_TX_INT_MASK1,0x30);
    HDMITX_WriteI2C_Byte(REG_TX_INT_MASK2,0xF8);
    HDMITX_WriteI2C_Byte(REG_TX_INT_MASK3,0x37);
    Switch_HDMITX_Bank(0);
    DISABLE_NULL_PKT();
    DISABLE_ACP_PKT();
    DISABLE_ISRC1_PKT();
    DISABLE_ISRC2_PKT();
    DISABLE_AVI_INFOFRM_PKT();
    DISABLE_AUD_INFOFRM_PKT();
    DISABLE_SPD_INFOFRM_PKT();
    DISABLE_MPG_INFOFRM_PKT();

	//////////////////////////////////////////////////////////////////
	// Setup Output Audio format.
	//////////////////////////////////////////////////////////////////
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,Instance[0].bOutputAudioMode); // regE1 bOutputAudioMode should be loaded from ROM image.


	HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,0xFF);
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0xFF);
    intclr = (HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS))|B_CLR_AUD_CTS | B_INTACTDONE ;
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr); // clear interrupt.
    intclr &= ~(B_INTACTDONE);
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr); // INTACTDONE reset to zero.
#if defined(CODE202)
    {
        ULONG n=6144L;
        Switch_HDMITX_Bank(1);
        HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF));
        HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF));
        HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF));
        Switch_HDMITX_Bank(0);
        HDMITX_WriteI2C_Byte(0xc4,0xfe);
        HDMITX_OrREG_Byte(REG_TX_PKT_SINGLE_CTRL,(1<<4)|(1<<5));
    }
#endif
// #ifdef EXTERN_HDCPROM
// #pragma message("EXTERN ROM CODED");
// 	HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xA0);
// #endif

#ifdef CONFIG_AST1500_CAT6613
	return TRUE;
#endif
}

//////////////////////////////////////////////////////////////////////
// export this for dynamic change input signal
//////////////////////////////////////////////////////////////////////
BOOL SetupVideoInputSignal(BYTE inputSignalType)
{
	Instance[0].bInputVideoSignalType = inputSignalType ;
    // SetInputMode(inputColorMode,Instance[0].bInputVideoSignalType);
    return TRUE ;
}

static void WaitTxVidStable()
{
    BYTE i ;
    for( i = 0 ; i < 20 ; i++ )
    {
        DelayMS(15);
        if((HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        DelayMS(15);
        if((HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        DelayMS(15);
        if((HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        DelayMS(15);
        if((HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_TXVIDSTABLE) == 0 )
        {
            continue ;
        }
        break ;
    }
	if (i == 20)
		printk("TxVid not Stable!!!\n");
	else
		printk("TxVid Stable\n");
}

BOOL EnableVideoOutput(VIDEOPCLKLEVEL level,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI)
{
    // bInputVideoMode,bOutputVideoMode,Instance[0].bInputVideoSignalType,bAudioInputType,should be configured by upper F/W or loaded from EEPROM.
    // should be configured by initsys.c
    // VIDEOPCLKLEVEL level ;

    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);

#ifndef CONFIG_AST1500_CAT6613
    Instance[0].bHDMIMode = (BYTE)bHDMI ;
#endif
    // 2009/12/09 added by jau-chih.tseng@ite.com.tw
    Switch_HDMITX_Bank(1);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1,0x00);
    Switch_HDMITX_Bank(0);
    //~jau-chih.tseng@ite.com.tw

#ifdef CONFIG_AST1500_CAT6613
    if(bHDMI)
#else
    if(Instance[0].bHDMIMode)
#endif
    {
        SetAVMute(TRUE);
    }

    SetInputMode(inputColorMode,Instance[0].bInputVideoSignalType);

    SetCSCScale(inputColorMode,outputColorMode);

#ifdef CONFIG_AST1500_CAT6613
    if(bHDMI)
#else
    if(Instance[0].bHDMIMode)
#endif
    {
        HDMITX_WriteI2C_Byte(REG_TX_HDMI_MODE,B_TX_HDMI_MODE);
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_HDMI_MODE,B_TX_DVI_MODE);
    }

#ifdef INVERT_VID_LATCHEDGE
    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1);
    uc |= B_VDO_LATCH_EDGE ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc);
#endif

//    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,          B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);

    SetupAFE(level); // pass if High Freq request
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,          B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);
    // Clive suggestion.
    // clear int3 video stable interrupt.

    WaitTxVidStable();
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);
    DelayMS(10);
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,          B_AUD_RST_HDMITX|B_AREF_RST|B_HDCP_RST_HDMITX);
    DelayMS(150);
    WaitTxVidStable();
    FireAFE();

    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,0);
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,B_CLR_VIDSTABLE);
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE);

	return TRUE ;
}
#if !defined(CODE202)
BOOL EnableAudioOutput(ULONG VideoPixelClock,BYTE bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF)
{
    BYTE bAudioChannelEnable ;
//    unsigned long N ;

#ifndef CONFIG_AST1500_CAT6613
    Instance[0].TMDSClock = VideoPixelClock ;
#endif
    Instance[0].bAudFs = bAudioSampleFreq ;

    HDMITX_DEBUG_PRINTF("EnableAudioOutput(0,%ld",VideoPixelClock);
    HDMITX_DEBUG_PRINTF(",%x",(int)bAudioSampleFreq);
    HDMITX_DEBUG_PRINTF(",%d",(int)ChannelNumber);
    HDMITX_DEBUG_PRINTF(",%d",(int)bAudSWL);
    HDMITX_DEBUG_PRINTF(",%d);\n",(int)bSPDIF);

    switch(ChannelNumber)
    {
    case 7:
    case 8:
        bAudioChannelEnable = 0xF ;
        break ;
    case 6:
    case 5:
        bAudioChannelEnable = 0x7 ;
        break ;
    case 4:
    case 3:
        bAudioChannelEnable = 0x3 ;
        break ;
    case 2:
    case 1:
    default:
        bAudioChannelEnable = 0x1 ;
        break ;
    }

    if(bSPDIF) bAudioChannelEnable |= B_AUD_SPDIF ;

    if( bSPDIF )
    {
        Switch_HDMITX_Bank(1);
        HDMITX_WriteI2C_Byte(REGPktAudCTS0,0x50);
        HDMITX_WriteI2C_Byte(REGPktAudCTS1,0x73);
        HDMITX_WriteI2C_Byte(REGPktAudCTS2,0x00);

        HDMITX_WriteI2C_Byte(REGPktAudN0,0);
        HDMITX_WriteI2C_Byte(REGPktAudN1,0x18);
        HDMITX_WriteI2C_Byte(REGPktAudN2,0);
        Switch_HDMITX_Bank(0);

        HDMITX_WriteI2C_Byte(0xC5, 2); // D[1] = 0, HW auto count CTS
    }
    else
    {
        SetNCTS(VideoPixelClock,bAudioSampleFreq);
    }

    /*
    if(VideoPixelClock != 0)
    {
        SetNCTS(VideoPixelClock,bAudioSampleFreq);
    }
    else
    {
        switch(bAudioSampleFreq)
        {
		case AUDFS_32KHz: N = 4096; break;
		case AUDFS_44p1KHz: N = 6272; break;
		case AUDFS_48KHz: N = 6144; break;
		case AUDFS_88p2KHz: N = 12544; break;
		case AUDFS_96KHz: N = 12288; break;
		case AUDFS_176p4KHz: N = 25088; break;
		case AUDFS_192KHz: N = 24576; break;
		default: N = 6144;
        }
        Switch_HDMITX_Bank(1);
        HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((N)&0xFF));
        HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((N>>8)&0xFF));
        HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((N>>16)&0xF));
        Switch_HDMITX_Bank(0);
        HDMITX_WriteI2C_Byte(REG_TX_PKT_SINGLE_CTRL,0); // D[1] = 0,HW auto count CTS
    }
    */

	//HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
    SetAudioFormat(ChannelNumber,bAudioChannelEnable,bAudioSampleFreq,bAudSWL,bSPDIF);

	/* Special for ASPEED Settings */
	//Antony, added to test if behaviour can be the same as ast1600
	if (bSPDIF)
	{	// use external
		HDMITX_SetREG_Byte(REG_TX_CLK_CTRL0,~(B_MCLK_SAMPLE | B_AUTO_OVER_SAMPLING_CLOCK| O_EXT_MCLK_SEL | M_EXT_MCLK_SEL) , (B_MCLK_SAMPLE | O_EXT_MCLK_SEL | B_EXT_256FS) ) ;
	}
	else
	{	// use internal
		HDMITX_SetREG_Byte(REG_TX_CLK_CTRL0,~(B_MCLK_SAMPLE | B_AUTO_OVER_SAMPLING_CLOCK| O_EXT_MCLK_SEL | M_EXT_MCLK_SEL) , (B_AUTO_OVER_SAMPLING_CLOCK | B_EXT_256FS) ) ;
	}

#ifdef CONFIG_AST1500_CAT6613
	#if REG_DBG
	DumpCatHDMITXReg();
	#endif
#else
    #ifdef DEBUG
    DumpCatHDMITXReg();
    #endif // DEBUG
#endif
    return TRUE ;
}
#endif


BOOL
GetEDIDData(int EDIDBlockID,BYTE *pEDIDData)
{
	int i, j;
	BYTE CheckSum ;

	if(!pEDIDData)
	{
		return FALSE ;
	}

	for (j = 3; j > 0; j--) {

		for (i = 3; i > 0; i--) {
			if(ReadEDID(pEDIDData,EDIDBlockID/2,(EDIDBlockID%2)*128,128) == ER_FAIL)
			{
				//return FALSE ;
				HDMITX_DEBUG_PRINTF("!! Read EDID block %d failed\n", EDIDBlockID);
				continue;
			} else
				break;
		}

		if (i == 0)
			return FALSE;

		for( i = 0, CheckSum = 0 ; i < 128 ; i++ )
		{
			CheckSum += pEDIDData[i] ; CheckSum &= 0xFF ;
		}

		if( CheckSum != 0 )
		{
			int _j;
			HDMITX_DEBUG_PRINTF("block %d Invalid checksum %02X\n", EDIDBlockID, CheckSum);
			for (_j = 0; _j < 128; _j++)
			{
				printk("%02X", pEDIDData[_j]);
				if ((_j % 16) == 15)
					printk("\n");
				else
					printk(" ");
			}
			//return FALSE ;
			//goto fail_and_clear_edid;
			DelayMS(100);
			continue;
		}
		else
			break;

	}

	if (j == 0)
		return FALSE;

    return TRUE ;
}


BOOL
EnableHDCP(BYTE bEnable)
{
	printk("--->EnableHDCP(%d)\n", bEnable);
#ifdef SUPPORT_HDCP
    if(bEnable)
    {
        if(ER_FAIL == HDCP_Authenticate())
        {
            HDMITX_DEBUG_PRINTF("EnableHDCP(): Authenticate return ER_FAIL\n");
            HDCP_ResetAuth();
//			printk("<---EnableHDCP(FALSE)\n");
			return FALSE ;
        }
//		printk("<---EnableHDCP(TRUE)\n");
    }
    else
    {
        HDCP_ResetAuth();
    }
#endif
    return TRUE ;
}


BOOL HDMITX_AudioOutput(
    BYTE bAudioSampleFreq,
    BYTE ChannelNumber,
    BYTE bAudSWL,
    BYTE bSPDIF,
    BYTE *AUD_DB,
    BYTE *ucIEC60958ChStat,
    BYTE Audio_Type
    );

BYTE
CheckHDMITX(BYTE *pHPD,BYTE *pHPDChange)
{
    BYTE intdata1,intdata2,intdata3,sysstat;
    BYTE  intclr3 = 0 ;
#ifndef CONFIG_AST1500_CAT6613
    BYTE PrevHPD = Instance[0].bHPD ;
#endif
	BYTE HPD ;

    sysstat = HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS);
	// HDMITX_DEBUG_PRINTF(("REG_TX_SYS_STATUS = %X \n",sysstat));

	if((sysstat & (B_HPDETECT/*|B_RXSENDETECT*/)) == (B_HPDETECT/*|B_RXSENDETECT*/))
	{
#if !NO_INT_MODE
		HDMITX_DEBUG_PRINTF("hotplug ON\n");
#endif
		HPD = TRUE;
	}
	else
	{
#if !NO_INT_MODE
		HDMITX_DEBUG_PRINTF("hotplug OFF\n");
#endif
		HPD = FALSE;
	}
	// 2007/06/20 added by jj_tseng@chipadvanced.com

    if(pHPDChange)
    {
    	*pHPDChange = FALSE ;

    }
    //~jj_tseng@chipadvanced.com 2007/06/20

    if(HPD==FALSE)
    {
        Instance[0].bAuthenticated = FALSE ;
    }

    if(sysstat & B_INT_ACTIVE)
    {
		HDMITX_DEBUG_PRINTF("REG_TX_SYS_STATUS = 0x%02X \n",(int)sysstat);

        intdata1 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1);
        HDMITX_DEBUG_PRINTF("INT_Handler: reg%X = %X\n",(int)REG_TX_INT_STAT1,(int)intdata1);
#if defined(CODE202)
        if(intdata1 & B_INT_AUD_OVERFLOW)
        {
#ifdef Debug_message
            HDMITX_DEBUG_PRINTF(("B_INT_AUD_OVERFLOW.\n"));
#endif
            HDMITX_OrREG_Byte(REG_TX_SW_RST,(B_AUD_RST_HDMITX|B_AREF_RST));
            //Bruce110425. This msleep() is required for releasing CPU to other drivers like
            //cat6023 or videoip.ko
            msleep(100);
            HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
            //AudioDelayCnt=AudioOutDelayCnt;
            //LastRefaudfreqnum=0;
        }
#endif

		if(intdata1 & B_INT_DDCFIFO_ERR)
		{
		    HDMITX_DEBUG_PRINTF("DDC FIFO Error.\n");
		    ClearDDCFIFO();
		}

#if !defined(CODE202)
#ifdef CONFIG_AST1500_CAT6613
		if(intdata1 & B_INT_AUD_OVERFLOW)
		{
			BYTE uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST);
			HDMITX_DEBUG_PRINTF("audio overflow\n");
			uc |= (B_AUD_RST_HDMITX | B_AREF_RST);
			HDMITX_WriteI2C_Byte(REG_TX_SW_RST, uc);
			//Bruce110425. This msleep() is required for releasing CPU to other drivers like
			//cat6023 or videoip.ko
			msleep(100);
			uc &= ~(B_AUD_RST_HDMITX | B_AREF_RST);
			HDMITX_WriteI2C_Byte(REG_TX_SW_RST, uc);
		}
#endif
#endif

		if(intdata1 & B_INT_DDC_BUS_HANG)
		{
		    HDMITX_DEBUG_PRINTF("DDC BUS HANG.\n");
            AbortDDC();

            if(Instance[0].bAuthenticated)
            {
                HDMITX_DEBUG_PRINTF("when DDC hang,and aborted DDC,the HDCP authentication need to restart.\n");
                #ifdef SUPPORT_HDCP
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
			//In REPEATER mode, HDCP state machine in video engine driver will retry authentication.
			CRT_HDCP1_Downstream_Port_Auth_Status_Callback(0, NULL, 0, 0, NULL);
#else //#if SUPPORT_HDCP_REPEATER
			HDCP_ResumeAuthentication();
#endif //#if SUPPORT_HDCP_REPEATER
#else
			HDCP_ResumeAuthentication();
#endif
                #endif
            }
		}


		if(intdata1 & (B_INT_HPD_PLUG/*|B_INT_RX_SENSE*/))
		{

            if(pHPDChange)
            {
				*pHPDChange = TRUE ;
			}

            if(HPD == FALSE)
            {
                HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_AREF_RST|B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_HDCP_RST_HDMITX);
                DelayMS(1);
                HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
                //HDMITX_DEBUG_PRINTF(("Unplug,%x %x\n",(int)HDMITX_ReadI2C_Byte(REG_TX_SW_RST),(int)HDMITX_ReadI2C_Byte(REG_TX_AFE_DRV_CTRL)));
            }
		}


        intdata2 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2);
        HDMITX_DEBUG_PRINTF("INT_Handler: reg%X = %X\n",(int)REG_TX_INT_STAT2,(int)intdata2);



		#ifdef SUPPORT_HDCP
		if(intdata2 & B_INT_AUTH_DONE)
		{
            HDMITX_DEBUG_PRINTF("interrupt Authenticate Done.\n");
            HDMITX_OrREG_Byte(REG_TX_INT_MASK2,(BYTE)B_T_AUTH_DONE_MASK);
            Instance[0].bAuthenticated = TRUE ;
            SetAVMute(FALSE);
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
			//to do???
#endif
#endif
		}

		if(intdata2 & B_INT_AUTH_FAIL)
		{
            HDMITX_DEBUG_PRINTF("interrupt Authenticate Fail.\n");
			AbortDDC();   // @emily add
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
			//In REPEATER mode, HDCP state machine in video engine driver will retry authentication.
			CRT_HDCP1_Downstream_Port_Auth_Status_Callback(0, NULL, 0, 0, NULL);
#else
			HDCP_ResumeAuthentication();
#endif
#else
			HDCP_ResumeAuthentication();
#endif
        }
        #endif // SUPPORT_HDCP

		intdata3 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT3);
		if(intdata3 & B_INT_VIDSTABLE)
		{
			sysstat = HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS);
			if(sysstat & B_TXVIDSTABLE)
			{
				FireAFE();
			}
		}
        HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,0xFF);
        HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0xFF);
        intclr3 = (HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS))|B_CLR_AUD_CTS | B_INTACTDONE ;
        HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr3); // clear interrupt.
        intclr3 &= ~(B_INTACTDONE);
        HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr3); // INTACTDONE reset to zero.
    }
    else
    {
#ifdef CONFIG_AST1500_CAT6613
	#if !NO_INT_MODE
		HDMITX_DEBUG_PRINTF("no interrupt pending!!!\n");
	#endif
#else
        if(pHPDChange)
        {
		    if(HPD != PrevHPD)
		    {
                *pHPDChange = TRUE;
            }
            else
            {
               *pHPDChange = FALSE;
            }
        }
#endif
    }

    if(pHPDChange)
    {
        if((*pHPDChange==TRUE) &&(HPD==FALSE))
        {
            HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
        }
    }

#ifndef CONFIG_AST1500_CAT6613//no use to us according ITE Max
    SetupAudioChannel(); // 2007/12/12 added by jj_tseng
#endif

    if(pHPD)
    {
         *pHPD = HPD    ;
    }

#ifndef CONFIG_AST1500_CAT6613
    Instance[0].bHPD = HPD ;
#endif
    return HPD ;
}

#ifdef CONFIG_AST1500_CAT6613
BOOL checkRxSENDetect(void)
{
	if (HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_RXSENDETECT)
		return TRUE;
	else
		return FALSE;
}
#endif


void
DisableCAT6613()
{
#ifdef CONFIG_AST1500_CAT6613
	HDMITX_DEBUG_PRINTF("DisableCAT6613\n");
	/*
	** This DisableCAT6613() is called on unplug. But after re-plug in, only CRT(Video) driver
	** will re-config the video parameters. The audio parameters won't be re-configurred
	** because there is no audio changes from source point of view.
	** Clean up all the outputparam hence causes audio disappear after HDMI hotplug.
	** To quick fix this issue, don't clean all outputparam. Instead, Just un-set "VideoSet".
	*/
	//memset(&Instance[0].outputparam, 0, sizeof(Output_Param));
	Instance[0].outputparam.VideoSet = 0;
	Instance[0].outputparam.AudioSet = 0;

	/*
	** Bruce110401. Resolve Panasonic TV snowing issue.
	** Reported from UK, new high end Panasonic TV will snow if we disable 6613 when HDCP
	** is ON and setup_video again with HDCP OFF.
	** To resolve this issue, we should SetAVMute before disabling 6613.
	** From Clive@ITE, HDCP standard says that source should SetAVMute before turning HDCP
	** Off.
	*/
	if (Instance[0].outputparam.bCapibility & CAP_HDMI) {
		SetAVMute(TRUE);
		DelayMS(200); //I add a delay here to make sure the sink get the GCP.
	}

#endif
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_AREF_RST|B_VID_RST_HDMITX|B_AUD_RST_HDMITX|B_HDCP_RST_HDMITX);
    DelayMS(1);
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
}

void
DisableVideoOutput()
{
    BYTE uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) | B_VID_RST_HDMITX ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc);
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD);
}


void
DisableAudioOutput()
{
#if defined(CODE202)
    HDMITX_OrReg_Byte(REG_TX_SW_RST,(B_AUD_RST_HDMITX | B_AREF_RST));
#else
    BYTE uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) | B_AUD_RST_HDMITX ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc);
#endif
}



BOOL
EnableAVIInfoFrame(BYTE bEnable,BYTE *pAVIInfoFrame)
{
    if(!bEnable)
    {
#ifdef CONFIG_AST1500_CAT6613
		//clear AVI InfoFrame content
		Switch_HDMITX_Bank(1);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB2, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB3, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB4, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB5, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB6, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB7, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB8, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB9, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB10, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB11, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB12, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB13, 0);
		HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_SUM, 0);
		Switch_HDMITX_Bank(0);
#endif
        DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }

    if(SetAVIInfoFrame((AVI_InfoFrame *)pAVIInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}

BOOL
EnableAudioInfoFrame(BYTE bEnable,BYTE *pAudioInfoFrame)
{
    if(!bEnable)
    {
        DISABLE_AVI_INFOFRM_PKT();
        return TRUE ;
    }


    if(SetAudioInfoFrame((Audio_InfoFrame *)pAudioInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}

void
SetAVMute(BYTE bEnable)
{
    BYTE uc ;

    Switch_HDMITX_Bank(0);
    uc = HDMITX_ReadI2C_Byte(REG_TX_GCP);
    uc &= ~B_TX_SETAVMUTE ;
    uc |= bEnable?B_TX_SETAVMUTE:0 ;
    HDMITX_WriteI2C_Byte(REG_TX_GCP,uc);
    HDMITX_WriteI2C_Byte(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

void
SetOutputColorDepthPhase(BYTE ColorDepth,BYTE bPhase)
{
    BYTE uc ;
    BYTE bColorDepth ;

    if(ColorDepth == 30)
    {
        bColorDepth = B_CD_30 ;
    }
    else if (ColorDepth == 36)
    {
        bColorDepth = B_CD_36 ;
    }
    else if (ColorDepth == 24)
    {
#if defined(CODE202)
        bColorDepth = B_CD_24 ;
        //bColorDepth = 0 ;//modify JJ by mail 20100423 1800 // not indicated
#else
        //Bruce110106. From JJ, it is safe to set 0 in this case.
        bColorDepth = 0;
#endif
    }
    else
    {
        bColorDepth = 0 ; // not indicated
    }

    Switch_HDMITX_Bank(0);
    uc = HDMITX_ReadI2C_Byte(REG_TX_GCP);
    uc &= ~B_COLOR_DEPTH_MASK ;
    uc |= bColorDepth&B_COLOR_DEPTH_MASK;
    HDMITX_WriteI2C_Byte(REG_TX_GCP,uc);
}

void
Get6613Reg(BYTE *pReg)
{
    int i ;
    BYTE reg ;
    Switch_HDMITX_Bank(0);
    for(i = 0 ; i < 0x100 ; i++)
    {
        reg = i & 0xFF ;
        pReg[i] = HDMITX_ReadI2C_Byte(reg);
    }
    Switch_HDMITX_Bank(1);
    for(reg = 0x30 ; reg < 0xB0 ; i++,reg++)
    {
        pReg[i] = HDMITX_ReadI2C_Byte(reg);
    }
    Switch_HDMITX_Bank(0);

}
//////////////////////////////////////////////////////////////////////
// SubProcedure process                                                       //
//////////////////////////////////////////////////////////////////////
#ifdef SUPPORT_DEGEN

typedef struct {
    MODE_ID id ;
    BYTE Reg90;
    BYTE Reg92;
    BYTE Reg93;
    BYTE Reg94;
    BYTE Reg9A;
    BYTE Reg9B;
    BYTE Reg9C;
    BYTE Reg9D;
    BYTE Reg9E;
    BYTE Reg9F;
} DEGEN_Setting ;


static _CODE DEGEN_Setting DeGen_Table[] = {
    {CEA_640x480p60      ,0x01,0x8E,0x0E,0x30,0x22,0x02,0x20,0xFF,0xFF,0xFF},
    // HDES = 142, HDEE = 782, VDES = 34, VDEE = 514
    {CEA_720x480p60      ,0x01,0x78,0x48,0x30,0x23,0x03,0x20,0xFF,0xFF,0xFF},
    // HDES = 120, HDEE = 840, VDES = 35, VDEE = 515
    {CEA_1280x720p60     ,0x07,0x02,0x02,0x61,0x18,0xE8,0x20,0xFF,0xFF,0xFF},
    // HDES = 258, HDEE = 1538, VDES = 24, VDEE = 744
//    {CEA_1920x1080i60    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x45,0x61,0x42},
//    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 581, VDE2 = 1121
    {CEA_1920x1080i60    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x46,0x62,0x42},
    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 582, VDE2 = 1122

    {CEA_720x480i60      ,0x01,0x75,0x45,0x30,0x11,0x01,0x10,0x17,0x07,0x21},
    // HDES = 117, HDEE = 837, VDES = 17, VDEE = 257, VDS2 = 279, VDE2 = 519
    {CEA_720x240p60      ,0x01,0x75,0x45,0x30,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 117, HDEE = 837, VDES = 17, VDEE = 257
    {CEA_1440x480i60     ,0x01,0xEC,0x8C,0x60,0x11,0x01,0x10,0x17,0x07,0x21},
    // HDES = 236, HDEE = 1676, VDES = 17, VDEE = 257, VDS2 = 279, VDE2 = 519
    {CEA_1440x240p60     ,0x01,0xEC,0x8C,0x60,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 236, HDEE = 1676, VDES = 17, VDEE = 257
    {CEA_2880x480i60     ,0x01,0x16,0x56,0xD2,0x11,0x01,0x10,0x17,0x07,0x21},
    // HDES = 534, HDEE = 3414, VDES = 17, VDEE = 257, VDS2 = 279, VDE2 = 519
    {CEA_2880x240p60     ,0x01,0x16,0x56,0xD2,0x11,0x01,0x10,0xFF,0xFF,0xFF},
    // HDES = 534, HDEE = 3414, VDES = 17, VDEE = 257
    {CEA_1440x480p60     ,0x01,0xF2,0x92,0x60,0x23,0x03,0x20,0xFF,0xFF,0xFF},
    // HDES = 242, HDEE = 1682, VDES = 35, VDEE = 515
    {CEA_1920x1080p60    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_720x576p50      ,0x01,0x82,0x52,0x30,0x2b,0x6b,0x20,0xFF,0xFF,0xFF},
    // HDES = 130, HDEE = 850, VDES = 43, VDEE = 619
    {CEA_1280x720p50     ,0x07,0x02,0x02,0x61,0x18,0xE8,0x20,0xFF,0xFF,0xFF},
    // HDES = 258, HDEE = 1538, VDES = 24, VDEE = 744
    {CEA_1920x1080i50    ,0x07,0xBE,0x3E,0x80,0x13,0x2F,0x20,0x46,0x62,0x42},
    // HDES = 190, HDEE = 2110, VDES = 19, VDEE = 559, VDS2 = 582, VDE2 = 1122
    {CEA_720x576i50      ,0x01,0x82,0x52,0x30,0x15,0x35,0x10,0x4D,0x6D,0x21},
    // HDES = 130, HDEE = 850, VDES = 21, VDEE = 309, VDS2 = 333, VDE2 = 621
    {CEA_1440x576i50     ,0x01,0x06,0xA6,0x61,0x15,0x35,0x10,0x4D,0x6D,0x21},
    // HDES = 262, HDEE = 1702, VDES = 21, VDEE = 309, VDS2 = 333, VDE2 = 621
    {CEA_720x288p50      ,0x01,0x82,0x52,0x30,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 130, HDEE = 850, VDES = 21, VDEE = 309
    {CEA_1440x288p50     ,0x01,0x06,0xA6,0x61,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 262, HDEE = 1702, VDES = 21, VDEE = 309
    {CEA_2880x576i50     ,0x01,0x0E,0x4E,0xD2,0x15,0x35,0x10,0x4D,0x6D,0x21},
    // HDES = 526, HDEE = 3406, VDES = 21, VDEE = 309, VDS2 = 333, VDE2 = 621
    {CEA_2880x288p50     ,0x01,0x0E,0x4E,0xD2,0x15,0x35,0x10,0xFF,0xFF,0xFF},
    // HDES = 526, HDEE = 3406, VDES = 21, VDEE = 309
    {CEA_1440x576p50     ,0x05,0x06,0xA6,0x61,0x2B,0x6B,0x20,0xFF,0xFF,0xFF},
    // HDES = 262, HDEE = 1702, VDES = 43, VDEE = 619
    {CEA_1920x1080p50    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p24    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p25    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {CEA_1920x1080p30    ,0x07,0xBE,0x3E,0x80,0x28,0x60,0x40,0xFF,0xFF,0xFF},
    // HDES = 190, HDEE = 2110, VDES = 40, VDEE = 1120
    {VESA_640x350p85     ,0x03,0x9E,0x1E,0x30,0x3E,0x9C,0x10,0xFF,0xFF,0xFF},
    // HDES = 158, HDEE = 798, VDES = 62, VDEE = 412
    {VESA_640x400p85     ,0x05,0x9E,0x1E,0x30,0x2B,0xBB,0x10,0xFF,0xFF,0xFF},
    // HDES = 158, HDEE = 798, VDES = 43, VDEE = 443
    {VESA_720x400p85     ,0x05,0xB2,0x82,0x30,0x2C,0xBC,0x10,0xFF,0xFF,0xFF},
    // HDES = 178, HDEE = 898, VDES = 44, VDEE = 444
    {VESA_640x480p60     ,0x01,0x8E,0x0E,0x30,0x22,0x02,0x20,0xFF,0xFF,0xFF},
    // HDES = 142, HDEE = 782, VDES = 34, VDEE = 514
    {VESA_640x480p72     ,0x01,0xA6,0x26,0x30,0x1E,0xFE,0x10,0xFF,0xFF,0xFF},
    // HDES = 166, HDEE = 806, VDES = 30, VDEE = 510
    {VESA_640x480p75     ,0x01,0xB6,0x36,0x30,0x12,0xF2,0x10,0xFF,0xFF,0xFF},
    // HDES = 182, HDEE = 822, VDES = 18, VDEE = 498
    {VESA_640x480p85     ,0x01,0x86,0x06,0x30,0x1B,0xFB,0x10,0xFF,0xFF,0xFF},
    // HDES = 134, HDEE = 774, VDES = 27, VDEE = 507
    {VESA_800x600p56     ,0x07,0xC6,0xE6,0x30,0x17,0x6F,0x20,0xFF,0xFF,0xFF},
    // HDES = 198, HDEE = 998, VDES = 23, VDEE = 623
    {VESA_800x600p60     ,0x07,0xD6,0xF6,0x30,0x1A,0x72,0x20,0xFF,0xFF,0xFF},
    // HDES = 214, HDEE = 1014, VDES = 26, VDEE = 626
    {VESA_800x600p72     ,0x07,0xB6,0xD6,0x30,0x1C,0x74,0x20,0xFF,0xFF,0xFF},
    // HDES = 182, HDEE = 982, VDES = 28, VDEE = 628
    {VESA_800x600p75     ,0x07,0xEE,0x0E,0x40,0x17,0x6F,0x20,0xFF,0xFF,0xFF},
    // HDES = 238, HDEE = 1038, VDES = 23, VDEE = 623
    {VESA_800X600p85     ,0x07,0xD6,0xF6,0x30,0x1D,0x75,0x20,0xFF,0xFF,0xFF},
    // HDES = 214, HDEE = 1014, VDES = 29, VDEE = 629
    {VESA_840X480p60     ,0x07,0xDE,0x2E,0x40,0x1E,0xFE,0x10,0xFF,0xFF,0xFF},
    // HDES = 222, HDEE = 1070, VDES = 30, VDEE = 510
    {VESA_1024x768p60    ,0x01,0x26,0x26,0x51,0x22,0x22,0x30,0xFF,0xFF,0xFF},
    // HDES = 294, HDEE = 1318, VDES = 34, VDEE = 802
    {VESA_1024x768p70    ,0x01,0x16,0x16,0x51,0x22,0x22,0x30,0xFF,0xFF,0xFF},
    // HDES = 278, HDEE = 1302, VDES = 34, VDEE = 802
    {VESA_1024x768p75    ,0x07,0x0E,0x0E,0x51,0x1E,0x1E,0x30,0xFF,0xFF,0xFF},
    // HDES = 270, HDEE = 1294, VDES = 30, VDEE = 798
    {VESA_1024x768p85    ,0x07,0x2E,0x2E,0x51,0x26,0x26,0x30,0xFF,0xFF,0xFF},
    // HDES = 302, HDEE = 1326, VDES = 38, VDEE = 806
    {VESA_1152x864p75    ,0x07,0x7E,0xFE,0x51,0x22,0x82,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1534, VDES = 34, VDEE = 898
    {VESA_1280x768p60R   ,0x03,0x6E,0x6E,0x50,0x12,0x12,0x30,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1390, VDES = 18, VDEE = 786
    {VESA_1280x768p60    ,0x05,0x3E,0x3E,0x61,0x1A,0x1A,0x30,0xFF,0xFF,0xFF},
    // HDES = 318, HDEE = 1598, VDES = 26, VDEE = 794
    {VESA_1280x768p75    ,0x05,0x4E,0x4E,0x61,0x21,0x21,0x30,0xFF,0xFF,0xFF},
    // HDES = 334, HDEE = 1614, VDES = 33, VDEE = 801
    {VESA_1280x768p85    ,0x05,0x5E,0x5E,0x61,0x25,0x25,0x30,0xFF,0xFF,0xFF},
    // HDES = 350, HDEE = 1630, VDES = 37, VDEE = 805
    {VESA_1280x960p60    ,0x07,0xA6,0xA6,0x61,0x26,0xE6,0x30,0xFF,0xFF,0xFF},
    // HDES = 422, HDEE = 1702, VDES = 38, VDEE = 998
    {VESA_1280x960p85    ,0x07,0x7E,0x7E,0x61,0x31,0xF1,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1662, VDES = 49, VDEE = 1009
    {VESA_1280x1024p60   ,0x07,0x66,0x66,0x61,0x28,0x28,0x40,0xFF,0xFF,0xFF},
    // HDES = 358, HDEE = 1638, VDES = 40, VDEE = 1064
    {VESA_1280x1024p75   ,0x07,0x86,0x86,0x61,0x28,0x28,0x40,0xFF,0xFF,0xFF},
    // HDES = 390, HDEE = 1670, VDES = 40, VDEE = 1064
    {VESA_1280X1024p85   ,0x07,0x7E,0x7E,0x61,0x2E,0x2E,0x40,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1662, VDES = 46, VDEE = 1070
    {VESA_1360X768p60    ,0x07,0x6E,0xBE,0x61,0x17,0x17,0x30,0xFF,0xFF,0xFF},
    // HDES = 366, HDEE = 1726, VDES = 23, VDEE = 791
    {VESA_1400x768p60R   ,0x03,0x6E,0xE6,0x50,0x1A,0x34,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1510, VDES = 26, VDEE = 1076
    {VESA_1400x768p60    ,0x05,0x76,0xEE,0x61,0x23,0x3D,0x40,0xFF,0xFF,0xFF},
    // HDES = 374, HDEE = 1774, VDES = 35, VDEE = 1085
    {VESA_1400x1050p75   ,0x05,0x86,0xFE,0x61,0x2D,0x47,0x40,0xFF,0xFF,0xFF},
    // HDES = 390, HDEE = 1790, VDES = 45, VDEE = 1095
    {VESA_1400x1050p85   ,0x05,0x96,0x0E,0x71,0x33,0x4D,0x40,0xFF,0xFF,0xFF},
    // HDES = 406, HDEE = 1806, VDES = 51, VDEE = 1101
    {VESA_1440x900p60R   ,0x03,0x6E,0x0E,0x60,0x16,0x9A,0x30,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1550, VDES = 22, VDEE = 922
    {VESA_1440x900p60    ,0x05,0x7E,0x1E,0x71,0x1E,0xA2,0x30,0xFF,0xFF,0xFF},
    // HDES = 382, HDEE = 1822, VDES = 30, VDEE = 930
    {VESA_1440x900p75    ,0x05,0x8E,0x2E,0x71,0x26,0xAA,0x30,0xFF,0xFF,0xFF},
    // HDES = 398, HDEE = 1838, VDES = 38, VDEE = 938
    {VESA_1440x900p85    ,0x05,0x96,0x36,0x71,0x2C,0xB0,0x30,0xFF,0xFF,0xFF},
    // HDES = 406, HDEE = 1846, VDES = 44, VDEE = 944
    {VESA_1600x1200p60   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p65   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p70   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p75   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1600x1200p85   ,0x07,0xEE,0x2E,0x81,0x30,0xE0,0x40,0xFF,0xFF,0xFF},
    // HDES = 494, HDEE = 2094, VDES = 48, VDEE = 1248
    {VESA_1680x1050p60R  ,0x03,0x6E,0xFE,0x60,0x1A,0x34,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 1790, VDES = 26, VDEE = 1076
    {VESA_1680x1050p60   ,0x05,0xC6,0x56,0x81,0x23,0x3D,0x40,0xFF,0xFF,0xFF},
    // HDES = 454, HDEE = 2134, VDES = 35, VDEE = 1085
    {VESA_1680x1050p75   ,0x05,0xD6,0x66,0x81,0x2D,0x47,0x40,0xFF,0xFF,0xFF},
    // HDES = 470, HDEE = 2150, VDES = 45, VDEE = 1095
    {VESA_1680x1050p85   ,0x05,0xDE,0x6E,0x81,0x33,0x4D,0x40,0xFF,0xFF,0xFF},
    // HDES = 478, HDEE = 2158, VDES = 51, VDEE = 1101
    {VESA_1792x1344p60   ,0x05,0x0E,0x0E,0x92,0x30,0x70,0x50,0xFF,0xFF,0xFF},
    // HDES = 526, HDEE = 2318, VDES = 48, VDEE = 1392
    {VESA_1792x1344p75   ,0x05,0x36,0x36,0x92,0x47,0x87,0x50,0xFF,0xFF,0xFF},
    // HDES = 566, HDEE = 2358, VDES = 71, VDEE = 1415
    {VESA_1856x1392p60   ,0x05,0x3E,0x7E,0x92,0x2D,0x9D,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2430, VDES = 45, VDEE = 1437
    {VESA_1856x1392p75   ,0x05,0x3E,0x7E,0x92,0x6A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2430, VDES = 106, VDEE = 1498
    {VESA_1920x1200p60R  ,0x03,0x6E,0xEE,0x70,0x1F,0xCF,0x40,0xFF,0xFF,0xFF},
    // HDES = 110, HDEE = 2030, VDES = 31, VDEE = 1231
    {VESA_1920x1200p60   ,0x05,0x16,0x96,0x92,0x29,0xD9,0x40,0xFF,0xFF,0xFF},
    // HDES = 534, HDEE = 2454, VDES = 41, VDEE = 1241
    {VESA_1920x1200p75   ,0x05,0x26,0xA6,0x92,0x33,0xE3,0x40,0xFF,0xFF,0xFF},
    // HDES = 550, HDEE = 2470, VDES = 51, VDEE = 1251
    {VESA_1920x1200p85   ,0x05,0x2E,0xAE,0x92,0x3A,0xEA,0x40,0xFF,0xFF,0xFF},
    // HDES = 558, HDEE = 2478, VDES = 58, VDEE = 1258
    {VESA_1920x1440p60   ,0x05,0x26,0xA6,0x92,0x3A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 550, HDEE = 2470, VDES = 58, VDEE = 1498
    {VESA_1920x1440p75   ,0x05,0x3E,0xBE,0x92,0x3A,0xDA,0x50,0xFF,0xFF,0xFF},
    // HDES = 574, HDEE = 2494, VDES = 58, VDEE = 1498
    {UNKNOWN_MODE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
} ;

BOOL ProgramDEGenModeByID(MODE_ID id,BYTE bInputSignalType)
{
    int i ;
    if( (bInputSignalType & (T_MODE_DEGEN|T_MODE_SYNCGEN|T_MODE_SYNCEMB) )==(T_MODE_DEGEN))
    {
        for( i = 0 ; DeGen_Table[i].id != UNKNOWN_MODE ; i++ )
        {
            if( id == DeGen_Table[i].id ) break ;
        }
        if( DeGen_Table[i].id == UNKNOWN_MODE )
        {
            return FALSE ;
        }

        Switch_HDMITX_Bank(0);
        HDMITX_WriteI2C_Byte(0x90,DeGen_Table[i].Reg90);
        HDMITX_WriteI2C_Byte(0x92,DeGen_Table[i].Reg92);
        HDMITX_WriteI2C_Byte(0x93,DeGen_Table[i].Reg93);
        HDMITX_WriteI2C_Byte(0x94,DeGen_Table[i].Reg94);
        HDMITX_WriteI2C_Byte(0x9A,DeGen_Table[i].Reg9A);
        HDMITX_WriteI2C_Byte(0x9B,DeGen_Table[i].Reg9B);
        HDMITX_WriteI2C_Byte(0x9C,DeGen_Table[i].Reg9C);
        HDMITX_WriteI2C_Byte(0x9D,DeGen_Table[i].Reg9D);
        HDMITX_WriteI2C_Byte(0x9E,DeGen_Table[i].Reg9E);
        HDMITX_WriteI2C_Byte(0x9F,DeGen_Table[i].Reg9F);
        return TRUE ;

    }
    return FALSE ;
}

#endif

#ifdef SUPPORT_SYNCEMBEDDED
/* ****************************************************** */
// sync embedded table setting,defined as comment.
/* ****************************************************** */
struct SyncEmbeddedSetting {
    BYTE fmt ;
    BYTE RegHVPol ; // Reg90
    BYTE RegHfPixel ; // Reg91
    BYTE RegHSSL ; // Reg95
    BYTE RegHSEL ; // Reg96
    BYTE RegHSH ; // Reg97
    BYTE RegVSS1 ; // RegA0
    BYTE RegVSE1 ; // RegA1
    BYTE RegVSS2 ; // RegA2
    BYTE RegVSE2 ; // RegA3

    ULONG PCLK ;
    BYTE VFreq ;
} ;

static _CODE struct SyncEmbeddedSetting SyncEmbTable[] = {
 // {FMT,0x90,0x91,
 //                 0x95,0x96,0x97,0xA0,0xA1,0xA2,0xA3,PCLK,VFREQ},
    {   1,0xF0,0x31,0x0E,0x6E,0x00,0x0A,0xC0,0xFF,0xFF,25175000,60},
    {   2,0xF0,0x31,0x0E,0x4c,0x00,0x09,0xF0,0xFF,0xFF,27000000,60},
    {   3,0xF0,0x31,0x0E,0x4c,0x00,0x09,0xF0,0xFF,0xFF,27000000,60},
    {   4,0x76,0x33,0x6c,0x94,0x00,0x05,0xA0,0xFF,0xFF,74175000,60},
    {   5,0x26,0x4A,0x56,0x82,0x00,0x02,0x70,0x34,0x92,74175000,60},
    {   6,0xE0,0x1B,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,27000000,60},
    {   7,0xE0,0x1B,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,27000000,60},
    {   8,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,27000000,60},
    {   9,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,27000000,60},
    {  10,0xe0,0x1b,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,54000000,60},
    {  11,0xe0,0x1b,0x11,0x4F,0x00,0x04,0x70,0x0A,0xD1,54000000,60},
    {  12,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,54000000,60},
    {  13,0x00,0xff,0x11,0x4F,0x00,0x04,0x70,0xFF,0xFF,54000000,60},
    {  14,0x00,0xff,0x1e,0x9A,0x00,0x09,0xF0,0xFF,0xFF,54000000,60},
    {  15,0x00,0xff,0x1e,0x9A,0x00,0x09,0xF0,0xFF,0xFF,54000000,60},
    {  16,0x06,0xff,0x56,0x82,0x00,0x04,0x90,0xFF,0xFF,148350000,60},
    {  17,0x00,0xff,0x0a,0x4A,0x00,0x05,0xA0,0xFF,0xFF,27000000,50},
    {  18,0x00,0xff,0x0a,0x4A,0x00,0x05,0xA0,0xFF,0xFF,27000000,50},
    {  19,0x06,0xff,0xB6,0xDE,0x11,0x05,0xA0,0xFF,0xFF,74250000,50},
    {  20,0x66,0x73,0x0e,0x3A,0x22,0x02,0x70,0x34,0x92,74250000,50},
    {  21,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,27000000,50},
    {  22,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,27000000,50},
    {  23,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,27000000,50},
    {  24,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,27000000,50},
    {  25,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,54000000,50},
    {  26,0xA0,0x1B,0x0a,0x49,0x00,0x02,0x50,0x3A,0xD1,54000000,50},
    {  27,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,54000000,50},
    {  28,0x00,0xff,0x0a,0x49,0x00,0x02,0x50,0xFF,0xFF,54000000,50},
    {  29,0x04,0xff,0x16,0x96,0x00,0x05,0xA0,0xFF,0xFF,54000000,50},
    {  30,0x04,0xff,0x16,0x96,0x00,0x05,0xA0,0xFF,0xFF,54000000,50},
    {  31,0x06,0xff,0x0e,0x3a,0x22,0x04,0x90,0xFF,0xFF,148500000,50},
#if defined(CODE202)
    {  32,0xF6,0xFF,0x7C,0xA8,0x22,0x04,0x90,0xFF,0xFF,  74000000L,24},// 1920x1080@24Hz
    {  33,0xF6,0xFF,0x0E,0x3A,0x22,0x04,0x90,0xFF,0xFF,  74000000L,25},// 1920x1080@25Hz
    {  34,0xF6,0xFF,0x56,0x82,0x00,0x04,0x90,0xFF,0xFF,  74000000L,30},// 1920x1080@30Hz
    {  60,0xF0,0xFF,0xDE,0x06,0x76,0x05,0xA0,0xFF,0xFF,  59400000L,24},// 1280x720@24Hz
    {  61,0xF0,0xFF,0x72,0x9A,0x99,0x05,0xA0,0xFF,0xFF,  74250000L,25},// 1280x720@25Hz
    {  62,0xF0,0xFF,0xDE,0x06,0x76,0x05,0xA0,0xFF,0xFF,  74250000L,30},// 1280x720@30Hz
#endif
    {0xFF,0xFF,0xff,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0}
} ;

BOOL
ProgramSyncEmbeddedVideoMode(BYTE VIC,BYTE bInputSignalType)
{
    int i ;
    // if Embedded Video,need to generate timing with pattern register

    HDMITX_DEBUG_PRINTF(("ProgramSyncEmbeddedVideoMode(%d,%x)\n",(int)VIC,(int)bInputSignalType));

    if( bInputSignalType & T_MODE_SYNCEMB )
    {
        for(i = 0 ; SyncEmbTable[i].fmt != 0xFF ; i++)
        {
            if(VIC == SyncEmbTable[i].fmt)
            {
                break ;
            }
        }

        if(SyncEmbTable[i].fmt == 0xFF)
        {
            return FALSE ;
        }

        HDMITX_WriteI2C_Byte(REG_TX_HVPol,SyncEmbTable[i].RegHVPol); // Reg90
        HDMITX_WriteI2C_Byte(REG_TX_HfPixel,SyncEmbTable[i].RegHfPixel); // Reg91

        HDMITX_WriteI2C_Byte(REG_TX_HSSL,SyncEmbTable[i].RegHSSL); // Reg95
        HDMITX_WriteI2C_Byte(REG_TX_HSEL,SyncEmbTable[i].RegHSEL); // Reg96
        HDMITX_WriteI2C_Byte(REG_TX_HSH,SyncEmbTable[i].RegHSH); // Reg97
        HDMITX_WriteI2C_Byte(REG_TX_VSS1,SyncEmbTable[i].RegVSS1); // RegA0
        HDMITX_WriteI2C_Byte(REG_TX_VSE1,SyncEmbTable[i].RegVSE1); // RegA1

        HDMITX_WriteI2C_Byte(REG_TX_VSS2,SyncEmbTable[i].RegVSS2); // RegA2
        HDMITX_WriteI2C_Byte(REG_TX_VSE2,SyncEmbTable[i].RegVSE2); // RegA3
    }

    return TRUE ;
}
#endif // SUPPORT_SYNCEMBEDDED

//~jj_tseng@chipadvanced.com 2007/01/02


//////////////////////////////////////////////////////////////////////
// Function: SetInputMode
// Parameter: InputMode,bInputSignalType
//      InputMode - use [1:0] to identify the color space for reg70[7:6],
//                  definition:
//                     #define F_MODE_RGB444  0
//                     #define F_MODE_YUV422 1
//                     #define F_MODE_YUV444 2
//                     #define F_MODE_CLRMOD_MASK 3
//      bInputSignalType - defined the CCIR656 D[0],SYNC Embedded D[1],and
//                     DDR input in D[2].
// Return: N/A
// Remark: program Reg70 with the input value.
// Side-Effect: Reg70.
//////////////////////////////////////////////////////////////////////

static void
SetInputMode(BYTE InputMode,BYTE bInputSignalType)
{
    BYTE ucData ;

//    HDMITX_DEBUG_PRINTF(("SetInputMode(%02X,%02X)\n",InputMode,bInputSignalType));

    ucData = HDMITX_ReadI2C_Byte(REG_TX_INPUT_MODE);

    ucData &= ~(M_INCOLMOD|B_2X656CLK|B_SYNCEMB|B_INDDR|B_PCLKDIV2);

    switch(InputMode & F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV422:
        ucData |= B_IN_YUV422 ;
        break ;
    case F_MODE_YUV444:
        ucData |= B_IN_YUV444 ;
        break ;
    case F_MODE_RGB444:
    default:
        ucData |= B_IN_RGB ;
        break ;
    }

    if(bInputSignalType & T_MODE_PCLKDIV2)
    {
        ucData |= B_PCLKDIV2 ; HDMITX_DEBUG_PRINTF("PCLK Divided by 2 mode\n");
    }
    if(bInputSignalType & T_MODE_CCIR656)
    {
        ucData |= B_2X656CLK ; HDMITX_DEBUG_PRINTF("CCIR656 mode\n");
    }

    if(bInputSignalType & T_MODE_SYNCEMB)
    {
        ucData |= B_SYNCEMB ; HDMITX_DEBUG_PRINTF("Sync Embedded mode\n");
    }

    if(bInputSignalType & T_MODE_INDDR)
    {
        ucData |= B_INDDR ; HDMITX_DEBUG_PRINTF("Input DDR mode\n");
    }

    HDMITX_WriteI2C_Byte(REG_TX_INPUT_MODE,ucData);
}

//////////////////////////////////////////////////////////////////////
// Function: SetCSCScale
// Parameter: bInputMode -
//             D[1:0] - Color Mode
//             D[4] - Colorimetry 0: ITU_BT601 1: ITU_BT709
//             D[5] - Quantization 0: 0_255 1: 16_235
//             D[6] - Up/Dn Filter 'Required'
//                    0: no up/down filter
//                    1: enable up/down filter when csc need.
//             D[7] - Dither Filter 'Required'
//                    0: no dither enabled.
//                    1: enable dither and dither free go "when required".
//            bOutputMode -
//             D[1:0] - Color mode.
// Return: N/A
// Remark: reg72~reg8D will be programmed depended the input with table.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
SetCSCScale(BYTE bInputMode,BYTE bOutputMode)
{
    BYTE ucData,csc ;
    BYTE filter = 0 ; // filter is for Video CTRL DN_FREE_GO,EN_DITHER,and ENUDFILT


    // (1) YUV422 in,RGB/YUV444 output (Output is 8-bit,input is 12-bit)
    // (2) YUV444/422  in,RGB output (CSC enable,and output is not YUV422)
    // (3) RGB in,YUV444 output   (CSC enable,and output is not YUV422)
    //
    // YUV444/RGB24 <-> YUV422 need set up/down filter.

    switch(bInputMode&F_MODE_CLRMOD_MASK)
    {
    #ifdef SUPPORT_INPUTYUV444
    case F_MODE_YUV444:
        HDMITX_DEBUG_PRINTF("Input mode is YUV444 ");
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF("Output mode is YUV444\n");
            csc = B_HDMITX_CSC_BYPASS ;
            break ;

        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF("Output mode is YUV422\n");
            if(bInputMode & F_MODE_EN_UDFILT) // YUV444 to YUV422 need up/down filter for processing.
            {
                filter |= B_TX_EN_UDFILTER ;
            }
            csc = B_HDMITX_CSC_BYPASS ;
            break ;
        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF("Output mode is RGB24\n");
            csc = B_HDMITX_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_DITHER) // YUV444 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTYUV422
    case F_MODE_YUV422:
        HDMITX_DEBUG_PRINTF("Input mode is YUV422\n");
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF("Output mode is YUV444\n");
            csc = B_HDMITX_CSC_BYPASS ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to YUV444 need up filter
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF("Output mode is YUV422\n");
            csc = B_HDMITX_CSC_BYPASS ;

            break ;

        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF("Output mode is RGB24\n");
            csc = B_HDMITX_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to RGB24 need up/dn filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTRGB
    case F_MODE_RGB444:
        HDMITX_DEBUG_PRINTF("Input mode is RGB24\n");
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            HDMITX_DEBUG_PRINTF("Output mode is YUV444\n");
            csc = B_HDMITX_CSC_RGB2YUV ;

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            break ;

        case F_MODE_YUV422:
            HDMITX_DEBUG_PRINTF("Output mode is YUV422\n");
            if(bInputMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need down filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            csc = B_HDMITX_CSC_RGB2YUV ;
            break ;

        case F_MODE_RGB444:
            HDMITX_DEBUG_PRINTF("Output mode is RGB24\n");
            csc = B_HDMITX_CSC_BYPASS ;
            break ;
        }
        break ;
    #endif
    }

#ifndef DISABLE_TX_CSC

    #ifdef SUPPORT_INPUTRGB
    // set the CSC metrix registers by colorimetry and quantization
    if(csc == B_HDMITX_CSC_RGB2YUV)
    {
        HDMITX_DEBUG_PRINTF("CSC = RGB2YUV %x ",csc);
        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF("ITU709 16-235 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            HDMITX_DEBUG_PRINTF("ITU709 0-255 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_0_255,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF("ITU601 16-235 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            HDMITX_DEBUG_PRINTF("ITU601 0-255 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_0_255,SIZEOF_CSCMTX);
            break ;
        }

    }
    #endif

    #ifdef SUPPORT_INPUTYUV
    if (csc == B_HDMITX_CSC_YUV2RGB)
    {
        HDMITX_DEBUG_PRINTF("CSC = YUV2RGB %x ",csc);

        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF("ITU709 16-235 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            HDMITX_DEBUG_PRINTF("ITU709 0-255 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_0_255,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            HDMITX_DEBUG_PRINTF("ITU601 16-235 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_16_235,SIZEOF_CSCMTX);
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            HDMITX_DEBUG_PRINTF("ITU601 0-255 ");
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET);
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_0_255,SIZEOF_CSCMTX);
            break ;
        }
    }
    #endif
#else// DISABLE_TX_CSC
    csc = B_HDMITX_CSC_BYPASS ;
#endif// DISABLE_TX_CSC

    ucData = HDMITX_ReadI2C_Byte(REG_TX_CSC_CTRL) & ~(M_CSC_SEL|B_TX_DNFREE_GO|B_TX_EN_DITHER|B_TX_EN_UDFILTER);
    ucData |= filter|csc ;

    HDMITX_WriteI2C_Byte(REG_TX_CSC_CTRL,ucData);

    // set output Up/Down Filter,Dither control

}


//////////////////////////////////////////////////////////////////////
// Function: SetupAFE
// Parameter: VIDEOPCLKLEVEL level
//            PCLK_LOW - for 13.5MHz (for mode less than 1080p)
//            PCLK MEDIUM - for 25MHz~74MHz
//            PCLK HIGH - PCLK > 80Hz (for 1080p mode or above)
// Return: N/A
// Remark: set reg62~reg65 depended on HighFreqMode
//         reg61 have to be programmed at last and after video stable input.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
// SetupAFE(BYTE ucFreqInMHz)
SetupAFE(VIDEOPCLKLEVEL level)
{
    BYTE uc ;
    // @emily turn off reg61 before SetupAFE parameters.
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST);/* 0x10 */
    // HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,0x3);
    HDMITX_DEBUG_PRINTF("SetupAFE()\n");

    //TMDS Clock < 80MHz    TMDS Clock > 80MHz
    //Reg61    0x03    0x03

    //Reg62    0x18    0x88
    //Reg63    Default    Default
    //Reg64    0x08    0x80
    //Reg65    Default    Default
    //Reg66    Default    Default
    //Reg67    Default    Default
    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1);

#ifdef FALLING_EDGE_TRIGGER
    uc |= B_VDO_LATCH_EDGE ;
#else
    uc &= ~B_VDO_LATCH_EDGE ;
#endif
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc);
    switch(level)
    {
    case PCLK_HIGH:
        HDMITX_WriteI2C_Byte(REG_TX_AFE_XP_CTRL,0x88); // reg62
        //Bruce120409. To pass HDMI pre-test 7-10. We have to change from 0x10 to 0x28.
        //HDMITX_WriteI2C_Byte(REG_TX_AFE_ISW_CTRL, 0x10); // reg63
        HDMITX_WriteI2C_Byte(REG_TX_AFE_ISW_CTRL, 0x28); // reg63
        HDMITX_WriteI2C_Byte(REG_TX_AFE_IP_CTRL,0x84); // reg64
        break ;
    default:
        HDMITX_WriteI2C_Byte(REG_TX_AFE_XP_CTRL,0x18); // reg62
        HDMITX_WriteI2C_Byte(REG_TX_AFE_ISW_CTRL, 0x10); // reg63
        HDMITX_WriteI2C_Byte(REG_TX_AFE_IP_CTRL,0x0C); // reg64
        break ;
    }
    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST);
    uc &= ~(B_REF_RST_HDMITX|B_VID_RST_HDMITX);
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc);
    DelayMS(1);
    // HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc|B_VID_RST_HDMITX);
    // DelayMS(100);
    // HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc);
    //~Jau-Chih.Tseng@ite.com.tw


}


//////////////////////////////////////////////////////////////////////
// Function: FireAFE
// Parameter: N/A
// Return: N/A
// Remark: write reg61 with 0x04
//         When program reg61 with 0x04,then audio and video circuit work.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////
static void
FireAFE()
{
    BYTE reg;
    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,0);

    for(reg = 0x61 ; reg <= 0x67 ; reg++)
    {
//        HDMITX_DEBUG_PRINTF(("Reg[%02X] = %02X\n",(int)reg,(int)HDMITX_ReadI2C_Byte(reg)));
    }
}

//////////////////////////////////////////////////////////////////////
// Audio Output
//////////////////////////////////////////////////////////////////////
#if !defined(CODE202)
//////////////////////////////////////////////////////////////////////
// Function: SetAudioFormat
// Parameter:
//    NumChannel - number of channel,from 1 to 8
//    AudioEnable - Audio source and type bit field,value of bit field are
//        ENABLE_SPDIF    (1<<4)
//        ENABLE_I2S_SRC3  (1<<3)
//        ENABLE_I2S_SRC2  (1<<2)
//        ENABLE_I2S_SRC1  (1<<1)
//        ENABLE_I2S_SRC0  (1<<0)
//    SampleFreq - the audio sample frequence in Hz
//    AudSWL - Audio sample width,only support 16,18,20,or 24.
//    AudioCatCode - The audio channel catalogy code defined in IEC 60958-3
// Return: ER_SUCCESS if done,ER_FAIL for otherwise.
// Remark: program audio channel control register and audio channel registers
//         to enable audio by input.
// Side-Effect: register bank will keep in bank zero.
//////////////////////////////////////////////////////////////////////


static SYS_STATUS
SetAudioFormat(BYTE NumChannel,BYTE AudioEnable,BYTE bSampleFreq,BYTE AudSWL,BYTE AudioCatCode)
{
    BYTE fs = bSampleFreq ;
    BYTE SWL ;

    BYTE SourceValid ;
    BYTE SoruceNum ;


//    HDMITX_DEBUG_PRINTF(("SetAudioFormat(%d channel,%02X,SampleFreq %d,AudSWL %d,%02X)\n",(int)NumChannel,(int)AudioEnable,(int)bSampleFreq,(int)AudSWL,(int)AudioCatCode));


    Instance[0].bOutputAudioMode |= 0x41 ;
    if(NumChannel > 6)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S3VALID | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 4 ;
    }
    else if (NumChannel > 4)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 3 ;
    }
    else if (NumChannel > 2)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S1VALID ;
        SoruceNum = 2 ;
    }
    else
    {
        SourceValid = B_AUD_ERR2FLAT ; // only two channel.
        SoruceNum = 1 ;
        Instance[0].bOutputAudioMode &= ~0x40 ;
    }

    AudioEnable &= ~ (M_AUD_SWL|B_SPDIFTC);

    switch(AudSWL)
    {
    case 16:
        SWL = AUD_SWL_16 ;
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        SWL = AUD_SWL_18 ;
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        SWL = AUD_SWL_20 ;
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
        SWL = AUD_SWL_24 ;
        AudioEnable |= M_AUD_24BIT ;
        break ;
    default:
        return ER_FAIL ;
    }


    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0);

    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,Instance[0].bOutputAudioMode); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,(Instance[0].bAudioChannelSwap&0xF)|(AudioEnable&B_AUD_SPDIF));
    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,SourceValid);

    // suggested to be 0x41

//     Switch_HDMITX_Bank(1);
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)); // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,AudioCatCode);
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,SoruceNum);
//     HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,0x21);
//     HDMITX_WriteI2C_Byte(REG_TX_AUD1CHST_CHTNUM,0x43);
//     HDMITX_WriteI2C_Byte(REG_TX_AUD2CHST_CHTNUM,0x65);
//     HDMITX_WriteI2C_Byte(REG_TX_AUD3CHST_CHTNUM,0x87);
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs); // choose clock
//     fs = ~fs ; // OFS is the one's complement of FS
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL);
//     Switch_HDMITX_Bank(0);

    Switch_HDMITX_Bank(1);
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)); // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,AudioCatCode);
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,SoruceNum);
    HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,0);
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs); // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL);
    Switch_HDMITX_Bank(0);

    if(!(AudioEnable | B_AUD_SPDIF))
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable);
    }

    Instance[0].bAudioChannelEnable = AudioEnable ;

    // HDMITX_AndREG_Byte(REG_TX_SW_RST,B_AUD_RST_HDMITX);    // enable Audio
    return ER_SUCCESS;
}
#endif


#if defined(CODE202)
static void
AutoAdjustAudio()
{
    unsigned long SampleFreq,cTMDSClock ;
    unsigned long N ;
    ULONG aCTS=0;
    BYTE fs, uc,LoopCnt=10;
#ifndef CONFIG_AST1500_CAT6613
    if(bForceCTS)
    {
        Switch_HDMITX_Bank(0);
        HDMITX_WriteI2C_Byte(0xF8, 0xC3) ;
        HDMITX_WriteI2C_Byte(0xF8, 0xA5) ;
        HDMITX_AndREG_Byte(REG_TX_PKT_SINGLE_CTRL,~B_SW_CTS) ; // D[1] = 0, HW auto count CTS
        HDMITX_WriteI2C_Byte(0xF8, 0xFF) ;
    }
#endif
    //delay1ms(50);
    Switch_HDMITX_Bank(1);
    N = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN2)&0xF) << 16 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN1)) <<8 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN0));

    while(LoopCnt--)
    {   ULONG TempCTS=0;
        aCTS = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt2)) << 12 ;
        aCTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt1)) <<4 ;
        aCTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt0)&0xf0)>>4  ;
        if(aCTS==TempCTS)
        {break;}
        TempCTS=aCTS;
    }
    Switch_HDMITX_Bank(0);
    if( aCTS == 0)
    {
//        HDMITX_DEBUG_PRINTF(("aCTS== 0"));
        return;
    }

    uc = HDMITX_ReadI2C_Byte(0xc1);

    cTMDSClock = Instance[0].TMDSClock ;
    //TMDSClock=GetInputPclk();
//    HDMITX_DEBUG_PRINTF(("PCLK = %u0,000\n",(WORD)(cTMDSClock/10000)));
    switch(uc & 0x70)
    {
    case 0x50:
        cTMDSClock *= 5 ;
        cTMDSClock /= 4 ;
        break ;
    case 0x60:
        cTMDSClock *= 3 ;
        cTMDSClock /= 2 ;
    }
    SampleFreq = cTMDSClock/aCTS ;
    SampleFreq *= N ;
    SampleFreq /= 128 ;
    //SampleFreq=48000;

//    HDMITX_DEBUG_PRINTF(("SampleFreq = %u0\n",(WORD)(SampleFreq/10)));
    if( SampleFreq>31000L && SampleFreq<=38050L ){fs = AUDFS_32KHz ;}
    else if (SampleFreq < 46550L )  {fs = AUDFS_44p1KHz ;}//46050
    else if (SampleFreq < 68100L )  {fs = AUDFS_48KHz ;}
    else if (SampleFreq < 92100L )  {fs = AUDFS_88p2KHz ;}
    else if (SampleFreq < 136200L ) {fs = AUDFS_96KHz ;}
    else if (SampleFreq < 184200L ) {fs = AUDFS_176p4KHz ;}
    else if (SampleFreq < 240200L ) {fs = AUDFS_192KHz ;}
    else if (SampleFreq < 800000L ) {fs = AUDFS_768KHz ;}
    else
    {
        fs = AUDFS_OTHER;
#ifdef Debug_message
        HDMITX_DEBUG_PRINTF(("fs = AUDFS_OTHER\n"));
#endif
    }
    if(Instance[0].bAudFs != fs)
    {
        Instance[0].bAudFs=fs;
        SetNCTS(Instance[0].TMDSClock,Instance[0].bAudFs); // set N, CTS by new generated clock.
        //CurrCTS=0;
        return;
    }
    return;
}
#else
static void
AutoAdjustAudio()
{
    unsigned long SampleFreq,TMDSClock ;
    unsigned long N ;
    unsigned long CTS ;
    BYTE fs, uc ;

//    bPendingAdjustAudioFreq = TRUE ;

//     if( CAT6611_AudioChannelEnable & B_AUD_SPDIF )
//     {
//         if(!(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK))
//         {
//             return ;
//         }
//     }

    Switch_HDMITX_Bank(1);
    N = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN2)&0xF) << 16 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN1)) <<8 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN0));

    CTS = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt2)) << 12 ;
    CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt1)) <<4 ;
    CTS |= (((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt0))>>4)&0xF ;
    Switch_HDMITX_Bank(0);

    // CTS = TMDSCLK * N / ( 128 * SampleFreq )
    // SampleFreq = TMDSCLK * N / (128*CTS)

    if( CTS == 0 )
    {
        return  ;
    }

    uc = HDMITX_ReadI2C_Byte(0xc1);

    TMDSClock = Instance[0].TMDSClock ;

    switch(uc & 0x70)
    {
    case 0x50:
        TMDSClock *= 5 ;
        TMDSClock /= 4 ;
        break ;
    case 0x60:
        TMDSClock *= 3 ;
        TMDSClock /= 2 ;
    }

    SampleFreq = TMDSClock/CTS ;
    SampleFreq *= N ;
    SampleFreq /= 128 ;

    if( SampleFreq>31000 && SampleFreq<=38050 )
    {
        Instance[0].bAudFs = AUDFS_32KHz ;
        fs = AUDFS_32KHz ;;
    }
    else if (SampleFreq < 46050 ) // 44.1KHz
    {
        Instance[0].bAudFs = AUDFS_44p1KHz ;
        fs = AUDFS_44p1KHz ;;
    }
    else if (SampleFreq < 68100 ) // 48KHz
    {
        Instance[0].bAudFs = AUDFS_48KHz ;
        fs = AUDFS_48KHz ;;
    }
    else if (SampleFreq < 92100 ) // 88.2 KHz
    {
        Instance[0].bAudFs = AUDFS_88p2KHz ;
        fs = AUDFS_88p2KHz ;;
    }
    else if (SampleFreq < 136200 ) // 96KHz
    {
        Instance[0].bAudFs = AUDFS_96KHz ;
        fs = AUDFS_96KHz ;;
    }
    else if (SampleFreq < 184200 ) // 176.4KHz
    {
        Instance[0].bAudFs = AUDFS_176p4KHz ;
        fs = AUDFS_176p4KHz ;;
    }
    else if (SampleFreq < 240200 ) // 192KHz
    {
        Instance[0].bAudFs = AUDFS_192KHz ;
        fs = AUDFS_192KHz ;;
    }
    else
    {
        Instance[0].bAudFs = AUDFS_OTHER;
        fs = AUDFS_OTHER;;
    }

//    bPendingAdjustAudioFreq = FALSE ;

    SetNCTS(Instance[0].TMDSClock, Instance[0].bAudFs); // set N, CTS by new generated clock.

    Switch_HDMITX_Bank(1); // adjust the new fs in channel status registers
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs); // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_OFS_WL);
    uc &= 0xF ;
    uc |= fs << 4 ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,uc);

    Switch_HDMITX_Bank(0);

}
#endif

#ifdef CONFIG_AST1500_CAT6613
void
SetupAudioChannel(void)
#else
static void
SetupAudioChannel()
#endif
{
    static BYTE bEnableAudioChannel=FALSE ;
    BYTE uc ;
    if( (HDMITX_ReadI2C_Byte(REG_TX_SW_RST) & (B_AUD_RST_HDMITX|B_AREF_RST)) == 0) // audio enabled
    {
        Switch_HDMITX_Bank(0);
        uc =HDMITX_ReadI2C_Byte(REG_TX_AUDIO_CTRL0);
        if((uc & 0x1f) == 0x10)
        {//SPDIF
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                SetNCTS(Instance[0].TMDSClock, Instance[0].bAudFs); // to enable automatic progress setting for N/CTS
                DelayMS(5);
                AutoAdjustAudio();
                Switch_HDMITX_Bank(0);
                HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable);
                bEnableAudioChannel=TRUE ;
            }
        }
        else if((uc & 0xF) == 0x00 )
        {//I2S
            SetNCTS(Instance[0].TMDSClock, Instance[0].bAudFs); // to enable automatic progress setting for N/CTS
            DelayMS(5);
            AutoAdjustAudio();
            Switch_HDMITX_Bank(0);
            HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable);
            bEnableAudioChannel=TRUE ;
        }
        else
        {
            /*
            if((HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)==0)
            {
                // AutoAdjustAudio();
                // ForceSetNCTS(CurrentPCLK, CurrentSampleFreq);
                if( bEnableAudioChannel == TRUE )
                {
                    Switch_HDMITX_Bank(0);
                    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, Instance[0].bAudioChannelEnable&0xF0);
                }
                bEnableAudioChannel=FALSE ;
            }
            */
        }
    }
}
//////////////////////////////////////////////////////////////////////
// Function: SetNCTS
// Parameter: PCLK - video clock in Hz.
//            Fs - Encoded audio sample rate
//                          AUDFS_22p05KHz  4
//                          AUDFS_44p1KHz 0
//                          AUDFS_88p2KHz 8
//                          AUDFS_176p4KHz    12
//
//                          AUDFS_24KHz  6
//                          AUDFS_48KHz  2
//                          AUDFS_96KHz  10
//                          AUDFS_192KHz 14
//
//                          AUDFS_768KHz 9
//
//                          AUDFS_32KHz  3
//                          AUDFS_OTHER    1

// Return: ER_SUCCESS if success
// Remark: set N value,the CTS will be auto generated by HW.
// Side-Effect: register bank will reset to bank 0.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetNCTS(ULONG PCLK,BYTE Fs)
#if defined(CODE202)//to do:check HBR
{
    ULONG n,MCLK,SampleFreq;
    BYTE LoopCnt=255,CTSStableCnt=0;
    ULONG diff;
    ULONG CTS=0,LastCTS=0;
    BOOL HBR_mode;
    BYTE aVIC;
    if(B_HBR & HDMITX_ReadI2C_Byte(REG_TX_AUD_HDAUDIO))
    {
        HBR_mode=TRUE;
    }
    else
    {
        HBR_mode=FALSE;
    }

    Switch_HDMITX_Bank(1);
    aVIC = (HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB4)&0x7f);
    Switch_HDMITX_Bank(0);

    if(aVIC)
    {
        switch(Fs)
        {
        case AUDFS_32KHz: n = 4096; break;
        case AUDFS_44p1KHz: n = 6272; break;
        case AUDFS_48KHz: n = 6144; break;
        case AUDFS_88p2KHz: n = 12544; break;
        case AUDFS_96KHz: n = 12288; break;
        case AUDFS_176p4KHz: n = 25088; break;
        case AUDFS_192KHz: n = 24576; break;
        default: n = 6144;
        }
    }
    else
    {
        switch(Fs)
        {
            case AUDFS_32KHz: SampleFreq = 32000L; break;
            case AUDFS_44p1KHz: SampleFreq = 44100L; break;
            case AUDFS_48KHz: SampleFreq = 48000L; break;
            case AUDFS_88p2KHz: SampleFreq = 88200L; break;
            case AUDFS_96KHz: SampleFreq = 96000L; break;
            case AUDFS_176p4KHz: SampleFreq = 176000L; break;
            case AUDFS_192KHz: SampleFreq = 192000L; break;
            default: SampleFreq = 768000L;
        }
        MCLK = SampleFreq * 256 ; // MCLK = fs * 256 ;
        n = MCLK / 2000;
    }
    // tr_printf((" n = %ld\n",n)) ;
    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF)) ;
#ifndef CONFIG_AST1500_CAT6613
    if(bForceCTS)
    {
        ULONG SumCTS=0;
        while(LoopCnt--)
        {
            delay1ms(30);
            CTS = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt2)) << 12 ;
            CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt1)) <<4 ;
            CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt0)&0xf0)>>4  ;
            if( CTS == 0)
            {
                continue;
            }
            else
            {
                if(LastCTS>CTS )
                    {diff=LastCTS-CTS;}
                else
                    {diff=CTS-LastCTS;}
                //HDMITX_DEBUG_PRINTF(("LastCTS= %u%u",(WORD)(LastCTS/10000),(WORD)(LastCTS%10000)));
                //HDMITX_DEBUG_PRINTF(("       CTS= %u%u\n",(WORD)(CTS/10000),(WORD)(CTS%10000)));
                LastCTS=CTS;
                if(5>diff)
                {
                    CTSStableCnt++;
                    SumCTS+=CTS;
                }
                else
                {
                    CTSStableCnt=0;
                    SumCTS=0;
                    continue;
                }
                if(CTSStableCnt>=32)
                {
                    LastCTS=(SumCTS>>5);
                    break;
                }
            }
        }
    }
    HDMITX_WriteI2C_Byte(REGPktAudCTS0,(BYTE)((LastCTS)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudCTS1,(BYTE)((LastCTS>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudCTS2,(BYTE)((LastCTS>>16)&0xF)) ;
#endif
    Switch_HDMITX_Bank(0) ;
#ifdef Force_CTS
    bForceCTS = TRUE;
#endif
    HDMITX_WriteI2C_Byte(0xF8, 0xC3) ;
    HDMITX_WriteI2C_Byte(0xF8, 0xA5) ;
#ifndef CONFIG_AST1500_CAT6613
    if(bForceCTS)
    {
        HDMITX_OrREG_Byte(REG_TX_PKT_SINGLE_CTRL,B_SW_CTS) ; // D[1] = 0, HW auto count CTS
    }
    else
#endif
    {
        HDMITX_AndREG_Byte(REG_TX_PKT_SINGLE_CTRL,~B_SW_CTS) ; // D[1] = 0, HW auto count CTS
    }
    HDMITX_WriteI2C_Byte(0xF8, 0xFF) ;

#ifndef CONFIG_AST1500_CAT6613//steven
    if(FALSE==HBR_mode) //LPCM
    {
        BYTE uData;
        Switch_HDMITX_Bank(1);
        HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|Fs);
        Fs = ~Fs ; // OFS is the one's complement of FS
        uData = (0x0f&HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_OFS_WL));
        HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,(Fs<<4)|uData);
        Switch_HDMITX_Bank(0);
    }
#endif

    return ER_SUCCESS ;
}
#else
{
    ULONG n,MCLK ;

    MCLK = Fs * 256 ; // MCLK = fs * 256 ;

//    HDMITX_DEBUG_PRINTF(("SetNCTS(%ld,%ld): MCLK = %ld\n",PCLK,(int)Fs,MCLK));

    if( PCLK )
    {
        switch (Fs) {
            case AUDFS_32KHz:
                switch (PCLK) {
                    case 74175000: n = 11648; break;
                    case 14835000: n = 11648; break;
                    default: n = 4096;
                }
                break;
            case AUDFS_44p1KHz:
                switch (PCLK) {
                    case 74175000: n = 17836; break;
                    case 14835000: n = 8918; break;
                    default: n = 6272;
                }
                break;
            case AUDFS_48KHz:
                switch (PCLK) {
                    case 74175000: n = 11648; break;
                    case 14835000: n = 5824; break;
                    default: n = 6144;
                }
                break;
            case AUDFS_88p2KHz:
                switch (PCLK) {
                    case 74175000: n = 35672; break;
                    case 14835000: n = 17836; break;
                    default: n = 12544;
                }
                break;
            case AUDFS_96KHz:
                switch (PCLK) {
                    case 74175000: n = 23296; break;
                    case 14835000: n = 11648; break;
                    default: n = 12288;
                }
                break;
            case AUDFS_176p4KHz:
                switch (PCLK) {
                    case 74175000: n = 71344; break;
                    case 14835000: n = 35672; break;
                    default: n = 25088;
                }
                break;
            case AUDFS_192KHz:
                switch (PCLK) {
                    case 74175000: n = 46592; break;
                    case 14835000: n = 23296; break;
                    default: n = 24576;
                }
                break;
            default: n = MCLK / 2000;
        }
    }
    else
    {
        switch(Fs)
        {
        case AUDFS_32KHz: n = 4096; break;
        case AUDFS_44p1KHz: n = 6272; break;
        case AUDFS_48KHz: n = 6144; break;
        case AUDFS_88p2KHz: n = 12544; break;
        case AUDFS_96KHz: n = 12288; break;
        case AUDFS_176p4KHz: n = 25088; break;
        case AUDFS_192KHz: n = 24576; break;
        default: n = 6144;
        }

    }


//    HDMITX_DEBUG_PRINTF(("N = %ld\n",n));
    Switch_HDMITX_Bank(1);
#if 0
    HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF));
    HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF));
    HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF));
#else
    //HBR must use this setting
    if (Instance[0].outputparam.Audio_Type & F_AUDIO_HBR) {
        HDMITX_WriteI2C_Byte(REGPktAudN0,0x00) ; //for HBR
        HDMITX_WriteI2C_Byte(REGPktAudN1,0x60) ;
        HDMITX_WriteI2C_Byte(REGPktAudN2,0x00) ;
    } else {
        HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF)) ;
        HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF)) ;
        HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF)) ;
    }
#endif
    Switch_HDMITX_Bank(0);

    HDMITX_WriteI2C_Byte(REG_TX_PKT_SINGLE_CTRL,0); // D[1] = 0,HW auto count CTS

    HDMITX_SetREG_Byte(REG_TX_CLK_CTRL0,~M_EXT_MCLK_SEL,B_EXT_256FS);
    return ER_SUCCESS ;
}
#endif

//////////////////////////////////////////////////////////////////////
// DDC Function.
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Function: ClearDDCFIFO
// Parameter: N/A
// Return: N/A
// Remark: clear the DDC FIFO.
// Side-Effect: DDC master will set to be HOST.
//////////////////////////////////////////////////////////////////////

static void
ClearDDCFIFO()
{
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_FIFO_CLR);
}

static void
GenerateDDCSCLK()
{
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_GEN_SCLCLK);
}
//////////////////////////////////////////////////////////////////////
// Function: AbortDDC
// Parameter: N/A
// Return: N/A
// Remark: Force abort DDC and reset DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
AbortDDC()
{
    BYTE CPDesire,SWReset,DDCMaster ;
    BYTE uc, timeout, i ;
    // save the SW reset,DDC master,and CP Desire setting.
#if defined(CODE202)
    HDMITX_OrReg_Byte(REG_TX_INT_CTRL,(1<<1));
#endif
    SWReset = HDMITX_ReadI2C_Byte(REG_TX_SW_RST);
    CPDesire = HDMITX_ReadI2C_Byte(REG_TX_HDCP_DESIRE);
    DDCMaster = HDMITX_ReadI2C_Byte(REG_TX_DDC_MASTER_CTRL);


    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,CPDesire&(~B_CPDESIRE)); // @emily change order
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,SWReset|B_HDCP_RST_HDMITX);         // @emily change order
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);

    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    // do abort DDC twice.
    for( i = 0 ; i < 2 ; i++ )
    {
        HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_ABORT);

        for( timeout = 0 ; timeout < 200 ; timeout++ )
        {
            uc = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
            if (uc&B_DDC_DONE)
            {
                break ; // success
            }

            if( uc & (B_DDC_NOACK|B_DDC_WAITBUS|B_DDC_ARBILOSE) )
            {
//                HDMITX_DEBUG_PRINTF(("AbortDDC Fail by reg16=%02X\n",(int)uc));
                break ;
            }
            DelayMS(1); // delay 1 ms to stable.
        }
    }
    //~Jau-Chih.Tseng@ite.com.tw


    // 2009/01/15 modified by Jau-Chih.Tseng@ite.com.tw
    //// restore the SW reset,DDC master,and CP Desire setting.
    //HDMITX_WriteI2C_Byte(REG_TX_SW_RST,SWReset);
    //HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,CPDesire);
    //HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,DDCMaster);
    //~Jau-Chih.Tseng@ite.com.tw
#if defined(CODE202)
    HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
#endif
}

//////////////////////////////////////////////////////////////////////
// Packet and InfoFrame
//////////////////////////////////////////////////////////////////////

// ////////////////////////////////////////////////////////////////////////////////
// // Function: SetAVMute()
// // Parameter: N/A
// // Return: N/A
// // Remark: set AVMute as TRUE and enable GCP sending.
// // Side-Effect: N/A
// ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// void
// SetAVMute()
// {
//     Switch_HDMITX_Bank(0);
//     HDMITX_WriteI2C_Byte(REG_TX_GCP,B_SET_AVMUTE);
//     HDMITX_WriteI2C_Byte(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
// }

// ////////////////////////////////////////////////////////////////////////////////
// // Function: SetAVMute(FALSE)
// // Parameter: N/A
// // Return: N/A
// // Remark: clear AVMute as TRUE and enable GCP sending.
// // Side-Effect: N/A
// ////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// void
// SetAVMute(FALSE)
// {
//     Switch_HDMITX_Bank(0);
//     HDMITX_WriteI2C_Byte(REG_TX_GCP,B_CLR_AVMUTE);
//     HDMITX_WriteI2C_Byte(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
// }



//////////////////////////////////////////////////////////////////////
// Function: ReadEDID
// Parameter: pData - the pointer of buffer to receive EDID ucdata.
//            bSegment - the segment of EDID readback.
//            offset - the offset of EDID ucdata in the segment. in byte.
//            count - the read back bytes count,cannot exceed 32
// Return: ER_SUCCESS if successfully getting EDID. ER_FAIL otherwise.
// Remark: function for read EDID ucdata from reciever.
// Side-Effect: DDC master will set to be HOST. DDC FIFO will be used and dirty.
//////////////////////////////////////////////////////////////////////

SYS_STATUS
ReadEDID(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count)
{
    SHORT RemainedCount,ReqCount ;
    BYTE bCurrOffset ;
    SHORT TimeOut ;
    BYTE *pBuff = pData ;
    BYTE ucdata ;
#ifdef CONFIG_AST1500_CAT6613
    unsigned int retry_cnt = 3;
#endif

    // HDMITX_DEBUG_PRINTF(("ReadEDID(%08lX,%d,%d,%d)\n",(ULONG)pData,(int)bSegment,(int)offset,(int)Count));
    if(!pData)
    {
//        HDMITX_DEBUG_PRINTF(("ReadEDID(): Invallid pData pointer %08lX\n",(ULONG)pData));
        return ER_FAIL ;
    }

#ifdef CONFIG_AST1500_CAT6613
    Switch_HDMITX_Bank(0);

    LowerDDCClock();
#endif

    if(HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG)
    {
        HDMITX_DEBUG_PRINTF("Called AboutDDC()\n");
        AbortDDC();

    }
#if defined(CODE202)
    HDMITX_OrReg_Byte(REG_TX_INT_CTRL,(1<<1));
#endif

#ifdef CONFIG_AST1500_CAT6613
    //Bruce100811. Unnecessary because it is done in next while()
    //ClearDDCFIFO();
#else
    ClearDDCFIFO();
#endif

    RemainedCount = Count ;
    bCurrOffset = offset ;

    while(RemainedCount > 0)
    {

        ReqCount = (RemainedCount > DDC_FIFO_MAXREQ)?DDC_FIFO_MAXREQ:RemainedCount ;
        // HDMITX_DEBUG_PRINTF(("ReadEDID(): ReqCount = %d,bCurrOffset = %d\n",(int)ReqCount,(int)bCurrOffset));
#ifdef CONFIG_AST1500_CAT6613
again:
#endif
        HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
        HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_FIFO_CLR);

#if defined(CODE202)
        for(TimeOut = 0 ; TimeOut < 200 ; TimeOut++)
#else
        for(TimeOut = 200 ; TimeOut > 0 ; TimeOut--)
#endif
        {
#ifdef CONFIG_AST1500_CAT6613
            DelayMS(1);
#endif
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);

            if(ucdata&B_DDC_DONE)
            {
                break ;
            }

            if((ucdata & B_DDC_ERROR)||(HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG))
            {
                HDMITX_DEBUG_PRINTF("Called AboutDDC()\n");
                AbortDDC();
                goto fail_out;
            }
        }
#ifdef CONFIG_AST1500_CAT6613
#if defined(CODE202)
        if (TimeOut == 200)
#else
        if (TimeOut == 0)
#endif
        {
            HDMITX_DEBUG_PRINTF("ReadEDID(): DDC TimeOut 1. \n");
            goto fail_out;
        }
#endif

        HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
        HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_EDID_ADDRESS); // for EDID ucdata get
        HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,bCurrOffset);
        HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,(BYTE)ReqCount);
        HDMITX_WriteI2C_Byte(REG_TX_DDC_EDIDSEG,bSegment);
        HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_EDID_READ);

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            DelayMS(1);
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
            if(ucdata & B_DDC_DONE)
            {
                break ;
            }

            if(ucdata & B_DDC_ERROR)
            {
                HDMITX_DEBUG_PRINTF("ReadEDID(): DDC_STATUS = %02X,fail.\n",(int)ucdata);
                AbortDDC();
                if (retry_cnt--)
                    goto again;
                else
                    goto fail_out;
            }
        }

        if(TimeOut == 0)
        {
            HDMITX_DEBUG_PRINTF("ReadEDID(): DDC TimeOut. \n");
            goto fail_out;
        }

        bCurrOffset += ReqCount ;
        RemainedCount -= ReqCount ;

        do
        {
            *(pBuff++) = HDMITX_ReadI2C_Byte(REG_TX_DDC_READFIFO);
            ReqCount -- ;
        }while(ReqCount > 0);

    }

#if defined(CODE202)
    HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
#endif
    NormalDDCClock();
    return ER_SUCCESS ;

fail_out:
#if defined(CODE202)
    HDMITX_AndReg_Byte(REG_TX_INT_CTRL,~(1<<1));
#endif
    NormalDDCClock();
    return ER_FAIL;
}



#ifdef SUPPORT_HDCP
//////////////////////////////////////////////////////////////////////
// Authentication
//////////////////////////////////////////////////////////////////////
static void
HDCP_ClearAuthInterrupt(void)
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_INT_MASK2) & (~(B_KSVLISTCHK_MASK|B_T_AUTH_DONE_MASK|B_AUTH_FAIL_MASK));
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL|B_CLR_AUTH_DONE|B_CLR_KSVLISTCHK);
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0);
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE);
}

static void
HDCP_ResetAuth(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0);
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,0);
    HDMITX_OrREG_Byte(REG_TX_SW_RST,B_HDCP_RST_HDMITX);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDCP_ClearAuthInterrupt();
    AbortDDC();
}
//////////////////////////////////////////////////////////////////////
// Function: HDCP_EnableEncryption
// Parameter: N/A
// Return: ER_SUCCESS if done.
// Remark: Set regC1 as zero to enable continue authentication.
// Side-Effect: register bank will reset to zero.
//////////////////////////////////////////////////////////////////////

#ifdef CONFIG_AST1500_CAT6613
SYS_STATUS HDCP_EnableEncryption(void)
#else
static SYS_STATUS
HDCP_EnableEncryption(void)
#endif
{
    Switch_HDMITX_Bank(0);
    return HDMITX_WriteI2C_Byte(REG_TX_ENCRYPTION,B_ENABLE_ENCRYPTION);
}

#ifdef CONFIG_AST1500_CAT6613
SYS_STATUS HDCP_DisableEncryption(void)
{
    Switch_HDMITX_Bank(0);
    return HDMITX_WriteI2C_Byte(REG_TX_ENCRYPTION,B_DISABLE_ENCRYPTION);
}
#endif

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Auth_Fire()
// Parameter: N/A
// Return: N/A
// Remark: write anything to reg21 to enable HDCP authentication by HW
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_Auth_Fire(void)
{
    // HDMITX_DEBUG_PRINTF(("HDCP_Auth_Fire():\n"));
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHDCP); // MASTERHDCP,no need command but fire.
    HDMITX_WriteI2C_Byte(REG_TX_AUTHFIRE,1);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StartAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Start the Cipher to free run for random number. When stop,An is
//         ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StartAnCipher(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_START_CIPHER_GEN);
    DelayMS(1); // delay 1 ms
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StopAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Stop the Cipher,and An is ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StopAnCipher(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GenerateAn
// Parameter: N/A
// Return: N/A
// Remark: start An ciper random run at first,then stop it. Software can get
//         an in reg30~reg38,the write to reg28~2F
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
HDCP_GenerateAn(void)
{
    BYTE Data[8] ;
#if 1
    HDCP_StartAnCipher();
    // HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_START_CIPHER_GEN);
    // DelayMS(1); // delay 1 ms
    // HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN);

    HDCP_StopAnCipher();

    Switch_HDMITX_Bank(0);
    // new An is ready in reg30
    HDMITX_ReadI2C_ByteN(REG_TX_AN_GEN,Data,8);
#else
    Data[0] = 0 ;Data[1] = 0 ;Data[2] = 0 ;Data[3] = 0 ;
    Data[4] = 0 ;Data[5] = 0 ;Data[6] = 0 ;Data[7] = 0 ;
#endif
    HDMITX_WriteI2C_ByteN(REG_TX_AN,Data,8);

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBCaps
// Parameter: pBCaps - pointer of byte to get BCaps.
//            pBStatus - pointer of two bytes to get BStatus
// Return: ER_SUCCESS if successfully got BCaps and BStatus.
// Remark: get B status and capability from HDCP reciever via DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x40); // BCaps offset
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,3);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1);

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            //HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC Done.\n"));
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
//            HDMITX_DEBUG_PRINTF(("HDCP_GetBCaps(): DDC fail by reg16=%02X.\n",ucdata));
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }
    ucdata = HDMITX_ReadI2C_Byte(REG_TX_BSTAT+1);
    *pBStatus = (USHORT)ucdata ;
    *pBStatus <<= 8 ;
    ucdata = HDMITX_ReadI2C_Byte(REG_TX_BSTAT);
    *pBStatus |= ((USHORT)ucdata&0xFF) ;
    *pBCaps = HDMITX_ReadI2C_Byte(REG_TX_BCAP);
    return ER_SUCCESS ;

}


//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBKSV
// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
// Return: ER_SUCCESS if successfuly got BKSV from Rx.
// Remark: Get BKSV from HDCP reciever.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBKSV(BYTE *pBKSV)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x00); // BKSV offset
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,5);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1);

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetBKSV(): DDC Done.\n");
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetBKSV(): DDC No ack or arbilose,%x,maybe cable did not connected. Fail.\n",ucdata);
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN(REG_TX_BKSV,(PBYTE)pBKSV,5);

    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function:HDCP_Authenticate
// Parameter: N/A
// Return: ER_SUCCESS if Authenticated without error.
// Remark: do Authentication with Rx
// Side-Effect:
//  1. Instance[0].bAuthenticated global variable will be TRUE when authenticated.
//  2. Auth_done interrupt and AUTH_FAIL interrupt will be enabled.
//////////////////////////////////////////////////////////////////////
static BYTE
countbit(BYTE b)
{
    BYTE i,count ;
    for( i = 0, count = 0 ; i < 8 ; i++ )
    {
        if( b & (1<<i) )
        {
            count++ ;
        }
    }
    return count ;
}

void
HDCP_Reset(void)
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) | B_HDCP_RST_HDMITX ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc);
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,0);
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    ClearDDCFIFO();
    AbortDDC();
}

static SYS_STATUS
HDCP_Authenticate(void)
{
    BYTE ucdata ;
    BYTE BCaps ;
    USHORT BStatus ;
    USHORT TimeOut ;

    BYTE revoked = FALSE ;
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
    unsigned char *BKSV = Instance[0].Bksv;
#else
    BYTE BKSV[5] ;
#endif
#else
	BYTE BKSV[5] ;
#endif

#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
    Instance[0].Bcaps = 0 ;
#endif
#endif

    Instance[0].bAuthenticated = FALSE ;

    // Authenticate should be called after AFE setup up.

    HDMITX_DEBUG_PRINTF("HDCP_Authenticate():\n");
    HDCP_Reset();
    // ClearDDCFIFO();
    // AbortDDC();

    Switch_HDMITX_Bank(0);

    for( TimeOut = 0 ; TimeOut < 20 ; TimeOut++ )
    {
        DelayMS(15);

        if(HDCP_GetBCaps(&BCaps,&BStatus) != ER_SUCCESS)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetBCaps fail.\n");
            return ER_FAIL ;
        }

        if(B_TX_HDMI_MODE == (HDMITX_ReadI2C_Byte(REG_TX_HDMI_MODE) & B_TX_HDMI_MODE ))
        {
            if((BStatus & B_CAP_HDMI_MODE)==B_CAP_HDMI_MODE)
            {
                break;
            }
        }
        else
        {
            if((BStatus & B_CAP_HDMI_MODE)!=B_CAP_HDMI_MODE)
            {
                break;
            }
        }
    }
#if defined(CONFIG_AST1500_CAT6613)
#if SUPPORT_HDCP_REPEATER
    Instance[0].Bcaps = BCaps ;
#endif
#endif

//    HDMITX_DEBUG_PRINTF(("BCaps = %02X BStatus = %04X\n",(int)BCaps,(int)BStatus));
    /*
    if((BStatus & M_DOWNSTREAM_COUNT)> 6)
    {
        HDMITX_DEBUG_PRINTF(("Down Stream Count %d is over maximum supported number 6,fail.\n",(int)(BStatus & M_DOWNSTREAM_COUNT)));
        return ER_FAIL ;
    }
    */

    HDCP_GetBKSV(BKSV);
//    HDMITX_DEBUG_PRINTF(("BKSV %02X %02X %02X %02X %02X\n",(int)BKSV[0],(int)BKSV[1],(int)BKSV[2],(int)BKSV[3],(int)BKSV[4]));

    for(TimeOut = 0, ucdata = 0 ; TimeOut < 5 ; TimeOut ++)
    {
        ucdata += countbit(BKSV[TimeOut]);
    }
    if( ucdata != 20 ) return ER_FAIL ;

    Switch_HDMITX_Bank(0); // switch bank action should start on direct register writting of each function.

    // 2006/08/11 added by jjtseng
    // enable HDCP on CPDired enabled.
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_HDCP_RST_HDMITX));
    //~jjtseng 2006/08/11

    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,B_CPDESIRE);
    HDCP_ClearAuthInterrupt();


    HDCP_GenerateAn();
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0);
    Instance[0].bAuthenticated = FALSE ;

    if((BCaps & B_CAP_HDMI_REPEATER) == 0)
    {
#if defined(CONFIG_AST1500_CAT6613)
#if SUPPORT_HDCP_REPEATER
		Instance[0].Bstatus = 0;
#endif
#endif
        HDCP_Auth_Fire();
        // wait for status ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            DelayMS(5); // delay 1ms
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_AUTH_STAT);
            // HDMITX_DEBUG_PRINTF(("reg46 = %02x reg16 = %02x\n",(int)ucdata,(int)HDMITX_ReadI2C_Byte(0x16)));

            if(ucdata & B_T_AUTH_DONE)
            {
                HDMITX_DEBUG_PRINTF("HDCP_Authenticate(): Authenticate done. OK.\n");
                Instance[0].bAuthenticated = TRUE ;
                break ;
            }

            ucdata = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2);
            if(ucdata & B_INT_AUTH_FAIL)
            {
                /*
                HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL);
                HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0);
                HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE);
                HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0);
                */
                HDMITX_DEBUG_PRINTF("HDCP_Authenticate(): Authenticate fail\n");
                Instance[0].bAuthenticated = FALSE ;
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
             HDMITX_DEBUG_PRINTF("HDCP_Authenticate(): Time out. return fail\n");
             Instance[0].bAuthenticated = FALSE ;
             return ER_FAIL ;
        }
        return ER_SUCCESS ;
    }

    return HDCP_Authenticate_Repeater();
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_VerifyIntegration
// Parameter: N/A
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark: no used now.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_VerifyIntegration(void)
{
    // if any interrupt issued a Auth fail,returned the Verify Integration fail.

    if(HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) & B_INT_AUTH_FAIL)
    {
        HDCP_ClearAuthInterrupt();
        Instance[0].bAuthenticated = FALSE ;
        return ER_FAIL ;
    }

    if(Instance[0].bAuthenticated == TRUE)
    {
        return ER_SUCCESS ;
    }

    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Authenticate_Repeater
// Parameter: BCaps and BStatus
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark:
// Side-Effect: as Authentication
//////////////////////////////////////////////////////////////////////

static void
HDCP_CancelRepeaterAuthenticate(void)
{
    HDMITX_DEBUG_PRINTF("HDCP_CancelRepeaterAuthenticate");
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST);
    AbortDDC();
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,B_LISTFAIL|B_LISTDONE);
    HDCP_ClearAuthInterrupt();
}

static void
HDCP_ResumeRepeaterAuthenticate(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,B_LISTDONE);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHDCP);
}

#ifdef SUPPORT_SHA
#define SHA_BUFF_COUNT 17
#ifdef CONFIG_AST1500_CAT6613
#if !SUPPORT_HDCP_REPEATER
static _XDATA BYTE KSVList[32] ;
#endif
#else
static _XDATA BYTE KSVList[32] ;
#endif
static _XDATA BYTE Vr[20] ;
static _XDATA BYTE M0[8] ;
static _XDATA BYTE V[20] ;
static _XDATA BYTE SHABuff[64] ;
static _XDATA ULONG w[SHA_BUFF_COUNT];

static _XDATA ULONG sha[5] ;

#define rol(x,y) (((x) << (y)) | (((ULONG)x) >> (32-y)))

void SHATransform(ULONG * h)
{
    int t,i;
    ULONG tmp ;


    h[0] = 0x67452301 ;
    h[1] = 0xefcdab89;
    h[2] = 0x98badcfe;
    h[3] = 0x10325476;
    h[4] = 0xc3d2e1f0;
    for( t = 0 ; t < 80 ; t++ )
    {
        if((t>=16)&&(t<80)) {
            i=(t+SHA_BUFF_COUNT-3)%SHA_BUFF_COUNT;
            tmp = w[i];
            i=(t+SHA_BUFF_COUNT-8)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            i=(t+SHA_BUFF_COUNT-14)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            i=(t+SHA_BUFF_COUNT-16)%SHA_BUFF_COUNT;
            tmp ^= w[i];
            w[t%SHA_BUFF_COUNT] = rol(tmp,1);
#ifdef DBG_HDCP
            HDMITX_DEBUG_PRINTF("w[%2d] = %08lX\n",t,w[t%SHA_BUFF_COUNT]);
#endif
        }

        if((t>=0)&&(t<20)) {
            tmp = rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t%SHA_BUFF_COUNT] + 0x5a827999;
#ifdef DBG_HDCP
            HDMITX_DEBUG_PRINTF("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]);
#endif
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;

        }
        if((t>=20)&&(t<40)) {
            tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t%SHA_BUFF_COUNT] + 0x6ed9eba1;
#ifdef DBG_HDCP
            HDMITX_DEBUG_PRINTF("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]);
#endif
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
        if((t>=40)&&(t<60)) {
            tmp = rol(h[0], 5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t%SHA_BUFF_COUNT] +
                0x8f1bbcdc;
#ifdef DBG_HDCP
            HDMITX_DEBUG_PRINTF("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]);
#endif
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
        if((t>=60)&&(t<80)) {
            tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t%SHA_BUFF_COUNT] + 0xca62c1d6;
#ifdef DBG_HDCP
            HDMITX_DEBUG_PRINTF("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]);
#endif
            h[4] = h[3];
            h[3] = h[2];
            h[2] = rol(h[1],30);
            h[1] = h[0];
            h[0] = tmp;
        }
    }
#ifdef DBG_HDCP
    HDMITX_DEBUG_PRINTF("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]);
#endif
    h[0] += 0x67452301 ;
    h[1] += 0xefcdab89;
    h[2] += 0x98badcfe;
    h[3] += 0x10325476;
    h[4] += 0xc3d2e1f0;
//    HDMITX_DEBUG_PRINTF(("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]));
}

/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end,and pass those blocks to the core SHA algorithm.
 */


void SHA_Simple(void *p,LONG len,BYTE *output)
{
    // SHA_State s;
    int i, t ;
    ULONG c ;
    char *pBuff = p ;


    for( i = 0 ; i < len ; i++ )
    {
        t = i/4 ;
        if( i%4 == 0 )
        {
            w[t] = 0 ;
        }
#if defined(CODE202)
        c = pBuff[i] ;
        c &= 0xFF ;
#else
        c = pBuff[i]&0xFF ;
#endif
        c <<= (3-(i%4))*8 ;
        w[t] |= c ;
//        HDMITX_DEBUG_PRINTF(("pBuff[%d] = %02x, c = %08lX, w[%d] = %08lX\n",i,pBuff[i],c,t,w[t]));
    }
    t = i/4 ;
    if( i%4 == 0 )
    {
        w[t] = 0 ;
    }
    c = 0x80 << ((3-i%4)*8);
    w[t]|= c ; t++ ;
    for( ; t < 15 ; t++ )
    {
        w[t] = 0 ;
    }
    w[15] = len*8  ;

#ifdef DBG_HDCP
    for( t = 0 ; t< 16 ; t++ )
    {
        HDMITX_DEBUG_PRINTF(("w[%2d] = %08lX\n",t,w[t]));
    }
#endif

    SHATransform(sha);

    for( i = 0 ; i < 5 ; i++ )
    {
#if defined(CODE202)
        output[i*4] = (BYTE)(sha[i]&0xFF);
        output[i*4+1] = (BYTE)((sha[i]>>8)&0xFF);
        output[i*4+2] = (BYTE)((sha[i]>>16)&0xFF);
        output[i*4+3]   = (BYTE)((sha[i]>>24)&0xFF);
#else
        output[i*4]   = (BYTE)((sha[i]>>24)&0xFF);
        output[i*4+1] = (BYTE)((sha[i]>>16)&0xFF);
        output[i*4+2] = (BYTE)((sha[i]>>8)&0xFF);
        output[i*4+3] = (BYTE)(sha[i]&0xFF);
#endif
    }
}

static SYS_STATUS
HDCP_CheckSHA(BYTE pM0[],USHORT BStatus,BYTE pKSVList[],int cDownStream,BYTE Vr[])
{
    int i,n ;

    for(i = 0 ; i < cDownStream*5 ; i++)
    {
        SHABuff[i] = pKSVList[i] ;
    }
    SHABuff[i++] = BStatus & 0xFF ;
    SHABuff[i++] = (BStatus>>8) & 0xFF ;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        SHABuff[i] = pM0[n] ;
    }
    n = i ;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        SHABuff[i] = 0 ;
    }
    // n = cDownStream * 5 + 2 /* for BStatus */ + 8 /* for M0 */ ;
    // n *= 8 ;
    // SHABuff[62] = (n>>8) & 0xff ;
    // SHABuff[63] = (n>>8) & 0xff ;
#ifdef DBG_HDCP
    for(i = 0 ; i < 64 ; i++)
    {
        if(i % 16 == 0)
        {
            HDMITX_DEBUG_PRINTF("SHA[]: ");
        }
        HDMITX_DEBUG_PRINTF((" %02X",SHABuff[i]));
        if((i%16)==15)
        {
            HDMITX_DEBUG_PRINTF("\n");
        }
    }
#endif

    SHA_Simple(SHABuff,n,V);

#ifdef DBG_HDCP
    HDMITX_DEBUG_PRINTF("V[] =");
    for(i = 0 ; i < 20 ; i++)
    {
        HDMITX_DEBUG_PRINTF((" %02X",(int)V[i]));
    }
    HDMITX_DEBUG_PRINTF("\nVr[] =");
    for(i = 0 ; i < 20 ; i++)
    {
        HDMITX_DEBUG_PRINTF((" %02X",(int)Vr[i]));
    }
    HDMITX_DEBUG_PRINTF("\n");
#endif

    for(i = 0 ; i < 20 ; i++)
    {
        if(V[i] != Vr[i])
        {
            return ER_FAIL ;
        }
    }
    return ER_SUCCESS ;
}
#endif // SUPPORT_SHA

static SYS_STATUS
HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream)
{
    BYTE TimeOut = 100 ;
    BYTE ucdata ;

    if( cDownStream == 0 )
    {
        return ER_SUCCESS ;
    }

    if( /* cDownStream == 0 || */ pKSVList == NULL)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,0x74);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x43);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,cDownStream * 5);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetKSVList(): DDC Done.\n");
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetKSVList(): DDC Fail by REG_TX_DDC_STATUS = %x.\n",ucdata);
            return ER_FAIL ;
        }
        DelayMS(5);
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        pKSVList[TimeOut] = HDMITX_ReadI2C_Byte(REG_TX_DDC_READFIFO);
    }

#ifdef DBG_HDCP
    HDMITX_DEBUG_PRINTF("HDCP_GetKSVList(): KSV");
    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        HDMITX_DEBUG_PRINTF((" %02X",(int)pKSVList[TimeOut]));
    }
    HDMITX_DEBUG_PRINTF("\n");
#endif
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetVr(BYTE *pVr)
{
    BYTE TimeOut  ;
    BYTE ucdata ;

    if(pVr == NULL)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,0x74);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x20);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,20);
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD);


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS);
        if(ucdata & B_DDC_DONE)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetVr(): DDC Done.\n");
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            HDMITX_DEBUG_PRINTF("HDCP_GetVr(): DDC fail by REG_TX_DDC_STATUS = %x.\n",(int)ucdata);
            return ER_FAIL ;
        }
        DelayMS(5);
    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF("HDCP_GetVr(): DDC fail by timeout.\n");
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(0);

    for(TimeOut = 0 ; TimeOut < 5 ; TimeOut++)
    {
        HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL ,TimeOut);
#if defined(CODE202)
        pVr[TimeOut*4]  = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE1);
        pVr[TimeOut*4+1] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE2);
        pVr[TimeOut*4+2] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE3);
        pVr[TimeOut*4+3] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE4);
#else
        pVr[TimeOut*4+3]  = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE1);
        pVr[TimeOut*4+2] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE2);
        pVr[TimeOut*4+1] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE3);
        pVr[TimeOut*4] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE4);
#endif
//        HDMITX_DEBUG_PRINTF(("V' = %02X %02X %02X %02X\n",(int)pVr[TimeOut*4],(int)pVr[TimeOut*4+1],(int)pVr[TimeOut*4+2],(int)pVr[TimeOut*4+3]));
    }

    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetM0(BYTE *pM0)
{
    int i ;

    if(!pM0)
    {
        return ER_FAIL ;
    }

    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,5); // read m0[31:0] from reg51~reg54
    pM0[0] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE1);
    pM0[1] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE2);
    pM0[2] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE3);
    pM0[3] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE4);
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,0); // read m0[39:32] from reg55
    pM0[4] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,1); // read m0[47:40] from reg55
    pM0[5] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,2); // read m0[55:48] from reg55
    pM0[6] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5);
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,3); // read m0[63:56] from reg55
    pM0[7] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5);

#if DBG_HDCP
    HDMITX_DEBUG_PRINTF("M[] =");
    for(i = 0 ; i < 8 ; i++){
//        HDMITX_DEBUG_PRINTF(("0x%02x,",(int)pM0[i]));
    }
    HDMITX_DEBUG_PRINTF("\n");
#endif
    return ER_SUCCESS ;
}



static SYS_STATUS
HDCP_Authenticate_Repeater(void)
{
    BYTE uc ;
    // BYTE revoked ;
    // int i ;
    BYTE cDownStream ;

    BYTE BCaps;
    USHORT BStatus ;
    USHORT TimeOut ;

    HDMITX_DEBUG_PRINTF("Authentication for repeater\n");
    // emily add for test,abort HDCP
    // 2007/10/01 marked by jj_tseng@chipadvanced.com
    // HDMITX_WriteI2C_Byte(0x20,0x00);
    // HDMITX_WriteI2C_Byte(0x04,0x01);
    // HDMITX_WriteI2C_Byte(0x10,0x01);
    // HDMITX_WriteI2C_Byte(0x15,0x0F);
    // DelayMS(100);
    // HDMITX_WriteI2C_Byte(0x04,0x00);
    // HDMITX_WriteI2C_Byte(0x10,0x00);
    // HDMITX_WriteI2C_Byte(0x20,0x01);
    // DelayMS(100);
    // test07 = HDMITX_ReadI2C_Byte(0x7);
    // test06 = HDMITX_ReadI2C_Byte(0x6);
    // test08 = HDMITX_ReadI2C_Byte(0x8);
    //~jj_tseng@chipadvanced.com
    // end emily add for test
    //////////////////////////////////////
    // Authenticate Fired
    //////////////////////////////////////

    HDCP_GetBCaps(&BCaps,&BStatus);
    DelayMS(2);
    HDCP_Auth_Fire();
    DelayMS(550); // emily add for test

#ifdef CONFIG_AST1500_CAT6613
    for(TimeOut = 250; TimeOut > 0 ; TimeOut --)
#else
    for(TimeOut = 250*6 ; TimeOut > 0 ; TimeOut --)
#endif
    {

        uc = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1);
        if(uc & B_INT_DDC_BUS_HANG)
        {
            HDMITX_DEBUG_PRINTF("DDC Bus hang\n");
            goto HDCP_Repeater_Fail ;
        }

        uc = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2);

        if(uc & B_INT_AUTH_FAIL)
        {
            /*
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL);
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0);
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE);
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0);
            */
            HDMITX_DEBUG_PRINTF("HDCP_Authenticate_Repeater(): B_INT_AUTH_FAIL.\n");
            goto HDCP_Repeater_Fail ;
        }
        // emily add for test
        // test =(HDMITX_ReadI2C_Byte(0x7)&0x4)>>2 ;
        if(uc & B_INT_KSVLIST_CHK)
        {
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_KSVLISTCHK);
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0);
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE);
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0);
            HDMITX_DEBUG_PRINTF("B_INT_KSVLIST_CHK\n");
            break ;
        }

        DelayMS(5);
    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF("Time out for wait KSV List checking interrupt\n");
        goto HDCP_Repeater_Fail ;
    }

    ///////////////////////////////////////
    // clear KSVList check interrupt.
    ///////////////////////////////////////
#ifdef CONFIG_AST1500_CAT6613
    for(TimeOut = 50 ; TimeOut > 0 ; TimeOut --)
#else
    for(TimeOut = 500 ; TimeOut > 0 ; TimeOut --)
#endif
    {
#ifdef CONFIG_AST1500_CAT6613
        if((TimeOut % 10) == 0)
#else
        if((TimeOut % 100) == 0)
#endif
        {
            HDMITX_DEBUG_PRINTF("Wait KSV FIFO Ready %d\n",TimeOut);
        }

        if(HDCP_GetBCaps(&BCaps,&BStatus) == ER_FAIL)
        {
            HDMITX_DEBUG_PRINTF("Get BCaps fail\n");
            goto HDCP_Repeater_Fail ;
        }

        if(BCaps & B_CAP_KSV_FIFO_RDY)
        {
             HDMITX_DEBUG_PRINTF("FIFO Ready\n");
             break ;
        }
        DelayMS(5);

    }

    if(TimeOut == 0)
    {
        HDMITX_DEBUG_PRINTF("Get KSV FIFO ready TimeOut\n");
        goto HDCP_Repeater_Fail ;
    }

    HDMITX_DEBUG_PRINTF("Wait timeout = %d\n",TimeOut);

    ClearDDCFIFO();
    GenerateDDCSCLK();
    cDownStream =  (BStatus & M_DOWNSTREAM_COUNT);

#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
	Instance[0].Bstatus = BStatus;
    if (BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
        HDMITX_DEBUG_PRINTF("topology maximun exceeded\n");
        goto HDCP_Repeater_Fail ;
    }
    if(/*cDownStream == 0 ||*/ cDownStream > 30)
    {
        HDMITX_DEBUG_PRINTF("Invalid Down stream count,fail\n");
        goto HDCP_Repeater_Fail ;
    }
#else
    if(/*cDownStream == 0 ||*/ cDownStream > 6 || BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
        HDMITX_DEBUG_PRINTF("Invalid Down stream count,fail\n");
        goto HDCP_Repeater_Fail ;
    }
#endif
#else
    if(/*cDownStream == 0 ||*/ cDownStream > 6 || BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
        HDMITX_DEBUG_PRINTF("Invalid Down stream count,fail\n");
        goto HDCP_Repeater_Fail ;
    }
#endif

#ifdef SUPPORT_SHA
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
    if(HDCP_GetKSVList(Instance[0].KSVList,cDownStream) == ER_FAIL)
#else
	if(HDCP_GetKSVList(KSVList,cDownStream) == ER_FAIL)
#endif
#else
    if(HDCP_GetKSVList(KSVList,cDownStream) == ER_FAIL)
#endif
    {
        goto HDCP_Repeater_Fail ;
    }

#if 0
    for(i = 0 ; i < cDownStream ; i++)
    {
        revoked=FALSE ; uc = 0 ;
        for( TimeOut = 0 ; TimeOut < 5 ; TimeOut++ )
        {
            // check bit count
            uc += countbit(KSVList[i*5+TimeOut]);
        }
        if( uc != 20 ) revoked = TRUE ;

        if(revoked)
        {
//            HDMITX_DEBUG_PRINTF(("KSVFIFO[%d] = %02X %02X %02X %02X %02X is revoked\n",i,(int)KSVList[i*5],(int)KSVList[i*5+1],(int)KSVList[i*5+2],(int)KSVList[i*5+3],(int)KSVList[i*5+4]));
             goto HDCP_Repeater_Fail ;
        }
    }
#endif


    if(HDCP_GetVr(Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

    if(HDCP_GetM0(M0) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

    // do check SHA
#ifdef CONFIG_AST1500_CAT6613
#if SUPPORT_HDCP_REPEATER
    if(HDCP_CheckSHA(M0,BStatus,Instance[0].KSVList,cDownStream,Vr) == ER_FAIL)
#else
	if(HDCP_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
#endif
#else
    if(HDCP_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
#endif
    {
        goto HDCP_Repeater_Fail ;
    }
#endif // SUPPORT_SHA


    HDCP_ResumeRepeaterAuthenticate();
    Instance[0].bAuthenticated = TRUE ;
    return ER_SUCCESS ;

HDCP_Repeater_Fail:
    HDCP_CancelRepeaterAuthenticate();
    return ER_FAIL ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_ResumeAuthentication
// Parameter: N/A
// Return: N/A
// Remark: called by interrupt handler to restart Authentication and Encryption.
// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////

void
HDCP_ResumeAuthentication(void)
{
    SetAVMute(TRUE);
    if(HDCP_Authenticate() == ER_SUCCESS)
    {
        HDCP_EnableEncryption();
    }
    SetAVMute(FALSE);
}



#endif // SUPPORT_HDCP


static void
ENABLE_NULL_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ACP_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ACP_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC1_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC1_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC2_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC2_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AVI_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AVI_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AUD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AUD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_SPD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_SPD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_MPG_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_MPG_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

#if defined(CODE202)
static void
ENABLE_GeneralPurpose_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}
#endif

static void
DISABLE_NULL_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,0);
}


static void
DISABLE_ACP_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ACP_CTRL,0);
}


static void
DISABLE_ISRC1_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC1_CTRL,0);
}


static void
DISABLE_ISRC2_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC2_CTRL,0);
}


static void
DISABLE_AVI_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AVI_INFOFRM_CTRL,0);
}


static void
DISABLE_AUD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AUD_INFOFRM_CTRL,0);
}


static void
DISABLE_SPD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_SPD_INFOFRM_CTRL,0);
}


static void
DISABLE_MPG_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_MPG_INFOFRM_CTRL,0);
}

#if defined(CODE202)
static void
DISABLE_GeneralPurpose_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,0);
}
#endif

//////////////////////////////////////////////////////////////////////
// Function: SetAVIInfoFrame()
// Parameter: pAVIInfoFrame - the pointer to HDMI AVI Infoframe ucData
// Return: N/A
// Remark: Fill the AVI InfoFrame ucData,and count checksum,then fill into
//         AVI InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    int i ;
    byte ucData ;

    if(!pAVIInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1,pAVIInfoFrame->pktbyte.AVI_DB[0]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB2,pAVIInfoFrame->pktbyte.AVI_DB[1]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB3,pAVIInfoFrame->pktbyte.AVI_DB[2]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB4,pAVIInfoFrame->pktbyte.AVI_DB[3]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB5,pAVIInfoFrame->pktbyte.AVI_DB[4]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB6,pAVIInfoFrame->pktbyte.AVI_DB[5]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB7,pAVIInfoFrame->pktbyte.AVI_DB[6]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB8,pAVIInfoFrame->pktbyte.AVI_DB[7]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB9,pAVIInfoFrame->pktbyte.AVI_DB[8]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB10,pAVIInfoFrame->pktbyte.AVI_DB[9]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB11,pAVIInfoFrame->pktbyte.AVI_DB[10]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB12,pAVIInfoFrame->pktbyte.AVI_DB[11]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB13,pAVIInfoFrame->pktbyte.AVI_DB[12]);
    for(i = 0,ucData = 0; i < 13 ; i++)
    {
        ucData -= pAVIInfoFrame->pktbyte.AVI_DB[i] ;
    }
    /*
    HDMITX_DEBUG_PRINTF(("SetAVIInfo(): "));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB1)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB2)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB3)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB4)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB5)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB6)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB7)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB8)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB9)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB10)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB11)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB12)));
    HDMITX_DEBUG_PRINTF(("%02X ",(int)HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB13)));
    HDMITX_DEBUG_PRINTF(("\n"));
    */
    ucData -= 0x80+AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN ;
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_SUM,ucData);


    Switch_HDMITX_Bank(0);
    ENABLE_AVI_INFOFRM_PKT();
    return ER_SUCCESS ;
}

#ifdef CONFIG_AST1500_CAT6613
void CopyAVIInfoFrame(BYTE *pAVIInfoFrame)
{
    Switch_HDMITX_Bank(1);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1,pAVIInfoFrame[1]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB2,pAVIInfoFrame[2]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB3,pAVIInfoFrame[3]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB4,pAVIInfoFrame[4]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB5,pAVIInfoFrame[5]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB6,pAVIInfoFrame[6]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB7,pAVIInfoFrame[7]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB8,pAVIInfoFrame[8]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB9,pAVIInfoFrame[9]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB10,pAVIInfoFrame[10]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB11,pAVIInfoFrame[11]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB12,pAVIInfoFrame[12]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB13,pAVIInfoFrame[13]);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_SUM,pAVIInfoFrame[0]);
    Switch_HDMITX_Bank(0);
    ENABLE_AVI_INFOFRM_PKT();
}
#endif

//////////////////////////////////////////////////////////////////////
// Function: SetAudioInfoFrame()
// Parameter: pAudioInfoFrame - the pointer to HDMI Audio Infoframe ucData
// Return: N/A
// Remark: Fill the Audio InfoFrame ucData,and count checksum,then fill into
//         Audio InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
    BYTE uc ;

    if(!pAudioInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1);
    uc = 0x80-(AUDIO_INFOFRAME_VER+AUDIO_INFOFRAME_TYPE+AUDIO_INFOFRAME_LEN );
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_CC,pAudioInfoFrame->pktbyte.AUD_DB[0]);
    uc -= HDMITX_ReadI2C_Byte(REG_TX_PKT_AUDINFO_CC); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_SF,pAudioInfoFrame->pktbyte.AUD_DB[1]);
    uc -= HDMITX_ReadI2C_Byte(REG_TX_PKT_AUDINFO_SF); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_CA,pAudioInfoFrame->pktbyte.AUD_DB[3]);
    uc -= HDMITX_ReadI2C_Byte(REG_TX_PKT_AUDINFO_CA); uc &= 0xFF ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_DM_LSV,pAudioInfoFrame->pktbyte.AUD_DB[4]);
    uc -= HDMITX_ReadI2C_Byte(REG_TX_PKT_AUDINFO_DM_LSV); uc &= 0xFF ;

    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_SUM,uc);


    Switch_HDMITX_Bank(0);
    ENABLE_AUD_INFOFRM_PKT();
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetSPDInfoFrame()
// Parameter: pSPDInfoFrame - the pointer to HDMI SPD Infoframe ucData
// Return: N/A
// Remark: Fill the SPD InfoFrame ucData,and count checksum,then fill into
//         SPD InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetSPDInfoFrame(SPD_InfoFrame *pSPDInfoFrame)
{
    int i ;
    BYTE ucData ;

    if(!pSPDInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1);
    for(i = 0,ucData = 0 ; i < 25 ; i++)
    {
        ucData -= pSPDInfoFrame->pktbyte.SPD_DB[i] ;
        HDMITX_WriteI2C_Byte(REG_TX_PKT_SPDINFO_PB1+i,pSPDInfoFrame->pktbyte.SPD_DB[i]);
    }
    ucData -= 0x80+SPD_INFOFRAME_VER+SPD_INFOFRAME_TYPE+SPD_INFOFRAME_LEN ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_SPDINFO_SUM,ucData); // checksum
    Switch_HDMITX_Bank(0);
    ENABLE_SPD_INFOFRM_PKT();
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetMPEGInfoFrame()
// Parameter: pMPEGInfoFrame - the pointer to HDMI MPEG Infoframe ucData
// Return: N/A
// Remark: Fill the MPEG InfoFrame ucData,and count checksum,then fill into
//         MPEG InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetMPEGInfoFrame(MPEG_InfoFrame *pMPGInfoFrame)
{
    int i ;
    BYTE ucData ;

    if(!pMPGInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1);

    HDMITX_WriteI2C_Byte(REG_TX_PKT_MPGINFO_FMT,pMPGInfoFrame->info.FieldRepeat|(pMPGInfoFrame->info.MpegFrame<<1));
    HDMITX_WriteI2C_Byte(REG_TX_PKG_MPGINFO_DB0,pMPGInfoFrame->pktbyte.MPG_DB[0]);
    HDMITX_WriteI2C_Byte(REG_TX_PKG_MPGINFO_DB1,pMPGInfoFrame->pktbyte.MPG_DB[1]);
    HDMITX_WriteI2C_Byte(REG_TX_PKG_MPGINFO_DB2,pMPGInfoFrame->pktbyte.MPG_DB[2]);
    HDMITX_WriteI2C_Byte(REG_TX_PKG_MPGINFO_DB3,pMPGInfoFrame->pktbyte.MPG_DB[3]);

    for(ucData = 0,i = 0 ; i < 5 ; i++)
    {
        ucData -= pMPGInfoFrame->pktbyte.MPG_DB[i] ;
    }
    ucData -= 0x80+MPEG_INFOFRAME_VER+MPEG_INFOFRAME_TYPE+MPEG_INFOFRAME_LEN ;

    HDMITX_WriteI2C_Byte(REG_TX_PKG_MPGINFO_SUM,ucData);

    Switch_HDMITX_Bank(0);
    ENABLE_SPD_INFOFRM_PKT();

    return ER_SUCCESS ;
}


// 2009/12/04 added by Ming-chih.lung@ite.com.tw

/////////////////////////////////////////////////////////////////////////////////////
// CAT6613 part
/////////////////////////////////////////////////////////////////////////////////////
void setCAT6613_ChStat(BYTE ucIEC60958ChStat[]);
void setCAT6613_UpdateChStatFs(ULONG Fs);
void setCAT6613_LPCMAudio(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF);
void setCAT6613_NLPCMAudio(void);
void setCAT6613_HBRAudio(BOOL bSPDIF);
void setCAT6613_DSDAudio(void);


void
setCAT6613_ChStat(BYTE ucIEC60958ChStat[])
{
    BYTE uc ;

#if 0
	if (Instance[0].outputparam.bSPDIF)
		return;
#endif

    Switch_HDMITX_Bank(1);
    uc = (ucIEC60958ChStat[0] <<1)& 0x7C ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,uc);
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,ucIEC60958ChStat[1]); // 192, audio CATEGORY
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,ucIEC60958ChStat[2]&0xF);
    HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,(ucIEC60958ChStat[2]>>4)&0xF);
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,ucIEC60958ChStat[3]); // choose clock
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,ucIEC60958ChStat[4]);
    Switch_HDMITX_Bank(0);
}

void
setCAT6613_UpdateChStatFs(ULONG Fs)
{
    BYTE uc ;

    /////////////////////////////////////
    // Fs should be the following value.
    // #define AUDFS_22p05KHz  4
    // #define AUDFS_44p1KHz 0
    // #define AUDFS_88p2KHz 8
    // #define AUDFS_176p4KHz    12
    //
    // #define AUDFS_24KHz  6
    // #define AUDFS_48KHz  2
    // #define AUDFS_96KHz  10
    // #define AUDFS_192KHz 14
    //
    // #define AUDFS_768KHz 9
    //
    // #define AUDFS_32KHz  3
    // #define AUDFS_OTHER    1
    /////////////////////////////////////

    Switch_HDMITX_Bank(1);
    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_CA_FS); // choose clock
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,uc); // choose clock
    uc &= 0xF0 ;
    uc |= (Fs&0xF);

    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_OFS_WL);
    uc &= 0xF ;
    uc |= ((~Fs) << 4)&0xF0 ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,uc);

    Switch_HDMITX_Bank(0);


}

void
setCAT6613_LPCMAudio(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF)
{

    BYTE AudioEnable, AudioFormat ;

	printk("setCAT6613_LPCMAudio(%d,%d,%d)\n",AudioSrcNum,AudSWL,bSPDIF);
    AudioEnable = 0 ;
    AudioFormat = Instance[0].bOutputAudioMode ;


    switch(AudSWL)
    {
    case 16:
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
    default:
        AudioEnable |= M_AUD_24BIT ;
        break ;
    }

    if( bSPDIF )
    {
        AudioFormat &= ~0x40 ;
        AudioEnable |= B_AUD_SPDIF|B_AUD_EN_I2S0 ;
    }
    else
    {
        AudioFormat |= 0x40 ;
        switch(AudioSrcNum)
        {
        case 4:
            AudioEnable |= B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 3:
            AudioEnable |= B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 2:
            AudioEnable |= B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;

        case 1:
        default:
            AudioFormat &= ~0x40 ;
            AudioEnable |= B_AUD_EN_I2S0 ;
            break ;

        }
    }
    AudioFormat|=0x01;//mingchih add
#if defined(CODE202)
    Instance[0].bAudioChannelEnable=AudioEnable;
#endif

    Switch_HDMITX_Bank(0);
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0);
#if !defined(CODE202)
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
#endif

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,AudioFormat); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.
#ifdef USE_SPDIF_CHSTAT
    if( bSPDIF )
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0);
    }
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0);
#endif // USE_SPDIF_CHSTAT

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,0x00); // regE5 = 0 ;

    if( bSPDIF )
    {
        BYTE i ;
#if defined(CODE202)
        HDMI_OrREG_TX_Byte(0x5c,(1<<6));
#endif
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
    }
#ifdef CONFIG_AST1500_CAT6613
    //ss: &FO will mark the i2s channel num. I think this is wrong
    //HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable);
#endif
}

void
setCAT6613_NLPCMAudio(void) // no Source Num, no I2S.
{
    BYTE AudioEnable, AudioFormat ;
    BYTE i ;

    AudioFormat = 0x01 ; // NLPCM must use standard I2S mode.
    AudioEnable = M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_I2S0 ;

    Switch_HDMITX_Bank(0);
#ifdef NLPCM_THROUGH_I2S
	HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT);
#else
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF);
#endif
#if !defined(CODE202)
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
#endif

    //Bruce121106. Fix Bug#2012061800. According to JJ, set RegE1[6].
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x41); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

#ifdef USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0);
#endif // USE_SPDIF_CHSTAT

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,0x00); // regE5 = 0 ;

    for( i = 0 ; i < 100 ; i++ )
    {
        if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
        {
            break ; // stable clock.
        }
    }
#ifdef NLPCM_THROUGH_I2S
	HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S0);
#else
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_I2S0);
#endif
}

void
setCAT6613_HBRAudio(BOOL bSPDIF)
{
    BYTE rst,uc ;
    Switch_HDMITX_Bank(0);

    rst = HDMITX_ReadI2C_Byte(REG_TX_SW_RST);

#if !defined(CODE202)
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst | (B_AUD_RST_HDMITX|B_AREF_RST) );
#endif

    //ss: 0x41 is current cat's setting
    //HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x47); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x41); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

    if( bSPDIF )
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF);
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL);
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT);
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0);
    }

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x08);
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,B_HBR); // regE5 = 0 ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1);
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc);

    if( bSPDIF )
    {
        BYTE i ;
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_SPDIF|B_AUD_EN_SPDIF);
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0);
    }
    HDMI_AndREG_TX_Byte(0x5c,~(1<<6));
    Instance[0].bAudioChannelEnable=HDMITX_ReadI2C_Byte(REG_TX_AUDIO_CTRL0);
#if !defined(CODE202)
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst & ~(B_AUD_RST_HDMITX|B_AREF_RST) );
#endif
}

void
setCAT6613_DSDAudio(void)
{
    // to be continue
    BYTE rst, uc ;
    rst = HDMITX_ReadI2C_Byte(REG_TX_SW_RST);

#if !defined(CODE202)
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst | (B_AUD_RST_HDMITX|B_AREF_RST) );
#endif

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x41); // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4); // default mapping.

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT);
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0);

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00);
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,B_DSD); // regE5 = 0 ;
#if !defined(CODE202)
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst & ~(B_AUD_RST_HDMITX|B_AREF_RST) );
#endif

    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1);
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc);



    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0);
}

void
EnableHDMIAudio(BYTE AudioType, BOOL bSPDIF,  ULONG SampleFreq,  BYTE ChNum, BYTE *pIEC60958ChStat, ULONG TMDSClock)
{
    static _IDATA BYTE ucIEC60958ChStat[5] ;
    BYTE Fs ;
	printk("EnableHDMIAudio(%02X,%d,%lu,%d,%lu)\n",AudioType,bSPDIF,SampleFreq,ChNum,TMDSClock);
#ifndef CONFIG_AST1500_CAT6613
    Instance[0].TMDSClock=TMDSClock;
#endif
#if defined(CODE202)
    Instance[0].bAudioChannelEnable=0;
    Instance[0].bSPDIF_OUT=bSPDIF;
#endif
#if defined(CODE202)
    HDMITX_OrReg_Byte(REG_TX_SW_RST,(B_AUD_RST_HDMITX | B_AREF_RST));
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL0,B_AUTO_OVER_SAMPLING_CLOCK|B_EXT_256FS|0x01);
    if(bSPDIF)
    {
        if(AudioType==T_AUDIO_HBR)
        {
            HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL0,0x81);
        }
        HDMITX_OrREG_Byte(REG_TX_AUDIO_CTRL0,B_AUD_SPDIF);
    }
    else
    {
        HDMITX_AndREG_Byte(REG_TX_AUDIO_CTRL0,(~B_AUD_SPDIF));
    }
#else
    if(bSPDIF)
        HDMITX_OrREG_Byte(REG_TX_AUDIO_CTRL0,B_AUD_SPDIF); // D[1] = 0,HW auto count CTS
    else
        HDMITX_AndREG_Byte(REG_TX_AUDIO_CTRL0,(~B_AUD_SPDIF)); // D[1] = 0,HW auto count CTS
#endif
     if( AudioType != T_AUDIO_DSD)
    {
#ifdef CONFIG_AST1500_CAT6613
		Fs= SampleFreq;
#else
        // one bit audio have no channel status.
        switch(SampleFreq)
        {
        case  44100L: Fs =  AUDFS_44p1KHz ; break ;
        case  88200L: Fs =  AUDFS_88p2KHz ; break ;
        case 176400L: Fs = AUDFS_176p4KHz ; break ;
        case  32000L: Fs =    AUDFS_32KHz ; break ;
        case  48000L: Fs =    AUDFS_48KHz ; break ;
        case  96000L: Fs =    AUDFS_96KHz ; break ;
        case 192000L: Fs =   AUDFS_192KHz ; break ;
        case 768000L: Fs =   AUDFS_768KHz ; break ;
        default:
            SampleFreq = 48000L ;
            Fs =    AUDFS_48KHz ;
            break ; // default, set Fs = 48KHz.
        }
#endif

#if defined(CODE202)
#ifdef SUPPORT_AUDIO_MONITOR
    Instance[0].bAudFs=AUDFS_OTHER;
#else
    Instance[0].bAudFs=Fs;
#endif
#else
        Fs=Instance[0].bAudFs;
#endif

        if( pIEC60958ChStat == NULL )
        {
            ucIEC60958ChStat[0] = 0 ;
            ucIEC60958ChStat[1] = 0 ;
            ucIEC60958ChStat[2] = (ChNum+1)/2 ;

            if(ucIEC60958ChStat[2]<1)
            {
                ucIEC60958ChStat[2] = 1 ;
            }
            else if( ucIEC60958ChStat[2] >4 )
            {
                ucIEC60958ChStat[2] = 4 ;
            }

            ucIEC60958ChStat[3] = Fs ;
#if defined(CODE202)//to do:check SUPPORT_AUDI_AudSWL
#if(SUPPORT_AUDI_AudSWL==16)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x02 ; // Fs | 24bit word length
#elif(SUPPORT_AUDI_AudSWL==18)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x04 ; // Fs | 24bit word length
#elif(SUPPORT_AUDI_AudSWL==20)
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x03 ; // Fs | 24bit word length
#else
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0x0B ; // Fs | 24bit word length
#endif
#else
            ucIEC60958ChStat[4] = ((~Fs)<<4) & 0xF0 | 0xB ; // Fs | 24bit word length
#endif
            pIEC60958ChStat = ucIEC60958ChStat ;
        }
    }

    switch(AudioType)
    {
    case T_AUDIO_HBR:
        HDMITX_DEBUG_PRINTF("T_AUDIO_HBR\n");
        pIEC60958ChStat[0] |= 1<<1 ;
        pIEC60958ChStat[2] = 0;
        pIEC60958ChStat[3] &= 0xF0 ;
        pIEC60958ChStat[3] |= AUDFS_768KHz ;
        pIEC60958ChStat[4] |= (((~AUDFS_768KHz)<<4) & 0xF0)| 0xB ;
        setCAT6613_ChStat(pIEC60958ChStat);
//#if !defined(CODE202)//to do:test
        SetNCTS(TMDSClock, AUDFS_192KHz);
//#endif
        setCAT6613_HBRAudio(bSPDIF);

        break ;
    case T_AUDIO_DSD:
        HDMITX_DEBUG_PRINTF("T_AUDIO_DSD\n");
//#if !defined(CODE202)//to do:test
        SetNCTS(TMDSClock, AUDFS_44p1KHz);
//#endif
        setCAT6613_DSDAudio();
        break ;
    case T_AUDIO_NLPCM:
        HDMITX_DEBUG_PRINTF("T_AUDIO_NLPCM\n");
        pIEC60958ChStat[0] |= 1<<1 ;
        setCAT6613_ChStat(pIEC60958ChStat);
//#if !defined(CODE202)//to do:test
        SetNCTS(TMDSClock, Fs);
//#endif
        setCAT6613_NLPCMAudio();
        break ;
    case T_AUDIO_LPCM:
        HDMITX_DEBUG_PRINTF("T_AUDIO_LPCM\n");
        pIEC60958ChStat[0] &= ~(1<<1);
        setCAT6613_ChStat(pIEC60958ChStat);
//#if !defined(CODE202)//to do:test
        SetNCTS(TMDSClock, Fs);
//#endif
#if defined(CODE202)
        setCAT6613_LPCMAudio((ChNum+1)/2, /*24*/SUPPORT_AUDI_AudSWL, bSPDIF);
#else
        setCAT6613_LPCMAudio((ChNum+1)/2, 24, bSPDIF);
#endif
        // can add auto adjust
        break ;
    }
#if defined(CODE202)
    HDMITX_AndREG_Byte(REG_TX_INT_MASK1,(~B_AUDIO_OVFLW_MASK));
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST_HDMITX|B_AREF_RST));
#endif
}

#if defined(CODE202)
SYS_STATUS Set_GeneralPurpose_PKT(BYTE *pData)
{
    int i ;
    BYTE uc ;

    if( pData == NULL )
    {
        return ER_FAIL ;

    }

    Switch_HDMITX_Bank(1);
    for( i = 0x38 ; i <= 0x56 ; i++)
    {
        HDMITX_WriteI2C_Byte(i, pData[i-0x38] ) ;
    }
    Switch_HDMITX_Bank(0);
    ENABLE_GeneralPurpose_PKT();
    //ENABLE_NULL_PKT() ;
    return ER_SUCCESS ;
}

BOOL
EnableVendorSpecificInfoFrame(BYTE bEnable, BYTE *pInfoFrame)
{
    BYTE checksum ;
	int i ;
    if( !bEnable)
    {
        DISABLE_GeneralPurpose_PKT();
        //DISABLE_NULL_PKT() ;
        return TRUE ;
    }

    do
    {
        if( !pInfoFrame ){ break ; }

        if( pInfoFrame[0] != 0x81 || pInfoFrame[1] != 0x01 )
        {
            break ; // is not a valid VSIP
        }

        pInfoFrame[4] = 0x03 ;
        pInfoFrame[5] = 0x0C ;
        pInfoFrame[6] = 0x00 ; // HDMI vendor specific ID

        checksum = (0 - pInfoFrame[0]- pInfoFrame[1]- pInfoFrame[2])&0xFF  ;

#ifdef CONFIG_AST1500_CAT6613
        for(i = 0 ; i < (pInfoFrame[2] & 0x1F) ; i++)
#else
        for( i = 0 ; i < pInfoFrame[2]&0x1F ; i++ )
#endif
        {
            checksum -= pInfoFrame[4+i] ;
        }
        checksum &= 0xFF ;
        pInfoFrame[3] = checksum ;

        if(Set_GeneralPurpose_PKT(pInfoFrame) == ER_SUCCESS)
        {
            return TRUE ;
        }
    }while(0) ;
    DISABLE_GeneralPurpose_PKT();
    //DISABLE_NULL_PKT() ;
    return FALSE ;
}
#endif


//~jj_tseng@chipadvanced.com 2008/08/18
//////////////////////////////////////////////////////////////////////
// Function: DumpCatHDMITXReg()
// Parameter: N/A
// Return: N/A
// Remark: Debug function,dumps the registers of CAT6611.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

void
DumpCatHDMITXReg(void)
{
#ifdef DEBUG
    int i,j ;
    BYTE ucData ;

    HDMITX_DEBUG_PRINTF("       ");
    for(j = 0 ; j < 16 ; j++)
    {
        HDMITX_DEBUG_PRINTF(" %02X",(int)j);
        if((j == 3)||(j==7)||(j==11))
        {
            HDMITX_DEBUG_PRINTF("  ");
        }
    }
    HDMITX_DEBUG_PRINTF("\n        -----------------------------------------------------\n");

    Switch_HDMITX_Bank(0);

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF("[%3X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF));
            HDMITX_DEBUG_PRINTF(" %02X",(int)ucData);
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF(" -");
            }
        }
        HDMITX_DEBUG_PRINTF("\n");
        if((i % 0x40) == 0x30)
        {
            HDMITX_DEBUG_PRINTF("        -----------------------------------------------------\n");
        }
    }

    Switch_HDMITX_Bank(1);
    for(i = 0x130; i < 0x1B0 ; i+=16)
    {
        HDMITX_DEBUG_PRINTF("[%3X]  ",i);
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF));
            HDMITX_DEBUG_PRINTF(" %02X",(int)ucData);
            if((j == 3)||(j==7)||(j==11))
            {
                HDMITX_DEBUG_PRINTF(" -");
            }
        }
        HDMITX_DEBUG_PRINTF("\n");
        if(i == 0x160)
        {
            HDMITX_DEBUG_PRINTF("        -----------------------------------------------------\n");
        }

    }
    Switch_HDMITX_Bank(0);
#endif
}
#endif
