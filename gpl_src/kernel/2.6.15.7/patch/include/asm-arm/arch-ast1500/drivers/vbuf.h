#ifndef _VBUG_H_
#define _VBUF_H_

#include <asm/arch/memory.h>
#include <asm/page.h>
#include <aspeed/features.h>

#define __TOTAL_BLOCKS ((((DISPLAY_MAX_X+7)&~7)/8)*(((DISPLAY_MAX_Y+7)&~7)/8))
#define __A8(x) (((x)+0x7)&~0x7) //8 align

#if (CONFIG_AST1500_SOC_VER >= 3)

#define DISPLAY_MAX_X		4096 /* in pixel */
#define DISPLAY_MAX_Y		2160 /* in pixel */
#define OSD_DISPLAY_MAX_X	1280 /* in pixel */
#define OSD_DISPLAY_MAX_Y	720 /* in pixel */
/*
  Host:
  +----------------------+ PHYS_OFFSET
  |Host_TOTAL_SIZE       |
  |                 112M |
  +----------------------+ VBUF_EN_PHY_BASE, VBUF1_EN_PHY_BASE, VBUF1_EN_SOURCE0_PHY_BASE
  |VIDEO_SOURCE_SIZE     |    VIDEO_SOURCE_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000,
  |               33.75M |
  +----------------------+ VBUF1_EN_SOURCE1_PHY_BASE
  |VIDEO_SOURCE_SIZE     |    VIDEO_SOURCE_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000,
  |               33.75M |
  +----------------------+ VBUF1_EN_STREAM_PHY_BASE
  |VIDEO_MAX_STREAM_SIZE |
  |                   8M |
  +----------------------+ VBUF1_EN_CRC_PHY_BASE
  |VIDEO_CRC_SIZE        |    VIDEO_CRC_SIZE = 0
  |                    0 |
  +----------------------+ VBUF1_EN_FLAG_PHY_BASE
  |VIDEO_FLAG_SIZE       |    VIDEO_FLAG_SIZE = __TOTAL_BLOCKS*4 = 0x00087000
  |                 540K |
  +----------------------+ VBUF1_EN_SRCBKUP_PHY_BASE
  |VBUF1_EN_SRCBKUP_SIZE |    VBUF1_EN_SRCBKUP_SIZE = VIDEO_SOURCE_SIZE
  |               33.75M |
  +----------------------+ VBUF1_EN_SNAPSHOT_PHY_BASE
  |VBUF1_EN_SNAPSHOT_SIZE|    VBUF1_EN_SNAPSHOT_SIZE = VIDEO_SOURCE_SIZE
  |               33.75M |
  +----------------------+

  Client:
  +----------------------+ PHYS_OFFSET
  |STATION_TOTAL_SIZE    |
  |                 112M |
  +----------------------+ VBUF_DE_PHY_BASE, VBUF1_DE_PHY_BASE, VBUF1_DE_STREAM_PHY_BASE
  |VIDEO_MAX_STREAM_SIZE |
  |                   8M |
  +----------------------+ VBUF1_DE_DEST0_PHY_BASE
  |VIDEO_DESTINATION_SIZE|    VIDEO_DESTINATION_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000,
  |               33.75M |
  +----------------------+ VBUF1_DE_DEST1_PHY_BASE
  |VIDEO_DESTINATION_SIZE|    VIDEO_DESTINATION_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000,
  |               33.75M |
  +----------------------+ VBUF1_DE_FLAG_PHY_BASE
  |VIDEO_FLAG_SIZE       |    VIDEO_FLAG_SIZE = __TOTAL_BLOCKS*1 = 0x00021C00
  |                 135K |
  +----------------------+
  |                 315K |
  +----------------------+ CRT_CONSOLE_PHY_BASE (Must be PAGE size aligned)
  |CRT_CONSOLE_SIZE      |    CRT_CONSOLE_SIZE = = VIDEO_DESTINATION_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000
  |               33.75M |
  +----------------------+ OSD_PHY_BASE
  |OSD_TOTAL_SIZE        |    OSD_TOTAL_SIZE = OSD_DISPLAY_MAX_X*OSD_DISPLAY_MAX_Y/2 = 0x00070800
  |                 450K |
  +----------------------+ CRT_SNAPSHOT_PHY_BASE
  |CRT_SNAPSHOT_SIZE     |    CRT_SNAPSHOT_SIZE = VIDEO_DESTINATION_SIZE = __TOTAL_BLOCKS*64*4 = 0x021C0000
  |               33.75M |
  +----------------------+
*/

