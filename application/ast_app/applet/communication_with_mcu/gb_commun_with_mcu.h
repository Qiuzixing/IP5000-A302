#ifndef _GB_COMMUN_WITH_MCU_H_
#define _GB_COMMUN_WITH_MCU_H_
#include "./commun_with_mcu/types.h"
#include "./commun_with_mcu/ipx5000_a30_cmd.h"

#define IPC_CH "@m_lm_get"
#define IPC_RELAY_CH "@m_lm_reply"
#define PARAM_SIZE 256

enum
{
    INVALID = 0,
    VALID = 1
};

enum
{
    GB_SUCCESS = 0,
    GB_FAIL
};

enum
{
    UPGRADE = 0,
    COMMUNICATION
};

typedef enum
{
    SEND_CMD = 0,
    QUERY_CMD,
    UNKNOW_CMD_TYPE = 0xFF
} cmd_type;

typedef enum
{
    // video command
    IPC_EVENT_HDMI_LINK_STATUS = 0,
    IPC_GET_LINK_STATUS,
    IPC_EVENT_HDMI_EDID,
    IPC_GET_EDID,
    IPC_EVENT_HDMI_VIDEO_STATUS,
    IPC_GET_VIDEO_STATUS,
    IPC_VIDEO_CONTROL,
    IPC_SET_VIDEO_MODE,
    IPC_SET_INPUT_SOURCE,
    IPC_GET_INPUT_SOURCE,

    // HDCP command
    IPC_EVENT_HDCP_CAP,
    IPC_GET_HDCP_CAP,
    IPC_SET_HDCP_CAP,
    IPC_EVENT_HDCP_STATUS,
    IPC_GET_HDCP_STATUS,

    //audio command
    IPC_EVENT_HDMI_AUDIO_STATUS,
    IPC_GET_AUDIO_STATUS,
    IPC_HDMI_AUDIO_CONTROL,
    IPC_SET_AUDIO_INSERT_EXTRACT,

    //other command
    IPC_UART_PASSTHROUGH,
    IPC_SET_GPIO_CONFIG,
    IPC_EVENT_GPIO_VAL,
    IPC_GET_GPIO_VAL,
    IPC_SET_GPIO_VAL,

    IPC_CmdCnt,
    IPC_UnknownCmd = 0xFF
} ipc_cmd_typedef;

typedef struct
{
    uint8_t ipc_cmd_name;
    char *ipc_cmd_str;
    uint8_t ipc_cmd_len;
    uint16_t a30_cmd;
    uint8_t type;
} ipc_cmd_struct;

typedef struct
{
    char cmd_param1[PARAM_SIZE];
    char cmd_param2[PARAM_SIZE];
    char cmd_param3[PARAM_SIZE];
} ipc_cmd_param; 
         

#endif
