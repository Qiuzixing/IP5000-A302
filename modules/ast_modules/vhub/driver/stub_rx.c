/*
 * $Id: stub_rx.c 76 2008-04-29 04:36:43Z hirofuchi $
 *
 * Copyright (C) 2003-2008 Takahiro Hirofuchi
 *
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 */
#include <linux/sched.h>
#include <asm/arch/ast-scu.h> /* astparam */

#include "usbip_common.h"
#include "stub.h"

#include HCD_HEADER

void start_iso_out(struct priv_ep_info *pei);
void start_iso_in(struct priv_ep_info *pei);

static int is_clear_halt_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	 return (req->bRequest == USB_REQ_CLEAR_FEATURE) &&
		 (req->bRequestType == USB_RECIP_ENDPOINT) &&
		 (req->wValue == USB_ENDPOINT_HALT);
}

static int is_set_interface_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	return (req->bRequest == USB_REQ_SET_INTERFACE) &&
		   (req->bRequestType == USB_RECIP_INTERFACE);
}

static int is_set_configuration_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	return (req->bRequest == USB_REQ_SET_CONFIGURATION) &&
		   (req->bRequestType == USB_RECIP_DEVICE);
}

static int is_reset_device_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	__u16 value;
	__u16 index;

	req = (struct usb_ctrlrequest *) urb->setup_packet;
	value = le16_to_cpu(req->wValue);
	index = le16_to_cpu(req->wIndex);

	if ((req->bRequest == USB_REQ_SET_FEATURE) &&
	    	(req->bRequestType == VHUB_PRIV_REQ_TYPE) &&
		(value = USB_PORT_FEAT_RESET)) {
		dbg_stub_rx("reset_device_cmd, port %u\n", index);
		return 1;
	} else 
		return 0;
}

static int tweak_clear_halt_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	int target_endp;
	int target_dir;
	int target_pipe;
	int ret;

	req = (struct usb_ctrlrequest *) urb->setup_packet;

	/*
	 * The stalled endpoint is specified in the wIndex value. The endpoint
	 * of the urb is the target of this clear_halt request (i.e., control
	 * endpoint).
	 */
	target_endp = le16_to_cpu(req->wIndex) & 0x000f;

	/* the stalled endpoint direction is IN or OUT?. USB_DIR_IN is 0x80. */
	target_dir = le16_to_cpu(req->wIndex) & 0x0080;

	if (target_dir)
		target_pipe = usb_rcvctrlpipe(urb->dev, target_endp);
	else
		target_pipe = usb_sndctrlpipe(urb->dev, target_endp);

	ret = usb_clear_halt(urb->dev, target_pipe);
	if (ret < 0)
		uinfo("clear_halt error: devnum %d endp %d, %d\n",
				urb->dev->devnum, target_endp, ret);
	else
		uinfo("clear_halt done: devnum %d endp %d\n",
				urb->dev->devnum, target_endp);

	return ret;
}

#define req_empty(pei) (list_empty(&(pei)->priv_wait) && list_empty(&(pei)->priv_submitted))

void tweak_stop_iso_out(struct stub_device *sdev, struct priv_ep_info *pei)
{
    unsigned long flags;
    
    spin_lock_irqsave(&sdev->priv_lock, flags);

    pei->intf = -1;
    if (!list_empty(&pei->priv_wait)) {
        //Make sure that we won't be in ISO_IDLE state.
        if (pei->state == ISO_IDLE) {
            pei->state = ISO_STARTING;
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            PREPARE_WORK(&pei->work, (void (*)(void *))(start_iso_out), (void *)pei);
            queue_work(sdev->wq, &pei->work);
            flush_workqueue(sdev->wq);
            spin_lock_irqsave(&sdev->priv_lock, flags);
        } else {
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            //Wait for the "wait" list cleaned up.
            printk("O");
            msleep(ISO_SUBMIT_LOW_BOUND);
            flush_workqueue(sdev->wq);
            spin_lock_irqsave(&sdev->priv_lock, flags);
        }
    }
    /* If there is still pending request. do something here. */
    if (!req_empty(pei)) {
        spin_unlock_irqrestore(&sdev->priv_lock, flags);
        printk("O1");
        msleep(10);
        spin_lock_irqsave(&sdev->priv_lock, flags);

        //Set state to ISO_ERR, the complete routine will change it back to ISO_IDLE.
        if (!req_empty(pei)) {
            pei->state = ISO_ERR;
        }

        //Wait for the state back to ISO_IDLE by itself.
        while (pei->state != ISO_IDLE) {
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            printk("O2");
            msleep(10);
            spin_lock_irqsave(&sdev->priv_lock, flags);
            //This is a very corn case where all requests completed, but state still stay in ISO_STARTED.
            if (req_empty(pei)) {
                pei->state = ISO_IDLE;
            }
        }
    }

    spin_unlock_irqrestore(&sdev->priv_lock, flags);

}

