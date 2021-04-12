
#ifndef _UTIL_H_
#define _UTIL_H_

#include <asm/arch/hardware.h>

#define ReadMemoryLong(baseaddress,offset)        (*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))
#define WriteMemoryLong(baseaddress,offset,data)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset)))=(u32)(data)
#define WriteMemoryBYTE(baseaddress,offset,data)  *(volatile u8 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (u8)(data)    
#define WriteMemoryLongWithMASK(baseaddress, offset, data, mask)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (((*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))&(~(mask))) | (u32)((data)&(mask)))

#endif /* _UTIL_H_ */

