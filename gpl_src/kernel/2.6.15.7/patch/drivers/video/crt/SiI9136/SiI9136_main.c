/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#include "SiI9136_main.h"
#if (!NO_INT_MODE)
#include <linux/interrupt.h>
#endif
#include "ast_utilities.h"
#include "defs.h"
#include "SiI9136_wrap.h"
#include "Externals.h"
#include "AV_Config.h"
#include "constants.h"
#include "TPI_Access.h"
#include "edid.h"
#include "HDCP.h"
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
#include "si_datatypes.h"
#include "si_apiCEC.h"
#include "si_apiCpi.h"
#endif
#if SUPPORT_HDCP_REPEATER
#include "TPI.h"
#include <aspeed/hdcp1.h>
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif

#define I2C_SII9136_SPEED 100000 //100KHz bus speed
#define LOOP_MSEC T_MONITORING_PERIOD //default 10ms

#define MODULE_NAME "SiI9136"
MODULE_LICENSE ("GPL");

static struct platform_device *pdev;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
struct sii9136_drv_data *drv_data;
#else
static struct sii9136_drv_data *drv_data;
#endif
static int debug_level = DEBUG_LEVEL_INFO;
int tv_access_flag = 0;
void sii9136_debug_level_cfg(int level)
{
	if ((level < DEBUG_LEVEL_TRACE) || (level > DEBUG_LEVEL_ERROR))
		return;
	debug_level = level;
}

int sii9136_debug_level(void)
{
	return debug_level;
}

/*
 * Now, we let CRT control whether force output as RGB or not,
 *  => undefine FORCE_OUTPUT_RGB of Si9136
 */
#undef FORCE_OUTPUT_RGB


#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
static void shadow_video_setting(struct sii9136_drv_data *d)
{
	u8 *src, *dst;
	u32 offset, size;

	size = sizeof(struct sii9136_video_data) - (sizeof(InfoFrame) * 3);
	offset = 0;

	src = (u8 *) &d->video;
	dst = (u8 *) &d->current_video;

	memcpy(dst + offset, src + offset, size);
}

static void shadow_video_hdmi_info(struct sii9136_drv_data *d)
{
	u8 *src, *dst;
	u32 offset, size;

	size = sizeof(InfoFrame) * 3;
	offset = sizeof(struct sii9136_video_data) - size;

	src = (u8 *) &d->video;
	dst = (u8 *) &d->current_video;

	memcpy(dst + offset, src + offset, size);
}
#endif

static void hdmi_infoframe_set(struct sii9136_drv_data *d)
{
	struct sii9136_video_data *todo = &d->video;
	InfoFrame *pinfo;

	if (!todo->hdmi)
		return;

	/* AVI IF */
	pinfo = &todo->avi_info;
	if (pinfo->number) {
#if defined(FORCE_OUTPUT_RGB)
		pinfo->data[4] &= ~(0x3 << 5);
#endif
		/* just need payload */
		sii9136_set_avi_info_frame(pinfo->data + 3);

	} else {
		/* no AVI infoframe? is it possible? */
		sii9136_disable_avi_if();
	}

	/* VS IF */
	pinfo = &todo->vs_info;
	if ((pinfo->number) && (0x81 == pinfo->data[0]))
		sii9136_set_vendor_specific_info_frame(pinfo->data);
	else
		sii9136_disable_vendor_specific_if();

	/* HDR IF */
	pinfo = &todo->hdr_info;
	if ((pinfo->number) && (0x87 == pinfo->data[0]))
		sii9136_set_hdr_info_frame(pinfo->data);
	else
		sii9136_disable_hdr_if();

#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	shadow_video_hdmi_info(d);
#endif
}

static u32 no_tmds_stable_check = 0; /* for special purpose, if set, do not wait for sii9136 tmds stable */

#define DEEP_COLOR_PIXEL_CLOCK_MAX_100HZ 1485000
#define UNDER_4K_MAX_100HZ 1700000

static void recover_by_hw_reset(void)
{
	u8 intr_cfg;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
	u8 cec_la = SI_CpiGetLogicalAddr();
#endif

	P_INFO("H/W reset due to CEC issue\n");
	intr_cfg = ReadByteTPI(0x3C);

	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDMI_TX);

	TPI_Init();

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
	SI_CpiSetLogicalAddr(cec_la);
#endif
	/* enable interurpt */
	WriteByteTPI(0x3C, intr_cfg);
}

static void wait_for_tmds_stable(void)
{
	int check_count;

	if (!no_tmds_stable_check)
		return;

	check_count = 0;

	while (!tmds_clock_stable()) { /* wait for clock stable */
		msleep(20);
		check_count++;
		if (check_count > 150) { /* wait 3000 ms */
			P_WARN("TMDS clock is not stable\n");
			break;
		}
	}
}

#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
static u32 _ready_for_infoframe_only = 1;

u32 ready_for_infoframe_only(void)
{
	return _ready_for_infoframe_only;
}

void ready_for_infoframe_only_set(u32 cfg)
{
	_ready_for_infoframe_only = cfg;
}

static int video_changed(struct sii9136_drv_data *d)
{
	struct sii9136_video_data *todo, *done;
	u32 compare_size;

	compare_size = sizeof(struct sii9136_video_data) - (sizeof(InfoFrame) * 3); /* check infoframe in hdmi_infoframe_set() */

	todo = &d->video;
	done = &d->current_video;

	done->infoframe_only = todo->infoframe_only; /* ignore infoframe_only */
	done->color_depth = todo->color_depth; /* ignore color depth, this kind of change should get filtered if not strict mode */

	if (memcmp(done, todo, compare_size) == 0) {
		if (ready_for_infoframe_only())
			return 0;
	}

	return 1;
}
#endif

static void __sii9136_set_video(struct sii9136_drv_data *d)
{
	MODE_ITEM *mode;
	struct sii9136_video_data *pvideo;
	u8 input_color_space, color_space_std, output_format, clock_sel = 1, edge_select;
	int check_count = 0;
	u32 expansion, compression;

	pvideo = &d->video;
	mode = pvideo->display_mode;

#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	if (pvideo->infoframe_only) {
		if (video_changed(d) == 0)
			goto xmitter_config_done;
	}
