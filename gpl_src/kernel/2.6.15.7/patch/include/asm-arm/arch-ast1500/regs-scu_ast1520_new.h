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

//#define __EXTRACT_REG(reg, mask, offset)      (((reg) & mask) >> offset)
//#define __CNSTRCT_REG(value, mask, offset)    (((value) << offset) & mask)

/*
 *  Register for SCU
 */
#define RSCU_PRTCT                              0x00        /*    protection key register    */
#define      PRTCT_UNLOCK                       0x1688A8A8

#define RSCU_RESET                              0x04        /*    system reset control register */
#define      RESET_CRT_MASK                     (0x1UL << 13)
#define      RESET_CRT(v)                       (((v) << 13) & RESET_CRT_MASK)
#define      RESET_VE_MASK                      (0x1UL << 6)
#define      RESET_VE(v)                        (((v) << 6) & RESET_VE_MASK)
#define RSCU_CLKSEL                              0x08        /*    clock selection register    */
#define      CLKSEL_PCLKD_MASK                  (0x7UL << 23)
#define      CLKSEL_PCLKD(v)                    (((v) << 23) & CLKSEL_PCLKD_MASK)
#define      CLKSEL_PCLKD_GET(r)                (((r) & CLKSEL_PCLKD_MASK) >> 23)
#define      CLKSEL_VPCLKDLYB321_MASK           (0x7UL << 8)
#define      CLKSEL_VPCLKDLYB321(v)             (((v) << 8) & CLKSEL_VPCLKDLYB321_MASK)
#define      CLKSEL_VPCLKDLYB321_GET(r)         (((r) & CLKSEL_VPCLKDLYB321_MASK) >> 8)
#define      CLKSEL_VPCLKDLYB321_0NS            (0x0)       /*    0000                           */
#define      CLKSEL_VPCLKDLYB321_0d5NS          (0x0)       /*    0001                           */
#define      CLKSEL_VPCLKDLYB321_1NS            (0x1)       /*    0010                           */
#define      CLKSEL_VPCLKDLYB321_1d5NS          (0x1)       /*    0011                           */
#define      CLKSEL_VPCLKDLYB321_2NS            (0x2)       /*    0100                           */
#define      CLKSEL_VPCLKDLYB321_2d5NS          (0x2)       /*    0101                           */
#define      CLKSEL_VPCLKDLYB321_3NS            (0x3)       /*    0110                           */
#define      CLKSEL_VPCLKDLYB321_3d5NS          (0x3)       /*    0111                           */
#define      CLKSEL_VPCLKDLYB321_I0NS           (0x4)       /*    1000                           */
#define      CLKSEL_VPCLKDLYB321_I0d5NS         (0x4)       /*    1001                           */
#define      CLKSEL_VPCLKDLYB321_I1NS           (0x5)       /*    1010                           */
#define      CLKSEL_VPCLKDLYB321_I1d5NS         (0x5)       /*    1011                           */
#define      CLKSEL_VPCLKDLYB321_I2NS           (0x6)       /*    1100                           */
#define      CLKSEL_VPCLKDLYB321_I2d5NS         (0x6)       /*    1101                           */
#define      CLKSEL_VPCLKDLYB321_I3NS           (0x7)       /*    1110                           */
#define      CLKSEL_VPCLKDLYB321_I3d5NS         (0x7)       /*    1111               */

