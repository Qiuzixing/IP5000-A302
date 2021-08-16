
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "tcp_client.h"

#define SIZE1 1024
#define SIZE2 255


//#define FIRMWARE_INFO_FILE  	"/etc/board_info.json"
//#define VIDEO_INFO_FILE			"/data/configs/kds-6x/channel/channel_map.json"
#define FIRMWARE_INFO_FILE		"/version.txt"
#define VIDEO_INFO_FILE			"/channel.txt"

#define FIRMWARE_SYMBOL			"version\": "
#define VIDEO_SYMBOL			"name\": "

// p3k CMD
#define		IP_SET_CMD			"#NET-CONFIG"
#define 	IP_GET_CMD			"#NET-CONFIG?"	
#define		IP_RECV_HEAD		"@NET-CONFIG"

#define 	DHCP_SET_CMD		"#NET-DHCP"
#define 	DHCP_GET_CMD		"#NET-DHCP?"
#define		DHCP_RECV_HEAD		"@NET-DHCP"

#define     EDID_LIST_GET_CMD	"#EDID-LIST?"
#define		EDID_LIST_HEAD		"EDID-LIST "

//#define 	EDID_GET_CMD		"#EDID-ACTIVE? 1"
#define 	EDID_SET_CMD_1		"#EDID-MODE 1,"
#define 	EDID_SET_CMD_2		"#EDID-ACTIVE 1,"
#define 	EDID_GET_CMD		"#EDID-MODE? 1"
#define 	EDID_RECV_HEAD_1 	"@EDID-MODE 1,"
#define		EDID_RECV_HEAD_2    "@EDID-ACTIVE "


#define 	HDCP_GET_CMD		"#HDCP-STAT? 0"
#define		HDCP_RECV_HEAD		"@HDCP-STAT 0," 

#define 	DEVICE_STATUS_GET 	"#DEV-STATUS?"
#define		DEVICE_RECV_HEAD	"@DEV-STATUS "

#define		INPUT_SET_1_CMD		"#X-ROUTE out.hdmi.1.video.1,in.hdmi.1.video.1"
#define		INPUT_SET_2_CMD		"#X-ROUTE out.hdmi.1.video.1,in.hdmi.2.video.1"
#define		INPUT_SET_3_CMD		"#X-ROUTE out.hdmi.1.video.1,in.hdmi.3.video.1" //USB?
#define 	INPUT_GET_CMD  		"#X-ROUTE? out.hdmi.1.video.1"
#define 	INPUT_RECV_HEAD		"@X-ROUTE out.hdmi.1.video.1,in."

#define 	CHANNEL_SET_CMD 	"#KDS-CHANNEL-SELECT video,"
#define 	CHANNEL_GET_CMD 	"#KDS-CHANNEL-SELECT? video"
#define     CHANNEL_RECV_HEAD 	"@KDS-CHANNEL-SELECT video,"

//#define 	VIDEO_OUT_LIST		"#KDS-RESOL? "
#define 	VIDEO_OUT_GET_CMD 	"#KDS-SCALE?"
#define 	VIDEO_OUT_RECV_HEAD "@KDS-SCALE "


int init_p3k_client(char *ip, int port)
{
	int err;
	err = tcp_client_init(ip, port);
	return err;
}

int get_specified_string_from_file(char *file, char *symbol, char info_buf[][SIZE2])
{
	int i = 0;
	char cmd[SIZE1] = {0};
	char space_char[2] = "\"";
	char *str = NULL;
	char *chr = NULL;
	FILE *fd = NULL;
	char recv_buf[SIZE1] = {0};

	sprintf(cmd, "cat %s", file);
	fd = popen(cmd, "r");
	if (fd == NULL)
	{
		printf("popen fail");
		return -1;
	}
	i = 0;
	
	while(!feof(fd))
	{
		memset(recv_buf, 0, SIZE1);
		fgets(recv_buf, SIZE1, fd);

		if (str = strstr(recv_buf, symbol))
		{
			str += strlen(symbol);
			str = strstr(str, "\"");
			chr = strtok(str, space_char);
			printf("%s\n", chr);
			strcpy(info_buf[i++], chr);	
		}
	}
	int m, n;
	int len = 0;

	for (m = i-1; m >= 0 ; m--)
	{
		len = strlen(info_buf[m]);
		for (n = 0; n < len; n++)
		{
			info_buf[m][n] = toupper(info_buf[m][n]);
		}
	}

}

