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

#if (CONFIG_AST1500_SOC_VER == 3)

#ifndef _REGS_SCU_AST1520_H_
#define _REGS_SCU_AST1520_H_

//#include <asm/arch/drivers/crt.h>

/*
 *  Register for SCU
 */
#define RSCU_PRTCT                                0x00        /*    protection key register    */
#define      PRTCT_UNLOCK                         0x1688A8A8

#define RSCU_RESET                                0x04        /*    system reset control register */
#define      RESET_DRAM_WDT                       (0x1UL << 0)
#define      RESET_I2C_RESET                      (0x1UL << 2)
#define      RESET_USB20P2_RESET                  (0x1UL << 3)
#define      RESET_VE_RESET                       (0x1UL << 6)
#define      RESET_MAC1_RESET                     (0x1UL << 11)
#define      RESET_MAC2_RESET                     (0x1UL << 12)
#define      RESET_CRT_RESET                      (0x1UL << 13)
#define      RESET_USB20_RESET                    (0x1UL << 14)
#define      RESET_USB11HC_RESET                  (0x1UL << 15)
#define      RESET_IR_RESET                       (0x1UL << 30)
#define      RESET_I2S_RESET                      (0x1UL << 31)

#define RSCU_CLKSEL                               0x08        /*    clock selection register    */
#define      CLKSEL_VESRC_MASK                    (0x3UL << 2)
#define             VESRC_MPLL                    (0)
#define             VESRC_HPLL                    (0x1UL << 2)
#define             VESRC_D1PLL                   (0x2UL << 2)
#define      CLKSEL_CRTCLKL_MASK                  (0x3UL << 8)
#define      CLKSEL_HPLL2MAC_DIV_MASK             (0x7UL << 16)
#define      CLKSEL_HPLL2MAC_DIV(v)               (((v) << 16) & CLKSEL_HPLL2MAC_DIV_MASK)
#define      CLKSEL_PCLKD_MASK                    (0x7UL << 23)
#define      CLKSEL_PCLKD(v)                      (((v) << 23) & CLKSEL_PCLKD_MASK)
#define      CLKSEL_PCLKD_GET(r)                  (((r) & CLKSEL_PCLKD_MASK) >> 23)
#define      CLKSEL_VECLKDIV_MASK                 (0x7UL << 28)
#define             VECLKDIV_DIV2                 (0x0UL << 28)
#define             VECLKDIV_DIV3                 (0x2UL << 28)
#define             VECLKDIV_DIV4                 (0x3UL << 28)
#define             VECLKDIV_DIV5                 (0x4UL << 28)
#define             VECLKDIV_DIV6                 (0x5UL << 28)
#define             VECLKDIV_DIV7                 (0x6UL << 28)
#define             VECLKDIV_DIV8                 (0x7UL << 28)
#define      CLKSEL_VESLOW_EN                     (0x1UL << 31)

#define RSCU_CLKSTOP                              0x0C        /*    clock stop control register    */
#define      CLKSTOP_ECLK_MASK                    (0x1UL << 0)
#define      CLKSTOP_ECLK(v)                      (((v) << 0) & CLKSTOP_ECLK_MASK)
#define      CLKSTOP_V1CLK_MASK                   (0x1UL << 3)
#define      CLKSTOP_V1CLK(v)                     (((v) << 3) & CLKSTOP_V1CLK_MASK)
#define      CLKSTOP_DCLK_MASK                    (0x1UL << 5)
#define      CLKSTOP_DCLK(v)                      (((v) << 5) & CLKSTOP_DCLK_MASK)
#define      CLKSTOP_USB20P2                      (0x1UL << 7)
#define      CLKSTOP_USB11HC                      (0x1UL << 9)
#define      CLKSTOP_D1CLK_MASK                   (0x1UL << 10)
#define      CLKSTOP_D1CLK(v)                     (((v) << 10) & CLKSTOP_D1CLK_MASK)
#define      CLKSTOP_USB20_ENCLK                  (0x1UL << 14)
#define      CLKSTOP_MAC1_MASK                    (0x1UL << 20)
#define      CLKSTOP_MAC2_MASK                    (0x1UL << 21)
#define      CLKSTOP_RSA_MASK                     (0x1UL << 24)
#define      CLKSTOP_UART_3_MASK                  (0x1UL << 25)
#define      CLKSTOP_UART_4_MASK                  (0x1UL << 26)
#define      CLKSTOP_SD_MASK                      (0x1UL << 27)
#define      CLKSTOP_I2S_MASK                     (0x1UL << 30)
#define      CLKSTOP_IR_MASK                      (0x1UL << 31)


