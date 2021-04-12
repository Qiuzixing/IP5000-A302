#ifndef _hal_h_
#define _hal_h_

#include "sys.h"

EXTERN_C_BEGIN

#include "hal_acute.h"
//#include "hal_mmp2.h"
//#include "hal_vimicro.h"
//#include "hal_test.h"
//#include "hal_ipcam.h"
//#include "hal_krfc.h"
//#include "hal_HS.h"
//#include "hal_hardi.h"


/* IPENB : IP ENable Bit */
#define HAL_IPENB_SMCC		((unsigned int) (1 << 0))
#define HAL_IPENB_SDC		((unsigned int) (1 << 1))
#define HAL_IPENB_DAI2		((unsigned int) (1 << 2))
#define HAL_IPENB_AC97		((unsigned int) (1 << 3))
#define HAL_IPENB_UART1		((unsigned int) (1 << 6))
#define HAL_IPENB_SSI1		((unsigned int) (1 << 8))
#define HAL_IPENB_I2C3		((unsigned int) (1 << 9))	/* LED, SAA1064 */
#define HAL_IPENB_I2C2		((unsigned int) (1 << 10))	/* I2S, UDA1380 */
#define HAL_IPENB_I2C1		((unsigned int) (1 << 11))	/* VFE, SAA7111A */
#define HAL_IPENB_PWM		((unsigned int) (1 << 12))
#define HAL_IPENB_WDT		((unsigned int) (1 << 13))
#define HAL_IPENB_TIMER2	((unsigned int) (1 << 14))
#define HAL_IPENB_TIMER1	((unsigned int) (1 << 15))
#define HAL_IPENB_GPIO		((unsigned int) (1 << 16))
#define HAL_IPENB_JPEG		((unsigned int) (1 << 18))
#define HAL_IPENB_DMAM		((unsigned int) (1 << 20))	/* DMA, memory */
#define HAL_IPENB_TV		((unsigned int) (1 << 21))
#define HAL_IPENB_LCD		((unsigned int) (1 << 23))
#define HAL_IPENB_CPTR		((unsigned int) (1 << 24))	/* i.e. VFE */
#define HAL_IPENB_EBDSRAM	((unsigned int) (1 << 25))	/* i.e. internal SRAM */
#define HAL_IPENB_RTC		((unsigned int) (1 << 26))
#define HAL_IPENB_DMAP		((unsigned int) (1 << 27))	/* DMA, peripheral */
#define HAL_IPENB_USB20		((unsigned int) (1 << 28))
#define HAL_IPENB_ETH110	((unsigned int) (1 << 30))


/* SYS */
#define HAL_REG_SYS_AdoClkCtrl			(HAL_REG_SYS_BASE+0x10)
#define HAL_REG_SYS_JumpSts				(HAL_REG_SYS_BASE+0x18)
#define HAL_REG_SYS_ModeSts				(HAL_REG_SYS_BASE+0x1C)

	/* 
	In the integration for HelloSoft, the IPSts and VerNum have
	been moved to 0x78 and 0x7C respectively.
	In the other integrations, these should be checked.
	*/
#ifndef _hal_HS_h_
#define HAL_REG_SYS_IPSts				(HAL_REG_SYS_BASE+0x20)
#define HAL_REG_SYS_VerNum				(HAL_REG_SYS_BASE+0x24)
#else
#define HAL_REG_SYS_IPSts				(HAL_REG_SYS_BASE+0x78)
#define HAL_REG_SYS_VerNum				(HAL_REG_SYS_BASE+0x7C)
#endif


/* CompactFlash controller */
#if defined(HAL_INC_CF)

#define HAL_REG_CFC_LBA					(HAL_REG_CFC_BASE+0x00)
#define HAL_REG_CFC_ATA					(HAL_REG_CFC_BASE+0x04)
#define HAL_REG_CFC_INITWAIT			(HAL_REG_CFC_BASE+0x08)
#define HAL_REG_CFC_FLAG				(HAL_REG_CFC_BASE+0x0C)
#define HAL_REG_CFC_MASK0				(HAL_REG_CFC_BASE+0x10)
#define HAL_REG_CFC_MODE				(HAL_REG_CFC_BASE+0x14)
#define HAL_REG_CFC_PWR			     	(HAL_REG_CFC_BASE+0x18)
#define HAL_REG_CFC_RSTWAIT				(HAL_REG_CFC_BASE+0x1C)
#define HAL_REG_CFC_DATA				(HAL_REG_CFC_BASE+0x20)
#define HAL_REG_CFC_MAN_DATA			(HAL_REG_CFC_BASE+0x00)
#define HAL_REG_CFC_MAN_ADDR			(HAL_REG_CFC_BASE+0x04)

//#define HAL_REG_CFC_LBA					(HAL_REG_CFC_BASE+0x00)
//#define HAL_REG_CFC_ATA					(HAL_REG_CFC_BASE+0x04)
//#define HAL_REG_CFC_INITWAIT			(HAL_REG_CFC_BASE+0x08)
//#define HAL_REG_CFC_FLAG				(HAL_REG_CFC_BASE+0x0C)
//#define HAL_REG_CFC_MASK0				(HAL_REG_CFC_BASE+0x10)
//#define HAL_REG_CFC_MASK1				(HAL_REG_CFC_BASE+0x14)
//#define HAL_REG_CFC_DATA				(HAL_REG_CFC_BASE+0x20)

#endif

/* DMAM */
#if defined(HAL_INC_DMAM)
#define HAL_REG_DMAM_SrcAddr			(HAL_REG_DMAM_BASE+0x0)
#define HAL_REG_DMAM_DestAddr			(HAL_REG_DMAM_BASE+0x4)
#define HAL_REG_DMAM_Ctrl				(HAL_REG_DMAM_BASE+0x8)
#define HAL_REG_DMAM_Enable				(HAL_REG_DMAM_BASE+0xC)
#endif

/* DMAP */
#if defined(HAL_INC_DMAP)
#define HAL_REG_DMAP_IntStat		(HAL_REG_DMAP_BASE+0x000)
#define HAL_REG_DMAP_IntTCStat		(HAL_REG_DMAP_BASE+0x004)
#define HAL_REG_DMAP_IntTCClr		(HAL_REG_DMAP_BASE+0x008)
#define HAL_REG_DMAP_IntErrStat		(HAL_REG_DMAP_BASE+0x00C)
#define HAL_REG_DMAP_IntErrClr		(HAL_REG_DMAP_BASE+0x010)
#define HAL_REG_DMAP_RawIntTCStat	(HAL_REG_DMAP_BASE+0x014)
#define HAL_REG_DMAP_RawIntErrStat	(HAL_REG_DMAP_BASE+0x018)
#define HAL_REG_DMAP_EnbldChns		(HAL_REG_DMAP_BASE+0x01C)
#define HAL_REG_DMAP_Sync			(HAL_REG_DMAP_BASE+0x024)

	/* each channel's base address */
#define HAL_REG_DMAP_C0_BASE		(HAL_REG_DMAP_BASE+0x100)
#define HAL_REG_DMAP_C1_BASE		(HAL_REG_DMAP_BASE+0x110)
#define HAL_REG_DMAP_C2_BASE		(HAL_REG_DMAP_BASE+0x120)
#define HAL_REG_DMAP_C3_BASE		(HAL_REG_DMAP_BASE+0x130)
#define HAL_REG_DMAP_C4_BASE		(HAL_REG_DMAP_BASE+0x140)
#define HAL_REG_DMAP_C5_BASE		(HAL_REG_DMAP_BASE+0x150)
#define HAL_REG_DMAP_C6_BASE		(HAL_REG_DMAP_BASE+0x160)
#define HAL_REG_DMAP_C7_BASE		(HAL_REG_DMAP_BASE+0x170)

	/* offsets of each channel's registers (w.r.t. HAL_REG_DMAP_Cn_BASE) */
	/*                                                           |       */
	/*                                                           |       */
	/*                                    7,6,5,4,3,2,1,0 -------+       */
#define HAL_DMAP_Cn_SrcAddr_Offset	0x0
#define HAL_DMAP_Cn_DestAddr_Offset	0x4
#define HAL_DMAP_Cn_Ctrl_Offset		0x8
#define HAL_DMAP_Cn_Config			0xC
#endif

/* watchdog timer (WDT) */
#if defined(HAL_INC_WDT)
#define HAL_REG_WDT_CntSts				(HAL_REG_WDT_BASE+0x00)
#define HAL_REG_WDT_Reload				(HAL_REG_WDT_BASE+0x04)
#define HAL_REG_WDT_Restart				(HAL_REG_WDT_BASE+0x08)
#define HAL_REG_WDT_Ctrl				(HAL_REG_WDT_BASE+0x0C)
#define HAL_REG_WDT_TimeOut				(HAL_REG_WDT_BASE+0x10)
#define HAL_REG_WDT_Clr					(HAL_REG_WDT_BASE+0x14)
#define HAL_REG_WDT_RstWd				(HAL_REG_WDT_BASE+0x18)
#endif

/* RTC */
#define HAL_REG_RTC_CounterStatus		(HAL_REG_RTC_BASE+0x00)
#define HAL_REG_RTC_Alarm				(HAL_REG_RTC_BASE+0x04)
#define HAL_REG_RTC_Reload				(HAL_REG_RTC_BASE+0x08)
#define HAL_REG_RTC_Control				(HAL_REG_RTC_BASE+0x0C)
#define HAL_REG_RTC_Restart				(HAL_REG_RTC_BASE+0x10)
#define HAL_REG_RTC_Reset				(HAL_REG_RTC_BASE+0x14)

/* VIC */
#define HAL_REG_VIC1_IRQStatus			(HAL_REG_VIC1_BASE+0x00)
#define HAL_REG_VIC1_FIQStatus			(HAL_REG_VIC1_BASE+0x04)
#define HAL_REG_VIC1_RawIntStatus		(HAL_REG_VIC1_BASE+0x08)
#define HAL_REG_VIC1_IntSelect			(HAL_REG_VIC1_BASE+0x0C)
#define HAL_REG_VIC1_IntEnable			(HAL_REG_VIC1_BASE+0x10)
#define HAL_REG_VIC1_IntEnableClear		(HAL_REG_VIC1_BASE+0x14)
#define HAL_REG_VIC1_SoftInt			(HAL_REG_VIC1_BASE+0x18)
#define HAL_REG_VIC1_SoftIntClear		(HAL_REG_VIC1_BASE+0x1C)
#define HAL_REG_VIC1_ProtectionEnable	(HAL_REG_VIC1_BASE+0x20)
#define HAL_REG_VIC1_Sense				(HAL_REG_VIC1_BASE+0x24)
#define HAL_REG_VIC1_BothEdge			(HAL_REG_VIC1_BASE+0x28)
#define HAL_REG_VIC1_Event				(HAL_REG_VIC1_BASE+0x2C)
#define HAL_REG_VIC1_EdgeTriClear		(HAL_REG_VIC1_BASE+0x38)

#if HAL_VIC_NUM == 2
#define HAL_REG_VIC2_IRQStatus			(HAL_REG_VIC2_BASE+0x00)
#define HAL_REG_VIC2_FIQStatus			(HAL_REG_VIC2_BASE+0x04)
#define HAL_REG_VIC2_RawIntStatus		(HAL_REG_VIC2_BASE+0x08)
#define HAL_REG_VIC2_IntSelect			(HAL_REG_VIC2_BASE+0x0C)
#define HAL_REG_VIC2_IntEnable			(HAL_REG_VIC2_BASE+0x10)
#define HAL_REG_VIC2_IntEnableClear		(HAL_REG_VIC2_BASE+0x14)
#define HAL_REG_VIC2_SoftInt			(HAL_REG_VIC2_BASE+0x18)
#define HAL_REG_VIC2_SoftIntClear		(HAL_REG_VIC2_BASE+0x1C)
#define HAL_REG_VIC2_ProtectionEnable	(HAL_REG_VIC2_BASE+0x20)
#define HAL_REG_VIC2_Sense				(HAL_REG_VIC2_BASE+0x24)
#define HAL_REG_VIC2_BothEdge			(HAL_REG_VIC2_BASE+0x28)
#define HAL_REG_VIC2_Event				(HAL_REG_VIC2_BASE+0x2C)
#define HAL_REG_VIC2_EdgeTriClear		(HAL_REG_VIC2_BASE+0x38)
#endif

/* JPEGD; "JPEG decoder only" IP */
#if defined(HAL_INC_JPEGD)
#define HAL_REG_JPEGD_SWReset			(HAL_REG_JPEG_BASE+0x40)
#define HAL_REG_JPEGD_Start				(HAL_REG_JPEG_BASE+0x44)
#define HAL_REG_JPEGD_DisStopAfterSOS	(HAL_REG_JPEG_BASE+0x48)
#define HAL_REG_JPEGD_SysLittleEndian	(HAL_REG_JPEG_BASE+0x4C)
#define HAL_REG_JPEGD_IntMask			(HAL_REG_JPEG_BASE+0x50)
#define HAL_REG_JPEGD_ClearInt			(HAL_REG_JPEG_BASE+0x54)
#define HAL_REG_JPEGD_FIFOThdHigh		(HAL_REG_JPEG_BASE+0x58)
#define HAL_REG_JPEGD_FIFOThdLow		(HAL_REG_JPEG_BASE+0x5C)
#define HAL_REG_JPEGD_YAddr				(HAL_REG_JPEG_BASE+0x84)
#define HAL_REG_JPEGD_CbAddr			(HAL_REG_JPEG_BASE+0x88)
#define HAL_REG_JPEGD_CrAddr			(HAL_REG_JPEG_BASE+0x8C)
#define HAL_REG_JPEGD_BSAddr			(HAL_REG_JPEG_BASE+0x90)
#define HAL_REG_JPEGD_IntRawSts			(HAL_REG_JPEG_BASE+0xC0)
#define HAL_REG_JPEGD_IntMaskedSts		(HAL_REG_JPEG_BASE+0xC4)
#define HAL_REG_JPEGD_DecImgWidth		(HAL_REG_JPEG_BASE+0xC8)
#define HAL_REG_JPEGD_DecImgHeight		(HAL_REG_JPEG_BASE+0xCC)
#define HAL_REG_JPEGD_YSamFact			(HAL_REG_JPEG_BASE+0xD0)
#define HAL_REG_JPEGD_CbSamFact			(HAL_REG_JPEG_BASE+0xD4)
#define HAL_REG_JPEGD_CrSamFact			(HAL_REG_JPEG_BASE+0xD8)
#endif

/* JPEG Controller */
#if defined(HAL_INC_JPEG2)
#define HAL_REG_JPEG_Ctrl				(HAL_REG_JPEG_BASE+0x00)
#define HAL_REG_JPEG_EncImgWidthH		(HAL_REG_JPEG_BASE+0x04)
#define HAL_REG_JPEG_EncImgWidthL		(HAL_REG_JPEG_BASE+0x08)
#define HAL_REG_JPEG_EncImgHeightH		(HAL_REG_JPEG_BASE+0x0C)
#define HAL_REG_JPEG_EncImgHeightL		(HAL_REG_JPEG_BASE+0x10)
	/* ?? 0x24 */
	/* ?? 0x28 */
#define HAL_REG_JPEG_SWReset			(HAL_REG_JPEG_BASE+0x40)
#define HAL_REG_JPEG_Start				(HAL_REG_JPEG_BASE+0x44)

#define HAL_REG_JPEG_SWGlobalReset		(HAL_REG_JPEG_BASE+0x80)
#define HAL_REG_JPEG_YAddr				(HAL_REG_JPEG_BASE+0x84)
#define HAL_REG_JPEG_CbAddr				(HAL_REG_JPEG_BASE+0x88)
#define HAL_REG_JPEG_CrAddr				(HAL_REG_JPEG_BASE+0x8C)
#define HAL_REG_JPEG_BSAddr				(HAL_REG_JPEG_BASE+0x90)
#define HAL_REG_JPEG_WFIFOAddr			(HAL_REG_JPEG_BASE+0x94)

#define HAL_REG_JPEG_DecStatus			(HAL_REG_JPEG_BASE+0xC0)
#define HAL_REG_JPEG_DecImgWidthH		(HAL_REG_JPEG_BASE+0xC4)
#define HAL_REG_JPEG_DecImgWidthL		(HAL_REG_JPEG_BASE+0xC8)
#define HAL_REG_JPEG_DecImgHeightH		(HAL_REG_JPEG_BASE+0xCC)
#define HAL_REG_JPEG_DecImgHeightL		(HAL_REG_JPEG_BASE+0xD0)
#define HAL_REG_JPEG_YSamFact			(HAL_REG_JPEG_BASE+0xD4)
#define HAL_REG_JPEG_CbSamFact			(HAL_REG_JPEG_BASE+0xD8)
#define HAL_REG_JPEG_CrSamFact			(HAL_REG_JPEG_BASE+0xDC)
#define HAL_REG_JPEG_BSLengthMSByte		(HAL_REG_JPEG_BASE+0xE0)
#define HAL_REG_JPEG_BSLength2ndMSByte	(HAL_REG_JPEG_BASE+0xE4)
#define HAL_REG_JPEG_BSLength2ndLSByte	(HAL_REG_JPEG_BASE+0xE8)
#define HAL_REG_JPEG_BSLengthLSByte		(HAL_REG_JPEG_BASE+0xEC)
#define HAL_REG_JPEG_LmnceQTbl			(HAL_REG_JPEG_BASE+0x200)
#define HAL_REG_JPEG_CmnceQTbl			(HAL_REG_JPEG_BASE+0x300)
#endif