int get_FIREMARE_VERSION(const char *file, const char *symbol, char info_buf[][SIZE2])
{
	int i = 0;
	char cmd[SIZE1] = {0};
	char space_char[2] = "\"";
	char *str = NULL;
	char *chr = NULL;
	FILE *fd = NULL;
	char recv_buf[SIZE1] = {0};
	
	sprintf(cmd, "cat %s", file);
	fd = popen(cmd, "r");
	if (fd == NULL)
	{
		printf("popen fail");
		return -1;
	}

	i = 0;
	
	while(!feof(fd))
	{
		memset(recv_buf, 0, SIZE1);
		fgets(recv_buf, SIZE1, fd);

		if (str = strstr(recv_buf, symbol))
		{
			str += strlen(symbol);
			str = strstr(str, "v") + 1;
			chr = strtok(str, space_char);
			if (strstr(recv_buf, "board"))
			{	
				strcat(info_buf[i], "HW VER ");
			}
			if (strstr(recv_buf, "firmware"))
			{
				strcat(info_buf[i], "FM VER ");
			}
			strcat(info_buf[i++], chr);			
		}
	}
	
}


int send_cmd_common(char *cmd, char *head, char *param, char *content)
{
	char mcmd[SIZE1] = {0};
	char recv_buf[SIZE1] = {0};
	
	sprintf(mcmd, "%s", cmd);
	if (param != NULL)
	{
		sprintf(mcmd, "%s%s", mcmd, param);
	}
	
	if (send_p3k_cmd_wait_rsp(mcmd, recv_buf, sizeof(recv_buf)))
	{
		printf("send_p3k_cmd_wait_rsp fail\n");
		return -1;
	}
	
	if (recv_buf[strlen(recv_buf) - 2] == '\r' || recv_buf[strlen(recv_buf) - 1] == '\n')
		recv_buf[strlen(recv_buf) - 2] = '\0';
	
	if (recv_buf[strlen(recv_buf) - 1] == '\r' || recv_buf[strlen(recv_buf) - 1] == '\n')
		recv_buf[strlen(recv_buf) - 1] = '\0';

	printf("recv_buf:[%s] --[%d]\n", recv_buf, __LINE__);
	char * str = NULL;
	str = strstr(recv_buf, head);
	if (str != NULL)
	{
		str += strlen(head);
		if (content != NULL)
			strcpy(content, str);
	}
	return 0;
}

int split_string(char *src, char *splite_chr, char **buf, int *num)
{
	int i = 0;
	char *substr = NULL;
	substr = strtok(src, splite_chr);
	if (substr == NULL)
		return -1;

	buf[i++] = strdup(substr);

	while(substr = strtok(NULL, splite_chr))
	{
		buf[i++] = strdup(substr);
	}
	*num = i;
}

int  fill_ip_become_15_byte(char *str, char *dest)
{
	int i;
	int num = 0;
	int offset = 0;
	char tmp[4] = "000";
	char splite_chr[2] = ".";
	char buf1[20] = {0};
	char *buf[10];
	for (i = 0; i < 10; i++)
	{
		buf[i] = NULL;
	}

	split_string(str, splite_chr, buf, &num);

	for(i = 0; i < num; i++)
	{
		strcpy(tmp, "000");
		offset = 3 - strlen(buf[i]);
		strcpy(tmp + offset, buf[i]);
		strcat(buf1, tmp);		
		strcat(buf1, ".");
	}
	strncpy(dest, buf1, 15);
	printf("dest:[%s]\n", dest);
}

