#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "debugtool.h"
#include "p3kswitch.h"
#include "common.h"
#include "funcexcute.h"
/*******************************************************************
���ļ����������þ�̬������Ϊ�����洢


******************************************************************/
#define MAX_PARAM_COUNT 10
typedef struct _P3K_PhraserToExecute_S
{
	char	 *cmd; //P3Kָ��
	int (*ParamPhraser)(char*reqparam,char*respParam,char*userdata); //p3k ������������
}P3K_PhraserToExecute_S;

#define PARAM_SEPARATOR ','

int P3K_OtherChanges(char * info)
{
	int ret = 0;
	//test_printf();
	ret = EX_AutomaticReporting(info);
	return ret;
}


static int P3K_PhraserWithSeparator(char separator,char * param, int len, char str[][MAX_PARAM_LEN] )
{
	int tmpLen = 0 ;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;


	if(param == NULL ||len <=0)
	{
		return -1;
	}
	while(tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata,separator);
		if(tmpdata != NULL)
		{

			tmpLen = tmpdata-tmpdata1;
			//memset(str[i],0,MAX_PARAM_LEN);
			memcpy(str[i],tmpdata1,tmpLen);
			i++;
			if(len > tmpdata-param+1)
			{
				tmpdata1 = tmpdata+1;
				tmpdata = tmpdata +1;
		       }
			else
			{
				break;
			}
		}

	}
	//memset(str[i],0,MAX_PARAM_LEN);
	memcpy(str[i],tmpdata1,strlen(tmpdata1));
	i++;

	//memcpy();
	return i;
}

static int P3K_CheckEdidMode(char*data)
{
	int tmpDirec = PASSTHRU;
	//char *tmp = data;


	if((!strcmp(data,"PASSTHRU"))||(!strcmp(data,"passthru")))
		{tmpDirec = PASSTHRU;}
	else if((!strcmp(data,"CUSTOM"))||(!strcmp(data,"custom")))
		{tmpDirec = CUSTOM;}
	else if((!strcmp(data,"DEFAULT"))||(!strcmp(data,"default")))
		{tmpDirec = DEFAULT;}
	return tmpDirec;
}

static int P3K_ModeToStr(int type,char*data)
{
	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(type)
		{
			case PASSTHRU:
				strcpy(tmpbuf,"PASSTHRU");
				break;
			case CUSTOM:
				strcpy(tmpbuf,"CUSTOM");
				break;
			case DEFAULT:
				strcpy(tmpbuf,"DEFAULT");
				break;
			default:
				strcpy(tmpbuf,"DEFAULT");
				break;
		}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;
}


