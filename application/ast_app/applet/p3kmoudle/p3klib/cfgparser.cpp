#include "json/json.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <sys/file.h> //for flock()

#include <math.h>

#include "cfgparser.h"
#include "funcexcute.h"
#include "ast_send_event.h"

#include "debugtool.h"

#include <errno.h>
#include <netdb.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>


Channel_Info 		g_channel_info;
Channel_Select		g_channel_select;

Audio_Info			g_audio_info;
Video_Info			g_video_info;
AutoSwitch_Info		g_autoswitch_info;
AVSetting_Info		g_avsetting_info;
EDID_Info			g_edid_info;
Device_Info			g_device_info;
Version_Info		g_version_info;
Time_Info			g_time_info;
User_Info			g_user_info;
VideoWall_Info		g_videowall_info;
Gateway_Info		g_gateway_info;
Network_Info		g_network_info;
Log_Info			g_log_info;

State_E				g_osd_enable;
int                 g_Udp_Socket;
int                 g_Udp_Inside_Socket;
ConnectionList_S    *g_connectionlist_info;

int 				g_init_control_port = 0;
int 				g_init_control_vlan = 0;

int 				g_bCfg = 0;

using namespace std;

char g_module[32] = "kds-7";

pthread_mutex_t g_cfg_lock;

int Cfg_InitModule(void)
{
	char module[32]="";
	char buf1[32] = "";

	mysystem("astparam r model_number",buf1,32);

#ifdef CONFIG_P3K_CLIENT
	if(strstr(buf1,"not defined") != 0)
	{
		strcpy(g_version_info.model,IPD_MODULE);
		DBG_WarnMsg("Cfg_InitModule not defined\n");
	}
	else
	{
		if(strcmp(buf1,IPD_MODULE) == 0)
			strcpy(g_version_info.model,IPD_MODULE);
		else if(strcmp(buf1,IPD_W_MODULE) == 0)
			strcpy(g_version_info.model,IPD_W_MODULE);
		else
			strcpy(g_version_info.model,IPD_MODULE);

		DBG_InfoMsg("Cfg_InitModule module = %s\n",g_version_info.model);
	}
#else
	if(strstr(buf1,"not defined") != 0)
	{
		strcpy(g_version_info.model,IPE_MODULE);
		DBG_WarnMsg("Cfg_InitModule not defined\n");
	}
	else
	{
		if(strcmp(buf1,IPE_MODULE) == 0)
			strcpy(g_version_info.model,IPE_MODULE);
		else if(strcmp(buf1,IPE_P_MODULE) == 0)
			strcpy(g_version_info.model,IPE_P_MODULE);
		else if(strcmp(buf1,IPE_W_MODULE) == 0)
			strcpy(g_version_info.model,IPE_W_MODULE);
		else
			strcpy(g_version_info.model,IPE_MODULE);

		DBG_InfoMsg("Cfg_InitModule module = %s\n",g_version_info.model);
	}

#endif

	return 0;
}

int Cfg_Check_File(char * path)
{
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("nAccessRet %s Failed\n",path);
		return -1;
	}

	Json::Reader reader;
	Json::Value root1;
	char pBuf[4096] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	int nRet = 0,nOffset = 0;
	while(1)
	{
//		printf("nOffset == %d sizeof(pBuf) = %d\n",nOffset,sizeof(pBuf));
		nRet = fread(pBuf+nOffset,1,sizeof(pBuf)-nOffset,fp);
//		printf("nRet == %d\n",nRet);
		if(nRet <= 0)
		{
			break;
		}
		else
		{
			nOffset += nRet;

			if(nOffset >= sizeof(pBuf))
				break;
		}

	}

	if(nOffset == 0)
	{
		DBG_ErrMsg("Read offset = 0 \n ");
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
		return -1;
	}

	if(strlen(pBuf) == 0)
	{
		DBG_ErrMsg("strlen(pBuf) = 0 \n ");
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
		return -1;
	}

	char begin[1]="";
	sscanf(pBuf,"%1s",&begin);

	if(begin[0] != '{')
	{
		DBG_ErrMsg("ERROR! star %s \n ",begin);
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
		return -1;
	}
	else if(reader.parse(pBuf, root1)== false)
	{
		DBG_ErrMsg("ERROR! %s is not json\n",path);
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
		return -1;
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}
int Cfg_Init(void)
{
	DBG_InfoMsg("Cfg_Init\n");
	pthread_mutex_init(&g_cfg_lock,NULL);

	if(g_bCfg == 1)
		Cfg_InitModule();
	else
		Cfg_Init_Version();

	Cfg_Create_DefaultFile();

	Cfg_Init_Channel();
	Cfg_Init_Audio();
	Cfg_Init_Video();
	Cfg_Init_AutoSwitch();
	Cfg_Init_AVSetting();
	Cfg_Init_EDID();
	Cfg_Init_Device();
	Cfg_Init_Time();
	Cfg_Init_User();
	Cfg_Init_VideoWall();
	Cfg_Init_Gateway();
	Cfg_Init_Network();
	Cfg_Init_Log();
	Cfg_Init_OSD();
	return 0;
}

int Cfg_Init_FromASTParam(void)
{
	DBG_InfoMsg("Cfg_Init_FromASTParam\n");
	return 0;
}

int Cfg_Init_Channel(void)
{
	DBG_InfoMsg("Cfg_Init_Channel\n");
#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_SEL_FILE);

	g_channel_select.video 	= 1;
	g_channel_select.audio 	= 1;
	g_channel_select.rs232 	= 1;
	g_channel_select.ir 	= 1;
	g_channel_select.usb 	= 1;
	g_channel_select.cec	= 1;

	//Check Channel cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		char buf1[16] = "";

		mysystem("astparam g ch_select_v",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.video = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.video = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.video);

			if(g_channel_select.video == 0)
				g_channel_select.video = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.video);
		}

		memset(buf1,0,16);
		mysystem("astparam g ch_select_a",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.audio = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.audio = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.audio);

			if(g_channel_select.audio == 0)
				g_channel_select.audio = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.audio);
		}

		memset(buf1,0,16);
		mysystem("astparam g ch_select_s",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.rs232 = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.rs232 = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.rs232);

			if(g_channel_select.rs232 == 0)
				g_channel_select.rs232 = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.rs232);
		}

		memset(buf1,0,16);
		mysystem("astparam g ch_select_r",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.ir = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.ir = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.ir);

			if(g_channel_select.ir == 0)
				g_channel_select.ir = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.ir);
		}

		memset(buf1,0,16);
		mysystem("astparam g ch_select_u",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.usb = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.usb = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.usb);

			if(g_channel_select.usb == 0)
				g_channel_select.usb = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.usb);
		}

		memset(buf1,0,16);
		mysystem("astparam g ch_select_c",buf1,16);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_select.cec = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_select.cec = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.cec);

			if(g_channel_select.cec == 0)
				g_channel_select.cec = 1;

			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_select.cec);
		}

		//create channel cfg from default value
		Cfg_Update(CHANNEL_INFO);
		return 0;
	}

	//Read	channel cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_CHANNEL_SELECT].empty())
		{
			Json::Value& root = root1[JSON_CHANNEL_SELECT];

			//init g_channel_info
			if(!root[JSON_CHAN_VIDEO].empty())
				if(root[JSON_CHAN_VIDEO].isInt())
					g_channel_select.video = root[JSON_CHAN_VIDEO].asInt();

			if(!root[JSON_CHAN_AUDIO].empty())
				if(root[JSON_CHAN_AUDIO].isInt())
					g_channel_select.audio = root[JSON_CHAN_AUDIO].asInt();

			if(!root[JSON_CHAN_RS232].empty())
				if(root[JSON_CHAN_RS232].isInt())
					g_channel_select.rs232 = root[JSON_CHAN_RS232].asInt();

			if(!root[JSON_CHAN_IR].empty())
				if(root[JSON_CHAN_IR].isInt())
					g_channel_select.ir = root[JSON_CHAN_IR].asInt();

			if(!root[JSON_CHAN_USB].empty())
				if(root[JSON_CHAN_USB].isInt())
					g_channel_select.usb = root[JSON_CHAN_USB].asInt();

			if(!root[JSON_CHAN_CEC].empty())
				if(root[JSON_CHAN_CEC].isInt())
					g_channel_select.cec = root[JSON_CHAN_CEC].asInt();

		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	if(g_bCfg == 1)
	{
		char cmd1[64] = "";

		sprintf(cmd1,"astparam s ch_select_v %04d",g_channel_select.video);
		system(cmd1);
		sprintf(cmd1,"astparam s ch_select_a %04d",g_channel_select.audio);
		system(cmd1);
		sprintf(cmd1,"astparam s ch_select_s %04d",g_channel_select.rs232);
		system(cmd1);
		sprintf(cmd1,"astparam s ch_select_r %04d",g_channel_select.ir);
		system(cmd1);
		sprintf(cmd1,"astparam s ch_select_u %04d",g_channel_select.usb);
		system(cmd1);
		sprintf(cmd1,"astparam s ch_select_c %04d",g_channel_select.cec);
		system(cmd1);

	}

#else

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_DEF_FILE);

	g_channel_info.channel_id = 1;
	strcpy(g_channel_info.channel_Name,"CH_01");

	//Check Channel cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		char* cmd1 = "astparam g ch_select";
		char buf1[64] = "";

		mysystem(cmd1,buf1,64);

		if(strstr(buf1,"not defined") != 0)
		{
			g_channel_info.channel_id = 1;
			DBG_WarnMsg("Cfg_Init_Channel not defined\n");
		}
		else
		{
			g_channel_info.channel_id = atoi(buf1);
			DBG_InfoMsg("Cfg_Init_Channel id = %d\n",g_channel_info.channel_id);
		}

		//create channel cfg from default value
		Cfg_Update(CHANNEL_INFO);
		return 0;
	}

	//Read  channel cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_CHANNEL].empty())
		{
			Json::Value& root = root1[JSON_CHANNEL];

			//init g_channel_info
			if(!root[JSON_CH_ID].empty())
			{
				if(root[JSON_CH_ID].isInt())
					g_channel_info.channel_id = root[JSON_CH_ID].asInt();
			}

			if(!root[JSON_CH_NAME].empty())
			{
				string ch_name = root[JSON_CH_NAME].asString();

				if(ch_name.size()<32)
					strcpy(g_channel_info.channel_Name,ch_name.c_str());
				else
					memcpy(g_channel_info.channel_Name,ch_name.c_str(),32);
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	if(g_bCfg == 1)
	{
		char cmd1[64] = "";
		char buf1[64] = "";

		sprintf(cmd1,"astparam s ch_select %04d",g_channel_info.channel_id);

		mysystem(cmd1,buf1,64);

	}
#endif

	return 0;
}

int Get_dante_name()
{
	DBG_InfoMsg("Cfg_Init_Audio g_bCfg =%d\n",g_bCfg);

	if(g_bCfg == 1)
	{
		DBG_WarnMsg("g_bCfg == 1\n");
		return -1;
	}

	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
	{
		DBG_InfoMsg("Start set_dante:1 \n");
		system("ipc @m_lm_set s set_dante:1");
		usleep(1000*1000);
		mysystem("astparam g dante_friendly_name", g_audio_info.dante_name,32);
	}

	return 0;
}
int Cfg_Init_Audio(void)
{
	DBG_InfoMsg("Cfg_Init_Audio\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif
	int bDanteUpdate = 0;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUDIO_FILE);

	g_audio_info.direction = DIRECTION_IN;
	g_audio_info.switch_mode = CONNECT_LAST;
	g_audio_info.input_pri[0] = AUDIO_IN_HDMI;
	g_audio_info.input_pri[1] = AUDIO_IN_ANALOG;

	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		g_audio_info.input_pri[2] = AUDIO_IN_DANTE;
	else
		g_audio_info.input_pri[2] = AUDIO_IN_NONE;

	g_audio_info.dst_port[0] = PORT_STREAM;
	g_audio_info.dst_port[1] = PORT_HDMI;

	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		g_audio_info.dst_port[2] = PORT_DANTE;
	else
		g_audio_info.dst_port[2] = PORT_NONE;

	g_audio_info.dst_port[3] = PORT_NONE;
	g_audio_info.source = AUDIO_IN_HDMI;
	sprintf(g_audio_info.dante_name,"dante");

	printf("g_bCfg = %d\n",g_bCfg);

	//Check autoswitch cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//get dante name
		if((g_bCfg == 0)&&(strcmp(g_version_info.model,IPE_P_MODULE) == 0))
			Get_dante_name();

		//create autoswitch cfg from default value
		Cfg_Update(AUDIO_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  autoawitch cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AUDIO].empty())
		{
			Json::Value& root = root1[JSON_AUDIO];

			if(!root[JSON_AUDIO_ANALOG_DIR].empty())
			{
				string mode = root[JSON_AUDIO_ANALOG_DIR].asString();

				if(mode == JSON_PARAM_OUT)
				{
					g_audio_info.direction = DIRECTION_OUT;
				}
				else
				{
					g_audio_info.direction = DIRECTION_IN;
				}
			}

			//init g_autoswitch_info
			if(!root[JSON_SWITCH_MODE].empty())
			{
				string mode = root[JSON_SWITCH_MODE].asString();

				if(mode == JSON_LAST_CONNECT)
				{
					g_audio_info.switch_mode = CONNECT_LAST;
				}
				else if(mode == JSON_PRIORITY)
				{
					g_audio_info.switch_mode = CONNECT_PRIROITY;
				}
				else if(mode == JSON_MANUAL)
				{
					g_audio_info.switch_mode = CONNECT_MANUAL;
				}

			}

			g_audio_info.input_pri[0] = AUDIO_IN_NONE;
			g_audio_info.input_pri[1] = AUDIO_IN_NONE;
			g_audio_info.input_pri[2] = AUDIO_IN_NONE;
			if(!root[JSON_PRIORITY].empty())
			{


				Json::Value& JsonPriorityArray = root[JSON_PRIORITY];
				//printf("JsonPriorityArray.size() = %d\n",JsonPriorityArray.size());

				int jjj = 0;

				for(unsigned int i = 0; i < JsonPriorityArray.size(); i++)
				{
					string input = JsonPriorityArray[i].asString();
					//printf("JsonPriorityArray[%d]:[%s]\n",i,input.c_str());

					if(input == JSON_AUDIO_DANTE)
					{
						if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
						{
							g_audio_info.input_pri[jjj] = AUDIO_IN_DANTE;
							jjj++;
						}
						continue;
					}
					else if(input == JSON_AUDIO_ANALOG)
					{
						if(g_audio_info.direction == DIRECTION_IN)
						{
							g_audio_info.input_pri[jjj] = AUDIO_IN_ANALOG;
							jjj++;
						}
						continue;
					}
					else if(input == JSON_AUDIO_HDMI)
					{
						g_audio_info.input_pri[jjj] = AUDIO_IN_HDMI;
						jjj++;
						continue;
					}
					else
					{
						continue;
					}
				}
			}

			g_audio_info.dst_port[0] = PORT_NONE;
			g_audio_info.dst_port[1] = PORT_NONE;
			g_audio_info.dst_port[2] = PORT_NONE;
			g_audio_info.dst_port[3] = PORT_NONE;
			if(!root[JSON_AUDIO_DEST].empty())
			{
				int jjj = 0;
				Json::Value& JsonDstArray = root[JSON_AUDIO_DEST];
				//printf("JsonDstArray.size() = %d\n",JsonDstArray.size());

				if(JsonDstArray.size() > 4)
					JsonDstArray.resize(4);

				for(unsigned int i = 0; i < JsonDstArray.size(); i++)
				{
					string output = JsonDstArray[i].asString();
					//printf("JsonDstArray[%d]:[%s]\n",i,output.c_str());

					if(output == JSON_AUDIO_DANTE)
					{
						if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
						{
							g_audio_info.dst_port[jjj] = PORT_DANTE;
							jjj++;
						}
						continue;
					}
					else if(output == JSON_AUDIO_ANALOG)
					{
						if(g_audio_info.direction == DIRECTION_OUT)
						{
							g_audio_info.dst_port[jjj] = PORT_ANALOG_AUDIO;
							jjj++;
						}

						continue;
					}
					else if(output == JSON_AUDIO_HDMI)
					{
						if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
						{
							g_audio_info.dst_port[jjj] = PORT_HDMI;
							jjj++;
						}
						continue;
					}
					else if(output == JSON_AUDIO_LAN)
					{
						g_audio_info.dst_port[jjj] = PORT_STREAM;
						jjj++;
						continue;
					}
					else
					{
						continue;
					}
				}
			}

			if(!root[JSON_SOURCE_SELECT].empty())
			{
				string input = root[JSON_SOURCE_SELECT].asString();
				if(input == JSON_AUDIO_DANTE)
				{
					if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
						g_audio_info.source = AUDIO_IN_DANTE;
					else
						g_audio_info.source = AUDIO_IN_NONE;
				}
				else if(input == JSON_AUDIO_ANALOG)
				{
					if(g_audio_info.direction == DIRECTION_IN)
						g_audio_info.source = AUDIO_IN_ANALOG;
					else
						g_audio_info.source = AUDIO_IN_NONE;
				}
				else if(input == JSON_AUDIO_HDMI)
					g_audio_info.source = AUDIO_IN_HDMI;
				else if(input == JSON_AUDIO_NONE)
					g_audio_info.source = AUDIO_IN_NONE;
				else
					g_audio_info.source = AUDIO_IN_NONE;

			}

			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(!root[JSON_DANTE_NAME].empty())
				{
					string name = root[JSON_DANTE_NAME].asString();
					sprintf(g_audio_info.dante_name,name.c_str());
				}
				else
				{
					if(g_bCfg == 0)
					{
						Get_dante_name();
						bDanteUpdate = 1;
					}
				}
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	if((strcmp(g_version_info.model,IPE_P_MODULE) == 0)&&(bDanteUpdate == 1))
		Cfg_Update(AUDIO_INFO);

	return 0;
}

int Cfg_Init_Video(void)
{
	DBG_InfoMsg("Cfg_Init_Video\n");
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);

	g_video_info.force_rgb = 1;
	g_video_info.scale_mode = 0;
	g_video_info.res_type = 0;

	g_audio_info.conn_guard_time = 10;

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(VIDEO_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  autoawitch cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SIGNAL].empty())
		{
			Json::Value& root = root1[JSON_AV_SIGNAL];

			if(!root[JSON_AV_RGB].empty())
			{
				string mode = root[JSON_AV_RGB].asString();

				if(mode == JSON_PARAM_OFF)
				{
					g_video_info.force_rgb = 0;
				}
				else
				{
					g_video_info.force_rgb = 1;
				}
			}

			if(!root[JSON_AV_OUT_RES].empty())
			{
				string res = root[JSON_AV_OUT_RES].asString();

				if(res == "2160p30")
				{
					g_video_info.scale_mode = 1;
					g_video_info.res_type = 74;
				}
				else if(res == "2160p25")
				{
					g_video_info.scale_mode = 1;
					g_video_info.res_type = 73;
				}
				else if(res == "1080p60")
				{
					g_video_info.scale_mode = 1;
					g_video_info.res_type = 16;
				}
				else if(res == "1080p50")
				{
					g_video_info.scale_mode = 1;
					g_video_info.res_type = 31;
				}
				else if(res == "720p60")
				{
					g_video_info.scale_mode = 1;
					g_video_info.res_type = 4;
				}
				else
				{
					g_video_info.scale_mode = 0;
					g_video_info.res_type = 0;
				}
			}

			if(!root[JSON_AV_AUD_GUARD].empty())
			{
				if(root[JSON_AV_AUD_GUARD].isInt())
					g_audio_info.conn_guard_time = root[JSON_AV_AUD_GUARD].asInt();

				if(g_audio_info.conn_guard_time < 0)
					g_audio_info.conn_guard_time = 0;
				else if(g_audio_info.conn_guard_time > 90)
					g_audio_info.conn_guard_time = 90;
			}

		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);


	return 0;
}

