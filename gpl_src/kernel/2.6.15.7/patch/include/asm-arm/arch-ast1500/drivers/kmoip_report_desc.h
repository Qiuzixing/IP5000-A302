/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _KMOIP_REPORT_DESC_H_
#define _KMOIP_REPORT_DESC_H_
#include <linux/usb_ch9.h>

#define KMOIP_2BYTES_MOUSE_CURSOR 0

struct my_hid_class_descriptor {
	__u8  bDescriptorType;
	__u16 wDescriptorLength;
} __attribute__ ((packed));

struct my_hid_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u16 bcdHID;
	__u8  bCountryCode;
	__u8  bNumDescriptors;

	struct my_hid_class_descriptor desc[1];
} __attribute__ ((packed));

struct my_kmoip_usb_endpoint_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;

	__u8  bEndpointAddress;
	__u8  bmAttributes;
	__le16 wMaxPacketSize;
	__u8  bInterval;
} __attribute__ ((packed));

struct my_usb_config_descriptor {
	struct usb_config_descriptor conf_desc_low_spd __attribute__ ((packed));
		struct usb_interface_descriptor kbd0_interface_desc __attribute__ ((packed));
			struct my_hid_descriptor kbd0_hid_desc __attribute__ ((packed));
			struct my_kmoip_usb_endpoint_descriptor kbd0_ep_desc_in __attribute__ ((packed));
		struct usb_interface_descriptor ms0_interface_desc __attribute__ ((packed));
			struct my_hid_descriptor ms0_hid_desc __attribute__ ((packed));
			struct my_kmoip_usb_endpoint_descriptor ms0_ep_desc_in __attribute__ ((packed));
		struct usb_interface_descriptor kbd1_interface_desc __attribute__ ((packed));
			struct my_hid_descriptor kbd1_hid_desc __attribute__ ((packed));
			struct my_kmoip_usb_endpoint_descriptor kbd1_ep_desc_in __attribute__ ((packed));
		struct usb_interface_descriptor tch0_interface_desc __attribute__ ((packed));
			struct my_hid_descriptor tch0_hid_desc __attribute__ ((packed));
			struct my_kmoip_usb_endpoint_descriptor tch0_ep_desc_in __attribute__ ((packed));
} __attribute__ ((packed));


#ifdef KMOIP_RDESC_DEFINE
/* Copied from MS keyboard
** Generic Desktop::keyboard (total IN 8 bytes, OUT 1 byte)
** 	Page: 0x07 keyboard/keypad
**		0xE0 ~ 0xE7 (control key)
**		1bit x 8: data variable type (1 or 0)
**		1bit x 8: constant array type (NULL)
**	Page (Output): 0x08 LED
**		0x01 ~ 0x03
**		1bit x 3: data variable (1 or 0)
**		5bits x 1: constant array (NULL)
**	Page: keyboard/keypad
**		0x00 ~ 0xFF
**		8bits x 6: data array type (key codes)
*/
static char vkm_kbd_0_rdesc[] = {
	0x05, 0x01, //Usage Page (Generic Desktop)
	0x09, 0x06, //Usage (Keyboard)
	0xA1, 0x01, //Collection (Application)
		0x05, 0x07, //Usage Page (Keyboard/Keypad)
		0x19, 0xE0, //Usage Minimum (Keyboard Left Control)
		0x29, 0xE7, //Usage Maximum (Keyboard Right GUI)
		0x15, 0x00, //Logical Minimum (0)
		0x25, 0x01, //Logical Maximum (1)
		0x95, 0x08, //Report Count (8)
		0x75, 0x01, //Report Size (1)
		0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

		0x95, 0x08, //Report Count (8)
		0x75, 0x01, //Report Size (1)
		0x81, 0x01, //Input (Cnst,Ary,Abs)

		0x05, 0x08, //Usage Page (LEDs)
		0x19, 0x01, //Usage Minimum (Num Lock)
		0x29, 0x03, //Usage Maximum (Scroll Lock)
		0x95, 0x03, //Report Count (3)
		0x75, 0x01, //Report Size (1)
		0x91, 0x02, //Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)

		0x95, 0x01, //Report Count (1)
		0x75, 0x05, //Report Size (5)
		0x91, 0x01, //Output (Cnst,Ary,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)

		0x05, 0x07, //Usage Page (Keyboard/Keypad)
		0x19, 0x00, //Usage Minimum (Undefined)
		0x2A, 0xFF, 0x00, //Usage Maximum
		0x15, 0x00, //Logical Minimum (0)
		0x26, 0xFF, 0x00, //Logical Maximum (255)
		0x95, 0x06, //Report Count (6)
		0x75, 0x08, //Report Size (8)
		0x81, 0x00, //Input (Data,Ary,Abs)
	0xC0 //End Collection
};

