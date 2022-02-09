/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef __VHUB_EP_H__
#define __VHUB_EP_H__

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#if NEW_256_STAGE_DESC
    /*HW 32/256 configurable vaule. Must be power of 2*/
    /*Configurable through HUB00:18 reg.*/
    #define MAX_STAGES 256
    #define NBUF_BASE 32   //For easy backward compatibility.
#else
    #define MAX_STAGES 32 /*HW fixed vaule. Must be power of 2*/
    #define NBUF_BASE MAX_STAGES
#endif

#if NEW_15_EPS
    #define MAX_PEP_NUM 15
#else
    #define MAX_PEP_NUM 21
#endif

struct ep_dma_desc_t {
    volatile __le32 buf_addr; //bit[3:27] 8 bytes aligned
    volatile __le32 ctrl;
} __attribute__ ((packed)); 

struct split_info_t {
    struct ep_dma_desc_t *desc;
    u32 bsize;
    unsigned char *buf; //buffer virtual addr
    dma_addr_t baddr; //buffer dma addr
} __attribute__ ((aligned(sizeof(unsigned int))));

struct buf_info_t {
    struct list_head blist;
    unsigned char *buf; //buffer virtual addr (base)
    dma_addr_t baddr; //buffer dma addr
    u32 actual_length; // -1 means this binfo is not used.
    u32 status; //Used for ISO split case. copied from *desc->ctrl
    u32 iso_seq;
    struct ep_dma_desc_t *desc;
    struct split_info_t *split_info;
#if DEV_FSPD_WEBCAM
    unsigned int split_cnt; //ToDo. initialize it to 0
#endif
} __attribute__ ((aligned(sizeof(unsigned int))));

struct head_info_t {
    struct list_head hlist; //point to next head
    struct list_head blist; //point to the blist
    unsigned char *buf;  //point to the virtual addr of the buf
    //dma_addr_t baddr;  //point to the dma addr of the buf
    u32 length; //the total length of the buf under this head_info
    u32 binfo_used; //number of blist actually used; for ISO device;
    struct urb *urb; //point to the urb binded to this head_info
    u32 seqnum;
} __attribute__ ((aligned(sizeof(unsigned int))));

#define EP_NUMBER(reg)     (((reg) & 0x00000F00) >> 8)
#define EP_TYPE(reg)          (((reg) & 0x00000070) >> 4)
#define EP_DEV_PORT(reg)  (((reg) & 0x0000000E) >> 1)
#define EP_TYPE_OUT(reg)  ((EP_TYPE(reg)) & BIT0)
#define EP_TYPE_ISO(reg)  ((EP_TYPE(reg)) == EP_TYPE_ISO_IN || (EP_TYPE(reg)) == EP_TYPE_ISO_OUT)
#define EP_TYPE_DISABLE 0
#define EP_TYPE_BULK_IN 2
#define EP_TYPE_BULK_OUT 3
#define EP_TYPE_INT_IN 4
#define EP_TYPE_INT_OUT 5
#define EP_TYPE_ISO_IN 6
#define EP_TYPE_ISO_OUT 7

struct intf_info_t {
    struct list_head list;//list under vhub
    struct vhub_device *vdev; //The vdev it belongs to
    struct usb_interface *interface; //information
    atomic_t cnt; //number of attached eps
    u32 ep_int_mask; //bit 0 to 20, ep_int_mask |= 1 << ep_info->idx
    spinlock_t lock; //locks to lock following lists
    struct list_head ep;

    struct tasklet_struct task;
    u32 killing; //0: normal stage, 1: try to kill this interface
};

/* ep state */
enum ep_state {
    ES_IDLE = 0,
    ES_IN_SUBMITING = 1,
    ES_IN_COMPLETING = 2,
    ES_OUT_SUBMITING = 1,
    ES_ISO_STARTING = 1,
    ES_ISO_STARTED = 2,
};


struct ep_info_t;

struct ep_info_t {
    struct vhub_device *vdev;
    struct usb_interface *interface; //which interface this ep belongs to.
    /* ep desc from remote */
    struct usb_endpoint_descriptor *desc;

    struct intf_info_t *intf_info;
    struct list_head list; //Used to link into one of intf_info's ep list
    u32 priority; //priority value for interface handler
    enum ep_state state;
    u32 (*polling_handler)(struct intf_info_t *, struct ep_info_t *);
    void (*urb_callback)(struct urb *urb, struct pt_regs *mem_flags);