#endif

	/*
	 * from SiI9136-3 HDMI Deep Color Transmitter Data Sheet, Table 20,
	 * the color depth support,
	 *    4K   (297 MHz): 8,
	 *  UXGA   (162 MHz): 8, 10
	 * 1080P (148.5 MHz): 8, 10, 12
	 *
	 * => patch color_depth(bus width) to 8 (24), when pixel clock > DEEP_COLOR_PIXEL_CLOCK_MAX_100HZ
	 */

	if (mode->DCLK10000 > DEEP_COLOR_PIXEL_CLOCK_MAX_100HZ)
		pvideo->color_depth = 24;

	P_TRACE("[%s]pvideo->hdmi %d, pvideo->audio %d, pvideo->color_depth %d, pvideo->color_mode %x\n",
		__func__, pvideo->hdmi, pvideo->audio, pvideo->color_depth, pvideo->color_mode);
start_setup:

	av_mute();

	if (HDCP_Started) {
		HDCP_Off_without_avmute();

		while (link_protection_level()) { /* wait until HDCP is disabled */
			msleep(20);
			check_count++;
			if (check_count > 150) { /* wait 3000 ms */
				P_WARN("HDCP is not disabled\n");
				break;
			}
		}
	}

	SiiMhlTxDrvTmdsControl(false, false);
	msleep(128);

	sii9136_video_mode(mode->DCLK10000/100, mode->RefreshRate, mode->HActive, mode->VActive);

	input_color_space = pvideo->input_color_mode & 0x3;

	if (2 == pvideo->colorimetry) /* 2b10 => 709 */
		color_space_std = 1;
	else if (1 == pvideo->colorimetry)  /*2b01 => 601 */
		color_space_std = 0;
	else {/* FIXME 2b00 => no data, 2b11 =>extended colorimetry */
		if (0 == pvideo->colorimetry) {
			/* From CEA-861D, chapter 5
			 * 480p, 480i, 576p, 576i, 240p and 288p => BT601
			 * 1080i, 1080p and 720p => BT709
			 */
			if (720 <= mode->HActive)
				color_space_std = 1;
			else
				color_space_std = 0;
		} else {
			/* TODO 2b11 =>extended colorimetry */
			/* neither 709 nor 601 is correct, just select 709 for sii9136 configuration */
			color_space_std = 1;
		}
	}

	/*
	 * CEA-861-D table 8 definition:(Y1 Y0)
	 * 00: RGB
	 * 01: YCbCr422
	 * 10: YCbCr444
	 */
#if defined(FORCE_OUTPUT_RGB)
	output_format = BITS_OUT_RGB;
#else
	output_format = pvideo->color_mode & 0x3;
#endif

	if (pvideo->input_color_mode & BITS_IN_LTD_RANGE)
		expansion = 1; // on
	else
		expansion = 2; // off

	if (pvideo->color_mode & BITS_OUT_LTD_RANGE)
		compression = 2; // on
	else
		compression = 1; // off

	P_INFO("Q Range: %s => %s\n", (expansion == 1) ? "Limited" : "Full",
	                              (compression == 2) ? "Limited" : "Full");
	sii9136_video_format(input_color_space, output_format, color_space_std, pvideo->color_depth, pvideo->hdmi, expansion, compression);

	/* TPI registers 0x08 */
	/*
	 * FIXME
	 * REG0x08[7:6]: TClkSel. Ratio of output TMDS clock to input video clock
	 *	00 – x0.5
	 *	01 – x1 (default)
	 *	10 – x2
	 *	11 – x4
	 * REG0x08[3:0]: Pixel Repetition Factor1
	 *	0000 – Pixel data is not replicated
	 *	0001 – Pixels are sent two times each
	 *	0011 – Pixels are sent four times each
	 *
	 * For pixel repetition, we have to double (or quadruple) pixel clock
	 *	repeat 1 time: pixel clock * 2 (pixel set 2 times)
	 *	repeat 3 time: pixel clock * 4
	 * 1. (input clock * N) + (no pixel-repetition) => OK
	 * 2. (input clock * N) + (pixel-repetition) => incorrect
	 * 3. (pixel-repetition) => incorrect
	 */
	switch (pvideo->pixelrep) {
	case 0: /* No Repetition (i.e., pixel sent once) */
		clock_sel = 1;
		break;
	case 1: /* pixel sent 2 times (i.e., repeated once) */
		clock_sel = 2;
		break;
	case 3: /* pixel sent 4 times */
		clock_sel = 3;
		break;
	default:
		P_WARN("[%s] pvideo->pixelrep %d\n", __func__, pvideo->pixelrep);
		break;
	}

	if (mode->DCLK10000 > UNDER_4K_MAX_100HZ)
		edge_select = 1;
	else
		edge_select = 0;

	sii9136_video_mode_input_bus_pixel_rep(clock_sel, 1, edge_select, 0);

	/*
	** Bruce180130. 2160p30Hz doesn't work if we don't do sii9136_software_reset() here.
	** From my test, looks like "to D2, to D0" is the key to make 2160p30 work.
	** This is why we call sii9136_software_reset() here.
	**
	** Bruce180126. software reset 'seems' causes troubles.
	** Looks like there is no need to do software reset here.
	** I'm going to skip software_reset() and field test it. <== Failed. See Bruce180130.
	*/
	if (sii9136_software_reset() != 0) {
		/* abnormal CEC register access after software reset, need to do HW reset to recover it */
		recover_by_hw_reset();
		goto start_setup;
	}

	SiiMhlTxDrvTmdsControl(true, false);

	wait_for_tmds_stable();

	av_unmute();

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
#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	shadow_video_setting(d);
	ready_for_infoframe_only_set(1);
xmitter_config_done:
#endif
	hdmi_infoframe_set(d);

	return;
}

static void __sii9136_set_audio(struct sii9136_drv_data *d)
{
	sii9136_set_audio_i2s(&d->audio);

	if (d->video.hdmi)
		sii9136_set_audio_info_frame(d->audio.infoframe.AUD_DB);
	else
		sii9136_disable_audio_if();
}

static void __sii9136_disable(struct sii9136_drv_data *d)
{
	av_mute();

	if (HDCP_Started) {
		int check_count = 0;

		HDCP_Off_without_avmute();
		while (link_protection_level()) { /* wait until HDCP is disabled */
			msleep(20);
			check_count++;
			if (check_count > 150) { /* wait 3000 ms */
				P_WARN("HDCP is not disabled\n");
				break;
			}
		}
	}

	/*
	 * we got an issue on ASUS VE226,
	 * there is often no sound after plug/unplug HDMI cable between SiI9136 and VE226 monitor
	 * this problem can get fixed after monitor power down&up
	 * We cannot figure out why this problem happed without analylzer,
	 * just guest monitor might need couple of times for going to more stable or no infoframe received
	 */
	/*
	 * Wait at least 128ms to allow control InfoFrames to pass through to the sink device
	 * for safety, we use 200ms here
	 */
	msleep(200);

	SiiMhlTxDrvTmdsControl(false, false);

#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	ready_for_infoframe_only_set(0); /* for HDCP, make sure sii9136 get configured after tmds get disabled */
#endif
}

