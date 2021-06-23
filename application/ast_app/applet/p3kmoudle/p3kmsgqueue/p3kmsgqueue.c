#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "p3kmsgqueue.h"
#include "debugtool.h"

typedef struct _P3KMsgQueueMng_S
{

	int totalRecvMsg;
	int readPoint;
	int writePoint;
	pthread_mutex_t selflock;
	P3KMsgQueueMember_S cmdMsg[MAX_QUEUE_BLOCK_NUMBER];
}P3KMsgQueueMng_S;

static P3KMsgQueueMng_S gs_queueMng = {0};

static  int GetQueueMngHandle(P3KMsgQueueMng_S *tmng)
{
	if(tmng == NULL)
	{
		return -1;
	}
	memcpy(tmng,&(gs_queueMng),sizeof(P3KMsgQueueMng_S));
	return 0;
}
static  int SetQueueMngHandle(P3KMsgQueueMng_S *tmng)
{
	if(tmng == NULL)
	{
		return -1;
	}
  	memcpy(&(gs_queueMng),tmng,sizeof(P3KMsgQueueMng_S));
	return 0;
}
int P3K_MsgQueueInit()
{
	
	memset(&gs_queueMng,0,sizeof(P3KMsgQueueMng_S));
	memset(gs_queueMng.cmdMsg,0,sizeof(P3KMsgQueueMember_S));
	gs_queueMng.readPoint = 0;
	gs_queueMng.writePoint = 0;
	gs_queueMng.totalRecvMsg = 0;

	pthread_mutex_init(&gs_queueMng.selflock,NULL);


	return 0;
}
int P3K_MsgQueueUinit()
{
	memset(&gs_queueMng,0,sizeof(P3KMsgQueueMng_S));
	gs_queueMng.readPoint = 0;
	gs_queueMng.writePoint = 0;
	gs_queueMng.totalRecvMsg = 0;

	pthread_mutex_destroy(&gs_queueMng.selflock);
}
int P3K_MSgQueueAddMsg(P3KMsgQueueMember_S *msg)
{
	
	if(msg == NULL)
	{
		return -1;
	}
	pthread_mutex_lock(&gs_queueMng.selflock);
	P3KMsgQueueMng_S  msgMng;
	
	GetQueueMngHandle(&msgMng);
	//DBG_InfoMsg(" wpoint =%d rpoint=%d\n",msgMng.writePoint,msgMng.readPoint);
	memset(&(msgMng.cmdMsg[msgMng.writePoint]),0,sizeof(P3KMsgQueueMember_S));
	memcpy(&(msgMng.cmdMsg[msgMng.writePoint]),msg,sizeof(P3KMsgQueueMember_S));
	if((msgMng.writePoint ==( msgMng.readPoint-1)) ||
	( (msgMng.writePoint == (MAX_QUEUE_BLOCK_NUMBER-1))&&(msgMng.readPoint == 0)))
	{
		msgMng.readPoint++;
		if(msgMng.readPoint> (MAX_QUEUE_BLOCK_NUMBER-1))
		{
			msgMng.readPoint = 0;
		}
		
		//printf("------------11------------\n");
	}
	msgMng.writePoint++;
	if(msgMng.writePoint > (MAX_QUEUE_BLOCK_NUMBER-1))
	{
		msgMng.writePoint = 0;
	}

	SetQueueMngHandle(&msgMng);
	
	//printf("-----------------------\n");
	pthread_mutex_unlock(&gs_queueMng.selflock);
	//printf("---------1--------------\n");
	return 0;
}
int P3K_MSgQueueGetMsg(P3KMsgQueueMember_S *msg)
{
	if(msg == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&gs_queueMng.selflock);
	P3KMsgQueueMng_S  msgMng;
	GetQueueMngHandle(&msgMng);
	if(msgMng.writePoint == msgMng.readPoint)
	{
		//DBG_InfoMsg("there is no msg\n");
		pthread_mutex_unlock(&gs_queueMng.selflock);
		return -1;
	}
	
	memcpy(msg,&(msgMng.cmdMsg[msgMng.readPoint]),sizeof(P3KMsgQueueMember_S));
	msgMng.readPoint++;
	if(msgMng.readPoint> (MAX_QUEUE_BLOCK_NUMBER-1))
	{
			msgMng.readPoint = 0;
	}
	SetQueueMngHandle(&msgMng);
	pthread_mutex_unlock(&gs_queueMng.selflock);
	return 0;

}


