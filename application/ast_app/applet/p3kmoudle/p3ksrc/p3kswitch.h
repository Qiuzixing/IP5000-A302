#ifndef __P3KSWITCH_H__
#define __P3KSWITCH_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"




/***************************
����ֵ:
cmdreq :����������Ϣ �ظ���param������','�Ÿ���
���ֵ:
cmdresp:����ظ���Ϣ
userdata:�û���������
����ֵ
�ɹ�����0��ʧ�ܷ���-1
****************************/
int P3K_SilmpleReqCmdProcess(P3K_SimpleCmdInfo_S *cmdreq,P3K_SimpleCmdInfo_S *cmdresp,char*userdata);
int P3K_CheckedSpeciCmd(char*cmd);

int P3K_SilmpleReqCmdExcuter();
#ifdef __cplusplus
}
#endif

#endif
