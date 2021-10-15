#include "json/json.h"
#include <iostream>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "cfgparser.h"
#include "funcexcute.h"
#include "ast_send_event.h"

#include "debugtool.h"

Channel_Info 		g_channel_info;
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

using namespace std;

char g_module[32] = "kds-7";

pthread_mutex_t g_cfg_lock;

int Cfg_InitModule(void)
{
	return 0;
	char module[32]="";
	GetBoardInfo(BOARD_MODEL, module, 32);

	if(strcmp(module,IPE_MODULE) == 0)
		strcpy(g_module,IPE_PATH);
	else if(strcmp(module,IPD_MODULE) == 0)
		strcpy(g_module,IPD_PATH);
	else if(strcmp(module,IPE_P_MODULE) == 0)
		strcpy(g_module,IPE_P_PATH);
	else if(strcmp(module,IPD_W_MODULE) == 0)
		strcpy(g_module,IPD_W_PATH);
	else if(strcmp(module,IPE_W_MODULE) == 0)
		strcpy(g_module,IPE_W_PATH);
	else
		strcpy(g_module,IPE_P_PATH);
	return 0;
}

int Cfg_Init(void)
{
	DBG_InfoMsg("Cfg_Init\n");
	pthread_mutex_init(&g_cfg_lock,NULL);
	//Cfg_InitModule();
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
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,CHANNEL_DEF_FILE);

	g_channel_info.channel_id = 0;
	strcpy(g_channel_info.channel_Name,"CH_00");

	//Check Channel cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_CHANNEL].empty())
		{
			Json::Value& root = root1[JSON_CHANNEL];

			//init g_channel_info
			if(!root[JSON_CH_ID].empty())
			{
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

	fclose(fp);

	return 0;
}

int Cfg_Init_Audio(void)
{
	DBG_InfoMsg("Cfg_Init_Audio\n");

#ifdef CONFIG_P3K_CLIENT
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AUDIO_FILE);

	g_audio_info.direction = DIRECTION_IN;
	g_audio_info.switch_mode = CONNECT_LAST;
	g_audio_info.input_pri[0] = AUDIO_IN_HDMI;
	g_audio_info.input_pri[1] = AUDIO_IN_ANALOG;
	g_audio_info.input_pri[2] = AUDIO_IN_DANTE;
	g_audio_info.dst_port[0] = PORT_NONE;
	g_audio_info.dst_port[1] = PORT_NONE;
	g_audio_info.dst_port[2] = PORT_NONE;
	g_audio_info.dst_port[3] = PORT_NONE;
	g_audio_info.source = AUDIO_IN_HDMI;
	sprintf(g_audio_info.dante_name,"dante");

	//Check autoswitch cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(AUDIO_INFO);
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

			if(!root[JSON_PRIORITY].empty())
			{
				Json::Value& JsonPriorityArray = root[JSON_PRIORITY];
				//printf("JsonPriorityArray.size() = %d\n",JsonPriorityArray.size());

				for(unsigned int i = 0; i < JsonPriorityArray.size(); i++)
				{
					string input = JsonPriorityArray[i].asString();
					//printf("JsonPriorityArray[%d]:[%s]\n",i,input.c_str());

					if(input == JSON_AUDIO_DANTE)
					{
						g_audio_info.input_pri[i] = AUDIO_IN_DANTE;
						continue;
					}
					else if(input == JSON_AUDIO_ANALOG)
					{
						g_audio_info.input_pri[i] = AUDIO_IN_ANALOG;
						continue;
					}
					else if(input == JSON_AUDIO_HDMI)
					{
						g_audio_info.input_pri[i] = AUDIO_IN_HDMI;
						continue;
					}
				}
			}

			if(!root[JSON_AUDIO_DEST].empty())
			{
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
							g_audio_info.dst_port[i] = PORT_DANTE;

						continue;
					}
					else if(output == JSON_AUDIO_ANALOG)
					{
						g_audio_info.dst_port[i] = PORT_ANALOG_AUDIO;
						continue;
					}
					else if(output == JSON_AUDIO_HDMI)
					{
						g_audio_info.dst_port[i] = PORT_HDMI;
						continue;
					}
					else if(output == JSON_AUDIO_LAN)
					{
						g_audio_info.dst_port[i] = PORT_STREAM;
						continue;
					}
					else
						break;
				}
			}

			if(!root[JSON_SOURCE_SELECT].empty())
			{
				string input = root[JSON_SOURCE_SELECT].asString();
				if(input == JSON_AUDIO_DANTE)
					g_audio_info.source = AUDIO_IN_DANTE;
				else if(input == JSON_AUDIO_ANALOG)
					g_audio_info.source = AUDIO_IN_ANALOG;
				else if(input == JSON_AUDIO_HDMI)
					g_audio_info.source = AUDIO_IN_HDMI;

			}

			if(!root[JSON_DANTE_NAME].empty())
			{
				string name = root[JSON_DANTE_NAME].asString();
				sprintf(g_audio_info.dante_name,name.c_str());
			}
		}
	}

	fclose(fp);
	return 0;
}

