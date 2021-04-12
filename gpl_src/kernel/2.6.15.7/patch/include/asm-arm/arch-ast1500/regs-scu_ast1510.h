/* This file need to be revised
**
*/

#if (CONFIG_AST1500_SOC_VER == 2)

#ifndef _REGS_SCU_AST1510_H_
#define _REGS_SCU_AST1510_H_

#include <asm/arch/drivers/crt.h>

#define VPCLKDLYB321(v)         (((v) >> 1) & 0x7UL)
#define VPCLKDLYB0(v)           ((v) & 0x1UL)
typedef enum {
	VPCLKDLY_0NS      =     (0x0),       /* 0000  */
	VPCLKDLY_0d5NS    =     (0x1),       /* 0001  */
	VPCLKDLY_1NS      =     (0x2),       /* 0010  */
	VPCLKDLY_1d5NS    =     (0x3),       /* 0011  */
	VPCLKDLY_2NS      =     (0x4),       /* 0100  */
	VPCLKDLY_2d5NS    =     (0x5),       /* 0101  */
	VPCLKDLY_3NS      =     (0x6),       /* 0110  */
	VPCLKDLY_3d5NS    =     (0x7),       /* 0111  */
	VPCLKDLY_I0NS     =     (0x8),       /* 1000  */ /* Clock inversed and delay ~ 0ns */
	VPCLKDLY_I0d5NS   =     (0x9),       /* 1001  */
	VPCLKDLY_I1NS     =     (0xA),       /* 1010  */
	VPCLKDLY_I1d5NS   =     (0xB),       /* 1011  */
	VPCLKDLY_I2NS     =     (0xC),       /* 1100  */
	VPCLKDLY_I2d5NS   =     (0xD),       /* 1101  */
	VPCLKDLY_I3NS     =     (0xE),       /* 1110  */
	VPCLKDLY_I3d5NS   =     (0xF)        /* 1111  */
} vpo_clk_delay;

/*
 *  Register for SCU
 */
#define RSCU_PRTCT                                0x00        /*    protection key register    */
#define      PRTCT_UNLOCK                         0x1688A8A8

#define RSCU_RESET                                0x04        /*    system reset control register */
#define      RESET_DRAM_WDT                       (0x1UL << 0)
#define      RESET_VE_RESET                       (0x1UL << 6)
#define      RESET_MAC1_RESET                     (0x1UL << 11)
#define      RESET_MAC2_RESET                     (0x1UL << 12)
#define      RESET_CRT_RESET                      (0x1UL << 13)
#define      RESET_USB20_RESET                    (0x1UL << 14)
#define      RESET_USB11HC_RESET                  (0x1UL << 15)
#define      RESET_I2S_RESET                      (0x1UL << 17)
#define      RESET_IR_RESET                       (0x1UL << 30)

#define RSCU_CLKSEL                               0x08        /*    clock selection register    */
#define      CLKSEL_VPACLKDLYB321_MASK            (0x7UL << 8)
#define      CLKSEL_VPACLKDLYB321(v)              (((v) << 8) & CLKSEL_VPACLKDLYB321_MASK)
#define      CLKSEL_VPACLKDLYB321_GET(r)          (((r) & CLKSEL_VPACLKDLYB321_MASK) >> 8)
#define      CLKSEL_PORTACLKSEL_MASK              (0x1UL << 11)
#define      CLKSEL_PORTACLKSEL(v)                (((v) << 11) & CLKSEL_PORTACLKSEL_MASK)
#define      CLKSEL_PCLKD_MASK                    (0x7UL << 23)
#define      CLKSEL_PCLKD(v)                      ((v) << 23) & CLKSEL_PCLKD_MASK)
#define      CLKSEL_PCLKD_GET(r)                  (((r) & CLKSEL_PCLKD_MASK) >> 23)