static unsigned int sii9136_exist(struct s_crt_drv *crt)
{
	return 1;
}

static unsigned int sii9136_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	if (1 < blk_num) {
		P_ERR("blk_num(%d) not supported\n", blk_num);
		return 1;
	}

	if (!CableConnected) {
		P_INFO("sink not plugged yet\n");
		return 2;
	}

	if (!edidDataValid) {
		P_INFO("EDID is not ready yet\n");
		return 3;
	}

	memcpy(pEdid, global_sink_edid + (blk_num * 128), 128);

	return 0;
}

static unsigned int sii9136_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum)
{
	if (!CableConnected) {
		P_INFO("sink not plugged yet\n");
		return 2;
	}

	if (!edidDataValid) {
		P_INFO("EDID is not ready yet\n");
		return 3;
	}

	*pChksum = global_sink_edid[EDID_CHECKSUM_OFFSET];

	return 0;
}

static unsigned int sii9136_disable(struct s_crt_drv *crt)
{
	struct workqueue_struct *wq;

	if (NULL == drv_data) {
		/* TODO */
		return -1;
	}

	wq = drv_data->wq;

	if (NULL == wq) {
		/* TODO */
		return -1;
	}

	cancel_delayed_work(&drv_data->disable_task);
	flush_workqueue(wq);

	queue_work(wq, &drv_data->disable_task);
	flush_workqueue(wq);

	return 0;
}

