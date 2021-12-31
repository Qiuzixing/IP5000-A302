#ifndef _RTL8364_I2C_DRIVER_H_
#define _RTL8364_I2C_DRIVER_H_
#include "rtk_types.h"
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

typedef struct eth_vlan_setting
{
    unsigned int p3k_port_tag;
    unsigned int rs232_gateway_tag;
    unsigned int dante_port_tag;
    unsigned int pvid;
}eth_vlan_setting;

typedef struct vlan_setting
{
    eth_vlan_setting eth0;
    eth_vlan_setting eth1;
}vlan_setting_param;

return_t enet_phy_init(void);
void a30_rtl8367_vlan_setting(vlan_setting_param *vlan_settings);
void a30_rtl8364_vlan_setting(vlan_setting_param *vlan_settings);
#endif
