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
 * AST1510 DDR2/DDR3 SDRAM controller initialization and calibration sequence
 *
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * Release date: 2012.12.30 formal release
 *
 * Optional define variable
 * 1. DRAM Size              //
 *    CONFIG_DRAM_512MBIT    // 512M bit
 *    CONFIG_DRAM_1GBIT      // 1G   bit (default)
 *    CONFIG_DRAM_2GBIT      // 2G   bit
 *    CONFIG_DRAM_4GBIT      // 4G   bit
 * 2. DRAM Speed             //
 *    CONFIG_DRAM_408        // 408MHz (DDR-800) (default)
 *    CONFIG_DRAM_504        // 504MHz (DDR-1008)
 *    CONFIG_DRAM_528        // 528MHz (DDR-1066)
 *    CONFIG_DRAM_552        // 552MHz (DDR-1104)
 *    CONFIG_DRAM_576        // 576MHz (DDR-1152)
 * 3. ECC Function enable
 *    CONFIG_DRAM_ECC        // define to enable ECC function
 *                           // when enabled, must define the ECC protected memory size at 0x1e6e0054
 * 4. CPU Speed
 *    CONFIG_CPU_384         // 384MHz
 *    CONFIG_CPU_408         // 408MHz (default)
 *    CONFIG_CPU_420         // 420MHz
 *    CONFIG_CPU_432         // 432MHz
 * 5. Others
 *    CONFIG_AST1500_USB_ONLY// USB fnction only, CPU=408, DDR=504
 ******************************************************************************
 */

#include <config.h>
#include <version.h>
/******************************************************************************
 Calibration Macro Start
 ******************************************************************************/
/* PATTERN_TABLE,
   init_delay_timer,
   check_delay_timer,
   clear_delay_timer,
   record_dll2_pass_range,
   record_dll2_pass_range_h,
   are for DRAM calibration */

PATTERN_TABLE:
    .word   0xff00ff00
    .word   0xcc33cc33
    .word   0xaa55aa55
    .word   0x88778877
    .word   0x92cc4d6e       @ 5
    .word   0x543d3cde
    .word   0xf1e843c7
    .word   0x7c61d253
    .word   0x00000000       @ 8

    .macro init_delay_timer
    ldr r0, =0x1e782024                          @ Set Timer3 Reload
    str r2, [r0]

    ldr r0, =0x1e6c0038                          @ Clear Timer3 ISR
    ldr r1, =0x00040000
    str r1, [r0]

    ldr r0, =0x1e782030                          @ Enable Timer3
    ldr r1, [r0]
    mov r2, #7
    orr r1, r1, r2, lsl #8
    str r1, [r0]

    ldr r0, =0x1e6c0090                          @ Check ISR for Timer3 timeout
    .endm

    .macro check_delay_timer
    ldr r1, [r0]
    bic r1, r1, #0xFFFBFFFF
    mov r2, r1, lsr #18
    cmp r2, #0x01
    .endm

    .macro clear_delay_timer
    ldr r0, =0x1e782030                          @ Disable Timer3
    ldr r1, [r0]
    bic r1, r1, #0x00000F00
    str r1, [r0]

    ldr r0, =0x1e6c0038                          @ Clear Timer3 ISR
    ldr r1, =0x00040000
    str r1, [r0]
    .endm

    .macro record_dll2_pass_range
    ldr                 r1, [r0]
    bic                 r2, r1, #0xFFFFFF00
    cmp                 r2, r3                   @ record min
    bicgt               r1, r1, #0x000000FF
    orrgt               r1, r1, r3
    bic                 r2, r1, #0xFFFF00FF
    cmp                 r3, r2, lsr #8           @ record max
    bicgt               r1, r1, #0x0000FF00
    orrgt               r1, r1, r3, lsl #8
    str                 r1, [r0]
    .endm

    .macro record_dll2_pass_range_h
    ldr                 r1, [r0]
    bic                 r2, r1, #0xFF00FFFF
    mov                 r2, r2, lsr #16
    cmp                 r2, r3                   @ record min
    bicgt               r1, r1, #0x00FF0000
    orrgt               r1, r1, r3, lsl #16
    bic                 r2, r1, #0x00FFFFFF
    cmp                 r3, r2, lsr #24          @ record max
    bicgt               r1, r1, #0xFF000000
    orrgt               r1, r1, r3, lsl #24
    str                 r1, [r0]
    .endm

    .macro print_hex_char
    cmp                 r1, #9
    addgt               r1, r1, #0x37
    addle               r1, r1, #0x30
    str                 r1, [r0]
    .endm

/******************************************************************************
 Calibration Macro End
 ******************************************************************************/

.globl lowlevel_init
lowlevel_init:

init_dram:
    /* save lr */
    mov r4, lr
/* Test - DRAM initial time */
    ldr r0, =0x1e782044
    ldr r1, =0xFFFFFFFF
    str r1, [r0]

    ldr r0, =0x1e782030
    ldr r1, [r0]
    bic r1, r1, #0x0000F000
    mov r2, #3
    orr r1, r1, r2, lsl #12
    str r1, [r0]
/* Test - DRAM initial time */

    /*Set Scratch register Bit 7 before initialize*/
    ldr r0, =0x1e6e2000
    ldr r1, =0x1688a8a8
    str r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]
/* Not necessary for AST1510
    ldr r0, =0x1e6e207c
    ldr r3, [r0]
    bic r3, r3, #0xFF00FFFF
    mov r3, r3, lsr #16
    ldr r0, =0x1e6e2088                          @ A0 workaround for PCIE
    ldr r1, [r0]
    ldr r2, =0x00100000
    cmp r3, #0
    orreq r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e202c                          @ Prefetch for PCIE
    ldr r1, [r0]
    ldr r2, =0x00080000
    orr r1, r1, r2
    str r1, [r0]
*/
    /* Check Scratch Register Bit 6 */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

#if !defined(CONFIG_CPU_384)
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0x00000F00
    mov r2, #0x7
    orr r1, r1, r2, lsl #8                       @ Set trap => H-PLL=408MHz, CPU:AHB=2:1
    str r1, [r0]

#if !defined(CONFIG_CPU_408)
#if defined(CONFIG_CPU_432)
    ldr r0, =0x1e6e2024                          @ H-PLL Frequency
    ldr r1, =0x40210                             @ 432 MHz
    str r1, [r0]
#elif defined(CONFIG_CPU_420)
    ldr r0, =0x1e6e2024                          @ H-PLL Frequency
    ldr r1, =0x40431                             @ 420 MHz
    str r1, [r0]
#endif
#endif
#endif

    /* Init watchdog */
    ldr r0, =0x1e6e2004                          @ enable reset DRAM
    ldr r1, [r0]
    orr r1, r1, #0x01
    str r1, [r0]

    ldr r0, =0x1e785004
    ldr r1, =0x004C4B40                          @ set to 5 second
    str r1, [r0]

    ldr r0, =0x1e785008
    ldr r1, =0x00004755
    str r1, [r0]

    ldr r0, =0x1e78500c
    ldr r1, =0x00000033
    str r1, [r0]

init_dram_start:
    /* Reset MMC */
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r1, =0x00000000
    ldr r0, =0x1e6e0034
    str r1, [r0]
    ldr r0, =0x1e6e0018
    str r1, [r0]
    ldr r0, =0x1e6e0024
    str r1, [r0]
    ldr r0, =0x1e6e0064
    str r1, [r0]
    ldr r0, =0x1e6e0060
    ldr r1, =0x000000F0
    str r1, [r0]

    ldr r0, =0x1e6e2020                          @ power down M-PLL
    ldr r1, [r0]
    mov r2, #0x03
    orr r1, r1, r2, lsl #16
    str r1, [r0]

    /* Delay about 3ms */
    ldr r0, =0x1e782030                          @ Init Timer3 Control
    ldr r1, [r0]
    bic r1, r1, #0x00000F00
    str r1, [r0]

    ldr r2, =0x00000C00                          @ Set Timer3 Reload = 3 ms
    init_delay_timer
