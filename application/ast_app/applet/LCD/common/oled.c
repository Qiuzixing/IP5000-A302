
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oled.h"
#include "oledfont.h"

u8 clounm_buf[8][128] = {0};

//	dat: 要发送的字节
//	mode；0 写命令. 1 写数据	
void OLED_WR_Byte(u8 dat, u8 mode)
{
	if(mode)
    {
		i2c_write_one_byte(0x40, dat);
    }
    else
    {
		 i2c_write_one_byte(0x00, dat);
    }
}

//颜色反转
void OLED_ColorTurn(u8 i)
{
	if(i==0)
	{
		OLED_WR_Byte(0xA6,OLED_CMD);
	}
	if(i==1)
	{
		OLED_WR_Byte(0xA7,OLED_CMD);
	}
}

void OLED_DisplayTurn(u8 i)
{
	if(i==0)
	{
		OLED_WR_Byte(0xC8,OLED_CMD);
		OLED_WR_Byte(0xA1,OLED_CMD);
	}
	if(i==1)
	{
		OLED_WR_Byte(0xC0,OLED_CMD);
		OLED_WR_Byte(0xA0,OLED_CMD);
	}
}

void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);
	OLED_WR_Byte(0x14,OLED_CMD);
	OLED_WR_Byte(0xAF,OLED_CMD);
}

void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD); //开启屏幕显示
	OLED_WR_Byte(0x10,OLED_CMD); //
	OLED_WR_Byte(0xAE,OLED_CMD); //
}