/* Capture buf size is 32bits(x444 mode) per pixel. And must block align.*/
#define VIDEO_SOURCE_SIZE           (__TOTAL_BLOCKS*64*4)
#define VIDEO_DESTINATION_SIZE      VIDEO_SOURCE_SIZE
/* FIXME
 * stream mode is used only in analog auto phase adjustment
 * if we set VR0A8[24], the compressed data will not be written to stream buffer
 * should we change VIDEO_MAX_STREAM_SIZE definition to '0'?
 */
#define VIDEO_MAX_STREAM_SIZE       0x800000    /* 64X128K = 0x800000 (8MB) */ //Reserved for non-desc mode
/* AST1500. Flag size is 4bits per block. AST1510 is 1Byte per block. AST1520 requires 4bytes per block for high precision ABCD encode, but 1byte for decode. */
#ifdef CONFIG_ARCH_AST1500_HOST
#define VIDEO_FLAG_SIZE             (__TOTAL_BLOCKS*4)
#else
#define VIDEO_FLAG_SIZE             (__TOTAL_BLOCKS*1)
#endif
/* CRC size is 8 Bytes per 8 blocks*/
#define VIDEO_CRC_SIZE             0 /* ((__A8(__TOTAL_BLOCKS)/8)*8), no more used */
#define VBUF1_EN_SRCBKUP_SIZE      (VIDEO_SOURCE_SIZE)
#define VBUF1_EN_SNAPSHOT_SIZE     (VIDEO_SOURCE_SIZE)

#define VBUF1_EN_TOTAL_SIZE        (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_CRC_SIZE+VIDEO_FLAG_SIZE+VBUF1_EN_SRCBKUP_SIZE+VBUF1_EN_SNAPSHOT_SIZE)
#define VBUF2_EN_TOTAL_SIZE        0 /* VBUF1_EN_TOTAL_SIZE */
#define VBUFM_EN_TOTAL_SIZE        0 /* (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_FLAG_SIZE) */ /* 0x1605000 = about 22.7M */

#define OSD_TOTAL_SIZE             ((OSD_DISPLAY_MAX_X*OSD_DISPLAY_MAX_Y)/2)
#define CRT_CONSOLE_SIZE           (VIDEO_DESTINATION_SIZE)
#define CRT_SNAPSHOT_SIZE          (VIDEO_DESTINATION_SIZE)
#define CRTBUF_TOTAL_SIZE          (CRT_CONSOLE_SIZE + OSD_TOTAL_SIZE + CRT_SNAPSHOT_SIZE)

#define VBUF1_DE_TOTAL_SIZE        PAGE_ALIGN(VIDEO_MAX_STREAM_SIZE + VIDEO_DESTINATION_SIZE*2 + VIDEO_FLAG_SIZE)
#define VBUF2_DE_TOTAL_SIZE        0 /* (VBUF1_DE_TOTAL_SIZE) */

#define VBUF_HOST_SIZE		0x09000000 /* 144M */
#define VBUF_STATION_SIZE	0x09000000 /* 144M */
///////////////////////////////////////////////////////////////////////////////
#else /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#define DISPLAY_MAX_X		1920 /* in pixel */
#define DISPLAY_MAX_Y		1200 /* in pixel */
#define OSD_DISPLAY_MAX_X	640 /* in pixel */
#define OSD_DISPLAY_MAX_Y	480 /* in pixel */

