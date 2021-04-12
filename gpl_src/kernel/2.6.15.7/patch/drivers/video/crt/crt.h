#ifndef _CRT_H_
#define _CRT_H_

#include <aspeed/features.h>

#include <linux/kernel.h> //might_sleep
#include <asm/arch/gpio.h>
#include <linux/workqueue.h>
#include <asm/semaphore.h>

#include <asm/arch/drivers/vesa.h>
#include <asm/arch/drivers/edid.h>
#include <asm/arch/drivers/video_hal.h>

#ifndef API_VER
#define API_VER 2
#endif

#define I2C_MONITOR_DDC_SPEED	40000

#define OSD_DISPLAY_MASK        (1<<0)
#define OSD_DISPLAY_DISABLE     (0<<0)
#define OSD_DISPLAY_ENABLE      (1<<0)
#define OSD_SET_ALPHA_MASK      0xFF0
#define OSD_SET_ALPHA_BIT       4
#define OSD_BITMAP_MASK         0xFFFF0000
#define OSD_BITMAP_BIT          16
#define OSD_FROM_GUI_NUM            0xFFFF
#if (CONFIG_AST1500_SOC_VER >= 3)
#define OSD_FROM_GUI 3
#else
#define OSD_FROM_GUI 1
#endif
#define OSD_FIX_PALETTE_COLOR 0
#define OSD_2_COLOR 1

#define OWNER_NONE      0  // no body uses crt
#define OWNER_CON		(1 << 0) //CRT_CONSOLE_PHY_BASE
#define OWNER_VE1		(1 << 1) //VBUF1_DE_DEST0_PHY_BASE or VBUF1_DE_DEST1_PHY_BASE
#define OWNER_HOST		(1 << 2) //Used for host loopback
#define OWNER_CURRENT	(1 << 3) //Don't Change. Special flag.


#ifdef Watcom
#define		CRT_REMAP_OFFSET	0x10000
#else
#define		CRT_REMAP_OFFSET	0x0
#endif


/* Used to indicate scaling up factor */
#define CRT_SCALE_1  1  //Don't scale
#define CRT_SCALE_2  2  //x 2
#define CRT_SCALE_3  3  //x 3/2
#define CRT_SCALE_4  4  //x 4/3

#if (CONFIG_AST1500_SOC_VER >= 3)
//AST1520 changes the range of scale factor.
#define CRT_SCALE_FACTOR_MAX            0x100000
#define CRT_SCALE_FACTOR_MAX_SHIFT_BITS (20)
#elif (CONFIG_AST1500_SOC_VER == 2)
//AST1510 changes the range of scale factor.
#define CRT_SCALE_FACTOR_MAX            0x40000
#define CRT_SCALE_FACTOR_MAX_SHIFT_BITS (18)
#else
#define CRT_SCALE_FACTOR_MAX            0x10000
#define CRT_SCALE_FACTOR_MAX_SHIFT_BITS (16)
#endif

#if 0
#define OUTPUT_SELECT_DIGITAL	0
#define OUTPUT_SELECT_ANALOG	1
#define OUTPUT_SELECT_NONE      (-1)
#endif


/********************************************************/
/*    CRT register                                      */
/********************************************************/
#define CRT_BASE_OFFSET						(0x6000+CRT_REMAP_OFFSET)

#define CRT1_CONTROL_REG					(0x00 + CRT_BASE_OFFSET)
#define CRT2_CONTROL_REG					(0x60 + CRT_BASE_OFFSET)
	#define GRAPH_DISPLAY_BIT				0
		#define GRAPH_DISPLAY_MASK				(1<<0)
		#define GRAPH_DISPLAY_ON			1
		#define GRAPH_DISPLAY_OFF			0
	#define HW_CURSOR_ENABLE_BIT            1
		#define HW_CURSOR_ENABLE_MASK       BIT1
		#define HW_CURSOR_ON                (1<<HW_CURSOR_ENABLE_BIT)
		#define HW_CURSOR_OFF               (0<<HW_CURSOR_ENABLE_BIT)
	#define ENABLE_OSD_MASK                 (1<<2)
		#define OSD_EN                          (1<<2)
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define ENABLE_CRT_INTERLACE_TIMING_MASK	0x00000008
#endif
	#define EN_X_SCALE_UP	BIT4
#if (CONFIG_AST1500_SOC_VER != 2)
	#define EN_Y_SCALE_UP	BIT5
#endif
	#define ENABLE_VE_FLIP	BIT6
	#define ENABLE_VE_FLIP_BIT	6
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define FORMAT_SELECT_BIT               7
		#define FORMAT_SELECT_MASK          (3 << FORMAT_SELECT_BIT)
		#define FORMAT_RGB565               (0 << FORMAT_SELECT_BIT) /* 32bit linear RGB565 if CRT60[9] is 0 */
		#define FORMAT_XRGB8888             (2 << FORMAT_SELECT_BIT) /* 32bit linear RGB8888 if CRT60[9] is 0 */
		#define FORMAT_RGB888_24BIT         (3 << FORMAT_SELECT_BIT) /* 24bit linear RGB888 if CRT60[9] is 0 */
		#define FORMAT_YUV444               (1 << FORMAT_SELECT_BIT) /* tile YUV444 if CRT60[9] is 1 */
		#define FORMAT_YUV420               (3 << FORMAT_SELECT_BIT) /* tile YUV422 if CRT60[9] is 1 */
	#define TILE_SELECT_BIT                 9
	#define TILE_SELECT_MASK                (1 << TILE_SELECT_BIT)
#else
	#define FORMAT_SELECT_BIT               8
		#define FORMAT_SELECT_MASK          (3<<8)
		#define FORMAT_RGB565               (0<<8)
		#define FORMAT_XRGB8888             (1<<8)
		#define FORMAT_YUV444               (2<<8)
		#define FORMAT_YUV420               (3<<8)
#endif
	#define SELECT_YUV444              	2
	#define CURSOR_PATTERN_FORMAT_BIT            10
		#define CURSOR_PATTERN_FORMAT_MASK       BIT10
		#define CURSOR_XRGB4444                (0<<CURSOR_PATTERN_FORMAT_BIT)
		#define CURSOR_ARGB4444                (1<<CURSOR_PATTERN_FORMAT_BIT)
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define HALF_DOWNSCALE_MASK                (1<<11)
		#define EN_HALF_DOWNSCALE              (1<<11)
#endif
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define YUV_FORMAT_SELECT_MASK             (1<<12) /* CRT60[13] is obsolete in AST1520 */
		#define YUV_FORMAT0                    (0<<12)
		#define YUV_FORMAT1                    (1<<12) /* "YUV422 tile" in to "4K YUV420" out. */
