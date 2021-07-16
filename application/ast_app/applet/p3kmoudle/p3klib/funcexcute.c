#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "funcexcute.h"

#include <sys/socket.h>
#include <netinet/in.h>

#define BUFSIZE 128
#define PORT 5588

int sendCmdtoGUI(const char *buf)
{
	char sendbuf[BUFSIZE] = {0};
	memcpy(sendbuf,buf,strlen(buf)+1);
	// 初始化
	int fd = socket(AF_INET,SOCK_DGRAM,0);	
	printf("fd: %d\n",fd);
	if(fd < 0)
	{
		printf("socket failed \n");
		return -1;
	}
	
	// 地址端口初始化
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

	close(fd);
	return mode;
}

int  EX_SetAudSrcMode(int mode)
{
	printf("EX_SetAudSrcMode mode =%d\n",mode);
	return 0;
}
int  EX_GetAudSrcMode(int *mode)
{
	int tmpmode = 0;
	*mode = tmpmode;
	return 0;
}

int EX_SetAudGainLevel(PortInfo_S*info,int gain)
{
	printf("gain =%d\n",gain);
	return 0;
}

int EX_SetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	printf("gain =%s\n",gain);
	return 0;
}

int EX_GetAudAnalogGainDir(AudioInfo_S*info,char * gain)
{
	char tmpGain[10] = {"IN"};
	memcpy(gain,tmpGain,strlen(tmpGain));
	return 0;
}

int EX_SetAutoSwitchPriority(AudioInfo_S * info,AudioInfo_S * gain,int count)
{
	pid_t status;
	int num = 1;
	char  cmd2[128] = {0};
	printf("%d,%d,%d,%d",info[1].direction,info[1].portFormat,info[1].portIndex,info[1].signal);
	sprintf(cmd2,"./sconfig --priority HDMI%d HDMI%d HDMI%d",info[2].portIndex,info[3].portIndex,info[4].portIndex);
	printf("%s\n",cmd2);
	status = system(cmd2);
	
	return 0;
}

