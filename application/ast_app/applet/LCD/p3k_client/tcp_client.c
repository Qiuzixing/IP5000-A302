
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>


#include "tcp_client.h"

#define BUF_SIZE 255 



int sock_fd;

int send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int size)
{
	int count;
	int err = -1;
	char send_buf[BUF_SIZE] = {0};

	fd_set rfds;
    struct timeval time = {2, 0};
	FD_ZERO(&rfds);
	FD_SET(sock_fd, &rfds);
	
	sprintf(send_buf, "%s\r", cmd);
	count = send(sock_fd, send_buf, strlen(send_buf), 0);
	if (count != strlen(send_buf))
	{
		printf("send %s error, fact num:[%d], will num:[%lu] \n", cmd, count, strlen(send_buf));
		return -1;
	}
	
	err = select(sock_fd + 1, &rfds, NULL, NULL, &time);
	if (err > 0)
	{
		count = recv(sock_fd, recv_buf, size, 0);
	}
	else if (err == 0)
	{
		printf("wait [%s] respond timeout\n", cmd);
	}

	return 0;
}


int tcp_client_init(char *ip, int port)
{
	int err;
	int count;
	char recv_buf[BUF_SIZE] = {0};

	int mport = port;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr)); 		 //每个字节都用0填充
	serv_addr.sin_family = AF_INET;  				//使用IPv4地址
	serv_addr.sin_addr.s_addr = inet_addr(ip);  	//具体的IP地址
	serv_addr.sin_port = htons(mport); 				//端口

	
	err = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (err == -1)
	{
		printf("tcp connect fail\n");
		close(sock_fd);
		return -1;
	}
	else
	{
		printf("connect success\n");
	}

	//need send "#\r", start communication with p3k server

	send_p3k_cmd_wait_rsp("#\r", recv_buf, sizeof(recv_buf));
	/*
	count = send(sock_fd, "#\r", strlen("#\r"), 0);
	recv(sock_fd, recv_buf, sizeof(recv_buf), 0);
	*/
	
	return 0;
}

int tcp_client_deinit()
{
	close(sock_fd);
}


#if 0

int main(int argc, char **argv)
{
	int err;
	int sock_fd;
	int count;

	if (argc != 3)
	{
		printf("input param error\n");
		return -1;
	}

	int port = atoi(argv[2]);
	char *ip_addr = NULL;
	ip_addr = strdup(argv[1]);

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
	serv_addr.sin_family = AF_INET;  //使用IPv4地址
	serv_addr.sin_addr.s_addr = inet_addr(ip_addr);  //具体的IP地址
	serv_addr.sin_port = htons(port);  //端口

	err = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (err == -1)
	{
		printf("tcp connect fail\n");
		free(ip_addr);
		return -1;
	}

	char send_buf[BUF_SIZE] = {0};
	char recv_buf[BUF_SIZE] = {0};
	int i;

	count = send(sock_fd, "#\r", strlen("#\r"), 0);
	recv(sock_fd, recv_buf, sizeof(recv_buf), 0);
	while (1)
	{
		memset(send_buf, 0, sizeof(send_buf));
		fgets(send_buf, sizeof(send_buf), stdin);
		send_buf[strlen(send_buf) - 1] = '\r';
		
		printf("buf=%s\n", send_buf);
		
		count = send(sock_fd, send_buf, strlen(send_buf), 0);
		if (count != strlen(send_buf))
		{
			//printf("send error, want to sent:[%d], Actually sent:[%d]\n");
			free(ip_addr);
			close(sock_fd);
			return -1;
		}
		memset(recv_buf, 0, sizeof(recv_buf));
		count = recv(sock_fd, recv_buf, sizeof(recv_buf), 0);

		printf("revc buf:[%s]\n", recv_buf);
		
	}	
	free(ip_addr);
	close(sock_fd);

	return 0;

}

#endif


