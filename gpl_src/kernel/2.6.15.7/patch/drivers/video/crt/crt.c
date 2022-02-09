#include <aspeed/features.h>
#include <asm/arch/drivers/util.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <asm/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/arch/drivers/I2C.h>

#include <asm/arch/drivers/crt.h>
#define VESA_GLOBALS
#include <asm/arch/drivers/vesa.h>
#include "astdebug.h"
#include <asm/arch/drivers/vbuf.h>
#include <asm/arch/regs-scu.h>
#include "osddata.h"
#include "logo.h"
#include "cursor.h"

#include <asm/arch/ast-scu.h>

static PFN_CRT_Tx_Hotplug_Callback tx_hotplug_callback = NULL;
#if defined(CONFIG_ARCH_AST1500_CLIENT)
static PFN_CRT_Tx_Hotplug_Callback tx_hotplug_cec_callback = NULL;
#endif
#if 0
ULONG AST3000DCLKTable [] = {
    0x00046515,     /* 00: VCLK25_175 */
	0x0008482a,		/* 01: VCLK31_574 */
    0x00047255,     /* 02: VCLK28_322 */
    0x0004682a,     /* 03: VCLK31_5         */
	0x00084b3c,		/* 04: VCLK32_76 */
	0x00084b3d,		/* 05: VCLK33_154 */
    0x0004672a,     /* 06: VCLK36          */
    0x00046c50,     /* 07: VCLK40           */
	0x00084945,		/* 08: VCLK45_978   */
    0x00046842,     /* 09: VCLK49_5         */
    0x00006c32,     /* 0A: VCLK50           */
	0x00006c35,		/* 0B: VCLK52_95 */
    0x00006a2f,     /* 0C: VCLK56_25        */
    0x00006c41,     /* 0D: VCLK65  */
	0x00006a3e,		/* 0E: VCLK74_48 */
    0x00006832,     /* 0F: VCLK75         */
    0x0000672e,     /* 10: VCLK78_75        */
	0x00006a42,		/* 11: VCLK79_373 */
	0x00006a44,		/* 12: VCLK81_624 */
	0x00006731,		/* 13: VCLK83_462 */
	0x00006c55,		/* 14: VCLK84_715 */
    0x0000683f,     /* 15: VCLK94_5         */
	0x00004928,		/* 16: VCLK106_5 = 0x0C		*/
    0x00004824,     /* 17: VCLK108          */
	0x00004723,		/* 18: VCLK119			*/
    0x0000482d,     /* 19: VCLK135          */
	0x00004a39,		/* 1A: VCLK136_358 */
	0x00006968,		/* 1B: VCLK138_5 */
	0x00004B37,		/* 1C: VCLK146_25		*/
	0x00006b88,     /* 1D: VCLK148_5        */
	0x00004C4D,	    /* 1E: VCLK154   		*/
    0x0000472e,     /* 1F: VCLK157_5        */
    0x00004836,     /* 20: VCLK162          */
};
#endif
#if 0
_OSDPalette OSDPallette[16] = {
	{0x1, 0x00, 0x00, 0x00}, //Black
	{0x1, 0x80, 0x00, 0x00}, //DarkRed
	{0x1, 0x00, 0x80, 0x00}, //DarkGreen
	{0x1, 0x80, 0x80, 0x00}, //DarkYellow
	{0x1, 0x00, 0x00, 0x80}, //DarkBlue
	{0x1, 0x80, 0x00, 0x80}, //DarkPurple
	{0x1, 0x00, 0x80, 0x80}, //DarkCyan
	{0x1, 0xC0, 0xC0, 0xC0}, //PaleGray
	{0x0, 0x80, 0x80, 0x80}, //MidGray //Transparent
	{0x1, 0xFF, 0x00, 0x00}, //Red
	{0x1, 0x00, 0xFF, 0x00}, //Green
	{0x1, 0xFF, 0xFF, 0x00}, //Yellow
	{0x1, 0x00, 0x00, 0xFF}, //Blue
	{0x1, 0xFF, 0x00, 0xFF}, //Magenta
	{0x1, 0x00, 0xFF, 0xFF}, //Cyan
	{0x1, 0xFF, 0xFF, 0xFF}, //White
};
#endif
#if 1
unsigned char AST_EDID_VGA[EDID_ROM_SIZE] =
{
#if (CONFIG_AST1500_SOC_VER >= 3) /* Use 1920x1080 as default */
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,
0x06, 0x19, 0x01, 0x03, 0x08, 0x2f, 0x1a, 0x78, 0x2a, 0x35, 0x85, 0xa6, 0x56, 0x48, 0x9a, 0x24,
0x12, 0x50, 0x54, 0xa5, 0x69, 0x00, 0x71, 0x00, 0x81, 0x00, 0x81, 0x40, 0xd1, 0xc0, 0x81, 0x80,
0x95, 0x00, 0xb3, 0x00, 0x01, 0x00, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
0x45, 0x00, 0xdc, 0x0c, 0x11, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x32, 0x0a, 0x0a,
0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x38,
0x4c, 0x1e, 0x53, 0x11, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
0x00, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x56, 0x47, 0x41, 0x0a, 0x20, 0x00, 0xf0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#else
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00,
0x06, 0x15, 0x01, 0x03, 0x0e, 0x2f, 0x1a, 0x78, 0x2e, 0x35, 0x85, 0xa6, 0x56, 0x48, 0x9a, 0x24,
0x12, 0x50, 0x54, 0xa5, 0x6b, 0xba, 0x71, 0x00, 0x81, 0x00, 0x81, 0x40, 0x95, 0x0f, 0x81, 0x80,
0x95, 0x00, 0xb3, 0x00, 0x01, 0x01, 0x64, 0x19, 0x00, 0x40, 0x41, 0x00, 0x26, 0x30, 0x18, 0x88,
0x36, 0x00, 0xdc, 0x0c, 0x11, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0xff, 0x00, 0x32, 0x0a, 0x0a,
0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x32,
0x4c, 0x1e, 0x5e, 0x15, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
0x00, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x56, 0x47, 0x41, 0x0a, 0x20, 0x00, 0xa0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
#endif
};
EXPORT_SYMBOL(AST_EDID_VGA);

unsigned char AST_EDID_DVI[EDID_ROM_SIZE] =
{
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x00, 0x15, 0x01, 0x00, 0x00, 0x00,
0x06, 0x15, 0x01, 0x03, 0x80, 0x2f, 0x1a, 0x78, 0x2e, 0x35, 0x85, 0xa6, 0x56, 0x48, 0x9a, 0x24,
0x12, 0x50, 0x54, 0xa5, 0x6b, 0xba, 0x71, 0x00, 0x81, 0x40, 0x81, 0x00, 0x95, 0x0f, 0x81, 0x80,
0x95, 0x00, 0xb3, 0x00, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
0x45, 0x00, 0xdd, 0x0c, 0x11, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x30, 0x0a, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x32,
0x4c, 0x1e, 0x5e, 0x15, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
0x00, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x44, 0x56, 0x49, 0x0a, 0x20, 0x00, 0xa8,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
EXPORT_SYMBOL(AST_EDID_DVI);

unsigned char AST_EDID_HDMI[EDID_ROM_SIZE] =
{
#if (CONFIG_AST1500_SOC_VER >= 2)
/* Add multi-ch audio support. */
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x01, 0x00, 0x02, 0x00, 0x00, 0x00,
0x05, 0x19, 0x01, 0x03, 0x80, 0x34, 0x1d, 0x78, 0x2a, 0xc7, 0x20, 0xa4, 0x55, 0x49, 0x99, 0x27,
0x13, 0x50, 0x54, 0xbf, 0xef, 0x00, 0x71, 0x40, 0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xb3, 0x00,
0xd1, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
0x45, 0x00, 0x09, 0x25, 0x21, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x31, 0x0a, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x37,
0x4b, 0x1e, 0x55, 0x10, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
0x00, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x48, 0x44, 0x4d, 0x49, 0x0a, 0x01, 0x88,
0x02, 0x03, 0x34, 0xc1, 0x4d, 0x01, 0x02, 0x03, 0x11, 0x12, 0x13, 0x04, 0x90, 0x1f, 0x0e, 0x0f,
0x1d, 0x1e, 0x35, 0x09, 0x7f, 0x04, 0x0f, 0x7f, 0x04, 0x15, 0x07, 0x50, 0x3d, 0x1f, 0xc0, 0x5f,
0x54, 0x01, 0x57, 0x06, 0x00, 0x67, 0x54, 0x00, 0x83, 0x5f, 0x00, 0x00, 0x67, 0x03, 0x0c, 0x00,
0x10, 0x00, 0x80, 0x21, 0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00,
0x09, 0x25, 0x21, 0x00, 0x00, 0x18, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28,
0x55, 0x00, 0x09, 0x25, 0x21, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20,
0xb8, 0x28, 0x55, 0x40, 0x09, 0x25, 0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40,
0x31, 0x20, 0x0c, 0x40, 0x55, 0x00, 0x09, 0x25, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x38,
#else
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x00, 0x15, 0x02, 0x00, 0x00, 0x00,
0x11, 0x15, 0x01, 0x03, 0x80, 0x34, 0x1d, 0x78, 0x2a, 0xc7, 0x20, 0xa4, 0x55, 0x49, 0x99, 0x27,
0x13, 0x50, 0x54, 0xbf, 0xef, 0x00, 0x71, 0x4f, 0x81, 0x40, 0x81, 0x80, 0x95, 0x00, 0xb3, 0x00,
0xd1, 0xc0, 0x01, 0x01, 0x01, 0x01, 0x02, 0x3a, 0x80, 0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c,
0x45, 0x00, 0x09, 0x25, 0x21, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xff, 0x00, 0x31, 0x0a, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x37,
0x4b, 0x1e, 0x55, 0x10, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc,
0x00, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x48, 0x44, 0x4d, 0x49, 0x0a, 0x01, 0x5d,
0x02, 0x03, 0x20, 0x41, 0x4d, 0x01, 0x02, 0x03, 0x11, 0x12, 0x13, 0x04, 0x90, 0x1f, 0x0e, 0x0f,
0x1d, 0x1e, 0x23, 0x09, 0x07, 0x07, 0x83, 0x01, 0x00, 0x00, 0x65, 0x03, 0x0c, 0x00, 0x10, 0x00,
0x8c, 0x0a, 0xd0, 0x8a, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x3e, 0x96, 0x00, 0x09, 0x25, 0x21, 0x00,
0x00, 0x18, 0x01, 0x1d, 0x00, 0x72, 0x51, 0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0x09, 0x25,
0x21, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x00, 0xbc, 0x52, 0xd0, 0x1e, 0x20, 0xb8, 0x28, 0x55, 0x40,
0x09, 0x25, 0x21, 0x00, 0x00, 0x1e, 0x8c, 0x0a, 0xd0, 0x90, 0x20, 0x40, 0x31, 0x20, 0x0c, 0x40,
0x55, 0x00, 0x09, 0x25, 0x21, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3a,
#endif
};
EXPORT_SYMBOL(AST_EDID_HDMI);

unsigned char AST_EDID_HDMI_4K[EDID_ROM_SIZE] =
{
#if 1
/* HDMI 1.4b, 4K, Multi-Ch Audio */
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x59, 0x24, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
0x05, 0x19, 0x01, 0x03, 0x80, 0x3d, 0x23, 0x78, 0x2a, 0x5f, 0xb1, 0xa2, 0x57, 0x4f, 0xa2, 0x28,
0x0f, 0x50, 0x54, 0xbf, 0xef, 0x80, 0x71, 0x40, 0x81, 0x00, 0x81, 0xc0, 0x81, 0x80, 0x95, 0x00,
0xa9, 0xc0, 0xb3, 0x00, 0xd1, 0x00, 0x04, 0x74, 0x00, 0x30, 0xf2, 0x70, 0x5a, 0x80, 0xb0, 0x58,
0x8a, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0xfd, 0x00, 0x18, 0x4b, 0x1e,
0x5a, 0x1e, 0x00, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x47,
0x65, 0x6e, 0x65, 0x72, 0x69, 0x63, 0x5f, 0x34, 0x4b, 0x0a, 0x20, 0x20, 0x00, 0x00, 0x00, 0xff,
0x00, 0x30, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x3f,
0x02, 0x03, 0x39, 0xc3, 0x4b, 0x90, 0x04, 0x1f, 0x13, 0x03, 0x12, 0x20, 0x0f, 0x1e, 0x24, 0x26,
0x35, 0x09, 0x7f, 0x04, 0x0f, 0x7f, 0x04, 0x15, 0x07, 0x50, 0x3d, 0x1f, 0xc0, 0x5f, 0x54, 0x01,
0x57, 0x06, 0x00, 0x67, 0x54, 0x00, 0x83, 0x5f, 0x00, 0x00, 0x6e, 0x03, 0x0c, 0x00, 0x10, 0x00,
0x80, 0x3c, 0x20, 0x10, 0x80, 0x01, 0x02, 0x03, 0x04, 0x02, 0x3a, 0x80, 0xd0, 0x72, 0x38, 0x2d,
0x40, 0x10, 0x2c, 0x45, 0x80, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1e, 0x01, 0x1d, 0x00, 0x72, 0x51,
0xd0, 0x1e, 0x20, 0x6e, 0x28, 0x55, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1e, 0x02, 0x3a, 0x80,
0x18, 0x71, 0x38, 0x2d, 0x40, 0x58, 0x2c, 0x45, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1e, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9d,
#else
/* HDMI 1.4b, 4K */
0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x74, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
0x05, 0x19, 0x01, 0x03, 0x80, 0x3D, 0x23, 0x78, 0x2A, 0x5F, 0xB1, 0xA2, 0x57, 0x4F, 0xA2, 0x28,
0x0F, 0x50, 0x54, 0xBF, 0xEF, 0x80, 0x71, 0x4F, 0x81, 0x00, 0x81, 0xC0, 0x81, 0x80, 0x95, 0x00,
0xA9, 0xC0, 0xB3, 0x00, 0xD1, 0x00, 0x04, 0x74, 0x00, 0x30, 0xF2, 0x70, 0x5A, 0x80, 0xB0, 0x58,
0x8A, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x18, 0x4B, 0x1E,
0x5A, 0x1E, 0x00, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x54,
0x53, 0x54, 0x4D, 0x30, 0x30, 0x30, 0x31, 0x0A, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00, 0xFF,
0x00, 0x54, 0x53, 0x54, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x0A, 0x20, 0x20, 0x20, 0x01, 0x9B,
0x02, 0x03, 0x23, 0xF1, 0x48, 0x90, 0x04, 0x1F, 0x13, 0x03, 0x12, 0x20, 0x22, 0x23, 0x09, 0x07,
0x07, 0x83, 0x01, 0x00, 0x00, 0x6D, 0x03, 0x0C, 0x00, 0x20, 0x00, 0x80, 0x3C, 0x20, 0x10, 0x60,
0x01, 0x02, 0x03, 0x02, 0x3A, 0x80, 0x18, 0x71, 0x38, 0x2D, 0x40, 0x58, 0x2C, 0x45, 0x00, 0x60,
0x59, 0x21, 0x00, 0x00, 0x1E, 0x02, 0x3A, 0x80, 0xD0, 0x72, 0x38, 0x2D, 0x40, 0x10, 0x2C, 0x45,
0x80, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1E, 0x01, 0x1D, 0x00, 0x72, 0x51, 0xD0, 0x1E, 0x20, 0x6E,
0x28, 0x55, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1E, 0x56, 0x5E, 0x00, 0xA0, 0xA0, 0xA0, 0x29,
0x50, 0x30, 0x20, 0x35, 0x00, 0x60, 0x59, 0x21, 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37,
#endif
};
EXPORT_SYMBOL(AST_EDID_HDMI_4K);
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
#define DRAM_FETCH_UNIT (128) //128bits per fetch
#define DRAM_FETCH_SHIFT (7)
#else
#define DRAM_FETCH_UNIT (64) //64bits per fetch
#define DRAM_FETCH_SHIFT (6)
#endif

#define CALC_TERMINAL_CNT(pixels, bpp) (((pixels) * (bpp) + (DRAM_FETCH_UNIT - 1)) >> DRAM_FETCH_SHIFT)


static u32 crt_setup(struct s_crt_drv *crt, unsigned int owner);
void EnableOSD(int nCRTIndex, int inbEnOSD);
static u32 setup_video(struct s_crt_drv *crt, struct s_crt_info *i);
void _crt_to_screen(struct s_crt_drv *crt, unsigned int screen);
static void crt_update_terminal_cnt(struct s_crt_drv *crt, struct s_crt_info *i, unsigned x_scale, unsigned int x_rem_pixel, unsigned int has_downscale);
static u32 timing_sn_convert(struct timing_convert *t, u32 sn);
static u32 crt_timing_index_to_preferred_timing(u32 index);
void _crt_disable_xmiter(struct s_crt_drv *crt);
void _crt_enable_flip(struct s_crt_drv *crt, void *dummy);
void _crt_disable_flip(struct s_crt_drv *crt, void *dummy);
unsigned int _crt_pwr_save(struct s_crt_drv *crt, int do_pwr_save);

#if (CONFIG_AST1500_SOC_VER >= 2)
static void _crt_enable_desktop(struct s_crt_drv *crt);
#endif

#if (CONFIG_AST1500_SOC_VER != 2)
void _crt_vw_enable(struct s_crt_drv *crt, struct s_crt_info *i);
#endif

static void crt_worker(struct s_crt_work *crt_work)
{
	struct s_crt_drv *crt = crt_work->crt;
	crt_work_func_t f = crt_work->func;
	void *ptr = crt_work->ptr;
	unsigned int free_ptr = crt_work->free_ptr;

	kfree(crt_work);

	BUG_ON(!f);
	BUG_ON(!crt);

	down(&crt->crt_lock);
	f(crt, ptr);
	up(&crt->crt_lock);

	if (ptr && free_ptr)
		kfree(ptr);
}

static struct s_crt_work *_alloc_crt_work(struct s_crt_drv *crt, crt_work_func_t func, void *ptr, unsigned int free_ptr)
{
	struct s_crt_work *crt_work;

	/* Bruce is lazy. Always use GFP_ATOMIC. */
	crt_work = kzalloc(sizeof(struct s_crt_work), GFP_ATOMIC);
	BUG_ON(!crt_work);
	crt_work->crt = crt;
	crt_work->func = func;
	crt_work->ptr = ptr;
	crt_work->free_ptr = free_ptr;
	INIT_WORK(&crt_work->work, (void (*)(void *))crt_worker, crt_work);

	return crt_work;
}
static void queue_crt_work(struct s_crt_drv *crt, crt_work_func_t func, void *ptr, unsigned int free_ptr)
{
	struct s_crt_work *crt_work;

	crt_work = _alloc_crt_work(crt, func, ptr, free_ptr);
	queue_work(crt->xmiter_wq, &crt_work->work);
}
#if 1 /* FIXME. Need maintain a delayed work list in order to be able to 'cancel' it. */
static void queue_crt_delayed_work(struct s_crt_drv *crt, crt_work_func_t func, void *ptr, unsigned int free_ptr, unsigned long msecs)
{
	struct s_crt_work *crt_work;

	crt_work = _alloc_crt_work(crt, func, ptr, free_ptr);
	queue_delayed_work(crt->xmiter_wq, &crt_work->work, msecs_to_jiffies(msecs));
}
#endif
void queue_crt_work_n_wait(struct s_crt_drv *crt, crt_work_func_t func, void *ptr, unsigned int free_ptr)
{
	struct s_crt_work *crt_work;

	crt_work = _alloc_crt_work(crt, func, ptr, free_ptr);
	queue_work(crt->xmiter_wq, &crt_work->work);
	flush_workqueue(crt->xmiter_wq);
}

inline unsigned int __reg_base(struct s_crt_drv *crt)
{
	unsigned int reg_base = 0;
#if (CONFIG_AST1500_SOC_VER >= 2)
	reg_base = 0x60;
#else
	if (crt->disp_select == CRT_1)
		reg_base = 0;
	else
		reg_base = 0x60;
#endif
	return reg_base;
}

/* This function is only used for GUI display setting. */
static int _lookupModeTable(struct s_crt_drv *crt, u16 x, u16 y, u16 rate)
{
	int i, index = -1;
	u32 mt_size = crt->mode_table_size;
	MODE_ITEM *mt = crt->mode_table;

	for (i = 0; i < mt_size; i++){
		if ((mt[i].HActive == x) &&
			(mt[i].VActive == y) &&
			(mt[i].RefreshRate == rate) &&
			(mt[i].ScanMode == Prog)){
			uinfo("mode #%d SN[0x%04X] found\n", i, crt_timing_index_to_sn(i));
			index = i;
			break;
		}
	}
	if (i == mt_size){
		uerr("unsupported mode (HActive = %d, VActive = %d, RefreshRate = %d)\n", x, y, rate);
		return DEFAULT_TIMING_IDX;
	}
	return index;
}

int lookupModeTable(struct s_crt_drv *crt, struct s_crt_info *info)
{
	/* Always use TimingTableIdx for VE and loopback. */
	if (info->owner != OWNER_CON) {
		if (FTCFG_IS_ENABLED(info->force_timing.cfg))
			return info->force_timing.timing_table_idx;
		else
			return info->TimingTableIdx;
	}

	/* Console Screen support 'force_timing'. */
	if (FTCFG_IS_ENABLED(info->force_timing.cfg))
		return info->force_timing.timing_table_idx;

	return _lookupModeTable(
	                        crt,
	                        info->width,
	                        info->height,
	                        info->refresh_rate);
}

#ifdef FPGA_TEST_ONLY

#if defined(CONFIG_AST1520_BOARD_FPGA_ARM11) || defined(CONFIG_AST1520_BOARD_FPGA_ARM9)
void vSetPllFPGA(unsigned int nCRT, u32 ulDCLK)
{
	ast_scu.scu_op(SCUOP_D1PLL_SET, (void*)ulDCLK);
}

#elif defined(CONFIG_AST1510_BOARD_FPGA)
void vSetPllFPGA(unsigned int nCRT, u32 ulDCLK)
{
	ast_scu.scu_op(SCUOP_D1PLL_SET, (void*)ulDCLK);

	msleep(100);
	/* FPGA supports 25.175, 28.322, 40, 65, 15.75, 13.5, 74.25 MHz. */
	if (ulDCLK < 140000) {
		//13.5 480i
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x5);
	}
	else if (ulDCLK < 200000) {
		//15.75
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x4);
	}
	else if (ulDCLK < 280000) {
		/* There is 2300FPGA PLL setting bug which can't configure
		** the default value correctly on power on.
		** To resolve this issue, simply change PLL setting to 1 and
		** back to 0.
		*/
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 1);
		msleep(100);
		//25.175. 640x480@60Hz
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0);
	}
	else if (ulDCLK < 350000) {
		//28.322
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x1);
	}
	else if (ulDCLK < 550000) {
		//40. 800x600@60Hz
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x2);
	}
	else if (ulDCLK < 700000) {
		//65. 1024x768@60Hz
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x3);
	}
	else {
		//74.25. 1080i 60Hz, 1080p 24Hz
		SetSCUReg(CRT1_VIDEO_PLL_REG + nCRT*0x60, 0x6);
	}
	msleep(100);

}
#else

void vSetPllFPGA(unsigned int nCRT, u32 ulDCLK)
{
	u32 ulData, ulMostMin, ulRange;

	/*
	** For CRT1, set SCU08 D[9:8]: 00/01/10/11: 25/28/40/65MHz
	** For CRT2, set SCU08B12, SCU2CB10: 00/01/10/11: 25/28/40/65MHz
	*/
	if (ulDCLK >= 250000)
		ulData = ulDCLK - 250000;
	else
		ulData = 250000 - ulDCLK;

	ulMostMin = ulData;
	ulRange = 0; //25MHz

	if (ulDCLK >= 280000)
		ulData = ulDCLK - 280000;
	else
		ulData = 280000 - ulDCLK;

	if (ulData < ulMostMin){
		ulMostMin = ulData;
		ulRange = 1; //28MHz
	}

	if (ulDCLK >= 400000)
		ulData = ulDCLK - 400000;
	else
		ulData = 400000 - ulDCLK;

	if (ulData < ulMostMin){
		ulMostMin = ulData;
		ulRange = 2; //40MHz
	}

	if (ulDCLK >= 650000)
		ulData = ulDCLK - 650000;
	else
		ulData = 650000 - ulDCLK;

	if (ulData < ulMostMin){
		ulMostMin = ulData;
		ulRange = 3; //65MHz
	}
#if 0 //Doesn't support anymore.
	if (ulDCLK >= 742500)
		ulData = ulDCLK - 742500;
	else
		ulData = 742500 - ulDCLK;

	if (ulData < ulMostMin){
		ulMostMin = ulData;
		ulRange = 3; //74.25MHz
	}
#endif
	switch (nCRT)
	{
	case CRT_1:
		ModSCUReg(SCU_CLOCK_SELECTION_REG, (ulRange<<8), (3UL<<8));
		uinfo("FPGA CRT Clock:0x%08x C(%d),R(%d)\n", GetSCUReg(SCU_CLOCK_SELECTION_REG), ulDCLK, ulRange);
		break;
	case CRT_2:
		ModSCUReg(SCU_MISC_CTRL_REG, ((ulRange&0x1)<<10), (1UL<<10));
		ModSCUReg(SCU_CLOCK_SELECTION_REG, (((ulRange&0x2) >> 1)<<12), (1UL<<12));
		uinfo("FPGA CRT Clk(%d),Rng(%d)\n", ulDCLK, ulRange);
		break;
	}
}
#endif //#ifdef CONFIG_AST1510_BOARD_FPGA

#endif

static void _set_pixel_clock(int nCRTIndex, MODE_ITEM *mode)
{

#if (CONFIG_AST1500_SOC_VER >= 3)
	ast_scu.scu_op(SCUOP_D1PLL_SET, (void*)mode->DCLK10000);

#else //#if (CONFIG_AST1500_SOC_VER >= 3)
	//uinfo("AstPLLIndex [%d] AstPLLCount [%x] \n", (int)AstPLLIndex, AstPLLTable[AstPLLIndex].ulCRT0C);
	SetSCUReg(CRT1_VIDEO_PLL_REG + nCRTIndex*0x60, mode->PixelClock);

#ifdef FPGA_TEST_ONLY
	/*
	** AST3000 FPGA board supports limits of pixel clock setting.
	** The programming method is also different.
	*/
	vSetPllFPGA(nCRTIndex, mode->DCLK10000);
#endif

#endif //#if (CONFIG_AST1500_SOC_VER >= 3)
}

static inline void __mod_reg84(struct s_crt_drv *crt, u32 value, u32 mask)
{
	u32 reg84;
	unsigned long flags;

	/*
	** Bruce160922. Shadow Reg84.
	** Problem:
	** Some CRT registers value won't take effect immediately after write.
	** Most of them updated the value after next v-sync.
	** It will cause trouble for example steps like:
	** 1. write a termical count in Reg84. (readback value won't take effect immediately)
	** 2. read-modify-write display_offset in Reg84:
	** 2.1 read will get 'old' unexpect termical count value.
	** 2.2 the 'old' tc | new display_offset then write back to Reg84.
	** So, the tc value written in first step is gone. ==> BUG.
	** Solution:
	** Use shadow register.
	** TODO:
	** Should apply to all similar CRT registers.
	** Use an array and shadow ALL CRT registers for MOD.
	*/
	spin_lock_irqsave(&crt->crt_reg_lock, flags);
	reg84 = crt->reg84;
	reg84 = (reg84 & (~mask)) | (value & mask);
	SetSCUReg(CRT1_DISPLAY_OFFSET + crt->disp_select*0x60, reg84);
	crt->reg84 = reg84;
	spin_unlock_irqrestore(&crt->crt_reg_lock, flags);
}


static u32 crt2_display_offset_shadow = 0;

static inline void __crt_display_offset_set(struct s_crt_drv *crt, u32 offset, u32 shadow)
{
	/* Caller must hold &crt->crt_lock */
	/*
	 * _crt_display_offset_set() is in protection of crt_lock from _crt_to_screen()
	 *
	 * 1. _chg_fb_src_format <= SetASTModeTimingTest <= ASTSetMode setup_video
	 *	<= crt_setup <= _crt_to_decode_screen/_crt_to_console_screen
	 *	<= _crt_to_screen
	 * 2. crt_panning_to <= _crt_vw_enable <= _crt_change_scale_factor
	 *	<= _crt_to_decode_screen <=_crt_to_screen
	 * 3. crt_panning_to <= _crt_vw_enable <= crt_vw_enable
	 */
	if (shadow)
		crt2_display_offset_shadow = offset;

	if (crt->op_state & COPS_VE_YUV_DESKTOP_OFF)
		goto exit;

	__mod_reg84(crt, offset, DISPLAY_OFFSET_MASK);
exit:
	return;
}

static inline void _crt_display_offset_set(struct s_crt_drv *crt, u32 offset)
{
	__crt_display_offset_set(crt, offset, 1);
}

static inline void _crt_display_offset_clear(struct s_crt_drv *crt)
{
	__crt_display_offset_set(crt, 0, 0);
}

static inline void _crt_display_offset_restore(struct s_crt_drv *crt)
{
	/* Caller must hold &crt->crt_lock */
	__mod_reg84(crt, crt2_display_offset_shadow, DISPLAY_OFFSET_MASK);
}

/*
** _chg_fb_src_format()
** This function runs the procedure needed when updating frame buffer source
** format without changing CRT output timing.
** For example, display 720p decompressed source to 1080p CRT timing.
*/
static int _chg_fb_src_format(struct s_crt_drv *crt,
                            int nCRTIndex,
                            u16 HActive,
                            eVIDEO_FORMAT ColorFormat,
                            struct s_crt_info *crt_info)
{
	u32 DisplayOffset, TerminalCount, bpp;

	switch (ColorFormat) {
	case YUV444_FORMAT:
		bpp = 32;
		break;
	case XRGB8888_FORMAT:
		bpp = 32;
		break;
	case RGB565_FORMAT:
		bpp = 16;
		break;
#if (CONFIG_AST1500_SOC_VER >= 2)
	case YUV420_FORMAT:
		bpp = 32; //This mode is used by VE only. And VE always use 32bpp as line offset.
		break;
#endif
	default:
		uerr("set_fb_source_info: unsupport format\n");
		return -1;
	}

	if (crt_info->owner == OWNER_CON) {
		DisplayOffset = (HActive) * bpp / 8;
	} else {
		DisplayOffset = crt_info->line_offset * bpp /8;
	}

	/*
	** IMPORTANT! YUV444_FORMAT actually place XYUV 32bits in memory,
	** but both VE and CRT just use 24bits. CRT terminal count is based
	** on 24bits counter, but line offset is based on 32bits memory offset.
	**
	** Terminal count must be multiple of 64bits.
	*/
	switch (ColorFormat) {
	case YUV444_FORMAT:
		bpp = 24; //Not 32!!
		break;
	case XRGB8888_FORMAT:
		bpp = 32;
		break;
	case RGB565_FORMAT:
		bpp = 16;
		break;
#if (CONFIG_AST1500_SOC_VER >= 2)
	case YUV420_FORMAT:
		bpp = 16;
		break;
#endif
	default:
		uerr("set_fb_source_info: unsupport format\n");
		return -1;
	}
	/*
	** For the cases having scaling up,
	** Ex:
	**   - 4K H.Down --> H.Up patch
	**   - Typical video wall usage
	**   - Hybrid mode
	**   - 4K 60Hz YUV420 --> SoC V3 need H.Upcale x2.
	** the terminal count will be updated again in __crt_change_scale_factor()
	** and crt_update_terminal_cnt() function.
	**
	** We only calculate the case without scaling up here.
	*/
	TerminalCount = CALC_TERMINAL_CNT(HActive, bpp);

	crt_info->terminal_cnt = TerminalCount;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	__mod_reg84(crt,
	            (TerminalCount << TERMINAL_COUNT_BIT) | DisplayOffset,
	            TERMINAL_COUNT_MASK | DISPLAY_OFFSET_MASK
#if (CONFIG_AST1500_SOC_VER >= 3)
	            /* there is no initial vaule for CRT 0x84[2:0], we clear it here to avoid broken image */
	            | BLKINIV_MASK
#endif
	            );

	_crt_display_offset_set(crt, DisplayOffset);

#if (CONFIG_AST1500_SOC_VER >= 3)// && (HW_PROFILE)
	/* Update terminal count for VEFlip */
	crt_info->terminal_cnt_yuv444 = CALC_TERMINAL_CNT(HActive, 24);
	crt_info->terminal_cnt_yuv422 = CALC_TERMINAL_CNT(HActive, 16);

	SetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT,
			VEFLIP_YUV444_TC(crt_info->terminal_cnt_yuv444)
			| VEFLIP_YUV422_TC(crt_info->terminal_cnt_yuv422));

	/* Reset base address offset here. Will be updated from Video Wall setup */
	SetSCUReg(CRT2_VEFLIP_YUV444_BASE_OFFSET, 0);
	SetSCUReg(CRT2_VEFLIP_YUV422_BASE_OFFSET, 0);

	crt->dec_buf_offset = 0;
#endif

#if (CONFIG_AST1500_SOC_VER >= 2)
	/*
	** VE driver will call crt_disable_flip() when _do_CLIENT_EVENT_GOT_HOST_CHANGE_MODE().
	** We have to re-enable it under force timing mode.
	** FIXME: Why we call crt_disable_flip() when _do_CLIENT_EVENT_GOT_HOST_CHANGE_MODE()?!
	*/
	if (ast_scu.ability.v_crt_flip) {
		if (crt_info->owner == OWNER_CON)
			_crt_disable_flip(crt, NULL);
#if (CONFIG_AST1500_SOC_VER == 2)
		else
			_crt_enable_flip(crt, NULL);
#endif
	}
#endif

	return 0;
}


#if (2 <= CONFIG_AST1500_SOC_VER)
void _crt_vsync_intr_location(struct s_crt_drv *crt, int fine_tune)
{
	MODE_ITEM *mode;
	u32 line;

	/*
	** Bruce151215.
	** User may force switch to Console Screen while VE decoding.
	** In this case, if we set wrong interrupt line, interrupt won't generate
	** and VE driver will 'desc full' because of no interrupt.
	**
	** FIXME. Console 30Hz, but VE 60Hz... ==> VE desc full again...
	*/
	mode = crt_get_current_mode(crt, crt->current_owner, NULL);

	/* Bruce141210: interrupt should depends on timing, not compress data.
	** Should use VActive instead of VPixels.
	*/
	line = mode->VActive;

	/* interlace mode */
	if (GetSCUReg(CRT2_CONTROL_REG) & ENABLE_CRT_INTERLACE_TIMING_MASK)
		line <<= 1;

	line += fine_tune;

	uinfo("Set VSync Interrupt on Line %d\n", line);
	ModSCUReg(CRT2_CONTROL2_REG, line << LINE_TO_INT_BIT, LINE_TO_INT_MASK);
}

void crt_vsync_intr_location(struct s_crt_drv *crt, int fine_tune)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_vsync_intr_location, (void *)fine_tune, 0);
}
#endif

static int SetASTModeTimingTest(struct s_crt_drv *crt,
							int      nCRTIndex,
							u16    	HPixels,
							u16    	VPixels,
							u8      	RefreshRate,
							eVIDEO_FORMAT   ColorFormat,
							struct s_crt_info *crt_info)
{
	u32   temp, RetraceStart, RetraceEnd;
	u32   ulFormat, mask = 0;
	MODE_ITEM *mt = crt->mode_table;
	int		AstModeIndex;
	u16 HActive, VActive;

	AstModeIndex = lookupModeTable(crt, crt_info);

	/*
	** HPixels, VPixels: The actual pixels used by VE decompress.
	** HActive, VActive: The expected CRT timing.
	** In most of cases HPixels will == HActive. But it will be different under
	** video wall case, host down-scaling case and hybrid mode.
	*/
	HActive = mt[AstModeIndex].HActive;
	VActive = mt[AstModeIndex].VActive;

	//  Set CRT Display Offset and Terminal Count
	_chg_fb_src_format(crt, nCRTIndex, HActive, ColorFormat, crt_info);

	if (crt->op_state & COPS_FORCING_TIMING)
		return 0;

	uinfo("Change CRT timing...%d x %d\n", HActive, VActive);
	/* Disable OSD. It is necessary because the video timing is going to change. */
	EnableOSD(nCRTIndex, 0);

	//  Access CRT Engine
	//  SetPolarity
	ModSCUReg(CRT1_CONTROL_REG + nCRTIndex*0x60,
			(mt[AstModeIndex].HorPolarity << HOR_SYNC_SELECT_BIT) | (mt[AstModeIndex].VerPolarity << VER_SYNC_SELECT_BIT),
			HOR_SYNC_SELECT_MASK|VER_SYNC_SELECT_MASK);

	//enable function single edge, dvo and dac
#if (CONFIG_AST1500_SOC_VER == 1)
	SetSCUReg(CRT1_CONTROL2_REG + nCRTIndex*0x60, 0x81);
#else
	// SoC V2 add CRT interrupt support. We should not touch this setting when changing video timing.
	ModSCUReg(CRT1_CONTROL2_REG + nCRTIndex*0x60, 0x80, ~LINE_TO_INT_MASK);
#endif

	//  Horizontal Timing
	temp = 0;
	/* Bruce141210.
	** DE timing should depends on CRT timing table. Not compression information.
	** So, we should use 'HActive' instead of 'HPixels'.
	** temp = ((HActive - 1) << HOR_ENABLE_END_BIT) | ((mt[AstModeIndex].HTotal - 1) << HOR_TOTAL_BIT);
	*/
	if ((ast_scu.ability.v_compatibility_mode & NC_1680_TO_1664) && (HActive == 1680)) {
		/* 1680x1050 patch. Convert 1680 => 1664. */
		temp = ((1664 - 1) << HOR_ENABLE_END_BIT) | ((mt[AstModeIndex].HTotal - 1) << HOR_TOTAL_BIT);
	} else {
		temp = ((HActive - 1) << HOR_ENABLE_END_BIT) | ((mt[AstModeIndex].HTotal - 1) << HOR_TOTAL_BIT);
	}
	SetSCUReg(CRT1_HOR_TOTAL_END_REG  + nCRTIndex*0x60, temp);

	RetraceStart = HActive + mt[AstModeIndex].HFPorch - 1;
	RetraceEnd = (RetraceStart + mt[AstModeIndex].HSyncTime);
	temp = 0;
	temp = (RetraceEnd << HOR_RETRACE_END_BIT) | (RetraceStart << HOR_RETRACE_START_BIT);
	SetSCUReg(CRT1_HOR_RETRACE_REG  + nCRTIndex*0x60, temp);

	//  Vertical Timing
	temp = 0;
	/* Bruce141210.
	** DE timing should depends on CRT timing table. Not compression information.
	** So, we should use 'HActive' instead of 'HPixels'.
	** temp = ((VActive - 1) << VER_ENABLE_END_BIT) | ((mt[AstModeIndex].VTotal - 1) << VER_TOTAL_BIT);
	*/
	temp = ((VActive - 1) << VER_ENABLE_END_BIT) | ((mt[AstModeIndex].VTotal - 1) << VER_TOTAL_BIT);
	SetSCUReg(CRT1_VER_TOTAL_END_REG  + nCRTIndex*0x60, temp);

	temp = 0;
	RetraceStart = VActive + mt[AstModeIndex].VFPorch - 1;
	RetraceEnd = (RetraceStart + mt[AstModeIndex].VSyncTime);
	temp = (RetraceEnd << VER_RETRACE_END_BIT) | (RetraceStart << VER_RETRACE_START_BIT);
	SetSCUReg(CRT1_VER_RETRACE_REG  + nCRTIndex*0x60, temp);

	if (crt_info->crt_output_format == YUV420_FORMAT) {
		/*
		** This mode is only valid for SoC > V3 A1.
		** CRT60[9] Set to 0. ==> disable CSC
		** CRT60[7:8] don't care
		** CRT60[12] Set to 1. ==> enable "YUV422 tile" in to "4K YUV420" out.
		** CRT6C[25] Set to 1.
		*/
		ulFormat = YUV_FORMAT1;
	} else {
		switch (ColorFormat) {
		case YUV444_FORMAT:
			ulFormat = FORMAT_YUV444;
#if (CONFIG_AST1500_SOC_VER >= 2)
			if (crt_info->crt_output_format == XRGB8888_FORMAT)
				ulFormat |= TILE_SELECT_MASK;

#if NEW_24BIT_MODE
			ulFormat |= YUV_FORMAT1;
#else
			ulFormat |= YUV_FORMAT0;
#endif
#endif
			break;
		case XRGB8888_FORMAT:
			ulFormat = FORMAT_XRGB8888;
			break;
		case RGB565_FORMAT:
			ulFormat = FORMAT_RGB565;
			break;
#if (CONFIG_AST1500_SOC_VER >= 2)
		case YUV420_FORMAT:
			ulFormat = FORMAT_YUV420;
#if (CONFIG_AST1500_SOC_VER == 2)
			ulFormat |= YUV_FORMAT3;
#endif
			if (crt_info->crt_output_format == XRGB8888_FORMAT)
				ulFormat |= TILE_SELECT_MASK;
			break;
#endif
		default:
			uerr("SetASTModeTiming: unsupport format\n");
			return -1;
		}
	}

#if (CONFIG_AST1500_SOC_VER >= 2)
	if (crt_info->owner != OWNER_CON) {
		ulFormat |= ENABLE_YUV_TILE_MODE; /* Enable YUV tile mode */

		if (ast_scu.ability.v_interlace_mode) {
			if (mt[AstModeIndex].ScanMode == Interl)
				ulFormat |= ENABLE_CRT_INTERLACE_TIMING_MASK; //Enable interlace mode.
		}
	}
#endif

	//  Set Color Format
	mask |= FORMAT_SELECT_MASK;
#if (2 <= CONFIG_AST1500_SOC_VER)
	mask |= (YUV_FORMAT_SELECT_MASK
		| ENABLE_CRT_INTERLACE_TIMING_MASK
		| EN_X_SCALE_UP
		| TILE_SELECT_MASK
		| ENABLE_YUV_TILE_MODE);

#endif /* #if (2 <= CONFIG_AST1500_SOC_VER) */

	ModSCUReg(CRT1_CONTROL_REG + nCRTIndex*0x60, ulFormat, mask);

#if (CONFIG_AST1500_SOC_VER >= 3)
	if (crt_info->crt_output_format == YUV420_FORMAT)
		ModSCUReg(CRT2_VIDEO_PLL_REG, YUV420_SWAP_UV, YUV420_SWAP_UV);
	else
		ModSCUReg(CRT2_VIDEO_PLL_REG, 0, YUV420_SWAP_UV);
#endif

	//  Set Threshold
	temp = 0;
	temp = (CRT_HIGH_THRESHOLD_VALUE << THRES_HIGHT_BIT) | (CRT_LOW_THRESHOLD_VALUE << THRES_LOW_BIT);
	ModSCUReg(CRT1_THRESHOLD_REG + nCRTIndex*0x60,
	           temp,
	           THRES_LOW_MASK | THRES_HIGHT_MASK);

	_set_pixel_clock(nCRTIndex, &mt[AstModeIndex]);

#if (CONFIG_AST1500_SOC_VER >= 2)
	/*
	** Bruce151215.
	** User may force switch to Console Screen while VE decoding.
	** In this case, if we set wrong interrupt line, interrupt won't generate
	** and VE driver will 'desc full' because of no interrupt.
	** In this case, VE driver may not call crt_vsync_intr_location(), so
	** set a default value crt_vsync_intr_location() here can resolve the problem.
	**
	** FIXME. Console 30Hz, but VE 60Hz... ==> VE desc full again...
	** FIXME. What if user switch back to 'decode screen' again? ==>
	**        VE still need to call crt_vsync_intr_location().
	**        Since this scenario is only for enginer test so far. I'm going
	**        to ignore those issues.
	*/
	/* enable CRT INT at last slice so that we don't need anti-flicker */
	_crt_vsync_intr_location(crt, -8);
#endif


#if (CONFIG_AST1500_SOC_VER == 3) && (PATCH_CODEV3 & PATCH_AST1520A0_BUG14082801_CRT_4K)
	if (mt[AstModeIndex].DCLK10000 > 1700000)
		ModSCUReg(CRT2_PANNING, Y_SYNC_PAN(1), Y_SYNC_PAN_MASK);
	else
		ModSCUReg(CRT2_PANNING, Y_SYNC_PAN(0), Y_SYNC_PAN_MASK);
#endif
	return 0;
}


static inline void _crt_set_vbuf(struct s_crt_drv *crt, u32 buffer_offset)
{
	ModSCUReg(CRT1_DISPLAY_ADDRESS + 0x60 * crt->disp_select, buffer_offset, DISPLAY_ADDRESS_MASK);
}

void crt_set_vbuf_atomic(struct s_crt_drv *crt, u32 buffer_offset)
{
	crt->dec_buf_base = buffer_offset;

	/* If we are not in decode screen, don't change. */
	if (crt->current_owner != OWNER_VE1)
		return;

	_crt_set_vbuf(crt, buffer_offset);
}


static int ASTSetMode (struct s_crt_drv *crt,
                int nCRTIndex,
                u32 VGABaseAddr,
                u16 Horizontal,
                u16 Vertical,
                u16 RefreshRate,
                eVIDEO_FORMAT ColorFormat,
                struct s_crt_info *i)
{
	int    bResult;

	_crt_pwr_save(crt, 0); //Bruce121220. Re-enable HSync here. Is there a better place?

	/*
	** Bruce170823. SCREEN_OFF_ON will disable DE which causes monitor black out.
	** We believe there is no need to toggle SCREEN_OFF_ON. I leave it here just because
	** this is legacy code and I don't want to break something I don't remember.
	*/
	if (!(crt->op_state & COPS_FORCING_TIMING))
		ModSCUReg(CRT1_CONTROL_REG + 0x60*nCRTIndex, (GRAPH_DISPLAY_OFF|SCREEN_OFF_ON), (GRAPH_DISPLAY_MASK|SCREEN_OFF_MASK));

	bResult = SetASTModeTimingTest (crt,
	                         nCRTIndex,
	                         Horizontal,
	                         Vertical,
	                         RefreshRate,
	                         ColorFormat,
	                         i);
	if (bResult)
		return bResult;

	// Enable CRT engine
	/*
	** Bruce111110. Note that we should setup all required video timing before
	** enabling GRAPH_DISPLAY_ON. Otherwise, CRT may go crazy and occupy memory
	** bus forever.
	*/
	if (!(crt->op_state & COPS_FORCING_TIMING)) {
		ModSCUReg(CRT1_CONTROL_REG + 0x60*nCRTIndex, (GRAPH_DISPLAY_ON|SCREEN_OFF_OFF), (GRAPH_DISPLAY_MASK|SCREEN_OFF_MASK));
		/*
		** Bruce150128. Add delay here before setup xmitter. Hopefully provide more
		** stable timing to xmitter.
		*/
		msleep(1);
	}
	return 0;
}



//AST1510 to do:
//This routine does not support 24-bit and YUV420.
static inline void vbufset_YUV444(unsigned char *vbuf, u32 width, u32 height, u32 color)
{
	int j,i;
	u32 Y,U,V;

	Y = (color & 0xFF000000) >> 24;
	Y |= (Y << 8) |(Y << 16) |(Y << 24);
	U = (color & 0x00FF0000) >> 16;
	U |= (U << 8) |(U << 16) |(U << 24);
	V = (color & 0x0000FF00) >> 8;
	V |= (V << 8) |(V << 16) |(V << 24);


	for (i=0; i<height; i++)
	{
		for (j=0; j<width/8; j++)
		{
			//Y 4byte per write
			*(u32 *)(vbuf + (i*width*4+j*32)) = Y;
			*(u32 *)(vbuf + (i*width*4+j*32) +4) = Y;
			//U
			*(u32 *)(vbuf + (i*width*4+j*32) +8) = U;
			*(u32 *)(vbuf + (i*width*4+j*32) +12) = U;
			//V
			*(u32 *)(vbuf + (i*width*4+j*32) +16) = V;
			*(u32 *)(vbuf + (i*width*4+j*32) +20) = V;
			//A
			//*(ULONG *)(ulCRTAddr + (i*iWidth*4+j*32) +24) = 0xFFFFFFFFF;
			//*(ULONG *)(ulCRTAddr + (i*iWidth*4+j*32) +28) = 0xFFFFFFFFF;
		}
	}
}

static inline void vbufset_RGB565(unsigned char *vbuf, u32 width, u32 height, u32 color)
{
	u32 x, y;

	// 1 pixel takes 16bits, so, u32 color represent 2 pixels.
	width = width >> 1;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*(u32 *)vbuf = color;
			vbuf += sizeof(u32);
		}
	}

}

static inline void __vbufset_XRGB8888_half(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, half, new_width, DisplayOffset, bpp = 32;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	half = new_width >> 1;

	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width; x++) {
			if (x < half)
				*(u32 *)vbuf = color;
			else
				*(u32 *)vbuf = color2;
			vbuf += sizeof(u32);
		}
	}
}

static inline void __vbufset_XRGB8888_x(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, new_width, DisplayOffset, bpp = 32;
	u32 w1, w2, r;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	/* We take XRGB reserved X part to indicate the Y line width. */
	w1 = (color & 0xFF000000) >> 24;
	w2 = (color2 & 0xFF000000) >> 24;
	w1 = (w1 == 0xFF) ? 1 : w1;
	w2 = (w2 == 0xFF) ? 1 : w2;
	w1 = (w1 == 0) ? 1 : w1;
	w2 = (w2 == 0) ? 1 : w2;

	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width;) {
			for (r = 0; (r < w1) && (x < new_width); r++, x++) {
				*(u32 *)vbuf = color;
				vbuf += sizeof(u32);
			}
			for (r = 0; (r < w1) && (x < new_width); r++, x++) {
				*(u32 *)vbuf = color2;
				vbuf += sizeof(u32);
			}
		}
	}
}

static inline void __vbufset_XRGB8888_xy(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, new_width, DisplayOffset, bpp = 32;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width; x++) {
			/* draw X boundary */
			if ((x == 0) || (x == (width - 1))) {
				*(u32 *)vbuf = color;
				vbuf += sizeof(u32);
				continue;
			}
			/* draw Y boundary */
			if ((y == 0) || (y == (height - 1))) {
				*(u32 *)vbuf = color;
				vbuf += sizeof(u32);
				continue;
			}
			/* draw X/Y line */
			if (x == y)
				*(u32 *)vbuf = color;
			else
				*(u32 *)vbuf = color2;
			vbuf += sizeof(u32);
		}
	}
}

static struct s_test_pattern_cfg {
	unsigned int start;
	unsigned char *vbuf;
	u32 width;
	u32 height;
	u32 color;
	u32 color2;
} test_pattern_cfg;

static inline void ___vbufset_XRGB8888_random(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, new_width, DisplayOffset, bpp = 32;
	u32 w1, w2, r;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	/* We take XRGB reserved X part to indicate the Y line width. */
	w1 = (color & 0xFF000000) >> 24;
	w2 = (color2 & 0xFF000000) >> 24;
	w1 = (w1 == 0xFF) ? 1 : w1;
	w2 = (w2 == 0xFF) ? 1 : w2;
	w1 = (w1 == 0) ? 1 : w1;
	w2 = (w2 == 0) ? 1 : w2;
	
	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width;) {
			for (r = 0; (r < w1) && (x < new_width); r++, x++) {
				*(u32 *)vbuf = color;
				vbuf += sizeof(u32);
			}
			for (r = 0; (r < w1) && (x < new_width); r++, x++) {
				*(u32 *)vbuf = color2;
				vbuf += sizeof(u32);
			}
		}
	}
}

void __vbufset_XRGB8888_random_work(struct s_crt_drv *crt, void *data_ptr)
{
	struct s_test_pattern_cfg *t = (struct s_test_pattern_cfg*)data_ptr;

	if (t->start) {
		t->color ^= 0x00AAAAAA;
		t->color2 ^= 0x00555555;
		___vbufset_XRGB8888_random(t->vbuf, t->width, t->height, t->color, t->color2);
	}

	if (t->start) {
		/* Since it is for test Only. I use the buggy queue_crt_delayed_work(). */
		queue_crt_delayed_work(crt, __vbufset_XRGB8888_random_work, &test_pattern_cfg, 0, 300);
	}
}

static inline void __vbufset_XRGB8888_random(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2, struct s_crt_drv *crt)
{
	test_pattern_cfg.start = 1;
	test_pattern_cfg.vbuf = vbuf;
	test_pattern_cfg.width = width;
	test_pattern_cfg.height = height;
	test_pattern_cfg.color = color;
	test_pattern_cfg.color2 = color2;
	queue_crt_work(crt, __vbufset_XRGB8888_random_work, &test_pattern_cfg, 0);
}

static inline void __vbufset_XRGB8888_color_bar(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, new_width, DisplayOffset, bpp = 32;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width; x++) {
			/* Change a color every 64 pixels */
			unsigned int p;

			p = (x >> 6) & 0x7;
			switch (p) {
			case 0:
				*(u32 *)vbuf = color;
				break;
			case 1:
				*(u32 *)vbuf = color2;
				break;
			case 2:
				*(u32 *)vbuf = 0xFFFF0000; //Red
				break;
			case 3:
				*(u32 *)vbuf = 0xFF00FF00; //Green
				break;
			case 4:
				*(u32 *)vbuf = 0xFF0000FF; //Blue
				break;
			case 5:
				*(u32 *)vbuf = 0xFFFFFFFF; //White
				break;
			case 6:
				*(u32 *)vbuf = 0xFF000000; //Black
				break;
			case 7:
				*(u32 *)vbuf = 0xFFFFFF00; //Red+Green = Yellow
				break;
			}
			vbuf += sizeof(u32);
		}
	}
}

#define get_r(c) (((c) & 0x00FF0000) >> 16)
#define get_g(c) (((c) & 0x0000FF00) >> 8)
#define get_b(c) (((c) & 0x000000FF))
#define to_xrgb(r, g, b) (0xFF000000 | (r) << 16 | (g) << 8 | (b))

static inline void __vbufset_XRGB8888_ramp1(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, xs, new_width, DisplayOffset, bpp = 32;
	u32 R, G ,B;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;

	for (y = 0; y < height; y++) {
		for (x = 0; x < new_width;) {
			R = get_r(color);
			G = get_g(color);
			B = get_b(color);
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R - 1) & 0xFF;
				G = (G - 1) & 0xFF;
				B = (B - 1) & 0xFF;
				vbuf += sizeof(u32);
			}
			R = (R + 1) & 0xFF;
			G = (G + 1) & 0xFF;
			B = (B + 1) & 0xFF;
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R + 1) & 0xFF;
				G = (G + 1) & 0xFF;
				B = (B + 1) & 0xFF;
				vbuf += sizeof(u32);
			}
			R = get_r(color2);
			G = get_g(color2);
			B = get_b(color2);
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R + 1) & 0xFF;
				G = (G + 1) & 0xFF;
				B = (B + 1) & 0xFF;
				vbuf += sizeof(u32);
			}
			R = (R - 1) & 0xFF;
			G = (G - 1) & 0xFF;
			B = (B - 1) & 0xFF;
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R - 1) & 0xFF;
				G = (G - 1) & 0xFF;
				B = (B - 1) & 0xFF;
				vbuf += sizeof(u32);
			}
			/* ramp red */
			/* breaks every 256 pixels */
			R = 0xFF;
			G = 0;
			B = 0;
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R - 1) & 0xFF;
				G = 0;
				B = 0;
				vbuf += sizeof(u32);
			}
			R = 0x0;
			G = 0;
			B = 0;
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = (R + 1) & 0xFF;
				G = 0;
				B = 0;
				vbuf += sizeof(u32);
			}
			/* ramp green */
			/* breaks every 256 pixels */
			R = 0;
			G = 0xFF;
			B = 0;
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = 0;
				G = (G - 1) & 0xFF;
				B = 0;
				vbuf += sizeof(u32);
			}
			R = 0;
			G = 0x0;
			B = 0;
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = 0;
				G = (G + 1) & 0xFF;
				B = 0;
				vbuf += sizeof(u32);
			}
			/* ramp blue */
			/* breaks every 256 pixels */
			R = 0;
			G = 0;
			B = 0xFF;
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = 0;
				G = 0;
				B = (B - 1) & 0xFF;
				vbuf += sizeof(u32);
			}
			R = 0;
			G = 0;
			B = 0x0;
			/* breaks every 256 pixels */
			for (xs = 0; (xs < 256) && (x < new_width); xs++, x++) {
				*(u32 *)vbuf = to_xrgb(R, G, B);
				R = 0;
				G = 0;
				B = (B + 1) & 0xFF;
				vbuf += sizeof(u32);
			}
		}
	}
}

#if 0
static inline void __vbufset_XRGB8888_ramp2(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, x1, s, new_width, DisplayOffset, bpp = 32;
	u32 R, G ,B;
	u32 hend, helf_width;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;
	hend = height >> 2;
	helf_width = width >> 1;

	for (y = 0; y < hend; y++) {
		R = get_r(color);
		G = get_g(color);
		B = get_b(color);
		for (x = 0; x < helf_width; x++) {
			s = 0xFF * x / helf_width;
			*(u32 *)vbuf = to_xrgb(
				(R - s) & 0xFF,
				(G - s) & 0xFF,
				(B - s) & 0xFF);
			vbuf += sizeof(u32);
		}
		R = get_r(color2);
		G = get_g(color2);
		B = get_b(color2);
		for (x1 = 0; x < new_width; x1++, x++) {
			s = 0xFF * x1 / helf_width;
			*(u32 *)vbuf = to_xrgb(
				(R + s) & 0xFF,
				(G + s) & 0xFF,
				(B + s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
	/* Red ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < helf_width; x++) {
			s = 0xFF * x / helf_width;
			*(u32 *)vbuf = to_xrgb(
				(0xFF - s) & 0xFF,
				0,
				0);
			vbuf += sizeof(u32);
		}
		for (x1 = 0; x < new_width; x1++, x++) {
			s = 0xFF * x1 / helf_width;
			*(u32 *)vbuf = to_xrgb(
				(0 + s) & 0xFF,
				0,
				0);
			vbuf += sizeof(u32);
		}
	}
	/* Green ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < helf_width; x++) {
			s = 0xFF * x / helf_width;
			*(u32 *)vbuf = to_xrgb(
				0,
				(0xFF - s) & 0xFF,
				0);
			vbuf += sizeof(u32);
		}
		for (x1 = 0; x < new_width; x1++, x++) {
			s = 0xFF * x1 / helf_width;
			*(u32 *)vbuf = to_xrgb(
				0,
				(0 + s) & 0xFF,
				0);
			vbuf += sizeof(u32);
		}
	}
	/* Blue ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < helf_width; x++) {
			s = 0xFF * x / helf_width;
			*(u32 *)vbuf = to_xrgb(
				0,
				0,
				(0xFF - s) & 0xFF);
			vbuf += sizeof(u32);
		}
		for (x1 = 0; x < new_width; x1++, x++) {
			s = (0xFF * x1 / helf_width);
			*(u32 *)vbuf = to_xrgb(
				0,
				0,
				(0 + s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
}
#endif
static inline void __vbufset_XRGB8888_ramp2(unsigned char *vbuf, u32 width, u32 height, u32 color, u32 color2)
{
	u32 x, y, s, new_width, DisplayOffset, bpp = 32;
	u32 R, G ,B;
	u32 hend, helf_width;

	DisplayOffset = (width * bpp) / 8;

	DisplayOffset &= ~DISPLAY_OFFSET_ALIGN;

	new_width = DisplayOffset * 8 / bpp;
	hend = height >> 3;
	helf_width = width >> 1;

	for (y = 0; y < hend; y++) {
		R = get_r(color);
		G = get_g(color);
		B = get_b(color);
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				(R - s) & 0xFF,
				(G - s) & 0xFF,
				(B - s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
	for (y = 0; y < hend; y++) {
		R = get_r(color2);
		G = get_g(color2);
		B = get_b(color2);
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				(R + s) & 0xFF,
				(G + s) & 0xFF,
				(B + s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
	/* Red ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				(0xFF - s) & 0xFF,
				0,
				0);
			vbuf += sizeof(u32);
		}
	}
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				(0 + s) & 0xFF,
				0,
				0);
			vbuf += sizeof(u32);
		}
	}
	/* Green ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				0,
				(0xFF - s) & 0xFF,
				0);
			vbuf += sizeof(u32);
		}
	}
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				0,
				(0 + s) & 0xFF,
				0);
			vbuf += sizeof(u32);
		}
	}
	/* Blue ramp; */
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = 0xFF * x / width;
			*(u32 *)vbuf = to_xrgb(
				0,
				0,
				(0xFF - s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
	for (y = 0; y < hend; y++) {
		for (x = 0; x < new_width; x++) {
			s = (0xFF * x / width);
			*(u32 *)vbuf = to_xrgb(
				0,
				0,
				(0 + s) & 0xFF);
			vbuf += sizeof(u32);
		}
	}
}

static inline void vbufset_XRGB8888(unsigned char *vbuf, u32 width, u32 height, u32 color)
{
	__vbufset_XRGB8888_x(vbuf, width, height, color, color);
}

void vbufset(unsigned char * vbuf, u32 width, u32 height, eVIDEO_FORMAT cf, u32 color)
{
	switch (cf) {
    case YUV444_FORMAT:
		vbufset_YUV444(vbuf, width, height, color);
		break;
    case YUV420_FORMAT:
		uerr("unsupported color format!\n");
		break;
    case RGB565_FORMAT:
		vbufset_RGB565(vbuf, width, height, color);
		break;
    case XRGB8888_FORMAT:
		vbufset_XRGB8888(vbuf, width, height, color);
		break;
	}
}

static inline void vbufset_purple(unsigned char * vbuf, u32 width, u32 height, eVIDEO_FORMAT cf)
{
	switch (cf) {
    case YUV444_FORMAT:
		vbufset_YUV444(vbuf, width, height, PURPLE_YUV444);
		break;
    case YUV420_FORMAT:
		uerr("unsupported color format!\n");
		break;
    case RGB565_FORMAT:
		vbufset_RGB565(vbuf, width, height, PURPLE_RGB565);
		break;
    case XRGB8888_FORMAT:
		vbufset_XRGB8888(vbuf, width, height, PURPLE_XRGB888);
		break;
	}
}


static inline void vbufset_black(unsigned char * vbuf, u32 width, u32 height, eVIDEO_FORMAT cf)
{
	switch (cf) {
    case YUV444_FORMAT:
		vbufset_YUV444(vbuf, width, height, BLACK_YUV444);
		break;
    case YUV420_FORMAT:
		uerr("unsupported color format!\n");
		break;
    case RGB565_FORMAT:
		vbufset_RGB565(vbuf, width, height, BLACK_RGB565);
		break;
    case XRGB8888_FORMAT:
		vbufset_XRGB8888(vbuf, width, height, BLACK_XRGB888);
		break;
	}
}

static inline void vbufset_blue(unsigned char * vbuf, u32 width, u32 height, eVIDEO_FORMAT cf)
{
	switch (cf) {
    case YUV444_FORMAT:
		vbufset_YUV444(vbuf, width, height, BLUE_YUV444);
		break;
    case YUV420_FORMAT:
		uerr("unsupported color format!\n");
		break;
    case RGB565_FORMAT:
		vbufset_RGB565(vbuf, width, height, BLUE_RGB565);
		break;
    case XRGB8888_FORMAT:
		vbufset_XRGB8888(vbuf, width, height, BLUE_XRGB888);
		break;
	}
}

static inline void vbufset_white(unsigned char * vbuf, u32 width, u32 height, eVIDEO_FORMAT cf)
{
	switch (cf) {
    case YUV444_FORMAT:
		vbufset_YUV444(vbuf, width, height, WHITE_YUV444);
		break;
    case YUV420_FORMAT:
		uerr("unsupported color format!\n");
		break;
    case RGB565_FORMAT:
		vbufset_RGB565(vbuf, width, height, WHITE_RGB565);
		break;
    case XRGB8888_FORMAT:
		vbufset_XRGB8888(vbuf, width, height, WHITE_XRGB888);
		break;
	}
}

void vbufset_test_mode(struct s_crt_drv *crt, struct s_crt_info *i, u32 type, u32 color, u32 color2)
{
	/* Stop queued worker. */
	test_pattern_cfg.start = 0;

	switch (type) {
	case 0:
		vbufset_black(TO_CRT_CONSOLE_BASE(crt->vbuf_va), i->width, i->height, i->crt_color);
		break;
	case 1:
		vbufset_blue(TO_CRT_CONSOLE_BASE(crt->vbuf_va), i->width, i->height, i->crt_color);
		break;
	case 2:
		vbufset_purple(TO_CRT_CONSOLE_BASE(crt->vbuf_va), i->width, i->height, i->crt_color);
		break;
	case 3:
		__vbufset_XRGB8888_half(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 4:
		__vbufset_XRGB8888_x(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 5:
		__vbufset_XRGB8888_xy(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 6:
		__vbufset_XRGB8888_color_bar(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 7:
		__vbufset_XRGB8888_ramp1(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 8:
		__vbufset_XRGB8888_ramp2(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2);
		break;
	case 9:
		__vbufset_XRGB8888_random(TO_CRT_CONSOLE_BASE(crt->vbuf_va),
			i->width, i->height, color, color2, crt);
		break;
	default:
		vbufset_white(TO_CRT_CONSOLE_BASE(crt->vbuf_va), i->width, i->height, i->crt_color);
		break;
	}
}

#if 0
static void DisableCRT(int nCRTIndex)
{
    ModSCUReg((CRT1_CONTROL_REG + 0x60*nCRTIndex), GRAPH_DISPLAY_OFF, GRAPH_DISPLAY_MASK);
}
#endif

//Should all OSD register ready, then enable OSD
void EnableOSD(int nCRTIndex, int inbEnOSD)
{
    if (inbEnOSD)
    {
        //Enable OSD
        ModSCUReg((CRT1_CONTROL_REG + 0x60*nCRTIndex), OSD_EN, ENABLE_OSD_MASK);
    }
    else
    {
        //Disable OSD
        ModSCUReg((CRT1_CONTROL_REG + 0x60*nCRTIndex), 0, ENABLE_OSD_MASK);
    }
}
#if 0
void SetOSDReg(int nCRTIndex,
    ULONG   ulOSDAddress, //Physical
    POSDInfo pOSDInfo)
{
    ULONG   ulValue;
    ULONG   ulOSDOffset, ulOSDCount;
    ULONG   ulCRTOffset = nCRTIndex*0x60;

    ulValue = (pOSDInfo->HStart&OSD_HOR_START_MASK)|((pOSDInfo->HEnd<<OSD_HOR_END_BIT)&OSD_HOR_END_MASK);
    SetSCUReg((CRT1_OSD_HORIZONTAL_REG+ulCRTOffset), ulValue);

    ulValue = (pOSDInfo->VStart&OSD_VER_START_MASK)|((pOSDInfo->VEnd<<OSD_VER_END_BIT)&OSD_VER_END_MASK);
    SetSCUReg((CRT1_OSD_VERTICAL_REG+ulCRTOffset), ulValue);

    SetSCUReg(CRT1_OSD_ADDRESS_REG+ulCRTOffset, ulOSDAddress);

    ulOSDOffset = (pOSDInfo->HEnd - pOSDInfo->HStart)/2;
    ulOSDCount  = (pOSDInfo->HEnd - pOSDInfo->HStart + 15)/16;
    ulValue     = (ulOSDOffset&OSD_OFFSET_MASK) | ((ulOSDCount<<OSD_TERMINAL_BIT)&OSD_TERMINAL_MASK);
    SetSCUReg(CRT1_OSD_OFFSET_REG+ulCRTOffset, ulValue);

    ulValue = (OSD_THRES_LOW_VALUE&OSD_THRES_LOW_MASK)|
            ((OSD_THRES_HIGH_VALUE<<OSD_THRES_HIGH_BIT)&OSD_THRES_HIGH_MASK)|
            ((OSD_ALPHA_VALUE<<OSD_ALPHA_BIT)&OSD_ALPHA_BIT_MASK);
    SetSCUReg(CRT1_OSD_THRESHOLD_REG+ulCRTOffset, ulValue);
}

//Fill Pallete
void FillPallete(void)
{
    ULONG i;
    ULONG PaletteData, PaletteIndex=0;

    //4 byte per time, total 32*4/16 = 16 palette
    //all color must map form [0-255] to [0-31]
    for (i=0 ; i<32; i+=4)
    {
        PaletteData = (OSDPallette[PaletteIndex].X << 15) | ((OSDPallette[PaletteIndex].R/8) << 10) | ((OSDPallette[PaletteIndex].G/8) << 5) | (OSDPallette[PaletteIndex].B/8);
        PaletteData |= (OSDPallette[PaletteIndex+1].X << 31) | ((OSDPallette[PaletteIndex+1].R/8) << 26) | ((OSDPallette[PaletteIndex+1].G/8) << 21) | ((OSDPallette[PaletteIndex+1].B/8) << 16);
        SetSCUReg(OSD_PALETTE_INDEX_BASE + i, PaletteData);
        PaletteIndex+=2;
    }
}

//Fill Data
void ShowOSD(
            struct s_crt_drv *crt,
            int nCRTIndex,
            ULONG ulInWidth,
            ULONG ulInHeight,
            ULONG ulBmpNum)
{
    ULONG   i, j;
    OSDInfo OSDInfo;
    ULONG   ulWidth, ulHeight;
    unsigned char *ulTempAddr;
    BYTE    *pOSDData;
    int     nOSDNum;

    OSDInfo.HStart  = (ulInWidth-400)/2;
    OSDInfo.HEnd    = OSDInfo.HStart+400;
    OSDInfo.VStart  = (ulInHeight-200)/2;
    OSDInfo.VEnd    = OSDInfo.VStart+200;

    ulWidth     = OSDInfo.HEnd - OSDInfo.HStart;
    ulHeight    = OSDInfo.VEnd - OSDInfo.VStart;

    ulTempAddr  = TO_CRT_OSD_BASE(crt->vbuf_va);

    nOSDNum = sizeof(OSDDATA)/sizeof(OSDDATA[0]);

    if (ulBmpNum < nOSDNum)
    {
        pOSDData = OSDDATA[ulBmpNum];
    }
    else
    {
        uerr("BmpNum[%d] is over OSDNum[%d]\n",(u32)ulBmpNum, nOSDNum);
        EnableOSD(nCRTIndex, FALSE);
        return;
    }

    //Fill Data
    for (i=0; i<ulHeight; i++)
    {
        for (j=0; j<ulWidth/2; j++)
        {
            *((BYTE *)ulTempAddr) = pOSDData[i*(ulWidth/2)+j];
            ulTempAddr++;
        }
    }

    FillPallete();

    SetOSDReg(nCRTIndex,
    OSD_PHY_BASE, //Physical
    &OSDInfo);

    EnableOSD(nCRTIndex, TRUE);
}
#endif

void setup_osd(struct s_crt_drv *crt, int nCRTIndex, unsigned int pattern_phy_addr, OSDInfo *pOSDInfo)
{
	unsigned int Value;
	unsigned int OSDOffset, OSDCount;
	unsigned int CRTOffset = nCRTIndex*0x60;

	Value = (pOSDInfo->HStart&OSD_HOR_START_MASK)|((pOSDInfo->HEnd<<OSD_HOR_END_BIT)&OSD_HOR_END_MASK);
	SetSCUReg((CRT1_OSD_HORIZONTAL_REG+CRTOffset), Value);

	Value = (pOSDInfo->VStart&OSD_VER_START_MASK)|((pOSDInfo->VEnd<<OSD_VER_END_BIT)&OSD_VER_END_MASK);
	SetSCUReg((CRT1_OSD_VERTICAL_REG+CRTOffset), Value);

	SetSCUReg(CRT1_OSD_ADDRESS_REG+CRTOffset, pattern_phy_addr);

#if (OSD_FROM_GUI == 3)
	// engine fetch count for each line. AST1520 fetch 128bits one at a time.
	OSDCount = (((pOSDInfo->HEnd - pOSDInfo->HStart)<<1) + 0xF)>>4;
#else
	// memory offset of each line
	//OSDOffset = (pOSDInfo->HEnd - pOSDInfo->HStart)/2;
	// pixel count for each line
	OSDCount = (pOSDInfo->HEnd - pOSDInfo->HStart + 0xF)>>4;
#endif
	// memory offset of each line (in bytes)
	OSDOffset = pOSDInfo->OSDBufLineOffset;
	Value = (OSDOffset&OSD_OFFSET_MASK) | ((OSDCount<<OSD_TERMINAL_BIT)&OSD_TERMINAL_MASK);
	SetSCUReg(CRT1_OSD_OFFSET_REG+CRTOffset, Value);

	Value = (OSD_THRES_LOW_VALUE&OSD_THRES_LOW_MASK)|
	        ((OSD_THRES_HIGH_VALUE<<OSD_THRES_HIGH_BIT)&OSD_THRES_HIGH_MASK)|
	        ((OSD_ALPHA_VALUE<<OSD_ALPHA_BIT)&OSD_ALPHA_BIT_MASK);

#if (OSD_FROM_GUI == 3)
	Value |= OSD_16BITS_MODE_EN;
	if (crt->OSD_force_2_color & 0x2) {
		Value |= OSD_16BITS_TRNSPRNT_EN;
		SetSCUReg(CRT2_OSD_TRANSPRNT_REG, OSD_TRNSPRNT_DATA(crt->OSD_force_2_color >> 16));
	}
#endif

	SetSCUReg(CRT1_OSD_THRESHOLD_REG+CRTOffset, Value);

#if (CONFIG_AST1500_SOC_VER >= 2)
	if (ast_scu.ability.v_interlace_mode) {
		if (GetSCUReg(CRT1_CONTROL_REG + CRTOffset) & ENABLE_CRT_INTERLACE_TIMING_MASK) {
			//Under Interlace Mode.
			ModSCUReg(CRT2_OSD_ADDRESS_REG, BIT1, BIT1);
		} else {
			ModSCUReg(CRT2_OSD_ADDRESS_REG, 0, BIT1);
		}
	}
#endif
}

#if OSD_2_COLOR
#define ABS_U32(v) (((int)(v) < 0)?(-(v)):(v))
#define RGB_555_TO_R5(c) (((c) >> 10) & 0x1F)
#define RGB_555_TO_G5(c) (((c) >> 5) & 0x1F)
#define RGB_555_TO_B5(c) (((c) >> 0) & 0x1F)

void to_2_color_palette(unsigned short *pPalette)
{
	unsigned short color = pPalette[0];
	unsigned short *p;
	unsigned int max_diff = 0;
	int max_i = 1, i;
	unsigned int r, g, b;

	r = RGB_555_TO_R5(color);
	g = RGB_555_TO_G5(color);
	b = RGB_555_TO_B5(color);

	p = &(pPalette[1]);
	for (i = 1; (i < PALETTE_SIZE) && (*p != 0); i++, p++) {
		unsigned int R, G, B;
		unsigned int diff;

		R = RGB_555_TO_R5(*p);
		G = RGB_555_TO_G5(*p);
		B = RGB_555_TO_B5(*p);
		R = ABS_U32(R-r);
		G = ABS_U32(G-g);
		B = ABS_U32(B-b);
		diff = R*R + G*G + B*B;

		//printk("p[%d]=0x%04X, diff=%d\n", i, *p, diff);
		if (diff > max_diff) {
			max_diff = diff;
			max_i = i;
		}
	}
	//printk("max_i=%d\n", max_i);
	color = pPalette[max_i];
	p = &(pPalette[1]);
	for (i = 1; (i < PALETTE_SIZE) && (*p != 0); i++, p++) {
		*p = color;
	}
}
#endif

void setup_pallete(unsigned short *pPalette, unsigned int force_2_color)
{
    unsigned int i;
    unsigned int PaletteData, PaletteIndex=0;

#if OSD_2_COLOR
	if (force_2_color)
		to_2_color_palette(pPalette);
#endif
	//Set palette[0] as transparent.
	pPalette[0] &= 0x7FFF;

	for (i = 0; i < 32; i += 4) {
		PaletteData = ((unsigned int)pPalette[PaletteIndex+1] << 16) & 0xFFFF0000;
		PaletteData |= ((unsigned int)pPalette[PaletteIndex]) & 0x0000FFFF;
		SetSCUReg(OSD_PALETTE_INDEX_BASE + i, PaletteData);
		PaletteIndex += 2;
	}
}

#if (OSD_FROM_GUI && OSD_FIX_PALETTE_COLOR)
static inline unsigned short RGB_555_TO_16(unsigned short s)
{
	unsigned short t;
	//unsigned short r, g, b;

	//B
	t = ((s >> 4) & 1)*(0x1F);
	//t |= ((((s >> 8) & 0x3) * 10) + 1) << 5;
	//R
	t |= (((s >> 14) & 1)*(0x1F)) << 10;
	//G need special care
	switch ((s >> 8) & 0x3)
	{
	case 0:
		break;
	case 1:
		t |= 10 << 5;
		break;
	case 2:
		t |= 20 << 5;
		break;
	case 3:
		t |= 31 << 5;
		break;
	default:
		break;
	}
/*
	r = (s & (1<<14)) ? (0x1F):(0);
	b = (s & (1<<4)) ? (0x1F):(0);
	g = ((s & (0x3<<8)) >> 8) * 10;

	t = (r << 10) | (g << 5) | b;
*/
	return t;
}
#endif


static inline unsigned int is_palette_match
	(unsigned short PaletteColor,
	unsigned short MatchColor,
	unsigned int force_2_color)
{
#if OSD_2_COLOR
	/*
	** For 2 color OSD case, we try to save all different colors and play the tricky in setup_pallete()
	*/
	if (force_2_color) {
		if (PaletteColor == MatchColor)
			return 1;
		return 0;
	}
#endif
	/*
	** RGB555 colors will be groups into RGB121 16 colors
	*/
	// Exclude pure white color
	if ((PaletteColor == 0x7FFF) && (PaletteColor != MatchColor))
		return 0;
	// Exclude pure black color
	if ((PaletteColor == 0x0000) && (PaletteColor != MatchColor))
		return 0;

	if ((PaletteColor & (1<<14)) != (MatchColor & (1<<14)))
		return 0;

	if ((PaletteColor & (1<<4)) != (MatchColor & (1<<4)))
		return 0;

	if ((PaletteColor & (0x3<<8)) != (MatchColor & (0x3<<8)))
		return 0;

	return 1;
}

/*
** Try to find the first matched index in palette and return its index.
** If not found, create it.
** If all 16 colors are used, return 0 (the transparent color).
** The first created color will always be transparent color.
*/
static short find_palette_idx
	(unsigned short *pPalette,
	unsigned short pixel,
	unsigned int force_2_color)
{
	int i;

	// ignore the MSB.
	//pixel &= 0x7FFF;
#if OSD_FIX_PALETTE_COLOR
	pixel = RGB_555_TO_16(pixel);
#endif
	//transparent bit will be used as valid bit before setup_pallete().
	pixel |= 0x8000;

	for (i = 0; i < PALETTE_SIZE; i++) {
		if (pPalette[i] == 0) {
			pPalette[i] = pixel;
			return i;
		}
#if !OSD_FIX_PALETTE_COLOR
		else if (is_palette_match((pPalette[i] & 0x7FFF), (pixel & 0x7FFF), force_2_color))
		{
			return i;
		}
#else
		else if ((pPalette[i] & 0x7FFF) == (pixel & 0x7FFF))
		{
			return i;
		}
#endif
	}

#if OSD_2_COLOR
	/* 2 color case should always set redundent colors as visible. */
	if (force_2_color)
		return 1;
#endif
	return 0;
}

#if OSD_FROM_GUI
static inline unsigned short RGB_888_TO_555(unsigned int s)
{
	unsigned short t;
	t = (unsigned short)((((unsigned int)(s) & 0x000000F8)) >> 3);
	t |= (unsigned short)((((unsigned int)(s) & 0x0000F800)) >> 6);
	t |= (unsigned short)((((unsigned int)(s) & 0x00F80000)) >> 9);
	return t;
}

void draw_osd_from_gui(
	struct s_crt_drv *crt,
	int nCRTIndex,
	unsigned int res_x,
	unsigned int res_y)
{
	unsigned char *data;
	unsigned int width, height;
	unsigned char *vbuf = TO_CRT_OSD_BASE(crt->vbuf_va);
	unsigned int x, y;
	unsigned int rowsize;
	unsigned int data_rowsize;
	unsigned char *vbuf_next_line;
	unsigned char *data_next_line;
	//unsigned int bpp = 16;
	unsigned short palette[PALETTE_SIZE];
	OSDInfo OSDInfo;
	unsigned int force_2_color = crt->OSD_force_2_color;
	unsigned int CenterAlign = 1;
	unsigned int y_offset = crt->OSD_y_offset;

	if (crt->OSD_y_offset == -1) {
		CenterAlign = 0;
		y_offset = 0;
	}
	//console frame buffer rowsize
	width = crt->CON_mode.width;
	height = crt->OSD_y_size;
	if ((y_offset + height) > crt->CON_mode.height)
		height = crt->CON_mode.height - y_offset;

	//console frame buffer will be translated into OSD frame buffer.
	data = TO_CRT_CONSOLE_BASE(crt->vbuf_va);
	//ASSUME consle is always 32bits per pixel, so, it is 4 bytes per pixel.
	data += ((width << 2) * y_offset);
	data_rowsize = width << 2;

	/* If we enabled video wall */

	if (res_x > width && CenterAlign) {
		OSDInfo.HStart  = (res_x-width) >> 1;
		OSDInfo.HEnd    = OSDInfo.HStart+width;
	} else {
		OSDInfo.HStart  = 0;
		OSDInfo.HEnd    = (res_x > width)?width:res_x;
		width = OSDInfo.HEnd;
	}
	if (res_y > height && CenterAlign) {
		OSDInfo.VStart	= (res_y-height) >> 1;
		OSDInfo.VEnd	= OSDInfo.VStart+height;
	} else {
		OSDInfo.VStart	= 0;
		OSDInfo.VEnd	= (res_y > height)?height:res_y;
		height = OSDInfo.VEnd;
	}

	//rowsize = ((width * bpp + (bpp-1))/bpp)/2; // 1/2 byte per pixel
	/*
	** OSD engine fetch 16 pixels one at a time. So the x pixels must be 16 align.
	** And 1/2 bytes per pixel.
	*/
	rowsize = (((width + 0xF)>>4)<<4)>>1;
	//vbuf += (height - 1) * rowsize;
	vbuf_next_line = vbuf + rowsize;
	data_next_line = data + data_rowsize;

	memset(palette, 0x0, sizeof(unsigned short)*PALETTE_SIZE);

	// XRGB1555 format. For Little-endian CPU only.
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x+=2) {
			unsigned short pixel_l, pixel_h;

			pixel_l = RGB_888_TO_555(*(unsigned int*)data);
			data += sizeof(unsigned int);
			pixel_h = RGB_888_TO_555(*(unsigned int*)data);
			data += sizeof(unsigned int);

			pixel_l = find_palette_idx(palette, pixel_l, force_2_color);
			pixel_h = find_palette_idx(palette, pixel_h, force_2_color);

			*(unsigned char*)vbuf = (unsigned char)(pixel_h << 4 | pixel_l);
			vbuf += sizeof(unsigned char);
		}
		//next line
		vbuf = vbuf_next_line;
		vbuf_next_line = vbuf + rowsize;
		data = data_next_line;
		data_next_line = data + data_rowsize;
	}

	OSDInfo.OSDBufLineOffset = rowsize;
	setup_pallete(palette, force_2_color);
	setup_osd(crt, nCRTIndex, OSD_PHY_BASE, &OSDInfo);

	EnableOSD(nCRTIndex, 1);
}
#endif //#if OSD_FROM_GUI

#if (CONFIG_AST1500_SOC_VER >= 3)
static void osd_position_overwrite(
	struct s_crt_drv *crt,
	POSDInfo pOSDInfo,
	unsigned int res_x,
	unsigned int res_y,
	unsigned int width,
	unsigned int height)
{
	int HStart, HEnd, VStart, VEnd;

	switch (crt->OSD_pos_align) {
		case OSD_POS_TL: //        1  //top-left
			HStart  = 0;
			HEnd    = width;
			VStart	= 0;
			VEnd	= height;
			break;
		case OSD_POS_T: //         2  //top
			HStart  = (res_x - width) >> 1;
			HEnd    = HStart + width;
			VStart	= 0;
			VEnd	= height;
			break;
		case OSD_POS_TR: //        3  //top-right
			HStart  = (res_x - width);
			HEnd    = HStart + width;
			VStart	= 0;
			VEnd	= height;
			break;
		case OSD_POS_L: //         4  //left
			HStart  = 0;
			HEnd    = width;
			VStart	= (res_y - height) >> 1;
			VEnd	= VStart + height;
			break;
		case OSD_POS_C: //         5  //center
			HStart  = (res_x - width) >> 1;
			HEnd    = HStart + width;
			VStart	= (res_y - height) >> 1;
			VEnd	= VStart + height;
			break;
		case OSD_POS_R: //         6  //right
			HStart  = (res_x - width);
			HEnd    = HStart + width;
			VStart	= (res_y - height) >> 1;
			VEnd	= VStart + height;
			break;
		case OSD_POS_BL: //        7  //bottom-left
			HStart  = 0;
			HEnd    = width;
			VStart	= (res_y - height);
			VEnd	= VStart + height;
			break;
		case OSD_POS_B: //         8  //bottom
			HStart  = (res_x - width) >> 1;
			HEnd    = HStart + width;
			VStart	= (res_y - height);
			VEnd	= VStart + height;
			break;
		case OSD_POS_BR: //        9  //bottom-right
			HStart  = (res_x - width);
			HEnd    = HStart + width;
			VStart	= (res_y - height);
			VEnd	= VStart + height;
			break;
		case OSD_POS_DEFAULT:
		default:
			HStart = pOSDInfo->HStart;
			HEnd = pOSDInfo->HEnd;
			VStart = pOSDInfo->VStart;
			VEnd = pOSDInfo->VEnd;
			break;
	}

	HStart += crt->OSD_pos_x_offset;
	HEnd += crt->OSD_pos_x_offset;
	VStart += crt->OSD_pos_y_offset;
	VEnd += crt->OSD_pos_y_offset;

	/* Keep OSD FB position in output timing boundary. */
	if (HStart < 0) {
		HStart = 0;
		HEnd = width;
	}
	if (HStart > res_x) {
		HStart = res_x - width;
		HEnd = HStart + width;
	}
	if (VStart < 0) {
		VStart = 0;
		VEnd = height;
	}
	if (VStart > res_y) {
		VStart = res_y - height;
		VEnd = VStart + height;
	}
	if (HEnd > res_x)
		HEnd = res_x;
	if (VEnd > res_y)
		VEnd = res_y;

	pOSDInfo->HStart = HStart;
	pOSDInfo->HEnd = HEnd;
	pOSDInfo->VStart = VStart;
	pOSDInfo->VEnd = VEnd;
#if 0
	uinfo("OSD HS:%d, HE:%d, VS:%d, VE:%d\n",
	      pOSDInfo->HStart,
	      pOSDInfo->HEnd,
	      pOSDInfo->VStart,
	      pOSDInfo->VEnd);
#endif
}

void draw_osd_16bits(
	struct s_crt_drv *crt,
	int nCRTIndex,
	unsigned int res_x,
	unsigned int res_y,
	unsigned int osd_num,
	u32 in_console)
{
	unsigned int width, height;
	OSDInfo OSDInfo;
	unsigned int CenterAlign = 1;
	unsigned int y_offset = crt->OSD_y_offset;
	u32 pattern_phy_addr;

	if (in_console)
		pattern_phy_addr = CRT_CONSOLE_PHY_BASE;
	else
		pattern_phy_addr = OSD_PHY_BASE;

	if (crt->OSD_y_offset == -1) {
		CenterAlign = 0;
		y_offset = 0;
	}
	//OSD frame buffer rowsize
	width = crt->CON_mode.width;
	height = crt->OSD_y_size;
	if ((y_offset + height) > crt->CON_mode.height)
		height = crt->CON_mode.height - y_offset;

	/* About OSDInfo.OSDBufLineOffset:
	** We assume OSD frame buffer is always RGB565 16bits format.
	** Which means 2 bytes per pixel. So, line offset will be (width * 2) bytes.
	*/
	OSDInfo.OSDBufLineOffset = width << 1;

#if (CONFIG_AST1500_SOC_VER >= 2)
	if (ast_scu.ability.v_interlace_mode) {
		if (GetSCUReg(CRT1_CONTROL_REG + nCRTIndex*0x60) & ENABLE_CRT_INTERLACE_TIMING_MASK) {
			//Under Interlace Mode.
			height >>= 1;
		}
	}
#endif

	/* If we enabled video wall */
	if (res_x > width && CenterAlign) {
		OSDInfo.HStart  = (res_x-width) >> 1;
		OSDInfo.HEnd    = OSDInfo.HStart+width;
	} else {
		OSDInfo.HStart  = 0;
		OSDInfo.HEnd    = (res_x > width)?width:res_x;
	}
	if (res_y > height && CenterAlign) {
		OSDInfo.VStart	= (res_y-height) >> 1;
		OSDInfo.VEnd	= OSDInfo.VStart+height;
	} else {
		OSDInfo.VStart	= 0;
		OSDInfo.VEnd	= (res_y > height)?height:res_y;
	}

	/* OSD position overwrite. */
	osd_position_overwrite(crt, &OSDInfo, res_x, res_y, width, height);

	if (0x1 & crt->OSD_force_2_color) {
		/* let color of first pxiel be transparent  */
		u16 bg_color;

		if (OSD_PHY_BASE == pattern_phy_addr)
			bg_color = *((u16 *)(TO_CRT_OSD_BASE(crt->vbuf_va)));
		else
			bg_color = *((u16 *)(TO_CRT_CONSOLE_BASE(crt->vbuf_va)));

		crt->OSD_force_2_color |= 0x2;
		crt->OSD_force_2_color &= ~(0xFFFF0000);
		crt->OSD_force_2_color |= (((u32)bg_color) << 16);
	}

	/*
	** Bruce150910. To avoid wrong OSD image, OSD must config during blank zone.
	** To simplify the implementation, we use a wait queue to wait in this thread.
	** This thread will wait until next VSync interrupt (ack_crt_vsync_int_stat_atomic()).
	** Technically, OSD should be config in VSync interrupt. But from my test,
	** config in this thread after vsync interrupt work fine. And we don't need
	** to worry about adding extra vsync interrupt process time.
	**
	** I use a 60ms timeout to ignore any corner cases that vsync interrupt
	** never got fired after wait.
	*/
	/*
	** Bruce151221. Current VSync interrupt is established in VE driver.
	** We have to make sure VSync interrupt is running before waiting for it.
	*/
	if (GetSCUReg(CRT1_CONTROL_REG + nCRTIndex*0x60) & INT_ON_MASK) {
		crt->osd_wait_done = 0;
		wait_event_interruptible_timeout(crt->osd_wait_queue, crt->osd_wait_done, msecs_to_jiffies(60));
	}

	setup_osd(crt, nCRTIndex, pattern_phy_addr, &OSDInfo);

	EnableOSD(nCRTIndex, 1);
}
#endif

void draw_osd(
	struct s_crt_drv *crt,
	int nCRTIndex,
	unsigned int res_x,
	unsigned int res_y,
	unsigned int osd_num)
{
	//Load BMP file. I use .h file as solution.
	struct s_bmp_file *bmp;
	struct s_bmp_file_hdr *hdr = &bmp->file_hdr;
	struct s_bmp_dib_hdr *dib = &bmp->dib_hdr;
	unsigned char *data;
	unsigned int width, height;
	unsigned char *vbuf = TO_CRT_OSD_BASE(crt->vbuf_va);
	unsigned int x, y;
	unsigned int rowsize;
	unsigned char *vbuf_next_line;
	unsigned int bpp = 16;
	unsigned short palette[PALETTE_SIZE];
	unsigned int nOSDNum;
	OSDInfo OSDInfo;
	unsigned int force_2_color = crt->OSD_force_2_color;

    //nOSDNum = sizeof(OSDDATA)/sizeof(OSDDATA[0]);
	nOSDNum = OSD_DATA_NUM;

	if (osd_num >= nOSDNum) {
        uerr("osd_num[%d] is over OSDNum[%d]\n",(u32)osd_num, nOSDNum);
        EnableOSD(nCRTIndex, 0);
        return;
	}
    bmp = (struct s_bmp_file *)OSDDATA[osd_num];
	hdr = &bmp->file_hdr;
	dib = &bmp->dib_hdr;

	//validate the osd bmp file. MUST be 16bit 5:5:5 format.
	if (hdr->identity != 0x4D42) {
		uerr("It is not BMP file!?\n");
		BUG();
	}
	if (dib->bit_per_pixel != bpp) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->compression != 3 /*BI_BITFILDS*/&& dib->compression != 0) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->width < 0 || dib->height < 0) {
		uerr("Not support %i, %i\n", dib->width, dib->height);
		BUG();
	}
	if (dib->width & 0xF) {
		uerr("BMP width MUST be multiple of 16 (%i)\n", dib->width);
		BUG();
	}
	data = ((unsigned char*)(bmp)) + hdr->data_offset;
	width = dib->width;
	height = dib->height;

    OSDInfo.HStart  = (res_x-width)/2;
    OSDInfo.HEnd    = OSDInfo.HStart+width;
    OSDInfo.VStart  = (res_y-height)/2;
    OSDInfo.VEnd    = OSDInfo.VStart+height;

	rowsize = ((width * bpp + (bpp-1))/bpp)/2; // 1/2 byte per pixel
	vbuf += (height - 1) * rowsize;
	vbuf_next_line = vbuf - rowsize;

	memset(palette, 0, sizeof(unsigned short)*PALETTE_SIZE);

	// XRGB1555 format. For Little-endian CPU only.
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x+=2) {
			unsigned short pixel_l, pixel_h;

			pixel_l = *(unsigned short*)data;
			data += sizeof(unsigned short);
			pixel_h = *(unsigned short*)data;
			data += sizeof(unsigned short);

			pixel_l = find_palette_idx(palette, pixel_l, force_2_color);
			pixel_h = find_palette_idx(palette, pixel_h, force_2_color);

			*(unsigned char*)vbuf = (unsigned char)(pixel_h << 4 | pixel_l);
			vbuf += sizeof(unsigned char);
		}
		//next line
		vbuf = vbuf_next_line;
		vbuf_next_line = vbuf - rowsize;
	}

	setup_pallete(palette, force_2_color);
	setup_osd(crt, nCRTIndex, OSD_PHY_BASE, &OSDInfo);

	EnableOSD(nCRTIndex, 1);
}


void OSDVideoOnHandler(struct s_crt_drv *crt)
{
	u32 ulCRTOffset;
	u32 ulValue;
	u32 ulWidth, ulHeight;
	u32 setting = crt->OSD_VideoOn_Setting;

	down(&crt->crt_lock);

	ulCRTOffset = crt->disp_select * 0x60;
	ulWidth 	= ((GetSCUReg(CRT1_HOR_TOTAL_END_REG + ulCRTOffset) & HOR_ENABLE_END_MASK) >> HOR_ENABLE_END_BIT) + 1;
	ulHeight	= ((GetSCUReg(CRT1_VER_TOTAL_END_REG + ulCRTOffset) & VER_ENABLE_END_MASK) >> VER_ENABLE_END_BIT) + 1;

	/* Bruce140813. For scaling up case.
	** OSD is processed before scaling up, which means OSD will also be scaled up.
	** We need the 'src' resolution for OSD calculation.
	*/
	if (crt->disp_select == CRT_2) {
		u32 sf;
		//scale x bit[4]
		if (GetSCUReg(CRT1_CONTROL_REG + ulCRTOffset) & EN_X_SCALE_UP)
		{
			sf = X_SCALE_UP_FACTOR_L_GET(GetSCUReg(CRT2_SCALE_UP_FACTOR))
				| X_SCALE_UP_FACTOR_H_GET(GetSCUReg(CRT2_THRESHOLD_REG));
			//(Scaling Factor) = (Src*CRT_SCALE_FACTOR_MAX)/Target
			ulWidth = (sf * ulWidth) / CRT_SCALE_FACTOR_MAX;
		}
#if (CONFIG_AST1500_SOC_VER != 2)
		//scale y bit[5]
		if (GetSCUReg(CRT1_CONTROL_REG + ulCRTOffset) & EN_Y_SCALE_UP)
		{
			sf = Y_SCALE_UP_FACTOR_L_GET(GetSCUReg(CRT2_SCALE_UP_FACTOR))
				| Y_SCALE_UP_FACTOR_H_GET(GetSCUReg(CRT2_THRESHOLD_REG));
			//(Scaling Factor) = (Src*CRT_SCALE_FACTOR_MAX)/Target
			ulHeight = (sf * ulHeight) / CRT_SCALE_FACTOR_MAX;
		}
#endif
	}

	if (OSD_DISPLAY_ENABLE == (setting & OSD_DISPLAY_MASK))
	{
		if (crt->current_owner == OWNER_NONE || crt->current_owner != OWNER_VE1) {
			//uinfo("Don't show OSD under non-decode screen\n");
			goto out;
		}
		//uinfo("OSDVideoOnHandler: Enable osd \n");
		ulValue = ((setting & OSD_BITMAP_MASK)>>OSD_BITMAP_BIT);

#if (OSD_FROM_GUI == 3)
		draw_osd_16bits(crt, crt->disp_select, ulWidth, ulHeight, ulValue, 1);
		/*
		** EnQ again if OSD is still in enable state.
		** Bruce151221. FIXME. Actually, only the background transparent color
		** is the concern under 16bits OSD case. Someday, we should add an extra
		** function to ONLY check and update background transparent color.
		** For now, we just queue this work again.
		*/
		if (OSD_DISPLAY_ENABLE == (crt->OSD_VideoOn_Setting & OSD_DISPLAY_MASK))
			queue_delayed_work(crt->wq, &crt->OsdVideoOnWork, msecs_to_jiffies(500));

#elif (OSD_FROM_GUI)
		if (ulValue == OSD_FROM_GUI_NUM) {
			draw_osd_from_gui(crt, crt->disp_select, ulWidth, ulHeight);
			// EnQ again if OSD is still in enable state.
			if (OSD_DISPLAY_ENABLE == (crt->OSD_VideoOn_Setting & OSD_DISPLAY_MASK))
			{
				queue_delayed_work(crt->wq, &crt->OsdVideoOnWork, msecs_to_jiffies(500));
			}
		} else {
			draw_osd(crt, crt->disp_select, ulWidth, ulHeight, ulValue);
		}
#else
		//ShowOSD(crt, nCRT, ulWidth, ulHeight, ulValue);
		draw_osd(crt, crt->disp_select, ulWidth, ulHeight, ulValue);
#endif
		ulValue = ((setting & OSD_SET_ALPHA_MASK)>>OSD_SET_ALPHA_BIT);
		ModSCUReg(CRT1_OSD_THRESHOLD_REG+ulCRTOffset, (ulValue<<OSD_ALPHA_BIT), OSD_ALPHA_BIT_MASK);
	}
	else
	{
		uinfo("OSDVideoOnHandler: Disable osd\n");
		EnableOSD(crt->disp_select, 0);
	}

out:
	up(&crt->crt_lock);
	return;
}


#if (3 == CONFIG_AST1500_SOC_VER)
/* from OSDVideoOnHandler() */
void LogoVideoOnHandler(struct s_crt_drv *crt)
{
	u32 ulCRTOffset;
	u32 ulValue;
	u32 ulWidth, ulHeight;
	u32 setting = crt->Logo_Setting;

	ulCRTOffset = crt->disp_select * 0x60;
	ulWidth = crt->CON_mode.width;
	ulHeight = crt->CON_mode.height;

	if (crt->disp_select == CRT_2) {
		u32 sf;
		/* scale x bit[4] */
		if (GetSCUReg(CRT1_CONTROL_REG + ulCRTOffset) & EN_X_SCALE_UP) {
			sf = X_SCALE_UP_FACTOR_L_GET(GetSCUReg(CRT2_SCALE_UP_FACTOR))
				| X_SCALE_UP_FACTOR_H_GET(GetSCUReg(CRT2_THRESHOLD_REG));
			ulWidth = (sf * ulWidth) / CRT_SCALE_FACTOR_MAX;
		}
		/* scale y bit[5] */
		if (GetSCUReg(CRT1_CONTROL_REG + ulCRTOffset) & EN_Y_SCALE_UP) {
			sf = Y_SCALE_UP_FACTOR_L_GET(GetSCUReg(CRT2_SCALE_UP_FACTOR))
				| Y_SCALE_UP_FACTOR_H_GET(GetSCUReg(CRT2_THRESHOLD_REG));
			ulHeight = (sf * ulHeight) / CRT_SCALE_FACTOR_MAX;
		}
	}

	if (OSD_DISPLAY_ENABLE == (setting & OSD_DISPLAY_MASK)) {
		ulValue = ((setting & OSD_BITMAP_MASK)>>OSD_BITMAP_BIT);

#if defined(LOGO_ON_OSD_BUFFER)
		draw_osd_16bits(crt, crt->disp_select, crt->CON_mode.width, crt->CON_mode.height, ulValue, 0);
#else
		draw_osd_16bits(crt, crt->disp_select, crt->CON_mode.width, crt->CON_mode.height, ulValue, 1);
#endif
		ulValue = ((setting & OSD_SET_ALPHA_MASK)>>OSD_SET_ALPHA_BIT);
	} else {
		ulValue = (GetSCUReg(CRT1_OSD_THRESHOLD_REG + ulCRTOffset) & 0x1F0000) >> OSD_ALPHA_BIT;

		if (0 < ulValue) {
			ulValue--;
		} else {
			crt->Logo_Setting = 0;
			crt->OSD_VideoOn_Setting = 0;
			return;
		}
	}

	ModSCUReg(CRT1_OSD_THRESHOLD_REG + ulCRTOffset,
		(ulValue << OSD_ALPHA_BIT),
		OSD_ALPHA_BIT_MASK);

	cancel_delayed_work(&crt->LogoWork);
	schedule_delayed_work(&crt->LogoWork, msecs_to_jiffies(100));
}
#endif

void __crt_change_scale_factor(
		struct s_crt_drv *crt,
		unsigned int hor_scale,
		unsigned int ver_scale,
		unsigned int x_rem_pixel,
		unsigned int half_down_scale)
{
	u32 val1 = 0, val2 = 0, val3 = 0;
	unsigned int tc;
	struct s_crt_info *crt_info;

	/* Disable OSD. It is necessary to aviod OSD scales. */
	EnableOSD(crt->disp_select, 0);

	//uinfo("hor_scale = 0x%08X, ver_scale = 0x%08X\n", hor_scale, ver_scale);
	if (hor_scale < CRT_SCALE_FACTOR_MAX){//do horizontal scaling
		val1 |= EN_X_SCALE_UP;
		val2 |= X_SCALE_UP_FACTOR_L(hor_scale);
		val3 |= X_SCALE_UP_FACTOR_H(hor_scale);
		SetSCUReg(CRT2_X_SCALE_COEF_IDX0, 0x7654321f);
		SetSCUReg(CRT2_X_SCALE_COEF_IDX1, 0xfedcba98);
	}
	if (ver_scale < CRT_SCALE_FACTOR_MAX){//do vertical scaling
		val2 |= Y_SCALE_UP_FACTOR_L(ver_scale);
		val3 |= Y_SCALE_UP_FACTOR_H(ver_scale);
#if (CONFIG_AST1500_SOC_VER != 2)
		val1 |= EN_Y_SCALE_UP;
		SetSCUReg(CRT2_Y_SCALE_COEF_IDX0, 0x7654321f);
		SetSCUReg(CRT2_Y_SCALE_COEF_IDX1, 0xfedcba98);
#endif
	}

	ModSCUReg(CRT2_THRESHOLD_REG,
	                   val3,
	                   X_SCALE_UP_FACTOR_H_MASK|Y_SCALE_UP_FACTOR_H_MASK);

	SetSCUReg(CRT2_SCALE_UP_FACTOR, val2);

	/* jsut setup scale parameters if soc_op_mode is 2 (AST1510 mode) */
	if (2 == ast_scu.ability.soc_op_mode)
		return;

	switch (crt->current_owner) {
	case OWNER_CON:
		crt_info = &crt->CON_mode;
		break;
	case OWNER_VE1:
		crt_info = &crt->VE1_mode;
		break;
	default:
		crt_info = &crt->CON_mode;
		uerr("un-supported screen?!(%d)\n", crt->current_owner);
		break;
	}

	tc = crt_info->terminal_cnt;
	if ((val1 & EN_X_SCALE_UP) || (half_down_scale)) {
#if 0 /* Old method. New method should be able to cover all the cases. */
		/*
		** Bruce141210. New approach to get correct terminal count.
		** Since scale_factor = ((Hsrc - 1) * CRT_SCALE_FACTOR_MAX)/(Hdest - 1)
		** So, we can caculate Hsrc from scale_factor and Hdest.
		** FIXME:
		** This approach assumes Hdest always == CRT timing's HActive.
		** Under video wall case, this assumption may NOT be always correct
		** since Hdest may have extra pixels for 'monitor frame compensation'.
		*/
		u32 Hd, Hs;

		Hd = ((GetSCUReg(CRT2_HOR_TOTAL_END_REG) & HOR_ENABLE_END_MASK) >> HOR_ENABLE_END_BIT) + 1;
		Hs = ((hor_scale * (Hd - 1) + (CRT_SCALE_FACTOR_MAX >> 1)) >> CRT_SCALE_FACTOR_MAX_SHIFT_BITS) + 1;

		/*
		** Bruce150507. SoC V3 A1.
		** For 4K YUV 420 format, CRT engine have to fetch x1.5 more pixels.
		*/
		if (crt_info->crt_output_format == YUV420_FORMAT)
			Hs += (Hs >> 1);

		uinfo("Hs=%d, Hd=%d\n", Hs, Hd);

		tc = CALC_TERMINAL_CNT(Hs, 24);
		ModSCUReg(CRT2_DISPLAY_OFFSET, (tc << TERMINAL_COUNT_BIT), TERMINAL_COUNT_MASK);

#if (CONFIG_AST1500_SOC_VER >= 3)
		SetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT,
		          VEFLIP_YUV444_TC(tc)
		          | VEFLIP_YUV422_TC(CALC_TERMINAL_CNT(Hs, 16)));
#endif
#else
		/*
		** Bruce150526:
		** 'x_scale' and 'x_rem_pixel' only valid when video wall is used.
		*/
		crt_update_terminal_cnt(crt, crt_info, hor_scale, x_rem_pixel, half_down_scale);
#endif
	}

#if (CONFIG_AST1500_SOC_VER == 2)
	ModSCUReg(CRT2_CONTROL_REG, val1, EN_X_SCALE_UP);
#elif (CONFIG_AST1500_SOC_VER == 1)
	ModSCUReg(CRT2_CONTROL_REG, val1, (EN_X_SCALE_UP | EN_Y_SCALE_UP));
#else
	if (half_down_scale)
		val1 |= EN_HALF_DOWNSCALE;

	ModSCUReg(CRT2_CONTROL_REG, val1, (EN_X_SCALE_UP | EN_Y_SCALE_UP | HALF_DOWNSCALE_MASK));
#endif

	if (!((val1 & EN_X_SCALE_UP) || (half_down_scale))) {
		__mod_reg84(crt, (tc << TERMINAL_COUNT_BIT), TERMINAL_COUNT_MASK);

#if (CONFIG_AST1500_SOC_VER >= 3)
		SetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT,
		          VEFLIP_YUV444_TC(crt_info->terminal_cnt_yuv444)
		          | VEFLIP_YUV422_TC(crt_info->terminal_cnt_yuv422));
#endif

	}

	if (crt->ve_scale_callback) {
		(crt->ve_scale_callback)(0, ver_scale, crt->vw_rotate);
	}
}

void _crt_restore_scale_factor(struct s_crt_drv *crt, void *dummy)
{
	/* If we are not in decode screen, don't change. */
	if (crt->current_owner != OWNER_VE1)
		return;

#if (CONFIG_AST1500_SOC_VER != 2)
	if (crt->vw_enabled || (crt->vw_rotate != NO)) {
		_crt_vw_enable(crt, &crt->VE1_mode);
		return;
	}
#endif

	__crt_change_scale_factor(crt, crt->dec_hor_scale, crt->dec_ver_scale, 0, crt->dec_half_down_scale);
}

void crt_change_scale_factor(
		struct s_crt_drv *crt,
		unsigned int hor_scale,
		unsigned int ver_scale,
		unsigned int half_down_scale)
{
	down(&crt->crt_lock);
	/*
	** Bruce110905. Bug Fix.
	** The scaling factor should be saved no mather video wall is on or off.
	** Otherwise, once video wall is turned off, there is a chance that crt
	** don't know the correct scale factor.
	*/
	crt->dec_hor_scale = hor_scale;
	crt->dec_ver_scale = ver_scale;
	crt->dec_half_down_scale = half_down_scale;
	up(&crt->crt_lock);
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_restore_scale_factor, NULL, 0);
}

void crt_change_scale_factor_atomic(
		struct s_crt_drv *crt,
		unsigned int hor_scale,
		unsigned int ver_scale,
		unsigned int half_down_scale)
{
	/*
	** Bruce161219. FIXME.
	** This function is for soc_op_mode == 1 only.
	** Current profile doesn't use this code path anymore. (Dynamic down/up scale)
	** I don't have a better way to maintain this code path without taking
	** the risk of CRT engine driver race condition.
	** Since this code path is no more used. So I'm going to leave it as what it was.
	*/
	/*
	** Bruce110905. Bug Fix.
	** The scaling factor should be saved no mather video wall is on or off.
	** Otherwise, once video wall is turned off, there is a chance that crt
	** don't know the correct scale factor.
	*/
	crt->dec_hor_scale = hor_scale;
	crt->dec_ver_scale = ver_scale;
	crt->dec_half_down_scale = half_down_scale;

	/* If we are not in decode screen, don't change. */
	if (crt->current_owner != OWNER_VE1)
		return;

#if (CONFIG_AST1500_SOC_VER != 2)
	if (crt->vw_enabled || (crt->vw_rotate != NO)) {
		_crt_vw_enable(crt, &crt->VE1_mode);
		return;
	}
#endif

	__crt_change_scale_factor(crt, crt->dec_hor_scale, crt->dec_ver_scale, 0, crt->dec_half_down_scale);
}

#if 0//dispaly is now determined by Tx detection.
int crt_choose_display(struct s_crt_drv *crt)
{
	int select = OUTPUT_SELECT_NONE;

	if (crt->is_client)
	{
		/*
		** In V3 board, Analog and digital share the same connector and
		** HP pin. The only way to determine the digital/analog is to read
		** the edid. So, I left this job to crt_get_display_edid()
		*/
		//Check digital first
		crt->disp_select = OUTPUT_SELECT_DIGITAL;
		if (xHal_check_hotplug(crt)) {
			//uinfo("digital output\n");
			select = OUTPUT_SELECT_DIGITAL;
			goto ok;
		}
#if 0
		{//Then analog
			unsigned char chksum;

			crt->disp_select = OUTPUT_SELECT_ANALOG;
			if (!xHal_rd_edid_chksum(crt, &chksum)) {
				//uinfo("analog output\n");
				select = OUTPUT_SELECT_ANALOG;
				goto ok;
			}
		}
#else
		//Then analog
		crt->disp_select = OUTPUT_SELECT_ANALOG;
		if (xHal_check_hotplug(crt)) {
			//uinfo("analog output\n");
			select = OUTPUT_SELECT_ANALOG;
			goto ok;
		}
#endif
		// Both digital and analog output is not available.
		uinfo("Both digital and analog display is not available.\n");
		select = OUTPUT_SELECT_NONE;

	}
	else
	{
#if 1
		//Host. choose from jumper. (PTV1500 and AST1500 platform)
		gpio_direction_input(GPIO_HOST_VIDEO_INPUT_SELECT);
		if (gpio_get_value(GPIO_HOST_VIDEO_INPUT_SELECT)){
			//uinfo("select analog source\n");
			select = OUTPUT_SELECT_ANALOG;
		} else {
			//uinfo("select digital source\n");
			select = OUTPUT_SELECT_DIGITAL;
		}
#else
		//Check digital first
		crt->disp_select = OUTPUT_SELECT_DIGITAL;
		if (xHal_check_hotplug(crt)) {
			//uinfo("digital output\n");
			select = OUTPUT_SELECT_DIGITAL;
			goto ok;
		}
		{//Then analog
			unsigned char chksum;

			crt->disp_select = OUTPUT_SELECT_ANALOG;
			if (!xHal_rd_edid_chksum(crt, &chksum)) {
				//uinfo("analog output\n");
				select = OUTPUT_SELECT_ANALOG;
				goto ok;
			}
		}

		// Both digital and analog output is not available.
		uinfo("Both digital and analog display is not available.\n");
		select = OUTPUT_SELECT_NONE;

#endif
	}

ok:
	if (crt->is_client && !crt->display_attached && (select != OUTPUT_SELECT_NONE)) {
		if (select == OUTPUT_SELECT_ANALOG)
			uinfo("select analog\n");
		else
			uinfo("select digital\n");
	}

	crt->disp_select = select;

	if (crt->disp_select == OUTPUT_SELECT_NONE)
	{ //We still need to choose a default output.
#ifdef CONFIG_AST1500_ANALOG_SINK
		crt->disp_select = OUTPUT_SELECT_ANALOG;
#else
		crt->disp_select = OUTPUT_SELECT_DIGITAL;
#endif
	}

	return select;
}
#endif

#if 0
int crt_get_display_edid(struct s_crt_drv *crt)
{
	int select = OUTPUT_SELECT_NONE;
	unsigned char EDID[EDID_ROM_SIZE];

	if (!crt->is_client) {
		uerr("This code path should never happen!\n");
		return select;
	}
#ifdef CLIENT_DIGITAL_OUTPUT_ONLY
	if (!CheckHotPlug(CRT_1)) {
		uinfo("no digital display attached\n");
		goto out;
	}
	if (!clientReadEdid(OUTPUT_SELECT_DIGITAL, EDID)){
		uinfo("cannot read digital edid\n");
		goto out;
	}
	select = OUTPUT_SELECT_DIGITAL;
#else
#ifdef CLIENT_ANALOG_OUTPUT_ONLY
	if (!clientReadEdid(OUTPUT_SELECT_ANALOG, EDID)) {
		uinfo("no analog display attached\n");
		goto out;
	}
	uinfo("analog output\n");
	select = OUTPUT_SELECT_ANALOG;
#else
#if	defined(CONFIG_ARCH_AST1500) || (CONFIG_ARCH_PTV1500)
	if (!CheckHotPlug(CRT_1)) {
		//read analog EDID
		if (!clientReadEdid(OUTPUT_SELECT_ANALOG, EDID)) {
			uinfo("no analog display attached\n");
			goto out;
		}
		uinfo("analog output\n");
		select = OUTPUT_SELECT_ANALOG;
	} else { //There is a digital hotput
		if (!clientReadEdid(OUTPUT_SELECT_DIGITAL, EDID)){
			uinfo("cannot read digital edid\n");
			goto out;
		}
		uinfo("digital output\n");
		select = OUTPUT_SELECT_DIGITAL;
	}
#else //For V3 board
	/* In V3 board digital and analog share the same DDC path.  */
	if (!CheckHotPlug(CRT_1))
		goto out;

	if (!clientReadEdid(OUTPUT_SELECT_DIGITAL, EDID)){
		uinfo("cannot read digital edid\n");
		goto out;
	}
	if (EdidIsDigital(EDID)) {
		uinfo("digital output\n");
		select = OUTPUT_SELECT_DIGITAL;
	} else {
		uinfo("analog output\n");
		select = OUTPUT_SELECT_ANALOG;
	}
#endif
#endif
#endif

	memcpy(crt->edid, EDID, EDID_ROM_SIZE);

out:
#if !defined(CLIENT_DIGITAL_OUTPUT_ONLY) && !defined(CLIENT_ANALOG_OUTPUT_ONLY)
	crt->disp_select = select;
#endif
	return select;
}
#else
static unsigned int read_edid(struct s_crt_drv *crt, unsigned char *pEdid)
{
#if 0
	int	retry = 0;

start:
#endif
	//Clean up the whole EDID
	memset(pEdid, 0, EDID_ROM_SIZE);

	/*
	** Some Monitor's analog port only provide 128B of EEPROM.
	** So, we have to read 256B seperately.
	*/
	if (xHal_rd_edid(crt, 0, pEdid))
		goto fail;

	//verify header
	if( pEdid[0] != 0x00 ||
	    pEdid[1] != 0xFF ||
	    pEdid[2] != 0xFF ||
	    pEdid[3] != 0xFF ||
	    pEdid[4] != 0xFF ||
	    pEdid[5] != 0xFF ||
	    pEdid[6] != 0xFF ||
	    pEdid[7] != 0x00)
    {
		uerr("invalid EDID header\n");
		udump(pEdid, EDID_BLOCK_SIZE);
		goto fail;
    }

	//Verify checksum
	if (verify_edid_block_checksum(pEdid)) {
		uerr("EDID block 0 checksum failed\n");
		udump(pEdid, EDID_BLOCK_SIZE);
		goto fail;
	}

	//Does it have extention blocks?
	if (pEdid[EXT_EDID_BLOCK] == 0) {
		uinfo("no extention block\n");
		goto ok;
	}

	if (xHal_rd_edid(crt, 1, (pEdid + EDID_BLOCK_SIZE)))
		goto fail;

	//Verify checksum
	if (verify_edid_block_checksum(pEdid + EDID_BLOCK_SIZE)) {
		uinfo("EDID dump for debug:\n");
		udump(pEdid, EDID_ROM_SIZE);

		if (ast_scu.ability.v_support_4k >= 3)
			memcpy(pEdid, AST_EDID_HDMI_4K, EDID_ROM_SIZE);
		else
			memcpy(pEdid, AST_EDID_HDMI, EDID_ROM_SIZE);

		uerr("EDID block 1 checksum failed\n");
		//goto fail;
		goto ok; //Ignore the checksum fail of EDID block 1. Some monitors have this problem.
	}

	//Check CEA
	if (!EdidHasCEA(pEdid)) {
		uerr("Invalid CEA info?!\n");
		removeDupBlock(pEdid + EDID_BLOCK_SIZE);
		goto ok; //It should be harmless. goto ok.
	}

ok:
	uinfo("EDID read:\n");
	udump(pEdid, EDID_ROM_SIZE);
	return 0;
fail:
	uerr("Read EDID failed\n");
	udump(pEdid, EDID_ROM_SIZE);

#if 0
	retry++;
	if (retry < 5) {
		msleep(100);
		goto start;
	}
#endif

	return -ENODEV;
}

static void crt_edid_update(struct s_crt_drv *crt, u8 *edid)
{
	edid_status *r = &crt->edid_parse_result;

	memcpy(crt->edid, edid, EDID_ROM_SIZE);
	crt->ast_edid->parse(crt->edid, r);

	/* Convert to crt preferred timing index here so that we don't have to concern unsupported format after. */
	if (r->valid)
		r->preferred_timing_index = crt_timing_index_to_preferred_timing(r->preferred_timing_index);
	else
		r->preferred_timing_index = crt_timing_sn_to_index(AST_TIMING_SN_1080P60); /* FIXME: PAL/NTSC has different default. */

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	if (r->hdmi_1x)
		crt_cec_pa_cfg(crt, r->hdmi_physical_address);
#endif
}

#define EDID_ERROR_CNT_MAX 3

/*
This routine will be invoked:
1. during initialization (crt_host_init & crt_client_init)
2. during periodic hotplug check with analog Tx (_poll_hotplug)
3. receiving a hotplug event with digital Tx (crt_hotplug_tx)
4. for a retry if the previous invocation failed (crt_get_display_edid)
*/
static void crt_get_display_edid(struct s_crt_drv *crt)
{
	int do_hotplug = 0, plug_status, crt_display_attached = crt->display_attached;
	int retry = 0;
	unsigned char EDID[EDID_ROM_SIZE];
	unsigned char checksum = 0;
	unsigned char *default_edid;
	static int edid_error = 0, last_plug_status = 0;

	if (!crt->tx_exist)
		return;

	down(&crt->crt_lock);

	/* Do nothing if nobody cares. */
	if (crt->connected_screen == OWNER_NONE) {
		goto done;
	}

#if 0
	if (crt_choose_display(crt) == OUTPUT_SELECT_NONE) {
		if (crt->display_attached)
			goto on_detach;
		goto done;
	}
#endif

	/* If the xmiter doesn't support hotplug detect (ex: analog),
	** xHal_check_hotplug() should always return success so that
	** further default EDID can be used.
	*/
	/* In host loopback mode, display is not chosen by hotplug pin, so, double check again here. */
	plug_status = xHal_check_hotplug(crt);

	if (!plug_status) {
		uinfo("unplugged\n");
		if (crt_display_attached)
			goto on_detach;
		goto done;
#if (3 <= CONFIG_AST1500_SOC_VER)
	} else if (crt->dual_port & crt->dual_port_exist) {
		/* check plug status change of primary port */
		if ((0x1 & plug_status) && (!(0x1 & last_plug_status))) {
			if (xHal_get_xmiter_cap(crt) != xCap_RGB) {
				/* force HDMI to read EDID */
				crt_display_attached = 0;

				/*
				 * For hybrid mode, we have to setup HDMI transmitter for AVI info frame
				 * once HDMI got unplug => plug
				 */
				if (FTCFG_IS_ENABLED(crt->VE1_mode.force_timing.cfg))
					crt->op_state &= ~COPS_FORCING_TIMING;
			}
		}

		/* check plug status change of secondary port */
		if ((0x2 & plug_status) && (!(0x2 & last_plug_status))) {
			if (xHal_get_xmiter_cap_secondary(crt) != xCap_RGB) {

				/*
				 * For hybrid mode, we have to setup HDMI transmitter for AVI info frame
				 * once HDMI got unplug => plug
				 */
				if (FTCFG_IS_ENABLED(crt->VE1_mode.force_timing.cfg))
					crt->op_state &= ~COPS_FORCING_TIMING;
			}
		}

		if (!(0x1 & plug_status)) {
			/* only secondary tx exists, skip EDID reading */
			crt->edid_valid = 0;
			/* primary is unplugged, clear edid_error */
			edid_error = 0;
			goto edid_done;
		}
#endif
	}

	/*
	 * assign default_edid after sanity check and exception check
	 * once we get here, we may need default_edid
	 */
	if (xHal_get_xmiter_cap(crt) == xCap_RGB) {
		/* use default analog EDID */
		default_edid = AST_EDID_VGA;
	} else {
		/* use default DVI EDID */
		default_edid = AST_EDID_DVI;
	}

	//read checksum only to minimize overhead incurred
	if (xHal_rd_edid_chksum(crt, &checksum) || checksum == 0)
	{
		//printk("cannot read EDID checksum, use default.\n");
		checksum = default_edid[EDID_CHECKSUM_OFFSET];

		/*
		 * EDID is not necessary for VGA monitor
		 * we may get this kind of display device and always failed to read EDID
		 * so never patch plug_status when EDID is not avaiable on VGA monitor
		 */
#if 0
		if (xHal_get_xmiter_cap(crt) == xCap_RGB) {
			/* For VGA, failed to read checksum means no monitor attached */
			plug_status &= ~0x1;
		}
#endif
		retry = 1;

		if (xHal_get_xmiter_cap(crt) == xCap_RGB)
			edid_error = 0;
	}

	/*
	 * DO NOT try to read EDID if edid_error > EDID_ERROR_CNT_MAX
	 * this is for the special monitor which EDID is broken
	 * if fail to read EDID => edid_error++
	 * unplug => clear edid_error
	 */
	if (EDID_ERROR_CNT_MAX < edid_error)
		goto done;

	//printk("display attached\n");
	/* Bruce120912. An extra checksum check here is the key for VGA polling case
	** to detect unplug. Somebody removed this check on Alpha firmware for unknown
	** reason. Now I add it back.
	*/
	if ((!crt_display_attached) ||
		(checksum != crt->edid[EDID_CHECKSUM_OFFSET]))
	{
		if (read_edid(crt, EDID)) {
			/* Fail. still not attached. */
			uerr("Plugged but cannot read EDID! Use default.\n");
			if (memcmp(default_edid, crt->edid, EDID_ROM_SIZE)) {
				crt_edid_update(crt, default_edid);
				do_hotplug = 1;
			}
			retry = 1;
		} else {
			/* OK. Update EDID */
			crt_edid_update(crt, EDID);
			do_hotplug = 1;
		}
	}

#if (3 <= CONFIG_AST1500_SOC_VER)
	crt->edid_valid = 1;
edid_done:

#endif

	/*
	** Bruce160217.
	** crt->display_attached will be referenced in _crt_to_screen()::crt_setup().
	** crt->display_attached must be updated before calling _crt_to_screen().
	*/
	crt->display_attached = plug_status;

	if (!crt_display_attached)
	{
		uinfo("display attached!\n");

		if (!(crt->op_state & COPS_PWR_SAVE)) {
			_crt_to_screen(crt, OWNER_CURRENT);
		}
		do_hotplug = 1;
	}
	else if (crt->always_set_tx) {
		if (!(crt->op_state & COPS_PWR_SAVE)) {
#if (CONFIG_AST1500_SOC_VER >= 3)
			if (0 == crt->Logo_Setting)
#endif
			_crt_to_screen(crt, OWNER_CURRENT);
		}
		crt->always_set_tx = 0;
	}

	last_plug_status = plug_status;

done:
	if (do_hotplug) {
		if (tx_hotplug_callback == NULL)
			uerr("hotplg callback not registered!!!\n");
		else
			tx_hotplug_callback(crt->display_attached);
	}


	if (retry) { /* retry */
		queue_delayed_work(crt->xmiter_wq,
		                  &crt->ReadDisplayEdidWork,
		                  msecs_to_jiffies(3000));
		edid_error++;
	} else {
		xHal_poll_hotplug(crt);

#if defined(CONFIG_ARCH_AST1500_CLIENT)
		if (edid_error > EDID_ERROR_CNT_MAX) {
			/* corrupted EDID will impact CEC */
			uinfo("EDID is corrupted, CEC may not works\n");
		}

		/* notify CEC to re-build topology when EDID is done */
		if (tx_hotplug_cec_callback != NULL)
			tx_hotplug_cec_callback(crt->display_attached);
#endif
	}
	up(&crt->crt_lock);
	return;

on_detach:
	//Disable current xmiter before change to another.
	uinfo("display detached\n");
	_crt_disable_xmiter(crt);

	crt->display_attached = 0;
	if (tx_hotplug_callback != NULL)
		tx_hotplug_callback(0);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	if (tx_hotplug_cec_callback != NULL)
		tx_hotplug_cec_callback(0);
#endif

	xHal_poll_hotplug(crt);
	edid_error = 0;
	last_plug_status = 0;

	up(&crt->crt_lock);
	return;
}
#endif

//#if (CONFIG_AST1500_SOC_VER >= 2)
//static void setup_video_multipin(void)
//{
//	ModSCUReg(SCU_PIN_CTRL5_REG, 2 << DIGITAL_VIDEO_FUNCTION_PINS_ENABLE_SHIFT, DIGITAL_VIDEO_FUNCTION_PINS_ENABLE_MASK);
//	ModSCUReg(SCU_PIN_CTRL2_REG, 0xFF3E0000, 0xFF3E0000);
//	ModSCUReg(SCU_PIN_CTRL3_REG, 0x000FFFFC, 0x000FFFFC);
//}
//#endif

static void reset_crt_engine(struct s_crt_drv *crt)
{
#if (CONFIG_AST1500_SOC_VER == 1)
	int display = crt->disp_select;

	if (display == CRT_1)
	{
		//CRT1 DAC with single edge must disable CRT2 DVO and DAC
		SetSCUReg(CRT2_CONTROL2_REG, 0);
		ast_scu.scu_op(SCUOP_CRT_CRT1_INIT, NULL);
	}
	else
#endif
	{ //digital output or otherwise
		ast_scu.scu_op(SCUOP_CRT_CRT2_INIT, NULL);
	}

	//We should keep CRT engine in OFF state before we configure the video timing.
#if (CONFIG_AST1500_SOC_VER == 1)
	if (display == CRT_1)
	{
		ModSCUReg(CRT1_CONTROL_REG, GRAPH_DISPLAY_OFF, GRAPH_DISPLAY_MASK);
	}
	else
#endif
	{
		ModSCUReg(CRT2_CONTROL_REG, GRAPH_DISPLAY_OFF, GRAPH_DISPLAY_MASK);
#if (CONFIG_AST1500_SOC_VER >= 2)
		/*
		** Bruce110603. A patch suggested from Jenmin. Aims to resolve HW cursor issue.
		** The default state of "cursor enable" bit may be enable under some condition.
		** The patch initialized the "cursor enable" bit.
		*/
		SetSCUReg(CRT2_CONTROL_REG, 0);
		SetSCUReg(CRT2_CURSOR_POSITION, 0xFFFFFFFF); //a dummy value forces enable state update.
#endif

		//disable scaling first(CRT2 CR60 bit4,5) only CRT2
#if (CONFIG_AST1500_SOC_VER == 2)
		ModSCUReg(CRT2_CONTROL_REG, 0, EN_X_SCALE_UP);
		//Moved to below.
		//ModSCUReg(CRT2_THRESHOLD_REG, Y_SCALE_UP_FACTOR_H(0), Y_SCALE_UP_FACTOR_H_MASK);
#else
		ModSCUReg(CRT2_CONTROL_REG, 0, (EN_X_SCALE_UP|EN_Y_SCALE_UP));
#endif
		ModSCUReg(CRT2_THRESHOLD_REG,
		                   X_SCALE_UP_FACTOR_H(0)|Y_SCALE_UP_FACTOR_H(0),
		                   X_SCALE_UP_FACTOR_H_MASK|Y_SCALE_UP_FACTOR_H_MASK);
		SetSCUReg(CRT2_SCALE_UP_FACTOR, 0);
	}
	/* Bruce131024.
	** We found some AST1500 chip hangs on boot.
	** Set base address to a default value seems resolve this issue.
	** There is no explanation on this.
	*/
	_crt_set_vbuf(crt, CRT_CONSOLE_PHY_BASE);
}


unsigned int _crt_pwr_save(struct s_crt_drv *crt, int do_pwr_save)
{
	unsigned int reg_base = __reg_base(crt);

	if (do_pwr_save) {
		crt->op_state |= COPS_PWR_SAVE;
		/* Bruce121220. We only disable HSync for turnning off monitor because VE driver need
		** VSync to actively trigger interrupt (AST1500).
		** Bruce150825:
		** For SoC ver >= 2, VSync interrupt is generated by CRT, not GPIO (SoC V1).
		** So, VSync interrupt won't stop even we disable VSync pin output.
		** So, the only concern of VSync interrupt is SoC V1.
		** Bruce150924:
		** For SoC Ver == 1, stopping vsync int will causes VE stream buffer full.
		** The VE driver will just fire an stream error event and keep going on.
		** Looks like it is safe to disable VSync now.
		*/
		ModSCUReg(CRT1_CONTROL_REG + reg_base, HOR_SYNC_OFF_ENABLE | VER_SYNC_OFF_ENABLE, HOR_SYNC_OFF_MASK | VER_SYNC_OFF_MASK);

		/* Disable DVO pins. New 4K (Samsung, DELL) monitors need disable DE. */
#if (CONFIG_AST1500_SOC_VER == 1)
		if (crt->disp_select == CRT_1)
		{
			ast_scu.scu_op(SCUOP_CRT_CRT1_PIN, (void*)0);
		}
		else
#endif
		{
			ast_scu.scu_op(SCUOP_CRT_CRT2_PIN, (void*)0);
		}
		uinfo("CRT enable power save\n");
	} else {
#if 0
		/*
		** Bruce171024. RctBug#2017102400. Screen never come back after enterred power save.
		** COPS_PWR_SAVE is also cleared when crt_to_xxxx_screen() get called.
		** If we skip here, DVO pins will never have a chance to enable.
		**
		** FIXME.
		** Maybe we should remove COPS_PWR_SAVE in crt_to_xxxx_screen().
		** But I am afraid of breaking something.
		*/
		if (!(crt->op_state & COPS_PWR_SAVE)) {
			/*uerr("\n\n!!!!I'm not power saving. Should not touch DVO pins!!\n\n");*/
			return 0;
		}
#endif
		crt->op_state &= ~COPS_PWR_SAVE;

		if (!(crt->op_state & COPS_FORCE_SYNC_OFF)) {
			ModSCUReg(CRT1_CONTROL_REG + reg_base, HOR_SYNC_OFF_DISABLE | VER_SYNC_OFF_DISABLE, HOR_SYNC_OFF_MASK | VER_SYNC_OFF_MASK);

			/* Enable DVO pins. */
#if (CONFIG_AST1500_SOC_VER == 1)
			if (crt->disp_select == CRT_1)
			{
				ast_scu.scu_op(SCUOP_CRT_CRT1_PIN, (void*)1);
			}
			else
#endif
			{
				ast_scu.scu_op(SCUOP_CRT_CRT2_PIN, (void*)1);
			}
		}

		uinfo("CRT disable power save\n");
	}
	return 0;
}

unsigned int crt_pwr_save(struct s_crt_drv *crt, int do_pwr_save)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_pwr_save, (void *)do_pwr_save, 0);
	return 0;
}

void _crt_force_sync_off(struct s_crt_drv *crt, int on)
{
	unsigned int reg_base = __reg_base(crt);

	if (on) {
		crt->op_state |= COPS_FORCE_SYNC_OFF;
		/* Bruce121220. We only disable HSync for turnning off monitor because VE driver need
		** VSync to actively trigger interrupt (AST1500).
		** Bruce150825:
		** For SoC ver >= 2, VSync interrupt is generated by CRT, not GPIO (SoC V1).
		** So, VSync interrupt won't stop even we disable VSync pin output.
		** So, the only concern of VSync interrupt is SoC V1.
		** Bruce150924:
		** For SoC Ver == 1, stopping vsync int will causes VE stream buffer full.
		** The VE driver will just fire an stream error event and keep going on.
		** Looks like it is safe to disable VSync now. ==> Actually not, read below:
		** Bruce160622:
		** For SoC Ver == 1, stream buffer full will cause new driver to do check_on_stream_buf_full()
		** and it will trigger system reboot.
		** Solution:
		** We modify VE driver to check COPS_FORCE_SYNC_OFF and do_skip_frame()
		** when necessary. The VE driver is now safe without VSync interrupt.
		*/
		ModSCUReg(CRT1_CONTROL_REG + reg_base, HOR_SYNC_OFF_ENABLE | VER_SYNC_OFF_ENABLE, HOR_SYNC_OFF_MASK | VER_SYNC_OFF_MASK);

		/* Disable DVO pins. New 4K (Samsung, DELL) monitors need disable DE. */
#if (CONFIG_AST1500_SOC_VER == 1)
		if (crt->disp_select == CRT_1)
		{
			ast_scu.scu_op(SCUOP_CRT_CRT1_PIN, (void*)0);
		}
		else
#endif
		{
			ast_scu.scu_op(SCUOP_CRT_CRT2_PIN, (void*)0);
		}

		uinfo("CRT force screen off\n");
	} else {
		crt->op_state &= ~COPS_FORCE_SYNC_OFF;
		ModSCUReg(CRT1_CONTROL_REG + reg_base, HOR_SYNC_OFF_DISABLE | VER_SYNC_OFF_DISABLE, HOR_SYNC_OFF_MASK | VER_SYNC_OFF_MASK);

		/* Enable DVO pins. */
#if (CONFIG_AST1500_SOC_VER == 1)
		if (crt->disp_select == CRT_1)
		{
			ast_scu.scu_op(SCUOP_CRT_CRT1_PIN, (void*)1);
		}
		else
#endif
		{
			ast_scu.scu_op(SCUOP_CRT_CRT2_PIN, (void*)1);
		}

		_crt_to_screen(crt, OWNER_CURRENT); //Will trigger HDCP re-init. Resolves BENQ monitor HDCP problem.
		uinfo("CRT turn on\n");
	}
}

static unsigned int _crt_to_loopback_screen(struct s_crt_drv *crt)
{
	unsigned int ret = 0;
	struct s_crt_info *i;

	uinfo("show loopback screen\n");

	/*
	** Bruce151215.
	** xmiter's HDCP status callback may happen during crt_setup().
	** In order to know the 'current_owner' in the callback, we set
	** 'current_owner' before calling crt_setup().
	*/
	crt->current_owner = OWNER_HOST;

	i = &crt->HOST_mode;
	if (xHal_setup_xmiter(crt, i)) {
		ret = -ENODEV;
		goto out;
	}
	i->video_ready = 1;

	if (crt->audio_ready)
	{
		ret = xHal_setup_audio_xmiter(crt);
	}

out:
	return ret;
}


static unsigned int _crt_to_decode_screen(struct s_crt_drv *crt)
{
	u32 ret = 0;

	uinfo("show decode screen\n");

	if (!FTCFG_IS_ENABLED(crt->VE1_mode.force_timing.cfg))
		crt->op_state &= ~COPS_FORCING_TIMING;

	/*
	** Bruce151215.
	** xmiter's HDCP status callback may happen during crt_setup().
	** In order to know the 'current_owner' in the callback, we set
	** 'current_owner' before calling crt_setup().
	*/
	crt->current_owner = OWNER_VE1;

	ret = crt_setup(crt, OWNER_VE1);
	if (ret)
		return ret;

	crt->VE1_mode.video_ready = 1;
	crt->VE1_mode.clr = 0;

	if (FTCFG_IS_ENABLED(crt->VE1_mode.force_timing.cfg))
		crt->op_state |= COPS_FORCING_TIMING;

	csr_to_decode_screen(crt);
	_crt_set_vbuf(crt, crt->dec_buf_base);

	if (crt->disp_select == CRT_2) {
		//Recover scale factor.
		_crt_restore_scale_factor(crt, NULL);
	}

	if (crt->audio_ready)
	{
		ret = xHal_setup_audio_xmiter(crt);
	}

	return ret;
}

static unsigned int _crt_to_console_screen(struct s_crt_drv *crt)
{
	u32 ret = 0;
	//struct s_crt_info info;

	uinfo("show console screen\n");

	/*
	** Bruce151215.
	** xmiter's HDCP status callback may happen during crt_setup().
	** In order to know the 'current_owner' in the callback, we set
	** 'current_owner' before calling crt_setup().
	*/
	crt->current_owner = OWNER_CON;

	//Bruce.Assume console mode will always be set at least once before videoip driver.
	ret = crt_setup(crt, OWNER_CON);
	if (ret) {
		return ret;
	}
	crt->CON_mode.clr = 0;

	csr_to_console_screen(crt);
	_crt_set_vbuf(crt, CRT_CONSOLE_PHY_BASE);

	if (crt->disp_select == CRT_2) {
		//Recover scale factor.
		__crt_change_scale_factor(crt, CRT_SCALE_FACTOR_MAX, CRT_SCALE_FACTOR_MAX, 0, 0);
	}

#if (CONFIG_AST1500_SOC_VER >= 2)
	if (ast_scu.board_info.soc_revision >= 21) {
		/*
		** RctBug#2012031900. Video driver will disable desktop on video mode change. If we switch
		** back to console screen in that case, we will see nothing until we enable desktop again.
		** Add the enable code here.
		*/
		_crt_enable_desktop(crt);
	}
#endif

	if (crt->audio_ready) {
		/*
		 * always set audio if console screen
		 * this is for the special audio hybrid case: console screen is HDMI
		 */
		ret = xHal_setup_audio_xmiter(crt);
	}

	return ret;
}


void _crt_to_screen(struct s_crt_drv *crt, unsigned int screen)
{
	/*
	** Bruce161215. _crt_to_screen() should always running under queue_crt_work
	** work queue. mutex protection is moved to the work queue.
	*/
	//Lock Start
	//down(&crt->crt_lock);
	if (screen == OWNER_CURRENT) {
		screen = crt->current_owner;
	}

	/* Always clear COPS_FORCING_TIMING state when changing to other screen. */
	if (crt->current_owner != screen)
		crt->op_state &= ~COPS_FORCING_TIMING;

	switch (screen) {
	case OWNER_CON:
		_crt_to_console_screen(crt);
		break;
	case OWNER_VE1:
		_crt_to_decode_screen(crt);
		break;
	case OWNER_HOST:
		_crt_to_loopback_screen(crt);
		break;
	case OWNER_NONE:
	default:
		uinfo("show NO screen\n");
		break;
	}
	//up(&crt->crt_lock);
	//Lock End
}


unsigned int crt_to_loopback_screen(struct s_crt_drv *crt)
{
	//crt_to_xxx_screen is the entry point to enable crt. When called, crt will automatically
	//leave power save state.
	/*
	** Bruce161215. FIXME.should crt->op_state need protected?
	*/
	crt->op_state &= ~(COPS_PWR_SAVE);
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_to_screen, (void *)OWNER_HOST, 0);

	return 0;
}

unsigned int crt_to_decode_screen(struct s_crt_drv *crt)
{
	//crt_to_xxx_screen is the entry point to enable crt. When called, crt will automatically
	//leave power save state.
	/*
	** Bruce161215. FIXME.should crt->op_state need protected?
	*/
	crt->op_state &= ~(COPS_PWR_SAVE);
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_to_screen, (void *)OWNER_VE1, 0);

	return 0;
}

unsigned int crt_to_console_screen(struct s_crt_drv *crt)
{
	//crt_to_xxx_screen is the entry point to enable crt. When called, crt will automatically
	//leave power save state.
	/*
	** Bruce161215. FIXME.should crt->op_state need protected?
	*/
	crt->op_state &= ~(COPS_PWR_SAVE);
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_to_screen, (void *)OWNER_CON, 0);

	return 0;
}

static void _crt_setup_xmiter_only(struct s_crt_drv *crt, unsigned int screen)
{
	struct s_crt_info *i;

	switch (screen) {
	case OWNER_CON:
		i = &crt->CON_mode;
		break;
	case OWNER_VE1:
		i = &crt->VE1_mode;
		break;
	case OWNER_HOST:
		i = &crt->HOST_mode;
		break;
	case OWNER_NONE:
	default:
		//uinfo("show NO screen\n");
		goto exit;
	}

	xHal_setup_xmiter(crt, i);
exit:

	return 0;
}

unsigned int crt_setup_xmiter_only_decode(struct s_crt_drv *crt)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t) _crt_setup_xmiter_only, (void *)OWNER_VE1, 0);

	return 0;
}

#if 0
static void draw_logo(struct s_crt_drv *crt)
{
	//Load BMP file. I use .h file as solution.
	struct s_bmp_file *bmp = (struct s_bmp_file *)logo_file;
	struct s_bmp_file_hdr *hdr = &bmp->file_hdr;
	struct s_bmp_dib_hdr *dib = &bmp->dib_hdr;
	unsigned char *data;
	unsigned int width, height;
	unsigned char *vbuf = TO_CRT_CONSOLE_BASE(crt->vbuf_va);
	unsigned int x, y;
	unsigned int rowsize;
	unsigned char *vbuf_next_line;
	unsigned int bpp = 32;
	unsigned int x_offset, y_offset;

	switch (crt->CON_mode.crt_color) {
	case RGB565_FORMAT:
		bpp = 16;
		break;
	case XRGB8888_FORMAT:
		bpp = 32;
		break;
	default:
		uerr("not supported color depth!?\n");
		BUG();
	}

	//validate the logo file
	if (hdr->identity != 0x4D42) {
		uerr("It is not BMP file!?\n");
		BUG();
	}
	if (dib->bit_per_pixel != bpp) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->bit_per_pixel != 32) {
		uerr("ToDo\n");
		BUG();
	}
	if (dib->compression != 3 /*BI_BITFILDS*/&& dib->compression != 0) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->width < 0 || dib->height < 0) {
		uerr("Not support %i, %i\n", dib->width, dib->height);
		BUG();
	}
	data = ((unsigned char*)(bmp)) + hdr->data_offset;
	width = dib->width;
	height = dib->height;
	rowsize = ((crt->CON_mode.width * bpp + 31)/32) * 4;
	//Put the logo in the middle of the screen
	x_offset = ((crt->CON_mode.width - width) >> 1) << 2; //in bytes
	y_offset = ((crt->CON_mode.height - height) >> 1) * rowsize; //in bytes
	uinfo("width=%d, height=%d, bpp=%d, rs=%d\n", width, height, dib->bit_per_pixel, rowsize);
	//Start Fill BMP into vbuf
	vbuf += (height - 1) * rowsize;
	vbuf += x_offset + y_offset;
	vbuf_next_line = vbuf - rowsize;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*(unsigned int *)vbuf = *(unsigned int *)data;
			vbuf += sizeof(unsigned int);
			data += sizeof(unsigned int);
		}
		//next line
		vbuf = vbuf_next_line;
		vbuf_next_line = vbuf - rowsize;
	}
}
#else

#if LOGO_FROM_FLASH
static struct s_bmp_file * _get_logo_from_flash(void)
{
	struct s_bmp_file *bmp = (struct s_bmp_file *)ioremap(LOGO_OFFSET, LOGO_SIZE);
	struct s_bmp_file *ori_bmp;
	struct s_bmp_file_hdr *hdr;
	struct s_bmp_dib_hdr *dib;

	ori_bmp = kmalloc(LOGO_SIZE, GFP_KERNEL);
	BUG_ON(ori_bmp == NULL);
	memcpy(ori_bmp, bmp, LOGO_SIZE);

	hdr = &ori_bmp->file_hdr;
	dib = &ori_bmp->dib_hdr;

	//validate the logo file
	if (hdr->identity != 0x4D42) {
		uerr("It is not BMP file!?\n");
		goto not_exist;
	}
	if (dib->width < 0 || dib->height < 0) {
		uerr("Not support %i, %i\n", dib->width, dib->height);
		goto not_exist;
	}
	if (dib->bit_per_pixel != 16 && dib->bit_per_pixel != 32) {
		uerr("ToDo\n");
		goto not_exist;
	}
	/* The difference between 565 and 555 is mainly by compression. ToDo. Mask must be checked. */
	if (dib->bit_per_pixel == 16 && dib->compression != 3) {
		uerr("Not support!\n");
		goto not_exist;
	}
	if (dib->compression != 3 /*BI_BITFILDS*/&& dib->compression != 0) {
		uerr("Not support!\n");
		goto not_exist;
	}
	iounmap (bmp);
	return ori_bmp;

not_exist:
	kfree(ori_bmp);
	iounmap (bmp);
	return (struct s_bmp_file *)logo_file;

}
#endif //#if LOGO_FROM_FLASH

static inline unsigned int RGB_565_TO_888(unsigned short s)
{
	unsigned int t;
	t = ((unsigned int)((unsigned short)(s) & 0x001F)) << 3;
	t |= ((unsigned int)((unsigned short)(s) & 0x07E0)) << 5;
	t |= ((unsigned int)((unsigned short)(s) & 0xF800)) << 8;
	return t;
}

#if (3 == OSD_FROM_GUI)
static inline unsigned short rgb_888_to_565(unsigned int rgb888Pixel)
{
	unsigned short t;

	t = (rgb888Pixel & 0xF80000) >> 8; /* red */
	t |= (rgb888Pixel & 0xFC00) >> 5; /* green */
	t |= (rgb888Pixel & 0xF8) >> 3; /* blue */

	return t;
}
#endif


#if (CONFIG_AST1500_SOC_VER >= 3)
#undef LOGO_ON_OSD_BUFFER
#endif

static void draw_logo(struct s_crt_drv *crt)
{
	//Load BMP file. I use .h file as solution.
	struct s_bmp_file *bmp;
	struct s_bmp_file_hdr *hdr;
	struct s_bmp_dib_hdr *dib;
	unsigned char *data;
	unsigned char *firstpixel;
	unsigned int width, height;
#if defined(LOGO_ON_OSD_BUFFER)
	unsigned char *vbuf = TO_CRT_OSD_BASE(crt->vbuf_va);
#else
	unsigned char *vbuf = TO_CRT_CONSOLE_BASE(crt->vbuf_va);
#endif
	unsigned int x, y;
	unsigned int bgx, bgy; //x and y counter for display background filling
	unsigned int rowsize;
	unsigned char *vbuf_next_line;
	unsigned int bpp = 32;
	unsigned int x_offset, y_offset;

#if LOGO_FROM_FLASH
	bmp = _get_logo_from_flash();
#else
	bmp = (struct s_bmp_file *)logo_file;
#endif
	hdr = &bmp->file_hdr;
	dib = &bmp->dib_hdr;

	switch (crt->CON_mode.crt_color) {
	case RGB565_FORMAT:
		bpp = 16;
		break;
	case XRGB8888_FORMAT:
		bpp = 32;
		break;
	default:
		uerr("not supported color depth!?\n");
		BUG();
	}

	//validate the logo file
	if (hdr->identity != 0x4D42) {
		uerr("It is not BMP file!?\n");
		BUG();
	}
#if !LOGO_FROM_FLASH
	if (dib->bit_per_pixel != bpp) {
		uerr("Not support!\n");
		//BUG();
		return;
	}
#endif
	if (dib->bit_per_pixel != 32 && dib->bit_per_pixel != 16) {
		uerr("ToDo\n");
		BUG();
	}

	if (dib->compression != 3 /*BI_BITFILDS*/&& dib->compression != 0) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->width < 0 || dib->height < 0) {
		uerr("Not support %i, %i\n", dib->width, dib->height);
		BUG();
	}
	data = ((unsigned char*)(bmp)) + hdr->data_offset;
	firstpixel = ((unsigned char*)(bmp)) + hdr->data_offset;
	width = dib->width;
	height = dib->height;
	rowsize = ((crt->CON_mode.width * bpp + (bpp-1))/bpp) * (bpp >> 3);
	/* Put the logo in the middle of the screen */
	x_offset = ((crt->CON_mode.width - width) >> 1) * (bpp >> 3); /* in bytes */
	y_offset = ((crt->CON_mode.height - height) >> 1) * rowsize; /* in bytes */
	uinfo("width=%d, height=%d, bpp=%d, rs=%d\n", width, height, dib->bit_per_pixel, rowsize);

	//fill background with the first pixel of the logo bmp
	{
#if MEMORY_TEST
		for (bgy = 0; bgy < crt->CON_mode.height; bgy++) {
			for (bgx = 0; bgx < crt->CON_mode.width; bgx++) {
				switch (((unsigned int)vbuf) & 0x1F) {
					case 0:
						*(unsigned int *)vbuf = 0xABC2C321;
						break;
					case 4:
						*(unsigned int *)vbuf = 0x6D33B291;
						break;
					case 8:
						*(unsigned int *)vbuf = 0x839E2DAC;
						break;
					case 0xC:
						*(unsigned int *)vbuf = 0x0E17BC38;
						break;
					case 0x10:
						*(unsigned int *)vbuf = 0x23E8EA76;
						break;
					case 0x14:
						*(unsigned int *)vbuf = 0x43FC06AA;
						break;
					case 0x18:
						*(unsigned int *)vbuf = 0x6A52A91B;
						break;
					case 0x1C:
						*(unsigned int *)vbuf = 0x60600AC4;
						break;
					default:
						printk("un-expected addr?! %p\n", vbuf);
						break;
				}
				vbuf += sizeof(unsigned int);
			}
		}
		goto out;
#else //#if MEMORY_TEST

#if (3 == OSD_FROM_GUI)
		unsigned short bg_color;
		if (16 == dib->bit_per_pixel)
			bg_color = *(unsigned short *)firstpixel;
		else
			bg_color = rgb_888_to_565(*(unsigned int *)firstpixel);

		for (bgy = 0; bgy < crt->CON_mode.height; bgy++) {
			for (bgx = 0; bgx < crt->CON_mode.width; bgx++) {
				*(unsigned short *)vbuf = bg_color;
				vbuf += sizeof(unsigned short);
			}
		}
#else
		unsigned int bg_color;
		if (32 == dib->bit_per_pixel)
			bg_color = *(unsigned int *)firstpixel;
		else
			bg_color = RGB_565_TO_888(*(unsigned short *)firstpixel);

		for (bgy = 0; bgy < crt->CON_mode.height; bgy++) {
			for (bgx = 0; bgx < crt->CON_mode.width; bgx++) {
				*(unsigned int *)vbuf = bg_color;
				vbuf += sizeof(unsigned int);
			}
		}
#endif
#endif //#if MEMORY_TEST
	}

	//done filling background, move vbuf to the starting point
#if defined(LOGO_ON_OSD_BUFFER)
	vbuf = TO_CRT_OSD_BASE(crt->vbuf_va);
#else
	vbuf = TO_CRT_CONSOLE_BASE(crt->vbuf_va);
#endif

	//Start Fill BMP into vbuf
	vbuf += (height - 1) * rowsize;
	vbuf += x_offset + y_offset;
	vbuf_next_line = vbuf - rowsize;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
#if (3 == OSD_FROM_GUI)
			if (16 == dib->bit_per_pixel) {
				*(unsigned short *)vbuf = *(unsigned short *)data;
				data += sizeof(unsigned short);
			} else {
				*(unsigned short *)vbuf = rgb_888_to_565(*(unsigned int *)data);
				data += sizeof(unsigned int);
			}
			vbuf += sizeof(unsigned short);
#else
			if (32 == dib->bit_per_pixel) {
				*(unsigned int *)vbuf = *(unsigned int *)data;
				data += sizeof(unsigned int);
			} else {
				*(unsigned int *)vbuf = RGB_565_TO_888(*(unsigned short *)data);
				data += sizeof(unsigned short);
			}
			vbuf += sizeof(unsigned int);
#endif
		}
		//next line
		if ((dib->bit_per_pixel == 16) && (width & 1)) {
			//Bruce121109. The BMP data is 4 bytes "a set" per row.
			//The buffer address could be non-4 bytes align, so, we can't just align address with 4.
			data += sizeof(unsigned short);
		}
		vbuf = vbuf_next_line;
		vbuf_next_line = vbuf - rowsize;
	}

#if LOGO_FROM_FLASH
#if MEMORY_TEST
out:
#endif
	if (bmp != (struct s_bmp_file *)logo_file)
		kfree(bmp);
#endif
}
#endif

#if 1
static inline void draw_version(struct s_crt_drv *crt)
{
	return;
}
#else
static void draw_version(struct s_crt_drv *crt)
{
	//Load BMP file. I use .h file as solution.
	struct s_bmp_file *bmp = (struct s_bmp_file *)ver_file;
	struct s_bmp_file_hdr *hdr = &bmp->file_hdr;
	struct s_bmp_dib_hdr *dib = &bmp->dib_hdr;
	unsigned char *data;
	unsigned int width, height;
	unsigned char *vbuf = TO_CRT_CONSOLE_BASE(crt->vbuf_va);
	unsigned int x, y;
	unsigned int rowsize;
	unsigned char *vbuf_next_line;
	unsigned int bpp = 32;
	unsigned int x_offset, y_offset;

	switch (crt->CON_mode.crt_color) {
	case RGB565_FORMAT:
		bpp = 16;
		break;
	case XRGB8888_FORMAT:
		bpp = 32;
		break;
	default:
		uerr("not supported color depth!?\n");
		BUG();
	}

	//validate the logo file
	if (hdr->identity != 0x4D42) {
		uerr("It is not BMP file!?\n");
		BUG();
	}
	if (dib->bit_per_pixel != bpp) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->bit_per_pixel != 32) {
		uerr("ToDo\n");
		BUG();
	}
	if (dib->compression != 3 /*BI_BITFILDS*/&& dib->compression != 0) {
		uerr("Not support!\n");
		BUG();
	}
	if (dib->width < 0 || dib->height < 0) {
		uerr("Not support %i, %i\n", dib->width, dib->height);
		BUG();
	}
	data = ((unsigned char*)(bmp)) + hdr->data_offset;
	width = dib->width;
	height = dib->height;
	rowsize = ((crt->CON_mode.width * bpp + 31)/32) * 4;
	//Put the version in the bottom left of the screen
	x_offset = 0; //in bytes
	y_offset = (crt->CON_mode.height - height) * rowsize; //in bytes
	uinfo("width=%d, height=%d, bpp=%d, rs=%d\n", width, height, dib->bit_per_pixel, rowsize);
	//Start Fill BMP into vbuf
	vbuf += (height - 1) * rowsize;
	vbuf += x_offset + y_offset;
	vbuf_next_line = vbuf - rowsize;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			*(unsigned int *)vbuf = *(unsigned int *)data;
			vbuf += sizeof(unsigned int);
			data += sizeof(unsigned int);
		}
		//next line
		vbuf = vbuf_next_line;
		vbuf_next_line = vbuf - rowsize;
	}
}
#endif

static u32 setup_video(struct s_crt_drv *crt, struct s_crt_info *i)
{
	u32 phy_ba;

	/*
	** Bruce101008. To resolve a DELL E248WFP monitor mode detection issue,
	** We need to turn off video output and wait for few hundred ms so that
	** DELL monitor can go into "mode detection" state and then can detect
	** 480p->1080p mode change correctly.
	*/
	if (!(crt->op_state & COPS_FORCING_TIMING))
		_crt_disable_xmiter(crt);

	/*
	//Bruce111114. We should not reset the whole CRT. Otherwise the CRT1 for I2S clock
	//will also be reseted and no sound. We turn off CRT in ASTSetMode() instead.
	//This is for AST1500 old "dual-head" design. It is no more maintained.
	*/
	//reset_crt_engine(crt->disp_select);


	//fill crt buffer with black, For avoiding the noise
	if (i->clr) {
		if (i->owner == OWNER_VE1) {
			/* Will take too long for 4K fb case. Don't do it for SoC >= V3. */
#if (3 > CONFIG_AST1500_SOC_VER)
			vbufset_black(TO_VB1_DE_DEST0_BASE(crt->vbuf_va),
			                  DISPLAY_MAX_X,
			                  DISPLAY_MAX_Y,
			                  i->crt_color);

			if (ast_scu.ability.v_crt_flip < 2)
				vbufset_black(TO_VB1_DE_DEST1_BASE(crt->vbuf_va),
				                  DISPLAY_MAX_X,
				                  DISPLAY_MAX_Y,
				                  i->crt_color);
#endif /* #if (3 > CONFIG_AST1500_SOC_VER) */

		} else { //OWNER_CON
#if NO_LOGO
			//vbufset_black(TO_CRT_CONSOLE_BASE(crt->vbuf_va), DISPLAY_MAX_X, DISPLAY_MAX_Y, i->crt_color);
#else
			//vbufset_white(TO_CRT_CONSOLE_BASE(crt->vbuf_va), DISPLAY_MAX_X, DISPLAY_MAX_Y, i->crt_color);
			draw_logo(crt);
			draw_version(crt);

#if (3 <= CONFIG_AST1500_SOC_VER) && defined(LOGO_ON_OSD_BUFFER)
			crt->OSD_force_2_color = 0;
			crt->OSD_y_offset = 0;
			crt->OSD_y_size = crt->CON_mode.height;
			crt->Logo_Setting = (OSD_DISPLAY_ENABLE | (OSD_ALPHA_VALUE << OSD_SET_ALPHA_BIT));
			schedule_work(&crt->LogoWork);
#endif
#endif
		}
	}

	if (i->owner == OWNER_VE1) {
		phy_ba = (u32)crt->dec_buf_base;
	} else { //OWNER_CON
		phy_ba = (u32)CRT_CONSOLE_PHY_BASE;
	}

	/*
	** Under multicast+no_tcp mode, client may receive video stream even there is no display
	** attached. If we don't setup CRT under this case, vsync interrupt won't operate and
	** stream buffer will be full and driver panic.
	*/
#if 0
	//If display is not attached yet, setup CRT and xmiter is meanless.
	if (!crt->display_attached)
		return 0;
#endif

	if(ASTSetMode (crt, crt->disp_select,
	               (u32)phy_ba,
	               i->width,
	               i->height,
	               i->refresh_rate,
	               i->crt_color,
	               i))
	{
		uerr("The resolution is not in mode table list");
		return -1;
	}

	if (!(crt->op_state & COPS_FORCING_TIMING)) {
		if (xHal_setup_xmiter(crt, i)) {
			return -1;
		}
	}
	return 0;
}


static inline unsigned int hdmi_VIC_to_H14b4K_VIC(e_hdmi_vic VIC)
{
	unsigned int HDMI_VIC = 0;
	/* Reference to HDMI spec 2.0 Appendix E.
	** Only HDMI 1.4b defined 4K need VSIF. Which are:
	** 2160p24/25/30Hz in HDMI 1.4b 8.2.3.1
	** VIC 95, 94, 93, 98
	*/
	switch (VIC) {
	case VIC_2160p30:
		HDMI_VIC = 1;
		break;
	case VIC_2160p25:
		HDMI_VIC = 2;
		break;
	case VIC_2160p24:
		HDMI_VIC = 3;
		break;
	case VIC_2160p24E:
		HDMI_VIC = 4;
		break;
	default:
		break;
	}
	/*
	** Return 0 means no H14b VSIF needed.
	** Return non-zero means need H14b VSIF with returned HDMI_VIC value.
	*/
	return HDMI_VIC;
}

static unsigned int use_yuv420_output(e_hdmi_vic VIC)
{
	/* Only VIC 96,97,101,102,106,107 can support 4:2:0 format. They are 4K. */
	if ((VIC == VIC_2160p50)
	 ||(VIC == VIC_2160p)
	 ||(VIC == VIC_2160p50E)
	 ||(VIC == VIC_2160pE)
	 ||(VIC == VIC_2160p50W1)
	 ||(VIC == VIC_2160pW1))
		return 1;
	else
		return 0;
}

static unsigned int hdmi_construct_vs_info(
	hdmi_vsif *vsif,
	e_hdmi_vic VIC)
{
	unsigned int HDMI_VIC = hdmi_VIC_to_H14b4K_VIC(VIC);

	if (!HDMI_VIC)
		return 0;

	vsif->H14b4K.Type = HDMI_VENDORSPEC_INFOFRAME_TYPE|0x80;
	vsif->H14b4K.Ver = HDMI_VENDORSPEC_INFOFRAME_VER;
	vsif->H14b4K.Len = 5;

	vsif->H14b4K.OUI_03 = 0x03;
	vsif->H14b4K.OUI_0C = 0x0C;
	vsif->H14b4K.OUI_00 = 0x00;

	vsif->H14b4K.VideoFormat = 1; /* means 4K present */

	vsif->H14b4K.HDMI_VIC = HDMI_VIC;

	{ /* Calculate InfoFrame Checksum */
		unsigned int i;

		vsif->H14b4K.Checksum = vsif->pktbyte.VSIF_HB[0]
		                    + vsif->pktbyte.VSIF_HB[1]
		                    + vsif->pktbyte.VSIF_HB[2];

		/* AVI_Info data[0] ~ [2] => header; data[3] ~ [16] => contents */
		for (i = 1; i <= 5; i++)
			vsif->H14b4K.Checksum += vsif->pktbyte.VSIF_PB[i];

		vsif->H14b4K.Checksum = 0x100 - vsif->H14b4K.Checksum;
	}

	return 1;
}

static u32 vic_pixel_repeat[] = {
	VIC_480i, VIC_480iH,
	VIC_240p, VIC_240pH,
	VIC_576i, VIC_576iH,
	VIC_288p, VIC_288pH,
	VIC_576i50, VIC_576i50H,
	VIC_480i59, VIC_480i59H,
	VIC_576i100, VIC_576i100H,
	VIC_480i119, VIC_480i119H,
};

static u32 vic_is_pixel_repeat(u32 vic)
{
	int i, size;

	size = ARRAY_SIZE(vic_pixel_repeat);

	for (i = 0; i < size; i++) {
		if (vic_pixel_repeat[i] == vic)
			return 1;
	}
	return 0;
}

static unsigned int hdmi_construct_avi_info(hdmi_avi_infoframe *avi, MODE_ITEM *mode)
{
	e_hdmi_vic vic;
	u32 aspect_ratio, pixel_rep;
	u32 i;

	vic = mode->HDMIType;

	if ((720 <= mode->VActive)
		|| ((Interl == mode->ScanMode) && (720 <= (mode->VActive << 1))))
		aspect_ratio = AVIINFO_CODED_FRAME_ASPECT_16x9;
	else
		aspect_ratio = AVIINFO_CODED_FRAME_ASPECT_4x3;

	pixel_rep = 0;
	pixel_rep = vic_is_pixel_repeat(vic);

	/* CEA-861-E p35. */
	avi->info.Type = HDMI_AVI_INFOFRAME_TYPE|0x80; /* |0x80 according to HDMI 1.4b spec */
	avi->info.Ver = HDMI_AVI_INFOFRAME_VER;
	avi->info.Len = HDMI_AVI_INFOFRAME_LEN;


	/* packet byte #0: Checksum */

	/* packet byte #1 */
	avi->info.Scan = 0;
	avi->info.BarInfo = 0;
	avi->info.ActiveFmtInfoPresent = 1;

	/*
	** Bruce160428. We always use AVIINFO_COLOR_MODE_RGB444 here.
	** And expect hdmi_avi_infoframe_fixup() will correct the value later.
	*/
	/* CEA-861-E p40 table 14. Always use RGB444? */
	avi->info.ColorMode = AVIINFO_COLOR_MODE_RGB444;
	if (use_yuv420_output(vic)) {
		if (ast_scu.board_info.soc_revision >= 31)
			avi->info.ColorMode = AVIINFO_COLOR_MODE_YUV420;
	}

	avi->info.FU1 = 0;

	/* packet byte #2 */
	avi->info.ActiveFormatAspectRatio = 0x8; /* Same as coded frame aspect ratio */
	avi->info.PictureAspectRatio = aspect_ratio;
	avi->info.Colorimetry = AVIINFO_COLORIMETRY_NA;

	/*
	 * Bruce200708. Check CEA-861-F Section 5.1.
	 * RGB default quantization value will be:
	 * full: if IT Video Format
	 * limited: if CE Video Format
	 *
	 * By search "CE Video Format" define.
	 * CE Video Format: Any Video Format listed in Table 1 except the 640x480p Video Format
	 * IT Video Format: Any Video Format that is not a CE Video Format.
	 *
	 * Since YCbCr and range overwritten are optional sink capability.
	 * To compliant with most of sink, we choose CE content with default RGB quantization.
	 * ==> RGB limited range.
	 */
	/* packet byte #3 */
	avi->info.Scaling = 0;
	avi->info.Quantization = 0; /* RGB qunatization, default */
	avi->info.ExtendedColorimetry = 0;
	avi->info.ITContent = 0;

	/* packet byte #4 */
	/*
	** According to HDMI 2.0 spec Appendix E.
	** Set AVI InfoFrame's VIC to 0 for HDMI 1.4b defined 4K.
	*/
	if (hdmi_VIC_to_H14b4K_VIC(vic))
		vic = 0;

	avi->info.VIC = vic;
	avi->info.FU3 = 0;

	/* packet byte #5 */
	avi->info.PixelRepetition = pixel_rep;
	avi->info.ContentType = 0;
	avi->info.YccQuantization = 0;

	/*
	 * Description in CEA-861-D
	 * a. Line Number of End of Top Bar—An unsigned integer value representing the last line of a horizontal
	 *    letterbox bar area at the top of the picture. Zero means no horizontal bar is present at the top of the
	 *    picture.
	 * b. Line Number of Start of Bottom Bar—An unsigned integer value representing the first line of a
	 *    horizontal letterbox bar area at the bottom of the picture. If greater than the Maximum Vertical Active
	 *    Lines of the known format, no horizontal bar is present at the bottom of the picture
	 * c. Pixel Number of End of Left Bar—An unsigned integer value representing the last horizontal pixel of
	 *    a vertical pillar-bar area at the left side of the picture. Zero means no vertical bar is present on the left
	 *    of the picture.
	 * d. Pixel Number of Start of Right Bar—An unsigned integer value representing the first horizontal pixel
	 *    of a vertical pillar-bar area at the right side of the picture. If greater than the Maximum Horizontal
	 *    Pixels of the known format, no vertical bar is present on the right side of the picture
	 */
	avi->info.Ln_End_Top = 0; /* no horizontal bar is present at the top of the picture. */
	avi->info.Ln_Start_Bottom = mode->VActive + 1; /* no horizontal bar is present at the bottom of the picture */
	avi->info.Pix_End_Left = 0; /* no vertical bar is present on the left of the picture. */
	avi->info.Pix_Start_Right = mode->HActive + 1; /* no vertical bar is present on the right side of the picture */

	/* Calculate AVI InfoFrame Checksum */

	avi->info.Checksum = avi->pktbyte.AVI_HB[0] + avi->pktbyte.AVI_HB[1] + avi->pktbyte.AVI_HB[2];

	/* AVI_Info data[0] ~ [2] => header; data[3] ~ [16] => contents */
	for (i = 1; i <= HDMI_AVI_INFOFRAME_LEN; i++)
		avi->info.Checksum += avi->pktbyte.AVI_PB[i];

	avi->info.Checksum = 0x100 - avi->info.Checksum;

	return 1;
}

u32 crt_timing_hdmi_vic_to_sn(u32 vic)
{
	struct hdmi_vic_to_sn_convert *t = vic_to_sn_tb;

	/* Check for invalid VIC code. */
	if ((vic == VIC_unknown) || (vic >= VIC_MAX))
		goto not_found;

	while ((t->vic != VIC_MAX) && (t->vic != vic))
		t++;

	if (t->vic == VIC_MAX)
		goto not_found;

	return t->sn;

not_found:
	uerr("Can't find valid VIC code %d\n", vic);
	return TIMING_SN_INVALID;
}
EXPORT_SYMBOL(crt_timing_hdmi_vic_to_sn);

static void hdmi_avi_infoframe_fixup(struct s_crt_drv *crt, struct s_crt_info *info, struct s_xmiter_mode *xmiter)
{
	hdmi_avi_infoframe *avi;
	int i;

	if (xmiter->AVI_Info.number == 0)
		return;

	avi = (hdmi_avi_infoframe *) xmiter->AVI_Info.data;

	/* always patch YUV420 to YUV444 because HDMI transmitter, sii9136, doesn't support CSC of YUV420 */
	if (avi->info.ColorMode == AVIINFO_COLOR_MODE_YUV420)
		avi->info.ColorMode = AVIINFO_COLOR_MODE_YUV444;

	/* pass-through strict mode */
	if (FTCFG_G_PASS_THROUGH(info->force_timing.cfg) == FTCFG_PASS_THROUGH_TYPE_STRICT)
		goto color_mode_and_colorimetry_fix_done;

	/*
	** Bruce160428. In current design, HDMI Tx never do CSC. So, we fixup the
	** AVI infoframe to be the same as CRT output format.
	**
	** In current design, only 4K output will use YUV format output.
	** For seamless switching, we always use bt.709 colorimetry for 4K output.
	**
	** BUG: bt601 source hybrid to 4K will introduce wrong CSC.
	** Unless we can dynamically change HDMI tx's AVI infoframe, we can't
	** resolve this problem.
	*/

	/*
	 * HDR10
	 *
	 * From Apple TV, HDR10 is YUV422 and BT.2020
	 * so keep color mode (allow 10-bit data trasmission) and colorimetry (avoid dullness issue) to support it
	 * the condition for keeping AVI infoframe is 'extended colorimetry' only (all color mode can be BT.2020)
	 *
	 * note:
	 * we got dullness issue with BT.2020 because we always set colorimetry to default before
	 */
	if ((avi->info.Colorimetry == AVIINFO_COLORIMETRY_EXTEND)
		&& (avi->info.ExtendedColorimetry == AVIINFO_EXTENDED_COLORIMETRY_BT2020)) {
		goto color_mode_and_colorimetry_fix_done;
	}

	switch (info->crt_output_format) {
	case YUV444_FORMAT:
		avi->info.ColorMode = AVIINFO_COLOR_MODE_YUV444;
		break;
	case YUV420_FORMAT:
		avi->info.ColorMode = AVIINFO_COLOR_MODE_YUV420;
		break;
	case XRGB8888_FORMAT:
	default:
		avi->info.ColorMode = AVIINFO_COLOR_MODE_RGB444;
		break;
	}

	avi->info.Colorimetry = AVIINFO_COLORIMETRY_NA; /* default colorimetry */

color_mode_and_colorimetry_fix_done:

	if (xmiter->flag & HDMI_FORCE_RGB_OUTPUT_LIMITED) {
		if (AVIINFO_COLOR_MODE_RGB444 == avi->info.ColorMode) {
			MODE_ITEM *mode;

			mode = crt_get_current_mode(crt, xmiter->owner, NULL);
			if ((VIC_MAX <= mode->HDMIType) || (VIC_DMT0659 >= mode->HDMIType)) {
				/* IT force to limited (1) */
				avi->info.Quantization = 1;
			} else {
				/* CE set to default (0) */
				avi->info.Quantization = 0;
			}
		}
	}

	/* Calculate AVI InfoFrame Checksum */
	avi->info.Checksum = 0x91; /* 0x82 + 0x02 + 0x0D */

	for (i = 1; i <= 13; i++)
		avi->info.Checksum += avi->pktbyte.AVI_PB[i];

	avi->info.Checksum = 0x100 - avi->info.Checksum;
}

static void hdmi_hdr_infoframe_fixup(struct s_crt_drv *crt, struct s_xmiter_mode *xmiter)
{
	if (0 == xmiter->HDR_Info.number)
		return;

	if (xmiter->flag & HDMI_HDR_MODE_OFF)
		xmiter->HDR_Info.number = 0;
}

static void hdmi_infoframe_fixup(struct s_crt_drv *crt, struct s_crt_info *info, struct s_xmiter_mode *xmiter)
{
	hdmi_avi_infoframe_fixup(crt, info, xmiter);
	hdmi_hdr_infoframe_fixup(crt, xmiter);
}

unsigned int crt_ft_cfg_to_mode_idx(struct s_crt_drv *crt, unsigned int cfg)
{
	unsigned int foolproof = 1;
	unsigned int idx = TIMING_INDEX_INVALID;
	unsigned int sn = TIMING_SN_INVALID;

	if (!FTCFG_IS_ENABLED(cfg))
		return idx;

	switch (FTCFG_G_TYPE(cfg)) {
	case FTCFG_TYPE_VIC:
		sn = crt_timing_hdmi_vic_to_sn(FTCFG_G_TIMING_CODE(cfg));
		break;
	case FTCFG_TYPE_TIMING_SN:
		sn = FTCFG_G_TIMING_CODE(cfg);
		/* To keep flexibility and test_mode, FTCFG_TYPE_TIMING_SN will not be foolproof. */
		foolproof = 0;
		break;
	case FTCFG_TYPE_EDID:
		if (crt->edid_parse_result.valid)
			sn = crt_timing_index_to_sn(crt->edid_parse_result.preferred_timing_index);
		else
			sn = AST_TIMING_SN_1080P60; /* FIXME: PAL/NTSC has different default. */
		break;
	default:
		uerr("Unsupported force timing configuration!! Ignore\n");
		break;
	}

	if (foolproof) {
		unsigned int t = sn;
		/*
		** If configured force timing value is out of spec or not supported,
		** we should return TIMING_INDEX_INVALID or pick a working timing.
		** This is important for the case of FTCFG_TYPE_EDID.
		** AST152X can't support 2160p60Hz output.
		*/
		sn = timing_sn_convert(table_interlace_to_progressive, sn);
		sn = timing_sn_convert(crt_preferred_timing_sn_tb, sn);
		if (t != sn)
			uinfo("Convert force timing format.sn(0x%04X to 0x%04X)\n", t, sn);
	}

	if (sn != TIMING_SN_INVALID)
		idx = crt_timing_sn_to_index(sn);

	/* Return TIMING_INDEX_INVALID for passthrough. */
	return idx;
}

static void crt_setup_force_timing_par(struct s_crt_drv *crt, struct s_crt_info *i)
{
	struct s_force_timing *ft = &i->force_timing;
	unsigned int cfg = ft->cfg;
	struct s_xmiter_mode *xmiter = &i->xmiter_mode;

	ft->timing_table_idx = crt_ft_cfg_to_mode_idx(crt, cfg);

	if (ft->timing_table_idx == TIMING_INDEX_INVALID) {
		/* For passthrough or invalid cfg. */
		/* xmiter_mode is typically the same as i, except force timing mode. */
		memset(xmiter, 0, sizeof(struct s_xmiter_mode));
		xmiter->owner = i->owner; //For lookupModeTable()
		xmiter->HDMI_Mode = i->HDMI_Mode;
		xmiter->flag = i->flag;
		xmiter->color_depth = i->color_depth;
		memcpy(&xmiter->AVI_Info, &i->AVI_Info, sizeof(InfoFrame));
		memcpy(&xmiter->HDMI_Info, &i->HDMI_Info, sizeof(InfoFrame));
		memcpy(&xmiter->HDR_Info, &i->HDR_Info, sizeof(InfoFrame));

		goto setup_done;
	}

	uinfo("Config output timing converter parameters...\n");

	/*
	** Setup xmiter_mode according to force timing.
	*/
	memset(xmiter, 0, sizeof(struct s_xmiter_mode));

	xmiter->owner = i->owner; //For lookupModeTable()
	//xmiter->TimingTableIdx = ft->timing_table_idx;

	/* FIXME. Shell we revise the define of HDMI_Mode?! */
	if (FTCFG_IS_DVI(cfg)) {
		xmiter->HDMI_Mode = s_DVI;
	} else {
		e_hdmi_vic VIC = crt->mode_table[ft->timing_table_idx].HDMIType;

		/*
		** Bruce150507. Only AST1520 A1 support 4K YUV420 format.
		** In most of cases, crt_output_format is determined by VE driver::_decide_crt_output_color_format().
		** The only exception is test_mode. This is a patch required for test_mode.
		*/
		if (use_yuv420_output(VIC)) {
			if (ast_scu.board_info.soc_revision >= 31) {
				i->crt_output_format = YUV420_FORMAT;
			}
		}

		xmiter->HDMI_Mode = s_HDMI_16x9;
		/* FIXME. Aspect copy from source and dynamically update to HDMI xmitter.?! or from EDID? */
		xmiter->AVI_Info.number =
			hdmi_construct_avi_info((hdmi_avi_infoframe *)(xmiter->AVI_Info.data),
					crt->mode_table + ft->timing_table_idx);


		xmiter->HDMI_Info.number =
			hdmi_construct_vs_info((hdmi_vsif *)(xmiter->HDMI_Info.data),
			                     VIC);
	}
	xmiter->flag = i->flag;

setup_done:
	/* we modify infoframe here instead of HDMI transmitter */
	hdmi_infoframe_fixup(crt, i, xmiter);
	return;
}

static void _force_crt_info_to_hdmi(struct s_crt_drv *crt, struct s_crt_info *i, int timing_idx)
{
	MODE_ITEM *timing;
	e_hdmi_vic VIC;

	/* SANYO TV need AVI info frame for display. */
	timing = &crt->mode_table[timing_idx];
	VIC = timing->HDMIType;

	/*
	** Bruce150507. Only AST1520 A1 support 4K YUV420 format.
	*/
	if (use_yuv420_output(VIC)) {
		if (ast_scu.board_info.soc_revision >= 31) {
			i->crt_output_format = YUV420_FORMAT;
		}
	}

	i->HDMI_Mode = s_HDMI_16x9;
	/* FIXME. Aspect copy from source and dynamically update to HDMI xmitter.?! or from EDID? */
	i->AVI_Info.number =
		hdmi_construct_avi_info((hdmi_avi_infoframe *)(i->AVI_Info.data), timing);

	i->HDMI_Info.number =
		hdmi_construct_vs_info((hdmi_vsif *)(i->HDMI_Info.data), VIC);

}

static unsigned int must_reconfig_crt(struct s_crt_drv *crt, struct s_crt_info *current_info, struct s_crt_info *new_info)
{
	unsigned int new_ft_cfg = new_info->force_timing.cfg;
	unsigned int current_ft_cfg = current_info->force_timing.cfg;

	/* If previous ft setting is different from the new one, then CRT state need a reset. */
	if (current_ft_cfg != new_ft_cfg)
		goto re_config;

	/* Check special FTCFG_TYPE_EDID case. */
	if (FTCFG_IS_ENABLED(current_ft_cfg)
	    && (FTCFG_G_TYPE(current_ft_cfg) == FTCFG_TYPE_EDID)) {
		/* EDID preferred timing != current configuration */
		unsigned int current_idx = current_info->force_timing.timing_table_idx;

		if (current_idx != crt->edid_parse_result.preferred_timing_index)
			goto re_config;
	}

	/* new sink does't support YUV and original crt output is YUV */
	if (current_info->crt_output_format != new_info->crt_output_format)
		goto re_config;

	/*
	** If desired HDCP setting is different from previous one, we need to setup
	** xmiter again in order to trigger HDCP and disable DAC dual output.
	** TODO:
	** - If timing is forced to HDCP enable, then this check should be ignored.
	** - How to handle repeater mode case?
	*/
	if (current_info->EnHDCP != new_info->EnHDCP)
		goto re_config;

	return 0;

re_config:
	/* Return true when we need to re-config crt. */
	return 1;
}

static unsigned _is_avi_info_valid(const unsigned char *data)
{
	unsigned tmp = 0;
	unsigned i;
	hdmi_avi_infoframe *avi = (hdmi_avi_infoframe *)data;
	unsigned ret = 0;

	/* 4 bytes header + 13 bytes data */
	for (i = 0; i < (HDMI_AVI_INFOFRAME_LEN + 4); i++) {
		tmp += data[i];
	}
	/* 1 byte checksum. mask 0xFF */
	tmp &= 0xFF;
	//printk("AVI Info checksum = %02x\n", checksum);
	if (tmp) {
		uerr("AVIInfo checksum failed (0x%02X)\n", tmp);
		goto out;
	}

	/* Validate bar info. */
	tmp = avi->info.BarInfo;
	/* Has vertical bar info? */
	if (tmp & 0x1) {
		/* Must not equal and not overlap. */
		if (avi->info.Pix_End_Left >= avi->info.Pix_Start_Right) {
			uerr("Invalid vertical bar info (L:%d, R:%d)\n"
			      , avi->info.Pix_End_Left, avi->info.Pix_Start_Right);
			goto out;
		}
	}
	/* Has horizontal bar info? */
	if (tmp & 0x2) {
		/* Must not equal and not overlap. */
		if (avi->info.Ln_End_Top >= avi->info.Ln_Start_Bottom) {
			uerr("Invalid horizontal bar info (T:%d, B:%d)\n"
			      , avi->info.Ln_End_Top, avi->info.Ln_Start_Bottom);
			goto out;
		}
	}
	/* return TRUE when all valid. */
	ret = 1;
out:
	return ret;
}

/* Any video format listed in Table 2 except 640x480p */
static u32 ce_video_map[8] = {
	0xFFFFFFFC, /*  2 ~  31 */
	0xFFFFFFFF, /* 32 ~  63 */
	0xFFFFFFFF, /* 64 ~  95 */
	0x00000FFF, /* 96 ~ 107 */
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
};

static u32 ce_video_format(InfoFrame *avi_info, InfoFrame *vs_info)
{
	hdmi_avi_infoframe *avi;
	hdmi_vsif *vsif;
	u32 shift;
	u8 vic;

	/* No AVI info means IT video format. */
	if (!avi_info || !avi_info->number)
		goto is_it;

	avi = (hdmi_avi_infoframe *)(avi_info->data);

	vic = avi->info.VIC;

	/* CE Video Format exception. HDMI 1.4b 4K30 format uses VIC==0 */
	if (0 == vic) {
		if (!vs_info || !vs_info->number)
			goto is_it;
		/* HDMI VSIF is valid */
		vsif = (hdmi_vsif *)(vs_info->data);
		/* valid OUI */
		if ((vsif->H14b4K.OUI_03 != 0x03) || (vsif->H14b4K.OUI_0C != 0x0C)
		    || (vsif->H14b4K.OUI_00 != 0x00)) {
			goto is_it;
		}
		/* check if has "extended resolution format" */
		if (vsif->H14b4K.VideoFormat != 1)
			goto is_it;
		/* has extended resolution format. It is a CE Video Format. */
		goto is_ce;
	}

	shift = vic & 0x1F;
	if (ce_video_map[vic >> 5] & (1 << shift))
		goto is_ce;

is_it:
	uinfo("is IT Video Format!\n");
	return 0;
is_ce:
	uinfo("is CE Video Format!\n");
	return 1;
}

static int _is_limited_range(InfoFrame *avi_info, InfoFrame *vs_info)
{
	hdmi_avi_infoframe *avi;

	/* No AVI info means IT video format. */
	if (!avi_info || !avi_info->number)
		goto is_full;

	avi = (hdmi_avi_infoframe *)(avi_info->data);

	if (avi->info.ColorMode == AVIINFO_COLOR_MODE_RGB444) {
		/* RGB out. check RGB quantization range. */
		switch (avi->info.Quantization) {
		case AVIINFO_RGB_RANGE_DFLT: //default
			if (ce_video_format(avi_info, vs_info))
				goto is_limited;
			// else
			goto is_full;
		case AVIINFO_RGB_RANGE_LMTD: //limited
			goto is_limited;
		case AVIINFO_RGB_RANGE_FULL: //full
			goto is_full;
		default: //reserved. Use full
			goto is_full;
		}
	} else {
		/* YUV out. check YUV quantization range */
		switch (avi->info.YccQuantization) {
		case AVIINFO_YUV_RANGE_LMTD:
			goto is_limited;
		case AVIINFO_YUV_RANGE_FULL:
			goto is_full;
		default:
			if (ce_video_format(avi_info, vs_info))
				goto is_limited;
			//else
			goto is_full;
		}
	}

is_full:
	return 0;
is_limited:
	return 1;
}

void crt_setup_par(struct s_crt_drv *crt, struct s_crt_info *info)
{
	struct s_crt_info *i;

	switch (info->owner) {
	case OWNER_CON:
#if MEMORY_TEST
		info->width = 1920;
		info->height = 1080;
		info->refresh_rate = 60;
#endif
		i = &crt->CON_mode;

		/* Auto choose default Signal Type for console screen. */
		if (crt->tx_select == XIDX_CLIENT_D) {
			/* Digital interface */
#if 0
#if 0
			/* Bruce120430. console screen use HDMI signal is meaningless. I thought it may resolve some
			** compatibility issues. But it turns out to be unrelated. Output HDMI signal causes several issues
			** 1. noisy audio tone sound (board dependent?!). 2. AVI info frame unavailable.
			** So, I decide to always use DVI signal.
			*/
			if (EdidHasCEA(crt->edid)) {
				i->HDMI_Mode = s_HDMI_4x3; //Always use 640x480
			} else
#endif
			{
				info->HDMI_Mode = s_DVI;
			}
#else
			if (ast_scu.astparam.console_default_is_hdmi) {
				info->TimingTableIdx = _lookupModeTable(crt, info->width, info->height, info->refresh_rate);
				info->HDMI_Mode = s_HDMI_4x3;
				info->AVI_Info.number = hdmi_construct_avi_info((hdmi_avi_infoframe *)(info->AVI_Info.data), crt->mode_table + info->TimingTableIdx);

			} else {
				info->HDMI_Mode = s_DVI;
			}
#endif
		} else {
			// Analog interface
			info->HDMI_Mode = s_RGB;
		}
		break;
	case OWNER_VE1:
		i = &crt->VE1_mode;
		break;
	case OWNER_HOST:
		i = &crt->HOST_mode;
		break;
	default:
		return;
		break;
	}

	if (must_reconfig_crt(crt, i, info))
		crt->op_state &= ~COPS_FORCING_TIMING;

	memcpy(i, info, sizeof(struct s_crt_info));

	/*
	 * Bruce200708. In current design, we don't do range convert from
	 * HDMI Rx => VE => CRT out. We are leaving this job to HDMI Tx (SiI9136).
	 * So, CRT's output quantization range will == source quantization range.
	 *
	 * We are assuming that source's range can be decided by `info`'s info frame.
	 * TODO. It will be better that range info is passed from host/VE explicitly.
	 */
	if (_is_limited_range(&i->AVI_Info, &i->HDMI_Info))
		i->flag |= CRT_OUT_LIMITED;

	uinfo("CRT output quantization range: %s\n", (i->flag & CRT_OUT_LIMITED) ? "limited" : "full");

	/* Bruce190711. A patch to fixup wrong AVI Info frame. DirectTV fix. */
	if (i->AVI_Info.number && !_is_avi_info_valid(i->AVI_Info.data)) {
		uerr("!! reconstruct avi info\n");
		uerr("before reconstruct:(%d)\n", i->AVI_Info.number);
		udump(i->AVI_Info.data, sizeof(hdmi_avi_infoframe));

		_force_crt_info_to_hdmi(crt, i, i->TimingTableIdx);

		uerr("after reconstruct:(%d)\n", i->AVI_Info.number);
		udump(i->AVI_Info.data, sizeof(hdmi_avi_infoframe));
	}

	if (OWNER_VE1 == i->owner) {
		/*
		 * With audio hybrid support in AST1520
		 * we have to force transmitter to HDMI mode for audio output
		 */
		if ((ast_scu.astparam.a_force_dual_output) || (3 <= ast_scu.ability.soc_op_mode)) {
			if ((s_RGB == i->HDMI_Mode) || (s_DVI == i->HDMI_Mode)) {
				_force_crt_info_to_hdmi(crt, i, i->TimingTableIdx);
			}
		}
	}

	crt_setup_force_timing_par(crt, i);

	crt->available_screen |= info->owner;
}

void crt_reset_par(struct s_crt_drv *crt, unsigned int owner)
{
	if (crt->current_owner == owner)
	{
		struct s_crt_info *i;
		switch (owner) {
#ifdef CONFIG_ARCH_AST1500_CLIENT
		case OWNER_CON:
			i = &crt->CON_mode;
			break;
		case OWNER_VE1:
			i = &crt->VE1_mode;
			break;
#endif
#ifdef CONFIG_ARCH_AST1500_HOST
		case OWNER_HOST:
			i = &crt->HOST_mode;
			break;
#endif
		default:
			uerr("un-supported screen?!(%d)\n", owner);
			return;
		}
		i->video_ready = 0;
		crt->audio_ready = 0;
	}
}

static u32 crt_setup(struct s_crt_drv *crt, unsigned int owner)
{
	u32 ret = 0;
	struct s_crt_info *i;
	int cts_7_33 = -1;

	if (!crt->is_client) {
		uerr("This code path should never happen!\n");
		return 0;
	}

	switch (owner) {
	case OWNER_CON:
		i = &crt->CON_mode;
		break;
	case OWNER_VE1:
		i = &crt->VE1_mode;

		/*
		** Bruce120412. This is a patch to pass HDMI CTS test 7-33.
		** We ALWAYS output DVI video format when the sink is a DVI monitor.
		*/
		/* Bruce120622. Since AST1500 should apply SYNC_REGEN issue. And the field issue
		** is using an DVI EDID. So, I have to ignore the HDMI CTS requirement to trigger
		** the SYNC_REGEN under DVI EDID (but HDMI signal source).
		** I think this this CTS patch is optional in real world. So, I just remove this patch
		** under AST1500 case.
		*/
		if (!(crt->debug_flag & CDBG_IGNORE_CTS7_33)) {
			cts_7_33 = i->xmiter_mode.HDMI_Mode;
			if (crt->display_attached) {
				if (crt->tx_select == XIDX_CLIENT_D) {
					//Digital interface
					if (!EdidHasCEA(crt->edid)) {
						uinfo("CTS7.33 force to DVI output\n");
						i->xmiter_mode.HDMI_Mode = s_DVI;
					}
				}
			}
		}
		break;
	default:
		uerr("un-supported screen?!(%d)\n", owner);
		ret = -ENODEV;
		goto err;
	}

	ret = setup_video(crt, i);

err:

	/* Change back. AST1510 RctBug#2012101900 */
	if (cts_7_33 != -1)
		i->xmiter_mode.HDMI_Mode = cts_7_33;

	return ret;
}

unsigned int crt_get_force_timing_idx(struct s_crt_drv *crt)
{
	unsigned int i;
	struct s_force_timing *ft = &crt->VE1_mode.force_timing;

#ifdef CONFIG_ARCH_AST1500_HOST
	BUG();
#endif

	if ((crt->available_screen & OWNER_VE1) && (FTCFG_IS_ENABLED(ft->cfg))) {
		i = ft->timing_table_idx;
	} else {
		/* Return invalid. */
		i = TIMING_INDEX_INVALID;
	}

	return i;
}

MODE_ITEM *crt_get_current_mode(struct s_crt_drv *crt, unsigned int owner, unsigned int *is_YUV420_out)
{
	int i;
	struct s_crt_info *info;
	MODE_ITEM *mt = crt->mode_table;

	switch (owner) {
#ifdef CONFIG_ARCH_AST1500_CLIENT
	case OWNER_CON:
		info = &crt->CON_mode;
		break;
	case OWNER_VE1:
		info = &crt->VE1_mode;
		break;
#endif
#ifdef CONFIG_ARCH_AST1500_HOST
	case OWNER_HOST:
		info = &crt->HOST_mode;
		break;
#endif
	default:
		uerr("un-supported screen?!(%d)\n", owner);
		/*
		 * only client VE need to call crt_get_current_mode()
		 * => BUG() here let programmer check further
		 */
		BUG();
		return NULL;
	}


	i = lookupModeTable(crt, info);

	if (is_YUV420_out)
		*is_YUV420_out = (info->crt_output_format == YUV420_FORMAT)?1:0;

	return mt + i;
}

unsigned int _crt_setup_audio(struct s_crt_drv *crt, unsigned int owner)
{
	struct s_crt_info *i;
	uinfo("crt_setup_audio (%d)\n", owner);
	switch (owner) {
#ifdef CONFIG_ARCH_AST1500_CLIENT
	case OWNER_CON:
		i = &crt->CON_mode;
		break;
	case OWNER_VE1:
		i = &crt->VE1_mode;
		break;
#endif
#ifdef CONFIG_ARCH_AST1500_HOST
	case OWNER_HOST:
		i = &crt->HOST_mode;
		break;
#endif
	default:
		uerr("un-supported screen?!(%d)\n", owner);
		return 0;
	}
	crt->audio_ready = 1;

	if ((crt->current_owner == owner) && (i->video_ready))
	{
		return xHal_setup_audio_xmiter(crt);
	}
	else
	{
		uinfo("video not ready yet\n");

		if (crt->current_owner == OWNER_CON) {
			/*
			 * always set audio if console screen
			 * this is for the special audio hybrid case: console screen is HDMI
			 */
			return xHal_setup_audio_xmiter(crt);
		}
		return 0;
	}
}
u32 crt_setup_audio(struct s_crt_drv *crt, unsigned int owner)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_setup_audio, (void *)owner, 0);
	return 0;
}

unsigned int _crt_disable_audio(struct s_crt_drv *crt, unsigned int owner)
{
	uinfo("crt_disable_audio (%d)\n", owner);
	if (crt->current_owner == owner)
		return xHal_setup_audio_xmiter(crt);
	else
		return 0;
}
u32 crt_disable_audio(struct s_crt_drv *crt, unsigned int owner)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_disable_audio, (void *)owner, 0);
	return 0;
}

static void crt_update_terminal_cnt(struct s_crt_drv *crt, struct s_crt_info *i, unsigned x_scale, unsigned int x_rem_pixel, unsigned int has_downscale)
{
	/*
	** Bruce141210. New approach to get correct terminal count.
	** Since scale_factor = ((Hsrc - 1) * CRT_SCALE_FACTOR_MAX)/(Hdest - 1)
	** So, we can caculate Hsrc from scale_factor and Hdest.
	** FIXME:
	** I'm not sure whether this formula is correct or not.
	*/
	unsigned int Hdest = ((GetSCUReg(CRT2_HOR_TOTAL_END_REG) & HOR_ENABLE_END_MASK) >> HOR_ENABLE_END_BIT) + 1;
	unsigned int tc_yuv444;
	unsigned int pixel_required;

	if (ast_scu.ability.v_crt_flip < 3)
		x_rem_pixel = 0;

	/* pixel_required = ((x_scale * (timing_width - 1) + (CRT_SCALE_FACTOR_MAX >> 1)) / CRT_SCALE_FACTOR_MAX) + 1 + x_rem_pixel; */
	pixel_required = ((x_scale * (Hdest - 1) + (CRT_SCALE_FACTOR_MAX >> 1)) >> CRT_SCALE_FACTOR_MAX_SHIFT_BITS) + 1 + x_rem_pixel;

	/*
	** Bruce150507. SoC V3 A1.
	** For 4K YUV 420 format, CRT engine have to fetch x1.5 more pixels.
	*/
	if (i->crt_output_format == YUV420_FORMAT)
		pixel_required += (pixel_required >> 1);

	uinfo("update terminal count Hs=%d, Hd=%d\n", pixel_required, Hdest);

	tc_yuv444 = CALC_TERMINAL_CNT(pixel_required, 24);

	__mod_reg84(crt, (tc_yuv444 << TERMINAL_COUNT_BIT), TERMINAL_COUNT_MASK);

#if (CONFIG_AST1500_SOC_VER >= 3)
	if (ast_scu.ability.v_crt_flip >= 3) {
		SetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT,
				VEFLIP_YUV444_TC(tc_yuv444)
				| VEFLIP_YUV422_TC(CALC_TERMINAL_CNT(pixel_required, 16)));
	}
#endif
}

#if (CONFIG_AST1500_SOC_VER >= 3)
#ifdef VW_DEBUG
#define uinfo_vw uinfo
#else
#define uinfo_vw(fmt, args...) do {} while(0)
#endif

static inline int _calc_rotate_y_compensate(struct s_crt_drv *crt, struct s_crt_info *i, u32 yuv422)
{
	unsigned int v_rem_pixel = 0;
	u32 capture_height, vertical_align;

	/*
	** SoC > V2, YUV422 fb format is 16x8 tile. Each slice keeps as 8 pixels
	** height.
	** Bruce150327. From above info, I think h_rem_pixel should be 8 pixels align.
	** Not sure why previous code use 16 pixels align?? ==> See below.
	*/
	/*
	** according to H.J.:
	** Although both YUV444 and YUV422's slice is 8 pixels height in frame buffer memory,
	** VE encode/decode engine actually uses:
	**	YUV422: video engine compress block is 16x16 => v_rem_pixel: 16 pixels align
	**	YUV444: video engine compress block is   8x8 => v_rem_pixel:  8 pixels align
	**
	** For some rotate cases,
	** We may get a gray bar in the bottom of original picture if Vactive is not 16 pixel-align in YUV422
	** So _calc_rotate_y_compensate() will return a value for further processing
	*/

	if (yuv422)
		vertical_align = 0xF;
	else
		vertical_align = 0x7;

	switch (crt->vw_rotate) {
	case V_FLIP:
	case RT_180:
		capture_height = i->height;
		v_rem_pixel = capture_height & vertical_align;
		if (v_rem_pixel)
			v_rem_pixel = vertical_align + 1 - v_rem_pixel;
		break;

	case RT_270:
		capture_height = i->width;
		v_rem_pixel = capture_height & vertical_align;
		if (v_rem_pixel)
			v_rem_pixel = vertical_align + 1 - v_rem_pixel;
		break;
	default:
		break;
	}

	return v_rem_pixel;
}

static inline int _calc_rotate_x_compensate(struct s_crt_drv *crt, struct s_crt_info *i)
{
	unsigned int h_rem_pixel = 0;
	u32 capture_width, horizontal_align, yuv422;

	switch (crt->vw_rotate) {
	case RT_90:
	case SLASH:
		/*
		** SoC > V2, YUV422 fb format is 16x8 tile. Each slice keeps as 8 pixels
		** height.
		** Bruce150327. From above info, I think h_rem_pixel should be 8 pixels align.
		** Not sure why previous code use 16 pixels align??
		** Bruce150617. Although tile format is 8 pixels aligned, VE decompress
		** is 16x16 pixels under 420 mode. So, it should be 16 pixles aligned
		** under 420 mode and 8 pixels aligned under YUV444 mode. There is
		** no way for SoC V3 to workaround this limitation.
		*/
		capture_width = i->height;
		h_rem_pixel = capture_width & 0x7UL;
		break;
	case RT_180:
		/*
		 * YUV444 8x8 tile => width is 8 pixel align
		 * YUV422 16x8 tile => width is 16 pixel align
		 * no solution for this limitation, just consider about 444 here
		 */
		yuv422 = 0;
		if (yuv422)
			horizontal_align = 0xF;
		else
			horizontal_align = 0x7;

		capture_width = i->width;
		h_rem_pixel = capture_width & horizontal_align;

		if (h_rem_pixel)
			h_rem_pixel = horizontal_align - h_rem_pixel;

		break;
	default:
		break;
	}

	return h_rem_pixel;
}


#define STRETCH_HINT_AUTO         0
#define STRETCH_HINT_STRETCH_OUT  1
#define STRETCH_HINT_FIT_IN       2

#define STRETCH_OFF   0
#define STRETCH_X     1
#define STRETCH_Y     2
#define STRETCH_ALL   3

static unsigned int choose_stretch_type(struct s_crt_drv *crt, unsigned int vw_enabled, unsigned int x_scale, unsigned int y_scale)
{
	unsigned int type;
	unsigned int max_row = crt->vw_max_row;
	unsigned int max_col = crt->vw_max_col;
	eVIDEO_ROTATE rotate = crt->vw_rotate;
	unsigned int is_special_rotate = (rotate >= B_SLASH) && (rotate <= SLASH);
	unsigned int hint = crt->vw_stretch_type;
#ifdef VW_DEBUG
	unsigned char *s[] = {
		"off",
		"stretch x",
		"stretch y",
		"stretch all"
	};
#endif
	/* If video wall doesn't keep aspect ratio. Decide which stretch type. */
	if (max_col > max_row) {

#ifdef VW_OLD_STRETCH_SELECT
		type = STRETCH_X;
#else
		/* Smaller scale factor means scale more. */
		if (x_scale > y_scale)
			type = STRETCH_Y;
		else
			type = STRETCH_X;
#endif
		/* Hint overwrite. */
		if (hint == STRETCH_HINT_FIT_IN)
			type = STRETCH_ALL;

	} else if (max_col < max_row) {

		/* Typical cases. */
#ifdef VW_OLD_STRETCH_SELECT
		type = STRETCH_Y;
#else
		/* Smaller scale factor means scale more. */
		if (x_scale > y_scale)
			type = STRETCH_Y;
		else
			type = STRETCH_X;
#endif

		/* A special case for 3x1 wall and rotate 270/90, which has similar 16:9 ratio */
		if (is_special_rotate) {
			/*
			 * video wall 2x1 case with 4K60Hz video source, capture:1920x2160
			 * 'row & column check' will choose STRETCH_X
			 * and caused scaled image cannot cover all screen in the bottom of VW
			 *
			 * => disable 'row & column check'
			 */
#if 0
			if ((max_row + 1) < ((max_col + 1) * 3))
				type = STRETCH_X;
			else if ((max_row + 1) > ((max_col + 1) * 3))
				type = STRETCH_Y;
			else if (hint == STRETCH_HINT_STRETCH_OUT)
#else
			if (hint == STRETCH_HINT_STRETCH_OUT)
#endif
				/* Smaller scale factor means scale more. */
				if (x_scale > y_scale)
					type = STRETCH_Y;
				else
					type = STRETCH_X;
			else
				type = STRETCH_ALL;
		}

		/* Hint overwrite. */
		if (hint == STRETCH_HINT_FIT_IN)
			type = STRETCH_ALL;

	} else if (is_special_rotate) {
		/* 1x1, 2x2, 3x3,.. is same aspect ratio but rotate. */
		type = STRETCH_X;

		/* Hint overwrite. */
		/*
		** For the case of "force stretch all, but need downscale". We don't take the hint.
		*/
		if ((hint == STRETCH_HINT_FIT_IN) && (y_scale < CRT_SCALE_FACTOR_MAX))
			type = STRETCH_ALL;
	}

	/* video wall and keep the same aspect ratio and no special rotate type */
	else if (vw_enabled)
		type = STRETCH_ALL;

	/* non-video wall, no special rotate, normal case. */
	else
		type = STRETCH_OFF;

	uinfo_vw("Stretch type = %s\n", s[type]);
	return type;
}

static unsigned int calc_x_scale_factor(
    struct s_crt_drv *crt,
    struct s_crt_info *i,
    unsigned int vw_enabled,
    unsigned short target_width,
    unsigned int downscale_first,
    unsigned int *x_src_pixel)
{
	unsigned int targetPixel;
	unsigned int srcPixel;
	unsigned int total_width_factor;
	unsigned int ha = crt->vw_fc_h_active;
	unsigned int ht = crt->vw_fc_h_total;
	unsigned int max_col = crt->vw_max_col;
	unsigned short source_width = i->width;
	unsigned int x_scale = CRT_SCALE_FACTOR_MAX;

	if ((crt->vw_rotate >= B_SLASH) && (crt->vw_rotate <= SLASH))
		source_width = i->height;

	if (downscale_first)
		source_width >>= 1;

	total_width_factor = ha + (ht * max_col);
	/*
	** Bruce150113. We assume ht and vt will always < 2048 (11 bits long).
	** Otherwise following formula may overflow. And 64bits op will
	** be required.
	*/
	srcPixel = (((ha * source_width) + (total_width_factor >> 1)) / total_width_factor) - 1;
	targetPixel = target_width - 1;
	if (vw_enabled)
		targetPixel += crt->vw_extra_h_scale_up;

	/*
	** Under timing hybrid mode, srcPixel may > targetPixel.
	** We return x_scale value even the value is > CRT_SCALE_FACTOR_MAX.
	** The caller use x_scale value to determine whether down scale is needed.
	**
	** if (srcPixel < targetPixel) <- skip this
	*/
	x_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;

	uinfo_vw("H.SrcPixel = (((%d * %d) + (%d >> 1)) / %d) - 1 = %d\n",	ha, source_width, total_width_factor, total_width_factor, srcPixel);
	uinfo_vw("x_scale = (%d * %d) / %d = %d (0x%08X)\n",	srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, x_scale, x_scale);

	*x_src_pixel = srcPixel;
	return x_scale;
}

static unsigned int calc_y_scale_factor(
    struct s_crt_drv *crt,
    struct s_crt_info *i,
    unsigned int vw_enabled,
    unsigned short target_height,
    unsigned int downscale_first)
{
	unsigned int targetPixel;
	unsigned int srcPixel;
	unsigned int total_height_factor;
	unsigned int va = crt->vw_fc_v_active;
	unsigned int vt = crt->vw_fc_v_total;
	unsigned int max_row = crt->vw_max_row;
	unsigned short source_height = i->height;
	unsigned int y_scale = CRT_SCALE_FACTOR_MAX;

	if ((crt->vw_rotate >= B_SLASH) && (crt->vw_rotate <= SLASH))
		source_height = i->width;

	if (downscale_first)
		source_height >>= 1;

	total_height_factor = va + (vt * max_row);
	/*
	** Bruce150113. We assume ht and vt will always < 2048 (11 bits long).
	** Otherwise following formula may overflow. And 64bits op will
	** be required.
	*/
	srcPixel = (((va * source_height) + (total_height_factor >> 1)) / total_height_factor) - 1;
	targetPixel = target_height - 1;
	if (vw_enabled)
		targetPixel += crt->vw_extra_v_scale_up;

	/*
	** Under timing hybrid mode, srcPixel may > targetPixel.
	** We return y_scale value even the value is > CRT_SCALE_FACTOR_MAX.
	** The caller use y_scale value to determine whether down scale is needed.
	**
	** if (srcPixel < targetPixel) <- skip this
	*/
	y_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;

	uinfo_vw("V.SrcPixel = (((%d * %d) + (%d >> 1)) / %d) - 1 = %d\n", va, source_height, total_height_factor, total_height_factor, srcPixel);
	uinfo_vw("ver_scale = (%d * %d) / %d = %d (0x%08X)\n", srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, y_scale, y_scale);

	return y_scale;
}

static unsigned int calc_x_start_position(
    struct s_crt_drv *crt,
    struct s_crt_info *i,
    unsigned int stretch_type,
    unsigned int x_scale,
    unsigned short target_width,
    unsigned int downscale_first)
{
	int x_start = 0;
	int h_shift = 0;
	/*
	** SoC > V2, YUV422 fb format is 16x8 tile. Each slice keeps as 8 pixels
	** height.
	*/
	if (ast_scu.ability.v_crt_flip >= 3)
		h_shift -= _calc_rotate_x_compensate(crt, i);

	/*
	** Handle cases:
	** For video wall RxC case where:
	** 1.1. R == C: STRETCH_ALL
	** 1.2. R > C: STRETCH_Y
	** 1.3. R < C: STRETCH_X
	** 2. 90,270 rotate: STRETCH_X
	*/
	if (stretch_type != STRETCH_OFF) {
		unsigned int ha = crt->vw_fc_h_active;
		unsigned int ht = crt->vw_fc_h_total;
		unsigned int max_col = crt->vw_max_col;
		unsigned int col_num = crt->vw_col_num % (max_col + 1);
		unsigned short source_width = i->width;

		/* Extra panning is supported under both video wall and rotate case. */
		h_shift += crt->vw_h_shift;

		if ((crt->vw_rotate >= B_SLASH) && (crt->vw_rotate <= SLASH)) {
			source_width = i->height;
		}

		if (stretch_type == STRETCH_Y) {
			/* x_scale is actually y_scale */
			unsigned int target_pixel;
			long long src_pixel;

			target_pixel = (target_width * ht * col_num + (ha >> 1)) / ha;
			src_pixel = ((long long)x_scale * ((long long)target_pixel - 1) + (long long)CRT_SCALE_FACTOR_MAX);
			/* do_div(src_pixel, CRT_SCALE_FACTOR_MAX); */
			src_pixel >>= CRT_SCALE_FACTOR_MAX_SHIFT_BITS;

			if (downscale_first)
				src_pixel <<= 1;

			uinfo_vw("Stretch Y: x_start = (%d * (%d - 1) + %d) >> %d = %lld\n", x_scale, target_pixel, CRT_SCALE_FACTOR_MAX, CRT_SCALE_FACTOR_MAX_SHIFT_BITS, src_pixel);
			/*
			** Considering the case: 3x2 wall + r0c1 + rotate 90.
			*/
			if (src_pixel > source_width)
				src_pixel = 0;
			x_start = (int)src_pixel;
		} else {
			unsigned int total_width_factor;
			total_width_factor = ha + (ht * max_col);
			x_start = ((source_width * col_num * ht) + (total_width_factor >> 1)) / total_width_factor;
		}
	}

	if (x_start > h_shift)
		x_start -= h_shift; // 8 pixels align
	else
		x_start = 0;

	return x_start;
}

static unsigned int _calc_y_start_position(
    struct s_crt_drv *crt,
    struct s_crt_info *i,
    unsigned int stretch_type,
    unsigned int y_scale,
    unsigned short target_height,
    unsigned int downscale_first,
    unsigned int yuv422)
{
	int y_start = 0;
	int v_shift = 0;

	if (ast_scu.ability.v_crt_flip >= 3)
		v_shift -= _calc_rotate_y_compensate(crt, i, yuv422);

	/*
	** Handle cases:
	** For video wall RxC case where:
	** 1.1. R == C: STRETCH_ALL
	** 1.2. R > C: STRETCH_Y
	** 1.3. R < C: STRETCH_X
	** 2. 90,270 rotate: STRETCH_X
	*/
	if (stretch_type != STRETCH_OFF) {
		unsigned int va = crt->vw_fc_v_active;
		unsigned int vt = crt->vw_fc_v_total;
		unsigned int max_row = crt->vw_max_row;
		unsigned int row_num = crt->vw_row_num % (max_row + 1);
		unsigned short source_height = i->height;

		/* Extra panning is supported under both video wall and rotate case. */
		v_shift += crt->vw_v_shift;

		if ((crt->vw_rotate >= B_SLASH) && (crt->vw_rotate <= SLASH)) {
			source_height = i->width;
		}

		if (stretch_type == STRETCH_X) {
			/* y_scale is actually x_scale */
			unsigned int target_pixel;
			long long src_pixel;

			target_pixel = (target_height * vt * row_num + (va >> 1)) / va;
			src_pixel = ((long long)y_scale * ((long long)target_pixel - 1) + (long long)CRT_SCALE_FACTOR_MAX);
			/* do_div(src_pixel, CRT_SCALE_FACTOR_MAX); */
			src_pixel >>= CRT_SCALE_FACTOR_MAX_SHIFT_BITS;

			if (downscale_first)
				src_pixel <<= 1;

			uinfo_vw("Stretch X: y_start = (%d * (%d - 1) + %d) >> %d = %lld\n", y_scale, target_pixel, CRT_SCALE_FACTOR_MAX, CRT_SCALE_FACTOR_MAX_SHIFT_BITS, src_pixel);
			if (src_pixel > source_height)
				src_pixel = 0;
			y_start = (int)src_pixel;
		} else {
			unsigned int total_height_factor;
			total_height_factor = va + (vt * max_row);
			y_start = ((source_height * row_num * vt) + (total_height_factor >> 1)) / total_height_factor;
		}
	}

	if (y_start > v_shift)
		y_start -= v_shift; // 8 pixels align
	else
		y_start = 0;

	return y_start;
}

#define calc_y_start_position(a, b, c, d, e, f) _calc_y_start_position(a, b, c, d, e, f, 0)
#define calc_y_start_position_yuv422(a, b, c, d, e, f) _calc_y_start_position(a, b, c, d, e, f, 1)

static unsigned int crt_panning_to(struct s_crt_drv *crt, struct s_crt_info *i, unsigned int x_start, unsigned int y_start, unsigned int y_start_yuv422, unsigned int has_downscale)
{
	unsigned int buf_offset, buf_offset_yuv422;
	unsigned int x_rem_pixel, y_rem_pixel, y_rem_pixel_yuv422;
	unsigned int y_offset, y_offset_yuv422;
	unsigned int display_offset;
	unsigned int x_pan, y_pan, y_pan_yuv422;

	 /* CRT display offset = line offset * bpp / 8. Assume bpp = 32. */
	if ((crt->vw_rotate >= B_SLASH) && (crt->vw_rotate <= SLASH))
		display_offset = (i->line_offset_rotate << 2) & ~DISPLAY_OFFSET_ALIGN;
	else
		display_offset = (i->line_offset << 2) & ~DISPLAY_OFFSET_ALIGN;

	/*
	** SoC > V2, YUV422 fb format is 16x8 tile. Each slice keeps as 8 pixels
	** height.
	*/
	x_rem_pixel = x_start & (0xFUL); // 16pixels align
	x_start = (x_start) & (~(0xFUL));
	/* Panning uses the value after downscale, which means it will pan 2 pixels when pan set 1. */
	x_pan = (has_downscale)?(x_rem_pixel >> 1):(x_rem_pixel);
	uinfo_vw("X start at %d+%d(%d) pixel\n", x_start, x_rem_pixel, x_pan);
	buf_offset = x_start << 5; // 32bits per pixel and 8 lines tiling
	//buf_offset_yuv422 = (buf_offset & (~(x_align - 1))) * 2 * 8; // YYUV 8888 bytes. 32bytes in total. 16x8 tile YUV422
	buf_offset_yuv422 = buf_offset >> 1;

	y_rem_pixel = y_start & 0x7UL;
	y_start = y_start & (~0x7UL); // 8 pixels align
	/* Panning uses the value after downscale, which means it will pan 2 pixels when pan set 1. */
	y_pan = (has_downscale)?(y_rem_pixel >> 1):(y_rem_pixel);
	uinfo_vw("Y start at %d+%d(%d) line\n", y_start, y_rem_pixel, y_pan);
	//Bruce110829. Bug fix for width not 32 pixel aligned. The same as PITCH_BUG.
	//voffset = voffset * width * 4; // use one slice (8pixel high) at a time.
	y_offset = y_start * display_offset; // use one slice (8pixel high) at a time.
	buf_offset += y_offset;


	y_rem_pixel_yuv422 = y_start_yuv422 & 0x7UL;
	y_start_yuv422 = y_start_yuv422 & (~0x7UL); /* 8 pixels align */
	y_pan_yuv422 = (has_downscale)?(y_rem_pixel_yuv422 >> 1):(y_rem_pixel_yuv422);
	uinfo_vw("Y start at %d+%d(%d) line (YUV422)\n", y_start_yuv422, y_rem_pixel_yuv422, y_pan_yuv422);
	y_offset_yuv422 = y_start_yuv422 * display_offset; /* use one slice (8pixel high) at a time. */
	buf_offset_yuv422 += y_offset_yuv422;

	/* Update pitch value for rotate case. */
	_crt_display_offset_set(crt, display_offset);

	if (ast_scu.ability.v_crt_flip >= 3) {
		/*
		** Bruce150113. Patch. Y panning of CRT84[0:2] need to patch the starting
		** base address to align with skipped data. The formula is:
		** YUV444 8x8 tile: v_rem_pixel * ( 8 * bpp/8 ) bytes
		** YUV422 16x8 tile: v_rem_pixel * ( 16 * bpp/8 ) bytes
		** So, both YUV444 and YUV422 add extra "v_rem_pixel * 32" bytes.
		**
		** Bruce150617. For 1/2 downscale case, the extra bytes is 64 bytes.
		** AND only Y_PAN bit[0:1] is valid.
		*/
		if (has_downscale) {
			y_pan &= 0x3UL;
			y_pan_yuv422 &= 0x3UL;
			buf_offset += y_pan << 6;
			buf_offset_yuv422 += y_pan_yuv422 << 6;
		} else {
			buf_offset += y_pan << 5;
			buf_offset_yuv422 += y_pan_yuv422 << 5;
		}

		/*
		** Bruce160105.
		** For SOC_OP_MODE == 1, 'VE Flip' is disabled so that CRT2_VEFLIP_YUV444_BASE_OFFSET
		** and CRT2_VEFLIP_YUV422_BASE_OFFSET has no effect. We should use
		** CRT2_DISPLAY_ADDRESS instead.
		**
		** SOC_OP_MODE == 1 doesn't have YUV444/422 switch, so it is OK that
		** we don't need CRT2_VEFLIP_YUV422_BASE_OFFSET feature.
		*/

		/*
		 * no matter whether'VE Flip' is enabled
		 * set CRT2_VEFLIP_YUV444_BASE_OFFSET and CRT2_VEFLIP_YUV444_BASE_OFFSET if support double buffer feature
		 */
		SetSCUReg(CRT2_VEFLIP_YUV444_BASE_OFFSET, buf_offset);
		SetSCUReg(CRT2_VEFLIP_YUV422_BASE_OFFSET, buf_offset_yuv422);

		/* Resolve 8,16 bits alignment limitation by pixel panning. */
		/*
		** Since V.Pan requires v_crt_flip == 3 feature and for feature consistence,
		** we only enable panning when v_crt_flip is enabled.
		*/
		ModSCUReg(CRT2_PANNING, X_PAN(x_pan), (X_PAN_MASK | Y_SYNC_PAN_MASK));
		__mod_reg84(crt, Y_PAN(y_pan), BLKINIV_MASK);
	}

	/*
	 * Following code about _crt_set_vbuf() should like
	 * if (VE Flip is ON)
	 *	_crt_set_vbuf(crt, crt->dec_buf_base);
	 * else
	 *	_crt_set_vbuf(crt, crt->dec_buf_base + buf_offset);
	 *
	 * When VE Flip is ON,
	 *	CRT display address is DECODE base address + CRT2_VEFLIP_YUV444_BASE_OFFSET (or CRT2_VEFLIP_YUV422_BASE_OFFSET)
	 *	because the base address is from VE, the content of CRT2_DISPLAY_ADDRESS is useless => no need to set it
	 * When VE Flip is OFF,
	 *	CRT display address is DECODE base address + buf_offset
	 *	CRT2_VEFLIP_YUV444_BASE_OFFSET and CRT2_VEFLIP_YUV422_BASE_OFFSET has no effect
	 *	we should use CRT2_DISPLAY_ADDRESS instead, so record buf_offset for the configuration of disable VE flip
	 */
	crt->dec_buf_offset = buf_offset;
	_crt_set_vbuf(crt, crt->dec_buf_base + buf_offset);

	/*
	** The return value will be used to calculate terminal count, so we should
	** return the pixel value before downscale.
	*/
	return x_rem_pixel;
}

static void vw_param_fixup(struct s_crt_drv *crt)
{
	if ((crt->vw_fc_h_active == 0) || (crt->vw_fc_h_total == 0))
		crt->vw_fc_h_active = crt->vw_fc_h_total = 1;
	if ((crt->vw_fc_v_active == 0) || (crt->vw_fc_v_total == 0))
		crt->vw_fc_v_active = crt->vw_fc_v_total = 1;

	if (crt->vw_fc_h_active > crt->vw_fc_h_total)
		crt->vw_fc_h_active = crt->vw_fc_h_total = 1;
	if (crt->vw_fc_v_active > crt->vw_fc_v_total)
		crt->vw_fc_v_active = crt->vw_fc_v_total = 1;

	/* FIXME: Correct the range of ha,ht,va,vt. */
	/*
	** Bruce150113. We assume ht and vt will always < 2048 (11 bits long).
	** Otherwise following formula may overflow. And 64bits op will
	** be required.
	*/
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#if (CONFIG_AST1500_SOC_VER >= 3)
#define HAS_DOWN_SCALE(x_scale, y_scale) (((x_scale) > CRT_SCALE_FACTOR_MAX) || ((y_scale) > CRT_SCALE_FACTOR_MAX))
#define NEED_DOWN_SCALE(x_src_pixel, y_scale) ((x_src_pixel > 1920) && (y_scale < CRT_SCALE_FACTOR_MAX))
#endif

#if defined(VW_API_V2_SUPPORT)
static u32 x_start_position(struct s_crt_drv *crt, struct s_crt_info *i)
{
	u32 x_start = 0, h_shift = 0, start, source_size;
	/*
	** SoC > V2, YUV422 fb format is 16x8 tile. Each slice keeps as 8 pixels
	** height.
	*/
	if (ast_scu.ability.v_crt_flip >= 3)
		h_shift -= _calc_rotate_x_compensate(crt, i);

	switch (crt->vw_rotate) {
	case RT_270:
		source_size = i->height;
		start = crt->vw_top_left_y;
		break;
	case RT_90:
		source_size = i->height;
		start = VW_COORDINATE_MAX - crt->vw_bottom_right_y;
		break;
	case RT_180:
		source_size = i->width;
		start = VW_COORDINATE_MAX - crt->vw_bottom_right_x;
		break;
	default:
		source_size = i->width;
		start = crt->vw_top_left_x;
		break;
	}

	/* round off by adding denominator / 2  */
	x_start = (source_size * start + (VW_COORDINATE_MAX >> 1)) / VW_COORDINATE_MAX;

	if (x_start > h_shift)
		x_start -= h_shift; /* 8 pixels align */
	else
		x_start = 0;

	return x_start;
}

#define y_start_position(a, b) _y_start_position(a, b, 0)
#define y_start_position_yuv422(a, b) _y_start_position(a, b, 1)

static u32 _y_start_position(struct s_crt_drv *crt, struct s_crt_info *i, u32 yuv422)
{
	int y_start = 0, v_shift = 0, start, source_size;

	if (ast_scu.ability.v_crt_flip >= 3)
		v_shift -= _calc_rotate_y_compensate(crt, i, yuv422);

	switch (crt->vw_rotate) {
	case RT_270:
		source_size = i->width;
		start = VW_COORDINATE_MAX - crt->vw_bottom_right_x;
		break;
	case RT_90:
		source_size = i->width;
		start = crt->vw_top_left_x;
		break;
	case RT_180:
		source_size = i->height;
		start = VW_COORDINATE_MAX - crt->vw_bottom_right_y;
		break;
	default:
		source_size = i->height;
		start = crt->vw_top_left_y;
		break;
	}

	/* round off by adding denominator / 2  */
	y_start = (source_size * start + (VW_COORDINATE_MAX >> 1)) / VW_COORDINATE_MAX;

	if (y_start > v_shift)
		y_start -= v_shift; /* 8 pixels align */
	else
		y_start = 0;

	return y_start;
}

static u32 _scale_factor(struct s_crt_drv *crt, struct s_crt_info *i, u32 target, u32 downscale_first, u32 is_x, u32 *src_pixel)
{
	u32 scale, src, source_size, share;

	switch (crt->vw_rotate) {
	case RT_270:
	case RT_90:
		if (is_x) {
			source_size = i->height;
			share = crt->vw_bottom_right_y - crt->vw_top_left_y;
		} else {
			source_size = i->width;
			share = crt->vw_bottom_right_x - crt->vw_top_left_x;
		}
		break;
	case RT_180:
	default:
		if (is_x) {
			source_size = i->width;
			share = crt->vw_bottom_right_x - crt->vw_top_left_x;
		} else {
			source_size = i->height;
			share = crt->vw_bottom_right_y - crt->vw_top_left_y;
		}
		break;
	}

	/* the volume of share is x2 - x1  (or y2 - y1), DONOT +1 */
#if 0
	share += 1;
#endif

	if (downscale_first)
		source_size >>= 1;

	src = (source_size * share  + (VW_COORDINATE_MAX >> 1)) / VW_COORDINATE_MAX;


	if (src == 0) {
		/* fix src (force to 1) if the range from user is too small to include one pixel */
		src = 1;
	}

	if (is_x)
		*src_pixel = src;

	scale = (src - 1) * CRT_SCALE_FACTOR_MAX / (target - 1);

	return scale;
}

#define x_scale_factor(a, b, c, d, e) _scale_factor(a, b, c, d, 1, e)
#define y_scale_factor(a, b, c, d) _scale_factor(a, b, c, d, 0, NULL)

static void _crt_vw_enable_v2(struct s_crt_drv *crt, struct s_crt_info *i)
{
	unsigned int x_scale, y_scale;
	unsigned int x_start, y_start, y_start_yuv422;
	unsigned int x_rem_pixel, x_src_pixel;
	unsigned short target_width, target_height;
	MODE_ITEM *mode;
	unsigned int downscale_first = 0;

	if (crt->current_owner != OWNER_VE1)
		return;

	mode = crt_get_current_mode(crt, OWNER_VE1, NULL);
	target_width = mode->HActive;
	target_height = mode->VActive;

	x_scale = x_scale_factor(crt, i, target_width, downscale_first, &x_src_pixel);
	y_scale = y_scale_factor(crt, i, target_height, downscale_first);

	if (HAS_DOWN_SCALE(x_scale, y_scale) | NEED_DOWN_SCALE(x_src_pixel, y_scale)) {
		downscale_first = 1;
		x_scale = x_scale_factor(crt, i, target_width, downscale_first, &x_src_pixel);
		y_scale = y_scale_factor(crt, i, target_height, downscale_first);
	}

	/* Remove invalid scaling values. */
	if (x_scale > CRT_SCALE_FACTOR_MAX)
		x_scale = CRT_SCALE_FACTOR_MAX;
	if (y_scale > CRT_SCALE_FACTOR_MAX)
		y_scale = CRT_SCALE_FACTOR_MAX;

	/* CRT line buf has only 1920 pixel. We disable y_scale when x_src_pixel > 1920. */
	if ((x_src_pixel > 1920) && (y_scale < CRT_SCALE_FACTOR_MAX)) {
		uinfo("Y scale doesn't support X > 1920 pixels!\n");
		y_scale = CRT_SCALE_FACTOR_MAX;
	}

	x_start = x_start_position(crt, i);
	y_start = y_start_position(crt, i);
	y_start_yuv422 = y_start_position_yuv422(crt, i);

	x_rem_pixel = crt_panning_to(crt, i, x_start, y_start, y_start_yuv422, downscale_first);
	__crt_change_scale_factor(crt, x_scale, y_scale, x_rem_pixel, downscale_first);
}
#endif /* #if defined(VW_API_V2_SUPPORT) */

#if (CONFIG_AST1500_SOC_VER >= 3)
void _crt_vw_enable(struct s_crt_drv *crt, struct s_crt_info *i)
{
	unsigned int vw_enabled = crt->vw_enabled;
	unsigned int stretch_type;
	unsigned int x_scale, y_scale;
	unsigned int x_start, y_start, y_start_yuv422;
	unsigned int x_rem_pixel;
	unsigned short target_width, target_height;
	MODE_ITEM *mode;
	unsigned int downscale_first = 0;
	unsigned int x_src_pixel;

	if (crt->current_owner != OWNER_VE1)
		return;

#if defined(VW_API_V2_SUPPORT)
	if (crt->vw_ver == 2) {
		_crt_vw_enable_v2(crt, i);
		return;
	}
#endif

	vw_param_fixup(crt);

	mode = crt_get_current_mode(crt, OWNER_VE1, NULL);
	target_width = mode->HActive;
	target_height = mode->VActive;

	x_scale = calc_x_scale_factor(crt, i, vw_enabled, target_width, downscale_first, &x_src_pixel);
	y_scale = calc_y_scale_factor(crt, i, vw_enabled, target_height, downscale_first);

	/*
	** Bruce150529. Support half downscale. SoC V3.
	** If we found downscale is required, normally we will take 'stretch out' option.
	** But if user insist using 'fit in', then we need to do downscale.
	*/
	if (((crt->vw_stretch_type == STRETCH_HINT_FIT_IN) && HAS_DOWN_SCALE(x_scale, y_scale))
		| NEED_DOWN_SCALE(x_src_pixel, y_scale)) {
		/* Means we need downscale. */
		uinfo("Need half downscale!\n");
		downscale_first = 1;
		x_scale = calc_x_scale_factor(crt, i, vw_enabled, target_width, downscale_first, &x_src_pixel);
		y_scale = calc_y_scale_factor(crt, i, vw_enabled, target_height, downscale_first);

#if 0
		/* Which scenario will hit this case? I think downscale is better, so remove following code. */
		if (HAS_DOWN_SCALE(x_scale, y_scale) | NEED_DOWN_SCALE(x_src_pixel, y_scale)) {
			/* If half downscale is not enough, we give up and ignore half downscale. */
			uinfo("BUT half downscale is not small enough! Ignore.\n");
			downscale_first = 0;
			x_scale = calc_x_scale_factor(crt, i, vw_enabled, target_width, downscale_first, &x_src_pixel);
			y_scale = calc_y_scale_factor(crt, i, vw_enabled, target_height, downscale_first);
		}
#endif
	}

	stretch_type = choose_stretch_type(crt, vw_enabled, x_scale, y_scale);

	if (stretch_type == STRETCH_X)
		y_scale = x_scale;
	else if (stretch_type == STRETCH_Y)
		x_scale = y_scale;

	/* Remove invalid scaling values. */
	if (x_scale > CRT_SCALE_FACTOR_MAX)
		x_scale = CRT_SCALE_FACTOR_MAX;
	if (y_scale > CRT_SCALE_FACTOR_MAX)
		y_scale = CRT_SCALE_FACTOR_MAX;

	/* CRT line buf has only 1920 pixel. We disable y_scale when x_src_pixel > 1920. */
	if ((x_src_pixel > 1920) && (y_scale < CRT_SCALE_FACTOR_MAX)) {
		uinfo("Y scale doesn't support X > 1920 pixels!\n");
		y_scale = CRT_SCALE_FACTOR_MAX;
	}

	x_start = calc_x_start_position(crt, i, stretch_type, x_scale, target_width, downscale_first);
	y_start = calc_y_start_position(crt, i, stretch_type, y_scale, target_height, downscale_first);
	y_start_yuv422 = calc_y_start_position_yuv422(crt, i, stretch_type, y_scale, target_height, downscale_first);

	x_rem_pixel = crt_panning_to(crt, i, x_start, y_start, y_start_yuv422, downscale_first);

	if (stretch_type != STRETCH_OFF) {
		BUG_ON(x_scale > CRT_SCALE_FACTOR_MAX);
		BUG_ON(y_scale > CRT_SCALE_FACTOR_MAX);

		__crt_change_scale_factor(crt, x_scale, y_scale, x_rem_pixel, downscale_first);
	} else {
		//Bruce110830. Restore to the scale factor which was set from video driver.
		__crt_change_scale_factor(crt, crt->dec_hor_scale, crt->dec_ver_scale, 0, crt->dec_half_down_scale);
	}
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#if (CONFIG_AST1500_SOC_VER == 1)
#define AST1500_SCALE_FACTOR_FORMULA_NEW 0
void _crt_vw_enable(struct s_crt_drv *crt, struct s_crt_info *i)
{
	unsigned int max_row = crt->vw_max_row;
	unsigned int max_col = crt->vw_max_col;
	unsigned int row_num = crt->vw_row_num % (max_row + 1);
	unsigned int col_num = crt->vw_col_num % (max_col + 1);
	unsigned int ha = crt->vw_fc_h_active;
	unsigned int ht = crt->vw_fc_h_total;
	unsigned int va = crt->vw_fc_v_active;
	unsigned int vt = crt->vw_fc_v_total;
	unsigned int hor_scale = CRT_SCALE_FACTOR_MAX, ver_scale = CRT_SCALE_FACTOR_MAX;
	unsigned short width = i->width;
	unsigned short height = i->height;
	unsigned int vfactor = CRT_SCALE_FACTOR_MAX;
	int h_rem_pixel = 0;
	int v_rem_pixel = 0;
	unsigned int total_width_factor;
	unsigned int total_height_factor;
	unsigned short timing_width, timing_height;

	if (crt->current_owner != OWNER_VE1)
		return;

	if ((ha == 0) || (ht == 0))
		ha = ht = 1;
	if ((va == 0) || (vt == 0))
		va = vt = 1;

	timing_width = crt->mode_table[i->TimingTableIdx].HActive;
	timing_height = crt->mode_table[i->TimingTableIdx].VActive;

	total_width_factor = ha + (ht * max_col);
	total_height_factor = va + (vt * max_row);

	/* calculate vbuf starting base address. */
	if (max_row == 0 && max_col == 0) {
		// 1x1 case. Don't do any shift and frame compensation.
		_crt_set_vbuf(crt, crt->dec_buf_base);
	} else {
		int buf_offset = 0;
		int buf_offset_yuv422 = 0;
		unsigned int x_align = 8; //8 pixel for AST1500
		int h_shift = 0;

		h_shift = (crt->vw_h_shift * x_align);
		buf_offset = ((width * col_num * ht) + (total_width_factor >> 1)) / total_width_factor;

		if (buf_offset > h_shift)
			buf_offset -= h_shift; // 8 pixels align
		else
			buf_offset = 0;

		h_rem_pixel = buf_offset & (x_align - 1);
		buf_offset = (buf_offset) & (~(x_align - 1)); // 8 pixels align (8x8 for YUV444)
		uinfo("X start at %d+%d pixel\n", buf_offset, h_rem_pixel);
		//terminal_cnt = width - buf_offset;
		buf_offset <<= 5; // 32bits per pixel and 8 lines tiling
		//buf_offset_yuv422 = (buf_offset & (~(x_align - 1))) * 2 * 8; // YYUV 8888 bytes. 32bytes in total. 16x8 tile YUV422
		buf_offset_yuv422 = buf_offset >> 1;

		// Vertical
		{
			int voffset;
			int v_shift = 0;

			v_shift = (crt->vw_v_shift << 3); // 8 pixels align
			voffset = ((height * row_num * vt) + (total_height_factor >> 1)) / total_height_factor;

			if (voffset > v_shift)
				voffset -= v_shift; // 8 pixels align
			else
				voffset = 0;

			v_rem_pixel = voffset & 0x7UL;
			voffset = voffset & (~0x7UL); // 8 pixels align
			uinfo("Y start at %d+%d line\n", voffset, v_rem_pixel);
			//Bruce110829. Bug fix for width not 32 pixel aligned. The same as PITCH_BUG.
			//voffset = voffset * width * 4; // use one slice (8pixel high) at a time.
			voffset = voffset * ((i->line_offset) << 2); // use one slice (8pixel high) at a time.
			buf_offset += voffset;
			buf_offset_yuv422 += voffset;
		}
		_crt_set_vbuf(crt, crt->dec_buf_base + buf_offset);
	}

	if (max_row || max_col) {
		/* Video wall is enabled */
		/* calculate scaling factor. */
		unsigned int targetPixel;
		unsigned int srcPixel;
#if (1 == AST1500_SCALE_FACTOR_FORMULA_NEW)
		/*
		** Bruce150113. We assume ht and vt will always < 2048 (11 bits long).
		** Otherwise following formula may overflow. And 64bits op will
		** be required.
		*/
		srcPixel = (((ha * width) + (total_width_factor >> 1)) / total_width_factor) - 1;
		targetPixel = timing_width - 1 + crt->vw_extra_h_scale_up;
		hor_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;
		uinfo("H.SrcPixel = (((%d * %d) + (%d >> 1)) / %d) - 1 = %d\n",	ha, width, total_width_factor, total_width_factor, srcPixel);
		uinfo("hor_scale = (%d * %d) / %d = %d (0x%08X)\n", srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, hor_scale, hor_scale);

		srcPixel = (((va * height) + (total_height_factor >> 1)) / total_height_factor) - 1;
		targetPixel = timing_height - 1 + crt->vw_extra_v_scale_up;
		ver_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;
		uinfo("V.SrcPixel = (((%d * %d) + (%d >> 1)) / %d) - 1 = %d\n", va, height, total_height_factor, total_height_factor, srcPixel);
		uinfo("ver_scale = (%d * %d) / %d = %d (0x%08X)\n", srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, ver_scale, ver_scale);
#else

		//Bruce131119. AST1500 old formula. AST1520 formula could apply to AST1500(?).
		/*
		 * old code:
		 *	hor_scale = (hor_scale * ha)/(ha + (ht * max_col));
		 *	ver_scale = (ver_scale * va)/(va + (vt * max_row));
		 *	hor_scale = (hor_scale * width) / (timing_width + crt->vw_extra_h_scale_up);
		 *	ver_scale = (ver_scale * height) / (timing_height + crt->vw_extra_v_scale_up);
		 */
		srcPixel = (ha * width) / total_width_factor;
		targetPixel = timing_width + crt->vw_extra_h_scale_up;
		hor_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;
		uinfo("H.SrcPixel = (%d * %d) / %d = %d\n", ha, width, total_width_factor, srcPixel);
		uinfo("hor_scale = (%d * %d) / %d = %d (0x%08X)\n", srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, hor_scale, hor_scale);

		srcPixel = (va * height) / total_height_factor;
		targetPixel = timing_height + crt->vw_extra_v_scale_up;
		ver_scale = (srcPixel * CRT_SCALE_FACTOR_MAX) / targetPixel;
		uinfo("V.SrcPixel = (%d * %d / %d) = %d\n", va, height, total_height_factor, srcPixel);
		uinfo("ver_scale = (%d * %d) / %d = %d (0x%08X)\n", srcPixel, CRT_SCALE_FACTOR_MAX, targetPixel, ver_scale, ver_scale);
#endif
		vfactor = ver_scale;
		__crt_change_scale_factor(crt, hor_scale, ver_scale, 0, 0);

		/*
		** Bruce modify the terminal count here because he is lazy.
		*/
		/* FIXME: following terminal_cnt caculation may be buggy when host downscaled first. */
		{
			unsigned int pixel_required;

			pixel_required = (width + max_col) / (max_col + 1);

			__mod_reg84(crt,
			            (CALC_TERMINAL_CNT(pixel_required, 24) << TERMINAL_COUNT_BIT),
			            TERMINAL_COUNT_MASK);
		}

	} else {
		//Bruce110830. Restore to the scale factor which was set from video driver.
		__crt_change_scale_factor(crt, crt->dec_hor_scale, crt->dec_ver_scale, 0, crt->dec_half_down_scale);
	}
}
#endif /* #if (CONFIG_AST1500_SOC_VER == 1) */

#if (CONFIG_AST1500_SOC_VER != 2)
void crt_vw_enable(struct s_crt_drv *crt, struct s_crt_info *i)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_vw_enable, (void *)i, 0);
}
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
#if (CONFIG_AST1500_SOC_VER == 1)
static unsigned int _init_all_xmiter_host(struct s_crt_drv *crt)
{
	unsigned int i;

	//determine video transmitter
	for (i = 0; i < NUM_XMITER; i++)
	{
		crt->tx_select = i;
		if ((crt->tx_exist = xHal_xmiter_exist(crt)))
		{
			if (xHal_init_xmiter(crt)){
				uinfo("failed to initialize Tx (%d)\n", i);
				BUG();
				return -ENODEV;
			}
			else
			{
				uinfo("found Tx (%d)\n", i);
				break;
			}
		}
	}
	if (i == NUM_XMITER)
	{
		gpio_direction_input(GPIO_HOST_VIDEO_INPUT_SELECT);
		if (gpio_get_value(GPIO_HOST_VIDEO_INPUT_SELECT))
		{
			uinfo("choose analog loopback\n");
			crt->tx_select = XIDX_HOST_A;
			crt->tx_exist = 1;
			I2CInit(I2C_HOST_VIDEO_LOOPBACK_DDC, I2C_MONITOR_DDC_SPEED);
		} else {
			uinfo("no digital loopback\n");
			crt->tx_select = XIDX_HOST_D;
			crt->tx_exist = 0;
		}
	}

	//determine CRT
	return 0;
}
#elif (CONFIG_AST1500_SOC_VER == 2)
static unsigned int _init_all_xmiter_host(struct s_crt_drv *crt)
{
	unsigned int digital_exist, analog_exist;

	if (ast_scu.board_info.video_loopback_type == VT_DISABLE)
	{
		uinfo("loopback TX unavailable!!!\n");
		crt->tx_exist = 0;
		crt->dual_port_exist = 0;
		return 0;
	}
	else
		uinfo("loopback TX available\n");

	//determine video transmitter
	crt->tx_select = XIDX_HOST_D;
	if ((digital_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Digital Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Digital Tx\n");
		}
	}
	crt->tx_select = XIDX_HOST_A;
	if ((analog_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Analog Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Analog Tx\n");
		}
	}
	if (digital_exist && analog_exist)
		crt->dual_port_exist = 1;

	// Always use digital even VIDEO_TYPE_ANALOG trapping is set. So that CAT6613 can handle EDID read.
	if (digital_exist) {
		crt->tx_select = XIDX_HOST_D;
		crt->tx_exist = 1;
	} else {
		unsigned int type;
		type = ast_scu.board_info.video_type;
		if (type == VT_ANALOG)
		{
			uinfo("choose analog loopback\n");
			crt->tx_select = XIDX_HOST_A;
			crt->tx_exist = 1;
			I2CInit(I2C_HOST_VIDEO_LOOPBACK_DDC, I2C_MONITOR_DDC_SPEED);
		} else {
			uinfo("no digital loopback\n");
			crt->tx_select = XIDX_HOST_D;
			crt->tx_exist = 0;
		}
	}
	//determine CRT
	return 0;
}
#else /* For SoC V3 and latter */
static unsigned int _init_all_xmiter_host(struct s_crt_drv *crt)
{
	unsigned int digital_exist, analog_exist;
	unsigned int type;

	type = ast_scu.board_info.video_loopback_type;

	if (VT_DISABLE == type) {
		uinfo("loopback TX unavailable!!!\n");
		crt->tx_exist = 0;
		crt->dual_port_exist = 0;
		return 0;
	} else
		uinfo("loopback TX available\n");

	//determine video transmitter
	crt->tx_select = XIDX_HOST_D;
	if ((digital_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Digital Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Digital Tx\n");
		}
	}
	crt->tx_select = XIDX_HOST_A;
	if ((analog_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Analog Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Analog Tx\n");
		}
	}

	if (digital_exist && analog_exist)
		crt->dual_port_exist = 1;
	else if (!digital_exist && !analog_exist) {
		/*
		 * impossible, analog interface always exists on our xmit info
		 * add this check here in case we remove analog in future
		 */
		crt->tx_exist = 0;
		crt->dual_port_exist = 0;
		return 0;
	}

	/*
	** Cases:
	** 1. D only: crt->dual_port_exist && type != VT_ANALOG
	** 2. A only: !digital_exist && type != VT_DISABLE
	** 3. D+A using DVI-I (share DDC): crt->dual_port_exist && type == VT_ANALOG ==> default configure
	** 4. D+A separate DDC: not support
	*/
	if (crt->dual_port_exist) {
		if (type != VT_ANALOG) {
			/* Case 1 */
			uinfo("Choose digital only loopback\n");
			crt->tx_select = XIDX_HOST_D;
		} else {
			/* Case 3 */
			uinfo("DVI-I loopback\n");
			crt->tx_select = XIDX_HOST_D;
		}
	} else if (VT_ANALOG == type) {
		/* Case 2 */
		uinfo("Choose analog only loopback\n");
		crt->tx_select = XIDX_HOST_A;
		I2CInit(I2C_HOST_VIDEO_LOOPBACK_DDC, I2C_MONITOR_DDC_SPEED);
		if (digital_exist) {
			/*
			 * only digital exist but user select analog
			 * patch dual_port_exist to let crt configure digital port if dual_port is true
			 */
			crt->dual_port_exist = 1;
		}
	} else { /* digital interface */
		uinfo("Choose digital only loopback\n");
		crt->tx_select = XIDX_HOST_D;
		if (analog_exist) {
			/*
			 * only analog exist but user select digital
			 * patch dual_port_exist to let crt configure analog port if dual_port is true
			 */
			crt->dual_port_exist = 1;
		}
	}

	crt->tx_exist = 1;

	//determine CRT
	return 0;
}
#endif //#if (CONFIG_AST1500_SOC_VER == 1)

#endif //#ifdef CONFIG_ARCH_AST1500_HOST

#ifdef CONFIG_ARCH_AST1500_CLIENT
#if (CONFIG_AST1500_SOC_VER == 1)
static unsigned int _init_all_xmiter_client(struct s_crt_drv *crt)
{
	unsigned int i;

	//determine video transmitter
	for (i = 0; i < NUM_XMITER; i++)
	{
		crt->tx_select = i;
		if ((crt->tx_exist = xHal_xmiter_exist(crt)))
		{
			if (xHal_init_xmiter(crt)){
				uinfo("failed to initialize Tx (%d)\n", i);
				BUG();
				return -ENODEV;
			}
			else
			{
				uinfo("found Tx (%d)\n", i);
				break;
			}
		}
	}
	if (i == NUM_XMITER)
	{
		uinfo("choose analog display\n");
		crt->tx_select = XIDX_CLIENT_A;
		crt->tx_exist = 1;
		I2CInit(I2C_CLIENT_VIDEO_DDC_EEPROM, I2C_MONITOR_DDC_SPEED);
	}

	//determine CRT
	if (crt->tx_select == XIDX_CLIENT_A)
		crt->disp_select = CRT_1;
	else
		crt->disp_select = CRT_2;
	return 0;
}

#elif (CONFIG_AST1500_SOC_VER == 2)
static unsigned int _init_all_xmiter_client(struct s_crt_drv *crt)
{
	unsigned int digital_exist, analog_exist;
	unsigned int type;
	type = ast_scu.board_info.video_type;

	if(type == VT_DISABLE)
	{
		uinfo("TX unavailable!!!\n");
		crt->tx_exist = 0;
		return 0;
	}
	else
		uinfo("TX available\n");

	//determine video transmitter
	crt->tx_select = XIDX_CLIENT_D;
	if ((digital_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Digital Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Digital Tx\n");
		}
	}
	crt->tx_select = XIDX_CLIENT_A;
	if ((analog_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Analog Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Analog Tx\n");
		}
	}
	if (digital_exist && analog_exist)
		crt->dual_port_exist = 1;

	//Choose digital if exists.
	if (digital_exist) {
		crt->tx_select = XIDX_CLIENT_D;
		crt->tx_exist = 1;
	} else {
		uinfo("choose analog display\n");
		crt->tx_select = XIDX_CLIENT_A;
		crt->tx_exist = 1;
		I2CInit(I2C_CLIENT_VIDEO_DDC_EEPROM, I2C_MONITOR_DDC_SPEED);
	}


#ifdef FPGA_TEST_ONLY
	//Force digital interface.
	crt->dual_port_exist = 1;
	crt->tx_select = XIDX_CLIENT_D;
	crt->tx_exist = 1;
#endif


	//determine CRT
	crt->disp_select = CRT_2;
	return 0;
}

#else
static unsigned int _init_all_xmiter_client(struct s_crt_drv *crt)
{
	unsigned int digital_exist, analog_exist;
	unsigned int type;
	type = ast_scu.board_info.video_type;

	if(type == VT_DISABLE)
	{
		uinfo("TX unavailable!!!\n");
		crt->tx_exist = 0;
		return 0;
	}
	else
		uinfo("TX available\n");

	//determine video transmitter
	crt->tx_select = XIDX_CLIENT_D;
	if ((digital_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Digital Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Digital Tx\n");
		}
	}
	crt->tx_select = XIDX_CLIENT_A;
	if ((analog_exist = xHal_xmiter_exist(crt))) {
		crt->tx_exist = 1; //xHal_init_xmiter() need this.
		if (xHal_init_xmiter(crt)){
			uinfo("failed to initialize Analog Tx\n");
			BUG();
			return -ENODEV;
		}
		else
		{
			uinfo("found Analog Tx\n");
		}
	}
	if (digital_exist && analog_exist)
		crt->dual_port_exist = 1;

	if ((type == VT_ANALOG) && (analog_exist)) {
		uinfo("choose analog display\n");
		crt->tx_select = XIDX_CLIENT_A;
		crt->tx_exist = 1;
		I2CInit(I2C_CLIENT_VIDEO_DDC_EEPROM, I2C_MONITOR_DDC_SPEED);
	} else if (digital_exist) {
		crt->tx_select = XIDX_CLIENT_D;
		crt->tx_exist = 1;
	} else {
		/* CRT built-in analog DAC output, so this case must be digital. */
		uerr("Choose digital output but can't find digital output transmitter!!?\n");
		/* Force digital output */
		crt->tx_select = XIDX_CLIENT_D;
		crt->tx_exist = 1;
	}

#ifdef FPGA_TEST_ONLY
	//Force digital interface.
	crt->dual_port_exist = 1;
	crt->tx_select = XIDX_CLIENT_D;
	crt->tx_exist = 1;
#endif

	//determine CRT
	crt->disp_select = CRT_2;
	return 0;
}
#endif //#if (CONFIG_AST1500_SOC_VER == 1)
#endif //#ifdef CONFIG_ARCH_AST1500_CLIENT

static unsigned int _init_all_xmiter(struct s_crt_drv *crt)
{
#ifdef CONFIG_ARCH_AST1500_HOST
	return _init_all_xmiter_host(crt);
#else
	return _init_all_xmiter_client(crt);
#endif

}

unsigned int crt_host_init(struct s_crt_drv *crt)
{
	unsigned int ret = 0;

	/* let sanpshot region be cacheable to speed memory access */
	crt->vbuf_va_c = ioremap_cached(VBUF1_EN_SNAPSHOT_PHY_BASE, CRT_SNAPSHOT_SIZE);

	crt->xmiter_wq = create_singlethread_workqueue("XMITER_WQ");
	BUG_ON(crt->xmiter_wq == NULL);
	INIT_WORK(&crt->ReadDisplayEdidWork, (void (*)(void *))(crt_get_display_edid), crt);

	if (_init_all_xmiter(crt)) {
		ret = -ENODEV;
		goto out;
	}

	/* Calling crt_get_display_edid() to trigger "display selection" and "polling when no display"*/
	crt_get_display_edid(crt);

out:
	return ret;
}

u32 crt_client_init(struct s_crt_drv *crt)
{
	u32 ret = 0;
	struct s_crt_info i;

	crt->vbuf_va = ioremap_nocache(VBUF_DE_PHY_BASE, VBUF_STATION_SIZE - CRT_SNAPSHOT_SIZE);

	/* let sanpshot region be cacheable to speed memory access */
	crt->vbuf_va_c = ioremap_cached(CRT_SNAPSHOT_PHY_BASE, CRT_SNAPSHOT_SIZE);

	uinfo("VBUF nocache Virtual Addr [0x%x], size(0x%x)\n", (u32)crt->vbuf_va, VBUF_STATION_SIZE - CRT_SNAPSHOT_SIZE);
	uinfo("VBUF cache Virtual Addr [0x%x], size(0x%x)\n", (u32)crt->vbuf_va_c, CRT_SNAPSHOT_SIZE);

#if (CONFIG_AST1500_SOC_VER >= 3)
	/*
	** Bruce171108. Initialize as STRETCH_HINT_FIT_IN instead of AUTO.
	** We need this init because LM won't set vw_stretch_type under SOC_OP_MODE == 1.
	*/
	crt->vw_stretch_type = STRETCH_HINT_FIT_IN;

	init_waitqueue_head(&crt->osd_wait_queue);
#endif
	crt->wq = create_singlethread_workqueue("OSD_wq");
	BUG_ON(crt->wq == NULL);
	INIT_WORK(&crt->OsdVideoOnWork, (void (*)(void *))(OSDVideoOnHandler), crt);
#if (3 == CONFIG_AST1500_SOC_VER)
	INIT_WORK(&crt->LogoWork, (void (*)(void *))(LogoVideoOnHandler), crt);
#endif

	crt->xmiter_wq = create_singlethread_workqueue("XMITER_WQ");
	BUG_ON(crt->xmiter_wq == NULL);
	INIT_WORK(&crt->ReadDisplayEdidWork, (void (*)(void *))(crt_get_display_edid), crt);

	crt->OSD_VideoOn_Setting = (8<<OSD_SET_ALPHA_BIT);

	crt->dec_buf_base = VBUF1_DE_DEST0_PHY_BASE;
	crt->dec_hor_scale = CRT_SCALE_FACTOR_MAX;
	crt->dec_ver_scale = CRT_SCALE_FACTOR_MAX;

	/* We setup a default screen info for console screen. */
	/* This is necessary when frame buffer driver is not compiled. */
	memset(&i, 0, sizeof(struct s_crt_info));
#if (OSD_FROM_GUI == 3)
	i.crt_color = RGB565_FORMAT;
#else
	i.crt_color = XRGB8888_FORMAT;
#endif
	i.crt_output_format = XRGB8888_FORMAT;
	i.edid = crt->edid;
	i.width = ast_scu.astparam.console_default_x; // CRT_CONSOLE_DEFAULT_X;
	i.height = ast_scu.astparam.console_default_y; // CRT_CONSOLE_DEFAULT_Y;
	i.refresh_rate = ast_scu.astparam.console_default_rr; // CRT_CONSOLE_DEFAULT_RR;
	i.owner = OWNER_CON;
	i.clr = 1;
	i.line_offset = 0;
	i.AVI_Info.number = 0;
	i.HDMI_Info.number = 0;
	i.HDR_Info.number = 0;
	crt_setup_par(crt, &i);

	if (_init_all_xmiter(crt)) {
		ret = -ENODEV;
		goto out;
	}
	// We reset crt_engine once we know the "disp_select"
	reset_crt_engine(crt);

	if (!crt->tx_exist)
		goto out;

#if (3 <= CONFIG_AST1500_SOC_VER)
	/* set dual_port to let console display on both digial and analog right after system booting */
	crt->dual_port = 1;
#endif

	/* Calling crt_get_display_edid() to trigger "display selection" and "polling when no display"*/
	/* Calling crt_get_display_edid() by queuing to workqueue to avoid race condition
	** when booting up. The 1.5 seconds delay is necessary to wait for CAT6613 stable.
	*/
	queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, msecs_to_jiffies(1500));
	//crt_get_display_edid(crt);

out:
	return ret;

}

static int crt_mode_table_vesa_start(struct s_crt_drv *crt)
{
	int i, size;
	MODE_ITEM *mode;

	mode = crt->mode_table;

	size = crt->mode_table_size;

	for (i = 0; i < size; i++) {
		if (mode->sn >= VESA_TIMING_START)
			break;
		mode++;
	}

	return i;
}

static int crt_mode_table_user_start(struct s_crt_drv *crt)
{
	int i, size;
	MODE_ITEM *mode;

	mode = crt->mode_table;

	size = crt->mode_table_size;

	for (i = 0; i < size; i++) {
		if (mode->sn >= USER_TIMING_START)
			break;
		mode++;
	}

	return i;
}

u32 crt_init(struct s_crt_drv *crt)
{
	u32 ret = 0;

	//Decide host or client board by trapping reg.
	crt->is_client = ast_scu.board_info.is_client;
//	crt->is_client = ast_scu.scu_op(SCUOP_SCU_IS_CLIENT_GET, NULL);
//	crt->is_client = GetSCUReg(SCU_HW_TRAPPING_REG) & CLIENT_MODE_EN_MASK;

	crt->mode_table = AstModeTable;
	crt->mode_table_size = AstModeCount;
	crt->mode_table_vesa_start = crt_mode_table_vesa_start(crt);
	crt->mode_table_user_start = crt_mode_table_user_start(crt);
	crt->current_owner = OWNER_NONE;

	init_MUTEX(&crt->crt_lock);
	spin_lock_init(&crt->crt_reg_lock);

	if (crt->is_client) {
		/* client */
		uinfo("init crt driver for client board\n");
		ret = crt_client_init(crt);
	} else {
		/* host */
		uinfo("init crt driver for host board\n");
		ret = crt_host_init(crt);
	}

	return ret;
}

void crt_exit(struct s_crt_drv *crt)
{
	if (crt->is_client) {

	    if (crt->wq) {
	        cancel_delayed_work(&crt->OsdVideoOnWork);
	        flush_workqueue(crt->wq);
	        destroy_workqueue(crt->wq);
	        crt->wq = NULL;
	    }

	    if (crt->xmiter_wq) {
			cancel_delayed_work(&crt->ReadDisplayEdidWork);
	        flush_workqueue(crt->xmiter_wq);
	        destroy_workqueue(crt->xmiter_wq);
	        crt->xmiter_wq = NULL;
	    }

		iounmap (crt->vbuf_va);
	}
	else {
	    if (crt->xmiter_wq) {
			cancel_delayed_work(&crt->ReadDisplayEdidWork);
	        flush_workqueue(crt->xmiter_wq);
	        destroy_workqueue(crt->xmiter_wq);
	        crt->xmiter_wq = NULL;
	    }
	}

	iounmap (crt->vbuf_va_c);
}

extern struct s_crt_drv *crt;
struct s_crt_drv *crt_connect(unsigned int owner)
{
	//crt_connect means the "owner" is interests in the display event.
	crt->connected_screen |= owner;
	//force polling once
	/*
	 * for client, i2s_init will call crt_connect(OWNER_NONE)
	 * so just force polling when owner is not OWNER_NONE
	 */
	if (OWNER_NONE != owner)
		queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, 0);
	return crt;
}

struct s_crt_drv *crt_get(void)
{
	return crt;
}

void crt_disconnect(struct s_crt_drv *crt, unsigned int owner)
{
#if 0
	if (!crt->is_client)
		return;
#endif
	crt->connected_screen &= ~owner;
	crt->available_screen &= ~owner;

	if (owner == OWNER_VE1) {
		crt->dec_buf_base = VBUF1_DE_DEST0_PHY_BASE;
		crt->dec_hor_scale = CRT_SCALE_FACTOR_MAX;
		crt->dec_ver_scale = CRT_SCALE_FACTOR_MAX;
	}

	if (crt->current_owner == owner) {
		//Disable current xmiter
		_crt_disable_xmiter(crt);

		crt->current_owner = OWNER_NONE;
	}

}

void crt_hotplug_tx(int plug)
{
	static int pre_plug = 0; //0: unplug 1: plug

	if (crt)
	{
		/*
		** All the plug/unplug should be handled in ReadDisplayEdidWork.
		** Handling plug/unplug event here has potential race condition issues.
		** An double check for unplug event is also necessary for some ximters.
		** There is no redundant debug message and code actually.
		*/
		cancel_delayed_work(&crt->ReadDisplayEdidWork);
		/*
		** Bruce161221. NEVER block crt work_queue in callback.
		** Callback should be non-blocking call.
		** Scenario:
		** - crt call setup xmiter -> blocking. Wait for xmiter thread complete.
		** - xmiter thread detected hotplug, callback crt.
		** - if crt callback flush_workqueue() here -> blocking and dead lock.
		*/
		//flush_workqueue(crt->xmiter_wq);

		if (pre_plug == 0 && plug == 1) {
			crt->always_set_tx = 1;
		}
		pre_plug = plug;

		queue_delayed_work(crt->xmiter_wq,
		                  &crt->ReadDisplayEdidWork,
		                  msecs_to_jiffies(100));
	}
}

void crt_reg_tx_hotplug_callback(PFN_CRT_Tx_Hotplug_Callback callback)
{	printk("register Tx hotplug callback\n");
	tx_hotplug_callback = callback;

	//Trigger a hotplug when registered so that EDID can always be got by register.
	tx_hotplug_callback(crt->display_attached);
}

void crt_dereg_tx_hotplug_callback(void)
{	printk("deregister Tx hotplug callback\n");
	tx_hotplug_callback = NULL;
}

#if defined(CONFIG_ARCH_AST1500_CLIENT)
void crt_reg_tx_hotplug_cec_callback(PFN_CRT_Tx_Hotplug_Callback callback)
{
	tx_hotplug_cec_callback = callback;

	/* Trigger a hotplug when registered so that EDID can always be got by register. */
	tx_hotplug_cec_callback(crt->display_attached);
}
EXPORT_SYMBOL(crt_reg_tx_hotplug_cec_callback);

void crt_dereg_tx_hotplug_cec_callback(void)
{
	tx_hotplug_cec_callback = NULL;
}
EXPORT_SYMBOL(crt_dereg_tx_hotplug_cec_callback);
#endif

void _crt_disable_xmiter(struct s_crt_drv *crt)
{
	//crt->op_state |= COPS_PWR_SAVE;
	crt->op_state &= ~COPS_FORCING_TIMING;
	xHal_disable_xmiter(crt);
}

void _crt_cfg_dual_port(struct s_crt_drv *crt, unsigned int on)
{
	crt->dual_port = on;

	if (crt->dual_port_exist) {
		//Calling xHal_init_dual_port_xmiter() will disable dual port output.
		xHal_init_dual_port_xmiter(crt);
		if (on) {
			_crt_to_screen(crt, OWNER_CURRENT);
		}
	}
}


unsigned int crt_get_tx_cap(struct s_crt_drv *crt)
{
#if (3 <= CONFIG_AST1500_SOC_VER)
	/*
	 * i2s_client_init() need the return value of this function to determine whether HDMI exists
	 * For dual output case, it is necessary to pass both tx capibility (primary and secondary) to I2S
	 */
	return xHal_get_xmiter_cap(crt) | xHal_get_xmiter_cap_secondary(crt);
#else
	return xHal_get_xmiter_cap(crt);
#endif
}

void verify_timing_table(struct s_crt_drv *crt)
{
	MODE_ITEM *mt = crt->mode_table;
	unsigned int mt_size = crt->mode_table_size;
	unsigned int i, tmp;
	char *genby[] =
	{
		"GenByVESA",
		"GenByGTF",
		"GenByCVT",
		"GenByCVT_RB",
		"GenByHDTV"
	};


	for (i = 0; i < mt_size; i++) {
		/* Verify Horizontal */
		tmp = mt[i].HSyncTime + mt[i].HFPorch + mt[i].HActive + mt[i].HBPorch;
		if (tmp != mt[i].HTotal) {
			printk("Error! idx(%d) %dx%d@%dHz (%s)\n",
				i, mt[i].HActive, mt[i].VActive, mt[i].RefreshRate, genby[mt[i].GenClock]);
			printk("HTotal(%d) != HS+HFP+HA+HBP(%d)\n", mt[i].HTotal, tmp);
		}
		/* Verify Veritical */
		tmp = mt[i].VSyncTime + mt[i].VFPorch + mt[i].VActive + mt[i].VBPorch;
		if (tmp != mt[i].VTotal) {
			printk("Error! idx(%d) %dx%d@%dHz (%s)\n",
				i, mt[i].HActive, mt[i].VActive, mt[i].RefreshRate, genby[mt[i].GenClock]);
			printk("VTotal(%d) != VS+VFP+VA+VBP(%d)\n", mt[i].VTotal, tmp);
		}
	}
}

#if (CONFIG_AST1500_SOC_VER >= 2)
void _enable_vsync_int(struct s_crt_drv *crt, unsigned int nCrt)
{
#if 0
	//The interrupt setting applies to AST1510.
	//Interrupt at line0. And only applies to CRT2 address.
	// (line 0 means first line of active window.)
//	BUG_ON(nCrt == CRT_1);
#if 0//line0 allows VE the most time to decompress. However bugs in anti-flicter prevent its use.

	if (ast_scu.ability.v_vga_hotplug_det_test) { //enable int on vsync start (retrace start)
		unsigned int l = GetSCUReg(CRT1_VER_RETRACE_REG + CRT_2*0x60);
		l = ((l >> VER_RETRACE_START_BIT) & 0x1FFF) + 1;
		uinfo("Enable VSync Interrupt on Line %d\n", l);
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
				   l << LINE_TO_INT_BIT,
				   LINE_TO_INT_MASK);

		//I just put the testing code here for no reason
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
		           0xFF<<8,
		           0xFFF<<8);

		ModSCUReg(SCU_CONTROL_REG,
		           ADC_ENGINE_RESET_ON,
		           ADC_ENGINE_RESET_MASK);
		msleep(1); //I pick a value for no reason again.
		ModSCUReg(SCU_CONTROL_REG,
		           ADC_ENGINE_RESET_OFF,
		           ADC_ENGINE_RESET_MASK);

		//Set clock to 12MHz
		//Assume current H-PLL is 384MHz.
		l = GetSCUReg(SCU_CLOCK_SELECTION_REG);
		l = (l >> 23) & 0x7;
		l = (2*(l+1));
		l = ((384)/(12 * l * 2)) - 1; //See datasheet for this formula.
		// Should be 0
		SetSCUReg(ADC_CLK_CTRL, l);
		//Enable ADC engine ch0
		// Should be 0x1000F
		SetSCUReg(ADC_CTRL,
		           (ADC_ENGINE_ON << ADC_ENGINE_ON_BIT) |
		           (0x7 << ADC_OP_MODE_BIT) |
		           (0x1 << ADC_CH_EN_BIT));

	} else {
#if 1
		uinfo("Enable VSync Interrupt on Line0\n");
		//Set to line0
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
		           0 << LINE_TO_INT_BIT,
		           LINE_TO_INT_MASK);
#else
		unsigned int l = GetSCUReg(CRT1_VER_RETRACE_REG + CRT_2*0x60);
		l = ((l >> VER_RETRACE_START_BIT) & 0x1FFF) + 1;
		uinfo("Enable VSync Interrupt on Line %d\n", l);
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
				   l << LINE_TO_INT_BIT,
				   LINE_TO_INT_MASK);
#endif
	}

#else
#if 0
	//enable VSync INT at last active line so that we don't need anti-flicter in most cases.
	{
		unsigned int VActive = (GetSCUReg(CRT1_VER_TOTAL_END_REG  + CRT_2*0x60) >> VER_ENABLE_END_BIT)/* + 1*/;
		uinfo("Enable VSync Interrupt on Line%d\n", VActive);
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
				   VActive << LINE_TO_INT_BIT,
				   LINE_TO_INT_MASK);
	}
#else
	//enable CRT INT at last slice so that we don't need anti-flicker
	{
		unsigned int VActive = (GetSCUReg(CRT1_VER_TOTAL_END_REG  + CRT_2*0x60) >> VER_ENABLE_END_BIT) + 1;
		if (GetSCUReg(CRT1_CONTROL_REG  + CRT_2*0x60) & ENABLE_CRT_INTERLACE_TIMING_MASK)
			//interlace mode
			VActive <<= 1;
		VActive -= 8;//start of last slice
		uinfo("Enable VSync Interrupt on Line%d\n", VActive);
		ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60,
				   VActive << LINE_TO_INT_BIT,
				   LINE_TO_INT_MASK);
	}
#endif
#endif
#endif
	//Enable
	ModSCUReg(CRT1_CONTROL_REG + CRT_2*0x60,
	           (1<<INT_ON_BIT) | (1<<INT_STATUS_BIT),
	           INT_ON_MASK|INT_STATUS_MASK);
}

void _disable_vsync_int(struct s_crt_drv *crt, unsigned int nCrt)
{
	//Enable
	ModSCUReg(CRT1_CONTROL_REG + CRT_2*0x60,
	           (0<<INT_ON_BIT) | (1<<INT_STATUS_BIT),
	           INT_ON_MASK|INT_STATUS_MASK);

#if (CONFIG_AST1500_SOC_VER >= 3)
	/* In case of vsync int is somehow disabled when OSD thread is waiting. */
	crt->osd_wait_done = 1;
	wake_up_interruptible(&crt->osd_wait_queue);
#endif
}

void disable_vsync_int(unsigned int nCrt)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_disable_vsync_int, (void *)nCrt, 0);
}

unsigned int get_crt_vsync_int_stat_atomic(int nCrt)
{
	return (INT_STATUS_MASK & GetSCUReg(CRT1_CONTROL_REG + CRT_2*0x60));
}

void ack_crt_vsync_int_stat_atomic(int nCrt)
{
	ModSCUReg(CRT1_CONTROL_REG + CRT_2*0x60,
	           (1<<INT_STATUS_BIT),
	           INT_STATUS_MASK);

#if (CONFIG_AST1500_SOC_VER >= 3)
	crt->osd_wait_done = 1;
	wake_up_interruptible(&crt->osd_wait_queue);
#endif
}

#else //#if (CONFIG_AST1500_SOC_VER >= 2)
void _enable_vsync_int(struct s_crt_drv *crt, unsigned int nCrt)
{
	unsigned int VP, EG;

	VP = (GetSCUReg(CRT1_CONTROL_REG + nCrt*0x60) & VER_SYNC_SELECT_MASK) >> VER_SYNC_SELECT_BIT;
	if (VP == VER_NEGATIVE) {
		EG = AST1500_GPIO_INT_FALLING_EDGE;
	} else {
		EG = AST1500_GPIO_INT_RISING_EDGE;
	}

	if (nCrt == CRT_1) {
		gpio_cfg(AST1500_GPH5, EG);
	} else { /*CRT_2*/
		gpio_cfg(AST1500_GPD1, EG);
	}
}
#endif

void enable_vsync_int(unsigned int nCrt)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_enable_vsync_int, (void *)nCrt, 0);
}

#if (CONFIG_AST1500_SOC_VER >= 2)
unsigned int crt_is_in_odd_field_atomic(void)
{
	unsigned int isOdd;

	isOdd = GetSCUReg(CRT2_STATUS_REG) & CRT_ODD_FIELD;

	//Return non-zero for Odd. 0 for even.
	return isOdd;
}

void _crt_disable_flip(struct s_crt_drv *crt, void *dummy)
{
	ModSCUReg(CRT2_CONTROL_REG,
	           0 << ENABLE_VE_FLIP_BIT,
	           1 << ENABLE_VE_FLIP_BIT);

#if (CONFIG_AST1500_SOC_VER >= 3)
	if (crt->current_owner == OWNER_VE1) {
		/*
		 * adjust CRT display address only in decode screen
		 *
		 * If we stop link right after reconnect
		 * InitializeVideoEngineClient() may got executed after crt_to_console_screen()
		 * without check current_owner, CRT display address will be infected with _crt_set_vbuf()
		 */
		/* we use CRT2_DISPLAY_ADDRESS if 'VE Flip' is disabled */
		_crt_set_vbuf(crt, crt->dec_buf_base + crt->dec_buf_offset);
	}
#endif
}

void crt_disable_flip(void)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_disable_flip, NULL, 0);
}

void _crt_enable_flip(struct s_crt_drv *crt, void *dummy)
{
	ModSCUReg(CRT2_CONTROL_REG,
	           ENABLE_VE_FLIP,
	           1 << ENABLE_VE_FLIP_BIT);
}

void crt_enable_flip(void)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_enable_flip, NULL, 0);
}

static void fill_yuv_422_tile(u8 *tile, u32 y, u32 u, u32 v)
{
	u8 *addr = tile, line;

	line = 0;

	while (line < 8) {
		memset(addr, y, 16);
		addr += 16;
		memset(addr, u, 8);
		addr += 8;
		memset(addr, v, 8);
		addr += 8;
		line++;
	}
}

static void _disable_desktop_yuv(struct s_crt_drv *crt)
{
	struct s_crt_info *crt_info;
	u32 x, max, i;
	u8 *addr;

	crt_info = &crt->VE1_mode;

	i = lookupModeTable(crt, crt_info);

	x = (crt_info->crt_output_format == YUV420_FORMAT) ? (crt->mode_table[i].HActive << 1) : (crt->mode_table[i].HActive);

	max = (x + 0xF) >> 4; /* (x + 15) / 16 */

	addr = (u8 *) TO_VB1_DE_DEST0_BASE(crt->vbuf_va);
	for (i = 0; i < max; i++) {
		fill_yuv_422_tile(addr, 0x00, 0x80, 0x80);
		addr += 0x100;
	}

	addr = (u8 *) TO_VB1_DE_DEST1_BASE(crt->vbuf_va);
	for (i = 0; i < max; i++) {
		fill_yuv_422_tile(addr, 0x00, 0x80, 0x80);
		addr += 0x100;
	}

	_crt_display_offset_clear(crt);
}

static void _crt_disable_desktop(struct s_crt_drv *crt)
{
	/* Caller must hold &crt->crt_lock */

	if (ast_scu.board_info.soc_revision < 21)
		return;

	/* Don't disable again. */
	if (crt->op_state & COPS_DESKTOP_OFF)
		return;

	crt->op_state |= COPS_DESKTOP_OFF;
	uinfo("disable desktop\n");
	/* For now, only VE decode case has YUV output format. */
	if ((crt->current_owner == OWNER_VE1) && (crt->VE1_mode.crt_output_format != XRGB8888_FORMAT)) {
		if (!(crt->op_state & COPS_VE_YUV_DESKTOP_OFF)) {
			_disable_desktop_yuv(crt);
			ModSCUReg(CRT2_CONTROL_REG, 0, DISABLE_DESKTOP_MASK); /* do not disable dektop when YUV */
			crt->op_state |= COPS_VE_YUV_DESKTOP_OFF;
		}
	} else {
		ModSCUReg(CRT2_CONTROL_REG, DISABLE_DESKTOP_MASK, DISABLE_DESKTOP_MASK);
	}
}

static void _crt_disable_desktop_work(struct s_crt_drv *crt, void *data)
{
	//down(&crt->crt_lock);
	_crt_disable_desktop(crt);
	//up(&crt->crt_lock);
}

void crt_disable_desktop(struct s_crt_drv *crt)
{
	queue_crt_work_n_wait(crt, _crt_disable_desktop_work, NULL, 0);
}

static void _crt_enable_desktop(struct s_crt_drv *crt)
{
	/* Caller must hold &crt->crt_lock */

	if (ast_scu.board_info.soc_revision < 21)
		return;

	/* Don't enable again. */
	if (!(crt->op_state & COPS_DESKTOP_OFF))
		return;

	uinfo("enable desktop\n");

	if (crt->op_state & COPS_VE_YUV_DESKTOP_OFF) {
		_crt_display_offset_restore(crt);
		crt->op_state &= ~(COPS_VE_YUV_DESKTOP_OFF);
	}

	ModSCUReg(CRT2_CONTROL_REG, 0, DISABLE_DESKTOP_MASK);

	crt->op_state &= ~(COPS_DESKTOP_OFF);
}

static void _crt_enable_desktop_work(struct s_crt_drv *crt, void *data)
{
	//down(&crt->crt_lock);
	_crt_enable_desktop(crt);
	//up(&crt->crt_lock);
}

void crt_enable_desktop(struct s_crt_drv *crt)
{
	queue_crt_work_n_wait(crt, _crt_enable_desktop_work, NULL, 0);
}

void crt_enable_desktop_atomic(struct s_crt_drv *crt)
{
	queue_crt_work(crt, _crt_enable_desktop_work, NULL, 0);
}
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */

/*
 * crt_set_csc_format - set CRT color space convert
 *
 * @en_bt709: 0: bt.601, not 0: bt.709
 * @full_range: be valid only when bt.601
 */
void _crt_set_csc_format(unsigned int en_bt709, u32 full_range)
{
#if (CONFIG_AST1500_SOC_VER >= 3)
	/*
	 * CRT supports 3 types of CSC:
	 *
	 * bt601 limited => full rgb
	 * bt601 full    => full rgb  (no range convert)
	 * bt709 full    => full rgb  (no range convert)
	 *
	 */
	ModSCUReg(CRT2_VIDEO_PLL_REG,
	          en_bt709 ? CSC_EN_BT709 : 0,
	          CSC_EN_BT709);

	if (!en_bt709)
		ModSCUReg(CRT2_CONTROL_REG,
			full_range ? ENABLE_FULL_RANGE_YUV : 0,
			ENABLE_FULL_RANGE_YUV);
#endif
}

#if (CONFIG_AST1500_SOC_VER >= 3)
void _crt_set_csc_format_work(struct s_crt_drv *crt, u32 csc_format)
{
	_crt_set_csc_format(csc_format & CSC_EN_BT709, csc_format & ENABLE_FULL_RANGE_YUV);
}
#endif

void crt_set_csc_format(unsigned int en_bt709, u32 full_range)
{
#if (CONFIG_AST1500_SOC_VER >= 3)
	u32 csc_format;

	/* For easily pass configure, I use bitmap. */
	csc_format = en_bt709 ? CSC_EN_BT709 : 0;
	csc_format |= full_range ? ENABLE_FULL_RANGE_YUV : 0;

	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_set_csc_format_work, (void *)csc_format, 0);
#endif
}

#if (CONFIG_AST1500_SOC_VER >= 3)
u32 crt_in_yuv422(void)
{
	/*
	** FIXME: How come following formula can verify CRT is running YUV422?
	*/
	return (GetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT) & TERMINAL_COUNT_MASK)
			== (GetSCUReg(CRT2_DISPLAY_OFFSET) & TERMINAL_COUNT_MASK);
}

void crt_sw_flip_atomic(u32 cfg, u32 yuv422)
{
	u32 val;

	/*
	 * cfg[5] is color format selection[0]
	 * cfg[4] is color format selection[1]
	 */
	val = (((cfg >> 5) & 0x1) | (((cfg >> 4) & 0x1) << 1)) << FORMAT_SELECT_BIT;
	ModSCUReg(CRT2_CONTROL_REG, val, FORMAT_SELECT_MASK);

	if (yuv422) {
		val = GetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT);
		crt->VE1_mode.crt_color = YUV420_FORMAT;
	} else {
		val = GetSCUReg(CRT2_VEFLIP_TERMINAL_COUNT) << 16;
		crt->VE1_mode.crt_color = YUV444_FORMAT;
	}

	__mod_reg84(crt, val, TERMINAL_COUNT_MASK);
}

void crt_display_address_cfg_atomic(u32 address)
{
	SetSCUReg(CRT2_DISPLAY_ADDRESS, address);
}
#elif (CONFIG_AST1500_SOC_VER >= 2)
u32 crt_terminal_cnt(void)
{
	return (GetSCUReg(CRT2_DISPLAY_OFFSET) >> 16) & 0xFFF;
}
#endif

static int match_TV_timing_table(PVIDEO_MODE_INFO pRefInfo)
{
	unsigned int i;
	int TimingMatchIdx = -1;
	MODE_ITEM *mt = crt->mode_table;
	u32 mt_size = TV_TIMING_TABLE_SIZE(crt);

	for (i = 0; i < mt_size; i++) {
		if (abs(pRefInfo->HActive - mt[i].HActive) > 2)
			continue;
		if (abs(pRefInfo->VActive - mt[i].VActive) > 2)
			continue;
		if (pRefInfo->ScanMode != mt[i].ScanMode)
			continue;
		if (pRefInfo->RefreshRate != mt[i].RefreshRate)
			continue;
		/* Check porality */
		if ((pRefInfo->HPolarity != mt[i].HorPolarity) ||
		   (pRefInfo->VPolarity != mt[i].VerPolarity))
		{
			continue;
		}

		//The VT from our mode detection result is not always accurate.
		// So, we treat mismatched VT as TV timing.
		TimingMatchIdx = i;

		/* Check Vertical total */
		if (pRefInfo->VTotal != mt[i].VTotal)
			continue;

#if 0 /* Bruce160720. s_HDMI_4x3 & s_HDMI_16x9 has no meaning to video timing. */
		/* Extra check for HDMI signal */
		switch (pRefInfo->SignalType)
		{
		case s_RGB:
			// This is the case for analog source.
		case s_DVI:
			// This is the case for digital source without AVInfoFrame.
			// Matched!!
			break;
		case s_HDMI_4x3:
			if (mt[i].GenClock != GenByHDTV)
				continue;
			break;
		case s_HDMI_16x9:
			if (mt[i].GenClock != GenByHDTV_16x9)
				continue;
			break;
		default:
			BUG();
		}
#endif
		//Matched!!
		return i;
	}
	//Can't match. Use the one with matched timing, but wrong aspect
	if (TimingMatchIdx != -1)
		uerr("Can't match TV table's aspect or Vertical Total\n");
	return TimingMatchIdx;
}


int crt_match_timing_table_analog(PVIDEO_MODE_INFO pRefInfo, int *preset_match)
{
#define MAX_MI 30
#if 0
#define MAX_VT_TOLERANCE 128
#define MIN_VT_TOLERANCE 2
#else
#define MAX_VT_TOLERANCE 3
#define MIN_VT_TOLERANCE 0
#endif
	struct {
		unsigned int idx;
		unsigned int diff;
	} matched_idx[MAX_MI];

	unsigned int mi = 0;
	unsigned int i, j;
	unsigned int RefreshRate, VT, VT_diff;
	eVScanMode ScanMode;
	eVPolarity HPor, VPor;
	unsigned int VT_tolerance = MIN_VT_TOLERANCE;
	MODE_ITEM *mt = crt->mode_table;
	u32 mt_size = crt->mode_table_size;

	uinfo("Try to match this analog timing:\n");
	uinfo("VT[%d]Rate[%d],%s,%s,%s, VSyncWidth:%d\n",
	                    pRefInfo->VTotal,
	                    pRefInfo->RefreshRate,
	                    (pRefInfo->HPolarity == PosPolarity)?("HPos"):("HNeg"),
	                    (pRefInfo->VPolarity == PosPolarity)?("VPos"):("VNeg"),
	                    (pRefInfo->ScanMode == Prog)?("Prog"):
	                    ((pRefInfo->ScanMode == Interl)?("Interlace"):("Unknown")),
#if (CONFIG_AST1500_SOC_VER >= 2)
	                    (pRefInfo->VSyncWidth)
#else
				-1
#endif
	                    );

	*preset_match = 0;
	//ToDo. Shell we initial the matched_idx[].

	RefreshRate = pRefInfo->RefreshRate;
	ScanMode = pRefInfo->ScanMode;
	VT = pRefInfo->VTotal;
	HPor = pRefInfo->HPolarity;
	VPor = pRefInfo->VPolarity;

	/*
	 * YPbPr from analog interface (IT9883C)
	 * For IT9883c, vsync delay and hsync delay is different (vsync's > hsync's)
	 * and cause hsync and vsync is not aligned in non-interlaced mode.
	 * The result of VE mode detection scan mode is always interlaced for this abnormal case.
	 *
	 * we ignore scan mode check for YPbPr when ScanMode is set to 'Unknown'.
	 */

	for (VT_tolerance = MIN_VT_TOLERANCE; VT_tolerance < MAX_VT_TOLERANCE; VT_tolerance++) {
		mi = 0;

		for (i = 0; i < mt_size; i++) {
			if ((ScanMode != Unknown) && (ScanMode != mt[i].ScanMode))
				continue;

#if 0
			if (VT_tolerance == MIN_VT_TOLERANCE)
			{ //First try. Try to match the same refresh rate.
				if (RefreshRate != mt[i].RefreshRate)
					continue;
			} else {
				//Otherwise, accept refresh rate nearby.
				unsigned int RR_diff;
				RR_diff = abs(RefreshRate - mt[i].RefreshRate);
				if (RR_diff > 5) {
					continue;
				}
			}
#else
			{
				unsigned int RR_diff;
				RR_diff = abs(RefreshRate - mt[i].RefreshRate);
				if (RR_diff > 3)
				{
					continue;
				}
			}

#if 0//In analog mode, only compare polarities when there is at least 1 matched timing.
			if ((HPor != mt[i].HorPolarity) ||
				(VPor != mt[i].VerPolarity))
			{
				continue;
			}
#endif
#endif

#if 0 //Don't block non-supported Pixel clock
			if ((DCLK9883_LIMIT * 10000) < mt[i].DCLK10000) {
				continue;
			}
#endif

			VT_diff = abs(VT - mt[i].VTotal);
			if (VT_diff <= VT_tolerance) {
				matched_idx[mi].idx = i;
				matched_idx[mi].diff = VT_diff;
				mi++;
				if (mi == MAX_MI)
					break;
			}
		}

		if (!mi) {
//			uerr("Can't find any good matched analog timing!!\n");
			continue;
			//return DEFAULT_TIMING_IDX; //Use default
		} else {

#if (CONFIG_AST1500_SOC_VER >= 2)
			if (mi > 1) {
				for (i = 0; i < mi; i++) {
					matched_idx[i].diff = abs(pRefInfo->VSyncWidth - mt[matched_idx[i].idx].VSyncTime);
				}
				//Sorting matched_idx[] by diff
				for (i = 0; i < mi; i++) {
					for (j = (i + 1); j < mi; j++) {
						if (matched_idx[i].diff > matched_idx[j].diff) {
							unsigned int t_idx, t_diff;
							//swap(i, j);
							t_idx = matched_idx[i].idx;
							t_diff = matched_idx[i].diff;
							matched_idx[i].idx = matched_idx[j].idx;
							matched_idx[i].diff = matched_idx[j].diff;
							matched_idx[j].idx = t_idx;
							matched_idx[j].diff = t_diff;
						}
					}
				}
			}
#endif
			uinfo("%d modes found with VT_tolerance %d\n", mi, VT_tolerance);
			for (i = 0; i < mi; i++) {
				uinfo("idx:%d, sn:%d, %dx%d@%dHz VT:%d(%d) %s %s, VSyncWidth:%d\n",
				     matched_idx[i].idx,
				     crt_timing_index_to_sn(matched_idx[i].idx),
				     mt[matched_idx[i].idx].HActive,
				     mt[matched_idx[i].idx].VActive,
				     mt[matched_idx[i].idx].RefreshRate,
				     mt[matched_idx[i].idx].VTotal,
				     matched_idx[i].diff,
				     (mt[matched_idx[i].idx].HorPolarity == PosPolarity)?("HPos"):("HNeg"),
				     (mt[matched_idx[i].idx].VerPolarity == PosPolarity)?("VPos"):("VNeg"),
				     mt[matched_idx[i].idx].VSyncTime
				     );
			}

			for (i = 0; i < mi; i++) {
				if ((mt[matched_idx[i].idx].RefreshRate == RefreshRate)
				     &&
				     (mt[matched_idx[i].idx].HorPolarity == HPor)
				     &&
				     (mt[matched_idx[i].idx].VerPolarity == VPor)
#if (CONFIG_AST1500_SOC_VER >= 2)
				     &&
				     (matched_idx[i].diff == 0)
#endif
				)
				{
					break;
				}
			}

			if (i == mi) {
#if (CONFIG_AST1500_SOC_VER >= 2)
				/* can't find perfect match. Ignore VSyncWidth. */
				j = 0;
				for (i = 0; i < mi; i++) {
					if ((mt[matched_idx[i].idx].RefreshRate == RefreshRate)
					     &&
					     (mt[matched_idx[i].idx].HorPolarity == HPor)
					     &&
					     (mt[matched_idx[i].idx].VerPolarity == VPor)
					)
					{
						j = i;
						break;
					}
				}
#else
				j = 0;
#endif
			} else {
				j= i;
			}

			if ((DCLK9883_LIMIT * 10000) >= mt[matched_idx[j].idx].DCLK10000)
				*preset_match = 1;
			else
				printk("Pixel clock might be unsupported by ADC9883.\n");
			return matched_idx[j].idx;
		}
#if 0
		//Sorting matched_idx[] by VT_diff
		for (i = 0; i < mi; i++) {
			for (j = (i + 1); j < mi; j++) {
				if (matched_idx[i].diff > matched_idx[j].diff) {
					unsigned int t_idx, t_diff;
					//swap(i, j);
					t_idx = matched_idx[i].idx;
					t_diff = matched_idx[i].diff;
					matched_idx[i].idx = matched_idx[j].idx;
					matched_idx[i].diff = matched_idx[j].diff;
					matched_idx[j].idx = t_idx;
					matched_idx[j].diff = t_diff;
				}
			}
		}
		/* If we can find the perfect match, use the most nearest VT. */
		if (VT_tolerance != MIN_VT_TOLERANCE) {
			uerr("Can't find a perfect match!Use %d instead(%d)\n", matched_idx[0].idx, matched_idx[0].diff);
			return matched_idx[0].idx;
		}

		for (i = 0; i < mi; i++) {
			if ((HPor == mt[matched_idx[i].idx].HorPolarity) &&
			   (VPor == mt[matched_idx[i].idx].VerPolarity)) {
				//We found a perfect match!!
				if ((DCLK9883_LIMIT * 10000) < mt[matched_idx[i].idx].DCLK10000) {
					uerr("Warning!! Out of 9883's support range!\n");
				}
			   return matched_idx[i].idx;
			}
		}
#endif
	}

#if 1
#undef MAX_VT_TOLERANCE
#undef MIN_VT_TOLERANCE
#define MAX_VT_TOLERANCE 128
#define MIN_VT_TOLERANCE 2
	uerr("Can't find good match! Try to find the closest one\n");
		for (VT_tolerance = MIN_VT_TOLERANCE;
			VT_tolerance < MAX_VT_TOLERANCE;
			VT_tolerance += (MAX_VT_TOLERANCE >> 2))
		{
			mi= 0;

			for (i = 0; i < mt_size; i++) {
				if ((ScanMode != Unknown) && (ScanMode != mt[i].ScanMode))
					continue;

				if (VT_tolerance == MIN_VT_TOLERANCE)
				{ //First try. Try to match the same refresh rate.
					if (RefreshRate != mt[i].RefreshRate)
						continue;
				} else {
					//Otherwise, accept refresh rate nearby.
					unsigned int RR_diff;
					RR_diff = abs(RefreshRate - mt[i].RefreshRate);
					if (RR_diff > 5) {
						continue;
					}
				}

#if 0 //Don't block non-supported Pixel clock
				if ((DCLK9883_LIMIT * 10000) < mt[i].DCLK10000) {
					continue;
				}
#endif

				VT_diff = abs(VT - mt[i].VTotal);
				if (VT_diff <= VT_tolerance) {
					matched_idx[mi].idx = i;
					matched_idx[mi].diff = VT_diff;
					mi++;
					if (mi == MAX_MI)
						break;
				}
			}

			if (!mi) {
//				uerr("Can't find any good matched analog timing!!\n");
				continue;
				//return DEFAULT_TIMING_IDX; //Use default
			} else {
#if 0
				uinfo("%d modes found on first match\n", mi);
#else
				uinfo("%d modes found with VT_tolerance %d\n", mi, VT_tolerance);
#endif
				for (i = 0; i < mi; i++) {
					uinfo("idx:%d, sn:%d, %dx%d@%dHz VT:%d(%d) %s %s\n",
						 matched_idx[i].idx,
						 crt_timing_index_to_sn(matched_idx[i].idx),
						 mt[matched_idx[i].idx].HActive,
						 mt[matched_idx[i].idx].VActive,
						 mt[matched_idx[i].idx].RefreshRate,
						 mt[matched_idx[i].idx].VTotal,
						 matched_idx[i].diff,
						 (mt[matched_idx[i].idx].HorPolarity == PosPolarity)?("HPos"):("HNeg"),
						 (mt[matched_idx[i].idx].VerPolarity == PosPolarity)?("VPos"):("VNeg")
						 );
				}
			}
			//Sorting matched_idx[] by VT_diff
			for (i = 0; i < mi; i++) {
				for (j = (i + 1); j < mi; j++) {
					if (matched_idx[i].diff > matched_idx[j].diff) {
						unsigned int t_idx, t_diff;
						//swap(i, j);
						t_idx = matched_idx[i].idx;
						t_diff = matched_idx[i].diff;
						matched_idx[i].idx = matched_idx[j].idx;
						matched_idx[i].diff = matched_idx[j].diff;
						matched_idx[j].idx = t_idx;
						matched_idx[j].diff = t_diff;
					}
				}
			}
			/* If we can find the perfect match, use the most nearest VT. */
			if (VT_tolerance != MIN_VT_TOLERANCE) {
				uinfo("Can't find a perfect match!Use %d(sn:%d) instead(%d)\n", matched_idx[0].idx, crt_timing_index_to_sn(matched_idx[0].idx), matched_idx[0].diff);
				return matched_idx[0].idx;
			}

			for (i = 0; i < mi; i++) {
				if ((HPor == mt[matched_idx[i].idx].HorPolarity) &&
				   (VPor == mt[matched_idx[i].idx].VerPolarity)) {
					//We found a perfect match!!
					if ((DCLK9883_LIMIT * 10000) < mt[matched_idx[i].idx].DCLK10000) {
						uinfo("Warning!! Out of 9883's support range!\n");
					}
				   return matched_idx[i].idx;
				}
			}
		}
#endif
	uerr("Can't find any match!Use default(%d) instead\n", DEFAULT_TIMING_IDX);
	return DEFAULT_TIMING_IDX;
#undef MAX_MI
#undef MAX_VT_TOLERANCE
#undef MIN_VT_TOLERANCE

}

/*
** To avoid strange mode capture error, we lookup the table and
** Use the resolution from mode table.
** pSourceModeInfo->HActive
** pSourceModeInfo->VActive
** pSourceModeInfo->ScanMode
** pSourceModeInfo->RefreshRate
** pSourceModeInfo->VTotal
** pSourceModeInfo->VPolarity
** pSourceModeInfo->HPolarity
** Above parameters MUST be valid for crt_match_timing_table();
*/

int crt_match_timing_table(PVIDEO_MODE_INFO pRefInfo, int *preset_match)
{
#define MAX_MI 30
	int i, j, X;
	struct {
		unsigned int idx;
		unsigned int diff;
	} matched_idx[MAX_MI];
	unsigned int mi = 0;
	unsigned int RefreshRate, HA, VA;
	eVScanMode ScanMode;
	unsigned int idx, prefer_idx;
	MODE_ITEM *mt = crt->mode_table;
	u32 mt_size = crt->mode_table_size;

	*preset_match = 0;

	if (pRefInfo->SignalType == s_RGB)
		return crt_match_timing_table_analog(pRefInfo, preset_match);

	uinfo("Try to match this digital timing:\n");
	uinfo("HA[%d]VA[%d]Rate[%d]VT[%d],%s,%s,%s,%s\n",
	                       pRefInfo->HActive,
	                       pRefInfo->VActive,
	                       pRefInfo->RefreshRate,
	                       pRefInfo->VTotal,
	                       (pRefInfo->HPolarity == PosPolarity)?("HPos"):("HNeg"),
	                       (pRefInfo->VPolarity == PosPolarity)?("VPos"):("VNeg"),
	                       (pRefInfo->ScanMode == Prog)?("Prog"):("Interlace"),
	                       SIGNAL_TYPE_STR(pRefInfo->SignalType)
	                       );


	i = match_TV_timing_table(pRefInfo);
	if (i != -1) {
		uinfo("TV timing idx:%d, sn:%d\n", i, crt_timing_index_to_sn(i));
		*preset_match = 1;
		return i;
	}

	uinfo("enter 1st pass\n");
	RefreshRate = pRefInfo->RefreshRate;
	ScanMode = pRefInfo->ScanMode;
	HA = pRefInfo->HActive;
	VA = pRefInfo->VActive;

	/* Find the timing matching HA/VA/ScanMode */
	/* If found, match the RR and return no-matter RR is matched or not. */
	if ((pRefInfo->SignalType == s_HDMI_4x3) || (pRefInfo->SignalType == s_HDMI_16x9)) {
		i = 0;
	} else {
		i = TV_TIMING_TABLE_SIZE(crt);
	}
	for (mi = 0; i < mt_size; i++) {
		if (mt[i].ScanMode != ScanMode)
			continue;
		if (abs(mt[i].HActive - HA) > 2)
			continue;
		if (abs(mt[i].VActive - VA) > 2)
			continue;
		//Found!
		matched_idx[mi].idx = i;
		matched_idx[mi].diff = abs(mt[i].RefreshRate - RefreshRate);
		mi++;
	}

	if (!mi) {
		uerr("Can't find any good matched VESA digital timing!!\n");
		goto second_match;
	}
	*preset_match = 1;
	//Sorting matched_idx[] by RR_diff
	for (i = 0; i < mi; i++) {
		for (j = (i + 1); j < mi; j++) {
			if (matched_idx[i].diff > matched_idx[j].diff) {
				unsigned int t_idx, t_diff;
				//swap(i, j);
				t_idx = matched_idx[i].idx;
				t_diff = matched_idx[i].diff;
				matched_idx[i].idx = matched_idx[j].idx;
				matched_idx[i].diff = matched_idx[j].diff;
				matched_idx[j].idx = t_idx;
				matched_idx[j].diff = t_diff;
			}
		}
	}
	uinfo("%d modes found on first match\n", mi);
	for (i = 0; i < mi; i++) {
		idx = matched_idx[i].idx;
		uinfo("idx:%d, sn:0x%04X, %dx%d@%dHz(%d) VT:%d %s %s\n",
		     idx,
		     crt_timing_index_to_sn(idx),
		     mt[idx].HActive,
		     mt[idx].VActive,
		     mt[idx].RefreshRate,
		     matched_idx[i].diff,
		     mt[idx].VTotal,
		     (mt[idx].HorPolarity == PosPolarity)?("HPos"):("HNeg"),
		     (mt[idx].VerPolarity == PosPolarity)?("VPos"):("VNeg")
		     );
	}

	/* Match polarity and VT if RefreshRate is the same. */
	for (prefer_idx = -1, i = 0; i < mi; i++) {
		idx = matched_idx[i].idx;
		if (mt[idx].RefreshRate != RefreshRate)
			continue;
		if (abs(mt[idx].VTotal - pRefInfo->VTotal) > 2)
			continue;
		prefer_idx = idx;
		if ((mt[idx].HorPolarity != pRefInfo->HPolarity) ||
		   (mt[idx].VerPolarity != pRefInfo->VPolarity))
		   continue;
		prefer_idx = idx;
		//This is the best match!
		break;
	}

	if (prefer_idx != -1) {
		uinfo("perfect timing idx:%d sn:%d\n", prefer_idx, crt_timing_index_to_sn(prefer_idx));
		return prefer_idx;
	}

	uinfo("imperfect timing! Use idx:%d sn:%d\n", matched_idx[0].idx, crt_timing_index_to_sn(matched_idx[0].idx));
	return matched_idx[0].idx;

#define X_STEP 50
#define X_MAX 2000
second_match:
	uinfo("enter 2nd pass\n");
	/* If not found, find the timing matching HA+-(X*2)/VA+-X/ScanMode. */
	/* If found, use the smallest (HA_Diff)+(VA_Diff*2). */
	/* If not found, increase X till found. */
	for (X = X_STEP; X < X_MAX; X+= X_STEP)
	{
		unsigned int HA_Diff, VA_Diff;

		if ((pRefInfo->SignalType == s_HDMI_4x3) || (pRefInfo->SignalType == s_HDMI_16x9)) {
			i = 0;
		} else {
			i = TV_TIMING_TABLE_SIZE(crt);
		}
		for (mi = 0; i < mt_size; i++) {
			if (mt[i].ScanMode != ScanMode)
				continue;

#if 0 //It is not make sense to always matching 60Hz when we can match RR and polarity later
			if (mt[i].RefreshRate != 60)
				continue;
#endif
#if 0
			/*
			** 091001.Current design assumes Capture HA will always be display HA(?).
			** To avoid potential bug, don't use timing table item larger then detected HA/VA.
			*/
			if (mt[i].HActive > HA || mt[i].VActive > VA)
				continue;
#endif
			HA_Diff = abs(HA - mt[i].HActive);
			if (HA_Diff > (X<<1))
				continue;
			VA_Diff = abs(VA - mt[i].VActive);
			if (VA_Diff > X)
				continue;
			//Found!
			matched_idx[mi].idx = i;
			matched_idx[mi].diff = HA_Diff + (VA_Diff << 1);
			mi++;
			if (mi == MAX_MI)
				break;
		}
		if (!mi)
			continue;

		//Sorting matched_idx[] by diff
		for (i = 0; i < mi; i++) {
			for (j = (i + 1); j < mi; j++) {
				if (matched_idx[i].diff > matched_idx[j].diff) {
					unsigned int t_idx, t_diff;
					//swap(i, j);
					t_idx = matched_idx[i].idx;
					t_diff = matched_idx[i].diff;
					matched_idx[i].idx = matched_idx[j].idx;
					matched_idx[i].diff = matched_idx[j].diff;
					matched_idx[j].idx = t_idx;
					matched_idx[j].diff = t_diff;
				}
			}
		}
		uinfo("%d modes found on 2nd match with Torance=%d\n", mi, X);
		for (i = 0; i < mi; i++) {
			idx = matched_idx[i].idx;
			uinfo("idx:%d, sn:%d, %dx%d@%dHz diff:%d VT:%d %s %s\n",
			     idx,
			     crt_timing_index_to_sn(idx),
			     mt[idx].HActive,
			     mt[idx].VActive,
			     mt[idx].RefreshRate,
			     matched_idx[i].diff,
			     mt[idx].VTotal,
			     (mt[idx].HorPolarity == PosPolarity)?("HPos"):("HNeg"),
			     (mt[idx].VerPolarity == PosPolarity)?("VPos"):("VNeg")
			     );
		}

		/* Try to match Refresh Rate and Polarity */
		for (i = 1; i < mi; i++) {
			if (matched_idx[0].diff != matched_idx[i].diff) {
				mi = i;
				break;
			}
		}
		if (mi == 1) goto second_match_done;
		// Sort according to RR
		for (i = 0; i < mi; i++) {
			idx = matched_idx[i].idx;
			matched_idx[i].diff += abs(mt[idx].RefreshRate - RefreshRate);
		}
		for (i = 0; i < mi; i++) {
			for (j = (i + 1); j < mi; j++) {
				if (matched_idx[i].diff > matched_idx[j].diff) {
					unsigned int t_idx, t_diff;
					//swap(i, j);
					t_idx = matched_idx[i].idx;
					t_diff = matched_idx[i].diff;
					matched_idx[i].idx = matched_idx[j].idx;
					matched_idx[i].diff = matched_idx[j].diff;
					matched_idx[j].idx = t_idx;
					matched_idx[j].diff = t_diff;
				}
			}
		}
		// Start matching Polarity
		for (i = 1; i < mi; i++) {
			if (matched_idx[0].diff != matched_idx[i].diff) {
				mi = i;
				break;
			}
		}
		if (mi == 1) goto second_match_done;
		// Sort according to polarity
		for (i = 0; i < mi; i++) {
			idx = matched_idx[i].idx;
			matched_idx[i].diff += (mt[idx].HorPolarity != pRefInfo->HPolarity)?(1):(0);
			matched_idx[i].diff += (mt[idx].VerPolarity != pRefInfo->VPolarity)?(1):(0);
		}
		for (i = 0; i < mi; i++) {
			for (j = (i + 1); j < mi; j++) {
				if (matched_idx[i].diff > matched_idx[j].diff) {
					unsigned int t_idx, t_diff;
					//swap(i, j);
					t_idx = matched_idx[i].idx;
					t_diff = matched_idx[i].diff;
					matched_idx[i].idx = matched_idx[j].idx;
					matched_idx[i].diff = matched_idx[j].diff;
					matched_idx[j].idx = t_idx;
					matched_idx[j].diff = t_diff;
				}
			}
		}
second_match_done:
		return matched_idx[0].idx;
	}

	uerr("Failed to find a good video timing, use default.\n");
	return DEFAULT_TIMING_IDX;

#undef MAX_MI
#undef X_STEP
#undef X_MAX
}

MODE_ITEM *crt_timing_by_index(u32 index)
{
	MODE_ITEM *mt = crt->mode_table;
	u32 mt_size = crt->mode_table_size;

	if (index < mt_size)
		return mt + index;
	else {
		/* return default timing */
		return mt + DEFAULT_TIMING_IDX;
	}
}
EXPORT_SYMBOL(crt_timing_by_index);

MODE_ITEM *crt_timing_by_sn(u32 sn)
{
	u32 index = crt_timing_sn_to_index(sn);

	return crt_timing_by_index(index);
}
EXPORT_SYMBOL(crt_timing_by_sn);

u32 crt_timing_index_to_sn(u32 index)
{
	MODE_ITEM *mode;

	mode = crt_timing_by_index(index);

	return mode->sn;
}
EXPORT_SYMBOL(crt_timing_index_to_sn);

static void _notify_user_wrong_version(void)
{
	static unsigned long notify_timeout = 0;

	if ((notify_timeout == 0) || time_after(jiffies, notify_timeout)) {
		ast_notify_user("e_wrong_fw");
		notify_timeout = jiffies + msecs_to_jiffies(5000);
	}
}

u32 crt_timing_sn_to_index(u32 sn)
{
	u32 index;

	/*
	 * timing table includes,
	 *	1. HDTV timing, SNs start from 0
	 *	2. VESA timing, SNs start from VESA_TIMING_START
	 *	3. User-defined timing, SNs start from USER_TIMING_START
	 */

	if (sn >= USER_TIMING_START) {
		index = sn - USER_TIMING_START + crt->mode_table_user_start;
		/* Validate the index value. */
		if (index >= crt->mode_table_size) {
			index = DEFAULT_TIMING_IDX;
			_notify_user_wrong_version();
		}
	} else if (sn >= VESA_TIMING_START) {
		index = sn - VESA_TIMING_START + crt->mode_table_vesa_start;
		/* Validate the index value. */
		if (index >= crt->mode_table_user_start) {
			index = DEFAULT_TIMING_IDX;
			_notify_user_wrong_version();
		}
	} else {
		index = sn;
		/* Validate the index value. */
		if (index >= crt->mode_table_vesa_start) {
			index = DEFAULT_TIMING_IDX;
			_notify_user_wrong_version();
		}
	}

	return index;
}
EXPORT_SYMBOL(crt_timing_sn_to_index);

static u32 timing_sn_convert(struct timing_convert *t, u32 sn)
{
	while (t->sn != TIMING_SN_INVALID) {
		if (sn == t->sn)
			return t->new_sn;
		t++;
	};

	return sn;
}

u32 crt_timing_index_3d_to_2d(u32 index)
{
	u32 sn, new_sn;

	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(hdmi_3d_to_2d, sn);

	uinfo("Convert 3D format.sn(0x%04X to 0x%04X)\n", sn, new_sn);
	return crt_timing_sn_to_index(new_sn);
}
EXPORT_SYMBOL(crt_timing_index_3d_to_2d);

u32 crt_timing_index_4k_to_1080p(u32 index)
{
	u32 sn, new_sn;

	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(hdmi_4k_to_1080p, sn);

	uinfo("Convert 4K format.sn(0x%04X to 0x%04X)\n", sn, new_sn);
	return crt_timing_sn_to_index(new_sn);
}
EXPORT_SYMBOL(crt_timing_index_4k_to_1080p);

u32 crt_timing_index_4k_to_4k_low(u32 index)
{
	u32 sn, new_sn;

	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(hdmi_4k_to_4k_low, sn);

	uinfo("Convert 4K format.sn(0x%04X to 0x%04X)\n", sn, new_sn);
	return crt_timing_sn_to_index(new_sn);
}
EXPORT_SYMBOL(crt_timing_index_4k_to_4k_low);

u32 crt_timing_index_4k_low_420_to_4k_low_444(u32 index)
{
	u32 sn, new_sn;

	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(hdmi_4k_low_420_to_4k_low_444, sn);

	uinfo("Convert 4K format.sn(0x%04X to 0x%04X)\n", sn, new_sn);
	return crt_timing_sn_to_index(new_sn);
}
EXPORT_SYMBOL(crt_timing_index_4k_low_420_to_4k_low_444);

u32 crt_timing_index_i_to_p(u32 index, unsigned int down_convert_1080i)
{
	u32 sn, new_sn;
	struct timing_convert *table_to_use = table_interlace_to_progressive;

	if (down_convert_1080i)
		table_to_use = table_1080i_to_720p;

	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(table_to_use, sn);

	uinfo("Convert interlace format.sn(0x%04X to 0x%04X)\n", sn, new_sn);
	return crt_timing_sn_to_index(new_sn);
}
EXPORT_SYMBOL(crt_timing_index_i_to_p);

static u32 crt_timing_index_to_preferred_timing(u32 index)
{
	u32 sn, new_sn;

	/*
	** This function is used to filter crt unsupported timing.
	*/
	sn = crt_timing_index_to_sn(index);
	new_sn = timing_sn_convert(crt_preferred_timing_sn_tb, sn);

	if (sn != new_sn)
		uinfo("Convert to CRT preferred timing format.sn(0x%04X to 0x%04X)\n", sn, new_sn);

	return crt_timing_sn_to_index(new_sn);
}

#if defined(CONFIG_ARCH_AST1500_CLIENT)
unsigned int crt_cec_send(struct s_crt_drv *crt, u8 *buf, u32 size)
{
	xHal_cec_send(crt, buf, size);

	return 0;
}
EXPORT_SYMBOL(crt_cec_send);

unsigned int crt_cec_topology(struct s_crt_drv *crt, u8 *buf, u32 scan)
{
	return xHal_cec_topology(crt, buf, scan);
}
EXPORT_SYMBOL(crt_cec_topology);

void crt_cec_pa_cfg(struct s_crt_drv *crt, u16 address)
{
	xHal_cec_pa_cfg(crt, address);
}
EXPORT_SYMBOL(crt_cec_pa_cfg);

void crt_cec_la_cfg(struct s_crt_drv *crt, u8 address)
{
	xHal_cec_la_cfg(crt, address);
}
EXPORT_SYMBOL(crt_cec_la_cfg);

#endif

EXPORT_SYMBOL(vbufset);
EXPORT_SYMBOL(crt_change_scale_factor); /* SoC V1 only. non-atomic context */
EXPORT_SYMBOL(crt_change_scale_factor_atomic); /* SoC V1 only. atomic context */
EXPORT_SYMBOL(crt_set_vbuf_atomic); /* SoC V1 only. atomic context */
EXPORT_SYMBOL(crt_to_loopback_screen);
EXPORT_SYMBOL(crt_to_console_screen);
EXPORT_SYMBOL(crt_to_decode_screen);
EXPORT_SYMBOL(crt_setup_xmiter_only_decode);
#if 0//steven:not used outside of CRT
EXPORT_SYMBOL(crt_setup);
#endif
EXPORT_SYMBOL(crt_pwr_save);
EXPORT_SYMBOL(crt_get_force_timing_idx);
EXPORT_SYMBOL(crt_get_current_mode);
EXPORT_SYMBOL(crt_setup_audio);
EXPORT_SYMBOL(crt_disable_audio);
EXPORT_SYMBOL(crt_connect);
EXPORT_SYMBOL(crt_get);
EXPORT_SYMBOL(crt_disconnect);
EXPORT_SYMBOL(crt_setup_par);
EXPORT_SYMBOL(crt_reset_par);
EXPORT_SYMBOL(crt_hotplug_tx);
EXPORT_SYMBOL(crt_reg_tx_hotplug_callback);
EXPORT_SYMBOL(crt_dereg_tx_hotplug_callback);
EXPORT_SYMBOL(enable_vsync_int);
EXPORT_SYMBOL(crt_match_timing_table);
#if 0//steven:useless
EXPORT_SYMBOL(crt_disable_xmiter);
EXPORT_SYMBOL(crt_setup_xmiter);
#endif
EXPORT_SYMBOL(crt_get_tx_cap);
EXPORT_SYMBOL(crt_ft_cfg_to_mode_idx);

#if (CONFIG_AST1500_SOC_VER >= 2)
EXPORT_SYMBOL(disable_vsync_int);
EXPORT_SYMBOL(get_crt_vsync_int_stat_atomic); /* in atomic context */
EXPORT_SYMBOL(ack_crt_vsync_int_stat_atomic); /* in atomic context */
EXPORT_SYMBOL(crt_is_in_odd_field_atomic); /* in atomic context */
EXPORT_SYMBOL(crt_disable_flip);
EXPORT_SYMBOL(crt_enable_flip);
EXPORT_SYMBOL(crt_disable_desktop);
EXPORT_SYMBOL(crt_enable_desktop);
EXPORT_SYMBOL(crt_enable_desktop_atomic);
EXPORT_SYMBOL(crt_vsync_intr_location);
#endif
EXPORT_SYMBOL(crt_set_csc_format);
#if (CONFIG_AST1500_SOC_VER >= 3)
EXPORT_SYMBOL(crt_in_yuv422);
EXPORT_SYMBOL(crt_sw_flip_atomic); /* in atomic context */
EXPORT_SYMBOL(crt_display_address_cfg_atomic); /* in atomic context */
#elif (CONFIG_AST1500_SOC_VER >= 2)
EXPORT_SYMBOL(crt_terminal_cnt);
#endif
#if 0 //Not use anymore
void crt_display_console(void)
{
	if (!crt)
	{
		printk("CRT not initialized yet!!!\n");
		return;
	}

	down(&crt->crt_lock);
	crt_setup(crt, OWNER_CON);
	crt->CON_mode.clr = 0;
	if (crt->disp_select == CRT_2)
		__crt_change_scale_factor(crt, CRT_SCALE_FACTOR_MAX, CRT_SCALE_FACTOR_MAX);
#ifdef AST1510_CRT_A1
	_crt_enable_desktop(crt);
#endif
	up(&crt->crt_lock);
}
EXPORT_SYMBOL(crt_display_console);
#endif


#if SUPPORT_HDCP_REPEATER

extern struct s_xmiter_info xmiter_info[];

void CRT_HDCP1_Downstream_Port_Set_Mode(unsigned char repeater)
{
	u32 mode = (u32)repeater;
	queue_crt_work_n_wait(crt, (crt_work_func_t)xHal_hdcp1_set_mode, (void *)mode, 0);
}
EXPORT_SYMBOL(CRT_HDCP1_Downstream_Port_Set_Mode);

void _CRT_HDCP1_Downstream_Port_Auth(struct s_crt_drv *crt, int enable)
{
	struct s_crt_info *mode;

	/* No HDCP on console screen, this is for analog output to display console screen properly */
	/*
	** Bruce151211. This function is designed for controlling HDCP from
	** VE driver. When the screen belongs to 'console screen', the HDCP mode
	** should always follow the setting passed to crt_steup_par().
	** So, this function should just return and do nothing.
	**
	** FIXME. We don't handle HDCP fail re-try here. Just return. Problem?
	*/
	if (OWNER_CON == crt->current_owner) {
		//mode->EnHDCP = HDCP_DISABLE;
		return;
	}

	switch (crt->current_owner) {
	case OWNER_VE1:
		mode = &crt->VE1_mode;
		break;
	case OWNER_CON:
		mode = &crt->CON_mode;
		break;
	case OWNER_HOST:
		mode = &crt->HOST_mode;
		break;
	default:
		mode = &crt->VE1_mode;
		break;
	}
	/* Bruce121211. Under repeater mode. The EnHDCP flag will be overwritten by this fun call.
	** We updated EnHDCP here.
	*/
	mode->EnHDCP = enable;

	xHal_hdcp1_auth(crt, enable);
}
void CRT_HDCP1_Downstream_Port_Auth(int enable)
{
	queue_crt_work_n_wait(crt, (crt_work_func_t)_CRT_HDCP1_Downstream_Port_Auth, (void *)enable, 0);
}
EXPORT_SYMBOL(CRT_HDCP1_Downstream_Port_Auth);

void CRT_HDCP1_Downstream_Port_Encrypt(unsigned char enable)
{
	u32 en = (u32)enable;
	queue_crt_work_n_wait(crt, (crt_work_func_t)xHal_hdcp1_encrypt, (void *)en, 0);
}
EXPORT_SYMBOL(CRT_HDCP1_Downstream_Port_Encrypt);

pfn_Update_HDCP1_Downstream_Port_Auth_Status_Callback pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback = NULL;

void CRT_HDCP1_Downstream_Port_Reg_Auth_Status_Callback(pfn_Update_HDCP1_Downstream_Port_Auth_Status_Callback	pcallback)
{
	pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback = pcallback;
}
EXPORT_SYMBOL(CRT_HDCP1_Downstream_Port_Reg_Auth_Status_Callback);

void CRT_HDCP1_Downstream_Port_Dereg_Auth_Status_Callback(void)
{
	pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback = NULL;
}
EXPORT_SYMBOL(CRT_HDCP1_Downstream_Port_Dereg_Auth_Status_Callback);

#define TO_AUTH_STRING(num) \
({ \
	unsigned char *str; \
	switch (num) { \
	case 0: \
		str = "FAILED"; \
		break; \
	case 1: \
		str = "PASS"; \
		break; \
	case 2: \
		str = "INIT"; \
		break; \
	default: \
		str = "UNKNOWN"; \
		break; \
	} \
	str; \
})

struct HDCP_callback_status {
	unsigned char authenticated;
	unsigned char Bcaps;
	unsigned short Bstatus;
	unsigned char Bksv[KSV_LENGTH];
	unsigned char KSV_FIFO[KSV_LENGTH * 0x7F];
};

static void _pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback(struct s_crt_drv *crt, void *data)
{
	struct HDCP_callback_status *status = data;

	if (pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback)
		pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback(
		    status->authenticated,
		    status->Bksv,
		    status->Bcaps,
		    status->Bstatus,
		    status->KSV_FIFO);
}

void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO)
{
	struct HDCP_callback_status *status;

	uinfo("HDCP Status Callback: %d (%s)\n", authenticated, TO_AUTH_STRING(authenticated));

	/* Under console screen, we are not going to handle HDCP status callback. */
	if (crt->current_owner == OWNER_CON) {
		uinfo("Ignore HDCP status callback\n");
		return;
	}
	status = kzalloc(sizeof(struct HDCP_callback_status), GFP_KERNEL);
	BUG_ON(status == NULL);
	status->authenticated = authenticated;
	status->Bcaps = Bcaps;
	status->Bstatus = Bstatus;
	if (authenticated == 1) {
		/* Only valid when authenticated == 1. */
		if (Bksv)
			memcpy(status->Bksv, Bksv, KSV_LENGTH);
		if (KSV_FIFO)
			memcpy(status->KSV_FIFO, KSV_FIFO, (Bstatus & 0x7f) * KSV_LENGTH);
	}
	/*
	** There are different context may trigger this callback.
	** To simplify the case and separate different context from
	** VE driver (caller), we use queue_crt_work() for real callback.
	*/
	queue_crt_work(crt, _pUpdate_HDCP1_Downstream_Port_Auth_Status_Callback, status, 1);
}
#endif