int get_ip(int NET_ID, char *IP, char *MASK, char *GATEWAY)
{
	int err;
	char *substr = NULL;
	char recv_buf[SIZE1] = {0};
	char splite_chr[2] = ",";
	
	char cmd[SIZE1] = {0};
	char respon_head[SIZE1] = {0};
	sprintf(cmd, "%s %d", IP_GET_CMD, NET_ID);
	sprintf(respon_head, "%s %d,", IP_RECV_HEAD, NET_ID);
	
	err = send_cmd_common(cmd, respon_head, NULL, recv_buf);
	if (err == -1)
	{
		return -1;
	}

	int i;
	int num = 0;
	char *ip_info[10];

	split_string(recv_buf, splite_chr, ip_info, &num);
	fill_ip_become_15_byte(ip_info[0], IP);
	fill_ip_become_15_byte(ip_info[1], MASK);
	fill_ip_become_15_byte(ip_info[2], GATEWAY);

	for (i = 0; i < num; i++)
	{
		if (ip_info[i] != NULL)
		{
			free(ip_info[i]);
			ip_info[i] = NULL;
		}
	}
	
	return 0;
}

int set_ip(int NET_ID, char *IP, char *MASK, char *GATEWAY)
{
	int err;
	char buf[SIZE1] = {0};
	char respon_head[SIZE1] = {0};
	
	sprintf(buf, "%d,%s,%s,%s", NET_ID, IP, MASK, GATEWAY);
	sprintf(respon_head, "%s %d,", IP_RECV_HEAD, NET_ID);
	
	err = send_cmd_common(IP_SET_CMD, respon_head, buf, NULL);
	return err;
}

/*
	#NET-DHCP?
	@NETDHCP netw_id,dhcp_mode
	0 off
	1 on
*/
int get_DHCP_status(int NET_ID, int *status)
{
	int err;
	char recv_buf[SIZE1] = {0};
	char cmd[SIZE1] = {0};
	char head[SIZE1] = {0};
	sprintf(cmd, "%s %d", DHCP_GET_CMD, NET_ID);
	sprintf(head, "%s %d,", DHCP_RECV_HEAD, NET_ID);
	err = send_cmd_common(cmd, head, NULL, recv_buf);

	*status = recv_buf[0] - '0';
	return err;
}

//#NETDHCP netw_id,dhcp_state
int set_DHCP_status(int NET_ID)
{
	int err;
	char cmd[SIZE1] = {0};
	char head[SIZE1] = {0};
	sprintf(cmd, "%s %d,1", DHCP_SET_CMD, NET_ID);
	sprintf(head, "%s %d,", DHCP_RECV_HEAD, NET_ID);
	err = send_cmd_common(DHCP_SET_CMD, DHCP_RECV_HEAD, NULL, NULL);
	return err;
}

//[0,DEFAULT],[2,SONY],[2,SONY]
int get_EDID_list(char EDID_buf[][SIZE2])
{
	int i = 0;
	int err;
	int num = 0;
	char *EDID_info[10];
	char recv_buf[SIZE1] = {0};
	char *tmp = NULL;
	err = send_cmd_common(EDID_LIST_GET_CMD, EDID_LIST_HEAD, NULL, recv_buf);
	if (err != 0)
		return -1;
	
	split_string(recv_buf, "]", EDID_info, &num);

	i = 0;
	for (i = 0; i < num; i++)
	{
		tmp = strstr(EDID_info[i], "[") + 1;
		if (tmp != NULL)
		{
			strcpy(EDID_buf[i], tmp);
		}
	}

	for (i = 0; i < num; i++)
	{
		if (EDID_info[i] != NULL)
		{
			free(EDID_info[i]);
			EDID_info[i] = NULL;
		}
	}
	
	return 0;
	
}

int set_EDID(int index)
{
	int err;
	char param[SIZE1] = {0};
	switch (index)
	{
		case -1:
		{
			sprintf(param, "%s", "PASSTHRU");		
			break;
		}
		
		//default
		case 0:
		{
			sprintf(param, "%s", "DEFAULT");	 
			break;
		}
		
		//custom
		default:
		{
			sprintf(param, "%s,%d", "CUSTOM", index);			
			break;
		}

	}

	err = send_cmd_common(EDID_SET_CMD_1, EDID_RECV_HEAD_1, param, NULL);
	if (index > 0)
	{
		memset(param, 0, SIZE1);
		sprintf(param, "%d", index);
		err = send_cmd_common(EDID_SET_CMD_2, EDID_RECV_HEAD_2, param, NULL);
	}
	
	return err;

}

