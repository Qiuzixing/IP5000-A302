/*
 * Board specific setup info
 *
 ******************************************************************************
 * ASPEED Technology Inc.
 * AST1520 DDR3 SDRAM controller initialization sequence
 *
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * Version     : 0
 * Release date: 2014.12.29
 *
 * Optional define variable
 * 1. DRAM Speed             //
 *    CONFIG_DRAM_1333       //
 *    CONFIG_DRAM_1600       // (default)
 *    CONFIG_DRAM_1866       //
 *    CONFIG_DRAM_2133       //
 * 2. UART5 message output   //
 *    CONFIG_DRAM_UART_38400 // set the UART baud rate to 38400, default is 115200
 * 3. Check DRAM init status, skip if ever done
 *    CONFIG_DDRINI_CHECK
 * 4. CPU Speed              //
 *    CONFIG_CPU_600         //
 *    CONFIG_CPU_792         //
 *    CONFIG_CPU_912         // (default)
 *    CONFIG_CPU_1008        //
 *    CONFIG_CPU_1056        //
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
#define ASTMMC_INIT_VER      0x00                @ 8bit verison number
#define ASTMMC_INIT_DATE     0x20141229          @ Release date

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
    .word   0x43403C37       @ 0x010
    .word   0xEE58C476       @ 0x014
    .word   0x00010000       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02101C60       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000020       @ 0x214
    .word   0x02001000       @ 0x2E0
    .word   0x0B00007B       @ 0x2E4
    .word   0x000AD018       @ 0x2E8
    .word   0x1FFFF104       @ 0x2EC
    .word   0x07090407       @ 0x2F0
    .word   0x78000700       @ 0x2F4
    .word   0x0B400800       @ 0x2F8
    .word   0x76573524       @ tRFC
    .word   0x00005B80       @ PLL
TIME_TABLE_DDR3_1600:
    .word   0x54504D38       @ 0x010
    .word   0x0F69F48E       @ 0x014
    .word   0x00000200       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02181E70       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000024       @ 0x214
    .word   0x02001200       @ 0x2E0
    .word   0x0D000094       @ 0x2E4
    .word   0x000D001B       @ 0x2E8
    .word   0x1FFFF105       @ 0x2EC
    .word   0x08090408       @ 0x2F0
    .word   0x90000800       @ 0x2F4
    .word   0x0D400A00       @ 0x2F8
    .word   0x8E69402C       @ tRFC
    .word   0x000071C1       @ PLL
TIME_TABLE_DDR3_1866:
    .word   0x63604F49       @ 0x010
    .word   0x1F7B15A6       @ 0x014
    .word   0x00010300       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02201014       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001500       @ 0x2E0
    .word   0x0F0000AD       @ 0x2E4
    .word   0x000F201E       @ 0x2E8
    .word   0x1FFFF106       @ 0x2EC
    .word   0x090A0407       @ 0x2F0
    .word   0xA8000A00       @ 0x2F4
    .word   0x0F400B00       @ 0x2F8
    .word   0xA67B4B33       @ tRFC
    .word   0x000066A0       @ PLL
TIME_TABLE_DDR3_2133:
    .word   0x7470504A       @ 0x010
    .word   0x3F8C36BD       @ 0x014
    .word   0x00000304       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02281024       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001700       @ 0x2E0
    .word   0x110000C5       @ 0x2E4
    .word   0x00115020       @ 0x2E8
    .word   0x1FFFF107       @ 0x2EC
    .word   0x0A0A0408       @ 0x2F0
    .word   0xC0000B00       @ 0x2F4
    .word   0x11400D00       @ 0x2F8
    .word   0xBD8C563B       @ tRFC
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

    .macro print_hex_dword
    ldr   r0, =0x1e784000
    mov   r1, r2, lsr #28
    print_hex_char
    mov   r1, r2, lsr #24
    print_hex_char
    mov   r1, r2, lsr #20
    print_hex_char
    mov   r1, r2, lsr #16
    print_hex_char
    mov   r1, r2, lsr #12
    print_hex_char
    mov   r1, r2, lsr #8
    print_hex_char
    mov   r1, r2, lsr #4
    print_hex_char
    mov   r1, r2
    print_hex_char
    .endm

    .macro print_hex_word
    ldr   r0, =0x1e784000
    mov   r1, r2, lsr #12
    print_hex_char
    mov   r1, r2, lsr #8
    print_hex_char
    mov   r1, r2, lsr #4
    print_hex_char
    mov   r1, r2
    print_hex_char
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
#if 1
    /* A0 demo board HW board patch Only. To be removed in A1 demo board. */
    /* Board should config HW strapping correctly. Using SW has side effect according to Gary. */
    ldr   r0, =0x1e6e207c
    ldr   r1, =0x01EB3080
    str   r1, [r0]
