/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __VHUB_H__
#define __VHUB_H__

#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb_ch9.h>
#include HCD_HEADER
#include <linux/workqueue.h>

//Used for ast_notify_user()
#include <asm/arch/platform.h>

#include "usbip_common.h"
#if MFG
#include "vhub_mfg.h"
#endif

#define DEV_FSPD_WEBCAM 0

typedef unsigned int bool;
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

struct vhub_hcd;
struct vhub_device;

#include "vhub_ep.h"
#include "vhub_port_state.h"

#if NEW_MPS_PROTECT
    #define GSIZE 0 //AST1510 don't need this.
#else
    #define GSIZE 1024/*16*/ //guard band size (16 bytes) for OUT buffers
#endif
#define MAX_EP0_BUF_SIZE (64 + GSIZE)
#define FULL_SPD_DIRTY_PATCH 1 //Used to patch when vhub's upstream port is in full speed.

enum _ctrl_state {
    ctrl_idle = 0,
    ctrl_accepted,
    ctrl_rd_data_more,
    ctrl_rd_data_kicked,
    ctrl_rd_data_done,
    ctrl_rd_data_done_no_buf,
    ctrl_rd_status_ack,
    ctrl_nd_done, //For no-data or OUT control
    ctrl_nd_status_ack, //For no-data or OUT control
    ctrl_wr_data_more,
    ctrl_wr_data_kicked,
    ctrl_wr_data_done,
    ctrl_wr_data_done_no_buf,
    ctrl_wr_data_wait_cb,
};

struct cfg_desc_cache {
    unsigned int length;
    unsigned char *cfg_desc;
};

struct vhub_device {
    struct usb_device udev;

    /*
     * devid specifies a remote usb device uniquely instead
     * of combination of busnum and devnum.
     */
    __u32 devid;

    /* speed of a remote device. #include <linux/usb_ch9.h>*/
    enum usb_device_speed speed;

    /*  vhci root-hub port to which this device is attached  */
#define ndev(vdev) ((vdev)->rhport + 1)
    __u32 rhport; //from 0 to (VHUB_NPORTS-1), also mapping the vdev[]'s index#

    struct usbip_device ud;
    struct vkm_dev *vkm;
    void *uas;
    unsigned int remote_wakeup;

    /* lock for the below link lists */
    spinlock_t priv_lock;

    /* vhci_priv is linked to one of them. */
    struct list_head priv_tx;
    struct list_head priv_rx;

    /* vhci_unlink is linked to one of them */
    struct list_head unlink_tx;
    struct list_head unlink_rx;

    /* vhci_tx thread sleeps for this queue */
    wait_queue_head_t waitq_tx;

    /* call back function to handle dev control messages */
    void (*dev_ep0_cb)(struct vhub_device* vdev, u32 event);

    /* used to transfer control messages */
    struct usb_ctrlrequest setup_pkt;

    u8 activeconfig;

    /* buffer for DEV EP0 */
    volatile unsigned char EP0_buf[MAX_EP0_BUF_SIZE] __attribute__ ((aligned (8)));
    unsigned char *EP0_buf_ptr; //a working pointer
    unsigned char *EP0_buf_big; //dynamic allocated buffer for EP0 access
    u16 EP0_buf_length;
    bool EP0_0pkt; //TRUE: will ack a 0-length pkt when necessary.

    enum _ctrl_state ctrl_state; //There is no lock for this state value. Use it atomically.
    struct urb *current_ctrl_urb;

    struct cfg_desc_cache cfg_desc_cache_hispd; //The config desc cache used when upstream is high speed
    struct cfg_desc_cache cfg_desc_cache_fulspd; //The config desc cache used when upstream is full speed
};

/* urb->hcpriv, use container_of() */
struct vhub_priv {
	unsigned long seqnum;
	struct list_head list;

	struct vhub_device *vdev;
	struct urb *urb;
};

struct vhub_unlink {
	/* seqnum of this request */
	unsigned long seqnum;

	struct list_head list;

	/* seqnum of the unlink target */
	unsigned long unlink_seqnum;
};

/*
** These are structures used to save the hub's descriptors
*/
struct usb_hub_cls_descriptor {
    __u8 bDescLength;
    __u8 bDescriptorType;
    __u8 bNbrPorts;
    __le16 wHubCharacteristics;
    __u8 bPwrOn2PwrGood;
    __u8 bHubContrCurrent;
    __u8 DeviceRemovable; // 7 ports use 1 byte
    __u8 PortPwrCtrlMask; //dummy.
} __attribute__ ((packed));
#define USB_DT_HUB_CLS_SIZE (sizeof(struct usb_hub_cls_descriptor))

struct my_usb_endpoint_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEndpointAddress;
	__u8  bmAttributes;
	__le16 wMaxPacketSize;
	__u8  bInterval;
} __attribute__ ((packed));


struct t_hub_descriptors {
    /* standard descriptor for hub */
    struct usb_device_descriptor dev_desc_high_spd __attribute__ ((aligned (8)));
    struct usb_qualifier_descriptor dev_qual_desc_full_spd __attribute__ ((aligned (8)));

    struct usb_config_descriptor conf_desc_high_spd __attribute__ ((aligned (8)));
    struct usb_interface_descriptor interface_desc_sTT;
    struct my_usb_endpoint_descriptor ep_desc_sTT;
    struct usb_interface_descriptor interface_desc_mTT;
    struct my_usb_endpoint_descriptor ep_desc_mTT;

    struct usb_config_descriptor ospd_conf_desc_full_spd __attribute__ ((aligned (8)));
    struct usb_interface_descriptor interface_desc_full_spd;
    struct my_usb_endpoint_descriptor ep_desc_full_spd;
    /* Hub class descriptor */
    struct usb_hub_cls_descriptor hub_cls_desc __attribute__ ((aligned (8)));
} __attribute__ ((packed));


struct attach_t {
    struct work_struct work;
    struct vhub_hcd *vhub;
    u32 port; //==nDev, ==rhport + 1
    enum usb_device_speed speed;
};

struct detach_t {
    struct work_struct work;
    struct vhub_hcd *vhub;
    u32 port; //==nDev, ==rhport + 1
};

struct work_info_t {
    struct work_struct work;
    struct vhub_hcd *vhub;
    u32 port;
    u32 status;
};

struct resume_t {
    struct work_struct work;
    struct vhub_hcd *vhub;
    u32 port; //==nDev, ==rhport + 1
};

/*
** Android 4.4.2 + CVTouch. Android output Bulk type SSPLIT packet to Int type pipe.
** VHUB won't accept it. To workaround this issue, set VHUB's ep type to bulk.
*/
#define VHUB_QUIRK_BULK_SSPLIT (0x1UL << 0)


/*
 * The number of ports is less than 16 ?
 * USB_MAXCHILDREN is statically defined to 16 in usb.h.  Its maximum value
 * would be 31 because the event_bits[1] of struct usb_hub is defined as
 * unsigned long in hub.h
 */
#if NEW_5_DOWN_STREAM_PORT
	#define VHUB_NPORTS 5
#else
	#define VHUB_NPORTS 7
#endif

#define EPBIGBUF_POOL_SIZE 10
struct dma_buf
{
    struct list_head list;
    unsigned char *buf; //buffer virtual addr (base)
    dma_addr_t baddr; //buffer dma addr
    u32 bsize; //size of the whole buffer
};

/* for usb_bus.hcpriv */
struct vhub_hcd {
    spinlock_t	lock;

    /*
     * wPortStatus bit field [0:15]
     * See USB 2.0 spec Table 11-21
     * wPortChange bit field [16:31]
     * See USB 2.0 spec Table 11-22
     * !!Start from 0 as port1
    */
    u32	port_status[VHUB_NPORTS];
    /* spec. Figure 11-10. */
    enum _port_state port_state[VHUB_NPORTS];

//	unsigned	resuming:1;
//	unsigned long	re_timeout;

