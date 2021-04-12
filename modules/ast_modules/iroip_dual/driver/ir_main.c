/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#if (CONFIG_AST1500_SOC_VER >= 2)

#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/dma-mapping.h>
#include <asm/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/semaphore.h>
#include "ir_types.h"
#include "../app/ir_ioctl.h"
#include "ir_task.h"
#include "ir_data.h"
#include "ir_network.h"
#include "client_mgnt.h"
#include "ir_txrx.h"
#include <linux/platform_device.h>

#include <asm/arch/ast-scu.h>

MODULE_LICENSE ("GPL");

#define IRS_WAIT_IR_RX 0
#define IRS_WAIT_IR_TX 1

#undef IR_NET_RX_TOKEN /* use single thread for IR net RX, no need to use rx token */

#define IR_NET_TX_SOCKET_NUM 2 /* one for remote, one for local ir decode/learing */
#define IR_NET_TX_SOCKET_INDEX_REMOTE 0
#define IR_NET_TX_SOCKET_INDEX_LOCAL 1

struct s_ir_dev
{
	unsigned int ir_major;
	int is_client;
	int exit;
	int net_exit;
	int state;
	struct ir_task net_rx_t;
	struct workqueue_struct *wq;
	struct work_struct ir_rx_done_work;
	struct work_struct ir_tx_done_work;
	struct work_struct net_rx_done_work;

	dma_addr_t rx_desc_base_pa;
	dma_addr_t rx_buf_base_pa;
	RX_DESC *rx_desc;
	unsigned char *rx_buf;
	unsigned int rx_desc_idx;

	dma_addr_t tx_desc_base_pa;
	dma_addr_t tx_buf_base_pa;
	TX_DESC *tx_desc;
	unsigned char *tx_buf;
	unsigned int tx_desc_idx;
	struct socket *net_tx_socket[IR_NET_TX_SOCKET_NUM];
	struct socket *net_rx_socket;
	struct socket *local_socket; /* for sending to user space */
#if defined(IR_NET_RX_TOKEN)
	struct semaphore net_rx_sem;
#endif
} ir_dev;

struct semaphore ir_user_lock;

//#define DEBUG_PROFILE_SCU_FREE_RUN_CNT
#if defined(DEBUG_PROFILE_SCU_FREE_RUN_CNT)
#define SCU_FREE_RUN_COUNTER (ASPEED_SCU_BASE + 0xE0)
static u32 profile_start_time;

#define profile_start() \
do { \
	profile_start_time = read_register(SCU_FREE_RUN_COUNTER); \
} while (0)

#define profile_finish() \
do {                                                                \
	u32 end, delta;                                                 \
                                                                    \
	end = read_register(SCU_FREE_RUN_COUNTER);                      \
                                                                    \
	if (end > profile_start_time)                                   \
		delta = end - profile_start_time;                           \
	else                                                            \
		delta = 0xFFFFFFFF - profile_start_time + end;              \
                                                                    \
	delta = delta / 24000;                                          \
	printk("\n %s(%d) delta %d ms (start 0x%.8x, end 0x%.8x)\n",    \
		__FUNCTION__, __LINE__, delta, profile_start_time, end);    \
} while (0)
#else
#define profile_start() do {} while (0)
#define profile_finish() do {} while (0)
#endif

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) "IRoIP2",
	.id = -1,
	.dev = {
		.release = the_pdev_release,
	},
};

//eng_is_interrupted() will ack all interrupt status, but only return what interests.
unsigned int eng_is_interrupted(void)
{
	volatile unsigned int status = read_register (IR_INT_STAT);
	volatile unsigned int int_mask = read_register(IR_INT_EN);

	//IR_info("Int!! 0x%08X\n", status);

	write_register(IR_INT_STAT, status);
	status &= int_mask;

	return status;
}

#define eng_kick_dma() do { write_register (IR_DESC_POLL, 1); } while (0)

static void eng_switch_port(int port)
{
#if (BOARD_DESIGN_VER_IR >= 204) && (BOARD_DESIGN_VER_IR < 300)
	if (port == IR_TX_PORT) {
		gpio_direction_output(GPIO_IR_IO_SWITCH, 1);
	} else {
		gpio_direction_input(GPIO_IR_IO_SWITCH);
	}
#endif
}

static void eng_stop(void)
{
	write_register_and(IR_ENG_CFG, I2S94_IR_EN(0), I2S94_IR_EN(1));

	/*
	** Bruce150521. According to Jazoe, the only concern here is we need to make sure DMA engine is idle.
	** This msleep(1) causes ~50(~15x3) ms in total latency. So, we remove it.
	*/
	//msleep(1);
}

