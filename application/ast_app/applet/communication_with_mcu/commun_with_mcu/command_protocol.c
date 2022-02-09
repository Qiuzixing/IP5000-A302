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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>


#include "command_protocol.h"
#include "uart_mcu_cmd.h"

#define E_TRANSFER         printf
#define W_TRANSFER        printf
#define I_TRANSFER          printf
#define D_TRANSFER         printf

void Example_CommandInterfaceDataProcessing(CmdProtocolParam * param);
int Example_CommandInterfaceWrite(U8 *buf, int len);
int Example_CommandInterfaceRead(U8 *buf, int len);


CommandInterfaceParam uParam;
extern uint8_t Recv_Cmd_Timeout;

#if(CMD_SYSTEM_EN)
char CommMutexInit=0;// o->power up  1-> init
#ifndef STM32_CHIP_EN
pthread_mutex_t CommMutex;
#else
rt_mutex_t CommMutex;
#endif
#endif //#if(CMD_SYSTEM_EN)

#if(CMD_CAL_CRC_SELECT == CAL_CRC16_TABLE)
static const U16 crc16_tab[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0, 
};

U16 CheckSum_crc16(U16 cksum, unsigned char *buf, int len)
{
    int i;
    for (i = 0;  i < len;  i++) {
        cksum = crc16_tab[((cksum>>8) ^ *buf++) & 0xFF] ^ (cksum << 8);
    }
    return cksum;
}
#elif(CMD_CAL_CRC_SELECT == CAL_CRC8_TABLE)
static const unsigned char crc_table[] =
{
    0x00,0x07,0x0E,0x09,0x1C,0x1B,0x12,0x15,0x38,0x3F,0x36,0x31,0x24,0x23,0x2A,0x2D,   
    0x70,0x77,0x7E,0x79,0x6C,0x6B,0x62,0x65,0x48,0x4F,0x46,0x41,0x54,0x53,0x5A,0x5D,   
    0xE0,0xE7,0xEE,0xE9,0xFC,0xFB,0xF2,0xF5,0xD8,0xDF,0xD6,0xD1,0xC4,0xC3,0xCA,0xCD,   
    0x90,0x97,0x9E,0x99,0x8C,0x8B,0x82,0x85,0xA8,0xAF,0xA6,0xA1,0xB4,0xB3,0xBA,0xBD,   
    0xC7,0xC0,0xC9,0xCE,0xDB,0xDC,0xD5,0xD2,0xFF,0xF8,0xF1,0xF6,0xE3,0xE4,0xED,0xEA,   
    0xB7,0xB0,0xB9,0xBE,0xAB,0xAC,0xA5,0xA2,0x8F,0x88,0x81,0x86,0x93,0x94,0x9D,0x9A,   
    0x27,0x20,0x29,0x2E,0x3B,0x3C,0x35,0x32,0x1F,0x18,0x11,0x16,0x03,0x04,0x0D,0x0A,   
    0x57,0x50,0x59,0x5E,0x4B,0x4C,0x45,0x42,0x6F,0x68,0x61,0x66,0x73,0x74,0x7D,0x7A,   
    0x89,0x8E,0x87,0x80,0x95,0x92,0x9B,0x9C,0xB1,0xB6,0xBF,0xB8,0xAD,0xAA,0xA3,0xA4,   
    0xF9,0xFE,0xF7,0xF0,0xE5,0xE2,0xEB,0xEC,0xC1,0xC6,0xCF,0xC8,0xDD,0xDA,0xD3,0xD4,   
    0x69,0x6E,0x67,0x60,0x75,0x72,0x7B,0x7C,0x51,0x56,0x5F,0x58,0x4D,0x4A,0x43,0x44,   
    0x19,0x1E,0x17,0x10,0x05,0x02,0x0B,0x0C,0x21,0x26,0x2F,0x28,0x3D,0x3A,0x33,0x34,   
    0x4E,0x49,0x40,0x47,0x52,0x55,0x5C,0x5B,0x76,0x71,0x78,0x7F,0x6A,0x6D,0x64,0x63,   
    0x3E,0x39,0x30,0x37,0x22,0x25,0x2C,0x2B,0x06,0x01,0x08,0x0F,0x1A,0x1D,0x14,0x13,   
    0xAE,0xA9,0xA0,0xA7,0xB2,0xB5,0xBC,0xBB,0x96,0x91,0x98,0x9F,0x8A,0x8D,0x84,0x83,   
    0xDE,0xD9,0xD0,0xD7,0xC2,0xC5,0xCC,0xCB,0xE6,0xE1,0xE8,0xEF,0xFA,0xFD,0xF4,0xF3
};
unsigned char cal_crc8_table(unsigned char crc, unsigned char *ptr, int len) 
{
 
    while (len--)
    {
        crc = crc_table[crc ^ *ptr++];
    }
    return (crc);
}
#elif(CMD_CAL_CRC_SELECT == CAL_CRC8)
unsigned char cal_crc8(unsigned char crc, unsigned char *ptr, int len)
{
    unsigned char i;
    crc = 0;
    while(len--)
    {
        crc ^= *ptr++;
        for(i = 0;i < 8;i++)
        {
            if(crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc; 
}
#endif
static int CommandProtocolSendCmdAddsub(CommandInterfaceParam *Parms , CmdProtocolParam * cmdparam , U8 mode);


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))
#endif

#if(CMD_ENCRYPTION_EN)
/*1.0定义密码序列*/
static const char *KeySequenceTable[]=
{
"123456789",
"abcdefg",
};

/**
* \brief          ARC4 context structure
*/
typedef struct
{
    int x;                      /*!< permutation index */
    int y;                      /*!< permutation index */
    unsigned char m[256];       /*!< permutation table */
}
arc4_context;
/*
* ARC4 key schedule
*/
void arc4_setup(arc4_context *ctx, const unsigned char *key, int keylen)
{
    int i, j, k, a;
    unsigned char *m;

    ctx->x = 0;
    ctx->y = 0;
    m = ctx->m;

    for (i = 0; i < 256; i++)
        m[i] = (unsigned char)i;

    j = k = 0;

    for (i = 0; i < 256; i++, k++)
    {
        if (k >= keylen) k = 0;

        a = m[i];
        j = (j + a + key[k]) & 0xFF;
        m[i] = m[j];
        m[j] = (unsigned char)a;
    }
}

/*
* ARC4 cipher function
*/
int arc4_crypt(arc4_context *ctx, int length, const unsigned char *input, unsigned char *output)
{
    int i, x, y, a, b;
    unsigned char *m;

    x = ctx->x;
    y = ctx->y;
    m = ctx->m;

    for (i = 0; i < length; i++)
    {
        x = (x + 1) & 0xFF; a = m[x];
        y = (y + a) & 0xFF; b = m[y];

        m[x] = (unsigned char)b;
        m[y] = (unsigned char)a;

        output[i] = (unsigned char)
            (input[i] ^ m[(unsigned char)(a + b)]);
    }

    ctx->x = x;
    ctx->y = y;

    return(0);
}


static int Arc4DecryptEncrypt(const unsigned char *input, unsigned char *output, int len,int keyIndex)
{/*加密、解密*/
    // TODO:  在此添加控件通知处理程序代码
    arc4_context arc4;
    if(keyIndex >= ARRAY_SIZE(KeySequenceTable))
    {
        memcpy(output , input , len);
        E_TRANSFER("keyIndex error");
        return -1;
    }
    // I_TRANSFER("keyIndex=%d,len=%d,%s\r\n", keyIndex,strlen(KeySequenceTable[keyIndex]),KeySequenceTable[keyIndex]);
    memset((char*)output, 0, len);
    arc4_setup(&arc4, (const unsigned char *)KeySequenceTable[keyIndex], strlen(KeySequenceTable[keyIndex]));
    arc4_crypt(&arc4, len, input, output);
    return 0;
}
#endif

static U32 CommandProtocolGetSystemMs(void)
{
#ifdef STM32_CHIP_EN// stm32 system time
    //return rt_tick_get()*10; // stm32 system time
    return rt_tick_get()*10; // stm32 system time
#else// linux system time
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif

}
void CommandProtocolThreadDelay(U32 delayms)
{
#ifdef STM32_CHIP_EN
    rt_thread_delay(ms_to_tick(delayms));
#else
    usleep(delayms*1000);
#endif

}

static void CommandProtocolBufPrintf(uint8_t * buf, int len)
{
    int i;
    if(len)
    {
        for(i=0; i<len ; i++)
        printf("%02x.",buf[i]);
        printf("\r\n");
    }
}
void CommandProtocolTransferInfoPrintf(CmdProtocolParam * param)
{
    int i=0;
    unsigned char * buf;
    buf=(unsigned char *)param;
    CommandProtocolBufPrintf(buf, sizeof(CmdProtocolParam) - 4);
    if(param->DataLen > DEFAULT_DATA_LEN)
    {
    buf=(unsigned char *)param->Data;
    for(i=0; i<param->DataLen; i++)
        {
        if(i%0x10==0)
        printf("\r\n");
        printf("%02x.",buf[i]);
        }
    }
    else
    {
        printf("0x%lx\r\n",param->Data);
    }
    printf("\r\n");
}
/*------------------------------------------------------------------
 * Func : CommandProtocolParam
 * Func : 中断一个BYTE的接收串口数据
 * fd :  设备ID
 *------------------------------------------------------------------*/


/*------------------------------------------------------------------
 * Func : CommandProtocolRecvOneByte
 * Func : 中断一个BYTE的接收串口数据
 * c :  接收的数据
 *------------------------------------------------------------------*/
static int CommandProtocolRecvOneByte(void *ctx , unsigned char c)
{/*增加在串口接收*/
    CommandInterfaceParam* Parms=(CommandInterfaceParam*)ctx;
    CmdProtocolParam *temp = (CmdProtocolParam *)(Parms->RecvTempbuf);

    if(Parms->recvTempIndex==0)
    {
        if(c==CMD_START_FLAG)
        {
            Parms->RecvTempbuf[Parms->recvTempIndex++]=c;
        }
        else
        {/* 数据开始标记判断不对*/
            Parms->recvTempIndex=0;
            return -1;
        }
    }
    else if(Parms->recvTempIndex==1)
    {
        if(c==CMD_START_FLAG2)
        {
            Parms->RecvTempbuf[Parms->recvTempIndex++]=c;
        }
        else if(c==CMD_START_FLAG)
        {
            Parms->recvTempIndex=0;
            Parms->RecvTempbuf[Parms->recvTempIndex++]=c;
        }
        else
        {/* 数据开始标记判断不对*/
            Parms->recvTempIndex=0;
            return -1;
        }
    }
    else if(Parms->recvTempIndex)
    {
        Parms->RecvTempbuf[Parms->recvTempIndex++]=c;
        if(Parms->recvTempIndex==6)
        {
            if(Tu16(temp->DataLen) > CMD_DATA_MAX_LEN)
            {/* 数据长度判断不对*/
                #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
                printf("<in>[%d]%s:", __LINE__, __FUNCTION__);
                CommandProtocolBufPrintf(Parms->RecvTempbuf, Parms->recvTempIndex);
                #endif

                Parms->recvTempIndex=0;
                E_TRANSFER("%s [%x.%x.%x]",Parms->name  ,temp->startflag,Tu16(temp->DataLen),temp->key.keyEn);
                return -2;
            }
        }
        else if((Parms->recvTempIndex >= sizeof(CmdProtocolParam)) && ((Parms->recvTempIndex-(sizeof(CmdProtocolParam) - 4)) >= min(Tu16(temp->DataLen),CMD_DATA_MAX_LEN)))
        {// recv finish
            int index=0;
            for(index=0; index<CMD_RECV_BUF_SIZE ; index++ )
            {
                if(Parms->CmdRecvBuf[index].startflag != CMD_START_FLAG)
                break;
            }
            if(index == CMD_RECV_BUF_SIZE )
            {/* 没有找到可用的BUF*/
                #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
                printf("<in>[%d]%s:", __LINE__, __FUNCTION__);
                CommandProtocolBufPrintf(Parms->RecvTempbuf, Parms->recvTempIndex);
                #endif
                Parms->recvTempIndex=0;
                E_TRANSFER("%s [%x.%x.%x]",Parms->name  ,temp->startflag,Tu16(temp->DataLen),temp->key.keyEn);
                return -3;
            }

            if(Tu16(temp->DataLen) > DEFAULT_DATA_LEN )
            {
                Parms->CmdRecvBuf[index].Data=(U32)malloc(min(Tu16(temp->DataLen),CMD_DATA_MAX_LEN));
                if(Parms->CmdRecvBuf[index].Data == 0x00)//NULL)
                {/* 系统不能分配内存*/
                    #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
                    printf("<in>[%d]%s:", __LINE__, __FUNCTION__);
                    CommandProtocolBufPrintf(Parms->RecvTempbuf, Parms->recvTempIndex);
                    #endif
                    Parms->recvTempIndex=0;
                    E_TRANSFER("%s [%x.%x.%x]" ,Parms->name  ,temp->startflag,Tu16(temp->DataLen),temp->key.keyEn);
                    return -4;
                }
                memcpy((U8 *)Parms->CmdRecvBuf[index].Data , &Parms->RecvTempbuf[(sizeof(CmdProtocolParam) - 4)] , min(Tu16(temp->DataLen),CMD_DATA_MAX_LEN));
                memcpy(&Parms->CmdRecvBuf[index] , Parms->RecvTempbuf , (sizeof(CmdProtocolParam) - 4));
                #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
                printf("<in>[%d]%s:", __LINE__, __FUNCTION__);
                CommandProtocolBufPrintf(Parms->RecvTempbuf, (sizeof(CmdProtocolParam) - 4));
                CommandProtocolBufPrintf(&Parms->RecvTempbuf[(sizeof(CmdProtocolParam) - 4)], min(Tu16(temp->DataLen),CMD_DATA_MAX_LEN));
                #endif
            }
            else
            {
                memcpy(&Parms->CmdRecvBuf[index] , Parms->RecvTempbuf , sizeof(CmdProtocolParam));
                #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
                printf("<in>[%d]%s:", __LINE__, __FUNCTION__);
                CommandProtocolBufPrintf(Parms->RecvTempbuf, sizeof(CmdProtocolParam));
                #endif
            }
            Parms->recvTempIndex=0;
            //CommandProtocolBufPrintf(Parms->RecvTempbuf, Parms->recvTempIndex);
            //I_TRANSFER(" ok[%x.%x.%x]\r\n" ,temp->startflag,temp->DataLen,temp->key);
            return 0;
        }
    }
    return 0;
}
/*------------------------------------------------------------------
 * Func : CommandProtocolSendCmdBuf
 * Func : 调用底层串口发送
 * index :  指定BUF序列号
 *------------------------------------------------------------------*/
static int  CommandProtocolSendCmdBuf(void *ctx , U16 index)
{
    CommandInterfaceParam* Parms=(CommandInterfaceParam*)ctx;
    #if (CMD_DEBUG == CMD_DEBUG_ALL)
    Parms->debugsendtimes++;
    #endif
#if CMD_SWAP_ENDIAN
    CmdProtocolParam uctemp;
    memcpy(&uctemp , (U8 *)&Parms->CmdSendBuf[index].startflag ,sizeof(CmdProtocolParam));
    uctemp.DataLen = Tu16(uctemp.DataLen);
    uctemp.checksum = Tu16(uctemp.checksum);


    if(Parms->CmdSendBuf[index].DataLen > DEFAULT_DATA_LEN)
    {
        Example_CommandInterfaceWrite( (U8 *)&uctemp.startflag , sizeof(CmdProtocolParam)-4);
        Example_CommandInterfaceWrite( (U8 *)Parms->CmdSendBuf[index].Data, Parms->CmdSendBuf[index].DataLen);
    }
    else
    {
        Example_CommandInterfaceWrite( (U8 *)&uctemp.startflag , sizeof(CmdProtocolParam));
    }
#else
    if(Parms->CmdSendBuf[index].DataLen > DEFAULT_DATA_LEN)
    {
        Example_CommandInterfaceWrite( (U8 *)&Parms->CmdSendBuf[index].startflag , sizeof(CmdProtocolParam)-4);
        Example_CommandInterfaceWrite( (U8 *)Parms->CmdSendBuf[index].Data, Parms->CmdSendBuf[index].DataLen);
    }
    else
    {
        Example_CommandInterfaceWrite( (U8 *)&Parms->CmdSendBuf[index].startflag , sizeof(CmdProtocolParam));
    }
#endif

    #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
    printf("<out>[%d]%s:", __LINE__, __FUNCTION__);
    if(Parms->CmdSendBuf[index].DataLen > DEFAULT_DATA_LEN)
    {
        CommandProtocolBufPrintf((U8 *)&Parms->CmdSendBuf[index].startflag, (sizeof(CmdProtocolParam) - 4));
        CommandProtocolBufPrintf((U8 *)Parms->CmdSendBuf[index].Data, Parms->CmdSendBuf[index].DataLen);
    }
    else
    {
        CommandProtocolBufPrintf((U8 *)&Parms->CmdSendBuf[index].startflag, sizeof(CmdProtocolParam));
    }
    #endif
#if CMD_HEART_BEAT_EN
    Parms->HeartBeatSendTime=CommandProtocolGetSystemMs();
#endif
    return 0;
}
#if (CMD_RECV_DATA_MODE == 0x00)
/*------------------------------------------------------------------
* Func : CommandInterfaceRead
* Func : 调用底层读取数据给到下面接口
* fd :  设备ID
* buf :  数据buf
* len :  BUF长度
* return :  0x00 成功  -1失败
*------------------------------------------------------------------*/
int  CommandInterfaceRead2(int fd ,U8 data)
{
    int ret=0;
    ret=CommandProtocolRecvOneByte(&uParam , data);
    return ret;
}
#endif
static int CommandProtocolRecvSendAct(CmdProtocolParam*  param)
{
    //CommandInterfaceParam* Parms=(CommandInterfaceParam*)ctx;

#if CMD_SWAP_ENDIAN
    CmdProtocolParam uctemp;
    memcpy(&uctemp , (U8 *)&param->startflag ,sizeof(CmdProtocolParam));
    uctemp.DataLen = Tu16(uctemp.DataLen);
    uctemp.checksum = Tu16(uctemp.checksum);

    if(param->DataLen > DEFAULT_DATA_LEN)
    {
        Example_CommandInterfaceWrite( (U8 *)&uctemp.startflag , sizeof(CmdProtocolParam)-4);
        Example_CommandInterfaceWrite( (U8 *)param->Data, param->DataLen);
    }
    else
    {
        Example_CommandInterfaceWrite( (U8 *)&uctemp.startflag , sizeof(CmdProtocolParam));
    }
#else
    if(param->DataLen > DEFAULT_DATA_LEN)
    { 
        Example_CommandInterfaceWrite( (U8 *)&param->startflag , sizeof(CmdProtocolParam)-4);
        Example_CommandInterfaceWrite( (U8 *)param->Data, param->DataLen);
    }
    else
    { 
        Example_CommandInterfaceWrite( (U8 *)&param->startflag , sizeof(CmdProtocolParam));
    }
#endif
    #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
//    my2_printf("<out>[%d]%s:", __LINE__, __FUNCTION__);
    if(param->DataLen > DEFAULT_DATA_LEN)
    {
        CommandProtocolBufPrintf((U8 *)&param->startflag, (sizeof(CmdProtocolParam) - 4));
        CommandProtocolBufPrintf((U8 *)param->Data, param->DataLen);
    }
    else
    {
        CommandProtocolBufPrintf((U8 *)&param->startflag, sizeof(CmdProtocolParam));
    }
#endif
    return 0;
}

/*------------------------------------------------------------------
 * Func : CommandProtocolClearSendCmdBuf
 * Func : 清除指定数据BUF
 * SequenceNumber :  指定BUF序列号
 * mode :  0x01-清除所有
 *------------------------------------------------------------------*/
static int  CommandProtocolClearSendCmdBuf(void *ctx , U16 SequenceNumber, U8 mode)
{
    int i=0;
    CommandInterfaceParam* Parms=(CommandInterfaceParam*)ctx;
    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
            if((Tu16(Parms->CmdSendBuf[i].SequenceNumber) == SequenceNumber)||(mode==0x01))
            {
                if(Parms->CmdSendBuf[i].DataLen > DEFAULT_DATA_LEN)
                free((U8 *)Parms->CmdSendBuf[i].Data);
                memset(&Parms->CmdSendBuf[i].startflag, 0x00, sizeof(CmdProtocolParam));
                if(mode==0x00)
                return 0x00;
            }
    }
return -1;
}
static int  CommandProtocolSeqToCmd(CommandInterfaceParam *Parms , U16 SequenceNumber)
{
    int i=0;
    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
            if(Tu16(Parms->CmdSendBuf[i].SequenceNumber) == SequenceNumber)
            {
                return Tu16(Parms->CmdSendBuf[i].CMD);
            }
    }
