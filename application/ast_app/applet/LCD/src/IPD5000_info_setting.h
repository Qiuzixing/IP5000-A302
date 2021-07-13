
#ifndef __IPD5000_INFO_SETTING_H__
#define __IPD5000_INFO_SETTING_H__

#define u8 unsigned char

int IPD5000_MAIN_MENU_SHOW(void);

static void IP_SETTING_MENU_SHOW_D(void);
static void VIDEO_IN_SELECT_SHOW_D(void);
static void VEDIO_OUT_RES_SHOW_D();
static void DHCP_SHOW_D();
static void FIRMWARE_INFO_SHOW_D();
static void DEVICE_STATUS_SHOW_D();
static void LAN_MODE_MENU_SHOW_D(void);
static void LAN_OPTION_SHOW_D();
static void LAN_INFO_SET_D(u8 offset, char *string, u8 lenth);


#endif

