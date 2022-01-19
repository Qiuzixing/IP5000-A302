/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/random.h>
#include "cec.h"
#include "network.h"
#include "debug.h"

static struct platform_device *pdev;
static struct cec_drv_data *drv_data;
static u32 drv_option = 0;

#if defined(CEC_TEST_MODE)
static u32 cec_test_flag = 0;
#endif

#if defined(CEC_REPORT_TO_APP)
enum
{
	OPEN_CEC_REPORT = 0,
	CLOSE_CEC_REPORT
};
static u32 cec_report_flag = CLOSE_CEC_REPORT;
#endif
module_param(drv_option, int, S_IRUGO);

wait_queue_head_t cec_wait_queue;
struct semaphore cec_user_lock;

char *cec_la_name[] = {
	"       TV" /* 0  */,
	"Recorder1" /* 1 */,
	"Recorder2" /* 2 */,
	"   Tuner1" /* 3 */,
	"  Player1" /* 4 */,
	"    Audio" /* 5 */,
	"   Tuner2" /* 6 */,
	"   Tuner3" /* 7 */,
	"  Player2" /*8 */,
	"Recorder3" /* 9 */,
	"   Tuner4" /*10 */,
	"  Player3" /*11 */,
	"Reserved1" /*12 */,
	"Reserved2" /*13 */,
	"  FreeUse" /*14 */,
	"UNREGISTERED" /* 15 */,
};

#if defined(AST_DEBUG)
struct op_code_entry {
	u16 opcode;
	u8 *name;
};

struct op_code_entry op_code_table[] = {
	{0x00, "Feature Abort"},
	{0x04, "Image View On"},
	{0x0D, "Text View On"},
	{0x1A, "Get Deck Status"},
	{0x1B, "Deck Status"},
	{0x32, "Set Menu Language"},
	{0x36, "Standby"},
	{0x44, "User Control Pressed"},
	{0x45, "User Control Released"},
	{0x46, "Give OSD Name"},
	{0x47, "Set OSD Name"},
	{0x70, "System Audio Mode Request"},
	{0x72, "Set System Audio Mode"},
	{0x7E, "System Audio Mode Status"},
	{0x80, "Routing Change"},
	{0x81, "Routing Information"},
	{0x82, "Active Source"},
	{0x83, "Give Physical Address"},
	{0x84, "Report Physical Address"},
	{0x86, "Set Stream Path"},
	{0x87, "Device Vendor ID"},
	{0x89, "Vendor Command"},
	{0x8C, "Give Device Vendor ID"},
	{0x8E, "Menu Status"},
	{0x8F, "Give Power Status"},
	{0x90, "Report Power Status"},
	{0x9D, "Inactive Source"},
	{0x9E, "CEC Version"},
	{0x9F, "Get CEC Version"},
	{0xA0, "Vendor Command With ID"},
};

static u32 op_code_table_size;

u8 *op_code_name(u16 opcode)
{
	u32 i;
	u8 *ret = "?";
	struct op_code_entry *popcode;

	popcode = op_code_table;

	for (i = 0; i < op_code_table_size; i++) {
		if (opcode == popcode->opcode) {
			ret = popcode->name;
			break;
		}
		popcode++;
	}

	return ret;
}

static int debug_level;

void cec_debug_level_cfg(int level)
{
	if ((level < DEBUG_LEVEL_TRACE) || (level > DEBUG_LEVEL_ERROR))
		return;
	debug_level = level;
}

int cec_debug_level(void)
{
	return debug_level;
}
#endif

static u16 pa_request = 0;
static int pa_requested(u16 la)
{
	int ret = 0;
	if (pa_request & (0x1 << la)) {
		pa_request &= ~(0x1 << la);
		ret = 1;
	}

	return ret;
}

#ifdef CONFIG_ARCH_AST1500_HOST
static u16 _set_stream_path = 0;
void record_set_stream_path(u8 *frame)
{
	u16 pa, active, i, *candidate;

	pa = (frame[2] << 8) | frame[3];
	candidate = (u16 *) drv_data->topology;
	active = *((u16 *) drv_data->topology);

	for (i = 1; i < CEC_MAP_NUM; i++) {
		if (!(active & (0x1 << i)))
			continue;

		if (pa == candidate[i]) {
			_set_stream_path |= (0x1 << i);
		}
	}
}
#endif

static void cec_record(u8 *buf, u32 size)
{
	u8 opcode = buf[1];
	u8 dest = buf[0] & 0xf;

	switch (opcode) {
	case 0x83: /* Give Physical Address */
		/*
		 * set pa_request and we can forward "Report Physical Address" frame
		 */
		pa_request |= (0x1 << dest);
		break;
#ifdef CONFIG_ARCH_AST1500_HOST
	case 0x86: /* Set Stream Path */
		record_set_stream_path(buf);
		break;
#endif
	default:
		break;
	}
}

static u32 cec_send_count(u8 *buf, u32 size)
{
	u8 op_code = buf[1], count = 1;

	switch (op_code) {
#ifdef CONFIG_ARCH_AST1500_HOST
	case 0x46: /* Give OSD Name */
		/*
		 * users get confused if NO OSD name on TV.
		 * always send this kind of packet (1 + 3) times to avoid this situation
		 */
		count = 4;
		break;
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
		/*
		 * sometimes no 'Set Stream Path' from Samsung 4K TV
		 * this cause video source doesn't send 'Active Source' to TV
		 * and deactivate TV CEC function
		 *
		 * we guess the reason is that TV didn't get 'Report Power Status'
		 * so add this section for further testing
		 */
	case 0x90: /* Report Power Status */
		count = 2;
		break;
#endif
	default:
		break;
	}

	return count;
}

static void _cec_send(u8 *buf, u32 size)
{
	unsigned long flags;

	spin_lock_irqsave(&drv_data->tx_lock, flags);
#ifdef CONFIG_ARCH_AST1500_HOST
	/* TODO PA remap ? */
	vrxhal_cec_send(buf, size);
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	crt_cec_send(crt_get(), buf, size);
#endif
	spin_unlock_irqrestore(&drv_data->tx_lock, flags);
}


static void cec_send(u8 *buf, u32 size)
{
	int i, count;

#if defined(AST_DEBUG)
	if (cec_debug_level() <= DEBUG_LEVEL_DEBUG) {
		int i;

		P_DBG("CEC Tx (Init:%.1x Dest:%.1x Opcode:%.2x) - ", (buf[0] >> 4) & 0xf, (buf[0] >> 0) & 0xf, buf[1]);
		for (i = 0; i < size; i++)
			printk(" %.2x", buf[i]);
		printk(" [%s]\n", op_code_name(buf[1]));
	}
#endif
	cec_record(buf, size);

	count = cec_send_count(buf, size);

	if (count) {
		_cec_send(buf, size);
		count--;

		for (i = 0; i < count; i++) {
			msleep(400);
			_cec_send(buf, size);
		}
	}

	return;
}

static void rx_queue_init(struct cec_drv_data *d)
{
	memset(d->rx_queue, 0,  CEC_RX_QUEUE_NUM * sizeof(struct cec_frame));
	d->rx_q_head = d->rx_q_tail = 0;
}

static u32 rx_queue_empty(struct cec_drv_data *d)
{
	if (d->rx_q_head == d->rx_q_tail)
		return 1;

	return 0;
}

static u32 rx_queue_full(struct cec_drv_data *d)
{
	if  (d->rx_q_head == ((d->rx_q_tail + 1) % CEC_RX_QUEUE_NUM))
		return 1;

	return 0;
}

