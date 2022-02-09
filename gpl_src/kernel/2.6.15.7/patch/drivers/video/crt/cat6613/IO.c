#include "ast_def.h"
#ifdef CONFIG_AST1500_CAT6613
#ifndef CONFIG_AST1500_CAT6613
///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >IO.c<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/
#include "typedef_hdmitx.h"
#include "main.h"
#include "Mcu.h"
#include "IO.h"
#include "EDID.h"
#include "Utility.h"
#include "TimerProcess.h"

// BYTE I2CADR=RXADR;
// BYTE I2CDEV=RXDEV;

///////////////////////////////////////////////////////////////////////////////
// Start: I2C for 8051
///////////////////////////////////////////////////////////////////////////////
void HotPlug(BOOL Enable)
{
    RX_HPD=Enable;
    RX_HPD1=Enable;
}
void set_8051_scl( BOOL bit_value )
{
     SCL_PORT=bit_value;

//    DelayUS(DELAY_TIME);
}

void set_8051_sda( BOOL bit_value,BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=bit_value;
            break;

#ifdef _2PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
#endif

#ifdef _3PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
#endif

#ifdef _4PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
         case 3:
             TX3_SDA_PORT=bit_value;
            break;
#endif

#ifdef _8PORT_
         case 1:
             TX1_SDA_PORT=bit_value;
            break;
         case 2:
             TX2_SDA_PORT=bit_value;
            break;
         case 3:
             TX3_SDA_PORT=bit_value;
            break;
         case 4:
             TX4_SDA_PORT=bit_value;
            break;
         case 5:
             TX5_SDA_PORT=bit_value;
            break;
         case 6:
             TX6_SDA_PORT=bit_value;
            break;
         case 7:
             TX7_SDA_PORT=bit_value;
            break;
#endif

         default:
             RX_SDA_PORT=bit_value;
     }
//    DelayUS(DELAY_TIME);
}

BOOL get_8051_sda( BYTE device )
{
     switch( device ) {
         case 0:
             TX0_SDA_PORT=1;
             return TX0_SDA_PORT;
            break;

#ifdef _2PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
#endif

#ifdef _3PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
#endif

#ifdef _4PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
         case 3:
             TX3_SDA_PORT=1;
             return TX3_SDA_PORT;
            break;
#endif

#ifdef _8PORT_
         case 1:
             TX1_SDA_PORT=1;
             return TX1_SDA_PORT;
            break;
         case 2:
             TX2_SDA_PORT=1;
             return TX2_SDA_PORT;
            break;
         case 3:
             TX3_SDA_PORT=1;
             return TX3_SDA_PORT;
            break;
         case 4:
             TX4_SDA_PORT=1;
             return TX4_SDA_PORT;
            break;
         case 5:
             TX5_SDA_PORT=1;
             return TX5_SDA_PORT;
            break;
         case 6:
             TX6_SDA_PORT=1;
             return TX6_SDA_PORT;
            break;
         case 7:
             TX7_SDA_PORT=1;
             return TX7_SDA_PORT;
            break;
#endif

         default:
             RX_SDA_PORT=1;
             return RX_SDA_PORT;
     }
}

void i2c_8051_start( BYTE device )
{
    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    set_8051_sda( LOW,device );
    set_8051_scl( LOW );
}

void i2c_8051_write( BYTE byte_data,BYTE device )
{
 BYTE data bit_cnt,tmp;
 BOOL data bit_value;

     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         tmp=(byte_data << bit_cnt) & 0x80;
         bit_value=tmp && 0x80;

         set_8051_sda( bit_value,device );
         set_8051_scl( HIGH );
         set_8051_scl( LOW );
     }
}

BOOL i2c_8051_wait_ack( BYTE device )
{
 BOOL data ack_bit_value;

    set_8051_sda( HIGH,device );
    set_8051_scl( HIGH );
    ack_bit_value=get_8051_sda( device );
    set_8051_scl( LOW );

    return ack_bit_value;
}