/* JPEG Controller (decoder enhancement version) */
#if defined(HAL_INC_JPEGC)
#define HAL_REG_JPEGC_Ctrl				(HAL_REG_JPEG_BASE+0x0)
#define HAL_REG_JPEGC_EncImgWidthH		(HAL_REG_JPEG_BASE+0x4)
#define HAL_REG_JPEGC_EncImgWidthL		(HAL_REG_JPEG_BASE+0x8)
#define HAL_REG_JPEGC_EncImgHeightH		(HAL_REG_JPEG_BASE+0xC)
#define HAL_REG_JPEGC_EncImgHeightL		(HAL_REG_JPEG_BASE+0x10)
#define HAL_REG_JPEGC_IFIFOHighT		(HAL_REG_JPEG_BASE+0x14)
#define HAL_REG_JPEGC_IFIFOLowT			(HAL_REG_JPEG_BASE+0x18)
#define HAL_REG_JPEGC_OFIFOHighT		(HAL_REG_JPEG_BASE+0x1C)
#define HAL_REG_JPEGC_OFIFOLowT			(HAL_REG_JPEG_BASE+0x20)
#define HAL_REG_JPEGC_SWReset			(HAL_REG_JPEG_BASE+0x40)
#define HAL_REG_JPEGC_Start				(HAL_REG_JPEG_BASE+0x44)
#define HAL_REG_JPEGC_IStartAddr		(HAL_REG_JPEG_BASE+0x48)
#define HAL_REG_JPEGC_OStartAddr		(HAL_REG_JPEG_BASE+0x4C)
#define HAL_REG_JPEGC_IStopAddr			(HAL_REG_JPEG_BASE+0x50)
#define HAL_REG_JPEGC_Get				(HAL_REG_JPEG_BASE+0x54)
#define HAL_REG_JPEGC_BlkStart			(HAL_REG_JPEG_BASE+0x58)
#define HAL_REG_JPEGC_Release			(HAL_REG_JPEG_BASE+0x5C)
#define HAL_REG_JPEGC_Status			(HAL_REG_JPEG_BASE+0x80)
#define HAL_REG_JPEGC_Ownership			(HAL_REG_JPEG_BASE+0x84)
#define HAL_REG_JPEGC_DecodeStatus		(HAL_REG_JPEG_BASE+0xC0)
#define HAL_REG_JPEGC_DecImgWidthH		(HAL_REG_JPEG_BASE+0xC4)
#define HAL_REG_JPEGC_DecImgWidthL		(HAL_REG_JPEG_BASE+0xC8)
#define HAL_REG_JPEGC_DecImgHeightH		(HAL_REG_JPEG_BASE+0xCC)
#define HAL_REG_JPEGC_DecImgHeightL		(HAL_REG_JPEG_BASE+0xD0)
#define HAL_REG_JPEGC_LumSampleFactor	(HAL_REG_JPEG_BASE+0xD4)
#define HAL_REG_JPEGC_CbSampleFactor	(HAL_REG_JPEG_BASE+0xD8)
#define HAL_REG_JPEGC_CrSampleFactor	(HAL_REG_JPEG_BASE+0xDC)
#define HAL_REG_JPEGC_LumQTable			(HAL_REG_JPEG_BASE+0x200)
#define HAL_REG_JPEGC_ChrQTable			(HAL_REG_JPEG_BASE+0x300)
#endif

/* LCD */
#if defined(HAL_INC_LCD)
#define HAL_REG_LCD_HCtrlR				(HAL_REG_LCD_BASE+0x00)
#define HAL_REG_LCD_VCtrlR				(HAL_REG_LCD_BASE+0x04)
#define HAL_REG_LCD_IFormR				(HAL_REG_LCD_BASE+0x08)
#define HAL_REG_LCD_DOutR				(HAL_REG_LCD_BASE+0x0C)
#define HAL_REG_LCD_TCtrlR				(HAL_REG_LCD_BASE+0x10)
#define HAL_REG_LCD_BaseR				(HAL_REG_LCD_BASE+0x18)
#define HAL_REG_LCD_IntCtrlR			(HAL_REG_LCD_BASE+0x24)
#define HAL_REG_LCD_IntStsR				(HAL_REG_LCD_BASE+0x28)
#define HAL_REG_LCD_DCtrlR				(HAL_REG_LCD_BASE+0x2C)
#define HAL_REG_LCD_GPIOFR				(HAL_REG_LCD_BASE+0x30)
#define HAL_REG_LCD_GPIOVR				(HAL_REG_LCD_BASE+0x34)
#define HAL_REG_LCD_TModeR				(HAL_REG_LCD_BASE+0x38)
#define HAL_REG_LCD_HALIGNR				(HAL_REG_LCD_BASE+0x3C)
#define HAL_REG_LCD_VALIGNR				(HAL_REG_LCD_BASE+0x40)
#define	HAL_REG_LCD_BCOLORR				(HAL_REG_LCD_BASE+0x44)
#define HAL_REG_LCD_PIXARRR				(HAL_REG_LCD_BASE+0x48)
#define HAL_REG_LCD_PALER				(HAL_REG_LCD_BASE+0x100)
#endif

/* TV (master) */
#if defined(HAL_INC_TV)
#define HAL_REG_TV_ContractCoef			(HAL_REG_TV_BASE+0x0)
#define HAL_REG_TV_SaturationCoef		(HAL_REG_TV_BASE+0x8)
#define HAL_REG_TV_Test1				(HAL_REG_TV_BASE+0xC)
#define HAL_REG_TV_Ctrl1				(HAL_REG_TV_BASE+0x10)
#define HAL_REG_TV_Ctrl2				(HAL_REG_TV_BASE+0x14)
#define HAL_REG_TV_ReConfigCSCCoef		(HAL_REG_TV_BASE+0x18)
#define HAL_REG_TV_CbCoef				(HAL_REG_TV_BASE+0x1C)
#define HAL_REG_TV_YCoef				(HAL_REG_TV_BASE+0x20)
#define HAL_REG_TV_CrCoef				(HAL_REG_TV_BASE+0x24)
#define HAL_REG_TV_Test2				(HAL_REG_TV_BASE+0x28)
#define HAL_REG_TV_BurstParam1H			(HAL_REG_TV_BASE+0x2C)
#define HAL_REG_TV_BurstParam1L			(HAL_REG_TV_BASE+0x30)
#define HAL_REG_TV_BurstParam2H			(HAL_REG_TV_BASE+0x34)
#define HAL_REG_TV_BurstParam2L			(HAL_REG_TV_BASE+0x38)
#define HAL_REG_TV_BurstParam3H			(HAL_REG_TV_BASE+0x3C)
#define HAL_REG_TV_BurstParam3L			(HAL_REG_TV_BASE+0x40)
#define HAL_REG_TV_Field0StartAddr		(HAL_REG_TV_BASE+0x100)
#define HAL_REG_TV_Field1StartAddr		(HAL_REG_TV_BASE+0x104)
#define HAL_REG_TV_ImageWidth			(HAL_REG_TV_BASE+0x108)
#define HAL_REG_TV_ImageHeight			(HAL_REG_TV_BASE+0x10C)
#define HAL_REG_TV_HorizontalOffset		(HAL_REG_TV_BASE+0x110)
#define HAL_REG_TV_VerticalOffset		(HAL_REG_TV_BASE+0x114)
#define HAL_REG_TV_Reset				(HAL_REG_TV_BASE+0x118)
#define HAL_REG_TV_Enable				(HAL_REG_TV_BASE+0x11C)

#define HAL_REG_TV_BGColor				(HAL_REG_TV_BASE+0x124)
#define HAL_REG_TV_CtrlReg10			(HAL_REG_TV_BASE+0x128)
#define HAL_REG_TV_CropWidth			(HAL_REG_TV_BASE+0x12C)
#define HAL_REG_TV_CropHeight			(HAL_REG_TV_BASE+0x130)
#define HAL_REG_TV_CropHorizontalOffset	(HAL_REG_TV_BASE+0x134)
#define HAL_REG_TV_CropVerticalOffset	(HAL_REG_TV_BASE+0x138)
#define HAL_REG_TV_FIFO_LO_THD			(HAL_REG_TV_BASE+0x170)
#define HAL_REG_TV_FIFO_HI_THD			(HAL_REG_TV_BASE+0x174)
#endif

/* USB2.0 */
#if defined(HAL_INC_USB20)
	/* register definition is seen in "halusbmass.c" file */
#endif

/* Timer */
#if defined(HAL_INC_TIMER1)
#define HAL_REG_TIMER1CNT1_Status		(HAL_REG_TIMER1_BASE+0x00)
#define HAL_REG_TIMER1CNT1_Reload		(HAL_REG_TIMER1_BASE+0x04)
#define HAL_REG_TIMER1CNT1_Match1		(HAL_REG_TIMER1_BASE+0x08)
#define HAL_REG_TIMER1CNT1_Match2		(HAL_REG_TIMER1_BASE+0x0C)

#define HAL_REG_TIMER1CNT2_Status		(HAL_REG_TIMER1_BASE+0x10)
#define HAL_REG_TIMER1CNT2_Reload		(HAL_REG_TIMER1_BASE+0x14)
#define HAL_REG_TIMER1CNT2_Match1		(HAL_REG_TIMER1_BASE+0x18)
#define HAL_REG_TIMER1CNT2_Match2		(HAL_REG_TIMER1_BASE+0x1C)

#define HAL_REG_TIMER1CNT3_Status		(HAL_REG_TIMER1_BASE+0x20)
#define HAL_REG_TIMER1CNT3_Reload		(HAL_REG_TIMER1_BASE+0x24)
#define HAL_REG_TIMER1CNT3_Match1		(HAL_REG_TIMER1_BASE+0x28)
#define HAL_REG_TIMER1CNT3_Match2		(HAL_REG_TIMER1_BASE+0x2C)

#define HAL_REG_TIMER1_Control			(HAL_REG_TIMER1_BASE+0x30)
#endif

#if defined(HAL_INC_TIMER2)
#define HAL_REG_TIMER2CNT1_Status		(HAL_REG_TIMER2_BASE+0x00)
#define HAL_REG_TIMER2CNT1_Reload		(HAL_REG_TIMER2_BASE+0x04)
#define HAL_REG_TIMER2CNT1_Match1		(HAL_REG_TIMER2_BASE+0x08)
#define HAL_REG_TIMER2CNT1_Match2		(HAL_REG_TIMER2_BASE+0x0C)

#define HAL_REG_TIMER2CNT2_Status		(HAL_REG_TIMER2_BASE+0x10)
#define HAL_REG_TIMER2CNT2_Reload		(HAL_REG_TIMER2_BASE+0x14)
#define HAL_REG_TIMER2CNT2_Match1		(HAL_REG_TIMER2_BASE+0x18)
#define HAL_REG_TIMER2CNT2_Match2		(HAL_REG_TIMER2_BASE+0x1C)

#define HAL_REG_TIMER2CNT3_Status		(HAL_REG_TIMER2_BASE+0x20)
#define HAL_REG_TIMER2CNT3_Reload		(HAL_REG_TIMER2_BASE+0x24)
#define HAL_REG_TIMER2CNT3_Match1		(HAL_REG_TIMER2_BASE+0x28)
#define HAL_REG_TIMER2CNT3_Match2		(HAL_REG_TIMER2_BASE+0x2C)

#define HAL_REG_TIMER2_Control			(HAL_REG_TIMER2_BASE+0x30)
#endif


/* AHBC */
#define HAL_REG_AHBC_PriorityCtrl		(HAL_REG_AHBC_BASE+0x80)
#define HAL_REG_AHBC_IntCtrl			(HAL_REG_AHBC_BASE+0x88)
#define HAL_REG_AHBC_Remap				(HAL_REG_AHBC_BASE+0x8C)

/* capture component */
#if defined(HAL_INC_CAPTURE)
#define HAL_REG_CAPTURE_Ctrl			(HAL_REG_CAPTURE_BASE+0x00)
//#define HAL_REG_CAPTURE_BWL				(HAL_REG_CAPTURE_BASE+0x08)
#define HAL_REG_CAPTURE_CropWidth		(HAL_REG_CAPTURE_BASE+0x10)
#define HAL_REG_CAPTURE_CropHeight		(HAL_REG_CAPTURE_BASE+0x14)
#define HAL_REG_CAPTURE_CptrSize		(HAL_REG_CAPTURE_BASE+0x18)
#define HAL_REG_CAPTURE_StartAddr		(HAL_REG_CAPTURE_BASE+0x1C)
#define HAL_REG_CAPTURE_StartAddr_Y		(HAL_REG_CAPTURE_BASE+0x20)
#define HAL_REG_CAPTURE_StartAddr_U		(HAL_REG_CAPTURE_BASE+0x24)
#define HAL_REG_CAPTURE_StartAddr_V		(HAL_REG_CAPTURE_BASE+0x28)
#endif

/* GPIO */
#if defined(HAL_INC_GPIO)
#define HAL_REG_GPIO_Data				(HAL_REG_GPIO_BASE+0x0)
#define HAL_REG_GPIO_Direction			(HAL_REG_GPIO_BASE+0x4)
#define HAL_REG_GPIO_IntSensCtrl		(HAL_REG_GPIO_BASE+0x8)
#define HAL_REG_GPIO_BothEdgeIntCtrl	(HAL_REG_GPIO_BASE+0xC)
#define HAL_REG_GPIO_IntEventCtrl		(HAL_REG_GPIO_BASE+0x10)
#define HAL_REG_GPIO_IntEnable			(HAL_REG_GPIO_BASE+0x14)
#define HAL_REG_GPIO_RawIntStatus		(HAL_REG_GPIO_BASE+0x1C)
#define HAL_REG_GPIO_MaskedIntStatus	(HAL_REG_GPIO_BASE+0x20)
#define HAL_REG_GPIO_IntClear			(HAL_REG_GPIO_BASE+0x24)
#define HAL_REG_GPIO_RWMask				(HAL_REG_GPIO_BASE+0x28)
#endif

/* UDSP2400 */
#if defined(HAL_INC_UDSP24)
#define HAL_REG_UDSP24_ID				(HAL_REG_UDSP24_BASE+0x0)
#define HAL_REG_UDSP24_Release			(HAL_REG_UDSP24_BASE+0x04)
#define HAL_REG_UDSP24_Reboot			(HAL_REG_UDSP24_BASE+0x08)
#define HAL_REG_UDSP24_MADDR			(HAL_REG_UDSP24_BASE+0x0C)
#define HAL_REG_UDSP24_MTYPE			(HAL_REG_UDSP24_BASE+0x0E)
#define HAL_REG_UDSP24_MDATA			(HAL_REG_UDSP24_BASE+0x10)
#endif

/* I2C1, for VFE (capture) */
#if defined(HAL_INC_I2C1)
#define HAL_REG_I2C1_Enable				(HAL_REG_I2C1_BASE+0x0)
#define HAL_REG_I2C1_Ctrl				(HAL_REG_I2C1_BASE+0x10)
#define HAL_REG_I2C1_Data				(HAL_REG_I2C1_BASE+0x20)
#define HAL_REG_I2C1_Status				(HAL_REG_I2C1_BASE+0x24)
#define HAL_REG_I2C1_PreScaleLowByte	(HAL_REG_I2C1_BASE+0x30)
#define HAL_REG_I2C1_PreScaleHighByte	(HAL_REG_I2C1_BASE+0x34)
#endif

/* I2C2, for I2S */
#if defined(HAL_INC_I2C2)
#define HAL_REG_I2C2_Enable				(HAL_REG_I2C2_BASE+0x0)
#define HAL_REG_I2C2_Ctrl				(HAL_REG_I2C2_BASE+0x10)
#define HAL_REG_I2C2_Data				(HAL_REG_I2C2_BASE+0x20)
#define HAL_REG_I2C2_Status				(HAL_REG_I2C2_BASE+0x24)
#define HAL_REG_I2C2_PreScaleLowByte	(HAL_REG_I2C2_BASE+0x30)
#define HAL_REG_I2C2_PreScaleHighByte	(HAL_REG_I2C2_BASE+0x34)
#endif

/* I2C3, for LED */
#if defined(HAL_INC_I2C3)
#define HAL_REG_I2C3_Enable				(HAL_REG_I2C3_BASE+0x0)
#define HAL_REG_I2C3_Ctrl				(HAL_REG_I2C3_BASE+0x10)
#define HAL_REG_I2C3_Data				(HAL_REG_I2C3_BASE+0x20)
#define HAL_REG_I2C3_Status				(HAL_REG_I2C3_BASE+0x24)
#define HAL_REG_I2C3_PreScaleLowByte	(HAL_REG_I2C3_BASE+0x30)
#define HAL_REG_I2C3_PreScaleHighByte	(HAL_REG_I2C3_BASE+0x34)
#endif

/* UART */
#if defined(HAL_INC_UART)
#define HAL_REG_UART_RBR				(HAL_REG_UART_BASE+0x00)
#define HAL_REG_UART_THR				(HAL_REG_UART_BASE+0x00)
#define HAL_REG_UART_DLL				(HAL_REG_UART_BASE+0x00)
#define HAL_REG_UART_IER				(HAL_REG_UART_BASE+0x04)
#define HAL_REG_UART_DLH				(HAL_REG_UART_BASE+0x04)
#define HAL_REG_UART_IIR				(HAL_REG_UART_BASE+0x08)
#define HAL_REG_UART_FCR				(HAL_REG_UART_BASE+0x08)
#define HAL_REG_UART_LCR				(HAL_REG_UART_BASE+0x0C)
#define HAL_REG_UART_MCR				(HAL_REG_UART_BASE+0x10)
#define HAL_REG_UART_LSR				(HAL_REG_UART_BASE+0x14)
#define HAL_REG_UART_MSR				(HAL_REG_UART_BASE+0x18)
#define HAL_REG_UART_SCR				(HAL_REG_UART_BASE+0x1C)
#endif


/* SSI */
#if defined(HAL_INC_SSI)
#define HAL_REG_SSI_CTRLR0				(HAL_REG_SSI_BASE+0x00)
#define HAL_REG_SSI_CTRLR1				(HAL_REG_SSI_BASE+0x04)
#define HAL_REG_SSI_SSIENR				(HAL_REG_SSI_BASE+0x08)
#define HAL_REG_SSI_MWCR				(HAL_REG_SSI_BASE+0x0C)
#define HAL_REG_SSI_SER					(HAL_REG_SSI_BASE+0x10)
#define HAL_REG_SSI_BAUDR				(HAL_REG_SSI_BASE+0x14)
#define HAL_REG_SSI_TXFTLR				(HAL_REG_SSI_BASE+0x18)
#define HAL_REG_SSI_RXFTLR				(HAL_REG_SSI_BASE+0x1C)
#define HAL_REG_SSI_TXFLR				(HAL_REG_SSI_BASE+0x20)
#define HAL_REG_SSI_RXFLR				(HAL_REG_SSI_BASE+0x24)
#define HAL_REG_SSI_SR					(HAL_REG_SSI_BASE+0x28)
#define HAL_REG_SSI_IMR					(HAL_REG_SSI_BASE+0x2C)
#define HAL_REG_SSI_ISR					(HAL_REG_SSI_BASE+0x30)
#define HAL_REG_SSI_RISR				(HAL_REG_SSI_BASE+0x34)
#define HAL_REG_SSI_TXOICR				(HAL_REG_SSI_BASE+0x38)
#define HAL_REG_SSI_RXOICR				(HAL_REG_SSI_BASE+0x3C)
#define HAL_REG_SSI_RXUICR				(HAL_REG_SSI_BASE+0x40)
#define HAL_REG_SSI_MSTICR				(HAL_REG_SSI_BASE+0x44)
#define HAL_REG_SSI_ICR					(HAL_REG_SSI_BASE+0x48)
#define HAL_REG_SSI_IDR					(HAL_REG_SSI_BASE+0x58)
#define HAL_REG_SSI_SSI_VERSION_ID		(HAL_REG_SSI_BASE+0x5C)
#define HAL_REG_SSI_DR					(HAL_REG_SSI_BASE+0x60)
#endif

