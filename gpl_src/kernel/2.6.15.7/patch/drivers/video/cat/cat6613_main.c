/*
 * ASPEED Options
 */
#define _MCU_
#define Linux_KernelMode
//#define Linux_UserMode
//#define UBOOTMode
//#define TestMode
#define DEBUG
#define USE_THREAD 0
#define HANDLE_INT_IN_THREAD 0

/*
 * CAT6613 Options
 */
#define HDMI_TX_I2C_SLAVE_ADDR	0x9A
#define HDMI_I2C_CHANNEL	0x01
 
#define SUPPORT_EDID
#define SUPPORT_HDCP
#define SUPPORT_INPUTRGB
#define SUPPORT_INPUTYUV444
#define SUPPORT_INPUTYUV422
//#define SUPPORT_SYNCEMBEDDED
//#define SUPPORT_DEGEN

//#ifdef EXTERN_HDCPROM
//#pragma message("Defined EXTERN_HDCPROM")
//#endif // EXTERN_HDCPROM

//#ifdef SUPPORT_SYNCEMBEDDED
//#pragma message("defined SUPPORT_SYNCEMBEDDED for Sync Embedded timing input or CCIR656 input.") 
//#endif

#ifndef _MCU_ // DSSSHA need large computation data rather than 8051 supported.
#define SUPPORT_DSSSHA
#endif

#if defined(SUPPORT_INPUTYUV444) || defined(SUPPORT_INPUTYUV422)
#define SUPPORT_INPUTYUV
#endif

#if	defined(Linux_KernelMode)
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>
#include <asm/arch/gpio.h>

#include "typedef.h"
#include <asm/arch/drivers/cat6613.h>
#include <asm/arch/drivers/crt.h>
#include "cat6613_ioctl.h"
#include "cat6613_drv.h"

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


/*
 * Global
 */
static UCHAR jVersion[] = "v.0.30.00"; 
//Display Options
BYTE VIC;
ULONG VideoPixelClock;
BYTE pixelrep;
HDMI_Aspec aspec;
HDMI_Colorimetry Colorimetry;
#ifdef	TestMode
static spinlock_t cat_s_lock_i2c;
#endif
Output_Param outputparam;
BYTE bHDMIMode = FALSE, bAudioEnable = FALSE, bHDCP = FALSE;

INSTANCE InitInstanceData = 
{
		0, /* bIntType */
		0,/* | T_MODE_CCIR656 | T_MODE_SYNCEMB | T_MODE_INDDR */ // bInputVideoSignalType
		0x01,// bOutputAudioMode
		0,// bAudioChannelSwap
                0, // BYTE bAudioChannelEnable ;
                0, //BYTE bAudFs ;
                0, // unsigned long TMDSClock ;			
		FALSE,//bAuthenticated
		FALSE,// bHDMIMode
		FALSE,// bIntPOL
		FALSE,// bHPD
		FALSE // bMuteRequested
} ;

#include "mcu.h"
#include "cat6613_drv.c"

#define DISABLE_INT() do {gpio_cfg(AST1500_GPD4, AST1500_GPIO_INT_DISABLE);} while (0)

/*
 * Export functions
 */
BOOL InitCAT6613(void)
{
    	
    /* Initialization */	
    HDMITX_WriteI2C_Byte(REG_TX_INT_CTRL,InitInstanceData.bIntType) ;
    InitInstanceData.bIntPOL = (InitInstanceData.bIntType&B_INTPOL_ACTH)?TRUE:FALSE ;

    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_REF_RST|B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;
    DelayMS(1) ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;

    // Avoid power loading in un play status.
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;

    // set interrupt mask,mask value 0 is interrupt available.
    HDMITX_WriteI2C_Byte(REG_TX_INT_MASK1,0xB2) ;
    HDMITX_WriteI2C_Byte(REG_TX_INT_MASK2,0xF8) ;
    HDMITX_WriteI2C_Byte(REG_TX_INT_MASK3,0x37) ;

    Switch_HDMITX_Bank(0) ;
    DISABLE_NULL_PKT() ;
    DISABLE_ACP_PKT() ;
    DISABLE_ISRC1_PKT() ;
    DISABLE_ISRC2_PKT() ;
    DISABLE_AVI_INFOFRM_PKT() ;
    DISABLE_AUD_INFOFRM_PKT() ;
    DISABLE_SPD_INFOFRM_PKT() ;
    DISABLE_MPG_INFOFRM_PKT();

    //////////////////////////////////////////////////////////////////
    // Setup Output Audio format.
    //////////////////////////////////////////////////////////////////
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,InitInstanceData.bOutputAudioMode) ; // regE1 bOutputAudioMode should be loaded from ROM image.

