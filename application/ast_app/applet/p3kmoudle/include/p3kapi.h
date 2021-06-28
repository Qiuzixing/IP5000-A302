#ifndef __P3KAPI_H__
#define __P3KAPI_H__


#ifdef __cplusplus
extern "C"{
#endif
#include "common.h"

 
typedef int (*P3K_MsgSend)(int handleId,char*data, int len); //����õ����ݷ���

typedef struct _P3KApiHandle_S
{
	int handleId;  //ע����handleId ����ͬ�û�ע�᷵�ز�ͬid ���֧��10��
	int (*P3kMsgRecv)(int handleId,char*data, int len); //������Ϣ�ص�
	P3K_MsgSend sendMsg;    //������Ϣ
}P3KApiHandle_S;


/*******************
����:ģ���ʼ��

********************/
int P3K_ApiInit();
/*******************
����:ģ��ȥ��ʼ��

********************/
int P3K_APIUnInit();
/*******************
����:ģ��ע�ắ��
�������:
handle:ע��handle 
********************/
int P3K_ApiRegistHandle(P3KApiHandle_S*handle);
/*******************
����:ģ��ע������
�������:
handle:ע��handle
********************/
int P3K_ApiUnRegistHandle(P3KApiHandle_S*handle);

/*******************
����:��ȡ�Խ���������Ϣ����
�������:
sum:��������
********************/

int P3K_ApiGetTotalRecvCmd(int *sum);

/*******************
����:��ȡ�Դ���������Ϣ����
�������:
sum:��������
********************/
int P3K_ApiGetTotalExcuteCmd(int *sum);

#ifdef __cplusplus
}
#endif

#endif


