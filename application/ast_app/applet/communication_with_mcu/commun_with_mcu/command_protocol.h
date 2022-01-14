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
#ifndef __COMMAND_PROTOCOL_H__
#define __COMMAND_PROTOCOL_H__
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "command_protocol_type.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 *1.0  协议中打印等级选择
 */
#define  CMD_DEBUG_NULL                         0x00
#define  CMD_DEBUG_NOMAL_SEND_RECV 0x01
#define  CMD_DEBUG_FILE_SEND_RECV     0x02
#define  CMD_DEBUG_AGING                      0x04
#define  CMD_DEBUG_SEND_RECV              0x08
#define  CMD_DEBUG_ALL                           0xFF

#define CMD_DEBUG  CMD_DEBUG_NULL

/*
 *2.0 平台选择,设置是否带系统
 */
#define CMD_SYSTEM_EN  0 // 0x01->system   0x00->mcu
#if CMD_SYSTEM_EN
    #ifndef __linux__
    #define STM32_CHIP_EN
    #endif
#endif
/*
 *2.1 常用设置宏
 */

#define PROTOCOL_VERSION   0x103 // V1.3
#define ProtocolOpenSize  10  
#define DEFAULT_DATA_LEN  0x04  //It cannot be modified
#define CMD_STRUCT_CRC_LEN  0x07 //It cannot be modified
#define CMD_RECV_BUF_SIZE  15
#define CMD_SEND_BUF_SIZE  30
#define CMD_RECORD_PROCESSING  max(5,CMD_RECV_BUF_SIZE)  //It cannot be modified

/*
 *2.1.1 先进先出使能模式
 *ENABLE: 保证先发的命令，先传输。
 *DISABLE: 不保证命令顺序，串口利用率高。
 */
#define CMD_SEND_MODE_FIFO_EN  0
#define CMD_SEND_TIME_GAP_MS  40  /*每次发送命令的时间间隔*/
#define CMD_SEND_TIME_REPEAT_MS  400  /*FIFO模式下无作用*/

/*
 *2.2 单次发送的数据大于CMD_DATA_MAX_LEN需要打开此宏
 *会使用malloc空间，超过2K，需要考虑MM空间
 */
#define CMD_LARGEPACKET_PROTOCOL   0 //The data sent in a single time is larger than the CMD_ DATA_ MAX_ Len needs to open this macro.

/*
 *2.3 数据校验方法选择
 *注意:协议双方必须选择相同级别，不能CRC16  与 CRC8通信
 */
#define CAL_CRC16_TABLE 0x01
#define CAL_CRC8_TABLE   0x02
#define CAL_CRC8               0x03
#define CMD_CAL_CRC_SELECT   CAL_CRC16_TABLE

#if(CMD_CAL_CRC_SELECT == CAL_CRC16_TABLE)
U16 CheckSum_crc16(U16 cksum, unsigned char *buf, int len);
#elif(CMD_CAL_CRC_SELECT == CAL_CRC8_TABLE)
unsigned char cal_crc8_table(unsigned char crc, unsigned char *ptr, int len);
#define CheckSum_crc16(a,b,c) cal_crc8_table(a,b,c)
#elif(CMD_CAL_CRC_SELECT == CAL_CRC8)
unsigned char cal_crc8(unsigned char crc, unsigned char *ptr, int len);
#define CheckSum_crc16(a,b,c) cal_crc8(a,b,c)
#endif

/* 
 *2.4 大小端转换开启 
 */
#define CMD_SWAP_ENDIAN     0 // default : 0
#if CMD_SWAP_ENDIAN
#define Tu32(x) (((x&0xff)<<24)|((x&0xff00)<<8)|((x&0xff0000)>>8)|((x&0xff000000)>>24))
#define Tu16(x) (((x&0xff)<<8)|((x&0xff00)>>8))
#else
#define Tu32(x) x
#define Tu16(x) x
#endif

/*
 *2.5 文件传输
 */
#define CMD_SEND_FILE_EN  0
#if 0//def STM32_CHIP_EN
#define CMD_SEND_FILE_MODE 1
#else
#define CMD_SEND_FILE_MODE 0
#endif
#define CMD_FILE_MIN_BUF_LEN   0x08 // When transferring the minimum file length, the bug must be greater than 4byte. If it is less than or equal to 4, there will be problems
#if CMD_SEND_FILE_EN
#define CMD_PATH_NAME_LEN   64  /* 修改，需要所有模块都要修改，不然传输文件有问题*/
#else
#define CMD_PATH_NAME_LEN   1
#endif

