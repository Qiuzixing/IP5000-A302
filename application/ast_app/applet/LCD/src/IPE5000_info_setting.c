
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

pthread_mutex_t g_lock_E;

enum 
{
	ENTER_KEY = 1,
	UP_KEY,
	DOWN_KEY,
	LEFT_KEY,
	RIGHT_KEY,
};


char ip_string_E[] = "192.168.060.213";
char mask_string_E[] = "255.255.255.000";
char gateway_string_E[] = "192.168.060.001";


const char* MAIN_MENU_strings_E[] = {
	"MAIN MENU", "IP SETTING", "EDID SETTING", "HDCP SETTING",
	"FIRMWARE INFO", "DEVICE STATUS",
};

const char* IP_SET_strings_E[] = {
	"IP SETTING", "LAN1 SETTING", "LAN2 SETTING", "LAN3 SETTING",
};
	
const char* LAN_MODE_strings_E[] = {
	"LAN MODE", "DHCP", "STATIC",
};

const char* LAN_OPTION_strings_E[] = {
	"LAN INFO", "LAN ADDR", "LAN MASK", "LAN GATEWAY",
};


const char* INACTIVE_EDID_strings_E[] = {
	"EDID SETTING", "PASS THRU", "EDID_2160P30", "EDID_SONY",
};

const char* ACTIVE_EDID_strings_E[] = {
	"*EDID SETTING", "*PASS THRU", "*EDID_2160P30", "*EDID_SONY",
};

const char* SAVE_EDID_SHOWWING_E[] = {
	"EDID SETTING", "PASS THRU", "EDID_2160P30", "EDID_SONY",
};

const char* FIRMWARE_strings_E[] = {
	"FIRMWARE INFO", "MODULE1 1.0.1", "MODULE1 1.0.2", "MODULE1 1.0.3",
};

const char* DEVICE_STATUS_strings_E[] = {
	"DEVICE_STATUS", "STATUS1", "STATUS2", "STATUS3", 
};

const char* DHCP_strings_E[] = {
	"DHCP SETTING", "DHCP ON", "DHCP OFF",
};

typedef struct
{
	int x;          //x坐标
	int y;			//y坐标
	int p; 			//记录字符数组中 要显示在第二行，即第一个数据栏的位置。
	int last_page;  //是否到达最后一页（翻页）
}info_param;

//记录正在屏幕显示的字符串，一个屏幕最多显示4行.
const char *MAIN_MENU_SHOWWING_E[4]	= 	{NULL, NULL, NULL, NULL}; 
const char *IP_SET_SHOWWING_E[4] 		= 	{NULL, NULL, NULL, NULL};
const char *LAN_MODE_SHOWWING_E[4]	= 	{NULL, NULL, NULL, NULL};
const char *LAN1_OPTION_SHOWWING_E[4] =	{NULL, NULL, NULL, NULL};
const char *EDID_SHOWWING_E[4] 		= 	{NULL, NULL, NULL, NULL};
const char *FIRMWARE_SHOWWING_E[4] 	= 	{NULL, NULL, NULL, NULL};
const char *DEVICE_STATUS_SHOWWING_E[4] = {NULL, NULL, NULL, NULL};

//记录中括号的X坐标位置
static int move_limit_E = 0; 

