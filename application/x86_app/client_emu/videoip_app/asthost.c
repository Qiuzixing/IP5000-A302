/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>

#include <getopt.h>
#include <signal.h>

#include "vdef.h"
#include "getinf.h"
#include "videoip_ioctl.h"

#define ASTHOST_GLOBALS
#include "global.h"
#include "astdebug.h"
#include "astnetwork.h"

void ast_send_event(const char *event);

typedef struct	_LISTEN_INFO
{
	int Txlsock;
	int V1Txlsock;
	int	V2Txlsock;
	int	VMTxlsock;
}LISTEN_INFO, *PLISTEN_INFO;

static const char version[] = 
	" ($Id: asthost.c 13 2007-05-27 16:17:59Z southstar $)";

char *mc_addr = NULL;
struct sockaddr_storage ss;
int no_tcp = 0;

static const FILE_SECTION Sections[] = {
{"[general]", 0xFF},
{"[quality0]", 0},
{"[quality1]", 1},
{"[quality2]", 2},
{"[quality3]", 3},
{"[quality4]", 4},
{"[quality5]", 5},
};
static int Section_Num = sizeof(Sections) / sizeof(FILE_SECTION);

static struct addrinfo *my_getaddrinfo(char *host, char *port, int ai_family)
{
	int ret;
	struct addrinfo hints, *ai_head;

	bzero(&hints, sizeof(hints));

	hints.ai_family   = ai_family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE; //for use in binding a socket for accepting incoming connections

	ret = getaddrinfo(host, port, &hints, &ai_head);
	if (ret) {
		err("%s: getaddrinfo is error", port);
		return NULL;
	}

	return ai_head;
}

static void log_addrinfo(struct addrinfo *ai)
{
	int ret;
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV];

	ret = getnameinfo(ai->ai_addr, ai->ai_addrlen, hbuf, sizeof(hbuf),
			sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
	if (ret)
		err("getnameinfo is error");

	info("listen at [%s]:%s", hbuf, sbuf);
}

#define MAXSOCK 20
int max_fd = 20; /* the highest file descriptor (for select()) */

static int listen_all_addrinfo(struct addrinfo *ai_head, int lsock[])
{
	struct addrinfo *ai;
	int n = 0;		/* number of sockets */

	for (ai = ai_head; ai && n < MAXSOCK; ai = ai->ai_next) {
		int ret;

		lsock[n] = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (lsock[n] < 0)
			continue;

		SetReuseaddr(lsock[n]);
		SetNodelay(lsock[n]);

		if (lsock[n] >= FD_SETSIZE) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = bind(lsock[n], ai->ai_addr, ai->ai_addrlen);
		if (ret < 0) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		ret = listen(lsock[n], SOMAXCONN);
		if (ret < 0) {
			close(lsock[n]);
			lsock[n] = -1;
			continue;
		}

		log_addrinfo(ai);

		/* next if succeed */
		n++;
	}

	if (n == 0) {
		err("no socket to listen to");
		return -1;
	}

	dbg("listen %d address%s", n, (n==1)?"":"es");

	return n;
}
	
static const struct option longopts[] = {
	{"daemon",	no_argument,	NULL, 'D'},
	{"debug",	no_argument,	NULL, 'd'},
	{"version",	no_argument,	NULL, 'v'},
	{"help",	no_argument,	NULL, 'h'},
	{"multicast",	required_argument,	NULL, 'm'},
	{"transport",	required_argument,	NULL, 't'},
	{NULL,		0,		NULL,  0}
};

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

