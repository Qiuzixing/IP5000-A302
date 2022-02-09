/*
 * $Id: stub_main.c 76 2008-04-29 04:36:43Z hirofuchi $
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

//Used for ast_notify_user()
#include <asm/arch/platform.h>

/* Version Information */
#define DRIVER_VERSION "$Id: stub_main.c 76 2008-04-29 04:36:43Z ASPEED $"
#define DRIVER_AUTHOR "Bruce"
#define DRIVER_DESC "Stub Driver for USBoIP"



/* stub_priv is allocated from stub_priv_cache */
struct kmem_cache *stub_priv_cache = NULL;


/*-------------------------------------------------------------------------*/

/* Define sysfs entries for the usbip driver */


/*
 * busid_tables defines matching busids that usbip can grab. A user can change
 * dynamically what device is locally used and what device is exported to a
 * remote host.
 */
#define MAX_BUSID 16
static char busid_table[MAX_BUSID][BUS_ID_SIZE];
static spinlock_t busid_table_lock;


int match_busid(char *busid)
{
	int i;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i][0])
			if (!strncmp(busid_table[i], busid, BUS_ID_SIZE)) {
				/* already registerd */
				spin_unlock(&busid_table_lock);
				return 0;
			}

	spin_unlock(&busid_table_lock);

	return 1;
}

static ssize_t show_match_busid(struct device_driver *drv, char *buf)
{
	int i;
	char *out = buf;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (busid_table[i][0])
			out += sprintf(out, "%s ", busid_table[i]);

	spin_unlock(&busid_table_lock);

	out += sprintf(out, "\n");

	return out - buf;
}

static int add_match_busid(char *busid)
{
	int i;

	if (!match_busid(busid))
		return 0;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (!busid_table[i][0]) {
			strncpy(busid_table[i], busid, BUS_ID_SIZE);
			spin_unlock(&busid_table_lock);
			return 0;
		}

	spin_unlock(&busid_table_lock);

	return -1;
}

int del_match_busid(char *busid)
{
	int i;

	spin_lock(&busid_table_lock);

	for (i = 0; i < MAX_BUSID; i++)
		if (!strncmp(busid_table[i], busid, BUS_ID_SIZE)) {
			/* found */
			memset(busid_table[i], 0, BUS_ID_SIZE);
			spin_unlock(&busid_table_lock);
			return 0;
		}

	spin_unlock(&busid_table_lock);

	return -1;
}

static ssize_t store_match_busid(struct device_driver *dev, const char *buf,
		size_t count)
{
	int len;
	char busid[BUS_ID_SIZE];

	if (count < 5)
		return -EINVAL;

	/* strnlen() does not include \0 */
	len = strnlen(buf + 4, BUS_ID_SIZE);

	/* busid needs to include \0 termination */
	if (!(len < BUS_ID_SIZE))
		return -EINVAL;

	strncpy(busid, buf + 4, BUS_ID_SIZE);


	if (!strncmp(buf, "add ", 4)) {
		if (add_match_busid(busid) < 0)
			return -ENOMEM;
		else {
			udbg("add busid %s\n", busid);
			return count;
		}
	} else if (!strncmp(buf, "del ", 4)) {
		if (del_match_busid(busid) < 0)
			return -ENODEV;
		else {
			udbg("del busid %s\n", busid);
			return count;
		}
	} else
		return -EINVAL;
}

static DRIVER_ATTR(match_busid, S_IRUSR|S_IWUSR, show_match_busid, store_match_busid);



/*-------------------------------------------------------------------------*/

/* Cleanup functions used to free private data */

static struct stub_priv *stub_priv_pop_from_listhead(struct list_head *listhead)
{
	struct stub_priv *priv, *tmp;

	list_for_each_entry_safe(priv, tmp, listhead, list) {
		list_del(&priv->list);
		return priv;
	}

	return NULL;
}

