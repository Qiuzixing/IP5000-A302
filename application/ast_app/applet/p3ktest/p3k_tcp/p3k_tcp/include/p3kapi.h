#ifndef __P3KAPI_H__
#define __P3KAPI_H__


#ifdef __cplusplus
extern "C"{
#endif
#include "common.h"

 
typedef int (*P3K_MsgSend)(int handleId,char*data, int len); //组包好的数据发送

typedef struct _P3KApiHandle_S
{
	int handleId;  //注册获得handleId ，不同用户注册返回不同id 最大支持10个
	int (*P3kMsgRecv)(int handleId,char*data, int len); //接收消息回调
	P3K_MsgSend sendMsg;    //发送消息
}P3KApiHandle_S;


/*******************
功能:模块初始化

********************/
int P3K_ApiInit();
/*******************
功能:模块去初始化

********************/
int P3K_APIUnInit();
/*******************
功能:模块注册函数
输入参数:
handle:注册handle 
********************/
int P3K_ApiRegistHandle(P3KApiHandle_S*handle);
/*******************
功能:模块注销函数
输入参数:
handle:注销handle
********************/
int P3K_ApiUnRegistHandle(P3KApiHandle_S*handle);

/*******************
功能:获取以接收命令消息总数
输出参数:
sum:命令总数
********************/

int P3K_ApiGetTotalRecvCmd(int *sum);

/*******************
功能:获取以处理命令消息总数
输出参数:
sum:命令总数
********************/
int P3K_ApiGetTotalExcuteCmd(int *sum);

#ifdef __cplusplus
}
#endif

#endif