static int rx_queue_enq(struct cec_drv_data *d, u8 *buf, u32 size)
{
	struct cec_frame *p;

	p = d->rx_queue + d->rx_q_tail;
	p->size = size;
	memcpy(p->buf, buf, size);
	d->rx_q_tail = (d->rx_q_tail + 1) % CEC_RX_QUEUE_NUM;
}

static struct cec_frame *rx_queue_deq(struct cec_drv_data *d)
{
	struct cec_frame *p;

	p = d->rx_queue + d->rx_q_head;
	d->rx_q_head = (d->rx_q_head + 1) % CEC_RX_QUEUE_NUM;

	return p;
}

static void cec_recv(u8 *buf, u32 size)
{
	if (!rx_queue_full(drv_data)) {
		rx_queue_enq(drv_data, buf, size);
		wake_up_interruptible(&cec_wait_queue);
	}
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
static u32 cec_pa_level(u16 pa)
{
	u32 level;
	if (pa == 0)
		level = 0;
	else if ((pa & 0xfff) == 0)
		level = 1;
	else if ((pa & 0x0ff) == 0)
		level = 2;
	else if ((pa & 0x00f) == 0)
		level = 3;
	else
		level = 4;

	return level;
}

static u16 client_pa(void)
{
	return crt_get()->edid_parse_result.hdmi_physical_address;
}

static u16 remap_pa_to_client(u16 pa)
{
	u16 new_pa, level;

	new_pa = client_pa();
	level = cec_pa_level(new_pa);

#if defined(CEC_PA_OCCUPY)
	new_pa |= (pa >> (level * 4));
#else
	new_pa |= ((pa & 0x0fff) >> ((level - 1) * 4)); /* it is ((pa << 4) >> (level * 4)) */
#endif
	return new_pa;
}

/* PA conversion from client to host is by lookup now */
#if 0
static u16 remap_pa_from_client(u16 pa)
{
	u16 new_pa, level;

	new_pa = client_pa();
	level = cec_pa_level(new_pa);

#if defined(CEC_PA_OCCUPY)
	new_pa = pa << (level * 4);
#else
	new_pa = (pa << (level * 4)) >> (1 * 4);
	new_pa |= 0x1000;
#endif

	return new_pa;
}
#endif

static u16 net_rx_remap_pa(struct cec_addr_info *paddr, u16 *ppa)
{
	P_TRACE("remap PA: 0x%.4x => 0x%.4x\n", ntohs(*ppa), paddr->pa);
	*ppa = htons(paddr->pa);
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

static int my_frame(struct cec_net_header *pnhdr)
{
	/* TODO check net header first*/
	return 1;
}

static int frame_validate_net_rx(struct cec_net_header *pnhdr, u8 *buf, u32 size)
{
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct cec_addr_info *paddr;
	u8 src, dest;
	int i;
#endif

	if (!my_frame(pnhdr))
		goto fail;

#ifdef CONFIG_ARCH_AST1500_CLIENT
	src = buf[0] >> 4;
	dest = buf[0] & 0xf;

	paddr = drv_data->addr_map;

	for (i = 0; i < CEC_MAP_NUM; i++) {
		paddr = drv_data->addr_map + i;

		if (!paddr->valid)
			continue;

		if (!paddr->remote)
			continue;

		/* check initiator */
		if (paddr->remote_la == src) {
			u16 opcode = buf[1], *ppa;

			/* modify LA */
			src = i;

			/* TODO modify PA for special case */
			switch (opcode) {
			case 0x84: /* report physical address */
			case 0x82: /* active source */
			case 0x9D: /* inactive source, try to avoid SONY TV compiain 'communication failed' */
				ppa = (u16 *)(buf + 2);
				net_rx_remap_pa(paddr, ppa);
				break;
			case 0x81: /* routing infomation */
			case 0x86: /* set stream path */
			case 0x70: /* system audio mode request */
			case 0xA2: /* set external timer */
			case 0x80: /* routing change */
				/* TODO drop these frame ?*/
				break;

			default:
				break;
			}

			break;
		}
	}

	if (i == CEC_MAP_NUM)
		goto fail;

	buf[0] = (src << 4) | dest;
#endif

#ifdef CONFIG_ARCH_AST1500_HOST

#endif

	return 0;
fail:
	return -1;
}

/*
 * cec_rx_frame_remap_la - determine which LA (dest) can be forwarded to remote and remap LA (dest)if necessary
 */
static int cec_rx_frame_remap_la(u8 *psrcdest)
{

#ifdef CONFIG_ARCH_AST1500_HOST
	/* forward all to client, we'll check at client side */
	return 0;
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
	int ret = -1;
	u8 dest;
	u16 active;
	struct cec_addr_info *pinfo;

	dest = (*psrcdest) & 0xf;

	if (dest == CEC_LA_UNREGISTERED) {
		ret = 0;
		goto exit;
	}

	active = *((u16 *) drv_data->topology);

	if (active & (0x1 << dest))
		goto exit;

	pinfo = drv_data->addr_map + dest;

	if ((pinfo->valid) && (pinfo->remote)) {
		ret = 0;
		/* remapped LA case */
		dest &= 0xF0;
		dest |= pinfo->remote_la;

		*psrcdest = dest;
	}
exit:
	if (ret != 0)
		P_TRACE("Not at remote: %s (%d)\n", cec_la_name[dest], dest);

	return ret;
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
static int my_path(u16 pa)
{
	int ret = 0;
	u16 _client_pa, level, mask;

	_client_pa = client_pa();
	level = cec_pa_level(_client_pa);

	mask = 0xffff << ((4 - level) * 4);

	if ((pa & mask) == _client_pa)
		ret = 1;

	return ret;
}

static int client_cec_rx_frame_remap_pa(u16 *ppa)
{
	int i, ret = -1;
	struct cec_addr_info *paddr;
	u16 pa;

	pa = ntohs(*ppa);

	if (!my_path(pa))
		goto exit;

	for (i = 0; i < CEC_MAP_NUM; i++) {
		paddr = drv_data->addr_map + i;
		if (!paddr->valid)
			continue;

		if (paddr->remote) {
			if (paddr->pa == pa) {
				*ppa = htons(paddr->remote_pa);
				ret = 0;
				break;
			}
		}
	}
exit:
	if (ret != 0)
		P_TRACE("cannot remap PA %.4x\n", pa);

	return ret;
}

static void record_pa(u8 la, u16 *ppa)
{
	struct cec_addr_info *paddr = drv_data->addr_map + la;

	if (!paddr->valid)
		return;

	if (paddr->remote)
		return;

	paddr->pa = ntohs(*ppa);
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

#ifdef CONFIG_ARCH_AST1500_HOST
static int osd_size = 0;
static char osd[14]; /* OSD name max. to 14 byte in ASCII */

static void osd_recover(u8 *buf, u32 size)
{
	u32 new_size, i;
	u8 *pnew;

	if (size <= 2)
		return;

	new_size = size - 2;
	pnew = buf + 2;

	if (new_size != osd_size) {
		memcpy(osd, pnew, new_size);
		osd_size = new_size;
		return;
	}

	if (pnew[new_size - 1] != 0) {
		if (pnew[new_size - 1] != osd[new_size - 1])
			memcpy(osd, pnew, new_size);
		return;
	}

	/* new OSD is corrupted, try to find which one is better */
	for (i = 0; i < new_size; i++) {
		if (osd[i] != pnew[i]) {
			if (pnew[i] == 0) {
				/* old OSD is better, patch */
				memcpy(pnew, osd, new_size);
			} else {
				/* new OSD is better, update */
				memcpy(osd, pnew, new_size);
			}

			break;
		}
	}
}
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#ifdef CONFIG_ARCH_AST1500_HOST
static void cec_frame_to_net_tx(struct cec_drv_data *d, u8 *buf, u32 size);

/*
 * gen_active_source - to generate 'Active Source' frame if needed
 *
 * there is no additional buffer (just registers) to store received CEC frame in IT6802 (HDMI Rx)
 * an old frame will get corrupted by new one if it cannot be processed in realtime
 * it has most impact on active source, for example:
 * (1) SONY TV may show 'communication failed' after source booting
 * (2) SONY TV cannot properly switch to new selected source
 *
 * So we monitor 'Set Stream Path' frame from TV, and generate a 'Active Source' frame
 */
static void gen_active_source(u16 la, u8 opcode)
{
	if (_set_stream_path & (0x1 << la)) {
		u32 clear = 0, generate = 0;

		switch (opcode) {
		case 0x82: /* active source */
			clear = 1;
			P_TRACE("got 'Active Source'\n");
			break;
		case 0x04: /* Image View On */
		case 0x0D: /* Text View On */
			/*
			 * SONY PlayStation always send 'Text View On' before 'Active Source'
			 * ignore this kind of frame
			 */
			break;
		case 0x84: /* Report Physical Address */
			/* IT6802 (HDMI Rx) request PA periodically, ignore it */
			break;
		default:
			clear = 1;
			generate = 1;
			break;
		}

		if (clear)
			_set_stream_path &= ~(0x1 << la);

		if (generate) {
			char buf[4];
			u16 *ppa;

			P_TRACE("gen 'Active Source'\n");
			ppa = (u16 *)drv_data->topology;
			ppa += la;

			buf[0] = (la << 4) | 0xf;
			buf[1] = 0x82;
			buf[2] = *ppa >> 8;
			buf[3] = *ppa & 0xff;

			cec_frame_to_net_tx(drv_data, buf, 4);
		}
	}
}
#endif

static int frame_validate_cec_rx(u8 *buf, u32 size)
{
	u8 *psrcdest, src, opcode;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	u16 *ppa;
#endif

	psrcdest = buf;

	/* logical address */
	if (cec_rx_frame_remap_la(psrcdest) != 0)
		goto invalid;

	opcode = buf[1];

#ifdef CONFIG_ARCH_AST1500_HOST
	/* check if we sent 'set stream path' before */
	gen_active_source((*psrcdest) >> 4, opcode);
#endif

	/* physical address */
	switch (opcode) {
#ifdef CONFIG_ARCH_AST1500_CLIENT
	case 0x86: /* Set Stream Path, parameter: physical address */
		ppa = (u16 *)(buf + 2);
		if (client_cec_rx_frame_remap_pa(ppa) != 0)
			goto invalid;

		break;
	case 0x80: /* Routing Change, parameter: old physical address + new physical address */
	case 0x82: /* Active Source, skip this to avoid switch misoperation at host side */
		goto invalid;
#endif
	case 0x84: /* Report Physical Address */
		src = (*psrcdest) >> 4;
#ifdef CONFIG_ARCH_AST1500_CLIENT
		record_pa(src, (u16 *)(buf + 2));
#endif
		if (pa_requested(src) == 0)
			goto invalid;
		break;
#ifdef CONFIG_ARCH_AST1500_HOST
	case 0x47: /* Set OSD Name*/
		/*
		 * a bug introduced by it6802(HDMI Rx): CEC Rx buffer got corrupted after sending frame
		 * it is easy to see this bug if we received a long CEC frame
		 *
		 * just try to recovery corrupted OSD here
		 */
		osd_recover(buf, size);
		break;
#endif
	default:
		break;
	}

	return 0;
invalid:
	return 1;
}

static void cec_frame_to_net_tx(struct cec_drv_data *d, u8 *buf, u32 size)
{
	struct kvec iov[2];
	struct cec_net_header net_header;
	u32 size_max, header_size, body_size;
	struct socket *sock;

	if (drv_data->drv_option & CEC_DRV_OPTION_DISABLE_EXTENSION) {
		P_TRACE("extension is disabled\n");
		return;
	}

	if (!drv_data->sk_net_tx_frame) {
		P_TRACE("sk_net_tx_frame is NULL\n");
		return;
	}

	if (frame_validate_cec_rx(buf, size) != 0)
		return;

	header_size = sizeof(struct cec_net_header);
	body_size = size;

	net_header.magic = CEC_MAGIC_NUM_FRAME;
	net_header.length = body_size;

	iov[0].iov_base = &net_header;
	iov[0].iov_len  = header_size;
	iov[1].iov_base = buf;
	iov[1].iov_len  = body_size;

	sock = drv_data->sk_net_tx_frame;

	size_max = iov[0].iov_len + iov[1].iov_len;

	socket_xmit_iov(1, IR_NET_PROT_UDP, sock, iov, 2, size_max, 0);

#if defined(AST_DEBUG)
	if (cec_debug_level() <= DEBUG_LEVEL_DEBUG) {
		int i;

		P_DBG("NET Tx (Init:%.1x Dest:%.1x Opcode:%.2x) - ", (buf[0] >> 4) & 0xf, (buf[0] >> 0) & 0xf, buf[1]);
		for (i = 0; i < size; i++)
			printk(" %.2x", buf[i]);
		printk("\n");
	}
#endif
}
#if defined(CEC_REPORT_TO_APP)
#define A_MAX_PAYLOAD 1024
#define NOTIFY_APP_MSG_FORMAT	"e_cec_cmd_report::"
#define CEC_CMD_MAX_NUM		100
#endif
static void _cec_recv(struct cec_frame *frame)
{
	u8 *buf;
	u32 size;
#if defined(CEC_REPORT_TO_APP)
	char msg[A_MAX_PAYLOAD] = {0};
	int index = 0;
	char num_to_hex[3] = {0};
#endif
	buf = frame->buf;
	size = frame->size;

	if (!size)
		goto exit;

#if defined(AST_DEBUG)
	if (cec_debug_level() <= DEBUG_LEVEL_DEBUG) {
		int i;

		P_DBG("CEC Rx (Init:%.1x Dest:%.1x Opcode:%.2x) - ", (buf[0] >> 4) & 0xf, (buf[0] >> 0) & 0xf, buf[1]);
		for (i = 0; i < size; i++)
			printk(" %.2x", buf[i]);
		printk(" [%s]\n", op_code_name(buf[1]));
	} else {
#endif
		/* always show message of 'Feature Abort' */
		if (buf[1] == 0x00) {
			int i;

			printk("CEC Rx (Init:%.1x Dest:%.1x Opcode:%.2x) - ", (buf[0] >> 4) & 0xf, (buf[0] >> 0) & 0xf, buf[1]);
			for (i = 0; i < size; i++)
				printk(" %.2x", buf[i]);
			printk(" [Feature Abort]\n");
		}
#if defined(AST_DEBUG)
	}
#endif
#if defined(CEC_REPORT_TO_APP)
	if(cec_report_flag == OPEN_CEC_REPORT)
	{
		int k = 0;
		snprintf(msg, A_MAX_PAYLOAD, "%s",NOTIFY_APP_MSG_FORMAT);
		index = strlen(NOTIFY_APP_MSG_FORMAT);
		if(size > CEC_CMD_MAX_NUM)
		{
			printk("Warning:receive cec cmd too long\n");
			goto exit;
		}
		for(k = 0;k < size;k++)
		{
			snprintf(num_to_hex,3,"%.2x",buf[k]);
			msg[index++] = num_to_hex[0];
			msg[index++] = num_to_hex[1];
			msg[index++] = ':';
		}
		msg[--index] = '\0';
		ast_notify_user(msg);
	}
#endif
	cec_frame_to_net_tx(drv_data, buf, size);
exit:
	return;
}

void cec_rx_frame_thread(void)
{
	struct cec_frame *frame;

	while (1) {
		if (wait_event_interruptible(cec_wait_queue, (!rx_queue_empty(drv_data)) != 0))
			break;

		frame = rx_queue_deq(drv_data);

		_cec_recv(frame);
	}
}

void net_rx_frame_thread(void)
{
	struct kvec iov[2];
	struct cec_net_header net_header;
	int bytes_received;
	u32 size_max, header_size, body_size;
	struct socket *sock;

	header_size = sizeof(struct cec_net_header);
	body_size = CEC_FRAME_MAX;

	iov[0].iov_base = &net_header;
	iov[0].iov_len  = header_size;
	iov[1].iov_base = drv_data->frame;
	iov[1].iov_len  = body_size;

	sock = drv_data->sk_net_rx_frame;

	size_max = iov[0].iov_len + iov[1].iov_len;

	P_TRACE("<%s> Enter\n", __func__);
	while (1) {
		if (signal_pending(current))
			break;

		/* shutdown socket cause return value of socket_xmit_iov is '0', add exit flag  */
		if ((drv_data->exit) || (drv_data->net_exit))
			break;

		bytes_received = socket_xmit_iov(0, IR_NET_PROT_UDP, sock, iov, 2, size_max, 0);

		if (bytes_received < 0) {
			/* do not break if bytes_received == 0 (peer close socket) */
			break;
		}

		if (bytes_received < header_size)
			continue;

		bytes_received -= header_size;

		if (net_header.magic != CEC_MAGIC_NUM_FRAME)
			continue;

		if (bytes_received != net_header.length)
			continue;

		/* TODO check this packet */
#if defined(AST_DEBUG)
		if (cec_debug_level() <= DEBUG_LEVEL_DEBUG) {
			int i, size = bytes_received;
			char *buf = drv_data->frame;

			P_DBG("NET Rx (Init:%.1x Dest:%.1x Opcode:%.2x) - ", (buf[0] >> 4) & 0xf, (buf[0] >> 0) & 0xf, buf[1]);
			for (i = 0; i < size; i++)
				printk(" %.2x", buf[i]);
			printk("\n");
		}
#endif

		if (drv_data->drv_option & CEC_DRV_OPTION_DISABLE_EXTENSION) {
			P_TRACE("extension is disabled\n");
			continue;
		}

		if (frame_validate_net_rx(&net_header, drv_data->frame, bytes_received) == 0)
			cec_send(drv_data->frame, bytes_received);
#if defined(CEC_TEST_MODE)
		else
		{
			if(cec_test_flag == 1)
			{
				cec_send(drv_data->frame, bytes_received);
			}
		}
	}
#endif
	P_TRACE("<%s> Leave\n", __func__);
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
u8 topology_la_type[] = {
	CEC_LA_TV		/* CEC_LA_TV               0 */,
	CEC_LA_RECORDER_1	/* CEC_LA_RECORDER_1       1 */,
	CEC_LA_RECORDER_1	/* CEC_LA_RECORDER_2       2 */,
	CEC_LA_TUNER_1		/* CEC_LA_TUNER_1          3 */,
	CEC_LA_PLAYER_1		/* CEC_LA_PLAYER_1         4 */,
	CEC_LA_AUDIO		/* CEC_LA_AUDIO            5 */,
	CEC_LA_TUNER_1		/* CEC_LA_TUNER_2          6 */,
	CEC_LA_TUNER_1		/* CEC_LA_TUNER_3          7 */,
	CEC_LA_PLAYER_1		/* CEC_LA_PLAYER_2         8 */,
	CEC_LA_RECORDER_1	/* CEC_LA_RECORDER_3       9 */,
	CEC_LA_TUNER_1		/* CEC_LA_TUNER_4          10 */,
	CEC_LA_PLAYER_1		/* CEC_LA_PLAYER_3         11 */,
	CEC_LA_RESERVED_1	/* CEC_LA_RESERVED_1       12 */,
	CEC_LA_RESERVED_1	/* CEC_LA_RESERVED_2       13 */,
	CEC_LA_FREE_USE		/* CEC_LA_FREE_USE         14 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_UNREGISTERED     15 */,
};

u8 topology_la_remap[] = {
	CEC_LA_FREE_USE		/* CEC_LA_TV               0 */,
	CEC_LA_RECORDER_2	/* CEC_LA_RECORDER_1       1 */,
	CEC_LA_RECORDER_3	/* CEC_LA_RECORDER_2       2 */,
	CEC_LA_TUNER_2		/* CEC_LA_TUNER_1          3 */,
	CEC_LA_PLAYER_2		/* CEC_LA_PLAYER_1         4 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_AUDIO            5 */,
	CEC_LA_TUNER_3		/* CEC_LA_TUNER_2          6 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_TUNER_3          7 */,
	CEC_LA_PLAYER_3		/* CEC_LA_PLAYER_2         8 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_RECORDER_3       9 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_TUNER_4          10 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_PLAYER_3         11 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_RESERVED_1       12 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_RESERVED_2       13 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_FREE_USE         14 */,
	CEC_LA_UNREGISTERED	/* CEC_LA_UNREGISTERED     15 */,
};

#if defined(CEC_ONLY_ONE_DEVICE)
struct la_type_info {
	u8 type;
	u8 num;
};

struct la_type_info priority_table[] =  {
	{CEC_LA_PLAYER_1, 3},
	{CEC_LA_TUNER_1, 4},
	{CEC_LA_AUDIO, 1},
	{CEC_LA_RECORDER_1, 3}
};

static u16 strip_remote_la(u16 remote_active, u16 local_active)
{
	u16 ret = 0;
	u8 type, num;
	int size, i, j;

	size = ARRAY_SIZE(priority_table);

	for (i = 0; i < size; i++) {
		type = priority_table[i].type;
		num = priority_table[i].num;

		for (j = AST_CEC_REMOTE_VALID_START; j <= AST_CEC_REMOTE_VALID_END; j++) {
			if (topology_la_type[j] != type)
				continue;
			if ((0x1 << j) & local_active)
				num--;
		}

		if (num == 0)
			continue;

		for (j = AST_CEC_REMOTE_VALID_START; j <= AST_CEC_REMOTE_VALID_END; j++) {
			if (topology_la_type[j] != type)
				continue;
			if ((0x1 << j) & remote_active)
				break;
		}

		if (j <= AST_CEC_REMOTE_VALID_END) {
			/* just support one device, break once we got one */
			break;
		}
	}

	if (j <= AST_CEC_REMOTE_VALID_END)
		ret = (0x1 << j);

	return ret;
}
#endif

static int build_address_map_by_remote(struct cec_addr_info *pinfo, u16 *remote)
{
	struct cec_addr_info *paddr;
	u32 pa_level;
	u16 active, pa, la;
	int i, cnt = 0;

	pa_level = cec_pa_level(client_pa());
	active = (*remote) & 0xFFE; /* only 1 ~ 11 */

#if defined(CEC_ONLY_ONE_DEVICE)
	active = strip_remote_la(active, *((u16 *) drv_data->topology));
#endif

	for (i = 0; i < CEC_MAP_NUM; i++) {
		if (!(active & (0x1 << i)))
			continue;

		pa = remote[i];
		P_TRACE("LA %d PA 0x%.4x %d.%d.%d.%d (%d)",
			i, pa, (pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf, cec_pa_level(pa));

		/* check level of physical address */
		if (pa_level + cec_pa_level(pa) > CEC_PA_LEVEL_MAX) {
			P_TRACE(" NG\n");
			continue;
		} else {
			P_TRACE(" OK\n");
		}

		la = topology_la_type[i];

		while (la != CEC_LA_UNREGISTERED) {
			paddr = pinfo + la;
			if (paddr->valid) {
				la = topology_la_remap[la];
				continue;
			}

			cnt++;
			paddr->valid = 1;
			paddr->remote = 1;
			paddr->remote_pa = pa;
			paddr->pa = remap_pa_to_client(pa);
			paddr->remote_la = i;
#if defined(CEC_ONLY_ONE_DEVICE)
			drv_data->la = la;
			crt_cec_la_cfg(crt_get(), la);
			goto build_done;
#else
			break;
#endif
		}
	}

#if defined(CEC_ONLY_ONE_DEVICE)
build_done:
#endif
	return cnt;
}

static int update_address_map_pre(void)
{
#if defined(CEC_ONLY_ONE_DEVICE)
	/*
	 * no idea why SONY TV do routing change (to 1.0.0.0) after received 'Inactive Source'
	 * so no 'Inactive Source' now
	 */
#if 0
	u8 la;
	u16 pa;

	la = drv_data->la;
	if (la != CEC_LA_UNREGISTERED) {
		u8 buf[4];

		pa = drv_data->addr_map[la].pa;

		buf[0] = ((la << 4) | 0x0); /* Dst is TV */
		buf[1] = 0x9D; /* Inactive Source */
		buf[2] = (pa >> 8);
		buf[3] = (pa & 0xFF);
		cec_send(buf, 4);
	}
#endif
#endif
}

static int update_address_map_post(void)
{
	struct cec_addr_info *paddr;
	u16 pa;
	u8 buf[16], la;
	int i;

	/*
	 * 'Report Physical Address' after logical address allocation
	 *
	 * CEC 10.1:
	 * Whenever a new physical address is discovered, a CEC device shall allocate its logical address
	 * and report the association between its logical and physical addresses by broadcasting the information on the bus.
	 */
	for (i = AST_CEC_REMOTE_VALID_START; i <= AST_CEC_REMOTE_VALID_END; i++) {
		paddr = drv_data->addr_map + i;
		if (paddr->valid && paddr->remote)
			break;
	}

	if (i <= AST_CEC_REMOTE_VALID_END) {
		u16 active;

		msleep(100); /* wait for transmission of 'Report Physical Address' frame */
		crt_cec_topology(crt_get(), drv_data->topology, 1); /* force update client topology status */
		active = *((u16 *) drv_data->topology);

		if (active & (0x1 << i)) {
#if defined(CEC_ONLY_ONE_DEVICE)
			crt_cec_la_cfg(crt_get(), 0xf); /* collision, set to unregistered */
#endif
			return -1;
		}
#if defined(CEC_ONLY_ONE_DEVICE)
		pa = paddr->pa;
		la = i;

		/* Report Physical Address */
		buf[0] = ((la << 4) | 0xF); /* Dst is broadcast */
		buf[1] = 0x84; /* Report Physical Address */
		buf[2] = (pa >> 8);
		buf[3] = (pa & 0xFF);
		buf[4] = topology_la_type[la];  /* device type */
		cec_send(buf, 5);
#endif
	}

	return 0;
}

static void update_address_map(struct cec_drv_data *d)
{
	int i;
	struct cec_addr_info *pinfo;
	u16 active, retry = 0;

	update_address_map_pre();

	pinfo = d->addr_map;

	crt_cec_topology(crt_get(), d->topology, 1); /* force update client topology status */

build_start:
	memset(pinfo, 0, sizeof(struct cec_addr_info) * CEC_MAP_NUM);
	active = *((u16 *) d->topology);
	for (i = 0; i < CEC_MAP_NUM; i++) {
		if (active & (0x1 << i))
			pinfo->valid = 1;
		pinfo++;
	}

	active = *((u16 *) d->topology_remote);

	if (active == 0) /* no CEC devices at remote side */
		return;

	/* update remote topology status */
	if (build_address_map_by_remote(d->addr_map, (u16 *) d->topology_remote)) {
		if (update_address_map_post() == -1) {
			u8 random_result;

			get_random_bytes(&random_result, 1);
			msleep((random_result & 0x7) * 1000);
			retry++;
			P_TRACE("build address map, retry counter %d\n", retry);
			goto build_start;
		}
	} else {
		/* can not build CEC device => run out of LA */
		u16 remote_active;

		remote_active = *((u16 *) d->topology_remote);

		if (remote_active) {
			/* clear topology status at client, let poll_work keep trying */
			memset(d->topology, 0, sizeof(u16));
		}

		active = *((u16 *) d->topology);
		P_TRACE("run out of LA, client active is 0x%.4x\n", active);
	}
}

static int need_update_by_remote_change(u8 *remote)
{
	if (memcmp(drv_data->topology_remote, remote, CEC_TOPOLOGY_BUF_SIZE_REMOTE)) {
#if defined(CEC_ONLY_ONE_DEVICE)
		struct cec_addr_info *paddr;

		u16 remote_active, *premote_pa, client_la;

		client_la = drv_data->la;

		if (client_la == CEC_LA_UNREGISTERED)
			return 1;


		remote_active = *((u16 *)remote);

		if (!(remote_active & (0x1 << client_la)))
			return 1;

		paddr = drv_data->addr_map + client_la;
		premote_pa = (u16 *)remote; premote_pa += paddr->remote_la;


		if ((*premote_pa) != paddr->remote_pa)
			return 1;
#else
		return 1;
#endif
	}

	return 0;
}

static int need_update_by_local(void)
{
	u16 local_active;

	local_active = *((u16 *) drv_data->topology);

	/* special case, LA 1 ~ 11 are all active or xmitter return 0xFFFF */
	if ((local_active & 0xFFE) == 0xFFE)
		return 1;

	return 0;
}

void net_rx_topology_thread(void)
{
	struct kvec iov[2];
	struct cec_net_header net_header;
	int bytes_received;
	u32 size_max, header_size, body_size;
	struct socket *sock;
	u8 topology_remote[CEC_TOPOLOGY_BUF_SIZE_REMOTE];

	header_size = sizeof(struct cec_net_header);
	body_size = CEC_TOPOLOGY_BUF_SIZE_REMOTE;

	iov[0].iov_base = &net_header;
	iov[0].iov_len  = header_size;
	iov[1].iov_base = topology_remote;
	iov[1].iov_len  = body_size;

	sock = drv_data->sk_net_rx_topology;

	size_max = iov[0].iov_len + iov[1].iov_len;
	P_TRACE("<%s> Enter\n", __func__);

	while (1) {
		if (signal_pending(current))
			break;

		/* shutdown socket cause return value of socket_xmit_iov is '0', add exit flag  */
		if ((drv_data->exit) || (drv_data->net_exit_topology))
			break;

		bytes_received = socket_xmit_iov(0, IR_NET_PROT_UDP, sock, iov, 2, size_max, 0);

		if (bytes_received < 0) {
			/* do not break if bytes_received == 0 (peer close socket) */
			break;
		}

		if (bytes_received < header_size)
			continue;

		bytes_received -= header_size;

		if (net_header.magic != CEC_MAGIC_NUM_TOPOLOGY)
			continue;

		if (bytes_received != net_header.length)
			continue;

		if (need_update_by_remote_change(topology_remote) || need_update_by_local()) {
			/* update topology status at client side */
			cancel_delayed_work(&drv_data->poll_work);
			flush_workqueue(drv_data->wq);
			crt_cec_topology(crt_get(), drv_data->topology, 1);
			queue_delayed_work(drv_data->wq, &drv_data->poll_work, msecs_to_jiffies(CEC_POLL_INTERVAL_IN_MS));

			/* update topology status at host side */
			memcpy(drv_data->topology_remote, topology_remote, CEC_TOPOLOGY_BUF_SIZE_REMOTE);

			/* */
			queue_work(drv_data->wq, &drv_data->topology_work);
		}
	}
	P_TRACE("<%s> Leave\n", __func__);
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

#ifdef CONFIG_ARCH_AST1500_HOST
static void cec_topology_to_net_tx(struct cec_drv_data *d)
{
	struct kvec iov[2];
	struct cec_net_header net_header;
	u32 size_max, header_size, body_size;
	struct socket *sock;

	if (!drv_data->sk_net_tx_topology) {
		P_TRACE("sk_net_tx_topology is NULL\n");
		return;
	}

	header_size = sizeof(struct cec_net_header);
	body_size = CEC_TOPOLOGY_BUF_SIZE;

	net_header.magic = CEC_MAGIC_NUM_TOPOLOGY;
	net_header.length = CEC_TOPOLOGY_BUF_SIZE;

	iov[0].iov_base = &net_header;
	iov[0].iov_len  = header_size;
	iov[1].iov_base = drv_data->topology;
	iov[1].iov_len  = body_size;

	sock = drv_data->sk_net_tx_topology;

	size_max = iov[0].iov_len + iov[1].iov_len;

	socket_xmit_iov(1, IR_NET_PROT_UDP, sock, iov, 2, size_max, 0);
}
#endif

static void poll_timer(struct cec_drv_data *d)
{
#ifdef CONFIG_ARCH_AST1500_CLIENT
	u16 topology, *pold, update = 0;


	pold = (u16 *)d->topology;

	if (*pold == 0xFFFF) {
		/* force xmitter to scan all LA */
		crt_cec_topology(crt_get(), (u8 *)&topology, 1);
	} else
		crt_cec_topology(crt_get(), (u8 *)&topology, 0);

	if (topology != (*pold))
		update = 1;

	/* have to update topology if PA changed, i.e. switch to another sink */
	if (d->pa != client_pa()) {
		update = 1;
		d->pa = client_pa();
	}

	if (update)
		queue_work(d->wq, &d->topology_work);
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
	if (vrxhal_cec_topology(d->topology) == 0) {
		cec_topology_to_net_tx(d);
	}
#endif

	if (d->wq)
		queue_delayed_work(d->wq, &d->poll_work, msecs_to_jiffies(CEC_POLL_INTERVAL_IN_MS));
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
void tx_hotplug_callback(unsigned int plugged)
{
	if (plugged & 0x1) {/* primary only in dual-output case */
		struct cec_drv_data *d = drv_data;

		cancel_delayed_work(&d->poll_work);
		flush_workqueue(d->wq);
		queue_work(d->wq, &d->topology_work);
		if (d->wq)
			queue_delayed_work(d->wq, &d->poll_work, msecs_to_jiffies(CEC_POLL_INTERVAL_IN_MS));
	}
}
#endif

static int cec_drv_init(struct cec_drv_data *d)
{
	struct workqueue_struct *wq;

	wq = create_singlethread_workqueue("cec_wq");
	if (!wq) {
		uerr("Failed to allocate wq?!\n");
		goto err;
	}

	d->drv_option = drv_option;
	d->wq = wq;

	spin_lock_init(&d->tx_lock);

	ast_task_init(&d->task_cec_rx_frame, "CEC_RX_FRAME", cec_rx_frame_thread);
	ast_task_init(&d->task_net_rx_frame, "CEC_NET_RX_FRAME", net_rx_frame_thread);

#ifdef CONFIG_ARCH_AST1500_CLIENT
	ast_task_init(&d->task_net_rx_topology, "CEC_NET_RX_TOPOLOGY", net_rx_topology_thread);
#endif

	INIT_WORK(&d->poll_work, (void (*)(void *))poll_timer, d);
#ifdef CONFIG_ARCH_AST1500_CLIENT
	INIT_WORK(&d->topology_work, (void (*)(void *))update_address_map, d);
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
#if defined(CEC_ONLY_ONE_DEVICE)
	d->la = CEC_LA_UNREGISTERED;
#endif
#endif
	rx_queue_init(d);
	init_waitqueue_head(&cec_wait_queue);

	init_MUTEX(&cec_user_lock);

#if defined(AST_DEBUG)
	op_code_table_size = ARRAY_SIZE(op_code_table);
	debug_level = DEBUG_LEVEL_INFO;
#endif
	return 0;
err:
	return -ENOMEM;

}

static void hook_init(void)
{
	reg_cecif_rx_callback(cec_recv);
#ifdef CONFIG_ARCH_AST1500_CLIENT
	crt_reg_tx_hotplug_cec_callback(tx_hotplug_callback);
#endif
}

static void hook_fini(void)
{
	dereg_cecif_rx_callback();
#ifdef CONFIG_ARCH_AST1500_CLIENT
	crt_dereg_tx_hotplug_cec_callback();
#endif
}

static void stop_thread_net(struct cec_drv_data *d)
{
	struct socket *sock;
	unsigned long status;

	d->net_exit = 1; /* set to notify net rx thread leave */

	/* Net Rx */
	if (d->sk_net_rx_frame != NULL) {
		sock = d->sk_net_rx_frame;
		status = sock->ops->shutdown(sock, 2);
	}

	if (d->task_net_rx_frame.thread != NULL)
		ast_stop_thread(&d->task_net_rx_frame);

	if (d->sk_net_rx_frame != NULL) {
		sock = d->sk_net_rx_frame;
		sockfd_put(sock);
		d->sk_net_rx_frame = NULL;
	}

#ifdef CONFIG_ARCH_AST1500_CLIENT
	/* Net Rx topology */
	d->net_exit_topology = 1; /* set to notify net rx thread leave */
	if (d->sk_net_rx_topology != NULL) {
		sock = d->sk_net_rx_topology;
		status = sock->ops->shutdown(sock, 2);
	}

	if (d->task_net_rx_topology.thread != NULL) {
		ast_stop_thread(&d->task_net_rx_topology);
	}

	if (d->sk_net_rx_topology != NULL) {
		sock = d->sk_net_rx_topology;
		sockfd_put(sock);
		d->sk_net_rx_topology = NULL;
	}
#endif

	/* Net Tx */
	if (d->sk_net_tx_frame != NULL) {
		sock = d->sk_net_tx_frame;
		d->sk_net_tx_frame = NULL;
		status = sock->ops->shutdown(sock, 2);
		sockfd_put(sock);
	}

#ifdef CONFIG_ARCH_AST1500_HOST
	/* Net Tx topology */
	if (d->sk_net_tx_topology != NULL) {
		sock = d->sk_net_tx_topology;
		status = sock->ops->shutdown(sock, 2);
		sockfd_put(sock);
		d->sk_net_tx_topology = NULL;
	}
#endif
}

static ssize_t show_topology(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	int num = 0, i;
#ifdef CONFIG_ARCH_AST1500_HOST
	u16 active;

	active = *((u16 *) d->topology);

	/* start from zero, to be the same as client */
	num += snprintf(buf + num, PAGE_SIZE - num, "%s: N\n", cec_la_name[0]);

	for (i = CEC_LA_RECORDER_1; i < CEC_LA_UNREGISTERED; i++) {
		num += snprintf(buf + num, PAGE_SIZE - num, "%s:", cec_la_name[i]);
		if (active & (0x1 << i)) {
			num += snprintf(buf + num, PAGE_SIZE - num, " Y");
			if (i > 0) {
				u16 *ppa = (u16 *) d->topology;

				ppa += i;
				num += snprintf(buf + num, PAGE_SIZE - num, " (%d.%d.%d.%d)",
					(*ppa >> 12) & 0xf, (*ppa >> 8) & 0xf, (*ppa >> 4) & 0xf, (*ppa >> 0) & 0xf);
			}

		} else
			num += snprintf(buf + num, PAGE_SIZE - num, " N");

		num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	}
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct cec_addr_info *pinfo = d->addr_map;
	u16 pa = client_pa();

	num += snprintf(buf + num, PAGE_SIZE - num, "Physical Address %d.%d.%d.%d Level %d\n\n",
			(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf, cec_pa_level(pa));

	for (i = 0; i < CEC_MAP_NUM; i++) {
		num += snprintf(buf + num, PAGE_SIZE - num, "%s:", cec_la_name[i]);
		if (pinfo->valid) {
			pa = pinfo->pa;
			num += snprintf(buf + num, PAGE_SIZE - num, " Y");

			num += snprintf(buf + num, PAGE_SIZE - num, " (%d.%d.%d.%d)",
				(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf);

			if (pinfo->remote) {
				pa = pinfo->remote_pa;
				num += snprintf(buf + num, PAGE_SIZE - num, " {Remote - %s:", cec_la_name[pinfo->remote_la]);
				num += snprintf(buf + num, PAGE_SIZE - num, " (%d.%d.%d.%d)",
					(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf);
				num += snprintf(buf + num, PAGE_SIZE - num, "}");
			}
		} else
			num += snprintf(buf + num, PAGE_SIZE - num, " N");

		num += snprintf(buf + num, PAGE_SIZE - num, "\n");
		pinfo++;
	}
#endif

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}
static DEVICE_ATTR(topology, S_IRUGO, show_topology, NULL);

static ssize_t show_topology_raw(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	int num = 0, i;
#ifdef CONFIG_ARCH_AST1500_HOST
	u16 active;

	active = *((u16 *) d->topology);

	/* start from zero, to be the same as client */
	num += snprintf(buf + num, PAGE_SIZE - num, " 0\n");

	for (i = CEC_LA_RECORDER_1; i < CEC_LA_UNREGISTERED; i++) {
		if (active & (0x1 << i)) {
			num += snprintf(buf + num, PAGE_SIZE - num, " 1");
			if (i > 0) {
				u16 *ppa = (u16 *) d->topology;

				ppa += i;
				num += snprintf(buf + num, PAGE_SIZE - num, " %.4x", *ppa);
			}

		} else
			num += snprintf(buf + num, PAGE_SIZE - num, " 0");

		num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	}
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct cec_addr_info *pinfo = d->addr_map;

	for (i = 0; i < CEC_MAP_NUM; i++) {
		num += snprintf(buf + num, PAGE_SIZE - num, " %d", pinfo->valid);
		if (pinfo->valid) {
			num += snprintf(buf + num, PAGE_SIZE - num, " %d", pinfo->remote);
			num += snprintf(buf + num, PAGE_SIZE - num, " %.4x", pinfo->pa);

			if (pinfo->remote) {
				num += snprintf(buf + num, PAGE_SIZE - num, " %.2d", pinfo->remote_la);
				num += snprintf(buf + num, PAGE_SIZE - num, " %.4x", pinfo->remote_pa);
			}
		}

		num += snprintf(buf + num, PAGE_SIZE - num, "\n");
		pinfo++;
	}
#endif

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}
static DEVICE_ATTR(topology_raw, S_IRUGO, show_topology_raw, NULL);

#ifdef CONFIG_ARCH_AST1500_CLIENT
static ssize_t show_pa(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	u16 pa = client_pa();

	num += snprintf(buf + num, PAGE_SIZE - num, "%d.%d.%d.%d (%d)\n",
			(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf, cec_pa_level(pa));

	return num;
}
static DEVICE_ATTR(pa, S_IRUGO, show_pa, NULL);
#endif

static ssize_t store_send(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned char data[CEC_FRAME_MAX];
	unsigned int i = 0;
	unsigned char *word, *str;
	char *sep = " |,:\n\r";

	str = (unsigned char *)buf;
	while (i < CEC_FRAME_MAX) {
		word = strsep((char **)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		data[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}

	if (i)
		cec_send(data, i);

	return count;
}
static DEVICE_ATTR(send, S_IWUSR, NULL, store_send);

static ssize_t store_recv(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned char data[CEC_FRAME_MAX];
	unsigned int i = 0;
	unsigned char *word, *str;
	char *sep = " |,:\n\r";

	str = (unsigned char *)buf;
	while (i < CEC_FRAME_MAX) {
		word = strsep((char **)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		data[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}

	if (i)
		cec_recv(data, i);

	return count;
}
static DEVICE_ATTR(recv, S_IWUSR, NULL, store_recv);

static ssize_t store_nrx(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	struct socket *sock;
	u32 cfg, c;
	int err = 0;

	c = sscanf(buf, "%d", &cfg);

	if (c == 0)
		goto exit;

	down(&cec_user_lock);

	d->net_exit = 1; /* set to notify net rx thread leave */
	if (d->sk_net_rx_frame != NULL) {
		sock = d->sk_net_rx_frame;
		sock->ops->shutdown(sock, 2);
	}

	if (d->task_net_rx_frame.thread != NULL) {
		ast_stop_thread(&d->task_net_rx_frame);
	}

	if (d->sk_net_rx_frame != NULL) {
		sock = d->sk_net_rx_frame;
		sockfd_put(sock);
		d->sk_net_rx_frame = NULL;
	}

	sock = sockfd_lookup(cfg, &err);

	if (sock) {
		d->net_exit = 0;
		d->sk_net_rx_frame = sock;
		ast_start_thread(&d->task_net_rx_frame);
	}

	up(&cec_user_lock);
exit:
	return count;
}
static DEVICE_ATTR(nrx, S_IWUSR, NULL, store_nrx);

static ssize_t store_ntx(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	struct socket *sock;
	u32 cfg, c;
	int err = 0;

	c = sscanf(buf, "%d", &cfg);

	if (c == 0)
		goto exit;

	if (d->sk_net_tx_frame != NULL) {
		unsigned long status;

		sock = d->sk_net_tx_frame;
		d->sk_net_tx_frame = NULL;
		status = sock->ops->shutdown(sock, 2);
		sockfd_put(sock);
	}

	sock = sockfd_lookup(cfg, &err);

	if (sock)
		d->sk_net_tx_frame = sock;
exit:

	return count;
}
static DEVICE_ATTR(ntx, S_IWUSR, NULL, store_ntx);

static ssize_t store_nrx_topology(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	struct socket *sock;
	u32 cfg, c;
	int err = 0;

	c = sscanf(buf, "%d", &cfg);

	if (c == 0)
		goto exit;

	down(&cec_user_lock);

	P_TRACE("<%s> Enter\n", __func__);
	d->net_exit_topology = 1; /* set to notify net rx thread leave */
	if (d->sk_net_rx_topology != NULL) {
		sock = d->sk_net_rx_topology;
		sock->ops->shutdown(sock, 2);
	}

	if (d->task_net_rx_topology.thread != NULL) {
		ast_stop_thread(&d->task_net_rx_topology);
	}

	if (d->sk_net_rx_topology != NULL) {
		sock = d->sk_net_rx_topology;
		sockfd_put(sock);
		d->sk_net_rx_topology = NULL;
	}

	sock = sockfd_lookup(cfg, &err);

	if (sock) {
		d->net_exit_topology = 0;
		d->sk_net_rx_topology = sock;
		ast_start_thread(&d->task_net_rx_topology);
	}

	P_TRACE("<%s> Leave\n", __func__);
	up(&cec_user_lock);
exit:
#endif
	return count;
}
static DEVICE_ATTR(nrx_topology, S_IWUSR, NULL, store_nrx_topology);

static ssize_t store_ntx_topology(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#ifdef CONFIG_ARCH_AST1500_HOST
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);
	struct socket *sock;
	u32 cfg, c;
	int err = 0;

	c = sscanf(buf, "%d", &cfg);

	if (c == 0)
		goto exit;

	if (d->sk_net_tx_topology != NULL) {
		unsigned long status;

		sock = d->sk_net_tx_topology;
		d->sk_net_tx_topology = NULL;
		status = sock->ops->shutdown(sock, 2);
		sockfd_put(sock);
	}

	sock = sockfd_lookup(cfg, &err);

	if (sock)
		d->sk_net_tx_topology = sock;

exit:
#endif
	return count;
}
static DEVICE_ATTR(ntx_topology, S_IWUSR, NULL, store_ntx_topology);

static ssize_t store_stop(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct cec_drv_data *d = platform_get_drvdata(pdev);

	down(&cec_user_lock);
	P_TRACE("<%s> Enter\n", __func__);

	/* do not stop thread of CEC rx */
#if 0
	if (d->task_cec_rx_frame.thread != NULL)
		ast_stop_thread(&d->task_cec_rx_frame);
#endif

	stop_thread_net(d);

#ifdef CONFIG_ARCH_AST1500_CLIENT
	/* clear remote topology status */
	d->topology_remote[0] = 0;
	d->topology_remote[1] = 0;
#endif
	P_TRACE("<%s> Leave\n", __func__);
	up(&cec_user_lock);

	return count;
}
static DEVICE_ATTR(stop, S_IWUSR, NULL, store_stop);

#if defined(AST_DEBUG)
static ssize_t show_debug_level(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "level: %d\n\n", cec_debug_level());
	num += snprintf(buf + num, PAGE_SIZE - num, "%d: trace\n", DEBUG_LEVEL_TRACE);
	num += snprintf(buf + num, PAGE_SIZE - num, "%d: debug\n", DEBUG_LEVEL_DEBUG);
	num += snprintf(buf + num, PAGE_SIZE - num, "%d: info\n", DEBUG_LEVEL_INFO);
	num += snprintf(buf + num, PAGE_SIZE - num, "%d: warning\n", DEBUG_LEVEL_WARNING);
	num += snprintf(buf + num, PAGE_SIZE - num, "%d: error\n", DEBUG_LEVEL_ERROR);

	return num;
}

static ssize_t store_debug_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);

	if (c == 0)
		goto exit;

	cec_debug_level_cfg(cfg);

exit:
	return count;
}
static DEVICE_ATTR(debug_level, (S_IWUSR | S_IRUGO), show_debug_level, store_debug_level);
#endif

#if defined(CEC_REPORT_TO_APP)
static ssize_t show_cec_report(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	if(cec_report_flag == OPEN_CEC_REPORT)
	{
		printk("open cec report\n");
	}
	else
	{
		printk("close cec report\n");
	}

	return num;
}

static ssize_t store_cec_report(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);
	if (c == 0)
		goto exit;

	if(cfg == 1)
	{
		cec_report_flag = OPEN_CEC_REPORT;
	}
	else
	{
		cec_report_flag = CLOSE_CEC_REPORT;
	}

exit:
	return count;
}
static DEVICE_ATTR(cec_report, (S_IWUSR | S_IRUGO), show_cec_report, store_cec_report);
#endif

#if defined(CEC_TEST_MODE)
static ssize_t show_cec_test(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	if(cec_test_flag == 1)
	{
		printk("open cec test_mode\n");
	}
	else
	{
		printk("close cec test_mode\n");
	}

	return num;
}

static ssize_t store_cec_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);
	if (c == 0)
		goto exit;

	if(cfg == 1)
	{
		cec_test_flag = 1;
	}
	else
	{
		cec_test_flag = 0;
	}

exit:
	return count;
}
static DEVICE_ATTR(cec_test, (S_IWUSR | S_IRUGO), show_cec_test, store_cec_test);
#endif
static struct attribute *dev_attrs[] = {
	&dev_attr_topology.attr,
	&dev_attr_topology_raw.attr,
#ifdef CONFIG_ARCH_AST1500_CLIENT
	&dev_attr_pa.attr,
#endif
	&dev_attr_send.attr,
	&dev_attr_recv.attr,
	&dev_attr_nrx.attr,
	&dev_attr_ntx.attr,
	&dev_attr_nrx_topology.attr,
	&dev_attr_ntx_topology.attr,
	&dev_attr_stop.attr,
#if defined(CEC_REPORT_TO_APP)
	&dev_attr_cec_report.attr,
#endif
#if defined(CEC_TEST_MODE)
	&dev_attr_cec_test.attr,
#endif
#if defined(AST_DEBUG)
	&dev_attr_debug_level.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static int drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct cec_drv_data *d;

	d = kzalloc(sizeof(struct cec_drv_data), GFP_KERNEL);

	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}

	drv_data = d;

	cec_drv_init(d);

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		goto err2;
	}

	platform_set_drvdata(pdev, (void *)d);

	hook_init();

        queue_work(d->wq, &d->poll_work);
	ast_start_thread(&d->task_cec_rx_frame);

	return ret;
err2:
	kfree(d);
err:
	return ret;
}

static int drv_remove(struct platform_device *pdev)
{
	struct cec_drv_data *d = platform_get_drvdata(pdev);

	if (!d)
		return 0;

	d->exit = 1;

	hook_fini();

	if (d->task_cec_rx_frame.thread != NULL)
		ast_stop_thread(&d->task_cec_rx_frame);

	stop_thread_net(d);

	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);

	drv_data = NULL;

	kfree(d);

	return 0;
}

static struct platform_driver cec_driver = {
	.probe          = drv_probe,
	.remove         = __devexit_p(drv_remove),
	.suspend        = NULL,
	.resume         = NULL,
	.driver         = {
		.name   = MODULE_NAME,
	},
};

MODULE_LICENSE("GPL");

static int __init drv_init(void)
{
	int ret;

	pdev = platform_device_register_simple(MODULE_NAME, -1, NULL, 0);

	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out;
	}

	ret = platform_driver_register(&cec_driver);

	if (ret == 0)
		goto out;

	platform_device_unregister(pdev);

out:
	uinfo("init done\n");
	return ret;
}

static void __exit drv_exit(void)
{
	platform_driver_unregister(&cec_driver);
	platform_device_unregister(pdev);
}

#ifndef MODULE
arch_initcall(drv_init);
#else
module_init(drv_init);
#endif
module_exit(drv_exit);
