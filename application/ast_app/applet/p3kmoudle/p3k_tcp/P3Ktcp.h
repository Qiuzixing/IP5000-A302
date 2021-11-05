#ifndef __P3KTCP_H__
#define __P3KTCP_H__

#ifdef __cplusplus
    extern "C"{
#endif
#include "common.h"
#include "../p3klib/cfgparser.h"

int Tcp_NetInit(int port);
void Create_LinkList(ConnectionList_S *list);
void HeadInsert_LinkList(ConnectionList_S * list, Connection_Info *pnew);
void Print_LinkList(ConnectionList_S *list);
void Destroy_LinkList(ConnectionList_S *list);
void DeleteById_LinkList(ConnectionList_S *list, int socket);


#ifdef __cplusplus
}
#endif

#endif