//@EDID-MODE 1,CUSTOM,1
int get_EDID(int *EDID_TYPE)
{
	int err;
	char recv_buf[SIZE1] = {0};
	
	err = send_cmd_common(EDID_GET_CMD, EDID_RECV_HEAD_1, NULL, recv_buf);
	if (strstr(recv_buf, "PASSTHRU"))
	{
		*EDID_TYPE = -1;
	}
	if (strstr(recv_buf, "CUSTOM"))
	{
		*EDID_TYPE = recv_buf[7] - '0';
	}
	if (strstr(recv_buf, "DEFAULT"))
	{
		*EDID_TYPE = recv_buf[8] - '0';
	}
	
	return err;
  
}

//0 OFF 1 ON
int get_HDCP_status(int *HDCP_STATUS)
{
	int err;
	char recv_buf[2] = {0};
	err = send_cmd_common(HDCP_GET_CMD, HDCP_RECV_HEAD, NULL, recv_buf);
	*HDCP_STATUS = recv_buf[0]- '0';
	return err;
}

//HW VER 0.1  FW VER 0.1.0 
int get_FIRMWARE_INFO(char info_buf[][SIZE2]) //数组指针
{
	int err;
	get_FIREMARE_VERSION(FIRMWARE_INFO_FILE, FIRMWARE_SYMBOL, info_buf);
	return err;
}

/*
0	– power on
1	– standby
2	– FW Background Download
3	– IP fallback address
*/
int get_DEVICE_STATUS(int *Dvc_Status)
{
	int err;
	char recv_buf[2] = {0};
	err = send_cmd_common(DEVICE_STATUS_GET, DEVICE_RECV_HEAD, NULL, recv_buf);
	*Dvc_Status = recv_buf[0]- '0';
	return err;
}

int get_INPUT_INFO(char *recv_buf)
{
	int err;
	err = send_cmd_common(INPUT_GET_CMD, INPUT_RECV_HEAD, NULL, recv_buf);
	if (err == -1)
	{
		printf("send_cmd_common fail\n");
		return -1;
	}
	return 0;
}

int set_INPUT_INFO(int num)
{
	int err;
	switch(num)
	{
		case 1:
		{
			err = send_cmd_common(INPUT_SET_1_CMD, INPUT_RECV_HEAD, NULL, NULL);
			break;
		}

		case 2:
		{
			err = send_cmd_common(INPUT_SET_2_CMD, INPUT_RECV_HEAD, NULL, NULL);
			break;
		}

		case 3:
		{
			err = send_cmd_common(INPUT_SET_3_CMD, INPUT_RECV_HEAD, NULL, NULL);
			break;
		}

	}
	
	return err;

}

int VIDEO_LIST(char info_buf[][SIZE2])
{
	int err;
	err = get_specified_string_from_file(VIDEO_INFO_FILE, VIDEO_SYMBOL, info_buf);
	return err;
}

int select_voide_channel(int num)
{
	int err;
	char cmd[SIZE1] = {0};
	sprintf(cmd, "%s%d", CHANNEL_SET_CMD, num);
	
	err = send_cmd_common(cmd, CHANNEL_RECV_HEAD, NULL, NULL); 
	return err;
}

int get_current_voide_channel(int *channel_num)
{
	int err;
	char recv_buf[2] = {0};
	
	err = send_cmd_common(CHANNEL_GET_CMD, CHANNEL_RECV_HEAD, NULL, recv_buf);
	if (err == -1)
		return -1;
	
	*channel_num = recv_buf[0] - '0';
	return err;
}

//#KDS-SCALE?
//@KDS-SCALE value,res_type
//value: 0	– Pass Thru。1	– Scaling
//res_type 1,2,3,4....
//@KDS-SCALE 1,16
int get_VIDEO_OUT(int *video_data)
{
	
	int err;
	char recv_buf[SIZE1] = {0};
	err = send_cmd_common(VIDEO_OUT_GET_CMD, VIDEO_OUT_RECV_HEAD, NULL, recv_buf);
	if (err == -1)
		return -1;
	
	*video_data = atoi(&recv_buf[2]);
	
	return err;
}


