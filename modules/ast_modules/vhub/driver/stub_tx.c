/*
 * $Id: stub_tx.c 76 2008-04-29 04:36:43Z hirofuchi $
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

#include "usbip_common.h"
#include "stub.h"


static void stub_free_priv_and_urb(struct stub_priv *priv)
{
	struct urb *urb = priv->urb;

	if (urb->setup_packet)
		kfree(urb->setup_packet);

	if (urb->transfer_buffer)
		kfree(urb->transfer_buffer);

	list_del(&priv->list);
	kmem_cache_free(stub_priv_cache, priv);

	usb_free_urb(urb);
}

/* be in spin_lock_irqsave(&sdev->priv_lock, flags) */
void stub_enqueue_ret_unlink(struct stub_device *sdev, __u32 seqnum, __u32 status)
{
	struct stub_unlink *unlink;

	unlink = kzalloc(sizeof(struct stub_unlink), GFP_ATOMIC);
	if (!unlink) {
		uerr("alloc stub_unlink\n");
		usbip_event_add(&sdev->ud, VDEV_EVENT_ERROR_MALLOC);
		return;
	}

	unlink->seqnum = seqnum;
	unlink->status = status;

	list_add_tail(&unlink->list, &sdev->unlink_tx);
}


static int dup_urb_iso(struct urb *urb, struct urb *urb_src)
{
    int np = urb_src->number_of_packets;
    int i;
    int ret = 0;

    if (!usb_pipeisoc(urb_src->pipe))
        return 0;

    for (i = 0; i < np; i++) {
        urb->iso_frame_desc[i].offset		= urb_src->iso_frame_desc[i].offset;
        urb->iso_frame_desc[i].length		= urb_src->iso_frame_desc[i].length;
        urb->iso_frame_desc[i].status		= 0;
        urb->iso_frame_desc[i].actual_length	= 0;
    }

    return ret;
}

void stub_dup_urb_complete(struct urb *urb)
{
    struct stub_device *sdev = (struct stub_device *) urb->context;

    dbg_stub_tx("dup urb complete! status %d\n", urb->status);

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

    do {
        struct priv_ep_info *priv_ep_info;
        unsigned long flags;
        
        priv_ep_info = &sdev->ep_out[usb_pipeendpoint(urb->pipe)];
        spin_lock_irqsave(&sdev->priv_lock, flags);
        if (priv_ep_info->state == ISO_STOPPING) {
            if (atomic_read(&priv_ep_info->submitted) > 0/*ISO_URB_SUBMIT_LOW_BOUND*/) {
                if (list_empty(&priv_ep_info->priv_wait)) {
                    //Not good, but I still can try to duplicate an urb request.
                    priv_ep_info->state = ISO_STARTED;
                    printk("Alive\n");
                } else {
                    //Great! do nothing!
                }
            } else {
                // Nothing left. Something wrong.
                priv_ep_info->state = ISO_ERR;
                printk("ISO_ERR\n");
            }
        }

        spin_unlock_irqrestore(&sdev->priv_lock, flags);
    } while (0);


    if (urb->setup_packet)
        kfree(urb->setup_packet);

    if (urb->transfer_buffer)
        kfree(urb->transfer_buffer);

    usb_free_urb(urb);
}

static struct urb * stub_clone_urb(struct urb *urb_src)
{
    struct urb *urb;

    /* setup a urb */
    if (usb_pipeisoc(urb_src->pipe))
        urb = usb_alloc_urb(urb_src->number_of_packets, GFP_ATOMIC);
    else
        urb = usb_alloc_urb(0, GFP_ATOMIC);

    if (!urb) {
        uerr("malloc urb\n");
        return NULL;
    }

    /* set priv->urb->transfer_buffer */
    if (urb_src->transfer_buffer_length > 0) {
        urb->transfer_buffer =
            kzalloc(urb_src->transfer_buffer_length, GFP_ATOMIC);
        if (!urb->transfer_buffer) {
            uerr("malloc x_buff\n");
            return NULL;
        }
    }
    
