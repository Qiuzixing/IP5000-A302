/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <linux/module.h>
#include "ast_task.h"

void ast_task_init(struct ast_task *task, char *name, void (*loop_ops)(struct ast_task *))
{
	task->thread = NULL;
	init_completion(&task->thread_done);

	task->name = name;
	task->loop_ops = loop_ops;
}

static int daemon(struct ast_task *task)
{
	lock_kernel();
	daemonize(task->name);
	allow_signal(SIGKILL);
	task->thread = current;
	unlock_kernel();

	complete (&task->thread_done);
	task->loop_ops(task);

	task->thread = NULL;
	complete_and_exit (&task->thread_done, 0);
}

void ast_start_thread(struct ast_task *task)
{
	kernel_thread((int (*)(void *))daemon, (void *)task, 0);
	wait_for_completion (&task->thread_done);
}

void ast_stop_thread(struct ast_task *task)
{
	if (task->thread != NULL) {
		send_sig(SIGKILL, task->thread, 1);
		wait_for_completion(&task->thread_done);
		task->thread = NULL;
	}
	/*
	** Bruce120119. init_completion for re-use. Fix Bug#2010102500.
	** Sometimes ud->tcp_rx.thread will be set to NULL before usbip_stop_threads()
	** been called. And so that above wait_for_completion() won't be called.
	** If we re-use this thread, and next time when we call usbip_stop_threads()
	** usbip_stop_threads() will return immediately because the wait_for_completion()
	** counter is not zero. This is how it hit Bug#2010102500.
	*/
	init_completion(&task->thread_done);
}
