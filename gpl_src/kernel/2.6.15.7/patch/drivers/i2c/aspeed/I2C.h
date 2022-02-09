
#ifndef __I2C_H__
#define __I2C_H__

#include <linux/types.h>
void I2CInit(u32, unsigned int);
int  SetI2CReg(u32, u32, u32, u8);
/* Will read back value and retry if write failed. */
int  SetI2CRegRetry(u32, u32, u32, u8);
int  GetI2CReg(u32, u32, u32, u8 *);
int IsI2CReady(u32 DeviceSelect, u32 DeviceAddress);
int IsI2CReadyLite(u32 DeviceSelect, u32 DeviceAddress);
int WaitI2CRdy(u32 DeviceSelect, u32 DeviceAddress, u32 ms);
int  SetI2CWord(u32, u32, u32, u16);
int  GetI2CWord(u32, u32, u32, u16 *);
int SetI2CBlock(u32, u32, u32, const u8 *, u32);
int GetI2CBlock(u32, u32, u32, u8 *, u32);
#endif
