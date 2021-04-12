/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#include <linux/module.h>
#include <linux/device.h> //struct device_attribute
#include <linux/sysfs.h>
#include <linux/fs.h> //for inode
#include <linux/cdev.h>

#include <net/sock.h> 
#include <linux/socket.h>
#include <linux/tcp.h> //socketfd_lookup()
#include <linux/file.h> //For sockfd_put()
#include <linux/usb.h> //For USB_DIR_OUT
#include <linux/random.h>

#define KMOIP_RDESC_DEFINE
#include <asm/arch/drivers/kmoip_report_desc.h>
#undef KMOIP_RDESC_DEFINE

#include "hid.h"
#include "kmoip.h"
#include "kmoip-roaming.h"

#define IS_MT_APPLICATION(usage) (((usage) == 0x000D0004)  /* Digitizer:TouchScreen */ \
                              || (((usage) >= 0x000D0001) && ((usage) <= 0x000D0006)))  /* Digitizer:01~06 */
#define IS_MS_APPLICATION(usage) ((usage) == 0x00010002) /* GenericDesktop:Mouse */ \
                              || ((usage) == 0x00010001) /* GenericDesktop:Pointer */
#define IS_KB_APPLICATION(usage) ((usage) == 0x00010006) /* GenericDesktop:Keyboard */ \
                              || ((usage) == 0x00010007) /* GenericDesktop:Keypad */ \
                              || ((usage) == 0x000C0001) /* Consumer:ConsumerControl */ \
                              || ((usage) == 0x00010080) /* GenericDesktop:SystemControl */

static const char driver_name[] = "kmoip";
static struct kmoip_drv *kmdrv = NULL;

static int kmoip_major = 255;
module_param(kmoip_major, int, 0);

/* Start of IOCTL defines */
typedef struct _IO_ACCESS_DATA {
	int upstreamfd;
	int downstreamfd;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

#define IOCTL_KMOIP_START_CLIENT  0x1115


/**
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__,  ##args);	\
	} while(0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "KMoIP: " fmt , ## args);	\
	} while(0)


static inline int __is_fifo_empty(struct list_head *fifo_head)
{
	return list_empty(fifo_head);
}

static inline void __enqueue_fifo(struct list_head *fifo_head, struct list_head *in_head)
{
	list_add_tail(in_head, fifo_head);
}

static inline struct list_head *__dequeue_fifo(struct list_head *fifo_head)
{
	struct list_head *out;

	if (list_empty(fifo_head)) {
		return NULL;
	}
	out = fifo_head->next;
	list_del_init(out);

	return out;
}

static inline void report_entry_init_list_head(struct kmoip_report *report)
{
	INIT_LIST_HEAD(&report->list);
}


void enqueue_report_pool_fifo(struct kmoip_drv *kmdrv, struct kmoip_report *report, int lock)
{
	unsigned long flags = 0;
	if (lock) spin_lock_irqsave(&kmdrv->lock, flags);

	//clean up
	report->seq_num = 0;
	report->context = NULL;
	report->bRequestType = 0;
	report->bRequest = 0;
	report->report_id = 0;
	report->report_type = 0;
	report->interface = 0;
	report->report_length = 0;
	__enqueue_fifo(&kmdrv->report_free_list, &report->list);

	if (lock) spin_unlock_irqrestore(&kmdrv->lock, flags);
}

struct kmoip_report *dequeue_report_pool_fifo(struct kmoip_drv *kmdrv, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags = 0;

	if (lock) spin_lock_irqsave(&kmdrv->lock, flags);
	list_head = __dequeue_fifo(&kmdrv->report_free_list);
	if (lock) spin_unlock_irqrestore(&kmdrv->lock, flags);

	if (!list_head)
		return NULL;

	return list_entry(list_head, struct kmoip_report, list);
}


void enqueue_report_fifo(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report, int lock)
{
	unsigned long flags = 0;

	//uerr("enqueue to target=%p\n", target);

	if (lock) spin_lock_irqsave(&kmdrv->lock, flags);

	__enqueue_fifo(&target->report_list, &report->list);
	target->report_q_size++;

	if (lock) spin_unlock_irqrestore(&kmdrv->lock, flags);
}

struct kmoip_report *dequeue_report_fifo(struct kmoip_drv *kmdrv, kmr_target_s *target, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags = 0;

	if (lock) spin_lock_irqsave(&kmdrv->lock, flags);

	list_head = __dequeue_fifo(&target->report_list);
	if (list_head) target->report_q_size--;

	if (lock) spin_unlock_irqrestore(&kmdrv->lock, flags);

	if (!list_head)
		return NULL;

	return list_entry(list_head, struct kmoip_report, list);
}

static unsigned which_kmoip_supported_device(struct hid_device *hid, struct usb_device *dev)
{
	int i;
	struct usb_host_config *actconfig;
	unsigned type = KDT_NOT_VALID;

	actconfig = dev->actconfig;

	//is pure HID device?
	for (i = 0; i < actconfig->desc.bNumInterfaces; i++) {
		struct usb_interface_descriptor *desc;
		struct usb_interface *interface;

		//intfc = config->intf_cache[i];
		interface = actconfig->interface[i];
		desc = &interface->cur_altsetting->desc;

		if (USB_CLASS_HID != desc->bInterfaceClass) {
			goto is_not;
		}
	}

	/* We need to look into report descriptor. */
	for (i = 0; i < hid->maxcollection; i++) {
		if (hid->collection[i].type == HID_COLLECTION_APPLICATION ||
			hid->collection[i].type == HID_COLLECTION_PHYSICAL) {
			if (IS_MT_APPLICATION(hid->collection[i].usage))
				type |= KDT_MT;
			if (IS_MS_APPLICATION(hid->collection[i].usage))
				type |= KDT_MS;
			if (IS_KB_APPLICATION(hid->collection[i].usage))
				type |= KDT_KB;
		}
	}

	/*
	 * A typical Win7 compatible MT device contains both MT and Mouse application collection.
	 * So, has_mt has higher priority.
	 * If a device report has both KB and MS, then it is a keyboard based device.
	*/
	if (type != KDT_NOT_VALID)
		return type;
	/* Else, is not a KMoIP supported device. */
is_not:
	uinfo("VID:%04X PID:%04X is not a KMoIP supported HID device.\n", 
		le16_to_cpu(dev->descriptor.idVendor),
		le16_to_cpu(dev->descriptor.idProduct)
		);
	return KDT_NOT_VALID;
}

int kmoip_connect(struct hid_device *hid)
{
	struct ast_hiddev *hiddev, *t;
	unsigned kmoip_type;

	kmoip_type = which_kmoip_supported_device(hid, hid->dev);
	if (kmoip_type == KDT_NOT_VALID)
		return 0;

	if (list_empty(&kmdrv->ast_hiddev_free_list)) {
		uerr("run out off ast_hiddev_free_list\n");
		BUG();
	}

	{ //double check
		list_for_each_entry_safe(hiddev, t, &kmdrv->ast_hiddev_list, list) {
			if (hiddev->hid == hid) {
				uerr("duplicated saved hid?! (%p)\n", hid);
				BUG();
			}
		}
	}

	hiddev = list_entry(kmdrv->ast_hiddev_free_list.next, struct ast_hiddev, list);

	hiddev->hid = hid;
	hiddev->exist = 1;
	hiddev->kmoip_dev_type = kmoip_type;

	list_move_tail(&hiddev->list, &kmdrv->ast_hiddev_list);
#if 0
	{ //double check
		list_for_each_entry_safe(hiddev, t, &kmdrv->ast_hiddev_list, list) {
			uinfo("hiddev_list:0x%p\n", hiddev->hid);
		}
	}
#endif
	hid->kmdrv = kmdrv;

	/* Reset last_kbd0_out_report so that new keyboard can sync LED status with host. */
	memset(&kmdrv->last_kbd0_out_report, 0, sizeof(struct kmoip_report));

	/*
	 * __is_duplicate() in downstream_work() is to check last_kbd0_out_report of current target
	 * for sync LED status, we have to reset it to trigger output report
	 */
	if (1) {
		roaming_s *roaming = &kmdrv->roaming;
		kmr_target_s *target;

		target = kmr_get_cur_target(roaming);
		memset(&target->last_kbd0_out_report, 0, sizeof(struct kmoip_report));
	}

	hid_open(hid);
	return 0;
}

void kmoip_disconnect(struct hid_device *hid)
{
	struct ast_hiddev *hiddev, *t;

	list_for_each_entry_safe(hiddev, t, &kmdrv->ast_hiddev_list, list) {
		if (hiddev->hid == hid) {
			hid_close(hid);
			hiddev->hid = NULL;
			hiddev->exist = 0;
			list_move_tail(&hiddev->list, &kmdrv->ast_hiddev_free_list);
			break;
		}
	}

	{ //double check
		list_for_each_entry_safe(hiddev, t, &kmdrv->ast_hiddev_list, list) {
			//uinfo("hiddev_list:0x%p\n", hiddev->hid);
			if (hiddev->hid == hid) {
				uerr("duplicated hid?! (%p)\n", hid);
				BUG();
			}
		}
	}

	hid->kmdrv = NULL;
}

