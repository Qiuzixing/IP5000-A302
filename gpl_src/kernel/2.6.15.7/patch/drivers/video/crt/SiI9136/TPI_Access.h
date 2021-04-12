/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  TPI_Access.h
 *
 * @brief Defines and prototypes of the TPI Access.
 *
 *****************************************************************************
*/


uint8_t ReadByteTPI(uint8_t);
void WriteByteTPI(uint8_t, uint8_t);

uint8_t ReadByteCBUS (uint8_t Offset);
void WriteByteCBUS (uint8_t Offset, uint8_t Data);
void ReadModifyWriteCBUS (uint8_t Offset, uint8_t Mask, uint8_t Value);

void ReadSetWriteTPI (uint8_t, uint8_t);
void ReadClearWriteTPI (uint8_t, uint8_t);
void ReadModifyWriteTPI (uint8_t Offset, uint8_t Mask, uint8_t Value);

void ReadBlockTPI (uint8_t, uint16_t, uint8_t *);
void WriteBlockTPI (uint8_t, uint16_t, uint8_t *);
#if AST_HDMITX
u8 sii9136_identify_chip(void);
#endif

uint8_t GetDDC_Access(uint8_t* SysCtrlRegVal);
uint8_t ReleaseDDC(uint8_t SysCtrlRegVal);
void ReadBlockEDID(uint8_t offset, uint8_t *buffer, uint16_t length);
void ReadSegmentBlockEDID(uint8_t segment, uint8_t offset, uint8_t *buffer, uint16_t length);

#ifdef READKSV
void ReadBlockHDCP(uint8_t, uint16_t, uint8_t *);
void ReadBlockHDCPAndDiscard(uint8_t, uint16_t);
#endif

uint8_t ReadIndexedRegister(uint8_t, uint8_t);
void WriteIndexedRegister(uint8_t, uint8_t, uint8_t);
void ReadModifyWriteIndexedRegister(uint8_t, uint8_t, uint8_t, uint8_t);
void ReadIndexedRegisterBlock(uint8_t PageNum, uint8_t RegOffset, uint8_t *pData, uint16_t NBytes);
void WriteIndexedRegisterBlock(uint8_t PageNum, uint8_t RegOffset, uint8_t *pData, uint16_t NBytes);

uint8_t ReadTxPage0Register(uint8_t);
void WriteTxPage0Register(uint8_t, uint8_t);
void ReadModifyWriteTxPage0Register(uint8_t, uint8_t, uint8_t);
#ifdef WP1API //(
uint8_t ReadTxPage1Register(uint8_t);
void WriteTxPage1Register(uint8_t, uint8_t);
void ReadModifyWriteTxPage1Register(uint8_t, uint8_t, uint8_t);
#endif
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Indexed Register Offsets, Constants
//====================================
#define INDEXED_PAGE_0_HUH		0x00
#define INDEXED_PAGE_0			0x01
#define INDEXED_PAGE_1			0x02
#define INDEXED_PAGE_2			0x03

#define DEVICE_ID_LOW_BYTE      0x02
#define DEVICE_ID_HI_BYTE       0x03
#define AUDIO_INPUT_LENGTH		0x24

#define TMDS_CONT_REG           0x82

#define SW_RESET                0x05
#define POWER_DOWN              0x6F


#define DIV_BY_2                0x00
#define MULT_BY_1               0x01
#define MULT_BY_2               0x02
#define MULT_BY_4               0x03

#ifdef DEV_INDEXED_PLL
uint8_t SetPLL(uint8_t);
#endif

// Prototype Declarations
//=======================

uint8_t SetInputWordLength(uint8_t);

uint8_t SetChannelLayout(uint8_t);