/*
	y: 显示起始列坐标,要取8的整数倍, 0, 8, 16 .... 8*15
	begin_num: 要显示src中要元素起始位
	dest: 记录src哪些元素被显示
	src: 从src中拿取字符串显示
	count: 从src的拿去元素的个数, 最多只能拿三个
*/
static void show_menu_info_E(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
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
	
	move_limit_E = i*2; //选择框向下最大移动坐标

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

static info_param down_up_respond_E(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
{
	int x = param.x;
	int y = param.y;
	int p = param.p;
	int last_page = param.last_page;

	switch (key)
	{
		case DOWN_KEY: //按下键
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

		case UP_KEY: //上键
		{
			x -= 2;
			//光标不移动才翻页
			if (x < 2)
			{
				x = 2;
				if ((count > 4) && (strcmp(showwing_strings[1], src_string[1])) != 0) //要超过4行才能翻页，
				{
					
					if ( p >= 4)
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

static int recv_init_E()
{
	int err = 0;
	pthread_mutex_init(&g_lock_E, NULL);
	
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

static int recv_key_info_E()
{
	int err = 0;
	struct msg_buf msg;

	pthread_mutex_lock(&g_lock_E);
	err = msg_recv_state(&msg);
	if (err == -1)
	{
		printf("msg_recv_state fail");
		return -1;
	}
	pthread_mutex_unlock(&g_lock_E);
	
	return msg.mtext[0];
}

//一级目录       1.主菜单显示
int IPE5000_MAIN_MENU_SHOW(void)
{
	printf("this is IPE5000\n");
	u8 count = sizeof(MAIN_MENU_strings_E)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	int key = 0;  
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	if (recv_init_E() != 0)
	{
		printf("recv_init_E() fail - [%s:%d]\n", __func__, __LINE__);
		return -1;
	}

	clear_whole_screen();
	show_strings(0, y, MAIN_MENU_strings_E[0], strlen(MAIN_MENU_strings_E[0]) ); 
	show_menu_info_E(y, 1, MAIN_MENU_SHOWWING_E, MAIN_MENU_strings_E, count>4? 3 : count-1);
	show_square_breakets(x);

	while (1)
	{
		key = recv_key_info_E();

		switch (key)
		{
			case DOWN_KEY:
			case UP_KEY:
			{
				param = down_up_respond_E(count, param, MAIN_MENU_SHOWWING_E, MAIN_MENU_strings_E, key);		
				break;
			}
			case RIGHT_KEY:
			case ENTER_KEY: //进入子目录
			{
				do {
					if ( strcmp(MAIN_MENU_SHOWWING_E[param.x/2], MAIN_MENU_strings_E[1]) == 0) // "IP SETTING"
					{
						//进入二级目录IP SET显示界面
						IP_SETTING_MENU_SHOW_E();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_E[param.x/2], MAIN_MENU_strings_E[2]) == 0) // EDID SETTING"
					{
						EDID_SET_E();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_E[param.x/2], MAIN_MENU_strings_E[3]) == 0) // HDCP SETTING
					{
						DHCP_SHOW_E();
						break;
					}
					
					if ( strcmp(MAIN_MENU_SHOWWING_E[param.x/2], MAIN_MENU_strings_E[4]) == 0) // FIRMWARE INFO
					{
						FIRMWARE_INFO_SHOW_E();
						break;
					}
					if ( strcmp(MAIN_MENU_SHOWWING_E[param.x/2], MAIN_MENU_strings_E[5]) == 0) //  DEVICE STATUS
					{
						DEVICE_STATUS_SHOW_E();
						break;
					}
					
				}while(0);
				
				//从子目录出来， 回复显示这一级目录
				clear_whole_screen();
				int i;
				for (i = 0; i < 4; i++)
				{
					show_strings(i*2, y, MAIN_MENU_SHOWWING_E[i], strlen(MAIN_MENU_SHOWWING_E[i]) ); 
				}
				show_square_breakets(param.x);
				
				for (i = 3; i > 0; i--)
				{
					//在show_menu_info_E设置了如果到达最后面，能显示的字符串已经少于3个，那么MAIN_MENU_SHOWWING_E剩余成员的就用空格代替 ，
					if (strlen(MAIN_MENU_SHOWWING_E[i]) != 1) 
					{
						move_limit_E = i*2;
						break;
					}
				}
				
				break;
			}
			
			case LEFT_KEY: //这里最高一级目录，没有上一级目录
			{
				break;
			}
		}
	}
}


//二级目录      2.1 IP SETTING菜单栏显示
static void IP_SETTING_MENU_SHOW_E(void)
{
	u8 count = sizeof(IP_SET_strings_E)/(sizeof(char*)); //IP_SET_strings的元素个数
	int p = 4; 
	int y = 16;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;

	int key = 0;  
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, IP_SET_strings_E[0], strlen(IP_SET_strings_E[0]) ); 
	show_menu_info_E(y, 1, IP_SET_SHOWWING_E, IP_SET_strings_E, count>4? 3 : count-1);
	show_square_breakets(x);

	while (1)
	{
		key = recv_key_info_E();
		switch (key)
		{
			case DOWN_KEY:
			case UP_KEY:
			{
				param = down_up_respond_E(count, param, IP_SET_SHOWWING_E, IP_SET_strings_E, key);
				break;
			}

			case RIGHT_KEY: //进入子目录
			case ENTER_KEY:
			{	
				do {			
					if (strcmp(IP_SET_SHOWWING_E[param.x/2], IP_SET_strings_E[1]) == 0) 
					{
						LAN_MODE_MENU_SHOW_E();
						break;
					}
					
					if (strcmp(IP_SET_SHOWWING_E[param.x/2], IP_SET_strings_E[2]) == 0)
					{
						LAN_MODE_MENU_SHOW_E();
						break;
					}
					
					if (strcmp(IP_SET_SHOWWING_E[param.x/2], IP_SET_strings_E[3]) == 0)
					{
						LAN_MODE_MENU_SHOW_E();	
						break;
					}
				} while (0);
				
				//从子目录出来，继续显示
				clear_whole_screen();
				int i;
				for (i = 0; i < 4; i++)
				{
					show_strings(i*2, y, IP_SET_SHOWWING_E[i], strlen(IP_SET_SHOWWING_E[i]) ); 
				}
				show_square_breakets(param.x);
				
				for (i = 3; i > 0; i--)
				{
					if (strlen(IP_SET_SHOWWING_E[i]) != 1) 
					{
						move_limit_E = i*2;
						break;
					}
				}
				
				break;
			}
		
			case LEFT_KEY: //返回上一级目录
			{	
				return;
			}
		}
		
	} 

}

//三级目录 LAN MODE 菜单栏显示
static void LAN_MODE_MENU_SHOW_E(void) //DHCP OR STATIC
{
	int key = 0;
 
	int y = 16; //有*要显示
	int x = 2; //方括号位置

	clear_whole_screen();
	show_strings(0, y, LAN_MODE_strings_E[0], strlen(LAN_MODE_strings_E[0]) ); 
	show_menu_info_E(y, 1, LAN_MODE_SHOWWING_E, LAN_MODE_strings_E, 2);
	show_square_breakets(x);
	
	while (1)
	{
		key = recv_key_info_E();

		switch (key)
		{
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
			
			case RIGHT_KEY:		//进入子目录
			case ENTER_KEY:
			{
				u8 m;
				show_a_star(x);			//[]在哪里，* 就显示在哪里行
				if (x == 2)
				{
					//显示内容变化
					LAN_MODE_SHOWWING_E[1] = LAN_MODE_strings_E[1] = "*DHCP";
					LAN_MODE_SHOWWING_E[2] = LAN_MODE_strings_E[2] = "STATIC";
					
					DHCP_strings_E[1] = "*DHCP ON";
					DHCP_strings_E[2] = "DHCP 0FF";
					//通知函数
					
				}
				
				if (x == 4) 			//选择static
				{
					//显示内容变化
					LAN_MODE_SHOWWING_E[1] = LAN_MODE_strings_E[1] = "DHCP";
					LAN_MODE_SHOWWING_E[2] = LAN_MODE_strings_E[2] = "*STATIC";
					
					DHCP_strings_E[1] = "DHCP ON";
					DHCP_strings_E[2] = "*DHCP 0FF";
					
					//通知函数
					
					//这里设置IP 
					LAN_OPTION_SHOW_E();
				}
				
				//继续显示这一级目录
				int i;
				for (i = 0; i < 3; i++)
				{
					if (LAN_MODE_SHOWWING_E[i][0] == '*')
						show_strings(i*2, y-8, LAN_MODE_SHOWWING_E[i], strlen(LAN_MODE_SHOWWING_E[i]) );
					else
						show_strings(i*2, y, LAN_MODE_SHOWWING_E[i], strlen(LAN_MODE_SHOWWING_E[i]) ); 
				}
				
				show_square_breakets(x);

				break;
			}
			
			case LEFT_KEY://返回上一级
			{
				return;
			}
		}
 }
}

//四级目录 
static void LAN_OPTION_SHOW_E() //ip mask gateway
{	
	int y = 16;
	int x = 2; //方括号位置
	int key = 0;
	u8 i, begin_num = 1;

	clear_whole_screen();
	for (i = 0; i < 4; i++)
	{
		show_strings(i*2, y, LAN_OPTION_strings_E[i], strlen(LAN_OPTION_strings_E[i]) );
	}
	show_square_breakets(x);
		
	while (1)
	{
		key = recv_key_info_E();
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
						LAN_INFO_SET_E(0, ip_string_E, strlen(ip_string_E));
						break;
					}
					if (x == 4) 
					{
						LAN_INFO_SET_E(1, mask_string_E, strlen(mask_string_E));
						break;
					}
					if (x == 6) 
					{
						LAN_INFO_SET_E(2, gateway_string_E, strlen(gateway_string_E));
						break;
					}
				} while (0);
					
				clear_whole_screen();
				//继续显示这一级目录
				int i;
				for (i = 0; i < 4; i++)
				{
					show_strings(i*2, y, LAN_OPTION_strings_E[i], strlen(LAN_OPTION_strings_E[i]) ); 
				}
				
				show_square_breakets(x);

				break;
			}
				
			case LEFT_KEY: //上一级目录
			{
				clear_whole_screen();
				return;
			}
		}

	}
	
}

//五级目录 LAN INFO set
static void LAN_INFO_SET_E(u8 offset, char *string, u8 lenth)  //注意ip显示的起始位置，
{
	u8 x = 0, y = 16, y1 = 0;
	int i = 0;
	int key = 0;
	
	clear_whole_screen();
	show_strings(x, y, LAN_OPTION_strings_E[offset], strlen(LAN_OPTION_strings_E[offset])); //显示标题
	show_strings(x+2, y1, string, strlen(string));
	show_a_char(x+2, y1, string[0], 1);
	//显示光标
	while (1)
	{
		key = recv_key_info_E();
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

			case RIGHT_KEY: //总共15位
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
				//通知函数
				
				return;
			}
		}
		
	}
}


