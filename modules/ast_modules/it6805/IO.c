///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <IO.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#include "config.h"
#include "MCU.h"
#include <stdio.h>
#include "IO.h"
#include "debug.h"

#define HIGH       	1
#define LOW        	0

void SetintActive(iTE_u1 bactive)
{
	bactive = 0;  //fake code
}


void set_8051_scl(iTE_u1 bit_value, iTE_u8 device)
{
	//     stall5us;
	switch (device)
	{
	case 0:
		DEV0_SCL_PORT = bit_value;
		break;
	case 1:
		DEV1_SCL_PORT = bit_value;
		break;
	case 2:
		DEV2_SCL_PORT = bit_value;
		break;
	case 3:
		DEV3_SCL_PORT = bit_value;
		break;
	case 4:
		DEV4_SCL_PORT = bit_value;
		break;
	default:
		DEV0_SCL_PORT = bit_value;
		break;
	}
	//     stall5us;
}

void set_8051_sda(iTE_u1 bit_value, iTE_u8 device)
{
	//     stall5us;
	switch (device)
	{
	case 0:
		DEV0_SDA_PORT = bit_value;
		break;
	case 1:
		DEV1_SDA_PORT = bit_value;
		break;
	case 2:
		DEV2_SDA_PORT = bit_value;
		break;
	case 3:
		DEV3_SDA_PORT = bit_value;
		break;
	case 4:
		DEV4_SDA_PORT = bit_value;
		break;

	default:
		DEV0_SDA_PORT = bit_value;
		break;
	}
	//     stall5us;
}

iTE_u1 get_8051_sda(iTE_u8 device)
{
	switch (device)
	{
	case 0:
		DEV0_SDA_PORT = 1;
		return DEV0_SDA_PORT;
		break;
	case 1:
		DEV1_SDA_PORT = 1;
		return DEV1_SDA_PORT;
		break;
	case 2:
		DEV2_SDA_PORT = 1;
		return DEV2_SDA_PORT;
		break;

	case 3:
		DEV3_SDA_PORT = 1;
		return DEV3_SDA_PORT;
		break;

	case 4:
		DEV4_SDA_PORT = 1;
		return DEV4_SDA_PORT;
		break;

	default:
		DEV0_SDA_PORT = 1;
		return DEV0_SDA_PORT;
		break;
	}
}

void i2c_8051_start(iTE_u8 device)
{
	set_8051_sda(HIGH, device);
	set_8051_scl(HIGH, device);
	set_8051_sda(LOW, device);
	set_8051_scl(LOW, device);
}

void i2c_8051_write(iTE_u8 byte_data, iTE_u8 device)
{
	iTE_u8 _DATA bit_cnt, tmp;
	iTE_u1 _DATA bit_value;

	for (bit_cnt = 0; bit_cnt<8; bit_cnt++) {
		tmp = (byte_data << bit_cnt) & 0x80;
		bit_value = tmp && 0x80;

		set_8051_sda(bit_value, device);
		set_8051_scl(HIGH, device);
		set_8051_scl(LOW, device);
	}
}

iTE_u1 i2c_8051_wait_ack(iTE_u8 device)
{
	iTE_u1 _DATA ack_bit_value;

	set_8051_sda(HIGH, device);
	set_8051_scl(HIGH, device);
	ack_bit_value = get_8051_sda(device);
	set_8051_scl(LOW, device);

	return ack_bit_value;
}

iTE_u8 i2c_8051_read(iTE_u8 device)
{
	iTE_u8 _DATA bit_cnt, byte_data;
	iTE_u1 _DATA bit_value;

	byte_data = 0;
	for (bit_cnt = 0; bit_cnt<8; bit_cnt++) {
		set_8051_scl(HIGH, device);

		bit_value = get_8051_sda(device);

		byte_data = (byte_data << 1) | bit_value;

		set_8051_scl(LOW, device);
	}

	return byte_data;
}

void i2c_8051_send_ack(iTE_u1 bit_value, iTE_u8 device)
{
	set_8051_sda(bit_value, device);
	set_8051_scl(HIGH, device);
	set_8051_scl(LOW, device);
	set_8051_sda(HIGH, device);
}

void i2c_8051_end(iTE_u8 device)
{
	set_8051_sda(LOW, device);
	set_8051_scl(HIGH, device);
	set_8051_sda(HIGH, device);
}

iTE_u1 i2c_write_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device)
{

	iTE_u8 _DATA i;



	i2c_8051_start(device);

	i2c_8051_write(address & 0xFE, device);
	if (i2c_8051_wait_ack(device) == 1)    {
		i2c_8051_end(device);
		return 0;
	}

	i2c_8051_write(offset, device);
	if (i2c_8051_wait_ack(device) == 1)    {
		i2c_8051_end(device);
		return 0;
	}

	for (i = 0; i<byteno - 1; i++) {
		i2c_8051_write(*p_data, device);
		if (i2c_8051_wait_ack(device) == 1) {
			i2c_8051_end(device);
			return 0;
		}
		p_data++;
	}

	i2c_8051_write(*p_data, device);
	if (i2c_8051_wait_ack(device) == 1)    {
		i2c_8051_end(device);
		return 0;
	}
	else {
		i2c_8051_end(device);
		return 1;
	}
}

iTE_u1 i2c_read_byte(iTE_u8 address, iTE_u8 offset, iTE_u8 byteno, iTE_u8 *p_data, iTE_u8 device)
{
	iTE_u8 _DATA i;

	i2c_8051_start(device);

	i2c_8051_write(address & 0xFE, device);
	if (i2c_8051_wait_ack(device) == 1) {
		i2c_8051_end(device);
		return 0;
	}

	i2c_8051_write(offset, device);
	if (i2c_8051_wait_ack(device) == 1) {
		i2c_8051_end(device);
		return 0;
	}

	i2c_8051_start(device);

	i2c_8051_write(address | 0x01, device);
	if (i2c_8051_wait_ack(device) == 1) {
		i2c_8051_end(device);
		return 0;
	}

	for (i = 0; i<byteno - 1; i++) {
		*p_data = i2c_8051_read(device);
		i2c_8051_send_ack(LOW, device);

		p_data++;
	}

	*p_data = i2c_8051_read(device);
	i2c_8051_send_ack(HIGH, device);
	i2c_8051_end(device);

	return 1;
}


