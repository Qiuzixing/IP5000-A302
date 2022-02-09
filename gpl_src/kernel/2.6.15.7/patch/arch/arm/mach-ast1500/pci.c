/*
 * arch/arm/mach-ixp4xx/common-pci.c 
 *
 * IXP4XX PCI routines for all platforms
 *
 * Maintainer: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003 Greg Ungerer <gerg@snapgear.com>
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/dma-mapping.h>
#include <asm/sizes.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <asm/hardware.h>
#include <asm/memory.h>

#if (CONFIG_AST1500_SOC_VER >= 2)
#define DMA_ZONE_SIZE SZ_256M
#else
#define DMA_ZONE_SIZE SZ_16M
#endif

/*
 * PCI cfg an I/O routines are done by programming a 
 * command/byte enable register, and then read/writing
 * the data from a data regsiter. We need to ensure
 * these transactions are atomic or we will end up
 * with corrupt data on the bus or in a driver.
 */
static DEFINE_SPINLOCK(ast1500_pci_lock);

/*
 * Mask table, bits to mask for quantity of size 1, 2 or 4 bytes.
 * 0 and 3 are not valid indexes...
 */
static u32 bytemask[] = {
	/*0*/	0,
	/*1*/	0xff,
	/*2*/	0xffff,
	/*3*/	0,
	/*4*/	0xffffffff,
};

int ast1500_pci_read(u32 addr, u32 cmd, u32* data)
{
	unsigned long flags;

	spin_lock_irqsave(&ast1500_pci_lock, flags);

        RESET_PCI_STATUS;

	*PCI_ADDR_REG = addr;

	/* set up and execute the read */    
	*PCI_CMD_REG = cmd;

	/* the result of the read is now in NP_RDATA */
	*data = *PCI_READ_REG; 

	spin_unlock_irqrestore(&ast1500_pci_lock, flags);
	
	return (CHK_PCI_STATUS);
}

int ast1500_pci_write(u32 addr, u32 cmd, u32 data)
{    
	unsigned long flags;

	spin_lock_irqsave(&ast1500_pci_lock, flags);

        RESET_PCI_STATUS;

	*PCI_ADDR_REG = addr;

	/* set up the write */
	*PCI_CMD_REG = cmd;

	/* execute the write by writing to NP_WDATA */
	*PCI_WRITE_REG = data;

	spin_unlock_irqrestore(&ast1500_pci_lock, flags);
	
	return (CHK_PCI_STATUS);
}

static u32 ast1500_config_addr(u8 bus_num, u16 devfn, int where)
{
	u32 addr;
	if (!bus_num) {
		/* type 0 */
		addr = BIT(32-PCI_SLOT(devfn)) | ((PCI_FUNC(devfn)) << 8) | 
		    (where & ~3);	
	} else {
		/* type 1 */
		addr = (bus_num << 16) | ((PCI_SLOT(devfn)) << 11) | 
			((PCI_FUNC(devfn)) << 8) | (where & ~3) | 1;
	}
	return addr;
}

static u32 byte_lane_enable_bits(u32 n, int size)
{
	if (size == 1)
		return (0xf & ~BIT(n)) << 4;
	if (size == 2)
		return (0xf & ~(BIT(n) | BIT(n+1))) << 4;
	if (size == 4)
		return 0;
	return 0xffffffff;
}

static int ast1500_pci_read_config(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *value)
{
	u32 n, byte_enables, addr, data;
	u8 bus_num = bus->number;

	*value = 0xffffffff;
	n = where % 4;
	byte_enables = byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	addr = ast1500_config_addr(bus_num, devfn, where);
	if (ast1500_pci_read(addr, byte_enables | NP_CMD_CONFIGREAD, &data))
		return PCIBIOS_DEVICE_NOT_FOUND;

	*value = (data >> (8*n)) & bytemask[size];
	
	return PCIBIOS_SUCCESSFUL;
}