/* Any video format listed in Table 2 except 640x480p */
static u32 ce_video_map[8] = {
	0xFFFFFFFC, /*  2 ~  31 */
	0xFFFFFFFF, /* 32 ~  63 */
	0xFFFFFFFF, /* 64 ~  95 */
	0x00000FFF, /* 96 ~ 107 */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static u32 ce_video_format(InfoFrame *avi_info, InfoFrame *vs_info)
{
	hdmi_avi_infoframe *avi;
	hdmi_vsif *vsif;
	u32 shift;
	u8 vic;

	/* No AVI info means IT video format. */
	if (!avi_info || !avi_info->number)
		goto is_it;

	avi = (hdmi_avi_infoframe *)(avi_info->data);

	vic = avi->info.VIC;

	/* CE Video Format exception. HDMI 1.4b 4K30 format uses VIC==0 */
	if (0 == vic) {
		if (!vs_info || !vs_info->number)
			goto is_it;
		/* HDMI VSIF is valid */
		vsif = (hdmi_vsif *)(vs_info->data);
		/* valid OUI */
		if ((vsif->H14b4K.OUI_03 != 0x03) || (vsif->H14b4K.OUI_0C != 0x0C)
		    || (vsif->H14b4K.OUI_00 != 0x00)) {
			goto is_it;
		}
		/* check if has "extended resolution format" */
		if (vsif->H14b4K.VideoFormat != 1)
			goto is_it;
		/* has extended resolution format. It is a CE Video Format. */
		goto is_ce;
	}

	shift = vic & 0x1F;
	if (ce_video_map[vic >> 5] & (1 << shift))
		goto is_ce;

is_it:
	P_INFO("is IT Video Format!\n");
	return 0;
is_ce:
	P_INFO("is CE Video Format!\n");
	return 1;
}

static unsigned int sii9136_setup_video(struct s_crt_drv *crt, struct s_crt_info *info)
{
	struct workqueue_struct *wq;
	MODE_ITEM *mt = crt->mode_table;
	struct sii9136_video_data *pvideo;
	struct s_xmiter_mode *xmiter_mode = &info->xmiter_mode;
	u8 *ptr;
	int i;

	if (NULL == drv_data) {
		/* TODO */
		return -1;
	}

	wq = drv_data->wq;

	if (NULL == wq) {
		/* TODO */
		return -1;
	}

	cancel_delayed_work(&drv_data->setup_video_task);
	flush_workqueue(wq);

	/* pvideo must be protected. */
	down(&drv_data->reg_lock);

	pvideo = &drv_data->video;

	//look up mode
	//DstWidth,DstHight is CRT resolution
	i = lookupModeTable(crt, info);

	P_TRACE("xmiter_mode->HDMI_Mode %d, info->crt_output_format %d, xmiter_mode->color_depth %d\n",
		xmiter_mode->HDMI_Mode, info->crt_output_format, xmiter_mode->color_depth);

        /* Following code are used to configure the xmiter's AVInfoFrame. (Digital Only). */
	switch (xmiter_mode->HDMI_Mode) {
	case s_RGB:
		//Means default mode from non-VE (OWNER_CON).
		//uinfo("DVI mode\n");
		pvideo->hdmi = 0;
		break;
	case s_DVI:
		//uinfo("DVI mode\n");
		pvideo->hdmi = 0;
		break;
	case s_HDMI_4x3:
		//uinfo("HDMI 4:3 mode\n");
		pvideo->hdmi = 1;
		break;
	case s_HDMI_16x9:
		//uinfo("HDMI 16:9 mode\n");
		pvideo->hdmi = 1;
		break;
	default:
		uerr("Unknown HDMI type!?(%d)\n", xmiter_mode->HDMI_Mode);
		BUG();
		break;
	}

	/* pvideo->input_color_mode is eVIDEO_FORMAT type*/
	switch (info->crt_output_format) {
	case YUV444_FORMAT: /* YUV444_FORMAT */
		pvideo->input_color_mode = BITS_IN_YCBCR444;
		break;
	case YUV420_FORMAT: /* YUV420_FORMAT */
		pvideo->input_color_mode = BITS_IN_YCBCR444; /* TBD? */
		break;
	default:
		pvideo->input_color_mode = BITS_IN_RGB;
		break;
	}

	if (info->flag & CRT_OUT_LIMITED)
		pvideo->input_color_mode |= BITS_IN_LTD_RANGE;
	else
		pvideo->input_color_mode |= BITS_IN_FULL_RANGE;

	pvideo->display_mode = mt + i;
	pvideo->color_depth = xmiter_mode->color_depth;
	memcpy(&pvideo->avi_info, &xmiter_mode->AVI_Info, sizeof(InfoFrame));
	memcpy(&pvideo->vs_info, &xmiter_mode->HDMI_Info, sizeof(InfoFrame));
	memcpy(&pvideo->hdr_info, &xmiter_mode->HDR_Info, sizeof(InfoFrame));

	P_TRACE("pvideo->hdmi %d, pvideo->audio %d\n", pvideo->hdmi, pvideo->audio);
	P_TRACE("pvideo->color_depth %d pvideo->input_color_mode %x\n", pvideo->color_depth, pvideo->input_color_mode);

	P_TRACE("pvideo->avi_info.number %d\n", pvideo->avi_info.number);
	if (pvideo->avi_info.number) {
		P_TRACE("pvideo->avi_info.data\n");
		ptr = pvideo->avi_info.data;
		P_TRACE("\t0x%.2x 0x%.2x 0x%.2x\n", ptr[0], ptr[1], ptr[2]);
		P_TRACE("\t0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
			ptr[3], ptr[4], ptr[5], ptr[6],
			ptr[7], ptr[8], ptr[9], ptr[10],
			ptr[11], ptr[12], ptr[13], ptr[14],
			ptr[15], ptr[16]);

		P_TRACE("InfoFrame: VIC %d, ColorMode %d, PixelRep. %d Colorimetry %d\n",
			ptr[7] & 0x7F,
			(ptr[4] >> 5) & 0x3,
			ptr[8] & 0xF,
			(ptr[5] >> 6) & 0x3);

		/* get information from infoframe */
		pvideo->vic = ptr[7] & 0x7F;

		if (drv_data->pixel_repetition_by_avi == 1)
			pvideo->pixelrep = ptr[8] & 0xF;
		else
			pvideo->pixelrep = 0;

		pvideo->colorimetry = (ptr[5] >> 6) & 0x3;

		switch((ptr[4] >> 5) & 0x3) {
		case 0: // RGB, default
			pvideo->color_mode = BITS_OUT_RGB;
			break;
		case 1: // YUV422
			pvideo->color_mode = BITS_OUT_YCBCR422;
			break;
		case 2: // YUV444
			pvideo->color_mode = BITS_OUT_YCBCR444;
			break;
		case 3: // YUV420
			pvideo->color_mode = BITS_OUT_YCBCR444; //cheat as YUV444
			break;
		default: // use RGB(?)
			pvideo->color_mode = BITS_OUT_RGB;
			break;
		}

		if (info->crt_output_format == YUV420_FORMAT)
			pvideo->color_mode = BITS_OUT_YCBCR444; //cheat as YUV444

		if ((pvideo->color_mode & 0x3) == BITS_OUT_RGB) {
			/* RGB out. check RGB quantization range. */
			switch ((ptr[6] >> 2) & 0x3) {
			case 0: //default
				if (ce_video_format(&pvideo->avi_info, &pvideo->vs_info))
					pvideo->color_mode |= BITS_OUT_LTD_RANGE;
				else
					pvideo->color_mode |= BITS_OUT_FULL_RANGE;
				break;
			case 1: //limited
				pvideo->color_mode |= BITS_OUT_LTD_RANGE;
				break;
			case 2: //full
				pvideo->color_mode |= BITS_OUT_FULL_RANGE;
				break;
			default: //reserved. Use full
				pvideo->color_mode |= BITS_OUT_FULL_RANGE;
				break;
			}
		} else {
			/* YUV out. check YUV quantization range */
			switch ((ptr[8] >> 6) & 0x3) {
			case 0: //limited
				pvideo->color_mode |= BITS_OUT_LTD_RANGE;
				break;
			case 1: //full
				pvideo->color_mode |= BITS_OUT_FULL_RANGE;
				break;
			default: //reserved. check CE Video Format?
				if (ce_video_format(&pvideo->avi_info, &pvideo->vs_info))
					pvideo->color_mode |= BITS_OUT_LTD_RANGE;
				else
					pvideo->color_mode |= BITS_OUT_FULL_RANGE;
				break;
			}
		}
	} else {
		pvideo->color_mode = BITS_OUT_RGB | BITS_OUT_FULL_RANGE;
		pvideo->pixelrep = 0;
		pvideo->colorimetry = 0;
	}

	if (pvideo->hdr_info.number) {
		P_TRACE("pvideo->hdr_info.data\n");
		ptr = pvideo->hdr_info.data;
		P_TRACE("\t0x%.2x 0x%.2x 0x%.2x\n", ptr[0], ptr[1], ptr[2]);
		P_TRACE("\t0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
			ptr[3], ptr[4], ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15], ptr[16], ptr[17], ptr[18]);
		P_TRACE("\t0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
			ptr[19], ptr[20], ptr[21], ptr[22], ptr[23], ptr[24], ptr[25], ptr[26], ptr[27], ptr[28], ptr[29]);
	}

#if (SUPPORT_HDMI_INFOFRAME_ONLY == 1)
	if (xmiter_mode->flag & HDMI_INFOFRAME_ONLY)
		pvideo->infoframe_only = 1;
#endif

	queue_work(wq, &drv_data->setup_video_task);

	up(&drv_data->reg_lock);

	flush_workqueue(wq);

	return 0;
}

