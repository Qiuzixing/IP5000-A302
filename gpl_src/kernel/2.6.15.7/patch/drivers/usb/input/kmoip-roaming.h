/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _KMOIP_ROAMING_H_
#define _KMOIP_ROAMING_H_

#include <linux/list.h>

#define KMR_DEFAULT_X        1920
#define KMR_DEFAULT_Y        1080

#define KMR_BOUNDARY_MARGIN  5 /* 5 dots. After roaming to new target, start at 5 dots margin. Used for "de-bouncing". */

#define KMROOB_MASTER    (-1)  /* Special: Back to master target. */
#define KMROOB_INSIDE    0  /* NOT out of boundary */
#define KMROOB_BR        1  /* out of boundary: bottom right */
#define KMROOB_TL        2  /* out of boundary: top left */
#define KMROOB_TOP       3  /* out of boundary: top */
#define KMROOB_BOTTOM    4  /* out of boundary: bottom */
#define KMROOB_LEFT      5  /* out of boundary: left */
#define KMROOB_RIGHT     6  /* out of boundary: right */

typedef struct {
	unsigned int size; // active area
	unsigned int kmcsr_to_tch0_scale; /* scaling factor from active position to target tch0 position. */
	unsigned int local_abs_to_kmcsr_scale; /* scaling factor from abs local to active position. */
	/* Aboves are required for initialization. */
	unsigned int csr; // current cursor
	unsigned int oob; // out of boundary. For edge trigger
	unsigned int default_size; // default value when @size is not available
} kmr_csr_s;

typedef struct {
	struct list_head list; /* list entry for target list. */
	struct list_head report_list; /* report queue head */
	unsigned int report_q_size; /* queue length of report_list. */
	struct work_struct uwork; //upstream work
	struct work_struct heartbeat_work; //upstream heartbeat_work work
	struct workqueue_struct *dwq; //downstream work queue
	struct work_struct dwork; //downstream work
	struct kmoip_report last_kbd0_out_report;
	void *context; /* Point to kmdrv. */
	unsigned int host_id; /* Current host's UID. */
	kmr_csr_s x;
	kmr_csr_s y;
	struct socket *upstream_socket;
	struct socket *downstream_socket;
#define KMRWS_SCKT_DEAD   0    //sockets not ready yet
#define KMRWS_SCKT_READY  1    //sockets are ready
#define KMRWS_LOCAL       2    //it is a local target
	unsigned int work_state;
	char mac[13]; /* For id check. case sensitive. */
	int idx_x; /* the physical x position of this client. Can be negitave value. Master is in the center (0,0). */
	int idx_y; /* the physical x position of this client.*/
} kmr_target_s;

typedef struct {
#define KMR_ENABLE              (1 << 0)
	unsigned int option; //KMR_XXXX
	kmr_target_s *target; /* current upstream context. */
	kmr_target_s *local_target; /* local dummy context. Used when disconnected. */
	struct list_head targets; /* list head of upstream obj. */
	unsigned target_num; /* keep tracking the number of attached targets. */
} roaming_s;

#define kmr_for_each_target_safe(target, n, roaming) list_for_each_entry_safe(target, n, &((roaming)->targets), list)
/* (0,0) is always the roaming master. Or the only target when roaming is disabled. */
#define kmr_target_is_master(target) (((target)->idx_x == 0) && ((target)->idx_y == 0))
#define kmr_target_is_local(roaming, target) ((roaming)->local_target == target)
#define kmr_target_got_active_zone(target) (((target)->x.size) && ((target)->y.size))
#define kmr_is_roaming_enabled(roaming) ((roaming)->target_num > 1)

unsigned kmr_csr_remap_from_rel(roaming_s *roaming, kmr_csr_s *csr, __s32 value, __s32 tch0_size);
unsigned kmr_csr_remap_from_abs(roaming_s *roaming, kmr_csr_s *csr, __s32 value, __s32 tch0_size, __s32 ori_size);
void kmr_update_active_zone(kmr_target_s *target, unsigned int active_x, unsigned int active_y);
kmr_target_s *kmr_get_cur_target(roaming_s *roaming);
void kmr_drv_init(roaming_s *roaming);
void kmr_alloc_local_target(roaming_s *roaming);
kmr_target_s *kmr_find_target(roaming_s *roaming, char *mac, int idx_x, int idx_y);
kmr_target_s *kmr_alloc_target(roaming_s *roaming, char *mac, int idx_x, int idx_y);
void kmr_free_target(roaming_s *roaming, kmr_target_s *target);
void kmr_reattach_target(roaming_s *roaming, kmr_target_s *target, kmr_target_s *old_target);
void kmr_detach_target(roaming_s *roaming, kmr_target_s *target);
void kmr_try_roaming(roaming_s *roaming, kmr_target_s *target);


#endif /* #ifndef _KMOIP_ROAMING_H_ */
