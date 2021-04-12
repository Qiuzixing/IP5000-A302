/* Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>
#include "astparam.h"

/* node query packet */
#define NQ_PKT_MAGIC      0x4E4F4445   /* NODE in ASCII     */
#define NQ_MC_IP          "225.1.0.1"
#define NQ_PORT           59100         /* queryer use this port for receive. */
#define NR_PORT           59101         /* responser use this port for receive. */
#define NQ_TIMEOUT        50           /* in ms */
#define NQ_DEFAULT_MAX    600          /* default number of --masx */
#define NQ_ESSENTIAL_SIZE 200          /* about 200 bytes */
#define NQ_NET_BANDWIDTH  1250         /* NQ expect to take 1250 KB/s (10Mbps) network bandwidth. */
#define NQ_DEFAULT_PATH "/var"

#define NQPKT_DEFAULT_REPLY_TYPE "essential"

/* This data structure must be 32bits aligned */
struct nqpkt_hdr_s {
	uint32_t magic;
	uint16_t reserved; /* Must be 0. Reserved for future version information. */
	uint16_t query_period;
	uint16_t block1_size; /* param_buf */
	uint16_t block2_size; /* mac_list */
	uint8_t payload[];
} __attribute__ ((packed));

#define NQPKT_HDR_SIZE           (sizeof(struct  nqpkt_hdr_s))
#define NQPKT_TOTAL_SIZE(nq_pkt) (NQPKT_HDR_SIZE + (nq_pkt)->block1_size + (nq_pkt)->block2_size)

#endif /* #ifndef __NODE_H__ */
