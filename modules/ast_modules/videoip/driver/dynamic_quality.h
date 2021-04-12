/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _DYNAMIC_QUALITY_H_
#define _DYNAMIC_QUALITY_H_

#include "global.h" /* VIDEOIP_DEVICE */

#ifdef CONFIG_ARCH_AST1500_HOST
void dynamic_quality_factors_encode_init(int quality_mode);
void stream_quality_adjust_per_frame_end(void);
void stream_quality_adjust_per_packet(void);

#if (CONFIG_AST1500_SOC_VER >= 3)
void desc_quality_adjust(VIDEOIP_DEVICE * v);
#endif

#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#ifdef CONFIG_ARCH_AST1500_CLIENT
void dynamic_quality_factors_decode_set(int quality_mode);
void dynamic_quality_factors_decode_init(int quality_mode);
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */


#endif /* #ifndef _DYNAMIC_QUALITY_H_ */
