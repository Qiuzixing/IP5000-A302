/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#include "ast_main.h"
#include "ast_wrap.h"
#include "Mhlrx.h"
#include "Mhlrx_reg.h"

unsigned char hdmirxrd(unsigned char RegAddr);
void chgbank(int bank);
unsigned char AudioFsCal(void);

#define VIDEO_STATE_STR(s) \
({ \
	char *str; \
	switch(s) \
	{ \
		case VSTATE_Off: \
			str = "VSTATE_Off"; \
			break; \
		case VSTATE_TerminationOff: \
			str = "VSTATE_TerminationOff"; \
			break; \
		case VSTATE_TerminationOn: \
			str = "VSTATE_TerminationOn"; \
			break; \
		case VSTATE_5VOff: \
			str = "VSTATE_5VOff"; \
			break; \
		case VSTATE_SyncWait: \
			str = "VSTATE_SyncWait"; \
			break; \
		case VSTATE_SWReset: \
			str = "VSTATE_SWReset"; \
			break; \
		case VSTATE_SyncChecking: \
			str = "VSTATE_SyncChecking"; \
			break; \
		case VSTATE_HDCPSet: \
			str = "VSTATE_HDCPSet"; \
			break; \
		case VSTATE_HDCP_Reset: \
			str = "VSTATE_HDCP_Reset"; \
			break; \
		case VSTATE_ModeDetecting: \
			str = "VSTATE_ModeDetecting"; \
			break; \
		case VSTATE_VideoOn: \
			str = "VSTATE_VideoOn"; \
			break; \
		case VSTATE_ColorDetectReset: \
			str = "VSTATE_ColorDetectReset"; \
			break; \
		case VSTATE_HDMI_OFF: \
			str = "VSTATE_HDMI_OFF"; \
			break; \
		case VSTATE_Reserved: \
			str = "VSTATE_Reserved"; \
			break; \
		default: \
			str = "? Unknown"; \
			break; \
	} \
	str; \
})


int is_dev_exists(void)
{
	return it6802_identify_chip();
}

int ast_dev_init(struct it680x_drv_data *d)
{
	//xxxxx 2013-0801
	it6802HPDCtrl(1,0);	// HDMI port , set HPD = 0
	//xxxxx

	//It took so long here. Do we really need it?
	//delay1ms(1000);	//for power sequence

	d->it6802data = IT6802_fsm_init(d);

	//xxxxx 2013-0801
	it6802HPDCtrl(1,1);	// HDMI port , set HPD = 1
	//xxxxx

	return 0;
}

void ast_fsm_timer_handler(struct it680x_drv_data *d)
{
	down(&d->reg_lock);
	IT6802_fsm();
	up(&d->reg_lock);
}

#if (INTR_MODE == 1)
void ast_intr_handler(struct it680x_drv_data *d)
{
	/* interrupt mode is CEC only */
	it6802_cec_intr_handler();
}
#endif

void ast_notify_video_state_change(void *drv_context)
{
	struct it680x_drv_data *d = (struct it680x_drv_data *)drv_context;

	if (IsVideoOn() && d->it6802data)
		vrx_video_event(1);
	else
		vrx_video_event(0);

}

