/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <linux/module.h>
#include "ir_task.h"

void ir_task_init(struct ir_task *i2st, char *name, void (*loop_ops)(struct ir_task *))
{
	i2st->thread = NULL;
	init_completion(&i2st->thread_done);
	i2st->name = name;
	i2st->loop_ops = loop_ops;
}

static int ir_daemon(struct ir_task *i2st)
{
	lock_kernel();
	daemonize(i2st->name);
	allow_signal(SIGKILL);
	i2st->thread = current;
	unlock_kernel();

	complete (&i2st->thread_done);
	i2st->loop_ops(i2st);
	
	i2st->thread = NULL;
	complete_and_exit (&i2st->thread_done, 0);
}

void start_ir_thread (struct ir_task *i2st)
{
//	if (!i2st)
//		return;
	kernel_thread((int (*)(void *))ir_daemon, (void *)i2st, 0);
	wait_for_completion (&i2st->thread_done);
}

void stop_ir_thread (struct ir_task *i2st)
{
	if (i2st->thread != NULL) {
		send_sig(SIGKILL, i2st->thread, 1);
		wait_for_completion(&i2st->thread_done);
		i2st->thread = NULL;
	}
}
