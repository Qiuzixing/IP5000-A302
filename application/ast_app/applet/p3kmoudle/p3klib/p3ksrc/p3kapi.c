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
#include "ldfw.h"
#include "cfgparser.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

//#include "funcexcute.h"
#define MAX_HANDLE 10
//static P3K_QeqCmdProcess gs_callHandle = NULL;

typedef struct _P3KLDFWStatus_S
{
	int num;  //注册获得handleId
	int handleId[10];
}P3KLDFWStatus_S;

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
static P3KLDFWStatus_S  gs_LDFWStatus = {0};
/*
static int  Create_Head(P3KLDFWStatus_S ** head)
{
	P3KLDFWStatus_S * tmp = (P3KLDFWStatus_S *)malloc(sizeof(P3KLDFWStatus_S));
	tmp->handleId = 0;
	tmp->status = 0;
	tmp->next = NULL;
	*head = tmp;
}

static int P3K_SetLDFWStatus(P3KLDFWStatus_S *gs_LDFWStatus, int handleId)
{
	P3KLDFWStatus_S * tmp = gs_LDFWStatus->next;
	while(tmp->next !=NULL)
	{
		if(tmp->handleId == handleId)
		{
			tmp->status = 1;
			return 0 ;
		}
		tmp = tmp->next;
	}

	return 0;
}*/

char* MyatoA( char *pDest )    
{    
     //assert( nullptr != pDest ); 
     char *i = pDest;
     for( ; *i != '\0'; ++i )    
     {    
         if( *i < 'a' || *i > 'z' )    
         {    
             continue;    
         }    
         *i -= 'a' - 'A';    
     }    
     return pDest;    
 }

char* MyAtoa( char *pDest )    
{    
     //assert( nullptr != pDest ); 
     char *i = pDest;
     for( ; *i != '\0'; ++i )    
     {    
         if( *i < 'A' || *i > 'Z' )    
         {    
             continue;    
         }    
         *i += 'a' - 'A';    
     }    
     return pDest;    
 }


static const unsigned char crcTableH[] = {

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40
};

static const unsigned char crcTableL[] = {

    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,
    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,
    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,
    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,
    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,
    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,
    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,
    0x41, 0x81, 0x80, 0x40

};

static unsigned short  CRC16( unsigned char * pucFrame, int usLen )
{
    unsigned char tmpcrcH = 0xFF;
    unsigned char tmpcrcL = 0xFF;
    int iIndex;
    while( usLen-- )
    {
        iIndex = tmpcrcL ^ *( pucFrame++ );
        tmpcrcL = ( unsigned char )( tmpcrcH ^ crcTableH[iIndex] );
        tmpcrcH = crcTableL[iIndex];
    }
    return ( unsigned short )( tmpcrcH << 8 | tmpcrcL );
}

static int P3K_GetLDFWStatus(P3KLDFWStatus_S * gs_LDFWStatus,int handleId)
{
	//gs_LDFWStatus.handleId = handleId;
	int i = 0;
	for(i = 0;i < gs_LDFWStatus->num;i++)
	{
		if(handleId == gs_LDFWStatus->handleId[i])
		{
			return gs_LDFWStatus->handleId[i];
		}
	}
	return 0;
}
static int P3K_SetLDFWStatus(P3KLDFWStatus_S * gs_LDFWStatus,int handleId)
{
	//gs_LDFWStatus.handleId = handleId;
	int i = 0;
	int j =0;
	for(i = 0;i < gs_LDFWStatus->num;i++)
	{
		if(handleId == gs_LDFWStatus->handleId[i])
		{
			gs_LDFWStatus->handleId[i] = 0;;
		}
		for(j = i +1;j < gs_LDFWStatus->num;j ++)
		{
			gs_LDFWStatus->handleId[j - 1] =gs_LDFWStatus->handleId[j];
			gs_LDFWStatus->handleId[j] = 0;
		}
	}
	gs_LDFWStatus->num -= 1;
	return 0;
}
static int LDFWflag = 0;

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