return -1;
}
/*------------------------------------------------------------------
 * Func : CommandProtocolGetSendBufNum
 * Func : 获取当前还有多少命令没有发送完成
* fd :  设备ID
* return  :  发送命令条数
 *------------------------------------------------------------------*/
static int  CommandProtocolGetSendEmptyBufNum(CommandInterfaceParam* Parms)
{
    int ret=0;
    int i=0;
    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
        if(Parms->CmdSendBuf[i].startflag != CMD_START_FLAG)
        {
        ret++;
        }
    }
return ret;
}
/*------------------------------------------------------------------
 * Func : CommandProtocolGetSendBufNum
 * Func : 获取当前还有多少命令没有发送完成
* fd :  设备ID
* return  :  发送命令条数
 *------------------------------------------------------------------*/
int  CommandProtocolGetSendBufNum(int fd)
{
    return CommandProtocolGetSendEmptyBufNum(&uParam);

}
int CommandProtocolWaitBufEmpty(int fd , int waittimeMs)
{
    int delaytimes=waittimeMs/30;
    do
    {
        if(CommandProtocolGetSendBufNum(fd)==0)
        return 0x00;
        // 5.  sleep 30 ms
        CommandProtocolThreadDelay(30);
    }while(delaytimes--);
    return -1;
}


static int CommandProtocolWaitBufRemaining(CommandInterfaceParam* Parms, int waittimeMs)
{
    int delaytimes=waittimeMs/30;
    do
    {
        if(CommandProtocolGetSendEmptyBufNum(Parms) > 0)
        return 0x00;
        // 5.  sleep 30 ms
        CommandProtocolThreadDelay(30);
    }while(delaytimes--);
    return -1;
}



/*------------------------------------------------------------------
* Func : CommandProtocolConfirmCmdsent
* Func : 检测命令是否已经成功发送出去
* fd :  设备ID
* SendSequenceNumber :   发送命令序列
* return :  0x00: 还在发送队列中，  0x01:已经发出    -2: error
*------------------------------------------------------------------*/
static int CommandProtocolConfirmCmdsent(int fd  , int SequenceNumber)
{
    int i=0;
    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
        if(uParam.CmdSendBuf[i].startflag  ==  CMD_START_FLAG)
        {
            if(Tu16(uParam.CmdSendBuf[i].SequenceNumber)  ==  SequenceNumber)
            return 0x00;
        }
    }
return 0x01;
}
#if(CMD_LARGEPACKET_PROTOCOL)
int CommandProtocolSendLargePacket(CommandInterfaceParam* Parms,  CmdProtocolParam * cmdparam )
{
    int i;
    CmdProtocolParam  cmdData;
    LargePacketProtocol  Packets;
    U8* srcbuf = (U8*)cmdparam->Data;
    U8* buffer;
    U16 PacketSize=(CMD_DATA_MAX_LEN-sizeof(LargePacketProtocol));
    
    memcpy(&cmdData, cmdparam, sizeof(CmdProtocolParam));
    cmdData.CMD = CMD_LARGE_PACKET_PROTOCOL;
    
    Parms->LargePacket.PacketsNum = cmdparam->DataLen /PacketSize;
    if(cmdparam->DataLen > (PacketSize * Parms->LargePacket.PacketsNum))
        Parms->LargePacket.PacketsNum++;
    Parms->LargePacket.PacketsLen=cmdparam->DataLen;
    Parms->LargePacket.SendCMD=cmdparam->CMD;

    for(i=0; i<Parms->LargePacket.PacketsNum ;i++)
        {
            memcpy(&Packets ,&Parms->LargePacket , sizeof(LargePacketProtocol));
            if(i == (Packets.PacketsNum-1))
            {
                Packets.SendAddr=i*PacketSize;
                Packets.SendLen=Packets.PacketsLen - Packets.SendAddr;
            }
            else
            {
                Packets.SendAddr=i*PacketSize;
                Packets.SendLen=PacketSize;
            }
            
            cmdData.DataLen=Packets.SendLen + sizeof(LargePacketProtocol)-4;
            buffer = (U8*)malloc(cmdData.DataLen);
            if(buffer == NULL)
            {
                free(buffer);
                E_TRANSFER("%s" ,Parms->name);
                return -1;
            }
            memcpy(buffer ,&Packets , sizeof(LargePacketProtocol)-4);
            memcpy(&buffer[sizeof(LargePacketProtocol)-4] ,&srcbuf[Packets.SendAddr] ,Packets.SendLen);
            cmdData.Data=(U32)buffer;
            if(CommandProtocolWaitBufRemaining(Parms,1000)<0)
            {
                free(buffer);
                E_TRANSFER("%s <%d>" ,Parms->name,i);
                return -1;
            }
            
            if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
            {
                I_TRANSFER("PacketsNum=%d ,PacketsLen=%d,cmdlen=%d" , Packets.PacketsNum, Packets.PacketsLen, cmdData.DataLen);
                I_TRANSFER("SendAddr=%d,SendLen=%d" ,Packets.SendAddr, Packets.SendLen);
            }
            if(CommandProtocolSendCmdAddsub(Parms , &cmdData , 1)<0)
            {
                free(buffer);
                E_TRANSFER("%s" ,Parms->name);
                return -1;
            }
        }
    return 0x00;
}
int CommandProtocolRecvLargePacket(CommandInterfaceParam* Parms, CmdProtocolParam * cmdparam )
{
    U8* buffer=(U8*)cmdparam->Data;
    LargePacketProtocol * Packets = (LargePacketProtocol *)cmdparam->Data;
    if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
    {
        I_TRANSFER("PacketsNum=%d ,PacketsLen=%d,cmdlen=%d" , Packets->PacketsNum, Packets->PacketsLen, cmdparam->DataLen);
        I_TRANSFER("SendAddr=%d,SendLen=%d" ,Packets->SendAddr, Packets->SendLen);
    }
    if(Parms->LargePacket.buf == NULL)
    {
        Parms->LargePacket.buf = (U8*)malloc(Packets->PacketsLen);
        if(Parms->LargePacket.buf == NULL)
        {
            E_TRANSFER("%s" ,Parms->name);
            return -1;
        }
    }

    if(Packets->PacketsNum)
    {
        memcpy(&Parms->LargePacket.buf[Packets->SendAddr] ,&buffer[sizeof(LargePacketProtocol)-4] ,Packets->SendLen);
        Parms->LargePacket.PacketsRecvNum++;
        
        if(Parms->LargePacket.PacketsRecvNum ==Packets->PacketsNum)
        {
            CmdProtocolParam  cmdData;
            memcpy(&cmdData, cmdparam, sizeof(CmdProtocolParam));
            cmdData.CMD = Packets->SendCMD;
            cmdData.DataLen= Packets->PacketsLen;
            cmdData.Data = (U32)Parms->LargePacket.buf;
            Example_CommandInterfaceDataProcessing(&cmdData);
            {
            free(Parms->LargePacket.buf);
            Parms->LargePacket.buf=NULL;
            }
            Parms->LargePacket.PacketsRecvNum=0;
        }
        return 0x00;
    }
    return -1;
}
#endif

