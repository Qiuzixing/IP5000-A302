/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _AST_I2C_H_
#define _AST_I2C_H_
uint8_t I2C_ReadByte(uint8_t deviceID, uint8_t offset);
int I2C_WriteByte(uint8_t deviceID, uint8_t offset, uint8_t value);
uint8_t I2C_ReadBlock(uint8_t deviceID, uint8_t offset, uint8_t *buffer, uint16_t length);
uint8_t I2C_WriteBlock(uint8_t deviceID, uint8_t offset, uint8_t *buffer, uint16_t length);
uint8_t I2C_ReadBlockAndDiscard(uint8_t deviceID, uint8_t offset, uint16_t length);
uint8_t I2C_ReadSegmentBlock(uint8_t deviceID, uint8_t segment, uint8_t offset, uint8_t *buffer, uint16_t length);
#endif
