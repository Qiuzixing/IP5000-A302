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

#include <getopt.h>
#include <netdb.h>       //resolve hostname
#include <string.h>      //memset
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include "ipc.h"
#include "./commun_with_mcu/ymodem.h"
#include "./commun_with_mcu/uart_mcu_cmd.h"
#include "./commun_with_mcu/command.h"
#include "gb_commun_with_mcu.h"
#include "./audio_switch/auto_swtich_socket.h"
#include "./dante_example_code/dante_cmd/dante_cmd_packet.h"
#include "./audio_switch/audio_switch_cmd.h"

#define mymin(a, b) (a > b ? b : a)
#define versionnum_len 15
#define err(fmt, args...) fprintf(stderr, "err: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#define info(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define dbg(fmt, args...) fprintf(stderr, "dbg: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__, ##args)

#define UART_PORT "/dev/ttyS2"
#define UART_COMFIG "115200-8n1"
#define IPC_CMD_TOO_OFTEN_LIMIT 40
#define IP5000_MCU_UPGRADE_PACKAGE  "/usr/share/ip5000_a30/"
#define IPE5000W_MCU_UPGRADE_PACKAGE  "/usr/share/ipe5000w_a30/"

int sock_fd = -1;
int uart_fd = -1;
int errno;
uint8_t file_path[120];
uint8_t file_path_len = 0;
uint8_t arm_version[30] = "GET VER "; //用于记录版本信息
uint8_t version_len = 0;

unsigned char dante_cmd_buff[DANTE_UART_BUFFER] = {0};
int dante_cmd_len = 0;
char *dante_host_name = NULL;
uint8_t Send_File_Flag = 0;
uint8_t Up_Succ_Flag = 0;
uint8_t Up_Fail_Flag = 0;
uint8_t Recv_Cmd_Timeout = 0; //read cmd timeout flag
int ipc_querycmd_index = 0;
uint8_t up_or_commun_flag = UPGRADE;
audio_inout_info_struct audio_inout_info;
uint8_t board_type_flag = IPE5000P;
uint8_t last_hdmi_in_index = 0;
uint8_t auto_av_report_flag = CLOSE_REPROT;
uint8_t current_play_port = HDMIRX1;

