
#ifndef __IPE5000_INFO_SETTING_H__
#define __IPE5000_INFO_SETTING_H__

#define u8 unsigned char 

#ifdef __cplusplus
extern "C"{
#endif

int IPE5000_SHOW_INIT(char *status);

static int IPE5000_UNLOCK_MENU();
static int IPE5000_LOCK_MENU();
static int IPE5000_MAIN_MENU_SHOW(void);
static int DEV_STATUS_E();
static int LAN_STATUS_E(int interface_id);
static int HDMI_STATUS_E();
static int CH_DEFINE_E();
static int TEMPERATURE_E();
static int DEV_INFO_E();
static int DEV_SETTINGS_E();
static int EDID_SETTING_E();
static int HDCP_SETTING_E();
static int CH_SELECT_E();
static int CH_NUM_SELECT_E();


int IPE5000_MAIN_MENU_SHOW(void);

#ifdef __cplusplus
}
#endif

#endif

