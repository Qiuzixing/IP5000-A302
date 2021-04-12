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
 * @file  TPI_Access.c
 *
 * @brief Implementation of the TPI Access.
 *
 *****************************************************************************
*/

#include <linux/module.h>

//#include <stdio.h>
#include "defs.h"
#include "si_basetypes.h"
#include "si_datatypes.h"
//#include "at89c51xd2.h"
#include "constants.h"
#include "defs.h"
//#include "gpio.h"


#include "i2c_slave_addrs.h"
#include "videomodedesc.h"
//#include "serialpacket.h"
#include "TPI_Regs.h"
//#include "AMF_Lib.h"
#include "Externals.h"
#include "Macros.h"
#if AST_HDMITX
#include "ast_i2c.h"
#else
#include "i2c_master_sw.h"
#endif
#include "TPI_Access.h"
//#include "delay.h"
#include "TPI_generic.h"
#include "tpidebug.h"
#include "ast_utilities.h"

#define T_DDC_ACCESS    50

#define TPI_INDEXED_PAGE_REG	0xBC
#define TPI_INDEXED_OFFSET_REG	0xBD
#define TPI_INDEXED_VALUE_REG	0xBE

#if AST_HDMITX
u8 sii9136_identify_chip(void)
{
	int i;
	u8 i2c_addr[2] = {0x72, 0x76}; /* selectd by CI2CA pin, 0 => 0x72, 1 => 0x76 */
	u8 sii9136_addr, result = 0;
	u8 devID;
	u16 wID ;

	for (i = 0; i < 2; i++) {
		sii9136_addr = i2c_addr[i];

		devID = I2C_ReadByte(sii9136_addr, TXL_PAGE_0_DEV_IDH_ADDR);
        	wID = devID;
	        wID <<= 8;
        	devID = I2C_ReadByte(sii9136_addr, TXL_PAGE_0_DEV_IDL_ADDR);
	        wID |= devID;

		if (0x9136 == wID) {
			uinfo("Find Sii9136 Chip I2C address 0x%X\n", (int) sii9136_addr);
			/* TODO to fix I2C address if it is 0x76 */
			result = 1;
			break;
		}

	}

	return result;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ReadByteTPI ()
//
// PURPOSE      :   Read one byte from a given offset of the TPI interface.
//
// INPUT PARAMS :   RegOffset of TPI register to be read; A pointer to the variable
//                  where the data read will be stored
//
// OUTPUT PARAMS:   Data - Contains the value read from the register value
//                  specified as the first input parameter
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE
//
// NOTE         :   ReadByteTPI() is ported from the PC based FW to the uC
//                  version while retaining the same function interface. This
//                  will save the need to modify higher level I/O functions
//                  such as ReadSetWriteTPI(), ReadClearWriteTPI() etc.
//                  A dummy returned value (always TRUE) is provided for
//                  the same reason
//
//////////////////////////////////////////////////////////////////////////////
uint8_t ReadByteTPI(uint8_t RegOffset)
{
    return I2C_ReadByte(SA_TX_Page0_Primary, RegOffset);
}


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION		:	WriteByteTPI ()
//
// PURPOSE		:	Write a value to a TPI register.
//
// INPUT PARAMS	:	RegOffset	-	the offset of the TPI register to be written.
//					Data	    -	the value to be written.
//
// OUTPUT PARAMS:	None
//
// GLOBALS USED	:	None
//
// RETURNS		:	void
//
//////////////////////////////////////////////////////////////////////////////

void WriteByteTPI(uint8_t RegOffset, uint8_t Data)
{
    I2C_WriteByte(SA_TX_Page0_Primary, RegOffset, Data);
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ReadModifyWriteTPI()
//
// PURPOSE		 :	Set or clear individual bits in a TPI register.
//
// INPUT PARAMS  :  RegOffset  :   the offset of the TPI register to be modified.
//                  Mask    :   "1" for each TPI register bit that needs to be
//                              modified
//					Value   :   The desired value for the register bits in their
//								proper positions
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  void
//
//////////////////////////////////////////////////////////////////////////////

void ReadModifyWriteTPI(uint8_t RegOffset, uint8_t Mask, uint8_t Value)
{
    uint8_t Temp;

    Temp = ReadByteTPI(RegOffset);		// Read the current value of the register.
    Temp &= ~Mask;					// Clear the bits that are set in Mask.
	Temp |= (Value & Mask);			// OR in new value. Apply Mask to Value for safety.
    WriteByteTPI(RegOffset, Temp);		// Write new value back to register.
    if (Value & ~Mask) // catch improper mask values
    {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"\n\nTPI Mask does not span Value offset 0x%x - \n",(int)RegOffset));
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION		:	ReadByteCBUS ()
//
// PURPOSE		:	Read the value from a CBUS register.
//
// INPUT PARAMS	:	RegOffset - the offset of the CBUS register to be read.
//
// OUTPUT PARAMS:	None
//
// GLOBALS USED	:	None
//
// RETURNS		:	The value read from the CBUS register.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t ReadByteCBUS (uint8_t RegOffset)
{
	return I2C_ReadByte(SA_TX_CBUS_Primary, RegOffset);
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION		:	WriteByteCBUS ()
//
// PURPOSE		:	Write a value to a CBUS register.
//
// INPUT PARAMS	:	RegOffset	-	the offset of the TPI register to be written.
//					Data	-	the value to be written.
//
// OUTPUT PARAMS:	None
//
// GLOBALS USED	:	None
//
// RETURNS		:	void
//
//////////////////////////////////////////////////////////////////////////////

void WriteByteCBUS(uint8_t RegOffset, uint8_t Data)
{
	I2C_WriteByte(SA_TX_CBUS_Primary, RegOffset, Data);
}

//////
//
/////

void ReadModifyWriteCBUS(uint8_t RegOffset, uint8_t Mask, uint8_t Value)
{
    uint8_t Temp;

    Temp = ReadByteCBUS(RegOffset);
    Temp &= ~Mask;
	Temp |= (Value & Mask);
    WriteByteCBUS(RegOffset, Temp);
    if (Value & ~Mask) // catch improper mask values
    {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"\n\nCBUS Mask does not span Value 0x%x:0x%x - \n",(int)RegOffset, (int)Mask));
    }
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   ReadIndexedRegister()
//
// PURPOSE      :   Read from an indexed register.
//
//                  Write:
//                      1. 0xBC => Indexed page num
//                      2. 0xBD => Indexed register offset
//
//                  Read:
//                      3. 0xBE => Returns the indexed register value
//
// INPUT PARAMS :   PageNum -   indexed page number
//                  RegOffset   -   offset of the register within the indexed page.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   The value read from the indexed register.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t ReadIndexedRegister(uint8_t PageNum, uint8_t RegOffset)
{
    WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);		// Indexed page
    WriteByteTPI(TPI_INDEXED_OFFSET_REG, RegOffset);	// Indexed register
    return ReadByteTPI(TPI_INDEXED_VALUE_REG); 		// Return read value
}

uint8_t ReadTxPage0Register(uint8_t RegOffset)
{
uint8_t retVal;
    if (bInTpiMode)
    {
        retVal = ReadIndexedRegister(INDEXED_PAGE_0, RegOffset);
    }
    else
    {
        retVal = I2C_ReadByte(SA_TX_Page0_Primary, RegOffset);
    }
    return retVal;
}
#ifdef WP1API //(
uint8_t ReadTxPage1Register(uint8_t RegOffset)
{
uint8_t retVal;
    if (bInTpiMode)
    {
        retVal = ReadIndexedRegister(INDEXED_PAGE_1, RegOffset);
    }
    else
    {
        retVal = I2C_ReadByte(SA_TX_Page1_Primary, RegOffset);
    }
    return retVal;
}
#endif //)


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   WriteIndexedRegister()
//
// PURPOSE      :   Write a value to an indexed register
//
//                  Write:
//                      1. 0xBC => Indexed page num
//                      2. 0xBD => Indexed register offset
//                      3. 0xBE => Set the indexed register value
//
// INPUT PARAMS	:	PageNum	-	indexed page number
//					RegOffset	-	offset of the register within the indexed page.
//					Data	-	the value to be written.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   None
//
//////////////////////////////////////////////////////////////////////////////

void WriteIndexedRegister(uint8_t PageNum, uint8_t RegOffset, uint8_t RegValue)
{
    WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);  // Indexed page
    WriteByteTPI(TPI_INDEXED_OFFSET_REG, RegOffset);  // Indexed register
    WriteByteTPI(TPI_INDEXED_VALUE_REG, RegValue);    // Write value
}

