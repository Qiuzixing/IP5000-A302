
//IPE5000P-A30

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

pthread_mutex_t g_lock;

//方向
enum 
{
	ENTER_KEY = 1,
	UP_KEY,
	DOWN_KEY,
	LEFT_KEY,
	RIGHT_KEY,
};

char ip_string[] = "192.168.060.213";
char mask_string[] = "255.255.255.0";
char gateway_string[] = "192.168.060.1";

const char* MAIN_MENU_strings[] = {
	"MAIN MENU", "IP SETTING", "INPUT SELECT", "EDID SETTING", "HDCP SETTING",
	"FIRMWARE INFO", "DEVICE STATUS",
};

const char* INPUT_SELECT_strings[] = {
	"INPUT SELECT", "HDMI IN1", "HDMI IN2", "USB IN3",
};
const char *ACTIVE_INPUT[] = {
	"INPUT SELECT", "HDMI IN1", "HDMI IN2", "USB IN3",
};

const char* INACTIVE_INPUT[] = {
	"INPUT SELECT", "*HDMI IN1", "*HDMI IN2", "*USB IN3",
};


const char* IP_SET_strings[] = {
	"IP SETTING", "LAN1 SETTING", "LAN2 SETTING", "LAN3 SETTING",
};
	
const char* LAN_MODE_strings[] = {
	"LAN1 MODE", "DHCP", "STATIC",
};

const char* LAN1_OPTION_strings[] = {
	"LAN1 ADDR", "LAN1 MASK", "LAN1 GATEWAY",
};

const char* INACTIVE_EDID_strings[] = {
	"EDID SETTING", "PASS THRU", "EDID_2160P30", "EDID_SONY",
};

const char* ACTIVE_EDID_strings[] = {
	"*EDID SETTING", "*PASS THRU", "*EDID_2160P30", "*EDID_SONY",
};

const char* SAVE_EDID_strings[] = {
	"EDID SETTING", "PASS THRU", "EDID_2160P30", "EDID_SONY",
};

const char* FIRMWARE_strings[] = {
	"FIRMWARE INFO", "MODULE1 1.0.1", "MODULE1 1.0.2", "MODULE1 1.0.3",
};

const char* DEVICE_STATUS_strings[] = {
	"DEVICE_STATUS", "STATUS1", "STATUS2", "STATUS3", 
};

const char* DHCP_strings[] = {
	"DHCP SETTING", "DHCP ON", "DHCP OFF",
};

typedef struct
{
	int x;          //x坐标
	int y;			//y坐标
	int p; 			//记录字符数组中 要显示在第二行，即第一个数据栏的位置。
	int last_page;  //是否到达最后一页（翻页）
	//int first_page; //是否达到第一页（翻页）
}info_param;

//记录正在屏幕显示的字符串，一个屏幕最多显示4行.
const char *MAIN_MENU_SHOWWING[4]	= 	{NULL, NULL, NULL, NULL}; 
const char *INPUT_SELECT_SHOWWING[4] =  {NULL, NULL, NULL, NULL};
const char *IP_SET_SHOWWING[4] 		= 	{NULL, NULL, NULL, NULL};
const char *LAN_MODE_SHOWWING[4]	= 	{NULL, NULL, NULL, NULL};
const char *LAN1_OPTION_SHOWWING[4] =	{NULL, NULL, NULL, NULL};
const char *EDID_SHOWWING[4] 		= 	{NULL, NULL, NULL, NULL};
const char *FIRMWARE_SHOWWING[4] 	= 	{NULL, NULL, NULL, NULL};
const char *DEVICE_STATUS_SHOWWING[4] = {NULL, NULL, NULL, NULL};

//记录中括号的X坐标位置
static int move_limit = 0; 

/*
	y: 显示起始列坐标,要取8的整数倍, 0, 8, 16 .... 8*15
	begin_num: 要显示src中要元素起始位
	dest: 记录src哪些元素被显示
	src: 从src中拿取字符串显示
	count: 从src的拿去元素的个数, 最多只能拿三个
*/
void show_menu_info(int y, u8 begin_elem, const char *dest[], const char *src[], u8 count) //2
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
	
	move_limit = i*2; //选择框向下最大移动坐标

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

info_param down_up_respond(int count, info_param param, const char* showwing_strings[], const char* src_string[], int key)
{
	int x = param.x;
	int y = param.y;
	int p = param.p;
	int last_page = param.last_page;

	if (key == DOWN_KEY) //按下键
	{
		//光标不移动才翻页
		x += 2;	
		if (x > move_limit) //X坐标超过最大值，取最大值
		{
			x = move_limit;
			
			if (count > 4)//要超过4行才能翻页，
			{
				//p被上行键更改了 或者count <= 7时，p不会被上行键更改，但页面已经被向上翻页了，通过MAIN_MENU_SHOWWING[3],MAIN_MENU_strings[3]来判断是否是最后一页。
				if ( (p + 3 < count) || (strcmp(showwing_strings[3], src_string[3]) == 0))
				{
					last_page = 0;	
				}
				
				if ((count - p > 0) && last_page == 0) //还有也可以翻，且没有翻到最后一页
				{
					if (p + 3 < count) 
					{	
						clear_three_line();
						show_menu_info(y, p, showwing_strings, src_string, 3); //数组是从0开始
						p += 3;
					}
					else 
					{
						clear_three_line();
						show_menu_info(y, p, showwing_strings, src_string, count-p);
						last_page = 1;
					}
					x = 2;
				}
			}
		}
		show_square_breakets(x);
		
	}

	if (key == UP_KEY) //上键
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
						show_menu_info(y, p, showwing_strings, src_string, 3);
					}
					else //p==4
					{
						clear_three_line();
						show_menu_info(y, p-3, showwing_strings, src_string, 3);
					}
					x = 2;
				}
			}
		}
		show_square_breakets(x);
	}

	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;
	
	return param;
}

static int recv_init()
{
	int err = 0;
	pthread_mutex_init(&g_lock, NULL);
	
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

static int recv_key_info()
{
	int err = 0;
	struct msg_buf msg;

	pthread_mutex_lock(&g_lock);
	err = msg_recv_state(&msg);
	if (err == -1)
	{
		printf("msg_recv_state fail");
		return -1;
	}
	pthread_mutex_unlock(&g_lock);
	
	return msg.mtext[0];
}

//一级目录       1.主菜单显示
int IPE5000P_MAIN_MENU_SHOW(void)
{
	printf("this is IPE5000P\n");
	u8 count = sizeof(MAIN_MENU_strings)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	if (recv_init() != 0)
	{
		printf("recv_init() fail - [%s:%d]\n", __func__, __LINE__);
		return -1;
	}
	
	clear_whole_screen();
	show_strings(0, y, MAIN_MENU_strings[0], strlen(MAIN_MENU_strings[0]) ); 
	show_menu_info(y, 1, MAIN_MENU_SHOWWING, MAIN_MENU_strings, count>4? 3 : count-1);
	show_square_breakets(x);
	
	int key = 0;
	while (1)
	{
		key = recv_key_info();
		
		if (key == DOWN_KEY || key == UP_KEY)
		{
			param = down_up_respond(count, param, MAIN_MENU_SHOWWING, MAIN_MENU_strings, key);		
		}
		if (key == RIGHT_KEY || key == ENTER_KEY) //进入子目录
		{
			do {
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[1]) == 0) // "IP SETTING"
				{
					//进入二级目录IP SET显示界面
					IP_SETTING_MENU_SHOW();
					break;
				}
				
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[2]) == 0) //INPUT SELECT
				{
					INPUT_SELECT_SHOW();
					break;
				}
				
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[3]) == 0) //  EDID SETTING"
				{
					EDID_SET();
					break;
				}
				
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[4]) == 0) // HDCP SETTING
				{
					DHCP_SHOW();
					break;
				}
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[5]) == 0) //  FIRMWARE INFO
				{
					FIRMWARE_INFO_SHOW();
					break;
				}
				if ( strcmp(MAIN_MENU_SHOWWING[param.x/2], MAIN_MENU_strings[5]) == 0) //  DEVICE STATUS
				{
					DEVICE_STATUS_SHOW();
					break;
				}
				
			}while(0);
			
			//从子目录出来， 回复显示这一级目录
			int i;
			for (i = 0; i < 4; i++)
			{
				show_strings(i*2, y, MAIN_MENU_SHOWWING[i], strlen(MAIN_MENU_SHOWWING[i]) ); 
			}
			
			show_square_breakets(param.x);
			
		}
		
		if (key == LEFT_KEY) //这里最高一级目录，没有上一级目录
		{
			
		}
			
	}
}

