/*
 * AST SMC SPI Driver
 *
 * Copyright 2004-2007 ASpeed Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>

#include <asm/hardware.h>
#include <asm/io.h>

#include <asm/arch/astsmc_spi.h>

#define DRV_AUTHOR	"Steven Ke"
#define DRV_DESC	"ASpeed SMC SPI Controller Driver"

MODULE_AUTHOR("Steven Ke");
MODULE_DESCRIPTION("ASpeed SMC SPI Controller Driver");
MODULE_LICENSE("GPL");

#define START_STATE	((void *)0)
#define RUNNING_STATE	((void *)1)
#define DONE_STATE	((void *)2)
#define ERROR_STATE	((void *)-1)
#define QUEUE_RUNNING	0
#define QUEUE_STOPPED	1

struct driver_data {
	/* SPI framework hookup */
	struct spi_master *master;

	/* Regs base of SPI controller */
	void __iomem *cntl_base;

	/* Driver message queue */
	struct workqueue_struct *workqueue;
	struct work_struct pump_messages;
	spinlock_t lock;
	struct list_head queue;
	int busy;
	int run;

	/* Message Transfer pump */
	struct tasklet_struct pump_transfers;

	/* Current message transfer state info */
	struct spi_message *cur_msg;
	struct spi_transfer *cur_transfer;
	struct chip_data *cur_chip;
	size_t len;
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;

	void (*write) (struct driver_data *);
	void (*read) (struct driver_data *);
};

struct chip_data {
	u8 chip_select_num;
	void (*write) (struct driver_data *);
	void (*read) (struct driver_data *);
};

#define CMD_MASK		0xFFFFFFF8
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

#define USERMODE		0x03
#define CE_LOW			0x00
#define CE_HIGH			0x04

/* Chip select operation functions for cs_change flag */
#if 0//steven:to do???
static void cs_active(struct driver_data *drv_data, struct chip_data *chip)
{
}

static void cs_deactive(struct driver_data *drv_data, struct chip_data *chip)
{
}
#endif

/* stop controller and re-config current chip*/
static void restore_state(struct driver_data *drv_data)
{
#if 0//steven:to do???
#endif
}

static void u8_writer(struct driver_data *drv_data)
{
	volatile void *addr;

//	printk("u8_writer (len = %d)\n", drv_data->len);
	addr = ((struct astsmc_spi_device *)(drv_data->cur_msg->spi->controller_data))->vir_dev_base;
//	printk("addr = %08X\n", (u32)addr);
	while (drv_data->tx < drv_data->tx_end) {
		iowrite8((*(u8 *)(drv_data->tx)), addr);
//		printk("%02X ", (*(u8 *)(drv_data->tx)));
			cpu_relax();
		++drv_data->tx;
	}
//	printk("\n");
#if 0//steven:test
	udelay(10);
#endif
}

static void u8_reader(struct driver_data *drv_data)
{
	volatile void *addr;

//	printk("u8_reader (len = %d)\n", drv_data->len);
	addr = ((struct astsmc_spi_device *)(drv_data->cur_msg->spi->controller_data))->vir_dev_base;
//	printk("addr = %08X\n", (u32)addr);
	while (drv_data->rx < drv_data->rx_end) {
		(*(u8 *)(drv_data->rx)) = ioread8(addr);
//		printk("%02X ", (*(u8 *)(drv_data->rx)));
		cpu_relax();
		drv_data->rx++;
	}
//	printk("\n");
}

/* test if ther is more transfer to be done */
static void *next_transfer(struct driver_data *drv_data)
{
	struct spi_message *msg = drv_data->cur_msg;
	struct spi_transfer *trans = drv_data->cur_transfer;

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		drv_data->cur_transfer =
		    list_entry(trans->transfer_list.next,
			       struct spi_transfer, transfer_list);
		return RUNNING_STATE;
	} else
		return DONE_STATE;
}

/*
 * caller already set message->status;
 * dma and pio irqs are blocked give finished message back
 */
