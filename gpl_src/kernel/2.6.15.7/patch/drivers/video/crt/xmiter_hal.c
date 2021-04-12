/*
 * This file define the interface between CRT and video xmiter
 */
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/crt.h>
#include "astdebug.h"
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/edid.h>
#include <linux/delay.h>

#if 0
static unsigned char AST_EDID_VGA[EDID_ROM_SIZE] =
{
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x74, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x14, 0x01, 0x03, 0x0E, 0x2F, 0x1A, 0x78, 0x2E, 0x35, 0x85, 0xA6, 0x56, 0x48, 0x9A, 0x24,
  0x12, 0x50, 0x54, 0xA5, 0x6B, 0xBA, 0x71, 0x00, 0x81, 0x00, 0x81, 0x40, 0x95, 0x0F, 0x81, 0x80,
  0x95, 0x00, 0xB3, 0x00, 0x01, 0x01, 0x94, 0x43, 0x80, 0x90, 0x72, 0x38, 0x28, 0x40, 0x80, 0xC8,
  0x35, 0x00, 0xDC, 0x0C, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x32, 0x0A, 0x0A,
  0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32,
  0x4C, 0x1E, 0x5E, 0x15, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x41, 0x53, 0x54, 0x31, 0x35, 0x30, 0x30, 0x5F, 0x56, 0x47, 0x41, 0x0A, 0x0A, 0x00, 0x77,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char AST_EDID_DVI[EDID_ROM_SIZE] =
{
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x74, 0x00, 0x15, 0x01, 0x00, 0x00, 0x00,
  0x06, 0x14, 0x01, 0x03, 0x80, 0x2F, 0x1A, 0x78, 0x2E, 0x35, 0x85, 0xA6, 0x56, 0x48, 0x9A, 0x24,
  0x12, 0x50, 0x54, 0xA5, 0x6B, 0xBA, 0x71, 0x00, 0x81, 0x40, 0x81, 0x00, 0x95, 0x0F, 0x81, 0x80,
  0x95, 0x00, 0xB3, 0x00, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
  0x45, 0x00, 0xDD, 0x0C, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x37, 0x37, 0x39,
  0x30, 0x35, 0x35, 0x32, 0x36, 0x30, 0x32, 0x36, 0x0A, 0x20, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32,
  0x4C, 0x1E, 0x5E, 0x15, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x41, 0x53, 0x54, 0x31, 0x35, 0x30, 0x30, 0x5F, 0x44, 0x56, 0x49, 0x0A, 0x0A, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char AST_EDID_HDMI[EDID_ROM_SIZE] =
{
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x74, 0x00, 0x15, 0x02, 0x00, 0x00, 0x00,
  0x06, 0x14, 0x01, 0x03, 0x80, 0x2F, 0x1A, 0x78, 0x2E, 0x35, 0x85, 0xA6, 0x56, 0x48, 0x9A, 0x24,
  0x12, 0x50, 0x54, 0xA5, 0x6B, 0xBA, 0x71, 0x00, 0x81, 0x00, 0x81, 0x40, 0x95, 0x0F, 0x81, 0x80,
  0x95, 0x00, 0xB3, 0x00, 0x01, 0x01, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C,
  0x45, 0x00, 0xDD, 0x0C, 0x11, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x31, 0x0A, 0x0A,
  0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x32,
  0x4C, 0x1E, 0x5E, 0x15, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC,
  0x00, 0x41, 0x53, 0x54, 0x31, 0x35, 0x30, 0x30, 0x5F, 0x48, 0x44, 0x4D, 0x49, 0x0A, 0x01, 0x8B,
  0x02, 0x03, 0x1B, 0xF1, 0x47, 0x04, 0x03, 0x01, 0x00, 0x12, 0x13, 0x00, 0x23, 0x09, 0x1F, 0x07,
  0x83, 0x01, 0x00, 0x00, 0x66, 0x03, 0x0C, 0x00, 0x10, 0x00, 0x00, 0x01, 0x1D, 0x00, 0x72, 0x51,
  0xD0, 0x1E, 0x20, 0x6E, 0x28, 0x55, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1F, 0x01, 0x1D, 0x00,
  0xBC, 0x52, 0xD0, 0x1E, 0x20, 0xB8, 0x28, 0x55, 0x40, 0xC4, 0x8E, 0x21, 0x00, 0x00, 0x1F, 0x8C,
  0x0A, 0xD0, 0x8A, 0x20, 0xE0, 0x2D, 0x10, 0x10, 0x3E, 0x96, 0x00, 0xC4, 0x8E, 0x21, 0x00, 0x00,
  0x19, 0x8C, 0x0A, 0xD0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0C, 0x40, 0x55, 0x00, 0xC4, 0x8E, 0x21,
  0x00, 0x00, 0x19, 0xD6, 0x09, 0x80, 0xA0, 0x20, 0xE0, 0x2D, 0x10, 0x08, 0x60, 0x22, 0x00, 0x12,
  0x8E, 0x21, 0x08, 0x08, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52,
};
#endif

static unsigned int _xHal_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum);

#include "xmiter_common.c"
#if (BOARD_DESIGN_VER_VIDEO >= 204)
#include "xmiter_dac.c"

struct s_xmiter_info xmiter_info[NUM_XMITER] =
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	{ //Client analog xmiter
		.chip = xmiter_internal_dac,
		.cap = xCap_RGB,
		.exist = xmiter_dac_exist,
		.setup = xmiter_setup_dac,
		.setup_audio = NULL,
		.disable = xmiter_disable_dac,
		.disable_audio = NULL,
		.init = xmiter_init_dac,
		.check_hotplug = NULL, //Asume analog interface is always plugged //_check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
	},
	{ //Client digital xmiter
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
	},
#elif defined(CONFIG_ARCH_AST1500_HOST)
	{ //Host loopback digital
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#else
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#endif
	},
	{ //Host loopback analog
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
		.chip = xmiter_internal_dac,
		.cap = xCap_RGB,
		.exist = xmiter_dac_exist,
		.setup = xmiter_setup_dac,
		.setup_audio = NULL,
		.disable = xmiter_disable_dac,
		.disable_audio = NULL,
		.init = xmiter_init_dac,
		.check_hotplug = NULL, //Asume analog interface is always plugged //_check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#else
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#endif
	},
#else //#if defined(CONFIG_ARCH_AST1500_CLIENT)
#endif //#if defined(CONFIG_ARCH_AST1500_CLIENT)
};
#elif (BOARD_DESIGN_VER_VIDEO >= 105)
//#if defined (CONFIG_AST1500_BOARD_V5) || defined (CONFIG_AST1510_BOARD_EVA_V2) || defined (CONFIG_AST1510_BOARD_EVA_V4) || defined(CONFIG_AST1510_BOARD_EVA_V1) || defined(CONFIG_AST1510_BOARD_FPGA)
#include "xmiter_dac.c"

