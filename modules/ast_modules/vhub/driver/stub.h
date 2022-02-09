/*
 * $Id: stub.h 76 2008-04-29 04:36:43Z hirofuchi $
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
#ifndef __STUB_H__
#define __STUB_H__

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/workqueue.h>
#define USE_NETLINK_USERSOCK 0

/* Used for fix ISO audio stop issue. */
/* 
** To fix this issue: To avoid q_length count to 1.
** Repeat urb request when q_length < ISO_REPEAT_THREASHOLD
** If stream stop detected, wait for q_length count to 0 before next urb request.
*/
enum i_state {
    ISO_IDLE = 0,
    ISO_STARTING,
    ISO_STARTED,
    ISO_STOPPING,
    ISO_ERR
};
#define ISO_SUBMIT_LOW_BOUND  50 // ms
#define ISO_URB_SUBMIT_LOW_BOUND  10 // # of urbs
#define ISO_URB_SUBMIT_UP_BOUND  10 // # of urbs


struct priv_ep_info {
    struct stub_device *sdev;
    struct list_head priv_submitted; //local request which is submitted to usbd
    struct list_head priv_wait; //remote request which is waiting for process
    struct list_head urb_completed; //local request which is completed from usbd
    atomic_t waiting; //number of requests in "wait" list
    atomic_t submitted; //number of requests in "submitted" list
    atomic_t urb_submitted; //number of urb requests submitted to usbd.
    enum i_state state;
    struct work_struct work;
    short intf; //interface number. -1 means not used.
};

struct stub_device {
	struct usb_interface *interface;
	struct list_head list;

	struct usbip_device ud;
	__u32 devid;

	/*
	 * stub_priv preserves private data of each urb.
	 * It is allocated as stub_priv_cache and assigned to urb->context.
	 *
	 * stub_priv is always linked to any one of 3 lists;
	 *	priv_init: linked to this until the comletion of a urb.
	 *	priv_tx  : linked to this after the completion of a urb.
	 *	priv_free: linked to this after the sending of the result.
	 *
	 * Any of these list operations should be locked by priv_lock.
	 */
	spinlock_t priv_lock;
	struct list_head priv_init;
	struct list_head priv_tx;
	struct list_head priv_free;

	/* see comments for unlinking in stub_rx.c */
	struct list_head unlink_tx;
	struct list_head unlink_free;

	struct priv_ep_info ep_out[16];
	struct priv_ep_info ep_in[16];


	wait_queue_head_t tx_waitq;

	struct workqueue_struct *wq;
    
};

/* private data into urb->priv */
struct stub_priv {
	unsigned long seqnum;
	struct list_head list;
	struct stub_device *sdev;
	struct urb *urb;

	int unlinking;
};

struct stub_unlink {
	unsigned long seqnum;
	struct list_head list;
	__u32 status;
};


extern struct kmem_cache *stub_priv_cache;


/*-------------------------------------------------------------------------*/
/* prototype declarations */

/* stub_tx.c */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
void stub_complete(struct urb *);
#else
void stub_complete(struct urb *, struct pt_regs *);
#endif
void stub_tx_loop(struct usbip_task *);

/* stub_dev.c */
extern struct usb_driver stub_driver;

/* stub_rx.c */
void stub_rx_loop(struct usbip_task *);
void stub_enqueue_ret_unlink(struct stub_device *, __u32, __u32);
void tweak_stop_iso_out(struct stub_device *sdev, struct priv_ep_info *pei);
void tweak_stop_iso_in(struct stub_device *sdev, struct priv_ep_info *pei);

/* stub_main.c */
int match_busid(char *busid);
int del_match_busid(char *busid);
void stub_device_cleanup_urbs(struct stub_device *sdev);
void stub_free_urb(struct urb *urb);
void stub_iso_out_shutdown(struct stub_device *sdev);
void stub_iso_in_shutdown(struct stub_device *sdev);

#if USE_NETLINK_USERSOCK
void notify_user(char *busid);
#else
#define notify_user(busid)
#endif


#endif