/* DAI transmitter */
#if defined(HAL_INC_DAI_I2S)
#define HAL_REG_DAITX_TRANS_ENABLE		(HAL_REG_DAITX_BASE+0x0)
#define HAL_REG_DAITX_TRANS_MUTE		(HAL_REG_DAITX_BASE+0x4)
#define HAL_REG_DAITX_TRANS_CONF		(HAL_REG_DAITX_BASE+0x8)
#define HAL_REG_DAITX_CLK_DIV			(HAL_REG_DAITX_BASE+0xC)
#define HAL_REG_DAITX_DATA_REG_LEFT		(HAL_REG_DAITX_BASE+0x10)
#define HAL_REG_DAITX_DATA_REG_RIGHT	(HAL_REG_DAITX_BASE+0x14)
#define HAL_REG_DAITX_TRANS_STAT		(HAL_REG_DAITX_BASE+0x18)
#define HAL_REG_DAITX_FIFO_LTH			(HAL_REG_DAITX_BASE+0x1C)
#endif

/* DAI receiver */
#if defined(HAL_INC_DAI_I2S)
#define HAL_REG_DAIRX_SW_RESET			(HAL_REG_DAIRX_BASE+0x0)
#define HAL_REG_DAIRX_ENABLE			(HAL_REG_DAIRX_BASE+0x4)
#define HAL_REG_DAIRX_CTRL				(HAL_REG_DAIRX_BASE+0x8)
#define HAL_REG_DAIRX_WPOS				(HAL_REG_DAIRX_BASE+0xC)
#define HAL_REG_DAIRX_WLEN				(HAL_REG_DAIRX_BASE+0x10)
#define HAL_REG_DAIRX_DATA				(HAL_REG_DAIRX_BASE+0x14)
#define HAL_REG_DAIRX_FIFOFLAG			(HAL_REG_DAIRX_BASE+0x18)
#define HAL_REG_DAIRX_FIFO_GTH			(HAL_REG_DAIRX_BASE+0x1C)
#endif

/* DAI2 */
#if defined(HAL_INC_DAI2)
#define HAL_REG_DAI2_mode				(HAL_REG_DAI2_BASE+0x00)
#define HAL_REG_DAI2_rx_ctrl			(HAL_REG_DAI2_BASE+0x04)
#define HAL_REG_DAI2_tx_ctrl			(HAL_REG_DAI2_BASE+0x08)
#define HAL_REG_DAI2_wlen				(HAL_REG_DAI2_BASE+0x0C)
#define HAL_REG_DAI2_wpos				(HAL_REG_DAI2_BASE+0x10)
#define HAL_REG_DAI2_slot				(HAL_REG_DAI2_BASE+0x14)
#define HAL_REG_DAI2_tx_fifo_lth		(HAL_REG_DAI2_BASE+0x18)
#define HAL_REG_DAI2_rx_fifo_gth		(HAL_REG_DAI2_BASE+0x1C)
#define HAL_REG_DAI2_clock				(HAL_REG_DAI2_BASE+0x20)
#define HAL_REG_DAI2_init				(HAL_REG_DAI2_BASE+0x24)
#define HAL_REG_DAI2_tx_fifo_flag		(HAL_REG_DAI2_BASE+0x28)
#define HAL_REG_DAI2_tx_left_data		(HAL_REG_DAI2_BASE+0x2C)
#define HAL_REG_DAI2_tx_right_data		(HAL_REG_DAI2_BASE+0x30)
#define HAL_REG_DAI2_rx_fifo_flag		(HAL_REG_DAI2_BASE+0x34)
#define HAL_REG_DAI2_rx_data			(HAL_REG_DAI2_BASE+0x38)
#define HAL_REG_DAI2_tx_fifo_cntr		(HAL_REG_DAI2_BASE+0x3C)
#define HAL_REG_DAI2_rx_fifo_cntr		(HAL_REG_DAI2_BASE+0x40)
#define HAL_REG_DAI2_tx_int_enable		(HAL_REG_DAI2_BASE+0x44)
#define HAL_REG_DAI2_rx_int_enable		(HAL_REG_DAI2_BASE+0x48)
#define HAL_REG_DAI2_CODEC_clk_ctrl		(HAL_REG_DAI2_BASE+0x4C)
#define HAL_REG_DAI2_CODEC_clk_div 		(HAL_REG_DAI2_BASE+0x50)
#endif

/* KBMS0 */
#if defined(HAL_INC_KBMS0)
#define HAL_REG_KBMS0_ACK				(HAL_REG_KBMS0_BASE+0x00)
#define HAL_REG_KBMS0_STS				(HAL_REG_KBMS0_BASE+0x04)
#define HAL_REG_KBMS0_DATA				(HAL_REG_KBMS0_BASE+0x08)
#define HAL_REG_KBMS0_IR				(HAL_REG_KBMS0_BASE+0x10)
#define HAL_REG_KBMS0_INHB				(HAL_REG_KBMS0_BASE+0x14)
#endif

/* KBMS1 */
#if defined(HAL_INC_KBMS1)
#define HAL_REG_KBMS1_ACK				(HAL_REG_KBMS1_BASE+0x00)
#define HAL_REG_KBMS1_STS				(HAL_REG_KBMS1_BASE+0x04)
#define HAL_REG_KBMS1_DATA				(HAL_REG_KBMS1_BASE+0x08)
#define HAL_REG_KBMS1_IR				(HAL_REG_KBMS1_BASE+0x10)
#define HAL_REG_KBMS1_INHB				(HAL_REG_KBMS1_BASE+0x14)
#endif

/* LCM */
#if defined(HAL_INC_LCM)
#define HAL_REG_LCM_CTRL_START			(HAL_REG_LCM_BASE+0x00)
#define HAL_REG_LCM_DATA_START			(HAL_REG_LCM_BASE+0x04)
#define HAL_REG_LCM_CTRL1				(HAL_REG_LCM_BASE+0x08)
#define HAL_REG_LCM_CTRL2				(HAL_REG_LCM_BASE+0x0C)
#define HAL_REG_DATA1					(HAL_REG_LCM_BASE+0x20)
#define HAL_REG_DATA2					(HAL_REG_LCM_BASE+0x24)
#define HAL_REG_DATA3					(HAL_REG_LCM_BASE+0x28)
#define HAL_REG_DATA4					(HAL_REG_LCM_BASE+0x2C)
#define HAL_REG_DATA5					(HAL_REG_LCM_BASE+0x30)
#define HAL_REG_DATA6					(HAL_REG_LCM_BASE+0x34)
#define HAL_REG_DATA7					(HAL_REG_LCM_BASE+0x38)
#define HAL_REG_DATA8					(HAL_REG_LCM_BASE+0x3C)
#endif

/* MPEG4 Decoder */
#if defined(HAL_INC_MPEG4_DEC)
#define HAL_REG_M4D_RESET				(HAL_REG_M4D_BASE+0x00)
#define HAL_REG_M4D_START				(HAL_REG_M4D_BASE+0x04)
#define HAL_REG_M4D_FUNC				(HAL_REG_M4D_BASE+0x0C)
#define HAL_REG_M4D_OUT_Y_ADDR			(HAL_REG_M4D_BASE+0x14)
#define HAL_REG_M4D_BITSTREAM_INI_ADD	(HAL_REG_M4D_BASE+0x18) 
#define HAL_REG_M4D_BITSTREAM_END_ADD	(HAL_REG_M4D_BASE+0x1C) 
#define HAL_REG_M4D_STOP             	(HAL_REG_M4D_BASE+0x20) 
#define HAL_REG_M4D_INT              	(HAL_REG_M4D_BASE+0x24) 
#define HAL_REG_M4D_RESO             	(HAL_REG_M4D_BASE+0x28) 
#define HAL_REG_M4D_TIME_INC_RESO    	(HAL_REG_M4D_BASE+0x2C) 
#define HAL_REG_M4D_TIME_INF         	(HAL_REG_M4D_BASE+0x30) 
#define HAL_REG_M4D_TIME_CODE        	(HAL_REG_M4D_BASE+0x34) 
#define HAL_REG_M4D_MC_Y0_ADDR     		(HAL_REG_M4D_BASE+0x44) 
#define HAL_REG_M4D_MC_Y1_ADDR      	(HAL_REG_M4D_BASE+0x48) 
#define HAL_REG_M4D_MC_CB_ADDR     		(HAL_REG_M4D_BASE+0x4C) 
#define HAL_REG_M4D_MC_CR_ADDR     		(HAL_REG_M4D_BASE+0x50) 
#define HAL_REG_M4D_OUT_CB_ADDR     	(HAL_REG_M4D_BASE+0x54) 
#define HAL_REG_M4D_OUT_CR_ADDR     	(HAL_REG_M4D_BASE+0x58) 
#endif


#define HAL_GETMEM32(m)		(*((volatile unsigned int   *) (m)))
#define HAL_GETMEM16(m)		(*((volatile unsigned short *) (m)))
#define HAL_GETMEM8(m)		(*((volatile unsigned char  *) (m)))

#define HAL_SETMEM32(m,v)	(*((volatile unsigned int   *) (m)) = ((unsigned int) (v)))
#define HAL_SETMEM16(m,v)	(*((volatile unsigned short *) (m)) = ((unsigned short) (v)))
#define HAL_SETMEM8(m,v)	(*((volatile unsigned char  *) (m)) = ((unsigned char) (v)))


#define HAL_GETREG32(r)		(*((volatile unsigned int   *) (r)))
#define HAL_GETREG(r)		(*((volatile unsigned int   *) (r)))
#define HAL_GETREG16(r)		(*((volatile unsigned short *) (r)))
#define HAL_GETREG8(r)		(*((volatile unsigned char  *) (r)))

#define HAL_SETREG32(r,v)	(*((volatile unsigned int   *) (r)) = ((unsigned int)   (v)))
#define HAL_SETREG(r,v)		(*((volatile unsigned int   *) (r)) = ((unsigned int)   (v)))
#define HAL_SETREG16(r,v)	(*((volatile unsigned short *) (r)) = ((unsigned short) (v)))
#define HAL_SETREG8(r,v)	(*((volatile unsigned char  *) (r)) = ((unsigned char)  (v)))


enum halWDTClkSrc_E
{
	halcWDTClk_pclk		= 0,
	halcWDTClk_extclk	= 1
};

enum halI2CId_E
{
	halcI2C1	= 1,
	halcI2C2	= 2,
	halcI2C3	= 3
};

enum halGPIOPin_E
{
	halcGPIOPin0	= 0,
	halcGPIOPin1	= 1,
	halcGPIOPin2	= 2,
	halcGPIOPin3	= 3,
	halcGPIOPin4	= 4,
	halcGPIOPin5	= 5,
	halcGPIOPin6	= 6,
	halcGPIOPin7	= 7,
	halcGPIOPin8	= 8,
	halcGPIOPin9	= 9,
	halcGPIOPin10	= 10,
	halcGPIOPin11	= 11,
	halcGPIOPin12	= 12,
	halcGPIOPin13	= 13,
	halcGPIOPin14	= 14,
	halcGPIOPin15	= 15,
	halcGPIOPin16	= 16,
	halcGPIOPin17	= 17,
	halcGPIOPin18	= 18,
	halcGPIOPin19	= 19,
	halcGPIOPin20	= 20,
	halcGPIOPin21	= 21,
	halcGPIOPin22	= 22,
	halcGPIOPin23	= 23,
	halcGPIOPin24	= 24,
	halcGPIOPin25	= 25,
	halcGPIOPin26	= 26,
	halcGPIOPin27	= 27,
	halcGPIOPin28	= 28,
	halcGPIOPin29	= 29,
	halcGPIOPin30	= 30,
	halcGPIOPin31	= 31,

	halcGPIOPinAll	= 88
};

enum halGPIOPushBtn_E
{
	halcPushBtn0	= 0,
	halcPushBtn1	= 1,
	halcPushBtn2	= 2,
	halcPushBtn3	= 3,
	halcPushBtn4	= 4,
	halcPushBtn5	= 5,
	halcPushBtn6	= 6,
	halcPushBtn7	= 7,

	halcPushBtnAll	= 99
};

enum halGPIOIntrTrigMode_E
{
	halcGPIOIntr_edge_positive = 1,
	halcGPIOIntr_edge_negative = 2,
	halcGPIOIntr_edge_both = 3,
	halcGPIOIntr_level_activeHigh = 4,
	halcGPIOIntr_level_activeLow = 5,
};

enum halAHBMaster_E
{
	halcAHBMaster_CPU		= 1,
	halcAHBMaster_DMA_A		= 2,
	halcAHBMaster_JPEGIn	= 17,
	halcAHBMaster_JPEGOut	= 18,
	halcAHBMaster_DMA_B		= 19
};

enum halTVInputFormat_E
{
	halcTVInputFormat_CbYCrY = 0,	// normal //
	halcTVInputFormat_CrYCbY = 1,	// swap Cb/Cr //
	halcTVInputFormat_YCbYCr = 2,	// swap Y/Cb,Cr //
	halcTVInputFormat_YCrYCb = 3	// swap Y/Cb,Cr; swap Cb/Cr //

	/*
	NOTE: 

		YCbYCr422 names that the order of Y1, Cb, Y2, and Cr
		from "lsb" to "msb".

        M                       L
        S                       S
        B                       B
        +-----------------------+
        |  Cr |  Y2 |  Cb |  Y1 |
        +-----------------------+

		LSB is the low address byte and MSB is the high address byte.
	*/
};

enum halTVLCDEncoder_E
{
	halcTVLCD_TV = 1,
	halcTVLCD_LCD= 2
};

enum halLCDInputFormat_E
{
	halcLCDInputFormat_YCbYCr422	= 0,
	halcLCDInputFormat_YCrYCb422,
	halcLCDInputFormat_CbYCrY422,
	halcLCDInputFormat_CrYCbY422,

	halcLCDInputFormat_RGB1bpp,		/* need to provide a color pallet */
	halcLCDInputFormat_RGB2bpp,
	halcLCDInputFormat_RGB4bpp,
	halcLCDInputFormat_RGB8bpp,

	halcLCDInputFormat_RGB16bpp,	/* (R,G,B) = (5b,6b,5b) */
	halcLCDInputFormat_BGR16bpp,	/* (B,G,R) = (5b,6b,5b) */

	halcLCDInputFormat_RGB24bpp,	/* (R,G,B) = (8b,8b,8b) */
	halcLCDInputFormat_BGR24bpp		/* (B,G,R) = (8b,8b,8b) */

	/*
		NOTE:

		YCbYCr422 names that the order of Y1, Cb, Y2, and Cr
		from "lsb" to "msb".

        M                       L
        S                       S
        B                       B
        +-----------------------+
        |  Cr |  Y2 |  Cb |  Y1 |   YCbYCr format
        +-----------------------+

        +-----------------------+
        |  Cb |  Y2 |  Cr |  Y1 |   YCrYCb format
        +-----------------------+



		RGB names that the order of R, G, and B are from
		"msb" to "lsb".
		
        M                       L
        S                       S
        B                       B
        +-----------------------+
        |  X  |  R  |  G  |  B  |   RGB
        +-----------------------+

        +-----------------------+
        |  X  |  B  |  G  |  R  |   BGR
        +-----------------------+
	*/
};

enum halTVSys_E
{
	halcTVSys_NTSC = 1,
	halcTVSys_PAL_M	= 2,
	halcTVSys_PAL_BDGHI	= 3,
	halcTVSys_PAL_Nc	= 4
};

enum halDisplay_E
{
	halcTV	= 1,
	halcLCD	= 2,
	halcVGA	= 3
};

enum halHSDelay_E
{
	halcHSNoDelay		= 0,
	halcHSDelay1Cycle	= 1,
	halcHSDelay2Cycle	= 2,
	halcHSDelay3Cycle	= 3
};

enum halCptrCntl_E
{
	halcCptrCntl_captureEnable	= 1,
	halcCptrCntl_captureDisable,

	halcCptrCntl_skipFrame,
	halcCptrCntl_noSkipFrame,

	halcCptrCntl_VSyncActiveHigh,
	halcCptrCntl_VSyncActiveLow,

	halcCptrCntl_HSyncActiveHigh,
	halcCptrCntl_HSyncActiveLow,

	halcCptrCntl_InterleaveMode,
	halcCptrCntl_BlockMode,

	halcCptrCntl_422Format,
	halcCptrCntl_420Format,

	halcCptrCntl_YCbYCr,
	halcCptrCntl_CbYCrY,
	halcCptrCntl_YCrYCb,
	halcCptrCntl_CrYCbY,

	halcCptrCntl_littleEndian,
	halcCptrCntl_bigEndian,

	halcCptrCntl_captureOnce,
	halcCptrCntl_captureAlways,

	halcCptrCntl_conventional,
	halcCptrCntl_advanced,

	halcCptrCntl_HSNoDelay,
	halcCptrCntl_HSDelay1Cycle,
	halcCptrCntl_HSDelay2Cycle,
	halcCptrCntl_HSDelay3Cycle
};

enum halJIFormat_E			/* input format of the jpeg codec */
{
	halcJIUnknown	= 0,
	halcJIYCbCr422	= 1,
	halcJIYCbCr420	= 2,
	halcJIYCbCr444	= 3,
	halcJIGrayLevel	= 4
};

enum halRawImgFmt_E
{
	halcRawImgFmt_Unknown	= 0,
	halcRawImgFmt_YCbCr444	= 1,
	halcRawImgFmt_YCbCr422	= 2,	/* 422H */
	halcRawImgFmt_YCbCr420	= 3,
	halcRawImgFmt_GrayLevel	= 4,
	halcRawImgFmt_YCbCr422V	= 5		/* 422V, added on 08/10/2006 */
};

enum halVICIntrTrigMode_E
{
	halc_edge_activeFalling	= 0,
	halc_edge_activeRaising	= 1,
	halc_edge_activeBoth	= 2,
	halc_level_activeHigh	= 3,
	halc_level_activeLow	= 4
};

enum halTimerId_E
{
	halcTimer1Cnt1	= 1,
	halcTimer1Cnt2	= 2,
	halcTimer1Cnt3	= 3,
	halcTimer2Cnt1	= 4,
	halcTimer2Cnt2	= 5,
	halcTimer2Cnt3	= 6
};

enum halUDSP24Mem_E
{
	halcUDSP24_PM		= 0,
	halcUDSP24_DM		= 1,
	halcUDSP24_CM		= 2
};

enum halDMAPId_E
{
	halcDMAPIdC0	=	0,	/* channel 0 */
	halcDMAPIdC1	=	1,	/* channel 1 */
	halcDMAPIdC2	=	2,
	halcDMAPIdC3	=	3,
	halcDMAPIdC4	=	4,
	halcDMAPIdC5	=	5,
	halcDMAPIdC6	=	6,
	halcDMAPIdC7	=	7	/* channel 7 */
};

enum halCFIRQId_E
{
	halcCFIRQ0	= 0,
	halcCFIRQ1	= 1
};

