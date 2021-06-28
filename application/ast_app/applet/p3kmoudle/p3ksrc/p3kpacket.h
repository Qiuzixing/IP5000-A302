#ifndef __P3KPACKET_H__
#define __P3KPACKET_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
/************************
�������
data  �������ݰ������ܲ�ֻһ������
len  ���ݰ�����
���ֵ:
cmd[] ������������
����ֵ
 int ��Ϊ�������������
*************************/
int P3K_SimpleReqPacketUnpack(char*data,int len,P3K_SimpleCmdInfo_S* cmd);

/************************
�������
data  :p3k�ַ���

���ֵ:
cmd ����������Ϣ
����ֵ
�ɹ�Ϊ0 ʧ��Ϊ -1
*************************/

int P3K_SimpleReqCmdUnpack(char*data,P3K_SimpleCmdInfo_S*cmdInfo);

/************************
�������
cmd ����������Ϣ
���ֵ:
dstdata  :p3k�ַ���
����ֵ
�����ַ�������
*************************/
int P3K_SimpleRespCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata);

int P3K_SimpleReqCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata);

int P3K_HandleSharkResp(char*data);

#ifdef __cplusplus
}
#endif

#endif