static void giveback(struct driver_data *drv_data)
{
	struct chip_data *chip = drv_data->cur_chip;
	struct spi_transfer *last_transfer;
	unsigned long flags;
	struct spi_message *msg;
	u32 val;

//	printk("enter giveback\n");
	spin_lock_irqsave(&drv_data->lock, flags);
	msg = drv_data->cur_msg;
//	printk("msg = %08X\n", msg);
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;
	drv_data->cur_chip = NULL;
//	printk("before queue_work\n");
	queue_work(drv_data->workqueue, &drv_data->pump_messages);
//	printk("after queue_work\n");
	spin_unlock_irqrestore(&drv_data->lock, flags);

	last_transfer = list_entry(msg->transfers.prev,
				   struct spi_transfer, transfer_list);
//	printk("last_transfer = %08X\n", last_transfer);

	msg->state = NULL;

	/* disable chip select signal. And not stop spi in autobuffer mode */
	//steven:deactivate CS here
	val = CE_HIGH | USERMODE;
//	printk("deactivate CS\n");
	iowrite32(val, drv_data->cntl_base + 4 * (chip->chip_select_num + 1));
#if 0//steven:test
	udelay(200);
#endif

	if (msg->complete)
	{
		msg->complete(msg->context);
	}
//	printk("leave giveback\n");
}

static void pump_transfers(unsigned long data)
{
	struct driver_data *drv_data = (struct driver_data *)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct chip_data *chip = NULL;
	u32 tranf_success = 1;

//	printk("pump_transfers\n");
	/* Get current state information */
	message = drv_data->cur_msg;
//	printk("message = %08X\n", message);
	transfer = drv_data->cur_transfer;
//	printk("transfer = %08X\n", transfer);
	chip = drv_data->cur_chip;

	/*
	 * if msg is error or done, report it back using complete() callback
	 */

	 /* Handle for abort */
	if (message->state == ERROR_STATE) {
//		printk("message->state == ERROR_STATE\n");
		message->status = -EIO;
		giveback(drv_data);
		return;
	}

	/* Handle end of message */
	if (message->state == DONE_STATE) {
//		printk("message->state == DONE_STATE\n");
		message->status = 0;
		giveback(drv_data);
		return;
	}

	/* Delay if requested at end of transfer */
	if (message->state == RUNNING_STATE) {
		previous = list_entry(transfer->transfer_list.prev,
				      struct spi_transfer, transfer_list);
		if (previous->delay_usecs)
			udelay(previous->delay_usecs);
	}

//	printk("check and set transfer buffers\n");
	if (transfer->tx_buf != NULL) {
//		printk("transfer->tx_buf[0] = %02X\n", ((u8 *)(transfer->tx_buf))[0]);
		drv_data->tx = (void *)transfer->tx_buf;
		drv_data->tx_end = drv_data->tx + transfer->len;
	} else {
		drv_data->tx = NULL;
	}

	if (transfer->rx_buf != NULL) {
		drv_data->rx = transfer->rx_buf;
		drv_data->rx_end = drv_data->rx + transfer->len;
	} else {
		drv_data->rx = NULL;
	}

	drv_data->write = chip->write;
	drv_data->read = chip->read;
	drv_data->len = transfer->len;

	//activate CS here
	if (message->state == START_STATE){
		u32 val;
//		printk("activate CS\n");
		if (transfer->tx_buf != NULL){
#if 0
//			printk("\nchip->tCK_Erase = %d\n", chip->tCK_Erase);
//			printk("\nchip->tCK_Read = %d\n", chip->tCK_Read);
			switch (((u8 *)(transfer->tx_buf))[0]){
			case OPCODE_RDID:
				val = (ioread32(drv_data->cntl_base + 4 * (chip->chip_select_num + 1)) & CMD_MASK);
 			case OPCODE_BE_4K:/* Erase 4KiB block */
 			case OPCODE_BE_32K:/* Erase 32KiB block */
 			case OPCODE_SE:/* Sector erase (usually 64KiB) */
				val = (((struct astsmc_spi_device *)(message->spi->controller_data))->tCK_Erase << 8);
				break;
			default:
				val = (((struct astsmc_spi_device *)(message->spi->controller_data))->tCK_Read << 8);
			}
			val |= (CE_LOW | USERMODE);
#else
			val = (CE_LOW | USERMODE);
#endif
			iowrite32(val, drv_data->cntl_base + 4 * (chip->chip_select_num + 1));
#if 0//steven:test
			udelay(200);
#endif
		}
		else
			printk("rx w/o tx first!!!\n");
	}

	/* speed and width has been set on per message */
	message->state = RUNNING_STATE;


	{
		/* IO mode write then read */
		if (drv_data->tx != NULL) {
			/* write only half duplex */
			drv_data->write(drv_data);

			if (drv_data->tx != drv_data->tx_end)
				tranf_success = 0;
		} else if (drv_data->rx != NULL) {
			/* read only half duplex */
			drv_data->read(drv_data);
			if (drv_data->rx != drv_data->rx_end)
				tranf_success = 0;
		}

		if (!tranf_success) {
			message->state = ERROR_STATE;
		} else {
			/* Update total byte transfered */
			message->actual_length += drv_data->len;

			/* Move to next transfer of this msg */
			message->state = next_transfer(drv_data);
		}

//		printk("schedule next transfer\n");
		/* Schedule next transfer tasklet */
		tasklet_schedule(&drv_data->pump_transfers);
	}
}

