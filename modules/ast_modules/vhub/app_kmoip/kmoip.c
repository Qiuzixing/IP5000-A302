/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define KMOIP_PORT_DS 59702 /* 6756 */
#define KMOIP_PORT_US 59703 /* 6757 */

#define IP_TTL_DEFAULT 64
#define IP4_MULTICAST(x)    (((x) & htonl(0xf0000000)) == htonl(0xe0000000))


typedef struct _IO_ACCESS_DATA {
	int upstreamfd;
	int downstreamfd;
} IO_ACCESS_DATA, *PIO_ACCESS_DATA;

#define IOCTL_KMOIP_START_CLIENT  0x1115
#define IOCTL_KMOIP_START_HOST    0x1116

static int udp_create_sender(char *dst_addr, int port)
{
	struct sockaddr_in addr;
	int fd;
	struct ip_mreq mreq;
	int yes = 1;

	printf("kmoip: udp_create_sender (%s, %d)\n", dst_addr, port);
	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* set up bind address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	/* bind to send address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(dst_addr);
	addr.sin_port = htons(port);

	if (IP4_MULTICAST(addr.sin_addr.s_addr)) {
		unsigned char loop = 0;
		unsigned char ttl = IP_TTL_DEFAULT;

		if (0 > setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)))
			perror("sockopt: mulitcast loopback");

		if (0 > setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)))
			perror("sockopt: ttl");
	}

	if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}

	return fd;
}

static int udp_create_receiver(char *mgroup, int port)
{
	struct sockaddr_in addr;
	struct ip_mreq mreq;
	int fd;
	int yes = 1;
	unsigned int ip_to_listen = 0;

	printf("kmoip: udp_create_receiver (%s, %d)\n", (mgroup)?(mgroup):("NULL"), port);
	if (mgroup)
		ip_to_listen = inet_addr(mgroup);

	/* create what looks like an ordinary UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	/* allow multiple sockets to use the same PORT number */
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
		perror("setsockopt (SO_REUSEADDR)");
		close(fd);
		return -1;
	}

	/* set up destination address */
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	/*
	** Bruce161102. By specifying addr.sin_addr.s_addr=ip_to_listen
	** only IP:Port matched will be forwarded from IP layer to app.
	** If specified with 'INADDR_ANY', then only 'port' will be matched.
	** Then different multicast IP will be forwarded to this socket when
	** port numbers are the same.
	*/
	if (IP4_MULTICAST(ip_to_listen)) {
		addr.sin_addr.s_addr = (in_addr_t)ip_to_listen;
	} else {
		/* for unicast, bind to any local address interface will be fine. */
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	addr.sin_port = htons(port);

	/* bind to receive address */
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	if (mgroup != NULL) {
		/* use setsockopt() to request that the kernel join a multicast group */
		mreq.imr_multiaddr.s_addr = (in_addr_t)ip_to_listen;
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
			perror("setsockopt (IP_ADD_MEMBERSHIP)");
			close(fd);
			return -1;
		}
	}

	return fd;
}

static int old_start_client(int ufd, int dfd)
{
	IO_ACCESS_DATA IO_Data;
	int fd = open ("/dev/kmoip", O_RDONLY);

	printf ("upstream socket = %d\n", ufd);
	printf ("downstream socket = %d\n", dfd);
	IO_Data.upstreamfd = ufd;
	IO_Data.downstreamfd = dfd;
	ioctl(fd, IOCTL_KMOIP_START_CLIENT, &IO_Data);
	close(fd);
}

// Return true is file is exist and allow for read/write
// Otherwise, Return false
inline int file_exist(const char *pathname)
{
	return !(access(pathname, /*R_OK|*/W_OK|F_OK));
}

static int start_client(int ufd, int dfd, char *mac, int idx_x, int idx_y)
{
#define buf_size 128
	char *sysfs_path = "/sys/devices/platform/kmoip/attach";
	char buf[buf_size];
	int ret = 0;
	FILE *fd = NULL;

	if (!file_exist(sysfs_path)) {
		ret = -1;
		goto done;
	}

	fd = fopen(sysfs_path, "w");
	if (fd == NULL) {
		printf("failed to open %s\n", sysfs_path);
		ret = -2;
		goto done;
	}

	memset(buf, 0, buf_size);
	snprintf(buf, buf_size, " %s %i %i %u %u", mac, idx_x, idx_y, ufd, dfd);
	ret = fwrite(buf, 1, strlen(buf), fd);
	if (ret != strlen(buf)) {
		printf("write %s failed?!\n", sysfs_path);
		ret = -3;
		goto done;
	}

	ret = 0;
	fflush(fd);
	fclose(fd);

done:
	return ret;
#undef buf_size

}

