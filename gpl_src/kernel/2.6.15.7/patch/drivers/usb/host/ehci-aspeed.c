/*
 * drivers/usb/host/ehci-aspeed.c
 *
 * ASPEED EHCI driver based on ehci-orion.c
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/arch/ast-scu.h>

#ifndef ehci_readl
#define ehci_readl(ehci, reg) readl(reg)
#endif

//#define rdl(off)	__raw_readl(hcd->regs + (off))
//#define wrl(off, val)	__raw_writel((val), hcd->regs + (off))

static int ehci_aspeed_setup(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval;

	retval = ehci_halt(ehci);
	if (retval)
		return retval;

	/*
	 * data structure init
	 */
	retval = ehci_init(hcd);
	if (retval)
		return retval;

	//hcd->has_tt = 0; //Gary says no

	ehci_reset(ehci);
	ehci_port_power(ehci, 0);

	return retval;
}

static const struct hc_driver ehci_aspeed_hc_driver = {
	.description = hcd_name,
	.product_desc = "ASPEED EHCI",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset = ehci_aspeed_setup,
	.start = ehci_run,
	.stop = ehci_stop,
//	.shutdown = ehci_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data = ehci_hub_status_data,
	.hub_control = ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend = ehci_bus_suspend,
	.bus_resume = ehci_bus_resume,
#endif
//	.relinquish_port = ehci_relinquish_port,
//	.port_handed_over = ehci_port_handed_over,
};


static int __init ehci_aspeed_drv_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	void __iomem *regs = NULL;
	int irq, err;

	if (usb_disabled())
		return -ENODEV;

	printk("Initializing ASPEED-SoC USB Host Controller\n");

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			pdev->dev.bus_id);
		err = -ENODEV;
		goto err1;
	}
	IRQ_SET_HIGH_LEVEL (irq);
	IRQ_SET_LEVEL_TRIGGER (irq);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			pdev->dev.bus_id);
		err = -ENODEV;
		goto err1;
	}

	if (!request_mem_region(res->start, res->end - res->start,
				ehci_aspeed_hc_driver.description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		err = -EBUSY;
		goto err1;
	}

	regs = ioremap_nocache(res->start, res->end - res->start);
	if (regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		err = -EFAULT;
		goto err2;
	}

	hcd = usb_create_hcd(&ehci_aspeed_hc_driver,
			&pdev->dev, pdev->dev.bus_id);
	if (!hcd) {
		err = -ENOMEM;
		goto err3;
	}

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = res->end - res->start;
	hcd->regs = (void __iomem *)regs;

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs +
		HC_LENGTH(ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);
	//hcd->has_tt = 0; //Gary says no
	ehci->sbrn = 0x20; //USB Bus revision number.
#if 0
	printk("rsrc_start=0x%08X, rsrc_len=0x%08X, hcd->regs=0x%08X, ehci->caps=0x%08X, ehci->regs=0x%08X, echi->hcs_params=0x%08X, cap=0x%08X\n",
	       hcd->rsrc_start, hcd->rsrc_len, hcd->regs,
	       ehci->caps, ehci->regs, ehci->hcs_params,
	       ehci_readl(ehci, &ehci->caps->hc_capbase));
#endif

	err = usb_add_hcd(hcd, irq, SA_INTERRUPT);
	if (err)
		goto err4;

	return 0;

err4:
	usb_put_hcd(hcd);
err3:
	iounmap(regs);
err2:
	release_mem_region(res->start, res->end - res->start + 1);
err1:
	dev_err(&pdev->dev, "init %s fail, %d\n",
		pdev->dev.bus_id, err);

	return err;
}

static int __exit ehci_aspeed_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	return 0;
}

MODULE_ALIAS("platform:aspeed-ehci");

static struct platform_driver ehci_aspeed_driver = {
	.probe		= ehci_aspeed_drv_probe,
	.remove		= __exit_p(ehci_aspeed_drv_remove),
	.driver.name	= "aspeed-ehci",
};