    /* set priv->urb->setup_packet */
    if (urb_src->setup_packet) {
        urb->setup_packet = kzalloc(8, GFP_ATOMIC);
        if (!urb->setup_packet) {
            uerr("allocate setup_packet\n");
            return NULL;
        }
        memcpy(urb->setup_packet, urb_src->setup_packet, 8);
    }

    return urb;
}

void stub_free_urb(struct urb *urb)
{
    if (urb->setup_packet)
        kfree(urb->setup_packet);

    if (urb->transfer_buffer)
        kfree(urb->transfer_buffer);

    usb_free_urb(urb);
}

static int stub_dup_in_urb_submit(struct stub_device *sdev, struct urb *urb_src)
{
    int ret = -ENOMEM;
    struct usbip_device *ud = &sdev->ud;
    struct urb *urb;
    struct priv_ep_info *pei;

    urb = stub_clone_urb(urb_src);
    if (!urb) {
        usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
        return ret;
    }

    pei = &sdev->ep_in[usb_pipeendpoint(urb_src->pipe)];
    
    /* set other members from the base header of pdu */
    urb->context                = (void *) pei;
    urb->dev                    = interface_to_usbdev(sdev->interface);
    urb->pipe                   = urb_src->pipe;
    urb->complete               = (usb_complete_t)stub_complete;

    urb->transfer_flags         = urb_src->transfer_flags;
    urb->transfer_buffer_length = urb_src->transfer_buffer_length;
    urb->start_frame            = urb_src->start_frame;
    urb->number_of_packets      = urb_src->number_of_packets;
    urb->interval               = urb_src->interval;

    urb->actual_length = 0;

    if (dup_urb_iso(urb, urb_src) < 0)
        return -EXDEV;

    /* urb is now ready to submit */
    atomic_inc(&pei->urb_submitted);
    ret = usb_submit_urb(urb, GFP_ATOMIC);
    printk("d");

    if (ret == 0)
        dbg_stub_rx("submit dup urb ok\n");
    else {
        uerr("submit_urb error, %d\n", ret);
        atomic_dec(&pei->urb_submitted);
        stub_free_urb(urb);
        /*
         * Pessimistic.
         * This connection will be discarded.
         */
        //usbip_event_add(ud, SDEV_EVENT_ERROR_SUBMIT);
    }

    return ret;
}

static int stub_dup_out_urb_submit(struct stub_device *sdev, struct urb *urb_src)
{
    int ret = -ENOMEM;
    struct usbip_device *ud = &sdev->ud;
    struct urb *urb;

    urb = stub_clone_urb(urb_src);
    if (!urb) {
        usbip_event_add(ud, SDEV_EVENT_ERROR_MALLOC);
        return ret;
    }
    
    /* set other members from the base header of pdu */
    urb->context                = (void *) sdev;
    urb->dev                    = interface_to_usbdev(sdev->interface);
    urb->pipe                   = urb_src->pipe;
    urb->complete               = (usb_complete_t)stub_dup_urb_complete;

    urb->transfer_flags         = urb_src->transfer_flags;
    urb->transfer_buffer_length = urb_src->transfer_buffer_length;
    urb->start_frame            = urb_src->start_frame;
    urb->number_of_packets      = urb_src->number_of_packets;
    urb->interval               = urb_src->interval;

    if (urb_src->transfer_buffer_length > 0) {
        memcpy(urb->transfer_buffer, urb_src->transfer_buffer, urb_src->transfer_buffer_length);
    }

    if (dup_urb_iso(urb, urb_src) < 0)
        return -EXDEV;

    /* urb is now ready to submit */
    ret = usb_submit_urb(urb, GFP_ATOMIC);

    if (ret == 0)
        dbg_stub_rx("submit dup urb ok\n");
    else {
        printk("D");
        //uerr("submit_urb error, %d\n", ret);

        /*
         * Pessimistic.
         * This connection will be discarded.
         */
        //usbip_event_add(ud, SDEV_EVENT_ERROR_SUBMIT);
    }

    return ret;
}

void stub_complete_priv_iso_in(struct stub_device *sdev, struct stub_priv *priv)
{
}