void tweak_stop_iso_in(struct stub_device *sdev, struct priv_ep_info *pei)
{
    unsigned long flags;
    
    spin_lock_irqsave(&sdev->priv_lock, flags);

    pei->intf = -1;
    if (!list_empty(&pei->priv_wait)) {
        //Make sure that we won't be in ISO_IDLE state.
        if (pei->state == ISO_IDLE) {
            pei->state = ISO_STARTING;
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            PREPARE_WORK(&pei->work, (void (*)(void *))(start_iso_in), (void *)pei);
            queue_work(sdev->wq, &pei->work);
            flush_workqueue(sdev->wq);
            spin_lock_irqsave(&sdev->priv_lock, flags);
        } else {
            //in ISO_STARTING, ISO_STARTED, ISO_ERR
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            //Wait for the "wait" list cleaned up.
            printk("w");
            msleep(ISO_SUBMIT_LOW_BOUND);
            flush_workqueue(sdev->wq);
            spin_lock_irqsave(&sdev->priv_lock, flags);
        }
    }
    /* If there is still pending request. do something here. */
    if (!req_empty(pei) || pei->state != ISO_IDLE) {
        spin_unlock_irqrestore(&sdev->priv_lock, flags);
        printk("w1");
        msleep(10);
        spin_lock_irqsave(&sdev->priv_lock, flags);

        //Set state to ISO_ERR, the complete routine will change it back to ISO_IDLE.
        if (!req_empty(pei)) {
            pei->state = ISO_ERR;
        }

        //Wait for the state back to ISO_IDLE by itself.
        while (pei->state != ISO_IDLE) {
            spin_unlock_irqrestore(&sdev->priv_lock, flags);
            printk("w2");
            msleep(10);
            spin_lock_irqsave(&sdev->priv_lock, flags);
            //This is a very corn case where all requests completed, but state still stay in ISO_STARTED.
            if (req_empty(pei) && !atomic_read(&pei->urb_submitted)) {
                struct urb *urb, *u;
                //free the extra urbs
                list_for_each_entry_safe(urb, u, &pei->urb_completed, urb_list) {
                    list_del_init(&urb->urb_list);
                    stub_free_urb(urb);
                }
                printk("ur(%d)\n", atomic_read(&pei->urb_submitted));

                pei->state = ISO_IDLE;
            }
        }
    }

    spin_unlock_irqrestore(&sdev->priv_lock, flags);


}

#undef req_empty

static int tweak_set_interface_cmd_iso_out
    (struct urb *urb, 
    __u16 alternate, 
    __u16 interface)
{
    struct stub_priv *priv = (struct stub_priv *) urb->context;
    struct stub_device *sdev = priv->sdev;
    struct usb_interface *iface;
    struct usb_host_interface *alt;
    struct usb_endpoint_descriptor *ep;
    int i;
    unsigned long flags;

    if (urb->dev->state == USB_STATE_SUSPENDED)
        return -EHOSTUNREACH;

    iface = usb_ifnum_to_if(urb->dev, interface);
    if (!iface) {
        return -EINVAL;
    }

    alt = usb_altnum_to_altsetting(iface, alternate);
    if (!alt) {
        return -EINVAL;
    }

    /* Handle the stop case. */
    if (!alternate) {
        for (i = 0; i < 16; i++) {
            if (sdev->ep_out[i].intf == interface) {
                printk("stop iso out ep[%d] intf(%d)\n", i, interface);
                tweak_stop_iso_out(sdev, &sdev->ep_out[i]);
            }
        }
    }

    /* handle the start case. */
    for (i = 0; i < alt->desc.bNumEndpoints; i++) {
        ep = &(alt->endpoint[i].desc);

        //printk("bmAttributes=0x%02x, bEndpointAddress=0x%02x, alt=%d\n", ep->bmAttributes, ep->bEndpointAddress, alternate);
        if (((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC)
            && ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT)) {

            do {
                struct priv_ep_info *pei;
                
                pei = &sdev->ep_out[ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK];
                spin_lock_irqsave(&sdev->priv_lock, flags);
                pei->intf = interface;
                pei->state = ISO_IDLE;
                printk("start iso out ep[%d] intf(%d)\n", 
                        ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK, interface);
                spin_unlock_irqrestore(&sdev->priv_lock, flags);
            } while (0);

            printk("I");
        }

    }

    return 0;
}