BYTE i2c_8051_read( BYTE device )
{
 BYTE data bit_cnt,byte_data;
 BOOL data bit_value;

     byte_data=0;
     for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
         set_8051_scl( HIGH );

         bit_value=get_8051_sda( device );

         byte_data=(byte_data << 1) | bit_value;

         set_8051_scl( LOW );
     }

     return byte_data;
}

void i2c_8051_send_ack( BOOL bit_value,BYTE device )
{
     set_8051_sda( bit_value,device );
     set_8051_scl( HIGH );
     set_8051_scl( LOW );
     set_8051_sda( HIGH,device );
}

void i2c_8051_end( BYTE device )
{
     set_8051_sda( LOW,device );
     set_8051_scl( HIGH );
     set_8051_sda( HIGH,device );
}

BOOL i2c_write_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
 BYTE data i;

     i2c_8051_start(device);                // S

     i2c_8051_write(address&0xFE,device);        // slave address (W)
     if( i2c_8051_wait_ack(device)==1 )    {        // As
         i2c_8051_end(device);
     return 0;
      }

     i2c_8051_write(offset,device);            // offset
     if( i2c_8051_wait_ack(device)==1 )    {        // As
         i2c_8051_end(device);
     return 0;
     }

     for(i=0; i<byteno-1; i++) {
          i2c_8051_write(*p_data,device);        // write d
          if( i2c_8051_wait_ack(device)==1 ) {        // As
              i2c_8051_end(device);
         return 0;
         }
         p_data++;
     }

     i2c_8051_write(*p_data,device);            // write last d
     if( i2c_8051_wait_ack(device)==1 )    {        // As
          i2c_8051_end(device);
     return 0;
     }
     else {
          i2c_8051_end(device);
     return 1;
     }
}

BOOL i2c_read_byte( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data,BYTE device )
{
 BYTE data i;

     i2c_8051_start(device);                // S

     i2c_8051_write(address&0xFE,device);        // slave address (W)
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_write(offset,device);            // offset
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     i2c_8051_start(device);

     i2c_8051_write(address|0x01,device);        // slave address (R)
     if( i2c_8051_wait_ack(device)==1 ) {        // As
         i2c_8051_end(device);
         return 0;
     }

     for(i=0; i<byteno-1; i++) {
         *p_data=i2c_8051_read(device);        // read d
         i2c_8051_send_ack(LOW,device);        // Am

         p_data++;
     }

     *p_data=i2c_8051_read(device);            // read last d
     i2c_8051_send_ack(HIGH,device);            // NAm
     i2c_8051_end(device);

    return 1;
}
///////////////////////////////////////////////////////////////////////////////
// I2C for original function call
///////////////////////////////////////////////////////////////////////////////
#if 0
// BYTE HDMI_ReadI2C_Byte(BYTE RegAddr)
// {
//  BYTE data p_data;
//
//  i2c_read_byte(I2CADR,RegAddr,1,&p_data,I2CDEV);
//
//  return p_data;
// }
//
// SYS_STATUS HDMI_WriteI2C_Byte(BYTE RegAddr,BYTE d)
// {
//  BOOL data flag;
//
//  flag=i2c_write_byte(I2CADR,RegAddr,1,&d,I2CDEV);
//
//  return !flag;
// }
//
// SYS_STATUS HDMI_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
// {
//  BOOL data flag;
//
//  flag=i2c_read_byte(I2CADR,RegAddr,N,pData,I2CDEV);
//
//  return !flag;
// }
//
// SYS_STATUS HDMI_WriteI2C_ByteN(SHORT RegAddr,BYTE *pData,int N)
// {
//  BOOL data flag;
//
//  flag=i2c_write_byte(I2CADR,RegAddr,N,pData,I2CDEV);
//
//  return !flag;
// }
#endif

BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
 BYTE data p_data;

 i2c_read_byte(TX0ADR,RegAddr,1,&p_data,TX0DEV);

 return p_data;
}

SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
 BOOL data flag;

 flag=i2c_write_byte(TX0ADR,RegAddr,1,&d,TX0DEV);

 return !flag;
}

SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_read_byte(TX0ADR,RegAddr,N,pData,TX0DEV);

 return !flag;
}

SYS_STATUS HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_write_byte(TX0ADR,RegAddr,N,pData,TX0DEV);

 return !flag;
}

BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr)
{
 BYTE data p_data;

 i2c_read_byte(RXADR,RegAddr,1,&p_data,RXDEV);

 return p_data;
}

SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE d)
{
 BOOL data flag;

 flag=i2c_write_byte(RXADR,RegAddr,1,&d,RXDEV);

 return !flag;
}

SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_read_byte(RXADR,RegAddr,N,pData,RXDEV);

 return !flag;
}
/*
SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
 BOOL data flag;

 flag=i2c_write_byte(RXADR,RegAddr,N,pData,RXDEV);

 return !flag;
}
*/
#ifdef _EDIDI2C_
// alex 070320
// for edid eeprom i2c bus

void SET_EDID_SCL( BOOL bit_value )
{
    if(EDID_ROM_DEV1==TRUE)
        EDID1_SCL=bit_value;
    else
        EDID_SCL=bit_value;
    //EDID_SCL=bit_value;
//    DelayUS(DELAY_TIME);
}

void SET_EDID_SDA( BOOL bit_value)
{
    if(EDID_ROM_DEV1==TRUE)
        EDID1_SDA=bit_value;
    else
        EDID_SDA=bit_value;
    //EDID_SDA=bit_value;
//    DelayUS(DELAY_TIME);
}

BOOL GET_EDID_SDA()
{
    BOOL Ack;
    if(EDID_ROM_DEV1==TRUE)
        Ack=EDID1_SDA;
    else
        Ack=EDID_SDA;

    return Ack;
    //return EDID_SDA;
}

void I2C_EDID_START()
{
    SET_EDID_SDA( HIGH );
    SET_EDID_SCL( HIGH );
    SET_EDID_SDA( LOW );
    SET_EDID_SCL( LOW );
}

void I2C_EDID_WRITE( BYTE byte_data )
{
    BYTE data bit_cnt,tmp;
    BOOL data bit_value;

    for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
        tmp=(byte_data << bit_cnt) & 0x80;
        bit_value=tmp && 0x80;

        SET_EDID_SDA( bit_value );
        SET_EDID_SCL( HIGH );
        SET_EDID_SCL( LOW );
    }
}

BOOL I2C_EDID_WAIT_ACK( void )
{
    BOOL data ack_bit_value;

    SET_EDID_SDA( HIGH );
    SET_EDID_SCL( HIGH );
    ack_bit_value=GET_EDID_SDA();
    SET_EDID_SCL( LOW );

    return ack_bit_value;
}

void I2C_EDID_END( void )
{
    SET_EDID_SDA( LOW );
    SET_EDID_SCL( HIGH );
    SET_EDID_SDA( HIGH );
}

BOOL EDID_WRITE_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data )
{
    BYTE data i;

    I2C_EDID_START();                // S

    I2C_EDID_WRITE(address&0xFE);        // slave address (W)
    if( I2C_EDID_WAIT_ACK()==1 )
        {        // As
            I2C_EDID_END();
            return 0;
        }

    I2C_EDID_WRITE(offset);            // offset
    if( I2C_EDID_WAIT_ACK()==1 )
        {        // As
            I2C_EDID_END();
            return 0;
        }

    for(i=0; i<byteno-1; i++)
        {
            I2C_EDID_WRITE(*p_data);        // write d
            if( I2C_EDID_WAIT_ACK()==1 )
                {        // As
                    I2C_EDID_END();
                    return 0;
                }
            p_data++;
        }

    I2C_EDID_WRITE(*p_data);            // write last d
    if( I2C_EDID_WAIT_ACK()==1 )
        {        // As
            I2C_EDID_END();
            return 0;
        }
    else
        {
            I2C_EDID_END();
            return 1;
        }

}

