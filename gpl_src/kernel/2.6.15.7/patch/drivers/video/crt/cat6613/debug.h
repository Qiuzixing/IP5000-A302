///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >debug.h<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/
#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef DEBUG
#ifdef CONFIG_AST1500_CAT6613
#if	defined(Linux_KernelMode)
#define HDMITX_DEBUG_PRINTF(x...) printk(KERN_ERR "cat6613: " x)
#define DUMPTX_DEBUG_PRINT(x...) printk(KERN_ERR "cat6613: " x)
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "cat6613: ***ERROR*** " fmt , ## args);	\
	} while(0)

#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "cat6613: " fmt , ## args);	\
	} while(0)

#else
#define HDMITX_DEBUG_PRINTF(x...) printf("cat6613: " x)
#define DUMPTX_DEBUG_PRINT(x...) printf("cat6613: " x)
#endif
#else
#define HDMITX_DEBUG_PRINTF(x) printf x
#define DUMPTX_DEBUG_PRINT(x) printf x
#endif
#else
#ifdef CONFIG_AST1500_CAT6613
#define HDMITX_DEBUG_PRINTF(x...)
#define DUMPTX_DEBUG_PRINT(x...)
#else
#define HDMITX_DEBUG_PRINTF(x)
#define DUMPTX_DEBUG_PRINT(x)
#endif
#endif

#endif//  _DEBUG_H_