    //The ep addr of remote
    //u8 ep_addr;
    // max pkt size of remote ep.
    u32 max_pkt_size;
    // Interval. Used for urb submit.
    int interval;
    // additional transaction opportunities per microframe. For ISO/Interrupt.
    u32 extra_transaction; //0: none(1 transaction per microframe), 1 (2 per microframe)...
    //The ep addr used in vhub HW, including the dir bit.
    u8 ep_addr_local;
    //The ep addr used in remote, including the dir bit.
    u8 ep_addr_remote;
    //The index number from 0 to 20. mapped to vhub->ep_info[] and the HW registers
    u32 idx;

    /* map to EPP00. Only [1:11] are valid.*/
    /* contain local ep addr, ep type and local port */
    u32 ctrl_reg;

    struct ep_dma_desc_t *dlist; //desc list virtual addr (base)
    dma_addr_t daddr; //desc list dma addr
    u32 dsize; //size of the whole desc list

    /* These information is used to release these resource. */
    unsigned char *buf; //buffer virtual addr (base)
    dma_addr_t baddr; //buffer dma addr
    u32 bsize; //size of the whole buffer
    u32 blksize; //The size in bytes of a chunk of buffer

#ifdef TASKLET_IN
    struct tasklet_struct list_task; //an tasklet to process list
#else
    struct workqueue_struct *list_work_q; //a workqueue to process list
    struct work_struct list_work; //the work to process list
#endif

    struct list_head blist; //holds the list of free struct buf_info_t{}
    struct list_head hlist; //holds the list of free struct head_info_t{}
    struct list_head dmalist; //holds the list of enqueued into dma OUT:struct buf_info_t{}; IN:struct head_info_t{}
    struct list_head reqlist; //holds the list of submited struct head_info_t{}
    struct list_head rdylist; //holds the list of hinfos, ready to enq to dma IN.
    unsigned char *list_buf; //point to the memory used for blist and hlist. Used to free the memory.
    u32 nbuf; //number of buffers to be allocated
    u32 nhead; //number of head info to be allocated.
    
    u32 crp; //the index of cpu rp from 0 to (MAX_STAGES - 1)

    bool onebyone; //If TRUE, use requets can only be sent one usb pkt at a time
    bool stall; // false: normal true: this ep is stalled
    spinlock_t lock; //spin lock to lock lists
    atomic_t nbinfo; //number of free binfo

    unsigned long time_stamp;
    u32 to_submit; //number of request to submit
    atomic_t seqnum; //test only. Used to trace the sequence of hinfo submission.
    u32 iso_pkt_num; //frames per descriptor

    unsigned int ep_quirk; //See VHUB_QUIRK;
};

#define blist_add_tail(ep_info, binfo) \
    do { \
        binfo_init(binfo); \
        list_add_tail(&(binfo)->blist, &(ep_info)->blist); \
        atomic_inc(&ep_info->nbinfo); \
    } while (0)
    
#define blist_move_tail(ep_info, binfo) \
    do { \
        binfo_init(binfo); \
        list_move_tail(&(binfo)->blist, &(ep_info)->blist); \
        atomic_inc(&(ep_info)->nbinfo); \
    } while (0)

#define blist_read_head(ep_info)  list_entry((ep_info)->blist.next, struct buf_info_t, blist)
//#define blist_del_head(ep_info) list_del((ep_info)->blist.next)
#define blist_is_empty(ep_info) list_empty(&(ep_info)->blist)
//#define blist_for_each_entry_safe(ep_info, binfo, t) list_for_each_entry_safe(binfo, t, &(ep_info)->blist, blist)

#define hdlist_add_tail(ep_info, hinfo) \
    do { \
        hinfo_init(hinfo); \
        list_add_tail(&(hinfo)->hlist, &(ep_info)->hlist); \
    } while (0)
    
#define hdlist_move_tail(ep_info, hinfo) \
    do { \
        hinfo_init(hinfo); \
        list_move_tail(&(hinfo)->hlist, &(ep_info)->hlist); \
    } while(0)

#define hdlist_read_head(ep_info)  list_entry((ep_info)->hlist.next, struct head_info_t, hlist)
#define hdlist_del_head(ep_info) list_del((ep_info)->hlist.next)
#define hdlist_is_empty(ep_info) list_empty(&(ep_info)->hlist)
#define hdlist_for_each_entry_safe(ep_info, hinfo, t) list_for_each_entry_safe(hinfo, t, &(ep_info)->hlist, hlist)