int Cfg_Init_Video(void)
{
	DBG_InfoMsg("Cfg_Init_Video\n");
	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);

	g_video_info.force_rgb = 1;

	//Check Video cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(VIDEO_INFO);
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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SIGNAL].empty())
		{
			Json::Value& root = root1[JSON_AV_SIGNAL];

			//init g_autoswitch_info
			if(!root[JSON_AV_RGB].empty())
			{
				string mode = root[JSON_AV_RGB].asString();
				//printf("g_video_info.force_rgb = %s\n",mode.c_str());

				if(mode == JSON_PARAM_OFF)
				{
					g_video_info.force_rgb = 0;
				}
				else
				{
					g_video_info.force_rgb = 1;
				}
			}

		}
	}

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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(AUTOSWITCH_INFO);
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

	g_avsetting_info.volume = 100;
	g_avsetting_info.mute_mode = OFF;
	g_avsetting_info.action = CODEC_ACTION_PLAY;
	g_avsetting_info.hdcp_mode[0] = ON;
	g_avsetting_info.hdcp_mode[1] = ON;
	g_avsetting_info.hdcp_mode[2] = ON;

	//Check autoswitch cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create autoswitch cfg from default value
		Cfg_Update(AV_SETTING);
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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_AV_SETTING].empty())
		{
			Json::Value& root = root1[JSON_AV_SETTING];

			//init g_avsetting_info
			if(!root[JSON_AV_VOLUME].empty())
				g_avsetting_info.volume = root[JSON_AV_VOLUME].asInt();

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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(EDID_INFO);
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
	GetBoardInfo(BOARD_HOSTNAME, g_device_info.hostname, 32);
	GetBoardInfo(BOARD_SN, g_device_info.sn, 32);

	//Check log cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

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
				g_device_info.standby_time = root[JSON_DEV_STANDBY_TIME].asInt();
			}
		}
	}

	fclose(fp);

	Cfg_Update(DEVICE_INFO);
	return 0;
}

int Cfg_Init_Version(void)
{
	DBG_InfoMsg("Cfg_Init_Version\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,VERSION_FILE);

	GetBoardInfo(BOARD_MODEL, g_version_info.model, 64);
	GetBoardInfo(BOARD_HW_VERSION, g_version_info.hw_version, 16);
	GetBoardInfo(BOARD_FW_VERSION, g_version_info.fw_version, 16);
	GetBoardInfo(BOARD_BUILD_DATE, g_version_info.build_time, 16);

	sprintf(g_version_info.file_version,"1.0.0");
	sprintf(g_version_info.standby_version,"1.0.0");
	sprintf(g_version_info.upg_time,"01-01-2020,00:00:00");


	//Check version cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(VERSION_INFO);
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(TIME_INFO);
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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_TIME_SETTING].empty())
		{
			Json::Value& root = root1[JSON_TIME_SETTING];

			if(!root[JSON_TIME_ZONE].empty())
				g_time_info.time_zone = root[JSON_TIME_ZONE].asInt();

			if(!root[JSON_TIME_DAYLIGHT].empty())
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
					g_time_info.ntp_sync_hour = ntp[JSON_NTP_SYNC_HOUR].asInt();

				if(!ntp[JSON_NTP_IP].empty())
				{
					string ip = ntp[JSON_NTP_IP].asString();
					sprintf(g_time_info.ntp_server,ip.c_str());
				}
			}

		}
	}

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
	g_user_info.seurity_status = ON;

	//Check user cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(USER_INFO);
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

	fclose(fp);		return 0;
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_VW_SETTING].empty())
		{
			Json::Value& root = root1[JSON_VW_SETTING];

			if(!root[JSON_VW_H_COUNT].empty())
				g_videowall_info.horizontal_count = root[JSON_VW_H_COUNT].asInt();

			if(!root[JSON_VW_V_COUNT].empty())
				g_videowall_info.vertical_count= root[JSON_VW_V_COUNT].asInt();

			if(!root[JSON_VW_POSITION].empty())
				g_videowall_info.relative_position= root[JSON_VW_POSITION].asInt();

			if(!root[JSON_VW_BEZEL].empty())
			{
				Json::Value& bezel = root[JSON_VW_BEZEL];

				if(!bezel[JSON_VW_BEZEL_H_VALUE].empty())
					g_videowall_info.bezel_horizontal_value = bezel[JSON_VW_BEZEL_H_VALUE].asInt();
				if(!bezel[JSON_VW_BEZEL_V_VALUE].empty())
					g_videowall_info.bezel_vertical_value = bezel[JSON_VW_BEZEL_V_VALUE].asInt();
				if(!bezel[JSON_VW_BEZEL_H_OFFSET].empty())
					g_videowall_info.bezel_horizontal_offset = bezel[JSON_VW_BEZEL_H_OFFSET].asInt();
				if(!bezel[JSON_VW_BEZEL_V_OFFSET].empty())
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
		}
	}
	fclose(fp);

	return 0;
}