struct s_xmiter_info xmiter_info[NUM_XMITER] =
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	{ //Client CRT1
#if defined(CONFIG_AST1500_ANALOG_SINK)
		.chip = xmiter_internal_dac,
		.cap = xCap_RGB,
		.exist = NULL,
		.setup = xmiter_setup_dac,
		.setup_audio = NULL,
		.disable = xmiter_disable_dac,
		.disable_audio = NULL,
		.init = xmiter_init_dac,
		.check_hotplug = NULL, //Asume analog interface is always plugged //_check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif

#else
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#endif
	},
	{ //Client CRT2
#if defined(CONFIG_AST1500_DIGITAL_SINK)
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#else
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#endif
	},
#elif defined(CONFIG_ARCH_AST1500_HOST)
	{ //Host loopback digital
#if defined(CONFIG_AST1500_DIGITAL_SOURCE) && defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#else /* #if defined(CONFIG_AST1500_DIGITAL_SOURCE) && defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK) */
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif
#endif /* #if defined(CONFIG_AST1500_DIGITAL_SOURCE) && defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK) */
	},
	{ //Host loopback analog
#if defined(CONFIG_AST1500_ANALOG_SOURCE) && defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK)
		.chip = xmiter_dummy,
		.cap = xCap_RGB,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL, //Asume analog interface is always plugged //_check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
#if SUPPORT_HDCP_REPEATER
		.Hdcp1_set_mode = NULL,
		.Hdcp1_auth = NULL,
		.Hdcp1_encrypt = NULL,
#endif

#else
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
#endif
	},
#else
#endif
};
#elif (BOARD_DESIGN_VER_VIDEO == 104)
//#elif defined(CONFIG_AST1500_BOARD_V4) || defined(CONFIG_AST1500_BOARD_1510FPGA)
#include "xmiter_dac.c"

