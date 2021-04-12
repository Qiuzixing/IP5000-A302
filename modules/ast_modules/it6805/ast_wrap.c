/*
 * Copyright (c) 2019 ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "ast_it6805.h"
#include "ast_wrap.h"

IT6805_DEV_DATA	iTE6805_DATA;
IT6805_VTIMING	iTE6805_CurVTiming;

unsigned char hdmirxrd(unsigned char RegAddr);
unsigned char AudioFsCal(void);

int is_dev_exists(void)
{
	return IT6805_IDENTIFY_CHIP();
}

int ast_dev_init(struct it680x_drv_data *d)
{
	d->dev_data = &iTE6805_DATA;
	iTE6805_DATA.drv_data = d;

	if (drv_param & PARAM_DISABLE_HDCP)
		iTE6805_DATA.STATE_HDCP = 0;
	else
		iTE6805_DATA.STATE_HDCP = 1;

	IT6805_FSM_INIT();

#if (AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ == 1)
	ast_it6805_down_scale_4k60_over_300mhz_cfg(1);
#else
	ast_it6805_down_scale_4k60_over_300mhz_cfg(0);
#endif

	return 0;
}

void ast_fsm_timer_handler(struct it680x_drv_data *d)
{
	down(&d->reg_lock);
	IT6805_FSM();
	up(&d->reg_lock);
}

#if (AST_IT6805_INTR == 1)
void ast_intr_handler(struct it680x_drv_data *d)
{
	/* interrupt mode is CEC only */
//	it6802_cec_intr_handler();
	iTE6805_hdmirx_CEC_irq();

}
#endif

void ast_notify_video_state_change(void *drv_context)
{
	struct it680x_drv_data *d = (struct it680x_drv_data *)drv_context;

	if (is_video_on())
		vrx_video_event(1);
	else
		vrx_video_event(0);
}

int ast_get_video_timing_info(struct it680x_drv_data *d, void *pIn)
{
	IT6805_DEV_DATA *dev_data = (IT6805_DEV_DATA *) d->dev_data;
	PVIDEO_MODE_INFO pModeInfo = (PVIDEO_MODE_INFO)pIn;
	IT6805_VTIMING *CurVTiming = &iTE6805_CurVTiming;

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

	if (dev_data->Flag_InputMode == MODE_HDMI) {
		switch (dev_data->avi_infoframe_picture_aspect_ratio) {
		case 1: /* 4:3 */
			pModeInfo->SignalType = s_HDMI_4x3;
			break;
		case 0: /* no data */
		case 2: /* 16:9 */
		default:
			pModeInfo->SignalType = s_HDMI_16x9;
			break;

		}
	} else
		pModeInfo->SignalType = s_DVI;

	switch (CurVTiming->ColorDepth) {
	case 0x00:
	case 0x40:
		pModeInfo->ColorDepth = 0; /* default 24 bits (non-deep color mode) */
		break;
	case 0x50:
		pModeInfo->ColorDepth = 30; /* 10 * 3 */
		break;
	case 0x60:
		pModeInfo->ColorDepth = 36; /* 12 * 3 */
		break;
	default:
		pModeInfo->ColorDepth = 0; /* default 24 bits (non-deep color mode) */
		break;
	}

	pModeInfo->EnHDCP = is_hdcp_on();

	switch (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat) {
	case Color_Format_RGB:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_RGB;
		break;
	case Color_Format_YUV422:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV422;
		break;
	case Color_Format_YUV444:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV444;
		break;
	case Color_Format_YUV420:
		pModeInfo->src_color_mode.pixel_format = COLOR_PIXEL_FORMAT_YUV420;
		break;
	}

	pModeInfo->src_color_mode.is_dual_edge = 0;
	switch (iTE6805_DATA.US_Video_Out_Data_Path) {
	case eTTL_DDR: case eTTL_DDR_BTA1004:
		pModeInfo->src_color_mode.is_dual_edge = 1;
		break;
	default:
		break;
	}

	if ((COLOR_PIXEL_FORMAT_RGB != pModeInfo->src_color_mode.pixel_format) && (iTE6805_DATA.AVIInfoFrame_Colorimetry == Colormetry_ITU709))
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_709;
	else
		pModeInfo->src_color_mode.colorimetry = COLOR_COLORIMETRY_BT_601;

	/* quantization range is needed by client. */
	if (iTE6805_DATA.AVIInfoFrame_Input_ColorFormat == Color_Format_RGB) {
		if (iTE6805_DATA.AVIInfoFrame_RGBQuantizationRange == RGB_RANGE_LIMIT)
			pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_LIMITED;
		else
			pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_FULL;
	} else {
		if (iTE6805_DATA.AVIInfoFrame_YUVQuantizationRange == YUV_RANGE_LIMIT)
			pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_LIMITED;
		else
			pModeInfo->src_color_mode.quantization = COLOR_QUANTIZATION_FULL;
	}

	up(&d->reg_lock);

	return 0;
}

