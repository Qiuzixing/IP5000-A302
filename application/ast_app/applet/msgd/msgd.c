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
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "type.h"
#include "debug.h"
#include "astnetwork.h"
#include "msgd.h"

#define I_C_PIPE "/var/info_local"
#define I_H_PIPE "/var/info_remote"
#define I_C_OSD_PIPE "/var/info_osd"
#define I_C_GUI_PIPE "/var/info_gui_action"

#define PIC_LOCATION "/share/"

static const struct option longopts[] = {
	{"host",	no_argument,	NULL, 'h'},
	{"client",	no_argument,	NULL, 'c'},
	{"gw_ip", required_argument,	NULL, 'd'},
	{NULL,		0,		NULL,  0}
};

static struct static_info sinfo;
static struct osd_info osd_info;


static void update_sinfo_local_ip(const char *ip);
static void update_sinfo_remote_ip(const char *ip);


static int my_accept(int lsock, char *host)
{
	int csock;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept\n");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, NI_MAXHOST, port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s\n", gai_strerror(ret));

	info("connected from %s:%s\n", host, port);

	return csock;
}

static int inform_gui_sinfo(int gsock)
{
	int ret = -1;
	struct info_hdr hdr;

	/* Send header */
	hdr.type = INFOTYPE_ST;
	hdr.data_len = sizeof(sinfo);
	ret = send(gsock, &hdr, sizeof(hdr), MSG_MORE);
	if (ret <= 0) {
		err("send hdr err (%d)\n", ret);
		goto err;
	}
	/* send data */
	ret = send(gsock, &sinfo, sizeof(sinfo), 0);
	if (ret <= 0) {
		err("send body err (%d)\n", ret);
		goto err;
	}

err:
	return ret;
}


static int check_rmsg(const int gsock, char *msg)
{
	int ret = 0;
	//We handle only messages begains as 'e_'
	if (strncmp(msg, "e_", strlen("e_")))
		return 0;
		
	if (!strncmp(msg, "e_ip_got", strlen("e_ip_got"))) {
		/* After Beta 15, avahi-daemon will be restarted for resolving console API issue
		**  and it causes resolving hostname failed.
		** To resolve this issue, we get the IP from event.
		*/
		char ip[MAX_STR_LEN];
		sscanf(msg, "e_ip_got::%[^:]%*s", ip);
		update_sinfo_local_ip(ip);
		ret = inform_gui_sinfo(gsock);
	} else if (!strncmp(msg, "e_ip_del", strlen("e_ip_del"))) {
		update_sinfo_local_ip("Unknown");
		ret = inform_gui_sinfo(gsock);
	} else if (!strncmp(msg, "e_ip_remote_got", strlen("e_ip_remote_got"))) {
		sscanf(msg, "e_ip_remote_got::%[^:]%*s", sinfo.RemoteIP);
		/* Also update local ip here, to avoid the case where ifup event was missing. */
		update_sinfo_local_ip(NULL);
		ret = inform_gui_sinfo(gsock);
	} else if (!strncmp(msg, "e_name_id", strlen("e_name_id"))) {
		sscanf(msg, "e_name_id::%[^:]%*s", sinfo.ID);
		ret = inform_gui_sinfo(gsock);
	} else {
		ret = 0;
	}
	
	return ret;
}

static int assemble_info(char *msg, struct gui_action_info *gui_data)
{
	//expected strings so far  "GUI_hide_dialog" "GUI_refresh_node" "GUI_show_dialog"
	int ret = 0;
	char* tmp;

	if (strncmp(msg, "GUI_hide_dialog", strlen("GUI_hide_dialog")) == 0)
	{
		gui_data->action_type = ACTION_GUI_CHANGES;
		gui_data->ub.show_dialog = GUI_HIDE_DIALOG;
		ret = 0;
	}
	else if(strncmp(msg, "GUI_show_dialog", strlen("GUI_show_dialog")) == 0)
	{
		gui_data->action_type = ACTION_GUI_CHANGES;
		gui_data->ub.show_dialog = GUI_SHOW_DIALOG;
		ret = 0;
	}
	else if(strncmp(msg, "GUI_refresh_node", strlen("GUI_refresh_node")) == 0)
	{
		gui_data->action_type = ACTION_NODE_REFRESH;
		gui_data->ub.refresh_node = GUI_REFRESH_NODE;
		ret = 0;
	}
	else if(strncmp(msg, "GUI_show_picture", strlen("GUI_show_picture")) == 0)
	{
		gui_data->action_type = ACTION_GUI_SHOW_PICTURE;

		//The command delimiter is double colon
		tmp = strtok (msg,":");
		tmp = strtok (NULL,":");
		strcpy(gui_data->ub.show_info.picture_name, PIC_LOCATION);
		strcat(gui_data->ub.show_info.picture_name, tmp);
		tmp= strtok (NULL, ":");
		if(strcmp("y",tmp) == 0) {
			gui_data->ub.show_info.show_text = GUI_SHOW_TEXT;
		} else {
			gui_data->ub.show_info.show_text = GUI_HIDE_TEXT;
		}

		ret = 0;
	}
	else
	{ 
		printf("msgd got unknown string\n");
		//invalid message string, check link_mgr.sh
		ret = 1;
	}

	return ret;
}