int Cfg_Init_Gateway(void)
{
	DBG_InfoMsg("Cfg_Init_Gateway\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,GATEWAY_FILE);

	g_gateway_info.cec_mode = ON;
	g_gateway_info.cec_output = 0;
	g_gateway_info.rs232_mode = ON;
	g_gateway_info.rs232_port = 50001;
	g_gateway_info.rs232_param.rate = 115200;
	g_gateway_info.rs232_param.bitWidth = 8;
	g_gateway_info.rs232_param.parity = PARITY_NONE;
	g_gateway_info.rs232_param.stopBitsMode = 1;
	g_gateway_info.ir_mode = ON;
	g_gateway_info.ir_direction = DIRECTION_OUT;

	//Check gateway cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

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
				g_gateway_info.rs232_port = root[JSON_GW_UART_PORT].asInt();
			}

			if(!root[JSON_GW_UART_PARAM].empty())
			{
				Json::Value& JsonParam = root[JSON_GW_UART_PARAM];

				if(!JsonParam[JSON_GW_UART_RATE].empty())
				{
					g_gateway_info.rs232_param.rate = root[JSON_GW_UART_RATE].asInt();
					if((g_gateway_info.rs232_param.rate != 9600)
						&&(g_gateway_info.rs232_param.rate != 19200)
						&&(g_gateway_info.rs232_param.rate != 38400)
						&&(g_gateway_info.rs232_param.rate != 57600)
						&&(g_gateway_info.rs232_param.rate != 115200))
					{
						g_gateway_info.rs232_param.rate = 115200;
					}

				}

				if(!JsonParam[JSON_GW_UART_BITS].empty())
				{
					g_gateway_info.rs232_param.bitWidth = root[JSON_GW_UART_BITS].asInt();
					if((g_gateway_info.rs232_param.bitWidth<5)||(g_gateway_info.rs232_param.bitWidth>8))
					{
						g_gateway_info.rs232_param.bitWidth = 8;
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
					g_gateway_info.rs232_param.stopBitsMode = root[JSON_GW_UART_STOP].asFloat();

					int tmp = (int)(g_gateway_info.rs232_param.stopBitsMode*10);
					if((tmp != 10)&&(tmp != 15)&&(tmp != 20))
					{
						g_gateway_info.rs232_param.stopBitsMode = 1;
					}
				}
			}
		}
	}

	fclose(fp);

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
	sprintf(g_network_info.multicast_ip,"225.0.100.0");
	g_network_info.multicast_ttl = 0;
	g_network_info.p3k_port = 0;
	g_network_info.p3k_vlan = 0;
	g_network_info.rs232_port = 0;
	g_network_info.rs232_vlan = 0;
	g_network_info.dante_port = 0;
	g_network_info.dante_vlan = 0;

	//Check gateway cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

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

	fread(pBuf,1,sizeof(pBuf),fp);

	if(reader.parse(pBuf, root1))
	{
		if(!root1[JSON_NETWORK].empty())
		{
			Json::Value& root = root1[JSON_NETWORK];

			for(int i = 0; i < 2; i++)
			{
				Json::Value eth;
				if((i == 0)&&(!root[JSON_NETWORK_ETH0].empty()))
					eth = root[JSON_NETWORK_ETH0];
				else if((i == 1)&&(!root[JSON_NETWORK_ETH1].empty()))
					eth = root[JSON_NETWORK_ETH1];
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
							sprintf(g_network_info.eth_info[i].ipAddr,addr.c_str());
						}

						if(!eth[JSON_NETWORK_MASK].empty())
						{
							string addr = eth[JSON_NETWORK_MASK].asString();
							sprintf(g_network_info.eth_info[i].mask,addr.c_str());
						}

						if(!eth[JSON_NETWORK_GW].empty())
						{
							string addr = eth[JSON_NETWORK_GW].asString();
							sprintf(g_network_info.eth_info[i].gateway,addr.c_str());
						}

						if(!eth[JSON_NETWORK_DNS1].empty())
						{
							string addr = eth[JSON_NETWORK_DNS1].asString();
							sprintf(g_network_info.eth_info[i].dns1,addr.c_str());
						}

						if(!eth[JSON_NETWORK_DNS2].empty())
						{
							string addr = eth[JSON_NETWORK_DNS2].asString();
							sprintf(g_network_info.eth_info[i].dns2,addr.c_str());
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
				g_network_info.tcp_port= root[JSON_NETWORK_TCP].asInt();
			if(!root[JSON_NETWORK_UDP].empty())
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
							sprintf(g_network_info.multicast_ip,addr.c_str());
						}

						if(!multi[JSON_NETWORK_TTL].empty())
							g_network_info.multicast_ttl = multi[JSON_NETWORK_TTL].asInt();
					}
				}
			}

			if(!root[JSON_NETWORK_PORT_SET].empty())
			{
				Json::Value& port = root[JSON_NETWORK_PORT_SET];

				if(!port[JSON_NETWORK_P3K].empty())
				{
					Json::Value& p3k_port = port[JSON_NETWORK_P3K];

					if(!p3k_port[JSON_NETWORK_PORT].empty())
					{
						string port = p3k_port[JSON_NETWORK_PORT].asString();
						if(port == JSON_NETWORK_ETH0)
							g_network_info.p3k_port = 0;
						else
							g_network_info.p3k_port = 1;
					}

					if(!p3k_port[JSON_NETWORK_VLAN].empty())
						g_network_info.p3k_vlan = p3k_port[JSON_NETWORK_VLAN].asInt();
				}
				if(!port[JSON_NETWORK_RS232].empty())
				{
					Json::Value& rs232_port = port[JSON_NETWORK_RS232];

					if(!rs232_port[JSON_NETWORK_PORT].empty())
					{
						string port = rs232_port[JSON_NETWORK_PORT].asString();
						if(port == JSON_NETWORK_ETH0)
							g_network_info.rs232_port = 0;
						else
							g_network_info.rs232_port = 1;
					}

					if(!rs232_port[JSON_NETWORK_VLAN].empty())
						g_network_info.rs232_vlan = rs232_port[JSON_NETWORK_VLAN].asInt();
				}
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
						g_network_info.rs232_vlan = dante_port[JSON_NETWORK_VLAN].asInt();

				}
			}

		}
	}

	fclose(fp);		return 0;
}