/* Copied from MS keyboard
** Report ID: 1 (total 1 + 7 bytes)
**	Page: 0x0C consumer device
**		0x0 ~ 0x3FF
**		16bits x 1: data array type (0~0x3FF)
**	Page: 0x07 keyboard/keypad
**		0x0~0xFF
**		8bits x 1: data array type (0~255)
**		8bits x 1: constant array type (NULL)
**	Page: 0xFF00 vendor-defined
**		Usage: 0xFE03 ~ 0xFE04
**		1bit x 2: data variable (1 or 0)
**	Page: 0xFF05 vendor-defined
**		5bits x 1: data variable (0 ~ 31)
**		9bits x 1: constant array type (NULL)
**	Page: 0xFF02 vendor-defined
**		8bits x 1: data variable (0 ~255)
**
** Report ID: 3 (total 1 + 1 bytes) Generic Desktop::system control
**		8bits x 1: data array
**
*/
static char vkm_kbd_1_rdesc[] = {
	0x05, 0x0C, //Usage Page (Consumer Devices)
	0x09, 0x01, //Usage (Consumer Control)
	0xA1, 0x01, //Collection (Application)
		0x85, 0x01, //Report ID (1)
		0x05, 0x0C, //Usage Page (Consumer Devices)
		0x19, 0x00, //Usage Minimum (Undefined)
		0x2A, 0xFF, 0x03, //Usage Maximum
		0x95, 0x01, //Report Count (1)
		0x75, 0x10, //Report Size (16)
		0x15, 0x00, //Logical Minimum (0)
		0x27, 0xFF, 0x03, 0x00, 0x00, //Logical Maximum (1023)
		0x81, 0x00, //Input (Data,Ary,Abs)

		0x05, 0x07, //Usage Page (Keyboard/Keypad)
		0x19, 0x00, //Usage Minimum (Undefined)
		0x29, 0xFF, //Usage Maximum
		0x75, 0x08, //Report Size (8)
		0x26, 0xFF, 0x00, //Logical Maximum (255)
		0x81, 0x00, //Input (Data,Ary,Abs)

		0x81, 0x01, //Input (Cnst,Ary,Abs)

		0x06, 0x00, 0xFF, //Usage Page (Vendor-Defined 1)
		0x0A, 0x03,	0xFE, //Usage (Vendor-Defined 65027)
		0x0A, 0x04, 0xFE, //Usage (Vendor-Defined 65028)
		0x95, 0x02, //Report Count (2)
		0x75, 0x01, //Report Size (1)
		0x25, 0x01, //Logical Maximum (1)
		0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

		0x0A, 0x05, 0xFF, //Usage (Vendor-Defined 65285)
		0x95, 0x01, //Report Count (1)
		0x75, 0x05, //Report Size (5)
		0x25, 0x1F, //Logical Maximum (31)
		0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

		0x75, 0x09, //Report Size (9)
		0x81, 0x01, //Input (Cnst,Ary,Abs)

		0x0A, 0x02, 0xFF, //Usage (Vendor-Defined 65282)
		0x26, 0xFF,	0x00, //Logical Maximum (255)
		0x75, 0x08, //Report Size (8)
		0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
	0xC0, //End Collection

	0x05, 0x01, //Usage Page (Generic Desktop)
	0x09, 0x80, //Usage (System Control)
	0xA1, 0x01, //Collection (Application)
		0x85, 0x03, //Report ID (3)
		0x19, 0x00, //Usage Minimum (Undefined)
		0x29, 0xFF, //Usage Maximum
		0x15, 0x00, //Logical Minimum (0)
		0x26, 0xFF, 0x00, //Logical Maximum (255)
		0x81, 0x00, //Input (Data,Ary,Abs)
	0xC0 //End Collection
};

