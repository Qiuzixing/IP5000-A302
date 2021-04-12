/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ******************************************************************************
 * ASPEED Technology Inc.
 * AST2500 DDR3/DDR4 SDRAM controller initialization sequence
 *
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * Version     : 1
 * Release date: 2014.09.09
 *
 * Change List :
 *  2014.07.25 : 1. Modify HPLL config sequence
 *  2014.07.30 : 1. Modify DDR3 AC parameters table
 *               2. Turn on ZQCS mode
 *  2014.08.13 : 1. Add disable XDMA
 *  2014.09.09 : 1. Disable CKE dynamic power down
 *
 * Optional define variable
 * 1. DRAM Speed             //
 *    CONFIG_DRAM_1333       //
 *    CONFIG_DRAM_1600       // (default)
 *    CONFIG_DRAM_1866       //
 *    CONFIG_DRAM_2133       //
 * 2. ECC Function enable
 *    CONFIG_DRAM_ECC        // define to enable ECC function
 *    CONFIG_DRAM_ECC_SIZE   // define the ECC protected memory size
 * 3. UART5 message output   //
 *    CONFIG_DRAM_UART_38400 // set the UART baud rate to 38400, default is 115200
 * 4. DRAM Type
 *    CONFIG_DDR3_8GSTACK    // DDR3 8Gbit Stack die
 *    CONFIG_DDR4_4GX8       // DDR4 4Gbit X8 dual part
 * 5. Firmware 2nd boot flash
 *    CONFIG_FIRMWARE_2ND_BOOT
 ******************************************************************************
 */

#include <config.h>
#include <version.h>

/******************************************************************************
  r4 : return program counter
  r5 : DDR speed timing table base address
  Free registers:
  r0, r1, r2, r3, r6, r7, r8, r9, r10, r11
 ******************************************************************************/
#define ASTMMC_INIT_VER      0x01                @ 8bit verison number
#define ASTMMC_INIT_DATE     0x20140909          @ Release date

#define ASTMMC_REGIDX_010    0x00
#define ASTMMC_REGIDX_014    0x04
#define ASTMMC_REGIDX_018    0x08
#define ASTMMC_REGIDX_020    0x0C
#define ASTMMC_REGIDX_024    0x10
#define ASTMMC_REGIDX_02C    0x14
#define ASTMMC_REGIDX_030    0x18
#define ASTMMC_REGIDX_214    0x1C
#define ASTMMC_REGIDX_2E0    0x20
#define ASTMMC_REGIDX_2E4    0x24
#define ASTMMC_REGIDX_2E8    0x28
#define ASTMMC_REGIDX_2EC    0x2C
#define ASTMMC_REGIDX_2F0    0x30
#define ASTMMC_REGIDX_2F4    0x34
#define ASTMMC_REGIDX_2F8    0x38
#define ASTMMC_REGIDX_RFC    0x3C
#define ASTMMC_REGIDX_PLL    0x40

TIME_TABLE_DDR3_1333:
    .word   0x53503C37       @ 0x010
    .word   0xFF58D47F       @ 0x014
    .word   0x00010000       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02101C60       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000020       @ 0x214
    .word   0x02001000       @ 0x2E0
    .word   0x0C000085       @ 0x2E4
    .word   0x000BA018       @ 0x2E8
    .word   0x1FFFF104       @ 0x2EC
    .word   0x07090407       @ 0x2F0
    .word   0x81000700       @ 0x2F4
    .word   0x0C400800       @ 0x2F8
    .word   0x7F5E3A27       @ tRFC
    .word   0x00005B80       @ PLL
TIME_TABLE_DDR3_1600:
    .word   0x64604D38       @ 0x010
    .word   0x2F690599       @ 0x014
    .word   0x00000300       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02181E70       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000024       @ 0x214
    .word   0x02001300       @ 0x2E0
    .word   0x0E0000A0       @ 0x2E4
    .word   0x000E001B       @ 0x2E8
    .word   0x1FFFF105       @ 0x2EC
    .word   0x08090408       @ 0x2F0
    .word   0x9B000800       @ 0x2F4
    .word   0x0E400A00       @ 0x2F8
    .word   0x9971452F       @ tRFC
    .word   0x000071C1       @ PLL
TIME_TABLE_DDR3_1866:
    .word   0x74704F49       @ 0x010
    .word   0x2F7B25B3       @ 0x014
    .word   0x00010300       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02201014       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001600       @ 0x2E0
    .word   0x100000BA       @ 0x2E4
    .word   0x0010501E       @ 0x2E8
    .word   0x1FFFF106       @ 0x2EC
    .word   0x090A0408       @ 0x2F0
    .word   0xB5000A00       @ 0x2F4
    .word   0x10400B00       @ 0x2F8
    .word   0xB3845137       @ tRFC
    .word   0x000066A0       @ PLL
TIME_TABLE_DDR3_2133:
    .word   0x8480504A       @ 0x010
    .word   0x5F8C46CC       @ 0x014
    .word   0x00000304       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02281024       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001900       @ 0x2E0
    .word   0x120000D5       @ 0x2E4
    .word   0x0012B021       @ 0x2E8
    .word   0x1FFFF107       @ 0x2EC
    .word   0x0A0A0409       @ 0x2F0
    .word   0xCF000B00       @ 0x2F4
    .word   0x12400D00       @ 0x2F8
    .word   0xCC975D3F       @ tRFC
    .word   0x000066A0       @ PLL

TIME_TABLE_DDR4_1333:
    .word   0x53503D26       @ 0x010
    .word   0xDF78D83A       @ 0x014
    .word   0x00019000       @ 0x018
    .word   0x04000000       @ 0x020
    .word   0x00000400       @ 0x024
    .word   0x00000200       @ 0x02C
    .word   0x00000303       @ 0x030
    .word   0x00000020       @ 0x214
    .word   0x03002200       @ 0x2E0
    .word   0x0C000085       @ 0x2E4
    .word   0x000BA01A       @ 0x2E8
    .word   0x1FFFF106       @ 0x2EC
    .word   0x07060507       @ 0x2F0
    .word   0x81000700       @ 0x2F4
    .word   0x0C400800       @ 0x2F8
    .word   0x7F5E3A3A       @ tRFC
    .word   0x00005B80       @ PLL

