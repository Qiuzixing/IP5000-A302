
#ifndef __IPE5000P_INFO_SETTING_H__
#define __IPE5000P_INFO_SETTING_H__

int IPE5000P_MAIN_MENU_SHOW(void);

static void IP_SETTING_MENU_SHOW_P(void);
static void INPUT_SELECT_SHOW_P();
static void EDID_SET_P(void);
static void HDCP_SHOW_P();
static void FIRMWARE_INFO_SHOW_P();
static void DEVICE_STATUS_SHOW_P();
static void LAN_MODE_MENU_SHOW_P(int lan_id);
static void DHCP_IP_SHOW_P(int lan_id);
static void LAN_OPTION_SHOW_P(int lan_id);
static void LAN_INFO_SET_P(int lan_id, u8 offset, char *string, u8 lenth);


#endif