static void kmoip_detach_socket(struct kmoip_drv *kmdrv, kmr_target_s *target)
{
	struct socket *usocket_to_kill, *dsocket_to_kill;

	/* Don't delete local target. */
	if (unlikely(target->work_state == KMRWS_LOCAL))
		return;

	/* TODO. Move target destruction code into kmoip-roaming.c */
	target->work_state = KMRWS_SCKT_DEAD;
	usocket_to_kill = target->upstream_socket;
	dsocket_to_kill = target->downstream_socket;

	if (dsocket_to_kill) {
		//shutdown sockets
		dsocket_to_kill->ops->shutdown(dsocket_to_kill, 2);
		flush_workqueue(target->dwq);
		//free sockets
		sockfd_put(dsocket_to_kill);
		destroy_workqueue(target->dwq);
	}
	if (usocket_to_kill) {
		//shutdown sockets
		cancel_delayed_work(&target->heartbeat_work);
		usocket_to_kill->ops->shutdown(usocket_to_kill, 2);
		flush_workqueue(kmdrv->uwq);
		//free sockets
		sockfd_put(usocket_to_kill);
	}
}

static void kmoip_detach_socket_ex(struct kmoip_drv *kmdrv, char *mac, int idx_x, int idx_y)
{
	roaming_s *roaming = &kmdrv->roaming;
	kmr_target_s *target, *tmp;

	if (strncmp(mac, "FFFFFFFFFFFF", 12)) {
		/* This code path is currently not used. */
		target = kmr_find_target(roaming, mac, idx_x, idx_y);
		if (!target)
			return;

		kmr_detach_target(roaming, target);
		kmoip_detach_socket(kmdrv, target);
		kmr_free_target(roaming, target);
		return;
	}

	/* "FFFFFFFFFFFF" means caller want to detach all targets including slaves and master. */
	kmdrv->is_socket_ready = 0;

	kmr_for_each_target_safe(target, tmp, roaming) {
		/* detach all slaves first */
		if (kmr_target_is_master(target))
			continue;

		kmr_detach_target(roaming, target);
		kmoip_detach_socket(kmdrv, target);
		kmr_free_target(roaming, target);
	}
	kmr_for_each_target_safe(target, tmp, roaming) {
		/* detach master */
		kmr_detach_target(roaming, target);
		kmoip_detach_socket(kmdrv, target);
		kmr_free_target(roaming, target);
	}
}

static void kmoip_attach_socket(
        struct kmoip_drv *kmdrv,
        struct socket *usocket,
        struct socket *dsocket,
        kmr_target_s *target)
{
	target->downstream_socket = dsocket;
	target->upstream_socket = usocket;
	target->host_id = 0;
	target->work_state = KMRWS_SCKT_READY;
	/* Reset last_kbd0_out_report so that new keyboard can sync LED status with host. */
	memset(&target->last_kbd0_out_report, 0, sizeof(struct kmoip_report));
}

static void kmoip_attach_socket_ex(
		struct kmoip_drv *kmdrv,
		struct socket *usocket,
		struct socket *dsocket,
		char *mac,
		int idx_x,
		int idx_y,
		unsigned kmr_option)
{
	roaming_s *roaming = &kmdrv->roaming;
	kmr_target_s *target, *old_target;

	old_target = kmr_find_target(roaming, mac, idx_x, idx_y);
	target = kmr_alloc_target(roaming, mac, idx_x, idx_y);
	kmoip_attach_socket(kmdrv, usocket, dsocket, target);

	kmr_reattach_target(roaming, target, old_target);

	/* (0,0) is always the roaming master. Or the only target when roaming is disabled. */
	if (kmr_target_is_master(target)) {
		kmdrv->is_socket_ready = 1;
		kmdrv->heartbeat_interval = msecs_to_jiffies(500);
	}

	//start work queue
	queue_work(target->dwq, &target->dwork);
	queue_delayed_work(kmdrv->uwq, &target->heartbeat_work, kmdrv->heartbeat_interval);

	/* free old target is not timing critical. So, do it in last stage. */
	if (old_target) {
		kmoip_detach_socket(kmdrv, old_target);
		kmr_free_target(roaming, old_target);
	}
}

static ssize_t show_debug(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "Hello World\n");

	return strlen(buf);
}
static ssize_t store_debug(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	uinfo("Hello store debug\n");
	
	return count;
}
static DEVICE_ATTR(debug, (S_IRUGO | S_IWUSR), show_debug, store_debug);

static ssize_t show_attach(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	kmr_target_s *target, *tmp, *active_target;
	roaming_s *roaming = &kmdrv->roaming;

	active_target = kmr_get_cur_target(roaming);
	kmr_for_each_target_safe(target, tmp, roaming) {
		/* Append a '*' at the tail of current active target. */
		if (unlikely(target == active_target))
			num += snprintf(buf + num, PAGE_SIZE - num, "[%d,%d] %s *\n", target->idx_x, target->idx_y, target->mac);
		else
			num += snprintf(buf + num, PAGE_SIZE - num, "[%d,%d] %s\n", target->idx_x, target->idx_y, target->mac);
	}

	return num;
}

