/*
 * Copyright (c) 2004-2013
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _VHUB_KMOIP_H_
#define _VHUB_KMOIP_H_

#include <linux/fs.h> //for inode
#include <linux/list.h>
#include <linux/workqueue.h>

#include <net/sock.h>
#include <linux/socket.h>
#include <linux/tcp.h> //socketfd_lookup()
#include <linux/file.h> //For sockfd_put()
#include <asm/arch/drivers/kmoip_common.h>
#include <asm/arch/drivers/kmoip_report_desc.h>

#define KMOIP_RHPORT 0
#define KMOIP_DEV_ID ((1500/*busnum*/ << 16) | 1/*devnum*/)

#define VKM_REPORT_LIST_SIZE 32
#define VKM_MAX_EP_IN_NUM (MAX_EP_ADDR + 1) // 1,2,3 valid. 0 reserved. Mapping to ep addr.

#define vdev_get_vkm(vdev) ((vdev)->vkm)
#define vdev_set_vkm(vdev, vkm) ((vdev)->vkm = (vkm))
#define urb_to_vdev(urb) container_of((struct usb_device *)((urb)->dev), struct vhub_device, udev)
#define urb_to_vkm(urb) vdev_get_vkm(urb_to_vdev(urb))
#define is_vkm_urb(urb) (!!urb_to_vkm(urb))

/* copied from usb_ch9.h::enum usb_device_state{} */
enum vkm_state {
	/* NOTATTACHED isn't in the USB spec, and this state acts
	 * the same as ATTACHED ... but it's clearer this way.
	 */
	VKM_STATE_NOTATTACHED = 0,

	/* the chapter 9 device states */
	VKM_STATE_ATTACHED,
	VKM_STATE_POWERED,
	VKM_STATE_DEFAULT,			/* limited function */
	VKM_STATE_ADDRESS,
	VKM_STATE_CONFIGURED,			/* most functions */

	VKM_STATE_SUSPENDED

	/* NOTE:  there are actually four different SUSPENDED
	 * states, returning to POWERED, DEFAULT, ADDRESS, or
	 * CONFIGURED respectively when SOF tokens flow again.
	 */
};

struct vkm_dev {
	spinlock_t lock; //generic driver lock
	struct socket *upstream_socket; //net receive from client
	struct socket *downstream_socket; //net tx to clients (multicast)
	unsigned int is_socket_ready;
	enum vkm_state state;
	unsigned int id;
	unsigned int remote_wakeup;

	struct workqueue_struct *vkm_wq; //vkm state machine work queue
	struct work_struct net_tx_work; //downstream work
	//struct work_struct tick_work; //periodical work
	struct work_struct urb_work; //handling urb request
	struct work_struct kbd0_report_work; //handling reports
	struct work_struct kbd1_report_work; //handling reports
	struct work_struct ms0_report_work; //handling reports
	struct work_struct tch0_report_work; //handling reports
	struct work_struct heartbeat_timeout_work; //handling heartbeat timeout
	struct work_struct init_work; //work queue inital

	struct workqueue_struct *rx_wq; //vkm net rx work queue
	struct work_struct net_rx_work; //upstream work

	struct list_head kbd0_report_in_list;
	struct list_head kbd1_report_in_list;
	struct list_head ms0_report_in_list;
	struct list_head tch0_report_in_list;
	struct list_head kbd0_report_out_list;
	struct list_head report_pool_list;
	struct kmoip_report reports[VKM_REPORT_LIST_SIZE];
	struct kmoip_report last_kbd0_out_report;
	unsigned int report_fifo_size[VKM_MAX_EP_IN_NUM];

	struct list_head urb_req_list; //a fifo for generic urb request
	struct list_head kbd0_urb_req_pending_list; //a fifo for urb IN report request
	struct list_head kbd1_urb_req_pending_list; //a fifo for urb IN report request
	struct list_head ms0_urb_req_pending_list; //a fifo for urb IN report request
	struct list_head tch0_urb_req_pending_list; //a fifo for urb IN report request

	u16 kbd0_0_duration; //kbd 0, report id 0. in 4ms
	u16 kbd1_1_duration; //kbd 1, report id 1
	u16 kbd1_3_duration; //kbd 1, report id 3
	u16 ms0_0_duration; //ms 0, report id 0
	u16 tch0_0_duration; //tch 0, report id 0

	u16 kbd0_0_protocol; // 0: boot protocol, 1: report protocol
	u16 ms0_0_protocol; // 0: boot protocol, 1: report protocol
	u16 tch0_0_protocol; // 0: boot protocol, 1: report protocol

	unsigned int downstream_seq_num;

	unsigned int token;
	unsigned long token_timeout; //in jiffies
	unsigned long token_interval; //in jiffies

	unsigned char *kbd0_rdesc;
	unsigned char *kbd1_rdesc;
	unsigned char *ms0_rdesc;
	unsigned char *tch0_rdesc;
	unsigned int kbd0_rdesc_size;
	unsigned int kbd1_rdesc_size;
	unsigned int ms0_rdesc_size;
	unsigned int tch0_rdesc_size;

	struct my_usb_config_descriptor *vkm_cfg_desc;
	unsigned int vkm_cfg_desc_size; //le16
};


extern int kmoip_attach(struct vhub_device *vdev, struct socket *usocket, struct socket *dsocket);
extern void kmoip_reattach_socket(struct vhub_device *vdev, struct socket *usocket, struct socket *dsocket);
extern void kmoip_detach_socket(struct vhub_device *vdev);
extern void kmoip_enqueue_urb(struct urb *urb);
extern void kmoip_dequeue_urb(struct urb *urb);

#endif //#ifndef _VHUB_KMOIP_H_