#endif
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    orr   r1, r1, #0x80
    str   r1, [r0]
    ldr   r0, =0x1e6e2148
    ldr   r1, =0x00190032
    str   r1, [r0]
    ldr   r0, =0x1e6e2024
    ldr   r2, =0x00380000
#if   defined (CONFIG_CPU_600)
    ldr   r1, =0xABC02620                        @ set HPLL =  600 MHz
#elif defined (CONFIG_CPU_792)
    ldr   r1, =0x93000400                        @ set HPLL =  792 MHz
#elif defined (CONFIG_CPU_1008)
    ldr   r1, =0xABC00520                        @ set HPLL = 1008 MHz
#elif defined (CONFIG_CPU_1056)
    ldr   r1, =0xABC00560                        @ set HPLL = 1056 MHz
#else
    ldr   r1, =0x930004A0                        @ set HPLL =  912 MHz
#endif
    orr   r2, r1, r2
    str   r2, [r0]
    ldr   r2, =0x00000100                        @ delay 1 us
wait_hpll_pd:
    subs  r2, r2, #0x01
    bne   wait_hpll_pd
    str   r1, [r0]

#if defined(ASTMMC_HPLL_1056) || defined(ASTMMC_HPLL_1008)
    ldr   r0, =0x1e6e2070                        @ set AXI/AHB = 3/1, when HPLL >= 1000
    ldr   r1, =0x00000400
    str   r1, [r0]
    ldr   r0, =0x1e6e207c
    ldr   r1, =0x00000200
    str   r1, [r0]
#endif

    ldr   r0, =0x1e6e2130
    ldr   r1, =0x00000580
    str   r1, [r0]
    ldr   r0, =0x1e6e2160                        @ set 24M Jitter divider
    ldr   r1, =0x0000AF10
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

    ldr   r0, =0x1e78018c                        @ control GPIOY3 to disable ENTEST rework
    ldr   r1, =0x00000008
    str   r1, [r0]
    ldr   r0, =0x1e7801E4
    ldr   r1, =0x00000008
    str   r1, [r0]
    ldr   r0, =0x1e78012C                        @ control GPIOQ6 to disable GPIOA rework
    ldr   r1, =0x00000040
    str   r1, [r0]
    ldr   r0, =0x1e780084
    ldr   r1, =0x00000040
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
    ldr   r0, =0x1e78502c
    mov   r1, #0
    str   r1, [r0]

/******************************************************************************
 Disable WDT3 for SPI Address mode (3 or 4 bytes) detection function
 ******************************************************************************/
    ldr   r0, =0x1e78504c
    mov   r1, #0
    str   r1, [r0]

#ifdef CONFIG_DDRINI_CHECK
    /* Check Scratch Register Bit 6 */
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    bic   r1, r1, #0xFFFFFFBF
    mov   r2, r1, lsr #6
    cmp   r2, #0x01
    beq   platform_exit
#endif

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
    ldr   r1, =0x00000000                        /* no VGA */
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
/*  bne   ddr4_init */
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
    ldr   r1, =0x00001001
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

    ldr   r0, =0x1e6e0004
    ldr   r1, =0x00000303                        @ Init to 8GB
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

    /********************************************
       PHY Init Process
     *******************************************/
