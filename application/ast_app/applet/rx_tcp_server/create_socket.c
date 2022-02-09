#include <sys/types.h>
#include <sys/select.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include "create_socket.h"

#define LISTEN_NUM 2
#define TCP_PORT_NUM 8888

int create_local_socket(void)
{
    int listen_fd;
    int ret;
    struct sockaddr_un srv_addr;
    listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("cannot create communication socket");
        return CREAT_FAIL;
    }
    //set server addr_param
    srv_addr.sun_family = AF_UNIX;
    strncpy(srv_addr.sun_path, UNIX_DOMAIN, sizeof(srv_addr.sun_path) - 1);
    unlink(UNIX_DOMAIN);
    //bind sockfd & addr
    ret = bind(listen_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret == -1)
    {
        perror("cannot bind local socket server\n");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return CREAT_FAIL;
    }
    listen(listen_fd, LISTEN_NUM);
    return listen_fd;
}

int send_msg_to_local_socket(unsigned char *buf)
{
    int ret;
    char snd_buf[1024];
    int i;
    static struct sockaddr_un srv_addr;
    //creat unix socket
    int connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (connect_fd < 0)
    {
        perror("cannot create communication socket");
        return 1;
    }
    srv_addr.sun_family = AF_UNIX;
    strcpy(srv_addr.sun_path, UNIX_DOMAIN);
    //connect server
    ret = connect(connect_fd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
    if (ret == -1)
    {
        perror("cannot connect to the server");
        close(connect_fd);
        return 1;
    }
    write(connect_fd, buf, sizeof(buf));
    close(connect_fd);
    return 0;
}

int create_tcp_socket_server(void)
{
    int listen_fd;
    int ret;
    int reuse = 1;
    struct sockaddr_in addr, clint_addr;
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("cannot create communication socket");
        return CREAT_FAIL;
    }
    //set server addr_param
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT_NUM);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); //So must be set SO_REUSEADDR for the next connection
    if(ret < 0)
    {
        perror("cannot setsockopt server socket");
    }
    ret = setsockopt( listen_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&reuse, sizeof(reuse) );//close Nagle,because the data is very small
    if(ret < 0)
    {
        perror("cannot setsockopt server socket");
    }

    ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("cannot bind server socket");
        close(listen_fd);
        return CREAT_FAIL;
    }

    listen(listen_fd, LISTEN_NUM);
    return listen_fd;
}

int create_udp_socket_server(void)
{
    int listen_fd;
    int ret;
    int reuse = 1;
    struct sockaddr_in addr, clint_addr;
    listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_fd < 0)
    {
        perror("cannot create communication socket");
        return CREAT_FAIL;
    }
    //set server addr_param
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT_NUM);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)); //So must be set SO_REUSEADDR for the next connection
    ret = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));

    if (ret == -1)
    {
        perror("cannot bind server socket");
        close(listen_fd);
        return CREAT_FAIL;
    }

    //listen(listen_fd, LISTEN_NUM);
    return listen_fd;
}