static int start_host(int ufd, int dfd)
{
#define buf_size 16
	char *sysfs_path = "/sys/devices/platform/vhci_hcd/attach_kmoip";
	char buf[buf_size];
	int ret = 0;
	FILE *fd = NULL;

	if (!file_exist(sysfs_path)) {
		ret = -1;
		goto done;
	}

	fd = fopen(sysfs_path, "w");
	if (fd == NULL) {
		printf("failed to open %s\n", sysfs_path);
		ret = -2;
		goto done;
	}

	memset(buf, 0, buf_size);
	snprintf(buf, buf_size, "%u %u", ufd, dfd);
	ret = fwrite(buf, 1, strlen(buf), fd);
	if (ret != strlen(buf)) {
		printf("write %s failed?!\n", sysfs_path);
		ret = -3;
		goto done;
	}

	ret = 0;
	fflush(fd);
	fclose(fd);

done:
	return ret;
#undef buf_size
}


static int do_client(char *addr, unsigned int unicast_mode, char *mac, int idx_x, int idx_y)
{
	int upstreamfd = -1, downstreamfd = -1;
	int ret = 0;

	//create the unicast UDP data sender
	upstreamfd = udp_create_sender(addr, KMOIP_PORT_US);
	if (upstreamfd < 0)
	{
		printf("failed to create the KMoIP upstream socket\n");
		ret = -1;
		goto done;
	}

	if (unicast_mode)
		downstreamfd = udp_create_receiver(NULL, KMOIP_PORT_DS);
	else
		downstreamfd = udp_create_receiver(addr, KMOIP_PORT_DS);

	if (downstreamfd < 0)
	{
		printf("failed to create KMoIP downstream socket\n");
		ret = -2;
		goto done;
	}

	ret = start_client(upstreamfd, downstreamfd, mac, idx_x, idx_y);
	if (ret)
		printf("kmoip: start_host() failed (%i)\n", ret);

done:
	return ret;
}

static int do_host(char *addr, unsigned int unicast_mode)
{
	int upstreamfd = -1, downstreamfd = -1;
	int ret = 0;

	//create the UDP unicast data receiver
	if (unicast_mode)
		upstreamfd = udp_create_receiver(NULL, KMOIP_PORT_US);
	else
		upstreamfd = udp_create_receiver(addr, KMOIP_PORT_US);

	if (upstreamfd < 0)
	{
		printf("failed to create the KMoIP upstream socket\n");
		ret = -1;
		goto done;
	}

	/*
	** Bruce170626.
	** Unicast mode: addr will be client's ip addr.
	** Multicast mode: addr will be multicast downstream addr.
	*/
	downstreamfd = udp_create_sender(addr, KMOIP_PORT_DS);

	if (downstreamfd < 0)
	{
		printf("failed to create KMoIP downstream socket\n");
		ret = -2;
		goto done;
	}

	ret = start_host(upstreamfd, downstreamfd);
	if (ret)
		printf("kmoip: start_host() failed (%i)\n", ret);

done:
	return ret;

}


static const char* short_opts = "hu:m:cr:x:y:";
static const struct option long_opts[] = {
	{"help",	0,	NULL, 'h'},
	{"unicast",	1,	NULL, 'u'},
	{"multicast",	1,	NULL, 'm'},
	{"is_client",	0,	NULL, 'c'},
	{"roaming",	1,	NULL, 'r'},
	{"idx_x",	1,	NULL, 'x'},
	{"idx_y",	1,	NULL, 'y'},
	{NULL,		0,		NULL,  0}
};

// $kmoip_c -m $MULTICAST_IP -a $HOST_IP
int  main(int argc, char *argv[])
{
	char *addr = NULL;
	char *host_addr = NULL;
	int ret = 0;
	unsigned int is_client = 0;
	unsigned int unicast_mode = 0;
	char *mac = NULL;
	int idx_x = 0, idx_y = 0;

	for (;;) {
		int c;

		c = getopt_long(argc, argv, short_opts, long_opts, NULL);

		if (c == -1)
			break;

		switch (c)
		{
			case 'u': /* unicast */
				/*
				** Bruce170626.
				** mc_addr could be unicast address under unicast mode.
				*/
				addr = optarg;
				unicast_mode = 1;
				printf("unicast address: %s\n", addr);
				break;
			case 'm': /* multicast */
				/*
				** Bruce170626.
				** mc_addr could be unicast address under unicast mode.
				*/
				addr = optarg;
				unicast_mode = 0;
				printf("multicast address: %s\n", addr);
				break;
			case 'c': /* Client or Host */
				/*
				** Bruce170626.
				** A7. heartbeat need IGMP queryer and there is no reason
				** we should use multicast IP under unicast mode.
				** So, we fall back to unicast IP under unicast mode.
				*/
				is_client = 1;
				break;
			case 'r': /* MAC for roaming */
				mac = optarg;
				break;
			case 'x': /* x index for roaming */
				idx_x = atoi(optarg);
				break;
			case 'y': /* y index for roaming */
				idx_y = atoi(optarg);
				break;
			default:
				//to do:print help manual
				goto done;
		}
	}

	if (is_client)
		do_client(addr, unicast_mode, mac, idx_x, idx_y);
	else
		do_host(addr, unicast_mode);

done:
	return ret;
}