static void eng_start(void)
{
	write_register_or(IR_ENG_CFG, I2S94_IR_EN(1));

	//Bruce110318. A delay is required after enable engine.
	barrier();
	udelay(200);
	eng_kick_dma();
}

static int eng_is_client(void)
{
	return ast_scu.board_info.is_client;
}

static inline void eng_scu_ir_disable(void)
{
	ast_scu.scu_op(SCUOP_IR_STOP, NULL);
}

static inline void eng_scu_init(void)
{
	ast_scu.scu_op(SCUOP_IR_INIT, NULL);
}

static void eng_disable_int(void)
{
	write_register(IR_INT_EN, 0);
}

static void eng_enable_rx_int(void)
{
	/*
	** Bruce110322. With the help of IR engine's "silent detection" and "timeout", IR engine can
	** correctly detect both short and long IR button press commands. Also, we assume that the
	** IR rx buffer and descriptor is always enough before "timeout" interrupt. It is OK to just
	** enable I2S98_EN_RX_DESC_END(1) as IR rx interrupt mask.
	*/
	//enable rx interrupt
	write_register(IR_INT_EN, IR_RX_INT_MASK);
}

static void eng_enable_tx_int(void)
{
	//enable tx interrupt
	write_register(IR_INT_EN, IR_TX_INT_MASK);
}

static void eng_tx_init(dma_addr_t desc_base)
{
	eng_switch_port(IR_TX_PORT);

	//Set IR clock
	write_register(IR_ENG_CLK_CFG,
			I2S7C_IRCLK_DIV(CURRENT_IR_CLK_DIV)
			| I2S7C_IRCLK_DIV_EN(1)
			| I2S7C_IRCLK_SRC(CURRENT_IRCLK_SRC)
			| I2S7C_IR_TX_MODE_SEL(1)
			);

	wmb();

	//fill in tx descriptor base register
	write_register(IR_DESC_BASE, desc_base & IR_BASE_ADDRESS_MASK);

	//IR_DMA_TH
	write_register(IR_DMA_TH, I2S90_TIMEOUT(SRC_TIMEOUT_PERIOD)|I2S90_FIFO_TH(0x7));//per jazoe
	wmb();
	//Clear Interrupt status
	write_register(IR_INT_STAT, 0xFFFFFFFF);
	wmb();
	//enable engine
//	printk("IR tx turn on bit I2S94_IR_EN\n");
	/*
	** Bruce150520. According to Jazoe, SRC_SILENT_PERIOD and SRC_IR_TIMEOUT_CTL
	** has no effect when IR Tx.
	*/
	write_register(IR_ENG_CFG,
			I2S94_SILENT_DET(SRC_SILENT_PERIOD)
			| I2S94_ENCODE_MODE(SRC_ENCODE_MODE)
			| I2S94_TIMEOUT_CTL(SRC_IR_TIMEOUT_CTL)
			| I2S94_IR_POLARITY(SRC_IR_TX_POLARITY)
			| I2S94_IR_EN(0)
			);
	wmb();
}

static void eng_rx_init(dma_addr_t desc_base)
{
	eng_switch_port(IR_RX_PORT);

	//Set IR clock
	write_register(IR_ENG_CLK_CFG,
			I2S7C_IRCLK_DIV(CURRENT_IR_CLK_DIV)
			| I2S7C_IRCLK_DIV_EN(1)
			| I2S7C_IRCLK_SRC(CURRENT_IRCLK_SRC)
			| I2S7C_IR_TX_MODE_SEL(0)
			);
	wmb();

	//fill in rx descriptor base register
	write_register(IR_DESC_BASE, desc_base & IR_BASE_ADDRESS_MASK);
	//IR_DMA_TH
//	write_register(IR_DMA_TH, I2S90_TIMEOUT(SRC_TIMEOUT_PERIOD) | I2S90_FIFO_TH(0xF));
	write_register(IR_DMA_TH, I2S90_TIMEOUT(SRC_TIMEOUT_PERIOD) | I2S90_FIFO_TH(0x7));//per jazoe
	wmb();
	//Clear Interrupt status
	write_register(IR_INT_STAT, 0xFFFFFFFF);

	//IR_ENG_CFG and enable engine
//	printk("IR rx turn on bit I2S94_IR_EN\n");
	write_register(IR_ENG_CFG,
			I2S94_SILENT_DET(SRC_SILENT_PERIOD)
			| I2S94_ENCODE_MODE(SRC_ENCODE_MODE)
			| I2S94_TIMEOUT_CTL(SRC_IR_TIMEOUT_CTL)
			| I2S94_IR_POLARITY(SRC_IR_RX_POLARITY)
			| I2S94_IR_EN(0)
			);
	wmb();
}

