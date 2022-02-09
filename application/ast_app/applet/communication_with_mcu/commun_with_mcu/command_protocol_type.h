/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限责任公司版权所有
* 系统名称：
* 文件名称：CommandProtocol.h
* 文件说明：
* 作    者：xdl
* 版本信息：V1.0
* 设计日期：
* 修改记录：
* 日    期      版    本        修改人      修改摘要
****************************************************************************/
#ifndef __COMMAND_PROTOCOL_TYPE_H__
#define __COMMAND_PROTOCOL_TYPE_H__
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_H_EN  0
#if DEBUG_H_EN
#include "debug.h"
#else
/* output log's level */
typedef enum
{
    ELOG_LVL_ASSERT                   =0,
    ELOG_LVL_ERROR                      =1,
    ELOG_LVL_WARN                        =2,
    ELOG_LVL_INFO                          =3,
    ELOG_LVL_DEBUG                       =4,
    ELOG_LVL_VERBOSE                    = 5,
}E_debugLvl;
extern E_debugLvl elogLev;

#define NONE                        "\x1b[0m\r\n"

/* output log front color */
#define F_BLACK                        "\x1b[30m"
#define F_RED                          "\x1b[31m"
#define F_GREEN                        "\x1b[32m"
#define F_YELLOW                       "\x1b[33m"
#define F_BLUE                         "\x1b[34m"
#define F_MAGENTA                      "\x1b[35m"
#define F_CYAN                         "\x1b[36m"
#define F_WHITE                        "\x1b[37m"

#define GB_ASSERT(type, fmt,...)   if(elogLev>=ELOG_LVL_ASSERT)    printf(F_MAGENTA type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define GB_ERROR(type, fmt, ...)      if(elogLev>=ELOG_LVL_ERROR)   printf(F_RED type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define GB_WARN(type, fmt, ...)      if(elogLev>=ELOG_LVL_WARN)    printf(F_YELLOW type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define GB_INFO(type, fmt, ...)       if(elogLev>=ELOG_LVL_INFO)    printf(F_CYAN type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define GB_DEBUG(type, fmt, ...)    if(elogLev>=ELOG_LVL_DEBUG)    printf(F_GREEN type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define GB_VERBOSE(type, fmt, ...) if(elogLev>=ELOG_LVL_VERBOSE)   printf(F_BLUE type "[%s,%d] " fmt NONE,  __FUNCTION__,__LINE__, ##__VA_ARGS__)

#endif

typedef unsigned char      BOOLEAN;
typedef unsigned char      BOOL;
typedef signed char        S8;
typedef unsigned char      U8;
typedef signed short       S16;
typedef unsigned short     U16;
typedef signed long        S32;
typedef unsigned long    U32;
typedef signed long long   S64;
typedef unsigned long long U64;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)                                 (sizeof(arr)/sizeof(arr[0]))
#endif
#ifndef min
#define min(a,b) (((a)<(b))? (a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))? (a):(b))
#endif
#ifndef FALSE
#define  FALSE   (0)
#endif

#ifndef TRUE
#define  TRUE    (1)
#endif

#ifndef IMX_SUCCESS
#define  IMX_SUCCESS   (0)
#endif

#ifndef IMX_FAILED
#define  IMX_FAILED    (-1)
#endif


typedef enum
{
    CMD_DEV_STM32,
    CMD_DEV_6A828=0x10,
    CMD_DEV_IMX6UL=0x20,
    CMD_DEV_TELNET=0x30,
    CMD_DEV_WEB=0x40,
    CMD_DEV_SERVER_TCP=0x50,
    CMD_DEV_ICSP=0x50,
    CMD_DEV_AIPNE,
    CMD_DEV_MASTER,
    CMD_DEV_AUTOSETUP,
    CMD_DEV_LED_IO,
    
}E_CMD_DEV;