void stub_complete_priv_iso_out(struct stub_device *sdev, struct stub_priv *priv)
{
    unsigned long flags;
    struct priv_ep_info *pei;
    int q_len = 0, i, r;
    struct urb *urb = priv->urb;
    int ret = 0;

    pei = &sdev->ep_out[usb_pipeendpoint(urb->pipe)];
    spin_lock_irqsave(&sdev->priv_lock, flags);

    r = atomic_dec_return(&pei->submitted);
    //printk("C[%d]", r);
    for (i = 0; i < urb->number_of_packets; i++) {
        if (urb->iso_frame_desc[i].status == -EXDEV) {
            pei->state = ISO_ERR;
            break;
        }
    }

    if (pei->state == ISO_STOPPING && !list_empty(&pei->priv_wait)) {
        pei->state = ISO_STARTED;
    }
    if (pei->state == ISO_STARTED) {
        struct stub_priv *to_submit;
submit_urb:
        if (list_empty(&pei->priv_wait)) {
            //duplicate an urb request only when interface is still alive.
            if (pei->intf != -1 && (!list_empty(&pei->priv_submitted))) {
                //printk("E");
#if 1
                pei->state = ISO_STOPPING;
                
                //spin_unlock_irqrestore(&sdev->priv_lock, flags);
                //submit an dummy urb.
                ret = stub_dup_out_urb_submit(sdev, urb);
                //spin_lock_irqsave(&sdev->priv_lock, flags);
                if (ret)
                    pei->state = ISO_ERR;
#else
                //priv_ep_info->state = ISO_STOPPING;
#endif
            } else {
                //This is the last completed urb, and there is no more urb in "wait" list.
                //printk("P");
                pei->state = ISO_STOPPING;
            }
        } else {
            //submit the next urb;
            to_submit = list_entry(pei->priv_wait.next, struct stub_priv, list);
            r = atomic_inc_return(&pei->submitted);
            list_move_tail(&to_submit->list, &pei->priv_submitted);
            //printk("N");
            q_len = atomic_sub_return(to_submit->urb->number_of_packets, &pei->waiting);
            //printk("Q(%d)\n", q_len);
            //spin_unlock_irqrestore(&sdev->priv_lock, flags);
            ret = usb_submit_urb(to_submit->urb, GFP_ATOMIC);
            //spin_lock_irqsave(&sdev->priv_lock, flags);
            if (ret) {
                udbg("ret=%d(%d)\n", ret, -EXDEV);
                q_len = atomic_add_return(to_submit->urb->number_of_packets, &pei->waiting);
                list_move(&to_submit->list, &pei->priv_wait);
                r =  atomic_dec_return(&pei->submitted);
                pei->state = ISO_ERR;
            }
        }
        if ((r < ISO_URB_SUBMIT_LOW_BOUND) 
            && (pei->state == ISO_STARTED)
            && (pei->intf != -1)) {
            // if request number is too low, we should submit more then 1 request here.
            //printk("L");
            goto submit_urb;
        }

    }

    /* link a urb to the queue of tx. */
    if (priv->unlinking) {
        stub_enqueue_ret_unlink(sdev, priv->seqnum, urb->status);
        stub_free_priv_and_urb(priv);
    } else {
        list_move_tail(&priv->list, &sdev->priv_tx);
        //printk("R(%lu)", priv->seqnum);
    }

    if ((pei->state == ISO_ERR || pei->state == ISO_STOPPING)
        && list_empty(&pei->priv_submitted)) {
        struct stub_priv *t;
        //Complete all the requests in "wait" list if ISO_ERR
        list_for_each_entry_safe(priv, t, &pei->priv_wait, list) {
            list_move_tail(&priv->list, &sdev->priv_tx);
            q_len = atomic_sub_return(priv->urb->number_of_packets, &pei->waiting);
            //printk("R(%lu)", priv->seqnum);
        }
        printk("F(%u)\n", q_len);
        pei->state = ISO_IDLE;
    }

#if DOUBLE_CHK
    if (pei->state != ISO_IDLE) {
        if (atomic_read(&pei->submitted) == 0)
            uerr("Stop1 (%d)\n", pei->state);
        if (list_empty(&pei->priv_submitted))
            uerr("Stop1-1 (%d)\n", pei->state);
    }
#endif
    spin_unlock_irqrestore(&sdev->priv_lock, flags);

    /* wake up tx_thread */
    wake_up(&sdev->tx_waitq);

}