const ipc_cmd_struct ipc_cmd_list[] =
    {
        // video command
        {IPC_EVENT_HDMI_LINK_STATUS,    "event_link_status",        sizeof("event_link_status"),    EVENT_HDMI_LINK_STATUS,             SEND_CMD},
        {IPC_GET_LINK_STATUS,           "get_link_status",          sizeof("get_link_status"),      CMD_HDMI_GET_LINK_STATUS,           QUERY_CMD},
        {IPC_EVENT_HDMI_EDID,           "event_edid",               sizeof("event_edid"),           EVENT_HDMI_EDID,                    SEND_CMD},
        {IPC_GET_EDID,                  "get_edid",                 sizeof("get_edid"),             CMD_HDMI_GET_EDID,                  QUERY_CMD},
        {IPC_EVENT_HDMI_VIDEO_STATUS,   "event_video_status",       sizeof("event_video_status"),   EVENT_HDMI_VIDEO_STATUS,            SEND_CMD},
        {IPC_GET_VIDEO_STATUS,          "get_video_status",         sizeof("get_video_status"),     CMD_HDMI_GET_VIDEO_STATUS,          QUERY_CMD},
        {IPC_VIDEO_CONTROL,             "set_video_control",        sizeof("set_video_control"),    CMD_HDMI_VIDEO_CONTROL,             SEND_CMD},
        {IPC_SET_VIDEO_MODE,            "set_video_mode",           sizeof("set_video_mode"),       CMD_HDMI_SET_VIDEO_MODE,            QUERY_CMD},
        {IPC_SET_INPUT_SOURCE,          "set_input_source",         sizeof("set_input_source"),     CMD_HDMI_SET_INPUT_SOURCE,          SEND_CMD},
        {IPC_GET_INPUT_SOURCE,          "get_input_source",         sizeof("get_input_source"),     CMD_HDMI_GET_INPUT_SOURCE,          SEND_CMD},
        // HDCP command     
        {IPC_EVENT_HDCP_CAP,            "event_hdcp_cap",           sizeof("event_hdcp_cap"),       EVENT_HDCP_CAP,                     SEND_CMD},
        {IPC_GET_HDCP_CAP,              "get_hdcp_cap",             sizeof("get_hdcp_cap"),         CMD_HDCP_GET_CAP,                   QUERY_CMD},
        {IPC_SET_HDCP_CAP,              "set_hdcp_cap",             sizeof("set_hdcp_cap"),         CMD_HDCP_SET_CAP,                   SEND_CMD},
        {IPC_EVENT_HDCP_STATUS,         "event_hdcp_status",        sizeof("event_hdcp_status"),    EVENT_HDCP_STATUS,                  SEND_CMD},
        {IPC_GET_HDCP_STATUS,           "get_hdcp_status",          sizeof("get_hdcp_status"),      CMD_HDCP_GET_STATUS,                QUERY_CMD},
        {IPC_GET_HDCP_MODE,             "get_hdcp_mode",            sizeof("get_hdcp_mode"),        CMD_HDCP_GET_MODE,                  QUERY_CMD},
        {IPC_SET_HDCP_MODE,             "set_hdcp_mode",            sizeof("set_hdcp_mode"),        CMD_HDCP_SET_MODE,                  SEND_CMD},
        //audio command     
        {IPC_EVENT_HDMI_AUDIO_STATUS,   "unknown",                  sizeof("unknown"),              EVENT_HDMI_AUDIO_STATUS,            SEND_CMD},
        {IPC_GET_AUDIO_STATUS,          "unknown",                  sizeof("unknown"),              CMD_HDMI_GET_AUDIO_STATUS,          SEND_CMD},
        {IPC_HDMI_AUDIO_CONTROL,        "set_hdmi_mute",            sizeof("set_hdmi_mute"),        CMD_HDMI_AUDIO_CONTROL,             SEND_CMD},
        {IPC_SET_AUDIO_INSERT_EXTRACT,  "set_audio_insert",         sizeof("set_audio_insert"),     CMD_HDMI_SET_AUDIO_INSERT_EXTRACT,  SEND_CMD},
        //other command
        {IPC_UART_PASSTHROUGH,          "set_dante",                sizeof("set_dante"),            CMD_UART_PASSTHROUGH,               SEND_CMD},
        {IPC_SET_GPIO_CONFIG,           "set_gpio_config",          sizeof("set_gpio_config"),      CMD_GPIO_CONFIG,                    SEND_CMD},
        {IPC_EVENT_GPIO_VAL,            "unknown",                  sizeof("unknown"),              EVENT_GPIO_VAL,                     SEND_CMD},
        {IPC_GET_GPIO_VAL,              "get_gpio_val",             sizeof("get_gpio_val"),         CMD_GPIO_GET_VAL,                   QUERY_CMD},
        {IPC_SET_GPIO_VAL,              "set_gpio_val",             sizeof("set_gpio_val"),         CMD_GPIO_SET_VAL,                   SEND_CMD},
        {IPC_OPEN_REPORT,               "open_report",              sizeof("open_report"),          0,                                  SEND_CMD},
        {IPC_LCD_GET_TYPE,              "get_lcd_type",             sizeof("get_lcd_type"),         CMD_LCD_GET_TYPE,                   SEND_CMD},
        {IPC_LCD_CONTROL,               "set_lcd_control",          sizeof("set_lcd_control"),      CMD_LCD_CONTROL,                    SEND_CMD},
        {IPC_LCD_SET_CONTENT,           "set_lcd_content",          sizeof("set_lcd_content"),      CMD_LCD_SET_CONTENT,                SEND_CMD},
        {IPC_SET_LED_CONTROL,           "set_led_control",          sizeof("set_led_control"),      CMD_LED_CONTROL,                    SEND_CMD},
        //audio_autoswitch
        {IPC_AUDIO_IN,                  "audio_in",                 sizeof("audio_in"),             0,                                  SEND_CMD},
        {IPC_AUDIO_OUT,                 "audio_out",                sizeof("audio_out"),            0,                                  SEND_CMD}
    }; 

static int setserial(int s, struct termios *cfg, int speed, int data, unsigned char parity, int stopb)
{
    speed_t speed_cfg;

    cfmakeraw(cfg);

    switch (speed)
    {
    case 50:
        speed_cfg = B50;
        break;
    case 75:
        speed_cfg = B75;
        break;
    case 110:
        speed_cfg = B110;
        break;
    case 134:
        speed_cfg = B134;
        break;
    case 150:
        speed_cfg = B150;
        break;
    case 200:
        speed_cfg = B200;
        break;
    case 300:
        speed_cfg = B300;
        break;
    case 600:
        speed_cfg = B600;
        break;
    case 1200:
        speed_cfg = B1200;
        break;
    case 1800:
        speed_cfg = B1800;
        break;
    case 2400:
        speed_cfg = B2400;
        break;
    case 4800:
        speed_cfg = B4800;
        break;
    case 9600:
        speed_cfg = B9600;
        break;
    case 19200:
        speed_cfg = B19200;
        break;
    case 38400:
        speed_cfg = B38400;
        break;
    case 57600:
        speed_cfg = B57600;
        break;
    case 115200:
        speed_cfg = B115200;
        break;
    case 230400:
        speed_cfg = B230400;
        break;
    default:
        speed_cfg = B115200;
        break;
    }

    cfsetispeed(cfg, speed_cfg);
    cfsetospeed(cfg, speed_cfg);

    switch (parity | 32)
    {
    case 'e':
        cfg->c_cflag |= PARENB;
        cfg->c_cflag &= ~PARODD;
        break;
    case 'o':
        cfg->c_cflag |= PARENB;
        cfg->c_cflag |= PARODD;
        break;
    case 'n':
    default:
        cfg->c_cflag &= ~PARENB;
        break;
    }

    cfg->c_cflag &= ~CSIZE;

    switch (data)
    {
    case 5:
        cfg->c_cflag |= CS5;
        break;
    case 6:
        cfg->c_cflag |= CS6;
        break;
    case 7:
        cfg->c_cflag |= CS7;
        break;
    case 8:
    default:
        cfg->c_cflag |= CS8;
        break;
    }

    if (stopb == 1)
        cfg->c_cflag &= ~CSTOPB;
    else
        cfg->c_cflag |= CSTOPB;

    return tcsetattr(s, TCSANOW, cfg);
}

