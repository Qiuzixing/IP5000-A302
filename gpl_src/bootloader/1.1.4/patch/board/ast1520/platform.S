/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
 * Board specific setup info
 *
 ******************************************************************************
 * ASPEED Technology Inc.
 * AST1520 DDR3 SDRAM controller initialization sequence
 *
 * Gary Hsu, <gary_hsu@aspeedtech.com>
 *
 * Version     : 10
 * Release date: 2017.09.27
 *
 * Optional define variable
 * 1. DRAM Speed             //
 *    CONFIG_DRAM_1333       //
 *    CONFIG_DRAM_1600       // (default)
 *    CONFIG_DRAM_1866       //
 *    CONFIG_DRAM_2133       //
 * 2. UART5 message output   //
 *    CONFIG_DRAM_UART_38400 // set the UART baud rate to 38400, default is 115200
 * 3. Check DRAM init status, skip if it had ever been executed
 *    CONFIG_DDRINI_CHECK
 * 4. CPU Speed              //
 *    CONFIG_CPU_792         // (default)
 *    CONFIG_CPU_912         //
 *    CONFIG_CPU_1008        //
 * 5. Enable DRAM Cache      //
 *    CONFIG_DRAM_CACHE
 *    CONFIG_DRAM_CACHE_SIZE
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
#define ASTMMC_INIT_VER      0x0A                @ 8bit verison number
#define ASTMMC_INIT_DATE     0x20170927          @ Release date

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
    .word   0xE758C476       @ 0x014
    .word   0x00010000       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02101C60       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000020       @ 0x214
    .word   0x02001000       @ 0x2E0
    .word   0x0B00007B       @ 0x2E4
    .word   0x000AD018       @ 0x2E8
    .word   0x29787104       @ 0x2EC
    .word   0x07090407       @ 0x2F0
    .word   0x78000700       @ 0x2F4
    .word   0x0B400800       @ 0x2F8
    .word   0x76573524       @ tRFC
    .word   0x00005B80       @ PLL
TIME_TABLE_DDR3_1600:
    .word   0x54504D38       @ 0x010
    .word   0x0969F48E       @ 0x014
    .word   0x00000200       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02181E70       @ 0x02C
    .word   0x00000040       @ 0x030
    .word   0x00000024       @ 0x214
    .word   0x02001200       @ 0x2E0
    .word   0x0D000094       @ 0x2E4
    .word   0x000D001B       @ 0x2E8
    .word   0x31D08105       @ 0x2EC
    .word   0x08090408       @ 0x2F0
    .word   0x90000800       @ 0x2F4
    .word   0x0D400A00       @ 0x2F8
    .word   0x8E69402C       @ tRFC
    .word   0x000071C1       @ PLL
TIME_TABLE_DDR3_1866:
    .word   0x63604F49       @ 0x010
    .word   0x1A7B15A6       @ 0x014
    .word   0x00010300       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02201014       @ 0x02C
    .word   0x00000042       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001500       @ 0x2E0
    .word   0x0F0000AD       @ 0x2E4
    .word   0x000F201E       @ 0x2E8
    .word   0x3A18A106       @ 0x2EC
    .word   0x090A0407       @ 0x2F0
    .word   0xA8000A00       @ 0x2F4
    .word   0x0F400B00       @ 0x2F8
    .word   0xA67B4B33       @ tRFC
    .word   0x000066A0       @ PLL
