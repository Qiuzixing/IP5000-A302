/*
 * (C) Copyright 2002-2004
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
 *
 * Copyright (C) 2003 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 * Modified to work with AMD flashes
 *
 * Copyright (C) 2004
 * Ed Okerson
 * Modified to work with little-endian systems.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * History
 * 01/20/2004 - combined variants of original driver.
 * 01/22/2004 - Write performance enhancements for parallel chips (Tolunay)
 * 01/23/2004 - Support for x8/x16 chips (Rune Raknerud)
 * 01/27/2004 - Little endian support Ed Okerson
 *
 * Tested Architectures
 * Port Width  Chip Width    # of banks	   Flash Chip  Board
 * 32	       16	     1		   28F128J3    seranoa/eagle
 * 64	       16	     1		   28F128J3    seranoa/falcon
 *
 */

/* The DEBUG define must be before common to enable debugging */
/* #define DEBUG	*/

#include <common.h>
#include <asm/processor.h>
#include <asm/byteorder.h>
#include <environment.h>
#ifdef	CFG_FLASH_SPI

/*
 * This file implements a Common Flash Interface (CFI) driver for U-Boot.
 * The width of the port and the width of the chips are determined at initialization.
 * These widths are used to calculate the address for access CFI data structures.
 * It has been tested on an Intel Strataflash implementation and AMD 29F016D.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 *
 * TODO
 *
 * Use Primary Extended Query table (PRI) and Alternate Algorithm Query
 * Table (ALT) to determine if protection is available
 *
 * Add support for other command sets Use the PRI and ALT to determine command set
 * Verify erase and program timeouts.
 */

#ifndef CFG_FLASH_BANKS_LIST
#define CFG_FLASH_BANKS_LIST { CFG_FLASH_BASE }
#endif

/* use CFG_MAX_FLASH_BANKS_DETECT if defined */
#ifdef CFG_MAX_FLASH_BANKS_DETECT
static ulong bank_base[CFG_MAX_FLASH_BANKS_DETECT] = CFG_FLASH_BANKS_LIST;
flash_info_t flash_info[CFG_MAX_FLASH_BANKS_DETECT];	/* FLASH chips info */
#else
static ulong bank_base[CFG_MAX_FLASH_BANKS] = CFG_FLASH_BANKS_LIST;
flash_info_t flash_info[CFG_MAX_FLASH_BANKS];		/* FLASH chips info */
#endif

/* Support Flash ID */
#define STM25P64		0x172020
#define STM25P128		0x182020
#define N25Q256			0x19ba20
#define N25Q512			0x20ba20
#define S25FL064A		0x160201
#define S25FL128P		0x182001
#define S25FL256S		0x190201
#define W25X16			0x1530ef
#define W25X64			0x1730ef
#define W25Q64BV		0x1740ef
#define W25Q128BV		0x1840ef
#define W25Q256FV		0x1940ef
#define MX25L1605D		0x1520C2
#define MX25L12805D		0x1820C2
#define MX25L25635E		0x1920C2
#define MX66L1G45G		0x1b20C2
#define SST25VF016B		0x4125bf
#define SST25VF064C		0x4b25bf
#define SST25VF040B		0x8d25bf
#define AT25DF161		0x02461F
#define AT25DF321		0x01471F

/* SPI Define */
#if	(CONFIG_AST1500_SOC_VER >= 3)
#define STCBaseAddress			0x1e620000
#define SCU_REVISION_REGISTER		0x1e6e207c
#define SCU_CACHE_CTRL_REGISTER		0x1e6e2118

#define SPICtrlRegOffset		0x10
#define SPICtrlRegOffset2		0x14

#define SPIMiscCtrlRegOffset		0x54

/* for DMA */
#define REG_FLASH_INTERRUPT_STATUS	0x08
#define REG_FLASH_DMA_CONTROL		0x80
#define REG_FLASH_DMA_FLASH_BASE	0x84
#define REG_FLASH_DMA_DRAM_BASE		0x88
#define REG_FLASH_DMA_LENGTH		0x8c

#define FLASH_STATUS_DMA_BUSY		0x0000
#define FLASH_STATUS_DMA_READY		0x0800
#define FLASH_STATUS_DMA_CLEAR		0x0800

#define FLASH_DMA_ENABLE		0x01

#else	/* #if	(CONFIG_AST1500_SOC_VER >= 3) */