/*
  Host:
  +----------------------+ PHYS_OFFSET
  |Host_TOTAL_SIZE       |
  |                  96M |
  +----------------------+ VBUF_EN_PHY_BASE, VBUF1_EN_PHY_BASE, VBUF1_EN_SOURCE0_PHY_BASE
  |VIDEO_SOURCE_SIZE     |    VIDEO_SOURCE_SIZE = 1920*1200*4 = 0x008CA000,
  |                9000K |
  +----------------------+ VBUF1_EN_SOURCE1_PHY_BASE
  |VIDEO_SOURCE_SIZE     |    VIDEO_SOURCE_SIZE = 1920*1200*4 = 0x008CA000,
  |                9000K |
  +----------------------+ VBUF1_EN_STREAM_PHY_BASE
  |VIDEO_MAX_STREAM_SIZE |
  |                   4M |
  +----------------------+ VBUF1_EN_CRC_PHY_BASE
  |VIDEO_CRC_SIZE        |    VIDEO_CRC_SIZE = 0
  |                    0 |
  +----------------------+ VBUF1_EN_FLAG_PHY_BASE
  |VIDEO_FLAG_SIZE       |    VIDEO_FLAG_SIZE = 0x5000
  |                  20K |
  +----------------------+ VBUF1_EN_SRCBKUP_PHY_BASE
  |VBUF1_EN_SRCBKUP_SIZE |    VBUF1_EN_SRCBKUP_SIZE = VIDEO_SOURCE_SIZE
  |                    0 |
  +----------------------+ VBUF1_EN_SNAPSHOT_PHY_BASE
  |VBUF1_EN_SNAPSHOT_SIZE|    VBUF1_EN_SNAPSHOT_SIZE = VIDEO_SOURCE_SIZE
  |               9000 K |
  +----------------------+ --------------------------------------------
  |  VBUF2_EN ...        |            no longer used
  +----------------------+
  |  VBUFM_EN ...        |
  +----------------------+

  Client:
  +----------------------+ PHYS_OFFSET
  |STATION_TOTAL_SIZE    |
  |                  96M |
  +----------------------+ VBUF_DE_PHY_BASE, VBUF1_DE_PHY_BASE, VBUF1_DE_STREAM_PHY_BASE
  |VIDEO_MAX_STREAM_SIZE |
  |                   4M |
  +----------------------+ VBUF1_DE_DEST0_PHY_BASE
  |VIDEO_DESTINATION_SIZE|    VIDEO_DESTINATION_SIZE = VIDEO_SOURCE_SIZE = 1920*1200*4 = 0x008CA000
  |               9000 K |
  +----------------------+ VBUF1_DE_DEST1_PHY_BASE
  |VIDEO_DESTINATION_SIZE|    VIDEO_DESTINATION_SIZE = VIDEO_SOURCE_SIZE = 1920*1200*4 = 0x008CA000
  |               9000 K |
  +----------------------+ VBUF1_DE_FLAG_PHY_BASE
  |VIDEO_FLAG_SIZE       |    VIDEO_FLAG_SIZE = 0x5000
  |                 20 K |
  +----------------------+ CRT_CONSOLE_PHY_BASE (Must be PAGE size aligned)
  |CRT_CONSOLE_SIZE      |    CRT_CONSOLE_SIZE = 0x12C000
  |               1200 K |
  +----------------------+ OSD_PHY_BASE
  |OSD_TOTAL_SIZE        |    OSD_TOTAL_SIZE = OSD_DISPLAY_MAX_X*OSD_DISPLAY_MAX_Y/2 = 640*480/2 = 0x00025800
  |                 150K |
  +----------------------+ CRT_SNAPSHOT_PHY_BASE
  |CRT_SNAPSHOT_SIZE     |    CRT_SNAPSHOT_SIZE = VIDEO_SOURCE_SIZE = 1920*1200*4 = 0x008CA000
  |               9000 K |
  +----------------------+ -----------------------------------------------
  |  VBUF2_DE ...        |           no longer used
  +----------------------+
*/
#define VIDEO_SOURCE_SIZE           0x8CA000  //0x900000    /* 1920X1200X4 = 0x8CA000 */
#define VIDEO_DESTINATION_SIZE      VIDEO_SOURCE_SIZE
#if (CONFIG_AST1500_SOC_VER == 2)
#ifdef HW_PROFILE
//Bruce131226. Reduce stream buf size to have more memory space for snapshot.
#define VIDEO_MAX_STREAM_SIZE       (0x800000 >> 1)    /* 64X128K = 0x800000 */
#else
#define VIDEO_MAX_STREAM_SIZE       0x400000    /* 32X128K = 0x400000 */
#endif
#else
#define VIDEO_MAX_STREAM_SIZE       0x400000    /* 32X128K = 0x400000 */
#endif
#define VIDEO_FLAG_SIZE             0x5000      /* 1920X1200/128 = 0x4650*/ //no more used
#define VIDEO_CRC_SIZE              0//0x50000     /* 1920/64X1200X8 = 0x46500*/
#define VBUF1_EN_SNAPSHOT_SIZE     (VIDEO_SOURCE_SIZE)
#define VBUF1_EN_SRCBKUP_SIZE      0

#define VBUF1_EN_TOTAL_SIZE        (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_CRC_SIZE+VIDEO_FLAG_SIZE+VBUF1_EN_SNAPSHOT_SIZE) /* 0x1655000 =  about 23M*/
#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_EN_TOTAL_SIZE        VBUF1_EN_TOTAL_SIZE
#define VBUFM_EN_TOTAL_SIZE        (VIDEO_SOURCE_SIZE*2+VIDEO_MAX_STREAM_SIZE+VIDEO_FLAG_SIZE) /* 0x1605000 = about 22.7M */
#endif