static int open_uart(char *uart_port, char *uart_config)
{
    int fd;
    int speed, data, stopb;
    unsigned char parity;
    unsigned int n;
    struct termios cfg;
    bzero(&cfg, sizeof(cfg));

    fd = open(uart_port, O_RDWR | O_NDELAY);
    if (fd < 0)
    {
        fprintf(stderr, "open uart error:%s", strerror(errno));
        //err("could not open device %s", uart_port);
        return -1;
    }

    n = sscanf(uart_config, "%d-%d%c%d", &speed, &data, &parity, &stopb);

    if (setserial(fd, &cfg, speed, data, parity, stopb) < 0)
    {
        printf("could not initialize device %s,exit upgrade", uart_port);
        //err("could not initialize device %s", uart_port);
        close(fd);
        return -3;
    }

    return fd;
}

static uint8_t get_version()
{
    uint8_t *p_filename = NULL; //用于记录fw.bin的文件名
    uint8_t *q = NULL;          //工具人
    DIR *dp;
    struct dirent *sdp = NULL;
    uint8_t num = 0; //用于记录bin文件的个数，只要不等于1，就不进行升级的操作
    uint8_t len = 0; //用于记录bin文件名的长度
    uint8_t i, k, j;
    uint8_t V_flag = 0; //用于判断fw文件名是否符合命名规范，也就是文件名中是否含有V
    if(board_type_flag == IPE5000W)
    {
        memcpy(file_path,IPE5000W_MCU_UPGRADE_PACKAGE,strlen(IPE5000W_MCU_UPGRADE_PACKAGE));
    }
    else
    {
        memcpy(file_path,IP5000_MCU_UPGRADE_PACKAGE,strlen(IP5000_MCU_UPGRADE_PACKAGE));
    }
    dp = opendir(file_path);
    if (NULL == dp)
    {
        fprintf(stderr, "opendir error,exit upgrade\n");
        return GB_FAIL;
    }

    while (NULL != (sdp = readdir(dp)))
    {
        if ((0 == strcmp(sdp->d_name, ".")) || (0 == strcmp(sdp->d_name, ".."))) //每个文件默认都有.与.. 过滤掉这两个文件
        {
            continue;
        }
        q = strstr(sdp->d_name, ".bin"); //寻找以.bin为结尾的文件名
        if (NULL == q)
        {
            continue;
        }
        else
        {
            p_filename = sdp->d_name;
            num++;
        }
    }

    if (0 == num)
    {
        fprintf(stderr, "There is no fw.bin file,exit upgrade\n");
        closedir(dp);
        return GB_FAIL;
    }
    else if (num > 1)
    {
        fprintf(stderr, "more than 1 fw.bin file,I don't know which one to choose,exit upgrade\n");
        closedir(dp);
        return GB_FAIL;
    }

    len = strlen(p_filename);
    file_path_len = strlen(file_path);
    for (i = 0; i < len; i++)
    {
        if ('V' == p_filename[i])
        {
            k = i;
            V_flag = 1;
            for (j = 0; k < i + versionnum_len; j++, k++)
            {
                if ((p_filename[k] != 'b') && (p_filename[k + 1] != 'i') && (p_filename[k + 2] != 'n'))
                {
                    arm_version[8 + j] = p_filename[k];
                }
                else
                {
                    break;
                }
            }
        }
        file_path[i + file_path_len] = p_filename[i];
    }
    if (0 == V_flag)
    {
        printf("File name error, version number information not found,exit upgrade\n");
        closedir(dp);
        return GB_FAIL;
    }
    arm_version[7 + j] = '\0'; //因为是以.bin结尾，所以.也在这里面，要把它去掉
    version_len = 7 + j;
    file_path[len + file_path_len] = '\0';
    closedir(dp);
    return GB_SUCCESS;
}

static void do_handle_get_cmd(uint16_t cmd,char *cmd_param)
{
    char *port_num = strtok(cmd_param,":");
    uint8_t port;
    if(port_num != NULL)
    {
        port = atoi(port_num);
    }
    else
    {
        port = HDMITX2;
    }
    APP_Comm_Send(cmd, &port, sizeof(port));
}

