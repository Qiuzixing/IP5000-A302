
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

char ip_string_D[] = "192.168.060.213";
char mask_string_D[] = "255.255.255.000";
char gateway_string_D[] = "192.168.060.001";

const char* MAIN_MENU_strings_D[] = {
	"MAIN MENU", "IP SETTING", "VIDEO IN", "HDCP SETTING",
	"FIRMWARE INFO", "DEVICE STATUS", "VEDIO_OUT",
};

const char* IP_SET_strings_D[] = {
	"IP SETTING", "LAN1 SETTING", "LAN2 SETTING", "LAN3 SETTING",
};
	
const char* LAN_MODE_strings_D[] = {
	"LAN MODE", "DHCP", "STATIC",
};

const char* LAN_OPTION_strings_D[] = {
	"LAN INFO", "LAN ADDR", "LAN MASK", "LAN GATEWAY",
};

//VIDEO IN
const char* INACTIVE_VIDEO_IN_strings_D[] = {
	"VIDEO_IN", "CHANNEL 1", "CHANNEL 2", "CHANNEL 3", "CHANNEL 4", "CHANNEL 5",
};

const char* ACTIVE_VIDEO_IN_strings_D[] = {
	"*VIDEO_IN", "*CHANNEL 1", "*CHANNEL 2", "*CHANNEL 3", "*CHANNEL 4", "*CHANNEL 5",
};

const char* SAVE_VIDEO_IN_SHOWWING_D[] = {
	"VIDEO_IN", "CHANNEL 1", "CHANNEL 2", "CHANNEL 3", "CHANNEL 4", "CHANNEL 5",
};

//FIRMWARE
const char* FIRMWARE_strings_D[] = {
	"FIRMWARE_INFO", "MODULE1 1.0.1", "MODULE1 1.0.2", "MODULE1 1.0.3",
};

const char* DEVICE_STATUS_strings_D[] = {
	"DEVICE_STATUS", "STATUS1", "STATUS2", "STATUS3", 
};

const char* DHCP_strings_D[] = {
	"DHCP SETTING", "*DHCP ON", "DHCP OFF",
};

