#ifndef __P3KPACKET_H__
#define __P3KPACKET_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
/************************
输入参数
data  接收数据包，可能不只一条命令
len  数据包长度
输出值:
cmd[] 返回命令数组
返回值
 int 型为解析的命令个数
*************************/
int P3K_SimpleReqPacketUnpack(char*data,int len,P3K_SimpleCmdInfo_S* cmd);

/************************
输入参数
data  :p3k字符串

输出值:
cmd 返回命令信息
返回值
成功为0 失败为 -1
*************************/

int P3K_SimpleReqCmdUnpack(char*data,P3K_SimpleCmdInfo_S*cmdInfo);

/************************
输入参数
cmd 返回命令信息
输出值:
dstdata  :p3k字符串
返回值
返回字符串长度
*************************/
int P3K_SimpleRespCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata);

int P3K_SimpleReqCmdBurstification(P3K_SimpleCmdInfo_S *cmd,char *dstdata);

int P3K_HandleSharkResp(char*data);

#ifdef __cplusplus
}
#endif

#endif
