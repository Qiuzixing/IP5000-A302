///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <io.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2011/06/23
//   @fileversion: HDMIRX_SAMPLE_2.13
//******************************************/

#ifndef _IO_h_
#define _IO_h_
#include "config.h"
#include "typedef.h"
#include "debug.h"

///////////////////////////////////////////////////////////////////////////////
// Start: I2C for 8051
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// I2C for original function call
///////////////////////////////////////////////////////////////////////////////

#ifndef AST_HDMIRX
BOOL i2c_write( BYTE address, BYTE offset, BYTE byteno, BYTE dataIn );
BYTE i2c_read( BYTE address, BYTE offset, BYTE byteno, BYTE* rddata );
#endif

BYTE ReadI2C_Byte(BYTE RegAddr);
SYS_STATUS WriteI2C_Byte(BYTE RegAddr, BYTE d);

SYS_STATUS ReadI2C_ByteN(BYTE RegAddr, BYTE *pData, int N);
SYS_STATUS WriteI2C_ByteN(SHORT RegAddr, BYTE *pData, int N);

BOOL i2c_read_byte( BYTE , BYTE , BYTE , BYTE *p_data, BYTE  );
BOOL i2c_write_byte( BYTE , BYTE , BYTE , BYTE *DataIn, BYTE  );

void hdmirx_clear_hpd(void) ;
void hdmirx_set_hpd(void) ;

///////////////// Delay Utility ////////////////////
void DelayUS(USHORT);
void delay1ms(USHORT);
#ifndef AST_HDMIRX
void HoldSystem();
BOOL DisableVGAChecking(void);
#endif

#define I2C_OrReg_Byte(reg,ormask) WriteI2C_Byte(reg,(ReadI2C_Byte(reg) | (ormask)))
#define I2C_AndReg_Byte(reg,andmask) WriteI2C_Byte(reg,(ReadI2C_Byte(reg) & (andmask)))
#define I2C_SetReg_Byte(reg,andmask,ormask) WriteI2C_Byte(reg,((ReadI2C_Byte(reg) & (andmask))|(ormask)))
#ifndef AST_HDMIRX
BOOL EDID_READ_BYTE( BYTE address, BYTE offset, BYTE byteno, BYTE *p_data, BYTE device);
BOOL EDID_WRITE_BYTE( BYTE address, BYTE offset, BYTE byteno, BYTE *p_data, BYTE device );
#ifdef 	_Myson_CS8953_
#include "CS8953sfr.h"
void Initial_SFR();
void Initial_OSC();
#endif
#endif

#ifndef Debug_message

#endif
#ifndef AST_HDMIRX
extern    BYTE   idata I2CADR;
extern    BYTE   idata I2CDEV;
#endif

#endif