enum halCFFlag_E
{
	halcCFFlag_NoCF			= 0,
	halcCFFlag_CardInsert	= 1,
	halcCFFlag_PwrRstComplete= 2,
//	halcCFFlag_DataOutCmd	= 3,
	halcCFFlag_Ready		= 4,
	halcCFFlag_StatusRdy	= 5,
	halcCFFlag_StatusDrq	= 6,
	halcCFFlag_StatusErr	= 7,
	halcCFFlag_FIFODrain	= 8,
	halcCFFlag_FIFOFill		= 9,
	halcCFFlag_FIFOEmpty	= 10,
	halcCFFlag_FIFOFull		= 11,
	halcCFFlag_FIFOUnderrun	= 12,
	halcCFFlag_FIFOOverrun	= 13,
	halcCFFlag_SecCntWrAct	= 14,
	halcCFFlag_SecCntRdAct	= 15,
	halcCFFlag_StatusAct	= 16,
	halcCFFlag_CmdAct		= 17,
	halcCFFlag_ErrorAct		= 18,
	halcCFFlag_FeatureAct	= 19,
	halcCFFlag_DataWrAct	= 20,
	halcCFFlag_DataRdAct	= 21,
	halcCFFlag_EccWrAct		= 22,
	halcCFFlag_EccRdAct		= 23,
	halcCFFlag_DrvHeadWrAct	= 24,
	halcCFFlag_DrvHeadRdAct	= 25,
	halcCFFlag_CylHighWrAct	= 26,
	halcCFFlag_CylHighRdAct	= 27,
	halcCFFlag_CylLowWrAct	= 28,
	halcCFFlag_CylLowRdAct	= 29,
	halcCFFlag_SecNumWrAct	= 30,
	halcCFFlag_SecNumRdAct	= 31,

	halcCFFlag_All			= 99
};

#if 0
enum halCFFlag_E
{
	halcCFFlag_NoCF			= 0,
	halcCFFlag_CardInsert	= 1,
	halcCFFlag_DataInCmd	= 2,
	halcCFFlag_DataOutCmd	= 3,
	halcCFFlag_NonDataCmd	= 4,
	halcCFFlag_Ready		= 5,
	halcCFFlag_StatusDrq	= 6,
	halcCFFlag_StatusRdy	= 7,
	halcCFFlag_StatusErr	= 8,
	halcCFFlag_FIFOOverrun	= 9,
	halcCFFlag_FIFOUnderrun	= 10,
	halcCFFlag_FIFOEmpty	= 11,
	halcCFFlag_FIFOFull		= 12,
	halcCFFlag_FIFOEccRdy	= 13,
	halcCFFlag_SecCntWrAct	= 14,
	halcCFFlag_SecCntRdAct	= 15,
	halcCFFlag_StatusAct	= 16,
	halcCFFlag_CmdAct		= 17,
	halcCFFlag_ErrorAct		= 18,
	halcCFFlag_FeatureAct	= 19,
	halcCFFlag_DataWrAct	= 20,
	halcCFFlag_DataRdAct	= 21,
	halcCFFlag_EccWrAct		= 22,
	halcCFFlag_EccRdAct		= 23,
	halcCFFlag_DrvHeadWrAct	= 24,
	halcCFFlag_DrvHeadRdAct	= 25,
	halcCFFlag_CylHighWrAct	= 26,
	halcCFFlag_CylHighRdAct	= 27,
	halcCFFlag_CylLowWrAct	= 28,
	halcCFFlag_CylLowRdAct	= 29,
	halcCFFlag_SecNumWrAct	= 30,
	halcCFFlag_SecNumRdAct	= 31,

	halcCFFlag_All			= 99
};
#endif

enum halISRMode_E
{
	halcIRQMode	= 1,
	halcFIQMode	= 2
};

enum halUSBEvent_E
{
	USB_NO_EVT          = 0,
	USB_CTRL_EVT        = 1,
	USB_BUKIN_EVT       = 2,
	USB_BUKOUT_EVT      = 3,
	USB_OTH_EVT         = 4,
	USB_CONN_EVT		= 5,
	USB_DISCONN_EVT		= 6,
	USB_RESET_EVT       = 7
};

enum halKBMSId_E
{
	halcKBMS0_Id		= 0,
	halcKBMS1_Id		= 1
};

enum halDAIId_E
{
	halcDAITX	= 1,
	halcDAIRX	= 2,
	halcDAITXRX	= 3
};

enum halDAIDataProtocol_E
{
	halcDAII2SMode			= 4,
	halcDAILJustMode		= 0,
	halcDAIRJustMode		= 1,
	halcDAIMSBExtMode		= 2,
	halcDAIDSPMode			= 6
};

typedef enum halWDTClkSrc_E			halWDTClkSrc_E;
typedef enum halI2CId_E				halI2CId_E;
typedef enum halGPIOPushBtn_E		halGPIOPushBtn_E;
typedef enum halGPIOPin_E			halGPIOPin_E;
typedef enum halGPIOIntrTrigMode_E	halGPIOIntrTrigMode_E;
typedef enum halAHBMaster_E			halAHBMaster_E;
typedef enum halIntrVector_E		halIntrVector_E;
typedef enum halLCDInputFormat_E	halLCDInputFormat_E;
typedef enum halTVInputFormat_E		halTVInputFormat_E;
typedef enum halTVLCDEncoder_E		halTVLCDEncoder_E;
typedef enum halTVSys_E				halTVSys_E;
typedef enum halDisplay_E			halDisplay_E;
typedef enum halHSDelay_E			halHSDelay_E;
typedef enum halCptrCntl_E			halCptrCntl_E;
typedef enum halJIFormat_E			halJIFormat_E;
typedef enum halRawImgFmt_E			halRawImgFmt_E;
typedef enum halVICIntrTrigMode_E	halVICIntrTrigMode_E;
typedef enum halTimerId_E			halTimerId_E;
typedef enum halUDSP24Mem_E			halUDSP24Mem_E;
typedef enum halDMAPId_E			halDMAPId_E;
typedef enum halCFIRQId_E			halCFIRQId_E;
typedef enum halCFFlag_E			halCFFlag_E;
typedef enum halISRMode_E			halISRMode_E;
typedef enum halUSBEvent_E			halUSBEvent_E;
typedef enum halKBMSId_E			halKBMSId_E;
typedef enum halDAIId_E				halDAIId_E;
typedef enum halDAIDataProtocol_E	halDAIDataProtocol_E;



typedef void (*halISR_FT)(void);
	/*
	The prototype of "interrupt service routines".
	*/




/* hal.c */
void halInitInMain(unsigned int nHCLK);
void halDelay(int nTicks);
int halGetTicks(void);
bool_T halRegisterISR(halISRMode_E isrMode,
				halIntrVector_E interrupt,
				halISR_FT serviceRoutine);

bool_T halUnregisterISR(halISRMode_E isrMode, halIntrVector_E interrupt);

void halIRQISR(void);
void halFIQISR(void);

#define halEnableInterrupt(interrupt)		halVICEnableIntr(interrupt)
#define halDisableInterrupt(interrupt)		halVICDisableIntr(interrupt)


/* halSys.c */
void halSysSelDisplay(halDisplay_E nDisplay);
unsigned int halSysGetHWVersion(void);
unsigned int halSysGetDIPSwitchValue(void);
unsigned int halSysGetIPStatus(void);


/* halAHBC.c */
void halAHBCRemapToSDRAM(void);
void halAHBCRemapToSRAM(void);
void halAHBCRemapToASRAM(void);
void halAHBCSetMasterHighPri(halAHBMaster_E);
void halAHBCSetMasterLowPri(halAHBMaster_E);


/* halGPIO.c */
#if defined(HAL_INC_GPIO)
void halGPIOUnmaskPushBtnIntr(halGPIOPushBtn_E pushBtn);
void halGPIOMaskPushBtnIntr(halGPIOPushBtn_E pushBtn);
void halGPIOSetPushBtnIntrMode(halGPIOPushBtn_E pushBtn,
				halGPIOIntrTrigMode_E nIntrTrigMode);
void halGPIOUnmaskIntr(halGPIOPin_E nPin);
void halGPIOMaskIntr(halGPIOPin_E nPin);
void halGPIOSetIntrMode(halGPIOPin_E nPin, halGPIOIntrTrigMode_E nIntrTrigMode);
#define halGPIOSetData(nData) (HAL_SETREG32(HAL_REG_GPIO_Data, nData))
void halGPIOSetPinData(halGPIOPin_E nPin, bool_T b);
void halGPIOTogglePinData(halGPIOPin_E nPin);
void halGPIOClearPinIntr(halGPIOPin_E nPin);
#else
#define halGPIOUnmaskPushBtnIntr(pushBtn)
#define halGPIOMaskPushBtnIntr(pushBtn)
#define halGPIOSetPushBtnIntrMode(pushBtn, nIntrTrigMode)
#define halGPIOUnmaskIntr(nPin)
#define halGPIOMaskIntr(nPin)
#define halGPIOSetIntrMode(nPin, nIntrTrigMode)
#define halGPIOSetData(nData) 
#define halGPIOSetPinData(nPin, b)
#define halGPIOTogglePinData(nPin)
#define halGPIOClearPinIntr(nPin)
#endif


/* halCapture.c */
#if defined(HAL_INC_CAPTURE)
bool_T halCaptureSetControl(halCptrCntl_E);
//void halCaptureHSPolarityActiveHigh(void);
//void halCaptureHSPolarityActiveLow(void);
//void halCaptureVSPolarityActiveHigh(void);
//void halCaptureVSPolarityActiveLow(void);
//void halCaptureEnable(void);
//void halCaptureDisable(void);
//void halCaptureSetHSDelay(halHSDelay_E nDelay);
bool_T halCaptureSetCropping(unsigned short nStartWidthPix, unsigned short nWidth,
							 unsigned short nStartHeightPix,unsigned short nHeight,
							 unsigned short nWDScale, unsigned short nHDScale);
void halCaptureSetCaptureStartAddr(unsigned int n);
void halCaptureSetBlockCaptureStartAddr(unsigned int Y, unsigned int Cb, unsigned int Cr);
#else
#define halCaptureHSPolarityActiveHigh()
#define halCaptureHSPolarityActiveLow()
#define halCaptureVSPolarityActiveHigh()
#define halCaptureVSPolarityActiveLow()
#define halCaptureEnable()
#define halCaptureDisable()
#define halCaptureSetHSDelay(nDelay)
#define halCaptureSetCropping(a, b, c, d, e, f) (FALSE)
#define halCaptureSetCaptureStartAddr(n)
#endif


/* halI2C.c */
#if defined(HAL_INC_I2C1) || defined(HAL_INC_I2C2) || defined(HAL_INC_I2C3)
void halI2CDisable(halI2CId_E nId);
void halI2CEnable(halI2CId_E nId, unsigned int nPCLK);
#else
#define halI2CDisable(nId)
#define halI2CEnable(nId, nPCLK)
#endif

#if defined(HAL_INC_I2C1)
void halI2C1WriteSAA7111A(unsigned int subAddr, unsigned int data);
void halI2C1ReadSAA7111A(unsigned int subAddr, unsigned int *data);
void halI2C1EnableVideoInViaSAA7111A(unsigned char *pRegValSets, int nNumSet);
void halI2C1DisableVideoinViaSAA7111A(void);

void halI2C1WriteTW9906(unsigned int subAddr, unsigned int data);
void halI2C1ReadTW9906(unsigned int subAddr, unsigned int *data);
void halI2C1EnableVideoInViaTW9906(unsigned char *pRegValSets, int nNumSet);
void halI2C1DisableVideoinViaTW9906(void);

void halI2C1WriteAL240(unsigned int subAddr, unsigned int data);
void halI2C1ReadAL240(unsigned int subAddr, unsigned int *data);
void halI2C1EnableVideoInViaAL240(unsigned char *pRegValSets, int nNumSet);
void halI2C1DisableVideoinViaAL240(void);
#else
#define halI2C1WriteSAA7111A(subAddr, data)
#define halI2C1ReadSAA7111A(subAddr, data)
#define halI2C1EnableVideoInViaSAA7111A(a, b)
#define halI2C1DisableVideoinViaSAA7111A()

#define halI2C1WriteTW9906(subAddr, data)
#define halI2C1ReadTW9906(subAddr, data)
#define halI2C1EnableVideoInViaTW9906(a, b)
#define halI2C1DisableVideoinViaTW9906()

#define halI2C1WriteAL240(subAddr, data)
#define halI2C1ReadAL240(subAddr, data)
#define halI2C1EnableVideoInViaAL240(a, b)
#define halI2C1DisableVideoinViaAL240()
#endif

#if defined(HAL_INC_I2C2)
typedef struct halUDA1380Setting_S
{
	unsigned int nPCLK;			//AMBA clock (APB clock); used by the DAI IP
	unsigned int nSYSCLK;		//input frequency of the UDA1380's SYSCLK pin
	unsigned int nSamplingRate; //the audio/speech sampling rate; I2S protocol needs this

	bool_T bEnableMicVGA;
	unsigned char nMicVGAGain;	//Microphone input VGA gain (4-bit long)

	unsigned short nDecVolCtrl;	//Decimator volume control (16-bit long, left and right)

	bool_T bEnableAnaIn;		//enable "analog input"
	bool_T bAnaInFromMic;		//the "analog input" is from microphone or from line-in
} halUDA1380Setting_S;

typedef struct halTLV320Setting_S
{
	unsigned int nPCLK;
	unsigned int nMCLK;			//input frequency of the TLV320's MCLK pin
	unsigned int nSamplingRate;

	bool_T bEnableDAC;

	bool_T bEnableADC;

	bool_T bAnaInFromMic;	//TRUE:ADC is from microphone; FALSE:ADC is from linein

	bool_T bSitoneAdded;	//add microphonein to headphone

	bool_T bBypassLineInToHeadphone; //bypass linein to headphone
} halTLV320Setting_S;

bool_T halI2C2WriteUDA1380(unsigned int regAddr,
		unsigned int nNumItems, unsigned int *pData);
bool_T halI2C2ReadUDA1380(unsigned int regAddr,
		unsigned int nNumItems, unsigned int *pData);
bool_T halI2C2InitUDA1380I2SMode(halUDA1380Setting_S *pSetting);
void halI2C2PowerOffUDA1380(void);

void halI2C2ResetTLV320(void);
bool_T halI2C2InitTLV320I2SMode(halTLV320Setting_S *pSetting);
void halI2C2PowerOffTLV320(void);
#else
#define halI2C2WriteUDA1380(regAddr, nNumItems, pData) (FALSE)
#define halI2C2ReadUDA1380(regAddr, nNumItems, pData) (FALSE)
#define halI2C2InitUDA1380I2SMode(pSetting)
#define halI2C2PowerOffUDA1380()
#endif

#if defined(HAL_INC_I2C3)
void halI2C3WriteLED(unsigned short nValue);
#else
#define halI2C3WriteLED(nValue);
#endif


/* halVIC.c */
void halVICReset(void);
void halVICSetIRQMode(halIntrVector_E);
void halVICSetFIQMode(halIntrVector_E);
void halVICSetIntrTrigMode(halIntrVector_E, halVICIntrTrigMode_E);
void halVICEnableIntr(halIntrVector_E);
void halVICDisableIntr(halIntrVector_E);
void halVICClearIntr(halIntrVector_E);
void halVICGenSWIntr(halIntrVector_E);
void halVICClearSWIntr(halIntrVector_E);
#define halVIC1ProtRegs(b) 								\
	(b ?												\
		(HAL_SETREG(HAL_REG_VIC1_ProtectionEnable, 1))	\
			:											\
		(HAL_SETREG(HAL_REG_VIC1_ProtectionEnable, 0))	\
	)


/* halTimer.c */
#if defined(HAL_INC_TIMER1)
bool_T halTimerEnable(halTimerId_E timerId,
					unsigned int nMatch1,
					unsigned int nMatch2,
					unsigned int nNumPClks);
void halTimerDisable(halTimerId_E timerId);
unsigned int halTimerGetStatus(halTimerId_E timerId);
#else
#define halTimerEnable(timerId, n1, n2, nClk) FALSE
#define halTimerDisable(timerId)
#define halTimerGetStatus(timerId) 0
#endif


/* halLCD.c */
#if defined(HAL_INC_LCD)
#define halLCDEnable(void)				(HAL_SETREG(HAL_REG_LCD_DOutR, 1))
#define halLCDDisable(void)				(HAL_SETREG(HAL_REG_LCD_DOutR, 0))
#define halLCDSetFrameDataStartAddr(x)	(HAL_SETREG(HAL_REG_LCD_BaseR, x))
void halLCDSetInputFormat(bool_T bSysLittleEndian, halLCDInputFormat_E nFmt);
void halLCDSetPaletteTblEntry(unsigned char nR, unsigned char nG, unsigned char nB,
							unsigned char nIdx);
void halLCDSetBoder(unsigned int nBW, unsigned nBH, unsigned char nR,
					unsigned char nG, unsigned char nB);
#else
#define halLCDEnable()
#define halLCDDisable()
#define halLCDSetFrameDataStartAddr(x)
#define halLCDSetInputFormat(bSysLittleEndian, nFmt)
#define halLCDSetPaletteTblEntry(nR, nG, nB, nIdx)
#define halLCDSetBoder(nBW, nBH, nR, nG, nB)
#endif


/* halTV.c */
#if defined(HAL_INC_TV)
void halTVReset(void);
void halTVEnable(halTVLCDEncoder_E);
void halTVDisable(void);
void halTVSetMasterMode(void);
void halTVSetSlaveMode(void);
void halTVSelSystem(halTVSys_E tvSystem);
void halTVSetEvenOddFrameWidth(unsigned int nW);
void halTVSetEvenOddFrameHeight(unsigned int nH);
void halTVSetEvenFrameStartAddr(unsigned int nAddress);
void halTVSetOddFrameStartAddr(unsigned int nAddress);
void halTVSetHorizontalOffset(unsigned char nHOffset);
void halTVSetVerticalOffset(unsigned char nVOffset);
void halTVSetDefaultYCbCrCoef(halTVSys_E tvSystem);
void halTVSetNewYCbCrCoef(halTVSys_E tvSystem,
						unsigned char nYCoef,
						unsigned char nCbCoef,
						unsigned char nCrCoef);
void halTVGetYCbCrCoef(unsigned char *pnYCoef,
						unsigned char *pnCbCoef,
						unsigned char *pnCrCoef);
bool_T halTVSetInputFormat(halTVInputFormat_E);
bool_T halTVShowImageOnTV(halTVInputFormat_E,
				unsigned int nHOffset,
				unsigned int nVOffset,
				unsigned int nW,
				unsigned int nH,
				unsigned int nF0Start,
				unsigned int nF1Start);
void halTVShowImageOnVGA(halTVInputFormat_E,
		unsigned int nRawImgWidth,
		unsigned int nRawImgHeight,
		unsigned int nRawImgStartAddr);
void halTVShowImageOnLCDPanel(halTVInputFormat_E,
		unsigned int nRawImgWidth,
		unsigned int nRawImgHeight,
		unsigned int nRawImgStartAddr);