void WriteTxPage0Register(uint8_t RegOffset, uint8_t Value)
{
    if (bInTpiMode)
    {
        WriteIndexedRegister(INDEXED_PAGE_0, RegOffset, Value);
    }
    else
    {
        I2C_WriteByte(SA_TX_Page0_Primary, RegOffset, Value);
    }
}
#ifdef WP1API //(
void WriteTxPage1Register(uint8_t RegOffset, uint8_t Value)
{
    if (bInTpiMode)
    {
        WriteIndexedRegister(INDEXED_PAGE_1, RegOffset, Value);
    }
    else
    {
        I2C_WriteByte(SA_TX_Page1_Primary, RegOffset, Value);
    }
}
#endif //)

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ReadModifyWriteIndexedRegister()
//
// PURPOSE		:	Set or clear individual bits in a TPI register.
//
// INPUT PARAMS  :	PageNum	-	indexed page number
//					RegOffset	-	the offset of the indexed register to be modified.
//                  Mask    -   "1" for each indexed register bit that needs to be
//                              modified
//					Value   -   The desired value for the register bits in their
//								proper positions
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  void
//
//////////////////////////////////////////////////////////////////////////////

void ReadModifyWriteIndexedRegister(uint8_t PageNum, uint8_t RegOffset, uint8_t Mask, uint8_t Value)
{
    uint8_t Temp;

    WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);
    WriteByteTPI(TPI_INDEXED_OFFSET_REG, RegOffset);
    Temp = ReadByteTPI(TPI_INDEXED_VALUE_REG);
                                                        // Read the current value of the register.
    Temp &= ~Mask;                                      // Clear the bits that are set in Mask.
	Temp |= (Value & Mask);                             // OR in new value. Apply Mask to Value for safety.

    WriteByteTPI(TPI_INDEXED_VALUE_REG, Temp);          // Write new value back to register.
    if (Value & ~Mask) // catch improper mask values
    {
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"\n\nIndexed Mask does not span Value 0x%x:0x%x - \n",(int)PageNum,(int)RegOffset));
    }
}