#define dmalist_binfo_move_tail(ep_info, binfo) list_move_tail(&(binfo)->blist, &(ep_info)->dmalist)
#define dmalist_binfo_read_head(ep_info) list_entry((ep_info)->dmalist.next, struct buf_info_t, blist)
#define dmalist_binfo_for_each_entry_safe(ep_info, binfo, t) list_for_each_entry_safe(binfo, t, &(ep_info)->dmalist, blist)
#define dmalist_hinfo_move_tail(ep_info, hinfo) list_move_tail(&(hinfo)->hlist, &(ep_info)->dmalist)
#define dmalist_hinfo_read_head(ep_info) list_entry((ep_info)->dmalist.next, struct head_info_t, hlist)
#define dmalist_hinfo_for_each_entry_safe(ep_info, hinfo, t) list_for_each_entry_safe(hinfo, t, &(ep_info)->dmalist, hlist)
#define dmalist_is_empty(ep_info) list_empty(&(ep_info)->dmalist)

#define binfo_next(binfo) list_entry((binfo)->blist.next, struct buf_info_t, blist)
#define hinfo_blist_move_tail(hinfo, binfo) list_move_tail(&(binfo)->blist, &(hinfo)->blist)
#define hinfo_blist_read_head(hinfo) list_entry((hinfo)->blist.next, struct buf_info_t, blist)
#define hinfo_blist_read_tail(hinfo) list_entry((hinfo)->blist.prev, struct buf_info_t, blist)
#define hinfo_blist_is_empty(hinfo) list_empty(&(hinfo)->blist)
#define hinfo_blist_for_each_entry_safe(hinfo, binfo, t) list_for_each_entry_safe(binfo, t, &(hinfo)->blist, blist) 
#define hinfo_blist_for_each_entry(hinfo, binfo) list_for_each_entry(binfo, &(hinfo)->blist, blist) 
#define hinfo_blist_for_each_entry_reverse(hinfo, binfo) list_for_each_entry_reverse(binfo, &(hinfo)->blist, blist) 
#define hinfo_init(hinfo) \
    do { \
        /*INIT_LIST_HEAD(&(hinfo)->hlist);*/ \
        INIT_LIST_HEAD(&(hinfo)->blist); \
        (hinfo)->buf = NULL; \
        (hinfo)->length = 0; \
        (hinfo)->binfo_used = 0; \
        (hinfo)->urb->transfer_buffer = NULL; \
        (hinfo)->urb->transfer_buffer_length = 0; \
        (hinfo)->urb->actual_length = 0; \
    } while (0)

#if DEV_FSPD_WEBCAM
	#define binfo_init(binfo) \
	    do { \
	        (binfo)->desc = NULL; \
	        (binfo)->actual_length = 0; \
	        (binfo)->status = 0; \
	        (binfo)->split_info = NULL; \
	        (binfo)->split_cnt = 0; \
	    } while (0)
#else
	#define binfo_init(binfo) \
	    do { \
	        (binfo)->desc = NULL; \
	        (binfo)->actual_length = 0; \
	        (binfo)->status = 0; \
	        (binfo)->split_info = NULL; \
	    } while (0)
#endif

#define reqlist_move_tail(ep_info, hinfo) list_move_tail(&(hinfo)->hlist, &(ep_info)->reqlist)
#define reqlist_read_head(ep_info)  list_entry((ep_info)->reqlist.next, struct head_info_t, hlist)
#define reqlist_is_empty(ep_info) list_empty(&(ep_info)->reqlist)
#define reqlist_for_each_entry_safe(ep_info, hinfo, t) list_for_each_entry_safe(hinfo, t, &(ep_info)->reqlist, hlist) 

#define rdylist_move_tail(ep_info, hinfo) list_move_tail(&(hinfo)->hlist, &(ep_info)->rdylist)
#define rdylist_read_head(ep_info)  list_entry((ep_info)->rdylist.next, struct head_info_t, hlist)
#define rdylist_is_empty(ep_info) list_empty(&(ep_info)->rdylist)
#define rdylist_for_each_entry_safe(ep_info, hinfo, t) list_for_each_entry_safe(hinfo, t, &(ep_info)->rdylist, hlist) 


#define spin_lock_on(plock, flags) spin_lock_irqsave((plock), (flags))
#define spin_lock_off(plock, flags) spin_unlock_irqrestore((plock), (flags))

#define llock_init(ep_info) spin_lock_init(&(ep_info)->lock)
#define llock_on(ep_info, flags) spin_lock_on(&(ep_info)->lock, flags)
#define llock_off(ep_info, flags) spin_lock_off(&(ep_info)->lock, flags)


