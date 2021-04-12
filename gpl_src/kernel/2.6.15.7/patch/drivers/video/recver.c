/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 *
 */
#if defined(CONFIG_ARCH_AST1500_HOST)
#include <linux/module.h>
#include <aspeed/features.h>
#include <asm/arch/drivers/video_hal.h>

static struct recver_info recver;

static PFN_UPDATE_VIDEO_INFO video_event;
static PFN_UPDATE_AUDIO_INFO audio_event;
static PFN_UPDATE_INFOFRAME infoframe;
static PFN_VIDEO_PIXEL_CLK pixel_clk;
static PFN_CECIF_RX _cecif_rx;
#if SUPPORT_HDCP_REPEATER
static pfn_HDCP1_Upstream_Port_Auth_Request_Callback hdcp_auth_request;
#endif

/*
 * video driver register/deregister callback function for recever
 */

/* video */
void vrxhal_reg_video_event_callback(PFN_UPDATE_VIDEO_INFO cb)
{
	video_event = cb;

	if (recver.video_update)
		recver.video_update();
}
EXPORT_SYMBOL(vrxhal_reg_video_event_callback);

void vrxhal_dereg_video_event_callback(void)
{
	video_event = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_video_event_callback);

void vrx_video_event(unsigned int status)
{
	if (video_event)
		video_event(status);
}
EXPORT_SYMBOL(vrx_video_event);

/* pixel clock */
void vrxhal_reg_pixel_clk_callback(PFN_VIDEO_PIXEL_CLK cb)
{
	pixel_clk = cb;
}
EXPORT_SYMBOL(vrxhal_reg_pixel_clk_callback);

void vrxhal_dereg_pixel_clk_callback(void)
{
	pixel_clk = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_pixel_clk_callback);

int vrxhal_pixel_clk(void)
{
	if (pixel_clk)
		return pixel_clk();

	return 0;
}
EXPORT_SYMBOL(vrxhal_pixel_clk);

/* infoframe */
/* XXX: cat6023 just calling this witg VSDB but it6802 do nothing about this */
void vrxhal_reg_infoframe_callback(PFN_UPDATE_INFOFRAME cb)
{
	infoframe = cb;
}
EXPORT_SYMBOL(vrxhal_reg_infoframe_callback);

void vrxhal_dereg_infoframe_callback(void)
{
	infoframe = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_infoframe_callback);

/*
 * notify video driver to get inforframe by specied type
 */
void vrx_infoframe(e_vrxhal_infoframe_type type)
{
	if (infoframe)
		infoframe(type);
}
EXPORT_SYMBOL(vrx_infoframe);

/* audio */
void vrxhal_reg_audio_event_callback(PFN_UPDATE_AUDIO_INFO cb)
{
	audio_event = cb;

	if (recver.audio_update)
		recver.audio_update();
}
EXPORT_SYMBOL(vrxhal_reg_audio_event_callback);

void vrxhal_dereg_audio_event_callback(void)
{
	audio_event = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_audio_event_callback);

void vrx_audio_event(Audio_Info info)
{
	if (audio_event)
		audio_event(info);
}
EXPORT_SYMBOL(vrx_audio_event);


#if SUPPORT_HDCP_REPEATER
void vrxhal_HDCP1_reg_upstream_port_auth_request_callback(pfn_HDCP1_Upstream_Port_Auth_Request_Callback cb)
{
	hdcp_auth_request = cb;
}
EXPORT_SYMBOL(vrxhal_HDCP1_reg_upstream_port_auth_request_callback);

void vrxhal_HDCP1_dereg_upstream_port_auth_request_callback(void)
{
	hdcp_auth_request = NULL;
}
EXPORT_SYMBOL(vrxhal_HDCP1_dereg_upstream_port_auth_request_callback);


void vrx_hdcp_auth_request(unsigned int enable)
{
	if (hdcp_auth_request)
		hdcp_auth_request(enable);
}
EXPORT_SYMBOL(vrx_hdcp_auth_request);
#endif

