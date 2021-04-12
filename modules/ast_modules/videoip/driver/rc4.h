#ifndef _RC4_H_
#define _RC4_H_

#if 0
/*  RC4 structure */
typedef struct _rc4_state
{
    int x;
    int y;
    int m[256];
} rc4_state;
#endif

void EnableRC4Encryption(u8 CompressMode, u8 IsAutoMode, u32 *key);

#endif /* #ifndef _RC4_H_ */