#define RSCU_FCNTCTRL                             0x10        /*    frequency counter control register    */
#define RSCU_FCNTVAL                              0x14        /*    frequency counter measure register     */
#define RSCU_D2PLL                                0x1C        /*    D2-PLL Parameter register       */

#define RSCU_MPLL                                 0x20        /*    M-PLL Parameter register        */
#define      MPLL_N_MASK                          (0x1FUL << 0)
#define      MPLL_N_GET(r)                        (((r) & MPLL_N_MASK) >> 0)
#define      MPLL_M_MASK                          (0xFFUL << 5)
#define      MPLL_M_GET(r)                        (((r) & MPLL_M_MASK) >> 5)
#define      MPLL_P_MASK                          (0x3FUL << 13)
#define      MPLL_P_GET(r)                        (((r) & MPLL_P_MASK) >> 13)
#define      MPLL_MPLL_OFF                        (0x1UL << 19)
#define      MPLL_BYPASS_EN                       (0x1UL << 20)
#define      MPLL_RESET_EN                        (0x1UL << 21)

#define RSCU_HPLL                                 0x24        /*    H-PLL Parameter register        */
#define      HPLL_N_MASK                          (0x1FUL << 0)
#define      HPLL_N_GET(r)                        (((r) & HPLL_N_MASK) >> 0)
#define      HPLL_M_MASK                          (0xFFUL << 5)
#define      HPLL_M_GET(r)                        (((r) & HPLL_M_MASK) >> 5)
#define      HPLL_P_MASK                          (0x3FUL << 13)
#define      HPLL_P_GET(r)                        (((r) & HPLL_P_MASK) >> 13)
#define      HPLL_HPLL_OFF                        (0x1UL << 19)
#define      HPLL_BYPASS_EN                       (0x1UL << 20)
#define      HPLL_RESET_EN                        (0x1UL << 21)

#define RSCU_DPLL                                 0x28
#define      DPLL_M_MASK                          (0xFFUL << 0)
#define      DPLL_M(v)                            (((v) << 0) & DPLL_M_MASK)
#define      DPLL_N_MASK                          (0x1FUL << 8)
#define      DPLL_N(v)                            (((v) << 8) & DPLL_N_MASK)
#define      DPLL_P_MASK                          (0x3FUL << 13)
#define      DPLL_P(v)                            (((v) << 13) & DPLL_P_MASK)
#define      DPLL_OD_MASK                         (0x7UL << 19)
#define      DPLL_OD(v)                           (((v) << 19) & DPLL_OD_MASK)
#define      DPLL_SIC_MASK                        (0x1FUL << 22)
#define      DPLL_SIC(v)                          (((v) << 22) & DPLL_SIC_MASK)
#define      DPLL_SIC_GET(r)                      (((r) & DPLL_SIC_MASK) >> 22)
#define      DPLL_SIP_MASK                        (0x1FUL << 27)
#define      DPLL_SIP(v)                          (((v) << 27) & DPLL_SIP_MASK)
#define      DPLL_SIP_GET(r)                      (((r) & DPLL_SIP_MASK) >> 27)

