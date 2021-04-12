/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
/*
 * Some code referenced from Linux kernl 4.14.4.
 * drivers/usb/storage/uas.c
 * drivers/usb/storage/uas-detect.h
 */

#include <linux/usb.h>
#include <linux/usb_ch9.h>

#if 0
#define uasdbg(fmt, args...) \
    do { \
        printk(fmt, ## args); \
    } while (0)
#else
#define uasdbg(fmt, args...) do {} while(0)
#endif


/* include/scsi/scsi_cmnd.h */
#define SCSI_SENSE_BUFFERSIZE 	96

#define USB_SC_SCSI     0x06            // Transparent SCSI
#define USB_PR_UAS      0x62            // UASP

/* From new kernel include/uapi/linux/usb/ch9.h */
#define USB_DT_PIPE_USAGE            0x24
#define USB_ENDPOINT_NUMBER_MASK     0x0f	/* in bEndpointAddress */
#define USB_ENDPOINT_DIR_MASK        0x80


/* From Linux driver::uas.c::devinfo->qdepth and Win10 CrystalDiskMark Test.
** USB 2.0 UASP Command queue depth is limited up to 32.
*/
#define UAS_CMND_QUEUE_DEPTH 32 //Must be 2^n

typedef struct {
	u32 tag;
	u32 blocks; /* 0:invalid, -1: means one by one barrier command. Otherwise, means mps blocks */
} cmnd_queue_t;

typedef struct {
	u32 tag; /* 0:not-tagged, 1 ~ (UAS_CMND_QUEUE_DEPTH-1) means has this tag. */
	u32 scsi_op; /* op code. Actually only 8bits long. */
	u32 blocks; /* 0:invalid, -1: means one by one barrier command. Otherwise, means mps blocks */
} tagged_cmnd_t;

struct uas_dev {
	u8 cmnd_addr; //local cmnd ep addr
	u8 status_addr; //local status ep addr
	u8 data_in_addr; //local data in ep addr
	u8 data_out_addr; //local data out ep addr
	u32 cmndq_wr_ptr; //cmd incoming. Write into command queue.
	u32 cmndq_rd_ptr; //driver ep IN consumes. Read from command queue.
	void *data_in_ep_info; // Used to enable data IN ep NAK interrupt.
	cmnd_queue_t cmnd_queue[UAS_CMND_QUEUE_DEPTH];
	tagged_cmnd_t tagged_cmnds[UAS_CMND_QUEUE_DEPTH]; // Maps to (iu->tag - 1). Will be copied/enqueued into cmnd_queue[] if got RRIU.
};


enum {
	IU_ID_COMMAND		= 0x01,
	IU_ID_STATUS		= 0x03,
	IU_ID_RESPONSE		= 0x04,
	IU_ID_TASK_MGMT		= 0x05,
	IU_ID_READ_READY	= 0x06,
	IU_ID_WRITE_READY	= 0x07,
};

enum {
	RC_TMF_COMPLETE         = 0x00,
	RC_INVALID_INFO_UNIT    = 0x02,
	RC_TMF_NOT_SUPPORTED    = 0x04,
	RC_TMF_FAILED           = 0x05,
	RC_TMF_SUCCEEDED        = 0x08,
	RC_INCORRECT_LUN        = 0x09,
	RC_OVERLAPPED_TAG       = 0x0a,
};

struct scsi_lun {
	__u8 scsi_lun[8];
};

struct command_iu {
	__u8 iu_id; //== IU_ID_COMMAND = 0x01
	__u8 rsvd1;
	__be16 tag;
	__u8 prio_attr;
	__u8 rsvd5;
	__u8 len;
	__u8 rsvd7;
	struct scsi_lun lun;
	__u8 cdb[16];    /* XXX: Overflow-checking tools may misunderstand */
} __attribute__((__packed__));

/* Common header for all IUs */
struct iu {
	__u8 iu_id;
	__u8 rsvd1;
	__be16 tag;
} __attribute__((__packed__));

struct sense_iu {
	__u8 iu_id;
	__u8 rsvd1;
	__be16 tag;
	__be16 status_qual;
	__u8 status;
	__u8 rsvd7[7];
	__be16 len;
	__u8 sense[SCSI_SENSE_BUFFERSIZE];
} __attribute__((__packed__));

struct response_iu {
	__u8 iu_id;
	__u8 rsvd1;
	__be16 tag;
	__u8 add_response_info[3];
	__u8 response_code;
} __attribute__((__packed__));

/**
 * usb_endpoint_num - get the endpoint's number
 * @epd: endpoint to be checked
 *
 * Returns @epd's number: 0 to 15.
 */
/* Linux::include/uapi/linux/usb/ch9.h::usb_endpoint_num() */
static inline int usb_endpoint_addr(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & (USB_ENDPOINT_NUMBER_MASK | USB_ENDPOINT_DIR_MASK);
}


static inline unsigned _sccmndq_idx_next(u32 idx)
{
	return ((idx + 1) & (UAS_CMND_QUEUE_DEPTH - 1));
}

static inline unsigned _sccmndq_is_full(struct uas_dev *uas)
{
	u32 next_wr_idx = _sccmndq_idx_next(uas->cmndq_wr_ptr);

	if (next_wr_idx == uas->cmndq_rd_ptr)
		return 1;

	return 0;
}

static inline unsigned _sccmndq_is_empty(struct uas_dev *uas)
{
	if (uas->cmndq_wr_ptr == uas->cmndq_rd_ptr)
		return 1;

	return 0;
}

static inline unsigned _sccmndq_enqueue(struct uas_dev *uas, u32 blocks, u16 tag)
{
	u32 next_wr_idx = _sccmndq_idx_next(uas->cmndq_wr_ptr);
	cmnd_queue_t *slot;

	/* Check if sclist is full. */
	if (next_wr_idx == uas->cmndq_rd_ptr)
		return -1;

	slot = &(uas->cmnd_queue[uas->cmndq_wr_ptr]);
	slot->tag = tag;
	slot->blocks = blocks;

	uas->cmndq_wr_ptr = next_wr_idx;
	return 0;
}

static inline unsigned _sccmndq_get_rd_blks(struct uas_dev *uas)
{
	if (_sccmndq_is_empty(uas))
		return 0;

	return uas->cmnd_queue[uas->cmndq_rd_ptr].blocks;
}

static inline unsigned _sccmndq_get_rd_tag(struct uas_dev *uas)
{
	if (_sccmndq_is_empty(uas))
		return 0; // tag value can't be 0

	return uas->cmnd_queue[uas->cmndq_rd_ptr].tag;
}

static inline unsigned _sccmndq_consume_rd_blks(struct uas_dev *uas, u32 blocks)
{
	cmnd_queue_t *slot = &(uas->cmnd_queue[uas->cmndq_rd_ptr]);
	u32 cur_blocks = slot->blocks;

	BUG_ON(cur_blocks < blocks);

	uasdbg("[%d]\n", blocks);

	cur_blocks -= blocks;

	slot->blocks = cur_blocks;

	/* If all blocks consumed, to next command. */
	if (cur_blocks == 0) 
		uas->cmndq_rd_ptr = _sccmndq_idx_next(uas->cmndq_rd_ptr);

	return cur_blocks;
}

static inline void _sccmndq_cancel(struct uas_dev *uas, u16 tag)
{
	cmnd_queue_t *slot;

	/*
	** BruceFIXME. I assume only the first cmnd will be cancelled.
	** Not sure parallel cancel is possible or not.
	** If it happens, we need to delete a command from middle of the dequeue.
	*/
	/* BruceFIXME. protect cmnd_queue? */
	if (_sccmndq_get_rd_tag(uas) != tag)
		return;

	/* Remove this command. And de-queue it. */
	slot = &(uas->cmnd_queue[uas->cmndq_rd_ptr]);
	slot->blocks = 0;
	slot->tag = 0;

	uas->cmndq_rd_ptr = _sccmndq_idx_next(uas->cmndq_rd_ptr);
}

static int uas_is_interface(struct usb_host_interface *intf)
{
	return (intf->desc.bInterfaceClass == USB_CLASS_MASS_STORAGE &&
		intf->desc.bInterfaceSubClass == USB_SC_SCSI &&
		intf->desc.bInterfaceProtocol == USB_PR_UAS);
}

static int uas_find_endpoints(struct usb_host_interface *alt,
			      struct usb_host_endpoint *eps[])
{
	struct usb_host_endpoint *endpoint = alt->endpoint;
	unsigned i, n_endpoints = alt->desc.bNumEndpoints;

	/*
	** Extra format.
	** Extra[0]: length in bytes
	** Extra[1]: Type: USB_DT_PIPE_USAGE (0x24)
	** Extra[2]: ID: 1:cmd, 2:status, 3:data IN, 4:data OUT.
	*/
	for (i = 0; i < n_endpoints; i++) {
		unsigned char *extra = endpoint[i].extra;
		int len = endpoint[i].extralen;
		while (len >= 3) {
			if (extra[1] == USB_DT_PIPE_USAGE) {
				unsigned pipe_id = extra[2];
				if (pipe_id > 0 && pipe_id < 5)
					eps[pipe_id - 1] = &endpoint[i];
				break;
			}
			len -= extra[0];
			extra += extra[0];
		}
	}

	if (!eps[0] || !eps[1] || !eps[2] || !eps[3])
		return -ENODEV;

	return 0;
}

static inline void tagcmnd_del(struct uas_dev *uas, u16 tag)
{
	tagged_cmnd_t *cmnd = uas->tagged_cmnds;
	u32 idx = tag - 1; // SCSI tag counts from 1.

	BUG_ON(idx >= UAS_CMND_QUEUE_DEPTH);

	cmnd += idx;

	if (cmnd->tag == 0)
		uerr("no cmnd to delete?! %d\n", tag);

	cmnd->tag = 0;
	cmnd->scsi_op = 0;
	cmnd->blocks = 0;
}

static inline void tagcmnd_set_barrier(struct uas_dev *uas, u16 tag)
{
	tagged_cmnd_t *cmnd = uas->tagged_cmnds;
	u32 idx = tag - 1; // SCSI tag counts from 1.

	BUG_ON(idx >= UAS_CMND_QUEUE_DEPTH);

	cmnd += idx;
	if (cmnd->tag != tag) {
		uerr("WTF tag?! %d:%x\n", cmnd->tag, cmnd->scsi_op);
	}

	cmnd->blocks = -1;
}

static inline void tagcmnd_reg(struct uas_dev *uas, u16 tag, u32 op, u32 blocks)
{
	tagged_cmnd_t *cmnd = uas->tagged_cmnds;
	u32 idx = tag - 1; // SCSI tag counts from 1.

	BUG_ON(idx >= UAS_CMND_QUEUE_DEPTH);

	cmnd += idx;
	if (cmnd->tag) {
		uerr("Duplicated command tag?! %d:%x,%d\n", tag, cmnd->scsi_op, blocks);
		//BruceTODO. remove queued cmnd.
	}

	cmnd->tag = tag;
	cmnd->scsi_op = op;
	cmnd->blocks = blocks;
}

static inline unsigned uas_get_rd_blks(struct vhub_device *vdev)
{
	return _sccmndq_get_rd_blks(vdev->uas);
}

static inline unsigned uas_consume_rd_blks(struct vhub_device *vdev, u32 blocks)
{
	return _sccmndq_consume_rd_blks(vdev->uas, blocks);
}

static inline void _del_scsi_cmd(struct uas_dev *uas, u16 tag)
{
	tagcmnd_del(uas, tag);
	_sccmndq_cancel(uas, tag);
}

#define UASP_PARSE_ALL_SCSI_CMND 0

static void uas_parse_scsi_cmd(struct vhub_device *vdev, struct ep_info_t *ep_info)
{
	struct uas_dev *uas = vdev->uas;
	struct head_info_t *hinfo;
	struct command_iu *ciu;

	if (hdlist_is_empty(ep_info))
		return;

	/* Only check Cmnd out PIPE. */
	if (ep_info->ep_addr_local != 4)
		return;

	hinfo = hdlist_read_head(ep_info);
	if (hinfo->buf) {
#if DOUBLE_CHK
		if (!hinfo->binfo_used) {
			uerr("FATAL!\n");
			BUG();
		}
#endif
		ciu = (struct command_iu *)hinfo->buf;
		/*
		** From linux::uas.c::uas_alloc_cmd_urb()
		** IU_ID_COMMAND == 0x01
		** UAS_SIMPLE_TAG == 0x00
		*/
		if ((ciu->iu_id == IU_ID_COMMAND) && (ciu->prio_attr == 0x00)) {
			char *cdb = ciu->cdb;
			u32 blks = 0;
			u8 op = cdb[0];

#if UASP_PARSE_ALL_SCSI_CMND
			u32 alen = 0;
			u32 mps = ep_info->max_pkt_size;
#endif
			/* See SCSI command reference for read 6/10/12/16/32 bytes command format. */
			switch(op) {
				case 0x08: //read 6 bytes
					if (cdb[4] == 0)
						blks = 256;
					else
						blks = cdb[4];
					break;
				case 0x28: //read 10 bytes
					blks = be16_to_cpu(*((u16 *)(cdb + 7)));
					break;
				case 0xA8: //read 12 bytes
					blks = be16_to_cpu(*((u16 *)(cdb + 6)));
					break;
				case 0x88: //read 16 bytes
					blks = be16_to_cpu(*((u16 *)(cdb + 10)));
					break;
				case 0x7F: //READ (32) and others.
					{
						u16 service_action = be16_to_cpu(*((u16 *)(cdb + 8)));

						switch(service_action) {
						case 0x09: //READ (32)
							blks = be16_to_cpu(*((u16 *)(cdb + 28)));
							break;
						case 0x0A: //VERIFY (32) (out)
						case 0x0B: //WRITE (32) (out)
						case 0x0C: //WRITE AND VERIFY (32) (out)
						case 0x0D: //WRITE SAME (32) (out)
							break;
						}
					}
					break;

#if UASP_PARSE_ALL_SCSI_CMND
				case 0x12: //INQUIRY
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 3)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:INQUIRY:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0xA0: //REPORT LUNS
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 5)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:REPORT LUNS:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x1A: //MODE SENSE (6)
					// use allocation length in bytes
					alen = *((u8 *)(cdb + 4));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:MODE SENSE:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x9E: //READ CAPACITY (10).and READ LONG (16)
					{
						u8 service_action = cdb[1] & 0x1F;

						switch(service_action) {
						case 0x10: //READ CAPACITY (10)
							// use allocation length in bytes
							alen = be16_to_cpu(*((u16 *)(cdb + 10)));
							blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
							udbg("%d:READ CAPACITY:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
							break;
						case 0x11: //READ LONG (16)
							// use BYTE TRANSFER LENGTH in bytes
							alen = be16_to_cpu(*((u16 *)(cdb + 12)));
							blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
							udbg("%d:READ LONG (16):%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
							break;
						}
					}
					break;
				case 0x25: //READ CAPACITY. Read 8 bytes of data.
					blks = 1;
					udbg("%d:READ CAPACITY 8:%d -> %d\n", be16_to_cpu(ciu->tag), 8, blks);
					break;

				/* Following op code from spec, but never seen. */
				case 0x4D: //LOG SENSE (in)
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 7)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:LOG SENSE:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x5A: //MODE SENSE (10)
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 7)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:MODE SENSE 10:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x5E: //PERSISTENT RESERVE IN
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 7)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:PERSISTENT RESERVE IN:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x3C: //READ BUFFER
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 5)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:READ BUFFER:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x37: //READ DEFECT DATA (10)
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 7)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:READ DEFECT DATA (10):%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0xB7: //READ DEFECT DATA (12)
					// use allocation length in bytes
					alen = be32_to_cpu(*((u16 *)(cdb + 6)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:READ DEFECT DATA (12):%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x3E: //READ LONG (10)
					// use BYTE TRANSFER LENGTH in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 7)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:READ LONG (10):%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x1C: //RECEIVE DIAGNOSTIC RESULTS
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 3))); //Not sure. Spec not clear.
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:RECEIVE DIAGNOSTIC RESULTS:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0xA3: //REPORT DEVICE IDENTIFIER
					// use allocation length in bytes
					alen = be16_to_cpu(*((u16 *)(cdb + 6)));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:REPORT DEVICE IDENTIFIER:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;
				case 0x03: //REQUEST SENSE
					// use allocation length in bytes
					alen = *((u8 *)(cdb + 4));
					blks = (alen > mps) ? ((alen + mps - 1) / mps) : (1);
					udbg("%d:REQUEST SENSE:%d -> %d\n", be16_to_cpu(ciu->tag), alen, blks);
					break;


				/* Following op code was seen, but no in xfer. */
				case 0x2A: //WRITE (10) (out)
				case 0x35: //SYNCHRONIZE CACHE (10)
				case 0x00: //TEST UNIT READY
				case 0x1B: //START STOP UNIT

				/* Following op code from spec, but never seen. */
				case 0x40: //CHANGE DEFINITION
				case 0x04: //FORMAT UNIT (out)
				case 0x4C: //LOG SELECT (out)
				case 0x15: //MODE SELECTION (6) (out)
				case 0x55: //MODE SELECTION (10) (out)
				case 0x5F: //PERSISTENT RESERVE OUT (out)
				case 0x07: //REASSIGN BLOCKS (out)
				case 0x17: //RELEASE (6) (out)
				case 0x57: //RELEASE (10) (out)
				case 0x16: //RESERVE(6) (out)
				case 0x56: //RESERVE(10) (out)
				case 0x01: //REZERO UNIT (out)
				case 0x0B: //SEEK (out)
				case 0x2B: //SEEK EXTENDED (out)
				case 0x1D: //SEND DIAGNOSTIC (out)
				case 0xA4: //SET DEVICE IDENTIFIER (out)
				case 0x91: //SYNCHRONIZE CACHE (16) (out)
				case 0x2F: //VERIFY (10) (out)
				case 0xAF: //VERIFY (12) (out)
				case 0x8F: //VERIFY (16) (out)
				case 0x0A: //WRITE (6) (out)
				case 0xAA: //WRITE (12) (out)
				case 0x8A: //WRITE (16) (out)
				case 0x2E: //WRITE AND VERIFY (10) (out)
				case 0xAE: //WRITE AND VERIFY (12) (out)
				case 0x8E: //WRITE AND VERIFY (16) (out)
				case 0x3B: //WRITE BUFFER (out)
				case 0x3F: //WRITE LONG (10) (out)
				case 0x9F: //WRITE LONG (16) (out)
				case 0x41: //WRITE SAME (10) (out)
				case 0x93: //WRITE SAME (16) (out)
					//udbg("%d:op%02X\n", be16_to_cpu(ciu->tag), op);
					break;
				default:
					udbg("?%d:op%02X\n", be16_to_cpu(ciu->tag), op);