//二级目录
void INPUT_SELECT_SHOW()
{
	u8 count = sizeof(INPUT_SELECT_strings)/(sizeof(char*)); //IP_SET_strings的元素个数
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
	show_strings(0, y, INPUT_SELECT_strings[0], strlen(INPUT_SELECT_strings[0]) ); 
	show_menu_info(y, 1, INPUT_SELECT_SHOWWING, INPUT_SELECT_strings, count>4? 3 : count-1);
	show_square_breakets(x);

	int key = 0;
	while (1)
	{
		key = recv_key_info();
		if (key == DOWN_KEY || key == UP_KEY)
		{
			param = down_up_respond(count, param, INPUT_SELECT_SHOWWING, INPUT_SELECT_strings, key);
		}
		if (key == RIGHT_KEY || key == ENTER_KEY)//确认
		{
			show_a_star(param.x); 
			
			int i, n;
			for (i = 0; i < count; i++)
			{
				if (strstr(ACTIVE_INPUT[i], INPUT_SELECT_SHOWWING[x/2]))
				{
					//重新换一下牌，
					for (n = 0; n < count; n++)
					{
						if (n == i)
							INPUT_SELECT_strings[n] = ACTIVE_INPUT[n];
						else
							INPUT_SELECT_strings[n] = INACTIVE_INPUT[n];
						//通知函数
						
					}
						
					break;
				}
			}			
		}

		if (key == LEFT_KEY)
		{
			clear_whole_screen();
			break;
		}

	}
	

}

//二级目录      2.1 IP SETTING菜单栏显示
void IP_SETTING_MENU_SHOW(void)
{
	u8 count = sizeof(IP_SET_strings)/(sizeof(char*)); //IP_SET_strings的元素个数
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;

	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, IP_SET_strings[0], strlen(IP_SET_strings[0]) ); 
	show_menu_info(y, 1, IP_SET_SHOWWING, IP_SET_strings, count>4? 3 : count-1);
	show_square_breakets(x);

	int key = 0;
	while (1)
	{
		key = recv_key_info();

		if (key == DOWN_KEY || key == UP_KEY)
		{
			param = down_up_respond(count, param, IP_SET_SHOWWING, IP_SET_strings, key);
		}
		
		if (key == RIGHT_KEY || key == ENTER_KEY)//进入子目录
		{			
			if (strcmp(IP_SET_SHOWWING[param.x/2], IP_SET_strings[1]) == 0) 
			{
				LAN_MODE_MENU_SHOW();
			}
			
			if (strcmp(IP_SET_SHOWWING[param.x/2], IP_SET_strings[2]) == 0)
			{
				LAN_MODE_MENU_SHOW();
			}
			
			if (strcmp(IP_SET_SHOWWING[param.x/2], IP_SET_strings[3]) == 0)
			{
				LAN_MODE_MENU_SHOW();				
			}
			
			//从子目录出来，继续显示
			int i;
			for (i = 0; i < 4; i++)
			{
				show_strings(i*2, y, IP_SET_SHOWWING[i], strlen(IP_SET_SHOWWING[i]) ); 
			}
			
			show_square_breakets(param.x);
		}
	
		if (key == LEFT_KEY) //返回上一级目录
		{	
			clear_whole_screen();
			break;
		}
		
	} 

}