#define RSCU_MISC1CTRL                            0x2C        /*    Misc. Control register */
#define      MISC1CTRL_D1PLLOFF_MASK              (0x1UL << 2)
#define      MISC1CTRL_D1PLLOFF(v)                (((v) << 2) & MISC1CTRL_D1PLLOFF_MASK)
#define      MISC1CTRL_DACOFF_MASK                (0x1UL << 3)
#define      MISC1CTRL_DACOFF(v)                  (((v) << 3) & MISC1CTRL_DACOFF_MASK)
#define      MISC1CTRL_D2PLLOFF_MASK              (0x1UL << 4)
#define      MISC1CTRL_D2PLLOFF(v)                (((v) << 4) & MISC1CTRL_D2PLLOFF_MASK)
#define      MISC1CTRL_SELDACSRC_MASK             (0x3UL << 16)
#define      MISC1CTRL_SELDACSRC(v)               (((v) << 16) & MISC1CTRL_SELDACSRC_MASK)
#define      MISC1CTRL_SELDACSRC_VGA              (0)
#define      MISC1CTRL_SELDACSRC_CRT2             (0x1)
#define      MISC1CTRL_SELDACSRC_PSSTHRGHA        (0x2)
#define      MISC1CTRL_SELDACSRC_PSSTHRGHB        (0x3)
#define      MISC1CTRL_SELDVOSRC_MASK             (0x1UL << 18)
#define      MISC1CTRL_SELDVOSRC(v)               (((v) << 18) & MISC1CTRL_SELDVOSRC_MASK)
#define      MISC1CTRL_DPLLPARAM_FRMSCU           (0x1UL << 20) /* D1-PLL parameter from SCU or from VGA) */
#define      MISC1CTRL_CRTCLKH_40M                (0x1UL << 21) /* CRT port output clock selection high bit. Set to 1 force 40MHz out */

#define RSCU_PCICONF1                             0x30        /*    PCI configuration setting register#1 */
#define RSCU_PCICONF2                             0x34        /*    PCI configuration setting register#2 */
#define RSCU_PCICONF3                             0x38        /*    PCI configuration setting register#3 */
#define RSCU_RESETSTAT                            0x3C        /*    System reset contrl/status register*/
#define RSCU_SOCSCRATCH0                          0x40        /*    SOC scratch 0~31 register */
#define RSCU_SOCSCRATCH1                          0x44        /*    SOC scratch 32~63 register */

#define RSCU_MACCLK                               0x48        /*    MAC interface clock delay setting register */
#define      MACCLK_MAC1TXDLY_MASK                (0x3FUL << 0)
#define      MACCLK_MAC1TXDLY(v)                  (((v) << 0) & MACCLK_MAC1TXDLY_MASK)
#define      MACCLK_MAC2TXDLY_MASK                (0x3FUL << 6)
#define      MACCLK_MAC2TXDLY(v)                  (((v) << 6) & MACCLK_MAC2TXDLY_MASK)
#define      MACCLK_MAC1RXDLY_MASK                (0x3FUL << 12)
#define      MACCLK_MAC1RXDLY(v)                  (((v) << 12) & MACCLK_MAC1RXDLY_MASK)
#define      MACCLK_MAC2RXDLY_MASK                (0x3FUL << 18)
#define      MACCLK_MAC2RXDLY(v)                  (((v) << 18) & MACCLK_MAC2RXDLY_MASK)
#define      MACCLK_MAC1RII_TX_FLINGEDGE          (0x1UL << 24)
#define      MACCLK_MAC2RII_TX_FLINGEDGE          (0x1UL << 25)
#define      MACCLK_RGMII_125M_CLK_SRC_SELECT_MASK		(0x1UL << 31)
#define      MACCLK_RGMII_125M_CLK_SRC_SELECT_EXTERNAL		(0x0UL << 31)
#define      MACCLK_RGMII_125M_CLK_SRC_SELECT_INTERNAL_PLL	(0x1UL << 31)

