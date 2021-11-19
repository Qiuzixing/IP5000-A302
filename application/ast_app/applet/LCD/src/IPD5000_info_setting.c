
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>


#include "oled.h"
#include "IPD5000_info_setting.h"
#include "msg_queue.h"
#include "send_p3k_cmd.h"
#define _GNU_SOURCE

#define SIZE_D 255
#define MIN_SIZE_D 20

#define LAN1_ID_D   0
#define LAN2_ID_D   1

#define CHANNEL_MAP "/data/configs/kds-7/channel/channel_map.json"

pthread_mutex_t g_lock_D;

time_t last_change_time_D = 0;
//方向
enum 
{
    ENTER_KEY = 1,
    UP_KEY,
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
};

typedef struct
{
    int x;          //x坐标
    int y;          //y坐标
    int p;          //记录字符数组中 要显示在第二行，即第一个数据栏的位置。
    int last_page;  //是否到达最后一页（翻页）
}info_param;


char *blank_D = " ";
#if 0
//从中用来获取显示数据的
const char *video_out[] = {
    "VIDEO OUT RES", "NO SIGNAL", "3840x2160P30", "3840x2160P25",
    "1920x1080P60", "1920X1080P50", "1280x720P60",  
};

const char *device_status[5] = {
    "DEVICE_STATUS", "POWER ON", "STANDY BY", "FW DOWNLOAD", "IP FALLBACK"
}; 


// save actual value    
char video_select_buf[MIN_SIZE_D][SIZE_D];
char firmware_buf[MIN_SIZE_D][SIZE_D];
#endif

// LEVEL 1
const char* MAIN_MENU_LIST_D[] = {
    "MAIN MENU", "DEV STATUS", "DEV INFO", "DEV SETTINGS",
};
// END LEVEL 1


// LEVEL 2 
const char* DEV_STATUS_LIST_D[] = {
    "DEV STATUS", "LAN1 STATUS", "LAN2 STATUS", "HDMI STATUS", "CHANNEL SEL", "TEMPERATURE",
};


char FW_VERSION_D[20] = {0};
char BL_VERSION_D[20] = {0};
char HW_VERSION_D[20] = {0};
const char* DEV_INFO_LIST_D[] = {
    "DEV INFO", (const char*)FW_VERSION_D, (const char*)BL_VERSION_D, (const char*)HW_VERSION_D,
};

const char* DEV_SETTINGS_LIST_D[] = {
    "DEV SETTINGS", "INPUT SETTING", "HDCP SETTING", "RESOL SETTING", "CH SELECT",
};
// END LEVEL 2


// LEVEL 3
char RESOL_BUF[20]       = {0};
char HDCP_STATUS_BUF[20] = {0};
const char* HDMI_STATUS_LIST_D[] = {
    "HDMI STATUS", (const char*)RESOL_BUF, (const char*)HDCP_STATUS_BUF,
};

const char* INPUT_VIDEO_SOURCE_TYPE[] = {
    "INPUT SETTING", "STREAM IN", "HDMI IN",
};

const char *HDCP_LIST_D[] = {"HDCP SETTING", "ON", "OFF"};

const char* RESOL_SCALE_LIST_D[] = {"RESOL SETTING", "PER EDID", "720p60", "1080p60", "1080p50", "2160p25", "2160p30"};
const char* SCALE_MODE_D[] = {"0,0", "1,4" , "1,16", "1,31", "1,73", "1,74"};

int CH_TATOL_NUM_D = 0;
char *CH_LIST_D[100] = {"NO SIGNAL",};

// END LEVEL 3


#if 0
const char* MAIN_MENU_strings_D[] = {
    "MAIN MENU", "VIDEO SELECT", "IP SETTING", "HDCP SETTING",
    "VIDEO OUT RES", "FIRMWARE INFO", "DEVICE STATUS",
};

const char* IP_SET_strings_D[] = {
    "IP SETTING", "LAN1 SETTING", "LAN2 SETTING",
};
    


const char* LAN_OPTION_strings_D[] = {
    "LAN INFO", "LAN ADDR", "LAN MASK", "LAN GATEWAY",
};

const char* HDCP_strings_D[] = {
    "HDCP SETTING", "HDCP ON", "HDCP OFF",
};


const char* FIRMWARE_strings_D[MIN_SIZE_D+1]        = {"FIRMWARE INFO",};
const char* DEVICE_STATUS_strings_D[MIN_SIZE_D+1]   = {"DEVICE STATUS",};
const char* VIDEO_OUT_string_D[2]       = {"VIDEO OUT RES", " "};

//用来存储网络配置信息: 网口1：ip, mask gateway.网口2: ip, mask gateway 共6个
char net_info_D[6][16] = {{0}, {0}, {0}, {0}, {0}, {0}};

//记录正在屏幕显示的字符串，一个屏幕最多显示4行.
const char *MAIN_MENU_SHOWWING_D[4]     =   {NULL, NULL, NULL, NULL}; 
const char *IP_SET_SHOWWING_D[4]        =   {NULL, NULL, NULL, NULL};

const char *LAN1_OPTION_SHOWWING_D[4]   =   {NULL, NULL, NULL, NULL};
const char *VIDEO_OUT_SHOWWING_D[4]     =   {NULL, NULL, NULL, NULL};
const char *FIRMWARE_SHOWWING_D[4]      =   {NULL, NULL, NULL, NULL};
const char *DEVICE_STATUS_SHOWWING_D[4] =   {NULL, NULL, NULL, NULL};
#endif

const char* SAVE_VIDEO_SELECT_D[MIN_SIZE_D+1]       = {"CHANNEL SEL",}; 
const char *VIDEO_IN_SHOWWING_D[4]           =    {NULL, NULL, NULL, NULL};

//Records the string being displayed
const char *MAIN_MENU_SHOWWING_D[4]       =    {NULL, NULL, NULL, NULL}; 
const char *DEV_STATUS_SHOWWING_D[4]      =    {NULL, NULL, NULL, NULL};
const char *DEV_INFO_SHOWWING_D[4]        =    {NULL, NULL, NULL, NULL};
const char *DEV_SETTINGS_SHOWWING_D[4]    =    {NULL, NULL, NULL, NULL};
const char *INPUT_VIDEO_SHOWWING_D[4]    =    {NULL, NULL, NULL, NULL};
const char *HDMI_STATUS_SHOWWING_D[4]     =    {NULL, NULL, NULL, NULL};
const char *RESOL_SHOWWING_D[4]           =    {NULL, NULL, NULL, NULL};
//const char *CHANNEL_SHOWWING_D[4]         =    {NULL, NULL, NULL, NULL};

//记录中括号的X坐标位置
static int move_limit_D = 0; 


#if 0
/*
    1.首先获取初始化数据，既要显示的东西
    2.如果有更改，那么就要往P3K发消息。send
    3.是不是要再获取一次，看有没有真正改变了。那么这个显示速度问题。
*/
int save_LAN_info_D()
{
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        net_info_D[i][15] = '\0';
    }
    get_ip(LAN1_ID_D, net_info_D[0], net_info_D[1], net_info_D[2]);
    get_ip(LAN2_ID_D, net_info_D[3], net_info_D[4], net_info_D[5]);
    
}

int save_VIDEO_SELECT_info_D()
{
    int i = 0, err = 0;
    memset(video_select_buf, 0, MIN_SIZE_D*SIZE_D);

    err = VIDEO_LIST(video_select_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_D; i++)
        {
            strcpy(video_select_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_D+1; i++)
    {
        SAVE_VIDEO_SELECT_D[i] = video_select_buf[i-1];
    }
}

int save_FIREWARE_info_D()
{
    int i = 0, err = 0;
    memset(firmware_buf, 0, MIN_SIZE_D*SIZE_D);

    err = get_FIRMWARE_INFO(firmware_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_D; i++)
        {
            strcpy(firmware_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_D+1; i++)
    {
        FIRMWARE_strings_D[i] = firmware_buf[i-1];
    }
}

int save_DEVICE_STATUS_info_D()
{
    int i = 0;
    int status = -1;
    get_DEVICE_STATUS(&status);
    DEVICE_STATUS_strings_D[1] = device_status[status+1];
}

int get_current_VIDEO_OUT_info_D()
{
    int s;
    int res_type;
    get_VIDEO_OUT(&res_type);
    switch(res_type)
    {
        case NO_SIGNAL:
        {
            s =1;
            break;
        }
        case VIDEO_OUT_74:
        {
            s = 2;
            break;
        }
        case VIDEO_OUT_73:
        {
            s = 3;
            break;
        }
        case VIDEO_OUT_16:
        {
            s = 4;
            break;
        }
        case VIDEO_OUT_31:
        {
            s = 5;
            break;
        }
        case VIDEO_OUT_4:
        {
            s = 6;
            break;
        }

    }
    VIDEO_OUT_string_D[1] = video_out[s];

}
#endif


int file_is_changed_D(const char* filename)
{
    int err = -1;
    struct stat state;
    
    if (filename == NULL)
        return -1;
    
    if (access(filename, F_OK|R_OK) == -1)
        return -1;

    if (stat(filename, &state) != 0)
        return -1;

    if (last_change_time_D == state.st_mtime)
    {
        return 0;
    }
    else
    {
        last_change_time_D = state.st_mtime;
        return 1;
    }
}

/*
void save_display_info_D()
{
    
    init_p3k_client("127.0.0.1", 6001);
    
    save_LAN_info_D();
    save_VIDEO_SELECT_info_D();
    save_FIREWARE_info_D();
    save_DEVICE_STATUS_info_D();
    get_current_VIDEO_OUT_info_D();
}
*/
u8 get_elem_num_D(const char **buf, int num)
{
    u8 i = 0;
    while (i < num)
    {
        if (buf[i] == NULL || strlen(buf[i]) <= 1)
        {
            break;
        }
        i++;
    }
    return i;
}

/*
    y: 显示起始列坐标,要取8的整数倍, 0, 8, 16 .... 8*15
    begin_num: 要显示src中要元素起始位
    dest: 记录src哪些元素被显示
    src: 从src中拿取字符串显示
    count: 从src的拿去元素的个数, 最多只能拿三个
*/
void show_menu_info_D(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
{
    //取剩下的显示，最多取三个
    u8 x = 0;
    int i; 
    
    dest[0] = src[0];

    if (count == 0)
    {
        move_limit_D = 2;
        return;
    }   
    for (i = 0; i < count; i++)
    {
        dest[1+i] = src[begin_elem+i];
    }
    
    move_limit_D = i*2; //选择框向下最大移动坐标

    for (i+=1; i < 4; i++)
    {
        dest[i] = blank_D;
    }
    
    //显示
    for (i = 1; i < 4; i++)
    {
        show_strings(x+2*i, y, dest[i], strlen(dest[i]), 1);
    }

}

//响应 下行键，上行键
info_param down_up_respond_D(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
{
    int x = param.x;
    int y = param.y;
    int p = param.p;
    int last_page = param.last_page;

    switch (key)
    {
        case DOWN_KEY:
        {
            //光标不移动才翻页
            x += 2; 
            if (x > move_limit_D) //X坐标超过最大值，取最大值
            {
                x = move_limit_D;
                
                if (count > 4)//要超过4行才能翻页，
                {
                    //p被上行键更改了 或者count <= 7时，p不会被上行键更改，但页面已经被向上翻页了，通过MAIN_MENU_SHOWWING[3],MAIN_MENU_strings_D[3]来判断是否是最后一页。
                    if ( (p + 3 < count) || (strcmp(showwing_strings[3], src_string[3]) == 0))
                    {
                        last_page = 0;  
                    }
                    
                    if ((count - p > 0) && last_page == 0) //还有也可以翻，且没有翻到最后一页
                    {
                        if (p + 3 < count) 
                        {   
                            clear_three_line();
                            show_menu_info_D(y, p, showwing_strings, src_string, 3); //数组是从0开始
                            p += 3;
                        }
                        else 
                        {
                            clear_three_line();
                            show_menu_info_D(y, p, showwing_strings, src_string, count-p);
                            last_page = 1;
                        }
                        x = 2;
                    }
                }
            }
            show_square_breakets(x);
            
            break;
        }

        case UP_KEY:
        {
            x -= 2;
            //光标不移动才翻页
            if (x < 2)
            {
                x = 2;
                if ((count > 4) && (strcmp(showwing_strings[1], src_string[1])) != 0) //要超过4行才能翻页，
                {
                    
                    if ( (p >= 4))
                    {
                        if (p > 4)
                        {
                            p -= 3;
                            clear_three_line();
                            show_menu_info_D(y, p, showwing_strings, src_string, 3);
                        }
                        else //p==4
                        {
                            clear_three_line();
                            show_menu_info_D(y, p-3, showwing_strings, src_string, 3);
                        }
                        x = 2;
                    }
                    
                }
            }
            show_square_breakets(x);
            
            break;
        }
    }
    
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;
    
    return param;
}

static int recv_init_D()
{
    int err = 0;
    pthread_mutex_init(&g_lock_D, NULL);
    
    err = msg_queue_destroy();
    if (err != 0)
    {
        printf("msg_queue_destroy fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }

    chmod("/var/run/send_key_pressed_info", S_IXUSR|S_IXGRP|S_IXOTH); //

    return 0;
}

static int recv_key_info_D()
{
    int err = 0;
    struct msg_buf msg;

    pthread_mutex_lock(&g_lock_D);
    err = msg_recv_state(&msg);
    if (err == -1)
    {
        printf("msg_recv_state fail");
        return -1;
    }
    pthread_mutex_unlock(&g_lock_D);
    
    return msg.mtext[0];
}

// The first level: 1. MAIN MENU
int IPD5000_MAIN_MENU_SHOW(void)
{
    printf("this is IPD5000\n");
    init_p3k_client("127.0.0.1", 6001);

    CH_TATOL_NUM_D = get_specified_string_from_file(CHANNEL_MAP, CH_LIST_D);
    printf("CH_TATOL_NUM_E = %d\n", CH_TATOL_NUM_D);
    
    u8 count = sizeof(MAIN_MENU_LIST_D)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    if (recv_init_D() != 0)
    {
        printf("recv_init_D() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_LIST_D[0], strlen(MAIN_MENU_LIST_D[0]), 1);
    show_menu_info_D(y, 1, MAIN_MENU_SHOWWING_D, MAIN_MENU_LIST_D, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_D(count, param, MAIN_MENU_SHOWWING_D, MAIN_MENU_LIST_D, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_LIST_D[1]) // DEV STATUS
                    {
                        DEV_STATUS_D();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_LIST_D[2]) // DEV INFO
                    {
                        DEV_INFO_D();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_LIST_D[3]) // DEV SETTINGS
                    {
                        DEV_SETTINGS_D();
                        break;
                    }
    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, MAIN_MENU_SHOWWING_D[i], strlen(MAIN_MENU_SHOWWING_D[i]), 1);
                for (i = 1; i < count; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_D[i], strlen(MAIN_MENU_SHOWWING_D[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_D[i]) != 1) 
                    {
                        move_limit_D = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                break;
            }
        }
    }
}

// the second level: 2.1 DEV STATUS
static int DEV_STATUS_D()
{
    u8 count = sizeof(DEV_STATUS_LIST_D)/(sizeof(char*)); //DEV_STATUS_D的元素个数
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, DEV_STATUS_LIST_D[0], strlen(DEV_STATUS_LIST_D[0]) ,1); 
    show_menu_info_D(y, 1, DEV_STATUS_SHOWWING_D, DEV_STATUS_LIST_D, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_D(count, param, DEV_STATUS_SHOWWING_D, DEV_STATUS_LIST_D, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_STATUS_SHOWWING_D[param.x/2] == DEV_STATUS_LIST_D[1]) // LAN1 STATUS 
                    {
                        LAN_STATUS_D(LAN1_ID_D);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_D[param.x/2] == DEV_STATUS_LIST_D[2]) // LAN1 STATUS 
                    {
                        LAN_STATUS_D(LAN2_ID_D);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_D[param.x/2] == DEV_STATUS_LIST_D[3]) // HDMI STATUS
                    {
                        HDMI_STATUS_D();
                        break;
                    }
    
                    if (DEV_STATUS_SHOWWING_D[param.x/2] == DEV_STATUS_LIST_D[4]) // CHANNEL SEL
                    {
                        CHANNEL_SHOW_D();
                        break;
                    }
                    if (DEV_STATUS_SHOWWING_D[param.x/2] == DEV_STATUS_LIST_D[5]) // TEMPERATURE
                    {
                        TEMPERATURE_D();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_STATUS_SHOWWING_D[i], strlen(DEV_STATUS_SHOWWING_D[i]), 1);
                for (i = 1; i < 4; i++)
                {
                    if (DEV_STATUS_SHOWWING_D[i] != NULL)
                        show_strings(i*2, y, DEV_STATUS_SHOWWING_D[i], strlen(DEV_STATUS_SHOWWING_D[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_STATUS_SHOWWING_D[i]) != 1) 
                    {
                        move_limit_D = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                return 0;
            }
        }
    }
    
}

// 1.1 DEV STATUS -> LAN SHOW
static void LAN_STATUS_D(int interface_id)
{
    char addr[15] = {0}, mask[15] = {0}, gateway[15] = {0};
    GET_IP(interface_id, addr, mask, gateway);

    clear_whole_screen();
    if (interface_id == LAN1_ID_D)
        show_strings(0, 16, "LAN 1 STATUS", strlen("LAN 1 STATUS"), 1);
    else if (interface_id == LAN2_ID_D)
        show_strings(0, 16, "LAN 2 STATUS", strlen("LAN 2 STATUS"), 1);
    
    show_strings(2, 8, addr, 15, 1);
    show_strings(4, 8, mask, 15, 1);
    show_strings(6, 8, gateway, 15, 1);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        
        switch (key)
        {
            case LEFT_KEY: 
            {  
                return;
            }
        }
    }
}

// 1.3 DEV STATUS -> HDMI STATUS
static int HDMI_STATUS_D()
{
    //1.actual resolution
    GET_ACTUAL_RESOLUTION(RESOL_BUF);
    
    //2.HDCP STATUS
    GET_HDCP_STATUS(HDCP_STATUS_BUF);

    u8 count = sizeof(HDMI_STATUS_LIST_D)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, HDMI_STATUS_LIST_D[0], strlen(HDMI_STATUS_LIST_D[0]) ,1); 
    show_menu_info_D(y, 1, HDMI_STATUS_LIST_D, HDMI_STATUS_LIST_D, count>4 ? 3 : count-1);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
        
}

// 1.4 DEV STATUS -> CHANNEL SHOW
static int CHANNEL_SHOW_D()
{
    int id = -1;
    char buf[20] = {0};
    id = GET_CHANNEL_ID();
    if (CH_TATOL_NUM_D == 0)
    {
        id = 0;
    }
        
    clear_whole_screen();
    show_strings(0, 16, "CHANNEL SEL", strlen("CHANNEL SEL") ,1);
    show_strings(2, 16, CH_LIST_D[id], strlen(CH_LIST_D[id]) ,1);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }

}

// 1.5 DEV STATUS -> TEMPERATURE
static int TEMPERATURE_D()
{
    int err = -1;
    char temp[20] = {0};
    
    err = GET_TEMPERATURE(temp);
    if (err == -1)
        return -1;

    clear_whole_screen();
    show_strings(0, 16, "TEMPERATURE", strlen("TEMPERATURE") ,1);
    show_strings(2, 16, temp, strlen(temp) ,1);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
    
}


// 2. DEV INFO
static int DEV_INFO_D()
{
    u8 count = sizeof(DEV_INFO_LIST_D)/(sizeof(char*));
    
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    //get device info
    GET_FW_VERSION(FW_VERSION_D);
    GET_BL_VERSION(BL_VERSION_D);
    GET_HW_VERSION(HW_VERSION_D);
    
    clear_whole_screen();
    show_strings(0, y, DEV_INFO_LIST_D[0], strlen(DEV_INFO_LIST_D[0]), 1); 
    show_menu_info_D(y, 1, DEV_INFO_SHOWWING_D, DEV_INFO_LIST_D, count>4? 3 : count-1);
    //show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            /*
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, DEV_INFO_SHOWWING_D, DEV_INFO_LIST_D, key);
                break;
            }
            */
            
            case LEFT_KEY:  //返回上一级目录
            {
                return 0;
            }
        }
    }
}


// 3  DEV SETTINGS
static int DEV_SETTINGS_D()
{
    u8 count = sizeof(DEV_SETTINGS_LIST_D)/(sizeof(char*)); //DEV_STATUS_D的元素个数
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, DEV_SETTINGS_LIST_D[0], strlen(DEV_SETTINGS_LIST_D[0]), 1); 
    show_menu_info_D(y, 1, DEV_SETTINGS_SHOWWING_D, DEV_SETTINGS_LIST_D, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_D(count, param, DEV_SETTINGS_SHOWWING_D, DEV_SETTINGS_LIST_D, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_SETTINGS_SHOWWING_D[param.x/2] == DEV_SETTINGS_LIST_D[1]) // INPUT SETTING 
                    {
                        INPUT_SETTING_D();
                        printf("return later\n");
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_D[param.x/2] == DEV_SETTINGS_LIST_D[2]) // HDCP SETTING
                    {
                        HDCP_SETTING_D();
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_D[param.x/2] == DEV_SETTINGS_LIST_D[3]) // RESOL SETTING
                    {
                        RESOL_SETTING_D();
                        break;
                    }
    
                    if (DEV_SETTINGS_SHOWWING_D[param.x/2] == DEV_SETTINGS_LIST_D[4]) // CH SELECT
                    {
                        CH_SELECT_D();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();

                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                for (i = 0; i < 4; i++)
                {
                    if (DEV_SETTINGS_SHOWWING_D[i] != NULL)
                        show_strings(i*2, y, DEV_SETTINGS_SHOWWING_D[i], strlen(DEV_SETTINGS_SHOWWING_D[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_SETTINGS_SHOWWING_D[i]) != 1) 
                    {
                        move_limit_D = i*2;
                        break;
                    }
                }

                break;
            }
            case LEFT_KEY:
            {   
                return 0;
            }
        }
    }

}

static int INPUT_SETTING_D()
{
    int err = -1;
    int type_i = -1;
    char type[100] = {0};
    
    err = GET_DECODE_VIDEO_INPUT(type);
    if (err == -1)
        return -1;

    printf("type:[%s]\n", type);
    
    if (strcasestr(type, "stream") != NULL)
    {
        type_i = 1;
    }
    else if (strcasestr(type, "hdmi") != NULL)
    {
        type_i = 0;
    }
    
    u8 count = sizeof(INPUT_VIDEO_SOURCE_TYPE)/(sizeof(char*)); //DEV_STATUS_D的元素个数
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, INPUT_VIDEO_SOURCE_TYPE[0], strlen(INPUT_VIDEO_SOURCE_TYPE[0]), 1); 
    show_menu_info_D(y, 1, INPUT_VIDEO_SHOWWING_D, INPUT_VIDEO_SOURCE_TYPE, count>4? 3 : count-1);
    show_square_breakets(x);
    
    if (type_i == 1)
    {
        show_a_star(2);
    }
    else if (type_i == 0)
    {
        show_a_star(4);
    }

    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case DOWN_KEY:
            {
                if (x == 2)
                {
                    x = 4;
                    show_square_breakets(x);
                }
                break;
            }
            
            case UP_KEY:
            {
                if (x == 4)
                {
                    x = 2;
                    show_square_breakets(x);
                }
                
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                if (x == 2) //stream
                {
                    SET_DECODE_VIDEO_INPUT_TYPE("stream");
                    show_a_star(x);
                }
                if (x == 4) //hdmi
                {                   
                    SET_DECODE_VIDEO_INPUT_TYPE("hdmi");
                    show_a_star(x);
                }
                break;  
            }
            case LEFT_KEY:
            {
                printf("return \n");
                return 0;
            }
#if 0
            case LEFT_KEY:
            {   
                printf("return \n");
                return 0;
            }
#endif
        }

    }

    
}

static int HDCP_SETTING_D()
{
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    int star_x = 2;
    char mode[20] = {0};
    GET_HDCP_MODE(mode);
    if (strstr(mode, "ON") != NULL)
    {
        star_x = 2;
    }
    else if (strstr(mode, "OFF") != NULL)
    {
        star_x = 4;
    }

    clear_whole_screen();  //新一级的目录，清屏
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        show_strings(i*2, y, HDCP_LIST_D[i], strlen(HDCP_LIST_D[i]), 1);
    }
    show_square_breakets(star_x);
    show_a_star(star_x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            {
                if (x == 2)
                {
                    x = 4;
                    show_square_breakets(x);
                }
                break;
            }
            
            case UP_KEY:
            {
                if (x == 4)
                {
                    x = 2;
                    show_square_breakets(x);
                }
                
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                if (x == 2)
                {
                    show_a_star(x);
                    SET_HDCP_MODE("ON");
                }
                
                if (x == 4)             
                {                   
                    show_a_star(x);
                    SET_HDCP_MODE("OFF");
                }
                break;
            }
            
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
    return 0;
}

static find_active_resol(char *mode)
{
    int i = 0, n =0;
    int num = sizeof(SCALE_MODE_D)/(sizeof(char*));
    
    for (i = 0; i < num; i++)
    {
        if (strstr(mode, SCALE_MODE_D[i]) != NULL)
        {
            for (n = 1; n < 4; n++)
            {
                if (RESOL_SHOWWING_D[n] == RESOL_SCALE_LIST_D[i+1])
                {
                    show_a_star(2*n);
                    break;
                }
            }
            
            break;
        }
    }

}
static int RESOL_SETTING_D()
{
    u8 count = sizeof(RESOL_SCALE_LIST_D)/(sizeof(char*));
    
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    char mode[20] = {0};
    int star_x = 2;
    GET_SCALE_MODE(mode);

    printf("mode: %s\n", mode);
    
    clear_whole_screen();
    show_strings(0, y, RESOL_SCALE_LIST_D[0], strlen(RESOL_SCALE_LIST_D[0]), 1); 
    show_menu_info_D(y, 1, RESOL_SHOWWING_D, RESOL_SCALE_LIST_D, count>4? 3 : count-1);
    show_square_breakets(x);
    find_active_resol(mode);
    
    int i = 0;

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_D(count, param, RESOL_SHOWWING_D, RESOL_SCALE_LIST_D, key);
                find_active_resol(mode);
                break;
            }
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                for (i = 1; i < count; i++)
                {
                    if (RESOL_SHOWWING_D[param.x/2] == RESOL_SCALE_LIST_D[i])
                    {
                        break;
                    }
                }
                if (i < count)
                {
                    show_a_star(param.x);
                    SET_SCALE_MODE(SCALE_MODE_D[i-1]);
                }
            
                break;
            }
            case LEFT_KEY:
            {
                return 0;
            }
            
        }

    }
    return 0;
}

static int CH_SELECT_D()
{
    int res = -1;
    
    res = file_is_changed_D(CHANNEL_MAP);
    if (res == 1)
    {
        CH_TATOL_NUM_D = get_specified_string_from_file(CHANNEL_MAP, CH_LIST_D);
    }

    u8 count = CH_TATOL_NUM_D;

    int x = 2; 
    int y = 16;
    int active_id = -1;
    active_id = GET_CHANNEL_ID();

    //要显示哪三个? 三三进行分组显示, 组号从0开始
    int now_group_id = 0;   //第几组，
    int member_id = 0;     //组内成员编号。 共三个:1,2,3
    now_group_id = (active_id -1)/3;
    member_id = active_id % 3;
    x = member_id * 2;

    int total_group_count = (count-1)/3 + 1;
    int count_last_group_member = (count)%3;
    
    clear_whole_screen();
    show_strings(0, y, CH_LIST_D[0], strlen(CH_LIST_D[0]), 1);

    //是否有三行内容显示
    int num  = 0;
    int temp_x = 2;
    while(num < 3)
    {
        if ((now_group_id*3+1+num) > count)
            break;
        show_strings(temp_x, y, CH_LIST_D[now_group_id*3+1+num], strlen(CH_LIST_D[now_group_id*3+1+num]), 1);
        temp_x += 2;
    }

    show_square_breakets(x);
    show_a_star(x);

    int trace_id = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            {
                if (x != 6)
                {
                    x+=2;
                    show_square_breakets(x);
                    trace_id = (now_group_id*3+1) + (x/2-1);
                }
                else
                {
                    if (now_group_id < total_group_count) //后面还有组可以显示，
                    {
                        now_group_id++;
                        num  = 0;
                        temp_x = 2;
                        while(num < 3)
                        {
                            if ((now_group_id*3+1+num) > count)
                                break;
                            show_strings(temp_x, y, CH_LIST_D[now_group_id*3+1+num], strlen(CH_LIST_D[now_group_id*3+1+num]), 1);
                            if (active_id == now_group_id*3+1+num)
                            {
                                show_a_star(temp_x);
                            }
                            temp_x += 2;
                        }
                        x = 2;
                        show_square_breakets(x);
                        trace_id = now_group_id*3+1;
                    }

                }
            }

            case UP_KEY:
            {
                if (x != 0)
                {
                    x-=2;
                    show_square_breakets(x);
                }
                else
                {
                    if (now_group_id > 0)
                    {
                        now_group_id--;
                        num  = 0;
                        temp_x = 2;
                        while(num < 3)
                        {
                            if ((now_group_id*3+1+num) > count)
                                break;
                            show_strings(temp_x, y, CH_LIST_D[now_group_id*3+1+num], strlen(CH_LIST_D[now_group_id*3+1+num]), 1);
                            if (active_id == now_group_id*3+1+num)
                            {
                                show_a_star(temp_x);
                            }
                            temp_x += 2;
                        }
                        x = 2;
                        show_square_breakets(x);    
                    }
                    
                }

            }
            case ENTER_KEY:
            case RIGHT_KEY:
            {
                show_a_star(x);
                
                char buf[2] = {0};
                sprintf(buf, "%d", trace_id);
                SET_CHANNEL_ID(buf);    
            }
            case LEFT_KEY:
            {
                return 0;
            }           
        }
    }
    return 0;
}




#if 0
//一级目录       1.主菜单显示
int IPD5000_MAIN_MENU_SHOW(void)
{
    printf("this is IPD5000\n");
    save_display_info_D();
    
    u8 count = sizeof(MAIN_MENU_strings_D)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    if (recv_init_D() != 0)
    {
        printf("recv_init_D() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_strings_D[0], strlen(MAIN_MENU_strings_D[0]) ); 
    show_menu_info_D(y, 1, MAIN_MENU_SHOWWING_D, MAIN_MENU_strings_D, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_D();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_D(count, param, MAIN_MENU_SHOWWING_D, MAIN_MENU_strings_D, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[1]) // VEDIO SELECET
                    {
                        VIDEO_IN_SELECT_SHOW_D();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[2]) // IP SETTING
                    {
                        IP_SETTING_MENU_SHOW_D();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[3]) // HDCP SETTING
                    {
                        HDCP_SHOW_D();
                        break;
                    }
    
                    if ( MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[4]) //  VIDEO OUT RES
                    {
                        VIDEO_OUT_RES_SHOW_D();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[5]) // FIRMWARE INFO
                    {
                        FIRMWARE_INFO_SHOW_D();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[6]) // DEVICE STATUS
                    {
                        DEVICE_STATUS_SHOW_D();
                        break;
                    }
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_D[i], strlen(MAIN_MENU_SHOWWING_D[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_D[i]) != 1) 
                    {
                        move_limit_D = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                break;
            }
        }
    }
}
#endif


#if 0
//二级目录      2.1 IP SETTING菜单栏显示
void IP_SETTING_MENU_SHOW_D(void)
{
    u8 count = sizeof(IP_SET_strings_D)/(sizeof(char*)); //IP_SET_strings的元素个数
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, IP_SET_strings_D[0], strlen(IP_SET_strings_D[0]) ); 
    show_menu_info_D(y, 1, IP_SET_SHOWWING_D, IP_SET_strings_D, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, IP_SET_SHOWWING_D, IP_SET_strings_D, key);
                break;
            }

            case RIGHT_KEY:
            case ENTER_KEY:
            {
                //LAN1
                if (param.x == 2)
                {
                    LAN_MODE_MENU_SHOW_D(LAN1_ID_D);
                }

                //LAN2
                if (param.x == 4)
                {
                    LAN_MODE_MENU_SHOW_D(LAN2_ID_D);
                }   

                clear_whole_screen();
                //从子目录出来，继续显示
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, IP_SET_SHOWWING_D[i], strlen(IP_SET_SHOWWING_D[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(IP_SET_SHOWWING_D[i]) != 1) 
                    {
                        move_limit_D = i*2;
                        break;
                    }
                }
                
                break;
            }
        
            case LEFT_KEY: 
            {  
                return;
            }
        }
    } 
    
}


//三级目录 LAN MODE 菜单栏显示
void LAN_MODE_MENU_SHOW_D(int lan_id) //DHCP OR STATIC
{
    int i = 0;
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    
    int DHCP_status = -1;
    get_DHCP_status(lan_id, &DHCP_status);
    
    clear_whole_screen();
    show_strings(0, y, LAN_MODE_strings_D[0], strlen(LAN_MODE_strings_D[0]), 0); 
    show_menu_info_D(y, 1, LAN_MODE_SHOWWING_D, LAN_MODE_strings_D, 2);
    show_square_breakets(x);

    //显示status： DHCP STATIS 
    show_a_star(4 - (2 * DHCP_status)); // x = 2 DHCP, x = 4 STATIC 

    int key = 0;
    while (1)
    {       
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            {
                if (x == 2)
                {
                    x = 4;
                    show_square_breakets(x);
                }
                break;
            }
            
            case UP_KEY:
            {
                if (x == 4)
                {
                    x = 2;
                    show_square_breakets(x);
                }
                
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                if (x == 2)
                {
                    show_a_star(x);         
                    set_DHCP_status(lan_id);
                    LAN_OPTION_SHOW_D(lan_id, 1);
                }
                
                if (x == 4)             //选择static
                {                   
                    LAN_OPTION_SHOW_D(lan_id, 0);
                }
                //继续显示这一级目录
                get_DHCP_status(lan_id, &DHCP_status);
                clear_whole_screen();
                show_strings(0, y, LAN_MODE_strings_D[0], strlen(LAN_MODE_strings_D[0]), 0); 
                show_menu_info_D(y, 1, LAN_MODE_SHOWWING_D, LAN_MODE_strings_D, 2);
                show_a_star(4 - (2 * DHCP_status));
                
                show_square_breakets(x);

                break;
            }
            
            case LEFT_KEY:
            {
                return;
            }
            
        }
        
    }
    
}


//四级目录 
static void LAN_OPTION_SHOW_D(int lan_id, int enable) //ip mask gateway
{   
    int y = 16;
    int x = 2; //方括号位置

    u8 i, begin_num = 1;

    clear_whole_screen();
    show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]), 0);
    for (i = 1; i < 4; i++)
    {
        show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]), 1);
    }
    show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            {
                x += 2;
                if ( x <= 6)
                {
                    show_square_breakets(x);
                }
                else
                    x = 6;
                
                break;
            }
            
            case UP_KEY:
            {
                x -= 2;
                if (x >= 2)
                {
                    show_square_breakets(x);
                }
                else
                    x = 2;
                
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                get_ip(lan_id, net_info_D[lan_id*3], net_info_D[lan_id*3 +1], net_info_D[lan_id*3 +2]);

                //x是坐标，x=2:ip, x=4:mask, x=6:gateway
                if (enable == 1)
                    DHCP_LAN_INFO_SHOW_D(lan_id, x/2, net_info_D[lan_id*3 + (x/2-1)], strlen(net_info_D[lan_id*3 + (x/2-1)]));
                else
                    LAN_INFO_SET_D(lan_id, x/2, net_info_D[lan_id*3 + (x/2-1)], strlen(net_info_D[lan_id*3 + (x/2-1)]));

                //子目录出来，继续显示这一级目录
                clear_whole_screen();
                int i;
                show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]), 0);
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]), 1); 
                }
                
                show_square_breakets(x);

                break;
            }
                
            case LEFT_KEY: //返回上一级目录
            {
                return;
            }
        }

    }
    
}

//五级目录 DHCP IP SHOW
static void DHCP_LAN_INFO_SHOW_D(int lan_id, int offset, char *string, u8 lenth)
{
    int x = 0, y = 16, y1 = 0;
    int key = 0;
    clear_whole_screen();
    show_strings(x, y, LAN_OPTION_strings_D[offset], strlen(LAN_OPTION_strings_D[offset]), 0); //显示标题
    show_strings(x+2, y1, string, strlen(string), 1);
    
    while (1)
    {
        key = recv_key_info_D();        
        if (key == LEFT_KEY)
        {
            break;
        }
    }   
}

//五级目录 LAN INFO set
static void LAN_INFO_SET_D(int lan_id, u8 offset, char *string, u8 lenth)
{
    u8 x = 0, y = 16, y1 = 0;
    int i = 0;
    int key = 0;

    clear_whole_screen();
    show_strings(x, y, LAN_OPTION_strings_D[offset], strlen(LAN_OPTION_strings_D[offset]), 0); //显示标题
    show_strings(x+2, y1, string, strlen(string), 1);
    show_a_char(x+2, y1, string[0], 1, 1);
    
    while (1)
    {   
        key = recv_key_info_D();

        switch (key)
        {
            case DOWN_KEY:
            {       
                if ( (i%4) == 0) //0,4,8,12
                {
                    if (string[i] < '2')
                    {
                        string[i]++;
                    }
                    else
                    {
                        string[i] = '0';
                    }

                }
                else
                {
                    if (string[i-(i%4)] == '2' ) //应为IP最高是255，如果它前面的是2了那么后面两位数字不能超过5
                    {
                        if (string[i] < '5')
                        {
                            string[i]++;
                        }
                        else
                        {
                            string[i] = '0';
                        }
                    }
                    else
                    {
                        if (string[i] < '9')
                        {
                            string[i]++;
                        }
                        else
                        {
                            string[i] = '0';
                        }
                    }
                    
                }
                
                show_a_char(x+2, i*8, string[i], 1, 1);//1

                break;
            }
            
            case UP_KEY:
            {
                if ( (i%4) == 0) //0,4,8,12
                {
                    if (string[i] > '0')
                    {
                        string[i]--;
                    }
                    else
                    {
                        string[i] = '2';
                    }

                }
                else
                {
                    if (string[i-(i%4)] == '2' ) //应为IP最高是255，如果它前面的是2了那么后面两位数字不能超过5
                    {
                        if (string[i] > '0')
                        {
                            string[i]--;
                        }
                        else
                        {
                            string[i] = '5';
                        }
                    }
                    else
                    {
                        if (string[i] > '0')
                        {
                            string[i]--;
                        }
                        else
                        {
                            string[i] = '9';
                        }
                    }
                    
                }
                
                show_a_char(x+2, i*8, string[i], 1); //1

                break;
            }

            case RIGHT_KEY:  //总共15位
            {
                show_a_char(x+2, i*8, string[i], 0, 1); //取消原来位置的光标
                i++;
                if (i == 3 || i == 7 || i == 11)  //跳过'.',
                    i++;
                
                if (i > 14)
                    i = 0;
                show_a_char(x+2, i*8, string[i], 1, 1); //在新位置上显示光标 //1

                break;
            }
            
            case LEFT_KEY:
            {
                show_a_char(x+2, i*8, string[i], 0, 1); 
                i--;
                
                if (i == 3 || i == 7 || i == 11)  //跳过'.',
                    i--;
                
                if (i < 0)
                    i = 14;
                show_a_char(x+2, i*8, string[i], 1, 1);//1

                break;
            }

            case ENTER_KEY:
            {
                show_a_char(x+2, i*8, string[i], 0, 1); //取消原来位置的光标

                //通知IP已经被改了
                set_ip(lan_id, net_info_D[lan_id*3], net_info_D[lan_id*3+1], net_info_D[lan_id*3+2]);
                return; //返回到上一级目录
            }
            
        }
        
    }
}
#endif

