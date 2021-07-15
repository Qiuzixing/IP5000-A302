#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <net/if.h>       /* ifreq struct */
#include <net/if_arp.h>
#include <pthread.h>
#include "list.h"
#include "common.h"
#include <sys/prctl.h>
#include <netdb.h>

#include "socketapi.h"
#include "debugtool.h"
#define DF_NET_MSGBUFLEN  20*1024

 
typedef enum _SOCKETTYPE_E{
	SOCKETTYPE_ENUM_NULL   = 0x00,
	SOCKETTYPE_ENUM_MAIN	= 0x01,
	SOCKETTYPE_ENUM_SUB		= 0x02,
}SOCKETTYPE_E;

typedef struct _SocketList_S{
	int 					sockfd;
	SOCKETTYPE_E 			socketType; 
	char					ip[16];
	int						port;
	struct					listnode _list;
}SocketList_S;

typedef struct _ExtParam_S{
	int mainSockfd;
    char *mulicastIp;
  NetCabGetInfocbFun		getNetCabInfo;
}ExtParam_S;

typedef struct _NetServerParam_S{
	int mainSockfd;
	NetTcpSerReadCbFun readcb;
	NetTcpSerCloseCbFun closecb;
}NetServerParam_S;

 static int bTsockFlag;
 static int uTsockFlag;
static  int GetSockInfo(int  socked, char *ipaddr,int *port)
{
	struct sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));

	socklen_t nSockAddrLen = sizeof(sockAddr);
	int iResult = getsockname(socked, (struct sockaddr *)&sockAddr, &nSockAddrLen);
	if (0 == iResult){
		*port = ntohs(sockAddr.sin_port);
		sprintf(ipaddr,"%s",inet_ntoa(sockAddr.sin_addr));
		return 1;
	}
	return 0;
} 


static int CheckSelectExcept(fd_set *exceptfds,SocketList_S *sockInfo)
{
	if(0 == FD_ISSET(sockInfo->sockfd, exceptfds)){
		return -1;
	}
	list_remove(&(sockInfo->_list));
	close(sockInfo->sockfd);
	free(sockInfo);
	return 0;
}



static int CheckSelectRead(fd_set *readfds,SocketList_S *head,SocketList_S *sockInfo,NetServerParam_S *param)
{
	static NetCliInfo_T cliInfo;
	memset(&cliInfo, 0, sizeof(NetCliInfo_T));
	NetCliInfo_T *cli = &cliInfo;
	int cliSockfd = -1;
	struct sockaddr_in client_addr;
	SocketList_S *newnode = NULL;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int ret = -1;

	fd_set readfd,errorfd;
	int maxfd = 0;
	int iret = 0;
	struct timeval timeout = {0};
	int timeOutCount;

	if(0 == FD_ISSET(sockInfo->sockfd, readfds)){
		return -1;
	}

	if(SOCKETTYPE_ENUM_MAIN == sockInfo->socketType){//主套接字
		cliSockfd = accept(sockInfo->sockfd, (struct sockaddr*)&client_addr, &addr_len);
		if(cliSockfd <= 0){
			DBG_ErrMsg("accept err\n");
			return -1;
		}
		
		newnode =  (SocketList_S *)malloc(sizeof(SocketList_S));
		if(NULL == newnode){
			return -1;
		}
		newnode->sockfd = cliSockfd;
		newnode->socketType = SOCKETTYPE_ENUM_SUB;
		newnode->port = ntohs(client_addr.sin_port);
		inet_ntop(AF_INET, (void *)(&client_addr.sin_addr.s_addr), newnode->ip, IP_ADRESS_LEN);
		//snprintf(newnode->ip,16,"%s",inet_ntoa(client_addr.sin_addr));
		list_add_tail(&(head->_list),&(newnode->_list));	
	}else if(SOCKETTYPE_ENUM_SUB == sockInfo->socketType){//从套接字
		//cli = (NetCliInfo_T *)malloc_checkout(sizeof(NetCliInfo_T));
		//if(NULL == cli){
		//	return KEY_FALSE;
		//}
		memset(cli,0,sizeof(NetCliInfo_T));
		//DF_DEBUG("socket read");
		timeOutCount = 0;
		while(    bTsockFlag==1){
			FD_ZERO(&readfd);
			FD_ZERO(&errorfd);
			FD_SET(sockInfo->sockfd,&readfd); //添加描述符
			FD_SET(sockInfo->sockfd,&errorfd); 
		 	maxfd = sockInfo->sockfd;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;//select函数会不断修改timeout的值，所以每次循环都应该重新赋值[windows不受此影响]	
			ret  = select (maxfd + 1, &readfd, NULL, &errorfd, &timeout);
			if (ret > 0){
				if(0 == FD_ISSET(sockInfo->sockfd, &readfd)){
					goto READ_ERROR;
				}
				ret = recv(sockInfo->sockfd,cli->recvmsg+cli->recvLen,sizeof(cli->recvmsg)-cli->recvLen,0);//获取网络消息
				if(ret >  0){
					//收到正确消息处理
					memcpy(cli->fromIP,sockInfo->ip,IP_ADRESS_LEN);
					cli->fromPort= sockInfo->port;
					cli->recvLen += ret;
					cli->recvSocket = sockInfo->sockfd;
					GetSockInfo(cli->recvSocket,cli->recvIp,&(cli->recvPort));
					{
						struct timeval timeout = {0,300 * 1000};    
						setsockopt(cli->recvSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(struct timeval));
						
						param->readcb(cli);
						break;
					}
					//if(ret != DF_NET_MSGBUFLEN){
					//	break;
					//}
					//DF_DEBUG("recv no end");
				}else{
					//客户端主动关闭套接字或链接错误关闭套接字
					//DF_DEBUG("clinet close socked,ret = %d",ret);
					cli->recvSocket = sockInfo->sockfd;
					char * closetcp = "close";
					memcpy(cli->recvmsg,closetcp,strlen(closetcp));
					param->readcb(cli);
					DBG_ErrMsg("clinet close socked\n");
					//sTimeOut
				goto READ_ERROR;
				}
			}else if (ret == 0){
				timeOutCount++;
				if(timeOutCount > 5){
					goto READ_ERROR;
				}
				//DF_DEBUG("select time out");
				continue;
			}else {
				//DF_DEBUG("select error");
				goto READ_ERROR;
			}
			usleep(10*1000);//程序空跑 CPU不至于100%		
		}
	}
	return 0;
READ_ERROR:
	if(NULL != cli->usrdef){
		free(cli->usrdef);
	}
	
//	free(cli);
#if 1
	list_remove(&(sockInfo->_list));
	close(sockInfo->sockfd);
	free(sockInfo);
#endif
	return 0;
}

 static int TcpServerSocketInit(int  serverport)
{
	unsigned short port = serverport;
	int connfd = 0,bindfd = 0;
	int listenfd = 0;
	int sockfd = 0;
	int opt = 1;
	
	struct sockaddr_in server_addr;	
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
	{
		DBG_ErrMsg("tcp server init ERR\n ");
		return -1;
	}
	//设置端口复用
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	bindfd = bind(sockfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr));
	if(bindfd != 0)
	{
		
		close(sockfd);
		return 0;
	}
	
	listenfd = listen(sockfd,LISTEN_MAX_FD);
	if(listenfd != 0)
	{	
		close(sockfd);
		return 0;
	}
	DBG_InfoMsg(" tcp server init  with socketfd %d \n",sockfd);
	return sockfd;
}
 void *TcpServerThead(void *arg)
{
	SocketList_S *head = NULL;
	SocketList_S *newnode = NULL;
	NetServerParam_S *param = NULL;
	struct listnode *node = NULL;
	struct listnode *nextnode = NULL;
	SocketList_S *sockInfo = NULL;

	fd_set readfd,errorfd;
	int maxfd = 0;
	int ret = 0;
	struct timeval timeout = {0};
	DBG_InfoMsg("TcpServerThead START\n");
	prctl(PR_SET_NAME, (unsigned long)"NetServerThead", 0,0,0);
	if(NULL == arg){
		return NULL;
	}
	param = (NetServerParam_S *)arg;
	if(0 >= param->mainSockfd){
		return NULL;
	}
	head = (SocketList_S *)malloc(sizeof(SocketList_S));
	if(NULL == head){
		close(param->mainSockfd);
		return NULL;
	}
	memset(head,0,sizeof(SocketList_S));
	head->socketType = SOCKETTYPE_ENUM_NULL;
	list_init(&(head->_list));
	newnode =  (SocketList_S *)malloc(sizeof(SocketList_S));
	if(NULL == newnode){
		free(head);
		close(param->mainSockfd);
		return NULL;
	}
	
	newnode->sockfd = param->mainSockfd;
	newnode->socketType = SOCKETTYPE_ENUM_MAIN;
	list_add_tail(&(head->_list),&(newnode->_list));
	while(    bTsockFlag == 1){
		FD_ZERO(&readfd);
		FD_ZERO(&errorfd);
		maxfd = -1;
		list_for_each(node,&(head->_list)){
			sockInfo = (SocketList_S *)node_to_item(node,SocketList_S,_list);
			FD_SET(sockInfo->sockfd,&readfd); //添加描述符
			FD_SET(sockInfo->sockfd,&errorfd); 
			if(maxfd < sockInfo->sockfd) maxfd = sockInfo->sockfd;
		}
		timeout.tv_sec = 1;
		timeout.tv_usec = 5000000;//select函数会不断修改timeout的值，所以每次循环都应该重新赋值[windows不受此影响]	
		ret  = select (maxfd + 1, &readfd, NULL, &errorfd, &timeout);
		if (ret > 0){
			//list_for_each(node, &(head->_list)){
			nextnode = NULL;
	 		for (node = head->_list.next; node != &(head->_list); ){
				sockInfo = (SocketList_S *)node_to_item(node,SocketList_S,_list);
				if (0 >= sockInfo->sockfd) {
					
					continue;
				}
				nextnode = node->next;
				if(!CheckSelectExcept(&errorfd,sockInfo));
				else if(!CheckSelectRead(&readfd,head,sockInfo,param));
				node = nextnode;
			}
		}else if (ret == 0){
			//DF_DEBUG("Service Select Timeout");
			continue;
		}else {
		
			list_for_each(node, &(head->_list)){
				sockInfo = (SocketList_S *)node_to_item(node, SocketList_S, _list);
				struct stat tStat;
				if (0 != fstat(sockInfo->sockfd, &tStat)) {
					//DF_DEBUG("fstat %d error:%s\n", sockInfo->sockfd, strerror(errno));					
					if(param != NULL && param->closecb != NULL)
					{
						param->closecb(sockInfo->sockfd);
					}
					list_remove(&(sockInfo->_list));
					close(sockInfo->sockfd);
					free(sockInfo);
					break;
				}
			}
		}
		usleep(10*1000);//程序空跑 CPU不至于100%
	}	
}
 

int SOCKET_CreateTcpServer(SocketWorkInfo_S*serverhandle)
{
	pthread_t pthid;
	NetServerParam_S *param;
       bTsockFlag= 1;

	param = (NetServerParam_S *)malloc(sizeof(NetServerParam_S));
	param->mainSockfd = TcpServerSocketInit(serverhandle->serverport);
	param->readcb = serverhandle->readcb;
	param->closecb = serverhandle->closecb;
	if(param->mainSockfd <= 0){
		free(param);
		return -1;
	}
	printf("------------------\n");
	serverhandle->sockfd = param->mainSockfd;
	pthread_create(&pthid, NULL, TcpServerThead, (void *)param);
        pthread_detach(pthid);
	return 0;

}
int  SOCKET_DestroyTcpServer(SocketWorkInfo_S*serverhandle)
{
	bTsockFlag= 0;

	if(serverhandle->sockfd <= 0){
		return 0;
	}
	close(serverhandle->sockfd);
	serverhandle->sockfd = -1;
	return 0;

}

static int  P3K_SelectHandleID(int iId,char * aIds)
{
	int num = 0;
	for(num =0;num < 10;num ++)
	{
		if(aIds[num] == iId)
		{
			return iId;	
		}
	}
	//aIds[strlen(aIds)] = iId;
	return 0;
}

