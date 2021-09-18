/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include <signal.h>
#include <time.h> //for time()
#include <errno.h>

// For send_event
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/netlink.h>
#define MAX_PAYLOAD 1024  /* maximum payload size*/


#include "astnetwork.h"
#include "debug.h"


#define HEARTBEAT_PORT 6751
#define HEARTBEAT_PORT_STR "6751"
static int heartbeatInt = 500;//msec
static int heartbeatTimeout = 3000;
#define CHECK_INTERVAL 1000 //msec
#define ALIVE_COUNT 10//5

#define MSG_TAG "msg_"
#define ACT_TAG "act_"
#define MSG_TO_TAG "msg_to" // in msg_toXXXXXXXX_string format
#define MSG_MAGIC_NUM 0x19760614
// CMD Types
#define MSG_CMDTYPE_HB 1 //heartbeat
#define MSG_CMDTYPE_ACTION 2 //action

// Client information
typedef struct
{
	unsigned long ip; //For ipV4. 4 bytes long.
	int alive; //alive count
} t_c_info;

typedef struct
{
	int MgcNum;
	int CmdType;
	int DataSize;
	unsigned long TargetIp;
	char data[MAX_PAYLOAD];
} t_msg_hdr;

#define MAX_MSG_PKT_SIZE (sizeof(t_msg_hdr))
#define MSG_PKT_HDR_SIZE (sizeof(t_msg_hdr) - MAX_PAYLOAD)

static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"gw_ip", required_argument,	NULL, 'd'},
	{"multicast", no_argument,	NULL, 'm'},
	{"token", required_argument,	NULL, 't'},
	{NULL,		0,		NULL,  0}
};


/* master file descriptor list */
fd_set master;
/* maximum file descriptor number */
int fdmax = 0;
/* listening socket descriptor */
int listener = -1, event_listener = -1;
t_c_info client_list[FD_SETSIZE];
int client_num = 0;
t_msg_hdr msg;
unsigned int is_multicast = 0;
int exit_program = 0;


static void signal_handler(int i)
{
	dbg("signal catched, code %d", i);

}

static void set_signal(void)
{
	struct sigaction act;

	bzero(&act, sizeof(act));
	act.sa_handler = signal_handler;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
}