TIME_TABLE_DDR4_1600:
    .word   0x63604F27       @ 0x010
    .word   0xFF9B0A45       @ 0x014
    .word   0x00019100       @ 0x018
    .word   0x04000000       @ 0x020
    .word   0x00000400       @ 0x024
    .word   0x00100414       @ 0x02C
    .word   0x00000303       @ 0x030
    .word   0x00000024       @ 0x214
    .word   0x03002900       @ 0x2E0
    .word   0x0F0000A0       @ 0x2E4
    .word   0x000E001E       @ 0x2E8
    .word   0x1FFFF108       @ 0x2EC
    .word   0x08070507       @ 0x2F0
    .word   0x9B000A00       @ 0x2F4
    .word   0x0F400A00       @ 0x2F8
    .word   0x99714545       @ tRFC
    .word   0x000071C1       @ PLL

TIME_TABLE_DDR4_1866:
    .word   0x73704038       @ 0x010
    .word   0x2FAD2C51       @ 0x014
    .word   0x00009304       @ 0x018
    .word   0x04000000       @ 0x020
    .word   0x00000400       @ 0x024
    .word   0x00180624       @ 0x02C
    .word   0x00000303       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x03003000       @ 0x2E0
    .word   0x110000BA       @ 0x2E4
    .word   0x00105021       @ 0x2E8
    .word   0x1FFFF109       @ 0x2EC
    .word   0x09080507       @ 0x2F0
    .word   0xB5000C00       @ 0x2F4
    .word   0x11400B00       @ 0x2F8
    .word   0xB3845151       @ tRFC
    .word   0x000066A0       @ PLL

TIME_TABLE_DDR4_2133:
    .word   0x84805239       @ 0x010
    .word   0x5FCF5D5D       @ 0x014
    .word   0x0000D304       @ 0x018
    .word   0x08000000       @ 0x020
    .word   0x00000400       @ 0x024
    .word   0x00200834       @ 0x02C
    .word   0x00000303       @ 0x030
    .word   0x0000002C       @ 0x214
    .word   0x03003600       @ 0x2E0
    .word   0x130000D5       @ 0x2E4
    .word   0x0012B025       @ 0x2E8
    .word   0x1FFFF10B       @ 0x2EC
    .word   0x0A080608       @ 0x2F0
    .word   0xCF000E00       @ 0x2F4
    .word   0x13400D00       @ 0x2F8
    .word   0xCC975D5D       @ tRFC
    .word   0x000066A0       @ PLL

    .macro init_delay_timer
    ldr   r0, =0x1e782024                        @ Set Timer3 Reload
    str   r2, [r0]

    ldr   r0, =0x1e6c0038                        @ Clear Timer3 ISR
    ldr   r1, =0x00040000
    str   r1, [r0]

    ldr   r0, =0x1e782030                        @ Enable Timer3
    mov   r2, #7
    mov   r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6c0090                        @ Check ISR for Timer3 timeout
    .endm

    .macro check_delay_timer
    ldr   r1, [r0]
    bic   r1, r1, #0xFFFBFFFF
    mov   r2, r1, lsr #18
    cmp   r2, #0x01
    .endm

    .macro clear_delay_timer
    ldr   r0, =0x1e78203C                        @ Disable Timer3
    mov   r2, #0xF
    mov   r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6c0038                        @ Clear Timer3 ISR
    ldr   r1, =0x00040000
    str   r1, [r0]
    .endm

    .macro init_spi_checksum
    ldr   r0, =0x1e620084
    ldr   r1, =0x20010000
    str   r1, [r0]
    ldr   r0, =0x1e62008C
    ldr   r1, =0x20000200
    str   r1, [r0]
    ldr   r0, =0x1e620080
    ldr   r1, =0x0000000D
    orr   r2, r2, r7
    orr   r1, r1, r2, lsl #8
    and   r2, r6, #0xF
    orr   r1, r1, r2, lsl #4
    str   r1, [r0]
    ldr   r0, =0x1e620008
    ldr   r2, =0x00000800
    .endm

    .macro print_hex_char
    and   r1, r1, #0xF
    cmp   r1, #9
    addgt r1, r1, #0x37
    addle r1, r1, #0x30
    str   r1, [r0]
    .endm

/******************************************************************************
 Calibration Macro End
 ******************************************************************************/

.globl lowlevel_init
lowlevel_init:

init_dram:
    /* save lr */
    mov   r4, lr

    /* Check Scratch to determine the HPLL ever been set or not */
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    tst   r1, #0x80
    bne   bypass_config_hpll

    /* AST2500-A0 patch for HPLL issue */
    /* Set H-PLL */
    ldr   r0, =0x1e6e2000
    ldr   r1, =0x1688a8a8
    str   r1, [r0]
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    orr   r1, r1, #0x80
    str   r1, [r0]
    ldr   r0, =0x1e6e2148
    ldr   r1, =0x00190032
    str   r1, [r0]
    ldr   r0, =0x1e6e2024
    ldr   r2, =0x00380000
/*  ldr   r1, =0xABC02620 */                     @ set HPLL =  600 MHz
    ldr   r1, =0x93000400                        @ set HPLL =  792 MHz
/*  ldr   r1, =0x930004A0 */                     @ set HPLL =  912 MHz
/*  ldr   r1, =0xABC00520 */                     @ set HPLL = 1008 MHz
/*  ldr   r1, =0xABC00560 */                     @ set HPLL = 1056 MHz
    orr   r2, r1, r2
    str   r2, [r0]
    ldr   r2, =0x00000100                        @ delay 1 us
wait_hpll_pd:
    subs  r2, r2, #0x01
    bne   wait_hpll_pd
    str   r1, [r0]

