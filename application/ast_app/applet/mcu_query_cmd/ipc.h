#ifndef _IPC_H_
#define _IPC_H_
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
#include <dirent.h>
#include "types.h"
typedef struct
{
    unsigned char port; // enum HDMIPort
    unsigned char cap;
    char  isConnect; // hdmirx is 5V status, hdmitx is RxSense status.
    char  isHpd; 
    unsigned char rsvd;
    unsigned char rawEdid[256];
    char  isStable;
    char  isMute;
    /* hacitve, vactive and frameRate only valid when isStable is true */
    unsigned short int hactive; 
    unsigned short int vactive;
    unsigned char frameRate;
    unsigned char colorspace;
    unsigned char deepcolor;
    unsigned char rsvd1;
}ipc_relay_msg;

#define MAX_LEN 1024

FILE *lock_ipc_query(char *file_name);
int ipc_set(int ipc_fd, char *value, unsigned int count);
int ipc_get(int ipc_fd,ipc_relay_msg *buffer,int buffer_len);
#endif /* #ifndef _IPC_H_ */
