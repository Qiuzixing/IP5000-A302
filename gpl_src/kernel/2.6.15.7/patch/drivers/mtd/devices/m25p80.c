/*
 * MTD SPI driver for ST M25Pxx (and similar) serial flash chips
 *
 * Author: Mike Lavender, mike@steroidmicros.com
 *
 * Copyright (c) 2005, Intec Automation Inc.
 *
 * Some parts are based on lart.c by Abraham Van Der Merwe
 *
 * Cleaned up and generalized based on mtd_dataflash.c
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/interrupt.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <asm/semaphore.h>

#include <asm/hardware.h>
#include <asm/io.h>
#ifdef CONFIG_SPI_ASTSMC
#include <asm/arch/astsmc_spi.h>
#else
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#endif

#include <asm/arch/ast-scu.h>

#define FLASH_PAGESIZE		256

#define CFI_MFR_AMD		0x0001
#define CFI_MFR_MACRONIX	0x00C2
#define CFI_MFR_WINBOND		0x00EF

#define S25FL128P		0x012018
#define MX25L12805D		0xC22018
#define MX25L25635E		0xC22019
#define W25Q128FV		0xEF4018
#define MX66L1G45G		0xC2201b

/* Flash opcodes. */
#define	OPCODE_WREN		0x06	/* Write enable */
#define	OPCODE_RDSR		0x05	/* Read status register */
#define	OPCODE_NORM_READ	0x03	/* Read data bytes (low frequency) */
#define	OPCODE_FAST_READ	0x0b	/* Read data bytes (high frequency) */
#define	OPCODE_PP		0x02	/* Page program (up to 256 bytes) */
#define	OPCODE_BE_4K 		0x20	/* Erase 4KiB block */
#define	OPCODE_BE_32K		0x52	/* Erase 32KiB block */
#define	OPCODE_SE		0xd8	/* Sector erase (usually 64KiB) */
#define	OPCODE_RDID		0x9f	/* Read JEDEC ID */

/* Status Register bits. */
#define	SR_WIP			1	/* Write in progress */
#define	SR_WEL			2	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define	SR_BP0			4	/* Block protect 0 */
#define	SR_BP1			8	/* Block protect 1 */
#define	SR_BP2			0x10	/* Block protect 2 */
#define	SR_SRWD			0x80	/* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	100000
#define	CMD_SIZE		4

#ifdef CONFIG_M25PXX_USE_FAST_READ
#define OPCODE_READ 	OPCODE_FAST_READ
#define FAST_READ_DUMMY_BYTE 1
#else
#define OPCODE_READ 	OPCODE_NORM_READ
#define FAST_READ_DUMMY_BYTE 0
#endif

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif

#ifndef CONFIG_SPI_ASTSMC

#if (CONFIG_AST1500_SOC_FLASH_MAPPING_TYPE == 2)
#define	CS_NUM			0
#else
#define	CS_NUM			2
#endif

#define CMD_MASK		0xFFFFFFF8

#define NORMALREAD		0x00
#define FASTREAD		0x01
#define NORMALWRITE		0x02
#define USERMODE		0x03

#define CE_LOW			0x00
#define CE_HIGH			0x04

#if defined(DEBUG_PROFILE_SCU_FREE_RUN_CNT)
#define __GetReg32(base, offset) (*(volatile u32 *)(IO_ADDRESS((u32)(base)+(u32)(offset))))

#define SCU_FREE_RUN_COUNTER 0xE0
static u32 profile_start_time;

static void profile_start(void)
{
	profile_start_time = __GetReg32(ASPEED_SCU_BASE, SCU_FREE_RUN_COUNTER);
}

static void profile_finish(void)
{
	u32 end, delta;

	end = __GetReg32(ASPEED_SCU_BASE, SCU_FREE_RUN_COUNTER);

	if (end > profile_start_time)
		delta = end - profile_start_time;
	else
		delta = 0xFFFFFFFF - profile_start_time + end;

	delta = delta / 24000;
	printk("\n delta time is %d ms (start 0x%.8x, end 0x%.8x)\n",
		delta, profile_start_time, end);
}
#endif

static inline u32 CTRL_REG_OFFSET(u32 cs_num)
{
	u32 offset = 0;
	switch (ASPEED_SMC_CONTROLLER_BASE) {
		case ASPEED_SMC_CONTROLLER_BASE_V1:
			offset = 0x4 * (cs_num + 1);
			break;
		case ASPEED_SMC_CONTROLLER_BASE_V2:
			offset = 0x10 + (cs_num * 4);
			break;
		default:
			printk("Unsupported ASPEED_SMC_CONTROLLER_BASE number (0x%08X)\n", ASPEED_SMC_CONTROLLER_BASE);
			BUG();
	}
	return(offset);
}
#endif

/****************************************************************************/

struct m25p {
	struct spi_device	*spi;
	struct semaphore	lock;
	struct mtd_info		mtd;
	unsigned		partitioned:1;
	u8			erase_opcode;
#ifdef CONFIG_SPI_ASTSMC
	u8			command[CMD_SIZE + FAST_READ_DUMMY_BYTE];
#else
	u8	*AST_SMC_base;
	u8	*AST_SMC_config_reg_addr;
	u8	*SPI_Flash_base;
	ulong	tCK_Write;
	ulong	tCK_Erase;
	ulong	tCK_Read;
	ulong	dummybyte;
	u32 addr_width;
#endif
};

#if (CONFIG_AST1500_SOC_VER >= 2)
static ulong AST2300_SPICLK_DIV[16] = {0x0F, 0x07, 0x0E, 0x06, 0x0D, 0x05, 0x0C, 0x04, \
                                     0x0B, 0x03, 0x0A, 0x02, 0x09, 0x01, 0x08, 0x00 };
#endif

static inline struct m25p *mtd_to_m25p(struct mtd_info *mtd)
{
	return container_of(mtd, struct m25p, mtd);
}

/****************************************************************************/

/*
 * Internal helper functions
 */

/*
 * Read the status register, returning its value in the location
 * Return the status register value.
 * Returns negative if error occurred.
 */
#ifdef CONFIG_SPI_ASTSMC
static int read_sr(struct m25p *flash)
{
	ssize_t retval;
	u8 code = OPCODE_RDSR;
	u8 val;

	retval = spi_write_then_read(flash->spi, &code, 1, &val, 1);

	if (retval < 0) {
		dev_err(&flash->spi->dev, "error %d reading SR\n",
				(int) retval);
		return retval;
	}

	return val;
}
#endif

#ifndef CONFIG_SPI_ASTSMC
#define STATUS_REG_CHECK
static void write_reg(struct m25p *flash, u8 opcode)
{
	u32 val;

	val = (flash->tCK_Write << 8) | CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = opcode;
	udelay(10);

	val = (flash->tCK_Write << 8) | CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);
}