int Cfg_Init_Log(void)
{
	DBG_InfoMsg("Cfg_Init_Log\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,LOG_SETTING_FILE);

	g_log_info.active = OFF;
	g_log_info.period = 3;

	//Check log cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);

		//create channel cfg from default value
		Cfg_Update(LOG_INFO);
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

	fclose(fp);
	return 0;
}

int Cfg_Create_DefaultFile(void)
{
	Cfg_Create_AutoswitchDelay();
	Cfg_Create_AVSignal();
	Cfg_Create_DisplaySleep();
	Cfg_Create_OsdSetting();
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
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

	string strAutoSwitchSetting = root1.toStyledString();
	fwrite(strAutoSwitchSetting.c_str(),1,strAutoSwitchSetting.size(),fp);

	fclose(fp);
	fflush(fp);
	return 0;
}

// /av_signal/av_signal.json
int Cfg_Create_AVSignal(void)
{
	DBG_InfoMsg("Cfg_Create_AVSignal\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,AV_SIGNAL_FILE);

	//Check Video cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_AV_IN_RES] = JSON_AV_PASSTHRU;
	root[JSON_AV_MAX_BITRATE] = JSON_AV_BEST_EFFORT;
	root[JSON_AV_FRAME_RATE] = 100;
	root[JSON_AV_COLOR_DEPTH] = JSON_AV_BYPASS;
	root[JSON_AV_RGB] = JSON_PARAM_OFF;
	root[JSON_AV_AUD_GUARD] = 90;


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

	string strAVSignal = root1.toStyledString();
	fwrite(strAVSignal.c_str(),1,strAVSignal.size(),fp);

	fclose(fp);
	fflush(fp);
	return 0;
}

// /display/display_sleep.json
int Cfg_Create_DisplaySleep(void)
{
	DBG_InfoMsg("Cfg_Create_DisplaySleep\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,DISPLAY_SLEEP_FILE);

	//Check Video cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_DISPLAY_SLEEP] = 5;
	root[JSON_DISPLAY_SHUTDOWN] = 10;
	root[JSON_DISPLAY_WAKEUP] = 10;

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

	string strDispDelay = root1.toStyledString();
	fwrite(strDispDelay.c_str(),1,strDispDelay.size(),fp);

	fclose(fp);
	fflush(fp);
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_OSD_TIMEOUT] = 30;
	root[JSON_OSD_POS] = JSON_OSD_TOP_LEFT;
	root[JSON_OSD_FONE_SIZE] = JSON_OSD_MEDIUM;
	root[JSON_OSD_MAX_PER_PAGE] = 5;
	root[JSON_OSD_MAX_CHANNEL] = 999;

	Json::Value root1;
	root1[JSON_OSD_CHANNEL] = root;

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

	string strOSD = root1.toStyledString();
	fwrite(strOSD.c_str(),1,strOSD.size(),fp);

	fclose(fp);
	fflush(fp);
	return 0;
}

// /secure/security_setting.json
int Cfg_Create_SecuritySetting(void)
{
	DBG_InfoMsg("Cfg_Create_SecuritySetting\n");

	char path[128] = "";
	sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_FILE);

	//Check Video cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_SECUR_802_1X_MODE] = JSON_PARAM_ON;

	Json::Value JsonCAArray;
	JsonCAArray.resize(1);
	Json::Value JsonCA = JsonCAArray[0];
	JsonCA[JSON_SECUR_CA] = "NULL";
	root[JSON_SECUR_CERTS] = JsonCAArray;


	root[JSON_SECUR_DEFAULT_AUTH] = JSON_SECUR_EAP_TLS;
	root[JSON_SECUR_EAP_TLS_CERT] = "NULL";
	root[JSON_SECUR_HTTPS_ON] = "yes";
	root[JSON_SECUR_HTTPS_CERT] = "NULL";
	root[JSON_SECUR_DEFAULT_HTTPS_CERT] = "NULL";

	Json::Value root1;
	root1[JSON_SECUR_SETTING] = root;

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
	sprintf(path,"%s%s%s",CONF_PATH,g_module,SECURITY_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	string strSecurity = root1.toStyledString();
	fwrite(strSecurity.c_str(),1,strSecurity.size(),fp);

	fclose(fp);
	fflush(fp);
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}

	Json::Value root;

	root[JSON_USB_KVM_MODE] = JSON_USB_KVM_KM;
	root[JSON_USB_KVM_ACTIVE] = JSON_PARAM_ON;
	root[JSON_USB_KVM_TIMEOUT] = 10;
	root[JSON_USB_KVM_ROW] = 1;
	root[JSON_USB_KVM_COL] = 1;

	Json::Value JsonRoamArray;
	JsonRoamArray.resize(1);
	Json::Value JsonRoam;
	JsonRoam[JSON_USB_KVM_MAC] = "";
	JsonRoam[JSON_USB_KVM_H] = "";
	JsonRoam[JSON_USB_KVM_V] = "";
	JsonRoamArray[0] = JsonRoam;

	root[JSON_USB_KVM_ROAMING] = JsonRoamArray;

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

	string strKvm = root1.toStyledString();
	fwrite(strKvm.c_str(),1,strKvm.size(),fp);

	fclose(fp);
	fflush(fp);
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
		//printf("nAccessRet %s Suceess\n",path);
		return 0;
	}
#if 1
	system("cp -rf /share/edid /data/configs/kds-7/");

#else
	Json::Value root;

	root["0"] = JSON_EDID_DEFAULT;
	root["1"] = "";
	root["2"] = "";
	root["3"] = "";
	root["4"] = "";
	root["5"] = "";
	root["6"] = "";
	root["7"] = "";

	Json::Value root1;
	root1[JSON_EDID_LIST] = root;

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_PATH);
	int s32AccessRet = access(path, F_OK);
	if(s32AccessRet != 0)
	{
		char cmd[256] = "";
		sprintf(cmd,"mkdir -p %s",path);

		system(cmd);
	}

	memset(path,0,128);
	sprintf(path,"%s%s%s",CONF_PATH,g_module,EDID_LIST_FILE);
	FILE *fp;
	fp = fopen(path, "w");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open %s\n",path);
		return -1;
	}

	string strEDIDList = root1.toStyledString();
	fwrite(strEDIDList.c_str(),1,strEDIDList.size(),fp);

	fclose(fp);
	fflush(fp);