#if 0
int show_current_VIDEO_SELECT()
{
    int i;
    int channel_num;
    get_current_voide_channel(&channel_num);
    for(i = 1; i < 4; i++)
    {
        if (VIDEO_IN_SHOWWING_D[i] == SAVE_VIDEO_SELECT_D[channel_num])
        {
            show_a_star(i*2);
            break;
        }
    }
}

//level 3 CHANNEL SELECT
void CHANNEL_SELECT_SHOW_D(void)
{
    u8 count = get_elem_num_D(SAVE_VIDEO_SELECT_D, MIN_SIZE_D+1);
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    //更新video select
    save_VIDEO_SELECT_info_D(); 
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, SAVE_VIDEO_SELECT_D[0], strlen(SAVE_VIDEO_SELECT_D[0]), 0); 
    show_menu_info_D(y, 1, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_SELECT_D, count>4? 3 : count-1);
    show_square_breakets(x);
    //每次进来要查询当前频道
    show_current_VIDEO_SELECT();

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_SELECT_D, key);
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                show_a_star(param.x);
                select_voide_channel(VIDEO_IN_SHOWWING_D[param.x/2] - '0');  //频道号是i                    
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case LEFT_KEY:
            {
                return; //返回上一级目录
            }
        }

    }
    
}
#endif

#if 0

//二级目录，
void VIDEO_IN_SELECT_SHOW_D(void)
{
    u8 count = get_elem_num_D(SAVE_VIDEO_SELECT_D, MIN_SIZE_D+1);
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    //更新video select
    save_VIDEO_SELECT_info_D(); 
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, SAVE_VIDEO_SELECT_D[0], strlen(SAVE_VIDEO_SELECT_D[0]), 0); 
    show_menu_info_D(y, 1, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_SELECT_D, count>4? 3 : count-1);
    show_square_breakets(x);
    //每次进来要查询当前频道
    show_current_VIDEO_SELECT();

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_SELECT_D, key);
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                show_a_star(param.x);
                select_voide_channel(VIDEO_IN_SHOWWING_D[param.x/2] - '0');  //频道号是i                    
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case LEFT_KEY:
            {
                return; //返回上一级目录
            }
        }

    }
    
}

//二级目录
void VIDEO_OUT_RES_SHOW_D()
{
    u8 count = 2;
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    //获取当前的VIDEO OUT
    get_current_VIDEO_OUT_info_D();
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, VIDEO_OUT_string_D[0], strlen(VIDEO_OUT_string_D[0]), 0); 
    show_strings(2, y, VIDEO_OUT_string_D[1], strlen(VIDEO_OUT_string_D[1]), 1);
    //show_square_breakets(x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case LEFT_KEY:
            {
                return; //返回上一级目录
            }
        }
    }

}

//二级目录，DHCP show
void HDCP_SHOW_D() //仅展示
{
    u8 count = 3;
    int i = 0;
    u8 y = 16;
    int key = 0;
    clear_whole_screen();
    
    //查询ON OFF
    int status = -1;
    get_HDCP_status(&status);
    
    show_strings(0, y, HDCP_strings_D[i], strlen(HDCP_strings_D[i]), 0);
    for (i = 1; i < count; i++)
    {
        show_strings(i*2, y, HDCP_strings_D[i], strlen(HDCP_strings_D[i]), 1);
    }

    show_a_star(4 - 2*status );
    
    
    while (1)
    {
        key = recv_key_info_D();
        
        if (key == LEFT_KEY)
        {
            break;
        }
        //其他键，无效。
    }
}

//二级目录
void FIRMWARE_INFO_SHOW_D() //仅展示
{
    u8 count = get_elem_num_D(FIRMWARE_strings_D, MIN_SIZE_D+1);
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    save_FIREWARE_info_D(); 
    clear_whole_screen();
    show_strings(0, y, FIRMWARE_strings_D[0], strlen(FIRMWARE_strings_D[0]), 0); 
    show_menu_info_D(y, 1, FIRMWARE_SHOWWING_D, FIRMWARE_strings_D, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, FIRMWARE_SHOWWING_D, FIRMWARE_strings_D, key);
                break;
            }
            
            //右键无效
            
            case LEFT_KEY:  //返回上一级目录
            {
                return;
            }
        }
    }

}
#endif

#if 0
//二级目录
void DEVICE_STATUS_SHOW_D() //仅展示
{
    u8 count = get_elem_num_D(DEVICE_STATUS_strings_D, MIN_SIZE_D+1);
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    clear_whole_screen();
    show_strings(0, y, DEVICE_STATUS_strings_D[0], strlen(DEVICE_STATUS_strings_D[0]), 0); 
    show_menu_info_D(y, 1, DEVICE_STATUS_SHOWWING_D, DEVICE_STATUS_strings_D, count>4? 3 : count-1);
    //show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_D();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_D(count, param, DEVICE_STATUS_SHOWWING_D, DEVICE_STATUS_strings_D, key);
                break;
            }
            //右键无效处理

            case LEFT_KEY:  //返回上一级目录
            {
                return;
            }
        }
    }
    
}
#endif