/*
// OUT:
// enq()
// {
//     wp = GET_WP();
//     crp = ep_info->crp;
//     dlist = ep_info->dlist + wp;
//     blist = &(ep_info->blist);
//     dmalist = &ep_info->dmalist;
//     wp += MAX_STAGES; crp += MAX_STAGES - 1;
//     if (wp > crp)
//        crp += MAX_STAGES;
//     while (wp != crp && !list_empty(blist))
//     {
//         binfo = list_entry(blist->next, struct buf_info_t, blist);
//         dlist->baddr = binfo->baddr;
//         dlist->ctrl = BIT31;
//         binfo->desc = dlist;
//         list_move_tail(&binfo->blist, dmalist);
//         wp++;
//         dlist = ep_info->dlist + WRAP_IDX(wp);
//     }
//     SET_WP(WRAP_IDX(wp));
//  }
//
// ACK_interrupt()
// {
//
//     hlist = &(ep_info->hlist);
//     dmalist = &(ep_info->dmalist);
//     if (list_empty(hlist) || list_empty(dmalist)) {
//         DON'T clear int status;
//         return;
//     }
//
//     rp = GET_RP();
//     crp = ep_info->crp;
//     if (crp > rp)
//        rp += MAX_STAGES;
//
//     hinfo = list_entry(hlist->next, struct head_info_t, hlist);
//     binfo = list_entry(dmalist->next, struct buf_info_t, blist);
//     xfer_buf = binfo->buf;
//     while (crp != rp && !list_empty(dmalist)) {
//         crp++;
//         list_move_tail(&binfo->blist, &hinfo->blist);
//         len = binfo->desc->length;
//
//         hinfo->length += len;
//         if (list_empty(dmalist) || crp == rp) {
//             u = TRUE;
//         } else {
//             binfo_n = list_entry(dmalist->next, struct buf_info_t, blist);
//             if (binfo->buf + max_pkt_size != binfo_n->buf) { //not continue memory
//                 u = TRUE;
//             } else if (len < max_pkt_size) { end of OUT
//                 u = TRUE;
//             } else if (ep_info->onebyone) {
//                 u = TRUE;
//             } else if (hinfo->length > 3584) {
//                 u = TRUE;
//             }
//             binfo = binfo_n;
//         }
// 
//         if (u) {
//             u = FALSE;
//             hinfo->buf = xfer_buf;
//             xfer_buf = binfo->buf;
//             list_move_tail(&hinfo->hlist, &ep_info->reqlist);
//             submit_urb_OUT(hinfo);
//             if (list_empty(hlist))
//                 break;
//             hinfo = list_entry(hlist->next, struct head_info_t, hlist);
//         }
//     }
//     ep_info->crp = crp;
//
//     if (empty flag ON)
//        enq();
//
//     if (crp != rp)
//        return but not clear status;
//
// }
//
// cb_OUT(urb)
//    {
//        ep_info = urb->context;
//        reqlist = &ep_info->reqlist;
//        BUG_ON(list_empty(reqlist));
//        hinfo = list_entry(reqlist->next, struct head_info_t, hlist);
//        BUG_ON(hinfo->urb != urb);
//        blist = &hinfo->blist;
//
//        INIT_LIST_HEAD(&hinfo->blist);
//        hinfo->buf = NULL;
//        hinfo->length = 0;
//        hinfo->urb->transfer_buf = NULL
//        hinfo->urb->transfer_length = 0;
//        hinfo->urb->actual_length = 0;
//        list_move_tail(&hinfo->hlist, &ep_info->hlist);
//
//        list_for_each_entry_safe(binfo, tmpbinfo, blist, blist) {
//            binfo->desc = NULL;
//            list_move_tail(&binfo->blist, &ep_info->blist);
//        }
//
//        enq();
//    }
//
//
// IN
// NAK int()
//    {
//        mps = ep_info->max_pkt_size;
//        len = 0;
//        DISABLE_NAK_INT();
//        hlist = &ep_info->hlist;
//        blist = &ep_info->blist;
//        if (list_empty(hlist) || list_empty(blist)) {
//            //don't clear status;
//            return;
//        }
//        hinfo = list_entry(hlist->next, struct head_info_t, hlist);
//        if (ep_info->onebyone) {
//            max = mps;
//        } else {
//            max = mps << 3;
//        }
//
//        binfo = list_entry(blist->next, struct buf_info_t, blist);
//        hinfo->buf = binfo->buf;
//        do {
//            len += mps;
//            list_move_tail(&binfo->blist, &hinfo->blist);
//            if (list_empty(blist))
//                break;
//            
//            binfo_n = list_entry(blist->next, struct buf_info_t, blist);
//            if (binfo->buf + mps != binfo_n->buf)
//                break;
//
//            binfo = binfo_n;
//        } while (len == max);
//        hinfo->length = len;
//        list_move_tail(&hinfo->hlist, &ep_info->reqlist);
//        submit_urb_IN(hinfo);
//    }
cb_IN(urb)
{
    ep_info = urb->context;
    reqlist = &ep_info->reqlist;
    BUG_ON(list_empty(reqlist));

    if (urb->status)
        stall();

    wp = GET_WP();
    crp = ep_info->crp;
    dlist = ep_info->dlist + wp;
    dmalist = &ep_info->dmalist;
    hinfo = list_entry(reqlist->next, struct head_info_t, hlist);
    blist = &hinfo->blist;
    freeblist = &ep_info->blist;

    wp += MAX_STAGES; 
    crp += MAX_STAGES - 1;
    if (wp > crp)
        crp += MAX_STAGES;

    actual_len = urb->actual_length;
    mps = ep_info->max_pkt_size;
    list_for_each_entry(binfo, blist, blist) {
        if (actual_len) {
            while (wp == crp) { //Make sure we can kick to dma
                yield();
                free_dma_buf_IN(ep_info);
                crp = ep_info->crp;
                crp += MAX_STAGES - 1;
                if (wp > crp)
                    crp += MAX_STAGES;
                continue;
            }
            dlist->baddr = binfo->baddr;
            if (actual_len < mps) {
                dlist->ctrl = actual_len;
                actual_len = 0;
            } else {
                dlist->ctrl = mps;
                actual_len -= mps;
            }
            binfo->desc = dlist;
            wp++;
            dlist = ep_info->dlist + WRAP_IDX(wp);
        } else {
            binfo->desc = NULL; //double make sure;
        }
    }

    if (!urb->status) {
        SET_WP(WRAP_IDX(wp));
    }

    list_move_tail(&hinfo->hlist, dmalist);

    free_dma_buf_IN(ep_info);
}

free_dma_buf_IN(ep_info)
{
    rp = GET_RP();
    crp = ep_info->rp;
    dmalist = ep_info->dmalist;
    if (list_empty(dmalist) || rp == crp);
        return;

    dlist = ep_info->dlist + crp;
    
    if (crp > rp)
        rp += MAX_STAGES;

    hinfo = list_entry(dmalist->next, struct head_info_t, hlist);
    blist = &hinfo->blist;

    do {
        if (list_empty(blist)) {
            INIT_LIST_HEAD(&hinfo->blist);
            hinfo->buf = NULL;
            hinfo->length = 0;
            hinfo->urb->transfer_buf = NULL
            hinfo->urb->transfer_length = 0;
            hinfo->urb->actual_length = 0;
            list_move_tail(&hinfo->hlist, &ep_info->hlist);

            if (list_empty(dmalist))
                break;
            hinfo = list_entry(dmalist->next, struct head_info_t, hlist);
            blist = &hinfo->blist;
        }

        binfo = list_entry(blist->next, struct buf_info_t, blist);
        if (binfo->desc == dlist) {
            binfo->desc = NULL;
            crp++;
            dlist = ep_info->dlist + WRAP_IDX(crp);
        } else if (binfo->desc == NULL) {
            //fine
        } else {
            BUG();
        }
        list_move_tail(&binfo->blist, &ep_info->blist);
        
    } while (crp != rp);
    
    ep_info->crp = WRAP_IDX(crp);

}
*/

void dump_dlist(struct ep_info_t *ep_info);
int ep_attach(struct vhub_device *vdev);
//int ep_detach(struct vhub_device *vdev);
int ep_enable_interface(struct vhub_device *vdev, struct usb_interface *intf);
void ep_detach_from_interface(struct vhub_device *vdev, struct usb_interface *intf);
bool ep_handle_ack_int(struct vhub_hcd *vhub, struct ep_info_t *ep_info);
bool ep_handle_nack_int(struct vhub_hcd *vhub, struct ep_info_t *ep_info);
void ep_halt(struct vhub_hcd *vhub);
/* return the remote ep addr of the vdev's local ep addr */
u16 ep_get_remote_ep_addr(struct vhub_device *vdev, u32 vaddr);
u16 ep_get_local_ep_addr(struct vhub_device *vdev, u16 remote_addr);
void ep_clear_halt(struct vhub_device *vdev, u32 vaddr);
void intf_notify_kill(struct vhub_device *vdev, struct intf_info_t *intf_info);
struct intf_info_t *find_matched_active_intf_info(struct vhub_hcd* vhub, struct usb_interface *intf);
#endif
