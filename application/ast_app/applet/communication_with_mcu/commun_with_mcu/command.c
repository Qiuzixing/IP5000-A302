/****************************************************************************
* 版权信息：（C）2013，深圳市拔超科技有限责任公司版权所有
* 系统名称：
* 文件名称：CommandProtocol.cpp
* 文件说明：
* 作    者：xdl
* 版本信息：V1.0
* 设计日期：
* 修改记录：
* 日    期      版    本        修改人      修改摘要
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "uart_mcu_cmd.h"
#include "command.h"
#include "../ipc.h"
#include "../gb_commun_with_mcu.h"
#include "../audio_switch/auto_swtich_socket.h"
#include "../dante_example_code/app/example/example_rx_uhip.h"
int APP_Comm_Recv(CmdProtocolParam * param);
int APP_Comm_Send(U16 CMD,U8 *buf, int len);
extern int ipc_querycmd_index;
extern const ipc_cmd_struct ipc_cmd_list[];
int dante_state = UNKNOW_DANTE_STATUS;

int Cmdfd;
CommandInterfaceFun Cmdfun;
U8 CmdinitFlag=0;//0->nomal  1->init   防止没有初始化就发送指令 
#if (DEBUG_H_EN == 0x00)
E_debugLvl elogLev=ELOG_LVL_ERROR;
#endif
#ifndef STM32_CHIP_EN
int uartfd;
#endif
extern int uart_fd;
extern int sock_fd;

/*------------------------------------------------------------------
 * Func : CommandInterfaceWrite
 * Func : 调用底层发送数据接口
 * buf :  数据buf
 * len :  数据len
 *------------------------------------------------------------------*/
int Example_CommandInterfaceWrite(U8 *buf, int len)
{
    /*调用底层发送数据接口*/
    uart_Write((const char *)buf, len);
    return 0;
}
#if 1
/*------------------------------------------------------------------
 * Func : CommandInterfaceRead
 * Func : 调用底层读取数据接口
 * buf :  数据buf
 * len :  BUF长度
 *------------------------------------------------------------------*/
int Example_CommandInterfaceRead(U8 *buf, int len)
{
    int nRead=0;
    /*调用底层读取数据接口*/
    //nRead = read( uartfd, buf, len);
    nRead = uart_Read(100, buf, len);
    return nRead;
}
#else
/*------------------------------------------------------------------
 * Func : CommandInterfaceRead
 * Func : 中断调用此函数读取数据
 * buf :  数据buf
 * len :  BUF长度
 *------------------------------------------------------------------*/
void Example_CommandInterfaceRead(unsigned char data)
{
    if(CmdinitFlag==0)
    {
        printf("need init");
        return ;
    }
    CommandInterfaceRead2(Cmdfd, (U8)data);
}
#endif

/*------------------------------------------------------------------
 * Func : Example_CommandInterfaceDataProcessing
 * Func : 处理接收命令函数
 * param :  接收到的数据，需要处理的BUF
 *------------------------------------------------------------------*/
void Example_CommandInterfaceDataProcessing(CmdProtocolParam * param)
{
    /*打印 接收到的串口数据*/
    #if CMD_BACKUP_CMD_EN
    //CommandProtocolTransferInfoPrintf(param);
    if(param->CMD == CMD_STM32_11)
    {/* 下面是增加需要长时间处理的函数功能*/
    /*下面函数只会备份命令，处理命令接口 CommandProtocolGetCMDLongerToProgress*/
        if(Cmdfun.CommandProtocolBackupCMDLongerToProgress(Cmdfd,param) < 0)
        APP_Comm_Recv(param);
    }
    else
    #endif
    {
        APP_Comm_Recv(param);
    }
}

/*------------------------------------------------------------------
 * Func : Example_CommandInterfaceDataSend
 * Func : 处理发送命令函数
 * CMD : 命令
 * buf :  数据buf
 * len :  BUF长度
 *------------------------------------------------------------------*/
int Example_CommandInterfaceDataSend(U16 CMD,U8 *buf, int len)
{
    CmdProtocolParam  param;
    unsigned char *buffer;
    memset(&param.startflag,0,sizeof(CmdProtocolParam));
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" init (error)\r\n");
        return -1;
    }

    param.CMD=CMD;
    param.DataLen=len;
    if(len>DEFAULT_DATA_LEN)
    {
        buffer=(unsigned char *)malloc(len);
        memcpy(buffer,buf,len);
        param.Data = (U32)buffer;
    }
    else
    {
        memcpy((unsigned char *)&param.Data,buf,len);
    }

    if(CommandProtocolSendCmdAdd(Cmdfd ,&param,0x00) < 0)
    {
        if(len>DEFAULT_DATA_LEN)
        free(buffer);
    }
    else
    {
        #if (CMD_AGING_TEST)
        printf("==>CommandProtocolSendCmdAdd(ok)  %d addr=0x%x " ,param.DataLen , param.Data);
        #endif
        return 0x00;
    }
return -1;
}
/*------------------------------------------------------------------
* Func : Example_TransferFileSend
* Func :   开始准备发送文件
* fd :  设备ID
* DeviceID :  传输文件ID , 文件标示符
* SrcPath :  源文件路径
* DstPath :  目标文件路径
* return :   0x00 -> ok    -1 -> NG
*------------------------------------------------------------------*/
int Example_TransferFileSend(U32 DeviceID , char * SrcPath, char * DstPath)
{
    #if CMD_SEND_FILE_EN
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return -1;
    }
    return Cmdfun.CommandProtocolTransferFileSendStart(Cmdfd , DeviceID , SrcPath , DstPath);
    #else
    return -1;
    #endif
}

/*------------------------------------------------------------------
 * Func : Example_HeartBeatStatue
 * Func : 获得心跳包连接状态
 * return  :   0-> lose  1-> link
 *------------------------------------------------------------------*/
int Example_HeartBeatStatue(void)
{
    #if CMD_HEART_BEAT_EN
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return -1;
    }
    return CommandProtocolHeartBeatStatue(Cmdfd,0x01) ;
    #else
    return 0x00;
    #endif
}
/*------------------------------------------------------------------
 * Func : Example_FileProgress
 * Func : 获取文件传输进度
    * return  :  进度[0~100]
 *------------------------------------------------------------------*/
int Example_FileProgress(void)
{
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return -1;
    }
    #if CMD_SEND_FILE_EN
    return CommandProtocolTransferFileProgress(Cmdfd) ;
    #else
    return -1;
    #endif
}
/*------------------------------------------------------------------
 * Func : Example_SendBufNum
 * Func : 获取是否还有命令等待发送。
* return  :  发送命令条数  ,-2  error
 *------------------------------------------------------------------*/
int Example_SendBufNum(void)
{
    return CommandProtocolGetSendBufNum(Cmdfd) ;
}
void Example_SendWait10ms(U32 times)//10ms
{
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return ;
    }
    do
    {
        if(Example_SendBufNum()== 0)
        break;
        CommandProtocolThreadDelay(10);// 10ms
    }while(times--);
}

    /*------------------------------------------------------------------
    * Func : CommandProtocolReset
    * Func :复位协议
    * fd :  设备ID
    * return  :   0-> lose  1-> link
    *------------------------------------------------------------------*/
int Example_CommandProtocolReset(void)
{
    if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return -1;
    }
    return CommandProtocolReset(Cmdfd) ;
}
/*------------------------------------------------------------------
 * Func : STM32_CommandInterface_Close
 * Func : 关闭数据 协议
 *------------------------------------------------------------------*/
void Example_CommandInterface_Close(void)
{
     if(CmdinitFlag ==0)
    {/*一定要初始化话才能发送命令*/
        printf(" please init (error)\r\n");
        return ;
    }
     CmdinitFlag = 0x00;
   CommandProtocolClose(Cmdfd);
}

/*------------------------------------------------------------------
 * Func : Example_CommandInterface_Init
 * Func : 初始化数据 协议
 * return :创建设备FD,  
 * return : -1 错误
 *------------------------------------------------------------------*/
void Example_CommandInterface_Init( )
{
    Cmdfd=CommandProtocolOpen(&Cmdfun , "mcu" , COMM_UART);
    if(Cmdfd>=0)
    {
        CommandInterfaceParam data;
        data.key.keyEn = 0x00;
        data.key.KeySequence = 0x00;
        data.DevSrc = CMD_DEV_STM32;
        data.DevDST = CMD_DEV_IMX6UL;
        data.debuglvl = CMD_DEBUG ;// CMD_DEBUG;
        CommandProtocolSetParam(Cmdfd , &data);
        CommandProtocolGetSendBufNum(Cmdfd );
        CmdinitFlag=1;
    }
}