struct s_xmiter_info xmiter_info[NUM_XMITER] =
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	{ //Client CRT1
		.chip = xmiter_internal_dac,
		.cap = xCap_RGB,
		.exist = NULL,
		.setup = xmiter_setup_dac,
		.setup_audio = NULL,
		.disable = xmiter_disable_dac,
		.disable_audio = NULL,
		.init = xmiter_init_dac,
		.check_hotplug = _check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
	},
	{ //Client CRT2
		.chip = xmiter_cat6613,
		.cap = (xCap_DVI | xCap_HDMI),
		.exist = cat6613_exist,
		.setup = cat6613_setup_video,
		.setup_audio = cat6613_setup_audio,
		.disable = cat6613_disable,
		.disable_audio = cat6613_disable_audio,
		.init = NULL,
		.check_hotplug = _check_hotplug_gpio,
		.rd_edid_chksum = cat6613_rd_edid_chksum,
		.rd_edid = cat6613_rd_edid,
		.poll_hotplug = NULL,
	},
#elif defined(CONFIG_ARCH_AST1500_HOST)
	{ //Host loopback digital
		.chip = xmiter_cat6613,
		.cap = (xCap_DVI | xCap_HDMI),
		.exist = cat6613_exist,
		.setup = cat6613_setup_video,
		.setup_audio = cat6613_setup_audio,
		.disable = cat6613_disable,
		.disable_audio = cat6613_disable_audio,
		.init = NULL,
		.check_hotplug = _check_hotplug_gpio,
		.rd_edid_chksum = cat6613_rd_edid_chksum,
		.rd_edid = cat6613_rd_edid,
		.poll_hotplug = NULL,
	},
	{ //Host loopback analog
		.chip = xmiter_dummy,
		.cap = xCap_RGB,
		.exist = cat6613_exist,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = cat6613_rd_edid_chksum,
		.rd_edid = cat6613_rd_edid,
		.poll_hotplug = _poll_hotplug,
	},
#else
#endif
};
#elif (BOARD_DESIGN_VER_VIDEO == 103)
//#elif defined(CONFIG_AST1500_BOARD_V3)
#include "xmiter_dac.c"

struct s_xmiter_info xmiter_info[NUM_XMITER] =
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	{ //Client CRT1
		.chip = xmiter_internal_dac,
		.cap = xCap_RGB,
		.exist = NULL,
		.setup = xmiter_setup_dac,
		.setup_audio = NULL,
		.disable = xmiter_disable_dac,
		.disable_audio = NULL,
		.init = xmiter_init_dac,
		.check_hotplug = _check_hotplug_edid,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
	},
	{ //Client CRT2
		.chip = xmiter_siI164,
		.cap = (xCap_DVI),
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = _check_hotplug_gpio,
		.rd_edid_chksum = _rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_i2c,
		.poll_hotplug = _poll_hotplug,
	},
#elif defined(CONFIG_ARCH_AST1500_HOST)
	{ //Host loopback digital
		.chip = xmiter_none,
		.cap = (xCap_NONE),
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
	},
	{ //Host loopback analog
		.chip = xmiter_dummy,
		.cap = xCap_RGB,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
	},
#else
#endif
};

#elif defined(CONFIG_ARCH_FGA1500)
#include "xmiter_ch7301.c"

struct s_xmiter_info xmiter_info[NUM_XMITER] =
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	{ //Client CRT1
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
	},
	{ //Client CRT2
		.chip = xmiter_ch7301,
		.cap = (xCap_DVI | xCap_RGB),
		.exist = NULL,
		.setup = xmiter_setup_ch7301,
		.setup_audio = NULL,
		.disable = xmiter_disable_ch7301,
		.disable_audio = NULL,
		.init = xmiter_init_ch7301,
		.check_hotplug = xmiter_check_hotplug_ch7301, //_check_hotplug_gpio,
#if 0//When these function pointers are NULLs, default EDID will be used.
		.rd_edid_chksum = _rd_edid_chksum_default,//_rd_edid_chksum_i2c,
		.rd_edid = _rd_edid_default, //_rd_edid_i2c,
#else
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
#endif
		.poll_hotplug = _poll_hotplug,
	},