static int ast1500_pci_write_config(struct pci_bus *bus,  unsigned int devfn, int where, int size, u32 value)
{
	u32 n, byte_enables, addr, data;
	u8 bus_num = bus->number;

	n = where % 4;
	byte_enables = byte_lane_enable_bits(n, size);
	if (byte_enables == 0xffffffff)
		return PCIBIOS_BAD_REGISTER_NUMBER;

	addr = ast1500_config_addr(bus_num, devfn, where);
	data = value << (8*n);
	if (ast1500_pci_write(addr, byte_enables | NP_CMD_CONFIGWRITE, data))
		return PCIBIOS_DEVICE_NOT_FOUND;

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops pci_ast1500_ops = {
	.read =  ast1500_pci_read_config,
	.write = ast1500_pci_write_config,
};

static struct resource io_mem = {
	.name	= "PCI I/O space",
	.start	= AST1500_PCI_MEM_BASE0,
	.end	= AST1500_PCI_MEM_END0,
	.flags	= IORESOURCE_IO,
};

static struct resource non_mem = {
	.name	= "PCI non-prefetchable",
	.start	= AST1500_PCI_MEM_BASE1,
	.end	= AST1500_PCI_MEM_END1,
	.flags	= IORESOURCE_MEM,
};

/*
 * Only first 16MB of memory can be accessed via PCI.
 * We use GFP_DMA to allocate safe buffers to do map/unmap.
 * This is really ugly and we need a better way of specifying
 * DMA-capable regions of memory.
 */
void __init ast1500_adjust_zones(int node, unsigned long *zone_size,
	unsigned long *zhole_size)
{
	unsigned int sz = DMA_ZONE_SIZE >> PAGE_SHIFT;

	/*
	 * Only adjust if > 16M on current system
	 */
	if (node || (zone_size[0] <= sz))
		return;

	zone_size[1] = zone_size[0] - sz;
	zone_size[0] = sz;
	zhole_size[1] = zhole_size[0];
	zhole_size[0] = 0;
}

/*
 * Setup DMA mask to 16MB on PCI devices. Ignore all other devices.
 */
static int ast1500_pci_platform_notify(struct device *dev)
{
#ifdef CONFIG_DMABOUNCE
	if(dev->bus == &pci_bus_type) {
		*dev->dma_mask =  DMA_ZONE_SIZE - 1;
		dev->coherent_dma_mask = DMA_ZONE_SIZE - 1;
		dmabounce_register_dev(dev, 1024, 4096);		
	}
#endif
	return 0;
}

static int ast1500_pci_platform_notify_remove(struct device *dev)
{
#ifdef CONFIG_DMABOUNCE
	if(dev->bus == &pci_bus_type) {
		dmabounce_unregister_dev(dev);		
	}
#endif
	return 0;
}

int dma_needs_bounce(struct device *dev, dma_addr_t dma_addr, size_t size)
{
#ifdef CONFIG_DMABOUNCE
	return (dev->bus == &pci_bus_type ) && ((dma_addr + size - ASPEED_SDRAM_BASE) >= DMA_ZONE_SIZE);
#else
	return 0;
#endif
}

int __init pci_ast1500_setup(int nr, struct pci_sys_data *sys)
{
	int ret = 0;
        unsigned long reg;
	
	if (nr >= 1)
		return 0;

        /* Enable Host Bridge */
        RESET_PCI_STATUS;

        *CRP_ADDR_REG = ((unsigned long)(NP_CMD_CONFIGWRITE) << 16) | 0x10;
        *CRP_WRITE_REG = BUS_OFFSET;
 	
        *CRP_ADDR_REG = ((unsigned long)(NP_CMD_CONFIGREAD) << 16) | 0x04;
        reg = *CRP_READ_REG;
    
        *CRP_ADDR_REG = ((unsigned long)(NP_CMD_CONFIGWRITE) << 16) | 0x04;
        *CRP_WRITE_REG = reg | 0x07;
           
        /* Resource Map */
	ret = request_resource(&ioport_resource, &io_mem);
	if (ret) {
		printk(KERN_ERR "PCI: unable to allocate I/O "
		       "memory region (%d)\n", ret);
		goto out;
	}
	ret = request_resource(&iomem_resource, &non_mem);
	if (ret) {
		printk(KERN_ERR "PCI: unable to allocate non-prefetchable "
		       "memory region (%d)\n", ret);
		goto release_io_mem;
	}
	
	/*
	 * bus->resource[0] is the IO resource for this bus
	 * bus->resource[1] is the mem resource for this bus
	 * bus->resource[2] is the prefetch mem resource for this bus
	 */
	sys->resource[0] = &io_mem;
	sys->resource[1] = &non_mem;
	sys->resource[2] = NULL;

	platform_notify = ast1500_pci_platform_notify;
	platform_notify_remove = ast1500_pci_platform_notify_remove;
	
	return 1;

release_io_mem:
	release_resource(&io_mem);
	
out:
	return 0;
	        	

}


struct pci_bus *pci_ast1500_scan_bus(int nr, struct pci_sys_data *sys)
{
	return pci_scan_bus(sys->busnr, &pci_ast1500_ops, sys);
}

void __init pci_ast1500_preinit(void)
{
	/* USB Host NEC uPD720101 IRQ Init */		
        VIC2_IRQ_SET_LOW_LEVEL (INT_INTA);
        VIC2_IRQ_SET_LEVEL_TRIGGER (INT_INTA); 
	set_irq_type(INT_INTA, IRQT_LOW);

        VIC2_IRQ_SET_LOW_LEVEL (INT_INTB);
        VIC2_IRQ_SET_LEVEL_TRIGGER (INT_INTB); 
	set_irq_type(INT_INTB, IRQT_LOW);

        VIC2_IRQ_SET_LOW_LEVEL (INT_INTC);
        VIC2_IRQ_SET_LEVEL_TRIGGER (INT_INTC); 
	set_irq_type(INT_INTC, IRQT_LOW);

#if (CONFIG_AST1500_SOC_VER >= 2)
	//I'm not sure if SoC V1 take INTD or not. So, I just add INTD support on V2 code.
        VIC2_IRQ_SET_LOW_LEVEL (INT_INTD);
        VIC2_IRQ_SET_LEVEL_TRIGGER (INT_INTD); 
	set_irq_type(INT_INTD, IRQT_LOW);
#endif
}

/*
 * map the specified device/slot/pin to an IRQ.   Different backplanes may need to modify this.
 */
static int __init ast1500_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	printk("devfn: slot(%d) fnc(%d)\n", PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

#if (CONFIG_AST1500_SOC_VER == 1)
		/* USB Host NEC uPD720101 */
		if (dev->vendor == 0x1033)
		{
		    switch (dev->device)
		    {
		    case 0x0035:
		        if (pin == 1)			/* OHCI 1 */   	
		            return INT_INTA;
		        else				/* OHCI 2 */
		            return INT_INTB;
		    case 0x00e0:			/* EHCI */
		        return INT_INTC;
		    default:
		        return -1;            
		    }
		}
	return -1;
#endif

	/* Following code is based on CONFIG_AST1510_BOARD_EVA_V2 HW configuration. */
	if (PCI_SLOT(dev->devfn) == 12) {
		/* USB Host NEC uPD720101 */
		if (dev->vendor == 0x1033)
		{
		    switch (dev->device)
		    {
		    case 0x0035:
		        if (pin == 1)			/* OHCI 1 */
		            return INT_INTB;
		        else				/* OHCI 2 */
		            return INT_INTC;
		    case 0x00e0:			/* EHCI */
		        return INT_INTD;
		    default:
		        return -1;            
		    }
		}
	}
	if (PCI_SLOT(dev->devfn) == 13) {
		/* USB Host NEC uPD720102. Uses only INTA*/
		if (dev->vendor == 0x1033)
		{
			switch (dev->device)
			{
			case 0x0035:			/* OHCI 1 */
				return INT_INTA;
			case 0x00e0:			/* EHCI */
				return INT_INTA;
			default:
				return -1;
			}
		}
	}
	return -1;
}

static struct hw_pci ast1500_pci __initdata = {
	.swizzle		= NULL,
	.map_irq		= ast1500_map_irq,
	.nr_controllers		= 1,
	.setup			= pci_ast1500_setup,
	.scan			= pci_ast1500_scan_bus,
	.preinit		= pci_ast1500_preinit,
};

static int __init ast1500_pci_init(void)
{
	u32 enable;
	
	enable = __raw_readl(IO_ADDRESS(0x1e6e2070));

#if (CONFIG_AST1500_SOC_VER == 2)
	#if 0//AST1510 A0
	enable &= (1UL << 26);
	#else
	enable &= (1UL << 23);
	#endif
#elif (CONFIG_AST1500_SOC_VER == 1)
	enable &= (1UL << 4);
#else
	enable = 0;
#endif
	/*
	** Bruce150326. Some HW configuration don't need PCI and running
	** pci_common_init() will halt uboot if HW grounded all PCI pins together.
	** We have to check the trpping pin to avoid this condition.
	*/
	if (enable)
		pci_common_init(&ast1500_pci);

	return 0;
}

subsys_initcall(ast1500_pci_init);
