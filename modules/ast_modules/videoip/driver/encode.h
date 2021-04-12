/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _ENCODE_H_
#define _ENCODE_H_

void AllocateEncodeBufHost(void);
int CheckOnStartHost(void);
void startVE_host(int no_data_int);
void stopVEHost(int stopWatchDog);
void HostV1Reset(int reset_ve);
void antiDither(unsigned int threshold);
int insertFullFrame(void);
unsigned int HostLoopbackOn(void);
unsigned int HostLoopbackOff(void);

void initialize_source_buffers(u32 color);
void ve_frame_rate_update(void);
void pack_q_cfg(PQCFG s, PQUALITY_CONFIG q_cfg);
void _prepare_for_capture(void);
void set_max_bit_rate(unsigned int max_mbps);
int suspend_host_ve(unsigned int timeout_ms);
void ve_apply_frame_rate_control(void);
void sw_watchdog_timer(unsigned long data);
void full_jpeg_timer(unsigned long data);
void setup_capture_timing_gen(
	PVIDEO_INFO pCurVideoInfo,
	unsigned int HStart,
	unsigned int HA,
	unsigned int VStart,
	unsigned int VA,
	eVScanMode ScanMode,
	PVIDEO_MODE_INFO pSourceModeInfo);

#if (CONFIG_AST1500_SOC_VER >= 2)
void change_QualityParam_BCD(unsigned int threshold);
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
void host_quick_reset(void);
void handle_encode_ve_hang(VIDEOIP_DEVICE *v);
#endif

void update_infoframe(u32 type);

#endif /* #ifndef _ENCODE_H_ */
