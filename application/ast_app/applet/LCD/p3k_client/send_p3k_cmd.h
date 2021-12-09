
#ifndef __SEND_P3K_CMD__
#define __SEND_P3K_CMD__

#ifdef __cplusplus
extern "C"{
#endif

#define SIZE0 255
#define SIZE1 1024
//#define SIZE2 255
#define LITTLE_SIZE 20

typedef struct 
{
	int ch_id;
	char ch_name[20];
}T_CH_MAP;


int init_p3k_client(char *ip, int port);
int get_specified_string_from_file(const char *file, char *channel_list[1000]);
int Decode_Get_Chenel_List(T_CH_MAP list[1000]);


int GET_IP(int NET_ID, char *IP, char *MASK, char *GATEWAY);
int SET_IP(int NET_ID, char *IP, char *MASK, char *GATEWAY);
int SET_DHCP_STATUS(int NET_ID);
int GET_EDID_LIST(char EDID_buf[][LITTLE_SIZE]);
int SET_EDID(char index);
int GET_EDID(char *edid_type);
int GET_HDCP_STATUS(char *hdcp_status);
int GET_HDCP_MODE(char *mode);
int SET_HDCP_MODE(char *mode);
int GET_CHANNEL_DEFINE(char *id);
int SET_CHANNEL_DEFINE(char *id);
int GET_CHANNEL_ID();
int SET_CHANNEL_ID(char *id);
int GET_TEMPERATURE(char *temp);
int GET_FW_VERSION(char *FW_VER);
int GET_BL_VERSION(char *BL_VER);
int GET_HW_VERSION(char *HW_VER);
int GET_ENCODE_VIDEO_INPUT(char *type);
int SET_ENCODE_VIDEO_INPUT(const char *type);
int GET_DECODE_VIDEO_INPUT(char *type);
int SET_DECODE_VIDEO_INPUT_TYPE(const char *type);
int GET_SCALE_MODE(char *mode);
int SET_SCALE_MODE(const char *mode);
int GET_ACTUAL_RESOLUTION(char *resol_buf);
int CHANNEL_LIST(char *info_buf[1000]);

 



#ifdef __cplusplus
}
#endif

#endif



