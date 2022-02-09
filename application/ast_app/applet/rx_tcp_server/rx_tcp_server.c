#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <asm/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include "rx_tcp_server.h"
#include "create_socket.h"

#define SELECT_TIME_OUT 200000
#define EDID_SIZE   256
#define TX_CMD_MAX_LENGTH   128
#define MAX_NUM(a, b, c) ((a) > (b)? ((a) > (c)? (a) : (c)) : ((b) > (c)? (b):(c)))

int set_keep_live(int fd)
{
    int keepAlive = 1; // 开启keepalive属性
    int keepIdle = 10; // 如该连接在10秒内没有任何数据往来,则进行探测 
    int keepInterval = 3; // 探测时发包的时间间隔为3 秒
    int keepCount = 2; // 探测尝试的次数.如果第1次探测包就收到响应了,则后1次的不再发.

    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
    setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle));
    setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
    setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
}

int socket_connected_verify(int sock)
{
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if ((info.tcpi_state == TCP_ESTABLISHED))
    {
        return CLIENT_CONNECT;
    }
    else
    {
        return CLIENT_NOT_CONNECT;
    }
}

static int check_whether_there_is_edid_data(char *edid_data)
{
    if(edid_data[0] == 0x00 && edid_data[1] == 0xff && edid_data[2] == 0xff && edid_data[3] == 0xff && \
       edid_data[4] == 0xff && edid_data[5] == 0xff && edid_data[6] == 0xff && edid_data[7] == 0x0 )
    {
        return 0;
    }
    else
    {
        return -1;
    }  
}

static int get_sock_cmd_index(char *cmd)
{
    static socket_cmd_struct sock_cmd_list[] = {
        {WAIT_EDID_REPORT,  "wait_rx_edid"},
        {GET_EDID,          "get_rx_edid"}
    };

    int index = 0;
    int ret = 0;
    for(index = 0;index<CMD_CNT;index++)
    {
        ret = strcmp(cmd,sock_cmd_list[index].cmd_str);
        if(ret == 0)
        {
            return index;
        }
    }

    return index;
}

int main(int argc, char **argv)
{

    int len = 0;
    char edid_buf[EDID_SIZE] = {0};
    char recv_tx_cmd[TX_CMD_MAX_LENGTH] = {0};
    struct sockaddr_un clt_addr;
    struct sockaddr_in clint_addr;
    int ret = 0;
    int com_fd = 0;
    int client_connect_flag = 0;
    int listen_local_fd = create_local_socket();
    int listen_tcp_fd = create_tcp_socket_server();

    int cli_fd = CLIENT_NOT_CONNECT;
    int new_cli_fd = CLIENT_NOT_CONNECT;
    if(CREAT_FAIL == listen_local_fd || CREAT_FAIL == listen_tcp_fd)
    {
        printf("create local socket fail,quit\n");
        return 0;
    }
    int maxfd = 0;
    struct timeval timeout;
    timeout.tv_sec = 0;
    fd_set rset;
    set_keep_live(listen_tcp_fd);
    while(1)
    {
        timeout.tv_usec = SELECT_TIME_OUT;
        FD_ZERO(&rset);
        FD_SET(listen_local_fd, &rset);
        FD_SET(listen_tcp_fd, &rset);
        maxfd = MAX_NUM(listen_local_fd,listen_tcp_fd,cli_fd);
        ret = select(maxfd+1,&rset,NULL,NULL,&timeout);
        
        switch(ret)
        {
            case -1:
                printf("select error,quit\n");
                goto error_out;
            case 0: //time out
                if(cli_fd != CLIENT_NOT_CONNECT)
                {
                    if(CLIENT_NOT_CONNECT == socket_connected_verify(cli_fd))
                    {
                        close(cli_fd);
                        cli_fd = CLIENT_NOT_CONNECT;
                    }
                }
                break;
            default:
                if(FD_ISSET(listen_local_fd,&rset))
                {
                    len=sizeof(clt_addr); 
                    com_fd=accept(listen_local_fd,(struct sockaddr*)&clt_addr,&len);
                    ret=read(com_fd,edid_buf,EDID_SIZE);
                    if (ret < 0) {
                        goto error_out;
                    }
                    printf("------\n");
                    if(cli_fd != CLIENT_NOT_CONNECT)
                    {
                        ret = write(cli_fd,edid_buf,EDID_SIZE);
                        if(ret != EDID_SIZE)
                        {
                            printf("write edid to client error\n");
                            goto error_out;
                        }
                    }
                }

                if(cli_fd != CLIENT_NOT_CONNECT)
                {
                    if(FD_ISSET(cli_fd,&rset))
                    {
                        int read_num=read(cli_fd,recv_tx_cmd,sizeof(recv_tx_cmd));
                        if (read_num < 0) {
                            goto error_out;
                        }
                        else if(read_num == 0)
                        {
                            close(cli_fd);
                            cli_fd = CLIENT_NOT_CONNECT;
                            printf("---------client quit\n");
                        }
                        else
                        {
                            
                        }    
                    }
                }
     
                if(FD_ISSET(listen_tcp_fd,&rset))
                {
                    socklen_t cli_addr_len = sizeof(clint_addr);
                    new_cli_fd = accept(listen_tcp_fd,(struct sockaddr*)&clint_addr,&cli_addr_len);
                    memset(recv_tx_cmd, 0, sizeof(recv_tx_cmd));
                    int read_num=read(new_cli_fd,recv_tx_cmd,sizeof(recv_tx_cmd));
                    if (read_num < 0) {
                        goto error_out;
                    }
                    else if(read_num == 0)
                    {
                        close(new_cli_fd);
                        new_cli_fd = CLIENT_NOT_CONNECT;
                        continue;
                    }
                    else
                    {
                        /* code */
                    }

                    switch(get_sock_cmd_index(recv_tx_cmd))
                    {
                        case WAIT_EDID_REPORT:
                            if(cli_fd != CLIENT_NOT_CONNECT)
                            {
                                close(cli_fd);
                                cli_fd = new_cli_fd;
                                FD_SET(cli_fd, &rset);
                            }
                            else
                            {
                                cli_fd = new_cli_fd;
                            }
                            break;
                        case GET_EDID:
                            if(0 == check_whether_there_is_edid_data(edid_buf))
                            {
                                ret = write(new_cli_fd,edid_buf,EDID_SIZE);
                                if(ret != EDID_SIZE)
                                {   
                                    printf("write edid to client error\n");
                                    goto error_out;
                                }
                            }
                            else
                            {
                                ret = write(new_cli_fd,"no_edid",sizeof("no_edid"));
                                if(ret != sizeof("no_edid"))
                                {
                                    goto error_out;
                                }
                                
                            }
                            close(new_cli_fd);
                            new_cli_fd = CLIENT_NOT_CONNECT; 
                            break;
                        case CMD_CNT:
                            break;
                        default:
                            break;
                    }
                }
                
                break;
        }
    }
error_out:
    if(new_cli_fd != CLIENT_NOT_CONNECT)
    {
        close(new_cli_fd);
    }
    if(cli_fd != CLIENT_NOT_CONNECT)
    {
        close(cli_fd);
    }
    close(listen_local_fd);
    close(listen_tcp_fd);
    return 0;
}
