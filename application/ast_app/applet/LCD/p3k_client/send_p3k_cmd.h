
#ifndef __SEND_P3K_CMD__
#define __SEND_P3K_CMD__

#ifdef __cplusplus
extern "C"{
#endif

#define SIZE0 255

int init_p3k_client(char *ip, int port);


int get_ip(int NET_ID, char *IP, char *MASK, char *GATEWAY);	
int set_ip(int NET_ID, char *IP, char *MASK, char *GATEWAY);
int get_DHCP_status(int NET_ID, int *status);	
int set_DHCP_status(int NET_ID);
int get_EDID_list(char EDID_buf[][SIZE0]);
int set_EDID(char edid_type);
char get_EDID();
int get_HDCP_status(int *HDCP_STATUS);
int get_FIRMWARE_INFO(char info_buf[][SIZE0]);
int get_DEVICE_STATUS(int *Dvc_Status);
int get_INPUT_INFO(char *recv_buf);
int set_INPUT_INFO(int num);
int VIDEO_LIST(char info_buf[][SIZE0]);
int select_voide_channel(int num);
int get_current_voide_channel(int *channel_num);
int get_VIDEO_OUT(int *video_data);



#ifdef __cplusplus
}
#endif

#endif