static int _get_avi_info(struct it680x_drv_data *d, unsigned char *pData)
{
	IT6805_DEV_DATA	*dev_data = (IT6805_DEV_DATA *) d->dev_data;
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

	pData[0] = HDMI_INFOFRAME_TYPE_AVI;
	pData[1] = hdmirxrd(REG_RX_AVI_VER);
	pData[2] = hdmirxrd(REG_RX_AVI_LENGTH);

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

#if (AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ_AVI_INFO_FIXUP == 1)
	if (dev_data->_iTE6805_4K_Mode_ == MODE_DownScale) {
		union hdmi_avi_any_infoframe *frame;
		u8 fix_vic = 0;

		frame = (union hdmi_avi_any_infoframe *) pData;

		switch (frame->avi.video_code) {
		case VIC_2160p50: /* 4K 50Hz */
			fix_vic = VIC_1080p50;
			break;
		case VIC_2160p: /* 4K 60Hz */
			fix_vic = VIC_1080p;
			break;
		default:
			break;
		}

		if (fix_vic) {
			int i;

			frame->avi.video_code = fix_vic;
			frame->avi.bar_info = 0;
			frame->avi.top_bar = 0;
			frame->avi.bottom_bar = 0;
			frame->avi.left_bar = 0;
			frame->avi.right_bar = 0;

			pData[3] = HDMI_INFOFRAME_TYPE_AVI + pData[1] + pData[2];

			for (i = 4; i <= 16; i++)
			      pData[3] += pData[i];
			pData[3] = 0x100 - pData[3];
		}
	}
#endif /* #if (AST_IT6805_DOWNSCALE_4K60_OVER_300MHZ_AVI_INFO_FIXUP == 1) */


	return 1;
}

int ast_get_avi_info(struct it680x_drv_data *d, unsigned char *pData)
{
	int ret;

	down(&d->reg_lock);

	ret = _get_avi_info(d, pData);

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

	pData[0] = HDMI_INFOFRAME_TYPE_VENDOR;
	pData[1] = hdmirxrd(0xDA);
	pData[2] = hdmirxrd(0xDB);
	pData[3] = hdmirxrd(0xDC);
	pData[4] = hdmirxrd(0xDD);
	pData[5] = hdmirxrd(0xDE);
	pData[6] = hdmirxrd(0xDF);
	pData[7] = hdmirxrd(0xE0);
	pData[8] = hdmirxrd(0xE1);
	pData[9] = hdmirxrd(0xE2);
	pData[10] = hdmirxrd(0xE3);

	chgbank(0);

	up(&d->reg_lock);

	return 1;
}

int ast_get_hdr_info(struct it680x_drv_data *d, unsigned char *buf)
{
	IT6805_DEV_DATA	*dev_data = (IT6805_DEV_DATA *) d->dev_data;
	int ret = 0, i;

	if (buf == NULL)
		goto exit;

	if (!dev_data->Flag_HAVE_DRM_PKT)
		goto exit;

	memcpy(buf, dev_data->DRM_HB, 3);
	memcpy(buf + 3, dev_data->DRM_DB, 28);

	ret = 1;
exit:
	return ret;
}

static u8 ast_audio_type_by_audio_caps(struct audio_caps *ac)
{
        /* ac->AudioFlag is converted to "audio type " by hdmi_input_audio() already */
	return ac->AudioFlag;
}

static u8 ast_audio_sample_frequency_by_audio_caps(struct audio_caps *ac)
{
	return ac->SampleFreq;
}

static u8 ast_audio_valid_channel_by_audio_caps(struct audio_caps *ac)
{
        /* ac->AudSrcEnable is converted to "valid channel # " by hdmi_input_audio() already */
	return ac->AudSrcEnable;
}