static void read_reg(struct m25p *flash, u8 opcode, u8 *data, int len)
{
	u32 val, i;

	val = (flash->tCK_Write << 8) | CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = opcode;
	udelay(10);

	for (i = 0; i < len; i++) {
		*data = *(volatile u8 *)(flash->SPI_Flash_base);
		data++;
	}

	val = (flash->tCK_Write << 8) | CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);
}

#if defined(STATUS_REG_CHECK)
/* Status Register bits. */
#define SR_WIP	1	/* Write in progress */
#define SR_WEL	2	/* Write enable latch */

static u8 read_sr(struct m25p *flash)
{
	u8 sr;

	read_reg(flash, OPCODE_RDSR, &sr, 1);
	return sr;
}
#endif

//reset AST SMC to fast read mode
static void reset_flash (struct m25p *flash)
{
	u32 ulCtrlData;

	ulCtrlData  = (0x0b0000) | (flash->tCK_Read << 8) | (flash->dummybyte << 6);
	ulCtrlData |= FASTREAD;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
}
#endif

/*
 * Set write enable latch with Write Enable command.
 * Returns negative if error occurred.
 */
#ifdef CONFIG_SPI_ASTSMC
static inline int write_enable(struct m25p *flash)
#else
static inline void write_enable(struct m25p *flash)
#endif
{
	u8 code = OPCODE_WREN;

#ifdef CONFIG_SPI_ASTSMC
	return spi_write_then_read(flash->spi, &code, 1, NULL, 0);
#else
	write_reg(flash, code);
#if defined(STATUS_REG_CHECK)
	while (!(SR_WEL & read_sr(flash)))
	{
		msleep(10);
	}
#endif
	
#endif
}

#ifdef CONFIG_SPI_ASTSMC
/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct m25p *flash)
{
	int count;
	int sr;

	/* one chip guarantees max 5 msec wait here after page writes,
	 * but potentially three seconds (!) after page erase.
	 */
	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if ((sr = read_sr(flash)) < 0)
			break;
		else if (!(sr & SR_WIP))
			return 0;

		/* REVISIT sometimes sleeping would be best */
	}

	return 1;
}
#endif

/*
 * Erase one sector of flash memory at offset ``offset'' which is any
 * address within the sector which should be erased.
 *
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct m25p *flash, u32 offset)
{
#ifdef CONFIG_SPI_ASTSMC
	DEBUG(MTD_DEBUG_LEVEL3, "%s: %s %dKiB at 0x%08x\n",
			flash->spi->dev.bus_id, __func__,
			flash->mtd.erasesize / 1024, offset);

	/* Wait until finished previous write command. */
	if (wait_till_ready(flash))
		return 1;

	/* Send write enable, then erase commands. */
	write_enable(flash);

	/* Set up command buffer. */
	flash->command[0] = flash->erase_opcode;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	spi_write(flash->spi, flash->command, CMD_SIZE);
#else
	u32	ulCtrlData;

	write_enable(flash);

	ulCtrlData	= (flash->tCK_Erase << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(flash->erase_opcode);
	udelay(10);

	if (4 == flash->addr_width) {
		barrier();
		*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0xff000000) >> 24);
		udelay(10);
	}

	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0xff0000) >> 16);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0x00ff00) >> 8);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0x0000ff));
	udelay(10);

	ulCtrlData	= (flash->tCK_Erase << 8);
	ulCtrlData |= CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	barrier();
#if defined(DEBUG_PROFILE_SCU_FREE_RUN_CNT)
	profile_start();
#endif
#if defined(STATUS_REG_CHECK)
	while (SR_WIP & read_sr(flash))
	{
		msleep(10);
	}
#endif
	
#if defined(DEBUG_PROFILE_SCU_FREE_RUN_CNT)
	profile_finish();
#endif

#endif

	return 0;
}

/****************************************************************************/

/*
 * MTD implementation
 */

/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
static int m25p80_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct m25p *flash = mtd_to_m25p(mtd);
	u32 addr,len;

#ifdef CONFIG_SPI_ASTSMC
	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %d\n",
			flash->spi->dev.bus_id, __func__, "at",
			(u32)instr->addr, instr->len);
#endif

	/* sanity checks */
	if (instr->addr + instr->len > flash->mtd.size)
		return -EINVAL;
	if ((instr->addr % mtd->erasesize) != 0
			|| (instr->len % mtd->erasesize) != 0) {
		return -EINVAL;
	}

	addr = instr->addr;
	len = instr->len;

  	down(&flash->lock);

	/* REVISIT in some cases we could speed up erasing large regions
	 * by using OPCODE_SE instead of OPCODE_BE_4K
	 */

	/* now erase those sectors */
	while (len) {
		if (erase_sector(flash, addr)) {
			instr->state = MTD_ERASE_FAILED;
			up(&flash->lock);
			return -EIO;
		}

		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}

#ifndef CONFIG_SPI_ASTSMC
	reset_flash(flash);
#endif

  	up(&flash->lock);

	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int m25p80_read(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
#ifdef CONFIG_SPI_ASTSMC
	struct spi_transfer t[2];
	struct spi_message m;

	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			flash->spi->dev.bus_id, __func__, "from",
			(u32)from, len);
#else
	unsigned long i;
#endif

	/* sanity checks */
	if (!len)
		return 0;

	if (from + len > flash->mtd.size)
		return -EINVAL;

#ifdef CONFIG_SPI_ASTSMC
	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	/* NOTE:
	 * OPCODE_FAST_READ (if available) is faster.
	 * Should add 1 byte DUMMY_BYTE.
	 */
	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE + FAST_READ_DUMMY_BYTE;
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = buf;
	t[1].len = len;
	spi_message_add_tail(&t[1], &m);
#endif

	/* Byte count starts at zero. */
	if (retlen)
		*retlen = 0;

	down(&flash->lock);

#ifdef CONFIG_SPI_ASTSMC
	/* Wait till previous write/erase is done. */
	if (wait_till_ready(flash)) {
		/* REVISIT status return?? */
		up(&flash->lock);
		return 1;
	}

	/* FIXME switch to OPCODE_FAST_READ.  It's required for higher
	 * clocks; and at this writing, every chip this driver handles
	 * supports that opcode.
	 */

	/* Set up the write data buffer. */
	flash->command[0] = OPCODE_READ;
	flash->command[1] = from >> 16;
	flash->command[2] = from >> 8;
	flash->command[3] = from;

	spi_sync(flash->spi, &m);

	*retlen = m.actual_length - CMD_SIZE - FAST_READ_DUMMY_BYTE;
#else
	for (i = 0; i < len; i++)
		*buf++ = *((u_char *)(flash->SPI_Flash_base + from + i));

	if (retlen)
		*retlen = len;
#endif

  	up(&flash->lock);

	return 0;
}