#elif defined(CONFIG_ARCH_AST1500_HOST)
	{ //Host loopback digital
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
	},
	{ //Host loopback analog
		.chip = xmiter_none,
		.cap = xCap_NONE,
		.exist = NULL,
		.setup = NULL,
		.setup_audio = NULL,
		.disable = NULL,
		.disable_audio = NULL,
		.init = NULL,
		.check_hotplug = NULL,
		.rd_edid_chksum = NULL,
		.rd_edid = NULL,
		.poll_hotplug = NULL,
	},
#else
#endif
};
#else
#error "Board type undefined?!"
#endif

unsigned int _get_xmiter_idx(struct s_crt_drv *crt)
//return index into xmiter_info
{
#if 0
	if (!crt->is_client)
	{
		switch (crt->disp_select)
		{
		case OUTPUT_SELECT_DIGITAL:
			return XIDX_HOST_D;
		case OUTPUT_SELECT_ANALOG:
			return XIDX_HOST_A;
		default:
			uerr("Unsupported xmiter select?!\n");
			break;
		}
		BUG();
		return -ENODEV;
	}

	/* Current CRT driver design assumes Digital use CRT2, Analog use CRT1. */
	switch (crt->disp_select)
	{
	case OUTPUT_SELECT_DIGITAL:
#if 0
		return XIDX_CLIENT_CRT2;
#else
		return XIDX_CLIENT_D;
#endif
		break;
	case OUTPUT_SELECT_ANALOG:
#if 0
		return XIDX_CLIENT_CRT1;
#else
		return XIDX_CLIENT_A;
#endif
		break;
	default:
		uerr("Unsupported xmiter select?!\n");
		break;
	}
	BUG();
	return -ENODEV;
#else
#if 0
	return crt->disp_select;
#else
	return crt->tx_select;
#endif
#endif
}

static inline struct s_xmiter_info *primary_xmiter_info(struct s_crt_drv *crt)
{
	return xmiter_info + _get_xmiter_idx(crt);
}

static inline struct s_xmiter_info *secondary_xmiter_info(struct s_crt_drv *crt)
{
	if (crt->dual_port & crt->dual_port_exist)
		return xmiter_info + ((_get_xmiter_idx(crt) + 1) % NUM_XMITER);

	return NULL;
}

static inline struct s_xmiter_info *logical_xmiter_info(struct s_crt_drv *crt, u32 index)
{
	struct s_xmiter_info *xmiter = NULL;

	switch (index) {
	case 0:
		xmiter = primary_xmiter_info(crt);
		break;
#if (3 <= CONFIG_AST1500_SOC_VER)
	/* FIXME only 1520 support dual-port? */
	case 1:
		xmiter = secondary_xmiter_info(crt);
		break;
#endif
	default:
		break;
	}

	return xmiter;
}

static inline u32 downstream_attached(struct s_crt_drv *crt, struct s_xmiter_info *xmiter)
{
	if (xmiter->check_hotplug)
		return xmiter->check_hotplug(crt);

	return 1;
}

unsigned int xHal_xmiter_exist(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	if (xmiter_info[idx].exist)
		return xmiter_info[idx].exist(crt);
	else
		return 0;
}
#if 0 //meanless
unsigned int xHal_xmiter_dual_port_exist(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	if (crt->dual_port) {
		unsigned int i = (idx + 1) % NUM_XMITER;
		if (xmiter_info[i].exist)
			return xmiter_info[i].exist(crt);
		else
			return 0;
	}

	return 0;
}
#endif
unsigned int xHal_poll_hotplug(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	if (xmiter_info[idx].poll_hotplug)
		return xmiter_info[idx].poll_hotplug(crt);
#if 0
	else if (!crt->display_attached) {
		/*
		** If current xmiter doesn't support poll_hotplug() and is_detached,
		** we need to force polling in case user changes to another display.
		*/
		queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, 2 * HZ);
	}
#endif
	return 0;
}

unsigned int xHal_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	unsigned idx = _get_xmiter_idx(crt);

#if 0
	if (!crt->tx_exist)
		return 1;
#endif

	if (xmiter_info[idx].rd_edid) {
		if (!xmiter_info[idx].rd_edid(crt, blk_num, pEdid)) {
			//Read EDID OK.
			return 0;
		}
		uerr("Failed to read EDID.\n");
	}
#if 1
	else
		uerr("no rd_edid registered on this Tx!!!\n");
#endif

