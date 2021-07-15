#ifndef __LDFW_H__
#define __LDFW_H__


#ifdef __cplusplus
extern "C"{
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"

int P3K_LDFWCmdProcess( int handleId,P3K_SimpleCmdInfo_S *cmdreq,P3K_SimpleCmdInfo_S *cmdresp);


#ifdef __cplusplus
}
#endif

#endif