#if CMD_SEND_FILE_EN
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileReportCmd
* Func :  报告文件传输状态
* Parms :  传输参数
* return :   0x00 -> ok    -1 -> NG
*------------------------------------------------------------------*/
int CommandProtocolTransferFileReportCmd(CommandInterfaceParam* Parms , U32 date)
{
    I_TRANSFER("%s %d" ,Parms->name ,(int)date);
    CmdProtocolParam  Cmd_param;
    Cmd_param.CMD=CMD_UPDATE_SW_REPORT_FILE;
    Cmd_param.DataLen=4;
    Cmd_param.Data=date;
    if(CommandProtocolSendCmdAddsub(Parms ,&Cmd_param,0x01) < 0)
    {
        E_TRANSFER("%s" ,Parms->name );
        return -1;
    }
    return 0;
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileStartCmd
* Func :  开始发送文件命令
* Parms :  传输参数
* return :   0x00 -> ok    -1 -> NG
*------------------------------------------------------------------*/
int CommandProtocolTransferFileStartCmd(CommandInterfaceParam* Parms )
{
    unsigned char *buffer;
    int i=0;
    CmdProtocolParam  Cmdparam;
    Cmdparam.CMD=CMD_UPDATE_SW_SEND_FILE_START;
    Cmdparam.DataLen=sizeof(File_Info_Param);
    buffer=(unsigned char *)malloc(Cmdparam.DataLen);
    File_Info_Param FileInfo;
    memcpy(&FileInfo,(U8 *)&Parms->FileInfo,Cmdparam.DataLen);
    memcpy(FileInfo.FileName, Parms->DstPath , CMD_PATH_NAME_LEN);
    memcpy(buffer,(U8 *)&FileInfo,Cmdparam.DataLen);
    Cmdparam.Data = (U32)buffer;
    
    
    if(CommandProtocolSendCmdAddsub(Parms ,&Cmdparam,0x01) < 0)
    {
    free(buffer);
    E_TRANSFER("error [%s][%s]", Parms->name ,Parms->FileInfo.FileName);
    return -1;
    }

     /*3.设置升级标志*/
    Parms->FileTransferSendStatus = FILE_TRANSFER_START_SEND;
    Parms->key.keyEn= 0;
    Parms->key.KeySequence= 0;
    Parms->FileTransferProgress=0;
    if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
    {
    I_TRANSFER("==>[%s]%s len=[%d]   Packets=%d,crc=%d\n" ,Parms->name, Parms->FileInfo.FileName ,Parms->FileInfo.FileLen , Parms->FileInfo.PacketsNum,Parms->FileInfo.FileCrc);
    }
    for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
    Parms->RecordProcessing[i] = 0x3FFF;

    return 0;
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferCheckFileInfo
* Func :  计算文件信息
* Path :  文件路径
* FileInfo :  文件信息
* return :   0x00 -> ok    -1 -> NG
*------------------------------------------------------------------*/
#if(CMD_SEND_FILE_MODE)
int CommandProtocolTransferCheckFileInfo(char * Path,File_Info_Param* FileInfo)
{
    U32 addr=0;
    int fd;
    U32 i;
    unsigned char *buffer;
    buffer=(unsigned char *)malloc(CMD_DATA_MAX_LEN);

    fd = open((const char*)Path, O_RDWR|O_BINARY ,NULL);
    if (fd < 0)
    {
    E_TRANSFER("open error [%s]",Path);
    free(buffer);
    return -1;
    }
    FileInfo->FileLen=lseek(fd, 0, SEEK_END);

    FileInfo->PacketsNum = (FileInfo->FileLen/CMD_DATA_MAX_LEN);
    if(FileInfo->FileLen%CMD_DATA_MAX_LEN)
        FileInfo->PacketsNum+=1;
    
    U16 checksum=0;
    for(i=0;i<FileInfo->PacketsNum ; i++)
    {
        addr=CMD_DATA_MAX_LEN*i;
        lseek(fd, addr, SEEK_SET);
        if((addr+CMD_DATA_MAX_LEN) < FileInfo->FileLen)
        {
            read(fd , buffer, CMD_DATA_MAX_LEN);
            checksum=CheckSum_crc16(checksum ,buffer , CMD_DATA_MAX_LEN);
        }
        else
        {
            addr = FileInfo->FileLen -addr;
            read(fd , buffer, addr);
            checksum=CheckSum_crc16(checksum ,buffer , addr);
            break;
        }
    }
    free(buffer);
    FileInfo->FileCrc = checksum;
    close(fd);
return 0x00;
}
#else
int CommandProtocolTransferCheckFileInfo(char * Path,File_Info_Param* FileInfo)
{
    U32 addr=0;
    FILE *fd;
    U32 i;
    unsigned char *buffer;
    buffer=(unsigned char *)malloc(CMD_DATA_MAX_LEN);
    #ifdef STM32_CHIP_EN
    int fd2 = open(Path, O_RDONLY, 0);
    FileInfo->FileLen=lseek(fd2, 0, SEEK_END);
    close(fd2);
    #endif
    fd = fopen((const char*)Path, "rb+");
    if (fd==NULL)
    {
    E_TRANSFER("open error [%s]",Path);
    free(buffer);
    return -1;
    }
    #ifndef STM32_CHIP_EN
    fseek(fd, 0, SEEK_END);
    FileInfo->FileLen= ftell(fd);
    #endif

    FileInfo->PacketsNum = (FileInfo->FileLen/CMD_DATA_MAX_LEN);
    if(FileInfo->FileLen%CMD_DATA_MAX_LEN)
        FileInfo->PacketsNum+=1;
    
    U16 checksum=0;
    for(i=0;i<FileInfo->PacketsNum ; i++)
    {
        addr=CMD_DATA_MAX_LEN*i;
        fseek(fd, addr, SEEK_SET);
        if((addr+CMD_DATA_MAX_LEN) < FileInfo->FileLen)
        {
            fread(buffer, 1, CMD_DATA_MAX_LEN, fd);
            checksum=CheckSum_crc16(checksum ,buffer , CMD_DATA_MAX_LEN);
        }
        else
        {
            addr = FileInfo->FileLen -addr;
            fread(buffer, 1, addr, fd);
            checksum=CheckSum_crc16(checksum ,buffer , addr);
            break;
        }
    }
    free(buffer);
    FileInfo->FileCrc = checksum;
    fclose(fd);
return 0x00;
}
#endif
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileSendStart
* Func :   开始准备发送文件
* fd :  设备ID
* DeviceID :  传输文件ID
* SrcPath :  源文件路径
* DstPath :  目标文件路径
* return :   0x00 -> ok    -1 -> NG
*------------------------------------------------------------------*/
int CommandProtocolTransferFileSendStart(int fd ,U32 DeviceID, char * SrcPath, char * DstPath)
{
     /*3.设置升级标志*/
    uParam.FileTransferSendStatus = FILE_TRANSFER_START_SEND;
    memset(uParam.DstPath ,0x00 ,CMD_PATH_NAME_LEN);
    memcpy(uParam.DstPath , DstPath , strlen(DstPath));
     
    if(CommandProtocolTransferCheckFileInfo(SrcPath , &uParam.FileInfo)<0)
    {
    E_TRANSFER("open [%s] [%s]", uParam.name,SrcPath);
    return -1;
    }
    memset(uParam.FileInfo.FileName ,0x00 ,CMD_PATH_NAME_LEN);
    memcpy(uParam.FileInfo.FileName , DstPath , strlen(DstPath));
    uParam.FileInfo.DeviceID=DeviceID;

    if(CommandProtocolTransferFileStartCmd(&uParam) < 0)
    {
    E_TRANSFER("error [%s] [%s][%s]", uParam.name,SrcPath,DstPath);
    return -1;
    }
    E_TRANSFER("send file[%s] [0x%x][0x%x]", uParam.FileInfo.FileName, uParam.FileInfo.FileLen, uParam.FileInfo.FileCrc);
    
    /*2.发送文件信息*/
    CommandProtocolWaitBufEmpty(fd,300);    
    memset(uParam.FileInfo.FileName ,0x00 ,CMD_PATH_NAME_LEN);
    memcpy(uParam.FileInfo.FileName , SrcPath , strlen(SrcPath));
    if(CommandProtocolWaitBufEmpty(fd,300)<0)
    {
    E_TRANSFER("%s, %d." , uParam.name , fd);
    return -1;
    }
    

     /*3.设置升级标志*/
    uParam.FileTransferSendStatus = FILE_TRANSFER_FILE_SEND;
    uParam.key.keyEn= 0;
    uParam.key.KeySequence= 0;
    uParam.FileTransferProgress=0;
    if(uParam.debuglvl & CMD_DEBUG_FILE_SEND_RECV)
    {
    I_TRANSFER("==>[%s]%s len=[%d]   Packets=%d,crc=%d\n" ,uParam.name, uParam.FileInfo.FileName ,uParam.FileInfo.FileLen , uParam.FileInfo.PacketsNum, uParam.FileInfo.FileCrc);
    }

    return 0;
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileRecvStart
* Func :  准备接收文件
* Parms :  传输参数
* Parms :  接收到的命令数据
*------------------------------------------------------------------*/
static int CommandProtocolTransferFileRecvStart(CommandInterfaceParam* Parms , CmdProtocolParam * Cmdparam)
{
    int i=0;
    File_Info_Param *FileInfo;
    FileInfo = (File_Info_Param *)Cmdparam->Data;
    memcpy((U8 *)&Parms->FileInfo, (U8 *)FileInfo,sizeof(File_Info_Param));
    #if(CMD_SEND_FILE_MODE)
    Parms->TransferFile = open((const char*)Parms->FileInfo.FileName, O_CREAT|O_TRUNC|O_RDWR|O_BINARY , NULL);
    if(Parms->TransferFile < 0)
    {
        Parms->TransferFile = open((const char*)Parms->FileInfo.FileName, O_CREAT|O_TRUNC|O_RDWR|O_BINARY , NULL);
        if(Parms->TransferFile <0)
        {
        E_TRANSFER("open error [%s]" ,Parms->FileInfo.FileName);
        //mkfs -t elm flash0
        dfs_mkfs("elm", "flash0");      //格式化外部Flash
        CommandProtocolThreadDelay(6000);
        APP_SYSReboot();
        //fclose(Parms->TransferFile);
        return 0xFF;
        }
    }
    #else   
    Parms->TransferFile = fopen((const char*)Parms->FileInfo.FileName, "w+");
    if(Parms->TransferFile == NULL)
    {    
    Parms->TransferFile = fopen((const char*)Parms->FileInfo.FileName, "w+");
    if(Parms->TransferFile != NULL)
        {
        //fclose(Parms->TransferFile);
        return 0xFF;
        }
    }
    #endif
    else
    {
    //fclose(Parms->TransferFile);
    } 
    Parms->key.keyEn= 0;
    Parms->key.KeySequence= 0;
    Parms->FileTransferProgress=0;
    Parms->FileTransferRecvStatus = FILE_RECV_FILE;
    for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
    Parms->RecordProcessing[i] = 0x3FFF;
    if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
    {
    I_TRANSFER("==>Recv start [%s]" ,Parms->FileInfo.FileName);
    }
    return 0;
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileSend
* Func :  发送文件
* Parms :  传输参数
*------------------------------------------------------------------*/
static int CommandProtocolTransferFileSend(CommandInterfaceParam* Parms)
{
    int i=0;
    U32 addr=0;
    U8 sendfinishflag=0;// 0-> nomal  1-> finish;
    CmdProtocolParam  Cmdparam;
    unsigned char *buffer;
    if(Parms->FileTransferSendStatus == FILE_TRANSFER_FILE_SEND)
    {
        memset(&Cmdparam.startflag,0,sizeof(CmdProtocolParam));
        Cmdparam.CMD=CMD_UPDATE_SW_SEND_FILE;
        Cmdparam.DataLen=CMD_DATA_MAX_LEN;
        buffer=(unsigned char *)malloc(CMD_DATA_MAX_LEN);
        addr=CMD_DATA_MAX_LEN*Parms->FileTransferProgress;
        #if(CMD_SEND_FILE_MODE)
        Parms->TransferFile = open((const char*)Parms->FileInfo.FileName, O_RDWR ,NULL);
        if(Parms->TransferFile < 0)
        {
            E_TRANSFER("%s [open error  %s]" ,Parms->name ,Parms->FileInfo.FileName);
            free(buffer);
            return -1;
        }

        lseek(Parms->TransferFile, addr, SEEK_SET);
        if((addr+CMD_DATA_MAX_LEN) < Parms->FileInfo.FileLen)
        {
            read(Parms->TransferFile, buffer, CMD_DATA_MAX_LEN);
        }
        else
        {
            Cmdparam.DataLen = Parms->FileInfo.FileLen -addr;
            if(Parms->Version >= 0x0101)
            {
                if(Cmdparam.DataLen <= 0x04)
                Cmdparam.DataLen=CMD_FILE_MIN_BUF_LEN;
            }
            read(Parms->TransferFile,buffer, Cmdparam.DataLen);
            Parms->FileTransferSendStatus=FILE_TRANSFER_WAIT_FINISH_FLAG;
        }
        close(Parms->TransferFile);
        #else
        Parms->TransferFile = fopen((const char*)Parms->FileInfo.FileName, "rb+");
        if(Parms->TransferFile == NULL)
        {
            E_TRANSFER("%s [open error  %s]" ,Parms->name ,Parms->FileInfo.FileName);
            return -1;
        }

        fseek(Parms->TransferFile, addr, SEEK_SET);
        if((addr+CMD_DATA_MAX_LEN) < Parms->FileInfo.FileLen)
        {
            fread(buffer, 1, CMD_DATA_MAX_LEN, Parms->TransferFile);
        }
        else
        {
            Cmdparam.DataLen = Parms->FileInfo.FileLen -addr;
            if(Parms->Version >= 0x0101)
            {
                if(Cmdparam.DataLen <= 0x04)
                Cmdparam.DataLen=CMD_FILE_MIN_BUF_LEN;
            }
            fread(buffer, 1, Cmdparam.DataLen, Parms->TransferFile);
            sendfinishflag=1;
        }
        fclose(Parms->TransferFile);
        #endif
        Cmdparam.Data = (U32)buffer;
        Cmdparam.SequenceNumber = Parms->SendSequenceNumber;

        if(CommandProtocolSendCmdAddsub(Parms ,&Cmdparam,0x00) < 0)
        {
            if(Cmdparam.DataLen > DEFAULT_DATA_LEN)
            free(buffer);
        }
        else
        {
            Parms->FileTransferProgress++;
            if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
            {
                I_TRANSFER("==>Send addr=0x%x len=0x%x[%d,%d] SN=%d\n" , addr , Cmdparam.DataLen,Parms->FileInfo.PacketsNum,Parms->FileTransferProgress,Cmdparam.SequenceNumber);
            }
        }
        if(sendfinishflag)
        {/*1.这个标志一定要在这处理，不能移动到前面
             2.因为 CommandProtocolSendCmdAddsub函数会调用*/
            Parms->FileTransferSendStatus=FILE_TRANSFER_WAIT_FINISH_FLAG;
        }
    }
    else if(Parms->FileTransferSendStatus ==FILE_TRANSFER_WAIT_FINISH_FLAG)
    {
        addr=0;
        for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
        {
            if(Parms->CmdSendBuf[i].startflag == CMD_START_FLAG)
            {
                addr++;
            }
        }
        if(addr==0)
        {/* 发送结束命令 */
            Parms->FileTransferSendStatus=FILE_TRANSFER_FINISH;
            memset(&Cmdparam.startflag,0,sizeof(CmdProtocolParam));
            Cmdparam.CMD=CMD_UPDATE_SW_SEND_FILE;
            Cmdparam.DataLen=4;
            if(CommandProtocolSendCmdAddsub(Parms ,&Cmdparam,0x00) >= 0)
            {
                if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
                {
                    I_TRANSFER("==>[%s] SendFileFinish" , Parms->name);
                }
            }
        
        }
    }
return 0;
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileRecvStart
* Func :  接收文件
* Parms :  传输参数
* Parms :  接收到的命令数据
*------------------------------------------------------------------*/
static int CommandProtocolTransferFileRecv(CommandInterfaceParam* Parms , CmdProtocolParam * Cmdparam)
{
    U32 addr=0;
    unsigned char *buffer;
    buffer = (unsigned char *)Cmdparam->Data;
    U32 FileTransfer_Progress;
    //I_TRANSFER("==>Recv   [%s]" ,Parms->FileInfo.FileName);
    //Parms->TransferFile= fopen((const char*)Parms->FileInfo.FileName, "rb+");
    if(Parms->TransferFile < 0 )
    {
        E_TRANSFER("%s  path=%s" ,Parms->name ,Parms->FileInfo.FileName);
        return 0xFF;
    }
    
    #if(CMD_SEND_FILE_MODE)
    {
        FileTransfer_Progress = Cmdparam->DevSrc*0x100;
        FileTransfer_Progress += Cmdparam->DevDST;

        if(FileTransfer_Progress == FILE_TRANSFER_SEND_END)
        {/* 发送结束命令 */
            Parms->FileTransferRecvStatus=FILE_RECV_FINISH;
            fsync(Parms->TransferFile);
            close(Parms->TransferFile);
        }
        else
        {
            addr=CMD_DATA_MAX_LEN*FileTransfer_Progress;
            lseek(Parms->TransferFile, addr, SEEK_SET);
            if(Parms->Version >= 0x0101)
            {
                if(Cmdparam->DataLen == CMD_FILE_MIN_BUF_LEN)
                {
                    if((Parms->FileInfo.FileLen % CMD_DATA_MAX_LEN) <= CMD_FILE_MIN_BUF_LEN)
                    Cmdparam->DataLen = Parms->FileInfo.FileLen % CMD_DATA_MAX_LEN;
                }
            }
            write(Parms->TransferFile , buffer,  Cmdparam->DataLen);
            //fsync(Parms->TransferFile);
            Parms->FileTransferProgress++;
        }
    }
    #else
    {
        FileTransfer_Progress = Cmdparam->DevSrc*0x100;
        FileTransfer_Progress += Cmdparam->DevDST;

        if(FileTransfer_Progress == FILE_TRANSFER_SEND_END)
        {/* 发送结束命令 */
            Parms->FileTransferRecvStatus=FILE_RECV_FINISH;
            fflush(Parms->TransferFile);
            fclose(Parms->TransferFile);
        }
        else
        {
            addr=CMD_DATA_MAX_LEN*FileTransfer_Progress;
            fseek(Parms->TransferFile, addr, SEEK_SET);
            if(Parms->Version >= 0x0101)
            {
                if(Cmdparam->DataLen == CMD_FILE_MIN_BUF_LEN)
                {
                    if((Parms->FileInfo.FileLen % CMD_DATA_MAX_LEN) <= CMD_FILE_MIN_BUF_LEN)
                    Cmdparam->DataLen = Parms->FileInfo.FileLen % CMD_DATA_MAX_LEN;
                }
            }
            fwrite(buffer, 1, Cmdparam->DataLen, Parms->TransferFile);
            //fflush(Parms->TransferFile);
            Parms->FileTransferProgress++;
        }
    }
    #endif
    if(Parms->FileTransferRecvStatus == FILE_RECV_FINISH)
    {
        File_Info_Param File_Info;
        if(CommandProtocolTransferCheckFileInfo((char*)Parms->FileInfo.FileName, &File_Info)<0)
        {
        E_TRANSFER("open error [%s][%s]" , Parms->name ,Parms->FileInfo.FileName);
        goto filecheckerror;
        }
        E_TRANSFER("Recv file[%s] [0x%x][0x%x]",File_Info.FileName,File_Info.FileLen,File_Info.FileCrc);

        if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
        {
        I_TRANSFER("==> finish addr=%x SN=%d [%d,%d] [%d,%d]\n" , addr , Cmdparam->SequenceNumber ,File_Info.FileLen,Parms->FileInfo.FileLen,File_Info.FileCrc , Parms->FileInfo.FileCrc);
        }
        if((File_Info.FileLen != Parms->FileInfo.FileLen) ||(File_Info.FileCrc!= Parms->FileInfo.FileCrc))
        {
            Parms->FileTransferRecvStatus=FILE_RECV_FINISH_NG;
            #if 0//(CMD_SEND_FILE_MODE)
            if(File_Info.FileLen == 0)
            {
                //mkfs -t elm flash0
                dfs_mkfs("elm", "flash0");      //格式化外部Flash
                CommandProtocolThreadDelay(6000);
                APP_SYSReboot();
            }
            #endif
            goto filecheckerror;
        }
        Parms->FileTransferRecvStatus=FILE_RECV_FINISH_OK;
        CommandProtocolTransferFileReportCmd(Parms ,2);
        return 1;    
    }
    else
    {
        if(Parms->debuglvl & CMD_DEBUG_FILE_SEND_RECV)
        {
        I_TRANSFER("==>Recv addr=0x%x len=0x%x[%d,%d] SN=%d\n" , addr , Cmdparam->DataLen,Parms->FileInfo.PacketsNum,Parms->FileTransferProgress,Cmdparam->SequenceNumber);
        }
    }

    

return 0;    
    filecheckerror:

        CommandProtocolTransferFileReportCmd(Parms , 0);
return 0xFF;
       
}
/*------------------------------------------------------------------
* Func : CommandProtocolTransferFileProgress
* Func :  文件传输进度
* fd :  设备ID
* return  :  进度[0~100]
*------------------------------------------------------------------*/
int CommandProtocolTransferFileProgress(int fd )
{
    U32 uctemp=0;
    uctemp=uParam.FileTransferProgress*100;
    uctemp=uctemp/uParam.FileInfo.PacketsNum;
    
    if(uParam.FileTransferSendStatus>=FILE_TRANSFER_START_SEND&& uParam.FileTransferSendStatus <=FILE_TRANSFER_WAIT_FINISH_FLAG)
    {
        if(uctemp>=100)
            uctemp=99;
    }
    else if(uParam.FileTransferSendStatus == FILE_TRANSFER_FINISH)
    {
            uParam.FileTransferSendStatus=FILE_TRANSFER_NULL;
            uctemp=100;
    }
    
    if((uParam.FileTransferRecvStatus==FILE_RECV_FILE) &&(uctemp>=100))
    {
            uctemp=99;
    }
    else if (uParam.FileTransferRecvStatus == FILE_RECV_FINISH_OK)
    {
            uctemp=100;
    }
   return uctemp; 
}
#endif
/*------------------------------------------------------------------
 * Func : CommandProtocolSendCmdAdd
 * Func :   增加发送的数据
 * Parms :  参数
 * param :  发送命令的BUF
 * mode :  发送模式  0x01->立即发送0x00 -> FIFO 先进先出
 * return :  SendSequenceNumber  用于检测数据是否发出
 * return :  -1 表示刚才的命令没有加入队列中
 *------------------------------------------------------------------*/
static int CommandProtocolSendCmdAddsub(CommandInterfaceParam *Parms , CmdProtocolParam * cmdparam , U8 mode)
{
    int i=0;
    CmdProtocolParam uctemp;
    #if CMD_ENCRYPTION_EN
    unsigned char * buf;
    #endif

    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
        if(Parms->CmdSendBuf[i].startflag  !=  CMD_START_FLAG)
        {
        U16 checksum=0;
        memcpy(&uctemp.startflag , cmdparam, sizeof(CmdProtocolParam));
        /* 1.0  增加固定标示*/
        uctemp.startflag = CMD_START_FLAG;
        uctemp.startflag2 = CMD_START_FLAG2;
        if(cmdparam->CMD==CMD_HEART_BEAT)
        {
            uctemp.SequenceNumber = 0;
        }
        else
        {
            uctemp.SequenceNumber = Parms->SendSequenceNumber++;
        }
        uctemp.key.keyEn =Parms->key.keyEn;
        uctemp.key.KeySequence =Parms->key.KeySequence;
        #if CMD_SEND_FILE_EN
        if(cmdparam->CMD==CMD_UPDATE_SW_SEND_FILE)
        {
            cmdparam->SequenceNumber=uctemp.SequenceNumber;
            if(Parms->FileTransferSendStatus == FILE_TRANSFER_FILE_SEND)
            {
            uctemp.DevSrc = Parms->FileTransferProgress/0x100;
            uctemp.DevDST = Parms->FileTransferProgress%0x100;
            }
            else if(Parms->FileTransferSendStatus == FILE_TRANSFER_FINISH)
            {/* 发送结束命令 */
            uctemp.DevSrc = (FILE_TRANSFER_SEND_END/0x100);
            uctemp.DevDST = (FILE_TRANSFER_SEND_END%0x100);
            }
        }
        else
        #endif
        {
        uctemp.DevSrc =Parms->DevSrc;
        uctemp.DevDST =Parms->DevDST;
        }
        /* 1.4  打印加密前数据*/
        if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
        {
            //mmandProtocolTransferInfoPrintf(&uctemp);
        }
        
        /* 1.5  判断加密标志进行加密*/
        #if CMD_ENCRYPTION_EN
        if(uctemp.key.keyEn)
        {
            buf=(unsigned char *)malloc(min(uctemp.DataLen,CMD_DATA_MAX_LEN));
            if(uctemp.DataLen > DEFAULT_DATA_LEN)
            {
            Arc4DecryptEncrypt((const unsigned char *)uctemp.Data, buf, uctemp.DataLen , uctemp.key.KeySequence);
            memcpy((unsigned char *)uctemp.Data , buf , uctemp.DataLen);
            }
            else
            {
            Arc4DecryptEncrypt((const unsigned char *)&uctemp.DevSrc, buf, 0x08 , uctemp.key.KeySequence);
            memcpy((unsigned char *)&uctemp.DevSrc , buf , 0x08);
            }
            free(buf);
        }
        #endif
        /* 1.6  更新当前checksum*/
        #if CMD_SWAP_ENDIAN
        uctemp.SequenceNumber = Tu16(uctemp.SequenceNumber);
        uctemp.CMD = Tu16(uctemp.CMD);
        if(uctemp.DataLen == DEFAULT_DATA_LEN)
        uctemp.Data = Tu32(uctemp.Data);
        #endif
        
        checksum=0;
        if(uctemp.DataLen > DEFAULT_DATA_LEN)
        {
        checksum=CheckSum_crc16(checksum , (unsigned char *)&uctemp.key, CMD_STRUCT_CRC_LEN);
        checksum=CheckSum_crc16(checksum , (unsigned char *)uctemp.Data , uctemp.DataLen);
        }
        else
        {
        checksum=CheckSum_crc16(checksum , (unsigned char *)&uctemp.key, CMD_STRUCT_CRC_LEN+4);
        }
        uctemp.checksum=checksum;


        /* 1.7  移动当前数据到BUF*/
        memcpy((unsigned char *)&Parms->CmdSendBuf[i].startflag , (unsigned char *)&uctemp.startflag, sizeof(CmdProtocolParam));

        /* 1.8  打印加密后的数据*/
        if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
        {
            //mmandProtocolTransferInfoPrintf(&Parms->CmdSendBuf[i]);
            if(uctemp.CMD != CMD_HEART_BEAT)
            {
            I_TRANSFER("[%s]add  sn=%d cmd=0x%x (md:%lu)" ,Parms->name, uctemp.SequenceNumber,uctemp.CMD,CommandProtocolGetSystemMs());
            }
        }

        /* 1.9  更新命令加入时间*/
        if(mode)
        {
            Parms->CmdSendBufTimes[i]=CommandProtocolGetSystemMs()-(CMD_SEND_TIME_REPEAT_MS*(CMD_SEND_BUF_SIZE+1));
        }
        else
        {
            Parms->CmdSendBufTimes[i]=CommandProtocolGetSystemMs()-CMD_SEND_TIME_REPEAT_MS;
            CommandProtocolThreadDelay(CMD_SEND_TIME_GAP_MS);
        }
        #if CMD_HEART_BEAT_EN
        Parms->CmdSendBufStartTimes=CommandProtocolGetSystemMs();
        #endif
        return (int)uctemp.SequenceNumber;
        }
    }

    /* 2.0  注意  :  没有添加成功*/
    /* 调用此函数的时，需要 free(param->Data)*/
    return -1;
}
/*------------------------------------------------------------------
 * Func : CommandProtocolSendCmdAdd
 * Func :   增加发送的数据
 * fd :  设备ID
 * param :  发送命令的BUF
 * mode :  发送模式  0x01->立即发送0x00 -> FIFO 先进先出
 * return :  SendSequenceNumber  用于检测数据是否发出
 * return :  -1 表示刚才的命令没有加入队列中
 *------------------------------------------------------------------*/
int CommandProtocolSendCmdAdd(int fd , CmdProtocolParam * cmdparam , U8 mode)
{
    int i=0;
    #if(CMD_ADD_CMD_DELAY_EN)
    int delaytimes=0;
    #endif
    #if CMD_SEND_FILE_EN
    if(uParam.FileTransferSendStatus)
    {
        E_TRANSFER("%s  %d" ,uParam.name ,uParam.FileTransferSendStatus);
        return -1;
    }
    #endif
    
    if(cmdparam->DataLen==0)
    {
        E_TRANSFER("%s" ,uParam.name );
        return -1;
    }
    
    #if(CMD_ADD_CMD_DELAY_EN)
    delaytimes=CMD_ADD_CMD_DELAY_MS/10;
    do
    {
            for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
            {
                if(uParam.CmdSendBuf[i].startflag != CMD_START_FLAG)
                {
                   delaytimes=0;
                }
            }
            // 5.  sleep 30 ms
            if(delaytimes)
            {
                CommandProtocolThreadDelay(10);
                delaytimes--;
                #if(CMD_SYSTEM_EN == 0x00)
                CommandProtocolRecvSend_thread_Handler();
                #endif
            }
    }while(delaytimes);
    #endif
    
    #if(CMD_LARGEPACKET_PROTOCOL)
    if(cmdparam->DataLen > CMD_DATA_MAX_LEN)
    {
        return CommandProtocolSendLargePacket(&uParam,cmdparam);
    }
    #endif    
return CommandProtocolSendCmdAddsub(&uParam , cmdparam , mode);
}
/*------------------------------------------------------------------
 * Func : CommandProtocolSetParam
 * Func : 命令处理线程
 * fd :  设备ID
 * keyEn :  是否加密
 * KeySequence :  加密序列
 * DevSrc :  发送设备
 * DevDST :  目标设备
 *------------------------------------------------------------------*/
int CommandProtocolSetParam(int fd, void* data)
{
    CommandInterfaceParam* uctemp = (CommandInterfaceParam*)data;
    
    uParam.key.keyEn=uctemp->key.keyEn;
    uParam.key.KeySequence=uctemp->key.KeySequence;
    uParam.DevSrc=uctemp->DevSrc;
    uParam.DevDST=uctemp->DevDST;
    uParam.debuglvl=uctemp->debuglvl;
    return 0;
}
#if CMD_HEART_BEAT_EN
/*------------------------------------------------------------------
* Func : CommandProtocolHeartBeatStatue
* Func :判断心跳是否丢失
* fd :  设备ID
* HeartBeatEnable:  0-> disable  1-> enable   自动心跳使能开关
* return  :   0-> lose  1-> link
*------------------------------------------------------------------*/
int CommandProtocolHeartBeatStatue(int fd,int HeartBeatEnable)
{    
    if(HeartBeatEnable)
    uParam.HeartBeatEnable = 0x01;
    else
    uParam.HeartBeatEnable = 0x00;
        
    return uParam.HeartBeatStatue;
}
#endif
/*------------------------------------------------------------------
 * Func : CommandProtocolReset
 * Func :复位协议
 * fd :  设备ID
 * return  :   0-> OK  -2->NG
 *------------------------------------------------------------------*/
int CommandProtocolReset(int fd)
{
    int i=0;    
    /*1.清除接收数据记录*/
    uParam.SendSequenceNumber=1;
    for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
    uParam.RecordProcessing[i] = 0x3FFF;
    uParam.RecordProcessingIndex=0;
    
    #if CMD_HEART_BEAT_EN
    uParam.HeartBeatStatue=0;
//    uParam.HeartBeatSendTime=CommandProtocolGetSystemMs();
//    uParam.HeartBeatCheckTime=CommandProtocolGetSystemMs();
    #endif
    
    /*2.清除发送的  BUF*/
    CommandProtocolClearSendCmdBuf(&uParam,0x00,0x01);
    
    /*3.清除接收的  BUF*/
    uParam.recvTempIndex=0;
    for(i=0; i<CMD_SEND_BUF_SIZE ; i++ )
    {
        if(uParam.CmdRecvBuf[i].startflag== CMD_START_FLAG)
        {
            if((uParam.CmdRecvBuf[i].DataLen > DEFAULT_DATA_LEN) && (uParam.CmdRecvBuf[i].Data != 0x00)) // NULL))
            free((U8 *)uParam.CmdRecvBuf[i].Data);
            memset(&uParam.CmdRecvBuf[i].startflag, 0x00, sizeof(CmdProtocolParam));
        }
    }
    return 0x00;
}

static int CommandProtocolRecordSequence(CommandInterfaceParam* Parms,int SequenceNumber)
{
    Parms->RecordProcessing[Parms->RecordProcessingIndex]=SequenceNumber;
    Parms->RecordProcessingIndex++;
    if(Parms->RecordProcessingIndex>=CMD_RECORD_PROCESSING)
    Parms->RecordProcessingIndex=0;
    return 0x00;
}

#if CMD_BACKUP_CMD_EN
/*------------------------------------------------------------------
* Func : CommandProtocolBackupCMDLongerToProgress
* Func : 备份长时间处理的命令
* fd :  设备ID
* Cmdparam :  接收到的命令数据
* int  :  备份状态(0x00-> ok    -1->NG)
*------------------------------------------------------------------*/
static int  CommandProtocolBackupCMDLongerToProgress(int fd,CmdProtocolParam * Cmdparam)
{
    int i=0;
    for(i=0; i<CMD_RECV_BUF_SIZE ; i++ )
    {
        if((uParam.BackupCMD[i].startflag == 0x00)
        &&(uParam.BackupCMD[i].startflag2 == 0x00))
        {
            if(Cmdparam->DataLen > 0x04)
                {
                memcpy(&uParam.BackupCMD[i] , Cmdparam , sizeof(CmdProtocolParam));
                U8* buffer=(U8*) malloc(Cmdparam->DataLen);
                memcpy(buffer , (U8*)Cmdparam->Data , Cmdparam->DataLen);
                uParam.BackupCMD[i].Data=(U32) buffer;
                }
            else
                {
                memcpy(&uParam.BackupCMD[i] , Cmdparam , sizeof(CmdProtocolParam));
                }
            uParam.BackupCMDNum++;
            return 0x00;
        }
    }
return -1;
}
/*------------------------------------------------------------------
* Func : CommandProtocolGetCMDLongerToProgress
* Func : 获取备份长时间处理的命令
* fd :  设备ID
* Cmdparam :  接收到的命令数据
* int  :  备份状态(0x00-> ok    -1->NG)
*------------------------------------------------------------------*/
static int  CommandProtocolGetCMDLongerToProgress(int fd,CmdProtocolParam * Cmdparam)
{
    int i=0;
    if(uParam.BackupCMDNum==0)
        return -1;
    for(i=0; i<CMD_RECV_BUF_SIZE ; i++ )
    {
        if((uParam.BackupCMD[i].startflag == CMD_START_FLAG)
        &&(uParam.BackupCMD[i].startflag2 == CMD_START_FLAG2))
        {
            memcpy(Cmdparam , &uParam.BackupCMD[i] , sizeof(CmdProtocolParam));
            uParam.BackupCMD[i].startflag=0;
            uParam.BackupCMD[i].startflag2=0;
            uParam.BackupCMDNum--;
            return 0x00;
        }
    }
return -1;
}
#endif

uint8_t Cmd_Ack = 0;
void set_cmd_ack(uint8_t ack)
{
    Cmd_Ack = ack;
}

uint8_t get_cmd_ack(void)
{
    return Cmd_Ack;
}

/*------------------------------------------------------------------
 * Func : CommandProtocolRecvSend_thread_Handler
 * Func : 命令处理线程
 * ctx :  整个进程参数
 *------------------------------------------------------------------*/
#define READ_BUF_LEN (sizeof(CommandInterfaceParam) + CMD_DATA_MAX_LEN)
//extern uint32_t count;
void *CommandProtocolRecvSend_thread_Handler(void)
{
    #if (CMD_DEBUG == CMD_DEBUG_ALL)
    static U32 time=0;
    U32 time3=0;
    #endif
    U32 uctemp;
    int index,i,ret;
    #if (CMD_ENCRYPTION_EN || CMD_RECV_DATA_MODE)
    unsigned char * buf;
    #endif
    U8 indexback = 0xFF;
    U16 checksum=0;
    CommandInterfaceParam* Parms=(CommandInterfaceParam*)&uParam;
    U32 SystemMs=0;
    static U8 recv_timeout_count = 0;

    #if CMD_SYSTEM_EN
    I_TRANSFER("#run# [%s]  V%d.%d",Parms->name,(PROTOCOL_VERSION>>8)&0xFF , PROTOCOL_VERSION&0xFF);
    
    do
    {
    #endif //#if CMD_SYSTEM_EN
        #if CMD_SYSTEM_EN
            #ifndef STM32_CHIP_EN
            /****互斥锁加锁***/
            pthread_mutex_lock(&CommMutex);
            #else
            rt_mutex_take(CommMutex, RT_WAITING_FOREVER);
            #endif
        #endif
        CMD_AGAIN:
        #if CMD_RECV_DATA_MODE/*重底层接收数据*/
        // if(Parms->fun.CommandInterfaceRead != NULL)
        // {
            
            buf=(unsigned char *)malloc(READ_BUF_LEN);
            index= Example_CommandInterfaceRead(buf , READ_BUF_LEN);
            if (index > 0)
            {
            #if(CMD_DEBUG_SEND_RECV & CMD_DEBUG)
            printf("<in>[%d]%s %d\r\n", __LINE__, __FUNCTION__, index);
            #endif
                for(i=0;i<index;i++)
                {
                    ret=CommandProtocolRecvOneByte(Parms , buf[i]);
                    if(ret <= -2)
                    {
                        E_TRANSFER("%s, %d." ,Parms->name , ret);
                    }
                }
                recv_timeout_count = 0;
            }
            else
            {
                 recv_timeout_count++;
                if(recv_timeout_count >= 20) // 200*20=4s
                {
                    Recv_Cmd_Timeout = 1;
                    E_TRANSFER("read respond cmd timeout count more max 20!\r\n");
                    free(buf);
                    return ;
                } 
            }
            free(buf);
        //}
        #endif
        
        SystemMs=CommandProtocolGetSystemMs();
        #if 1/*处理接收数据*/
        /* 1.0  查找时间最先收到的命令*/
        uctemp=0;
        indexback = 0xFF;
        for(index=0; index<CMD_RECV_BUF_SIZE ; index++ )
        {
            if((Parms->CmdRecvBuf[index].startflag == CMD_START_FLAG)
            &&(Parms->CmdRecvBuf[index].startflag2 == CMD_START_FLAG2))
            {
                indexback=index;
                break;
            }
        }

        /* 2.0  处理当前命令*/
        if(indexback < CMD_RECV_BUF_SIZE)
        {
            /* 2.1.0更新 当前串口发送时间*/
            index = indexback;
            #if CMD_HEART_BEAT_EN
            Parms->CmdSendBufStartTimes = SystemMs;
            #endif
            
            /* 2.1.1  确认命令是checksum*/
            checksum=0;
            if(Tu16(Parms->CmdRecvBuf[index].DataLen) > DEFAULT_DATA_LEN)
            {
                checksum=CheckSum_crc16(checksum , (unsigned char *)&Parms->CmdRecvBuf[index].key, CMD_STRUCT_CRC_LEN);
                checksum=CheckSum_crc16(checksum , (unsigned char *)Parms->CmdRecvBuf[index].Data , Tu16(Parms->CmdRecvBuf[index].DataLen));
            }
            else
            {
                checksum=CheckSum_crc16(checksum , (unsigned char *)&Parms->CmdRecvBuf[index].key, CMD_STRUCT_CRC_LEN+4);
            }
            
            /* 2.1.2 打印当前接收处理前的数据*/
            // CommandProtocolTransferInfoPrintf(&Parms->CmdRecvBuf[index]);// 
            
            /* 2.1.3  确认是否需要大小端转换*/
            #if CMD_SWAP_ENDIAN
            Parms->CmdRecvBuf[index].DataLen = Tu16(Parms->CmdRecvBuf[index].DataLen);
            Parms->CmdRecvBuf[index].checksum = Tu16(Parms->CmdRecvBuf[index].checksum);
            Parms->CmdRecvBuf[index].SequenceNumber = Tu16(Parms->CmdRecvBuf[index].SequenceNumber);
            Parms->CmdRecvBuf[index].CMD= Tu16(Parms->CmdRecvBuf[index].CMD);
            if(Parms->CmdRecvBuf[index].DataLen == DEFAULT_DATA_LEN)
            Parms->CmdRecvBuf[index].Data= Tu32(Parms->CmdRecvBuf[index].Data);
            #endif
            
            /* 2.2.0  确认命checksum*/
            if(checksum == Parms->CmdRecvBuf[index].checksum)
            {
                /* 2.2.1   确认命令是否已经加密*/
                #if CMD_ENCRYPTION_EN
                if(Parms->CmdRecvBuf[index].key.keyEn)
                {
                    buf=(unsigned char *)malloc(min(Parms->CmdRecvBuf[index].DataLen,CMD_DATA_MAX_LEN));
                    if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                    {
                        Arc4DecryptEncrypt((const unsigned char *)Parms->CmdRecvBuf[index].Data, buf, Parms->CmdRecvBuf[index].DataLen,Parms->CmdRecvBuf[index].key.KeySequence);
                        memcpy((unsigned char *)Parms->CmdRecvBuf[index].Data , buf , Parms->CmdRecvBuf[index].DataLen);
                    }
                    else
                    {
                        Arc4DecryptEncrypt((const unsigned char *)&Parms->CmdRecvBuf[index].DevSrc, buf, 0x08 , Parms->CmdRecvBuf[index].key.KeySequence);
                        memcpy((unsigned char *)&Parms->CmdRecvBuf[index].DevSrc , buf , 0x08);
                    }
                    free(buf);

                    /* 2.2.2  打印解密后的数据*/
                    // CommandProtocolTransferInfoPrintf(&Parms->CmdRecvBuf[index]);
                }
                #endif

                /* 2.2.3  长时间没有收到命令,清除当前部分记录*/
                if((SystemMs - Parms->RecvCmdTimeGap) >= 20000)
                {
                    for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
                    Parms->RecordProcessing[i] = 0x3FFF;
                }
                Parms->RecvCmdTimeGap=SystemMs;
                
                /* 2.2.4  打印部分调试信息*/
                if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
                {
                    if((CommandProtocolSeqToCmd(Parms,Parms->CmdRecvBuf[index].SequenceNumber ) != CMD_HEART_BEAT)
                        &&(Parms->CmdRecvBuf[index].CMD != CMD_HEART_BEAT))
                        {
                        W_TRANSFER("==>checksum OK  [%s]SN=%d  cmd =0x%x cs=0x%x,0x%x len=%d\n" 
                            ,Parms->name 
                            ,Parms->CmdRecvBuf[index].SequenceNumber
                            ,Parms->CmdRecvBuf[index].CMD
                            ,Parms->CmdRecvBuf[index].checksum,checksum
                            ,Parms->CmdRecvBuf[index].DataLen);
                        }
                }
                /* 2.2.5  确认命令  为  ACK 命令, 清除上次发送出去的命令*/
                if(Parms->CmdRecvBuf[index].CMD == CMD_ACK)
                {/* 清除 发送命令BUF的 SequenceNumber*/
                    #if CMD_HEART_BEAT_EN
                    Parms->HeartBeatCheckTime=SystemMs;
                    #endif
                    #if(CMD_DEBUG == CMD_DEBUG_ALL)
                    Parms->debugacktimes++;
                    #endif
                    CommandProtocolClearSendCmdBuf(Parms , Parms->CmdRecvBuf[index].SequenceNumber , 0);
                    if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                    free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                    memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));

                    set_cmd_ack(1);
                }
                else
                {
                        /* 2.2.6  过滤已经处理的的命令*/
                        for(indexback=0; indexback<CMD_RECORD_PROCESSING ; indexback++ )
                        {
                            if(Parms->RecordProcessing[indexback] == Parms->CmdRecvBuf[index].SequenceNumber)
                            break;
                        }
                        
                        /* 2.2.7  清除命令记录指令*/
                        if(Parms->CmdRecvBuf[index].CMD == CMD_CLEAR_RECORD_FLAG)
                        {
                            for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
                            Parms->RecordProcessing[i] = 0x3FFF;
                            Parms->SendSequenceNumber=0;
                        }
                        else if((Parms->CmdRecvBuf[index].CMD > CMD_HEART_BEAT) && (indexback >= CMD_RECORD_PROCESSING))
                        {
                            /* 2.3.2  调用命令处理函数*/
                            #if CMD_SEND_FILE_EN
                            if(Parms->CmdRecvBuf[index].CMD == CMD_UPDATE_SW_SEND_FILE_START)
                            {
                                if(CommandProtocolTransferFileRecvStart(Parms,&Parms->CmdRecvBuf[index]) ==0xFF)
                                {
                                    if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                                    free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                                    memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));
                                    E_TRANSFER("=open file=");
                                    goto CMD_AGAIN;
                                }
                                CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                Example_CommandInterfaceDataProcessing(&Parms->CmdRecvBuf[index]);
                            }
                            else if(Parms->CmdRecvBuf[index].CMD == CMD_UPDATE_SW_REPORT_FILE)
                            {
                                if(Parms->CmdRecvBuf[index].Data ==0)
                                {
                                    CommandProtocolClearSendCmdBuf(Parms , 0 ,0x01);
                                    CommandProtocolTransferFileStartCmd(Parms);
                                    Parms->FileTransferSendStatus=FILE_TRANSFER_FILE_SEND;
                                }
                                else if(Parms->CmdRecvBuf[index].Data ==2)
                                {
                                    
                                }
                                CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                            }
                            else if(Parms->CmdRecvBuf[index].CMD == CMD_UPDATE_SW_SEND_FILE)
                            {
                                if(Parms->FileTransferRecvStatus)
                                {
                                    if(Parms->FileTransferRecvStatus == FILE_RECV_FINISH_NG)
                                    ret=0xFF;
                                    else
                                    ret=CommandProtocolTransferFileRecv(Parms,&Parms->CmdRecvBuf[index]);
                                    if(ret == 0xFF)
                                    {
                                        if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                                        free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                                        /* 2.3.4  清除当前BUF准备下次接收*/
                                        memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));
                                        CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                        goto CMD_AGAIN;
                                    }
                                    if(Parms->FileTransferRecvStatus == FILE_RECV_FINISH_OK)
                                    {
                                        Example_CommandInterfaceDataProcessing(&Parms->CmdRecvBuf[index]);
                                    }
                                    CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                }
                                else
                                {
                                    if(Parms->FileTransferRecvStatus != FILE_RECV_FINISH_NG)
                                    CommandProtocolTransferFileReportCmd(Parms , 0);
                                    Parms->FileTransferRecvStatus = FILE_RECV_FINISH_NG;
                                    
                                    if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                                    free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                                    /* 2.3.4  清除当前BUF准备下次接收*/
                                    memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));
                                    CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                    goto CMD_AGAIN;
                                }
                            }
                            else if(Parms->CmdRecvBuf[index].CMD == CMD_UPDATE_SW_READY_UPDATE)
                            {
                                    Parms->FileTransferRecvStatus = FILE_RECV_NULL;
                                    CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                    Example_CommandInterfaceDataProcessing(&Parms->CmdRecvBuf[index]);
                            }
                            else
                            #endif
                            #if(CMD_LARGEPACKET_PROTOCOL)
                            if(Parms->CmdRecvBuf[index].CMD == CMD_LARGE_PACKET_PROTOCOL)
                            {
                                CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                CommandProtocolRecvLargePacket(Parms , &Parms->CmdRecvBuf[index]);
                            }
                            else
                            #endif
                            {
                                    CommandProtocolRecordSequence(Parms,Parms->CmdRecvBuf[index].SequenceNumber); /* 记录已经处理的命令*/
                                    Example_CommandInterfaceDataProcessing(&Parms->CmdRecvBuf[index]);
                            }
                        }
                        else if(Parms->CmdRecvBuf[index].CMD == CMD_HEART_BEAT)
                        {
                            Parms->Version = (U16)Parms->CmdRecvBuf[index].Data;
                        }
                        /* 2.3.2  回复命令ACK*/
                        if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                        free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                        Parms->CmdRecvBuf[index].DataLen = DEFAULT_DATA_LEN;
                        Parms->CmdRecvBuf[index].CMD = CMD_ACK;
                        Parms->CmdRecvBuf[index].key.keyEn= 0x00;
                        Parms->CmdRecvBuf[index].Data= 0x00;
                        #if CMD_SWAP_ENDIAN
                        Parms->CmdRecvBuf[index].SequenceNumber = Tu16(Parms->CmdRecvBuf[index].SequenceNumber);
                        Parms->CmdRecvBuf[index].CMD = Tu16(Parms->CmdRecvBuf[index].CMD);
                        if(Parms->CmdRecvBuf[index].DataLen == DEFAULT_DATA_LEN)
                        Parms->CmdRecvBuf[index].Data = Tu32(Parms->CmdRecvBuf[index].Data);
                        #endif
                        checksum=0;
                        checksum=CheckSum_crc16(checksum , (unsigned char *)&Parms->CmdRecvBuf[index].key, CMD_STRUCT_CRC_LEN+4);
                        Parms->CmdRecvBuf[index].checksum=checksum;

                        /* 2.3.3  回复*/
                        CommandProtocolRecvSendAct(&Parms->CmdRecvBuf[index]);
                        /* 2.3.4  清除当前BUF准备下次接收*/
                        memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));
                    }
                }
                else
                {
                    if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
                    {
                    if((CommandProtocolSeqToCmd(Parms,Parms->CmdRecvBuf[index].SequenceNumber ) != CMD_HEART_BEAT)
                        &&(Parms->CmdRecvBuf[index].CMD != CMD_HEART_BEAT))
                        {
                        W_TRANSFER("==>checksum NG  [%s]SN=%d  cmd =0x%x cs=0x%x,0x%x len=%d\n" 
                            ,Parms->name 
                            ,Parms->CmdRecvBuf[index].SequenceNumber
                            ,Parms->CmdRecvBuf[index].CMD
                            ,Parms->CmdRecvBuf[index].checksum,checksum
                            ,Parms->CmdRecvBuf[index].DataLen);
                        }
                    }
                    /* 2.5.5  清除当前BUF准备下次接收*/
                    if(Parms->CmdRecvBuf[index].DataLen > DEFAULT_DATA_LEN)
                    free((unsigned char *)Parms->CmdRecvBuf[index].Data);
                    memset(&Parms->CmdRecvBuf[index].startflag, 0x00, sizeof(CmdProtocolParam));
                }
                goto CMD_AGAIN;
            }
            #endif//
        
            SystemMs=CommandProtocolGetSystemMs();
            /*处理发送数据*/
            #if(CMD_SEND_MODE_FIFO_EN)
            if((SystemMs > Parms->SendCmdTimeGap) && ((SystemMs-Parms->SendCmdTimeGap) >= (CMD_SEND_TIME_GAP_MS)))
            {
                Parms->SendCmdTimeGap=SystemMs;
                indexback = 0xFF;
                uctemp = 0xFFFFFF;
                #if CMD_SEND_FILE_EN
                if((Parms->FileTransferSendStatus != FILE_TRANSFER_NULL) || (Parms->FileTransferRecvStatus != FILE_RECV_NULL))
                {
                    uctemp=200;/*命令重发时间为 :200 ms*/
                    for(index=0; index<CMD_SEND_BUF_SIZE ; index++ )
                    {
                        if((Parms->CmdSendBuf[index].startflag == CMD_START_FLAG)
                        &&(Parms->CmdSendBuf[index].startflag2 == CMD_START_FLAG2))
                        {                    
                            if((SystemMs-Parms->CmdSendBufTimes[index]) >= uctemp)
                            {
                            uctemp=(SystemMs-Parms->CmdSendBufTimes[index]);
                            indexback=index;
                            }
                        }
                    }             
                }
                else
                #endif
                {
                    /* 1.0  查找时间等待最长的命令*/
                    for(index=0; index<CMD_SEND_BUF_SIZE ; index++ )
                    {
                        if((Parms->CmdSendBuf[index].startflag == CMD_START_FLAG)
                        &&(Parms->CmdSendBuf[index].startflag2 == CMD_START_FLAG2))
                        {
                            if(Parms->CmdSendBuf[index].SequenceNumber < uctemp)
                            {
                            uctemp=Parms->CmdSendBuf[index].SequenceNumber;
                            indexback=index;
                            }
                        }
                    }
                }
                /* 2.0  查找后，发送命令*/
                #if (CMD_HEART_BEAT_EN)
                if((indexback < CMD_SEND_BUF_SIZE)&&((SystemMs - Parms->CmdSendBufStartTimes) <= 5000))
                #else
                if(indexback < CMD_SEND_BUF_SIZE)
                #endif
                {
                    if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
                    {
                    if(Tu16(Parms->CmdSendBuf[indexback].CMD) != CMD_HEART_BEAT)
                        {
                        W_TRANSFER("==>[%s]SN=%d  cmd =0x%x len=%d cs=0x%x(ms:%d)\n" 
                            ,Parms->name 
                            ,Tu16(Parms->CmdSendBuf[indexback].SequenceNumber)
                            ,Tu16(Parms->CmdSendBuf[indexback].CMD)
                            ,Parms->CmdSendBuf[indexback].DataLen
                            ,Parms->CmdSendBuf[indexback].checksum
                            ,CommandProtocolGetSystemMs());
                        }
                    }
                    if((Parms->CmdSendBuf[indexback].DataLen > DEFAULT_DATA_LEN) && (Parms->CommMode == COMM_UART))
                    {/* 按照115200  理想传输, 设置时间间隔*/
                        #define CMD_PROC_TIME  10 // 80  
                        uctemp=(Parms->CmdSendBuf[indexback].DataLen+13)/10;
                        //W_TRANSFER("%d,%d,%d" , uctemp,SendCmdTimeGap,SendCmdTimeGap+(uctemp + CMD_PROC_TIME) - CMD_SEND_TIME_GAP_MS); 
                        if(CMD_SEND_TIME_GAP_MS < (uctemp + CMD_PROC_TIME))
                            Parms->SendCmdTimeGap += (uctemp + CMD_PROC_TIME) - CMD_SEND_TIME_GAP_MS;
                    }
                     CommandProtocolSendCmdBuf(Parms , indexback);
                    /* 更新 当前串口发送时间*/
                    Parms->CmdSendBufTimes[indexback]=SystemMs;
                }
            }                
            #else
            if(((SystemMs > Parms->SendCmdTimeGap) && ((SystemMs-Parms->SendCmdTimeGap) >= (CMD_SEND_TIME_GAP_MS)) && (Parms->CommMode == COMM_UART))
                ||(Parms->CommMode == COMM_UDP))
            {
                checksendcommagain:
                Parms->SendCmdTimeGap=SystemMs;
                #if CMD_SEND_FILE_EN
                if((Parms->FileTransferSendStatus==FILE_TRANSFER_FILE_SEND)||(Parms->FileTransferSendStatus==FILE_TRANSFER_WAIT_FINISH_FLAG))
                uctemp=200;/*命令重发时间为 :200 ms*/
                else
                #endif
                uctemp=CMD_SEND_TIME_REPEAT_MS;/*命令重发时间为 :400 ms*/
                indexback = 0xFF;
                
                /* 1.0  查找时间等待最长的命令*/
                for(index=0; index<CMD_SEND_BUF_SIZE ; index++ )
                {
                    if((Parms->CmdSendBuf[index].startflag == CMD_START_FLAG)
                    &&(Parms->CmdSendBuf[index].startflag2 == CMD_START_FLAG2))
                    {                    
                        if((SystemMs-Parms->CmdSendBufTimes[index]) >= uctemp)
                        {
                        uctemp=(SystemMs-Parms->CmdSendBufTimes[index]);
                        indexback=index;
                        }
                    }
                }             
                /* 2.0  查找后，发送命令*/
                #if (CMD_HEART_BEAT_EN)
                if((indexback < CMD_SEND_BUF_SIZE)&&((SystemMs - Parms->CmdSendBufStartTimes) <= 5000))
                #else
                if(indexback < CMD_SEND_BUF_SIZE)
                #endif
                {
                    if(Parms->debuglvl & CMD_DEBUG_NOMAL_SEND_RECV)
                    {
                    if(Tu16(Parms->CmdSendBuf[indexback].CMD) != CMD_HEART_BEAT)
                        {
                        W_TRANSFER("==>[%s]SN=%d  cmd =0x%x len=%d cs=0x%x(ms:%d)\n" 
                            ,Parms->name 
                            ,Tu16(Parms->CmdSendBuf[indexback].SequenceNumber)
                            ,Tu16(Parms->CmdSendBuf[indexback].CMD)
                            ,Parms->CmdSendBuf[indexback].DataLen
                            ,Parms->CmdSendBuf[indexback].checksum
                            ,CommandProtocolGetSystemMs());
                        }
                    }
                    if((Parms->CmdSendBuf[indexback].DataLen > DEFAULT_DATA_LEN) && (Parms->CommMode == COMM_UART))
                    {/* 按照115200  理想传输, 设置时间间隔*/
                        #define CMD_PROC_TIME2  10   
                        uctemp=(Parms->CmdSendBuf[indexback].DataLen+13)/10;
                        //W_TRANSFER("%d,%d,%d" , uctemp,SendCmdTimeGap,SendCmdTimeGap+(uctemp + CMD_PROC_TIME) - CMD_SEND_TIME_GAP_MS); 
                        if(CMD_SEND_TIME_GAP_MS < (uctemp + CMD_PROC_TIME2))
                            Parms->SendCmdTimeGap += (uctemp + CMD_PROC_TIME2) - CMD_SEND_TIME_GAP_MS;
                    }
                    CommandProtocolSendCmdBuf(Parms , indexback);
                    /* 更新 当前串口发送时间*/
                    Parms->CmdSendBufTimes[indexback]=SystemMs;
                    if(Parms->CommMode==COMM_UDP)/*网络可以连续送数据包*/
                    goto checksendcommagain;
                }
            }
            #endif
        
            #if CMD_SEND_FILE_EN/*处理发送数据,发送文件时调用*/
            if(CommandProtocolGetSendEmptyBufNum(Parms)
                &&((Parms->FileTransferSendStatus==FILE_TRANSFER_FILE_SEND)||(Parms->FileTransferSendStatus==FILE_TRANSFER_WAIT_FINISH_FLAG)))
            {
            /* 传输文件时，自动读取文件发送*/
            CommandProtocolTransferFileSend(Parms);
            }
            #endif
                
            #if CMD_HEART_BEAT_EN /* 心跳包，处理*/
            #if CMD_SEND_FILE_EN
            if((Parms->HeartBeatEnable) && (Parms->FileTransferSendStatus ==FILE_TRANSFER_NULL)&&(Parms->FileTransferRecvStatus ==FILE_RECV_NULL))
            #else
            if(Parms->HeartBeatEnable)
            #endif
            {
                if(((SystemMs -Parms->HeartBeatSendTime) > 1000) && (CommandProtocolGetSendEmptyBufNum(Parms) == CMD_SEND_BUF_SIZE))// 1S
                {/*判断心跳包，发送时间，发包*/
                    CmdProtocolParam  HeartBeatparam;
                    memset(&HeartBeatparam.startflag,0,sizeof(CmdProtocolParam));
                    HeartBeatparam.CMD=CMD_HEART_BEAT;
                    HeartBeatparam.DataLen=4;
                    HeartBeatparam.Data=PROTOCOL_VERSION;
                    CommandProtocolSendCmdAddsub(Parms,&HeartBeatparam,0x00);
                    Parms->HeartBeatSendTime=SystemMs;
                }
                else
                {/*判断设备是否丢失*/
                    if((SystemMs -Parms->HeartBeatCheckTime) > 3000)// 3S
                    {
                        if(Parms->HeartBeatStatue)
                        {
                            for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
                            {
                                Parms->RecordProcessing[i] = 0x3FFF;
                            }
                            Parms->HeartBeatStatue=0;
                        }
                       
                    }
                    else
                    {
                        if(Parms->HeartBeatCheckTime > Parms->HeartBeatSendTime)
                        {
                            Parms->HeartBeatStatue = 1;
                        }
                    }
                }
            }
            #endif

        #if (CMD_DEBUG == CMD_DEBUG_ALL)
        if(Parms->debuglvl & CMD_DEBUG_AGING)
        {
            time3=SystemMs/1000;
            if((time3 - time) >=5)
            {
            E_TRANSFER("%s S=%d ms=%ld [%d,%d]\n" ,Parms->name, SystemMs/1000, SystemMs%1000,Parms->debugsendtimes, Parms->debugacktimes);
            time=time3;
            }
        }
        #endif
        
        
    #if CMD_SYSTEM_EN
        #ifndef STM32_CHIP_EN
        /****互斥锁解锁***/
        pthread_mutex_unlock(&CommMutex);
        #else
        rt_mutex_release(CommMutex);
        #endif
        
        // 5.  sleep 30 ms
        if(Parms->CommMode == COMM_UDP)
        CommandProtocolThreadDelay(5);
        else
        CommandProtocolThreadDelay(10);
    }while(Parms->ThreadRun);
        #ifndef STM32_CHIP_EN
        pthread_exit(NULL); 
        #endif
    #endif //#if CMD_SYSTEM_EN
    return 0;
}

