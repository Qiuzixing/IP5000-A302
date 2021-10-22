/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  TPI.c
 *
 * @brief Implementation of the TPI.
 *
 *****************************************************************************
*/

//#include <stdio.h>
#include "defs.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
//#include "gpio.h"

#include "i2c_slave_addrs.h"
#include "videomodedesc.h"
//#include "AMF_Lib.h"
#include "serialpacket.h"
#include "TPI_generic.h"
#include "SiI923_02-92_34_4.h"
#include "Externals.h"
#include "Macros.h"
#include "TPI_Regs.h"
#include "TPI_Access.h"
#include "TPI.h"
//#include "delay.h"
#include "AV_Config.h"
//#include "i2c_master_sw.h"
#include "si_apiConfigure.h"
#include "RegBits.h"
#include "txhal.h"
//#include "hal_timers.h"
//#include "SerialPort.h"
#include "tpidebug.h"
#include "edid.h"
#include "HDCP.h"
#ifdef DEV_SUPPORT_EHDMI  //{
#include "ehdmi.h"
#endif //}

#if (IS_CEC == 1) //{
#include "si_apiCpi.h"
#include "si_cpi_regs.h"
#include "si_cec_enums.h"
#include "si_apiCEC.h"
#endif //}

#ifdef MHL_CONNECTION_STATE_MACHINE //(

#include "mhl_defs.h"
#include "si_cbus_tx.h"
#include "si_cbusDefs.h"
#include "si_cbus_regs.h"
#include "queue.h"
#include "statetable.h"
mhlTx_config_t mhlTxConfig={0};

#else //)(

uint8_t g_InterruptStatusImage=0;

#endif //)
#include "tpidrv.h"

#if AST_HDMITX
#include "ast_utilities.h"
#endif

uint8_t txPowerState;		// Can be referenced externally by chip-specific TX HAL file, so cannot be static.
extern int tv_access_flag;

#if (AST_HDMITX)
static u32 has_external_hdcp(void)
{
	struct s_xmiter_info *pinfo;
	u32 ret = 0;

#if defined(CONFIG_ARCH_AST1500_HOST)
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
	pinfo = xmiter_info + XIDX_HOST_D;
#endif
#elif defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#else
	return ret;
#endif
	if (xCap_EXTERNAL_HDCP & pinfo->cap)
		ret = 1;

	return ret;
}
#endif /* #if (AST_HDMITX) */


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      : StartTPI()
//
// PURPOSE       : Start HW TPI mode by writing 0x00 to TPI address 0xC7. This
//                 will take the Tx out of power down mode.
//
// INPUT PARAMS  : None
//
// OUTPUT PARAMS : void
//
// GLOBALS USED  : None
//
// RETURNS       : TRUE if HW TPI started successfully. FALSE if failed to.
//
//////////////////////////////////////////////////////////////////////////////

static uint8_t StartTPI(void)
{
	uint8_t devID = 0x00;
	uint16_t wID = 0x0000;

	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>StartTPI()\n"));

    WriteByteTPI(PAGE_0_TPI_INFO_B7_ADDR, 0x00);            // Write "0" to 72:C7 to start HW TPI mode
	DelayMS(500); //This delay is necessary waiting for switch into TPI mode, 9024ACNU takes the longest.
    SetInTpiMode;
	devID = ReadIndexedRegister(INDEXED_PAGE_0, TXL_PAGE_0_DEV_IDH_ADDR);
	wID = devID;
	wID <<= 8;
	devID = ReadIndexedRegister(INDEXED_PAGE_0, TXL_PAGE_0_DEV_IDL_ADDR);
	wID |= devID;

    devID = ReadByteTPI(PAGE_0_TPI_DEV_ID_ADDR);


	if (devID == SiI_DEVICE_ID)
	{
    	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,"0x%04X %s\n", (int) wID,SiI_DEVICE_STRING));
		return TRUE;
	}

    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,"0x%04X\n", (int) wID));
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,"Unsupported TX\n"));
    return FALSE;
}

static void InitializeStateVariables (void)
{

    ClrTclkStable;
    SetCheckTclkStable;

    ClrTmdsPoweredUp;
    ResetDownStreamHotPlugDetectionStatus;
    ClrDsRxPoweredUp;
    ClrRxClockUnstable;
    ClrHDCPIsAuthenticated;
	ClrEdidDataValid;							// Move this into EDID_Init();
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  EnableInterrupts()
//
// PURPOSE       :  Enable the interrupts specified in the input parameter
//
// INPUT PARAMS  :  A bit pattern with "1" for each interrupt that needs to be
//                  set in the Interrupt Enable Register (TPI offset 0x3C)
//
// OUTPUT PARAMS :  void
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////
uint8_t EnableInterrupts(uint8_t Interrupt_Pattern)
{
    TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>EnableInterrupts()\n"));

    TXHAL_EnableInterrupts(Interrupt_Pattern);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  DisableInterrupts()
//
// PURPOSE       :  Enable the interrupts specified in the input parameter
//
// INPUT PARAMS  :  A bit pattern with "1" for each interrupt that needs to be
//                  cleared in the Interrupt Enable Register (TPI offset 0x3C)
//
// OUTPUT PARAMS :  void
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////
static uint8_t DisableInterrupts(uint8_t Interrupt_Pattern)
{
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>DisableInterrupts()\n"));
	ReadClearWriteTPI(PAGE_0_TPI_INTR_EN_ADDR, Interrupt_Pattern);

    TXHAL_DisableInterrupts();

    return TRUE;
}

#if AST_HDMITX
void av_mute(void)
{
	if (TPI_PAGE_0_REG_TPI_OUTPUT_MODE_B0_RD(ReadByteTPI(PAGE_0_TPI_SC_ADDR))) {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"AV Mute\n"));
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, AV_MUTE_MASK, AV_MUTE_MUTED);
	}
}

void av_unmute(void)
{
	/*
	 * In HDMI => DVI case, unmute is necessary,
	 * so do not check whether output mode is HDMI or not
	 */
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"AV Unmute\n"));
	ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, AV_MUTE_MASK, AV_MUTE_NORMAL);
}

//#define NO_SW_REST
#undef NO_SW_REST

#if defined(RESET_TEST)
static int _reset_test = 0;
void reset_test_cfg(int cfg)
{
	_reset_test = cfg;
}

int reset_test(void)
{
	return _reset_test;
}
#endif

int sii9136_software_reset(void)
{
#ifndef NO_SW_REST
#if (IS_CEC == 1)
	int reset_fix = 0;
	/*
	 * Software Reset may affect I2C communication with the CPI module. To avoid this issue perform the following:
	 * 1.  Set W equal to the number of write accesses to the CPI module (I2C address 0xC0 or 0xC4).
	 * 2.  Calculate N = 8 – (W mod 8), where W is the number of write accesses from step 1.
	 * 3.  Before executing the Software Reset, perform N writes (from step 2) into any of the CPI page (I2C address 0xC0 or 0xC4) registers 0x00 through 0x07.
	 * 4.  Execute Software Reset by setting and clearing bit 0 in TPI
	 */
#if defined(RESET_TEST)
	reset_fix = _reset_test;
#endif
	SI_TxPreSwReset(reset_fix);
#endif /* #if (IS_CEC == 1) */
#endif

	/*
	** Bruce180130. From my test, following sequences:
	** #1. to D2, to D0, sw reset: 2160p30 pass
	** #2. sw reset, to D2, to D0: 2160p30 pass (programming guide recommended for "D0 to D2")
	** #3. to D2, to D0: 2160p30 pass
	** #4. sw reset: 2160p30 failed
	**
	** Looks like "D2 to D0" is the key.
	** We decided to field test sequence #2. (because #1 was original code having LG TV HDCP issue.)
	*/
	/* software reset in SiI-SW-1090-SRC-v.3.5.10 just set Reg0x40[0] and clear Reg0x40[0] */
#ifndef NO_SW_REST
	ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, BIT_0, HIGH);
	DelayMS(10); /* leave this one */
	ReadModifyWriteTPI(PAGE_0_TPI_SW_RST_EN_ADDR, BIT_0, LOW);
