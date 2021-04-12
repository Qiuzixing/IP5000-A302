/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 *
 */
#ifndef _CEC_H_
#define _CEC_H_
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/err.h>
#include <linux/kernel.h> /*printk*/
#include <linux/hardirq.h> /* in_interrupt */

#include <asm/arch/drivers/board_def.h>
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/video_hal.h>

#include "ast_task.h"

#define MODULE_NAME "cec"

#define CEC_ONLY_ONE_DEVICE
#undef CEC_PA_OCCUPY
/*
 * udbg - print debug messages if CONFIG_VIDEO_DEBUG is defined
 * @fmt:
 * @args:
 */
#define udbg(fmt, args...)                                              \
	do {                                                            \
		printk(KERN_ERR "%-10s: %s(%d): " fmt, (in_interrupt() ? "interrupt" : (current)->comm), __FUNCTION__ , __LINE__, ##args); \
	} while (0)

/*
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)                                              \
	do {                                                            \
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt, (in_interrupt() ? "interrupt" : (current)->comm), __FUNCTION__, __LINE__,  ##args); \
	} while (0)

/*
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...) do { printk(KERN_INFO MODULE_NAME": " fmt , ## args); } while (0);

#define CEC_LA_TV		0
#define CEC_LA_RECORDER_1	1
#define CEC_LA_RECORDER_2	2
#define CEC_LA_TUNER_1		3
#define CEC_LA_PLAYER_1		4
#define CEC_LA_AUDIO		5
#define CEC_LA_TUNER_2		6
#define CEC_LA_TUNER_3		7
#define CEC_LA_PLAYER_2		8
#define CEC_LA_RECORDER_3	9
#define CEC_LA_TUNER_4		10
#define CEC_LA_PLAYER_3		11
#define CEC_LA_RESERVED_1	12
#define CEC_LA_RESERVED_2	13
#define CEC_LA_FREE_USE		14
#define CEC_LA_UNREGISTERED	15

#define AST_CEC_REMOTE_VALID_START CEC_LA_RECORDER_1
#define AST_CEC_REMOTE_VALID_END CEC_LA_PLAYER_3

#define CEC_FRAME_MAX	32 /* header(1) + opcode(1) + operand (max to 14 byte) = 16, use 32 for future expansion */

#define CEC_POLL_INTERVAL_IN_MS (10*1000)
#define CEC_MAP_NUM 15 /* for LA:0~14 */

#ifdef CONFIG_ARCH_AST1500_CLIENT
#define CEC_TOPOLOGY_BUF_SIZE_IN_WORD 1 /* active status, bit map */
#else
#define CEC_TOPOLOGY_BUF_SIZE_IN_WORD (1 + CEC_MAP_NUM) /* 1:active status, bit map, 15: 0~14 PA */
#endif
#define CEC_TOPOLOGY_BUF_SIZE (CEC_TOPOLOGY_BUF_SIZE_IN_WORD * 2)
#ifdef CONFIG_ARCH_AST1500_CLIENT
#define CEC_TOPOLOGY_BUF_SIZE_REMOTE_IN_WORD (1 + CEC_MAP_NUM) /* 1:active status, bit map, 15: 0~14 PA */
#define CEC_TOPOLOGY_BUF_SIZE_REMOTE (CEC_TOPOLOGY_BUF_SIZE_REMOTE_IN_WORD * 2)
#endif

#if defined(CEC_PA_OCCUPY)
#define CEC_PA_LEVEL_MAX (4) /* spec. is max to 5, we'll take 1, so (5 - 1) */
#else
#define CEC_PA_LEVEL_MAX (5) /* spec. is max to 5 */
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
struct cec_addr_info {
	u8 valid;
	u8 remote;
	u8 remote_la;
	u8 resvered;

	u16 pa;
	u16 remote_pa;
} __attribute__ ((packed));
#endif

#define CEC_RX_QUEUE_NUM 8 /* must be power of 2 */

struct cec_frame {
	u32 size;
	u8 buf[CEC_FRAME_MAX];
};

struct cec_drv_data {
	int is_client;
	int exit;
	int net_exit;
	int net_exit_topology;
	int state;
	struct ast_task task_cec_rx_frame;
	struct ast_task task_net_rx_frame;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct ast_task task_net_rx_topology;
#endif
	struct workqueue_struct *wq;
	struct work_struct poll_work;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct work_struct topology_work;
#endif

	struct socket *sk_net_tx_frame;
	struct socket *sk_net_rx_frame;
#ifdef CONFIG_ARCH_AST1500_HOST
	struct socket *sk_net_tx_topology; /* host */
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	struct socket *sk_net_rx_topology; /* client */
#endif

	struct cec_frame rx_queue[CEC_RX_QUEUE_NUM];
	u32 rx_q_head;
	u32 rx_q_tail;

	u8 frame[CEC_FRAME_MAX];
	u8 topology[CEC_TOPOLOGY_BUF_SIZE];
#ifdef CONFIG_ARCH_AST1500_CLIENT
	u16 pa;
#if defined(CEC_ONLY_ONE_DEVICE)
	u8 la;
#endif
	u8 topology_remote[CEC_TOPOLOGY_BUF_SIZE_REMOTE];
	struct cec_addr_info addr_map[CEC_MAP_NUM];
#endif
	spinlock_t tx_lock;
	u32 drv_option;
};

#ifdef CONFIG_ARCH_AST1500_HOST
int vrxhal_cec_topology(unsigned char *pdata);
int vrxhal_cec_send(unsigned char *pdata, unsigned int size);
#endif

#define CEC_DRV_OPTION_DISABLE_EXTENSION	(0x1 << 0)
#endif /* #ifndef _CEC_H_ */