#if (CONFIG_AST1500_SOC_VER == 2)
	#define SCU_REVISION_REGISTER		0x1e6e207c
#endif

#define STCBaseAddress		0x16000000
#define SPICtrlRegOffset	0x04
#define SPICtrlRegOffset2	0x0C
#endif	/* #if	(CONFIG_AST1500_SOC_VER >= 3) */

#define CMD_MASK		0xFFFFFFF8

#define NORMALREAD		0x00
#define	FASTREAD		0x01
#define NORMALWRITE		0x02
#define USERMODE		0x03

#define CE_LOW			0x00
#define CE_HIGH			0x04

/* (CONFIG_AST1500_SOC_VER >= 2) only */
/* For SPI supporting more than 1 bit data pins.*/
#define IOMODEx1		0x00000000
#define IOMODEx2		0x20000000
#define IOMODEx2_dummy		0x30000000
#define IOMODEx4		0x40000000
#define IOMODEx4_dummy		0x50000000

#define DUMMY_COMMAND_OUT	0x00008000
/* (CONFIG_AST1500_SOC_VER >= 2) only */

/* specificspi */
#define SpecificSPI_N25Q512	0x00000001		

static ulong AST2300_SPICLK_DIV[16] = {0x0F, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04, \
                                       0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x00 };

/* udelay */
#ifdef CONFIG_FLASH_SKIP_UDELAY
#define flash_udelay(x)	;
#else
#define flash_udelay(x)	udelay(x);
#endif
                              
/*-----------------------------------------------------------------------
 * Functions
 */
static void reset_flash (flash_info_t * info);
static void enable_write (flash_info_t * info);
static void write_status_register (flash_info_t * info, uchar data);
/* enable 4 bytes mode addressing space. */
static void enable4b (flash_info_t * info);
static void enable4b_spansion (flash_info_t * info);
static void enable4b_numonyx (flash_info_t * info);

static ulong flash_get_size (ulong base, int banknum);
static int flash_write_buffer (flash_info_t *info, uchar *src, ulong addr, int len);
#if defined(CFG_ENV_IS_IN_FLASH) || defined(CFG_ENV_ADDR_REDUND) || (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base);
#endif


/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
inline uchar *flash_make_addr (flash_info_t * info, flash_sect_t sect, uint offset)
{
#ifdef CONFIG_2SPIFLASH
        if (info->start[0] >= PHYS_FLASH_2)
	    return ((uchar *) (info->start[sect] + (offset * 1) - (PHYS_FLASH_2 - PHYS_FLASH_2_BASE) ));        
        else
	    return ((uchar *) (info->start[sect] + (offset * 1)));
#else
	return ((uchar *) (info->start[sect] + (offset * 1)));
#endif	
}

/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
inline uchar flash_read_uchar (flash_info_t * info, uint offset)
{
	uchar *cp;

	cp = flash_make_addr (info, 0, offset);
#if defined(__LITTLE_ENDIAN)
	return (cp[0]);
#else
	return (cp[1 - 1]);
#endif
}

/*-----------------------------------------------------------------------
 * read a short word by swapping for ppc format.
 */
ushort flash_read_ushort (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ushort retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG
	debug ("ushort addr is at %p 1 = %d\n", addr,
	       1);
	for (x = 0; x < 2 * 1; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	retval = ((addr[(1)] << 8) | addr[0]);
#else
	retval = ((addr[(2 * 1) - 1] << 8) |
		  addr[1 - 1]);
#endif

	debug ("retval = 0x%x\n", retval);
	return retval;
}

/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maiximum
 * port size word. Swap for ppc format.
 */
ulong flash_read_long (flash_info_t * info, flash_sect_t sect, uint offset)
{
	uchar *addr;
	ulong retval;

#ifdef DEBUG
	int x;
#endif
	addr = flash_make_addr (info, sect, offset);

#ifdef DEBUG
	debug ("long addr is at %p 1 = %d\n", addr,
	       1);
	for (x = 0; x < 4 * 1; x++) {
		debug ("addr[%x] = 0x%x\n", x, addr[x]);
	}
#endif
#if defined(__LITTLE_ENDIAN)
	retval = (addr[0] << 16) | (addr[(1)] << 24) |
		(addr[(2 * 1)]) | (addr[(3 * 1)] << 8);
#else
	retval = (addr[(2 * 1) - 1] << 24) |
		(addr[(1) - 1] << 16) |
		(addr[(4 * 1) - 1] << 8) |
		addr[(3 * 1) - 1];
#endif
	return retval;
}

/*-----------------------------------------------------------------------
 */
static void reset_flash (flash_info_t * info)
{
	ulong ulCtrlData, CtrlOffset;

	if (info->CE == 2) {
		CtrlOffset = SPICtrlRegOffset2;
	} else {
		CtrlOffset = SPICtrlRegOffset;
	}

#if (CONFIG_AST1500_SOC_VER >= 3)
	ulCtrlData = info->iomode | (info->readcmd << 16) | (info->tCK_Read << 8) | (info->dummybyte << 6) | FASTREAD;
#else
	ulCtrlData  = (info->readcmd << 16) | (info->tCK_Read << 8) | (info->dummybyte << 6) | FASTREAD;
	if (info->dualport)
		ulCtrlData  |= 0x08;
#endif
	*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
}

static void enable_write (flash_info_t * info)
{
	ulong base;
        ulong ulCtrlData, CtrlOffset;
        uchar jReg;
        
        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
                  
	//base = info->start[0];
	base = flash_make_addr (info, 0, 0);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x06);
        flash_udelay(10);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        flash_udelay(10);
        do {                                
            jReg = *(volatile uchar *) (base);
        } while (!(jReg & 0x02));                  	                
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

}
	