#define RSCU_CLKSTOP                              0x0C        /*    clock stop control register    */
#define      CLKSTOP_ECLK_MASK                    (0x1UL << 0)
#define      CLKSTOP_ECLK(v)                      (((v) << 0) & CLKSTOP_ECLK_MASK)
#define      CLKSTOP_V1CLK_MASK                   (0x1UL << 3)
#define      CLKSTOP_V1CLK(v)                     (((v) << 3) & CLKSTOP_V1CLK_MASK)
#define      CLKSTOP_DCLK_MASK                    (0x1UL << 5)
#define      CLKSTOP_DCLK(v)                      (((v) << 5) & CLKSTOP_DCLK_MASK)
#define      CLKSTOP_USB11HC                      (0x1UL << 9)
#define      CLKSTOP_D1CLK_MASK                   (0x1UL << 10)
#define      CLKSTOP_D1CLK(v)                     (((v) << 10) & CLKSTOP_D1CLK_MASK)
#define      CLKSTOP_V2CLK_MASK                   (0x1UL << 12)
#define      CLKSTOP_V2CLK(v)                     (((v) << 12) & CLKSTOP_V2CLK_MASK)
#define      CLKSTOP_USB20_ENCLK                  (0x1UL << 14)
#define      CLKSTOP_I2S_MASK                     (0x1UL << 18)
#define      CLKSTOP_I2S                          (0x1UL << 18)
#define      CLKSTOP_MAC1_MASK                    (0x1UL << 20)
#define      CLKSTOP_MAC2_MASK                    (0x1UL << 21)
#define      CLKSTOP_IR_MASK                      (0x1UL << 24)

#define RSCU_HPLL                                 0x24        /*    H-PLL Parameter register        */
#define      HPLL_DNMRATR_MASK                    (0xFUL << 0)
#define      HPLL_DNMRATR(v)                      (((v) << 0) & HPLL_DNMRATR_MASK)
#define      HPLL_DNMRATR_GET(r)                  (((r) & HPLL_DNMRATR_MASK) >> 0)
#define      HPLL_OD_MASK                         (0x1UL << 4)
#define      HPLL_OD(v)                           (((v) << 4) & HPLL_OD_MASK)
#define      HPLL_OD_GET(r)                       (((r) & HPLL_OD_MASK) >> 4)
#define      HPLL_NMRTR_MASK                      (0x3FUL << 5)
#define      HPLL_NMRTR(v)                        (((v) << 5) & HPLL_NMRTR_MASK)
#define      HPLL_NMRTR_GET(r)                    (((r) & HPLL_NMRTR_MASK) >> 5)
#define      HPLL_PD_MASK                         (0x7UL << 12)
#define      HPLL_PD(v)                           (((v) << 12) & HPLL_PD_MASK)
#define      HPLL_PD_HIGH_BIT                     (0x1UL << 14)
#define      HPLL_HPLL_OFF                        (0x1UL << 16)
#define      HPLL_BYPASS_EN                       (0x1UL << 17)
#define      HPLL_FROMREG_EN                      (0x1UL << 18)

#define RSCU_MISC1CTRL                            0x2C        /*    Misc. Control register */
#define      MISC1CTRL_D1PLLOFF_MASK              (0x1UL << 2)
#define      MISC1CTRL_D1PLLOFF(v)                (((v) << 2) & MISC1CTRL_D1PLLOFF_MASK)
#define      MISC1CTRL_VDODACOFF_MASK             (0x1UL << 3)
#define      MISC1CTRL_VDODACOFF(v)               (((v) << 3) & MISC1CTRL_VDODACOFF_MASK)
#define      MISC1CTRL_D2PLLOFF_MASK              (0x1UL << 4)
#define      MISC1CTRL_D2PLLOFF(v)                (((v) << 4) & MISC1CTRL_D2PLLOFF_MASK)
#define      MISC1CTRL_VPACLKDLYB0_MASK           (0x1UL << 9)
#define      MISC1CTRL_VPACLKDLYB0(v)             (((v) << 9) & MISC1CTRL_VPACLKDLYB0_MASK)
#define      MISC1CTRL_VPACLKDLYB0_GET(r)         (((r) & MISC1CTRL_VPACLKDLYB0_MASK) >> 9)
#define      MISC1CTRL_SELDACSRC_MASK             (0x3UL << 16)
#define      MISC1CTRL_SELDACSRC(v)               (((v) << 16) & MISC1CTRL_SELDACSRC_MASK)
#define      MISC1CTRL_SELDVOSRC_MASK             (0x1UL << 18)
#define      MISC1CTRL_SELDVOSRC(v)               (((v) << 18) & MISC1CTRL_SELDVOSRC_MASK)
#define      MISC1CTRL_SELPLL_MASK                (0x3UL << 20)
#define      MISC1CTRL_SELPLL(v)                  (((v) << 20) & MISC1CTRL_SELPLL_MASK)
#define      MISC1CTRL_SELPLL_D1VGA_D2GFX         (0x0)
#define      MISC1CTRL_SELPLL_D1I2S_D2GFX         (0x2)
#define      MISC1CTRL_SELPLL_D1VGA_D2I2S         (0x3)

