///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <config.h>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define AST_HDMIRX //This flag is used to identify those code sections modified by aspeed. CONFIG_AST1500_CAT6023 might be used instead.

#if defined(AST_HDMIRX)
#define ENABLE_DETECT_VSIF_PKT	TRUE
#define ENABLE_DETECT_DRM_PKT	TRUE
#define _MCU_8051_EVB_		FALSE
#define _ENABLE_EDID_RAM_	FALSE
#define DYNAMIC_HDCP_ENABLE_DISABLE	TRUE
#define AST_IT6805_INTR 1
#define AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ 1
#if (AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ == 1)
#define AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ_AVI_INFO_FIXUP 1
#endif
#endif

#ifdef _iTE6805_
#pragma message("defined _iTE6805_ config")
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


/*************** Important Note ****************/
// 1. PORT1 HPD is GPIO control, please find iTE6805_Set_HPD_Ctrl function and fill the GPIO setting to it
//    When 5V on, 6805 interrupt occur will set HPD high, you don't need to program HPD by your upper layer code

// 2. In multi thread system, If you need to get 6805 status like AVIInfoframe, 5V status,
//    please using extern _iTE6805_DATA	iTE6805_DATA; for getting related variable
//    because if you using function and it call chgbank, will occur register write error
//    ex. chgbank(2); ...here. multi-thread it may change to another bank by other thread... hdmirxwr(0x20, 0x01); // will write wrong bank
//    and this is easy to find, you could printf register from bank0 to bank 7 every 10s, and compare it is right or not.

// 3. If you need to change port, call void iTE6805_Port_Select(iTE_u8 ucPortSel);
// 4. If you need to change EDID, call void iTE6805_Port_SetEDID(iTE_u8 SET_PORT, iTE_u8 EDID_INDEX);
// 5. you could adjust #define TIMEOUT_SCDT_CHECK_COUNT	MS_TimeOut(5) in iTE6805_SYS.c because every system polling interval is different
// 6. you don't need to call iTE6805_Init_fsm(); for init 6805, in new verion (v1.20 later) will auto init in void iTE6805_FSM();
//    only you need is put void iTE6805_FSM() to your system.
// 7. for init(and init only) main port setting you could find iTE6805_Port_Select_Body in iTE6805_Init_fsm as example
/*************** Important Note ****************/



//////////////////////////////////////////////////////////////////////////////////////////
// EVB Option
//////////////////////////////////////////////////////////////////////////////////////////

#ifndef _MCU_8051_EVB_
#define _MCU_8051_EVB_ TRUE // if not ITE EVB, do not set this.
#endif
// DEMO code
// if EDID_WP0 = 0, when 4k then output dowscale to 1080p
// if EDID_WP0 = 1, if EDID_WP1 = 0 ,when 4k the output is Odd Even Mode
// if EDID_WP0 = 1, if EDID_WP1 = 1 ,when 4k the output is LeftRight Mode
// EVB_4096_DOWNSCALE_TO_2048_OR_1920_CONTROL_BY_PIN - for EVB DownScale By PIN P0^0
// EVB_AUTO_DETECT_PORT_BY_PIN - for P3^0 pin auto detect port


//////////////////////////////////////////////////////////////////////////////////////////
// Config For Set to 68051/68052
//////////////////////////////////////////////////////////////////////////////////////////
// IT68051: iTE68051=TRUE	iTE68052=FALSE
// IT68052: iTE68051=FALSE	iTE68052=TRUE
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef iTE68051
	#define iTE68051 TRUE	// iTE68051
#else
	#if (_MCU_8051_EVB_==TRUE)
	#pragma message("iTE68051 is pre-defined.")
	#endif
#endif

#ifndef iTE68052
	#define iTE68052 FALSE	// iTE68052
#else
	#if (_MCU_8051_EVB_==TRUE)
	#pragma message("iTE68052 is pre-defined.")
	#endif
#endif

#ifndef iTE6807
	#define iTE6807 FALSE
#else
	#if (_MCU_8051_EVB_==TRUE)
	#pragma message("iTE6807 is pre-defined.")
	#endif
#endif
#define MAIN_PORT PORT0 // please refer EVB_AUTO_DETECT_PORT_BY_PIN for set main port

#if defined(AST_HDMIRX)
#else
#if iTE68051
	#pragma message("USE IT68051") ;
#elif iTE68052
	#pragma message("USE IT68052") ;