    atomic_t seqnum;

    /*
     * NOTE:
     * wIndex shows the port number and begins from 1.
     * But, the index of this array begins from 0.
     */
    struct vhub_device vdev[VHUB_NPORTS];

    /* vhub_device which has not been assiged its address yet */
    int pending_port;

    /* buffer for HUB EP0 */
    volatile unsigned char EP0_buf[MAX_EP0_BUF_SIZE] __attribute__ ((aligned (8)));

    /* vhub state to host. See spec Fig 9-1 */
    enum usb_device_state state;

    /* call back function to handle hub control messages */
    void (*hub_ep0_cb)(struct vhub_hcd* vhub, u32 event);

    /* record for reporting wHubStatus/wHubChange */
    u32 hub_wHubStatusChange __attribute__ ((aligned (8)));

    /* the hub's device address */
    u8 devnum;
    /* the hub's configuration value (bConfigurationValue)*/
    u8 activeconfig;
    /* to record hub's remote wakeup bit/status. 1 means ON */
    u8 hub_remote_wakeup;

    struct t_hub_descriptors hub_descs;

    struct ep_info_t ep_info[MAX_PEP_NUM];

    unsigned char __iomem *iobase;
    atomic_t  test;

    struct list_head active_intf;//list of active interfaces (having nonzero size eps)
    struct workqueue_struct *hub_work_thread;
    struct work_struct event_handler;
    struct work_struct bus_reset_work;
    struct work_struct hub_int_events;

    enum _ctrl_state ctrl_state;
    unsigned int EP0_buf_length;
    unsigned char *EP0_buf_ptr;

    unsigned int set_addr_hack; //in us. 0 means no hack.
    unsigned int hid_urb_interval; //in ms. 0 means use value from device descriptor. Default is 35ms.
    unsigned int quirk; //See VHUB_QUIRK_XXXX
    unsigned int active_zone_x; //Used for KMoIP roaming.
    unsigned int active_zone_y; //Used for KMoIP roaming

    struct list_head ep_big_buf_free_list;
    struct list_head ep_big_buf_list;
    struct dma_buf ep_big_buf_pool[EPBIGBUF_POOL_SIZE];
};


extern struct vhub_hcd *the_controller;
extern struct attribute_group dev_attr_group;

/*-------------------------------------------------------------------------*/
/* prototype declaration */
/*-------------------------------------------------------------------------*/
/* vhub_hcd.c */
void vhub_usbip_rx_loop(struct usbip_task *ut);
void vhub_usbip_tx_loop(struct usbip_task *ut);
int vhub_urb_enqueue(struct usb_hcd *hcd, struct usb_host_endpoint *ep, struct urb *urb, gfp_t mem_flags);
int vhub_urb_dequeue(struct usb_hcd *hcd, struct urb *urb);

#define port2rhport(port) ((port) - 1)
#define rhport2port(rhport) ((rhport) + 1)

static inline struct vhub_device *rhport_to_vdev(__u32 rhport)
{
	return &the_controller->vdev[rhport];
}

static inline struct vhub_hcd *hcd_to_vhub(struct usb_hcd *hcd)
{
	return (struct vhub_hcd *) (hcd->hcd_priv);
}

static inline struct usb_hcd *vhub_to_hcd(struct vhub_hcd *vhub)
{
	return container_of((void *) vhub, struct usb_hcd, hcd_priv);
}

static inline struct device *vhub_dev(struct vhub_hcd *vhub)
{
	return vhub_to_hcd(vhub)->self.controller;
}

static inline struct vhub_hcd *vdev_to_vhub(struct vhub_device *vdev)
{
	return the_controller;
}

static inline struct usb_hcd *vdev_to_hcd(struct vhub_device *vdev)
{
	return vhub_to_hcd(vdev_to_vhub(vdev));
}

#endif