void halTVSetBackGroundColor(unsigned char Y, unsigned char Cb, unsigned char Cr);
#else
#define halTVReset()
#define halTVEnable()
#define halTVDisable()
#define halTVSetMasterMode()
#define halTVSetSlaveMode()
#define halTVSelSystem(tvSystem)
#define halTVSetEvenOddFrameWidth(nW)
#define halTVSetEvenOddFrameHeight(nH)
#define halTVSetEvenFrameStartAddr(nAddress)
#define halTVSetOddFrameStartAddr(nAddress)
#define halTVSetHorizontalOffset(nHOffset)
#define halTVSetVerticalOffset(nVOffset)
#define halTVSetDefaultYCbCrCoef(tvSystem)
#define halTVSetNewYCbCrCoef(tvSystem, nYCoef, nCbCoef, nCrCoef)
#define halTVGetYCbCrCoef(pnYCoef,pnCbCoef,pnCrCoef)
#define halTVSetInputFormat(format) (FALSE)
#define halTVShowImage(format, nHOffset, nVOffset, nW, nH, nF0Start, nF1Start) (FALSE)
#define halTVSetBackGroundColor(Y, Cb, Cr)
#endif


/* halJPEGD.c */
halRawImgFmt_E halJPEGDDecodeP1(
		unsigned int nBSAddr,
		unsigned int *pnWidth,
		unsigned int *pnHeight);

bool_T halJPEGDDecodeP2(unsigned int nYAddr,
					   unsigned int nCbAddr,
					   unsigned int nCrAddr);

/* halJPEG2.c */
#if defined(HAL_INC_JPEG2)
halRawImgFmt_E halJPEGDecode(unsigned int nBSAddr,		/* I, bitstream address */
					unsigned int nOutputAddr,	/* I, output address */
					unsigned int *pnWidth,
					unsigned int *pnHeight,
					unsigned int *pnYAddr,	/* O, address of the Y component */
					unsigned int *pnCbAddr,	/* O, address of the Cb component */
					unsigned int *pnCrAddr);	/* O, address of the Cr component */

bool_T halJPEGEncode(halRawImgFmt_E nFmt,	/* raw image format */
					unsigned int nWidth,
					unsigned int nHeight,
					unsigned int nYAddr,	/* address of the Y component */
					unsigned int nCbAddr,	/* address of the Cb component */
					unsigned int nCrAddr,	/* address of the Cr component */
					unsigned int nOutputAddr,	/* address of the encoded output (bitstream) */ 
					unsigned int *pnBSSize);	/* O, bitstream size */
#else
//#define halJPEGDecode(nBSAddr,nOutputAddr,pnWidth,pnHeight,pnYAddr,pnCbAddr,pnCrAddr) (halcRawImgFmt_Unknown)
//#define halJPEGEncode(nFmt,nWidth,nHeight,nYAddr,nCbAddr,nCrAddr,nOutputAddr,pnBSSize) (FALSE)
#endif

/* halJPEGC.c */
#if defined(HAL_INC_JPEGC)
void halJPEGCReset(void);

bool_T halJPEGEncode(int nIStartAddr, int nISize,
		int nIWidth, int nIHeight,
		int nOStartAddr, int *pnOSize,
		halJIFormat_E nFormat);

bool_T halJPEGEncode2(int nIStartAddr, int nISize,
		int nIWidth, int nIHeight,
		int nOStartAddr, int *pnOSize,
		halJIFormat_E nFormat, unsigned char pLumTb[64],
		unsigned char pChrTb[64]);
		/* halJPEGEncode2() uses user-defined Q-tables (luminance, chrominance) */

bool_T halJPEGEncode3(int nIStartAddr, int nISize,
		int nIWidth, int nIHeight,
		int nOStartAddr, int *pnOSize,
		halJIFormat_E nFormat, int nQuality[2]);
		/* halJPEGEncode3() uses nQuality to compute suitable Q-tables */

bool_T halJPEGDecode(int nIStartAddr, int nISize,
		int nOStartAddr,
		unsigned int *pnWidth, unsigned int *pnHeight);
#else
//#define halJPEGCReset()
//#define halJPEGEncode(nIStartAddr, nISize, nIWidth, nIHeight, nOStartAddr, pnOSize, nFormat) (FALSE)
//#define halJPEGEncode2(nIStartAddr, nISize, nIWidth, nIHeight, nOStartAddr, pnOSize, nFormat, pLumTb, pChroTb) (FALSE)
//#define halJPEGEncode3(nIStartAddr, nISize, nIWidth, nIHeight, nOStartAddr, pnOSize, nFormat, nQuality) (FALSE)
//#define halJPEGDecode(nIStartAddr, nISize, nOStartAddr, pnWidth, pnHeight) (FALSE)
#endif


/* halUART.c */
#if defined(HAL_INC_UART)
void halUARTReset(void);
bool_T halUARTInitInMain(unsigned int nPClk, unsigned int nBaudRate);
unsigned int halUARTGetChar(void);
void halUARTPutChar(char c);
int halUARTPutString(char *szString);
int halUARTPrintf(const char *szFormat, ...);
#define halUARTEnableDataAvailableIntr() HAL_SETREG32(HAL_REG_UART_IER, 0x1)
#endif



/* halSSI.c */
#if 1

	/* API 1 */

#if defined(HAL_INC_SSI)
void halSSI_SPISetup(unsigned int nPCLK);
unsigned char halAT250x0AReadStatus(void);
void halAT250x0AWriteStatus(unsigned char nSts);
bool_T halAT250x0AReadData(unsigned short nAddr, unsigned char *pData);
void halAT250x0AWriteData(unsigned short nAddr, unsigned char nData);
#else
#define halSSI_SPISetup(nPCLK)
#define halAT250x0AReadStatus() (0)
#define halAT250x0AWriteStatus(nSts)
#define halAT250x0AReadData(nAddr, pData) (FALSE)
#define halAT250x0AWriteData(nAddr, nData)
#endif

#else

	/* API 2 */

#if defined(HAL_INC_SSI)
bool_T halSSI_SPISetup(unsigned short nSCKDV, unsigned short nDFS);
bool_T halSSI_SPIWriteRead(unsigned short *pWriteBuffer,
							unsigned short *pReadBuffer,
							unsigned int nWriteSize,
							unsigned int nReadSize);
bool_T halSSI_SPIWrite(unsigned short *pBuffer, unsigned int nSize);
#else
#define halSSI_SPISetup(nSCKDV, nDFS) (FALSE)
#define halSSI_SPIWriteRead(pWriteBuffer, pReadBuffer, nWriteSize, nReadSize) (FALSE)
#define halSSI_SPIWrite(pBuffer, nSize) (FALSE)
#endif

#endif


/* halDAI.c */
typedef struct halDAISetting_S
{
	bool_T bStereo;
	int nSamplingRate;
	halDAIDataProtocol_E nDataProtocol;
	int nTXFIFOThreshold;
	int nRXFIFOThreshold;
} halDAISetting_S;

#if defined(HAL_INC_DAI2)
INLINE bool_T halDAIIsBusy(void);
INLINE void halDAIResetFIFO(void);
INLINE void halDAIEnable(halDAIId_E);
INLINE void halDAIDisable(halDAIId_E);
bool_T halDAIConfig(halDAISetting_S *pSetting, unsigned int *pCodecCLK);
#define halDAIGetAudioOSCCLK() (HAL_AUDIO_OSC_CLK)
#else
#define halDAIIsBusy() (TRUE)
#define halDAIResetFIFO() 
#define halDAIEnable(a)
#define halDAIDisable(a)
#define halDAIConfig(a, b) (FALSE)
#define halDAIGetAudioOSCCLK() 0
#endif


/* halusbmass.c */
#if defined(HAL_INC_USB20)

#define HAL_USB20_INIT_BUF_SIZE	8600

/*
 * prototypes of USB's mass-storage IO routines
 */
typedef unsigned int (*halUSBMSGetCapacity_FT) (void);

typedef bool_T (*halUSBMSReadNSector_FT) (unsigned int /* start sector */,
										unsigned int /* sector number*/,
										void * /* buffer address */);

typedef bool_T (*halUSBMSWriteNSector_FT) (unsigned int /* start sector */,
										unsigned int /* sector number*/,
										void * /* buffer address */);

typedef PACKED_STRUCT halUSBMSIORoutine_S
{
	halUSBMSGetCapacity_FT getCapacity;
	halUSBMSReadNSector_FT readNSector;
	halUSBMSWriteNSector_FT writeNSector;
} halUSBMSIORoutine_S;


unsigned int usbInitInMain(halUSBMSIORoutine_S *ioRoutine, unsigned int *pUSBBuf);
	/*
	 * the buffer size pointed by pUSBBuf could not be smaller than
	 * HAL_USB20_INIT_BUF_SIZE
	 */

halUSBEvent_E usbIRQHandler(void);
void usbMassStorageEventHandler(void);
#else
#define usbInitInMain(ioRoutine)
#define usbIRQHandler() (USB_NO_EVT)
#define usbMassStorageEventHandler()
#endif


/* halUDSP24.c */
#if defined(HAL_INC_UDSP24)
void halUDSP24Reboot(void);
bool_T halUDSP24WriteMasterID(unsigned int nId);
unsigned int halUDSP24ReadMasterID(void);
void halUDSP24ReleaseMasterID(void);

void halUDSP24ReadMem(halUDSP24Mem_E nMType, unsigned short nUMAddr, unsigned int *pnContent);
void halUDSP24ReadMemSeg(halUDSP24Mem_E nMType, unsigned short nUMAddr, void *pDstMem,
		unsigned short nUMWords, bool_T bLittleEndian);

void halUDSP24WriteMem(halUDSP24Mem_E nMType, unsigned short nUMAddr, unsigned int nContent);
void halUDSP24WriteMemSeg(halUDSP24Mem_E nMType, unsigned short nUMAddr, void *pSrcMem,
		unsigned short nUMWords, bool_T bLittleEndian);
#else
#define halUDSP24Reboot()
#define halUDSP24WriteMasterID(nId) (FALSE)
#define halUDSP24ReadMasterID() (0)
#define halUDSP24ReleaseMasterID()
#define halUDSP24WriteMemory(nMType, nAddr, nContent)
#define halUDSP24ReadMem(nMType, nUMAddr, pnContent)
#define halUDSP24ReadMemSeg(nMType, nUMAddr, pDstMem, nUMWords, bLittleEndian)
#define halUDSP24WriteMem(nMType, nUMAddr, nContent)
#define halUDSP24WriteMemSeg(nMType, nUMAddr, pSrcMem, nUMWords, bLittleEndian)
#endif


/* halFlash.c */
void halFlashReset(void);
bool_T halFlashEraseAll(void);

bool_T halFlashIsValidFrame(unsigned int nFrameNum);
bool_T halFlashEraseFrame(unsigned int nFrameNum);
bool_T halFlashProgramFrame(unsigned int nFrameNum,
				unsigned char *pBuf, unsigned int nSize);
bool_T halFlashGetFrameSize(unsigned int nFrameNum, unsigned int *pnSize);
bool_T halFlashGetFrameOffsetAddr(unsigned int nFrameNum, unsigned int *pnStartAddr);
bool_T halFlashGetFrameNum(unsigned int nPhyAddr, unsigned int *pnFrameNum);

bool_T halFlashProgramSeg(unsigned int nSegStartAddr,
				unsigned char *pBuf, unsigned int nSize);
bool_T halFlashProgram32(unsigned int nAddr, unsigned int nData);
/*bool_T halFlashProgram16(unsigned int nAddr, unsigned short nData);*/
bool_T halFlashProgram8(unsigned int nAddr, unsigned char nData);

#define halFlashRead32(nAddr)	HAL_GETMEM32(HAL_FLASH_BASE + (nAddr & 0xFFFFFFFC))
#define halFlashRead16(nAddr)	HAL_GETMEM16(HAL_FLASH_BASE + (nAddr & 0xFFFFFFFE))
#define halFlashRead8(nAddr)	HAL_GETMEM8(HAL_FLASH_BASE + nAddr)


/* halWDT.c */
#if defined(HAL_INC_WDT)
void halWDTEnable(void);
void halWDTDisable(void);
void halWDTReStartNew(unsigned int nPeriod, halWDTClkSrc_E sourceClk,
		bool_T bResetOut, bool_T bIntrSys, bool_T bResetSys);
void halWDTReStart(void);
#define halWDTClearTimeOut() (HAL_SETREG(HAL_REG_WDT_Clr, 1))
#else
#define halWDTEnable()
#define halWDTDisable()
#define halWDTReStartNew(nPeriod, sourceClk, bResetOut, bIntrSys, bResetSys)
#define halWDTReStart()
#define halWDTClearTimeOut() 
#endif


/* halSM.c */
#if defined(HAL_INC_SM)
bool_T halSMCardExist(void);
bool_T halSMInstall(void);
bool_T halSMUninstall(void);
unsigned int halSMGetCapacity(void);
bool_T halSMReadNSector(unsigned nstartSect,unsigned int nSectNum,void *buffer);
bool_T halSMWriteNSector(unsigned int nStartSect, unsigned int nSectNum,void *buffer);
#else
#define halSMCardExist() (FALSE)
#define halSMInstall() (FALSE)
#define halSMUninstall() (FALSE)
#define halSMGetCapacity(void) (0)
#define halSMReadNSector(nstartSect, nSectNum, buffer) (FALSE)
#define halSMWriteNSector(nStartSect, nSectNum, buffer) (FALSE)
#endif



/* halSD.c   : for GUC SD_MMC IP, use the same interface for sd and mmc cards
   halSD2.c  : for GUC SD_MMC IP, only sd card access function
   halDWSD.c : for design_ware SD_MMC IP, only for sd card access

   1. to compile halSD.c, one need to define HAL_INC_SD and not define HAL_INC_SD2
   2. to compile halSD2.c, one need to define HAL_INC_SD and HAL_INC_SD2
   3. to compile halDWSD.c, one need to define HAL_INC_DWSD
   
   stan's opinion : either open HAL_INC_SD or HAL_INC_DWSD
*/
bool_T halSDCardExist(void);
bool_T halSDInstall(int sd_pclk);
bool_T halSDUninstall(void);
void halSDDMAEnable(unsigned int en);
unsigned int halSDGetCapacity(void);
bool_T halSDWriteProtect(void);
bool_T halSDReadNSector( unsigned int nStartSect, unsigned int nSectNum, void *pBuf);
bool_T halSDWriteNSector( unsigned int nStartSect, unsigned int nSectNum, void *pBuf);



/* halMMC2.c */
#if defined(HAL_INC_MMC2)

//========================Register Mapping ==============================
#define	HAL_REG_MMC_BASE							HAL_REG_SDC_BASE
#define HAL_REG_MMC_PWRCTRLR					(HAL_REG_MMC_BASE+0x00)
#define HAL_REG_MMC_CLKCTRLR					(HAL_REG_MMC_BASE+0x04)
#define HAL_REG_MMC_ARGR							(HAL_REG_MMC_BASE+0x08)
#define HAL_REG_MMC_CMDR							(HAL_REG_MMC_BASE+0x0C)
#define HAL_REG_MMC_RESPCMDR					(HAL_REG_MMC_BASE+0x10)
#define HAL_REG_MMC_RESP0R						(HAL_REG_MMC_BASE+0x14)
#define HAL_REG_MMC_RESP1R						(HAL_REG_MMC_BASE+0x18)
#define HAL_REG_MMC_RESP2R						(HAL_REG_MMC_BASE+0x1C)
#define HAL_REG_MMC_RESP3R						(HAL_REG_MMC_BASE+0x20)
#define HAL_REG_MMC_DATTMR						(HAL_REG_MMC_BASE+0x24)
#define HAL_REG_MMC_DATLENR						(HAL_REG_MMC_BASE+0x28)
#define HAL_REG_MMC_DATCTRLR					(HAL_REG_MMC_BASE+0x2C)
#define HAL_REG_MMC_DATCNTR						(HAL_REG_MMC_BASE+0x30)
#define HAL_REG_MMC_STSR							(HAL_REG_MMC_BASE+0x34)
#define HAL_REG_MMC_INTRMASKR					(HAL_REG_MMC_BASE+0x38)
#define HAL_REG_MMC_CLRR							(HAL_REG_MMC_BASE+0x3C)
#define HAL_REG_MMC_DATR							(HAL_REG_MMC_BASE+0x40)

//========================MMC Function Export ==============================
void	halMMC_Init_i(void);
unsigned char halMMCCardExist(void);
unsigned char halMMCInstall(unsigned int nPCLK);
unsigned char halMMCUninstall(void);
void halMMCDMAEnable(unsigned char bEn, unsigned char bChanelNo);
unsigned int halMMCGetCapacity(void);
unsigned char halMMCReadNSector( unsigned int nStartSect, unsigned int nSectNum, void *pBuf);
unsigned char halMMCWriteNSector( unsigned int nStartSect, unsigned int nSectNum, void *pBuf);
#endif


#if defined(HAL_INC_MPEG4_DEC)
#define dMP4_BLOCK_END		0x4
#define dMP4_REO_DONE			0x2
#define	dMP4_DONE					0x1

typedef PACKED_STRUCT halMP4_DEC_CONFIGURE_S
{
	unsigned short int	nMP4_DEC_Func;
	unsigned int				nBitstream_Ini_Addr;
	unsigned int				nBitstream_End_Addr;
	unsigned int				nY_Out_Addr;
	unsigned int				nCb_Out_Addr;
	unsigned int				nCr_Out_Addr;	
	unsigned int				nY0_MC_Addr;
	unsigned int				nCb0_MC_Addr;
	unsigned int				nCr0_MC_Addr;
	unsigned int				nY1_MC_Addr;
	unsigned int				nCb1_MC_Addr;
	unsigned int				nCr1_MC_Addr;	
} halMP4_DEC_CONFIGURE_S;

typedef PACKED_STRUCT halMP4_DEC_INFO_S
{
	unsigned short int 	nWidth;
	unsigned short int	nHeight;
	unsigned short int  nFix_Time_Inc;
	unsigned short int  nTime_Inc_Reso;
} halMP4_DEC_INFO_S;

typedef PACKED_STRUCT halMP4_DEC_TIME_INFO_S
{	
	unsigned short int	nTime_Inc;
	unsigned char 			nMod_Time_Base;	
} halMP4_DEC_TIME_INFO_S;


typedef PACKED_STRUCT halMP4_DEC_TIME_CODE_S
{	
	unsigned char				nTime_Hours;
	unsigned char				nTime_Mins;
	unsigned char 			nTime_Secs;
} halMP4_DEC_TIME_CODE_S;
typedef PACKED_STRUCT halMP4_DEC_START_S
{
	unsigned char				bMP4_Start;
	unsigned char				bREO_Start;
	unsigned char				bBLOCK_Start;
	unsigned char				bLast_Block;	
} halMP4_DEC_START_S;

typedef PACKED_STRUCT halMP4_DEC_TIMER_S
{
	unsigned int nTimer_Cnt;
	unsigned int nTimer_Mod;
} halMP4_DEC_TIMER_S;

void halMPEG4DECReset(void);
void halMPEG4DECGetInfo(halMP4_DEC_INFO_S *mp4_dec_info_s);
void halMPEG4DECGetTimeInfo(halMP4_DEC_TIME_INFO_S *mp4_dec_time_info_s);
unsigned char halMPEG4DECGetTimeCode(struct halMP4_DEC_TIME_CODE_S *mp4_dec_time_code_s);
unsigned char halMPEG4DECGetInt(void);
unsigned char halMPEG4DECStart(halMP4_DEC_START_S *mp4_dec_start_s,halMP4_DEC_TIMER_S *mp4_dec_timer_s,
															 unsigned char *nINT_Flag,unsigned char bFirstVOP);													 