#endif
	/* power state => D2 => D0*/
	ReadModifyWriteTPI(PAGE_0_TPI_PWR_ST_ADDR, TPI_PAGE_0_REG_TPI_PWR_STATE_MASK, 2);
	mb();
	ReadModifyWriteTPI(PAGE_0_TPI_PWR_ST_ADDR, TPI_PAGE_0_REG_TPI_PWR_STATE_MASK, 0);

#ifndef NO_SW_REST
#if (IS_CEC == 1)
	if (!cec_ok_after_reset()) {
#if defined(RESET_TEST)
		_reset_test = 0;
#endif
		return -1;
	}
#endif
#endif

	/*
	 * delay after software reset
	 *
	 * HDCP no encryption bug:
	 * enable TDMS right after software reset cause HDCP state machine run into abnormal state
	 * cannot fix this situation even if disable/enable HDCP
	 * only disable & enable TMDS to let HDCP work
	 *
	 * add a delay before tmds enable (wait for software reset completion)
	 */
	/*
	** Bruce171127.
	** __sii9136_set_video() will call sii9136_software_reset() before calling SiiMhlTxDrvTmdsControl(true, false).
	** From test, we found that it will cause SiI9136 run into a bad HDCP state.
	** SiI9136 will trigger an HPD interrupt without any further changes.
	** And will not response to setting Reg2A[0]. HDCP will stop working from that point.
	** To resolve this problem, we found adding a small delay after sii9136_software_reset() will resolve this issue.
	** The delay value is get from other HDCP related function. Not strictly measured.
	*/
	DelayMS(128); /* SiI-SW-1090-SRC-v.3.5.10 dealy 20 ms after software reet, we select 32ms */

	return 0;
}

u32 tmds_clock_stable(void)
{
	u8 val;
	val = ReadByteTPI(PAGE_0_TPI_SW_RST_EN_ADDR);

	return TPI_PAGE_0_PDO_RD(val) & TPI_PAGE_0_P_STABLE_RD(val);
}
#endif

#if !AST_HDMITX
//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      : SetPreliminaryInfoFrames()
//
// PURPOSE       : Set InfoFrames for default (initial) setup only
//
// INPUT PARAMS  : VIC, output mode,
//
// OUTPUT PARAMS : void
//
// GLOBALS USED  : None
//
// RETURNS       : TRUE
//
//////////////////////////////////////////////////////////////////////////////
static uint8_t SetPreliminaryInfoFrames(void)
{
    uint8_t i;
    //API_Cmd Command;        // to allow using function SetAVI_InfoFrames()

        TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>SetPreliminaryInfoFrames()\n"));

    for (i = 0; i < MAX_COMMAND_ARGUMENTS; i++)
	{
        g_ScratchPad.CommData.payload.bytes[i] = 0;
	}

    g_ScratchPad.CommData.header.PayloadSizeInBytes = 0; // fixes SetAVI_InfoFrames() from going into an infinite loop

    g_ScratchPad.CommData.payload.vidModeChange.VIC = VideoModeDescription.VIC;

	switch (VideoModeDescription.OutputColorSpace)
	{
		case VMD_COLOR_SPACE_YCBCR422:
            g_ScratchPad.CommData.payload.setInfoFrames.infoFrameData.aviInfoFrame.DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0 = 0x02;
			break;
		case VMD_COLOR_SPACE_YCBCR444:
            g_ScratchPad.CommData.payload.setInfoFrames.infoFrameData.aviInfoFrame.DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0 = 0x01;
			break;
		case VMD_COLOR_SPACE_RGB:
		default:
        	g_ScratchPad.CommData.payload.setInfoFrames.infoFrameData.aviInfoFrame.DataByte1_RV_Y1_Y0_A0_B1_B0_S1_S0 = 0x00;
			break;
	}

    SetAVI_InfoFrames();

	// Setup the Vendor Specific Info Frame
	Call_Set_VSIF;  // code generation by this line is controlled in av_config.h

    return TRUE;
}
#endif

static void TxPowerStateD0 (void)
{

	ReadModifyWriteTPI(PAGE_0_TPI_PWR_ST_ADDR, TX_POWER_STATE_MASK, TX_POWER_STATE_D0);
    TXHAL_PowerStateD0();

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TX Power State D0\n"));
#ifndef MHL_CONNECTION_STATE_MACHINE //{
	txPowerState = TX_POWER_STATE_D0;
#endif //}
}

#ifdef MHL_CONNECTION_STATE_MACHINE
static void TxPowerStateD3 (void)
{
    TXHAL_PowerStateD3();

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TX Power State D3\n"));

#ifndef MHL_CONNECTION_STATE_MACHINE //{
	txPowerState = TX_POWER_STATE_D3;
#endif //}
    CLEAR_CBUS_TOGGLE();  // this is a macro that is non-trivially defined only when needed.
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   HotPlugService()
//
// PURPOSE      :   Implement Hot Plug Service Loop activities
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   void
//
// GLOBALS USED :   LinkProtectionLevel
//
// RETURNS      :   An error code that indicates success or cause of failure
//
//////////////////////////////////////////////////////////////////////////////

void HotPlugService (void)
{
#if (AST_HDMITX)
	char intr_cfg = HOT_PLUG_EVENT | RX_SENSE_EVENT | AUDIO_ERROR_EVENT | V_READY_EVENT;

	/*
	 * always include HDCP_CHANGE_EVENT
	 *
	 * if not, we will have a problem with following scenario,
	 * 1. source HDCP is off
	 * 2. plug HDMI cable
	 * 3. source HDCP is on
	 * 4. something happened but HDCP event
	 * video will get displayed after 4.
	 */
	intr_cfg |= HDCP_CHANGE_EVENT;
#if (IS_CEC == 1)
	intr_cfg |= RX_SENSE_STATE; /* we use CPI event in place if Rx Sense */
#endif
#endif
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>HotPlugService()\n"));

	DisableInterrupts(0xFF);

	/*
	 * video/audio setup is controlled by CRT
	 * no configuration when hotplug if AST_HDMITX is true
	 */
#if !AST_HDMITX

	// workaround for Bug#18128
	if (VideoModeDescription.OutputColorDepth == VMD_COLOR_DEPTH_8BIT) {
		// Yes it is, so force 16bpps first!
		VideoModeDescription.OutputColorDepth = VMD_COLOR_DEPTH_16BIT;
		InitVideo(X1);
		// Now put it back to 8bit and go do the expected InitVideo() call
		VideoModeDescription.OutputColorDepth = VMD_COLOR_DEPTH_8BIT;
	}
	// end workaround

	InitVideo(X1); /* Set PLL Multiplier to x1 upon power up */

#ifndef MHL_CONNECTION_STATE_MACHINE
	TxPowerStateD0();
#endif

	if (IsHDMI_Sink()) {
		/* Set InfoFrames only if HDMI output mode */
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"AUDIO -> Basic\n"));
		SetPreliminaryInfoFrames();
		SetBasicAudio(); /* set audio interface to basic audio (an external command is needed to set to any other mode */
	} else {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"AUDIO -> Muted\n"));
		SetAudioMute(AUDIO_MUTE_MUTED);
	}
#endif /* #if !AST_HDMITX */

#ifdef DEV_SUPPORT_EHDMI
//	EHDMI_ARC_Common_Enable();
	EHDMI_ARC_Common_With_HEC_Enable();
#endif

	if (HDCP_TxSupports) {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "HDCP_TxSupports\n"));
		if (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED) {
			TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "VMD_HDCP_AUTHENTICATED\n"));
			if (HDCP_AksvValid) {

#if (!AST_HDMITX)
				/* AV MUTE */
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "TMDS -> Enabled (Video Muted)\n"));

#ifdef EKKI_FPGA
				I2C_WriteByte(SA_TX_Ekki_Page_0, PAGE_0_TPI_PIX_CLK_MSB_ADDR, 0x20); /* Enable tmds of TX PHY at 0x01[5] */
#endif

				SiiMhlTxTmdsEnable(false);
#else /* #if (!AST_HDMITX) */
				/*
				** Bruce170109. BenQ monitor compatibility fix.
				** Power off/on BenQ monitor will not trigger hotplug,
				** but only TMDS power down/up. The original SII driver's
				** logic here is strange. After AVMute, there is no more
				** HDCP interrupt happen under this case. So that Sii9136
				** will mute forever.
				** Solution:
				** Don't do AVMute here.
				** TODO:
				** Don't know the side effect. Maybe notify CRT is a better way to go.
				*/
				SiiMhlTxTmdsEnable(true);