#if defined(IR_NET_RX_TOKEN)
#define ir_get_net_rx_token(pIr) down_interruptible(&pIr->net_rx_sem)
#define ir_put_net_rx_token(pIr) up(&pIr->net_rx_sem)
#define ir_init_net_rx_token(pIr) init_MUTEX(&pIr->net_rx_sem)
#else
#define ir_get_net_rx_token(pIr) (0)
#define ir_put_net_rx_token(pIr) do {} while (0)
#define ir_init_net_rx_token(pIr) do {} while (0)
#endif

static void ir_tx_desc_init(struct s_ir_dev *ir)
{
	unsigned int i;

	TX_DESC *tx_desc = ir->tx_desc;
	dma_addr_t tx_buf_base_pa = ir->tx_buf_base_pa;

	for (i = 0; i < NUM_TX_DESCRIPTOR; i++) {
		tx_desc[i].base_address = (tx_buf_base_pa & IR_BASE_ADDRESS_MASK) + (i * SIZE_TX_PACKET);
		tx_desc[i].tx_desc1_u.tx_desc1.owner_bit = SW_OWN;
		tx_desc[i].tx_desc1_u.tx_desc1.end_bit = 0;
		tx_desc[i].tx_desc1_u.tx_desc1.length = 0;
	}
	tx_desc[NUM_TX_DESCRIPTOR - 1].tx_desc1_u.tx_desc1.end_bit = 1;
}

static void ir_tx_restart(struct s_ir_dev *ir, unsigned int init_desc)
{
	eng_stop();

	if (init_desc) {
		ir_tx_desc_init(ir);
	}
	ir->rx_desc_idx = 0;
	ir->tx_desc_idx = 0;

	ir->state = IRS_WAIT_IR_TX;

	eng_tx_init(ir->tx_desc_base_pa);
	eng_enable_tx_int();
	eng_start();
}

static void ir_rx_restart(struct s_ir_dev *ir)
{
	unsigned int i;

	RX_DESC *rx_desc = ir->rx_desc;
	dma_addr_t rx_buf_base_pa = ir->rx_buf_base_pa;

	eng_stop();

	for (i = 0; i < NUM_RX_DESCRIPTOR; i++) {
		rx_desc[i].base_address = (rx_buf_base_pa & IR_BASE_ADDRESS_MASK) + (i * SIZE_RX_PACKET);
		rx_desc[i].rx_desc1_u.rx_desc1.owner_bit = ENGINE_OWN;
		rx_desc[i].rx_desc1_u.rx_desc1.end_bit = 0;
		rx_desc[i].rx_desc1_u.rx_desc1.length = SIZE_RX_PACKET;
	}
	rx_desc[NUM_RX_DESCRIPTOR - 1].rx_desc1_u.rx_desc1.end_bit = 1;
	ir->rx_desc_idx = 0;
	ir->tx_desc_idx = 0;

	ir->state = IRS_WAIT_IR_RX;

	eng_rx_init(ir->rx_desc_base_pa);
	eng_enable_rx_int();
	eng_start();
}

void handle_net_rx_done(void *data)
{
	struct s_ir_dev *ir = (struct s_ir_dev *)data;

	//IR_info("handle_net_rx_done\n");
	if (ir->exit)
		return;

	eng_disable_int();
	eng_stop();

	/* When driver handling net_rx_done event, all pending ir_rx_done should be canceled. */
	/*
	** Bruce110930. cancel_delayed_work() only canceled the work enqueued by queue_delayed_work().
	** Should use flush_workqueue() to flush all pending "ir_rx_done".
	** BTW, flush_workqueue() won't deadlock when calling by itself.
	*/
	cancel_delayed_work(&ir->ir_rx_done_work);
	flush_workqueue(ir->wq);

	ir_tx_restart(ir, 0);
}

void handle_ir_tx_done(void *data)
{
	struct s_ir_dev *ir = (struct s_ir_dev *)data;

	//IR_info("handle_ir_tx_done\n");
	if (ir->exit)
		return;

	//ir->tx_desc_idx = 0;

	ir_rx_restart(ir);

	ir_put_net_rx_token(ir);
}

