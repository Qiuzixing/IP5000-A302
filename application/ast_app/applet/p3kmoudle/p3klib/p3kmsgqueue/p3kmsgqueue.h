#ifndef __P3KMSGQUEUE_H__
#define __P3KMSGQUEUE_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
#define MAX_QUEUE_BLOCK_NUMBER  20


typedef struct _P3KMsgQueueMember_S
{
	int handleId;
	P3K_SimpleCmdInfo_S cmdinfo;
	
}P3KMsgQueueMember_S;
/*****************
初始化queue
返回值
0成功，-1失败
******************/
int P3K_MsgQueueInit();
/*****************
去初始化queue
返回值
0成功，-1失败
******************/
int P3K_MsgQueueUinit();
/*****************
添加命令消息
输入值:
msg : 成员信息
返回值
0成功，-1失败
******************/
int P3K_MSgQueueAddMsg(P3KMsgQueueMember_S *msg);
/*****************
添加命令消息
输出值:
msg :成员信息
返回值
0成功，-1失败
******************/
int P3K_MSgQueueGetMsg(P3KMsgQueueMember_S *msg);

#ifdef __cplusplus
}
#endif

#endif