#endif
			}
			uasdbg("%d:op%02x %d\n", be16_to_cpu(ciu->tag), op, blks);
			tagcmnd_reg(uas, be16_to_cpu(ciu->tag), op, blks);
		}
	}
}

static inline u32 _enqueue_scsi_cmd(struct uas_dev *uas, u16 tag)
{
	u32 idx = tag - 1;
	u32 blocks = uas->tagged_cmnds[idx].blocks;

	if (blocks == 0)
		return 0;

	uasdbg("Q:%d,%d\n", tag, uas->tagged_cmnds[idx].blocks);

	/* Move cmnd block from tagged_cmnds[tag - 1].blocks into sccmndq. */
	_sccmndq_enqueue(uas, blocks,  tag);

	/* tagged cmnd will be dequeued when receiving status info. */
	//tagcmnd_del(uas, tag);

	return blocks;
}

static inline int uas_evaluate_response_iu(struct response_iu *riu)
{
	u8 response_code = riu->response_code;

	switch (response_code) {
	case RC_INCORRECT_LUN:
		//cmnd->result = DID_BAD_TARGET << 16;
		break;
	case RC_TMF_SUCCEEDED:
		//cmnd->result = DID_OK << 16;
		break;
	case RC_TMF_NOT_SUPPORTED:
		//cmnd->result = DID_TARGET_FAILURE << 16;
		break;
	case RC_TMF_COMPLETE:
		//??
		break;
	default:
		uerr("response iu: %d\n", response_code);
		//cmnd->result = DID_ERROR << 16;
		break;
	}

	return ((response_code == RC_TMF_SUCCEEDED) || (response_code == RC_TMF_COMPLETE));
}

