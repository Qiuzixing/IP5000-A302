/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifdef ASTHOST_GLOBALS
#define ASTHOST_EXT
#else
#define ASTHOST_EXT extern
#endif

ASTHOST_EXT int videoip_use_stderr;
ASTHOST_EXT int videoip_use_syslog;
ASTHOST_EXT int videoip_use_debug;

#define BASE_DIR "/usr/local/bin/"

#endif /* _GLOBAL_H_ */