static void write_status_register (flash_info_t * info, uchar data)
{
	ulong base;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;
        
        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
    
	//base = info->start[0];
	base = flash_make_addr (info, 0, 0);

        enable_write (info);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x01);
        flash_udelay(10);
        *(uchar *) (base) = (uchar) (data);                
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        flash_udelay(10);
        do {                                
            jReg = *(volatile uchar *) (base);
        } while (jReg & 0x01);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

}

static void enable4b (flash_info_t * info)
{
        ulong base;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;

        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
    
        //base = info->start[0];
        base = flash_make_addr (info, 0, 0);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0xb7);
        ulCtrlData &= CMD_MASK;                          	                
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
 
} /* enable4b */

static void enable4b_spansion (flash_info_t * info)
{
        ulong base;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;
        
        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }

        //base = info->start[0];
        base = flash_make_addr (info, 0, 0);

        /* Enable 4B: BAR0 D[7] = 1 */
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x17);
        flash_udelay(10);
        *(uchar *) (base) = (uchar) (0x80);    
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x16);
        flash_udelay(10);
        do {
            jReg = *(volatile uchar *) (base);
        } while (!(jReg & 0x80));
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

} /* enable4b_spansion */

static void enable4b_numonyx (flash_info_t * info)
{
        ulong base;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;

        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }
    
        //base = info->start[0];
        base = flash_make_addr (info, 0, 0);

        /* Enable Write */
        enable_write (info);

        /* Enable 4B: CMD:0xB7 */
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0xB7);
        flash_udelay(10);
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

} /* enable4b_numonyx */

/*
** get_clk_src_Hz - return the value of CLKIN. 24MHz or 25MHz.
*/
static unsigned int get_clk_src_Hz(void)
{
	unsigned int scu70;

	scu70 = *((volatile ulong*)0x1e6e2070);
	if (scu70 & (1 << 23)) {
		return 25000000;
	}
	return 24000000;
}

/*
** get_HPLL_Hz - return H-PLL value in Hz.
*/
static unsigned int get_HPLL_Hz(void)
{
	unsigned int scu24, clk;

	scu24 = *((volatile ulong*)0x1e6e2024);

	//If HPLL is turned off
	if (scu24 & (1 << 19))
		return 0;

	clk = get_clk_src_Hz();
	//If HPLL bypass mode is enabled.
	if (scu24 & (1 << 20))
		return clk;
	{
		unsigned int P, M, N;
		//P = SCU24[18:13]
		//M = SCU24[12:5]
		//N = SCU24[4:0]
		//hpll = clk * [(M+1) /(N+1)] / (P+1)
		P = (scu24 >> 13) & 0x3F;
		M = (scu24 >> 5) & 0xFF;
		N = scu24 & 0x1F;
		return (clk / 1000 * ((M + 1) / (N + 1)) / (P + 1) * 1000);
	}
}

