
#ifndef __IPE5000_INFO_SETTING_H__
#define __IPE5000_INFO_SETTING_H__

#define u8 unsigned char 

#ifdef __cplusplus
extern "C"{
#endif

static int DEV_STATUS_E();
static void LAN_STATUS_E(int interface_id);
static int HDMI_STATUS_E();
static int CH_DEFINE_E();
static int TEMPERATURE_E();
static int DEV_INFO_E();
static int DEV_SETTINGS_E();
static int EDID_SETTING_E();
static int HDCP_SETTING_E();
static int CH_SELECT_E();

int IPE5000_MAIN_MENU_SHOW(void);

#ifdef __cplusplus
}
#endif

#endif