/*
 * cec driver register/deregister callback function for recever
 */
void reg_cecif_rx_callback(PFN_CECIF_RX cb)
{
	_cecif_rx = cb;
}
EXPORT_SYMBOL(reg_cecif_rx_callback);

void dereg_cecif_rx_callback(void)
{
	_cecif_rx = NULL;
}
EXPORT_SYMBOL(dereg_cecif_rx_callback);

void cecif_rx(unsigned char *buf, unsigned int size)
{
	if (_cecif_rx)
		_cecif_rx(buf, size);
}
EXPORT_SYMBOL(cecif_rx);

/*
 * receiver driver interface
 */
int vrx_register_info(struct recver_info *info)
{
	memcpy(&recver, info, sizeof(struct recver_info));
	return 0;
}
EXPORT_SYMBOL(vrx_register_info);

int vrxhal_is_dev_exist(void)
{
	if (recver.dev_exist)
		return recver.dev_exist();

	return 0;
}
EXPORT_SYMBOL(vrxhal_is_dev_exist);

void vrxhal_sync_audio(int force_update)
{
	if (recver.sync_audio)
		recver.sync_audio(force_update);

}
EXPORT_SYMBOL(vrxhal_sync_audio);

int vrxhal_get_video_timing_info(void *mode_info)
{
	if (recver.video_timing_info)
		return recver.video_timing_info(mode_info);

	return -1; /* return -1 let caller take default */
}
EXPORT_SYMBOL(vrxhal_get_video_timing_info);

int vrxhal_get_video_avi_info(unsigned char *pdata)
{
	if (recver.video_avi_info)
		return recver.video_avi_info(pdata);

	return 0;
}
EXPORT_SYMBOL(vrxhal_get_video_avi_info);

int vrxhal_get_video_vsd_info(unsigned char *pdata)
{
	if (recver.video_vsd_info)
		return recver.video_vsd_info(pdata);

	return 0;
}
EXPORT_SYMBOL(vrxhal_get_video_vsd_info);

int vrxhal_get_video_hdr_info(unsigned char *pdata)
{
	if (recver.video_hdr_info)
		return recver.video_hdr_info(pdata);

	return 0;
}
EXPORT_SYMBOL(vrxhal_get_video_hdr_info);

int vrxhal_get_video_hdmi_n_cts(unsigned int *pn, unsigned int *pcts)
{
	if (recver.hdmi_n_cts)
		return recver.hdmi_n_cts(pn, pcts);

	*pn = 0;
	*pcts = 0;

	return 0;
}
EXPORT_SYMBOL(vrxhal_get_video_hdmi_n_cts);

int vrxhal_hdmi_hpd_ctrl(unsigned int level)
{
	if (recver.hdmi_hpd_ctrl)
		return recver.hdmi_hpd_ctrl(level);

	/* hdmi_hpd_ctrl is not available, return -1 */
	return -1;
}
EXPORT_SYMBOL(vrxhal_hdmi_hpd_ctrl);

int vrxhal_cec_send(unsigned char *pdata, unsigned int size)
{
	if (recver.cec_send)
		return recver.cec_send(pdata, size);

	return 0;
}
EXPORT_SYMBOL(vrxhal_cec_send);

int vrxhal_cec_topology(unsigned char *pdata)
{
	if (recver.cec_topology)
		return recver.cec_topology(pdata);

	return -1;
}
EXPORT_SYMBOL(vrxhal_cec_topology);

void vrxhal_HDCP1_set_upstream_port_mode(unsigned char repeater)
{
	if (recver.hdcp_mode)
		recver.hdcp_mode(repeater);
}
EXPORT_SYMBOL(vrxhal_HDCP1_set_upstream_port_mode);


void vrxhal_HDCP1_set_upstream_port_auth_status(unsigned int state, unsigned short Bstatus, void *KSV_list)
{
	if (recver.hdcp_status)
		recver.hdcp_status(state, Bstatus, KSV_list);
}
EXPORT_SYMBOL(vrxhal_HDCP1_set_upstream_port_auth_status);
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */
