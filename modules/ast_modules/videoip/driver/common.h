/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include "global.h"

#define ABS(x, y) (((x) > (y))?((x)-(y)):((y)-(x)))

#define NEW_FRAME_RATE_CONTROL 1

/* request full frame reason,  */
#define RFF_REASON_OOS			(0)
#define RFF_REASON_STREAM_ERROR		(1)
#define RFF_REASON_HEADER_CORRUPTED	(2)
#define RFF_REASON_VE_HANG		(3)
#define RFF_REASON_MODE_CHANGED		(4)
#define RFF_REASON_RESUME_FROM_PAUSE	(5)
#define RFF_REASON_MASK			(0x1f)
#define RFF_HIGH_PRIORITY_MASK		(0x1 << 5)


/* event handler */
#define VIDEOIP_EH_SHUTDOWN			    (1 << 0)
#define VIDEOIP_EH_RESETV1HOST		    (1 << 1)
#define VIDEOIP_EH_RESETV1CLIENT	    (1 << 2)
#define VIDEOIP_EH_NEW_FRAME		    (1 << 3)
#define VIDEOIP_EH_V1ERR_RESETHOST      (1 << 4)
#define VIDEOIP_EH_V1ERR_RESETCLIENT    (1 << 5)
#define VIDEOIP_EH_V1MODE_READY		    (1 << 6)
#define VIDEOIP_EH_V2MODE_READY		    (1 << 7)
#define VIDEOIP_EH_RESETV2HOST		    (1 << 8)
#define VIDEOIP_EH_RESETV2CLIENT	    (1 << 9)
#define VIDEOIP_EH_V2ERR_RESETHOST      (1 << 10)
#define VIDEOIP_EH_V2ERR_RESETCLIENT    (1 << 11)

void event_worker_thread_init(void);
void event_worker_thread_destroy(void);

#define W_FREE      1
#define W_NO_FREE   0
int videoip_event_delay_add_n_wait(ve_worker_func worker, void *pdata, unsigned int free_pdata, unsigned int delay, unsigned int wait, unsigned int session_id);
#define videoip_event_delay_add(worker, pdata, free_pdata, delay) do { videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, delay, 0, 0); } while (0)
#define videoip_event_add_n_wait(worker, pdata, free_pdata) do { videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, 0, 1, 0); } while (0)
#define videoip_event_add_ex(worker, pdata, free_pdata) do { videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, 0, 0, 0); } while (0)
#define videoip_event_add(worker, pdata) do { videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, W_FREE, 0, 0, 0); } while (0)
/* videoip_event_*_c means need to check session_id */
#define videoip_event_delay_add_c(worker, pdata, free_pdata, delay) videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, delay, 0, VideoDev.session_id)
#define videoip_event_add_n_wait_c(worker, pdata, free_pdata) videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, 0, 1, VideoDev.session_id)
#define videoip_event_add_ex_c(worker, pdata, free_pdata) videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, free_pdata, 0, 0, VideoDev.session_id)
#define videoip_event_add_c(worker, pdata) videoip_event_delay_add_n_wait((ve_worker_func)worker, (void *)pdata, W_FREE, 0, 0, VideoDev.session_id)

void videoip_thread_init(struct videoip_task *ut, char *name,
		void (*loop_ops)(struct videoip_task *));
void videoip_thread_start(struct videoip_task *ut);
void videoip_thread_stop(struct videoip_task *ut);

u32 interlace_to_prog_idx(int index);
unsigned int patch_pitch(unsigned int width);
void init_e_cfg(PENGINE_CONFIG pE_cfg);
void _dump_cap_info(PCAPTURE_INFO pCapInfo);
void init_pkt_info_hdr(LpPacketInfo pPkt_info);
void key_init_default(VIDEOIP_DEVICE *v);
u32 key_hash(VIDEOIP_DEVICE *v);
void key_setup(VIDEOIP_DEVICE *v, u32 *key);
void key_reset(VIDEOIP_DEVICE *v, u32 encrypt_off);

#define TIME_ABNORMAL_THRESHOLD (VideoDev.abnormalThreshold)

static inline int TIME_INTERVAL(unsigned long new, unsigned long old)
{
	unsigned int t;
	t = ((new>=old)?(new-old):(0xFFFFFFFF-old+new+1));

	if (t > TIME_ABNORMAL_THRESHOLD) {
		printk("T new(%lu), old(%lu)\n", new, old);
		t = 0;
	}
	return t;
}

void ast_notify_user_wrong_version(unsigned int from_ver, unsigned int my_ver);

#endif /* #ifndef _COMMON_H_ */