#ifdef _EDIDPARSE_
SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d)
{
    BOOL  flag;
    BYTE  rddata,ErrorCnt=0;
    do {
        flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
        delay1ms(10);
        rddata=HDMI_ReadEDID_Byte(RegAddr);

        if(ErrorCnt)delay1ms(500);
        ErrorCnt++;
//        HDMITX_DEBUG_PRINTF(( " 01 error CNT=%X \n",ErrorCnt));
        if(ErrorCnt>5)break;
    }
    while( (flag==FALSE) || (rddata!=d));

#ifndef RX_SINGAL_PORT
    EDID_ROM_DEV1=TRUE;
    ErrorCnt=0;
    do {
        flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
        delay1ms(10);
        rddata=HDMI_ReadEDID_Byte(RegAddr);

        if(ErrorCnt)delay1ms(500);
        ErrorCnt++;
//        HDMITX_DEBUG_PRINTF(( " 02 error CNT=%X \n",ErrorCnt));
        if(ErrorCnt>5)break;
    }
    while( (flag==FALSE) || (rddata!=d));

    EDID_ROM_DEV1=FALSE;
#endif
    return !flag;

}
#else
#if 0
SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d)
{
    BYTE flag=0;
    EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
#ifndef RX_SINGAL_PORT
    EDID_ROM_DEV1=TRUE;
    EDID_WRITE_BYTE(EDID_ADR,RegAddr,1,&d);
    EDID_ROM_DEV1=FALSE;
#endif
    return !flag;

}
#endif
#endif

/*
SYS_STATUS HDMI_WriteEDID_ByteN(SHORT RegAddr,BYTE *pData,int N)
{
    BOOL data flag;

    flag=EDID_WRITE_BYTE(EDID_ADR,RegAddr,N,pData);

    return !flag;
}
*/


BYTE I2C_EDID_READ( void )
{
    BYTE data bit_cnt,byte_data;
    BOOL data bit_value;

    byte_data=0;
    for(bit_cnt=0; bit_cnt<8; bit_cnt++) {
        SET_EDID_SCL( HIGH );

        bit_value=GET_EDID_SDA();

        byte_data=(byte_data << 1) | bit_value;

        SET_EDID_SCL( LOW );
    }

    return byte_data;
}

void I2C_EDID_SEND_ACK( BOOL bit_value )
{
    SET_EDID_SDA( bit_value );
    SET_EDID_SCL( HIGH );
    SET_EDID_SCL( LOW );
    SET_EDID_SDA( HIGH );
}

BOOL EDID_READ_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data )
{
    BYTE data i;

    I2C_EDID_START();                // S

    I2C_EDID_WRITE(address&0xFE);        // slave address (W)
    if( I2C_EDID_WAIT_ACK()==1 ) {        // As
        I2C_EDID_END();
        return 0;
    }

    I2C_EDID_WRITE(offset);            // offset
    if( I2C_EDID_WAIT_ACK()==1 ) {        // As
        I2C_EDID_END();
        return 0;
    }

    I2C_EDID_START();

    I2C_EDID_WRITE(address|0x01);        // slave address (R)
    if( I2C_EDID_WAIT_ACK()==1 ) {        // As
        I2C_EDID_END();
        return 0;
    }

    for(i=0; i<byteno-1; i++) {
        *p_data=I2C_EDID_READ();        // read d
        I2C_EDID_SEND_ACK(LOW);        // Am

        p_data++;
    }

    *p_data=I2C_EDID_READ();            // read last d
    I2C_EDID_SEND_ACK(HIGH);            // NAm
    I2C_EDID_END();

    return 1;
}

BYTE HDMI_ReadEDID_Byte(BYTE RegAddr)
{
    BYTE data p_data;

    EDID_READ_BYTE(EDID_ADR,RegAddr,1,&p_data);
    return p_data;
}

/*
SYS_STATUS HDMI_ReadEDID_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
    BOOL data flag;

    flag=EDID_READ_BYTE(EDID_ADR,RegAddr,N,pData);

    return !flag;
}
*/

#endif    // _EDIDI2C_

#ifdef RX_KEY_SWITCH