#define OSD_TOTAL_SIZE             ((OSD_DISPLAY_MAX_X*OSD_DISPLAY_MAX_Y)/2)
#define CRT_CONSOLE_SIZE           (0x12C000) //640x480x32bits /*VIDEO_DESTINATION_SIZE*/
#define CRT_SNAPSHOT_SIZE          (VIDEO_DESTINATION_SIZE)
#define CRTBUF_TOTAL_SIZE          (CRT_CONSOLE_SIZE + OSD_TOTAL_SIZE + CRT_SNAPSHOT_SIZE)

#if (CONFIG_AST1500_SOC_VER == 2)/* For SoC V2 only. Supports HW_PROFILE */
	#define VBUF1_DE_TOTAL_SIZE        PAGE_ALIGN(VIDEO_MAX_STREAM_SIZE + VIDEO_DESTINATION_SIZE)
#else
	#define VBUF1_DE_TOTAL_SIZE        PAGE_ALIGN(VIDEO_MAX_STREAM_SIZE + VIDEO_DESTINATION_SIZE*2) /* 0xD00000 = 13M*/
#endif

#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_DE_TOTAL_SIZE        (VBUF1_DE_TOTAL_SIZE)
#endif

#define VBUF_HOST_SIZE		0x02000000 /* 32M */
#define VBUF_STATION_SIZE	0x02000000 /* 32M */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

///////////////////////////////////////////////////////////////////////////////

#define HOST_TOTAL_SIZE		(num_physpages << PAGE_SHIFT)
#define STATION_TOTAL_SIZE	(num_physpages << PAGE_SHIFT)

#define VBUF_EN_PHY_BASE               (PHYS_OFFSET + HOST_TOTAL_SIZE)
#define VBUF1_EN_PHY_BASE              (VBUF_EN_PHY_BASE)

#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_EN_PHY_BASE              (VBUF1_EN_PHY_BASE + VBUF1_EN_TOTAL_SIZE)
#define VBUFM_EN_PHY_BASE              (VBUF2_EN_PHY_BASE + VBUF2_EN_TOTAL_SIZE)
#endif

#define VBUF1_EN_SOURCE0_PHY_BASE      (VBUF1_EN_PHY_BASE)
#define VBUF1_EN_SOURCE1_PHY_BASE      (VBUF1_EN_SOURCE0_PHY_BASE + VIDEO_SOURCE_SIZE)
#define VBUF1_EN_STREAM_PHY_BASE       (VBUF1_EN_SOURCE1_PHY_BASE + VIDEO_SOURCE_SIZE)
#define VBUF1_EN_CRC_PHY_BASE          (VBUF1_EN_STREAM_PHY_BASE + VIDEO_MAX_STREAM_SIZE)
#define VBUF1_EN_FLAG_PHY_BASE         (VBUF1_EN_CRC_PHY_BASE + VIDEO_CRC_SIZE)
#define VBUF1_EN_SRCBKUP_PHY_BASE     (VBUF1_EN_FLAG_PHY_BASE + VIDEO_FLAG_SIZE)
#define VBUF1_EN_SNAPSHOT_PHY_BASE    (VBUF1_EN_SRCBKUP_PHY_BASE + VBUF1_EN_SRCBKUP_SIZE)

#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_EN_SOURCE0_PHY_BASE      (VBUF2_EN_PHY_BASE)
#define VBUF2_EN_SOURCE1_PHY_BASE      (VBUF2_EN_SOURCE0_PHY_BASE + VIDEO_SOURCE_SIZE)
#define VBUF2_EN_STREAM_PHY_BASE       (VBUF2_EN_SOURCE1_PHY_BASE + VIDEO_SOURCE_SIZE)
#define VBUF2_EN_CRC_PHY_BASE          (VBUF2_EN_STREAM_PHY_BASE + VIDEO_MAX_STREAM_SIZE)
#define VBUF2_EN_FLAG_PHY_BASE         (VBUF2_EN_CRC_PHY_BASE + VIDEO_CRC_SIZE)
#endif