/*
** get_HCLK_MHz - return H-CLK (AHB clock) value in MHz
*/
static unsigned int get_HCLK_MHz(void)
{
	unsigned int div, hpll;
	unsigned int scu70 = *((volatile ulong*) 0x1e6e2070);
	unsigned int cpu2axi, axi2ahb;

	hpll = get_HPLL_Hz();
#if	(CONFIG_AST1500_SOC_VER >= 3)
	/* AST152x always 2 */
	cpu2axi = 2;
#else
	cpu2axi = ((scu70 >> 8) & 1) + 1;
#endif
	axi2ahb = ((scu70 >> 9) & 0x7) + 1;

	printf("HPLL=%d MHz, AXI=%d MHz, HCLK(AHB)=%d MHz, (%d:%d:1)\n",
	       hpll/1000000,
	       hpll/cpu2axi/1000000,
	       hpll/cpu2axi/axi2ahb/1000000,
	       cpu2axi, axi2ahb);

	return hpll/cpu2axi/axi2ahb/1000000;
}

static ulong flash_get_size (ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int j;
	unsigned long sector;
	int erase_region_size;
	ulong ulCtrlData, CtrlOffset;
	ulong ulID;
	uchar ch[3];
	ulong cpuclk, div, reg;
	ulong WriteClk, EraseClk, ReadClk;
	ulong vbase;
	ulong SCURevision;

	info->start[0] = base;
	vbase = flash_make_addr (info, 0, 0);

#if	(CONFIG_AST1500_SOC_VER >= 3)
	CtrlOffset = SPICtrlRegOffset;
	info->CE = 0;
#else
	if (vbase == PHYS_FLASH_1)
	{
		CtrlOffset = SPICtrlRegOffset2;
		info->CE = 2;
	}
	else
	{
		CtrlOffset = SPICtrlRegOffset;
		info->CE = 0;
	}
#endif

	/* Get Flash ID */
	ulCtrlData  = *(ulong *) (STCBaseAddress + CtrlOffset) & CMD_MASK;
	ulCtrlData |= CE_LOW | USERMODE;
	*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
	flash_udelay(200);
	*(uchar *) (vbase) = (uchar) (0x9F);
	flash_udelay(10);
	ch[0] = *(volatile uchar *)(vbase);
	flash_udelay(10);
	ch[1] = *(volatile uchar *)(vbase);
	flash_udelay(10);
	ch[2] = *(volatile uchar *)(vbase);
	flash_udelay(10);
	ulCtrlData  = *(ulong *) (STCBaseAddress + CtrlOffset) & CMD_MASK;
	ulCtrlData |= CE_HIGH | USERMODE;
	*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
	flash_udelay(200);
	ulID = ((ulong)ch[0]) | ((ulong)ch[1] << 8) | ((ulong)ch[2] << 16) ;
	info->flash_id = ulID;

	printf("SPI Flash ID: 0x%06X \n", ulID);

	/* init default */
	info->iomode = IOMODEx1;
	info->address32 = 0;
	info->quadport = 0;
	info->specificspi = 0;

	switch (info->flash_id)
	{
	case STM25P64:
		info->sector_count = 128;
		info->size = 0x800000;
		erase_region_size = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 40;
		EraseClk = 20;
		ReadClk  = 40;
		break;

	case STM25P128:
		info->sector_count = 64;
		info->size = 0x1000000;
		erase_region_size  = 0x40000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
		break;

	case N25Q256:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 512;
		info->size = 0x2000000;
		info->address32 = 1;
#endif
		break;

	case N25Q512:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256; 
		info->specificspi = SpecificSPI_N25Q512;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 1024;
		info->size = 0x4000000;
		info->address32 = 1;
#endif
		break;
	case W25X16:
		info->sector_count = 32;
		info->size = 0x200000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x3b;
		info->dualport = 1;
		info->dummybyte = 1;
		info->iomode = IOMODEx2;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;

	case W25X64:
		info->sector_count = 128;
		info->size = 0x800000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x3b;
		info->dualport = 1;
		info->dummybyte = 1;
		info->iomode = IOMODEx2;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;

	case W25Q64BV:
		info->sector_count = 128;
		info->size = 0x800000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x3b;
		info->dualport = 1;
		info->dummybyte = 1;
		info->iomode = IOMODEx2; 
		info->buffersize = 256;
		WriteClk = 80;
		EraseClk = 40;
		ReadClk  = 80;
		break;

	case W25Q128BV:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x3b;
		info->dualport = 1;
		info->dummybyte = 1;
		info->iomode = IOMODEx2; 
		info->buffersize = 256;
		WriteClk = 104;
		EraseClk = 50;
		ReadClk  = 104;
		break;

	case W25Q256FV:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 512;
		info->size = 0x2000000;
		info->address32 = 1;
#endif
		break;
	case S25FL064A:
		info->sector_count = 128;
		info->size = 0x800000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;            
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;

	case S25FL128P:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 100;
		EraseClk = 40;
		ReadClk  = 100;
		break;

	case S25FL256S:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 512;
		info->size = 0x2000000;
		info->address32 = 1;
#endif
		break;
	case MX66L1G45G:
		info->sector_count = 256;
		info->size = 16 * 1024 * 1024;
		erase_region_size  = 64 * 1024;
		info->readcmd = 0x03;
		info->dualport = 0;
		info->dummybyte = 0;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 2048;
		info->size = 128 * 1024 * 1024;
		info->address32 = 1;
#endif
		break;

	case MX25L25635E:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER >= 3)
		info->sector_count = 512;
		info->size = 0x2000000;
		info->address32 = 1;
#if defined(CONFIG_FLASH_SPIx2_Dummy)
		info->readcmd = 0xbb;
		info->dummybyte = 1;
		info->dualport = 1;
		info->iomode = IOMODEx2_dummy;
#elif defined(CONFIG_FLASH_SPIx4_Dummy)
		info->readcmd = 0xeb;
		info->dummybyte = 3;
		info->dualport = 0;
		info->iomode = IOMODEx4_dummy;
		info->quadport = 1;
		info->dummydata = 0xaa;
#endif
#endif
		break;

	case MX25L12805D:
		info->sector_count = 256;
		info->size = 0x1000000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		/*
		SCU7C: Silicon Revision ID Register
		D[31:24]: Chip ID
		0: AST1500
		1: AST1510

		D[23:16] Silicon revision ID for AST1510 generation and later
		0: A0
		1: A1
		2: A2
		.
		.
		.
		FPGA revision starts from 0x80

		AST1510 A0 SPI can't run faster than 50Mhz
		*/
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
#if (CONFIG_AST1500_SOC_VER == 2)
		SCURevision = *(ulong *) (SCU_REVISION_REGISTER);
		if (((SCURevision >> 24) & 0xff) == 0x01) { //AST2300
			if (((SCURevision >> 16) & 0xff) == 0x00) { //A0
				WriteClk = 25;
				EraseClk = 20;
				ReadClk  = 25;
			}
		}
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
#if defined(CONFIG_FLASH_SPIx2_Dummy)
		info->readcmd = 0xbb;
		info->dummybyte = 1;
		info->dualport = 1;
		info->iomode = IOMODEx2_dummy;
#elif defined(CONFIG_FLASH_SPIx4_Dummy)
		info->readcmd = 0xeb;
		info->dummybyte = 3;
		info->dualport = 0;
		info->iomode = IOMODEx4_dummy;
		info->quadport = 1;
		info->dummydata = 0xaa;
#endif
#endif
		break;

	case MX25L1605D:
		info->sector_count = 32;
		info->size = 0x200000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 256;
		WriteClk = 50;
		EraseClk = 20;
		ReadClk  = 50;
		break;

	case SST25VF016B:
		info->sector_count = 32;
		info->size = 0x200000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 1;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;

	case SST25VF064C:
		info->sector_count = 128;
		info->size = 0x800000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 1;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;

	case SST25VF040B:
		info->sector_count = 8;
		info->size = 0x80000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 1;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;


	case AT25DF161:
		info->sector_count = 32;
		info->size = 0x200000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 1;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;
 
	case AT25DF321:
		info->sector_count = 32;
		info->size = 0x400000;
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;
		info->dualport = 0;
		info->dummybyte = 1;
		info->buffersize = 1;
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;
		break;
	default:	/* use JEDEC ID */
		printf("Unsupported SPI Flash!! 0x%08X\n", info->flash_id);
		erase_region_size  = 0x10000;
		info->readcmd = 0x0b;                        
		info->dualport  = 0;            
		info->dummybyte = 1;
		info->buffersize = 1;            
		WriteClk = 50;
		EraseClk = 25;
		ReadClk  = 50;  
		if ((info->flash_id & 0xFF) == 0x1F)	/* Atmel */
		{
			switch (info->flash_id & 0x001F00)
			{
			case 0x000400:
				info->sector_count = 8;
				info->size = 0x80000;
				break;
			case 0x000500:
				info->sector_count = 16;
				info->size = 0x100000;
				break;	            	                
			case 0x000600:
				info->sector_count = 32;
				info->size = 0x200000;
				break;
			case 0x000700:
				info->sector_count = 64;
				info->size = 0x400000;
				break;
			case 0x000800:
				info->sector_count = 128;
				info->size = 0x800000;
				break;
			case 0x000900:
				info->sector_count = 256;
				info->size = 0x1000000;
				break;
			default:
				printf("Can't support this SPI Flash!! \n");
				return 0;
			}
		} /* Atmel JDEC */        
		else	/* JDEC */
		{
			switch (info->flash_id & 0xFF0000)
			{
			case 0x120000:	
				info->sector_count = 4;
				info->size = 0x40000;
				break;
			case 0x130000:	
				info->sector_count = 8;
				info->size = 0x80000;
				break;
			case 0x140000:	
				info->sector_count =16;
				info->size = 0x100000;
				break;
			case 0x150000:	
				info->sector_count =32;
				info->size = 0x200000;
				break;
			case 0x160000:	
				info->sector_count =64;
				info->size = 0x400000;
				break;
			case 0x170000:	
				info->sector_count =128;
				info->size = 0x800000;
				break;
			case 0x180000:	
				info->sector_count =256;
				info->size = 0x1000000;
				break;
			case 0x190000:	
				info->sector_count =256;
				info->size = 0x1000000;
#if (CONFIG_AST1500_SOC_VER >= 3)
				info->sector_count = 512;
				info->size = 0x2000000;
				info->address32 = 1;
#if defined(CONFIG_FLASH_SPIx2_Dummy)
				info->readcmd = 0xbb;
				info->dummybyte = 1;
				info->dualport = 1;
				info->iomode = IOMODEx2_dummy;
#elif defined(CONFIG_FLASH_SPIx4_Dummy)
				info->readcmd = 0xeb;
				info->dummybyte = 3;
				info->dualport = 0;
				info->iomode = IOMODEx4_dummy;
				info->quadport = 1;
				info->dummydata = 0xaa;
#endif
#endif
				break;
			case 0x200000:	
				info->sector_count =256;
				info->size = 0x1000000;
				if ((info->flash_id & 0xFF) == 0x20)	/* numonyx */
					info->specificspi = SpecificSPI_N25Q512;
#if (CONFIG_AST1500_SOC_VER >= 3)
				info->sector_count = 1024;
				info->size = 0x4000000;
				info->address32 = 1;
#if defined(CONFIG_FLASH_SPIx2_Dummy)
				info->readcmd = 0xbb;
				info->dummybyte = 1;
				info->dualport = 1;
				info->iomode = IOMODEx2_dummy;
#elif defined(CONFIG_FLASH_SPIx4_Dummy)
				info->readcmd = 0xeb;
				info->dummybyte = 3;
				info->dualport = 0;
				info->iomode = IOMODEx4_dummy;
				info->quadport = 1;
				info->dummydata = 0xaa;
#endif
#endif
				break;

			default:
				printf("Can't support this SPI Flash!! \n");
				return 0;
			}
		} /* JDEC */
	}

	debug ("erase_region_count = %d erase_region_size = %d\n",
			erase_region_count, erase_region_size);

	sector = base;
	for (j = 0; (j < info->sector_count) && (j < CFG_MAX_FLASH_SECT); j++) {
		info->start[j] = sector;
		sector += erase_region_size;
		info->protect[j] = 0; /* default: not protected */
	}

	//cpuclk means HCLK here.
	cpuclk = get_HCLK_MHz();