inline void stub_complete_priv(struct stub_device *sdev, struct stub_priv *priv)
{
    unsigned long flags;
    struct urb *urb = priv->urb;
    
    if (usb_pipeisoc(urb->pipe)) {
        if (usb_pipeout(urb->pipe)) { //iso out
            stub_complete_priv_iso_out(sdev, priv);
        } else { //iso in
            stub_complete_priv_iso_in(sdev, priv);
        }
    } else {
        /* link a urb to the queue of tx. */
        spin_lock_irqsave(&sdev->priv_lock, flags);

        if (priv->unlinking) {
            stub_enqueue_ret_unlink(sdev, priv->seqnum, urb->status);
            stub_free_priv_and_urb(priv);
        } else
            list_move_tail(&priv->list, &sdev->priv_tx);


        spin_unlock_irqrestore(&sdev->priv_lock, flags);

        /* wake up tx_thread */
        wake_up(&sdev->tx_waitq);
    }
}

void stub_complete_iso_in
    (struct stub_device *sdev, struct priv_ep_info *pei, struct urb *urb)
{
    unsigned long flags;
    int ret = 0, q_len = 0;
    struct stub_priv *priv;
    int r, i;

    //put the urb into "completed" list
    spin_lock_irqsave(&sdev->priv_lock, flags);
    list_move_tail(&urb->urb_list, &pei->urb_completed);
    r = atomic_dec_return(&pei->urb_submitted);
    //printk("c[%d]", r);

    for (i = 0; i < urb->number_of_packets; i++) {
        if (urb->iso_frame_desc[i].status == -EXDEV) {
            pei->state = ISO_ERR;
            break;
        }
    }

    if (pei->state == ISO_STOPPING && !list_empty(&pei->priv_wait)) {
        pei->state = ISO_STARTED;
    }
    //if we are ready to submit the next urb, submit it
    if (pei->state == ISO_STARTED) {
submit_urb:
        if (list_empty(&pei->priv_wait)) {
            //duplicate an urb request only when interface is still alive.
            if (pei->intf != -1 && r < ISO_URB_SUBMIT_LOW_BOUND) {
                printk("e");
                pei->state = ISO_STOPPING;
                
                spin_unlock_irqrestore(&sdev->priv_lock, flags);
                //submit an dummy urb.
                ret = stub_dup_in_urb_submit(sdev, urb);
                spin_lock_irqsave(&sdev->priv_lock, flags);
                if (ret) {
                    udbg("ret=%d(%d)\n", ret, -EXDEV);
                    pei->state = ISO_ERR;
                }
            }
            r = atomic_read(&pei->urb_submitted);
        } else {
            //submit the next urb;
            priv = list_entry(pei->priv_wait.next, struct stub_priv, list);
            urb = priv->urb;
            q_len = atomic_sub_return(urb->number_of_packets, &pei->waiting);
            //save the pei into context, instead of priv.
            urb->context = (void*)pei;
            //say goodbye to the urb.
            priv->urb = NULL;
            list_move_tail(&priv->list, &pei->priv_submitted);
            atomic_inc_return(&pei->submitted);            
            r = atomic_inc_return(&pei->urb_submitted);
            //printk("n(%d)\n", q_len);
            spin_unlock_irqrestore(&sdev->priv_lock, flags);

            ret = usb_submit_urb(urb, GFP_ATOMIC);

            spin_lock_irqsave(&sdev->priv_lock, flags);
            if (ret) {
                udbg("ret=%d(%d)\n", ret, -EXDEV);
                r = atomic_dec_return(&pei->urb_submitted);
                atomic_dec_return(&pei->submitted);
                priv->urb = urb;
                urb->context = (void*)priv;
                q_len = atomic_add_return(urb->number_of_packets, &pei->waiting);
                list_move(&priv->list, &pei->priv_wait);
                pei->state = ISO_ERR;
            }
        }
        //BruceToDo. check urb_submitted upper bound.
        if ((r < ISO_URB_SUBMIT_LOW_BOUND) 
            && (pei->state == ISO_STARTED)
            && (pei->intf != -1)) {
            // if request number is too low, we should submit more then 1 request here.
            printk("l");
            goto submit_urb;
        }

    }

    //dequeue "priv" from the "submitted" list and complete the urb.
    while (!list_empty(&pei->priv_submitted) && !list_empty(&pei->urb_completed)) {
        priv = list_entry(pei->priv_submitted.next, struct stub_priv, list);
        urb =  list_entry(pei->urb_completed.next, struct urb, urb_list);
        priv->urb = urb;
        urb->context = (void*)priv; //MUST! tx_loop() will need it.
        list_del_init(&urb->urb_list);
        //printk("t");

        /* link a urb to the queue of tx. */
        if (priv->unlinking) {
            stub_enqueue_ret_unlink(sdev, priv->seqnum, urb->status);
            stub_free_priv_and_urb(priv);
        } else
            list_move_tail(&priv->list, &sdev->priv_tx);
    }


    if ((pei->state == ISO_ERR || pei->state == ISO_STOPPING)
        && list_empty(&pei->priv_submitted)) {
        struct stub_priv *t;
        struct urb *u;
        //Complete all the requests in "wait" list if ISO_ERR
        list_for_each_entry_safe(priv, t, &pei->priv_wait, list) {
            q_len = atomic_sub_return(priv->urb->number_of_packets, &pei->waiting);
            priv->urb->status = -EXDEV;
            priv->urb->actual_length = 0;
            list_move_tail(&priv->list, &sdev->priv_tx);
        }
        printk("f(%d)urb(%d)\n", q_len, atomic_read(&pei->urb_submitted));

        //free the extra urbs
        list_for_each_entry_safe(urb, u, &pei->urb_completed, urb_list) {
            list_del_init(&urb->urb_list);
            stub_free_urb(urb);
        }
        printk("done(%d)\n", atomic_read(&pei->urb_submitted));

        if (!atomic_read(&pei->urb_submitted))
            pei->state = ISO_IDLE;
    }


    spin_unlock_irqrestore(&sdev->priv_lock, flags);

    /* wake up tx_thread */
    wake_up(&sdev->tx_waitq);
}