ddr3_phy_init_retry:
    /* Wait DDR PHY init done - timeout 300 ms */
    ldr   r2, =0x000493E0                        @ Set Timer3 Reload = 300 ms
    init_delay_timer
    ldr   r3, =0x1e6e0060
ddr3_phy_init:
    check_delay_timer
    beq   ddr3_phy_init_timeout
    ldr   r1, [r3]
    tst   r1, #0x01
    bne   ddr3_phy_init

    /* Check DDR PHY init status */
    ldr   r0, =0x1e6e0300
    ldr   r2, =0x000A0000
    ldr   r1, [r0]
    tst   r1, r2
    beq   ddr3_phy_init_success

ddr3_phy_init_timeout:
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
ddr3_delay_0:
    check_delay_timer
    bne   ddr3_delay_0
    clear_delay_timer
    /* end delay 10us */

    ldr   r0, =0x1e6e0060                        @ Fire PHY Init
    mov   r1, #0x05
    str   r1, [r0]
    b     ddr3_phy_init_retry

ddr3_phy_init_success:
    clear_delay_timer
    ldr   r0, =0x1e6e0060
    mov   r1, #0x06
    str   r1, [r0]

    /*******************************************/
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x31                              @ '1'
    str   r1, [r0]
/* Debug - UART console message */

    ldr   r0, =0x1e6e000c
    ldr   r1, =0x00000040
    str   r1, [r0]

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

    /*******************************
     Check DRAM Size
     1Gb : 0x80000000 ~ 0x87FFFFFF
     2Gb : 0x80000000 ~ 0x8FFFFFFF
     4Gb : 0x80000000 ~ 0x9FFFFFFF
    *******************************/
    ldr   r0, =0x1e6e0004
    ldr   r6, [r0]
    bic   r6, r6, #0x00000023                    @ record MCR04
    ldr   r7, [r5, #ASTMMC_REGIDX_RFC]

ddr3_check_dram_size:
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    str   r1, [r0]
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    str   r1, [r0]
    ldr   r0, =0x80100000
    ldr   r1, =0x1D1E1F10
    str   r1, [r0]
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 4Gbit
    orreq r6, r6, #0x02
    moveq r7, r7, lsr #16
    beq   ddr3_check_dram_size_end
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 2Gbit
    orreq r6, r6, #0x01
    moveq r7, r7, lsr #8
    beq   ddr3_check_dram_size_end

ddr3_check_dram_size_end:
    ldr   r0, =0x1e6e0004
    str   r6, [r0]
    ldr   r0, =0x1e6e0014
    ldr   r1, [r0]
    bic   r1, r1, #0x000000FF
    and   r7, r7, #0xFF
    orr   r1, r1, r7
    str   r1, [r0]

    b     Calibration_Start
.LTORG
/******************************************************************************
 End DDR3 Init
 ******************************************************************************/

/******************************************************************************
 Other features configuration
 *****************************************************************************/
Calibration_Start:

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

    b     set_scratch

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

   /* Print PHY timing information */
    ldr   r0, =0x1e784014
wait_print_1:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_1

    ldr   r0, =0x1e6e0380
    ldr   r2, [r0]
    print_hex_dword
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]

    ldr   r0, =0x1e784014
wait_print_2:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_2

    ldr   r0, =0x1e6e03A0
    ldr   r2, [r0]
    print_hex_dword
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]

    ldr   r0, =0x1e784014
wait_print_3:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_3

    ldr   r0, =0x1e6e03C0
    ldr   r2, [r0]
    print_hex_word
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    ldr   r0, =0x1e6e03D0
    ldr   r2, [r0]
    print_hex_word
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]

platform_exit:

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

    ldr   r0, =0x1e6e2090                        @ Enable MAC interface pull low
    ldr   r1, [r0]
    bic   r1, r1, #0x0000F000
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

    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr

