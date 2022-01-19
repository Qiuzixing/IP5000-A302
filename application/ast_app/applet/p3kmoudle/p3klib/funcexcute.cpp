#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h>

#include "funcexcute.h"
#include "ast_send_event.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "classsum.h"
#include <unistd.h>

#include "json/json.h"
#include <iostream>
#include <string>
#include <fstream>

#include "cfgparser.h"
#include "debugtool.h"

using namespace std;

#define BUFSIZE 128
#define PORT 6003
static pthread_t Beacon_id;

static char  sUpInfo[256] = {0};

void trim_string_eol(char* buf)
{
	for (int i = strlen(buf); i > 0; i --)
	{
		if (isspace(buf[i - 1]))
		{
			buf[i - 1] = '\0';
		}
		else
		{
			break;
		}
	}
}

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

	//DBG_InfoMsg("mysystem cmdstring:%s,buf:%s\n",cmdstring,buf);
	return EX_NO_ERR;
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
		int size = strlen(info);
		info[size - 1] = 0;

		fclose(fp);
	}
	else if(type == BOARD_BUILD_DATE)
	{
		fp = fopen(KDS_VSRSION_FILE, "r");
		if (fp == NULL) {
			DBG_ErrMsg("ERROR! can't open version\n");
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
		fclose(fp);
	}
	else if(type == BOARD_SN)
	{
		mysystem("astparam r serial_number",info, size);

		if(strstr(info,"not defined") != 0)
		{
			memset(info,0,size);
			strcpy(info,"UNKNOWN");
		}
	}
	else if(type == BOARD_MAC)
	{
		mysystem("astparam r ethaddr",info, size);

		if(strstr(info,"not defined") != 0)
		{
			memset(info,0,size);
			mysystem("astparam g ethaddr",info, size);

			if(strstr(info,"not defined") != 0)
				mysystem("/usr/local/bin/random_mac -c",info, size);
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
			if(type == BOARD_HW_VERSION)
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
		fclose(fp);
	}


	DBG_InfoMsg("GetBoardInfo type:%d,info:%s end\n",type,info);
	return EX_NO_ERR;
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
	//return EX_NO_ERR;

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
/*	if(strncmp(sendbuf,"GET",3) == 0)
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
*/	{
		int ret = sendto(fd,sendbuf,sizeof(sendbuf),0,(struct sockaddr*)&sockaddr_dest,len);
		//DBG_InfoMsg("ret: %d\n",ret);
		//DBG_InfoMsg("send finished \n");
	}

	//close(fd);
	return mode;
}

int IsPortValid(  PortDirectionType_E direction, PortSignalType_E portFormat,int portIndex,	SignalType_E signal,int index)
{
	DBG_InfoMsg("direction = %d, portFormat = %d, portIndex = %d, signal = %d, index = %d\n",direction ,portFormat, portIndex, signal,index);
	//cec,rs232,usb
	if(portFormat == PORT_RS232)
	{
		if((portIndex == 1)&&(signal == SIGNAL_RS232)&&(index == 1))
		{
			return EX_NO_ERR;
		}
	}
	else if(portFormat == PORT_USB_A) // IPD+IPE_P
	{
		if((portIndex == 1)&&(index == 1)&&(signal == SIGNAL_USB)&&
			((strcmp(g_version_info.model,IPD_MODULE) == 0)
			 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)))
		{
			return EX_NO_ERR;
		}
	}
	else if(portFormat == PORT_USB_B) // IPE+IPE_P
	{
		if((portIndex == 1)&&(index == 1)&&(signal == SIGNAL_USB)&&
			((strcmp(g_version_info.model,IPE_MODULE) == 0)
			 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)))
		{
			return EX_NO_ERR;
		}
	}
	//video,audio,ir
	else if((direction == DIRECTION_IN)&&(index == 1))
	{
		if((signal == SIGNAL_VIDEO)||(signal == SIGNAL_CEC))
		{
			if(strcmp(g_version_info.model,IPE_MODULE) == 0)
			{
				if((portFormat == PORT_HDMI)&&(portIndex == 1))
				{
					return EX_NO_ERR;
				}
			}
			else if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(portFormat == PORT_HDMI)
				{
					//for web Main > AV Routing hdmi.3
					if((portIndex == 1)||(portIndex == 2)||(portIndex == 3))
					{
						return EX_NO_ERR;
					}
				}
				else if((portFormat == PORT_USB_C)&&(portIndex == 3))
				{
					return EX_NO_ERR;
				}
			}
			else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
			{
				if((portFormat == PORT_HDMI)&&(portIndex == 1))
				{
					return EX_NO_ERR;
				}
				else if((portFormat == PORT_USB_C)&&(portIndex == 2))
				{
					return EX_NO_ERR;
				}
			}
			else if(strcmp(g_version_info.model,IPD_MODULE) == 0)
			{
				if((portFormat == PORT_HDMI)&&(portIndex == 1))
				{
					return EX_NO_ERR;
				}
				else if((portFormat == PORT_STREAM)&&(portIndex == 1))
				{
					return EX_NO_ERR;
				}
			}
			else//IPD_W_MODULE
			{
				if((portFormat == PORT_STREAM)&&(portIndex == 1))
				{
					return EX_NO_ERR;
				}
			}
		}
		else if(signal == SIGNAL_AUDIO)
		{
			if(portFormat == PORT_HDMI)
			{
				if(strcmp(g_version_info.model,IPE_MODULE) == 0)
				{
					if(portIndex == 1)
						return EX_NO_ERR;
				}
				else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
				{
					if(portIndex == 1)
						return EX_NO_ERR;
				}
				else if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					if((portIndex == 1)&&(portIndex -= 2))
						return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_USB_C)
			{
				if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
				{
					if(portIndex == 2)
						return EX_NO_ERR;
				}
				else if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					if(portIndex == 3)
						return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_ANALOG_AUDIO)
			{
				if((portIndex == 1)&&
					((strcmp(g_version_info.model,IPE_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)))
				{
					//for web AV Settings > Audio IPE_P_MODULE
					//(#X-PRIORITY out.hdmi.1.audio,[in.dante.1.audio,in.hdmi.1.audio,in.analog_audio.1.audio])
					//if(g_audio_info.direction == DIRECTION_IN)
						return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_DANTE)
			{
				if((portIndex == 1)&&(strcmp(g_version_info.model,IPE_P_MODULE) == 0))
				{
					return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_STREAM)
			{
				if((portIndex == 1)&&
					((strcmp(g_version_info.model,IPD_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPD_W_MODULE) == 0)))
				{
					return EX_NO_ERR;
				}
			}
		}
		else if(signal == SIGNAL_IR)
		{
			if((portIndex == 1)&&(portFormat == PORT_RS232))
			{
				if(g_gateway_info.ir_direction == DIRECTION_IN)
				{
					return EX_NO_ERR;
				}
			}
		}
	}
	else if((direction == DIRECTION_OUT)&&(index == 1))
	{
		if((signal == SIGNAL_VIDEO)||(signal == SIGNAL_CEC))
		{
			if(portFormat == PORT_HDMI)
			{
				if((portIndex == 1)&&
				((strcmp(g_version_info.model,IPE_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPD_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPD_W_MODULE) == 0)))
				{
					return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_STREAM)
			{
				if((portIndex == 1)&&
					((strcmp(g_version_info.model,IPE_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_W_MODULE) == 0)))
				{
					return EX_NO_ERR;
				}
			}
		}
		else if(signal == SIGNAL_AUDIO)
		{
			if(portFormat == PORT_HDMI)
			{
				if((portIndex == 1)&&
				((strcmp(g_version_info.model,IPE_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPD_MODULE) == 0)
				 ||(strcmp(g_version_info.model,IPD_W_MODULE) == 0)))
				{
					return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_STREAM)
			{
				if((portIndex == 1)&&
					((strcmp(g_version_info.model,IPE_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
					 ||(strcmp(g_version_info.model,IPE_W_MODULE) == 0)))
				{
					return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_DANTE)
			{
				if((portIndex == 1)&&(strcmp(g_version_info.model,IPE_P_MODULE) == 0))
				{
					return EX_NO_ERR;
				}
			}
			else if(portFormat == PORT_ANALOG_AUDIO)
			{
				if(portIndex == 1)
				{
					if((strcmp(g_version_info.model,IPE_MODULE) == 0)
						 ||(strcmp(g_version_info.model,IPE_P_MODULE) == 0))
					{
						if(g_audio_info.direction == DIRECTION_OUT)
							return EX_NO_ERR;
					}
					else if((strcmp(g_version_info.model,IPD_MODULE) == 0)
						 ||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
					{
						return EX_NO_ERR;
					}
				}
			}
		}
		else if(signal == SIGNAL_IR)
		{
			if((portIndex == 1)&&(portFormat == PORT_RS232))
			{
				if(g_gateway_info.ir_direction == DIRECTION_OUT)
				{
					return EX_NO_ERR;
				}
			}
		}
	}
	else if((direction == DIRECTION_BOTH)&&(index == 1))
	{
		if((portIndex == 1)&&
			((portFormat == PORT_RS232)
			||(portFormat == PORT_ANALOG_AUDIO)
			||(portFormat == PORT_IR)))
		{
			return EX_NO_ERR;
		}
	}

	DBG_WarnMsg(" !!! Error Param \n");

	return EX_PARAM_ERR;
}

bool IsPortActive(  PortDirectionType_E direction, PortSignalType_E portFormat,int portIndex,SignalType_E signal,int index)
{
	if(IsPortValid(direction,portFormat,portIndex,signal,index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return false;
	}

#ifdef CONFIG_P3K_HOST
	if(direction == DIRECTION_IN)
	{
		if(signal == SIGNAL_VIDEO)
		{
			if((strcmp(g_version_info.model,IPE_P_MODULE) == 0)||(strcmp(g_version_info.model,IPE_W_MODULE) == 0))
			{
				char buf[64];
				memset(buf,0,64);
				//Get Cuurent hdmi in
				mysystem("/usr/local/bin/sconfig --show input", buf, 64);
				if(strstr(buf,"HDMI3") != 0)
				{
					if((portFormat == PORT_USB_C)&&(portIndex == 3))
						return true;
				}
				else if(strstr(buf,"HDMI2") != 0)
				{
					if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
					{
						if((portFormat == PORT_HDMI)&&(portIndex == 2))
							return true;
					}
					else
					{
						if((portFormat == PORT_USB_C)&&(portIndex == 2))
							return true;
					}
				}
				else if(strstr(buf,"HDMI1") != 0)//HDMI1
				{
					if((portFormat == PORT_HDMI)&&(portIndex == 1))
						return true;
				}
			}
		}
		else if(signal == SIGNAL_AUDIO)
		{
			char buf[64];
			memset(buf,0,64);

			mysystem("/usr/local/bin/sconfig --show audio-input", buf, 64);
			if(strstr(buf,"hdmi") != 0)
			{
				if(portFormat == PORT_HDMI)
				{
					//get current hdmi
					memset(buf,0,64);
					//Get Cuurent hdmi in
					mysystem("/usr/local/bin/sconfig --show input", buf, 64);
					if(strstr(buf,"HDMI3") != 0)
					{
						if((portFormat == PORT_USB_C)&&(portIndex == 3))
							return true;
					}
					else if(strstr(buf,"HDMI2") != 0)
					{
						if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
						{
							if((portFormat == PORT_HDMI)&&(portIndex == 2))
								return true;
						}
						else
						{
							if((portFormat == PORT_USB_C)&&(portIndex == 2))
								return true;
						}
					}
					else if(strstr(buf,"HDMI1") != 0)//HDMI1
					{
						if((portFormat == PORT_HDMI)&&(portIndex == 1))
							return true;
					}
				}
			}
			else if(strstr(buf,"analog") != 0)
			{
				if((portFormat == PORT_ANALOG_AUDIO)&&(g_audio_info.direction == DIRECTION_IN))
					return true;
			}
			else if(strstr(buf,"dante") != 0)
			{
				if((strcmp(g_version_info.model,IPE_P_MODULE) == 0)
					&&(portFormat == PORT_DANTE))
				 	return true;
			}
			else if(strstr(buf,"no") != 0)
				return false;
		}
	}
	else if(direction == DIRECTION_OUT)
	{
		if((signal == SIGNAL_AUDIO)&&(strcmp(g_version_info.model,IPE_P_MODULE) == 0))
		{
			for(int i=0;i<=3;i++)
			{
				if(g_audio_info.dst_port[i] == portFormat)
					return true;
			}
		}
	}

#else
	return true;
#endif
	return false;
}
//"#KDS-AUD" 0:hdmi;1:analog;4:dante
int  EX_SetAudSrcMode(int mode)
{
	DBG_InfoMsg("EX_SetAudSrcMode mode =%d\n",mode);

#ifdef CONFIG_P3K_HOST
	char sCmd[64] = "";
	if(mode == 0)	//hdmi
		sprintf(sCmd,"e_p3k_audio_src::hdmi");
	else if(mode == 1)	//analog
	{
		if(g_audio_info.direction == DIRECTION_IN)
		{
			sprintf(sCmd,"e_p3k_audio_src::analog");
		}
		else
		{
			sprintf(sCmd,"e_p3k_audio_src::no");
			DBG_WarnMsg(" !!! Error g_audio_info.direction == DIRECTION_OUT \n");
			return EX_MODE_ERR;
		}
	}
	else if(mode == 2)	 //no
	{
		sprintf(sCmd,"e_p3k_audio_src::no");
	}
	else if(mode == 4)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			sprintf(sCmd,"e_p3k_audio_src::dante");
		}
		else
		{
			sprintf(sCmd,"e_p3k_audio_src::no");
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_CMD_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error mode:%d \n",mode);
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Source(SIGNAL_AUDIO,mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;
}
int  EX_GetAudSrcMode(int *mode)
{
	*mode = 0; //default is hdmi
	char buf[64] = "";
	memset(buf,0,64);

#ifdef CONFIG_P3K_CLIENT
	DBG_WarnMsg("This is not encoder!!!\n");
	return EX_CMD_ERR;
#endif

	//Get Cuurent hdmi in
	mysystem("/usr/local/bin/sconfig --show audio-input", buf, 64);
	if(strstr(buf,"hdmi") != 0)
		*mode = 0;
	else if(strstr(buf,"analog") != 0)
	{
		if(g_audio_info.direction == DIRECTION_IN)
			*mode = 1;
		else
			*mode = 2;
	}
	else if(strstr(buf,"dante") != 0)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		 	*mode = 4;
		else
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_CMD_ERR;
		}
	}
	else if(strstr(buf,"no") != 0)
		*mode = 2;
	else
		*mode = 2;

//	Cfg_Get_Autoswitch_Source(SIGNAL_AUDIO,mode);
	return EX_NO_ERR;
}

//only for analog out
int EX_SetAudGainLevel(PortInfo_S*info,int gain)
{
	DBG_InfoMsg("gain =%d\n",gain);
	char sCmd[64] = "";

#ifdef CONFIG_P3K_HOST
	if(g_audio_info.direction == DIRECTION_IN)
	{
		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}
#endif

	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, info->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg("IsPortValid Err\n");
		return EX_PARAM_ERR;
	}
	if((info->signal == SIGNAL_AUDIO)
		&&(info->direction == DIRECTION_OUT)
		&&(info->portFormat == PORT_ANALOG_AUDIO)
		&&(info->portIndex == 1)
		&&(info->index == 1)
		&&((gain >=0)&&(gain <= 100)))
	{
		sprintf(sCmd,"e_p3k_audio_level::%d",gain);

		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_AV_Volume(gain);
	}
	else
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}

int EX_SetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	DBG_InfoMsg("EX_SetAudAnalogGainDir, gain =%s\n",gain);
	PortSignalType_E type;
	PortDirectionType_E direction;

	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, 1) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return EX_PARAM_ERR;
	}

	char sCmd[64] = "";
	type = info->portFormat;

	if((info->portIndex != 1)
		||((info->portFormat != PORT_IR)&&(info->portFormat != PORT_ANALOG_AUDIO))
		||((info->signal != SIGNAL_IR)&&(info->signal != SIGNAL_AUDIO))
		||(info->direction != DIRECTION_BOTH))
	{
		DBG_ErrMsg("parameter error\n");
		return EX_PARAM_ERR;
	}

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
			DBG_WarnMsg(" !!! Error Param \n");
			printf("EX_SetIRDir Error, gain =%s\n",gain);
			return EX_PARAM_ERR;
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

			DBG_WarnMsg(" !!! Error Param \n");
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Port_Dir(type,direction);
	return EX_NO_ERR;
}


int EX_GetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	PortSignalType_E type;
	PortDirectionType_E direction;

	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, 1) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return EX_PARAM_ERR;
	}

	if((info->portIndex != 1)
		||((info->portFormat != PORT_IR)&&(info->portFormat != PORT_ANALOG_AUDIO))
		||((info->signal != SIGNAL_IR)&&(info->signal != SIGNAL_AUDIO))
		||(info->direction != DIRECTION_BOTH))
	{
		DBG_ErrMsg("parameter error\n");
		return EX_PARAM_ERR;
	}

	if((info->portFormat == PORT_IR)||(info->portFormat == PORT_ANALOG_AUDIO))
	{
		type = info->portFormat;
		Cfg_Get_Port_Dir(type,&direction);
	}

	if(direction == DIRECTION_OUT)
		strcpy(gain,"out");
	else if(direction == DIRECTION_IN)
		strcpy(gain,"in");
	else
	{
#ifdef CONFIG_P3K_HOST
		strcpy(gain,"in");
#else
		strcpy(gain,"out");
#endif
	}

	return EX_NO_ERR;
}



