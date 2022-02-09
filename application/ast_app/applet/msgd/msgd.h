/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef __MSGD_H__
#define __MSGD_H__

#define MSGD_IP "127.0.0.1"
#define MSGD_FE_PORT 6850
#define MSGD_FE_PORT_STR "6850"

#define STR_BUF_SIZE 64
#define MAX_STR_LEN (STR_BUF_SIZE - 1)

#define OSD_STR_BUF_SIZE 128

#define INFOTYPE_RT 68500001
#define INFOTYPE_ST 68500002
#define INFOTYPE_OSD 68500003
#define INFOTYPE_GUI_ACTION 68500004

#define CMD_GET_ST  68510001

/* Note that ALL following data structure is not "packed". */
struct info_hdr {
    unsigned int type;
    unsigned int data_len;
    char data[];
};

struct static_info {
    char IP[STR_BUF_SIZE];
    char FW[STR_BUF_SIZE];
    char RemoteIP[STR_BUF_SIZE];
    char ID[STR_BUF_SIZE];
};

struct runtime_info {
    char str[STR_BUF_SIZE];
};

struct osd_info {
    unsigned int font_color;
    int font_size;
    char str[OSD_STR_BUF_SIZE];
};

// action_type
#define ACTION_GUI_CHANGES 1
#define ACTION_NODE_REFRESH 2
#define ACTION_GUI_SHOW_PICTURE 3
// action switches
#define GUI_SHOW_DIALOG 1
#define GUI_HIDE_DIALOG 0
#define GUI_REFRESH_NODE 1
#define GUI_NOREFRESH_NODE 0
#define GUI_SHOW_TEXT 1
#define GUI_HIDE_TEXT 0

struct gui_show_info {
	unsigned int show_text;		//1: show, 0: hide
    char picture_name[STR_BUF_SIZE];
};

struct gui_action_info {
  unsigned int action_type;
  union {
    char buf[STR_BUF_SIZE];
    unsigned int show_dialog; //1: show, 0: hide
    unsigned int refresh_node; //1: refresh, 0: no refresh
	struct gui_show_info show_info;
  } ub;
};
#endif // __MSGD_H__