/*注意& 规则:
1.新增加的模块，按照下面规律往下增加宏定义;宏都是定义表示开始地址。
2.已经定义的地址，修改
3.所有模块都需要在下面宏中体现
4.每个模块之间的通信范围  0x1000
5.模块内部不同功能区分范围 0x100 [例如:IR  CEC  AUDIO  VIDEO  OSD BASE ....]
*/
#define  BASE_CMD_START_RANGE      0x0000 /* range  [0x0000~0x1000]   20190621日增加*/
#define  STM32_CMD_START_RANGE    0x1000 /* range  [0x1000~0x2000]   20190621日增加*/
#define  SCALER_CMD_START_RANGE  0x2000 /* range  [0x2000~0x3000]   20190621日增加*/
#define  AIPNET_CMD_START_RANGE  0x3000 /* range  [0x3000~0x4000]   20190621日增加*/
#define  ICSP_CMD_START_RANGE      0x4000 /* range  [0x4000~0x5000]   20190621日增加*/
#define  WEB_CMD_START_RANGE      0x5000 /* range  [0x5000~0x6000]   20190625日增加*/


/*
命令接口
基本命令 范围 [0x0000~0x1000]
*/
typedef enum _E_BASE_CMD_Param1
{
    CMD_ACK=BASE_CMD_START_RANGE,//0x0000  /*协议使用*/
    CMD_HEART_BEAT, /*协议使用*/
    CMD_LARGE_PACKET_PROTOCOL,/*协议使用*/
    CMD_CLEAR_RECORD_FLAG,/*协议使用*/

    //模块调试命令
    CMD_COMM_TESET_MODULE =0x20,/* 通信测试模块, U32 data*/
    /* CMD_COMM_VIDEO_DEBUG: MCU收到命令后, MCU回复字符串包含下面信息
        1.信号是否LOCK
        2.输入输出 HDCP
        3.信号通道选择情况
    */
    CMD_COMM_VIDEO_DEBUG,
    
    CMD_UPDATE_MAC_AND_SN=0X100, 
    CMD_SEND_LKT4106,/*发送KEY 算法*/ 
    CMD_SEND_LKT4106_NVM, /*发送KEY NVM*/ 
    CMD_COMM_ARM_INFO, 
    CMD_COMM_GPIO_IDBUTTON,// master to idbutton
    CMD_COMM_GPIO_IDBUTTON_REPORT, 
    
    CMD_UPDATE_SW_GET_DEVICE_ID=0X300, 
    CMD_UPDATE_SW_REPORT_DEVICE_ID,          
    CMD_UPDATE_SW_SEND_FILE_START,       
    CMD_UPDATE_SW_SEND_FILE,                   
    CMD_UPDATE_SW_REPORT_FILE,              
    CMD_UPDATE_SW_READY_UPDATE=0x380,            
    CMD_UPDATE_SW_REPORT_UPDATE,          
    CMD_UPDATE_SW_FACTORY_FWIMAGE,
    CMD_UPDATE_SW_FACTORY_FWIMAGE_REPORT,
    CMD_UPDATE_SW_EP_STATUS_REPORT,/*上报升级状态*/
    CMD_UPDATE_SW_VALENS_STATUS_REPORT,/*上报升级状态*/
    CMD_UPDATE_SW_EP_PROGRESS_REPORT,// 0~100 上报升级进度
    CMD_UPDATE_SW_VALENS_PROGRESS_REPORT,// 0~100 上报升级进度
    CMD_UPDATE_SW_CHECK_SENDFILE,/*确认文件完整性*/
    
    CMD_UPDATE_KEY_LOAD=0x3A0,
    CMD_UPDATE_KEY_LOAD_REPORT,
    CMD_UPDATE_KEY_CLEAR,
    CMD_UPDATE_KEY_CLEAR_REPORT,
    CMD_UPDATE_KEY_CHECK,
    CMD_UPDATE_KEY_CHECK_REPORT,
}E_BASE_CMD_Param1;


/*
STM32
基本命令 范围 [0x1000~0x2000]
*/
typedef enum _E_CMD_Param1
{
    /*基本*/
    CMD_STM32_1=STM32_CMD_START_RANGE, //0x1000
    CMD_STM32_11,/*此命令需要长时间处理*/
    
    CMD_STM32_2= STM32_CMD_START_RANGE +0x100 ,


}E_CMD_Param1;


#ifdef __cplusplus
}
#endif
#endif//#ifndef __COMMAND_PROTOCOL_TYPE_H__
