/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VE_ENCODE_H_
#define _VE_ENCODE_H_
#ifdef CONFIG_ARCH_AST1500_HOST

#include "util.h" /* SetVideoReg() */
#include "vreg.h"
/*
** Stream buffer pointers:
** write pointer: VIDEO1_BUF_CODEC_OFFSET_READ. HW encoded position.
** frame end write pointer: VIDEO1_COMPRESS_FRAME_END_READ. HW's last encoded frame end position.
** interrupt ack pointer: VIDEO1_COMPRESS_BUF_PROCESS_OFFSET_REG. SW tell HW the position it is 'processing' so that VE won't interrupt again.
** read pointer: VIDEO1_COMPRESS_BUF_READ_OFFSET_REG. The typical read pointer which SW tell HW the position it is processed and can be reused by HW.
**
** rd_ptr --> int_ack_ptr --> frame_end_wr_ptr --> wr_ptr
*/
#define ve_encd_stream_get_wr_ptr() \
({ \
	u32 ptr = GetVideoReg(VIDEO1_BUF_CODEC_OFFSET_READ + VE_REG_OFFSET) & BUF_CODEC_OFFSET_MASK; \
	ptr; \
})

#define ve_encd_stream_get_frame_end_wr_ptr() \
({ \
	u32 ptr = GetVideoReg(VIDEO1_COMPRESS_FRAME_END_READ + VE_REG_OFFSET) & COMPRESS_FRAME_END_READ_MASK; \
	ptr; \
})
#define ve_encd_stream_set_int_ack_ptr(ptr) do { SetVideoReg(VIDEO1_COMPRESS_BUF_PROCESS_OFFSET_REG + VE_REG_OFFSET, (ptr)); mb(); } while (0)
#define ve_encd_stream_set_rd_ptr(ptr) do { SetVideoReg(VIDEO1_COMPRESS_BUF_READ_OFFSET_REG + VE_REG_OFFSET, (ptr)); mb(); } while (0)


unsigned int ve_encd_capture_polling(u32 *rd_ptr, unsigned int *end_cmd, u32 *time_stamp);
void ve_encd_set_buffers(u32 vbuf1_en_source0_phy_base, u32 vbuf1_en_source1_phy_base, u32 vbuf1_en_stream_phy_base, u32 vbuf1_en_crc_phy_base, u32 vbuf1_en_flag_phy_base);


#if (CONFIG_AST1500_SOC_VER >= 2)
void ve_encd_dither_detection(void);
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */


#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */
#endif /* #ifndef _VE_ENCODE_H_ */