static int P3K_CheckPortDirection(char*data)
{
	int tmpDirec = DIRECTION_IN;
	//char *tmp = data;


	if(!strcmp(data,"in") || !strcmp(data,"[in"))
		{tmpDirec = DIRECTION_IN;}
	else	if(!strcmp(data,"out") || !strcmp(data,"[out"))
		{tmpDirec = DIRECTION_OUT;}
	else	if(!strcmp(data,"both") || !strcmp(data,"[both"))
		{tmpDirec = DIRECTION_BOTH;}
	return tmpDirec;
}
static int P3K_PortDirectionToStr(int direc,char*data)
{

	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(direc)
	{
		case DIRECTION_IN:
			strcpy(tmpbuf,"in");
			break;
		case DIRECTION_OUT:
			strcpy(tmpbuf,"out");
			break;
		case DIRECTION_BOTH:
			strcpy(tmpbuf,"both");
			break;
		default:
			strcpy(tmpbuf,"in");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;

}

static int P3K_CheckPortFormat(char*data)
{
	int tmpFormat = PORT_HDMI;
	//char *tmp = data;

	if(!strcmp(data,"hdmi"))
		{tmpFormat = PORT_HDMI;}

	else if(!strcmp(data,"analog_audio"))
		{tmpFormat = PORT_ANALOG_AUDIO;}

	else if(!strcmp(data,"analog"))
		{tmpFormat = PORT_ANALOG_AUDIO;}

	else if(!strcmp(data,"rs232"))
		{tmpFormat = PORT_RS232;}

	else if(!strcmp(data,"ir"))
		{tmpFormat = PORT_IR;}

	else if(!strcmp(data,"usb_a"))
		{tmpFormat = PORT_USB_A;}

	else if(!strcmp(data,"usb_b"))
		{tmpFormat = PORT_USB_B;}

	else if(!strcmp(data,"usb_c"))
		{tmpFormat = PORT_USB_C;}
	else if(!strcmp(data,"dante"))
		{tmpFormat = PORT_DANTE;}
	else if(!strcmp(data,"hdbt"))
		{tmpFormat = PORT_HDBT;}
	else if(!strcmp(data,"amplified_audio"))
		{tmpFormat = PORT_AMPLIFIED_AUDIO;}
	else if(!strcmp(data,"tos"))
		{tmpFormat = PORT_TOS;}
	else if(!strcmp(data,"spdif"))
		{tmpFormat = PORT_SPDIF;}
	else if(!strcmp(data,"mic"))
		{tmpFormat = PORT_MIC;}
	else if(!strcmp(data,"stream"))
		{tmpFormat = PORT_STREAM;}
	return tmpFormat;
}
static int P3K_PortFormatToStr(int format,char*data)
{
	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(format)
	{
		case PORT_HDMI:
			strcpy(tmpbuf,"hdmi");
			break;
		case PORT_USB_A:
			strcpy(tmpbuf,"usb_a");
			break;
		case PORT_USB_B:
			strcpy(tmpbuf,"usb_b");
			break;
		case PORT_USB_C:
			strcpy(tmpbuf,"usb_c");
			break;
		case PORT_ANALOG_AUDIO:
			strcpy(tmpbuf,"analog_audio");
			break;
		case PORT_IR:
			strcpy(tmpbuf,"ir");
			break;
		case PORT_RS232:
			strcpy(tmpbuf,"rs232");
			break;
		case PORT_DANTE:
			strcpy(tmpbuf,"dante");
			break;
		case PORT_HDBT:
			strcpy(tmpbuf,"hdbi");
			break;
		case PORT_AMPLIFIED_AUDIO:
			strcpy(tmpbuf,"amplified_audio");
			break;
		case PORT_TOS:
			strcpy(tmpbuf,"tos");
			break;
		case PORT_SPDIF:
			strcpy(tmpbuf,"spdif");
			break;
		case PORT_MIC:
			strcpy(tmpbuf,"mic");
			break;
		case PORT_STREAM:
			strcpy(tmpbuf,"stream");
			break;
		default:
			strcpy(tmpbuf,"analog_audio");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;

}

static int P3K_StateToStr(int direc,char*data)
{

	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(direc)
	{
		case ON:
			strcpy(tmpbuf,"on");
			break;
		case OFF:
			strcpy(tmpbuf,"off");
			break;
		default:
			strcpy(tmpbuf,"off");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;

}


static int P3K_CheckSignalType(char*data)
{
	int tmpFormat = SIGNAL_VIDEO;
	//char *tmp = data;
	if(!strcmp(data,"audio"))
	{	tmpFormat = SIGNAL_AUDIO;}
	else if(!strcmp(data,"video"))
	{	tmpFormat = SIGNAL_VIDEO;}
	else if(!strcmp(data,"ir"))
	{	tmpFormat = SIGNAL_IR;}
	else if(!strcmp(data,"usb"))
	{	tmpFormat = SIGNAL_USB;}
	else if(!strcmp(data,"arc"))
	{	tmpFormat = SIGNAL_ARC;}
	else if(!strcmp(data,"rs232"))
	{	tmpFormat = SIGNAL_RS232;}
	else if(!strcmp(data,"av_test_pattern"))
	{   tmpFormat = SIGNAL_TEST;}
	return tmpFormat;
}

static int P3K_CheckStateType(char*data)
{
	int tmpFormat = OFF;
	//char *tmp = data;
	if(!strcmp(data,"off"))
	{	tmpFormat = OFF;}
	else if(!strcmp(data,"on"))
	{	tmpFormat = ON;}


	return tmpFormat;
}


static int P3K_SignaleTypeToStr(int signaltype,char*data)
{

	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(signaltype)
	{
		case SIGNAL_VIDEO:
			strcpy(tmpbuf,"video");
			break;
		case SIGNAL_AUDIO:
			strcpy(tmpbuf,"audio");
			break;
		case SIGNAL_IR:
			strcpy(tmpbuf,"ir");
			break;
		case SIGNAL_ARC:
			strcpy(tmpbuf,"arc");
			break;
		case SIGNAL_USB:
			strcpy(tmpbuf,"usb");
			break;
		case SIGNAL_RS232:
			strcpy(tmpbuf,"rs232");
			break;
		default:
			strcpy(tmpbuf,"video");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;

}
static int P3K_CheckAudioSampleRate(char*data)
{
	int tmpRate = SAMPLE_RATE_44100;
	char *tmp = data;
		if(!strcmp(tmp,"44.1K"))
			{tmpRate = SAMPLE_RATE_44100;}
		else if(!strcmp(tmp,"48K"))
			{tmpRate = SAMPLE_RATE_48000;}
		else if(!strcmp(tmp,"32K"))
			{tmpRate = SAMPLE_RATE_32000;}
		else if(!strcmp(tmp,"22.5K"))
			{tmpRate = SAMPLE_RATE_22500;}
		else if(!strcmp(tmp,"16K"))
			{tmpRate = SAMPLE_RATE_16000;}
		else if(!strcmp(tmp,"8K"))
			{tmpRate = SAMPLE_RATE_8000;}
	return tmpRate;

}
static int P3K_AudioSampleRateToStr(int samplerate,char*data)
{
	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(samplerate)
	{
		case SAMPLE_RATE_44100:
			strcpy(tmpbuf,"44.1K");
			break;
		case SAMPLE_RATE_48000:
			strcpy(tmpbuf,"48K");
			break;
		case SAMPLE_RATE_32000:
			strcpy(tmpbuf,"32K");
			break;
		case SAMPLE_RATE_22500:
			strcpy(tmpbuf,"22.5K");
			break;
		case SAMPLE_RATE_16000:
			strcpy(tmpbuf,"16K");
			break;
		case SAMPLE_RATE_8000:
			strcpy(tmpbuf,"8K");
			break;
		default:
			strcpy(tmpbuf,"N/A");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;

}
static int P3K_CheckAudioType(char*data)
{
	int tmpType= AUDIO_FORMAT_PCM;
	//char *tmp = data;
	if(!strcmp(data,"PCM"))
	{tmpType = AUDIO_FORMAT_PCM;}

	return tmpType;

}
static int P3K_AudioTypeToStr(int type,char*data)
{
	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(type)
	{
		case AUDIO_FORMAT_PCM:
			strcpy(tmpbuf,"PCM");
			break;
		default:
			strcpy(tmpbuf,"PCM");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;
}


static int P3K_CheckUartParity(char*data)
{
	int tmpType= AUDIO_FORMAT_PCM;
	//char *tmp = data;
	if(!strcmp(data,"none"))
	{tmpType = PARITY_NONE;}
	else if(!strcmp(data,"odd"))
	{tmpType = PARITY_ODD;}
	else if(!strcmp(data,"even"))
	{tmpType = PARITY_EVEN;}
	else if(!strcmp(data,"mark"))
	{tmpType = PARITY_MARK;}
	else if(!strcmp(data,"space"))
	{tmpType = PARITY_SPACE;}

	return tmpType;

}
static int P3K_ParityToStr(int type,char *data)
{
	char tmpbuf[32]={0};
	memset(tmpbuf,0,sizeof(tmpbuf));
	switch(type)
	{
		case PARITY_NONE:
			strcpy(tmpbuf,"none");
			break;
		case PARITY_ODD:
			strcpy(tmpbuf,"odd");
			break;
		case PARITY_EVEN:
			strcpy(tmpbuf,"even");
			break;
		case PARITY_MARK:
			strcpy(tmpbuf,"mark");
			break;
		case PARITY_SPACE:
			strcpy(tmpbuf,"space");
			break;
		default:
			strcpy(tmpbuf,"none");
			break;
	}
	memcpy(data,tmpbuf,strlen(tmpbuf));
	return 0;
}
static int  P3K_PhraserParam(char *param,int len,	char str[][MAX_PARAM_LEN] )
{
	int tmpLen = 0 ;
	//int s32Ret = 0;
	int i = 0;
	char *tmpdata = param;
	char *tmpdata1 = param;


	if(param == NULL ||len <=0)
	{
		return -1;
	}
	while(tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata,PARAM_SEPARATOR);
		if(tmpdata != NULL)
		{

			tmpLen = tmpdata-tmpdata1;
			//memset(str[i],0,MAX_PARAM_LEN);
			memcpy(str[i],tmpdata1,tmpLen);

			i++;
			if(len > tmpdata-param+1)
			{
				tmpdata1 = tmpdata+1;
				tmpdata = tmpdata +1;
		       }
			else
			{
				break;
			}
		}

	}
	//memset(str[i],0,MAX_PARAM_LEN);
	memcpy(str[i],tmpdata1,strlen(tmpdata1));

	i++;

	//memcpy();
	return i;

}
static int P3K_GetPortInfo(char*param,PortInfo_S*info)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	//char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserWithSeparator('.',param, strlen(param), str);


	info->direction  = P3K_CheckPortDirection(str[0]);
	info->portFormat = P3K_CheckPortFormat(str[1]);
	info->portIndex = atoi(str[2]);
	if(count > 3)
	{
		info->signal = P3K_CheckSignalType(str[3]);
		info->index = atoi(str[4]);
	}
	return 0;
}
static int P3K_GetPortSInfo(char*param,PortInfo_S*info,int num)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	//char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserWithSeparator('.',param, strlen(param), str);


	info[num].direction  = P3K_CheckPortDirection(str[0]);
	info[num].portFormat = P3K_CheckPortFormat(str[1]);
	info[num].portIndex = atoi(str[2]);
	if(count > 3)
	{
		info[num].signal = P3K_CheckSignalType(str[3]);
		info[num].index = atoi(str[4]);
	}
	return 0;
}

static int P3K_SetAudioInputMode(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("SetAudioInputMode\n");
	//�����ײ���Ҫ����
	int mode;
	int count = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	char tmpparam[MAX_PARAM_LEN] = {0};
	//sscanf(param,"%1d",mode);
	mode = atoi(str[0]);
	//��������
	s32Ret = EX_SetAudSrcMode(mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetAudSrcMode err\n");
	}

	//�ظ������cmd ��cmd������
	sprintf(tmpparam,"%d",mode);
	memcpy(respParam,tmpparam,MAX_PARAM_LEN);
	return  0;
}
static int P3K_GetAudioInputMode(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("GetAudioInputMode\n");

	int mode = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret = EX_GetAudSrcMode(&mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetAudSrcMode err\n");
	}
	sprintf(tmpparam,"%d",mode);
	memcpy(respParam,tmpparam,MAX_PARAM_LEN);
    return  0;
}
static int P3K_SetAudLevel(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_SetAudLevel\n");
	//�����ײ���Ҫ����
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	
	//char tmpparam[MAX_PARAM_LEN] = {0};
	int gain = 0;
	PortInfo_S  tmpInfo = {0};
	int s32Ret = 0;
	//in.analog_audio.1.audio.1,10
	memset(&tmpInfo,0,sizeof(PortInfo_S));
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	gain = atoi(str[1]);
	P3K_GetPortInfo(str[0],&tmpInfo);
	s32Ret =  EX_SetAudGainLevel(&tmpInfo,gain);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetAudGainLevel err\n");
	}
	//�ظ������cmd ��cmd������
	memcpy(respParam,reqparam,strlen(reqparam));
	return  0;

}

static int P3K_GetAudLevel(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetAudLevel\n");
	//int count = 0;
	//char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int gain = 0;
	// in.analog_audio.1.audio.1
	P3K_GetPortInfo(reqparam,&tmpInfo);

	s32Ret = EX_GetAudGainLevel(&tmpInfo,&gain);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetAudGainLevel err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,gain);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetAudParam(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetAudParam\n");
	//int count = 0;
	int s32Ret = 0;
	//char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	PortInfo_S tmpInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	AudioSignalInfo_S audioInfo = {0};
	char sampleRate[32]={0};
	char audioType[32] ={0};
	char signal[32] ={0};
	char direc[32] = {0};
	char format[32] = {0};
	//  out.hdmi.1
	tmpInfo.signal = SIGNAL_AUDIO;
	tmpInfo.index = 1;
	P3K_GetPortInfo(reqparam,&tmpInfo);

	s32Ret = EX_GetAudParam(&tmpInfo,&audioInfo);
	 if(s32Ret)
	{
		DBG_ErrMsg("EX_GetAudParam err\n");
	}
	 P3K_AudioSampleRateToStr(audioInfo.sampleRate,sampleRate);

	 P3K_PortDirectionToStr(tmpInfo.direction, direc);
	 P3K_PortFormatToStr(tmpInfo.portFormat,format);
	 //P3K_AudioTypeToStr(audioInfo.format,audioType);

	 P3K_SignaleTypeToStr(tmpInfo.signal, signal);
	 if(audioInfo.chn > 0)
	 {
	 	sprintf(tmpparam,"%s.%s.%d.%s.%d,%d,%s,%s",direc,format,
		   tmpInfo.index,signal,tmpInfo.index,audioInfo.chn,sampleRate,audioInfo.format);
	 }
	 else
	 {
	 	sprintf(tmpparam,"%s.%s.%d.%s.%d,N/A,N/A,N/A",direc,format,
		   tmpInfo.index,signal,tmpInfo.index);
	 }

	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}

static int P3K_SetAutoSwitchMode(char*reqparam,char*respParam,char*userdata)
{
	//#X-AV-SW-MODE <direction_type>. <port_format>. <port_index>. <signal_type>. <index>,connection_mode<CR>
	//~nn@X-AV-SW-MODE <direction_type>. <port_format>. <port_index>. <signal_type>. <index>,connection_mode<CR><LF>
	DBG_InfoMsg("P3K_SetAutoSwitchMode\n");
	 //out.hdmi.1.video.1,2
	int mode;
	int count = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	PortInfo_S tmpInfo = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	//char tmpparam[MAX_PARAM_LEN] = {0};
	//sscanf(param,"%1d",mode);
	mode = atoi(str[1]);
	P3K_GetPortInfo(str[0],&tmpInfo);

	s32Ret = EX_SetAutoSwitchMode(&tmpInfo, mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetAutoSwitchMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));

	return 0;
}
static int P3K_GetAutoSwitchMode(char*reqparam,char*respParam,char*userdata)
{


	DBG_InfoMsg("P3K_GetAutoSwitchMode\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int mode = 0;
	// in.analog_audio.1.audio.1
	P3K_GetPortInfo(reqparam,&tmpInfo);

	s32Ret = EX_GetAutoSwitchMode(&tmpInfo,&mode);
	if(s32Ret)
	{
		DBG_ErrMsg("P3K_GetAutoSwitchMode err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_GetEdid(char*reqparam,char*respParam,char*userdata)
{

	//#GEDID io_mode,in_index<CR>
	//~nn@GEDID io_mode,in_index,size<CR><LF>
	DBG_InfoMsg("P3K_GetEdid\n");
		int mode = 0;
	int index = 0;
	int size = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);
	index = atoi(str[1]);

	s32Ret = EX_GetEDIDSupport(index, mode, &size);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetEDIDSupport err\n");
	}
	sprintf(respParam,"%s,%d",reqparam,size);
	return 0;
}
static int P3K_CopyEdid(char*reqparam,char*respParam,char*userdata)
{
	//#CPEDID edid_io,src_id,edid_io,dest_bitmap<CR>
	//#CPEDID edid_io,src_id,edid_io,dest_bitmap,safe_mode<CR>
	//~nn@CPEDID edid_io,src_id,edid_io,dest_bitmap<CR><LF>
	//~nn@CPEDID edid_io,src_id,edid_io,dest_bitmap,safe_mode<CR><LF>
	DBG_InfoMsg("P3K_CopyEdid\n");
	EDIDPortInfo_S src;
	EDIDPortInfo_S dest;
	int s32Ret = 0;
	int destBitMap = 0;
	int safeMode = -1;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	src.type = atoi(str[0]);
	src.id = atoi(str[1]);
	dest.type = atoi(str[2]);
	dest.type = atoi(str[3]);
	#if 0
	sscanf(str[4]);
	destBitMap = atoi(str[4]);
	if(count >5)
	{
		safeMode = atoi(str[5]);
	}
	#endif
	s32Ret = EX_CopyEDID(&src,&dest,destBitMap,safeMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_CopyEDID err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}


static int P3K_SetEdidCsMode(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR>
	//~nn@EDID CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_SetEdidCsMode\n");
	int mode;
	int count = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	PortInfo_S tmpInfo = {0};
	// in.hdmi.3.video.1,0
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	char tmpparam[MAX_PARAM_LEN] = {0};
	//sscanf(param,"%1d",mode);
	mode = atoi(str[1]);
	P3K_GetPortInfo(str[0],&tmpInfo);

	s32Ret = EX_SetEDIDColorSpaceMode(&tmpInfo, mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetEDIDColorSpaceMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetEdidCsMode(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-CS? <direction_type>.<port_format>.<port_index>. <signal_type>. <index><CR>
	//~nn@EDID-CS <direction_type>.<port_format>.<port_index>. <signal_type>. <index>,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_GetEdidCsMode\n");
	int count = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	int mode = 0;
	// in.hdmi.3.video.1
	P3K_GetPortInfo(reqparam,&tmpInfo);
	s32Ret = EX_GetEDIDColorSpaceMode(&tmpInfo,&mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetEDIDColorSpaceMode err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetEdidLockMode(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_SetEdidLockMode\n");
	int in_index = 0;
	int lockFlag = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	//#LOCK-EDID 2,1<CR>
	//~nn@LOCK-EDID 2,1<CR><LF>
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	in_index = atoi(str[0]);
	lockFlag = atoi(str[1]);

	s32Ret = EX_SetEDIDLockStatus(in_index, lockFlag);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetEDIDLockStatus err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetEdidLockMode(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetEdidLockMode\n");
	int in_index = 0;
	int lockFlag = 0;
	int s32Ret =0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	//#LOCK-EDID? 2<CR>
	//~nn@LOCK-EDID? 2,1<CR><LF>
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	in_index = atoi(str[0]);
	s32Ret = EX_GetEDIDLockStatus(in_index,&lockFlag);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetEDIDLockStatus err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,lockFlag);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetHDCPMode(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_SetHDCPMode\n");
	int in_index = 0;
	int mode = 0;
	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	in_index = atoi(str[0]);
	mode = atoi(str[1]);

	///#HDCP-MOD in_index,mode<CR>
	///~nn@HDCP-MOD in_index,mode<CR><LF>
	s32Ret = EX_SetHDCPMode(in_index, mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetHDCPMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));

	return 0;
}
static int P3K_GetHDCPMode(char*reqparam,char*respParam,char*userdata)
{
	//#HDCP-MOD? in_index<CR>
	//~nn@HDCP-MOD in_index,mode<CR><LF>
	DBG_InfoMsg("P3K_GetHDCPMode\n");
	int s32Ret = 0;
	int in_index = 0;
	int mode = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	in_index = atoi(str[0]);
	s32Ret = EX_GetHDCPMode(in_index, &mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetHDCPMode err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}

static int P3K_GetHDCPStatus(char*reqparam,char*respParam,char*userdata)
{

	//#HDCP STAT? io_mode,in_index<CR>
	//~nn@HDCP STAT io_mode,in_index,status<CR><LF>
	DBG_InfoMsg("P3K_GetHDCPStatus\n");
	int index = 0;
	int status = 0;
	int mode = 0;

	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);
	index = atoi(str[1]);

	status = EX_GetHDCPStatus(mode, index);

	sprintf(tmpparam,"%s,%d",reqparam,status);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_SetVideoWallMode(char*reqparam,char*respParam,char*userdata)
{
	//#VIEW-MOD mode,horizontal, vertical<CR>
	//~nn@VIEW-MOD mode, horizontal, vertical<CR><LF>

	DBG_InfoMsg("P3K_SetVideoWallMode\n");

	int s32Ret = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int count = 0;
	int mode = 0;
	ViewModeInfo_S  info;
	char tmpparam[MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);
	if(mode == 15)
	{
		info.hStyle = atoi(str[1]);
		info.vStyle = atoi(str[2]);
	}

	s32Ret =  EX_SetViewMode(mode, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetViewMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetVideoWallMode(char*reqparam,char*respParam,char*userdata)
{
	//#VIEW-MOD? <CR>
	//~nn@VIEW-MOD mode, horizontal, vertical<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallMode\n");
	int mode = 0;
	int s32Ret =0;
	ViewModeInfo_S info;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret = EX_GetViewMode(&mode, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetViewMode err\n");
	}
       sprintf(tmpparam,"%d,%d,%d",mode,info.hStyle,info.vStyle);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetWndBezel(char*reqparam,char*respParam,char*userdata)
{
	//#WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR>
	//~nn@WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR><LF>
	DBG_InfoMsg("P3K_SetWndBezel\n");
	int mode = 0;
	int s32Ret = 0;
	int out_index = 0;
	WndBezelinfo_S info ;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);
	out_index = atoi(str[1]);
	info.hValue = atoi(str[2]);
	info.vValue = atoi(str[3]);
	info.hOffset = atoi(str[4]);
	info.vOffset = atoi(str[5]);

	s32Ret = EX_SetWndBezelInfo(mode, out_index, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetWndBezelInfo err\n");
	}

	memcpy(respParam,reqparam,strlen(reqparam));
	return  0;
}
static int P3K_GetWndBezel(char*reqparam,char*respParam,char*userdata)
{
	//#WND-BEZEL? <CR>
	//~nn@WND-BEZEL mode,out_index,h_value,v_value,h_offset, v_offset<CR><LF>
	DBG_InfoMsg("P3K_GetWndBezel\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int mode = 0;
	int out_index = 0;
	WndBezelinfo_S info ;

	s32Ret = EX_GetWndBezelInfo(&mode,&out_index, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetWndBezelInfo err\n");
	}
	sprintf(tmpparam,"%d,%d,%d,%d,%d,%d",mode,out_index,info.hValue,info.vValue,info.hOffset,info.vOffset);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return  0;
}
static int P3K_SetVideoWallRotaion(char*reqparam,char*respParam,char*userdata)
{
	//#VIDEO-WALL-SETUP out_id,rotation<CR>
	//~nn@VIDEO-WALL-SETUP out_id,rotation<CR><LF>
	DBG_InfoMsg("P3K_SetVideoWallRotaion\n");
	int wallId;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	VideoWallSetupInfo_S info;
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	wallId = atoi(str[0]);
	info.rotation= atoi(str[1]);
	s32Ret = EX_SetVideoWallSetupInfo(wallId, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVideoWallSetupInfo err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));

	return  0;
}
static int P3K_GetVideoWallRotaion(char*reqparam,char*respParam,char*userdata)
{
	//#VIDEO-WALL-SETUP? <CR>
	//~nn@VIDEO-WALL-SETUP out_id,rotation<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallRotaion\n");
	int wallId;
	int s32Ret = 0;
	VideoWallSetupInfo_S info;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoWallSetupInfo(&wallId, &info);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoWallSetupInfo err\n");
	}
	sprintf(tmpparam,"%d,%d",wallId,info.rotation);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return  0;
}
static int P3K_StartOverlay(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-START-OVERLAY profile_name,time_limit<CR>
	//~nn@KDS-START-OVERLAY profile_name,time_limit<CR><LF>
	DBG_InfoMsg("P3K_StartOverlay\n");
	int timeOver = 0;
	int ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);

	timeOver = atoi(str[1]);
	ret = EX_StartOverlay(str[0], timeOver);
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_StopOverlay(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-STOP-OVERLAY <CR>
	//~nn@KDS-STOP-OVERLAY <CR><LF>
	DBG_InfoMsg("P3K_StopOverlay\n");
	int s32Ret = 0;
	s32Ret = EX_StopOverlay();
	if(s32Ret)
	{
		DBG_ErrMsg("EX_StopOverlay err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_SetChannelId(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-DEFINE-CHANNEL ch_id<CR>
	//~nn@KDS-DEFINE-CHANNEL ch_id<CR><LF>
	DBG_InfoMsg("P3K_SetChannelId\n");
	int s32Ret = 0;
	int chn = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	chn = atoi(str[0]);

	s32Ret = EX_SetEncoderAVChannelId(chn);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetEncoderAVChannelId err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetChannelId(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-DEFINE-CHANNEL? <CR>
	//~nn@KDS-DEFINE-CHANNEL ch_id<CR><LF>
	DBG_InfoMsg("P3K_GetChannelId\n");
	int chn = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	s32Ret =  EX_GetEncoderAVChannelId(&chn);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetEncoderAVChannelId err\n");
	}
	sprintf(tmpparam,"%d",chn);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetChannleSelection(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-CHANNEL-SELECT  signal_type,ch_id<CR>
	//~nn@KDS-CHANNEL-SELECT  signal_type,ch_id<CR><LF>
	DBG_InfoMsg("P3K_SetChannleSelection\n");
	int chn  = 0;
	int s32Ret = 0;
	int count = 0;
	ChSelect_S sSelect = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	sSelect.signal = P3K_CheckSignalType(str[0]);
	sSelect.ch_id= atoi(str[1]);

	s32Ret = EX_SetDecoderAVChannelId(&sSelect);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetDecoderAVChannelId err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetChannleSelection(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetChannleSelection\n");
	int s32Ret = 0;
	int chn = 0;
	int count = 0;
	ChSelect_S sSelect = {0};
	char str1[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	sSelect.signal = P3K_CheckSignalType(str[0]);

	s32Ret =  EX_GetDecoderAVChannelId(&sSelect);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDecoderAVChannelId err\n");
	}
	P3K_SignaleTypeToStr(sSelect.signal,str1);
	sprintf(tmpparam,"%s,%d",str1,sSelect.ch_id);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetVideoOutMode(char*reqparam,char*respParam,char*userdata)
{
	//#IMAGE-PROP scaler_id,,video_mode<CR>
	//~nn@IMAGE-PROP scaler_id,video_mode��<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutMode\n");
	int s32Ret = 0;
	int scalerId = 0;
	int videoMode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	scalerId = atoi(str[0]);
	videoMode = atoi(str[1]);

	s32Ret = EX_SetVideoImageStatus(scalerId, videoMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVideoImageStatus err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetVideoOutMode(char*reqparam,char*respParam,char*userdata)
{
	//#IMAGE PROP? scaler_id<CR>:
	//~nn@IMAGE-PROP scaler_id,video_mode��<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutMode\n");
	int s32Ret = 0;
	int scalerId = 0;
	int videoMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	scalerId = atoi(str[0]);

	s32Ret = EX_GetVideoImageStatus(scalerId, &videoMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoImageStatus err\n");
	}
	sprintf(tmpparam,"%d,%d",scalerId,videoMode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetVideoAction(char*reqparam,char*respParam,char*userdata)
{
	//#KDS ACTION kds_mode<CR>
	//~nn@KDS ACTION kds_mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoAction\n");
	int type = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};


	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	type = atoi(str[0]);

	s32Ret = EX_SetVideoCodecAction(type);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVideoCodecAction err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetVideoAction(char*reqparam,char*respParam,char*userdata)
{
	//#KDS ACTION? <CR>
	//~nn@KDS ACTION kds_mode<CR><LF>
	DBG_InfoMsg("P3K_GetVideoAction\n");
	int type = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoCodecAction(&type);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoCodecAction err\n");
	}

	sprintf(tmpparam,"%d",type);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetVideoOutCS(char*reqparam,char*respParam,char*userdata)
{
	//#CS-CONVERT out_index,cs_mode<CR>
	//~nn@CS-CONVERT out_index,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutCS\n");
	int s32Ret = 0;
	int outId = 0;
	int csMode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	outId = atoi(str[0]);
	csMode = atoi(str[1]);
	s32Ret = EX_SetColorSpaceConvertMode(outId, csMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetColorSpaceConvertMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetVideoOutCS(char*reqparam,char*respParam,char*userdata)
{
	//#CS-CONVERT? out_index<CR>
	//~nn@CS-CONVERT out_index,cs_mode<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutCS\n");
	int outId;
	int csMode;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	outId = atoi(str[0]);

	s32Ret = EX_GetColorSpaceConvertMode(outId, &csMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetColorSpaceConvertMode err\n");
	}
	sprintf(tmpparam,"%d,%d",outId,csMode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetVideoOutScaler(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-SCALE value<CR>
	//~nn@KDS-SCALE value,res_type<CR><LF>
	DBG_InfoMsg("P3K_SetVideoOutScaler\n");
	int s32Ret = 0;
	int mode = 0;
	int res  = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode =atoi(str[0]);
	res = atoi(str[1]);

	s32Ret = EX_SetVideoImageScaleMode(mode, res);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVideoImageScaleMode err\n");
	}
	sprintf(tmpparam,"%d,%d",mode,res);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetVideoOutScaler(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-SCALE? <CR>
	//~nn@KDS-SCALE value,res_type<CR><LF>
	DBG_InfoMsg("P3K_GetVideoOutScaler\n");
	int mode = 0;
	int s32Ret = 0;
	char res[64]={0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoImageScaleMode(&mode, res);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoImageScaleMode err\n");
	}
	sprintf(tmpparam,"%d,%s",mode,res);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetVideoReslotion(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-RESOL? io_mode,io_index,is_native<CR>
	//~nn@KDS-RESOL? io_mode,io_index,is_native,resolution<CR><LF>
	DBG_InfoMsg("P3K_GetVideoReslotion\n");
	int s32Ret = 0;
	int ioMode = 0;
	int ioIndex = 0;
	int nativeFlag = 0;
	int reslotion = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	ioMode = atoi(str[0]);
	ioIndex = atoi(str[1]);
	nativeFlag = atoi(str[2]);
	s32Ret = EX_GetVideoViewReslotion(ioMode,ioIndex, nativeFlag,&reslotion);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoViewReslotion err\n");
	}
	sprintf(tmpparam,"%s,%d",reqparam,reslotion);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetVideoFrameRate(char*reqparam,char*respParam,char*userdata)
{

	//#KDS-FR? <CR>
	//~nn@KDS-FR value<CR><LF>
	DBG_InfoMsg("P3K_GetVideoFrameRate\n");
	int fps = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoFrameRate(&fps);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoFrameRate err\n");
	}
	sprintf(tmpparam,"%d",fps);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetVideoBitRate(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-BR? <CR>
	//~nn@KDS-BR bitrate<CR><LF>
	DBG_InfoMsg("P3K_GetVideoBitRate\n");
	int bitRate = 0;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVideoBitRate(&bitRate);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVideoBitRate err\n");
	}
	sprintf(tmpparam,"%d",bitRate);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SendCECMsg(char*reqparam,char*respParam,char*userdata)
{
	//#CEC-SND port_index,sn_id,cmd_name,cec_len,cec_command<CR>
	//~nn@CEC-SND port_index,sn_id,cmd_name,cec_mode<CR><LF>
	DBG_InfoMsg("P3K_SendCECMsg\n");
	CECMessageInfo_S cecMsg = {0};
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	cecMsg.portId = atoi(str[0]);
	cecMsg.serialNumb = atoi(str[1]);
	memcpy(cecMsg.cmdName,str[2],strlen(str[2]));
	cecMsg.hexByte = atoi(str[3]);
	memcpy(cecMsg.cmdComent,str[4],strlen(str[4]));

	ret = EX_SendCECMsg(&cecMsg);

	sprintf(tmpparam,"%d,%d,%s,%d",cecMsg.portId,cecMsg.serialNumb,cecMsg.cmdName,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_RecvCECNtfy(char*reqparam,char*respParam,char*userdata)
{
	//#CEC-NTFY <CR>
       //~nn@CEC-NTFY port_index,len,<cec_command��><CR><LF>
       DBG_InfoMsg("P3K_RecvCECNtfy\n");
	int s32Ret =0;
	int portId;
	int hexByte;
	char cmdComment[CEC_MAX_CMD_COMENT_LEN+1] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_RecvCECNtfy(&portId, &hexByte, cmdComment);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_RecvCECNtfy err\n");
	}
	sprintf(tmpparam,"%d,%d,%s",portId,hexByte,cmdComment);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetCECGWMode(char*reqparam,char*respParam,char*userdata)
{
	//#CEC-GW-PORT-ACTIVE gatewaymode <CR>P3K_GetCECGWMode
	//~nn@CEC-GW-PORT-ACTIVE gatewaymode <CR><LF>
	DBG_InfoMsg("P3K_SetCECGWMode\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int mode = 0;

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);

	s32Ret =  EX_SetCECGateWayMode(mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetCECGateWayMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetCECGWMode(char*reqparam,char*respParam,char*userdata)
{
	//#CEC-GW-PORT-ACTIVE? gatewaymode <CR>
	//~nn@CEC-GW-PORT-ACTIVE gatewaymode <CR><LF>
	DBG_InfoMsg("P3K_GetCECGWMode\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	mode =  EX_GetCECGateWayMode();
	sprintf(tmpparam,"%d",mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SendIRMsg(char*reqparam,char*respParam,char*userdata)
{
	//#IR-SND ir_index,sn_id,cmd_name,repeat_amount,total_packages, package_id,<pronto command��><CR>
	//~nn@IR-SND ir_index,sn_id,cmd_name,ir_status<CR><LF>
	DBG_InfoMsg("P3K_SendIRMsg\n");
	IRMessageInfo_S irMsg = {0};
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);

	irMsg.irId =atoi(str[0]);
	irMsg.serialNumb = atoi(str[1]);
	memcpy(irMsg.cmdName,str[2],strlen(str[2]));
	irMsg.repeat = atoi(str[3]);
	irMsg.totalPacket = atoi(str[4]);
	irMsg.packId = atoi(str[5]);
	memcpy(irMsg.cmdComent,str[6],strlen(str[6]));

	ret = EX_SendIRmessage(&irMsg);

	sprintf(tmpparam,"%d,%d,%s,%d",irMsg.irId,irMsg.serialNumb,irMsg.cmdName,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_StopIRMsg(char*reqparam,char*respParam,char*userdata)
{
	//#IR-STOP ir_index,sn_id,cmd_name<CR>
	//~nn@IR-STOP ir_index,sn_id,cmd_name,ir_status<CR><LF>
	DBG_InfoMsg("P3K_StopIRMsg\n");
	int ret = 0;
	int irId = 0;
	int serialId = 0;
	char command[IR_MAX_CMD_COMENT_LEN+1] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	irId =atoi(str[0]);
	serialId = atoi(str[1]);
	memcpy(command,str[2],strlen(str[2]));

	ret = EX_SendIRStop(irId, serialId, command);

	sprintf(tmpparam,"%d,%d,%s,%d",irId,serialId,command,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetXROUTEMatch(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_SetXROUTEMatch\n");
	//#X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR>
	//~nn@X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR><LF>
	PortInfo_S outInfo[12] = {0};
	PortInfo_S inInfo;
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char str1[][MAX_PARAM_LEN] = {0};
	char str2[][MAX_PARAM_LEN] = {0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	if(count == 2)
	{
		P3K_GetPortSInfo(str[0], &outInfo,0);
		P3K_GetPortInfo(str[1], &inInfo);
	}
	if(count > 2 )//&& !memcmp(str[0],"[",strlen("[")))
	{
		/*char * aTmp = "";
		memcpy(aTmp,str[count - 2],strlen(str[count - 2]));
		memset(str[count - 2],0,sizeof(str[count - 2]));*/
		memcpy(str[count - 2],str[count - 2],(strlen(str[count - 2])-1));
		int i =0;
		for(i=0;i<count-1;i++)
		{
			P3K_GetPortSInfo(str[i], &outInfo,i);
		}
		P3K_GetPortInfo(str[count-1], &inInfo);
	}
	 //printf(",,,.,.,%s\n",str[count-1]);
	s32Ret = EX_SetRouteMatch(&outInfo,&inInfo,count-1);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetRouteMatch err\n");
	}

	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetXROUTEMatch(char*reqparam,char*respParam,char*userdata)
{
	//#X-ROUTE? <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1><CR>
	//~nn@X-ROUTE <direction_type1>. <port_type1>. <port_index1>. <signal_type1>. <index1>,<direction_type2>. <port_type2>. <port_index2>. <signal_type2>. <index2><CR><LF>
	DBG_InfoMsg("P3K_GetXROUTEMatch\n");
	int s32Ret = 0;
	PortInfo_S outInfo;
	PortInfo_S inInfo;
	char direc[32] = {0};
	char portFormat[32] = {0};
	char signal[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char str1[][MAX_PARAM_LEN] = {0};
	char str2[][MAX_PARAM_LEN] = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);

	P3K_GetPortInfo(str[0], &outInfo);


	s32Ret =  EX_GetRouteMatch(&outInfo, &inInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetRouteMatch err\n");
	}
	P3K_PortDirectionToStr(outInfo.direction, direc);
	P3K_PortFormatToStr(outInfo.portFormat, portFormat);
	P3K_SignaleTypeToStr(outInfo.signal,signal);
	sprintf(tmpparam,"%s.%s.%d.%s.%d",direc,portFormat,outInfo.portIndex,signal,outInfo.index);
	memset(direc,0,sizeof(direc));
	memset(portFormat,0,sizeof(portFormat));
	memset(signal,0,sizeof(signal));

	P3K_PortDirectionToStr(inInfo.direction, direc);
	P3K_PortFormatToStr(inInfo.portFormat, portFormat);
	P3K_SignaleTypeToStr(inInfo.signal,signal);

	sprintf(tmpparam+strlen(tmpparam),",%s.%s.%d.%s.%d",direc,portFormat,inInfo.portIndex,signal,inInfo.index);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetUartConf(char*reqparam,char*respParam,char*userdata)
{
	//#UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR>
	//~nn@UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR><LF>
	DBG_InfoMsg("P3K_SetUartConf\n");
	int s32Ret = 0;
	UartMessageInfo_S uartConf = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	uartConf.comNumber = atoi(str[0]);
	uartConf.rate = atoi(str[1]);
	uartConf.bitWidth = atoi(str[2]);
	uartConf.parity  = P3K_CheckUartParity(str[3]);
	uartConf.stopBitsMode = strtod(str[4],NULL);
	uartConf.serialType = 0;
	uartConf.term_485 = 0;
	if(count > 5)
	{
		uartConf.serialType = atoi(str[5]);
		uartConf.term_485 = atoi(str[5]);
	}

	s32Ret = EX_SetUartConf(&uartConf);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetUartConf err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int floatTostr(float in,char*dststr)
{
	char tmpstr[16] ={0};
	int len = 0;
	int i = 0;
	sprintf(tmpstr,"%f",in);
	len = strlen(tmpstr);
	if(tmpstr[len-1] !='0')
	{
		memcpy(dststr,tmpstr,len);
		return 0;
	}
	for(i= len-1 ;i>0;i--)
	{
		if(tmpstr[i] !='0')
		{
			if(tmpstr[i] == '.')
			{
				memcpy(dststr,tmpstr,i);
			}
			else
			{
				memcpy(dststr,tmpstr,i+1);
			}
			break;
		}
	}
	return 0;
}
static int P3K_GetUartConf(char*reqparam,char*respParam,char*userdata)
{
	//#UART? com_id<CR>
	//~nn@UART com_id,baud_rate,data_bits,parity,stop_bits_mode, serial_type,485_term<CR><LF>
	DBG_InfoMsg("P3K_GetUartConf\n");
	int s32Ret = 0;
	UartMessageInfo_S uartConf = {0};
	int comId = 0;
	char parity[32] ={0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char floatStr[16]={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	comId = atoi(str[0]);


	s32Ret = EX_GetUartConf(comId, &uartConf);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetUartConf err\n");
	}
	P3K_ParityToStr(uartConf.parity, parity);
	floatTostr( uartConf.stopBitsMode,floatStr);
	if(uartConf.serialType == 0)
	{
		sprintf(tmpparam+strlen(tmpparam),"%d,%d,%d,%s,%s",
		   	   uartConf.comNumber,uartConf.rate,uartConf.bitWidth,parity,
		          floatStr);
	}
	else
	{
		sprintf(tmpparam+strlen(tmpparam),"%d,%d,%d,%s,%s,%d,%d",
		   	   uartConf.comNumber,uartConf.rate,uartConf.bitWidth,parity,
			   floatStr,uartConf.serialType,uartConf.term_485);
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_AddComRoute(char*reqparam,char*respParam,char*userdata)
{
	//#COM-ROUTE-ADD com_id,port_type,port_id,eth_rep_en,timeout<CR>
	//~nn@COM ROUTE ADD com_id,port_type,port_id,eth_rep_en,timeout<CR><LF>
	DBG_InfoMsg("P3K_AddComRoute\n");
	int s32Ret = 0;
	ComRouteInfo_S routeInfo = {0};
	int comId = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	comId = atoi(str[0]);
	routeInfo.portType = atoi(str[1]);
	routeInfo.portNumber = atoi(str[2]);
	routeInfo.rePlay = atoi(str[3]);
	routeInfo.HeartTimeout = atoi(str[4]);

	s32Ret = EX_AddComRoute(&routeInfo, comId);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_AddComRoute err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_RemoveComRoute(char*reqparam,char*respParam,char*userdata)
{
	//#COM-ROUTE-REMOVE com_id<CR>
	//~nn@COM-ROUTE-REMOVE com_id<CR><LF>
	DBG_InfoMsg("P3K_RemoveComRoute\n");
	int s32Ret = 0;
	int comId = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	comId = atoi(str[0]);

	s32Ret = EX_RemoveComRoute(comId);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_AddComRoute err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetComRouteInfo(char*reqparam,char*respParam,char*userdata)
{
	//#COM-ROUTE? com_id<CR>
	//~nn@COM-ROUTE com_id,port_type,port_id,eth_rep_en,ping_val<CR><LF>
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);

	int comId = 0;;
	int ret = 0;
	ComRouteInfo_S info;
	ret = EX_GetComRoute(comId,&info);

	if(ret == 0) //enable
	{
		sprintf(respParam,"1,%d,%d,%d,%d",info.portType,info.portNumber,info.rePlay,info.HeartTimeout);
	}

	return 0;
}
static int P3K_GetEthTunnel(char*reqparam,char*respParam,char*userdata)
{
	//#ETH-TUNNEL? tunnel_id<CR>
	//~nn@ETH-TUNNEL tunnel_id,cmd_name,port_type,port_id,eth_ip,remote_port_id,eth_rep_en,connection_type<CR><LF>
	return 0;
}
static int P3K_SetUSBMode(char*reqparam,char*respParam,char*userdata)
{
	//#KVM-USB-CTRL usb_type<CR>
      //~nn@KVM-USB-CTRL usb_type<CR><LF>
   //   EX_GetUSBCtrl(int * type)
   	int type = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	type = atoi(str[0]);
	s32Ret = EX_SetUSBCtrl(type);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetUSBCtrl err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetMulticastStatus(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-MULTICAST? <CR>
	//~nn@KDS-ACTION group_ip,ttl<CR><LF>
	DBG_InfoMsg("P3K_GetMulticastStatus\n");
	int s32Ret = 0;
	char gIp[MAX_IP_ADDR_LEN]={0};
	int ttl = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetMulticastInfo(gIp, &ttl);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetMulticastInfo err\n");
	}
	sprintf(tmpparam,"%s,%d",gIp,ttl);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetMacAddr(char*reqparam,char*respParam,char*userdata)
{
	//#FCT-MAC mac_address<CR>
	//~nn@FCT-MAC mac_address<CR><LF>
	DBG_InfoMsg("P3K_SetMacAddr\n");
	int s32Ret = 0;
	int id = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	if(count == 1)
	{
		s32Ret = EX_SetMacAddr(id, str[0]);
	}
	else
	{

	}
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetMacAddr err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetMacAddr(char*reqparam,char*respParam,char*userdata)
{
	//#NET-MAC? id<CR>
	//~nn@NET MAC id,mac_address<CR><LF>
	DBG_InfoMsg("P3K_GetMacAddr\n");
	int netId = 0;
	int s32Ret =0;
	char mac[MAC_ADDR_LEN] ={0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	netId = atoi(str[0]);

	s32Ret = EX_GetMacAddr(netId,mac);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetMacAddr err\n");
	}
	sprintf(tmpparam,"%d,%s",netId,mac);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetDNSName(char*reqparam,char*respParam,char*userdata)
{
	//#NAME machine_name<CR>
	//~nn@NAME machine_name<CR><LF>
	DBG_InfoMsg("P3K_SetDNSName\n");
	int s32Ret =0;
	int count = 0;
	int id = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	id = atoi(str[0]);

	s32Ret = EX_SetDNSName(id,str[1]);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetDNSName err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetDNSName(char*reqparam,char*respParam,char*userdata)
{
	//#NAME? <CR>
	//~nn@NAME machine_name<CR><LF>
	DBG_InfoMsg("P3K_GetDNSName\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char name[MAX_DEV_NAME_LEN+1] ={0};
	int id = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	int count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	id = atoi(str[0]);
	s32Ret = EX_GetDNSName(id,name);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDNSName err\n");
	}
	sprintf(tmpparam,"%d,%s",id,name);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_ResetDNSName(char*reqparam,char*respParam,char*userdata)
{
	//#NAME-RST <CR>
	//~nn@NAME-RST ok<CR><LF>
	DBG_InfoMsg("P3K_ResetDNSName\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char name[MAX_DEV_NAME_LEN+1] ={0};

	s32Ret = EX_ResetDNSName(name);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_ResetDNSName err\n");
	}
	sprintf(tmpparam,"%s",name);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetDHCPMode(char*reqparam,char*respParam,char*userdata)
{

	//#NET-DHCP netw_id,dhcp_state<CR>
	//~nn@NET-DHCP netw_id,dhcp_state<CR><LF>
	DBG_InfoMsg("P3K_SetDHCPMode\n");
	int s32Ret = 0;
	int nedId  = 0;
	int dhcpMode  = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	if(count == 1)
	{
		dhcpMode = atoi(str[0]);

	}
	else if(count == 2)
	{
		nedId = atoi(str[0]);
		dhcpMode = atoi(str[1]);
	}
	else
	{
		DBG_ErrMsg("EX_SetDHCPMode err\n");
		return 0;
	}

	s32Ret = EX_SetDHCPMode(nedId, dhcpMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetDHCPMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));

	return 0;
}
static int P3K_GetDHCPMode(char*reqparam,char*respParam,char*userdata)
{
	//#NET-DHCP? netw_id<CR>
	//~nn@NET-DHCP netw_id,dhcp_mode<CR><LF>
	DBG_InfoMsg("P3K_GetDHCPMode\n");
	int s32Ret = 0;
	int netId = 0;
	int mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	if(count > 0)
	{
		netId = atoi(str[0]);
	}

	s32Ret = EX_GetDHCPMode(netId, &mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDHCPMode err\n");
	}
	sprintf(tmpparam,"%d,%d",netId,mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetNetConf(char*reqparam,char*respParam,char*userdata)
{
	//#NET-CONFIG netw_id,net_ip,net_mask,gateway,[dns1],[dns2]<CR>
	//~nn@NET-CONFIG netw_id,net_ip,net_mask,gateway<CR><LF>
	DBG_InfoMsg("P3K_SetNetConf\n");
	int  s32Ret = 0;
	int netId;
	NetWorkInfo_S netInfo = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);


	if(strlen(str[0])<7)
	{
		//printf("----------------------------\n");
		netId = atoi(str[0]);
		memcpy(netInfo.ipAddr,str[1],strlen(str[1]));
		memcpy(netInfo.mask,str[2],strlen(str[2]));
		memcpy(netInfo.gateway,str[3],strlen(str[3]));
		if(count > 4)
		{
			memcpy(netInfo.dns1,str[4],strlen(str[4]));
		}
		if(count > 5)
		{
			memcpy(netInfo.dns1,str[5],strlen(str[5]));
		}

	}
	else
	{
	//	printf("--------------22--------------\n");
		memcpy(netInfo.ipAddr,str[0],strlen(str[0]));
		memcpy(netInfo.mask,str[1],strlen(str[1]));
		memcpy(netInfo.gateway,str[2],strlen(str[2]));
		if(count > 3)
		{
			memcpy(netInfo.dns1,str[3],strlen(str[3]));
		}
		if(count > 4)
		{
			memcpy(netInfo.dns1,str[4],strlen(str[4]));
		}
	}


	s32Ret = EX_SetNetWorkConf(netId, &netInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetNetWorkConf err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetNetConf(char*reqparam,char*respParam,char*userdata)
{
	//#NET-CONFIG? netw_id<CR>
	//~nn@NET-CONFIG netw_id,net_ip,net_mask,gateway<CR><LF>
	DBG_InfoMsg("P3K_GetNetConf\n");
	int s32Ret = 0;
	int netId = 0;
	NetWorkInfo_S netInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	netId =atoi(str[0]);


	s32Ret = EX_GetNetWorkConf(netId, &netInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetNetWorkConf err\n");
	}
	sprintf(tmpparam,"%d,%s,%s,%s",netId,netInfo.ipAddr,netInfo.mask,netInfo.gateway);
	if(strlen(netInfo.dns1) > 0)
	{
		strcat(tmpparam,",");
		strcat(tmpparam,netInfo.dns1);
	}
	if(strlen(netInfo.dns2) > 0)
	{
		strcat(tmpparam,",");
		strcat(tmpparam,netInfo.dns2);
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetEthPort(char*reqparam,char*respParam,char*userdata)
{
	//#ETH-PORT port_type,port_id<CR>
	//~nn@ETH-PORT port_type,port_id<CR><LF>
	DBG_InfoMsg("P3K_SetEthPort\n");
	int s32Ret = 0;
	char ethType[16]={0};
	int ethPort;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(ethType,str[0],sizeof(ethType));
	ethPort = atoi(str[1]);

	s32Ret = EX_SetNetPort(ethType,ethPort);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetNetPort err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetEthPort(char*reqparam,char*respParam,char*userdata)
{
	//#ETH-PORT? port_type<CR>
	//~nn@ETH-PORT port_type,port_id<CR><LF>
	DBG_InfoMsg("P3K_GetEthPort\n");
	int s32Ret = 0;
	char ethType[16] ={0};
	int ethPort;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(ethType,str[0],sizeof(ethType));

	s32Ret = EX_GetNetPort(ethType, &ethPort);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetNetPort err\n");
	}
	sprintf(tmpparam,"%s,%d",ethType,ethPort);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetSecurity(char*reqparam,char*respParam,char*userdata)
{
	//#SECUR security_state<CR>
	//~nn@SECUR security_state<CR><LF>
	DBG_InfoMsg("P3K_SetSecurity\n");
	int s32Ret = 0;
	int status = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	status = atoi(str[0]);

	s32Ret = EX_SetSecurityStatus(status);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetSecurityStatus err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_SetLogin(char*reqparam,char*respParam,char*userdata)
{
	//#LOGIN login_level,password<CR>
	//~nn@LOGIN login_level,password ok<CR><LF>
	//or
	//~nn@LOGIN err_004<CR><LF>
	DBG_InfoMsg("P3K_SetLogin\n");

	int ret = 0;
	char userName[32]={0};
	char usrerPasswd[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(userName,str[0],strlen(str[0]));
	memcpy(usrerPasswd,str[1],strlen(str[1]));

	sprintf(tmpparam,"%s,%s",reqparam,"ok");
	ret = EX_Login(userName, usrerPasswd);
	if(ret != 0)
	{
		memset(tmpparam,0,sizeof(tmpparam));
		sprintf(tmpparam,"%s","err_004");
	}

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetLogin(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetLogin\n");
	int s32Ret = 0;
	char userName[32]={0};
	char usrerPasswd[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLoginInfo(userName, usrerPasswd);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetLoginInfo err\n");
	}
	sprintf(tmpparam,"%s",userName);

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_LogOut(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_LogOut\n");
	int s32Ret = 0;
	//#LOGOUT<CR>
	//~nn@LOGOUT ok<CR><LF>
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_Logout();
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetLoginInfo err\n");
	}
	sprintf(tmpparam,"%s","ok");

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetVersion(char*reqparam,char*respParam,char*userdata)
{
	//#VERSION? <CR>
	//~nn@VERSION firmware_version<CR><LF>
	DBG_InfoMsg("P3K_GetVersion\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char ver[32] = {0};

	s32Ret = EX_GetDevVersion(ver);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDevVersion err\n");
	}
	sprintf(tmpparam,"%s",ver);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetFCTMODEL(char*reqparam,char*respParam,char*userdata)
{
	//#FCT-MODEL model_name<CR>
	//~nn@FCT-MODEL model_name<CR><LF>
	DBG_InfoMsg("P3K_SetFCTMODEL\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);


	s32Ret = EX_SetDeviceNameModel(str[0]);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetDeviceNameModel err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetFCTMODEL(char*reqparam,char*respParam,char*userdata)
{
	//#MODEL? <CR>
	//~nn@MODEL model_name<CR><LF>
	DBG_InfoMsg("P3K_GetFCTMODEL\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char mod[MAX_DEV_MOD_NAME_LEN+1];

	s32Ret = EX_GetDeviceNameModel(mod);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDeviceNameModel err\n");
	}
	sprintf(tmpparam,"%s",mod);

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_Upgrade(char*reqparam,char*respParam,char*userdata)
{
	//#UPGRADE<CR>
	//~nn@UPGRADE ok<CR><LF>
	DBG_InfoMsg("P3K_Upgrade\n");
	int s32Ret = -1;
	char tmpparam[MAX_PARAM_LEN] = {0};

    struct stat buf;
	memset(&buf, 0, sizeof(buf));
	if ((0 == stat("/dev/shm/IP5000-A30_upgrade.tar.gz", &buf)) && (buf.st_size > 0))
	{
		s32Ret = EX_Upgrade();
		if(s32Ret)
		{
			DBG_ErrMsg("EX_Upgrade err\n");
			sprintf(tmpparam,"%s","err 002");
		}
		else
		{
			sprintf(tmpparam,"%s","ok");
		}
	}
	else
	{
		sprintf(tmpparam,"%s","err 002");
	}

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return s32Ret;
}
static int P3K_UpgradeStatus(char*reqparam,char*respParam,char*userdata)
{
	//#UPGRADE-STATUS<CR>
	//~nn@UPGRADE-STATUS ongoing,100,0<CR><LF>
	DBG_InfoMsg("P3K_UpgradeStatus\n");
	int s32Ret = 0;

	GetUpgradeStatus(respParam, MAX_PARAM_LEN);
	return 0;
}
static int P3K_SetSerailNum(char*reqparam,char*respParam,char*userdata)
{
	//#FCT-SN serial_num<CR>
	//~nn@FCT-SN serial_num<CR><LF>
	DBG_InfoMsg("P3K_SetSerailNum\n");
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);

	s32Ret = EX_SetSerialNumber(str[0]);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetSerialNumber err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetSerailNum(char*reqparam,char*respParam,char*userdata)
{
	//#SN? <CR>
	//~nn@SN serial_num<CR><LF>
	DBG_InfoMsg("P3K_GetSerailNum\n");
	int s32Ret = 0;
	char num[SERIAL_NUMBER_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetSerialNumber(num);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetSerialNumber err\n");
	}
	sprintf(tmpparam,"%s",num);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_SetLockFP(char*reqparam,char*respParam,char*userdata)
{
	//#LOCK-FP lock/unlock<CR>
	//~nn@LOCK-FP lock/unlock<CR><LF>
	DBG_InfoMsg("P3K_SetLockFP\n");
	int s32Ret = 0;

	int flag = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	flag = atoi(str[0]);

	s32Ret = EX_SetLockFP(flag);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetSerialNumber err\n");
	}

	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetLockFP(char*reqparam,char*respParam,char*userdata)
{
	//#LOCK-FP? <CR>
	//~nn@LOCK-FP lock/unlock<CR><LF>
	DBG_InfoMsg("P3K_GetLockFP\n");
	int s32Ret = 0;
	int flag = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLockFP(&flag);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetLockFP err\n");
	}
	sprintf(tmpparam,"%d",flag);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_SetIDV(char*reqparam,char*respParam,char*userdata)
{
	//#IDV<CR>
	//~nn@IDV ok<CR><LF>
	DBG_InfoMsg("P3K_SetIDV\n");
	int s32Ret = 0;

	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_SetIDV();
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetIDV err\n");
	}
	sprintf(tmpparam,"%s","ok");
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_SetStandByMode(char*reqparam,char*respParam,char*userdata)
{
	//#STANDBY value<CR>P3K_GetStandByMode
	//~nn@STANDBY value<CR><LF>
	DBG_InfoMsg("P3K_SetStandByMode\n");
	int s32Ret = 0;

	int mode = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	mode = atoi(str[0]);
	s32Ret = EX_SetStandbyMode(mode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetStandbyMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetStandByMode(char*reqparam,char*respParam,char*userdata)
{
	//#STANDBY value<CR>P3K_GetStandByMode
	//~nn@STANDBY value<CR><LF>
	DBG_InfoMsg("P3K_GetStandByMode\n");
	int s32Ret = 0;

	int iValue = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetStandbyMode(&iValue);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetStandbyMode err\n");
	}
	sprintf(tmpparam,"%d",iValue);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}


static int P3K_DoReset(char*reqparam,char*respParam,char*userdata)
{
	//#RESET<CR>
	//~nn@RESET ok<CR><LF>
	DBG_InfoMsg("P3K_DoReset\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_DeviceReset();
	if(s32Ret)
	{
		DBG_ErrMsg("EX_DeviceReset err\n");
	}
	sprintf(tmpparam,"%s","ok");
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_DoFactory(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_DoFactory\n");
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_FactoryRecovery();
	if(s32Ret)
	{
		DBG_ErrMsg("EX_FactoryRecovery err\n");
	}
	sprintf(tmpparam,"%s","ok");

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetBeaconEn(char*reqparam,char*respParam,char*userdata)
{
	//#BEACON-EN port_id,status,rate<CR>
	//~nn@BEACON-EN port_id,status,rate<CR><LF>
	DBG_InfoMsg("P3K_SetBeaconEn\n");
	int s32Ret = 0;
	int portNumber = 0;
	int status = 0;
	int sec = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	portNumber = atoi(str[0]);
	status = atoi(str[1]);
	sec = atoi(str[2]);
	s32Ret = EX_SetBeacon(portNumber, status, sec);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetBeacon err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetBeaconInfo(char*reqparam,char*respParam,char*userdata)
{
	//#BEACON INFO? port_id<CR>
	//~nn@BEACON INFO port_id,ip_string,udp_port,tcp_port, mac_address, model,name<CR><LF>
	DBG_InfoMsg("P3K_GetBeaconInfo\n");
	int s32Ret = 0;
	int portNumber;
	BeaconInfo_S beaconInfo = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	portNumber = atoi(str[0]);

	s32Ret = EX_GetBeaconInfo(portNumber, &beaconInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetBeaconInfo err\n");
	}
	sprintf(tmpparam,"%d,%s,%d,%d,%s,%s,%s",portNumber,beaconInfo.ipAddr,beaconInfo.udpPort,
		   beaconInfo.tcpPort,beaconInfo.macAddr,beaconInfo.deviceMod,beaconInfo.deviceName);

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetBuildTime(char*reqparam,char*respParam,char*userdata)
{
	//#BUILD-DATE? <CR>
	//~nn@BUILD-DATE? date,time<CR><LF>
	DBG_InfoMsg("P3K_GetBuildTime\n");
	int s32Ret = 0;

	char date[32] ={0};
	char time[32] ={0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetDevBuildDate(date, time);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDevBuildDate err\n");
	}
	sprintf(tmpparam,"%s,%s",date,time);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetTime(char*reqparam,char*respParam,char*userdata)
{
	//#TIME day_of_week,date,data<CR>
	//~nn@TIME day_of_week,date,data<CR><LF>
	DBG_InfoMsg("P3K_SetTime\n");
	int s32Ret = 0;
	char pweekDay[16] = {0};
	char pdate[32]= {0};
	char ptime[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
//	printf("%s len=%d\n",reqparam,strlen(reqparam));
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
//	printf("%s------%s\n",str[0],str[1]);

	sprintf(pdate,"%s",str[1]);
	sprintf(pweekDay,"%s",str[0]);
	sprintf(ptime,"%s",str[2]);
	//memcpy(weekDay,str[0],strlen(str[0]));

	//memcpy(date,str[1],strlen(str[1]));
	//memcpy(time,str[2],strlen(str[2]));

	s32Ret = EX_SetTimeAndDate(pweekDay,pdate,ptime);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetTimeAndDate err\n");
	}
	//printf("%s\n",reqparam);
	sprintf(tmpparam,"%s,%s,%s",pweekDay,pdate,ptime);
	memcpy(respParam,tmpparam,strlen(tmpparam));

	return 0;
}
static int P3K_GetTime(char*reqparam,char*respParam,char*userdata)
{
	//#TIME? <CR>
	//~nn@TIME day_of_week,date,data<CR><LF>
	DBG_InfoMsg("P3K_GetTime\n");
	int s32Ret = 0;
	char weekDay[16] = {0};
	char date[32]= {0};
	char time[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetTimeAndDate(weekDay, date,time);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetTimeAndDate err\n");
	}
	sprintf(tmpparam,"%s,%s,%s",weekDay,date,time);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetTimeZero(char*reqparam,char*respParam,char*userdata)
{
	//#TIME-LOC utc_off,dst_state<CR>
	//~nn@TIME-LOC utc_off,dst_state<CR><LF>
	DBG_InfoMsg("P3K_SetTimeZero\n");
	int s32Ret = 0;
	int tz = 0;
	int timingMethod = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	tz = atoi(str[0]);
	timingMethod = atoi(str[1]);
	s32Ret = EX_SetTimeZero( tz,timingMethod);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetTimeZero err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetTimeZero(char*reqparam,char*respParam,char*userdata)
{
	//#TIME-LOC? <CR>
	//~nn@TIME-LOC utc_off,dst_state<CR><LF>
	DBG_InfoMsg("P3K_GetTimeZero\n");
	int s32Ret = 0;
	int tz = 0;
	int timingMethod = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetTimeZero(&tz,&timingMethod);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetTimeZero err\n");
	}
	sprintf(tmpparam,"%d,%d",tz,timingMethod);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetTimeServer(char*reqparam,char*respParam,char*userdata)
{
	//#TIME-SRV mode,time_server_ip,sync_hour<CR>
	//~nn@TIME SRV mode,time_server_ip,sync_hour,server_status<CR><LF>
	DBG_InfoMsg("P3K_SetTimeServer\n");

	TimeSyncConf_S syncInfo = {0};
	int status = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	syncInfo.enable = atoi(str[0]);
	memcpy(syncInfo.serverIp,str[1],sizeof(syncInfo.serverIp));
	syncInfo.syncInerval = atoi(str[2]);

	status = EX_SetTimeSyncInfo(&syncInfo);

	sprintf(tmpparam,"%s,%d",reqparam,status);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetTimeServer(char*reqparam,char*respParam,char*userdata)
{
	//#TIME-SRV? <CR>
	//~nn@TIME-SRV mode,time_server_ip,sync_hour,server_status<CR><LF>
	DBG_InfoMsg("P3K_GetTimeServer\n");

	TimeSyncConf_S syncInfo = {0};
	int status = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	status = EX_GetTimeSyncInfo(&syncInfo);

	sprintf(tmpparam,"%d,%s,%d,%d",syncInfo.enable,syncInfo.serverIp,syncInfo.syncInerval,status);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetSignalList(char*reqparam,char*respParam,char*userdata)
{
	//#SIGNALS-LIST? <CR>
	//~nn@SIGNALS-LIST [<direction_type>. <port_format>. <port_index>. <signal_type>. <index>,..,]<CR><LF>
	DBG_InfoMsg("P3K_GetSignalList\n");
	int ret = 0;
	char siglist[10][MAX_SIGNALE_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;
	ret = EX_GetSignalList(&siglist[0],10);
	if(ret > 10)
	{
		DBG_WarnMsg("P3K_GetSignalList num=%d over 1o\n",ret);
		ret =10;
	}
	for(i = 0;i<ret ;i++)
	{
		printf(" ret = %d ,sig=%s\n",ret,siglist[i]);
		strncat(tmpparam,siglist[i],MAX_SIGNALE_LEN);
		if(i <  (ret-1))
		{
			strncat(tmpparam,",",1);
		}
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetPortList(char*reqparam,char*respParam,char*userdata)
{
	//#PORTS-LIST? <CR>
	//~nn@PORTS-LIST [<direction_type>. <port_format>. <port_index>,..,]<CR><LF>
	DBG_InfoMsg("P3K_GetPortList\n");
	int ret = 0;

	char portlist[10][MAX_PORT_LEN] = {0};
	int i = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetPortList(portlist,10);
	if(ret > 10)
	{
		DBG_WarnMsg("P3K_GetPortList num=%d over 1o\n",ret);
		ret =10;
	}
	for(i = 0;i<ret ;i++)
	{
		strncat(tmpparam,portlist[i],MAX_PORT_LEN);
		if(i< (ret-1))
		{
			strncat(tmpparam,",",1);
		}
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_GetActiveCli(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-ACTIVE-CLNT? <CR>
	//~nn@KDS-ACTIVE-CLNT value<CR><LF>
	DBG_InfoMsg("P3K_GetActiveCli\n");

	char tmpparam[MAX_PARAM_LEN] = {0};
	int ret = 0;
	ret = EX_GetActiveCliNUm();
	sprintf(tmpparam,"%d",ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
static int P3K_SetLogAction(char*reqparam,char*respParam,char*userdata)
{
	//#LOG-ACTION action,period<CR>
	//~nn@LOG-ACTION action,period<CR><LF>
	DBG_InfoMsg("P3K_SetLogAction\n");
	int s32Ret = 0;

	int action =0;
	int period =0;

	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	action = atoi(str[0]);
	period = atoi(str[1]);

	s32Ret = EX_SetLogEvent(action, period);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetLogEvent err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));

	return 0;
}

static int P3K_GetLogAction(char*reqparam,char*respParam,char*userdata)
{
	//#LOG-ACTION?<CR>
	//~nn@LOG-ACTION action,period<CR><LF>
	DBG_InfoMsg("P3K_GetLogAction\n");
	int ret = 0;
	int action =0;
	int period =0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetLogEvent(&action,&period);
	if(ret)
	{
		DBG_ErrMsg("EX_GetLogEvent err\n");
	}
	sprintf(tmpparam,"%d,%d",action,period);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}


static int P3K_GetLogTail(char*reqparam,char*respParam,char*userdata)
{
	//#LOG-TAIL? line_num<CR>
	//~nn@LOG-TAILnn<CR><LF>
	//Line content #1<CR><LF>
	//Line content #2<CR><LF>
	DBG_InfoMsg("P3K_GetLogTail\n");
	int ret = 0;
	int number = 10;
	char log[MAX_USR_STR_LEN+1] ={0};

	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;
	int i =0;
	char pestr[10] ={0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	if(count > 0)
	{
		number = atoi(str[0]);
	}
	char tmplog[20][MAX_ONELOG_LEN] ={0};

	ret = EX_GetLogTail(number, tmplog);
	for(i = 0;i<ret;i++)
	{
		memset(pestr,0,sizeof(pestr));
		strncat(log,tmplog[i],MAX_ONELOG_LEN);
		sprintf(pestr,"#%d\r\n",i);
		strcat(log,pestr);

	}
	memcpy(userdata,log,strlen(log));
	sprintf(tmpparam,"%d",number);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetAudioInfo(char*param,AudioInfo_S*info)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	char tmpparam[MAX_PARAM_LEN] = {0};
	int count = 0;

	count = P3K_PhraserWithSeparator('.',param, strlen(param), str);


	info->direction  = P3K_CheckPortDirection(str[0]);
	info->portFormat = P3K_CheckPortFormat(str[1]);
	info->portIndex = atoi(str[2]);
	if(count > 3)
	{
		info->signal = P3K_CheckSignalType(str[3]);
		//info->index = atoi(str[4]);
	}
	return 0;
}

static int P3K_SetAudAnalogDir(char*reqparam,char*respParam,char*userdata)
{
	//#PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type> , direction<CR>
	//~nn@PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type>  , direction<CR><LF>
	DBG_InfoMsg("P3K_SetAudAnalogDir\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char gain[MAX_PARAM_LEN] = {0};
	AudioInfo_S  tmpInfo = {0};
	memset(&tmpInfo,0,sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam),str);
	memcpy(gain,str[1],strlen(str[1]));
	P3K_GetAudioInfo(str[0],&tmpInfo);
	s32Ret =  EX_SetAudAnalogGainDir(&tmpInfo,gain);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetAudAnalogGainDir err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return  0;
}

static int P3K_GetAudAnalogDir(char*reqparam,char*respParam,char*userdata)
{
	//#PORT-DIRECTION? <direction_type>. <port_format>. <port_index>. <signal_type> , direction<CR>
	//~nn@PORT-DIRECTION <direction_type>. <port_format>. <port_index>. <signal_type>  , direction<CR><LF>
	DBG_InfoMsg("P3K_GetAudAnalogDir\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	PortInfo_S tmpInfo = {0};
	char gain[MAX_PARAM_LEN] = {0};
	// both.analog.1.audio
	P3K_GetAudioInfo(reqparam,&tmpInfo);
	s32Ret = EX_GetAudAnalogGainDir(&tmpInfo,gain);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetAudAnalogGainDir err\n");
	}
	sprintf(tmpparam,"%s,%s",reqparam,gain);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetAudioSInfo(char*param,AudioInfo_S*info,int num)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;

	int count = P3K_PhraserWithSeparator('.',param, strlen(param), str);


		info[num].direction  = P3K_CheckPortDirection(str[0]);
		info[num].portFormat = P3K_CheckPortFormat(str[1]);
		info[num].portIndex = atoi(str[2]);
		if(count > 3)
		{
			info[num].signal = P3K_CheckSignalType(str[3]);
			//info->index = atoi(str[4]);
		}

	return 0;
}

static int P3K_SetAutoSwitchPriority(char*reqparam,char*respParam,char*userdata)
{
	//#X-PRIORITY <direction_type>. <port_format>. <port_index> .<signal_type> , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR>
	//~nn@X-PRIORITY <direction_type>. <port_format>. <port_index>.<signal_type>  , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR><LF>
	DBG_InfoMsg("P3K_SetAutoSwitchPriority\n");
	int count = 0;
	int i = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	AudioInfo_S  tmpparam[MAX_PARAM_COUNT]= {0};
	AudioInfo_S  tmpInfo = {0};
	memset(&tmpInfo,0,sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam),str);
	for(i=0;i<count;i++)
	{
		P3K_GetAudioSInfo(str[i],&tmpparam,i);

	}
	s32Ret =  EX_SetAutoSwitchPriority(&tmpInfo,&tmpparam,count);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetAutoSwitchPriority err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return  0;

}

static int P3K_GetAutoSwitchPriority(char*reqparam,char*respParam,char*userdata)
{
	//#X-PRIORITY? <direction_type>. <port_format>. <port_index>.<signal_type>
	//~nn@X-PRIORITY <direction_type>. <port_format>. <port_index>.<signal_type>  , [<direction_type>. <port_format>. <port_index>.<signal_type> ,...]<CR><LF>
	DBG_InfoMsg("P3K_GetAutoSwitchPriority\n");
	int count = 0;
	int num = 0;
	int ret = 0;
	int s32Ret = 0;
	char aStr[4] = ",[";
	char aStr1[3] = "]";
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char aStr2[MAX_PARAM_LEN] = {0};
	AudioInfo_S  tmp[MAX_PARAM_COUNT]= {0};
	AudioInfo_S  tmpInfo = {0};

	memset(&tmpInfo,0,sizeof(AudioInfo_S));
	count = P3K_PhraserParam(reqparam, strlen(reqparam),str);
	P3K_GetAudioSInfo(str,&tmp,0);

	ret =  EX_GetAutoSwitchPriority(&tmp,count);
	strncat(tmpparam,str[0],strlen(str[0]));
	strncat(tmpparam,aStr,strlen(aStr));
	for(num = 1;num<ret;num++)
	{
		char dir[16] = {0};
		char port[16] = {0};
		char signal[16] = {0};

		P3K_PortDirectionToStr(tmp[num].direction,dir);
		P3K_PortFormatToStr(tmp[num].portFormat,port);
		P3K_SignaleTypeToStr(tmp[num].signal,signal);
		sprintf(aStr2,"%s.%s.%d.%s,",dir,port,tmp[num].portIndex,signal);
		strncat(tmpparam,aStr2,strlen(aStr2));

		printf("num:%d aStr2:%s tmpparam:%s \n",num,aStr2,tmpparam);
		memset(aStr2,0,sizeof(aStr2));
		//memset(tmp,0,sizeof(tmp));
	}

	if(num == ret)
	{
		char dir[16] = {0};
		char port[16] = {0};
		char signal[16] = {0};

		P3K_PortDirectionToStr(tmp[num].direction,dir);
		P3K_PortFormatToStr(tmp[num].portFormat,port);
		P3K_SignaleTypeToStr(tmp[num].signal,signal);
		sprintf(aStr2,"%s.%s.%d.%s]",dir,port,tmp[num].portIndex,signal);
		strncat(tmpparam,aStr2,strlen(aStr2));
		printf("num:%d aStr2:%s tmpparam:%s \n",num,aStr2,tmpparam);
		memset(aStr2,0,sizeof(aStr2));

	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return  0;

}

static int P3K_SetEDIDMode(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-MODE   Input_id, Mode, Index<CR>
	//~nn@#EDID-MODE   Input_id, Mode, Index<CR><LF>
	DBG_InfoMsg("P3K_SetEDIDMode\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	EdidInfo_S tmpInfo = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	tmpInfo.input_id=atoi(str[0]);
	tmpInfo.mode=P3K_CheckEdidMode(str[1]);
	tmpInfo.index=atoi(str[2]);
	s32Ret = EX_SetEDIDMode(&tmpInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetEDIDMode err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetEDIDMode(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-MODE?  Input_id<CR>
	//~nn@#EDID-MODE   Input_id, Mode, Index<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDMode\n");
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	int s32Ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	EdidInfo_S tmpInfo = {0};
	int cmdID = 0;
	char parity[32] = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	cmdID = atoi(str[0]);

	s32Ret = EX_GetEDIDMode(cmdID,&tmpInfo);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetEDIDMode err\n");
	}
	P3K_ModeToStr(tmpInfo.mode,parity);
	if(tmpInfo.mode == CUSTOM)
		sprintf(tmpparam+strlen(tmpparam),"%d,%s,%d",tmpInfo.input_id,parity,tmpInfo.index);
	else
		sprintf(tmpparam+strlen(tmpparam),"%d,%s",tmpInfo.input_id,parity);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetEDIDList(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-LIST? <CR>
	//~nn@#EDID-LIST [0,��DEFAULT��],...<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDList\n");
	int ret = 0;
	int i = 0;
	char edidlist[8][MAX_EDID_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetEdidList(edidlist,8);
	if(ret > 10)
	{
		DBG_WarnMsg("P3K_GetEDIDList num=%d over 10\n",ret);
		ret =10;
	}
	for(i = 0;i<ret ;i++)
	{
		strncat(tmpparam,edidlist[i],MAX_EDID_LEN);
		if(i< (ret-1))
		{
			strncat(tmpparam,",",1);

		}
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_AddEDID(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-ADD Index,"Name" <CR>
	//~nn@#EDID-ADD Index,"Name"<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDList\n");
	int cmdID = 0;
	int s32Ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	EdidName_S edidname = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	edidname.index = atoi(str[0]);
	memcpy(edidname.name,str[1],strlen(str[1]));

	s32Ret = EX_AddEDID(&edidname);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_AddEDID err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_RemoveEDID(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-ADD Index <CR>
	//~nn@#EDID-RM Index<CR><LF>
	DBG_InfoMsg("P3K_RemoveEDID\n");
	int comID = 0;
	int s32Ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	EdidName_S edidname = {0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	comID = atoi(str[0]);

	s32Ret = EX_RemoveEDID(comID);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_RemoveEDID err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Activate specific EDID
static int P3K_SetActiveEDID(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-ACTIVE  Input_id, Index<CR>
	//~nn@EDID-ACTIVE  Input_id, Index<CR><LF>
	DBG_InfoMsg("P3K_SetActiveEDID\n");
	int input_ID = 0;
	int index_ID = 0;
	int s32Ret = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	input_ID = atoi(str[0]);
	index_ID = atoi(str[1]);

	s32Ret = EX_SetActiveEDID(input_ID,index_ID);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_RemoveEDID err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Get Current Activate EDID
static int P3K_GetActiveEDID(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-ACTIVE?  Input_id<CR>
	//~nn@EDID-ACTIVE  Input_id, Index<CR><LF>
	DBG_InfoMsg("P3K_SetActiveEDID\n");
	int input_ID = 0;
	int ret = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	input_ID = atoi(str[0]);

	ret = EX_GetActiveEDID(input_ID);
	sprintf(tmpparam,"%d,%d",input_ID,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set MAC on Net device to be EDID source
static int P3K_SetEDIDNetSrc(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-NET-SRC input_id, src_mac<CR>
	//~nn@EDID-NET-SRC input_id, src_mac<CR><LF>
	DBG_InfoMsg("P3K_SetEDIDNetSrc\n");
	int s32Ret = 0;
	int input_id = 0;
	int id = 0;
	int count = 0;
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	input_id = atoi(str[0]);
	s32Ret = EX_SetEDIDNetSrc(input_id,str[1]);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetMacAddr err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Get MAC on Net device to be EDID source
static int P3K_GetEDIDNetSrc(char*reqparam,char*respParam,char*userdata)//X
{
	//#EDID-NET-SRC input_id, src_mac<CR>
	//~nn@EDID-NET-SRC input_id, src_mac<CR><LF>
	DBG_InfoMsg("P3K_GetEDIDNetSrc\n");
	int netid = 0;
	int count = 0;
	int s32Ret = 0;
	char mac[MAC_ADDR_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	netid= atoi(str[0]);
	s32Ret = EX_GetEDIDNetSrc(netid,mac);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetMacAddr err\n");
	}
	sprintf(tmpparam,"%d,%s",netid,mac);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Get Control Gateway Messages Counter from certain period
static int P3K_GetRecvMsgNum(char*reqparam,char*respParam,char*userdata)
{
	//#GTW-MSG-NUM? mmessage_type,data<CR>
	//~nn@GTW-MSG-NUM message_type,date,counter<CR><LF>
	DBG_InfoMsg("P3K_GetRecvMsgNum\n");
	int count = 0;
	int ret = 0;
	int msg_type = 0;
	char pdate[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	msg_type = atoi(str[0]);
	memcpy(pdate,str[1],strlen(str[1]));

	ret =  EX_GetRecvMsgNum(msg_type,pdate);
	sprintf(tmpparam,"%d,%s,%d",msg_type,pdate,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Get SEND Control Gateway Messages Counter from certain period
static int P3K_GetSendMsgNum(char*reqparam,char*respParam,char*userdata)
{
	//#GTW-MSG-NUM-SEND? mmessage_type,data<CR>
	//~nn@GTW-MSG-NUM-SEND message_type,date,counter<CR><LF>
	DBG_InfoMsg("P3K_GetSendMsgNum\n");
	int count = 0;
	int ret = 0;
	int msg_type = 0;
	char pdate[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	msg_type = atoi(str[0]);
	memcpy(pdate,str[1],strlen(str[1]));

	ret =  EX_GetSendMsgNum(msg_type,pdate);
	sprintf(tmpparam,"%d,%s,%d",msg_type,pdate,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}


static int P3K_GetVidOutRatio(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-RATIO? <CR>
	//~nn@KDS-RATIO value<CR><LF>
	DBG_InfoMsg("P3K_GetVidOutRatio\n");
	int s32Ret = 0;
	char value[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret =  EX_GetVidOutRatio(value);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVidOutRatio err\n");
	}
	sprintf(tmpparam,"%s",value);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set Encoder AV channel Name.
static int P3K_SetChannelName(char*reqparam,char*respParam,char*userdata)//P3K_SetDanteName
{
	//#KDS-DEFINE-Name xxxx<CR>
	//~nn@KDS-DEFINE-NAME xxxx<CR><LF>
	DBG_InfoMsg("P3K_SetChannelName\n");
	int s32Ret = 0;
	int count = 0;
	char aName[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(aName,str[0],strlen(str[0]));

	s32Ret =  EX_SetChannelName(aName);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetChannelName err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Set Encoder Dante Hostl Name.
static int P3K_SetDanteName(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-DANTE-NAME  xxxx<CR>
	//~nn@KDS-DANTE-NAME  xxxx<CR><LF>
	DBG_InfoMsg("P3K_SetDanteName\n");
	int s32Ret = 0;
	int count = 0;
	char aName[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(aName,str[0],strlen(str[0]));

	s32Ret =  EX_SetDanteName(aName);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetDanteName err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetChannelName(char*reqparam,char*respParam,char*userdata)//
{
	//#KDS-DEFINE-Name?<CR>
	//~nn@KDS-DEFINE-NAME xxxx<CR><LF>
	DBG_InfoMsg("P3K_GetChannelName\n");
	int s32Ret = 0;
	char aName[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret =  EX_GetChannelName(aName);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetChannelName err\n");
	}
	sprintf(tmpparam,"%s",aName);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetDanteName(char*reqparam,char*respParam,char*userdata)//
{
	//#KDS-DANTE-NAME ? <CR>
	//~nn@KDS-DANTE-NAME  xxxx<CR><LF>
	DBG_InfoMsg("P3K_GetDanteName\n");
	int s32Ret = 0;
	char aName[32]= {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret =  EX_GetDanteName(aName);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetDanteName err\n");
	}
	sprintf(tmpparam,"%s",aName);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetMuteInfo(char*param,MuteInfo_S * info)
{
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;

	int count = P3K_PhraserWithSeparator('.',param, strlen(param), str);
		info->direction  = P3K_CheckPortDirection(str[0]);
		info->portFormat = P3K_CheckPortFormat(str[1]);
		info->portIndex = atoi(str[2]);
		info->signal = P3K_CheckSignalType(str[3]);
		info->index = atoi(str[4]);

	return 0;
}

//Set action to mute/unmute by encoder/decoder
static int P3K_SetAudMute(char*reqparam,char*respParam,char*userdata)
{
	//#X-MUTE <direction_type>. <port_format>. <port_index> .<signal_type>.<index>,state<CR>
	//~nn@ X-MUTE <direction_type>. <port_format>. <port_index> .<signal_type>.<index>,state <CR><LF>
	DBG_InfoMsg("P3K_SetVidMute\n");
	int s32Ret = 0;
	int count = 0;
	MuteInfo_S sMute = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	P3K_SetMuteInfo(str[0],&sMute);
	sMute.state = P3K_CheckStateType(str[1]);
	s32Ret =  EX_SetVidMute(&sMute);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVidMute err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetAudMute(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-AUDIO-MUTE?<CR>
	//~nn@KDS-AUDIO-MUTE mute_mode<CR><LF>
	DBG_InfoMsg("P3K_GetChannelName\n");
	int ret = 0;
	int count = 0;
	char aState[6] = {0};
	MuteInfo_S sMute = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	P3K_SetMuteInfo(str,&sMute);
	ret =  EX_GetVidMute(&sMute);
	P3K_StateToStr(sMute.state,aState);
	sprintf(tmpparam,"%s,%s",reqparam,aState);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetConnectionList(char*reqparam,char*respParam,char*userdata)
{
	//#CONNECTION-LIST? <CR>
	//~nn@CONNECTION-LIST [ip_string,port_type,client_port,device_port],...<CR><LF>
	DBG_InfoMsg("P3K_GetConnectionList\n");
	int ret = 0;
	char connectionlist[10][MAX_SIGNALE_LEN] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	int i = 0;
	ret = EX_GetConnectionList(&connectionlist[0],10);
	if(ret > 10)
	{
		DBG_WarnMsg("P3K_GetConnectionList num=%d over 10\n",ret);
		ret =10;
	}
	for(i = 0;i<ret ;i++)
	{
		printf(" ret = %d ,sig=%s\n",ret,connectionlist[i]);
		strncat(tmpparam,connectionlist[i],MAX_SIGNALE_LEN);
		if(i <  (ret-1))
		{
			strncat(tmpparam,",",1);

		}
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetStandbyVersion(char*reqparam,char*respParam,char*userdata)
{
	//#STANDBY-VERSION? ? <CR>
	//~nn@STANDBY-VERSION?  standby_version<CR><LF>
	DBG_InfoMsg("P3K_GetStandbyVersion\n");
	int s32Ret = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetStandbyVersion(aVersion);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetStandbyVersion err\n");
	}
	sprintf(tmpparam,"%s",aVersion);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Get hardware version number.
static int P3K_GetHWVersion(char*reqparam,char*respParam,char*userdata)
{
	//#HW-VERSION? <CR>
	//~nn@HW-VERSION hardware_version<CR><LF>
	DBG_InfoMsg("P3K_GetHWVersion\n");
	int s32Ret = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetHWVersion(aVersion);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetHWVersion err\n");
	}
	sprintf(tmpparam,"%s",aVersion);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Get Device Status.
static int P3K_GetDevStatus(char*reqparam,char*respParam,char*userdata)
{
	//#DEV-STATUS? <CR>
	//~nn@DEV-STATUS 0<CR><LF>
	DBG_InfoMsg("P3K_GetDevStatus\n");
	int ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	ret = EX_GetDevStatus();
	sprintf(tmpparam,"%d",ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetHWTemp(char*reqparam,char*respParam,char*userdata)
{
	//#HW-TEMP? region_id <CR>
	//~nn@HW?TEMP region_id, temperature<CR><LF>
	DBG_InfoMsg("P3K_GetHWTemp\n");
	int ret = 0;
	int region_id = 0;
	int count = 0;
	int iMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	region_id = atoi(str[0]);
	iMode = atoi(str[1]);
	ret = EX_GetHWTemp(region_id,iMode);
	sprintf(tmpparam,"%d,%d",region_id,ret);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Firmware Version Last Upgrade Date/Time.
static int P3K_GetUPGTime(char*reqparam,char*respParam,char*userdata)
{
	//#UPG-TIME? <CR>
	//~nn@UPG-TIME date,data<CR><LF>
	DBG_InfoMsg("P3K_GetUPGTime\n");
	int s32Ret = 0;
	char aDay[16] = {0};
	char aTime[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetUPGTime(aDay,aTime);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetUPGTime err\n");
	}
	sprintf(tmpparam,"%s,%s",aDay,aTime);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetAudOutput(char*reqparam,char*respParam,char*userdata)
{
	//#KDS_AUD_OUTPUT  [ list of Audio output]<CR>
	//~nn@KDS-AUD_OUTPUT [ list of Audio output]<CR><LF>
	DBG_InfoMsg("P3K_SetAudOutput\n");
	int s32Ret = 0;
	int count = 0;
	int mute_mode = 0;
	int aud[MAX_PARAM_COUNT] = {0};
	char aStr1[64] = {0};
	char aStr2[64] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	sscanf(reqparam,"[%s",aStr1);
	memcpy(aStr2,aStr1,(strlen(aStr1)-1));
	count = P3K_PhraserParam(aStr2,strlen(aStr2),str);
	s32Ret =  EX_SetVidOutput(str,count);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVidOutput err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Get Decoder Audio output list.
static int P3K_GetAudOutput(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-AUD_OUTPUTE ? <CR>
	//~nn@KDS-AUD_OUTPUT [ list of Audio output]<CR><LF>
	DBG_InfoMsg("P3K_GetAudOutput\n");
	int s32Ret = 0;
	char tmp[32] = {0};
	char aOutput[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetVidOutput(tmp);
	sprintf(aOutput,"[%s]",tmp);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetVidOutput err\n");
	}
	sprintf(tmpparam," %s",aOutput);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set Decoder OSD Display
static int P3K_SetOsdDisplay(char*reqparam,char*respParam,char*userdata)
{
	//#KDS_OSD_DISPLAY  mode<CR>
	//~nn@KDS-OSD_DISPLAY mode<CR><LF>
	DBG_InfoMsg("P3K_SetOsdDisplay\n");
	int s32Ret = 0;
	int count = 0;
	int iOsdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iOsdmode = atoi(str[0]);

	s32Ret =  EX_SetOsdDisplay(iOsdmode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetOsdDisplay err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetOsdDisplay(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-OSD_DISPLAY ? <CR>
	//~nn@KDS-OSD_DISPLAYmode<CR><LF>
	DBG_InfoMsg("P3K_GetOsdDisplay\n");
	int s32Ret = 0;
	int osdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	osdmode = EX_GetOsdDisplay();
	sprintf(tmpparam,"%d",osdmode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}
//Set Daisy Chain.
static int P3K_SetDaisyChain(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-DAISY-CHAIN daisy_state<CR>
	//~nn@KDS-DAISY-CHAIN daisy_state<CR><LF>
	DBG_InfoMsg("P3K_SetOsdDisplay\n");
	int s32Ret = 0;
	int count = 0;
	int daisy_state = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	daisy_state = atoi(str[0]);

	s32Ret =  EX_SetDaisyChain(daisy_state);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetOsdDisplay err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetDaisyChain(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-DAISY-CHAIN?<CR>
	//~nn@KDS-DAISY-CHAIN daisy_state<CR><LF>
	DBG_InfoMsg("P3K_GetDaisyChain\n");
	int s32Ret = 0;
	int osdmode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	osdmode = EX_GetDaisyChain();
	sprintf(tmpparam,"%d",osdmode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set unicast / multicast.
static int P3K_SetMethod(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-METHOD method<CR>
	//~nn@KDS-METHOD  method<CR><LF>
	DBG_InfoMsg("P3K_SetMethod\n");
	int s32Ret = 0;
	int count = 0;
	int method = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	method = atoi(str[0]);

	s32Ret =  EX_SetMethod(method);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetMethod err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetMethod(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-METHOD? <CR>
	//~nn@KDS-METHOD method<CR><LF>
	DBG_InfoMsg("P3K_GetMethod\n");
	int s32Ret = 0;
	int method = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	method = EX_GetMethod();
	sprintf(tmpparam,"%d",method);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set Inactivity auto-logout time
static int P3K_SetTimeOut(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-LOGOUT-TIMEOUT time<CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetTimeOut\n");
	int s32Ret = 0;
	int count = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iTime = atoi(str[0]);

	s32Ret =  EX_SetTimeOut(iTime);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetTimeOut err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetTimeOut(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetTimeOut\n");
	int s32Ret = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iTime = EX_GetTimeOut();
	sprintf(tmpparam,"%d",iTime);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Set Inactivity auto-logout time
static int P3K_SetStandbyTimeOut(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-LOGOUT-TIMEOUT time<CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetStandbyTimeOut\n");
	int s32Ret = 0;
	int count = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iTime = atoi(str[0]);

	s32Ret =  EX_SetStandbyTimeOut(iTime);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetTimeOut err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetStandbyTimeOut(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_SetStandbyTimeOut\n");
	int s32Ret = 0;
	int iTime = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iTime = EX_GetStandbyTimeOut();
	sprintf(tmpparam,"%d",iTime);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}



//Set multicast group address and TTL value.
static int P3K_SetMulticastStatus(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-MULTICAST  group_ip,ttl <CR>
	//~nn@KDS-ACTION group_ip,ttl<CR><LF>
	DBG_InfoMsg("P3K_SetMulticastStatus\n");
	int s32Ret = 0;
	int count = 0;
	int iTtl = 0;
	char ip[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(ip,str[0],strlen(str[0]));
	iTtl = atoi(str[1]);

	s32Ret =  EX_SetMulticastStatus(ip,iTtl);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetMulticastStatus err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

//Set gateway network port
static int P3K_SetGatewayPort(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-GW-ETH gw_type,netw_id<CR>
	//~nn@KDS-GW-ETH gw_type,netw_id<CR><LF>
	DBG_InfoMsg("P3K_SetGatewayPort\n");
	int s32Ret = 0;
	int count = 0;
	int iGw_Type = 0;
	int iNetw_Id = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iGw_Type = atoi(str[0]);
	iNetw_Id = atoi(str[1]);

	s32Ret =  EX_SetGatewayPort(iGw_Type,iNetw_Id);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetGatewayPort err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetGatewayPort(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-GW-ETH? gw_type<CR>
	//~nn@KDS-GW-ETH gw_type,netw_id<CR><LF>
	DBG_InfoMsg("P3K_GetGatewayPort\n");
	int s32Ret = 0;
	int iGw_Type = 0;
	int iNetw_Id = 0;
	int count = 0;
	char aVersion[24] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iGw_Type = atoi(str[0]);

	iNetw_Id = EX_GetGatewayPort(iGw_Type);
	sprintf(tmpparam,"%d,%d",iGw_Type,iNetw_Id);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetVlanTag(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-VLAN-TAG gw_type,xxxx<CR>
	//~nn@KDS-VLAN-TAG gw_type,xxxx<CR><LF>
	DBG_InfoMsg("P3K_SetVlanTag\n");
	int s32Ret = 0;
	int count = 0;
	int iGw_Type = 0;
	int iTag = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iGw_Type = atoi(str[0]);
	iTag = atoi(str[1]);

	s32Ret =  EX_SetVlanTag(iGw_Type,iTag);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetVlanTag err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetVlanTag(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-VLAN-TAG? gw_type<CR>
	//~nn@KDS-VLAN-TAG gw_type,xxxx<CR><LF>
	DBG_InfoMsg("P3K_GetVlanTag\n");
	int s32Ret = 0;
	int iGw_Type = 0;
	int iTag = 0;
	int count = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iGw_Type = atoi(str[0]);

	iTag = EX_GetVlanTag(iGw_Type);
	sprintf(tmpparam,"%d,%d",iGw_Type,iTag);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//Reset password
static int P3K_SetPassword(char*reqparam,char*respParam,char*userdata)
{
	//#PASS old_pass,new_pass <CR>
	//~nn@PASS old_pass,new_pass <CR><LF>
	//or
	//~nn@PASS err 004<CR><LF>
	//(if bad old password entered)
	DBG_InfoMsg("P3K_SetPassword\n");
	int ret = 0;
	int count = 0;
	char login_level[12] = {0};
	char iNew_Pass[12] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	//iOld_Pass = atoi(str[0]);
	//iNew_Pass = atoi(str[1]);
	memcpy(login_level,str[0],strlen(str[0]));
	memcpy(iNew_Pass,str[1],strlen(str[1]));

	ret =  EX_SetPassword(login_level,iNew_Pass);
	sprintf(tmpparam,"%s,%s",login_level,iNew_Pass);
	if(ret != 0)
	{
		memset(tmpparam,0,sizeof(tmpparam));
		sprintf(tmpparam,"%s","err_004");
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetPassword(char*reqparam,char*respParam,char*userdata)
{
	//#PASS? login_level<CR>
	//~nn@PASS login_level password<CR><LF>
	DBG_InfoMsg("P3K_GetPassword\n");
	int s32Ret = 0;
	int count =0;
	char aLogin_level[16] = {0};
	int iPassWord = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};
	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(aLogin_level,str[0],strlen(str[0]));
	iPassWord = EX_GetPassword(aLogin_level);
	if(iPassWord)
	{
		sprintf(tmpparam,"%s,%d",aLogin_level,iPassWord);
	}
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetRollback(char*reqparam,char*respParam,char*userdata)
{
	//#ROLLBACK<CR>
	//~nn@ROLLBACK ok<CR><LF>
	DBG_InfoMsg("P3K_SetRollback\n");
	int ret = 0;
	int count = 0;
	char type[16] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	ret =  EX_SetRollback(type);
	if(ret)
	{
		DBG_ErrMsg("EX_SetRollback err\n");
	}
	sprintf(tmpparam,"%s",type);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetLogResetEvent(char*reqparam,char*respParam,char*userdata)
{
	//#LOG-RESET?<CR>
	//~nn@LOG-RESET log_type,date,time<CR><LF>
	DBG_InfoMsg("P3K_GetLogResetEvent\n");
	int s32Ret = 0;
	char weekDay[16] = {0};
	int iLog_Type = 0;
	char date[32]= {0};
	char time[32] = {0};
	char tmpparam[MAX_PARAM_LEN] = {0};

	s32Ret = EX_GetLogResetEvent(&iLog_Type, date,time);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_GetLogResetEvent err\n");
	}
	sprintf(tmpparam,"%d,%s,%s",iLog_Type,date,time);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

//set tr gateway mode
static int P3K_SetIRGateway(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-IR-GW mode<CR>
	//~nn@KDS-IR-GW mode<CR><LF>
	DBG_InfoMsg("P3K_SetIRGateway\n");
	int ret = 0;
	int count = 0;
	int iGW_mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iGW_mode = atoi(str[0]);

	ret =  EX_SetIRGateway(iGW_mode);
	if(ret)
	{
		DBG_ErrMsg("EX_SetIRGateway err\n");
	}
	//sprintf(tmpparam,"%s",type);
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetIRGateway(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-IR-GW?<CR>
	//~nn@KDS-IR-GW mode<CR><LF>
	DBG_InfoMsg("P3K_GetIRGateway\n");
	int s32Ret = 0;
	int iIr_mode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};

	iIr_mode = EX_GetIRGateway();
	sprintf(tmpparam,"%d",iIr_mode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_RmEDID(char*reqparam,char*respParam,char*userdata)
{
	//#EDID-RM Index <CR>
	//~nn@#EDID-RM Index<CR><LF>
	//or
	//~nn@EDID-RM err<CR><LF>
	DBG_InfoMsg("P3K_RmEDID\n");
	int ret = 0;
	int count = 0;
	int iEDID = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	iEDID = atoi(str[0]);
	sprintf(tmpparam,"%d",iEDID);
	ret =  EX_RemoveEDID(iEDID);//EX_RmEDID(iEDID);
	if(ret != 0)
	{
		memset(tmpparam,0,sizeof(tmpparam));
		sprintf(tmpparam,"%s","err");
	}

	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetVideoWallStretch(char*reqparam,char*respParam,char*userdata)
{
	//#WND-STRETCH out_index,mode<CR>
	//~nn@WND-STRETCH out_index,mode<CR><LF>
	DBG_InfoMsg("P3K_SetVideoWallStretch\n");
	int s32Ret = 0;
	int count = 0;
	int index = 0;
	int iMode = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	index = atoi(str[0]);
	iMode = atoi(str[1]);
	s32Ret =  EX_SetVideoWallStretch(index,iMode);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_SetTimeOut err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}
static int P3K_GetVideoWallStretch(char*reqparam,char*respParam,char*userdata)
{
	//#KDS-LOGOUT-TIMEOUT? <CR>
	//~nn@KDS-LOGOUT-TIMEOUT time<CR><LF>
	DBG_InfoMsg("P3K_GetVideoWallStretch\n");
	int s32Ret = 0;
	int iMode = 0;
	int count = 0;
	int index = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	index = atoi(str[0]);
	iMode = EX_GetVideoWallStretch(index);
	sprintf(tmpparam,"%d,%d",index,iMode);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_SetCfgModify(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_SetCfgModify\n");
	int ret = 0;
	ret =  EX_SetCfgModify(reqparam);
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_Discovery(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_Discovery\n");
	int count = 0;
	char iIP[24] = "";
	int iport = 0;
	int u32ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(iIP,str[0],strlen(str[0]));
    iport = atoi(str[1]);
	u32ret = EX_Discovery(iIP,iport);
	return 0;
}
static int P3K_BEACON(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_BEACON\n");
	int count = 0;
	int iport_id = 0;
    int istatus = 0;
    int irate = 0;
	int u32ret = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
    iport_id = atoi(str[0]);
    istatus = atoi(str[1]);
    irate = atoi(str[2]);
    printf("[%d,%d,%d]",iport_id,istatus,irate);
	u32ret = EX_Beacon(iport_id,istatus,irate);
    if(u32ret)
	{
		DBG_ErrMsg("EX_Beacon err\n");
	}
    memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_ConfBeaconInfo(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_ConfBeaconInfo\n");
	int s32Ret = 0;
	int count = 0;
	char muticastIP[64] = "";
	int port = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	count = P3K_PhraserParam(reqparam,strlen(reqparam),str);
	memcpy(muticastIP,str[0],strlen(str[0]));
	port = atoi(str[1]);
    printf("[%s,%d]\n",muticastIP,port);
	s32Ret =  EX_ConfBeaconInfo(muticastIP,port);
	if(s32Ret)
	{
		DBG_ErrMsg("EX_ConfBeaconInfo err\n");
	}
	memcpy(respParam,reqparam,strlen(reqparam));
	return 0;
}

static int P3K_GetBeaconConf(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetBeaconConf\n");
	int s32Ret = 0;
	char iMode = 0;
	char muticastIP[64] = "";
	int port = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};


	s32Ret = EX_GetBeaconConf(muticastIP,&port);
    if(s32Ret)
	{
		DBG_ErrMsg("EX_GetBeaconConf err\n");
	}
	sprintf(tmpparam,"%s,%d",muticastIP,port);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}

static int P3K_GetBEACON(char*reqparam,char*respParam,char*userdata)
{
	DBG_InfoMsg("P3K_GetBEACON\n");
	int s32Ret = 0;
	int port = 0;
	int status = 0;
    int rate = 0;
	char tmpparam[MAX_PARAM_LEN] = {0};
	char str[MAX_PARAM_COUNT][MAX_PARAM_LEN] ={0};

	s32Ret = EX_GetBeacon(&port,&status,&rate);
    if(s32Ret)
	{
		DBG_ErrMsg("EX_GetBeacon err\n");
	}
	sprintf(tmpparam,"%d,%d,%d",port,status,rate);
	memcpy(respParam,tmpparam,strlen(tmpparam));
	return 0;
}


/*   P3K_SetIRGateway  */
/*P3K_GetBEACON
int P3K_SilmpleSpecReqCmdProcess(P3K_SimpleSpecCmdInfo_S * cmdreq, P3K_SimpleSpecCmdInfo_S * cmdresp)
{
	static P3K_SpecPhraserToExecute_S cliSpecialFunc[] = {

										      {"LDFW",NULL},

										      	{NULL,NULL}

	};
	if(cmdreq == NULL || cmdresp == NULL)
	{
		return -1;
	}
}
*/
int P3K_SilmpleReqCmdProcess(P3K_SimpleCmdInfo_S *cmdreq,P3K_SimpleCmdInfo_S *cmdresp,char*userdata)
{
	static P3K_PhraserToExecute_S cliFunc[]={

									{"KDS-AUD",P3K_SetAudioInputMode},
									{"KDS-AUD?",P3K_GetAudioInputMode},
									{"X-AUD-LVL",P3K_SetAudLevel},
									{"X-AUD-LVL?",P3K_GetAudLevel},
									{"X-AUD-DESC?",P3K_GetAudParam},
									{"X-AV-SW-MODE",P3K_SetAutoSwitchMode},
									{"X-AV-SW-MODE?",P3K_GetAutoSwitchMode},
									{"GEDID",P3K_GetEdid},
									{"CPEDID",P3K_CopyEdid},
									{"EDID-CS",P3K_SetEdidCsMode},
									{"EDID-CS?",P3K_GetEdidCsMode},
									{"LOCK-EDID",P3K_SetEdidLockMode},
									{"LOCK-EDID?",P3K_GetEdidLockMode},
									{"HDCP-MOD",P3K_SetHDCPMode},
									{"HDCP-MOD?",P3K_GetHDCPMode},
									{"HDCP-STAT?",P3K_GetHDCPStatus},
									{"VIEW-MOD",P3K_SetVideoWallMode},
									{"VIEW-MOD?",P3K_GetVideoWallMode},
									{"WND-BEZEL",P3K_SetWndBezel},
									{"WND-BEZEL?",P3K_GetWndBezel},
									{"VIDEO-WALL-SETUP",P3K_SetVideoWallRotaion},
									{"VIDEO-WALL-SETUP?",P3K_GetVideoWallRotaion},
									{"KDS-START-OVERLAY",P3K_StartOverlay},
									{"KDS-STOP-OVERLAY",P3K_StopOverlay},
									{"KDS-DEFINE-CHANNEL",P3K_SetChannelId},
									{"KDS-DEFINE-CHANNEL?",P3K_GetChannelId},
									{"KDS-CHANNEL-SELECT",P3K_SetChannleSelection},
									{"KDS-CHANNEL-SELECT?",P3K_GetChannleSelection},
									{"IMAGE-PROP",P3K_SetVideoOutMode},
									{"IMAGE-PROP?",P3K_GetVideoOutMode},
									{"KDS-ACTION",P3K_SetVideoAction},
									{"KDS-ACTION?",P3K_GetVideoAction},
									{"CS-CONVERT",P3K_SetVideoOutCS},
									{"CS-CONVERT?",P3K_GetVideoOutCS},
									{"KDS-SCALE",P3K_SetVideoOutScaler},
									{"KDS-SCALE?",P3K_GetVideoOutScaler},
									{"KDS-RESOL?",P3K_GetVideoReslotion},
									{"KDS-FR?",P3K_GetVideoFrameRate},
									{"KDS-BR?",P3K_GetVideoBitRate},
									{"CEC-SND",P3K_SendCECMsg},
									{"CEC-NTFY",P3K_RecvCECNtfy},
									{"CEC-GW-PORT-ACTIVE",P3K_SetCECGWMode},
									{"CEC-GW-PORT-ACTIVE?",P3K_GetCECGWMode},
									{"IR-SND",P3K_SendIRMsg},
									{"IR-STOP",P3K_StopIRMsg},
									{"X-ROUTE",P3K_SetXROUTEMatch},
									{"X-ROUTE?",P3K_GetXROUTEMatch},
									{"UART",P3K_SetUartConf},
									{"UART?",P3K_GetUartConf},
									{"COM-ROUTE-ADD",P3K_AddComRoute},
									{"COM-ROUTE-REMOVE",P3K_RemoveComRoute},
									{"COM-ROUTE?",P3K_GetComRouteInfo},
									{"ETH-TUNNEL?",P3K_GetEthTunnel},
									{"KVM-USB-CTRL",P3K_SetUSBMode},
									{"KDS-MULTICAST?",P3K_GetMulticastStatus},
									{"FCT-MAC",P3K_SetMacAddr},
									{"NET-MAC?",P3K_GetMacAddr},
									{"NAME-RST",P3K_ResetDNSName},
									{"NET-DHCP",P3K_SetDHCPMode},
									{"NET-DHCP?",P3K_GetDHCPMode},
									{"NET-CONFIG",P3K_SetNetConf},
									{"NET-CONFIG?",P3K_GetNetConf},
									{"ETH-PORT",P3K_SetEthPort},
									{"ETH-PORT?",P3K_GetEthPort},
									{"SECUR",P3K_SetSecurity},
									{"LOGIN",P3K_SetLogin},
									{"LOGIN?",P3K_GetLogin},
									{"LOGOUT",P3K_LogOut},
									{"VERSION?",P3K_GetVersion},
									{"UPGRADE",P3K_Upgrade},
									{"UPGRADE-STATUS?",P3K_UpgradeStatus},
									{"FCT-MODEL",P3K_SetFCTMODEL},
									{"MODEL?",P3K_GetFCTMODEL},
									{"FCT-SN",P3K_SetSerailNum},
									{"SN?",P3K_GetSerailNum},
									{"LOCK-FP",P3K_SetLockFP},
									{"LOCK-FP?",P3K_GetLockFP},
									{"IDV",P3K_SetIDV},
									{"STANDBY",P3K_SetStandByMode},
									{"STANDBY?",P3K_GetStandByMode},
									{"RESET",P3K_DoReset},
									{"FACTORY",P3K_DoFactory},
									{"BEACON-EN",P3K_BEACON},
									{"BEACON-EN?",P3K_GetBEACON},
									{"BEACON-INFO?",P3K_GetBeaconInfo},
									{"BUILD-DATE?",P3K_GetBuildTime},
									{"TIME",P3K_SetTime},
									{"TIME?",P3K_GetTime},
									{"TIME-LOC",P3K_SetTimeZero},
									{"TIME-LOC?",P3K_GetTimeZero},
									{"TIME-SRV",P3K_SetTimeServer},
									{"TIME-SRV?",P3K_GetTimeServer},
									{"SIGNALS-LIST?",P3K_GetSignalList},
									{"PORTS-LIST?",P3K_GetPortList},
									{"KDS-ACTIVE-CLNT?",P3K_GetActiveCli},
									{"LOG-ACTION",P3K_SetLogAction},
									{"LOG-ACTION?",P3K_GetLogAction},
									{"LOG-TAIL?",P3K_GetLogTail},
									{"PORT-DIRECTION",P3K_SetAudAnalogDir},
									{"PORT-DIRECTION?",P3K_GetAudAnalogDir},
									{"X-PRIORITY",P3K_SetAutoSwitchPriority},
									{"X-PRIORITY?",P3K_GetAutoSwitchPriority},
									{"EDID-MODE",P3K_SetEDIDMode},
									{"EDID-MODE?",P3K_GetEDIDMode},
									{"EDID-LIST?",P3K_GetEDIDList},
									{"EDID-ADD",P3K_AddEDID},
									{"EDID-ACTIVE",P3K_SetActiveEDID},
									{"EDID-ACTIVE?",P3K_GetActiveEDID},
									{"EDID-NET-SRC",P3K_SetEDIDNetSrc},
									{"EDID-NET-SRC?",P3K_GetEDIDNetSrc},
									{"GTW-MSG-NUM?",P3K_GetRecvMsgNum},
									{"GTW-MSG-NUM-SEND?",P3K_GetSendMsgNum},
									{"KDS-RATIO?",P3K_GetVidOutRatio},
									{"KDS-DEFINE-CHANNEL-NAME",P3K_SetChannelName},
									{"KDS-DEFINE-CHANNEL-NAME?",P3K_GetChannelName},
									{"NAME",P3K_SetDNSName},
									{"NAME?",P3K_GetDNSName},
									{"X-MUTE",P3K_SetAudMute},
									{"X-MUTE?",P3K_GetAudMute},
									{"NET-STAT?",P3K_GetConnectionList},
									{"STANDBY-VERSION?",P3K_GetStandbyVersion},
									{"HW-VERSION?",P3K_GetHWVersion},
									{"DEV-STATUS?",P3K_GetDevStatus},
									{"HW-TEMP?",P3K_GetHWTemp},
									{"KDS-AUD-OUTPUT",P3K_SetAudOutput},
									{"UPG-TIME?",P3K_GetUPGTime},
									{"KDS-AUD-OUTPUT?",P3K_GetAudOutput},
									{"KDS-OSD-DISPLAY",P3K_SetOsdDisplay},
									{"KDS-OSD-DISPLAY?",P3K_GetOsdDisplay},
									{"KDS-DAISY-CHAIN",P3K_SetDaisyChain},
									{"KDS-DAISY-CHAIN?",P3K_GetDaisyChain},
									{"KDS-METHOD",P3K_SetMethod},
									{"KDS-METHOD?",P3K_GetMethod},
									{"LOGOUT-TIMEOUT",P3K_SetTimeOut},
									{"LOGOUT-TIMEOUT?",P3K_GetTimeOut},
									{"KDS-MULTICAST",P3K_SetMulticastStatus},
									{"KDS-GW-ETH",P3K_SetGatewayPort},
									{"KDS-GW-ETH?",P3K_GetGatewayPort},
									{"KDS-VLAN-TAG",P3K_SetVlanTag},
									{"KDS-VLAN-TAG?",P3K_GetVlanTag},
									{"PASS",P3K_SetPassword},
									{"PASS?",P3K_GetPassword},
									{"ROLLBACK",P3K_SetRollback},
									{"LOG-RESET?",P3K_GetLogResetEvent},
									{"KDS-IR-GW",P3K_SetIRGateway},
									{"KDS-IR-GW?",P3K_GetIRGateway},
									{"STANDBY-TIMEOUT",P3K_SetStandbyTimeOut},
									{"STANDBY-TIMEOUT?",P3K_GetStandbyTimeOut},
									{"EDID-RM",P3K_RmEDID},
									{"WND-STRETCH",P3K_SetVideoWallStretch},
									{"WND-STRETCH?",P3K_GetVideoWallStretch},
									{"KDS-CFG-MODIFY",P3K_SetCfgModify},
									{"UDPNET-CONFIG?",P3K_Discovery},
									{"BEACON-CONF",P3K_ConfBeaconInfo},
									{"BEACON-CONF?",P3K_GetBeaconConf},
									{NULL,NULL}
	};

	if(cmdreq == NULL || cmdresp == NULL)
	{
		DBG_WarnMsg("P3K cmdreq or cmd resp is NULL\n ");
		return -1;
	}
       int i = 0;
	for (i = 0; ;i++)
	{
		if(cliFunc[i].cmd== NULL)
		{
			DBG_WarnMsg("cmd[ %s ]does not support\n",cmdreq->command);
			memcpy(cmdresp->command,cmdreq->command,strlen(cmdreq->command));
			memcpy(cmdresp->param,"err001",strlen("err001"));
			return 0;
		}
		if(strcmp(cliFunc[i].cmd,cmdreq->command) == 0)
		{
			if(cliFunc[i].ParamPhraser != NULL)
			{
				memset(cmdresp->command,0,sizeof(cmdresp->command));
				memcpy(cmdresp->command,cmdreq->command,strlen(cmdreq->command));
				memset(cmdresp->param,0,sizeof(cmdresp->param));
				cliFunc[i].ParamPhraser(cmdreq->param,cmdresp->param,userdata);
				printf(">>>> %s %s \n",cmdreq->command,cmdresp->param);
			}
			break;
		}
	}
	return 0;
}


int P3K_CheckedSpeciCmd(char*cmd)
{
	static char *specCmd[] ={

							"LDFW",
							NULL,
	};

	int i = 0;
	for(i = 0;;i++)
	{
		if(specCmd[i] == NULL)
		{
			break;
		}
		if(strcmp(specCmd[i],cmd) == 0)
		{
			return 1;
		}
	}
	return 0;
}




/*
								{"NAME",P3K_SetDanteName},
									{"NAME?",P3K_GetDanteName},
*/