#if 0
BOOL  GetCompressCrt(char *filename, PCTRL_VIDEO pCtrlVideo)
{

    BYTE          string[81],name[81];
    BYTE          StringToken[256];
    long          temp;
	int			i, quality_level;
    FILE          *fp;

    fp = fopen(filename, "rb");

	if (!fp)
		return FALSE;

	printf("%s:\n", filename);
	pCtrlVideo->QualityNum = 0;
    while (fgets ((char *)string, 80, fp) != NULL) {
		if (string[0] == '['){//section title
//			printf("section \"%s\"\n", string);
			for (i = 0; i < Section_Num; i++){
//				printf("section #%d %s\n", i, Sections[i].SectionName);
				if (!strncmp(string, Sections[i].SectionName, strlen(Sections[i].SectionName))){
//					printf("section #%d matched\n", i);
					break;
				}
			}
			if (i < Section_Num){
				quality_level = Sections[i].QualityLevel;
				printf("quality level = %d\n", quality_level);
				if (quality_level != 0xFF)
				{
					pCtrlVideo->QualityNum++;
				}
			}
			else
				quality_level = 0xFFFF;
		}
		else{
			if (quality_level < 0xFF){//setting of a specific quality
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "NormalLumaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					printf("NormalLumaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalLumaTableSelection = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "NormalChromaTableSelection") == 0) {
					temp = atol((char *)StringToken);
					printf("NormalChromaTableSelection = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].NormalChromaTableSelection = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HighQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					printf("HighQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighQualityEnable = temp;
					continue;			 
				}		 
				
				if (strcmp ((char *)name, "BestQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					printf("BestQualityEnable = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestQualityEnable = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "HighDeValueOrHighLumaJpeg") == 0) {
					temp = atol((char *)StringToken);
					printf("HighDeValueOrHighLumaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HighDeValueOrHighLumaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "BestDeValueOrHighChromaJpeg") == 0) {
					temp = atol((char *)StringToken);
					printf("BestDeValueOrHighChromaJpeg = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BestDeValueOrHighChromaJpeg = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "FrameRateControl") == 0) {
					temp = atol((char *)StringToken);
					printf("FrameRateControl = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].FrameRate = temp; 
					continue;
				}
				if (strcmp ((char *)name, "CodecMode") == 0) {
					temp = atol((char *)StringToken);
					printf("CodecMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].CodecMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "HorScale") == 0) {
					temp = atol((char *)StringToken);
					printf("HorScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].HorScale = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "VerScale") == 0) {
					temp = atol((char *)StringToken);
					printf("VerScale = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].VerScale = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "YUV420") == 0) {
					temp = atol((char *)StringToken);
					printf("YUV420 = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].YUV420 = temp;
					continue;			 
				}
				if (strcmp ((char *)name, "JpegOnlyMode") == 0) {
					temp = atol((char *)StringToken);
					printf("JpegOnlyMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].JpegOnlyMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "VQ4ColorMode") == 0) {
					temp = atol((char *)StringToken);
					printf("VQ4ColorMode = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].VQ4ColorMode = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableBCD") == 0) {
					temp = atol((char *)StringToken);
					printf("EnableBCD = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableBCD = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "BCDThreshold") == 0) {
					temp = atol((char *)StringToken);
					printf("BCDThreshold = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].BCDThreshold = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableABCD") == 0) {
					temp = atol((char *)StringToken);
					printf("EnableABCD = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableABCD = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "ABCDThreshold") == 0) {
					temp = atol((char *)StringToken);
					printf("ABCDThreshold = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].ABCDThreshold = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "Pass2BCDDelay") == 0) {
					temp = atol((char *)StringToken);
					printf("Pass2BCDDelay = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].Pass2BCDDelay = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "Pass3BCDDelay") == 0) {
					temp = atol((char *)StringToken);
					printf("Pass3BCDDelay = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].Pass3BCDDelay = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "TruncatedBits") == 0) {
					temp = atol((char *)StringToken);
					printf("TruncatedBits = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].TruncatedBits = temp;
					continue;			 
				}		 
				if (strcmp ((char *)name, "EnableDithering") == 0) {
					temp = atol((char *)StringToken);
					printf("EnableDithering = %d\n", temp);
					pCtrlVideo->QualityParam[quality_level].EnableDithering = temp;
					continue;			 
				}		 
			}
			else if (quality_level == 0xFF){//general setting
				sscanf ((char *)string, "%[^=] = %s", name, StringToken);
				
				if (strcmp ((char *)name, "DynamicQualityEnable") == 0) {
					temp = atol((char *)StringToken);
					printf("DynamicQualityEnable = %d\n", temp);
					pCtrlVideo->DynamicQualityEnable = temp;
					continue;
				}
				if (strcmp ((char *)name, "DefaultQualityMode") == 0) {
					temp = atol((char *)StringToken);
					printf("DefaultQualityMode = %d\n", temp);
					pCtrlVideo->DefaultQualityMode = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktSize") == 0) {
					temp = atol((char *)StringToken);
					printf("StreamBufPktSize = %d\n", temp);
					pCtrlVideo->StreamBufPktSize = temp;
					continue;
				}
				if (strcmp ((char *)name, "StreamBufPktNum") == 0) {
					temp = atol((char *)StringToken);
					printf("StreamBufPktNum = %d\n", temp);
					pCtrlVideo->StreamBufPktNum = temp;
					continue;
				}
			}
			else{
				printf("should not happen!!!\n");
			}
		}
    }

    fclose (fp);

	return TRUE;
}
#endif

static int my_accept_sa(int lsock)
{
	int csock;
	//struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, sizeof(host), port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

	info("connected from %s:%s", host, port);

	return csock;
}

static int my_accept(int lsock)
{
	int csock;
	struct sockaddr_storage ss;
	socklen_t len = sizeof(ss);
	char host[NI_MAXHOST], port[NI_MAXSERV];
	int ret;

	bzero(&ss, sizeof(ss));

	csock = accept(lsock, (struct sockaddr *) &ss, &len);
	if (csock < 0) {
		err("accept");
		return -1;
	}

	ret = getnameinfo((struct sockaddr *) &ss, len,
			host, sizeof(host), port, sizeof(port),
			(NI_NUMERICHOST | NI_NUMERICSERV));
	if (ret)
		err("getnameinfo, %s", gai_strerror(ret));

	info("connected from %s:%s", host, port);

	return csock;
}

static void fill_hw_udp_header(PIOCTL_REMOTE_INFO pHostInfo)
{
	struct socket_info_t *skinfo = &pHostInfo->skinfo;

	memset(skinfo, 0, sizeof(struct socket_info_t));

	switch (pHostInfo->CastingMode)
	{
	case 0: // unicast. TCP ctrl + TCP data
		// HW UDP can't support this mode. Do nothing
		break;
	case 1: // unicast. TCP ctrl + UDP data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		unicast_dst_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest);
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
		break;
	case 2: // multicast. TCP ctrl + UDP (MC) data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		multicast_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest);
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
		break;
	case 3: // multicast. UDP (MC) ctrl + UDP (MC) data
		get_dst_ip(pHostInfo->DataSockFd, &skinfo->ip_daddr, &skinfo->udp_dest);
		get_my_ip(pHostInfo->DataSockFd, &skinfo->ip_saddr, &skinfo->udp_source);
		multicast_ip_to_mac(htonl(skinfo->ip_daddr), skinfo->mac_dest);
		get_my_mac(pHostInfo->DataSockFd, skinfo->mac_source);
		get_mtu(pHostInfo->DataSockFd, &skinfo->mtu);
		break;
	default:
		break;
	};

	printf("\n\n\n\n================\n");
	printf("HW UDP Socket Info:\n");
	printf("MTU: %d\n", skinfo->mtu);
	printf("Dst IP addr: 0x%08X\n", skinfo->ip_daddr);
	printf("Dst UDP Port: 0x%04X\n", skinfo->udp_dest);
	printf("Dst MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", skinfo->mac_dest[0], skinfo->mac_dest[1], skinfo->mac_dest[2], skinfo->mac_dest[3], skinfo->mac_dest[4], skinfo->mac_dest[5]);
	printf("Src MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", skinfo->mac_source[0], skinfo->mac_source[1], skinfo->mac_source[2], skinfo->mac_source[3], skinfo->mac_source[4], skinfo->mac_source[5]);
	printf("Src IP addr: 0x%08X\n", skinfo->ip_saddr);
	printf("Src UDP Port: 0x%04X\n", skinfo->udp_source);
	printf("================\n");
}

static void do_standalone_mode(int daemonize)
{
	int i, ret;
	struct addrinfo *ctrl_ai_head = NULL;
	struct addrinfo *data_ai_head = NULL;
	int	ctrl_n;
	int	data_n = 0;
	int ctrl_lsock[MAXSOCK];
	int data_lsock[MAXSOCK];
	int	devfd;
	IOCTL_REMOTE_INFO	HostInfo; 
	int DataSockFd = -1;
	int CtrlSockFd = -1;
	fd_set	fds;

	FD_ZERO(&fds);
	memset(&HostInfo, 0, sizeof(IOCTL_REMOTE_INFO));
			
	if (daemonize) {
		if (daemon(0,0) < 0)
			err("daemonizing failed");

		videoip_use_syslog = 1;
	}

	set_signal();

	if (mc_addr)
	{
		if (strncmp(mc_addr, "udp", 3))//multicast
		{
			/* If multicast address is asigned and is not 'udp' then create multicast socket. */
			DataSockFd = udp_create_sender(mc_addr, VIDEOIP_V1UDP_PORT);
			if (DataSockFd == -1) {
				exit(1);
			}
			HostInfo.DataSockFd = DataSockFd;

			if (no_tcp) {
				//UDP-only multicast
				HostInfo.CastingMode = 3;
			} else {
				 //multicast
				HostInfo.CastingMode = 2;
			}
			devfd = open ("/dev/videoip", O_RDONLY);
			fill_hw_udp_header(&HostInfo);
			ioctl(devfd, IOCTL_HOST_ENABLE_REMOTE, &HostInfo);
			close(devfd);
			if (close(DataSockFd)) {
				err("Failed to close data socket (%d)\n", errno);
			}
			DataSockFd = -1;
		}
		else//unicast
			HostInfo.CastingMode = 1;
	} 
	else//TCP-only unicast 
	{
		HostInfo.CastingMode = 0;
	}
	
	if (no_tcp) {
		//add the virtual client, from which host receives CMDs in UDP-only mode
		CtrlSockFd = udp_connect(mc_addr, VIDEOIP_RX_PORT);
		if (CtrlSockFd < 0) {
			err("multicast socket for ctrl connect fail\n");
			exit(1);
		}
		HostInfo.CtrlSockFd = CtrlSockFd;

		devfd = open ("/dev/videoip", O_RDONLY);
		//fill_hw_udp_header(&HostInfo);
		ioctl (devfd, IOCTL_HOST_ADD_CLIENT, &HostInfo);
		close(devfd);

		ast_send_event("e_videoip_init_ok");
		goto out;
	}
	else
	{
		if (!mc_addr) {
			data_ai_head = my_getaddrinfo(NULL, VIDEOIP_V1TX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
			if (!data_ai_head)
				return;

			data_n = listen_all_addrinfo(data_ai_head, data_lsock);
			if (data_n <= 0)
			{
				err("V1Tx no socket to listen to");
				exit(1);
			}

			if (data_n > 1)
			{
				err("V1Tx listion sock > 1");
				exit(1);
			}
		}
		/* Rx socket */		
		ctrl_ai_head = my_getaddrinfo(NULL, VIDEOIP_RX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
		if (!ctrl_ai_head)
			return;
		ctrl_n = listen_all_addrinfo(ctrl_ai_head, ctrl_lsock);
		if (ctrl_n <= 0)
			err("no socket to listen to");
		info("Rx sock num [%d]", ctrl_n);
		for(i=0; i<ctrl_n;i++)
			FD_SET(ctrl_lsock[i], &fds);	

		ast_send_event("e_videoip_init_ok");
		//loop
		while(1)
		{
			info( "\n\nwaiting for connections" );
			info("-----------------------------------------------------------");
			ret = select(max_fd+1, &fds, NULL, NULL, NULL);
			if( ret < 0 ) {
				if( errno != EINTR )
					err( "select returned funny value" );
	
				info( "bye..." );
				goto out;
			}
	
			for( i=0; i<ctrl_n; i++ ) {
				if( FD_ISSET(ctrl_lsock[i], &fds)) {
					CtrlSockFd = my_accept_sa(ctrl_lsock[i]);
					if(CtrlSockFd < 0 ) {
						err("accept()-ing failed");
						continue;
					}
					//dbg("accepted new socket: %d", sock);
					HostInfo.CtrlSockFd = CtrlSockFd;
					if (!mc_addr) {//TCP-only unicast
						DataSockFd = my_accept(data_lsock[data_n-1]);
						if(DataSockFd < 0) {
							err("V1Tx accept()-ing failed");
						}
						HostInfo.DataSockFd = DataSockFd;
					} else if (!strncmp(mc_addr, "udp", 3)){//unicast
						DataSockFd = udp_create_sender(inet_ntoa(((struct sockaddr_in*)(&ss))->sin_addr), 
													   VIDEOIP_V1UDP_PORT);
						HostInfo.DataSockFd = DataSockFd;
					}
					devfd = open("/dev/videoip", O_RDONLY);
					if (DataSockFd != -1) {
						fill_hw_udp_header(&HostInfo);
						ioctl(devfd, IOCTL_HOST_ENABLE_REMOTE, &HostInfo);
					}
					ioctl (devfd, IOCTL_HOST_ADD_CLIENT, &HostInfo);
					if (DataSockFd > 0) {
						if (close(DataSockFd)) {
							err("Failed to close data socket (%d)\n", errno);
						}
						DataSockFd = -1;
					}
					close(devfd);
					if (close(CtrlSockFd)) {
						err("Failed to close control socket fd (%d)\n", errno);
					}
					CtrlSockFd = -1;
				}
			}
		}
	}

out:
	if (!no_tcp) {
		freeaddrinfo(ctrl_ai_head);
		if (!mc_addr)
			freeaddrinfo(data_ai_head);
	}

	if (CtrlSockFd != -1)
		close(CtrlSockFd);
	if (DataSockFd != -1)
		close(DataSockFd);
	
	return;
}

#if 0
static void do_standalone_mode(BOOL daemonize)
{
	int ret;
	int V1Txlsock[MAXSOCK];
	int V1Txcsock[MAXSOCK];
	int V2Txlsock[MAXSOCK];
	int V2Txcsock[MAXSOCK];
	int VMTxlsock[MAXSOCK];
	int VMTxcsock[MAXSOCK];

	int Txlsock[MAXSOCK];
	int Txcsock[MAXSOCK];	
	int Rxlsock[MAXSOCK];
	int Rxcsock[MAXSOCK];
	
	int i;
	
	struct addrinfo *ai_V1Txhead;
	struct addrinfo *ai_V2Txhead;
	struct addrinfo *ai_V3Txhead;
	
	struct addrinfo *ai_Txhead;
	struct addrinfo *ai_Rxhead;
	int n;
	int	devfd;
	
	IOCTL_HOST_INFO	HostInfo; 

	devfd = open ("/dev/videoip", O_RDONLY);
	
	if (daemonize) {
		if (daemon(0,0) < 0)
			err("daemonizing failed");

		videoip_use_syslog = 1;
	}

	set_signal();

	
	//V1Tx
	ai_V1Txhead = my_getaddrinfo(NULL, VIDEOIP_V1TX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
	if (!ai_V1Txhead)
		return;

	n = listen_all_addrinfo(ai_V1Txhead, V1Txlsock);
	if (n <= 0)
		err("no socket to listen to");
	//V2Tx
	ai_V2Txhead = my_getaddrinfo(NULL, VIDEOIP_V1TX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
	if (!ai_V1Txhead)
		return;

	n = listen_all_addrinfo(ai_V2Txhead, V1Txlsock);
	if (n <= 0)
		err("no socket to listen to");
	
	/* ---------- */
	ai_Txhead = my_getaddrinfo(NULL, VIDEOIP_TX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
	if (!ai_Txhead)
		return;

	n = listen_all_addrinfo(ai_Txhead, Txlsock);
	if (n <= 0)
		err("no socket to listen to");
	
	ai_Rxhead = my_getaddrinfo(NULL, VIDEOIP_RX_PORT_STRING, PF_UNSPEC); //PF_UNSPEC stands for using IPV6 or IPV4 by system
	if (!ai_Rxhead)
		return;

	n = listen_all_addrinfo(ai_Rxhead, Rxlsock);
	if (n <= 0)
		err("no socket to listen to");

	/* accept two port */
	for (i = 0; i < n; i++) {
        V1Txcsock[i] = my_accept(V1Txlsock[i]);
		//printf("V1Tx csock[%d] : %d \n", i, V1Txcsock[i]);
	}
	HostInfo.V1SockFd = V1Txcsock[i-1];

	for (i = 0; i < n; i++) {
        Rxcsock[i] = my_accept(Rxlsock[i]);
		//printf("Rx csock[%d] : %d \n", i, Rxcsock[i]);
	}
	HostInfo.CmdSockFd = Rxcsock[i-1];

	for (i = 0; i < n; i++) {
        Rxcsock[i] = my_accept(Txlsock[i]);
		//printf("Rx csock[%d] : %d \n", i, Rxcsock[i]);
	}
	HostInfo.CmdTxSockFd = Txcsock[i-1];
	
	//info("videoipd start (%s)", version);
	ioctl (devfd, IOCTL_HOST_START, &HostInfo);	
    pause(); //suspend process until signal
	info("asthost shutdown");

	close(devfd);
	for (int i = 0; i < n; i++) {
        close (V1Txcsock[i]);
	}
	for (int i = 0; i < n; i++) {
        close (Txcsock[i]);
	}	
	for (int i = 0; i < n; i++) {
        close (Rxcsock[i]);
	}
	
	freeaddrinfo(ai_V1Txhead);
	freeaddrinfo(ai_Txhead);
	freeaddrinfo(ai_Rxhead);
	return;
}
#endif

int main(int argc, char *argv[])
{
	int daemonize = 0;

	enum {
		cmd_standalone_mode = 1,
		cmd_help,
		cmd_version
	} cmd = cmd_standalone_mode;


	videoip_use_stderr = 1;
	videoip_use_syslog = 0;


	for (;;) {
		int c;
		int index = 0;

		c = getopt_long(argc, argv, "vhdDm:t:", longopts, &index);

		if (c == -1)
			break;

		switch (c) {
			case 'd':
				videoip_use_debug = 1;
				continue;
			case 'v':
				cmd = cmd_version;
				break;
			case 'h':
				cmd = cmd_help;
				break;
			case 'D':
				daemonize = 1;
				break;
			case 'm':
				mc_addr = optarg;
				break;
			case 't':
				if (strncmp(optarg, "no_tcp", 6) == 0)
				{
					printf("No TCP transport\n");
					no_tcp = 1;
				}
				break;
			case '?':
				break;
			default:
				err("getopt");
		}
	}

	switch (cmd) {
		case cmd_standalone_mode:
			do_standalone_mode(daemonize);
			break;
		case cmd_version:
			printf("%s\n", version);
			break;
		case cmd_help:
			//show_help();
			break;
		default:
			info("unknown cmd");
			//show_help();
	}

	return 0;
}