void halMPEG4DECStart_i(halMP4_DEC_START_S *mp4_dec_start_s,unsigned char *nINT_Flag);
void halMPEG4DECConfigure(struct halMP4_DEC_CONFIGURE_S *mp4_dec_config_s);
void halMPEG4DECOutputBuffer(struct halMP4_DEC_CONFIGURE_S *mp4_dec_config_s);
void halMPEG4DECSetBistreamAdd(unsigned int nIni_Add,unsigned int nEnd_Add);
#endif


/*********************************************/
/* Please add new IP's APIs below this line. */
/*********************************************/
/* halH264ENC.c */
#if defined(HAL_INC_H264_ENC)
//========================Register Mapping ==============================
#define HAL_REG_H264E_CMD						(HAL_REG_H264E_BASE+0x000)
#define HAL_REG_H264E_STAT					(HAL_REG_H264E_BASE+0x020)
#define HAL_REG_H264E_INT						(HAL_REG_H264E_BASE+0x024)
#define HAL_REG_H264E_PAR1					(HAL_REG_H264E_BASE+0x200)
#define HAL_REG_H264E_PAR2					(HAL_REG_H264E_BASE+0x204)
#define HAL_REG_H264E_PAR3					(HAL_REG_H264E_BASE+0x208)
#define HAL_REG_H264E_PAR4					(HAL_REG_H264E_BASE+0x20C)
#define HAL_REG_H264E_PAR5					(HAL_REG_H264E_BASE+0x210)
#define HAL_REG_H264E_PAR6					(HAL_REG_H264E_BASE+0x214)
#define HAL_REG_H264E_PAR7					(HAL_REG_H264E_BASE+0x218)
#define HAL_REG_H264E_PAR8					(HAL_REG_H264E_BASE+0x21C)
#define HAL_REG_H264E_PAR9					(HAL_REG_H264E_BASE+0x220)
#define HAL_REG_H264E_PAR10					(HAL_REG_H264E_BASE+0x224)
#define HAL_REG_H264E_PAR11					(HAL_REG_H264E_BASE+0x228)
#define HAL_REG_H264E_PAR12					(HAL_REG_H264E_BASE+0x22C)
#define HAL_REG_H264E_PAR13					(HAL_REG_H264E_BASE+0x230)
#define HAL_REG_H264E_PAR14					(HAL_REG_H264E_BASE+0x234)
#define HAL_REG_H264E_PAR15					(HAL_REG_H264E_BASE+0x238)
#define HAL_REG_H264E_PAR16					(HAL_REG_H264E_BASE+0x23C)
#define HAL_REG_H264E_PAR17					(HAL_REG_H264E_BASE+0x240)
#define HAL_REG_H264E_PAR18					(HAL_REG_H264E_BASE+0x244)
#define HAL_REG_H264E_PAR19					(HAL_REG_H264E_BASE+0x248)
#define HAL_REG_H264E_INFO_REG1			(HAL_REG_H264E_BASE+0x250)
#define HAL_REG_H264E_INFO_REG2			(HAL_REG_H264E_BASE+0x254)
#define HAL_REG_H264E_INFO_REG3			(HAL_REG_H264E_BASE+0x298)
#define HAL_REG_H264E_INFO_REG4			(HAL_REG_H264E_BASE+0x29C)
#define HAL_REG_H264E_PAR20					(HAL_REG_H264E_BASE+0x2C4)
#define HAL_REG_H264E_PAR21					(HAL_REG_H264E_BASE+0x2BC)
#define HAL_REG_H264E_PAR22					(HAL_REG_H264E_BASE+0x25C)
#define HAL_REG_H264E_PAR23					(HAL_REG_H264E_BASE+0x2B4)
#define HAL_REG_H264E_PAR24					(HAL_REG_H264E_BASE+0x2CC)
#define HAL_REG_H264E_PAR25					(HAL_REG_H264E_BASE+0x2DC)
#define HAL_REG_H264E_PAR26					(HAL_REG_H264E_BASE+0x2E8)
#define HAL_REG_H264E_PAR27					(HAL_REG_H264E_BASE+0x2F0)
#define HAL_REG_H264E_Y0_ADDR				(HAL_REG_H264E_BASE+0x400)
#define HAL_REG_H264E_Y1_ADDR				(HAL_REG_H264E_BASE+0x404)
#define HAL_REG_H264E_BAR3					(HAL_REG_H264E_BASE+0x408)
#define HAL_REG_H264E_BAR4					(HAL_REG_H264E_BASE+0x40C)
#define HAL_REG_H264E_BAR19					(HAL_REG_H264E_BASE+0x448)
#define HAL_REG_H264E_BUFSIZE19			(HAL_REG_H264E_BASE+0x44C)
#define HAL_REG_H264E_BAR20					(HAL_REG_H264E_BASE+0x450)
#define HAL_REG_H264E_BUFSIZE20			(HAL_REG_H264E_BASE+0x454)
#define HAL_REG_H264E_ST_BUF_SEL		(HAL_REG_H264E_BASE+0x458)
#define HAL_REG_H264E_CODEC_MEM_CTL	(HAL_REG_H264E_BASE+0x45C)
#define HAL_REG_H264E_ST_BUF_ADDR		(HAL_REG_H264E_BASE+0x460)
#define HAL_REG_H264E_ST_BUF_INT		(HAL_REG_H264E_BASE+0x470)
#define	HAL_REG_H264E_CTRL					(HAL_REG_H264E_BASE+0x4A0)
#define	HAL_REG_H264E_U0_ADDR				(HAL_REG_H264E_BASE+0x4A4)
#define	HAL_REG_H264E_V0_ADDR				(HAL_REG_H264E_BASE+0x4A8)
#define	HAL_REG_H264E_U1_ADDR				(HAL_REG_H264E_BASE+0x4AC)
#define	HAL_REG_H264E_V1_ADDR				(HAL_REG_H264E_BASE+0x4B0)

typedef PACKED_STRUCT halH264_ENC_STAT_S
{
	unsigned char nActionState:2;
	unsigned char nPicType:2;
}halH264_ENC_STAT_S;


typedef PACKED_STRUCT halH264_ENC_INT_CONF_S
{
	unsigned char bEND_En:1;
	unsigned char bPIC_END_En:1;
}halH264_ENC_INT_CONF_S;

typedef PACKED_STRUCT halH264_ENC_INT_S
{
	unsigned char bEnd:1;
	unsigned char bPIC_End:1;	
	unsigned char bBUF_Full:1;
}halH264_ENC_INT_S;

typedef PACKED_STRUCT halH264_ENC_CROP_S
{
	unsigned char  bEn;
	unsigned short nLeft;
	unsigned short nRight;
	unsigned short nTop;
	unsigned short nBottom;
}halH264_ENC_CROP_S;

typedef PACKED_STRUCT halH264_ENC_MISC_S
{
	unsigned char bConstrain_Intra_Pred:1;
	unsigned char nChro_QP_Idx_Off:5;
	unsigned char	bConstrain_Set2:1;
	unsigned char	bConstrain_Set1:1;
	unsigned char	bConstrain_Set0:1;
	unsigned char	nProfic_IDC:8;
	unsigned char	nLevel_IDC:8;					
}halH264_ENC_MISC_S;

typedef PACKED_STRUCT halH264_ENC_RATE_S
{
	unsigned short	nRC_Rate:13;
	unsigned short  nMax_Rate:14;
	unsigned short	nVBZ_Size:9;
	unsigned short	nFrameRate:6;
	unsigned short	nI_InterVal:10;
	unsigned short	nSPS_InterVal:10;
	unsigned short	nPPS_InterVal:10;
	unsigned char		bSkip_Off:1;
	unsigned char 	nSkip_Mode:3;
}halH264_ENC_RATE_S;

typedef PACKED_STRUCT halH264_ENC_IMG_ADJUST_S
{
	unsigned char		nENC_Img_Val:6;		
	unsigned char 	nInit_Img_Val:6;
	unsigned char		nISlice_Img_Mode:6;
	unsigned char		bISlice_Img_Value_En:1;
	unsigned char		nISlice_Img_Value:6;
	unsigned short	nStill_Img_Value1:13;
	unsigned short	nStill_Img_Value2:16;	
	unsigned short	nStill_Flat_Value1:14;	
}halH264_ENC_IMG_ADJUST_S;

typedef PACKED_STRUCT halH264_ENC_DEBLK_S
{
	unsigned char nDisable_DeBlk_Filter_I:2;
	unsigned char nAlpha_C0_Off_I:4;
	unsigned char nBeta_Off_I:4;
	unsigned char nDisable_DeBlk_Filter_P:2;
	unsigned char nAlpha_C0_Off_P:4;
	unsigned char nBeta_Off_P:4;					
}halH264_ENC_DEBLK_S;

typedef PACKED_STRUCT halH264_ENC_CONF_S
{
	unsigned char bVBR:1;
	unsigned char bField:1;
	unsigned char bHW_Reorder:1;
	unsigned char bInIMG_LittleEndian:1;
	unsigned char bOutBS_LittleEndian:1;
	unsigned char bInIMG_Filed2Frame:1;
	unsigned short nWidth:11;
	unsigned short nHeight:11;
}halH264_ENC_CONF_S;

typedef PACKED_STRUCT halH264_ENC_BASE_ADR_S
{
	unsigned int nYPlane0Addr;
	unsigned int nUPlane0Addr;
	unsigned int nVPlane0Addr;
	unsigned int nYPlane1Addr;
	unsigned int nUPlane1Addr;
	unsigned int nVPlane1Addr;
	unsigned int nYUVPlane0;
	unsigned int nYUVPlane1;
	unsigned int nBar19Addr;
	unsigned int nBar19Size;
	unsigned int nBar20Addr;
	unsigned int nBar20Size;
}halH264_ENC_BASE_ADR_S;

//========================H264 ENC Function Export ==============================
void halH264ENC_Reset(void);
void halH264ENCInit_i(void);
void halH264ENC_Init(halH264_ENC_CONF_S *halh264_enc_conf_s);
void halH264ENC_Rate_Set(halH264_ENC_RATE_S *halh264_enc_rate_s);
void halH264ENC_Misc_Set(halH264_ENC_MISC_S *halh264_enc_misc_s);
void halH264ENC_Conf_DeBlk(halH264_ENC_DEBLK_S *halh264_enc_deblk_s);
void halH264ENC_Get_INT(halH264_ENC_INT_S *halh264_enc_int_s);
void halH264ENC_INT_Conf(halH264_ENC_INT_CONF_S *halh264_enc_int_conf_s);									
void halH264ENC_Set_Base_Addr(halH264_ENC_BASE_ADR_S *halh264_enc_base_adr_s);
void halH264ENC_StreamSel(void);
void halH264ENC_Start(unsigned char bStart);
void halH264ENC_Set_Crop(halH264_ENC_CROP_S *halh264_enc_crop_s);
void halH264ENC_GetStat(halH264_ENC_STAT_S *halh264_enc_stat_s);
unsigned char halH264ENC_GetMemWork(void);
unsigned char halH264ENC_GetCurStrem(void);
unsigned int halH264ENC_GetFrameSize(void);
unsigned int halH264ENC_GetStreamSize(void);
#endif


/* halH264DEC.c */
#if defined(HAL_INC_H264_DEC)
//========================Register Mapping ==============================
#define HAL_REG_H264D_CMD						(HAL_REG_H264D_BASE+0x000)
#define HAL_REG_H264D_STAT					(HAL_REG_H264D_BASE+0x020)
#define HAL_REG_H264D_INT						(HAL_REG_H264D_BASE+0x024)
#define HAL_REG_H264D_PAR1					(HAL_REG_H264D_BASE+0x100)
#define HAL_REG_H264D_PAR2					(HAL_REG_H264D_BASE+0x104)
#define HAL_REG_H264D_PAR3					(HAL_REG_H264D_BASE+0x108)
#define HAL_REG_H264D_PAR4					(HAL_REG_H264D_BASE+0x10C)
#define HAL_REG_H264D_PAR5					(HAL_REG_H264D_BASE+0x110)
#define HAL_REG_H264D_PAR6					(HAL_REG_H264D_BASE+0x114)
#define HAL_REG_H264D_PAR7					(HAL_REG_H264D_BASE+0x118)
#define HAL_REG_H264D_PAR8					(HAL_REG_H264D_BASE+0x11C)
#define HAL_REG_H264D_PAR9					(HAL_REG_H264D_BASE+0x120)
#define HAL_REG_H264D_HPAR1					(HAL_REG_H264D_BASE+0x300)
#define HAL_REG_H264D_HPAR2					(HAL_REG_H264D_BASE+0x304)
#define HAL_REG_H264D_HPAR3					(HAL_REG_H264D_BASE+0x308)
#define HAL_REG_H264D_HPAR4					(HAL_REG_H264D_BASE+0x30C)
#define HAL_REG_H264D_HPAR5					(HAL_REG_H264D_BASE+0x310)
#define HAL_REG_H264D_HPAR6					(HAL_REG_H264D_BASE+0x314)
#define HAL_REG_H264D_HPAR7					(HAL_REG_H264D_BASE+0x318)
#define HAL_REG_H264D_HPAR8					(HAL_REG_H264D_BASE+0x31C)
#define HAL_REG_H264D_HPAR9					(HAL_REG_H264D_BASE+0x320)
#define HAL_REG_H264D_HPAR10				(HAL_REG_H264D_BASE+0x324)
#define HAL_REG_H264D_HPAR11				(HAL_REG_H264D_BASE+0x328)
#define HAL_REG_H264D_HPAR12				(HAL_REG_H264D_BASE+0x32C)
#define HAL_REG_H264D_HPAR13				(HAL_REG_H264D_BASE+0x330)
#define HAL_REG_H264D_HPAR14				(HAL_REG_H264D_BASE+0x334)
#define HAL_REG_H264D_HPAR15				(HAL_REG_H264D_BASE+0x338)
#define HAL_REG_H264D_HPAR16				(HAL_REG_H264D_BASE+0x33C)
#define HAL_REG_H264D_HPAR17				(HAL_REG_H264D_BASE+0x340)
#define HAL_REG_H264D_HPAR18				(HAL_REG_H264D_BASE+0x344)
#define HAL_REG_H264D_HPAR19				(HAL_REG_H264D_BASE+0x348)
#define HAL_REG_H264D_HPAR20				(HAL_REG_H264D_BASE+0x34C)
#define HAL_REG_H264D_HPAR21				(HAL_REG_H264D_BASE+0x350)
#define HAL_REG_H264D_HPAR22				(HAL_REG_H264D_BASE+0x354)
#define HAL_REG_H264D_HPAR23				(HAL_REG_H264D_BASE+0x358)
#define HAL_REG_H264D_HPAR24				(HAL_REG_H264D_BASE+0x35C)
#define HAL_REG_H264D_HPAR25				(HAL_REG_H264D_BASE+0x360)
#define HAL_REG_H264D_HPAR26				(HAL_REG_H264D_BASE+0x364)
#define HAL_REG_H264D_HPAR27				(HAL_REG_H264D_BASE+0x368)
#define HAL_REG_H264D_HPAR28				(HAL_REG_H264D_BASE+0x36C)
#define HAL_REG_H264D_HPAR29				(HAL_REG_H264D_BASE+0x370)
#define HAL_REG_H264D_HPAR30				(HAL_REG_H264D_BASE+0x374)
#define HAL_REG_H264D_HPAR31				(HAL_REG_H264D_BASE+0x378)
#define HAL_REG_H264D_HPAR32				(HAL_REG_H264D_BASE+0x37C)
#define HAL_REG_H264D_BAR1					(HAL_REG_H264D_BASE+0x400)
#define HAL_REG_H264D_BAR2					(HAL_REG_H264D_BASE+0x404)
#define HAL_REG_H264D_BAR3					(HAL_REG_H264D_BASE+0x408)
#define HAL_REG_H264D_BAR4					(HAL_REG_H264D_BASE+0x40C)
#define HAL_REG_H264D_BAR5					(HAL_REG_H264D_BASE+0x410)
#define HAL_REG_H264D_BAR6					(HAL_REG_H264D_BASE+0x414)
#define HAL_REG_H264D_BAR7					(HAL_REG_H264D_BASE+0x418)
#define HAL_REG_H264D_BAR8					(HAL_REG_H264D_BASE+0x41C)
#define HAL_REG_H264D_BAR9					(HAL_REG_H264D_BASE+0x420)
#define HAL_REG_H264D_BAR10					(HAL_REG_H264D_BASE+0x424)
#define HAL_REG_H264D_BAR11					(HAL_REG_H264D_BASE+0x429)
#define HAL_REG_H264D_BAR12					(HAL_REG_H264D_BASE+0x42C)
#define HAL_REG_H264D_BAR13					(HAL_REG_H264D_BASE+0x430)
#define HAL_REG_H264D_BAR14					(HAL_REG_H264D_BASE+0x434)
#define HAL_REG_H264D_BAR15					(HAL_REG_H264D_BASE+0x438)
#define HAL_REG_H264D_BAR16					(HAL_REG_H264D_BASE+0x43C)
#define HAL_REG_H264D_BAR17					(HAL_REG_H264D_BASE+0x440)
#define HAL_REG_H264D_BAR18					(HAL_REG_H264D_BASE+0x444)
#define HAL_REG_H264D_BAR19					(HAL_REG_H264D_BASE+0x448)
#define HAL_REG_H264D_BUFSIZE19			(HAL_REG_H264D_BASE+0x44C)
#define HAL_REG_H264D_BAR20					(HAL_REG_H264D_BASE+0x450)
#define HAL_REG_H264D_BUFSIZE20			(HAL_REG_H264D_BASE+0x454)
#define HAL_REG_H264D_ST_BUF_SEL		(HAL_REG_H264D_BASE+0x458)
#define HAL_REG_H264D_CODEC_MEM_CTL	(HAL_REG_H264D_BASE+0x45C)
#define HAL_REG_H264D_ST_BUF_ADDR		(HAL_REG_H264D_BASE+0x460)
#define HAL_REG_H264D_DISP_CTRL			(HAL_REG_H264D_BASE+0x470)
#define HAL_REG_H264D_REO_ADDR1			(HAL_REG_H264D_BASE+0x474)
#define HAL_REG_H264D_REO_ADDR2			(HAL_REG_H264D_BASE+0x478)
#define HAL_REG_H264D_ST_BUF_INT		(HAL_REG_H264D_BASE+0x480)