#define RSCU_MISC2CTRL                            0x4C        /*    Misc. 2 Control register */
#define      MISC2CTRL_I2STXP1EN_MASK             (0x1UL << 9)
#define      MISC2CTRL_I2STXP1EN(v)               (((v) << 9) & MISC2CTRL_I2STXP1EN_MASK)
#define      MISC2CTRL_I2SRXP1EN_MASK             (0x1UL << 10)
#define      MISC2CTRL_I2SRXP1EN(v)               (((v) << 10) & MISC2CTRL_I2SRXP1EN_MASK)
#define      MISC2CTRL_I2SCLKMSTRP0EN_MASK        (0x1UL << 11)
#define      MISC2CTRL_I2SCLKMSTRP0EN(v)          (((v) << 11) & MISC2CTRL_I2SCLKMSTRP0EN_MASK)
#define      MISC2CTRL_I2SCLKMSTRP1EN_MASK        (0x1UL << 12)
#define      MISC2CTRL_I2SCLKMSTRP1EN(v)          (((v) << 12) & MISC2CTRL_I2SCLKMSTRP1EN_MASK)
#define      MISC2CTRL_I2SWSMSTRP0EN_MASK         (0x1UL << 13)
#define      MISC2CTRL_I2SWSMSTRP0EN(v)           (((v) << 13) & MISC2CTRL_I2SWSMSTRP0EN_MASK)
#define      MISC2CTRL_I2SWSMSTRP1EN_MASK         (0x1UL << 14)
#define      MISC2CTRL_I2SWSMSTRP1EN(v)           (((v) << 14) & MISC2CTRL_I2SWSMSTRP1EN_MASK)
#define      MISC2CTRL_RGMII_125M_CLK_DIV_RATIO_MASK   (0x7UL << 20)
#define      MISC2CTRL_RGMII_125M_CLK_DIV_RATIO(v)	(((v) << 20) & MISC2CTRL_RGMII_125M_CLK_DIV_RATIO_MASK)
#define      MISC2CTRL_RGMII_125M_CLK_DIV_SRC_SELECT_MASK	(0x1UL << 23)
#define      MISC2CTRL_RGMII_125M_CLK_DIV_SRC_SELECT_D2_PLL	(0x0UL << 23)
#define      MISC2CTRL_RGMII_125M_CLK_DIV_SRC_SELECT_M_PLL	(0x1UL << 23)


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
#define      HWSTRAP1_CLKIN_25MHZ                 (0x1UL << 23)
#define      HWSTRAP1_GPIOE_PASSTHROUGH           (0x1UL << 22)
#define      HWSTRAP1_GPIOD_PASSTHROUGH           (0x1UL << 21)
#define      HWSTRAP1_AXI2AHB_RATIO_MASK          (0x7UL << 9)
#define      HWSTRAP1_AXI2AHB_RATIO_GET(r)        (((r) & HWSTRAP1_AXI2AHB_RATIO_MASK) >> 9)

#define RSCU_RAMDOMGEN                            0x74        /*    random number generator register */
#define RSCU_RAMDOMDATA                           0x78        /*    random number generator data output*/

#define RSCU_REVISION                             0x7C        /*    Silicon revision ID register */
#define      REVISION_MASK                        (0xFFUL << 16)
#define      REVISION_GET(r)                      (((r) & REVISION_MASK) >> 16)

#define RSCU_MFPC1                                0x80        /*    Multi-function Pin Control#1*/
#define      MFPC1_MAC1PHYINT_EN                  (0x1UL << 0)
#define      MFPC1_MAC2PHYINT_EN                  (0x1UL << 1)
#define      MFPC1_UART4PINS_MASK                 (0xFFUL << 24)
#define      MFPC1_UART4PINS(v)                   (((v) << 24) & MFPC1_UART4PINS_MASK)
#define      MFPC1_UART4PINS_ALL                  (0xFF)
#define      MFPC1_UART4PINS_RXTX                 (0xC0)