/* pop a msg from queue and kick off real transfer */
static void pump_messages(struct work_struct *work)
{
	struct driver_data *drv_data;
	unsigned long flags;

//	printk("pump_messages\n");
	drv_data = container_of(work, struct driver_data, pump_messages);

	/* Lock queue and check for queue work */
	spin_lock_irqsave(&drv_data->lock, flags);
	if (list_empty(&drv_data->queue) || drv_data->run == QUEUE_STOPPED) {
		/* pumper kicked off but no work to do */
		drv_data->busy = 0;
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

	/* Make sure we are not already running a message */
	if (drv_data->cur_msg) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return;
	}

//	printk("extract message\n");
	/* Extract head of queue */
	drv_data->cur_msg = list_entry(drv_data->queue.next,
				       struct spi_message, queue);
//	printk("drv_data->cur_msg = %08X\n", drv_data->cur_msg);

	/* Setup the SSP using the per chip configuration */
	drv_data->cur_chip = spi_get_ctldata(drv_data->cur_msg->spi);
	restore_state(drv_data);

	list_del_init(&drv_data->cur_msg->queue);

	/* Initial message state */
//steven	drv_data->cur_msg->state = START_STATE;
	drv_data->cur_transfer = list_entry(drv_data->cur_msg->transfers.next,
					    struct spi_transfer, transfer_list);
//	printk("drv_data->cur_transfer = %08X\n", drv_data->cur_transfer);
//	printk("schedule transfer\n");
	/* Mark as busy and launch transfers */
	tasklet_schedule(&drv_data->pump_transfers);
	drv_data->busy = 1;
	spin_unlock_irqrestore(&drv_data->lock, flags);
}

/*
 * got a msg to transfer, queue it in drv_data->queue.
 * And kick off message pumper
 */
static int transfer(struct spi_device *spi, struct spi_message *msg)
{
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);
	unsigned long flags;

//	printk("transfer (msg = %08X)\n", msg);
	spin_lock_irqsave(&drv_data->lock, flags);

	if (drv_data->run == QUEUE_STOPPED) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -ESHUTDOWN;
	}

	msg->actual_length = 0;
	msg->status = -EINPROGRESS;
	msg->state = START_STATE;

	dev_dbg(&spi->dev, "adding an msg in transfer() \n");
//	printk("enqueue message\n");
	list_add_tail(&msg->queue, &drv_data->queue);

	if (drv_data->run == QUEUE_RUNNING && !drv_data->busy)
		queue_work(drv_data->workqueue, &drv_data->pump_messages);

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return 0;
}

/* first setup for new devices */
static int setup(struct spi_device *spi)
{
	struct chip_data *chip;

//	printk("setup\n");
	/* Only alloc (or use chip_info) on first setup */
	chip = spi_get_ctldata(spi);
	if (chip == NULL) {
		chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;
	}

	chip->chip_select_num = spi->chip_select;
	chip->read = u8_reader;
	chip->write = u8_writer;

	spi_set_ctldata(spi, chip);

	return 0;
}

/*
 * callback for spi framework.
 * clean driver specific data
 */
static void cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);
#if 0//steven:to do???
#endif
	kfree(chip);
}

static inline int init_queue(struct driver_data *drv_data)
{
	INIT_LIST_HEAD(&drv_data->queue);
	spin_lock_init(&drv_data->lock);

	drv_data->run = QUEUE_STOPPED;
	drv_data->busy = 0;

	/* init transfer tasklet */
	tasklet_init(&drv_data->pump_transfers,
		     pump_transfers, (unsigned long)drv_data);

	/* init messages workqueue */
	INIT_WORK(&drv_data->pump_messages, pump_messages, &drv_data->pump_messages);
	drv_data->workqueue =
	    create_singlethread_workqueue(drv_data->master->cdev.dev->bus_id);
	if (drv_data->workqueue == NULL)
		return -EBUSY;

	return 0;
}

