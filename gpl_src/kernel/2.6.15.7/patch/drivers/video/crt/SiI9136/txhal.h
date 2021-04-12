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
 * @file  txhal.h
 *
 * @brief Function prototypes of the TXHAL.
 *
 *****************************************************************************
*/


void TXHAL_InitMicroGpios (void);
void TXHAL_InitPreReset (void);
void TXHAL_InitPostReset (void);
void TXHAL_ColorDepthConfig (void);
void TXHAL_Init (void);
void TXHAL_PowerStateD0(void);
void TXHAL_DisablePullUpForD3(void);
void TXHAL_PowerStateD3(void);
void TXHAL_EnableInterrupts(uint8_t Interrupt_Pattern);
void TXHAL_DisableInterrupts(void);
void TXHAL_OnMHLCableDisconnected(void);
void TXHAL_HBR_On (int);
void TXHAL_HBR_Off (void);
void TXHAL_Delays (void);
void TXHAL_InitCBusRegs(void);