/**
 * stub_complete - completion handler of a usbip urb
 * @urb: pointer to the urb completed
 * @regs:
 *
 * When a urb has completed, the USB core driver calls this function mostly in
 * the interrupt context. To return the result of a urb, the completed urb is
 * linked to the pending list of returning.
 *
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
void stub_complete(struct urb *urb)
#else
void stub_complete(struct urb *urb, struct pt_regs *regs)
#endif
{
	struct stub_priv *priv;
	struct stub_device *sdev;

	dbg_stub_tx("complete! status %d\n", urb->status);


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

        //BruceTestOnly
#if 1
        if (usb_pipeisoc(urb->pipe)) {
            u32 i;
            //u32 d;
            for (i = 0; i < urb->number_of_packets; i++) {
                if (urb->iso_frame_desc[i].status == -EXDEV) {
                    printk("?");
                }
            }
        }
#endif

    if (usb_pipeisoc(urb->pipe) && usb_pipein(urb->pipe)) {
#if 0//DOUBLE_CHK
        if (1/*urb->actual_length == 0*/) {
            struct usb_iso_packet_descriptor *isodesc = urb->iso_frame_desc;
            u32 _i;
            
            udbg("iso_num=%d, err_cnt=%d, a_len=%d, t_len=%d\n", 
                urb->number_of_packets, 
                urb->error_count, urb->actual_length, urb->transfer_buffer_length);
            for (_i = 0; _i < urb->number_of_packets; _i++) {
                udbg("offset=%d, len=%d, a_len=%d, status=%d, buf=%d\n",
                    isodesc[_i].offset, 
                    isodesc[_i].length, 
                    isodesc[_i].actual_length, 
                    isodesc[_i].status,
                    *(u32*)(((unsigned char*)urb->transfer_buffer) + isodesc[_i].offset));
            }

        }
#endif