#elif iTE6807
	#pragma message("USE IT6807") ;
#else
	#error not defined
#endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// 68051 68052 6807 Common Config
//////////////////////////////////////////////////////////////////////////////////////////

// 68051 68052 4K mode setting/ 6807 4K mode (MODE_DownScale or MODE_EvenOdd (default))
#ifndef iTE6805_4K60_Mode
#define iTE6805_4K60_Mode MODE_LeftRight // MODE_EvenOdd or MODE_LeftRight or MODE_DownScale or MODE_EvenOdd_Plus_DownScale // If define DEMO, this define will be ignored, iTE6805_4K60_Mode setting will by WP1 pin setting
#endif

// If downscaling, 4K30/4K25/4K24 also setting downscale or not
#define ENABLE_4K_MODE_ALL_DownScaling_1080p	TRUE	// this config is define for TI, for all 4k downscaling
#define ENABLE_4K_MODE_4096x2048_DownScale_To_1920x1080p	TRUE	// for 4096x2160 down to 1920x1080p, original downscaling is 2048x1080p

// 4K downscale parameter
#define DownScale_Filter 0x01 			// DownScale Filter				: DownScale Quality setting - Default :0x01 ---- 0x01 : 1:6:1 , 0x10 : 1:2:1 , 0x00 : bypass
#define DownScale_YCbCr_Color_Range 1	// DownScale YCbCr Color Range	: 1 : Full Range  0 : Limit Range

// 4K Left-Right mode parameter
#ifndef Enable_LR_Overlap
#define Enable_LR_Overlap 0	// 1 for enable overlap when LR mode only
#endif

// output color setting
#define COLOR_DEPTH_BY_GCPINFOFRAME	TRUE	// If define this config output color depth will set by GCP infoframe
#define Output_Color_Depth 12				// only set to 8 : 8bit , 10 : 10bit, 12 : 12bit


// Dual-pixel condition
#if 0
#define DUAL_PIXEL_MODE_PCLK_CONDITION 160000	// default PCLK > 3G will output dual pixel mode (two port output),
#else
#define DUAL_PIXEL_MODE_PCLK_CONDITION 320000	// default PCLK > 3G will output dual pixel mode (two port output),
#endif
												// this define is for if wanna dual port output change to 1.6G upper , fix the default value 320000 to 160000


// Vender Specific InformFrame Detect (usually for detect 3D info of video)
#ifndef ENABLE_DETECT_VSIF_PKT
#define ENABLE_DETECT_VSIF_PKT FALSE 	// please find iTE6805_Detect3DFrame() function for setting call back function
#endif



// Dynamic Range InformFrame Detect
#ifndef ENABLE_DETECT_DRM_PKT
#define ENABLE_DETECT_DRM_PKT FALSE   // for detect HDR pkt by funtion iTE6805_DRM_Detect();
#endif                                // using void iTE6805_CHECK_DRMInfoframe(iTE_u8 *pFlag_HAVE_DRM_PKT" , iTE_u8  ---> "*pFlag_NEW_DRM_PKT_RECEIVE<---) for detect new DRM PKT
                                      // using void iTE6805_CHECK_DRMInfoframe(iTE_u8 ---> *pFlag_HAVE_DRM_PKT" <--- , iTE_u8 "*pFlag_NEW_DRM_PKT_RECEIVE) for receive MDR PKT or not (change by iTE68051, customer using it for check)
                                      // !!!! and every platform need to adjust MAX_NO_DRM_PKT_RECEIVE_COUNT for judge have DRM PKT or not because every while fsm loop interval is different!!!!

// Dynamic set HDCP Enable/Disable
#ifndef DYNAMIC_HDCP_ENABLE_DISABLE
#define DYNAMIC_HDCP_ENABLE_DISABLE FALSE  // this define for dynamic HDCP enable/disable by using variable iTE6805_DATA.STATE_HDCP = HDCP_ENABLE/HDCP_DISABLE
#endif                                    // Customer can change HDCP state in upper code level, default HDCP is enable


#ifndef DYNAMIC_HDCP_ENABLE_DISABLE_INIT_VALUE               // If DYNAMIC_HDCP_ENABLE_DISABLE=TRUE, then you may need to set this one for default HDCP is Enable or Disable.
#define DYNAMIC_HDCP_ENABLE_DISABLE_INIT_VALUE  HDCP_ENABLE // HDCP_ENABLE or HDCP_DISABLE.
#endif


