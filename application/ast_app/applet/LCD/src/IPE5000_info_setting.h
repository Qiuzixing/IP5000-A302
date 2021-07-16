
#ifndef __IPE5000_INFO_SETTING_H__
#define __IPE5000_INFO_SETTING_H__

#define u8 unsigned char 

int IPE5000_MAIN_MENU_SHOW(void);

static void IP_SETTING_MENU_SHOW_E(void);
static void EDID_SET_E(void);
static void DHCP_SHOW_E();
static void FIRMWARE_INFO_SHOW_E();
static void DEVICE_STATUS_SHOW_E();
static void LAN_MODE_MENU_SHOW_E(void);
static void LAN_OPTION_SHOW_E();
static void LAN_INFO_SET_E(u8 offset, char *string, u8 lenth);


#endif