/*  ldr   r0, =0x1e6e2070                        @ set AXI/AHB = 3/1, when HPLL >= 1000
    ldr   r1, =0x00000400
    str   r1, [r0]
    ldr   r0, =0x1e6e207c
    ldr   r1, =0x00000200
    str   r1, [r0]
*/
    ldr   r0, =0x1e6e2130
    ldr   r1, =0x00000580
    str   r1, [r0]
    ldr   r0, =0x1e6e2160                        @ set 24M Jitter divider
    ldr   r1, =0x00000B01
    str   r1, [r0]
    ldr   r0, =0x1e6e200c                        @ enable UART clock
    ldr   r1, [r0]
    ldr   r2, =0xF9FE7FFF
    and   r1, r1, r2
    str   r1, [r0]

    ldr   r0, =0x1e6e2180                        @ disable XDMA
    ldr   r1, =0x000C003B
    str   r1, [r0]

    /* Enable Watchdog */
    ldr   r0, =0x1e785004
    ldr   r1, =0x00000BB8                        @ Set to 3 ms
    str   r1, [r0]
    add   r0, r0, #0x04
    ldr   r1, =0x00004755
    str   r1, [r0]
    add   r0, r0, #0x04
    mov   r1, #0x13
    str   r1, [r0]

    ldr   r0, =0x1e78018c                        @ control GPIOZ3 to disable ENTEST rework
    ldr   r1, =0x00000800
    str   r1, [r0]
    ldr   r0, =0x1e7801e4
    ldr   r1, =0x00000800
    str   r1, [r0]
    /* AST2500-A0 patch for HPLL issue - end */

wait_hpll_init:
    b     wait_hpll_init

bypass_config_hpll:
    /* Enable Timer separate clear mode */
    ldr   r0, =0x1e782038
    mov   r1, #0xAE
    str   r1, [r0]

/* Test - DRAM initial time */
    ldr   r0, =0x1e78203c
    ldr   r1, =0x0000F000
    str   r1, [r0]

    ldr   r0, =0x1e782044
    ldr   r1, =0xFFFFFFFF
    str   r1, [r0]

    ldr   r0, =0x1e782030
    mov   r2, #3
    mov   r1, r2, lsl #12
    str   r1, [r0]
/* Test - DRAM initial time */

    /*Set Scratch register Bit 7 before initialize*/
    ldr   r0, =0x1e6e2000
    ldr   r1, =0x1688a8a8
    str   r1, [r0]

    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    orr   r1, r1, #0x80
    str   r1, [r0]

    /* Enable AXI_P */
    ldr   r0, =0x00000016
    mrc   p15, 0, r1, c15, c2, 4
    mcr   p15, 0, r0, c15, c2, 4

/******************************************************************************
 Disable WDT2 for 2nd boot function
 ******************************************************************************/
#ifndef CONFIG_FIRMWARE_2ND_BOOT
    ldr   r0, =0x1e78502c
    mov   r1, #0
    str   r1, [r0]
#endif

/******************************************************************************
 Disable WDT3 for SPI Address mode (3 or 4 bytes) detection function
 ******************************************************************************/
    ldr   r0, =0x1e785040
    ldr   r1, [r0]
    ldr   r0, =0x1e720010
    str   r1, [r0]
    ldr   r0, =0x1e78504c
    mov   r1, #0
    str   r1, [r0]

    /* Check Scratch Register Bit 6 */
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    bic   r1, r1, #0xFFFFFFBF
    mov   r2, r1, lsr #6
    cmp   r2, #0x01
    beq   platform_exit

    /* Set M-PLL */
    ldr   r2, =0x63002400                        @ load PLL parameter for 24Mhz CLKIN (396)
#ifdef CONFIG_DRAM_1333
    ldr   r0, =0x1e6e2148
    ldr   r1, [r0]
    bic   r1, r1, #0xFF
    orr   r1, r1, #0x38
    str   r1, [r0]
    ldr   r2, =0x930066C0                        @ load PLL parameter for 24Mhz CLKIN (330)
#endif
#ifdef CONFIG_DRAM_1866
    ldr   r2, =0x630024C0                        @ load PLL parameter for 24Mhz CLKIN (468)
#endif
#ifdef CONFIG_DRAM_2133
    ldr   r0, =0x1e6e2148
    ldr   r1, [r0]
    bic   r1, r1, #0xFF
    orr   r1, r1, #0x35
    str   r1, [r0]
    ldr   r2, =0x7B002560                        @ load PLL parameter for 24Mhz CLKIN (528)
#endif

    ldr   r0, =0x1e6e2070                        @ Check CLKIN freq
    ldr   r1, [r0]
    mov   r1, r1, lsr #23
    tst   r1, #0x01
    beq   set_MPLL
    ldr   r2, =0x630023E0                        @ load PLL parameter for 25Mhz CLKIN (400)
#ifdef CONFIG_DRAM_1333
    ldr   r0, =0x1e6e2148
    ldr   r1, [r0]
    bic   r1, r1, #0xFF
    orr   r1, r1, #0x38
    str   r1, [r0]
    ldr   r2, =0x93006680                        @ load PLL parameter for 25Mhz CLKIN (331)
#endif
#ifdef CONFIG_DRAM_1866
    ldr   r2, =0x63002480                        @ load PLL parameter for 25Mhz CLKIN (462)
#endif
#ifdef CONFIG_DRAM_2133
    ldr   r0, =0x1e6e2148
    ldr   r1, [r0]
    bic   r1, r1, #0xFF
    orr   r1, r1, #0x35
    str   r1, [r0]
    ldr   r2, =0x7B002520                        @ load PLL parameter for 25Mhz CLKIN (525)
#endif

set_MPLL:
    ldr   r0, =0x1e6e2020                        @ M-PLL (DDR SDRAM) Frequency
    str   r2, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e78400c
    mov   r1, #0x83
    str   r1, [r0]

    ldr   r0, =0x1e6e202c
    ldr   r2, [r0]
    mov   r2, r2, lsr #12
    tst   r2, #0x01
    ldr   r0, =0x1e784000
    moveq r1, #0x0D                              @ Baudrate 115200
    movne r1, #0x01                              @ Baudrate 115200, div13
