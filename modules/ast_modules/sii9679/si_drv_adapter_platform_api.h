/******************************************************************************
 *
 * Copyright 2013, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of
 * Silicon Image, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file
 *
 * @brief Minimum Platform API function set for SII9679 Driver
 *
 *****************************************************************************/

#ifndef SI_DRV_ADAPTER_PLATFORM_API_H
#define SI_DRV_ADAPTER_PLATFORM_API_H

#ifdef __cplusplus
extern "C"{
#endif

/***** #include statements ***************************************************/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>

//#include <stdio.h>
#include "si_datatypes.h"
#include "si_drv_adapter.h"
/***** public macro definitions **********************************************/

#define AST_HDMITX 1

#if AST_HDMITX
//#define SiiInst_t (void *)
#define SII_ENV_BUILD_LOG_PRINT 0
#define SII_ENV_BUILD_ASSERT 0
#endif

#ifndef SII_ENV_BUILD_ASSERT
/**
* @brief Build-time definition to chose whether or nor to use assertion mechanism
*
* Add "-DSII_ENV_BUILD_ASSERT=0" to the compiler invocation command line to
* remove @ref SII_OS_DEBUG_ASSERT() code from the build.
*
* Add "-DSII_ENV_BUILD_ASSERT=1" to the compiler invocation command line to
* include @ref SII_OS_DEBUG_ASSERT() code and check the assertion conditions.
*
*/
#define SII_ENV_BUILD_ASSERT 1
#endif // SII_ENV_BUILD_ASSERT

#ifndef SII_ENV_BUILD_LOG_PRINT
/**
* @brief Build-time definition to chose whether or not to print log messages
*
* Add "-DSII_ENV_BUILD_LOG_PRINT=0" to the compiler invocation command line to
* remove log printing.
*
* Add "-DSII_ENV_BUILD_LOG_PRINT=1" to the compiler invocation command line to
* enable log printing.
*
*/
#define SII_ENV_BUILD_LOG_PRINT 1
#endif // SII_ENV_BUILD_LOG_PRINT


#ifndef SII_OS_DEBUG_ASSERT
#if(SII_ENV_BUILD_ASSERT)
/**
* @brief Assertion macro to check internal error conditions
*/
#  define SII_OS_DEBUG_ASSERT( expr ) \
      ( (void)( (/*lint -e{506}*/!(expr)) ? SiiOsDebugAssert(__FILE__, __LINE__, (uint32_t)(expr), NULL),((void)1) : ((void)0) ) )
#else // SII_OS_DEBUG_ASSERT
/**
* @brief Dummy assertion macro
*/
#  define SII_OS_DEBUG_ASSERT( expr ) ( (void)0 )
#endif // SII_OS_DEBUG_ASSERT
#endif // SII_OS_DEBUG_ASSERT

#ifndef SII_MEMCPY
/**
* @brief Macro to customize \c memcpy() function
*
* Add "-DSII_MEMCPY=my_memcpy" to replace all \c memcpy() calls with \c my_memcpy() calls.
* This can be useful in systems where standard \c string.h library is prohibited for using.
*
*/
#define  SII_MEMCPY(pdes, psrc, size)     memcpy(pdes, psrc, size)
#endif // SII_MEMCPY

#ifndef SII_MEMCMP
/**
* @brief Macro to customize \c memcmp() function
*
* Add "-DSII_MEMCMP=my_memcmp" to replace all \c memcmp() calls with \c my_memcmp() calls.
* This can be useful in systems where standard \c string.h library is prohibited for using.
*
*/
#  define  SII_MEMCMP(pdes, psrc, size)     memcmp(pdes, psrc, size)
#endif // SII_MEMCMP

#ifndef SII_MEMSET
/**
* @brief Macro to customize \c memset() function
*
* Add "-DSII_MEMSET=my_memset" to replace all \c memset() calls with \c my_memset() calls.
* This can be useful in systems where standard \c string.h library is prohibited from using.
*
*/
#  define  SII_MEMSET(pdes, value, size)    memset(pdes, value, size)
#endif // SII_MEMSET


#ifndef MSG_ALWAYS
// Debug print message level
/*****************************************************************************/
/** @defgroup SII_LOG_MSG_VAL Log channel selection
* @brief Log channel selection
*
* Used to select log channel from DEBUG_PRINT() macros.
*
******************************************************************************/
/* @{ */
#define MSG_ALWAYS              0x00 //!< Log channel with important messages
#define MSG_ERR                 0x01 //!< Error log channel
//#define MSG_STAT                0x02 //!< Middle importance statistic log channel
#define MSG_DBG                 0x03 //!< Verbose debug messages log channel

#define MSG_LEVEL               0x03
/* @} */
#endif // MSG_ALWAYS

#ifndef SII_MI2C_TEN
/*****************************************************************************/
/** @defgroup SII_I2C_TRANSACTION_BIT_MASK I2C transaction bit mask
* @brief I2C transaction bit mask
*
* Specifies type of I2C transaction in \c cmdFlags member of @ref SiiI2cMsg_t
* used in a parameter of @ref SiiHalMasterI2cTransfer() function.
*
* @see SiiI2cMsg_t
* @see SiiHalMasterI2cTransfer()
*
******************************************************************************/
/* @{ */
#define SII_MI2C_TEN        0x0010  //!< Set for ten bit chip address; cleared otherwise
#define SII_MI2C_RD         0x0001  //!< Set for read data operation; cleared for write operation
#define SII_MI2C_WR         0x0000  //!< Absent of read == write
/* @} */
#endif // SII_MI2C_TEN


extern unsigned int g_debugLineNo;
extern char *g_debugFileName;

#if (SII_ENV_BUILD_LOG_PRINT != 0)
#  ifndef DEBUG_PRINT
/**
* @brief Macro for log messages printing
*/
#ifndef WIN32
#if AST_HDMITX
#define DEBUG_PRINT(l,x)  if(l <= MSG_LEVEL) {g_debugLineNo = __LINE__; g_debugFileName = __FILE__; SiiOsDebugPrintFormatDecorations(); printk x;}
#else
#define DEBUG_PRINT(l,x)  if(l <= MSG_LEVEL) {g_debugLineNo = __LINE__; g_debugFileName = __FILE__; SiiOsDebugPrintFormatDecorations(); printf x;}
#endif
#else
#define DEBUG_PRINT(l,x)  {g_debugLineNo = __LINE__; g_debugFileName = __FILE__; SiiOsDebugPrintFormatDecorations(); printf x;}
// #define PRINTF(x)  { printf x; }
#endif
#  endif // DEBUG_PRINT
#else // SII_ENV_BUILD_LOG_PRINT
#  undef DEBUG_PRINT
/**
* @brief Dummy log print macro
*/
#  define DEBUG_PRINT(...)
#endif // SII_ENV_BUILD_LOG_PRINT


/***** public type definitions ***********************************************/

/**
* @brief Platform layer functions error code
*/
typedef enum
{
    SII_OS_STATUS_SUCCESS,           //!< No error
    SII_OS_STATUS_ERR_INVALID_PARAM, //!< One of parameters of called OSAL function is invalid
    SII_OS_STATUS_ERR_NOT_AVAIL,     //!< OSAL function does not support requested operation
    SII_OS_STATUS_ERR_FAILED         //!< Other error types
} SiiOsStatus_t;


typedef enum
{
    SII_OS_DEBUG_FORMAT_SIMPLE    = 0x0000u,
    SII_OS_DEBUG_FORMAT_FILEINFO  = 0x0001u,
    SII_OS_DEBUG_FORMAT_TIMESTAMP = 0x0004u
} SiiOsDebugFormat_e;

/**
* @brief I2C transaction details for @ref SiiHalMasterI2cTransfer() function
*
* Similar to Linux's \c struct \c i2c_msg structure.
*/
typedef struct
{
    uint16_t addr;      //!< Slave I2C address
    uint16_t cmdFlags;  //!< Flags defining message actions
    uint16_t len;       //!< Buffer length in bytes
    uint8_t *pBuf;      //!< Pointer to input (for write operations) or
                        //!< Pointer to output (for read operations) buffer
} SiiI2cMsg_t;


/***** public functions ******************************************************/

/*****************************************************************************/
/**
* @brief I2C transaction request
*
* The function is similar to Linux's \c i2c_transfer():
* @code
* int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msg, int num);
* @endcode
*
* @param[in] busId   Selects I2C bus
* @param[in] pMsgs   Pointer to array of @ref SiiI2cMsg_t structures
* @param[in] msgNum  Number of @ref SiiI2cMsg_t structures
*
* Typical write operation is described by one SiiI2cMsg_t structure.
* \c pMsgs->pBuf points to a buffer where the first byte
* specifies Silicon Image's register and the rest is the block of data
* to write.
* \c pMsgs->cmdFlags does not have @ref SII_MI2C_RD bit set
* to mark the write transaction.
*
* Typical read operation is described by two SiiI2cMsg_t structures.
* The first \c pMsgs->pBuf points to a buffer where the first byte
* specifies Silicon Image's register.
* \c pMsgs->cmdFlags does not have @ref SII_MI2C_RD bit set
* to mark the write transaction.
* The second \c pMsgs->pBuf points to an input buffer where read data is to be copied over.
* \c pMsgs->cmdFlags has @ref SII_MI2C_RD bit set to mark the read transaction.
*
* @retval SII_OS_STATUS_SUCCESS     No error
* @retval SII_OS_STATUS_ERR_FAILED  Error occurred during the I2C operation
*
* @see SII_I2C_TRANSACTION_BIT_MASK
*
*****************************************************************************/
SiiOsStatus_t SiiHalMasterI2cTransfer (int busId, SiiI2cMsg_t *pMsgs, uint8_t msgNum);

/*****************************************************************************/
/**
* @brief Time delay
*
* Does nothing for specified number of microseconds.
*
* @note Although the time unit is microseconds, it is not required
*       provide the resolution up to the microseconds.
*       The minimum required resolution is 10 ms.
*
* @param[in] timeUsec Delay in microseconds
*
* @retval SII_OS_STATUS_SUCCESS Always return @ref SII_OS_STATUS_SUCCESS
*
*****************************************************************************/
SiiOsStatus_t SiiOsTaskSleepUsec(uint32_t timeUsec);

#if (SII_ENV_BUILD_ASSERT != 0)
// Assertion handler
// Not needed if SII_ENV_BUILD_ASSERT is set to 0
/*****************************************************************************/
/**
* @brief Debug assertion handler
*
* This function is called when the @ref SII_OS_DEBUG_ASSERT() macro is called
* and the expression in the parameter is \c 0.
*
* @param[in] pFileName    String with the file name where the assertion happened
* @param[in] lineNumber   String with the line number where the assertion happened
* @param[in] expressionEvaluation  Reserved for the future; do not use
* @param[in] pConditionText        Reserved for the future; do not use
*
* @see SII_OS_DEBUG_ASSERT()
* @see SII_ENV_BUILD_ASSERT
*
*****************************************************************************/
void SiiOsDebugAssert (const char *pFileName, uint32_t lineNumber, uint32_t expressionEvaluation, const char *pConditionText);
#endif // SII_ENV_BUILD_ASSERT

#if (SII_ENV_BUILD_LOG_PRINT != 0)
/*****************************************************************************/
/**
* @brief Log printing
*
* Print given message into a log (e.g. on a terminal).
* The messages can be filtered (some of them are printed and some not)
* depending on \c printFlags parameter.
*
* Not needed if @ref SII_ENV_BUILD_LOG_PRINT is set to \c 0.
*
* @param[in] printFlags @ref SII_LOG_MSG_VAL "Log channel" selection
* @param[in] ...        \c printf() - like parameters
*
* @see SII_LOG_MSG_VAL
* @see DEBUG_PRINT()
* @see SII_ENV_BUILD_LOG_PRINT
*
*****************************************************************************/
void SiiPlatformLoggerPrintf (int printFlags, ...);

void SiiOsDebugPrintFormatDecorations(void);
#endif // SII_ENV_BUILD_LOG_PRINT

#ifdef __cplusplus
}
#endif

#endif // SI_DRV_ADAPTER_PLATFORM_API_H


