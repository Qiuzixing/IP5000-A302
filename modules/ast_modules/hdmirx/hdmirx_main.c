#include "ast_def.h"

#ifdef CONFIG_AST1500_CAT6023

#include <asm/arch/gpio.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/crt.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include "hdmirx_export.h"
#include "hdmirx.h"
#include "io.h"
#include "version.h"
#if SUPPORT_HDCP_REPEATER
#include <aspeed/hdcp1.h>
extern unsigned int NotifiedHDCPOn;
#endif

struct cdev *my_cdev = NULL;
static int dev_major = 251;
static int dev_minor = 0;

static int tCAT6023_init = 0;

static unsigned int always_check_int = 0;
static unsigned int enable_loopback = 0;

static struct workqueue_struct *wq = NULL;
static struct work_struct timerwork;
#if USE_INTERRUPT
static struct work_struct interruptwork;
#endif

extern Video_State_Type VState;
extern Audio_State_Type AState;
static Audio_Info pre_audio_info;

#define ENABLE_INT() do {gpio_cfg(GPIO_CAT6023_INT, AST1500_GPIO_INT_LEVEL_LOW);} while (0)
#define DISABLE_INT() \
do { \
	gpio_cfg(GPIO_CAT6023_INT, AST1500_GPIO_INT_DISABLE); \
	/* ack interrupt. Disabling GPIO int won't clear its status, \
	** so we need to do it by ourself. */ \
	gpio_ack_int_stat(GPIO_CAT6023_INT); \
} while (0)
#define POLL_INT_INIT() gpio_direction_input(GPIO_CAT6023_INT)
#define POLL_HAS_INT() ((gpio_get_value(GPIO_CAT6023_INT))?FALSE:TRUE)

int drv_param = 0;
module_param(drv_param, int, S_IRUGO);

#if USE_INTERRUPT
static irqreturn_t cat6023_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
	//check share interrupt
	if (!gpio_get_int_stat(GPIO_CAT6023_INT))
		return IRQ_NONE;

	ErrorF("In IRQ routine ....\n")

	DISABLE_INT();

	queue_work(wq, &interruptwork);

	return IRQ_HANDLED;
} /* cat6023_interrupt */

static void cat6023_interrupthandler(void)
{
	Check_HDMInterrupt();

	ENABLE_INT();
}
#endif

static void cat6023_timerhandler(unsigned long ptr)
{
#if !USE_INTERRUPT
	static unsigned int cnt = 0;
#endif

#if !USE_INTERRUPT
	if (POLL_HAS_INT() || always_check_int)
		Check_HDMInterrupt();
#endif

#if !USE_INTERRUPT
	if ((++cnt % TIMER_LOOP_COUNT) == 0)
#endif
	CheckHDMIRX();

#if USE_INTERRUPT
	queue_delayed_work(wq, &timerwork, msecs_to_jiffies(LOOP_MSEC));
#else
	queue_delayed_work(wq, &timerwork, msecs_to_jiffies(INT_LOOP_MSEC));
#endif
}

void UpdateVideoInfo(Video_State_Type state)
{
	printk("UpdateVideoInfo (%d)\n", state);

	if (state == VSTATE_VideoOn)
		vrx_video_event(1);
	else
		vrx_video_event(0);
}

void cat6613_setup_loopback_video(VIDEOPCLKLEVEL level, unsigned char bHDMI, unsigned char *pAVI_InfoFrame, unsigned char *pHDMI_InfoFrame);
void setup_loopback_video(void)
{
	struct s_crt_drv *crt = crt_get();
	VIDEOPCLKLEVEL level;
	unsigned char hdmi;
	unsigned char avi_infoframe[31];
	unsigned char hdmi_infoframe[31];
	unsigned char *pavi_infoframe = NULL;
	unsigned char *phdmi_infoframe = NULL;

	if (crt == NULL)
		return;

	if (!crt->tx_exist) {
		printk("no loopback Tx\n");
		return;
	}
	if (!enable_loopback) {
		printk("loopback disabled\n");
		return;
	}

	if (HDMIRX_ReadI2C_Byte(REG_RX_VID_XTALCNT_128PEL) > 0x2b)
		level = PCLK_LOW;
	else
		level = PCLK_HIGH;

	hdmi = IsHDMIRXHDMIMode();
	if (hdmi) {
		if (GetAVIInfoFrame(avi_infoframe))
			pavi_infoframe = avi_infoframe;
		if (GetVENDORSPECInfoFrame(hdmi_infoframe))
			phdmi_infoframe = hdmi_infoframe;
	}

	cat6613_setup_loopback_video(level, hdmi, pavi_infoframe, phdmi_infoframe);
}