#ifdef CONFIG_DRAM_UART_38400
    moveq r1, #0x27                              @ Baudrate 38400
    movne r1, #0x03                              @ Baudrate 38400 , div13
#endif
    str   r1, [r0]

    ldr   r0, =0x1e784004
    mov   r1, #0x00
    str   r1, [r0]

    ldr   r0, =0x1e78400c
    mov   r1, #0x03
    str   r1, [r0]

    ldr   r0, =0x1e784008
    mov   r1, #0x07
    str   r1, [r0]

    ldr   r0, =0x1e784000
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
    mov   r1, #0x41                              @ 'A'
    str   r1, [r0]
    mov   r1, #0x4D                              @ 'M'
    str   r1, [r0]
    mov   r1, #0x20                              @ ' '
    str   r1, [r0]
    mov   r1, #0x49                              @ 'I'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x69                              @ 'i'
    str   r1, [r0]
    mov   r1, #0x74                              @ 't'
    str   r1, [r0]
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, #0x56                              @ 'V'
    str   r1, [r0]
    mov   r1, #ASTMMC_INIT_VER
    mov   r1, r1, lsr #4
    print_hex_char
    mov   r1, #ASTMMC_INIT_VER
    print_hex_char
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
/* Debug - UART console message */

    clear_delay_timer

    /* Delay about 3ms */
    ldr   r2, =0x00000BB8                        @ Set Timer3 Reload = 3 ms
    init_delay_timer
wait_mpll_init:
    check_delay_timer
    bne   wait_mpll_init
    clear_delay_timer
    /* end delay 3ms */

/******************************************************************************
 Init DRAM common registers
 ******************************************************************************/
    ldr   r0, =0x1e6e0000
    ldr   r1, =0xFC600309
    str   r1, [r0]

    /* Reset MMC */
    ldr   r1, =0x00000000
    ldr   r0, =0x1e6e0004
    ldr   r2, =0x1e6e0090
reset_mmc:
    str   r1, [r0]
    add   r0, r0, #4
    cmp   r2, r0
    bge   reset_mmc

    ldr   r0, =0x1e6e0034                        @ disable SDRAM reset
    ldr   r1, =0x00000080
    str   r1, [r0]

    ldr   r0, =0x1e6e0008
    ldr   r1, =0x2001000F                        /* VGA */
    str   r1, [r0]

    ldr   r0, =0x1e6e0038
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e6e003c
    str   r1, [r0]

    ldr   r0, =0x1e6e0040
    ldr   r1, =0xF4444444
    str   r1, [r0]

    ldr   r0, =0x1e6e0044
    ldr   r1, =0x244222FF
    str   r1, [r0]

    ldr   r0, =0x1e6e0048
    ldr   r1, =0x22222222
    str   r1, [r0]

    ldr   r0, =0x1e6e004c
    ldr   r1, =0x22222222
    str   r1, [r0]

    ldr   r0, =0x1e6e0050
    ldr   r1, =0x80000000
    str   r1, [r0]

    ldr   r1, =0x00000000
    ldr   r0, =0x1e6e0054
    str   r1, [r0]
    ldr   r0, =0x1e6e0070
    str   r1, [r0]
    ldr   r0, =0x1e6e0074
    str   r1, [r0]
    ldr   r0, =0x1e6e0078
    str   r1, [r0]
    ldr   r0, =0x1e6e007c
    str   r1, [r0]

    ldr   r0, =0x1e6e0208                        @ PHY Setting
    str   r1, [r0]
    ldr   r0, =0x1e6e0218
    str   r1, [r0]
    ldr   r0, =0x1e6e0220
    str   r1, [r0]
    ldr   r0, =0x1e6e0228
    str   r1, [r0]
    ldr   r0, =0x1e6e0230
    str   r1, [r0]
    ldr   r0, =0x1e6e02a8
    str   r1, [r0]
    ldr   r0, =0x1e6e02b0
    str   r1, [r0]

    ldr   r0, =0x1e6e0240
    ldr   r1, =0x86000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0244
    ldr   r1, =0x00008600
    str   r1, [r0]

    ldr   r0, =0x1e6e0248
    ldr   r1, =0x80000000
    str   r1, [r0]

    ldr   r0, =0x1e6e024c
    ldr   r1, =0x80808080
    str   r1, [r0]

    /* Check DRAM Type by H/W Trapping */
    ldr   r0, =0x1e6e2070
    ldr   r1, [r0]
    ldr   r2, =0x01000000                        @ bit[24]=1 => DDR4
    tst   r1, r2
    bne   ddr4_init
    b     ddr3_init
.LTORG

/******************************************************************************
 DDR3 Init
 ******************************************************************************/
ddr3_init:
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x33                              @ '3'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
/* Debug - UART console message */

    adrl  r5, TIME_TABLE_DDR3_1600               @ init DRAM parameter table
#ifdef CONFIG_DRAM_1333
    adrl  r5, TIME_TABLE_DDR3_1333
#endif
#ifdef CONFIG_DRAM_1866
    adrl  r5, TIME_TABLE_DDR3_1866
#endif
#ifdef CONFIG_DRAM_2133
    adrl  r5, TIME_TABLE_DDR3_2133
#endif

    ldr   r0, =0x1e6e0004
#ifdef CONFIG_DDR3_8GSTACK
    ldr   r1, =0x00000323                        @ Init to 8GB stack