static int tweak_set_interface_cmd_iso_in
    (struct urb *urb, 
    __u16 alternate, 
    __u16 interface)
{
    struct stub_priv *priv = (struct stub_priv *) urb->context;
    struct stub_device *sdev = priv->sdev;
    struct usb_interface *iface;
    struct usb_host_interface *alt;
    struct usb_endpoint_descriptor *ep;
    int i;
    unsigned long flags;

    if (urb->dev->state == USB_STATE_SUSPENDED)
        return -EHOSTUNREACH;

    iface = usb_ifnum_to_if(urb->dev, interface);
    if (!iface) {
        return -EINVAL;
    }

    alt = usb_altnum_to_altsetting(iface, alternate);
    if (!alt) {
        return -EINVAL;
    }

    /* Handle the stop case. */
    if (!alternate) {
        for (i = 0; i < 16; i++) {
            if (sdev->ep_in[i].intf == interface) {
                printk("stop iso in ep[%d] intf(%d)\n", i, interface);
                tweak_stop_iso_in(sdev, &sdev->ep_in[i]);
            }
        }
    }

    /* handle the start case. */
    for (i = 0; i < alt->desc.bNumEndpoints; i++) {
        ep = &(alt->endpoint[i].desc);

        //printk("bmAttributes=0x%02x, bEndpointAddress=0x%02x, alt=%d\n", ep->bmAttributes, ep->bEndpointAddress, alternate);
        if (((ep->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_ISOC)
            && ((ep->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)) {

            do {
                struct priv_ep_info *pei;
                
                pei = &sdev->ep_in[ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK];
                spin_lock_irqsave(&sdev->priv_lock, flags);
                pei->intf = interface;
                pei->state = ISO_IDLE;
                printk("start iso in ep[%d] intf(%d)\n", 
                        ep->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK, interface);
                spin_unlock_irqrestore(&sdev->priv_lock, flags);
            } while (0);

            printk("i");
        }

    }

    return 0;
}


static int tweak_set_interface_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	__u16 alternate;
	__u16 interface;
	int ret;

	req = (struct usb_ctrlrequest *) urb->setup_packet;
	alternate = le16_to_cpu(req->wValue);
	interface = le16_to_cpu(req->wIndex);

	udbg("set_interface: inf %u alt %u\n", interface, alternate);

	ret = tweak_set_interface_cmd_iso_out(urb, alternate, interface);
	if (ret < 0)
	    uerr("tweak iso out failed!\n");

	ret = tweak_set_interface_cmd_iso_in(urb, alternate, interface);
	if (ret < 0)
	    uerr("tweak iso in failed!\n");

	ret = usb_set_interface(urb->dev, interface, alternate);
	if (ret < 0)
		uinfo("set_interface error: inf %u alt %u, %d\n",
				interface, alternate, ret);
	else
		uinfo("set_interface done: inf %u alt %u\n", interface, alternate);


	return ret;
}

static int tweak_set_configuration_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	__u16 config;


	udbg("set_configuration is not fully supported yet\n");

	req = (struct usb_ctrlrequest *) urb->setup_packet;
	config = le16_to_cpu(req->wValue);

	/*
	 * I have never seen a multi-config device. Very rare.
	 * For most devices, this will be called to choose a default
	 * configuration only once in an initialization phase.
	 *
	 * set_configuration may change a device configuration and its device
	 * drivers will be unbound and assigned for a new device configuration.
	 * This means this usbip driver will be also unbound when called, then
	 * eventually reassigned to the device as far as driver matching
	 * condition is kept.
	 *
	 * Unfortunatelly, an existing usbip connection will be dropped
	 * due to this driver unbinding. So, skip here.
	 * A user may need to set a special configuration value before
	 * exporting the device.
	 */
	uinfo("set_configuration (%d) to %s\n", config, urb->dev->dev.bus_id);
	uinfo("but, skip!\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
	return 0;
	//return usb_driver_set_configuration(urb->dev, config);
#else
	//uinfo("no support of set_config in 2.6.18\n");
	return 0;
#endif
}

static int tweak_reset_device_cmd(struct urb *urb)
{
	struct usb_ctrlrequest *req;
	__u16 value;
	__u16 index;
	int ret;

	req = (struct usb_ctrlrequest *) urb->setup_packet;
	value = le16_to_cpu(req->wValue);
	index = le16_to_cpu(req->wIndex);

	udbg("reset_device (port %d) to %s\n", index, urb->dev->dev.bus_id);

	/* all interfaces should be owned by usbip driver, so just reset it. */
	ret = usb_lock_device_for_reset(urb->dev, NULL);
	if (ret < 0) {
		uerr("lock for reset\n");
		return ret;
	}

	/* try to reset the device */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
	ret = usb_reset_composite_device(urb->dev, NULL);
#else
	ret = usb_reset_device(urb->dev);
#endif
	if (ret < 0)
		uerr("device reset\n");

	usb_unlock_device(urb->dev);

	return ret;
}


#if 0
/*
 * clear_halt, set_interface, and set_configuration require special tricks.
 */
static void tweak_special_requests(struct urb *urb)
{
	if (!urb || !urb->setup_packet)
		return;

	if (usb_pipetype(urb->pipe) != PIPE_CONTROL)
		return;

	if (is_clear_halt_cmd(urb))
		/* tweak clear_halt */
		 tweak_clear_halt_cmd(urb);

	else if (is_set_interface_cmd(urb))
		/* tweak set_interface */
		tweak_set_interface_cmd(urb);

	else if (is_set_configuration_cmd(urb))
		/* tweak set_configuration */
		tweak_set_configuration_cmd(urb);

	else if (is_reset_device_cmd(urb))
		tweak_reset_device_cmd(urb);
	else
		dbg_stub_rx("no need to tweak\n");
}


#else

void complete_tweak_urb(struct urb *urb, int status)
{
	struct stub_priv *priv = (struct stub_priv *) urb->context;
	struct stub_device *sdev = priv->sdev;
	unsigned long flags;

	urb->status = status;
	dbg_stub_rx("complete tweak urb! status %d\n", status);


	switch (urb->status) {
		case 0:
			/* OK */
			break;
		case -ENOENT:
			uinfo("stopped by a call of usb_kill_urb()"
					"because of cleaning up a virtual connection\n");
			return;
		case -ECONNRESET:
			uinfo("unlinked by a call of usb_unlink_urb()\n");
			break;
		case -EPIPE:
			uinfo("endpoint %d is stalled\n", usb_pipeendpoint(urb->pipe));
			break;
		case -ESHUTDOWN:
			uinfo("device removed?\n");
			break;
		default:
			uinfo("urb completion with non-zero status %d\n", urb->status);
	}


	/* link a urb to the queue of tx. */
	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_move_tail(&priv->list, &sdev->priv_tx);

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	/* wake up tx_thread */
	wake_up(&sdev->tx_waitq);
}


/*
 * clear_halt, set_interface, and set_configuration require special tricks.
 * return:
 * 0: urb should be submited to bus driver.
 * 1: urb should NOT be submitted to bus driver.
 */
static int tweak_special_requests(struct urb *urb)
{
	int status;

	if (!urb || !urb->setup_packet)
		return 0;

	if (usb_pipetype(urb->pipe) != PIPE_CONTROL)
		return 0;

	if (is_clear_halt_cmd(urb)) {
		/*
		** I found a WD external HD which can't accept "clear halt" twice.
		** So, we can't submit this urb twice. Return 1.
		*/
		/* tweak clear_halt */
		status = tweak_clear_halt_cmd(urb);
		complete_tweak_urb(urb, status);
		return 1;
	}
	else if (is_set_interface_cmd(urb)) {
		/* tweak set_interface */
		status = tweak_set_interface_cmd(urb);
		complete_tweak_urb(urb, status);
		return 1;
	}
	else if (is_set_configuration_cmd(urb)) {
		/* tweak set_configuration */
		status = tweak_set_configuration_cmd(urb);
		/*
		** Bruce140529. Nexus 7 (Nvidia Tegra SoC) Android 4.4.2 MTP/PTP fix.
		** Some Android MTP or PTP device will disconnect itself from USB host
		** when USB host set configure twice.
		** Linux USB host driver will do set configure in tweak_reset_device_cmd(),
		** So, we should totally ignore set config here to avoid set config twice.
		*/
		complete_tweak_urb(urb, status);
		return 1;
	}
	else if (is_reset_device_cmd(urb)) {
		if (ast_scu.astparam.usb_quirk & USBIP_QUIRK_NO_RESET_TWICE) {
			udbg("skip reset\n");
			status = 0;
		} else {
			status = tweak_reset_device_cmd(urb);
		}
		complete_tweak_urb(urb, status);
		return 1;
	}
	else
		dbg_stub_rx("no need to tweak\n");

	return 0;
}
#endif
/*
 * stub_recv_unlink() unlinks the URB by a call to usb_unlink_urb().
 * By unlinking the urb asynchronously, stub_rx can continuously
 * process coming urbs.  Even if the urb is unlinked, its completion
 * handler will be called and stub_tx will send a return pdu.
 *
 * See also comments about unlinking strategy in vhci_hcd.c.
 */
static int stub_recv_cmd_unlink(struct stub_device *sdev, struct usbip_header *pdu)
{
	struct list_head *listhead = &sdev->priv_init;
	struct list_head *ptr;
	unsigned long flags;

	struct stub_priv *priv;


	spin_lock_irqsave(&sdev->priv_lock, flags);

	for (ptr = listhead->next; ptr != listhead; ptr = ptr->next) {
		priv = list_entry(ptr, struct stub_priv, list);
		if (priv->seqnum == pdu->u.cmd_unlink.seqnum) {
			int ret;

			uinfo("unlink urb %p\n", priv->urb);

			/*
			 * This matched urb is not completed yet (i.e., be in
			 * flight in usb hcd hardware/driver). Now we are
			 * cancelling it. The unlinking flag means that we are
			 * now not going to return the normal result pdu of a
			 * submission request, but going to return a result pdu
			 * of the unlink request.
			 */
			priv->unlinking = 1;

			/*
			 * In the case that unlinking flag is on, prev->seqnum
			 * is changed from the seqnum of the cancelling urb to
			 * the seqnum of the unlink request. This will be used
			 * to make the result pdu of the unlink request.
			 */
			priv->seqnum = pdu->base.seqnum;

			spin_unlock_irqrestore(&sdev->priv_lock, flags);

			/*
			 * usb_unlink_urb() is now out of spinlocking to avoid
			 * spinlock recursion since stub_complete() is
			 * sometimes called in this context but not in the
			 * interrupt context.  If stub_complete() is executed
			 * before we call usb_unlink_urb(), usb_unlink_urb()
			 * will return an error value. In this case, stub_tx
			 * will return the result pdu of this unlink request
			 * though submission is completed and actual unlinking
			 * is not executed. OK?
			 */
			/* In the above case, urb->status is not -ECONNRESET,
			 * so a driver in a client host will know the failure
			 * of the unlink request ?
			 */
			msleep(1); //Bruce130627. Dirty patch to avoid race condition. (urb, priv freed). Works!!
			ret = usb_unlink_urb(priv->urb);
			if (ret != -EINPROGRESS)
				uerr("faild to unlink a urb %p, ret %d\n", priv->urb, ret);

			return 0;
		}
	}

	dbg_stub_rx("seqnum %d is not pending\n", pdu->u.cmd_unlink.seqnum);

	/*
	 * The urb of the unlink target is not found in priv_init queue. It was
	 * already completed and its results is/was going to be sent by a
	 * CMD_RET pdu. In this case, usb_unlink_urb() is not needed. We only
	 * return the completeness of this unlink request to vhci_hcd.
	 */
	stub_enqueue_ret_unlink(sdev, pdu->base.seqnum, 0);

	spin_unlock_irqrestore(&sdev->priv_lock, flags);


	return 0;
}

static int valid_request(struct stub_device *sdev, struct usbip_header *pdu)
{
	struct usbip_device *ud = &sdev->ud;

	if (pdu->base.devid == sdev->devid) {
		spin_lock(&ud->lock);
		if (ud->status == SDEV_ST_USED) {
			/* A request is valid. */
			spin_unlock(&ud->lock);
			return 1;
		}
		spin_unlock(&ud->lock);
	}

	return 0;
}

static struct stub_priv *stub_priv_alloc(struct stub_device *sdev,
		struct usbip_header *pdu)
{
	struct stub_priv *priv;
	struct usbip_device *ud = &sdev->ud;
	unsigned long flags;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	priv = kmem_cache_alloc(stub_priv_cache, GFP_ATOMIC);
	if (!priv) {
		uerr("alloc stub_priv\n");
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
		return NULL;
	}

	memset(priv, 0, sizeof(struct stub_priv));

	priv->seqnum = pdu->base.seqnum;
	priv->sdev = sdev;

	/*
	 * After a stub_priv is linked to a list_head,
	 * our error handler can free allocated data.
	 */
	list_add_tail(&priv->list, &sdev->priv_init);

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return priv;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
static inline int usb_endpoint_xfer_bulk(
		const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
			USB_ENDPOINT_XFER_BULK);
}
static inline int usb_endpoint_xfer_control(
		const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
			USB_ENDPOINT_XFER_CONTROL);
}
static inline int usb_endpoint_xfer_int(
		const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
			USB_ENDPOINT_XFER_INT);
}
static inline int usb_endpoint_xfer_isoc(
		const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
			USB_ENDPOINT_XFER_ISOC);
}
#endif