#else
	#define YUV_FORMAT_SELECT_MASK             (3<<12)
		#define YUV_FORMAT0                    (0<<12)
		#define YUV_FORMAT1                    (1<<12)
		#define YUV_FORMAT2                    (2<<12)
		#define YUV_FORMAT3                    (3<<12)
#endif
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define ENABLE_FULL_RANGE_YUV              (1<<14)
#endif
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define ENABLE_YUV_TILE_MODE               (1<<15)
#endif
	#define HOR_SYNC_SELECT_BIT				16
		#define HOR_SYNC_SELECT_MASK			1<<16
		#define HOR_NEGATIVE				1
		#define HOR_POSITIVE				0
	#define VER_SYNC_SELECT_BIT				17
		#define VER_SYNC_SELECT_MASK			1<<17
		#define VER_NEGATIVE				1
		#define VER_POSITIVE				0
	#define HOR_SYNC_OFF_MASK                 (1<<18)
		#define HOR_SYNC_OFF_ENABLE               (1<<18)
		#define HOR_SYNC_OFF_DISABLE      (0<<18)
	#define VER_SYNC_OFF_MASK                 (1<<19)
		#define VER_SYNC_OFF_ENABLE           (1<<19)
		#define VER_SYNC_OFF_DISABLE          (0<<19)
	#define SCREEN_OFF_MASK                 (1<<20) // Disable DE, desktop, OSD, cursor. Keep H/V Sync.
		#define SCREEN_OFF_ON               (1<<20)
		#define SCREEN_OFF_OFF              (0<<20)
	#define DISABLE_DESKTOP_MASK 			0x00800000 //bit[23]. disable desktop. Keep H/V Sync, DE, OSD and cursor.
	#define INT_ON_BIT                      30
	#define INT_ON_MASK                     (0x1<<INT_ON_BIT)
	#define INT_STATUS_BIT                  31
	#define INT_STATUS_MASK                 (0x1<<INT_STATUS_BIT)

#define CRT1_CONTROL2_REG					(0x04 + CRT_BASE_OFFSET)
#define CRT2_CONTROL2_REG					(0x64 + CRT_BASE_OFFSET)
	#define PWR_ON_DAC                           0x1
	#define PWR_OFF_DAC                          0x0
	#define PWR_ON_DAC_MASK                0x1
	#define LINE_TO_INT_BIT                 20
	#define LINE_TO_INT_MASK                (0xFFF<<LINE_TO_INT_BIT)

#define CRT1_STATUS_REG						(0x08 + CRT_BASE_OFFSET)
#define CRT2_STATUS_REG						(0x68 + CRT_BASE_OFFSET)
	#define CRT_VDE_MASK					(1<<1)
	#define CRT_VDE						(1<<1)
	#define CRT_ODD_FIELD                   (1<<4)
	#define CRT_ODD_FIELD_SYNC              (1<<5)

#define CRT1_VIDEO_PLL_REG                  (0x0C + CRT_BASE_OFFSET)
#define CRT2_VIDEO_PLL_REG                  (0x6C + CRT_BASE_OFFSET)
	#define POST_DIV_BIT                    18
	#define POST_DIV_MASK                   3<<18
	#define DIV_1_1                     0
	//#define DIV_1_2                     1
	#define DIV_1_2                     2
	#define DIV_1_4                     3
#if (CONFIG_AST1500_SOC_VER >= 3) //A1 feature
	#define CSC_EN_BT709                    (0x1UL << 26) /* Choose format for YUV2RGB. 1:BT709 0:BT601 */
	#define YUV420_SWAP_UV                  (0x1UL << 25)
#endif

#define CRT1_HOR_TOTAL_END_REG              (0x10 + CRT_BASE_OFFSET)
#define CRT2_HOR_TOTAL_END_REG              (0x70 + CRT_BASE_OFFSET)
	#define HOR_TOTAL_BIT                   0
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define HOR_TOTAL_MASK                  0x1FFFUL
#else
	#define HOR_TOTAL_MASK                  0xFFFUL
#endif
	#define HOR_ENABLE_END_BIT              16
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define HOR_ENABLE_END_MASK             (0x1FFFUL << HOR_ENABLE_END_BIT)
#else
	#define HOR_ENABLE_END_MASK             (0xFFFUL << HOR_ENABLE_END_BIT)
#endif

#define CRT1_HOR_RETRACE_REG                (0x14 + CRT_BASE_OFFSET)
#define CRT2_HOR_RETRACE_REG                (0x74 + CRT_BASE_OFFSET)
	#define HOR_RETRACE_START_BIT           0
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define HOR_RETRACE_START_MASK          0x1FFFUL
#else
	#define HOR_RETRACE_START_MASK          0xFFFUL
#endif
	#define HOR_RETRACE_END_BIT             16
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define HOR_RETRACE_END_MASK            (0x1FFFUL << HOR_RETRACE_END_BIT)
#else
	#define HOR_RETRACE_END_MASK            (0xFFFUL << HOR_RETRACE_END_BIT)
#endif

#define CRT1_VER_TOTAL_END_REG              (0x18 + CRT_BASE_OFFSET)
#define CRT2_VER_TOTAL_END_REG              (0x78 + CRT_BASE_OFFSET)
	#define VER_TOTAL_BIT                   0
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define VER_TOTAL_MASK                  0xFFFUL
#else
	#define VER_TOTAL_MASK                  0x7FFUL
#endif
	#define VER_ENABLE_END_BIT              16
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define VER_ENABLE_END_MASK             (0xFFFUL << VER_ENABLE_END_BIT)
#else
	#define VER_ENABLE_END_MASK             (0x7FFUL << VER_ENABLE_END_BIT)
#endif

#define CRT1_VER_RETRACE_REG                (0x1C + CRT_BASE_OFFSET)
#define CRT2_VER_RETRACE_REG                (0x7C + CRT_BASE_OFFSET)
	#define VER_RETRACE_START_BIT           0
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define VER_RETRACE_START_MASK          0xFFFUL
#else
	#define VER_RETRACE_START_MASK          0x7FFUL
#endif
	#define VER_RETRACE_END_BIT             16
#if (CONFIG_AST1500_SOC_VER >= 2)
	#define VER_RETRACE_END_MASK            (0xFFFUL << VER_RETRACE_END_BIT)
#else
	#define VER_RETRACE_END_MASK            (0x7FFUL << VER_RETRACE_END_BIT)