TIME_TABLE_DDR3_2133:
    .word   0x7470504A       @ 0x010
    .word   0x3C8C36BD       @ 0x014
    .word   0x00000304       @ 0x018
    .word   0x00000000       @ 0x020
    .word   0x00000000       @ 0x024
    .word   0x02281024       @ 0x02C
    .word   0x00000042       @ 0x030
    .word   0x00000028       @ 0x214
    .word   0x02001700       @ 0x2E0
    .word   0x110000C5       @ 0x2E4
    .word   0x00115020       @ 0x2E8
    .word   0x4260C107       @ 0x2EC
    .word   0x0A0A0408       @ 0x2F0
    .word   0xC0000B00       @ 0x2F4
    .word   0x11400D00       @ 0x2F8
    .word   0xBD8C563B       @ tRFC
    .word   0x00006C20       @ PLL

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

    /* Clear AHB bus lock condition */
    ldr   r0, =0x1e600000
    ldr   r1, =0xAEED1A03
    str   r1, [r0]
    ldr   r0, =0x1e600084
    ldr   r1, =0x00010000
    str   r1, [r0]
    add   r0, r0, #0x4
    mov   r1, #0x0
    str   r1, [r0]

    ldr   r0, =0x1e6e2000
    ldr   r1, =0x1688a8a8
    str   r1, [r0]

    /* Reset again */
    ldr   r0, =0x1e6e2070                        @ check fast reset flag
    ldr   r2, =0x08000000
    ldr   r1, [r0]
    tst   r1, r2
    beq   bypass_first_reset

    ldr   r0, =0x1e785010
    ldr   r3, [r0]
    cmp   r3, #0x0
    beq   start_first_reset
    add   r0, r0, #0x04
    mov   r3, #0x77
    str   r3, [r0]
    ldr   r0, =0x1e720004
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    add   r0, r0, #0x04
    str   r1, [r0]
    ldr   r0, =0x1e6e207c                        @ clear fast reset flag
    str   r2, [r0]
    ldr   r0, =0x1e6e203c                        @ clear watchdog reset flag
    ldr   r1, [r0]
    and   r1, r1, #0x01
    str   r1, [r0]
    ldr   r0, =0x1e78501c                        @ restore normal mask
    ldr   r1, =0x023FFFF3                        @ added 2016.09.06
    str   r1, [r0]
    b     bypass_first_reset

start_first_reset:
    ldr   r0, =0x1e62009c                        @ clear software strap flag for doing again after reset
    ldr   r1, =0xAEEDFC20
    str   r1, [r0]
    ldr   r0, =0x1e785004
    ldr   r1, =0x00000001
    str   r1, [r0]
    ldr   r0, =0x1e785008
    ldr   r1, =0x00004755
    str   r1, [r0]
    ldr   r0, =0x1e78501c                        @ enable full mask
    ldr   r1, =0x03FFFFFF                        @ added 2016.09.06
    str   r1, [r0]
    ldr   r0, =0x1e78500c                        @ enable Full reset
    ldr   r1, =0x00000033
    str   r1, [r0]
wait_first_reset:
    b     wait_first_reset

bypass_first_reset:
    /* Configure USB ports to the correct pin state */
//#ifdef CONFIG_AST1500_CLIENT
    ldr   r0, =0x1e6e2090
    ldr   r1, =0x20000000
    str   r1, [r0]
    ldr   r0, =0x1e6e2094
    ldr   r1, =0x00004000
    str   r1, [r0]
//#else
    ldr   r0, =0x1e6e200c
    ldr   r2, =0x00004000
    ldr   r1, [r0]
    orr   r1, r1, r2
    str   r1, [r0]
//#endif
    ldr   r0, =0x1e6e207c                        @ overwrite strap setting
    ldr   r1, =0x07EBF0A0
    str   r1, [r0]
    ldr   r0, =0x1e6e2070                        @ overwrite strap setting
    ldr   r1, =0x00000040
    str   r1, [r0]

#if defined(CONFIG_CPU_1008)
    ldr   r0, =0x1e6e2070                        @ set AXI/AHB = 3/1, when HPLL >= 1000
    ldr   r1, =0x00000400
    str   r1, [r0]
    ldr   r0, =0x1e6e207c
    ldr   r1, =0x00000200
    str   r1, [r0]
#endif

    /* Set H-PLL */
