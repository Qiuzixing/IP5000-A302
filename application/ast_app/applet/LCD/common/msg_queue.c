#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "msg_queue.h"

#define PROJ_ID			66
#define MSG_MAGIC		0x55

static int msg_id = -1;
int msg_queue_create(void)
{
	key_t key = ftok("/var/run", PROJ_ID);
	if(key == -1){
		perror("ftok");
		return -1;
	}

	msg_id = msgget(key, 0666 | IPC_CREAT);
	if(msg_id == -1){
		perror("msgget");
		return -1;
	}
	return 0;
}

int msg_queue_destroy()
{
	int err = 0;
	if (msg_id != -1)
	{
		err = msgctl(msg_id, IPC_RMID, NULL);
		if(err != 0)
			printf("%s msg_queue_destroy =%s\n", __FILE__, strerror(errno));
	}
	msg_id = -1;
	return err;
}

int msg_send_state(int state)
{
	int err = 0;
	msg_buf_t msg;
	msg.mtype = MSG_MAGIC;
	msg.mtext[0] = state;
	err = msgsnd(msg_id, (void *)&msg, MAX_SIZE, 0);
	if(err < 0){
		printf("%s(..) msg_send fail= %s\n", __func__, strerror(errno));
		return -1;
	}
	return err;
}

int msg_recv_state(msg_buf_t *msg)
{
	if(msgrcv(msg_id, msg, MAX_SIZE, 0, 0) == -1){
		printf("%s:recv state fail= %s\n", __func__, strerror(errno));
		return -1;
	}
	return 0;
}

