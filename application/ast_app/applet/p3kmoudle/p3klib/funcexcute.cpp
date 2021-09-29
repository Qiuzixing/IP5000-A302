#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "funcexcute.h"
#include "ast_send_event.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "classsum.h"

#include "json/json.h"
#include <iostream>
#include <string>
#include <fstream>

#include "cfgparser.h"
#include "debugtool.h"

using namespace std;

#define BUFSIZE 128
#define PORT 5588

static char  sUpInfo[256] = {0};

int mysystem(char* cmdstring, char* buf, int len)
{
	pid_t status;
	FILE * ptr;

	memset(buf,0,len);

	status = system(cmdstring);
	if((ptr = popen(cmdstring,"r")) != NULL)
	{
		fgets(buf,len,ptr);
		pclose(ptr);
	}

	if(strstr(cmdstring,"astparam")==0)
	{
		int size = strlen(buf);
		buf[size - 1] = 0;
	}

	DBG_InfoMsg("mysystem cmdstring:%s,buf:%s\n",cmdstring,buf);
	return 0;
}

int GetBoardInfo(BoardInfoType_E type, char* info, unsigned int size)
{
	FILE *fp;

	memset(info,0,size);

	DBG_InfoMsg("GetBoardInfo type:%d,size:%d start\n",type,size);

	if(type == BOARD_HOSTNAME)
	{
		fp = fopen(KDS_HOSTNAME_FILE, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open hostname\n");
			return -1;
		}
		fread(info,1,size,fp);
	}
	else if(type == BOARD_BUILD_DATE)
	{
		fp = fopen(KDS_VSRSION_FILE, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open hostname\n");
			return -1;
		}

		char model[128];
		char fw_version[128];
		char date[128];
		fgets(model, 128, fp);
		fgets(fw_version, 128, fp);
		fgets(date, 128, fp);

		if(strlen(date) > 0)
		{
			strcpy(info,date);
		}
		else
		{
			DBG_ErrMsg("ERROR! can't open date\n");
		}
	}
	else
	{
		Json::Reader reader;
		Json::Value root;
		char pBuf[1024] = "";

		fp = fopen(KDS_BOARD_INFO_FILE, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open boardinfo\n");
			return -1;
		}

		fread(pBuf,1,sizeof(pBuf),fp);

		if(reader.parse(pBuf, root))
		{
			if(type == BOARD_MODEL)
			{
				if(!root["model"].empty())
				{
					string buf = root["model"].asString();
					if(buf.length() > size)
					{
						DBG_ErrMsg("ERROR! model is too long\n");
					}
					else
					{
						strcpy(info,buf.c_str());
					}
				}
			}
			else if(type == BOARD_SN)
			{
				if(!root["serial number"].empty())
				{
					string buf = root["serial number"].asString();
					if(buf.length() > size)
					{
						DBG_ErrMsg("ERROR! serial number is too long\n");
					}
					else
					{
						strcpy(info,buf.c_str());
					}
				}
			}
			else if(type == BOARD_MAC)
			{
				if(!root["mac address"].empty())
				{
					string buf = root["mac address"].asString();
					if(buf.length() > size)
					{
						DBG_ErrMsg("ERROR! smac address is too long\n");
					}
					else
					{
						strcpy(info,buf.c_str());
					}
				}
			}
			else if(type == BOARD_HW_VERSION)
			{
				if(!root["board version"].empty())
				{
					string buf = root["board version"].asString();
					if(buf.length() > size)
					{
						DBG_ErrMsg("ERROR! board version is too long\n");
					}
					else
					{
						strcpy(info,buf.c_str());
					}
				}
			}
			else if(type == BOARD_FW_VERSION)
			{
				if(!root["firmware version"].empty())
				{
					string buf = root["firmware version"].asString();
					if(buf.length() > size)
					{
						DBG_ErrMsg("ERROR! firmware version is too long\n");
					}
					else
					{
						strcpy(info,buf.c_str());
					}
				}
			}
		}
	}
	fclose(fp);

	DBG_InfoMsg("GetBoardInfo type:%d,info:%s end\n",type,info);
	return 0;
}

int classTest(int a,int b)
{
	Classsum test ;
	int sumnum=0;
	sumnum = test.sum(a,b);

	return sumnum;
}

int sendCmdtoGUI(const char *buf)
{
	DBG_InfoMsg("sendCmdtoGUI \n");
	return 0;

	//
	char sendbuf[BUFSIZE] = {0};
	memcpy(sendbuf,buf,strlen(buf)+1);
	// ³õÊ¼»¯
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	//DBG_InfoMsg("fd: %d\n",fd);
	if(fd < 0)
	{
		DBG_ErrMsg("socket failed \n");
		return -1;
	}

	// µØÖ·¶Ë¿Ú³õÊ¼»¯
	struct sockaddr_in sockaddr_dest;
	sockaddr_dest.sin_family = AF_INET;
	sockaddr_dest.sin_addr.s_addr = inet_addr("127.0.0.1");
	sockaddr_dest.sin_port = htons(PORT);

	int mode = 0;
	int len = sizeof(sockaddr_dest);
	if(strncmp(sendbuf,"GET",3) == 0)
	{
		int ret = sendto(fd,sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&sockaddr_dest,len);
		//DBG_InfoMsg("ret: %d\n",ret);
		//DBG_InfoMsg("send finished \n");

		char recvbuf[64] = {0};
		struct sockaddr_in peer;
		socklen_t  peerlen = sizeof(peer);
		int recv = recvfrom(fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&peer, &peerlen);
		//DBG_InfoMsg("recv: %d\n",recv);
		//DBG_InfoMsg("recvbuf: %s\n",recvbuf);

		mode = atoi(recvbuf);
		//DBG_InfoMsg("mode: %d\n",mode);
	}
	else
	{
		int ret = sendto(fd,sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&sockaddr_dest,len);
		//DBG_InfoMsg("ret: %d\n",ret);
		//DBG_InfoMsg("send finished \n");
	}

	//close(fd);
	return mode;
}

//"#KDS-AUD" 0:hdmi;1:analog;4:dante
int  EX_SetAudSrcMode(int mode)
{
	DBG_InfoMsg("EX_SetAudSrcMode mode =%d\n",mode);

#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(mode == 0)
		sprintf(sCmd,"e_p3k_audio_src::hdmi");
	else if(mode == 1)
	{
		if(g_audio_info.direction == DIRECTION_IN)
		{
			sprintf(sCmd,"e_p3k_audio_src::analog");
		}
		else
		{
			DBG_WarnMsg(" !!! Error g_audio_info.direction == DIRECTION_OUT \n");
			return 0;
		}
	}
	else if(mode == 4)
		sprintf(sCmd,"e_p3k_audio_src::dante");
	else
	{
		DBG_ErrMsg(" !!! Error mode:%d \n",mode);
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Source(SIGNAL_AUDIO,mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int  EX_GetAudSrcMode(int *mode)
{
	Cfg_Get_Autoswitch_Source(SIGNAL_AUDIO,mode);
	return 0;
}

int EX_SetAudGainLevel(PortInfo_S*info,int gain)
{
	DBG_InfoMsg("gain =%d\n",gain);
	char sCmd[64] = "";

	if(info->signal == SIGNAL_AUDIO)
	{
		sprintf(sCmd,"e_p3k_audio_level::%d",gain);

		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_AV_Volume(gain);
	}
	else
	{
		return 0;
	}

	return 0;
}

int EX_SetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	DBG_InfoMsg("EX_SetAudAnalogGainDir, gain =%s\n",gain);
	PortSignalType_E type;
	PortDirectionType_E direction;

	char sCmd[64] = "";
	type = info->portFormat;
	if(info->portFormat == PORT_IR)
	{
		if((!strcmp(gain,"IN"))||(!strcmp(gain,"in")))
		{
			sprintf(sCmd,"e_p3k_ir_dir::in");
			direction = DIRECTION_IN;
		}
		else if((!strcmp(gain,"OUT"))||(!strcmp(gain,"out")))
		{
			sprintf(sCmd,"e_p3k_ir_dir::out");
			direction = DIRECTION_OUT;
		}
		else
		{
			printf("EX_SetIRDir Error, gain =%s\n",gain);
			return 0;
		}
	}
	else if(info->portFormat == PORT_ANALOG_AUDIO)
	{
		if((!strcmp(gain,"IN"))||(!strcmp(gain,"in")))
		{
			sprintf(sCmd,"e_p3k_audio_dir::in");
			direction = DIRECTION_IN;
		}
		else if((!strcmp(gain,"OUT"))||(!strcmp(gain,"out")))
		{
			sprintf(sCmd,"e_p3k_audio_dir::out");
			direction = DIRECTION_OUT;
		}
		else
		{
			printf("EX_SetAudioDir Error, gain =%s\n",gain);
			return 0;
		}
	}
	else
	{
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Port_Dir(type,direction);
	return 0;
}


int EX_GetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	PortSignalType_E type;
	PortDirectionType_E direction;

	if((info->portFormat == PORT_IR)||(info->portFormat == PORT_ANALOG_AUDIO))
	{
		type = info->portFormat;
		Cfg_Get_Port_Dir(type,&direction);
	}

	if(direction == DIRECTION_OUT)
		strcpy(gain,"out");
	else
		strcpy(gain,"in");

	return 0;
}



int EX_SetEDIDMode(EdidInfo_S *info)
{
	//DBG_InfoMsg("gain =%s\n",gain);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return 0;
	}

	DBG_InfoMsg(">>EX_SetEDIDMode %d,%d\n",info->input_id,info->index);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(info->mode == PASSTHRU)
	{
		if(strlen(g_edid_info.net_src) > 0)
			sprintf(sCmd,"e_p3k_video_edid_passthru::%s",g_edid_info.net_src);
	}
	else if(info->mode == DEFAULT)
		sprintf(sCmd,"e_p3k_video_edid_default");
	else if(info->mode == CUSTOM)
	{
		sprintf(sCmd,"e_p3k_video_edid_custom::%d",info->index);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",info->mode);
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	Cfg_Set_EDID_Mode(info->mode,info->index);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_GetEDIDMode(int cmdID,EdidInfo_S * info)
{
	EdidModeType_E mode;
	int idx;

	Cfg_Get_EDID_Mode(&mode,&idx);

 	//EdidInfo_S tmpconf;
	info->input_id = (unsigned int)cmdID;
	info->mode = mode;

	if(mode == CUSTOM)
		info->index = idx;
	else
		info->index = 0;
	DBG_InfoMsg(">>EX_GetEDIDMode\n");
	return 0;
}

int EX_AddEDID(EdidName_S * info)
{
	DBG_InfoMsg(">>EX_AddEDID %d %s\n",info->index,info->name);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return 0;
	}

#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_add::%d::%s",info->index,info->name);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_RemoveEDID(int comID)
{
	DBG_InfoMsg(">>EX_RemoveEDID %d \n",comID);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return 0;
	}

	if(comID <= 0)
	{
		DBG_WarnMsg(">>>>>>>comID : %d is Wrong!!!\n",comID);
		return 0;
	}

	if(comID < g_edid_info.active_id)
		g_edid_info.active_id = g_edid_info.active_id -1;

#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_remove::%d",comID);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_SetActiveEDID(int input_ID,int index_ID )
{
	DBG_InfoMsg(">>EX_SetActiveEDID %d,%d \n",input_ID,index_ID);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return 0;
	}

	if(g_edid_info.edid_mode != CUSTOM)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.edid_mode != PASSTHRU\n");
		return 0;
	}
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_custom::%d",index_ID);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_EDID_Active(index_ID);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_SetEDIDNetSrc(int input_ID,char*macAddr )
{
	DBG_InfoMsg(">>EX_SetEDIDNetSrc id=%d mac=%s \n",input_ID,macAddr);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return 0;
	}

	if(g_edid_info.edid_mode != PASSTHRU)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.edid_mode != PASSTHRU\n");
		return 0;
	}

	if(strlen(macAddr) <= 0)
	{
		DBG_WarnMsg(">>>>>>>strlen(macAddr) <= 0\n");
		return 0;
	}
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_passthru::%s",macAddr);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_EDID_NetSrc(macAddr);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetEDIDNetSrc(int input_ID,char*macAddr )
{
	//char * version = "00-14-22-01-23-45";
	//memcpy(macAddr,version,strlen(version));
	Cfg_Get_EDID_NetSrc(macAddr);
	return 0;
}

int EX_GetRecvMsgNum(int msg,char*date )
{
	int iMsgNum = 100;

	return iMsgNum;
}

int EX_GetVidOutRatio(char*date )
{
	//char value[32] = "16:9";
	//memcpy(date,value,strlen(value));

	char* cmd1 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*Signal Type: HDMI (.*).*$#\\1#gp'";
	char buf1[16] = "";

	mysystem(cmd1,buf1,16);

	if(strlen(buf1) > 1)
		memcpy(date,buf1,strlen(buf1));
	else
		strcpy(date,"NA");
	return 0;
}

int EX_SetChannelName(char date[32] )
{
#ifdef CONFIG_P3K_HOST
		Cfg_Set_EncChannel_Name(date);
#else
		DBG_WarnMsg(" !!! This is Decoder\n");
#endif

	DBG_InfoMsg("AV channel Name %s\n",date);

	return 0;
}

int EX_SetDanteName(char date[32])
{
	DBG_InfoMsg("Set Encoder Dante Hostl Name. %s\n",date);

	return 0;
}

int EX_SetVidMute(MuteInfo_S * mute )
{

	DBG_InfoMsg(">>EX_SetVidMute %d,%d,%d,%d,%d,%d\n",mute->direction,mute->portFormat,mute->portIndex,
		mute->signal,mute->index,mute->state);
	//0	¨C off for unmute  1	¨C on for mute
	char sCmd[64] = "";
	if(mute->signal == SIGNAL_AUDIO)
	{
		sprintf(sCmd,"e_p3k_audio_mute::%d",mute->state);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
		Cfg_Set_AV_Mute(mute->state);
	}
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal=%d\n",mute->signal);
		return 0;
	}

	return 0;
}