#ifndef CONFIG_SPI_ASTSMC
//use page program (0x02) command to program up to 256 bytes at a time
static void flash_write_buffer(struct m25p *flash, const u8 *src, u32 addr, int len)
{
	u32	j;
	u32	ulCtrlData;

	write_enable(flash);

	ulCtrlData  = (flash->tCK_Write << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(OPCODE_PP);
	udelay(10);

	if (4 == flash->addr_width) {
		barrier();
		*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0xff000000) >> 24);
		udelay(10);
	}

	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0xff0000) >> 16);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x00ff00) >> 8);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x0000ff));
	udelay(10);

	for (j = 0; j < len; j++) {
		barrier();
		*(u8 *)(flash->SPI_Flash_base) = *(u8 *)(src++);
		udelay(10);
	}

	ulCtrlData  = (flash->tCK_Write << 8);
	ulCtrlData |= CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	barrier();
#if defined(STATUS_REG_CHECK)
	while (SR_WIP & read_sr(flash))
	{
		msleep(10);
	}
#endif
	
}
#endif

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int m25p80_write(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct m25p *flash = mtd_to_m25p(mtd);
#ifdef CONFIG_SPI_ASTSMC
	u32 page_offset, page_size;
	struct spi_transfer t[2];
	struct spi_message m;
#else
	int count;
#endif

#ifdef CONFIG_SPI_ASTSMC
	DEBUG(MTD_DEBUG_LEVEL2, "%s: %s %s 0x%08x, len %zd\n",
			flash->spi->dev.bus_id, __func__, "to",
			(u32)to, len);
#endif

	if (retlen)
		*retlen = 0;

	/* sanity checks */
	if (!len)
		return(0);

	if (to + len > flash->mtd.size)
		return -EINVAL;

#ifdef CONFIG_SPI_ASTSMC
	spi_message_init(&m);
	memset(t, 0, (sizeof t));

	t[0].tx_buf = flash->command;
	t[0].len = CMD_SIZE;
	spi_message_add_tail(&t[0], &m);

	t[1].tx_buf = buf;
	spi_message_add_tail(&t[1], &m);
#endif

  	down(&flash->lock);

#ifdef CONFIG_SPI_ASTSMC
	/* Wait until finished previous write command. */
	if (wait_till_ready(flash)) {
		up(&flash->lock);
		return 1;
	}

	write_enable(flash);
#endif

#ifdef CONFIG_SPI_ASTSMC
	/* Set up the opcode in the write buffer. */
	flash->command[0] = OPCODE_PP;
	flash->command[1] = to >> 16;
	flash->command[2] = to >> 8;
	flash->command[3] = to;

	/* what page do we start with? */
	page_offset = to % FLASH_PAGESIZE;

	/* do all the bytes fit onto one page? */
	if (page_offset + len <= FLASH_PAGESIZE) {
		t[1].len = len;

		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_SIZE;
	} else {
		u32 i;

		/* the size of data remaining on the first page */
		page_size = FLASH_PAGESIZE - page_offset;

		t[1].len = page_size;
		spi_sync(flash->spi, &m);

		*retlen = m.actual_length - CMD_SIZE;

		/* write everything in PAGESIZE chunks */
		for (i = page_size; i < len; i += page_size) {
			page_size = len - i;
			if (page_size > FLASH_PAGESIZE)
				page_size = FLASH_PAGESIZE;

			/* write the next page to flash */
			flash->command[1] = (to + i) >> 16;
			flash->command[2] = (to + i) >> 8;
			flash->command[3] = (to + i);

			t[1].tx_buf = buf + i;
			t[1].len = page_size;

			wait_till_ready(flash);

			write_enable(flash);

			spi_sync(flash->spi, &m);

			if (retlen)
				*retlen += m.actual_length - CMD_SIZE;
		}
	}
#else
	/* get lower aligned address */
	if (to & (FLASH_PAGESIZE - 1)) {
		count = len >= FLASH_PAGESIZE ? (FLASH_PAGESIZE - (to & (FLASH_PAGESIZE - 1))):len;
		flash_write_buffer (flash, buf, to, count);
		to += count;
		buf += count;
		len -= count;
		if (retlen)
			*retlen += count;
	}

	/* prog */
	while (len > 0) {
		count = len >= FLASH_PAGESIZE ? FLASH_PAGESIZE:len;
		flash_write_buffer (flash, buf, to, count);
		to += count;
		buf += count;
		len -= count;
		if (retlen)
			*retlen += count;
	}
	reset_flash(flash);
#endif

	up(&flash->lock);

	return 0;
}

/****************************************************************************/

/*
 * SPI device driver setup and teardown
 */

struct flash_info {
	char		*name;

	/* JEDEC id zero means "no ID" (most older chips); otherwise it has
	 * a high byte of zero plus three data bytes: the manufacturer id,
	 * then a two byte device id.
	 */
	u32		jedec_id;

	/* The size listed here is what works with OPCODE_SE, which isn't
	 * necessarily called a "sector" by the vendor.
	 */
	unsigned	sector_size;
	u16		n_sectors;

	u16		flags;
#define	SECT_4K		0x01		/* OPCODE_BE_4K works uniformly */
};

/* NOTE: double check command sets and memory organization when you add
 * more flash chips.  This current list focusses on newer chips, which
 * have been converging on command sets which including JEDEC ID.
 */
static struct flash_info __devinitdata m25p_data [] = {

	/* Atmel -- some are (confusingly) marketed as "DataFlash" */
	{ "at25fs010",  0x1f6601, 32 * 1024, 4, SECT_4K, },
	{ "at25fs040",  0x1f6604, 64 * 1024, 8, SECT_4K, },

	{ "at25df041a", 0x1f4401, 64 * 1024, 8, SECT_4K, },
	{ "at25df641",  0x1f4800, 64 * 1024, 128, SECT_4K, },

	{ "at26f004",   0x1f0400, 64 * 1024, 8, SECT_4K, },
	{ "at26df081a", 0x1f4501, 64 * 1024, 16, SECT_4K, },
	{ "at26df161a", 0x1f4601, 64 * 1024, 32, SECT_4K, },
	{ "at26df321",  0x1f4701, 64 * 1024, 64, SECT_4K, },

	/* Spansion -- single (large) sector size only, at least
	 * for the chips listed here (without boot sectors).
	 */
	{ "s25sl004a", 0x010212, 64 * 1024, 8, },
	{ "s25sl008a", 0x010213, 64 * 1024, 16, },
	{ "s25sl016a", 0x010214, 64 * 1024, 32, },
	{ "s25sl032a", 0x010215, 64 * 1024, 64, },
	{ "s25sl064a", 0x010216, 64 * 1024, 128, },

	{ "s25fl128p", 0x012018, 64 * 1024, 256, },