#if (CONFIG_AST1500_SOC_VER == 2) //AST1510 Only patch
	/* limit Max SPI CLK to 50MHz (Datasheet v1.2) */
	if (WriteClk > 50) WriteClk = 50;
	if (EraseClk > 50) EraseClk = 50;
	if (ReadClk > 50)  ReadClk  = 50;
#endif

	div = 1;
	while ( ((cpuclk/div) > WriteClk) && (div < 16) )
	{
		div++;
	}
	info->tCK_Write = AST2300_SPICLK_DIV[div-1];

	div = 1;
	while ( ((cpuclk/div) > EraseClk) && (div < 16) )
	{
		div++;
	}
	info->tCK_Erase = AST2300_SPICLK_DIV[div-1];

	div = 1;
	while ( ((cpuclk/div) > ReadClk) && (div < 16) )
	{
		div++;
	}
	info->tCK_Read = AST2300_SPICLK_DIV[div-1];

	//printf("tW=%x,tE=%x,tR=%x\n", info->tCK_Write, info->tCK_Erase, info->tCK_Read);

	/* unprotect flash */
	write_status_register(info, 0);

	if (info->quadport)
		write_status_register(info, 0x40);	/* enable QE */

	if (info->address32)
	{
#if (CONFIG_AST1500_SOC_VER == 2)
		reg = *((volatile ulong*) 0x1e6e2070);	/* set H/W Trappings */
		reg |= 0x10;
		*((volatile ulong*) 0x1e6e2070) = reg;
#endif
		reg  = *((volatile ulong*) 0x1e620004);	/* enable 32b control bit*/
		reg |= (0x01 << info->CE);
		*((volatile ulong*) 0x1e620004) = reg;

		/* set flash chips to 32bits addressing mode */
		if ((info->flash_id & 0xFF) == 0x01)	/* Spansion */
			enable4b_spansion(info);
		else if ((info->flash_id & 0xFF) == 0x20)	/* Numonyx */
			enable4b_numonyx(info);
		else /* MXIC, Winbond */
			enable4b(info);

	}

	reset_flash(info);

	return (info->size);
}


