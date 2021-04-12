/*
 *
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *   History      :
 *    1. 2014/12/19 Bruce first version
 *
********************************************************************************/


#include <asm/arch/ast-scu.h>
#include <aspeed/features_video.h>
#include "ast-scu-drv.h"
#define ASTPARAM


#ifdef ASTPARAM
// This program assumes both RW and RO block uses the same size (64KB).
#define RW_OFFSET (ASPEED_SMC_FLASH_BASE+0xFE0000)
#define RO_OFFSET (ASPEED_SMC_FLASH_BASE+0xFF0000)
#define BLOCK_SIZE 0x10000 //64KB

static char *buf_rw = NULL;
static char *buf_ro = NULL;
static  unsigned long crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);
static unsigned long crc32(unsigned long crc, char *buf, unsigned int len)
{
    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}

static int matchParam(char *buffer, char *s1, int i2)
{
	char *param = buffer + 4;
	while (*s1 == param[i2++])
		if (*s1++ == '=')
			return(i2);
	if ((*s1 == '\0') && (param[i2 - 1] == '='))
		return(i2);
	return(-1);
}

static int getParam(char *buffer, char *name)
{
	int j, k, nxt;
	char *param = buffer + 4;

		k = -1;

		for (j = 0; param[j] != '\0'; j = nxt+1) {
			for (nxt = j; param[nxt] != '\0'; ++nxt)
				;

			k = matchParam(buffer, name, j);
			if (k < 0) {
				continue;
			}

			//uinfo("%s=%s\n", name, &param[k]);
			break;
		}

		if (k < 0) {
			//uinfo("\"%s\" not defined\n", name);
			return -1;
		}

		return (k + 4);
}

static char * astparam_find(char *name)
{
	int found_idx = -1;
	char *found = NULL;

	found_idx = getParam(buf_rw, name);
	if (found_idx < 0) {
		found_idx = getParam(buf_ro, name);
		if (found_idx < 0)
			goto out;

		found = &buf_ro[found_idx];
	} else {
		found = &buf_rw[found_idx];
	}
out:
	if (found_idx < 0)
		uinfo("\"%s\" not defined\n", name);
	else
		uinfo("%s=%s\n", name, found);

	return found;
}

int astparam_get_resolution(char *name, unsigned int *x, unsigned int *y, unsigned int *vHz, unsigned int *hdmi)
{
	char *found;
	u32 scanned = 0, ix, iy, ivHz, ihdmi;

	found = astparam_find(name);
	if (!found)
		return -1;

	/* Found result. Convert it */
	scanned = sscanf(found, "%dx%d@%d,%d", &ix, &iy, &ivHz, &ihdmi);

	if (scanned >= 3) {
		*x = ix;
		*y = iy;
		*vHz = ivHz;
		if (scanned >= 4)
			*hdmi = ihdmi;
	}
	uinfo("%s Resolution:%dx%d@%dHz,%d\n", name, *x, *y, *vHz, *hdmi);

	return 0;
}

int astparam_get_hex(char *name, unsigned int *result)
{
	char *found;

	found = astparam_find(name);
	if (!found)
		return -1;

	/* Found result. Convert it */
	*result = (unsigned int)simple_strtoul(found, NULL, 16);

	return 0;
}

int astparam_get_int(char *name, unsigned int *result)
{
	char *found;

	found = astparam_find(name);
	if (!found)
		return -1;

	/* Found result. Convert it */
	*result = (unsigned int)simple_strtoul(found, NULL, 0);

	return 0;
}

int astparam_get_yn(char *name, unsigned int *result)
{
	char *found;

	found = astparam_find(name);
	if (!found)
		return -1;

	/* Found result. Check string length. */
	if (strlen(found) != 1)
		return -1;

	if (*found == 'y')
		*result = 1;
	else
		*result = 0;

	return 0;
}

