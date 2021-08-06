#ifndef _SET_HDCP_STATUS_H_
#define _SET_HDCP_STATUS_H_

#define TX_HDCP_STATUS_DIR "/sys/devices/platform/it6802/ast_hdcp_status"
#define RX_HDCP_STATUS_DIR "/sys/devices/platform/gsv200x/ast_hdcp_status"

void set_hdcp_status(const char *file_name,unsigned char value);

#endif