#define POWER_KEY_RELEASE  1
#define POWER_KEY_PRESS       0
BYTE GetswitchRx(){
      BYTE KEY_in;
      KEY_in=RX_SWITCH_KEY;
      switch(KEY_in){
               case POWER_KEY_RELEASE :
                    return 0;

            case POWER_KEY_PRESS:
                 delay1ms(1);
                 KEY_in=RX_SWITCH_KEY;
                 if (KEY_in==POWER_KEY_RELEASE){
                     return 0;
                 }else{
                     delay1ms(20);
                 }
                 //while(KEY_in !=POWER_KEY_RELEASE)
                //        KEY_in=RX_SWITCH_KEY;
       }             return 1;

}

#endif


#else
#include "typedef_hdmitx.h"
#include "mcu.h"
#if	defined(Linux_UserMode)
static SYS_STATUS
I2C_Read_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    I2C_ACCESS_DATA I2C_DATA;
    BYTE *pI2CData;
    unsigned long   i;

    pI2CData = pData;
    
    I2C_DATA.channel = 2;	
    for (i = 0; i < N; i++)
    {
        I2C_DATA.address = Addr;
        I2C_DATA.index = RegAddr + i;
        if (ioctl (fd, IOCTL_I2C_READ, &I2C_DATA) < 0)
        {
            printf("[CAT] Read from Address:%x Index:%x Failed!! \n", Addr, (RegAddr+i));
            return ER_FAIL;        	
        }

        *(BYTE *) (pI2CData++) = I2C_DATA.data;
    }

    return ER_SUCCESS ;
}

static SYS_STATUS
I2C_Write_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    I2C_ACCESS_DATA I2C_DATA;
    BYTE *pI2CData;
    unsigned long   i;

    pI2CData = pData;
    
    I2C_DATA.channel = 2;	
    for (i = 0; i < N; i++)
    {
        I2C_DATA.address = Addr;
        I2C_DATA.index = RegAddr + i;
        I2C_DATA.data = *(BYTE *) (pI2CData++);
        if (ioctl (fd, IOCTL_I2C_WRITE, &I2C_DATA) < 0)
        {
            printf("[CAT] Write to Address:%x Index:%x Failed!! \n", Addr, (RegAddr+i));
            return ER_FAIL;        	
        }
    }

    return ER_SUCCESS ;
}
#elif	defined(Linux_KernelMode)
#include <asm/arch/drivers/I2C.h>

#ifndef CAN_NOT_FIND_DEVICE
#define CAN_NOT_FIND_DEVICE     1
#endif

#ifndef SET_I2C_DONE
#define SET_I2C_DONE            0
#endif

#ifdef	TestMode
#define	APB_BRIDGE_2_BASE       0x1E780000
#define	I2C_BASE		0xA000
#define AC_TIMING               0x77743335
#define LOOP_COUNT              0x100000

#define ReadMemoryLong(baseaddress,offset)        (*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))
#define WriteMemoryLong(baseaddress,offset,data)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset)))=(u32)(data)
#define WriteMemoryBYTE(baseaddress,offset,data)  *(volatile u8 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (u8)(data)    
#define WriteMemoryLongWithMASK(baseaddress, offset, data, mask)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (((*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))&(~(mask))) | (u32)((data)&(mask)))

void I2CInit(BYTE   DeviceSelect)
{
//  Reset
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 0);
//  Set Speed
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, AC_TIMING);
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0x7333f823);
//  Lower Speed
//    WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0, 0x33317805);
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x08, 0);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable Master Mode
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 1);
//  Enable Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0xAF);
}

int  SetI2CReg(u32   DeviceSelect, 
               u32   DeviceAddress, 
               u32   RegisterIndex, 
               BYTE   RegisterValue)
{
    ULONG   Status; 
    ULONG   Count = 0;
    ULONG flags;

    spin_lock_irqsave(&cat_s_lock_i2c, flags);
    	
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

		if (2 == Status)
		{
			//Clear Interrupt
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
			//Re-Send Start and Send Device Address while NACK return
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		}
		else
		{
			if (Count > LOOP_COUNT) {
                            goto SetI2CReg_Fail;
			}
		}
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Value and Stop
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterValue);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
	barrier();