#endif /* #if (!AST_HDMITX) */

#if AST_HDMITX
				EnableInterrupts(intr_cfg | SECURITY_CHANGE_EVENT);
#else
				EnableInterrupts(HOT_PLUG_EVENT | RX_SENSE_EVENT | AUDIO_ERROR_EVENT | SECURITY_CHANGE_EVENT | V_READY_EVENT | HDCP_CHANGE_EVENT);
#endif
			}
#if AST_HDMITX
		} else {
			EnableInterrupts(intr_cfg);
#endif
		}
	} else {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL, "TMDS -> Enabled\n"));

#ifdef EKKI_FPGA
		I2C_WriteByte(SA_TX_Ekki_Page_0, PAGE_0_TPI_PIX_CLK_MSB_ADDR, 0x20); /* Enable tmds of TX PHY at 0x01[5] */
#endif

		SiiMhlTxTmdsEnable(true);
#if AST_HDMITX
		EnableInterrupts(intr_cfg);
#else
		EnableInterrupts(HOT_PLUG_EVENT | RX_SENSE_EVENT | AUDIO_ERROR_EVENT | V_READY_EVENT);
#endif
	}

#ifdef MHL_CONNECTION_STATE_MACHINE
	TxPowerStateD0();

	WriteByteTPI(PAGE_0_TPI_INFO_B13_ADDR, 0x00);	    		// Set last byte of Audio InfoFrame
	WriteByteTPI(PAGE_0_TPI_END_RIGHT_BAR_MSB_ADDR, 0x00);	    // Set last byte of AVI InfoFrame
#endif
}

static void OnDownstreamRxPoweredDown(void)
{
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"DSRX -> Powered Down\n"));
	ClrDsRxPoweredUp;

	if (HDCP_Started)
	{
		HDCP_Off();
	}

#if (AST_HDMITX)
	/*
	 * For HDCP2.2 compliance test,
	 * the reason we got fail at client side might be:
	 *	we disable TMDS during SiI9678 HDCP authentication (when equipmet simulate unplug/plug)
	 * So we add external HDCP chip check here to skip TMDS disable behavior
	 */
	if (!has_external_hdcp())
		SiiMhlTxDrvTmdsControl(false, HDCPIsAuthenticated);
#else
	SiiMhlTxDrvTmdsControl(false, HDCPIsAuthenticated);
#endif
	ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK,OUTPUT_MODE_DVI);	// Set to DVI output mode to reset HDCP
}

static void OnDownstreamRxPoweredUp(void)
{
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"DSRX -> Powered Up %s %02X\n",HDCPIsAuthenticated?"IS authenticated":"NOT authenticated",(int)HDCPIsAuthenticated));
	SetDsRxPoweredUp;

	HotPlugService();

	ClrUSRX_OutputChange;
#ifdef MHL_CONNECTION_STATE_MACHINE //(
	ClrHDCP_Override;
	HDCP_Init();
#endif //)
}

#if (AST_HDMITX)
void edid_read_retry_version(void)
{
	int retry = 0;


	while (DoEdidRead() != EDID_OK)	 {
		ClrEdidDataValid; /* clear and we can do reading next time */
		DelayMS(200);

		retry++;

		if (retry > 3) {
			break;
		}
	}

	SetEdidDataValid;
}
#endif /* #if (AST_HDMITX) */

static void OnHdmiCableConnected(void)
{
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"HDMI Connected\n"));
	tv_access_flag = 1;
#if (AST_HDMITX)
	SetCableConnected;
	HDCP_RETRY_COUNTER_RESET;
#endif

#ifdef HDCP_DONT_CLEAR_BSTATUS //{
#else //}{
	if ((HDCP_TxSupports) && (HDCP_AksvValid) && (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED))
	{
		WriteTxPage0Register( TXL_PAGE_0_TXDS_BSTATUS1_ADDR, 0x00); // Clear BStatus
		WriteTxPage0Register( TXL_PAGE_0_TXDS_BSTATUS2_ADDR, 0x00);
	}
#endif //}

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Reading EDID\n"));
#if (AST_HDMITX)
	edid_read_retry_version();
#else
	Call_DoEdidRead;  // whether or not this line produces any code is defined in av_config.h
#endif

#ifdef READKSV //{
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"READKSV\n"));
	ReadModifyWriteTPI(PAGE_0_TPI_HW_OPT3_ADDR, 0x08, 0x08);
#endif //}

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Selecting output mode\n"));
	if (IsHDMI_Sink())              // select output mode (HDMI/DVI) according to sink capabilty
	{
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"HDMI Sink Detected\n"));
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_HDMI);
#if (AST_HDMITX)
#if (IS_CEC == 1)
	si_cpi_topology_poll_cfg(1);
#endif
#endif
	}
	else
	{
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"DVI Sink Detected\n"));
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_DVI);
	}

#ifdef MHL_CONNECTION_STATE_MACHINE //(
	OnDownstreamRxPoweredUp();		// RX power not determinable? Force to on for now.
#endif //)

#if (AST_HDMITX)
	crt_hotplug_tx(1);
#endif
}

static void OnHdmiCableDisconnected(void)
{

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"HDMI Disconnected\n"));
	tv_access_flag = 0;
#if (AST_HDMITX)
	ClrCableConnected;
#endif

	//dd hdmiCableConnected = FALSE; is this done higher level?
    //jb  Yes it is!

	ClrEdidDataValid;

	OnDownstreamRxPoweredDown();

#if (AST_HDMITX)
	crt_hotplug_tx(0);
#endif
}

#ifdef MHL_CONNECTION_STATE_MACHINE //{
static void OnMHLCableConnected(void)
{

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MHL Connected\n"));

	if( !bInTpiMode )
	{
	    StartTPI();
	    //EnableInterrupts();
	    TxPowerStateD0();
	}

#ifdef SYS_BOARD_STARTERKIT //{
	pinM2U_VBusCtrl = 1;
#ifdef SK_X02 //{
#else //}{
	pinVBusIsolate = 1;
#endif //}
	pinMHLConn = 0;
	pinUsbConn = 1;
#endif //}

	WriteTxPage0Register( SW_WVN_PAGE_0_MHLTX_CTL1_ADDR, 0x10);

	// Un-force HPD
	ReadModifyWriteTxPage0Register( SW_WVN_PAGE_0_INT_CTRL_ADDR, INT_CTRL_REG_HPD_OUT_OVR_EN_MASK, INT_CTRL_REG_HPD_OUT_OVR_EN_DISABLE);


	ReadModifyWriteCBUS(TX_WVN_PAGE_C8_CBUS_INTR_STATUS_ADDR, BIT_4, 0x00);		// Clear status bit


	if (pin9290_938x == settingMode9290)
	{
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Setting DDC Burst Mode\n"));
		WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_COMMON_CONFIG_ADDR, DDC_XLTN_TIMEOUT_MAX_VAL | 0x0E); 	// Increase DDC translation layer timer (burst mode)
		WriteByteCBUS(TX_WVN_PAGE_C8_UNDEFINED_ADDR_47, 0x03);
	}
	else
	{
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Setting DDC Byte Mode\n"));
		WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_COMMON_CONFIG_ADDR, DDC_XLTN_TIMEOUT_MAX_VAL | 0x02); 	// Increase DDC translation layer timer (byte mode)
		ReadModifyWriteCBUS(TX_WVN_PAGE_C8_CBUS_DDCTST_CONTROL_ADDR, BIT_1, BIT_1);				// Enable segment pointer workaround
	}
	// Notify upper layer of cable connection
	SiiMhlTxNotifyConnection(true);

}


static void OnMHLCableDisconnected(void)
{

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MHL Disconnected\n"));

	OnHdmiCableDisconnected();
	// Notify upper layer of cable connection
	SiiMhlTxNotifyConnection(false);

    TXHAL_OnMHLCableDisconnected();
}

void GoToD3 (void)
{

#ifdef SYS_BOARD_STARTERKIT //{
	//pinWol2MHLRxPwr = 1;
#ifdef SK_X02 //{
	pinM2U_VBusCtrl = 0;
#else //}{
	pinVBusIsolate = 0;
#endif //}
	pinMHLConn = 1;
	pinUsbConn = 0;
#endif //}

	HDCP_Off();

	TxPowerStateD3();

    ResetDownStreamHotPlugDetectionStatus
    SiiMhlTxNotifyDsHpdChange( 0 );
	ClrDsRxPoweredUp
    // see TXHAL_PowerStateD3 for clearing the InTpiMode flag
}