static int ir_rx_to_net_tx(struct s_ir_dev *ir, int size, int num)
{
	struct kvec iov[3];
	unsigned int bytes_xmited;
	NetHeader_Struct NetHeader;
	struct socket *sock;
	int is_tcp = 0;
	int snd_num = num;
	int snd_size = size;
	int i;

	IR_info("IR R(%d), (%d)\n", size, num);

	//Bruce110321. We ASSUME the IR data won't never exceed 64KB (UDP limit).
	if (size + sizeof(NetHeader_Struct) >= 64*1024) {
		//Just discards the portion of data which larger than 64KB.
		snd_num = RX_SIZE_TO_DESC_CNT(64*1024 - sizeof(NetHeader_Struct)) - 1;
		snd_size = snd_num * SIZE_RX_PACKET;
	}

	/* header */
	memset(&NetHeader, 0, sizeof(NetHeader));
	NetHeader.magic = IR_MAGIC_NUM;
	NetHeader.length = snd_size;
	iov[0].iov_base = &NetHeader;
	iov[0].iov_len	= sizeof(NetHeader_Struct);

	for (i = 0; i < IR_NET_TX_SOCKET_NUM; i++) {
		sock = ir->net_tx_socket[i];

		if (sock == NULL)
			continue;

		/* Body */
		if ((ir->rx_desc_idx + snd_num) <= NUM_RX_DESCRIPTOR) {
			iov[1].iov_base = (UCHAR *)ir->rx_buf + ir->rx_desc_idx * SIZE_RX_PACKET;
			iov[1].iov_len	= snd_size;
			bytes_xmited = socket_xmit_iov(1, is_tcp, sock, iov, 2, sizeof(NetHeader_Struct) + snd_size, 0);
		} else {
			iov[1].iov_base = (UCHAR *)ir->rx_buf + ir->rx_desc_idx * SIZE_RX_PACKET;
			iov[1].iov_len	= (NUM_RX_DESCRIPTOR - ir->rx_desc_idx) * SIZE_RX_PACKET;
			iov[2].iov_base = (UCHAR *)ir->rx_buf;
			iov[2].iov_len	= snd_size - (NUM_RX_DESCRIPTOR - ir->rx_desc_idx) * SIZE_RX_PACKET;
			bytes_xmited = socket_xmit_iov(1, is_tcp, sock, iov, 3, sizeof(NetHeader_Struct) + snd_size, 0);
		}

		if (bytes_xmited != (sizeof(NetHeader_Struct) + snd_size)) {
			IR_err("sent %d bytes while sending %d bytes\n", bytes_xmited, sizeof(NetHeader_Struct) + snd_size);

			if (i == IR_NET_TX_SOCKET_INDEX_LOCAL) {
				/* local, to release local socket */
				sock->ops->shutdown(sock, 2);
				sockfd_put(sock);
				ir->net_tx_socket[i] = NULL;
			} else {
				return -1;
			}
		}
	}

	return 0;
}

static void ir_rx_recycle(struct s_ir_dev *ir, int num)
{
	unsigned int i;
	volatile RX_DESC *cur_desc;

	for (i = 0; i < num; i++) {
		cur_desc = &ir->rx_desc[ir->rx_desc_idx];
		cur_desc->rx_desc1_u.rx_desc1.length = SIZE_RX_PACKET;
		cur_desc->rx_desc1_u.rx_desc1.rx_timeout = 0;
		barrier();
		cur_desc->rx_desc1_u.rx_desc1.owner_bit = ENGINE_OWN;

		ir->rx_desc_idx = (ir->rx_desc_idx + 1) % NUM_RX_DESCRIPTOR;
	}
}

void handle_ir_rx_done(void *data)
{
	struct s_ir_dev *ir = (struct s_ir_dev *)data;
	volatile RX_DESC *cur_desc;
	volatile unsigned int packet_num = 0, size;
	unsigned int index;
	unsigned int ir_rx_completed;

poll_again:
	if (ir->exit)
		return;

	//IR_info("ir rx handler\n");
	if (ir->state == IRS_WAIT_IR_TX) {
		//Bruce110930. This case should never happen. If it happens, we should ignore it.
		IR_err("Wrong state!!! IRS_WAIT_IR_TX\n");
		return;
	}
	index = ir->rx_desc_idx;
	cur_desc = &ir->rx_desc[index];
	size = 0;
	packet_num = 0;
	ir_rx_completed = 0;

	while ((cur_desc->rx_desc1_u.rx_desc1.owner_bit == SW_OWN)) {
		packet_num++;
		size += cur_desc->rx_desc1_u.rx_desc1.length;

		/*double check*/
		if ((cur_desc->rx_desc1_u.rx_desc1.rx_timeout == 0)
			&& (cur_desc->rx_desc1_u.rx_desc1.length != SIZE_RX_PACKET)) {
			printk("This is HW bug!\n");
			BUG();
		}
		//IR_info("IR Ri(%d) PktNum(%d) Size(%d)\n", index, packet_num, size);
		if (cur_desc->rx_desc1_u.rx_desc1.rx_timeout == 1) {
			ir_rx_completed = 1;
			break;
		}

		index = (index + 1) % NUM_RX_DESCRIPTOR;
		cur_desc = &ir->rx_desc[index];

		if (index == ir->rx_desc_idx) {
			IR_info("IR rx buffer under run\n");
			break;
		}
	}

#if (CONFIG_AST1500_SOC_VER == 3)
	/*
	** Bruce150519. SoC V3 changes timeout behavior: Engine will reset timeout counter
	** on each descriptor write complete, instead of keep counting down in SoC V2.
	** It will introduce trouble when long pressing remote button.
	** When long press remote, descriptors are completed without 'last interrupt'
	** and 'timout bit in desc". So, driver will not transfer the Ir data untill
	** user release remote button (triggered 'silence').
	** To resolve this problem, we:
	** 1. enable 'desc complete' interrupt. IR98[0].
	** 2. always send completed descriptors. (always set ir_rx_completed = 1).
	*/
	ir_rx_completed = 1;
#endif
	//enqueue to Net_Tx_Thread
	if (ir_rx_completed && packet_num) {
		if (ir_rx_to_net_tx(ir, size, packet_num) != 0)
			ir->exit = 1;

		ir_rx_recycle(ir, packet_num);

		if (ir->exit)
			return;

		eng_kick_dma();

		yield();
		goto poll_again;
	}

	eng_enable_rx_int();
}

