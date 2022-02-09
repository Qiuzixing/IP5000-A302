
#ifndef __IPD5000_INFO_SETTING_H__
#define __IPD5000_INFO_SETTING_H__

#define u8 unsigned char

#ifdef __cplusplus
extern "C"{
#endif

int IPD5000_SHOW_INIT(char *status);

static int IPD5000_UNLOCK_MENU();
static int IPD5000_LOCK_MENU();
static int IPD5000_MAIN_MENU_SHOW(void);
static int DEV_STATUS_D();
static int LAN_STATUS_D(int interface_id);
static int HDMI_STATUS_D();
static int CHANNEL_SHOW_D();
static int TEMPERATURE_D();
static int DEV_INFO_D();
static int DEV_SETTINGS_D();
static int INPUT_SETTING_D();
static int HDCP_SETTING_D();
static int RESOL_SETTING_D();
static int CH_SELECT_D();
static int CH_NUM_SELECT_D();



#ifdef __cplusplus
}
#endif


#endif