/* 
 *2.6 处理时间过长的命令
 *可以在CommandInterfaceDataProcessing函数，增加相应的命令
 */
#define CMD_BACKUP_CMD_EN   0

/*
 *2.7 心跳使能，可以通过心跳判断设备是否连接
 */
#define CMD_HEART_BEAT_EN  1

/*
 *2.8 增加命令时，是否需要延时添加
 *ENABLE:当发现BUF不够时，会先DELAY 500MS再尝试添加
 *DISABLE:BUF够时，是不会等到，添加不成功的命令将不会发送到对方。
 */
#define CMD_ADD_CMD_DELAY_EN  0
#define CMD_ADD_CMD_DELAY_MS  500// 最大等待时间

/*
 *2.9 协议接收数据的方式(根据软件架构选择)
 *enable: 协议线程读取
 *disable:中断直接给到协议
 */
#define CMD_RECV_DATA_MODE  1

/*
 *3.0  是否需要加密
 *enable: 开启
 *disable:关闭(对方发过加密数据将不能设别)
 */
#define CMD_ENCRYPTION_EN  0

/*
 *3.0  打印命令
 *enable: 开启
 *disable:关闭
 */
#define CMD_DISPLAY_STR  0

typedef enum
{
    COMM_UART,
    COMM_UDP,
}E_COMM_STATUS;

#pragma pack (1)

typedef struct _File_Info_Param
{
    U32 DeviceID;  /* */
    U8 FileName[CMD_PATH_NAME_LEN];  /* 升级文件名字*/
    U32 PacketsNum;  /*文件有效数据包个数*/
    U32 FileLen;  /*文件长度*/
    U32 FileCrc;  /*文件总校验值*/
}File_Info_Param;

#if CMD_LARGEPACKET_PROTOCOL
typedef struct _LargePacketProtocol
{
/*
注意:
1.最大支持64K 数据包。
2.芯片一次分配内存空间大小，也会限制 最大数据包。
*/
    U8 PacketsNum;  /*文件有效数据包个数*/
    U8 PacketsRecvNum;  /*接收到的数据包数量*/
    U16 PacketsLen;  /* 数据包的总长度*/
    U16 SendAddr;  /* 数据包的偏移地址*/
    U16 SendLen;  /* 数据包的偏移地址*/
    U32 SendCMD;  /* 对应的命令*/
    U8* buf;  /* 升级文件名字*/
}LargePacketProtocol;
#endif
typedef struct _Key_Param
{
    #define CMD_KEY_FLAG   0x80
    U8 KeySequence:7;  /*加解密密钥序列*/
    U8 keyEn:1;  /*0x00-> nomal   0x01->encryption 加密 标志 (SerialNumber,param  , data)*/
}Key_Param;