void TPI_GoToD3 (void)
{
	GoToD3();
}

void CheckTxFifoStable (void)
{

	uint8_t bTemp;

	bTemp = ReadIndexedRegister(INDEXED_PAGE_0, TXL_PAGE_0_IADJUST_ADDR);
	if ((bTemp & (BIT_7 | BIT_6)) != 0x00)
	{
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"FIFO Overrun / Underrun\n"));
		WriteTxPage0Register( TXL_PAGE_0_SRST_ADDR, BIT_4 | ASR_VALUE);	// Assert MHL FIFO Reset
		DelayMS(1);
		WriteTxPage0Register( TXL_PAGE_0_SRST_ADDR, ASR_VALUE);			// Deassert MHL FIFO Reset
	}
}
#endif //}

static int tpi_init_first_time = 1;

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      : TPI_Init()
//
// PURPOSE       : TPI initialization: HW Reset, Interrupt enable.
//
// INPUT PARAMS  : None
//
// OUTPUT PARAMS : void
//
// GLOBALS USED  : None
//
// RETURNS      :   TRUE
//
//////////////////////////////////////////////////////////////////////////////
uint8_t TPI_Init(void)
{
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>TPI_Init()\n"));

#if !(AST_HDMITX)
	{
		int i = 0;
		char *pChar;
		// Always display the Firmware Boilerplate
		printf("TPI Firmware Version ");
		for (i=0,pChar=(char *)&TPI_FW_VERSION; i < (int)sizeof(TPI_FW_VERSION); i++)
		{
			printf("%c", pChar[i]);
		}
		printf("\n");
	}
#endif

#ifdef MHL_CONNECTION_STATE_MACHINE
	mhlTxConfig.linkMode |= 0x03;
#endif

	txPowerState = TX_POWER_STATE_D0;				// Chip powers up in D2 mode, but init to D0 for testing purpose.
#if AST_HDMITX
	if (tpi_init_first_time)
		InitializeStateVariables();
#else
	InitializeStateVariables();
#endif
	TxHW_Reset();									// Toggle TX reset pin


	if (StartTPI())									// Enable HW TPI mode, check device ID
	{
#ifndef MHL_CONNECTION_STATE_MACHINE //(
#if AST_HDMITX
		if (tpi_init_first_time) {
			ClrHDCP_Override;
			HDCP_Init();
		}
#else
		ClrHDCP_Override;
		HDCP_Init();
#endif

#endif //)
#ifdef DEV_SUPPORT_EHDMI //{
		EHDMI_Init();
#endif //}

#ifdef MHL_CONNECTION_STATE_MACHINE //{

		OnMHLCableDisconnected();					// Default to USB Mode.
		txPowerD3();

		// CpCbusInitialize is called when an MHL connection is established.
		//  No need for it here

#endif //}

#if (IS_CEC == 1) //{
		SI_CecInit();
#endif //}

		tpi_init_first_time = 0;
		return TRUE;
	}

	tpi_init_first_time = 0;
	return FALSE;
}


#ifdef MHL_CONNECTION_STATE_MACHINE
static void WakeUpFromD3 (void)
{

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Waking up...\n"));

	InitializeStateVariables ();
	TXHAL_InitPostReset();
	//WriteInitialRegisterValues();
#if (IS_CEC == 1) //{
    SI_CecInit();
#endif //}
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      : HW_Reset()
//
// PURPOSE       : Send a
//
// INPUT PARAMS  : None
//
// OUTPUT PARAMS : void
//
// GLOBALS USED  : None
//
// RETURNS       : Void
//
//////////////////////////////////////////////////////////////////////////////

void TxHW_Reset(void)
{
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>TxHW_Reset()\n"));
	TXHAL_InitPreReset();

#if AST_HDMITX
	/* 
	 * driver just do HW reset before starting HW TPI mode
	 * we do HW reset at the beginning of sii9136_probe(), so there is no need to do again
	 *
	 */
#else
	PinTxHwReset = LOW;
	DelayMS(TX_HW_RESET_PERIOD);
	PinTxHwReset = HIGH;
	DelayMS(TX_HW_RESET_PERIOD);
#endif
	TXHAL_InitPostReset();
}


void RAPContentOn (void)
{
    SiiMhlTxTmdsEnable( true );
}
void RAPContentOff (void)
{
    SiiMhlTxTmdsEnable( false );
}

void RestartHDCP (void)
{
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"HDCP -> Restart %s\n", ((tmdsPoweredUp)?"tmdsPoweredUp":"NOT tmdsPoweredUp")));

    SiiMhlTxDrvTmdsControl( false,HDCPIsAuthenticated );
    HDCP_Off();
    SiiMhlTxTmdsEnable(false);

#ifdef MHL_CONNECTION_STATE_MACHINE //)
    HalTimerSet(TIMER_HDCP,HDCP_WORKAROUND_INTERVAL);
#endif //)
}

void SetAudioMute (uint8_t audioMute)
{
	ReadModifyWriteTPI(PAGE_0_TPI_CONFIG3_ADDR, AUDIO_MUTE_MASK, audioMute);
}

#if (AST_HDMITX)
static u32 _no_tmds_disable = 0;

void no_tmds_disable_cfg(u32 cfg)
{
	_no_tmds_disable = cfg;
}

u32 no_tmds_disable(void)
{
	return (_no_tmds_disable || (ast_scu.astparam.hdcp_cts_option & HDCP_CTS_TMDS_ALWAYS_ON));
}
#endif

void SiiMhlTxDrvTmdsControl( bool_t enable, bool_t UnMute )
{
	if (enable)
	{
		uint8_t Mask,Value;
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TMDS -> Enabled, %s\n",UnMute?"UnMuted":"Muted"));

#ifdef EKKI_FPGA //{
		I2C_WriteByte(SA_TX_Ekki_Page_0, PAGE_0_TPI_PIX_CLK_MSB_ADDR, 0x20);	// Enable tmds of TX PHY at 0x01[5]
#endif //}
		Mask  = LINK_INTEGRITY_MODE_MASK | TMDS_OUTPUT_CONTROL_MASK   |         AV_MUTE_MASK;
		Value = LINK_INTEGRITY_DYNAMIC   | TMDS_OUTPUT_CONTROL_ACTIVE | (UnMute?AV_MUTE_NORMAL:AV_MUTE_MUTED);
		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR,  Mask, Value);
		SetTmdsPoweredUp;
	}
	else
	{
#if (AST_HDMITX)
		if (no_tmds_disable())
			return;
#endif

		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TMDS -> Disabled\n"));

#ifdef EKKI_FPGA //{
		I2C_WriteByte(SA_TX_Ekki_Page_0, PAGE_0_TPI_PIX_CLK_MSB_ADDR, 0x00);	// Disable tmds of TX PHY at 0x01[5]
#endif //}

		ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR
			,  TMDS_OUTPUT_CONTROL_MASK	        | AV_MUTE_MASK
			,  TMDS_OUTPUT_CONTROL_POWER_DOWN   | AV_MUTE_MUTED
				    );
		ClrTmdsPoweredUp;
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   TPI_Poll ()
//
// PURPOSE      :   Poll Interrupt Status register for new interrupts
//
// INPUT PARAMS :   None
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   LinkProtectionLevel
//
// RETURNS      :   None
//
//////////////////////////////////////////////////////////////////////////////

#ifndef MHL_CONNECTION_STATE_MACHINE//{
static uint8_t get_rx_sense_state(uint8_t InterruptStatusImage)
{
#if (AST_HDMITX)
	uint8_t ints, inte;
	/*
	** Bruce180126. When enabled CEC interrupt Reg3D[3],
	** RX_SENSE_STATE status is not RX_SENSE_STATE anymore.
	** To workaround this, we disable CEC interrupt first,
	** then read it.
	*/
	inte = ReadByteTPI(PAGE_0_TPI_INTR_EN_ADDR);
	inte &= RX_SENSE_STATE;
	DisableInterrupts(inte);
	mb();
	ints = ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR);
	mb();
	EnableInterrupts(inte);
	return ints;