#ifdef SUPPORT_HDCP
    //////////////////////////////////////////////////////////////////
    // Setup HDCP ROM
    //////////////////////////////////////////////////////////////////
    InitCAT6613_HDCPROM() ;
// #ifdef EXTERN_HDCPROM
// #pragma message("EXTERN ROM CODED") ;
// 	HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xA0) ;
// #endif
#endif

    return TRUE;
    
} /* InitCAT6613 */
EXPORT_SYMBOL(InitCAT6613);

void
DisableCAT6613(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_AREF_RST|B_VID_RST|B_AUD_RST|B_HDCP_RST) ;
    DelayMS(1) ;
    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;
}
EXPORT_SYMBOL(DisableCAT6613);

BOOL
GetEDIDData(int EDIDBlockID,BYTE *pEDIDData)
{
    if(!pEDIDData)
    {
	return FALSE ;
    }

    if(ReadEDID(pEDIDData,EDIDBlockID/2,(EDIDBlockID%2)*128,128) == ER_FAIL)
    {
        return FALSE ;
    }

    return TRUE ;
} /* GetEDIDData */
EXPORT_SYMBOL(GetEDIDData);

BOOL SetupVideoInputSignal(BYTE inputSignalType)
{
    InitInstanceData.bInputVideoSignalType = inputSignalType ;
    // SetInputMode(inputColorMode,InitInstanceData.bInputVideoSignalType) ;
    return TRUE ;
}
EXPORT_SYMBOL(SetupVideoInputSignal);

BOOL
HDMITX_ChangeDisplayOption(HDMI_Video_Type OutputVideoTiming)
{
    //HDMI_Video_Type  t=HDMI_480i60_16x9;
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
    default:	/* not HDMI mode */
        VIC = 0 ;
        pixelrep = 0 ;
        aspec = HDMI_4x3 ;
        Colorimetry = HDMI_ITU601 ;    
        return FALSE;
    }

#if 0
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
#endif

/* NO define */
#if 0
    if( Colorimetry == HDMI_ITU709 )
    {
        bInputColorMode |= F_VIDMODE_ITU709 ;
    }
    else
    {
        bInputColorMode &= ~F_VIDMODE_ITU709 ;
    }
    
    if( Colorimetry != HDMI_640x480p60)
    {
        bInputColorMode |= F_VIDMODE_16_235 ;
    }
    else
    {
        bInputColorMode &= ~F_VIDMODE_16_235 ;
    }
#endif

    return TRUE;

} /* HDMITX_ChangeDisplayOption */
//EXPORT_SYMBOL(HDMITX_ChangeDisplayOption);
 
BOOL HDMITX_SetOutput(BYTE bInputColorMode,BYTE bOutputColorMode,
                      HDMI_Video_Type OutputVideoTiming, ULONG VCLK,
                      BYTE  bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF,
                      BYTE  bCapibility, BYTE ColorDepth)
{
    ErrorF("HDMITX_SetOutput() \n");
    
    /* Set Capability */
    bHDMIMode = bCapibility & CAP_HDMI;
    bHDCP = bCapibility & CAP_HDCP;    
    bAudioEnable = bCapibility & CAP_AUDIO;

    /* Change Display Options */
    if (HDMITX_ChangeDisplayOption(OutputVideoTiming) == FALSE)
    {
        ErrorF("Not HDMI Video Timing");	
        VideoPixelClock = VCLK;
        //bHDMIMode = FALSE;    	
    }

    /* Update to Instance */
    InitInstanceData.TMDSClock = VideoPixelClock ;
	
    /* Enable Video */    
    EnableVideoOutput(VideoPixelClock, bInputColorMode, bOutputColorMode,bHDMIMode, ColorDepth) ;
    
    /* Enable HDMI */
    if( bHDMIMode )
        ConfigAVIInfoFrame(bOutputColorMode, VIC, pixelrep) ;
	else
	{
		EnableAVIInfoFrame(FALSE ,NULL);
	}
        
    /* Enable HDCP */    
    if (bHDCP)
        EnableHDCP(TRUE) ;
        
    /* Enable Audio */
    if( bAudioEnable )
    {
        EnableAudioOutput(bAudioSampleFreq, ChannelNumber, bAudSWL, bSPDIF);            
        ConfigAudioInfoFrm() ;
        SetupAudioChannel() ;
    }
    _SetAVMute(FALSE) ;
    
    /* save output param */
    outputparam.UseOutputParam = TRUE;
    outputparam.bInputColorMode = bInputColorMode;
    outputparam.bOutputColorMode = bOutputColorMode;
    outputparam.OutputVideoTiming = OutputVideoTiming;
    outputparam.VCLK = VCLK;
    outputparam.bAudioSampleFreq = bAudioSampleFreq;
    outputparam.ChannelNumber = ChannelNumber;
    outputparam.bAudSWL = bAudSWL;
    outputparam.bSPDIF = bSPDIF;
    outputparam.bCapibility = bCapibility;
	outputparam.ColorDepth = ColorDepth;
   
    return TRUE;
    
} /* HDMITX_SetOutput */
EXPORT_SYMBOL(HDMITX_SetOutput);