typedef PACKED_STRUCT halH264_DEC_DISP_S
{
	unsigned char bInterlace:1;
	unsigned char bL_Endian_Ref_W:1;
	unsigned char bL_Endian_Ref_R:1;
	unsigned char bL_Endian_Stream_R:1;
	unsigned char bL_Endian_Output_W:1;
	unsigned char nYUV_Order:2;								
}halH264_DEC_DISP_S;
typedef PACKED_STRUCT halH264_DEC_REF_IDX_S
{
	unsigned char nRef_Idx0:5;
	unsigned char nRef_Idx1:5;
	unsigned char nRef_Idx2:5;
	unsigned char nRef_Idx3:5;
	unsigned char nRef_Idx4:5;
	unsigned char nRef_Idx5:5;
	unsigned char nRef_Idx6:5;
	unsigned char nRef_Idx7:5;
	unsigned char nRef_Idx8:5;
	unsigned char nRef_Idx9:5;
	unsigned char nRef_Idx10:5;
	unsigned char nRef_Idx11:5;
	unsigned char nRef_Idx12:5;
	unsigned char nRef_Idx13:5;
	unsigned char nRef_Idx14:5;
	unsigned char nRef_Idx15:5;	
}halH264_DEC_REF_IDX_S;
typedef PACKED_STRUCT halH264_DEC_SEI_RAW_OUTPUT_S
{
	unsigned char bOutput0:1;
	unsigned char bOutput1:1;
	unsigned char bOutput2:1;
	unsigned char bOutput3:1;
	unsigned char bOutput4:1;
	unsigned char bOutput5:1;
	unsigned char bOutput6:1;
	unsigned char bOutput7:1;
	unsigned char bOutput8:1;
	unsigned char bOutput9:1;
	unsigned char bOutput10:1;
	unsigned char bOutput11:1;
	unsigned char bOutput12:1;
	unsigned char bOutput13:1;
	unsigned char bOutput14:1;
	unsigned char bOutput15:1;
	unsigned char bOutput16:1;
	unsigned char bOutput17:1;
	unsigned char bOutput18:1;
	unsigned char bOutput19:1;	
	unsigned char bOutput20:1;
	unsigned char bOutput21:1;		
}halH264_DEC_SEI_RAW_OUTPUT_S;
typedef PACKED_STRUCT halH264_DEC_START_S
{
	unsigned char bStart:1;
	unsigned char bStop:1;
	unsigned char bReActivate:1;
	unsigned char bMonStart:1;		
}halH264_DEC_START_S;

typedef PACKED_STRUCT halH264_DEC_MISC_S
{
	unsigned char	nMax_Work_Num:5;
	unsigned char bRef_HW_Ctrl:1;
	unsigned char	bErr_Conceal:1;
	unsigned char bNALHRDBP_Present:1;
	unsigned char bVCLHRDBP_Present:1;
	unsigned char bCPBDPBDelay_Present:1;
}halH264_DEC_MISC_S;

typedef PACKED_STRUCT halH264_DEC_STAT_S
{
	unsigned char nActionState:2;
	unsigned char nPICType:2;
	unsigned char nPrimary_PIC_Type:2;
	unsigned char bMB_Adaptive_Frame_Field:1;
	unsigned char nSlice_Type:4;
}halH264_DEC_STAT_S;

typedef PACKED_STRUCT halH264_DEC_INT_CONF_S
{
	unsigned char bEnd_En:1;
	unsigned char bERR_En:1;
	unsigned char bPIC_End_En:1;
	unsigned char bHead_Dec_End_En:1;
	unsigned char bRef_PIC_Man_En:1;	
}halH264_DEC_INT_CONF_S;

typedef PACKED_STRUCT halH264_DEC_INT_S
{
	unsigned char bEnd:1;
	unsigned char bERR:1;
	unsigned char bPIC_End:1;
	unsigned char bHead_Dec_End:1;
	unsigned char bRef_PIC_Man:1;
	unsigned char bBUF_Full:1;
}halH264_DEC_INT_S;

typedef PACKED_STRUCT halH264_DEC_BASE_ADDR_S
{		
	unsigned int nYUVPlane0;
	unsigned int nYUVPlane1;
	unsigned int nYUVPlane2;
	unsigned int nYUVPlane3;
	unsigned int nYUVPlane4;
	unsigned int nYUVPlane6;
	unsigned int nYUVPlane7;
	unsigned int nYUVPlane8;
	unsigned int nYUVPlane9;
	unsigned int nYUVPlane10;
	unsigned int nYUVPlane11;	
	unsigned int nYUVPlane12;
	unsigned int nYUVPlane13;
	unsigned int nYUVPlane14;
	unsigned int nYUVPlane15;
	unsigned int nYUVPlane16;
	unsigned int nYUVPlane17;
	unsigned int nYUVPlane18;			
	unsigned int nBar19Addr;
	unsigned int nBar19Size;
	unsigned int nBar20Addr;
	unsigned int nBar20Size;	
	unsigned int nF0_Addr;
	unsigned int nF1_Addr;
}halH264_DEC_BASE_ADDR_S;

typedef PACKED_STRUCT halH264_DEC_VIDEO_SIZE_S
{
	unsigned short nWidth;
	unsigned short nHeight;
	unsigned char  bCrop_Flag:1;
	unsigned short nCropRect_Left_Off;
	unsigned short nCropRect_Right_Off;
	unsigned short nCropRect_Top_Off;
	unsigned short nCropRect_Bottom_Off;
}halH264_DEC_VIDEO_SIZE_S;

typedef PACKED_STRUCT halH264_DEC_FRAME_INFO_S
{
	unsigned char bFrame_MBS_Only:1;
	unsigned char bField:1;
	unsigned char bBottom_Field:1;
	unsigned char nNum_Ref_Frames:5;
	unsigned char nNAL_Unit_Type:5;
	unsigned char nNum_Reo_Frames:5;
	unsigned char bNo_Output_Of_Prior_Pics:1;
	unsigned char nPic_Order_Cnt_Type:2;
	unsigned char bGaps_In_Frame_Num_Value_Allowed:1;
	unsigned char bRe_Pic_List_Reo:1;
	unsigned char bLong_Term_Ref:1;
	unsigned char bAdaptive_Ref_Pic_Mark_Mode:1;	
}halH264_DEC_FRAME_INFO_S;

typedef PACKED_STRUCT halH264_DEC_NAL_ERR_S
{	
	unsigned char bERR_NAL_End:1;
	unsigned char bERR_Forbidden_Zero:1;
	unsigned char bERR_Ref_IDC:1;
	unsigned char bERR_NAL_Unit_Type:1;
}halH264_DEC_NAL_ERR_S;

typedef PACKED_STRUCT halH264_DEC_NAL_ERR_CONF_S
{	
	unsigned char bERR_NAL_End_En:1;
	unsigned char bERR_Forbidden_Zero_En:1;
	unsigned char bERR_Ref_IDC_En:1;
	unsigned char bERR_NAL_Unit_Type_En:1;
}halH264_DEC_NAL_ERR_CONF_S;

typedef PACKED_STRUCT halH264_DEC_SPS_ERR_S
{	
	unsigned char	bERR_HRD:1;
	unsigned char	bERR_VUI:1;
	unsigned char	bERR_SPS_OTHER:1;
	unsigned char	bERR_Direct_8x8_Interference:1;	
	unsigned char bERR_MB_Adaptive_Frame_Field:1;
	unsigned char bERR_Profile:1;
	unsigned char bERR_Level_IDC:1;
	unsigned char bERRSEQ_Para_Set_ID:1;
	unsigned char bLog2_Max_Frame_Num_Minus4:1;
	unsigned char bERR_PIC_Order_Cnt_Type:1;
	unsigned char bERR_Log2_Max_Pic_Order_Cnt_LSB_Minus4:1;
	unsigned char bErr_Num_Ref_Frame:1;
	unsigned char bErr_Pic_Width_In_MBS_Minus:1;
	unsigned char bErr_PIC_Height_In_Map_Units_Minus1:1;
}halH264_DEC_SPS_ERR_S;

typedef PACKED_STRUCT halH264_DEC_SPS_ERR_CONF_S
{	
	unsigned char	bERR_HRD_En:1;
	unsigned char	bERR_VUI_En:1;
	unsigned char	bERR_SPS_OTHER_En:1;
	unsigned char	bERR_Direct_8x8_Interference_En:1;	
	unsigned char bERR_MB_Adaptive_Frame_Field_En:1;
	unsigned char bERR_Profile_En:1;
	unsigned char bERR_Level_IDC_En:1;
	unsigned char bERRSEQ_Para_Set_ID_En:1;
	unsigned char bLog2_Max_Frame_Num_Minus4_En:1;
	unsigned char bERR_PIC_Order_Cnt_Type_En:1;
	unsigned char bERR_Log2_Max_Pic_Order_Cnt_LSB_Minus4_En:1;
	unsigned char bErr_Num_Ref_Frame_En:1;
	unsigned char bErr_Pic_Width_In_MBS_Minus_En:1;
	unsigned char bErr_PIC_Height_In_Map_Units_Minus1_En:1;
}halH264_DEC_SPS_ERR_CONF_S;

typedef PACKED_STRUCT halH264_DEC_PPS_ERR_S
{
	unsigned char bERR_PPS_ID:1;
	unsigned char bERR_SPS_ID:1;
	unsigned char bERR_Entropy_Flag:1;
	unsigned char bERR_Num_Slice:1;
	unsigned char bERR_Num_Ref_Idx:1;
	unsigned char bERR_Weighted_Flag:1;
	unsigned char bERR_Weighted_IDC:1;
	unsigned char bERR_PIC_Init_Q:1;
	unsigned char bERR_CQP_Idx_Ofs:1;
	unsigned char bERR_Redundant_Flag:1;	
}halH264_DEC_PPS_ERR_S;

typedef PACKED_STRUCT halH264_DEC_PPS_ERR_CONF_ERR_S
{
	unsigned char bERR_PPS_ID_En:1;
	unsigned char bERR_SPS_ID_En:1;
	unsigned char bERR_Entropy_Flag_En:1;
	unsigned char bERR_Num_Slice_En:1;
	unsigned char bERR_Num_Ref_Idx_En:1;
	unsigned char bERR_Weighted_Flag_En:1;
	unsigned char bERR_Weighted_IDC_En:1;
	unsigned char bERR_PIC_Init_Q_En:1;
	unsigned char bERR_CQP_Idx_Ofs_En:1;
	unsigned char bERR_Redundant_Flag_En:1;	
}halH264_DEC_PPS_ERR_CONF_ERR_S;

typedef PACKED_STRUCT halH264_DEC_SH_ERR_S
{
	unsigned char bERR_End:1;
	unsigned char bERR_Slice_Type:1;
	unsigned char bERR_PPS_ID:1;
	unsigned char bERR_IDR_PIC_ID:1;
	unsigned char bERR_NUM_Ref_IDC:1;		
	unsigned char bERR_DBF_IDC:1;
	unsigned char bERR_Slic_A_C0_Ofs:1;		
	unsigned char bERR_Slic_Beta_Ofs:1;		
	unsigned char bERR_ROPN_IDC:1;
	unsigned char bERR_MMCO:1;		
}halH264_DEC_SH_ERR_S;

typedef PACKED_STRUCT halH264_DEC_SH_ERR_CONF_S
{
	unsigned char bERR_End_En:1;
	unsigned char bERR_Slice_Type_En:1;
	unsigned char bERR_PPS_ID_En:1;
	unsigned char bERR_IDR_PIC_ID_En:1;
	unsigned char bERR_NUM_Ref_IDC_En:1;		
	unsigned char bERR_DBF_IDC_En:1;
	unsigned char bERR_Slic_A_C0_Ofs_En:1;		
	unsigned char bERR_Slic_Beta_Ofs_En:1;		
	unsigned char bERR_ROPN_IDC_En:1;
	unsigned char bERR_MMCO_En:1;		
}halH264_DEC_SH_ERR_CONF_S;

typedef PACKED_STRUCT halH264_DEC_SD_ERR_S
{
	unsigned char bERR_1ST_MB_In_Slice:1;
	unsigned char bERR_SD_End:1;
	unsigned char bERR_SD_MB_Type:1;
	unsigned char bERR_SD_CBP:1;
	unsigned char bERR_SD_MBP_REF_IDX:1;
	unsigned char bERR_MBP_ICPM:1;
	unsigned char bERR_SMB_REF_Idx:1;
	unsigned char bERR_ZSL_It_Run:1;
	unsigned char bERR_CV_Coeft:1;
	unsigned char bERR_CV_LV_Prefix:1;
	unsigned char bERR_CV_Tzeros:1;
	unsigned char bERR_CV_Run_Before:1;
}halH264_DEC_SD_ERR_S;

typedef PACKED_STRUCT halH264_DEC_SD_ERR_CONF_S
{
	unsigned char bERR_1ST_MB_In_Slice_En:1;
	unsigned char bERR_SD_End_En:1;
	unsigned char bERR_SD_MB_Type_En:1;
	unsigned char bERR_SD_CBP_En:1;
	unsigned char bERR_SD_MBP_REF_IDX_En:1;
	unsigned char bERR_MBP_ICPM_En:1;
	unsigned char bERR_SMB_REF_Idx_En:1;
	unsigned char bERR_ZSL_It_Run_En:1;
	unsigned char bERR_CV_Coeft_En:1;
	unsigned char bERR_CV_LV_Prefix_En:1;
	unsigned char bERR_CV_Tzeros_En:1;
	unsigned char bERR_CV_Run_Before_En:1;
}halH264_DEC_SD_ERR_CONF_S;

typedef PACKED_STRUCT halH264_DEC_HP_END_S
{
	unsigned char bSPS_End:1;
	unsigned char bPPS_End:1;
	unsigned char bSEI_End:1;
	unsigned char bAUD_End:1;
	unsigned char bEOSEQ_End:1;
	unsigned char bSH_End:1;
	unsigned char bEnd_Of_Stream_End:1;
	unsigned char bFiller_Data_End:1;
}halH264_DEC_HP_END_S;

typedef PACKED_STRUCT halH264_DEC_HP_END_CONF_S
{
	unsigned char bSPS_End_En:1;
	unsigned char bPPS_End_En:1;
	unsigned char bSEI_End_En:1;
	unsigned char bAUD_End_En:1;
	unsigned char bEOSEQ_End_En:1;
	unsigned char bSH_End_En:1;
	unsigned char bEnd_Of_Stream_End_En:1;
	unsigned char bFiller_Data_End_En:1;	
}halH264_DEC_HP_END_CONF_S;
//========================H264 DEC Function Export ==============================
unsigned char halH264DEC_GetCurStrem(void);
unsigned char halH264DEC_GetMemWork(void);
unsigned short halH264DEC_Get_Head_Size(void);
void halH264DEC_Get_SEI_PayLoad(unsigned short *pPayLoadType, unsigned short *pPayLoadSize);
void halH264DEC_Reset(void);
void halH264DEC_StreamSel(void);
void halH264DEC_GetStat(halH264_DEC_STAT_S *halh264_dec_stat_s);
void halH264DEC_Get_Head(unsigned short *pBuf);
void halH264DEC_Display_Conf(halH264_DEC_DISP_S *halh264_dec_disp_s);
void halH264DEC_Start(halH264_DEC_START_S *halh264_dec_start_s);
void halH264DEC_Config_Misc(halH264_DEC_MISC_S *halh264_dec_misc_s);
void halH264DEC_Config_Ref_IDX(halH264_DEC_REF_IDX_S *halh264_dec_ref_idx_s);
void halH264DEC_Config_SEI_Raw_Output(halH264_DEC_SEI_RAW_OUTPUT_S *halh264_dec_sei_raw_output_s);
void halH264DEC_Set_REO_Addr(unsigned int nF0_Addr, unsigned int nF1_Addr);
void halH264DEC_Set_Work_Mem_Addr(halH264_DEC_BASE_ADDR_S *halh264_dec_base_addr_s);
void halH264DEC_INT_Conf(halH264_DEC_INT_CONF_S *halh264_dec_int_conf_s);
void halH264DEC_Get_INT(halH264_DEC_INT_S *halh264_dec_int_s);
void halH264DEC_NAL_ERR_Conf(halH264_DEC_NAL_ERR_CONF_S *halh264_dec_nal_err_conf_s);
void halH264DEC_Get_NAL_ERR(halH264_DEC_NAL_ERR_S *halh264_dec_nal_err_s);
void halH264DEC_SPS_ERR_Conf(halH264_DEC_SPS_ERR_CONF_S *halh264_dec_sps_err_conf_s);
void halH264DEC_Get_SPS_ERR(halH264_DEC_SPS_ERR_S *halh264_dec_sps_err_s);
void halH264DEC_PPS_ERR_Conf(halH264_DEC_PPS_ERR_CONF_ERR_S *halh264_dec_pps_err_conf_s);
void halH264DEC_Get_PPS_ERR(halH264_DEC_PPS_ERR_S *halh264_dec_pps_err_s);
void halH264DEC_SH_ERR_Conf(halH264_DEC_SH_ERR_CONF_S *halh264_dec_sh_err_conf_s);
void halH264DEC_Get_SH_ERR(halH264_DEC_SH_ERR_S *halh264_dec_sh_err_s);
void halH264DEC_SD_ERR_Conf(halH264_DEC_SD_ERR_CONF_S *halh264_dec_sd_err_conf_s);
void halH264DEC_Get_SD_ERR(halH264_DEC_SD_ERR_S *halh264_dec_sd_err_s);
void halH264DEC_HP_END_Conf(halH264_DEC_HP_END_CONF_S *halh264_dec_hp_end_conf_s);
void halH264DEC_Get_HP_END(halH264_DEC_HP_END_S *halh264_dec_hp_end_s);
void halH264DEC_Get_Video_Size(halH264_DEC_VIDEO_SIZE_S *halh264_dec_video_size_s);
void halH264DEC_Get_Frame_Info(halH264_DEC_FRAME_INFO_S *halh264_dec_frame_info_s);
unsigned short halH264DEC_Get_Err_MB_Num(void);

#endif



/* halRTC.c */
#if defined(HAL_INC_RTC)
unsigned int halRTCGetCounterStatus(unsigned int *pnDDDD, unsigned int *pnHH,
							unsigned int *pnMM, unsigned int *pnSS);
void halRTCSetSecAlarm(bool_T b);
bool_T halRTCSetAlarmTime(unsigned char nHH,
 						unsigned char nMM,
 						unsigned char nSS);
#else
#define halRTCGetCounterStatus(pnDDDD, pnHH, pnMM, pnSS) (0)
#define halRTCSetSecAlarm(b)
#endif

/* halDMAP.c */
#if defined(HAL_INC_DMAP)
int halDMAPGetChannelNum(void);
bool_T halDMAPIsChannelBusy(halDMAPId_E nCnId);
#else
#define halDMAPGetChannelNum() (0)
#define halDMAPIsChannelBusy(nCnId) (TRUE)
#endif