#endif

#define CRT1_DISPLAY_ADDRESS				(0x20 + CRT_BASE_OFFSET)
#define CRT2_DISPLAY_ADDRESS				(0x80 + CRT_BASE_OFFSET)
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define DISPLAY_ADDRESS_MASK			0x3FFFFFFF
#elif (CONFIG_AST1500_SOC_VER >= 2)
	#define DISPLAY_ADDRESS_MASK			0x1FFFFFFF
#else
	#define DISPLAY_ADDRESS_MASK			0x0FFFFFFF
#endif

#define CRT1_DISPLAY_OFFSET                 (0x24 + CRT_BASE_OFFSET)
#define CRT2_DISPLAY_OFFSET                 (0x84 + CRT_BASE_OFFSET)
	#define TERMINAL_COUNT_BIT              16
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define BLKINIV_MASK	0x7
	#define Y_PAN(v)                        ((v) & BLKINIV_MASK)
	#define DISPLAY_OFFSET_ALIGN            0xF /* 16 byte alignment*/
	#define DISPLAY_OFFSET_MASK             0xFFF0
	#define TERMINAL_COUNT_MASK             (0x1FFF << TERMINAL_COUNT_BIT)
#elif (CONFIG_AST1500_SOC_VER >= 2)
	#define DISPLAY_OFFSET_ALIGN            0x7 /* 8 byte alignment*/
	#define DISPLAY_OFFSET_MASK             0x7FF8
	#define TERMINAL_COUNT_MASK             (0xFFF << TERMINAL_COUNT_BIT)
#else
	#define DISPLAY_OFFSET_ALIGN            0x7 /* 8 byte alignment*/
	#define DISPLAY_OFFSET_MASK             0x7FF8
	#define TERMINAL_COUNT_MASK             (0x7FF << TERMINAL_COUNT_BIT)
#endif

#define CRT1_THRESHOLD_REG                  (0x28 + CRT_BASE_OFFSET)
#define CRT2_THRESHOLD_REG                  (0x88 + CRT_BASE_OFFSET)
	#define THRES_LOW_BIT                   0
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define THRES_LOW_MASK                   (0x3F)
#elif (CONFIG_AST1500_SOC_VER >= 2)
	#define THRES_LOW_MASK                   (0x7F)
#else
	#define THRES_LOW_MASK                   (0x3F)
#endif
	#define THRES_HIGHT_BIT                 8
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define THRES_HIGHT_MASK                 (0x3F << THRES_HIGHT_BIT)
#elif (CONFIG_AST1500_SOC_VER >= 2)
	#define THRES_HIGHT_MASK                 (0x7F << THRES_HIGHT_BIT)
#else
	#define THRES_HIGHT_MASK                 (0x3F << THRES_HIGHT_BIT)
#endif
#if (CONFIG_AST1500_SOC_VER >= 3)
			#define    CRT_LOW_THRESHOLD_VALUE             0x30
			#define    CRT_HIGH_THRESHOLD_VALUE            0x38
#elif (CONFIG_AST1500_SOC_VER >= 2)
	#ifdef FPGA_TEST_ONLY
			#define    CRT_LOW_THRESHOLD_VALUE             0x29
			#define    CRT_HIGH_THRESHOLD_VALUE            0x3C
	#else //#ifdef FPGA_TEST_ONLY
		#if 0//DDR2
			#define    CRT_LOW_THRESHOLD_VALUE             0x3F
			#define    CRT_HIGH_THRESHOLD_VALUE            0x6F
		#else//DDR3
			#define    CRT_LOW_THRESHOLD_VALUE             0x60
			#define    CRT_HIGH_THRESHOLD_VALUE            0x78
		#endif
	#endif //#ifdef FPGA_TEST_ONLY
#else
//for fix 1920X1080
//#define    CRT_LOW_THRESHOLD_VALUE             0x12
#define    CRT_LOW_THRESHOLD_VALUE             0x16
#define    CRT_HIGH_THRESHOLD_VALUE            0x1E
#endif
	#define X_SCALE_UP_FACTOR_H_BIT       (16)
	#define Y_SCALE_UP_FACTOR_H_BIT       (20)
#if (CONFIG_AST1500_SOC_VER >= 3)
	#define X_SCALE_UP_FACTOR_H_MASK      (0xFUL << X_SCALE_UP_FACTOR_H_BIT)
	#define X_SCALE_UP_FACTOR_H(v)        ((((v) >> 16) & 0xF) << X_SCALE_UP_FACTOR_H_BIT)
	#define Y_SCALE_UP_FACTOR_H_MASK      (0xFUL << Y_SCALE_UP_FACTOR_H_BIT)
	#define Y_SCALE_UP_FACTOR_H(v)        ((((v) >> 16) & 0xF) << Y_SCALE_UP_FACTOR_H_BIT)
#elif (CONFIG_AST1500_SOC_VER == 2)
	#define X_SCALE_UP_FACTOR_H_MASK      (0x3UL << X_SCALE_UP_FACTOR_H_BIT)
	#define X_SCALE_UP_FACTOR_H(v)        ((((v) >> 16) & 0x3) << X_SCALE_UP_FACTOR_H_BIT)
	#define Y_SCALE_UP_FACTOR_H_MASK      (0x1UL << Y_SCALE_UP_FACTOR_H_BIT)
	#define Y_SCALE_UP_FACTOR_H(v)        (((v)?1:0) << Y_SCALE_UP_FACTOR_H_BIT) //1510 support only 2X vertical scaling.
#elif (CONFIG_AST1500_SOC_VER == 1)
	#define X_SCALE_UP_FACTOR_H_MASK      (0)
	#define X_SCALE_UP_FACTOR_H(v)        (0)
	#define Y_SCALE_UP_FACTOR_H_MASK      (0)
	#define Y_SCALE_UP_FACTOR_H(v)        (0)
#endif
	#define X_SCALE_UP_FACTOR_H_GET(r)    ((((r) & X_SCALE_UP_FACTOR_H_MASK) >> X_SCALE_UP_FACTOR_H_BIT) << 16)
	#define Y_SCALE_UP_FACTOR_H_GET(r)    ((((r) & Y_SCALE_UP_FACTOR_H_MASK) >> Y_SCALE_UP_FACTOR_H_BIT) << 16)

