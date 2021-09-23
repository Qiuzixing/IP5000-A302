/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


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

#include <netdb.h> //resolve hostname
#include <string.h> //memset
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>
#include <string.h>
#include <errno.h>


#define MAXMESG 2048
#define MSG_ID_VIDEO_STATUS     0x1002U
#define MSG_ID_SINK_STATUS      0x1003U
#define MSG_ID_HOT_KEY_REPORT   0x2007U
#define MSG_ID_LDAP_STATUS      0x1011U

typedef struct
{
    unsigned long ip; //For ipV4. 4 bytes long.
} t_c_info;

int udp_create_socket(char* ip, int port)
{
    struct sockaddr_in addr;
    int fd;
    int yes = 1;

    /* create what looks like an ordinary UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        return -1;
    }

    /* allow multiple sockets to use the same PORT number */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        perror("setsockopt (SO_REUSEADDR)");
        return -1;
    }

    /* set up destination address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    /* bind to receive address */
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return -1;
    }

    return fd;
}

struct lv
{
    unsigned int len;
    unsigned char* val;
};

struct notify_hdr
{
    unsigned int len;
    unsigned int id;
    unsigned int seq;
    unsigned int ver;
    unsigned int r1;
    unsigned int r2;
};

struct st_video_lost_notify
{
    struct notify_hdr hdr;
    struct lv device_name;
    unsigned char status;   // 0 lost, 1 found
};

void send_notify_msg(char* ip, int port, unsigned char status)
{
    static unsigned int seq = 0;
    unsigned char mb[MAXMESG];
    struct sockaddr_in daddr;
    struct st_video_lost_notify msg;
    msg.hdr.len = 0;
    msg.hdr.id = MSG_ID_LDAP_STATUS;
    msg.hdr.seq = seq++;
    msg.hdr.ver = 1;
    msg.hdr.r1 = 0;
    msg.hdr.r2 = 0;
    int snd_fd = udp_create_socket("0.0.0.0", 0);
    FILE* fpn = fopen("/etc/hostname", "r");
    char device_name[256];
    device_name[0] = '\0';

    if (fpn)
    {
        fgets(device_name, 31, fpn);

        //remove possible new line in the end
        if (device_name[strlen(device_name) - 1] == '\n')
            device_name[strlen(device_name) - 1] = '\0';

        fclose(fpn);
    }

    msg.device_name.len = strlen(device_name);
    msg.device_name.val = device_name;
    msg.status = status;
    unsigned char* pos = mb;
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.hdr.id);
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.hdr.seq);
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.hdr.ver);
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.hdr.r1);
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.hdr.r2);
    pos += 4;
    *((unsigned int*)(pos)) = htonl(msg.device_name.len + 4);
    pos += 4;
    memcpy((pos), msg.device_name.val, msg.device_name.len);
    pos += msg.device_name.len;
    *((unsigned char*)(pos)) = msg.status;
    pos += 1;

    msg.hdr.len = pos - mb;
    pos = mb;
    *((unsigned int*)(pos)) = htonl(msg.hdr.len);
    /* Send to SC009. */
    memset(&daddr, 0, sizeof(daddr));
    daddr.sin_family = AF_INET;
    daddr.sin_addr.s_addr = inet_addr(ip);
    daddr.sin_port = htons(port);
    sendto(snd_fd, mb, msg.hdr.len, 0, (struct sockaddr*)&daddr, sizeof(daddr));

    if (snd_fd > 0)
    {
        close(snd_fd);
    }
}