static int  P3K_PhraserIDParam(char *param,int len,	char str[][MAX_PARAM_LEN] )
{
	int tmpLen = 0 ;
	int s32Ret = 0;
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



int SOCKET_TcpSendMessage(int sockfd,char *msg,int len)
{
	int ret = -1;
	int iSend = 0;
	int Total = 0;    
	int lenSend = 0;
	int cont = 0;
	struct timeval tv;    
	fd_set wset,errorfd; 
	
	while(1)
	{
		FD_ZERO(&wset);    
		FD_ZERO(&errorfd);
		FD_SET(sockfd, &wset);
		FD_SET(sockfd, &errorfd);
		tv.tv_sec = 3;//3;	 
		tv.tv_usec = 0;//500;	
//		DF_DEBUG("select start");
		iSend = select(sockfd + 1, NULL, &wset, &errorfd, &tv);
//		DF_DEBUG("select end");
		if (iSend > 0)//3.5秒之内可以send，即socket可以写入	 
		{	
			if(0 != FD_ISSET(sockfd, &errorfd)){
				return -1;
			}
			if(0 == FD_ISSET(sockfd, &wset)){
				return -1;
			}
			//printf("<<<<<<<<%s\n",msg);
			//flag = 1;
			
			lenSend = send(sockfd,msg + Total,len - Total,0);    
			if(lenSend <= 0)				{	
			     DBG_ErrMsg("socketfd %d send data length %d  err\n",sockfd,len);
				ret = -1;
				break;
			}
			
			Total += lenSend;	
			if(Total == len)	
			{	
				DBG_InfoMsg("socketfd %d send data length %d \n",sockfd,len);
				ret = len;	
				break;
			}
		}
		else if (iSend == 0)
		{
			
			cont ++;
			if (2 == cont)
			{
				return -1;
			}
			continue;
		}
		else  //3.5秒之内socket还是不可以写入，认为发送失败	
		{
			DBG_ErrMsg("socketfd %d send data length %d  err\n",sockfd,len);
			ret = -1;
			if (sockfd > 0)
			{
			}
			break;
		}	
	}
	return ret;

}


void *addMulicastTimerTask(void *arg)
{
	ExtParam_S *param = NULL;
	param = (ExtParam_S *)arg;
	struct ip_mreq mreq;
	prctl(PR_SET_NAME, (unsigned long)"addMulicastTimerTask", 0,0,0);
	while(    uTsockFlag==1){
		NetCab_T info = {0};
		int i = 0;
		usleep(5000*1000);
		param->getNetCabInfo(&info);
		for(i = 0;i < info.ethCount;i++){
		//	DF_DEBUG("ethip = %s\r\n",info.ethip[0]);
		//	DF_DEBUG("mulicastIp = %s\r\n",param->mulicastIp);
			memset(&mreq, 0,sizeof(struct ip_mreq));
			mreq.imr_multiaddr.s_addr = inet_addr(param->mulicastIp);
			mreq.imr_interface.s_addr = inet_addr(info.ethip[i]);
	//		DF_DEBUG("5S Checked Start\r\n");
			if (setsockopt(param->mainSockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq,sizeof(struct ip_mreq)) < 0) {
	//		  perror("Add Membership\r\n");
	//		  DF_DEBUG("setsockopt\n");
		    }
	//		DF_DEBUG("5S Checked End\r\n");
		}
	}

	free(param);
	return 0;
}

void *udpServerThead(void *arg)
{
	prctl(PR_SET_NAME, (unsigned long)"udpServerThead", 0,0,0);
	unsigned int socklen, n;
	/* 循环接收网络上来的组播消息 */
	NetCliInfo_T cliinfo;
	memset(&cliinfo,0,sizeof(NetCliInfo_T));
	NetCliInfo_T *cli = &cliinfo;
	NetServerParam_S *param = NULL;
	struct sockaddr_in peeraddr;
	param = (NetServerParam_S *)arg;
	socklen = sizeof(struct sockaddr_in);
	while(    uTsockFlag == 1) {
		//cli = (NetCliInfo_T *)malloc_checkout(sizeof(NetCliInfo_T));
		//if(NULL == cli){
		//	SleepMs(100);
		//	continue;
		//}
		memset(cli->recvmsg,0,sizeof(cli->recvmsg));
		
		n = recvfrom(param->mainSockfd, cli->recvmsg, DF_NET_MSGBUFLEN, 0,(struct sockaddr *) &peeraddr, &socklen);
		if (n < 0) {
			DBG_ErrMsg("recvfrom err in udptalk!\n");
		//	free(cli);
			continue;
		}else {
			/* 成功接收到数据报 */
			cli->fromPort = ntohs(peeraddr.sin_port);
			inet_ntop(AF_INET,&peeraddr.sin_addr,cli->fromIP,IP_ADRESS_LEN); 
			//DF_DEBUG("SUCCESS RECV MSG\r\n");
			cli->recvLen = n;
			param->readcb(cli);
		//	free(cli);
		}
		
	}

	free(param);
}

int NetMulicastServerSocketInit(char *mulicastip,int mulicastPort)
{
	struct sockaddr_in peeraddr;
//	struct in_addr ia;
	int sockfd;
	unsigned int socklen;
//	struct hostent *group;
	struct ip_mreq mreq;
	int ret = -1;
	unsigned  char  one = 1; 
    char  sock_opt = 1; 
	
	/* 创建 socket 用于UDP通讯 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		DBG_ErrMsg("socket creating err in udptalk\n");
		return -1;
	}

	/* 设置要加入组播的地址 */
	memset(&mreq, 0,sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(mulicastip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

 	if((setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, ( void  *) &sock_opt,  sizeof  (sock_opt)))== -1) { 
	
     }  // 设置 允许重用本地地址和端口
	 
    if  ((setsockopt(sockfd, IPPROTO_IP,IP_MULTICAST_LOOP,&one,  sizeof  (unsigned  char ))) == -1){ 
		
     }  //	
	/* 把本机加入组播地址，即本机网卡作为组播成员，只有加入组才能收到组播消息 */


	if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq,sizeof(struct ip_mreq)) < 0) {
		perror("Add Membership");
		
		return -1;
	}

	socklen = sizeof(struct sockaddr_in);
	memset(&peeraddr, 0, socklen);
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(mulicastPort);
	peeraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET, mulicastip, &peeraddr.sin_addr);
	/* 绑定自己的端口和IP信息到socket上 */
	if (bind(sockfd, (struct sockaddr *) &peeraddr,sizeof(struct sockaddr_in)) != 0 ){
		DBG_ErrMsg("Bind error\n");
		//assert(0);
		return -1;
	}
	
	return sockfd;
}

