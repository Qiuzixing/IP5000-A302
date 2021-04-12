/*
 * Copyright (c) 2019 ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/sched.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>

#include "typedef.h"

u8 i2c_read_byte(u8 address, u8 offset, u8 byteno, u8 *pdata, u8 device)
{
	int i;

	for (i = 0; i < byteno; i++) {
		if (GetI2CReg(I2C_HDMI_RX, address, (offset + i), pdata + i) != 0) {
			yield();
			return 0;
		}
		yield();
	}
	yield();
	return 1 ;
}

u8 i2c_write_byte(u8 address, u8 offset, u8 byteno, u8 *pdata, u8 device)
{
	int i;

	for (i = 0; i < byteno; i++) {
		if (SetI2CReg(I2C_HDMI_RX, address, (offset + i), *(pdata + i))) {
			yield();
			return 0;
		}
		yield();
	}
	yield();
	return 1 ;
}