	/* SST -- large erase sizes are "overlays", "sectors" are 4K */
	{ "sst25vf040b", 0xbf258d, 64 * 1024, 8, SECT_4K, },
	{ "sst25vf080b", 0xbf258e, 64 * 1024, 16, SECT_4K, },
	{ "sst25vf016b", 0xbf2541, 64 * 1024, 32, SECT_4K, },
	{ "sst25vf032b", 0xbf254a, 64 * 1024, 64, SECT_4K, },

	/* ST Microelectronics -- newer production may have feature updates */
	{ "m25p05",  0x202010,  32 * 1024, 2, },
	{ "m25p10",  0x202011,  32 * 1024, 4, },
	{ "m25p20",  0x202012,  64 * 1024, 4, },
	{ "m25p40",  0x202013,  64 * 1024, 8, },
	{ "m25p80",         0,  64 * 1024, 16, },
	{ "m25p16",  0x202015,  64 * 1024, 32, },
	{ "m25p32",  0x202016,  64 * 1024, 64, },
	{ "m25p64",  0x202017,  64 * 1024, 128, },
	{ "m25p128", 0x202018, 256 * 1024, 64, },

	{ "m45pe80", 0x204014,  64 * 1024, 16, },
	{ "m45pe16", 0x204015,  64 * 1024, 32, },

	{ "m25pe80", 0x208014,  64 * 1024, 16, },
	{ "m25pe16", 0x208015,  64 * 1024, 32, SECT_4K, },

	/* Winbond -- w25x "blocks" are 64K, "sectors" are 4KiB */
	{ "w25x10",  0xef3011, 64 * 1024, 2, SECT_4K, },
	{ "w25x20",  0xef3012, 64 * 1024, 4, SECT_4K, },
	{ "w25x40",  0xef3013, 64 * 1024, 8, SECT_4K, },
	{ "w25x80",  0xef3014, 64 * 1024, 16, SECT_4K, },
	{ "w25x16",  0xef3015, 64 * 1024, 32, SECT_4K, },
	{ "w25x32",  0xef3016, 64 * 1024, 64, SECT_4K, },
	{ "w25x64",  0xef3017, 64 * 1024, 128, SECT_4K, },
	{ "w25q128", 0xef4018, 64 * 1024, 256, SECT_4K, },

	/* MxIC -- */
	{ "mx25l12805d", 0xc22018, 64 * 1024, 256, SECT_4K},
    { "mx25l25635e", 0xc22019, 64 * 1024, 512, SECT_4K},
	{ "mx66l1g45g",  0xc2201b, 64 * 1024, 2048, SECT_4K},
};

#ifdef CONFIG_SPI_ASTSMC
static struct flash_info *__devinit jedec_probe(struct spi_device *spi)
#else
static struct flash_info *__devinit jedec_probe(struct m25p *flash)
#endif
{
	int			tmp;
	u8			code = OPCODE_RDID;
	u8			id[3];
	u32			jedec;
	struct flash_info	*info;

#ifdef CONFIG_SPI_ASTSMC
	/* JEDEC also defines an optional "extended device information"
	 * string for after vendor-specific data, after the three bytes
	 * we use here.  Supporting some chips might require using it.
	 */
	tmp = spi_write_then_read(spi, &code, 1, id, 3);
	if (tmp < 0) {
		DEBUG(MTD_DEBUG_LEVEL0, "%s: error %d reading JEDEC ID\n",
			spi->dev.bus_id, tmp);
		return NULL;
	}
#else
	read_reg(flash, code, id, 3);
#endif

	jedec = id[0];
	jedec = jedec << 8;
	jedec |= id[1];
	jedec = jedec << 8;
	jedec |= id[2];

	for (tmp = 0, info = m25p_data;
			tmp < ARRAY_SIZE(m25p_data);
			tmp++, info++) {
		if (info->jedec_id == jedec)
			return info;
	}
#ifdef CONFIG_SPI_ASTSMC
	dev_err(&spi->dev, "unrecognized JEDEC id %06x\n", jedec);
#else
	printk("unrecognized JEDEC id %06x\n", jedec);
#endif
	return NULL;
}

#ifndef CONFIG_SPI_ASTSMC
static struct mtd_partition ast1500_flash_partitions[] = {
	{
		.name = "bootloader",
		.size = 0x70000,
		.offset = 0,
		/*.mask_flags = MTD_CAP_ROM*/
	}, {
		.name = "misc",
		.size = 0x10000,
		.offset = 0x70000,
	}, {
		.name = "kernel",
		.size = 0x380000,
		.offset = 0x80000,
	}, {
		.name = "rootfs",
		.size = 0xA00000,
		.offset = 0x400000,
	}, {
		.name = "kernel2",
		.size = 0x380000,
		.offset = 0xE00000,
	}, {
		.name = "rootfs2",
		.size = 0xA00000,
		.offset = 0x1180000,
	},	{
		.name = "data",
		.size = 0x440000,
		.offset = 0x1B80000,
	}, {
		.name = "logo",
		.size = 0x20000,
		.offset = 0x1FC0000,
	}, {
		.name = "param",
		.size = 0x10000,
		.offset = 0x1FE0000,
	}, {
		.name = "ROparam",
		.size = 0x10000,
		.offset = 0x1FF0000,
	}
};

static struct flash_platform_data ast1500_flash_data = {
	.name = "ast1500Flash",
	.parts = ast1500_flash_partitions,
	.nr_parts = ARRAY_SIZE(ast1500_flash_partitions),
};
#endif

static void spi_address_4b_cfg(struct m25p *flash, u32 cfg)
{
	u32 val;

	val = *(volatile u32 *)(flash->AST_SMC_base + 0x4);
	if (cfg)
		val |= 1;
	else
		val &= ~1;

	*(volatile u32 *)(flash->AST_SMC_base + 0x4) = val;
}

static void flash_mxic_4b_cfg(struct m25p *flash, u32 en)
{
	u8 op;

	if (en)
		op = 0xb7; /* EN4B: enter 4-byte mode */
	else
		op = 0xe9; /* EX4B: exit 4-byte mode*/

	write_reg(flash, op);
}

static void flash_4b_cfg(struct m25p *flash, u32 jedec_id, u32 en)
{
	switch (jedec_id >> 16) {
	/* TODO: not MXIC flash */
	case CFI_MFR_MACRONIX:
		flash_mxic_4b_cfg(flash, en);
		break;
	default:
		printk("unsupportd flash: JEDECID 0x%.8x\n", jedec_id);
		break;
	}
}

/*
 * board specific setup should have ensured the SPI clock used here
 * matches what the READ command supports, at least until this driver
 * understands FAST_READ (for clocks over 25 MHz).
 */
