#ifndef _FEATURES_VIDEO_H_
#define _FEATURES_VIDEO_H_

/* Defines for NEW_COMPATIBILITY_MODE (0x0) */
#define NC_LNG_BLK_HDR   (0x1UL << 0)
#define NC_FUL_YUV       (0x1UL << 1)
#define NC_PITCH_ALIGN32 (0x1UL << 2) /* Pitch must be multiple of 32. SoC V1,V2 */
#define NC_1680_TO_1664  (0x1UL << 3) /* SoC V1 need patch 1680x1050 to 1664x1050 */

#if (CONFIG_AST1500_SOC_VER >= 3)
	#define NEW_VGA_HOTPLUG_DET_TEST 0 /* This is special testing code to detect CRT DAC's hotplug. */
	#define NEW_INTERLACE_MODE 2
	#define NEW_DITHER_DETECT 2
	#define NEW_IGNORE_HSYNC_GLITCH 2
	#define NEW_AUTO_VSYNC_DELAY 2
	#define NEW_MODE_DET 3
	#define HW_PROFILE 3
	#define NEW_420 3
	#define NEW_420_DEFAULT 0 /* Need to modify Config_800.inf to disable pass2/3. */
	#define NEW_SHIFT_BITS 2
	#define NEW_24BIT_MODE 0 /* AST1520 doesn't support NEW_24BIT_MODE */
	#define NEW_DELAY_BCD 2
	#define NEW_ABCD 2
	#define NEW_DESC_DMA_MODE 3
	#define NEW_ANTI_TEARING_MODE 3
	#define NEW_BCD 3 /* Double buffer BCD */
	#define NEW_GEN_LOCK 3 /* HW VSync packet for gen lock */
	#define NEW_COMPATIBILITY_MODE (NC_LNG_BLK_HDR | NC_FUL_YUV)
	#define NEW_SUPPORT_4K 3
	#define NEW_CSC 3
	#define NEW_CRT_FLIP 3
#elif (CONFIG_AST1500_SOC_VER == 2)
	#define NEW_VGA_HOTPLUG_DET_TEST 0 /* This is special testing code to detect CRT DAC's hotplug. */
	#define NEW_INTERLACE_MODE 2
	#define NEW_DITHER_DETECT 2
	#define NEW_IGNORE_HSYNC_GLITCH 2
	#define NEW_AUTO_VSYNC_DELAY 2
	#define NEW_MODE_DET 2
	#define HW_PROFILE 2
	#define NEW_420 2 /* Need to modify Config_800.inf to disable pass2/3. */
	#define NEW_420_DEFAULT 0 /* Need to modify Config_800.inf to disable pass2/3. */
	#define NEW_SHIFT_BITS 2
	#define NEW_24BIT_MODE 2
	#define NEW_DELAY_BCD 2
	#define NEW_ABCD 2
	#define NEW_DESC_DMA_MODE 0
	#define NEW_ANTI_TEARING_MODE 0
	#define NEW_BCD 0
	#define NEW_GEN_LOCK 0 /* HW VSync packet for gen lock */
	#define NEW_COMPATIBILITY_MODE (NC_PITCH_ALIGN32)
	#define NEW_SUPPORT_4K 0
	#define NEW_CSC 0
	#define NEW_CRT_FLIP 2
#else
	/* (CONFIG_AST1500_SOC_VER == 1) */
	#define NEW_VGA_HOTPLUG_DET_TEST 0
	#define NEW_INTERLACE_MODE 0
	#define NEW_DITHER_DETECT 0
	#define NEW_IGNORE_HSYNC_GLITCH 0
	#define NEW_AUTO_VSYNC_DELAY 0
	#define NEW_MODE_DET 0
	#define HW_PROFILE 0
	#define NEW_420 0 /* Need to modify Config_800.inf to disable pass2/3. */
	#define NEW_420_DEFAULT 0 /* Need to modify Config_800.inf to disable pass2/3. */
	#define NEW_SHIFT_BITS 0
	#define NEW_24BIT_MODE 0
	#define NEW_DELAY_BCD 0
	#define NEW_ABCD 0
	#define NEW_DESC_DMA_MODE 0
	#define NEW_ANTI_TEARING_MODE 0
	#define NEW_BCD 0
	#define NEW_GEN_LOCK 0 /* HW VSync packet for gen lock */
	#define NEW_COMPATIBILITY_MODE (NC_PITCH_ALIGN32 | NC_1680_TO_1664)
	#define NEW_SUPPORT_4K 0
	#define NEW_CSC 0
	#define NEW_CRT_FLIP 0
#endif

#endif /* #define _FEATURES_VIDEO_H_ */

