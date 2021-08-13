/*
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      :
 *    1. 2013/04/04 Bruce first version
 *
********************************************************************************/

#ifndef _AST_SCU_AST15XX_H_
#define _AST_SCU_AST15XX_H_

typedef enum {
	VT_ANALOG   = 0x0,
	VT_HDMI     = 0x2,
	VT_DVI      = 0x3,
	VT_DISABLE  = 0x1,
} video_type_e;

typedef enum {
	A30_IPE5000   = 0x0,
	A30_IPE5000P     = 0x1,
	A30_IPE5000W      = 0x2,
	A30_IPD5000  = 0x3,
	A30_IPD5000W  = 0x4,
	A30_BOARD_CNT
} a30_board_type;

typedef struct{
	a30_board_type type_index;
	char *model_number;
}a30_board_map;

#define I2S_SAMPLE_RATE_32K    3
#define I2S_SAMPLE_RATE_44d1K  0
#define I2S_SAMPLE_RATE_48K    2
#define I2S_SAMPLE_RATE_88d2K  8
#define I2S_SAMPLE_RATE_96K    10
#define I2S_SAMPLE_RATE_176d4K 12
#define I2S_SAMPLE_RATE_192K   14

#define BOARD_REV_PATCH_NONE			0x00000000
#define BOARD_REV_PATCH_SOC_VERSION_MASK	(0xF << 8)
#define BOARD_REV_PATCH_SOC_VERSION(rev)	((rev) & BOARD_REV_PATCH_SOC_VERSION_MASK)

/* board revsion patch definition on AST1520/AST1525*/
#define BOARD_REV_PATCH_MINOR_VERSION_MASK	(0xF << 4)
#define BOARD_REV_PATCH_MINOR_VERSION(rev)	((rev) & BOARD_REV_PATCH_MINOR_VERSION_MASK)
#define BOARD_REV_PATCH_RESERVED_3		(0x1 << 3) /* reserved for future use */
#define BOARD_REV_PATCH_RESERVED_2		(0x1 << 2) /* reserved for customer */
#define BOARD_REV_PATCH_RESERVED_1		(0x1 << 1) /* reserved for customer */
#define BOARD_REV_PATCH_VIDEO_SPLITTER		(0x1 << 0)

#define BOARD_REV_PATCH_DEFAULT		BOARD_REV_PATCH_NONE

#define CRT_CONSOLE_DEFAULT_X    1280
#define CRT_CONSOLE_DEFAULT_Y    720
#define CRT_CONSOLE_DEFAULT_RR   60 //Refresh rate
#define CRT_CONSOLE_DEFAULT_HDMI 1 /* is HDMI? */

#define HDCP_CTS_DISABLE_AUTO_FW_UPDATE  (0x1UL << 31)
#define HDCP_CTS_HDCP14_ONLY             (0x1UL << 0) //SiI9678 Only. Useless so far.
#define HDCP_CTS_HDCP22_ONLY             (0x1UL << 1) //SiI9678 Only. Useless so far.
#define HDCP_CTS_TMDS_ALWAYS_ON          (0x1UL << 2) //SiI9136 patch. Never turn off TMDS for passing HDCP 2.2 CTS.
#define HDCP_CTS_AUTO_TEST_MODE          (0x1UL << 3) //Client Only. Automatically start test mode for HDCP 2.2 CTS.

typedef struct {
	u32 is_client; // 0: host, 1: client
	video_type_e video_type;
	video_type_e video_loopback_type; //host only
	unsigned int is_internal_video_capture; //host only
	u32 board_revision;
	u32 ir_on;
	unsigned int soc_ver; // 1:AST1500, 2:AST1510, 3:AST1520
	unsigned int soc_revision; // X0: A0, X1: A1. Where 'X' is SOC_VER
	unsigned int ve_encode_clk;
} board_info_t;