#if 0//tell crt_get_display_edid to use default EDID by returning a nonzero value
	uerr("Use the default EDID value.\n");
	if (xmiter_info[idx].cap == xCap_RGB)
		memcpy(pEdid, AST_EDID_VGA + (EDID_BLOCK_SIZE * blk_num), EDID_BLOCK_SIZE);
	else
		memcpy(pEdid, AST_EDID_HDMI + (EDID_BLOCK_SIZE * blk_num), EDID_BLOCK_SIZE);
	return 0;
#else
	return 1;
#endif
}


static unsigned int _xHal_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum)
{
	unsigned idx = _get_xmiter_idx(crt);

	if (xmiter_info[idx].rd_edid_chksum)
		return xmiter_info[idx].rd_edid_chksum(crt, pChksum);
#if 1
	uerr("no rd_edid_chksum registered on this Tx!!!\n");
#endif

	return -ENODEV;
}


unsigned int xHal_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum)
{
#if 0
	unsigned idx = _get_xmiter_idx(crt);
#endif
#if 0
	if (!crt->tx_exist)
		return 1;
#endif

	if (!_xHal_rd_edid_chksum(crt, pChksum)) {
		//Success
		return 0;
	}

#if 0//tell crt_get_display_edid to use checksum from the default EDID by returning a nonzero value
	uerr("Failed to read edid checksum. Use the default value.\n");
	if (xmiter_info[idx].cap == xCap_RGB)
		*pChksum = AST_EDID_VGA[EDID_CHECKSUM_OFFSET];
	else
		*pChksum = AST_EDID_HDMI[EDID_CHECKSUM_OFFSET];
	return 0;
#else
	//Fail
	return 1;
#endif
}


/*
** Return a bit-field value for the status of each transmitter:
** bit[0]: primary port
** bit[1]: secondary port
** 1: plugged
** 0: unplugged
**/
unsigned int xHal_check_hotplug(struct s_crt_drv *crt)
{
	struct s_xmiter_info *xmiter;
	u32 ret = 0;
	int i;

	if (!crt->tx_exist)
		return 0;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (!xmiter)
			continue;

		if (xmiter->check_hotplug) {
			if (xmiter->check_hotplug(crt))
				ret |= (1 << i);
		} else {
			ret |= (1 << i);
		}
	}

	return ret;
}


unsigned int xHal_init_xmiter(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	/* Return unplugged if this port's xmiter is not available. */
#if 0
	if (xmiter_info[idx].chip == xmiter_none)
#else
	if (!crt->tx_exist)
#endif
		return 0;

	if (xmiter_info[idx].init)
		return xmiter_info[idx].init(crt);

	return 0;
}

unsigned int xHal_init_dual_port_xmiter(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	unsigned int i = (idx + 1) % NUM_XMITER;
	if (crt->dual_port_exist) {
		if (xmiter_info[i].init)
			xmiter_info[i].init(crt);
	}

	return 0;
}

unsigned int xHal_get_xmiter_cap(struct s_crt_drv *crt)
{
	unsigned idx = _get_xmiter_idx(crt);

	return xmiter_info[idx].cap;
}

#if (3 <= CONFIG_AST1500_SOC_VER)
unsigned int xHal_get_xmiter_cap_secondary(struct s_crt_drv *crt)
{
	struct s_xmiter_info *xmiter = secondary_xmiter_info(crt);

	if (xmiter)
		return xmiter->cap;

	return xCap_NONE;
}
#endif

/*
 * Following xHal functions control both xmiter info if dual-port exists
*/
unsigned int xHal_disable_xmiter(struct s_crt_drv *crt)
{
	struct s_xmiter_info *xmiter;
	int i;

	if (!crt->tx_exist)
		return 0;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (xmiter->disable)
				xmiter->disable(crt);
		}
	}
	return 0;
}

unsigned int xHal_setup_xmiter(struct s_crt_drv *crt, struct s_crt_info *info)
{
	struct s_xmiter_info *xmiter;
	int i;

	if (!crt->tx_exist)
		return 0;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter == NULL)
			continue;

		if (crt->current_owner == OWNER_HOST) {
			/* host loopback, set digital interface only when video input is digital */
			if ((info->HDMI_Mode == s_RGB)
				&& (xmiter->cap & (xCap_DVI | xCap_HDMI))) {
				/*
				 * input is analog and (HDMI_Mode == s_RGB)
				 * xmiter is digital (xCap_DVI or xCap_HDMI)
				 * => disable
				 */
				if (xmiter->disable)
					xmiter->disable(crt);
				continue;
			}
		}

		if (xmiter->setup) {
			if ((info->flag & HDMI_INFOFRAME_ONLY) && (xmiter->cap == xCap_RGB))
				continue;

			xmiter->setup(crt, info);
		}
	}

	if (info->flag & HDMI_INFOFRAME_ONLY) {
		/* just HDR infoframe changed, skip HDCP configuration */
		goto exit;
	}

	/* CRT call HDCP right after xmiter setup. */
	xHal_hdcp1_auth(crt, info->EnHDCP);