int EX_SetEDIDMode(EdidInfo_S *info)
{
	DBG_InfoMsg(">>EX_SetEDIDMode %d,%d\n",info->input_id,info->index);

#ifdef CONFIG_P3K_HOST
	if(info->input_id != 1)
	{
		DBG_WarnMsg(" !!! Error Param \n");
		return EX_PARAM_ERR;
	}
	//DBG_InfoMsg("gain =%s\n",gain);
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}

	char sCmd[64] = "";
	if(info->mode == PASSTHRU)
	{
		if(strlen(g_edid_info.net_src) > 0)
			sprintf(sCmd,"e_p3k_video_edid_passthru::%s",g_edid_info.net_src);
		else
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}
	}
	else if(info->mode == DEFAULT)
		sprintf(sCmd,"e_p3k_video_edid_default");
	else if(info->mode == CUSTOM)
	{
		if((info->index >=0)&&(info->index <=(MAX_EDID-1)))
			sprintf(sCmd,"e_p3k_video_edid_custom::%d",info->index);
		else
		{
			DBG_WarnMsg(" !!! Error Param \n");
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",info->mode);
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	Cfg_Set_EDID_Mode(info->mode,info->index);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_GetEDIDMode(int cmdID,EdidInfo_S * info)
{
	EdidModeType_E mode;
	int idx;

	if(cmdID != 1)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	Cfg_Get_EDID_Mode(&mode,&idx);

 	//EdidInfo_S tmpconf;
	info->input_id = (unsigned int)cmdID;
	info->mode = mode;

	if(mode == CUSTOM)
		info->index = idx;
	else
		info->index = 0;
	DBG_InfoMsg(">>EX_GetEDIDMode\n");
	return EX_NO_ERR;
}

int EX_AddEDID(EdidName_S * info)
{
	DBG_InfoMsg(">>EX_AddEDID %d %s\n",info->index,info->name);

#ifdef CONFIG_P3K_HOST
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");

		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}

	if((info->index >=1)&&(info->index <=(MAX_EDID-1)))
	{
		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_video_edid_add::%d::%s",info->index,info->name);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_RemoveEDID(int comID)
{
	DBG_InfoMsg(">>EX_RemoveEDID %d \n",comID);

#ifdef CONFIG_P3K_HOST
	if(g_edid_info.lock_mode == ON)
	{
		DBG_InfoMsg(">>>>>>>g_edid_info.lock_mode == ON\n");

		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}

	if((comID < 1)||(comID > (MAX_EDID-1)))
	{
		DBG_WarnMsg(">>>>>>>comID : %d is Wrong!!!\n",comID);
		return EX_PARAM_ERR;
	}

	if(comID < g_edid_info.active_id)
		g_edid_info.active_id = g_edid_info.active_id -1;

	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_remove::%d",comID);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_SetActiveEDID(int input_ID,int index_ID )
{
	DBG_InfoMsg(">>EX_SetActiveEDID %d,%d \n",input_ID,index_ID);

#ifdef CONFIG_P3K_HOST
	if(g_edid_info.lock_mode == ON)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return EX_MODE_ERR;
	}

	if(g_edid_info.edid_mode != CUSTOM)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.edid_mode != CUSTOM\n");
		return EX_MODE_ERR;
	}

	if(input_ID != 1)
	{
		DBG_WarnMsg("input_ID != 1\n");
		return EX_PARAM_ERR;
	}

	if(index_ID > (MAX_EDID-1))
	{
		DBG_WarnMsg("index_ID > (MAX_EDID-1)\n");
		return EX_PARAM_ERR;
	}

	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_custom::%d",index_ID);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_EDID_Active(index_ID);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_SetEDIDNetSrc(int input_ID,char*macAddr )
{
	DBG_InfoMsg(">>EX_SetEDIDNetSrc id=%d mac=%s \n",input_ID,macAddr);

#ifdef CONFIG_P3K_HOST
	if(g_edid_info.lock_mode == ON)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.lock_mode == ON\n");
		return EX_MODE_ERR;
	}

	if(g_edid_info.edid_mode != PASSTHRU)
	{
		DBG_WarnMsg(">>>>>>>g_edid_info.edid_mode != PASSTHRU\n");
		return EX_MODE_ERR;
	}

	if(strlen(macAddr) <= 0)
	{
		DBG_WarnMsg(">>>>>>>strlen(macAddr) <= 0\n");
		return EX_PARAM_ERR;
	}

	if(input_ID != 1)
	{
		DBG_WarnMsg("input_ID != 1\n");
		return EX_PARAM_ERR;
	}

	char sCmd[64] = "";
	sprintf(sCmd,"e_p3k_video_edid_passthru::%s",macAddr);
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_EDID_NetSrc(macAddr);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetEDIDNetSrc(int input_ID,char*macAddr )
{
	//char * version = "00-14-22-01-23-45";
	//memcpy(macAddr,version,strlen(version));
	Cfg_Get_EDID_NetSrc(macAddr);
	return EX_NO_ERR;
}

int EX_GetRecvMsgNum(int msg,char*date )
{
	//int iMsgNum = 100;
	//DBG_WarnMsg("Not support!!!\n");

	sprintf(date,"0");

	return EX_CMD_ERR;
}

int EX_GetVidOutRatio(char*date )
{
	//char value[32] = "16:9";
	//memcpy(date,value,strlen(value));

	char* cmd1 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*Signal Type: HDMI (.*).*$#\\1#gp'";
	char buf1[32] = "";

	mysystem(cmd1,buf1,32);

	if(strlen(buf1) > 1)
		memcpy(date,buf1,strlen(buf1));
	else
		strcpy(date,"N/A");
	return EX_NO_ERR;
}

int EX_SetChannelName(char date[32] )
{
#ifdef CONFIG_P3K_HOST
	Cfg_Set_EncChannel_Name(date);
	DBG_InfoMsg("AV channel Name %s\n",date);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;
}

int EX_SetVidMute(MuteInfo_S * mute )
{

	DBG_InfoMsg(">>EX_SetVidMute %d,%d,%d,%d,%d,%d\n",mute->direction,mute->portFormat,mute->portIndex,
		mute->signal,mute->index,mute->state);

	if(IsPortValid(mute->direction, mute->portFormat, mute->portIndex, mute->signal, mute->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if(mute->signal != SIGNAL_AUDIO)
	{
		DBG_WarnMsg("mute->signal != SIGNAL_AUDIO\n");
		return EX_PARAM_ERR;
	}

	if(mute->portFormat == PORT_HDMI)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if((mute->portIndex != 1)&&(mute->portIndex != 2))
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if((strcmp(g_version_info.model,IPE_MODULE) == 0)
			||(strcmp(g_version_info.model,IPE_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_ANALOG_AUDIO)
	{
		if(mute->portIndex != 1)
		{
			DBG_ErrMsg("parameter error\n");
			return EX_PARAM_ERR;
		}

		if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			if(mute->direction != g_audio_info.direction)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			DBG_ErrMsg("parameter error\n");
			return EX_PARAM_ERR;
		}
	}
	else if(mute->portFormat == PORT_STREAM)
	{
		if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_USB_C)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 3)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 2)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
		}
	}

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
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}


int EX_SetTimeOut(int  iTime )
{

	DBG_InfoMsg(">>EX_SetTimeOut %d\n",iTime);
	Cfg_Set_User_LogoutTime(iTime);
	return EX_NO_ERR;
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
	if(index != g_videowall_info.relative_position)
	{
		DBG_WarnMsg(" !!! Error para index:%d\n",index);
		return EX_MODE_ERR;
	}
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
		return EX_PARAM_ERR;
	}
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetVideoWallStretch(int  index)
{
	if(index != g_videowall_info.relative_position)
	{
		DBG_WarnMsg(" !!! Error para index:%d\n",index);
		return EX_MODE_ERR;
	}

	int mode =1;
	Cfg_Get_VM_Stretch(&mode);

	return mode;
}


int EX_SetStandbyTimeOut(int  iTime )
{
	DBG_InfoMsg(">>EX_SetStandbyTimeOut %d\n",iTime);
	Cfg_Set_Dev_StandbyTime(iTime);

	return EX_NO_ERR;
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

	char cmd[64] = "";

	char* cmd1 = "astparam misc g cursys";
	char buf1[64] = "";

	mysystem(cmd1,buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		DBG_WarnMsg("EX_SetRollback not defined\n");
		return EX_MODE_ERR;
	}
	else if(strstr(buf1,"a") != 0)
	{
		strcpy(g_version_info.standby_version,g_version_info.fw_version);
		sprintf(cmd, "astparam misc s stb_ver %s",g_version_info.standby_version);
		system(cmd);

		Cfg_Update_Version();
		system("astparam misc s cursys b");
//		system("reboot");
	}
	else if(strstr(buf1,"b") != 0)
	{
		strcpy(g_version_info.standby_version,g_version_info.fw_version);
		sprintf(cmd, "astparam misc s stb_ver %s",g_version_info.standby_version);
		system(cmd);

		Cfg_Update_Version();
		system("astparam misc s cursys a");
//		system("reboot");
	}
	else
	{
		return EX_NO_ERR;
	}

	ast_send_event(0xFFFFFFFF,"e_p3k_reset");

	return EX_NO_ERR;
}


int EX_SetIRGateway(int  iIr_mode)
{
	DBG_InfoMsg("Tr gw mode = %d\n",iIr_mode);
	Cfg_Set_GW_IR_Mode((State_E)iIr_mode);

	return EX_NO_ERR;
}

int EX_GetIRGateway(void)
{
	int iIr_mode = g_gateway_info.ir_mode;//0-off 1-on
	//DBG_InfoMsg("EX_GetIRGateway gw mode = %d\n",iIr_mode);
	return iIr_mode;
}

int EX_SetMulticastStatus(char * ip,int ttl )
{
	Cfg_Set_Net_Multicast("0.0.0.0",ttl);
	char sCmd[64] = "";
	sprintf(sCmd,"e_set_ttl::%d",ttl);
	ast_send_event(0xFFFFFFFF,sCmd);

	return EX_NO_ERR;
}


int EX_SetPassword(char * login_level,char * iNew_Pass)
{
	DBG_InfoMsg("name=%s\n",login_level);

	if(!strcmp(login_level,"admin"))
		Cfg_Set_User_Pass(1,iNew_Pass);
	else
	{
		DBG_WarnMsg("EX_SetPassword login_level:%s is Wrong!!!\n",login_level);
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}


int EX_GetPassword(char * login_level, char * ologin_Pass)
{
	char * admin = "admin";

	if(0 == strcmp(login_level,admin))
	{
		DBG_InfoMsg("name=%s\n",login_level);
		strcpy(ologin_Pass,g_user_info.password);
		return EX_NO_ERR;
	}

	DBG_WarnMsg(" !!! Error para\n");
	return EX_PARAM_ERR;
}

int EX_SetGatewayPort(int iGw_Type,int iNetw_Id)
{
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if(iGw_Type == Net_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE)!= 0)
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_PARAM_ERR;
		}
	}


	if(((iGw_Type == Net_CONTROL)||(iGw_Type == Net_DANTE))
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
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}

int EX_GetGatewayPort(int iGw_Type)
{
	int iNetw_Id = 0;
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if(iGw_Type == Net_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE)!= 0)
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_PARAM_ERR;;
		}
	}

	if((iGw_Type == Net_CONTROL)||(iGw_Type == Net_DANTE))
	{
		Cfg_Get_Net_GW_Port((NetGWType_E)iGw_Type,&iNetw_Id);
	}
	else
	{
		DBG_WarnMsg("EX_GetGatewayPort iGw_Type:%d is Wrong!!!\n",iGw_Type);
		return EX_PARAM_ERR;;
	}

	return iNetw_Id;
}


// iGw_Type: 0 p3k & RS232; 1: Dante
int EX_GetVlanTag(int iGw_Type)
{
	int iTag = 11;
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if(iGw_Type == Net_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE)!= 0)
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_PARAM_ERR;
		}
	}

	if((iGw_Type == Net_CONTROL)||(iGw_Type == Net_DANTE))
	{
		Cfg_Get_Net_GW_Vlan((NetGWType_E)iGw_Type,&iTag);
	}
	else
	{
		DBG_WarnMsg("EX_GetVlanTag iGw_Type:%d is Wrong!!!\n",iGw_Type);
		return EX_PARAM_ERR;;
	}

	return iTag;
}

// iGw_Type: 0 p3k & RS232; 1: Dante
int EX_SetVlanTag(int iGw_Type,int iTag)
{
	DBG_InfoMsg("iGw_Type=%d\n",iGw_Type);

	if(iGw_Type == Net_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE)!= 0)
		{
			DBG_WarnMsg("This is not switcher!!!\n");
			return EX_PARAM_ERR;
		}
	}

	if((iGw_Type == Net_CONTROL)||(iGw_Type == Net_DANTE))
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

		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
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
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	Cfg_Set_Net_Method((NetMethodType_E)mode);
	return EX_NO_ERR;
}
int EX_GetMethod(void)
{
	NetMethodType_E mode = Net_MULTICAST;

	Cfg_Get_Net_Method(&mode);
	return (int)mode;
}

int EX_SetOsdDisplay(int    mode )
{
	State_E osd_mode = OFF;

	if(mode == 0)
		osd_mode = OFF;
	else if((mode == 1)||(mode == 2))
		osd_mode = ON;
	else
	{
		DBG_ErrMsg(">>mode Err %d\n",mode);
		return EX_PARAM_ERR;
	}

	Cfg_Set_OSD_Diaplay(osd_mode);

    char SetOsdDisplay[BUFSIZE] = {0};
	sprintf(SetOsdDisplay,"SET_OSD_DISPALY  %d",mode);
	int ret = sendCmdtoGUI(SetOsdDisplay);

	DBG_InfoMsg("sendCmdtoGUI ret = %d",ret);

	return EX_NO_ERR;
}
int EX_GetOsdDisplay(void)
{
//	char getOSDDispalyCmd[BUFSIZE] = {"GET_OSD_DISPALY"};
	int mode = g_osd_enable;

	return mode;
}

