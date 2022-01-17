#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debugtool.h"
#include "p3kpacket.h"
#define P3K_REQMSG_PREFIX   '#'
#define P3K_REQMSG_SUFFIX  '\r'
#define P3K_RESPMSG_PREFIX  "~01@"
#define P3K_RESPMSG_SUFFIX  "\r\n"

int P3K_HandleShark(char *data)
{
	
	if(NULL == data)
	{
		return -1;
	}
	sprintf(data,"#\r");
	return 0;
}
int P3K_CheckHandleShark(char*data)
{
	if(data == NULL)
	{
		return -1;
	}
	if(data[0]== P3K_REQMSG_PREFIX  && data[1] == P3K_REQMSG_SUFFIX)
	{
		return 0;
	}
	return -1;
}

int P3K_HandleSharkResp(char*data)
{
	if(data == NULL)
	{
		return -1;
	}
	char tmpdata[32] ;
	memset(tmpdata,0,32);
	sprintf(tmpdata,"%s%s",P3K_RESPMSG_PREFIX,P3K_RESPMSG_SUFFIX);

	memcpy(data,tmpdata,strlen(tmpdata));
	return 0;
}

int P3K_SimpleRespCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata)
{
	char tmpdata[4096] ={0};
	int tmplen = 0;
	if(dstdata == NULL || cmd == NULL)
	{
		return -1;
	}
	memset(tmpdata,0,4096);
	char * tmp = &cmd->command[strlen(cmd->command)-1];
	char * endl = "?";
	if(!memcmp(tmp,endl,strlen(endl)))
	{
		char newcmd[32] = {0};
		memcpy(newcmd,cmd->command,strlen(cmd->command)-1);
		memset(cmd->command,0,sizeof(cmd->command));
		memcpy(cmd->command,newcmd,strlen(newcmd));
	}
	sprintf(tmpdata,"%s%s %s%s",P3K_RESPMSG_PREFIX,cmd->command,cmd->param,P3K_RESPMSG_SUFFIX);
	tmplen = strlen(tmpdata);
	DBG_InfoMsg("resp data %s\n",tmpdata);
	memcpy(dstdata,tmpdata,tmplen);
	return tmplen;
}
int P3K_SimpleReqCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata)
{
	
	if(dstdata == NULL || cmd == NULL)
	{
		return -1;
	}
	char tmpdata[384] ={0};
	//int tmplen = 0;
	memset(tmpdata,0,384);
	sprintf(tmpdata,"%c%s %s%c",P3K_REQMSG_PREFIX,cmd->command,cmd->param,P3K_REQMSG_SUFFIX);
	return 0;
}

int P3K_SimpleReqPacketUnpack(char*data,int len,P3K_SimpleCmdInfo_S *cmd)
{
	
	if(data == NULL)
	{
		return -1;
	}
	int tmpLen = 0 ;
	int s32Ret = 0;
	int i = 0;
	char *tmpdata = data;
	char *tmpdata1 = data;
	char *startdata = data;
	P3K_SimpleCmdInfo_S tmpcmd;
	char cmddata[MAX_PARAM_LEN] ;
	//static char tmpdata[MAX_PARAM_LEN] = ;
	while(tmpdata != NULL)
	{
		tmpdata = strchr(tmpdata,P3K_REQMSG_SUFFIX);
		if(tmpdata != NULL)
		{
			memset(cmddata,0,MAX_PARAM_LEN);
			tmpLen = tmpdata-tmpdata1;
			memcpy(cmddata,tmpdata1,tmpLen+1);
			tmpLen +=1;
			if(cmddata[0] == P3K_REQMSG_PREFIX)
			{
				memset(&tmpcmd,0,sizeof(P3K_SimpleCmdInfo_S));
				s32Ret = P3K_SimpleReqCmdUnpack(cmddata,&tmpcmd);
				if(!s32Ret)
				{
					memcpy(&(cmd[i]),&tmpcmd,sizeof(P3K_SimpleCmdInfo_S));
	
					i++;
				}
			}
			else
			{
				DBG_WarnMsg("msg str =%s not  p3k msg\n",cmddata);
			}

			if(i >= 16)
			{
				DBG_WarnMsg("[P3K_SimpleReqPacketUnpack] i >= 16\n ");
				break;
			}

			//printf("[P3K_SimpleReqPacketUnpack] while tmpdata[%d] : %s\n ",i,tmpdata);

			if(startdata != data)
			{
				DBG_WarnMsg(">>>>>>>>pppppp sizeof(P3K_SimpleCmdInfo_S) tmpdata : %p; startdata : %p; data : %p\n ",tmpdata,startdata,data);
			}

			if(len > tmpdata-data+1)
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
	if(tmpdata)
	{
		return i;
	}
	if(tmpdata1 != data )
	{
		DBG_WarnMsg("left str = [ %s ]\n",tmpdata1);
		
	}
	else
	{
		DBG_WarnMsg("input packts have no p3kmsg");
	}
	//memcpy();
	return i;
}
int P3K_SimpleReqCmdUnpack(char*data,P3K_SimpleCmdInfo_S*cmdInfo)
{
	P3K_SimpleCmdInfo_S cmd;
	//int i = 0;
	if(data == NULL)
	{
		return -1;
	}
	if(data[0] !=P3K_REQMSG_PREFIX)
	{
		DBG_ErrMsg("input data %s does not P3K message\n",data);
		return -1;
	}
	else if( *(data+(strlen(data)-1) ) != P3K_REQMSG_SUFFIX)
	{
		DBG_ErrMsg("input data %s does not P3K message\n",data);
		return -2;
	}
	if(data[1] == P3K_REQMSG_SUFFIX)
	{
		//Œ’ ÷
		DBG_InfoMsg("HANDSHARK\n");
		return 0;
	}
	memset(&cmd,0,sizeof(P3K_SimpleCmdInfo_S));
	sscanf(&(data[1]),"%s %[^\r]",cmd.command,cmd.param);
	DBG_InfoMsg(" %s %s\n",cmd.command,cmd.param);


	memcpy(cmdInfo,&cmd,sizeof(P3K_SimpleCmdInfo_S));
	return 0;
}

