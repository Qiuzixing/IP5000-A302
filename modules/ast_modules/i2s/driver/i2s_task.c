/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <linux/module.h>
#include "i2s_task.h"

void i2s_task_init(struct i2s_task *i2st, char *name, void (*loop_ops)(struct i2s_task *))
{
	i2st->thread = NULL;
	init_completion(&i2st->thread_done);
	i2st->name = name;
	i2st->loop_ops = loop_ops;
	i2st->arg = NULL;
}

static int i2s_daemon(struct i2s_task *i2st)
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

void start_i2s_thread (struct i2s_task *i2st)
{
//	if (!i2st)
//		return;
	kernel_thread((int (*)(void *))i2s_daemon, (void *)i2st, 0);
	wait_for_completion (&i2st->thread_done);
}

void stop_i2s_thread (struct i2s_task *i2st)
{
	if (i2st->thread != NULL) {
		send_sig(SIGKILL, i2st->thread, 1);
		wait_for_completion(&i2st->thread_done);
		i2st->thread = NULL;
	}
	/*
	** Bruce120119. init_completion for re-use. Fix Bug#2010102500.
	** Sometimes ud->tcp_rx.thread will be set to NULL before usbip_stop_threads()
	** been called. And so that above wait_for_completion() won't be called.
	** If we re-use this thread, and next time when we call usbip_stop_threads()
	** usbip_stop_threads() will return immediately because the wait_for_completion()
	** counter is not zero. This is how it hit Bug#2010102500.
	*/
	init_completion(&i2st->thread_done);
}