#define RSCU_MFPC2                                0x84        /*    Multi-function Pin Control#2*/
#define      MFPC2_CRT_ALL_EN                     (0x0F << 12)
#define      MFPC2_ENVGAHS_MASK                   (0x1UL << 12)   /* Enable VGA/CRT VGAHS output function pin */
#define      MFPC2_ENVGAHS_EN                     (0x1UL << 12)
#define      MFPC2_ENVGAVS_MASK                   (0x1UL << 13)   /*Enable VGA/CRT VGAVS output function pin */
#define      MFPC2_ENVGAVS_EN                     (0x1UL << 13)
#define      MFPC2_ENVGADDCCLK_MASK               (0x1UL << 14)   /*Enable VGA/CRT DDCCLK output function pin */
#define      MFPC2_ENVGADDCCLK_EN                 (0x1UL << 14)
#define      MFPC2_ENVGADDCDAT_MASK               (0x1UL << 15)   /*Enable VGA/CRT DDCDAT output function pin */
#define      MFPC2_ENVGADDCDAT_EN                 (0x1UL << 15)
#define      MFPC2_DVIA_PINS_24BIT_ALL		  (0x7F9F << 17)
#define      MFPC2_DVIA_PINS_30BIT_ALL		  (0x7FFF << 17)
#define      MFPC2_DVIA_PINS_OFF		  (0x0000 << 17)
#define      MFPC2_DVIA_PINS_MASK		  (0x7FFF << 17)


#define RSCU_MFPC3                                0x88        /*    Multi-function Pin Control#3*/
#define      MFPC3_DVIA_PINS_24BIT_ALL		  (0xFF3FC << 0)
#define      MFPC3_DVIA_PINS_30BIT_ALL		  (0xFFFFF << 0)
#define      MFPC3_DVIA_PINS_OFF		  (0x00000 << 0)
#define      MFPC3_DVIA_PINS_MASK		  (0xFFFFF << 0)
#define      MFPC3_USB11HCP3_EN                   (0x3UL << 20)
#define      MFPC3_USB11HCP4_EN                   (0x3UL << 22)
#define      MFPC3_MAC1MDIO_EN                    (0x3UL << 30)
#define      MFPC3_VPOA_ALL_EN                    (0x3F << 24)

#define RSCU_MFPC4                                0x8C        /*    Multi-function Pin Control#4*/
#define      MFPC4_VPOA_ALL_EN                    (0xFF << 0)
#define      MFPC4_GPIOE_6_PASSTHROUGH_TO_7       (0x1 << 15)
#define      MFPC4_GPIOE_4_PASSTHROUGH_TO_5       (0x1 << 14)
#define      MFPC4_GPIOE_2_PASSTHROUGH_TO_3       (0x1 << 13)
#define      MFPC4_GPIOE_0_PASSTHROUGH_TO_1       (0x1 << 12)

#define RSCU_MFPC5                                0x90        /*    Multi-function Pin Control#5*/
#define      MFPC5_MAC2MDIO_EN                    (0x1UL << 2)
#define      MFPC5_DVIAPINS_MASK                  (0x3UL << 4)
#define      MFPC5_DVIAPINS(v)                    (((v) << 4) & MFPC5_DVIAPINS_MASK)
#define      MFPC5_DVIAPINS_DISABLE               (0x0)
#define      MFPC5_DVIAPINS_PA24BITS              (0x2)
#define      MFPC5_DVIAPINS_PA30BITS              (0x3)
#define      MFPC5_DVIBPINS_MASK                  (0x1UL << 6)
#define      MFPC5_DVIBPINS_EN                    (0x1UL << 6)
#define      MFPC5_IO6PINS_EN_MASK                (0x1UL << 7)
#define      MFPC5_I2C3_EN                        (0x1UL << 16)
#define      MFPC5_I2C4_EN                        (0x1UL << 17)
#define      MFPC5_I2C5_EN                        (0x1UL << 18)
#define      MFPC5_I2C6_EN                        (0x1UL << 19)
#define      MFPC5_I2C7_EN                        (0x1UL << 20)
#define      MFPC5_I2C8_EN                        (0x1UL << 21)
#define      MFPC5_I2C9_EN                        (0x1UL << 22)
#define      MFPC5_I2C10_EN                       (0x1UL << 23)
#define      MFPC5_I2C11_EN                       (0x1UL << 24)
#define      MFPC5_I2C12_EN                       (0x1UL << 25)
#define      MFPC5_I2C13_EN                       (0x1UL << 26)
#define      MFPC5_I2C14_EN                       (0x1UL << 27)
#define      MFPC5_USB11HCP34_EN                  (0x1UL << 28)
#define      MFPC5_USB20_MASK                     (0x1UL << 29)
#define      MFPC5_USB20(v)                       (((v) << 29) & MFPC5_USB20_MASK)
#define      MFPC5_USB20_VHUB                     (0x0)
#define      MFPC5_USB20_HC                       (0x1)


