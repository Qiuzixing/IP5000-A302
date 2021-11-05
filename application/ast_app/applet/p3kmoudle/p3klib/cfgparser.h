#ifndef __CONFPARSER_H__
#define __CONFPARSER_H__

#include "funcexcute.h"

#ifdef __cplusplus
extern "C" {
#endif

/************ MODULE DEFINE*************************************/
#define IPE_MODULE        	"KDS-EN7"
#define IPD_MODULE         	"KDS-DEC7"
#define IPE_P_MODULE       	"KDS-SW3-EN7"
#define IPE_W_MODULE        "WP-SW2-EN7"
#define IPD_W_MODULE        "WP-DEC7"

#define IPE_PATH        	"kds-en7"
#define IPD_PATH          	"kds-dec7"
#define IPE_P_PATH        	"kds-sw3-en7"
#define IPE_W_PATH         	"wp-sw2-en7"
#define IPD_W_PATH         	"wp-dec7"

/************ CFG PATH DEFINE************************************/
#define CONF_PATH  "/data/configs/"

#define AUTOSWITCH_PATH 	"/switch"
#define CHANNEL_PATH 		"/channel"

#define AV_SIGNAL_PATH 		"/av_signal"
#define AV_SETTING_PATH 	"/av_setting"

#define DISPLAY_PATH		"/display"
#define GATEWAY_PATH		"/gateway"
#define VERSION_PATH		"/version"
#define OSD_PATH			"/osd"
#define OVERLAY_PATH		"/overlay"
#define SECURITY_PATH		"/secure"
#define KVM_PATH			"/usb"
#define AUDIO_PATH			"/audio"

#define DEVICE_PATH			"/device"

#define EDID_PATH			"/edid"

#define LOG_PATH			"/log"

#define NETWORK_PATH	    "/network"
#define TIME_PATH			"/time"
#define USER_PATH			"/user"
#define VIDEOWALL_PATH		"/vw"


/********************FILE PATH DEFINE****************************************/

#define AUTOSWITCH_DELAY_FILE 	"/switch/auto_switch_delays.json"
#define AUTOSWITCH_FILE 	"/switch/auto_switch_setting.json"

#define CHANNEL_MAP_FILE 	"/channel/channel_map.json"
#define CHANNEL_DEF_FILE 	"/channel/channel_define.json"

#define AV_SIGNAL_FILE 		"/av_signal/av_signal.json"
#define AV_SETTING_FILE 	"/av_setting/av_setting.json"

#define DISPLAY_SLEEP_FILE	"/display/display_sleep.json"
#define GATEWAY_FILE		"/gateway/gateway.json"
#define VERSION_FILE		"/version/version.json"
#define OSD_FILE			"/osd/osd.json"
#define OVERLAY_FILE		"/overlay/overlay_setting.json"
#define SECURITY_HTTPS_FILE	"/secure/https_setting.json"
#define SECURITY_8021X_FILE	"/secure/ieee802_1x_setting.json"

#define KVM_FILE			"/usb/km_usb.json"
#define AUDIO_FILE			"/audio/audio_setting.json"

#define DEVICE_FILE			"/device/device_setting.json"

#define EDID_SETTING_FILE	"/edid/edid_setting.json"
#define EDID_LIST_FILE		"/edid/edid_list.json"

#define LOG_SETTING_FILE	"/log/log_setting.json"

#define NETWORK_FILE	    "/network/network_setting.json"
#define TIME_FILE			"/time/time_setting.json"
#define USER_FILE			"/user/user_setting.json"
#define VIDEOWALL_FILE		"/vw/video_wall_setting.json"
#define VIDEOWALL_TEST		"/vw/video_wall_test_pattern.png"


#define JSON_PARAM_ON		"on"
#define JSON_PARAM_OFF		"off"
#define JSON_PARAM_IN		"in"
#define JSON_PARAM_OUT		"out"

#define JSON_AV_SIGNAL			"av_signal"
#define JSON_AV_IN_RES			"input_maximum_resolution"
#define	JSON_AV_PASSTHRU		"pass_through"
#define JSON_AV_MAX_BITRATE 	"maximum_bit_rate"
#define JSON_AV_BEST_EFFORT		"best_effort"
#define JSON_AV_FRAME_RATE  	"frame_rate_percentage"
#define JSON_AV_COLOR_DEPTH 	"color_depth"
#define JSON_AV_BYPASS			"bypass"
#define JSON_AV_RGB				"force_rgb"
#define JSON_AV_AUD_GUARD		"audio_connection_guard_time_sec"

#define JSON_DISPLAY_DELAY		"display_delays"
#define JSON_DISPLAY_SLEEP    	"sleep_delay_on_signal_loss_sec"
#define JSON_DISPLAY_SHUTDOWN	"shutdown_delay_on_signal_loss_sec"
#define JSON_DISPLAY_WAKEUP		"wake_up_delay_on_signal_detection_sec"


#define JSON_VERSION			"version"
#define JSON_VER_FILE			"fileversion"
#define JSON_VER_DEVICE			"device"
#define JSON_VER_MODEL			"model"
#define JSON_VER_HW_VERSION		"hw_version"
#define JSON_VER_FW_VERSION		"fw_version"
#define JSON_VER_UPG_TIME		"upg_time"
#define JSON_VER_STB_VERSION	"standby_version"

#define JSON_GW					"gateway"
#define JSON_GW_CEC_MODE		"cec_mode"
#define JSON_GW_CEC_DEST      	"cec_destination"
#define JSON_GW_CEC_DEST_PASS 	"passthrough"
#define JSON_GW_CEC_DEST_IN  	"hdmi_in"
#define JSON_GW_CEC_DEST_OUT  	"hdmi_out"
#define JSON_GW_CEC_DEST_LOOP 	"hdmi_loop"
#define JSON_GW_UART_MODE		"rs232_mode"
#define JSON_GW_UART_PORT		"rs232_port"
#define JSON_GW_UART_PARAM		"rs232_parameter"
#define JSON_GW_UART_RATE		"baud_rate"
#define JSON_GW_UART_BITS		"data_bits"
#define JSON_GW_UART_PARITY		"parity"
#define JSON_GW_UART_PARITY_NONE	"none"
#define JSON_GW_UART_PARITY_EVEN	"even"
#define JSON_GW_UART_PARITY_ODD		"odd"
#define JSON_GW_UART_PARITY_MARK	"mark"
#define JSON_GW_UART_PARITY_SPACE	"space"
#define JSON_GW_UART_STOP			"stop_bits_mode"
#define JSON_GW_IR_MODE				"ir_mode"
#define JSON_GW_IR_DIR				"ir_direction"

#define JSON_AUDIO				"audio_setting"
#define JSON_AUDIO_ANALOG_DIR	"analog_direction"
#define JSON_SWITCH_MODE		"switch_mode"
#define JSON_LAST_CONNECT		"last_connected"
#define JSON_PRIORITY			"priority"
#define JSON_MANUAL				"manual"
#define JSON_AUDIO_DANTE		"dante"
#define JSON_AUDIO_ANALOG		"analog"
#define JSON_AUDIO_HDMI			"hdmi"
#define JSON_AUDIO_LAN			"lan"
#define JSON_AUDIO_DEST			"destination_select"
#define JSON_SOURCE_SELECT		"source_select"
#define JSON_DANTE_NAME			"dante_name"

#define JSON_AUTOSWITCH			"auto_switch_setting"

#define JSON_HDMI_1				"hdmi_in1"
#define JSON_USBC_2    			"usb_in2"
#define JSON_HDMI_2				"hdmi_in2"
#define JSON_USBC_3    			"usb_in3"

#define JSON_STREAM				"stream"

#define JSON_CHAN_LIST			"channels_list"

#define JSON_CHANNEL			"channel_define"
#define JSON_CH_ID				"id"
#define JSON_CH_NAME			"name"

#define JSON_AV_SETTING			"av_setting"
#define JSON_AV_VOLUME			"volume"
#define JSON_AV_MUTE			"mute"
#define JSON_AV_ACTION			"action"
#define JSON_AV_PLAY			"play"
#define JSON_AV_STOP			"stop"
#define JSON_AV_HDCP			"hdcp_mode"

#define JSON_DEV_SETTING		"device_setting"
#define JSON_DEV_HOSTNAME		"host_name"
#define JSON_DEV_MAC			"mac_address"
#define JSON_DEV_SN				"serial_number"
#define JSON_DEV_STANDBY_TIME	"auto_standby_timeout"
#define JSON_DEV_FP_LOCK		"fp_lock"

#define JSON_EDID_LIST			"edid_list"
#define JSON_EDID_SETTING		"edid_setting"
#define JSON_EDID_DEFAULT		"default.bin"
#define JSON_EDID_LOCK			"lock_mode"
#define JSON_EDID_MODE			"edid_mode"
#define JSON_EDID_DEFAULTE		"default"
#define JSON_EDID_CUSTOM		"custom"
#define JSON_EDID_PASSTHRU		"passthrough"
#define JSON_EDID_SRC			"net_src"
#define JSON_EDID_ACTIVE		"active_edid"


#define JSON_LOG_SETTING		"log_setting"
#define JSON_LOG_ACTIVE			"active"
#define JSON_LOG_PERIOD			"period"
#define JSON_LOG_DAILY			"daily"
#define JSON_LOG_WEEKLY			"weekly"

#define JSON_NETWORK			"network_setting"
#define JSON_NETWORK_ETH0		"eth0"
#define JSON_NETWORK_ETH1		"eth1"
#define JSON_NETWORK_MODE		"mode"
#define JSON_NETWORK_STATIC		"static"
#define JSON_NETWORK_DHCP		"dhcp"
#define JSON_NETWORK_IP			"ip_address"
#define JSON_NETWORK_MASK		"mask_address"
#define JSON_NETWORK_GW			"gateway_address"
#define JSON_NETWORK_DNS1		"dns1"
#define JSON_NETWORK_DNS2		"dns2"
#define JSON_NETWORK_DAISY		"daisy_chain"
#define JSON_NETWORK_TCP		"tcp_port"
#define JSON_NETWORK_UDP		"udp_port"
#define JSON_NETWORK_METHOD		"method"
#define JSON_NETWORK_MULTICAST	"multicast"
#define JSON_NETWORK_UNI 		"unicast"
#define JSON_NETWORK_GROUP_IP	"group_ip"
#define JSON_NETWORK_TTL		"ttl"
#define JSON_NETWORK_PORT_SET	"port_setting"
#define JSON_NETWORK_P3K		"p3k"
#define JSON_NETWORK_RS232		"rs232"
#define JSON_NETWORK_DANTE		"dante"
#define JSON_NETWORK_PORT		"port"
#define JSON_NETWORK_VLAN		"vlan_tag"
#define JSON_NETWORK_BEACON_INFO	"beacon_info"
#define JSON_NETWORK_BEACON_EN		"beacon_en"
#define JSON_NETWORK_BEACON_IP		"beacon_ip"
#define JSON_NETWORK_BEACON_PORT	"beacon_port"
#define JSON_NETWORK_BEACON_TIME	"beacon_time"


#define JSON_TIME_SETTING		"time_setting"
#define JSON_TIME_ZONE			"time_zone"
#define JSON_TIME_DAYLIGHT		"daylight_savings_time"
#define JSON_NTP_SERVER			"ntp_server"
#define JSON_NTP_MODE			"mode"
#define JSON_NTP_IP				"ip_address"
#define JSON_NTP_SYNC_HOUR		"daily_sync_hour"
#define JSON_NTP_STATUS			"server_status"

#define JSON_USER_SETTING		"user_setting"
#define JSON_USER_NAME			"name"
#define JSON_USER_ROLE			"role"
#define JSON_USER_ADMIN			"admin"
#define JSON_USER_USER			"user"
#define JSON_USER_PASS			"password"
#define JSON_USER_LOGOUT		"logout_timeout"
#define JSON_USER_SECUR			"security_status"


#define JSON_VW_SETTING			"video_wall_setting"
#define JSON_VW_H_COUNT			"horizontal_count "
#define JSON_VW_V_COUNT			"vertical_count"
#define JSON_VW_POSITION		"relative_position"
#define JSON_VW_BEZEL			"bezel_compensation"
#define JSON_VW_BEZEL_H_VALUE	"horizontal_value"
#define JSON_VW_BEZEL_V_VALUE	"vertical_value"
#define JSON_VW_BEZEL_H_OFFSET	"horizontal_offset"
#define JSON_VW_BEZEL_V_OFFSET	"vertical_offset"
#define JSON_VW_STRETCH			"stretch_type"
#define JSON_VW_STRETCH_IN		"fit_in"
#define JSON_VW_STRETCH_OUT		"fit_out"
#define JSON_VW_ROTATION		"rotation"

#define JSON_AUTOSWITCH_DELAY		"auto_switch_delays"
#define JSON_AUTOSWITCH_LOSS    	"signal_loss_switching"
#define JSON_AUTOSWITCH_DETECT  	"signal_detection"
#define JSON_AUTOSWITCH_UNPLUG  	"cable_unplug"
#define JSON_AUTOSWITCH_OFF			"power_off_upon_signal_loss"
#define JSON_AUTOSWITCH_PLUGIN		"cable_plugin"
#define JSON_AUTOSWITCH_OVERRIDE  	"manual_override_inactive_signal"

#define JSON_OSD_CHANNEL			"channel_menu"
#define JSON_OSD_TIMEOUT			"timeout_sec"
#define JSON_OSD_POS				"position"
#define	JSON_OSD_TOP_LEFT			"top_left"
#define JSON_OSD_FONE_SIZE			"font_size"
#define JSON_OSD_MEDIUM				"medium"
#define JSON_OSD_MAX_PER_PAGE		"max_channels_per_page"
#define	JSON_OSD_MAX_CHANNEL		"max_channels"
#define JSON_OSD_DEVICE_INFO		"device_info"
#define JSON_OSD_DEVICE_INFO_EN		"enabled"
#define JSON_OSD_DEVICE_INFO_TIME   "timeout"

#define JSON_HTTPS_SETTING			"https_setting"
#define JSON_HTTPS_MODE				"mode"
#define JSON_HTTPS_CERTS			"certificate_file_name"
#define JSON_HTTPS_KEY				"private_key_password"
#define JSON_HTTPS_METHOD			"method"

#define JSON_8021X_SETTING			"ieee802_1x_setting"
#define JSON_8021X_MODE				"mode"
#define JSON_SECUR_DEFAULT_AUTH		"default_authentication"
#define JSON_SECUR_EAP_TLS			"eap_tls"
#define JSON_TLS_SETTING			"eap_tls_setting"
#define JSON_TLS_USER				"tls_username"
#define JSON_TLS_CA					"tls_ca_certificate"
#define JSON_TLS_CLIENT				"tls_client_certificate"
#define JSON_TLS_KEY				"tls_private_key"
#define JSON_TLS_PASS				"tls_private_password"
#define JSON_MSCHAP_SETTING			"eap_mschap_setting"
#define JSON_MSCHAP_USER			"mschap_username"
#define JSON_MSCHAP_PASS			"mschap_password"

#define	JSON_USB_KVM_CONFIG			"usb_kvm_config"
#define	JSON_USB_KVM_MODE			"kvm_usb_mode"
#define	JSON_USB_KVM_KM				"km"
#define	JSON_USB_KVM_USB			"usb"
#define	JSON_USB_KVM_ACTIVE			"active_per_request"
#define	JSON_USB_KVM_TIMEOUT		"kvm_timeout_sec"
#define	JSON_USB_KVM_ROW			"kvm_row"
#define	JSON_USB_KVM_COL			"kvm_col"
#define	JSON_USB_KVM_ROAMING		"km_roaming"
#define	JSON_USB_KVM_MAC			"mac"
#define	JSON_USB_KVM_H				"h"
#define	JSON_USB_KVM_V				"v"


#define MAX_EDID			8

typedef struct   _Channel_Info
{
	int  channel_id;	// 0 ~ 999
	char channel_Name[32];
}Channel_Info;

typedef struct   _Audio_Info
{
	PortDirectionType_E direction;
	AVConnectMode_E		switch_mode;
	AudioInputMode_E	input_pri[3];
	PortSignalType_E	dst_port[4];
	AudioInputMode_E	source;
	char				dante_name[32];
}Audio_Info;

typedef struct   _Video_Info
{
	int force_rgb;	//0:disable;1:enable			/av_signal/av_signal.json
	int scale_mode; //0:pass thru;1:scaling
	int res_type;	// valid for scaling mode
}Video_Info;

typedef struct   _AutoSwitch_Info
{
	AVConnectMode_E		switch_mode;
	int					input_pri[3];
	int 				source;
}AutoSwitch_Info;

typedef struct   _AVSetting_Info
{
	int 				volume;
	State_E				mute_mode;
	CodecActionType_E	action;
	State_E				hdcp_mode[3];
}AVSetting_Info;

typedef struct   _EDID_Info
{
	State_E 		lock_mode;	//0:off;1:on
	EdidModeType_E 	edid_mode;
	char	 		net_src[32];
	int 			active_id;
	char			EDID_List[MAX_EDID][64];
}EDID_Info;

typedef struct   _Version_Info
{
	char file_version[16];
	char model[64];
	char fw_version[16];
	char hw_version[16];
	char standby_version[16];
	char build_time[32];
	char upg_time[32];
}Version_Info;

typedef struct   _Device_Info
{
	char hostname[32];
	char mac_addr[32];
	char sn[32];
	int  standby_time;
	State_E fp_lock;
}Device_Info;

typedef struct   _Time_Info
{
	int 	time_zone;
	int 	daylight_saving;
	State_E	ntp_mode;
	char 	ntp_server[32];
	int 	ntp_sync_hour;
	State_E	ntp_server_status;
}Time_Info;

typedef struct   _User_Info
{
	char	user_name[32];
	char   	password[32];
	int 	user_role;//		0: user 1: admin
	int 	logout_time;
	State_E	seurity_status;
}User_Info;

typedef struct   _VideoWall_Info
{
	int horizontal_count;
	int vertical_count;
	int relative_position;
	int bezel_horizontal_value;
	int bezel_vertical_value;
	int bezel_horizontal_offset;
	int bezel_vertical_offset;
	int stretch_type;//0: fit in;1:fit out
	int rotation;	//0;90;180;270
}VideoWall_Info;


typedef struct   _Gateway_Info
{
	State_E cec_mode;
	int 	cec_output; // 0: passthru; 1: HDMI IN; 2: HDMI OUT; 3: HDMI Loop
	State_E rs232_mode;
	int 	rs232_port;
	UartMessageInfo_S	rs232_param;
	State_E	ir_mode;
	PortDirectionType_E	ir_direction;

}Gateway_Info;

typedef struct   _Network_Info
{
	NetWorkInfo_S eth_info[2];	//0:disable;1:enable
	State_E		  daisy_chain;
	int 		  tcp_port;
	int 		  udp_port;
	NetMethodType_E	  method;		// 1: unicast; 2:multi
	char 		  multicast_ip[32];
	int 		  multicast_ttl;
	int 		  p3k_port;
	int 		  p3k_vlan;
	int 		  rs232_port;
	int 		  rs232_vlan;
	int 		  dante_port;
	int 		  dante_vlan;
    State_E       beacon_en;
    int           BEACON;
    int           beacon_time;
	char 		  beacon_ip[MAX_IP_ADDR_LEN];
	int 		  beacon_port;
}Network_Info;

typedef struct   _Log_Info
{
	int    active;	//0:disable;1:enable
	int 	period; // 2:daily; 3: weekly
}Log_Info;

typedef struct _Connection_Info{
    char                        ip[24];
	int                         port;
	struct _Connection_Info	    *next;
	struct _Connection_Info	    *pre;
}Connection_Info;

typedef struct _ConnectionList_S{
	int                         size;
    Connection_Info            *head;
    Connection_Info            *tail;
}ConnectionList_S;


extern Channel_Info 		g_channel_info;
extern Audio_Info			g_audio_info;
extern Video_Info			g_video_info;
extern AutoSwitch_Info		g_autoswitch_info;
extern AVSetting_Info		g_avsetting_info;
extern EDID_Info			g_edid_info;
extern Device_Info			g_device_info;
extern Version_Info			g_version_info;
extern Time_Info			g_time_info;
extern User_Info			g_user_info;
extern VideoWall_Info		g_videowall_info;
extern Gateway_Info			g_gateway_info;
extern Network_Info			g_network_info;
extern Log_Info				g_log_info;
extern State_E				g_osd_enable;

extern int                  g_Udp_Socket;     
extern int                  g_Udp_Inside_Socket;
extern ConnectionList_S     *g_connectionlist_info;
typedef enum _SyncInfoType_E
{
	CHANNEL_INFO = 0,
	AUDIO_INFO,
	VIDEO_INFO,
	AV_SETTING,
	AUTOSWITCH_INFO,
	EDID_INFO,
	DEVICE_INFO,
	VERSION_INFO,
	TIME_INFO,
	USER_INFO,
	VIDEOWALL_INFO,
	GATEWAY_INFO,
	NETWORK_INFO,
	LOG_INFO
}SyncInfoType_E;

int Cfg_InitModule(void);
int Cfg_Init(void);
int Cfg_Init_FromASTParam(void);
int Cfg_Init_Channel(void);
int Cfg_Init_Audio(void);
int Cfg_Init_Video(void);
int Cfg_Init_AutoSwitch(void);
int Cfg_Init_AVSetting(void);
int Cfg_Init_EDID(void);
int Cfg_Init_Device(void);
int Cfg_Init_Version(void);
int Cfg_Init_Time(void);
int Cfg_Init_User(void);
int Cfg_Init_VideoWall(void);
int Cfg_Init_Gateway(void);
int Cfg_Init_Network(void);
int Cfg_Init_Log(void);
int Cfg_Init_OSD(void);

//Create Config file from default param
int Cfg_Create_DefaultFile(void);
int Cfg_Create_AutoswitchDelay(void);
int Cfg_Create_AVSignal(void);
int Cfg_Create_DisplaySleep(void);
int Cfg_Create_OsdSetting(void);
int Cfg_Create_OverlaySetting(void);
int Cfg_Create_SecuritySetting(void);
int Cfg_Create_KVMSetting(void);
int Cfg_Create_EDIDList(void);
int Cfg_Create_Channel(void);


int Cfg_Update(SyncInfoType_E type);
int Cfg_Update_Channel(void);
int Cfg_Update_Audio(void);
int Cfg_Update_Video(void);
int Cfg_Update_AutoSwitch(void);
int Cfg_Update_AVSetting(void);
int Cfg_Update_EDID(void);
int Cfg_Update_Device(void);
int Cfg_Update_Version(void);
int Cfg_Update_Time(void);
int Cfg_Update_User(void);
int Cfg_Update_VideoWall(void);
int Cfg_Update_Gateway(void);
int Cfg_Update_Network(void);
int Cfg_Update_Log(void);
int Cfg_Update_OSD(void);


int Cfg_Set_EncChannel_ID(int id);
int Cfg_Get_EncChannel_ID(int* id);
int Cfg_Set_EncChannel_Name(char* name);
int Cfg_Get_EncChannel_Name(char* name);

int Cfg_Set_Port_Dir(PortSignalType_E type,PortDirectionType_E direction);
int Cfg_Get_Port_Dir(PortSignalType_E type,PortDirectionType_E* direction);
int Cfg_Set_Autoswitch_Mode(SignalType_E type,AVConnectMode_E mode);
int Cfg_Get_Autoswitch_Mode(SignalType_E type,AVConnectMode_E* mode);
int Cfg_Set_Autoswitch_Priority(SignalType_E type,int port1, int port2,int port3);
int Cfg_Get_Autoswitch_Priority(SignalType_E type,int* port1, int* port2, int* port3);
int Cfg_Set_Autoswitch_Source(SignalType_E type,int port);
int Cfg_Get_Autoswitch_Source(SignalType_E type,int* port);
int Cfg_Set_Audio_Dest(int count, PortSignalType_E* port);
int Cfg_Get_Audio_Dest(int* port1, PortSignalType_E* port);

int Cfg_Set_Video_RGB(int mode);
int Cfg_Get_Video_RGB(int* mode);

int Cfg_Set_AV_Volume(int vol);
int Cfg_Get_AV_Volume(int* vol);
int Cfg_Set_AV_Mute(State_E mode);
int Cfg_Get_AV_Mute(State_E* mode);
int Cfg_Set_AV_Action(CodecActionType_E mode);
int Cfg_Get_AV_Action(CodecActionType_E* mode);
int Cfg_Set_AV_HDCP(int port, State_E mode);
int Cfg_Get_AV_HDCP(int port, State_E* mode);

int Cfg_Set_EDID_Lock(State_E mode);
int Cfg_Get_EDID_Lock(State_E* mode);
int Cfg_Set_EDID_NetSrc(char* src);
int Cfg_Get_EDID_NetSrc(char* src);
int Cfg_Set_EDID_Mode(EdidModeType_E mode, int idx);
int Cfg_Get_EDID_Mode(EdidModeType_E* mode, int* idx);
int Cfg_Set_EDID_Active(int idx);
int Cfg_Get_EDID_Active(int* idx);
int Cfg_Get_EDID_List(char info[][MAX_EDID_LEN],int num);



int Cfg_Set_Dev_HostName(int id, char* name);
int Cfg_Get_Dev_HostName(int id, char* name);
int Cfg_Set_Dev_StandbyTime(int time);
int Cfg_Get_Dev_StandbyTime(int* time);
int Cfg_Set_Dev_FPLock(State_E mode);
int Cfg_Get_Dev_FPLock(State_E* mode);

int Cfg_Set_UPG_Info(char* time, char* standby);
int Cfg_Get_UPG_Info(char* time, char* standby);

int Cfg_Set_Time_Loc(int utc_off, int state);
int Cfg_Get_Time_Loc(int* utc_off, int* state);
int Cfg_Set_Time_Srv(TimeSyncConf_S conf);
int Cfg_Get_Time_Srv(TimeSyncConf_S* conf);

int Cfg_Set_User_Pass(int level, char* pass);
int Cfg_Get_User_Pass(int level, char* pass);
int Cfg_Set_User_Secur(State_E mode);
int Cfg_Get_User_Secur(State_E* mode);
int Cfg_Set_User_LogoutTime(int time);
int Cfg_Get_User_LogoutTime(int* time);

int Cfg_Set_VM_Mod(int horiziontal,int vertical);
int Cfg_Get_VM_Mod(int* horiziontal,int* vertical);
int Cfg_Set_VM_Setup(int position,int rotation);
int Cfg_Get_VM_Setup(int* position,int* rotation);
int Cfg_Set_VM_Bezel(int position,WndBezelinfo_S cfg);
int Cfg_Get_VM_Bezel(int* position,WndBezelinfo_S* cfg);

int Cfg_Set_VM_Stretch(int mode);
int Cfg_Get_VM_Stretch(int* mode);

int Cfg_Set_GW_CEC_Mode(int mode);
int Cfg_Get_GW_CEC_Mode(int* mode);
int Cfg_Set_GW_Uart_Param(UartMessageInfo_S param);
int Cfg_Get_GW_Uart_Param(UartMessageInfo_S* param);
int Cfg_Set_GW_COM_Add(int port);
int Cfg_Set_GW_COM_Remove();
int Cfg_Get_GW_COM_Status(State_E* mode,int* port);
int Cfg_Set_GW_IR_Mode(State_E mode);
int Cfg_Get_GW_IR_Mode(State_E* mode);

int Cfg_Set_Log_Action(int mode, int period);
int Cfg_Get_Log_Action(int* mode, int* period);


int Cfg_Set_OSD_Diaplay(State_E mode);
int Cfg_Get_OSD_Diaplay(State_E* mode);

int Cfg_Set_Net_Config(int netId,NetWorkInfo_S*netInfo);
int Cfg_Get_Net_Config(int netId,NetWorkInfo_S*netInfo);
int Cfg_Set_Net_DHCP(int netId,int dhcp);
int Cfg_Get_Net_DHCP(int netId,int* dhcp);
int Cfg_Set_Net_DaisyChain(State_E mode);
int Cfg_Get_Net_DaisyChain(State_E* mode);
int Cfg_Set_Net_Port(NetPortType_E type, int port); //type:udp;tcp
int Cfg_Get_Net_Port(NetPortType_E type, int* port);
int Cfg_Set_Net_Method(NetMethodType_E type); //1:uni;2 multi
int Cfg_Get_Net_Method(NetMethodType_E* type);
int Cfg_Set_Net_Multicast(char * ip,int ttl); //type:udp;tcp
int Cfg_Get_Net_Multicast(char * ip,int* ttl);
int Cfg_Set_Net_GW_Port(NetGWType_E type, int port); //type:p3k,rs232,dante
int Cfg_Get_Net_GW_Port(NetGWType_E type, int* port);
int Cfg_Set_Net_GW_Vlan(NetGWType_E type, int vlan); //type:p3k,rs232,dante
int Cfg_Get_Net_GW_Vlan(NetGWType_E type, int* vlan);

int Cfg_Set_Enc_AVSignal_Info();
int Cfg_Set_Dec_Usb_KVM();


#ifdef __cplusplus
}
#endif

#endif