static void setup_loopback_audio(Audio_Info *audio_info)
{
	struct s_crt_drv *crt = crt_get();

	if (crt == NULL)
		return;

	if (!crt->tx_exist)
		return;

	if (!enable_loopback) {
		printk("loopback disabled\n");
		return;
	}

	if (audio_info->Audio_On == 1) {
		int i;
		crt->audio_info.SampleFrequency = audio_info->SampleFrequency;
		crt->audio_info.ValidCh = audio_info->ValidCh;
		crt->audio_info.SampleSize = audio_info->SampleSize;
#if (API_VER >= 2)
		crt->audio_info.bSPDIF = 0;
		crt->audio_info.Audio_Type = audio_info->Audio_Type;
		for(i = 0; i < 5; i++)
			crt->audio_info.AUD_DB[i] = audio_info->AUD_DB[i];
		for(i = 0; i < 5; i++)
			crt->audio_info.ucIEC60958ChStat[i] = audio_info->ucIEC60958ChStat[i];
#endif

		printk("%02X,%d,%d\n",crt->audio_info.Audio_Type,crt->audio_info.SampleFrequency,crt->audio_info.ValidCh);
		crt_setup_audio(crt, OWNER_HOST);
	} else {
		/* Bruce101130.
		** Set both bAudioSampleFreq 0 && ChannelNumber to 0 means disable audio.
		*/
		crt->audio_info.SampleFrequency = 0;
		crt->audio_info.ValidCh = 0;
		crt->audio_info.SampleSize = 0;
#if (API_VER >= 2)
		crt->audio_info.bSPDIF = 0;
#endif
		crt_setup_audio(crt, OWNER_HOST);
	}
}

extern HDMI_VTiming s_CurrentVM ;
#if SUPPORT_HDCP_REPEATER
extern BYTE bHDCPMode;
#endif

static int video_timing_info(void *pIn)
{
	BYTE val;
	PVIDEO_MODE_INFO pModeInfo = (PVIDEO_MODE_INFO)pIn;

	memset(pModeInfo, 0, sizeof(VIDEO_MODE_INFO));

	pModeInfo->HActive = s_CurrentVM.HActive;
	pModeInfo->VActive = s_CurrentVM.VActive;
	pModeInfo->HTotal = s_CurrentVM.HTotal;
	pModeInfo->VTotal = s_CurrentVM.VTotal;
	pModeInfo->PCLK = s_CurrentVM.PCLK;
	pModeInfo->HFrontPorch = s_CurrentVM.HFrontPorch;
	pModeInfo->HSyncWidth = s_CurrentVM.HSyncWidth;
	pModeInfo->HBackPorch = s_CurrentVM.HBackPorch;
	pModeInfo->VFrontPorch = s_CurrentVM.VFrontPorch;
	pModeInfo->VSyncWidth = s_CurrentVM.VSyncWidth;
	pModeInfo->VBackPorch = s_CurrentVM.VBackPorch;

	if (s_CurrentVM.ScanMode == PROG)
		pModeInfo->ScanMode = Prog;
	else
		pModeInfo->ScanMode = Interl;

	if (s_CurrentVM.VPolarity == Vneg)
		pModeInfo->VPolarity = NegPolarity;
	else
		pModeInfo->VPolarity = PosPolarity;

	if (s_CurrentVM.HPolarity == Hneg)
		pModeInfo->HPolarity = NegPolarity;
	else
		pModeInfo->HPolarity = PosPolarity;

	if (IsHDMIRXHDMIMode()) {
		val = getHDMIRXAspectRatio();
		switch (val)
		{
		case B_AVI_PIC_ASRATIO_NODATA:
			pModeInfo->SignalType = s_HDMI_16x9; //Use 16x9 as default
			break;
		case B_AVI_PIC_ASRATIO_4_3:
			pModeInfo->SignalType = s_HDMI_4x3;
			break;
		case B_AVI_PIC_ASRATIO_16_9:
			pModeInfo->SignalType = s_HDMI_16x9;
			break;
		default:
			pModeInfo->SignalType = s_HDMI_16x9;
			break;
		}
	} else {
		pModeInfo->SignalType = s_DVI;
	}


	val = getHDMIRXInputColorMode();
	switch (val) {
	case F_MODE_RGB24:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_RGB;
		break;
	case F_MODE_YUV422:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV422;
		break;
	case F_MODE_YUV444:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV444;
		break;
	default: /* impossible */
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_RGB;
		break;
	}

	val = getHDMIRXOutputColorDepth();
	switch (val) {
	case 0://000b
		pModeInfo->ColorDepth = 0;//default 24 bits (non-deep color mode)
		break;
	case 4://100b
		pModeInfo->ColorDepth = 24;//8 * 3
		break;
	case 5://101b
		pModeInfo->ColorDepth = 30;//10 * 3
		break;
	case 6://110b
		pModeInfo->ColorDepth = 36;//12 * 3
		break;
	case 7://111b
		pModeInfo->ColorDepth = 48;//16 * 3
		break;
	default:
		pModeInfo->ColorDepth = 0;//8 * 3
	}
#if SUPPORT_HDCP_REPEATER
	if (bHDCPMode & HDCP_REPEATER) {
		//Video engine driver will query this information after HDCP authentication is done.
		pModeInfo->EnHDCP = HDCP_DISABLE;
	} else {
		pModeInfo->EnHDCP = (IsHDCPOn())?(HDCP_V_1X):(HDCP_DISABLE);
		NotifiedHDCPOn = pModeInfo->EnHDCP;
	}
#else
	pModeInfo->EnHDCP = (IsHDCPOn())?(HDCP_V_1X):(HDCP_DISABLE);
#endif

	pModeInfo->src_color_mode.is_dual_edge = 0;

	if ((COLOR_PIXEL_FORMAT_RGB != pModeInfo->src_color_mode.pixel_format) && (getHDMIRXInputColorimetry() & F_MODE_ITU709))
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_709;
	else
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_601;

	if (getHDMIRXInputQuantization() & F_MODE_16_235)
		pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_LIMITED;
	else
		pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_FULL;

	return 0;
}