static int socket_msg_struct_conver(socket_msg *msg,char port,char is_connect,char is_valid)
{
    int ret = 0;
    switch(port)
    {
        case HDMIRX1:
            msg->source = "hdmi1";
            break;
        case HDMIRX2:
            msg->source = "hdmi2";
            break;
        case HDMIRX3:
            msg->source = "hdmi3";
            break;
        default:
            ret = -1;
            break;
    }

    if(is_connect == 1)
    {
        msg->type = "plugin";
    }
    else if(is_connect == 0)
    {
        msg->type = "plugout";
    }
    else
    {

    }
    
    if(is_valid == 1)
    {
        msg->type = "signal-valid";
    }
    else if(is_valid == 0)
    {
        msg->type = "signal-invalid";
    }
    else
    {

    }
    
    return ret;
}

int APP_Comm_Recv(CmdProtocolParam * param)
{

    struct CmdDataEDID edid_data;
    // 2. hdcp
    struct CmdDataHDCPCap hdcp_cap;
    struct CmdDataHDCPStatus hdcp_status;
    struct CmdDataHDCPMode hdcp_mode;
    // 3. video
    struct CmdDataLinkStatus vdo_link;
    struct CmdDataVideoStatus vdo_status;
    struct CmdDataVideoMode vdo_mode;
    struct CmdDataInputSorce vdo_source;
    // 4. audio
    struct CmdDataAudioInsertAndExtract ado_insert;
    struct CmdDataAudioControl ado_mode;
    struct CmdDataAudioStatus ado_status;
    struct CmdDataGpioCfg *gpio_cfg = NULL;
    struct CmdDataGpioVal *gpio_val= NULL;
    struct CmdDataGpioList *gpio_list= NULL;
    struct CmdDataUartPassthrough *uart_pass = NULL;

    socket_msg send_socket_msg;
    int i = 0;
    ipc_relay_msg ipc_msg;
    memset(&ipc_msg, 0, sizeof(ipc_msg));
    uint8_t dante_data_buf[DANTE_UART_BUFFER] = {0};
    U16 uart_data_len = 0;
    switch(param->CMD)
    {
        case CMD_UPDATE_MCU_STATUS:
        case CMD_UPDATE_MCU_VERSION:
        case CMD_UPDATE_READY_TO_BOOT:
        case CMD_UPDATE_READY_REC_FILE:
        case CMD_UPDATE_CRC:
            update(param);
            break;
        case EVENT_HDCP_STATUS:
            memset(&hdcp_status, 0, sizeof(hdcp_status));
            memcpy(&hdcp_status, &param->Data, sizeof(hdcp_status));
            printf("port[0x%x] hdcp isEncrypted[0x%x] status[0x%x]\n", hdcp_status.port, hdcp_status.isEncrypted, hdcp_status.status);
            break;
            break;
        case EVENT_HDMI_LINK_STATUS:
            memset(&vdo_link, 0, sizeof(vdo_link));
            memcpy(&vdo_link, &param->Data, sizeof(vdo_link));
            printf("port[0x%x] connect [0x%x] isHpd [0x%x]\n", vdo_link.port, vdo_link.isConnect,vdo_link.isHpd);
            if(0 == socket_msg_struct_conver(&send_socket_msg,vdo_link.port,vdo_link.isConnect,-1))
            {
                sendEvent(sock_fd,send_socket_msg.type,send_socket_msg.source);
            }
            
            break;
        case EVENT_HDMI_VIDEO_STATUS:
            memset(&vdo_status, 0, sizeof(vdo_status));
            memcpy(&vdo_status, (struct CmdDataVideoStatus *)param->Data, sizeof(vdo_status));
            printf("port[0x%x] stable [0x%x]\n", vdo_status.port, vdo_status.isStable); 
            if(0 == socket_msg_struct_conver(&send_socket_msg,vdo_status.port,-1,vdo_status.isStable))
            {
                sendEvent(sock_fd,send_socket_msg.type,send_socket_msg.source);
            }
            if(vdo_status.isStable == 1)
            {
                audioSendEventMsg(sock_fd,"plugin","hdmi");
            }
            else if(vdo_status.isStable == 0)
            {
                audioSendEventMsg(sock_fd,"plugout","hdmi");
            }
            else
            {
                /* code */
            }
            
            
            break;
        case EVENT_HDMI_EDID:
            memset(&edid_data, 0, sizeof(edid_data));
            memcpy(&edid_data,(struct CmdDataEDID *)param->Data, sizeof(edid_data));
            if(ipc_querycmd_index > 0 && ipc_cmd_list[ipc_querycmd_index - 1].a30_cmd == EVENT_HDMI_EDID)
            {
                memcpy(ipc_msg.rawEdid, edid_data.rawEdid, sizeof(ipc_msg.rawEdid));
                ipc_set(IPC_RELAY_CH,&ipc_msg,sizeof(ipc_msg));
                ipc_querycmd_index = -1;
            }
            break;
        case EVENT_HDCP_CAP:
            memset(&hdcp_cap, 0, sizeof(hdcp_cap));
            memcpy(&hdcp_cap, &param->Data, sizeof(hdcp_cap));
            printf("port[0x%x] hdcp cap[0x%x]\n", hdcp_cap.port, hdcp_cap.cap);
            
            if(ipc_querycmd_index > 0 && ipc_cmd_list[ipc_querycmd_index - 1].a30_cmd == EVENT_HDCP_CAP)
            {
                ipc_msg.port = hdcp_cap.port;
                ipc_msg.cap = hdcp_cap.cap;
                ipc_set(IPC_RELAY_CH,&ipc_msg,sizeof(ipc_msg));
                ipc_querycmd_index = -1;
            }
            break;
        case CMD_UART_PASSTHROUGH:
            memcpy(dante_data_buf,(const char*)param->Data,param->DataLen);
            uart_data_len = (dante_data_buf[3]<<8 | dante_data_buf[2]);
            //the data start from the dante_data_buf[4]
            handle_uhip_rx(dante_data_buf+4,uart_data_len);
            if(dante_state == DANTE_AUDIO_DETECTED)
            {
                printf("dante audio connect\n");
                audioSendEventMsg(sock_fd,"plugin","dante");
            }
            else if(dante_state == DANTE_AUDIO_DISCONNECT)
            {
                printf("dante audio disconnect\n");
                audioSendEventMsg(sock_fd,"plugout","dante");
            }
            else
            {
                //
            }
            dante_state = UNKNOW_DANTE_STATUS;
            break;
        default:
            printf("warning:known mcu cmd\n");
            break;
    }

    return 0;
}

