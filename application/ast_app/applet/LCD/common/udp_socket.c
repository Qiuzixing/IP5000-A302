#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "udp_socket.h"

int udp_init()
{
	int fd;
	int s32One = 1;
	int err = -1;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		perror("udp init fail:");
		return -1;
	}

    err = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&s32One, sizeof(int));
	if (err != 0)
	{
		perror("set socket reuseaddr fail:");
		return -1;
	}

	return fd;
	
}

int SetSocketBufSize(int fd, int send_size)
{
	int bufSize = send_size;
    int err = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(int));
	if (err != 0)
	{
		perror("set send buf fail");
		return -1;
	}
	return 0;
}

int RecvSocketBufSize(int fd, int recv_size)
{
	int bufSize = recv_size;
    int err = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(int));
	if (err != 0)
	{
		perror("set send buf fail");
		return -1;
	}
	return 0;
	
}


int udp_bind(int fd, char *addr, int port)
{
	int err = -1;
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); 		 //每个字节都用0填充
	serv_addr.sin_family = AF_INET;  				//使用IPv4地址
	serv_addr.sin_addr.s_addr = inet_addr(addr);  	//具体的IP地址
	serv_addr.sin_port = htons(port); 				//端口

	 err = bind(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	 if (err != 0)
	 {
	 	perror("udp bind fail:");
		return -1;
	 }

	 return 0;
	
}

int send_event(int fd, char *addr, int port)
{
	int err = -1;
	
	char send_buf[20] = "MSG ARRIVED";
	int count = 0;

	
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));		 //每个字节都用0填充
	serv_addr.sin_family = AF_INET; 				//使用IPv4地址
	serv_addr.sin_addr.s_addr = inet_addr(addr);		//具体的IP地址
	serv_addr.sin_port = htons(port);				//端口

	count = sendto(fd, send_buf, strlen(send_buf), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (count != strlen(send_buf))
	{
		perror("send HELLO fail");
		return -1;
	}

	return 0;
}

int recv_event(int fd, char *recv_buf)
{
	int err = -1;
	int count = 0;
	struct sockaddr_in remote_addr;
	memset(&remote_addr, 0, sizeof(remote_addr));
	int tAddrLen = sizeof(remote_addr);

	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	struct timeval time = {10, 0};

	err = select(fd + 1, &rfds, NULL, NULL, &time);
	if (err > 0)
	{
		count = recvfrom(fd, (void*)recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remote_addr, &tAddrLen);
	}
	else if (err == 0)
	{
		printf("recv timeout\n");
		return -2;
	}
	else
	{
		printf("recv fail\n");
		return -1;
	}
	
	return 0;
}