#define VBUF1_EN_OFFSET             (VBUF1_EN_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_SOURCE0_OFFSET     (VBUF1_EN_SOURCE0_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_SOURCE1_OFFSET     (VBUF1_EN_SOURCE1_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_STREAM_OFFSET      (VBUF1_EN_STREAM_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_CRC_OFFSET         (VBUF1_EN_CRC_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_FLAG_OFFSET        (VBUF1_EN_FLAG_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF1_EN_SNAPSHOT_OFFSET    (VBUF1_EN_SNAPSHOT_PHY_BASE - VBUF_EN_PHY_BASE)

#if (CONFIG_AST1500_SOC_VER >= 3) //For double buffer BCD
#define VBUF1_EN_SRCBKUP_OFFSET     (VBUF1_EN_SRCBKUP_PHY_BASE - VBUF_EN_PHY_BASE)
#endif

#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_EN_OFFSET            (VBUF2_EN_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF2_EN_SOURCE0_OFFSET    (VBUF2_EN_SOURCE0_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF2_EN_SOURCE1_OFFSET    (VBUF2_EN_SOURCE1_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF2_EN_STREAM_OFFSET     (VBUF2_EN_STREAM_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF2_EN_CRC_OFFSET        (VBUF2_EN_CRC_PHY_BASE - VBUF_EN_PHY_BASE)
#define VBUF2_EN_FLAG_OFFSET       (VBUF2_EN_FLAG_PHY_BASE - VBUF_EN_PHY_BASE)
#endif


#define VBUF_DE_PHY_BASE               (PHYS_OFFSET + STATION_TOTAL_SIZE)
#define VBUF1_DE_PHY_BASE              (VBUF_DE_PHY_BASE)
#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_DE_PHY_BASE              (VBUF1_DE_PHY_BASE + VBUF1_DE_TOTAL_SIZE + CRTBUF_TOTAL_SIZE)
#endif
#define VBUF1_DE_STREAM_PHY_BASE       (VBUF1_DE_PHY_BASE)
#define VBUF1_DE_DEST0_PHY_BASE        (VBUF1_DE_STREAM_PHY_BASE + VIDEO_MAX_STREAM_SIZE)
#define VBUF1_DE_DEST1_PHY_BASE        (VBUF1_DE_DEST0_PHY_BASE + VIDEO_DESTINATION_SIZE)
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define VBUF1_DE_FLAG_PHY_BASE         (VBUF1_DE_DEST1_PHY_BASE + VIDEO_DESTINATION_SIZE)
#endif
#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_DE_STREAM_PHY_BASE       (VBUF2_DE_PHY_BASE)
#define VBUF2_DE_DEST0_PHY_BASE             (VBUF2_DE_STREAM_PHY_BASE + VIDEO_MAX_STREAM_SIZE)
#define VBUF2_DE_DEST1_PHY_BASE             (VBUF2_DE_DEST0_PHY_BASE + VIDEO_DESTINATION_SIZE)
#endif

/*
 * the memory for framebuffer must be PAGE aligned
 * CRT_CONSOLE_PHY_BASE is PAGE-aligned already because we let VBUF1_DE_TOTAL_SIZE be PAGE-aligned
 */
#define CRT_CONSOLE_PHY_BASE       (VBUF1_DE_PHY_BASE + VBUF1_DE_TOTAL_SIZE)
#define OSD_PHY_BASE               (CRT_CONSOLE_PHY_BASE + CRT_CONSOLE_SIZE)
#define CRT_SNAPSHOT_PHY_BASE      (OSD_PHY_BASE+OSD_TOTAL_SIZE)

#define VBUF1_DE_STREAM_OFFSET     (VBUF1_DE_STREAM_PHY_BASE - VBUF_DE_PHY_BASE)
#define VBUF1_DE_DEST0_OFFSET      (VBUF1_DE_DEST0_PHY_BASE - VBUF_DE_PHY_BASE)
#define VBUF1_DE_DEST1_OFFSET      (VBUF1_DE_DEST1_PHY_BASE - VBUF_DE_PHY_BASE)
#if (CONFIG_AST1500_SOC_VER >= 3)
#define VBUF1_DE_FLAG_PHY_OFFSET   (VBUF1_DE_FLAG_PHY_BASE - VBUF_DE_PHY_BASE)
#endif
#define CRT_CONSOLE_OFFSET         (CRT_CONSOLE_PHY_BASE - VBUF_DE_PHY_BASE)
#define OSD_OFFSET                 (OSD_PHY_BASE - VBUF_DE_PHY_BASE)
#define CRT_SNAPSHOT_OFFSET            (CRT_SNAPSHOT_PHY_BASE - VBUF_DE_PHY_BASE)