int EX_SetTimeOut(int  iTime )
{

	DBG_InfoMsg(">>EX_SetTimeOut %d\n",iTime);
	Cfg_Set_User_LogoutTime(iTime);
	return 0;
}
int EX_GetTimeOut(void)
{

	int iTime = 0;
	Cfg_Get_User_LogoutTime(&iTime);

	return iTime;
}

// 0 - fit in
// 1 - fit out
int EX_SetVideoWallStretch(int  index,int mode )
{
	DBG_InfoMsg(">>EX_SetVideoWallStretch %d\n",index);
#ifdef CONFIG_P3K_CLIENT
	if((mode == 0)||(mode == 1))
	{
		Cfg_Set_VM_Stretch(mode);

		if(mode == 0)
			ast_send_event(0xFFFFFFFF,"e_vw_stretch_type_2");
		else
			ast_send_event(0xFFFFFFFF,"e_vw_stretch_type_1");
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
	}
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoWallStretch(int  index)
{
	int mode =1;
	Cfg_Get_VM_Stretch(&mode);

	return mode;
}


int EX_SetStandbyTimeOut(int  iTime )
{
	DBG_InfoMsg(">>EX_SetStandbyTimeOut %d\n",iTime);
	Cfg_Set_Dev_StandbyTime(iTime);

	return 0;
}
int EX_GetStandbyTimeOut(void)
{

	int iTime =10;
	Cfg_Get_Dev_StandbyTime(&iTime);

	return iTime;
}

int EX_SetRollback(char * type)
{
	char * aOk = "ok";
	memcpy(type,aOk,strlen(aOk));

	char* cmd1 = "astparam misc g cursys";
	char buf1[64] = "";

	mysystem(cmd1,buf1,16);

	if(strstr(buf1,"not defined") != 0)
	{
		DBG_WarnMsg("EX_SetRollback not defined\n");
		return 0;
	}
	else if(strstr(buf1,"a") != 0)
	{
		strcpy(g_version_info.standby_version,g_version_info.fw_version);
		Cfg_Update_Version();
		system("astparam misc s cursys b");
		system("reboot");
	}
	else if(strstr(buf1,"b") != 0)
	{
		strcpy(g_version_info.standby_version,g_version_info.fw_version);
		Cfg_Update_Version();
		system("astparam misc s cursys a");
		system("reboot");
	}
	else
	{
		return 0;
	}


	return 0;
}


int EX_SetIRGateway(int  iIr_mode)
{
	DBG_InfoMsg("Tr gw mode = %d\n",iIr_mode);
	Cfg_Set_GW_IR_Mode((State_E)iIr_mode);
	char sCmd[64] = "";
#if 1
	if(iIr_mode == 1)
	{
		system("astparam s ir_guest_on y;");
		system("astparam s ir_sw_decode_on y;astparam save");
	}
	else
	{
		system("astparam s ir_guest_on n;");
		system("astparam s ir_sw_decode_on n;astparam save");
	}
#else
	if((iIr_mode == 0)||(iIr_mode == 1))
	{
		sprintf(sCmd,"e_p3k_ir_gw::%d",iIr_mode);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para iIr_mode:%d\n",iIr_mode);
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#endif
	return 0;
}

int EX_GetIRGateway(void)
{
	int iIr_mode = g_gateway_info.ir_mode;//0-off 1-on
	//DBG_InfoMsg("EX_GetIRGateway gw mode = %d\n",iIr_mode);
	return iIr_mode;
}

int EX_SetMulticastStatus(char * ip,int ttl )
{
	//DBG_InfoMsg("ip=%s\n",ip);
	if(g_network_info.method == Net_MULTICAST)
	{
		Cfg_Set_Net_Multicast(ip,ttl);
		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_net_multicast::%s",ip);
		//DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		memset(sCmd,0,64);
		sprintf(sCmd,"e_p3k_net_ttl::%d",ttl);
		//DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	return 0;
}

int EX_RmEDID(int iEDID)
{
	int id = 2;
	if(id == iEDID)
	{
		return -1;
	}
		return 0;
}



int EX_SetPassword(char * login_level,char * iNew_Pass)
{
	DBG_InfoMsg("name=%s\n",login_level);

	if(!strcmp(login_level,"admin"))
		Cfg_Set_User_Pass(1,iNew_Pass);
	else if(!strcmp(login_level,"user"))
		Cfg_Set_User_Pass(0,iNew_Pass);
	else
		DBG_WarnMsg("EX_SetPassword login_level:%s is Wrong!!!\n",login_level);

	return 0;
}


int EX_GetPassword(char * login_level)
{
	char * admin = "admin";
	int pass=33333;
	if(0 == memcmp(login_level,admin,strlen(admin)))
	{
		DBG_InfoMsg("name=%s\n",login_level);
		return pass;
	}
	return 0;
}

int EX_SetGatewayPort(int iGw_Type,int iNetw_Id)
{
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if(((iGw_Type == Net_P3K)||(iGw_Type == Net_RS232)||(iGw_Type == Net_DANTE))
		&&((iNetw_Id == 0)||(iNetw_Id == 1)))
	{
		Cfg_Set_Net_GW_Port((NetGWType_E)iGw_Type,iNetw_Id);
		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_net_gw_port::%d::%d",iGw_Type,iNetw_Id);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		DBG_WarnMsg("EX_SetGatewayPort iGw_Type:%d is Wrong!!!\n",iGw_Type);
	}

	return 0;
}

int EX_GetGatewayPort(int iGw_Type)
{
	int iNetw_Id = 0;
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);
	if((iGw_Type == Net_P3K)||(iGw_Type == Net_RS232)||(iGw_Type == Net_DANTE))
	{
		Cfg_Get_Net_GW_Port((NetGWType_E)iGw_Type,&iNetw_Id);
	}
	else
	{
		DBG_WarnMsg("EX_GetGatewayPort iGw_Type:%d is Wrong!!!\n",iGw_Type);
	}

	return iNetw_Id;
}


// iGw_Type: 0 p3k; 1: RS232; 2: Dante
int EX_GetVlanTag(int iGw_Type)
{
	int iTag = 11;
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if((iGw_Type == Net_P3K)||(iGw_Type == Net_RS232)||(iGw_Type == Net_DANTE))
	{
		Cfg_Get_Net_GW_Vlan((NetGWType_E)iGw_Type,&iTag);
	}
	else
	{
		DBG_WarnMsg("EX_GetVlanTag iGw_Type:%d is Wrong!!!\n",iGw_Type);
	}

	return iTag;
}

// iGw_Type: 0 p3k; 1: RS232; 2: Dante
int EX_SetVlanTag(int iGw_Type,int iTag)
{
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if((iGw_Type == 0)||(iGw_Type == 1)||(iGw_Type == 2))
	{
		Cfg_Set_Net_GW_Vlan((NetGWType_E)iGw_Type,iTag);

		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_net_vlan::%d::%d",iGw_Type,iTag);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		printf("EX_SetVlanTag iGw_Type:%d is Wrong!!!\n",iGw_Type);
	}

	return 0;
}

int EX_SetMethod(int  mode )
{

	DBG_InfoMsg(">>EX_SetMethod %d\n",mode);

	if((mode == Net_UNICAST)||(mode == Net_MULTICAST))
	{
		char sCmd[32] = "";
		sprintf(sCmd,"e_p3k_net_method::%d",mode);

		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}

	Cfg_Set_Net_Method((NetMethodType_E)mode);
	return 0;
}
int EX_GetMethod(void)
{
	NetMethodType_E mode = Net_MULTICAST;

	Cfg_Get_Net_Method(&mode);
	return (int)mode;
}

int EX_SetOsdDisplay(int  mode )
{
	char setOSDDisplayCmd[BUFSIZE] = {0};
	char * str = "SET_OSD_DISPLAY";
	sprintf(setOSDDisplayCmd,"%s %d",str,mode);
	int ret = sendCmdtoGUI(setOSDDisplayCmd);
	DBG_InfoMsg(">>EX_SetOsdDisplay %d\n",ret);
	//osd mode {0	¨C off,1	¨C on,2	¨C display now,}
	return 0;
}
int EX_GetOsdDisplay(void)
{
	char getOSDDispalyCmd[BUFSIZE] = {"GET_OSD_DISPALY"};
	int mode = 0;
	mode = sendCmdtoGUI(getOSDDispalyCmd);
	//int mode =1;

	return mode;
}

int EX_SetDaisyChain(int  mode )
{
	DBG_InfoMsg(">>EX_SetDaisyChain %d\n",mode);
	if((mode == 0)||(mode == 1))
	{
		char sCmd[32] = "";
		sprintf(sCmd,"e_p3k_net_daisychain::%d",mode);

		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
		Cfg_Set_Net_DaisyChain((State_E)mode);
	}

	return 0;
}
int EX_GetDaisyChain(void)
{

	State_E daisy_state = OFF;
	Cfg_Get_Net_DaisyChain(&daisy_state);

	return (int)daisy_state;
}

int EX_GetVidMute(MuteInfo_S * mute)
{
	//int mute_mode = 0;
	//return mute_mode;
	State_E state;

	Cfg_Get_AV_Mute(&state);

	mute->state = state;
	return 0;
}

int EX_SetVidOutput(char info[][MAX_PARAM_LEN],int count )
{
	char sCmd[128] = "";
	int num = 0;
	for(num = 0;num < count;num ++)
	{
		DBG_InfoMsg("%s\n",info[num]);
	}

	PortSignalType_E port[4];
	int nCount = 0;
	sprintf(sCmd,"e_p3k_audio_dst");
	if(num == 0)
	{
		sprintf(sCmd,"%s::0",sCmd);
	}
	else
	{
		sprintf(sCmd,"%s::%d",sCmd,count);
		for(int i = 0; i < count;i++)
		{
			int out_id = atoi(info[i]);
			if(out_id == AUDIO_OUT_HDMI)
			{
				sprintf(sCmd,"%s::hdmi",sCmd);
				port[nCount] = PORT_HDMI;
				nCount++;
			}
			else if(out_id == AUDIO_OUT_ANALOG)
			{
				if(g_audio_info.direction == DIRECTION_OUT)
				{
					sprintf(sCmd,"%s::analog",sCmd);
					port[nCount] = PORT_ANALOG_AUDIO;
					nCount++;
				}
				else
				{
					DBG_WarnMsg("ERROR g_audio_info.direction == DIRECTION_IN\n");
					//return 0;
				}
			}
			else if(out_id == AUDIO_OUT_DANTE)
			{
				sprintf(sCmd,"%s::dante",sCmd);
				port[nCount] = PORT_DANTE;
				nCount++;
			}
			else if(out_id == AUDIO_OUT_STREAM)
			{
				sprintf(sCmd,"%s::lan",sCmd);
				port[nCount] = PORT_STREAM;
				nCount++;
			}
		}
	}



//	if(nCount > 0)
	{
		Cfg_Set_Audio_Dest(nCount,port);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	return 0;
}


int EX_GetVidOutput(char * date)
{
	char str[128] = "";

	DBG_InfoMsg("EX_GetVidOutput\n");

	for(int i = 0; i < 4; i++)
	{
		DBG_InfoMsg("EX_GetVidOutput i:%d, dst:%d\n",i,g_audio_info.dst_port[i]);
		if(g_audio_info.dst_port[i] == PORT_HDMI)
		{
			if(strlen(str) > 0)
				sprintf(str,"%s%d",str,AUDIO_OUT_HDMI);
			else
				sprintf(str,"%d",AUDIO_OUT_HDMI);
		}
		else if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
		{
			if(strlen(str) > 0)
				sprintf(str,"%s%d",str,AUDIO_OUT_ANALOG);
			else
				sprintf(str,"%d",AUDIO_OUT_ANALOG);

		}
		else if(g_audio_info.dst_port[i] == PORT_STREAM)
		{
			if(strlen(str) > 0)
				sprintf(str,"%s%d",str,AUDIO_OUT_STREAM);
			else
				sprintf(str,"%d",AUDIO_OUT_STREAM);
		}
		else if(g_audio_info.dst_port[i] == PORT_DANTE)
		{
			if(strlen(str) > 0)
				sprintf(str,"%s%d",str,AUDIO_OUT_DANTE);
			else
				sprintf(str,"%d",AUDIO_OUT_DANTE);
		}
		else
		{
			break;
		}

		sprintf(str,"%s,",str);

		DBG_InfoMsg("EX_GetVidOutput str:%s\n",str);
	}

	strcpy(date,str);

	DBG_InfoMsg("EX_GetVidOutput %s\n",date);

	return 0;
}

int EX_GetHWVersion(char * date)
{
	//char * version = "1.12.123";
	//memcpy(date,version,strlen(version));

	if(date != NULL)
	{
		GetBoardInfo(BOARD_HW_VERSION, date, 24);
	}
	return 0;
}

int EX_GetStandbyVersion(char * date)
{
	//char * version = "1.12.123";
	//memcpy(date,version,strlen(version));

	strcpy(date,g_version_info.standby_version);
	return 0;
}

int EX_GetDevStatus(void)
{
	int status =0;
	//0 power on
	// 1 standby
	// 2 FW Background Download
	// 3  IP fallback address
	return status;
}
int EX_GetHWTemp(int  id,int iMode)
{
	int iTemp = 0;
	if(iMode == 0){
	iTemp = 50;
		}
	return iTemp;
}

int EX_GetUPGTime(char * day,char * time)
{
//	char *str = "05-12-2018";
//	char *str1 = "14:30:00";
//	memcpy(day,str,strlen(str));
//	memcpy(time,str1,strlen(str1));

	DBG_InfoMsg(" EX_GetUPGTime\n");
	sscanf(g_version_info.upg_time,"%s,%s",day,time);

	return 0;
}

int EX_GetChannelName(char * date)
{
#ifdef CONFIG_P3K_HOST
		Cfg_Get_EncChannel_Name(date);
#else
		DBG_InfoMsg(" !!! This is Decoder\n");
#endif

//	char name[32] = "bbc";
//	memcpy(date,name,strlen(name));
	return 0;
}

int EX_GetDanteName(char * date)
{

	char name[32] = "KDS-7-MAC";
	memcpy(date,name,strlen(name));
	return 0;
}

int EX_GetSendMsgNum(int msg,char*date )
{
	int iMsgNum = 100;

	return iMsgNum;
}

int EX_GetActiveEDID(int input_ID )
{
	int index = 1;

	Cfg_Get_EDID_Active(&index);
	return index;
}

int EX_GetEdidList(char info[][MAX_EDID_LEN],int num)
{
 	int tmpnum = 0;
	//char *str ="[0,DEFAULT]";
	//char *str1= "[2,SONY]";
	//char *str2= "[5,PANASONIC]";
	//memcpy(info[0],str,strlen(str));
	//memcpy(info[1],str1,strlen(str1));
	//memcpy(info[2],str1,strlen(str2));

	tmpnum = Cfg_Get_EDID_List(info,num);
	return tmpnum;
}


int EX_GetAudGainLevel(PortInfo_S*info,int *gain)
{
	//int tmpGain = 20;
	// *gain = tmpGain;

	Cfg_Get_AV_Volume(gain);
	return  0;
}
int EX_GetAudParam(PortInfo_S*info,AudioSignalInfo_S*param)
{
	param->chn = 2;
	memset(param->format,0,16);
	param->sampleRate= SAMPLE_RATE_NONE;

#ifdef CONFIG_P3K_HOST
	char* cmd_ch = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Valid Ch: (.*).*$#\\1#gp'";
	char* cmd_type = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Type: (.*).*$#\\1#gp'";
	char* cmd_freq = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Sample Freq: (.*)KHz.*$#\\1#gp'";
#else
	char* cmd_ch = "cat /sys/devices/platform/1500_i2s/output_audio_info | sed -rn 's#^.*Valid Ch: (.*).*$#\\1#gp'";
	char* cmd_type = "cat /sys/devices/platform/1500_i2s/output_audio_info | sed -rn 's#^.*Type: (.*).*$#\\1#gp'";
	char* cmd_freq = "cat /sys/devices/platform/1500_i2s/output_audio_info | sed -rn 's#^.*Sample Freq: (.*)KHz.*$#\\1#gp'";
#endif
	char buf_ch[16] = "";
	char buf_type[16] = "";
	char buf_freq[16] = "";

	mysystem(cmd_ch, buf_ch, 16);
	mysystem(cmd_type, buf_type, 16);
	mysystem(cmd_freq, buf_freq, 16);

	param->chn = atoi(buf_ch);
	sscanf(buf_type,"%s (%*s)",param->format);

	DBG_InfoMsg("EX_GetAudParam param->format %s\n",param->format);

	int rate = atoi(buf_freq);
	if(rate == 8)
		param->sampleRate = SAMPLE_RATE_8000;
	else if(rate == 16)
		param->sampleRate = SAMPLE_RATE_16000;
	else if(rate == 22)
		param->sampleRate = SAMPLE_RATE_22500;
	else if(rate == 32)
		param->sampleRate = SAMPLE_RATE_32000;
	else if(rate == 44)
		param->sampleRate = SAMPLE_RATE_44100;
	else if(rate == 48)
		param->sampleRate = SAMPLE_RATE_48000;

	return 0;
}
int EX_SetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E mode)
{
	DBG_InfoMsg("EX_SetAutoSwitchMode mode =%d\n",mode);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(info->signal == SIGNAL_VIDEO)
		sprintf(sCmd,"e_p3k_switch_mode");
	else if(info->signal == SIGNAL_AUDIO)
		sprintf(sCmd,"e_p3k_audio_mode");
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal=%d\n",info->signal);
		return 0;
	}

	if(mode == CONNECT_MANUAL)
		sprintf(sCmd,"%s::manual",sCmd);
	else if(mode == CONNECT_PRIROITY)
		sprintf(sCmd,"%s::priority",sCmd);
	else if(mode == CONNECT_LAST)
		sprintf(sCmd,"%s::FILO",sCmd);
	else
	{
		DBG_WarnMsg(" !!!parameter Error mode = %d\n",mode);
		return 0;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Mode(info->signal,mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E *mode)
{
	if((info->signal == SIGNAL_VIDEO)||(info->signal == SIGNAL_AUDIO))
	{
		Cfg_Get_Autoswitch_Mode(info->signal,mode);
	}
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal = %d\n",info->signal);
		return 0;
	}

	return 0;
}

//#X-PRIORITY <direction_type>. <port_format>. <port_index> .<signal_type> , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR>
int EX_SetAutoSwitchPriority(AudioInfo_S * info,AudioInfo_S * gain,int count)
{
	DBG_InfoMsg("EX_SetAutoSwitchPriority,Count = %d\n",count);
	if(count == 0)
	{
		DBG_WarnMsg(" !!! Error Count = %d\n",count);
		return 0;
	}

#ifdef CONFIG_P3K_HOST
	int port[4] = {255,255,255,255};
	char sCmd[128] = "";
	if(gain[0].signal == SIGNAL_VIDEO)
	{
		sprintf(sCmd,"e_p3k_switch_pri");

		for(int i = 1;i < count;i++)
		{
			sprintf(sCmd,"%s::HDMI%d",sCmd,gain[i].portIndex);
			port[i] = gain[i].portIndex;
		}
	}
	else if(gain[0].signal == SIGNAL_AUDIO)
	{
		sprintf(sCmd,"e_p3k_audio_pri");

		for(int i = 1; i < count;i++)
		{
			if(gain[i].portFormat == PORT_HDMI)
			{
				sprintf(sCmd,"%s::hdmi",sCmd);
				port[i] = AUDIO_IN_HDMI;
			}
			else if(gain[i].portFormat == PORT_ANALOG_AUDIO)
			{
				//if(g_audio_info.direction == DIRECTION_IN)
				{
					sprintf(sCmd,"%s::analog",sCmd);
					port[i] = AUDIO_IN_ANALOG;
				}
				//else
				//{
				//	DBG_WarnMsg(" !!! g_audio_info.direction == DIRECTION_OUT \n");
				//	break;
				//}
			}
			else if(gain[i].portFormat == PORT_DANTE)
			{
				sprintf(sCmd,"%s::dante",sCmd);
				port[i] = AUDIO_IN_DANTE;
			}
		}
	}
	else
	{
		DBG_WarnMsg(" !!!EX_SetAutoSwitchPriority parameter Error\n");
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Priority(gain[0].signal,port[1],port[2],port[3]);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif

	return 0;
}

int EX_GetAutoSwitchPriority(AudioInfo_S * gain,int count)
{
	int num = 0;
#ifdef CONFIG_P3K_HOST
	SignalType_E type = gain[0].signal;
	if((gain[0].signal == SIGNAL_VIDEO)||(gain[0].signal == SIGNAL_AUDIO))
	{
		int port[4] = {255,255,255,255};
		Cfg_Get_Autoswitch_Priority(gain[0].signal,&port[1],&port[2],&port[3]);
		DBG_InfoMsg("EX_GetAutoSwitchPriority %d,%d,%d\n",port[1],port[2],port[3]);

		for(int i = 1; i<=3; i++)
		{
			if(type == SIGNAL_VIDEO)
			{
				gain[i].direction = DIRECTION_IN;
				gain[i].signal = SIGNAL_VIDEO;
				if((port[i] == 1)||(port[i] == 2))
				{
					gain[i].portFormat = PORT_HDMI;
					gain[i].portIndex = port[i];
					num++;
				}
				else if(port[i] == 3)
				{
					gain[i].portFormat = PORT_USB_C;
					gain[i].portIndex = port[i];
					num++;
				}
				else
				{
					num = i + 1;
				 	break;
				}
			}
			else
			{
				gain[i].direction = DIRECTION_IN;
				gain[i].signal = SIGNAL_AUDIO;
				if(port[i] == AUDIO_IN_HDMI)
				{
					gain[i].portFormat = PORT_HDMI;
					gain[i].portIndex = 1;
					num++;
				}
				else if(port[i] == AUDIO_IN_ANALOG)
				{
					gain[i].portFormat = PORT_ANALOG_AUDIO;
					gain[i].portIndex = 1;
					num++;
				}
				else if(port[i] == AUDIO_IN_DANTE)
				{
					gain[i].portFormat = PORT_DANTE;
					gain[i].portIndex = 1;
					num++;
				}
				else
				{
					num = i + 1;
				 	break;
				}
			}
		}
	}
	else
	{
		DBG_WarnMsg(" !!!EX_GetAutoSwitchPriority parameter Error\n");
		return 0;
	}
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif

/*
	pid_t status;
	//int ret =0;
	int num = 0;
	char buf[24] = {0};
	char * p;
	char m[] = " ";
	FILE * ptr;
	char * cmd2 = "./sconfig --show priority";
	status = system(cmd2);
	if((ptr = popen(cmd2,"r")) != NULL)
	{
		fgets(buf,24,ptr);
		pclose(ptr);
	}
	p = strtok(buf,m);
	while(p)
	{
		//printf(">>>%s\n",p);
		if(!memcmp(p,"HDMI3",strlen("HDMI3")))
		{
			gain[num].direction = DIRECTION_IN;
			gain[num].portFormat = PORT_USB_C;
			gain[num].portIndex = 3;
			gain[num].signal = SIGNAL_VIDEO;
		}
		else if(!memcmp(p,"HDMI2",strlen("HDMI2")))
		{
			gain[num].direction = DIRECTION_IN;
			gain[num].portFormat = PORT_HDMI;
			gain[num].portIndex = 2;
			gain[num].signal = SIGNAL_VIDEO;
		}
		else if(!memcmp(p,"HDMI1",strlen("HDMI1")))
		{
			gain[num].direction = DIRECTION_IN;
			gain[num].portFormat = PORT_HDMI;
			gain[num].portIndex = 1;
			gain[num].signal = SIGNAL_VIDEO;
		}
		num += 1;
		p = strtok(NULL,m);
	}
*/	return num;
}
int EX_CopyEDID(EDIDPortInfo_S*src,EDIDPortInfo_S*dest,int bitMap,int safemode)
{

	return 0;
}
int EX_GetEDIDSupport(int index ,int mode ,int *size)
{
	*size = 256;
	return 0;
}
int EX_SetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E mode)
{
	DBG_InfoMsg("EX_SetEDIDColorSpaceMode mode =%d\n",mode);
	return 0;
}
int EX_GetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E *mode)
{

	 *mode = COLOR_RGB;
	return 0;
}

int EX_SetEDIDLockStatus(int index,int lock)
{
	DBG_InfoMsg("EX_SetEDIDLockStatus index =%d lock=%d\n",index,lock);
#ifdef CONFIG_P3K_HOST
	Cfg_Set_EDID_Lock((State_E)lock);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
return 0;
}
int EX_GetEDIDLockStatus(int index,int *lock)
{
#ifdef CONFIG_P3K_HOST
	State_E tmp_lock;
	Cfg_Get_EDID_Lock(&tmp_lock);

	*lock = (int)tmp_lock;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_SetHDCPMode(int index,HDCPMode_E mode)
{
	DBG_InfoMsg("EX_SetHDCPMode index=%d mode=%d\n",index,mode);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";

	if((mode == HDCP_OFF)||(mode == HDCP_ON)/*||(mode == HDCP_MIRROR)*/)
	{
		sprintf(sCmd,"e_p3k_video_hdcp_mode::%d::%d",index,mode);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	Cfg_Set_AV_HDCP(index, (State_E)mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetHDCPMode(int index,HDCPMode_E *mode)
{
#ifdef CONFIG_P3K_HOST
	State_E tmp_mode;
	Cfg_Get_AV_HDCP(index, &tmp_mode);

	*mode = (HDCPMode_E)tmp_mode;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetHDCPStatus(int io,int index)
{
	int status = 1;
#ifdef CONFIG_P3K_HOST
	State_E tmp_mode;

	char* cmd1 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*HDCP: (.*).*$#\\1#gp'";
	char buf1[16] = "";

	mysystem(cmd1,buf1,16);

	if(strstr(buf1,"Off") != 0)
		status = 0;
	else
		status = 1;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif

	return status;
}

int EX_SetViewMode(ViewMode_E mode,ViewModeInfo_S*info)
{
	DBG_InfoMsg("EX_SetViewMode h = %d v = %d\n",info->hStyle,info->vStyle);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((mode == VIEW_MODE_VIDEOWALL)
		&&(info->hStyle >= 1)
		&&(info->hStyle <= 16)
		&&(info->vStyle >= 1)
		&&(info->vStyle <= 16))
	{
		sprintf(sCmd,"e_vw_pos_layout_%d_%d",(info->vStyle-1),(info->hStyle-1));

		//sprintf(sCmd,"e_p3k_video_vw_mode::%d::%d",info->hStyle,info->vStyle);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_VM_Mod(info->hStyle,info->vStyle);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d,h = %d,v = %d\n",mode,info->hStyle,info->vStyle);
	}
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetViewMode(ViewMode_E *mode,ViewModeInfo_S*info)
{
    *mode = VIEW_MODE_VIDEOWALL;
	Cfg_Get_VM_Mod(&(info->hStyle),&(info->vStyle));

	return 0;
}

int EX_SetWndBezelInfo( int mode ,int index, WndBezelinfo_S*info)
{
	DBG_InfoMsg("EX_SetWndBezelInfo =%d %d %d %d\n",info->hValue,info->vValue,info->hOffset,info->vOffset);
	return 0;

#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((index >= 1)&&(index <= 256))
	{
		sprintf(sCmd,"e_vw_h_shift_l_%d",info->hOffset);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		memset(sCmd,0,64);
		sprintf(sCmd,"e_vw_h_shift_u_%d",info->vOffset);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_VM_Bezel(index,*info);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para index:%d\n",index);
	}


#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetWndBezelInfo( int *mode ,int *index, WndBezelinfo_S*info)
{
	*mode = 0;
	Cfg_Get_VM_Bezel(index,info);
	return 0;
}
int EX_SetVideoWallSetupInfo(int id,VideoWallSetupInfo_S *info)
{
	DBG_InfoMsg("EX_SetVideoWallSetupInfo id =%d,rotaion=%d\n",id,info->rotation);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((id >= 1) &&(id <= 256) &&(info->rotation <= 3))
	{
		int max_id = g_videowall_info.horizontal_count * g_videowall_info.vertical_count;

		if(id <= max_id)
		{
			int max_row = g_videowall_info.vertical_count;
			int max_col = g_videowall_info.horizontal_count;
			int row = (id-1) / max_col;
			int col = (id-1) % max_col;
			sprintf(sCmd,"e_vw_enable_%d_%d_%d_%d",(max_row-1),(max_col-1),row,col);
			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);

			if(info->rotation == ROTATION_180)
			{
				ast_send_event(0xFFFFFFFF,"e_vw_rotate_3");
			}
			else if(info->rotation == ROTATION_270)
			{
				ast_send_event(0xFFFFFFFF,"e_vw_rotate_6");
			}
			else if(info->rotation == ROTATION_90)
			{
				ast_send_event(0xFFFFFFFF,"e_vw_rotate_5");
			}
			else
			{
				ast_send_event(0xFFFFFFFF,"e_vw_rotate_0");
			}

			Cfg_Set_VM_Setup(id,info->rotation);
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para id:%d,info->rotation:%d\n",id,info->rotation);
	}

#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoWallSetupInfo(int* id,VideoWallSetupInfo_S *info)
{
	int rotation = 0;
	Cfg_Get_VM_Setup(id,&rotation);

	info->rotation = (VideoRotation_E)rotation;
	return 0;
}
int EX_StartOverlay(char*confFile,int outtime)
{
	char startOverlayCmd[BUFSIZE] = {0};
	sprintf(startOverlayCmd,"START_OVERLAY %s %d",confFile,outtime);
	int mode = sendCmdtoGUI(startOverlayCmd);
	DBG_InfoMsg("filename = %s, outtime =%d mode =%d\n",confFile,outtime,mode);
	return 0;
}
int EX_StopOverlay(void)
{
	const char stopOverlay[BUFSIZE] = {"STOP_OVERLAY"};

	int mode = sendCmdtoGUI(stopOverlay);
	DBG_InfoMsg(" mode =%d\n",mode);
	return 0;
}

int EX_SetEncoderAVChannelId(int id)
{
	DBG_InfoMsg("EX_SetEncoderAVChannelId id=%d\n",id);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_reconnect::%04d",id);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_EncChannel_ID(id);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetEncoderAVChannelId(int *id)
{
#ifdef CONFIG_P3K_HOST
	Cfg_Get_EncChannel_ID(id);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_SetDecoderAVChannelId(ChSelect_S * id)
{
	DBG_InfoMsg(" EX_SetDecoderAVChannelId id =%d\n",id->ch_id);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	sprintf(sCmd,"e_reconnect::%04d",id->ch_id);

	if(id->signal == SIGNAL_IR)
		sprintf(sCmd,"%s::r",sCmd);

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetDecoderAVChannelId(ChSelect_S * id)
{
	DBG_InfoMsg(">>%d\n",id->signal);

	char* cmd1 = "astparam g ch_select_v";
	char buf1[64] = "";

	mysystem(cmd1,buf1,16);

	if(strstr(buf1,"not defined") != 0)
	{
		id->ch_id = 1;
		DBG_WarnMsg("EX_GetDecoderAVChannelId not defined\n");
	}
	else
	{
		id->ch_id = atoi(buf1);
		DBG_InfoMsg("EX_GetDecoderAVChannelId id = %d\n",id->ch_id);
	}

	return 0;
}
int EX_SetVideoImageStatus(int scalerId,VideoStatusType_E status)
{
	DBG_InfoMsg(" EX_SetVideoImageStatus status =%d\n",status);
	return 0;
}
int EX_GetVideoImageStatus(int scalerId,VideoStatusType_E *status)
{
	*status = VIDEO_ENABLED;
	return 0;
}

int EX_SetVideoCodecAction(CodecActionType_E type)
{
	DBG_InfoMsg("EX_SetVideoCodecAction type=%d\n",type);
	char sCmd[64] = "";
	if(type == CODEC_ACTION_PLAY)
		sprintf(sCmd,"e_reconnect");
	else if(type == CODEC_ACTION_STOP)
		sprintf(sCmd,"e_stop_link");
	else
	{
		DBG_WarnMsg(" !!! Error para %d\n",type);
		return 0;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_AV_Action(type);

	return 0;
}

int EX_GetVideoCodecAction(CodecActionType_E *type)
{
	Cfg_Get_AV_Action(type);
	return 0;
}
int EX_SetColorSpaceConvertMode(int index,int convertMode)
{
	DBG_InfoMsg("EX_SetColorSpaceConvertMode id = %d mode =%d\n",index,convertMode);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	if((convertMode == 0)||(convertMode == 1))
	{
		sprintf(sCmd,"e_p3k_video_rgb::%d",convertMode);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para %d\n",convertMode);
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Video_RGB(convertMode);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetColorSpaceConvertMode(int index,int *convertMode)
{
#ifdef CONFIG_P3K_CLIENT
	Cfg_Set_Video_RGB(*convertMode);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}

//e_p3k_video_scale::pass|1080p60|1080p50|2160p30|2160p25|720p60
int EX_SetVideoImageScaleMode(int mode,int res)
{
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	if(mode == 0)
		sprintf(sCmd,"e_p3k_video_scale::pass");
	else if(mode == 1)
	{
		if(res == 74)//2160p30
			sprintf(sCmd,"e_p3k_video_scale::2160p30");
		else if(res == 73)//2160p25
			sprintf(sCmd,"e_p3k_video_scale::2160p25");
		else if(res == 16)//1080p60
			sprintf(sCmd,"e_p3k_video_scale::1080p60");
		else if(res == 31)//1080p50
			sprintf(sCmd,"e_p3k_video_scale::1080p50");
		else if(res == 4)//720p60
			sprintf(sCmd,"e_p3k_video_scale::720p60");
		else
		{
			DBG_WarnMsg(" !!! Error para res:%d\n",res);
			return 0;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
		return 0;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
#endif
	return 0;
}



char* strGetResolution[] = {
						 (char*)"NULL",
                         (char*)"[640]X[480] [60] [P]",
						 (char*)"[720]X[480] [60] [P]",
						 (char*)"[720]X[480] [60] [P]",
						 (char*)"[1280]X[720] [60] [P]",
						 (char*)"[1920]X[1080] [60] [I]",
						 (char*)"[720]X[480] [60] [I]",
						 (char*)"[720]X[480] [60] [I]",
						 (char*)"[720]X[240] [60] [P]",
						 (char*)"[720]X[240] [60] [P]",
						 (char*)"[2880]X[480] [60] [I]",
						 (char*)"[2880]X[480] [60] [I]",
						 (char*)"[2880]X[240] [60] [P]",
						 (char*)"[2880]X[240] [60] [P]",
						 (char*)"[1440]X[480] [60] [P]",
						 (char*)"[1440]X[480] [60] [P]",
						 (char*)"[1920]X[1080] [60] [P]",
						 (char*)"[720]X[576] [50] [P]",
						 (char*)"[720]X[576] [50] [P]",
						 (char*)"[1280]X[720] [50] [P]",
						 (char*)"[1920]X[1080] [50] [I]",
						 (char*)"[720]X[576] [50] [I]",
						 (char*)"[720]X[576] [50] [I]",
						 (char*)"[720]X[288] [50] [P]",
						 (char*)"[720]X[288] [50] [P]",
						 (char*)"[2880]X[576] [50] [I]",
						 (char*)"[2880]X[576] [50] [I]",
						 (char*)"[2880]X[288] [50] [P]",
						 (char*)"[2880]X[288] [50] [P]",
						 (char*)"[1440]X[576] [50] [P]",
						 (char*)"[1440]X[576] [50] [P]",
						 (char*)"[1920]X[1080] [50] [P]",
						 (char*)"[1920]X[1080] [24] [P]",
						 (char*)"[1920]X[1080] [25] [P]",
						 (char*)"[1920]X[1080] [30] [P]",
						 (char*)"[2880]X[480] [60] [P]",
						 (char*)"[2880]X[480] [60] [P]",
						 (char*)"[2880]X[576] [50] [P]",
						 (char*)"[2880]X[576] [50] [P]",
						 (char*)"[1920]X[1080] [50] [I]",
						 (char*)"[1920]X[1080] [100] [I]",
						 (char*)"[1280]X[720] [100] [P]",
						 (char*)"[720]X[576] [100] [P]",
						 (char*)"[720]X[576] [100] [P]",
						 (char*)"[720]X[576] [100] [I]",
						 (char*)"[720]X[576] [100] [I]",
						 (char*)"[1920]X[1080] [120] [I]",
						 (char*)"[1280]X[720] [120] [P]",
						 (char*)"[720]X[480] [120] [P]",
						 (char*)"[720]X[480] [120] [P]",
						 (char*)"[720]X[480] [120] [I]",
						 (char*)"[720]X[480] [120] [I]",
						 (char*)"[720]X[576] [200] [P]",
						 (char*)"[720]X[576] [200] [P]",
						 (char*)"[720]X[576] [200] [I]",
						 (char*)"[720]X[576] [200] [I]",
						 (char*)"[720]X[480] [240] [P]",
						 (char*)"[720]X[480] [240] [P]",
						 (char*)"[720]X[480] [240] [I]",
						 (char*)"[720]X[480] [240] [I]",
						 (char*)"[1280]X[720] [24] [P]",
						 (char*)"[1280]X[720] [25] [P]",
						 (char*)"[1280]X[720] [30] [P]",
						 (char*)"[1920]X[1080] [120] [P]",
						 (char*)"[1920]X[1080] [100] [P]",
						 (char*)"[800]X[600] [60] [P]",
						 (char*)"[1024]X[768] [60] [P]",
						 (char*)"[1280]X[768] [60] [P]",
						 (char*)"[1280]X[1024] [60] [P]",
						 (char*)"[1600]X[1200] [60] [P]",
						 (char*)"[1680]X[1050] [60] [P]",
						 (char*)"[1920]X[1200] [60] [P]",
						 (char*)"[3840]X[2160] [24] [P]",
						 (char*)"[3840]X[2160] [25] [P]",
						 (char*)"[3840]X[2160] [30] [P]",
						 (char*)"[3840]X[2160] [50] [P]",
						 (char*)"[3840]X[2160] [60] [P]",
                         };

int EX_GetVideoImageScaleMode(int *mode,char*res)
{
	*mode = 1;
	strcpy(res,"16");
	return 0;
}
int EX_GetVideoViewReslotion(int mode, int index, int nativeFlag,int * res)
{
	int a = 254;

	// Capture Windows: [1280]X[1024] [75]Hz

	char* cmd1 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*Capture Windows:(.*)Hz.*$#\\1#gp'";
	char* cmd2 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*Scan Mode: (.*).*$#\\1#gp'";
	char buf1[64] = "";
	char buf2[64] = "";

	mysystem(cmd1,buf1,64);
	mysystem(cmd2,buf2,64);

	if(strcmp("Progressive",buf2) == 0)
		sprintf(buf1,"%s [P]",buf1);
	else
		sprintf(buf1,"%s [I]",buf1);

	DBG_InfoMsg("EX_GetVideoViewReslotion %s\n",buf1);

	for(int i = 0; i <= 76; i++)
	{
		//printf("index: %d, res: %s\n",i,strGetResolution[i]);
		if(strstr(buf1,strGetResolution[i]) != 0)
		{
			a = i;
			break;
		}
	}

	*res = a;

	return 0;
}
int EX_GetVideoFrameRate(int *fps)
{
	*fps = 60;
	return 0;
}
int EX_GetVideoBitRate(int *kbps)
{
      *kbps=1000;
	return 0;
}

int EX_SendCECMsg(CECMessageInfo_S*info)
{
	int sendStatus = 0;
	char sCmd[128] = "";
	if((info->hexByte > 0)&&(info->hexByte <= 16))
	{
		sprintf(sCmd,"e_p3k_cec_send::%s",info->cmdComent);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para info->hexByte:%d\n",info->hexByte);
		return 0;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	return sendStatus;
}

int EX_RecvCECNtfy(int *port,int*hexByte,char*cmdComment)
{
	*port = 0;
	*hexByte = 2;
	strcpy(cmdComment,"AB01");
	return 0;
}
int EX_SetCECGateWayMode(int mode)
{
	printf("EX_SetCECGateWayMode mode =%d\n",mode);
	char sCmd[64] = "";
	if((mode >= 0)&&(mode<=3))
	{
		sprintf(sCmd,"e_p3k_cec_gw::%d",mode);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_GW_CEC_Mode(mode);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
		return 0;
	}

	return 0;
}

int EX_GetCECGateWayMode(void)
{
	int mode = 0;

	Cfg_Get_GW_CEC_Mode(&mode);
	return mode;
}

int EX_SendIRmessage(IRMessageInfo_S*info)
{
	DBG_InfoMsg("EX_SendIRmessage cmdname =%s cmdcomment =%s\n",info->cmdName,info->cmdComent);
	char sCmd[128] = "";

	sprintf(sCmd,"e_p3k_ir_send::%s",info->cmdComent);

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);	return 0;
}
int EX_SendIRStop(int irId,int serialNumb,char*command)
{
	int status = 0;
	return status;
}
int EX_SetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo,int num)
{
	char sCmd[128] = "";

	DBG_InfoMsg("EX_SetRouteMatch  num=====%d  %d   %d\n",num, inPortInfo[2].index,matchPortInfo->index);

#ifdef CONFIG_P3K_HOST
	if(matchPortInfo->signal == SIGNAL_VIDEO)
	{
		if((matchPortInfo->portIndex >= 1)&&(matchPortInfo->portIndex <= 3))
		{
			sprintf(sCmd,"e_p3k_switch_in::HDMI%d",matchPortInfo->portIndex);
			Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,matchPortInfo->portIndex);
		}
	}
	else if(matchPortInfo->signal == SIGNAL_AUDIO)
	{
		PortSignalType_E port[4];
		int nCount = 0;
		sprintf(sCmd,"e_p3k_audio_dst");
		if(num == 0)
		{
			sprintf(sCmd,"%s::0",sCmd);
		}
		else
		{
			sprintf(sCmd,"%s::%d",sCmd,num);
		}

		for(int i = 0; i < num;i++)
		{
			if(inPortInfo[i].portFormat == PORT_HDMI)
			{
				sprintf(sCmd,"%s::hdmi",sCmd);
				port[nCount] = PORT_HDMI;
				nCount++;
			}
			else if(inPortInfo[i].portFormat == PORT_ANALOG_AUDIO)
			{
				if(g_audio_info.direction == DIRECTION_OUT)
				{
					sprintf(sCmd,"%s::analog",sCmd);
					port[nCount] = PORT_ANALOG_AUDIO;
					nCount++;
				}
				else
				{
					printf("ERROR g_audio_info.direction == DIRECTION_IN\n");
				//	return 0;
				}
			}
			else if(inPortInfo[i].portFormat == PORT_DANTE)
			{
				sprintf(sCmd,"%s::dante",sCmd);
				port[nCount] = PORT_DANTE;
				nCount++;
			}
			else if(inPortInfo[i].portFormat == PORT_STREAM)
			{
				sprintf(sCmd,"%s::lan",sCmd);
				port[nCount] = PORT_STREAM;
				nCount++;
			}
		}

		if(nCount > 0)
			Cfg_Set_Audio_Dest(nCount,port);

	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	if(matchPortInfo->signal == SIGNAL_VIDEO)
	{
		if(matchPortInfo->portFormat == PORT_HDMI)
		{
			sprintf(sCmd,"e_p3k_switch_in::HDMI");
			Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,1);
		}
		else if(matchPortInfo->portFormat == PORT_STREAM)
		{
			sprintf(sCmd,"e_p3k_switch_in::STREAM");
			Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,2);
		}
		else if(matchPortInfo->signal == SIGNAL_TEST)
		{
		}
		else
		{
			DBG_WarnMsg("!!! Error matchPortInfo->portFormat %d\n",matchPortInfo->portFormat);
			return -1;
		}

		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else if(matchPortInfo->signal == SIGNAL_TEST)
	{
		printf("!!! matchPortInfo->signal == SIGNAL_TEST\n");

	}


#endif
	return 0;
}

int EX_GetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo)
{
	if((inPortInfo->signal == SIGNAL_VIDEO)
		&&(inPortInfo->direction == DIRECTION_OUT))
	{
		DBG_InfoMsg("EX_GetRouteMatch\n");
#ifdef CONFIG_P3K_HOST
		pid_t status;
		//int ret =0;
		char buf[24] = {0};
		char * p;
		char m[] = " ";
		FILE * ptr;
		char * cmd2 = "/usr/local/bin/sconfig --show input";
		status = system(cmd2);
		if((ptr = popen(cmd2,"r")) != NULL)
		{
			fgets(buf,24,ptr);
			pclose(ptr);
		}
		p = strtok(buf,m);

		DBG_InfoMsg("EX_GetRouteMatch buf %s p %s\n",buf,p);
		matchPortInfo->signal = SIGNAL_VIDEO;
		if(!memcmp(p,"HDMI3",strlen("HDMI3")))
		{
			matchPortInfo->direction = DIRECTION_IN;
			matchPortInfo->portFormat = PORT_USB_C;
			matchPortInfo->portIndex = 3;
			matchPortInfo->index = 1;
		}
		else if(!memcmp(p,"HDMI2",strlen("HDMI2")))
		{
			matchPortInfo->direction = DIRECTION_IN;
			matchPortInfo->portFormat = PORT_HDMI;
			matchPortInfo->portIndex = 2;
			matchPortInfo->index = 1;
		}
		else if(!memcmp(p,"HDMI1",strlen("HDMI1")))
		{
			matchPortInfo->direction = DIRECTION_IN;
			matchPortInfo->portFormat = PORT_HDMI;
			matchPortInfo->portIndex = 1;
			matchPortInfo->index = 1;
		}
#else
	int port = 0;
	Cfg_Get_Autoswitch_Source(SIGNAL_VIDEO,&port);
	matchPortInfo->direction = DIRECTION_IN;
	matchPortInfo->portIndex = 1;
	matchPortInfo->index = 1;

	if(port == 1)
		matchPortInfo->portFormat = PORT_HDMI;
	else
		matchPortInfo->portFormat = PORT_STREAM;
#endif
	}
	else
	{
		// ???
	}
	return 0;
}

int EX_SetUartConf(UartMessageInfo_S*conf)
{
	Cfg_Set_GW_Uart_Param(*conf);

	char cmd[128] = "";
	if(conf->parity == PARITY_ODD)
		sprintf(cmd,"astparam s s0_baudrate %d-%do%d;astparam save",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);
	else if(conf->parity == PARITY_EVEN)
		sprintf(cmd,"astparam s s0_baudrate %d-%de%d;astparam save",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);
	else
		sprintf(cmd,"astparam s s0_baudrate %d-%dn%d;astparam save",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);

	system(cmd);
	return 0;
}
int EX_GetUartConf(int comId,UartMessageInfo_S*conf)
{
	UartMessageInfo_S tmpconf;

	Cfg_Get_GW_Uart_Param(&tmpconf);

	conf->bitWidth = tmpconf.bitWidth;
	conf->comNumber = comId;
	conf->rate = tmpconf.rate;
	conf->stopBitsMode = tmpconf.stopBitsMode;
	conf->serialType =0;
	conf->term_485 = 0;
	conf->parity = tmpconf.parity;

	return 0;
}
int EX_AddComRoute(ComRouteInfo_S*info,int comId)
{
	Cfg_Set_GW_COM_Add(info->portNumber);

	system("astparam s no_soip n;astparam save");

	return 0;
}
int EX_RemoveComRoute(int comId)
{
	Cfg_Set_GW_COM_Remove();

	system("astparam s no_soip y;astparam save");
	return 0;
}
int EX_GetComRoute(int comId,ComRouteInfo_S*info)
{
	State_E mode;
	int port;

	Cfg_Get_GW_COM_Status(&mode,&port);
	if(mode == ON)
	{
		info->HeartTimeout = 60;
		info->portNumber = port;
		info->portType = 1;
		info->rePlay = 0;
	}
	else
	{
		info->HeartTimeout = 0;
		info->portNumber = 0;
		info->portType = 0;
		info->rePlay = 0;
		return -1;
	}
	return 0;
}
int EX_GetOpenTunnelParam(int tunnelId,TunnelParam_S*param)
{

	param->portNumber= 50001;
	param->portType = 0;
	param->connectType = 0;
	param->reMoteNumber = 4000;
	param->rePlay = 0;
	strcpy(param->ipAddr,"192.168.0.122");
	strcpy(param->comName,"COM");
	return 0;
}
int EX_SetUSBCtrl(int type)
{
    DBG_InfoMsg("EX_SetUSBCtrl =%d\n",type);
	return 0;
}
int EX_GetMulticastInfo(char*ip,int *ttl)
{
	Cfg_Get_Net_Multicast(ip,ttl);
	return 0;
}

int EX_SetMacAddr(int netid,char*macAddr)
{
	DBG_InfoMsg("EX_SetMacAddr mac =%s\n",macAddr);
	return 0;
}
int EX_GetMacAddr(int netid,char*macAddr)
{
//	strcpy(macAddr,"00-14-22-01-23-45");
	DBG_InfoMsg("EX_GetMacAddr\n");
	int nMac[6];
	if(macAddr != NULL)
	{
		char buf[128];
		GetBoardInfo(BOARD_MAC, buf, 128);

		DBG_InfoMsg("buf = %s\n",buf);

		sscanf(buf,"%x:%x:%x:%x:%x:%x",&nMac[0],&nMac[1],&nMac[2],&nMac[3],&nMac[4],&nMac[5]);

		sprintf(macAddr,"%02x-%02x-%02x-%02x-%02x-%02x",nMac[0],nMac[1],nMac[2],nMac[3],nMac[4],nMac[5]);
	}

	netid = 0;
	return 0;
}
int EX_SetDNSName(int id,char*name)
{
	DBG_InfoMsg("EX_SetDNSName name =%s\n",name);
	if(id == 1)
	{
#ifdef CONFIG_P3K_HOST
		{	if(strlen(name)>0)
			{
				char sCmd[64] = "";
				sprintf(sCmd,"e_p3k_net_dante_name::%s",name);
				DBG_InfoMsg("ast_send_event %s\n",sCmd);
				ast_send_event(0xFFFFFFFF,sCmd);
			}
		}
#else
		DBG_WarnMsg(" !!! This is Decoder\n");
		return -1;
#endif
	}
	else if(id == 0)
	{
		if(strlen(name)>0)
		{
			char sCmd[64] = "";
			sprintf(sCmd,"e_p3k_net_hostname::%s",name);
			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);
		}
	}
	else
	{
		return -1;
	}

	Cfg_Set_Dev_HostName(id,name);

	return 0;
}
int EX_GetDNSName(int id,char*name)
{
	if(id == 0)
	{
		GetBoardInfo(BOARD_HOSTNAME, name, MAX_DEV_NAME_LEN);
	}
	else if(id == 1)
	{
		Cfg_Get_Dev_HostName(1,name);
	}
	else
	{
		return -1;
	}
	//strcpy(name,"room-1");
	return 0;
}
int EX_ResetDNSName(char *name)
{
	strcpy(name,"kramer_0102");
	return 0;
}
int EX_SetDHCPMode(int netid,int mode)
{
	DBG_InfoMsg("EX_SetDHCPMode ethid= %d mode =%d\n",netid,mode);
	if((netid == 0)||(netid == 1))
	{
		if((mode == 0)||(mode == 1))
		{
			char sCmd[32] = "";
			if(mode == 0)
				sprintf(sCmd,"e_p3k_net_dhcp::%d::static",netid);
			else
				sprintf(sCmd,"e_p3k_net_dhcp::%d::dhcp",netid);

			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);

			Cfg_Set_Net_DHCP(netid, mode);
		}
	}

	return 0;
}
int EX_GetDHCPMode(int netid,int* mode)
{
	*mode =1;
	if((netid == 0)||(netid == 1))
	{
		*mode = g_network_info.eth_info[netid].dhcp_enable;
	}
	return 0;
}
int EX_SetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{
	DBG_InfoMsg("EX_SetNetWorkConf ip %s\n",netInfo->ipAddr);

	if((netId == 0)||(netId == 1))
	{
		if(g_network_info.eth_info[netId].dhcp_enable == 0)
		{
			char sCmd[128] = "";
			sprintf(sCmd,"e_p3k_net_conf::%d::%s::%s::%s",netId,netInfo->ipAddr,netInfo->mask,netInfo->gateway);

			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);

			Cfg_Set_Net_Config(netId,netInfo);
		}
	}
	return 0;
}
int EX_GetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{
	char ip_buf[32] = "";
	char mask_buf[32] = "";
	char gw_buf[32] = "";

	char* ip_cmd = "ifconfig |grep inet| sed -n '1p'|awk '{print $2}'|awk -F ':' '{print $2}'";
	char* mask_cmd = "ifconfig |grep inet| sed -n '1p'|awk '{print $4}'|awk -F ':' '{print $2}'";
	char* gw_cmd = "route -n | grep eth0 | grep UG | awk '{print $2}'";

	mysystem(ip_cmd, ip_buf, 32);
	mysystem(mask_cmd, mask_buf, 32);
	mysystem(gw_cmd, gw_buf, 32);

	strcpy(netInfo->ipAddr,ip_buf);
	strcpy(netInfo->mask,mask_buf);

	if(strlen(gw_buf)<=0)
		strcpy(netInfo->gateway,"0.0.0.0");
	else
		strcpy(netInfo->gateway,gw_buf);

	strcpy(netInfo->dns1,"0.0.0.0");
	strcpy(netInfo->dns2,"0.0.0.0");
	return 0;
}
int EX_SetNetPort(char* portType,int portNumber)
{
	DBG_InfoMsg("EX_SetNetPort portType= %s portNumber =%d\n",portType,portNumber);
	if((!strcmp(portType,"udp"))||(!strcmp(portType,"UDP")))
		Cfg_Set_Net_Port(Net_UDP,portNumber);
	else if((!strcmp(portType,"tcp"))||(!strcmp(portType,"TCP")))
		Cfg_Set_Net_Port(Net_TCP,portNumber);
	else
		printf("EX_SetNetPort portType: %s Error\n",portType);

	return 0;
}
int EX_GetNetPort(char* portType,int *portNumber)
{
	if((!strcmp(portType,"udp"))||(!strcmp(portType,"UDP")))
		Cfg_Get_Net_Port(Net_UDP,portNumber);
	else if((!strcmp(portType,"tcp"))||(!strcmp(portType,"TCP")))
		Cfg_Get_Net_Port(Net_TCP,portNumber);
	else
		DBG_WarnMsg("EX_GetNetPort portType: %s Error\n",portType);

	return 0;
}
int EX_SetSecurityStatus(int status)
{
	DBG_InfoMsg("EX_SetSecurityStatus %d\n",status);
	if((status == 0)||(status == 1))
	{
		Cfg_Set_User_Secur((State_E)status);
	}
	return 0;
}
int EX_Login(char*name,char*password)
{
	DBG_InfoMsg("EX_Login name= %s password =%s\n",name,password);
	if((!memcmp(name,"admin",strlen("admin")))
		&&((! strcmp(password,g_user_info.password))/*||(! strcmp(password,"33333"))*/))
	{
		return 0;
	}
	return -1;
}
int EX_GetLoginInfo(char*name,char*password)
{
	strcpy(name,"admin");
	return 0;
}
int EX_Logout(void)
{
	return 0;
}
int EX_GetDevVersion(char*version)
{
	//char tmp[32];
	if(version != NULL)
	{
		GetBoardInfo(BOARD_FW_VERSION, version, 32);
	}

	//strcpy(version,tmp);
	return 0;
}
int EX_Upgrade(void)
{

//	Cfg_Set_UPG_Info();
	strcpy(g_version_info.standby_version,g_version_info.fw_version);

	time_t secTime;
	struct tm *ptime =NULL;
	secTime = time(NULL);
	ptime = localtime(&secTime);

	sprintf(g_version_info.upg_time,"%02d-%02d-%04d,%02d:%02d:%02d",ptime->tm_mon+1,ptime->tm_mday,ptime->tm_year+1900,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);

	Cfg_Update_Version();

	printf("EX_Upgrade \n");

	ast_send_event(0xFFFFFFFF,"e_p3k_download_fw_start");

	ast_send_event(0xFFFFFFFF,"e_stop_link");

	sleep(2);

	ast_send_event(0xFFFFFFFF,"e_p3k_upgrade_fw");

	return 0;
}
int EX_SetDeviceNameModel(char*mod)
{
	DBG_InfoMsg("EX_SetDeviceNameModel %s\n",mod);
	return 0;
}
int EX_GetDeviceNameModel(char*mod)
{
	if(mod != NULL)
	{
		GetBoardInfo(BOARD_MODEL, mod, MAX_DEV_MOD_NAME_LEN);
	}

	return 0;
}
int EX_SetSerialNumber(char*data)
{
	DBG_InfoMsg("EX_SetSerialNumber %s\n",data);
	return 0;
}
int EX_GetSerialNumber(char*data)
{
	//strcpy(data,"12345678987654");
	if(data != NULL)
	{
		GetBoardInfo(BOARD_SN, data, SERIAL_NUMBER_LEN);
	}
	return 0;
}

int EX_SetLockFP(int lockFlag)
{
	DBG_InfoMsg("EX_SetLockFP %d\n",lockFlag);
	if((lockFlag == 0)||(lockFlag == 1))
	{
		Cfg_Set_Dev_FPLock((State_E)lockFlag);
		if(lockFlag == 0)
			ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_off");
		else
			ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_on");

	}
	return 0;
}
int EX_GetLockFP(int *lockFlag)
{
	State_E tmp_flag;

	Cfg_Get_Dev_FPLock(&tmp_flag);

	*lockFlag = (int)tmp_flag;

	return 0;
}
int EX_SetIDV(void)
{
	ast_send_event(0xFFFFFFFF,"e_p3k_flag_me");
	return 0;
}
int EX_SetStandbyMode(int Mode)
{
	DBG_InfoMsg("EX_SetStandbyMode %d\n",Mode);
	return 0;
}

int EX_GetStandbyMode(int * value)
{
	int mode = 1;
	value = &mode;
	return 0;
}

int EX_SetBeacon(int portNumber,int status,int rateSecond)
{
	DBG_InfoMsg("EX_SetBeacon portNumber= %d status= %d rateSecond=%d\n",portNumber,status,rateSecond);
	return 0;
}
int EX_GetBeaconInfo(int portNumber,BeaconInfo_S*info)
{
	info->portNumber = portNumber;
	info->tcpPort = 5000;
	info->udpPort = 50000;
	strcpy(info->deviceMod,"dip-20");
	strcpy(info->deviceName,"room-1");
	strcpy(info->ipAddr,"192.168.0.10");
	strcpy(info->macAddr,"11-22-33-44-55-66");


	return 0;
}
int EX_DeviceReset(void)
{
	DBG_InfoMsg("EX_DeviceReset\n");
	system("reboot");
	return 0;
}
int EX_FactoryRecovery(void)
{
	DBG_InfoMsg("EX_FactoryRecovery\n");

	ast_send_event(0xFFFFFFFF,"e_button_link_5");
	return 0;
}
int EX_GetDevBuildDate(char *date,char*hms)
{
	if((date != NULL)&&(hms!= NULL))
	{
		char buf[128];
		GetBoardInfo(BOARD_BUILD_DATE, buf, 128);

		sscanf(buf,"%s,%s",date,hms);
	}
	return 0;
}
int EX_SetTimeAndDate(char*weekDay,char*date,char*hms)
{
	DBG_InfoMsg("week:%s date:%s hms:%s\n",weekDay,date,hms);

	int year=0,mon=0,day=0,hh=0,mm=0,ss=0;
	struct tm set_tm;
	struct timeval set_tv;
	time_t timep;

	sscanf(date,"%d-%d-%d",&mon,&day,&year);
	sscanf(hms,"%d:%d:%d",&hh,&mm,&ss);

	set_tm.tm_year 	= year - 1900;
	set_tm.tm_mon 	= mon -1;
	set_tm.tm_mday 	= day;
	set_tm.tm_hour	= hh;
	set_tm.tm_min	= mm;
	set_tm.tm_sec	= ss;

	timep = mktime(&set_tm);

	set_tv.tv_sec = timep;
	set_tv.tv_usec = 0;

	if(settimeofday(&set_tv,(struct timezone*)0) < 0)
	{
		printf("set time error");
	}


	return 0;
}
int EX_GetTimeAndDate(char*weekDay,char*date,char*hms)
{
	time_t secTime;
	struct tm *ptime =NULL;
	char *str[] = {"sun","mon","tue","wed","thu","fri","sat"};
	secTime = time(NULL);
	ptime = localtime(&secTime);

	sprintf(weekDay,"%s",str[ptime->tm_wday]);
	sprintf(date,"%02d-%02d-%04d",ptime->tm_mon+1,ptime->tm_mday,ptime->tm_year+1900);
	sprintf(hms,"%02d:%02d:%02d",ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
	return 0;
}

int EX_GetLogResetEvent(int * iLog,char*date,char*hms)
{
	time_t secTime;
	struct tm *ptime =NULL;
	//char *str[] = {"sun","mon","tue","wed","thu","fri","sat"};
	secTime = time(NULL);
	ptime = localtime(&secTime);
	int a = 1;
	iLog = &a;
	//auto -1 ,mamual  -2
	sprintf(date,"%02d/%02d/%04d",ptime->tm_mday,ptime->tm_mon+1,ptime->tm_year+1900);
	sprintf(hms,"%02d:%02d:%02d",ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
	return 0;
}

int EX_SetTimeZero(int tz,int timingMethod)
{
	DBG_InfoMsg("EX_SetTimeZero tz = %d,timingMethod = %d\n",tz,timingMethod);
	Cfg_Set_Time_Loc(tz,timingMethod);

	return 0;
}
int EX_GetTimeZero(int *tz,int *timingMethod)
{
	Cfg_Get_Time_Loc(tz,timingMethod);

	return 0;
}
int EX_SetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	DBG_InfoMsg("EX_SetTimeSyncInfo server =%s\n",syncInfo->serverIp);
	Cfg_Set_Time_Srv(*syncInfo);

	if(syncInfo->enable == 0)
	{
		ast_send_event(0xFFFFFFFF,"e_p3k_ntp_enable_off");
	}
	else
	{
		char sCmd[128] = "";
		sprintf(sCmd,"e_p3k_ntp_enable_on::%s::%d",syncInfo->serverIp,syncInfo->syncInerval);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	return 0;
}
int EX_GetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	int status = 0;
	//syncInfo->enable = 0;
	//syncInfo->syncInerval = 10;
	//strcpy(syncInfo->serverIp,"192.168.0.168");

	Cfg_Get_Time_Srv(syncInfo);
	return status;
}
int EX_GetSignalList(char info[][MAX_SIGNALE_LEN],int num)
{
	int tmpnum = 2;
	char *str ="in.hdmi.1.audio.1";
	char *str1= "out.hdmi.1.video.1";
	memcpy(info[0],str,strlen(str));
	memcpy(info[1],str1,strlen(str1));
	return tmpnum;
}

int EX_GetConnectionList(char info[][MAX_SIGNALE_LEN],int num)
{
	int tmpnum = 4;
	char *str ="[(TCP:80,0.0.0.0:0),LISTEN]";
	char *str1= "[(TCP:5000,0.0.0.0:0),LISTEN]";
	char *str2= "[(TCP:80,192.168.114.3:52400),ESTABLISHED]";
	char *str3= "[(TCP:5000,192.168.1.100:51647),ESTABLISHED]";
	memcpy(info[0],str,strlen(str));
	memcpy(info[1],str1,strlen(str1));
	memcpy(info[2],str2,strlen(str2));
	memcpy(info[3],str3,strlen(str3));
	return tmpnum;
}

int EX_GetPortList(char info[][MAX_PORT_LEN],int num)
{
	int tmpnum = 3;
	strcpy(info[0],"in.hdmi.1");
	strcpy(info[1],"out.hdmi.1");
	strcpy(info[2],"out.hdmi.2");
	return tmpnum;
}
int EX_GetActiveCliNUm(void)
{
	int num = 6;
	return num ;
}
int EX_SetLogEvent(int action,int period)
{
	DBG_InfoMsg("EX_SetLogEvent action = %d period= %d\n",action,period);
	Cfg_Set_Log_Action(action,period);
	return 0;
}

int EX_GetLogEvent(int * action,int * period)
{
	DBG_InfoMsg("EX_SetLogEvent action = %d period= %d\n",action,period);
	Cfg_Get_Log_Action(action,period);

	return 0;
}

int EX_GetLogTail(int lineNumber,char log[][MAX_ONELOG_LEN])
{
	int i = 0;
	for(i=0 ;i <lineNumber;i++)
	{
		strcpy(log[i],"logtest");
	}
	return i;
}

int EX_AutomaticReporting(char * info)
{
	//char * str = "~01@KDS-DANTE-NAME  KDS-LONG\r\n";
	//char * str = "";
	memcpy(info,sUpInfo,strlen(sUpInfo));
	int iLength = 0;
	iLength = strlen(sUpInfo);
	memset(sUpInfo,0,sizeof(sUpInfo));
	return iLength;
}

int Clear_Re(void)
{
	memset(sUpInfo,0,sizeof(sUpInfo));
	return 0;
}

int EX_SetCfgModify(char* cfgName)
{
	DBG_InfoMsg("EX_SetCfgModify : %s\n",cfgName);

	if(strstr(cfgName,"av_signal") != 0)
	{
		Cfg_Set_Enc_AVSignal_Info();
	}
	else if(strstr(cfgName,"km_usb") != 0)
	{
		Cfg_Set_Dec_Usb_KVM();
	}

	return 0;
}