#define ReadModifyWriteTxPageFooRegister(slaveAddr,pageNum,offset,mask,value)   \
uint8_t temp;                                                                   \
    if(bInTpiMode)                                                              \
    {                                                                           \
        temp = ReadIndexedRegister(pageNum,offset);                             \
        temp &= ~(mask);                                                        \
        temp |=  (value); /*don't do any superfluous ANDing here*/              \
        WriteIndexedRegister(pageNum,offset,temp);                              \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        temp = I2C_ReadByte(slaveAddr,offset);                                  \
        temp &= ~(mask);                                                        \
        temp |=  (value); /*don't do any superfluous ANDing here*/              \
        I2C_WriteByte(slaveAddr,offset,temp);                                   \
    }                                                                           \
    if (value & ~mask) /* catch improper mask values */                         \
    {                                                                           \
		TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"\n\nTxPageFoo Mask does not span Value 0x%x:0x%x - \n",(int)slaveAddr,(int)offset)); \
    }

void ReadModifyWriteTxPage0Register(uint8_t RegOffset, uint8_t Mask, uint8_t Value)
{
    ReadModifyWriteTxPageFooRegister(SA_TX_Page0_Primary,INDEXED_PAGE_0,RegOffset,Mask,Value)
}

#ifdef WP1API //(

void ReadModifyWriteTxPage1Register(uint8_t RegOffset, uint8_t Mask, uint8_t Value)
{
    ReadModifyWriteTxPageFooRegister(SA_TX_Page1_Primary,INDEXED_PAGE_1,RegOffset,Mask,Value)
}

