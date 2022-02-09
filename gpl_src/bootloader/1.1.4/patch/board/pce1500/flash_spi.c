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
#define S25FL064A		0x160201
#define S25FL128P		0x182001
#define W25X16			0x1530ef
#define W25X64			0x1730ef
#define MX25L1605D		0x1520C2
#define MX25L12805D		0x1820C2

/* SPI Define */
#define STCBaseAddress		0x16000000
#define SPICtrlRegOffset	0x04
#define SPICtrlRegOffset2	0x0C

#define CMD_MASK		0xFFFFFFF8

#define NORMALREAD		0x00
#define	FASTREAD		0x01
#define NORMALWRITE		0x02
#define USERMODE		0x03

#define CE_LOW			0x00
#define CE_HIGH			0x04

#define BufferSize		256

/*-----------------------------------------------------------------------
 * Functions
 */

typedef unsigned long flash_sect_t;

static void reset_flash (flash_info_t * info);
static void enable_write (flash_info_t * info);
static void write_status_register (flash_info_t * info, uchar data);
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
        
        if (info->CE == 2)
        {
            CtrlOffset = SPICtrlRegOffset2;
        }    
        else
        {
            CtrlOffset = SPICtrlRegOffset;
        }    
                    
        if (info->dualport)
            ulCtrlData  = (0x3b0000) | (info->tCK_Read << 8) | (info->dummybyte << 6) | 0x08;        
        else
            ulCtrlData  = (0x0b0000) | (info->tCK_Read << 8) | (info->dummybyte << 6);
        ulCtrlData |= FASTREAD;
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
        udelay(200);
        *(uchar *) (base) = (uchar) (0x06);
        udelay(10);
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {                                
            jReg = *(volatile uchar *) (base);
        } while (!(jReg & 0x02));                  	                
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

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
        udelay(200);
        *(uchar *) (base) = (uchar) (0x01);
        udelay(10);
        *(uchar *) (base) = (uchar) (data);                
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData = CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {                                
            jReg = *(volatile uchar *) (base);
        } while (jReg & 0x01);                  	                
        ulCtrlData = CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

}

/*
 *
 */