#endif
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

	//Check Video cfg
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 == nAccessRet)
	{
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

	string strChanList = root1.toStyledString();
	fwrite(strChanList.c_str(),1,strChanList.size(),fp);

	fclose(fp);
	fflush(fp);
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
	DBG_InfoMsg("This is Decoder\n");
	return 0;
#endif

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

	string strChannelDef = root1.toStyledString();
	fwrite(strChannelDef.c_str(),1,strChannelDef.size(),fp);

	fclose(fp);
	fflush(fp);
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
		for(int i = 0; i < 3; i++)
		{
			Json::Value& JasonPri = JsonPriorityArray[i];

			if(g_audio_info.input_pri[i] == AUDIO_IN_HDMI)
				JasonPri = JSON_AUDIO_HDMI;
			else if(g_audio_info.input_pri[i] == AUDIO_IN_ANALOG)
				JasonPri = JSON_AUDIO_ANALOG;
			else if(g_audio_info.input_pri[i] == AUDIO_IN_DANTE)
				JasonPri = JSON_AUDIO_DANTE;
			else
				continue;
		}

		root[JSON_PRIORITY] = JsonPriorityArray;

		Json::Value JsonDestArray;
		JsonDestArray.resize(4);
		for(int i = 0; i < 4; i++)
		{
			Json::Value& JsonDst = JsonDestArray[i];

			if(g_audio_info.dst_port[i] == PORT_HDMI)
				JsonDst = JSON_AUDIO_HDMI;
			else if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
				JsonDst = JSON_AUDIO_ANALOG;
			else if(g_audio_info.dst_port[i] == PORT_DANTE)
				JsonDst = JSON_AUDIO_DANTE;
			else if(g_audio_info.dst_port[i] == PORT_STREAM)
				JsonDst = JSON_AUDIO_LAN;
			else
			{
				JsonDestArray.resize(i);
				break;
			}
		}
		root[JSON_AUDIO_DEST] = JsonDestArray;

		if(g_audio_info.source == AUDIO_IN_HDMI)
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_HDMI;
		else if(g_audio_info.source == AUDIO_IN_ANALOG)
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_ANALOG;
		else
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_DANTE;

		if(strlen(g_audio_info.dante_name) > 0)
			root[JSON_DANTE_NAME] = g_audio_info.dante_name;
	}
	else
	{
		Json::Value JsonPriorityArray;
		JsonPriorityArray.resize(2);
		for(int i = 0; i < 2; i++)
		{
			Json::Value& JasonPri = JsonPriorityArray[i];

			if(g_audio_info.input_pri[i] == AUDIO_IN_HDMI)
				JasonPri = JSON_AUDIO_HDMI;
			else if(g_audio_info.input_pri[i] == AUDIO_IN_ANALOG)
				JasonPri = JSON_AUDIO_ANALOG;
			else
				continue;
		}

		root[JSON_PRIORITY] = JsonPriorityArray;

		Json::Value JsonDestArray;
		JsonDestArray.resize(3);
		for(int i = 0; i < 3; i++)
		{
			Json::Value& JsonDst = JsonDestArray[i];

			if(g_audio_info.dst_port[i] == PORT_HDMI)
				JsonDst = JSON_AUDIO_HDMI;
			else if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
				JsonDst = JSON_AUDIO_ANALOG;
			else if(g_audio_info.dst_port[i] == PORT_STREAM)
				JsonDst = JSON_AUDIO_LAN;
			else
			{
				JsonDestArray.resize(i);
				break;
			}
		}
		root[JSON_AUDIO_DEST] = JsonDestArray;

		if(g_audio_info.source == AUDIO_IN_HDMI)
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_HDMI;
		else
			root[JSON_SOURCE_SELECT] = JSON_AUDIO_ANALOG;

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

	string strAutoSwitch = root1.toStyledString();
	fwrite(strAutoSwitch.c_str(),1,strAutoSwitch.size(),fp);

	fclose(fp);
	fflush(fp);
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
	int nAccessRet = access(path,F_OK | R_OK | W_OK);
	if(0 > nAccessRet)
	{
		printf("nAccessRet %s Failed\n",path);
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

		fread(pBuf,1,sizeof(pBuf),fp);

		if(reader.parse(pBuf, root1))
		{
			DBG_InfoMsg("open %s Success !!!\n",path);
		}

		fclose(fp);
	}

	if(!root1[JSON_AV_SIGNAL].empty())
	{
		Json::Value& root = root1[JSON_AV_SIGNAL];

		if(g_video_info.force_rgb == 0)
			root[JSON_AV_RGB] = JSON_PARAM_OFF;
		else
			root[JSON_AV_RGB] = JSON_PARAM_ON;
	}
	else
	{
		Json::Value root;
		if(g_video_info.force_rgb == 0)
			root[JSON_AV_RGB] = JSON_PARAM_OFF;
		else
			root[JSON_AV_RGB] = JSON_PARAM_ON;

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

	string strAVsetting = root1.toStyledString();
	fwrite(strAVsetting.c_str(),1,strAVsetting.size(),fp2);

	fclose(fp2);
	fflush(fp2);
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

	string strAutoSwitch = root1.toStyledString();
	fwrite(strAutoSwitch.c_str(),1,strAutoSwitch.size(),fp);

	fclose(fp);
	fflush(fp);

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

	string strAVSetting = root1.toStyledString();
	fwrite(strAVSetting.c_str(),1,strAVSetting.size(),fp);

	fclose(fp);
	fflush(fp);		return 0;
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

	string strEDIDSetting = root1.toStyledString();
	fwrite(strEDIDSetting.c_str(),1,strEDIDSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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

	string strDeviceSetting = root1.toStyledString();
	fwrite(strDeviceSetting.c_str(),1,strDeviceSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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

	string strVersionSetting = root1.toStyledString();
	fwrite(strVersionSetting.c_str(),1,strVersionSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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

	string strTimeSetting = root1.toStyledString();
	fwrite(strTimeSetting.c_str(),1,strTimeSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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

	string strUSerSetting = root1.toStyledString();
	fwrite(strUSerSetting.c_str(),1,strUSerSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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

	//string strVideowallSetting = fast_writer.write(root1);
	string strVideowallSetting = root1.toStyledString();
	fwrite(strVideowallSetting.c_str(),1,strVideowallSetting.size(),fp);

	fclose(fp);
	fflush(fp);

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

	int tmp = g_gateway_info.rs232_param.stopBitsMode*10;
	if((tmp == 10)||(tmp == 15)||(tmp == 20))
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

	string strGateway = root1.toStyledString();
	fwrite(strGateway.c_str(),1,strGateway.size(),fp);

	fclose(fp);
	fflush(fp);

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
			eth[JSON_NETWORK_DNS1] = g_network_info.eth_info[i].dns1;
			eth[JSON_NETWORK_DNS1] = g_network_info.eth_info[i].dns2;
		}
		else
		{
			eth[JSON_NETWORK_MODE] = JSON_NETWORK_DHCP;
		}

		if(i == 0)
			root[JSON_NETWORK_ETH0] = eth;
		else
			root[JSON_NETWORK_ETH1] = eth;
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
		multi[JSON_NETWORK_GROUP_IP] = g_network_info.multicast_ip;
		multi[JSON_NETWORK_TTL] = g_network_info.multicast_ttl;

		root[JSON_NETWORK_MULTICAST] = multi;
	}

	Json::Value port;
	Json::Value p3k;
	Json::Value rs232;
	Json::Value dante;

	if(g_network_info.p3k_port == 0)
		p3k[JSON_NETWORK_PORT] = JSON_NETWORK_ETH0;
	else
		p3k[JSON_NETWORK_PORT] = JSON_NETWORK_ETH1;
 	p3k[JSON_NETWORK_VLAN] = g_network_info.p3k_vlan;
 	port[JSON_NETWORK_P3K] = p3k;

	if(g_network_info.rs232_port == 0)
		rs232[JSON_NETWORK_PORT] = JSON_NETWORK_ETH0;
	else
		rs232[JSON_NETWORK_PORT] = JSON_NETWORK_ETH1;
 	rs232[JSON_NETWORK_VLAN] = g_network_info.rs232_vlan;
 	port[JSON_NETWORK_RS232] = rs232;

	if(g_network_info.dante_port == 0)
		dante[JSON_NETWORK_PORT] = JSON_NETWORK_ETH0;
	else
		dante[JSON_NETWORK_PORT] = JSON_NETWORK_ETH1;
 	dante[JSON_NETWORK_VLAN] = g_network_info.dante_vlan;
 	port[JSON_NETWORK_DANTE] = dante;

	root[JSON_NETWORK_PORT_SET] = port;

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

	string strNetwork = root1.toStyledString();
	fwrite(strNetwork.c_str(),1,strNetwork.size(),fp);

	fclose(fp);
	fflush(fp);
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

	string strLogSetting = root1.toStyledString();
	fwrite(strLogSetting.c_str(),1,strLogSetting.size(),fp);

	fclose(fp);
	fflush(fp);
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
		g_autoswitch_info.input_pri[0] = port1;
		g_autoswitch_info.input_pri[1] = port2;
		g_autoswitch_info.input_pri[2] = port3;
		Cfg_Update(AUTOSWITCH_INFO);
	}
	else if(type == SIGNAL_AUDIO)
	{
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
		printf("Cfg_Get_Autoswitch_Priority Video %d,%d,%d\n",g_autoswitch_info.input_pri[0],g_autoswitch_info.input_pri[1],g_autoswitch_info.input_pri[2]);
	}
	else if(type == SIGNAL_AUDIO)
	{
		*port1 = g_audio_info.input_pri[0];
		*port2 = g_audio_info.input_pri[1];
		*port3 = g_audio_info.input_pri[2];
		printf("Cfg_Get_Autoswitch_Priority Audio %d,%d,%d\n",g_audio_info.input_pri[0],g_audio_info.input_pri[1],g_audio_info.input_pri[2]);
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
		g_audio_info.source = (AudioInputMode_E)port;
		Cfg_Update(AUDIO_INFO);
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
		if(strlen(name) <= 31)
			strcpy(g_audio_info.dante_name,name);
		else
			memcpy(g_audio_info.dante_name,name,31);

		Cfg_Update(AUDIO_INFO);
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
		strcpy(name,g_audio_info.dante_name);
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
		memcpy(netInfo,&(g_network_info.eth_info[netId]),sizeof(NetWorkInfo_S));
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
		memcpy(&(g_network_info.eth_info[netId]),netInfo,sizeof(NetWorkInfo_S));
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
	if(g_network_info.method == Net_MULTICAST)
	{
		*ttl = g_network_info.multicast_ttl;
		strcpy(ip,g_network_info.multicast_ip);
	}
	else
	{
		*ttl = 0;
		strcpy(ip,"0.0.0.0");
		DBG_WarnMsg("Cfg_Get_Net_Multicast method:%d is Wrong!!!\n",g_network_info.method);
	}
	return 0;
}
int Cfg_Set_Net_GW_Port(NetGWType_E type, int port) //type:p3k,rs232,dante
{
	DBG_InfoMsg("Cfg_Set_Net_GW_Port\n");
	if(type == Net_P3K) //p3k
		g_network_info.p3k_port = port;
	else if(type == Net_RS232) //RS232
		g_network_info.rs232_port = port;
	else if(type == Net_DANTE) //Dante
		g_network_info.dante_port = port;
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
	if(type == Net_P3K) //p3k
		 *port = g_network_info.p3k_port;
	else if(type == Net_RS232) //RS232
		*port = g_network_info.rs232_port;
	else if(type == Net_DANTE) //Dante
		*port = g_network_info.dante_port;
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
	if(type == Net_P3K) //p3k
		g_network_info.p3k_vlan = vlan;
	else if(type == Net_RS232) //RS232
		g_network_info.rs232_vlan = vlan;
	else if(type == Net_DANTE) //Dante
		g_network_info.dante_vlan = vlan;
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
	if(type == Net_P3K) //p3k
		*vlan = g_network_info.p3k_vlan;
	else if(type == Net_RS232) //RS232
		*vlan = g_network_info.rs232_vlan;
	else if(type == Net_DANTE) //Dante
		*vlan = g_network_info.dante_vlan;
	else
	{
		DBG_WarnMsg("Cfg_Get_Net_GW_Vlan type:%d is Wrong!!!\n",type);
		return -1;
	}
	return 0;
}

int Cfg_Set_OSD_Diaplay(State_E mode)
{
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
					DBG_WarnMsg("JSON_AV_MAX_BITRATE Param: %s  Error!!!",bitrate.c_str());
				}

				printf("%s\n",cmd);
			}

			if(!root[JSON_AV_FRAME_RATE].empty())
			{
				char cmd[256] = "";
				int frame = root[JSON_AV_FRAME_RATE].asInt();

				if((frame <= 100)&&(frame >= 0))
				{
					int param = frame * 60 / 100;
					sprintf(cmd,"astparam s v_frame_rate %d;astparam save",param);
					system(cmd);
				}
				else
				{
					DBG_WarnMsg("JSON_AV_FRAME_RATE Param: %d  Error!!!",frame);
				}

				printf("%s\n",cmd);
			}
#endif
		}
	}

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

					if((!JsonKVM[JSON_USB_KVM_MAC].empty())&&(!JsonKVM[JSON_USB_KVM_H].empty())&&(!JsonKVM[JSON_USB_KVM_V].empty()))
					{
						string mac = JsonKVM[JSON_USB_KVM_MAC].asString();
						int    x   = JsonKVM[JSON_USB_KVM_H].asInt();
						int    y   = JsonKVM[JSON_USB_KVM_V].asInt();

						printf("JsonKVMArray[%d]:[mac: %s][x: %d][y: %d]\n",i,mac.c_str(),x,y);

						if((x == 0)&&(y == 0))
						{
							DBG_InfoMsg("This is master [x: %d][y: %d]\n",x,y);
						}
						else if(mac.size()>1)
						{
							if(strlen(param) == 0)
								sprintf(param,"%s,%d,%d",mac.c_str(),x,y);
							else
								sprintf(param,"%s:%s,%d,%d",param,mac.c_str(),x,y);
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

			if(!root[JSON_USB_KVM_TIMEOUT].empty())
			{
				char cmd[256] = "";
				int interval =  root[JSON_USB_KVM_TIMEOUT].asInt();

				if((interval <= 10)&&(interval >= 0))
				{
					sprintf(cmd,"astparam s kmoip_token_interval %d;astparam save",interval*60*1000);
					system(cmd);
				}

				DBG_InfoMsg("%s\n",cmd);
			}
		}
	}

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
	char module[32]="";
	int ncount = 1;
	GetBoardInfo(BOARD_MODEL, module, 32);
	if(strcmp(module,IPE_P_MODULE) == 0)
		ncount = 3;
	else if(strcmp(module,IPE_W_MODULE) == 0)
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

