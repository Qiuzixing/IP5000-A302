/* Type Define */
#ifndef UCHAR
#define UCHAR  unsigned char
#endif

#ifndef	USHORT
#define USHORT unsigned short
#endif

#ifndef ULONG
#define ULONG unsigned long int
#endif

#ifndef LONG
#define LONG long int
#endif

#ifndef BOOL
#define BOOL  int
#endif

#ifndef TRUE
#define	TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef struct {
	
    USHORT usModeIndex;
    USHORT usWidth;
    USHORT usHeight;
    USHORT usBitPerPixel;
    USHORT usRefreshRate;
    
} _ModeInfo;

typedef struct {
	
    BOOL   EnableScaling;	
    USHORT usWidth;
    USHORT usHeight;
	
} _LCDInfo;

#include "vgahw.h"