#if 0 //Bruce100331. Why we doing this?! I think it is a buggy code.
        if (!urb->status && urb->actual_length == 0) {
            urb->actual_length = urb->transfer_buffer_length;
        }
#endif
    }

    if (usb_pipeisoc(urb->pipe) && usb_pipein(urb->pipe)) {
        struct priv_ep_info *pei;
        pei = (struct priv_ep_info *)urb->context;
        sdev = pei->sdev;
        stub_complete_iso_in(sdev, pei, urb);

        return;
    } else {
        priv = (struct stub_priv *) urb->context;
        sdev = priv->sdev;
        stub_complete_priv(sdev, priv);
    }

}


/*-------------------------------------------------------------------------*/
/* fill PDU */

static inline void setup_base_pdu(struct usbip_header_basic *base,
		__u32 command, __u32 seqnum)
{
	base->command = command;
	base->seqnum  = seqnum;
	base->devid   = 0;
	base->ep      = 0;
	base->direction   = 0;
}

static void setup_ret_submit_pdu(struct usbip_header *rpdu, struct urb *urb)
{
	struct stub_priv *priv = (struct stub_priv *) urb->context;

	setup_base_pdu(&rpdu->base, USBIP_RET_SUBMIT, priv->seqnum);

	usbip_pack_pdu(rpdu, urb, USBIP_RET_SUBMIT, 1);
}

static void setup_ret_unlink_pdu(struct usbip_header *rpdu,
		struct stub_unlink *unlink)
{
	setup_base_pdu(&rpdu->base, USBIP_RET_UNLINK, unlink->seqnum);

	rpdu->u.ret_unlink.status = unlink->status;
}


/*-------------------------------------------------------------------------*/
/* send RET_SUBMIT */

