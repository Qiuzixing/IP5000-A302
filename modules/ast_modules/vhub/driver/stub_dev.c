/*
 * $Id: stub_dev.c 76 2008-04-29 04:36:43Z hirofuchi $
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

#include <linux/file.h> //For sockfd_put()
#include "usbip_common.h"
#include "stub.h"



static int stub_probe(struct usb_interface *interface,  const struct usb_device_id *id);
static void stub_disconnect(struct usb_interface *interface);


/*
 * Define device IDs here if you want to explicitly limit exportable devices.
 * In the most cases, wild card matching will be ok because driver binding can
 * be changed dynamically by a userland program.
 */
static struct usb_device_id stub_table[] = {
#if 0
	/* just an example */
	{ USB_DEVICE(0x05ac, 0x0301) },   /* Mac 1 button mouse */
	{ USB_DEVICE(0x0430, 0x0009) },   /* Plat Home Keyboard */
	{ USB_DEVICE(0x059b, 0x0001) },   /* Iomega USB Zip 100 */
	{ USB_DEVICE(0x04b3, 0x4427) },   /* IBM USB CD-ROM */
	{ USB_DEVICE(0x05a9, 0xa511) },   /* LifeView USB cam */
	{ USB_DEVICE(0x55aa, 0x0201) },   /* Imation card reader */
	{ USB_DEVICE(0x046d, 0x0870) },   /* Qcam Express(QV-30) */
	{ USB_DEVICE(0x04bb, 0x0101) },   /* IO-DATA HD 120GB */
	{ USB_DEVICE(0x04bb, 0x0904) },   /* IO-DATA USB-ET/TX */
	{ USB_DEVICE(0x04bb, 0x0201) },   /* IO-DATA USB-ET/TX */
	{ USB_DEVICE(0x08bb, 0x2702) },   /* ONKYO USB Speaker */
	{ USB_DEVICE(0x046d, 0x08b2) },   /* Logicool Qcam 4000 Pro */
#endif
	/* magic for wild card */
	{ .driver_info = 1 },
	{ }                                     /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, stub_table);


struct usb_driver stub_driver = {
	.name		= "usbip",

	.probe		= stub_probe,
	.disconnect	= stub_disconnect,

	.id_table	= stub_table,
};



/*-------------------------------------------------------------------------*/

/* Define sysfs entries for a usbip-bound device */


/*
 * usbip_status shows status of usbip as long as this driver is bound to the
 * target device.
 */