static ssize_t store_attach(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct kmoip_drv *kmdrv;
	unsigned int ufd, dfd;
	int only_detach = 0;
	struct socket *usocket = NULL, *dsocket = NULL;
	char mac[13];
	int idx_x, idx_y;

	kmdrv = (struct kmoip_drv *)dev->driver_data;
	BUG_ON(!kmdrv);

	/*
	* @ufd: upstream port socket fd.
	* @dfd: downstream port socket fd.
	 */
	sscanf(buf, "%s %i %i %u %u", mac, &idx_x, &idx_y, &ufd, &dfd);

	//printk("ufd(%d), dfd(%d)\n", ufd, dfd);
	if ((ufd == 0) && (dfd == 0)) {
		only_detach = 1;
	} else {
		usocket = sockfd_lookup(ufd, NULL);
		if (!usocket)
			return  -EINVAL;

		dsocket = sockfd_lookup(dfd, NULL);
		if (!dsocket)
			return  -EINVAL; //BruceToDo. Fix memory leak. fdput(ufd)
	}

	if (only_detach) {
		kmoip_detach_socket_ex(kmdrv, mac, idx_x, idx_y);
	} else {
		/* re-attach or attach */
		kmoip_attach_socket_ex(kmdrv, usocket, dsocket, mac, idx_x, idx_y, KMR_ENABLE);
	}

	return count;
}
static DEVICE_ATTR(attach, (S_IRUGO | S_IWUSR), show_attach, store_attach);

static ssize_t show_keycode(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kmoip_drv *kmdrv;

	kmdrv = (struct kmoip_drv *)dev->driver_data;

	sprintf(buf, "%u\n", kmdrv->show_keycode);

	return strlen(buf);
}
static ssize_t store_keycode(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct kmoip_drv *kmdrv;
	unsigned int show_keycode;

	kmdrv = (struct kmoip_drv *)dev->driver_data;
	BUG_ON(!kmdrv);

	sscanf(buf, "%u", &show_keycode);
	kmdrv->show_keycode = show_keycode;
	
	return count;
}
static DEVICE_ATTR(keycode, (S_IRUGO | S_IWUSR), show_keycode, store_keycode);

static ssize_t show_hotkey(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kmoip_drv *kmdrv;

	kmdrv = (struct kmoip_drv *)dev->driver_data;

	sprintf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X \n", 
		(unsigned int)kmdrv->hotkey[0][0], (unsigned int)kmdrv->hotkey[0][1], 
		(unsigned int)kmdrv->hotkey[1][0], (unsigned int)kmdrv->hotkey[1][1], 
		(unsigned int)kmdrv->hotkey[2][0], (unsigned int)kmdrv->hotkey[2][1], 
		(unsigned int)kmdrv->hotkey[3][0], (unsigned int)kmdrv->hotkey[3][1], 
		(unsigned int)kmdrv->hotkey[4][0], (unsigned int)kmdrv->hotkey[4][1], 
		(unsigned int)kmdrv->hotkey[5][0], (unsigned int)kmdrv->hotkey[5][1], 
		(unsigned int)kmdrv->hotkey[6][0], (unsigned int)kmdrv->hotkey[6][1], 
		(unsigned int)kmdrv->hotkey[7][0], (unsigned int)kmdrv->hotkey[7][1]
		);

	return strlen(buf);
}
static ssize_t store_hotkey(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct kmoip_drv *kmdrv;
	unsigned int hotkey[KMOIP_MAX_HOTKEY_SLOT][2];

	kmdrv = (struct kmoip_drv *)dev->driver_data;
	BUG_ON(!kmdrv);

	memset(hotkey, 0, sizeof(hotkey));

	sscanf(buf, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
		(unsigned int *)&hotkey[0][0], (unsigned int *)&hotkey[0][1], 
		(unsigned int *)&hotkey[1][0], (unsigned int *)&hotkey[1][1], 
		(unsigned int *)&hotkey[2][0], (unsigned int *)&hotkey[2][1], 
		(unsigned int *)&hotkey[3][0], (unsigned int *)&hotkey[3][1], 
		(unsigned int *)&hotkey[4][0], (unsigned int *)&hotkey[4][1], 
		(unsigned int *)&hotkey[5][0], (unsigned int *)&hotkey[5][1], 
		(unsigned int *)&hotkey[6][0], (unsigned int *)&hotkey[6][1], 
		(unsigned int *)&hotkey[7][0], (unsigned int *)&hotkey[7][1]
		);

	{
		int i;
		for (i = 0; i < KMOIP_MAX_HOTKEY_SLOT; i++) {
			kmdrv->hotkey[i][0] = (unsigned char)hotkey[i][0];
			kmdrv->hotkey[i][1] = (unsigned char)hotkey[i][1];
		}
	}
	
	return count;
}
static DEVICE_ATTR(hotkey, (S_IRUGO | S_IWUSR), show_hotkey, store_hotkey);

