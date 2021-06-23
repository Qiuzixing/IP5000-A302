#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "handlelist.h"
#include "p3kpacket.h"
#include "p3kapi.h"
#include "p3kswitch.h"
#include "p3kmsgqueue.h"
#include "debugtool.h"
#define MAX_HANDLE 10
//static P3K_QeqCmdProcess gs_callHandle = NULL;

typedef struct _P3KReqistMsg_S
{
	int handleId;  //注册获得handleId
	P3K_MsgSend sendMsg;    //发送消息
}P3KReqistMsg_S;
typedef struct _P3KRegistHandleMng_S
{
	pthread_mutex_t selfLock;
	P3KReqistMsg_S handle;
	HandleList_S listHandleHead;
}P3KRegistHandleMng_S;


typedef struct _P3kApiMng_S
{
	int enable;
	int recvMsgSum;
	int excuteMsgSum;
}P3kApiMng_S;
static  P3KRegistHandleMng_S gs_handleMng = {0};
static  P3kApiMng_S gs_modleMng = {0};

static HandleList_S* P3K_GetMngHandleHead();
static  int P3K_HandleListInit()
{
	memset(&gs_handleMng,0,sizeof(P3KRegistHandleMng_S));
	pthread_mutex_init(&gs_handleMng.selfLock,NULL);
	return 0;
}
static int P3K_HandleListUnInit()
{
	P3KReqistMsg_S *resultHandle=NULL;

	int tempUsrID = 0;
	do
	{
		tempUsrID++;
		HandleManageGetHandleByInt(P3K_GetMngHandleHead(), handleId, tempUsrID, resultHandle, P3KReqistMsg_S);
		if(resultHandle != NULL)
		{
			HandleManageDelHandle(P3K_GetMngHandleHead(), (void *)resultHandle);
			//break;
		}
	}while( tempUsrID < MAX_HANDLE);
	if(resultHandle != NULL)
	{
		DBG_ErrMsg("there is regist handle no unregist\n");
		return -1;
	}
       memset(&gs_handleMng,0,sizeof(P3KRegistHandleMng_S));
	pthread_mutex_destroy(&gs_handleMng.selfLock);
	return 0;

}
static int P3K_ApiMngListInit()
{
	memset(&gs_modleMng,0,sizeof(P3kApiMng_S));
	gs_modleMng.enable = 1;
	return 0;
}
static int P3K_ApiMngListUnInit()
{
	gs_modleMng.enable = 0;
	memset(&gs_modleMng,0,sizeof(P3kApiMng_S));
	
	return 0;
}
static int P3K_GetApiInitFlag()
{

	return gs_modleMng.enable ;
}
static int P3K_GetRecvMsgSum()
{
	int tmp = gs_modleMng.recvMsgSum;
	return tmp;
}
static int P3K_AddRecvMsgSum(int num)
{
	gs_modleMng.recvMsgSum += num;
	return 0;
}
static HandleList_S* P3K_GetMngHandleHead()
{

	return &(gs_handleMng.listHandleHead);
}
static int P3K_MakeUsrID()//注册ID
{
	P3KReqistMsg_S *resultHandle=NULL;

	int tempUsrID = 0;
	do
	{
		tempUsrID++;
		HandleManageGetHandleByInt(P3K_GetMngHandleHead(), handleId, tempUsrID, resultHandle, P3KReqistMsg_S);		
	}while(resultHandle != NULL && tempUsrID < MAX_HANDLE);
	if(resultHandle != NULL)
	{
		return -1;
	}
	return tempUsrID;
}
static P3KReqistMsg_S* P3K_GetReqistMsgByID(int id)//通过ID获取Handle
{
	P3KReqistMsg_S *handle = NULL;
	void *result = NULL;
	HandleManageGetHandleByInt(P3K_GetMngHandleHead(),handleId,id,result,P3KReqistMsg_S);
	handle = (P3KReqistMsg_S *)result;
	if(handle == NULL)
	{
		DBG_ErrMsg("handle is NULL\n");
		return NULL;
	}
	return handle;
	
}

static void * P3K_DataExcuteProc(void*arg)
{
	P3KMsgQueueMember_S pmsg;
	P3KReqistMsg_S *registMsg = NULL;
	P3K_SimpleCmdInfo_S respCmdInfo;
	char dstdata[512] = {0};
	int s32Ret = 0;
	int tmplen = 0;
	char userDefine[MAX_USR_STR_LEN+1] = {0};
	//函数处理执行线程
	prctl(PR_SET_NAME, (unsigned long)"P3K_DataExcuteProc", 0,0,0);
	while(P3K_GetApiInitFlag())
	{
		memset(&pmsg,0,sizeof(P3KMsgQueueMember_S));
		
		s32Ret = P3K_MSgQueueGetMsg(&pmsg);
		if(s32Ret != 0)
		{
            			usleep(20*1000);
				continue;
		}
		memset(&respCmdInfo,0,sizeof(P3K_SimpleCmdInfo_S));
		//解析处理数据
		//特殊命令分开执行可放在这里?
	
		memset(userDefine,0,MAX_USR_STR_LEN);
		printf("----------------------------------\n");
		s32Ret = P3K_SilmpleReqCmdProcess(&pmsg.cmdinfo,&respCmdInfo,userDefine);
		if(s32Ret != 0)
		{
			continue;
		}
		memset(dstdata,0,sizeof(dstdata));
		//组包
		tmplen = P3K_SimpleRespCmdBurstification(&respCmdInfo, dstdata);
		//发送数据
		registMsg = P3K_GetReqistMsgByID(pmsg.handleId);
		if(registMsg)
		{
			registMsg->sendMsg(pmsg.handleId,dstdata,tmplen);
			if(strlen(userDefine) > 0)
			{
				registMsg->sendMsg(pmsg.handleId,userDefine,strlen(userDefine));
			}
		}
	}

	return  NULL;
}