static ssize_t show_status(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int status;

	if (!sdev) {
		uerr("sdev is null\n");
		return -ENODEV;
	}

	spin_lock(&sdev->ud.lock);
	status = sdev->ud.status;
	spin_unlock(&sdev->ud.lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", status);
}
static DEVICE_ATTR(usbip_status, S_IRUGO, show_status, NULL);


/*
 * usbip_sockfd gets a socket descriptor of an established TCP connection that
 * is used to transfer usbip requests by kernel threads. -1 is a magic number
 * by which usbip connection is finished.
 */
static ssize_t store_sockfd(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int sockfd = 0;
	struct socket *socket;

	if (!sdev) {
		uerr("sdev is null\n");
		return -ENODEV;
	}

	sscanf(buf, "%d", &sockfd);

	if (sockfd != -1) {
		uinfo("stub up\n");

		spin_lock(&sdev->ud.lock);

		if (sdev->ud.status != SDEV_ST_AVAILABLE) {
			uerr("not ready\n");
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

		socket = sockfd_to_socket(sockfd);
		if (!socket) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

#if 0
		setnodelay(socket);
		setkeepalive(socket);
		setreuse(socket);
#endif

		sdev->ud.tcp_socket = socket;

		spin_unlock(&sdev->ud.lock);

		usbip_start_threads(&sdev->ud);

		spin_lock(&sdev->ud.lock);
		sdev->ud.status = SDEV_ST_USED;
		spin_unlock(&sdev->ud.lock);

	} else {
		uinfo("stub down\n");

		spin_lock(&sdev->ud.lock);
		if (sdev->ud.status != SDEV_ST_USED) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}
		spin_unlock(&sdev->ud.lock);

		usbip_event_add(&sdev->ud, SDEV_EVENT_DOWN);
	}

	return count;
}
static DEVICE_ATTR(usbip_sockfd, S_IWUSR, NULL, store_sockfd);

static int stub_add_files(struct device *dev)
{
	int err = 0;

	err = device_create_file(dev, &dev_attr_usbip_status);
	if (err)
		goto err_status;

	err = device_create_file(dev, &dev_attr_usbip_sockfd);
	if (err)
		goto err_sockfd;

	err = device_create_file(dev, &dev_attr_usbip_debug);
	if (err)
		goto err_debug;

	return 0;

err_debug:
	device_remove_file(dev, &dev_attr_usbip_sockfd);

err_sockfd:
	device_remove_file(dev, &dev_attr_usbip_status);

err_status:
	return err;
}

static void stub_remove_files(struct device *dev)
{
	device_remove_file(dev, &dev_attr_usbip_status);
	device_remove_file(dev, &dev_attr_usbip_sockfd);
	device_remove_file(dev, &dev_attr_usbip_debug);
}



/*-------------------------------------------------------------------------*/

/* Event handler functions called by an event handler thread */

static void stub_shutdown_connection(struct usbip_device *ud)
{
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);

	/*
	 * When removing an exported device, kernel panic sometimes occurred
	 * and then EIP was sk_wait_data of stub_rx thread. Is this because
	 * sk_wait_data returned though stub_rx thread was already finished by
	 * step 1?
	 */
	if (ud->tcp_socket) {
		udbg("shutdown tcp_socket %p\n", ud->tcp_socket);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
		/* in /usr/include/sys/socket.c, SHUT_RDWR is defined as 2. */
		ud->tcp_socket->ops->shutdown(ud->tcp_socket, 2);
#else
		kernel_sock_shutdown(ud->tcp_socket, SHUT_RDWR);
#endif
	}

	if (sdev->wq)
		flush_workqueue(sdev->wq);

	/* 1. stop threads */
	usbip_stop_threads(ud);

	/* 2. close the socket */
	/*
	 * tcp_socket is freed after threads are killed.
	 * So usbip_xmit do not touch NULL socket.
	 */
	if (ud->tcp_socket) {
		//sock_release(ud->tcp_socket);
		sockfd_put(ud->tcp_socket);
		ud->tcp_socket = NULL;
	}
#if 0
	stub_iso_out_shutdown(sdev);
	stub_iso_in_shutdown(sdev);

	if (sdev->wq) {
		destroy_workqueue(sdev->wq);
		sdev->wq = NULL;
	}
	/* 3. free used data */
	stub_device_cleanup_urbs(sdev);

	/* 4. free stub_unlink */
	{
		unsigned long flags;
		struct stub_unlink *unlink, *tmp;

		spin_lock_irqsave(&sdev->priv_lock, flags);

		list_for_each_entry_safe(unlink, tmp, &sdev->unlink_tx, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		list_for_each_entry_safe(unlink, tmp, &sdev->unlink_free, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		spin_unlock_irqrestore(&sdev->priv_lock, flags);
	}
#endif

	/* 
	** In the case where TCP error, stub_device_reset() will be called by event_handler.
	** The solution is:
	** In host side, the device will be removed.
	** In client side, we should notify userland program and re-do the attach again.
	**
	** In this case, the stub driver won't be removed and there is no hotplug event fired.
	** Try to do "./bind_driver --export-to xxx" to export the device again.
	**
	** The shutdown handler is always the first handler called, so this is the ONLY place
	** we know what the event is in stub_dev.c. 
	** We don't notify_user for the case of "remove" and "unusable".
	*/
	if (!(ud->event & (USBIP_EH_BYE | USBIP_EH_UNUSABLE))) {
		notify_user(sdev->interface->dev.parent->bus_id);
	}
}



static void _stub_shutdown_connection(struct usbip_device *ud)
{
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);

	stub_iso_out_shutdown(sdev);
	stub_iso_in_shutdown(sdev);

	if (sdev->wq) {
		destroy_workqueue(sdev->wq);
		sdev->wq = NULL;
	}
	/* 3. free used data */
	stub_device_cleanup_urbs(sdev);

	/* 4. free stub_unlink */
	{
		unsigned long flags;
		struct stub_unlink *unlink, *tmp;

		spin_lock_irqsave(&sdev->priv_lock, flags);

		list_for_each_entry_safe(unlink, tmp, &sdev->unlink_tx, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		list_for_each_entry_safe(unlink, tmp, &sdev->unlink_free, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		spin_unlock_irqrestore(&sdev->priv_lock, flags);
	}

}

static void stub_device_reset(struct usbip_device *ud)
{
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);
	struct usb_device *udev = interface_to_usbdev(sdev->interface);
	int ret;

	udbg("device reset\n");
	ret = usb_lock_device_for_reset(udev, sdev->interface);
	if (ret < 0) {
		uerr("lock for reset\n");

		spin_lock(&ud->lock);
		ud->status = SDEV_ST_ERROR;
		spin_unlock(&ud->lock);

		return;
	}

	/* try to reset the device */
	ret = usb_reset_device(udev);

	usb_unlock_device(udev);

	spin_lock(&ud->lock);
	if (ret) {
		uerr("device reset\n");
		ud->status = SDEV_ST_ERROR;

	} else {
		uinfo("device reset success\n");
		ud->status = SDEV_ST_AVAILABLE;

	}
	spin_unlock(&ud->lock);

	return;
}

static void stub_device_unusable(struct usbip_device *ud)
{
	spin_lock(&ud->lock);
	ud->status = SDEV_ST_ERROR;
	spin_unlock(&ud->lock);
}


/*-------------------------------------------------------------------------*/

/**
 * stub_device_alloc - allocate a new stub_device struct
 * @interface: usb_interface of a new device
 *
 * Allocates and initializes a new stub_device struct.
 */
static struct stub_device * stub_device_alloc(struct usb_interface *interface)
{
	struct stub_device *sdev;
	int busnum = interface_to_busnum(interface);
	int devnum = interface_to_devnum(interface);
	int i;
	struct priv_ep_info *pei;

	udbg("allocating stub device\n");
	/* yes, it's a new device */
	sdev = (struct stub_device *) kzalloc(sizeof(struct stub_device), GFP_KERNEL);
	if (!sdev) {
		uerr("no memory for stub_device\n");
		return NULL;
	}

	sdev->interface = interface;

	/*
	 * devid is defined with devnum when this driver is first allocated.
	 * devnum may change later if a device is reset. However, devid never
	 * changes during a usbip connection.
	 */
	sdev->devid     = (busnum << 16) | devnum;

	usbip_task_init(&sdev->ud.tcp_rx, "stub_rx", stub_rx_loop);
	usbip_task_init(&sdev->ud.tcp_tx, "stub_tx", stub_tx_loop);

	sdev->ud.side = USBIP_STUB;
	sdev->ud.status = SDEV_ST_AVAILABLE;
	// sdev->ud.lock = SPIN_LOCK_UNLOCKED;
	spin_lock_init(&sdev->ud.lock);
	sdev->ud.tcp_socket = NULL;

	INIT_LIST_HEAD(&sdev->priv_init);
	INIT_LIST_HEAD(&sdev->priv_tx);
	INIT_LIST_HEAD(&sdev->priv_free);
	INIT_LIST_HEAD(&sdev->unlink_free);
	INIT_LIST_HEAD(&sdev->unlink_tx);

	sdev->wq = create_singlethread_workqueue("sdev_wq");
	BUG_ON(sdev->wq == NULL);
	for (i = 0; i < 16; i++) {
		pei = &(sdev->ep_out[i]);
		INIT_LIST_HEAD(&pei->priv_submitted);
		INIT_LIST_HEAD(&pei->urb_completed);
		INIT_LIST_HEAD(&pei->priv_wait);
		atomic_set(&pei->waiting, 0);
		atomic_set(&pei->submitted, 0);
		atomic_set(&pei->urb_submitted, 0);
		INIT_WORK(&pei->work, NULL, NULL);
		pei->sdev = sdev;
		pei->intf = -1;

		pei = &(sdev->ep_in[i]);
		INIT_LIST_HEAD(&pei->priv_submitted);
		INIT_LIST_HEAD(&pei->urb_completed);
		INIT_LIST_HEAD(&pei->priv_wait);
		atomic_set(&pei->waiting, 0);
		atomic_set(&pei->submitted, 0);
		atomic_set(&pei->urb_submitted, 0);
		INIT_WORK(&pei->work, NULL, NULL);
		pei->sdev = sdev;
		pei->intf = -1;
	}
    
	// sdev->priv_lock = SPIN_LOCK_UNLOCKED;
	spin_lock_init(&sdev->priv_lock);

	init_waitqueue_head(&sdev->tx_waitq);

	sdev->ud.eh_ops.shutdown = stub_shutdown_connection;
	sdev->ud.eh_ops.reset    = stub_device_reset;
	sdev->ud.eh_ops.unusable = stub_device_unusable;

	usbip_start_eh(&sdev->ud);

	udbg("register new interface\n");
	return sdev;
}

static int stub_device_free(struct stub_device *sdev)
{
	if (!sdev)
		return -EINVAL;

	kfree(sdev);
	udbg("kfree udev ok\n");

	return 0;
}


/*-------------------------------------------------------------------------*/

/*
 * If a usb device has multiple active interfaces, this driver is bound to all
 * the active interfaces. However, usbip exports *a* usb device (i.e., not *an*
 * active interface). Currently, a userland program must ensure that it
 * looks at the usbip's sysfs entries of only the first active interface.
 *
 * TODO: use "struct usb_device_driver" to bind a usb device.
 * However, it seems it is not fully supported in mainline kernel yet (2.6.19.2).
 */
static int stub_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct stub_device *sdev = NULL;
	char *udev_busid = interface->dev.parent->bus_id;
	int err = 0;

	udbg("Enter\n");

	/* check we should claim or not by busid_table */
	if (match_busid(udev_busid)) {
		uinfo("this device %s is not in match_busid table. skip!\n", udev_busid);

		/*
		 * Return value should be ENODEV or ENOXIO to continue trying
		 * other matched drivers by the driver core.
		 * See driver_probe_device() in driver/base/dd.c
		 */
		return -ENODEV;
	}

	if (udev->descriptor.bDeviceClass ==  USB_CLASS_HUB) {
		udbg("this device %s is a usb hub device. skip!\n", udev_busid);
		return -ENODEV;
	}

	if (!strcmp(udev->bus->bus_name, "vhci_hcd")) {
		udbg("this device %s is attached on vhci_hcd. skip!\n", udev_busid);
		return -ENODEV;
	}

	/* ok. this is my device. */

	if ((sdev = stub_device_alloc(interface))) {
		//struct usb_device *udev = interface_to_usbdev(interface);

		uinfo("USB/IP Stub: register a new interface (bus %u dev %u ifn %u)\n",
				udev->bus->busnum, udev->devnum,
				interface->cur_altsetting->desc.bInterfaceNumber);
	} else
		return -ENOMEM;

	/* set private data to usb_interface */
	usb_set_intfdata(interface, sdev);

	err = stub_add_files(&interface->dev);
	if (err) {
		uerr("create sysfs files for %s\n", udev_busid);
		return err;
	}

	return 0;
}


/*
 * called in usb_disconnect() or usb_deregister()
 * but only if actconfig(active configuration) exists
 */
static void stub_disconnect(struct usb_interface *interface)
{
	struct stub_device *sdev = usb_get_intfdata(interface);
	char *udev_busid = interface->dev.parent->bus_id;

	udbg("Enter\n");

	/* get stub_device */
	if (!sdev) {
		err(" could not get device from inteface data");
		//BUG();
		return;
	}

	/* Bruce080530
	** Del the busid from busid table, 
	** so that usbip.ko won't auto binded when next device pluged-in
	** If the busid doesn't exists, it means that it already deleted by "store_match_busid()".
	*/
	if (del_match_busid(udev_busid) == -1)
		uerr("can't find matched busid to delete!?\n");

	/* Let hotplug framework handle this. */
	//notify_user(udev_busid);

	usb_set_intfdata(interface, NULL);


	/*
	 * NOTE:
	 * rx/tx threads are invoked for each usb_device.
	 */
	stub_remove_files(&interface->dev);

	/* 1. shutdown the current connection */
	usbip_event_add(&sdev->ud, SDEV_EVENT_REMOVED);

	/* 2. wait for the stop of the event handler */
	usbip_stop_eh(&sdev->ud);

	/* Bruce. I found that usb_kill_urb() will sometimes deadlock the whole system.
	** (To reproduce this problem, move mouse while unexporting it.)
	** To avoid this situation, all submitted urbs MUST completed before
	** stub_device_cleanup_urbs(). (Which means calling usb_kill_urb() is pointless)
	** 
	** So, I move the whole clean up routine to "stub_disconnect()" from
	** "stub_shutdown_connection()".
	** By doing this way, all tx/rx thread will be stopped in "stub_shutdown_connection()",
	** and ALL urbs will complete in the following "stub_reset()" event.
	*/
	_stub_shutdown_connection(&sdev->ud);

	/* 3. free sdev */
	stub_device_free(sdev);


	udbg("bye\n");
}
