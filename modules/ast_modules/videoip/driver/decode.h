/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _DECODE_H_
#define _DECODE_H_

#define VE_RESET_OPT_NONE		0x0
#define VE_RESET_OPT_CLEAR_INFO	0x1
#define VE_RESET_OPT_QUICK		(VE_RESET_OPT_NONE)
#define VE_RESET_OPT_DEFAULT	(VE_RESET_OPT_CLEAR_INFO)

int CheckOnStartClient(void);
int isVEStartedClient(void);
void stopVEClient(void);
void ClientV1Reset(unsigned int option);
void ClientStopVideo(void);
void ClientSetupVideo(LpPacketInfo pRecvInfo);
int ClientHandleModeChg(LpPacketInfo pRecvInfo);
void setVEDestBuffer(void);
void startVE_client(void);
int InitializeVideoEngineClient(
                               PGENERAL_INFO      pGeneralInfo,
                               PVIDEO_INFO          pVideoInfo,
                               unsigned int            CompressMode);
int PreDecodeClient(
			   PCAPTURE_INFO pCap_info,
			   int              nCRTIndex,
			   eVIDEO_FORMAT    ColorFormat,
			   u8             CenterMode,
			   PVIDEO_INFO      lpCurVideoInfo,
			   PGENERAL_INFO    lpGeneralInfo);


void do_skip_frame(int req_full_frame, int why);
void hdmi_force_rgb_output_cfg(u32 cfg);
u32 hdmi_force_rgb_output(void);
void hdmi_hdr_mode_off_cfg(u32 cfg);
u32 hdmi_hdr_mode_off(void);
void create_q_factors(PQ_FACTOR pFactors, PCAPTURE_INFO pCapInfo, PQUALITY_CONFIG pQCfg);
void sw_watchdog_timer(unsigned long data);
void full_jpeg_timer(unsigned long data);
void pwr_save(unsigned long data);
void trigger_client_mode_chg(void);

#if SYNC_WITH_VSYNC
void client_dequeue_frame(void);
void chk_pwr_save(unsigned long *j);
#endif

#if (3 <= CONFIG_AST1500_SOC_VER)
void quick_ve_reset_decode(VIDEOIP_DEVICE *v);
void V1RxClient_loop_desc(struct videoip_task *ut);
void on_crt_scaling_change(unsigned int line, unsigned int factor, eVIDEO_ROTATE rotate);
void _on_crt_scaling_change(void *crt_chg);

/* SW flip related. */
void client_crt_csc_fixup(PCAPTURE_INFO pCap_info);
void sw_flip_interlace_display_address(VIDEOIP_DEVICE *v, u32 odd);
u32 profile_config(u32 index);
#endif /* #if (3 <= CONFIG_AST1500_SOC_VER) */

#endif /* #ifndef _DECODE_H_ */
