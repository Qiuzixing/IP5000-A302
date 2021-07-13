#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#define MAX_SIZE		1
typedef struct msg_buf{
	long mtype;
	char mtext[MAX_SIZE];
}msg_buf_t;

int msg_queue_create(void);
int msg_queue_destroy(void);
int msg_send_state( int state);
int msg_recv_state(msg_buf_t *msg);

#endif