//三级目录 LAN MODE 菜单栏显示
void LAN_MODE_MENU_SHOW(void) //DHCP OR STATIC
{
	//su8 count = sizeof(LAN_MODE_strings)/(sizeof(char*)); //IP_SET_strings的元素个数
 
	int y = 16; //有*要显示
	int x = 2; //方括号位置

	clear_whole_screen();
	show_strings(0, y, LAN_MODE_strings[0], strlen(LAN_MODE_strings[0]) ); 
	show_menu_info(y, 1, LAN_MODE_SHOWWING, LAN_MODE_strings, 2);
	show_square_breakets(x);

	int key = 0;  
	while(1)
	{
		key = recv_key_info();
		
		if (key == DOWN_KEY) 
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
		}
		
		if (key == UP_KEY)
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
		}
		
		if (key == RIGHT_KEY || key == ENTER_KEY) 		//进入子目录
		{			
			u8 m;
			show_a_star(x);			//[]在哪里，* 就显示在哪里行
			if (x == 2)
			{
				//显示内容变化
				LAN_MODE_SHOWWING[1] = LAN_MODE_strings[1] = "*DHCP";
				LAN_MODE_SHOWWING[2] = LAN_MODE_strings[2] = "STATIC";
				
				DHCP_strings[1] = "*DHCP ON";
				DHCP_strings[2] = "DHCP 0FF";
				//通知函数
				
			}
			
			if (x == 4) 			//选择static
			{
				//显示内容变化
				LAN_MODE_SHOWWING[1] = LAN_MODE_strings[1] = "DHCP";
				LAN_MODE_SHOWWING[2] = LAN_MODE_strings[2] = "*STATIC";
				
				DHCP_strings[1] = "DHCP ON";
				DHCP_strings[2] = "*DHCP 0FF";
				
				//通知函数
				
				//这里设置IP 
				LAN_OPTION_SHOW();
			}
			
			//继续显示这一级目录
			int i;
			for (i = 0; i < 3; i++)
			{
				if (LAN_MODE_SHOWWING[i][0] == '*')
					show_strings(i*2, y-8, LAN_MODE_SHOWWING[i], strlen(LAN_MODE_SHOWWING[i]) );
				else
					show_strings(i*2, y, LAN_MODE_SHOWWING[i], strlen(LAN_MODE_SHOWWING[i]) ); 
			}
			
			show_square_breakets(x);
			
		}
		
		if (key == LEFT_KEY)//返回上一级
		{
			clear_whole_screen();
			break;
		}
 }
}

//四级目录 
void LAN_OPTION_SHOW() //ip mask gateway
{	
	int y = 8;
	int x = 0; //方括号位置

	u8 i, begin_num = 1;

	clear_whole_screen();
	for (i = 0; i < 3; i++)
	{
		show_strings(i*2, y, LAN1_OPTION_strings[i], strlen(LAN1_OPTION_strings[i]) );
	}
	show_square_breakets(x);
		
	int key = 0; 
	
	while(1)
	{
		key = recv_key_info();
		if (key == DOWN_KEY)
		{			
			x += 2;
			if ( x <= 4)
			{
				show_square_breakets(x);
			}
			else
				x = 4;
		}
		
		if (key == UP_KEY)
		{
			x -= 2;
			if (x >= 0)
			{
				show_square_breakets(x);
			}
			else
				x = 0;
		}
		
		if (key == RIGHT_KEY || key == ENTER_KEY)
		{
			//ip设置
			if (x == 0) 
			{
				LAN_INFO_SET(0, ip_string, strlen(ip_string));
			}
			if (x == 2) 
			{
				LAN_INFO_SET(1, mask_string, strlen(mask_string));
			}
			if (x == 4) 
			{
				LAN_INFO_SET(2, gateway_string, strlen(gateway_string));
			}

			clear_whole_screen();
			int i;
			for (i = 0; i < 3; i++)
			{
				show_strings(i*2, y, LAN1_OPTION_strings[i], strlen(LAN1_OPTION_strings[i]) ); 
			}
			
			show_square_breakets(x);
			
		}
			
		if (key == LEFT_KEY) //上一级目录
		{
			clear_whole_screen();
			break;
		}

	}
	
}

//五级目录 LAN INFO set
void LAN_INFO_SET(u8 offset, char *string, u8 lenth)  //注意ip显示的起始位置，
{
	u8 x = 0, y = 16, y1 = 0;
	int i = 0;

	clear_whole_screen();
	show_strings(x, y, LAN1_OPTION_strings[offset], strlen(LAN1_OPTION_strings[offset])); //显示标题
	show_strings(x+2, y1, string, strlen(string));
	int key = 0;  
	
	while(1)
	{
		key = recv_key_info();
		
		if (key == DOWN_KEY)
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
		}
		
		if (key == UP_KEY)
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
		}

		if (key == RIGHT_KEY) //总共15位
		{
			show_a_char(x+2, i*8, string[i], 0); //取消原来位置的光标
			i++;
			if (i == 3 || i == 7 || i == 11)  //跳过'.',
				i++;
			
			if (i > 14)
				i = 0;
			show_a_char(x+2, i*8, string[i], 1); //在新位置上显示光标 //1
		}
		
		if (key == LEFT_KEY)
		{
			show_a_char(x+2, i*8, string[i], 0); 
			i--;
			
			if (i == 3 || i == 7 || i == 11)  //跳过'.',
				i--;
			
			if (i < 0)
				i = 14;
			show_a_char(x+2, i*8, string[i], 1);//1
		}

		if (key == ENTER_KEY)
		{
			show_a_char(x+2, i*8, string[i], 0); //取消原来位置的光标
			//通知函数
			clear_whole_screen();
			//光标回退标题栏
			break;
		}
		
	}
}