static void auto_switch_cec(unsigned char input_source)
{
    uint16_t gpio_cmd = CMD_GPIO_SET_VAL;
    char hdmi1_cec[] = "3:67:1:68:0:69:0"; 
    char hdmi2_cec[] = "3:67:1:68:1:69:0"; 
    char hdmi3_cec[] = "3:67:1:68:0:69:1"; 
    char ipe5000w_hdmi1_cec[] = "1:77:0";
    char ipe5000w_hdmi2_cec[] = "1:77:1";
    switch(input_source)
    {
        case HDMIRX1:
            if(board_type_flag == IPE5000W)
            {
                do_handle_set_gpio_val(gpio_cmd,ipe5000w_hdmi1_cec);
            }
            else
            {
                do_handle_set_gpio_val(gpio_cmd,hdmi1_cec);
            }
            break;
        case HDMIRX2:
            if(board_type_flag == IPE5000W)
            {
                do_handle_set_gpio_val(gpio_cmd,ipe5000w_hdmi2_cec);
            }
            else
            {
                do_handle_set_gpio_val(gpio_cmd,hdmi2_cec);
            }
            break;
        case HDMIRX3:
            do_handle_set_gpio_val(gpio_cmd,hdmi3_cec);
            break;
        default:
            break;
    }
}

static void do_handle_input_source(uint16_t cmd,char *cmd_param)
{
    struct CmdDataInputSorce vdo_source;

    memset((unsigned char *)&vdo_source, 0, sizeof(vdo_source));
    char *tx_port_num = strtok(cmd_param,":");
    char *rx_port_num = strtok(NULL,":");
    if(tx_port_num != NULL && rx_port_num != NULL)
    {
        vdo_source.txPort = atoi(tx_port_num);
        vdo_source.rxPort = atoi(rx_port_num);
        current_play_port = vdo_source.rxPort;
        printf("vdo_source.txPort:%d\nvdo_source.rxPort:%d\n",vdo_source.txPort,vdo_source.rxPort);
        APP_Comm_Send(cmd, (U8*)&vdo_source, sizeof(vdo_source));
        uint16_t gpio_cmd = CMD_GPIO_SET_VAL;
        char select_usb_type_c[] = "1:66:0";
        char select_usb_type_b[] = "1:66:1";
        switch(board_type_flag)
        {
            case IPE5000P:
                //Automatic switching of usb-b or usb-c
                if(vdo_source.rxPort == HDMIRX3)
                {
                    do_handle_set_gpio_val(gpio_cmd,select_usb_type_c);
                }
                else
                {
                    do_handle_set_gpio_val(gpio_cmd,select_usb_type_b);
                }
                auto_switch_cec(vdo_source.rxPort);
                break;
            case IPE5000W:
                auto_switch_cec(vdo_source.rxPort);
                break;
            default:
                break;
        }
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
    }

}

void do_handle_set_hdcp_cap(uint16_t cmd,char *cmd_param)
{
    char *port_num = strtok(cmd_param,":");
    char *cap = strtok(NULL,":");
    struct CmdDataHDCPCap hdcp_cap;
    memset((unsigned char *)&hdcp_cap, 0, sizeof(hdcp_cap));
    if(port_num != NULL)
    {
        hdcp_cap.port = atoi(port_num);
    }
    else
    {
        hdcp_cap.port = HDMITX2;
    }

    if(cap != NULL)
    {
        hdcp_cap.cap = atoi(cap);
    }
    else
    {
        hdcp_cap.cap = 2;
    }
    APP_Comm_Send(cmd, (U8*)&hdcp_cap, sizeof(hdcp_cap));
}

void do_handle_set_hdcp_mode(uint16_t cmd,char *cmd_param)
{
    char *port_num = strtok(cmd_param,":");
    char *mode = strtok(NULL,":");
    struct CmdDataHDCPMode hdcp_mode;
    memset((unsigned char *)&hdcp_mode, 0, sizeof(hdcp_mode));
    if(port_num != NULL)
    {
        hdcp_mode.port = atoi(port_num);
    }
    else
    {
        hdcp_mode.port = HDMIRX1;
    }

    if(mode != NULL)
    {
        hdcp_mode.mode = atoi(mode);
    }
    else
    {
        hdcp_mode.mode = 0;
    }
    APP_Comm_Send(cmd, (U8*)&hdcp_mode, sizeof(hdcp_mode));
}

static void do_handle_set_gpio_config(uint16_t cmd,char *cmd_param)
{
    char *tmp_p = strtok(cmd_param,":");
    uint8_t uctemp = 0;
    if(tmp_p != NULL)
    {
        uctemp = atoi(tmp_p);
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
        return;
    }
    struct CmdDataGpioCfg *gpio_cfg = NULL;
    int i = 0;
    gpio_cfg = (struct CmdDataGpioCfg *)calloc(uctemp*2 + 4, sizeof(uint8_t));
    if (NULL == gpio_cfg)
    {
        printf("Failed to allocate memory\n");
        return;
    }
    gpio_cfg->numOfGpio = uctemp;     
    gpio_cfg->active = 1;
    for(i=0;i<uctemp;i++)
    {
        tmp_p = strtok(NULL,":");
        if(tmp_p != NULL)
        {
            gpio_cfg->gpio[i][0] = atoi(tmp_p);
        }
        else
        {
            printf("warn:Illegal parameter, discard directly\n");
            free(gpio_cfg);
            return;
        }
        
        tmp_p = strtok(NULL,":");
        if(tmp_p != NULL)
        {
            gpio_cfg->gpio[i][1] = atoi(tmp_p);
        }
        else
        {
            printf("warn:Illegal parameter, discard directly\n");
            free(gpio_cfg);
            return;
        }
    }

    APP_Comm_Send(cmd, (U8*)gpio_cfg, uctemp*2 + 4);
    free(gpio_cfg);
}

static void do_handle_get_gpio_val(uint16_t cmd,char *cmd_param)
{
    char *tmp_p = strtok(cmd_param,":");
    uint8_t uctemp = 0;
    if(tmp_p != NULL)
    {
        uctemp = atoi(tmp_p);
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
        return;
    }
    struct CmdDataGpioList *gpio_list= NULL;
    int i = 0;
    gpio_list = (struct CmdDataGpioList *)calloc(uctemp + 2, sizeof(uint8_t));
    if (NULL == gpio_list)
    {
        printf("Failed to allocate memory\n");
        return;
    }
    gpio_list->numOfGpio = uctemp;
 
    for(i=0;i<uctemp;i++)
    {
        tmp_p = strtok(NULL,":");
        if(tmp_p != NULL)
        {
            gpio_list->gpioPin[i] = atoi(tmp_p);
        }
        else
        {
            printf("warn:Illegal parameter, discard directly\n");
            free(gpio_list);
            return;
        }
    }
    APP_Comm_Send(cmd, (U8*)gpio_list, uctemp + 2);
    free(gpio_list);
}

void do_handle_set_gpio_val(uint16_t cmd,char *cmd_param)
{
    char *tmp_p = strtok(cmd_param,":");
    uint8_t uctemp = 0;
    if(tmp_p != NULL)
    {
        uctemp = atoi(tmp_p);
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
        return;
    }
    struct CmdDataGpioVal *gpio_val= NULL;
    int i = 0;
    gpio_val = (struct CmdDataGpioVal *)calloc(uctemp*2 + 2, sizeof(uint8_t));
    if (NULL == gpio_val)
    {
        printf("Failed to allocate memory\n");
        return;
    }
    gpio_val->numOfGpio = uctemp; 
    for(i=0;i<uctemp;i++)
    {
        tmp_p = strtok(NULL,":");
        if(tmp_p != NULL)
        {
            gpio_val->gpio[i][0] = atoi(tmp_p);
        }
        else
        {
            printf("warn:Illegal parameter, discard directly\n");
            free(gpio_val);
            return;
        }
        
        tmp_p = strtok(NULL,":");
        if(tmp_p != NULL)
        {
            gpio_val->gpio[i][1] = atoi(tmp_p);
        }
        else
        {
            printf("warn:Illegal parameter, discard directly\n");
            free(gpio_val);
            return;
        }

        if(gpio_val->gpio[i][0] == 70)  //70 Indicates setting analog input or output
        {
            if(gpio_val->gpio[i][1] == 1)   //line_in
            {
                mute_control(ANALOG_IN_MUTE,UNMUTE);
            }
            else                            //line_out
            {
                mute_control(ANALOG_IN_MUTE,MUTE);
            }
        }
    }
    APP_Comm_Send(cmd, (U8*)gpio_val, uctemp*2 + 2);
    free(gpio_val);
}

void do_handle_get_lcd_type(uint16_t cmd,char *cmd_param)
{
    uint32_t NullValue = 0;
    APP_Comm_Send(cmd, (U8*)(&NullValue),4);
}

void do_handle_set_lcd_control(uint16_t cmd,char *cmd_param)
{
    struct CmdDataLCDControl lcd_control;
    char *lcd_type = strtok(cmd_param,":");
    char *enable = strtok(NULL,":");
    if(lcd_type != NULL && enable != NULL)
    {
        lcd_control.type = atoi(lcd_type);
        lcd_control.enable = atoi(enable);
    }
    APP_Comm_Send(cmd, (U8*)(&lcd_control), sizeof(struct CmdDataLCDControl));
}

void do_handle_set_lcd_content(uint16_t cmd,char *cmd_param)
{
    struct CmdDataLCDContect *lcd_contect;
    char *display_num = strtok(cmd_param,":");
    if(display_num != NULL)
    {
        lcd_contect = (struct CmdDataLCDContect *)calloc(strlen(display_num)+sizeof(struct CmdDataLCDContect), sizeof(uint8_t));
        lcd_contect->dataLength = strlen(display_num);
        strcpy((char*)&(lcd_contect->data),(const char*)(display_num));
        lcd_contect->type = LCD_TYPE_LED;
        APP_Comm_Send(CMD_LCD_SET_CONTENT, (U8*)lcd_contect, sizeof(struct CmdDataLCDContect)+strlen(display_num));
        free(lcd_contect);
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
    }
}

void do_handle_set_led_control(uint16_t cmd,char *cmd_param)
{
    struct CmdDataLEDControl LEDControl;
    char *led = strtok(cmd_param,":");
    char *led_mode = strtok(NULL,":");
    if(led != NULL && led_mode != NULL)
    {
        LEDControl.led = atoi(led);
        LEDControl.mode = atoi(led_mode);
        if(LED_MODE_FLASH == LEDControl.mode)
        {
            char *delay_on = strtok(NULL,":");
            char *delay_off = strtok(NULL,":");
            LEDControl.onTime = atoi(delay_on);
            LEDControl.offTime = atoi(delay_off);
        }
    }
    APP_Comm_Send(cmd, (U8*)(&LEDControl), sizeof(struct CmdDataLEDControl));
}

void do_handle_set_audio_insert_extract(uint16_t cmd,char *cmd_param)
{
    struct CmdDataAudioInsertAndExtract ado_insert;
    char *from_port = strtok(cmd_param,":");
    char *to_port = strtok(NULL,":");
    if(from_port != NULL && to_port != NULL)
    {
        ado_insert.fromPort = atoi(from_port);
        ado_insert.toPort = atoi(to_port);
    }
    printf("cmd[0x%x] audio fromport[0x%x] toport[0x%x]\n", cmd, ado_insert.fromPort, ado_insert.toPort);
    ado_insert.ttlMode = TTLMODE_I2S;
    ado_insert.audioCoding = AUDIO_CODING_LPCM;
    ado_insert.channels = AUDIO_CH_2;
    ado_insert.i2sFormat = I2S_FORMAT_NORMAL;
    ado_insert.mclkRatio = MCLK_RATIO_256FS;
    ado_insert.sampleDepth = AUDIO_SF_48KHZ;
    APP_Comm_Send(cmd, (U8*)&ado_insert, sizeof(ado_insert));
}

static void do_handle_uart_pass(uint16_t cmd,char *cmd_param)
{
    char *dante_cmd = strtok(cmd_param,":");
    int cmd_index = 0;
    int i = 0;
    struct CmdDataUartPassthrough *uart_pass = NULL;
    if(dante_cmd != NULL)
    {
        cmd_index = atoi(dante_cmd);
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
        return;
    }
    switch(cmd_index)
    {
        case SET_DANTE_HOSTNAME:
            dante_host_name = strtok(NULL,":");
            if(dante_host_name == NULL)
            {
                printf("warn:Illegal parameter, discard directly\n");
                return;
            }
            break;
        case SET_DANTE_DEFAULT_NAME_GET_IT:
            dante_host_name = NULL;
            break;
        default:
            break;
    }

    if(-1 == __example_main(cmd_index))
    {
        return;
    }

    uart_pass = (struct CmdDataUartPassthrough *)calloc(dante_cmd_len + 4, sizeof(uint8_t));
    if (NULL == uart_pass)
    {
        printf("Failed to allocate memory\n");
        return;
    }
    uart_pass->fromPort = UART_PORT_2;
    uart_pass->toPort = UART_PORT_4;
    uart_pass->dataLength = dante_cmd_len;
    for(i=0;i<dante_cmd_len;i++)
    {
        uart_pass->data[i] = dante_cmd_buff[i];
        printf("uart_pass->data[%d] = 0x%x\n",i,uart_pass->data[i]);
    }
    APP_Comm_Send(CMD_UART_PASSTHROUGH, (U8*)uart_pass, dante_cmd_len + 4);
    free(uart_pass);
}

void do_handle_set_hdmi_mute(uint16_t cmd,char *cmd_param)
{
    struct CmdDataAudioControl ado_mode;
    memset((unsigned char *)&ado_mode, 0, sizeof(ado_mode));
    char *port = strtok(cmd_param,":");
    char *enable = strtok(NULL,":");
    char *mute = strtok(NULL,":");
    if((port != NULL) && (enable != NULL) && (mute != NULL))
    {
        ado_mode.port = atoi(port);
        ado_mode.enable = atoi(enable);
        ado_mode.mute = atoi(mute);
        APP_Comm_Send(cmd, (U8*)&ado_mode, sizeof(ado_mode));
    }
    else
    {
        printf("warn:Illegal parameter, discard directly\n");
    }
}

static void handle_audio()
{
    if(board_type_flag == IPE5000 || board_type_flag == IPE5000W)
    {
        ipe5000_and_ipe5000w_autoaudio_control();
        return;
    }

    mute_control(ANALOG_OUT_MUTE,MUTE);
    mute_control(DANTE_MUTE,MUTE);
    set_io_select(HDMI);

    all_switch_set_high();
    mDelay(300);
    audio_switch();
}

static void do_handle_audio_in(char *cmd_param)
{
    char *audio_in_type = strtok(cmd_param,":");
    int i = 0;
    int ret = 0;
    if(auto_av_report_flag == OPEN_REPROT && audio_in_type != NULL)
    {
        audio_inout_info.audio_in = atoi(audio_in_type);
        handle_audio();
    }
}

