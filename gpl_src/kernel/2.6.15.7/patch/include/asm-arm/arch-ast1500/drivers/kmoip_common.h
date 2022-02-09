/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _KMOIP_COMMON_H_
#define _KMOIP_COMMON_H_

#define KMOIP_MAGIC_NUM 0x4B4D4950
#define KMOIP_MAX_REPORT_BUF_SIZE 64 //64bytes

/*
** Bruce150312. Resolve KDS401 KVM switch compatibility issue.
** Looks like KDS401 doesn't recognize composite USB device with more than 2 interface and endpoints.
** And it will only IN endpoint address 1 and 2. Never 3.
** Solution:
** We change ep addr. Swap MS0 and KBD1. So that ep 1 and 2 matches KBD0 and MS0.
** TODO. Someday, we combine KBD0 and KBD1 into a single KBD so that there is only
** two interface and endpoints.
*/
#define KBD0_EP_ADDR (0x1)
#define KBD1_EP_ADDR (0x3)
#define MS0_EP_ADDR  (0x2)
#define TCH0_EP_ADDR (0x4)
#define MAX_EP_ADDR  (TCH0_EP_ADDR)

#define KBD0_INTF_NUM      (0)
#define KBD1_INTF_NUM      (2)
#define MS0_INTF_NUM       (1)
#define TCH0_INTF_NUM      (3)
#define MAX_INTF_NUM       (4)

// HID spec. P.51
#define HID_REPORT_TYPE_INPUT     (0x01) //input. hid.h::HID_INPUT_REPORT
#define HID_REPORT_TYPE_OUTPUT    (0x02) //output. hid.h::HID_OUTPUT_REPORT
#define HID_REPORT_TYPE_FEATURE   (0x03) //feature. hid.h::HID_FEATURE_REPORT
// VKM specific
#define VKM_REPORT_TYPE_HEARTBEAT (0xF1)

/* driver code uses kmoip_endpoint_type as array index. So, IN's numbering MUST be 1,2,3,... */
enum kmoip_endpoint_type {
	KMOIP_REPORT_KBD0_IN = KBD0_EP_ADDR,
	KMOIP_REPORT_KBD1_IN = KBD1_EP_ADDR,
	KMOIP_REPORT_MS0_IN = MS0_EP_ADDR,
	KMOIP_REPORT_TCH0_IN = TCH0_EP_ADDR,
	KMOIP_REPORT_KBD0_OUT = (0x80 | KBD0_EP_ADDR),
};

struct kmoip_report {
	unsigned int magic_num; //0x4B4D4950 // KMIP
	unsigned int from; // an id 
	unsigned int to; // an id. Used to specify host's id.
	unsigned int seq_num;
	/*
	 * union data structure here
	 * list.next ==> active x. from host.
	 * list.prev ==> active y. from host.
	*/
	struct list_head list;
	void *context; //used to save some local information
	//enum kmoip_enpoint_type report_type;

	/* Start of struct usb_ctrlrequest{} */
	__u8 bRequestType;
	__u8 bRequest;
	__u8 report_id;
	__u8 report_type;
	__u16 interface;
	__u16 report_length;
	unsigned char report[KMOIP_MAX_REPORT_BUF_SIZE];
} __attribute__ ((packed));

#endif //#ifndef _KMOIP_COMMON_H_