void net_rx_thread_udp(struct s_ir_dev *ir, struct socket *skt)
{
	IR_info("--->net_rx_thread_udp(%p)\n", skt);

	while (1) {
		volatile TX_DESC *cur_desc;
		unsigned int packet_num, i;
		NetHeader_Struct NetHeader;
		struct kvec iov[3];
		int bytes_received;

		bytes_received = 0;
		if (ir_get_net_rx_token(ir)) {
			IR_err("semaphore interrupted?!\n");
			break;
		}

		if ((ir->exit) || (ir->net_exit))
			break;

		//Bruce110321. We ASSUME the IR data won't never exceed 64KB (UDP limit).
		//header
		iov[0].iov_base = &NetHeader;
		iov[0].iov_len	= sizeof(NetHeader_Struct);
		//Body
		//Bruce110321. We assume we always start receiving network data from ir_tx buf base.		
		iov[1].iov_base = (UCHAR *)ir->tx_buf;
		iov[1].iov_len	= MAX_UDP_RX_SIZE;
		bytes_received = socket_xmit_iov(0, IR_NET_PROT_UDP, skt, iov, 2, sizeof(NetHeader_Struct) + MAX_UDP_RX_SIZE, 0);

		if ((ir->exit) || (ir->net_exit))
			break;

		if (bytes_received < 0) {
			/* do not break if bytes_received == 0 (peer close socket) */
			break;
		}

		if (bytes_received < sizeof(NetHeader_Struct)) {
			ir_put_net_rx_token(ir);
			continue;
		}

		bytes_received -= sizeof(NetHeader_Struct);

		if (NetHeader.magic != IR_MAGIC_NUM) {
			ir_put_net_rx_token(ir);
			continue;
		}

		if (bytes_received != NetHeader.length) {
			ir_put_net_rx_token(ir);
			continue;
		}

		IR_info("Net R(%d)\n", bytes_received);
		packet_num = TX_SIZE_TO_DESC_CNT(bytes_received);

		for (i = 0; i < packet_num; i++){
			cur_desc = &ir->tx_desc[ir->tx_desc_idx];
			if (bytes_received > SIZE_TX_PACKET) {
				cur_desc->tx_desc1_u.tx_desc1.length= SIZE_TX_PACKET;
				bytes_received -= SIZE_TX_PACKET;
			} else {
				BUG_ON(i != (packet_num - 1));
				cur_desc->tx_desc1_u.tx_desc1.length = bytes_received;
				//size -= size;
			}
			cur_desc->tx_desc1_u.tx_desc1.owner_bit = ENGINE_OWN;

			ir->tx_desc_idx = (ir->tx_desc_idx + 1) % NUM_TX_DESCRIPTOR;
		}

		if (!ir->exit)
			queue_work(ir->wq, &ir->net_rx_done_work);
	}
	IR_info("net_rx_thread_udp()<---\n");
}

void Net_Rx_Thread(struct ir_task *irt)
{
	struct s_ir_dev *ir = NULL;
	struct socket *skt = NULL;
	ir = (struct s_ir_dev *)irt->pContext;
	skt = ir->net_rx_socket;
	net_rx_thread_udp(ir, skt);
}