delay_R:
    check_delay_timer
    bne delay_R
    clear_delay_timer
    /* end delay 3ms */

set_mpll:
    ldr r0, =0x1e6e2020                          @ M-PLL Frequency
    ldr r1, =0x01F0                              @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x0261
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x0120
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x02A1
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x0140
#endif
    str r1, [r0]

/* Debug - UART console message */
    ldr r0, =0x1e78400c
    mov r1, #0x83
    str r1, [r0]

    ldr r0, =0x1e6e202c
    ldr r2, [r0]
    mov r2, r2, lsr #12
    tst r2, #0x01
    ldr r0, =0x1e784000
#if 0 //24576 patch
    moveq r1, #0xA0                              @ Baudrate 9600
    movne r1, #0x0C                              @ Baudrate 9600, div13 //doesn't work?!
#else
    moveq r1, #0x0D                              @ Baudrate 115200
    movne r1, #0x01                              @ Baudrate 115200, div13
#endif
    str r1, [r0]

    ldr r0, =0x1e784004
    mov r1, #0x00
    str r1, [r0]

    ldr r0, =0x1e78400c
    mov r1, #0x03
    str r1, [r0]

    ldr r0, =0x1e784008
    mov r1, #0x07
    str r1, [r0]

    ldr r0, =0x1e784000
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x41                                @ 'A'
    str r1, [r0]
    mov r1, #0x4D                                @ 'M'
    str r1, [r0]
    mov r1, #0x20                                @ ' '
    str r1, [r0]
    mov r1, #0x49                                @ 'I'
    str r1, [r0]
    mov r1, #0x6E                                @ 'n'
    str r1, [r0]
    mov r1, #0x69                                @ 'i'
    str r1, [r0]
    mov r1, #0x74                                @ 't'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
/* Debug - UART console message */

    /* Delay about 10ms */
    ldr r2, =0x00002700                          @ Set Timer3 Reload = 10 ms
    init_delay_timer
delay_0:
    check_delay_timer
    bne delay_0
    clear_delay_timer
    /* end delay 10ms */

    ldr   r0, =0x1e7200a0                        @ Init the retry counter
    mov   r1, #0
    str   r1, [r0]

/******************************************************************************
 Init DRAM common registers
 ******************************************************************************/
    ldr r0, =0x1e6e0064                          @ REG_MADJ, reset DLL
    ldr r1, =0x00034C4C
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00034C4C
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00034C4C
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00136868
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00136868
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0068                          @ REG_SADJ
    ldr r1, =0x00001800
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00001600
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00001600
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00004534
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00004534
#endif
    str r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x00000010                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay_1:
    check_delay_timer
    bne delay_1
    clear_delay_timer
    /* end delay 10us */

    ldr r0, =0x1e6e0064                          @ REG_MADJ | 0xC0000, enable DLL
    ldr r1, [r0]
    ldr r2, =0xC0000
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e0008
    ldr r1, =0x00F0047F                          @ Video/CRT place at highest 32MB area
    str r1, [r0]

    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0x0000000C
    orr r1, r1, #0x08                            @ Set the protected area = 32MB
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0xFF666666
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x2FF26222
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x22222222
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x22222222
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x80000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0074
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0078
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e007c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0080
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0084
    ldr r1, =0x00FFFFFF
    str r1, [r0]

    ldr r0, =0x1e6e0088                          @ REG_DQIDLY
    ldr r1, =0x00000089                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000088
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000088
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000080
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000078
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0020                          @ REG_DQSIC
    ldr r1, =0x00000160                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000180
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000180
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000180
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000180
#endif
    str r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x00000010                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay_2:
    check_delay_timer
    bne delay_2
    clear_delay_timer
    /* end delay 10us */

    /* Check DRAM Type by H/W Trapping */
    ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xFEFFFFFF                      @ bit[24]=1 => DDR2
    mov r2, r1, lsr #24
    cmp r2, #0x01
    beq ddr2_init
    bne ddr3_init
.LTORG

/******************************************************************************
 DDR3 Init

 tRCD   = 15     ns
 tRAS   = 37.5   ns
 tRRD   = max(4 CK,10 ns)
 tRP    = 15     ns
 tRFC   = 110    ns
 tRTP   = max(4 CK,7.5 ns)
 tWR    = 15     ns
 tXSNR  = max(10 CK,200 ns)
 tWTR   = max(4 CK,7.5 ns)
 tFAW   = 50     ns
 tMRD   = max(15 CK,20 ns)
 tCL    = 6(408MHz), 7(528MHz), 9(>576MHz)
 tCWL   = 5(408MHz), 6(528MHz), 7(>576MHz)
 ******************************************************************************/
ddr3_init:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x33                                @ '3'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
/* Debug - UART console message */

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000131                          @ DRAM_CONFIG 1Gbit
#if defined(CONFIG_DRAM_2GBIT)
    ldr r1, =0x00000132                          @ DRAM_CONFIG 2Gbit
#elif defined(CONFIG_DRAM_4GBIT)
    ldr r1, =0x00000133                          @ DRAM_CONFIG 4Gbit
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0010                          @ REG_AC1
    ldr r1, =0x33302825                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x43402926
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x43402A37
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x43402A37
#endif
    str r1, [r0]

    /* Check DRAM CL Timing by H/W Trapping */
/*  ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #9                           @ Set CL
    ldr r1, =0x00020000
    add r2, r2, r1
    ldr r1, [r0]
    bic r1, r1, #0xFBFFFFFF
    mov r1, r1, lsr #6                           @ Set CWL
    orr r2, r2, r1
    ldr r1, =0x00300000
    add r2, r2, r1
*/
    ldr r0, =0x1e6e0014                          @ REG_AC2
    ldr r1, =0xBB338615                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0xEE44B61C
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0xFF56B61D
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0xFF56C61F
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e0060                          @ REG_DRV
    ldr r1, =0x000000FA                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x000000FA
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x000000FA
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x000000FA
#endif
    str r1, [r0]

    ldr r0, =0x1e6e006c                          @ REG_IOZ
    ldr r1, =0x00000034                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x00000034
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000034
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000034
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x4000A170
    str r1, [r0]

    ldr r0, =0x1e6e0018
    ldr r1, =0x00002370
    str r1, [r0]

Start_MCLK2X_Phase_CBR_DDR3:
    ldr r0, =0x1e6e001c
    ldr r2, =0x08000000                          @ D[27] = 1, locked
Wait_MCLK2X_lock_MCLK_DDR3:
    ldr r1, [r0]
    tst r1, r2
    beq Wait_MCLK2X_lock_MCLK_DDR3

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005C04
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000002                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay3_1:
    check_delay_timer
    bne delay3_1
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, [r0]
    mov r1, r1, lsr #8
    and r1, r1, #0xff                            @ data = (data >> 8) & 0xff

    tst r1, #0x08                                @ DLL1.1 finetune for optimize CLK duty cycle
    bne Out_Range_DDR3                           @ when meet (dll & 0x08) || (dll & 0x7) < 2 || dll < 2)
    cmp r1, #2
    blt Out_Range_DDR3
    and r1, r1, #0x7
    cmp r1, #2
    blt Out_Range_DDR3
    b   In_Range_DDR3