#define CRT2_SCALE_UP_FACTOR	(0x8C + CRT_BASE_OFFSET)
	#define X_SCALE_UP_FACTOR_L_BIT       (0)
	#define Y_SCALE_UP_FACTOR_L_BIT       (16)
	#define X_SCALE_UP_FACTOR_L_MASK      (0xFFFFUL << X_SCALE_UP_FACTOR_L_BIT)
	#define X_SCALE_UP_FACTOR_L(v)        (((v) & 0xFFFFUL) << X_SCALE_UP_FACTOR_L_BIT)
#if (CONFIG_AST1500_SOC_VER >= 3) || (CONFIG_AST1500_SOC_VER == 1)
	#define Y_SCALE_UP_FACTOR_L_MASK      (0xFFFFUL << Y_SCALE_UP_FACTOR_L_BIT)
	#define Y_SCALE_UP_FACTOR_L(v)        (((v) & 0xFFFFUL) << Y_SCALE_UP_FACTOR_L_BIT)
#elif (CONFIG_AST1500_SOC_VER == 2)
	#define Y_SCALE_UP_FACTOR_L_MASK      (0)
	#define Y_SCALE_UP_FACTOR_L(v)        (0)
#endif
	#define X_SCALE_UP_FACTOR_L_GET(r)    (((r) & X_SCALE_UP_FACTOR_L_MASK) >> X_SCALE_UP_FACTOR_L_BIT)
	#define Y_SCALE_UP_FACTOR_L_GET(r)    (((r) & Y_SCALE_UP_FACTOR_L_MASK) >> Y_SCALE_UP_FACTOR_L_BIT)


#define CRT1_CURSOR_OFFSET                  (0x30 + CRT_BASE_OFFSET)
#define CRT2_CURSOR_OFFSET                  (0x90 + CRT_BASE_OFFSET)
#define CRT2_PANNING                        (0x9C + CRT_BASE_OFFSET)
	#define X_PAN_BIT                       (0)
	#define X_PAN_MASK                      (0xFFUL)
	#define X_PAN(v)                        (((v) << X_PAN_BIT) & X_PAN_MASK)
	#define Y_SYNC_PAN_BIT                  (16)
	#define Y_SYNC_PAN_MASK                 (0xFFUL << Y_SYNC_PAN_BIT)
	#define Y_SYNC_PAN(v)                   (((v) << Y_SYNC_PAN_BIT) & Y_SYNC_PAN_MASK)
#define CRT1_CURSOR_POSITION                (0x34 + CRT_BASE_OFFSET)
#define CRT2_CURSOR_POSITION                (0x94 + CRT_BASE_OFFSET)
#define CRT1_CURSOR_PATTERN                 (0x38 + CRT_BASE_OFFSET)
#define CRT2_CURSOR_PATTERN                 (0x98 + CRT_BASE_OFFSET)



/********************************************************/
/*    OSD register                                      */
/********************************************************/

#define PALETTE_SIZE 16

#if (CONFIG_AST1500_SOC_VER >= 3)
#define     OSD_THRES_HIGH_VALUE            0x1E
#define     OSD_THRES_LOW_VALUE             0x1D //0x18 Bruce150827 Change from 0x18 to 0x1D for 4K 16bits OSD.
#define     OSD_ALPHA_VALUE                 31  /* 0~31 */
#else
#define     OSD_THRES_HIGH_VALUE            8
#define     OSD_THRES_LOW_VALUE             3
#define     OSD_ALPHA_VALUE                 15 /* 0~15 */
#endif

#define     CRT1_OSD_HORIZONTAL_REG         (0x40 + CRT_BASE_OFFSET)
#define     CRT2_OSD_HORIZONTAL_REG         (0xA0 + CRT_BASE_OFFSET)
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_HOR_START_MASK          0x1FFF /* [12:0] */
#else
    #define     OSD_HOR_START_MASK          0x7FF
#endif
    #define     OSD_HOR_END_BIT             16
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_HOR_END_MASK            0x1FFF0000 /* [28:16] */
#else
    #define     OSD_HOR_END_MASK            0x7FF0000
#endif
#define     CRT1_OSD_VERTICAL_REG           (0x44 + CRT_BASE_OFFSET)
#define     CRT2_OSD_VERTICAL_REG           (0xA4 + CRT_BASE_OFFSET)
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_VER_START_MASK          0xFFF /* [11:0] */
#else
    #define     OSD_VER_START_MASK          0x7FF
#endif
    #define     OSD_VER_END_BIT             16
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_VER_END_MASK            0xFFF0000 /* [27:16] */
#else
    #define     OSD_VER_END_MASK            0x7FF0000
#endif
#define     CRT1_OSD_ADDRESS_REG            (0x48 + CRT_BASE_OFFSET)
#define     CRT2_OSD_ADDRESS_REG            (0xA8 + CRT_BASE_OFFSET)
#define     CRT1_OSD_OFFSET_REG             (0x4C + CRT_BASE_OFFSET)
#define     CRT2_OSD_OFFSET_REG             (0xAC + CRT_BASE_OFFSET)
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_OFFSET_MASK             0x7FFF /* [14:4] is bit[14:4] */
#else
    #define     OSD_OFFSET_MASK             0x7FF
#endif
    #define     OSD_TERMINAL_BIT            16
    #define     OSD_TERMINAL_MASK           0xFFF0000
#define     CRT1_OSD_THRESHOLD_REG          (0x50 + CRT_BASE_OFFSET)
#define     CRT2_OSD_THRESHOLD_REG          (0xB0 + CRT_BASE_OFFSET)
    #define     OSD_THRES_LOW_BIT           0
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_THRES_LOW_MASK          0x1F /* [4:0] */
#else
    #define     OSD_THRES_LOW_MASK          0xF
#endif
    #define     OSD_THRES_HIGH_BIT          8
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_THRES_HIGH_MASK         0x1F00 /* [12:8] */
#else
    #define     OSD_THRES_HIGH_MASK         0xF00
#endif
    #define     OSD_ALPHA_BIT               16
#if (CONFIG_AST1500_SOC_VER >= 3)
    #define     OSD_ALPHA_BIT_MASK          0x1F0000
    #define     OSD_16BITS_MODE_EN          (0x1UL << 24)
    #define     OSD_16BITS_TRNSPRNT_EN      (0x1UL << 25)
#else
    #define     OSD_ALPHA_BIT_MASK          0xF0000
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
#define     CRT2_OSD_TRANSPRNT_REG          (0xB4 + CRT_BASE_OFFSET)
    #define     OSD_TRNSPRNT_DATA(v)        ((v) & 0xFFFFUL)
#endif