/*-----------------------------------------------------------------------
 */
static int flash_write_buffer (flash_info_t *info, uchar *src, ulong addr, int len)
{
        ulong j, base, offset;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;

        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
            
	base = info->start[0];
	offset = addr - base;
	base = flash_make_addr (info, 0, 0);
	
        enable_write (info);

        ulCtrlData  = (info->tCK_Write << 8);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x02);
        flash_udelay(10);
        if (info->address32)
        {
            *(uchar *) (base) = (uchar) ((offset & 0xff000000) >> 24);
            flash_udelay(10);
        }
        *(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
        flash_udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
        flash_udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x0000ff));
        flash_udelay(10);

        for (j=0; j<len; j++)
        {
            *(uchar *) (base) = *(uchar *) (src++);
            flash_udelay(10);
        }    
                 
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        flash_udelay(10);
        do {                             
            jReg = *(volatile uchar *) (base);
        } while ((jReg & 0x01));
        ulCtrlData &= CMD_MASK;
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        flash_udelay(200);

        /* RFSR */
        if (info->specificspi == SpecificSPI_N25Q512)
        {
            ulCtrlData &= CMD_MASK;
            ulCtrlData |= CE_LOW | USERMODE;
            *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
            flash_udelay(200);
            *(uchar *) (base) = (uchar) (0x70);
            flash_udelay(10);
            do {
                jReg = *(volatile uchar *) (base);
            } while (!(jReg & 0x80));
            ulCtrlData &= CMD_MASK;
            ulCtrlData |= CE_HIGH | USERMODE;
            *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
            flash_udelay(200);
        }
}	

