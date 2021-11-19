
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tcp_client.h"
#include "send_p3k_cmd.h"

static const char* ResolutionList[] = {
    "NO SIGNAL",
    "640X480P60",
    "720X480P60",
    "720X480P60",
    "1280X720P60",
    "1920X1080I60",
    "720X480I60",
    "720X480I60",
    "720X240P60",
    "720X240P60",
    "2880X480I60",
    "2880X480I60",
    "2880X240P60",
    "2880X240P60",
    "1440X480P60",
    "1440X480P60",
    "1920X1080P60",
    "720X576P50",
    "720X576P50",
    "1280X720P50",
    "1920X1080I50",
    "720X576I50",
    "720X576I50",
    "720X288P50",
    "720X288P50",
    "2880X576I50",
    "2880X576I50",
    "2880X288P50",
    "2880X288P50",
    "1440X576P50",
    "1440X576P50",
    "1920X1080P50",
    "1920X1080P24",
    "1920X1080P25",
    "1920X1080P30",
    "2880X480P60",
    "2880X480P60",
    "2880X576P50",
    "2880X576P50",
    "1920X1080I50",
    "1920X1080I100",
    "1280X720P100",
    "720X576P100",
    "720X576P100",
    "720X576I100",
    "720X576I100",
    "1920X1080I120",
    "1280X720P120",
    "720X480P120",
    "720X480P120",
    "720X480I120",
    "720X480I120",
    "720X576P200",
    "720X576P200",
    "720X576I200",
    "720X576I200",
    "720X480P240",
    "720X480P240",
    "720X480I240",
    "720X480I240",
    "1280X720P24",
    "1280X720P25",
    "1280X720P30",
    "1920X1080P120",
    "1920X1080P100",
    "800X600P60",
    "1024X768P60",
    "1280X768P60",
    "1280X1024P60",
    "1600X1200P60",
    "1680X1050P60",
    "1920X1200P60",
    "3840X2160P24",
    "3840X2160P25",
    "3840X2160P30",
    "3840X2160P50",
    "3840X2160P60",
};

//#define FIRMWARE_INFO_FILE    "/etc/board_info.json"
#define VIDEO_INFO_FILE         "/data/configs/kds-7/channel/channel_map.json"

//#define FIRMWARE_SYMBOL           "version\": "

// IP GET
#define     IP_SET_CMD          "#NET-CONFIG"
#define     IP_GET_CMD          "#NET-CONFIG?"  
#define     IP_RECV_HEAD        "@NET-CONFIG"

// SET DHCP
#define     DHCP_SET_CMD        "#NET-DHCP"
#define     DHCP_GET_CMD        "#NET-DHCP?"
#define     DHCP_RECV_HEAD      "@NET-DHCP"

//EDID
//#define   EDID_GET_CMD        "#EDID-ACTIVE? 1"
#define     EDID_LIST_GET_CMD   "#EDID-LIST?"
#define     EDID_LIST_HEAD      "@EDID-LIST "
#define     EDID_SET_CMD_1      "#EDID-MODE 1,"
#define     EDID_SET_CMD_2      "#EDID-ACTIVE 1,"
#define     EDID_GET_CMD        "#EDID-MODE? 1"
#define     EDID_RECV_HEAD_1    "@EDID-MODE 1,"
#define     EDID_RECV_HEAD_2    "@EDID-ACTIVE "

//hdcp   0-OFF  1-ON
#define     HDCP_GET_CMD            "#HDCP-STAT? 0,1"
#define     HDCP_RECV_HEAD          "@HDCP-STAT 0,1,"

#define     HDCP_MODE_SET_CMD       "#HDCP-MOD 1,"
#define     HDCP_MODE_GET_CMD       "#HDCP-MOD? 1"
#define     HDCP_MODE_RECV_HEAD     "@HDCPMOD 1,"



#define     DEVICE_STATUS_GET       "#DEV-STATUS?"
#define     DEVICE_RECV_HEAD        "@DEV-STATUS "

// encode
#define     EN_INPUT_SET_HDMI1_CMD  "#X-ROUTE out.hdmi.1.video.1,in.hdmi.1.video.1"
#define     EN_INPUT_SET_HDMI2_CMD  "#X-ROUTE out.hdmi.1.video.1,in.hdmi.2.video.1"
#define     EN_INPUT_SET_USB3_CMD   "#X-ROUTE out.hdmi.1.video.1,in.usb_c.3.video.1" 
#define     EN_INPUT_GET_CMD        "#X-ROUTE? out.hdmi.1.video.1"
#define     EN_INPUT_RECV_HEAD      "@X-ROUTE out.hdmi.1.video.1,in."

// decode input type
#define     DE_INPUT_GET_CMD        "#X-ROUTE? out.hdmi.1.video.1"
#define     DE_INPUT_RECV_HEAD      "@X-ROUTE out.hdmi.1.video.1,in."
#define     DE_SET_INPUT_HDMI_CMD   "#X-ROUTE out.hdmi.1.video.1,in.hdmi.1.video.1"
#define     DE_SET_INPUT_STREAM_CMD "#X-ROUTE out.hdmi.1.video.1,in.stream.1.video.1"

// decode channel select
#define     CHANNEL_SET_ID_CMD      "#KDS-CHANNEL-SELECT video,"
#define     CHANNEL_GET_ID_CMD      "#KDS-CHANNEL-SELECT? video"
#define     CHANNEL_ID_RECV_HEAD    "@KDS-CHANNEL-SELECT video,"

//decode scale mode
#define     SCALE_MODE_SET_CMD      "#KDS-SCALE "
#define     SCALE_MODE_GET_CMD      "#KDS-SCALE?"
#define     SCALE_MODE_RECV_HEAD    "@KDS-SCALE "

// encode channel define
#define     CHANNEL_DEFINE_SET_CMD  "#KDS-DEFINE-CHANNEL "  
#define     CHANNEL_DEFINE_GET_CMD  "#KDS-DEFINE-CHANNEL?"
#define     CHANNEL_DEFINE_RECV_HEAD "@KDS-DEFINE-CHANNEL "


//temperature
#define     TEMP_GET_CMD            "#HW-TEMP? 0,0"
#define     TEMP_RECV_HEAD          "@HW-TEMP 0, "

//version
#define     FW_VER_GET_CMD          "#VERSION?"
#define     FW_RECV_HEAD            "@VERSION "
#define     BL_VER_GET_CMD          "#BL-VERSION?"
#define     BL_RECV_HEAD            "@BL-VERSION "
#define     HW_VER_GET_CMD          "#HW-VERSION?"
#define     HW_RECV_HEAD            "@HW-VERSION "


// actual resolution
#define     RESLO_GET_CMD           "#KDS-RESOL? 0,1,1"
#define     RESLO_RECV_HEAD         "@KDS-RESOL 0,1,1,"

int init_p3k_client(char *ip, int port)
{
    int err;
    int i = 0;
    for(i = 0; i < 5; i++)
    {
        if(tcp_client_init(ip, port) == 0)
            break;
        else
            sleep(2);
    }
    return err;
}

int get_specified_string_from_file(const char *file, char *channel_list[100]) 
{
    printf("get_specified entry \n");
    int i = 0, n = 0;
    char *str = NULL;
    FILE *fd = NULL;
    char recv_buf[SIZE0] = {0};
    
    fd = fopen(file, "r");
    if (fd == NULL)
    {
        printf("fopen fail");
        return -1;
    }
    
    while(!feof(fd))
    {
        char *info_buf = (char*)malloc(16);
        
        memset(recv_buf, 0, SIZE0);
        fgets(recv_buf, SIZE0, fd);

        if (str = strstr(recv_buf, "id\" :"))
        {
            str += strlen("id\" :");
            while(str[0] == ' ')
            {
                str++;
            }
            
            info_buf[0] = str[0];
            info_buf[1] = ':';

            memset(recv_buf, 0, SIZE0);
            fgets(recv_buf, SIZE0, fd);
            if (str = strstr(recv_buf, "name"))
            {
                n = 0;
                str += strlen("name");
                str = strstr(str, ":") + 1;
                str = strstr(str, "\"") + 1;
                while (str[n] != '"' )
                {
                    info_buf[n+2] = toupper(str[n]);
                    n++;
                }

                printf("%s\n", info_buf);
                channel_list[i] = info_buf;
                
                i++;
            }
        }
    
    }
    fclose(fd);

    return i;
}

