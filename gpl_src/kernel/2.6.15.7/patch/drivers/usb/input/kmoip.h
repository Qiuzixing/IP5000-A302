/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef __KMOIP_H__
#define __KMOIP_H__

#include <linux/device.h> //struct device_attribute
#include <linux/fs.h> //for inode
#include <linux/list.h>
#include <linux/workqueue.h>

//#include "hid.h"
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <asm/arch/drivers/kmoip_common.h>

#include "kmoip-roaming.h"

struct hid_device;

#define MAX_HIDDEV_NUM 32
#define KMOIP_REPORT_LIST_SIZE 16
#define KMOIP_MAX_HOTKEY_SLOT 8

#define KDT_NOT_VALID  0
#define KDT_KB         (1 << 0) /* keyboard */
#define KDT_MS         (1 << 1) /* mouse */
#define KDT_MT         (1 << 2) /* multi-touch */

typedef void (*work_handler)(void *);

struct ast_hiddev {
	struct list_head list;
	int exist;
	struct hid_device *hid;
	unsigned kmoip_dev_type; /* KDT_KB, KDT_MS or KDT_MT */
};

struct kmoip_drv {
	struct cdev cdev;
	struct platform_device *pdev; //A pointer back to platform device.
	struct hid_device *kbd0_hiddev; //A "fake" hid_device data structure used to construct my virtual keyboard report.
	struct hid_device *kbd1_hiddev; //A "fake" hid_device data structure used to construct my virtual keyboard report.
	struct hid_device *ms0_hiddev; //A "fake" hid_device data structure used to construct my virtual mouse report.
	struct hid_device *tch0_hiddev; //A "fake" hid_device data structure used to construct my virtual touch0 report.
	unsigned int kbd0_event_pending;
	unsigned int kbd1_1_event_pending;
	unsigned int kbd1_3_event_pending;
	unsigned int ms0_event_pending;
	unsigned int tch0_event_pending;
	spinlock_t lock; //generic driver lock
	unsigned int id;

	struct ast_hiddev hiddevs[MAX_HIDDEV_NUM];
	struct list_head ast_hiddev_list;
	struct list_head ast_hiddev_free_list;

	unsigned int is_socket_ready;

	struct workqueue_struct *uwq; //upstream work queue
	struct work_struct init_work; //work to init the wq thread. (set nice).
	unsigned int seq_num;
	struct list_head report_free_list;
	struct kmoip_report reports[KMOIP_REPORT_LIST_SIZE];
	struct kmoip_report last_kbd0_out_report;

	unsigned int heartbeat_interval;
	unsigned int show_keycode;
	unsigned char hotkey[KMOIP_MAX_HOTKEY_SLOT][2];

	unsigned long token_interval; //default 100ms (in jiffies)

	work_handler upstream_func;
	work_handler heartbeat_func;
	work_handler downstream_func;

	roaming_s roaming;
};

extern int __init kmoip_init(void);
extern void kmoip_exit(void);
extern int kmoip_connect(struct hid_device *hid);
extern void kmoip_disconnect(struct hid_device *hid);
extern void kmoip_kick_upstream_handler(struct kmoip_drv *kmdrv, kmr_target_s *target);
extern void hotkey_filter(struct kmoip_drv *kmdrv, struct kmoip_report *report);

extern int hidkm_report_event(struct hid_device *hid, struct hid_report *report);
extern int hidkm_process_event(struct hid_device *hid, struct hid_field *field, struct hid_usage *usage, __s32 value, struct pt_regs *regs);
extern struct hid_device *ast_create_hid_class_device(char rdesc[], int rsize, char bInterfaceNumber);
extern int hidkm_input_event(struct hid_device *hid, struct kmoip_report *report);

extern void enqueue_report_pool_fifo(struct kmoip_drv *kmdrv, struct kmoip_report *report, int lock);
extern struct kmoip_report *dequeue_report_pool_fifo(struct kmoip_drv *kmdrv, int lock);
extern void enqueue_report_fifo(struct kmoip_drv *kmdrv, kmr_target_s *target, struct kmoip_report *report, int lock);
extern struct kmoip_report *dequeue_report_fifo(struct kmoip_drv *kmdrv, kmr_target_s *target, int lock);

#endif //#ifndef __KMOIP_H__

