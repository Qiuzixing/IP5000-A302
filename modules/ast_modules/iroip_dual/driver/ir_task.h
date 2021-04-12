/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _IR_TASK_H
#define _IR_TASK_H

#include <linux/completion.h>

struct ir_task {
	struct task_struct *thread;
	struct completion thread_done;
	char *name;
	void (*loop_ops)(struct ir_task *);
	void *pContext;
//	Audio_Info old_audio_info;
};

void ir_task_init (struct ir_task *, char *, void (*loop_ops)(struct ir_task *));
void start_ir_thread(struct ir_task *);
void stop_ir_thread(struct ir_task *);

#endif
