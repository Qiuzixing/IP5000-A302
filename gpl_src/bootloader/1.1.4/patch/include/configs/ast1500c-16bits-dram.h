/*
 * (C) Copyright 2004
 * Peter Chen <peterc@socle-tech.com.tw>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * Version Identity 
 */
#define CONFIG_IDENT_STRING	" ASPEED (v.0.21) "

/*
 * High Level Configuration Options
 * (easy to change)
 */
//#define CONFIG_INIT_CRITICAL			/* define for U-BOOT 1.1.1 */
#undef  CONFIG_INIT_CRITICAL			/* undef for  U-BOOT 1.1.4 */ 
#define CONFIG_ARM926EJS	1		/* This is an arm926ejs CPU */
#define CONFIG_AST1500		1		/* When compiling "Client" this flag SHOULD also enable. */
#define CONFIG_AST1500_CLIENT		1
#define CONFIG_AST1500_SOC_VER		1 //1:AST1500, 2:AST1510, 3:AST1520

/*
** RAM configuration
*/
//#define CONFIG_AST1500_X32_RAM 1  /* undefine it when use x16 RAM */
#define CONFIG_AST1500_RAMSIZE_1Gb 1 /* Total 128MBx2(32bits) or 128MBx1(16bits) */
//#define CONFIG_AST1500_RAMSIZE_512Mb 1 /* Total 64MBx2 or 64MBx1 */
//#define CONFIG_AST1500_RAMSIZE_256Mb 1 /* Total 32MBx2 or 32MBx1 */


#define CONFIG_USE_IRQ				/* we don't need IRQ/FIQ stuff */
#define CONFIG_MISC_INIT_R 

/* 
 * Environment Config
 */
#define CONFIG_CMDLINE_TAG	 1		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1
#define	CONFIG_BOOTARGS 	"root=/dev/ram rw"
#define CONFIG_BOOTDELAY	1		/* autoboot after 1 seconds	*/
#define CONFIG_BOOTCOMMAND	"bootm 14080000 14400000"
#define CONFIG_BOOTFILE		"allc.bin"
#define CONFIG_VERIFY	'n'
#define CONFIG_ENV_OVERWRITE
#define CONFIG_COMMANDS		((CONFIG_CMD_DFL  | CFG_CMD_NET) | CFG_CMD_ENV | CFG_CMD_FLASH | CFG_CMD_PING | CONFIG_CMD_ETHACT | CFG_CMD_PCI | CFG_CMD_I2C | CFG_CMD_EEPROM) & ~CFG_CMD_PCMCIA

/* 
 * CPU Setting
 */
#define CPU_CLOCK_RATE		18000000	/* 16.5 MHz clock for the ARM core */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */

/*
 * Stack sizes,  The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */

/*
 * Memory Configuration
 */
#define CONFIG_NR_DRAM_BANKS	1	   	/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x40000000 	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x4000000 	/* 64 MB */

#define CFG_SDRAM_BASE		0x40000000

/*
 * FLASH Configuration
 */
#define PHYS_FLASH_1		0x14000000 	/* Flash Bank #1 */

#define CFG_FLASH_BASE		PHYS_FLASH_1
#define CFG_FLASH_BANKS_LIST 	{ PHYS_FLASH_1 }

#define CFG_MAX_FLASH_BANKS 	1
#define CFG_MAX_FLASH_SECT	(256)		/* max number of sectors on one chip */

#define CFG_ASTCONFIG_OFFSET		0xFE0000 	/* AST configuration sector starts here  */
#define CFG_ASTCONFIG_SIZE		0x010000 	/* Total Size of AST configuration sector */

#define CFG_ENV_IS_IN_FLASH	1
#define CFG_ENV_OFFSET		0xFF0000 	/* environment starts here  */
#define CFG_ENV_SIZE		0x010000 	/* Total Size of Environment Sector */

#define __LITTLE_ENDIAN
#define CFG_FLASH_SPI

#define CFG_MONITOR_BASE	TEXT_BASE
#define CFG_MONITOR_LEN		(192 << 10)

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(20*CFG_HZ) 	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(20*CFG_HZ) 	/* Timeout for Flash Write */

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory		*/

