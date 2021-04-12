/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _AST_TASK_H_
#define _AST_TASK_H_

#include <linux/completion.h>

struct ast_task {
	struct task_struct *thread;
	struct completion thread_done;
	char *name;
	void (*loop_ops)(struct ast_task *);
};

void ast_task_init(struct ast_task *, char *, void (*loop_ops)(struct ast_task *));
void ast_start_thread(struct ast_task *);
void ast_stop_thread(struct ast_task *);

#endif /* #ifndef _TASK_H_ */