int astparam_init_shadow(void)
{
	char *ro = NULL, *rw = NULL;

	buf_ro = kmalloc(BLOCK_SIZE, GFP_KERNEL);
	if (buf_ro == NULL)
		goto fail;

	buf_rw = kmalloc(BLOCK_SIZE, GFP_KERNEL);
	if (buf_rw == NULL)
		goto fail;


	ro = ioremap(RO_OFFSET, BLOCK_SIZE);
	rw = ioremap(RW_OFFSET, BLOCK_SIZE);
	if ((ro == NULL) || (rw == NULL))
		goto fail;

	memcpy(buf_ro, ro, BLOCK_SIZE);
	memcpy(buf_rw, rw, BLOCK_SIZE);

	/* Valid buf. If invalid, just NULL the buf. */
	if ((*(unsigned int *)buf_ro == 0)
		|| (*(unsigned int*)buf_ro != crc32(0, buf_ro + 4, BLOCK_SIZE - 4))) {
		/* Set buf to empty */
		memset(buf_ro + 4, '\0', 8);
	}

	if ((*(unsigned int *)buf_rw == 0)
		|| (*(unsigned int*)buf_rw != crc32(0, buf_rw + 4, BLOCK_SIZE - 4))) {
		/* Set buf to empty */
		memset(buf_rw + 4, '\0', 8);
	}

	iounmap(ro);
	iounmap(rw);

	return 0;

fail:
	iounmap(ro);
	iounmap(rw);
	astparam_destroy_shadow();
	uerr("astparam_init_shadow failed?!\n");
	BUG();
	return -1;
}

void astparam_destroy_shadow(void)
{
	if (buf_rw)
		kfree(buf_rw);

	if (buf_ro)
		kfree(buf_ro);

	buf_rw = NULL;
	buf_ro = NULL;
}


#endif /* #ifdef ASTPARAM */

static unsigned int get_hdcp_cts_option(void)
{
	unsigned int result;

	if (astparam_get_hex("hdcp_cts_option", &result)) {
		/* No "hdcp_cts_option" defined. Use default: 0 */
		result = 0;
	}
	return result;
}

static unsigned int get_gpio_inv(void)
{
	unsigned int result;

	if (astparam_get_hex("gpio_inv", &result)) {
		/* No "gpio_inv" defined. Use default: 0 */
		result = 0;
	}
	return result;
}

static unsigned int get_kmoip_poll_interval(scu_t *scu)
{
	unsigned int result;

	if (astparam_get_int("kmoip_poll_interval", &result)) {
		/* No "kmoip_poll_interval" defined. Use default.*/
		/*
		** Bruce160629.
		** AST1500/AST1510 CPU is limited. KMoIP mouse will have long long latency
		** when video encode is busy. Set kmoip_poll_interval >= 35 to resolve
		** this issue. Someday, we should look into this issue and find a better
		** solution.
		** AST152x works fine with kmoip_poll_interval set to 10.
		*/
		if (scu->ability.soc_op_mode >= 3)
			result = 10;
		else
			result = 35;
	}
	return result;
}

static unsigned int get_a_force_dual_output(void)
{
	unsigned int result;

	if (astparam_get_yn("a_force_dual_output", &result)) {
		/* No "a_force_dual_output" defined. Use default: 0 */
		result = 0;
	}
	return result;
}