#define CRT2_X_SCALE_COEF_IDX0 (0xC0 + CRT_BASE_OFFSET)
#define CRT2_X_SCALE_COEF_IDX1 (0xC4 + CRT_BASE_OFFSET)
#if (CONFIG_AST1500_SOC_VER != 2)
#define CRT2_Y_SCALE_COEF_IDX0 (0xC8 + CRT_BASE_OFFSET)
#define CRT2_Y_SCALE_COEF_IDX1 (0xCC + CRT_BASE_OFFSET)
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
#define CRT2_VEFLIP_YUV444_BASE_OFFSET          (0xD0 + CRT_BASE_OFFSET)
#define CRT2_VEFLIP_YUV422_BASE_OFFSET          (0xD4 + CRT_BASE_OFFSET)
#define CRT2_VEFLIP_TERMINAL_COUNT              (0xD8 + CRT_BASE_OFFSET)
	#define VEFLIP_YUV444_TC(v)                 (((v) & 0x1FFFUL) << 0)
	#define VEFLIP_YUV422_TC(v)                 (((v) & 0x1FFFUL) << 16)
#endif

#define    OSD_PALETTE_INDEX01_REG           (0xE0 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEX23_REG           (0xE4 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEX45_REG           (0xE8 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEX67_REG           (0xEC + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEX89_REG           (0xF0 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEXAB_REG           (0xF4 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEXCD_REG           (0xF8 + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEXEF_REG           (0xFC + CRT_BASE_OFFSET)
#define    OSD_PALETTE_INDEX_BASE             OSD_PALETTE_INDEX01_REG

#if (CONFIG_AST1500_SOC_VER == 1)
#define		CRT_1								0
#endif
#define		CRT_2								1

#if (CONFIG_AST1500_SOC_VER >= 3)
#define VW_API_V2_SUPPORT
#endif

/* FORMAT TYPE */
typedef enum
{
    YUV444_FORMAT = 0,
    YUV420_FORMAT,
    RGB565_FORMAT,
    XRGB8888_FORMAT,
} eVIDEO_FORMAT;

typedef enum
{
	NO = 0,
	V_FLIP,
	H_FLIP,
	RT_180,
	B_SLASH,
	RT_90,
	RT_270,
	SLASH,
} eVIDEO_ROTATE;

typedef struct {
    unsigned int HStart;
    unsigned int HEnd;
    unsigned int VStart;
    unsigned int VEnd;
	unsigned int OSDBufLineOffset;
} OSDInfo, *POSDInfo;

typedef struct {
    u8 R;
    u8 G;
    u8 B;
} OSDPalette, *POSDPalette;

#define IO_IN_USE_NONE	0
#define IO_IN_USE_CODEC	1//When Codec is in use, only 1 upstream I2S channel and 1 downstream I2S channel are available.
#define IO_IN_USE_HDMI		2

#if 0 //Move to video_hal.h
typedef struct _Audio_Info {
    u8 Audio_On;//useless to audio Tx side
    u8 SampleFrequency;
    u8 ValidCh; //valid value are 2,4,6,8. non-HD NLPCM's value will be 2. Used to map to the number of I2S channel used.
    u8 SampleSize;
#if (API_VER >= 2)
	u8 io_in_use;
	u8 enable_upstream;
	u8 DownSampling;
	u8 Mode16bits;
	u8 Audio_Type; //0x80 On, 0x00 and LPCM, 0x40 HBR, 0x10 NLPCM, 0x20 DSD
	u8 bSPDIF;
	/* caner add for fixing AudioInfoFrame issue */
	u8 AUD_DB[5];
	u8 ucIEC60958ChStat[5];
#endif
} Audio_Info;

typedef struct _INFO_FRAME_
{
	u8	number;//0 means data are invalid. Will be reset to 0 when mode changes.
	u8	data[31];
}
InfoFrame, *LpInfoFrame;
#endif

/* Used for s_crt_info.flag */
#define CRT_INFO_PATCH_INTERLACE	(1 << 0)
//#define HDMI_FORCE_RGB_OUTPUT		(1 << 1)
//#define HDMI_4K60HZ_YUV420_OUTPUT	(1 << 2)
#define HDMI_FORCE_RGB_OUTPUT_LIMITED	(1 << 3)
#define HDMI_HDR_MODE_OFF       (1 << 4)
#define HDMI_INFOFRAME_ONLY     (1 << 5)
#define CRT_OUT_LIMITED         (1 << 6) /* Set if CRT output is limited RGB, otherwise is full RGB */

struct s_xmiter_mode {
	unsigned int owner;
	unsigned int flag;
	eVSignalType HDMI_Mode;
	u8 color_depth;
	InfoFrame	AVI_Info;
	InfoFrame	HDMI_Info;
	InfoFrame	HDR_Info;
};

/* -- Start of Force Timing CFG macros -- */
/* Force timing mode is enabled. */
#define FTCFG_G_ENABLE(r)           (((r) >> 31) & 0x1)
#define FTCFG_S_ENABLE(r, v)        ((r) | (((v) & 0x1) << 31))
#define FTCFG_IS_ENABLED(r)         ((r) & (0x1 << 31))
/* DVI/VGA is enabled. */
#define FTCFG_G_DVI(r)              (((r) >> 30) & 0x1)
#define FTCFG_S_DVI(r, v)           ((r) | (((v) & 0x1) << 30))
#define FTCFG_IS_DVI(r)             ((r) & (0x1 << 30))

/* Pass-through type select. */
#define FTCFG_PASS_THROUGH_OFFSET	(28)
#define FTCFG_PASS_THROUGH_MASK		(0x1)
#define FTCFG_G_PASS_THROUGH(r)		(((r) >> FTCFG_PASS_THROUGH_OFFSET) & FTCFG_PASS_THROUGH_MASK)
#define FTCFG_S_PASS_THROUGH(r, v)	((r) | (((v) & FTCFG_PASS_THROUGH_MASK) << FTCFG_PASS_THROUGH_OFFSET))
#define FTCFG_PASS_THROUGH_TYPE_DEFAULT	(0) /* fast-switch mode */
#define FTCFG_PASS_THROUGH_TYPE_STRICT	(1) /* strict mode */

