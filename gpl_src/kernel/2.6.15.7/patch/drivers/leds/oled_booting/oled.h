#ifndef __OLED_H
#define __OLED_H 

#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
#define u8 unsigned char 
#define u16 unsigned int
#define u32 unsigned long
#define OLED_I2C_BUS_NUM		5
#define OLED_I2C_BUS_SPEED	    400000
#define OLED_I2C_ADDR	        0x78

enum
{
    NO_FOUND = 0,
    EXIST   ,
};

void oled_set_booting(void);
#endif