static struct stub_priv *dequeue_from_priv_tx(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv, *tmp;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(priv, tmp, &sdev->priv_tx, list) {
		list_move_tail(&priv->list, &sdev->priv_free);
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return NULL;
}
#if 0
static int stub_send_ret_submit(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv, *tmp;

	struct msghdr msg;
	struct kvec iov[3];
	size_t txsize;

	size_t total_size = 0;

	while ((priv = dequeue_from_priv_tx(sdev)) != NULL) {
		int ret;
		struct urb *urb = priv->urb;
		struct usbip_header pdu_header;
		void *iso_buffer = NULL;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&msg, 0, sizeof(msg));
		memset(&iov, 0, sizeof(iov));

		dbg_stub_tx("setup txdata urb %p\n", urb);


		/* 1. setup usbip_header */
		setup_ret_submit_pdu(&pdu_header, urb);
		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		/* 2. setup transfer buffer */
		if (usb_pipein(urb->pipe) && urb->actual_length > 0) {
			iov[1].iov_base = urb->transfer_buffer;
			iov[1].iov_len  = urb->actual_length;
			txsize += urb->actual_length;
		}

		/* 3. setup iso_packet_descriptor */
		if (usb_pipetype(urb->pipe) == PIPE_ISOCHRONOUS) {
			ssize_t len = 0;

			iso_buffer = usbip_alloc_iso_desc_pdu(urb, &len);
			if (!iso_buffer) {
				usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_MALLOC);
				return -1;
			}

			iov[2].iov_base = iso_buffer;
			iov[2].iov_len  = len;
			txsize += len;
		}

again: //Bruce. try to workaround the -EFAULT problem
		ret = kernel_sendmsg(sdev->ud.tcp_socket, &msg, iov, 3, txsize);
		if (ret != txsize) {
			if (ret == -EFAULT)
				goto again;
			uerr("sendmsg failed!, retval %d for %zd\n", ret, txsize);
			if (iso_buffer)
				kfree(iso_buffer);
			usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_TCP);
			return -1;
		}

		if (iso_buffer)
			kfree(iso_buffer);

		dbg_stub_tx("send txdata\n");

		total_size += txsize;
	}


	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(priv, tmp, &sdev->priv_free, list) {
		stub_free_priv_and_urb(priv);
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return total_size;
}
#else
static int stub_send_ret_submit(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv, *tmp;

	struct kvec iov[3];
	size_t txsize;

	size_t total_size = 0;

	while ((priv = dequeue_from_priv_tx(sdev)) != NULL) {
		int ret;
		struct urb *urb = priv->urb;
		struct usbip_header pdu_header;
		void *iso_buffer = NULL;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&iov, 0, sizeof(iov));

		dbg_stub_tx("setup txdata urb %p\n", urb);

		/*
		** Bruce120117. EHCI driver will not update urb->actual_length
		** under (full speed?) ISO case. I'm not sure whether it is a bug
		** or not, but it is safe to ignore the "urb->actual_length > 0" check
		*/
		if (usb_pipein(urb->pipe)/* && urb->actual_length > 0*/) {
			/*
			** Bruce130523. host vhub driver relies on urb->actual_length
			** to receive data from client driver. We must make sure
			** urb->actual_length is correct under any case.
			** For the case of urb fail (urb->status is non-zero), the urb->actual_length could
			** be undefined. setup_ret_submit_pdu() will package the urb->actual_length into
			** pdu_header, so we should correct the actual_lenth before calling setup_ret_submit_pdu().
			*/
			if (urb->status) {
				urb->actual_length = 0;
			}
			
			/*
			** ISO urbs may contain multiple packets in one urb
			** which causes discontinuous data in urb->transfer_buffer.
			** The most easy way is resolve this is to transfer ALL
			** allocated buffer back to host.
			** We need to recover urb->actual_length in the host side
			** under this case.
			*/
			if (usb_pipetype(urb->pipe) == PIPE_ISOCHRONOUS)
			{
				urb->actual_length = urb->transfer_buffer_length;
			}
		}

		/* 1. setup usbip_header */
		setup_ret_submit_pdu(&pdu_header, urb);
		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		/* 2. setup transfer buffer */
		/* Bruce121224. AST1510 UHCI + C-Media audio Failed. Due to wrong actual
		** length info on stall. Copy wrong actual_length data corrupts memory.
		** Solution: actual_length is meaningless when urb->status is fail.
		** Bruce130523. Move urb->status check to above before calling setup_ret_submit_pdu().
		** See above comment.
		*/
		if (usb_pipein(urb->pipe) && urb->actual_length > 0) {
			if (urb->actual_length > urb->transfer_buffer_length) {
				uerr("!!!urb actual len (%d) > buf len (%d)\n", urb->actual_length, urb->transfer_buffer_length);
				/* Bruce. It is a "try" to workaround unexpected problems. The problem is actually not seen yet. */
				urb->actual_length = urb->transfer_buffer_length;
			}
			
			iov[1].iov_base = urb->transfer_buffer;
			iov[1].iov_len  = urb->actual_length;
			txsize += urb->actual_length;
		}

		/* 3. setup iso_packet_descriptor */
		if (usb_pipetype(urb->pipe) == PIPE_ISOCHRONOUS) {
			ssize_t len = 0;

			iso_buffer = usbip_alloc_iso_desc_pdu(urb, &len);
			if (!iso_buffer) {
				usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_MALLOC);
				return -1;
			}

			iov[2].iov_base = iso_buffer;
			iov[2].iov_len  = len;
			txsize += len;
		}

again: //Bruce. try to workaround the -EFAULT problem
		ret = usbip_xmit_ex(1, sdev->ud.tcp_socket, iov, 3, txsize, 0);
		if (ret != txsize) {
			if (ret == -EFAULT)
				goto again;
			uerr("sendmsg failed!, retval %d for %zd\n", ret, txsize);
			if (iso_buffer)
				kfree(iso_buffer);
			usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_TCP);
			return -1;
		}

		if (iso_buffer)
			kfree(iso_buffer);

		dbg_stub_tx("send txdata\n");

		total_size += txsize;
	}


	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(priv, tmp, &sdev->priv_free, list) {
		stub_free_priv_and_urb(priv);
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return total_size;
}
#endif

/*-------------------------------------------------------------------------*/
/* send RET_UNLINK */