Out_Range_DDR3:
    ldr r0, =0x1e6e0064
    ldr r1, [r0]
    bic r1, r1, #0xC0000                         @ enable reset DLL
    add r1, r1, #0x04
    and r2, r1, #0xff
    cmp r2, #96                                  @ define the sadj high limit
#if defined(CONFIG_DRAM_528)
    cmp r2, #76
#elif defined(CONFIG_DRAM_552)
    cmp r2, #136
#elif defined(CONFIG_DRAM_576)
    cmp r2, #136
#endif
    bgt In_Range_DDR3                            @ out of the maximum MAdj allowed
    str r1, [r0]

    bic r1, r1, #0xFFEFFFFF                      @ if(r1 & 0x00100000)
    cmp r1, #0
    moveq r2, r2, lsr #2                         @ dll refin div 1
    addeq r2, r2, #5                             @ sadj_dllcko = (sadj_mclk2x >> 2) + 5
    movne r2, r2, lsr #3                         @ dll refin div 2
    addne r2, r2, #3                             @ sadj_dllcko = (sadj_mclk2x >> 2) + 3

    ldr r0, =0x1e6e0068
    ldr r1, [r0]
    and r1, r1, #0xFF
    add r2, r2, r1
    orr r1, r1, r2, lsl #8
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000003                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay3_2:
    check_delay_timer
    bne delay3_2
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e0064                          @ disable reset DLL
    ldr r1, [r0]
    ldr r2, =0xC0000
    orr r1, r1, r2
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000003                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay3_3:
    check_delay_timer
    bne delay3_3
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e0018
    ldr r1, [r0]
    bic r1, r1, #0x200                           @ disable calibration
    str r1, [r0]
    ldr r2, =0x00000200                          @ redo calibration
    orr r1, r1, r2                               @ or 0x200
    str r1, [r0]
    b   Start_MCLK2X_Phase_CBR_DDR3

In_Range_DDR3:
    ldr r0, =0x1e6e0018
    ldr r2, =0x00000C00
    ldr r1, [r0]
    orr r1, r1, r2                               @ or 0xC00
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000040
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000190                          @ Set Timer3 Reload = 400 us
    init_delay_timer
delay3_4:
    check_delay_timer
    bne delay3_4
    clear_delay_timer
    /* end delay 400us */

    /* Check DRAM CL Timing by H/W Trapping */
/*  ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #21                          @ Set CL
    ldr r1, =0x00000010
    add r2, r2, r1
    ldr r1, [r0]
    bic r1, r1, #0xFBFFFFFF
    mov r1, r1, lsr #7                           @ Set CWL
    orr r2, r2, r1
*/
    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x04001720                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x04081930
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x04101B50
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x04101B50
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e0030                          @ REG_EMRS
    ldr r1, =0x00000000                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x00000000
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000000
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000000
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS2
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS3
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x04001620                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x04081830
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x04101A50
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x04101A50
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e000c                          @ Refresh 8 times
    ldr r1, =0x00005C48
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c                          @ Set refresh cycle
    ldr r1, =0x00005C01
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, [r0]
    bic r1, r1, #0xFFF9FFFF
    mov r2, r1, lsr #3                           @ get CL

    ldr r0, =0x1e6e0034                          @ REG_PWC
    ldr r1, =0x00000303                          @ 408 MHz
#if defined(CONFIG_DRAM_528)
    ldr r1, =0x00000303
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000303
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000303
#endif
    orr r1, r1, r2
    str r1, [r0]

/*  ldr r0, =0x1e6e0080
    ldr r2, =0x40000000
Wait_DQI_Delay_Lock_DDR3:
    ldr r1, [r0]
    tst r1, r2                                   @ D[30]=1, locked
    beq Wait_DQI_Delay_Lock_DDR3

    ldr r0, =0x1e6e0020
    ldr r2, =0x00000800
Wait_DQSI_Delay_Lock_DDR3:
    ldr r1, [r0]
    tst r1, r2                                   @ D[11] = 1, locked
    beq Wait_DQSI_Delay_Lock_DDR3
*/
    b   Calibration_Start
.LTORG
/******************************************************************************
 End DDR3 Init
 ******************************************************************************/

/******************************************************************************
 DDR2 Init

 tRCD   = 15    ns
 tRAS   = 45    ns
 tRRD   = 10    ns
 tRP    = 15    ns
 tRFC   = 127.5 ns
 tRTP   = 7.5   ns
 tWR    = 15    ns
 tXSNR  = 200   ns
 tWTR   = 7.5   ns
 tFAW   = 50    ns
 tMRD   = 4     CK
 ******************************************************************************/
ddr2_init:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x32                                @ '2'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
/* Debug - UART console message */

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000131                          @ DRAM_CONFIG 1Gbit
#if defined(CONFIG_DRAM_512MBIT)
    ldr r1, =0x00000110                          @ DRAM_CONFIG 512Mbit
#elif defined(CONFIG_DRAM_2GBIT)
    ldr r1, =0x00000132                          @ DRAM_CONFIG 2Gbit
#elif defined(CONFIG_DRAM_4GBIT)
    ldr r1, =0x00000133                          @ DRAM_CONFIG 4Gbit
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0010                          @ REG_AC1
    ldr r1, =0x33302725                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x33302926
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x33302926
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x33302926
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x43402926
#endif
    str r1, [r0]

    /* Check DRAM CL Timing by H/W Trapping */
/*  ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #5                           @ Set CL
    mov r1, r2, lsr #4                           @ Set CWL
    orr r2, r2, r1
    ldr r1, =0x00110000
    add r2, r2, r1
*/
    ldr r0, =0x1e6e0014                          @ REG_AC2
    ldr r1, =0xBB33A219                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0xEE44D221
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0xEE44D221
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0xFF44D222
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0xFF44E224
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e0060                          @ REG_DRV
    ldr r1, =0x000000FA                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x000000FA
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x000000F5
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x000000F9
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x000000FA
#endif
    str r1, [r0]

    ldr r0, =0x1e6e006c                          @ REG_IOZ
    ldr r1, =0x00000013                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000024
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000025
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000025
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000034
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0018                          @ fixed DQI delay
    ldr r1, =0x4000A122                          @ DQS input mask normal mode for 528 MHz
    str r1, [r0]                                 @ DQS input mask delay 2     for 528 MHz

    ldr r0, =0x1e6e0018
    ldr r1, =0x00002322
    str r1, [r0]

    /* Delay about 3us */
    ldr r2, =0x00000003                          @ Set Timer3 Reload = 3 us
    init_delay_timer
delay2_0:
    check_delay_timer
    bne delay2_0
    clear_delay_timer
    /* end delay 3us */

Start_MCLK2X_Phase_CBR_DDR2:
    ldr r0, =0x1e6e001c
    ldr r2, =0x08000000
Wait_MCLK2X_lock_MCLK_DDR2:
    ldr r1, [r0]
    tst r1, r2                                   @ D[27] = 1, locked
    beq Wait_MCLK2X_lock_MCLK_DDR2

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005C04
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000002                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay2_1:
    check_delay_timer
    bne delay2_1
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, [r0]
    mov r1, r1, lsr #8
    and r1, r1, #0xff                            @ data = (data >> 8) & 0xff

    tst r1, #0x08                                @ DLL1.1 finetune for optimize CLK duty cycle
    bne Out_Range_DDR2                           @ when meet (dll & 0x08) || (dll & 0x7) < 2 || dll < 2)
    cmp r1, #2
    blt Out_Range_DDR2
    and r1, r1, #0x7
    cmp r1, #2
    blt Out_Range_DDR2
    b   In_Range_DDR2

Out_Range_DDR2:
    ldr r0, =0x1e6e0064
    ldr r1, [r0]
    bic r1, r1, #0xC0000                         @ enable reset DLL
    add r1, r1, #0x04
    and r2, r1, #0xff
    cmp r2, #96                                  @ support maximum 432 MHz