#define RSCU_CLKSTOP                            0x0C        /*    clock stop control register    */
#define      CLKSTOP_CRT1CLKOFF_MASK        (0x1UL << 10)
#define      CLKSTOP_CRT1CLKOFF(v)          (((v) << 10) & CLKSTOP_CRT1CLKOFF_MASK)
#define      CLKSTOP_CRT1CLKOFF_GET(r)      (((r) & CLKSTOP_CRT1CLKOFF_MASK) >> 10)
#define      CLKSTOP_CRT1CLKOFF_OFF         (1)
#define      CLKSTOP_CRT1CLKOFF_ON          (0)
#define      CLKSTOP_VCLKOFF_MASK        (0x1UL << 3)
#define      CLKSTOP_VCLKOFF(v)          (((v) << 3) & CLKSTOP_VCLKOFF_MASK) //For video capture 1
#define      CLKSTOP_ECLKOFF_MASK        (0x1UL << 0)
#define      CLKSTOP_ECLKOFF(v)          (((v) << 0) & CLKSTOP_ECLKOFF_MASK) //For video engine
#define RSCU_FCNTCTRL                           0x10        /*    frequency counter control register    */
#define RSCU_FCNTVAL                            0x14        /*    frequency counter measure register     */
#define RSCU_D1PLL                              0x18        /*    D1-PLL Parameter register    */
#define RSCU_D2PLL                              0x1C        /*    D2-PLL Parameter register       */
#define RSCU_MPLL                               0x20        /*    M-PLL Parameter register        */
#define RSCU_HPLL                               0x24        /*    H-PLL Parameter register        */
#define      HPLL_FROMREG_EN                    (0x1UL << 18)
#define      HPLL_BYPASS_EN                     (0x1UL << 17)
#define      HPLL_HPLL_OFF                      (0x1UL << 16)
#define      HPLL_NMRTR_MASK                (0x3FUL << 5)
#define      HPLL_NMRTR(v)                  (((v) << 5) & HPLL_NMRTR_MASK)
#define      HPLL_NMRTR_GET(r)              (((r) & HPLL_NMRTR_MASK) >> 5)
#define      HPLL_OD_MASK                   (0x1UL << 4)
#define      HPLL_OD(v)                     (((v) << 4) & HPLL_OD_MASK)
#define      HPLL_OD_GET(r)                 (((r) & HPLL_OD_MASK) >> 4)
#define      HPLL_DNMRATR_MASK              (0xFUL << 0)
#define      HPLL_DNMRATR(v)                (((v) << 0) & HPLL_DNMRATR_MASK)
#define      HPLL_DNMRATR_GET(r)            (((r) & HPLL_DNMRATR_MASK) >> 0)
#define RSCU_FCNTCOM                        0x28        /*    frequency counter comparsion register */
#define RSCU_MISC1CTRL                          0x2C        /*    Misc. Control register */
#define      MISC1CTRL_CRT1CLKSEL_MASK      (0x1UL << 18)
#define      MISC1CTRL_CRT1CLKSEL(v)        (((v) << 18) & MISC1CTRL_CRT1CLKSEL_MASK)
#define      MISC1CTRL_CRT1CLKSEL_GET(r)    (((r) & MISC1CTRL_CRT1CLKSEL_MASK) >> 18)
#define      MISC1CTRL_CRT1CLKSEL_D1PLL     (0)
#define      MISC1CTRL_CRT1CLKSEL_D2PLL     (1)
#define      MISC1CTRL_DACSRCSEL_MASK       (0x3UL << 16)
#define      MISC1CTRL_DACSRCSEL(v)         (((v) << 16) & MISC1CTRL_DACSRCSEL_MASK)
#define      MISC1CTRL_DACSRCSEL_GET(r)     (((r) & MISC1CTRL_DACSRCSEL_MASK) >> 16)
#define      MISC1CTRL_DACSRCSEL_VGA       (0)
#define      MISC1CTRL_DACSRCSEL_CRT2       (0x1)
#define      MISC1CTRL_DACSRCSEL_PSSTHRGH        (0x2)
#define      MISC1CTRL_VPCLKDLYB0_MASK          (0x1UL << 9)
#define      MISC1CTRL_VPCLKDLYB0(v)            (((v) << 9) & MISC1CTRL_VPCLKDLYB0_MASK)
#define      MISC1CTRL_VPCLKDLYB0_GET(r)        (((r) & MISC1CTRL_VPCLKDLYB0_MASK) >> 9)
#define      MISC1CTRL_VPCLKDLYB0_0NS           (0x0)       /*    0000                           */
#define      MISC1CTRL_VPCLKDLYB0_0d5NS         (0x1)       /*    0001                           */
#define      MISC1CTRL_VPCLKDLYB0_1NS           (0x0)       /*    0010                           */
#define      MISC1CTRL_VPCLKDLYB0_1d5NS         (0x1)       /*    0011                           */
#define      MISC1CTRL_VPCLKDLYB0_2NS           (0x0)       /*    0100                           */
#define      MISC1CTRL_VPCLKDLYB0_2d5NS         (0x1)       /*    0101                           */
#define      MISC1CTRL_VPCLKDLYB0_3NS           (0x0)       /*    0110                           */
#define      MISC1CTRL_VPCLKDLYB0_3d5NS         (0x1)       /*    0111                           */
#define      MISC1CTRL_VPCLKDLYB0_I0NS          (0x0)       /*    1000                           */
#define      MISC1CTRL_VPCLKDLYB0_I0d5NS        (0x1)       /*    1001                           */
#define      MISC1CTRL_VPCLKDLYB0_I1NS          (0x0)       /*    1010                           */
#define      MISC1CTRL_VPCLKDLYB0_I1d5NS        (0x1)       /*    1011                           */
#define      MISC1CTRL_VPCLKDLYB0_I2NS          (0x0)       /*    1100                           */
#define      MISC1CTRL_VPCLKDLYB0_I2d5NS        (0x1)       /*    1101                           */
#define      MISC1CTRL_VPCLKDLYB0_I3NS          (0x0)       /*    1110                           */
#define      MISC1CTRL_VPCLKDLYB0_I3d5NS        (0x1)       /*    1111                           */
#define      MISC1CTRL_D2PLLOFF_MASK        (0x1UL << 4)
#define      MISC1CTRL_D2PLLOFF(v)          (((v) << 4) & MISC1CTRL_D2PLLOFF_MASK)
#define      MISC1CTRL_D2PLLOFF_GET(r)      (((r) & MISC1CTRL_D2PLLOFF_MASK) >> 4)
#define      MISC1CTRL_D2PLLOFF_OFF         (1)
#define      MISC1CTRL_D2PLLOFF_ON          (0)
#define      MISC1CTRL_DACOFF_MASK          (0x1UL << 3)
#define      MISC1CTRL_DACOFF(v)            (((v) << 3) & MISC1CTRL_DACOFF_MASK)
#define      MISC1CTRL_DACOFF_GET(r)        (((r) & MISC1CTRL_DACOFF_MASK) >> 3)
#define      MISC1CTRL_DACOFF_OFF           (1)
#define      MISC1CTRL_DACOFF_ON            (0)
#define      MISC1CTRL_D1PLLOFF_MASK        (0x1UL << 2)
#define      MISC1CTRL_D1PLLOFF(v)          (((v) << 2) & MISC1CTRL_D1PLLOFF_MASK)
#define      MISC1CTRL_D1PLLOFF_GET(r)      (((r) & MISC1CTRL_D1PLLOFF_MASK) >> 2)
#define      MISC1CTRL_D1PLLOFF_OFF         (1)
#define      MISC1CTRL_D1PLLOFF_ON          (0)