exit:
	return 0;
}

unsigned int xHal_setup_audio_xmiter(struct s_crt_drv *crt)
{
	struct s_xmiter_info *xmiter;
	int i;

	if (!crt->tx_exist)
		return 0;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (xmiter->setup_audio)
				xmiter->setup_audio(&crt->audio_info);
		}
	}

	return 0;
}

unsigned int xHal_xmiter_disable_audio(struct s_crt_drv *crt)
{
	struct s_xmiter_info *xmiter;
	int i;

	if (!crt->tx_exist)
		return 0;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (xmiter->disable_audio)
				xmiter->disable_audio();
		}
	}

	return 0;
}

#if SUPPORT_HDCP_REPEATER
void xHal_hdcp1_set_mode(struct s_crt_drv *crt, u32 mode)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (!downstream_attached(crt, xmiter))
				continue;

			if (xmiter->Hdcp1_set_mode)
				xmiter->Hdcp1_set_mode(mode);
		}
	}
}

void xHal_hdcp1_auth(struct s_crt_drv *crt, u32 enable)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (!downstream_attached(crt, xmiter))
				continue;

			if (xmiter->Hdcp1_auth)
				xmiter->Hdcp1_auth(enable);
		}
	}
}

void xHal_hdcp1_encrypt(struct s_crt_drv *crt, u32 enable)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (!downstream_attached(crt, xmiter))
				continue;

			if (xmiter->Hdcp1_encrypt)
				xmiter->Hdcp1_encrypt(enable);
		}
	}
}
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT)
void xHal_cec_send(struct s_crt_drv *crt, u8 *buf, u32 size)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			/*
			 * DO NOT check attached status before sending
			 *
			 * some TVs put HPD at low when they are in 'Standby'
			 * we have to ignore HPD status if we want to wakeup these kind of devices
			 */
#if 0
			if (!downstream_attached(crt, xmiter))
				continue;
#endif

			if (xmiter->cec_send)
				xmiter->cec_send(buf, size);
		}
	}
}

int xHal_cec_topology(struct s_crt_drv *crt, u8 *buf, u32 scan)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			/*
			 * skip attached check
			 * have to clear topology status if downstream is not attached
			 */
#if 0
			if (!downstream_attached(crt, xmiter))
				continue;
#endif

			if (xmiter->cec_topology)
				return xmiter->cec_topology(buf, scan);
		}
	}

	return -1;
}

void xHal_cec_pa_cfg(struct s_crt_drv *crt, u16 address)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (!downstream_attached(crt, xmiter))
				continue;

			if (xmiter->cec_pa_cfg)
				xmiter->cec_pa_cfg(address);
		}
	}
}

void xHal_cec_la_cfg(struct s_crt_drv *crt, u8 address)
{
	struct s_xmiter_info *xmiter;
	int i;

	for (i = 0; i < NUM_XMITER; i++) {
		xmiter = logical_xmiter_info(crt, i);
		if (xmiter) {
			if (!downstream_attached(crt, xmiter))
				continue;

			if (xmiter->cec_la_cfg)
				xmiter->cec_la_cfg(address);
		}
	}
}

#include <asm/arch/drivers/video_hal.h>

static PFN_CECIF_RX _cecif_rx;

/*
 * cec driver register/deregister callback function for transmitter
 */
void reg_cecif_rx_callback(PFN_CECIF_RX cb)
{
	_cecif_rx = cb;
}
EXPORT_SYMBOL(reg_cecif_rx_callback);

void dereg_cecif_rx_callback(void)
{
	_cecif_rx = NULL;
}
EXPORT_SYMBOL(dereg_cecif_rx_callback);

void cecif_rx(unsigned char *buf, unsigned int size)
{
	if (_cecif_rx)
		_cecif_rx(buf, size);
}
EXPORT_SYMBOL(cecif_rx);
#endif