/* Copied from MS Mouse
** Generic Desktop::Mouse (total 4 bytes)
**	Page: 0x09 Button
**		0x01 ~ 0x08
**		1bits x 8: data variable (1 or 0)
**	Page: 0x01 Generic desktop
**		Usage: 0x1 pointer, Usage X: 0x30
**		Usage: 0x1 pointer, Usage Y: 0x31
**		8bits x 2: data variable (-127~127)
**	Page: 0x38 Wheel
**		8btis x 1: data variable (-127~127)
*/
static char vkm_ms_0_rdesc[] = {
	0x05, 0x01, //Usage Page (Generic Desktop)
	0x09, 0x02, //Usage (Mouse)
	0xA1, 0x01, //Collection (Application)
		0x05, 0x09, //Usage Page (Button)
		0x19, 0x01, //Usage Minimum (Button 1)
		0x29, 0x08, //Usage Maximum (Button 8)
		0x15, 0x00, //Logical Minimum (0)
		0x25, 0x01, //Logical Maximum (1)
		0x95, 0x08, //Report Count (8)
		0x75, 0x01, //Report Size (1)
		0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

		/*
		0x95, 0x01, //Report Count (1)
		0x75, 0x05, //Report Size (5)
		0x81, 0x03, //Input (Cnst,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
		*/

		0x05, 0x01, //Usage Page (Generic Desktop)
		0x09, 0x01, //Usage (Pointer)
		0xA1, 0x00, //Collection (Physical)
			0x09, 0x30, //Usage (X)
			0x09, 0x31, //Usage (Y)
#if KMOIP_2BYTES_MOUSE_CURSOR
			0x16, 0x01, 0x80, // LOGICAL_MINIMUM (-32767)
			0x26, 0xFF, 0x7F,// LOGICAL_MAXIMUM (32767)
			0x75, 0x10, // REPORT_SIZE (16)
			0x95, 0x02, // REPORT_COUNT (2)
#else
			0x15, 0x81, //Logical Minimum (-127)
			0x25, 0x7F, //Logical Maximum (127)
			0x75, 0x08, //Report Size (8)
			0x95, 0x02, //Report Count (2)
#endif
			0x81, 0x06, //Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)
		0xC0, //End Collection

		0x09, 0x38, //Usage (Wheel)
		0x15, 0x81, //Logical Minimum (-127)
		0x25, 0x7F, //Logical Maximum (127)
		0x75, 0x08, //Report Size (8)
		0x95, 0x01, //Report Count (1)
		0x81, 0x06, //Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)
	0xC0 //End Collection
};
#ifdef SIMPLE_TCH0
/* Modified from vkm_ms_0_rdesc
** Generic Desktop::Pointer (0x01::0x01) (total 5 bytes)
**	Page: 0x09 Button
**		0x01 ~ 0x08
**		1bits x 8: data variable (1 or 0)
**	Page: 0x01 Generic desktop
**		Usage: 0x1 pointer, Usage X: 0x30
**		Usage: 0x1 pointer, Usage Y: 0x31
**		8bits x 2: data variable (0 ~ 32767). Absolute position.
*/
static char vkm_tch_0_rdesc[] = {
		0x05, 0x01, //Usage Page (Generic Desktop)
		0x09, 0x02, //Usage (Mouse)
		0xA1, 0x01, //Collection (Application)
			0x05, 0x09, //Usage Page (Button)
			0x19, 0x01, //Usage Minimum (Button 1)
			0x29, 0x08, //Usage Maximum (Button 8)
			0x15, 0x00, //Logical Minimum (0)
			0x25, 0x01, //Logical Maximum (1)
			0x95, 0x08, //Report Count (8)
			0x75, 0x01, //Report Size (1)
			0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
	
			/*
			0x95, 0x01, //Report Count (1)
			0x75, 0x05, //Report Size (5)
			0x81, 0x03, //Input (Cnst,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
			*/
	
			0x05, 0x01, //Usage Page (Generic Desktop)
			0x09, 0x01, //Usage (Pointer)
			0xA1, 0x00, //Collection (Physical)
				0x09, 0x30, //Usage (X)
				0x09, 0x31, //Usage (Y)
				0x15, 0x00, // LOGICAL_MINIMUM (0)
				0x26, 0xFF, 0x7F,// LOGICAL_MAXIMUM (32767)
				0x75, 0x10, // REPORT_SIZE (16)
				0x95, 0x02, // REPORT_COUNT (2)
				0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
			0xC0, //End Collection
		0xC0 //End Collection
};
#else
/* Modified from vkm_ms_0_rdesc
** Generic Desktop::Pointer (0x01::0x01) (total 6 bytes)
**	Page: 0x09 Button
**		0x01 ~ 0x08
**		1bits x 8: data variable (1 or 0)
**	Page: 0x01::0x38 Generic Desktop::Wheel
**		0x08 ~ 0x01
**		8bits x 1: data variable (-127 ~ 127)
**	Page: 0x01 Generic desktop
**		Usage: 0x1 pointer, Usage X: 0x30
**		Usage: 0x1 pointer, Usage Y: 0x31
**		8bits x 2: data variable (0 ~ 32767). Absolute position.
*/
static char vkm_tch_0_rdesc[] = {
		0x05, 0x01, //Usage Page (Generic Desktop)
		0x09, 0x02, //Usage (Mouse)
		0xA1, 0x01, //Collection (Application)
			0x09, 0x01, //Usage (Pointer)
			0xA1, 0x00, //Collection (Physical)
				0x05, 0x09, //Usage Page (Button)
				0x19, 0x01, //Usage Minimum (Button 1)
				0x29, 0x08, //Usage Maximum (Button 8)
				0x15, 0x00, //Logical Minimum (0)
				0x25, 0x01, //Logical Maximum (1)
				0x95, 0x08, //Report Count (8)
				0x75, 0x01, //Report Size (1)
				0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
		
				/*
				0x95, 0x01, //Report Count (1)
				0x75, 0x05, //Report Size (5)
				0x81, 0x03, //Input (Cnst,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
				*/

				0x75, 0x08, //Report Size (8)
				0x95, 0x01, //Report Count (1)
				0x05, 0x01, //Usage Page (Generic Desktop)
				0x09, 0x38, //Usage (Wheel)
				0x15, 0x81, //Logical Minimum (-127)
				0x25, 0x7F, //Logical Maximum (127)
				0x81, 0x06, //Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)
			0xC0, //End Collection
			0x09, 0x01, //Usage (Pointer)
			0xA1, 0x00, //Collection (Physical)
				0x05, 0x01, //Usage Page (Generic Desktop)
				0x09, 0x30, //Usage (X)
				0x09, 0x31, //Usage (Y)
				0x15, 0x00, // LOGICAL_MINIMUM (0)
				0x26, 0xFF, 0x7F,// LOGICAL_MAXIMUM (32767)
				0x75, 0x10, // REPORT_SIZE (16)
				0x95, 0x02, // REPORT_COUNT (2)
				0x81, 0x02, //Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
			0xC0, //End Collection
		0xC0 //End Collection
};
#endif /* #ifdef SIMPLE_TCH0 */

#endif /* #ifdef KMOIP_RDESC_DEFINE */

#endif //#ifndef _KMOIP_REPORT_DESC_H_