int Cfg_Init_AutoSwitch(void)
{
	DBG_InfoMsg("Cfg_Init_AutoSwitch\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_FILE);

	g_autoswitch_info.switch_mode = CONNECT_LAST;
	g_autoswitch_info.input_pri[0] = 1;
	g_autoswitch_info.input_pri[1] = 2;
	g_autoswitch_info.input_pri[2] = 3;
#ifdef CONFIG_P3K_HOST
	g_autoswitch_info.source = 1;
#else
	g_autoswitch_info.source = 2;
#endif
	//Check autoswitch cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(AUTOSWITCH_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  autoawitch cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AUTOSWITCH].empty())
		{
			Json::Value& root = root1[JSON_AUTOSWITCH];

#ifdef CONFIG_P3K_HOST
			//init g_autoswitch_info
			if(!root[JSON_SWITCH_MODE].empty())
			{
				string mode = root[JSON_SWITCH_MODE].asString();
				//printf("g_autoswitch_info.switch_mode = %s\n",mode.c_str());

				if(mode == JSON_LAST_CONNECT)
				{
					g_autoswitch_info.switch_mode = CONNECT_LAST;
				}
				else if(mode == JSON_PRIORITY)
				{
					g_autoswitch_info.switch_mode = CONNECT_PRIROITY;
				}
				else if(mode == JSON_MANUAL)
				{
					g_autoswitch_info.switch_mode = CONNECT_MANUAL;
				}

			}

			if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
			{
				if(!root[JSON_PRIORITY].empty())
				{
					Json::Value& JsonPriorityArray = root[JSON_PRIORITY];
					//printf("JsonPriorityArray.size() = %d\n",JsonPriorityArray.size());

					for(unsigned int i = 0; i < JsonPriorityArray.size(); i++)
					{
						string input = JsonPriorityArray[i].asString();
						//printf("JsonPriorityArray[%d]:[%s]\n",i,input.c_str());

						if(input == JSON_HDMI_1)
						{
							g_autoswitch_info.input_pri[i] = 1;
							continue;
						}
						else if(input == JSON_USBC_2)
						{
							g_autoswitch_info.input_pri[i] = 2;
							continue;
						}
					}
				}

				if(!root[JSON_SOURCE_SELECT].empty())
				{
					string input = root[JSON_SOURCE_SELECT].asString();
					if(input == JSON_HDMI_1)
						g_autoswitch_info.source = 1;
					else if(input == JSON_USBC_2)
						g_autoswitch_info.source = 2;
				}
			}
			else
			{
				if(!root[JSON_PRIORITY].empty())
				{
					Json::Value& JsonPriorityArray = root[JSON_PRIORITY];
					//printf("JsonPriorityArray.size() = %d\n",JsonPriorityArray.size());

					for(unsigned int i = 0; i < JsonPriorityArray.size(); i++)
					{
						string input = JsonPriorityArray[i].asString();
						//printf("JsonPriorityArray[%d]:[%s]\n",i,input.c_str());

						if(input == JSON_HDMI_1)
						{
							g_autoswitch_info.input_pri[i] = 1;
							continue;
						}
						else if(input == JSON_HDMI_2)
						{
							g_autoswitch_info.input_pri[i] = 2;
							continue;
						}
						else if(input == JSON_USBC_3)
						{
							g_autoswitch_info.input_pri[i] = 3;
							continue;
						}
					}
				}

				if(!root[JSON_SOURCE_SELECT].empty())
				{
					string input = root[JSON_SOURCE_SELECT].asString();
					if(input == JSON_HDMI_1)
						g_autoswitch_info.source = 1;
					else if(input == JSON_HDMI_2)
						g_autoswitch_info.source = 2;
					else if(input == JSON_USBC_3)
						g_autoswitch_info.source = 3;
				}
			}
#else
			if(strcmp(g_version_info.model,IPD_W_MODULE) == 0)
			{
				g_autoswitch_info.source = 2;
			}
			else
			{
				if(!root[JSON_SOURCE_SELECT].empty())
				{
					string input = root[JSON_SOURCE_SELECT].asString();
					if(input == JSON_HDMI_1)
						g_autoswitch_info.source = 1;
					else if(input == JSON_STREAM)
						g_autoswitch_info.source = 2;
				}
			}


#endif
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	//printf("Cfg_Init_AutoSwitch switch_mode:%d,input_pri[0]:%d input_pri[1]:%d input_pri[2]:%d end\n",
	//	g_autoswitch_info.switch_mode,g_autoswitch_info.input_pri[0],g_autoswitch_info.input_pri[1],g_autoswitch_info.input_pri[2]);
	return 0;
}

int Cfg_Init_AVSetting(void)
{
	DBG_InfoMsg("Cfg_Init_AVSetting\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SETTING_FILE);

	g_avsetting_info.volume = 80;
	g_avsetting_info.mute_mode = OFF;
	g_avsetting_info.action = CODEC_ACTION_PLAY;
	g_avsetting_info.hdcp_mode[0] = ON;
	g_avsetting_info.hdcp_mode[1] = ON;
	g_avsetting_info.hdcp_mode[2] = ON;

	//Check autoswitch cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(AV_SETTING);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  avsetting cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SETTING].empty())
		{
			Json::Value& root = root1[JSON_AV_SETTING];

			//init g_avsetting_info
			if(!root[JSON_AV_VOLUME].empty())
			{
				if(root[JSON_AV_VOLUME].isInt())
				{
					g_avsetting_info.volume = root[JSON_AV_VOLUME].asInt();
				}
			}

			if(!root[JSON_AV_MUTE].empty())
			{
				string mode =  root[JSON_AV_MUTE].asString();

				if(mode == JSON_PARAM_ON)
					g_avsetting_info.mute_mode = ON;
				else
					g_avsetting_info.mute_mode = OFF;
			}

			if(!root[JSON_AV_ACTION].empty())
			{
				string action =  root[JSON_AV_ACTION].asString();

				if(action == JSON_AV_STOP)
					g_avsetting_info.action = CODEC_ACTION_STOP;
				else
					g_avsetting_info.action = CODEC_ACTION_PLAY;
			}

#ifdef CONFIG_P3K_HOST
			if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
			{
				if(!root[JSON_AV_HDCP].empty())
				{
					Json::Value& JsonHDCP = root[JSON_AV_HDCP];

					for(int i = 0; i < 2; i++)
					{
						string mode;
						if((i == 0)&&(!JsonHDCP[JSON_HDMI_1].empty()))
							mode =  JsonHDCP[JSON_HDMI_1].asString();
						else if((i == 1)&&(!JsonHDCP[JSON_USBC_2].empty()))
							mode =  JsonHDCP[JSON_USBC_2].asString();
						else
							continue;

						if(mode == JSON_PARAM_ON)
							g_avsetting_info.hdcp_mode[i]= ON;
						else if(mode == JSON_PARAM_OFF)
							g_avsetting_info.hdcp_mode[i] = OFF;
					}

				}			}
			else
			{
				if(!root[JSON_AV_HDCP].empty())
				{
					Json::Value& JsonHDCP = root[JSON_AV_HDCP];

					for(int i = 0; i < 3; i++)
					{
						string mode;
						if((i == 0)&&(!JsonHDCP[JSON_HDMI_1].empty()))
							mode =  JsonHDCP[JSON_HDMI_1].asString();
						else if((i == 1)&&(!JsonHDCP[JSON_HDMI_2].empty()))
							mode =  JsonHDCP[JSON_HDMI_2].asString();
						else if((i == 2)&&(!JsonHDCP[JSON_USBC_3].empty()))
							mode =  JsonHDCP[JSON_USBC_3].asString();
						else
							continue;

						if(mode == JSON_PARAM_ON)
							g_avsetting_info.hdcp_mode[i]= ON;
						else if(mode == JSON_PARAM_OFF)
							g_avsetting_info.hdcp_mode[i] = OFF;
					}

				}
			}
#endif
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

int Cfg_Init_EDID(void)
{
	DBG_InfoMsg("Cfg_Init_EDID\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_SETTING_FILE);

	g_edid_info.lock_mode = ON;
	g_edid_info.edid_mode = DEFAULT;
	sprintf(g_edid_info.net_src,"0.0.0.0");
	g_edid_info.active_id = 0;

	//Check log cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(EDID_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  Log cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_EDID_SETTING].empty())
		{
			Json::Value& root = root1[JSON_EDID_SETTING];

			//init g_channel_info
			if(!root[JSON_EDID_LOCK].empty())
			{
				string mode = root[JSON_EDID_LOCK].asString();
				if(mode == JSON_PARAM_ON)
					g_edid_info.lock_mode = ON;
				else
					g_edid_info.lock_mode = OFF;
			}

			if(!root[JSON_EDID_MODE].empty())
			{
				string mode = root[JSON_EDID_MODE].asString();

				if(mode == JSON_EDID_PASSTHRU)
					g_edid_info.edid_mode = PASSTHRU;
				else if(mode == JSON_EDID_CUSTOM)
					g_edid_info.edid_mode = CUSTOM;
				else
					g_edid_info.edid_mode = DEFAULT;
			}

			if(!root[JSON_EDID_SRC].empty())
			{
				string src = root[JSON_EDID_SRC].asString();
				strcpy(g_edid_info.net_src,src.c_str());
			}

			if(!root[JSON_EDID_ACTIVE].empty())
			{
				string id = root[JSON_EDID_ACTIVE].asString();
				g_edid_info.active_id = atoi(id.c_str());
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

int Cfg_Init_Device(void)
{
	DBG_InfoMsg("Cfg_Init_Device\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DEVICE_FILE);

	g_device_info.fp_lock = OFF;
	g_device_info.standby_time = 30;
	GetBoardInfo(BOARD_MAC, g_device_info.mac_addr, 32);
	GetBoardInfo(BOARD_SN, g_device_info.sn, 32);

	char buf1[64] = "";

	mysystem("astparam g hostname_customized",buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		GetBoardInfo(BOARD_HOSTNAME, g_device_info.hostname, MAX_DEV_NAME_LEN);
	}
	else
	{
		strcpy(g_device_info.hostname,buf1);
	}
		//Check log cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(DEVICE_INFO);
		return 0;
	}

	//Read  Log cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_DEV_SETTING].empty())
		{
			Json::Value& root = root1[JSON_DEV_SETTING];

			if(!root[JSON_DEV_FP_LOCK].empty())
			{
				string mode = root[JSON_DEV_FP_LOCK].asString();
				if(mode == JSON_PARAM_ON)
				{
					g_device_info.fp_lock = ON;
					//printf("g_device_info.fp_lock = ON;\n");
				}
				else
				{
					g_device_info.fp_lock = OFF;
					//printf("g_device_info.fp_lock = OFF;\n");
				}
			}

			if(!root[JSON_DEV_STANDBY_TIME].empty())
			{
				if(root[JSON_DEV_STANDBY_TIME].isInt())
					g_device_info.standby_time = root[JSON_DEV_STANDBY_TIME].asInt();
			}

			if(!root[JSON_DEV_HOSTNAME].empty())
			{
				string hostname = root[JSON_DEV_HOSTNAME].asString();

				if(hostname.size() > 1)
				{
					strcpy(g_device_info.hostname,hostname.c_str());

					if(g_bCfg == 1)
					{
						char cmd[128] = "";
						sprintf(cmd,"astparam s hostname_customized %s",g_device_info.hostname);
						system(cmd);
					}
				}
			}
		}
	}
    if(g_network_info.beacon_en == ON)
    {
         EX_Beacon(1,1,g_network_info.beacon_time);
    }

	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	Cfg_Update(DEVICE_INFO);
	return 0;
}

int Cfg_Init_Version(void)
{
	DBG_InfoMsg("Cfg_Init_Version\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VERSION_FILE);

	//GetBoardInfo(BOARD_MODEL, g_version_info.model, 64);
	Cfg_InitModule();
	GetBoardInfo(BOARD_HW_VERSION, g_version_info.hw_version, 16);
	GetBoardInfo(BOARD_FW_VERSION, g_version_info.fw_version, 16);
	GetBoardInfo(BOARD_BUILD_DATE, g_version_info.build_time, 16);

	sprintf(g_version_info.file_version,"1.0.0");

	char time[64] = "";
	mysystem("astparam misc g upg_time",time,64);

	time_t secTime;
	struct tm *ptime =NULL;
	if(strstr(time,"not defined") != 0)
		secTime = 0;
	else
		secTime = atol(time);

	ptime = localtime(&secTime);
	sprintf(g_version_info.upg_time,"%02d-%02d-%04d,%02d:%02d:%02d",ptime->tm_mon+1,ptime->tm_mday,ptime->tm_year+1900,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);

	char stb_ver[64] = "";
	mysystem("astparam misc g stb_ver",stb_ver,64);
	if(strstr(stb_ver,"not defined") != 0)
		strcpy(g_version_info.standby_version,g_version_info.fw_version);
	else
		strcpy(g_version_info.standby_version,stb_ver);

//	sprintf(g_version_info.upg_time,"01-01-2020,00:00:00");


	//Check version cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(VERSION_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  version cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
//	sprintf(g_version_info.standby_version,"1.0.0");
//	sprintf(g_version_info.upg_time,"01-01-2020,00:00:00");
		if(!root1[JSON_VERSION].empty())
		{
			Json::Value& root = root1[JSON_VERSION];

			if(!root[JSON_VER_FILE].empty())
			{
				string info = root[JSON_VER_FILE].asString();
				sprintf(g_version_info.file_version,info.c_str());
			}
			if(!root[JSON_VER_UPG_TIME].empty())
			{
				string info = root[JSON_VER_UPG_TIME].asString();
				sprintf(g_version_info.upg_time,info.c_str());
			}
			if(!root[JSON_VER_STB_VERSION].empty())
			{
				string info = root[JSON_VER_STB_VERSION].asString();
				sprintf(g_version_info.standby_version,info.c_str());
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	Cfg_Update(VERSION_INFO);
	return 0;
}

int Cfg_Init_Time(void)
{
	DBG_InfoMsg("Cfg_Init_Time\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,TIME_FILE);

	g_time_info.time_zone = 0;
	g_time_info.daylight_saving = 0;
	g_time_info.ntp_mode = OFF;
	g_time_info.ntp_sync_hour = 0;
	g_time_info.ntp_server_status = OFF;
	sprintf(g_time_info.ntp_server,"0.0.0.0");

	//Check time cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(TIME_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read Time cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_TIME_SETTING].empty())
		{
			Json::Value& root = root1[JSON_TIME_SETTING];

			if(!root[JSON_TIME_ZONE].empty())
				if(root[JSON_TIME_ZONE].isInt())
					g_time_info.time_zone = root[JSON_TIME_ZONE].asInt();

			if(!root[JSON_TIME_DAYLIGHT].empty())
				if(root[JSON_TIME_DAYLIGHT].isInt())
					g_time_info.daylight_saving = root[JSON_TIME_DAYLIGHT].asInt();

			if(!root[JSON_NTP_SERVER].empty())
			{
				Json::Value& ntp = root[JSON_NTP_SERVER];

				if(!ntp[JSON_NTP_MODE].empty())
				{
					string mode = ntp[JSON_NTP_MODE].asString();
					if(mode == JSON_PARAM_ON)
						g_time_info.ntp_mode = ON;
					else
						g_time_info.ntp_mode = OFF;
				}

				if(!ntp[JSON_NTP_STATUS].empty())
				{
					string mode = ntp[JSON_NTP_STATUS].asString();
					if(mode == JSON_PARAM_ON)
						g_time_info.ntp_server_status= ON;
					else
						g_time_info.ntp_server_status = OFF;
				}

				if(!ntp[JSON_NTP_SYNC_HOUR].empty())
					if(ntp[JSON_NTP_SYNC_HOUR].isInt())
						g_time_info.ntp_sync_hour = ntp[JSON_NTP_SYNC_HOUR].asInt();

				if(!ntp[JSON_NTP_IP].empty())
				{
					string ip = ntp[JSON_NTP_IP].asString();
					sprintf(g_time_info.ntp_server,ip.c_str());
				}
			}

		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

int Cfg_Init_User(void)
{
	DBG_InfoMsg("Cfg_Init_User\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,USER_FILE);

	sprintf(g_user_info.user_name, "admin");
	sprintf(g_user_info.password, "admin");
	g_user_info.user_role = 1;	//admin
	g_user_info.logout_time = 10;
	g_user_info.seurity_status = OFF;

	//Check user cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(USER_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  user cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_USER_SETTING].empty())
		{
			Json::Value& root = root1[JSON_USER_SETTING];

			if(!root[JSON_USER_NAME].empty())
			{
				string name = root[JSON_USER_NAME].asString();

				if(name.size() < 32)
					strcpy(g_user_info.user_name,name.c_str());
				else
					memcpy(g_user_info.user_name,name.c_str(),31);
			}

			if(!root[JSON_USER_PASS].empty())
			{
				string pass = root[JSON_USER_PASS].asString();

				if(pass.size() < 32)
					strcpy(g_user_info.password,pass.c_str());
				else
					memcpy(g_user_info.password,pass.c_str(),31);
			}

			if(!root[JSON_USER_ROLE].empty())
			{
				string role = root[JSON_USER_ROLE].asString();

				if(role == JSON_USER_ADMIN)
					g_user_info.user_role = 1;
				else
					g_user_info.user_role = 0;
			}

			if(!root[JSON_USER_LOGOUT].empty())
			{
				if(root[JSON_USER_LOGOUT].isInt())
					g_user_info.logout_time = root[JSON_USER_LOGOUT].asInt();
			}

			if(!root[JSON_USER_SECUR].empty())
			{
				string mode = root[JSON_USER_SECUR].asString();

				if(mode == JSON_PARAM_ON)
					g_user_info.seurity_status = ON;
				else
					g_user_info.seurity_status = OFF;
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);		return 0;
}

int Cfg_Init_VW_fromParam(void)
{
	char* cmd1 = "astparam g vw_stretch_type";
	char buf1[64] = "";

	mysystem(cmd1,buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		g_videowall_info.stretch_type = 0;
	}
	else
	{
		int type = atoi(buf1);
		if(type == 2)
		{
			g_videowall_info.stretch_type = 0;
		}
		else
		{
			g_videowall_info.stretch_type = 1;
		}
	}

	DBG_InfoMsg("Cfg_Init_VW_fromParam stretch_type = %d\n",g_videowall_info.stretch_type);

	memset(buf1,0,64);
	mysystem("astparam g vw_rotate",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		g_videowall_info.rotation = ROTATION_0;
	else
	{
		int rotation = atoi(buf1);

		if(rotation == 3)
			g_videowall_info.rotation = ROTATION_180;
		else if(rotation == 6)
			g_videowall_info.rotation = ROTATION_270;
		else if(rotation == 5)
			g_videowall_info.rotation = ROTATION_90;
		else
			g_videowall_info.rotation = ROTATION_0;
	}

	DBG_InfoMsg("Cfg_Init_VW_fromParam rotation = %d\n",g_videowall_info.rotation);

	memset(buf1,0,64);
	mysystem("astparam g vw_pos_max_row",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		g_videowall_info.vertical_count = 1;
	else{
		g_videowall_info.vertical_count = atoi(buf1) + 1;

		if(g_videowall_info.vertical_count > 16)
			g_videowall_info.vertical_count = 16;
	}

	memset(buf1,0,64);
	mysystem("astparam g vw_pos_max_col",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		g_videowall_info.horizontal_count = 1;
	else{
		g_videowall_info.horizontal_count = atoi(buf1) + 1;

		if(g_videowall_info.horizontal_count > 16)
			g_videowall_info.horizontal_count = 16;
	}


	memset(buf1,0,64);
	mysystem("astparam g vw_row",buf1,64);
	int row = 0;
	if(strstr(buf1,"not defined") != 0)
		row = 1;
	else{
		row = atoi(buf1);

		if(row > 16)
			row = 16;
	}

	memset(buf1,0,64);
	mysystem("astparam g vw_column",buf1,64);
	int col =0;
	if(strstr(buf1,"not defined") != 0)
		col = 1;
	else{
		col = atoi(buf1);

		if(col > 16)
			col = 16;
	}
	g_videowall_info.relative_position = row * g_videowall_info.horizontal_count + col + 1;

	DBG_InfoMsg("Cfg_Init_VW_fromParam vw_column = %d\n",g_videowall_info.horizontal_count);


	return 0;
}

int Cfg_InitParam_VW_fromJson(void)
{
	char cmd1[64] = "";

	if(g_videowall_info.stretch_type == 0)
		system("astparam s vw_stretch_type 2");
	else
		system("astparam s vw_stretch_type 1");

	if(g_videowall_info.rotation == ROTATION_180)
		system("astparam s vw_rotate 3");
	else if(g_videowall_info.rotation == ROTATION_270)
		system("astparam s vw_rotate 6");
	else if(g_videowall_info.rotation == ROTATION_90)
		system("astparam s vw_rotate 5");
	else
		system("astparam s vw_rotate 0");

	if((g_videowall_info.vertical_count >= 0)&&(g_videowall_info.vertical_count <= 16))
	{
		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_max_row %d",g_videowall_info.vertical_count-1);
		system(cmd1);

		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_pos_max_row %d",g_videowall_info.vertical_count-1);
		system(cmd1);
	}

	if((g_videowall_info.horizontal_count >= 0)&&(g_videowall_info.horizontal_count <= 16))
	{
		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_max_column %d",g_videowall_info.horizontal_count-1);
		system(cmd1);

		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_pos_max_col %d",g_videowall_info.horizontal_count-1);
		system(cmd1);
	}

	int row = (g_videowall_info.relative_position-1) / g_videowall_info.vertical_count;
	int col = (g_videowall_info.relative_position-1) % g_videowall_info.vertical_count;


	if((row >= 0)&&(row <= 15)&&(col >= 0)&&(col <= 15))
	{
		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_row %d",row);
		system(cmd1);
		memset(cmd1,0,64);
		sprintf(cmd1,"astparam s vw_column %d",col);
		system(cmd1);
	}

	return 0;
}

int Cfg_Init_VideoWall(void)
{
	DBG_InfoMsg("Cfg_Init_VideoWall\n");

#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VIDEOWALL_FILE);

	g_videowall_info.horizontal_count = 1;
	g_videowall_info.vertical_count = 1;
	g_videowall_info.relative_position = 1;
	g_videowall_info.stretch_type = 0;
	g_videowall_info.rotation = 0;
	g_videowall_info.bezel_horizontal_value = 0;
	g_videowall_info.bezel_vertical_value = 0;
	g_videowall_info.bezel_horizontal_offset = 0;
	g_videowall_info.bezel_vertical_offset = 0;

	//Check user cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		Cfg_Init_VW_fromParam();

		//create channel cfg from default value
		Cfg_Update(VIDEOWALL_INFO);
		return 0;
	}

	//Read  user cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_VW_SETTING].empty())
		{
			Json::Value& root = root1[JSON_VW_SETTING];

			if(!root[JSON_VW_H_COUNT].empty())
				if(root[JSON_VW_H_COUNT].isInt())
					g_videowall_info.horizontal_count = root[JSON_VW_H_COUNT].asInt();

			if(!root[JSON_VW_V_COUNT].empty())
				if(root[JSON_VW_V_COUNT].isInt())
					g_videowall_info.vertical_count= root[JSON_VW_V_COUNT].asInt();

			if(!root[JSON_VW_POSITION].empty())
				if(root[JSON_VW_POSITION].isInt())
					g_videowall_info.relative_position= root[JSON_VW_POSITION].asInt();

			if(!root[JSON_VW_BEZEL].empty())
			{
				Json::Value& bezel = root[JSON_VW_BEZEL];

				if(!bezel[JSON_VW_BEZEL_H_VALUE].empty())
					if(bezel[JSON_VW_BEZEL_H_VALUE].isInt())
						g_videowall_info.bezel_horizontal_value = bezel[JSON_VW_BEZEL_H_VALUE].asInt();

				if(!bezel[JSON_VW_BEZEL_V_VALUE].empty())
					if(bezel[JSON_VW_BEZEL_V_VALUE].isInt())
						g_videowall_info.bezel_vertical_value = bezel[JSON_VW_BEZEL_V_VALUE].asInt();

				if(!bezel[JSON_VW_BEZEL_H_OFFSET].empty())
					if(bezel[JSON_VW_BEZEL_H_OFFSET].isInt())
						g_videowall_info.bezel_horizontal_offset = bezel[JSON_VW_BEZEL_H_OFFSET].asInt();

				if(!bezel[JSON_VW_BEZEL_V_OFFSET].empty())
					if(bezel[JSON_VW_BEZEL_V_OFFSET].isInt())
						g_videowall_info.bezel_vertical_offset = bezel[JSON_VW_BEZEL_V_OFFSET].asInt();
			}

			if(!root[JSON_VW_STRETCH].empty())
			{
				string mode = root[JSON_VW_STRETCH].asString();

				if(mode == JSON_VW_STRETCH_OUT)
					g_videowall_info.stretch_type = 1;
				else
					g_videowall_info.stretch_type = 0;
			}

			if(!root[JSON_VW_ROTATION].empty())
			{
				if(root[JSON_VW_ROTATION].isInt())
				{
					int rotation = root[JSON_VW_ROTATION].asInt();

					if((rotation == ROTATION_0)
						||(rotation == ROTATION_90)
						||(rotation == ROTATION_180)
						||(rotation == ROTATION_270))
						g_videowall_info.rotation = rotation;
					else
						g_videowall_info.rotation = rotation;
				}
			}

		}
	}


	flock(fileno(fp), LOCK_UN);
	fclose(fp);


	if(g_bCfg == 1)
	{
		Cfg_InitParam_VW_fromJson();
		return 0;
	}

	return 0;
}

int Cfg_Init_GW_fromParam()
{
	//	astparam s soip_guest_on y
	//  astparam s soip_type 2
	//  astparam s soip_port $_para1

	char* cmd1 = "astparam g soip_guest_on";
	char buf1[64] = "";

	mysystem(cmd1,buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		g_gateway_info.rs232_mode = ON;
	}
	else if(strstr(buf1,"y") != 0)
	{
		g_gateway_info.rs232_mode = ON;
	}
	else
	{
		g_gateway_info.rs232_mode = OFF;
	}
	DBG_InfoMsg("Cfg_Init_GW_fromParam rs232_mode = %d\n",g_gateway_info.rs232_mode);

	memset(buf1,0,64);
	mysystem("astparam g soip_port",buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		g_gateway_info.rs232_port = 5001;
	}
	else
	{
		g_gateway_info.rs232_port = atoi(buf1);
	}
	DBG_InfoMsg("Cfg_Init_GW_fromParam rs232_mode = %d\n",g_gateway_info.rs232_port);

	memset(buf1,0,64);
	mysystem("astparam g s0_baudrate",buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		g_gateway_info.rs232_param.rate = 115200;
		g_gateway_info.rs232_param.bitWidth = 8;
		g_gateway_info.rs232_param.parity = PARITY_NONE;
		g_gateway_info.rs232_param.stopBitsMode = 1;
	}
	else
	{
		int tmp1,tmp2,tmp4;
		char tmp3;
		int count = sscanf(buf1,"%d-%d%c%d",&tmp1,&tmp2,&tmp3,&tmp4);
		if(count != 4)
		{
			g_gateway_info.rs232_param.rate = 115200;
			g_gateway_info.rs232_param.bitWidth = 8;
			g_gateway_info.rs232_param.parity = PARITY_NONE;
			g_gateway_info.rs232_param.stopBitsMode = 1;
		}
		else
		{
			g_gateway_info.rs232_param.rate = tmp1;
			g_gateway_info.rs232_param.bitWidth = tmp2;
			g_gateway_info.rs232_param.stopBitsMode = tmp4;

			if(tmp3 == 'o')
				g_gateway_info.rs232_param.parity = PARITY_ODD;
			else if(tmp3 == 'e')
				g_gateway_info.rs232_param.parity = PARITY_EVEN;
			else
				g_gateway_info.rs232_param.parity = PARITY_NONE;

		}
		//printf("g_gateway_info.rs232_param.stopBitsMode\n",g_gateway_info.rs232_param.stopBitsMode);

	}


	if((g_gateway_info.rs232_param.bitWidth<5)||(g_gateway_info.rs232_param.bitWidth>8))
	{
		g_gateway_info.rs232_param.bitWidth = 8;
	}

	if((g_gateway_info.rs232_param.rate != 9600)
	&&(g_gateway_info.rs232_param.rate != 19200)
	&&(g_gateway_info.rs232_param.rate != 38400)
	&&(g_gateway_info.rs232_param.rate != 57600)
	&&(g_gateway_info.rs232_param.rate != 115200))
	{
		g_gateway_info.rs232_param.rate = 115200;
	}

	int tmp = (int)(g_gateway_info.rs232_param.stopBitsMode*10);
	if((g_gateway_info.rs232_param.stopBitsMode != 1)
		&&(g_gateway_info.rs232_param.stopBitsMode != 2))
	{
		g_gateway_info.rs232_param.stopBitsMode = 1;
	}
	return 0;
}

int Cfg_InitParam_GW_fromJson()
{
	char cmd1[64] = "";

	if(g_gateway_info.rs232_mode == ON)
	{
		system("astparam s soip_guest_on y");
		system("astparam s soip_type 2");

		sprintf(cmd1,"astparam s soip_port %d",g_gateway_info.rs232_port);
		system(cmd1);
	}

	else
	{
		system("astparam s soip_guest_on n");
		system("astparam s soip_type 2");

		sprintf(cmd1,"astparam s soip_port %d",g_gateway_info.rs232_port);
		system(cmd1);
	}

	memset(cmd1,0,64);
	if(g_gateway_info.rs232_param.parity == PARITY_ODD)
		sprintf(cmd1,"astparam s s0_baudrate %d-%do%d",g_gateway_info.rs232_param.rate,g_gateway_info.rs232_param.bitWidth,g_gateway_info.rs232_param.stopBitsMode);
	else if(g_gateway_info.rs232_param.parity == PARITY_EVEN)
		sprintf(cmd1,"astparam s s0_baudrate %d-%de%d",g_gateway_info.rs232_param.rate,g_gateway_info.rs232_param.bitWidth,g_gateway_info.rs232_param.stopBitsMode);
	else
		sprintf(cmd1,"astparam s s0_baudrate %d-%dn%d",g_gateway_info.rs232_param.rate,g_gateway_info.rs232_param.bitWidth,g_gateway_info.rs232_param.stopBitsMode);

	system(cmd1);

	//printf("Cfg_InitParam_GW_fromJson %d %s\n",g_gateway_info.rs232_param.stopBitsMode,cmd1);

	return 0;
}

int Cfg_Init_Gateway(void)
{
	DBG_InfoMsg("Cfg_Init_Gateway\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,GATEWAY_FILE);

	g_gateway_info.cec_mode = ON;
#ifdef CONFIG_P3K_HOST
	g_gateway_info.cec_output = 1;
#else
	g_gateway_info.cec_output = 2;
#endif
	g_gateway_info.rs232_mode = ON;
	g_gateway_info.rs232_port = 5001;
	g_gateway_info.rs232_param.rate = 115200;
	g_gateway_info.rs232_param.bitWidth = 8;
	g_gateway_info.rs232_param.parity = PARITY_NONE;
	g_gateway_info.rs232_param.stopBitsMode = 1;
	g_gateway_info.ir_mode = ON;
	g_gateway_info.ir_direction = DIRECTION_OUT;

	//Check gateway cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		Cfg_Init_GW_fromParam();
		//create autoswitch cfg from default value
		Cfg_Update(GATEWAY_INFO);
		return 0;
	}

	//Read  gateway cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_GW].empty())
		{
			Json::Value& root = root1[JSON_GW];

			//init g_gateway_info
			if(!root[JSON_GW_CEC_MODE].empty())
			{
				string mode = root[JSON_GW_CEC_MODE].asString();

				if(mode == JSON_PARAM_OFF)
					g_gateway_info.cec_mode = OFF;
				else
					g_gateway_info.cec_mode = ON;
			}

			if(!root[JSON_GW_CEC_DEST].empty())
			{
				string mode = root[JSON_GW_CEC_DEST].asString();

				if(mode == JSON_GW_CEC_DEST_PASS)
					g_gateway_info.cec_output = 0;
				else if(mode == JSON_GW_CEC_DEST_IN)
					g_gateway_info.cec_output = 1;
				else if(mode == JSON_GW_CEC_DEST_OUT)
					g_gateway_info.cec_output = 2;
				else if(mode == JSON_GW_CEC_DEST_LOOP)
					g_gateway_info.cec_output = 3;
			}

			if(!root[JSON_GW_IR_DIR].empty())
			{
				string mode = root[JSON_GW_IR_DIR].asString();

				if(mode == JSON_PARAM_IN)
					g_gateway_info.ir_direction = DIRECTION_IN;
				else
					g_gateway_info.ir_direction = DIRECTION_OUT;
			}

			if(!root[JSON_GW_IR_MODE].empty())
			{
				string mode = root[JSON_GW_IR_MODE].asString();

				if(mode == JSON_PARAM_ON)
					g_gateway_info.ir_mode = ON;
				else
					g_gateway_info.ir_mode = OFF;
			}

			if(!root[JSON_GW_UART_MODE].empty())
			{
				string mode = root[JSON_GW_UART_MODE].asString();

				if(mode == JSON_PARAM_OFF)
					g_gateway_info.rs232_mode = OFF;
				else
					g_gateway_info.rs232_mode = ON;
			}

			if(!root[JSON_GW_UART_PORT].empty())
			{
				if(root[JSON_GW_UART_PORT].isInt())
					g_gateway_info.rs232_port = root[JSON_GW_UART_PORT].asInt();
			}

			if(!root[JSON_GW_UART_PARAM].empty())
			{
				Json::Value& JsonParam = root[JSON_GW_UART_PARAM];

				if(!JsonParam[JSON_GW_UART_RATE].empty())
				{
					if(JsonParam[JSON_GW_UART_RATE].isInt())
					{
						g_gateway_info.rs232_param.rate = JsonParam[JSON_GW_UART_RATE].asInt();
						if((g_gateway_info.rs232_param.rate != 9600)
							&&(g_gateway_info.rs232_param.rate != 19200)
							&&(g_gateway_info.rs232_param.rate != 38400)
							&&(g_gateway_info.rs232_param.rate != 57600)
							&&(g_gateway_info.rs232_param.rate != 115200))
						{
							g_gateway_info.rs232_param.rate = 115200;
						}
					}
				}

				if(!JsonParam[JSON_GW_UART_BITS].empty())
				{
					if(JsonParam[JSON_GW_UART_BITS].isInt())
					{
						g_gateway_info.rs232_param.bitWidth = JsonParam[JSON_GW_UART_BITS].asInt();
						if((g_gateway_info.rs232_param.bitWidth<5)||(g_gateway_info.rs232_param.bitWidth>8))
						{
							g_gateway_info.rs232_param.bitWidth = 8;
						}
					}
				}


				if(!JsonParam[JSON_GW_UART_PARITY].empty())
				{
					string mode = JsonParam[JSON_GW_UART_PARITY].asString();

					if(mode == JSON_GW_UART_PARITY_NONE)
						g_gateway_info.rs232_param.parity= PARITY_NONE;
					else if(mode == JSON_GW_UART_PARITY_ODD)
						g_gateway_info.rs232_param.parity= PARITY_ODD;
					else if(mode == JSON_GW_UART_PARITY_EVEN)
						g_gateway_info.rs232_param.parity= PARITY_EVEN;
					else if(mode == JSON_GW_UART_PARITY_MARK)
						g_gateway_info.rs232_param.parity= PARITY_MARK;
					else if(mode == JSON_GW_UART_PARITY_SPACE)
						g_gateway_info.rs232_param.parity= PARITY_SPACE;
					else
						g_gateway_info.rs232_param.parity= PARITY_NONE;
				}

				if(!JsonParam[JSON_GW_UART_STOP].empty())
				{
					if(JsonParam[JSON_GW_UART_STOP].isInt())
					{
						g_gateway_info.rs232_param.stopBitsMode = JsonParam[JSON_GW_UART_STOP].asInt();

						if((g_gateway_info.rs232_param.stopBitsMode != 1)
							&&(g_gateway_info.rs232_param.stopBitsMode != 2))
						{
							g_gateway_info.rs232_param.stopBitsMode = 1;
						}
					}
				}

			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	if(g_bCfg == 1)
	{
		Cfg_InitParam_GW_fromJson();
		return 0;
	}

	return 0;
}

int Cfg_Init_NW_fromParam()
{
	char* cmd1 = "astparam g soip_guest_on";
	char buf1[64] = "";

	memset(buf1,0,64);
	mysystem("astparam g ip_mode",buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		g_network_info.eth_info[0].dhcp_enable = 1;
	}
	else if(strstr(buf1,"static") != 0)
	{
		g_network_info.eth_info[0].dhcp_enable = 0;

		memset(buf1,0,64);
		mysystem("astparam g ipaddr",buf1,64);

		memset(g_network_info.eth_info[0].ipAddr,0,MAX_IP_ADDR_LEN);
		if(strstr(buf1,"not defined") != 0)
		{
#ifdef CONFIG_P3K_HOST
			sprintf(g_network_info.eth_info[0].ipAddr,"169.254.0.222");
#else
			sprintf(g_network_info.eth_info[0].ipAddr,"169.254.0.111");
#endif
		}
		else
		{
			sprintf(g_network_info.eth_info[0].ipAddr,buf1);
		}


		memset(buf1,0,64);
		mysystem("astparam g netmask",buf1,64);

		memset(g_network_info.eth_info[0].mask,0,MAX_IP_ADDR_LEN);
		if(strstr(buf1,"not defined") != 0)
		{
			sprintf(g_network_info.eth_info[0].mask,"255.255.0.0");
		}
		else
		{
			sprintf(g_network_info.eth_info[0].mask,buf1);
		}

		memset(buf1,0,64);
		mysystem("astparam g gatewayip",buf1,64);

		memset(g_network_info.eth_info[0].gateway,0,MAX_IP_ADDR_LEN);
		if(strstr(buf1,"not defined") != 0)
		{
			sprintf(g_network_info.eth_info[0].gateway,"169.254.0.254");
		}
		else
		{
			sprintf(g_network_info.eth_info[0].gateway,buf1);
		}
	}
	else
	{
		g_network_info.eth_info[0].dhcp_enable = 1;
	}

	memset(buf1,0,64);
	mysystem("astparam g multicast_on",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		g_network_info.method = Net_MULTICAST;
	else if(strcmp(buf1,"n") == 0)
		g_network_info.method = Net_UNICAST;
	else
		g_network_info.method = Net_MULTICAST;

	return 0;
}

int Cfg_InitParam_NW_fromJson()
{
	char cmd1[64] = "";

	if(g_network_info.eth_info[0].dhcp_enable == 1)
	{
		system("astparam s ip_mode dhcp");
	}
	else
	{
		system("astparam s ip_mode static");

		sprintf(cmd1,"astparam s ipaddr %s",g_network_info.eth_info[0].ipAddr);
		system(cmd1);
		sprintf(cmd1,"astparam s netmask %s",g_network_info.eth_info[0].mask);
		system(cmd1);
		sprintf(cmd1,"astparam s gatewayip %s",g_network_info.eth_info[0].gateway);
		system(cmd1);
	}

	if(g_network_info.method == Net_MULTICAST)
		system("astparam s multicast_on y");
	else
		system("astparam s multicast_on n");

	return 0;
}

int Cfg_Init_Network(void)
{
	DBG_InfoMsg("Cfg_Init_Network\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,NETWORK_FILE);

	g_network_info.eth_info[0].dhcp_enable = 1;
	g_network_info.eth_info[1].dhcp_enable = 1;
	g_network_info.daisy_chain = OFF;
	g_network_info.tcp_port = 5000;
	g_network_info.udp_port = 50000;
	g_network_info.method = Net_MULTICAST;
	sprintf(g_network_info.multicast_ip,"0.0.0.0");
	g_network_info.multicast_ttl = 64;
	g_network_info.control_port = 0;
	g_network_info.control_vlan = 1;
	g_network_info.dante_port = 0;
	g_network_info.dante_vlan = 1;
	g_network_info.beacon_en = OFF;
    g_network_info.beacon_time = 10;
	sprintf(g_network_info.beacon_ip,"224.0.0.250");
	g_network_info.beacon_port = 50000;

	//Check gateway cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		Cfg_Init_NW_fromParam();
		//create autoswitch cfg from default value
		Cfg_Update(NETWORK_INFO);
		return 0;
	}

	//Read  gateway cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[2048] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_NETWORK].empty())
		{
			Json::Value& root = root1[JSON_NETWORK];

			for(int i = 0; i < 2; i++)
			{
				Json::Value eth;
				if((i == 0)&&(!root[JSON_NETWORK_STREAM].empty()))
					eth = root[JSON_NETWORK_STREAM];
				else if((i == 1)&&(!root[JSON_NETWORK_CONTROL].empty()))
					eth = root[JSON_NETWORK_CONTROL];
				else
					continue;

				if(!eth[JSON_NETWORK_MODE].empty())
				{
					string mode = eth[JSON_NETWORK_MODE].asString();

					if(mode == JSON_NETWORK_STATIC)
					{
						g_network_info.eth_info[i].dhcp_enable = 0;

						if(!eth[JSON_NETWORK_IP].empty())
						{
							string addr = eth[JSON_NETWORK_IP].asString();
							memset(g_network_info.eth_info[i].ipAddr,0,MAX_IP_ADDR_LEN);
							sprintf(g_network_info.eth_info[i].ipAddr,addr.c_str());
						}

						if(!eth[JSON_NETWORK_MASK].empty())
						{
							string addr = eth[JSON_NETWORK_MASK].asString();
							memset(g_network_info.eth_info[i].mask,0,MAX_IP_ADDR_LEN);
							sprintf(g_network_info.eth_info[i].mask,addr.c_str());
						}

						if(!eth[JSON_NETWORK_GW].empty())
						{
							string addr = eth[JSON_NETWORK_GW].asString();
							memset(g_network_info.eth_info[i].gateway,0,MAX_IP_ADDR_LEN);
							sprintf(g_network_info.eth_info[i].gateway,addr.c_str());
						}

					}
					else
					{
						g_network_info.eth_info[i].dhcp_enable = 1;
					}
				}
			}

			if(!root[JSON_NETWORK_DAISY].empty())
			{
				string mode = root[JSON_NETWORK_DAISY].asString();

				if(mode == JSON_PARAM_OFF)
					g_network_info.daisy_chain= OFF;
				else
					g_network_info.daisy_chain= ON;
			}

			if(!root[JSON_NETWORK_TCP].empty())
				if(root[JSON_NETWORK_TCP].isInt())
					g_network_info.tcp_port= root[JSON_NETWORK_TCP].asInt();

			if(!root[JSON_NETWORK_UDP].empty())
				if(root[JSON_NETWORK_UDP].isInt())
					g_network_info.udp_port= root[JSON_NETWORK_UDP].asInt();

			if(!root[JSON_NETWORK_METHOD].empty())
			{
				string mode = root[JSON_NETWORK_METHOD].asString();

				if(mode == JSON_NETWORK_UNI)
					g_network_info.method = Net_UNICAST;
				else
				{
					g_network_info.method = Net_MULTICAST;

					if(!root[JSON_NETWORK_MULTICAST].empty())
					{
						Json::Value& multi = root[JSON_NETWORK_MULTICAST];

						if(!multi[JSON_NETWORK_GROUP_IP].empty())
						{
							string addr = multi[JSON_NETWORK_GROUP_IP].asString();
							memset(g_network_info.multicast_ip,0,32);
							sprintf(g_network_info.multicast_ip,addr.c_str());
						}

						if(!multi[JSON_NETWORK_TTL].empty())
							if(multi[JSON_NETWORK_TTL].isInt())
								g_network_info.multicast_ttl = multi[JSON_NETWORK_TTL].asInt();
					}
				}
			}

			if(!root[JSON_NETWORK_PORT_SET].empty())
			{
				Json::Value& port = root[JSON_NETWORK_PORT_SET];

				if(!port[JSON_NETWORK_CONTROL].empty())
				{
					Json::Value& control_port = port[JSON_NETWORK_CONTROL];

					if(!control_port[JSON_NETWORK_PORT].empty())
					{
						string port = control_port[JSON_NETWORK_PORT].asString();
						if(port == JSON_NETWORK_ETH0)
							g_network_info.control_port = 0;
						else
							g_network_info.control_port = 1;
					}

					if(!control_port[JSON_NETWORK_VLAN].empty())
						if(control_port[JSON_NETWORK_VLAN].isInt())
							g_network_info.control_vlan = control_port[JSON_NETWORK_VLAN].asInt();

					g_init_control_port = g_network_info.control_port;
	 				g_init_control_vlan = g_network_info.control_vlan;
				}

				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					if(!port[JSON_NETWORK_DANTE].empty())
					{
						Json::Value& dante_port = port[JSON_NETWORK_DANTE];

						if(!dante_port[JSON_NETWORK_PORT].empty())
						{
							string port = dante_port[JSON_NETWORK_PORT].asString();
							if(port == JSON_NETWORK_ETH0)
								g_network_info.dante_port = 0;
							else
								g_network_info.dante_port = 1;
						}

						if(!dante_port[JSON_NETWORK_VLAN].empty())
							if(dante_port[JSON_NETWORK_VLAN].isInt())
								g_network_info.dante_vlan = dante_port[JSON_NETWORK_VLAN].asInt();

					}
				}
			}

			if(!root[JSON_NETWORK_BEACON_INFO].empty())
			{
				Json::Value& beacon_info = root[JSON_NETWORK_BEACON_INFO];

				if(!beacon_info[JSON_NETWORK_BEACON_EN].empty())
				{
					string mode =  beacon_info[JSON_NETWORK_BEACON_EN].asString();

					if(mode == JSON_PARAM_ON)
						g_network_info.beacon_en = ON;
					else
						g_network_info.beacon_en = OFF;
				}

				if(!beacon_info[JSON_NETWORK_BEACON_IP].empty())
				{
					string addr = beacon_info[JSON_NETWORK_BEACON_IP].asString();
					memset(g_network_info.beacon_ip,0,MAX_IP_ADDR_LEN);
					sprintf(g_network_info.beacon_ip,addr.c_str());
				}

				if(!beacon_info[JSON_NETWORK_BEACON_PORT].empty())
				{
					if(beacon_info[JSON_NETWORK_BEACON_PORT].isInt())
						g_network_info.beacon_port = beacon_info[JSON_NETWORK_BEACON_PORT].asInt();
				}
                if(!beacon_info[JSON_NETWORK_BEACON_TIME].empty())
				{
					if(beacon_info[JSON_NETWORK_BEACON_TIME].isInt())
					{
						int time = beacon_info[JSON_NETWORK_BEACON_TIME].asInt();
	                    if(1 <= time && time <= 1800)
	                    {
	                        g_network_info.beacon_time = time;
	                    }
	                    else
	                    {
	                        g_network_info.beacon_time = 10;
	                    }
					}
				}
			}

		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	if(g_bCfg == 1)
	{
		Cfg_InitParam_NW_fromJson();
		return 0;
	}
	return 0;
}

int Cfg_Init_Log(void)
{
	DBG_InfoMsg("Cfg_Init_Log\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,LOG_SETTING_FILE);

	g_log_info.active = OFF;
	g_log_info.period = 3;

	//Check log cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("Cfg_Check_File %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(LOG_INFO);
		return 0;
	}

	if(g_bCfg == 1)
	{
		return 0;
	}

	//Read  Log cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_LOG_SETTING].empty())
		{
			Json::Value& root = root1[JSON_LOG_SETTING];

			//init g_channel_info
			if(!root[JSON_LOG_ACTIVE].empty())
			{
				string mode = root[JSON_LOG_ACTIVE].asString();
				if(mode == JSON_PARAM_ON)
					g_log_info.active = ON;
				else
					g_log_info.active = OFF;
			}

			if(!root[JSON_LOG_PERIOD].empty())
			{
				string period = root[JSON_LOG_PERIOD].asString();

				if(period == JSON_LOG_DAILY)
					g_log_info.period = 2;
				else
					g_log_info.period = 3;
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

int Cfg_Init_OSD(void)
{
	DBG_InfoMsg("Cfg_Init_OSD\n");
#ifdef CONFIG_P3K_HOST
		DBG_InfoMsg("This is Encoder\n");
		return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_FILE);

	g_osd_enable = OFF;

	//Read  Log cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_OSD_DEVICE_INFO].empty())
		{
			Json::Value& root = root1[JSON_OSD_DEVICE_INFO];

			//init g_channel_info
			if(!root[JSON_OSD_DEVICE_INFO_EN].empty())
			{
				string mode = root[JSON_OSD_DEVICE_INFO_EN].asString();
				if(mode == JSON_PARAM_ON)
					g_osd_enable = ON;
				else
					g_osd_enable = OFF;
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;}

int Cfg_Create_DefaultFile(void)
{
	Cfg_Create_AutoswitchDelay();
	Cfg_Create_AVSignal();
	Cfg_Create_DisplaySleep();
	Cfg_Create_OsdSetting();
	Cfg_Create_OverlaySetting();
	Cfg_Create_SecuritySetting();
	Cfg_Create_KVMSetting();
	Cfg_Create_Channel();
	Cfg_Create_EDIDList();
	return 0;
}

// /switch/auto_switch_delays.json
int Cfg_Create_AutoswitchDelay(void)
{
	DBG_InfoMsg("Cfg_Create_AutoswitchDelay\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_DELAY_FILE);

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);

		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_AUTOSWITCH_LOSS] = 10;
	root[JSON_AUTOSWITCH_DETECT] = 0;
	root[JSON_AUTOSWITCH_UNPLUG] = 0;
	root[JSON_AUTOSWITCH_OFF] = 900;
	root[JSON_AUTOSWITCH_PLUGIN] = 0;
	root[JSON_AUTOSWITCH_OVERRIDE] = 10;


	Json::Value root1;
	root1[JSON_AUTOSWITCH_DELAY] = root;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_DELAY_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAutoSwitchSetting = fast_writer.write(root1);
	fwrite(strAutoSwitchSetting.c_str(),1,strAutoSwitchSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Init_AV_FromParam(void)
{
	return 0;
}

int Cfg_InitParam_AV_FromJson(void)
{
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);


	//Read	avsignal cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SIGNAL].empty())
		{
			Json::Value& root = root1[JSON_AV_SIGNAL];

#ifdef CONFIG_P3K_HOST
			if(!root[JSON_AV_MAX_BITRATE].empty())
			{
				char cmd[256] = "";
				string bitrate =  root[JSON_AV_MAX_BITRATE].asString();

				if(bitrate == JSON_AV_BEST_EFFORT)
				{
					sprintf(cmd,"astparam s profile auto");
					system(cmd);
				}
				else if((bitrate == "10")||(bitrate == "50")||(bitrate == "100")||(bitrate == "150")||(bitrate == "200"))
				{
					sprintf(cmd,"astparam s profile %sM",bitrate.c_str());
					system(cmd);
				}
				else
				{
					DBG_WarnMsg("JSON_AV_MAX_BITRATE Param: %s	Error!!!",bitrate.c_str());
				}

				//printf("%s\n",cmd);
			}

			if(!root[JSON_AV_FRAME_RATE].empty())
			{
				char cmd[256] = "";

				if(root[JSON_AV_FRAME_RATE].isInt())
				{
					int frame = root[JSON_AV_FRAME_RATE].asInt();

					if((frame <= 100)&&(frame >= 0))
					{
						int param = frame * 60 / 100;
						sprintf(cmd,"astparam s v_frame_rate %d",param);
						system(cmd);
					}
					else
					{
						DBG_WarnMsg("JSON_AV_FRAME_RATE Param: %d  Error!!!",frame);
					}

					//printf("%s\n",cmd);
				}
			}
#else
			if(!root[JSON_AV_RGB].empty())
			{
				char cmd[256] = "";
				string forcergb = root[JSON_AV_RGB].asString();

				if(forcergb == JSON_PARAM_ON)
				{
					sprintf(cmd,"astparam s v_hdmi_force_rgb_output 5");
					system(cmd);
				}
				else
				{
					sprintf(cmd,"astparam s v_hdmi_force_rgb_output 0");
					system(cmd);
				}

				//printf("%s\n",cmd);
			}

			if(!root[JSON_AV_OUT_RES].empty())
			{
				string res = root[JSON_AV_OUT_RES].asString();

				if(res == "2160p30")
					system("astparam s v_output_timing_convert 8000005F");
				else if(res == "2160p25")
					system("astparam s v_output_timing_convert 8000005E");
				else if(res == "1080p60")
					system("astparam s v_output_timing_convert 80000010");
				else if(res == "1080p50")
					system("astparam s v_output_timing_convert 8000001F");
				else if(res == "720p60")
					system("astparam s v_output_timing_convert 80000004");
				else
					system("astparam s v_output_timing_convert 0");
			}
#endif
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	return 0;
}

// /av_signal/av_signal.json
int Cfg_Create_AVSignal(void)
{
	DBG_InfoMsg("Cfg_Create_AVSignal\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		//printf("nAccessRet %s Suceess\n",path);
		Cfg_InitParam_AV_FromJson();
		return 0;
	}

	Json::Value root;

	root[JSON_AV_IN_RES] = JSON_AV_PASSTHRU;
	root[JSON_AV_MAX_BITRATE] = JSON_AV_BEST_EFFORT;
	root[JSON_AV_FRAME_RATE] = 100;
	root[JSON_AV_COLOR_DEPTH] = JSON_AV_BYPASS;
	root[JSON_AV_RGB] = JSON_PARAM_OFF;
	root[JSON_AV_AUD_GUARD] = 10;
	root[JSON_AV_OUT_RES] = JSON_AV_PASSTHRU;

	char buf1[64] = "";

	memset(buf1,0,64);

#ifdef CONFIG_P3K_HOST
	mysystem("astparam g profile",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		root[JSON_AV_MAX_BITRATE] = JSON_AV_BEST_EFFORT;
	else if(strstr(buf1,"auto") != 0)
		root[JSON_AV_MAX_BITRATE] = JSON_AV_BEST_EFFORT;
	else if(strstr(buf1,"10M") != 0)
		root[JSON_AV_MAX_BITRATE] = "10";
	else if(strstr(buf1,"50M") != 0)
		root[JSON_AV_MAX_BITRATE] = "50";
	else if(strstr(buf1,"100M") != 0)
		root[JSON_AV_MAX_BITRATE] = "100";
	else if(strstr(buf1,"150M") != 0)
		root[JSON_AV_MAX_BITRATE] = "150";
	else if(strstr(buf1,"200M") != 0)
		root[JSON_AV_MAX_BITRATE] = "200";
	else
		root[JSON_AV_MAX_BITRATE] = JSON_AV_BEST_EFFORT;

	memset(buf1,0,64);
	mysystem("astparam g v_frame_rate",buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		root[JSON_AV_FRAME_RATE] = 100;
	}
	else
	{
		int frame = atoi(buf1);
		if((frame >=0) &&(frame <=60))
		{
			int param = frame * 100 / 60;
			root[JSON_AV_FRAME_RATE] = param;
		}
		else
			root[JSON_AV_FRAME_RATE] = 100;
	}

#else
	mysystem("astparam g v_hdmi_force_rgb_output",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		root[JSON_AV_RGB] = JSON_PARAM_ON;
	else
	{
		int mode = atoi(buf1);
		if(mode == 0)
			root[JSON_AV_RGB] = JSON_PARAM_OFF;
		else
			root[JSON_AV_RGB] = JSON_PARAM_ON;
	}

	mysystem("astparam g v_output_timing_convert",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		root[JSON_AV_OUT_RES] = JSON_AV_PASSTHRU;
	else if(strstr(buf1,"8000005F") != 0)
		root[JSON_AV_OUT_RES] = "2160p30";
	else if(strstr(buf1,"8000005E") != 0)
		root[JSON_AV_OUT_RES] = "2160p25";
	else if(strstr(buf1,"80000010") != 0)
		root[JSON_AV_OUT_RES] = "1080p60";
	else if(strstr(buf1,"8000001F") != 0)
		root[JSON_AV_OUT_RES] = "1080p50";
	else if(strstr(buf1,"80000004") != 0)
		root[JSON_AV_OUT_RES] = "720p60";
	else
		root[JSON_AV_OUT_RES] = JSON_AV_PASSTHRU;

#endif


	Json::Value root1;
	root1[JSON_AV_SIGNAL] = root;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAVSignal = fast_writer.write(root1);

	fwrite(strAVSignal.c_str(),1,strAVSignal.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

// /display/display_sleep.json
int Cfg_Create_DisplaySleep(void)
{
	DBG_InfoMsg("Cfg_Create_DisplaySleep\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DISPLAY_SLEEP_FILE);

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_DISPLAY_SLEEP] = 0;
	root[JSON_DISPLAY_SHUTDOWN] = 0;
	root[JSON_DISPLAY_WAKEUP] = 0;

	Json::Value root1;
	root1[JSON_DISPLAY_DELAY] = root;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DISPLAY_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DISPLAY_SLEEP_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strDispDelay = fast_writer.write(root1);

	fwrite(strDispDelay.c_str(),1,strDispDelay.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	return 0;
}

// /osd/osd.json
int Cfg_Create_OsdSetting(void)
{
	DBG_InfoMsg("Cfg_Create_OsdSetting\n");

#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_FILE);

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_OSD_TIMEOUT] = 30;
	root[JSON_OSD_POS] = JSON_OSD_TOP_LEFT;
	root[JSON_OSD_FONE_SIZE] = JSON_OSD_MEDIUM;
	root[JSON_OSD_MAX_PER_PAGE] = 5;
	root[JSON_OSD_MAX_CHANNEL] = 999;

	Json::Value root_dev;
	root_dev[JSON_OSD_DEVICE_INFO_EN] = JSON_PARAM_OFF;
	root_dev[JSON_OSD_DEVICE_INFO_TIME] = 2;

	Json::Value root1;
	root1[JSON_OSD_CHANNEL] = root;
	root1[JSON_OSD_DEVICE_INFO] = root_dev;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strOSD = fast_writer.write(root1);

	fwrite(strOSD.c_str(),1,strOSD.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

// /edid/edid_list.json
int Cfg_Create_OverlaySetting(void)
{
	DBG_InfoMsg("Cfg_Create_OverlaySetting\n");

#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OVERLAY_PATH);

	//Check overlay path
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("nAccessRet %s Failed\n",path);
		system("cp -rf /share/overlay /data/configs/kds-7/");
		return -1;
	}

	/*int nAccessRet = Cfg_Check_File(path);

	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}
	*/



	return 0;
}


// /secure/security_setting.json
int Cfg_Create_SecuritySetting(void)
{
	DBG_InfoMsg("Cfg_Create_SecuritySetting\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_HTTPS_FILE);

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 != nAccessRet)
	{
		Json::Value root;

		root[JSON_HTTPS_MODE] = JSON_PARAM_ON;
		root[JSON_HTTPS_CERTS] = "kramer_tls.pem";
		root[JSON_HTTPS_KEY] = "123456";
		root[JSON_HTTPS_METHOD] = "in";

		Json::Value root1;
		root1[JSON_HTTPS_SETTING] = root;

		memset(path,0,128);
		sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_PATH);
		int s32AccessRet = access(path, F_OK);
		if(s32AccessRet != 0)
		{
			char cmd[256] = "";
			sprintf(cmd,"mkdir -p %s",path);

			system(cmd);
		}

		memset(path,0,128);
		sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_HTTPS_FILE);
		FILE *fp;
		fp = fopen(path, "w");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open %s\n",path);
			return -1;
		}

		if (flock(fileno(fp), LOCK_EX)) {
			DBG_ErrMsg("%s lock failed\n",path);
			fclose(fp);
			return -1;
		}

		Json::FastWriter fast_writer;
		string str8021X = fast_writer.write(root1);

		fwrite(str8021X.c_str(),1,str8021X.size(),fp);

		fflush(fp);
		fsync(fileno(fp));
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
	}

	sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_8021X_FILE);

	//Check Video cfg

	nAccessRet = Cfg_Check_File(path);
	if(0 != nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Failed\n",path);

		//printf("nAccessRet %s Suceess\n",path);
		Json::Value root;

		root[JSON_8021X_MODE] = JSON_PARAM_OFF;
		root[JSON_SECUR_DEFAULT_AUTH] = JSON_SECUR_EAP_TLS;

		Json::Value JsonTls;
		JsonTls[JSON_TLS_USER]= "";
		JsonTls[JSON_TLS_CA]= "";
		JsonTls[JSON_TLS_CLIENT]= "";
		JsonTls[JSON_TLS_KEY]= "";
		JsonTls[JSON_TLS_PASS]= "";

		root[JSON_TLS_SETTING] = JsonTls;

		Json::Value JsonMschap;
		JsonMschap[JSON_MSCHAP_USER]= "";
		JsonMschap[JSON_MSCHAP_PASS]= "";


		root[JSON_MSCHAP_SETTING] = JsonMschap;

		Json::Value root1;
		root1[JSON_8021X_SETTING] = root;

		memset(path,0,128);
		sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_8021X_FILE);
		FILE *fp;
		fp = fopen(path, "w");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open %s\n",path);
			return -1;
		}

		if (flock(fileno(fp), LOCK_EX)) {
			DBG_ErrMsg("%s lock failed\n",path);
			fclose(fp);
			return -1;
		}

		Json::FastWriter fast_writer;
		string str8021X = fast_writer.write(root1);

		fwrite(str8021X.c_str(),1,str8021X.size(),fp);

		fflush(fp);
		fsync(fileno(fp));
		flock(fileno(fp), LOCK_UN);
		fclose(fp);
	}

	return 0;
}

int Cfg_Init_KVM_FromParam(void)
{
	return 0;
}

int Cfg_InitParam_KVM_FromJson(void)
{
	DBG_InfoMsg("Cfg_InitParam_KVM_FromJson\n");

#ifdef CONFIG_P3K_HOST
	DBG_WarnMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,KVM_FILE);

	//Read	kvm cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[2048] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	int nRet = 0,nOffset = 0;
	while(1)
	{
		//printf("nOffset == %d sizeof(pBuf) = %d\n",nOffset,sizeof(pBuf));
		nRet = fread(pBuf+nOffset,1,sizeof(pBuf)-nOffset,fp);
		//printf("nRet == %d\n",nRet);
		if(nRet <= 0)
		{
			break;
		}
		else
		{
			nOffset += nRet;

			if(nOffset >= sizeof(pBuf))
				break;
		}
	}

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_USB_KVM_CONFIG].empty())
		{
			Json::Value& root = root1[JSON_USB_KVM_CONFIG];

			if(!root[JSON_USB_KVM_MODE].empty())
			{
				char cmd[256] = "";
				string mode =  root[JSON_USB_KVM_MODE].asString();

				if(mode == JSON_USB_KVM_KM)
				{
					sprintf(cmd,"astparam s no_kmoip n");
					system(cmd);
				}
				else if(mode == JSON_USB_KVM_USB)
				{
					sprintf(cmd,"astparam s no_kmoip y");
					system(cmd);
				}

				DBG_InfoMsg("%s\n",cmd);
			}

			if(!root[JSON_USB_KVM_ROAMING].empty())
			{
				Json::Value& JsonKVMArray = root[JSON_USB_KVM_ROAMING];

				char cmd1[512] = "";
				char param[512] = "";

				for(unsigned int i = 0; i < JsonKVMArray.size(); i++)
				{
					Json::Value& JsonKVM = JsonKVMArray[i];

					string str_mac,str_x,str_y;
					if(!JsonKVM[JSON_USB_KVM_MAC].empty())
					{
						string str_mac = JsonKVM[JSON_USB_KVM_MAC].asString();
						if(str_mac.size() == 0)
						{
							printf("str_mac = no string \n");
							continue;
						}
					}

					if(!JsonKVM[JSON_USB_KVM_H].empty())
					{
						string str_x = JsonKVM[JSON_USB_KVM_H].asString();
						if(str_x.size() == 0)
						{
							printf("str_x = no string \n");
							continue;
						}
					}

					if(!JsonKVM[JSON_USB_KVM_V].empty())
					{
						string str_y = JsonKVM[JSON_USB_KVM_V].asString();
						if(str_y.size() == 0)
						{
							printf("str_y = no string \n");
							continue;
						}
					}

					if((!JsonKVM[JSON_USB_KVM_MAC].empty())&&(!JsonKVM[JSON_USB_KVM_H].empty())&&(!JsonKVM[JSON_USB_KVM_V].empty()))
					{
						string mac = JsonKVM[JSON_USB_KVM_MAC].asString();

						if((JsonKVM[JSON_USB_KVM_H].isInt())&&(JsonKVM[JSON_USB_KVM_V].isInt()))
						{
							int    x   = JsonKVM[JSON_USB_KVM_H].asInt();
							int    y   = JsonKVM[JSON_USB_KVM_V].asInt();

							printf("JsonKVMArray[%d]:[mac: %s][x: %d][y: %d]\n",i,mac.c_str(),x,y);

							if(mac.size()==12)
							{
								if(strlen(param) == 0)
									sprintf(param,"%s,%d,%d",mac.c_str(),x,y);
								else
									sprintf(param,"%s:%s,%d,%d",param,mac.c_str(),x,y);
							}
						}
					}


				}

				if(strlen(param) > 0)
				{
					sprintf(cmd1,"astparam s kmoip_roaming_layout %s",param);
					system(cmd1);

				}

				DBG_InfoMsg("%s\n",cmd1);
			}

			if(!root[JSON_USB_KVM_TIMEOUT].empty())
			{
				char cmd2[256] = "";

				if(root[JSON_USB_KVM_TIMEOUT].isInt())
				{
					int interval =  root[JSON_USB_KVM_TIMEOUT].asInt();

					if((interval <= 10)&&(interval >= 0))
					{
						sprintf(cmd2,"astparam s kmoip_token_interval %d",interval*1000);
						system(cmd2);
					}

					DBG_InfoMsg("%s\n",cmd2);
				}
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

// /usb/km_usb.json
int Cfg_Create_KVMSetting(void)
{
	DBG_InfoMsg("Cfg_Create_KVMSetting\n");

#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,KVM_FILE);

	//Check Video cfg

	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		Cfg_InitParam_KVM_FromJson();
		return 0;
	}

	Json::Value root;

	root[JSON_USB_KVM_MODE] = JSON_USB_KVM_KM;
	root[JSON_USB_KVM_ACTIVE] = JSON_PARAM_ON;
	root[JSON_USB_KVM_TIMEOUT] = 10;
	root[JSON_USB_KVM_ROW] = 1;
	root[JSON_USB_KVM_COL] = 1;

	Json::Value JsonRoamArray;
	JsonRoamArray.resize(0);

	root[JSON_USB_KVM_ROAMING] = JsonRoamArray;

	char buf1[384] = "";

	memset(buf1,0,384);
	mysystem("astparam g kmoip_roaming_layout",buf1,512);

	if(strstr(buf1,"not defined") != 0)
	{
		root[JSON_USB_KVM_ROW] = 1;
		root[JSON_USB_KVM_COL] = 1;

		Json::Value JsonRoamArray;
		JsonRoamArray.resize(0);

		root[JSON_USB_KVM_ROAMING] = JsonRoamArray;
	}
	else
	{
		int x_min = 0,y_min = 0,x_max = 0,y_max = 0;
		int i = 0;
		Json::Value JsonRoamArray;
		JsonRoamArray.resize(16);

		for(i=0;i<16;i++)
		{
			int x,y;
			char mac[16] = "";
			char buf2[384] = "";

			int count = sscanf(buf1,"%[0123456789abcdefABCDEF],%d,%d:%s",mac,&x,&y,buf2);


			DBG_InfoMsg("count = %d\n",count);
			if(count < 3)
			{
				DBG_ErrMsg("count = %d\n",count);
				break;
			}
			else
			{
				if(strlen(mac) != 12)
				{
					DBG_ErrMsg("strlen(mac):%d != 12\n",strlen(mac));
					break;
				}

				if((x > 15)||(x < -15)||(y > 15)||(y < -15))
				{
					DBG_ErrMsg("x=%d or y=%d\n",x,y);
					break;
				}
				if(x < x_min)
					x_min = x;
				if(x > x_max)
					x_max = x;
				if(y < y_min)
					y_min = y;
				if(y > y_max)
					y_max = y;

				if(((x_max - x_min) > 15)||((y_max - y_min) > 15))
				{
					DBG_ErrMsg("x_max=%d x_min=%d y_max=%d y_min=%d\n",x_max,x_min,y_max,y_min);
					break;
				}

				Json::Value JsonKVM;

				JsonKVM[JSON_USB_KVM_MAC] = mac;
				JsonKVM[JSON_USB_KVM_H]   = x;
				JsonKVM[JSON_USB_KVM_V]   = y;

				JsonRoamArray[i] = JsonKVM;
			}

			if(count == 4)
			{
				if(strlen(buf2)>=16)
				{
					memset(buf1,0,sizeof(buf1));
					strcpy(buf1,buf2);
				}
				else
				{
					i++;

					DBG_ErrMsg("strlen(buf2):%s < 16\n",buf2);
					break;
				}
			}

			if(count == 3)
			{
				i++;

				DBG_InfoMsg("count == 3\n");
				break;
			}



		}

		if(i>0)
			JsonRoamArray.resize(i);

		root[JSON_USB_KVM_ROW] = (y_max - y_min)+1;
		root[JSON_USB_KVM_COL] = (x_max - x_min)+1;

		root[JSON_USB_KVM_ROAMING] = JsonRoamArray;

	}

	memset(buf1,0,64);
	mysystem("astparam g no_kmoip",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		root[JSON_USB_KVM_MODE] = JSON_USB_KVM_KM;
	else if(strstr(buf1,"y") != 0)
		root[JSON_USB_KVM_MODE] = JSON_USB_KVM_USB;
	else
		root[JSON_USB_KVM_MODE] = JSON_USB_KVM_KM;

	memset(buf1,0,64);
	mysystem("astparam g kmoip_token_interval",buf1,64);

	if(strstr(buf1,"not defined") != 0)
		root[JSON_USB_KVM_TIMEOUT] = 10;
	else
	{
		int time = atoi(buf1);
		int interval = time / 1000;
		if((interval <= 10)&&(interval >= 0))
			root[JSON_USB_KVM_TIMEOUT] = interval;
		else
			root[JSON_USB_KVM_TIMEOUT] = 10;
	}

	Json::Value root1;
	root1[JSON_USB_KVM_CONFIG] = root;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,KVM_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,KVM_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strKvm = fast_writer.write(root1);
	fwrite(strKvm.c_str(),1,strKvm.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

// /edid/edid_list.json
int Cfg_Create_EDIDList(void)
{
	DBG_InfoMsg("Cfg_Create_EDIDList\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_LIST_FILE);

	//Check EDID
	int nAccessRet = Cfg_Check_File(path);
	if(0 == nAccessRet)
	{
		return 0;
	}

	system("cp -rf /share/edid /data/configs/kds-7/");

	return 0;
}

int Cfg_Create_Channel(void)
{
	DBG_InfoMsg("Cfg_Create_Channel\n");

#ifdef CONFIG_P3K_HOST
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

#else
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_MAP_FILE);

	//Check channel map
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		DBG_InfoMsg("Cfg_Check_File %s Suceess\n",path);
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value JsonChanArray;
	JsonChanArray.resize(50);

	for(int i=1;i<51;i++)
	{
		Json::Value JsonChan;
		JsonChan[JSON_CH_ID] = i;

		char name[16] = "";
		sprintf(name,"ch_%03d",i);
		JsonChan[JSON_CH_NAME] = name;

		JsonChanArray[i-1] = JsonChan;
	}


	Json::Value root1;
	root1[JSON_CHAN_LIST] = JsonChanArray;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_MAP_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;

	string strChanList = fast_writer.write(root1);
	fwrite(strChanList.c_str(),1,strChanList.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
#endif
	return 0;
}

int Cfg_Update(SyncInfoType_E type)
{
	DBG_InfoMsg("Cfg_Update\n");
	pthread_mutex_lock(&(g_cfg_lock));

	switch(type)
	{
		case CHANNEL_INFO:
			Cfg_Update_Channel();
			break;
		case AUDIO_INFO:
			Cfg_Update_Audio();
			break;
		case VIDEO_INFO:
			Cfg_Update_Video();
			break;
		case AV_SETTING:
			Cfg_Update_AVSetting();
			break;
		case AUTOSWITCH_INFO:
			Cfg_Update_AutoSwitch();
			break;
		case EDID_INFO:
			Cfg_Update_EDID();
			break;
		case DEVICE_INFO:
			Cfg_Update_Device();
			break;
		case VERSION_INFO:
			Cfg_Update_Version();
			break;
		case TIME_INFO:
			Cfg_Update_Time();
			break;
		case USER_INFO:
			Cfg_Update_User();
			break;
		case VIDEOWALL_INFO:
			Cfg_Update_VideoWall();
			break;
		case GATEWAY_INFO:
			Cfg_Update_Gateway();
			break;
		case NETWORK_INFO:
			Cfg_Update_Network();
			break;
		case LOG_INFO:
			Cfg_Update_Log();
			break;
		default:
			break;
	}

	pthread_mutex_unlock(&(g_cfg_lock));

	return 0;
}

int Cfg_Update_Channel(void)
{
	DBG_InfoMsg("Cfg_Update_Channel\n");

#ifdef CONFIG_P3K_CLIENT
	Json::Value root;

	root[JSON_CHAN_VIDEO] = g_channel_select.video;
	root[JSON_CHAN_AUDIO] = g_channel_select.audio;
	root[JSON_CHAN_RS232] = g_channel_select.rs232;
	root[JSON_CHAN_IR] = g_channel_select.ir;
	root[JSON_CHAN_USB] = g_channel_select.usb;
	root[JSON_CHAN_CEC] = g_channel_select.cec;

	Json::Value root1;
	root1[JSON_CHANNEL_SELECT] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_SEL_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strChannelSel = fast_writer.write(root1);

	fwrite(strChannelSel.c_str(),1,strChannelSel.size(),fp);

	fflush(fp);
//	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

#else

	Json::Value root;

	root[JSON_CH_ID] = g_channel_info.channel_id;
	root[JSON_CH_NAME] = g_channel_info.channel_Name;

	Json::Value root1;
	root1[JSON_CHANNEL] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_DEF_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strChannelDef = fast_writer.write(root1);

	fwrite(strChannelDef.c_str(),1,strChannelDef.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
#endif
	return 0;
}

int Cfg_Update_Audio(void)
{
	DBG_InfoMsg("Cfg_Update_Audio\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	Json::Value root;

	if(g_audio_info.direction == DIRECTION_IN)
		root[JSON_AUDIO_ANALOG_DIR] = JSON_PARAM_IN;
	else
		root[JSON_AUDIO_ANALOG_DIR] = JSON_PARAM_OUT;


	if(g_audio_info.switch_mode == CONNECT_MANUAL)
		root[JSON_SWITCH_MODE] = JSON_MANUAL;
	else if(g_audio_info.switch_mode == CONNECT_PRIROITY)
		root[JSON_SWITCH_MODE] = JSON_PRIORITY;
	else
		root[JSON_SWITCH_MODE] = JSON_LAST_CONNECT;

	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
	{
		Json::Value JsonPriorityArray;
		JsonPriorityArray.resize(3);
		int jjj = 0;
		for(int i = 0; i < 3; i++)
		{

			Json::Value& JasonPri = JsonPriorityArray[jjj];

			if(g_audio_info.input_pri[i] == AUDIO_IN_HDMI)
			{
				JasonPri = JSON_AUDIO_HDMI;
				jjj++;
			}
			else if(g_audio_info.input_pri[i] == AUDIO_IN_ANALOG)
			{
				JasonPri = JSON_AUDIO_ANALOG;
				jjj++;
			}
			else if(g_audio_info.input_pri[i] == AUDIO_IN_DANTE)
			{
				JasonPri = JSON_AUDIO_DANTE;
				jjj++;
			}
			else
				continue;
		}

		JsonPriorityArray.resize(jjj);

		root[JSON_PRIORITY] = JsonPriorityArray;

		Json::Value JsonDestArray;
		JsonDestArray.resize(4);
		jjj = 0;
		for(int i = 0; i < 4; i++)
		{
			Json::Value& JsonDst = JsonDestArray[jjj];

			if(g_audio_info.dst_port[i] == PORT_HDMI)
			{
				JsonDst = JSON_AUDIO_HDMI;
				jjj++;
			}
			else if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
			{
				JsonDst = JSON_AUDIO_ANALOG;
				jjj++;
			}
			else if(g_audio_info.dst_port[i] == PORT_DANTE)
			{
				JsonDst = JSON_AUDIO_DANTE;
				jjj++;
			}
			else if(g_audio_info.dst_port[i] == PORT_STREAM)
			{
				JsonDst = JSON_AUDIO_LAN;
				jjj++;
			}
			else
			{
				continue;
			}
		}
		JsonDestArray.resize(jjj);
		root[JSON_AUDIO_DEST] = JsonDestArray;

		if(g_audio_info.source == AUDIO_IN_HDMI)
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_HDMI;
		else if(g_audio_info.source == AUDIO_IN_ANALOG)
		{
			if(g_audio_info.direction == DIRECTION_IN)
				root[JSON_SOURCE_SELECT] = JSON_AUDIO_ANALOG;
			else
				root[JSON_SOURCE_SELECT] = JSON_AUDIO_NONE;
		}
		else if(g_audio_info.source == AUDIO_IN_DANTE)
		{
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_DANTE;
		}
		else
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_NONE;


		if(g_bCfg == 0)
		{
			if(strlen(g_audio_info.dante_name) > 0)
				root[JSON_DANTE_NAME] = g_audio_info.dante_name;
		}

	}
	else // IPE5000W & IPE5000
	{
		int jjj = 0;
		Json::Value JsonPriorityArray;
		JsonPriorityArray.resize(2);
		for(int i = 0; i < 2; i++)
		{
			Json::Value& JasonPri = JsonPriorityArray[jjj];

			if(g_audio_info.input_pri[i] == AUDIO_IN_HDMI)
			{
				JasonPri = JSON_AUDIO_HDMI;
				jjj++;
			}
			else if(g_audio_info.input_pri[i] == AUDIO_IN_ANALOG)
			{
				JasonPri = JSON_AUDIO_ANALOG;
				jjj++;
			}
			else
				continue;
		}
		JsonPriorityArray.resize(jjj);

		root[JSON_PRIORITY] = JsonPriorityArray;

		Json::Value JsonDestArray;
		JsonDestArray.resize(3);
		jjj = 0;
		for(int i = 0; i < 3; i++)
		{
			Json::Value& JsonDst = JsonDestArray[jjj];

			if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
			{
				JsonDst = JSON_AUDIO_ANALOG;
				jjj++;
			}
			else if(g_audio_info.dst_port[i] == PORT_STREAM)
			{
				JsonDst = JSON_AUDIO_LAN;
				jjj++;
			}
			else
			{
				continue;
			}
		}

		JsonDestArray.resize(jjj);
		root[JSON_AUDIO_DEST] = JsonDestArray;

		if(g_audio_info.source == AUDIO_IN_HDMI)
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_HDMI;
		else if(g_audio_info.source == AUDIO_IN_ANALOG)
		{
			if(g_audio_info.direction == DIRECTION_IN)
				root[JSON_SOURCE_SELECT] = JSON_AUDIO_ANALOG;
			else
				root[JSON_SOURCE_SELECT] = JSON_AUDIO_NONE;
		}
		else
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_NONE;

	}




	Json::Value root1;
	root1[JSON_AUDIO] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUDIO_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUDIO_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAutoSwitch = fast_writer.write(root1);

	fwrite(strAutoSwitch.c_str(),1,strAutoSwitch.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_Video(void)
{
	DBG_InfoMsg("Cfg_Update_Video\n");
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);

	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";

	//Check Video cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		DBG_ErrMsg("nAccessRet %s Failed\n",path);
	}
	else
	{
		//Read  av setting cfg
		FILE *fp;
		fp = fopen(path, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open %s\n",path);
			return -1;
		}

		if (flock(fileno(fp), LOCK_EX)) {
			DBG_ErrMsg("%s lock failed\n",path);
			fclose(fp);
			return -1;
		}

		fread(pBuf,1,sizeof(pBuf),fp);

		if(reader.parse(pBuf, root1))
		{
			DBG_InfoMsg("open %s Success !!!\n",path);
		}

		flock(fileno(fp), LOCK_UN);

		fclose(fp);
	}

	if(!root1[JSON_AV_SIGNAL].empty())
	{
		Json::Value& root = root1[JSON_AV_SIGNAL];

		if(g_video_info.force_rgb == 0)
			root[JSON_AV_RGB] = JSON_PARAM_OFF;
		else
			root[JSON_AV_RGB] = JSON_PARAM_ON;

#ifdef CONFIG_P3K_CLIENT
		if(g_video_info.scale_mode == 0)
			root[JSON_AV_OUT_RES] = JSON_AV_PASSTHRU;
		else if(g_video_info.scale_mode == 1)
		{
			if(g_video_info.res_type == 74)//2160p30
				root[JSON_AV_OUT_RES] = "2160p30";
			else if(g_video_info.res_type == 73)//2160p25
				root[JSON_AV_OUT_RES] = "2160p25";
			else if(g_video_info.res_type == 16)//1080p60
				root[JSON_AV_OUT_RES] = "1080p60";
			else if(g_video_info.res_type == 31)//1080p50
				root[JSON_AV_OUT_RES] = "1080p50";
			else if(g_video_info.res_type == 4)//720p60
				root[JSON_AV_OUT_RES] = "720p60";
			else
			{
				DBG_WarnMsg(" !!! Error para res:%d\n",g_video_info.res_type);
				return 0;
			}
		}

#endif
	}
	else
	{
		Json::Value root;
		if(g_video_info.force_rgb == 0)
			root[JSON_AV_RGB] = JSON_PARAM_OFF;
		else
			root[JSON_AV_RGB] = JSON_PARAM_ON;

#ifdef CONFIG_P3K_CLIENT
		if(g_video_info.scale_mode == 0)
			root[JSON_AV_OUT_RES] = JSON_AV_PASSTHRU;
		else if(g_video_info.scale_mode == 1)
		{
			if(g_video_info.res_type == 74)//2160p30
				root[JSON_AV_OUT_RES] = "2160p30";
			else if(g_video_info.res_type == 73)//2160p25
				root[JSON_AV_OUT_RES] = "2160p25";
			else if(g_video_info.res_type == 16)//1080p60
				root[JSON_AV_OUT_RES] = "1080p60";
			else if(g_video_info.res_type == 31)//1080p50
				root[JSON_AV_OUT_RES] = "1080p50";
			else if(g_video_info.res_type == 4)//720p60
				root[JSON_AV_OUT_RES] = "720p60";
			else
			{
				DBG_WarnMsg(" !!! Error para res:%d\n",g_video_info.res_type);
				return 0;
			}
		}

#endif
		root1[JSON_AV_SIGNAL] = root;
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);
	FILE *fp2;
	fp2 = fopen(path, "w");
	if (fp2 == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp2), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp2);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAVsetting = fast_writer.write(root1);

	fwrite(strAVsetting.c_str(),1,strAVsetting.size(),fp2);

	fflush(fp2);
	fsync(fileno(fp2));
	flock(fileno(fp2), LOCK_UN);
	fclose(fp2);
	return 0;
}

int Cfg_Update_AutoSwitch(void)
{
	DBG_InfoMsg("Cfg_Update_AutoSwitch\n");

	Json::Value root;

#ifdef CONFIG_P3K_HOST
	if(g_autoswitch_info.switch_mode == CONNECT_MANUAL)
	{
		root[JSON_SWITCH_MODE] = JSON_MANUAL;
	}
	else if(g_autoswitch_info.switch_mode == CONNECT_PRIROITY)
	{
		root[JSON_SWITCH_MODE] = JSON_PRIORITY;
	}
	else
	{
		root[JSON_SWITCH_MODE] = JSON_LAST_CONNECT;
	}

	Json::Value JsonPriorityArray;
	if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
	{
		JsonPriorityArray.resize(2);
		for(int i = 0; i < 2; i++)
		{
			Json::Value& JasonPri = JsonPriorityArray[i];

			if(g_autoswitch_info.input_pri[i] == 1)
				JasonPri = JSON_HDMI_1;
			else if(g_autoswitch_info.input_pri[i] == 2)
				JasonPri = JSON_USBC_2;
			else
				continue;
		}

		root[JSON_PRIORITY] = JsonPriorityArray;

		if(g_autoswitch_info.source == 1)
			root[JSON_SOURCE_SELECT] = JSON_HDMI_1;
		else if(g_autoswitch_info.source == 2)
			root[JSON_SOURCE_SELECT] = JSON_USBC_2;
	}
	else
	{
		JsonPriorityArray.resize(3);
		for(int i = 0; i < 3; i++)
		{
			Json::Value& JasonPri = JsonPriorityArray[i];

			if(g_autoswitch_info.input_pri[i] == 1)
				JasonPri = JSON_HDMI_1;
			else if(g_autoswitch_info.input_pri[i] == 2)
				JasonPri = JSON_HDMI_2;
			else if(g_autoswitch_info.input_pri[i] == 3)
				JasonPri = JSON_USBC_3;
			else
				continue;
		}

		root[JSON_PRIORITY] = JsonPriorityArray;

		if(g_autoswitch_info.source == 1)
			root[JSON_SOURCE_SELECT] = JSON_HDMI_1;
		else if(g_autoswitch_info.source == 2)
			root[JSON_SOURCE_SELECT] = JSON_HDMI_2;
		else
			root[JSON_SOURCE_SELECT] = JSON_USBC_3;
	}
#else
	if(strcmp(g_version_info.model,IPD_W_MODULE) == 0)
	{
		root[JSON_SOURCE_SELECT] = JSON_STREAM;
	}
	else
	{
		if(g_autoswitch_info.source == 1)
			root[JSON_SOURCE_SELECT] = JSON_HDMI_1;
		else
			root[JSON_SOURCE_SELECT] = JSON_STREAM;
	}
#endif
	Json::Value root1;
	root1[JSON_AUTOSWITCH] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAutoSwitch = fast_writer.write(root1);

	fwrite(strAutoSwitch.c_str(),1,strAutoSwitch.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	return 0;
}

int Cfg_Update_AVSetting(void)
{
	DBG_InfoMsg("Cfg_Update_AVSetting\n");

	Json::Value root;

	root[JSON_AV_VOLUME] = g_avsetting_info.volume;

	if(g_avsetting_info.mute_mode == ON)
		root[JSON_AV_MUTE] = JSON_PARAM_ON;
	else
		root[JSON_AV_MUTE] = JSON_PARAM_OFF;



	if(g_avsetting_info.action == CODEC_ACTION_STOP)
		root[JSON_AV_ACTION] = JSON_AV_STOP;
	else
		root[JSON_AV_ACTION] = JSON_AV_PLAY;


#ifdef CONFIG_P3K_HOST
	Json::Value JsonHDCP;
	{
		if(g_avsetting_info.hdcp_mode[0] == ON)
			JsonHDCP[JSON_HDMI_1] = JSON_PARAM_ON;
		else
			JsonHDCP[JSON_HDMI_1] = JSON_PARAM_OFF;

		if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if(g_avsetting_info.hdcp_mode[1] == ON)
				JsonHDCP[JSON_USBC_2] = JSON_PARAM_ON;
			else
				JsonHDCP[JSON_USBC_2] = JSON_PARAM_OFF;
		}
		else
		{
			if(g_avsetting_info.hdcp_mode[1] == ON)
				JsonHDCP[JSON_HDMI_2] = JSON_PARAM_ON;
			else
				JsonHDCP[JSON_HDMI_2] = JSON_PARAM_OFF;


			if(g_avsetting_info.hdcp_mode[2] == ON)
				JsonHDCP[JSON_USBC_3] = JSON_PARAM_ON;
			else
				JsonHDCP[JSON_USBC_3] = JSON_PARAM_OFF;
		}
	}

	root[JSON_AV_HDCP] = JsonHDCP;
#endif
	Json::Value root1;
	root1[JSON_AV_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SETTING_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SETTING_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strAVSetting = fast_writer.write(root1);

	fwrite(strAVSetting.c_str(),1,strAVSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_EDID(void)
{
	DBG_InfoMsg("Cfg_Update_EDID\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	Json::Value root;

	if(g_edid_info.lock_mode == ON)
		root[JSON_EDID_LOCK] = JSON_PARAM_ON;
	else
		root[JSON_EDID_LOCK] = JSON_PARAM_OFF;

	if(g_edid_info.edid_mode == CUSTOM)
		root[JSON_EDID_MODE] = JSON_EDID_CUSTOM;
	else 	if(g_edid_info.edid_mode == PASSTHRU)
		root[JSON_EDID_MODE] = JSON_EDID_PASSTHRU;
	else
		root[JSON_EDID_MODE] = JSON_EDID_DEFAULTE;

	root[JSON_EDID_SRC] = g_edid_info.net_src;

	char id[4] = "";
	sprintf(id,"%d",g_edid_info.active_id);
	root[JSON_EDID_ACTIVE] = id;

	Json::Value root1;
	root1[JSON_EDID_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_SETTING_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strEDIDSetting = fast_writer.write(root1);

	fwrite(strEDIDSetting.c_str(),1,strEDIDSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_Device(void)
{
	DBG_InfoMsg("Cfg_Update_Device\n");

	Json::Value root;

	if(g_device_info.fp_lock == ON)
		root[JSON_DEV_FP_LOCK] = JSON_PARAM_ON;
	else
		root[JSON_DEV_FP_LOCK] = JSON_PARAM_OFF;

	root[JSON_DEV_HOSTNAME] = g_device_info.hostname;
	root[JSON_DEV_MAC] = g_device_info.mac_addr;
	root[JSON_DEV_SN] = g_device_info.sn;
	root[JSON_DEV_STANDBY_TIME] = g_device_info.standby_time;


	Json::Value root1;
	root1[JSON_DEV_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DEVICE_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DEVICE_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strDeviceSetting = fast_writer.write(root1);

	fwrite(strDeviceSetting.c_str(),1,strDeviceSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_Version(void)
{
	DBG_InfoMsg("Cfg_Update_Device\n");

	Json::Value root;

	root[JSON_VER_FILE] = g_version_info.file_version;
	root[JSON_VER_UPG_TIME] = g_version_info.upg_time;
	root[JSON_VER_STB_VERSION] = g_version_info.standby_version;

	DBG_InfoMsg("Cfg_Update_Device JSON_VER_STB_VERSION %s\n",g_version_info.standby_version);

	Json::Value device;
	device[JSON_VER_MODEL] = g_version_info.model;
	device[JSON_VER_HW_VERSION] = g_version_info.hw_version;
	device[JSON_VER_FW_VERSION] = g_version_info.fw_version;

	root[JSON_VER_DEVICE] = device;

	Json::Value root1;
	root1[JSON_VERSION] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VERSION_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VERSION_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strVersionSetting = fast_writer.write(root1);

	fwrite(strVersionSetting.c_str(),1,strVersionSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_Time(void)
{
	DBG_InfoMsg("Cfg_Update_Time\n");

	Json::Value root;

	root[JSON_TIME_ZONE] = g_time_info.time_zone;
	root[JSON_TIME_DAYLIGHT] = g_time_info.daylight_saving;

	Json::Value ntp;
	if(g_time_info.ntp_mode== ON)
		ntp[JSON_NTP_MODE] = JSON_PARAM_ON;
	else
		ntp[JSON_NTP_MODE] = JSON_PARAM_OFF;

	ntp[JSON_NTP_IP] = g_time_info.ntp_server;
	ntp[JSON_NTP_SYNC_HOUR] = g_time_info.ntp_sync_hour;

	if(g_time_info.ntp_server_status== ON)
		ntp[JSON_NTP_STATUS] = JSON_PARAM_ON;
	else
		ntp[JSON_NTP_STATUS] = JSON_PARAM_OFF;

	root[JSON_NTP_SERVER] = ntp;

	Json::Value root1;
	root1[JSON_TIME_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,TIME_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,TIME_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strTimeSetting = fast_writer.write(root1);

	fwrite(strTimeSetting.c_str(),1,strTimeSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_User(void)
{
	DBG_InfoMsg("Cfg_Update_User\n");
	Json::Value root;

	root[JSON_USER_NAME] = g_user_info.user_name;
	root[JSON_USER_PASS] = g_user_info.password;

	if(g_user_info.user_role == 0)
		root[JSON_USER_ROLE] = JSON_USER_USER;
	else
		root[JSON_USER_ROLE] = JSON_USER_ADMIN;

	root[JSON_USER_LOGOUT] = g_user_info.logout_time;

	if(g_user_info.seurity_status == ON)
		root[JSON_USER_SECUR] = JSON_PARAM_ON;
	else
		root[JSON_USER_SECUR] = JSON_PARAM_OFF;

	Json::Value root1;
	root1[JSON_USER_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,USER_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,USER_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strUSerSetting = fast_writer.write(root1);

	fwrite(strUSerSetting.c_str(),1,strUSerSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_VideoWall(void)
{
	DBG_InfoMsg("Cfg_Update_VideoWall\n");

#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	Json::Value root;

	root[JSON_VW_H_COUNT] = g_videowall_info.horizontal_count;
	root[JSON_VW_V_COUNT] = g_videowall_info.vertical_count;
	root[JSON_VW_POSITION] = g_videowall_info.relative_position;

	if(g_videowall_info.stretch_type == 0)
		root[JSON_VW_STRETCH] = JSON_VW_STRETCH_IN;
	else
		root[JSON_VW_STRETCH] = JSON_VW_STRETCH_OUT;

	root[JSON_VW_ROTATION] = g_videowall_info.rotation;

	Json::Value bezel;
	bezel[JSON_VW_BEZEL_H_VALUE] = g_videowall_info.bezel_horizontal_value;
	bezel[JSON_VW_BEZEL_V_VALUE] = g_videowall_info.bezel_vertical_value;
	bezel[JSON_VW_BEZEL_H_OFFSET] = g_videowall_info.bezel_horizontal_offset;
	bezel[JSON_VW_BEZEL_V_OFFSET] = g_videowall_info.bezel_vertical_offset;

	root[JSON_VW_BEZEL] = bezel;

	Json::Value root1;
	root1[JSON_VW_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VIDEOWALL_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VIDEOWALL_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strVideowallSetting = fast_writer.write(root1);

	fwrite(strVideowallSetting.c_str(),1,strVideowallSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	return 0;
}

int Cfg_Update_Gateway(void)
{
	DBG_InfoMsg("Cfg_Update_Gateway\n");

	Json::Value root;

	if(g_gateway_info.cec_mode == ON)
		root[JSON_GW_CEC_MODE] = JSON_PARAM_ON;
	else
		root[JSON_GW_CEC_MODE] = JSON_PARAM_OFF;

	if(g_gateway_info.cec_output == 1)
		root[JSON_GW_CEC_DEST] = JSON_GW_CEC_DEST_IN;
	else if(g_gateway_info.cec_output == 2)
		root[JSON_GW_CEC_DEST] = JSON_GW_CEC_DEST_OUT;
	else if(g_gateway_info.cec_output == 3)
		root[JSON_GW_CEC_DEST] = JSON_GW_CEC_DEST_LOOP;
	else
		root[JSON_GW_CEC_DEST] = JSON_GW_CEC_DEST_PASS;

	if(g_gateway_info.rs232_mode == ON)
		root[JSON_GW_UART_MODE] = JSON_PARAM_ON;
	else
		root[JSON_GW_UART_MODE] = JSON_PARAM_OFF;

	root[JSON_GW_UART_PORT] = g_gateway_info.rs232_port;

	Json::Value JsonUartParam;

	if((g_gateway_info.rs232_param.rate == 9600)
		||(g_gateway_info.rs232_param.rate == 19200)
		||(g_gateway_info.rs232_param.rate == 38400)
		||(g_gateway_info.rs232_param.rate == 57600)
		||(g_gateway_info.rs232_param.rate == 115200))
	{
		JsonUartParam[JSON_GW_UART_RATE] = g_gateway_info.rs232_param.rate;
	}
	else
	{
		g_gateway_info.rs232_param.rate = 115200;
		JsonUartParam[JSON_GW_UART_RATE] = g_gateway_info.rs232_param.rate;
	}

	if((g_gateway_info.rs232_param.bitWidth>=5)&&(g_gateway_info.rs232_param.bitWidth<=8))
	{
		JsonUartParam[JSON_GW_UART_BITS] = g_gateway_info.rs232_param.bitWidth;
	}
	else
	{
		g_gateway_info.rs232_param.bitWidth = 8;
		JsonUartParam[JSON_GW_UART_BITS] = g_gateway_info.rs232_param.bitWidth;
	}

	if(g_gateway_info.rs232_param.parity == PARITY_NONE)
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_NONE;
	else if(g_gateway_info.rs232_param.parity == PARITY_ODD)
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_ODD;
	else if(g_gateway_info.rs232_param.parity == PARITY_EVEN)
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_EVEN;
	else if(g_gateway_info.rs232_param.parity == PARITY_MARK)
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_MARK;
	else if(g_gateway_info.rs232_param.parity == PARITY_SPACE)
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_SPACE;
	else
		JsonUartParam[JSON_GW_UART_PARITY] = JSON_GW_UART_PARITY_NONE;

	if((g_gateway_info.rs232_param.stopBitsMode == 1)||(g_gateway_info.rs232_param.stopBitsMode == 2))
		JsonUartParam[JSON_GW_UART_STOP] = g_gateway_info.rs232_param.stopBitsMode;
	else
	{
		g_gateway_info.rs232_param.stopBitsMode = 1;
		JsonUartParam[JSON_GW_UART_STOP] = g_gateway_info.rs232_param.stopBitsMode;
	}

	root[JSON_GW_UART_PARAM] = JsonUartParam;

	if(g_gateway_info.ir_mode == ON)
		root[JSON_GW_IR_MODE] = JSON_PARAM_ON;
	else
		root[JSON_GW_IR_MODE] = JSON_PARAM_OFF;

	if(g_gateway_info.ir_direction == DIRECTION_IN)
		root[JSON_GW_IR_DIR] = JSON_PARAM_IN;
	else
		root[JSON_GW_IR_DIR] = JSON_PARAM_OUT;

	Json::Value root1;
	root1[JSON_GW] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,GATEWAY_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,GATEWAY_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strGateway = fast_writer.write(root1);

	fwrite(strGateway.c_str(),1,strGateway.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	return 0;
}

int Cfg_Update_Network(void)
{
	DBG_InfoMsg("Cfg_Update_Network\n");

	Json::Value root;

	for(int i = 0;i <= 1; i++)
	{
		Json::Value eth;
		if(g_network_info.eth_info[i].dhcp_enable == 0)
		{
			eth[JSON_NETWORK_MODE] = JSON_NETWORK_STATIC;
			eth[JSON_NETWORK_IP] = g_network_info.eth_info[i].ipAddr;
			eth[JSON_NETWORK_MASK] = g_network_info.eth_info[i].mask;
			eth[JSON_NETWORK_GW] = g_network_info.eth_info[i].gateway;
		}
		else
		{
			eth[JSON_NETWORK_MODE] = JSON_NETWORK_DHCP;
		}

		if(i == 0)
			root[JSON_NETWORK_STREAM] = eth;
		else
			root[JSON_NETWORK_CONTROL] = eth;
	}

	if(g_network_info.daisy_chain == ON)
		root[JSON_NETWORK_DAISY] = JSON_PARAM_ON;
	else
		root[JSON_NETWORK_DAISY] = JSON_PARAM_OFF;

	root[JSON_NETWORK_TCP] = g_network_info.tcp_port;
	root[JSON_NETWORK_UDP] = g_network_info.udp_port;

	if(g_network_info.method == Net_UNICAST)
		root[JSON_NETWORK_METHOD] = JSON_NETWORK_UNI;
	else
	{
		root[JSON_NETWORK_METHOD] = JSON_NETWORK_MULTICAST;

		Json::Value multi;
		multi[JSON_NETWORK_GROUP_IP] = "0.0.0.0";
		multi[JSON_NETWORK_TTL] = g_network_info.multicast_ttl;

		root[JSON_NETWORK_MULTICAST] = multi;
	}

	Json::Value port;
	Json::Value control;
	Json::Value dante;

	if(g_network_info.control_port == 0)
		control[JSON_NETWORK_PORT] = JSON_NETWORK_ETH0;
	else
		control[JSON_NETWORK_PORT] = JSON_NETWORK_ETH1;
 	control[JSON_NETWORK_VLAN] = g_network_info.control_vlan;
 	port[JSON_NETWORK_CONTROL] = control;

	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
	{
		if(g_network_info.dante_port == 0)
			dante[JSON_NETWORK_PORT] = JSON_NETWORK_ETH0;
		else
			dante[JSON_NETWORK_PORT] = JSON_NETWORK_ETH1;
	 	dante[JSON_NETWORK_VLAN] = g_network_info.dante_vlan;
	 	port[JSON_NETWORK_DANTE] = dante;
	}

	root[JSON_NETWORK_PORT_SET] = port;

	Json::Value beacon_info;
	if(g_network_info.beacon_en == ON)
		beacon_info[JSON_NETWORK_BEACON_EN] = JSON_PARAM_ON;
	else
		beacon_info[JSON_NETWORK_BEACON_EN] = JSON_PARAM_OFF;


	beacon_info[JSON_NETWORK_BEACON_IP] = g_network_info.beacon_ip;
	beacon_info[JSON_NETWORK_BEACON_PORT] = g_network_info.beacon_port;
    beacon_info[JSON_NETWORK_BEACON_TIME] = g_network_info.beacon_time;
	root[JSON_NETWORK_BEACON_INFO] = beacon_info;


	Json::Value root1;
	root1[JSON_NETWORK] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,NETWORK_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,NETWORK_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strNetwork = fast_writer.write(root1);

	fwrite(strNetwork.c_str(),1,strNetwork.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);

	return 0;
}

int Cfg_Update_Log(void)
{
	DBG_InfoMsg("Cfg_Update_Log\n");
	Json::Value root;

	if(g_log_info.active == ON)
		root[JSON_LOG_ACTIVE] = JSON_PARAM_ON;
	else
		root[JSON_LOG_ACTIVE] = JSON_PARAM_OFF;

	if(g_log_info.period == 2)
		root[JSON_LOG_PERIOD] = JSON_LOG_DAILY;
	else
		root[JSON_LOG_PERIOD] = JSON_LOG_WEEKLY;


	Json::Value root1;
	root1[JSON_LOG_SETTING] = root;

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,LOG_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,LOG_SETTING_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strLogSetting = fast_writer.write(root1);

	fwrite(strLogSetting.c_str(),1,strLogSetting.size(),fp);

	fflush(fp);
	fsync(fileno(fp));
	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Update_OSD(void)
{
	DBG_InfoMsg("Cfg_Update_OSD\n");
#ifdef CONFIG_P3K_HOST
	DBG_InfoMsg("This is Encoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_FILE);

	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";

	//Check OSD cfg
	int nAccessRet = Cfg_Check_File(path);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);
	}
	else
	{
		//Read  OSD setting cfg
		FILE *fp;
		fp = fopen(path, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open %s\n",path);
			return -1;
		}

		if (flock(fileno(fp), LOCK_EX)) {
			DBG_ErrMsg("%s lock failed\n",path);
			fclose(fp);
			return -1;
		}

		fread(pBuf,1,sizeof(pBuf),fp);

		if(reader.parse(pBuf, root1))
		{
			DBG_InfoMsg("open %s Success !!!\n",path);
		}

		flock(fileno(fp), LOCK_UN);

		fclose(fp);
	}

	if(!root1[JSON_OSD_DEVICE_INFO].empty())
	{
		Json::Value& root = root1[JSON_OSD_DEVICE_INFO];

		if(g_osd_enable == OFF)
			root[JSON_OSD_DEVICE_INFO_EN] = JSON_PARAM_OFF;
		else
			root[JSON_OSD_DEVICE_INFO_EN] = JSON_PARAM_ON;
	}
	else
	{
		Json::Value root;
		if(g_osd_enable == OFF)
			root[JSON_OSD_DEVICE_INFO_EN] = JSON_PARAM_OFF;
		else
			root[JSON_OSD_DEVICE_INFO_EN] = JSON_PARAM_ON;


		root1[JSON_OSD_DEVICE_INFO] = root;
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,OSD_FILE);
	FILE *fp2;
	fp2 = fopen(path, "w");
	if (fp2 == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp2), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp2);
		return -1;
	}

	Json::FastWriter fast_writer;
	string strOSDsetting = fast_writer.write(root1);

	fwrite(strOSDsetting.c_str(),1,strOSDsetting.size(),fp2);

	fflush(fp2);
	fsync(fileno(fp2));
	flock(fileno(fp2), LOCK_UN);
	fclose(fp2);
	return 0;

}

int Cfg_Set_DecChannel_ID(ChSelect_S * id)
{
	DBG_InfoMsg("Cfg_Set_DecChannel_ID\n");

#if 1
	for(int i = 0;i < id->i_signalnum;i ++)
    {
		DBG_InfoMsg("signal[%d]: %d; id: %d",i,id->signal[i],id->ch_id);
	    if(id->signal[i] == SIGNAL_VIDEO)
			g_channel_select.video = id->ch_id;
		else if(id->signal[i] == SIGNAL_AUDIO)
			g_channel_select.audio = id->ch_id;
		else if(id->signal[i] == SIGNAL_RS232)
			g_channel_select.rs232 = id->ch_id;
		else if(id->signal[i] == SIGNAL_IR)
			g_channel_select.ir = id->ch_id;
		else if(id->signal[i] == SIGNAL_USB)
			g_channel_select.usb = id->ch_id;
		else if(id->signal[i] == SIGNAL_CEC)
			g_channel_select.cec = id->ch_id;

    }
#else
	g_channel_select.video = id->ch_id;
	g_channel_select.audio = id->ch_id;
	g_channel_select.rs232 = id->ch_id;
	g_channel_select.ir = id->ch_id;
	g_channel_select.usb = id->ch_id;
	g_channel_select.cec = id->ch_id;
#endif
	Cfg_Update(CHANNEL_INFO);
	return 0;
}

int Cfg_Set_EncChannel_ID(int id)
{
	DBG_InfoMsg("Cfg_Set_EncChannel_ID\n");
	g_channel_info.channel_id = id;
	Cfg_Update(CHANNEL_INFO);
	return 0;
}

int Cfg_Get_EncChannel_ID(int* id)
{
	DBG_InfoMsg("Cfg_Get_EncChannel_ID\n");
	*id = g_channel_info.channel_id;
	return 0;
}

int Cfg_Set_EncChannel_Name(char* name)
{
	DBG_InfoMsg("Cfg_Set_EncChannel_Name\n");
	if(strlen(name) <= 31)
		strcpy(g_channel_info.channel_Name,name);
	else
		memcpy(g_channel_info.channel_Name,name,31);
	Cfg_Update(CHANNEL_INFO);
	return 0;
}

int Cfg_Get_EncChannel_Name(char* name)
{
	DBG_InfoMsg("Cfg_Get_EncChannel_Name\n");
	strcpy(name,g_channel_info.channel_Name);
	return 0;
}

int Cfg_Set_Port_Dir(PortSignalType_E type,PortDirectionType_E direction)
{
	DBG_InfoMsg("Cfg_Set_Port_Dir\n");
	if(type == PORT_ANALOG_AUDIO)
	{
		g_audio_info.direction = direction;
		Cfg_Update(AUDIO_INFO);
	}
	else if(type == PORT_IR)
	{
		g_gateway_info.ir_direction = direction;
		Cfg_Update(GATEWAY_INFO);
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_Port_Dir type:%d\n",type);
	}
	return 0;
}

int Cfg_Get_Port_Dir(PortSignalType_E type,PortDirectionType_E* direction)
{
	DBG_InfoMsg("Cfg_Get_Port_Dir\n");
	if(type == PORT_ANALOG_AUDIO)
	{
		*direction = g_audio_info.direction;
	}
	else if(type == PORT_IR)
	{
		*direction = g_gateway_info.ir_direction;
	}
	else
	{
		DBG_ErrMsg("Cfg_Get_Port_Dir type:%d\n",type);
	}
	return 0;
}

int Cfg_Set_Autoswitch_Mode(SignalType_E type,AVConnectMode_E mode)
{
	DBG_InfoMsg("Cfg_Set_Autoswitch_Mode\n");
	if(type == SIGNAL_VIDEO)
	{
		g_autoswitch_info.switch_mode = mode;
		Cfg_Update(AUTOSWITCH_INFO);
	}
	else if(type == SIGNAL_AUDIO)
	{
		g_audio_info.switch_mode = mode;
		Cfg_Update(AUDIO_INFO);
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_Autoswitch_Mode type:%d\n",type);
	}

	return 0;
}

int Cfg_Get_Autoswitch_Mode(SignalType_E type,AVConnectMode_E* mode)
{
	DBG_InfoMsg("Cfg_Get_Autoswitch_Mode\n");
	if(type == SIGNAL_VIDEO)
	{
		 *mode = g_autoswitch_info.switch_mode;
	}
	else if(type == SIGNAL_AUDIO)
	{
		 *mode = g_audio_info.switch_mode;
	}
	else
	{
		DBG_ErrMsg("Cfg_Get_Autoswitch_Mode type:%d\n",type);
	}

	return 0;
}
int Cfg_Set_Autoswitch_Priority(SignalType_E type,int port1, int port2,int port3)
{
	DBG_InfoMsg("Cfg_Set_Autoswitch_Priority\n");
	if(type == SIGNAL_VIDEO)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			g_autoswitch_info.input_pri[0] = port1;
			g_autoswitch_info.input_pri[1] = port2;
			g_autoswitch_info.input_pri[2] = port3;
			Cfg_Update(AUTOSWITCH_INFO);
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			g_autoswitch_info.input_pri[0] = port1;
			g_autoswitch_info.input_pri[1] = port2;
			Cfg_Update(AUTOSWITCH_INFO);
		}
		else
		{
			DBG_ErrMsg("This is not switcher\n");
			return -1;

		}
	}
	else if(type == SIGNAL_AUDIO)
	{
		if(g_audio_info.direction == DIRECTION_OUT)
		{
			if((port1 == AUDIO_IN_ANALOG)||(port2 == AUDIO_IN_ANALOG)||(port3 == AUDIO_IN_ANALOG))
			{
				DBG_ErrMsg("analog is out\n");
				return -1;
			}
		}
		else if(strcmp(g_version_info.model,IPE_P_MODULE) != 0)
		{
			if((port1 == AUDIO_IN_DANTE)||(port2 == AUDIO_IN_DANTE)||(port3 == AUDIO_IN_DANTE))
			{
				DBG_ErrMsg("This is not switcher\n");
				return -1;
			}

		}
		g_audio_info.input_pri[0] = (AudioInputMode_E)port1;
		g_audio_info.input_pri[1] = (AudioInputMode_E)port2;
		g_audio_info.input_pri[2] = (AudioInputMode_E)port3;
		Cfg_Update(AUDIO_INFO);
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_Autoswitch_Priority type:%d\n",type);
	}
	return 0;
}

int Cfg_Get_Autoswitch_Priority(SignalType_E type,int* port1, int* port2, int* port3)
{
	DBG_InfoMsg("Cfg_Get_Autoswitch_Priority\n");
	if(type == SIGNAL_VIDEO)
	{
		*port1 = g_autoswitch_info.input_pri[0];
		*port2 = g_autoswitch_info.input_pri[1];
		*port3 = g_autoswitch_info.input_pri[2];
	}
	else if(type == SIGNAL_AUDIO)
	{
		*port1 = g_audio_info.input_pri[0];
		*port2 = g_audio_info.input_pri[1];
		*port3 = g_audio_info.input_pri[2];
	}
	else
	{
		DBG_ErrMsg("Cfg_Get_Autoswitch_Priority type:%d\n",type);
	}
	return 0;
}

int Cfg_Set_Autoswitch_Source(SignalType_E type,int port)
{
	DBG_InfoMsg("Cfg_Set_Autoswitch_Source\n");
	if(type == SIGNAL_VIDEO)
	{
		g_autoswitch_info.source = port;
		Cfg_Update(AUTOSWITCH_INFO);
	}
	else if(type == SIGNAL_AUDIO)
	{
		if((port == AUDIO_IN_HDMI)||(port == AUDIO_IN_NONE))
		{
			g_audio_info.source = (AudioInputMode_E)port;
			Cfg_Update(AUDIO_INFO);
		}
		else if(port == AUDIO_IN_ANALOG)
		{
			if(g_audio_info.direction == DIRECTION_IN)
			{
				g_audio_info.source = (AudioInputMode_E)port;
				Cfg_Update(AUDIO_INFO);
			}
			else
			{
				DBG_ErrMsg("AUDIO_IN_ANALOG is out\n");
				return -1;
			}
		}
		else if(port == AUDIO_IN_DANTE)
		{
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				g_audio_info.source = (AudioInputMode_E)port;
				Cfg_Update(AUDIO_INFO);
			}
			else
			{
				DBG_ErrMsg("!!!This is not switcher\n");
				return -1;
			}
		}
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_Autoswitch_Source type:%d\n",type);
	}
	return 0;
}

int Cfg_Get_Autoswitch_Source(SignalType_E type,int* port)
{
	DBG_InfoMsg("Cfg_Get_Autoswitch_Source\n");
	if(type == SIGNAL_VIDEO)
	{
		 *port = g_autoswitch_info.source;
	}
	else if(type == SIGNAL_AUDIO)
	{
		 *port = (int)(g_audio_info.source);
	}
	else
	{
		DBG_ErrMsg("Cfg_Get_Autoswitch_Source type:%d\n",type);
	}
	return 0;
}
int Cfg_Set_Audio_Dest(int count, PortSignalType_E* port)
{
	DBG_InfoMsg("Cfg_Set_Audio_Dest\n");

	for(int i = 0; i < 4; i++)
	{
		g_audio_info.dst_port[i] = PORT_NONE;
	}

	if(count > 4)
		count = 4;

	for(int i = 0; i < count; i++)
	{
		if(port[i] == PORT_NONE)
			break;
		else if((port[i] == PORT_DANTE)&&(strcmp(g_version_info.model,IPE_P_MODULE) != 0))
		{
			break;
		}
		else if((port[i] == PORT_ANALOG_AUDIO)&&(g_audio_info.direction == DIRECTION_IN))
		{
			break;
		}
		else
			g_audio_info.dst_port[i] = port[i];
	}

	Cfg_Update(AUDIO_INFO);

	return 0;
}

int Cfg_Get_Audio_Dest(int* count, PortSignalType_E* port)
{
	DBG_InfoMsg("Cfg_Get_Audio_Dest\n");

	return 0;
}

int Cfg_Set_Video_Res(int mode,int res)
{
	DBG_InfoMsg("Cfg_Set_Video_Res mode:%d,res:%d\n",mode,res);
	g_video_info.scale_mode = mode;
	g_video_info.res_type = res;
	Cfg_Update(VIDEO_INFO);

	return 0;
}

int Cfg_Set_Video_RGB(int mode)
{
	DBG_InfoMsg("Cfg_Set_Video_RGB\n");
	g_video_info.force_rgb = mode;
	Cfg_Update(VIDEO_INFO);

	return 0;
}

int Cfg_Get_Video_RGB(int* mode)
{
	DBG_InfoMsg("Cfg_Get_Video_RGB\n");
	*mode = g_video_info.force_rgb;
	return 0;
}

int Cfg_Set_AV_Volume(int vol)
{
	DBG_InfoMsg("Cfg_Set_AV_Volume\n");
	g_avsetting_info.volume = vol;
	Cfg_Update(AV_SETTING);

	return 0;
}
int Cfg_Get_AV_Volume(int* vol)
{
	DBG_InfoMsg("Cfg_Get_AV_Volume\n");
	*vol = g_avsetting_info.volume;
	return 0;
}

int Cfg_Set_AV_Mute(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_AV_Mute\n");
	g_avsetting_info.mute_mode = mode;
	Cfg_Update(AV_SETTING);
	return 0;
}

int Cfg_Get_AV_Mute(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_AV_Mute\n");
	*mode = g_avsetting_info.mute_mode;
	return 0;
}

int Cfg_Set_AV_Action(CodecActionType_E mode)
{
	DBG_InfoMsg("Cfg_Set_AV_Action\n");
	g_avsetting_info.action = mode;
	Cfg_Update(AV_SETTING);
	return 0;
}

int Cfg_Get_AV_Action(CodecActionType_E* mode)
{
	DBG_InfoMsg("Cfg_Get_AV_Action\n");
	*mode = g_avsetting_info.action;
	return 0;
}

int Cfg_Set_AV_HDCP(int port, State_E mode)
{
	DBG_InfoMsg("Cfg_Set_AV_HDCP\n");

	if((port >= 1)&&(port <=3))
	{
		g_avsetting_info.hdcp_mode[port-1] = mode;
		Cfg_Update(AV_SETTING);
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_AV_HDCP %d\n",port);
	}
	return 0;
}

int Cfg_Get_AV_HDCP(int port, State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_AV_HDCP\n");

	if((port >= 1)&&(port <=3))
	{
		*mode = g_avsetting_info.hdcp_mode[port-1];
	}
	else
	{
		*mode = g_avsetting_info.hdcp_mode[0];
		DBG_ErrMsg("Cfg_Get_AV_HDCP %d\n",port);
	}
	return 0;
}

int Cfg_Set_EDID_Lock(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_EDID_Lock\n");

	g_edid_info.lock_mode = mode;
	Cfg_Update(EDID_INFO);

	return 0;
}
int Cfg_Get_EDID_Lock(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_EDID_Lock\n");

	*mode = g_edid_info.lock_mode;

	return 0;
}
int Cfg_Set_EDID_NetSrc(char* src)
{
	DBG_InfoMsg("Cfg_Set_EDID_NetSrc\n");

	strcpy(g_edid_info.net_src,src);
	Cfg_Update(EDID_INFO);

	return 0;
}
int Cfg_Get_EDID_NetSrc(char* src)
{
	DBG_InfoMsg("Cfg_Get_EDID_NetSrc\n");

	strcpy(src,g_edid_info.net_src);

	return 0;
}
int Cfg_Set_EDID_Mode(EdidModeType_E mode, int idx)
{
	DBG_InfoMsg("Cfg_Set_EDID_Mode\n");

	g_edid_info.edid_mode = mode;

	if(mode == CUSTOM)
		g_edid_info.active_id = idx;

	Cfg_Update(EDID_INFO);

	return 0;
}
int Cfg_Get_EDID_Mode(EdidModeType_E* mode, int* idx)
{
	DBG_InfoMsg("Cfg_Get_EDID_Mode\n");

	*mode = g_edid_info.edid_mode;

	if(g_edid_info.edid_mode == CUSTOM)
		*idx = g_edid_info.active_id;
	return 0;
}

int Cfg_Set_EDID_Active(int idx)
{
	DBG_InfoMsg("Cfg_Set_EDID_Active\n");

	if(g_edid_info.edid_mode == CUSTOM)
		g_edid_info.active_id = idx;

	Cfg_Update(EDID_INFO);

	return 0;
}
int Cfg_Get_EDID_Active(int* idx)
{
	DBG_InfoMsg("Cfg_Get_EDID_Active\n");

	*idx = g_edid_info.active_id;

	return 0;
}

int Cfg_Set_Dev_HostName(int id, char* name)
{
	DBG_InfoMsg("Cfg_Set_Dev_HostName\n");

	if(id == 0)//hostname
	{
		if(strlen(name) <= 31)
			strcpy(g_device_info.hostname,name);
		else
			memcpy(g_device_info.hostname,name,31);

		Cfg_Update(DEVICE_INFO);
	}
	else if(id == 1)//dante name
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(strlen(name) <= 31)
				strcpy(g_audio_info.dante_name,name);
			else
				memcpy(g_audio_info.dante_name,name,31);

			Cfg_Update(AUDIO_INFO);
		}
		else
		{
			DBG_ErrMsg("This is not switcher\n");
		}
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_Dev_HostName %d\n",id);
	}
	return 0;
}

int Cfg_Get_Dev_HostName(int id, char* name)
{
	DBG_InfoMsg("Cfg_Get_Dev_HostName\n");

	if(id == 0)//hostname
	{
		strcpy(name,g_device_info.hostname);
	}
	else if(id == 1)//dante name
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			strcpy(name,g_audio_info.dante_name);
		else
			DBG_ErrMsg("This is not switcher \n");
	}
	else
	{
		DBG_ErrMsg("Cfg_Get_Dev_HostName %d\n",id);
	}
	return 0;
}

int Cfg_Set_Dev_StandbyTime(int time)
{
	DBG_InfoMsg("Cfg_Set_Dev_StandbyTime\n");

	g_device_info.standby_time = time;
	Cfg_Update(DEVICE_INFO);

	return 0;
}
int Cfg_Get_Dev_StandbyTime(int* time)
{
	DBG_InfoMsg("Cfg_Get_Dev_StandbyTime\n");
	*time = g_device_info.standby_time;

	return 0;
}
int Cfg_Set_Dev_FPLock(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_Dev_FPLock\n");

	g_device_info.fp_lock = mode;
	Cfg_Update(DEVICE_INFO);

	return 0;
}
int Cfg_Get_Dev_FPLock(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_Dev_FPLock\n");
	*mode = g_device_info.fp_lock;
	return 0;
}

int Cfg_Set_UPG_Info(char* time, char* standby)
{
	DBG_InfoMsg("Cfg_Set_UPG_Info\n");
	strcpy(g_version_info.upg_time,time);
	strcpy(g_version_info.standby_version,standby);
	Cfg_Update(VERSION_INFO);
	return 0;
}
int Cfg_Get_UPG_Info(char* time, char* standby)
{
	DBG_InfoMsg("Cfg_Get_UPG_Info\n");
	return 0;
}

int Cfg_Set_Time_Loc(int utc_off, int state)
{
	DBG_InfoMsg("Cfg_Set_Time_Loc\n");
	g_time_info.time_zone = utc_off;
	g_time_info.daylight_saving = state;
	Cfg_Update(TIME_INFO);
	return 0;
}

int Cfg_Get_Time_Loc(int* utc_off, int* state)
{
	DBG_InfoMsg("Cfg_Get_Time_Loc\n");
	*utc_off = g_time_info.time_zone;
	*state = g_time_info.daylight_saving;
	return 0;
}
int Cfg_Set_Time_Srv(TimeSyncConf_S conf)
{
	DBG_InfoMsg("Cfg_Set_Time_Srv\n");
	g_time_info.ntp_mode = (State_E)(conf.enable);

	if(strlen(conf.serverIp) < 32)
		strcpy(g_time_info.ntp_server, conf.serverIp);

	g_time_info.ntp_sync_hour = conf.syncInerval;
	Cfg_Update(TIME_INFO);

	return 0;
}
int Cfg_Get_Time_Srv(TimeSyncConf_S* conf)
{
	DBG_InfoMsg("Cfg_Get_Time_Srv\n");

	conf->enable = (int)(g_time_info.ntp_mode);
	conf->syncInerval = g_time_info.ntp_sync_hour;

	strcpy(conf->serverIp,g_time_info.ntp_server);

	return 0;
}

int Cfg_Set_User_Pass(int level, char* pass)
{
	DBG_InfoMsg("Cfg_Set_User_Secur\n");
	g_user_info.user_role = level;

	if(strlen(pass) < 32)
		strcpy(g_user_info.password,pass);
	else
		DBG_ErrMsg("Cfg_Set_User_Secur pass too long!!!\n");

	Cfg_Update(USER_INFO);
	return 0;
}

int Cfg_Get_User_Pass(int level, char* pass)
{
	DBG_InfoMsg("Cfg_Get_User_Pass\n");
	if(g_user_info.user_role == level)
		strcpy(pass,g_user_info.password);
	else
		DBG_ErrMsg("Cfg_Set_User_Secur level:%d is wrong!!!\n",level);

	return 0;
}

int Cfg_Set_User_Secur(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_User_Secur\n");
	g_user_info.seurity_status = mode;

	Cfg_Update(USER_INFO);
	return 0;
}
int Cfg_Get_User_Secur(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_User_Secur\n");
	*mode = g_user_info.seurity_status;

	return 0;
}
int Cfg_Set_User_LogoutTime(int time)
{
	DBG_InfoMsg("Cfg_Set_User_LogoutTime\n");
	g_user_info.logout_time = time;

	Cfg_Update(USER_INFO);
	return 0;
}
int Cfg_Get_User_LogoutTime(int* time)
{
	DBG_InfoMsg("Cfg_Get_User_LogoutTime\n");
	*time = g_user_info.logout_time;

	return 0;
}

int Cfg_Set_VM_Mod(int horiziontal,int vertical)
{
	DBG_InfoMsg("Cfg_Set_VM_Mod\n");
	g_videowall_info.horizontal_count = horiziontal;
	g_videowall_info.vertical_count = vertical;

	Cfg_Update(VIDEOWALL_INFO);
	return 0;
}
int Cfg_Get_VM_Mod(int* horiziontal,int* vertical)
{
	DBG_InfoMsg("Cfg_Get_VM_Mod\n");

	*horiziontal = g_videowall_info.horizontal_count;
	*vertical = g_videowall_info.vertical_count;

	return 0;
}
int Cfg_Set_VM_Setup(int position,int rotation)
{
	DBG_InfoMsg("Cfg_Set_VM_Setup\n");
	g_videowall_info.relative_position = position;
	g_videowall_info.rotation = rotation;

	Cfg_Update(VIDEOWALL_INFO);

	return 0;
}
int Cfg_Get_VM_Setup(int* position,int* rotation)
{
	DBG_InfoMsg("Cfg_Get_VM_Setup\n");

	*position = g_videowall_info.relative_position;
	*rotation = g_videowall_info.rotation;

	return 0;
}
int Cfg_Set_VM_Bezel(int position,WndBezelinfo_S cfg)
{
	DBG_InfoMsg("Cfg_Set_VM_Bezel\n");

	if(position == g_videowall_info.relative_position)
	{
		g_videowall_info.bezel_horizontal_value = cfg.hValue;
		g_videowall_info.bezel_horizontal_offset = cfg.hOffset;
		g_videowall_info.bezel_vertical_value = cfg.vValue;
		g_videowall_info.bezel_vertical_offset = cfg.vOffset;

		Cfg_Update(VIDEOWALL_INFO);
	}
	else
	{
		DBG_ErrMsg("Cfg_Set_VM_Bezel position:%d is Wrong!!!\n",position);
	}

	return 0;
}
int Cfg_Get_VM_Bezel(int* position,WndBezelinfo_S* cfg)
{
	DBG_InfoMsg("Cfg_Get_VM_Bezel\n");

	*position = g_videowall_info.relative_position;
	cfg->hValue = g_videowall_info.bezel_horizontal_value;
	cfg->hOffset = g_videowall_info.bezel_horizontal_offset;
	cfg->vValue = g_videowall_info.bezel_vertical_value;
	cfg->vOffset = g_videowall_info.bezel_vertical_offset;

	return 0;
}

int Cfg_Set_VM_Stretch(int mode)
{
	DBG_InfoMsg("Cfg_Set_VM_Stretch\n");
	g_videowall_info.stretch_type = mode;

	Cfg_Update(VIDEOWALL_INFO);
	return 0;
}
int Cfg_Get_VM_Stretch(int* mode)
{
	DBG_InfoMsg("Cfg_Get_VM_Stretch\n");
	*mode = g_videowall_info.stretch_type;

	return 0;
}

int Cfg_Set_Log_Action(int mode, int period)
{
	DBG_InfoMsg("Cfg_Set_Log_Action\n");
	g_log_info.active = mode;
	g_log_info.period = period;

	Cfg_Update(LOG_INFO);
	return 0;
}
int Cfg_Get_Log_Action(int* mode, int* period)
{
	DBG_InfoMsg("Cfg_Get_Log_Action\n");
	*mode = g_log_info.active;
	*period = g_log_info.period;
	return 0;
}

int Cfg_Set_GW_CEC_Mode(int mode)
{
	DBG_InfoMsg("Cfg_Set_GW_CEC_Mode\n");

	if(mode == 0)
	{
		g_gateway_info.cec_mode = OFF;
		g_gateway_info.cec_output = mode;
	}
	else
	{
		g_gateway_info.cec_mode = ON;
		g_gateway_info.cec_output = mode;
	}

	Cfg_Update(GATEWAY_INFO);
	return 0;
}
int Cfg_Get_GW_CEC_Mode(int* mode)
{
	DBG_InfoMsg("Cfg_Get_GW_CEC_Mode\n");
	*mode = g_gateway_info.cec_output;
	return 0;
}
int Cfg_Set_GW_Uart_Param(UartMessageInfo_S param)
{
	DBG_InfoMsg("Cfg_Set_GW_Uart_Param\n");

	memcpy(&(g_gateway_info.rs232_param),&param,sizeof(UartMessageInfo_S));
	Cfg_Update(GATEWAY_INFO);
	return 0;
}
int Cfg_Get_GW_Uart_Param(UartMessageInfo_S* param)
{
	DBG_InfoMsg("Cfg_Get_GW_Uart_Param\n");

	memcpy(param,&(g_gateway_info.rs232_param),sizeof(UartMessageInfo_S));
	return 0;
}
int Cfg_Set_GW_COM_Add(int port)
{
	DBG_InfoMsg("Cfg_Set_GW_COM_Add\n");

	g_gateway_info.rs232_mode = ON;
	g_gateway_info.rs232_port = port;
	Cfg_Update(GATEWAY_INFO);

	return 0;
}
int Cfg_Set_GW_COM_Remove()
{
	DBG_InfoMsg("Cfg_Set_GW_COM_Remove\n");

	g_gateway_info.rs232_mode = OFF;
	g_gateway_info.rs232_port = 0;
	Cfg_Update(GATEWAY_INFO);

	return 0;
}
int Cfg_Get_GW_COM_Status(State_E* mode, int* port)
{
	DBG_InfoMsg("Cfg_Get_GW_COM_Remove\n");

	*mode = g_gateway_info.rs232_mode;
	*port = g_gateway_info.rs232_port;

	return 0;
}
int Cfg_Set_GW_IR_Mode(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_GW_IR_Mode\n");
	g_gateway_info.ir_mode = mode;

	Cfg_Update(GATEWAY_INFO);
	return 0;
}
int Cfg_Get_GW_IR_Mode(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_GW_IR_Mode\n");
	*mode = g_gateway_info.ir_mode;

	return 0;
}

int Cfg_Set_Net_Config(int netId,NetWorkInfo_S*netInfo)
{
	DBG_InfoMsg("Cfg_Set_Net_Config\n");

	if((netId == 0)||(netId == 1))
	{
		memcpy(&(g_network_info.eth_info[netId]),netInfo,sizeof(NetWorkInfo_S));
		Cfg_Update(NETWORK_INFO);
	}
	else
	{
		DBG_WarnMsg("Cfg_Set_Net_Config netId:%d is Wrong!!!\n",netId);
	}
	return 0;
}

int Cfg_Get_Net_Config(int netId,NetWorkInfo_S*netInfo)
{
	DBG_InfoMsg("Cfg_Get_Net_Config\n");

	if((netId == 0)||(netId == 1))
	{
		memcpy(netInfo,&(g_network_info.eth_info[netId]),sizeof(NetWorkInfo_S));
	}
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_Config netId:%d is Wrong!!!\n",netId);
	}
	return 0;
}

int Cfg_Set_Net_DHCP(int netId,int dhcp)
{
	DBG_InfoMsg("Cfg_Set_Net_DHCP\n");

	if((netId == 0)||(netId == 1))
	{
		g_network_info.eth_info[netId].dhcp_enable = dhcp;
		Cfg_Update(NETWORK_INFO);
	}
	else
	{
		DBG_WarnMsg("Cfg_Set_Net_DHCP netId:%d is Wrong!!!\n",netId);
	}

	return 0;
}
int Cfg_Get_Net_DHCP(int netId,int* dhcp)
{
	DBG_InfoMsg("Cfg_Get_Net_DHCP\n");

	if((netId == 0)||(netId == 1))
	{
		*dhcp = g_network_info.eth_info[netId].dhcp_enable;
	}
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_DHCP netId:%d is Wrong!!!\n",netId);
	}

	return 0;
}
int Cfg_Set_Net_DaisyChain(State_E mode)
{
	DBG_InfoMsg("Cfg_Set_Net_DaisyChain\n");

	g_network_info.daisy_chain = mode;
	Cfg_Update(NETWORK_INFO);

	return 0;
}

int Cfg_Get_Net_DaisyChain(State_E* mode)
{
	DBG_InfoMsg("Cfg_Get_Net_DaisyChain\n");
	*mode = g_network_info.daisy_chain;

	return 0;
}

int Cfg_Set_Net_Port(NetPortType_E type, int port) //type:udp;tcp
{
	DBG_InfoMsg("Cfg_Set_Net_Port\n");

	if(type == Net_UDP)//udp
		g_network_info.udp_port = port;
	else if(type == Net_TCP)
		g_network_info.tcp_port = port;
	else
	{
		DBG_WarnMsg("Cfg_Set_Net_Port type:%d is Wrong!!!\n",type);
		return -1;
	}

	Cfg_Update(NETWORK_INFO);

	return 0;
}
int Cfg_Get_Net_Port(NetPortType_E type, int* port)
{
	DBG_InfoMsg("Cfg_Get_Net_Port\n");

	if(type == Net_UDP)//udp
		*port = g_network_info.udp_port;
	else if(type == Net_TCP)
		*port = g_network_info.tcp_port;
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_Port type:%d is Wrong!!!\n",type);
		return -1;
	}

	return 0;
}
int Cfg_Set_Net_Method(NetMethodType_E type) //1:uni;2 multi
{
	DBG_InfoMsg("Cfg_Set_Net_Method\n");

	g_network_info.method = type;
	Cfg_Update(NETWORK_INFO);

	//astparam multicast_on;

	return 0;
}
int Cfg_Get_Net_Method(NetMethodType_E* type)
{
	DBG_InfoMsg("Cfg_Get_Net_Method\n");

	*type = g_network_info.method;
	return 0;
}
int Cfg_Set_Net_Multicast(char * ip,int ttl) //type:udp;tcp
{
	DBG_InfoMsg("Cfg_Set_Net_Method\n");
	if(g_network_info.method == Net_MULTICAST)
	{
		g_network_info.multicast_ttl = ttl;
		strcpy(g_network_info.multicast_ip,ip);
		Cfg_Update(NETWORK_INFO);
	}
	else
		DBG_WarnMsg("Cfg_Set_Net_Multicast method:%d is Wrong!!!\n",g_network_info.method);

	return 0;
}
int Cfg_Get_Net_Multicast(char * ip,int* ttl)
{
	DBG_InfoMsg("Cfg_Set_Net_Method\n");

	*ttl = g_network_info.multicast_ttl;
	strcpy(ip,"0.0.0.0");

	return 0;
}
int Cfg_Set_Net_GW_Port(NetGWType_E type, int port) //type:p3k,rs232,dante
{
	DBG_InfoMsg("Cfg_Set_Net_GW_Port\n");
	if(type == Net_CONTROL)
		g_network_info.control_port = port;
	else if(type == Net_DANTE) //Dante
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			g_network_info.dante_port = port;
		}
		else
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return -1;
		}
	}
	else
	{
		DBG_WarnMsg("Cfg_Set_Net_GW_Port type:%d is Wrong!!!\n",type);
		return -1;
	}

	Cfg_Update(NETWORK_INFO);
	return 0;
}
int Cfg_Get_Net_GW_Port(NetGWType_E type, int* port)
{
	DBG_InfoMsg("Cfg_Get_Net_GW_Port\n");
	if(type == Net_CONTROL) //p3k
		 *port = g_network_info.control_port;
	else if(type == Net_DANTE) //Dante
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			*port = g_network_info.dante_port;
		}
		else
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return -1;
		}
	}
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_GW_Port type:%d is Wrong!!!\n",type);
		return -1;
	}

	return 0;
}
int Cfg_Set_Net_GW_Vlan(NetGWType_E type, int vlan) //type:p3k,rs232,dante
{
	DBG_InfoMsg("Cfg_Set_Net_GW_Vlan\n");
	if(type == Net_CONTROL) //p3k
		g_network_info.control_vlan = vlan;
	else if(type == Net_DANTE) //Dante
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			g_network_info.dante_vlan = vlan;
		}
		else
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return -1;
		}
	}
	else
	{
		DBG_WarnMsg("Cfg_Set_Net_GW_Vlan type:%d is Wrong!!!\n",type);
		return -1;
	}

	Cfg_Update(NETWORK_INFO);
	return 0;
}
int Cfg_Get_Net_GW_Vlan(NetGWType_E type, int* vlan)
{
	DBG_InfoMsg("Cfg_Get_Net_GW_Vlan\n");
	if(type == Net_CONTROL) //p3k
		*vlan = g_network_info.control_vlan;
	else if(type == Net_DANTE) //Dante
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			*vlan = g_network_info.dante_vlan;
		}
		else
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return -1;
		}
	}
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_GW_Vlan type:%d is Wrong!!!\n",type);
		return -1;
	}
	return 0;
}

int Cfg_Set_OSD_Diaplay(State_E mode)
{
	g_osd_enable = mode;
	Cfg_Update_OSD();
	return 0;
}
int Cfg_Get_OSD_Diaplay(State_E* mode)
{
	return 0;
}

int Cfg_Get_EDID_List(char info[][MAX_EDID_LEN],int num)
{
	DBG_InfoMsg("Cfg_Get_EDID_List\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_WarnMsg("This is Decoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_LIST_FILE);
	sprintf(g_edid_info.EDID_List[0],"default.bin");
	sprintf(info[0],"[0,\"default.bin\"]");

	for(int i=1;i<MAX_EDID;i++)
	{
		memset(g_edid_info.EDID_List[i],0,32);
	}

	//Read	Log cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	int ncount = 1;
	fread(pBuf,1,sizeof(pBuf),fp);
	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_EDID_LIST].empty())
		{
			Json::Value& root = root1[JSON_EDID_LIST];
			for(int i=1;i<MAX_EDID;i++)
			{
				char idx[4];
				sprintf(idx,"%d",i);
				if(!root[idx].empty())
				{
					string name = root[idx].asString();
					if(name.size()> 0)
					{
						sprintf(g_edid_info.EDID_List[i],"%s",name.c_str());
						sprintf(info[ncount],"[%d,\"%s\"]",i,name.c_str());

						printf("Cfg_Get_EDID_List info[%d]: %s\n",ncount,info[ncount]);
						ncount++;
					}
					else
						break;
				}
			}
		}
	}

	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return ncount;
}

int Cfg_Set_Enc_AVSignal_Info()
{
	DBG_InfoMsg("Cfg_Set_Enc_AVSignal_Info\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);


	//Read	avsignal cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SIGNAL].empty())
		{
			Json::Value& root = root1[JSON_AV_SIGNAL];

#ifdef CONFIG_P3K_HOST
			char cmd[256] = "";

			if(!root[JSON_AV_MAX_BITRATE].empty())
			{
				string bitrate =  root[JSON_AV_MAX_BITRATE].asString();

				if(bitrate == JSON_AV_BEST_EFFORT)
				{
					sprintf(cmd,"astparam s profile auto;astparam save");
					system(cmd);
				}
				else if((bitrate == "10")||(bitrate == "50")||(bitrate == "100")||(bitrate == "150")||(bitrate == "200"))
				{
					sprintf(cmd,"astparam s profile %sM;astparam save",bitrate.c_str());
					system(cmd);
				}
				else
				{
					DBG_WarnMsg("JSON_AV_MAX_BITRATE Param: %s  Error!!!\n",bitrate.c_str());
				}

			}

			if(!root[JSON_AV_FRAME_RATE].empty())
			{
				if(root[JSON_AV_FRAME_RATE].isInt())
				{
					memset(cmd,0,256);
					int frame = root[JSON_AV_FRAME_RATE].asInt();

					if((frame <= 100)&&(frame >= 0))
					{
						int param = frame * 60 / 100;
						sprintf(cmd,"astparam s v_frame_rate %d;astparam save",param);
						system(cmd);
					}
					else
					{
						DBG_WarnMsg("JSON_AV_FRAME_RATE Param: %d  Error!!!\n",frame);
					}
				}
			}

			if(!root[JSON_AV_AUD_GUARD].empty())
			{
				if(root[JSON_AV_AUD_GUARD].isInt())
				{
					int guard = root[JSON_AV_AUD_GUARD].asInt();

					if((guard >= 0)&&(guard <= 90))
					{
						if(guard != g_audio_info.conn_guard_time)
						{
							memset(cmd,0,256);
							sprintf(cmd,"e_audio_detect_time::%d",guard);

							ast_send_event(0xFFFFFFFF,cmd);

							DBG_InfoMsg("cmd : %s\n",cmd);
							g_audio_info.conn_guard_time = guard;
						}
						else
						{
							DBG_WarnMsg("JSON_AV_AUD_GUARD Param: %d  is not change!!!\n",guard);
						}
					}
					else
					{
						DBG_WarnMsg("JSON_AV_AUD_GUARD Param: %d  Error!!!\n",guard);
					}
				}
			}
#endif
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);
	return 0;
}

int Cfg_Set_Dec_Usb_KVM()
{
	DBG_InfoMsg("Cfg_Set_Dec_Usb_KVM\n");

#ifdef CONFIG_P3K_HOST
		DBG_WarnMsg("This is Encoder\n");
		return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,KVM_FILE);


	//Read	kvm cfg
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_USB_KVM_CONFIG].empty())
		{
			Json::Value& root = root1[JSON_USB_KVM_CONFIG];

			if(!root[JSON_USB_KVM_MODE].empty())
			{
				char cmd[256] = "";
				string mode =  root[JSON_USB_KVM_MODE].asString();

				if(mode == JSON_USB_KVM_KM)
				{
					sprintf(cmd,"astparam s no_kmoip n;astparam save");
					system(cmd);
				}
				else if(mode == JSON_USB_KVM_USB)
				{
					sprintf(cmd,"astparam s no_kmoip y;astparam save");
					system(cmd);
				}

				DBG_InfoMsg("%s\n",cmd);
			}

			if(!root[JSON_USB_KVM_ROAMING].empty())
			{
				Json::Value& JsonKVMArray = root[JSON_USB_KVM_ROAMING];
				printf("JsonKVMArray.size() = %d\n",JsonKVMArray.size());

				char cmd[512] = "";
				char param[512] = "";

				for(unsigned int i = 0; i < JsonKVMArray.size(); i++)
				{
					Json::Value& JsonKVM = JsonKVMArray[i];

					string str_mac,str_x,str_y;
					if(!JsonKVM[JSON_USB_KVM_MAC].empty())
					{
						string str_mac = JsonKVM[JSON_USB_KVM_MAC].asString();
						if(str_mac.size() == 0)
						{
							printf("str_mac = no string \n");
							continue;
						}
					}

					if(!JsonKVM[JSON_USB_KVM_H].empty())
					{
						string str_x = JsonKVM[JSON_USB_KVM_H].asString();
						if(str_x.size() == 0)
						{
							printf("str_x = no string \n");
							continue;
						}
					}

					if(!JsonKVM[JSON_USB_KVM_V].empty())
					{
						string str_y = JsonKVM[JSON_USB_KVM_V].asString();
						if(str_y.size() == 0)
						{
							printf("str_y = no string \n");
							continue;
						}
					}

					if((!JsonKVM[JSON_USB_KVM_MAC].empty())&&(!JsonKVM[JSON_USB_KVM_H].empty())&&(!JsonKVM[JSON_USB_KVM_V].empty()))
					{
						string mac = JsonKVM[JSON_USB_KVM_MAC].asString();

						if((JsonKVM[JSON_USB_KVM_H].isInt())&&(JsonKVM[JSON_USB_KVM_V].isInt()))
						{
							int    x   = JsonKVM[JSON_USB_KVM_H].asInt();
							int    y   = JsonKVM[JSON_USB_KVM_V].asInt();

							printf("JsonKVMArray[%d]:[mac: %s][x: %d][y: %d]\n",i,mac.c_str(),x,y);

							if(mac.size()==12)
							{
								if(strlen(param) == 0)
									sprintf(param,"%s,%d,%d",mac.c_str(),x,y);
								else
									sprintf(param,"%s:%s,%d,%d",param,mac.c_str(),x,y);
							}
						}
					}


				}

				if(strlen(param) > 0)
				{
					sprintf(cmd,"astparam s kmoip_roaming_layout %s;astparam save",param);
					system(cmd);
				}

				DBG_InfoMsg("%s\n",cmd);
			}
			else
			{
				system("astparam s kmoip_roaming_layout;astparam save;e e_kmoip_roaming_chg");
			}

			if(!root[JSON_USB_KVM_TIMEOUT].empty())
			{
				if(root[JSON_USB_KVM_TIMEOUT].isInt())
				{
					char cmd[256] = "";
					int interval =  root[JSON_USB_KVM_TIMEOUT].asInt();

					if((interval <= 10)&&(interval >= 0))
					{
						sprintf(cmd,"astparam s kmoip_token_interval %d;astparam save",interval*1000);
						system(cmd);
					}

					DBG_InfoMsg("%s\n",cmd);
				}
			}
		}
	}

	flock(fileno(fp), LOCK_UN);
	fclose(fp);
	return 0;
}

int Cfg_Init_Param()
{
	DBG_InfoMsg("Cfg_Init_Param\n");

	return 0;

	//fp_lock
	if(g_device_info.fp_lock == 0)
		ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_off");
	else
		ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_on");

	//ir_dir
	if(g_gateway_info.ir_direction == DIRECTION_IN)
		ast_send_event(0xFFFFFFFF,"e_p3k_ir_dir::in");
	else
		ast_send_event(0xFFFFFFFF,"e_p3k_ir_dir::out");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	//DECODER SOURCE
	if(g_autoswitch_info.source == 1)
		ast_send_event(0xFFFFFFFF,"e_p3k_switch_in::HDMI");
	else
		ast_send_event(0xFFFFFFFF,"e_p3k_switch_in::STREAM");

#else
	DBG_InfoMsg("This is Encoder\n");
	//HDCP
//	char module[32]="";
	int ncount = 1;
//	GetBoardInfo(BOARD_MODEL, module, 32);
	if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		ncount = 3;
	else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		ncount = 2;

	for(int i = 1;i <= ncount;i++)
		EX_SetHDCPMode(i,(HDCPMode_E)(g_avsetting_info.hdcp_mode[i-1]));


	//EDID
	char sCmd[64] = "";
	if(g_edid_info.edid_mode == PASSTHRU)
		sprintf(sCmd,"e_p3k_video_edid_passthru::%s",g_edid_info.net_src);
	else if(g_edid_info.edid_mode == CUSTOM)
		sprintf(sCmd,"e_p3k_video_edid_custom::%d",g_edid_info.active_id);
	else
		sprintf(sCmd,"e_p3k_video_edid_default");

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

#endif


	return 0;
}

int Cfg_Set_Switch_Delay()
{
	DBG_InfoMsg("Cfg_Set_Switch_Delay\n");

#ifdef CONFIG_P3K_CLIENT
		DBG_WarnMsg("This is Decoder\n");
		return 0;
#endif

	if(strcmp(g_version_info.model,IPE_MODULE) == 0)
	{
		DBG_WarnMsg("This is Switcher\n");
		return 0;
	}

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUTOSWITCH_DELAY_FILE);


	//Read	kvm cfg
	int plugInTime=0, plugOutTime=0, signalLossTime=10, manualOverrideTime=10;
	Json::Reader reader;
	Json::Value root1;
	char pBuf[1024] = "";
	FILE *fp;
	fp = fopen(path, "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	if (flock(fileno(fp), LOCK_EX)) {
		DBG_ErrMsg("%s lock failed\n",path);
		fclose(fp);
		return -1;
	}

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AUTOSWITCH_DELAY].empty())
		{
			Json::Value& root = root1[JSON_AUTOSWITCH_DELAY];

			if(!root[JSON_AUTOSWITCH_LOSS].empty())
			{
				if(root[JSON_AUTOSWITCH_LOSS].isInt())
					signalLossTime =  root[JSON_AUTOSWITCH_LOSS].asInt();
			}

			if(!root[JSON_AUTOSWITCH_UNPLUG].empty())
			{
				if(root[JSON_AUTOSWITCH_UNPLUG].isInt())
					plugOutTime =  root[JSON_AUTOSWITCH_UNPLUG].asInt();
			}

			if(!root[JSON_AUTOSWITCH_PLUGIN].empty())
			{
				if(root[JSON_AUTOSWITCH_PLUGIN].isInt())
					plugInTime =  root[JSON_AUTOSWITCH_PLUGIN].asInt();
			}

			if(!root[JSON_AUTOSWITCH_OVERRIDE].empty())
			{
				if(root[JSON_AUTOSWITCH_OVERRIDE].isInt())
					manualOverrideTime =  root[JSON_AUTOSWITCH_OVERRIDE].asInt();
			}
		}
	}

	flock(fileno(fp), LOCK_UN);

	fclose(fp);

	char cmd[64] = "";
	sprintf(cmd,"sconfig --delay-time %d %d %d %d",plugInTime, plugOutTime, signalLossTime, manualOverrideTime);

	system(cmd);

	DBG_InfoMsg("cmd: %s\n",cmd);
	return 0;
}

int Cfg_Set_Display_Sleep()
{
	char cmd[64] = "";
	DBG_InfoMsg("Cfg_Set_Display_Sleep\n");

#ifndef CONFIG_P3K_CLIENT
		DBG_WarnMsg("This is not Decoder\n");
		return 0;
#endif
	ast_send_event(0xFFFFFFFF, "e_display_sleep");

	return 0;
}

int get_local_ip(const char *eth_inf, char *ip)
{
	int i=0;
	int sockfd;
	struct ifconf ifconf;
	char buf[512];
	struct ifreq *ifreq;	  //初始化ifconf

//	printf("get_local_ip: %s\n", eth_inf);

	ifconf.ifc_len = 512;
	ifconf.ifc_buf = buf;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
	{
	  	perror("socket");
	  	sprintf(ip,"0.0.0.0");
		return -1;
	}

	ioctl(sockfd, SIOCGIFCONF, &ifconf);    //获取所有接口信息

	//接下来一个一个的获取IP地址
	ifreq = (struct ifreq*)buf;
	for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
	{
	  	if(ifreq->ifr_flags == AF_INET)
	  	{
			  //for ipv4
//			  printf("i = [%d] name = [%s]\n", i,ifreq->ifr_name);
//			  printf("local addr = [%s]\n",inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));

			  if(strstr(ifreq->ifr_name,eth_inf) != 0)
			  {
			  		sprintf(ip,inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
					close(sockfd);
					return 0;
			  }
			  ifreq++;
		}
	}

	sprintf(ip,"0.0.0.0");
	close(sockfd);

	return 0;
}


int GetIPInfo(int netId,char* ip_addr,char* ip_mask)
{
	char eth[16] = "";
	if(netId == 0)
	{
		sprintf(eth,"eth0");
	}
	else if(netId == 1)
	{
		if((g_init_control_vlan >=2)&&(g_init_control_vlan <=4093))
			sprintf(eth,"eth0.%d",g_init_control_vlan);
		else if(g_init_control_port == 0)
			sprintf(eth,"eth0");
		else if(g_init_control_port == 1)
			sprintf(eth,"eth0.4094");
	}
	else
	{
		DBG_ErrMsg("netId=%d \n",netId);
		return -1;
	}


	if(ip_mask == NULL)
	{
		get_local_ip(eth, ip_addr);
//		printf("GetIPInfo ip_addr = %s\n",ip_addr);
	}
	else
	{
		char ip_buf[32] = "";
		int  mask;
		char ip_cmd[128] = "";

		sprintf(ip_cmd,"ip addr show %s | grep 'inet' |sed 's/^.*inet //g'|sed 's#brd.*$##g'",eth);
		mysystem(ip_cmd, ip_buf, 32);

		if(strlen(ip_buf) < 9)
		{
//			printf("ip_buf :%s\n",ip_buf);
			sprintf(ip_addr,"0.0.0.0");

			if(ip_mask!= NULL)
				sprintf(ip_mask,"0.0.0.0");

			return 0;
		}

		int count = sscanf(ip_buf,"%[^/]/%d",ip_addr,&mask);
		unsigned int mask_code = 0;
		if(strlen(ip_addr) < 7)
		{
//			printf("ip_addr :%s\n",ip_addr);
			sprintf(ip_addr,"0.0.0.0");
		}

		//DBG_InfoMsg("ip_addr=%s \n",ip_addr);

		if((ip_mask!= NULL)&&(count == 2))
		{
			if((mask >= 8)&&(mask <= 32))
			{
				mask_code = (int)(pow(2,mask)-1);
				mask_code = mask_code<<(32 - mask);
				sprintf(ip_mask,"%d.%d.%d.%d",(mask_code&0xFF000000)>>24,(mask_code&0xFF0000)>>16,(mask_code&0xFF00)>>8,(mask_code&0xFF));
			}

			//DBG_InfoMsg("ip_mask=%s \n",ip_mask);
		}
	}

	return 0;
}

int UpdateLocalIP(char* ip_addr)
{
	char cmd1[64] = "";

	sprintf(cmd1,"astparam s soip_local_addr %s",ip_addr);
	system(cmd1);

	return 0;
}

int NotifyIPtoSOIP2(char* ip_addr)
{
	UpdateLocalIP(ip_addr);

	if(g_gateway_info.rs232_mode == ON)
	{
		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_soip_gw_on::%d",g_gateway_info.rs232_port);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}

	return 0;
}