static ssize_t show_token_interval(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kmoip_drv *kmdrv;

	kmdrv = (struct kmoip_drv *)dev->driver_data;
	sprintf(buf, "%d\n", jiffies_to_msecs(kmdrv->token_interval));

	return strlen(buf);
}
static ssize_t store_token_interval(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct kmoip_drv *kmdrv;
	unsigned long ms = 0;

	sscanf(buf, "%ld", &ms);

	kmdrv = (struct kmoip_drv *)dev->driver_data;
	kmdrv->token_interval = msecs_to_jiffies(ms);
	
	return count;
}
static DEVICE_ATTR(token_interval, (S_IRUGO | S_IWUSR), show_token_interval, store_token_interval);

static struct attribute *kmoip_attrs[] = {
	&dev_attr_debug.attr,
	&dev_attr_attach.attr,
	&dev_attr_keycode.attr,
	&dev_attr_hotkey.attr,
	&dev_attr_token_interval.attr,
	NULL,
};

struct attribute_group kmoip_attr_group = {
	.attrs = kmoip_attrs,
};

/*
** This ioctl code is no more used. Use sysfs instead.
** I don't delete the code just in case some day I will need it.
*/
static int kmoip_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
#if 0
	PIO_ACCESS_DATA io_data;
	int *err = 0;
	struct kmoip_drv *kmdrv;

	kmdrv = (struct kmoip_drv *)filp->private_data;
	io_data = (PIO_ACCESS_DATA)arg;

	switch(cmd) {
	case IOCTL_KMOIP_START_CLIENT: //  0x1115
		uinfo("start client: ufd:%d, dfd:%d\n", io_data->upstreamfd, io_data->downstreamfd);
		kmdrv->upstream_socket = sockfd_lookup(io_data->upstreamfd, err);
		kmdrv->downstream_socket = sockfd_lookup(io_data->downstreamfd, err);
		kmdrv->is_socket_ready = 1;
		//start work queue
		queue_work(kmdrv->dwq, &kmdrv->dwork);
		break;
	default:
		uerr("Unknown Ioctrl? (0x%08X)\n", cmd);
		break;
	}
#endif
	return 0;
}

static int
socket_xmit_iov_udp(
	int send,
	struct socket *sock,
	struct kvec *rq_iov,
	size_t rq_iovlen,
	int size,
	int msg_flags)
{
	int result;
	struct msghdr msg;
	unsigned int total = 0;

	//dbg_xmit("enter\n");

	if (!sock || !size) {
		uerr("socket_xmit_iov_udp: invalid arg, sock %p size %d\n",
				sock, size);
		return -1;
	}

	sock->sk->sk_allocation = GFP_NOIO;
	msg.msg_name	= NULL;
	msg.msg_namelen = 0;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_namelen    = 0;
	msg.msg_flags	   = msg_flags | MSG_NOSIGNAL;
	if (send){
		result = kernel_sendmsg(sock, &msg, rq_iov, rq_iovlen, size);
		if (result < 0 && result != -ECONNREFUSED){
			uerr("!!!kernel_sendmsg return %d, size(%d)!!!\n", result, size);
			goto err;
		}
	}
	else{
		result = kernel_recvmsg(sock, &msg, rq_iov, rq_iovlen, size, MSG_WAITALL);
		if (result <= 0){
			uerr("!!!kernel_recvmsg return %d!!!\n", result);
			goto err;
		}
	}
	
	size -= result;
	total += result;
	
	if (send && (size > 0))
		uerr("Can't finish send?! (%d)\n", size);

	return total;

err:
	//uerr("S{%d},T{%d}\n", result, total);
	return result;
}

