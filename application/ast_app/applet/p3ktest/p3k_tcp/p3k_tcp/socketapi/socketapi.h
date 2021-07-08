#ifndef __SOCKETAPI_H__
#define __SOCKETAPI_H__
#ifdef __cplusplus
extern "C"{
#endif
#define LISTEN_MAX_FD 20
#define IP_ADRESS_LEN 16


typedef struct _NetCliInfo_T{
	char 	fromIP[IP_ADRESS_LEN];
	int		fromPort;
	int		recvSocket;
	int 	recvLen;
	char	recvmsg[20*1024*1024];
	char	recvIp[IP_ADRESS_LEN];
	int		recvPort;
	char	ethName[IP_ADRESS_LEN];
	void	*usrdef;
}NetCliInfo_T;

typedef struct _NetCab_T{
	int					ethCount;
	char				ethip[10][IP_ADRESS_LEN];//Íø¿¨IP
	char				ethname[10][IP_ADRESS_LEN];//Íø¿¨Ãû×Ö
}NetCab_T;



typedef int (*NetTcpSerReadCbFun)(NetCliInfo_T *cli);
typedef int (*NetTcpSerCloseCbFun)(int sockfd);
typedef int (*NetCabGetInfocbFun)(NetCab_T *info);

typedef struct _SocketWorkInfo_S
{
	char				mulicastip[IP_ADRESS_LEN];//×é²¥µØÖ·
	int					mulicastPort;
	int 				serverport;
	int					sockfd;
	NetTcpSerReadCbFun 	readcb;
	NetTcpSerCloseCbFun 	closecb;
	NetCabGetInfocbFun    getNetCabInfo;
}SocketWorkInfo_S;



//server
int SOCKET_CreateTcpServer(SocketWorkInfo_S*serverhandle);

int  SOCKET_DestroyTcpServer(SocketWorkInfo_S*serverhandle);

int SOCKET_TcpSendMessage(int sockfd,char *msg,int len);

int SOCKET_Connect(char*ip,int port);

//udp ×é²¥
int SOCKET_CreateUdpMulticast(SocketWorkInfo_S *multicasthandle);

int SOCKET_DestroyUdpMulticast(SocketWorkInfo_S *multicasthandle);

int  SOCKET_UdpSendMessage(char *ip, int port,char *msg,int len,char *ethname);


#ifdef __cplusplus
}
#endif

#endif