int EX_SetDaisyChain(int     mode )
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
	else
	{
		DBG_ErrMsg(">>mode Err %d\n",mode);
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetDaisyChain(void)
{

	State_E daisy_state = OFF;
	Cfg_Get_Net_DaisyChain(&daisy_state);

	return (int)daisy_state;
}

int EX_GetVidMute(MuteInfo_S * mute)
{
	if(IsPortValid(mute->direction, mute->portFormat, mute->portIndex, mute->signal, mute->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if(mute->signal != SIGNAL_AUDIO)
	{
		DBG_WarnMsg("mute->signal != SIGNAL_AUDIO\n");
		return EX_PARAM_ERR;
	}

	if(mute->portFormat == PORT_HDMI)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if((mute->portIndex != 1)&&(mute->portIndex != 2))
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if((strcmp(g_version_info.model,IPE_MODULE) == 0)
			||(strcmp(g_version_info.model,IPE_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_ANALOG_AUDIO)
	{
		if(mute->portIndex != 1)
		{
			DBG_ErrMsg("parameter error\n");
			return EX_PARAM_ERR;
		}

		if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			if(mute->direction != g_audio_info.direction)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_DANTE)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			DBG_ErrMsg("parameter error\n");
			return EX_PARAM_ERR;
		}
	}
	else if(mute->portFormat == PORT_STREAM)
	{
		if((strcmp(g_version_info.model,IPD_MODULE) == 0)
			||(strcmp(g_version_info.model,IPD_W_MODULE) == 0))
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			if(mute->direction != DIRECTION_OUT)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 1)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
	}
	else if(mute->portFormat == PORT_USB_C)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 3)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if(mute->direction != DIRECTION_IN)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}

			if(mute->portIndex != 2)
			{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
				DBG_ErrMsg("parameter error\n");
				return EX_PARAM_ERR;
		}
	}

	//int mute_mode = 0;
	//return mute_mode;
	State_E state;

	Cfg_Get_AV_Mute(&state);

	mute->state = state;
	return EX_NO_ERR;
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
				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					sprintf(sCmd,"%s::hdmi",sCmd);
					port[nCount] = PORT_HDMI;
					nCount++;
				}
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
					DBG_ErrMsg("parameter error\n");
					return EX_PARAM_ERR;
				}
			}
			else if(out_id == AUDIO_OUT_DANTE)
			{
				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					sprintf(sCmd,"%s::dante",sCmd);
					port[nCount] = PORT_DANTE;
					nCount++;
				}
				else
				{
					DBG_ErrMsg("parameter error\n");
					return EX_PARAM_ERR;
				}
			}
			else if(out_id == AUDIO_OUT_STREAM)
			{
				sprintf(sCmd,"%s::lan",sCmd);
				port[nCount] = PORT_STREAM;
				nCount++;
			}
			else
			{
				DBG_WarnMsg(" !!! Error para\n");
				return EX_PARAM_ERR;
			}
		}
	}


	{
		Cfg_Set_Audio_Dest(nCount,port);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	return EX_NO_ERR;
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
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(strlen(str) > 0)
					sprintf(str,"%s%d",str,AUDIO_OUT_HDMI);
				else
					sprintf(str,"%d",AUDIO_OUT_HDMI);
			}
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
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(strlen(str) > 0)
					sprintf(str,"%s%d",str,AUDIO_OUT_DANTE);
				else
					sprintf(str,"%d",AUDIO_OUT_DANTE);
			}
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

	return EX_NO_ERR;
}

int EX_GetHWVersion(char * date)
{
	//char * version = "1.12.123";
	//memcpy(date,version,strlen(version));

	if(date != NULL)
	{
		GetBoardInfo(BOARD_HW_VERSION, date, 24);
	}
	return EX_NO_ERR;
}

int EX_GetStandbyVersion(char * date)
{
	//char * version = "1.12.123";
	//memcpy(date,version,strlen(version));

	strcpy(date,g_version_info.standby_version);
	return EX_NO_ERR;
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
	char buf1[16] = "";

	mysystem("get_temperature",buf1,16);

	int iTemp = atoi(buf1);

	if(iMode == 1)
		iTemp = 18*iTemp/10 + 32;
	else if(iMode != 0)
	{
		DBG_ErrMsg("parameter error\n");
		return EX_PARAM_ERR;
	}

	return iTemp;
}

int EX_GetUPGTime(char * day,char * time)
{
	DBG_InfoMsg(" EX_GetUPGTime\n");
	sscanf(g_version_info.upg_time,"%[^,],%s",day,time);

	return EX_NO_ERR;
}