#else
	return InterruptStatusImage;
#endif
}


void TPI_Poll (void)
{
	uint8_t InterruptStatusImage;

	if (txPowerState == TX_POWER_STATE_D0)
	{
		InterruptStatusImage = ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR);

		/*
		** Bruce180131. sii9136_software_reset() patch.
		** HOT_PLUG_EVENT may somehow missing if we applied sii9136_software_reset() in __sii9136_set_video()
		** To workaround this, we poll hotplug status when !CableConnected.
		**
		** How to reproduce:
		** Client#1-+->YAMAHA AVR->SONY TV
		**          |
		** Client#2-+
		**
		** Use AVR to switch between clients. There will have a chance that
		** plug INT event get lost and driver keeps in detached state.
		*/
		if ((InterruptStatusImage & HOT_PLUG_EVENT) || !CableConnected)
		{
			if (InterruptStatusImage & HOT_PLUG_EVENT) {
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"HPD  -> \n"));
				ReadSetWriteTPI(PAGE_0_TPI_INTR_EN_ADDR, HOT_PLUG_EVENT);  // Enable HPD interrupt bit
			}
			// Repeat this loop while cable is bouncing:
			do
			{
				WriteByteTPI(PAGE_0_TPI_INTR_ST_ADDR, HOT_PLUG_EVENT);
				DelayMS(T_HPD_DELAY);//leave this one -revisit later // Delay for metastability protection and to help filter out connection bouncing
				InterruptStatusImage = ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR);    // Read Interrupt status register
			} while (InterruptStatusImage & HOT_PLUG_EVENT);              // loop as long as HP interrupts recur

			if (downStreamHotPlugDetectionStatusChanged(InterruptStatusImage))
			{
				if (hdmiCableConnected(InterruptStatusImage)) {
					OnHdmiCableConnected();
					ReadModifyWriteTxPage0Register(TXL_PAGE_0_SYS_CTRL3_ADDR, 0x08, 0x08);
				} else {
					OnHdmiCableDisconnected();
				}

				g_InterruptStatusImage = InterruptStatusImage;
				if (!hdmiCableConnected(InterruptStatusImage)) {
					TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TPI_Poll Done!\n"));
					goto tpi_poll_exit; /* DO NOT return, have to handle CEC */
					/* Bruce180131. Don't clear other ints. In case driver need it. */
					return;
				}
			}
		}

		// Check for change in rx power
		do {
			uint8_t rx_sense_state;
			rx_sense_state = get_rx_sense_state(InterruptStatusImage);

			if (((rx_sense_state & RX_SENSE_STATE)?dsRxPoweredUpBit:0) ^ (dsRxPoweredUp?dsRxPoweredUpBit:0))
			{
				// record the value
				AssignDsRxPoweredUp(rx_sense_state & RX_SENSE_STATE)
				// act accordingly when changed
				if (hdmiCableConnected(rx_sense_state))
				{
					if (dsRxPoweredUp) {
						OnDownstreamRxPoweredUp();

#if (AST_HDMITX)
						/*
						 * driver always sets to DVI output in OnDownstreamRxPoweredDown() when sink power down
						 * we have to recover output mode to avoid no sound issue if sink is HDMI
						 */
						if (sii9136_output_hdmi())
							ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_HDMI);
						else
							ReadModifyWriteTPI(PAGE_0_TPI_SC_ADDR, OUTPUT_MODE_MASK, OUTPUT_MODE_DVI);
#endif
					} else {
#if (AST_HDMITX)
						if (!hdmiCableConnected(ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR))) {
							g_InterruptStatusImage = 0;
							OnHdmiCableDisconnected();
						} else {
							OnDownstreamRxPoweredDown();
						}
#else
						OnDownstreamRxPoweredDown();
#endif
					}
				} else {
					/* cable is not connected, let related variables are what we want */
					g_InterruptStatusImage = 0;
					OnHdmiCableDisconnected();
				}
				ClearInterrupt(RX_SENSE_EVENT);
			}
		} while (0);

#if (AST_HDMITX)
		/* Need to check Rx Sense even Cable is not connected. */
		if (!CableConnected)
			goto tpi_poll_exit;
#endif

		// Check if Audio Error event has occurred:
		if (InterruptStatusImage & AUDIO_ERROR_EVENT)
		{
			//TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TP -> Audio Error Event\n"));
			//  The hardware handles the event without need for host intervention (PR, p. 31)
			ClearInterrupt(AUDIO_ERROR_EVENT);
		}

		if (hdmiCableConnected(InterruptStatusImage))
		{
			if (dsRxPoweredUp)
			{
				if (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED)
				{
					HDCP_CheckStatus(InterruptStatusImage);
				}
			}
		}

		if ((tmdsPoweredUp) && (USRX_OutputChange))
		{
			TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,"TP -> Video Mode...\n"));
			SiiMhlTxDrvTmdsControl( false, false);

			if (HDCP_Started)
			{
				HDCP_Off();
			}

			HotPlugService();
			ClrUSRX_OutputChange;
		}

#if (AST_HDMITX)
tpi_poll_exit:
#if (IS_CEC == 1)
		/*
		 * handle CEC even if cable is disconnected
		 * if not, we'll get flooding of 'DSRX -> Powered Down' message when CEC interrupt raised after cable disconnected
		 */
		SI_CecHandler(0, 0); /* handle CEC even if cable is disconnected */
#endif
		WriteByteTPI(PAGE_0_TPI_INTR_ST_ADDR, InterruptStatusImage);
#else /* #if (AST_HDMITX) */
#if (IS_CEC == 1)
		SI_CecHandler(0, 0);
#endif
#endif /* #if (AST_HDMITX) */
	}
	else if (txPowerState == TX_POWER_STATE_D3)
	{
#if (AST_HDMITX)
		//BruceToCheck.
#else
		if (PinTxInt == 0)
		{
			TPI_Init();
		}
#endif
	}
}
#else //}{
#if 0 // see TXHAL_InitCBusRegs
///////////////////////////////////////////////////////////////////////////
// InitCBusRegs
//
///////////////////////////////////////////////////////////////////////////
static void InitCBusRegs( void )
{
	uint8_t		regval;

	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: InitCBusRegs\n"));
	// Increase DDC translation layer timer
//	WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_COMMON_CONFIG_ADDR, 0x32);

}
#endif
#define INTERVAL_RSEN_DEGLITCH 150

ByteQueue_t txEventQueue={0,0,{0}};

PStateTableEntry_t LookupNextState(TxPowerState_e state, uint8_t event)
{
PStateTableEntry_t pStateTableEntry = TxPowerStateTransitionAndResponseTable[state].pStateRow;
uint16_t transitionActionFlags;
    do
    {
        transitionActionFlags = pStateTableEntry->transitionActionFlags;
        if (event == pStateTableEntry->event)
        {
            TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"lookup state:%d event:%d\n",(int)state,(int)event));
            return pStateTableEntry;
        }
        pStateTableEntry++;
    }while (!(txtfLastEntryThisRow & transitionActionFlags));
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"lookup failed. event:%d\n",(int)event));
    return NULL;
}

// pragma below disables and restores interrupts during the function that follows
#pragma disable
TxPowerStateEvent_e GetNextTxEvent()
{
    if (0==QUEUE_DEPTH(txEventQueue))
    {
        return txpseNoOp;
    }
    else
    {
    TxPowerStateEvent_e retVal;
        retVal = txEventQueue.queue[txEventQueue.head];
        ADVANCE_QUEUE_HEAD(txEventQueue)
        return retVal;
    }
}

// pragma below disables and restores interrupts during the function that follows
#pragma disable
bool_t PutNextTxEventImpl(TxPowerStateEvent_e event)
{
    if (QUEUE_FULL(txEventQueue))
    {
        //queue is full
        return false;
    }
    // at least one slot available
    txEventQueue.queue[txEventQueue.tail] = event;
    ADVANCE_QUEUE_TAIL(txEventQueue)
    return true;
}
// use this wrapper to do debugging output for the routine above.
bool_t PutNextTxEventWrapper(TxPowerStateEvent_e event,char *pszEvent,int iLine)
{
	bool_t retVal;

    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"PutNextTxEventWrapper: called from line:%d event: %s(%d) depth:%d head: %d tail:%d\n"
                ,iLine
                ,pszEvent
                ,(int)event
                ,(int)QUEUE_DEPTH(txEventQueue)
                ,(int)txEventQueue.head
                ,(int)txEventQueue.tail
                ));
    retVal = PutNextTxEventImpl(event);

    if (!retVal)
    {
        TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"queue full, when adding event %d called from line:%d\n",(int)event,iLine));
    }
    return retVal;
}