#define CFG_PROMPT		"boot# " 	/* Monitor Command Prompt	*/
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x40000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x44FFFFFF	/* 256 MB in DRAM	*/

#define	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0x43000000	/* default load address */

#define CFG_TIMERBASE		0x1E782000	/* use timer 1 */
#define CFG_HZ			(1*1000*1000)	/* use external clk (1M) */
#define CONFIG_ZERO_BOOTDELAY_CHECK		/* still check console when bootdelay set to 0 */

/*
 * Serial Configuration
 */
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	4
#define CFG_NS16550_CLK		24000000
#define CFG_NS16550_COM1	0x1e783000
#define CFG_NS16550_COM2	0x1e784000
#define	CFG_LOADS_BAUD_CHANGE
#define CONFIG_SERIAL1		1
#define CONFIG_CONS_INDEX	2
#define CONFIG_BAUDRATE		115200
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * USB device configuration
 */
/*
#define CONFIG_USB_DEVICE	1
#define CONFIG_USB_TTY		1

#define CONFIG_USBD_VENDORID		0x1234
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Siemens"
#define CONFIG_USBD_PRODUCT_NAME	"SX1"
*/

/*
 * I2C configuration
 */
#define CONFIG_HARD_I2C
#define CFG_I2C_SPEED		100000
#define CFG_I2C_SLAVE		1
#define CONFIG_DRIVER_ASPEED_I2C

/*
* EEPROM configuration
*/
#define CFG_I2C_EEPROM_ADDR_LEN 2
#define CFG_I2C_EEPROM_ADDR 0xA0


/*
 * NIC configuration
 */
#define __BYTE_ORDER __LITTLE_ENDIAN
#define __LITTLE_ENDIAN_BITFIELD
#define CONFIG_MAC_PARTITION
#define CONFIG_FARADAYNIC
#define CONFIG_MAC1_PHY_LINK_INTERRUPT
//#define CONFIG_MAC2_ENABLE
//#define CONFIG_MAC2_MII_ENABLE
//#define CONFIG_MAC2_PHY_LINK_INTERRUPT
/*
*-------------------------------------------------------------------------------
* NOTICE: MAC1 and MAC2 now have their own seperate PHY configuration.
* We use 2 bits for each MAC in the scratch register(D[15:11] in 0x1E6E2040) to
* inform kernel driver.
* The meanings of the 2 bits are:
* 00(0): Dedicated PHY
* 01(1): ASPEED's EVA + INTEL's NC-SI PHY chip EVA
* 10(2): ASPEED's MAC is connected to NC-SI PHY chip directly
* 11: Reserved
*
* We use CONFIG_MAC1_PHY_SETTING and CONFIG_MAC2_PHY_SETTING in U-Boot
* 0: Dedicated PHY
* 1: ASPEED's EVA + INTEL's NC-SI PHY chip EVA
* 2: ASPEED's MAC is connected to NC-SI PHY chip directly
* 3: Reserved
*-------------------------------------------------------------------------------
*/
#define CONFIG_MAC1_PHY_SETTING		0
#define CONFIG_MAC2_PHY_SETTING		0
//#define REALTEK_PHY_SUPPORT
#define CONFIG_NET_MULTI
#define CONFIG_ETHACT		faradaynic#0
#define CONFIG_GATEWAYIP	192.168.0.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_IPADDR		192.168.0.188
#define CONFIG_SERVERIP		192.168.0.126
#define CONFIG_ETHADDR		00:C0:A8:12:34:56
#define CONFIG_ETH1ADDR		00:C0:A8:12:34:57
#define CONFIG_SCENARIO		usb_only

/*
 * PCI configuration
 */
#define CONFIG_PCI
#define CONFIG_PCI_HOST PCI_HOST_FORCE
#define CONFIG_PCI_PNP
//#define CONFIG_PCI_SCAN_SHOW

/*
 * SLT
 */
#define CONFIG_SLT
#define CFG_CMD_SLT		(CFG_CMD_REGTEST | CFG_CMD_MACTEST | CFG_CMD_VIDEOTEST | CFG_CMD_HACTEST | CFG_CMD_OSDTEST | CFG_CMD_2DTEST | CFG_CMD_HWCTEST | CFG_CMD_MICTEST)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */
