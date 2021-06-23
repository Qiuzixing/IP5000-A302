#ifndef __P3KSWITCH_H__
#define __P3KSWITCH_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"




/***************************
输入值:
cmdreq :命令请求信息 回复中param参数以','号隔开
输出值:
cmdresp:命令回复信息
userdata:用户附加数据
返回值
成功返回0，失败返回-1
****************************/
int P3K_SilmpleReqCmdProcess(P3K_SimpleCmdInfo_S *cmdreq,P3K_SimpleCmdInfo_S *cmdresp,char*userdata);
int P3K_CheckedSpeciCmd(char*cmd);

int P3K_SilmpleReqCmdExcuter();
#ifdef __cplusplus
}
#endif

#endif