uint8_t ExamineIntr4(void)
{
	uint8_t intr4Image;
	uint8_t intr4IntsHandled=0;
    intr4Image = ReadTxPage0Register(SW_WVN_PAGE_0_INTR4_ADDR);
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"intr4Image:0x%x\n",(int)intr4Image));
#ifdef SWWA_20005 //{
    if (intr4Image & INTR4_SCDT_CHANGE)
    {
    	TPI_DEBUG_PRINT(("SCDT Changed -> "));
    	OnScdtChange();
        intr4IntsHandled |= INTR4_SCDT_CHANGE;
    }
#endif //}
	if (intr4Image & INTR4_MHL_EST)
	{
		// MHL Mode Established
        PutNextTxEvent(txpseMHL_Established);
        intr4IntsHandled |= INTR4_MHL_EST;
	}

	if (intr4Image & INTR4_USB_EST)
	{
		// usb mode established
        PutNextTxEvent(txpseUSB_Established);
        intr4IntsHandled |= INTR4_USB_EST;
	}

	if (intr4Image & INTR4_CBUS_LKOUT)
	{
        TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"CBus Lockout\n"));
		// CBus Lockout
        PutNextTxEvent(txpseCBUS_LockOut);
        intr4IntsHandled |= INTR4_CBUS_LKOUT;
	}

   	if (intr4Image & INTR4_RGND_RDY)
   	{
   		// RGND Detection
        PutNextTxEvent(txpseRGND_Ready);
        intr4IntsHandled |= INTR4_RGND_RDY;
   	}

    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"exit intr4Image:0x%x\n",(int)intr4Image));
    return intr4IntsHandled;
}

bool_t SiiMhlTxDrvCBusBusy(void)
{
    return mscCmdInProgress ? true : false;
}

bool_t SiiMhlTxDrvSendCbusBurst(uint8_t commandOrOffset
                                , uint8_t *values
                                , uint8_t length
                                )
{
int i, address;
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"SiiMhlTxDrvSendCbusBurst %02X %02X\n",(int)commandOrOffset,(int)length));
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_CMD_OR_OFFSET_ADDR        , commandOrOffset);
    WriteByteCBUS( TX_WVN_PAGE_C8_CBUS_MSC_WRITE_BURST_LEN_ADDR , length -1      );
    for (i = 0,address = TX_WVN_PAGE_C8_REG_CBUS_SCRATCHPAD_0_ADDR; i < length;++i,++address)
    {
        WriteByteCBUS(address,values[i]);
    }
    WriteByteCBUS( TX_WVN_PAGE_C8_MSC_COMMAND_START_ADDR        ,  MSC_START_BIT_WRITE_BURST);
    return true;
}

bool_t SiiMhlTxDrvSendCbusCommandNoValues(uint8_t commandOrOffset
                                , uint8_t startbit
                                )
{
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"SiiMhlTxDrvSendCbusCommandNoValues %02X %02x\n",(int)commandOrOffset,(int)startbit));
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_CMD_OR_OFFSET_ADDR    , commandOrOffset);
    WriteByteCBUS( TX_WVN_PAGE_C8_MSC_COMMAND_START_ADDR    , startbit       );
    return true;
}
bool_t SiiMhlTxDrvSendCbusCommand1Value(uint8_t commandOrOffset
                                , uint8_t value
                                , uint8_t startbit
                                )
{
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"SiiMhlTxDrvSendCbusCommand1Value %02X %02X %02X\n",(int)commandOrOffset,(int)value,(int)startbit));
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_CMD_OR_OFFSET_ADDR    , commandOrOffset);
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_1ST_TRANSMIT_DATA_ADDR, value          );
    WriteByteCBUS( TX_WVN_PAGE_C8_MSC_COMMAND_START_ADDR    , startbit       );
    return true;
}
bool_t SiiMhlTxDrvSendCbusCommand2Values(uint8_t commandOrOffset
                                , uint8_t value0
                                , uint8_t value1
                                , uint8_t startbit
                                )
{
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"SiiMhlTxDrvSendCbusCommand2Values %02X %02X %02X %02X\n",(int)commandOrOffset,(int)value0,(int)value1,(int)startbit));
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_CMD_OR_OFFSET_ADDR    , commandOrOffset);
	WriteByteCBUS( TX_WVN_PAGE_C8_MSC_1ST_TRANSMIT_DATA_ADDR, value0         );
    WriteByteCBUS( TX_WVN_PAGE_C8_MSC_2ND_TRANSMIT_DATA_ADDR, value1         );
    WriteByteCBUS( TX_WVN_PAGE_C8_MSC_COMMAND_START_ADDR    , startbit       );
    return true;
}


bool_t     SiiMhlTxDrvSendCbusCommand ( cbus_req_t *pReq  )
{
    SetMscCmdInProgress
    switch(pReq->command)
    {
    case MHL_SET_INT:
    case MHL_WRITE_STAT:
        return SiiMhlTxDrvSendCbusCommand1Value(pReq->offsetData
                                , pReq->payload_u.msgData[0]
                                , MSC_START_BIT_WRITE_REG
                                );
    case MHL_READ_DEVCAP:
        return SiiMhlTxDrvSendCbusCommandNoValues(pReq->offsetData
                                , MSC_START_BIT_READ_REG
                                );
 	case MHL_GET_STATE:			// 0x62 -
	case MHL_GET_VENDOR_ID:		// 0x63 - for vendor id
	case MHL_SET_HPD:			// 0x64	- Set Hot Plug Detect in follower
	case MHL_CLR_HPD:			// 0x65	- Clear Hot Plug Detect in follower
	case MHL_GET_SC1_ERRORCODE:		// 0x69	- Get channel 1 command error code
	case MHL_GET_DDC_ERRORCODE:		// 0x6A	- Get DDC channel command error code.
	case MHL_GET_MSC_ERRORCODE:		// 0x6B	- Get MSC command error code.
	case MHL_GET_SC3_ERRORCODE:		// 0x6D	- Get channel 3 command error code.
        return SiiMhlTxDrvSendCbusCommandNoValues(pReq->command
                                , MSC_START_BIT_MSC_CMD
                                );
    case MHL_MSC_MSG:
        return SiiMhlTxDrvSendCbusCommand2Values(pReq->command
                                , pReq->payload_u.msgData[0]
                                , pReq->payload_u.msgData[1]
                                , MSC_START_BIT_VS_CMD
                                );
    case MHL_WRITE_BURST:
        return SiiMhlTxDrvSendCbusBurst(pReq->offsetData
                                , pReq->payload_u.pdatabytes
                                , pReq->length
                                );
    default:
        ClrMscCmdInProgress
        return false;
    }
}



void GatherTxEvents(TxPowerState_e txPowerState, uint8_t *pTpiIntsHandled, uint8_t *pIntr4IntsHandled)
{
PStateTableRowHeader_t   pStateHeader = &TxPowerStateTransitionAndResponseTable[txPowerState];

    if (pStateHeader->stateActionFlags & txsfExaminePinTxInt)
    {
		if (pinTxInt == 0)
		{

            *pIntr4IntsHandled = ExamineIntr4();
        }
    }

    if (pStateHeader->stateActionFlags & txsfCheckDeferRSEN_SamplingTimerExpired)
    {
        if (HalTimerExpired(TIMER_DEFER_RSEN_SAMPLING))
        {
            TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"DTX\n"));  // Deferral Timer Expired
            PutNextTxEvent(txpseDeferRSEN_SamplingTimerExpired);
        }
    }
    if (pStateHeader->stateActionFlags & txsfExamineCBUSIntrStatus)
    {
    uint8_t cBusInt;
        cBusInt = ReadByteCBUS(TX_WVN_PAGE_C8_CBUS_INTR_STATUS_ADDR);
    	if( cBusInt )
    	{
    		//
    		// Clear all interrupts that were raised even if we did not process
    		//
    		WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_INTR_STATUS_ADDR, cBusInt);

    	    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: Clear CBUS INTR_1: %02X\n", (int) cBusInt));
    	}

        if (settingMode9290 != pin9290_938x)
        {

            if ( MSC_ABORT & cBusInt)
            {
   				WriteByteCBUS(TX_WVN_PAGE_C8_MSC_REQUESTER_ABORT_REASON_ADDR, 0xFF);		// Clear interrupts
   				WriteByteCBUS(TX_WVN_PAGE_C8_MSC_RESPONDER_ABORT_REASON_ADDR, 0xFF);

   				WriteByteCBUS(TX_WVN_PAGE_C8_MSC_CMD_OR_OFFSET_ADDR, 0x62);		// Issue GET STATE
   				WriteByteCBUS(TX_WVN_PAGE_C8_MSC_COMMAND_START_ADDR, 0x01);

   				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"PVC Abort\n"));
            }

            if (cBusInt & MSC_MT_DONE)
            {
        		ClrMscCmdInProgress
                // only do this after cBusInt interrupts are cleared above
        		SiiMhlTxMscCommandDone( ReadByteCBUS( TX_WVN_PAGE_C8_MSC_MT_RCVD_DATA0_ADDR) );
            }