typedef struct _CmdProtocolParam
{
    #define CMD_START_FLAG 0xA5
    U8 startflag; /*固定0xA5*/
    #define CMD_START_FLAG2 0x5A
    U8 startflag2; /*固定0x5A*/
    U16 DataLen;  /*数据长度( data 数据总和长度)*/
    U16 checksum; /*(SequenceNumber,param  , data 数据 的checksum)*/
    Key_Param key;
    U16 SequenceNumber; /*(0x01~0xffff 自加,用于数据应答)*/
    U8 DevSrc;//E_CMD_DEV
    U8 DevDST;//E_CMD_DEV
    U16 CMD; /* 命令表*/   
    
    #define CMD_DATA_MAX_LEN   512
    /*
    1.datalen等于4，直接是4BYTE的数据
    2.datalen大于4，数据指针，动态分配。（Size小于等于512）
    */
    U32 Data; 
}CmdProtocolParam;
#pragma pack () 
typedef struct _CommandInterfaceFun
{
/* 需要实现*/
    /*------------------------------------------------------------------
    * Func : CommandInterfaceWrite
    * Func : 调用底层发送串口数据接口
    * buf :  数据buf
    * count :  数据len
    *------------------------------------------------------------------*/
    int (* CommandInterfaceWrite)(U8 *buf, int len);
    #if (CMD_RECV_DATA_MODE)
    /*------------------------------------------------------------------
    * Func : CommandInterfaceRead
    * Func : 调用底层读取数据接口
    * buf :  数据buf
    * count :  BUF长度
    * return :  读取的数据长度
    *------------------------------------------------------------------*/
    int (* CommandInterfaceRead)(U8 *buf, int len);
    #endif

    /*------------------------------------------------------------------
    * Func : CommandInterfaceDataProcessing
    * Func : 处理接收命令函数
    * param :  接收到的数据，需要处理的BUF
    *------------------------------------------------------------------*/
    void (* CommandInterfaceDataProcessing)(CmdProtocolParam * param);
    
/* 下面已经实现，直接调用*/
    #if (CMD_RECV_DATA_MODE == 0)
    /*------------------------------------------------------------------
    * Func : CommandInterfaceRead2
    * Func : 调用底层读取数据给到下面接口
    * fd :  设备ID
    * buf :  数据buf
    * count :  BUF长度
    * return :  0x00 成功  -1失败
    *------------------------------------------------------------------*/
    int (* CommandInterfaceRead2)(int fd ,U8 *buf, int len);
    #endif

    /*------------------------------------------------------------------
    * Func : CommandProtocolConfirmCmdsent
    * Func : 检测命令是否已经成功发送出去
    * fd :  设备ID
    * SendSequenceNumber :   发送命令序列
    * return :  0x00: 还在发送队列中，  0x01:已经发出    -2: error
    *------------------------------------------------------------------*/
    int (* CommandProtocolConfirmCmdsent)(int fd , int SequenceNumber);
    
    /*------------------------------------------------------------------
    * Func : CommandProtocolSendCmdAdd
    * Func :   增加发送的数据
    * fd :  设备ID
    * cmdparam :  发送命令的BUF
    * mode :  发送模式  0x01->立即发送0x00 -> FIFO 先进先出
    * return :  SendSequenceNumber  用于检测数据是否发出
    * return :  -1 表示刚才的命令没有加入队列中
    *------------------------------------------------------------------*/
    int (* CommandProtocolSendCmdAdd)(int fd , CmdProtocolParam * cmdparam , U8 mode);
    
    
    /*------------------------------------------------------------------
    * Func : CommandProtocolGetSendBufNum
    * Func : 获取当前还有多少命令没有发送完成
    * fd :  设备ID
    * return  :  发送命令条数  ,-2  error
    *------------------------------------------------------------------*/
    int (* CommandProtocolGetSendBufNum)(int fd );

    /*------------------------------------------------------------------
     * Func : CommandProtocolSetParam
     * Func : 命令处理线程
     * fd :  设备ID
     * data:  传递 keyEn KeySequence DevSrc DevDST debuglvl 参数
     *------------------------------------------------------------------*/
     int (* CommandProtocolSetParam)(int fd, void* data);
    
    /*------------------------------------------------------------------
    * Func : CommandProtocolReset
    * Func :复位协议
    * fd :  设备ID
    * return  :   0-> OK  -2->NG
    *------------------------------------------------------------------*/
    int (*CommandProtocolReset)(int fd);

    #if CMD_SEND_FILE_EN
    /*------------------------------------------------------------------
    * Func : CommandProtocolTransferFileSendStart
    * Func :   开始准备发送文件
    * fd :  设备ID
    * DeviceID :  传输文件ID
    * SrcPath :  源文件路径
    * DstPath :  目标文件路径
    * return :   0x00 -> ok    -1 -> NG
    *------------------------------------------------------------------*/
    int (* CommandProtocolTransferFileSendStart)(int fd ,U32 DeviceID, char * SrcPath, char * DstPath);

    /*------------------------------------------------------------------
    * Func : CommandProtocolTransferFileProgress
    * Func :  文件传输进度
    * fd :  设备ID
    * return  :  进度[0~100]
    *------------------------------------------------------------------*/
    int (* CommandProtocolTransferFileProgress)(int fd );
    #endif
    
    #if CMD_HEART_BEAT_EN
    /*------------------------------------------------------------------
    * Func : CommandProtocolHeartBeatStatue
    * Func :判断心跳是否丢失
    * fd :  设备ID
    * HeartBeatEnable:  0-> disable  1-> enable   自动心跳使能开关
    * return  :   0-> lose  1-> link
    *------------------------------------------------------------------*/
    int (*CommandProtocolHeartBeatStatue)(int fd,int HeartBeatEnable);
    #endif
    
    #if CMD_BACKUP_CMD_EN
    /*------------------------------------------------------------------
    * Func : CommandProtocolBackupCMDLongerToProgress
    * Func : 备份长时间处理的命令
    * fd :  设备ID
    * Cmdparam :  接收到的命令数据
    * int  :  备份状态(0x00-> ok    -1->NG)
    *------------------------------------------------------------------*/
    int  (*CommandProtocolBackupCMDLongerToProgress)(int fd,CmdProtocolParam * Cmdparam);
    /*------------------------------------------------------------------
    * Func : CommandProtocolGetCMDLongerToProgress
    * Func : 获取备份长时间处理的命令
    * fd :  设备ID
    * Cmdparam :  接收到的命令数据
    * int  :  备份状态(0x00-> ok    -1->NG)
    *------------------------------------------------------------------*/
    int  (*CommandProtocolGetCMDLongerToProgress)(int fd,CmdProtocolParam * Cmdparam);
    #endif
}CommandInterfaceFun;
    /*------------------------------------------------------------------
    * Func : CommandInterfaceRead2
    * Func : 调用底层读取数据给到下面接口
    * fd :  设备ID
    * buf :  数据buf
    * count :  BUF长度
    * return :  0x00 成功  -1失败
    *------------------------------------------------------------------*/
    int  CommandInterfaceRead2(int fd ,uint8_t data);
    /*------------------------------------------------------------------
    * Func : CommandProtocolSendCmdAdd
    * Func :   增加发送的数据
    * fd :  设备ID
    * cmdparam :  发送命令的BUF
    * mode :  发送模式  0x01->立即发送0x00 -> FIFO 先进先出
    * return :  SendSequenceNumber  用于检测数据是否发出
    * return :  -1 表示刚才的命令没有加入队列中
    *------------------------------------------------------------------*/
    int  CommandProtocolSendCmdAdd(int fd , CmdProtocolParam * cmdparam , U8 mode);
    /*------------------------------------------------------------------
    * Func : CommandProtocolTransferFileSendStart
    * Func :   开始准备发送文件
    * fd :  设备ID
    * DeviceID :  传输文件ID
    * SrcPath :  源文件路径
    * DstPath :  目标文件路径
    * return :   0x00 -> ok    -1 -> NG
    *------------------------------------------------------------------*/
    int  CommandProtocolTransferFileSendStart(int fd ,U32 DeviceID, char * SrcPath, char * DstPath);

    /*------------------------------------------------------------------
    * Func : CommandProtocolTransferFileProgress
    * Func :  文件传输进度
    * fd :  设备ID
    * return  :  进度[0~100]
    *------------------------------------------------------------------*/
    int CommandProtocolTransferFileProgress(int fd );
    /*------------------------------------------------------------------
    * Func : CommandProtocolGetSendBufNum
    * Func : 获取当前还有多少命令没有发送完成
    * fd :  设备ID
    * return  :  发送命令条数  ,-2  error
    *------------------------------------------------------------------*/
    int  CommandProtocolGetSendBufNum(int fd );
    /*------------------------------------------------------------------
    * Func : CommandProtocolReset
    * Func :复位协议
    * fd :  设备ID
    * return  :   0-> OK  -2->NG
    *------------------------------------------------------------------*/
    int CommandProtocolReset(int fd);
    /*------------------------------------------------------------------
     * Func : CommandProtocolSetParam
     * Func : 命令处理线程
     * fd :  设备ID
     * data:  传递 keyEn KeySequence DevSrc DevDST debuglvl 参数
     *------------------------------------------------------------------*/
     int  CommandProtocolSetParam(int fd, void* data);