typedef struct {
	unsigned int v_vga_hotplug_det_test;    //#define NEW_VGA_HOTPLUG_DET_TEST 0 /* This is special testing code to detect CRT DAC's hotplug. */
	unsigned int v_interlace_mode;          //#define NEW_INTERLACE_MODE 1
	unsigned int v_dither_detect;           //#define NEW_DITHER_DETECT 1
	unsigned int v_ignore_hsync_glitch;     //#define NEW_IGNORE_HSYNC_GLITCH 1
	unsigned int v_auto_vsync_delay;        //#define NEW_AUTO_VSYNC_DELAY 1
	unsigned int v_mode_detection;          //#define NEW_MODE_DET 1
	unsigned int v_hw_profile;              //#define HW_PROFILE 0
	unsigned int v_420;                     //#define NEW_420 1 /* Need to modify Config_800.inf to disable pass2/3. */
	unsigned int v_420_default;             //#define NEW_420_DEFAULT 0 /* Need to modify Config_800.inf to disable pass2/3. */
	unsigned int v_shift_bits;              //#define NEW_SHIFT_BITS 1
	unsigned int v_24bits_mode;             //#define NEW_24BIT_MODE 0 /* AST1520 doesn't support NEW_24BIT_MODE */
	unsigned int v_delay_bcd;               //#define NEW_DELAY_BCD 1
	unsigned int v_abcd;                    //#define NEW_ABCD 1
	unsigned int v_desc_dma_mode;           //#define NEW_DESC_DMA_MODE 0
	unsigned int v_anti_tearing_mode;       //#define NEW_ANTI_TEARING_MODE 0
	unsigned int v_bcd;                     //#define NEW_BCD 0 /* Double buffer BCD */
	unsigned int v_gen_lock;                //#define NEW_GEN_LOCK 0 /* HW VSync packet for gen lock */
	unsigned int v_compatibility_mode;      //#define NEW_COMPATIBILITY_MODE (0)
	unsigned int v_support_4k;              //#define NEW_SUPPORT_4K 0
	unsigned int v_csc;
	unsigned int v_crt_flip;
	unsigned int soc_op_mode;               /* 1: AST1500 2:AST1510 3:AST1520 */
} ability_info_t;

typedef struct {
	/* Default CRT console screen's resolution. */
	unsigned int console_default_x;
	unsigned int console_default_y;
	unsigned int console_default_rr;
	unsigned int console_default_is_hdmi;
	/* HDCP_CTS_XXXX some HDCP CTS options */
	unsigned int hdcp_cts_option; // See HDCP_CTS_XXXX
	/* KMoIP options */
	unsigned int kmoip_poll_interval; /* Used by host and client */
	/* GPIO options */
	unsigned int gpio_inv; /* 32bits bitmap. Used to force invert GPIO read value. */
	/* Audio options */
	unsigned int a_force_dual_output; /* force dual output even under old soc_op_mode. */
#if (CONFIG_AST1500_SOC_VER >= 3)
	/* Target ring value. Used to find optimized default v_clk_delay. */
	unsigned int v_clk_delay_target;
	/* CRT Tx Clock Delay. Maps to SCUD8, but only takes SCUD8[6:11]. */
	unsigned int v_clk_delay;
	/* CRT Tx Clock Duty. Apply to 4K case only. */
	unsigned int v_clk_duty;
#endif

	/* USB quirk */
/* host */
/* #define VHUB_QUIRK_BULK_SSPLIT              (0x1UL << 0)  defined in vhub.h */
/* client */
#define USB_HID_QUIRK_WRONG_EP_NUM          (0x1UL << 16)
#define USB_HID_QUIRK_CLEAR_HALT_ON_INIT    (0x1UL << 17)
#define USBIP_QUIRK_NO_RESET_TWICE          (0x1UL << 18)

	unsigned int usb_quirk;

	/* Video Transmitter Option. For SiI9136. */
	/*
	 * [1:0]: clock edge select: 1: rising, 0: falling
	 * [1]: 4K clock edge select
	 * [0]: 1080 clock edge select
	 */
	unsigned int v_tx_drv_option;
	/*
	 * net driver option
	 * [0]: for RTL8211FS PHY, fiber & copper coexist at the same board with single one PHY
	 *  0: auto detection (hardware)
	 *  1: software forced the mode, we switch between fiber and copper in each reset operation
	 */
	unsigned int net_drv_option;

	/* qzx2021.8.13:use char *model_number before,but I don't know why in rtl8364_i2c_driver.c it will be NULL,and it is useful in leds-ast1500.c.
					So I don't use char *,I use emun type */
	a30_board_type model_number;
} astparam_t;

void scu_init_ability(ability_info_t *ab);
void scu_init_astparam(void *scu, astparam_t *astparam);
int astparam_init_shadow(void);
void astparam_destroy_shadow(void);
int astparam_get_hex(char *name, unsigned int *result);
int astparam_get_yn(char *name, unsigned int *result);
int astparam_get_int(char *name, unsigned int *result);

#endif //#ifndef _AST_SCU_AST15XX_H_