#define RSCU_MFPC6                                0x94        /*    Multi-function Pin Control#6*/
#define      MFPC6_DVI_SOURCE_MASK                (0x1UL << 2)
#define      MFPC6_DVI_SOURCE_SEL(v)              (((v) << 2) & MFPC6_DVI_SOURCE_MASK)
#define      MFPC6_DVI_SOURCE_SEL_GET(r)          (((r) & MFPC6_DVI_SOURCE_MASK) >> 2)
#define      MFPC6_DVI_SOURCE_PORT_A              (0x0)
#define      MFPC6_DVI_SOURCE_PORT_B              (0x1)
#define      MFPC6_DVOPIN_MASK                    (0x3UL)
#define      MFPC6_DVOPIN(v)                      (((v) << 0) & MFPC6_DVOPIN_MASK)
#define      MFPC6_DVOPIN_GET(r)                  (((r) & MFPC6_DVOPIN_MASK) >> 0)
#define      MFPC6_DVOPIN_DISABLE                 (0)
#define      MFPC6_DVOPIN_24BITS                  (0x1UL)
#define      MFPC6_DVOPIN_30BITS                  (0x2UL)
#define      MFPC6_I2SP0PINEN_MASK                (0x1UL << 3)
#define      MFPC6_I2SP0PINEN(v)                  (((v) << 3) & MFPC6_I2SP0PINEN_MASK)
#define      MFPC6_I2SP1PINEN_MASK                (0x1UL << 4)
#define      MFPC6_I2SP1PINEN(v)                  (((v) << 4) & MFPC6_I2SP1PINEN_MASK)
#define      MFPC6_USB20P2_MASK                   (0x3UL << 13)
#define      MFPC6_USB20P2(v)                     (((v) << 13) & MFPC6_USB20P2_MASK)
#define      MFPC6_USB20P2_USB11HID               (0x0)
#define      MFPC6_USB20P2_USB20DEV               (0x1)
#define      MFPC6_USB20P2_USB20HCP2              (0x2)

#define      MFPC6_I2SMCLKMSTRP0EN_MASK           (0x1UL << 18)
#define      MFPC6_I2SMCLKMSTRP0EN(v)             (((v) << 18) & MFPC6_I2SMCLKMSTRP0EN_MASK)
#define      MFPC6_I2SMCLKMSTRP1EN_MASK           (0x1UL << 19)
#define      MFPC6_I2SMCLKMSTRP1EN(v)             (((v) << 19) & MFPC6_I2SMCLKMSTRP1EN_MASK)

#define RSCU_WDTRESET                             0x9C        /*    Watchdog Reset Selection */

#define RSCU_MFPC7                                0xA0        /*    Multi-function Pin Control#7*/
#define      MFPC7_MAC1MII_OFF_MASK               0x0003F03F  /*    GPIOT[0:5], GPIOU[4:7], GPIOV[0:1] */
#define      MFPC7_MAC2MII_OFF_MASK               0x00FC0FC0  /*    GPIOT[6:7], GPIOU[0:3], GPIOV[3:7] */

#define RSCU_MFPC8                                0xA4        /*    Multi-function Pin Control#8*/
#define      MFPC8_VPOA_ALL_EN                    (0xFFFF << 16)

#define RSCU_MFPC9                                0xA8        /*    Multi-function Pin Control#9*/
#define      MFPC9_VPOA_ALL_EN                    (0x0F << 0)

#define RSCU_MACCLK_100M                          0xB8        /*    MAC interface clock delay 100M setting register */
#define RSCU_MACCLK_10M                           0xBC        /*    MAC interface clock delay 10M setting register */
#define RSCU_PWRSAVEN                             0xC0        /*    Power Saving Wakeup Enable*/
#define RSCU_PWRSAVCTRL                           0xC4        /*    Power Saving Wakeup Control*/
#define RSCU_HWSTRAP2                             0xD0        /*    Haardware strapping register set 2*/