static unsigned int sii9136_setup_audio(Audio_Info *audio_info)
{
	struct workqueue_struct *wq;
	struct sii9136_audio_data *pd;

	if (NULL == drv_data) {
		/* TODO */
		return -1;
	}

	wq = drv_data->wq;

	if (NULL == wq) {
		/* TODO */
		return -1;
	}

	cancel_delayed_work(&drv_data->setup_audio_task);
	flush_workqueue(wq);

	/* pd must be protected. */
	down(&drv_data->reg_lock);

	P_TRACE("audio_info:\n");
	P_TRACE("SampleFrequency %d, ValidCh %d, SampleSize %d bSPDIF %d, Audio_Type 0x%x\n",
		audio_info->SampleFrequency, audio_info->ValidCh, audio_info->SampleSize, 
		audio_info->bSPDIF, audio_info->Audio_Type);

	P_TRACE("audio_info->ucIEC60958ChStat:\n");
	P_TRACE("\t0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n", 
		audio_info->ucIEC60958ChStat[0], audio_info->ucIEC60958ChStat[1],
		audio_info->ucIEC60958ChStat[2], audio_info->ucIEC60958ChStat[3],
		audio_info->ucIEC60958ChStat[4]);

	P_TRACE("audio_info->AUD_DB:\n");
	P_TRACE("\t0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
		audio_info->AUD_DB[0], audio_info->AUD_DB[1],
		audio_info->AUD_DB[2], audio_info->AUD_DB[3],
		audio_info->AUD_DB[4]);

	P_TRACE("Audio Info:\n");
	P_TRACE("\tchannel count %d, coding type %d, sample size %d, sample frequency %d\n",
			audio_info->AUD_DB[0] & 0x7,
			(audio_info->AUD_DB[0] >> 4) & 0xF,
			audio_info->AUD_DB[1] & 0x3,
			(audio_info->AUD_DB[1] >> 2) & 0x7);

	pd = &drv_data->audio;

	pd->sample_frequency = audio_info->SampleFrequency; 
	pd->valid_ch = audio_info->ValidCh;
	pd->sample_size = audio_info->SampleSize;
	pd->audio_type = audio_info->Audio_Type;
	pd->is_spdif = audio_info->bSPDIF;
	memcpy(pd->infoframe.AUD_DB, audio_info->AUD_DB, AUDIO_INFOFRAME_LEN);
	memcpy(pd->iec60958_channel_status, audio_info->ucIEC60958ChStat, 5);

	queue_work(wq, &drv_data->setup_audio_task);

	up(&drv_data->reg_lock);

	flush_workqueue(wq);

	return 0;
}

static void sii9136_disable_audio(void)
{
	/* TODO */
	/* XXX Nobody call xHal_xmiter_disable_audio(), should we need to register .disable_audio? */
}

static unsigned int sii9136_init(struct s_crt_drv *crt)
{
	return sii9136_disable(crt);
}

static unsigned int sii9136_check_hotplug(struct s_crt_drv *crt)
{
	if (!CableConnected)
		return 0;

	return 1;
}

#ifdef CONFIG_ARCH_AST1500_HOST
/* TODO host loopback */
#if 0
static void sii9136_setup_loopback_video(VIDEOPCLKLEVEL level, unsigned char bHDMI, unsigned char *pAVI_InfoFrame, unsigned char *pHDMI_InfoFrame)
{
	/* TODO */
}
#endif
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
unsigned int sii9136_cec_send(unsigned char *buf, unsigned int size)
{
	down(&drv_data->cec_lock);
	si_cec_tx_en_q(buf, size);
	up(&drv_data->cec_lock);
	queue_work(drv_data->wq, &drv_data->work_cec_tx);

	return 0;
}

static int sii9136_cec_topology(unsigned char *buf, u32 scan)
{
	u16 *ptr = (u16 *) buf;

	if (CableConnected) {
		if (scan)
			si_cpi_topology_poll_cfg(1); /* enable discovery */

		drv_data->cec_topology = si_cec_topology_status();
	} else {
		drv_data->cec_topology = 0;
	}

	*ptr = drv_data->cec_topology;

	return 0;
}

static void sii9136_cec_pa_cfg(u16 address)
{
	if (address != SI_CecGetDevicePA())
		SI_CecSetDevicePA(address);
}

static void sii9136_cec_la_cfg(u8 address)
{
	if (address != SI_CpiGetLogicalAddr())
		SI_CpiSetLogicalAddr(address);
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1) */

#if SUPPORT_HDCP_REPEATER
static void sii9136_hdcp1_downstream_port_set_mode(unsigned char repeater)
{
	/* TODO */
	/* CRT_HDCP1_Downstream_Port_Set_Mode gets called only in HOST, complete this later */
}

static void hdcp1_auth(int enable)
{
	down(&drv_data->reg_lock);

	uinfo("HDCP -> CRT Request %s\n", (enable ? "enable" : "disable"));
	if (enable) {
		VideoModeDescription.HDCPAuthenticated = VMD_HDCP_AUTHENTICATED;
		/*
		** Bruce171127. Add polling sink_support_hdcp() here.
		** If the sink doesn't support HDCP at all. Calling HDCP_On() will not
		** trigger any interrupt and no further events.
		** So, we need to check sink_support_hdcp() here.
		** TODO:
		** Maybe SiI9136 driver should maintain its own timer to monitor HDCP status
		** and callback if necessary.
		*/
		if (!sink_support_hdcp() || (enable != HDCP_V_1X)) {
			/* sink doesn't support HDCP or HDCP version is not 1.x */
			CRT_HDCP1_Downstream_Port_Auth_Status_Callback(0, NULL, 0, 0, NULL);

			/*
			 * sometimes, sink do support HDCP but sii9136 doesn't think so
			 * call RestartHDCP() to recovery HDCP function
			 */
			if (!sink_support_hdcp()) {
				HDCP_RETRY_COUNTER_INC;

				if (HDCP_RETRY_COUNTER > 3) {
					RestartHDCP();
					HDCP_RETRY_COUNTER_RESET;
				}
			}
		} else {
			/* be paired with 'enable == 0' */
			SetHDCP_TxSupports;

			/*
			 * sometimes, sii9136 enable HDCP but cannot get HDCP done with sink
			 * call RestartHDCP() to recovery HDCP function
			 */
			if (HDCP_Started) {
				HDCP_RETRY_COUNTER_INC;
				if (HDCP_RETRY_COUNTER > 5) {
					HDCP_RETRY_COUNTER_RESET;
					RestartHDCP();
					wait_for_tmds_stable();
				}
			} else {
				HDCP_RETRY_COUNTER_RESET;
			}
			/* Bruce180126. Seeing no need to restart TMDS by calling RestartHDCP(). */
			//RestartHDCP();
#if 0
			HDCP_On();
			av_unmute();
#else
			HDCP_CheckStatus_One();
#endif
		}
	} else {
		HDCP_RETRY_COUNTER_RESET;
		VideoModeDescription.HDCPAuthenticated = VMD_HDCP_NOT_AUTHENTICATED;
		/* to skip HDCP procedures in HotPlugService() */
		ClrHDCP_TxSupports;
		HDCP_Off_without_avmute();
	}
	up(&drv_data->reg_lock);
}

static void hdcp1_auth_start(struct sii9136_drv_data *d)
{
	hdcp1_auth(1);
}

static void hdcp1_auth_stop(struct sii9136_drv_data *d)
{
	hdcp1_auth(0);
}

