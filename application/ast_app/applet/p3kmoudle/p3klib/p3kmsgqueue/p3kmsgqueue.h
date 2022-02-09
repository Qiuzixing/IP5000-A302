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
��ʼ��queue
����ֵ
0�ɹ���-1ʧ��
******************/
int P3K_MsgQueueInit();
/*****************
ȥ��ʼ��queue
����ֵ
0�ɹ���-1ʧ��
******************/
int P3K_MsgQueueUinit();
/*****************
���������Ϣ
����ֵ:
msg : ��Ա��Ϣ
����ֵ
0�ɹ���-1ʧ��
******************/
int P3K_MSgQueueAddMsg(P3KMsgQueueMember_S *msg);
/*****************
���������Ϣ
���ֵ:
msg :��Ա��Ϣ
����ֵ
0�ɹ���-1ʧ��
******************/
int P3K_MSgQueueGetMsg(P3KMsgQueueMember_S *msg);

#ifdef __cplusplus
}
#endif

#endif