typedef enum
{
    FILE_TRANSFER_NULL,
    FILE_TRANSFER_START_SEND,
    FILE_TRANSFER_FILE_SEND,
    FILE_TRANSFER_WAIT_FINISH_FLAG,
    FILE_TRANSFER_FINISH,
}E_FILE_TRANSFER_STATUS;
typedef enum
{
    FILE_RECV_NULL,
    FILE_RECV_FILE,
    FILE_RECV_FINISH,
    FILE_RECV_FINISH_OK,
    FILE_RECV_FINISH_NG,
}E_FILE_RECV_STATUS;

typedef struct _CommandInterfaceParam
{
    CommandInterfaceFun fun;
/* 下面已经实现，会使用到的数据变量*/
    U8 name[6];
    U16 Version;
    #if CMD_SYSTEM_EN
        #ifndef STM32_CHIP_EN
        pthread_t Taskthread;
        #else
        rt_thread_t Taskthread;
        #endif
    #endif
    
    U8 debuglvl; 
    #if (CMD_DEBUG == CMD_DEBUG_ALL)
    U32 debugsendtimes;
    U32 debugacktimes;
    #endif
    
#if CMD_SEND_FILE_EN
    /*1.文件传输*/
    #define FILE_TRANSFER_SEND_END 0x7fff
    U8 FileTransferSendStatus;// 0-> nomal  1-> start send 2-> send file 3-> wait send finish  4-> finish
    U8 FileTransferRecvStatus;// 0-> nomal  1-> Recv file  2-> recv finish ok  3-> recv finish ok
    U16 FileTransferProgress;/* 文件传输进度*/
    File_Info_Param FileInfo;
    char DstPath[CMD_PATH_NAME_LEN];
    #if(CMD_SEND_FILE_MODE)
    int TransferFile;
    #else
    FILE *TransferFile;
    #endif
#endif
    
    /*2.心跳*/
#if CMD_HEART_BEAT_EN
    U32 CmdSendBufStartTimes;/*记录数据开始发送时间， 超时取消重发*/
    U32 HeartBeatCheckTime;
    U32 HeartBeatSendTime;
    U8 HeartBeatStatue:1;// 0-> lose  1-> link
    U8 HeartBeatEnable:1;// 0-> disable  1-> enable
    U8 ThreadRun:1;// 0-> close  1-> open(run)
    U8 CommMode:2; // 0->uart  1-> UDP  
    U8 Reserve0:3; 
#else
    U8 ThreadRun:1;// 0-> close  1-> open(run)
    U8 CommMode:2; // 0->uart  1-> UDP  
    U8 Reserve0:5; 
#endif

    /*3.保存的设置参数*/
    Key_Param key;
    U8 DevSrc;//E_UART_DEV
    U8 DevDST;//E_UART_DEV

    /*4.0发送数据缓存*/
    CmdProtocolParam CmdSendBuf[CMD_SEND_BUF_SIZE]; 
    U32 SendCmdTimeGap;
    U32 CmdSendBufTimes[CMD_SEND_BUF_SIZE];/*记录数据发送时间，可以按照FIFO 方式重发*/
    /*4.1接收数据缓存*/
    CmdProtocolParam CmdRecvBuf[CMD_RECV_BUF_SIZE];
    U32 RecvCmdTimeGap;
    /*4.2 记录最近已经处理的 SequenceNumber*/
    U16 RecordProcessing[CMD_RECORD_PROCESSING];
    U8 RecordProcessingIndex;
        
    /*4.3 每次增加命令的 命令序列*/
    U16 SendSequenceNumber;
    /*4.4接收一条命令临时BUF ,数据索引*/
    U16 recvTempIndex;
    U8 RecvTempbuf[sizeof(CmdProtocolParam) + CMD_DATA_MAX_LEN];

    #if CMD_BACKUP_CMD_EN
    /*5.0  备份需要长时间处理的命令*/
    CmdProtocolParam BackupCMD[CMD_RECV_BUF_SIZE];
    U8 BackupCMDNum; /*备份命令数量*/
    #endif
    
    #if(CMD_LARGEPACKET_PROTOCOL)
    LargePacketProtocol LargePacket;
    #endif
}CommandInterfaceParam;
void CommandProtocolThreadDelay(U32 delayms);