static ulong flash_get_size (ulong base, int banknum)
{
	flash_info_t *info = &flash_info[banknum];
	int j;
	unsigned long sector;
	int erase_region_size;
        ulong ulCtrlData, CtrlOffset;
        int usID;
        ulong cpuclk, div, reg;
        ulong WriteClk, EraseClk, ReadClk;
        ulong vbase;
 
	info->start[0] = base;
	vbase = flash_make_addr (info, 0, 0);

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
            
        /* Get Flash ID */
        ulCtrlData  = *(ulong *) (STCBaseAddress + CtrlOffset) & CMD_MASK;
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (vbase) = (uchar) (0x9F);
        udelay(10);
        usID = *(int *) (vbase) & 0xFFFFFF;
        ulCtrlData  = *(ulong *) (STCBaseAddress + CtrlOffset) & CMD_MASK;        
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        info->flash_id = usID;
        
//        printf("SPI Flash ID: %x \n", usID);

        switch (info->flash_id)
        {
        case STM25P64:
	    info->sector_count = 128;
	    info->size = 0x800000;        	
            erase_region_size  = 0x10000;
            info->dualport = 0;
            info->dummybyte = 1;
            WriteClk = 40;
            EraseClk = 20;
            ReadClk  = 40;
            break;
            
        case S25FL128P:
	    info->sector_count = 256;
	    info->size = 0x1000000;        	
            erase_region_size  = 0x10000;
            info->dualport = 0;            
            info->dummybyte = 1;
            WriteClk = 100;
            EraseClk = 40;
            ReadClk  = 100;            
            break;

        case W25X16:
	    info->sector_count = 32;
	    info->size = 0x200000;        	
            erase_region_size  = 0x10000;
            info->dualport = 1;            
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 25;
            ReadClk  = 50;            
            break;
            
        case W25X64:
	    info->sector_count = 128;
	    info->size = 0x800000;        	
            erase_region_size  = 0x10000;
            info->dualport = 1;            
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 25;
            ReadClk  = 50;            
            break;

        case S25FL064A:
	    info->sector_count = 128;
	    info->size = 0x800000;        	
            erase_region_size  = 0x10000;
            info->dualport = 0;            
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 25;
            ReadClk  = 50;            
            break;
            
        case STM25P128:
	    info->sector_count = 64;
	    info->size = 0x1000000;        	
            erase_region_size  = 0x40000;
            info->dualport = 0;
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 20;
            ReadClk  = 50;
            break;
                          
        case MX25L12805D:
	    info->sector_count = 256;
	    info->size = 0x1000000;        	
            erase_region_size  = 0x10000;
            info->dualport = 0;
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 20;
            ReadClk  = 50;
            break;

        case MX25L1605D:
	    info->sector_count = 32;
	    info->size = 0x200000;        	
            erase_region_size  = 0x10000;
            info->dualport = 0;
            info->dummybyte = 1;
            WriteClk = 50;
            EraseClk = 20;
            ReadClk  = 50;
            break;
                          
        default:
            printf("Can't support this SPI Flash!! \n");
            break;
        }
			
        debug ("erase_region_count = %d erase_region_size = %d\n",
		erase_region_count, erase_region_size);

	sector = base;			
	for (j = 0; j < info->sector_count; j++) {
		
		info->start[j] = sector;
		sector += erase_region_size;
		info->protect[j] = 0; /* default: not protected */
	}

#if 0
        /* set SPI flash extended info */
        reg = *((volatile ulong*) 0x1e6e2070);
        switch (reg & 0xe00)
        {
        case 0x000:
                 cpuclk = 266;
                 break;
        case 0x200:
                 cpuclk = 233;
                 break;        
        case 0x400:
                 cpuclk = 200;
                 break;        
        case 0x600:
                 cpuclk = 166;
                 break;        
        case 0x800:
                 cpuclk = 133;
                 break;        
        case 0xA00:
                 cpuclk = 100;
                 break;        
        case 0xC00:
                 cpuclk = 300;
                 break;        
        case 0xE00:	
                 cpuclk = 24;
                 break;        
        }	
#else
		reg = *((volatile ulong*)0x1e6e2024);
		if (reg & 0x00040000)
		{
			ulong denumerator, numerator, output_divider, post_divider;
			debug("H-PLL from SCU24[0:17]\n");
			denumerator = (reg & 0xF);
			output_divider = ((reg & 0x10) >> 4);
			numerator = ((reg & 0x7E0) >> 5);
			post_divider = ((reg & 0x7000) >> 12);
			switch (post_divider)
			{
			case 4:
				post_divider = 2;
				break;
			case 5:
				post_divider = 4;
				break;
			case 6:
				post_divider = 8;
				break;
			case 7:
				post_divider = 16;
				break;
			default:
				post_divider = 1;
			}
			cpuclk = 24 * (2 - output_divider) * ((numerator + 2) / (denumerator + 1)) / post_divider;
			reg = *((volatile ulong*)0x1e6e2070);
		}
		else
		{
			debug("H-PLL from trappings\n");
			reg = *((volatile ulong*)0x1e6e2070);
			switch (reg & 0xe00)
			{
			case 0x000:
				 cpuclk = 266;
				 break;
			case 0x200:
				 cpuclk = 233;
				 break; 	   
			case 0x400:
				 cpuclk = 200;
				 break; 	   
			case 0x600:
				 cpuclk = 166;
				 break; 	   
			case 0x800:
				 cpuclk = 133;
				 break; 	   
			case 0xA00:
				 cpuclk = 100;
				 break; 	   
			case 0xC00:
				 cpuclk = 300;
				 break; 	   
			case 0xE00: 
				 cpuclk = 24;
				 break; 	   
			}
		}
#endif
        switch (reg & 0x3000)
        {
        case 0x1000:
                 cpuclk /= 2;
                 break;
        case 0x2000:
                 cpuclk /= 4;
                 break;        
        case 0x3000:	
                 cpuclk /= 3;
                 break;        
        }	
        div = 2;
        info->tCK_Write = 7;
        while ( (cpuclk/div) > WriteClk )
        {
            info->tCK_Write--;	
            div +=2;	
        }
        div = 2;
        info->tCK_Erase = 7;
        while ( (cpuclk/div) > EraseClk )
        {
            info->tCK_Erase--;	
            div +=2;	
        }
        div = 2;
        info->tCK_Read = 7;
        while ( (cpuclk/div) > ReadClk )
        {
            info->tCK_Read--;	
            div +=2;	
        }
	
	/* unprotect flash */	
        write_status_register(info, 0);
        
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
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x02);
        udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
        udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
        udelay(10);
        *(uchar *) (base) = (uchar) ((offset & 0x0000ff));
        udelay(10);

        for (j=0; j<len; j++)
        {
            *(uchar *) (base) = *(uchar *) (src++);
            udelay(10);
        }    
                 
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_LOW | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);
        *(uchar *) (base) = (uchar) (0x05);
        udelay(10);
        do {                             
            jReg = *(volatile uchar *) (base);
        } while ((jReg & 0x01));
        ulCtrlData  = (info->tCK_Write << 8);
        ulCtrlData |= CE_HIGH | USERMODE;
        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
        udelay(200);

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

	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
                        /* start erasing */
                        enable_write(info);

	                base = info->start[0];
                        offset = info->start[sect] - base;
	                base = flash_make_addr (info, 0, 0);

                        ulCtrlData  = (info->tCK_Erase << 8);
                        ulCtrlData |= CE_LOW | USERMODE;
                        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
                        udelay(200);
                        *(uchar *) (base) = (uchar) (0xd8);
                        udelay(10);
                        *(uchar *) (base) = (uchar) ((offset & 0xff0000) >> 16);
                        udelay(10);
                        *(uchar *) (base) = (uchar) ((offset & 0x00ff00) >> 8);
                        udelay(10);
                        *(uchar *) (base) = (uchar) ((offset & 0x0000ff));
                        udelay(10);

                        ulCtrlData  = (info->tCK_Erase << 8);
                        ulCtrlData |= CE_HIGH | USERMODE;
                        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
                        udelay(200);

                        ulCtrlData  = (info->tCK_Erase << 8);
                        ulCtrlData |= CE_LOW | USERMODE;
                        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
                        udelay(200);
                        *(uchar *) (base) = (uchar) (0x05);
                        udelay(10);
                        do {                                                            
                            jReg = *(volatile uchar *) (base);
                        } while ((jReg & 0x01));
                        ulCtrlData  = (info->tCK_Erase << 8);
                        ulCtrlData |= CE_HIGH | USERMODE;
                        *(ulong *) (STCBaseAddress + CtrlOffset) = ulCtrlData;
                        udelay(200);

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
#endif /* CFG_FLASH_SPI */