/*-----------------------------------------------------------------------
 *
 * export functions
 *  
 */
 
/*-----------------------------------------------------------------------
 * 
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		size += flash_info[i].size = flash_get_size (bank_base[i], i);
		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
#ifndef CFG_FLASH_QUIET_TEST
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",
				i, flash_info[i].size, flash_info[i].size << 20);
#endif /* CFG_FLASH_QUIET_TEST */
		}
	}

	/* Monitor protection ON by default */
#if (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len  - 1,
		       flash_get_info(CFG_MONITOR_BASE));
#endif

	/* Environment protection ON by default */
#ifdef CFG_ENV_IS_IN_FLASH
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SECT_SIZE - 1,
		       flash_get_info(CFG_ENV_ADDR));
#endif

	/* Redundant environment protection ON by default */
#ifdef CFG_ENV_ADDR_REDUND
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR_REDUND,
		       CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1,
		       flash_get_info(CFG_ENV_ADDR_REDUND));
#endif
	return (size);
}

/*-----------------------------------------------------------------------
 */
#if defined(CFG_ENV_IS_IN_FLASH) || defined(CFG_ENV_ADDR_REDUND) || (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->size && info->start[0] <= base &&
		    base <= info->start[0] + info->size - 1)
			break;
	}

	return i == CFG_MAX_FLASH_BANKS ? 0 : info;
}
#endif

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	flash_sect_t sect;

        ulong base, offset;
        ulong ulSMMBase, ulCtrlData, CtrlOffset;
        uchar jReg;
        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
                    
	if ((s_first < 0) || (s_first > s_last)) {
		puts ("- no sectors to erase\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		putc ('\n');
	}

	ulCtrlData  = (info->tCK_Erase << 8);
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			/* start erasing */
			enable_write(info);

			base = info->start[0];
			offset = info->start[sect] - base;
			base = flash_make_addr (info, 0, 0);

			ulCtrlData &= CMD_MASK;
			ulCtrlData |= CE_LOW | USERMODE;
			*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
			flash_udelay(200);
			*(uchar *) (base) = (uchar) (0xd8);
			flash_udelay(10);
			if (info->address32)
			{ 
				*(uchar *) (base) = (uchar) ((offset & 0xff000000) >> 24);
				flash_udelay(10);
			}
			*(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
			flash_udelay(10);
			*(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
			flash_udelay(10);
			*(uchar *) (base) = (uchar) ((offset & 0x0000ff));
			flash_udelay(10);

			ulCtrlData &= CMD_MASK;
			ulCtrlData |= CE_HIGH | USERMODE;
			*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
			flash_udelay(200);

			ulCtrlData &= CMD_MASK;
			ulCtrlData |= CE_LOW | USERMODE;
			*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
			flash_udelay(200);
			*(uchar *) (base) = (uchar) (0x05);
			flash_udelay(10);
			do {
				jReg = *(volatile uchar *) (base);
			} while ((jReg & 0x01));
			ulCtrlData &= CMD_MASK;
			ulCtrlData |= CE_HIGH | USERMODE;
			*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
			flash_udelay(200);

			/* RFSR */
			if (info->specificspi == SpecificSPI_N25Q512)
			{
				ulCtrlData &= CMD_MASK;
				ulCtrlData |= CE_LOW | USERMODE;
				*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
				flash_udelay(200);
				*(uchar *) (base) = (uchar) (0x70);
				flash_udelay(10);
				do {
					jReg = *(volatile uchar *) (base);
				} while (!(jReg & 0x80));
				ulCtrlData &= CMD_MASK;
				ulCtrlData |= CE_HIGH | USERMODE;
				*(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
				flash_udelay(200);
			}

			putc ('.');
		}
	}
	puts (" done\n");
	
	reset_flash(info);
	
	return rcode;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	putc ('\n');
	return;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	int count;
	unsigned char pat[] = {'|', '-', '/', '\\'};
	int patcnt;
	ulong BufferSize = info->buffersize;
	/* get lower aligned address */	
	if (addr & (BufferSize - 1))
	{
		count = cnt >= BufferSize ? (BufferSize - (addr & 0xff)):cnt;	
		flash_write_buffer (info, src, addr, count);
		addr+= count;
		src += count;
		cnt -= count;	
	}
	
	/* prog */		
	while (cnt > 0) {				
		count = cnt >= BufferSize ? BufferSize:cnt;
		flash_write_buffer (info, src, addr, count);
		addr+= count;
		src += count;
		cnt -= count;
		printf("%c\b", pat[(patcnt++) & 0x03]);
	}

	reset_flash(info);

	return (0);
}

#ifdef	CFG_FLASH_DMA //For AST1500_SOC_VER >= 2. Not used for now.
void * memmove_dma(void * dest,const void *src,size_t count)
{
	ulong count_align, poll_time, data;
	
	count_align = (count + 3) & 0xFFFFFFFC;	/* 4-bytes align */
	poll_time = 100;			/* set 100 us as default */

	/* force end of burst read */
	*(volatile ulong *) (STCBaseAddress + SPICtrlRegOffset) |= CE_HIGH;
	*(volatile ulong *) (STCBaseAddress + SPICtrlRegOffset) &= ~CE_HIGH;

	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_CONTROL) = (ulong) (~FLASH_DMA_ENABLE);	
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_FLASH_BASE) = (ulong *) (src);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_DRAM_BASE) = (ulong *) (dest);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_LENGTH) = (ulong) (count_align);
	*(ulong *) (STCBaseAddress + REG_FLASH_DMA_CONTROL) = (ulong) (FLASH_DMA_ENABLE);
	
	/* wait poll */
	do {
		flash_udelay(poll_time);	
		data = *(ulong *) (STCBaseAddress + REG_FLASH_INTERRUPT_STATUS);
	} while (!(data & FLASH_STATUS_DMA_READY));
	
	/* clear status */
	*(ulong *) (STCBaseAddress + REG_FLASH_INTERRUPT_STATUS) |= FLASH_STATUS_DMA_CLEAR;
}	
#endif

#endif /* CFG_FLASH_SPI */
