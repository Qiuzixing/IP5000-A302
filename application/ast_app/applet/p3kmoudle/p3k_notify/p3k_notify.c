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

#include "../p3klib/cfgparser.h"



int main (int argc, char const *argv[])
{
    int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
	if(sock_fd < 0)
		perror("");
	//绑定
	struct sockaddr_in myaddr;
	myaddr.sin_family=AF_INET;
	myaddr.sin_port = htons(9000);
	//inet_pton(AF_INET,"10.36.145.183",&myaddr.sin_addr.s_addr); //服务器的ip
	myaddr.sin_addr.s_addr = 0;//通配地址  将所有的地址通通绑定
	if(bind(sock_fd,(struct sockaddr*)&myaddr,sizeof(myaddr))<0)
		perror("bind");

	struct sockaddr_in addr;// ipv4套接字地址结构体 
	addr.sin_family =AF_INET;
	addr.sin_port = htons(60001);  //服务器端口
	inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr); //服务器的ip
	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);
	char ip[16]="";
	char buf[1024]="";
    printf("[%d][%s]\n",argc,argv[1]);
    memset(buf,0,sizeof(buf));
    //memcpy(buf,argv[1],strlen(argv[1]));
    sprintf(buf,"#P3K-NOTIFY %s\r",argv[1]);
    int ret = sendto(sock_fd,buf,strlen(buf),0, (struct sockaddr*)&addr,sizeof(addr));
    if(ret > 0)
    {
        printf("snedto %d",ret);
    }
	close(sock_fd);

	return 0;

    
}


