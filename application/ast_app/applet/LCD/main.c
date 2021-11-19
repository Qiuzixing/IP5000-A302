
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "oled.h"

#include "IPD5000_info_setting.h"
#include "IPE5000_info_setting.h"
#include "IPE5000P_info_setting.h"


#define I2C_BUS_NUM     5
#define I2C_FREQ    400000
#define I2C_ADDR    0x78

enum
{
    LCD_ON = 0,
    LCD_OFF
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("plasea input correct parameter\n");
        return -1;
    }

    i2c_init(I2C_BUS_NUM, I2C_FREQ, I2C_ADDR, LCD_ON);
    OLED_Init();
    OLED_DisplayTurn(1); //0正常显示，屏幕翻转显示 

    do 
    {
        if (strcmp(argv[1], "IPD5000") == 0)
        {
            if (IPD5000_MAIN_MENU_SHOW() == -1)
            {
                printf("IPD5000 SHOW fail\n");
            }
            break;
        }
        
        if (strcmp(argv[1], "IPE5000") == 0)
        {
            if (IPE5000_MAIN_MENU_SHOW() == -1)
            {
                printf("IPE5000 SHOW fail\n");
            }
            break;
        }
            
        if (strcmp(argv[1], "IPE5000P") == 0)
        {
            if (IPE5000P_MAIN_MENU_SHOW() == -1)
            {
                printf("IPE5000P SHOW fail\n");
            }
            break;
        }
    }while (0);
    
    return 0;
}