static int inform_gui_action_info(const int gsock, char *msg)
{
	int ret = -1;
	struct info_hdr hdr;
	struct gui_action_info data;

	ret = assemble_info(msg, &data);
	if (ret != 0) {
		// <0 means error. == 0 means runtime information. > 0 means this message is accepted by check_rmsg().
		goto err;
	}
	
	/* Send header */
	hdr.type = INFOTYPE_GUI_ACTION;
	hdr.data_len = sizeof(data);
	ret = send(gsock, &hdr, sizeof(hdr), MSG_MORE);
	if (ret <= 0) {
		err("send hdr err (%d)\n", ret);
		goto err;
	}
	/* send data */
	ret = send(gsock, &data, sizeof(data), 0);
	if (ret <= 0) {
		err("send body err (%d)\n", ret);
		goto err;
	}
	
err:
	return ret;
}

static int inform_gui_osd(const int gsock, char *msg)
{
	int ret = -1;
	struct info_hdr hdr;
	char *ptr;

	/* send data */
	ptr = strsep(&msg, ",");
	if (ptr == NULL)
		return 0;
	strncpy(osd_info.str, ptr, OSD_STR_BUF_SIZE - 1);
	ptr = strsep(&msg, ",");
	osd_info.font_size = atoi(ptr);
	if (!osd_info.font_size)
		osd_info.font_size = 20;
#if 0
	if (ptr == NULL)
		osd_info.font_size = 20;
	else {
		osd_info.font_size = atoi(ptr);
	}
#endif
	ptr = strsep(&msg, ",");
	osd_info.font_color = (unsigned int)strtoul(ptr, NULL, 0);
	if (!osd_info.font_color)
		osd_info.font_color = 0xFF00FF00;
#if 0
		if (ptr == NULL)
		osd_info.font_color = 0xFF00FF00;
	else {
		osd_info.font_color = (unsigned int)strtoul(ptr, NULL, 0);
	}
#endif
	//sscanf(msg, "%x %d %s", &osd_info.font_color, &osd_info.font_size, osd_info.str);
#if 0
	osd_info.font_color = 0xFF00FF00;
	osd_info.font_size = 20;
	strncpy(osd_info.str, msg, OSD_STR_BUF_SIZE - 1);
#endif
	/* Send header */
	hdr.type = INFOTYPE_OSD;
	hdr.data_len = sizeof(osd_info);
	ret = send(gsock, &hdr, sizeof(hdr), MSG_MORE);
	if (ret <= 0) {
		err("send hdr err (%d)\n", ret);
		goto err;
	}
	ret = send(gsock, &osd_info, hdr.data_len, 0);
	if (ret <= 0) {
		err("send body err (%d)\n", ret);
		goto err;
	}
	info("OSD msg %08X %d %s\n", osd_info.font_color, osd_info.font_size, osd_info.str);
	
err:
	return ret;
}

static int inform_gui_rinfo(const int gsock, char *msg)
{
	int ret = -1;
	struct info_hdr hdr;

	ret = check_rmsg(gsock, msg);
	if (ret != 0) {
		// <0 means error. == 0 means runtime information. > 0 means this message is accepted by check_rmsg().
		goto err;
	}
	
	/* Send header */
	hdr.type = INFOTYPE_RT;
	hdr.data_len = strlen(msg) + 1;
	ret = send(gsock, &hdr, sizeof(hdr), MSG_MORE);
	if (ret <= 0) {
		err("send hdr err (%d)\n", ret);
		goto err;
	}
	/* send data */
	ret = send(gsock, msg, strlen(msg) + 1, 0);
	if (ret <= 0) {
		err("send body err (%d)\n", ret);
		goto err;
	}

	
err:
	return ret;
}


static int handle_msg_from_gui(int gsock)
{
	int ret = -1;
	struct info_hdr hdr;
	
	ret = recv(gsock, &hdr, sizeof(hdr), MSG_WAITALL);
	if (ret <= 0) {
		err("error recv (%d)\n", ret);
		goto err;
	}

	switch (hdr.type)
	{
	case CMD_GET_ST:
		ret = inform_gui_sinfo(gsock);
		break;
	default:
		err("Unknown cmd (%d)\n", hdr.type);
	}

	
err:
	return ret;
}

