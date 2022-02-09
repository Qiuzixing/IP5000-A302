#ifndef _RTL8364_I2C_DRIVER_H_
#define _RTL8364_I2C_DRIVER_H_
#include <common.h>
typedef enum
{
    STATUS_OK = 0U,
    STATUS_ERROR,
    STATUS_BUSY,
    STATUS_TIMEOUT
} return_t;

struct rtl8364_msg
{
	unsigned int addr;
	unsigned int value;
};

return_t enet_phy_init(void);


#endif
