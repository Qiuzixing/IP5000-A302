#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ldfw.h"
#include "debugtool.h"
#define  ERR_STR  "errnn"
#define  REA_STR  "ready"
#define  OK_STR  "ok"
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
	char aReady[256] = {0};
	handeId= P3K_LDWFGetUpgradeHandle();
	printf("handeId==%d\n",handeId);
	if(handeId > 0)
	{
		memcpy(cmdresp->command,cmdreq->command,MAX_COMMANDNAME_LEN);
		memcpy(cmdresp->param,ERR_STR,strlen(ERR_STR));
		return -1;
	}
	//分配空间
	else
	{
		//if()
		int fd = open("./demo.txt", O_CREAT |O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
		int length = atoi(cmdreq->param);
		lseek(fd,length  - 1, SEEK_SET);
		write(fd, "1", 1); 
		sprintf(aReady,"%s,%s",cmdreq->param,REA_STR);
		memcpy(cmdresp->command,cmdreq->command,MAX_COMMANDNAME_LEN);
		memcpy(cmdresp->param,aReady,strlen(aReady));
		//rewind(fd);
	}
	//升级handle
	P3K_LDWFSetUpgradeHandle(handleId);
	//返回
	
	return 0;
};
int P3K_LDWFRecvFileData(char*data)
{
	if(P3K_LDWFGetUpgradeHandle() <= 0)
	{
		DBG_InfoMsg("no user to trans upgradefile\n");
		return 0;
	}
	//char aHeadData[2] = {0};
	//char aData[]
	char aPacketID[4] = {0};
	char aLength[4] = {0};
	char aData[4] = {0};
	char aCRC[4] = {0};
	//acketID = data[0];
	//sprintf(aPacketID,"%s%s",data[0],data[1]);
	//sprintf(aLength,"%s%s",data[2],data[3]);
	//sprintf(aData,"%s%s",data[4],data[5]);
	//sprintf(aCRC,"%s%s",data[6],data[7]);
	memcpy(aPacketID,data,2);
	printf("AAAAaPacketID = %s\n",aPacketID);
	
	//
	return 0;
}

