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
#include <unistd.h>
#include "create_socket.h"

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
        perror("cannot bind server socket");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return CREAT_FAIL;
    }
    return listen_fd;
}

int send_msg_to_local_socket(char *buf)
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
    write(connect_fd, buf, 256);
    close(connect_fd);
    return 0;
}