BOOL HDMITX_DisplayOutput(BYTE bInputColorMode,BYTE bOutputColorMode,
                          HDMI_Video_Type OutputVideoTiming, ULONG VCLK,
                          BYTE  bCapibility, BYTE ColorDepth)
{
    ErrorF("HDMITX_DisplayOutput() \n");
    
    /* Set Capability */
    bHDMIMode = bCapibility & CAP_HDMI;
    bHDCP = bCapibility & CAP_HDCP;    

    /* Change Display Options */
    if (HDMITX_ChangeDisplayOption(OutputVideoTiming) == FALSE)
    {
        ErrorF("Not HDMI Video Timing");	
        VideoPixelClock = VCLK;
        //bHDMIMode = FALSE;    	
    }

    /* Update to Instance */
    InitInstanceData.TMDSClock = VideoPixelClock ;
	
    /* Enable Video */    
    EnableVideoOutput(VideoPixelClock, bInputColorMode, bOutputColorMode,bHDMIMode, ColorDepth) ;
    
    /* Enable HDMI */
    if( bHDMIMode )
        ConfigAVIInfoFrame(bOutputColorMode, VIC, pixelrep) ;
	else
	{
		EnableAVIInfoFrame(FALSE ,NULL);
	}
        
    /* Enable HDCP */    
    if (bHDCP)
        EnableHDCP(TRUE) ;

    /* Enable Audio */
    if( bAudioEnable )
    {
        EnableAudioOutput(outputparam.bAudioSampleFreq, outputparam.ChannelNumber, outputparam.bAudSWL, outputparam.bSPDIF);            
        ConfigAudioInfoFrm() ;
        SetupAudioChannel() ;
    }
        
    _SetAVMute(FALSE) ;

    /* save output param */
    outputparam.UseOutputParam = TRUE;
    outputparam.bInputColorMode = bInputColorMode;
    outputparam.bOutputColorMode = bOutputColorMode;
    outputparam.OutputVideoTiming = OutputVideoTiming;
    outputparam.VCLK = VCLK;
    outputparam.bCapibility = (outputparam.bCapibility & ~(CAP_HDMI | CAP_HDCP)) | bCapibility;
	outputparam.ColorDepth = ColorDepth;
     
    return TRUE;
    
} /* HDMITX_DisplayOutput */
EXPORT_SYMBOL(HDMITX_DisplayOutput);

BOOL HDMITX_AudioOutput(BYTE bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF)
{
    ErrorF("HDMITX_AudioOutput() \n");

    bAudioEnable = TRUE;

#if I2S_DBG
	/*
	** Enabling video causes screen flashing. It is not necessary.
	*/
    EnableVideoOutput(VideoPixelClock, 
                                    outputparam.bInputColorMode, 
                                    outputparam.bOutputColorMode,
                                    (outputparam.bCapibility & CAP_HDMI),
                                    outputparam.ColorDepth);

    ConfigAVIInfoFrame(outputparam.bOutputColorMode, VIC, pixelrep) ;

    /* Enable HDCP */    
    if (bHDCP)
        EnableHDCP(TRUE) ;
#endif //#if I2S_DBG

    {
        EnableAudioOutput(bAudioSampleFreq, ChannelNumber, bAudSWL, bSPDIF);            
        ConfigAudioInfoFrm() ;
        SetupAudioChannel() ;
    }
    _SetAVMute(FALSE) ;

    /* save output param */
    outputparam.UseOutputParam = TRUE;
    outputparam.bAudioSampleFreq = bAudioSampleFreq;
    outputparam.ChannelNumber = ChannelNumber;
    outputparam.bAudSWL = bAudSWL;
    outputparam.bSPDIF = bSPDIF;
    outputparam.bCapibility = outputparam.bCapibility | CAP_AUDIO;
    
    return TRUE;
    
} /* HDMITX_AudioOutput */
EXPORT_SYMBOL(HDMITX_AudioOutput);

void HDMITX_Reconfig(void)
{
	if (outputparam.UseOutputParam == TRUE){
		HDMITX_SetOutput(outputparam.bInputColorMode, \
			outputparam.bOutputColorMode, \
			outputparam.OutputVideoTiming, \
			outputparam.VCLK, \
			outputparam.bAudioSampleFreq, \
			outputparam.ChannelNumber, \
			outputparam.bAudSWL, \
			outputparam.bSPDIF, \
			outputparam.bCapibility,
			outputparam.ColorDepth);
		if (InitInstanceData.bMuteRequested)
			_SetAVMute(1);
	}
}
EXPORT_SYMBOL(HDMITX_Reconfig);