#define RSCU_RESET2                               0xD4        /*    system reset control register set 2*/
#define      RESET2_CRT2_RESET                    (0x1UL << 5)

#define RSCU_CLKSEL_SET2                          0xD8        /*    clock selection register set 2 */
#define      INPUT_CLK_DELAY_CTL_MASK             (0x1UL << 30)
#define      INPUT_CLK_DELAY_CTL(v)               (((v) << 30) & INPUT_CLK_DELAY_CTL_MASK)
#define      INPUT_CLK_DELAY_INVERT_MASK          (0x1UL << 29)
#define      INPUT_CLK_DELAY_INVERT(v)            (((v) << 29) & INPUT_CLK_DELAY_INVERT_MASK)
#define      INPUT_CLK_DELAY_SEL_MASK             (0x1fUL << 24)
#define      INPUT_CLK_DELAY_SEL(v)               (((v) << 24) & INPUT_CLK_DELAY_SEL_MASK)
#define      OUTPUT_CLK_DELAY_INVERT_MASK         (0x1UL << 11)
#define      OUTPUT_CLK_DELAY_INVERT(v)           (((v) << 11) & OUTPUT_CLK_DELAY_INVERT_MASK)
#define      OUTPUT_CLK_DELAY_SEL_MASK            (0x1fUL << 6)
#define      OUTPUT_CLK_DELAY_SEL(v)              (((v) << 6)& OUTPUT_CLK_DELAY_SEL_MASK)
#define      OUTPUT_CLK_DELAY_SHIFT               (6)
#define      OUTPUT_CLK_DELAY_MASK                (0x3F << 6)

#define RSCU_CNTR4L                               0xE0        /*    SCU Free Run Counter Read Back #4*/
#define RSCU_CNTR4H                               0xE4        /*    SCU Free Run Counter Extended Read Back #4*/

#define RSCU_ULCKIP                               0xF0        /*    Unlock IP */
#define      ULCKIP_VEDCDE                        0x2000DEEA
#define      ULCKIP_I2S                           0x10ADDEED
#define      ULCKIP_I2S_ON                        (0x1UL << 0)
#define      ULCKIP_IR                            0x10ADDEED  /*    AST1520 IR and I2S share the same unlock */
#define      ULCKIP_IR_ON                         (0x1UL << 0)

#define RSCU_DPLLEXT0                             0x130
#define      DPLLEXT0_OFF                         (0x1UL << 0)
#define      DPLLEXT0_BYPASS                      (0x1UL << 1)
#define      DPLLEXT0_RESET                       (0x1UL << 2)
#define      DPLLEXT0_PARH_MASK                   (0x1FFUL << 5)
#define      DPLLEXT0_PARH(v)                     (((v) << 5) & DPLLEXT0_PARH_MASK)
#define RSCU_DPLLEXT1                             0x134
#define RSCU_DPLLEXT2                             0x138

#define RSCU_D2PLLEXT0                            0x13C
#define      D2PLLEXT0_OFF                        (0x1UL << 0)
#define      D2PLLEXT0_BYPASS                     (0x1UL << 1)
#define      D2PLLEXT0_RESET                      (0x1UL << 2)
#define      D2PLLEXT0_PARH_MASK                  (0x1FFUL << 5)
#define      D2PLLEXT0_PARH(v)                    (((v) << 5) & D2PLLEXT0_PARH_MASK)
#define RSCU_D2PLLEXT1                            0x140
#define RSCU_D2PLLEXT2                            0x144

#define RSCU_PCIECFCTL                            0x180       /*    PCI-E Configuration Setting Control Register */
#define RSCU_BMCMMIODEC                           0x184       /*    BMC MMIO Decode Setting Register */
#define RSCU_DECAREA1                             0x188       /*    1st relocated controller decode area location */
#define RSCU_DECAREA2                             0x18C       /*    2nd relocated controller decode area location */
#define RSCU_MBOXAREA                             0x190       /*    Mailbox decode area location*/
#define RSCU_SRMARA0                              0x194       /*    Shared SRAM area decode location*/
#define RSCU_SRMARA1                              0x198       /*    Shared SRAM area decode location*/
#define RSCU_BMCCLSID                             0x19C       /*    BMC device class code and revision ID */
#define RSCU_BMCDEVID                             0x1A4       /*    BMC device ID */