//  Wait STOP
    do {
        Count++;
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
//  Disable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0, 0x10);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    return SET_I2C_DONE;
    
SetI2CReg_Fail:
    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    return CAN_NOT_FIND_DEVICE;

}

int  GetI2CReg(u32 DeviceSelect, 
	       u32 DeviceAddress, 
	       u32 RegisterIndex,
	       BYTE *outValue)
{
    BYTE    Data;
    ULONG   ulError = 0;
    ULONG   Status;
    int		i;
    ULONG flags;

    spin_lock_irqsave(&cat_s_lock_i2c, flags);

	//uinfo("GetI2CReg\n");
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
	barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait TX ACK
	i = 0;
    //uinfo("Start and Send Device Address\n");
    do {
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;
    } while (Status != 1);
    
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
	i = 0;
    //uinfo("Send Device Register Index\n");
    do {
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;        
    } while (Status != 1);
    
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Start, Send Device Address + 1(Read Mode), Receive Data
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress + 1);
	barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x1B);
//  Wait Rx Done
	i = 0;
    //uinfo("Start, Send Device Address + 1(Read Mode), Receive Data\n");
    do {
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x04) >> 2;
        
        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;           
    } while (Status != 1);

STOPI2C:   
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

//  Enable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
//  Wait STOP
	i = 0;
    //uinfo("Enable STOP Interrupt\n");
    do {
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
        
        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;                 
    } while (Status != 1);
    
//  Disable STOP Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    if (1 == ulError)
    {
        return 1;
    }
    else
    {
        //  Read Received Data
        Data = (BYTE)((ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00) >> 8);
        *outValue = Data;
        return    0;
    }
}
#endif

static SYS_STATUS
I2C_Read_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    BYTE *pI2CData, jData;
    unsigned long   i;

    pI2CData = pData;
    
    for (i = 0; i < N; i++)
    {
        if (GetI2CReg(HDMI_I2C_CHANNEL, Addr, (RegAddr + i), &jData) != 0)
        {
            return ER_FAIL;	
        }
	
        *(BYTE *) (pI2CData++) = jData;
    }

    return ER_SUCCESS ;
	
}

static SYS_STATUS
I2C_Write_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    unsigned long   i;
    
    for (i = 0; i < N; i++)
    {
        if (SetI2CReg(HDMI_I2C_CHANNEL, Addr, (RegAddr + i), *(BYTE *)(pData + i)) != 0)
        {
            return ER_FAIL;	
        }	
    }

    return ER_SUCCESS ;
}
#endif

static BYTE
I2C_Read_Byte(BYTE Addr,BYTE RegAddr)
{
    BYTE data ;
    I2C_Read_ByteN(Addr,RegAddr,&data,1)  ;
    return data ;
}

static SYS_STATUS
I2C_Write_Byte(BYTE Addr,BYTE RegAddr,BYTE Data)
{
    return I2C_Write_ByteN(Addr,RegAddr,&Data,1) ;
}

BYTE
HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
	return I2C_Read_Byte(HDMI_TX_I2C_SLAVE_ADDR,RegAddr) ;
}

SYS_STATUS
HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE Data)
{
	return I2C_Write_Byte(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,Data) ;
}

SYS_STATUS
HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Read_ByteN(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}

SYS_STATUS
HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Write_ByteN(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}


#ifndef CONFIG_AST1500_CAT6613
BYTE
HDMIRX_ReadI2C_Byte(BYTE RegAddr)
{
	return I2C_Read_Byte(HDMI_RX_I2C_SLAVE_ADDR,RegAddr) ;
}

SYS_STATUS
HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE Data)
{
	return I2C_Write_Byte(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,Data) ;
}

SYS_STATUS
HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Read_ByteN(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}

SYS_STATUS
HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Write_ByteN(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}
#endif

#endif
#endif

