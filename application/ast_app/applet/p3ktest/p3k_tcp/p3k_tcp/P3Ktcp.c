#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketapi.h"
#include "p3kapi.h"
#include "handlelist.h"
#include "debugtool.h"
SocketWorkInfo_S gs_netHandle = {0};

int flag = 0;
typedef struct _TcpCliToP3KInfo_S
{
	int sockfd;
	P3KApiHandle_S p3kHandle;
}TcpCliP3KInfo_S;
typedef struct _TcpCliMng_S
{
	TcpCliP3KInfo_S info;
	HandleList_S listHandleHead;
}TcpCliMng_S;

static TcpCliMng_S gs_cliHandleMng = {0};
static SocketWorkInfo_S*Tcp_NetGetNetReristHandle()
{

	return &gs_netHandle;
}
static HandleList_S* Tcp_NetGetMngHandleHead()
{

	return &(gs_cliHandleMng.listHandleHead);
}
static TcpCliP3KInfo_S* Tcp_NetGetHandleMsgByID(int sockFd)//通过ID获取Handle
{
	TcpCliP3KInfo_S *handle = NULL;
	void *result = NULL;
	HandleManageGetHandleByInt(Tcp_NetGetMngHandleHead(),sockfd,sockFd,result,TcpCliP3KInfo_S);
	handle = (TcpCliP3KInfo_S *)result;
	if(handle == NULL)
	{
		DBG_ErrMsg("handle is NULL\n");
		return NULL;
	}
	return handle;
	
}
static TcpCliP3KInfo_S* Tcp_NetGetP3KHandleMsgByID(int handleId)//通过ID获取Handle
{
	TcpCliP3KInfo_S *handle = NULL;
	void *result = NULL;
	HandleManageGetHandleByInt(Tcp_NetGetMngHandleHead(),p3kHandle.handleId,handleId,result,TcpCliP3KInfo_S);
	handle = (TcpCliP3KInfo_S *)result;
	if(handle == NULL)
	{
		DBG_ErrMsg("handle is NULL\n");
		return NULL;
	}
	return handle;
	
}
static int Tcp_P3KHandleListUnInit()
{
	TcpCliP3KInfo_S *resultHandle=NULL;

		for(;;)
		{
			resultHandle = HandleManageGetNextHandle(Tcp_NetGetMngHandleHead());
			if(resultHandle!=NULL)
			{
				P3K_ApiUnRegistHandle(&(resultHandle->p3kHandle));
				HandleManageDelHandle(Tcp_NetGetMngHandleHead(), resultHandle);
				resultHandle = NULL;
			}
			else
			{
				break;
			}
		}
	return 0;

}
int Tcp_NetSenddata(int userId,char*data,int len)
{

	//USR id找到对应socket
	 TcpCliP3KInfo_S* info = Tcp_NetGetP3KHandleMsgByID(userId);
	if(info == NULL)
	{
		return -1;
	}
	SOCKET_TcpSendMessage(info->sockfd,data,len);
	return 0;
}

void * LoginTimtOut(void * fd)
{
	int socket_fd = *(int *)fd;
	sleep(10);
	if(flag == 0)
	{
		printf("timeout");
		close(socket_fd);
	}
	pthread_exit(NULL);
}


int Tcp_NetRecvMsg(NetCliInfo_T *cli)
{
	//如果是新socket则注册

	pthread_t pth_time;
	pthread_create(&pth_time, NULL, LoginTimtOut, &cli->recvSocket);
	int ret = 0;
	int handleId = 0;
	TcpCliP3KInfo_S *info = NULL;
	info = Tcp_NetGetHandleMsgByID(cli->recvSocket);
	if(info == NULL)
	{
			TcpCliP3KInfo_S * hd =(TcpCliP3KInfo_S*)malloc(sizeof(TcpCliP3KInfo_S));
			memset(hd,0,sizeof(TcpCliP3KInfo_S));
			hd->sockfd =cli->recvSocket;
			hd->p3kHandle.sendMsg = Tcp_NetSenddata;
			ret = P3K_ApiRegistHandle(& hd->p3kHandle);
			if(ret != 0)
			{
				free(hd);
				DBG_ErrMsg("P3K_ApiRegistHandle err\n ");
				return -1;
			}
			HandleManageAddHandle(Tcp_NetGetMngHandleHead (), (void *)hd);
			info = hd;
			
	}
	if(!memcmp(cli->recvmsg,"#LOGIN admin,33333",strlen("#LOGIN admin,33333")))
	{
		flag = 1;
	}
	//数据下发
	handleId = info->p3kHandle.handleId;
	info->p3kHandle.P3kMsgRecv(handleId,cli->recvmsg,cli->recvLen);
	
	return 0;
}
int Tcp_NetClose(int sockeFd)
{
	//注销
	 TcpCliP3KInfo_S* info = Tcp_NetGetP3KHandleMsgByID(sockeFd);
	if(info )
	{
		P3K_ApiUnRegistHandle(&(info->p3kHandle));
		free(info);
	}
	return 0;
}
int Tcp_NetGetNetCab(NetCab_T *cab)
{
	return 0;
}

int Tcp_NetInit(int port)
{
	P3K_ApiInit();
	SocketWorkInfo_S*handle = Tcp_NetGetNetReristHandle();
	handle->readcb =Tcp_NetRecvMsg;
	handle->closecb = Tcp_NetClose;
	handle->serverport = port;
//	handle->getNetCabInfo = Tcp_NetGetNetCab;
	SOCKET_CreateTcpServer(handle);

	//P3K_ApiRegistHandle(& handle);
	return 0;
}
int Tcp_NetUnInit()
{
	
	SOCKET_DestroyTcpServer(Tcp_NetGetNetReristHandle());
	Tcp_P3KHandleListUnInit();
	P3K_APIUnInit();
	return 0;
}
int main ()
{
	Tcp_NetInit(5000);
	char ch =0;
	while(1)
	{
		ch =getchar();
		if(ch == 'q')
		{
			break;
		}
		usleep(1000000);
	}
	Tcp_NetUnInit();
	return 0;
}