#define RSCU_PCICONF1                             0x30        /*    PCI configuration setting register#1 */
#define RSCU_PCICONF2                             0x34        /*    PCI configuration setting register#2 */
#define RSCU_PCICONF3                             0x38        /*    PCI configuration setting register#3 */
#define RSCU_RESETSTAT                            0x3C        /*    SCU3C: System Reset Control Register */
#define RSCU_SOCSCRATCH0                          0x40        /*    SOC scratch 0~31 register */
#define RSCU_SOCSCRATCH1                          0x44        /*    SOC scratch 32~63 register */

#define RSCU_MISC2CTRL                            0x4C        /*    Misc. Control register */
#define      MISC2CTRL_DACGAIN_MASK               (0x7UL << 3)
#define      MISC2CTRL_DACGAIN(v)                 (((v) << 3) & MISC2CTRL_DACGAIN_MASK)

/* SCU50 ~ SCU6C: VGA Scratch Register */
#define RSCU_VGASCRATCH0                          0x50
#define RSCU_VGASCRATCH1                          0x54
#define RSCU_VGASCRATCH2                          0x58
#define RSCU_VGASCRATCH3                          0x5C
#define RSCU_VGASCRATCH4                          0x60
#define RSCU_VGASCRATCH5                          0x64
#define RSCU_VGASCRATCH6                          0x68
#define RSCU_VGASCRATCH7                          0x6C

#define RSCU_HWSTRAP1                             0x70        /*    SCU70: Hardware Trapping Register */
#define      HWSTRAP1_HPLLSLT_MASK                (0x3UL << 8)
#define      HWSTRAP1_HPLLSLT_GET(r)              (((r) & HWSTRAP1_HPLLSLT_MASK) >> 8)
#define      HWSTRAP1_HPLLSLT_SET(v)              (((v) << 8) & HWSTRAP1_HPLLSLT_MASK)
#define      HWSTRAP1_HPLLSLT_384MHZ              (0x0)
#define      HWSTRAP1_HPLLSLT_360MHZ              (0x1)
#define      HWSTRAP1_HPLLSLT_336MHZ              (0x2)
#define      HWSTRAP1_HPLLSLT_408MHZ              (0x3)
#define      HWSTRAP1_AHBCLKDIV_MASK              (0x3UL << 10)
#define      HWSTRAP1_AHBCLKDIV_GET(r)            (((r) & HWSTRAP1_AHBCLKDIV_MASK) >> 10)
#define      HWSTRAP1_AHBCLKDIV_SET(v)            (((v) << 10) & HWSTRAP1_AHBCLKDIV_MASK)
#define      HWSTRAP1_AHBCLKDIV_DIV1              (0x0)
#define      HWSTRAP1_AHBCLKDIV_DIV2              (0x1)
#define      HWSTRAP1_AHBCLKDIV_DIV4              (0x2)
#define      HWSTRAP1_AHBCLKDIV_DIV3              (0x3)
#define      HWSTRAP1_VDOTYPE_MASK                (0x3UL << 28)
#define      HWSTRAP1_VDOTYPE_GET(r)              (((r) & HWSTRAP1_VDOTYPE_MASK) >> 28)
#define      HWSTRAP1_LBSTUS_MASK                 (0x1UL << 30)	/* lookback status */
#define      HWSTRAP1_ISCLIENT_MASK               (0x1UL << 31)