// Power Saving Setting
#define ENABLE_6805_POWER_SAVING_MODE FALSE		// 20181220 change to FALSE cause customer always setting to FALSE and test ok (EVB board = TRUE still ok but not test that much)
#define ENABLE_6805_POWER_DETECT_ONLY_TMDS_CLOCK_POWER_ON FALSE		// WARNING :  USING THIS DEFINE CTS WILL NOT PASS BUT POWER MORE SAVING
																																	// If 5v detect -> ONLY TMDS Clock differential pair power on ,
																																	// If Clock detect -> ALL power on (TMDS RGB Channel + Clock and the others)

// When CSC Bypass , 420 need to Convert to RGB or not
#ifndef ENABLE_YUV420_CONVERT_TO_RGB_WHEN_CSC_BYPASS
#define ENABLE_YUV420_CONVERT_TO_RGB_WHEN_CSC_BYPASS FALSE	// When eVidOutConfig setting to BYPASS_CSC
#endif                                                      // If TRUE = when YUV420 convert to RGB444 Full Range
                                                            // If FALSE = just BYPASS 420
                                                            // depend on chip behind 6805 can accept 420 or not, IT chip default need to set to TRUE.

// this config enable for RGB limit in to TTL RGB full range out, do not enable if you really needed it
#ifndef ENABLE_RGB_LIMIT_TO_RGB_FULL_RANGE
#define ENABLE_RGB_LIMIT_TO_RGB_FULL_RANGE FALSE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// iTE6805/6807 Config/output CSC setting
//////////////////////////////////////////////////////////////////////////////////////////

// Any Color Space input default setting to output RGB will be Full Ranged.

// 68051 TTL config/68052 output CSC setting/6807 CSC setting
#ifndef eVidOutConfig
#define eVidOutConfig eTTL_SepSync_FullBusDDR_BYPASS_CSC
#endif
//eTTL_SepSync_FullBusSDR_RGB444,		// SDR to 66121 can't output 1080p mode
//eTTL_SepSync_FullBusSDR_YUV444,
//eTTL_SepSync_FullBusSDR_YUV422,
//eTTL_SepSync_FullBusSDR_BYPASS_CSC,
//eTTL_SepSync_FullBusDDR_RGB444,
//eTTL_SepSync_FullBusDDR_YUV444,
//eTTL_SepSync_FullBusDDR_YUV422,
//eTTL_SepSync_FullBusDDR_BYPASS_CSC,
//eTTL_SepSync_HalfBusDDR_RGB444,		//(6028 no half bus mode and SDR mode can't output 4k60 because LVDS HW limit)
//eTTL_SepSync_HalfBusDDR_YUV444,
//eTTL_SepSync_HalfBusDDR_BYPASS_CSC,
//eTTL_EmbSync_FullBusSDR_YUV422,
//eTTL_EmbSync_FullBusDDR_YUV422,
//eTTL_EmbSync_FullBusDDR_BYPASS_CSC,
//eTTL_BTA1004_SDR,
//eTTL_BTA1004_DDR

// Clock delay setting
#if defined(AST_HDMIRX)
#define Clock_Delay	0	// arg from 0 to 7
#else
#define Clock_Delay  3	// arg from 0 to 7	- DDR = 2, SDR = 3 for EVB
#endif
#define Clock_Invert 0	// arg 0/1			- DDR = 1, SDR = 0 for EVB

#define TMDS_162M_LESS_SDR_BIGGER_DDR FALSE
// If TMDS < 162 , setting to (SDR)eTTL_SDR
// If TMDS >= 162 , setting to (DDR)eTTL_DDR
// If set this will ignore eVidOutConfig's SDR DDR setting, default undefined


//////////////////////////////////////////////////////////////////////////////////////////
// 68052 config
//////////////////////////////////////////////////////////////////////////////////////////

// 68052 LVDS Config
#define LVDSSwap	FALSE

#ifndef LVDSMode
#define LVDSMode	JEIDA		// JEIDA or VESA
#endif

#ifndef LVDSColDep
#define LVDSColDep	LVDS8BIT	// LVDS6BIT LVDS8BIT LVDS10BIT
#endif

#ifndef LVDSSSC
#define LVDSSSC		DISABLE		// ENABLE or DISABLE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// 6807 VBO config
//////////////////////////////////////////////////////////////////////////////////////////