#ifdef CONFIG_SPI_ASTSMC
static int __devinit m25p_probe(struct spi_device *spi)
#else
static int __devinit m25p_probe(struct platform_device *pdev)
#endif
{
	struct flash_platform_data	*data;
	struct m25p			*flash;
	struct flash_info		*info;
	unsigned			i;
#ifdef CONFIG_SPI_ASTSMC
	struct resource *resource = NULL;
	struct astsmc_spi_device *astsmc_spi_flash = NULL;
	u32 dev_base;
#endif
	int status = 0;

#ifdef CONFIG_SPI_ASTSMC
	astsmc_spi_flash = spi->controller_data;
	resource = request_mem_region(astsmc_spi_flash->phy_dev_base, astsmc_spi_flash->dev_len, "SMC_MEMORY_BASE");
	if (resource == NULL)
	{
		printk("Cannot request memory region for SPI Flash\n");
		status = -ENOMEM;
		goto error_request_memory_region;
	}
	astsmc_spi_flash->vir_dev_base= ioremap(astsmc_spi_flash->phy_dev_base, astsmc_spi_flash->dev_len);
	if (astsmc_spi_flash->vir_dev_base == NULL) {
		printk("Cannot map SPI Flash\n");
		status = -ENODEV;
		goto error_ioremp;
	}
#else
//	printk("--->m25p_probe\n");
	flash = pdev->dev.driver_data;
	//steven:request memory regions before ioremap?
	flash->AST_SMC_base = ioremap(ASPEED_SMC_CONTROLLER_BASE, ASPEED_SMC_CONTROLLER_IO_RANGE);
	if (flash->AST_SMC_base == NULL) {
		printk("Cannot map SMC\n");
		status = -ENXIO;
		goto error_ioremp_smc;
	}

	flash->AST_SMC_config_reg_addr = flash->AST_SMC_base + CTRL_REG_OFFSET(CS_NUM);;
	flash->SPI_Flash_base = ioremap(ASPEED_SMC_FLASH_BASE, ASPEED_SMC_FLASH_SIZE);
	if (flash->SPI_Flash_base == NULL) {
		printk("Cannot map Flash\n");
		status = -ENXIO;
		goto error_ioremp_flash;
	}
//	printk("Flash base = %08X\n", flash->SPI_Flash_base);
#endif

	/* Platform data helps sort out which chip type we have, as
	 * well as how this board partitions it.  If we don't have
	 * a chip ID, try the JEDEC id commands; they'll work for most
	 * newer chips, even if we don't recognize the particular chip.
	 */
#ifdef CONFIG_SPI_ASTSMC
	data = spi->dev.platform_data;
#else
	data = &ast1500_flash_data;
#endif
#ifdef CONFIG_SPI_ASTSMC
	if (data && data->type) {
		for (i = 0, info = m25p_data;
				i < ARRAY_SIZE(m25p_data);
				i++, info++) {
			if (strcmp(data->type, info->name) == 0)
				break;
		}

		/* unrecognized chip? */
		if (i == ARRAY_SIZE(m25p_data)) {
			DEBUG(MTD_DEBUG_LEVEL0, "%s: unrecognized id %s\n",
					spi->dev.bus_id, data->type);
			info = NULL;

		/* recognized; is that chip really what's there? */
		} else if (info->jedec_id) {
			struct flash_info	*chip = jedec_probe(spi);

			if (!chip || chip != info) {
				dev_warn(&spi->dev, "found %s, expected %s\n",
						chip ? chip->name : "UNKNOWN",
						info->name);
				info = NULL;
			}
		}
	} else
		info = jedec_probe(spi);
#else
		info = jedec_probe(flash);
#endif
	if (!info) {
		status = -ENODEV;
		goto error_unknown_flash;
	}

#ifdef CONFIG_SPI_ASTSMC
	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (!flash) {
		status = -ENOMEM;
		goto error_unknown_flash;
	}
	flash->spi = spi;
#endif
	init_MUTEX(&flash->lock);
#ifdef CONFIG_SPI_ASTSMC
	dev_set_drvdata(&spi->dev, flash);
#endif

	{
		u32 cpuclk, div;
		u32 WriteClk, EraseClk, ReadClk;

		printk("Flash type:");
		switch (info->jedec_id) { /* steven:add supported flash models here */
		case S25FL128P:
			printk("S25FL128P\n");
			WriteClk = 100;
			EraseClk = 40;
			ReadClk  = 100;
			break;
		case MX25L12805D:
        case MX25L25635E:
		case W25Q128FV: /* winbond 25Q128FV */
			if (MX25L12805D == info->jedec_id)
				printk("MX25L12805D\n");
            else if (MX25L25635E == info->jedec_id)
                printk("MX25L25635E\n");
			else
				printk("W25Q128FV\n");
#if (CONFIG_AST1500_SOC_VER == 2)
			if (0x01000003 == ioread32(IO_ADDRESS(ASPEED_SCU_BASE) + 0x7c)) {
				/* Lower clock for AST2300 A0 */
				printk("AST1510 A0\n");
				WriteClk = 25;
				EraseClk = 20;
				ReadClk  = 25;
			} else {
				WriteClk = 50;
				EraseClk = 20;
				ReadClk  = 50;
			}
#else
			WriteClk = 50;
			EraseClk = 20;
#if !defined(CONFIG_AST1500_BOARD_V3)
			ReadClk  = 104;
#else
			ReadClk  = 50;
#endif
#endif

#ifndef CONFIG_SPI_ASTSMC
			flash->dummybyte = 1;
#endif
			break;
		case MX66L1G45G:
			printk("MX66L1G45G\n");
			WriteClk = 50;
			EraseClk = 20;
			ReadClk  = 104;
#ifndef CONFIG_SPI_ASTSMC
			flash->dummybyte = 1;
#endif
			break;
		default:
			printk("unknown (should never be here!!!)\n");
			goto error_unknown_flash;
		}

#if (CONFIG_AST1500_SOC_VER == 1)
		//cpuclk means HCLK here.
		cpuclk = ast_scu.HCLK_Mhz;
		div = 2;
#ifdef CONFIG_SPI_ASTSMC
		astsmc_spi_flash->tCK_Write = 7;
#else
		flash->tCK_Write = 7;
#endif
		while ((cpuclk/div) > WriteClk) {
#ifdef CONFIG_SPI_ASTSMC
			astsmc_spi_flash->tCK_Write--;
#else
			flash->tCK_Write--;
#endif
			div += 2;
		}
		div = 2;
#ifdef CONFIG_SPI_ASTSMC
		astsmc_spi_flash->tCK_Erase = 7;
#else
		flash->tCK_Erase = 7;
#endif
		while ((cpuclk/div) > EraseClk) {
#ifdef CONFIG_SPI_ASTSMC
			astsmc_spi_flash->tCK_Erase--;
#else
			flash->tCK_Erase--;
#endif
			div += 2;
		}
		div = 2;
#ifdef CONFIG_SPI_ASTSMC
		astsmc_spi_flash->tCK_Read = 7;
#else
		flash->tCK_Read = 7;
#endif
		while ((cpuclk/div) > ReadClk) {
#ifdef CONFIG_SPI_ASTSMC
			astsmc_spi_flash->tCK_Read--;
#else
			flash->tCK_Read--;
#endif
			div += 2;
		}

#elif (CONFIG_AST1500_SOC_VER >= 2)
		//cpuclk means HCLK here.
		cpuclk = ast_scu.HCLK_Mhz;

		div = 1;
		while (((cpuclk/div) > WriteClk) && (div < 16))
			div++;

		flash->tCK_Write = AST2300_SPICLK_DIV[div-1];

		div = 1;
		while (((cpuclk/div) > EraseClk) && (div < 16))
			div++;

		flash->tCK_Erase = AST2300_SPICLK_DIV[div-1];

		div = 1;
		while (((cpuclk/div) > ReadClk) && (div < 16))
			div++;

		flash->tCK_Read = AST2300_SPICLK_DIV[div-1];

#else
		#error "SPI clock not defined!"
#endif //#if (CONFIG_AST1500_SOC_VER >= 2)

	}

	if (data && data->name)
		flash->mtd.name = data->name;
#ifdef CONFIG_SPI_ASTSMC
	else
		flash->mtd.name = spi->dev.bus_id;
#endif

	flash->mtd.type = MTD_NORFLASH;
//steven:not supported	flash->mtd.writesize = 1;
	flash->mtd.flags = MTD_CAP_NORFLASH;
	flash->mtd.size = info->sector_size * info->n_sectors;
	flash->mtd.erase = m25p80_erase;
	flash->mtd.read = m25p80_read;
	flash->mtd.write = m25p80_write;

	if (0x1000000 < flash->mtd.size) {
		/* we need 4 byte mode when flash size > 128Mbit */
		flash->addr_width = 4;
		spi_address_4b_cfg(flash, 1);
		flash_4b_cfg(flash, info->jedec_id, 1);
	} else {
		flash->addr_width = 3;
		spi_address_4b_cfg(flash, 0);
		flash_4b_cfg(flash, info->jedec_id, 0);
	}

	/* prefer "small sector" erase if possible */
	if (info->flags & SECT_4K) {
		flash->erase_opcode = OPCODE_BE_4K;
		flash->mtd.erasesize = 4096;
	} else {
		flash->erase_opcode = OPCODE_SE;
		flash->mtd.erasesize = info->sector_size;
	}

#ifndef CONFIG_SPI_ASTSMC
	reset_flash(flash);
#endif

#ifdef CONFIG_SPI_ASTSMC
	dev_info(&spi->dev, "%s (%d Kbytes)\n", info->name,
			flash->mtd.size / 1024);
#endif

	DEBUG(MTD_DEBUG_LEVEL2,
		"mtd .name = %s, .size = 0x%.8x (%uMiB) "
			".erasesize = 0x%.8x (%uKiB) .numeraseregions = %d\n",
		flash->mtd.name,
		flash->mtd.size, flash->mtd.size / (1024*1024),
		flash->mtd.erasesize, flash->mtd.erasesize / 1024,
		flash->mtd.numeraseregions);

	if (flash->mtd.numeraseregions)
		for (i = 0; i < flash->mtd.numeraseregions; i++)
			DEBUG(MTD_DEBUG_LEVEL2,
				"mtd.eraseregions[%d] = { .offset = 0x%.8x, "
				".erasesize = 0x%.8x (%uKiB), "
				".numblocks = %d }\n",
				i, flash->mtd.eraseregions[i].offset,
				flash->mtd.eraseregions[i].erasesize,
				flash->mtd.eraseregions[i].erasesize / 1024,
				flash->mtd.eraseregions[i].numblocks);


	/* partitions should match sector boundaries; and it may be good to
	 * use readonly partitions for writeprotected sectors (BP2..BP0).
	 */
	if (mtd_has_partitions()) {
		struct mtd_partition	*parts = NULL;
		int			nr_parts = 0;

#ifdef CONFIG_MTD_CMDLINE_PARTS
		static const char *part_probes[] = { "cmdlinepart", NULL, };

		nr_parts = parse_mtd_partitions(&flash->mtd,
				part_probes, &parts, 0);
#endif

		if (nr_parts <= 0 && data && data->parts) {
			parts = data->parts;
			nr_parts = data->nr_parts;
		}

		if (nr_parts > 0) {
			for (i = 0; i < nr_parts; i++) {
				DEBUG(MTD_DEBUG_LEVEL2, "partitions[%d] = "
					"{.name = %s, .offset = 0x%.8x, "
						".size = 0x%.8x (%uKiB) }\n",
					i, parts[i].name,
					parts[i].offset,
					parts[i].size,
					parts[i].size / 1024);
			}
			flash->partitioned = 1;
			status = add_mtd_partitions(&flash->mtd, parts, nr_parts);
			if (status != 0)
				goto error_mtd;
			return 0;
		}
	}
#ifdef CONFIG_SPI_ASTSMC
	else if (data->nr_parts)
		dev_warn(&spi->dev, "ignoring %d default partitions on %s\n",
				data->nr_parts, data->name);
#endif
	status = add_mtd_device(&flash->mtd);
	if (status == 1)
	{
		status = -ENODEV;
		goto error_mtd;
	}
	return 0;

#ifdef CONFIG_SPI_ASTSMC
error_mtd:
	kfree(flash);
error_unknown_flash:
	iounmap(astsmc_spi_flash->vir_dev_base);
error_ioremp:
	release_mem_region(astsmc_spi_flash->phy_dev_base, astsmc_spi_flash->dev_len);
error_request_memory_region:
	return status;
#else
error_mtd:
	printk("error_mtd\n");
error_unknown_flash:
	printk("error_unknown_flash\n");
	iounmap(flash->SPI_Flash_base);
error_ioremp_flash:
	printk("error_ioremp_flash\n");
	iounmap(flash->AST_SMC_base);
error_ioremp_smc:
	printk("error_ioremp_smc\n");
	return status;
#endif
}

