
#ifndef __IPE5000P_INFO_SETTING_H__
#define __IPE5000P_INFO_SETTING_H__

int IPE5000P_MAIN_MENU_SHOW(void);

static void IP_SETTING_MENU_SHOW(void);
static void INPUT_SELECT_SHOW();
static void EDID_SET(void);
static void DHCP_SHOW();
static void FIRMWARE_INFO_SHOW();
static void DEVICE_STATUS_SHOW();

static void LAN_MODE_MENU_SHOW(void);
static void LAN_OPTION_SHOW();
static void LAN_INFO_SET(u8 offset, char *string, u8 lenth);


#endif