int ast_get_video_timing_info(struct it680x_drv_data *d, void *pIn)
{
	struct it6802_dev_data *it6802 = (struct it6802_dev_data *)d->it6802data;
	PVIDEO_MODE_INFO pModeInfo = (PVIDEO_MODE_INFO)pIn;
	VTiming *CurVTiming = it6802->CurVTiming;

	memset(pModeInfo, 0, sizeof(VIDEO_MODE_INFO));

	down(&d->reg_lock);

	pModeInfo->HActive = CurVTiming->HActive;
	pModeInfo->VActive = CurVTiming->VActive;
	pModeInfo->HTotal = CurVTiming->HTotal;
	pModeInfo->VTotal = CurVTiming->VTotal;
	pModeInfo->PCLK = CurVTiming->PCLK;
	pModeInfo->HFrontPorch = CurVTiming->HFrontPorch;
	pModeInfo->HSyncWidth = CurVTiming->HSyncWidth;
	pModeInfo->HBackPorch = CurVTiming->HBackPorch;
	pModeInfo->VFrontPorch = CurVTiming->VFrontPorch;
	pModeInfo->VSyncWidth = CurVTiming->VSyncWidth;
	pModeInfo->VBackPorch = CurVTiming->VBackPorch;

	if (CurVTiming->ScanMode == 0)
		pModeInfo->ScanMode = Prog;
	else
		pModeInfo->ScanMode = Interl;

	if (CurVTiming->VPolarity == 0)
		pModeInfo->VPolarity = NegPolarity;
	else
		pModeInfo->VPolarity = PosPolarity;

	if (CurVTiming->HPolarity == 0)
		pModeInfo->HPolarity = NegPolarity;
	else
		pModeInfo->HPolarity = PosPolarity;

	if (it6802->m_bUpHDMIMode) {
		switch (it6802->AviPicAsRatio)
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

	switch (it6802->GCPColorDepth)
	{
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
		//GCP's Color depth is actually 4bits long, but spec only uses 3bits.
		pModeInfo->ColorDepth = 0;//8 * 3
	}
#if 0//BruceToDo. SUPPORT_HDCP_REPEATER
	if (bHDCPMode & HDCP_REPEATER) {
		//Video engine driver will query this information after HDCP authentication is done.
		pModeInfo->EnHDCP = 0;
	} else {
		pModeInfo->EnHDCP = IsHDCPOn();
		NotifiedHDCPOn = pModeInfo->EnHDCP;
	}
#else
	it6802->last_hdcp_status = sys_hdcp_on();
	pModeInfo->EnHDCP = it6802->last_hdcp_status;
#endif

	switch (it6802->m_bInputVideoMode & F_MODE_CLRMOD_MASK) {
	case F_MODE_RGB444:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_RGB;
		break;
	case F_MODE_YUV422:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV422;
		break;
	case F_MODE_YUV444:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV444;
		break;
	case F_MODE_YUV420:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV420;
		break;
	default: /* impossible */
		BUG();
	}

	pModeInfo->src_color_mode.is_dual_edge = 0;
	switch(it6802->m_VidOutDataTrgger)
	{
		case eSDR:
			//default mode
			break;
		case eHalfPCLKDDR:
			pModeInfo->src_color_mode.is_dual_edge = 1;
			break;
		case eHalfBusDDR:
			//pass through
		case eSDR_BTA1004:
			//pass through
		case eDDR_BTA1004:
			//pass through
		default:
			HDMIRX_VIDEO_PRINTF(("Unsupported m_VidOutDataTrgger %d?\n", it6802->m_VidOutDataTrgger));
			BUG();
			break;
	}

	if ((COLOR_PIXEL_FORMAT_RGB != pModeInfo->src_color_mode.pixel_format) && (it6802->m_bInputVideoMode & F_MODE_ITU709))
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_709;
	else
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_601;

	if (it6802->m_bInputVideoMode & F_MODE_16_235)
		pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_LIMITED;
	else
		pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_FULL;

	up(&d->reg_lock);

	return 0;
}


int _get_avi_info(struct it680x_drv_data *d, unsigned char *pData)
{
	int check = 1, retry = 0, retry_limit = 5;

	/* FIXME
	 * disable m_NewAVIInfoFrameF check temporarily.
	 * we have to review fsm code to determine when we should set/clear m_NewAVIInfoFrameF.
	 */
#if 0
	if (it6802->m_NewAVIInfoFrameF != TRUE) {
		printk("<%s> it6802->m_NewAVIInfoFrameF is not TRUE.\n", __func__);
		return 0; //means not valid
	}
#endif

	if (pData == NULL) {
		return 1; //means I don't mind?!. Copied from it6604 driver.
	}

start:
	chgbank(2);

	pData[0] = AVI_INFOFRAME_TYPE; //0x82
	pData[1] = hdmirxrd(REG_RX_AVI_VER);
	pData[2] = hdmirxrd(REG_RX_AVI_LENGTH) & 0x1F;

	pData[3] = hdmirxrd(REG_RX_AVI_DB0);
	pData[4] = hdmirxrd(REG_RX_AVI_DB1);
	pData[5] = hdmirxrd(REG_RX_AVI_DB2);
	pData[6] = hdmirxrd(REG_RX_AVI_DB3);
	pData[7] = hdmirxrd(REG_RX_AVI_DB4);
	pData[8] = hdmirxrd(REG_RX_AVI_DB5);
	pData[9] = hdmirxrd(REG_RX_AVI_DB6);
	pData[10] = hdmirxrd(REG_RX_AVI_DB7);
	pData[11] = hdmirxrd(REG_RX_AVI_DB8);
	pData[12] = hdmirxrd(REG_RX_AVI_DB9);
	pData[13] = hdmirxrd(REG_RX_AVI_DB10);
	pData[14] = hdmirxrd(REG_RX_AVI_DB11);
	pData[15] = hdmirxrd(REG_RX_AVI_DB12);
	pData[16] = hdmirxrd(REG_RX_AVI_DB13);
	//Extra registers.
	//pData[17] = hdmirxrd(REG_RX_AVI_DB14);
	//pData[18] = hdmirxrd(REG_RX_AVI_DB15);

	chgbank(0);

	if (check) {
		int i;
		u8 sum = 0;

		for (i = 0; i <= 16; i++)
			sum += pData[i];

		if (sum != 0) {
			retry++;
			if (retry < retry_limit)
				goto start;
		}
	}

	return 1;
}

int ast_get_avi_info(struct it680x_drv_data *d, unsigned char *pData)
{
	struct it6802_dev_data *it6802 = (struct it6802_dev_data *)d->it6802data;
	int ret = 0;

	down(&d->reg_lock);

	ret = _get_avi_info(d, pData);

	it6802->m_NewAVIInfoFrameF = FALSE;

	up(&d->reg_lock);

	return ret;
}

int ast_get_vsd_info(struct it680x_drv_data *d, unsigned char *pData)
{
	if (pData == NULL) {
		return 0;
	}

	down(&d->reg_lock);

	chgbank(2);

	pData[0] = hdmirxrd(REG_RX_224); // Should be VENDORSPEC_INFOFRAME_TYPE 0x81
	pData[1] = hdmirxrd(REG_RX_225);
	pData[2] = hdmirxrd(REG_RX_226) & 0x1F;

	pData[3] = hdmirxrd(REG_RX_227);
	pData[4] = hdmirxrd(REG_RX_228);
	pData[5] = hdmirxrd(REG_RX_229);
	pData[6] = hdmirxrd(REG_RX_22A);
	pData[7] = hdmirxrd(REG_RX_22B);
	pData[8] = hdmirxrd(REG_RX_22C);
	pData[9] = hdmirxrd(REG_RX_22D);
	pData[10] = hdmirxrd(REG_RX_22E);

	chgbank(0);

	up(&d->reg_lock);

	if (VENDORSPEC_INFOFRAME_TYPE != pData[0])
		return 0;

#if 0
	{
		unsigned int i;
		printk("<%s>VSD InfoFrame:\n", __func__);
		for (i = 0; i < 32; i++)
			printk("%02X ", pData[i]);
		printk("\n");
	}
#endif

	return 1;
}


int _get_hdr_info(struct it680x_drv_data *d, unsigned char *pData)
{
	u32 i;

	if (NULL == pData)
		return 0;

	chgbank(2);

	for (i = 0; i < HDR_INFOFRAME_LEN; i++)
		pData[i] = hdmirxrd(REG_RX_253 + i);

	chgbank(0);

	if (HDR_INFOFRAME_TYPE != pData[0])
		return 0;

	return 1;
}

int ast_get_hdr_info(struct it680x_drv_data *d, unsigned char *pData)
{
	struct it6802_dev_data *it6802 = (struct it6802_dev_data *)d->it6802data;
	int ret;

	if (it6802->hdr_info_shadow.raw.data[0] != HDR_INFOFRAME_TYPE)
		return 0;

	down(&d->reg_lock);

	ret = _get_hdr_info(d, pData);

	up(&d->reg_lock);

	return ret;
}

static u8 ast_audio_type_by_audio_caps(AUDIO_CAPS *ac)
{
	u8 flag, ret = 0;

	flag = ac->AudioFlag; /* AudioFlag is set by getHDMIRXInputAudio() */

	if (flag & B_CAP_AUDIO_ON) {
		ret |= AST_AUDIO_ON;

		/* HBR */
		if (flag & B_CAP_HBR_AUDIO)
			ret |= AST_AUDIO_HBR;

		/* DSD */
		if (flag & B_CAP_DSD_AUDIO)
			ret |= AST_AUDIO_DSD;

		/* NLPCM */
		if (!(flag & (B_CAP_HBR_AUDIO | B_CAP_DSD_AUDIO | B_CAP_LPCM)))
			ret |= AST_AUDIO_NLPCM;
	}

	return ret;
}

static void _get_audio_info(struct it680x_drv_data *d, Audio_Info *ai)
{
	struct it6802_dev_data *it6802 = (struct it6802_dev_data *)d->it6802data;
	AUDIO_CAPS *ac;

	memset(ai, 0, sizeof(Audio_Info));

	if (it6802->m_AState != ASTATE_AudioOn)
		return;

	ac = &it6802->m_RxAudioCaps;

	ai->Audio_On = 1;
	ai->Audio_Type = ast_audio_type_by_audio_caps(ac);
	ai->SampleFrequency = ac->SampleFreq;
	if (ac->AudioFlag & B_AUDIO_LAYOUT) {
		/* For multi-ch layout */
		if (ac->AudSrcEnable & B_AUDIO_SRC_VALID_3)
			ai->ValidCh = 8;
		else if (ac->AudSrcEnable & B_AUDIO_SRC_VALID_2)
			ai->ValidCh = 6;
		else if (ac->AudSrcEnable & B_AUDIO_SRC_VALID_1)
			ai->ValidCh = 4;
		else
			ai->ValidCh = 2;
	} else {
		/* For 2Ch layout */
		ai->ValidCh = 2;
	}

	chgbank(2);
	ai->AUD_DB[0] = hdmirxrd(REG_RX_245);
	ai->AUD_DB[1] = hdmirxrd(REG_RX_246);
	ai->AUD_DB[2] = hdmirxrd(REG_RX_247);
	ai->AUD_DB[3] = hdmirxrd(REG_RX_248);
	ai->AUD_DB[4] = hdmirxrd(REG_RX_249);

#if 0
	ai->hdmi_n = (unsigned long) (hdmirxrd(REG_RX_2C0) & 0x0F);
	ai->hdmi_n += (unsigned long) (hdmirxrd(REG_RX_2BF) << 4);
	ai->hdmi_n += (unsigned long) (hdmirxrd(REG_RX_2BE) << 12);
	ai->hdmi_cts = (unsigned long) ((hdmirxrd(REG_RX_2C0) & 0xF0) >> 4);
	ai->hdmi_cts += (unsigned long) (hdmirxrd(REG_RX_2C2) << 4);
	ai->hdmi_cts += (unsigned long) (hdmirxrd(REG_RX_2C1) << 12);
#endif
	chgbank(0);

	ai->ucIEC60958ChStat[0] = hdmirxrd(REG_RX_AUD_CHSTAT0);
	ai->ucIEC60958ChStat[1] = hdmirxrd(REG_RX_AUD_CHSTAT1);
	ai->ucIEC60958ChStat[2] = hdmirxrd(REG_RX_AUD_CHSTAT2);
	ai->ucIEC60958ChStat[3] = hdmirxrd(REG_RX_AUD_CHSTAT3);
	ai->ucIEC60958ChStat[4] = hdmirxrd(REG_RX_AUD_CHSTAT4);
	/*
	 * the sample_rate_table in i2s/driver/i2s_main.c has following items,
	 *    44.1KHz, 48KHz, 32KHz, 88.2KHz, 96KHz, 176.4KHz, 192KHz
	 *  We have to check sampling frequency before passing to I2S
	 */
	switch (ai->SampleFrequency) {
	case B_FS_44100: case B_FS_48000: case B_FS_32000:
	case B_FS_88200: case B_FS_96000: case B_FS_176400:
	case B_FS_192000:
		/* exist in I2S sample rate table */
		break;
#ifdef _HBR_I2S_
	case B_FS_768000:
		ai->SampleFrequency = B_FS_192000;
		ai->ValidCh = 8;
		break;
#endif
	default:
		{
			unsigned char forced_freq;
			/*
			** Bruce150106. When SampleFrequency is not valid, we try to get SampleFrequency
			** from AudioFsCal(). If AudioFsCal() can't find a proper value, then
			** Give it a default value (B_FS_48000).
			** We don't use the value from ChStat because the value may be invalid
			** and kill I2S driver.
			*/
			forced_freq = AudioFsCal();
			if (forced_freq == B_FS_NOTID)
				forced_freq = B_FS_48000;

			ai->SampleFrequency = forced_freq;
		}
	}
}

int ast_get_hdmi_n_cts(struct it680x_drv_data *d, unsigned int *pn, unsigned int *pcts)
{
	struct it6802_dev_data *it6802 = (struct it6802_dev_data *)d->it6802data;

	*pn = it6802->hdmi_n;
	*pcts = it6802->hdmi_cts;

	return 0;
}

static unsigned int enable_loopback = 1;

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

static void patch_audio_info(Audio_Info *patched, Audio_Info *src)
{
	memcpy(patched, src, sizeof(Audio_Info));

	if (PARAM_AUDIO_FORCE_LPCM2CH48KHZ & drv_param) {
		/* Force 2ch LPCM 48KHz audio */
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

	/*
	** Bruce150626. PARAM_AUDIO_ALWAYS_ON is an individual setting for the case:
	** case: force ON, but don't force 48KHz.
	** case: force ON, but uses other sample frequency like 192KHz
	** case: force 48KHz, but don't always force ON.
	*/
	if (PARAM_AUDIO_ALWAYS_ON & drv_param) {
		/* Always audio on */
		patched->Audio_On = 1;
	}
}

static void ast_audio_info_update(struct it680x_drv_data *d)
{
	Audio_Info *audio_info = &d->audio_info;
	unsigned int chged = 0;
	unsigned int i;
	Audio_Info nai, *new_audio_info = &nai;

	down(&d->reg_lock);

	_get_audio_info(d, new_audio_info);

	do {
		if (audio_info->Audio_On != new_audio_info->Audio_On) {
			chged = 1;
			break;
		}

		if (!new_audio_info->Audio_On)
			break;

		if (audio_info->Audio_Type != new_audio_info->Audio_Type) {
			chged = 1;
			break;
		}

		if ((audio_info->SampleFrequency != new_audio_info->SampleFrequency)
		 || (audio_info->ValidCh != new_audio_info->ValidCh)
		 || (audio_info->AUD_DB[0] != new_audio_info->AUD_DB[0])
		 || (audio_info->AUD_DB[1] != new_audio_info->AUD_DB[1])
		 || (audio_info->AUD_DB[2] != new_audio_info->AUD_DB[2])
		 || (audio_info->AUD_DB[3] != new_audio_info->AUD_DB[3])
		 || (audio_info->AUD_DB[4] != new_audio_info->AUD_DB[4]))
		{
			chged = 1;
			break;
		}

		if ((audio_info->ucIEC60958ChStat[0] != new_audio_info->ucIEC60958ChStat[0])
		 || (audio_info->ucIEC60958ChStat[1] != new_audio_info->ucIEC60958ChStat[1])
		 || (audio_info->ucIEC60958ChStat[2] != new_audio_info->ucIEC60958ChStat[2])
		 || (audio_info->ucIEC60958ChStat[3] != new_audio_info->ucIEC60958ChStat[3])
		 || (audio_info->ucIEC60958ChStat[4] != new_audio_info->ucIEC60958ChStat[4]))
		{
			chged = 1;
			break;
		}
	} while (0);

	if (chged) {
		audio_info->Audio_On = new_audio_info->Audio_On;
		if (audio_info->Audio_On) {
			audio_info->Audio_Type = new_audio_info->Audio_Type;
			audio_info->SampleFrequency = new_audio_info->SampleFrequency;
			audio_info->ValidCh = new_audio_info->ValidCh;
			for (i = 0; i < 5; i++)
				audio_info->AUD_DB[i] = new_audio_info->AUD_DB[i];

			for (i = 0; i < 5; i++)
				audio_info->ucIEC60958ChStat[i] = new_audio_info->ucIEC60958ChStat[i];
		}
	}

	up(&d->reg_lock);

	if (chged || d->force_update_audio_info) {
		Audio_Info patched_audio_info;

		uinfo("Notify audio info.\n");
		patch_audio_info(&patched_audio_info, audio_info);
		setup_loopback_audio(&patched_audio_info);
		d->force_update_audio_info = 0;
		//(d->audio_event_callback)(patched_audio_info);
		vrx_audio_event(patched_audio_info);
	}
}

void ast_schedule_audio_info_update(struct it680x_drv_data *d, int force_update)
{
	//flush_workqueue(d->wq);
	/*
	** Bruce 'think' there is no race condition modifying d->force_update_audio_info here.
	** It is true when we ALWAYS call ast_audio_info_update() through ast_schedule_audio_info_update().
	*/
	d->force_update_audio_info = force_update;
	PREPARE_WORK(&d->audio_update_work, (void (*)(void *))ast_audio_info_update, d);
	queue_work(d->wq, &d->audio_update_work);
}


