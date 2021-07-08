#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ldfw.h"
#include "debugtool.h"
#define  ERR_STR  "errnn"
typedef struct _UpgradeFilePacket_S
{
	int packId;
	short size;
	short crc;
	char*data;
}UpgradeFilePacket_S;
typedef struct _UpgradeFileInfo_S
{
	int totalsize;
	int count ;
	UpgradeFilePacket_S packets[];
}UpgradeFileInfo_S;
typedef struct _UpgradeFileTransStatus_S
{
	int handleId;
	int status;
	int totalSize;
	int recvSize;
}UpgradeFileTransStatus_S;

static UpgradeFileTransStatus_S gs_transStatus ={0};

static int P3K_LDWFSetUpgradeHandle(int handleId)
{
	gs_transStatus.handleId = handleId;
	return 0;
}
int P3K_LDWFGetUpgradeHandle()
{

	return gs_transStatus.handleId;
}

int P3K_LDWFCmdProcess(int handleId,P3K_SimpleCmdInfo_S *cmdreq,P3K_SimpleCmdInfo_S *cmdresp)
{
	int handeId = 0;
	handeId= P3K_LDWFGetUpgradeHandle();
	if(handeId > 0)
	{
		memcpy(cmdresp->command,cmdreq->command,MAX_COMMANDNAME_LEN);
		memcpy(cmdresp->param,ERR_STR,strlen(ERR_STR));
		return -1;
	}
	//分配空间
	 
	
	//升级handle
	P3K_LDWFSetUpgradeHandle(handleId);
	//返回
	
	return 0;
};
int P3K_LDWFRecvFileData(char*data,int len)
{
	if(P3K_LDWFGetUpgradeHandle() <= 0)
	{
		DBG_InfoMsg("no user to trans upgradefile\n");
		return 0;
	}
	//
	return 0;
}

