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
 * @file  Macros.h
 *
 * @brief Macros definitions.
 *
 *****************************************************************************
*/

#define ClearInterrupt(x)                   WriteByteTPI(PAGE_0_TPI_INTR_ST_ADDR, x)                   // write "1" to clear interrupt bit
