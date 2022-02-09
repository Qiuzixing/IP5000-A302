///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >IO.h<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/

#ifndef _IO_h_
#define _IO_h_

#ifndef CONFIG_AST1500_CAT6613
#include "mcu.h"
#include "reg_c51.h"
#include "main.h"
#include "utility.h"
#endif

///////////////////////////////////////////////////////////////////////////////
// Start: I2C for 8051
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// I2C for original function call
///////////////////////////////////////////////////////////////////////////////
#define BUAD_RATE               57600L  //19200L
#define BAUD_SETTING            (65536L - (CLOCK / (32L * BUAD_RATE)))




// BYTE HDMI_ReadI2C_Byte(BYTE RegAddr);
// SYS_STATUS HDMI_WriteI2C_Byte(BYTE RegAddr,BYTE d);
// SYS_STATUS HDMI_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
// SYS_STATUS HDMI_WriteI2C_ByteN(SHORT RegAddr,BYTE *pData,int N);
// #define HDMI_OrReg_Byte(reg,ormask) HDMI_WriteI2C_Byte(reg,(HDMI_ReadI2C_Byte(reg) | (ormask)))
// #define HDMI_AndReg_Byte(reg,andmask) HDMI_WriteI2C_Byte(reg,(HDMI_ReadI2C_Byte(reg) & (andmask)))
// #define HDMI_SetReg_Byte(reg,andmask,ormask) HDMI_WriteI2C_Byte(reg,((HDMI_ReadI2C_Byte(reg) & (andmask))|(ormask)))


#ifndef CONFIG_AST1500_CAT6613
BYTE HDMIRX_ReadI2C_Byte(BYTE RegAddr);
SYS_STATUS HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE d);
SYS_STATUS HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
//SYS_STATUS HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
#define HDMIRX_OrReg_Byte(reg,ormask) HDMIRX_WriteI2C_Byte(reg,(HDMIRX_ReadI2C_Byte(reg) | (ormask)))
#define HDMIRX_AndReg_Byte(reg,andmask) HDMIRX_WriteI2C_Byte(reg,(HDMIRX_ReadI2C_Byte(reg) & (andmask)))
#define HDMIRX_SetReg_Byte(reg,andmask,ormask) HDMIRX_WriteI2C_Byte(reg,((HDMIRX_ReadI2C_Byte(reg) & (andmask))|(ormask)))
#endif

BYTE HDMITX_ReadI2C_Byte(BYTE RegAddr);
SYS_STATUS HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE d);
SYS_STATUS HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
SYS_STATUS HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N);
#define HDMITX_OrReg_Byte(reg,ormask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) | (ormask)))
#define HDMITX_AndReg_Byte(reg,andmask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) & (andmask)))
#define HDMITX_SetReg_Byte(reg,andmask,ormask) HDMITX_WriteI2C_Byte(reg,((HDMITX_ReadI2C_Byte(reg) & (andmask))|(ormask)))


#ifdef _EDIDI2C_
// alex 070320
// for edid eeprom i2c bus
SYS_STATUS HDMI_WriteEDID_Byte(BYTE RegAddr,BYTE d);
BOOL EDID_WRITE_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data );
#ifdef _ENPARSE_
BYTE I2C_EDID_READ( void );
void I2C_EDID_SEND_ACK( BOOL bit_value );
BOOL EDID_READ_BYTE( BYTE address,BYTE offset,BYTE byteno,BYTE *p_data );
BYTE HDMI_ReadEDID_Byte(BYTE RegAddr);
#endif    // _PARSEEDID
#endif    // _EDIDI2C_

extern    BYTE    I2CADR;
extern    BYTE    I2CDEV;
extern    BYTE    EDID_ROM_DEV1;

#ifdef RX_KEY_SWITCH
BYTE GetswitchRx();
#endif
void HotPlug(BOOL Enable);
#endif



