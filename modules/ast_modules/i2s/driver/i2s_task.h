/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_TASK_H
#define _I2S_TASK_H

#include <linux/completion.h>
//#include "../../cat6023/cat6023.h"

struct i2s_task {
	struct task_struct *thread;
	struct completion thread_done;
	char *name;
	void (*loop_ops)(struct i2s_task *);
//	Audio_Info old_audio_info;
	void *arg;
};

void i2s_task_init (struct i2s_task *, char *, void (*loop_ops)(struct i2s_task *));
void start_i2s_thread(struct i2s_task *);
void stop_i2s_thread(struct i2s_task *);

#endif
