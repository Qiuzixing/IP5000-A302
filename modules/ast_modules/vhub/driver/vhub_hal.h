/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */



#ifndef __VHUB_HAL_H__
#define  __VHUB_HAL_H__
#include <asm/hardware.h>
#include <asm/io.h>

#include "vhub.h"

#define BIT0  (1<<0)
#define BIT1  (1<<1)
#define BIT2  (1<<2)
#define BIT3  (1<<3)
#define BIT4  (1<<4)
#define BIT5  (1<<5)
#define BIT6  (1<<6)
#define BIT7  (1<<7)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)

/* For VHUB_INTS/VHUB_INTC */
#define VHUB_INT_MASK_HIGH (BIT6 | BIT7 | BIT8)
#define VHUB_INT_MASK_LOW (BIT0 |BIT1 |BIT2 |BIT3 |BIT4 |BIT5)
#define VHUB_INT_MASK (VHUB_INT_MASK_LOW | VHUB_INT_MASK_HIGH)

//HUB
#define VHUB_BASE                                  (0x1E6A0000)
#define VHUB_ROOT_CTRL                        (0 + VHUB_BASE)
#define VHUB_ROOT_CFG                         (0x04 + VHUB_BASE)
#define VHUB_INTC                                  (0x08 + VHUB_BASE)
#define VHUB_INTS                                  (0x0C + VHUB_BASE)
#define VHUB_EP_ACK_INTC                    (0x10 + VHUB_BASE)
#define VHUB_EP_NACK_INTC                  (0x14 + VHUB_BASE)
#define VHUB_EP_ACK_INTS                    (0x18 + VHUB_BASE)
#define VHUB_EP_NACK_INTS                  (0x1C + VHUB_BASE)
#define VHUB_SW_RESET                         (0x20 + VHUB_BASE)
#define VHUB_USB_STATUS                     (0x24 + VHUB_BASE)
#define VHUB_EP_TOGGLE_CTRL             (0x28 + VHUB_BASE)
#define VHUB_ISO_FAIL_CNT                  (0x2C + VHUB_BASE)
#define VHUB_HUB_EP0_CTRL                 (0x30 + VHUB_BASE)
#define VHUB_HUB_EP0_BUF                   (0x34 + VHUB_BASE)
#define VHUB_HUB_EP1_CTRL                  (0x38 + VHUB_BASE)
#define VHUB_HUB_EP1_BITMAP             (0x3C + VHUB_BASE)

#define VHUB_ROOT_SETUP_BUF_L         (0x80 + VHUB_BASE)
#define VHUB_ROOT_SETUP_BUF_H         (0x4 + VHUB_ROOT_SETUP_BUF_L)
#define VHUB_DEV1_SETUP_BUF_L         (0x88 + VHUB_BASE)
#define VHUB_DEV1_SETUP_BUF_H         (0x4 + VHUB_DEV1_SETUP_BUF_L)
#define VHUB_DEV2_SETUP_BUF_L         (0x90 + VHUB_BASE)
#define VHUB_DEV2_SETUP_BUF_H         (0x4 + VHUB_DEV2_SETUP_BUF_L)
#define VHUB_DEV3_SETUP_BUF_L         (0x98 + VHUB_BASE)
#define VHUB_DEV3_SETUP_BUF_H         (0x4 + VHUB_DEV3_SETUP_BUF_L)
#define VHUB_DEV4_SETUP_BUF_L         (0xA0 + VHUB_BASE)
#define VHUB_DEV4_SETUP_BUF_H         (0x4 + VHUB_DEV4_SETUP_BUF_L)
#define VHUB_DEV5_SETUP_BUF_L         (0xA8 + VHUB_BASE)
#define VHUB_DEV5_SETUP_BUF_H         (0x4 + VHUB_DEV5_SETUP_BUF_L)
#define VHUB_DEV6_SETUP_BUF_L         (0xB0 + VHUB_BASE)
#define VHUB_DEV6_SETUP_BUF_H         (0x4 + VHUB_DEV6_SETUP_BUF_L)
#define VHUB_DEV7_SETUP_BUF_L         (0xB8 + VHUB_BASE)
#define VHUB_DEV7_SETUP_BUF_H         (0x4 + VHUB_DEV7_SETUP_BUF_L)

//Device
#define VHUB_DEV1_BASE               (0x100 + VHUB_BASE)
#define VHUB_DEV2_BASE               (0x110 + VHUB_BASE)
#define VHUB_DEV3_BASE               (0x120 + VHUB_BASE)
#define VHUB_DEV4_BASE               (0x130 + VHUB_BASE)
#define VHUB_DEV5_BASE               (0x140 + VHUB_BASE)
#define VHUB_DEV6_BASE               (0x150 + VHUB_BASE)
#define VHUB_DEV7_BASE               (0x160 + VHUB_BASE)

#define _VHUB_DEV_CTRL               (0)
#define _VHUB_DEV_INTS               (0x04)
#define _VHUB_DEV_EP0_CTRL       (0x08)
#define _VHUB_DEV_EP0_BUF         (0x0C)

