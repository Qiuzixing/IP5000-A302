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
	//pthread_mutex_t selflock;
	P3KMsgQueueMember_S cmdMsg[MAX_QUEUE_BLOCK_NUMBER];
}P3KMsgQueueMng_S;

typedef struct _P3KMsgQueueInfo_S
{
	pthread_mutex_t selflock;
	P3KMsgQueueMng_S queueMng;
}P3KMsgQueueInfo_S;

static P3KMsgQueueInfo_S gs_queueInfo = {0};

static  int GetQueueMngHandle(P3KMsgQueueMng_S *tmng)
{
	if(tmng == NULL)
	{
		return -1;
	}
	memcpy(tmng,&(gs_queueInfo.queueMng),sizeof(P3KMsgQueueMng_S));
	return 0;
}
static  int SetQueueMngHandle(P3KMsgQueueMng_S *tmng)
{
	if(tmng == NULL)
	{
		return -1;
	}
  	memcpy(&(gs_queueInfo.queueMng),tmng,sizeof(P3KMsgQueueMng_S));
	return 0;
}
int P3K_MsgQueueInit()
{
	printf("<P3K_MsgQueueInit>\n");
	memset(&gs_queueInfo,0,sizeof(P3KMsgQueueInfo_S));
	memset(gs_queueInfo.queueMng.cmdMsg,0,sizeof(P3KMsgQueueMember_S));
	gs_queueInfo.queueMng.readPoint = 0;
	gs_queueInfo.queueMng.writePoint = 0;
	gs_queueInfo.queueMng.totalRecvMsg = 0;

	pthread_mutex_init(&(gs_queueInfo.selflock),NULL);


	return 0;
}
int P3K_MsgQueueUinit()
{
	printf("<P3K_MsgQueueUinit>\n");
	memset(&gs_queueInfo,0,sizeof(P3KMsgQueueInfo_S));
	gs_queueInfo.queueMng.readPoint = 0;
	gs_queueInfo.queueMng.writePoint = 0;
	gs_queueInfo.queueMng.totalRecvMsg = 0;

	pthread_mutex_destroy(&(gs_queueInfo.selflock));

	return 0;

}
int P3K_MSgQueueAddMsg(P3KMsgQueueMember_S *msg)
{
	
	if(msg == NULL)
	{
		return -1;
	}
	//printf("/----------111-------------//\n");
	pthread_mutex_lock(&(gs_queueInfo.selflock));
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
		
		//printf("------------111------------\n");
	}
	msgMng.writePoint++;
	if(msgMng.writePoint > (MAX_QUEUE_BLOCK_NUMBER-1))
	{
		msgMng.writePoint = 0;
	}

	SetQueueMngHandle(&msgMng);
	
	//printf("/----------11-------------//\n");
	pthread_mutex_unlock(&(gs_queueInfo.selflock));
	//printf("/---------1--------------//\n");
	return 0;
}
int P3K_MSgQueueGetMsg(P3KMsgQueueMember_S *msg)
{
	if(msg == NULL)
	{
		return -1;
	}
	
	pthread_mutex_lock(&(gs_queueInfo.selflock));
	//printf("selflock   Get===\n");
	P3KMsgQueueMng_S  msgMng;
	GetQueueMngHandle(&msgMng);
	if(msgMng.writePoint == msgMng.readPoint)
	{
		//DBG_InfoMsg("there is no msg\n");
		pthread_mutex_unlock(&gs_queueInfo.selflock);
		return -1;
	}
	
	memcpy(msg,&(msgMng.cmdMsg[msgMng.readPoint]),sizeof(P3KMsgQueueMember_S));
	msgMng.readPoint++;
	if(msgMng.readPoint> (MAX_QUEUE_BLOCK_NUMBER-1))
	{
			msgMng.readPoint = 0;
	}
	//printf("/---------P3K_MSgQueueGetMsg--------------/\n");
	SetQueueMngHandle(&msgMng);
	pthread_mutex_unlock(&(gs_queueInfo.selflock));
	//printf("selflock===%p\n",gs_queueMng.selflock);
	//printf("/---------P3K_MSgQueueGetMsg1--------------/\n");
	return 0;

}


