/*
 *****************************************************************************
 *
 * Copyright 2010, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1060
 * East Arques Avenue, Sunnyvale, California 94085
 *****************************************************************************
 */
/*
 *****************************************************************************
 * @file  si_cpCEC.h
 *
 * @brief CP 9387 Starter Kit CEC handler.
 *
 *****************************************************************************
*/

#ifndef _CECHANDLER_H_
#define _CECHANDLER_H_
//#include <si_datatypes.h>

//------------------------------------------------------------------------------
// API Function Templates
//------------------------------------------------------------------------------

uint8_t CpArcEnable( uint8_t mode );
void CpHecEnable( uint8_t enable );

char *CpCecTranslateLA( uint8_t bLogAddr );
char *CpCecTranslateOpcodeName( SI_CpiData_t *pMsg );
uint8_t CpCecPrintCommand( SI_CpiData_t *pMsg, uint8_t isTX );

uint8_t CpCecRxMsgHandler( SI_CpiData_t *pCpi );

#endif // _CECHANDLER_H_