#ifdef SWWA_18958 //(
            if ( DDC_ABORT & cBusInt)
            {
            uint8_t bTemp,bPost;
   				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"\nDDC Abort\n\n"));

   				WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_NUM_DDC_ABORT_ADDR, 0xFF);

   				bTemp = ReadByteCBUS(TX_WVN_PAGE_C8_CBUS_NUM_DDC_ABORT_ADDR);
   				DelayMS(3); // leave this one
   				bPost = ReadByteCBUS(TX_WVN_PAGE_C8_CBUS_NUM_DDC_ABORT_ADDR);

   				if ((bPost > (bTemp + 50)))
   				{
   					TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Reset CBUS\n"));
   					WriteTxPage0Register( TXL_PAGE_0_SRST_ADDR, 0x08);				// Reset CBUS
   					WriteTxPage0Register( TXL_PAGE_0_SRST_ADDR, 0x00);
   					TXHAL_InitCBusRegs();

   					ForceUsbIdSwitchOpen();
   					ReleaseUsbIdSwitchOpen();

                   	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: disconnected\n"));
   					OnMHLCableDisconnected();
   				}
            }
#endif //)
        	// MSC_MSG (RCP/RAP)
        	if((TX_WVN_PAGE_C8_MSC_MR_MSC_MSG_MASK & cBusInt))
        	{
            uint8_t mscMsg[2];
        	    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: MSC_MSG Received\n"));
        		//
        		// Two bytes arrive at registers 0x18 and 0x19
        		//
                mscMsg[0] = ReadByteCBUS( 0x18 );
                mscMsg[1] = ReadByteCBUS( 0x19 );

        	    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: MSC MSG: %02X %02X\n", (int)mscMsg[0], (int)mscMsg[1] ));
        		SiiMhlTxGotMhlMscMsg( mscMsg[0], mscMsg[1] );
        	}

            if (pStateHeader->stateActionFlags & txsfSampleRSEN)
            {
            uint8_t RSEN;
            static uint8_t prevRSEN = SYS_STAT_RSEN;
            uint8_t sysStat;
                sysStat = ReadIndexedRegister(INDEXED_PAGE_0, TXL_PAGE_0_SYS_STAT_ADDR);
                RSEN = SYS_STAT_RSEN & sysStat;
                if (0 == RSEN)
                {
                    if (HalTimerExpired(TIMER_RSEN_DEGLITCH))
                    {
                        if (0 == prevRSEN)
                        {
                            TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"RSEN Low sysStat:0x%x\n",(unsigned int)sysStat));
                            PutNextTxEvent(txpseRSEN_SampledLow);
                        }
                        else
                        {
                            HalTimerSet(TIMER_RSEN_DEGLITCH,INTERVAL_RSEN_DEGLITCH);
                        }
                    }
                }
                else
                {
                    HalTimerSet(TIMER_RSEN_DEGLITCH,INTERVAL_RSEN_DEGLITCH);
                }
                prevRSEN = RSEN;
            }
        }
        {
        uint8_t cbusInt;

        	//
        	// Now look for interrupts on register 0x1E. CBUS_MSC_INT2
        	// 7:4 = Reserved
        	//   3 = msc_mr_write_state = We got a WRITE_STAT
        	//   2 = msc_mr_set_int. We got a SET_INT
        	//   1 = reserved
        	//   0 = msc_mr_write_burst. We received WRITE_BURST
        	//
        	cbusInt = ReadByteCBUS(TX_WVN_PAGE_C8_CBUS_MSC_INT2_ADDR);
        	if(cbusInt)
        	{
        		//
        		// Clear all interrupts that were raised even if we did not process
        		//
        		WriteByteCBUS(TX_WVN_PAGE_C8_CBUS_MSC_INT2_ADDR, cbusInt);

        	    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: Clear CBUS INTR_2: %02X\n", (int) cbusInt));
        	}
        	if(cbusInt & TX_WVN_PAGE_C8_MSC_MR_SET_INT_MASK)
        	{
            uint8_t intr[4];
            uint8_t i,address;

        		// We are interested only in first two bytes.

           		for(i=0,address = TX_WVN_PAGE_C8_REG_CBUS_INTR_SET_0_ADDR; address <= TX_WVN_PAGE_C8_REG_CBUS_INTR_SET_3_ADDR; address++,i++)
        		{
                    intr[i]= ReadByteCBUS( address );
        			// Clear all
        			WriteByteCBUS( address,  intr[i] );
        		}

        		SiiMhlTxGotMhlIntr( intr[0], intr[1] );
        	}
        	if ((cbusInt & TX_WVN_PAGE_C8_MSC_MR_WRITE_STATE_MASK) || HalTimerExpired(TIMER_WRITE_STAT))
        	{
            uint8_t status[4];
            int i, address;

        		// We are interested only in first two bytes.
                for (i = 0,address=TX_WVN_PAGE_C8_REG_CBUS_STATUS_0_ADDR
                        ; i < 4
                        ; ++i,++address)
                {
                    status[i] = ReadByteCBUS(address);
                    // clear as we go
                    WriteByteCBUS(address,0xFF /*future status[i]*/ );
                }
           		SiiMhlTxGotMhlStatus( status[0], status[1] );
                HalTimerSet(TIMER_WRITE_STAT,50);
        	}
        }
    }

    if (pStateHeader->stateActionFlags & txsfExamineTpiIntr)
    {
    uint8_t InterruptStatusImage;
    uint8_t intsHandled=0;

    	InterruptStatusImage = ReadByteTPI(PAGE_0_TPI_INTR_ST_ADDR);
		// Check if NMI has occurred
		if (InterruptStatusImage == NON_MASKABLE_INT)
		{

			TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TP -> NMI Detected\n"));
			/* Reset and re-initialize */
#if AST_HDMITX
			ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDMI_TX);
#endif
			TPI_Init();
			HotPlugService();
            intsHandled |= NON_MASKABLE_INT;
		}
		if (InterruptStatusImage & HOT_PLUG_EVENT_MASK)
		{

            *pIntr4IntsHandled = ExamineIntr4();
            intsHandled |= InterruptStatusImage & ~HOT_PLUG_EVENT_MASK;
		}

        // Check if Audio Error event has occurred:
        if (InterruptStatusImage & AUDIO_ERROR_EVENT)
        {
        	//TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"TP -> Audio Error Event\n"));
        	//  The hardware handles the event without need for host intervention (PR, p. 31)
            intsHandled |= AUDIO_ERROR_EVENT;
        }


        if (txsfCheckHDCPStatus & pStateHeader->stateActionFlags)
        {
            if (dsRxPoweredUp)
            {
           	    if (VideoModeDescription.HDCPAuthenticated == VMD_HDCP_AUTHENTICATED)
           		{
           			HDCP_CheckStatus(InterruptStatusImage);
           		}
            }
        }
        if (txsfCheckHDCPTimer & pStateHeader->stateActionFlags)
        {
            if (HalTimerExpired(TIMER_HDCP))
            {
        	    RestartHDCP();
            }
        }

        *pTpiIntsHandled = intsHandled;

    }
    if (pStateHeader->stateActionFlags & txsfProcessCBusEvents)
    {
        MhlTxDriveStates( );
#ifdef MSC_TESTER //{
    	start_msc_tester();
#endif //} MSC_TESTER
    }

    if (pStateHeader->stateActionFlags & txsfCheckMSCRequesterAbortHotPlug)
    {
    uint8_t abortReason;
		abortReason = ReadByteCBUS(TX_WVN_PAGE_C8_MSC_REQUESTER_ABORT_REASON_ADDR);
        if (downStreamHotPlugDetectionStatusChanged(abortReason))
		{
            SiiMhlTxNotifyDsHpdChange(abortReason);
			if (hdmiCableConnected(abortReason))
			{
                PutNextTxEvent(txpseHDMI_CableConnected);
			}
			else
			{
                // this only means that the downstream HDMI connection is gone
                // in the case of a dongle, MHL could still be connected

                PutNextTxEvent(txpseHDMI_CableDisconnected);
			}
		}
        mhlTxConfig.mscRequesterAbortReason = abortReason;

#if 1 //(
		if (!hdmiCableConnected(abortReason))
		{
			return;
		}
#endif //)

		CheckTxFifoStable();
    }
    if (pStateHeader->stateActionFlags & txsfCheckForVideoModeChange)
    {
        if ((tmdsPoweredUp) && (USRX_OutputChange))
        {
            TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,"TP -> Video Mode...\n"));

            SiiMhlTxDrvTmdsControl( false, HDCPIsAuthenticated);

         	if (HDCP_Started)
           	{
       	    	HDCP_Off();
           	}

    		HotPlugService();
    		ClrUSRX_OutputChange
      	}
    }
}

