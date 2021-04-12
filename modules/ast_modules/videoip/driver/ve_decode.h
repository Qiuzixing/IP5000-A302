/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VE_DECODE_H_
#define _VE_DECODE_H_
#ifdef CONFIG_ARCH_AST1500_CLIENT
/*
** Stream buffer pointers for decode:
** write pointer: VIDEO1_DECOMPRESS_BUF_WRITE_OFFSET_REG. SW write position.
** read pointer: VIDEO1_BUF_CODEC_OFFSET_READ. HW read position.
**
** SW write for decode, HW read to wr_ptr.
** rd_ptr --> wr_ptr
*/
#define ve_decd_stream_get_rd_ptr() \
({ \
	u32 ptr = GetVideoReg(VIDEO1_BUF_CODEC_OFFSET_READ); \
	ptr; \
})

#define ve_decd_stream_get_wr_ptr() \
({ \
	u32 ptr = GetVideoReg(VIDEO1_DECOMPRESS_BUF_WRITE_OFFSET_REG); \
	ptr; \
})

#define ve_decd_stream_set_wr_ptr(ptr) do { SetVideoReg(VIDEO1_DECOMPRESS_BUF_WRITE_OFFSET_REG, (ptr)); mb(); } while (0)

#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */
#endif /* #ifndef _VE_DECODE_H_ */