static inline void ep_IN_ready(struct ep_info_t * ep_info);

static void uas_stat_cmplt(struct vhub_device *vdev, struct urb *urb)
{
	struct uas_dev *uas = vdev->uas;
	struct iu *iu = urb->transfer_buffer;
	unsigned int idx, tag;
	int status = urb->status;

	if (status) {
		if (status != -ENOENT && status != -ECONNRESET && status != -ESHUTDOWN)
			uerr("stat urb: status %d\n", status);
		goto out;
	}

	tag = (unsigned int)be16_to_cpu(iu->tag);
	idx = tag - 1;
	if (idx >= UAS_CMND_QUEUE_DEPTH) {
		uerr("stat urb: invalid uas-tag %d?!\n", tag);
		goto out;
	}

	switch (iu->iu_id) {
	case IU_ID_STATUS: /* xfer completed status. */
		{
			struct sense_iu *sense_iu = urb->transfer_buffer;
			if (sense_iu->status != 0) {
				/* cancel data transfers on error */
				uerr("IU_ID_STATUS error?!\n");
			}
			uasdbg("%d:status %d\n", tag, sense_iu->status);
			_del_scsi_cmd(uas, tag);
		}
		break;
	case IU_ID_READ_READY:
		{
			tagged_cmnd_t *cmnd = &uas->tagged_cmnds[idx];

			if (!cmnd->tag) {
				uerr("stat urb: no pending cmd for uas-tag %d\n", tag);
				/* Try workaround it using op 0. It should be a BUG(). */
				tagcmnd_reg(uas, tag, 0, -1);
			} else if (!cmnd->blocks) {
				uasdbg("stat urb: unknown pending cmd for uas-tag %d, op:%02x\n", tag, cmnd->scsi_op);
				/*
				** For unknown commands, we use "blocks == -1" as barrier.
				** If driver see "blocks == -1", it will acts as one by one mode.
				*/
				tagcmnd_set_barrier(uas, tag);
			}
			if (_enqueue_scsi_cmd(uas, tag))
				ep_IN_ready(uas->data_in_ep_info);
		}
		break;
	case IU_ID_WRITE_READY:
		/* Don't need to handle write case. */
		break;
	case IU_ID_RESPONSE:
		if (!uas_evaluate_response_iu((struct response_iu *)iu)) {
			/* Error, cancel data transfers */
			uerr("IU_ID_RESPONSE error?!\n");
		}
		uasdbg("%d:resp\n", tag);
		_del_scsi_cmd(uas, tag);
		break;
	default:
		uerr("bogus IU!? %d\n", iu->iu_id);
	}

	return;
out:
	//BruceTODO?
	//uas_cancel_scsi_cmd_all(vdev);
	return;
}