#define RSCU_PCICONF1                       0x30        /*    PCI configuration setting register#1 */
#define RSCU_PCICONF2                       0x34        /*    PCI configuration setting register#2 */
#define RSCU_PCICONF3                       0x38        /*    PCI configuration setting register#3 */
#define RSCU_RESETSTAT                      0x3C        /*    System reset contrl/status register*/
#define RSCU_SOCSCRATCH0                    0x40        /*    SOC scratch 0~31 register */
#define RSCU_SOCSCRATCH1                    0x44        /*    SOC scratch 32~63 register */
#define RSCU_MACCLK                         0x48        /*    MAC interface clock delay setting register */
#define RSCU_MISC2CTRL                      0x4C        /*    Misc. 2 Control register */

/* SCU50 ~ SCU6C: VGA Scratch Register */
#define RSCU_VGASCRATCH0                    0x50
#define RSCU_VGASCRATCH1                    0x54
#define RSCU_VGASCRATCH2                    0x58
#define RSCU_VGASCRATCH3                    0x5C
#define RSCU_VGASCRATCH4                    0x60
#define RSCU_VGASCRATCH5                    0x64
#define RSCU_VGASCRATCH6                    0x68
#define RSCU_VGASCRATCH7                    0x6C

#define RSCU_HWSTRAP1                       0x70        /*    SCU70: Hardware Trapping Register */
#define      HWSTRAP1_HPLLSLT_MASK          (0x3UL << 8)
#define      HWSTRAP1_HPLLSLT_GET(r)        (((r) & HWSTRAP1_HPLLSLT_MASK) >> 8)
#define      HWSTRAP1_HPLLSLT_384MHZ        (0x0)
#define      HWSTRAP1_HPLLSLT_360MHZ        (0x1)
#define      HWSTRAP1_HPLLSLT_336MHZ        (0x2)
#define      HWSTRAP1_HPLLSLT_408MHZ        (0x3)
#define RSCU_RAMDOMGEN                      0x74        /*    random number generator register */
#define RSCU_RAMDOMDATA                     0x78        /*    random number generator data output*/
#define RSCU_REVISION                       0x7C        /*    Silicon revision ID register */
#define RSCU_MFPC1                          0x80        /*    Multi-function Pin Control#1*/
#define RSCU_MFPC2                          0x84        /*    Multi-function Pin Control#2*/
#define RSCU_MFPC3                          0x88        /*    Multi-function Pin Control#3*/
#define RSCU_MFPC4                          0x8C        /*    Multi-function Pin Control#4*/
#define RSCU_MFPC5                          0x90        /*    Multi-function Pin Control#5*/
#define      MFPC5_DVIPINS_MASK             (0x3UL << 4)
#define      MFPC5_DVIPINS(v)               (((v) << 4) & MFPC5_DVIPINS_MASK)
#define      MFPC5_DVIPINS_DISABLE          (0x0)
#define      MFPC5_DVIPINS_PB24BITS         (0x1)
#define      MFPC5_DVIPINS_PA24BITS         (0x2)
#define      MFPC5_DVIPINS_PA30BITS         (0x3)

