/* Copyright (c) 2004-2016
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef __ASTPARAM_H__
#define __ASTPARAM_H__

// RW and RO block uses the same size (64KB). We use 64KB as the boundary here.
#define ASTPARAM_BLOCK_SIZE (0x10000) //64KB
#define MIN_ASTPARAM_DATA_SIZE (6) // 4 header + 2 null end = 6 bytes

char *getParam(char *buffer, const char *name);
int setParam(char *buffer, unsigned int max_buf_size, const char *name, const char *value);
void dumpParam(char *buffer, unsigned int max_buf_size);

char *dataFieldParam(char *buffer);
char *popParamKeyValue(char *param, char **p_key_value);
int hasParamKeyValue(char *buffer, const char *key_value_pair);

#endif /* #define __ASTPARAM_H__ */