//二级目录， 2.2EDID_SET
void EDID_SET(void)
{
	u8 count = sizeof(SAVE_EDID_strings)/(sizeof(char*));
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
	show_strings(0, y, SAVE_EDID_strings[0], strlen(SAVE_EDID_strings[0]) ); 
	show_menu_info(y, 1, EDID_SHOWWING, SAVE_EDID_strings, count>4? 3 : count-1);
	show_square_breakets(x);
	int key = 0;  

	while(1)
	{
		key = recv_key_info();
		
		if (key == DOWN_KEY || key == UP_KEY)
			param = down_up_respond(count, param, EDID_SHOWWING, SAVE_EDID_strings, key);

		if (key == RIGHT_KEY || key == ENTER_KEY) //标记*号
		{			
			show_a_star(param.x); 
			
			int i, n;
			for (i = 0; i < count; i++)
			{
				if (strstr(ACTIVE_EDID_strings[i], EDID_SHOWWING[x/2]))
				{
					//重新换一下牌，
					for (n = 0; n < count; n++)
					{
						if (n == i)
							SAVE_EDID_strings[n] = ACTIVE_EDID_strings[n];
						else
							SAVE_EDID_strings[n] = INACTIVE_EDID_strings[n];
						//通知函数
					}
						
					break;
				}
			}
			
		}
		
		if (key == LEFT_KEY) 
		{
			clear_whole_screen();
			break;
		}

	}
	
}

//二级目录，DHCP show
void DHCP_SHOW() //仅展示
{
	u8 y = 16;
	clear_whole_screen();

	int i;
	for (i = 0; i < 3; i++)
	{
		if (DHCP_strings[i][0] == '*')
			show_strings(i*2, y-8, DHCP_strings[i], strlen(DHCP_strings[i]));
		else
			show_strings(i*2, y, DHCP_strings[i], strlen(DHCP_strings[i]));
	}
	
	int key = 0;  
	
	while(1)
	{
		key = recv_key_info();
		if (key == LEFT_KEY)
		{
			clear_whole_screen();
			break;
		}
	}
}

//二级目录
void FIRMWARE_INFO_SHOW() //仅展示
{
	u8 count = sizeof(FIRMWARE_strings)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, FIRMWARE_strings[0], strlen(FIRMWARE_strings[0]) ); 
	show_menu_info(y, 1, FIRMWARE_SHOWWING, FIRMWARE_strings, count>4? 3 : count-1);
	show_square_breakets(x);

	int key = 0;  
	while(1)
	{
		key = recv_key_info();
		
		if (key == DOWN_KEY || key == UP_KEY)
		param = down_up_respond(count, param, FIRMWARE_SHOWWING, FIRMWARE_strings, key);

		if (key == LEFT_KEY)  //返回上一级目录
		{
			clear_whole_screen();
			break;
		}

	}

}

//二级目录
void DEVICE_STATUS_SHOW() //仅展示
{
	u8 count = sizeof(DEVICE_STATUS_strings)/(sizeof(char*));
	int p = 4; 
	int y = 8;
	int x = 2; //方括号位置
	int last_page = 0, first_page = 0;
	
	info_param param;
	param.x = x;
	param.y = y;
	param.p = p;
	param.last_page = last_page;

	clear_whole_screen();
	show_strings(0, y, DEVICE_STATUS_strings[0], strlen(DEVICE_STATUS_strings[0]) ); 
	show_menu_info(y, 1, DEVICE_STATUS_SHOWWING, DEVICE_STATUS_strings, count>4? 3 : count-1);
	show_square_breakets(x);

	int key = 0;  
	while(1)
	{
		key = recv_key_info();
		
		param = down_up_respond(count, param, DEVICE_STATUS_SHOWWING, DEVICE_STATUS_strings, key);
		
		if (key == LEFT_KEY)  //返回上一级目录
		{
			clear_whole_screen();
			break;
		}
	}
	
}