static int video_avi_info(unsigned char *pData)
{
        return GetAVIInfoFrame(pData);
}

static int video_vsd_info(unsigned char *pData)
{
        return GetVENDORSPECInfoFrame(pData);
}

static int video_hdr_info(unsigned char *pData)
{
        /* TODO */
        return 0;
}

void patch_audio_info(Audio_Info *patched, Audio_Info *src)
{
	memcpy(patched, src, sizeof(Audio_Info));

	if (PARAM_AUDIO_FORCE_LPCM2CH48KHZ & drv_param) {
		//Force 2ch LPCM 48KHz audio
		patched->SampleFrequency = 2;
		patched->ValidCh = 2;
		patched->SampleSize = 24;
		patched->bSPDIF = 0;
		patched->Audio_Type = 0x80;

		patched->AUD_DB[0] = 0x01;
		patched->AUD_DB[1] = 0x00;
		patched->AUD_DB[2] = 0x00;
		patched->AUD_DB[3] = 0x00;
		patched->AUD_DB[4] = 0x00;

		patched->ucIEC60958ChStat[0] = 0x00;
		patched->ucIEC60958ChStat[1] = 0x02;
		patched->ucIEC60958ChStat[2] = 0x10;
		patched->ucIEC60958ChStat[3] = 0x02;
		patched->ucIEC60958ChStat[4] = 0xD2;
	}

	if (PARAM_AUDIO_ALWAYS_ON & drv_param) {
		//Always audio on
		patched->Audio_On = 1;
	}
}