/* Move all ISO eps to ISO_IDLE state and set alt to 0. */
/* After this function, all priv should moved out of pei. */
void stub_iso_in_shutdown(struct stub_device *sdev)
{
    int e, ret;
    struct usb_device *udev = interface_to_usbdev(sdev->interface);
    struct usb_interface *iface;
    struct priv_ep_info *pei;

    if (udev->state == USB_STATE_SUSPENDED)
        return;

    for (e = 0; e < 16; e++) {
        short intf;
        
        pei = &sdev->ep_in[e];
        intf = pei->intf;
        if (intf == -1)
            continue;
        
        iface = usb_ifnum_to_if(udev, intf);
        if (!iface) {
            continue;
        }

        tweak_stop_iso_in(sdev, pei);

        ret = usb_set_interface(udev, intf, 0);
        if (ret < 0)
        	uerr("set_interface error: inf %u alt %u, %d\n",
        			intf, 0, ret);
        else
        	uinfo("set_interface done: inf %u alt %u\n", intf, 0);
            
#if DOUBLE_CHK
        if (pei->state != ISO_IDLE)
            uerr("wrong state (%d)\n", pei->state);
        if (!list_empty(&pei->priv_submitted))
            uerr("priv_submitted is not empty?!\n");
        if (!list_empty(&pei->priv_wait))
            uerr("priv_wait is not empty?!\n");
        if (!list_empty(&pei->urb_completed))
            uerr("urb_completed is not empty?!\n");
#endif
    }

    
}



/* Move all ISO eps to ISO_IDLE state and set alt to 0. */
/* After this function, all priv should moved out of pei. */
void stub_iso_out_shutdown(struct stub_device *sdev)
{
    int e, ret;
    struct usb_device *udev = interface_to_usbdev(sdev->interface);
    struct usb_interface *iface;
    struct priv_ep_info *pei;

    if (udev->state == USB_STATE_SUSPENDED)
        return;

    for (e = 0; e < 16; e++) {
        short intf;
        
        pei = &sdev->ep_out[e];
        intf = pei->intf;
        if (intf == -1)
            continue;
        
        iface = usb_ifnum_to_if(udev, intf);
        if (!iface) {
            continue;
        }

        tweak_stop_iso_out(sdev, pei);

        ret = usb_set_interface(udev, intf, 0);
        if (ret < 0)
        	uerr("set_interface error: inf %u alt %u, %d\n",
        			intf, 0, ret);
        else
        	uinfo("set_interface done: inf %u alt %u\n", intf, 0);
            
            
#if DOUBLE_CHK
        if (pei->state != ISO_IDLE)
            uerr("wrong state (%d)\n", pei->state);
        if (!list_empty(&pei->priv_submitted))
            uerr("priv_submitted is not empty?!\n");
        if (!list_empty(&pei->priv_wait))
            uerr("priv_wait is not empty?!\n");
        if (!list_empty(&pei->urb_completed))
            uerr("urb_completed is not empty?!\n");
#endif
    }
}


static struct stub_priv *stub_priv_pop(struct stub_device *sdev)
{
	unsigned long flags;
	struct stub_priv *priv;

	spin_lock_irqsave(&sdev->priv_lock, flags);

	priv = stub_priv_pop_from_listhead(&sdev->priv_init);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	priv = stub_priv_pop_from_listhead(&sdev->priv_tx);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	priv = stub_priv_pop_from_listhead(&sdev->priv_free);
	if (priv) {
		spin_unlock_irqrestore(&sdev->priv_lock, flags);
		return priv;
	}

	spin_unlock_irqrestore(&sdev->priv_lock, flags);
	return NULL;
}

void stub_device_cleanup_urbs(struct stub_device *sdev)
{
	struct stub_priv *priv;

	udbg("free sdev %p\n", sdev);

	while ((priv = stub_priv_pop(sdev))) {
		struct urb *urb = priv->urb;

		udbg("   free urb %p\n", urb);
		/* Bruce. I found that usb_kill_urb() will sometimes deadlock the whole system.
		** To avoid this situation, all submitted urbs MUST completed before
		** stub_device_cleanup_urbs(). (Which means calling usb_kill_urb() is pointless)
		** So, I move the whole clean up routine to "stub_disconnect()" from
		** "stub_shutdown_connection()".
		*/
		usb_kill_urb(urb);

		kmem_cache_free(stub_priv_cache, priv);

		if (urb->transfer_buffer != NULL)
			kfree(urb->transfer_buffer);

		if (urb->setup_packet != NULL)
			kfree(urb->setup_packet);

		usb_free_urb(urb);
	}
}


