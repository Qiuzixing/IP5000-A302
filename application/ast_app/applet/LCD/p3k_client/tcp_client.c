
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "tcp_client.h"

#define BUF_SIZE 255 

pthread_t  pid = -1;
pthread_mutex_t mutex;
pthread_cond_t cond;
int thread_stop = 0;
int had_send_cmd = 0;
int had_recv_msg = 0;

int sock_fd;

char p3k_cmd_head[100] = {0};
char p3k_respond_msg[1024] = {0};

void* Entry(void *arg)
{
    int err = 1;
    int count = 0;
    char recv_buf[1024] = {0};

    fd_set rfds;
    struct timeval time;

    char *begin_str = NULL;
    char *end_str = NULL;
    
    while(thread_stop == 0)
    {
        FD_ZERO(&rfds);
        FD_SET(sock_fd, &rfds);
        time.tv_sec = 6;
        time.tv_usec = 0;
        
        err = select(sock_fd + 1, &rfds, NULL, NULL, &time);
        if (err > 0)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            count = recv(sock_fd, recv_buf, sizeof(recv_buf), 0);
            if (count > 0)
            {
                recv_buf[count] = '\0';
                printf("Entry recv : %s\n", recv_buf);
                
                if ( (had_send_cmd == 1) && (strlen(p3k_cmd_head) > 0) )
                {
                    begin_str = strstr(recv_buf, p3k_cmd_head);
                    if (begin_str == NULL)
                    {
                        /*
                            1.如果一直有消息来，但是没有相匹配的消息，要怎么办.
                            2.p3k_cmd_head应该要清零。
                        */
                        continue;
                    }
                    else
                    {
                        if (end_str = strstr(recv_buf, "\r"))
                        { 
                            begin_str = begin_str + strlen(p3k_cmd_head);
                            memcpy(p3k_respond_msg, begin_str, end_str - begin_str);
                            p3k_respond_msg[end_str - begin_str] = '\0';
                        }
                        had_send_cmd = 0;
                        had_recv_msg = 1;
                        pthread_cond_signal(&cond);
                    }
                }
                
            }
            else
            {
                if (had_send_cmd == 1)
                {
                    pthread_cond_signal(&cond);
                }
                printf("recv fail\n");
            }
            
        }
        else if (err == 0)  //timeout
        {
            if (had_send_cmd == 1)
            {               
                printf("recv %s timeout\n", p3k_cmd_head);
                pthread_cond_signal(&cond);
            }
        }
        else //error
        {
            printf("select fail\n");
            if (had_send_cmd == 1)
            {
                pthread_cond_signal(&cond);
            }
            
            return NULL;
        }
        
    }

    return NULL;
}

int send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int recv_size, char *head, int head_size)
{
    int count;
    int err = -1;
    char buf[1024] = {0};
    char send_buf[BUF_SIZE] = {0};

    if (recv_buf == NULL || head == NULL)
    {
        printf("P3K cmd head is NULL\n");
        return -1;
    }
    
    //pthread_mutex_lock(&mutex);
    
    sprintf(send_buf, "%s\r", cmd);
    strncpy(p3k_cmd_head, head, head_size);
    had_send_cmd = 1;
    
    count = send(sock_fd, send_buf, strlen(send_buf), 0);
    if (count != strlen(send_buf))
    {
        printf("send %s error, fact num:[%d], will num:[%lu] \n", cmd, count, strlen(send_buf));
        //pthread_mutex_unlock(&mutex);
        return -1;
    }
    
    pthread_cond_wait( &cond, &mutex);
    //pthread_mutex_unlock(&mutex);

    had_send_cmd = 0;
    //must clear p3k_cmd_head,
    memset(p3k_cmd_head, 0, sizeof(p3k_cmd_head));

    if (had_recv_msg == 1)
    {
        strncpy(recv_buf, p3k_respond_msg, strlen(p3k_respond_msg));
        had_recv_msg = 0;
    }
    
    return 0;
}

int old_send_p3k_cmd_wait_rsp(char *cmd, char *recv_buf, int size)
{
    int count;
    int err = -1;
    char send_buf[BUF_SIZE] = {0};

    fd_set rfds;
    struct timeval time = {10, 0};
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

static int recv_msg_thread_init()
{
    int err = -1;
    
    thread_stop = 0;
    had_send_cmd = 0;
    had_recv_msg = 0;
    
    err = pthread_mutex_init(&mutex, NULL);
    if (err == -1)
    {
        printf("pthread_mutex_init fail\n");
        return -1;
    }
    
    err = pthread_cond_init(&cond, NULL); 
    if (err == -1)
    {
        printf("pthread_cond_init\n");
        return -1;
    }
    
    err = pthread_create(&pid, NULL, Entry, NULL);
    if (err == -1)
    {
        printf("pthread_create fail\n");
        return -1;
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
    memset(&serv_addr, 0, sizeof(serv_addr));        //每个字节都用0填充
    serv_addr.sin_family = AF_INET;                 //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr(ip);      //具体的IP地址
    serv_addr.sin_port = htons(mport);              //端口

    
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

    //start a thread for receive respond message
    if (-1 == recv_msg_thread_init())
    {
        close(sock_fd);
        return -1;
    }
    
    //need send "#\r", start communication with p3k server
    send(sock_fd, "#\r", strlen("#\r"), 0);

    
    return 0;
}

int tcp_client_deinit()
{
    thread_stop = 1;
    pthread_join(pid, NULL);
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