/* Force timing mode config type select. */
#define FTCFG_G_TYPE(r)             (((r) >> 24) & 0x7)
#define FTCFG_S_TYPE(r, v)          ((r) | (((v) & 0x7) << 24))
#define FTCFG_TYPE_VIC              (0) /* Use VIC code. */
#define FTCFG_TYPE_TIMING_SN        (1) /* Use firmware vesa table SN. */
#define FTCFG_TYPE_EDID             (2) /* Auto choose from EDID. */
/* Convert 3D to 2D. */
#define FTCFG_G_OP_CNVRT_3D(r)      (((r) >> 23) & 0x1)
#define FTCFG_S_OP_CNVRT_3D(r, v)   ((r) | (((v) & 0x1) << 23))
#define FTCFG_IS_CNVRT_3D_EN(r)     ((r) & (0x1 << 23))
/* Mode change on refresh rate change. */
#define FTCFG_G_OP_CHG_RR(r)        (((r) >> 20) & 0x1)
#define FTCFG_S_OP_CHG_RR(r, v)     ((r) | (((v) & 0x1) << 20))
#define FTCFG_IS_CHG_RR_EN(r)       ((r) & (0x1 << 20))
/* Only force timing when source timing is bigger than... */
#define FTCFG_G_OP_LESS_THAN(r)     (((r) >> 16) & 0x1)
#define FTCFG_S_OP_LESS_THAN(r, v)  ((r) | (((v) & 0x1) << 16))
#define FTCFG_IS_LESS_THAN_EN(r)    ((r) & (0x1 << 16))
/* The timing code. VIC or firmware vesa table index. */
#define FTCFG_G_TIMING_CODE(r)      (((r) >>  0) & 0xFFFFUL)
#define FTCFG_S_TIMING_CODE(r, v)   ((r) | (((v) & 0xFFFFUL) << 0))
/* -- End of Force Timing CFG macros -- */

struct s_force_timing {
	unsigned int cfg;
	unsigned int timing_table_idx;
};

struct s_crt_info {
	//int display_select; //OUTPUT_SELECT_ANALOG, OUTPUT_SELECT_DIGITAL, OUTPUT_SELECT_NONE
	eVIDEO_FORMAT crt_color; /* Indicate the used color format in FB. May not always be accurate considerring HW quality profile mode. */
	eVIDEO_FORMAT crt_output_format; /* Can only be YUV444_FORMAT, YUV420_FORMAT and XRGB8888_FORMAT */
	unsigned char *edid;
	unsigned int owner; //Who request: OWNER_CON or OWNER_VE
	unsigned int clr; // 1: clear buf, 0: don't clear
	u16 width;
	u16 height;
	u16 refresh_rate;
	/*
	** applies to reg::0x24. Normally will be width, but will be different when applying 32 aligned patch.
	** 0 means use the normal case.
	*/
	unsigned int line_offset;
	unsigned int line_offset_rotate;
	unsigned int terminal_cnt; //Will be valid only after timing been set. Used to refect the change of scaling.
#if (CONFIG_AST1500_SOC_VER >= 3)
	unsigned int terminal_cnt_yuv444; //Will be valid only after timing been set. Used to refect the change of scaling.
	unsigned int terminal_cnt_yuv422; //Will be valid only after timing been set. Used to refect the change of scaling.
#endif
	eVSignalType HDMI_Mode;
	int TimingTableIdx; //It is only used when owner == OWNER_VE
	u8 color_depth;
	unsigned char EnHDCP; /* False: Disable HDCP, True: See video_hal.h HDCP_DISABLE defines */
	unsigned int flag;
	unsigned int video_ready;
	InfoFrame	AVI_Info;
	InfoFrame	HDMI_Info;
	InfoFrame	HDR_Info;
	struct s_force_timing force_timing;
	struct s_xmiter_mode xmiter_mode;
};

#define CDBG_INFO       1
#define CDBG_VERIFY_MT  (1<<1)
#define CDBG_DAC_TEST   (1<<2)
#define CDBG_DESK_OFF   (1<<3)
#define CDBG_CASTING_MODE_3   (1<<4)
#define CDBG_IGNORE_CTS7_33   (1<<5)

/* Define of Crt OP State */
#define COPS_PWR_SAVE        (1 << 0) //Turn off sync. But will auto turn on when to_xxx_screen been called.
#define COPS_DESKTOP_OFF     (1 << 1)
#define COPS_FORCE_SYNC_OFF  (1 << 2) //Turn off sync until explicitly turned on.
#define COPS_FORCING_TIMING  (1 << 3) //Set when force timing mode is running.
#define COPS_VE_YUV_DESKTOP_OFF (1 << 4) /* desktop off when (VE screen) && (output is YUV). It is a sub-state of COPS_DESKTOP_OFF. */

#if 0
#define DISPLAY_SELECT_CRT1	0
#define DISPLAY_SELECT_CRT2	1
#endif
struct s_crt_drv {
	u32 debug_flag;
	u32 is_client;
	MODE_ITEM *mode_table;
	u32 mode_table_size;
	u32 mode_table_vesa_start;
	u32 mode_table_user_start;

	int tx_select; //xmiter select
	int disp_select; //CRT_1 or CRT_2 select
	int display_attached;//0:dettached, 1:primary attached, 2:secondary attached
	unsigned char *vbuf_va;
	unsigned char *vbuf_va_c; /* cacheable */

	/* client board only */
	struct workqueue_struct *wq;
	struct work_struct OsdVideoOnWork;
#if (CONFIG_AST1500_SOC_VER >= 3)
	struct work_struct LogoWork;
#endif
	struct workqueue_struct *xmiter_wq;
	struct work_struct ReadDisplayEdidWork;
	u32 OSD_VideoOn_Setting;
#if (CONFIG_AST1500_SOC_VER >= 3)
	u32 Logo_Setting;
	u32 edid_valid;
#endif
	u8 edid[EDID_ROM_SIZE];

	struct ast_edid_ops *ast_edid;
	edid_status edid_parse_result;

	unsigned int current_owner;
	struct s_crt_info VE1_mode;
	struct s_crt_info CON_mode;
	struct s_crt_info HOST_mode;
	u32 dec_buf_base;
#if (CONFIG_AST1500_SOC_VER >= 3)
	u32 dec_buf_offset;
#endif
	u32 available_screen; //mode information is available
	u32 connected_screen;
	u32 dec_hor_scale;
	u32 dec_ver_scale;
	u32 dec_half_down_scale; //SoC V3 only