static int net_report_upstream(struct socket *upstream_socket, struct kmoip_report *report)
{
	int tx_size;
	struct kvec iov[1];

	iov[0].iov_base = (char *)report;
	iov[0].iov_len	= sizeof(struct kmoip_report);

	tx_size = socket_xmit_iov_udp(1, upstream_socket, iov, 1, sizeof(struct kmoip_report), 0);
	if (tx_size < 0) {
		if (tx_size != -ECONNREFUSED) {
			/* Bruce110617. For bug#2011061700. Just ignore this packet should be OK.
			** From google.
			** A previous UDP packet to the same machine and port got an ICMP message,
			** probably port not available.  Linux treats some ICMP messages as
			** ECONNREFUSED for UDP.  Not all *nix do this, Alan Cox says that Linux
			** is conforming to the RFC (search linux-net and linux-kernel archives for
			** ECONNREFUSED).
			** 
			** Simple workaround.  Treat ECONNREFUSED as "try again", up to an
			** arbitrary number of attempts.
			** 
			** for (i = 0; i < 5; ++i) {
			**     if (sendto(...) >= 0 || errno != ECONNREFUSED)
			**         break;
			** }
			** 
			** Works on all systems.
			*/
			uerr("upstream socket failed!! (%i)\n", tx_size);
			return -1;
		}
	}
	return 0;
}

static void _handle_kbd0_report_out(struct kmoip_drv *kmdrv, struct kmoip_report *report)
{
	struct ast_hiddev *hiddev;

	if (report->report_length == 0) {
		report->report[0] = 0;
	}
	list_for_each_entry(hiddev, &kmdrv->ast_hiddev_list, list) {
		//uinfo("hiddev_list:0x%p\n", hiddev->hid);
		hidkm_input_event(hiddev->hid, report);
	}
}

static void handle_report_out(struct kmoip_drv *kmdrv, struct kmoip_report *report)
{
#if 0
	{
		int i;
		int max_size; //in bytes

		max_size = report->report_length;
		printk("%d report out dump = ", report->interface);
		for (i = 0; i < max_size; i++) {
			printk(" %02X", report->report[i]);
		}
		printk("\n");
	}
#endif
	if (((report->bRequestType) == (USB_DIR_OUT|USB_TYPE_CLASS|USB_RECIP_INTERFACE)) &&
		(report->bRequest == HID_REQ_SET_REPORT))
	{ //Set report
		if ((report->report_type == HID_REPORT_TYPE_OUTPUT) &&
			(report->interface == KBD0_INTF_NUM))
		{ //LED
			_handle_kbd0_report_out(kmdrv, report);
		}
	}
	//ignore non-supported reports. TBD.
}

static inline int __is_duplicate(kmr_target_s *target, struct kmoip_report *report)
{
	struct kmoip_report *last_report = &target->last_kbd0_out_report;
	int duplicate = 0;

	if ((report->seq_num == last_report->seq_num) &&
		(report->from == last_report->from))
	{
		//is duplicated
		duplicate = 1;
	}

	return duplicate;
}

static void downstream_work(kmr_target_s *target)
{
	struct kmoip_drv *kmdrv = target->context;
	struct kmoip_report *report = NULL;
	struct kvec iov[1];
	int rx_size;
	int report_size;

	report_size = sizeof(struct kmoip_report);

	/* We should always monitor target status no matter target is current active or not. */
	while (target->work_state == KMRWS_SCKT_READY)
	{
		report = dequeue_report_pool_fifo(kmdrv, 1);
		if (!report) {
			uerr("OOM\n");
			yield();
			continue;
		}

		iov[0].iov_base = (char *)report;
		iov[0].iov_len = report_size;

		rx_size = socket_xmit_iov_udp(0,
							target->downstream_socket, 
							iov, 
							1, 
							report_size, 
							0);

		if (rx_size <= 0){
			uerr("socket_xmit_iov_udp rx failed\n");
			goto discard;
		}
		
		if (rx_size < report_size) {
			uerr("wrong total size!(%d)\n", rx_size);
			goto discard;
		}

		if (report->magic_num != KMOIP_MAGIC_NUM) {
			uerr("Wrong Magic number?! (0x%08X vs 0x%08X)\n", report->magic_num, KMOIP_MAGIC_NUM);
			goto discard;
		}
		/* Accept and record this report. */
		target->host_id = report->from;
		/* Update only when different. */
		if (!__is_duplicate(target, report)) {
			memcpy(&target->last_kbd0_out_report, report, sizeof(struct kmoip_report));
			/*
			** A trick. To reuse report->list to deliver active x/y value to client.
			*/
			kmr_update_active_zone(target, (unsigned int)report->list.next, (unsigned int)report->list.prev);
			/* Update physical KB if currently active. */
			if (kmr_get_cur_target(&kmdrv->roaming) == target)
				handle_report_out(kmdrv, report);
		}
discard:
		/* report's list entry is corrupted by x/y trick. Need to re-init it before enqueue. */
		report_entry_init_list_head(report);
		enqueue_report_pool_fifo(kmdrv, report, 1);
	}
}