void UpdateAudioInfo(BOOL ForeceUpdate)
{
	//Antony, merged newer code base from CAT to solve audio channel problem
	BYTE Freq, ValidCh, SampleSize, Audio_Type, auStat, chStat[5];
	BYTE AUD_DB[5];
	int i = 0;
	u32 n, cts;
	BOOL bUpdate = FALSE;

	printk("UpdateAudioInfo\n");
	if (ForeceUpdate)
		bUpdate = TRUE;

	if (AState == ASTATE_AudioOff) {
		if (pre_audio_info.Audio_On != 0) {
			bUpdate = TRUE;
			pre_audio_info.Audio_On = 0;
		}
	} else if (AState == ASTATE_AudioOn) {
		getHDMIRXAudioInfo(&Freq, &ValidCh, AUD_DB);
		if (Freq == B_Fs_HBR) {
			Freq = B_Fs_192KHz;
			ValidCh = 8;
		}
		if ((Freq != B_Fs_44p1KHz) &&
			(Freq != B_Fs_48KHz) &&
			(Freq != B_Fs_32KHz) &&
			(Freq != B_Fs_88p2KHz) &&
			(Freq != B_Fs_96KHz) &&
			(Freq != B_Fs_176p4KHz) &&
			(Freq != B_Fs_192KHz))
		{
			ErrorF("invalid sampling rate index (%d)\n", Freq);
			return;
		}

		if ((ValidCh != 2) && (ValidCh != 4) && (ValidCh != 6) && (ValidCh != 8)) {
			ErrorF("invalid channel number (%d)\n", ValidCh);
			return;
		}

		if (pre_audio_info.Audio_On != 1) {
			bUpdate = TRUE;
			pre_audio_info.Audio_On = 1;
		}

#if (API_VER < 2)
		if ((pre_audio_info.SampleFrequency != Freq) || (pre_audio_info.ValidCh != ValidCh)) {
			bUpdate = TRUE;
			pre_audio_info.SampleFrequency = Freq;
			pre_audio_info.ValidCh = ValidCh;
		}
#else
		Audio_Type  = getHDMIRXAudioStatus() & 0xF0;
		if (pre_audio_info.Audio_Type != Audio_Type) {
			bUpdate = TRUE;
			pre_audio_info.Audio_Type = Audio_Type;
		}

		if ((pre_audio_info.SampleFrequency != Freq) || (pre_audio_info.ValidCh != ValidCh) ||
		    (pre_audio_info.AUD_DB[0] != AUD_DB[0]) || (pre_audio_info.AUD_DB[1] != AUD_DB[1]) || (pre_audio_info.AUD_DB[2] != AUD_DB[2])||
		    (pre_audio_info.AUD_DB[3] != AUD_DB[3]) || (pre_audio_info.AUD_DB[4] != AUD_DB[4]))
		{
			bUpdate = TRUE;
			pre_audio_info.SampleFrequency = Freq;
			pre_audio_info.ValidCh = ValidCh;
			for (i=0;i<5;i++)
				pre_audio_info.AUD_DB[i] = AUD_DB[i];
		}
		getHDMIRXAudioChannelStatus(chStat);
		if ((pre_audio_info.ucIEC60958ChStat[0] != chStat[0]) || (pre_audio_info.ucIEC60958ChStat[1] != chStat[1]) || (pre_audio_info.ucIEC60958ChStat[2] != chStat[2])||
		    (pre_audio_info.ucIEC60958ChStat[3] != chStat[3]) || (pre_audio_info.ucIEC60958ChStat[4] != chStat[4]))
		{
			bUpdate = TRUE;
			for (i=0;i<5;i++)
				pre_audio_info.ucIEC60958ChStat[i] = chStat[i];
		}
		get_hdmi_rx_n(&n);
		get_hdmi_rx_cts(&cts);
#endif
	}

	if (bUpdate) {
		Audio_Info patched_audio_info;

		patch_audio_info(&patched_audio_info, &pre_audio_info);
		vrx_audio_event(patched_audio_info);
		setup_loopback_audio(&patched_audio_info);
	}
}

static void sync_audio(int force_update)
{
	UpdateAudioInfo(force_update);
}

static void video_update(void)
{
	UpdateVideoInfo(VState);
}

static void audio_update(void)
{
	UpdateAudioInfo(TRUE);
}


void UpdateInfoFrame(Video_State_Type state, BYTE type)
{
	printk("UpdateInfoFrame (%d)\n", state);

	if (state == VSTATE_VideoOn) {
		e_vrxhal_infoframe_type info_type = VRXHAL_VENDORSPEC_INFOFRAME_TYPE;

		switch (type) {
		case VENDORSPEC_INFOFRAME_TYPE:
			info_type = VRXHAL_VENDORSPEC_INFOFRAME_TYPE;
			break;
		case AVI_INFOFRAME_TYPE:
			info_type = VRXHAL_AVI_INFOFRAME_TYPE;
			break;
		case SPD_INFOFRAME_TYPE:
			info_type = VRXHAL_SPD_INFOFRAME_TYPE;
			break;
		case AUDIO_INFOFRAME_TYPE:
			info_type = VRXHAL_AUDIO_INFOFRAME_TYPE;
			break;
		case MPEG_INFOFRAME_TYPE:
			info_type = VRXHAL_MPEG_INFOFRAME_TYPE;
			break;
		default:
			BUG();
			break;
		};
		vrx_infoframe(info_type);
	}
}

