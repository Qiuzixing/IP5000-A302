/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _GETINF_H_
#define _GETINF_H_

#if 0//static
BOOL  GetGeneralINFData (char *filename, PGENERAL_INFO pGeneralInfo);
BOOL  GetVIDEOINFData (char *filename, PVIDEO_INFO pVideoInfo);
BOOL  GetVIDEOMINFData (char *filename, PVIDEOM_INFO pVideoInfo);
#endif
void LoadInf(PENGINE_CONFIG pEngConfig);
void LoadInf_V2(PENGINE_CONFIG_V2 pEngConfig);

#endif