static void sii9136_hdcp1_downstream_port_auth(int enable)
{
	struct workqueue_struct *wq;

	if (NULL == drv_data) {
		/* TODO */
		return;
	}

	wq = drv_data->wq;

	if (NULL == wq) {
		/* TODO */
		return;
	}

	flush_workqueue(wq);

	if (enable) {
		queue_work(wq, &drv_data->hdcp_auth_start_task);
	} else {
		queue_work(wq, &drv_data->hdcp_auth_stop_task);
	}

	flush_workqueue(wq);
}

static void sii9136_hdcp1_downstream_port_encrypt(unsigned char enable)
{
	/* TODO */
	/* cat6613 do nothing here, is it necessary to implement this ? */
}
#endif

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
	pinfo->chip		= xmiter_sii9136;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
	pinfo->cap		= (xCap_DVI | xCap_HDMI | xCap_CEC);
#else
	pinfo->cap		= (xCap_DVI | xCap_HDMI);
#endif
	pinfo->exist		= sii9136_exist;
	pinfo->setup		= sii9136_setup_video;
	pinfo->setup_audio	= sii9136_setup_audio;
	pinfo->disable		= sii9136_disable;
	pinfo->disable_audio	= sii9136_disable_audio;
	pinfo->init		= sii9136_init;
	pinfo->check_hotplug	= sii9136_check_hotplug;
	pinfo->rd_edid_chksum	= sii9136_rd_edid_chksum;
	pinfo->rd_edid		= sii9136_rd_edid;
	pinfo->poll_hotplug	= NULL;
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
	pinfo->cec_send		= sii9136_cec_send;
	pinfo->cec_topology	= sii9136_cec_topology;
	pinfo->cec_pa_cfg	= sii9136_cec_pa_cfg;
	pinfo->cec_la_cfg	= sii9136_cec_la_cfg;
#endif
#if SUPPORT_HDCP_REPEATER
	pinfo->Hdcp1_set_mode	= sii9136_hdcp1_downstream_port_set_mode;
	pinfo->Hdcp1_auth	= sii9136_hdcp1_downstream_port_auth;
	pinfo->Hdcp1_encrypt	= sii9136_hdcp1_downstream_port_encrypt;
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

static void sii9136_drv_init(struct sii9136_drv_data *d)
{
	init_MUTEX(&d->reg_lock);
#if (IS_CEC == 1)
	init_MUTEX(&d->cec_lock);
#endif
	sii9136_clk_edge_cfg(ast_scu.astparam.v_tx_drv_option & DRVOP_EDGE_SELECT_MASK);

#if (PIXEL_REPETITION_BY_AVIINFO == 1)
	d->pixel_repetition_by_avi = 1;
#endif
}

static ssize_t show_registers(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, num = 0;
	u32 start, end;

	start = 0x00;
	end = 0x61;

        num += snprintf(buf + num, PAGE_SIZE - num, "%s register dump:\n", MODULE_NAME);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n       ");
	for (i = 0; i <= 0xF; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", i);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
			num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.2x: ", i);

		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", ReadByteTPI(i));
	}
	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	return num;
}

static ssize_t store_registers(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 offset, value, c;

	c = sscanf(buf, "%x %x", &offset, &value);

	if (2 != c) {
		printk("Usage:\nOffset [Value]\n");
	} else {
		printk("Register 0x%02X is 0x%02X.\n", offset, value);
		WriteByteTPI(offset, value);
	}

	return count;
}
static DEVICE_ATTR(registers, (S_IRUGO | S_IWUSR), show_registers, store_registers);

#if (IS_CEC == 1)
static ssize_t show_cpi(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, num = 0;
	u32 start, end;

	start = 0x880;
	end = 0x8E7;


	num += snprintf(buf + num, PAGE_SIZE - num, "\n        ");
	for (i = 0; i <= 0xF; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", i);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
			num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.2x: ", i);

		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", SiIRegioRead(i));
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	return num;
}

static ssize_t store_cpi(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 offset, value, c;

	c = sscanf(buf, "%x %x", &offset, &value);

	if (c != 2) {
		printk("Usage:\nOffset [Value]\n");
	} else {
		printk("Register 0x%02X is 0x%02X.\n", offset, value);
		SiIRegioWrite(offset, value);

		{
			u32 r;

			r = SiIRegioRead(offset);

			if (r != value)
				printk("write Register fail: should be 0x%02X but 0x%02X.\n", value, r);

		}
	}

	return count;
}
static DEVICE_ATTR(cpi, (S_IRUGO | S_IWUSR), show_cpi, store_cpi);

static ssize_t show_cinfo(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0, i;

	u32 data, offset;

	offset = 0x881; data = SiIRegioRead(offset);
	num += snprintf(buf + num, PAGE_SIZE - num, "CEC spec 1.%d (0x%.2x)\n", data & 0xF, data);
	offset = 0x883; data = SiIRegioRead(offset);
	num += snprintf(buf + num, PAGE_SIZE - num, "CEC core version 0x%.2x\n", data);

	num += snprintf(buf + num, PAGE_SIZE - num, "Capture Devices:\n");
	for (i = 0; i <= 0xF; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " %.2d", i);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	offset = 0x8A2; data = SiIRegioRead(offset);
	for (i = 0; i <= 0x7; i++)
		if ((0x1 << i) & data)
			num += snprintf(buf + num, PAGE_SIZE - num, "  Y");
		else
			num += snprintf(buf + num, PAGE_SIZE - num, "  N");
	offset = 0x8A3; data = SiIRegioRead(offset);
	for (i = 0; i <= 0x7; i++)
		if ((0x1 << i) & data)
			num += snprintf(buf + num, PAGE_SIZE - num, "  Y");
		else
			num += snprintf(buf + num, PAGE_SIZE - num, "  N");
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");


	return num;
}
static DEVICE_ATTR(cinfo, (S_IRUGO), show_cinfo, NULL);

static ssize_t show_topology(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "0x%.4x\n", drv_data->cec_topology);

	return num;
}
static DEVICE_ATTR(topology, (S_IRUGO), show_topology, NULL);

static ssize_t store_pa_req(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	queue_work(drv_data->wq, &drv_data->work_cec_physical_address);

	return count;
}
static DEVICE_ATTR(pa_req, S_IWUSR, NULL, store_pa_req);

static ssize_t show_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", si_cpi_discovery_timeout());

	return num;
}

static ssize_t store_timeout(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
		si_cpi_discovery_timeout_cfg(cfg);

	return count;
}
static DEVICE_ATTR(discovery_timeout, (S_IWUSR | S_IRUGO), show_timeout, store_timeout);
#endif

static ssize_t show_no_tmds_disable(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", no_tmds_disable());

	return num;
}

