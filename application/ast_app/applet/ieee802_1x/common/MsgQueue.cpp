
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

#include "MsgQueue.h"
#include "log.h"

#define QueuePath			"/var/run/"
#define PROJ_ID				66

int MessageQueue::QueueInit()
{
	GBLog_Log(LL_INFO, "MessageQueue::QueueInit() enter");
	key_t key;
	key = ftok(QueuePath, PROJ_ID);
	if (key == -1)
	{
		GBLog_Log(LL_INFO, "ftok fail:[%s]", strerror(errno));
		return -1;
	}

	m_s32QueueId = msgget(key, IPC_CREAT);
	if (m_s32QueueId == -1)
	{
		GBLog_Log(LL_INFO, "msgget fail:[%s]", strerror(errno));
		return -1;
	}

	return 0;
}

int MessageQueue::QueueDeinit()
{
	int err = 0;
	if (m_s32QueueId != -1)
	{
		err = msgctl(m_s32QueueId, IPC_RMID, NULL);
		if(err != 0)
		GBLog_Log(LL_INFO, "msgctl fail:[%s]", strerror(errno));
	}
	
	return err;
}


int MessageQueue::MsgSend(int i_s32Status)
{
	int err;
	struct MsgBuf Msg;
	Msg.mtype = MSG_MAGIC;
	Msg.mtext[0] = i_s32Status;
		
	 err = msgsnd(m_s32QueueId, &Msg, TEXT_SIZE, 0);
	 if (err == -1)
	 {
	 	GBLog_Log(LL_INFO, "msgsnd fail:[%s]", strerror(errno));
		return -1;
	 }
	return 0;
}

int MessageQueue::MsgRecv(struct MsgBuf *i_pMsg)
{ 
	int err;
	err = msgrcv(m_s32QueueId, i_pMsg, TEXT_SIZE, 0, 0);
	if (err == -1)
	{
		GBLog_Log(LL_INFO, "msgrc fail:[%s]", strerror(errno));
		return -1;
	}
	return 0;
}




