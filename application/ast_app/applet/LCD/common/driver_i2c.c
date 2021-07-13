
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "driver_i2c.h"

#define BUS_INIT_FILE	  "/sys/devices/platform/i2c/bus_init"
#define IO_SELECT_FILE	  "/sys/devices/platform/i2c/io_select"
#define IO_VALUE_FILE	  "/sys/devices/platform/i2c/io_value"
#define LCD_POWER "/sys/class/leds/lcd_power/brightness"

enum
{
    LCD_ON = 0,
    LCD_OFF
};
	
#define BUF_SIZE 100

static int value_fd = 0;
static int FD;

static void lcd_power(unsigned char power_value)
{
    int fd = open(LCD_POWER,O_RDWR);
    int num = 0;
    char buf[100] = {0};
    if(fd < 0 )
    {
        printf("open %s error\n",LCD_POWER);
        return;
    }
    if(power_value == LCD_ON)
    {
        sprintf(buf, "%d\n", LCD_ON);
    }
    else
    {
        sprintf(buf, "%d\n", LCD_OFF);
    }
    num = write(fd,buf,strlen(buf));
    if(num <= 0)
    {
        printf("write %s error\n",LCD_POWER);
        close(fd);
        return;
    }

    close(fd);
}


static int i2c_bus_init(int bus_num, int frequency)
{
	int num = 0;
	int fd = 0;
	fd = open(BUS_INIT_FILE, O_RDWR);
	if (fd <= 0)
	{
		perror("fopen bus_init");
		return -1;
	}
	
	char buf[BUF_SIZE] = {0};
	sprintf(buf, "%d %u\n", bus_num, frequency);

	num = write(fd, buf, strlen(buf));
	if (num == 0)
	{
		perror("fwrite bus_init");
		close(fd);
		return -1;
	}
	
	close(fd);
	
	return 0;

}


static int i2c_dev_init(int bus_num, u8 dev_addr) 
{
	int count = 0;
	int fd = 0;
    char buf[BUF_SIZE] = {0};
	fd = open(IO_SELECT_FILE, O_RDWR);
	if (fd <= 0)
	{
		perror("fopen io_select");
		return -1;
	}
    sprintf(buf, "%d 0x%2x\n", bus_num, dev_addr);
	count = write(fd, buf, strlen(buf));
	if (count == 0)
	{
		perror("fwrite io_select");
		close(fd);
		return -1;
	}
	
	close(fd);
	
	return 0;
}

int i2c_write_one_byte(u8 reg, u8 data)
{
	int num;
	unsigned char buf[BUF_SIZE] = {0};
    int fd = 0;
	fd = open(IO_VALUE_FILE, O_RDWR);
	if (fd <= 0)
	{
		perror("fopen VALUE_FILE");
		return -1;
	}

	sprintf(buf, "%x %x\n", reg, data); //写数据偏移量为reg，

	num = write(fd, buf, strlen(buf));
	if (num == 0)
	{
		perror("fwrite VALUE_FILE");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}

int i2c_file_open()
{
	FD = open(IO_VALUE_FILE, O_RDWR);
	if (FD == -1)
	{
		perror("open VALUE_FILE");
		return -1;
	}
	return 0;
}

int i2c_file_close()
{
	close(FD);
	return 0;
}

/*
	I2C写多个字节
	data: 要写的字节数组
	size: 要写的字节数
*/

int i2c_write_multi_byte(u8 *data, u8 size)
{
	int i=0, j=0;
	int num;
	//一次写128个是否可以
	unsigned char buf[6] = {0};
	if (i2c_file_open() == -1)
	{
		return -1;
	}
		
	for (i=0; i<size; i++)
	{
		sprintf(buf, "%2x %2x\n", 0x40, data[i]);
		num = write(FD, buf, strlen(buf));
		if (num == -1)
		{
			perror("fwrite VALUE_FILE");
			return -1;
		}
	}
		
	i2c_file_close();
	
	return 0;
}


void i2c_init(unsigned int  i2c_bus_num, unsigned int i2c_freq, u8 i2c_addr, int enable)
{
	lcd_power(enable);
    i2c_bus_init(i2c_bus_num, i2c_freq);
    i2c_dev_init(i2c_bus_num, i2c_addr);
}