#undef AST_AUDIO_DBG
#ifdef AST_AUDIO_DBG
static void _dump_audio_info(Audio_Info *ai)
{
	int i;
	u32 freq = ai->SampleFrequency;

	uinfo ("Audio_On = %d\n", ai->Audio_On);
	if (!ai->Audio_On)
		return;

	/* Sink should don't care bit[4:5] first */
	switch (freq & 0xF) {
	case AST_FS_32000: //	(3)	/*   32 kHz, b--0011 */
		uinfo("Sample Freq %d = 0x%02x = 32 kHz, b--0011\n", freq, freq);
		break;

	case AST_FS_44100: //	(0)	/*   44.1 kHz, b--0000 */
		uinfo("Sample Freq %d = 0x%02x = 44.1 kHz, b--0000\n", freq, freq);
		break;
	case AST_FS_88200: //	(8)	/*   88.2 kHz, b--1000 */
		uinfo("Sample Freq %d = 0x%02x = 88.2 kHz, b--1000\n", freq, freq);
		break;
	case AST_FS_176400: //	(12)	/*  176.4 kHz, b--1100 */
		uinfo("Sample Freq %d = 0x%02x = 176.4 kHz, b--1100\n", freq, freq);
		break;

	case AST_FS_48000: //	(2)	/*   48 kHz, b--0010 */
		uinfo("Sample Freq %d = 0x%02x = 48 kHz, b--0010\n", freq, freq);
		break;
	case AST_FS_96000: //	(10)	/*   96 kHz, b--1010 */
		uinfo("Sample Freq %d = 0x%02x = 96 kHz, b--1010\n", freq, freq);
		break;
	case AST_FS_192000: //	(14)	/*  192 kHz, b--1110 */
		uinfo("Sample Freq %d = 0x%02x = 192 kHz, b--1110\n", freq, freq);
		break;

	case AST_FS_768000: //	(9)	/*  768 kHz, b--1001 */
		uinfo("Sample Freq %d = 0x%02x = 768 kHz, b--1001\n", freq, freq);
		break;
	/* Check new defined sample freq. */
	default:
		switch (freq) {
			case AST_FS_64000: //	(11)	/*   64 kHz, b001011 */
				uinfo("Sample Freq %d = 0x%02x = 64 kHz, b001011\n", freq, freq);
				break;
			case AST_FS_128000: //	(43)	/*  128 kHz, b101011 */
				uinfo("Sample Freq %d = 0x%02x = 128 kHz, b101011\n", freq, freq);
				break;
			case AST_FS_256000: //	(27)	/*  256 kHz, b011011 */
				uinfo("Sample Freq %d = 0x%02x = 256 kHz, b011011\n", freq, freq);
				break;
			case AST_FS_512000: //	(59)	/*  512 kHz, b111011 */
				uinfo("Sample Freq %d = 0x%02x = 512 kHz, b111011\n", freq, freq);
				break;
			case AST_FS_1024000: //	(53)	/* 1024 kHz, b110101 */
				uinfo("Sample Freq %d = 0x%02x = 1024 kHz, b110101\n", freq, freq);
				break;

			case AST_FS_352800: //	(13)	/*  352.8 kHz, b001101 */
				uinfo("Sample Freq %d = 0x%02x = 352.8 kHz, b001101\n", freq, freq);
				break;
			case AST_FS_705600: //	(45)	/*  705.6 kHz, b101101 */
				uinfo("Sample Freq %d = 0x%02x = 705.6 kHz, b101101\n", freq, freq);
				break;
			case AST_FS_1411200: //	(29)	/* 1411.2 kHz, b011101 */
				uinfo("Sample Freq %d = 0x%02x = 1411.2 kHz, b011101\n", freq, freq);
				break;

			case AST_FS_384000: //	(5)	/*  384 kHz, b000101 */
				uinfo("Sample Freq %d = 0x%02x = 384 kHz, b000101\n", freq, freq);
				break;
			case AST_FS_1536000: //	(21)	/* 1536 kHz, b010101 */
				uinfo("Sample Freq %d = 0x%02x = 1536 kHz, b010101\n", freq, freq);
				break;
			default:
				uinfo("Sample Freq = Unknown!?\n");
				break;
		}
		break;
	}

	uinfo("ValidCh(layout) = %d\n", ai->ValidCh);
	uinfo("SampleSize = %d\n", ai->SampleSize);
	uinfo("Audio_Type = %X\n", ai->Audio_Type);

	if (ai->Audio_Type & AST_AUDIO_HBR)
		uinfo("Audio_Type = HBR\n");
	if (ai->Audio_Type & AST_AUDIO_DSD)
		uinfo("Audio_Type = DSD\n");
	if (ai->Audio_Type & AST_AUDIO_NLPCM)
		uinfo("Audio_Type = NLPCM\n");

	for (i = 0; i < 5; i++)
		uinfo("AUD_DB[%d] = %02X\n", i, ai->AUD_DB[i]);

	for (i = 0; i < 5; i++)
		uinfo("IEC60958ChStat[%d] = %02X\n", i, ai->ucIEC60958ChStat[i]);
}
#endif /* #ifdef AST_AUDIO_DBG */