const char* VIDEO_OUT_string_D[] = {
	"VIDEO_OUT", "*4096X2160P60", "1920X1080P60", "4096X2160P60", "4096X2160P60",
};

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
//static int last_x = 0, now_x = 0;
static int move_limit_D = 0; 

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
		return;
	
	for (i = 0; i < count; i++)
	{
		dest[1+i] = src[begin_elem+i];
	}
	
	move_limit_D = i*2; //选择框向下最大移动坐标

	for (i+=1; i < 4; i++)
	{
		dest[i] = " ";
	}
	
	//显示
	for (i = 1; i < 4; i++)
	{
		if (dest[i][0] == '*')
			show_strings(x+2*i, 8, dest[i], strlen(dest[i]));
		else
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
							//first_page = 1;
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
	u8 count = sizeof(MAIN_MENU_strings_D)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
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
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[1]) == 0) // IP SETTING
					{
						//进入二级目录IP SET显示界面
						IP_SETTING_MENU_SHOW_D();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[2]) == 0) // VEDIO IN SELECET
					{
						VIDEO_IN_SELECT_SHOW_D();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[3]) == 0) // HDCP SETTING
					{
						DHCP_SHOW_D();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[4]) == 0) // FIRMWARE INFO
					{
						FIRMWARE_INFO_SHOW_D();
						break;
					}
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[5]) == 0) //  DEVICE STATUS
					{
						DEVICE_STATUS_SHOW_D();
						break;
					}
					if ( strcmp(MAIN_MENU_SHOWWING_D[param.x/2], MAIN_MENU_strings_D[6]) == 0) //  VEDIO OUT RES
					{
						VEDIO_OUT_RES_SHOW_D();
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
	int last_page = 0, first_page = 0;

	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;
	//param.first_page = last_page;

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
				do {
					if (strcmp(IP_SET_SHOWWING_D[param.x/2], IP_SET_strings_D[1]) == 0) 
					{
						LAN_MODE_MENU_SHOW_D();
						break;
					}
					
					if (strcmp(IP_SET_SHOWWING_D[param.x/2], IP_SET_strings_D[2]) == 0)
					{
						LAN_MODE_MENU_SHOW_D();
						break;
					}
					
					if (strcmp(IP_SET_SHOWWING_D[param.x/2], IP_SET_strings_D[3]) == 0)
					{
						LAN_MODE_MENU_SHOW_D();		
						break;
					}
				} while(0);
					
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
void LAN_MODE_MENU_SHOW_D(void) //DHCP OR STATIC
{
	//su8 count = sizeof(LAN_MODE_strings_D)/(sizeof(char*)); //IP_SET_strings的元素个数
 
	int y = 16; //有*要显示
	int x = 2; //方括号位置

	clear_whole_screen();
	show_strings(0, y, LAN_MODE_strings_D[0], strlen(LAN_MODE_strings_D[0]) ); 
	show_menu_info_D(y, 1, LAN_MODE_SHOWWING_D, LAN_MODE_strings_D, 2);
	show_square_breakets(x);

	int key = 0;
	while (1)
	{		
		key = recv_key_info_D();
		switch (key)
		{
			//if (g_down_key == 1) 
			case DOWN_KEY:
			{
				x += 2;
				if (x <= 4)
				{
					show_square_breakets(x);
				}
				else
				{
					x = 4;
				}
				break;
			}
			
			//if (g_up_key == 1)
			case UP_KEY:
			{
				x -= 2;
				if (x >= 2)
				{
					show_square_breakets(x);
				}
				else
				{
					x = 2;
				}
				break;
			}
			
			//if (g_right_key == 1 || g_ok_key == 1) 		//进入子目录
			case RIGHT_KEY:
			case ENTER_KEY:
			{				
				u8 m;
				show_a_star(x);			//[]在哪里，* 就显示在哪里行
				if (x == 2)
				{
					//显示内容变化
					LAN_MODE_SHOWWING_D[1] = LAN_MODE_strings_D[1] = "*DHCP";
					LAN_MODE_SHOWWING_D[2] = LAN_MODE_strings_D[2] = "STATIC";
					
					DHCP_strings_D[1] = "*DHCP ON";
					DHCP_strings_D[2] = "DHCP 0FF";
					//通知函数
					
				}
				
				if (x == 4) 			//选择static
				{
					//显示内容变化
					LAN_MODE_SHOWWING_D[1] = LAN_MODE_strings_D[1] = "DHCP";
					LAN_MODE_SHOWWING_D[2] = LAN_MODE_strings_D[2] = "*STATIC";
					
					DHCP_strings_D[1] = "DHCP ON";
					DHCP_strings_D[2] = "*DHCP 0FF";
					
					//通知函数
					
					//这里设置IP 
					LAN_OPTION_SHOW_D();
				}

				//继续显示这一级目录
				int i;
				for (i = 0; i < 3; i++)
				{
					if (LAN_MODE_SHOWWING_D[i][0] == '*')
						show_strings(i*2, y-8, LAN_MODE_SHOWWING_D[i], strlen(LAN_MODE_SHOWWING_D[i]) );
					else
						show_strings(i*2, y, LAN_MODE_SHOWWING_D[i], strlen(LAN_MODE_SHOWWING_D[i]) ); 
				}
				
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
void LAN_OPTION_SHOW_D() //ip mask gateway
{	
	int y = 16;
	int x = 2; //方括号位置

	u8 i, begin_num = 1;

	clear_whole_screen();
	for (i = 0; i < 4; i++)
	{
		show_strings(i*2, y, LAN_OPTION_strings_D[i], strlen(LAN_OPTION_strings_D[i]) );
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
				do {
					//ip设置
					if (x == 2) 
					{
						LAN_INFO_SET_D(0, ip_string_D, strlen(ip_string_D));
						break;
					}
					if (x == 4) 
					{
						LAN_INFO_SET_D(1, mask_string_D, strlen(mask_string_D));
						break;
					}
					if (x == 6) 
					{
						LAN_INFO_SET_D(2, gateway_string_D, strlen(gateway_string_D));
						break;
					}
				} while (0);
				clear_whole_screen();
				//继续显示这一级目录
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
				clear_whole_screen();
				return;
			}
		}

	}
	
}

//五级目录 LAN INFO set
void LAN_INFO_SET_D(u8 offset, char *string, u8 lenth)  //注意ip显示的起始位置，
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
				if (string[i] < '9')
				{
					string[i]++;
				}
				else
				{
					string[i] = '0';
				}
				show_a_char(x+2, i*8, string[i], 1);//1

				break;
			}
			
			case UP_KEY:
			{
				if (string[i] > '0')
				{
					string[i]--;
				}
				else
				{
					string[i] = '9';
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
					return;
					//i = 0;
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
				//这里需要通知IP已经被改了
				
				
				return; //返回到上一级目录
			}
			
		}
		
	}
}