#if   defined (CONFIG_CPU_1008)
    ldr   r2, =0xABC00520                        @ set HPLL = 1008 MHz
    ldr   r3, =0x00001502
#elif defined (CONFIG_CPU_912)
    ldr   r2, =0x930004A0                        @ set HPLL =  912 MHz
    ldr   r3, =0x00001302
#else
    ldr   r2, =0x93000400                        @ set HPLL =  792 MHz
    ldr   r3, =0x00002104
#endif

    ldr   r0, =0x1e6e2070                        @ Check CLKIN = 25MHz
    ldr   r1, [r0]
    mov   r1, r1, lsr #23
    tst   r1, #0x01
    beq   set_HPLL

#if   defined (CONFIG_CPU_1008)
    ldr   r2, =0xABC004E0                        @ set HPLL = 1000 MHz
    ldr   r3, =0x00007D0C
#elif defined (CONFIG_CPU_912)
    ldr   r2, =0x93000460                        @ set HPLL =  900 MHz
    ldr   r3, =0x00004B08
#else
    ldr   r2, =0x930003E0                        @ set HPLL =  800 MHz
    ldr   r3, =0x00001903
#endif

set_HPLL:
    ldr   r0, =0x1e6e2008                        @ divide HPLL by 16
    ldr   r1, [r0]
    orr   r1, r1, #0xF0
    str   r1, [r0]
    ldr   r0, =0x1e6e2024                        @ set HPLL
    str   r2, [r0]
    ldr   r0, =0x1e6e2148                        @ wait HPLL lock
wait_HPLL_lock:
    ldr   r1, [r0]
    mov   r1, r1, lsr #22
    tst   r1, #0x01
    beq   wait_HPLL_lock
    ldr   r0, =0x1e6e2160                        @ set 24M Jitter divider
    str   r3, [r0]
    ldr   r0, =0x1e6e2008                        @ restore normal HPLL divider
    ldr   r1, [r0]
    bic   r1, r1, #0xF0
    str   r1, [r0]

bypass_config_hpll:
    /* Set SPI to high rate */
    ldr   r0, =0x1e620010
    ldr   r1, =0x000B0D41
    str   r1, [r0]

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

    ldr   r0, =0x1e6e0000
    ldr   r1, =0xFC600309
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
#if defined (CONFIG_DRAM_1333)
    ldr   r2, =0xC48066C0                        @ load PLL parameter for 24Mhz CLKIN (330)
#elif defined (CONFIG_DRAM_1866)
    ldr   r2, =0x930024C0                        @ load PLL parameter for 24Mhz CLKIN (468)
#elif defined (CONFIG_DRAM_2133)
    ldr   r2, =0xABC02560                        @ load PLL parameter for 24Mhz CLKIN (528)
#else
    ldr   r2, =0x93002400                        @ load PLL parameter for 24Mhz CLKIN (396)
#endif

    ldr   r0, =0x1e6e2070                        @ Check CLKIN = 25MHz
    ldr   r1, [r0]
    mov   r1, r1, lsr #23
    tst   r1, #0x01
    beq   set_MPLL
#if defined (CONFIG_DRAM_1333)
    ldr   r2, =0xC4806680                        @ load PLL parameter for 25Mhz CLKIN (331)
#elif defined (CONFIG_DRAM_1866)
    ldr   r2, =0x93002480                        @ load PLL parameter for 25Mhz CLKIN (462)
#elif defined (CONFIG_DRAM_2133)
    ldr   r2, =0xABC02520                        @ load PLL parameter for 25Mhz CLKIN (525)
#else
    ldr   r2, =0x930023E0                        @ load PLL parameter for 25Mhz CLKIN (400)
#endif

set_MPLL:
    ldr   r0, =0x1e6e2020                        @ M-PLL (DDR SDRAM) Frequency
    str   r2, [r0]

    clear_delay_timer

    /* Delay about 3ms */
    ldr   r2, =0x00000BB8                        @ Set Timer3 Reload = 3 ms
    init_delay_timer