#if	defined(Linux_KernelMode)
MODULE_LICENSE ("GPL");

int  ioaccess_major = 0;

int  ioaccess_ioctl (struct inode *inode, struct file *filp, unsigned cmd, unsigned long arg);

struct file_operations ioaccess_fops = {
    ioctl:    ioaccess_ioctl,
};

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
        if (HDMITX_SetOutput(output_option.bInputColorMode, output_option.bOutputColorMode, \
                             output_option.OutputVideoTiming, output_option.VCLK, \
                             output_option.bAudioSampleFreq, output_option.ChannelNumber, output_option.bAudSWL, output_option.bSPDIF, \
                             output_option.bCapibility, output_option.ColorDepth) == FALSE)
        {                     
            ret = -1;
        }
        
        break;    
            
    default:
        ret = -1;    	
    }
    
    return ret;
}

static irqreturn_t cat6613_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
    ULONG uldata;
    	
    /* Check if the interrupt is triggered by CAT6613 */
#if 0    
    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x18));
    if (uldata & 0x10000000)
    {	/* clear IRQ */
        *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x18)) = uldata | 0x10000000;
    }
    else
    {	/* no handle */
        return IRQ_NONE;    	
    }
#endif    	    
    //check share interrupt
    if (!gpio_get_int_stat(AST1500_GPD4)) {
	return IRQ_NONE;
    }
    ErrorF("In IRQ routine .... \n")
     
    //ack interrupt
    gpio_ack_int_stat(AST1500_GPD4);

    /* IRQ Handle */
    CheckHDMITX();
    
    ErrorF("exit IRQ routine\n")
    return IRQ_HANDLED;
} /* cat6613_interrupt */

int my_init(void)
{
    int    result, retval;
    USHORT VendorID, DeviceID; 	
    BYTE pData[4];
    ULONG uldata;

    INFO("Load CAT6613 Driver %s \n", jVersion);
//#ifdef CONFIG_ARCH_AST1500_HOST
	//release CAT6613 from reset state
	gpio_direction_output(AST1500_GPD5, 1);
//#endif
	

    /* I2C init */
#ifdef	TestMode
    spin_lock_init(&cat_s_lock_i2c);
    I2CInit(HDMI_I2C_CHANNEL);
#endif    
    	
    /* Check ID */
    Switch_HDMITX_Bank(0) ;    
    HDMITX_ReadI2C_ByteN(0x00, pData, 4);
    VendorID = *(USHORT *)(pData) & 0xFF00;
    DeviceID = *(USHORT *)(pData + 2) & 0x0FFF;
    if ((VendorID != 0xCA00) || (DeviceID != 0x0613))
    {
        INFO("Can't find CAT6613 \n");
        ErrorF("Get VendorID:%x, DeviceID:%x \n", VendorID, DeviceID);
        return -1;	
    }	

    result = register_chrdev (ioaccess_major, "cat6613", &ioaccess_fops);
    if (result < 0) {
        INFO("Can't get major.\n");
        return result;
    }
    
    if (ioaccess_major == 0) {
        ioaccess_major = result;
        INFO("A ioaccess_major = %d\n", ioaccess_major);
    }
     
    /* request IRQ */
    retval = request_irq (20, &cat6613_interrupt, SA_SHIRQ, "cat6613", "cat6613");
    if (retval) {
        INFO("Unable to get IRQ");
        return -1;
    }

    /* Init */
    InitCAT6613() ;

    /* init irq: GPIOD4 low-level trigger */
#if 0    
    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x10));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x10)) = uldata | 0x10000000;

    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x0c));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x0c)) = uldata & 0xEFFFFFFF;
     
    uldata = *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x08));
    *(u32 *)(IO_ADDRESS(ASPEED_GPIO_BASE + 0x08)) = uldata | 0x10000000;
#endif
    gpio_cfg(AST1500_GPD4, AST1500_GPIO_INT_LEVEL_LOW);

#ifdef CONFIG_ARCH_AST1500_HOST
	//enable loopback audio Tx
	HDMITX_AudioOutput(2, 2, 24, 1);
	//disable loopback Tx by default
	SetAVMute(1);
#endif

    return 0;
}
                                                                                
void my_cleanup(void)
{
    INFO("UnLoad CAT6613 \n");

    /* Disable interrupt */
    DISABLE_INT();

    DisableCAT6613();
    
    unregister_chrdev (ioaccess_major, "cat6613");
    free_irq (20, "cat6613");
    
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