//二级目录， 2.2EDID_SET
static void EDID_SET_E(void)
{
	u8 count = sizeof(SAVE_EDID_SHOWWING_E)/(sizeof(char*));
	int p = 4; 
	int y = 16; //有*要显示
	int x = 2; //方括号位置
	int last_page = 0;
	int key = 0;  
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();  //新一级的目录，清屏
	show_strings(0, y, SAVE_EDID_SHOWWING_E[0], strlen(SAVE_EDID_SHOWWING_E[0]) ); 
	show_menu_info_E(y, 1, EDID_SHOWWING_E, SAVE_EDID_SHOWWING_E, count>4? 3 : count-1);
	show_square_breakets(x);

	//下次进来怎么知道上一次的*位置
	while (1)
	{
		key = recv_key_info_E();
		switch (key)
		{
			case DOWN_KEY:
			case UP_KEY:
			{
				param = down_up_respond_E(count, param, EDID_SHOWWING_E, SAVE_EDID_SHOWWING_E, key);
				break;
			}
			
			case RIGHT_KEY: //标记*号
			case ENTER_KEY:
			{
				show_a_star(param.x); 
				
				int i, n;
				for (i = 1; i < count; i++)
				{
					if (strstr(ACTIVE_EDID_strings_E[i], EDID_SHOWWING_E[param.x/2]))
					{
						//重新换一下牌，
						for (n = 0; n < count; n++)
						{
							if (n == i)
								SAVE_EDID_SHOWWING_E[n] = ACTIVE_EDID_strings_E[n];
							else
								SAVE_EDID_SHOWWING_E[n] = INACTIVE_EDID_strings_E[n];
							//通知函数
						}
							
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

//二级目录，DHCP show
static void DHCP_SHOW_E() //仅展示
{
	u8 y = 16;
	int key = 0;
	clear_whole_screen();

	int i;
	for (i = 0; i < 3; i++)
	{
		if (DHCP_strings_E[i][0] == '*')
			show_strings(i*2, y-8, DHCP_strings_E[i], strlen(DHCP_strings_E[i]));
		else
			show_strings(i*2, y, DHCP_strings_E[i], strlen(DHCP_strings_E[i]));
	}
	
	while (1)
	{
		key = recv_key_info_E();
		
		if (key == LEFT_KEY)
		{
			break;
		}
	}
}

//二级目录
static void FIRMWARE_INFO_SHOW_E() //仅展示
{
	u8 count = sizeof(FIRMWARE_strings_E)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0;
	int key = 0;  

	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, FIRMWARE_strings_E[0], strlen(FIRMWARE_strings_E[0]) ); 
	show_menu_info_E(y, 1, FIRMWARE_SHOWWING_E, FIRMWARE_strings_E, count>4? 3 : count-1);
	show_square_breakets(x);
	
	while (1)
	{
		key = recv_key_info_E();
		switch (key)
		{
			case DOWN_KEY:
			case UP_KEY:
			{
				param = down_up_respond_E(count, param, FIRMWARE_SHOWWING_E, FIRMWARE_strings_E, key);
				break;
			}
			
			case LEFT_KEY:  //返回上一级目录
			{
				return;
			}
		}
	}

}

//二级目录
static void DEVICE_STATUS_SHOW_E() //仅展示
{
	u8 count = sizeof(DEVICE_STATUS_strings_E)/(sizeof(char*));
	int p = 4; 
	int y = 16;
	int x = 2; //方括号位置
	int last_page = 0;
	int key = 0;  

	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, DEVICE_STATUS_strings_E[0], strlen(DEVICE_STATUS_strings_E[0]) ); 
	show_menu_info_E(y, 1, DEVICE_STATUS_SHOWWING_E, DEVICE_STATUS_strings_E, count>4? 3 : count-1);
	show_square_breakets(x);

	while (1)
	{
		key = recv_key_info_E();
		switch (key)
		{
			case DOWN_KEY:
			case UP_KEY:
			{
				param = down_up_respond_E(count, param, DEVICE_STATUS_SHOWWING_E, DEVICE_STATUS_strings_E, key);
				break;
			}

			case LEFT_KEY:  //返回上一级目录
			{
				return;
			}
		}
	}
	
}