#define RSCU_PINCTRL1                             0x74     /*  SCU74: Multi-function Pin Control #1	*/
#define      PINCTRL1_VGAPIN_MASK                 (0x1UL << 15)
#define      PINCTRL1_VGAPIN(v)                   (((v) << 15) & PINCTRL1_VGAPIN_MASK)
#define      PINCTRL1_VDOPORTA_MASK               (0x1UL << 16)
#define      PINCTRL1_VDOPORTA(v)                 (((v) << 16) & PINCTRL1_VDOPORTA_MASK)
#define      PINCTRL1_VDOPORTB_MASK               (0x1UL << 17)
#define      PINCTRL1_VDOPORTB(v)                 (((v) << 17) & PINCTRL1_VDOPORTB_MASK)
#define      PINCTRL1_VDOVP1_MASK                 (0x1UL << 22)
#define      PINCTRL1_VDOVP1(v)                   (((v) << 22) & PINCTRL1_VDOVP1_MASK)
#define      PINCTRL1_VDOVP2_MASK                 (0x1UL << 23)
#define      PINCTRL1_VDOVP2(v)                   (((v) << 23) & PINCTRL1_VDOVP2_MASK)

#define RSCU_REVISION                             0x7C        /*  SCU7C: Silicon Revision ID Register		*/
#define      REVISION_MASK                        (0xFFUL << 16)
#define      REVISION_GET(r)                      (((r) & REVISION_MASK) >> 16)

#define RSCU_PINCTRL2                             0x84		/*	SCU84: Multi-function Pin Control #2	*/
/* ??
#define      PINCTRL2_VDOPORTASIGEDG_MASK         (0x1UL << 0)
#define      PINCTRL2_VDOPORTASIGEDG(v)           (((v) << 0) & PINCTRL2_VDOPORTASIGEDG_MASK)
#define      PINCTRL2_VDOPORTBSIGEDG_MASK         (0x1UL << 1)
#define      PINCTRL2_VDOPORTBSIGEDG(v)           (((v) << 1) & PINCTRL2_VDOPORTBSIGEDG_MASK)
*/
#define      PINCTRL2_ENVGAHS_MASK                (0x1UL << 12)   /* Enable VGA/CRT VGAHS output function pin */
#define      PINCTRL2_ENVGAHS_EN                  (0x1UL << 12)
#define      PINCTRL2_ENVGAVS_MASK                (0x1UL << 13)   /*Enable VGA/CRT VGAVS output function pin */
#define      PINCTRL2_ENVGAVS_EN                  (0x1UL << 13)
#define      PINCTRL2_ENVGADDCCLK_MASK            (0x1UL << 14)   /*Enable VGA/CRT DDCCLK output function pin */
#define      PINCTRL2_ENVGADDCCLK_EN              (0x1UL << 14)
#define      PINCTRL2_ENVGADDCDAT_MASK            (0x1UL << 15)   /*Enable VGA/CRT DDCDAT output function pin */
#define      PINCTRL2_ENVGADDCDAT_EN              (0x1UL << 15)

#define RSCU_PINCTRL3                             0x88        /*  Multi-function Pin Control #3   */
#define RSCU_PINCTRL4                             0x8C        /*  Multi-function Pin Control #4   */
#define      PINCTRL4_NOITRNLPULLDWN_GPIOH        (0x1UL << 23)

#define RSCU_PINCTRL5                             0x90        /* Multi-function Pin Control #5 */
#define      PINCTRL5_USB11P2PIN_MASK             (0x1UL << 3)
#define      PINCTRL5_USB11P2PIN(v)               (((v) << 3) & PINCTRL5_USB11P2PIN_MASK)
#define      PINCTRL5_USB11P2PIN_HC               (0x1)
#define      PINCTRL5_USB11P2PIN_HID              (0x0)
#define      PINCTRL5_DGLVDOPIN_MASK              (0x3UL << 4)
#define      PINCTRL5_DGLVDOPIN(v)                (((v) << 4) & PINCTRL5_DGLVDOPIN_MASK)  /* Enable digital video function pins*/
#define      PINCTRL5_UART6_EN                    (0x1UL << 7)

#define RSCU_ULCKIP                               0xF0
#define      ULCKIP_VEDCDE                        0x2000DEEA
#define      ULCKIP_I2S                           0xA0E0E0D3
#define      ULCKIP_I2S_ON                        (0x1UL << 2)
#define      ULCKIP_IR                            0x10ADDEED
#define      ULCKIP_IR_ON                         (0x1UL << 4)

#endif //#ifndef _REGS_SCU_AST1510_H_
#endif //#if (CONFIG_AST1500_SOC_VER == 2)