#if defined(CONFIG_DRAM_504)
    cmp r2, #76
#elif defined(CONFIG_DRAM_528)
    cmp r2, #76
#elif defined(CONFIG_DRAM_552)
    cmp r2, #136
#elif defined(CONFIG_DRAM_576)
    cmp r2, #136
#endif
    bgt In_Range_DDR2                            @ out of the maximum MAdj allowed
    str r1, [r0]

    bic r1, r1, #0xFFEFFFFF                      @ if(r1 & 0x00100000)
    cmp r1, #0
    moveq r2, r2, lsr #2                         @ dll refin div 1
    addeq r2, r2, #5                             @ sadj_dllcko = (sadj_mclk2x >> 2) + 5
    movne r2, r2, lsr #3                         @ dll refin div 2
    addne r2, r2, #3                             @ sadj_dllcko = (sadj_mclk2x >> 2) + 3

    ldr r0, =0x1e6e0068
    ldr r1, [r0]
    and r1, r1, #0xFF
    add r2, r2, r1
    orr r1, r1, r2, lsl #8
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000003                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay2_2:
    check_delay_timer
    bne delay2_2
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e0064                          @ disable reset DLL
    ldr r1, [r0]
    ldr r2, =0xC0000
    orr r1, r1, r2
    str r1, [r0]

    /* Delay about 2us */
    ldr r2, =0x00000003                          @ Set Timer3 Reload = 2 us
    init_delay_timer
delay2_3:
    check_delay_timer
    bne delay2_3
    clear_delay_timer
    /* end delay 2us */

    ldr r0, =0x1e6e0018
    ldr r1, [r0]
    bic r1, r1, #0x200                           @ disable calibration
    str r1, [r0]
    ldr r2, =0x00000200                          @ redo calibration
    orr r1, r1, r2                               @ or 0x200
    str r1, [r0]
    b   Start_MCLK2X_Phase_CBR_DDR2

In_Range_DDR2:
    ldr r0, =0x1e6e0018
    ldr r2, =0x00000C00
    ldr r1, [r0]
    orr r1, r1, r2                               @ or 0xC00
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00000000
    str r1, [r0]

    /* Delay about 400us */
    ldr r2, =0x00000190                          @ Set Timer3 Reload = 400 us
    init_delay_timer
delay2_4:
    check_delay_timer
    bne delay2_4
    clear_delay_timer
    /* end delay 400us */

    /* Check DRAM CL Timing by H/W Trapping */
/*  ldr r0, =0x1e6e2070
    ldr r1, [r0]
    bic r1, r1, #0xF9FFFFFF
    mov r2, r1, lsr #21                          @ Set CL
    ldr r1, =0x00000040
    orr r2, r2, r1
*/
    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x00000D63                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000F73
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000F73
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000F73
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000F73
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e0030                          @ REG_EMRS
    ldr r1, =0x00000040                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000040
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000004
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000040
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000040
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS2
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS3
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    /* Delay about 10us */
    ldr r2, =0x0000000A                          @ Set Timer3 Reload = 10 us
    init_delay_timer
delay2_5:
    check_delay_timer
    bne delay2_5
    clear_delay_timer
    /* end delay 10us */

    ldr r0, =0x1e6e000c                          @ Refresh 8 times
    ldr r1, =0x00005C08
    str r1, [r0]

    ldr r0, =0x1e6e002c                          @ REG_MRS
    ldr r1, =0x00000C63                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000E73
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000E73
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000E73
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000E73
#endif
/*  orr r1, r1, r2 */
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set MRS
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030                          @ REG_EMRS
    ldr r1, =0x000003C0                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x000003C0
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x000003C0
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x000003C0
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x000003C0
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030                          @ REG_EMRS
    ldr r1, =0x00000040                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000040
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000004
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000040
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000040
#endif
    str r1, [r0]

    ldr r0, =0x1e6e0028                          @ Set EMRS
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c                          @ Set refresh cycle
    ldr r1, =0x00005A01
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, [r0]
    bic r1, r1, #0xFFF9FFFF
    mov r2, r1, lsr #3                           @ get CL

    ldr r0, =0x1e6e0034                          @ REG_PWC
    ldr r1, =0x00000503                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x00000D03
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x00000D03
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x00000D03
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x00000503
#endif
    orr r1, r1, r2
    str r1, [r0]

/*  ldr r0, =0x1e6e0080
    ldr r2, =0x40000000
Wait_DQI_Delay_Lock_DDR2:
    ldr r1, [r0]
    tst r1, r2                                   @ D[30]=1, locked
    beq Wait_DQI_Delay_Lock_DDR2

    ldr r0, =0x1e6e0020
    ldr r2, =0x00000800
Wait_DQSI_Delay_Lock_DDR2:
    ldr r1, [r0]
    tst r1, r2                                   @ D[11] = 1, locked
    beq Wait_DQSI_Delay_Lock_DDR2
*/
    b   Calibration_Start
.LTORG
/******************************************************************************
 End DDR2 Init
 ******************************************************************************/
Calibration_Start:
/******************************************************************************
 Calibration Code Start
    SRAM buffer definition
    0x1E720000 : Pass 1, DLLI MIN value range
    0x1E720008 : DQS0 DLL valid range, 2nd time CBR
    0x1E72000C : DQS1 DLL valid range, 2nd time CBR
    0x1E720010 : DQ0  DLL valid range, Pass 1
    0x1E720014 : DQ1  DLL valid range, Pass 1
    ....
    0x1E720048 : DQ14 DLL valid range, Pass 1
    0x1E72004C : DQ15 DLL valid range, Pass 1
    0x1E720090 : DLL1 SAdj record
    0x1E720094 : DQL  Pass1 finetune result
    0x1E720098 : DQH  Pass1 finetune result
    0x1E72009C : DRAM initial time, (us)
    0x1E720100 : DQIDLY=00, DLL valid range
    0x1E720104 : DQIDLY=01, DLL valid range
    ....
    0x1E720178 : DQIDLY=30, DLL valid range
    0x1E72017C : DQIDLY=31, DLL valid range
    0x1E720180 : DQSI-MCLK2X P-phase pass record DLL2= 0-31
    0x1E720184 : DQSI-MCLK2X P-phase pass record DLL2=32-63
    0x1E720188 : DQSI-MCLK2X N-phase pass record DLL2= 0-31
    0x1E72018C : DQSI-MCLK2X N-phase pass record DLL2=32-63
 ******************************************************************************/
/* Init SRAM buffer */
    ldr r1, =0x000000ff
    ldr r0, =0x1e720000
    ldr r2, =0x1e7200a0
init_sram_start:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start

    ldr r1, =0x00000000
    ldr r0, =0x1e7200a4
    ldr r2, =0x1e720100
init_sram_start2:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start2

    ldr r1, =0x00ff00ff
    ldr r0, =0x1e720100
    ldr r2, =0x1e720180
init_sram_start3:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start3

    ldr r1, =0x00000000
    ldr r0, =0x1e720180
    ldr r2, =0x1e720200
init_sram_start4:
    str r1, [r0]
    add r0, r0, #4
    cmp r0, r2
    blt init_sram_start4

    ldr r0, =0x1e6e0068                          @ save the DLL1 SAdj initial value
    ldr r1, [r0]
    ldr r0, =0x1e720090
    str r1, [r0]