int CommandProtocolTransferCheckFileInfo(char * Path,File_Info_Param* FileInfo);

/*------------------------------------------------------------------
 * Func : CommandProtocolMutexInit
 * Func : 创建线程互斥锁
 注意:调用CommandProtocolOpen函数前，需要初始化Mutex
 *------------------------------------------------------------------*/
int CommandProtocolMutexInit(void);
/*------------------------------------------------------------------
 * Func : CommandProtocolOpen
 * Func : 创建数据交互协议
 * fun :  传递函数指针
 * CommMode :    0->uart  1-> UDP  
 * name :  协议名字
 *------------------------------------------------------------------*/
 int CommandProtocolOpen(CommandInterfaceFun *fun,const char* name,U8 CommMode);
/*------------------------------------------------------------------
 * Func : CommandProtocolParam
 * Func : 中断一个BYTE的接收串口数据
 * fd :  设备ID
 *------------------------------------------------------------------*/
CommandInterfaceParam* CommandProtocolParam(int fd);
/*------------------------------------------------------------------
 * Func : CommandProtocolRecvSend_thread_Handler
 * Func : 命令处理线程
 * ctx :  整个进程参数
 *------------------------------------------------------------------*/
void *CommandProtocolRecvSend_thread_Handler(void);

/*------------------------------------------------------------------
* Func : CommandProtocolOpen
* Func :关闭数据交互协议
* fd :  设备ID
*------------------------------------------------------------------*/
int CommandProtocolClose(int fd);

const char * CommandProtocolString(U16 cmd);
void set_cmd_ack(uint8_t ack);
uint8_t get_cmd_ack(void);
int CommandProtocolHeartBeatStatue(int fd,int HeartBeatEnable);
#ifdef __cplusplus
}
#endif
#endif//#ifndef __COMMAND_PROTOCOL_H__