static int  P3K_PhraserIDParam(char *param,int len,	char str[][MAX_PARAM_LEN] )
{
	int tmpLen = 0 ;
	//int s32Ret = 0;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;


	if(param == NULL ||len <=0)
	{
		return -1;
	}
	while(tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata,',');
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

//启动udp监听50000端口
void * P3K_UdpServer()
{
    int old_Udp_port;
    old_Udp_port = g_network_info.udp_port;
RESET_Udp:
    
	g_Udp_Socket= socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in myaddr;
	myaddr.sin_family=AF_INET;
	myaddr.sin_port = htons(g_network_info.udp_port);
	myaddr.sin_addr.s_addr = 0;//通配地址  将所有的地址绑定
	int opt = 1;  
    // sockfd为需要端口复用的套接字  
    setsockopt(g_Udp_Socket,SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	if(bind(g_Udp_Socket,(struct sockaddr*)&myaddr,sizeof(myaddr))<0)
		DBG_ErrMsg("Udp bind err");
	char recvBuf[1500] = "";
    char aSendp3k[1500] = "";
    char aflag[12] = "";
    P3K_SimpleCmdInfo_S Sendp3k;
    P3K_SimpleCmdInfo_S respCmdInfo;
    char userDefine[MAX_USR_STR_LEN+1] = {0};
	struct sockaddr_in client;
         socklen_t len = sizeof(client);  
	while(1)
	{
	    if(old_Udp_port != g_network_info.udp_port)
        {
            old_Udp_port = g_network_info.udp_port;
            printf("udp_port_change\n");
            close(g_Udp_Socket);
            goto RESET_Udp;
        }   
		memset(recvBuf,0,sizeof(recvBuf));
		int s = recvfrom(g_Udp_Socket, recvBuf, sizeof(recvBuf)-1,0,(struct sockaddr*)&client,&len);
		if(s > 0)
		{
			printf("Udp Recv Msg:%s\n",recvBuf);
            MyatoA(recvBuf);
            if(!memcmp(recvBuf,"#NET-IP?",strlen("#NET-IP?")) || !memcmp(recvBuf,"#ETH-PORT?",strlen("#ETH-PORT?")) || !memcmp(recvBuf,"#NET-CONFIG?",strlen("#NET-CONFIG?")))
            {
                memset(&respCmdInfo,0,sizeof(P3K_SimpleCmdInfo_S));
		        memset(userDefine,0,MAX_USR_STR_LEN);
                if(strlen(recvBuf) == strlen("#NET-IP?"))
                {
                    sprintf(aSendp3k,"IP,%s,%d",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    memcpy(Sendp3k.command,"UDPNET-CONFIG?",strlen("UDPNET-CONFIG?"));
                    memcpy(Sendp3k.param,aSendp3k,strlen(aSendp3k));
                    printf("[%s:%s]\n",Sendp3k.command,Sendp3k.param);
                    P3K_SilmpleReqCmdProcess(&Sendp3k,&respCmdInfo,userDefine);
                }
                else if(strlen(recvBuf) == strlen("#NET-CONFIG?"))
                {
                    sprintf(aSendp3k,"CONFIG,%s,%d",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    memcpy(Sendp3k.command,"UDPNET-CONFIG?",strlen("UDPNET-CONFIG?"));
                    memcpy(Sendp3k.param,aSendp3k,strlen(aSendp3k));
                    printf("[%s:%s]\n",Sendp3k.command,Sendp3k.param);
                    P3K_SilmpleReqCmdProcess(&Sendp3k,&respCmdInfo,userDefine);
                }
                else
                {
                    memcpy(aflag,recvBuf+11,3);
                    printf("[.%s.]{%s}\n",recvBuf,aflag);
                    sprintf(aSendp3k,"%s,%s,%d",aflag,inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    memcpy(Sendp3k.command,"UDPNET-CONFIG?",strlen("UDPNET-CONFIG?"));
                    memcpy(Sendp3k.param,aSendp3k,strlen(aSendp3k));
                    printf("[%s:%s]\n",Sendp3k.command,Sendp3k.param);
                    P3K_SilmpleReqCmdProcess(&Sendp3k,&respCmdInfo,userDefine);
                }
            }
		}
        else
        {
            usleep(20*100);
        }
	}
    return;
}

void Sendtoclient(char * msg, int len)
{
    HandleList_S * tmphandle = &(gs_handleMng.listHandleHead);
	P3KReqistMsg_S *upregistMsg = NULL;
    do{
    	upregistMsg = (P3KReqistMsg_S *)HandleManageGetNextHandle(tmphandle);
    	if(upregistMsg)
    	{
    		upregistMsg->sendMsg(upregistMsg->handleId,msg,len);
    	}
    	upregistMsg = NULL;
    	tmphandle = tmphandle->next;
	}while(tmphandle != NULL && tmphandle->next != NULL);
}

void * P3K_UdpInsideServer()
{
    int old_Udp_port;
	g_Udp_Inside_Socket= socket(AF_INET,SOCK_DGRAM,0);
	struct sockaddr_in myaddr;
	myaddr.sin_family=AF_INET;
	myaddr.sin_port = htons(60001);
	myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");//通配地址  将所有的地址绑定
	int opt = 1;  
    // sockfd为需要端口复用的套接字  
    setsockopt(g_Udp_Inside_Socket,SOL_SOCKET,SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	if(bind(g_Udp_Inside_Socket,(struct sockaddr*)&myaddr,sizeof(myaddr))<0)
		DBG_ErrMsg("Udp bind err");

    int s32Ret = 0;
    int tmplen = 0;
	char recvBuf[1500] = "";
    char aSendp3k[1500] = "";
    P3K_SimpleCmdInfo_S Sendp3k;
    P3K_SimpleCmdInfo_S respCmdInfo;
    P3K_SimpleCmdInfo_S cmd;
    char userDefine[MAX_USR_STR_LEN+1] = {0};
    char dstdata[512] = {0};
    
	struct sockaddr_in client;
    socklen_t len = sizeof(client);  
	while(1)
	{  
		memset(recvBuf,0,sizeof(recvBuf));
		int s = recvfrom(g_Udp_Inside_Socket, recvBuf, sizeof(recvBuf)-1,0,(struct sockaddr*)&client,&len);
		if(s > 0 && !memcmp(recvBuf,"#P3K-NOTIFY",strlen("#P3K-NOTIFY")))
		{
            memcpy(cmd.command,"P3K-NOTIFY",strlen("P3K-NOTIFY"));
            strcpy(cmd.param,recvBuf+12);
            s32Ret = P3K_SilmpleReqCmdProcess(&cmd,&respCmdInfo,userDefine);
            strcpy(respCmdInfo.command,userDefine);
			if(s32Ret != 0)
			{
				continue;
			}
            memset(dstdata,0,sizeof(dstdata));
			//组包
			tmplen = P3K_SimpleRespCmdBurstification(&respCmdInfo, dstdata);
			//发送数据
            Sendtoclient(dstdata,tmplen);
		}
        else
        {
            usleep(20*100);
        }
	}
    return;
}



void ParseMsgSetOrGet(P3K_SimpleCmdInfo_S *cmd,char *dstdata)
{
    char * tmp = &cmd->command[strlen(cmd->command)-1];
    memcpy(dstdata,tmp,strlen(tmp));
}

static void * P3K_DataExcuteProc(void*arg)
{
	P3KMsgQueueMember_S pmsg;
	P3KReqistMsg_S *registMsg = NULL;
	P3K_SimpleCmdInfo_S respCmdInfo;
	char dstdata[512] = {0};
	int s32Ret = 0;
	int tmplen = 0;
	char aOtherCh[128] = {0};
	char userDefine[MAX_USR_STR_LEN+1] = {0};
    char  aSetOrGet[4] = "";
    char  aEndFlag[4] = "?";
	//函数处理执行线程
	prctl(PR_SET_NAME, (unsigned long)"P3K_DataExcuteProc", 0,0,0);
      Cfg_Init();
 	int i_Udps32Ret = 0;
	pthread_t tUdpserver;
	i_Udps32Ret = pthread_create(&tUdpserver, NULL, P3K_UdpServer, NULL);
	if(i_Udps32Ret)
	{
		DBG_ErrMsg("Udp Server Start Error\n");
	}
	else{
        printf("Udp Server Start\n");}
    pthread_detach(tUdpserver);

    pthread_t tUdp_inside_server;
	i_Udps32Ret = pthread_create(&tUdp_inside_server, NULL, P3K_UdpInsideServer, NULL);
	if(i_Udps32Ret)
	{
		DBG_ErrMsg("Udp inside Server Start Error\n");
	}
	else{
        printf("Udp inside Server Start\n");}
    pthread_detach(tUdp_inside_server);
    
	while(1)//P3K_GetApiInitFlag())
	{
		memset(&pmsg,0,sizeof(P3KMsgQueueMember_S));
		memset(aOtherCh,0,sizeof(aOtherCh));
		s32Ret = P3K_MSgQueueGetMsg(&pmsg);
		if(s32Ret != 0)
		{
            	usleep(200*1000);
				continue;
		}
		memset(&respCmdInfo,0,sizeof(P3K_SimpleCmdInfo_S));
		//解析处理数据
		//特殊命令分开执行可放在这里?

		memset(userDefine,0,MAX_USR_STR_LEN);
			s32Ret = P3K_SilmpleReqCmdProcess(&pmsg.cmdinfo,&respCmdInfo,userDefine);
			if(s32Ret != 0)
			{
				continue;
			}
            memset(aSetOrGet,0,sizeof(aSetOrGet));
            ParseMsgSetOrGet(&respCmdInfo,aSetOrGet);
			memset(dstdata,0,sizeof(dstdata));
			//组包
			tmplen = P3K_SimpleRespCmdBurstification(&respCmdInfo, dstdata);
			//发送数据
			if(!memcmp(aSetOrGet,aEndFlag,strlen(aEndFlag)) || (!strcasecmp(respCmdInfo.command,"LOGIN")))
            {
                //printf("Get\n");
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
            else{
                 Sendtoclient(dstdata,tmplen);
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

long fun(char *s)
{
	int i,t;             //t记录临时加的数
	long sum =0;
	for(i=0;s[i];i++)
		{
		if(s[i]>='0'&&s[i]<='9'){
			t=s[i]-'0';     }  //当字符是0~9时保持原数不变
		if(s[i]>='a'&&s[i]<='z'){
				t=s[i]-'a'+10;}
		if(s[i]>='A'&&s[i]<='Z'){
				t=s[i]-'A'+10;}
		sum=sum*16+t;
	}
	return sum;
}


static int P3K_RecvMessage(int handleId,char*data,int len )
{
	if(data == NULL)
	{
		return -1;
	}
	P3K_SimpleCmdInfo_S cmd[16];
	P3K_SimpleCmdInfo_S  cmdresp;
	P3KMsgQueueMember_S queueMmber;
	int cmdCount = 0;
	int i = 0;
	int flag = 0;
	int iFlieSize = 0;
	memset(cmd,0,sizeof(P3K_SimpleCmdInfo_S)*16);
    
	//flag = P3K_GetLDFWStatus(&gs_LDFWStatus, handleId);
	if(LDFWflag == 1)
	{
		char  num[10] = {0};
		char numlength[5] = {0};
		int iLength = 0;
		P3KReqistMsg_S *registMsg = NULL;
		char aMsg[56] = {0};
		sprintf(numlength,"%02x%02x",data[2],data[3]);
		iLength = (fun(numlength)) - 2;
		printf(">>>>>packetID =%02x%02x length =%d\n",data[0],data[1],iLength);
		if( 0 == memcmp(data,"end",strlen("end")))
		{
			//P3K_SetLDFWStatus(&gs_LDFWStatus, flag);
			DBG_InfoMsg("file end\n");
			LDFWflag = 0;
		}
		else
		{
			char * str =data;
			unsigned short CRCnum = CRC16((unsigned char *)(data+4),iLength);
			char hexcrc[5] = {0};
			//memcpy(hexcrc,data+iLength+2,2);
			sprintf(hexcrc,"%02x%02x",data[iLength+4],data[iLength+5]);
			char sum[5] = {0};
			char sum1[5] = {0};
			sprintf(sum1,"%04x",CRCnum);
			//printf("crc==%s,newcrc= %s....%d\n",hexcrc,sum1,CRCnum);
			if(0 == memcmp(sum1,hexcrc,strlen(hexcrc)))
			{

				P3K_LDWFRecvFileData(data,&num,iLength);
				sprintf(aMsg,"~%s ok\r\n",num);
				//printf("<<<%s\n",aMsg);
				registMsg = P3K_GetReqistMsgByID(handleId);
				if(registMsg)
				{
					registMsg->sendMsg(handleId,aMsg,strlen(aMsg));
				}
			}
			else{
				char * ret = "error";
				registMsg = P3K_GetReqistMsgByID(handleId);
				if(registMsg)
				{
					registMsg->sendMsg(handleId,ret,strlen(ret));
				}
				DBG_ErrMsg("UPGRADE err\n");
				LDFWflag = 0;
			}
		}
	}
	else if(flag == 0)
	{
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
				//gs_LDFWStatus.handleId[gs_LDFWStatus.num] = handleId;
				//gs_LDFWStatus.num += 1;
				iFlieSize = atoi(cmd[i].param);

				char str[] = "~01@";
				char aMsg[256] = {0};
				DBG_InfoMsg("SPECIAL CMD =%s\n",cmd[i].command);
				P3KReqistMsg_S *registMsg = NULL;
				int ret = P3K_LDWFCmdProcess(handleId,&cmd[i],&cmdresp);
				if(ret != 0)
				{
					DBG_ErrMsg("UPGRADE err\n");
					continue;
				}
				else
				{
					LDFWflag = 1;
					sprintf(aMsg,"%s%s %s\r\n",str,cmdresp.command,cmdresp.param);
					printf(">>>>%s\n",aMsg);
					registMsg = P3K_GetReqistMsgByID(handleId);
					if(registMsg)
					{
						registMsg->sendMsg(handleId,aMsg,strlen(aMsg));
					}
				}
				//P3K_LDWFSetUpgradeHandle(handleId);
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
			printf("<<<<<<recv%s\n",data);
			memset(&queueMmber,0,sizeof(queueMmber));
			queueMmber.handleId =handleId;
			memcpy(&queueMmber.cmdinfo,&cmd[i],sizeof(P3K_SimpleCmdInfo_S));
			P3K_MSgQueueAddMsg(&queueMmber);
		}
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