wait_mpll_init:
    check_delay_timer
    bne   wait_mpll_init
    clear_delay_timer
    /* end delay 3ms */

    /* Reset MMC */
reset_mmc:
    ldr   r0, =0x1e78505c
    ldr   r1, =0x00000004
    str   r1, [r0]
    ldr   r0, =0x1e785044
    ldr   r1, =0x00000001
    str   r1, [r0]
    ldr   r0, =0x1e785048
    ldr   r1, =0x00004755
    str   r1, [r0]
    ldr   r0, =0x1e78504c
    ldr   r1, =0x00000013
    str   r1, [r0]
wait_mmc_reset:
    ldr   r1, [r0]
    tst   r1, #0x02
    bne   wait_mmc_reset

    ldr   r0, =0x1e78504c
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e78505c
    ldr   r1, =0x023FFFF3
    str   r1, [r0]
    ldr   r0, =0x1e785044
    ldr   r1, =0x000F4240
    str   r1, [r0]
    ldr   r0, =0x1e785048
    ldr   r1, =0x00004755
    str   r1, [r0]
    ldr   r0, =0x1e785054
    ldr   r1, =0x00000077
    str   r1, [r0]

    ldr   r0, =0x1e6e0000
    ldr   r1, =0xFC600309
wait_mmc_reset_done:
    str   r1, [r0]
    ldr   r2, [r0]
    cmp   r2, #0x1
    bne   wait_mmc_reset_done

    ldr   r0, =0x1e6e0034                        @ disable MMC request
    ldr   r1, =0x00020000
    str   r1, [r0]

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
    ldr   r0, =0x1e784014
wait_print:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print
    ldr   r0, =0x1e784000
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
/* Debug - UART console message */

/******************************************************************************
 Enable watchdog to avoid DRAM initial sequence hang
 ******************************************************************************/
    ldr   r0, =0x1e785004
    ldr   r1, =0x000F4240
    str   r1, [r0]
    ldr   r0, =0x1e785008
    ldr   r1, =0x00004755
    str   r1, [r0]
    ldr   r0, =0x1e78500c
    ldr   r1, =0x00000013
    str   r1, [r0]

/******************************************************************************
 Init DRAM common registers
 ******************************************************************************/
    ldr   r0, =0x1e6e0034                        @ disable SDRAM reset
    ldr   r1, =0x00020080
    str   r1, [r0]

    ldr   r0, =0x1e6e0008
    ldr   r1, =0x00000000                        /* no VGA */
    str   r1, [r0]

    ldr   r0, =0x1e6e0038                        @ disable all DRAM requests except CPU during PHY init
    ldr   r1, =0xFFFFEBFF
    str   r1, [r0]

    ldr   r0, =0x1e6e0040
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x88448844
#else
    ldr   r1, =0xE2222222
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e0044
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x24422288
#else
    ldr   r1, =0x24422244
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e0048
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x22222222
#else
    ldr   r1, =0x22622222
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e004c
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x22222222
#else
    ldr   r1, =0x22282222
#endif
    str   r1, [r0]

    ldr   r0, =0x1e6e0050
    ldr   r1, =0x80000000
    str   r1, [r0]

    ldr   r1, =0x00000000
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
/*  ldr   r0, =0x1e6e2070
    ldr   r1, [r0]
    ldr   r2, =0x01000000                        @ bit[24]=1 => DDR4
    tst   r1, r2
    bne   ddr4_init */
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

#if   defined (CONFIG_DRAM_1333)
    adrl  r5, TIME_TABLE_DDR3_1333               @ Init DRAM parameter table
#elif defined (CONFIG_DRAM_1866)
    adrl  r5, TIME_TABLE_DDR3_1866
#elif defined (CONFIG_DRAM_2133)
    adrl  r5, TIME_TABLE_DDR3_2133