static inline int start_queue(struct driver_data *drv_data)
{
	unsigned long flags;

	spin_lock_irqsave(&drv_data->lock, flags);

	if (drv_data->run == QUEUE_RUNNING || drv_data->busy) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		return -EBUSY;
	}

	drv_data->run = QUEUE_RUNNING;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;
	drv_data->cur_chip = NULL;
	spin_unlock_irqrestore(&drv_data->lock, flags);

	queue_work(drv_data->workqueue, &drv_data->pump_messages);

	return 0;
}

static inline int stop_queue(struct driver_data *drv_data)
{
	unsigned long flags;
	unsigned limit = 500;
	int status = 0;

	spin_lock_irqsave(&drv_data->lock, flags);

	/*
	 * This is a bit lame, but is optimized for the common execution path.
	 * A wait_queue on the drv_data->busy could be used, but then the common
	 * execution path (pump_messages) would be required to call wake_up or
	 * friends on every SPI message. Do this instead
	 */
	drv_data->run = QUEUE_STOPPED;
	while (!list_empty(&drv_data->queue) && drv_data->busy && limit--) {
		spin_unlock_irqrestore(&drv_data->lock, flags);
		msleep(10);
		spin_lock_irqsave(&drv_data->lock, flags);
	}

	if (!list_empty(&drv_data->queue) || drv_data->busy)
		status = -EBUSY;

	spin_unlock_irqrestore(&drv_data->lock, flags);

	return status;
}

static inline int destroy_queue(struct driver_data *drv_data)
{
	int status;

	status = stop_queue(drv_data);
	if (status != 0)
		return status;

	destroy_workqueue(drv_data->workqueue);

	return 0;
}

static int __init astsmc_spi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct spi_master *master;
	struct driver_data *drv_data = 0;
	int status = 0;

//	printk("\nastsmc_spi_probe\n");
	/* Allocate master with space for drv_data */
	master = spi_alloc_master(dev, sizeof(struct driver_data) + 16);
	if (!master) {
		dev_err(&pdev->dev, "can not alloc spi_master\n");
		return -ENOMEM;
	}

	drv_data = spi_master_get_devdata(master);
	drv_data->master = master;

	master->bus_num = pdev->id;
//	printk("\nmaster->bus_num = %d\n", master->bus_num);
	master->cleanup = cleanup;
	master->setup = setup;
	master->transfer = transfer;

	/* Find and map our resources */
	drv_data->cntl_base = ioremap(ASPEED_SMC_CONTROLLER_BASE, 0x2000000);
	if (drv_data->cntl_base == NULL) {
		dev_err(dev, "Cannot map SMC\n");
		status = -ENXIO;
		goto out_error_ioremap;
	}

	/* Initial and start queue */
	status = init_queue(drv_data);
	if (status != 0) {
		dev_err(dev, "problem initializing queue\n");
		goto out_error_queue_alloc;
	}

	status = start_queue(drv_data);
	if (status != 0) {
		dev_err(dev, "problem starting queue\n");
		goto out_error_queue_alloc;
	}

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, drv_data);
	status = spi_register_master(master);
	if (status != 0) {
		dev_err(dev, "problem registering spi master\n");
		goto out_error_queue_alloc;
	}
	
	return status;

out_error_queue_alloc:
	destroy_queue(drv_data);
#if 0//steven
out_error_no_dma_ch:
#endif
	iounmap((void *) drv_data->cntl_base);
out_error_ioremap:
	spi_master_put(master);

	return status;
}

/* stop hardware and remove the driver */
static int __devexit astsmc_spi_remove(struct platform_device *pdev)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	int status = 0;

	if (!drv_data)
		return 0;

	/* Remove the queue */
	status = destroy_queue(drv_data);
	if (status != 0)
		return status;

	/* Disconnect from the SPI framework */
	spi_unregister_master(drv_data->master);

	/* Prevent double remove */
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver astsmc_spi_driver = {
	.driver	= {
		.name	= "astsmc-spi-master",
		.owner	= THIS_MODULE,
	},
	.probe		= astsmc_spi_probe,
	.suspend	= NULL,//steven:to do???
	.resume		= NULL,//steven:to do???
	.remove		= astsmc_spi_remove,
};

static int __init astsmc_spi_init(void)
{
	return platform_driver_register(&astsmc_spi_driver);
}
module_init(astsmc_spi_init);

static void __exit astsmc_spi_exit(void)
{
	platform_driver_unregister(&astsmc_spi_driver);
}
module_exit(astsmc_spi_exit);
