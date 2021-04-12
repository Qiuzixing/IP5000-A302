/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#ifndef _VIDEOIP_SYSFS_H_
#define _VIDEOIP_SYSFS_H_

#include <linux/sysfs.h>

#ifdef CONFIG_ARCH_AST1500_HOST
extern struct attribute_group host_attr_group;
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
extern struct attribute_group client_attr_group;
#endif

#endif /* #ifndef _VIDEOIP_SYSFS_H_ */

