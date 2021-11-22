
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>

#include "oled.h"
#include "IPE5000P_info_setting.h"
#include "msg_queue.h"
#include "send_p3k_cmd.h"
#define _GNU_SOURCE

#define SIZE_P 255
#define MIN_SIZE_P 20

#define LAN1_ID_P   0
#define LAN2_ID_P   1
#define CHANNEL_MAP "/data/configs/kds-7/channel/channel_map.json"

pthread_mutex_t g_lock_P;

time_t last_change_time_P = 0;

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
<<<<<<< HEAD


=======


>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
char *blank_P = " ";

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
char video_select_buf[MIN_SIZE_P][SIZE_P];
char firmware_buf[MIN_SIZE_P][SIZE_P];
#endif

// LEVEL 1
const char* MAIN_MENU_LIST_P[] = {
    "MAIN MENU", "DEV STATUS", "DEV INFO", "DEV SETTINGS",
};
// END LEVEL 1


// LEVEL 2 
const char* DEV_STATUS_LIST_P[] = {
    "DEV STATUS", "LAN1 STATUS", "LAN2 STATUS", "HDMI STATUS", "CH DEFINE", "TEMPERATURE",
};

char FW_VERSION_P[20] = {0};
char BL_VERSION_P[20] = {0};
char HW_VERSION_P[20] = {0};
const char* DEV_INFO_LIST_P[] = {
    "DEV INFO", (const char*)FW_VERSION_P, (const char*)BL_VERSION_P, (const char*)HW_VERSION_P,
};

const char* DEV_SETTINGS_LIST_P[] = {
    "DEV SETTINGS", "INPUT SETTING", "EDID SETTING", "HDCP SETTING", "CH DEFINE",
};
// END LEVEL 2


// LEVEL 3
char RESOL_BUF_P[20]       = {0};
char HDCP_STATUS_BUF_P[20] = {0};
const char* HDMI_STATUS_LIST_P[] = {
    "HDMI STATUS", (const char*)RESOL_BUF_P, (const char*)HDCP_STATUS_BUF_P,
<<<<<<< HEAD
};

const char* INPUT_VIDEO_SOURCE_TYPE_P[] = {
    "INPUT SETTING", "HDMI IN1", "HDMI IN2", "USB_C IN",
};

=======
};

const char* INPUT_VIDEO_SOURCE_TYPE_P[] = {
    "INPUT SETTING", "HDMI IN1", "HDMI IN2", "USB_C IN",
};

>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
char EDID_BUF_P[10][20] = {"EDID SETTING",{0},{0},{0},{0},{0},{0},{0},{0},{0}};

const char *EDID_LIST_P[] = {(const char *)EDID_BUF_P[0], (const char *)EDID_BUF_P[1],(const char *)EDID_BUF_P[2],(const char *)EDID_BUF_P[3],(const char *)EDID_BUF_P[4],
                       (const char *)EDID_BUF_P[5],(const char *)EDID_BUF_P[6],(const char *)EDID_BUF_P[7],(const char *)EDID_BUF_P[8],(const char *)EDID_BUF_P[9]};

const char *HDCP_LIST_P[] = {"HDCP SETTING", "ON", "OFF"};

int CH_TATOL_NUM_P = 0;
char *CH_LIST_P[100] = {"NO SIGNAL",};

// END LEVEL 3


#if 0
const char* MAIN_MENU_strings_P[] = {
    "MAIN MENU", "VIDEO SELECT", "IP SETTING", "HDCP SETTING",
    "VIDEO OUT RES", "FIRMWARE INFO", "DEVICE STATUS",
};

const char* IP_SET_strings_P[] = {
    "IP SETTING", "LAN1 SETTING", "LAN2 SETTING",
};
    


const char* LAN_OPTION_strings_P[] = {
    "LAN INFO", "LAN ADDR", "LAN MASK", "LAN GATEWAY",
};

const char* HDCP_strings_P[] = {
    "HDCP SETTING", "HDCP ON", "HDCP OFF",
};


const char* FIRMWARE_strings_P[MIN_SIZE_P+1]        = {"FIRMWARE INFO",};
const char* DEVICE_STATUS_strings_P[MIN_SIZE_P+1]   = {"DEVICE STATUS",};
const char* VIDEO_OUT_string_P[2]       = {"VIDEO OUT RES", " "};

//用来存储网络配置信息: 网口1：ip, mask gateway.网口2: ip, mask gateway 共6个
char net_info_P[6][16] = {{0}, {0}, {0}, {0}, {0}, {0}};

//记录正在屏幕显示的字符串，一个屏幕最多显示4行.
const char *MAIN_MENU_SHOWWING_P[4]     =   {NULL, NULL, NULL, NULL}; 
const char *IP_SET_SHOWWING_P[4]        =   {NULL, NULL, NULL, NULL};

const char *LAN1_OPTION_SHOWWING_P[4]   =   {NULL, NULL, NULL, NULL};
const char *VIDEO_OUT_SHOWWING_P[4]     =   {NULL, NULL, NULL, NULL};
const char *FIRMWARE_SHOWWING_P[4]      =   {NULL, NULL, NULL, NULL};
const char *DEVICE_STATUS_SHOWWING_P[4] =   {NULL, NULL, NULL, NULL};
#endif


const char* SAVE_VIDEO_SELECT_P[MIN_SIZE_P+1]    = {"CHANNEL SEL",}; 
const char *VIDEO_IN_SHOWWING_P[4]               =    {NULL, NULL, NULL, NULL};

//Records the string being displayed
const char *MAIN_MENU_SHOWWING_P[4]       =    {NULL, NULL, NULL, NULL}; 
const char *DEV_STATUS_SHOWWING_P[4]      =    {NULL, NULL, NULL, NULL};
const char *DEV_INFO_SHOWWING_P[4]        =    {NULL, NULL, NULL, NULL};
const char *DEV_SETTINGS_SHOWWING_P[4]    =    {NULL, NULL, NULL, NULL};
const char *INPUT_VIDEO_SHOWWING_P[4]     =    {NULL, NULL, NULL, NULL};
const char *HDMI_STATUS_SHOWWING_P[4]     =    {NULL, NULL, NULL, NULL}; 
const char *EDID_SHOWWING_P[4]            =    {NULL, NULL, NULL, NULL};

//记录中括号的X坐标位置
static int move_limit_P = 0; 

#if 0
int save_LAN_info_P()
{
    int i = 0;
    for (i = 0; i < 6; i++)
    {
        net_info_P[i][15] = '\0';
    }
    get_ip(LAN1_ID_P, net_info_P[0], net_info_P[1], net_info_P[2]);
    get_ip(LAN2_ID_P, net_info_P[3], net_info_P[4], net_info_P[5]);
    
}

int save_VIDEO_SELECT_info_P()
{
    int i = 0, err = 0;
    memset(video_select_buf, 0, MIN_SIZE_P*SIZE_P);

    err = VIDEO_LIST(video_select_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_P; i++)
        {
            strcpy(video_select_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_P+1; i++)
    {
        SAVE_VIDEO_SELECT_P[i] = video_select_buf[i-1];
    }
}

int save_FIREWARE_info_P()
{
    int i = 0, err = 0;
    memset(firmware_buf, 0, MIN_SIZE_P*SIZE_P);

    err = get_FIRMWARE_INFO(firmware_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_P; i++)
        {
            strcpy(firmware_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_P+1; i++)
    {
        FIRMWARE_strings_P[i] = firmware_buf[i-1];
    }
}

int save_PEVICE_STATUS_info_P()
{
    int i = 0;
    int status = -1;
    get_PEVICE_STATUS(&status);
    DEVICE_STATUS_strings_P[1] = device_status[status+1];
}

int get_current_VIDEO_OUT_info_P()
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
    VIDEO_OUT_string_P[1] = video_out[s];

}
#endif
/*
void save_display_info_P()
{
    
    init_p3k_client("127.0.0.1", 6001);
    
    save_LAN_info_P();
    save_VIDEO_SELECT_info_P();
    save_FIREWARE_info_P();
    save_PEVICE_STATUS_info_P();
    get_current_VIDEO_OUT_info_P();
}

void show_current_edid_P()
{
    int i;
    char edid_type;
    edid_type = GET_EDID();
    
    for (i = 1; i < 4; i++ )
    {
        if (EDID_SHOWWING_P[i][0] == edid_type);
        {
            show_a_star(i*2);
            break;
        }
    }
}
*/

int file_is_changed_P(const char* filename)
{
    int err = -1;
    struct stat state;
    
    if (filename == NULL)
        return -1;
    
    if (access(filename, F_OK|R_OK) == -1)
        return -1;

    if (stat(filename, &state) != 0)
        return -1;

    if (last_change_time_P == state.st_mtime)
    {
        return 0;
    }
    else
    {
        last_change_time_P = state.st_mtime;
        return 1;
    }
}
<<<<<<< HEAD

u8 get_elem_num_P(const char **buf, int num)
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
void show_menu_info_P(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
{
    //取剩下的显示，最多取三个
    u8 x = 0;
    int i; 
    
    dest[0] = src[0];

    if (count == 0)
    {
        move_limit_P = 2;
        return;
    }   
    for (i = 0; i < count; i++)
    {
        dest[1+i] = src[begin_elem+i];
    }
    
    move_limit_P = i*2; //选择框向下最大移动坐标

    for (i+=1; i < 4; i++)
    {
        dest[i] = blank_P;
    }
    
    //显示
    for (i = 1; i < 4; i++)
    {
        show_strings(x+2*i, y, dest[i], strlen(dest[i]), 1);
    }
=======
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69

u8 get_elem_num_P(const char **buf, int num)
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

<<<<<<< HEAD
//响应 下行键，上行键
info_param down_up_respond_P(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
{
=======
/*
    y: 显示起始列坐标,要取8的整数倍, 0, 8, 16 .... 8*15
    begin_num: 要显示src中要元素起始位
    dest: 记录src哪些元素被显示
    src: 从src中拿取字符串显示
    count: 从src的拿去元素的个数, 最多只能拿三个
*/
void show_menu_info_P(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
{
    //取剩下的显示，最多取三个
    u8 x = 0;
    int i; 
    
    dest[0] = src[0];

    if (count == 0)
    {
        move_limit_P = 2;
        return;
    }   
    for (i = 0; i < count; i++)
    {
        dest[1+i] = src[begin_elem+i];
    }
    
    move_limit_P = i*2; //选择框向下最大移动坐标

    for (i+=1; i < 4; i++)
    {
        dest[i] = blank_P;
    }
    
    //显示
    for (i = 1; i < 4; i++)
    {
        show_strings(x+2*i, y, dest[i], strlen(dest[i]), 1);
    }

}

//响应 下行键，上行键
info_param down_up_respond_P(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
{
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
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
            if (x > move_limit_P) //X坐标超过最大值，取最大值
            {
                x = move_limit_P;
                
                if (count > 4)//要超过4行才能翻页，
                {
                    //p被上行键更改了 或者count <= 7时，p不会被上行键更改，但页面已经被向上翻页了，通过MAIN_MENU_SHOWWING[3],MAIN_MENU_strings_P[3]来判断是否是最后一页。
                    if ( (p + 3 < count) || (strcmp(showwing_strings[3], src_string[3]) == 0))
                    {
                        last_page = 0;  
                    }
                    
                    if ((count - p > 0) && last_page == 0) //还有也可以翻，且没有翻到最后一页
                    {
                        if (p + 3 < count) 
                        {   
                            clear_three_line();
                            show_menu_info_P(y, p, showwing_strings, src_string, 3); //数组是从0开始
                            p += 3;
                        }
                        else 
                        {
                            clear_three_line();
                            show_menu_info_P(y, p, showwing_strings, src_string, count-p);
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
                            show_menu_info_P(y, p, showwing_strings, src_string, 3);
                        }
                        else //p==4
                        {
                            clear_three_line();
                            show_menu_info_P(y, p-3, showwing_strings, src_string, 3);
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

static int recv_init_P()
{
    int err = 0;
    pthread_mutex_init(&g_lock_P, NULL);
<<<<<<< HEAD

	err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
	
=======
    
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
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

<<<<<<< HEAD
    chmod("/var/run/send_key_pressed_info", S_IXUSR|S_IXGRP|S_IXOTH); 

    return 0;
}

static int recv_key_info_P()
{
    int err = 0;
    struct msg_buf msg;

    pthread_mutex_lock(&g_lock_P);
    err = msg_recv_state(&msg);
    if (err == -1)
    {
        printf("msg_recv_state fail");
        return -1;
    }
    pthread_mutex_unlock(&g_lock_P);
    
    return msg.mtext[0];
}

// The first level
int IPE5000P_MAIN_MENU_SHOW(void)
{
    printf("this is IPE5000P\n");
    init_p3k_client("127.0.0.1", 6001);
    
    u8 count = sizeof(MAIN_MENU_LIST_P)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    if (recv_init_P() != 0)
    {
        printf("recv_init_P() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_LIST_P[0], strlen(MAIN_MENU_LIST_P[0]), 1);
    show_menu_info_P(y, 1, MAIN_MENU_SHOWWING_P, MAIN_MENU_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, MAIN_MENU_SHOWWING_P, MAIN_MENU_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[1]) // DEV STATUS
                    {
                        DEV_STATUS_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[2]) // DEV INFO
                    {
                        DEV_INFO_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[3]) // DEV SETTINGS
                    {
                        DEV_SETTINGS_P();
                        break;
                    }
    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]), 1);
                for (i = 1; i < count; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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
=======
    chmod("/var/run/send_key_pressed_info", S_IXUSR|S_IXGRP|S_IXOTH); //

    return 0;
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
}

// the second level: 1.DEV STATUS
static int DEV_STATUS_P()
{
<<<<<<< HEAD
    u8 count = sizeof(DEV_STATUS_LIST_P)/(sizeof(char*)); //DEV_STATUS_P的元素个数
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
    show_strings(0, y, DEV_STATUS_LIST_P[0], strlen(DEV_STATUS_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_STATUS_SHOWWING_P, DEV_STATUS_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, DEV_STATUS_SHOWWING_P, DEV_STATUS_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[1]) // LAN1 STATUS 
                    {
                        LAN_STATUS_P(LAN1_ID_P);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[2]) // LAN1 STATUS 
                    {
                        LAN_STATUS_P(LAN2_ID_P);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[3]) // HDMI STATUS
                    {
                        HDMI_STATUS_P();
                        break;
                    }
    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[4]) // CHANNEL SEL
                    {
                        CH_DEFINE_P();
                        break;
                    }
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[5]) // TEMPERATURE
                    {
                        TEMPERATURE_P();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_STATUS_SHOWWING_P[i], strlen(DEV_STATUS_SHOWWING_P[i]), 1);
                for (i = 1; i < 4; i++)
                {
                    if (DEV_STATUS_SHOWWING_P[i] != NULL)
                        show_strings(i*2, y, DEV_STATUS_SHOWWING_P[i], strlen(DEV_STATUS_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_STATUS_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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
static void LAN_STATUS_P(int interface_id)
{
    char addr[15] = {0}, mask[15] = {0}, gateway[15] = {0};
    GET_IP(interface_id, addr, mask, gateway);

    clear_whole_screen();
    if (interface_id == LAN1_ID_P)
        show_strings(0, 16, "LAN 1 STATUS", strlen("LAN 1 STATUS"), 1);
    else if (interface_id == LAN2_ID_P)
        show_strings(0, 16, "LAN 2 STATUS", strlen("LAN 2 STATUS"), 1);
    
    show_strings(2, 8, addr, 15, 1);
    show_strings(4, 8, mask, 15, 1);
    show_strings(6, 8, gateway, 15, 1);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        
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
static int HDMI_STATUS_P()
{
    //1.resolution
    GET_ACTUAL_RESOLUTION(RESOL_BUF_P);
    
    //2.HDCP STATUS
    GET_HDCP_STATUS(HDCP_STATUS_BUF_P);

    u8 count = sizeof(HDMI_STATUS_LIST_P)/(sizeof(char*));
=======
    int err = 0;
    struct msg_buf msg;

    pthread_mutex_lock(&g_lock_P);
    err = msg_recv_state(&msg);
    if (err == -1)
    {
        printf("msg_recv_state fail");
        return -1;
    }
    pthread_mutex_unlock(&g_lock_P);
    
    return msg.mtext[0];
}

// The first level
int IPE5000P_MAIN_MENU_SHOW(void)
{
    printf("this is IPE5000P\n");
    init_p3k_client("127.0.0.1", 6001);
    
    u8 count = sizeof(MAIN_MENU_LIST_P)/(sizeof(char*));
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

<<<<<<< HEAD
    clear_whole_screen();
    show_strings(0, y, HDMI_STATUS_LIST_P[0], strlen(HDMI_STATUS_LIST_P[0]) ,1); 
    show_menu_info_P(y, 1, HDMI_STATUS_LIST_P, HDMI_STATUS_LIST_P, count>4 ? 3 : count-1);
=======
    if (recv_init_P() != 0)
    {
        printf("recv_init_P() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_LIST_P[0], strlen(MAIN_MENU_LIST_P[0]), 1);
    show_menu_info_P(y, 1, MAIN_MENU_SHOWWING_P, MAIN_MENU_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
<<<<<<< HEAD
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
        
}

// 1.4 DEV STATUS -> CHANNEL SELECT
static int CH_DEFINE_P()
{
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);
    

    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE") ,1);
    show_strings(2, 16, channel_id, strlen(channel_id) ,1);

=======
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, MAIN_MENU_SHOWWING_P, MAIN_MENU_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[1]) // DEV STATUS
                    {
                        DEV_STATUS_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[2]) // DEV INFO
                    {
                        DEV_INFO_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_LIST_P[3]) // DEV SETTINGS
                    {
                        DEV_SETTINGS_P();
                        break;
                    }
    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]), 1);
                for (i = 1; i < count; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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

// the second level: 1.DEV STATUS
static int DEV_STATUS_P()
{
    u8 count = sizeof(DEV_STATUS_LIST_P)/(sizeof(char*)); //DEV_STATUS_P的元素个数
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
    show_strings(0, y, DEV_STATUS_LIST_P[0], strlen(DEV_STATUS_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_STATUS_SHOWWING_P, DEV_STATUS_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();
<<<<<<< HEAD
        
        switch (key)
        {
            case LEFT_KEY:
            {
=======

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, DEV_STATUS_SHOWWING_P, DEV_STATUS_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[1]) // LAN1 STATUS 
                    {
                        LAN_STATUS_P(LAN1_ID_P);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[2]) // LAN1 STATUS 
                    {
                        LAN_STATUS_P(LAN2_ID_P);
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[3]) // HDMI STATUS
                    {
                        HDMI_STATUS_P();
                        break;
                    }
    
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[4]) // CHANNEL SEL
                    {
                        CH_DEFINE_P();
                        break;
                    }
                    if (DEV_STATUS_SHOWWING_P[param.x/2] == DEV_STATUS_LIST_P[5]) // TEMPERATURE
                    {
                        TEMPERATURE_P();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_STATUS_SHOWWING_P[i], strlen(DEV_STATUS_SHOWWING_P[i]), 1);
                for (i = 1; i < 4; i++)
                {
                    if (DEV_STATUS_SHOWWING_P[i] != NULL)
                        show_strings(i*2, y, DEV_STATUS_SHOWWING_P[i], strlen(DEV_STATUS_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_STATUS_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                return 0;
            }
        }
    }
<<<<<<< HEAD

}

// 1.5 DEV STATUS -> TEMPERATURE
static int TEMPERATURE_P()
{
    int err = -1;
    char temp[20] = {0};
    
    err = GET_TEMPERATURE(temp);
    if (err == -1)
        return -1;

    clear_whole_screen();
    show_strings(0, 16, "TEMPERATURE", strlen("TEMPERATURE") ,1);
    show_strings(2, 16, temp, strlen(temp) ,1);
    
=======
    
}

// 1.1 DEV STATUS -> LAN SHOW
static void LAN_STATUS_P(int interface_id)
{
    char addr[15] = {0}, mask[15] = {0}, gateway[15] = {0};
    GET_IP(interface_id, addr, mask, gateway);

    clear_whole_screen();
    if (interface_id == LAN1_ID_P)
        show_strings(0, 16, "LAN 1 STATUS", strlen("LAN 1 STATUS"), 1);
    else if (interface_id == LAN2_ID_P)
        show_strings(0, 16, "LAN 2 STATUS", strlen("LAN 2 STATUS"), 1);
    
    show_strings(2, 8, addr, 15, 1);
    show_strings(4, 8, mask, 15, 1);
    show_strings(6, 8, gateway, 15, 1);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        
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
static int HDMI_STATUS_P()
{
    //1.resolution
    GET_ACTUAL_RESOLUTION(RESOL_BUF_P);
    
    //2.HDCP STATUS
    GET_HDCP_STATUS(HDCP_STATUS_BUF_P);

    u8 count = sizeof(HDMI_STATUS_LIST_P)/(sizeof(char*));
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
    show_strings(0, y, HDMI_STATUS_LIST_P[0], strlen(HDMI_STATUS_LIST_P[0]) ,1); 
    show_menu_info_P(y, 1, HDMI_STATUS_LIST_P, HDMI_STATUS_LIST_P, count>4 ? 3 : count-1);

>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
<<<<<<< HEAD
    
}


// 2. DEV INFO
static int DEV_INFO_P()
{
    u8 count = sizeof(DEV_INFO_LIST_P)/(sizeof(char*));
    
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
    GET_FW_VERSION(FW_VERSION_P);
    GET_BL_VERSION(BL_VERSION_P);
    GET_HW_VERSION(HW_VERSION_P);
    
    clear_whole_screen();
    show_strings(0, y, DEV_INFO_LIST_P[0], strlen(DEV_INFO_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_INFO_SHOWWING_P, DEV_INFO_LIST_P, count>4? 3 : count-1);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case LEFT_KEY:  //返回上一级目录
=======
        
}

// 1.4 DEV STATUS -> CHANNEL SELECT
static int CH_DEFINE_P()
{
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);
    

    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE") ,1);
    show_strings(2, 16, channel_id, strlen(channel_id) ,1);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();
        
        switch (key)
        {
            case LEFT_KEY:
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
            {
                return 0;
            }
        }
    }
<<<<<<< HEAD
}


// 3  DEV SETTINGS
static int DEV_SETTINGS_P()
{
    u8 count = sizeof(DEV_SETTINGS_LIST_P)/(sizeof(char*)); //DEV_STATUS_P的元素个数
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
    show_strings(0, y, DEV_SETTINGS_LIST_P[0], strlen(DEV_SETTINGS_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_SETTINGS_SHOWWING_P, DEV_SETTINGS_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
=======

}

// 1.5 DEV STATUS -> TEMPERATURE
static int TEMPERATURE_P()
{
    int err = -1;
    char temp[20] = {0};
    
    err = GET_TEMPERATURE(temp);
    if (err == -1)
        return -1;

    clear_whole_screen();
    show_strings(0, 16, "TEMPERATURE", strlen("TEMPERATURE") ,1);
    show_strings(2, 16, temp, strlen(temp) ,1);
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
<<<<<<< HEAD
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, DEV_SETTINGS_SHOWWING_P, DEV_SETTINGS_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[1]) // INPUT SETTING 
                    {
                        INPUT_SETTING_P();
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[2]) // EDID SETTING 
                    {
                        EDID_SETTING_P();
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[3]) // HDCP SETTING
                    {
                        HDCP_SETTING_P();
                        break;
                    }
    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[4]) // CH DEFINE
                    {
                        CH_SELECT_P();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_SETTINGS_SHOWWING_P[0], strlen(DEV_SETTINGS_SHOWWING_P[0]), 1); 
                for (i = 1; i < 4; i++)
                {
                    if (DEV_SETTINGS_SHOWWING_P[i] != NULL)
                        show_strings(i*2, y, DEV_SETTINGS_SHOWWING_P[i], strlen(DEV_SETTINGS_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_SETTINGS_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
=======
            case LEFT_KEY:
            {
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                return 0;
            }
        }
    }
<<<<<<< HEAD

}

static int INPUT_SETTING_P()
{
    int err = -1;
    int type_i = -1;
    char type[100] = {0};

    
    err = GET_ENCODE_VIDEO_INPUT(type);
    if (err == -1)
        return -1;
    
    strcpy(type, "usb.3");
    if (strcasestr(type, "hdmi.1") != NULL)
    {
        type_i = 1;
    }
    else if (strcasestr(type, "hdmi.2") != NULL)
    {
        type_i = 2;
    }
    else if (strcasestr(type, "usb.3") != NULL)
    {
        type_i = 3;
    }
    
    u8 count = sizeof(INPUT_VIDEO_SOURCE_TYPE_P)/(sizeof(char*));
    printf("count=%d\n", count);
    
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

=======
    
}


// 2. DEV INFO
static int DEV_INFO_P()
{
    u8 count = sizeof(DEV_INFO_LIST_P)/(sizeof(char*));
    
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

<<<<<<< HEAD
    clear_whole_screen();
    show_strings(0, y, INPUT_VIDEO_SOURCE_TYPE_P[0], strlen(INPUT_VIDEO_SOURCE_TYPE_P[0]), 1); 
    show_menu_info_P(y, 1, INPUT_VIDEO_SHOWWING_P, INPUT_VIDEO_SOURCE_TYPE_P, count>4? 3 : count-1);
    show_square_breakets(x);
    show_a_star(2*type_i);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case DOWN_KEY:  
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, INPUT_VIDEO_SHOWWING_P, INPUT_VIDEO_SOURCE_TYPE_P, key);
                show_square_breakets(param.x);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {           
                show_a_star(param.x);
                SET_ENCODE_VIDEO_INPUT(INPUT_VIDEO_SOURCE_TYPE_P[param.x/2]);
                break;              
            }
            case LEFT_KEY:
            {   
                return 0;
            }
        }

    }

    
}

static int EDID_SETTING_P()
{
    char buf[20] = {0};
    u8 count = GET_EDID_LIST(EDID_BUF_P);
    GET_EDID(buf);

    
    int p = 4;
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
=======
    //get device info
    GET_FW_VERSION(FW_VERSION_P);
    GET_BL_VERSION(BL_VERSION_P);
    GET_HW_VERSION(HW_VERSION_P);
    
    clear_whole_screen();
    show_strings(0, y, DEV_INFO_LIST_P[0], strlen(DEV_INFO_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_INFO_SHOWWING_P, DEV_INFO_LIST_P, count>4? 3 : count-1);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case LEFT_KEY:  //返回上一级目录
            {
                return 0;
            }
        }
    }
}


// 3  DEV SETTINGS
static int DEV_SETTINGS_P()
{
    u8 count = sizeof(DEV_SETTINGS_LIST_P)/(sizeof(char*)); //DEV_STATUS_P的元素个数
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;
<<<<<<< HEAD
        
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, EDID_LIST_P[0], strlen(EDID_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, EDID_SHOWWING_P, (const char **)EDID_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int i = 0;
    for (i = 1; i < 4; i++)
    {   
        if (strstr(EDID_SHOWWING_P[i], buf) != NULL)
        {
            show_a_star(2*i);
        }       
    }

    int key = 0; 
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, EDID_SHOWWING_P, EDID_LIST_P, key);
                for (i = 1; i < 4; i++)
                {   
                    if (strstr(EDID_SHOWWING_P[i], buf) != NULL)
                    {
                        show_a_star(2*i);
                    }       
                }
                break;
            }
            
            case RIGHT_KEY: //标记*号
            case ENTER_KEY:
            {
                show_a_star(param.x); 
                SET_EDID(EDID_SHOWWING_P[param.x/2][0]);
                break;
            }
            
            case LEFT_KEY:
            {
=======

    clear_whole_screen();
    show_strings(0, y, DEV_SETTINGS_LIST_P[0], strlen(DEV_SETTINGS_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, DEV_SETTINGS_SHOWWING_P, DEV_SETTINGS_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, DEV_SETTINGS_SHOWWING_P, DEV_SETTINGS_LIST_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[1]) // INPUT SETTING 
                    {
                        INPUT_SETTING_P();
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[2]) // EDID SETTING 
                    {
                        EDID_SETTING_P();
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[3]) // HDCP SETTING
                    {
                        HDCP_SETTING_P();
                        break;
                    }
    
                    if (DEV_SETTINGS_SHOWWING_P[param.x/2] == DEV_SETTINGS_LIST_P[4]) // CH DEFINE
                    {
                        CH_SELECT_P();
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_SETTINGS_SHOWWING_P[0], strlen(DEV_SETTINGS_SHOWWING_P[0]), 1); 
                for (i = 1; i < 4; i++)
                {
                    if (DEV_SETTINGS_SHOWWING_P[i] != NULL)
                        show_strings(i*2, y, DEV_SETTINGS_SHOWWING_P[i], strlen(DEV_SETTINGS_SHOWWING_P[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_SETTINGS_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                return 0;
            }
        }
    }
<<<<<<< HEAD
    
}

static int HDCP_SETTING_P()
{
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
=======

}

static int INPUT_SETTING_P()
{
    int err = -1;
    int type_i = -1;
    char type[100] = {0};

    
    err = GET_ENCODE_VIDEO_INPUT(type);
    if (err == -1)
        return -1;
    
    strcpy(type, "usb.3");
    if (strcasestr(type, "hdmi.1") != NULL)
    {
        type_i = 1;
    }
    else if (strcasestr(type, "hdmi.2") != NULL)
    {
        type_i = 2;
    }
    else if (strcasestr(type, "usb.3") != NULL)
    {
        type_i = 3;
    }
    
    u8 count = sizeof(INPUT_VIDEO_SOURCE_TYPE_P)/(sizeof(char*));
    printf("count=%d\n", count);
    
    int p = 4; 
    int y = 16;
    int x = 2; //方括号位置
    int last_page = 0;

>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

<<<<<<< HEAD
    int star_x = 2;
    char mode[20] = {0};
    GET_HDCP_MODE(mode);
    if (mode[0] == '1')
    {
        star_x = 2;
    }
    else if (mode[0] == '0')
    {
        star_x = 4;
    }

    clear_whole_screen();  //新一级的目录，清屏
    int i = 0;
    for (i = 0; i < 3; i++)
    {
        show_strings(i*2, y, HDCP_LIST_P[i], strlen(HDCP_LIST_P[i]), 1);
    }
    show_square_breakets(star_x);
    show_a_star(star_x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            {
            	star_x += 2;
                if (star_x > 4)
                {
                    star_x = 4;
                }
                show_square_breakets(star_x);
                break;
            }
            case UP_KEY:
            {
				star_x -= 2;
                if (star_x < 2)
                {
                    star_x = 2;
                }
				show_square_breakets(star_x);
=======
    clear_whole_screen();
    show_strings(0, y, INPUT_VIDEO_SOURCE_TYPE_P[0], strlen(INPUT_VIDEO_SOURCE_TYPE_P[0]), 1); 
    show_menu_info_P(y, 1, INPUT_VIDEO_SHOWWING_P, INPUT_VIDEO_SOURCE_TYPE_P, count>4? 3 : count-1);
    show_square_breakets(x);
    show_a_star(2*type_i);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case DOWN_KEY:  
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, INPUT_VIDEO_SHOWWING_P, INPUT_VIDEO_SOURCE_TYPE_P, key);
                show_square_breakets(param.x);
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
<<<<<<< HEAD
            {               
                if (star_x == 2)
                {
                    show_a_star(star_x);
                    SET_HDCP_MODE("ON");
                }
                
                if (star_x == 4)             
                {                   
                    show_a_star(star_x);
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
}


static int CH_SELECT_P()
{   
    int x = 2; 
    int y = 16;
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);

    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE") ,1);
    show_strings(2, 16, channel_id, strlen(channel_id) ,1);
    show_a_char(2,  16, channel_id[0], 1, 1);

    int i = 0;
    int key = 0;
=======
            {           
                show_a_star(param.x);
                SET_ENCODE_VIDEO_INPUT(INPUT_VIDEO_SOURCE_TYPE_P[param.x/2]);
                break;              
            }
            case LEFT_KEY:
            {   
                return 0;
            }
        }

    }

    
}

static int EDID_SETTING_P()
{
    char buf[20] = {0};
    u8 count = GET_EDID_LIST(EDID_BUF_P);
    GET_EDID(buf);

    
    int p = 4;
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;
        
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, EDID_LIST_P[0], strlen(EDID_LIST_P[0]), 1); 
    show_menu_info_P(y, 1, EDID_SHOWWING_P, (const char **)EDID_LIST_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int i = 0;
    for (i = 1; i < 4; i++)
    {   
        if (strstr(EDID_SHOWWING_P[i], buf) != NULL)
        {
            show_a_star(2*i);
        }       
    }

    int key = 0; 
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
<<<<<<< HEAD
            {
                if (channel_id[i] < '9')
                {
                    channel_id[i]++;
                }
                else
                {
                    if (i == 2)
                    {
                        if (channel_id[0] == '0' && channel_id[1] == '0')
                        channel_id[i] = '1';
                    }
                    else
                    {
                        channel_id[i] = '0';
                    }
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case UP_KEY:
            {   
                if (i == 2)
                {
                    if (channel_id[0] == '0' && channel_id[1] == '0')
                    {
                        if (channel_id[i] > '1')
                        {
                            channel_id[i]--;
                        }
                        else
                        {
                            channel_id[i] = '9';
                        }
                    }
                    else
                    {
                        if (channel_id[i] > '0')
                        {
                            channel_id[i]--;
                        }
                        else
                        {
                            channel_id[i] = '9';
                        }
                    }
                    
                }
                else
                {
                    if (channel_id[i] > '0')
                    {
                        channel_id[i]--;                
                    }
                    else
                    {
                        channel_id[i] = '9';
                    }
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case RIGHT_KEY:
            {
                show_a_char(2, 16+i*8, channel_id[i], 0, 1);
                if (i < 2)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case LEFT_KEY:
            {
                #if 0
                show_a_char(2, 16+i*8, channel_id[i], 0, 1);
                if (i > 0)
                {
                    i--;    
                }
                else
                {
                    return 0;    // back to the previous level
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
                #endif
                
                return 0;
            }
            case ENTER_KEY:
            {
                SET_CHANNEL_DEFINE(channel_id);
                return 0;
            }

        }
    }
    return 0;
}



#if 0
int show_current_VIDEO_SELECT()
{
    int i;
    int channel_num;
    get_current_voide_channel(&channel_num);
    for(i = 1; i < 4; i++)
    {
        if (VIDEO_IN_SHOWWING_P[i] == SAVE_VIDEO_SELECT_P[channel_num])
        {
            show_a_star(i*2);
            break;
        }
    }
}

//level 3 CHANNEL SELECT
void CHANNEL_SELECT_SHOW_P(void)
{
    //更新video select
    save_VIDEO_SELECT_info_P(); 
    u8 count = get_elem_num_P(SAVE_VIDEO_SELECT_P, MIN_SIZE_P+1);
=======
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, EDID_SHOWWING_P, EDID_LIST_P, key);
                for (i = 1; i < 4; i++)
                {   
                    if (strstr(EDID_SHOWWING_P[i], buf) != NULL)
                    {
                        show_a_star(2*i);
                    }       
                }
                break;
            }
            
            case RIGHT_KEY: //标记*号
            case ENTER_KEY:
            {
                show_a_star(param.x); 
                SET_EDID(EDID_SHOWWING_P[param.x/2][0]);
                break;
            }
            
            case LEFT_KEY:
            {
                return 0;
            }
        }
    }
    
}

static int HDCP_SETTING_P()
{
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

<<<<<<< HEAD

    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, SAVE_VIDEO_SELECT_P[0], strlen(SAVE_VIDEO_SELECT_P[0]), 0); 
    show_menu_info_P(y, 1, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, count>4? 3 : count-1);
    show_square_breakets(x);
    //每次进来要查询当前频道
    show_current_VIDEO_SELECT();

    int i = 0;
=======
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
        show_strings(i*2, y, HDCP_LIST_P[i], strlen(HDCP_LIST_P[i]), 1);
    }
    show_square_breakets(star_x);
    show_a_star(star_x);
    
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
<<<<<<< HEAD
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, key);
                //show_current_VIDEO_SELECT();
=======
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
                
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
<<<<<<< HEAD
                show_a_star(param.x);
                select_voide_channel(VIDEO_IN_SHOWWING_P[param.x/2] - '0');  //频道号是i                    
                //show_current_VIDEO_SELECT();
=======
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
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
                break;
            }
            
            case LEFT_KEY:
            {
<<<<<<< HEAD
                return; //返回上一级目录
            }
        }

    }
    
}

int DHCP_SETTING_P()
{
    u8 count = 3;
    
    int i = 0;
    u8 y = 16;
    int key = 0;
    clear_whole_screen();
    
    //查询ON OFF
    int status = -1;
    get_HDCP_status(&status);
    
    show_strings(0, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 0);
    for (i = 1; i < count; i++)
    {
        show_strings(i*2, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 1);
    }

    show_a_star(4 - 2*status );
    
    
    while (1)
    {
        key = recv_key_info_P();
        
        if (key == LEFT_KEY)
        {
            break;
        }
        //其他键，无效。
    }
=======
                return 0;
            }
        }
    }   
}


static int CH_SELECT_P()
{   
    int x = 2; 
    int y = 16;
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);

    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE") ,1);
    show_strings(2, 16, channel_id, strlen(channel_id) ,1);
    show_a_char(2,  16, channel_id[0], 1, 1);

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            {
                if (channel_id[i] < '9')
                {
                    channel_id[i]++;
                }
                else
                {
                    if (i == 2)
                    {
                        if (channel_id[0] == '0' && channel_id[1] == '0')
                        channel_id[i] = '1';
                    }
                    else
                    {
                        channel_id[i] = '0';
                    }
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case UP_KEY:
            {   
                if (i == 2)
                {
                    if (channel_id[0] == '0' && channel_id[1] == '0')
                    {
                        if (channel_id[i] > '1')
                        {
                            channel_id[i]--;
                        }
                        else
                        {
                            channel_id[i] = '9';
                        }
                    }
                    else
                    {
                        if (channel_id[i] > '0')
                        {
                            channel_id[i]--;
                        }
                        else
                        {
                            channel_id[i] = '9';
                        }
                    }
                    
                }
                else
                {
                    if (channel_id[i] > '0')
                    {
                        channel_id[i]--;                
                    }
                    else
                    {
                        channel_id[i] = '9';
                    }
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case RIGHT_KEY:
            {
                show_a_char(2, 16+i*8, channel_id[i], 0, 1);
                if (i < 2)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
            }
            case LEFT_KEY:
            {
                #if 0
                show_a_char(2, 16+i*8, channel_id[i], 0, 1);
                if (i > 0)
                {
                    i--;    
                }
                else
                {
                    return 0;    // back to the previous level
                }
                show_a_char(2, 16+i*8, channel_id[i], 1, 1);
                break;
                #endif
                
                return 0;
            }
            case ENTER_KEY:
            {
                SET_CHANNEL_DEFINE(channel_id);
                return 0;
            }

        }
    }
    return 0;
>>>>>>> 1cc50a553ba25919b4221eb37fc5a96e9915eb69
}
#endif




#if 0
//一级目录       1.主菜单显示
int IPD5000_MAIN_MENU_SHOW(void)
{
    printf("this is IPD5000\n");
    save_display_info_P();
    
    u8 count = sizeof(MAIN_MENU_strings_P)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    if (recv_init_P() != 0)
    {
        printf("recv_init_P() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_strings_P[0], strlen(MAIN_MENU_strings_P[0]) ); 
    show_menu_info_P(y, 1, MAIN_MENU_SHOWWING_P, MAIN_MENU_strings_P, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, MAIN_MENU_SHOWWING_P, MAIN_MENU_strings_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[1]) // VEDIO SELECET
                    {
                        VIDEO_IN_SELECT_SHOW_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[2]) // IP SETTING
                    {
                        IP_SETTING_MENU_SHOW_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[3]) // HDCP SETTING
                    {
                        HDCP_SHOW_P();
                        break;
                    }
    
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[4]) //  VIDEO OUT RES
                    {
                        VIDEO_OUT_RES_SHOW_P();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[5]) // FIRMWARE INFO
                    {
                        FIRMWARE_INFO_SHOW_P();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_P[param.x/2], MAIN_MENU_strings_P[6]) // DEVICE STATUS
                    {
                        DEVICE_STATUS_SHOW_P();
                        break;
                    }
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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
void IP_SETTING_MENU_SHOW_P(void)
{
    u8 count = sizeof(IP_SET_strings_P)/(sizeof(char*)); //IP_SET_strings的元素个数
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
    show_strings(0, y, IP_SET_strings_P[0], strlen(IP_SET_strings_P[0]) ); 
    show_menu_info_P(y, 1, IP_SET_SHOWWING_P, IP_SET_strings_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, IP_SET_SHOWWING_P, IP_SET_strings_P, key);
                break;
            }

            case RIGHT_KEY:
            case ENTER_KEY:
            {
                //LAN1
                if (param.x == 2)
                {
                    LAN_MODE_MENU_SHOW_P(LAN1_ID_P);
                }

                //LAN2
                if (param.x == 4)
                {
                    LAN_MODE_MENU_SHOW_P(LAN2_ID_P);
                }   

                clear_whole_screen();
                //从子目录出来，继续显示
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, IP_SET_SHOWWING_P[i], strlen(IP_SET_SHOWWING_P[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(IP_SET_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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




#if 0
int show_current_VIDEO_SELECT()
{
    int i;
    int channel_num;
    get_current_voide_channel(&channel_num);
    for(i = 1; i < 4; i++)
    {
        if (VIDEO_IN_SHOWWING_P[i] == SAVE_VIDEO_SELECT_P[channel_num])
        {
            show_a_star(i*2);
            break;
        }
    }
}

//level 3 CHANNEL SELECT
void CHANNEL_SELECT_SHOW_P(void)
{
    //更新video select
    save_VIDEO_SELECT_info_P(); 
    u8 count = get_elem_num_P(SAVE_VIDEO_SELECT_P, MIN_SIZE_P+1);
    int p = 4; 
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;


    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, SAVE_VIDEO_SELECT_P[0], strlen(SAVE_VIDEO_SELECT_P[0]), 0); 
    show_menu_info_P(y, 1, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, count>4? 3 : count-1);
    show_square_breakets(x);
    //每次进来要查询当前频道
    show_current_VIDEO_SELECT();

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, key);
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                show_a_star(param.x);
                select_voide_channel(VIDEO_IN_SHOWWING_P[param.x/2] - '0');  //频道号是i                    
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

int DHCP_SETTING_P()
{
    u8 count = 3;
    
    int i = 0;
    u8 y = 16;
    int key = 0;
    clear_whole_screen();
    
    //查询ON OFF
    int status = -1;
    get_HDCP_status(&status);
    
    show_strings(0, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 0);
    for (i = 1; i < count; i++)
    {
        show_strings(i*2, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 1);
    }

    show_a_star(4 - 2*status );
    
    
    while (1)
    {
        key = recv_key_info_P();
        
        if (key == LEFT_KEY)
        {
            break;
        }
        //其他键，无效。
    }
}
#endif




#if 0
//一级目录       1.主菜单显示
int IPD5000_MAIN_MENU_SHOW(void)
{
    printf("this is IPD5000\n");
    save_display_info_P();
    
    u8 count = sizeof(MAIN_MENU_strings_P)/(sizeof(char*));
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    if (recv_init_P() != 0)
    {
        printf("recv_init_P() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    
    clear_whole_screen();
    show_strings(0, y, MAIN_MENU_strings_P[0], strlen(MAIN_MENU_strings_P[0]) ); 
    show_menu_info_P(y, 1, MAIN_MENU_SHOWWING_P, MAIN_MENU_strings_P, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_key_info_P();

        switch (key)
        {
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_P(count, param, MAIN_MENU_SHOWWING_P, MAIN_MENU_strings_P, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[1]) // VEDIO SELECET
                    {
                        VIDEO_IN_SELECT_SHOW_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[2]) // IP SETTING
                    {
                        IP_SETTING_MENU_SHOW_P();
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[3]) // HDCP SETTING
                    {
                        HDCP_SHOW_P();
                        break;
                    }
    
                    if ( MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[4]) //  VIDEO OUT RES
                    {
                        VIDEO_OUT_RES_SHOW_P();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_P[param.x/2] == MAIN_MENU_strings_P[5]) // FIRMWARE INFO
                    {
                        FIRMWARE_INFO_SHOW_P();
                        break;
                    }
                    if (MAIN_MENU_SHOWWING_P[param.x/2], MAIN_MENU_strings_P[6]) // DEVICE STATUS
                    {
                        DEVICE_STATUS_SHOW_P();
                        break;
                    }
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_P[i], strlen(MAIN_MENU_SHOWWING_P[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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
void IP_SETTING_MENU_SHOW_P(void)
{
    u8 count = sizeof(IP_SET_strings_P)/(sizeof(char*)); //IP_SET_strings的元素个数
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
    show_strings(0, y, IP_SET_strings_P[0], strlen(IP_SET_strings_P[0]) ); 
    show_menu_info_P(y, 1, IP_SET_SHOWWING_P, IP_SET_strings_P, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, IP_SET_SHOWWING_P, IP_SET_strings_P, key);
                break;
            }

            case RIGHT_KEY:
            case ENTER_KEY:
            {
                //LAN1
                if (param.x == 2)
                {
                    LAN_MODE_MENU_SHOW_P(LAN1_ID_P);
                }

                //LAN2
                if (param.x == 4)
                {
                    LAN_MODE_MENU_SHOW_P(LAN2_ID_P);
                }   

                clear_whole_screen();
                //从子目录出来，继续显示
                int i;
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, IP_SET_SHOWWING_P[i], strlen(IP_SET_SHOWWING_P[i]) ); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(IP_SET_SHOWWING_P[i]) != 1) 
                    {
                        move_limit_P = i*2;
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
void LAN_MODE_MENU_SHOW_P(int lan_id) //DHCP OR STATIC
{
    int i = 0;
    int y = 16; //有*要显示
    int x = 2; //方括号位置
    
    int DHCP_status = -1;
    get_PHCP_status(lan_id, &DHCP_status);
    
    clear_whole_screen();
    show_strings(0, y, LAN_MODE_strings_P[0], strlen(LAN_MODE_strings_P[0]), 0); 
    show_menu_info_P(y, 1, LAN_MODE_SHOWWING_P, LAN_MODE_strings_P, 2);
    show_square_breakets(x);

    //显示status： DHCP STATIS 
    show_a_star(4 - (2 * DHCP_status)); // x = 2 DHCP, x = 4 STATIC 

    int key = 0;
    while (1)
    {       
        key = recv_key_info_P();
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
                    set_PHCP_status(lan_id);
                    LAN_OPTION_SHOW_P(lan_id, 1);
                }
                
                if (x == 4)             //选择static
                {                   
                    LAN_OPTION_SHOW_P(lan_id, 0);
                }
                //继续显示这一级目录
                get_PHCP_status(lan_id, &DHCP_status);
                clear_whole_screen();
                show_strings(0, y, LAN_MODE_strings_P[0], strlen(LAN_MODE_strings_P[0]), 0); 
                show_menu_info_P(y, 1, LAN_MODE_SHOWWING_P, LAN_MODE_strings_P, 2);
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
static void LAN_OPTION_SHOW_P(int lan_id, int enable) //ip mask gateway
{   
    int y = 16;
    int x = 2; //方括号位置

    u8 i, begin_num = 1;

    clear_whole_screen();
    show_strings(i*2, y, LAN_OPTION_strings_P[i], strlen(LAN_OPTION_strings_P[i]), 0);
    for (i = 1; i < 4; i++)
    {
        show_strings(i*2, y, LAN_OPTION_strings_P[i], strlen(LAN_OPTION_strings_P[i]), 1);
    }
    show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
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
                get_ip(lan_id, net_info_P[lan_id*3], net_info_P[lan_id*3 +1], net_info_P[lan_id*3 +2]);

                //x是坐标，x=2:ip, x=4:mask, x=6:gateway
                if (enable == 1)
                    DHCP_LAN_INFO_SHOW_P(lan_id, x/2, net_info_P[lan_id*3 + (x/2-1)], strlen(net_info_P[lan_id*3 + (x/2-1)]));
                else
                    LAN_INFO_SET_P(lan_id, x/2, net_info_P[lan_id*3 + (x/2-1)], strlen(net_info_P[lan_id*3 + (x/2-1)]));

                //子目录出来，继续显示这一级目录
                clear_whole_screen();
                int i;
                show_strings(i*2, y, LAN_OPTION_strings_P[i], strlen(LAN_OPTION_strings_P[i]), 0);
                for (i = 0; i < 4; i++)
                {
                    show_strings(i*2, y, LAN_OPTION_strings_P[i], strlen(LAN_OPTION_strings_P[i]), 1); 
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
static void DHCP_LAN_INFO_SHOW_P(int lan_id, int offset, char *string, u8 lenth)
{
    int x = 0, y = 16, y1 = 0;
    int key = 0;
    clear_whole_screen();
    show_strings(x, y, LAN_OPTION_strings_P[offset], strlen(LAN_OPTION_strings_P[offset]), 0); //显示标题
    show_strings(x+2, y1, string, strlen(string), 1);
    
    while (1)
    {
        key = recv_key_info_P();        
        if (key == LEFT_KEY)
        {
            break;
        }
    }   
}

//五级目录 LAN INFO set
static void LAN_INFO_SET_P(int lan_id, u8 offset, char *string, u8 lenth)
{
    u8 x = 0, y = 16, y1 = 0;
    int i = 0;
    int key = 0;

    clear_whole_screen();
    show_strings(x, y, LAN_OPTION_strings_P[offset], strlen(LAN_OPTION_strings_P[offset]), 0); //显示标题
    show_strings(x+2, y1, string, strlen(string), 1);
    show_a_char(x+2, y1, string[0], 1, 1);
    
    while (1)
    {   
        key = recv_key_info_P();

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
                set_ip(lan_id, net_info_P[lan_id*3], net_info_P[lan_id*3+1], net_info_P[lan_id*3+2]);
                return; //返回到上一级目录
            }
            
        }
        
    }
}
#endif

#if 0
//二级目录，
void VIDEO_IN_SELECT_SHOW_P(void)
{
    u8 count = get_elem_num_P(SAVE_VIDEO_SELECT_P, MIN_SIZE_P+1);
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
    save_VIDEO_SELECT_info_P(); 
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, SAVE_VIDEO_SELECT_P[0], strlen(SAVE_VIDEO_SELECT_P[0]), 0); 
    show_menu_info_P(y, 1, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, count>4? 3 : count-1);
    show_square_breakets(x);
    //每次进来要查询当前频道
    show_current_VIDEO_SELECT();

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, VIDEO_IN_SHOWWING_P, SAVE_VIDEO_SELECT_P, key);
                //show_current_VIDEO_SELECT();
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                show_a_star(param.x);
                select_voide_channel(VIDEO_IN_SHOWWING_P[param.x/2] - '0');  //频道号是i                    
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
void VIDEO_OUT_RES_SHOW_P()
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
    get_current_VIDEO_OUT_info_P();
    clear_whole_screen();  //新一级的目录，清屏
    show_strings(0, y, VIDEO_OUT_string_P[0], strlen(VIDEO_OUT_string_P[0]), 0); 
    show_strings(2, y, VIDEO_OUT_string_P[1], strlen(VIDEO_OUT_string_P[1]), 1);
    //show_square_breakets(x);
    
    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
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
void HDCP_SHOW_P() //仅展示
{
    u8 count = 3;
    int i = 0;
    u8 y = 16;
    int key = 0;
    clear_whole_screen();
    
    //查询ON OFF
    int status = -1;
    get_HDCP_status(&status);
    
    show_strings(0, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 0);
    for (i = 1; i < count; i++)
    {
        show_strings(i*2, y, HDCP_strings_P[i], strlen(HDCP_strings_P[i]), 1);
    }

    show_a_star(4 - 2*status );
    
    
    while (1)
    {
        key = recv_key_info_P();
        
        if (key == LEFT_KEY)
        {
            break;
        }
        //其他键，无效。
    }
}

//二级目录
void FIRMWARE_INFO_SHOW_P() //仅展示
{
    u8 count = get_elem_num_P(FIRMWARE_strings_P, MIN_SIZE_P+1);
    int p = 4; 
    int y = 8;
    int x = 2; //方括号位置
    int last_page = 0;
    
    info_param param;
    param.x = x;
    param.y = y;
    param.p = p;
    param.last_page = last_page;

    save_FIREWARE_info_P(); 
    clear_whole_screen();
    show_strings(0, y, FIRMWARE_strings_P[0], strlen(FIRMWARE_strings_P[0]), 0); 
    show_menu_info_P(y, 1, FIRMWARE_SHOWWING_P, FIRMWARE_strings_P, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, FIRMWARE_SHOWWING_P, FIRMWARE_strings_P, key);
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
void DEVICE_STATUS_SHOW_P() //仅展示
{
    u8 count = get_elem_num_P(DEVICE_STATUS_strings_P, MIN_SIZE_P+1);
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
    show_strings(0, y, DEVICE_STATUS_strings_P[0], strlen(DEVICE_STATUS_strings_P[0]), 0); 
    show_menu_info_P(y, 1, DEVICE_STATUS_SHOWWING_P, DEVICE_STATUS_strings_P, count>4? 3 : count-1);
    //show_square_breakets(x);

    int key = 0;
    while (1)
    {
        key = recv_key_info_P();
        switch (key)
        {
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_P(count, param, DEVICE_STATUS_SHOWWING_P, DEVICE_STATUS_strings_P, key);
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