static struct usb_host_endpoint *get_ep_from_epnum(struct usb_device *udev,
		int epnum0, int dir0)
{
	struct usb_host_config *config;
	int i = 0, j = 0;
	struct usb_host_endpoint *ep = NULL;
	u8 bEndpointAddress;

	if (epnum0 == 0)
		return &udev->ep0;

	config = udev->actconfig;
	if (!config)
		return NULL;

	dir0 = ((dir0 == USBIP_DIR_IN) ? (USB_DIR_IN) : (USB_DIR_OUT));
	bEndpointAddress = (epnum0 | dir0);

	for (i = 0; i < config->desc.bNumInterfaces; i++) {
		struct usb_host_interface *setting;

		setting = config->interface[i]->cur_altsetting;

		for (j = 0; j < setting->desc.bNumEndpoints; j++) {
			ep = &setting->endpoint[j];

			if (ep->desc.bEndpointAddress == bEndpointAddress)
				return ep;
            
		}
	}

	return NULL;
}


static int get_pipe(struct stub_device *sdev, int epnum, int dir)
{
	struct usb_device *udev = interface_to_usbdev(sdev->interface);
	struct usb_host_endpoint *ep;
	struct usb_endpoint_descriptor *epd = NULL;

	ep = get_ep_from_epnum(udev, epnum, dir);
	if (!ep) {
		uerr("no such endpoint?, %d", epnum);
		BUG();
	}

	epd = &ep->desc;


#if 0
	/* epnum 0 is always control */
	if (epnum == 0) {
		if (dir == USBIP_DIR_OUT)
			return usb_sndctrlpipe(udev, 0);
		else
			return usb_rcvctrlpipe(udev, 0);
	}
#endif

	if (usb_endpoint_xfer_control(epd)) {
		if (dir == USBIP_DIR_OUT)
			return usb_sndctrlpipe(udev, epnum);
		else
			return usb_rcvctrlpipe(udev, epnum);
	}

	if (usb_endpoint_xfer_bulk(epd)) {
		if (dir == USBIP_DIR_OUT)
			return usb_sndbulkpipe(udev, epnum);
		else
			return usb_rcvbulkpipe(udev, epnum);
	}

	if (usb_endpoint_xfer_int(epd)) {
		if (dir == USBIP_DIR_OUT)
			return usb_sndintpipe(udev, epnum);
		else
			return usb_rcvintpipe(udev, epnum);
	}

	if (usb_endpoint_xfer_isoc(epd)) {
		if (dir == USBIP_DIR_OUT)
			return usb_sndisocpipe(udev, epnum);
		else
			return usb_rcvisocpipe(udev, epnum);
	}

	/* NOT REACHED */
	uerr("get pipe, epnum %d\n", epnum);
	return 0;
}