static ssize_t store_no_tmds_disable(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (1 <= c) {
		if (cfg)
			no_tmds_disable_cfg(1);
		else
			no_tmds_disable_cfg(0);
	}
	return count;
}
static DEVICE_ATTR(no_tmds_disable, (S_IRUGO | S_IWUSR), show_no_tmds_disable, store_no_tmds_disable);

static ssize_t show_no_tmds_stable_check(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", no_tmds_stable_check);

	return num;
}

static ssize_t store_no_tmds_stable_check(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1) {
		if (cfg)
			no_tmds_stable_check = 1;
		else
			no_tmds_stable_check = 0;
	}
	return count;
}
static DEVICE_ATTR(no_tmds_stable_check, (S_IRUGO | S_IWUSR), show_no_tmds_stable_check, store_no_tmds_stable_check);

static ssize_t show_clk_edge_select(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", sii9136_clk_edge());

	return num;
}

static ssize_t store_clk_edge_select(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
		sii9136_clk_edge_cfg(cfg);

	return count;
}
static DEVICE_ATTR(clk_edge_select, (S_IRUGO | S_IWUSR), show_clk_edge_select, store_clk_edge_select);


static ssize_t show_debug_level(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", sii9136_debug_level());

	return num;
}

static ssize_t store_debug_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
	       sii9136_debug_level_cfg(cfg);

	return count;
}
static DEVICE_ATTR(debug_level, (S_IRUGO | S_IWUSR), show_debug_level, store_debug_level);

static ssize_t show_sw_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "globalFlags 0x%.4x\n", globalFlags);
	num += snprintf(buf + num, PAGE_SIZE - num, "tmdsPoweredUp %d\n", tmdsPoweredUp);
	num += snprintf(buf + num, PAGE_SIZE - num, "dsRxPoweredUp %d\n", dsRxPoweredUp);
	num += snprintf(buf + num, PAGE_SIZE - num, "HDCP_Started %d\n", HDCP_Started);
	num += snprintf(buf + num, PAGE_SIZE - num, "edidDataValid %d\n", edidDataValid);
	num += snprintf(buf + num, PAGE_SIZE - num, "HDCPIsAuthenticated %d\n", HDCPIsAuthenticated);
	num += snprintf(buf + num, PAGE_SIZE - num, "CableConnected %d\n", CableConnected);

	{
		int i;

		num += snprintf(buf + num, PAGE_SIZE - num, "EDID:\n");
		for (i = 0; i < 256; i++) {
			if ((i & 0xF) == 0)
				num += snprintf(buf + num, PAGE_SIZE - num, "\n");
			num += snprintf(buf + num, PAGE_SIZE - num, " %.2x", global_sink_edid[i]);

		}
		num += snprintf(buf + num, PAGE_SIZE - num, "\n", CableConnected);

	}

	return num;
}
static DEVICE_ATTR(sw_info, (S_IRUGO), show_sw_info, NULL);

#if defined(RESET_TEST)
static ssize_t show_reset_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", reset_test());

	return num;
}

static ssize_t store_reset_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
		reset_test_cfg(cfg);

	return count;
}
static DEVICE_ATTR(reset_test, (S_IRUGO | S_IWUSR), show_reset_test, store_reset_test);
#endif /* #if defined(RESET_TEST) */

static ssize_t show_pixel_repetition_by_avi(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", drv_data->pixel_repetition_by_avi);
	num += snprintf(buf + num, PAGE_SIZE - num, "\nEXPERIMENTAL! DO NOT CHANGE THIS SETTING UNLESS YOU HAVE GOOD CAUSE TO DO SO!\n\n");

	return num;
}

static ssize_t store_pixel_repetition_by_avi(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1) {
		if (cfg)
			drv_data->pixel_repetition_by_avi = 1;
		else
			drv_data->pixel_repetition_by_avi = 0;
	}

	return count;
}
static DEVICE_ATTR(pixel_repetition_by_avi, (S_IRUGO | S_IWUSR), show_pixel_repetition_by_avi, store_pixel_repetition_by_avi);

static ssize_t show_tv_access(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", tv_access_flag);
	return num;
}

static DEVICE_ATTR(tv_access, S_IRUGO, show_tv_access, NULL);