/* i range from 1 to 7 */
#if 0
#define VHUB_DEV_BASE(i)                   (VHUB_DEV##i##_BASE)
#define VHUB_DEV_CTRL(i)                   (_VHUB_DEV_CTRL + VHUB_DEV##i##_BASE)
#define VHUB_DEV_INTS(i)                   (_VHUB_DEV_INTS + VHUB_DEV##i##_BASE)
#define VHUB_DEV_EP0_CTRL(i)           (_VHUB_DEV_EP0_CTRL + VHUB_DEV##i##_BASE)
#define VHUB_DEV_EP0_BUF(i)             (_VHUB_DEV_EP0_BUF + VHUB_DEV##i##_BASE)
#define VHUB_DEV_SETUP_BUF_L(i)     (VHUB_DEV##i##_SETUP_BUF_L)
#define VHUB_DEV_SETUP_BUF_H(i)     (VHUB_DEV##i##_SETUP_BUF_H)
#endif
/* i range from 1 to 7 */
#define VHUB_DEV_BASE(i)                   (VHUB_DEV1_BASE + (((i) - 1) * 16))
#define VHUB_DEV_CTRL(i)                   (_VHUB_DEV_CTRL + VHUB_DEV_BASE(i))
#define VHUB_DEV_INTS(i)                   (_VHUB_DEV_INTS + VHUB_DEV_BASE(i))
#define VHUB_DEV_EP0_CTRL(i)           (_VHUB_DEV_EP0_CTRL + VHUB_DEV_BASE(i))
#define VHUB_DEV_EP0_BUF(i)             (_VHUB_DEV_EP0_BUF + VHUB_DEV_BASE(i))
#define VHUB_DEV_SETUP_BUF_L(i)     (VHUB_ROOT_SETUP_BUF_L + ((i) * 8))
#define VHUB_DEV_SETUP_BUF_H(i)     (VHUB_ROOT_SETUP_BUF_H + ((i) * 8))


//EndPoint (from 0 to 20)
#define VHUB_PEP_BASE(i)                              (0x200  + ((i)*0x10) + VHUB_BASE)

#define _VHUB_PEP_CTRL                                 (0x00)
#define _VHUB_PEP_DESC_CTRL                       (0x04)
#define _VHUB_PEP_DESC_BUF                        (0x08)
#define _VHUB_PEP_DESC_STATUS                  (0x0C)
#if NEW_256_STAGE_DESC
    #define RMASK_EPP0C_WP        (0x000000FF)
    #define RMASK_EPP0C_RP         (0x0000FF00)
#else
    #define RMASK_EPP0C_WP        (0x0000001F)
    #define RMASK_EPP0C_RP         (0x00001F00)
#endif

/* i range from 0 to 20 */
#define VHUB_PEP_CTRL(i)                               (_VHUB_PEP_CTRL + VHUB_PEP_BASE(i))
#define VHUB_PEP_DESC_CTRL(i)                     (_VHUB_PEP_DESC_CTRL + VHUB_PEP_BASE(i))
#define VHUB_PEP_DESC_BUF(i)                      (_VHUB_PEP_DESC_BUF + VHUB_PEP_BASE(i))
#define VHUB_PEP_DESC_STATUS(i)                (_VHUB_PEP_DESC_STATUS + VHUB_PEP_BASE(i))

#define SET_REG_32(addr, value)    iowrite32(cpu_to_le32(value), the_controller->iobase + ((addr) - VHUB_BASE))
#define GET_REG_32(addr)               (le32_to_cpu(ioread32(the_controller->iobase + ((addr) - VHUB_BASE))))
#define MEMCPY_FROMIO(pDes, IoOffset, bytes_cnt) \
    memcpy_fromio((pDes), the_controller->iobase + ((IoOffset) - VHUB_BASE), (bytes_cnt));

/* use & or | as op */
#define MOD_REG_32(addr, op, value) \
    do { \
        u32 _r; \
        _r = GET_REG_32((addr)); \
        _r op##= (value); \
        SET_REG_32((addr), _r); \
    } while (0)

int vhub_hal_initialization(struct vhub_hcd *vhub);
void vhub_hal_halt(struct vhub_hcd *vhub);
int vhub_hal_enable_dev(struct vhub_hcd *vhub, u32 port, enum usb_device_speed speed);
int vhub_hal_detach_dev(struct vhub_hcd *vhub, u32 port); //port start from 1
void vhub_hal_indicate_port_change(struct vhub_hcd *vhub, u32 port);
void vhub_hal_detach_upstream(void);
bool is_vhub_configured(void);
void rh_port_connect(int rhport, enum usb_device_speed speed);
void rh_port_disconnect(int rhport);
void rh_port_resume(int rhport);

static inline void vhub_hal_disable_device_port(struct vhub_hcd *vhub, u32 port)
{
    //MOD_REG_32(VHUB_DEV_CTRL(port), &, ~BIT0);
    SET_REG_32(VHUB_DEV_CTRL(port), 0);
}

static inline void vhub_hal_clear_port_change(u32 port)
{
    MOD_REG_32(VHUB_HUB_EP1_BITMAP, &, ~(1 << (port)));
}

void dev_free_actconfig(struct vhub_device *vdev);
void vhub_hal_wakeup_upstream_port(struct vhub_hcd *vhub);
void hub_SetPortFeatureCompleted(struct vhub_hcd *vhub);

#define hub_suspended(vhub) ((vhub)->state > USB_STATE_SUSPENDED)
#define hub_in_hi_spd() (GET_REG_32(VHUB_USB_STATUS) & BIT27)

/* Only be valid after cur_altsetting is set. */
#define dev_active_intf_cls_type(usb_interface_ptr) \
            ((usb_interface_ptr)->cur_altsetting->desc.bInterfaceClass)
#define dev_active_intf_sub_cls_type(usb_interface_ptr) \
            ((usb_interface_ptr)->cur_altsetting->desc.bInterfaceSubClass)
#define dev_active_intf_protocol_type(usb_interface_ptr) \
            ((usb_interface_ptr)->cur_altsetting->desc.bInterfaceProtocol)

#endif /* #ifndef __VHUB_HAL_H__ */