int APP_Comm_Send(U16 CMD,U8 *buf, int len)
{
    int ret;
    ret=Example_CommandInterfaceDataSend(CMD,buf,len);
    if(ret >=0 )
    {
         set_cmd_ack(0);
        printf("add CMD=0x%x len=%d(OK)\r\n" , CMD,len);
    }
    else
    {/*命令没有添加, 等待1S 再重新发送*/
        #if(CMD_ADD_CMD_DELAY_EN == 0x00)
        Example_SendWait10ms(100);
        ret=Example_CommandInterfaceDataSend(CMD,buf,len);
        if(ret >=0 )
        {
            printf("add CMD=0x%x len=%d(OK)\r\n" , CMD,len);
        }
        else
        #endif
        {/*打印此，表示命令没有增加进协议，不会发送到远端*/
            printf("add CMD=0x%x len=%d (error )\r\n" , CMD,len);
        }
    }
    return ret;
}

void Example_thread_Handler(void)
{

        // 1.
        #if CMD_BACKUP_CMD_EN
      CmdProtocolParam cmdParam;
        if(Cmdfun.CommandProtocolGetCMDLongerToProgress(Cmdfd,&cmdParam) >= 0)
        {/* 下面是增加需要长时间处理的函数功能*/
            APP_Comm_Recv(&cmdParam);
            if(cmdParam.DataLen > 0x04)
            free((U8*)cmdParam.Data);
        }
        #endif

        #if(CMD_SYSTEM_EN == 0x00)
        /*不跑系统，下面需要main函数中轮询*/

        if(CmdinitFlag == 0x01)
        CommandProtocolRecvSend_thread_Handler();
        #endif

        // 5.  sleep
        //CommandProtocolThreadDelay(10);// 10ms

}