static struct attribute *dev_attrs[] = {
	&dev_attr_registers.attr,
#if (IS_CEC == 1)
	&dev_attr_cpi.attr,
	&dev_attr_cinfo.attr,
	&dev_attr_topology.attr,
	&dev_attr_pa_req.attr,
	&dev_attr_discovery_timeout.attr,
#endif
	&dev_attr_no_tmds_disable.attr,
	&dev_attr_no_tmds_stable_check.attr,
	&dev_attr_clk_edge_select.attr,
	&dev_attr_debug_level.attr,
	&dev_attr_sw_info.attr,
	&dev_attr_tv_access.attr,
#if defined(RESET_TEST)
	&dev_attr_reset_test.attr,
#endif
	&dev_attr_pixel_repetition_by_avi.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static void sii9136_fsm_timer(struct sii9136_drv_data *d)
{
	sii9136_fsm_timer_handler(d);

#if NO_INT_MODE
	if (d->wq)
		queue_delayed_work(d->wq, &d->timerwork, msecs_to_jiffies(LOOP_MSEC));
#else
	ENABLE_INT();
#endif
}

#if (IS_CEC == 1)
static void sii9136_cec_tx(struct sii9136_drv_data *d)
{
	sii9136_cec_tx_handler(d);
}

static void sii9136_cec_topology_timer(struct sii9136_drv_data *d)
{
	if (d->wq)
		queue_delayed_work(d->wq, &d->work_cec_topology, msecs_to_jiffies(10*1000));

	sii9136_cec_topology_handler(d);
}

static void sii9136_cec_request_pa(struct sii9136_drv_data *d)
{
	sii9136_cec_request_pa_handler(d);
}
#endif

static int destroy_works_and_thread(struct sii9136_drv_data *d)
{
	struct workqueue_struct *wq = d->wq;

	if (wq) {
		d->wq = NULL;
		cancel_delayed_work(&d->timerwork);
#if (IS_CEC == 1)
		cancel_delayed_work(&d->work_cec_tx);
		cancel_delayed_work(&d->work_cec_topology);
		cancel_delayed_work(&d->work_cec_physical_address);
#endif
		flush_workqueue(wq);
		destroy_workqueue(wq);
	}

	return 0;
}

static int create_works_and_thread(struct sii9136_drv_data *d)
{
	struct workqueue_struct *wq;

	/* timer handler init. */
	wq = create_singlethread_workqueue("SiI9136_wq");
	if (!wq) {
		uerr("Failed to allocate wq?!\n");
		goto err;
	}

	BUG_ON(wq == NULL);
	d->wq = wq;

	INIT_WORK(&d->timerwork, (void (*)(void *))sii9136_fsm_timer, d);
	INIT_WORK(&d->setup_video_task, (void (*)(void *))__sii9136_set_video, d);
	INIT_WORK(&d->setup_audio_task, (void (*)(void *))__sii9136_set_audio, d);
	INIT_WORK(&d->hdcp_auth_start_task, (void (*)(void *))hdcp1_auth_start, d);
	INIT_WORK(&d->hdcp_auth_stop_task, (void (*)(void *))hdcp1_auth_stop, d);
	INIT_WORK(&d->disable_task, (void (*)(void *))__sii9136_disable, d);
#if (IS_CEC == 1)
	INIT_WORK(&d->work_cec_tx, (void (*)(void *))sii9136_cec_tx, d);
	INIT_WORK(&d->work_cec_topology, (void (*)(void *))sii9136_cec_topology_timer, d);
	INIT_WORK(&d->work_cec_physical_address, (void (*)(void *))sii9136_cec_request_pa, d);
#endif

	return 0;
err:
	return -ENOMEM;
}

#if (!NO_INT_MODE)
static irqreturn_t sii9136_intr(int irq, void *dev_id, struct pt_regs *regs)
{
	/* check share interrupt */
	if (!gpio_get_int_stat(GPIO_CAT6613_INT))
		return IRQ_NONE;

	DISABLE_INT();

	/* ack interrupt */
	gpio_ack_int_stat(GPIO_CAT6613_INT);

	/* IRQ Handle */
	queue_work(drv_data->wq, &drv_data->timerwork);

	return IRQ_HANDLED;
}

static int intr_init(struct sii9136_drv_data *d)
{
	/* request IRQ */
	if (request_irq (INT_GPIO, &sii9136_intr, SA_SHIRQ, MODULE_NAME, d))
		return -1;

	/* enable interrupt after registering ISR */
	ENABLE_INT();

	/* clear pending interurpt execept hot-plug */
	WriteByteTPI(0x3D, ReadByteTPI(0x3D) & 0xFE);

	/* enable HPD interurpt */
	WriteByteTPI(0x3C, 1);

	return 0;
}
#endif

#if defined(CONFIG_AST1500_SII9678)
#define EXTERNAL_HDCP_22_TX
static void external_chip_hw_reset_for_sii9678(void)
{
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_TX);
	msleep(350);
}
#define EXTERNAL_HDCP_22_TX_RESET external_chip_hw_reset_for_sii9678
#endif

static int __devinit sii9136_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct sii9136_drv_data *d;

	/*	** Note! Probe fail won't trigger dev remove! WTF Linux driver framework.	*/
	uinfo("Hello probe!\n");
	//BruceToDo. Do the probe thing. Ask SCU driver for information?


	/* FIXME
	 * this is software workaround for multiple transmitters on a single board
	 */
	if (is_xmiter_info_registered()) {
		uinfo("Already have another transmitter registered !\n");
		goto out;
	}

	/* Do HW reset */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDMI_TX);


	/* I2C init */
	I2CInit(I2C_HDMI_TX, I2C_SII9136_SPEED);
	if (!is_dev_exists()) {
		//Just ignore it when device not exists. BruceToCheck. Should return -ENODEV?
		uerr("SiI9136-3 device not exists?!\n");
		goto out;
	}
	d = kzalloc(sizeof(struct sii9136_drv_data), GFP_KERNEL);
	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}

	sii9136_drv_init(d);

	if (sii9136_dev_init(d)) {
		uerr("Device init failed?!\n");
		ret = -ENOMEM;
		goto err2;
	}

	if (create_works_and_thread(d)) {
		goto err2;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		BUG();
		goto err1;
	}

	platform_set_drvdata(pdev, (void*)d);

	xmiter_info_init();

	drv_data = d;

	/* Start SiI9136 state machine */
#if (NO_INT_MODE)
	queue_delayed_work(d->wq, &d->timerwork, msecs_to_jiffies(LOOP_MSEC));
#else
	if (intr_init(d)) {
		drv_data = NULL;
		xmiter_info_fini();
		platform_set_drvdata(pdev, NULL);
		goto err1;
	}
#endif

#if defined(EXTERNAL_HDCP_22_TX)
	/*
	 * move HDCP2.2 TX reset after ISR got registered
	 * we'll miss the fisrt HPD event from SiI9678 if we reset SiI9678 before intr_init
	 */
	EXTERNAL_HDCP_22_TX_RESET();
#endif
#if (IS_CEC == 1)
	queue_work(drv_data->wq, &drv_data->work_cec_topology);
#endif
out:
	return ret;

err1:
	destroy_works_and_thread(d);
err2:
	if (d)
		kfree(d);
err:
	return ret;
}

static int __devexit sii9136_remove(struct platform_device *pdev)
{
	struct sii9136_drv_data *d = platform_get_drvdata(pdev);
	uinfo("Hello remove!\n");
	if (!d)
		return 0;

	drv_data = NULL;

	xmiter_info_fini();

	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);
	//BruceToDo. move to _remove_workqueue()
	destroy_works_and_thread(d);
	kfree(d);

	return 0;
}

static struct platform_driver sii9136_driver = {
	.probe		= sii9136_probe,
	.remove		= __devexit_p(sii9136_remove),
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
		.name	= MODULE_NAME,
	},
};

static int __init sii_init(void)
{
	int ret;

	uinfo("SiI9136 driver based on Silicon Image sample driver 1081 Ver8.04\n");
	pdev = platform_device_register_simple(
	                           MODULE_NAME, /* driver name string */
	                           -1,          /* id */
	                           NULL,        /* struct resource to alloc */
	                           0);
	/* resource number */
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out;
	}
	ret = platform_driver_register(&sii9136_driver);
	if (ret == 0)
		goto out;

	platform_device_unregister(pdev);

out:
	uinfo("init done\n");
	return ret;
}

static void __exit sii_exit(void)
{
	platform_driver_unregister(&sii9136_driver);
	platform_device_unregister(pdev);
}

#ifndef MODULE
arch_initcall(sii_init);
#else
module_init(sii_init);
#endif
module_exit(sii_exit);