#else
    adrl  r5, TIME_TABLE_DDR3_1600
#endif

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
    ldr   r1, =0x55E00B0B
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
    ldr   r1, =0x00020091
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
    ldr   r1, =0x0001AF93
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
 Global Process
 ******************************************************************************/
    /********************************************
     DDRPHY Init Process
    ********************************************/
ddr_phy_init_process:
    clear_delay_timer
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

    b     ddr3_phyinit_done

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
    mov   r3, #0x38                              @ '8'
    beq   check_dram_size_end
    ldr   r0, =0x90100000
    ldr   r1, =0x35363738
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 4Gbit
    orreq r6, r6, #0x02
    moveq r7, r7, lsr #16
    mov   r3, #0x34                              @ '4'
    beq   check_dram_size_end
    ldr   r0, =0x88100000
    ldr   r1, =0x292A2B2C
    ldr   r2, [r0]
    cmp   r2, r1                                 @ == 2Gbit
    orreq r6, r6, #0x01
    moveq r7, r7, lsr #8
    mov   r3, #0x32                              @ '2'
    beq   check_dram_size_end
    mov   r3, #0x31                              @ '1'

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
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    str   r3, [r0]
    mov   r1, #0x47                              @ 'G'
    str   r1, [r0]
    mov   r1, #0x62                              @ 'b'
    str   r1, [r0]
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
/* Debug - UART console message */

#ifdef CONFIG_DRAM_CACHE
    /* Enable DRAM Cache */
    ldr   r0, =0x1e6e0054
    ldr   r1, =CONFIG_DRAM_CACHE_SIZE
    str   r1, [r0]

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
    mov   r2, #3                                 @ 0x3 for range control, and 0x1 no range control
    orr   r1, r1, r2, lsl #10
    str   r1, [r0]
#endif

    /* Set DRAM requests threshold */
    ldr   r0, =0x1e6e001c
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x00000000
#else
    ldr   r1, =0x00000000
#endif
    str   r1, [r0]
    ldr   r0, =0x1e6e0038
#ifdef CONFIG_AST1500_CLIENT
    ldr   r1, =0x00000000
#else
    ldr   r1, =0x00000000
#endif
    str   r1, [r0]

    /********************************************
     DDRTest
    ********************************************/
ddr_test_start:
    ldr   r0, =0x1e6e0074
    ldr   r1, =0x0000FFFF                        @ test size = 64KB
    str   r1, [r0]
    ldr   r0, =0x1e6e007c
    ldr   r1, =0xFF00FF00
    str   r1, [r0]

ddr_test_burst:
    mov   r1, #0x00                              @ initialize loop index, r1 is loop index
ddr_test_burst_loop:
    ldr   r0, =0x1e6e0070
    ldr   r2, =0x00000000
    str   r2, [r0]
    mov   r2, r1, lsl #3
    orr   r2, r2, #0xC1                          @ test command = 0xC1 | (datagen << 3)
    str   r2, [r0]
    ldr   r3, =0x3000
    ldr   r11, =0x20000
ddr_wait_engine_idle_1:
    subs  r11, r11, #1
    beq   ddr_test_fail
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[12] = idle bit
    beq   ddr_wait_engine_idle_1

    ldr   r0, =0x1e6e0070                        @ read fail bit status
    ldr   r3, =0x2000
    ldr   r2, [r0]
    tst   r2, r3                                 @ D[13] = fail bit
    bne   ddr_test_fail

    add   r1, r1, #1                             @ increase the test mode index
    cmp   r1, #0x01                              @ test 1 modes
    bne   ddr_test_burst_loop

    ldr   r0, =0x1e6e0070
    ldr   r1, =0x00000000
    str   r1, [r0]
    b     set_scratch                            @ CBRTest() return(1)