int SOCKET_CreateUdpMulticast(SocketWorkInfo_S *multicasthandle)
{
	pthread_t pthid;
	pthread_t pthaddMulicast;
	NetServerParam_S *param;
	ExtParam_S *extParam = NULL;
	param = (NetServerParam_S *)malloc(sizeof(NetServerParam_S));
	extParam = (ExtParam_S *)malloc(sizeof(ExtParam_S));
	param->mainSockfd = NetMulicastServerSocketInit(multicasthandle->mulicastip,multicasthandle->mulicastPort);
	param->readcb = multicasthandle->readcb;
	if(param->mainSockfd <= 0){
		free(param);
		//free(extParam);
		return -1;
	}
      uTsockFlag= 1;
	DBG_InfoMsg("NetCreateUdpMulicast!!!!\n");	
	multicasthandle->sockfd = param->mainSockfd;	
	extParam->mainSockfd = param->mainSockfd;	
	extParam->mulicastIp = multicasthandle->mulicastip;
	extParam->getNetCabInfo = multicasthandle->getNetCabInfo;
	
	pthread_attr_t attr;
		
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&pthaddMulicast, &attr, addMulicastTimerTask, (void *)extParam);
	pthread_attr_destroy (&attr);

	pthread_create(&pthid, NULL, udpServerThead, (void *)param);
	//pthread_create(&pthaddMulicast, NULL, addMulicastTimerTask,(void *)extParam);
       pthread_detach(pthid);
    //pthread_detach(pthaddMulicast);
	return 0;

}
int SOCKET_DestroyUdpMulticast(SocketWorkInfo_S *multicasthandle)
{
	uTsockFlag = 0;

	if(multicasthandle->sockfd <= 0){
		return 0;
	}
	close(multicasthandle->sockfd);
	 multicasthandle->sockfd = -1;
	return 0;
}
int  SOCKET_UdpSendMessage(char *ip, int port,char *msg,int len,char *ethname)
{
	struct sockaddr_in peeraddr;
	unsigned int socklen;
	int sockfd;
	/* 创建 socket 用于UDP通讯 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd <= 0){
		DBG_ErrMsg("socket err\n");
		return -1;
	}
	
	socklen = sizeof(struct sockaddr_in);
	memset(&peeraddr, 0, socklen);
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(port);
	inet_pton(AF_INET,ip, &peeraddr.sin_addr);
	
	//struct sockaddr_in addrSelf;//本地地址  
    //addrSelf.sin_addr.S_un.S_addr = inet_addr(ethIP);//指定网卡的地址  
    //addrSelf.sin_family = AF_INET;  
	//bind(sockfd , (struct sockaddr *)&addrSelf , sizeof(struct sockaddr_in));

#if defined(WIN32)
	
#else
	struct ifreq inter;
 	memset(&inter, 0x00, sizeof(inter));
	strncpy(inter.ifr_ifrn.ifrn_name,ethname,strlen(ethname));
//	DF_DEBUG("ethname = %s",ethname);
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&inter, sizeof(inter))  < 0) {
    
      /* Deal with error... */
	}
#endif

	sendto(sockfd,msg,len,0,(struct sockaddr *)&peeraddr,socklen);
	close(sockfd);
	return 0;


}


