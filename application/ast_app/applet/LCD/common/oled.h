
#ifndef __OLED_H__
#define __OLED_H__
#include "driver_i2c.h"



#define OLED_CMD  0	
#define OLED_DATA 1	

void OLED_Init(void);

void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);

void OLED_WR_Byte(u8 dat,u8 mode);
void clear_a_line(u8 x);
void clear_three_line();
//void new_clear_three_line();
void clear_whole_screen(void);


void show_a_char(u8 x, u8 y, u8 chr, u8 cursor);
void show_strings(u8 x, u8 y, const char *str, u8 lenth);

void show_square_breakets(u8 x);
void show_a_star(u8 x);

void clear_list();

void clear_whole_screen();


#endif