static void update_sinfo_fw()
{
#define MAX_SIZE 256
	FILE *fp;
	char name[MAX_SIZE];
	char ver[MAX_SIZE];
	char date[MAX_SIZE];
	int lver = 0;
	
	fp = fopen("/etc/version", "r");
	if (fp == NULL) {
		err("ERROR! can't open event_pipe?!\n");
		goto err;
	}
	fgets(name, MAX_SIZE, fp);
	fgets(ver, MAX_SIZE, fp);
	fgets(date, MAX_SIZE, fp);

	for (lver = strlen(ver); lver > 0 && (ver[lver - 1] == '\r' || ver[lver - 1] == '\n'); lver--)
	{
		ver[lver - 1] = '\0';
	}
	
	snprintf(sinfo.FW, MAX_STR_LEN, "%s %s", date, ver);
	err("%s\n", sinfo.FW);
	
	fclose(fp);
err:
	return;

#undef MAX_SIZE
}

static void update_sinfo_local_ip(const char *ip)
{
	char Buf [NI_MAXHOST];
	struct hostent *Host;

	/* If the ip is valid, use it. */
	if (ip != NULL) {
		strncpy(sinfo.IP, ip, MAX_STR_LEN);
		return;
	}
	/* else get it by myself */
	// Resolve hostname may fail, so, don't set IP as Unknown.
	//strcpy(sinfo.IP, "Unknown");

	gethostname(Buf, NI_MAXHOST);
	printf ("%s\n", Buf);
	strcat(Buf, ".local");
	Host = (struct hostent *)gethostbyname(Buf);
	if (Host) {
		printf("The name :: %s\n", Host->h_name) ;
		strcpy(sinfo.IP, (char*)inet_ntoa(*((struct in_addr *)Host->h_addr)));
		printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)Host->h_addr)));
	} else {
		err("Failed to get host by name\n");
	}
}

static void init_sinfo()
{
	
	update_sinfo_local_ip("Unknown");
	strcpy(sinfo.RemoteIP, "Unknown");
	strcpy(sinfo.ID, "Unknown");
	update_sinfo_fw();
}

static inline int open_info_gui_action(FILE **fp)
{
	int fd;
	
	fd = open(I_C_GUI_PIPE, (O_RDWR | O_NONBLOCK));
	if (fd < 0) {
		err("ERROR! fd can't open %s?!\n", I_C_GUI_PIPE);
	}
	*fp = fdopen(fd, "r+");

	return fd;
}

static inline int open_info_client_osd(FILE **fp)
{
	int fd;
	
	fd = open(I_C_OSD_PIPE, (O_RDWR | O_NONBLOCK));
	if (fd < 0) {
		err("ERROR! fd can't open %s?!\n", I_C_OSD_PIPE);
	}
	*fp = fdopen(fd, "r+");

	return fd;
}

static inline int open_info_client(FILE **fp)
{
	int fd;
	
	fd = open(I_C_PIPE, (O_RDWR | O_NONBLOCK));
	if (fd < 0) {
		err("ERROR! fd can't open %s?!\n", I_C_PIPE);
	}
	*fp = fdopen(fd, "r+");

	return fd;
}

static inline int open_info_host(FILE **fp)
{
	int fd;

	fd = open(I_H_PIPE, (O_RDWR | O_NONBLOCK));
	if (fd < 0) {
		err("ERROR! fd can't open %s?!\n", I_H_PIPE);
	}
	*fp = fdopen(fd, "r+");

	return fd;
}

static int handle_gui_action_from_client(int gsock, FILE *fp)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	//rewind()  is a must for pipe operation.
	rewind(fp);
	while((read = getdelim(&line, &len, '\0', fp)) != -1) {
		/* eat all '\n' at the end of string */
		while (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		
		inform_gui_action_info(gsock, line);
	}
	if (line)
		free(line);
	
	return 0;
}