int P3K_DataExcuteThread()
{
	int s32Ret = 0;
	pthread_t tExcute;
	s32Ret = pthread_create(&tExcute, NULL, P3K_DataExcuteProc, NULL);
	if(s32Ret)
	{
		return -1;
	}
       pthread_detach(tExcute);
	return 0;
}
static int P3K_RecvMessage(int handleId,char*data,int len )
{
	if(data == NULL)
	{
		return -1;
	}
	P3K_SimpleCmdInfo_S cmd[16];
	P3KMsgQueueMember_S queueMmber;
	int cmdCount = 0;
	int i = 0;
	memset(cmd,0,sizeof(P3K_SimpleCmdInfo_S)*16);


	cmdCount = P3K_SimpleReqPacketUnpack(data,len,cmd);
	if(cmdCount<= 0)
	{
		//特殊命令附加数据
		
	}
	else
	{
		DBG_InfoMsg("cmdcount=%d\n",cmdCount);
	}
	for(i = 0 ;i < cmdCount;i++)
	{
		//特殊命令分开执行
		if(P3K_CheckedSpeciCmd(cmd[i].command))
		{
			
			DBG_InfoMsg("SPECIAL CMD =%s\n",cmd[i].command);
			continue;
		}
		//握手消息
		if(strlen(cmd[i].command)==0)
		{
			char handleshark[16] ={0};
			P3KReqistMsg_S *registMsg = NULL;
			
			P3K_HandleSharkResp(handleshark);
			registMsg = P3K_GetReqistMsgByID(handleId);
			if(registMsg)
			{
				registMsg->sendMsg(handleId,handleshark,strlen(handleshark));
			}
			continue;
		}
		//通用命令放入缓存
		memset(&queueMmber,0,sizeof(queueMmber));
		queueMmber.handleId =handleId;
		memcpy(&queueMmber.cmdinfo,&cmd[i],sizeof(P3K_SimpleCmdInfo_S));
		P3K_MSgQueueAddMsg(&queueMmber);
	}
}
int P3K_ApiInit()
{
	int s32Ret = 0;
 	if( P3K_GetApiInitFlag())
 	{
 		DBG_ErrMsg("P3K_Api already init\n");
		return -1;
	}
	P3K_ApiMngListInit();
	P3K_HandleListInit();
	//消息队列初始化
	s32Ret = P3K_MsgQueueInit();
	if(s32Ret)
	{
		DBG_ErrMsg("P3K_MsgQueueInit init err\n");
		return -1;
	}
	//消息处理线程
	
	s32Ret = P3K_DataExcuteThread();
	if(s32Ret != 0)
	{
		DBG_ErrMsg("P3K_DataExcuteThread start err\n");
		return -1;
	}
	return 0;
}

int P3K_APIUnInit()
{
	P3K_HandleListUnInit();
	P3K_ApiMngListUnInit();
	P3K_MsgQueueUinit();
	return 0;
}


//注册
int P3K_ApiRegistHandle(P3KApiHandle_S*handle)
{
	if(handle == NULL)
	{

		return -1;
	}
	int phandleId = 0;
	int s32Ret = 0;
	P3KReqistMsg_S  *registMsg =(P3KReqistMsg_S *)malloc(sizeof(P3KReqistMsg_S));
	phandleId = P3K_MakeUsrID();
	if(phandleId <= 0)
	{
		DBG_ErrMsg("handleID over the  range\n");
		return -1;
	}
	handle->P3kMsgRecv = P3K_RecvMessage;
	handle->handleId = phandleId;
	memset(registMsg,0,sizeof(P3KReqistMsg_S));
	registMsg->handleId = phandleId;
	if(handle->sendMsg != NULL)
	{
		registMsg->sendMsg = handle->sendMsg;
	}
	//join to handle list
	s32Ret = HandleManageAddHandle(P3K_GetMngHandleHead(),( void *) registMsg);
	if(s32Ret != 0)
	{
		free(registMsg);
		DBG_ErrMsg("join to  handle list err\n");
		return -1;
	}
	return 0;
}
//注销
int P3K_ApiUnRegistHandle(P3KApiHandle_S*handle)
{
	if(handle == NULL)
	{
		return -1;
	}
	P3KReqistMsg_S *registMsg = P3K_GetReqistMsgByID(handle->handleId);
	if(registMsg == NULL)
	{
		return -1;
	}
	HandleManageDelHandle(P3K_GetMngHandleHead(),(void*)registMsg);
	registMsg->sendMsg = NULL;
	registMsg->handleId = 0;
	handle->handleId = 0;
	handle->P3kMsgRecv = NULL;
	
	return 0;
}
int P3K_ApiGetTotalRecvCmd(int *sum)
{

	return 0;
}
int P3K_ApiGetTotalExcuteCmd(int *sum)
{

	return 0;
}

