#ifndef _CREATE_SOCKET_H_
#define _CREATE_SOCKET_H_

#define UNIX_DOMAIN "/tmp/UNIX.domain"

enum
{
    CREAT_ERROR = 0,
    CREAT_FAIL,
    CREAT_SUCCESS
};

int create_local_socket(void);
int send_msg_to_local_socket(unsigned char *buf);
int create_udp_socket_server(void);
#endif