#define RSCU_MFPC6                          0x94        /*    Multi-function Pin Control#6*/
#define      MFPC6_DVOPIN_MASK              (0x3UL)
#define      MFPC6_DVOPIN_DISABLE           (0)
#define      MFPC6_DVOPIN_24BITS            (0x1UL)
#define      MFPC6_DVOPIN_30BITS            (0x2UL)
#define      MFPC6_DVOPIN(v)                (((v) << 0) & MFPC6_DVOPIN_MASK)
#define      MFPC6_DVOPIN_GET(r)            (((r) & MFPC6_DVOPIN_MASK) >> 0)

#define RSCU_WDTRESET                       0x9C        /*    Watchdog Reset Selection */
#define RSCU_MFPC8                          0xA4        /*    Multi-function Pin Control#8*/
#define RSCU_MFPC9                          0xA8        /*    Multi-function Pin Control#9*/
#define RSCU_PWRSAVEN                       0xC0        /*    Power Saving Wakeup Enable*/
#define RSCU_PWRSAVCTRL                     0xC4        /*    Power Saving Wakeup Control*/
#define RSCU_HWSTRAP2                       0xD0        /*    Haardware strapping register set 2*/
#define RSCU_CNTR4L                         0xE0        /*    SCU Free Run Counter Read Back #4*/
#define RSCU_CNTR4H                         0xE4        /*    SCU Free Run Counter Extended Read Back #4*/
#define RSCU_ULCKIP                         0xF0        /*    Unlock IP */
#define      ULCKIP_VEDCDE                  0x2000DEEA

//
#define RSCU_PCIECFCTL                      0x180       /*    PCI-E Configuration Setting Control Register */
#define RSCU_BMCMMIODEC                     0x184       /*    BMC MMIO Decode Setting Register */
#define RSCU_DECAREA1                       0x188       /*    1st relocated controller decode area location */
#define RSCU_DECAREA2                       0x18C       /*    2nd relocated controller decode area location */
#define RSCU_MBOXAREA                       0x190       /*    Mailbox decode area location*/
#define RSCU_SRMARA0                        0x194       /*    Shared SRAM area decode location*/
#define RSCU_SRMARA1                        0x198       /*    Shared SRAM area decode location*/
#define RSCU_BMCCLSID                       0x19C       /*    BMC device class code and revision ID */
#define RSCU_BMCDEVID                       0x1A4       /*    BMC device ID */




#endif //#ifndef _REGS_SCU_AST1520_H_

#endif //#if (CONFIG_AST1500_SOC_VER == 3)
