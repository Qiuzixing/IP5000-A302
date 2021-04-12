/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#include <linux/sched.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>

#include "typedef.h"
#ifdef Enable_IT6802_CEC
#include "Mcu.h" /* for CEC_ADDR */
#endif

BOOL i2c_read_byte(BYTE address, BYTE offset, BYTE byteno, BYTE *p_data, BYTE device)
{
	int i;

	for (i = 0; i < byteno; i++) {
		if (GetI2CReg(I2C_HDMI_RX, address, (offset + i), p_data + i) != 0) {
			yield();
			return 0;
		}
		yield();
	}
	yield();
	return 1 ;
}

BOOL i2c_write_byte(BYTE address, BYTE offset, BYTE byteno, BYTE *p_data, BYTE device)
{
	int i;

	for (i = 0; i < byteno; i++) {
		if (SetI2CReg(I2C_HDMI_RX, address, (offset + i), *(p_data + i))) {
			yield();
			return 0;
		}
		yield();
	}
	yield();
	return 1 ;
}

#ifdef Enable_IT6802_CEC
BYTE IT6802_CEC_ReadI2C_Byte(BYTE RegAddr)
{
	BYTE p_data;
	BOOL FLAG;

	FLAG = i2c_read_byte(CEC_ADDR, RegAddr, 1, &p_data, IT6802CECGPIOid);

	if (FLAG == 0) {
		CEC_DEBUG_PRINTF(("IT6802_CEC I2C ERROR !!!"));
		CEC_DEBUG_PRINTF(("=====  Read Reg0x%X=\n", RegAddr));

	}

	return p_data;
}


SYS_STATUS IT6802_CEC_WriteI2C_Byte(BYTE offset, BYTE buffer)
{
	BOOL  flag;

	flag = i2c_write_byte(CEC_ADDR, offset, 1, &buffer, IT6802CECGPIOid);

	return !flag;
}
#endif