	struct s_csr_info *csr_info;
	Audio_Info audio_info;
	unsigned int audio_ready;
	u8 tx_exist; //used to determine whether digital loopback is available in host. Analog loopback is uncontrollable anyway.
	u32 always_set_tx;
	u32 OSD_force_2_color; //D[0]:force 2 color; D[1]:Transparent ON/OFF; D[16:31]:transparent 16bit RGB565 value
#if OSD_FROM_GUI
	u32 OSD_y_offset; //The start offset of vertical line
	u32 OSD_y_size; //How many vertical lines to display
#define OSD_POS_DEFAULT   0  //Use the setting of /sysfs/osd_from_gui. center or top-left
#define OSD_POS_TL        1  //top-left
#define OSD_POS_T         2  //top
#define OSD_POS_TR        3  //top-right
#define OSD_POS_L         4  //left
#define OSD_POS_C         5  //center
#define OSD_POS_R         6  //right
#define OSD_POS_BL        7  //bottom-left
#define OSD_POS_B         8  //bottom
#define OSD_POS_BR        9  //bottom-right
	int OSD_pos_align;
	int OSD_pos_x_offset; //alignment_start_x + OSD_pos_x_offset = OSD start X
	int OSD_pos_y_offset; //alignment_start_y + OSD_pos_y_offset = OSD start Y
	wait_queue_head_t osd_wait_queue;
	unsigned int osd_wait_done;
#endif
#if (CONFIG_AST1500_SOC_VER != 2)
	unsigned int vw_max_row; // 0:disable, 1:/2, 2:/3, 3:/4
	unsigned int vw_max_col; // 0:disable, 1:/2, 2:/3, 3:/4
	unsigned int vw_row_num; //0, 1, 2, 3
	unsigned int vw_col_num; //0, 1, 2, 3
	unsigned int vw_fc_h_total; // frame comp. in 0.1mm. include left and right border
	unsigned int vw_fc_h_active; // frame comp. in 0.1mm
	unsigned int vw_fc_v_total; // frame comp. in 0.1mm. include top and bottom border
	unsigned int vw_fc_v_active; //  frame comp.in 0.1mm
	unsigned int vw_extra_h_scale_up; // in pixel
	unsigned int vw_extra_v_scale_up; // in pixel
	int vw_h_shift; // shift of horizontal start position.
	int vw_v_shift; // shift of vertical start position.
	unsigned int vw_enabled;
	unsigned int vw_stretch_type;
#if defined(VW_API_V2_SUPPORT)
	u32 vw_top_left_x;
	u32 vw_top_left_y;
	u32 vw_bottom_right_x;
	u32 vw_bottom_right_y;
	u32 vw_ver;
#endif /* #if defined(VW_API_V2_SUPPORT) */
#endif /* #if (CONFIG_AST1500_SOC_VER != 2) */
	eVIDEO_ROTATE vw_rotate;
	unsigned int dual_port;
	unsigned int dual_port_exist;
	struct semaphore crt_lock;
	unsigned int op_state; //For COPS_XXXX
	void (*ve_scale_callback)(unsigned int line, unsigned int factor, eVIDEO_ROTATE rotate);
	spinlock_t	crt_reg_lock;
	u32 reg84;
};

typedef void (*crt_work_func_t)(struct s_crt_drv *, void *data_ptr);

struct s_crt_work {
	struct work_struct work;
	struct s_crt_drv *crt;
	crt_work_func_t func;
	void *ptr;
	unsigned int free_ptr;
};

typedef void (*PFN_CRT_Tx_Hotplug_Callback)(unsigned int /*plugged*/);

void enable_vsync_int(unsigned int nCrt);

#if (CONFIG_AST1500_SOC_VER >= 2)
void disable_vsync_int(unsigned int nCrt);
#else
#define disable_vsync_int(nCrt) \
do { \
	if (nCrt == CRT_1) { \
		gpio_cfg(AST1500_GPH5, AST1500_GPIO_INT_DISABLE); \
		gpio_ack_int_stat(AST1500_GPH5); \
	} else { /*CRT_2*/ \
		gpio_cfg(AST1500_GPD1, AST1500_GPIO_INT_DISABLE); \
		gpio_ack_int_stat(AST1500_GPD1); \
	} \
} while (0)
#endif

#if (CONFIG_AST1500_SOC_VER == 1)
#define stop_vsync_int() \
do { \
	disable_vsync_int(CRT_1); \
	disable_vsync_int(CRT_2); \
} while (0)
#else
#define stop_vsync_int() \
do { \
	disable_vsync_int(CRT_2); \
} while (0)
#endif

static inline void start_vsync_int(int nCrt)
{
	stop_vsync_int();
	enable_vsync_int(nCrt);
}

#if (CONFIG_AST1500_SOC_VER >= 2)
unsigned int get_crt_vsync_int_stat_atomic(int nCrt);
void ack_crt_vsync_int_stat_atomic(int nCrt);
#else
static inline unsigned int get_crt_vsync_int_stat_atomic(int nCrt)
{
	if (nCrt == CRT_1) {
		return gpio_get_int_stat(AST1500_GPH5);
	} else {
		return gpio_get_int_stat(AST1500_GPD1);
	}
}

static inline void ack_crt_vsync_int_stat_atomic(int nCrt)
{
	if (nCrt == CRT_1) {
		gpio_ack_int_stat(AST1500_GPH5);
	} else {
		gpio_ack_int_stat(AST1500_GPD1);
	}
}
#endif

static inline unsigned int get_vsync_int_stat(int nCrt)
{
	return get_crt_vsync_int_stat_atomic(nCrt);
}

static inline void ack_vsync_int(int nCrt)
{
	ack_crt_vsync_int_stat_atomic(nCrt);
}


/* Convert RGB888 to YUV:
** Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16
** U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128
** V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128
*/
#define PURPLE_YUV444 0x49DDA600
#define PURPLE_RGB565 0x801F801F
#define PURPLE_XRGB888 0xFF8000FF
#define BLACK_YUV444  0x10808000
#define BLACK_RGB565 0x00000000
#define BLACK_XRGB888 0xFF000000
#define BLUE_YUV444  0x29F06E00
#define BLUE_RGB565 0x001F001F
#define BLUE_XRGB888 0xFF0000FF
#define WHITE_YUV444  0xEB808000
#define WHITE_RGB565 0xFFFFFFFF
#define WHITE_XRGB888 0xFFFFFFFF
#define GREY_YUV444  0x80808080

void queue_crt_work_n_wait(struct s_crt_drv *crt, crt_work_func_t func, void *ptr, unsigned int free_ptr);

u32 crt_init(struct s_crt_drv *crt);
void crt_exit(struct s_crt_drv *crt);
int lookupModeTable(struct s_crt_drv *crt, struct s_crt_info *info);
void verify_timing_table(struct s_crt_drv *crt);


/* Exported Functions */
extern unsigned char AST_EDID_HDMI_4K[];
extern unsigned char AST_EDID_HDMI[];
extern unsigned char AST_EDID_DVI[];
extern unsigned char AST_EDID_VGA[];