static void DefaultUpdateInfoFrame(BYTE type)
{
	ErrorF("Default InfoFrame Callback\n");
}

static int dev_exist(void)
{
	return tCAT6023_init;
}

#if SUPPORT_HDCP_REPEATER
extern RXHDCP_State_Type	RHDCPState;

static void hdcp_mode(unsigned char repeater)
{
	hdmirx_clear_hpd();

	if (repeater)
		RxHDCPSetRepeater();
	else
		RxHDCPSetReceiver();

	hdmirx_set_hpd();
}

unsigned char Get_HDCP1_Upstream_Port_Encryption_Status(void)
{
	printk("Get_HDCP1_Upstream_Port_Encryption_Status\n");
	return ((unsigned char)(IsHDCPOn()));
}
EXPORT_SYMBOL(Get_HDCP1_Upstream_Port_Encryption_Status);

pfn_HDCP1_Upstream_Port_Auth_Request_Callback pHDCP1_Upstream_Port_Auth_Request_Callback = NULL;
void vrxhal_HDCP1_reg_upstream_port_auth_request_callback(pfn_HDCP1_Upstream_Port_Auth_Request_Callback pcallback)
{
	pHDCP1_Upstream_Port_Auth_Request_Callback = pcallback;
}
EXPORT_SYMBOL(vrxhal_HDCP1_reg_upstream_port_auth_request_callback);
void vrxhal_HDCP1_dereg_upstream_port_auth_request_callback(void)
{
	pHDCP1_Upstream_Port_Auth_Request_Callback = NULL;
}
EXPORT_SYMBOL(vrxhal_HDCP1_dereg_upstream_port_auth_request_callback);

pfn_HDCP1_Upstream_Port_Encryption_Status_Callback pHDCP1_Upstream_Port_Encryption_Status_Callback = NULL;
void Reg_HDCP1_Upstream_Port_Encryption_Status_Callback(pfn_HDCP1_Upstream_Port_Encryption_Status_Callback pcallback)
{
	pHDCP1_Upstream_Port_Encryption_Status_Callback = pcallback;
}
EXPORT_SYMBOL(Reg_HDCP1_Upstream_Port_Encryption_Status_Callback);
void Dereg_HDCP1_Upstream_Port_Encryption_Status_Callback(void)
{
	pHDCP1_Upstream_Port_Encryption_Status_Callback = NULL;
}
EXPORT_SYMBOL(Dereg_HDCP1_Upstream_Port_Encryption_Status_Callback);

#ifdef AST_HDMIRX
extern unsigned int HDCP_encryption_enabled;
#endif

static void hdcp_status(unsigned int state, unsigned short Bstatus, void *KSV_list)
{
	printk("vrxhal_HDCP1_set_upstream_port_auth_status(%u)\n", state);
	if (!(bHDCPMode & HDCP_REPEATER)) {
		printk("HDMI Rx not in REPEATER mode!!!\n");
		return;
	}
	//Do nothing if authentication failed.
	if (state != HDCP1_Repeater_Upstream_Port_State_Authenticated) {
		goto check;
	}

	if (RHDCPState == RXHDCP_AuthDone) {
		unsigned short	tmp;
		RxHDCPGetBstatus(&tmp);
		tmp &= ~(M_DOWNSTREAM_COUNT | B_DOWNSTREAM_OVER | M_REPEATER_DEPTH | B_MAX_CASCADE_EXCEEDED);//preserve Bstatus[12(HDMI_MODE),13(HDMI_RESERVED_2),14:15(Rsvd)] 
		Bstatus |= tmp;
		printk("Bstatus: %04X\n", Bstatus);
		setRxHDCPBStatus(Bstatus);
		if (state == HDCP1_Repeater_Upstream_Port_State_Authenticated) {
			unsigned int cnt = Bstatus & M_DOWNSTREAM_COUNT;
			if (cnt) {
				unsigned int i;
				unsigned char *buf = KSV_list;
				printk("KSV list:\n");
				for (i = 0; i < (5 * cnt); i++)
				{
					if ((i % 5) == 4)
						printk("%02X\n", buf[i]);
					else
						printk("%02X ", buf[i]);
				}
			}
			setRxHDCPKSVList(0, KSV_list, cnt);
			setRxHDCPCalcSHA();
		} else {
			//to do
			RxHDCPSetRepeater();
		}
	} else {
		printk("RHDCPState != RXHDCP_AuthDone\n");
	}

check:

#ifdef AST_HDMIRX
	{
		unsigned int enabled = IsEncryptionOn();
		//printk("Encryption status == %s\n", (enabled)?("ON"):("OFF"));
		if (!HDCP_encryption_enabled && enabled) {
			HDCP_encryption_enabled = TRUE;
		}
		else if (HDCP_encryption_enabled && !enabled) {
			HDCP_encryption_enabled = FALSE;
			if (NotifiedHDCPOn) {
				NotifiedHDCPOn = FALSE;
				if (pHDCP1_Upstream_Port_Auth_Request_Callback)
					pHDCP1_Upstream_Port_Auth_Request_Callback(NotifiedHDCPOn);
			}
		}
	}
#endif
}

