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

#define mymin(a, b) (a > b ? b : a)
#define versionnum_len 15
#define err(fmt, args...) fprintf(stderr, "err: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__, ##args)
#define info(fmt, args...) fprintf(stderr, fmt "\n", ##args)
#define dbg(fmt, args...) fprintf(stderr, "dbg: %13s:%4d (%-12s) " fmt "\n", __FILE__, __LINE__, __func__, ##args)

#define UART_PORT "/dev/ttyS2"
#define UART_COMFIG "115200-8n1"
#define IPC_CMD_TOO_OFTEN_LIMIT 40


int uart_fd = -1;
int errno;
uint8_t file_path[120] = "/usr/share/ch6001/";
uint8_t file_path_len = 0;
uint8_t arm_version[30] = "GET VER "; //用于记录版本信息
uint8_t version_len = 0;

uint8_t Send_File_Flag = 0;
uint8_t Up_Succ_Flag = 0;
uint8_t Up_Fail_Flag = 0;
uint8_t Recv_Cmd_Timeout = 0; //read cmd timeout flag
int ipc_querycmd_index = 0;
uint8_t up_or_commun_flag = UPGRADE;


const ipc_cmd_struct ipc_cmd_list[] =
    {
        // video command
        {IPC_EVENT_HDMI_LINK_STATUS,    "event_link_status",        sizeof("event_link_status"),    EVENT_HDMI_LINK_STATUS,             SEND_CMD},
        {IPC_GET_LINK_STATUS,           "get_link_status",          sizeof("get_link_status"),      CMD_HDMI_GET_LINK_STATUS,           QUERY_CMD},
        {IPC_EVENT_HDMI_EDID,           "event_edid",               sizeof("event_edid"),           EVENT_HDMI_EDID,                    SEND_CMD},
        {IPC_GET_EDID,                  "get_edid",                 sizeof("get_edid"),             CMD_HDMI_GET_EDID,                  QUERY_CMD},
        {IPC_EVENT_HDMI_VIDEO_STATUS,   "event_video_status",       sizeof("event_video_status"),   EVENT_HDMI_VIDEO_STATUS,            SEND_CMD},
        {IPC_GET_VIDEO_STATUS,          "get_video_status",         sizeof("get_video_status"),     CMD_HDMI_GET_VIDEO_STATUS,          QUERY_CMD,},
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
        //audio command     
        {IPC_EVENT_HDMI_AUDIO_STATUS,   "unknown",                  sizeof("unknown"),              EVENT_HDMI_AUDIO_STATUS,            SEND_CMD},
        {IPC_GET_AUDIO_STATUS,          "unknown",                  sizeof("unknown"),              CMD_HDMI_GET_AUDIO_STATUS,          SEND_CMD},
        {IPC_HDMI_AUDIO_CONTROL,        "unknown",                  sizeof("unknown"),              CMD_HDMI_AUDIO_CONTROL,             SEND_CMD},
        {IPC_SET_AUDIO_INSERT_EXTRACT,  "unknown",                  sizeof("unknown"),              CMD_HDMI_SET_AUDIO_INSERT_EXTRACT,  SEND_CMD}
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

static void do_handle_get_cmd(uint16_t cmd,char *port_num)
{
    uint8_t port;
    if(port_num[0] != 0)
    {
        port = atoi(port_num);
    }
    else
    {
        port = HDMITX2;
    }
    APP_Comm_Send(cmd, &port, sizeof(port));
}

static void do_handle_input_source(uint16_t cmd,char *tx_port_num,char *rx_port_num)
{
    struct CmdDataInputSorce vdo_source;
    memset((unsigned char *)&vdo_source, 0, sizeof(vdo_source));
    if(tx_port_num[0] != 0)
    {
        vdo_source.txPort = atoi(tx_port_num);
    }
    else
    {
        vdo_source.txPort = HDMITX2;
    }

    if(rx_port_num[0] != 0)
    {
        vdo_source.rxPort = atoi(rx_port_num);
    }
    else
    {
        vdo_source.rxPort = HDMITX2;
    }
    APP_Comm_Send(cmd, (U8*)&vdo_source, sizeof(vdo_source));
}

static void do_handle_set_hdcp_cap(uint16_t cmd,char *port_num,char *cap)
{
    struct CmdDataHDCPCap hdcp_cap;
    memset((unsigned char *)&hdcp_cap, 0, sizeof(hdcp_cap));
    if(port_num[0] != 0)
    {
        hdcp_cap.port = atoi(port_num);
    }
    else
    {
        hdcp_cap.port = HDMITX2;
    }

    if(cap[0] != 0)
    {
        hdcp_cap.cap = atoi(cap);
    }
    else
    {
        hdcp_cap.cap = 2;
    }
    APP_Comm_Send(cmd, (U8*)&hdcp_cap, sizeof(hdcp_cap));
}

static void do_handle_ipc_cmd(int index,ipc_cmd_param cmd_param)
{
    struct CmdDataEDID edid_data;
    // 2. hdcp
    
    struct CmdDataHDCPStatus hdcp_status;
    struct CmdDataHDCPMode hdcp_mode;
    // 3. video
    struct CmdDataLinkStatus vdo_link;
    struct CmdDataVideoStatus vdo_status;
    struct CmdDataVideoMode vdo_mode;
    
    // 4. audio
    struct CmdDataAudioInsertAndExtract ado_insert;
    struct CmdDataAudioControl ado_mode;
    struct CmdDataAudioStatus ado_status;
    struct CmdDataGpioCfg *gpio_cfg = NULL;
    struct CmdDataGpioVal *gpio_val= NULL;
    struct CmdDataGpioList *gpio_list= NULL;
    struct CmdDataUartPassthrough *uart_pass = NULL;
    uint32_t uctemp = CMD_NULL_DATA;

    ipc_relay_msg ipc_msg;
    memset(&ipc_msg, 0, sizeof(ipc_msg));
    if(0 == Example_HeartBeatStatue())
    {
        printf("serial is Disconnect\n");
        if(ipc_cmd_list[index].type == QUERY_CMD)
        {
            ipc_set(IPC_RELAY_CH,&ipc_msg,sizeof(ipc_msg));
        }
        return;
    }
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
        do_handle_get_cmd(ipc_cmd_list[index].a30_cmd,cmd_param.cmd_param1);
        break;

    case IPC_VIDEO_CONTROL:
        break;
    case IPC_SET_VIDEO_MODE:
        break;
    case IPC_SET_INPUT_SOURCE:
    case IPC_GET_INPUT_SOURCE:
        do_handle_input_source(ipc_cmd_list[index].a30_cmd,cmd_param.cmd_param1,cmd_param.cmd_param2);
        break;
    
    case IPC_SET_HDCP_CAP:
        do_handle_set_hdcp_cap(ipc_cmd_list[index].a30_cmd,cmd_param.cmd_param1,cmd_param.cmd_param2);
        break;

    // audio command
    case IPC_EVENT_HDMI_AUDIO_STATUS:
        break;
    case IPC_GET_AUDIO_STATUS:
        break;
    case IPC_HDMI_AUDIO_CONTROL:
        break;
    case IPC_SET_AUDIO_INSERT_EXTRACT:
        break;
    default:
        break;
    }

}


static void parse_param(char *param_list,ipc_cmd_param *cmd_param)
{
    int i = 0;
    int num = 0;
    memset(&cmd_param,0,sizeof(cmd_param));
    if(NULL != param_list)
    {
        num = strlen(param_list);
        if(num > PARAM_SIZE)
        {
            return;
        }
        for(i = 0;i < num;i++)
        {
            cmd_param->cmd_param1[i] = param_list[i];
        }
    }
    else
    {
        return;
    }
    
    param_list = strtok(NULL,":");
    if(NULL != param_list)
    {
        num = strlen(param_list);
        if(num > PARAM_SIZE)
        {
            return;
        }
        for(i = 0;i < num;i++)
        {
            cmd_param->cmd_param2[i] = param_list[i];
        }
    }
    else
    {
        return;
    }

    param_list = strtok(NULL,":");
    if(NULL != param_list)
    {
        num = strlen(param_list);
        if(num > PARAM_SIZE)
        {
            return;
        }
        for(i = 0;i < num;i++)
        {
            cmd_param->cmd_param3[i] = param_list[i];
        }
    }
    else
    {
        return;
    }

}

static int do_handle_buffer(char *handle_buffer,ipc_cmd_param *cmd_param)
{
    int i = 0;
    int ret = 0;
    char *ipc_cmd = strtok(handle_buffer,":");

    parse_param(strtok(NULL,":"),cmd_param);

    for (i = 0; i < IPC_CmdCnt; i++)
    {
        ret = strcmp(ipc_cmd_list[i].ipc_cmd_str, ipc_cmd);
        if (ret == 0)
        {   
            if(ipc_cmd_list[i].type == QUERY_CMD)
            {
                ipc_querycmd_index = i;
            }
            return i;
        }
    }
    return IPC_UnknownCmd;
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

void print_usage() {
	/* TODO */
	printf("Usage: communication_with_mcu -u/-c\n");
}

int main(int argc, char *argv[])
{
    int ret;
	ipc_cmd_param cmd_param;
    int ipc_cmd_index = 0;
    unsigned char read_buffer[MAX_LEN] = {0};
    int opt = 0;
    int long_index =0;
    static struct option long_options[] = {
        {"upgrade",                     no_argument,       0,  'u' },
		{"communication",               no_argument,       0,  'c' },
		{0,                             0,                 0,  0   }
	};

    while ((opt = getopt_long_only(argc, argv, "", long_options, &long_index )) != -1) {
		switch (opt) {
		case 'u': 
            up_or_commun_flag = UPGRADE;
			break;
		case 'c': 
            up_or_commun_flag = COMMUNICATION; 
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
    struct timeval timeout;
    struct timespec last_time;
    timeout.tv_sec = 0;
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
                    ipc_cmd_index = do_handle_buffer(read_buffer,&cmd_param);
                    if(ipc_cmd_index != IPC_UnknownCmd)
                    {
                        do_handle_ipc_cmd(ipc_cmd_index,cmd_param);
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
