#ifndef _GB_DANTE_CMD_PACKET_H_
#define _GB_DANTE_CMD_PACKET_H_

typedef enum
{
    SET_DANTE_HOSTNAME = 0,
    SET_DANTE_DEFAULT_NAME_GET_IT,
    SET_DANTE_REBOOT,
    UNKNOW_DANTE_CMD_TYPE = 0xFF
} dante_cmd;

int dnt_reboot_packet(unsigned char *buf);

#endif