static struct stub_unlink *dequeue_from_unlink_tx(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_unlink *unlink, *tmp;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(unlink, tmp, &sdev->unlink_tx, list) {
		list_move_tail(&unlink->list, &sdev->unlink_free);
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return unlink;
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return NULL;
}

#if 0
static int stub_send_ret_unlink(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_unlink *unlink, *tmp;

	struct msghdr msg;
	struct kvec iov[1];
	size_t txsize;

	size_t total_size = 0;

	while ((unlink = dequeue_from_unlink_tx(sdev)) != NULL) {
		int ret;
		struct usbip_header pdu_header;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&msg, 0, sizeof(msg));
		memset(&iov, 0, sizeof(iov));

		dbg_stub_tx("setup ret unlink %lu\n", unlink->seqnum);

		/* 1. setup usbip_header */
		setup_ret_unlink_pdu(&pdu_header, unlink);
		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		ret = kernel_sendmsg(sdev->ud.tcp_socket, &msg, iov, 1, txsize);
		if (ret != txsize) {
			uerr("sendmsg failed!, retval %d for %zd\n", ret, txsize);
			usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_TCP);
			return -1;
		}


		dbg_stub_tx("send txdata\n");

		total_size += txsize;
	}


	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(unlink, tmp, &sdev->unlink_free, list) {
		list_del(&unlink->list);
		kfree(unlink);
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return total_size;
}
#else
static int stub_send_ret_unlink(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_unlink *unlink, *tmp;

	struct kvec iov[1];
	size_t txsize;

	size_t total_size = 0;

	while ((unlink = dequeue_from_unlink_tx(sdev)) != NULL) {
		int ret;
		struct usbip_header pdu_header;

		txsize = 0;
		memset(&pdu_header, 0, sizeof(pdu_header));
		memset(&iov, 0, sizeof(iov));

		dbg_stub_tx("setup ret unlink %lu\n", unlink->seqnum);

		/* 1. setup usbip_header */
		setup_ret_unlink_pdu(&pdu_header, unlink);
		usbip_header_correct_endian(&pdu_header, 1);

		iov[0].iov_base = &pdu_header;
		iov[0].iov_len  = sizeof(pdu_header);
		txsize += sizeof(pdu_header);

		ret = usbip_xmit_ex(1, sdev->ud.tcp_socket, iov, 1, txsize, 0);
		if (ret != txsize) {
			uerr("sendmsg failed!, retval %d for %zd\n", ret, txsize);
			usbip_event_add(&sdev->ud, SDEV_EVENT_ERROR_TCP);
			return -1;
		}


		dbg_stub_tx("send txdata\n");

		total_size += txsize;
	}


	spin_lock_irqsave(&sdev->priv_lock, flags);

	list_for_each_entry_safe(unlink, tmp, &sdev->unlink_free, list) {
		list_del(&unlink->list);
		kfree(unlink);
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);

	return total_size;
}
#endif

/*-------------------------------------------------------------------------*/

void stub_tx_loop(struct usbip_task *ut)
{
	struct usbip_device *ud = container_of(ut, struct usbip_device, tcp_tx);
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);

	while (1) {
		if (signal_pending(current)) {
			dbg_stub_tx("signal catched\n");
			break;
		}

		if (usbip_event_happend(ud))
			break;

		/*
		 * send_ret_submit comes earlier than send_ret_unlink.  stub_rx
		 * looks at only priv_init queue. If the completion of a URB is
		 * earlier than the receive of CMD_UNLINK, priv is moved to
		 * priv_tx queue and stub_rx does not find the target priv. In
		 * this case, vhci_rx receives the result of the submit request
		 * and then receives the result of the unlink request. The
		 * result of the submit is given back to the usbcore as the
		 * completion of the unlink request. The request of the
		 * unlink is ignored. This is ok because a driver who calls
		 * usb_unlink_urb() understands the unlink was too late by
		 * getting the status of the given-backed URB which has the
		 * status of usb_submit_urb().
		 */
		if (stub_send_ret_submit(sdev) < 0)
			break;

		if (stub_send_ret_unlink(sdev) < 0)
			break;

		wait_event_interruptible(sdev->tx_waitq,
				(!list_empty(&sdev->priv_tx) ||
				 !list_empty(&sdev->unlink_tx)));
	}
}
