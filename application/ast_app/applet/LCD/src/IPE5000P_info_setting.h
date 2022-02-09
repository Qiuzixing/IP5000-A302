
#ifndef __IPE5000P_INFO_SETTING_H__
#define __IPE5000P_INFO_SETTING_H__

#define u8 unsigned char 

#ifdef __cplusplus
extern "C"{
#endif

int IPE5000P_SHOW_INIT(char *status);

static int IPE5000P_UNLOCK_MENU();
static int IPE5000P_LOCK_MENU();
static int IPE5000P_MAIN_MENU_SHOW(void);
static int DEV_STATUS_P();
static int LAN_STATUS_P(int interface_id);
static int HDMI_STATUS_P();
static int CH_DEFINE_P();
static int TEMPERATURE_P();
static int DEV_INFO_P();
static int DEV_SETTINGS_P();
static int INPUT_SETTING_P();
static int EDID_SETTING_P();
static int HDCP_SETTING_P();
static int CH_SELECT_P();
static int CH_NUM_SELECT_P();


int IPE5000P_MAIN_MENU_SHOW(void);

#ifdef __cplusplus
}
#endif


#endif