#ifdef CONFIG_SPI_ASTSMC
static int __devexit m25p_remove(struct spi_device *spi)
#else
static int __devexit m25p_remove(struct platform_device *pdev)
#endif
{
#ifdef CONFIG_SPI_ASTSMC
	struct m25p	*flash = dev_get_drvdata(&spi->dev);
#else
	struct m25p *flash = pdev->dev.driver_data;
#endif
	int		status;

	/* Clean up MTD stuff. */
	if (mtd_has_partitions() && flash->partitioned)
		status = del_mtd_partitions(&flash->mtd);
	else
		status = del_mtd_device(&flash->mtd);

#ifdef CONFIG_SPI_ASTSMC
	if (status == 0)
		kfree(flash);
#endif

	return 0;
}

#ifdef CONFIG_SPI_ASTSMC
static struct spi_driver m25p80_driver = {
	.driver = {
		.name	= "m25p80",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe	= m25p_probe,
	.remove	= __devexit_p(m25p_remove),

	/* REVISIT: many of these chips have deep power-down modes, which
	 * should clearly be entered on suspend() to minimize power use.
	 * And also when they're otherwise idle...
	 */
};
#else
static const char driver_name[] = "m25p80";
static void the_pdev_release(struct device *dev)
{
	return;
}
static struct platform_device m25p80_dev = {
	/* should be the same name as driver_name */
	.name = (char *)driver_name,
	.id = -1,
	.dev = {
		.release = the_pdev_release,
		.driver_data = NULL,
	},
};

static struct platform_driver m25p80_driver = {
    .probe = m25p_probe,
    .remove	= __devexit_p(m25p_remove),
//steven:need to confirm    .suspend = NULL,
//steven:need to confirm    .resume	= NULL,
    .driver	= {
        .name = (char *) driver_name,
        .owner = THIS_MODULE,
    },
};
#endif

#define MX25U1632F_CHIP	1

#define __DSPModReg32(addr, data, mask) \
		*(volatile u32 *)(IO_ADDRESS(addr)) = \
	(((*(volatile u32 *)(IO_ADDRESS(addr)))&(~(mask))) \
		| (u32)((data)&(mask)))
#define ClrRegBits(addr, bit_mask) __DSPModReg32(addr, 0, bit_mask)
#define SetRegBits(addr, bit_mask) __DSPModReg32(addr, bit_mask, bit_mask)
#define ReadMemoryLong(baseaddress,offset)        (*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))

#define GPIO_MUX_SCU90 			0x1E6E2090
#define GPIO_MUX_SCU94 			0x1E6E2094

#define GPIOE_H_DIR_REG			0x1e780024
#define GPIOE_H_DATA_REG		0x1e780020

static void spi_switch_init(void)
{
	u32 bit_mask = 1;
	u32 bit_clear = 0x000000c0;
	u32 bit_set = 0x03000000;
#ifdef CONFIG_ARCH_AST1500_HOST
	ClrRegBits(GPIO_MUX_SCU90,bit_clear);
	ClrRegBits(GPIO_MUX_SCU94,bit_mask<<5);

	SetRegBits(GPIOE_H_DIR_REG,bit_set);
#endif
}

static void spi_switch_low(void)
{
	u32 bit_mask = 1;
	u32 bit_clear = 0x03000000;
#ifdef CONFIG_ARCH_AST1500_HOST
	ClrRegBits(GPIOE_H_DATA_REG,bit_clear);
#endif
}

static void spi_switch_high(void)
{
	u32 bit_mask = 1;
	u32 bit_set = 0x03000000;
#ifdef CONFIG_ARCH_AST1500_HOST
	SetRegBits(GPIOE_H_DATA_REG,bit_set);
#endif
}

static void spi_switch_to_mx25u1632f(void)
{	
	spi_switch_init();
	spi_switch_high();
}

static void spi_switch_to_default(void)
{	
	spi_switch_init();
	spi_switch_low();
}

#ifdef MX25U1632F_CHIP
#define MX25U1632F_OP_WREN 			0x06
#define MX25U1632F_OP_RDSR 			0x05
#define MX25U1632F_OP_READ 			0x03
#define MX25U1632F_OP_EARSE 		0x20
#define MX25U1632F_OP_EARSE_ALL 	0x60

#define MX25U1632F_SR_WEL 2
#define MX25U1632F_SR_WIP 1

//Because the hardware passes through two SPI switches, the speed can only reach 15MB
/*	
	SPI drive clock register correspondence
	0x00	--	HCLK/16
	0x01	--	HCLK/14
	0x02	--	HCLK/12
	0x03	--	HCLK/10
	0x04	--	HCLK/8
	0x05	--	HCLK/6
	0x06	--	HCLK/4
	0x07	--	HCLK/2
	0x08	--	HCLK/15
	0x09	--	HCLK/13
	0x0a	--	HCLK/11
	0x0b	--	HCLK/9
	0x0c	--	HCLK/7
	0x0d	--	HCLK/5
	0x0e	--	HCLK/3
	0x0f	--	HCLK
*/
#define MX25U1632F_TCK_WRITE		0x00
#define MX25U1632F_TCK_EARSE		0x00
#define MX25U1632F_TCK_READ			0x00

static void mx25u1632f_write_reg(struct m25p *flash, u8 opcode)
{
	u32 val;

	val = (MX25U1632F_TCK_WRITE << 8) | CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = opcode;
	udelay(10);

	val = (MX25U1632F_TCK_WRITE << 8) | CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);
}