static void heartbeat_work(kmr_target_s *target)
{
	struct kmoip_report *report;
	struct kmoip_drv *kmdrv = target->context;

	if (target->work_state != KMRWS_SCKT_READY)
		return;

	report = dequeue_report_pool_fifo(kmdrv, 1);
	if (!report)
		goto refire;

	report->magic_num = KMOIP_MAGIC_NUM;
	report->from = kmdrv->id;
	report->to = target->host_id;
	report->seq_num = kmdrv->seq_num++;
	report->report_id = 0;
	report->report_type = VKM_REPORT_TYPE_HEARTBEAT;
	report->interface = 0;
	report->report_length = 0;
	enqueue_report_fifo(kmdrv, target, report, 1);
	kmoip_kick_upstream_handler(kmdrv, target);

refire:
	queue_delayed_work(kmdrv->uwq, &target->heartbeat_work, kmdrv->heartbeat_interval);
}

static void init_work(struct kmoip_drv *kmdrv)
{
	/*
	** This is an one time work, used to change the work queue's thread priority.
	*/
	set_user_nice(current, -18);
}

static void upstream_work(kmr_target_s *target)
{
	struct kmoip_report *report;
	struct kmoip_drv *kmdrv = target->context;

	for(;;)
	{
		report = dequeue_report_fifo(kmdrv, target, 1);
		if (!report)
			break;

		if (target->work_state == KMRWS_SCKT_READY)
			net_report_upstream(target->upstream_socket, report);
			//BruceToDo. Check send result. How to handle fail?

		if (report->report_type == HID_REPORT_TYPE_INPUT && report->interface == KBD0_INTF_NUM) {
			hotkey_filter(kmdrv, report);
			if (kmdrv->show_keycode) {
				printk("keycode: %02X %02X\n", report->report[0], report->report[2]);
			}
		}

		enqueue_report_pool_fifo(kmdrv, report, 1);
	}
}

static void kmoip_drv_exit(struct kmoip_drv *kmdrv)
{
	//TBD
}

static int kmoip_drv_init(struct kmoip_drv *kmdrv)
{
	unsigned int from;

	spin_lock_init(&kmdrv->lock);

	kmr_drv_init(&kmdrv->roaming);

	kmdrv->kbd0_hiddev = ast_create_hid_class_device(vkm_kbd_0_rdesc, sizeof(vkm_kbd_0_rdesc), KBD0_INTF_NUM);
	if (!kmdrv->kbd0_hiddev)
		return -ENODEV;

	kmdrv->kbd1_hiddev = ast_create_hid_class_device(vkm_kbd_1_rdesc, sizeof(vkm_kbd_1_rdesc), KBD1_INTF_NUM);
	if (!kmdrv->kbd1_hiddev)
		return -ENODEV;

	kmdrv->ms0_hiddev = ast_create_hid_class_device(vkm_ms_0_rdesc, sizeof(vkm_ms_0_rdesc), MS0_INTF_NUM);
	if (!kmdrv->ms0_hiddev)
		return -ENODEV;

	kmdrv->tch0_hiddev = ast_create_hid_class_device(vkm_tch_0_rdesc, sizeof(vkm_tch_0_rdesc), TCH0_INTF_NUM);
	if (!kmdrv->tch0_hiddev)
		return -ENODEV;

	get_random_bytes(&from, sizeof(unsigned int));
	kmdrv->id = from;
	kmdrv->seq_num = from;
	kmdrv->token_interval = msecs_to_jiffies(100);

	/* init ast_hiddev_free_list */
	{
		struct ast_hiddev *hiddevs;
		int i;
	
		INIT_LIST_HEAD(&kmdrv->ast_hiddev_list);
		INIT_LIST_HEAD(&kmdrv->ast_hiddev_free_list);
		hiddevs = kmdrv->hiddevs;
		for (i = 0; i < MAX_HIDDEV_NUM; i++) {
			INIT_LIST_HEAD(&(hiddevs[i].list));
			list_add_tail(&(hiddevs[i].list), &kmdrv->ast_hiddev_free_list);
		}
	}
	/* init upstream report work/workqueue */
	kmdrv->upstream_func = (work_handler)upstream_work;
	kmdrv->heartbeat_func = (work_handler)heartbeat_work;
	kmdrv->downstream_func = (work_handler)downstream_work;
	{
		struct kmoip_report *reports;
		int i;

		kmdrv->uwq = create_singlethread_workqueue("kmoip_upstream");
		INIT_WORK(&kmdrv->init_work, (void (*)(void *))init_work, kmdrv);
		queue_work(kmdrv->uwq, &kmdrv->init_work);

		INIT_LIST_HEAD(&kmdrv->report_free_list);
		reports = kmdrv->reports;
		for (i = 0; i < KMOIP_REPORT_LIST_SIZE; i++) {
			INIT_LIST_HEAD(&(reports[i].list));
			reports[i].magic_num = KMOIP_MAGIC_NUM; // KMIP
			reports[i].from = from;
			list_add_tail(&(reports[i].list), &kmdrv->report_free_list);			
		}
	}

	kmr_alloc_local_target(&kmdrv->roaming);
	return 0;
}



