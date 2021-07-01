#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "command_protocol.h"
#include "ipx5000_a30_cmd.h"

void Example_thread_Handler(void);
void Example_CommandInterface_Init();
int Example_HeartBeatStatue(void);
// void Example_CommandInterfaceRead(unsigned char data);

#endif
