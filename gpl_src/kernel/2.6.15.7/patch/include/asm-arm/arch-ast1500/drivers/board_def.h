#ifndef _AST1500_BOARD_DEF_H
#define	_AST1500_BOARD_DEF_H

#include <linux/autoconf.h>

/*
** Whoever need PATCH_CODEV3 has to redefine it in its own specific board_def.
*/
#define PATCH_CODEV3 (0)
/*
** Patch code define
*/
/* VR238, VR23C need init to 0, otherwise encode interrupt like crazy */
#define PATCH_AST1520A0_BUG14071001_VR238               (0x1UL << 0)
/* VE dynamic adjust clock function doesn't work for encode. AST1520 don't need it. Disable it. */
#define PATCH_AST1520A0_BUG14071002_VE_SLOW             (0x1UL << 1)
/* AST1520A0 MAC's high priority queue is buggy. Don't use it. */
#define PATCH_AST1520A0_BUG14030300_MAC_HQ              (0x1UL << 2)
/* AST1520A0 MAC FTL false alert under 100Mbps. Don't use it. */
#define PATCH_AST1520A0_BUG14080600_MAC_FTL             (0x1UL << 3)
/* AST1520A0 Can't do full chip watch dog reset due to HPLL issue. Don't use it. */
#define PATCH_AST1520A0_BUG14071000_HPLL_RESET          (0x1UL << 4)
/* AST1520A0 SCU engine reset may kill DRAM controller. WORKAROUND:Should wait engine before engine reset. */
#define PATCH_AST1520A0_BUG14081201_SCU_RESET           (0x1UL << 5)
/* AST1520A0 4K H. downscaling. For full frame rate performance. */
#define PATCH_AST1520A0_BUG14090302_VE_4K30HZ_HDOWN     (0x1UL << 6)
/* AST1520A0 Video Wall anti-tearing patch. */
#define PATCH_AST1520A0_BUG14090203_VE_ANTI_FLICKER     (0x1UL << 7)
/* AST1520A0 I2S dual output port */
#define PATCH_AST1520A0_I2S_DUAL_OUTPUT                 (0x1UL << 8)
/* AST1520A0 CRT 4K output first h line green and flashing. */
#define PATCH_AST1520A0_BUG14082801_CRT_4K              (0x1UL << 9)
/* AST1520 standard JPEG decode hang. */
#define PATCH_AST1520A1_BUG15051500_JPEG_VE_HANG        (0x1UL << 10)


#if defined(CONFIG_ARCH_FGA1500) //AST3000_FPGA

	#include "board_def_1500_fpga.h"

#elif defined(CONFIG_ARCH_PTV1500) //AST1500_BOARD_V2

	#include "board_def_1500_V2.h"

#elif defined(CONFIG_AST1500_BOARD_V3) //AST1500_BOARD_V3

	#include "board_def_1500_V3.h"

/* CONFIG_AST1500_BOARD_V4 ----------------------- */
#elif defined(CONFIG_AST1500_BOARD_V4)

	#include "board_def_1500_V4.h"

/* CONFIG_AST1500_BOARD_V5 ----------------------- */
#elif defined (CONFIG_AST1500_BOARD_V5) || defined(CONFIG_AST1510_BOARD_EVA_V1)

	#include "board_def_1500_V5.h"

#elif defined(CONFIG_AST1510_BOARD_FPGA)

	#include "board_def_1510_fpga.h"

#elif defined(CONFIG_AST1510_BOARD_EVA_V2)

	#include "board_def_1510_V2.h"

#elif defined(CONFIG_AST1510_BOARD_EVA_V4)

	#include "board_def_1510_V4.h"

#elif defined(CONFIG_AST1510_BOARD_EVA_V5)

	#include "board_def_1510_V5.h"

#elif defined(CONFIG_AST1520_BOARD_FPGA_ARM9)

	#include "board_def_1520_fpga_arm9.h"

#elif defined(CONFIG_AST1520_BOARD_FPGA_ARM11)

	#include "board_def_1520_fpga_arm11.h"

#elif defined(CONFIG_AST1520_BOARD_EVA_V1)

	#include "board_def_1520_V1.h"

#elif defined(CONFIG_AST1520_BOARD_EVA_V2)

	#include "board_def_1520_V2.h"

#elif defined(CONFIG_AST1520_BOARD_EVA_V3)

	#include "board_def_1520_V3.h"

#else
	#error "Board type undefined?!"
#endif

#endif //#ifndef _BOARD_DEF_H

