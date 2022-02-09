#ifndef _ASPEED_FEATURES_
#define _ASPEED_FEATURES_

#include <asm/arch/drivers/board_def.h>
#include <aspeed/features_video.h>

#if (CONFIG_AST1500_SOC_VER >= 2)
#define SUPPORT_HDCP_REPEATER	1
#else
#define SUPPORT_HDCP_REPEATER	1
#endif
#if SUPPORT_HDCP_REPEATER
#ifdef CONFIG_ARCH_AST1500_HOST
/*
** Bruce150701. Buggy (RctBug#2015062301). Don't use it.
** Bruce150915. 4K loopback (SiI9678) need VideoDev.Hdcp1_repeater_downstream_port_context.retry
** work to do re-try and init HDCP. We can't just disable HDCP_AUTHENTICATE_LOOPBACK.
*/
#define HDCP_AUTHENTICATE_LOOPBACK	1
#define HDCP1_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT	0
#else
#define HDCP_AUTHENTICATE_LOOPBACK	0
#define HDCP1_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT	1
#endif
#define HDCP2_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT	1
#define HDCP2_Encryption	1
#else//#if SUPPORT_HDCP_REPEATER
#define HDCP_AUTHENTICATE_LOOPBACK	0
#define HDCP1_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT	0
#define HDCP2_STRICTLY_TRANSMIT_LOW_VALUE_CONTENT	0
#define HDCP2_Encryption	0
#endif

#endif