static void do_handle_audio_out(char *cmd_param)
{
    char *audio_out_type = strtok(cmd_param,":");
    int i = 0;
    int ret = 0;
    while(audio_out_type != NULL)
    {
        audio_inout_info.audio_out[i] = atoi(audio_out_type);
        i++;
        if( i == AUDIO_OUT_TYPE_NUM)
        {
            break;
        }
        audio_out_type = strtok(NULL,":");
    }
    audio_inout_info.audio_out[i] = AUDIO_OUT_NULL;
    if(auto_av_report_flag == OPEN_REPROT && audio_inout_info.audio_in != AUDIO_IN_NULL)
    {
        //handle_audio();
    }
}

static void do_handle_ipc_cmd(int index,char *cmd_param)
{
    uint32_t uctemp = CMD_NULL_DATA;

    switch (ipc_cmd_list[index].ipc_cmd_name)
    {
    case IPC_EVENT_HDMI_LINK_STATUS:
    case IPC_EVENT_HDMI_EDID:
    case IPC_EVENT_HDMI_VIDEO_STATUS:
    case IPC_EVENT_HDCP_STATUS:
    case IPC_EVENT_HDCP_CAP:
        APP_Comm_Send(ipc_cmd_list[index].a30_cmd, &uctemp, 4);
        break;

    case IPC_GET_LINK_STATUS:
    case IPC_GET_EDID:
    case IPC_GET_VIDEO_STATUS:
    case IPC_GET_HDCP_CAP:
    case IPC_GET_HDCP_STATUS:
        do_handle_get_cmd(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;

    case IPC_VIDEO_CONTROL:
        break;
    case IPC_SET_VIDEO_MODE:
        break;
    case IPC_SET_INPUT_SOURCE:
    case IPC_GET_INPUT_SOURCE:
        do_handle_input_source(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    
    case IPC_SET_HDCP_CAP:
        do_handle_set_hdcp_cap(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_SET_HDCP_MODE:
        do_handle_set_hdcp_mode(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;

    // audio command
    case IPC_EVENT_HDMI_AUDIO_STATUS:
        break;
    case IPC_GET_AUDIO_STATUS:
        break;
    case IPC_HDMI_AUDIO_CONTROL:
        do_handle_set_hdmi_mute(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_SET_AUDIO_INSERT_EXTRACT:
        do_handle_set_audio_insert_extract(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    
    // other command
    case IPC_UART_PASSTHROUGH:
        do_handle_uart_pass(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_SET_GPIO_CONFIG:
        do_handle_set_gpio_config(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_EVENT_GPIO_VAL:
        break;
    case IPC_GET_GPIO_VAL:
        do_handle_get_gpio_val(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_SET_GPIO_VAL:
        do_handle_set_gpio_val(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_OPEN_REPORT:
        auto_av_report_flag = OPEN_REPROT;
        break;
    case IPC_LCD_GET_TYPE:
        do_handle_get_lcd_type(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_LCD_CONTROL:
        do_handle_set_lcd_control(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_LCD_SET_CONTENT:
        do_handle_set_lcd_content(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    case IPC_SET_LED_CONTROL:
        do_handle_set_led_control(ipc_cmd_list[index].a30_cmd,cmd_param);
        break;
    //audio_autoswitch
    case IPC_AUDIO_IN:
        do_handle_audio_in(cmd_param);
        break;
    case IPC_AUDIO_OUT:
        do_handle_audio_out(cmd_param);
        break;
    default:
        break;
    }
}

static char *get_ipc_cmd_index(char *handle_buffer,char *ipc_cmd_index)
{
    int i = 0;
    int ret = 0;
    char *ipc_param = NULL;
    char *ipc_cmd = strtok_r(handle_buffer,":",&ipc_param);

    for (i = 0; i < IPC_CmdCnt; i++)
    {
        ret = strcmp(ipc_cmd_list[i].ipc_cmd_str, ipc_cmd);
        if (ret == 0)
        {   
            if(ipc_cmd_list[i].type == QUERY_CMD)
            {
                ipc_querycmd_index = i;
            }
            *ipc_cmd_index = i;
            break;
        }
    }
    if(i == IPC_CmdCnt)
    {
        *ipc_cmd_index = IPC_UnknownCmd;
    }
    return ipc_param;
}

static int check_mcu_version(void)
{
    if (GB_FAIL == get_version()) //得到fw文件的文件名与提取文件名中的版本号，且文件名中V后面必须跟版本号，然后以.bin结束，如ch6001V1.0.0.bin
    {
        return GB_FAIL;
    }
    arm_send_cmd(CMD_UPDATE_GET_MCU_STATUS);
    while(1)
    {
        Example_thread_Handler();
        if (Send_File_Flag && get_cmd_ack())
        {
            Send_File_Flag = 0;
            ymodem_send_recv_file();
        }
        if (Up_Succ_Flag && get_cmd_ack())
        {
            printf("upg Succ\n");
            break;
        }
        if (Up_Fail_Flag || Recv_Cmd_Timeout)
        {
            printf("upg Fail\n");
            return GB_FAIL;
        }
    }
    return GB_SUCCESS;

}

int time_difference_from_last_time(struct timespec* last_time)
{
    struct timespec cur_time;
    clock_gettime(CLOCK_MONOTONIC,&cur_time);
    int time_diff = cur_time.tv_sec - last_time->tv_sec;
    return time_diff;
}

static void set_audio_inout_default()
{
    audio_inout_info.audio_in = AUDIO_IN_NULL;
    memset(audio_inout_info.audio_out,AUDIO_OUT_NULL,sizeof(audio_inout_info.audio_out));
}

static void mkdir_ready_file(void)
{
    FILE *fp = NULL;
    if((fp = fopen("/tmp/socket_ready", "w+")) == NULL)
    {
        printf("creat /tmp/socket_ready fail\n");
        return;
    }
    fclose(fp);
}

void print_usage() {
	/* TODO */
	printf("Usage: communication_with_mcu -u/-c [-b]\n");
}

int main(int argc, char *argv[])
{
    int ret;
    char ipc_cmd_index = 0;
    char *ipc_cmd_param = NULL;
    char read_buffer[MAX_LEN] = {0};
    int opt = 0;
    int long_index =0;
    static struct option long_options[] = {
        {"upgrade",                     no_argument,       0,  'u' },
		{"communication",               no_argument,       0,  'c' },
        {"board_type",                    no_argument,       0,  'b' },
		{0,                             0,                 0,  0   }
	};
    while ((opt = getopt_long_only(argc, argv, "ucb:", long_options, &long_index )) != -1) {
		switch (opt) {
		case 'u':
            up_or_commun_flag = UPGRADE;
			break;
		case 'c':
            up_or_commun_flag = COMMUNICATION;
			break;
        case 'b':
            board_type_flag = (unsigned char)strtoul(optarg, NULL, 0);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

    uart_fd = open_uart(UART_PORT, UART_COMFIG);
    if (uart_fd < 0)
    {
        close(uart_fd);
        return 0;
    }

    Example_CommandInterface_Init();

    if( UPGRADE == up_or_commun_flag )
    {
        ret = check_mcu_version();
        close(uart_fd);
        if(GB_SUCCESS == ret)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    fd_set rset;
    int maxfd = 0;
    int ipc_fd = -1;
    int mkdir_ready_flag = 0;
    struct timeval timeout;
    struct timespec last_time;
    timeout.tv_sec = 0;
    sock_fd = create_unixsocket(MSG_SOCKET);
    if (sock_fd == -1) {
        perror("unix socker error.");
        return -1;
    }
    set_audio_inout_default();
    arm_send_cmd(CMD_CLEAR_RECORD_FLAG);
    clock_gettime(CLOCK_MONOTONIC,&last_time);
    while(1)
    {
        if(ipc_fd < 0)
        {
            ipc_fd = create_uds_client(IPC_CH);
            if(ipc_fd < 0)
            {
                info("ipc_fd() failed [%d:%s]\n", errno, strerror(errno));
                return -1;
            }
            if(mkdir_ready_flag == 0)
            {
                mkdir_ready_file();
                mkdir_ready_flag = 1;
            }
        }
        timeout.tv_usec = 200000;
        FD_ZERO(&rset);
        FD_SET(ipc_fd, &rset);
        FD_SET(uart_fd, &rset);
        
        maxfd = ipc_fd > uart_fd?ipc_fd+1:uart_fd+1;
        ret = select(maxfd,&rset,NULL,NULL,&timeout);
        switch(ret)
        {
            case -1: 
                printf("select error,quit\n");
                return;
            case 0: //time out
                Example_thread_Handler();
                clock_gettime(CLOCK_MONOTONIC,&last_time);
                break;
            default:
                if(FD_ISSET(ipc_fd,&rset))  
                {
                    ret = read(ipc_fd, read_buffer, MAX_LEN);
                    if (ret < 0) {
                        info("ipc_get() failed [%d:%s]\n", errno, strerror(errno));
                        close(ipc_fd);
                        return -1;
                    }
                    close(ipc_fd);
                    ipc_fd = -1;
                    ipc_cmd_param = get_ipc_cmd_index(read_buffer,&ipc_cmd_index);
                    if(ipc_cmd_index != IPC_UnknownCmd)
                    {
                        do_handle_ipc_cmd(ipc_cmd_index,ipc_cmd_param);
                    } 
                    memset(read_buffer, 0, MAX_LEN); 
                }

                if(FD_ISSET(uart_fd,&rset))
                {
                    Example_thread_Handler();
                    clock_gettime(CLOCK_MONOTONIC,&last_time);
                }
                break;
        }

        if(time_difference_from_last_time(&last_time) > 1)    //Prevent IPC_CMD from calling less than 200ms once
        {
            Example_thread_Handler();
            clock_gettime(CLOCK_MONOTONIC,&last_time);
        } 
    }
    return 0;
}