static int handle_msg_from_client_osd(int gsock, FILE *fp)
{
	//char msg[STR_BUF_SIZE];
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	//rewind()  is a must for pipe operation.
	rewind(fp);
	while((read = getdelim(&line, &len, '\0', fp)) != -1) {
		/* eat all '\n' at the end of string */
		while (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		
		inform_gui_osd(gsock, line);
	}
	if (line)
		free(line);

	return 0;
}


static int handle_msg_from_client(int gsock, FILE *fp)
{
	char msg[STR_BUF_SIZE];

	//rewind()  is a must for pipe operation.
	rewind(fp);
	while(fgets(msg, MAX_STR_LEN, fp) != NULL)
	{
		/* eat all '\n' at the end of string */
		while (msg[strlen(msg) - 1] == '\n')
			msg[strlen(msg) - 1] = '\0';
		
		info("%s len:%d\n", msg, strlen(msg));

		inform_gui_rinfo(gsock, msg);
	}
	
	return 0;
}

static int handle_msg_from_host(int gsock, FILE *fp)
{

	char msg[STR_BUF_SIZE];

	//rewind()  is a must for pipe operation.
	rewind(fp);
	fgets(msg, MAX_STR_LEN, fp);
	/* eat all '\n' at the end of string */
	while (msg[strlen(msg) - 1] == '\n')
		msg[strlen(msg) - 1] = '\0';
	
	info("%s len:%d\n", msg, strlen(msg));
	
	return inform_gui_rinfo(gsock, msg);
}


static void do_msgd_host(void)
{
	int csock = -1, lsock = -1;
	struct timeval timeout;
	char host_n[NI_MAXHOST];
	fd_set fds;
	struct sockaddr_in gw_addr;
	int ret = -1;
	int fd_c, fd_h, fd_osd, fd_gui;
	FILE *fp_c, *fp_h, *fp_osd, *fp_gui;
	int max_fd = 0;


	fd_c = open_info_client(&fp_c);
	if (fd_c < 0)
		goto done;
	fd_h = open_info_host(&fp_h);
	if (fd_h < 0)
		goto done;
	fd_osd = open_info_client_osd(&fp_osd);
	if (fd_osd < 0)
		goto done;
	fd_gui = open_info_gui_action(&fp_gui);
	if (fd_gui < 0)
		goto done;
	max_fd = fd_gui;
	
	lsock = socket(AF_INET, SOCK_STREAM, 0);
	if (lsock == -1) {
		err("error socket\n");
		goto done;
	}
	SetReuseaddr(lsock);
	SetNodelay(lsock);

	gw_addr.sin_family = AF_INET;
	gw_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	gw_addr.sin_port = htons(MSGD_FE_PORT);
	ret = bind(lsock, (struct sockaddr *)&gw_addr, sizeof(gw_addr));
	if (ret != 0) {
		err("error bind\n");
		goto done;
	}

	ret = listen(lsock, 1);
	if (ret != 0) {
		err("error listen\n");
		goto done;
	}

	init_sinfo();
	//send_event(-1, "e_heartbeat_init_ok");
	while (1) {
		csock = my_accept(lsock, host_n);
		if(csock < 0) {
			err("error accept\n");
			continue;
		}
		break;
	}
	if (csock > max_fd)
		max_fd = csock;

	info("gui socket connected\n");
	//send_event(-1, "e_attaching");
	while (1) {
		FD_ZERO(&fds);
		FD_SET(csock, &fds);
		FD_SET(fd_c, &fds);
		FD_SET(fd_h, &fds);
		FD_SET(fd_osd, &fds);
		FD_SET(fd_gui, &fds);
		ret = select(max_fd + 1, &fds, NULL, NULL, NULL);
		if (ret < 0) {
			err("error select\n");
			//to do
		} else if (ret == 0) {
			info("timeouted\n");
			break;
		} 
		if (FD_ISSET(csock, &fds)){
			info("from GUI\n");
			ret = handle_msg_from_gui(csock);
			if (ret <= 0)
				break;
		} 
		if (FD_ISSET(fd_c, &fds)){
			info("from client\n");
			ret = handle_msg_from_client(csock, fp_c);
			if (ret < 0)
				break;
		} 
		if (FD_ISSET(fd_h, &fds)){
			info("from host\n");
			ret = handle_msg_from_host(csock, fp_h);
			if (ret < 0)
				break;
		} 
		if (FD_ISSET(fd_osd, &fds)){
			info("from client osd\n");
			ret = handle_msg_from_client_osd(csock, fp_osd);
			if (ret < 0)
				break;
		}
		if (FD_ISSET(fd_gui, &fds)){
			info("from client gui action\n");
			ret = handle_gui_action_from_client(csock, fp_gui);
			if (ret < 0)
				break;
		}
	}

done:
	err("msgd exiting\n");
	if (csock >= 0)
		close(csock);
	if (lsock >= 0)
		close(lsock);
	if (fd_c > 0)
		close(fd_c);
	if (fd_h > 0)
		close(fd_h);
	if (fd_osd > 0)
		close(fd_osd);
	//send_event(-1, "e_no_heartbeat");

	exit (ret);
}


int main(int argc, char *argv[])
{
	char *gw_ip;
	
	enum {
		cmd_host,
		cmd_client,
		cmd_help
	} cmd = cmd_host;


	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "hcd:", longopts, &index);

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
			case '?':
				cmd = cmd_help;
				break;
			default:
				err("getopt\n");
		}
	}

	switch (cmd) {
		case cmd_host:
			do_msgd_host();
			break;
		case cmd_client:
			//argv[2] should be the host's ip address
			//do_heartbeat_client(gw_ip);
			break;
		case cmd_help:
			//show_help();
			//ast_heartbeat -h 
			//ast_heartbeat -c  -d gw_ip
			break;
		default:
			err("unknown cmd\n");
			//show_help();
	}

	return 0;
}
