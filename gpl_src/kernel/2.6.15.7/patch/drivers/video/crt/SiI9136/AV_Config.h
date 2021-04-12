/*
 *****************************************************************************
 *
 * Copyright 2002-2009, 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  AV_Config.h
 *
 * @brief Audio and video configurations.
 *
 *****************************************************************************
*/
#ifndef _AV_CONFIG_H_ //{
#define _AV_CONFIG_H_
enum AV_ConfigErrorCodes
{
    DE_CANNOT_BE_SET_WITH_EMBEDDED_SYNC,
    V_MODE_NOT_SUPPORTED,
    SET_EMBEDDED_SYC_FAILURE,
    I2S_MAPPING_SUCCESSFUL,
    I2S_INPUT_CONFIG_SUCCESSFUL,
    I2S_HEADER_SET_SUCCESSFUL,
    EHDMI_ARC_SINGLE_SET_SUCCESSFUL,
    EHDMI_ARC_COMMON_SET_SUCCESSFUL,
    EHDMI_HEC_SET_SUCCESSFUL,
    EHDMI_ARC_CM_WITH_HEC_SET_SUCCESSFUL,
    AUD_MODE_NOT_SUPPORTED,
    I2S_NOT_SET,
    DE_SET_OK,
    VIDEO_MODE_SET_OK,
    AUDIO_MODE_SET_OK,
    GBD_SET_SUCCESSFULLY,
    DE_CANNOT_BE_SET_WITH_3D_MODE,
};

// Time Constants Used in AV_Config.c only
//========================================
#define T_RES_CHANGE_DELAY      128         // delay between turning TMDS bus off and changing output resolution

// Deep Color Constants
//=====================
#define DC_48                                   0x01

// Prototype Declarations
//=======================
void SetBasicAudio(void);
uint8_t InitVideo(uint8_t);
void SetFormat(uint8_t *Data);
uint8_t SetEmbeddedSync(void);
void EnableEmbeddedSync(void);
uint8_t SetDE(void);

uint8_t ChangeVideoMode(void);
uint8_t MapI2S(void);
uint8_t ConfigI2SInput(void);
uint8_t SetAudioMode(void);

uint8_t SetI2S_StreamHeader(void);
uint8_t SetEHDMI_ARC_sm(void);
uint8_t SetEHDMI_ARC_cm(void);
uint8_t SetEHDMI_HEC(void);
uint8_t SetEHDMI_ARCcm_with_HEC(void);

uint8_t IsVideoModeSupported(uint8_t);
uint8_t IsAudioModeSupported(uint8_t);
uint8_t SetAVI_InfoFrames(AVIInfoFrameData_t *aviInfoFrame);
uint8_t SetAudioInfoFrames(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

uint8_t SetGBD_InfoFrame(void);

void Set_VSIF(void);
#define Call_Set_VSIF   Set_VSIF();
#define Call_DoEdidRead DoEdidRead();

#if (AST_HDMITX)
#include "SiI9136_main.h"
void sii9136_video_mode(u16 pixel_clock, u16 vfreq, u16 pixels , u16 lines);
void sii9136_video_mode_input_bus_pixel_rep(u8 tclk_sel, u8 input_bus_sel, u8 edge_select, u8 pixel_repetition);
void sii9136_video_format(u32 input_color_space, u32 output_format, u32 color_space_std, u32 color_depth, u32 hdmi, u32 input_range_expansion, u32 output_range_compression);
void sii9136_output_hdmi_cfg(u32 enable);
u32 sii9136_output_hdmi(void);
void sii9136_clk_edge_cfg(u32 cfg);
u32 sii9136_clk_edge(void);

void sii9136_set_avi_info_frame(u8 *content);
void sii9136_set_vendor_specific_info_frame(u8 *frame);
void sii9136_set_hdr_info_frame(u8 *frame);
void sii9136_set_audio_info_frame(u8 *content);

void sii9136_disable_avi_if(void);
void sii9136_disable_vendor_specific_if(void);
void sii9136_disable_hdr_if(void);
void sii9136_disable_audio_if(void);

void sii9136_set_audio_i2s(struct sii9136_audio_data *d);

#endif

#endif //}
