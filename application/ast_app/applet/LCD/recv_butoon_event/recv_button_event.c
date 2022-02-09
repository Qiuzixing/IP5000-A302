#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <dirent.h>

#include <getopt.h>
#include <netdb.h>       //resolve hostname
#include <string.h>      //memset
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>

#include "msg_queue.h"
#include "udp_socket.h"

#define ENTER_BUTTON      "e_key_enter_pressed"
#define UP_BUTTON         "e_key_up_pressed"
#define DOWN_BUTTON       "e_key_down_pressed"
#define LEFT_BUTTON       "e_key_left_pressed"
#define RIGHT_BUTTON      "e_key_right_pressed"

#define THINK_AUDIO_OUT_TIME 5
#define MAX_PAYLOAD 1024  /* maximum payload size*/
#define UNIX_PATH_MAX    108
#define UDS_PATH_NAME_LM_EVENT  "#lm_event"

//direction
enum 
{
    ENTER_KEY = 1,
    UP_KEY,
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
    UP_CONTINUE_PRESS,
    DOWN_CONTINUE_PRESS,
    TIMEOUT,
};


#define msg(fmt, args...) \
    do { \
        fprintf(stderr, fmt, ##args); \
    } while(0)

static int recv_event_msg(int event_listener, struct nlmsghdr *nlh)
{
    struct iovec iov;
    struct msghdr msghdr;
    int len = 0;

    memset(&iov, 0, sizeof(iov));
    memset(&msghdr, 0, sizeof(msghdr));
    iov.iov_base = (void *)nlh;
    iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);


    msghdr.msg_iov = &iov;
    msghdr.msg_iovlen = 1;

    len = recvmsg(event_listener, &msghdr, 0);
    if (len <= 0) {
        /*
        ** Bruce160304.
        ** Test: Fire 'e msg_toFFFFFFFF_e_pwr_status_chg_0' repeatly and very fast on host.
        ** Problem: client's recvmsg() will return '-1' for unknown reason. And exit(). Which stopped the LM system.
        ** Solution: Ignore this error seems fine. Not sure if there is any event lost?!
        */
        msg("EVENT: recvmsg failed?! (%d)[%d:%s]\n", len, errno, strerror(errno));
        len = -1;
        goto done;
    }
    if (!NLMSG_OK(nlh, len)) {
        msg("invalid nlh?!\n");
        len = -1;
        goto done;
    }

done:
    return len;
}

static int create_event_listener(void)
{
    struct sockaddr_nl src_addr;
    int event_listener = 0;
    
    event_listener = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
    if (event_listener == -1) {
        msg("Not root\n");
        goto done;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();  /* self pid */
    /* interested in group 1<<0 */
    src_addr.nl_groups = -1;
    if (bind(event_listener, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
        msg("event listener bind failed\n");
        goto done;
    }

    return event_listener;
done:
    if (event_listener && (event_listener != -1))
        close(event_listener);
    return -1;
}

int main()
{
    int netlink_fd = -1;
    int len = 0;
    int udp_socket = -1;
    int err = -1;
    int button_event = 0;

    
    unsigned char *event_msg;
    
    struct nlmsghdr *nlh = NULL;
    unsigned char nlbuf[NLMSG_SPACE(MAX_PAYLOAD)];
    nlh = (struct nlmsghdr *)nlbuf;
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    
    netlink_fd = create_event_listener();
    if (netlink_fd < 0)
    {
        printf("create_event_listener fail\n");
        return -1;
    }

    udp_socket = udp_init();
    if (udp_socket < 0)
    {
        return -1;
    }
    
    err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail");
        return -1;
    }
    
    int is_button = 0;
    char *substr = NULL;
    while(1)
    {
        is_button = 0;
        button_event = 0;
        
        len = recv_event_msg(netlink_fd, nlh);
        if (len > 0)
        {
            event_msg = NLMSG_DATA(nlh);
            event_msg[len] = 0;
            
            if(0 == strncmp(ENTER_BUTTON, event_msg, len) )
            {
                button_event = ENTER_KEY;
                is_button = 1;
            }
            else if ( 0 == strncmp(UP_BUTTON, event_msg, len) )
            {
                button_event = UP_KEY;
                is_button = 1;
            }
            else if (0 == strncmp(DOWN_BUTTON, event_msg, len))
            {
                button_event = DOWN_KEY;
                is_button = 1;
            }
            else if (0 == strncmp(LEFT_BUTTON, event_msg, len))
            {
                button_event = LEFT_KEY;
                is_button = 1;
            }
            else if (0 == strncmp(RIGHT_BUTTON, event_msg, len))
            {
                button_event = RIGHT_KEY;
                is_button = 1;
            }
            else if (substr = strstr(event_msg, "e_key_up_"))
            {
                substr = substr + strlen("e_key_up_");
                if (substr[0] >= '0' && substr[0] <= '9')
                {
                    button_event = UP_CONTINUE_PRESS;
                    is_button = 1;
                }
            }
            else if (substr = strstr(event_msg, "e_key_down_"))
            {
                substr = substr + strlen("e_key_down_");
                if (substr[0] >= '0' && substr[0] <= '9')
                {
                    button_event = DOWN_CONTINUE_PRESS;
                    is_button = 1;
                }
            }

            if (is_button == 1)
            {
                int num = 0;
                while(-1 == msg_send_state(button_event) )
                {
                    printf("msg_send_state fail\n");
                    msg_queue_create();
                    num++;
                    if (num >= 3)
                    {
                        break;
                    }
                }
                if (num < 3)
                {
                    send_event(udp_socket, "127.0.0.1", 10201);

                    is_button = 0;
                    button_event = 0;
                }
            
                #if 0
                err = msg_send_state(button_event);
                if (err == -1)
                {
                    printf("msg_send_state fail\n");
                }
                else
                {
                    send_event(udp_socket, "127.0.0.1", 10201);
                    
                    is_button = 0;
                    button_event = 0;
                }
                #endif
                
            }
            
        }
        else
        {
            printf("recv_event fail");
        }
    }
    close(netlink_fd);
    close(udp_socket);

    return 0;
}