static unsigned int get_v_tx_drv_option(void)
{
	unsigned int result;

	if (astparam_get_hex("v_tx_drv_option", &result)) {
		/* No "v_tx_drv_option" defined. Use default. */
		/*
		** Bruce170526.
		** From test result. The best clock range is:
		** For SiI9136 falling latch:
		** - clk duty SCU1D8: 0x63
		** - clk delay SCUD8: 0x63d ~ 0x77d. around ring 7000 ps. ==> has higher variation between chips.
		** For SiI9136 rising latch:
		** - clk duty SCU1D8: 0x63. 0x65 failed under high temperature.
		** - clk delay SCUD8: 0x23d ~ 0x37d. around ring 3400 ps
		**
		** We pick rising latch because smaller SCUD8 has better/stable clk SSO result.
		**
		** Confirmed using SI's eva board and our measurement.
		** SiI9136's latch setting is different under 4K case.
		** When SiI9136 Reg08[4] set to 0. 9136 is falling + delay half clock (1.68ns) + 0.46ns (for red line issue) latch. ==> delay 2.14ns
		** When SiI9136 Reg08[4] set to 1. 9136 is rising + delay 1.37ns latch.
		** For non-4K case,
		** When SiI9136 Reg08[4] set to 0. 9136 is rising latch.
		** When SiI9136 Reg08[4] set to 1. 9136 is falling latch.
		*/
		/*
		** Bruce170511.
		** When SiI9136 Reg08[4] set to 0, we get data latch error in Red pin even under 720p.
		** When SiI9136 Reg08[4] set to 1, issue resolved.
		** ==> 0 means 9136 rising edge latch. 1 means falling edge latch.
		** Current code wording could be wrong. Need to confirm with SI.
		** ==> Set to 0x3 is the way to go.
		*/
		/* SiI9136: DRVOP_EDGE_SELECT */
		result = 0x3;
	}
	return result;
}

static unsigned int get_net_drv_option(void)
{
	unsigned int result;

	if (astparam_get_hex("net_drv_option", &result)) {
		/* No "net_drv_option" defined. Use default. */
		result = 0x0;
	}
	return result;
}

static void scu_chg_ability(ability_info_t *ab, unsigned int ver)
{
	uinfo("Use SoC OP Mode %d under SoC Ver %d\n", ver, ast_scu.board_info.soc_ver);

	switch (ver) {
	case 1: /* cfg to support AST1500 */
		if (ast_scu.board_info.soc_ver <= 1)
			break;
		//ab->v_vga_hotplug_det_test = NEW_VGA_HOTPLUG_DET_TEST;
		/*
		** Bruce161124. AST152X -> AST1500 interlace mode.
		**
		** VideoDev.progressively_capture_interlace == 0:
		** Host keep interlace HW capability and encode interlace mode as 2 fields in one frame.
		** Client decode interlace frame, but display only upper part of frame.
		** ==> User will see only first field information.
		**
		** VideoDev.progressively_capture_interlace == 1:
		** Host capture interlace timing using progressive way.
		** ==> each field is captured as separated frame.
		** Client decode each frame and display odd/even frame one by one.
		** ==> User will see picture flashing, but get complete odd/even information.
		*/
		//ab->v_interlace_mode = 0;
		//ab->v_dither_detect = NEW_DITHER_DETECT;
		//ab->v_ignore_hsync_glitch = NEW_IGNORE_HSYNC_GLITCH;
		//ab->v_auto_vsync_delay = NEW_AUTO_VSYNC_DELAY;
		//ab->v_mode_detection = NEW_MODE_DET;
		ab->v_hw_profile = 0;
		ab->v_420 = 0;
		ab->v_420_default = 0;
		ab->v_shift_bits = 0;
		ab->v_24bits_mode = 0;
		ab->v_delay_bcd = 0;
		//ab->v_abcd = NEW_ABCD;
		ab->v_desc_dma_mode = 0;
		/*
		 * patch v_anti_tearing_mode to disable VE double buffer
		 * this is a workaround for special wall case which source/display refresh rate is unmatched
		 */
		ab->v_anti_tearing_mode = 0;
		//ab->v_bcd = NEW_BCD;
		ab->v_gen_lock = 0;
		ab->v_compatibility_mode = (NC_PITCH_ALIGN32 | NC_1680_TO_1664);
		ab->v_support_4k = 0;
		ab->v_csc = 0;
		ab->soc_op_mode = 1;
		break;
	case 2: /* cfg to support AST1510 */
		if (ast_scu.board_info.soc_ver <= 2)
			break;
		//ab->v_vga_hotplug_det_test = NEW_VGA_HOTPLUG_DET_TEST;
		ab->v_interlace_mode = 2;
		//ab->v_dither_detect = NEW_DITHER_DETECT;
		//ab->v_ignore_hsync_glitch = NEW_IGNORE_HSYNC_GLITCH;
		//ab->v_auto_vsync_delay = NEW_AUTO_VSYNC_DELAY;
		//ab->v_mode_detection = NEW_MODE_DET;
		ab->v_hw_profile = 2;
		ab->v_420 = 2;
		//ab->v_420_default = NEW_420_DEFAULT;
		//ab->v_shift_bits = NEW_SHIFT_BITS;
		ab->v_24bits_mode = 0;
		//ab->v_delay_bcd = NEW_DELAY_BCD;
		//ab->v_abcd = NEW_ABCD;
		ab->v_desc_dma_mode = 0;
		/* just do the same thing as case 1 */
		ab->v_anti_tearing_mode = 0;
		//ab->v_bcd = NEW_BCD;
		ab->v_gen_lock = 0;
		ab->v_compatibility_mode = NC_PITCH_ALIGN32;
		ab->v_support_4k = 0;
		ab->v_csc = 0;
		ab->soc_op_mode = 2;
		break;
	default:
		break;
	};
}