static int send_cmd_common(char *cmd, char *head, char *param, char *content)
{
    char mcmd[SIZE1] = {0};
    char recv_buf[SIZE1] = {0};
    
    sprintf(mcmd, "%s", cmd);
    if (param != NULL)
    {
        sprintf(mcmd, "%s%s", mcmd, param);
    }
    printf("send p3k cmd: %s\n", mcmd);
    if (send_p3k_cmd_wait_rsp(mcmd, recv_buf, sizeof(recv_buf)))
    {
        printf("send_p3k_cmd_wait_rsp fail\n");
        return -1;
    }
    
    if (recv_buf[strlen(recv_buf) - 2] == '\r' || recv_buf[strlen(recv_buf) - 1] == '\n')
        recv_buf[strlen(recv_buf) - 2] = '\0';
    
    if (recv_buf[strlen(recv_buf) - 1] == '\r' || recv_buf[strlen(recv_buf) - 1] == '\n')
        recv_buf[strlen(recv_buf) - 1] = '\0';
    printf("response: %s\n", recv_buf);
    
    char * str = NULL;
    if (content != NULL && head != NULL)
    {
        str = strstr(recv_buf, head);
        if (str != NULL)
        {
            str += strlen(head);
            strcpy(content, str);
        }
    }
    return 0;
}

static int split_string(char *src, char *splite_chr, char buf[6][15], int *recv_num)
{
    int i = 0;
    char *substr = NULL;
    substr = strtok(src, splite_chr);
    if (substr == NULL)
        return -1;

    strcpy(buf[i++], substr);
    while(substr = strtok(NULL, splite_chr))
    {
        strcpy(buf[i++], substr);
    }
    *recv_num = i;
}

static int  fill_ip_become_15_byte(char *str, char *dest)
{
    int i;
    int num = 0;
    int offset = 0;
    char tmp[4] = "000";
    char splite_chr[2] = ".";
    char buf1[LITTLE_SIZE] = {0};
    char buf[6][15] = {{0},{0},{0},{0},{0},{0}};

    split_string(str, splite_chr, buf, &num);

    for(i = 0; i < num; i++)
    {
        strcpy(tmp, "000");
        offset = 3 - strlen(buf[i]);
        strcpy(tmp + offset, buf[i]);
        strcat(buf1, tmp);      
        strcat(buf1, ".");
    }
    strncpy(dest, buf1, 15);
}

int GET_IP(int NET_ID, char *IP, char *MASK, char *GATEWAY)
{
    int err;
    char *substr = NULL;
    char recv_buf[SIZE0] = {0};
    
    char cmd[SIZE0] = {0};
    char head[SIZE0] = {0};
    sprintf(cmd, "%s %d", IP_GET_CMD, NET_ID);
    sprintf(head, "%s %d,", IP_RECV_HEAD, NET_ID);
    
    err = send_cmd_common(cmd, head, NULL, recv_buf);
    if (err == -1)
    {
        return -1;
    }

    int i;
    int num = 0;
    char ip_info[6][15] = {{0},{0},{0},{0},{0},{0}};;

    printf("recv_buf:[%s]\n", recv_buf);
    
    split_string(recv_buf, ",", ip_info, &num);
    fill_ip_become_15_byte(ip_info[0], IP);
    fill_ip_become_15_byte(ip_info[1], MASK);
    fill_ip_become_15_byte(ip_info[2], GATEWAY);

    printf("IP = %s\n", IP);

    return 0;
}

int SET_IP(int NET_ID, char *IP, char *MASK, char *GATEWAY)
{
    int err;
    char buf[SIZE1] = {0};
    char respon_head[SIZE1] = {0};
    
    sprintf(buf, " %d,%s,%s,%s", NET_ID, IP, MASK, GATEWAY);
    sprintf(respon_head, "%s %d,", IP_RECV_HEAD, NET_ID);
    
    err = send_cmd_common(IP_SET_CMD, respon_head, buf, NULL);
    return err;
}


#if 0
//#NETDHCP netw_id,dhcp_state
int SET_DHCP_STATUS(int NET_ID)
{
    int err;
    char cmd[SIZE1] = {0};
    char head[SIZE1] = {0};
    sprintf(cmd, "%s %d,1", DHCP_SET_CMD, NET_ID);
    sprintf(head, "%s %d,", DHCP_RECV_HEAD, NET_ID);
    err = send_cmd_common(DHCP_SET_CMD, DHCP_RECV_HEAD, NULL, NULL);
    return err;
}
#endif

//EDID-LIST? [0,"default.bin"],[1,"111.bin"],[4,"4444.bin"],[6,"666666.bin"] 
int GET_EDID_LIST(char EDID_buf[][LITTLE_SIZE])
{
    int i = 0, n = 0, m = 0;
    int err;
    char recv_buf[SIZE1] = {0};
    char *substr1 = NULL;
    char *substr2 = NULL;

    err = send_cmd_common(EDID_LIST_GET_CMD, EDID_LIST_HEAD, NULL, recv_buf);
    if (err != 0)
        return -1;

    i = 1;
    substr1 = recv_buf; 
    while(1)
    {
        substr1 = strstr(substr1, "[");
        if (substr1 != NULL)
        {
            substr1++;
            substr2 = strstr(substr1, "]");
            if (substr2 != NULL)
            {
                substr2--;
                m = 0;
                for (n = 0; n < ((substr2 - substr1)<13?(substr2 - substr1):13); n++)
                {
                    if (substr1[n] != '"')
                    {
                        EDID_buf[i][m] = toupper(substr1[n]);
                        m++;
                    }
                }
                EDID_buf[i][m] = '\0';
                i++;
            }
            else 
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    return i;
}

int SET_EDID(char index)
{
    int err;
    char param[SIZE1] = {0};
    switch (index)
    {
        case 'P':
        {
            sprintf(param, "%s", "PASSTHRU");       
            break;
        }
        
        case '0': //default
        {
            sprintf(param, "%s", "DEFAULT");     
            break;
        }
        
        default: //custom
        {
            sprintf(param, "%s,%c", "CUSTOM", index);           
            break;
        }

    }

    err = send_cmd_common(EDID_SET_CMD_1, EDID_RECV_HEAD_1, param, NULL);
    if (err == -1)
        return -1;
    
    if (index != 'P' && index != '0')
    {
        //memset(param, 0, SIZE1);
        //sprintf(param, "%c", index);
        param[0] = index;
        param[1] = '\0';
        err = send_cmd_common(EDID_SET_CMD_2, EDID_RECV_HEAD_2, param, NULL);
        if (err == -1)
            return -1;
    }
    
    return err;
}

//recv_buf: @EDID-MODE 1,CUSTOM,1
int GET_EDID(char *edid_type)
{
    int err;
    char *str = NULL;
    char recv_buf[SIZE1] = {0};
    if (edid_type == NULL)
        return -1;
    
    err = send_cmd_common(EDID_GET_CMD, EDID_RECV_HEAD_1, NULL, recv_buf);
    if (err == -1)
        return -1;

    strncpy(edid_type, recv_buf, 15);
    /*
    if (strcasestr(recv_buf, "PASSTHRU"))
    {
        strcpy(edid_type, "PASSTHRU");
        break;
    }
    else if (strcasestr(recv_buf, "DEFAULT"))
    {
        strcpy(edid_type, "DEFAULT");
        break;
    }
    else if (strcasestr(recv_buf, "CUSTOM"))
    {
        strcpy(edid_type, recv_buf);
        break;
    }
    */
    return 0;
  
}

//0 OFF, 1 ON
int GET_HDCP_STATUS(char *hdcp_status)
{
    int res;
    char recv_buf[2] = {0};
    res = send_cmd_common(HDCP_GET_CMD, HDCP_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;
    
    res = recv_buf[0]- '0';
    if (res == 0)
    {
        strcpy(hdcp_status, "OFF");
        hdcp_status[3] = '\0';
    }
    else if (res == 1)
    {
        strcpy(hdcp_status, "ON");
        hdcp_status[2] = '\0';
    }
    else
    {
        strcpy(hdcp_status, "SHOW FAIL");
        hdcp_status[strlen("SHOW FAIL")] = '\0';
    }
    return 0;
}

int GET_HDCP_MODE(char *mode)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};

    res = send_cmd_common(HDCP_MODE_GET_CMD, HDCP_MODE_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(mode, recv_buf);
    
    return 0;
}

// 0-HDCP OFF,   1-HDCP ON 
int  SET_HDCP_MODE(char *mode)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    char send_buf[100] = {0};

    if (strcmp(mode, "OFF") == 0)
    {
        res = send_cmd_common(HDCP_MODE_SET_CMD, HDCP_MODE_RECV_HEAD, "0", NULL);
    }
    else if (strcmp(mode, "ON") == 0)
    {
        res = send_cmd_common(HDCP_MODE_SET_CMD, HDCP_MODE_RECV_HEAD, "1", NULL);
    }

    if (res == -1)
        return -1;
    
    return 0;
}


#if 0
//HW VER 0.1  FW VER 0.1.0 
int get_FIRMWARE_INFO(char info_buf[][SIZE2]) //数组指针
{
    int err = 0;
    err = get_FIREMARE_VERSION(FIRMWARE_INFO_FILE, FIRMWARE_SYMBOL, info_buf);
    return err;
}



/*
0   – power on
1   – standby
2   – FW Background Download
3   – IP fallback address
*/

int get_DEVICE_STATUS(int *Dvc_Status)
{
    int err;
    char recv_buf[2] = {0};
    err = send_cmd_common(DEVICE_STATUS_GET, DEVICE_RECV_HEAD, NULL, recv_buf);
    *Dvc_Status = recv_buf[0]- '0';
    return err;
}

int get_INPUT_INFO(char *recv_buf)
{
    int err;
    err = send_cmd_common(INPUT_GET_CMD, INPUT_RECV_HEAD, NULL, recv_buf);
    if (err == -1)
        printf("send_cmd_common fail\n");
    
    return err;
}

int set_INPUT_INFO(int num)
{
    int err;
    switch(num)
    {
        case 1:
        {
            err = send_cmd_common(INPUT_SET_1_CMD, INPUT_RECV_HEAD, NULL, NULL);
            break;
        }

        case 2:
        {
            err = send_cmd_common(INPUT_SET_2_CMD, INPUT_RECV_HEAD, NULL, NULL);
            break;
        }

        case 3:
        {
            err = send_cmd_common(INPUT_SET_3_CMD, INPUT_RECV_HEAD, NULL, NULL);
            break;
        }

    }
    
    return err;
}
#endif

int CHANNEL_LIST(char *info_buf[100])
{
    int err;
    err = get_specified_string_from_file(VIDEO_INFO_FILE, info_buf);
    return err;
}

/*
int select_voide_channel(int num)
{
    int err;
    char cmd[SIZE1] = {0};
    sprintf(cmd, "%s%d", CHANNEL_SET_CMD, num);
    
    err = send_cmd_common(cmd, CHANNEL_RECV_HEAD, NULL, NULL); 
    return err;
}

int get_current_voide_channel(int *channel_num)
{
    int err;
    char recv_buf[2] = {0};
    
    err = send_cmd_common(CHANNEL_GET_CMD, CHANNEL_RECV_HEAD, NULL, recv_buf);
    if (err == -1)
        return -1;
    
    *channel_num = recv_buf[0] - '0';
    return err;
}
*/

int GET_CHANNEL_DEFINE(char *id)
{
    int i = 0;
    int res = 0;
    char str[LITTLE_SIZE] = {0};
    char recv_buf[LITTLE_SIZE] = {0};
    
    res = send_cmd_common(CHANNEL_DEFINE_GET_CMD, CHANNEL_DEFINE_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    int lenth = strlen(recv_buf);
    
    if (lenth < 3)
    {
        for (i = 0; i < lenth; i++)
        {
            str[3-lenth+i] = recv_buf[i];
        }
    
        for (i = 0; i < 3-lenth; i++)
        {
            str[i] = '0';
        }
        strcpy(id, str);
    }
    else
    {
        strcpy(id, recv_buf);
    }
    str[3] = 0;
    
    
    return 0;
}

int SET_CHANNEL_DEFINE(char *id)
{
    int res = 0;    
    res = send_cmd_common(CHANNEL_DEFINE_SET_CMD, CHANNEL_DEFINE_RECV_HEAD, id, NULL);
    if (res == -1)
        return -1;  
    return 0;
}

int GET_CHANNEL_ID()
{
    int res = 0;
    int id = -1;
    char recv_buf[LITTLE_SIZE] = {0};
    
    res = send_cmd_common(CHANNEL_GET_ID_CMD, CHANNEL_ID_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    id = recv_buf[0] - '0';
    return id;
}

int SET_CHANNEL_ID(char *id)
{
    int res = 0;
    res = send_cmd_common(CHANNEL_SET_ID_CMD, CHANNEL_ID_RECV_HEAD, id, NULL);
    if (res == -1)
        return -1;  
    return 0;   
}

int GET_TEMPERATURE(char *temp)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};

    res = send_cmd_common(TEMP_GET_CMD, TEMP_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(temp, recv_buf);
    
    return 0;
}

int GET_FW_VERSION(char *FW_VER)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    res = send_cmd_common(FW_VER_GET_CMD , FW_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(FW_VER, recv_buf);
    
    return 0;
}

int GET_BL_VERSION(char *BL_VER)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    res = send_cmd_common(BL_VER_GET_CMD, BL_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(BL_VER, recv_buf);
    
    return 0;
}

int GET_HW_VERSION(char *HW_VER)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    res = send_cmd_common(HW_VER_GET_CMD, HW_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(HW_VER, recv_buf);
    
    return 0;
}

//#X-ROUTE
int GET_ENCODE_VIDEO_INPUT(char *type)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    res = send_cmd_common(EN_INPUT_GET_CMD, EN_INPUT_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;
    strcpy(type, recv_buf);
    
    return 0;
    
}

int SET_ENCODE_VIDEO_INPUT(const char *type)
{
    int res = 0;
    if (strstr(type, "HDMI IN1") != NULL)
    {
        res = send_cmd_common(DE_SET_INPUT_STREAM_CMD, NULL, NULL, NULL);
    }
    else if (strstr(type, "HDMI IN2") != NULL)
    {
        res = send_cmd_common(DE_SET_INPUT_STREAM_CMD, NULL, NULL, NULL);
    }
    else if (strstr(type, "USB_C IN") != NULL)
    {
        res = send_cmd_common(DE_SET_INPUT_STREAM_CMD, NULL, NULL, NULL);
    }

    if (res == -1)
        return -1;
    else
        return 0;
    
}


int GET_DECODE_VIDEO_INPUT(char *type)
{
    int res = 0;
    char recv_buf[LITTLE_SIZE] = {0};
    res = send_cmd_common(DE_INPUT_GET_CMD, DE_INPUT_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(type, recv_buf);
    
    return 0;
}

int SET_DECODE_VIDEO_INPUT_TYPE(const char *type)
{
    int res = 0;
    if (strstr(type, "stream") != NULL)
    {
        res = send_cmd_common(DE_SET_INPUT_STREAM_CMD, NULL, NULL, NULL);
    }
    else if (strstr(type, "hdmi") != NULL)
    {
        res = send_cmd_common(DE_SET_INPUT_HDMI_CMD, NULL, NULL, NULL);
    }

    if (res == -1)
        return -1;
    else
        return 0;
}

#if 0
//#KDS-SCALE?
//@KDS-SCALE value,res_type
//value: 0  – Pass Thru。1   – Scaling
//res_type 1,2,3,4....
//@KDS-SCALE 1,16
int get_VIDEO_OUT(int *video_data)
{
    
    int err;
    char recv_buf[SIZE1] = {0};
    err = send_cmd_common(VIDEO_OUT_GET_CMD, VIDEO_OUT_RECV_HEAD, NULL, recv_buf);
    if (err == -1)
        return -1;
    
    *video_data = atoi(&recv_buf[2]);
    
    return err;
}
#endif

//@KDS-SCALE? 0,0
int GET_SCALE_MODE(char *mode)
{
    int res;
    char recv_buf[SIZE1] = {0};
    res = send_cmd_common(SCALE_MODE_GET_CMD, SCALE_MODE_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    strcpy(mode, recv_buf);
    return 0;
}

int SET_SCALE_MODE(const char *mode)
{
    int res;
    res = send_cmd_common(SCALE_MODE_SET_CMD, SCALE_MODE_RECV_HEAD, (char*)mode, NULL);
    if (res == -1)
        return -1;

    return 0;
}

//#KDS-RESOL? 0,1,1
int GET_ACTUAL_RESOLUTION(char *resol_buf)
{
    int res;
    int lenth = 0;
    char recv_buf[20] = {0};
    res = send_cmd_common(RESLO_GET_CMD, RESLO_RECV_HEAD, NULL, recv_buf);
    if (res == -1)
        return -1;

    printf("recv:[%s]\n", recv_buf);
    res = atoi(recv_buf);

    if ((res > 0) && (res < 76))
    {
        lenth = strlen(ResolutionList[res]);
        strncpy(resol_buf, ResolutionList[res], lenth);
        resol_buf[lenth] = '\0';
    }
    else
    {
        strcpy(resol_buf, "NO SIGNAL");
        resol_buf[9] = '\0';
    }
    return 0;
}




