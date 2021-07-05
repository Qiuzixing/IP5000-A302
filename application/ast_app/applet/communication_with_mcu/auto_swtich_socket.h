#ifndef _AUTO_SWITCH_SOCKET_H_
#define _AUTO_SWITCH_SOCKET_H_

#define MSG_SOCKET    "/tmp/message"

#ifndef MAIN_SOCKET_NAME
#define MAIN_SOCKET_NAME    "/tmp/mainswitch"
#endif


typedef struct 
{
    char *type;
    char *source;
}socket_msg;


int create_unixsocket(const char *name);
int sendEvent(int sock, const char *type, const char *source);
#endif