/*
** board_info MUST be ready before calling _scu_init_ability()
*/
void scu_init_ability(ability_info_t *ab)
{
	unsigned int ver = 0;

	ab->v_vga_hotplug_det_test = NEW_VGA_HOTPLUG_DET_TEST;
	ab->v_interlace_mode = NEW_INTERLACE_MODE;
	ab->v_dither_detect = NEW_DITHER_DETECT;
	ab->v_ignore_hsync_glitch = NEW_IGNORE_HSYNC_GLITCH;
	ab->v_auto_vsync_delay = NEW_AUTO_VSYNC_DELAY;
	ab->v_mode_detection = NEW_MODE_DET;
	ab->v_hw_profile = HW_PROFILE;
	ab->v_420 = NEW_420;
	ab->v_420_default = NEW_420_DEFAULT;
	ab->v_shift_bits = NEW_SHIFT_BITS;
	ab->v_24bits_mode = NEW_24BIT_MODE;
	ab->v_delay_bcd = NEW_DELAY_BCD;
	ab->v_abcd = NEW_ABCD;
	ab->v_desc_dma_mode = NEW_DESC_DMA_MODE;
	ab->v_anti_tearing_mode = NEW_ANTI_TEARING_MODE;
	ab->v_bcd = NEW_BCD;
	ab->v_gen_lock = NEW_GEN_LOCK;
	ab->v_compatibility_mode = NEW_COMPATIBILITY_MODE;
	ab->v_support_4k = NEW_SUPPORT_4K;
	ab->v_csc = NEW_CSC;
	ab->v_crt_flip = NEW_CRT_FLIP;
	ab->soc_op_mode = ast_scu.board_info.soc_ver;

	if (astparam_get_hex("soc_op_mode", &ver) < 0)
		ver = ast_scu.board_info.soc_ver;

	scu_chg_ability(ab, ver);
}

void scu_init_astparam(void *context, astparam_t *astparam)
{
	scu_t *scu = (scu_t *)context;

	astparam->console_default_x = CRT_CONSOLE_DEFAULT_X;
	astparam->console_default_y = CRT_CONSOLE_DEFAULT_Y;
	astparam->console_default_rr = CRT_CONSOLE_DEFAULT_RR;
	astparam->console_default_is_hdmi = CRT_CONSOLE_DEFAULT_HDMI;

	astparam_get_resolution("ui_default_res",
		&astparam->console_default_x, &astparam->console_default_y,
		&astparam->console_default_rr, &astparam->console_default_is_hdmi);

	astparam->hdcp_cts_option = get_hdcp_cts_option();
	astparam->kmoip_poll_interval = get_kmoip_poll_interval(scu);
	astparam->gpio_inv = get_gpio_inv();
	astparam->a_force_dual_output = get_a_force_dual_output(); /* force dual output even under old soc_op_mode. */
	astparam->v_tx_drv_option = get_v_tx_drv_option();
	astparam->net_drv_option = get_net_drv_option();
}