// 6807
#define VBO_SwapSel 0

// for reference output Lan Count, Byte Mode
// please refer variable iTE6805_DATA.VBO_LaneCount, iTE6805_DATA.VBO_ByteMode
// The variable is auto setting by input PCLK
// Downscaling (1080p) = 2 lan , > 3.2G = 8Lan , > 1.6 G and < 3.2 G = 4Lan, > 1G and < 1.6 G = 2Lan, <1G = 1Lan
// Byte Mode depend on input color depth
// please using iTE6805_4K60_Mode variable for 6807 setting downscale mode or not
// and maybe you also need  ENABLE_4K_MODE_ALL_DownScaling_1080p, ENABLE_4K_MODE_4096x2048_DownScale_To_1920x1080p define

// One Section Mode or Two Section Mode
// default iTE6807_EnableTwoSectionMode = 0 = ONE section mode, please reference datasheet for more detail (in 68051 is also called odd-even mode)
// default iTE6807_EnableTwoSectionMode = 1 = TWO section mode, please reference datasheet for more detail (in 68051 is also called left-right mode)
#define iTE6807_EnableTwoSectionMode 0

#define iTE6807_Force_ByteMode 0
#if (iTE6807_Force_ByteMode==1)
#define Force_ByteMode 4	// 3 = 3B, 4 = 4B, 5 = 5B
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// EQ Config
//////////////////////////////////////////////////////////////////////////////////////////

#define _ENABLE_EXTERN_EQ_CTRL_ FALSE	// If TRUE, using iTE6805_Set_EQ_LEVEL(iTE_u8 PORT, EQ_LEVEL EQ) for Customer control EQ level
										// ex. iTE6805_Set_EQ_LEVEL(PORT0, EQ_LEVEL_2);
										// ex. iTE6805_Set_EQ_LEVEL(PORT1, EQ_AUTO);

#if (_ENABLE_EXTERN_EQ_CTRL_==TRUE)
	#define _ENABLE_AUTO_EQ_ TRUE		// Must = TRUE for control EQ level
	#define EQ_INIT_VALUE EQ_LEVEL_3	// EQ Default Value EQ_LEVEL_3 = 0x9F , EQ can be 0x80 &{ 0x7F, 0x7E, 0x3F, 0x3E, 0x1F, 0x1E, 0x0F, 0x0E, 0x07, 0x06, 0x03, 0x02, 0x01, 0x00 };
#endif

#ifndef _ENABLE_AUTO_EQ_
#define _ENABLE_AUTO_EQ_ TRUE	// TRUE or FALSE
#endif

#ifndef EQ_INIT_VALUE
#define EQ_INIT_VALUE 0x9F		// EQ Default Value , EQ can be 0x80 &{ 0x7F, 0x7E, 0x3F, 0x3E, 0x1F, 0x1E, 0x0F, 0x0E, 0x07, 0x06, 0x03, 0x02, 0x01, 0x00 };
#endif

//#define EQ_KURO_TEST			// for debug EQ

//////////////////////////////////////////////////////////////////////////////////////////
// HDCP Repeater
//////////////////////////////////////////////////////////////////////////////////////////

// enable 6805 as repeater or not in
#ifndef _ENABLE_6805_AS_HDCP_REPEATER_
#define _ENABLE_6805_AS_HDCP_REPEATER_	FALSE
#endif

// enable 6805 code for repeater, setting to FALSE when you never use repeater function for saving code size
#ifndef _ENABLE_6805_AS_HDCP_REPEATER_CODE_
#define _ENABLE_6805_AS_HDCP_REPEATER_CODE_	FALSE
#endif

// this define for chip(a)(HDMI->6805->'a chip'->HDMI) after 6805,
// if that 'a chip' HDMI HPD-unplg, 6805 pretend to be non-repeater device
// (still could get input from HDMI, if it is repeater, it can't.)
#ifndef _ENABLE_6805_BE_NON_HDCP_REPEATER_WHEN_HDMI_UNPLUG_
#define _ENABLE_6805_BE_NON_HDCP_REPEATER_WHEN_HDMI_UNPLUG_	FALSE
#endif

// for HDCP1.4 compliance test, need to disable HDCP2.0 or test fail (hdcp bank reserve reg should be 0)
#ifndef CTS_HDCP14
#define CTS_HDCP14 FALSE
#endif