#define RSCU_PLLGENLCKCTRL                        0x1C0       /*    Video Extension PLL Gen-lock control */
#define      PLLGENLCKCTRL_VGENLCK_EN             (0x1UL << 0)
#define      PLLGENLCKCTRL_D1PLLLCK_EN            (0x1UL << 1)
#define      PLLGENLCKCTRL_D2PLLLCK_EN            (0x1UL << 2)
#define      PLLGENLCKCTRL_VGENLCK_FROM_MPLL      (0x1UL << 3)
#define      PLLGENLCKCTRL_SRC_RATIO_MASK         (0xFUL << 4)
#define      PLLGENLCKCTRL_SRC_RATIO(v)           (((v) << 4) & PLLGENLCKCTRL_SRC_RATIO_MASK)
#define      PLLGENLCKCTRL_AGENLCK_EN             (0x1UL << 8)
#define      PLLGENLCKCTRL_AGENLCK_H_SRC_MASK     (0x1UL << 9)
#define      PLLGENLCKCTRL_AGENLCK_H_SRC_D2PLL    (0x1UL << 9)
#define      PLLGENLCKCTRL_AGENLCK_H_SRC_D1PLL    (0x0UL << 9)
#define      PLLGENLCKCTRL_AGENLCK_SRC_MASK       (0x1UL << 10)
#define      PLLGENLCKCTRL_AGENLCK_SRC_I2SCLKO    (0x1UL << 10)
#define      PLLGENLCKCTRL_AGENLCK_SRC_D2PLL      (0x0UL << 10)

#define RSCU_VGENLCKCTRL                          0x1C4       /*    Video Extension PLL Gen-lock control */
#define      VGENLCKCTRL_F_MASK_A0                (0xFFFFFFUL << 0)
#define      VGENLCKCTRL_F_MASK_A1                (0x1FFFFFFUL << 0)
#define      VGENLCKCTRL_F_A0(v)                  (((v) << 0) & VGENLCKCTRL_F_MASK_A0)
#define      VGENLCKCTRL_F_A1(v)                  (((v) << 0) & VGENLCKCTRL_F_MASK_A1)

#define RSCU_AGENLCK_A                            0x1C8       /*    Audio Extension PLL Gen-lock Accumulation Value */
#define RSCU_AGENLCK_R                            0x1CC       /*    Audio Extension PLL Gen-lock Residual Value */
#define RSCU_AGENLCK_N                            0x1D0       /*    Audio Extension PLL Gen-lock Divider Value */
#define RSCU_AGENLCK_Q                            0x1D4       /*    Audio Extension PLL Gen-lock Quotient Value */

#define RSCU_CLK_DUTY_SELECTION                   0x1D8       /*    Clock Duty Selection */
#define      D1CLK_SEL_MASK                       0x7F
#define      D1CLK_SEL(v)                         (((v) << 0) & D1CLK_SEL_MASK)
#define      D2CLK_SEL_MASK                       (0x7F << 8)
#define      D2CLK_SEL(v)                         (((v) << 8) & D2CLK_SEL_MASK)
#define      D3CLK_SEL_MASK                       (0x7F << 16)
#define      D3CLK_SEL(v)                         (((v) << 16) & D3CLK_SEL_MASK)
#define      D4CLK_SEL_MASK                       (0x7F << 24)
#define      D4CLK_SEL(v)                         (((v) << 24) & D3CLK_SEL_MASK)

#define RSCU_CLK_DUTY_SELECTION_1                 0x1DC       /* Clock Duty Selection for RGMII */

#endif //#ifndef _REGS_SCU_AST1520_H_
#endif //#if (CONFIG_AST1500_SOC_VER == 3)
