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
int fd;
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
	//����ռ�
	else
	{
		//if()
#ifdef CONFIG_P3K_HOST
		fd = open("/dev/shm/fw.tar.gz", O_CREAT |O_TRUNC | O_RDWR, 0666);
#else
		fd = open("/dev/shm/fw.tar.gz", O_CREAT |O_TRUNC | O_RDWR, 0666);//S_IRUSR | S_IWUSR);
#endif
		int length = atoi(cmdreq->param);
		//lseek(fd,length  - 1, SEEK_SET);
		//write(fd, "1", 1);
		sprintf(aReady,"%s,%s",cmdreq->param,REA_STR);
		memcpy(cmdresp->command,cmdreq->command,MAX_COMMANDNAME_LEN);
		memcpy(cmdresp->param,aReady,strlen(aReady));
		lseek(fd,0, SEEK_SET);
		//rewind(fd);
	}
	//����handle
	P3K_LDWFSetUpgradeHandle(handleId);
	//����

	return 0;
};
int P3K_LDWFRecvFileData(char*data,char * aPnum ,int len)
{
	if(P3K_LDWFGetUpgradeHandle() <= 0)
	{
		DBG_InfoMsg("no user to trans upgradefile\n");
		return 0;
	}
	//char aHeadData[2] = {0};
	//char aData[]
	char aPacketID[8] = {0};
	char aLength[8] = {0};
	char aData[8] = {0};
	char aCRC[8] = {0};
	char * str = "";
	//memcpy(aPacketID,data,2);
	sprintf(aPacketID,"%02x%02x",data[0],data[1]);
	//memcpy(str,data,strlen(data)-4);
	write(fd, data+4, len);
	//memcpy(fd,data,len);
	//printf("AAAAaPacketID = %s   \n",aPacketID);
	memcpy(aPnum,aPacketID,strlen(aPacketID));
	//
	return 0;
}