static irqreturn_t IR_ISR(int irq, void *dev_id, struct pt_regs *regs)
{
	struct s_ir_dev *ir = (struct s_ir_dev *)dev_id;
	unsigned int status;

	//eng_is_interrupted() will read and ack all interrupt status.
	status = eng_is_interrupted();
	if (!status)
		return IRQ_NONE;

	switch (ir->state) {
	case IRS_WAIT_IR_RX:
		if (status & IR_RX_INT_MASK) {
			eng_disable_int();
			// IR rx done event
			queue_work(ir->wq, &ir->ir_rx_done_work);
		}
		break;
	case IRS_WAIT_IR_TX:
		if (status & IR_TX_INT_MASK) {
			eng_disable_int();
			// IR tx done event
			queue_work(ir->wq, &ir->ir_tx_done_work);
		}
		break;
	default:
		IR_err("WTF:%d\n", ir->state);
		break;
	}
	return IRQ_HANDLED;
}

static int ir_tx_buf_alloc(struct s_ir_dev *ir)
{
	//tx ring buffer allocation
	if (ir->tx_buf == NULL) {
		ir->tx_buf = dma_alloc_coherent(NULL, 
						SIZE_TX_PACKET * NUM_TX_DESCRIPTOR,
						(dma_addr_t *)&ir->tx_buf_base_pa,
						GFP_KERNEL);
		if (ir->tx_buf == NULL) {
			printk ("Can't allocate tx buffer\n");
			return 0;
		}
		memset((void *)ir->tx_buf, 0, SIZE_TX_PACKET * NUM_TX_DESCRIPTOR);
	}
	//tx descriptor allocation
	if (ir->tx_desc == NULL) {
		ir->tx_desc = dma_alloc_coherent(NULL, 
						SIZE_TX_DESCRIPTOR * NUM_TX_DESCRIPTOR,
						(dma_addr_t *)&ir->tx_desc_base_pa,
						GFP_KERNEL);
		if (ir->tx_desc == NULL) {
			printk ("Can't allocate tx descriptor\n");
			return 0;
		}
		memset((void *)ir->tx_desc, 0, SIZE_TX_DESCRIPTOR * NUM_TX_DESCRIPTOR);
	}

//	printk ("IR tx_buf = %x\n", (u32)tx_buf);
//	printk ("IR tx_desc = %x\n", (u32)tx_desc);
	IR_info("tx_buf_dma = %x\n", ir->tx_buf_base_pa);
	IR_info("tx_desc_dma = %x\n", ir->tx_desc_base_pa);
	IR_info("PACKETSIZE = %x\n", SIZE_TX_PACKET);
	IR_info("NUM_TX_DESCRIPTOR = %x\n", NUM_TX_DESCRIPTOR);

	return 1;
}

static int ir_rx_buf_alloc(struct s_ir_dev *ir)
{
	//rx ring buffer allocation
	if (ir->rx_buf == NULL) {
		ir->rx_buf = dma_alloc_coherent(NULL,
			SIZE_RX_PACKET * NUM_RX_DESCRIPTOR,
			(dma_addr_t *)&ir->rx_buf_base_pa,
			GFP_KERNEL);
		if (ir->rx_buf == NULL) {
			printk ("Can't allocate rx buffer\n");
			BUG();
			return 0;
		}
	}
	memset ((void *)ir->rx_buf, 0 , SIZE_RX_PACKET * NUM_RX_DESCRIPTOR);

	//rx descriptor allocation
	if (ir->rx_desc == NULL) {
		ir->rx_desc = dma_alloc_coherent(NULL,
			SIZE_RX_DESCRIPTOR * NUM_RX_DESCRIPTOR,
			(dma_addr_t *)&ir->rx_desc_base_pa,
			GFP_KERNEL);
		if (ir->rx_desc == NULL) {
			printk ("Can't allocate rx descriptor\n");
			BUG();
			return 0;
		}
	}
	memset ((void *)ir->rx_desc, 0 , SIZE_RX_DESCRIPTOR * NUM_RX_DESCRIPTOR);

//	printk ("IR rx_buf = %x\n", (u32)rx_buf);
//	printk ("IR rx_desc = %x\n", (u32)rx_desc);
	IR_info("rx_buf_base_pa = %x\n", ir->rx_buf_base_pa);
	IR_info("rx_desc_base_pa = %x\n", ir->rx_desc_base_pa);
	IR_info("PACKETSIZE = %x\n", SIZE_RX_PACKET);
	IR_info("NUM_RX_DESCRIPTOR = %x\n", NUM_RX_DESCRIPTOR);

	return 1;
}

