
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "driver_i2c.h"

#define BUS_INIT_FILE	  "/sys/devices/platform/i2c/bus_init"
#define IO_SELECT_FILE	  "/sys/devices/platform/i2c/io_select"
#define IO_VALUE_FILE	  "/sys/devices/platform/i2c/io_value"
#define IO_BUF_FILE	 	 "/sys/devices/platform/i2c/io_buf"

#define LCD_POWER 		  "/sys/class/leds/lcd_power/brightness"

enum
{
    LCD_ON = 0,
    LCD_OFF
};
	
#define BUF_SIZE 100

static int lcd_power(unsigned char power_value)
{
    int fd = open(LCD_POWER,O_RDWR);
    int num = 0;
    char buf[100] = {0};
    if(fd == -1)
    {
        printf("open %s error\n",LCD_POWER);
        exit(0);
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
    if(num == -1)
    {
        printf("write %s error\n",LCD_POWER);
        close(fd);
        exit(0);
    }

    close(fd);
	return 0;
}


static int i2c_bus_init(int bus_num, int frequency)
{
	int num = 0;
	int fd = 0;
	fd = open(BUS_INIT_FILE, O_RDWR);
	if (fd == -1)
	{
		perror("fopen bus_init");
		exit(0);
	}
	
	char buf[BUF_SIZE] = {0};
	sprintf(buf, "%d %u\n", bus_num, frequency);

	num = write(fd, buf, strlen(buf));
	if (num == -1)
	{
		perror("fwrite bus_init");
		close(fd);
		exit(0);
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
	if (fd == -1)
	{
		perror("fopen io_select");
		exit(0);
	}
    sprintf(buf, "%d 0x%2x\n", bus_num, dev_addr);
	count = write(fd, buf, strlen(buf));
	if (count == -1)
	{
		perror("fwrite io_select");
		close(fd);
		exit(0);
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
	if (fd == -1)
	{
		perror("fopen VALUE_FILE");
		exit(0);
	}

	sprintf(buf, "%x %x\n", reg, data); //写数据偏移量为reg，

	num = write(fd, buf, strlen(buf));
	if (num == -1)
	{
		perror("fwrite VALUE_FILE");
		close(fd);
		exit(0);
	}

	close(fd);
	return 0;
}

int i2c_write_multi_byte(u8 *data, u8 size)
{
#if 0
	int i = 0, fd;
	unsigned char buf[6] = {0};
	fd = open(IO_VALUE_FILE, O_RDWR);
	if (fd == -1)
	{
		perror("open VALUE_FILE");
		exit(0);
	}
		
	for (i = 0; i < size; i++)
	{
		sprintf(buf, "%2x %2x\n", 0x40, data[i]);
		if (write(fd, buf, strlen(buf)) == -1)
		{
			perror("fwrite VALUE_FILE");
			close(fd);
			exit(0);
		}
	}
	close(fd);
	
 #else
	int fd;
	int num;
	unsigned char buf[256] = {0};
	
	fd = open(IO_BUF_FILE, O_RDWR);
	if (fd == -1)
	{
		perror("open IO_BUF_FILE");
		return -1;
	}
	buf[0] = 0x40;
    memcpy(buf + 1, data, size);
	num = write(fd, buf, size + 1);
	if (num == 0)
	{
		perror("fwrite BUF_FILE");
		close(fd);
		return -1;
	}
	close(fd);
#endif

	return 0;
}


int i2c_init(unsigned int  i2c_bus_num, unsigned int i2c_freq, u8 i2c_addr, int enable)
{
	lcd_power(enable);
	i2c_bus_init(i2c_bus_num, i2c_freq);
    i2c_dev_init(i2c_bus_num, i2c_addr);
	
}


