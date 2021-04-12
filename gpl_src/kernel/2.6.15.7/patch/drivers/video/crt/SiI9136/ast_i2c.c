/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <linux/module.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include "ast_utilities.h"
#include "i2c_slave_addrs.h"

uint8_t I2C_ReadByte(uint8_t deviceID, uint8_t offset)
{
	uint8_t value = 0;

	GetI2CReg(I2C_HDMI_TX, deviceID, offset, &value);

	return value;
}

int I2C_WriteByte(uint8_t deviceID, uint8_t offset, uint8_t value)
{
	return SetI2CReg(I2C_HDMI_TX, deviceID, offset, value);
}

uint8_t I2C_ReadBlock(uint8_t deviceID, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	int i;

#if (3 <= CONFIG_AST1500_SOC_VER)
	/* FIXME workaround, block access for sii9136 CEC doesn't work */
	if (deviceID == SA_TX_CPI_Primary)
		goto byte_access;

	if (0 != GetI2CBlock(I2C_HDMI_TX, deviceID, offset, buffer, length))
		goto byte_access;

	return 0;

byte_access:
#endif
	for (i = 0; i < length; i++) {
		if (GetI2CReg(I2C_HDMI_TX, deviceID, (offset + i), buffer + i) != 0) {
			yield();
			return 1;
		}
		yield();
	}
	yield();

	return 0; /* means success */
}

uint8_t I2C_WriteBlock(uint8_t deviceID, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	int i;
#if (3 <= CONFIG_AST1500_SOC_VER)
	/* FIXME workaround, block access for sii9136 CEC doesn't work */
	if (deviceID == SA_TX_CPI_Primary)
		goto byte_access;

	if (SetI2CBlock(I2C_HDMI_TX, deviceID, offset, buffer, length) != 0)
		goto byte_access;

	return 0;

byte_access:
#endif

	for (i = 0; i < length; i++) {
		if (SetI2CReg(I2C_HDMI_TX, deviceID, (offset + i), *(buffer + i))) {
			yield();
			return 1;
		}
		yield();
	}
	yield();
	return 0;
}

uint8_t I2C_ReadBlockAndDiscard(uint8_t deviceID, uint8_t offset, uint16_t length)
{
	int i;
	uint8_t buffer[1]; //A 1 u8 size dummy buffer.

	for (i = 0; i < length; i++) {
		if (GetI2CReg(I2C_HDMI_TX, deviceID, (offset + i), buffer) != 0) {
			yield();
			return 1; //means fail
		}
		yield();
	}
	yield();
	return 0; //means success
}

uint8_t I2C_ReadSegmentBlock(uint8_t deviceID, uint8_t segment, uint8_t offset, uint8_t *buffer, uint16_t length)
{
	/* From: Wiki::DDC
	** Earlier DDC implementations used simple 8-bit data offset when
	** communicating with the EDID memory in the monitor, limiting the
	** storage size to 28 bytes = 256 bytes, but allowing the use of cheap
	** 2-Kbit EEPROMs. In E-DDC, a special I2C addressing scheme was introduced,
	** in which multiple 256-byte segments could be selected. To do this,
	** a single 8-bit segment index is passed to the display via the I2C address
	** 30h. (Because this access is always a write, the first I2C octet will
	** always be 60h.). Data from the selected segment is then immediately read
	** via the regular DDC2 address using a repeated I2C 'START' signal.
	** However, VESA specification defines the segment index value range as
	** 00h to 7Fh, so this only allows addressing 128 segments * 256 bytes = 32KB.
	** The segment index register is volatile, defaulting to zero and
	** automatically resetting to zero after each NACK or STOP. Therefore,
	** it must be set every time access to data above the first 256-byte segment
	** is performed. The auto-reset mechanism is to provide for backward
	** compatibility to, for example, DDC2B hosts, otherwise they may be stuck
	** at a segment other than 00h in some rare cases.
	*/
	uerr("Don't know how to handle it?!\n");
	//BruceToDo. New I2C driver interface to read segment block.
	return 0;
}