void ir_tx_buf_free(struct s_ir_dev *ir)
{
	if (ir->tx_buf)
		dma_free_coherent(NULL, NUM_TX_DESCRIPTOR * SIZE_TX_PACKET, (void *)ir->tx_buf, ir->tx_buf_base_pa);
	if (ir->tx_desc)
		dma_free_coherent(NULL, NUM_TX_DESCRIPTOR * SIZE_TX_DESCRIPTOR, (void *)ir->tx_desc, ir->tx_desc_base_pa);
}

void ir_rx_buf_free(struct s_ir_dev *ir)
{
	if (ir->rx_buf)
		dma_free_coherent(NULL, NUM_RX_DESCRIPTOR * SIZE_RX_PACKET, (void *)ir->rx_buf, ir->rx_buf_base_pa);
	if (ir->rx_desc)
		dma_free_coherent(NULL, NUM_RX_DESCRIPTOR * SIZE_RX_DESCRIPTOR, (void *)ir->rx_desc, ir->rx_desc_base_pa);
}

void ir_drv_init(struct s_ir_dev *ir)
{
	ir->wq = create_singlethread_workqueue("IR_WQ");
	BUG_ON(ir->wq == NULL);

	INIT_WORK(&ir->ir_rx_done_work, handle_ir_rx_done, (void*)ir);
	INIT_WORK(&ir->ir_tx_done_work, handle_ir_tx_done, (void*)ir);
	INIT_WORK(&ir->net_rx_done_work, handle_net_rx_done, (void*)ir);

	ir_init_net_rx_token(ir);

	eng_scu_init();

	IRQ_SET_HIGH_LEVEL(IR_INT);
	IRQ_SET_LEVEL_TRIGGER(IR_INT);

	if (request_irq(IR_INT, IR_ISR, SA_SHIRQ, "iroip", ir)) {
		IR_err("Unable to request IR IRQ\n");
		BUG();
		return;
	}

	ir_rx_buf_alloc(ir);
	ir_tx_buf_alloc(ir);
	ir_tx_desc_init(ir);
}

static void ir_socket_shutdown(struct s_ir_dev *ir)
{
	int i;

	if (ir->net_rx_socket != NULL) {
		ir->net_rx_socket->ops->shutdown(ir->net_rx_socket, 2);
		IR_info("IR downstream socket shut down\n");
	}

	for (i = 0; i < IR_NET_TX_SOCKET_NUM; i++) {
		if (ir->net_tx_socket[i] != NULL)
			ir->net_tx_socket[i]->ops->shutdown(ir->net_tx_socket[i], 2);

		if (i == IR_NET_TX_SOCKET_INDEX_REMOTE)
			IR_info("IR upstream socket shut down\n");
	}
}

static void ir_socket_put(struct s_ir_dev *ir)
{
	int i;

	if (ir->net_rx_socket != NULL) {
		sockfd_put(ir->net_rx_socket);
		IR_info("data downstream socket released\n");
		ir->net_rx_socket = NULL;
	}

	for (i = 0; i < IR_NET_TX_SOCKET_NUM; i++) {
		if (ir->net_tx_socket[i] != NULL) {
			sockfd_put(ir->net_tx_socket[i]);
			ir->net_tx_socket[i] = NULL;
		}

		if (i == IR_NET_TX_SOCKET_INDEX_REMOTE)
			IR_info("data upstream socket released\n");
	}
}

static void ir_stop(void)
{
	struct s_ir_dev *ir = &ir_dev;

	ir->net_exit = 1; /* set to notify net rx thread leave */

	ir_socket_shutdown(ir);

	flush_workqueue(ir->wq);

	if (ir->net_rx_t.thread != NULL) {
		stop_ir_thread(&ir->net_rx_t);
		IR_info("net rx thread stopped\n");
	}

	ir->net_exit = 0;

	ir_socket_put(ir);

	ir_init_net_rx_token(ir);

	eng_stop();
}