extern void vbufset(unsigned char *vbuf, u32 width, u32 height, eVIDEO_FORMAT cf, u32 color);
extern void crt_change_scale_factor(struct s_crt_drv *crt, unsigned int hor_scale, unsigned int ver_scale, unsigned int half_down_scale);
extern void crt_change_scale_factor_atomic(struct s_crt_drv *crt, unsigned int hor_scale, unsigned int ver_scale, unsigned int half_down_scale);
extern void crt_set_vbuf_atomic(struct s_crt_drv *crt, u32 buffer_offset);
extern unsigned int crt_to_console_screen(struct s_crt_drv *crt);
extern unsigned int crt_to_decode_screen(struct s_crt_drv *crt);
extern unsigned int crt_to_loopback_screen(struct s_crt_drv *crt);
extern unsigned int crt_setup_xmiter_only_decode(struct s_crt_drv *crt);
extern void crt_setup_par(struct s_crt_drv *crt, struct s_crt_info *info);
extern void crt_reset_par(struct s_crt_drv *crt, unsigned int owner);
extern struct s_crt_drv *crt_connect(unsigned int owner);
extern struct s_crt_drv *crt_get(void);
extern void crt_disconnect(struct s_crt_drv *crt, unsigned int owner);
extern void crt_hotplug_tx(int plug);
extern void crt_reg_tx_hotplug_callback(PFN_CRT_Tx_Hotplug_Callback);
extern void crt_dereg_tx_hotplug_callback(void);
extern int crt_match_timing_table(PVIDEO_MODE_INFO pRefInfo, int *preset_match);
extern MODE_ITEM *crt_timing_by_index(u32);
extern MODE_ITEM *crt_timing_by_sn(u32);
extern u32 crt_timing_index_to_sn(u32 index);
extern u32 crt_timing_sn_to_index(u32 sn);
extern u32 crt_timing_index_3d_to_2d(u32 index);
extern u32 crt_timing_index_4k_to_1080p(u32 index);
extern u32 crt_timing_index_4k_to_4k_low(u32 index);
extern u32 crt_timing_index_4k_low_420_to_4k_low_444(u32 index);
extern u32 crt_timing_index_i_to_p(u32 index, unsigned int down_convert_1080i);
extern u32 crt_timing_hdmi_vic_to_sn(u32 vic);

extern unsigned int crt_pwr_save(struct s_crt_drv *crt, int do_pwr_save);

#if 0//steven:useless
extern unsigned int crt_setup_xmiter(struct s_crt_drv *crt, struct s_crt_info *info);
#endif
#if 0//steven:not used outside of CRT
unsigned int crt_setup(struct s_crt_drv *crt, unsigned int owner);
#endif
unsigned int crt_setup_audio(struct s_crt_drv *crt, unsigned int owner);
unsigned int crt_disable_audio(struct s_crt_drv *crt, unsigned int owner);
unsigned int crt_get_tx_cap(struct s_crt_drv *crt);
void _crt_cfg_dual_port(struct s_crt_drv *crt, unsigned int on);
void _crt_force_sync_off(struct s_crt_drv *crt, int on);
unsigned int crt_get_force_timing_idx(struct s_crt_drv *crt);
MODE_ITEM *crt_get_current_mode(struct s_crt_drv *crt, unsigned int owner, unsigned int *is_YUV420_out);

#if (CONFIG_AST1500_SOC_VER >= 2)
extern unsigned int crt_is_in_odd_field_atomic(void);
void crt_disable_flip(void);
void crt_enable_flip(void);
void crt_disable_desktop(struct s_crt_drv *crt);
void crt_enable_desktop(struct s_crt_drv *crt);
void crt_enable_desktop_atomic(struct s_crt_drv *crt);
void crt_vsync_intr_location(struct s_crt_drv *crt, int fine_tune);
#endif

#if (CONFIG_AST1500_SOC_VER != 2)
#if defined(VW_API_V2_SUPPORT)
/*
 * the reason we define VW_COORDINATE_MAX (scale) 10000:
 * one 90 inch tv is ~ 203cm x 121cm
 * for 8x8 video wall, the wall size is ~ 16 m x 9 m (1600 cm x 900 cm)
 * if installation tolerances is 0.2cm (2mm)
 * 1600cm/0.2cm = 8000, so choose a number > 8000
 */
#define VW_COORDINATE_MAX 10000
#endif
void crt_vw_enable(struct s_crt_drv *crt, struct s_crt_info *i);
void crt_force_rgb_output_cfg(struct s_crt_drv *crt, u32 enable);
u32 crt_force_rgb_output(struct s_crt_drv *crt);
#endif /* #if (CONFIG_AST1500_SOC_VER != 2) */

#if (CONFIG_AST1500_SOC_VER >= 3)
u32 crt_in_yuv422(void);
void crt_sw_flip_atomic(u32 cfg, u32 yuv422);
void crt_display_address_cfg_atomic(u32 address);
#elif (CONFIG_AST1500_SOC_VER >= 2)
u32 crt_terminal_cnt(void);
#endif
void crt_set_csc_format(unsigned int en_bt709, u32 full_range);
unsigned int crt_ft_cfg_to_mode_idx(struct s_crt_drv *crt, unsigned int cfg);

//This routine is different from crt_to_console_screen in that it will not change owner of crt to console.
void crt_display_console(void);

#if SUPPORT_HDCP_REPEATER
//interface between video driver and HDMI Tx driver
typedef void (*pfn_Update_HDCP1_Downstream_Port_Auth_Status_Callback)(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
void CRT_HDCP1_Downstream_Port_Set_Mode(unsigned char repeater);
void CRT_HDCP1_Downstream_Port_Auth(int enable);
void CRT_HDCP1_Downstream_Port_Encrypt(unsigned char enable);
void CRT_HDCP1_Downstream_Port_Reg_Auth_Status_Callback(pfn_Update_HDCP1_Downstream_Port_Auth_Status_Callback);
void CRT_HDCP1_Downstream_Port_Dereg_Auth_Status_Callback(void);
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif

extern void vbufset_test_mode(struct s_crt_drv *crt, struct s_crt_info *i, u32 type, u32 color, u32 color2);
#include "xmiter_hal.h"

unsigned int crt_cec_send(struct s_crt_drv *crt, u8 *buf, u32 size);
unsigned int crt_cec_topology(struct s_crt_drv *crt, u8 *buf, u32 scan);
void crt_cec_pa_cfg(struct s_crt_drv *crt, u16 address);
void crt_cec_la_cfg(struct s_crt_drv *crt, u8 address);
#endif /* _CRT_H_ */