/*-------------------------------------------------------------------------*/
#if USE_NETLINK_USERSOCK
struct sock *nl_sk = NULL;
#include <linux/netlink.h>
//#include <linux/skbuff.h>

#define MAX_PAYLOAD 1024

void notify_user(char *busid)
{
    struct sk_buff *skb = NULL;
    struct nlmsghdr *nlh;
    int err;
    static int seq = 0;
    
    skb = alloc_skb(NLMSG_SPACE(MAX_PAYLOAD), GFP_KERNEL);
    if (!skb) {
        uerr("failed to allocate skb\n");
        goto nlmsg_failure;
    }
    
    nlh = NLMSG_NEW(skb, 
                0, /* pid. from kernel. */
                seq++, 
                0, /* message content. */
                BUS_ID_SIZE, /* length */
                0 /* flags */
                );

    strncpy(NLMSG_DATA(nlh), busid, BUS_ID_SIZE);

    /* sender is in group 1<<0 */
    NETLINK_CB(skb).pid = 0;  /* from kernel */
    NETLINK_CB(skb).dst_pid = 0;  /* multicast */
    /* to mcast group 1<<1 */
    NETLINK_CB(skb).dst_group = 1;

    /*multicast the message to all listening processes*/
    err = netlink_broadcast(nl_sk, skb, 0, 1/*dst_groups*/, GFP_KERNEL);
    if (err)
        uerr("netlink_broadcast failed?! (%d)\n", err);

nlmsg_failure:
    return;
}
#endif

static int __init usb_stub_init(void)
{
	int ret;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,22)
	stub_priv_cache = kmem_cache_create("stub_priv", sizeof(struct stub_priv),
			0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
	stub_priv_cache = kmem_cache_create("stub_priv", sizeof(struct stub_priv),
			0, SLAB_HWCACHE_ALIGN, NULL);
#endif
	if (!stub_priv_cache) {
		uerr("create stub_priv_cache\n");
		return -ENOMEM;
	}

	ret = usb_register(&stub_driver);
	if (ret) {
		uerr("usb_register failed %d\n", ret);
		return ret;
	}


	uinfo(DRIVER_DESC "" DRIVER_VERSION);

	memset(busid_table, 0, sizeof(busid_table));
	spin_lock_init(&busid_table_lock);

#if USE_NETLINK_USERSOCK
	nl_sk = netlink_kernel_create(NETLINK_USERSOCK, 1, NULL, THIS_MODULE);
	if (!nl_sk) {
		uerr("can't open netlink sk!?(%p)\n", nl_sk);
		return -ENODEV;
	}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
	ret = driver_create_file(&stub_driver.driver, &driver_attr_match_busid);
#else
	ret = driver_create_file(&stub_driver.drvwrap.driver, &driver_attr_match_busid);
#endif /* if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18) */

	if (ret) {
		uerr("create driver sysfs\n");
		return ret;
	}

	//A7 removed
	//ast_notify_user("e_usbip_init_ok");
	return ret;
}

static void __exit usb_stub_exit(void)
{
	udbg("enter\n");

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18)
	driver_remove_file(&stub_driver.driver, &driver_attr_match_busid);
#else
	driver_remove_file(&stub_driver.drvwrap.driver, &driver_attr_match_busid);
#endif /* LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18) */

#if USE_NETLINK_USERSOCK
	if (nl_sk)
		sock_release(nl_sk->sk_socket);
#endif
	/*
	 * deregister() calls stub_disconnect() for all devices. Device
	 * specific data is cleared in stub_disconnect().
	 */
	usb_deregister(&stub_driver);

	kmem_cache_destroy(stub_priv_cache);

	//A7 removed
	//ast_notify_user("e_usbip_stopped");
	udbg("bye\n");
}




module_init (usb_stub_init);
module_exit (usb_stub_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