/* Here is note from ehci-sched.c: iso_stream_schedule()
 * This scheduler plans almost as far into the future as it has actual
 * periodic schedule slots.  (Affected by TUNE_FLS, which defaults to
 * "as small as possible" to be cache-friendlier.)  That limits the size
 * transfers you can stream reliably; avoid more than 64 msec per urb.
 * Also avoid queue depths of less than ehci's worst irq latency (affected
 * by the per-urb URB_NO_INTERRUPT hint, the log2_irq_thresh module parameter,
 * and other factors); or more than about 230 msec total (for portability,
 * given EHCI_TUNE_FLS and the slop).  Or, write a smarter scheduler!
 */

void start_iso_in(struct priv_ep_info *pei)
{
    struct stub_device *sdev= pei->sdev;
    unsigned long flags;
    int ret = 0, n = 0;
    int q_len;
    struct stub_priv *priv;
    struct stub_priv *t;

    spin_lock_irqsave(&sdev->priv_lock, flags);

    if (pei->state == ISO_STARTING) {

        q_len = atomic_read(&pei->waiting);
        printk("s(%d)", q_len);
        //BUG_ON(list_empty(&pei->priv_wait));

        list_for_each_entry_safe(priv, t, &pei->priv_wait, list) {
            struct urb *urb = priv->urb;
            int nop = urb->number_of_packets;
            
            atomic_sub_return(nop, &pei->waiting);
            //save the pei into context, instead of priv.
            urb->context = (void*)pei;
            //say goodbye to the urb.
            priv->urb = NULL;
            list_move_tail(&priv->list, &pei->priv_submitted);
            atomic_inc(&pei->submitted);
            atomic_inc(&pei->urb_submitted);
            n += nop;

            //printk("iso IN pipe(0x%08x) interval(%d)\n", urb->pipe, urb->interval);
            //spin_unlock_irqrestore(&sdev->priv_lock, flags);
            ret = usb_submit_urb(urb, GFP_ATOMIC);
            //spin_lock_irqsave(&sdev->priv_lock, flags);
            if (ret) {
                int r;
                
                n -= nop;
                r = atomic_dec_return(&pei->urb_submitted);
                atomic_dec(&pei->submitted);
                priv->urb = urb;
                urb->context = (void*)priv;
                atomic_add_return(nop, &pei->waiting);
                list_move(&priv->list, &pei->priv_wait);
                uerr("submit iso in urb (%p) failed (%d). (%d) submitted\n", priv, ret, r);
                if (r)
                    pei->state = ISO_ERR;
                else {
                    //This could be a fautal error and we are failed to start the iso stream flow.
                    //We MUST release all requests here or we won't be able to stop this interface.
                    BUG_ON(!list_empty(&pei->priv_submitted));
                    BUG_ON(!list_empty(&pei->urb_completed));
                    //Complete all the requests in "wait" list.
                    list_for_each_entry_safe(priv, t, &pei->priv_wait, list) {
                        q_len = atomic_sub_return(priv->urb->number_of_packets, &pei->waiting);
                        priv->urb->status = -EXDEV;
                        priv->urb->actual_length = 0;
                        list_move_tail(&priv->list, &sdev->priv_tx);
                        printk("fatal(%d)", q_len);
                    }
                    wake_up(&sdev->tx_waitq);
                    pei->state = ISO_IDLE;
                }
                break;
            }
            //BruceToDo. Don't over 230 ms. Here is a dirty implement base on full spd device.
            if (n >= (nop * 2))
                break;
        }

    }
    else {
        uerr("unexpected state?!\n");
        BUG();
    }
    //everything goes well.
    if (pei->state == ISO_STARTING)
        pei->state = ISO_STARTED;

    spin_unlock_irqrestore(&sdev->priv_lock, flags);

}