#if CMD_SYSTEM_EN
#ifdef STM32_CHIP_EN
static void CommandProtocolRecvSend_thread_Handler_STM32(void *ctx)
{
    CommandProtocolRecvSend_thread_Handler(ctx);
}
#endif
/*------------------------------------------------------------------
 * Func : CommandProtocolMutexInit
 * Func : 创建线程互斥锁
 *------------------------------------------------------------------*/
int CommandProtocolMutexInit(void)
{
    if(CommMutexInit==0)
    {
        #ifndef STM32_CHIP_EN
        if(pthread_mutex_init(&CommMutex, NULL) != 0)
        {
        E_TRANSFER("mutex init\n");
        return -1;
        }
        #else
        CommMutex= rt_mutex_create("cmd mutex", RT_IPC_FLAG_FIFO);
        if(NULL == CommMutex)
        {
        E_TRANSFER("mutex init\n");
        return -1;
        }
        #endif
        CommMutexInit=1;
    }
return 0x00;
}
#endif
/*------------------------------------------------------------------
 * Func : CommandProtocolOpen
 * Func : 创建数据交互协议
 * fun :  传递函数指针
 * CommMode :  0->uart  1-> UDP  
 * name :  协议名字
 *------------------------------------------------------------------*/
 int CommandProtocolOpen(CommandInterfaceFun *fun,const char* name,U8 CommMode)
{ 
    int i;
    memset((unsigned char* )&uParam , 0x00 , sizeof(CommandInterfaceParam));

    uParam.SendSequenceNumber=1;
    for(i=0; i<CMD_RECORD_PROCESSING ; i++ )
    uParam.RecordProcessing[i] = 0x3FFF;
    uParam.RecordProcessingIndex=0;
    uParam.recvTempIndex=0;

    uParam.debuglvl=CMD_DEBUG;
    #if(CMD_DEBUG == CMD_DEBUG_ALL)
    uParam.debugsendtimes=0;
    uParam.debugacktimes=0;
    #endif
    #if CMD_HEART_BEAT_EN
    uParam.HeartBeatStatue=0;
    uParam.HeartBeatEnable=1;
    uParam.HeartBeatSendTime=CommandProtocolGetSystemMs();
    uParam.HeartBeatCheckTime=CommandProtocolGetSystemMs();
    #endif
    uParam.ThreadRun=0x01;
    uParam.CommMode = (CommMode==COMM_UDP) ? 0x01 : 0x00;
    
    memcpy(uParam.name, name , min(5,strlen(name)));
//    my2_printf("#run# %s", uParam.name);

    return 0x01;
}
 
/*------------------------------------------------------------------
* Func : CommandProtocolOpen
* Func :关闭数据交互协议
* fd :  设备ID
*------------------------------------------------------------------*/
int CommandProtocolClose(int fd)
{
    uParam.ThreadRun = 0;
    CommandProtocolThreadDelay(300);
    I_TRANSFER("");
    return 0x00;
}

typedef struct T_CommandProtocolStringTable_
{
    U16 cmd;
    const char * str;
} T_CommandProtocolStringTable;

static const T_CommandProtocolStringTable CommandProtocolStringTable[] =
{
// nomal
    {CMD_ACK, "CMD_ACK"},
#if (CMD_DISPLAY_STR)
    {CMD_STM32_1, "CMD_STM32_1"},
    {CMD_STM32_2, "CMD_STM32_2"},
#endif    
};
const char * CommandProtocolString(U16 cmd)
{
    int i=0;
    for (i = 0; i < ARRAY_SIZE(CommandProtocolStringTable); i++)
    {
        if(cmd == CommandProtocolStringTable[i].cmd)
        {
            return  CommandProtocolStringTable[i].str;
        }
    }
    return  "CMD_???";
}