static void mx25u1632f_read_reg(struct m25p *flash, u8 opcode, u8 *data, int len)
{
	u32 val, i;

	val = (MX25U1632F_TCK_WRITE << 8) | CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = opcode;
	udelay(10);

	for (i = 0; i < len; i++) {
		*data = *(volatile u8 *)(flash->SPI_Flash_base);
		data++;
	}

	val = (MX25U1632F_TCK_WRITE << 8) | CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);
}

static u8 mx25u1632f_read_sr(struct m25p *flash)
{
	u8 sr;

	mx25u1632f_read_reg(flash, MX25U1632F_OP_RDSR, &sr, 1);
	return sr;
}

static inline void mx25u1632f_write_enable(struct m25p *flash)
{
	u8 code = MX25U1632F_OP_WREN;
	u8 sr = 0;
	mx25u1632f_write_reg(flash, code);

#if defined(STATUS_REG_CHECK)
	while (!(MX25U1632F_SR_WEL & mx25u1632f_read_sr(flash)))
	{
		//msleep(10);
	}
#endif
}

static void mx25u1632f_write_buffer(struct m25p *flash, const u8 *src, u32 addr, int len)
{
	u32	j;
	u32	ulCtrlData;
	spi_switch_to_mx25u1632f();
	mx25u1632f_write_enable(flash);

	ulCtrlData  = (MX25U1632F_TCK_WRITE << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(OPCODE_PP);
	udelay(10);

	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0xff0000) >> 16);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x00ff00) >> 8);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x0000ff));
	udelay(10);

	for (j = 0; j < len; j++) {
		barrier();
		*(u8 *)(flash->SPI_Flash_base) = *(u8 *)(src++);
		udelay(10);
	}

	ulCtrlData  = (MX25U1632F_TCK_WRITE << 8);
	ulCtrlData |= CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	barrier();
#if defined(STATUS_REG_CHECK)
	while (MX25U1632F_SR_WIP & mx25u1632f_read_sr(flash))
	{
		//msleep(10);
	}