void TPI_Poll (void)
{

uint8_t tpiIntsHandled   =0;
uint8_t intr4IntsHandled =0;

TxPowerStateEvent_e event;
    /* New algorithm
    1) gather all events, executing state actions based upon flags stored in state row header
    2) lookup the event according to the current state.
    3) execute transition actions based upon transitionActionFlags set in the looked up state table entry.
    */
    GatherTxEvents(txPowerState,&tpiIntsHandled, &intr4IntsHandled);
    while (txpseNoOp != (event = GetNextTxEvent()))
    {
    PStateTableEntry_t psteTemp;
        psteTemp = LookupNextState(txPowerState, event);
        if (NULL == psteTemp)
        {
            TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"invalid event:%d for state: %d\n",(int)event,(int)txPowerState));
        }
        else
        {
        uint16_t transitionActionFlags;

            transitionActionFlags = psteTemp->transitionActionFlags;
            if (transitionActionFlags & txtfInitRGND_Ready)
            {
            uint8_t bTemp;
            	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"RGND Detection (D3)\n"));
            	WakeUpFromD3();
                bTemp = ReadTxPage0Register(SW_WVN_PAGE_0_DISC_STAT2_ADDR);
            	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"[99] -> %02X\n", (int)bTemp));

            	bTemp &= DISC_STAT2_RGND_MASK;
            	WriteTxPage0Register(SW_WVN_PAGE_0_DISC_CTRL1_ADDR
            	    , DISC_CTRL1_REG_DISC_CYC_20_ATTEMPTS
            	    | DISC_CTRL1_REG_DISC_CYCLE_100uS
            	    | DISC_CTRL1_REG_PON_STROBE_ACTIVE_LOW
            	    | DISC_CTRL1_REG_DISC_EN_ENABLE
            	    /*0x25*/);					// Force Power State to ON
                if (bTemp == DISC_STAT2_RGND_1K_OHM)
            	{
            		ReadModifyWriteTxPage0Register(SW_WVN_PAGE_0_DISC_CTRL6_ADDR, DISC_CTRL6_REG_DRV_FLT_SEL_MASK, DISC_CTRL6_REG_DRV_FLT_SEL_FLOAT);

                    // discovery pullup 10K ohm, IDLE pullup Open (OFF)
                    ReadModifyWriteTxPage0Register(SW_WVN_PAGE_0_DISC_CTRL4_ADDR
                        , DISC_CTRL4_REG_CBUSDISC_PUP_SEL_MASK | DISC_CTRL4_REG_CBUSIDLE_PUP_SEL_MASK
                        , DISC_CTRL4_REG_CBUSDISC_PUP_SEL_10K  | DISC_CTRL4_REG_CBUSIDLE_PUP_SEL_OFF
                        );

                    CALL_CBUS_WAKEUP_GENERATOR; // defined (optionally as nothing) in defs.h on a project by project basis.
                    // this would be a good place to set a timer and issue a state transition to an idle state for the wakeup-to-discovery interval.

            	}
            	else
            	{
                    TXHAL_DisablePullUpForD3();
            		ReadModifyWriteTxPage0Register(SW_WVN_PAGE_0_DISC_CTRL6_ADDR, DISC_CTRL6_REG_DRV_FLT_SEL_MASK, DISC_CTRL6_REG_DRV_FLT_SEL_USB);
            	}

            }
            if (transitionActionFlags & txtfMHL_Established)
            {
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"MHL Mode Est\n"));
				OnMHLCableConnected(); // this starts TPI Mode
            }
            if (transitionActionFlags & txtfSetDeferRSEN_SamplingTimer)
            {
                HalTimerSet(TIMER_DEFER_RSEN_SAMPLING,500);
                TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"calling TXHAL_InitCBusRegs\n"));
                TXHAL_InitCBusRegs( );
            }
            if (transitionActionFlags & txtfSetDeGlitchTimer)
            {
                HalTimerSet(TIMER_RSEN_DEGLITCH,INTERVAL_RSEN_DEGLITCH);
            }
            if (transitionActionFlags & txtfUSB_Established)
            {
				TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"USB Mode Established\n"));
				DelayMS(100); // TODO-break this up, this needs to be MUCH MUCH longer.  Perhaps 60 seconds. -evaluate during code merge.
				GoToD3 ();
            }
            if (transitionActionFlags & txtfCBUS_LockOut)
            {
                if (!bInTpiMode)
                {
                	InitForceUsbIdSwitchOpen();
                	InitReleaseUsbIdSwitchOpen();
                }
                else
                {
                	ForceUsbIdSwitchOpen();
                	ReleaseUsbIdSwitchOpen();
                }
            }
#ifndef EKKI_FPGA //(
            if (transitionActionFlags & txtfGoToD3)
            {
				GoToD3 ();
            }
#endif //)
            if (transitionActionFlags & txtfHDMICableConnected)
            {
				OnHdmiCableConnected();
            }
            if (transitionActionFlags & txtfHDMICableDisconnected)
            {
				OnHdmiCableDisconnected();
            }
            if (transitionActionFlags & txtfHDCPAuthenticated)
            {
                SetHDCPIsAuthenticated
                SiiMhlTxTmdsEnable(HDCPIsAuthenticated);
            }
            if (transitionActionFlags & txtfHDCPDeAuthenticated)
            {

            }
            TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"old state: %d new state: %d\n",(int)txPowerState,(int)psteTemp->newState));
            txPowerState = psteTemp->newState;
        }
    }

	if (intr4IntsHandled)
	{
    	WriteTxPage0Register( SW_WVN_PAGE_0_INTR4_ADDR, intr4IntsHandled);			// Clear interrupts that were handled
	}

    if (tpiIntsHandled)
    {
		WriteByteTPI (PAGE_0_TPI_INTR_ST_ADDR, tpiIntsHandled);	// Clear this interrupt.
    }
}
///////////////////////////////////////////////////////////////////////////////
//
// SiiMhlTxDrvPowBitChange
//
// This function or macro is invoked from MhlTx component to
// control the VBUS power. If powerOn is sent as non-zero, one should assume
// peer does not need power so quickly remove VBUS power.
//
// if value of "powerOn" is 0, then this routine must turn the VBUS power on
// within 50ms of this call to meet MHL specs timing.
//
void	SiiMhlTxDrvPowBitChange( bool_t powerOn )
{
	if( powerOn )
	{
		TPI_DEBUG_PRINT( (TPI_DEBUG_CHANNEL,"Drv: Peer's POW bit is set. Turn the VBUS power OFF here.\n"));
	}
	else
	{
		TPI_DEBUG_PRINT( (TPI_DEBUG_CHANNEL,"Drv:Peer's POW bit is cleared. Turn the VBUS power ON here.\n"));
	}
}


#endif  //}

