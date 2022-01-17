#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "socketapi.h"
#include "p3kapi.h"
#include "handlelist.h"
#include "debugtool.h"
#include "P3Ktcp.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <net/if.h>       /* ifreq struct */
#include <net/if_arp.h>
SocketWorkInfo_S gs_netHandle = {0};
SocketWorkInfo_S gs_netHandle1 = {0};

int flagS =0;
typedef struct _TimeOut_S
{
	int  soket;
	int flag ;
    int InOrOut;
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


void Create_LinkList(ConnectionList_S *list)
{
    Connection_Info *p_head = malloc(sizeof(Connection_Info));
    strcpy(p_head->ip,"0");
    p_head->port = 0;
    p_head->next = NULL;
    p_head->pre = NULL;

    list->head = p_head;
    list->tail = p_head;
    list->size = 0;
}
void HeadInsert_LinkList(ConnectionList_S * list, Connection_Info *pnew)
{
    if (list == NULL || pnew == NULL || list->head == NULL)
		return;
	Connection_Info *head = list->head;
	if (head->next == NULL)
		list->tail = pnew;

	pnew->next = head->next;
	pnew->pre = head;
	if(head->next != NULL)
		head->next->pre = pnew;
	head->next = pnew;
	list->size++;
	return 0;  
}
void Print_LinkList(ConnectionList_S *list)
{}
void DeleteById_LinkList(ConnectionList_S *list, int socket)
{
    if (list == NULL || list->head == NULL || list->head->next == NULL)
		return;
	Connection_Info *pcur = list->head->next;
	Connection_Info *tmp = NULL;
    struct sockaddr_in sa;
    int len1 = sizeof(sa);
    getpeername(socket,(struct sockaddr *)&sa,&len1);
    int delport = ntohs(sa.sin_port);
    char delip[24] = "";
    strcpy(delip,inet_ntoa(sa.sin_addr));
	while (pcur != NULL)
	{
		if (pcur->port == delport && !memcmp(pcur->ip,delip,strlen(delip)))
		{

			pcur->pre->next = pcur->next;//pcur的前一个的next指向pcur的后一个
			if(pcur->next !=NULL)
				pcur->next->pre = pcur->pre;//pcur的下一个的pre指向pcur的前一个
	
			if (pcur == list->tail)//pcur当前节点等于尾节点
				list->tail = pcur->pre;//更新尾节点

			tmp = pcur->next;//tmp保存了pcur的下一个节点的地址
			free(pcur);//释放当前节点
			pcur = tmp;//pcur向后指
			list->size--;//更新size
		}
		else
		{
			pcur = pcur->next;
		}
	
	}
}

void Destroy_LinkList(ConnectionList_S *list)
{
    if (list == NULL)
		return;
	Connection_Info *head = list->head;
	Connection_Info *tmp=NULL;
	while (head != NULL)
	{
		tmp = head->next;
		free(head);
		head = tmp;
	}
	free(list);
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
int Tcp_NetSenddata(int userId,char*data,int len,int flag)
{

	//USR id找到对应socket
	 TcpCliP3KInfo_S* info = Tcp_NetGetP3KHandleMsgByID(userId);
	if(info == NULL)
	{
		return -1;
	}
    if(flag == 1){
        struct sockaddr_in sa;
        int len1 = sizeof(sa);
        if(!getpeername(info->sockfd,(struct sockaddr *)&sa,&len1))
        {
            //printf("Dest ip:%s port:%d\n",inet_ntoa(sa.sin_addr),ntohs(sa.sin_port));
            int port = ntohs(sa.sin_port); 
            if(!memcmp(inet_ntoa(sa.sin_addr),"127.0.0.1",strlen("127.0.0.1")))
            {
                if(port == 6002)
                 {}
                 else{
                    return;
                 }
            }
        }
    	SOCKET_TcpSendMessage(info->sockfd,data,len);
    }
    else
    {
        SOCKET_TcpSendMessage(info->sockfd,data,len);
    }
	return 0;
}
void Create_TimeHead(TimeOut_S ** head)
{
	TimeOut_S * tmp = (TimeOut_S *)malloc(sizeof(TimeOut_S));
	tmp->flag =0;
    tmp->InOrOut = 0;
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
			char * data = "con_list";
            SendToConList(data);
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

void UnInitTcpSocket(TimeOut_S * head)
{
    TimeOut_S * tmp = sTimeOut->next;
	TimeOut_S * rec = sTimeOut;
	while(tmp != NULL)
	{
        if(tmp->InOrOut == 0)
		{
			DeleteById_LinkList(g_connectionlist_info,tmp->soket);
            char * data = "con_list";
            SendToConList(data);
            close(tmp->soket);
			rec->next = tmp->next;
			free(tmp);
			//break;
			//return;
		}
		tmp = tmp->next;
		rec = rec->next;
	}
	return;
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
			DeleteById_LinkList(g_connectionlist_info,socket);
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
	if(strcasecmp(cli->recvmsg,"#SECUR?\r") == 0)
	{
		printf("#SECUR?\r");
		bSeur = 0;
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
            if(cli->recvPort == 6001)
            {
                printf("......in link\n");
                pnew->InOrOut = 1;
            }
            else{
                printf(".....out link\n");
                pnew->InOrOut = 0;
                Connection_Info * pnewconnection = (Connection_Info *)malloc(sizeof(Connection_Info));
                pnewconnection->port = cli->fromPort;
                strcpy(pnewconnection->ip,cli->fromIP);
                pnewconnection->next = NULL;
                pnewconnection->pre = NULL;
                HeadInsert_LinkList(g_connectionlist_info,pnewconnection);
            }
			pnew->soket = cli->recvSocket;
			pnew->next = NULL;
			HeadInsert(sTimeOut,pnew);
            
			pthread_t pth_time;

			pthread_attr_t	s_tThreadAttr;
			pthread_attr_init(&s_tThreadAttr);
			pthread_attr_setstacksize(&s_tThreadAttr, 512*1024);
			pthread_create(&pth_time, &s_tThreadAttr, LoginTimtOut, &cli->recvSocket);
		}
	}
    else if(!memcmp(cli->recvmsg,"#\r",strlen(cli->recvmsg)) && Cheak_TcpStartLink(sTimeOut,cli->recvSocket) == 0&&(bSeur == 0))
      {
            TimeOut_S * pnew = (TimeOut_S *)malloc(sizeof(TimeOut_S));
			pnew->flag = 1;
            if(cli->recvPort == 6001)
            {
                printf("......in link\n");
                pnew->InOrOut = 1;
            }
            else{
                printf(".....out link\n");
                pnew->InOrOut = 0;
                Connection_Info * pnewconnection = (Connection_Info *)malloc(sizeof(Connection_Info));
                pnewconnection->port = cli->fromPort;
                strcpy(pnewconnection->ip,cli->fromIP);
                pnewconnection->next = NULL;
                pnewconnection->pre = NULL;
                HeadInsert_LinkList(g_connectionlist_info,pnewconnection);
            }
			pnew->soket = cli->recvSocket;
			pnew->next = NULL;
			HeadInsert(sTimeOut,pnew);
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

	char ip_buf[32] = "";
	strcpy(ip_buf,g_InitIP);
	printf("[!!!!!!222222222IP Addr]g_InitIP:%s,ip_buf:%s\n",g_InitIP,ip_buf);

    while(1)
    {
		memset(ip_buf,0,32);
		GetIPInfo(1,ip_buf,NULL);
//		printf("[!!!!!!222222222IP Addr]g_InitIP:%s,ip_buf:%s\n",g_InitIP,ip_buf);
		if((strcmp(ip_buf,g_InitIP) == 0)&&(handle->serverport == g_network_info.tcp_port))
        {
            usleep(1000*1000);
        }
        else
        {
			if(strcmp(ip_buf,g_InitIP) != 0)
			{
				printf("!!!notifty to soip2\n");
				NotifyIPtoSOIP2(ip_buf);
			}
			printf("[!!!!!!IP Port]handle->serverport:%d; g_network_info.tcp_port:%d\n",handle->serverport,g_network_info.tcp_port);
			printf("[!!!!!!IP Addr]g_InitIP:%s,ip_buf:%s\n",g_InitIP,ip_buf);
			strcpy(g_InitIP,ip_buf);
            UnInitTcpSocket(sTimeOut);
            SOCKET_DestroyTcpServer(Tcp_NetGetNetReristHandle());

			usleep(2000*1000);
            Tcp_NetInit(g_network_info.tcp_port);
            return;
        }
    }
    return;
}

int Tcp_NetInit(int port)
{
	SocketWorkInfo_S * handle = Tcp_NetGetNetReristHandle();
	handle->readcb =Tcp_NetRecvMsg;
	handle->closecb = Tcp_NetClose;
	handle->serverport = port;
//	handle->getNetCabInfo = Tcp_NetGetNetCab;
	SOCKET_CreateTcpServer(handle);
    pthread_t TcpCmd;

	pthread_attr_t	s_tThreadAttr;
	pthread_attr_init(&s_tThreadAttr);
	pthread_attr_setstacksize(&s_tThreadAttr, 512*1024);
	pthread_create(&TcpCmd, &s_tThreadAttr, TcpCmd_cb, handle);
    pthread_detach(TcpCmd);
    
	return 0;
}

int Tcp_NNetInit()
{
//缁戝畾127.0.0.1:6001绔彛鐢ㄤ簬鍐呴儴閫氫俊
	Create_TimeHead(&sTimeOut);
    HandleManageInitHead(&gs_cliHandleMng.listHandleHead);
	P3K_ApiInit();

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

 int  PhraserParam(char *param,int len,	char str[][4] )
{
	int tmpLen = 0 ;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;

	if(param == NULL ||len <=0)
	{
		return -1;
	}
	while(tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata,'.');
		if(tmpdata != NULL)
		{
			tmpLen = tmpdata-tmpdata1;
			memcpy(str[i],tmpdata1,tmpLen);
			i++;
			if(len > tmpdata-param+1)
			{
				tmpdata1 = tmpdata+1;
				tmpdata = tmpdata +1;
		       }
			else
			{
				break;
			}
		}
	}
	memcpy(str[i],tmpdata1,strlen(tmpdata1));
	i++;
	return i;
}


int check_isIp(char* IP)
{
    if(strlen(IP) < 7 || strlen(IP) > 15)
        return -1;
    char str[4][4] ={0};
    int count = 0;
    count = PhraserParam(IP,strlen(IP),str);
    if(count != 4)
        return -1;
    int ip1 = atoi(str[0]);
    int ip2 = atoi(str[1]);
    int ip3 = atoi(str[2]);
    int ip4 = atoi(str[3]);
    if(0 < ip1 && ip1 <= 255 && 0 <= ip2 && ip2 <= 255 && 0 <= ip3 && ip3 <= 255 && 0 <= ip4 && ip4 <= 255)
    {
        char tmpip[16] = "";
        sprintf(tmpip,"%d.%d.%d.%d",ip1,ip2,ip3,ip4);
        printf("listen_ip[%s]\n",tmpip);
        memcpy(g_InitIP,tmpip,strlen(tmpip));
    }
    else{
        return -1;
    }
    return 0;
}

int main (int argc, char const *argv[])
{
	if((argc == 2)&&(!memcmp(argv[1],"-c",strlen("-c"))))
	{
		g_bCfg = 1;
		printf("P3k TCP only init cfg g_bCfg = %d\n",g_bCfg);
		Cfg_Init();
		return 0;
	}
    else if((argc == 2))
    {
        int ret = check_isIp(argv[1]);
        if(ret!=0){
            printf("Param Error!\n");
            return 0;
        }
    }

    Tcp_NNetInit();

    Cfg_Init_Network();
    int portNumber;

	if(strlen(g_InitIP)<7)
	{
		GetIPInfo(1,g_InitIP,NULL);
	}

	while(1)
	{
		if(strcmp(g_InitIP,"0.0.0.0") == 0)
		{
			usleep(1000*1000);
			GetIPInfo(1,g_InitIP,NULL);
		}
		else
			break;
	}

	UpdateLocalIP(g_InitIP);

	if(argc == 3)
	{
		if(!memcmp(argv[2],"-l",strlen("-l")))
		{
			sTcpLogin.iFlag = 1;
		}
		//Tcp_NetInit(atoi(argv[1]));
	}
	else
	{   
	    printf("P3k TCP port[%d]\n",g_network_info.tcp_port);
		Tcp_NetInit(g_network_info.tcp_port);
		sTcpLogin.iFlag = 0;
		flagS = 1;
	}
    
    g_connectionlist_info = malloc(sizeof(ConnectionList_S));
    Create_LinkList(g_connectionlist_info);
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
    Destroy_LinkList(g_connectionlist_info);
	Tcp_NetUnInit();
	return 0;
}