#if (CONFIG_AST1500_SOC_VER == 1)
#define VBUF2_DE_STREAM_OFFSET     (VBUF2_DE_STREAM_PHY_BASE - VBUF_DE_PHY_BASE)
#define VBUF2_DE_DEST0_OFFSET      (VBUF2_DE_DEST0_PHY_BASE - VBUF_DE_PHY_BASE)
#define VBUF2_DE_DEST1_OFFSET      (VBUF2_DE_DEST1_PHY_BASE - VBUF_DE_PHY_BASE)
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
#if (VBUF1_EN_TOTAL_SIZE > VBUF_HOST_SIZE)
	#error "Wrong Host Vbuf size!"
#endif
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
#if ((VBUF1_DE_TOTAL_SIZE + CRTBUF_TOTAL_SIZE) > VBUF_STATION_SIZE)
	#error "Wrong Client Vbuf size!"
#endif
#endif

#define TO_VB1_EN_BASE(ba) ((ba)+VBUF1_EN_OFFSET)
#define TO_VB1_EN_SOURCE0_BASE(ba) ((ba)+VBUF1_EN_SOURCE0_OFFSET)
#define TO_VB1_EN_SOURCE1_BASE(ba) ((ba)+VBUF1_EN_SOURCE1_OFFSET)
#define TO_VB1_EN_STREAM_BASE(ba) ((ba)+VBUF1_EN_STREAM_OFFSET)
#define TO_VB1_EN_CRC_BASE(ba) ((ba)+VBUF1_EN_CRC_OFFSET)
#define TO_VB1_EN_FLAG_BASE(ba) ((ba)+VBUF1_EN_FLAG_OFFSET)
/* Bruce150515.SNAPSHOT_BASE is mapped as cache-able memory space base now.
#define TO_VB1_EN_SNAPSHOT_BASE(ba) ((ba)+VBUF1_EN_SNAPSHOT_OFFSET)
*/
#if (CONFIG_AST1500_SOC_VER >= 3) //For double buffer BCD
#define TO_VB1_EN_SRCBKUP_BASE(ba) ((ba)+VBUF1_EN_SRCBKUP_OFFSET)
#endif
#if (CONFIG_AST1500_SOC_VER == 1)
#define TO_VB2_EN_BASE(ba) ((ba)+VBUF2_EN_OFFSET)
#define TO_VB2_EN_SOURCE0_BASE(ba) ((ba)+VBUF2_EN_SOURCE0_OFFSET)
#define TO_VB2_EN_SOURCE1_BASE(ba) ((ba)+VBUF2_EN_SOURCE1_OFFSET)
#define TO_VB2_EN_STREAM_BASE(ba) ((ba)+VBUF2_EN_STREAM_OFFSET)
#define TO_VB2_EN_CRC_BASE(ba) ((ba)+VBUF2_EN_CRC_OFFSET)
#define TO_VB2_EN_FLAG_BASE(ba) ((ba)+VBUF2_EN_FLAG_OFFSET)
#endif

#define TO_VB1_DE_STREAM_BASE(ba)    ((ba)+VBUF1_DE_STREAM_OFFSET)
#define TO_VB1_DE_DEST0_BASE(ba)    ((ba)+VBUF1_DE_DEST0_OFFSET)
#define TO_VB1_DE_DEST1_BASE(ba)    ((ba)+VBUF1_DE_DEST1_OFFSET)
#if (CONFIG_AST1500_SOC_VER >= 3)
#define TO_VB1_DE_FLAG_BASE(ba)    ((ba)+VBUF1_DE_FLAG_PHY_OFFSET)
#endif
#if (CONFIG_AST1500_SOC_VER == 1)
#define TO_VB2_DE_STREAM_BASE(ba)    ((ba)+VBUF2_DE_STREAM_OFFSET)
#define TO_VB2_DE_DEST0_BASE(ba)    ((ba)+VBUF2_DE_DEST0_OFFSET)
#define TO_VB2_DE_DEST1_BASE(ba)    ((ba)+VBUF2_DE_DEST1_OFFSET)
#endif

#define TO_CRT_CONSOLE_BASE(ba)    ((ba)+CRT_CONSOLE_OFFSET)
#define TO_CRT_OSD_BASE(ba)    ((ba)+OSD_OFFSET)
/* Bruce150515.SNAPSHOT_BASE is mapped as cache-able memory space base now.
#define TO_CRT_SNAPSHOT_BASE(ba)    ((ba)+CRT_SNAPSHOT_OFFSET)
*/

#endif /* #ifndef _VBUG_H_ */