//二级目录， 2.2EDID_SET
void VIDEO_IN_SELECT_SHOW_D(void)
{
	u8 count = sizeof(SAVE_VIDEO_IN_SHOWWING_D)/(sizeof(char*));
	int p = 4; 
	int y = 16; //有*要显示
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();  //新一级的目录，清屏
	show_strings(0, y, SAVE_VIDEO_IN_SHOWWING_D[0], strlen(SAVE_VIDEO_IN_SHOWWING_D[0]) ); 
	show_menu_info_D(y, 1, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_IN_SHOWWING_D, count>4? 3 : count-1);
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
				param = down_up_respond_D(count, param, VIDEO_IN_SHOWWING_D, SAVE_VIDEO_IN_SHOWWING_D, key);
				break;
			}
			
			//if (g_right_key == 1 || g_ok_key == 1) //标记*号
			case RIGHT_KEY:
			case ENTER_KEY:
			{				
				show_a_star(param.x); 
				
				int i, n;
				for (i = 1; i < count; i++)
				{
					if (strstr(ACTIVE_VIDEO_IN_strings_D[i], VIDEO_IN_SHOWWING_D[param.x/2]))
					{
						//重新换一下牌，
						for (n = 0; n < count; n++)
						{
							if (n == i)
								SAVE_VIDEO_IN_SHOWWING_D[n] = ACTIVE_VIDEO_IN_strings_D[n];
							else
								SAVE_VIDEO_IN_SHOWWING_D[n] = INACTIVE_VIDEO_IN_strings_D[n];
							//通知函数
						}
						break;
					}
				}
				
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
	
	u8 count = sizeof(VIDEO_OUT_string_D)/(sizeof(char*));
	int p = 4; 
	int y = 16; //有*要显示
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();  //新一级的目录，清屏
	show_strings(0, y, VIDEO_OUT_string_D[0], strlen(VIDEO_OUT_string_D[0]) ); 
	show_menu_info_D(y, 1, VIDEO_OUT_SHOWWING_D, VIDEO_OUT_string_D, count>4? 3 : count-1);
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
				param = down_up_respond_D(count, param, VIDEO_OUT_SHOWWING_D, VIDEO_OUT_string_D, key);
				break;
			}
			
			case RIGHT_KEY: //标记*号
			case ENTER_KEY:
			{
				break;
			}
			
			case LEFT_KEY:
			{
				return; //返回上一级目录
			}
		}

	}

}

//二级目录，DHCP show
void DHCP_SHOW_D() //仅展示
{
	u8 y = 16;
	clear_whole_screen();

	int i;
	for (i = 0; i < 3; i++)
	{
		if (DHCP_strings_D[i][0] == '*')
			show_strings(i*2, y-8, DHCP_strings_D[i], strlen(DHCP_strings_D[i]));
		else
			show_strings(i*2, y, DHCP_strings_D[i], strlen(DHCP_strings_D[i]));
	}
	
	int key = 0;
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
	u8 count = sizeof(FIRMWARE_strings_D)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;
	//param.first_page = last_page;

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
	u8 count = sizeof(DEVICE_STATUS_strings_D)/(sizeof(char*));
	int p = 4; 
	int y = 16;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, DEVICE_STATUS_strings_D[0], strlen(DEVICE_STATUS_strings_D[0]) ); 
	show_menu_info_D(y, 1, DEVICE_STATUS_SHOWWING_D, DEVICE_STATUS_strings_D, count>4? 3 : count-1);
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



