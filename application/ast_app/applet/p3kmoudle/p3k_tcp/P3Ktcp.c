#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketapi.h"
#include "p3kapi.h"
#include "handlelist.h"
#include "debugtool.h"
#include "P3Ktcp.h"
#include "../p3klib/cfgparser.h"
SocketWorkInfo_S gs_netHandle = {0};
SocketWorkInfo_S gs_netHandle1 = {0};

int flagS =0;
typedef struct _TimeOut_S
{
	int  soket;
	int flag ;
	struct _TimeOut_S * next;
}TimeOut_S;
TimeOut_S * sTimeOut = NULL;

typedef struct _TcpLogin_S
{
	int iFlag;
	int num;
	char socketId[256];
}TcpLogin_S;
static TcpLogin_S sTcpLogin = {0};

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

static SocketWorkInfo_S*Tcp_NetGetNetReristHandle1()
{

	return &gs_netHandle1;
}


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
void Create_TimeHead(TimeOut_S ** head)
{
	TimeOut_S * tmp = (TimeOut_S *)malloc(sizeof(TimeOut_S));
	tmp->flag =0;
	tmp->soket = 0;
	tmp->next =NULL;
	*head = tmp;
}
void Distory_TimeHead(TimeOut_S * head)
{
	if(head == NULL)
	{
		return;
	}
	TimeOut_S * tmp = NULL;
	while(head != NULL)
	{
		tmp = head->next;
		free(head);
		head = tmp;
	}
}
void HeadInsert(TimeOut_S * head,TimeOut_S * pnew)
{
	if (head == NULL || pnew == NULL)
		return;
	pnew->next = head->next;//将新的节点的next指针指向首节点
	head->next = pnew;//头结点的next指针指向新的节点
}
void SearchTcp(TimeOut_S * head,int socket)
{
	if(head == NULL)
	{
		return;
	}
	TimeOut_S * tmp = head->next;
	while(tmp != NULL)
	{
		if(tmp->soket == socket)
		{
			tmp->flag =1;
			return;
		}
		tmp = tmp->next;
	}
	return;
}

void LogoutTcp(TimeOut_S * head,int socket)
{
	if(head == NULL)
	{
		return;
	}
	TimeOut_S * tmp = head->next;
	while(tmp != NULL)
	{
		if(tmp->soket == socket)
		{
			tmp->flag =0;
			return;
		}
		tmp = tmp->next;
	}
	return;
}
int Cheak_TcpStartLink(TimeOut_S * head,int socket)
{
	if(head == NULL)
	{
		return;
	}
	TimeOut_S * tmp = head->next;
	while(tmp != NULL)
	{
		if(tmp->soket == socket )
		{
			return tmp->soket;
		}
		tmp = tmp->next;
	}
	return 0;
}

int Cheak_TcpLink(TimeOut_S * head,int socket)
{
	if(head == NULL)
	{
		return;
	}
	TimeOut_S * tmp = head->next;
	while(tmp != NULL)
	{
		if(tmp->soket == socket && tmp->flag == 1)
		{
			return tmp->soket;
		}
		tmp = tmp->next;
	}
	return 0;
}
void * LoginTimtOut(void * fd)
{
	int soket = *(int *)fd;
	sleep(300);
	TimeOut_S * tmp = sTimeOut->next;
	TimeOut_S * rec = sTimeOut;
	while(tmp != NULL)
	{
		if(tmp->soket == soket && tmp->flag ==0)
		{
			DBG_ErrMsg("timeout\n");
			//Tcp_NetClose(*(int *)fd);
			//close(*(int *)fd);
			rec->next = tmp->next;
			free(tmp);
			break;
			//return;
		}
		tmp = tmp->next;
		rec = rec->next;
	}
	pthread_exit(NULL);
}

void Delete_TcpLink(TimeOut_S * head,int socket)
{
	TimeOut_S * tmp = sTimeOut->next;
	TimeOut_S * rec = sTimeOut;
	while(tmp != NULL)
	{
		if(tmp->soket == socket)
		{
			//DBG_ErrMsg("timeout\n");
			//Tcp_NetClose(*(int *)fd);
			//close(*(int *)fd);
			rec->next = tmp->next;
			free(tmp);
			break;
			//return;
		}
		tmp = tmp->next;
		rec = rec->next;
	}
	return;
}

int Tcp_chekid(int socket,TcpLogin_S * sTcpLogin)
{
	int i = 0;
	for(i = 0;i < sTcpLogin->num;i++)
	{
		if(socket == sTcpLogin->socketId[i])
		{
			return socket;
		}
	}
	//sTcpLogin->socketId[i] == socket;
	return 0;
}