static ssize_t show_unlink(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t store_unlink(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	return count;
}
static DEVICE_ATTR(unlink, (S_IRUGO | S_IWUSR), show_unlink, store_unlink);

static ssize_t store_stop(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	down(&ir_user_lock);
	ir_stop();
	up(&ir_user_lock);
	//A7 removed
	//ast_notify_user("e_iroip_stopped");

	return count;
}
static DEVICE_ATTR(stop, S_IWUSR, NULL, store_stop);


static struct attribute *dev_attrs[] = {
	&dev_attr_unlink.attr,
	&dev_attr_stop.attr,
	NULL,
};

struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static int IR_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = -1, err;
	struct s_ir_dev *ir = &ir_dev;
	IO_ACCESS_DATA ioctl_data;
	int fd;

	copy_from_user(&ioctl_data, (void*)arg, sizeof(ioctl_data));

	down(&ir_user_lock);

	switch (cmd) {
	case IOCTL_IR_START_REMOTE:
		printk("IOCTL_IR_START_REMOTE\n");
		fd = ioctl_data.data;
		ir->net_tx_socket[IR_NET_TX_SOCKET_INDEX_REMOTE] = sockfd_lookup(fd, &err);
		if (ir->net_tx_socket[IR_NET_TX_SOCKET_INDEX_REMOTE] == NULL) {
			IR_err("Unable to get downstream socket(%d)\n", fd);
			BUG();
			break;
		}
		ret = 0;
		break;
	case IOCTL_IR_START_RX:
		ir_rx_restart(ir);
		ret = 0;
		break;
	case IOCTL_IR_START_TX:
		printk("IOCTL_IR_START_TX\n");
		if (ir->net_rx_socket != NULL) {
			ir->net_exit = 1;
			ir->net_rx_socket->ops->shutdown(ir->net_rx_socket, 2);
			flush_workqueue(ir->wq);
			stop_ir_thread(&ir->net_rx_t);
			ir->net_exit = 0;
			sockfd_put(ir->net_rx_socket);
			ir->net_rx_socket = NULL;
		}
		fd = ioctl_data.data;
		ir->net_rx_socket = sockfd_lookup(fd, &err);
		if (ir->net_rx_socket == NULL) {
			IR_err("Unable to get upstream socket(%d)\n", fd);
			BUG();
			break;
		}
		ir_task_init(&ir->net_rx_t, "IR_NET_RX", Net_Rx_Thread);
		ir->net_rx_t.pContext = ir; //Point to ir itself.
		start_ir_thread(&ir->net_rx_t);
		ret = 0;
		break;
	case IOCTL_IR_START_LOCAL:
		fd = ioctl_data.data;
		ir->net_tx_socket[IR_NET_TX_SOCKET_INDEX_LOCAL] = sockfd_lookup(fd, &err);

		if (ir->net_tx_socket[IR_NET_TX_SOCKET_INDEX_LOCAL] == NULL) {
			IR_err("Unable to get local socket(%d)\n", fd);
			/* NO BUG() here, just keep it NULL */
			break;
		}

		ret = 0;
		break;
        default:
		IR_err("unsupported CMD(%d)\n", cmd);
	}

	/* ir->exit may got set if we close socket before, clear it to continue IR function */
	if (ret == 0)
		ir->exit = 0;

	up(&ir_user_lock);

	return ret;
}

static struct file_operations ir_fops = {
	.ioctl = IR_ioctl,
};

int IR_init(void)
{
	int result;
	struct s_ir_dev *ir = &ir_dev;

	IR_info("--->IR_init\n");

	memset((void *)ir, 0, sizeof(struct s_ir_dev));

	ir->ir_major = 266;

	result = register_chrdev(ir->ir_major, "IRoIP2", &ir_fops);
	if (result < 0) {
		IR_err("register_chrdev failed (major = %d)\n", ir->ir_major);
		return result;
	}

	if (ir->ir_major == 0) {
		ir->ir_major = result;
		IR_info("allocated major = %d\n", ir->ir_major);
	} else
		IR_info("configured major = %d\n", ir->ir_major);

	init_MUTEX(&ir_user_lock);

	if (eng_is_client()) {
		IR_info("client\n");
		ir->is_client = 1;
	} else {
		IR_info("host\n");
		ir->is_client = 0;
	}

	ir_drv_init(ir);

	if (platform_device_register(&pdev) < 0) {
		IR_err("platform_driver_register err\n");
		return -ENOMEM;
	}

	if (sysfs_create_group(&pdev.dev.kobj, &dev_attr_group)) {
		IR_err("can't create sysfs files\n");
		return -ENOMEM;
	}

	IR_info("IR_init<---\n");
	return 0;
}

void IR_cleanup(void)
{
	struct s_ir_dev *ir = &ir_dev;

	IR_info("--->IR_cleanup\n");
	ir->exit = 1;

	eng_scu_ir_disable();

	free_irq(IR_INT, ir);

	ir_stop();

	destroy_workqueue(ir->wq);

	sysfs_remove_group(&pdev.dev.kobj, &dev_attr_group);
	platform_device_unregister(&pdev);

	ir_tx_buf_free(ir);
	ir_rx_buf_free(ir);

	unregister_chrdev (ir->ir_major, "IRoIP2");

	//A7 removed
	//ast_notify_user("e_iroip_stopped");

	IR_info("IR_cleanup<---\n");
	return;
}

module_init (IR_init);
module_exit (IR_cleanup);

#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */
