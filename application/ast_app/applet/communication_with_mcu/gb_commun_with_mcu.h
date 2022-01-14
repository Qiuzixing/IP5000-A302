#ifndef _GB_COMMUN_WITH_MCU_H_
#define _GB_COMMUN_WITH_MCU_H_
#include "./commun_with_mcu/types.h"
#include "./commun_with_mcu/ipx5000_a30_cmd.h"

#define IPC_CH "@m_lm_get"
#define IPC_RELAY_CH "@m_lm_reply"
#define AUDIO_OUT_TYPE_NUM 4

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

enum
{
    IPE5000 = 0,
    IPE5000P,
    IPD5000,
    IPD5000W,
    IPE5000W,
    UNKNOW_BOARD
};

enum
{
    DANTE_AUDIO_DETECTED = 0,
    DANTE_AUDIO_DISCONNECT ,
    UNKNOW_DANTE_STATUS = 0xff
};

enum
{
    OPEN_REPROT = 0,
    CLOSE_REPROT ,
};

typedef enum
{
    SEND_CMD = 0,
    QUERY_CMD,
    UNKNOW_CMD_TYPE = 0xFF
} cmd_type;

typedef enum
{
    AUDIO_IN_DANTE = 0,
    AUDIO_IN_ANALOG ,
    AUDIO_IN_HDMI,
    AUDIO_IN_NONE,

    AUDIO_IN_NULL = 0xff
}audio_in_type;

typedef enum
{
    AUDIO_OUT_DANTE = 0,
    AUDIO_OUT_ANALOG ,
    AUDIO_OUT_HDMI,
    AUDIO_OUT_LAN,

    AUDIO_OUT_NULL = 0xff
}audio_out_type;

typedef struct  {
    uint8_t audio_in;
    uint8_t audio_out[AUDIO_OUT_TYPE_NUM + 1];  //The last value must be AUDIO_OUT_NULL;
}audio_inout_info_struct;

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
    IPC_GET_HDCP_MODE,
    IPC_SET_HDCP_MODE,

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
    IPC_OPEN_REPORT,
    IPC_LCD_GET_TYPE,
    IPC_LCD_CONTROL,
    IPC_LCD_SET_CONTENT,
    IPC_SET_LED_CONTROL,

    //audio_autoswitch
    IPC_AUDIO_IN,
    IPC_AUDIO_OUT,

    //cec command
    IPC_CEC_SEND,
    IPC_CEC_CMD_REPORT,
    IPC_POWERUP_CEC_REPORT,
    
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

typedef enum
{
    USB_B = 0,
    USB_C ,
    USB_Unknown ,
}usb_types;

typedef struct
{
    usb_types current_chose_usb_type;
} audo_switch_usb_struct;

static void do_handle_get_gpio_val(uint16_t cmd,char *cmd_param);
void do_handle_set_gpio_val(uint16_t cmd,char *cmd_param);
void do_handle_set_audio_insert_extract(uint16_t cmd,char *cmd_param);
void do_handle_set_hdcp_cap(uint16_t cmd,char *cmd_param);
void do_handle_set_led_control(uint16_t cmd,char *cmd_param);
#define DANTE_UART_BUFFER 256

#endif
