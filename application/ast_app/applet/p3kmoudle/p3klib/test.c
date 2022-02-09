#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "p3kapi.h"

P3KApiHandle_S gs_p3khandle;
static int T_senddata(int handleId,char *data,int len)
{
	static int number = 0;
	number ++;
	printf("handle id=%d,data = [%s] \n",handleId,data);
	printf("totalresp  msg num=%d \n",number);
	
	return 0;
}
char *cmdStr[] ={
				"#KDS-AUD 1\r",
				"#KDS-AUD?\r",
				"#X-AUD-LVL in.analog_audio.1.audio.1,10\r",
				"#X-AUD-LVL? in.analog_audio.1.audio.1\r",
				"#X-AUD-DESC? out.hdmi.1\r",
				"#X-AV-SW-MODE out.hdmi.1.video.1,2\r",
			 	"#X-AV-SW-MODE? out.hdmi.1.video.1\r",
				//"#GEDID 0,1\r",
				//"#CPEDID 1,1,0,0x1\r",
				//"#EDID-CS in.hdmi.3.video.1,0\r",
				//"#EDID-CS? in.hdmi.3.video.1\r",
				"#LOCK-EDID 2,1\r",
				"#LOCK-EDID? 2\r",
				"#HDCP-MOD 1,0\r",
			 	 "#HDCP-MOD? 1\r",
				"#HDCP-STAT? 0,1\r",
				"#VIEW-MOD 15,2,2\r",
				"#VIEW-MOD?\r",
				"#WND-BEZEL 0,1,2,2,0,0\r",
				"#WND-BEZEL?\r",
				"#VIDEO-WALL-SETUP 10,1\r",
				"#VIDEO-WALL-SETUP?\r",
				"#KDS-START-OVERLAY overlay,0\r",
				"#KDS-STOP-OVERLAY\r",
				"#KDS-DEFINE-CHANNEL 1\r",
				"#KDS-DEFINE-CHANNEL?\r",
				"#KDS-CHANNEL-SELECT 1\r",
				"#KDS-CHANNEL-SELECT?\r",
				"#IMAGE-PROP 1,0\r",
				"#IMAGE-PROP? 1\r",
				"#KDS-ACTION 0\r",
				"#KDS-ACTION?\r",
				"#CS-CONVERT 1,1\r",
				"#CS-CONVERT? 1\r",
				"#KDS-SCALE 1\r",
				"#KDS-SCALE?\r",
				"#KDS-RESOL? 1,1,1\r",
				"#KDS-FR?\r",
				"#KDS-BR?\r",
				"#CEC-SND 1,1,1,2,E004\r",
				"#CEC-NTFY\r",
				"#CEC-GW-PORT-ACTIVE 1\r",
				"#IR-SND 1,1,1,1,1,1,1\r",
				"#IR-STOP 2,1,power\r",
				"#X-ROUTE out.hdmi.3.video.1,in.hdmi.2.video.1\r",
				"#X-ROUTE? out.hdmi.5.video.1\r",
				"#UART 1,9600,8,none,1.5\r",
				"#UART? 1\r",
				"#COM-ROUTE-ADD 1,1,50001,1,1\r",
				"#COM-ROUTE-REMOVE 1\r",
				"#KDS-MULTICAST?\r",
				"#FCT-MAC 00-14-22-01-23-45\r",
				"#NET-MAC? 0\r",
				"#NAME room-442\r",
				"#NAME?\r",
				"#NAME-RST\r",
				"#NET-DHCP 1,1\r",
				"#NET-DHCP? 1\r",
				"#NET-CONFIG 0,192.168.113.10,255.255.0.0,192.168.0.1\r",
				"#NET-CONFIG? 0\r",
				"#ETH-PORT TCP,5000\r",
				"#ETH-PORT? UDP\r",
				"#SECUR 1\r",
				"#LOGIN admin,33333\r",
				"#LOGIN?\r",
				"#LOGOUT\r",
				"#VERSION?\r",
				"#UPGRADE\r",
				"#FCT-MODEL dip-20\r",
				"#MODEL?\r",
				"#FCT-SN 19763840581123\r",
				"#SN?\r",
				"#LOCK-FP 0\r",
				"#LOCK-FP?\r",
				"#IDV\r",
				"#STANDBY 1\r",
				"#RESET\r",
				"#FACTORY\r",
				"#BEACON-EN 5000,0,1\r",
				"#BEACON-INFO?\r",
				"#BUILD-DATE?\r",
				"#TIME mon,05-12-2018,14:30:00\r",
				"#TIME?\r",
				"#TIME-LOC 3,0\r",
				"#TIME-LOC?\r",
				"#TIME-SRV 1,128.138.140.44,0\r",
				"#TIME-SRV?\r",
				"#KDS-ACTIVE-CLNT?\r",
				"#LOG-ACTION 1 1\r",
				"#SIGNALS-LIST?\r",
				"#PORTS-LIST?\r",
				"#LOG-TAIL? 2\r",
				NULL
};
int main(int argc ,char*argv[])
{
	P3K_ApiInit();
	gs_p3khandle.sendMsg = T_senddata;
	P3K_ApiRegistHandle(&gs_p3khandle);
	//char  cmdbuf[512] ={0};
	//char strCmd[64] ;
	//char strParam[256] ;
	//char ch = 0;
	int handleId=gs_p3khandle.handleId;
	int i= 0;
	while(1)
	{
		if(cmdStr[i] == NULL)
		{
			i= 0;
			break;
		}
		printf("handleid=%d ,cmd = [%s] \n",handleId,cmdStr[i]);
		gs_p3khandle.P3kMsgRecv(handleId,cmdStr[i],strlen(cmdStr[i]));
		#if 0
		ch = getchar();
		if(ch == 'q')
		{
			break;
		}
		#endif
		usleep(20*1000);
		i++;
	}
	P3K_ApiUnRegistHandle(&gs_p3khandle);
	P3K_APIUnInit();
	return 0;
}