/* halMPEG4ENC.c */
#if defined(HAL_INC_MPEG4_ENC)
//========================Register Mapping ==============================
#define HAL_REG_M4E_RESET						(HAL_REG_M4E_BASE+0x00)
#define HAL_REG_M4E_START						(HAL_REG_M4E_BASE+0x04)
#define HAL_REG_M4E_SYSCTRL					(HAL_REG_M4E_BASE+0x08)
#define HAL_REG_M4E_STUFFCODE				(HAL_REG_M4E_BASE+0x0C)
#define HAL_REG_M4E_WIDTH						(HAL_REG_M4E_BASE+0x10)
#define HAL_REG_M4E_HEIGHT					(HAL_REG_M4E_BASE+0x14)
#define HAL_REG_M4E_TIMERESO				(HAL_REG_M4E_BASE+0x18)
#define HAL_REG_M4E_VOL_CTRL				(HAL_REG_M4E_BASE+0x1C)
#define HAL_REG_M4E_VOP_CTRL1				(HAL_REG_M4E_BASE+0x20)
#define HAL_REG_M4E_VOP_CTRL2				(HAL_REG_M4E_BASE+0x24)
#define HAL_REG_M4E_TIME_INC				(HAL_REG_M4E_BASE+0x28)
#define HAL_REG_M4E_FRAME_WORD_CNT	(HAL_REG_M4E_BASE+0x2C)
#define HAL_REG_M4E_RESYNC_CTRL			(HAL_REG_M4E_BASE+0x30)
#define HAL_REG_M4E_INT   					(HAL_REG_M4E_BASE+0x3C)
#define HAL_REG_M4E_SRC_Y_ADDR			(HAL_REG_M4E_BASE+0x40)
#define	HAL_REG_M4E_SRC_CB_ADDR			(HAL_REG_M4E_BASE+0x44)
#define HAL_REG_M4E_SRC_CR_ADDR			(HAL_REG_M4E_BASE+0x48)
#define HAL_REG_M4E_REC_Y0_ADDR			(HAL_REG_M4E_BASE+0x4C)
#define HAL_REG_M4E_REC_CB0_ADDR		(HAL_REG_M4E_BASE+0x50)
#define	HAL_REG_M4E_REC_CR0_ADDR		(HAL_REG_M4E_BASE+0x54)
#define HAL_REG_M4E_REC_Y1_ADDR			(HAL_REG_M4E_BASE+0x58)
#define HAL_REG_M4E_REC_CB1_ADDR		(HAL_REG_M4E_BASE+0x5C)
#define HAL_REG_M4E_REC_CR1_ADDR		(HAL_REG_M4E_BASE+0x60)
#define	HAL_REG_M4E_OUTPUT_ADDR			(HAL_REG_M4E_BASE+0x64)
// MP4 I/F
#define HAL_REG_M4IF_START					(HAL_REG_M4IF_BASE+0x00)
#define HAL_REG_M4IF_BB_INI					(HAL_REG_M4IF_BASE+0x04)
#define HAL_REG_M4IF_BB_END					(HAL_REG_M4IF_BASE+0x08)
#define HAL_REG_M4IF_FUNC						(HAL_REG_M4IF_BASE+0x0C)
#define HAL_REG_M4IF_INT						(HAL_REG_M4IF_BASE+0x10)

enum {eMPEG4ENC_STOP,eMPEG4ENC_REC};

typedef PACKED_STRUCT halMP4_ENC_CONFIGURE_S
{
	unsigned short int	nMP4_ENC_Ctrl;		//Bit 7-0==>Register 0x08, Bit 8==>1/0:32 Bit/8 Bit VOP Word Alignment																				
	unsigned char 			nStuffing_Code;
	unsigned char				bVOP_Word_End;
	unsigned short int	nVOL_CTRL;		
	unsigned short int	nVOP_CTRL1;	
	unsigned short int	nVOP_CTRL2;		
	unsigned int				nY0_SRC_Addr;
	unsigned int				nCb0_SRC_Addr;
	unsigned int				nCr0_SRC_Addr;
	unsigned int				nY1_SRC_Addr;
	unsigned int				nCb1_SRC_Addr;
	unsigned int				nCr1_SRC_Addr;	
	unsigned int				nY0_REC_Addr;
	unsigned int				nCb0_REC_Addr;
	unsigned int				nCr0_REC_Addr;	
	unsigned int				nY1_REC_Addr;
	unsigned int				nCb1_REC_Addr;
	unsigned int				nCr1_REC_Addr;					
}halMP4_ENC_CONFIGURE_S;
typedef PACKED_STRUCT halMP4_ENC_IMG_INFO_S
{
	unsigned short int	nWidth;
	unsigned short int  nHeight;
	unsigned short int	nTime_Inc_Reso;
}halMP4_ENC_IMG_INFO_S;
typedef PACKED_STRUCT halMP4_ENC_SYNC_S
{
	unsigned char nReSync_Ctrl;		//Bit 0==>1/0:Enable/Disable ReSync, Bit 1==>1/0:Enable/Disable Header Extension Code
	unsigned short int 	nMax_VP_Word_Cnt;
}halMP4_ENC_SYNC_S;
typedef PACKED_STRUCT halMP4_ENC_TIME_INC_S
{
	unsigned char 			bMod_Time_Base;				//1:One Sec Elasped Since Previous Encoded Frame
	unsigned short int 	nTime_Inc;
}halMP4_ENC_TIME_INC_S;
typedef PACKED_STRUCT halMP4_ENC_VOP_S
{
	unsigned char nVOP_Quant:5;
	unsigned char bVOP_Coded:1;
	unsigned char nVOP_Type:1;
}halMP4_ENC_VOP_S;
//MP4 I/F
typedef PACKED_STRUCT halMP4_IF_CONFIGURE_S
{
	unsigned int	nBB_INI_Addr;
	unsigned int	nBB_END_Addr;
	unsigned char nByte_Order:1;
	unsigned char bClk_Inverse:1;
}halMP4_IF_CONFIGURE_S;
//========================MP4 ENC Function Export ==============================
void halMPEG4ENCReset(void);
void halMPEGENC_Proc(unsigned char nFunc,unsigned char *bM4E_Done,unsigned char nBuf_Idx);
unsigned short int halMPEG4ENCGetFrameWordCnt(void);
void halMPEG4ENCSetTimeInc(halMP4_ENC_TIME_INC_S *mp4_enc_time_inc_s);
void halMPEG4ENCSetImgInfo(halMP4_ENC_IMG_INFO_S *mp4_enc_img_info_s);
void halMPEG4ENCSetReSync(struct halMP4_ENC_SYNC_S *mp4_enc_sync_s);
void halMPEG4ENCConfigure(struct halMP4_ENC_CONFIGURE_S *mp4_enc_configure_s);
void halMPEG4ENCSetVOP(halMP4_ENC_VOP_S *mp4_enc_vop_s);
void halMPEG4ENCOutputAddress(int nAddr);
void halMPEG4ENCSRCAddress(int nYAdd,int nCbAdd,int nCrAdd);
//MP4 I/F
void halMP4IFConfigure(halMP4_IF_CONFIGURE_S *mp4_if_configure_s);
void halMP4IFStart(void);
unsigned char halMP4IFGetInt(void);
#endif



/* halGE.c */
#if defined(HAL_INC_GE)
#define HAL_REG_GE_BLTWH						(HAL_REG_GE_BASE+0x00)
#define HAL_REG_GE_EN								(HAL_REG_GE_BASE+0x04)
#define HAL_REG_GE_CTRL							(HAL_REG_GE_BASE+0x08)
#define HAL_REG_GE_IFORM						(HAL_REG_GE_BASE+0x0C)
#define HAL_REG_GE_BLTCTRL					(HAL_REG_GE_BASE+0x10)
#define HAL_REG_GE_SIADDR						(HAL_REG_GE_BASE+0x14)
#define HAL_REG_GE_DIADDR						(HAL_REG_GE_BASE+0x18)
#define HAL_REG_GE_DOADDR						(HAL_REG_GE_BASE+0x1C)
#define HAL_REG_GE_PTADDR						(HAL_REG_GE_BASE+0x20)
#define HAL_REG_GE_CIWIDTH					(HAL_REG_GE_BASE+0x24)
#define HAL_REG_GE_HCPOS						(HAL_REG_GE_BASE+0x2C)
#define HAL_REG_GE_RFCOLOR					(HAL_REG_GE_BASE+0x30)
#define HAL_REG_GE_PFGCOLOR					(HAL_REG_GE_BASE+0x38)
#define HAL_REG_GE_PBGCOLOR					(HAL_REG_GE_BASE+0x3C)
#define HAL_REG_GE_HCCOLOR0					(HAL_REG_GE_BASE+0x40)
#define HAL_REG_GE_HCCOLOR1					(HAL_REG_GE_BASE+0x44)
#define HAL_REG_GE_HCCOLOR2					(HAL_REG_GE_BASE+0x48)
#define HAL_REG_GE_TMINCOLOR				(HAL_REG_GE_BASE+0x4C)
#define HAL_REG_GE_TMAXCOLOR				(HAL_REG_GE_BASE+0x50)
#define HAL_REG_GE_LDCOLOR					(HAL_REG_GE_BASE+0x50)
#define HAL_REG_GE_PAT1							(HAL_REG_GE_BASE+0x54)
#define HAL_REG_GE_PAT2							(HAL_REG_GE_BASE+0x58)
#define HAL_REG_GE_LDPARAM					(HAL_REG_GE_BASE+0x5C)
#define HAL_REG_GE_AVALUE						(HAL_REG_GE_BASE+0x60)
#define HAL_REG_GE_CQSTR						(HAL_REG_GE_BASE+0x80)
#define HAL_REG_GE_CQEN							(HAL_REG_GE_BASE+0x84)
#define HAL_REG_GE_CQCTRL						(HAL_REG_GE_BASE+0x88)
#define HAL_REG_GE_CQSTS						(HAL_REG_GE_BASE+0x8C)
#define HAL_REG_GE_INTCTRL					(HAL_REG_GE_BASE+0x90)
#define HAL_REG_GE_INTSTS						(HAL_REG_GE_BASE+0x94)
#define HAL_REG_GE_HCTABLE					(HAL_REG_GE_BASE+0x300)
#define HAL_REG_GE_PALETTE					(HAL_REG_GE_BASE+0x400)

enum{eGE_CQ_START,e_GE_CQ_SEND,e_GE_CQ_STOP};
enum{eGE_CQ_ACK,eGE_CQ_CMD_ERR,e_GE_CQ_EMPTY,eGE_CQ_FINISH};

#define dCQ_LEN					32
#define dCQ_WRITE_TH		dCQ_LEN/2

typedef PACKED_STRUCT halGE_BLT_SIZE_S
{
	unsigned short int 	nHeight;
	unsigned short int 	nWidth;	
	unsigned char 		 	nDirection; //00:Upper-Left, 01:Upper-Right, 10:Lower-Left, 11: Lower-Right
}halGE_BLT_SIZE_S;
typedef PACKED_STRUCT halGE_INT_S
{
	unsigned char bCQFIC_En:1;
	unsigned char bCQEIC_En:1;
	unsigned char bGAIC_En:1;
	unsigned char bCQFIC_Status:1;
	unsigned char bCQEIC_Status:1;
	unsigned char bGAIC_Status:1;		
}halGE_INT_S;
typedef PACKED_STRUCT halGE_IMG_ADD_S
{
	unsigned int 	nSRAddr;
	unsigned int 	nDesInAddr;
	unsigned int 	nDesOutAddr;
}halGE_IMG_ADD_S;
typedef PACKED_STRUCT halGE_COLOR_S
{
	unsigned char nR;
	unsigned char nG;
	unsigned char nB;
	unsigned char nA;	
}halGE_COLOR_S;
typedef PACKED_STRUCT halGE_HC_CONFIGURE_S
{
	unsigned char bSingleMode;
	unsigned int 	*nHCTableAddr;
	halGE_COLOR_S	color0_s;
	halGE_COLOR_S	color1_s;
	halGE_COLOR_S	color2_s;
}halGE_HC_CONFIGURE_S;
typedef PACKED_STRUCT halGE_Trans_S
{
	halGE_COLOR_S max_color_s;
	halGE_COLOR_S min_color_s;	
}halGE_Trans_S;
typedef PACKED_STRUCT halGE_IMG_FORMAT_S
{
	unsigned char bARGBOutSwap:1;
	unsigned char bARGBInSwap:1;
	unsigned char bBigEndian:1;
	unsigned char nSourceIMGBPP:3;	//0:32 BBP,1:24 BBP,2:8 BBP
}halGE_IMG_FORMAT_S;
typedef PACKED_STRUCT halGE_RECTANGLE_S
{
	halGE_COLOR_S			color_s;
	halGE_BLT_SIZE_S	size_s;	
	halGE_IMG_ADD_S		img_add_s;
}halGE_RECTANGLE_S;
typedef PACKED_STRUCT halGE_PATTERN_S
{
	halGE_COLOR_S 		fg_color_s;
	halGE_COLOR_S			bg_color_s;
	halGE_BLT_SIZE_S	size_s;
	halGE_IMG_ADD_S		img_add_s;	
	unsigned int			nPatternData1;
	unsigned int 			nPatternData2;
}halGE_PATTERN_S;
typedef PACKED_STRUCT halGE_LINE_S
{
	halGE_COLOR_S				color_s;
	halGE_BLT_SIZE_S		size_s;
	halGE_IMG_ADD_S			img_add_s;
	unsigned char				nDirection;
	unsigned short int 	dY;
	unsigned short int 	dX;
}halGE_LINE_S;
typedef PACKED_STRUCT halGE_MISC_S
{
	unsigned char nRDIC:2;
	unsigned char nRSIC:2;
	unsigned char nGEPD:2;
	unsigned char	nAFS:3;
	unsigned char nGAVS:8;
}halGE_MISC_S;
typedef PACKED_STRUCT halGE_ROP_S
{
	halGE_BLT_SIZE_S		size_s;
	halGE_IMG_ADD_S			img_add_s;	
	unsigned char 			nROP;
}halGE_ROP_S;
typedef PACKED_STRUCT halGE_CQ_CONFIGURE_S
{
	unsigned char CQRS:2;
	unsigned char CQBT:1;
	unsigned char CQBC:8;
	unsigned char CQRA:8;
	unsigned int	*pCQAddr;	
	unsigned int	nRP;	
	unsigned int	nCount;
}halGE_CQ_CONFIGURE_S;
typedef PACKED_STRUCT halGE_CQ_STATUS_S
{
	unsigned char bCQFull:1;
	unsigned char bCQEmpty:1;
	unsigned char CQWP:8;
	unsigned char CQRP:8;
}halGE_CQ_STATUS_S;
//========================GE Function Export ==============================
void halGESetIMGAddr(halGE_IMG_ADD_S *ge_img_add_s);
void halGESetIMGFormat(halGE_IMG_FORMAT_S *ge_img_format_s);
void halGESetPalette(unsigned int *nAddr,unsigned char bInter);
void halGEReset(void);
void halGESetHWCursor(halGE_HC_CONFIGURE_S *ge_hc_configure_s);
void halGESetHWCursorEn(unsigned short int X, unsigned short int Y,unsigned char bEn);
void halGESetClipImgWidth(unsigned short int nClipIMGWidth, unsigned char bClipIMGSr, unsigned char bClipIMGDes);
void halGERectangleFill(halGE_RECTANGLE_S *ge_rectangle_s,unsigned char bEn);
void halGESetTrans(halGE_Trans_S *ge_trans_s,unsigned char bSingleColor,unsigned char bTransObj);
void halGEPatternFill(halGE_PATTERN_S *ge_pattern_s,unsigned char bEn);
void halGELineDraw(halGE_LINE_S *ge_line_s);
void halGESetMisc(halGE_MISC_S *ge_misc_s);
void halGESetROP(halGE_ROP_S *ge_rop_s);
unsigned char halGECQFunc(halGE_CQ_CONFIGURE_S *ge_cq_configure_s,unsigned char nFunc);
void halGEEn(unsigned char bEn);
void halGESetInt(halGE_INT_S *ge_int_s);
void halGEGetInt(halGE_INT_S *ge_int_s);
void halGESetBLTHW(halGE_BLT_SIZE_S *ge_blt_size_s);
#endif


#if defined(HAL_INC_LCM)
void halLCMWriteOneData(unsigned char nRow, unsigned char nCol,char cData, unsigned char bEnable);
char halLCMGetOneData(unsigned char nRow, unsigned char nCol);
void halLCMWriteData(unsigned char nRow, unsigned char nCol,char *pData, unsigned char nDataCnt);
void halLCMClearOneRow(unsigned char nRow);
#endif


/* halCF.c */
#if defined(HAL_INC_CF)

bool_T halCFExist(void);

bool_T halCFInstall(unsigned int nPCLK);
bool_T halCFUninstall(void);

void halCFMaskIRQnIntr(halCFIRQId_E irqn, halCFFlag_E flag);
void halCFUnmaskIRQnIntr(halCFIRQId_E irqn, halCFFlag_E flag);

int halCFGetMediaSize(void);

bool_T halCFReadSector(unsigned int nSect, unsigned char *pBuf);
bool_T halCFReadNSectors(unsigned int nStartSect, int nSectCnt, unsigned char *pBuf);

bool_T halCFWriteSector(unsigned int nSect, unsigned char *pBuf);
bool_T halCFWriteNSectors(unsigned int nStartSect, int nSectCnt, unsigned char *pBuf);

bool_T halCFWriteMediaSectorNByte(
		unsigned int nSector,
		unsigned int nOffset,
		unsigned int nSize,
		unsigned char *pBuf);

bool_T halCFReadMediaSectorNByte(
		unsigned int nSector,
		unsigned int nOffset,
		unsigned int nSize,
		unsigned char *pBuf);

#else

#define halCFExist() (FALSE)
#define halCFInstall(nPCLK) (FALSE)
#define halCFUninstall() (FALSE)
#define halCFMaskIRQnIntr(irqn, flag) 
#define halCFUnmaskIRQnIntr(irqn, flag) 

#define halCFGetMediaSize() (0)

#define halCFReadSector(nSect, pBuf) (FALSE)
#define halCFReadNSectors(nStartSect, nSectCnt, pBuf) (FALSE)

#define halCFWriteSector(nSect, pBuf) (FALSE)
#define halCFWriteNSectors(nStartSect, nSectCnt, pBuf) (FALSE)

#define halCFWriteMediaSectorNByte(nSector, nOffset, nSize, pBuf) (FALSE)

#define halCFReadMediaSectorNByte(nSector, nOffset, nSize, pBuf) (FALSE)
#endif




/* halKBMS.c */
#if defined(HAL_INC_KBMS0) || defined(HAL_INC_KBMS1)
#else
#endif


/* halNFC.c */
void halNFCGetChipParam(unsigned int *pnNumTotalBlocks,
					 unsigned int *pnNumPagesPerBlock,
					 unsigned int *pnNumSectsPerPage);
void init_flash(void);
void read_ID(void);
void erase_block(int _chipID, int nBlockNum);
void read_phy_sector(unsigned int _start_sector,
					unsigned int _data_length,
					unsigned int *pBuf,
					int _chipID);
void write_multi_phy_page(unsigned int _write_start_sector,
						unsigned int _data_length,
						unsigned int *pBuf,
						int _chipID);




EXTERN_C_END

#endif /* _hal_h_ */