static int stub_submit_priv_iso_in(struct stub_device *sdev, struct stub_priv *priv)
{
    unsigned long flags;
    int q_len;
    struct priv_ep_info *pei;

    pei = &sdev->ep_in[usb_pipeendpoint(priv->urb->pipe)];
    
    spin_lock_irqsave(&sdev->priv_lock, flags);

    /* queue priv into "wait" tail */
    list_move_tail(&priv->list, &pei->priv_wait);
    q_len = atomic_add_return(priv->urb->number_of_packets, &pei->waiting);
    //printk("a(%d)", q_len);

    /* If the state is IDLE, change state to STARTING,
    ** submit an urb and move to "submitted" tail. */
    if (pei->state == ISO_IDLE 
         && (q_len > (priv->urb->number_of_packets * ISO_URB_SUBMIT_LOW_BOUND))) {
        pei->state = ISO_STARTING;
        spin_unlock_irqrestore(&sdev->priv_lock, flags);
        
        PREPARE_WORK(&pei->work, (void (*)(void *))(start_iso_in), (void *)pei);
        queue_work(sdev->wq, &pei->work);
#if 0
        queue_delayed_work(sdev->wq, 
                &pei->work, 
                msecs_to_jiffies(ISO_SUBMIT_LOW_BOUND));
#endif
        return 0;
    }
    else if (pei->state == ISO_ERR && list_empty(&pei->priv_submitted)) {
        //This is for the case where the duplicated urb request completed but the "wait" list is still empty.
        pei->state = ISO_IDLE;
    }
    spin_unlock_irqrestore(&sdev->priv_lock, flags);
    return 0;
}