void send_event(unsigned int dest_pid, char *event_msg)
{
	struct msghdr msg_hdr;

	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
#if 0
	struct sockaddr_nl src_addr;
	int sock_fd;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (sock_fd == -1) {
		err("socket()) failed\n");
		goto err_out;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		err("netlink send bind failed\n");
		goto err_out;
	}
#endif
	dbg("Send event:%s\n", event_msg);

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = dest_pid;   /* 0 for Linux Kernel */
	dest_addr.nl_groups = 1; /* multicast */

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh) {
		err("failed to allocate nlh\n");
		goto err_out;
	}

	/* Fill the netlink message header */
	memset(nlh, 0, sizeof(struct nlmsghdr));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();  /* self pid */
	nlh->nlmsg_flags = 0;
	/* Fill in the netlink message payload */
	//!! REMEMBER to ADD ONE SPACE after the event_msg
	sprintf(NLMSG_DATA(nlh), "%s", event_msg);
	//strcpy(NLMSG_DATA(nlh), event_msg);

	memset(&iov, 0, sizeof(struct iovec));
	memset(&msg_hdr, 0, sizeof(struct msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg_hdr.msg_name = (void *)&dest_addr;
	msg_hdr.msg_namelen = sizeof(dest_addr);
	msg_hdr.msg_iov = &iov;
	msg_hdr.msg_iovlen = 1;
#if 0
	sendmsg(sock_fd, &msg_hdr, 0);
#else
	sendmsg(event_listener, &msg_hdr, 0);
#endif

err_out:
#if 0
	if (sock_fd != -1)
		close(sock_fd);
#endif
	if (nlh)
		free(nlh);
}


static int create_event_listener(void)
{
	struct sockaddr_nl src_addr;
	int event_listener;
	
	event_listener = socket(PF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);
	if (event_listener == -1) {
		err("Not root\n");
		goto done;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();  /* self pid */
	/* interested in group 1<<0 */
	src_addr.nl_groups = -1;
	if (bind(event_listener, (struct sockaddr*)&src_addr, sizeof(src_addr))) {
		err("event listener bind failed\n");
		goto done;
	}

	return event_listener;
done:
	if (event_listener >= 0)
		close(event_listener);
	return -1;
}

static int create_listener(unsigned int is_multicast)
{
	/* server address */
	struct sockaddr_in serveraddr;
	/* listening socket descriptor */
	int listener;
	int backlog = 1;

	/* get the listener */
	if((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		err("error socket\n");
		goto done;
	}
	SetReuseaddr(listener);
	SetNodelay(listener);

	/* bind */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(HEARTBEAT_PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);
 
	if(bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{
		err("error bind\n");
		goto done;
	}
 
	/* listen */
	if (is_multicast)
		backlog = 10;

	if(listen(listener, backlog) == -1)
	{
		err("error listen\n");
		goto done;
	}

	return listener;
done:
	if (listener >= 0)
		close(listener);
	
	return -1;
}

static int get_client_fd_by_ip(unsigned long ip)
{
	int i;
	
	for (i=0; i <=fdmax; i++) {
		if(!FD_ISSET(i, &master))
			continue;
		if (i == listener)
			continue;
		if (i == event_listener)
			continue;
		if (client_list[i].ip == 0)
			continue;
		if (client_list[i].ip != ip)
			continue;
		return i;
	}
	return -1;
}

static void remove_connection(int fd, int exit)
{
	char str[512];
	
	printf("remove client[%d] %s\n", fd, inet_ntoa(client_list[fd].ip));
	sprintf(str, "e_no_heartbeat_%08X", client_list[fd].ip);
	send_event(-1, str);
#if 0 //will be done when exit_program == 1
	//Under unicast mode, there is only one client.
	if (!is_multicast)
		send_event(-1, "e_no_heartbeat");
#endif

	/* remove from master set */
	if (fd == fdmax) fdmax--;
	
	FD_CLR(fd, &master);
	/* remove from client info list */
	client_list[fd].ip = 0;
	client_list[fd].alive = 0;
	client_num--;

	/* close it... */
	close(fd);

	/* 
	** We can't determine whether client_num == 0 is caused by
	** link  off or simply all clients disconnected. And if link is off,
	** most of XXXoIP drivers assume the link manager will unload it
	** to recover the network error. So, we also need to exit_program (e_no_heartbeat)
	** in this case.
	*/
	if (exit && client_num == 0 /*&& !is_multicast*/)
		exit_program = 1;

	dbg("clien_num:%d, exit:%d, fdmax:%d\n", client_num, exit_program, fdmax);
}

static int handle_new_connection(int lfd, unsigned int is_multicast)
{
	int newfd, oldfd = -1;
	int addrlen;
	/* client address */
	struct sockaddr_in clientaddr;
	char str[512];

	if ((client_num) && !is_multicast) {
		err("Won't accept other clients under unicast mode.\n");
		return -1;
	}
	addrlen = sizeof(clientaddr);
	if((newfd = accept(lfd, (struct sockaddr *)&clientaddr, &addrlen)) == -1)
	{
		err("Server-accept() error!");
		return -1;
	}
	else
	{
		dbg("Server-accept() is OK...\n");
		info("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);

		/* 
		** Under multicast mode, VideoIP send "e_video_start_working" only once.
		** Sending e_attaching blink the LED and will never stop blinking.
		** So, we don't send "e_attaching" under multicast mode.
		*/
		if (!client_num && !is_multicast) {
			info("heartbeat socket connected\n");
			send_event(-1, "e_attaching");
		}
		
		/* We should detach the client from client_list[] with the same ip address before attaching. */
#if 0
		oldfd = get_client_fd_by_ip((unsigned long)clientaddr.sin_addr.s_addr);
		if (oldfd != -1) {
			remove_connection(oldfd, 0);
		}
#endif
		client_list[newfd].ip = (unsigned long)clientaddr.sin_addr.s_addr;
		client_list[newfd].alive = ALIVE_COUNT;
		
		client_num++;

		/* Notify which IP is attaching. */
		sprintf(str, "e_attaching_%08X", client_list[newfd].ip);
		send_event(-1, str);

		if (!is_multicast) {
			//Under unicast mode, we should close the listener so that no more clients can connect to.
			close(listener);
			FD_CLR(listener, &master);
			listener = -1;
		}

		return newfd;
	}
}

static int send_to_client(int fd, t_msg_hdr *msg, int msg_size)
{
#if 0
	struct tcp_info t_info;
	socklen_t optlen;
	
	if (getsockopt(fd, SOL_SOCKET, TCP_INFO, &t_info, &optlen) < 0) {
		info("getsockopt failed!?\n");
		return -1;
	}
	info("socket state:%x\n", t_info.tcpi_state);
#endif

	if (send(fd, msg, msg_size, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
		if (errno == EAGAIN) {
			info("Send msg failed! Socket buf full?!\n");
			return 0;
		}
		info("Send msg failed! Client disconnected?!\n");
		//remove_connection(fd,1);
		return -1;
	}

	return 0;
}


static int send_to_all_clients(t_msg_hdr *msg, int msg_size)
{
	int i;
	
	/* Send to every client. */
	for (i=0; i<=fdmax; i++) {
		if(!FD_ISSET(i, &master))
			continue;
		if (i == listener)
			continue;
		if (i == event_listener)
			continue;
		
		if (send(i, msg, msg_size, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
			if (errno == EAGAIN) {
				info("Send msg failed! Socket buf full?!\n");
				continue; //ToDo?
			}
			info("Send msg failed! Client disconnected?!\n");
			remove_connection(i, 1);
		}
	}

	return 0;
}


static int send_to_target_client(t_msg_hdr *msg, int msg_size)
{
	int i;
	
	/* Send to specified client. */
	for (i=0; i<=fdmax; i++) {
		if(!FD_ISSET(i, &master))
			continue;
		if (i == listener)
			continue;
		if (i == event_listener)
			continue;
		if (client_list[i].ip != msg->TargetIp) {
			//info("fd=%d, ip=%08X, to_match=%08X", i, client_list[i].ip, msg->TargetIp);
			continue;
		}
		//info("Send to target client\n");
		if (send(i, msg, msg_size, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
			if (errno == EAGAIN) {
				info("Send msg failed! Socket buf full?!\n");
				continue; //ToDo?
			}
			info("Send msg failed! Client disconnected?!\n");
			remove_connection(i, 1);
		}
	}

	return 0;
}


static int recv_event(int event_listener, t_msg_hdr *msg)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msghdr;
	int len = 0;
	unsigned char tmp[NLMSG_SPACE(MAX_PAYLOAD)];
	
	memset(&dest_addr, 0, sizeof(dest_addr));
	nlh = (struct nlmsghdr *)tmp;
#if 0
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	if (!nlh)
		err("failed to allocate nlh\n");
#endif

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	memset(&iov, 0, sizeof(iov));
	memset(&msghdr, 0, sizeof(msghdr));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msghdr.msg_name = (void *)&dest_addr;
	msghdr.msg_namelen = sizeof(dest_addr);
	msghdr.msg_iov = &iov;
	msghdr.msg_iovlen = 1;

	len = recvmsg(event_listener, &msghdr, 0);
	if (len <= 0) {
		err("recvmsg failed?! (%d)\n", len);
	}
	if (!NLMSG_OK(nlh, len))
		err("invalid nlh?!\n");

	/* we got some data from event socket. Send it out. */
	if (strncmp(MSG_TAG, NLMSG_DATA(nlh), strlen(MSG_TAG)) != 0) {
		//This is not what we interested. ignore it.
		dbg("ignore event:%s\n", NLMSG_DATA(nlh));
		len = 0;
		goto done;
	}

	//Found!
	strncpy(msg->data, NLMSG_DATA(nlh), MAX_PAYLOAD);
	len = strlen(msg->data);
	if (len > MAX_PAYLOAD) {
		len = MAX_PAYLOAD;
		msg->data[len] = 0; //null-end in case of....
	}
	info("Got EVENT:%s\n", msg->data);

	msg->MgcNum = MSG_MAGIC_NUM;
	msg->CmdType = MSG_CMDTYPE_ACTION;
	msg->DataSize = len + 1; //plus null end
	msg->TargetIp = 0xFFFFFFFF; //default is broadcast
	
	// handle the special "msg_toXXXXXXXX_string"
	if (strncmp(MSG_TO_TAG, msg->data, strlen(MSG_TO_TAG)) == 0) {
		// Send to specified ip address
		char ip_str[9];
		// get the ip_str
		memcpy(ip_str, msg->data + strlen(MSG_TO_TAG), 8);
		ip_str[8] = 0; //null-end
		// update the TargetIp field
		msg->TargetIp = strtoul(ip_str, NULL, 16);
		// remote the "toXXXXXXXX_"
		strcpy(msg->data + strlen(MSG_TAG), 
		       msg->data + strlen(MSG_TO_TAG) + 9);
		// update the DataSize field
		msg->DataSize = strlen(msg->data) + 1; //plus null end
		//info("Got msg_to EVENT:%s from %08X(%s)\n", msg->data, msg->TargetIp, ip_str);
	}

	len = (MSG_PKT_HDR_SIZE+msg->DataSize);
	
done:
#if 0
	if (nlh)
		free(nlh);
#endif
	return len;
}


static int __recv_event(int event_listener, t_msg_hdr *msg)
{
	/* buffer for client data */
	int nbytes;

	/* handle data from event socket */
	if((nbytes = recv(event_listener, msg->data, MAX_PAYLOAD, 0)) <= 0)
	{
		/* got error or connection closed by client */
		if(nbytes == 0)
			/* connection closed */
			printf("event socket hung up\n");
		else
			err("recv() error lol!");
		
		//ToDo. Do nothing?
	}
	else
	{
		/* we got some data from event socket. Send it out. */
		msg->data[nbytes] = 0; //null-end
		if (strncmp(MSG_TAG, msg->data, strlen(MSG_TAG)) != 0) {
			//This is not what we interested. ignore it.
			dbg("ignore event:%s\n", msg->data);
			return 0;
		}
		//Found!
		info("Got EVENT:%s\n", msg->data);

		msg->MgcNum = MSG_MAGIC_NUM;
		msg->CmdType = MSG_CMDTYPE_ACTION;
		msg->DataSize = nbytes + 1; //plus null end
		msg->TargetIp = 0;
		
		return (MSG_PKT_HDR_SIZE+msg->DataSize);
		
		
	}

	return 0;
}

static int do_cmd_action_client(int fd, char *data)
{
	char cmd[MAX_PAYLOAD];

	//replace MSG_TAG as ACT_TAG
	sprintf(cmd, "%s%08X_%s", ACT_TAG, 0, data+strlen(MSG_TAG));
	send_event(-1, cmd);
	return 0;
}

static int do_cmd_action(int fd, char *data)
{
	char cmd[MAX_PAYLOAD];

	//replace MSG_TAG as ACT_TAG
	sprintf(cmd, "%s%08X_%s", ACT_TAG, client_list[fd].ip, data+strlen(MSG_TAG));
	send_event(-1, cmd);
	return 0;
}


static int handle_msg(int fd,  t_msg_hdr *msg)
{
	int nbytes;
	int ret = 0;

	dbg("handle_msg>>");
	if((nbytes = recv(fd, msg, MSG_PKT_HDR_SIZE, 0)) <= 0)
	{
		/* got error or connection closed by client */
		if(nbytes == 0)
			/* connection closed */
			printf("socket %d hung up\n", fd);
		else
			err("recv() header error lol!");
		
		return -1;
	}
	/* we got some data from a client*/
	dbg("got hdr>>");

	if (msg->MgcNum != MSG_MAGIC_NUM) {
		err("Bad magic number?!0x%08x\n", msg->MgcNum);
		return -1;
	}
	if (msg->DataSize > MAX_PAYLOAD) {
		err("Bad DataSize %d\n", msg->DataSize);
		return -1;
	}

	if (msg->DataSize) {
		dbg("getting data>>");
		if((nbytes = recv(fd, msg->data, msg->DataSize, 0)) <= 0)
		{
			/* got error or connection closed by client */
			if(nbytes == 0)
				/* connection closed */
				printf("socket %d hung up\n", fd);
			else
				err("recv() data error lol!");
			
			return -1;
		}
		if (msg->DataSize != nbytes) {
			err("Wrong size?! Expect (%d). Actual(%d)\n", msg->DataSize, nbytes);
			return -1;
		}

	}
	//Now, we got header and data.
	dbg("got hdr and data>>");
	switch (msg->CmdType) {
	case MSG_CMDTYPE_HB:
		//echo back
		ret = send_to_client(fd, msg, nbytes);
		break;
	case MSG_CMDTYPE_ACTION:
		ret = do_cmd_action(fd, msg->data);
		break;
	default:
		err("Unknown command type? %d\n", msg->CmdType);
	}

	dbg("done<<<\n");

	return ret;
}


static int handle_msg_client(int fd,  t_msg_hdr *msg)
{
	int nbytes;

	dbg("msg coming...\n");
	if((nbytes = recv(fd, msg, MSG_PKT_HDR_SIZE, 0)) <= 0)
	{
		/* got error or connection closed by client */
		if(nbytes == 0)
			/* connection closed */
			printf("socket %d hung up\n", fd);
		else
			err("recv() header error lol!");
		
		return -1;
	}
	/* we got some data from a client*/
	if (msg->MgcNum != MSG_MAGIC_NUM) {
		err("Bad magic number?!0x%08x\n", msg->MgcNum);
		return -1;
	}
	if (msg->DataSize > MAX_PAYLOAD) {
		err("Bad DataSize %d\n", msg->DataSize);
		return -1;
	}

	if (msg->DataSize) {
		if((nbytes = recv(fd, msg->data, msg->DataSize, 0)) <= 0)
		{
			/* got error or connection closed by client */
			if(nbytes == 0)
				/* connection closed */
				printf("socket %d hung up\n", fd);
			else
				err("recv() data error lol!");
			
			return -1;
		}
		if (msg->DataSize != nbytes) {
			err("Wrong size?! Expect (%d). Actual(%d)\n", msg->DataSize, nbytes);
			return -1;
		}
	}

	//Now, we got header and data.
	switch (msg->CmdType) {
	case MSG_CMDTYPE_HB:
		//Do nothing
		break;
	case MSG_CMDTYPE_ACTION:
		do_cmd_action_client(fd, msg->data);
		break;
	default:
		err("Unknown command type? %d\n", msg->CmdType);
	}

	return 0;
}


static void check_alive(void)
{
	int i;

	for (i=0; i<=fdmax; i++) {
		if(!FD_ISSET(i, &master))
			continue;
		if (i == listener)
			continue;
		if (i == event_listener)
			continue;
		if (client_list[i].ip == 0)
			continue;
		
		dbg("check alive %d\n", i);

		if (client_list[i].alive <= 1) {
			remove_connection(i, 1);
		} else {
			client_list[i].alive--;
		}
	}
}

static void do_heartbeat_host(void)
{
	/* temp file descriptor list for select() */
	fd_set read_fds;
	/* newly accept()ed socket descriptor */
	int newfd;
	int nbytes;
	int i;
	int ret = -1;
	time_t last_check_time = time(NULL);


	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	/* zero client list */
	memset(client_list, 0, sizeof(t_c_info)*FD_SETSIZE);
 
	if ((event_listener = create_event_listener()) == -1)
		goto done;

	/* add the listener to the master set */
	FD_SET(event_listener, &master);
	/* keep track of the biggest file descriptor */
	fdmax = event_listener; /* so far, it's this one*/

	if ((listener = create_listener(is_multicast)) == -1)
		goto done;

	/* add the listener to the master set */
	FD_SET(listener, &master);
	/* keep track of the biggest file descriptor */
	if(listener > fdmax)
	{ /* keep track of the maximum */
		fdmax = listener;
	}
 
	send_event(-1, "e_heartbeat_init_ok");
	/* loop */
	for(;;)
	{
		struct timeval timeout;

		for(;;) {
			if (exit_program)
				goto done;
		
			/* copy it */
			read_fds = master;

			timeout.tv_usec = 0;
			timeout.tv_sec = (CHECK_INTERVAL / 1000);
			ret = select(fdmax+1, &read_fds, NULL, NULL, &timeout);
			if(ret < 0) {
				err("Server-select() error lol!");
				goto done;
			}
			//dbg("T(%d)(%d)\n", time(NULL), last_check_time);
			if (abs(time(NULL) - last_check_time) >= (CHECK_INTERVAL / 1000)) {
				//Do the periodic check.
				check_alive();
				//update check_time
				last_check_time = time(NULL);
			}
			if (ret > 0) {
				dbg("Server-select() is OK...\n");
				break;
			}
		}
 
		/*run through the existing connections looking for data to be read*/
		for(i = 0; i <= fdmax; i++)
		{
			if(FD_ISSET(i, &read_fds))
			{ /* we got one... */
				if(i == listener)
				{
					/* handle new connections */
					newfd = handle_new_connection(listener, is_multicast);
					if (newfd >= 0)
					{
						FD_SET(newfd, &master); /* add to master set */
						if(newfd > fdmax)
						{ /* keep track of the maximum */
							fdmax = newfd;
						}
					}
				}
				else if (i == event_listener)
				{
					//handle msg events
					nbytes = recv_event(event_listener, &msg);
					if (nbytes > 0) {
						if (msg.TargetIp == 0xFFFFFFFF) {
							send_to_all_clients(&msg, nbytes);
						} else {
							send_to_target_client(&msg, nbytes);
						}
					}
				}
				else
				{
					/* handle data from a client */
					if (handle_msg(i, &msg)) {
						remove_connection(i, 1);
					} else {
						//update alive count
						client_list[i].alive = ALIVE_COUNT;
					}
				}
			}
		}
	}

done:	
	send_event(-1, "e_no_heartbeat");

	for(i = 0; i <= fdmax; i++)
	{
		if(FD_ISSET(i, &master))
			close(i);
	}

	exit (ret);
}

static int send_hb(int sockfd, t_msg_hdr *msg)
{
	int msg_size;

	dbg("send HB\n");
	msg->MgcNum = MSG_MAGIC_NUM;
	msg->CmdType = MSG_CMDTYPE_HB;
	msg->DataSize = 0; //plus null end
	msg->TargetIp = 0;
	msg_size = MSG_PKT_HDR_SIZE + msg->DataSize;

	if (send(sockfd, msg, msg_size, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
		if (errno == EAGAIN) {
			info("Send heartbeat failed! Socket buf full?!\n");
			return 0;
		}
		info("Send heartbeat failed! disconnected?!\n");
		return -1;
	}

	return 0;
}

static void do_heartbeat_client(char *host, char *token)
{
	int sockfd = -1;
	struct timeval timeout;
	int ret = -1;
	/* temp file descriptor list for select() */
	fd_set read_fds;
	int nbytes;
	int alive = ALIVE_COUNT;
	int i;
	int is_connected = 0;
	time_t last_check_time = time(NULL);
	char event[MAX_PAYLOAD];

	/* event_listener is also used to send event, so must be create before sockfd. */
	if ((event_listener = create_event_listener()) == -1)
		goto done;

	/* add the listener to the master set */
	FD_SET(event_listener, &master);
	/* keep track of the biggest file descriptor */
	fdmax = event_listener;
	
	//sockfd = tcp_connect(host, HEARTBEAT_PORT_STR);
	sockfd = tcp_connect_timeout(host, HEARTBEAT_PORT_STR, 2);
	if (sockfd == -1) {
		err("error connect heartbeat socket\n");
		goto done;
	}
	info("heartbeat socket connected\n");

	/* add the listener to the master set */
	FD_SET(sockfd, &master);
	/* keep track of the biggest file descriptor */
	if(sockfd > fdmax)
	{ /* keep track of the maximum */
		fdmax = sockfd;
	}


	//send_event(-1, "e_heartbeat_init_ok");

	for(;;) {

		for(;;) {
			/* copy it */
			read_fds = master;

			if (is_connected) {
				timeout.tv_usec = 0;
				timeout.tv_sec = (CHECK_INTERVAL / 1000);
			} else {
				timeout.tv_usec = (ALIVE_COUNT - alive)*100*1000;
				timeout.tv_sec = 0;
			}
			ret = select(fdmax+1, &read_fds, NULL, NULL, &timeout);
			if(ret < 0) {
				err("Client-select() error lol!");
				goto done;
			}
			if (abs(time(NULL) - last_check_time) >= (CHECK_INTERVAL / 1000)
			   || !is_connected)
			{
				//Do the periodic check.
				//dbg("T");
				if (alive <= 1) {
					goto done;
				}
				alive--;
				//update check_time
				last_check_time = time(NULL);

				//Send heartbeat
				if (send_hb(sockfd, &msg))
					goto done;
			}
			if (ret > 0) {
				dbg("Client-select() is OK...\n");
				break;
			}
		}
		//message from event
		if(FD_ISSET(event_listener, &read_fds))
		{ /* we got event... */
			//handle msg events
			nbytes = recv_event(event_listener, &msg);
			if (nbytes > 0) {
				if (send(sockfd, &msg, nbytes, MSG_DONTWAIT|MSG_NOSIGNAL) < 0) {
					if (errno == EAGAIN) {
						info("Send msg failed! Socket buf full?!\n");
					} else {
						info("Send msg failed! disconnected?!\n");
						goto done;
					}
				}
			}

		}
		//message from host.
		if(FD_ISSET(sockfd, &read_fds))
		{ /* we got one... */
			/* handle data from host */
			if (handle_msg_client(sockfd, &msg)) {
				goto done;
			}
			//update alive count
			if (!is_connected) {
				if (strlen(token))
					sprintf(event, "e_heartbeat_init_ok::%s", token);
				else
					sprintf(event, "e_heartbeat_init_ok");
				
				send_event(-1, event);
				is_connected = 1;
			}
			alive = ALIVE_COUNT;
		}
		
	}
	
done:
	if (strlen(token))
		sprintf(event, "e_no_heartbeat::%s", token);
	else
		sprintf(event, "e_no_heartbeat");
	
	send_event(-1, event);

	if (sockfd >= 0)
		close(sockfd);
	if (event_listener >= 0)
		close(event_listener);

	exit(ret);

}

int main(int argc, char *argv[])
{
	char *gw_ip;
	char token[64];
	
	enum {
		cmd_host,
		cmd_client,
		cmd_help
	} cmd = cmd_host;

	//initial token
	token[0] = '\0';

	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcd:mt:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'h':
				cmd = cmd_host;
				break;
			case 'c':
				cmd = cmd_client;
				break;
			case 'd':
				gw_ip = optarg;
				break;
			case 'm':
				is_multicast = 1;
				break;
			case 't':
				strncpy(token, optarg, 64);
				break;
			case '?':
				cmd = cmd_help;
				break;
			default:
				err("getopt\n");
		}
	}

	//Bruce120724. I see no necessary to catch signal?!
	//set_signal();
	
	switch (cmd) {
		case cmd_host:
			do_heartbeat_host();
			break;
		case cmd_client:
			//argv[2] should be the host's ip address
			do_heartbeat_client(gw_ip, token);
			break;
		case cmd_help:
			//show_help();
			//ast_heartbeat -h 
			//ast_heartbeat -c  -d gw_ip
			break;
		default:
			info("unknown cmd\n");
			//show_help();
	}

	return 0;
}