#endif //)

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ReadSetWriteTPI(uint8_t RegOffset, uint8_t Pattern)
//
// PURPOSE       :  Write "1" to all bits in TPI offset "RegOffset" that are set
//                  to "1" in "Pattern"; Leave all other bits in "RegOffset"
//                  unchanged.
//
// INPUT PARAMS  :  RegOffset  :   TPI register offset
//                  Pattern :   GPIO bits that need to be set
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////
void ReadSetWriteTPI(uint8_t RegOffset, uint8_t Pattern)
{
    uint8_t Tmp;

    Tmp = ReadByteTPI(RegOffset);

    Tmp |= Pattern;
    WriteByteTPI(RegOffset, Tmp);
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ReadClearWriteTPI(uint8_t RegOffset, uint8_t Pattern)
//
// PURPOSE       :  Write "0" to all bits in TPI offset "RegOffset" that are set
//                  to "1" in "Pattern"; Leave all other bits in "RegOffset"
//                  unchanged.
//
// INPUT PARAMS  :  RegOffset  :   TPI register offset
//                  Pattern :   "1" for each TPI register bit that needs to be
//                              cleared
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE
//
//////////////////////////////////////////////////////////////////////////////
void ReadClearWriteTPI(uint8_t RegOffset, uint8_t Pattern)
{
    uint8_t Tmp;

    Tmp = ReadByteTPI(RegOffset);

    Tmp &= ~Pattern;
    WriteByteTPI(RegOffset, Tmp);
}



////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION         :   ReadBlockTPI ()
//
// PURPOSE          :   Read NBytes from offset Addr of the TPI slave address
//                      into a byte Buffer pointed to by Data
//
// INPUT PARAMETERS :   TPI register offset, number of bytes to read and a
//                      pointer to the data buffer where the data read will be
//                      saved
//
// OUTPUT PARAMETERS:   pData - pointer to the buffer that will store the TPI
//                      block to be read
//
// RETURNED VALUE   :   VOID
//
// GLOBALS USED     :   None
//
////////////////////////////////////////////////////////////////////////////////
void ReadBlockTPI(uint8_t TPI_Offset, uint16_t NBytes, uint8_t * pData)
{
    I2C_ReadBlock(SA_TX_Page0_Primary, TPI_Offset, pData, NBytes);
}


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION         :   WriteBlockTPI ()
//
// PURPOSE          :   Write NBytes from a byte Buffer pointed to by Data to
//                      the TPI I2C slave starting at offset Addr
//
// INPUT PARAMETERS :   TPI register offset to start writing at, number of bytes
//                      to write and a pointer to the data buffer that contains
//                      the data to write
//
// OUTPUT PARAMETERS:   None.
//
// RETURNED VALUES  :   void
//
// GLOBALS USED     :   None
//
////////////////////////////////////////////////////////////////////////////////
void WriteBlockTPI(uint8_t TPI_Offset, uint16_t NBytes, uint8_t * pData)
{
	/*
	 * TODO
	 * We got a problem when access register 0x9 and 0xA via block access
	 * workaroud => change to byte mode
	 */
#if 0
    I2C_WriteBlock(SA_TX_Page0_Primary, TPI_Offset, pData, NBytes);
#else
	int i;
	for (i = 0; i < NBytes; i++)
		I2C_WriteByte(SA_TX_Page0_Primary, TPI_Offset + i, *(pData + i));
#endif
}


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  GetDDC_Access(void)
//
// PURPOSE       :  Request access to DDC bus from the receiver
//
// INPUT PARAMS  :  None
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE if bus obtained successfully. FALSE if failed.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t GetDDC_Access (uint8_t* SysCtrlRegVal)
{
	uint8_t sysCtrl;
	uint8_t DDCReqTimeout = T_DDC_ACCESS;
	uint8_t TPI_ControlImage;

	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>GetDDC_Access()\n"));

	sysCtrl = ReadByteTPI (PAGE_0_TPI_SC_ADDR);			// Read and store original value. Will be passed into ReleaseDDC()
	*SysCtrlRegVal = sysCtrl;

	sysCtrl |= DDC_BUS_REQUEST_REQUESTED;
	WriteByteTPI (PAGE_0_TPI_SC_ADDR, sysCtrl);

	while (DDCReqTimeout--)											// Loop till 0x1A[1] reads "1"
	{
		TPI_ControlImage = ReadByteTPI(PAGE_0_TPI_SC_ADDR);

		if (TPI_ControlImage & DDC_BUS_GRANT_MASK)					// When 0x1A[1] reads "1"
		{
			sysCtrl |= DDC_BUS_GRANT_GRANTED;
			WriteByteTPI(PAGE_0_TPI_SC_ADDR, sysCtrl);		// lock host DDC bus access (0x1A[2:1] = 11)
    	    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: DDCReq Granted %02X\n", (int)DDCReqTimeout ));
			return TRUE;
		}
		WriteByteTPI(PAGE_0_TPI_SC_ADDR, sysCtrl);			// 0x1A[2] = "1" - Requst the DDC bus
		DelayMS(200); //TODO: break this one up if time allows.
	}

	WriteByteTPI(PAGE_0_TPI_SC_ADDR, sysCtrl);				// Failure... restore original value.
    TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Drv: DDCReqTimeout %02X\n", (int)DDCReqTimeout ));
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION      :  ReleaseDDC(void)
//
// PURPOSE       :  Release DDC bus
//
// INPUT PARAMS  :  None
//
// OUTPUT PARAMS :  None
//
// GLOBALS USED  :  None
//
// RETURNS       :  TRUE if bus released successfully. FALSE if failed.
//
//////////////////////////////////////////////////////////////////////////////

uint8_t ReleaseDDC(uint8_t SysCtrlRegVal)
{
	uint8_t DDCReqTimeout = T_DDC_ACCESS;
	uint8_t TPI_ControlImage;

	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>ReleaseDDC()\n"));

	SysCtrlRegVal &= ~BITS_2_1;					// Just to be sure bits [2:1] are 0 before it is written

	while (DDCReqTimeout--)						// Loop till 0x1A[1] reads "0"
	{
		// Cannot use ReadClearWriteTPI() here. A read of TPI_SYSTEM_CONTROL is invalid while DDC is granted.
		// Doing so will return 0xFF, and cause an invalid value to be written back.
		//ReadClearWriteTPI(TPI_SYSTEM_CONTROL,BITS_2_1); // 0x1A[2:1] = "0" - release the DDC bus

		WriteByteTPI(PAGE_0_TPI_SC_ADDR, SysCtrlRegVal);
		TPI_ControlImage = ReadByteTPI(PAGE_0_TPI_SC_ADDR);

		if (!(TPI_ControlImage & BITS_2_1))		// When 0x1A[2:1] read "0"
			return TRUE;
#if AST_HDMITX
		DelayMS(10);
#endif
	}

	return FALSE;								// Failed to release DDC bus control
}


////////////////////////////////////////////////////////////////////////////////
//
// FUNCTION         :   ReadBlockHDCP ()
//
// PURPOSE          :   Read NBytes from offset Addr of the HDCP slave address
//                      into a byte Buffer pointed to by Data
//
// INPUT PARAMETERS :   HDCP port offset, number of bytes to read and a
//                      pointer to the data buffer where the data read will be
//                      saved
//
// OUTPUT PARAMETERS:   Data, a buffer that contains the block read from the CPI
//
// RETURNED VALUE   :   void
//
// GLOBALS USED     :   None
//
////////////////////////////////////////////////////////////////////////////////
#ifdef READKSV
void ReadBlockHDCP(uint8_t TPI_Offset, uint16_t NBytes, uint8_t * pData)
{
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>ReadBlockHDCP()\n"));
    I2C_ReadBlock(SA_RX_HDCP_EDDC, TPI_Offset, pData, NBytes);
}
void ReadBlockHDCPAndDiscard(uint8_t TPI_Offset, uint16_t NBytes)
{
	TPI_TRACE_PRINT((TPI_TRACE_CHANNEL,">>ReadBlockHDCP()\n"));
    I2C_ReadBlockAndDiscard(SA_RX_HDCP_EDDC, TPI_Offset, NBytes);
}
#endif


void ReadIndexedRegisterBlock(uint8_t PageNum, uint8_t RegOffset, uint8_t *pData, uint16_t NBytes)
{
	uint16_t count;

    WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);							// Internal page

	for (count = 0; count < NBytes; count++)
	{
	    WriteByteTPI(TPI_INDEXED_OFFSET_REG, (uint8_t) (RegOffset + count));	// Indexed register
    	pData[count] = ReadByteTPI(TPI_INDEXED_VALUE_REG);					// Read value from indexed register
	}
}

void WriteIndexedRegisterBlock(uint8_t PageNum, uint8_t RegOffset, uint8_t *pData, uint16_t NBytes)
{
	uint16_t count;

    WriteByteTPI(TPI_INDEXED_PAGE_REG, PageNum);							// Internal page

	for (count = 0; count < NBytes; count++)
	{
	    WriteByteTPI(TPI_INDEXED_OFFSET_REG, (uint8_t) (RegOffset + count));	// Indexed register
    	WriteByteTPI(TPI_INDEXED_VALUE_REG, pData[count]);					// Write value to indexed register
	}
}

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetPLL()
//
// PURPOSE      :   Set the 9024/9024/9222 chip PLL to multiply the input pixel
//                  clock by the value passed to this function as a parameter
//                  registers:
//
// INPUT PARAMS :   PLL Multiplication factor (represents 0.5, 1, 2 or 4)
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE
//
//////////////////////////////////////////////////////////////////////////////
#ifdef DEV_INDEXED_PLL
uint8_t SetPLL(uint8_t TClkSel)
{
    uint8_t RegValue;
    uint8_t Pattern;

    // Set up register 0x82[6:5] (same for all input pixel frequencies for a specific multiplier value):

    RegValue = ReadIndexedRegister(INDEXED_PAGE_0, TMDS_CONT_REG);

    Pattern = TClkSel << 5;
    RegValue &= ~BITS_6_5;
    RegValue |= Pattern;

    WriteByteTPI(TPI_REGISTER_VALUE_REG, RegValue);

    return TRUE;
}
#endif


//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetChannelLayout()
//
// PURPOSE      :   Set up the Channel layout field of internal register 0x2F
//                  (0x2F[1])
//
// INPUT PARAMS :   Number of audio channels: "0 for 2-Channels ."1" for 8.
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE
//
//////////////////////////////////////////////////////////////////////////////
#ifdef SetChannelLayout
uint8_t SetChannelLayout(uint8_t Count)
{
    // Indexed register 0x7A:0x2F[1]:
    WriteByteTPI(TPI_INDEXED_PAGE_REG, 0x02); // Internal page 1
    WriteByteTPI(TPI_REGISTER_OFFSET_REG, 0x2F);

    Count &= THREE_LSBITS;

    if (Count == TWO_CHANNEL_LAYOUT)
    {
        // Clear 0x2F:
        ReadClearWriteTPI(TPI_REGISTER_VALUE_REG, BIT_1);
    }

    else if (Count == EIGHT_CHANNEL_LAYOUT)
    {
        // Set 0x2F[0]:
        ReadSetWriteTPI(TPI_REGISTER_VALUE_REG, BIT_1);
    }

    return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     :   SetInputWordLength()
//
// PURPOSE      :   Tell the Tx what the I2S input word is
//
// INPUT PARAMS :   Input word length code per SiI-PR-1032-0.02, table 20
//
// OUTPUT PARAMS:   None
//
// GLOBALS USED :   None
//
// RETURNS      :   TRUE
//
// NOTE         :   Used by 9022/4 only. 9022A/24A and 9334 set that value
//                  automatically when 0x25[5:3] is set
//
//////////////////////////////////////////////////////////////////////////////
#ifdef SETINPUTWORDLENGTH
uint8_t SetInputWordLength(uint8_t Length)
{
    uint8_t RegValue;

    RegValue = ReadIndexedRegister(INDEXED_PAGE_1, AUDIO_INPUT_LENGTH);

    RegValue &= ~LOW_NIBBLE;
    RegValue |= Length;

    WriteByteTPI(TPI_REGISTER_VALUE_REG, RegValue);

    return TRUE;
}
#endif


void ReadBlockEDID(uint8_t offset, uint8_t *buffer, uint16_t length)
{
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Read Block EDID Entry\n"));
	I2C_ReadBlock(SA_EDID, offset, buffer, length);
	TPI_DEBUG_PRINT((TPI_DEBUG_CHANNEL,"Read Block EDID Exit\n"));
}

void ReadSegmentBlockEDID(uint8_t segment, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	I2C_ReadSegmentBlock(SA_EDID, segment, offset, buffer, length);
}

