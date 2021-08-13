#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

using namespace std;

#define BUFSIZE 128
#define PORT 5588

static char  sUpInfo[256] = {0};

int GetBoardInfo(BoardInfoType_E type, char* info, int size)
{
	FILE *fp;

	memset(info,0,size);

	printf("GetBoardInfo type:%d,size:%d start\n",type,size);

	if(type == BOARD_HOSTNAME)
	{
		fp = fopen(KDS_HOSTNAME_FILE, "r");
		if (fp == NULL) {
			printf("ERROR! can't open hostname\n");
			return -1;
		}
		fread(info,1,size,fp);
	}
	else if(type == BOARD_BUILD_DATE)
	{
		fp = fopen(KDS_VSRSION_FILE, "r");
		if (fp == NULL) {
			printf("ERROR! can't open hostname\n");
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
			printf("ERROR! can't open date\n");
		}
	}
	else
	{
		Json::Reader reader;
		Json::Value root;
		char pBuf[1024] = "";

		fp = fopen(KDS_BOARD_INFO_FILE, "r");
		if (fp == NULL) {
			printf("ERROR! can't open boardinfo\n");
			return -1;
		}

		int len = fread(pBuf,1,sizeof(pBuf),fp);

		if(reader.parse(pBuf, root))
		{
			if(type == BOARD_MODEL)
			{
				if(!root["model"].empty())
				{
					string buf = root["model"].asString();
					if(buf.length() > size)
					{
						printf("ERROR! model is too long\n");
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
						printf("ERROR! serial number is too long\n");
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
						printf("ERROR! smac address is too long\n");
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
						printf("ERROR! board version is too long\n");
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
						printf("ERROR! firmware version is too long\n");
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

	printf("GetBoardInfo type:%d,info:%s end\n",type,info);
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
	char sendbuf[BUFSIZE] = {0};
	memcpy(sendbuf,buf,strlen(buf)+1);
	// ³õÊ¼»¯
	int fd = socket(AF_INET,SOCK_DGRAM,0);
	printf("fd: %d\n",fd);
	if(fd < 0)
	{
		printf("socket failed \n");
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
		printf("ret: %d\n",ret);
		printf("send finished \n");

		char recvbuf[64] = {0};
		struct sockaddr_in peer;
		socklen_t  peerlen = sizeof(peer);
		int recv = recvfrom(fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&peer, &peerlen);
		printf("recv: %d\n",recv);
		printf("recvbuf: %s\n",recvbuf);

		mode = atoi(recvbuf);
		printf("mode: %d\n",mode);
	}
	else
	{
		int ret = sendto(fd,sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&sockaddr_dest,len);
		printf("ret: %d\n",ret);
		printf("send finished \n");
	}

	//close(fd);
	return mode;
}

//"#KDS-AUD" 0:hdmi;1:analog;4:dante
int  EX_SetAudSrcMode(int mode)
{
//	int a =5;
//	int b = 7;
//	printf("...........%d\n",classTest(a,b));
	printf("EX_SetAudSrcMode mode =%d\n",mode);

#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(mode == 0)
		sprintf(sCmd,"e_p3k_audio_src::hdmi");
	else if(mode == 1)
		sprintf(sCmd,"e_p3k_audio_src::analog");
	else if(mode == 4)
		sprintf(sCmd,"e_p3k_audio_src::dante");
	else
	{
		printf(" !!! Error mode:%d \n",mode);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}
int  EX_GetAudSrcMode(int *mode)
{
	char * str = "~01@KDS-DANTE-NAME  KDS-LONG\r\n";
	memcpy(sUpInfo,str,strlen(str));
	int tmpmode = 0;
	*mode = tmpmode;
	return 0;
}

int EX_SetAudGainLevel(PortInfo_S*info,int gain)
{
	printf("gain =%d\n",gain);
	char sCmd[64] = "";

	if(info->signal == SIGNAL_AUDIO)
	{
		sprintf(sCmd,"e_p3k_audio_level::%d",gain);
	}
	else
	{
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	return 0;
}

int EX_SetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	printf("EX_SetAudAnalogGainDir, gain =%s\n",gain);
	char sCmd[64] = "";

	if(info->portFormat == PORT_IR)
	{
		if((!strcmp(gain,"IN"))||(!strcmp(gain,"in")))
		{
			sprintf(sCmd,"e_p3k_ir_dir::in");
		}
		else if((!strcmp(gain,"OUT"))||(!strcmp(gain,"out")))
		{
			sprintf(sCmd,"e_p3k_ir_dir::out");
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
		}
		else if((!strcmp(gain,"OUT"))||(!strcmp(gain,"out")))
		{
			sprintf(sCmd,"e_p3k_audio_dir::out");
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
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	return 0;
}


int EX_GetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	char tmpGain[10] = {"IN"};
	memcpy(gain,tmpGain,strlen(tmpGain));
	return 0;
}



int EX_SetEDIDMode(EdidInfo_S *info)
{
	//printf("gain =%s\n",gain);
	printf(">>EX_SetEDIDMode %d,%d\n",info->input_id,info->index);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(info->mode == PASSTHRU)
		sprintf(sCmd,"e_p3k_video_edid_mode::passthru");
	else if(info->mode == DEFAULT)
		sprintf(sCmd,"e_p3k_video_edid_mode::default");
	else if(info->mode == CUSTOM)
	{
		sprintf(sCmd,"e_p3k_video_edid_mode::custom::%d",info->index);
	}
	else
	{
		printf(" !!! Error para mode:%d\n",info->mode);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_GetEDIDMode(int cmdID,EdidInfo_S * info)
{
 	EdidInfo_S tmpconf;
	info->input_id = cmdID;
	info->mode = CUSTOM;
	info->index = 1;
	//printf(">>EX_GetEDIDMode\n");
	return 0;
}

int EX_AddEDID(EdidName_S * info)
{
	printf(">>EX_AddEDID %d %s\n",info->index,info->name);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_add::%d::%s",info->index,info->name);
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_RemoveEDID(int comID)
{
	printf(">>EX_RemoveEDID %d \n",comID);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_remove::%d",comID);
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_SetActiveEDID(int input_ID,int index_ID )
{
	printf(">>EX_SetActiveEDID %d,%d \n",input_ID,index_ID);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_active::%d",index_ID);
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_SetEDIDNetSrc(int input_ID,char*macAddr )
{
	printf(">>EX_SetEDIDNetSrc id=%d mac=%s \n",input_ID,macAddr);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_src::%s",macAddr);
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetEDIDNetSrc(int input_ID,char*macAddr )
{
	char * version = "00-14-22-01-23-45";
	memcpy(macAddr,version,strlen(version));
	return 0;
}

int EX_GetRecvMsgNum(int msg,char*date )
{
	int iMsgNum = 100;

	return iMsgNum;
}

int EX_GetVidOutRatio(char*date )
{
	char value[32] = "16:9";
	memcpy(date,value,strlen(value));

	return 0;
}

int EX_SetChannelName(char date[32] )
{

	printf("AV channel Name %s\n",date);

	return 0;
}

int EX_SetDanteName(char date[32])
{
	printf("Set Encoder Dante Hostl Name. %s\n",date);

	return 0;
}

int EX_SetVidMute(MuteInfo_S * mute )
{

	printf(">>EX_SetVidMute %d,%d,%d,%d,%d,%d\n",mute->direction,mute->portFormat,mute->portIndex,
		mute->signal,mute->index,mute->state);
	//0	¨C off for unmute  1	¨C on for mute
	char sCmd[64] = "";
	if(mute->signal == SIGNAL_AUDIO)
		sprintf(sCmd,"e_p3k_audio_mute::%d",mute->state);
	else
	{
		printf(" !!!parameter Error signal=%d\n",mute->signal);
		return 0;
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	return 0;
}


int EX_SetTimeOut(int  iTime )
{

	printf(">>EX_SetOsdDisplay %d\n",iTime);
	//time ¨C minutes of logout time
	return 0;
}
int EX_GetTimeOut(void)
{

	int iTime =10;

	return iTime;
}

int EX_SetVideoWallStretch(int  index,int mode )
{
	printf(">>EX_SetVideoWallStretch %d\n",index);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((mode == 0)||(mode == 1))
		sprintf(sCmd,"e_p3k_video_vw_stretch::%d",mode);
	else
	{
		printf(" !!! Error para mode:%d\n",mode);
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoWallStretch(int  index)
{

	int mode =1;

	return mode;
}


int EX_SetStandbyTimeOut(int  iTime )
{

	printf(">>EX_SetStandbyTimeOut %d\n",iTime);
	//time ¨C minutes of logout time
	return 0;
}
int EX_GetStandbyTimeOut(void)
{

	int iTime =10;

	return iTime;
}

int EX_SetRollback(char * type)
{
	char * aOk = "ok";
	memcpy(type,aOk,strlen(aOk));
	return 0;
}


int EX_SetIRGateway(int  iIr_mode)
{
	printf("Tr gw mode = %d\n",iIr_mode);
	char sCmd[64] = "";
	if((iIr_mode == 0)||(iIr_mode == 1))
	{
		sprintf(sCmd,"e_p3k_ir_gw::%d",iIr_mode);
	}
	else
	{
		printf(" !!! Error para iIr_mode:%d\n",iIr_mode);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	return 0;
}

int EX_GetIRGateway(void)
{
	int iIr_mode = 1;//0-off 1-on
	printf("Tr gw mode = %d\n",iIr_mode);
	return iIr_mode;
}

int EX_SetMulticastStatus(char * ip,int ttl )
{
	printf("ip=%s\n",ip);
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
	printf("name=%s\n",login_level);

	char * admin = "admin";
	if(0 == memcmp(login_level,admin,strlen(admin)))
	{

		return 0;
	}
	printf(" oldpassword error\n");
		return -1;
}


int EX_GetPassword(char * login_level)
{
	char * admin = "admin";
	int pass=33333;
	if(0 == memcmp(login_level,admin,strlen(admin)))
	{
		printf("name=%s\n",login_level);
		//iNew_Pass = &pass;
		return pass;
	}
	//printf(" oldpassword error\n");
	return -1;
}

int EX_SetGatewayPort(int iGw_Type,int iNetw_Id)
{
	printf("iGw_Type=%d\n",iGw_Type);
	return 0;
}

int EX_GetVlanTag(int iGw_Type)
{
	int iTag = 11;
	printf("iGw_Type=%d\n",iGw_Type);
	return iTag;
}

int EX_SetVlanTag(int iGw_Type,int iTag)
{
	printf("iGw_Type=%d\n",iGw_Type);
	return 0;
}

int EX_GetGatewayPort(int iGw_Type)
{
	int iNetw_Id = 0;
	printf("iGw_Type=%d\n",iGw_Type);
	return iNetw_Id;
}

int EX_SetMethod(int  mode )
{

	printf(">>EX_SetOsdDisplay %d\n",mode);
	//osd mode {0	¨C off,1	¨C on,2	¨C display now,}
	return 0;
}
int EX_GetMethod(void)
{

	int mode =1;

	return mode;
}

int EX_SetOsdDisplay(int  mode )
{
	char setOSDDisplayCmd[BUFSIZE] = {0};
	char * str = "SET_OSD_DISPLAY";
	sprintf(setOSDDisplayCmd,"%s %d",str,mode);
	int ret = sendCmdtoGUI(setOSDDisplayCmd);
	//printf(">>EX_SetOsdDisplay %d\n",mode);
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

	printf(">>EX_SetDaisyChain %d\n",mode);
	//0	¨C OFF (disables dainsy chain)
	//11	¨C ON (enables dainsy chain)
	return 0;
}
int EX_GetDaisyChain(void)
{

	int daisy_state =1;

	return daisy_state;
}

int EX_GetVidMute(MuteInfo_S * mute)
{
	//int mute_mode = 0;
	//return mute_mode;
	mute->state = ON;
	return 0;
}

int EX_SetVidOutput(char info[][MAX_PARAM_LEN],int count )
{
	int num = 0;
	for(num = 0;num < count;num ++)
		{
			printf("%s\n",info[num]);

		}
	return 0;
}


int EX_GetVidOutput(char * date)
{
//Audio output {0	¨C HDMI  1	¨C Analog  2	¨C Stream,3	¨C Dante}
	char * str = "1,2,3";
	memcpy(date,str,strlen(str));
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
	char * version = "1.12.123";
	memcpy(date,version,strlen(version));
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
	int iTemp;
	if(iMode == 0){
	iTemp = 50;
		}
	return iTemp;
}

int EX_GetUPGTime(char * day,char * time)
{
	char *str = "05-12-2018";
	char *str1 = "14:30:00";
	memcpy(day,str,strlen(str));
	memcpy(time,str1,strlen(str1));
	return 0;
}

int EX_GetChannelName(char * date)
{

	char name[32] = "bbc";
	memcpy(date,name,strlen(name));
	return 0;
}

int EX_GetDanteName(char * date)
{

	char name[32] = "KDS-6x-MAC";
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
	printf(">>EX_GetActiveEDID %d\n",input_ID);
	return index;
}

int EX_GetEdidList(char info[][MAX_EDID_LEN],int num)
{
 	int tmpnum = 3;
	char *str ="[0,DEFAULT]";
	char *str1= "[2,SONY]";
	char *str2= "[5,PANASONIC]";
	memcpy(info[0],str,strlen(str));
	memcpy(info[1],str1,strlen(str1));
	memcpy(info[2],str1,strlen(str2));
	return tmpnum;
}


int EX_GetAudGainLevel(PortInfo_S*info,int *gain)
{
	int tmpGain = 20;
	 *gain = tmpGain;
	return  0;
}
int EX_GetAudParam(PortInfo_S*info,AudioSignalInfo_S*param)
{
	param->chn = 2;
	param->format = AUDIO_FORMAT_PCM;
	param->sampleRate= SAMPLE_RATE_44100;
	return 0;
}
int EX_SetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E mode)
{
	printf("EX_SetAutoSwitchMode mode =%d\n",mode);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(info->signal == SIGNAL_VIDEO)
		sprintf(sCmd,"e_p3k_switch_mode");
	else if(info->signal == SIGNAL_AUDIO)
		sprintf(sCmd,"e_p3k_audio_mode");
	else
	{
		printf(" !!!parameter Error signal=%d\n",info->signal);
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
		printf(" !!!parameter Error mode = %d\n",mode);
		return 0;
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E *mode)
{
	pid_t status;
	char * cmd2 = "./sconfig --show mode";
	status = system(cmd2);
	*mode = CONNECT_MANUAL;
	return 0;
}

//#X-PRIORITY <direction_type>. <port_format>. <port_index> .<signal_type> , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR>
int EX_SetAutoSwitchPriority(AudioInfo_S * info,AudioInfo_S * gain,int count)
{
	printf("EX_SetAutoSwitchPriority\n");
	if(count == 0)
	{
		printf(" !!! Error Count = %d\n",count);
		return 0;
	}

#ifdef CONFIG_P3K_HOST
	char sCmd[128] = "";
	if(gain[0].signal == SIGNAL_VIDEO)
	{
		sprintf(sCmd,"e_p3k_switch_pri");

		for(int i = 1;i < count;i++)
		{
			sprintf(sCmd,"%s::HDMI%d",sCmd,gain[i].portIndex);
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
			}
			else if(gain[i].portFormat == PORT_ANALOG_AUDIO)
			{
				sprintf(sCmd,"%s::analog",sCmd);
			}
			else if(gain[i].portFormat == PORT_DANTE)
			{
				sprintf(sCmd,"%s::dante",sCmd);
			}
		}
	}
	else
	{
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif

	return 0;
}

int EX_GetAutoSwitchPriority(AudioInfo_S * gain,int count)
{
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
	return num;
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
	printf("EX_SetEDIDColorSpaceMode mode =%d\n",mode);
	return 0;
}
int EX_GetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E *mode)
{

	 *mode = COLOR_RGB;
	return 0;
}

int EX_SetEDIDLockStatus(int index,int lock)
{
	printf("EX_SetEDIDLockStatus index =%d lock=%d\n",index,lock);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if((lock == 0)||(lock == 1))
		sprintf(sCmd,"e_p3k_video_edid_lock::%d",lock);
	else
	{
		printf(" !!! Error para lock:%d\n",lock);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
return 0;
}
int EX_GetEDIDLockStatus(int index,int *lock)
{
	*lock = 0;
	return 0;
}
int EX_SetHDCPMode(int index,HDCPMode_E mode)
{
	printf("EX_SetHDCPMode index=%d mode=%d\n",index,mode);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";

	if((mode == HDCP_OFF)||(mode == HDCP_ON)||(mode == HDCP_MIRROR))
	{
		sprintf(sCmd,"e_p3k_video_hdcp_mode::%d::%d",index,mode);
	}
	else
	{
		printf(" !!! Error para mode:%d\n",mode);
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetHDCPMode(int index,HDCPMode_E *mode)
{
	*mode = HDCP_ON;
	return 0;
}
int EX_GetHDCPStatus(int io,int index)
{
	int status = 0;


	return status;
}

int EX_SetViewMode(ViewMode_E mode,ViewModeInfo_S*info)
{
	printf("EX_SetViewMode h = %d v = %d\n",info->hStyle,info->vStyle);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((mode == VIEW_MODE_VIDEOWALL)
		&&(info->hStyle >= 1)
		&&(info->hStyle <= 16)
		&&(info->vStyle >= 1)
		&&(info->vStyle <= 16))
	{
		sprintf(sCmd,"e_p3k_video_vw_mode::%d::%d",info->hStyle,info->vStyle);
	}
	else
	{
		printf(" !!! Error para mode:%d,h = %d,v = %d\n",mode,info->hStyle,info->vStyle);
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetViewMode(ViewMode_E *mode,ViewModeInfo_S*info)
{
      *mode = VIEW_MODE_VIDEOWALL;
	 info->hStyle = 10;
	 info->vStyle = 5;
	return 0;
}

int EX_SetWndBezelInfo( int mode ,int index, WndBezelinfo_S*info)
{
	printf("EX_SetWndBezelInfo =%d %d %d %d\n",info->hValue,info->vValue,info->hOffset,info->vOffset);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((index >= 1)&&(index <= 256))
		sprintf(sCmd,"e_p3k_video_vw_bezel::%d::%d::%d::%d::%d",index,info->hValue,info->vValue,info->hOffset,info->vOffset);
	else
	{
		printf(" !!! Error para index:%d\n",index);
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetWndBezelInfo( int *mode ,int *index, WndBezelinfo_S*info)
{
	*mode = 0;
	*index = 1;
	info->hValue =320;
	info->vValue =160;
	info->hOffset = 10;
	info->vOffset = 10;
	return 0;
}
int EX_SetVideoWallSetupInfo(int id,VideoWallSetupInfo_S *info)
{
	printf("EX_SetVideoWallSetupInfo id =%d,rotaion=%d\n",id,info->rotation);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";

	if((id >= 1) &&(id <= 256) &&(info->rotation <= 3))
		sprintf(sCmd,"e_p3k_video_vw_id::%d::%d",id,info->rotation);
	else
	{
		printf(" !!! Error para id:%d,info->rotation:%d\n",id,info->rotation);
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoWallSetupInfo(int* id,VideoWallSetupInfo_S *info)
{
	*id = 10;
	info->rotation = ROTATION_0;
	return 0;
}
int EX_StartOverlay(char*confFile,int outtime)
{
	char startOverlayCmd[BUFSIZE] = {0};
	sprintf(startOverlayCmd,"START_OVERLAY %s %d",confFile,outtime);
	int mode = sendCmdtoGUI(startOverlayCmd);
	printf("filename = %s, outtime =%d mode =%d\n",confFile,outtime,mode);
	return 0;
}
int EX_StopOverlay(void)
{
	const char stopOverlay[BUFSIZE] = {"STOP_OVERLAY"};

	int mode = sendCmdtoGUI(stopOverlay);
	printf(" mode =%d\n",mode);
	return 0;
}

int EX_SetEncoderAVChannelId(int id)
{
	printf("EX_SetEncoderAVChannelId id=%d\n",id);
#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	sprintf(sCmd,"e_reconnect::%04d",id);
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}
int EX_GetEncoderAVChannelId(int *id)
{
	*id = 1;
	return 0;
}

int EX_SetDecoderAVChannelId(ChSelect_S * id)
{
	printf(" EX_SetDecoderAVChannelId id =%d\n",id->ch_id);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	sprintf(sCmd,"e_reconnect::%04d",id->ch_id);

	if(id->signal == SIGNAL_IR)
		sprintf(sCmd,"%s::r",sCmd);

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetDecoderAVChannelId(ChSelect_S * id)
{
	printf(">>%d\n",id->signal);
	id->ch_id = 1;
	return 0;
}
int EX_SetVideoImageStatus(int scalerId,VideoStatusType_E status)
{
	printf(" EX_SetVideoImageStatus status =%d\n",status);
	return 0;
}
int EX_GetVideoImageStatus(int scalerId,VideoStatusType_E *status)
{
	*status = VIDEO_ENABLED;
	return 0;
}

int EX_SetVideoCodecAction(CodecActionType_E type)
{
	printf("EX_SetVideoCodecAction type=%d\n",type);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	if(type == CODEC_ACTION_PLAY)
		sprintf(sCmd,"e_reconnect");
	else if(type == CODEC_ACTION_STOP)
		sprintf(sCmd,"e_stop_link");
	else
	{
		printf(" !!! Error para %d\n",type);
		return 0;
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoCodecAction(CodecActionType_E *type)
{
	*type = CODEC_ACTION_STOP;
	return 0;
}
int EX_SetColorSpaceConvertMode(int index,int convertMode)
{
	printf("EX_SetColorSpaceConvertMode id = %d mode =%d\n",index,convertMode);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	if((convertMode == 0)||(convertMode == 1))
	{
		sprintf(sCmd,"e_p3k_video_rgb::%d",convertMode);
	}
	else
	{
		printf(" !!! Error para %d\n",convertMode);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	printf(" !!! This is Decoder\n");
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetColorSpaceConvertMode(int index,int *convertMode)
{
	*convertMode = 0;
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
			printf(" !!! Error para res:%d\n",res);
			return 0;
		}
	}
	else
	{
		printf(" !!! Error para mode:%d\n",mode);
		return 0;
	}

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Encoder\n");
#endif
	return 0;
}
int EX_GetVideoImageScaleMode(int *mode,char*res)
{
	*mode = 1;
	strcpy(res,"16");
	return 0;
}
int EX_GetVideoViewReslotion(int mode, int index, int nativeFlag,int * res)
{
	int a = 1;
	res = &a;
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
		printf(" !!! Error para info->hexByte:%d\n",info->hexByte);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
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
	}
	else
	{
		printf(" !!! Error para mode:%d\n",mode);
		return 0;
	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	return 0;
}

int EX_GetCECGateWayMode(void)
{
	int mode = 0;
	return mode;
}

int EX_SendIRmessage(IRMessageInfo_S*info)
{
	printf("EX_SendIRmessage cmdname =%s cmdcomment =%s\n",info->cmdName,info->cmdComent);
	char sCmd[128] = "";

	sprintf(sCmd,"e_p3k_ir_send::%s",info->cmdComent);

	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);	return 0;
}
int EX_SendIRStop(int irId,int serialNumb,char*command)
{
	int status = 0;
	return status;
}
int EX_SetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo,int num)
{

	printf("EX_SetRouteMatch  num=====%d  %d   %d\n",num, inPortInfo[2].index,matchPortInfo->index);
#ifdef CONFIG_P3K_HOST
	char sCmd[128] = "";
	if(matchPortInfo->signal == SIGNAL_VIDEO)
	{
		sprintf(sCmd,"e_p3k_switch_in::HDMI%d",matchPortInfo->portIndex);

	}
	else if(matchPortInfo->signal == SIGNAL_AUDIO)
	{
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
			}
			else if(inPortInfo[i].portFormat == PORT_ANALOG_AUDIO)
			{
				sprintf(sCmd,"%s::analog",sCmd);
			}
			else if(inPortInfo[i].portFormat == PORT_DANTE)
			{
				sprintf(sCmd,"%s::dante",sCmd);
			}
			else if(inPortInfo[i].portFormat == PORT_STREAM)
			{
				sprintf(sCmd,"%s::lan",sCmd);
			}
		}

	}
	printf("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	printf(" !!! This is Decoder\n");
#endif
	return 0;
}

int EX_GetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo)
{
	//matchPortInfo->direction = 0;
	//matchPortInfo->index = 1;
	//matchPortInfo->portFormat = 1;
	//matchPortInfo->signal = 1;
	//matchPortInfo->portIndex = 1;
	return 0;
}
int EX_SetUartConf(UartMessageInfo_S*conf)
{
	printf("EX_SetUartConf stopbit =%f\n",conf->stopBitsMode);
	return 0;
}
int EX_GetUartConf(int comId,UartMessageInfo_S*conf)
{
	UartMessageInfo_S tmpconf;

	conf->bitWidth = 8;
	conf->comNumber = comId;
	conf->rate = 115200;
	conf->serialType =0;
	conf->stopBitsMode =1;
	conf->term_485 = 0;

	return 0;
}
int EX_AddComRoute(ComRouteInfo_S*info,int comId)
{
	return 0;
}
int EX_RemoveComRoute(int comId)
{
	return 0;
}
int EX_GetComRoute(int comId,ComRouteInfo_S*info)
{
	info->HeartTimeout = 60;
	info->portNumber = 50001;
	info->portType = 0;
	info->rePlay = 0;
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

     	printf("EX_SetUSBCtrl =%d\n",type);
	return 0;
}
int EX_GetMulticastInfo(char*ip,int *ttl)
{
	strcpy(ip,"255.255.255.239");
	int iTTL = 10;
	ttl = &iTTL;
	return 0;
}

int EX_SetMacAddr(int netid,char*macAddr)
{
	printf("EX_SetMacAddr mac =%s\n",macAddr);
	return 0;
}
int EX_GetMacAddr(int netid,char*macAddr)
{
//	strcpy(macAddr,"00-14-22-01-23-45");
	printf("EX_GetMacAddr\n");
	int nMac[6];
	if(macAddr != NULL)
	{
		printf("macAddr != NULL\n");

		char buf[128];
		GetBoardInfo(BOARD_MAC, buf, 128);

		printf("buf = %s\n",buf);

		sscanf(buf,"%x:%x:%x:%x:%x:%x",&nMac[0],&nMac[1],&nMac[2],&nMac[3],&nMac[4],&nMac[5]);

		sprintf(macAddr,"%02x-%02x-%02x-%02x-%02x-%02x",nMac[0],nMac[1],nMac[2],nMac[3],nMac[4],nMac[5]);
	}

	netid = 0;
	return 0;
}
int EX_SetDNSName(int id,char*name)
{
	printf("EX_SetDNSName name =%s\n",name);
	if(id == 1)
	{
#ifdef CONFIG_P3K_HOST
		{	if(strlen(name)>0)
			{
				char sCmd[64] = "";
				sprintf(sCmd,"e_p3k_audio_dante_name::%s",name);
				printf("ast_send_event %s\n",sCmd);
				ast_send_event(0xFFFFFFFF,sCmd);
			}
		}
#else
		printf(" !!! This is Decoder\n");
#endif
	}

	return 0;
}
int EX_GetDNSName(int id,char*name)
{
	if(id == 0)
	{
		GetBoardInfo(BOARD_HOSTNAME, name, MAX_DEV_NAME_LEN);
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
	printf("EX_SetDHCPMode ethid= %d mode =%d\n",netid,mode);
	return 0;
}
int EX_GetDHCPMode(int netid,int* mode)
{
	*mode =1;
	return 0;
}
int EX_SetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{
	printf("EX_SetNetWorkConf ip %s\n",netInfo->ipAddr);
	return 0;
}
int EX_GetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{

	strcpy(netInfo->ipAddr,"192.168.0.10");
	strcpy(netInfo->mask,"255.255.0.0");
	strcpy(netInfo->gateway,"192.168.0.1");
	strcpy(netInfo->dns1,"114.114.114.114");
	strcpy(netInfo->dns2,"8.8.8.8");
	return 0;
}
int EX_SetNetPort(char* portType,int portNumber)
{
	printf("EX_SetNetPort portType= %s portNumber =%d\n",portType,portNumber);
	return 0;
}
int EX_GetNetPort(char* portType,int *portNumber)
{
	*portNumber = 50000;
	return 0;
}
int EX_SetSecurityStatus(int status)
{
	printf("EX_SetSecurityStatus %d\n",status);
	return 0;
}
int EX_Login(char*name,char*password)
{

	printf("EX_Login name= %s password =%s\n",name,password);
	if(! memcmp(name,"admin",strlen("admin")) && ! memcmp(password,"33333",strlen("33333")))
	{
		//char * cmd = "./mainswitch &";
		//system(cmd);
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
	printf("EX_Upgrade \n");

	ast_send_event(0xFFFFFFFF,"e_stop_link");

	sleep(2);

	ast_send_event(0xFFFFFFFF,"e_p3k_upgrade_fw");

	return 0;
}
int EX_SetDeviceNameModel(char*mod)
{
	printf("EX_SetDeviceNameModel %s\n",mod);
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
	printf("EX_SetSerialNumber %s\n",data);
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
	printf("EX_SetLockFP %d\n",lockFlag);
	return 0;
}
int EX_GetLockFP(int *lockFlag)
{
	*lockFlag = 1;
	return 0;
}
int EX_SetIDV(void)
{
	return 0;
}
int EX_SetStandbyMode(int Mode)
{
	printf("EX_SetStandbyMode %d\n",Mode);
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
	printf("EX_SetBeacon portNumber= %d status= %d rateSecond=%d\n",portNumber,status,rateSecond);
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
	return 0;
}
int EX_FactoryRecovery(void)
{
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
	printf("week:%s date:%s hms:%s\n",weekDay,date,hms);
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
	sprintf(date,"%04d/%02d/%02d",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday);
	sprintf(hms,"%02d:%02d:%02d",ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
	return 0;
}

int EX_GetLogResetEvent(int * iLog,char*date,char*hms)
{
	time_t secTime;
	struct tm *ptime =NULL;
	char *str[] = {"sun","mon","tue","wed","thu","fri","sat"};
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

	printf("EX_SetTimeZero tz = %d,timingMethod = %d\n",tz,timingMethod);
	return 0;
}
int EX_GetTimeZero(int *tz,int *timingMethod)
{
	*tz = 8;
	*timingMethod = 0;
	return 0;
}
int EX_SetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	printf("EX_SetTimeSyncInfo server =%s\n",syncInfo->serverIp);
	return 0;
}
int EX_GetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	int status = 0;
	syncInfo->enable = 0;
	syncInfo->syncInerval = 10;
	strcpy(syncInfo->serverIp,"192.168.0.168");
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
	printf("EX_SetLogEvent action = %d period= %d\n",action,period);
	return 0;
}

int EX_GetLogEvent(int * action,int * period)
{
	//printf("EX_SetLogEvent action = %d period= %d\n",action,period);
	int a =1;
	int b  = 2;
	action = &a;
	period = &b;
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

int EX_AutomaticReporting(char * info)//Active reporting
{
	//char * str = "~01@KDS-DANTE-NAME  KDS-LONG\r\n";
	char * str = "";
	memcpy(info,sUpInfo,strlen(sUpInfo));
	int iLength = 0;
	iLength = strlen(sUpInfo);
	//memset(sUpInfo,0,sizeof(sUpInfo));
	return iLength;
}

int Clear_Re(void)
{
	memset(sUpInfo,0,sizeof(sUpInfo));
	return 0;
}