static void *uas_vdev_init(struct vhub_device *vdev, struct usb_host_interface *intf)
{
	struct uas_dev *uas;
	struct usb_host_endpoint *eps[4] = { };
	int r;

	if (!uas_is_interface(intf)) {
		return NULL;
	}
	udbg("This is UASP storage interface!\n");
	uas = kzalloc(sizeof(struct uas_dev), GFP_ATOMIC);
	BUG_ON(!uas);

#if 0 //Done by kzalloc().
	uas->cmndq_wr_ptr = 0;
	uas->cmndq_rd_ptr = 0;
	memset(uas->cmnd_queue, 0, sizeof(u32) * UAS_CMND_QUEUE_DEPTH);
#endif

	r = uas_find_endpoints(intf, eps);
	if (r) {
		uerr("Can't find corresponding UAS end points?!\n");
		goto err;
	}
	uas->cmnd_addr = usb_endpoint_addr(&eps[0]->desc);
	uas->status_addr = usb_endpoint_addr(&eps[1]->desc);
	uas->data_in_addr = usb_endpoint_addr(&eps[2]->desc);
	uas->data_out_addr = usb_endpoint_addr(&eps[3]->desc);

	udbg("cmd:%02x, status:%02x, in:%02x, out:%02x\n", uas->cmnd_addr, uas->status_addr, uas->data_in_addr ,uas->data_out_addr);

	vdev->uas = uas;

	return (void *)uas;
err:
	kfree(uas);
	return NULL;
}