// x；横坐标，只能显示4行，取值有 0,2,4,6.共四个
// y；列坐标，取8的整数倍， 0,8,16......,119
void clear_one_word(u8 x, u8 y)
{
	u8 m = 0x0f;
	u8 p = 0x07;
	m &= y;
	p &= (y >> 4);
	p |= 0x10;

	u8 n, i;
	for (i = 0; i < 2; i++)
	{
		OLED_WR_Byte(0xb0+x+i,OLED_CMD);
		OLED_WR_Byte(m,OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(p,OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置.
		
		for(n=0; n<8; n++)
		{	
			OLED_WR_Byte(0x00, OLED_DATA);	
		}
	}

}

void clear_list()
{
	int i, n;
	for (n=0; n < 16; n++)
	{
		for (i=2; i<8 ;i+=2)
		{
			clear_one_word(i, n*8);
		}
	}
}

void clear_a_line(u8 x)
{
	u8 i;
	u8 data[128] = {0};
	for (i=x; i<x+2; i++)
	{
		OLED_WR_Byte(0xb0+i, OLED_CMD);
		OLED_WR_Byte(0x00, OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(0x10, OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置.
		i2c_write_multi_byte(data, 128);
	}
}

void clear_three_line()
{
	int n ,i;
	for (i = 2; i < 8; i+=2)
	{
		clear_a_line(i);		
	}
}

void light_whole_screen()
{
	int i = 0;
	char data[128];
	memset(data, 255, 128);
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0+i, OLED_CMD);
		OLED_WR_Byte(0x00, OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(0x10, OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置
		
		i2c_write_multi_byte(data, 128);
	}
}

void clear_whole_screen()
{
	int i;

	u8 data[128] = {0};

	for (i=0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0+i, OLED_CMD);
		OLED_WR_Byte(0x00, OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(0x10, OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置.
		
		i2c_write_multi_byte(data, 128);
	}
}

/*
	chr: chr在char_16_8[][8]中位置,char_16_8[][8]是字模数组
*/
int get_offset(char chr)
{
	u8 offset = -1;
	do {
		if (chr >= 'A' && chr <= 'Z')
		{
			offset = (chr - 'A' + 10) * 2; //'A'是数组第22个元素
			break;
		}
		
		if (chr >= '0' && chr <= '9')  //'0'是数组第0个元素
		{
			offset = (chr - '0') * 2;
			break;
		}

		if (chr > 'a' && chr <= 'z')
		{
			offset = (chr - 'a' + 36) * 2;
			break;
		}
		
		if (chr == '*')
		{
			offset = 62*2;
			break;
		}

		
		if (chr == '.')
		{
			offset = 63*2;
			break;
		}
		
		if (chr == '[')
		{
			offset = 64*2;
			break;
		}
		if (chr == ']')
		{
			offset = 65*2;
			break;
		}

		if (chr == '_')
		{
			offset = 66*2;
			break;
		}
		
		if (chr == ' ')
		{
			offset = 67*2;
			break;
		}
		if (chr == ',')
		{
			offset = 68*2;
			break;
		}
		if (chr == ':')
		{
			offset = 69*2;
			break;
		}
	} while(0);	
	return offset;


}


/*
	x；横坐标，只能显示4行，取值有 0,2,4,6.共四个
	y；列坐标，8的整数倍， 0,8,16......,119
	chr: 要更新的字符，
	cursor: 底部是否要光标显示，1是， 0否
*/
void show_a_char(u8 x, u8 y, u8 chr, u8 cursor) 
{
	u8 m = 0x0f;
	u8 p = 0x07;
	m &= y;
	p &= (y >> 4);
	p |= 0x10;  //取起始列地址

	u8 n, i;
	u8 word[8];
	int offset;
	offset = get_offset(chr);
		
	for (i = 0; i < 2; i++)
	{
		OLED_WR_Byte(0xb0+x+i,OLED_CMD);
		OLED_WR_Byte(m,OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(p,OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置.

		memset(word, 0, 8);
		for(n=0; n<8; n++)
		{	
			if (cursor ==1)
				word[n] |= (char_16_8[66*2+i][n] | char_16_8[offset+i][n]); 
			else
				word[n] |= (char_16_8[offset+i][n]);				
		}
		
		//每次写8字节
		i2c_write_multi_byte(word, 8);
	}	

}

void show_strings(u8 x, u8 y, const char *str, u8 lenth)
{
	u8 m = 0x0f;
	u8 p = 0x07;
	m &= y;
	p &= (y >> 4);
	p |= 0x10;  //取起始列地址

	//每次写128个字节
	int offset;
	int t;
	u8 n, i;
	u8 word[128] = {0};
	u8 count = 0;
		
	for (i = 0; i < 2; i++)
	{
		count = 0;
		memset(word, 0, 128);
		OLED_WR_Byte(0xb0+x+i,OLED_CMD);
		OLED_WR_Byte(m,OLED_CMD);   //设置起始列的低四位字节
		OLED_WR_Byte(p,OLED_CMD);   //设置起始列的高四位字节 两个拼起来组成列的起始位置.
		for (t = 0; t < lenth; t++)
		{
			offset = get_offset(str[t]);
			for(n=0; n<8; n++)
			{	
				word[count++] |= char_16_8[offset+i][n];
			}
		}
		//一次写多个字节,在这里写128
		i2c_write_multi_byte(word, count);
	}
}

void show_square_breakets(u8 x)
{	
	u8 i, y = 0;
	for (i = 0; i < 8; i = i+2)
	{
		if (i == x)
		{
			show_a_char(i, y, '[', 0);
			show_a_char(i, y+14*8, ']', 0);
		}
		else
		{
			show_a_char(i, y, ' ', 0);
			show_a_char(i, y+14*8, ' ', 0);
		}
	}
}

void show_a_star(u8 x)
{
	u8 i, y = 8;
	for (i = 2; i < 8; i = i+2)
	{
		if (i == x)
			show_a_char(i, y, '*', 0);		
		else
			show_a_char(i, y, ' ', 0);
	}
}

void OLED_Init(void)
{
    OLED_WR_Byte(0xAE,OLED_CMD); /*display off*/ 
    OLED_WR_Byte(0x00,OLED_CMD); /*set lower column address*/ 
    OLED_WR_Byte(0x10,OLED_CMD); /*set higher column address*/
	OLED_WR_Byte(0xB0,OLED_CMD); /*set page address*/ 
	OLED_WR_Byte(0x40,OLED_CMD); /*set display start lines*/ 
	OLED_WR_Byte(0x81,OLED_CMD); /*contract control*/ 
	OLED_WR_Byte(0x88,OLED_CMD); /*4d*/ 
	OLED_WR_Byte(0x82,OLED_CMD); /* iref resistor set and adjust ISEG*/ 
	OLED_WR_Byte(0x00,OLED_CMD); 
	OLED_WR_Byte(0xA1,OLED_CMD); /*set segment remap 0xA0*/ 
	OLED_WR_Byte(0xA2,OLED_CMD); /*set seg pads hardware configuration*/ 
	OLED_WR_Byte(0xA4,OLED_CMD); /*Disable Entire Display On (0xA4/0xA5)*/ 
	OLED_WR_Byte(0xA6,OLED_CMD); /*normal / reverse*/ 
	OLED_WR_Byte(0xA8,OLED_CMD); /*multiplex ratio*/ 
	OLED_WR_Byte(0x3F,OLED_CMD); /*duty = 1/64*/ 
	OLED_WR_Byte(0xC8,OLED_CMD); /*Com scan direction 0XC0*/ 
	OLED_WR_Byte(0xD3,OLED_CMD); /*set display offset*/ 
	OLED_WR_Byte(0x00,OLED_CMD); /* */ 
	OLED_WR_Byte(0xD5,OLED_CMD); /*set osc division*/ 
	OLED_WR_Byte(0xa0,OLED_CMD); 
	OLED_WR_Byte(0xD9,OLED_CMD); /*set pre-charge period*/ 
	OLED_WR_Byte(0x22,OLED_CMD); 
	OLED_WR_Byte(0xdb,OLED_CMD); /*set vcomh*/ 
	OLED_WR_Byte(0x40,OLED_CMD); 
	OLED_WR_Byte(0x31,OLED_CMD); /* Set pump 7.4v */ 
	OLED_WR_Byte(0xad,OLED_CMD); /*set charge pump enable*/ 
	OLED_WR_Byte(0x8b,OLED_CMD); /*Set DC-DC enable (0x8a=disable; 0x8b=enable) */ 
	
	//OLED_WR_Byte(0x23,OLED_CMD); // 闪烁显示
	//OLED_WR_Byte(0x00,OLED_CMD);
	
	clear_whole_screen();
	OLED_WR_Byte(0xAF,OLED_CMD);
}



