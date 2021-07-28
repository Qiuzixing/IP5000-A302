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

#define THINK_AUDIO_OUT_TIME 5
#define MAX_PAYLOAD 1024  /* maximum payload size*/
#define UNIX_PATH_MAX    108
#define UDS_PATH_NAME_LM_EVENT  "#lm_event"
#define msg(fmt, args...) \
	do { \
		fprintf(stderr, fmt, ##args); \
	} while(0)

enum
{
	AUDIO_IN = 0,
	AUDIO_OUT,
	AUDIO_UNKNOW
};

static int recv_event(int event_listener, struct nlmsghdr *nlh)
{
	struct iovec iov;
	struct msghdr msghdr;
	int len = 0;

	memset(&iov, 0, sizeof(iov));
	memset(&msghdr, 0, sizeof(msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

#if 0 /* From man page, msg_name is optional. */
	struct sockaddr_nl dest_addr;
	memset(&dest_addr, 0, sizeof(dest_addr));
	msghdr.msg_name = (void *)&dest_addr;
	msghdr.msg_namelen = sizeof(dest_addr);
#endif

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

int time_difference_from_last_time(struct timespec* last_time)
{
    struct timespec cur_time;
    clock_gettime(CLOCK_MONOTONIC,&cur_time);
    int time_diff = cur_time.tv_sec - last_time->tv_sec;
    return time_diff;
}

int main(int argc, char *argv[])
{
	struct nlmsghdr *nlh = NULL;
	unsigned char nlbuf[NLMSG_SPACE(MAX_PAYLOAD)];
	int len, netlink_fd = -1;
	unsigned char *event_msg;

	int flag = AUDIO_UNKNOW;

	int ret = 0;
    int maxfd = 0;
	fd_set rset;
	struct timeval timeout;
	struct timespec last_time;
	FD_ZERO(&rset);

    nlh = (struct nlmsghdr *)nlbuf;
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    netlink_fd = create_event_listener();
	if (netlink_fd == -1)
		printf("prepare_netlink failed\n");

	maxfd = netlink_fd + 1;
    /* Read message from kernel. Just retry if something goes wrong. */
	while(1)
	{
		timeout.tv_sec = THINK_AUDIO_OUT_TIME;
		FD_ZERO(&rset);
		FD_SET(netlink_fd, &rset);
		ret = select(maxfd,&rset,NULL,NULL,&timeout);

		switch(ret)
		{
			case -1: 
				printf("select error,quit\n");
				return;
			case 0: //time out
				if(flag == AUDIO_OUT)
				{
					if(time_difference_from_last_time(&last_time) > THINK_AUDIO_OUT_TIME)
					{
						printf("audio out\n");
						flag = AUDIO_UNKNOW;
					}
				}
				break;
			default:
				len = recv_event(netlink_fd, nlh);
				sched_yield();
				if (len < 0)
				{
					continue;
				}
				/* Got netlink message. */
				event_msg = NLMSG_DATA(nlh);
				/* Make sure event_msg is null ended. */
				event_msg[len] = 0;
				if(0 == strncmp("e_audio_detect_pressed",event_msg,sizeof("e_audio_detect_pressed")))
				{
					if(flag == AUDIO_UNKNOW)
					{
						printf("audio in\n");
						flag = AUDIO_IN;
					}
					clock_gettime(CLOCK_MONOTONIC,&last_time);
				}
				else if(0 == strncmp("e_audio_detect_released",event_msg,sizeof("e_audio_detect_released")))
				{
					if(flag == AUDIO_IN)
					{
						flag = AUDIO_OUT;
						clock_gettime(CLOCK_MONOTONIC,&last_time);
					}
				}
				else
				{
					if(flag == AUDIO_OUT)
					{
						if(time_difference_from_last_time(&last_time) > THINK_AUDIO_OUT_TIME)
						{
							printf("audio out\n");
							flag = AUDIO_UNKNOW;
						}
					}
				}
				break;
		}
	}

}
