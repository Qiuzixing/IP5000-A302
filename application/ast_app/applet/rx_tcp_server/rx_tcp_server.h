#ifndef _RX_TCP_SERVER_H_
#define _RX_TCP_SERVER_H_

enum
{
    CLIENT_NOT_CONNECT = -1,
    CLIENT_CONNECT  = 0,
    CLIENT_UNKONW_STATUS
};

typedef enum
{
    WAIT_EDID_REPORT = 0,
    GET_EDID ,
    CMD_CNT ,
    KNOWN_SOCKET_CMD
}socket_cmd;

typedef struct
{
    socket_cmd cmd_index;
    char *cmd_str;
} socket_cmd_struct;

#endif
