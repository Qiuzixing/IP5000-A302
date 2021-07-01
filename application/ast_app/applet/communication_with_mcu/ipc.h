#ifndef _IPC_H_
#define _IPC_H_
#include "./commun_with_mcu/types.h"
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

int ipc_get(char *ipc_ch,char *buffer);
int ipc_set(char *ipc_ch, ipc_relay_msg *value, unsigned int count);
#endif /* #ifndef _IPC_H_ */