#else
    ldr   r1, =0x00000303                        @ Init to 8GB
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e0010
    ldr   r1, [r5, #ASTMMC_REGIDX_010]
    str   r1, [r0]

    ldr   r0, =0x1e6e0014
    ldr   r1, [r5, #ASTMMC_REGIDX_014]
    str   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, [r5, #ASTMMC_REGIDX_018]
    str   r1, [r0]

    /* DRAM Mode Register Setting */
    ldr   r0, =0x1e6e0020                        @ MRS_4/6
    ldr   r1, [r5, #ASTMMC_REGIDX_020]
    str   r1, [r0]

    ldr   r0, =0x1e6e0024                        @ MRS_5
    ldr   r1, [r5, #ASTMMC_REGIDX_024]
    str   r1, [r0]

    ldr   r0, =0x1e6e002c                        @ MRS_0/2
    ldr   r1, [r5, #ASTMMC_REGIDX_02C]
    mov   r2, #0x1
    orr   r1, r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0030                        @ MRS_1/3
    ldr   r1, [r5, #ASTMMC_REGIDX_030]
    str   r1, [r0]

    /* Start DDR PHY Setting */
    ldr   r0, =0x1e6e0200
    ldr   r1, =0x02492AAE
    str   r1, [r0]

    ldr   r0, =0x1e6e0204
#ifdef CONFIG_DDR3_8GSTACK
    ldr   r1, =0x10001001
#else
    ldr   r1, =0x00001001
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e020c
    ldr   r1, =0x55E00A0A
    str   r1, [r0]

    ldr   r0, =0x1e6e0210
    ldr   r1, =0x20000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0214
    ldr   r1, [r5, #ASTMMC_REGIDX_214]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e0
    ldr   r1, [r5, #ASTMMC_REGIDX_2E0]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e4
    ldr   r1, [r5, #ASTMMC_REGIDX_2E4]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e8
    ldr   r1, [r5, #ASTMMC_REGIDX_2E8]
    str   r1, [r0]

    ldr   r0, =0x1e6e02ec
    ldr   r1, [r5, #ASTMMC_REGIDX_2EC]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f0
    ldr   r1, [r5, #ASTMMC_REGIDX_2F0]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f4
    ldr   r1, [r5, #ASTMMC_REGIDX_2F4]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f8
    ldr   r1, [r5, #ASTMMC_REGIDX_2F8]
    str   r1, [r0]

    ldr   r0, =0x1e6e0290
    ldr   r1, =0x00100008
    str   r1, [r0]

    ldr   r0, =0x1e6e02c0
    ldr   r1, =0x00000006
    str   r1, [r0]

    /* Controller Setting */
    ldr   r0, =0x1e6e0060                        @ Fire DDRPHY Init
    ldr   r1, =0x00000005
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x00000091
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0120
    mov   r1, #0x00
    str   r1, [r0]
    b     ddr_phy_init_process

ddr3_phyinit_done:

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x31                              @ '1'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e000c
    ldr   r1, =0x00000040
    str   r1, [r0]

#ifdef CONFIG_DDR3_8GSTACK
    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000025
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000027
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000023
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000021
    str   r1, [r0]
#endif

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000005
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000007
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000003
    str   r1, [r0]

    ldr   r0, =0x1e6e0028
    ldr   r1, =0x00000011
    str   r1, [r0]

    ldr   r0, =0x1e6e000c
    ldr   r1, =0x00005C41
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r2, =0x70000000
ddr3_check_dllrdy:
    ldr   r1, [r0]
    tst   r1, r2
    bne   ddr3_check_dllrdy

    ldr   r0, =0x1e6e000c
    ldr   r1, =0x42AA5C81
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x0001AF91
    str   r1, [r0]

    ldr   r0, =0x1e6e0120                        @ VGA Compatible Mode
    ldr   r1, [r5, #ASTMMC_REGIDX_PLL]
    str   r1, [r0]

    b     Calibration_End
.LTORG
/******************************************************************************
 End DDR3 Init
 ******************************************************************************/
/******************************************************************************
 DDR4 Init
 ******************************************************************************/
ddr4_init:
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x34                              @ '4'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
/* Debug - UART console message */

    adrl  r5, TIME_TABLE_DDR4_1600               @ init DRAM parameter table
#ifdef CONFIG_DRAM_1333
    adrl  r5, TIME_TABLE_DDR4_1333
#endif
#ifdef CONFIG_DRAM_1866
    adrl  r5, TIME_TABLE_DDR4_1866
#endif
#ifdef CONFIG_DRAM_2133
    adrl  r5, TIME_TABLE_DDR4_2133
#endif

    ldr   r0, =0x1e6e0004
#ifdef CONFIG_DDR4_4GX8
    ldr   r1, =0x00002313                        @ Init to 8GB
#else
    ldr   r1, =0x00000313                        @ Init to 8GB
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e0010
    ldr   r1, [r5, #ASTMMC_REGIDX_010]
    str   r1, [r0]

    ldr   r0, =0x1e6e0014
    ldr   r1, [r5, #ASTMMC_REGIDX_014]
    str   r1, [r0]

    ldr   r0, =0x1e6e0018
    ldr   r1, [r5, #ASTMMC_REGIDX_018]
    str   r1, [r0]

    /* DRAM Mode Register Setting */
    ldr   r0, =0x1e6e0020                        @ MRS_4/6
    ldr   r1, [r5, #ASTMMC_REGIDX_020]
    str   r1, [r0]

    ldr   r0, =0x1e6e0024                        @ MRS_5
    ldr   r1, [r5, #ASTMMC_REGIDX_024]
    str   r1, [r0]

    ldr   r0, =0x1e6e002c                        @ MRS_0/2
    ldr   r1, [r5, #ASTMMC_REGIDX_02C]
    mov   r2, #0x1
    orr   r1, r1, r2, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0030                        @ MRS_1/3
    ldr   r1, [r5, #ASTMMC_REGIDX_030]
    str   r1, [r0]

    /* Start DDR PHY Setting */
    ldr   r0, =0x1e6e0200
    ldr   r1, =0x42492AAE
    str   r1, [r0]

    ldr   r0, =0x1e6e0204
    ldr   r1, =0x08002000
    str   r1, [r0]

    ldr   r0, =0x1e6e020c
    ldr   r1, =0x55E00A0A
    str   r1, [r0]

    ldr   r0, =0x1e6e0210
    ldr   r1, =0x20000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0214
    ldr   r1, [r5, #ASTMMC_REGIDX_214]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e0
    ldr   r1, [r5, #ASTMMC_REGIDX_2E0]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e4
    ldr   r1, [r5, #ASTMMC_REGIDX_2E4]
    str   r1, [r0]

    ldr   r0, =0x1e6e02e8
    ldr   r1, [r5, #ASTMMC_REGIDX_2E8]
    str   r1, [r0]

    ldr   r0, =0x1e6e02ec
    ldr   r1, [r5, #ASTMMC_REGIDX_2EC]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f0
    ldr   r1, [r5, #ASTMMC_REGIDX_2F0]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f4
    ldr   r1, [r5, #ASTMMC_REGIDX_2F4]
    str   r1, [r0]

    ldr   r0, =0x1e6e02f8
    ldr   r1, [r5, #ASTMMC_REGIDX_2F8]
    str   r1, [r0]

    ldr   r0, =0x1e6e0290
    ldr   r1, =0x00100008
    str   r1, [r0]

    ldr   r0, =0x1e6e02c4
/*  ldr   r1, =0x3C183C3C */
    ldr   r1, =0x481F5050
    str   r1, [r0]

    ldr   r0, =0x1e6e02c8
/*  ldr   r1, =0x00631E0E */
    ldr   r1, =0x00632313
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x0001A991
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
/* Debug - UART console message */

    /********************************************
     PHY Vref Scan
     r6 : min
     r7 : max
     r8 : passcnt
     r9 : CBRtest result
     r10: loopcnt
     r11: free
    ********************************************/
ddr4_vref_phy_cal_start:
    mov   r6, #0xFF
    mov   r7, #0x0
    mov   r8, #0x0
    mov   r10, #0x0

    ldr   r0, =0x1e6e0120
    ldr   r1, =0x00000001
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x61                              @ 'a'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e02c0
    ldr   r1, =0x00001806
    str   r1, [r0]

ddr4_vref_phy_loop:
    ldr   r0, =0x1e6e0060
    ldr   r1, =0x00000000
    str   r1, [r0]

    add   r10, r10, #0x01
    cmp   r10, #0xFF
    beq   ddr4_vref_phy_test_fail                @ no valid margin and retry

    ldr   r0, =0x1e6e02cc
    orr   r1, r10, r10, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0060
    ldr   r1, =0x00000005
    str   r1, [r0]
    b     ddr_phy_init_process

ddr4_vref_phy_phyinit_done:

    b     cbr_test_start

ddr4_vref_phy_cbrtest_done:
    cmp   r9, #0x01
    bne   ddr4_vref_phy_test_fail
    add   r8, r8, #0x01
    cmp   r6, r10
    movgt r6, r10
    cmp   r7, r10
    movlt r7, r10
    b     ddr4_vref_phy_loop

ddr4_vref_phy_test_fail:
    cmp   r8, #0x0
    bne   ddr4_vref_phy_loop_end
    cmp   r10, #0xFF
    beq   ddr4_vref_phy_cal_start
    b     ddr4_vref_phy_loop

ddr4_vref_phy_loop_end:
    ldr   r0, =0x1e6e02cc
    add   r1, r6, r7
    add   r1, r1, #0x01
    mov   r2, r1, lsr #1
    orr   r1, r2, r2, lsl #8
    str   r1, [r0]

    /* Record vref result to SRAM */
    ldr   r0, =0x1e720000
    orr   r1, r6, r2, lsl #16
    orr   r1, r1, r7, lsl #8
    str   r1, [r0]

    /********************************************
     DDR Vref Scan
     r6 : min
     r7 : max
     r8 : passcnt
     r9 : CBRtest result
     r10: loopcnt
     r11: free
    ********************************************/
ddr4_vref_ddr_cal_start:
    mov   r6, #0xFF
    mov   r7, #0x0
    mov   r8, #0x0
    mov   r10, #0x0

    ldr   r0, =0x1e6e0120
    ldr   r1, =0x00000002
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x62                              @ 'b'
    str   r1, [r0]
/* Debug - UART console message */

ddr4_vref_ddr_loop:
    ldr   r0, =0x1e6e0060
    ldr   r1, =0x00000000
    str   r1, [r0]

    add   r10, r10, #0x01
    cmp   r10, #0x40
    beq   ddr4_vref_ddr_test_fail                @ no valid margin and retry

    ldr   r0, =0x1e6e02c0
    mov   r1, #0x06
    orr   r1, r1, r10, lsl #8
    str   r1, [r0]

    ldr   r0, =0x1e6e0060
    ldr   r1, =0x00000005
    str   r1, [r0]
    b     ddr_phy_init_process

ddr4_vref_ddr_phyinit_done:

    b     cbr_test_start

ddr4_vref_ddr_cbrtest_done:
    cmp   r9, #0x01
    bne   ddr4_vref_ddr_test_fail
    add   r8, r8, #0x01
    cmp   r6, r10
    movgt r6, r10
    cmp   r7, r10
    movlt r7, r10
    b     ddr4_vref_ddr_loop

ddr4_vref_ddr_test_fail:
    cmp   r8, #0x0
    bne   ddr4_vref_ddr_loop_end
    cmp   r10, #0x40
    beq   ddr4_vref_ddr_cal_start
    b     ddr4_vref_ddr_loop

ddr4_vref_ddr_loop_end:
    ldr   r0, =0x1e6e0060
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e02c0
    add   r1, r6, r7
    add   r1, r1, #0x01
    mov   r2, r1, lsr #1
    mov   r1, r2, lsl #8
    orr   r1, r1, #0x06
    str   r1, [r0]

    /* Record vref result to SRAM */
    ldr   r0, =0x1e720004
    orr   r1, r6, r2, lsl #16
    orr   r1, r1, r7, lsl #8
    str   r1, [r0]
    add   r0, r0, #0x04
    mov   r1, #0x0
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x63                              @ 'c'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0120
    ldr   r1, =0x00000003
    str   r1, [r0]

    ldr   r0, =0x1e6e0060                        @ Fire DDRPHY Init
    ldr   r1, =0x00000005
    str   r1, [r0]
    b     ddr_phy_init_process

ddr4_phyinit_done:
    /*******************************************/
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x31                              @ '1'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e000c
    ldr   r1, =0x42AA5C81
    str   r1, [r0]

    ldr   r0, =0x1e6e0034
    ldr   r1, =0x0001AF91
    str   r1, [r0]

    ldr   r0, =0x1e6e0120                        @ VGA Compatible Mode
    ldr   r1, [r5, #ASTMMC_REGIDX_PLL]
    str   r1, [r0]

    b     Calibration_End

.LTORG
/******************************************************************************
 End DDR4 Init
 ******************************************************************************/
/******************************************************************************
 Global Process
 ******************************************************************************/
    /********************************************
     DDRPHY Init Process
    ********************************************/
ddr_phy_init_process:
    /* Wait DDR PHY init done - timeout 300 ms */
    ldr   r2, =0x000493E0                        @ Set Timer3 Reload = 300 ms
    init_delay_timer
    ldr   r3, =0x1e6e0060
ddr_phy_init:
    check_delay_timer
    beq   ddr_phy_init_timeout
    ldr   r1, [r3]
    tst   r1, #0x01
    bne   ddr_phy_init

    /* Check DDR PHY init status */
    ldr   r0, =0x1e6e0300
    ldr   r2, =0x000A0000
    ldr   r1, [r0]
    tst   r1, r2
    beq   ddr_phy_init_success

ddr_phy_init_timeout:
    ldr   r0, =0x1e6e0060                        @ Reset PHY
    mov   r1, #0x00
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x2E                              @ '.'
    str   r1, [r0]
/* Debug - UART console message */

    clear_delay_timer
    /* Delay about 10us */
    ldr   r2, =0x0000000A                        @ Set Timer3 Reload = 10 us
    init_delay_timer
ddr_phy_init_delay_0:
    check_delay_timer
    bne   ddr_phy_init_delay_0
    clear_delay_timer
    /* end delay 10us */

    ldr   r0, =0x1e6e0060                        @ Fire PHY Init
    mov   r1, #0x05
    str   r1, [r0]
    b     ddr_phy_init_process

ddr_phy_init_success:
    clear_delay_timer
    ldr   r0, =0x1e6e0060
    mov   r1, #0x06
    str   r1, [r0]

    ldr   r0, =0x1e6e0120
    ldr   r1, [r0]
    cmp   r1, #0
    beq   ddr3_phyinit_done
    cmp   r1, #1
    beq   ddr4_vref_phy_phyinit_done
    cmp   r1, #2
    beq   ddr4_vref_ddr_phyinit_done
    bne   ddr4_phyinit_done

    /********************************************
     CBRTest
    ********************************************/
cbr_test_start:
    ldr   r0, =0x1e6e000c
    ldr   r1, =0x00005C01
    str   r1, [r0]
    ldr   r0, =0x1e6e0074
    ldr   r1, =0x0000FFFF                        @ test size = 64KB
    str   r1, [r0]
    ldr   r0, =0x1e6e007c
    ldr   r1, =0xFF00FF00
    str   r1, [r0]

cbr_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000085
    str   r1, [r0]
    ldr   r3, =0x3000
    ldr   r11, =0x50000
cbr_wait_engine_idle_0:
    subs  r11, r11, #1
    beq   cbr_test_fail
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr_wait_engine_idle_0

    ldr   r0, =0x1e6e0070                        @ read fail bit status
    ldr   r3, =0x2000
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[13] = fail bit
    bne   cbr_test_fail

cbr_test_burst:
    mov   r1, #0x00                              @ initialize loop index, r1 is loop's index
cbr_test_burst_loop:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0xC1                          @ test command = 0xC1 | (datagen << 3)
    str   r2, [r0]
    ldr   r3, =0x3000
    ldr   r11, =0x20000
cbr_wait_engine_idle_1:
    subs  r11, r11, #1
    beq   cbr_test_fail
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr_wait_engine_idle_1

    ldr   r0, =0x1e6e0070                        @ read fail bit status
    ldr   r3, =0x2000
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[13] = fail bit
    bne   cbr_test_fail

    add   r1, r1, #1                             @ increase the test mode index
    cmp   r1, #0x04                              @ test 4 modes
    bne   cbr_test_burst_loop

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    mov   r9, #0x1
    b     cbr_test_pattern_end                   @ CBRTest() return(1)

cbr_test_fail:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    mov   r9, #0x0                               @ CBRTest() return(0)

cbr_test_pattern_end:
    ldr   r0, =0x1e6e000c
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e6e0120
    ldr   r1, [r0]
    cmp   r1, #1
    beq   ddr4_vref_phy_cbrtest_done
    bne   ddr4_vref_ddr_cbrtest_done

.LTORG
/******************************************************************************
 Other features configuration
 *****************************************************************************/
Calibration_End:
    /*******************************
     Check DRAM Size
     1Gb : 0x80000000 ~ 0x87FFFFFF
     2Gb : 0x80000000 ~ 0x8FFFFFFF
     4Gb : 0x80000000 ~ 0x9FFFFFFF
     8Gb : 0x80000000 ~ 0xBFFFFFFF
    *******************************/
    ldr   r0, =0x1e6e0004
    ldr   r6, [r0]
    bic   r6, r6, #0x00000003                    @ record MCR04
    ldr   r7, [r5, #ASTMMC_REGIDX_RFC]

check_dram_size:
    ldr   r0, =0xA0100000
    ldr   r1, =0x41424344
    str   r1, [r0]
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    str   r1, [r0]
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    str   r1, [r0]
    ldr   r0, =0x80100000
    ldr   r1, =0x1D1E1F10
    str   r1, [r0]
    ldr   r0, =0xA0100000
    ldr   r1, =0x41424344
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 8Gbit
    orreq r6, r6, #0x03
    moveq r7, r7, lsr #24
    beq   check_dram_size_end
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 4Gbit
    orreq r6, r6, #0x02
    moveq r7, r7, lsr #16
    beq   check_dram_size_end
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 2Gbit
    orreq r6, r6, #0x01
    moveq r7, r7, lsr #8

check_dram_size_end:
    ldr   r0, =0x1e6e0004
    str   r6, [r0]
    ldr   r0, =0x1e6e0014
    ldr   r1, [r0]
    bic   r1, r1, #0x000000FF
    and   r7, r7, #0xFF
    orr   r1, r1, r7
    str   r1, [r0]

    /* Version Number */
    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    mov   r2, #ASTMMC_INIT_VER
    orr   r1, r1, r2, lsl #20
    str   r1, [r0]

    ldr   r0, =0x1e6e0088
    ldr   r1, =ASTMMC_INIT_DATE
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x32                              @ '2'
    str   r1, [r0]
/* Debug - UART console message */

    /* Enable DRAM Cache */
    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    mov   r2, #1
    orr   r2, r1, r2, lsl #12
    str   r2, [r0]
    ldr   r3, =0x00080000
dram_cache_init:
    ldr   r2, [r0]
    tst   r2, r3
    beq   dram_cache_init
    mov   r2, #1
    orr   r1, r1, r2, lsl #10
    str   r1, [r0]

set_scratch:
    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    orr   r1, r1, #0x40
    str   r1, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x6F                              @ 'o'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x65                              @ 'e'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
/* Debug - UART console message */

platform_exit:
#ifdef CONFIG_DRAM_ECC
    ldr   r0, =0x1e6e0004
    ldr   r1, [r0]
    orr   r1, r1, #0x80
    str   r1, [r0]

    ldr   r0, =0x1e6e0054
    ldr   r1, =CONFIG_DRAM_ECC_SIZE              /* ECC protected memory size */
    str   r1, [r0]

    ldr   r0, =0x1e6e007C
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e6e0074
    str   r1, [r0]

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000221
    str   r1, [r0]

    ldr   r2, =0x00001000
ECC_Init_Flag:
    ldr   r1, [r0]
    tst   r1, r2                                 @ D[12] = 1, Done
    beq   ECC_Init_Flag

    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0050
    ldr   r1, =0x80000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0050
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000400                        @ Enable ECC auto-scrubbing
    str   r1, [r0]
#endif

    ldr   r0, =0x1e6e2008                        @ Set Video ECLK phase
    ldr   r1, [r0]
    ldr   r2, =0xfffffff3
    and   r1, r1, r2
    orr   r1, r1, #0x08
    str   r1, [r0]

    ldr   r0, =0x1e6e2048                        @ Set MAC interface delay timing
    ldr   r1, =0x00492082
    str   r1, [r0]

    ldr   r0, =0x1e6e2070                        @ Set MAC AHB bus clock
    ldr   r1, [r0]
    mov   r2, #0x07                              @ Default RMII, set MHCLK = HPLL/16
    tst   r1, #0xC0
    movne r2, #0x02                              @ if RGMII,     set MHCLK = HPLL/6
    ldr   r0, =0x1e6e2008
    ldr   r1, [r0]
    bic   r1, r1, #0x00070000
    orr   r1, r1, r2, lsl #16
    str   r1, [r0]

/******************************************************************************
 SPI Timing Calibration
 ******************************************************************************/
    mov   r2, #0x0
    mov   r6, #0x0
    mov   r7, #0x0
    init_spi_checksum
spi_checksum_wait_0:
    ldr   r1, [r0]
    tst   r1, r2
    beq   spi_checksum_wait_0
    ldr   r0, =0x1e620090
    ldr   r5, [r0]                               @ record golden checksum
    ldr   r0, =0x1e620080
    mov   r1, #0x0
    str   r1, [r0]

    ldr   r0, =0x1e620010                        @ set to fast read mode
    ldr   r1, =0x000B0041
    str   r1, [r0]

    ldr   r6, =0x00F7E6D0                        @ Init spiclk loop
    mov   r8, #0x0                               @ Init delay record

spi_cbr_next_clkrate:
    mov   r6, r6, lsr #0x4
    cmp   r6, #0x0
    beq   spi_cbr_end

    mov   r7, #0x0                               @ Init delay loop
    mov   r8, r8, lsl #4

spi_cbr_next_delay_s:
    mov   r2, #0x8
    init_spi_checksum
spi_checksum_wait_1:
    ldr   r1, [r0]
    tst   r1, r2
    beq   spi_checksum_wait_1
    ldr   r0, =0x1e620090
    ldr   r2, [r0]                               @ read checksum
    ldr   r0, =0x1e620080
    mov   r1, #0x0
    str   r1, [r0]
    cmp   r2, r5
    bne   spi_cbr_next_delay_e

    mov   r2, #0x0
    init_spi_checksum
spi_checksum_wait_2:
    ldr   r1, [r0]
    tst   r1, r2
    beq   spi_checksum_wait_2
    ldr   r0, =0x1e620090
    ldr   r2, [r0]                               @ read checksum
    ldr   r0, =0x1e620080
    mov   r1, #0x0
    str   r1, [r0]
    cmp   r2, r5
    bne   spi_cbr_next_delay_e

    orr   r8, r8, r7                             @ record passed delay
    b     spi_cbr_next_clkrate

spi_cbr_next_delay_e:
    add   r7, r7, #0x1
    cmp   r7, #0x6
    blt   spi_cbr_next_delay_s
    b     spi_cbr_next_clkrate

spi_cbr_end:
    ldr   r0, =0x1e620094
    str   r8, [r0]
    ldr   r0, =0x1e620010
    mov   r1, #0x0
    str   r1, [r0]

/* Test - DRAM initial time */
    ldr   r0, =0x1e782040
    ldr   r1, [r0]
    ldr   r0, =0xFFFFFFFF
    sub   r1, r0, r1
    ldr   r0, =0x1e6e008c
    str   r1, [r0]
    ldr   r0, =0x1e78203c
    ldr   r1, =0x0000F000
    str   r1, [r0]
/* Test - DRAM initial time */

    /* Disable Timer separate mode */
    ldr   r0, =0x1e782038
    ldr   r1, =0xEA
    str   r1, [r0]

    ldr r0, =0x1e6e2070                          @ enable VBIOS
    ldr r1, [r0]
    orr r1, r1, #0x20
    str r1, [r0]

    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr

