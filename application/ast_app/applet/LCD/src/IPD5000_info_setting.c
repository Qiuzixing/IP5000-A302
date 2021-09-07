
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>

#include "oled.h"
#include "IPD5000_info_setting.h"
#include "msg_queue.h"
#include "send_p3k_cmd.h"

#define SIZE_D 255
#define MIN_SIZE_D 20

#define LAN1_ID_D	0
#define LAN2_ID_D	1

pthread_mutex_t g_lock_D;

//方向
enum 
{
	ENTER_KEY = 1,
	UP_KEY,
	DOWN_KEY,
	LEFT_KEY,
	RIGHT_KEY,
};

/*
PASSTHRU/3840x2160P30 	74
	/3840x2160P25		73
	/1920x1080P60/		16
	1920X1080P50/       31
	1280x720P60			4
*/
#define NO_SIGNAL		0
#define VIDEO_OUT_74 	74
#define VIDEO_OUT_73	73
#define VIDEO_OUT_16	16
#define VIDEO_OUT_31	31
#define VIDEO_OUT_4		4

char *blank = " ";
//从中用来获取显示数据的
const char *video_out[] = {
	"VIDEO OUT RES", "NO SIGNAL", "3840x2160P30", "3840x2160P25",
	"1920x1080P60", "1920X1080P50", "1280x720P60",	
};

const char *device_status[5] = {
	"DEVICE_STATUS", "POWER ON", "STANDY BY", "FW DOWNLOAD", "IP FALLBACK"
}; 
	
char video_select_buf[MIN_SIZE_D][SIZE_D];
char firmware_buf[MIN_SIZE_D][SIZE_D];

//用来显示的
const char* MAIN_MENU_strings_D[] = {
	"MAIN MENU", "VIDEO SELECT", "IP SETTING", "HDCP SETTING",
	"VEDIO OUT RES", "FIRMWARE INFO", "DEVICE STATUS",
};

const char* IP_SET_strings_D[] = {
	"IP SETTING", "LAN1 SETTING", "LAN2 SETTING",
};
	
const char* LAN_MODE_strings_D[] = {
	"LAN MODE", "DHCP", "STATIC",
};

const char* LAN_OPTION_strings_D[] = {
	"LAN INFO", "LAN ADDR", "LAN MASK", "LAN GATEWAY",
};

const char* HDCP_strings_D[] = {
	"HDCP SETTING", "HDCP ON", "HDCP OFF",
};

const char* SAVE_VIDEO_SELECT_D[MIN_SIZE_D+1]		= {"VIDEO SELECT",}; 
const char* FIRMWARE_strings_D[MIN_SIZE_D+1]		= {"FIRMWARE INFO",};
const char* DEVICE_STATUS_strings_D[MIN_SIZE_D+1]	= {"DEVICE STATUS",};
const char* VIDEO_OUT_string_D[2]		= {"VIDEO OUT RES", " "};

//用来存储网络配置信息: 网口1：ip, mask gateway.网口2: ip, mask gateway 共6个
char net_info_D[6][16] = {{0}, {0}, {0}, {0}, {0}, {0}};

typedef struct
{
	int x;          //x坐标
	int y;			//y坐标
	int p; 			//记录字符数组中 要显示在第二行，即第一个数据栏的位置。
	int last_page;  //是否到达最后一页（翻页）
}info_param;

//记录正在屏幕显示的字符串，一个屏幕最多显示4行.
const char *MAIN_MENU_SHOWWING_D[4]		= 	{NULL, NULL, NULL, NULL}; 
const char *IP_SET_SHOWWING_D[4] 		= 	{NULL, NULL, NULL, NULL};
const char *LAN_MODE_SHOWWING_D[4]		= 	{NULL, NULL, NULL, NULL};
const char *LAN1_OPTION_SHOWWING_D[4] 	=	{NULL, NULL, NULL, NULL};
const char *VIDEO_IN_SHOWWING_D[4] 		= 	{NULL, NULL, NULL, NULL};
const char *VIDEO_OUT_SHOWWING_D[4]		= 	{NULL, NULL, NULL, NULL};
const char *FIRMWARE_SHOWWING_D[4] 		= 	{NULL, NULL, NULL, NULL};
const char *DEVICE_STATUS_SHOWWING_D[4] = 	{NULL, NULL, NULL, NULL};

//记录中括号的X坐标位置
static int move_limit_D = 0; 

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

void save_display_info_D()
{
	
	init_p3k_client("127.0.0.1", 5000);
	
	save_LAN_info_D();
	save_VIDEO_SELECT_info_D();
	save_FIREWARE_info_D();
	save_DEVICE_STATUS_info_D();
	get_current_VIDEO_OUT_info_D();
}


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
		dest[i] = blank;
	}
	
	//显示
	for (i = 1; i < 4; i++)
	{
		show_strings(x+2*i, y, dest[i], strlen(dest[i]));
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
	
					if ( MAIN_MENU_SHOWWING_D[param.x/2] == MAIN_MENU_strings_D[4]) //  VEDIO OUT RES
					{
						VEDIO_OUT_RES_SHOW_D();
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
	show_strings(0, y, LAN_MODE_strings_D[0], strlen(LAN_MODE_strings_D[0]) ); 
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
				
				if (x == 4) 			//选择static
				{					
					LAN_OPTION_SHOW_D(lan_id, 0);
				}
				//继续显示这一级目录
				get_DHCP_status(lan_id, &DHCP_status);
				clear_whole_screen();
				show_strings(0, y, LAN_MODE_strings_D[0], strlen(LAN_MODE_strings_D[0]) ); 
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
	for (i = 0; i < 4; i++)
	{
		show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]));
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
				for (i = 0; i < 4; i++)
				{
					show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]) ); 
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
	show_strings(x, y, LAN_OPTION_strings_D[offset], strlen(LAN_OPTION_strings_D[offset])); //显示标题
	show_strings(x+2, y1, string, strlen(string));
	
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
	show_strings(x, y, LAN_OPTION_strings_D[offset], strlen(LAN_OPTION_strings_D[offset])); //显示标题
	show_strings(x+2, y1, string, strlen(string));
	show_a_char(x+2, y1, string[0], 1);
	
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
				
				show_a_char(x+2, i*8, string[i], 1);//1

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
				show_a_char(x+2, i*8, string[i], 0); //取消原来位置的光标
				i++;
				if (i == 3 || i == 7 || i == 11)  //跳过'.',
					i++;
				
				if (i > 14)
					i = 0;
				show_a_char(x+2, i*8, string[i], 1); //在新位置上显示光标 //1

				break;
			}
			
			case LEFT_KEY:
			{
				show_a_char(x+2, i*8, string[i], 0); 
				i--;
				
				if (i == 3 || i == 7 || i == 11)  //跳过'.',
					i--;
				
				if (i < 0)
					i = 14;
				show_a_char(x+2, i*8, string[i], 1);//1

				break;
			}

			case ENTER_KEY:
			{
				show_a_char(x+2, i*8, string[i], 0); //取消原来位置的光标

				//通知IP已经被改了
				set_ip(lan_id, net_info_D[lan_id*3], net_info_D[lan_id*3+1], net_info_D[lan_id*3+2]);
				return; //返回到上一级目录
			}
			
		}
		
	}
}

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
	show_strings(0, y, SAVE_VIDEO_SELECT_D[0], strlen(SAVE_VIDEO_SELECT_D[0]) ); 
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
void VEDIO_OUT_RES_SHOW_D()
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
	show_strings(0, y, VIDEO_OUT_string_D[0], strlen(VIDEO_OUT_string_D[0]) ); 
	show_strings(2, y, VIDEO_OUT_string_D[1], strlen(VIDEO_OUT_string_D[1]) );
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

	for (i = 0; i < count; i++)
	{
		show_strings(i*2, y, HDCP_strings_D[i], strlen(HDCP_strings_D[i]));
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
	show_strings(0, y, FIRMWARE_strings_D[0], strlen(FIRMWARE_strings_D[0]) ); 
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
	show_strings(0, y, DEVICE_STATUS_strings_D[0], strlen(DEVICE_STATUS_strings_D[0]) ); 
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