static void _get_audio_info(struct it680x_drv_data *d, Audio_Info *ai)
{
	IT6805_DEV_DATA	*dev_data = (IT6805_DEV_DATA *) d->dev_data;
	struct audio_caps *ac;

	memset(ai, 0, sizeof(Audio_Info));

	uinfo("_get_audio_info (state %s)\n", (dev_data->STATEA == STATEA_AudioOn)?"ON":"NOT_ON");

	if (dev_data->STATEA != STATEA_AudioOn)
		return;

	ac = &dev_data->ac;

	ai->Audio_On = 1;
	ai->Audio_Type = ast_audio_type_by_audio_caps(ac);
	ai->SampleFrequency = ast_audio_sample_frequency_by_audio_caps(ac);
	ai->ValidCh = ast_audio_valid_channel_by_audio_caps(ac);

	chgbank(2);
	ai->AUD_DB[0] = hdmirxrd(REG_RX_AUDIO_DB1);
	ai->AUD_DB[1] = hdmirxrd(REG_RX_AUDIO_DB2);
	ai->AUD_DB[2] = hdmirxrd(REG_RX_AUDIO_DB3);
	ai->AUD_DB[3] = hdmirxrd(REG_RX_AUDIO_DB4);
	ai->AUD_DB[4] = hdmirxrd(REG_RX_AUDIO_DB5);

	chgbank(0);

	ai->ucIEC60958ChStat[0] = hdmirxrd(REG_RX_AUD_CHSTAT0);
	ai->ucIEC60958ChStat[1] = hdmirxrd(REG_RX_AUD_CHSTAT1);
	ai->ucIEC60958ChStat[2] = hdmirxrd(REG_RX_AUD_CHSTAT2);
	ai->ucIEC60958ChStat[3] = hdmirxrd(REG_RX_AUD_CHSTAT3);
	ai->ucIEC60958ChStat[4] = hdmirxrd(REG_RX_AUD_CHSTAT4);

#ifdef AST_AUDIO_DBG
	_dump_audio_info(ai);
#endif
	/*
	 * the sample_rate_table in i2s/driver/i2s_main.c has following items,
	 *    44.1KHz, 48KHz, 32KHz, 88.2KHz, 96KHz, 176.4KHz, 192KHz
	 *  We have to check sampling frequency before passing to I2S
	 */
	switch (ai->SampleFrequency) {
	case AST_FS_44100: case AST_FS_48000: case AST_FS_32000:
	case AST_FS_88200: case AST_FS_96000: case AST_FS_176400:
	case AST_FS_192000:
		/* exist in I2S sample rate table */
		break;
	case AST_FS_768000: /* HBR */
		ai->SampleFrequency = AST_FS_192000;
		ai->ValidCh = 8;
		break;
	default:
		/* TODO */
#if 0
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
#endif
		break;
	}
}

int ast_get_hdmi_n_cts(struct it680x_drv_data *d, unsigned int *pn, unsigned int *pcts)
{
	IT6805_DEV_DATA	*dev_data = (IT6805_DEV_DATA *) d->dev_data;

	*pn = dev_data->hdmi_n;
	*pcts = dev_data->hdmi_cts;

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
	 * Bruce200318. WTF?!
	 * I can't see why I 'think' force_update_audio_info has no race condition
	 * even ast_audio_info_update() is always scheduled by ast_schedule_audio_info_update().
	 * Unless above flush_workqueue() is not commented, it DOES has race condition.
	 * And I don't know why flush_workqueue() was commented?!
	 * I check git log history. flush_workqueue() was commented on the birth of this
	 * driver. Maybe we hit some kind of dead lock situation or this function is required to be
	 * atomic?!
	 */
	/*
	** Bruce 'think' there is no race condition modifying d->force_update_audio_info here.
	** It is true when we ALWAYS call ast_audio_info_update() through ast_schedule_audio_info_update().
	*/
	d->force_update_audio_info = force_update;
	PREPARE_WORK(&d->audio_update_work, (void (*)(void *))ast_audio_info_update, d);
	queue_work(d->wq, &d->audio_update_work);
}

void ast_hpd_ctrl(struct it680x_drv_data *d, unsigned int level)
{
	down(&d->reg_lock);

	if (level)
		IT6805_HPD_CTRL(PORT0 , HPD_EXTRNL_HIGH);
	else
		IT6805_HPD_CTRL(PORT0 , HPD_EXTRNL_LOW);

	up(&d->reg_lock);
}

