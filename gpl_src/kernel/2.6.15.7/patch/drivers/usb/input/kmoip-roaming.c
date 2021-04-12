/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <linux/module.h>
#include <asm/unaligned.h> /* for __s32 */
#include <asm/arch/platform.h> /* for ast_notify_user() */

#include "hid.h"
#include "kmoip.h"
#include "kmoip-roaming.h"

int kmr_do_roaming(roaming_s *roaming, kmr_target_s *roam_from, unsigned oob);

/**
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__,  ##args);	\
	} while(0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "KMoIP: " fmt , ## args);	\
	} while(0)

inline kmr_target_s *kmr_get_cur_target(roaming_s *roaming)
{
	/* Always use local target is current target is not available. */
	return (roaming->target) ? (roaming->target) : (roaming->local_target);
}

inline void kmr_set_cur_target(roaming_s *roaming, kmr_target_s *target)
{
	roaming->target = target;
	//uerr("current target set to %p\n", roaming->target);
}

kmr_target_s *kmr_new_target(roaming_s *roaming)
{
	kmr_target_s *target;
	struct kmoip_drv *kmdrv = container_of(roaming, struct kmoip_drv, roaming);

	target = kzalloc(sizeof(*target), GFP_KERNEL);
	BUG_ON(!target);
	INIT_LIST_HEAD(&target->list);
	INIT_LIST_HEAD(&target->report_list);
	INIT_WORK(&target->uwork, (void (*)(void *))kmdrv->upstream_func, target);
	INIT_WORK(&target->heartbeat_work, (void (*)(void *))kmdrv->heartbeat_func, target);
	INIT_WORK(&target->dwork, (void (*)(void *))kmdrv->downstream_func, target);

	target->dwq = create_singlethread_workqueue("kmoip_downstream");
	target->context = kmdrv;

	return target;
}

void kmr_free_target(roaming_s *roaming, kmr_target_s *target)
{
	uerr("Freeing target=%p\n", target);
	list_del(&target->list);
	kfree(target);
}

void _kmr_add_target(roaming_s *roaming, kmr_target_s *target)
{
	list_add_tail(&target->list, &roaming->targets);
	roaming->target_num++;
}

kmr_target_s *kmr_find_target(roaming_s *roaming, char *mac, int idx_x, int idx_y)
{
	unsigned long flags;
	kmr_target_s *target, *found = NULL;
	struct kmoip_drv *kmdrv = container_of(roaming, struct kmoip_drv, roaming);

	spin_lock_irqsave(&kmdrv->lock, flags);

	/* A target is identified by corresponding client's MAC OR x/y position. */
	list_for_each_entry(target, &roaming->targets, list) {
		if ((target->idx_x == idx_x) && (target->idx_y == idx_y)) {
			found = target;
			break;
		}
		/* It is acceptable input MAC is NULL. Caller want to find target with only index match. */
		if (mac && !strncmp(target->mac, mac, 12)) {
			found = target;
			break;
		}
	}

	spin_unlock_irqrestore(&kmdrv->lock, flags);
	return found;
}

static inline void _kmr_dequeue_target(roaming_s *roaming, kmr_target_s *target)
{
	roaming->target_num--;
	list_del_init(&target->list);
}

kmr_target_s *kmr_alloc_target(roaming_s *roaming, char *mac, int idx_x, int idx_y)
{
	kmr_target_s *target;

#if 0 /* Assume the caller must delete duplicate target before calling kmr_alloc_target(). */
	target = kmr_find_target(roaming, usocket);
	if (target)
		return target;
#endif
	target = kmr_new_target(roaming);

	/* A target is identified by corresponding client's MAC OR x/y position. */
	strncpy(target->mac, mac, 12);
	target->idx_x = idx_x;
	target->idx_y = idx_y;

	target->x.default_size = KMR_DEFAULT_X;
	target->y.default_size = KMR_DEFAULT_Y;

	return target;
}

/*
** This is a special function called once on driver init.
** Allocate a target to be used as local.
** No up/down sockets.
** With work_state set to 2.
**
** Note: Must be called after kmdrv->xxx_func inited.
*/
void kmr_alloc_local_target(roaming_s *roaming)
{
	kmr_target_s *target;

	target = kmr_new_target(roaming);
	target->x.size = KMR_DEFAULT_X; // Useless?
	target->y.size = KMR_DEFAULT_Y; // Useless?
	target->x.default_size = KMR_DEFAULT_X;
	target->y.default_size = KMR_DEFAULT_Y;

	/* TODO. Someday, we should shadow required information and pre-caculate scale factors here. */
	/* Set follow flags to 0 to trigger re-caculation. */
	target->x.kmcsr_to_tch0_scale = 0;
	target->x.local_abs_to_kmcsr_scale = 0;
	target->y.kmcsr_to_tch0_scale = 0;
	target->y.local_abs_to_kmcsr_scale = 0;

	target->work_state = KMRWS_LOCAL;
	roaming->local_target = target;
}

void kmr_detach_target(roaming_s *roaming, kmr_target_s *target)
{
	unsigned long flags;
	struct kmoip_drv *kmdrv = container_of(roaming, struct kmoip_drv, roaming);

	spin_lock_irqsave(&kmdrv->lock, flags);
	target->work_state = KMRWS_SCKT_DEAD;
	_kmr_dequeue_target(roaming, target);
	if (kmr_get_cur_target(roaming) == target) {
		/*
		** Roam back to Master. 
		** If is already master, means going to free master.
		** So, force it to local (set to NULL).
		*/
		if (kmr_target_is_master(target))
			kmr_set_cur_target(roaming, NULL);
		else
			kmr_do_roaming(roaming, target, KMROOB_MASTER);
	}
	spin_unlock_irqrestore(&kmdrv->lock, flags);
}

void kmr_reattach_target(roaming_s *roaming, kmr_target_s *target, kmr_target_s *old_target)
{
	unsigned long flags;
	struct kmoip_drv *kmdrv = container_of(roaming, struct kmoip_drv, roaming);

	spin_lock_irqsave(&kmdrv->lock, flags);
	if (old_target) {
		/* swap if old_target exists. */
		old_target->work_state = KMRWS_SCKT_DEAD;
		_kmr_dequeue_target(roaming, old_target);
		_kmr_add_target(roaming, target);
		/* swap current target if active. */
		if (kmr_get_cur_target(roaming) == old_target)
			kmr_set_cur_target(roaming, target);

		/* Swap socket/upstream. Copy old cursor position. */
		target->x.csr = old_target->x.csr;
		target->y.csr = old_target->y.csr;
		/* The caller is responsible for kmr_free_target(old_target) */
	} else if (kmr_target_is_master(target)) {
		/* This is the case when first atttached (master). */
		_kmr_add_target(roaming, target);
		kmr_set_cur_target(roaming, target);
	} else {
		_kmr_add_target(roaming, target);
	}
	spin_unlock_irqrestore(&kmdrv->lock, flags);
}

void kmr_drv_init(roaming_s *roaming)
{
	INIT_LIST_HEAD(&roaming->targets);
}

static inline unsigned _kmr_csr_from_rel(kmr_csr_s *csr, __s32 value)
{
	unsigned c, size;
	unsigned edge_triggered = KMROOB_INSIDE;

	c = csr->csr;
	size = csr->size;
	/* Ignore, when size is not available yet. */
	if (unlikely(size == 0)) {
		size = csr->default_size;
		/*
		 * Bruce191030. To improve user experience,
		 * we speedup mouse cursor moving speed when
		 * cursor is in a inactive screen (host without
		 * video source/active zone info).
		 * The speedup value '4' comes from real world test.
		 * (1920x1080 resolution).
		 *
		 * We can't just skip this inactive screen because
		 * OOB detection will not be able to differenciate
		 * left or top. It will cause user can't roaming
		 * from an inactive screen to top/bottom screen.
		 */
		value = value * 4;
	}
	/* make sure result is in [0 to size] boundary */
	if (value > 0) {
		c = c + value;
		if (c > size) {
			c = size;
			edge_triggered = KMROOB_BR;
		}
	} else { /* negtive offset */
		if (-value > c) {
			c = 0;
			edge_triggered = KMROOB_TL;
		} else {
			c = c + value;
		}
	}

	if (edge_triggered) {
		csr->oob = edge_triggered;
	}
out:
	return c;
}

static inline unsigned _kmr_csr_from_abs(kmr_csr_s *csr, __s32 value, __s32 ori_size)
{
#define _ABS_EXTRA_SCALE_SHIFT 10
	unsigned c;
	unsigned int scale = csr->local_abs_to_kmcsr_scale;

	/* BruceToDo. Not Tested Yet. */

	if (unlikely(scale == 0)) {
		unsigned size = csr->size;

		if (unlikely(size == 0))
			size = csr->default_size;

		/* kmr csr (active zone) size is usually < local abs size. */
		scale = (size << _ABS_EXTRA_SCALE_SHIFT) / ori_size;
		if (unlikely(scale == 0)) {
			scale = 1 << _ABS_EXTRA_SCALE_SHIFT;
			uerr("local_abs_to_kmcsr_scale is 0?!\n");
		}
		csr->local_abs_to_kmcsr_scale = scale;
	}

	/* map abs to active zone abs position */
#if 0
	size = csr->size;
	c = (value * size + (ori_size -1)) / ori_size;
#else
	c = (value * scale) >> _ABS_EXTRA_SCALE_SHIFT;
#endif
	return c;
#undef _ABS_EXTRA_SCALE_SHIFT
}

static inline unsigned _kmr_csr_to_server_abs(kmr_csr_s *csr, unsigned tch0_size)
{
#define _EXTRA_SCALE_SHIFT   16
	unsigned int scale = csr->kmcsr_to_tch0_scale;
	unsigned int cur_csr = csr->csr;

	/* pre-caculate divider. DO NOT divide everytime. */
	if (unlikely(scale == 0)) {
		unsigned int csr_size = csr->size;
		/* Just in case csr->size is not available yet. */
		if (unlikely(csr_size == 0))
			csr_size = csr->default_size;
		/* TODO. is there a way to incrase precision? */
		/*
		** tch0_csr / tch0_size = cur_csr / cur_size
		** ==> tch0_csr = cur_csr * tch0_size / cur_size
		*/
		scale = (tch0_size << _EXTRA_SCALE_SHIFT) / csr_size;
		if (unlikely(scale == 0)) {
			/* We assume kmr csr size (active size) is always smaller than tch0_size. */
			scale = 1 << _EXTRA_SCALE_SHIFT;
			uerr("active_to_tch0_scale is 0?!\n");
		}
		csr->kmcsr_to_tch0_scale = scale;
	}

	return ((cur_csr * scale) >> _EXTRA_SCALE_SHIFT);
#undef _EXTRA_SCALE_SHIFT
}

kmr_target_s *kmr_select_new_target(roaming_s *roaming, kmr_target_s *from, unsigned oob)
{
	int new_x, new_y;
	kmr_target_s *to;

	new_x = from->idx_x;
	new_y = from->idx_y;

	switch (oob) {
		case KMROOB_TOP:
			new_y += 1;
			break;
		case KMROOB_BOTTOM:
			new_y -= 1;
			break;
		case KMROOB_LEFT:
			new_x -= 1;
			break;
		case KMROOB_RIGHT:
			new_x += 1;
			break;
		case KMROOB_MASTER:
			new_x = 0;
			new_y = 0;
			break;
		default:
			BUG();
	}
	to = kmr_find_target(roaming, NULL, new_x, new_y);
	if (!to)
		to = from;

	return to;
}

int kmr_do_roaming(roaming_s *roaming, kmr_target_s *roam_from, unsigned oob)
{
	kmr_target_s *roam_to;
	kmr_csr_s *to_x, *to_y, *from_x, *from_y;
	unsigned to_x_size, to_y_size, from_x_size, from_y_size;

	roam_to = kmr_select_new_target(roaming, roam_from, oob);

	/* Reset oob. */
	roam_from->x.oob = KMROOB_INSIDE;
	roam_from->y.oob = KMROOB_INSIDE;

	if (roam_from == roam_to)
		return 0;

	if (unlikely(roam_to->work_state != KMRWS_SCKT_READY))
		return 0;

	to_x = &roam_to->x;
	to_y = &roam_to->y;
	from_x = &roam_from->x;
	from_y = &roam_from->y;

	/* Change seq_num to trigger downstream report KB0 update. */
	roam_to->last_kbd0_out_report.seq_num--;
#if 0
	/* Do not do romaing if target active zone is unknown. */
	if (!kmr_target_got_active_zone(roam_to))
		return 0;
	if (!kmr_target_got_active_zone(roam_from))
		return 0;
#else
	to_x_size = (to_x->size) ? (to_x->size) : (to_x->default_size);
	to_y_size = (to_y->size) ? (to_y->size) : (to_y->default_size);
	from_x_size = (from_x->size) ? (from_x->size) : (from_x->default_size);
	from_y_size = (from_y->size) ? (from_y->size) : (from_y->default_size);
#endif
	/* initiate x/y */
	/*
	** We assume monitors are placed in equal width and height.
	** Just like a regular video wall placement.
	** Not like 'mosaic' type video wall placement.
	*/
	switch (oob) {
		case KMROOB_TOP:
			to_x->csr = from_x->csr * to_x_size / from_x_size;
			to_y->csr = to_y_size - KMR_BOUNDARY_MARGIN;
			break;
		case KMROOB_BOTTOM:
			to_x->csr = from_x->csr * to_x_size / from_x_size;
			to_y->csr = KMR_BOUNDARY_MARGIN;
			break;
		case KMROOB_LEFT:
			to_x->csr = to_x_size - KMR_BOUNDARY_MARGIN;
			to_y->csr = from_y->csr * to_y_size / from_y_size;
			break;
		case KMROOB_RIGHT:
			to_x->csr = KMR_BOUNDARY_MARGIN;
			to_y->csr = from_y->csr * to_y_size / from_y_size;;
			break;
		case KMROOB_MASTER: /* Roam back to center of master screen. */
			to_x->csr = to_x_size >> 1;
			to_y->csr = to_y_size >> 1;
			break;
		default:
			BUG();
	}
	kmr_set_cur_target(roaming, roam_to);
	/* return 0 means no roaming occur. Otherwise return 1. */
	return 1;
}

void kmr_try_roaming(roaming_s *roaming, kmr_target_s *target)
{
	kmr_csr_s *x, *y;

	x = &target->x;
	y = &target->y;

	if (likely(!(x->oob || y->oob))) {
		return;
	} else {
		unsigned oob = KMROOB_INSIDE;
#if 0
		char msg[1024];

		snprintf(msg, 1024, "e_kmoip_ms_edge_trg::%d::%d::%d::%d", 
		         x->size, y->size, x->csr, y->csr);

		ast_notify_user(msg);
#endif
		/*
		 * LIMIT:
		 * When both x and y are oob, x always has higher priority.
		 */
		if (x->oob) {
			if (x->oob == KMROOB_BR)
				oob = KMROOB_RIGHT;
			else
				oob = KMROOB_LEFT;
		} else if (y->oob){
			if (y->oob == KMROOB_BR)
				oob = KMROOB_BOTTOM;
			else
				oob = KMROOB_TOP;
		}

		if (likely(oob != KMROOB_INSIDE))
			kmr_do_roaming(roaming, target, oob);
	}
}

unsigned kmr_csr_remap_from_rel(roaming_s *roaming, kmr_csr_s *csr, __s32 value, __s32 tch0_size)
{
	/* Save position back to 'roaming' */
	csr->csr = _kmr_csr_from_rel(csr, value);
#if 0
	if (csr->oob) {
		char msg[1024];
		kmr_csr_s *x, *y;
		kmr_target_s *target = kmr_get_cur_target(roaming);
	
		x = &target->x;
		y = &target->y;

		snprintf(msg, 1024, "e_kmoip_ms_edge_trg::%d::%d::%d::%d", 
		         x->size, y->size, x->csr, y->csr);

		ast_notify_user(msg);
		csr->oob = 0;
	}
#endif
	return _kmr_csr_to_server_abs(csr, (unsigned)tch0_size);
}

unsigned kmr_csr_remap_from_abs(roaming_s *roaming, kmr_csr_s *csr, __s32 value, __s32 tch0_size, __s32 ori_size)
{
	/* Save position back to 'roaming' */
	csr->csr = _kmr_csr_from_abs(csr, value, ori_size);

	return _kmr_csr_to_server_abs(csr, (unsigned)tch0_size);
}

void kmr_update_active_zone(kmr_target_s *target, unsigned int active_x, unsigned int active_y)
{
	kmr_csr_s *x, *y;

	x = &target->x;
	y = &target->y;

	/* Handle a special case when KMR master first init. */
	if (kmr_target_is_master(target)) {
		if (!kmr_target_got_active_zone(target)) {
			/* Place the cursor in the middle of screen. */
			x->csr = active_x >> 1;
			y->csr = active_y >> 1;
		}
	}
	x->size = active_x;
	y->size = active_y;

	/* TODO. Someday, we should shadow required information and pre-caculate scale factors here. */
	/* Set follow flags to 0 to trigger re-caculation. */
	x->kmcsr_to_tch0_scale = 0;
	x->local_abs_to_kmcsr_scale = 0;
	y->kmcsr_to_tch0_scale = 0;
	y->local_abs_to_kmcsr_scale = 0;
}

