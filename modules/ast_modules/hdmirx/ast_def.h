#ifndef _HDMIRX_AST_DEF_H_
#define _HDMIRX_AST_DEF_H_

#include <aspeed/features.h>

#define AST_HDMIRX//This flag is used to identify those code sections modified by aspeed. CONFIG_AST1500_CAT6023 might be used instead.

#define DEBUG 
//#define SUPPORT_REPEATER
#define Timer_Handle
//#define Timer_UpdateAudio
#define VRX_DBG 1
#define USE_INTERRUPT 0
#define NLPCM_THROUGH_I2S
#define AST_MODE_DETECT_PATCH	0

#define INFO(args...) printk(KERN_INFO "HDMIRX: " args); 
#define ErrorF(args...) printk(KERN_ERR "HDMIRX: " args); 

#endif
