#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

//////////////////////////////////////////////////
// data type
//////////////////////////////////////////////////
#ifdef _MCU_
typedef int BOOL ;
#define  code
#define _IDATA idata
#define _XDATA xdata
#else
typedef int BOOL ;
#define 
#define _IDATA
#define _XDATA
#endif // _MCU_



typedef char CHAR,*PCHAR ;
typedef unsigned char UCHAR,*PUCHAR ;
typedef unsigned char byte,*pbyte ;
typedef unsigned char BYTE,*PBYTE ;

typedef short SHORT,*PSHORT ;
typedef unsigned short USHORT,*PUSHORT ;
typedef unsigned short word,*pword ;
typedef unsigned short WORD,*PWORD ;

typedef long LONG,*PLONG ;
typedef unsigned long ULONG,*PULONG ;
typedef unsigned long dword,*pdword ;
typedef unsigned long DWORD,*PDWORD ;

#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define FAIL -1

#define ON 1
#define OFF 0

typedef enum _SYS_STATUS {
    ER_SUCCESS = 0,
    ER_FAIL,
    ER_RESERVED
} SYS_STATUS ;

#endif // _TYPEDEF_H_