// for enable 6615 call back function or not
#ifndef _Enable_6615_CB_
#define _Enable_6615_CB_ FALSE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// EDID Setting
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENABLE_EDID_RAM_
#define _ENABLE_EDID_RAM_ TRUE
#endif

#define EDID_COUNT 1        // Total EDID count, please find Customer_EDID for fill EDID content

// void iTE6805_Port_SetEDID(iTE_u8 SET_PORT, iTE_u8 EDID_INDEX);
// ex. iTE6805_Port_SetEDID(PORT0, 1); = set PORT0 EDID index = 1 = Customer_EDID[1]
// ex. iTE6805_Port_SetEDID(PORT1, 2); = set PORT1 EDID index = 2 = Customer_EDID[2]
// and iTE6805_Port_Select will also auto change EDID.

#define EDID_PORT0_INDEX 0  // PORT0 EDID init index
#define EDID_PORT1_INDEX 0  // PORT1 EDID init index
//#define _HDMI_SWITCH_	// do not need it anymore because it's RX cec physical addr = 0.0.0.0


//////////////////////////////////////////////////////////////////////////////////////////
// CEC Config
//////////////////////////////////////////////////////////////////////////////////////////
//#if defined(AST_HDMIRX)
#if 0
#define _ENABLE_IT6805_CEC_	FALSE
#define _ENABLE_IT6805_CEC_I2C_	FALSE
#else
#define _ENABLE_IT6805_CEC_ TRUE
#define _ENABLE_IT6805_CEC_I2C_ TRUE
#endif
#define CEC_VENDOR_ID 0x0000
#define iTE6805CECGPIOid		0	// for identife iTE6805_CEC use Pin 1.0 & 1.1
//#define DISABLE_CEC_REPLY			// for test only


#if defined(AST_HDMIRX)
#if (_ENABLE_IT6805_CEC_ == TRUE) && (AST_IT6805_INTR == 1)
#define AST_IT6805_INTR_CEC 1
#else
#define AST_IT6805_INTR_CEC 0
#endif
#endif /* #if defined(AST_HDMIRX) */

//////////////////////////////////////////////////////////////////////////////////////////
// Other Config
//////////////////////////////////////////////////////////////////////////////////////////
#define ADDR_HDMI   0x90
#define	ADDR_MHL	0xE0	//Software programmable I2C address of iTE6805 MHL
#define	ADDR_EDID	0xA8	//Software programmable I2C address of iTE6805 EDID RAM
#define ADDR_CEC	0xC8	//Software programmable I2C address of iTE6805 CEC
#define ADDR_LVDS	0xB4	//Software programmable I2C address of iTE6805 LVDS



//////////////////////////////////////////////////////////////////////////////////////////
// MHL Config
//////////////////////////////////////////////////////////////////////////////////////////
#if defined(AST_HDMIRX)
#define _ENABLE_IT6805_MHL_FUNCTION_	FALSE
#define _ENABLE_MHL_SUPPORT_3D_		FALSE
#define _ENABLE_IT6805_MHL_I2C_		TRUE /* for mhlrxwr() and mhlrxrd() */
#else
#define _ENABLE_IT6805_MHL_FUNCTION_	TRUE
#define _ENABLE_MHL_SUPPORT_3D_			TRUE
#define _ENABLE_IT6805_MHL_I2C_			TRUE
#endif

//////////////////////////////////////////////////////////////////////////////////////////
// 6805 RD define
//////////////////////////////////////////////////////////////////////////////////////////
// IO mode
#define Reg_IOMode 0x40	// default 0x40, 0x00 / 0x40 / 0x80

//////////////////////////////////////////////////////////////////////////////////////////
// Other
//////////////////////////////////////////////////////////////////////////////////////////
#ifndef _ENABLE_6805_INT_MODE_FUNCTION_
#define _ENABLE_6805_INT_MODE_FUNCTION_ FALSE
#endif

// message when complier
#if (_MCU_8051_EVB_==TRUE)
#if ((!iTE68051)&&(!iTE68052)&&(!iTE6807))
#error message("no define ITE68051 and ITE68052 and ITE6807")
#endif
#if (iTE68051)
#pragma message("Defined for IT68051")
#endif
#if (iTE68052)
#pragma message("Defined for IT68052")
#endif
#if (iTE6807)
#pragma message("Defined for IT6807")
#endif
#endif


#endif // _CONFIG_H_