static inline void uas_reg_data_in_ep(struct vhub_device *vdev, void *ep_info)
{
	struct uas_dev *uas = vdev->uas;
	uas->data_in_ep_info = ep_info;
}


static inline void *uas_vdev_free(struct vhub_device *vdev)
{
	if (vdev->uas) {
		kfree(vdev->uas);
		vdev->uas = NULL;
	}

	return NULL;
}

static inline int vdev_is_uas(struct vhub_device *vdev)
{
	if (vdev->uas)
		return 1;

	return 0;
}

static inline int uas_is_data_in_ep(struct vhub_device *vdev, u8 local_addr)
{
	struct uas_dev *uas = vdev->uas;

	if (uas->data_in_addr == local_addr)
		return 1;

	return 0;
}

static inline int uas_is_data_out_ep(struct vhub_device *vdev, u8 local_addr)
{
	struct uas_dev *uas = vdev->uas;

	if (uas->data_out_addr == local_addr)
		return 1;

	return 0;
}

static inline int uas_is_data_ep(struct vhub_device *vdev, u8 local_addr)
{
	if (uas_is_data_out_ep(vdev, local_addr) || uas_is_data_in_ep(vdev, local_addr))
		return 1;

	return 0;
}

static inline int uas_is_cmnd_ep(struct vhub_device *vdev, u8 local_addr)
{
	struct uas_dev *uas = vdev->uas;

	if (uas->cmnd_addr == local_addr)
		return 1;

	return 0;
}

static inline int uas_is_status_ep(struct vhub_device *vdev, u8 local_addr)
{
	struct uas_dev *uas = vdev->uas;

	if (uas->status_addr == local_addr)
		return 1;

	return 0;
}