/* Start
  r0 = free
  r1 = free
  r2 = free
  r3 = free
  r4 = record the return pc value, do not use
  r5 = pattern table index
  r6 = pass count
  r7 = dram DLL2 parameter index (0x1e6e0068), max is 0x4C
*/
/******************************************************************************
 Fine DQI delay and DQSI-MCLK phase
  r8  = DQIDLY count
  r9  = DQSI-MCLK2X phase count
  r10 = pattern fail retry counter, initialize to 2 (fail 2 times)
  r11 = passcnt accumulator for each DQIDLY
 *****************************************************************************/
CBR0_START:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x30                                @ '0'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    str   r1, [r0]

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 1KB
    ldr   r1, =0x000003FF
    str   r1, [r0]

    mov   r8, #0x00                              @ init DQIDLY
    mov   r9, #0x00                              @ init DQSI-MCLK2X phase
    mov   r11, #0x01                             @ init passcnt accumulator

cbr0_next_dqidly:
    cmp   r9, #0x00
    bne   cbr0_next_dqsiphase
    cmp   r11, #0x00
    addeq r8, r8, #0x01                          @ jump 1 stage if no pass at previous stage
    mov   r11, #0x00
    add   r8, r8, #0x01
    cmp   r8, #23                                @ max DQIDLY = 23
    bgt   CBR0_END

    ldr   r0, =0x1e6e000c                        @ disable refresh, wait at least 50ns before enable it again
    ldr   r3, [r0]
    mov   r2, #0x0
    str   r2, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    and   r1, r8, #0x07
    add   r1, r1, #0x30                          @ '0-7'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r8, lsl #16
    str   r1, [r0]
    mov   r9, #0x01                              @ '1':p_phase, '0':n_phase

    ldr   r0, =0x1e6e000c                        @ force load DQIDLY
    ldr   r1, [r0]                               @ dummy read
    str   r3, [r0]
    b     cbr0_dll2_scan_start

cbr0_next_dqsiphase:
    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    orr   r1, r1, r9, lsl #23                    @ set DQSI-MCLK2X phase
    str   r1, [r0]
    mov   r9, #0x00

cbr0_dll2_scan_start:
    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr0_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    str   r7, [r0]
    ldr   r2, =0x40404040                        @ DLL2 max is 0x40404040
    cmp   r7, r2
    bge   cbr0_next_dqidly
    ldr   r2, =0x01010101
    add   r7, r7, r2

/* CBRScan3() start */
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr0_next_test_pattern:
    mov   r10, #2                                @ set the retry loop = 2 of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr0_test_burst

    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add 1 after loop check so we need to decrease 1
    cmp   r3, #0x00
    beq   cbr0_next_dqidly                       @ pass at dlli = 0, invalid
    add   r6, r6, #0x01                          @ increment pass count
    add   r11, r11, #0x01                        @ increment pass count

    ldr   r0, =0x1e720180                        @ record DLL2 pass window
    cmp   r9, #0x00                              @ DQSI-MCLK2X phase check
    addeq r0, r0, #0x08
    cmp   r3, #32
    addge r0, r0, #0x4
    and   r1, r3, #0x1F
    mov   r2, #0x1
    mov   r2, r2, lsl r1
    ldr   r1, [r0]
    orr   r1, r1, r2
    str   r1, [r0]

    ldr   r0, =0x1e720100                        @ record DLL2 min:max value for each DQIDLY
    add   r0, r0, r8, lsl #2
    cmp   r9, #0x00                              @ DQSI-MCLK2X phase check
    beq   cbr0_test_pass_dqsin
    record_dll2_pass_range
    b     cbr0_next_dll2_parameter

cbr0_test_pass_dqsin:
    record_dll2_pass_range_h
    b     cbr0_next_dll2_parameter

cbr0_test_pattern_fail:
    cmp   r6, #5                                 @ passcnt >= 5
    bge   cbr0_next_dqidly
    ldr   r0, =0x1e720100                        @ reset DLL2 min:max value
    add   r0, r0, r8, lsl #2
    ldr   r1, [r0]
    ldr   r2, =0xFFFF0000
    ldr   r3, =0x000000FF
    cmp   r9, #0x00
    moveq r2, r2, lsr #16
    moveq r3, r3, lsl #16
    and   r1, r1, r2
    orr   r1, r1, r3
    str   r1, [r0]
    b     cbr0_next_dll2_parameter               @ CBRScan3() end and test result fail, go to next step

/****************************
 Test fail retry loop
 ***************************/
cbr0_pattern_fail_retry:

/* CBRTest3() start */
cbr0_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x000000C1
    str   r1, [r0]
    ldr   r3, =0x3000
cbr0_wait_engine_idle_0:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr0_wait_engine_idle_0

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    bne   cbr0_test_fail

cbr0_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000085
    str   r1, [r0]
    ldr   r3, =0x3000
cbr0_wait_engine_idle_1:
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr0_wait_engine_idle_1

    ldr   r2, [r0]                               @ read fail bit status
    mov   r1, #0x0
    str   r1, [r0]
    mov   r2, r2, lsr #13                        @ D[13] = fail bit
    cmp   r2, #0x00
    beq   cbr0_test_pass

/* CBRTest3() end */

cbr0_test_fail:
    subs  r10, r10, #1
    bne   cbr0_pattern_fail_retry
    b     cbr0_test_pattern_fail                 @ CBRScan3() return(0)

cbr0_test_pass:
    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr0_next_test_pattern

CBR0_END:
    mov   r5, #0x0                               @ init DQIDLY search count
    mov   r6, #0x0                               @ init g_margin
    mov   r8, #0x0                               @ init g_side
    mov   r7, #0x0                               @ init maximum margin DQIDLY,DQSI-MCLK2X phase
