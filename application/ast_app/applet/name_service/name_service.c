/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <sched.h>
#include "debug.h"
#include "astnetwork.h"
#include "name_service.h"

#define AST_HOST_NAME_FILE "/etc/hostname"
#define AST_DEVICE_NAME_FILE "/etc/device_name"
#define AST_DEVICE_STATUS_FILE "/var/ast_device_status"
#define AST_VERSION_FILE "/etc/version"
#define AST_DNT_MAC_FILE "/etc/dnt_mac"

#if 0
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
#endif

extern char* astparam(int argc, char** argv);

static int get_channel_number()
{
    int argc = 3;
    char* argv[3] = {"astparam", "g", "ch_select"};
    char* pVal = astparam(argc, argv);
	if (NULL == pVal)
	{
		argv[1] = "r";
		pVal = astparam(argc, argv);
		if (NULL != pVal)
		{
			return atoi(pVal);
		}
	}
	return 1;	// Default Channel Number is 1;
}

static void do_reply(AST_Device_Type device_type, AST_Device_Function device_function)
{
	int q_fd, r_fd;
	ssize_t ret;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	query_struct query;
	reply_struct reply;
	char grp_addr[] = AST_NAME_SERVICE_GROUP_ADDR;

	q_fd = udp_create_receiver(grp_addr, AST_NAME_SERVICE_QUERY_PORT);
	if (q_fd == -1) {
		exit(EXIT_FAILURE);
	}
	
	while (1) {
		ret = recvfrom(q_fd, &query, sizeof(query), 0, (struct sockaddr *)&addr, &addr_len);
		if (ret == -1) {
			err("recvfrom error (%d:%s)\n", errno, strerror(errno));
#if 0 /* Bruce160815. Try not exit anyway. */
			close(q_fd);
			exit(EXIT_FAILURE);
#else
			sched_yield();
			continue;
#endif
		} else if (ret == 0) {
#if 0 /* Try not exit anyway. */
			break;
#else
			sched_yield();
			continue;
#endif
		} else {
			dbg("get name query from %s\n", inet_ntoa(addr.sin_addr));
			dbg("query device_type = %d\n", query.device_type);
			dbg("query device_function = %d\n", query.device_function);
			if (((query.device_type == Type_Any) || (device_type == query.device_type)) &&
				((query.device_function == Function_Any) || (device_function == query.device_function)))
			{
				r_fd = udp_create_sender();
				if (r_fd != -1) {
					FILE *fps = fopen(AST_DEVICE_STATUS_FILE, "r");
					FILE *fpn = fopen(AST_DEVICE_NAME_FILE, "r");
					FILE *fph = fopen(AST_HOST_NAME_FILE, "r");
					FILE *fpv = fopen(AST_VERSION_FILE, "r");
					FILE *fpd = fopen(AST_DNT_MAC_FILE, "r");
					memset(&reply, 0, sizeof(reply));
					if ((fps) && (fpn) && (fph))
					{
						fgets(reply.device_status, 31, fps);
						//remove possible new line in the end
						if (reply.device_status[strlen(reply.device_status) - 1] == '\n')
							reply.device_status[strlen(reply.device_status) - 1] = '\0';
						fgets(reply.device_name, 31, fpn);
						//remove possible new line in the end
						if (reply.device_name[strlen(reply.device_name) - 1] == '\n')
							reply.device_name[strlen(reply.device_name) - 1] = '\0';
						fgets(reply.model_name, 31, fpv);
						//remove possible new line in the end
						if (reply.model_name[strlen(reply.model_name) - 1] == '\n')
							reply.model_name[strlen(reply.model_name) - 1] = '\0';
						fgets(reply.version, 15, fpv);
						//remove possible new line in the end
						if (reply.version[strlen(reply.version) - 1] == '\n')
							reply.version[strlen(reply.version) - 1] = '\0';
						if (fpd)
						{
							fgets(reply.dnt_mac, sizeof(reply.dnt_mac), fpd);
							//remove possible new line in the end
							if (reply.dnt_mac[strlen(reply.dnt_mac) - 1] == '\n')
								reply.dnt_mac[strlen(reply.dnt_mac) - 1] = '\0';
						}
						fgets(reply.hostname, sizeof(reply.hostname), fph);
						//remove possible new line in the end
						if (reply.hostname[strlen(reply.hostname) - 1] == '\n')
							reply.hostname[strlen(reply.hostname) - 1] = '\0';

						addr.sin_port = htons(AST_NAME_SERVICE_REPLY_PORT);
						reply.device_type = device_type;
						reply.device_function = device_function;
						reply.protocol_version = 4;
						reply.service_capability = 0x05; // Telnet + http now only
						reply.channel_number = get_channel_number();
						reply.reserved[0] = '\0';
						sendto(r_fd, &reply, sizeof(reply), 0, (struct sockaddr *)&addr, addr_len);
					}
					if (fps)
						fclose(fps);
					if (fpn)
						fclose(fpn);
					if (fpv)
						fclose(fpv);
					close(r_fd);
				}
			}
			else
			{
				dbg("ignore\n");
			}
		}
	}

	close(q_fd);

	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	AST_Device_Type device_type = Type_Unknown;
	AST_Device_Function device_function = Function_Unknown;
	struct option longopts[] = {
		{"type",	required_argument,	NULL, 't'},
		{"function",	required_argument,	NULL, 'f'},
//		{"status",	required_argument,	NULL, 's'},
		{NULL,		0,		NULL,  0}
	};

	for (;;) {
		int c;
		int index = 0;

//		c = getopt_long(argc, argv, "qrn:t:f:s:", longopts, &index);
		c = getopt_long(argc, argv, "t:f:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 't':
				dbg("-t%s\n", optarg);
				if (strncmp(optarg, "host", 4) == 0)
					device_type = Type_Host;
				else if (strncmp(optarg, "client", 6) == 0)
					device_type = Type_Client;
				break;
			case 'f':
				dbg("-f%s\n", optarg);
				if (strncmp(optarg, "usb", 3) == 0)
					device_function = Function_USB;
				else if (strncmp(optarg, "digital", 7) == 0)
					device_function = Function_Digital;
				else if (strncmp(optarg, "analog", 6) == 0)
					device_function = Function_Analog;
				break;
			default:
				err("getopt error (%d)\n", c);
		}
	}
	{
		dbg("device_type = %d\n", device_type);
		dbg("device_function = %d\n", device_function);
	}

	daemon(0,0);
#if 0
	set_signal();
#endif

	do_reply(device_type, device_function);			

	return 0;
}

