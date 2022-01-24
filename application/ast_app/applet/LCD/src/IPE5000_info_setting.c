#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>

#include "oled.h"
#include "IPE5000_info_setting.h"
#include "msg_queue.h"
#include "send_p3k_cmd.h"
#include  "recv_button_status.h"

#define _GNU_SOURCE

#define SIZE_E 255
#define MIN_SIZE_E 20

#define LAN1_ID_E   0
#define LAN2_ID_E   1

#define CHANNEL_MAP "/data/configs/kds-7/channel/channel_map.json"

time_t last_change_time_E = 0;

typedef struct
{
    int x;          //x坐标
    int y;          //y坐标
    int p;          //记录字符数组中 要显示在第二行，即第一个数据栏的位置。
    int last_page;  //是否到达最后一页（翻页）
}info_param;

char *blank_E = " ";


// LEVEL 1
const char* MAIN_MENU_LIST_E[] = {
    "MAIN MENU", "DEV STATUS", "DEV INFO", "DEV SETTINGS",
};
// END LEVEL 1


// LEVEL 2 
const char* DEV_STATUS_LIST_E[] = {
    "DEV STATUS", "LAN1 STATUS", "LAN2 STATUS", "HDMI STATUS", "CH DEFINE", "TEMPERATURE",
};

char FW_VERSION_E[20] = {0};
char BL_VERSION_E[20] = {0};
char HW_VERSION_E[20] = {0};
const char* DEV_INFO_LIST_E[] = {
    "DEV INFO", (const char*)FW_VERSION_E, (const char*)BL_VERSION_E, (const char*)HW_VERSION_E,
};

const char* DEV_SETTINGS_LIST_E[] = {
    "DEV SETTINGS", "EDID SETTING", "HDCP SETTING", "CH DEFINE",
};
// END LEVEL 2


// LEVEL 3
char RESOL_BUF_E[20]       = {0};
char HDCP_STATUS_BUF_E[20] = {0};
const char* HDMI_STATUS_LIST_E[] = {
    "HDMI STATUS", (const char*)RESOL_BUF_E, (const char*)HDCP_STATUS_BUF_E,
};

char EDID_BUF_E[20][20] = {{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
                           {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},};

const char *EDID_LIST_E[] = {"EDID SETTING", (const char *)EDID_BUF_E[0],(const char *)EDID_BUF_E[1],(const char *)EDID_BUF_E[2],(const char *)EDID_BUF_E[3],(const char *)EDID_BUF_E[4],
                                             (const char *)EDID_BUF_E[5],(const char *)EDID_BUF_E[6],(const char *)EDID_BUF_E[7],(const char *)EDID_BUF_E[8],(const char *)EDID_BUF_E[9],};

const char *HDCP_LIST_E[] = {"HDCP SETTING", "ON", "OFF"};

// END LEVEL 3

//Records the string being displayed
const char *MAIN_MENU_SHOWWING_E[4]       =    {NULL, NULL, NULL, NULL}; 
const char *DEV_STATUS_SHOWWING_E[4]      =    {NULL, NULL, NULL, NULL};
const char *DEV_INFO_SHOWWING_E[4]        =    {NULL, NULL, NULL, NULL};
const char *DEV_SETTINGS_SHOWWING_E[4]    =    {NULL, NULL, NULL, NULL};
const char *HDMI_STATUS_SHOWWING_E[4]     =    {NULL, NULL, NULL, NULL};
const char *EDID_SHOWWING_E[4]            =    {NULL, NULL, NULL, NULL};


//记录中括号的X坐标位置
static int move_limit_E = 0; 

/*
    1.首先获取初始化数据，既要显示的东西
    2.如果有更改，那么就要往P3K发消息。send
    3.是不是要再获取一次，看有没有真正改变了。那么这个显示速度问题。
*/

#if 0
int save_VIDEO_SELECT_info_E()
{
    int i = 0, err = 0;
    memset(video_select_buf, 0, MIN_SIZE_E*SIZE_E);

    err = VIDEO_LIST(video_select_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_E; i++)
        {
            strcpy(video_select_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_E+1; i++)
    {
        SAVE_VIDEO_SELECT_E[i] = video_select_buf[i-1];
    }
}

int save_FIREWARE_info_E()
{
    int i = 0, err = 0;
    memset(firmware_buf, 0, MIN_SIZE_E*SIZE_E);

    err = get_FIRMWARE_INFO(firmware_buf);
    if (err == -1)
    {
        for (i = 0; i < MIN_SIZE_E; i++)
        {
            strcpy(firmware_buf[i], " ");
        }
    }
    
    for(i = 1; i < MIN_SIZE_E+1; i++)
    {
        FIRMWARE_strings_E[i] = firmware_buf[i-1];
    }
}

int save_EEVICE_STATUS_info_E()
{
    int i = 0;
    int status = -1;
    get_EEVICE_STATUS(&status);
    DEVICE_STATUS_strings_E[1] = device_status[status+1];
}

int get_current_VIDEO_OUT_info_E()
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
    VIDEO_OUT_string_E[1] = video_out[s];

}

void save_display_info_E()
{
    
    init_p3k_client("127.0.0.1", 6001);
    
    save_LAN_info_E();
    save_VIDEO_SELECT_info_E();
    save_FIREWARE_info_E();
    save_EEVICE_STATUS_info_E();
    get_current_VIDEO_OUT_info_E();
}

void show_current_edid_E()
{
    int i;
    char edid_type;
    edid_type = GET_EDID();
    
    for (i = 1; i < 4; i++ )
    {
        if (EDID_SHOWWING_E[i][0] == edid_type);
        {
            show_a_star(i*2);
            break;
        }
    }
}
#endif

int file_is_changed_E(const char* filename)
{
    int err = -1;
    struct stat state;
    
    if (filename == NULL)
        return -1;
    
    if (access(filename, F_OK|R_OK) == -1)
        return -1;

    if (stat(filename, &state) != 0)
        return -1;

    if (last_change_time_E == state.st_mtime)
    {
        return 0;
    }
    else
    {
        last_change_time_E = state.st_mtime;
        return 1;
    }
}

u8 get_elem_num_E(const char **buf, int num)
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
void show_menu_info_E(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
{
    //取剩下的显示，最多取三个
    u8 x = 0;
    int i; 
    
    dest[0] = src[0];

    if (count == 0)
    {
        move_limit_E = 2;
        for (i = 1; i < 4; i++)
        {
            dest[i] = blank_E;
        }
        return;
    }   
    for (i = 0; i < count; i++)
    {
        dest[1+i] = src[begin_elem+i];
    }
    
    move_limit_E = i*2; //选择框向下最大移动坐标

    for (i+=1; i < 4; i++)
    {
        dest[i] = blank_E;
    }
    
    //显示
    for (i = 1; i < 4; i++)
    {
        show_strings(x+2*i, y, dest[i], strlen(dest[i]), 1);
    }

}

//响应 下行键，上行键
info_param down_up_respond_E(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
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
            if (x > move_limit_E) //X坐标超过最大值，取最大值
            {
                x = move_limit_E;
                
                if (count > 4)//要超过4行才能翻页，
                {
                    //p被上行键更改了 或者count <= 7时，p不会被上行键更改，但页面已经被向上翻页了，通过MAIN_MENU_SHOWWING[3],MAIN_MENU_strings_E[3]来判断是否是最后一页。
                    if ( (p + 3 < count) || (strcmp(showwing_strings[3], src_string[3]) == 0))
                    {
                        last_page = 0;  
                    }
                    
                    if ((count - p > 0) && last_page == 0) //还有也可以翻，且没有翻到最后一页
                    {
                        if (p + 3 < count) 
                        {   
                            clear_three_line();
                            show_menu_info_E(y, p, showwing_strings, src_string, 3); //数组是从0开始
                            p += 3;
                        }
                        else 
                        {
                            clear_three_line();
                            show_menu_info_E(y, p, showwing_strings, src_string, count-p);
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
                            show_menu_info_E(y, p, showwing_strings, src_string, 3);
                        }
                        else //p==4
                        {
                            clear_three_line();
                            show_menu_info_E(y, p-3, showwing_strings, src_string, 3);
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

int IPE5000_SHOW_INIT(char *status)
{
    printf("this is IPE5000\n");
    init_p3k_client("127.0.0.1", 6001);
    
    if (strcmp(status, "unlock") == 0)
    {
        IPE5000_UNLOCK_MENU();
    }
    else if(strcmp(status, "lock") == 0)
    {
        IPE5000_LOCK_MENU();
    }
    
    deinit_p3k_client();
    return 0;
}

static int IPE5000_UNLOCK_MENU()
{
    if (recv_button_init() != 0)
    {
        printf("recv_button_init() fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
    if (run_recv_butoon_event() != 0)
    {
        printf("run_recv_butoon_event fail\n");
        return -1;
    }
    
    CH_NUM_SELECT_E();
    
    return 0;
}

static int IPE5000_LOCK_MENU()
{
    system("pkill -9 recv_button_event");

    char show_buf[20] = {0};
    char current_id[20] = {0};
    char last_id[20] = {0};
    
    GET_CHANNEL_DEFINE(current_id);
    strcpy(last_id, current_id);
    sprintf(show_buf, "CH %s", current_id);
    
    clear_whole_screen();
    show_strings(2, 40, show_buf, strlen(show_buf), 1);
    show_strings(4, 40, "LOCKED", strlen("LOCKED"), 1);
    
    while(1)
    {
        GET_CHANNEL_DEFINE(current_id);;

        if (strcmp(last_id, current_id) != 0)
        {   
            strcpy(last_id, current_id);
            show_strings(2, 64, current_id, strlen(current_id), 1);
        }
        
        sleep(5);
    }

    return 0;
}



// The first level
static int IPE5000_MAIN_MENU_SHOW(void)
{
    printf("IPE5000_MAIN_MENU_SHOW\n");
    u8 count = sizeof(MAIN_MENU_LIST_E)/(sizeof(char*));
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
    show_strings(0, y, MAIN_MENU_LIST_E[0], strlen(MAIN_MENU_LIST_E[0]), 1);
    show_menu_info_E(y, 1, MAIN_MENU_SHOWWING_E, MAIN_MENU_LIST_E, count>4? 3 : count-1);
    show_square_breakets(x);

    int key = 0;
    while (1)
    { 
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_E(count, param, MAIN_MENU_SHOWWING_E, MAIN_MENU_LIST_E, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if ( MAIN_MENU_SHOWWING_E[param.x/2] == MAIN_MENU_LIST_E[1]) // DEV STATUS
                    {
                        if (SHOW_TIMEOUT == DEV_STATUS_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_E[param.x/2] == MAIN_MENU_LIST_E[2]) // DEV INFO
                    {
                        if (SHOW_TIMEOUT == DEV_INFO_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                    if (MAIN_MENU_SHOWWING_E[param.x/2] == MAIN_MENU_LIST_E[3]) // DEV SETTINGS
                    {
                        if (SHOW_TIMEOUT == DEV_SETTINGS_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, MAIN_MENU_SHOWWING_E[i], strlen(MAIN_MENU_SHOWWING_E[i]), 1);
                for (i = 1; i < count; i++)
                {
                    show_strings(i*2, y, MAIN_MENU_SHOWWING_E[i], strlen(MAIN_MENU_SHOWWING_E[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(MAIN_MENU_SHOWWING_E[i]) != 1) 
                    {
                        move_limit_E = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
}

// the second level: 1.DEV STATUS
static int DEV_STATUS_E()
{
    u8 count = sizeof(DEV_STATUS_LIST_E)/(sizeof(char*)); //DEV_STATUS_E的元素个数
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
    show_strings(0, y, DEV_STATUS_LIST_E[0], strlen(DEV_STATUS_LIST_E[0]) ,1); 
    show_menu_info_E(y, 1, DEV_STATUS_SHOWWING_E, DEV_STATUS_LIST_E, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_E(count, param, DEV_STATUS_SHOWWING_E, DEV_STATUS_LIST_E, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    if (DEV_STATUS_SHOWWING_E[param.x/2] == DEV_STATUS_LIST_E[1]) // LAN1 STATUS 
                    {
                        if (SHOW_TIMEOUT == LAN_STATUS_E(LAN1_ID_E))
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_E[param.x/2] == DEV_STATUS_LIST_E[2]) // LAN1 STATUS 
                    {
                        if (SHOW_TIMEOUT == LAN_STATUS_E(LAN2_ID_E))
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                    if (DEV_STATUS_SHOWWING_E[param.x/2] == DEV_STATUS_LIST_E[3]) // HDMI STATUS
                    {
                        if (SHOW_TIMEOUT == HDMI_STATUS_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
    
                    if (DEV_STATUS_SHOWWING_E[param.x/2] == DEV_STATUS_LIST_E[4]) // CHANNEL SEL
                    {
                        if (SHOW_TIMEOUT == CH_DEFINE_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    if (DEV_STATUS_SHOWWING_E[param.x/2] == DEV_STATUS_LIST_E[5]) // TEMPERATURE
                    {
                        if (SHOW_TIMEOUT == TEMPERATURE_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_STATUS_SHOWWING_E[i], strlen(DEV_STATUS_SHOWWING_E[i]), 1);
                for (i = 1; i < 4; i++)
                {
                    if (DEV_STATUS_SHOWWING_E[i] != NULL)
                        show_strings(i*2, y, DEV_STATUS_SHOWWING_E[i], strlen(DEV_STATUS_SHOWWING_E[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_STATUS_SHOWWING_E[i]) != 1) 
                    {
                        move_limit_E = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                return 0;
            }

            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
    
}

// 1.1 DEV STATUS -> LAN SHOW
static int LAN_STATUS_E(int interface_id)
{
    char addr[16] = {0}, mask[16] = {0}, gateway[16] = {0};
    GET_IP(interface_id, addr, mask, gateway);
    //printf("addr:[%s]\n", addr);
    
    clear_whole_screen();
    if (interface_id == LAN1_ID_E)
        show_strings(0, 16, "LAN 1 STATUS", strlen("LAN 1 STATUS"), 1);
    else if (interface_id == LAN2_ID_E)
        show_strings(0, 16, "LAN 2 STATUS", strlen("LAN 2 STATUS"), 1);
    
    show_strings(2, 8, addr, 15, 1);
    show_strings(4, 8, mask, 15, 1);
    show_strings(6, 8, gateway, 15, 1);

    int key = 0;
    while (1)
    {
        key = recv_button_status();
        
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case LEFT_KEY: 
            {  
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
}

// 1.3 DEV STATUS -> HDMI STATUS
static int HDMI_STATUS_E()
{
    //1.resolution
    GET_ACTUAL_RESOLUTION(RESOL_BUF_E);
    
    //2.HDCP STATUS
    GET_HDCP_STATUS(HDCP_STATUS_BUF_E);

    u8 count = sizeof(HDMI_STATUS_LIST_E)/(sizeof(char*));
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
    show_strings(0, y, HDMI_STATUS_LIST_E[0], strlen(HDMI_STATUS_LIST_E[0]) ,1); 
    show_menu_info_E(y, 1, HDMI_STATUS_SHOWWING_E, HDMI_STATUS_LIST_E, count>4 ? 3 : count-1);

    int key = 0;
    while (1)
    { 
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case LEFT_KEY:
            {
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
        
}

// 1.4 DEV STATUS -> CHANNEL SELECT
static int CH_DEFINE_E()
{
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);
    
    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE") ,1);
    show_strings(2, 16, channel_id, strlen(channel_id) ,1);
    
    int key = 0;
    while (1)
    { 
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case LEFT_KEY:
            {
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }

}

// 1.5 DEV STATUS -> TEMPERATURE
static int TEMPERATURE_E()
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
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case LEFT_KEY:
            {
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
    
}


// 2. DEV INFO
static int DEV_INFO_E()
{
    u8 count = sizeof(DEV_INFO_LIST_E)/(sizeof(char*));
    
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
    GET_FW_VERSION(FW_VERSION_E);
    GET_BL_VERSION(BL_VERSION_E);
    GET_HW_VERSION(HW_VERSION_E);
    
    clear_whole_screen();
    show_strings(0, y, DEV_INFO_LIST_E[0], strlen(DEV_INFO_LIST_E[0]), 1); 
    show_menu_info_E(y, 1, DEV_INFO_SHOWWING_E, DEV_INFO_LIST_E, count>4? 3 : count-1);

    int key = 0;
    while (1)
    {
        key = recv_button_status();
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case LEFT_KEY:  //返回上一级目录
            {
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
}


// 3  DEV SETTINGS
static int DEV_SETTINGS_E()
{
    u8 count = sizeof(DEV_SETTINGS_LIST_E)/(sizeof(char*)); //DEV_STATUS_E的元素个数
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
    show_strings(0, y, DEV_SETTINGS_LIST_E[0], strlen(DEV_SETTINGS_LIST_E[0]), 1); 
    show_menu_info_E(y, 1, DEV_SETTINGS_SHOWWING_E, DEV_SETTINGS_LIST_E, count>4? 3 : count-1);
    show_square_breakets(x);
    
    int key = 0;
    while (1)
    { 
        key = recv_button_status();

        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case UP_KEY:
            case DOWN_KEY:
            {
                param = down_up_respond_E(count, param, DEV_SETTINGS_SHOWWING_E, DEV_SETTINGS_LIST_E, key);
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {
                do {
                    
                    if (DEV_SETTINGS_SHOWWING_E[param.x/2] == DEV_SETTINGS_LIST_E[1]) // EDID SETTING 
                    {
                        if (SHOW_TIMEOUT == EDID_SETTING_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                    if (DEV_SETTINGS_SHOWWING_E[param.x/2] == DEV_SETTINGS_LIST_E[2]) // HDCP SETTING
                    {
                        if (SHOW_TIMEOUT == HDCP_SETTING_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
    
                    if (DEV_SETTINGS_SHOWWING_E[param.x/2] == DEV_SETTINGS_LIST_E[3]) // CH DEFINE
                    {
                        if (SHOW_TIMEOUT == CH_SELECT_E())
                        {
                            return SHOW_TIMEOUT;
                        }
                        break;
                    }
                    
                }while(0);

                clear_whole_screen();
                //从子目录出来， 恢复显示这一级目录
                int i = 0;
                show_strings(0, y, DEV_SETTINGS_SHOWWING_E[0], strlen(DEV_SETTINGS_SHOWWING_E[0]), 1); 
                for (i = 1; i < 4; i++)
                {
                    if (DEV_SETTINGS_SHOWWING_E[i] != NULL)
                        show_strings(i*2, y, DEV_SETTINGS_SHOWWING_E[i], strlen(DEV_SETTINGS_SHOWWING_E[i]), 1); 
                }
                show_square_breakets(param.x);

                for (i = 3; i > 0; i--)
                {
                    if (strlen(DEV_SETTINGS_SHOWWING_E[i]) != 1) 
                    {
                        move_limit_E = i*2;
                        break;
                    }
                }
                
                break;
            }
            case LEFT_KEY:
            {   
                return 0;
            }
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }

}

static int EDID_SETTING_E()
{
    char buf[20] = {0};
    u8 count = GET_EDID_LIST(EDID_BUF_E) + 1;
    
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
    show_strings(0, y, EDID_LIST_E[0], strlen(EDID_LIST_E[0]), 1);
    show_menu_info_E(y, 1, EDID_SHOWWING_E, EDID_LIST_E, count>4? 3 : count-1);
    show_square_breakets(x);

    //CUSTOM,1
    int i = 0;
    
    for (i = 1; i < 4; i++)
    {   
        if (strcasecmp(buf, "passthru") == 0)
        {

        }
        else
        {
            if ( EDID_SHOWWING_E[i][0] == buf[0])
            {
                show_a_star(2*i);
            }
        }
    }

    int key = 0; 
    while (1)
    {
        key = recv_button_status();
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case DOWN_KEY:
            case UP_KEY:
            {
                param = down_up_respond_E(count, param, EDID_SHOWWING_E, EDID_LIST_E, key);
                for (i = 1; i < 4; i++)
                {   
                    if (EDID_SHOWWING_E[i][0] == buf[0])
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
                SET_EDID(EDID_SHOWWING_E[param.x/2][0]);
                break;
            }
            
            case LEFT_KEY:
            {
                return 0;
            }

            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
    
}

static int HDCP_SETTING_E()
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
        show_strings(i*2, y, HDCP_LIST_E[i], strlen(HDCP_LIST_E[i]), 1);
    }
    show_square_breakets(star_x);
    show_a_star(star_x);

    int key = 0;
    while (1)
    {
        key = recv_button_status();
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
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
                break;
            }
            
            case RIGHT_KEY:
            case ENTER_KEY:
            {               
                if (star_x == 2)
                {
                    show_a_star(star_x);
                    SET_HDCP_MODE("ON");
                }
                
                else if (star_x == 4)            
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

            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }   
}

static int CH_SELECT_E()
{
    int x = 2; 
    int y = 16;
    char show_buf[20] = {0};
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);

    sprintf(show_buf, "CH %s", channel_id);
    clear_whole_screen();
    show_strings(0, 16, "CH DEFINE", strlen("CH DEFINE"), 1);
    show_strings(2, 16, show_buf, strlen(show_buf), 1);
    show_a_char(2,  40, channel_id[0], 1, 1);
    
    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_button_status();
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            case UP_KEY:
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
                show_a_char(2, 40+i*8, channel_id[i], 1, 1);
                break;
            }
            case DOWN_KEY:
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
                show_a_char(2, 40+i*8, channel_id[i], 1, 1);
                break;
            }
            case RIGHT_KEY:
            {
                show_a_char(2, 40+i*8, channel_id[i], 0, 1);
                if (i < 2)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }
                show_a_char(2, 40+i*8, channel_id[i], 1, 1);
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
            case TIMEOUT:
            {
                return SHOW_TIMEOUT;
            }
        }
    }
    return 0;
}

static int CH_NUM_SELECT_E()
{   
    int x = 2; 
    int y = 16;

    char show_buf[20] = {0};
    char last_id[20] = {0};
    char channel_id[20] = {0};
    GET_CHANNEL_DEFINE(channel_id);
    strcpy(last_id, channel_id);

    sprintf(show_buf, "CH %s", channel_id);
    clear_whole_screen();
    show_strings(2, 40, show_buf, strlen(show_buf) ,1);
    //show_a_char(2,  64, channel_id[0], 1, 1);

    int i = 0;
    int key = 0;
    while (1)
    {
        key = recv_button_status();
        switch (key)
        {
            case FAULT:
            {
                clear_whole_screen();
                show_strings(2, 0, "GET KEY FAIL", strlen("GET KEY FAIL"), 1);
                exit(-1);
            }
            #if 0
            case UP_KEY:
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
                        {
                            channel_id[i] = '1';
                        }
                        else
                        {
                            channel_id[i] = '0';
                        }
                    }
                    else
                    {
                        channel_id[i] = '0';
                    }
                }
                
                show_a_char(2, 64+i*8, channel_id[i], 1, 1);
                SET_CHANNEL_DEFINE(channel_id);
                break;
            }
            case DOWN_KEY:
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

                show_a_char(2, 64+i*8, channel_id[i], 1, 1);
                SET_CHANNEL_DEFINE(channel_id);
                break;
            }
            case RIGHT_KEY:
            {
                show_a_char(2, 64+i*8, channel_id[i], 0, 1);
                if (i < 2)
                {
                    i++;
                }
                else
                {
                    i = 0;
                }

                show_a_char(2, 64+i*8, channel_id[i], 1, 1);
                
                break;
            }
            case LEFT_KEY:
            {
                show_a_char(2, 64+i*8, channel_id[i], 0, 1);
                if (i > 0)
                {
                    i--;    
                }
                else
                {
                    i = 2;
                }

                show_a_char(2, 64+i*8, channel_id[i], 1, 1);

                break;
                
            }
            #endif
            
            case ENTER_KEY:
            {
                strcpy(last_id, channel_id);

                IPE5000_MAIN_MENU_SHOW();
                
                GET_CHANNEL_DEFINE(channel_id);
                strcpy(last_id, channel_id);
                sprintf(show_buf, "CH %s", channel_id);
                
                clear_whole_screen();
                show_strings(2, 40, show_buf, strlen(show_buf) ,1);
                //show_a_char(2,  64, channel_id[0], 1, 1);
                i = 0;
                
                break;
            }
            case TIMEOUT:
            {
                GET_CHANNEL_DEFINE(channel_id);
                if (strcmp(last_id, channel_id) != 0)
                {
                    strcpy(last_id, channel_id);
                    sprintf(show_buf, "CH %s", channel_id);
                    
                    clear_a_line(2);
                    show_strings(2, 40, show_buf, strlen(show_buf) ,1);
                    //show_a_char(2,  64+i*8, channel_id[i], 1, 1);
                }
                
                break;
            }
        }
    }
    return 0;
}