ddr_test_fail:
/* Debug - UART console message */
    ldr   r0, =0x1e784000
    mov   r1, #0x46                              @ 'F'
    str   r1, [r0]
    mov   r1, #0x61                              @ 'a'
    str   r1, [r0]
    mov   r1, #0x69                              @ 'i'
    str   r1, [r0]
    mov   r1, #0x6C                              @ 'l'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]
    ldr   r0, =0x1e784014
wait_print_0:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_0
/* Debug - UART console message */
    b     reset_mmc

set_scratch:
    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr   r0, =0x1e6e2040
    ldr   r1, [r0]
    orr   r1, r1, #0x41
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

    /* Print DQ read margin */
    ldr   r0, =0x1e784014
wait_print_4:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_4

    ldr   r0, =0x1e784000
    mov   r1, #0x52                              @ 'R'
    str   r1, [r0]
    mov   r1, #0x65                              @ 'e'
    str   r1, [r0]
    mov   r1, #0x61                              @ 'a'
    str   r1, [r0]
    mov   r1, #0x64                              @ 'd'
    str   r1, [r0]
    mov   r1, #0x20                              @ ' '
    str   r1, [r0]
    mov   r1, #0x6D                              @ 'm'
    str   r1, [r0]
    mov   r1, #0x61                              @ 'a'
    str   r1, [r0]
    mov   r1, #0x72                              @ 'r'
    str   r1, [r0]
    mov   r1, #0x67                              @ 'g'
    str   r1, [r0]
    mov   r1, #0x69                              @ 'i'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x2D                              @ '-'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x4C                              @ 'L'
    str   r1, [r0]
    mov   r1, #0x3A                              @ ':'
    str   r1, [r0]

    ldr   r0, =0x1e784014
wait_print_5:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_5

    ldr   r7, =0x000001FE                        @ divide by 510
    mov   r8, #10                                @ multiply by 10
print_DQL_eye_margin:
    ldr   r0, =0x1e6e03d0
    ldr   r2, [r0]
    and   r2, r2, #0xFF
    ldr   r0, =0x1e784000
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
    mov   r1, #0x2E                              @ '.'
    str   r1, [r0]
    mov   r3, #0x4                               @ print 4 digits
print_DQL_div_loop:
    mul   r2, r8, r2
    cmp   r2, r7
    blt   print_DQL_div_0
    mov   r6, #0x0
print_DQL_div_digit:
    sub   r2, r2, r7
    add   r6, r6, #0x1
    cmp   r2, r7
    bge   print_DQL_div_digit
    b     print_DQL_div_n

print_DQL_div_0:
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
    b     print_DQL_next
print_DQL_div_n:
    add   r1, r6, #0x30                          @ print n
    str   r1, [r0]
print_DQL_next:
    subs  r3, r3, #1
    beq   print_DQH_eye_margin
    cmp   r2, #0x0
    beq   print_DQH_eye_margin
    b     print_DQL_div_loop

print_DQH_eye_margin:
    mov   r1, #0x2F                              @ '/'
    str   r1, [r0]
    mov   r1, #0x44                              @ 'D'
    str   r1, [r0]
    mov   r1, #0x48                              @ 'H'
    str   r1, [r0]
    mov   r1, #0x3A                              @ ':'
    str   r1, [r0]

    ldr   r0, =0x1e784014
wait_print_6:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_6

    ldr   r0, =0x1e6e03d0
    ldr   r2, [r0]
    mov   r2, r2, lsr #8
    and   r2, r2, #0xFF
    ldr   r0, =0x1e784000
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
    mov   r1, #0x2E                              @ '.'
    str   r1, [r0]
    mov   r3, #0x4                               @ print 4 digits
print_DQH_div_loop:
    mul   r2, r8, r2
    cmp   r2, r7
    blt   print_DQH_div_0
    mov   r6, #0x0
print_DQH_div_digit:
    sub   r2, r2, r7
    add   r6, r6, #0x1
    cmp   r2, r7
    bge   print_DQH_div_digit
    b     print_DQH_div_n