/*
 * Open the device; in fact, there's nothing to do here.
 */
static int kmoip_open (struct inode *inode, struct file *filp)
{
	//uinfo("@@@kmoip_open\n");
	filp->private_data = kmdrv;
	return 0;
}


/*
 * Closing is just as simpler.
 */
static int kmoip_release(struct inode *inode, struct file *filp)
{
	//uinfo("@@@kmoip_release\n");
	return 0;
}

static struct file_operations kmoip_fops = {
	.ioctl  =  kmoip_ioctl,
	.open    = kmoip_open,
	.release = kmoip_release,
};

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device kmoip_pdev = {
	/* should be the same name as driver_name */
	.name = (char *) driver_name,
	.id = -1,
	.dev = {
		.release = the_pdev_release,
	},
};

int __init kmoip_init(void)
{
	int result;
	dev_t	dev;
	struct cdev *cdev;
	
	kmdrv = kzalloc(sizeof(struct kmoip_drv), GFP_KERNEL);
	if (!kmdrv) {
		uerr("allocate kmdrv failed!!\n");
		BUG();
		return -ENOMEM;
	}

	/* Init and register cdev */
	if (kmoip_major){
		dev = MKDEV(kmoip_major, 0);
		result = register_chrdev_region(dev, 1, driver_name);
	}
	else {
		result = alloc_chrdev_region(&dev, 0, 1, driver_name);
		kmoip_major = MAJOR(dev);
	}
	if (result < 0) {
		uerr("unable to get major %d\n", kmoip_major);
		BUG();
		goto fail_out;
	}
	//uinfo("major # is %d\n", kmoip_major);

	cdev = &kmdrv->cdev;
	cdev_init(cdev, &kmoip_fops);
	cdev->owner = THIS_MODULE;
	result = cdev_add(cdev, dev, 1);
	if (result < 0){
		uerr("Error %d adding kmoip", result);
		goto fail_out0;
	}

	/* register device as platform device so that we can have a valid device struct.
	** And then use it to attach to sysfs.
	*/
	result = platform_device_register(&kmoip_pdev);
	if (result < 0)
	{
		uerr("platform_driver_register err\n");
		BUG();
		goto fail_out1;
	}

	/* kmdrv will be saved in kmoip_pdev->dev.driver_data.
	** Can be get by platform_get_drvdata(&kmoip_pdev).
	*/
	platform_set_drvdata(&kmoip_pdev, kmdrv);
	kmdrv->pdev = &kmoip_pdev;
	/* register sysfs */
	result = sysfs_create_group(&kmoip_pdev.dev.kobj, &kmoip_attr_group);
	if (result) {
		uerr("create sysfs failed\n");
		goto fail_out1;
	}

	result = kmoip_drv_init(kmdrv);
	if (result) {
		uerr("kmoip_drv_init failed (%i)\n", result);
		goto fail_out2;
	}
	return result;

fail_out2:
	sysfs_remove_group(&kmoip_pdev.dev.kobj, &kmoip_attr_group);
fail_out1:
	cdev_del(cdev);
fail_out0:
	unregister_chrdev_region(MKDEV(kmoip_major, 0), 1);
fail_out:
	if (kmdrv)
		kfree(kmdrv);

	return result;
}

void kmoip_exit(void)
{
	struct cdev *cdev;

	if (!kmdrv)
		return;
	
	cdev = &kmdrv->cdev;

	sysfs_remove_group(&kmoip_pdev.dev.kobj, &kmoip_attr_group);

	cdev_del(cdev);

	unregister_chrdev_region(MKDEV(kmoip_major, 0), 1);

	kmoip_drv_exit(kmdrv);

	kfree(kmdrv);
}

void kmoip_kick_upstream_handler(struct kmoip_drv *kmdrv, kmr_target_s *target)
{
	queue_work(kmdrv->uwq, &target->uwork);
}

EXPORT_SYMBOL(kmoip_init);
EXPORT_SYMBOL(kmoip_exit);
EXPORT_SYMBOL(kmoip_connect);
EXPORT_SYMBOL(kmoip_disconnect);
EXPORT_SYMBOL(kmoip_kick_upstream_handler);
EXPORT_SYMBOL(enqueue_report_pool_fifo);
EXPORT_SYMBOL(dequeue_report_pool_fifo);
EXPORT_SYMBOL(enqueue_report_fifo);
EXPORT_SYMBOL(dequeue_report_fifo);


