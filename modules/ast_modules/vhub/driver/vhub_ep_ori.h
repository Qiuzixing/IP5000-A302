
#ifndef __VHUB_EP_H__
#define __VHUB_EP_H__

#define MAX_STAGES 32 /*HW fixed vaule. Must be power of 2*/
#define MAX_PEP_NUM 21

struct ep_dma_desc_t {
    volatile __le32 buf_addr; //bit[3:27] 8 bytes aligned
    volatile __le32 ctrl;
} __attribute__ ((packed)); 

#define OWN_CPU             0 //own by cpu/idle, free to use
#define OWN_VHUB           1 //own by vhub/dma, 
#define OWN_URB             2 //own by urb, sumbit to remote
#define OWN_URB_END    4 //own by urb, sumbit to remote and is the last piece of urb buffer
#define set_own(ep_info, bidx, value) atomic_set(&((ep_info)->blist[bidx].own), value)
#define get_own(ep_info, bidx) atomic_read(&((ep_info)->blist[bidx].own))
struct buf_info_t {
    unsigned char *buf; //buffer virtual addr (base)
    dma_addr_t baddr; //buffer dma addr
    /* An Own flag to indicate the owner of the buffer */
    atomic_t own;
};

#define EP_NUMBER(reg)     (((reg) & 0x00000F00) >> 8)
#define EP_TYPE(reg)          (((reg) & 0x00000070) >> 4)
#define EP_DEV_PORT(reg)  (((reg) & 0x0000000E) >> 1)
#define EP_TYPE_OUT(reg)  ((EP_TYPE(reg)) & BIT0)
#define EP_TYPE_DISABLE 0
#define EP_TYPE_BULK_IN 2
#define EP_TYPE_BULK_OUT 3
#define EP_TYPE_INT_IN 4
#define EP_TYPE_INT_OUT 4
#define EP_TYPE_ISO_IN 6
#define EP_TYPE_ISO_OUT 7

struct ep_info_t {
    struct vhub_device *vdev;
    /* ep desc from remote */
    struct usb_endpoint_descriptor *desc;

    //The ep addr of remote
    u8 ep_addr;
    // max pkt size of remote ep
    u16 max_pkt_size;
    // Interval. Used for urb submit.
    int interval;

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

    struct buf_info_t *blist; //the memory space holds the list of struct buf_info_t{}
    u16 blist_size; //blist arrary's size (the element number of blist) MUST be order of 2
    
    u16 ri; //cpu rd index for dma from 0 to (blist_size - 1). ONLY updated in int. and initial.
    u16 wi; //cpu write index for dma from 0 to (blist_size -1). ONLY updated in cb. and initial.
    u16 fi; //the index to free the buffer's own from 3/4 back to 0. ONLY updated in cb. and initial.
    u16 crp; //the index of cpu rp from 0 to (MAX_STAGES - 1)

    bool onebyone; //If TRUE, use requets can only be sent one usb pkt at a time
    bool stall; // false: normal true: this ep is stalled
    u32 max;
};

int ep_attach(struct vhub_device *vdev);
void ep_handle_ack_int(struct vhub_hcd *vhub, struct ep_info_t *ep_info);
void ep_handle_nack_int(struct vhub_hcd *vhub, struct ep_info_t *ep_info);
void ep_halt(struct vhub_hcd *vhub);
/* return the remote ep addr of the vdev's local ep addr */
u16 ep_get_remote_ep_addr(struct vhub_device *vdev, u32 vaddr);
u16 ep_get_local_ep_addr(struct vhub_device *vdev, u32 remote_addr);
void ep_clear_halt(struct vhub_device *vdev, u32 vaddr);
#endif