print_DQH_div_0:
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
    b     print_DQH_next
print_DQH_div_n:
    add   r1, r6, #0x30                          @ print n
    str   r1, [r0]
print_DQH_next:
    subs  r3, r3, #1
    beq   print_DQ_eye_margin_last
    cmp   r2, #0x0
    beq   print_DQ_eye_margin_last
    b     print_DQH_div_loop

print_DQ_eye_margin_last:
    mov   r1, #0x20                              @ ' '
    str   r1, [r0]
    mov   r1, #0x43                              @ 'C'
    str   r1, [r0]
    mov   r1, #0x4B                              @ 'K'
    str   r1, [r0]

    ldr   r0, =0x1e784014
wait_print_7:
    ldr   r1, [r0]
    tst   r1, #0x40
    beq   wait_print_7

    ldr   r0, =0x1e784000
    mov   r1, #0x20                              @ ' '
    str   r1, [r0]
    mov   r1, #0x28                              @ '('
    str   r1, [r0]
    mov   r1, #0x6D                              @ 'm'
    str   r1, [r0]
    mov   r1, #0x69                              @ 'i'
    str   r1, [r0]
    mov   r1, #0x6E                              @ 'n'
    str   r1, [r0]
    mov   r1, #0x3A                              @ ':'
    str   r1, [r0]
    mov   r1, #0x30                              @ '0'
    str   r1, [r0]
    mov   r1, #0x2E                              @ '.'
    str   r1, [r0]
    mov   r1, #0x33                              @ '3'
    str   r1, [r0]
    mov   r1, #0x35                              @ '5'
    str   r1, [r0]
    mov   r1, #0x29                              @ ')'
    str   r1, [r0]
    mov   r1, #0x0D                              @ '\r'
    str   r1, [r0]
    mov   r1, #0x0A                              @ '\n'
    str   r1, [r0]

platform_exit:

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

    ldr   r6, =0x000F7E60                        @ Init spiclk loop
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

/******************************************************************************
 Miscellaneous Setting
 ******************************************************************************/
    /* Disable no use clock */
    ldr   r0, =0x1e6e200c
    ldr   r1, [r0]
    ldr   r2, =0x10080110
    orr   r1, r1, r2
    str   r1, [r0]

    ldr   r0, =0x1e6e2004                        @ Enable JTAG Master, solve ARM stucked by JTAG issue
    ldr   r1, [r0]
    bic   r1, r1, #0x00400000
    str   r1, [r0]

    ldr   r0, =0x1e6e2048                        @ Set MAC interface delay timing = 1G
    ldr   r1, =0x00082208
    str   r1, [r0]
    ldr   r0, =0x1e6e20b8                        @ Set MAC interface delay timing = 100M
    str   r1, [r0]
    ldr   r0, =0x1e6e20bc                        @ Set MAC interface delay timing = 10M
    str   r1, [r0]

    mov   r2, #0x02                              @ Default set MHCLK = HPLL/6
    ldr   r0, =0x1e6e2008
    ldr   r1, [r0]
    bic   r1, r1, #0x00070000
    orr   r1, r1, r2, lsl #16
    str   r1, [r0]

    ldr   r0, =0x1e6e21dc                        @ Set MAC duty
    ldr   r1, =0x00666E00
    str   r1, [r0]

    ldr   r0, =0x1e6e2090                        @ Enable MAC interface pull low
    ldr   r1, [r0]
    bic   r1, r1, #0x0000F000
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

    /* Disable watchdog */
    ldr   r0, =0x1e78500c
    ldr   r1, =0x00000000
    str   r1, [r0]
    ldr   r0, =0x1e785004
    ldr   r1, =0xFFFFFFFF
    str   r1, [r0]
    ldr   r0, =0x1e785008
    ldr   r1, =0x00004755
    str   r1, [r0]

    /* restore lr */
    mov   lr, r4

    /* back to arch calling code */
    mov   pc, lr