/**
 * list_for_each_entry_safe_reverse
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))



void start_iso_out(struct priv_ep_info *priv_ep_info)
{
    struct stub_device *sdev= priv_ep_info->sdev;
    unsigned long flags;
    struct list_head list, list1;
    int ret = 0, n = 0;
    int q_len;
    struct stub_priv *priv;
    struct stub_priv *t;

    spin_lock_irqsave(&sdev->priv_lock, flags);

    if (priv_ep_info->state == ISO_STARTING) {

        q_len = atomic_read(&priv_ep_info->waiting);
        printk("S(%d)", q_len);
        //BUG_ON(list_empty(&priv_ep_info->priv_wait));

        INIT_LIST_HEAD(&list);
        INIT_LIST_HEAD(&list1);
        list_for_each_entry_safe(priv, t, &priv_ep_info->priv_wait, list) {
            list_move_tail(&priv->list, &list);
            atomic_sub_return(priv->urb->number_of_packets, &priv_ep_info->waiting);
            n += priv->urb->number_of_packets;
            //BruceToDo. Need to caculate how many urbs to submit at once.
            if (n >= (priv->urb->number_of_packets) * 2)
                break;
        }
        //BruceTestOnly. I often get urb out of sequence when I unlock spin_lock here.
        //spin_unlock_irqrestore(&sdev->priv_lock, flags);

        list_for_each_entry_safe(priv, t, &list, list) {
            //printk("iso OUT pipe(0x%08x) interval(%d)\n", priv->urb->pipe, priv->urb->interval);
            ret = usb_submit_urb(priv->urb, GFP_ATOMIC);
            if (ret) {
                //BUG_ON(ret);
                break;
            }
            list_move_tail(&priv->list, &list1);            
            //printk("t");
        }
        //spin_lock_irqsave(&sdev->priv_lock, flags);
        //BUG_ON(!list_empty(&list));
        if (!list_empty(&list)) {
            //There is a race condition where urb completed before following code being excuted.
            if (!list_empty(&list1)) {
                //we can just change the state and complete routine will handle it.
                //list_splice(&list1, priv_ep_info->priv_submitted->prev);
                list_for_each_entry_safe(priv, t, &list1, list) {
                    atomic_inc(&priv_ep_info->submitted);
                    list_move_tail(&priv->list, &priv_ep_info->priv_submitted);
                }
                list_for_each_entry_safe_reverse(priv, t, &list, list) {
                    atomic_add_return(priv->urb->number_of_packets, &priv_ep_info->waiting);
                    list_move(&priv->list, &priv_ep_info->priv_wait);
                }

                priv_ep_info->state = ISO_ERR;
                printk("E1\n");
            } else {
                //we failed to submit any request?!Complete all request in sequence.
                //BruceToDo. update the urb status?!
                list_for_each_entry_safe(priv, t, &list, list) {
                    list_move_tail(&priv->list, &sdev->priv_tx);
                }
                list_for_each_entry_safe(priv, t, &priv_ep_info->priv_wait, list) {
                    list_move_tail(&priv->list, &sdev->priv_tx);
                    atomic_sub_return(priv->urb->number_of_packets, &priv_ep_info->waiting);
                }
                priv_ep_info->state = ISO_IDLE;
                printk("E2\n");
                /* wake up tx_thread */
                wake_up(&sdev->tx_waitq);
            }
        }
        else {
            //everything goes well.
            list_for_each_entry_safe(priv, t, &list1, list) {
                atomic_inc(&priv_ep_info->submitted);
                list_move_tail(&priv->list, &priv_ep_info->priv_submitted);
            }

            priv_ep_info->state = ISO_STARTED;
        }
    }
    else {
        uerr("unexpected state?!\n");
        BUG();
    }

    spin_unlock_irqrestore(&sdev->priv_lock, flags);
}

static int stub_submit_priv_iso_out(struct stub_device *sdev, struct stub_priv *priv)
{
    unsigned long flags;
    int q_len;
    struct priv_ep_info *priv_ep_info;

    priv_ep_info = &sdev->ep_out[usb_pipeendpoint(priv->urb->pipe)];
    
    spin_lock_irqsave(&sdev->priv_lock, flags);

    /* queue priv into "wait" tail */
    list_move_tail(&priv->list, &priv_ep_info->priv_wait);
    q_len = atomic_add_return(priv->urb->number_of_packets, &priv_ep_info->waiting);
    //printk("A(%lu)", priv->seqnum);
    //printk("A");

    /* If the state is IDLE, change state to STARTING,
    ** submit an urb and move to "submitted" tail. */
    if (priv_ep_info->state == ISO_IDLE 
         && (q_len > (priv->urb->number_of_packets * ISO_URB_SUBMIT_LOW_BOUND))) {
        priv_ep_info->state = ISO_STARTING;
        spin_unlock_irqrestore(&sdev->priv_lock, flags);
        
        PREPARE_WORK(&priv_ep_info->work, (void (*)(void *))(start_iso_out), (void *)priv_ep_info);
        queue_work(sdev->wq, &priv_ep_info->work);
#if 0
        queue_delayed_work(sdev->wq, 
                &priv_ep_info->work, 
                msecs_to_jiffies(ISO_SUBMIT_LOW_BOUND));
#endif
        return 0;
    }
    else if (priv_ep_info->state == ISO_ERR && list_empty(&priv_ep_info->priv_submitted)) {
        //This is for the case where the duplicated urb request completed but the "wait" list is still empty.
        priv_ep_info->state = ISO_IDLE;
    }
    spin_unlock_irqrestore(&sdev->priv_lock, flags);
    return 0;

}

static inline int stub_submit_priv(struct stub_device *sdev, struct stub_priv *priv)
{
    if (usb_pipeisoc(priv->urb->pipe)) {
        if (usb_pipeout(priv->urb->pipe))
            return stub_submit_priv_iso_out(sdev, priv);
        else
            return stub_submit_priv_iso_in(sdev, priv);

    } else { //requests excepts for iso
        /* urb is now ready to submit */
        return usb_submit_urb(priv->urb, GFP_KERNEL);
    }

    return -ENODEV;
}