#endif /* #ifdef SUPPORT_HDCP_REPEATER */

#if VRX_DBG
static ssize_t show_driver_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "VState = %d\nAState = %d\n", VState, AState);
	return strlen(buf);
}
static DEVICE_ATTR(driver_state, S_IRUGO, show_driver_state, NULL);

static ssize_t show_register(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int i;
	printk("HDMI Rx register dump:\n");
	printk("bank0\n");
	Switch_HDMIRX_Bank(0);

	for (i = 0; i < 0x100; i++) {
		if ((i & 0xF) == 0)
			printk("%02X: %02X ", i, HDMIRX_ReadI2C_Byte(i));
		else if ((i & 0xF) == 0xF)
			printk("%02X\n", HDMIRX_ReadI2C_Byte(i));
		else
			printk("%02X ", HDMIRX_ReadI2C_Byte(i));
	}

	printk("bank1\n");
	Switch_HDMIRX_Bank(1);
	for (i = 0x80; i < 0x100; i++) {
		if ((i & 0xF) == 0)
			printk("%02X: %02X ", i, HDMIRX_ReadI2C_Byte(i));
		else if ((i & 0xF) == 0xF)
			printk("%02X\n", HDMIRX_ReadI2C_Byte(i));
		else
			printk("%02X ", HDMIRX_ReadI2C_Byte(i));
	}
	Switch_HDMIRX_Bank(0);
	return 0;
}

static ssize_t store_register(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int bank, offset, value;
	unsigned int c;

	c = sscanf(buf, "%x %x %x", &bank, &offset, &value);
	if (c == 2) {
		if (bank == 0) {
			Switch_HDMIRX_Bank(0);
			value = HDMIRX_ReadI2C_Byte(offset);
		} else if (bank == 1) {
			Switch_HDMIRX_Bank(1);
			value = HDMIRX_ReadI2C_Byte(offset);
			Switch_HDMIRX_Bank(0);
		}
		printk("HDMI Rx bank%X register%02X is %02X.\n", bank, offset, value);
	} else if (c == 3) {
		printk("Set HDMI Rx bank%X register%02X to %02X.\n", bank, offset, value);
		if (bank == 0) {
			Switch_HDMIRX_Bank(0);
			HDMIRX_WriteI2C_Byte(offset, value);
		} else if (bank == 1) {
			Switch_HDMIRX_Bank(1);
			HDMIRX_WriteI2C_Byte(offset, value);
			Switch_HDMIRX_Bank(0);
		}
	} else {
		printk("Usage:\nBank Offset [Value]\n");
	}
	return count;
}
static DEVICE_ATTR(register, (S_IRUGO | S_IWUSR), show_register, store_register);

static ssize_t store_LoopbackEnable(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = crt_get();
	int enable;

	if (!crt->tx_exist) {
		printk("no loopback Tx!!!\n");
		goto done;
	}

	sscanf(buf, "%d", &enable);
	if (enable == 1) {
		printk("enable loopback\n");
	} else if (enable == 0) {
		printk("disable loopback\n");
	} else {
		printk("0 and 1 only please\n");
		goto done;
	}
	if (enable_loopback != enable)
		enable_loopback = enable;

done:
	return strlen(buf);
}
static DEVICE_ATTR(LoopbackEnable, S_IWUSR, NULL, store_LoopbackEnable);