#endif
	spi_switch_to_default();
}

static void mx25u1632f_earse_chip_all(struct m25p *flash)
{
	u32	ulCtrlData;

	spi_switch_to_mx25u1632f();

	mx25u1632f_write_enable(flash);
	ulCtrlData	= (MX25U1632F_TCK_EARSE << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(MX25U1632F_OP_EARSE_ALL);
	udelay(10);

	ulCtrlData	= (MX25U1632F_TCK_EARSE << 8);
	ulCtrlData |= CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	barrier();
#if defined(STATUS_REG_CHECK)
	while (MX25U1632F_SR_WIP & mx25u1632f_read_sr(flash))
	{
		//msleep(10);
	}
#endif
	spi_switch_to_default();
}

static void mx25u1632f_earse_buffer(struct m25p *flash, u32 offset)
{
	u32	ulCtrlData;

	mx25u1632f_write_enable(flash);
	ulCtrlData	= (MX25U1632F_TCK_EARSE << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(MX25U1632F_OP_EARSE);
	udelay(10);

	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0xff0000) >> 16);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0x00ff00) >> 8);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((offset & 0x0000ff));
	udelay(10);
	
	ulCtrlData	= (MX25U1632F_TCK_EARSE << 8);
	ulCtrlData |= CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	barrier();
#if defined(STATUS_REG_CHECK)
	while (MX25U1632F_SR_WIP & mx25u1632f_read_sr(flash))
	{
		//msleep(10);
	}
#endif
}

static void mx25u1632f_read_buffer(struct m25p *flash, u32 addr, u8 *data, int len)
{
	u32	i,val;
	u32	ulCtrlData;
	spi_switch_to_mx25u1632f();
	ulCtrlData  = (MX25U1632F_TCK_WRITE << 8);
	ulCtrlData |= CE_LOW | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = ulCtrlData;
	udelay(200);

	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)(MX25U1632F_OP_READ);
	udelay(10);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0xff0000) >> 16);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x00ff00) >> 8);
	barrier();
	*(volatile u8 *)(flash->SPI_Flash_base) = (u8)((addr & 0x0000ff));
	udelay(10);

	for (i = 0; i < len; i++) {
		*data = *(volatile u8 *)(flash->SPI_Flash_base);
		data++;
	}

	val = (MX25U1632F_TCK_WRITE << 8) | CE_HIGH | USERMODE;
	*(volatile u32 *)(flash->AST_SMC_config_reg_addr) = val;
	udelay(200);
	spi_switch_to_default();
}

static ssize_t store_mx25u1632f_read(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct m25p *flash = (struct m25p *)dev->driver_data;
	uint32_t addr = 0;
	uint32_t len = 0;
	u8 data[4] = {0};
	u8 i = 0;
	sscanf(buf, "%x", &addr);
	down(&flash->lock);
	mx25u1632f_read_buffer(flash,addr,data,4);
	up(&flash->lock);
	for(i = 0;i<4;i++)
	{
		printk("data[%d] = 0x%x\n",i,data[i]);
	}

	return count;
}
DEVICE_ATTR(mx25u1632f_read, (S_IWUSR), NULL, store_mx25u1632f_read);

static ssize_t store_mx25u1632f_write_file(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct m25p *flash = (struct m25p *)dev->driver_data;
	static uint32_t flash_addr = 0x00;
	down(&flash->lock);
	mx25u1632f_write_buffer(flash,(u8*)buf,flash_addr,count);
	up(&flash->lock);
	flash_addr += count;

	return count;
}
DEVICE_ATTR(mx25u1632f_write_file, (S_IWUSR), NULL, store_mx25u1632f_write_file);

static ssize_t store_mx25u1632f_earse_all(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct m25p *flash = (struct m25p *)dev->driver_data;
	down(&flash->lock);
	mx25u1632f_earse_chip_all(flash);
	up(&flash->lock);
	return count;
}
DEVICE_ATTR(mx25u1632f_earse_all, (S_IWUSR), NULL, store_mx25u1632f_earse_all);

static struct attribute *dev_attrs[] = {
	&dev_attr_mx25u1632f_read.attr,
	&dev_attr_mx25u1632f_write_file.attr,
	&dev_attr_mx25u1632f_earse_all.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};
#endif

static int m25p80_init(void)
{
#ifdef CONFIG_SPI_ASTSMC
	return spi_register_driver(&m25p80_driver);
#else
	//steven:need to confirm
	struct m25p	*flash;
	int ret = 0;

	ret = platform_driver_register(&m25p80_driver);
	if (ret < 0) {
		printk("platform_driver_register m25p80_driver err\n");
		goto err_reg_platform_driver;
	}
	printk("m25p80 driver registered\n");

	m25p80_dev.dev.driver_data = kzalloc(sizeof *flash, GFP_KERNEL);
	if (m25p80_dev.dev.driver_data == NULL)
	{
		printk("failed to allocate struct m25p\n");
		ret = -ENOMEM;
		goto err_alloc_driver_data;
	}

	printk("register m25p80 device\n");
	ret = platform_device_register(&m25p80_dev);
	if (ret < 0) {
		printk("platform_device_register m25p80_dev err\n");
		goto err_reg_platform_device;
	}
#ifdef MX25U1632F_CHIP
	ret = sysfs_create_group(&m25p80_dev.dev.kobj, &dev_attr_group);
	if (ret) {
		printk("can't create sysfs files\n");
		goto err_reg_platform_device;

	}
#endif
	printk("m25p80 device registered\n");

	printk("m25p80.ko loaded\n");
	return 0;

err_reg_platform_device:
	if (m25p80_dev.dev.driver_data)
	{
		kfree(m25p80_dev.dev.driver_data);
		m25p80_dev.dev.driver_data = NULL;
	}
err_alloc_driver_data:
	platform_driver_unregister(&m25p80_driver);
err_reg_platform_driver:
	return ret;
#endif
}

static void m25p80_exit(void)
{
#ifdef CONFIG_SPI_ASTSMC
	spi_unregister_driver(&m25p80_driver);
#else
	platform_device_unregister(&m25p80_dev);
	if (m25p80_dev.dev.driver_data)
	{
		kfree(m25p80_dev.dev.driver_data);
		m25p80_dev.dev.driver_data = 0;
	}
	platform_driver_unregister(&m25p80_driver);
	printk("m25p80.ko unloaded\n");
#endif
}

#ifdef CONFIG_SPI_ASTSMC
module_init (m25p80_init);
#else
#ifndef MODULE
//steven:need to confirm
subsys_initcall(m25p80_init);
#else
module_init (m25p80_init);
#endif
#endif
module_exit(m25p80_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mike Lavender");
MODULE_DESCRIPTION("MTD SPI driver for ST M25Pxx flash chips");