int EX_GetChannelName(char * date)
{
#ifdef CONFIG_P3K_HOST
		Cfg_Get_EncChannel_Name(date);
#else
		DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_GetActiveEDID(int input_ID )
{
#ifdef CONFIG_P3K_HOST
	if(input_ID != 1)
	{
		DBG_WarnMsg("input_ID != 1\n");
		return EX_PARAM_ERR;
	}

	int index = 1;

	Cfg_Get_EDID_Active(&index);
	return index;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
}

int EX_GetEdidList(char info[][MAX_EDID_LEN],int num)
{
#ifdef CONFIG_P3K_HOST
 	int tmpnum = 0;
	tmpnum = Cfg_Get_EDID_List(info,num);
	return tmpnum;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
}

int EX_GetSendMsgNum(int msg,char*date )
{
	int iMsgNum = 100;

	return iMsgNum;
}

int EX_GetAudGainLevel(PortInfo_S*info,int *gain)
{
#ifdef CONFIG_P3K_HOST
	if(g_audio_info.direction == DIRECTION_IN)
	{
		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}
#endif
	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, info->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if((info->signal == SIGNAL_AUDIO)
		&&(info->direction == DIRECTION_OUT)
		&&(info->portFormat == PORT_ANALOG_AUDIO)
		&&(info->portIndex == 1)
		&&(info->index == 1))
	{
		Cfg_Get_AV_Volume(gain);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return  EX_NO_ERR;
}
int EX_GetAudParam(PortInfo_S*info,AudioSignalInfo_S*param)
{
	param->chn = 2;
	memset(param->format,0,16);
	param->sampleRate= SAMPLE_RATE_NONE;

	if(IsPortActive(info->direction, info->portFormat, info->portIndex, info->signal, info->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if(info->signal != SIGNAL_AUDIO)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

#ifdef CONFIG_P3K_HOST
	char* cmd_ch = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Valid Ch: (.*).*$#\\1#gp'";
	char* cmd_type = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Type: (.*).*$#\\1#gp'";
	char* cmd_freq = "cat /sys/devices/platform/1500_i2s/input_audio_info | sed -rn 's#^.*Sample Freq: (.*)KHz.*$#\\1#gp'";
#else
	if(info->direction != DIRECTION_OUT)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

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
	if(strlen(param->format) == 0)
		sprintf(param->format,"N/A");

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
	else if(rate == 88)
		param->sampleRate = SAMPLE_RATE_88200;
	else if(rate == 96)
		param->sampleRate = SAMPLE_RATE_96000;
	else if(rate == 176)
		param->sampleRate = SAMPLE_RATE_176400;
	else if(rate == 192)
		param->sampleRate = SAMPLE_RATE_192000;

	return 0;
}
int EX_SetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E mode)
{
	DBG_InfoMsg("EX_SetAutoSwitchMode mode =%d\n",mode);
#ifdef CONFIG_P3K_HOST
	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, info->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if((info->direction != DIRECTION_OUT)
		||(info->portIndex != 1)
		||(info->index != 1))

	{
		DBG_WarnMsg(" !!!parameter Error\n");
		return EX_PARAM_ERR;
	}

	char sCmd[64] = "";
	if(info->signal == SIGNAL_VIDEO)
	{
		if(strcmp(g_version_info.model,IPE_MODULE) == 0)
		{
			DBG_WarnMsg("!!!Error Cmd\n");
			return EX_CMD_ERR;
		}

		if((info->portFormat != PORT_HDMI)&&(info->portFormat != PORT_STREAM))
		{
			DBG_WarnMsg(" !!!parameter Error portFormat=%d\n",info->portFormat);
			return EX_PARAM_ERR;
		}
		sprintf(sCmd,"e_p3k_switch_mode");
	}
	else if(info->signal == SIGNAL_AUDIO)
	{
		if((info->portFormat != PORT_HDMI)
			&&(info->portFormat != PORT_STREAM)
			&&(info->portFormat != PORT_ANALOG_AUDIO))
		{
			DBG_WarnMsg(" !!!parameter Error portFormat=%d\n",info->portFormat);
			return EX_PARAM_ERR;
		}
		sprintf(sCmd,"e_p3k_audio_mode");
	}
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal=%d\n",info->signal);
		return EX_PARAM_ERR;
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
		return EX_PARAM_ERR;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Mode(info->signal,mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetAutoSwitchMode(PortInfo_S*info,AVConnectMode_E *mode)
{
#ifdef CONFIG_P3K_HOST

	if(IsPortValid(info->direction, info->portFormat, info->portIndex, info->signal, info->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if((info->direction != DIRECTION_OUT)
		||(info->portIndex != 1)
		||(info->index != 1))

	{
		DBG_WarnMsg(" !!!parameter Error\n");
		return EX_PARAM_ERR;
	}

	if(info->signal == SIGNAL_VIDEO)
	{
		if(strcmp(g_version_info.model,IPE_MODULE) == 0)
		{
			DBG_WarnMsg("!!!Error Cmd\n");
			return EX_CMD_ERR;
		}

		if((info->portFormat != PORT_HDMI)&&(info->portFormat != PORT_STREAM))
		{
			DBG_WarnMsg(" !!!parameter Error portFormat=%d\n",info->portFormat);
			return EX_PARAM_ERR;
		}
	}
	else if(info->signal == SIGNAL_AUDIO)
	{
		if((info->portFormat != PORT_HDMI)
			&&(info->portFormat != PORT_STREAM)
			&&(info->portFormat != PORT_ANALOG_AUDIO))
		{
			DBG_WarnMsg(" !!!parameter Error portFormat=%d\n",info->portFormat);
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal=%d\n",info->signal);
		return EX_PARAM_ERR;
	}

	if((info->signal == SIGNAL_VIDEO)||(info->signal == SIGNAL_AUDIO))
	{
		Cfg_Get_Autoswitch_Mode(info->signal,mode);
	}
	else
	{
		DBG_WarnMsg(" !!!parameter Error signal = %d\n",info->signal);
		return EX_PARAM_ERR;
	}

#else
		DBG_WarnMsg(" !!! This is Decoder\n");
		return EX_CMD_ERR;
#endif
		return EX_NO_ERR;

}

//#X-PRIORITY <direction_type>. <port_format>. <port_index> .<signal_type> , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR>
int EX_SetAutoSwitchPriority(AudioInfo_S * info,AudioInfo_S * gain,int count)
{
	DBG_InfoMsg("EX_SetAutoSwitchPriority,Count = %d\n",count);
	if((count == 0)||(count >4))
	{
		DBG_WarnMsg(" !!! Error para\n");
		DBG_WarnMsg(" !!! Error Count = %d\n",count);
		return EX_PARAM_ERR;
	}

#ifdef CONFIG_P3K_HOST

	int port[4] = {255,255,255,255};
	char sCmd[128] = "";
	if(gain[0].signal == SIGNAL_VIDEO)
	{
		if(strcmp(g_version_info.model,IPE_MODULE) == 0)
		{
			DBG_WarnMsg("!!!Error Cmd\n");
			return EX_CMD_ERR;
		}

		sprintf(sCmd,"e_p3k_switch_pri");

		for(int i = 1;i < count;i++)
		{
			if(IsPortValid(gain[i].direction, gain[i].portFormat, gain[i].portIndex, gain[i].signal, 1) == EX_PARAM_ERR)
			{
				DBG_WarnMsg(" !!! Error para\n");
				return EX_PARAM_ERR;
			}

			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(gain[i].portFormat == PORT_HDMI)
				{
					if((gain[i].portIndex != 1)&&(gain[i].portIndex != 2))
					{
						DBG_WarnMsg(" !!! Error para\n");
						return EX_PARAM_ERR;
					}
				}
				else if(gain[i].portFormat == PORT_USB_C)
				{
					if(gain[i].portIndex != 3)
					{
						DBG_WarnMsg(" !!! Error para\n");
						return EX_PARAM_ERR;
					}
				}
				else
				{
					DBG_WarnMsg(" !!! Error para\n");
					return EX_PARAM_ERR;
				}
			}
			else //IPE_W
			{
				if(gain[i].portFormat == PORT_HDMI)
				{
					if(gain[i].portIndex != 1)
					{
						DBG_WarnMsg(" !!! Error para\n");
						return EX_PARAM_ERR;
					}
				}
				else if(gain[i].portFormat == PORT_USB_C)
				{
					if(gain[i].portIndex != 2)
					{
						DBG_WarnMsg(" !!! Error para\n");
						return EX_PARAM_ERR;
					}
				}
				else
				{
					return EX_PARAM_ERR;
				}


			}
			sprintf(sCmd,"%s::HDMI%d",sCmd,gain[i].portIndex);
			port[i] = gain[i].portIndex;
		}
	}
	else if(gain[0].signal == SIGNAL_AUDIO)
	{
		sprintf(sCmd,"e_p3k_audio_pri");
		int jjj = 1;

		for(int i = 1; i < count;i++)
		{
			if(IsPortValid(gain[i].direction, gain[i].portFormat, gain[i].portIndex, gain[i].signal, 1) == EX_PARAM_ERR)
			{
				DBG_WarnMsg("IsPortValid\n");
				return EX_PARAM_ERR;
			}

			if(gain[i].portFormat == PORT_HDMI)
			{
				if(strcmp(g_version_info.model,IPE_MODULE) == 0)
				{
					if(gain[i].portIndex != 1)
					{
						DBG_WarnMsg("gain[i].portIndex != 1\n");
						return EX_PARAM_ERR;
					}
				}
				else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
				{
					if(gain[i].portIndex != 1)
					{
						DBG_WarnMsg("gain[i].portIndex != 1\n");
						return EX_PARAM_ERR;
					}
				}
				else
				{
					if((gain[i].portIndex != 1)&&(gain[i].portIndex != 2))
					{
						DBG_WarnMsg(" !!! Error para\n");
						return EX_PARAM_ERR;
					}
				}

				sprintf(sCmd,"%s::hdmi",sCmd);
				port[jjj] = AUDIO_IN_HDMI;
				jjj++;
			}
			else if(gain[i].portFormat == PORT_USB_C)
			{
				if(strcmp(g_version_info.model,IPE_MODULE) == 0)
				{
					DBG_WarnMsg(" !!! Error para\n");
					return EX_PARAM_ERR;
				}
				else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
				{
					if(gain[i].portIndex != 2)
					{
						DBG_WarnMsg("gain[i].portIndex != 2\n");
						return EX_PARAM_ERR;
					}
				}
				else
				{
					if(gain[i].portIndex != 3)
					{
						DBG_WarnMsg("gain[i].portIndex != 3\n");
						return EX_PARAM_ERR;
					}
				}

				sprintf(sCmd,"%s::hdmi",sCmd);
				port[jjj] = AUDIO_IN_HDMI;
				jjj++;
			}
			else if(gain[i].portFormat == PORT_ANALOG_AUDIO)
			{
				if(gain[i].portIndex != 1)
				{
					DBG_WarnMsg("gain[i].portIndex != 1\n");
					return EX_PARAM_ERR;
				}
				if(g_audio_info.direction == DIRECTION_IN)
				{
					sprintf(sCmd,"%s::analog",sCmd);
					port[jjj] = AUDIO_IN_ANALOG;
					jjj++;
				}
				else
				{
					DBG_WarnMsg("g_audio_info.direction != DIRECTION_IN\n");
					return EX_PARAM_ERR;
				}
			}
			else if(gain[i].portFormat == PORT_DANTE)
			{
				if(gain[i].portIndex != 1)
				{
					DBG_WarnMsg("gain[i].portIndex != 1\n");
					return EX_PARAM_ERR;
				}
				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					sprintf(sCmd,"%s::dante",sCmd);
					port[jjj] = AUDIO_IN_DANTE;
					jjj++;
				}
			}
			else
			{
				DBG_WarnMsg("gain[%d].portFormat %d\n",i,gain[i].portFormat);
				return EX_PARAM_ERR;
			}
		}
	}
	else
	{
		DBG_WarnMsg(" !!!EX_SetAutoSwitchPriority parameter Error\n");
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Autoswitch_Priority(gain[0].signal,port[1],port[2],port[3]);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_GetAutoSwitchPriority(AudioInfo_S * gain,int count)
{
	int num = 0;
#ifdef CONFIG_P3K_HOST
	if(IsPortValid(gain[0].direction, gain[0].portFormat, gain[0].portIndex, gain[0].signal, 1) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if((gain[0].direction != DIRECTION_OUT)
		||(gain[0].portIndex != 1)
		||((gain[0].signal != SIGNAL_VIDEO)&&(gain[0].signal != SIGNAL_AUDIO)))
	{
		DBG_WarnMsg(" !!!parameter Error\n");
		return EX_PARAM_ERR;
	}

	SignalType_E type = gain[0].signal;
	if((gain[0].signal == SIGNAL_VIDEO)||(gain[0].signal == SIGNAL_AUDIO))
	{
		int port[4] = {255,255,255,255};
		Cfg_Get_Autoswitch_Priority(gain[0].signal,&port[1],&port[2],&port[3]);
		DBG_InfoMsg("EX_GetAutoSwitchPriority %d,%d,%d\n",port[1],port[2],port[3]);

		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
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
					gain[num+1].direction = DIRECTION_IN;
					gain[num+1].signal = SIGNAL_AUDIO;
					if(port[i] == AUDIO_IN_HDMI)
					{
						gain[num+1].portFormat = PORT_HDMI;
						gain[num+1].portIndex = 1;
						num++;
					}
					else if(port[i] == AUDIO_IN_ANALOG)
					{
						if(g_audio_info.direction == DIRECTION_IN)
						{
							gain[num+1].portFormat = PORT_ANALOG_AUDIO;
							gain[num+1].portIndex = 1;
							num++;
						}
					}
					else if(port[i] == AUDIO_IN_DANTE)
					{
						gain[num+1].portFormat = PORT_DANTE;
						gain[num+1].portIndex = 1;
						num++;
					}
					else
					{
						//num = i + 1;
					 	break;
					}
				}
			}
		}
		else
		{
			for(int i = 1; i<=2; i++)
			{
				if((type == SIGNAL_VIDEO)&&(strcmp(g_version_info.model,IPE_W_MODULE) == 0))
				{
					gain[i].direction = DIRECTION_IN;
					gain[i].signal = SIGNAL_VIDEO;
					if(port[i] == 1)
					{
						gain[i].portFormat = PORT_HDMI;
						gain[i].portIndex = port[i];
						num++;
					}
					else if(port[i] == 2)
					{
						gain[i].portFormat = PORT_USB_C;
						gain[i].portIndex = port[i];
						num++;
					}
					else
					{
						//num = i + 1;
					 	break;
					}
				}
				else if(type == SIGNAL_AUDIO)
				{
					if(strcmp(g_version_info.model,IPE_MODULE) == 0)
					{
						gain[num+1].direction = DIRECTION_IN;
						gain[num+1].signal = SIGNAL_AUDIO;
						if(port[i] == AUDIO_IN_HDMI)
						{
							gain[num+1].portFormat = PORT_HDMI;
							gain[num+1].portIndex = 1;
							num++;
						}
						else if(port[i] == AUDIO_IN_ANALOG)
						{
							if(g_audio_info.direction == DIRECTION_IN)
							{
								gain[num+1].portFormat = PORT_ANALOG_AUDIO;
								gain[num+1].portIndex = 1;
								num++;
							}
						}
						else
						{
						//	num = i + 1;
						 	break;
						}
					}
					else //IPE_W_MODULE
					{
						gain[num+1].direction = DIRECTION_IN;
						gain[num+1].signal = SIGNAL_AUDIO;
						if(port[i] == AUDIO_IN_HDMI)
						{
							gain[num+1].portFormat = PORT_HDMI;
							gain[num+1].portIndex = 1;
							num++;
						}
						else if(port[i] == AUDIO_IN_ANALOG)
						{
							gain[num+1].portFormat = PORT_ANALOG_AUDIO;
							gain[num+1].portIndex = 1;
							num++;
						}
						else
						{
						//	num = i + 1;
						 	break;
						}
					}
				}
			}
		}
	}
	else
	{
		DBG_WarnMsg(" !!!EX_GetAutoSwitchPriority parameter Error\n");
		return EX_PARAM_ERR;
	}
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif

	return num;
}
int EX_CopyEDID(EDIDPortInfo_S*src,EDIDPortInfo_S*dest,int bitMap,int safemode)
{

	return EX_NO_ERR;
}
int EX_GetEDIDSupport(int index ,int mode ,int *size)
{
	*size = 256;
	return EX_NO_ERR;
}
int EX_SetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E mode)
{
	DBG_InfoMsg("EX_SetEDIDColorSpaceMode mode =%d\n",mode);
	return EX_NO_ERR;
}
int EX_GetEDIDColorSpaceMode(PortInfo_S *info,ColorSpaceMode_E *mode)
{

	 *mode = COLOR_RGB;
	return EX_NO_ERR;
}

int EX_SetEDIDLockStatus(int index,int lock)
{
	DBG_InfoMsg("EX_SetEDIDLockStatus index =%d lock=%d\n",index,lock);
#ifdef CONFIG_P3K_HOST
	Cfg_Set_EDID_Lock((State_E)lock);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetEDIDLockStatus(int index,int *lock)
{
#ifdef CONFIG_P3K_HOST
	State_E tmp_lock;
	Cfg_Get_EDID_Lock(&tmp_lock);

	*lock = (int)tmp_lock;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

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
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);
	Cfg_Set_AV_HDCP(index, (State_E)mode);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetHDCPMode(int index,HDCPMode_E *mode)
{
#ifdef CONFIG_P3K_HOST
	State_E tmp_mode;
	Cfg_Get_AV_HDCP(index, &tmp_mode);

	*mode = (HDCPMode_E)tmp_mode;
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetHDCPStatus(int io,int index)
{
	int status = 0;

	State_E tmp_mode;

	char* cmd1 = "cat /sys/devices/platform/videoip/timing_info | sed -rn 's#^.*HDCP: (.*).*$#\\1#gp'";
	char buf1[16] = "";

	mysystem(cmd1,buf1,16);

	if(strstr(buf1,"On") != 0)
		status = 1;
	else
		status = 0;

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
		return EX_PARAM_ERR;
	}
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetViewMode(ViewMode_E *mode,ViewModeInfo_S*info)
{
    *mode = VIEW_MODE_VIDEOWALL;
	Cfg_Get_VM_Mod(&(info->hStyle),&(info->vStyle));

	return EX_NO_ERR;
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
		return EX_PARAM_ERR;
	}


#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetWndBezelInfo( int *mode ,int *index, WndBezelinfo_S*info)
{
	*mode = 0;
	Cfg_Get_VM_Bezel(index,info);
	return EX_NO_ERR;
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
			else if(info->rotation == ROTATION_90)
			{
				ast_send_event(0xFFFFFFFF,"e_vw_rotate_0");
			}
			else
			{
				DBG_WarnMsg(" !!! Error para\n");
				return EX_PARAM_ERR;
			}

			Cfg_Set_VM_Setup(id,info->rotation);
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para id:%d,info->rotation:%d\n",id,info->rotation);
		return EX_PARAM_ERR;
	}

#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetVideoWallSetupInfo(int* id,VideoWallSetupInfo_S *info)
{
	int rotation = 0;
	Cfg_Get_VM_Setup(id,&rotation);

	info->rotation = (VideoRotation_E)rotation;
	return EX_NO_ERR;
}
int EX_StartOverlay(char*confFile,int outtime)
{
	char startOverlayCmd[BUFSIZE] = {0};
	sprintf(startOverlayCmd,"START_OVERLAY %s %d",confFile,outtime);
	int mode = sendCmdtoGUI(startOverlayCmd);
	DBG_InfoMsg("filename = %s, outtime =%d mode =%d\n",confFile,outtime,mode);
	return EX_NO_ERR;
}
int EX_StopOverlay(void)
{
	const char stopOverlay[BUFSIZE] = {"STOP_OVERLAY"};

	int mode = sendCmdtoGUI(stopOverlay);
	DBG_InfoMsg(" mode =%d\n",mode);
	return EX_NO_ERR;
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
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetEncoderAVChannelId(int *id)
{
#ifdef CONFIG_P3K_HOST
	Cfg_Get_EncChannel_ID(id);
#else
	DBG_WarnMsg(" !!! This is Decoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}

int EX_SetDecoderAVChannelId(ChSelect_S * id)
{
	DBG_InfoMsg(" EX_SetDecoderAVChannelId id =%d i_signalnum = %d\n",id->ch_id,id->i_signalnum);
    int i = 0;
    for(i = 0;i < id->i_signalnum;i ++)
    {
	    printf("%d...\n",id->signal[i]);
    }
#ifdef CONFIG_P3K_CLIENT

	if(g_avsetting_info.action == CODEC_ACTION_PLAY)
	{
		char sCmd[64] = "";
		sprintf(sCmd,"e_reconnect::%04d",id->ch_id);

		char ch_v[2] = "";
		char ch_u[2] = "";
		char ch_a[2] = "";
		char ch_r[2] = "";
		char ch_s[2] = "";
		char ch_c[2] = "";

	   for(i = 0;i < id->i_signalnum;i ++)
	    {
		    if(id->signal[i] == SIGNAL_VIDEO)
				sprintf(ch_v,"v");
			else if(id->signal[i] == SIGNAL_USB)
				sprintf(ch_u,"u");
			else if(id->signal[i] == SIGNAL_AUDIO)
				sprintf(ch_a,"a");
			else if(id->signal[i] == SIGNAL_IR)
				sprintf(ch_r,"r");
			else if(id->signal[i] == SIGNAL_RS232)
				sprintf(ch_s,"s");
			else if(id->signal[i] == SIGNAL_CEC)
				sprintf(ch_c,"c");
	    }

		sprintf(sCmd,"%s::%s%s%s%s%s%s",sCmd,ch_v,ch_u,ch_a,ch_r,ch_s,ch_c);

		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		for(i = 0;i < id->i_signalnum;i ++)
	    {
			char sCmd[64] = "";
		    if(id->signal[i] == SIGNAL_VIDEO)
				sprintf(sCmd,"astparam s ch_select_v %04d",id->ch_id);
			else if(id->signal[i] == SIGNAL_USB)
				sprintf(sCmd,"astparam s ch_select_u %04d",id->ch_id);
			else if(id->signal[i] == SIGNAL_AUDIO)
				sprintf(sCmd,"astparam s ch_select_a %04d",id->ch_id);
			else if(id->signal[i] == SIGNAL_IR)
				sprintf(sCmd,"astparam s ch_select_r %04d",id->ch_id);
			else if(id->signal[i] == SIGNAL_RS232)
				sprintf(sCmd,"astparam s ch_select_s %04d",id->ch_id);
			else if(id->signal[i] == SIGNAL_CEC)
				sprintf(sCmd,"astparam s ch_select_c %04d",id->ch_id);

			system(sCmd);

			DBG_InfoMsg(" %s\n",sCmd);
	    }

		system("astparam save");
	}

	Cfg_Set_DecChannel_ID(id);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetDecoderAVChannelId(ChSelect_S * id)
{
	//DBG_InfoMsg(">>%d\n",id->signal);
	printf(">>>>>%d\n",id->signal[id->i_signalnum-1]);
    id->ch_id = 1;
	char* cmd1 = "astparam g ch_select_v";
	char buf1[64] = "";

	mysystem(cmd1,buf1,64);

	if(strstr(buf1,"not defined") != 0)
	{
		id->ch_id = 1;
		DBG_WarnMsg("EX_GetDecoderAVChannelId not defined\n");
	}
	else
	{
		id->ch_id = atoi(buf1);
		DBG_InfoMsg("EX_GetDecoderAVChannelId id = %d\n",id->ch_id);

		if(id->ch_id == 0)
			id->ch_id = 1;
	}

	return EX_NO_ERR;
}
int EX_SetVideoImageStatus(int scalerId,VideoStatusType_E status)
{
	DBG_InfoMsg(" EX_SetVideoImageStatus status =%d\n",status);
	return EX_NO_ERR;
}
int EX_GetVideoImageStatus(int scalerId,VideoStatusType_E *status)
{
	*status = VIDEO_ENABLED;
	return EX_NO_ERR;
}

int EX_SetVideoCodecAction(CodecActionType_E type)
{
	DBG_InfoMsg("EX_SetVideoCodecAction type=%d\n",type);
	char sCmd[64] = "";
	if(type == CODEC_ACTION_PLAY)
		sprintf(sCmd,"e_reconnect");
	else if(type == CODEC_ACTION_STOP)
		sprintf(sCmd,"e_stop_link");
	else if(type == CODEC_ACTION_SAVECONFIG)
	{
		system("sync");
		return EX_NO_ERR;
	}
	else
	{
		DBG_WarnMsg(" !!! Error para %d\n",type);
		return EX_PARAM_ERR;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_AV_Action(type);

	return EX_NO_ERR;
}

int EX_GetVideoCodecAction(CodecActionType_E *type)
{
	Cfg_Get_AV_Action(type);
	return EX_NO_ERR;
}
int EX_SetColorSpaceConvertMode(int index,int convertMode)
{
	DBG_InfoMsg("EX_SetColorSpaceConvertMode id = %d mode =%d\n",index,convertMode);
#ifdef CONFIG_P3K_CLIENT
	char sCmd[64] = "";
	if(convertMode == 0)
	{
		sprintf(sCmd,"e_p3k_video_rgb::0");
	}
	else if(convertMode == 1)
	{
		sprintf(sCmd,"e_p3k_video_rgb::5");
	}
	else
	{
		DBG_WarnMsg(" !!! Error para %d\n",convertMode);
		return EX_PARAM_ERR;
	}
	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Video_RGB(convertMode);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

}
int EX_GetColorSpaceConvertMode(int index,int *convertMode)
{
#ifdef CONFIG_P3K_CLIENT
	Cfg_Get_Video_RGB(convertMode);
#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

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
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
		return EX_PARAM_ERR;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Video_Res(mode,res);

#else
	DBG_WarnMsg(" !!! This is Encoder\n");
	return EX_CMD_ERR;
#endif
	return EX_NO_ERR;

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
	*mode = 0;
	strcpy(res,"0");

	char buf[64] = "";

	mysystem("astparam g v_output_timing_convert",buf,64);
	if(strstr(buf,"not defined") != 0)
	{
		*mode = 0;
		strcpy(res,"0");
	}
	else if(strcmp("0",buf) == 0)
	{
		*mode = 0;
		strcpy(res,"0");
	}
	else if(strcmp("8000005F",buf) == 0)
	{
		*mode = 1;
		strcpy(res,"74");
	}
	else if(strcmp("8000005E",buf) == 0)
	{
		*mode = 1;
		strcpy(res,"73");
	}
	else if(strcmp("80000010",buf) == 0)
	{
		*mode = 1;
		strcpy(res,"16");
	}
	else if(strcmp("8000001F",buf) == 0)
	{
		*mode = 1;
		strcpy(res,"31");
	}
	else if(strcmp("80000004",buf) == 0)
	{
		*mode = 1;
		strcpy(res,"4");
	}


	return EX_NO_ERR;
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

	if(strcmp(" [1920]X[2160] [60]",buf1) == 0)
	{
		strcpy(buf1, " [3840]X[2160] [60]");
	}

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

	return EX_NO_ERR;
}
int EX_GetVideoFrameRate(int *fps)
{
	*fps = 60;
	return EX_NO_ERR;
}
int EX_GetVideoBitRate(int *kbps)
{
      *kbps=1000;
	return EX_NO_ERR;
}

int EX_SendCECMsg(CECMessageInfo_S*info)
{
	int sendStatus = 0;
	char sCmd[128] = "";

	DBG_InfoMsg("EX_SendCECMsg\n");

	if(g_gateway_info.cec_mode == OFF)
	{
		DBG_WarnMsg("CEC Gateway Disable\n");
		sendStatus = 1;
		return EX_MODE_ERR;
	}
	else
	{
		DBG_WarnMsg("cec param info->portId = %d\n",info->portId);
		if((info->hexByte > 0)&&(info->hexByte <= 16))
		{
			//check command valid
			if(strlen(info->cmdComent) != (info->hexByte * 2))
			{
				DBG_WarnMsg("cec param Err info->hexByte = %d , info->cmdComent = %s\n",info->hexByte,info->cmdComent);
				return EX_PARAM_ERR;
			}

			if(strspn(info->cmdComent, "0123456789abcdefABCDEF")!=strlen(info->cmdComent))
			{
				DBG_WarnMsg("cec cmd Err %s\n",info->cmdComent);
				return EX_PARAM_ERR;
			}

			sprintf(sCmd,"e_p3k_cec_send:");
			for(int i=0;i<info->hexByte;i++)
				sprintf(sCmd,"%s:%c%c",sCmd,info->cmdComent[2*i],info->cmdComent[2*i+1]);
		}
		else
		{
			DBG_WarnMsg(" !!! Error para info->hexByte:%d\n",info->hexByte);
			return EX_PARAM_ERR;
		}
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
	return EX_NO_ERR;
}
int EX_SetCECGateWayMode(int mode)
{
	printf("EX_SetCECGateWayMode mode =%d\n",mode);
	char sCmd[64] = "";
	if((mode >= 0)&&(mode<=3))
	{
#ifdef CONFIG_P3K_HOST
		if((strcmp(g_version_info.model,IPE_W_MODULE) == 0)
			&&((mode == 2)||(mode == 3)))
		{
			DBG_WarnMsg(" !!! IPE_W_MODULE, Error para mode:%d\n",mode);
			return EX_PARAM_ERR;
		}

		if(mode == 0)
			sprintf(sCmd,"e_p3k_cec_gw::over_ip");
		else if(mode == 1) //in
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_in");
		else if(mode == 2) //out
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_out");
		else //loop == 2
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_out");
#else
		if((strcmp(g_version_info.model,IPD_W_MODULE) == 0)
			&&((mode == 1)||(mode == 3)))
		{
			DBG_WarnMsg(" !!! IPD_W_MODULE, Error para mode:%d\n",mode);
			return EX_PARAM_ERR;
		}

		if(mode == 0)
			sprintf(sCmd,"e_p3k_cec_gw::over_ip");
		else if(mode == 1) //in
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_in");
		else if(mode == 2) //out
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_out");
		else //loop == 1
			sprintf(sCmd,"e_p3k_cec_gw::hdmi_in");
#endif
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);

		Cfg_Set_GW_CEC_Mode(mode);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para mode:%d\n",mode);
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
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
	/*
	if((g_gateway_info.ir_mode == OFF)||(g_gateway_info.ir_direction == DIRECTION_IN))
	{
		DBG_ErrMsg("SendIR Err! g_gateway_info.ir_mode = %d, g_gateway_info.ir_direction = %d\n",g_gateway_info.ir_mode,g_gateway_info.ir_direction);
		return EX_MODE_ERR;
	}

	char sCmd[512] = "";

	if(strspn(info->cmdComent, "0123456789abcdefABCDEF")!=strlen(info->cmdComent))
	{
		DBG_WarnMsg("IR cmd Err %s\n",info->cmdComent);
		return EX_PARAM_ERR;
	}

	sprintf(sCmd,"irs");
	int len = strlen(info->cmdComent) /4;
	for(int i=0;i<len;i++)
		sprintf(sCmd,"%s %c%c%c%c",sCmd,info->cmdComent[4*i],info->cmdComent[4*i+1],info->cmdComent[4*i+2],info->cmdComent[4*i+3]);

	system(sCmd);
	*/
	DBG_InfoMsg("system %s\n",sCmd);

	return EX_NO_ERR;
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
	if(IsPortValid(matchPortInfo->direction, matchPortInfo->portFormat, matchPortInfo->portIndex, matchPortInfo->signal, matchPortInfo->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if(matchPortInfo->signal == SIGNAL_VIDEO)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			if((matchPortInfo->portIndex >= 1)&&(matchPortInfo->portIndex <= 3))
			{
				sprintf(sCmd,"e_p3k_switch_in::HDMI%d",matchPortInfo->portIndex);
				Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,matchPortInfo->portIndex);
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			if((matchPortInfo->portIndex >= 1)&&(matchPortInfo->portIndex <= 2))
			{
				sprintf(sCmd,"e_p3k_switch_in::HDMI%d",matchPortInfo->portIndex);
				Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,matchPortInfo->portIndex);
			}
		}
		else
		{
			DBG_WarnMsg("!!! Error This is IPE5000\n");
			return -1;
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
			if(IsPortValid(inPortInfo[i].direction, inPortInfo[i].portFormat, inPortInfo[i].portIndex, inPortInfo[i].signal, inPortInfo[i].index) == EX_PARAM_ERR)
			{
				DBG_WarnMsg(" !!! Error para\n");
				return EX_PARAM_ERR;
			}

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
				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
				{
					sprintf(sCmd,"%s::dante",sCmd);
					port[nCount] = PORT_DANTE;
					nCount++;
				}
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
	if(IsPortValid(matchPortInfo->direction, matchPortInfo->portFormat, matchPortInfo->portIndex, matchPortInfo->signal, matchPortInfo->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	if(matchPortInfo->signal == SIGNAL_VIDEO)
	{
		if(matchPortInfo->portFormat == PORT_HDMI)
		{
			if(strcmp(g_version_info.model,IPD_MODULE) == 0)
			{
				sprintf(sCmd,"e_p3k_switch_in::HDMI");
				Cfg_Set_Autoswitch_Source(SIGNAL_VIDEO,1);
			}
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
			return EX_PARAM_ERR;
		}

		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else if(matchPortInfo->signal == SIGNAL_TEST)
	{
		printf("!!! matchPortInfo->signal == SIGNAL_TEST\n");

	}
#endif
	return EX_NO_ERR;
}

int EX_GetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo)
{
	if(IsPortValid(inPortInfo->direction, inPortInfo->portFormat, inPortInfo->portIndex, inPortInfo->signal, inPortInfo->index) == EX_PARAM_ERR)
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

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
		if(strcmp(g_version_info.model,IPE_W_MODULE)==0)
		{
			if(!memcmp(p,"HDMI2",strlen("HDMI2")))
			{
				matchPortInfo->direction = DIRECTION_IN;
				matchPortInfo->portFormat = PORT_USB_C;
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
		}
		else if(strcmp(g_version_info.model,IPE_P_MODULE)==0)
		{
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
	else if((inPortInfo->signal == SIGNAL_AUDIO)
		&&(inPortInfo->direction == DIRECTION_OUT))
	{
		DBG_InfoMsg("EX_GetRouteMatch AUDIO\n");
#ifdef CONFIG_P3K_HOST
		matchPortInfo->signal = SIGNAL_AUDIO;

		matchPortInfo->direction = DIRECTION_IN;
		matchPortInfo->portFormat = PORT_HDMI;
		matchPortInfo->portIndex = 1;
		matchPortInfo->index = 1;

		char buf[64] = "";
		memset(buf,0,64);

		//Get Cuurent hdmi in
		mysystem("/usr/local/bin/sconfig --show audio-input", buf, 64);
		if(strstr(buf,"hdmi") != 0)
			matchPortInfo->portFormat = PORT_HDMI;
		else if(strstr(buf,"analog") != 0)
			matchPortInfo->portFormat = PORT_ANALOG_AUDIO;
		else if(strstr(buf,"dante") != 0)
		{
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			 matchPortInfo->portFormat = PORT_DANTE;
		}
#endif
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;

}

int EX_SetUartConf(UartMessageInfo_S*conf)
{
	if(conf->comNumber == 1)
	{
		Cfg_Set_GW_Uart_Param(*conf);

		char cmd[128] = "";
		if(conf->parity == PARITY_ODD)
			sprintf(cmd,"e_p3k_soip_param::%d-%do%d",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);
		else if(conf->parity == PARITY_EVEN)
			sprintf(cmd,"e_p3k_soip_param::%d-%de%d",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);
		else if(conf->parity == PARITY_NONE)
			sprintf(cmd,"e_p3k_soip_param::%d-%dn%d",conf->rate,conf->bitWidth,(int)conf->stopBitsMode);
		else
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}

		DBG_InfoMsg("ast_send_event %s\n",cmd);
		ast_send_event(0xFFFFFFFF,cmd);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetUartConf(int comId,UartMessageInfo_S*conf)
{
	if(comId == 1)
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
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_AddComRoute(ComRouteInfo_S*info,int comId)
{
	if(comId == 1)
	{
		if((info->portNumber<5000)||(info->portNumber>5999))
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}

		if(info->portNumber == g_network_info.tcp_port)
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}

		Cfg_Set_GW_COM_Add(info->portNumber);

		//system("astparam s no_soip n;astparam save");
		char sCmd[64] = "";
		sprintf(sCmd,"e_p3k_soip_gw_on::%d",info->portNumber);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}


	return EX_NO_ERR;
}
int EX_RemoveComRoute(int comId)
{
	if(comId == 1)
	{
		Cfg_Set_GW_COM_Remove();

		DBG_InfoMsg("ast_send_event e_p3k_soip_gw_off\n");
		ast_send_event(0xFFFFFFFF,"e_p3k_soip_gw_off");
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}
int EX_GetComRoute(int comId,ComRouteInfo_S*info)
{
	State_E mode;
	int port;

	if(comId == 1)
	{
		Cfg_Get_GW_COM_Status(&mode,&port);
		if(mode == ON)
		{
			info->HeartTimeout = 60;
			info->portNumber = port;
			info->portType = 0;
			info->rePlay = 0;
		}
		else
		{
			info->HeartTimeout = 0;
			info->portNumber = 0;
			info->portType = 0;
			info->rePlay = 0;

			DBG_WarnMsg(" !!! Error Mode \n");
			return EX_NO_ERR;
			//return EX_MODE_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}
int EX_GetOpenTunnelParam(int tunnelId,TunnelParam_S*param)
{

	param->portNumber= 5001;
	param->portType = 0;
	param->connectType = 0;
	param->reMoteNumber = 4000;
	param->rePlay = 0;
	strcpy(param->ipAddr,"192.168.0.122");
	strcpy(param->comName,"COM");
	return EX_NO_ERR;
}
int EX_SetUSBCtrl(int type)
{
    DBG_InfoMsg("EX_SetUSBCtrl =%d\n",type);
	return EX_NO_ERR;
}
int EX_GetMulticastInfo(char*ip,int *ttl)
{
	Cfg_Get_Net_Multicast(ip,ttl);
	return EX_NO_ERR;
}

int EX_SetMacAddr(int netid,char*macAddr)
{
	DBG_InfoMsg("EX_SetMacAddr mac =%s\n",macAddr);
	if((netid == 0)||(netid == 1))
	{
		int nMac[6];
		if(macAddr != NULL)
		{
			char cmd[128] = "";

			int ret = sscanf(macAddr,"%02x-%02x-%02x-%02x-%02x-%02x",&nMac[0],&nMac[1],&nMac[2],&nMac[3],&nMac[4],&nMac[5]);

			if(ret == 6)
			{
				DBG_WarnMsg("ret = %d\n",ret);
				sprintf(cmd,"astparam w ethaddr %02x:%02x:%02x:%02x:%02x:%02x;astparam save ro",nMac[0],nMac[1],nMac[2],nMac[3],nMac[4],nMac[5]);
				system(cmd);
			}
			else
			{
				DBG_WarnMsg(" !!! Error para\n");
				return EX_PARAM_ERR;
			}
		}
		else
		{
			DBG_WarnMsg("macAddr == NULL\n");
			return EX_PARAM_ERR;
		}
	}
	else
    {
		DBG_WarnMsg("EX_SetMacAddr netId : %d Error\n",netid );
		return EX_PARAM_ERR;
    }

	return EX_NO_ERR;
}
int EX_GetMacAddr(int netid,char*macAddr)
{
//	strcpy(macAddr,"00-14-22-01-23-45");
	DBG_InfoMsg("EX_GetMacAddr %s\n",g_device_info.mac_addr);
	if((netid == 0)||(netid == 1))
	{
		int nMac[6];
		if(macAddr != NULL)
		{
			sscanf(g_device_info.mac_addr,"%x:%x:%x:%x:%x:%x",&nMac[0],&nMac[1],&nMac[2],&nMac[3],&nMac[4],&nMac[5]);

			sprintf(macAddr,"%02X-%02X-%02X-%02X-%02X-%02X",nMac[0],nMac[1],nMac[2],nMac[3],nMac[4],nMac[5]);
		}
	}
	return EX_NO_ERR;
}
int EX_SetDNSName(int id,char*name)
{
	DBG_InfoMsg("EX_SetDNSName name =%s\n",name);
	if(id == 1)
	{
#ifdef CONFIG_P3K_HOST
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{	if(strlen(name)>0)
			{
				if(strcmp(g_audio_info.dante_name,name) == 0)
				{
					DBG_WarnMsg(" !!! dante_name %s is the same\n",name);
					return EX_NO_ERR;
				}
				else
				{
					char sCmd[64] = "";
					sprintf(sCmd,"e_p3k_net_dante_name::%s",name);
					DBG_InfoMsg("ast_send_event %s\n",sCmd);
					ast_send_event(0xFFFFFFFF,sCmd);
				}
			}
		}
		else
		{
			DBG_WarnMsg(" !!! This is not switcher\n");
			return EX_PARAM_ERR;
		}
#else
		DBG_WarnMsg(" !!! This is Decoder\n");
		return EX_PARAM_ERR;
#endif
	}
	else if(id == 0)
	{
		if(strlen(name)>0)
		{
			char sCmd[64] = "";
			sprintf(sCmd,"e_chg_hostname::%s",name);
			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	Cfg_Set_Dev_HostName(id,name);

	return EX_NO_ERR;
}
int EX_GetDNSName(int id,char*name)
{
	if(id == 0)
	{
		mysystem("astparam g hostname_customized",name,64);

		if(strstr(name,"not defined") != 0)
			GetBoardInfo(BOARD_HOSTNAME, name, MAX_DEV_NAME_LEN);
	}
	else if(id == 1)
	{
		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			Cfg_Get_Dev_HostName(1,name);
		else
		{
			DBG_WarnMsg(" !!! This is not switcher\n");
			return EX_CMD_ERR;
		}
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	//strcpy(name,"room-1");
	return EX_NO_ERR;
}
int EX_ResetDNSName(char *name)
{
	int nMac[6];
	//MODEL-MAC
	memset(name,0,MAX_DEV_NAME_LEN);

	sscanf(g_device_info.mac_addr,"%x:%x:%x:%x:%x:%x",&nMac[0],&nMac[1],&nMac[2],&nMac[3],&nMac[4],&nMac[5]);

	sprintf(name,"%s-%02X%02X%02X%02X%02X%02X",g_version_info.model,nMac[0],nMac[1],nMac[2],nMac[3],nMac[4],nMac[5]);

	if(strlen(name)>0)
	{
		char sCmd[64] = "";
		sprintf(sCmd,"e_chg_hostname::%s",name);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	Cfg_Set_Dev_HostName(0,name);

	return EX_NO_ERR;
}
int EX_SetDHCPMode(int netid,int mode)
{
	DBG_InfoMsg("EX_SetDHCPMode ethid= %d mode =%d\n",netid,mode);
	if(netid == 0)
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
		else
		{
			DBG_WarnMsg("EX_SetDHCPMode mode : %d Error\n",mode );
			return EX_PARAM_ERR;
		}
	}
	else if(netid == 1)
	{
		if((mode == 0)||(mode == 1))
			Cfg_Set_Net_DHCP(netid, mode);
		else
		{
			DBG_WarnMsg("EX_SetDHCPMode mode : %d Error\n",mode );
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg("EX_SetDHCPMode netId : %d Error\n",netid );
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetDHCPMode(int netid,int* mode)
{
	*mode =1;
	if((netid == 0)||(netid == 1))
	{
		*mode = g_network_info.eth_info[netid].dhcp_enable;
	}
	else
    {
		DBG_WarnMsg("EX_GetDHCPMode netId : %d Error\n",netid );
		return EX_PARAM_ERR;
    }

	return EX_NO_ERR;

}
int EX_SetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{
	DBG_InfoMsg("EX_SetNetWorkConf ip %s\n",netInfo->ipAddr);

	if(netId == 0)
	{
		if(g_network_info.eth_info[netId].dhcp_enable == 0)
		{
			char sCmd[128] = "";
			sprintf(sCmd,"e_p3k_net_conf::%d::%s::%s::%s",netId,netInfo->ipAddr,netInfo->mask,netInfo->gateway);

			DBG_InfoMsg("ast_send_event %s\n",sCmd);
			ast_send_event(0xFFFFFFFF,sCmd);
		}
		else
	    {
			DBG_WarnMsg("EX_SetNetWorkConf netId : %s dhcp_enable == 0\n",netId );
			return EX_MODE_ERR;
    	}
	}

	if((netId == 0)||(netId == 1))
	{
		Cfg_Set_Net_Config(netId,netInfo);
		if(g_network_info.eth_info[netId].dhcp_enable == 1)
		{
			DBG_WarnMsg("EX_SetNetWorkConf netId : %s dhcp_enable == 0\n",netId );
			return EX_MODE_ERR;
		}
	}
	else
    {
		DBG_WarnMsg("EX_SetNetWorkConf netId : %s Error\n",netId );
		return EX_PARAM_ERR;
    }

	return EX_NO_ERR;

}
int EX_GetNetWorkConf(int netId,NetWorkInfo_S*netInfo)
{
	char ip_buf[32] = "";
	char mask_buf[32] = "";
	GetIPInfo(netId,ip_buf,mask_buf);

	char gw_buf[32] = "";
	char* gw_cmd = "route -n | grep eth0 | grep UG | awk '{print $2}'";
	mysystem(gw_cmd, gw_buf, 32);

	strcpy(netInfo->ipAddr,ip_buf);
	strcpy(netInfo->mask,mask_buf);

	if(strlen(gw_buf)<=0)
		strcpy(netInfo->gateway,"0.0.0.0");
	else
		strcpy(netInfo->gateway,gw_buf);

	strcpy(netInfo->dns1,"0.0.0.0");
	strcpy(netInfo->dns2,"0.0.0.0");
	return EX_NO_ERR;
}
int EX_SetNetPort(char* portType,int portNumber)
{
	DBG_InfoMsg("EX_SetNetPort portType= %s portNumber =%d\n",portType,portNumber);
	if((!strcmp(portType,"udp"))||(!strcmp(portType,"UDP")))
	{

        if(portNumber == g_network_info.udp_port)
        {
            printf("new port == old port err\n");
            return EX_NO_ERR;
        }
        int old_port = g_network_info.udp_port;
        Cfg_Set_Net_Port(Net_UDP,portNumber);
        int sock_fd = socket(AF_INET,SOCK_DGRAM,0);
        struct sockaddr_in myaddr;
    	myaddr.sin_family=AF_INET;
    	myaddr.sin_port = htons(49000);
    	//inet_pton(AF_INET,"10.36.145.183",&myaddr.sin_addr.s_addr); //鏈嶅姟鍣ㄧ殑ip
    	myaddr.sin_addr.s_addr = 0;
    	if(bind(sock_fd,(struct sockaddr*)&myaddr,sizeof(myaddr))<0)
    		perror("bind");

		char ip_buf[32] = "";

		GetIPInfo(1,ip_buf,NULL);

    	struct sockaddr_in addr;// ipv4濂楁帴瀛楀湴鍧€缁撴瀯浣?
    	addr.sin_family =AF_INET;
    	addr.sin_port = htons(old_port);
    	inet_pton(AF_INET,ip_buf,&addr.sin_addr.s_addr); //服务器的ip
    	struct sockaddr_in server_addr;
    	socklen_t len = sizeof(server_addr);
        char buf[24] = "udp_port_change";

        int ret = sendto(sock_fd,buf,strlen(buf),0, (struct sockaddr*)&addr,sizeof(addr));
        printf("--[%d]\n",ret);
        close(sock_fd);
    }
	else if((!strcmp(portType,"tcp"))||(!strcmp(portType,"TCP")))
	{
		if((portNumber<5000)||(portNumber>5999))
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}

		if(portNumber == g_gateway_info.rs232_port)
		{
			DBG_WarnMsg("EX_SetNetPort portType: %s Error\n",portType);
			return EX_PARAM_ERR;
		}
        if(portNumber != g_network_info.tcp_port)
        {
            Cfg_Set_Net_Port(Net_TCP,portNumber);
        }
	}
    else
    {
		DBG_WarnMsg("EX_SetNetPort portType: %s Error\n",portType);
		return EX_PARAM_ERR;
    }

	return EX_NO_ERR;
}
int EX_GetNetPort(char* portType,int *portNumber)
{
	if((!strcmp(portType,"udp"))||(!strcmp(portType,"UDP")))
		Cfg_Get_Net_Port(Net_UDP,portNumber);
	else if((!strcmp(portType,"tcp"))||(!strcmp(portType,"TCP")))
		Cfg_Get_Net_Port(Net_TCP,portNumber);
	else
	{
		DBG_WarnMsg("EX_GetNetPort portType: %s Error\n",portType);
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_SetSecurityStatus(int status)
{
	DBG_InfoMsg("EX_SetSecurityStatus %d\n",status);
	if((status == 0)||(status == 1))
	{
		Cfg_Set_User_Secur((State_E)status);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}

int EX_GetSecurityStatus(int* status)
{
	DBG_InfoMsg("EX_GetSecurityStatus\n");
	Cfg_Get_User_Secur((State_E*)status);
	return EX_NO_ERR;
}

int EX_Login(char*name,char*password)
{
	DBG_InfoMsg("EX_Login name= %s password =%s\n",name,password);
	if((!strcmp(name,"admin"))
		&&((! strcmp(password,g_user_info.password))/*||(! strcmp(password,"33333"))*/))
	{
		return EX_NO_ERR;
	}
	DBG_WarnMsg(" !!! Error para\n");
	return EX_PARAM_ERR;
}
int EX_GetLoginInfo(char*name,char*password)
{
	strcpy(name,"admin");
	return EX_NO_ERR;
}
int EX_Logout(void)
{
	return EX_NO_ERR;
}
int EX_GetDevVersion(char*version)
{
	//char tmp[32];
	if(version != NULL)
	{
		GetBoardInfo(BOARD_FW_VERSION, version, 32);
	}

	//strcpy(version,tmp);
	return EX_NO_ERR;
}

int EX_GetBootVersion(char*version)
{
	if(version != NULL)
	{
		strcpy(version,"v1.1.4");
	}

	return EX_NO_ERR;
}

#define EMPTY_FW_STATUS		"ongoing,0,0"
static void CreateFWStatusFile(void)
{
	FILE *pf = fopen("/www/fw_status.txt", "w+");
	if (NULL != pf)
	{
		fwrite(EMPTY_FW_STATUS, 1, strlen(EMPTY_FW_STATUS), pf);
		fclose(pf);
	}
}

int EX_Upgrade(void)
{

//	Cfg_Set_UPG_Info();
	strcpy(g_version_info.standby_version,g_version_info.fw_version);

	time_t secTime;
	struct tm *ptime =NULL;
	secTime = time(NULL);

	if(g_time_info.ntp_mode == ON)
	{
		DBG_InfoMsg("NTP ON; time_zone = %d\n",g_time_info.time_zone);
//		secTime += g_time_info.time_zone * 3600;
	}

	if(secTime < 0)
		secTime = 0;

	ptime = localtime(&secTime);

	sprintf(g_version_info.upg_time,"%02d-%02d-%04d,%02d:%02d:%02d",ptime->tm_mon+1,ptime->tm_mday,ptime->tm_year+1900,ptime->tm_hour,ptime->tm_min,ptime->tm_sec);

	char cmd[64] = "";
	sprintf(cmd, "astparam misc s upg_time %d",(int)secTime);
	system(cmd);

	memset(cmd,0,64);
	sprintf(cmd, "astparam misc s stb_ver %s",g_version_info.standby_version);
	system(cmd);

	Cfg_Update_Version();

	printf("EX_Upgrade \n");

	ast_send_event(0xFFFFFFFF,"e_p3k_download_fw_start");

	ast_send_event(0xFFFFFFFF,"e_stop_link");

	sleep(2);

	CreateFWStatusFile();
	ast_send_event(0xFFFFFFFF,"e_p3k_upgrade_fw");

	return EX_NO_ERR;
}
int EX_SetDeviceNameModel(char*mod)
{
	DBG_InfoMsg("EX_SetDeviceNameModel %s\n",mod);

	if((mod != NULL)&&(strlen(mod) > 0)&&(strlen(mod) <= 19))
	{
		char cmd[128] = "";

		sprintf(cmd,"astparam w model_number %s;astparam save ro",mod);
		system(cmd);
	}
	else
	{
		DBG_WarnMsg("model Err,strlen(mod)=%d\n",strlen(mod));
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}
int EX_GetDeviceNameModel(char*mod)
{
	DBG_InfoMsg("EX_GetDeviceNameModel %s\n",g_version_info.model);

	if(mod != NULL)
	{
		strcpy(mod,g_version_info.model);
	}

	return EX_NO_ERR;
}
int EX_SetSerialNumber(char*data)
{
	DBG_InfoMsg("EX_SetSerialNumber %s\n",data);
	if((data != NULL)&&(strlen(data) == 14))
	{
		if(strspn(data, "0123456789")==14)
		{
			char cmd[128] = "";
			sprintf(cmd,"astparam w serial_number %s;astparam save ro",data);
			system(cmd);
		}
		else
		{
			DBG_WarnMsg("strspn(data)== %d\n",strspn(data, "0123456789"));
			return EX_PARAM_ERR;
		}
	}
	else
	{
		DBG_WarnMsg("data Err!\n",strspn(data, "0123456789"));
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetSerialNumber(char*data)
{
	//strcpy(data,"12345678987654");
	if(data != NULL)
	{
		strcpy(data,g_device_info.sn);
	}
	return 0;
}

int EX_SetLockFP(int lockFlag)
{
	DBG_InfoMsg("EX_SetLockFP %d\n",lockFlag);

	if(lockFlag == g_device_info.fp_lock)
	{
		DBG_WarnMsg(" !!! Mode para\n");
		return EX_MODE_ERR;
	}

	if((lockFlag == 0)||(lockFlag == 1))
	{
		Cfg_Set_Dev_FPLock((State_E)lockFlag);
		if(lockFlag == 0)
			ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_off");
		else
			ast_send_event(0xFFFFFFFF,"e_p3k_fp_lock_on");

	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}
int EX_GetLockFP(int *lockFlag)
{
	State_E tmp_flag;

	Cfg_Get_Dev_FPLock(&tmp_flag);

	*lockFlag = (int)tmp_flag;

	return EX_NO_ERR;
}
int EX_SetIDV(void)
{
	ast_send_event(0xFFFFFFFF,"e_p3k_flag_me");
	return EX_NO_ERR;
}
int EX_SetStandbyMode(int Mode)
{
	DBG_InfoMsg("EX_SetStandbyMode %d\n",Mode);
	return EX_NO_ERR;
}

int EX_GetStandbyMode(int * value)
{
	int mode = 1;
	value = &mode;
	return EX_NO_ERR;
}

int EX_SetBeacon(int portNumber,int status,int rateSecond)
{
	DBG_InfoMsg("EX_SetBeacon portNumber= %d status= %d rateSecond=%d\n",portNumber,status,rateSecond);
	return EX_NO_ERR;
}
int EX_GetBeaconInfo(int portNumber,BeaconInfo_S*info)
{
    char ip_buf[32] = "";
	GetIPInfo(1,ip_buf,NULL);

    //ip_buf,g_network_info.udp_port,g_network_info.tcp_port,g_device_info.mac_addr,g_version_info.model
	info->portNumber = portNumber;
	info->tcpPort = g_network_info.tcp_port;
	info->udpPort = g_network_info.udp_port;
	strcpy(info->deviceMod,g_version_info.model);
    strcpy(info->deviceName,g_device_info.hostname);
	strcpy(info->ipAddr,ip_buf);
	strcpy(info->macAddr,g_device_info.mac_addr);

	return EX_NO_ERR;
}
int EX_DeviceReset(void)
{
	DBG_InfoMsg("EX_DeviceReset\n");
	ast_send_event(0xFFFFFFFF,"e_p3k_reset");
	//system("sleep 5;reboot&");
	return EX_NO_ERR;
}
int EX_FactoryRecovery(void)
{
	DBG_InfoMsg("EX_FactoryRecovery\n");

	ast_send_event(0xFFFFFFFF,"e_button_link_5");
	return EX_NO_ERR;
}
int EX_GetDevBuildDate(char *date,char*hms)
{
	if((date != NULL)&&(hms!= NULL))
	{
		char buf[128];
		GetBoardInfo(BOARD_BUILD_DATE, buf, 128);

		sscanf(buf,"%s,%s",date,hms);
	}
	return EX_NO_ERR;
}
int EX_SetTimeAndDate(char*weekDay,char*date,char*hms)
{
	DBG_InfoMsg("week:%s date:%s hms:%s\n",weekDay,date,hms);

	if(g_time_info.ntp_mode == OFF)
	{
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
	}
	else
	{
		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetTimeAndDate(char*weekDay,char*date,char*hms)
{
	time_t secTime;
	struct tm *ptime =NULL;
	char *str[] = {"sun","mon","tue","wed","thu","fri","sat"};
	secTime = time(NULL);

	if(g_time_info.ntp_mode == ON)
	{
		DBG_InfoMsg("NTP ON; time_zone = %d\n",g_time_info.time_zone);
//		secTime += g_time_info.time_zone * 3600;
	}

	if(secTime < 0)
		secTime = 0;

	ptime = localtime(&secTime);

	sprintf(weekDay,"%s",str[ptime->tm_wday]);
	sprintf(date,"%02d-%02d-%04d",ptime->tm_mon+1,ptime->tm_mday,ptime->tm_year+1900);
	sprintf(hms,"%02d:%02d:%02d",ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
	return EX_NO_ERR;
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
	return EX_NO_ERR;
}

int EX_SetTimeZero(int tz,int timingMethod)
{
	DBG_InfoMsg("EX_SetTimeZero tz = %d,timingMethod = %d\n",tz,timingMethod);

	if((tz >= -12)&&(tz <= 12))
	{
		Cfg_Set_Time_Loc(tz,timingMethod);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}

	return EX_NO_ERR;
}
int EX_GetTimeZero(int *tz,int *timingMethod)
{
	Cfg_Get_Time_Loc(tz,timingMethod);

	return EX_NO_ERR;
}
int EX_SetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	DBG_InfoMsg("EX_SetTimeSyncInfo server =%s\n",syncInfo->serverIp);
	Cfg_Set_Time_Srv(*syncInfo);

	if(syncInfo->enable == 0)
	{
		ast_send_event(0xFFFFFFFF,"e_p3k_ntp_enable_off");
	}
	else if(syncInfo->enable == 1)
	{
		char sCmd[128] = "";
		sprintf(sCmd,"e_p3k_ntp_enable_on::%s::%d",syncInfo->serverIp,syncInfo->syncInerval);
		DBG_InfoMsg("ast_send_event %s\n",sCmd);
		ast_send_event(0xFFFFFFFF,sCmd);
	}
	else
	{
		DBG_WarnMsg(" !!! Error para\n");
		return EX_PARAM_ERR;
	}
	return EX_NO_ERR;
}
int EX_GetTimeSyncInfo(TimeSyncConf_S*syncInfo)
{
	Cfg_Get_Time_Srv(syncInfo);
	return EX_NO_ERR;
}
int EX_GetSignalList(char info[][MAX_SIGNALE_LEN],int num)
{
	int tmpnum = 0;
	char *str ="in.hdmi.1.audio.1";
	char *str1= "out.hdmi.1.video.1";
	memcpy(info[0],str,strlen(str));
	memcpy(info[1],str1,strlen(str1));

	char buf[64] = "";

#ifdef CONFIG_P3K_HOST
	int hdmi_in_idx = 0,hdmi_out_idx = 0;
	//video
	mysystem("cat /sys/devices/platform/videoip/timing_info", buf, 64);

	if(strstr(buf,"Not Available") != 0)
	{
		DBG_WarnMsg("Video Not Available\n");
	}
	else
	{
		if((strcmp(g_version_info.model,IPE_P_MODULE) == 0)||(strcmp(g_version_info.model,IPE_W_MODULE) == 0))
		{
			memset(buf,0,64);
			//Get Cuurent hdmi in
			mysystem("/usr/local/bin/sconfig --show input", buf, 64);
			if(strstr(buf,"HDMI3") != 0)
			{
				strcpy(info[0],"in.usb_c.3.video.1");
				hdmi_in_idx = 3;
			}
			else if(strstr(buf,"HDMI2") != 0)
			{
				if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
					strcpy(info[0],"in.hdmi.2.video.1");
				else
					strcpy(info[0],"in.usb_c.2.video.1");
				hdmi_in_idx = 2;
			}
			else
			{
				strcpy(info[0],"in.hdmi.1.video.1");
				hdmi_in_idx = 1;
			}
		}
		else
		{
			strcpy(info[0],"in.hdmi.1.video.1");
			hdmi_in_idx = 1;
		}

		strcpy(info[1],"out.stream.1.video.1");

		tmpnum = 2;

		if((strcmp(g_version_info.model,IPE_P_MODULE) == 0)||(strcmp(g_version_info.model,IPE_MODULE) == 0))
		{
			mysystem("astparam g tv_access", buf, 64);

			if(strstr(buf,"not defined") != 0)
			{
				DBG_WarnMsg("tv_access not defined\n");
			}
			else
			{
				if(strstr(buf,"y") != 0)
				{
					strcpy(info[2],"out.hdmi.1.video.1");
					hdmi_out_idx = 1;
					tmpnum++;
				}
			}
		}
	}

	//audio
	mysystem("cat /sys/devices/platform/1500_i2s/input_audio_info", buf, 64);

	if(strstr(buf,"State: Off") != 0)
	{
		DBG_WarnMsg("Audio Not Available\n");
	}
	else
	{
		mysystem("/usr/local/bin/sconfig --show audio-input", buf, 64);
		if(strstr(buf,"hdmi") != 0)
		{
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				if(hdmi_in_idx == 1)
					strcpy(info[tmpnum],"in.hdmi.1.audio.1");
				else if(hdmi_in_idx == 2)
					strcpy(info[tmpnum],"in.hdmi.2.audio.1");
				else if(hdmi_in_idx == 3)
					strcpy(info[tmpnum],"in.usb_c.3.audio.1");
				else
				{
					DBG_WarnMsg("No Audio\n");
					return tmpnum;
				}

			}
			else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
			{
				if(hdmi_in_idx == 1)
					strcpy(info[tmpnum],"in.hdmi.1.audio.1");
				else if(hdmi_in_idx == 2)
					strcpy(info[tmpnum],"in.usb_c.2.audio.1");
				else
				{
					DBG_WarnMsg("No Audio\n");
					return tmpnum;
				}
			}
			else
				strcpy(info[tmpnum],"in.hdmi.1.audio.1");

			tmpnum++;
		}
		else if(strstr(buf,"analog") != 0)
		{
			if(g_audio_info.direction == DIRECTION_IN)
			{
				strcpy(info[tmpnum],"in.analog_audio.1.audio.1");
				tmpnum++;
			}
			else
			{
				DBG_WarnMsg("Audio analog Not input\n");
				return tmpnum;
			}
		}
		else if(strstr(buf,"dante") != 0)
		{
			if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
			{
				strcpy(info[tmpnum],"in.dante.1.audio.1");
				tmpnum++;
			}
			else
			{
				DBG_WarnMsg("This is not switcher!!!\n");
				return tmpnum;
			}
		}
		else
		{
			DBG_WarnMsg("No Audio\n");
			return tmpnum;
		}

		if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
		{
			for(int i = 0; i < 4; i++)
			{
				if(g_audio_info.dst_port[i] == PORT_HDMI)
				{
					if(hdmi_out_idx == 1)
						strcpy(info[tmpnum++],"out.hdmi.1.audio.1");
				}
				else if(g_audio_info.dst_port[i] == PORT_ANALOG_AUDIO)
				{
					if(g_audio_info.direction == DIRECTION_OUT)
					{
						strcpy(info[tmpnum++],"out.analog_audio.1.audio.1");
					}
				}
				else if(g_audio_info.dst_port[i] == PORT_DANTE)
				{
					strcpy(info[tmpnum++],"out.dante.1.audio.1");
				}
				else if(g_audio_info.dst_port[i] == PORT_STREAM)
				{
					strcpy(info[tmpnum++],"out.stream.1.audio.1");
				}
				else
				{
					continue;
				}
			}
		}
		else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
		{
			strcpy(info[tmpnum++],"out.stream.1.audio.1");

			if(hdmi_out_idx == 1)
				strcpy(info[tmpnum++],"out.hdmi.1.audio.1");
		}
		else
		{
			strcpy(info[tmpnum++],"out.stream.1.audio.1");

			if(hdmi_out_idx == 1)
				strcpy(info[tmpnum++],"out.hdmi.1.audio.1");

			if(g_audio_info.direction == DIRECTION_OUT)
				strcpy(info[tmpnum++],"out.analog_audio.1.audio.1");
		}

	}
#else
	int Audio = 0;
	mysystem("cat /sys/devices/platform/1500_i2s/output_audio_info", buf, 64);

	if(strstr(buf,"State: Off") != 0)
	{
		DBG_WarnMsg("Audio Not Available\n");
	}
	else
	{
		Audio = 1;
	}

	int port = g_autoswitch_info.source;

	if((strcmp(g_version_info.model,IPD_MODULE) == 0)&&(port == 1))//Local HDMI
	{
		mysystem("astparam g rx_local_input", buf, 64);

		if(strstr(buf,"not defined") != 0)
		{
			DBG_WarnMsg("tv_access not defined\n");
		}
		else
		{
			if(strstr(buf,"y") != 0)
			{
				strcpy(info[tmpnum++],"in.hdmi.1.video.1");

				if(Audio == 1)
				{
					strcpy(info[tmpnum++],"in.hdmi.1.audio.1");
				}
			}
		}
	}
	else //stream
	{
		mysystem("cat /sys/devices/platform/videoip/timing_info", buf, 64);

		if(strstr(buf,"Not Available") != 0)
		{
			DBG_WarnMsg("Not Available\n");
		}
		else
		{
			strcpy(info[tmpnum++],"in.stream.1.video.1");

			if(Audio == 1)
			{
				strcpy(info[tmpnum++],"in.stream.1.audio.1");
			}
		}
	}

	if(tmpnum >= 1)
	{
		if(strcmp(g_version_info.model,IPD_MODULE) == 0)
		{
			mysystem("astparam g tv_access", buf, 64);

			if(strstr(buf,"not defined") != 0)
			{
				DBG_WarnMsg("tv_access not defined\n");
			}
			else
			{
				if(strstr(buf,"y") != 0)
				{
					strcpy(info[tmpnum++],"out.hdmi.1.video.1");
					if(Audio == 1)
					{
						strcpy(info[tmpnum++],"out.hdmi.1.audio.1");
					}
				}

			}
		}
		else
		{
			mysystem("cat /sys/devices/platform/SiI9136/tv_access", buf, 64);

			if(strstr(buf,"1") != 0)
			{
				strcpy(info[tmpnum++],"out.hdmi.1.video.1");
				if(Audio == 1)
				{
					strcpy(info[tmpnum++],"out.hdmi.1.audio.1");
				}
			}
		}
	}

#endif

	return tmpnum;
}


void fun(char *str)
{
	char *str_c=str;
	int i,j=0;
	for(i=0;str[i]!='\0';i++)
	{
		if(str[i]!=' ')
			str_c[j++]=str[i];
	}
	str_c[j]='\0';
	str=str_c;
}

int getIndexOfSigns(char ch)
{
    if(ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if(ch >= 'A' && ch <='F')
    {
        return ch - 'A' + 10;
    }
    if(ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    return -1;
}

long hexToDec(char *source)
{
    long sum = 0;
    long t = 1;
    int i, len;

    len = strlen(source);
    for(i=len-1; i>=0; i--)
    {
        sum += t * getIndexOfSigns(*(source + i));
        t *= 16;
    }
    return sum;
}

void IPHextoDec(char * hexIp,char * DecIp)
{
	char ip1[2] = {0};
	char ip2[2] = {0};
	char ip3[2] = {0};
	char ip4[2] = {0};
	char ip[16] = {0};
	sscanf(hexIp,"%2s%2s%2s%2s",ip1,ip2,ip3,ip4);
	sprintf(ip,"%d.%d.%d.%d",hexToDec(ip4),hexToDec(ip3),hexToDec(ip2),hexToDec(ip1));
	memcpy(DecIp,ip,strlen(ip));
}

int EX_GetConnectionList(char info[][MAX_SIGNALE_LEN],int num)
{
    FILE *fp;
	int i = 0;
	char tcptmp[1024] = {0};
	char strNum[8] = {0};
	char strlocalIP[16] = {0};
	char strlocalPORT[8] = {0};
	char strIP[16] = {0};
	char strPORT[8] = {0};
	char strFLAG[4] = {0};
	char strOther[512] = {0};
	char tmp[16] = "";
	fp = fopen("/proc/net/tcp", "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open /proc/net/tcp\n");
		return -1;
	}
	do{
		memset(tcptmp,0,strlen(tcptmp));
		memset(strlocalIP,0,strlen(strlocalIP));
		memset(strlocalPORT,0,strlen(strlocalPORT));
		memset(strIP,0,strlen(strIP));
		memset(strPORT,0,strlen(strPORT));
		memset(strFLAG,0,strlen(strFLAG));
		fgets(tcptmp, 1023,fp);
		fun(tcptmp);
		char strsrc[64] = "";
		//trim_string_eol(info);
		sscanf(tcptmp,"%[0-9]:%8s:%4s%8s:%4s%2s%s",strNum,strlocalIP,strlocalPORT,strIP,strPORT,strFLAG,strOther);
		if(0 == strcasecmp(strFLAG,"01"))
		{
			int iLocalport = hexToDec(strlocalPORT);
			if(i >= 100)
				break; 
			if(iLocalport == 80 || iLocalport == 443 || iLocalport == g_network_info.tcp_port || iLocalport == g_gateway_info.rs232_port)
			{
				char strDecIP[16] = "";
				char strinfo[64] = "";
				IPHextoDec(strIP,strDecIP);
				sprintf(strinfo,"[(TCP:%d,%s:%d),ESTABLISHED]",iLocalport,strDecIP,hexToDec(strPORT));
				memcpy(info[i],strinfo,strlen(strinfo));
				i++;
				//printf("{{%s}}\n",strinfo);
			}
		}
	}while(strlen(tcptmp) > 1);
	fclose(fp);
	return i;
}

int EX_GetPortList(char info[][MAX_PORT_LEN],int num)
{
	int tmpnum = 0;
	if(strcmp(g_version_info.model,IPE_MODULE) == 0)
	{
		//hdmi & usb-c
		strcpy(info[0],"in.hdmi.1");
		strcpy(info[1],"out.hdmi.1");
		strcpy(info[2],"out.stream.1");
		//analog
		if(g_audio_info.direction == DIRECTION_OUT)
			strcpy(info[3],"out.analog_audio.1");
		else
			strcpy(info[3],"in.analog_audio.1");
		//RS-232
		strcpy(info[4],"both.rs232.1");
		//IR
		if(g_gateway_info.ir_direction == DIRECTION_OUT)
			strcpy(info[5],"out.ir.1");
		else
			strcpy(info[5],"in.ir.1");

		//usb-b
		strcpy(info[6],"both.usb_b.1");
		tmpnum = 7;
	}
	else if(strcmp(g_version_info.model,IPE_W_MODULE) == 0)
	{
		strcpy(info[0],"in.hdmi.1");
		strcpy(info[1],"in.usb_c.2");
		strcpy(info[2],"out.stream.1");
		strcpy(info[3],"in.analog_audio.1");
		//RS-232
		strcpy(info[4],"both.rs232.1");
		//IR
		if(g_gateway_info.ir_direction == DIRECTION_OUT)
			strcpy(info[5],"out.ir.1");
		else
			strcpy(info[5],"in.ir.1");

		tmpnum = 6;
	}
	else if(strcmp(g_version_info.model,IPE_P_MODULE) == 0)
	{
		strcpy(info[0],"in.hdmi.1");
		strcpy(info[1],"in.hdmi.2");
		strcpy(info[2],"in.usb_c.3");
		strcpy(info[3],"out.hdmi.1");
		strcpy(info[4],"out.stream.1");
		//dante
		strcpy(info[5],"both.dante.1");
		//analog
		if(g_audio_info.direction == DIRECTION_OUT)
			strcpy(info[6],"out.analog_audio.1");
		else
			strcpy(info[6],"in.analog_audio.1");
		//RS-232
		strcpy(info[7],"both.rs232.1");
		//IR
		if(g_gateway_info.ir_direction == DIRECTION_OUT)
			strcpy(info[8],"out.ir.1");
		else
			strcpy(info[8],"in.ir.1");

		//usb-b
		strcpy(info[9],"both.usb_a.1");
		strcpy(info[10],"both.usb_b.1");
		tmpnum = 11;
	}
	else if(strcmp(g_version_info.model,IPD_MODULE) == 0)
	{
		strcpy(info[0],"in.hdmi.1");
		strcpy(info[1],"in.stream.1");
		strcpy(info[2],"out.hdmi.1");

		//analog
		strcpy(info[3],"out.analog_audio.1");

		//RS-232
		strcpy(info[4],"both.rs232.1");

		//IR
		if(g_gateway_info.ir_direction == DIRECTION_OUT)
			strcpy(info[5],"out.ir.1");
		else
			strcpy(info[5],"in.ir.1");

		//usb-a
		strcpy(info[6],"both.usb_a.1");

		tmpnum = 7;
	}
	else if(strcmp(g_version_info.model,IPD_W_MODULE) == 0)
	{
		strcpy(info[0],"in.stream.1");
		strcpy(info[1],"out.hdmi.1");

		//analog
		strcpy(info[2],"out.analog_audio.1");

		//RS-232
		strcpy(info[3],"both.rs232.1");

		//IR
		if(g_gateway_info.ir_direction == DIRECTION_OUT)
			strcpy(info[4],"out.ir.1");
		else
			strcpy(info[4],"in.ir.1");

		tmpnum = 5;
	}

	return tmpnum;
}
int EX_GetActiveCliNUm(void)
{
	int num = 0;
	return num ;
}
int EX_SetLogEvent(int action,int period)
{
	DBG_InfoMsg("EX_SetLogEvent action = %d period= %d\n",action,period);

	char sCmd[128] = "";
	sprintf(sCmd,"e_log");

	if(action == 1)
		sprintf(sCmd,"e_log::start");
	else if(action == 2)
		sprintf(sCmd,"e_log::pause");
	else if(action == 3)
		sprintf(sCmd,"e_log::resume");
	else if(action == 4)
		sprintf(sCmd,"e_log::reset");
	else
	{
		DBG_ErrMsg("action: %d\n",action);
		return EX_PARAM_ERR;
	}

	if(period == 2)
		sprintf(sCmd,"%s::daily",sCmd);
	else if(period == 3)
		sprintf(sCmd,"%s::weekly",sCmd);
	else
	{
		DBG_ErrMsg("period: %d\n",period);
		return EX_PARAM_ERR;
	}

	DBG_InfoMsg("ast_send_event %s\n",sCmd);
	ast_send_event(0xFFFFFFFF,sCmd);

	Cfg_Set_Log_Action(action,period);
	return EX_NO_ERR;
}

int EX_GetLogEvent(int * action,int * period)
{
	DBG_InfoMsg("EX_SetLogEvent action = %d period= %d\n",action,period);
	Cfg_Get_Log_Action(action,period);

	return EX_NO_ERR;
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
	else if(strstr(cfgName,"auto_switch_delays") != 0)
	{
		Cfg_Set_Switch_Delay();
	}
	else if(strstr(cfgName,"display_sleep") != 0)
	{
		Cfg_Set_Display_Sleep();
	}

	return EX_NO_ERR;
}

void * Beacon_cb(void * fd)
{
    printf("Beacon_cb %d\n",BEACONThread);
    if(BEACONThread == 1){
            return 0;}
    BEACONThread == 1;
	//int time = *(int *)fd;
	//char  sock_opt = 1;
	unsigned  char  one = 1;
    struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr(g_network_info.beacon_ip);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    printf("multicast_ip[%s]\n",g_network_info.beacon_ip);
    setsockopt(g_Udp_Socket,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq));

	/*if((setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR, ( void  *) &sock_opt,  sizeof  (sock_opt)))== -1) {
     }
    if  ((setsockopt(sockfd, IPPROTO_IP,IP_MULTICAST_LOOP,&one,  sizeof  (unsigned  char ))) == -1){
    }*/

    char ip_buf[32] = "";
	GetIPInfo(1,ip_buf,NULL);

	struct sockaddr_in dst_addr;
	bzero(&dst_addr,sizeof(dst_addr));
	dst_addr.sin_family = AF_INET;
	dst_addr.sin_port = htons(g_network_info.beacon_port);
	//dst_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	dst_addr.sin_addr.s_addr = inet_addr(g_network_info.beacon_ip);

	char  cBeaconInfo[1024] = "";
    //~01@BEACON-INFO 1,192.168.60.2,50000,5000,11-22-33-44-55-66,KDS-EN6,IPE5000
    sprintf(cBeaconInfo,"~01@BEACON-INFO 1,%s,%d,%d,%s,%s,%s\r\n",ip_buf,g_network_info.udp_port,g_network_info.tcp_port,g_device_info.mac_addr,g_version_info.model,g_device_info.hostname);

	int ret = 0;
      printf("[%s]\n",cBeaconInfo);
	while(1){
		if(g_network_info.beacon_en == ON)
        {
		    ret = sendto(g_Udp_Socket,cBeaconInfo,strlen(cBeaconInfo),0,(struct sockaddr *)&dst_addr,sizeof(dst_addr));
		}
        else{
            break;
        }
		usleep(g_network_info.beacon_time * 1000000 - 260 * 1000);
	}
	return 0;
}

int EX_Beacon(int iPort_Id,int iStatus,int iTime)
{
    if(iPort_Id == 1)
    {

        if(iStatus == 1)
        {
			if(g_network_info.beacon_en  != ON)
            {
				g_network_info.beacon_en  = ON;

				pthread_attr_t	s_tThreadAttr;
				pthread_attr_init(&s_tThreadAttr);
				pthread_attr_setstacksize(&s_tThreadAttr, 512*1024);

	            pthread_create(&Beacon_id, &s_tThreadAttr, Beacon_cb, NULL);
	            pthread_detach(Beacon_id);
			}

        }
        else if(iStatus == 0)
        {
			if(g_network_info.beacon_en  != OFF)
            {
				g_network_info.beacon_en  = OFF;
			}
        }
		else
		{
			DBG_WarnMsg(" !!! Error para\n");
			return EX_PARAM_ERR;
		}
    }
    if(1 <= iTime && iTime <= 1800)
    {
        g_network_info.beacon_time = iTime;
    }
    else
    {
        //g_network_info.beacon_time = 10;
		DBG_WarnMsg(" !!! Error para\n");
        return EX_PARAM_ERR;
    }
    Cfg_Update(NETWORK_INFO);
	return EX_NO_ERR;
}
//g_network_info
int EX_Discovery(char *aflag,char*ip,int iPort)
{
    char ip_buf[32] = "";
	char mask_buf[32] = "";
	GetIPInfo(1,ip_buf,mask_buf);

	char gw_buf[32] = "";
	char* gw_cmd = "route -n | grep eth0 | grep UG | awk '{print $2}'";
	mysystem(gw_cmd, gw_buf, 32);

    char Send_buf[128] = "";
    //sprintf(Send_buf,"~01@NET-IP %s,%s,%s\r\n",ip_buf,mask_buf,gw_buf);

    if(!strcasecmp(aflag,"TCP"))
    {
        sprintf(Send_buf,"~01@ETH-PORT %s,%d\r\n",aflag,g_network_info.tcp_port);
    }
    else if(!strcasecmp(aflag,"UDP"))
    {
        sprintf(Send_buf,"~01@ETH-PORT %s,%d\r\n",aflag,g_network_info.udp_port);
    }
    else if(!memcmp(aflag,"CONFIG",strlen("CONFIG")))
    {
        sprintf(Send_buf,"~01@NET-CONFIG %s,%s,%s\r\n",ip_buf,mask_buf,gw_buf);
    }
    else
    {
        sprintf(Send_buf,"~01@NET-IP %s\r\n",ip_buf);
    }
	struct sockaddr_in addr;
	addr.sin_family =AF_INET;
	addr.sin_port = htons(iPort);// iPort
	inet_pton(AF_INET,ip,&addr.sin_addr.s_addr); //服务器的ip
	struct sockaddr_in server_addr;
	socklen_t len = sizeof(server_addr);

    int ret = sendto(g_Udp_Socket,Send_buf,strlen(Send_buf),0, (struct sockaddr*)&addr,sizeof(addr));
    printf("sendto ip:%s,port:%d [%d]{%s}\n",ip,iPort,ret,Send_buf);
	return EX_NO_ERR;
}

int EX_ConfBeaconInfo(char *muticastIP,int port)
{
    memcpy(g_network_info.beacon_ip,muticastIP,strlen(muticastIP));
    g_network_info.beacon_port = port;
    //Cfg_Set_Net_Port(Net_UDP,port);
    //Cfg_Set_Net_Multicast(muticastIP,0);
    Cfg_Update(NETWORK_INFO);
    if(g_network_info.beacon_en == ON)
    {
        BEACONThread = 0;
        //g_network_info.beacon_en = OFF;
        pthread_cancel(Beacon_id);

		pthread_attr_t	s_tThreadAttr;
		pthread_attr_init(&s_tThreadAttr);
		pthread_attr_setstacksize(&s_tThreadAttr, 512*1024);
        pthread_create(&Beacon_id, NULL, Beacon_cb, NULL);
        pthread_detach(Beacon_id);
    }
	else
	{
		DBG_WarnMsg(" !!! Error Mode \n");
		return EX_MODE_ERR;
	}

    return EX_NO_ERR;
}

int EX_GetBeaconConf(char *muticastIP,int *port)
{
    memcpy(muticastIP,g_network_info.beacon_ip,strlen(g_network_info.beacon_ip));
    *port = g_network_info.beacon_port ;
    return EX_NO_ERR;
}

int EX_GetBeacon(int *iPort_Id,int *iStatus,int *iTime)
{
    *iPort_Id = 1;
    if(g_network_info.beacon_en == ON)
    {
        *iStatus = 1;
    }
    else
    {
        *iStatus = 0;
    }
    *iTime = g_network_info.beacon_time;
	return EX_NO_ERR;
}


void GetUpgradeStatus(char* info, unsigned int size)
{
	FILE *fp;

	memset(info,0,size);

	fp = fopen("/www/fw_status.txt", "r");
	if (fp == NULL) {
		DBG_ErrMsg("ERROR! can't open /www/fw_status.txt\n");
		snprintf(info, size - 1, "err,0,2"); // No such file
		return;
	}
	fgets(info, size, fp);
	trim_string_eol(info);
	fclose(fp);

	DBG_InfoMsg("GetUpgradeStatus info:%s end\n",info);
}



int EX_NTFYPhraser(Notify_S *s_NTFYInfo,char *tmpparam)
{
    if(s_NTFYInfo->NCmd == NTFY_CEC_MSG)
    {
        sprintf(tmpparam,"1,%d,",s_NTFYInfo->iParamNum);
    }
    if(s_NTFYInfo->NCmd == NTFY_BUTTON)
    {
        if(buttonbool == 0){
            return -1;
        }
    }
    if(s_NTFYInfo->NCmd == NTFY_INPUT || s_NTFYInfo->NCmd == NTFY_OUTPUT)
    {
        int i = 0;
        char siglist[10][MAX_SIGNALE_LEN] = {0};
        int ret = EX_GetSignalList(&siglist[0],10);
        if(ret > 10)
            return -1;
        else
        {
            sprintf(tmpparam+strlen(tmpparam),"[");
            for(i = 0;i < ret;i++)
            {
                strncat(tmpparam,siglist[i],MAX_SIGNALE_LEN);
        		if(i <  (ret-1))
        		{
        			strncat(tmpparam,",",1);
        		}
            }
            sprintf(tmpparam+strlen(tmpparam),"]");
        }
    }
    if(s_NTFYInfo->NCmd == NTFY_CON_LIST)
    {
        char connectionlist[100][MAX_SIGNALE_LEN] = {0};
    	int i = 0;
    	int ret = EX_GetConnectionList(&connectionlist[0],10);
    	if(ret > 10)
    	{
    		DBG_WarnMsg("P3K_GetConnectionList num=%d over 10\n",ret);
    		ret =10;
    	}
    	for(i = 0;i < ret ;i++)
    	{
    		printf(" ret = %d ,sig=%s\n",ret,connectionlist[i]);
    		strncat(tmpparam,connectionlist[i],MAX_SIGNALE_LEN);
    		if(i <  (ret-1))
    		{
    			strncat(tmpparam,",",1);

    		}
    	}
    }
    printf("{s_NTFYInfo [%d][%d]}\n",s_NTFYInfo->NCmd,s_NTFYInfo->iParamNum);
    return EX_NO_ERR;
}

int EX_TESTMODE()
{
    buttonbool = 1;
    return EX_NO_ERR;
}

const char* strHelpCmd[] = {
	"#",
	"BEACON-EN",
	"BEACON-INFO?",
	"BUILD-DATE?",
	"CEC-GW-PORT-ACTIVE",
	"CEC-NTFY",
	"CEC-SND",
	"COM-ROUTE?",
	"COM-ROUTE-ADD",
	"COM-ROUTE-REMOVE",
#ifdef CONFIG_P3K_CLIENT
	"CS-CONVERT",
	"CS-CONVERT?",
#else
	"EDID-ACTIVE",
	"EDID-ACTIVE?",
	"EDID-LIST?",
	"EDID-MODE",
	"EDID-MODE?",
	"EDID-NET-SRC?",
	"EDID-NET-SRC",
	"EDID-RM",
#endif
	"ETH-PORT",
	"ETH-PORT?",
	"FACTORY",
	"FCT-MAC",
	"FCT-MODEL",
	"FCT-SN",
#ifdef 	CONFIG_P3K_HOST
	"HDCP-MOD",
	"HDCP-MOD?",
#endif
	"HDCP-STAT?",
	"HELP"
	"HW-TEMP?",
	"HW-VERSION?",
	"IDV",
	"IR-SND",
	"KDS-ACTION",
	"KDS-ACTION?",
	"KDS-AUD",
	"KDS-AUD?",
#ifdef CONFIG_P3K_CLIENT
	"KDS-CHANNEL-SELECT",
	"KDS-CHANNEL-SELECT?",
	"KDS-DAISY-CHAIN",
	"KDS-DAISY-CHAIN?",
#else
	"KDS-DEFINE-CHANNEL",
	"KDS-DEFINE-CHANNEL?",
	"KDS-DEFINE-CHANNEL-NAME",
	"KDS-DEFINE-CHANNEL-NAME?",
#endif
	"KDS-GW-ETH",
	"KDS-GW-ETH?",
	"KDS-IR-GW",
	"KDS-IR-GW?",
	"KDS-METHOD",
	"KDS-METHOD?",
	"KDS-MULTICAST",
#ifdef CONFIG_P3K_CLIENT
	"KDS-OSD-DISPLAY",
	"KDS-OSD-DISPLAY?",
#endif
	"KDS-RATIO?",
	"KDS-RESOL?",
#ifdef CONFIG_P3K_CLIENT
	"KDS-SCALE",
	"KDS-SCALE?",
#endif
	"KDS-VLAN-TAG",
	"KDS-VLAN-TAG?",
	"LDFW",
#ifdef 	CONFIG_P3K_HOST
	"LOCK-EDID",
	"LOCK-EDID?",
#endif
	"LOCK-FP",
	"LOCK-FP?",
	"LOG-ACTION",
	"LOGIN",
	"LOGIN?",
	"LOGOUT",
	"MODEL?",
	"NAME",
	"NAME?",
	"NAME-RST",
	"NET-CONFIG",
	"NET-CONFIG?",
	"NET-DHCP",
	"NET-DHCP?",
	"NET-MAC?",
	"NET-STAT?",
	"NET-IP?",
	"PASS",
	"PASS?",
	"PORT-DIRECTION",
	"PORT-DIRECTION?",
	"PORT-LIST?",
	"RESET",
	"ROLLBACK",
	"SECUR",
	"SECUR?",
	"SIGNALS-LIST?",
	"SN?",
	"STANDBY-VERSION?",
	"TIME",
	"TIME?",
	"TIME-LOC",
	"TIME-LOC?",
	"TIME-SRV",
	"TIME-SRV?",
	"UART",
	"UART?",
	"UPG-TIME?",
	"UPGRADE",
	"VERSION?",
#ifdef CONFIG_P3K_CLIENT
	"VIDEO-WALL-SETUP",
	"VIDEO-WALL-SETUP?",
	"VIEW-MOD",
	"VIEW-MOD?",
	"WND-STRETCH",
	"WND-STRETCH?",
#endif
	"X-AUD-DESC?",
	"X-AUD-LVL",
	"X-AUD-LVL?",
#ifdef CONFIG_P3K_HOST
	"X-AV-SW-MODE",
	"X-AV-SW-MODE?",
	"X-PRIORITY",
	"X-PRIORITY?",
#endif
	"X-ROUTE",
	"X-ROUTE?",
 };

 int EX_HelpCmd(char* sHelpString)
 {
 	sprintf(sHelpString,"%s",strHelpCmd[0]);

   	for(unsigned int i = 1; i < sizeof(strHelpCmd)/sizeof(strHelpCmd[0]); ++i)
	{
	    sprintf(sHelpString,"%s, %s",sHelpString,strHelpCmd[i]);
	}

	DBG_InfoMsg("info:%s \n",sHelpString);
 	return EX_NO_ERR;
 }


