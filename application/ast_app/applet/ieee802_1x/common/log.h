
#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#define GBLog_Log(level, format, args...) printf(format"\n", ##args)


#endif // __LOG_H__

