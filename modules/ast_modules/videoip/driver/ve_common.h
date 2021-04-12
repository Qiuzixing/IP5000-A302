/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _VE_COMMON_H_
#define _VE_COMMON_H_

void PrintVideoReg(u32 inFrom, u32 inTo);
size_t PrintRegSys(u32 inFrom, u32 inTo, char *buf);

u32 UnlockVideoReg(u32 Key);
void EnableVideoInterrupt(u32 value);
void DisableVideoInterrupt(u32 value);
void ClearVideoInterrupt(u32 value);
u32 ReadVideoInterrupt(void);
u32 ReadVideoInterruptMask(void);
void setStreamBufferSize(void);

void scu_change_ability_per_efuse(void);

#endif /* #ifndef _VE_COMMON_H_ */