#if SUPPORT_HDCP_REPEATER
static ssize_t show_hdcp_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (IsHDCPOn())
		sprintf(buf, "HDCP is ON.\n");
	else
		sprintf(buf, "HDCP is OFF.\n");

	if (bHDCPMode & HDCP_REPEATER) {
		sprintf(buf + strlen(buf), "HDMI Rx is in REPEATER mode.\n");
		sprintf(buf + strlen(buf), "HDMI Rx is in state %d\n", RHDCPState);
	}
	else
		sprintf(buf + strlen(buf), "HDMI Rx is in RECEIVER mode.\n");
    return strlen(buf);
}
static DEVICE_ATTR(hdcp_status, S_IRUGO, show_hdcp_status, NULL);
#endif


static struct attribute *dev_attrs[] = {
	&dev_attr_driver_state.attr,
	&dev_attr_register.attr,
	&dev_attr_LoopbackEnable.attr,
#if SUPPORT_HDCP_REPEATER
	&dev_attr_hdcp_status.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) "cat6023",
	.id = -1,
	.dev = {
		.release = the_pdev_release,
	},
};
#endif

static void HWReset_HDMIRX(void)
{
	gpio_direction_output(GPIO_CAT6023_RST, 0);
	delay1ms(100);
	gpio_direction_output(GPIO_CAT6023_RST, 1);
}

#if !SUPPORT_HDCP_REPEATER
extern BYTE bHDCPMode;
static void RxHDCPSetReceiver(void)
{
	PowerDownHDMI();
	bHDCPMode = HDCP_RECEIVER;
	InitHDMIRX(TRUE);
}
#endif

static void recver_info_init(void)
{
	struct recver_info recver;

	memset(&recver, 0, sizeof(struct recver_info));

	recver.chip = recver_chip_cat6023;
	recver.cap = recver_cap_none;
	recver.dev_exist = dev_exist;
	recver.sync_audio = sync_audio;
	recver.video_timing_info = video_timing_info;
	recver.video_avi_info = video_avi_info;
	recver.video_vsd_info = video_vsd_info;
	recver.video_hdr_info = video_hdr_info;
	recver.video_update = video_update;
	recver.audio_update = audio_update;
#if SUPPORT_HDCP_REPEATER
	recver.hdcp_mode = hdcp_mode;
	recver.hdcp_status = hdcp_status;
#endif
	recver.hdmi_n_cts = NULL;

	vrx_register_info(&recver);
}

static void recver_info_fini(void)
{
	struct recver_info recver;

	memset(&recver, 0, sizeof(struct recver_info));
	vrx_register_info(&recver);
}