static void stub_recv_cmd_submit(struct stub_device *sdev, struct usbip_header *pdu)
{
	int ret;
	struct stub_priv *priv;
	struct usbip_device *ud = &sdev->ud;
	struct usb_device *udev = interface_to_usbdev(sdev->interface);
	int pipe = get_pipe(sdev, pdu->base.ep, pdu->base.direction);

	priv = stub_priv_alloc(sdev, pdu);
	if (!priv)
		return;

	/* setup a urb */
	if (usb_pipeisoc(pipe))
		priv->urb = usb_alloc_urb(pdu->u.cmd_submit.number_of_packets, GFP_KERNEL);
	else
		priv->urb = usb_alloc_urb(0, GFP_KERNEL);

	if (!priv->urb) {
		uerr("malloc urb\n");
		usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
		return;
	}

	/* set priv->urb->transfer_buffer */
	if (pdu->u.cmd_submit.transfer_buffer_length > 0) {
		priv->urb->transfer_buffer =
			kzalloc(pdu->u.cmd_submit.transfer_buffer_length, GFP_KERNEL);
		if (!priv->urb->transfer_buffer) {
			uerr("malloc x_buff\n");
			usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
			return;
		}
	}

	/* set priv->urb->setup_packet */
	priv->urb->setup_packet = kzalloc(8, GFP_KERNEL);
	if (!priv->urb->setup_packet) {
		uerr("allocate setup_packet\n");
		usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
		return;
	}
	memcpy(priv->urb->setup_packet, &pdu->u.cmd_submit.setup, 8);

	/* set other members from the base header of pdu */
	priv->urb->context                = (void *) priv;
	priv->urb->dev                    = udev;
	priv->urb->pipe                   = pipe;
	priv->urb->complete               = (usb_complete_t)stub_complete;

	usbip_pack_pdu(pdu, priv->urb, USBIP_CMD_SUBMIT, 0);


	if (usbip_recv_xbuff(ud, priv->urb) < 0)
		return;

	if (usbip_recv_iso(ud, priv->urb) < 0)
		return;

#if 0
#define udump(buf, size) \
    do { \
        int _i; \
        printk(KERN_INFO "dump 0x%08x:\n", (u32)(buf)); \
        for (_i = 0; _i < (size); _i++) { \
            printk("%02x ", *(((unsigned char*)(buf)) + _i)); \
            if (_i == 15) printk("\n"); \
        } \
        printk("\n"); \
    } while (0)


        do {//BruceTestOnly
            if (!(!usb_pipein(priv->urb->pipe) && priv->urb->transfer_buffer_length > 0))
                break;
            uinfo("ep submit:\n");
            udump(priv->urb->transfer_buffer, priv->urb->transfer_buffer_length);
        } while (0);
#endif



	/* 
	** This assumption is wrong: "no need to submit an intercepted request, but harmless?"
	** I found a WD external HD which can't accept "clear halt" twice.
	*/
	if (tweak_special_requests(priv->urb)) {
		//means don't submit the urb. just return;
		goto out;
	}

	
	/* urb is now ready to submit */
	ret = stub_submit_priv(sdev, priv);

	if (ret == 0)
		dbg_stub_rx("submit urb ok, seqnum %u\n", pdu->base.seqnum);
	else {
		uerr("submit_urb error, %d\n", ret);
		usbip_dump_header(pdu);
		usbip_dump_urb(priv->urb);

		/*
		 * Pessimistic.
		 * This connection will be discarded.
		 */
		usbip_event_add(ud, SDEV_EVENT_ERROR_SUBMIT);
	}

out:
	dbg_stub_rx("Leave\n");
	return;
}

/* recv a pdu */
static void stub_rx_pdu(struct usbip_device *ud)
{
	int ret;
	struct usbip_header pdu;
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);


	dbg_stub_rx("Enter\n");

	memset(&pdu, 0, sizeof(pdu));


	/* 1. receive a pdu header */
again:
	ret = usbip_xmit(0, ud->tcp_socket, (char *) &pdu, sizeof(pdu),0);
	if (ret != sizeof(pdu)) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv a header, %d\n", ret);
		usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
		return;
	}

	usbip_header_correct_endian(&pdu, 0);

	if (dbg_flag_stub_rx)
		usbip_dump_header(&pdu);

	if (!valid_request(sdev, &pdu)) {
		uerr("recv invalid request\n");
		usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
		return;
	}

	switch (pdu.base.command) {
		case USBIP_CMD_UNLINK:
			stub_recv_cmd_unlink(sdev, &pdu);
			break;

		case USBIP_CMD_SUBMIT:
			stub_recv_cmd_submit(sdev, &pdu);
			break;

		default:
			/* NOTREACHED */
			uerr("unknown pdu\n");
			usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
			return;
	}

}

void stub_rx_loop(struct usbip_task *ut)
{
	struct usbip_device *ud = container_of(ut, struct usbip_device, tcp_rx);

	while (1) {
		if (signal_pending(current)) {
			dbg_stub_rx("signal caught!\n");
			break;
		}

		if (usbip_event_happend(ud))
			break;

		stub_rx_pdu(ud);
	}
}

