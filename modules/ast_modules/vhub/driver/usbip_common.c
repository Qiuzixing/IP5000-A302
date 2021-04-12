/*
 * $Id: usbip_common.c 76 2008-04-29 04:36:43Z hirofuchi $
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

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include "usbip_common.h"
#include <asm/arch/drivers/kmoip_common.h>

#define KMOIP_RDESC_DEFINE
#include <asm/arch/drivers/kmoip_report_desc.h>
#undef KMOIP_RDESC_DEFINE

/* version information */
#define DRIVER_VERSION "$Id: usbip_common.c 76 2008-04-29 04:36:43Z hirofuchi $"
#define DRIVER_AUTHOR "Takahiro Hirofuchi <hirofuchi _at_ users.sourceforge.net>"
#define DRIVER_DESC "usbip common driver"


#define VKM_VID 0x2245
#define VKM_PID 0x1501

static struct usb_device_descriptor vkm_dev_desc = {
	.bLength = 0x12,
	.bDescriptorType = 1, //Device
	.bcdUSB = __constant_cpu_to_le16(0x0110),
	.bDeviceClass = USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 8,
	.idVendor = __constant_cpu_to_le16(VKM_VID),
	.idProduct = __constant_cpu_to_le16(VKM_PID),
	.bcdDevice = 0,
	.iManufacturer = 0, //string desc ptr
	.iProduct = 0, //string desc ptr
	.iSerialNumber = 0, //string desc ptr
	.bNumConfigurations = 1
};

static struct my_usb_config_descriptor vkm_cfg_desc = {
	.conf_desc_low_spd = {
		.bLength = 0x9,
		.bDescriptorType = 2, //config desc
		.wTotalLength = __constant_cpu_to_le16(sizeof(struct my_usb_config_descriptor)),
		.bNumInterfaces = MAX_INTF_NUM, //kbd0, ms0, kbd1, tch0
		.bConfigurationValue = 1,
		.iConfiguration = 0, //string desc ptr
		.bmAttributes = (USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_WAKEUP),
		.bMaxPower = 0x32 //100mA, This is a value get from river's sample code. TBD.
	},
	.kbd0_interface_desc = {
		.bLength = 0x9,
		.bDescriptorType = 0x4, //Interface type
		.bInterfaceNumber = KBD0_INTF_NUM,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_HID,
		.bInterfaceSubClass = 0x1, //It is a boot interface
		.bInterfaceProtocol = 0x1, //keyboard
		.iInterface = 0 //string desc ptr
	},
		.kbd0_hid_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x21, //HID
			.bcdHID = __constant_cpu_to_le16(0x0111),
			.bCountryCode = 0,
			.bNumDescriptors = 1,
			.desc[0].bDescriptorType = 0x22, //Report
			.desc[0].wDescriptorLength = 0 //sizeof(vkm_kbd_0_rdesc). will be init by kmoip_attach().
		},
		.kbd0_ep_desc_in = {
			.bLength = 0x7,
			.bDescriptorType = 5, //Endpoint
			.bEndpointAddress = (USB_DIR_IN | KBD0_EP_ADDR),
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = 0x8, // 8 bytes
			.bInterval = 0x0A // 35 ms --> 16ms
		},
	.kbd1_interface_desc = {
		.bLength = 0x9,
		.bDescriptorType = 0x4, //Interface type
		.bInterfaceNumber = KBD1_INTF_NUM,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_HID,
		.bInterfaceSubClass = 0x0, //non-boot dev
		.bInterfaceProtocol = 0x0,
		.iInterface = 0 //string desc ptr
	},
		.kbd1_hid_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x21, //HID
			.bcdHID = __constant_cpu_to_le16(0x0111),
			.bCountryCode = 0,
			.bNumDescriptors = 1,
			.desc[0].bDescriptorType = 0x22, //Report
			.desc[0].wDescriptorLength = 0 //sizeof(vkm_kbd_1_rdesc). will be init by kmoip_attach().
		},
		.kbd1_ep_desc_in = {
			.bLength = 0x7,
			.bDescriptorType = 5, //Endpoint
			.bEndpointAddress = (USB_DIR_IN | KBD1_EP_ADDR),
			.bmAttributes = USB_ENDPOINT_XFER_INT,
			.wMaxPacketSize = 0x8, // 8 bytes
			.bInterval = 0x0A // 35 ms --> 16ms
		},
	.ms0_interface_desc = {
		.bLength = 0x9,
		.bDescriptorType = 0x4, //Interface type
		.bInterfaceNumber = MS0_INTF_NUM,
		.bAlternateSetting = 0,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_HID,
#if KMOIP_2BYTES_MOUSE_CURSOR
		.bInterfaceSubClass = 0x0, //Generic HID
		.bInterfaceProtocol = 0x0, //None
#else
		.bInterfaceSubClass = 0x1, //It is a boot interface
		.bInterfaceProtocol = 0x2, //mouse
#endif
		.iInterface = 0 //string desc ptr
	},
		.ms0_hid_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x21, //HID
			.bcdHID = __constant_cpu_to_le16(0x0111),
			.bCountryCode = 0,
			.bNumDescriptors = 1,
			.desc[0].bDescriptorType = 0x22, //Report
			.desc[0].wDescriptorLength = 0 //sizeof(vkm_ms_0_rdesc). will be init by kmoip_attach().
		},
		.ms0_ep_desc_in = {
			.bLength = 0x7,
			.bDescriptorType = 5, //Endpoint
			.bEndpointAddress = (USB_DIR_IN | MS0_EP_ADDR),
			.bmAttributes = USB_ENDPOINT_XFER_INT,
#if KMOIP_2BYTES_MOUSE_CURSOR
			.wMaxPacketSize = 0x6, // 6 bytes
#else
			.wMaxPacketSize = 0x4, // 4 bytes
#endif
			.bInterval = 0x0A // 35 ms --> 16ms
		},
	/* Pointer type touch device. (absolute mouse) */
	.tch0_interface_desc = {
			.bLength = 0x9,
			.bDescriptorType = 0x4, //Interface type
			.bInterfaceNumber = TCH0_INTF_NUM,
			.bAlternateSetting = 0,
			.bNumEndpoints = 1,
			.bInterfaceClass = USB_CLASS_HID,
			.bInterfaceSubClass = 0x1, //It is a boot interface
			.bInterfaceProtocol = 0x2, // Mouse
			.iInterface = 0 //string desc ptr
		},
			.tch0_hid_desc = {
				.bLength = 0x9,
				.bDescriptorType = 0x21, //HID
				.bcdHID = __constant_cpu_to_le16(0x0111), /* HID ver 1.11 */
				.bCountryCode = 0,
				.bNumDescriptors = 1,
				.desc[0].bDescriptorType = 0x22, //Report
				.desc[0].wDescriptorLength = 0 //sizeof(vkm_tch_0_rdesc). will be init by kmoip_attach().
			},
			.tch0_ep_desc_in = {
				.bLength = 0x7,
				.bDescriptorType = 5, //Endpoint
				.bEndpointAddress = (USB_DIR_IN | TCH0_EP_ADDR),
				.bmAttributes = USB_ENDPOINT_XFER_INT,
				.wMaxPacketSize = 0x6, // 6 bytes
				.bInterval = 0x0A // 35 ms --> 16ms
			},
};



/*-------------------------------------------------------------------------*/
/* debug routines */

#ifdef CONFIG_USBIP_DEBUG
unsigned long usbip_debug_flag = 0xffffffff;
#else
unsigned long usbip_debug_flag = 0;
#endif
EXPORT_SYMBOL(usbip_debug_flag);


/* FIXME */
struct device_attribute dev_attr_usbip_debug;
EXPORT_SYMBOL(dev_attr_usbip_debug);


static ssize_t show_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%lx\n", usbip_debug_flag);
}

static ssize_t store_flag(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long flag;

	sscanf(buf, "%lx", &flag);
	usbip_debug_flag = flag;

	return count;
}
DEVICE_ATTR(usbip_debug, (S_IRUGO | S_IWUSR), show_flag, store_flag);

static void usbip_dump_buffer(char *buff, int bufflen)
{
	int i;

	if (bufflen > 128) {
		for (i = 0; i< 128; i++) {
			if (i%24 == 0)
				printk("   ");
			printk("%02x ", (unsigned char ) buff[i]);
			if (i%4 == 3) printk("| ");
			if (i%24 == 23) printk("\n");
		}
		printk("... (%d byte)\n", bufflen);
		return;
	}

	for (i = 0; i< bufflen; i++) {
		if (i%24 == 0)
			printk("   ");
		printk("%02x ", (unsigned char ) buff[i]);
		if (i%4 == 3)
			printk("| ");
		if (i%24 == 23)
			printk("\n");
	}
	printk("\n");

}

static void usbip_dump_pipe(unsigned int p)
{
	unsigned char type = usb_pipetype(p);
	unsigned char ep = usb_pipeendpoint(p);
	unsigned char dev = usb_pipedevice(p);
	unsigned char dir = usb_pipein(p);

	printk("dev(%d) ", dev);
	printk("ep(%d) ",  ep);
	printk("%s ", dir ? "IN" : "OUT");

	switch(type) {
		case PIPE_ISOCHRONOUS :
			printk("%s ", "ISO");
			break;
		case PIPE_INTERRUPT :
			printk("%s ", "INT");
			break;
		case PIPE_CONTROL :
			printk("%s ", "CTL");
			break;
		case PIPE_BULK :
			printk("%s ", "BLK");
			break;
		default :
			printk("ERR");
	}

	printk("\n");

}

static void usbip_dump_usb_device(struct usb_device *dev)
{
	int i;

	if (!dev) {
		printk("      dump usb dev: null pointer!!\n");
		return;
	}

	printk("       devnum(%d) devpath(%s)", dev->devnum, dev->devpath);

	switch(dev->speed) {
		case USB_SPEED_HIGH :
			printk(" SPD_HIGH");
			break;
		case USB_SPEED_FULL :
			printk(" SPD_FULL");
			break;
		case USB_SPEED_LOW :
			printk(" SPD_LOW");
			break;
		case USB_SPEED_UNKNOWN :
			printk(" SPD_UNKNOWN");
			break;
		default :
			printk(" SPD_ERROR");
	}

	printk(" tt %p, ttport %d", dev->tt, dev->ttport);
	printk("\n");

	printk("                    ");
	for (i = 0; i < 16; i++) {
		printk(" %2u", i);
	}

	printk("\n");

	printk("       toggle0(IN) :");
	for (i = 0; i< 16; i++) {
		printk(" %2u", ( dev->toggle[0] & (1 << i) ) ? 1 : 0);
	}

	printk("\n");

	printk("       toggle1(OUT):");
	for (i = 0; i< 16; i++) {
		printk(" %2u", ( dev->toggle[1] & (1 << i) ) ? 1 : 0);
	}

	printk("\n");


	printk("       epmaxp_in   :");
	for (i = 0; i < 16; i++) {
		if (dev->ep_in[i])
			printk(" %2u",  dev->ep_in[i]->desc.wMaxPacketSize);
	}

	printk("\n");

	printk("       epmaxp_out  :");
	for (i = 0; i < 16; i++) {
		if (dev->ep_out[i])
			printk(" %2u",  dev->ep_out[i]->desc.wMaxPacketSize);
	}

	printk("\n       ");

	printk("parent %p, bus %p", dev->parent, dev->bus);
	printk("\n       ");

	printk("descriptor %p, config %p, actconfig %p, rawdescriptors %p",
			&dev->descriptor, dev->config, dev->actconfig, dev->rawdescriptors);
	printk("\n       ");

	printk("have_langid %d, string_langid %d", dev->have_langid, dev->string_langid);
	printk("\n       ");

	printk("maxchild %d, children %p", dev->maxchild, dev->children);

	printk("\n");
}

static void usbip_dump_request_type(__u8 rt)
{
	switch(rt & USB_RECIP_MASK) {
		case USB_RECIP_DEVICE:
			printk("DEVICE");
			break;
		case USB_RECIP_INTERFACE:
			printk("INTERF");
			break;
		case USB_RECIP_ENDPOINT:
			printk("ENDPOI");
			break;
		case USB_RECIP_OTHER:
			printk("OTHER ");
			break;
		default:
			printk("------");
	}
}

void usbip_dump_usb_ctrlrequest(struct usb_ctrlrequest *cmd)
{
	if (!cmd) {
		printk("      %s : null pointer\n", __FUNCTION__);
		return;
	}

	printk("       ");
	printk("bRequestType(%02X) ", cmd->bRequestType);
	printk("bRequest(%02X) " , cmd->bRequest);
	printk("wValue(%04X) ", cmd->wValue);
	printk("wIndex(%04X) ", cmd->wIndex);
	printk("wLength(%04X) ", cmd->wLength);

	printk("\n       ");

	if ((cmd->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		printk("STANDARD ");
		switch(cmd->bRequest) {
			case USB_REQ_GET_STATUS:
				printk("GET_STATUS");
				break;
			case USB_REQ_CLEAR_FEATURE:
				printk("CLEAR_FEAT");
				break;
			case USB_REQ_SET_FEATURE:
				printk("SET_FEAT  ");
				break;
			case USB_REQ_SET_ADDRESS:
				printk("SET_ADDRRS");
				break;
			case USB_REQ_GET_DESCRIPTOR:
				printk("GET_DESCRI");
				break;
			case USB_REQ_SET_DESCRIPTOR:
				printk("SET_DESCRI");
				break;
			case USB_REQ_GET_CONFIGURATION:
				printk("GET_CONFIG");
				break;
			case USB_REQ_SET_CONFIGURATION:
				printk("SET_CONFIG");
				break;
			case USB_REQ_GET_INTERFACE:
				printk("GET_INTERF");
				break;
			case USB_REQ_SET_INTERFACE:
				printk("SET_INTERF");
				break;
			case USB_REQ_SYNCH_FRAME:
				printk("SYNC_FRAME");
				break;
			default:
				printk("REQ(%02X) ", cmd->bRequest);
		}

		printk(" ");
		usbip_dump_request_type(cmd->bRequestType);

	} else if ((cmd->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS)
		printk("CLASS   ");

	else if ((cmd->bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR)
		printk("VENDOR  ");

	else if ((cmd->bRequestType & USB_TYPE_MASK) == USB_TYPE_RESERVED)
		printk("RESERVED");

	printk("\n");
}
EXPORT_SYMBOL(usbip_dump_usb_ctrlrequest);

void usbip_dump_urb (struct urb *purb)
{
	if (!purb) {
		printk("   dump urb: null pointer!!\n");
		return;
	}

	printk("   urb                   :%p\n", purb);
	printk("   dev                   :%p\n", purb->dev);

	usbip_dump_usb_device(purb->dev);

	printk("   pipe                  :%08x ", purb->pipe);

	usbip_dump_pipe(purb->pipe);

	printk("   status                :%d\n", purb->status);
	printk("   transfer_flags        :%08X\n", purb->transfer_flags);
	printk("   transfer_buffer       :%p\n", purb->transfer_buffer);
	printk("   transfer_buffer_length:%d\n", purb->transfer_buffer_length);
	printk("   actual_length         :%d\n", purb->actual_length);
	printk("   setup_packet          :%p\n", purb->setup_packet);

	if (purb->setup_packet && usb_pipetype(purb->pipe) == PIPE_CONTROL)
		usbip_dump_usb_ctrlrequest((struct usb_ctrlrequest *) purb->setup_packet);

	printk("   start_frame           :%d\n", purb->start_frame);
	printk("   number_of_packets     :%d\n", purb->number_of_packets);
	printk("   interval              :%d\n", purb->interval);
	printk("   error_count           :%d\n", purb->error_count);
	printk("   context               :%p\n", purb->context);
	printk("   complete              :%p\n", purb->complete);
}
EXPORT_SYMBOL(usbip_dump_urb);

void usbip_dump_header(struct usbip_header *pdu)
{
	udbg("BASE: cmd %u seq %u devid x%x dir %u ep %u\n",
			pdu->base.command,
			pdu->base.seqnum,
			pdu->base.devid,
			pdu->base.direction,
			pdu->base.ep);

	switch(pdu->base.command) {
		case USBIP_CMD_SUBMIT:
			udbg("CMD_SUBMIT: x_flags %u x_len %u sf %u #p %u iv %u\n",
					pdu->u.cmd_submit.transfer_flags,
					pdu->u.cmd_submit.transfer_buffer_length,
					pdu->u.cmd_submit.start_frame,
					pdu->u.cmd_submit.number_of_packets,
					pdu->u.cmd_submit.interval);
					break;
		case USBIP_CMD_UNLINK:
			udbg("CMD_UNLINK: seq %u\n", pdu->u.cmd_unlink.seqnum);
			break;
		case USBIP_RET_SUBMIT:
			udbg("RET_SUBMIT: st %d al %u sf %d ec %d\n",
					pdu->u.ret_submit.status,
					pdu->u.ret_submit.actual_length,
					pdu->u.ret_submit.start_frame,
					pdu->u.ret_submit.error_count);
                    break;
		case USBIP_RET_UNLINK:
			udbg("RET_UNLINK: status %d\n", pdu->u.ret_unlink.status);
			break;
		default:
			/* NOT REACHED */
			udbg("UNKNOWN\n");
	}
}
EXPORT_SYMBOL(usbip_dump_header);

unsigned char *kmoip_get_vkm_cfg_desc(unsigned int *size)
{
	*size = le16_to_cpu(vkm_cfg_desc.conf_desc_low_spd.wTotalLength);
	return (unsigned char *)&vkm_cfg_desc;
}
EXPORT_SYMBOL(kmoip_get_vkm_cfg_desc);

unsigned char *kmoip_get_vkm_dev_desc(unsigned int *size)
{
	*size = vkm_dev_desc.bLength;
	return (unsigned char *)&vkm_dev_desc;
}
EXPORT_SYMBOL(kmoip_get_vkm_dev_desc);

unsigned char *kmoip_get_kbd0_rdesc(unsigned int *size)
{
	*size = sizeof(vkm_kbd_0_rdesc);
	return (unsigned char *)vkm_kbd_0_rdesc;
}
EXPORT_SYMBOL(kmoip_get_kbd0_rdesc);

unsigned char *kmoip_get_kbd1_rdesc(unsigned int *size)
{
	*size = sizeof(vkm_kbd_1_rdesc);
	return (unsigned char *)vkm_kbd_1_rdesc;
}
EXPORT_SYMBOL(kmoip_get_kbd1_rdesc);

unsigned char *kmoip_get_ms0_rdesc(unsigned int *size)
{
	*size = sizeof(vkm_ms_0_rdesc);
	return (unsigned char *)vkm_ms_0_rdesc;
}
EXPORT_SYMBOL(kmoip_get_ms0_rdesc);

unsigned char *kmoip_get_tch0_rdesc(unsigned int *size)
{
	*size = sizeof(vkm_tch_0_rdesc);
	return (unsigned char *)vkm_tch_0_rdesc;
}
EXPORT_SYMBOL(kmoip_get_tch0_rdesc);

/*-------------------------------------------------------------------------*/
/* thread routines */

int usbip_thread(void *param)
{
	struct usbip_task *ut = (struct usbip_task *) param;

	if (!ut)
		return -EINVAL;

	lock_kernel();
	daemonize(ut->name);
	allow_signal(SIGKILL);
	ut->thread = current;
	unlock_kernel();

	/* srv.rb must wait for rx_thread starting */
	complete(&ut->thread_done);

	/* start of while loop */
	ut->loop_ops(ut);

	/* end of loop */
	ut->thread = NULL;

	complete_and_exit(&ut->thread_done, 0);
}

void usbip_start_threads(struct usbip_device *ud)
{
	/*
	 * threads are invoked per one device (per one connection).
	 */
	kernel_thread((int(*)(void *))usbip_thread, (void *)&ud->tcp_rx, 0);
	kernel_thread((int(*)(void *))usbip_thread, (void *)&ud->tcp_tx, 0);

	/* confirm threads are starting */
	wait_for_completion(&ud->tcp_rx.thread_done);
	wait_for_completion(&ud->tcp_tx.thread_done);
}
EXPORT_SYMBOL(usbip_start_threads);

void usbip_stop_threads(struct usbip_device *ud)
{
	/* kill threads related to this sdev, if v.c. exists */
	if (ud->tcp_rx.thread != NULL) {
		send_sig(SIGKILL, ud->tcp_rx.thread, 1);
		wait_for_completion(&ud->tcp_rx.thread_done);
		udbg("rx_thread for ud %p has finished\n", ud);
	}
	/*
	** Bruce120119. init_completion for re-use. Fix Bug#2010102500.
	** Sometimes ud->tcp_rx.thread will be set to NULL before usbip_stop_threads()
	** been called. And so that above wait_for_completion() won't be called.
	** If we re-use this thread, and next time when we call usbip_stop_threads()
	** usbip_stop_threads() will return immediately because the wait_for_completion()
	** counter is not zero. This is how it hit Bug#2010102500.
	*/
	init_completion(&ud->tcp_rx.thread_done);

	if (ud->tcp_tx.thread != NULL) {
		send_sig(SIGKILL, ud->tcp_tx.thread, 1);
		wait_for_completion(&ud->tcp_tx.thread_done);
		udbg("tx_thread for ud %p has finished\n", ud);
	}
	init_completion(&ud->tcp_tx.thread_done);
}
EXPORT_SYMBOL(usbip_stop_threads);

void usbip_task_init(struct usbip_task *ut, char *name,
		void (*loop_ops)(struct usbip_task *))
{
	ut->thread = NULL;
	init_completion(&ut->thread_done);
	ut->name = name;
	ut->loop_ops = loop_ops;
}
EXPORT_SYMBOL(usbip_task_init);


/*-------------------------------------------------------------------------*/
/* socket routines */

 /*  Send/receive messages over TCP/IP. I refer drivers/block/nbd.c */
int usbip_xmit(int send, struct socket *sock, char *buf, int size, int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov;
	int total = 0;

	/* for blocks of if (dbg_flag_xmit) */
	char *bp = buf;
	int osize= size;

	dbg_xmit("enter\n");

	if (!sock || !buf || !size) {
		uerr("usbip_xmit: invalid arg, sock %p buff %p size %d\n",
				sock, buf, size);
		return -EINVAL;
	}


	if (dbg_flag_xmit) {
		if (send) {
			if (!in_interrupt())
				printk(KERN_DEBUG "%-10s:", current->comm);
			else
				printk(KERN_DEBUG "interupt  :");

			printk("usbip_xmit: sending... , sock %p, buf %p, size %d, msg_flags %d\n",
					sock, buf, size, msg_flags);
			usbip_dump_buffer(buf, size);
		}
	}


	do {
		sock->sk->sk_allocation = GFP_NOIO;
		iov.iov_base    = buf;
		iov.iov_len     = size;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

		if (send)
			result = kernel_sendmsg(sock, &msg, &iov, 1, size);
		else
			result = kernel_recvmsg(sock, &msg, &iov, 1, size, MSG_WAITALL);

		if (result <= 0) {
			/* Bruce. It is noisy.
			uerr("usbip_xmit: %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total); */
			goto err;
		}

		size -= result;
		buf += result;
		total += result;

	} while (size > 0);


	if (dbg_flag_xmit) {
		if (!send) {
			if (!in_interrupt())
				printk(KERN_DEBUG "%-10s:", current->comm);
			else
				printk(KERN_DEBUG "interupt  :");

			printk("usbip_xmit: receiving....\n");
			usbip_dump_buffer(bp, osize);
			printk("usbip_xmit: received, osize %d ret %d size %d total %d\n",
					osize, result, size, total);
		}

		if (send) {
			printk("usbip_xmit: send, total %d\n", total);
		}
	}

	return total;

err:
	return result;
}
EXPORT_SYMBOL(usbip_xmit);


/*
 * Adjust the kvec to move on 'n' bytes (from fs/smbfs/sock.c)
 */
static int
_move_iov(struct kvec **data, size_t *num, struct kvec *vec, unsigned amount)
{
	struct kvec *iv = *data;
	int i;
	int len;

	/*
	 *	Eat any sent kvecs
	 */
	while (iv->iov_len <= amount) {
		amount -= iv->iov_len;
		iv++;
		(*num)--;
	}

	/*
	 *	And chew down the partial one
	 */
	vec[0].iov_len = iv->iov_len-amount;
	vec[0].iov_base =((unsigned char *)iv->iov_base)+amount;
	iv++;

	len = vec[0].iov_len;

	/*
	 *	And copy any others
	 */
	for (i = 1; i < *num; i++) {
		vec[i] = *iv++;
		len += vec[i].iov_len;
	}

	//*data = vec;
	return len;
}



int usbip_xmit_ex
    (int send,
    struct socket *sock,
    struct kvec *rq_iov,
    size_t rq_iovlen,
    int size,
    int msg_flags)
{
	int result;
	struct msghdr msg;
	struct kvec iov[4];
	unsigned int total = 0;

	dbg_xmit("enter\n");

	if (!sock || !size) {
		uerr("usbip_xmit: invalid arg, sock %p size %d\n",
				sock, size);
		return -EINVAL;
	}

	if (dbg_flag_xmit) {
		if (send) {
			if (!in_interrupt())
				printk(KERN_DEBUG "%-10s:", current->comm);
			else
				printk(KERN_DEBUG "interupt  :");

			printk("usbip_xmit: sending... , sock %p, size %d, msg_flags %d\n",
					sock, size, msg_flags);
		}
	}


	do {

		sock->sk->sk_allocation = GFP_NOIO;
		msg.msg_name    = NULL;
		msg.msg_namelen = 0;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_namelen    = 0;
		msg.msg_flags      = msg_flags | MSG_NOSIGNAL;

		_move_iov(&rq_iov, &rq_iovlen, iov, total);

		if (send)
			result = kernel_sendmsg(sock, &msg, iov, rq_iovlen, size);
		else
			result = kernel_recvmsg(sock, &msg, iov, rq_iovlen, size, MSG_WAITALL);

		if (result <= 0) {
			/* Bruce. It is noisy.
			uerr("usbip_xmit: %s sock %p buf %p size %u ret %d total %d\n",
					send ? "send" : "receive", sock, buf, size, result, total); */
			goto err;
		}

		size -= result;
		total += result;

	} while (size > 0);

	return total;

err:
	printk("S{%d},T{%d}\n", result, total);
	return result;
}
EXPORT_SYMBOL(usbip_xmit_ex);


struct socket *sockfd_to_socket(unsigned int sockfd)
{
	struct socket *socket;
	struct file *file;
	struct inode *inode;

	file = fget(sockfd);
	if (!file) {
		uerr("invalid sockfd\n");
		return NULL;
	}

	inode = file->f_dentry->d_inode;

	if (!inode || !S_ISSOCK(inode->i_mode))
		return NULL;

	socket = SOCKET_I(inode);

	return socket;
}
EXPORT_SYMBOL(sockfd_to_socket);



/*-------------------------------------------------------------------------*/
/* pdu routines */

/* there may be more cases to tweak the flags. */
static unsigned int tweak_transfer_flags(unsigned int flags)
{

	if (flags & URB_NO_TRANSFER_DMA_MAP)
		/*
		 * vhci_hcd does not provide DMA-mapped I/O. The upper
		 * driver does not need to set this flag. The remote
		 * usbip.ko does not still perform DMA-mapped I/O for
		 * DMA-caplable host controllers. So, clear this flag.
		 */
		flags &= ~URB_NO_TRANSFER_DMA_MAP;

	if (flags & URB_NO_SETUP_DMA_MAP)
		flags &= ~URB_NO_SETUP_DMA_MAP;

	return flags;
}

static void usbip_pack_cmd_submit(struct usbip_header *pdu, struct urb *urb, int pack)
{
	struct usbip_header_cmd_submit *spdu = &pdu->u.cmd_submit;

	/*
	 * Some members are not still implemented in usbip. I hope this issue
	 * will be discussed when usbip is ported to other operating systems.
	 */
	if (pack) {
		/* vhci_tx.c */
		spdu->transfer_flags		= tweak_transfer_flags(urb->transfer_flags);

		spdu->transfer_buffer_length	= urb->transfer_buffer_length;
		spdu->start_frame		= urb->start_frame;
		spdu->number_of_packets		= urb->number_of_packets;
		spdu->interval			= urb->interval;
	} else  {
		/* stub_rx.c */
		urb->transfer_flags         = spdu->transfer_flags;

		urb->transfer_buffer_length = spdu->transfer_buffer_length;
		urb->start_frame            = spdu->start_frame;
		urb->number_of_packets      = spdu->number_of_packets;
		urb->interval               = spdu->interval;
	}
}

static void usbip_pack_ret_submit(struct usbip_header *pdu, struct urb *urb, int pack)
{
	struct usbip_header_ret_submit *rpdu = &pdu->u.ret_submit;

	if (pack) {
		/* stub_tx.c */

		rpdu->status		= urb->status;
		rpdu->actual_length	= urb->actual_length;
		rpdu->start_frame	= urb->start_frame;
		rpdu->error_count	= urb->error_count;
	} else {
		/* vhci_rx.c */

		urb->status		= rpdu->status;
		urb->actual_length	= rpdu->actual_length;
		urb->start_frame	= rpdu->start_frame;
		urb->error_count	= rpdu->error_count;
	}
}


void usbip_pack_pdu(struct usbip_header *pdu, struct urb *urb, int cmd, int pack)
{
	switch(cmd) {
		case USBIP_CMD_SUBMIT:
			usbip_pack_cmd_submit(pdu, urb, pack);
			break;
		case USBIP_RET_SUBMIT:
			usbip_pack_ret_submit(pdu, urb, pack);
			break;
		default:
			err("unknown command");
			/* NOTREACHED */
			//BUG();
	}
}
EXPORT_SYMBOL(usbip_pack_pdu);


static void correct_endian_basic(struct usbip_header_basic *base, int send)
{
	if (send) {
		base->command	= cpu_to_be32(base->command);
		base->seqnum	= cpu_to_be32(base->seqnum);
		base->devid	= cpu_to_be32(base->devid);
		base->direction	= cpu_to_be32(base->direction);
		base->ep	= cpu_to_be32(base->ep);
	} else {
		base->command	= be32_to_cpu(base->command);
		base->seqnum	= be32_to_cpu(base->seqnum);
		base->devid	= be32_to_cpu(base->devid);
		base->direction	= be32_to_cpu(base->direction);
		base->ep	= be32_to_cpu(base->ep);
	}
}

static void correct_endian_cmd_submit(struct usbip_header_cmd_submit *pdu, int send)
{
	if (send) {
		pdu->transfer_flags = cpu_to_be32(pdu->transfer_flags);

		cpu_to_be32s(&pdu->transfer_buffer_length);
		cpu_to_be32s(&pdu->start_frame);
		cpu_to_be32s(&pdu->number_of_packets);
		cpu_to_be32s(&pdu->interval);
	} else {
		pdu->transfer_flags = be32_to_cpu(pdu->transfer_flags);

		be32_to_cpus(&pdu->transfer_buffer_length);
		be32_to_cpus(&pdu->start_frame);
		be32_to_cpus(&pdu->number_of_packets);
		be32_to_cpus(&pdu->interval);
	}
}

static void correct_endian_ret_submit(struct usbip_header_ret_submit *pdu, int send)
{
	if (send) {
		cpu_to_be32s(&pdu->status);
		cpu_to_be32s(&pdu->actual_length);
		cpu_to_be32s(&pdu->start_frame);
		cpu_to_be32s(&pdu->error_count);
	} else {
		be32_to_cpus(&pdu->status);
		be32_to_cpus(&pdu->actual_length);
		be32_to_cpus(&pdu->start_frame);
		be32_to_cpus(&pdu->error_count);
	}
}

static void correct_endian_cmd_unlink(struct usbip_header_cmd_unlink *pdu, int send)
{
	if (send)
		pdu->seqnum = cpu_to_be32(pdu->seqnum);
	else
		pdu->seqnum = be32_to_cpu(pdu->seqnum);
}

static void correct_endian_ret_unlink(struct usbip_header_ret_unlink *pdu, int send)
{
	if (send)
		cpu_to_be32s(&pdu->status);
	else
		be32_to_cpus(&pdu->status);
}

void usbip_header_correct_endian(struct usbip_header *pdu, int send)
{
	__u32 cmd = 0;

	if (send)
		cmd = pdu->base.command;

	correct_endian_basic(&pdu->base, send);

	if (!send)
		cmd = pdu->base.command;

	switch (cmd) {
		case USBIP_CMD_SUBMIT:
			correct_endian_cmd_submit(&pdu->u.cmd_submit, send);
			break;
		case USBIP_RET_SUBMIT:
			correct_endian_ret_submit(&pdu->u.ret_submit, send);
			break;

		case USBIP_CMD_UNLINK:
			correct_endian_cmd_unlink(&pdu->u.cmd_unlink, send);
			break;
		case USBIP_RET_UNLINK:
			correct_endian_ret_unlink(&pdu->u.ret_unlink, send);
			break;

		default:
			/* NOTREACHED */
			err("unknown command in pdu header: %d", cmd);
			usbip_dump_header(pdu);
			BUG();
	}
}
EXPORT_SYMBOL(usbip_header_correct_endian);

static void usbip_iso_pakcet_correct_endian(struct usbip_iso_packet_descriptor *iso, int send)
{
	/* does not need all members. but copy all simply. */
	if (send) {
		iso->offset	= cpu_to_be32(iso->offset);
		iso->length	= cpu_to_be32(iso->length);
		iso->status	= cpu_to_be32(iso->status);
		iso->actual_length = cpu_to_be32(iso->actual_length);
	} else {
		iso->offset	= be32_to_cpu(iso->offset);
		iso->length	= be32_to_cpu(iso->length);
		iso->status	= be32_to_cpu(iso->status);
		iso->actual_length = be32_to_cpu(iso->actual_length);
	}
}

static void usbip_pack_iso(struct usbip_iso_packet_descriptor *iso,
		struct usb_iso_packet_descriptor *uiso, int pack)
{
	if (pack) {
		iso->offset		= uiso->offset;
		iso->length		= uiso->length;
		iso->status		= uiso->status;
		iso->actual_length	= uiso->actual_length;
	} else {
		uiso->offset		= iso->offset;
		uiso->length		= iso->length;
		uiso->status		= iso->status;
		uiso->actual_length	= iso->actual_length;
	}
}


/* must free buffer */
void *usbip_alloc_iso_desc_pdu(struct urb *urb, ssize_t *bufflen)
{
	void *buff;
	struct usbip_iso_packet_descriptor *iso;
	int np = urb->number_of_packets;
	ssize_t size = np * sizeof(*iso);
	int i;

	buff = kzalloc(size, GFP_KERNEL);
	if (!buff)
		return NULL;

	for (i = 0; i < np; i++) {
		iso = buff + (i * sizeof(*iso));

		usbip_pack_iso(iso, &urb->iso_frame_desc[i], 1);
		usbip_iso_pakcet_correct_endian(iso, 1);
	}

	*bufflen = size;

	return buff;
}
EXPORT_SYMBOL(usbip_alloc_iso_desc_pdu);

/* some members of urb must be substituted before. */
int usbip_recv_iso(struct usbip_device *ud, struct urb *urb)
{
	void *buff;
	struct usbip_iso_packet_descriptor *iso;
	int np = urb->number_of_packets;
	int size = np * sizeof(*iso);
	int i;
	int ret;

	if (!usb_pipeisoc(urb->pipe))
		return 0;

	/* my Bluetooth dongle gets ISO URBs which are np = 0 */
	if (np == 0) {
		uerr("iso np == 0\n");
		usbip_dump_urb(urb);
		return 0;
	}

	buff = kzalloc(size, GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

again:
	ret = usbip_xmit(0, ud->tcp_socket, buff, size, 0);
	if (ret != size) {
		if (ret == -EFAULT)
			goto again;
		uerr("recv iso_frame_descriptor, %d\n", ret);
		kfree(buff);

		if (ud->side == USBIP_STUB)
			usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
		else
			usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);

		return -EPIPE;
	}

	for (i = 0; i < np; i++) {
		iso = buff + (i * sizeof(*iso));

		usbip_iso_pakcet_correct_endian(iso, 0);
		usbip_pack_iso(iso, &urb->iso_frame_desc[i], 0);
	}


	kfree(buff);

	return ret;
}
EXPORT_SYMBOL(usbip_recv_iso);


/* some members of urb must be substituted before. */
int usbip_recv_xbuff(struct usbip_device *ud, struct urb *urb)
{
	int ret;
	int size;

	if (ud->side == USBIP_STUB) {
		/* stub_rx.c */
		/* the direction of urb must be OUT. */
		if (usb_pipein(urb->pipe))
			return 0;

		size = urb->transfer_buffer_length;
	} else {
		/* vhci_rx.c */
		/* the direction of urb must be IN. */
		if (usb_pipeout(urb->pipe))
			return 0;

		size = urb->actual_length;
	}

	/* no need to recv xbuff */
	if (!(size > 0))
		return 0;
again:
	ret = usbip_xmit(0, ud->tcp_socket, (char *) urb->transfer_buffer, size, 0);
	if (ret != size) {
		if (ret == -EFAULT || ret == -EINVAL)
			goto again;

		uerr("recv xbuf, %d\n", ret);
		if (ud->side == USBIP_STUB) {
			usbip_event_add(ud, SDEV_EVENT_ERROR_TCP);
		} else {
			usbip_event_add(ud, VDEV_EVENT_ERROR_TCP);
			return -EPIPE;
		}
	}

	return ret;
}
EXPORT_SYMBOL(usbip_recv_xbuff);


/*-------------------------------------------------------------------------*/

static int __init usbip_common_init(void)
{
	uinfo(DRIVER_DESC "" DRIVER_VERSION);

	return 0;
}

static void __exit usbip_common_exit(void)
{
	return;
}




module_init(usbip_common_init);
module_exit(usbip_common_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