cbr0_search_dll_margin_s:
    ldr   r0, =0x1e720100
    add   r0, r0, r5, lsl #2
    ldr   r1, [r0]
    and   r2, r1, #0xFF                          @ get dllmin_p
    mov   r1, r1, lsr #8
    and   r3, r1, #0xFF                          @ get dllmax_p
    subs  r2, r3, r2                             @ get margin-P
    movmi r2, #0x0
    mov   r1, r1, lsr #8
    and   r3, r1, #0xFF                          @ get dllmin_n
    mov   r1, r1, lsr #8
    and   r1, r1, #0xFF                          @ get dllmax_n
    subs  r3, r1, r3                             @ get margin-N
    movmi r3, #0x0
    add   r1, r2, r3
    cmp   r1, #0x0
    beq   cbr0_search_dll_margin_e               @ if margin-P = 0 && margin-N = 0

    ldr   r9, [r0]
    ldr   r0, =0x1e720180
    cmp   r2, r3
    orrlt r5, r5, #0x80                          @ margin-N > margin-P
    addlt r0, r0, #0x08
    movlt r9, r9, lsr #16
    movge r3, r2                                 @ max(margin-P/N)
    add   r2, r3, #1
    cmp   r2, r6
    blt   cbr0_search_dll_margin_e               @ if max(margin-P/N) + 1 < g_margin

    and   r1, r9, #0xFF                          @ r1 = dlli counter
    cmp   r1, #32
    ldrge r2, [r0, #0x4]                         @ load pass window
    ldrlt r2, [r0]
    and   r1, r1, #0x1F
    mov   r10, #0x1                              @ init test bit mask
    mov   r10, r10, lsl r1
    and   r1, r9, #0xFF
cbr0_search_dllmin_margin_s:
    tst   r2, r10
    beq   cbr0_search_dllmin_margin_e
    mov   r10, r10, lsr #1
    cmp   r1, #32
    ldreq r2, [r0]
    ldreq r10, =0x80000000
    subs  r1, r1, #0x1
    bne   cbr0_search_dllmin_margin_s

cbr0_search_dllmin_margin_e:
    and   r2, r9, #0xFF
    sub   r11, r2, r1                            @ get dllmin side margin

    mov   r9, r9, lsr #8
    and   r1, r9, #0xFF                          @ r1 = dlli counter
    cmp   r1, #32
    ldrge r2, [r0, #0x4]                         @ load pass window
    ldrlt r2, [r0]
    and   r1, r1, #0x1F
    mov   r10, #0x1                              @ init test bit mask
    mov   r10, r10, lsl r1
    and   r1, r9, #0xFF
cbr0_search_dllmax_margin_s:
    tst   r2, r10
    beq   cbr0_search_dllmax_margin_e
    mov   r10, r10, lsl #1
    cmp   r1, #31
    ldreq r2, [r0, #0x4]
    ldreq r10, =0x00000001
    add   r1, r1, #0x1
    cmp   r1, #64
    bne   cbr0_search_dllmax_margin_s

cbr0_search_dllmax_margin_e:
    and   r2, r9, #0xFF
    sub   r1, r1, r2                             @ get dllmax side margin
    cmp   r1, r11
    movlt r11, r1
    subs  r1, r11, r8                            @ side_margin - g_side
    movmi r1, #0x0

cbr0_check_dll_margin:                           @ if max(margin-P/N) > g_margin+1 && (side_margin > g_side || side_margin > 8)
    add   r2, r6, #0x1
    cmp   r3, r2
    ble   cbr0_check_dll_margin2
    cmp   r1, #0
    bgt   cbr0_set_dll_margin
    cmp   r11, #8
    bgt   cbr0_set_dll_margin

cbr0_check_dll_margin2:                          @ if (side_margin - g_side) > 1 && g_side < 8
    cmp   r8, #8
    bgt   cbr0_search_dll_margin_e
    cmp   r1, #1
    ble   cbr0_search_dll_margin_e

cbr0_set_dll_margin:
    cmp   r3, r6
    movgt r6, r3
    mov   r7, r5
    mov   r8, r11

cbr0_search_dll_margin_e:
    and   r5, r5, #0x7F
    add   r5, r5, #0x01
    cmp   r5, #20                                @ last DQIDLY = 20
    blt   cbr0_search_dll_margin_s

    ldr   r0, =0x1e6e000c                        @ disable refresh, wait at least 50ns before enable it again
    ldr   r3, [r0]
    mov   r2, #0x0
    str   r2, [r0]

/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, r7, lsr #4
    and   r1, r1, #0xF
    print_hex_char
    and   r1, r7, #0xF
    print_hex_char
/* Debug - UART console message */

    ldr   r0, =0x1e6e0018
    ldr   r1, [r0]
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r7, lsl #16
    str   r1, [r0]

    ldr   r0, =0x1e6e0068
    mov   r1, #0x0
    str   r1, [r0]

    ldr   r0, =0x1e6e000c                        @ force load DQIDLY
    ldr   r1, [r0]                               @ dummy read
    str   r3, [r0]

    /* Delay about 20us */
    ldr r2, =0x00000014                          @ Set Timer5 Reload = 20 us
    init_delay_timer
delay_5:
    check_delay_timer
    bne delay_5
    clear_delay_timer
    /* end delay 20us */

/******************************************************************************
 Fine tune per bit DQ input delay -- Pass 1, left edge align
  r8  = free
  r9  = DQ fail bit accumulator
  r10 = pattern fail counter, initialize to 5 (fail 5 times)
  r11 = free
 *****************************************************************************/
CBR1_START:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x31                                @ '1'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
/* Debug - UART console message */

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr1_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    str   r7, [r0]
    ldr   r2, =0x40404040                        @ parameter max is to 0x40404040
    cmp   r7, r2
    bge   CBR1_END
    ldr   r2, =0x01010101
    add   r7, r7, r2

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 4KB
    ldr   r1, =0x00000FFF
    str   r1, [r0]

/* CBRScan2() start */
    ldr   r9, =0xFFFF                            @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr1_next_test_pattern:
    mov   r10, #5                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr1_test_single

cbr1_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr1_test_pass_dqi
    cmp   r6, #10
    bge   CBR1_END
    b     cbr1_next_dll2_parameter               @ CBRScan2() end and test result fail, go to next step

cbr1_test_pass_dqi:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add 1 after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count
    ldr   r0, =0x1e720010
    mov   r8, #0x01
cbr1_test_pass_dqi_loop_s:
    tst   r9, r8
    beq   cbr1_test_pass_dqi_loop_e
    record_dll2_pass_range

cbr1_test_pass_dqi_loop_e:
    add   r0, r0, #0x04
    mov   r8, r8, lsl #1
    ldr   r1, =0xFFFF
    tst   r8, r1
    bne   cbr1_test_pass_dqi_loop_s
    b     cbr1_next_dll2_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr1_pattern_fail_retry:

/* CBRTest2() start */
cbr1_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r3, =0x1000
    ldr   r1, =0x1000
cbr1_wait_engine_idle_0:
    subs  r1, r1, #1
    beq   cbr1_test_single_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr1_wait_engine_idle_0

cbr1_test_single_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r11, [r0]
    orr   r11, r11, r11, lsr #16
    bic   r11, r11, #0xFF000000
    bic   r11, r11, #0x00FF0000

    ldr   r1, =0xFFFF
    cmp   r11, r1
    beq   cbr1_test_fail

cbr1_test_burst:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    ldr   r2, =0x00000041
    str   r2, [r0]
    ldr   r3, =0x1000
    ldr   r1, =0x1000
cbr1_wait_engine_idle_1:
    subs  r1, r1, #1
    beq   cbr1_test_burst_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr1_wait_engine_idle_1

cbr1_test_burst_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r2, [r0]
    orr   r2, r2, r2, lsr #16
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r11, r11, r2

    ldr   r2, =0xFFFF
    cmp   r11, r2
    bne   cbr1_test_pass
/* CBRTest2() end */

cbr1_test_fail:
    subs  r10, r10, #1
    bne   cbr1_pattern_fail_retry
    mov   r9, #0x00
    b     cbr1_test_pattern_end                  @ CBRScan2() return(0)

cbr1_test_pass:
    ldr   r1, =0xFFFF                            @ record the pass bit
    eor   r11, r11, r1
    and   r9, r9, r11                            @ DQ pass bit
    cmp   r9, #0x00
    beq   cbr1_test_pattern_end                  @ CBRScan2() return(0)

    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr1_next_test_pattern

CBR1_END:
    mov   r5, #0x0                               @ init DQ DLL_min sum
    mov   r6, #0x0                               @ init DQ DLL_min valid count
    ldr   r0, =0x1e72000c
    ldr   r3, =0x1e720050
cbr1_search_dllmin_s:
    add   r0, r0, #0x04
    cmp   r0, r3
    beq   cbr1_search_dllmin_e
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r2, r2, r1                             @ dllmax - dllmin
    bmi   cbr1_search_dllmin_s                   @ no valid margin found, bypass fine tune
    cmp   r2, #8                                 @ (dllmax - dllmin) < 8
    blt   cbr1_search_dllmin_s                   @ no enough margin found, bypass fine tune
    add   r5, r5, r1
    add   r6, r6, #1
    b     cbr1_search_dllmin_s

cbr1_search_dllmin_e:
    cmp   r6, #16
    bne   init_dram_start                        @ not all bits valid, retry again

    mov   r5, r5, lsr #4
    ldr   r0, =0x1e720000
    str   r5, [r0]

    mov   r6, #0x00                              @ init DQL CBR value
    ldr   r0, =0x1e720030
    ldr   r7, =0x1e72000c
cbr1_set_result_dql:
    sub   r0, r0, #4
    cmp   r0, r7
    beq   cbr1_set_result_next
    mov   r6, r6, lsl #3
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    mov   r3, r1                                 @ dll = dllmin
    cmp   r5, r3
    blt   cbr1_set_result_dql_neg
    sub   r1, r5, r3
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #2                                 @ dqi_tune max = 2
    movgt r1, #2
    orr   r6, r6, r1
    b     cbr1_set_result_dql

cbr1_set_result_dql_neg:
    sub   r1, r3, r5
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #2                                 @ dqi_tune max = -2
    movgt r1, #2
    mov   r2, #8
    sub   r1, r2, r1
    and   r1, r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dql

cbr1_set_result_next:
    ldr   r0, =0x1e6e0080                        @ save DQL fine tune result
    str   r6, [r0]
    ldr   r0, =0x1e720094
    str   r6, [r0]
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r6, r6, lsl #8
    mov   r2, #6
print_dql_result:
    mov   r1, r6, lsr #28
    print_hex_char
    mov   r6, r6, lsl #4
    subs  r2, r2, #1
    bne   print_dql_result
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
/* Debug - UART console message */

    mov   r6, #0x00                              @ init DQH CBR value
    ldr   r0, =0x1e720050
    ldr   r7, =0x1e72002c
cbr1_set_result_dqh:
    sub   r0, r0, #4
    cmp   r0, r7
    beq   cbr1_set_result_end
    mov   r6, r6, lsl #3
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    mov   r3, r1                                 @ dll = dllmin
    cmp   r5, r3
    blt   cbr1_set_result_dqh_neg
    sub   r1, r5, r3
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    cmp   r1, #3                                 @ dqi_tune max = 3
    movgt r1, #3
    subs  r1, r1, #1
    movmi r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dqh

cbr1_set_result_dqh_neg:
    sub   r1, r3, r5
    mov   r2, #19
    mul   r1, r2, r1
    mov   r1, r1, lsr #5                         @ dqi_tune = ((gold_dll - dll) * 19) >> 5
    add   r1, r1, #1
    cmp   r1, #3
    movgt r1, #3
    mov   r2, #8
    sub   r1, r2, r1
    and   r1, r1, #7
    orr   r6, r6, r1
    b     cbr1_set_result_dqh

cbr1_set_result_end:
    ldr   r0, =0x1e6e0084                        @ save DQH fine tune result
    str   r6, [r0]
    ldr   r0, =0x1e720098
    str   r6, [r0]
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r6, r6, lsl #8
    mov   r2, #6
print_dqh_result:
    mov   r1, r6, lsr #28
    print_hex_char
    mov   r6, r6, lsl #4
    subs  r2, r2, #1
    bne   print_dqh_result
/* Debug - UART console message */

    b     CBR3_START

/******************************************************************************
 Search the DLL2 detail margin
 *****************************************************************************/
CBR3_PSTART:
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    ldr   r1, =0x1e720008
    ldr   r6, [r1]
    add   r1, r1, #4
    ldr   r7, [r1]
    mov   r6, r6, lsl #8
    mov   r7, r7, lsl #8
    mov   r2, #6
print_dll0_result_tmp:
    mov   r1, r6, lsr #28
    print_hex_char
    mov   r6, r6, lsl #4
    subs  r2, r2, #1
    bne   print_dll0_result_tmp
    mov   r1, #0x3A                              @ ':'
    str   r1, [r0]
    mov   r2, #6
print_dll1_result_tmp:
    mov   r1, r7, lsr #28
    print_hex_char
    mov   r7, r7, lsl #4
    subs  r2, r2, #1
    bne   print_dll1_result_tmp
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
/* Debug - UART console message */

CBR3_START:
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
    mov r1, #0x43                                @ 'C'
    str r1, [r0]
    mov r1, #0x42                                @ 'B'
    str r1, [r0]
    mov r1, #0x52                                @ 'R'
    str r1, [r0]
    mov r1, #0x32                                @ '2'
    str r1, [r0]
    mov r1, #0x2D                                @ '-'
    str r1, [r0]
/* Debug - UART console message */

    mov   r6, #0x00                              @ init pass count
    mov   r7, #0x00                              @ init DLL2 parameter index
    ldr   r1, =0x000000ff
    ldr   r0, =0x1e720008                        @ init DQL dllmax,dllmin
    str   r1, [r0]
    ldr   r0, =0x1e72000c                        @ init DQH dllmax,dllmin
    str   r1, [r0]

    ldr   r0, =0x1e7200a0                        @ CBR3 iteration counter
    ldr   r1, [r0]
    add   r1, r1, #1
    str   r1, [r0]
    cmp   r1, #0x03
    bgt   init_dram_start

/****************************
 DLL2 delay margin test loop
 ***************************/
cbr3_next_dll2_parameter:
    ldr   r0, =0x1e6e0068                        @ load DLL2 parameter
    str   r7, [r0]
    ldr   r2, =0x40404040                        @ parameter max is to 0x40404040
    cmp   r7, r2
    bge   CBR3_END
    ldr   r2, =0x01010101
    add   r7, r7, r2

    ldr   r0, =0x1e6e0074                        @ set the testing DRAM size = 64KB
    ldr   r1, =0x0000FFFF
    str   r1, [r0]

/* CBRScan() start */
    mov   r9, #0x03                              @ init test status
    adrl  r5, PATTERN_TABLE                      @ init pattern table index
/****************************
 Test pattern iteration loop
 ***************************/
cbr3_next_test_pattern:
    mov   r10, #5                                @ set the retry loop of each pattern
    ldr   r1, [r5]                               @ load test pattern
    ldr   r0, =0x1e6e007c
    str   r1, [r0]
    cmp   r1, #0x00                              @ the last data in pattern is 0x00
    bne   cbr3_test_single

cbr3_test_pattern_end:
    cmp   r9, #0x00
    bne   cbr3_test_pass_dql
    cmp   r6, #10
    bge   CBR3_END
    b     cbr3_next_dll2_parameter               @ CBRScan() end and test result fail, go to next step

cbr3_test_pass_dql:
    and   r3, r7, #0xFF
    sub   r3, r3, #0x01                          @ we add one after loop check so we need to decrease 1
    add   r6, r6, #0x01                          @ increment pass count
    tst   r9, #0x01
    beq   cbr3_test_pass_dqh

    ldr   r0, =0x1E720008
    record_dll2_pass_range

cbr3_test_pass_dqh:
    tst   r9, #0x02
    beq   cbr3_next_dll2_parameter
    ldr   r0, =0x1E72000c
    record_dll2_pass_range
    b     cbr3_next_dll2_parameter

/****************************
 Test fail retry loop
 ***************************/
cbr3_pattern_fail_retry:

/* CBRTest() start */
cbr3_test_single:
    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r1, =0x00000005
    str   r1, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr3_wait_engine_idle_0:
    subs  r8, r8, #1
    beq   cbr3_test_single_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr3_wait_engine_idle_0

cbr3_test_single_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r11, [r0]
    orr   r11, r11, r11, lsr #16
    bic   r11, r11, #0xFF000000
    bic   r11, r11, #0x00FF0000

    ldr   r1, =0xFF
    tst   r11, r1
    beq   cbr3_test_burst
    tst   r11, r1, lsl #8
    bne   cbr3_test_fail

cbr3_test_burst:
    mov   r1, #0x00                              @ initialize loop index, r1 is loop index
cbr3_test_burst_loop:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0x41                          @ test command = 0x41 | (datagen << 3)
    str   r2, [r0]
    ldr   r3, =0x1000
    ldr   r8, =0x10000
cbr3_wait_engine_idle_1:
    subs  r8, r8, #1
    beq   cbr3_test_burst_end
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   cbr3_wait_engine_idle_1

cbr3_test_burst_end:
    ldr   r0, =0x1e6e0078                        @ read fail bit status
    ldr   r2, [r0]
    orr   r2, r2, r2, lsr #16
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r11, r11, r2

    ldr   r2, =0xFF
    tst   r11, r2
    beq   cbr3_next_test_burst_mode
    tst   r11, r2, lsl #8
    beq   cbr3_next_test_burst_mode
/* CBRTest() end */

cbr3_test_fail:
    subs  r10, r10, #1
    bne   cbr3_pattern_fail_retry
    mov   r9, #0x00
    b     cbr3_test_pattern_end                  @ CBRScan() return(0)

cbr3_next_test_burst_mode:
    add   r1, r1, #1                             @ increase the test mode index
    cmp   r1, #0x08                              @ there are 8 modes
    bne   cbr3_test_burst_loop

    ldr   r1, =0xFF                              @ record the pass byte
    tst   r11, r1
    andne r9, r9, #0x02                          @ DQL fail
    tst   r11, r1, lsl #8
    andne r9, r9, #0x01                          @ DQH fail
    cmp   r9, #0x00
    beq   cbr3_test_pattern_end                  @ CBRScan() return(0)

    add   r5, r5, #0x04                          @ increase the test pattern index
    b     cbr3_next_test_pattern

CBR3_END:
    ldr   r0, =0x1e72000c                        @ check DQH margin
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR3_PSTART                            @ no valid margin found, retry again
    cmp   r5, #13                                @ (dllmax - dllmin) < 13
    blt   CBR3_PSTART                            @ no enough margin found, retry again
    mov   r2, #17
    mul   r2, r5, r2
    add   r1, r1, r2, lsr #5                     @ dllmin[1] + ((dllmax[1] - dllmin[1]) * 17) >> 5
    mov   r3, r1, lsl #8
    ldr   r1, [r0]                               @ store the dll search result
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    orr   r1, r1, r3, lsl #8
    str   r1, [r0]
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r6, r1, lsl #8
    mov   r2, #6
print_dll0_result:
    mov   r1, r6, lsr #28
    print_hex_char
    mov   r6, r6, lsl #4
    subs  r2, r2, #1
    bne   print_dll0_result
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e720008                        @ check DQL margin
    ldr   r1, [r0]
    mov   r2, r1, lsr #8
    and   r2, r2, #0xFF                          @ get dllmax
    and   r1, r1, #0xFF                          @ get dllmin
    subs  r5, r2, r1                             @ dllmax - dllmin
    bmi   CBR3_PSTART                            @ no valid margin found, retry again
    cmp   r5, #13                                @ (dllmax - dllmin) < 13
    blt   CBR3_PSTART                            @ no enough margin found, retry again
    mov   r2, #17
    mul   r2, r5, r2
    add   r1, r1, r2, lsr #5                     @ dllmin[0] + ((dllmax[0] - dllmin[0]) * 17) >> 5
    ldr   r2, [r0]                               @ store the dll search result
    bic   r2, r2, #0xFF000000
    bic   r2, r2, #0x00FF0000
    orr   r2, r2, r1, lsl #16
    str   r2, [r0]
    orr   r3, r3, r1
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r6, r2, lsl #8
    mov   r2, #6
print_dll1_result:
    mov   r1, r6, lsr #28
    print_hex_char
    mov   r6, r6, lsl #4
    subs  r2, r2, #1
    bne   print_dll1_result
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]

    ldr   r0, =0x1e720090                        @ load the saved dll1 sadj value
    ldr   r1, [r0]
    bic   r1, r1, #0xFF000000
    bic   r1, r1, #0x00FF0000
    ldr   r0, =0x1e6e0068                        @ save the result dll value
    orr   r1, r1, r3, lsl #16
    str   r1, [r0]

/******************************************************************************
 CBR Finish
 *****************************************************************************/
    ldr r0, =0x1e6e0120                          @ VGA Compatible Mode
    ldr r1, =0x000050C0                          @ 408 MHz
#if defined(CONFIG_DRAM_504)
    ldr r1, =0x000055C0
#elif defined(CONFIG_DRAM_528)
    ldr r1, =0x000055C0
#elif defined(CONFIG_DRAM_552)
    ldr r1, =0x000056C0
#elif defined(CONFIG_DRAM_576)
    ldr r1, =0x000057C0
#endif
    str r1, [r0]

/******************************************************************************
 Calibration Code End
 ******************************************************************************/

set_scratch:
    /*Set Scratch register Bit 6 after ddr initial finished */
/* Not necessary for AST1510
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]
*/
/* Debug - UART console message */
    ldr r0, =0x1e784000
    mov r1, #0x44                                @ 'D'
    str r1, [r0]
    mov r1, #0x6F                                @ 'o'
    str r1, [r0]
    mov r1, #0x6E                                @ 'n'
    str r1, [r0]
    mov r1, #0x65                                @ 'e'
    str r1, [r0]
    mov r1, #0x0D                                @ '\r'
    str r1, [r0]
    mov r1, #0x0A                                @ '\n'
    str r1, [r0]
/* Debug - UART console message */

platform_exit:
#ifdef      CONFIG_DRAM_ECC
    ldr r0, =0x1e6e0004
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x05000000                          /* ECC protected memory size, default set at 80M   */
    str r1, [r0]

    ldr r0, =0x1e6e007C
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000221
    str r1, [r0]

    ldr r0, =0x1e6e0070
    ldr r2, =0x00001000
ECC_Init_Flag:
    ldr r1, [r0]
    tst r1, r2                                   @ D[12] = 1, Done
    beq ECC_Init_Flag

    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x80000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]
#endif

/* Not necessary for AST1510
    ldr r0, =0x1e6e2088
    ldr r1, [r0]
    ldr r2, =0xffefffff
    and r1, r1, r2                               @ A0 workaround for PCIE
    str r1, [r0]
*/
    ldr r0, =0x1e6e2008                          @ Set Video ECLK phase
    ldr r1, [r0]
    ldr r2, =0xfffffff3
    and r1, r1, r2
    orr r1, r1, #0x08
    str r1, [r0]

    ldr r0, =0x1e6e2004
    ldr r1, [r0]
    ldr r2, =0xFFBFFFFF                          @ Enable JTAG Master, solve ARM stucked by JTAG issue
    and r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e6e2048                          @ Set MAC interface delay timing
    ldr r1, =0x2244
    str r1, [r0]

    ldr r0, =0x1e6e2070                          @ Set MAC AHB bus clock
    ldr r1, [r0]
    mov r2, #0x04                                @ Default RMII, set MHCLK = HPLL/10
    tst r1, #0xC0
    movne r2, #0x01                              @ if RGMII, set MHCLK = HPLL/4
    ldr r0, =0x1e6e2008
    ldr r1, [r0]
    bic r1, r1, #0x00070000
    orr r1, r1, r2, lsl #16
    str r1, [r0]

/* Test - DRAM initial time */
    ldr r0, =0x1e782040
    ldr r1, [r0]
    ldr r0, =0xFFFFFFFF
    sub r1, r0, r1
    ldr r0, =0x1e72009c
    str r1, [r0]
    ldr r0, =0x1e782030
    ldr r1, [r0]
    bic r1, r1, #0x0000F000
    str r1, [r0]
/* Test - DRAM initial time */

    /* Disable watchdog */
    ldr r0, =0x1e78500c
    mov r1, #0x0
    str r1, [r0]

    /* Disable unused clock and reset */
    ldr r0, =0x1e6e2004
    ldr r1, [r0]
    ldr r2, =0x00000120
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x1e78200c
    ldr r1, [r0]
    ldr r2, =0x06010130
    orr r1, r1, r2
    str r1, [r0]

    /* restore lr */
    mov lr, r4

    /* back to arch calling code */
    mov pc, lr