int Tcp_NetRecvMsg(NetCliInfo_T *cli)
{
	//如果是新socket则注册
	//printf(">>>>>socketfd====%d\n",cli->recvSocket);
	if(!memcmp(cli->recvmsg,"close",strlen("close")) )
	{
		printf("<><><>%d close link\n",cli->recvSocket);
		Delete_TcpLink(sTimeOut,cli->recvSocket);
		return 0;
	}
	int ret = 0;
	int handleId = 0;
	int bSeur = 0;

	if((!strcmp(cli->fromIP,"127.0.0.1"))||(g_user_info.seurity_status == 0))
	{
		bSeur = 0;
		//printf(">>>>>>>>bSeur = 0\n");
	}
	else
	{
		bSeur = 1;
		//printf(">>>>>>>>bSeur = 1\n");
	}

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

	if(!memcmp(cli->recvmsg,"#\r",strlen(cli->recvmsg)) && Cheak_TcpStartLink(sTimeOut,cli->recvSocket) == 0&&(bSeur == 1))// && flagS == 0)
	{
		//printf("###  %d\n",cli->recvSocket);
		handleId = info->p3kHandle.handleId;
		info->p3kHandle.P3kMsgRecv(handleId,cli->recvmsg,cli->recvLen);

		DBG_ErrMsg("new link\n");
		//if(flagS == 0)
		if(bSeur == 1)
		{
			TimeOut_S * pnew = (TimeOut_S *)malloc(sizeof(TimeOut_S));
			pnew->flag = 0;
			pnew->soket = cli->recvSocket;
			pnew->next = NULL;
			HeadInsert(sTimeOut,pnew);
			pthread_t pth_time;
			pthread_create(&pth_time, NULL, LoginTimtOut, &cli->recvSocket);
		}
	}

	if(bSeur == 1)
	{
		char msg[64] = "";
		sprintf(msg,"#LOGIN admin,%s\r",g_user_info.password);

		//printf("login msg:%s\n",msg);

		if(!memcmp(cli->recvmsg,msg,strlen(cli->recvmsg)) && Cheak_TcpStartLink(sTimeOut,cli->recvSocket) == cli->recvSocket)
		{
			sTcpLogin.socketId[sTcpLogin.num] = cli->recvSocket;
			sTcpLogin.num += 1;
			SearchTcp(sTimeOut,cli->recvSocket);
		}
	}
	//数据下发
	if((Cheak_TcpLink(sTimeOut,cli->recvSocket) == cli->recvSocket || bSeur == 0))//&& Cheak_TcpLink(sTimeOut,cli->recvSocket) == cli->recvSocket//Tcp_chekid(info->sockfd,&sTcpLogin) > 0
	{
		handleId = info->p3kHandle.handleId;
		info->p3kHandle.P3kMsgRecv(handleId,cli->recvmsg,cli->recvLen);
	}
	else
	{
		DBG_WarnMsg(">>>>>>>>>>>>>>>>not login\n");
	}

	//logout
	if(bSeur == 1)
	{
		char msg[64] = "#LOGOUT\r";

		printf("recvmsg:%s;\n",cli->recvmsg);
		printf("msg:%s\n",msg);
		if(!memcmp(cli->recvmsg,msg,strlen(cli->recvmsg)) && Cheak_TcpStartLink(sTimeOut,cli->recvSocket) == cli->recvSocket)
		{
			printf("recvmsg:%s;msg:%s\n",cli->recvmsg,msg);
			sTcpLogin.socketId[sTcpLogin.num] = cli->recvSocket;
			sTcpLogin.num += 1;
			LogoutTcp(sTimeOut,cli->recvSocket);
		}
	}
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

void * TcpCmd_cb(void * fd)
{
    SocketWorkInfo_S*handle = (SocketWorkInfo_S*)fd;
    while(1)
    {
        //printf("11\n");
        if(handle->serverport == g_network_info.tcp_port)
        {
            usleep(50*1000);
        }
        else
        {
            SOCKET_DestroyTcpServer(Tcp_NetGetNetReristHandle());
            Tcp_NetInit(g_network_info.tcp_port);
        }
    }
    return;
}

int Tcp_NetInit(int port)
{
	Create_TimeHead(&sTimeOut);
	P3K_ApiInit();
	SocketWorkInfo_S * handle = Tcp_NetGetNetReristHandle();
	handle->readcb =Tcp_NetRecvMsg;
	handle->closecb = Tcp_NetClose;
	handle->serverport = port;
//	handle->getNetCabInfo = Tcp_NetGetNetCab;
	
    pthread_t TcpCmd;
	pthread_create(&TcpCmd, NULL, TcpCmd_cb, handle);
    pthread_detach(TcpCmd);
    SOCKET_CreateTcpServer(handle);
    

	return 0;
}

int Tcp_NNetInit()
{
//缁戝畾127.0.0.1:6001绔彛鐢ㄤ簬鍐呴儴閫氫俊
    SocketWorkInfo_S*handle1 = Tcp_NetGetNetReristHandle1();
    handle1->readcb =Tcp_NetRecvMsg;
    handle1->closecb = Tcp_NetClose;
    handle1->serverport = 6001;
    SOCKET_CreateTcp_NServer(handle1);
}

int Tcp_NetUnInit()
{
	Distory_TimeHead(sTimeOut);
	SOCKET_DestroyTcpServer(Tcp_NetGetNetReristHandle());
    SOCKET_DestroyTcpServer(Tcp_NetGetNetReristHandle1());
	Tcp_P3KHandleListUnInit();
	P3K_APIUnInit();
	return 0;
}



int main (int argc, char const *argv[])
{
	if((argc == 2)&&(!memcmp(argv[1],"-c",strlen("-c"))))
	{
		printf("P3k TCP only init cfg\n");
		Cfg_Init();
		return 0;
	}

    Cfg_Init_Network();
    int portNumber;

	if(argc == 3)
	{
		if(!memcmp(argv[2],"-l",strlen("-l")))
		{
			sTcpLogin.iFlag = 1;
		}
		Tcp_NetInit(atoi(argv[1]));
	}
	else
	{   
	    printf("P3k TCP port[%d]\n",g_network_info.tcp_port);
		Tcp_NetInit(g_network_info.tcp_port);
		sTcpLogin.iFlag = 0;
		flagS = 1;
	}
    Tcp_NNetInit();
	char ch =0;
	while(1)
	{
		ch =getchar();
		if(ch == 'q')
		{
			break;
		}
		usleep(2000000);
	}
	Tcp_NetUnInit();
	return 0;
}