#define SwitchHDMIRXBank(x) HDMIRX_WriteI2C_Byte(REG_RX_BANK,(x)&1)
#define I2C_CAT6023_SPEED 100000
#define CAT6023_DETECT_RETRY_LIMIT 3
int my_init(void)
{
	int result = 0;
	USHORT DeviceID;
	BYTE pData[2];
	dev_t dev = 0;
	int i;

	INFO("Load HDMIRX Driver %s\n", VERSION_STRING);

	HWReset_HDMIRX();

	/* I2C init */
	I2CInit(I2C_HDMI_RX, I2C_CAT6023_SPEED);

	//detect CAT6023
	for (i = 0; i < CAT6023_DETECT_RETRY_LIMIT; i++) {
		if (!SwitchHDMIRXBank(0))
			break;
	}

	if (i == CAT6023_DETECT_RETRY_LIMIT){
		INFO("HDMI_RX_I2C_SLAVE_ADDR access failed\n");
		result = -ENODEV;
		goto err;
	}

	/* Check ID */
	HDMIRX_ReadI2C_ByteN(0x02, pData, 2);
	DeviceID = *(USHORT *)(pData);
	if (DeviceID != 0x6023) {
		INFO("Can't find CAT6023\n");
		ErrorF("Get DeviceID:%x \n", DeviceID);
		result = -ENODEV;
		goto err;
	}

	INFO("CAT6023 found\n");


	/*
	 * Get a range of minor numbers to work with, asking for a dynamic
	 * major unless directed otherwise at load time.
	 */
	if (dev_major) {
		dev = MKDEV(dev_major, dev_minor);
		result = register_chrdev_region(dev, 1, "cat6023");
	} else {
		result = alloc_chrdev_region(&dev, dev_minor, 1, "cat6023");
		dev_major = MAJOR(dev);
	}

	if (result < 0) {
		ErrorF("Can't get major %d\n", dev_major);
		goto err;
	}

	/* Register Driver */
	my_cdev = cdev_alloc();
	if (!my_cdev) {
		ErrorF("Failed to allocate cdev?!\n");
		result = -ENOMEM;
		goto free_dev_num;
	}
	my_cdev->owner = THIS_MODULE;

	result = cdev_add(my_cdev, dev, 1);
	if (result < 0) {
		INFO("Can't get major.\n");
		goto free_cdev;
	}

#if USE_INTERRUPT
	/* request IRQ */
	DISABLE_INT();
	result = request_irq(20, &cat6023_interrupt, SA_SHIRQ, "cat6023", "cat6023");
	if (result) {
		INFO("Unable to get IRQ");
		goto free_cdev;
	}
#endif

	/* Init */
	hdmirx_clear_hpd();
	RxHDCPSetReceiver();
	hdmirx_set_hpd();

	/* timer handler init. */
	wq = create_singlethread_workqueue("cat6023_wq");
	if (!wq) {
		ErrorF("Failed to allocate wq?!\n");
		result = -ENOMEM;
		goto free_interrupt;
	}
	BUG_ON(wq == NULL);
	INIT_WORK(&timerwork, (void (*)(void *))cat6023_timerhandler, NULL);
#if USE_INTERRUPT
	INIT_WORK(&interruptwork, (void (*)(void *))cat6023_interrupthandler, NULL);
#endif
	queue_delayed_work(wq, &timerwork, msecs_to_jiffies(LOOP_MSEC));

#if VRX_DBG
	result = platform_device_register(&pdev);
	if (result < 0) {
		INFO("platform_driver_register err\n");
		goto free_timer;
	}

	result = sysfs_create_group(&pdev.dev.kobj, &dev_attr_group);
	if (result) {
		INFO("can't create sysfs files\n");
		goto free_pdev;
	}
#endif

#if USE_INTERRUPT
	/* init irq: GPIOD6 low-level trigger */
	ENABLE_INT();
#else
	POLL_INT_INIT();
#endif

	pre_audio_info.SampleSize = 24;

	recver_info_init();

	tCAT6023_init = 1;

	return 0;

//Error handling
free_pdev:
	platform_device_unregister(&pdev);
free_timer:
	if (wq) {
#if USE_INTERRUPT
		cancel_delayed_work(&interruptwork);
#endif
		cancel_delayed_work(&timerwork);
		flush_workqueue(wq);
		destroy_workqueue(wq);
		wq = NULL;
	}
free_interrupt:
#if USE_INTERRUPT
	free_irq(20, "cat6023");
#endif
free_cdev:
	cdev_del(my_cdev);
free_dev_num:
	unregister_chrdev_region(dev, 1);
err:
	//always return 0 so that insmod cat6023.ko won't fail even if CAT6023 is unavailable, since videoip.ko refers to a symbol from cat6023.ko
	return 0;
}

void my_cleanup(void)
{
	INFO("Unload HDMIRX Driver\n");

	if (tCAT6023_init) {

		tCAT6023_init = 0;

		recver_info_fini();

#if USE_INTERRUPT
		/* Disable interrupt */
		DISABLE_INT();
#endif

#if VRX_DBG
		sysfs_remove_group(&pdev.dev.kobj, &dev_attr_group);
		platform_device_unregister(&pdev);
#endif

		if (wq) {
#if USE_INTERRUPT
			cancel_delayed_work(&interruptwork);
#endif
			cancel_delayed_work(&timerwork);
			flush_workqueue(wq);
			destroy_workqueue(wq);
			wq = NULL;
		}

#if USE_INTERRUPT
		free_irq (20, "cat6023");
#endif

		cdev_del(my_cdev);
		unregister_chrdev_region(MKDEV(dev_major, dev_minor), 1);

	}
	return;
}

module_init (my_init);
module_exit (my_cleanup);

MODULE_LICENSE ("GPL");
#endif