int EX_GetAutoSwitchPriority(AudioInfo_S * gain,int count)
{
	pid_t status;
	int ret =0;
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

int EX_SetEDIDMode(EdidInfo_S *info)
{
	//printf("gain =%s\n",gain);
	printf(">>EX_SetEDIDMode %d,%d\n",info->input_id,info->index);
	return 0;
}

int EX_GetEDIDMode(int cmdID,EdidInfo_S * info)
{
 	EdidInfo_S tmpconf;
	info->input_id = cmdID;
	info->mode = 1;
	info->index = 1;
	//printf(">>EX_GetEDIDMode\n");
	return 0;
}

int EX_AddEDID(EdidName_S * info)
{
	printf(">>EX_AddEDID %d %s\n",info->index,info->name);
	return 0;
}

int EX_RemoveEDID(int comID)
{
	printf(">>EX_RemoveEDID %d \n",comID);
	return 0;
}

int EX_SetActiveEDID(int input_ID,int index_ID )
{
	printf(">>EX_SetActiveEDID %d,%d \n",input_ID,index_ID);
	return 0;
}

int EX_SetEDIDNetSrc(int input_ID,char*macAddr )
{
	printf(">>EX_SetEDIDNetSrc id=%d mac=%s \n",input_ID,macAddr);
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

int EX_SetVidMute(int mute )
{
	
	printf(">>EX_SetVidMute %d\n",mute);
	//0	– off for unmute  1	– on for mute

	return 0;
}


int EX_SetTimeOut(int  iTime )
{
	
	printf(">>EX_SetOsdDisplay %d\n",iTime);
	//time – minutes of logout time
	return 0;
}
int EX_GetTimeOut(void)
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

int EX_SetPassword(char * iOld_Pass,char * iNew_Pass)
{
	printf("iOld_Pass=%s\n",iOld_Pass);
	char * oldpass = "33333";
	if(0 == memcmp(iOld_Pass,oldpass,strlen(iOld_Pass)))
	{
		return 0;
	}
	printf(" oldpassword error\n");
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
	//osd mode {0	– off,1	– on,2	– display now,}
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
	//osd mode {0	– off,1	– on,2	– display now,}
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
	//0	– OFF (disables dainsy chain)
	//11	– ON (enables dainsy chain)
	return 0;
}
int EX_GetDaisyChain(void)
{
	
	int daisy_state =1;

	return daisy_state;
}

int EX_SetWndStretch(int  mode )
{
	
	printf(">>SetWndStretch %d\n",mode);
	//mode - Strech mode:
			//0 = fit in
			//11= fit out

	return 0;
}

int EX_GetWndStretch(int *mode )
{
	
	mode =0;

	return 0;
}

int EX_GetVidMute(void)
{
	int mute_mode = 0;
	return mute_mode;
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
//Audio output {0	– HDMI  1	– Analog  2	– Stream,3	– Dante}
	char * str = "1,2,3";
	memcpy(date,str,strlen(str));
	return 0;
}

int EX_GetHWVersion(char * date)
{
	char * version = "1.12.123";
	memcpy(date,version,strlen(version));
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
int EX_GetHWTemp(int  id)
{
	int iTemp = 50;
	
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
	pid_t status;
	char * cmd2 = "./sconfig --mode manual";
	status = system(cmd2);
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
	printf("filename = %s, outtime =%d\n",confFile,outtime);
	return 0;
}
int EX_StopOverlay(void)
{
	const char stopOverlay[BUFSIZE] = {"STOP_OVERLAY"};
	
	int mode = sendCmdtoGUI(stopOverlay);
	return 0;
}

int EX_SetEncoderAVChannelId(int id)
{
	printf("EX_SetEncoderAVChannelId id=%d\n",id);
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
	return 0;
}
int EX_GetColorSpaceConvertMode(int index,int *convertMode)
{
	*convertMode = 0;
	return 0;
}
int EX_SetVideoImageScaleMode(int mode,char*res)
{
	strcpy(res,"16");
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
	return 0;
}

int EX_GetCECGateWayMode(void)
{
	int mode = 0;
	return mode;
}

int EX_SendIRmessage(IRMessageInfo_S*info)
{
	printf("EX_SendIRmessage cmdname =%s cmdcomment =%s\n",info->cmdName,info->cmdName);
	return 0;
}
int EX_SendIRStop(int irId,int serialNumb,char*command)
{
	int status = 0;
	return status;
}
int EX_SetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo)
{

	printf("EX_SetRouteMatch %d   %d",inPortInfo->direction,inPortInfo->portFormat);
	return 0;
}

int EX_GetRouteMatch(PortInfo_S*inPortInfo,PortInfo_S*matchPortInfo)
{
	matchPortInfo->direction = 0;
	matchPortInfo->index = 1;
	matchPortInfo->portFormat = 1;
	matchPortInfo->signal = 1;
	matchPortInfo->portIndex = 1;
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

	strcpy(macAddr,"00-14-22-01-23-45");
	return 0;
}
int EX_SetDNSName(char*name)
{
	printf("EX_SetDNSName name =%s\n",name);
	return 0;
}
int EX_GetDNSName(char*name)
{
	strcpy(name,"room-1");
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
	char * tmp = "1.2.1";
	strcpy(version,tmp);
	return 0;
}
int EX_Upgrade(void)
{
	return 0;
}
int EX_SetDeviceNameModel(char*mod)
{
	printf("EX_SetDeviceNameModel %s\n",mod);
	return 0;
}
int EX_GetDeviceNameModel(char*mod)
{
	strcpy(mod,"dip-20");
	return 0;
}
int EX_SetSerialNumber(char*data)
{
	printf("EX_SetSerialNumber %s\n",data);
	return 0;
}
int EX_GetSerialNumber(char*data)
{
	strcpy(data,"12345678987654");
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
	time_t secTime;
	struct tm *ptime =NULL;
	secTime = time(NULL);
	ptime = localtime(&secTime);

	sprintf(date,"%04d/%02d/%02d",ptime->tm_year+1900,ptime->tm_mon+1,ptime->tm_mday);
	sprintf(hms,"%02d:%02d:%02d",ptime->tm_hour,ptime->tm_min,ptime->tm_sec);
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
	
	iLog = 1;
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
	int tmpnum = 3;
	char *str ="[192.168.0.1,0,5000,6001]";
	char *str1= "[192.168.0.2,0,5000,6002]";
	char *str2= "[192.168.0.3,0,5000,6002]";
	memcpy(info[0],str,strlen(str));
	memcpy(info[1],str1,strlen(str1));
	memcpy(info[2],str1,strlen(str2));
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

int EX_AutomaticReporting(char * info)
{
	//char * str = "~01@KDS-DANTE-NAME  KDS-LONG\r\n";
	char * str = "";
	memcpy(info,str,strlen(str));
	int iLength = 0;
	iLength = strlen(info);
	return iLength;
}



