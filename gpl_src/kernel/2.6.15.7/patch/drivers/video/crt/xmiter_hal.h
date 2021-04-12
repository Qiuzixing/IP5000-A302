#ifndef __XMITER_HAL_H__
#define __XMITER_HAL_H__

#include <aspeed/features.h>

typedef enum
{
	xmiter_none = 0,
	xmiter_dummy,
	xmiter_internal_dac, //CRT1 analog
	xmiter_cat6613,
	xmiter_sii9136,
	xmiter_ch7301,
	xmiter_siI164, //DVI
} eXmiterChip;

typedef enum
{
	xCap_NONE = 0,
	xCap_RGB = 1,
	xCap_DVI = 2,
	xCap_HDMI = 4,
	xCap_EXTERNAL_HDCP = 8,
	xCap_CEC = 16,
} eXmiterCap;

typedef unsigned int (*PFN_xmiter_exist)(struct s_crt_drv *crt);
typedef unsigned int (*PFN_xmiter_setup)(struct s_crt_drv *crt, struct s_crt_info *info);
typedef unsigned int (*PFN_xmiter_setup_audio)(Audio_Info *audio_info);
typedef unsigned int (*PFN_xmiter_disable)(struct s_crt_drv *crt);
typedef void (*PFN_xmiter_disable_audio)(void);
typedef unsigned int (*PFN_xmiter_init)(struct s_crt_drv *crt);
typedef unsigned int (*PFN_check_hotplug)(struct s_crt_drv *crt);
typedef unsigned int (*PFN_xmiter_rd_edid_chksum)(struct s_crt_drv *crt, unsigned char *pChksum);
typedef unsigned int (*PFN_xmiter_rd_edid)(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid);
typedef unsigned int (*PFN_poll_hotplug)(struct s_crt_drv *crt);
#ifdef CONFIG_ARCH_AST1500_CLIENT
typedef unsigned int (*PFN_cec_send)(unsigned char *buf, unsigned int size);
typedef int (*PFN_cec_topology)(unsigned char *buf, u32 scan);
typedef void (*PFN_cec_pa_cfg)(unsigned short address);
typedef void (*PFN_cec_la_cfg)(unsigned char address);
#endif
#if SUPPORT_HDCP_REPEATER
typedef void (*PFN_HDCP1_Downstream_Port_Set_Mode)(unsigned char repeater);
typedef void (*PFN_HDCP1_Downstream_Port_Auth)(int enable);
typedef void (*PFN_HDCP1_Downstream_Port_Encrypt)(unsigned char enable);
#endif

struct s_xmiter_info {
	eXmiterChip chip;
	eXmiterCap cap;
	PFN_xmiter_exist exist;
	PFN_xmiter_setup setup;
	PFN_xmiter_setup_audio setup_audio;
	PFN_xmiter_disable disable;
	PFN_xmiter_disable_audio disable_audio;
	PFN_xmiter_init init;
	PFN_check_hotplug check_hotplug;
	PFN_xmiter_rd_edid_chksum rd_edid_chksum;
	PFN_xmiter_rd_edid rd_edid;
	//Use poll_hotplug() only when xmiter doesn't support hotplug event(crt_hotplug_tx())
	PFN_poll_hotplug poll_hotplug;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	PFN_cec_send cec_send;
	PFN_cec_topology cec_topology;
	PFN_cec_pa_cfg cec_pa_cfg;
	PFN_cec_la_cfg cec_la_cfg;
#endif
#if SUPPORT_HDCP_REPEATER
	PFN_HDCP1_Downstream_Port_Set_Mode Hdcp1_set_mode;
	PFN_HDCP1_Downstream_Port_Auth Hdcp1_auth;
	PFN_HDCP1_Downstream_Port_Encrypt Hdcp1_encrypt;
#endif
};


unsigned int xHal_xmiter_exist(struct s_crt_drv *crt);
unsigned int xHal_setup_xmiter(struct s_crt_drv *crt, struct s_crt_info *info);
unsigned int xHal_setup_audio_xmiter(struct s_crt_drv *crt);
unsigned int xHal_disable_xmiter(struct s_crt_drv *crt);
unsigned int xHal_xmiter_disable_audio(struct s_crt_drv *crt);
unsigned int xHal_init_xmiter(struct s_crt_drv *crt);
unsigned int xHal_check_hotplug(struct s_crt_drv *crt);
unsigned int xHal_poll_hotplug(struct s_crt_drv *crt);
unsigned int xHal_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid);
unsigned int xHal_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum);
unsigned int xHal_get_xmiter_cap(struct s_crt_drv *crt);
#if (3 <= CONFIG_AST1500_SOC_VER)
unsigned int xHal_get_xmiter_cap_secondary(struct s_crt_drv *crt);
#endif
unsigned int xHal_init_dual_port_xmiter(struct s_crt_drv *crt);
unsigned int _get_xmiter_idx(struct s_crt_drv *crt);
#if SUPPORT_HDCP_REPEATER
void xHal_hdcp1_set_mode(struct s_crt_drv *crt, u32 mode);
void xHal_hdcp1_auth(struct s_crt_drv *crt, u32 enable);
void xHal_hdcp1_encrypt(struct s_crt_drv *crt, u32 enable);
#endif
#if defined(CONFIG_ARCH_AST1500_CLIENT)
void xHal_cec_send(struct s_crt_drv *crt, u8 *buf, u32 size);
int xHal_cec_topology(struct s_crt_drv *crt, u8 *buf, u32 scan);
void xHal_cec_pa_cfg(struct s_crt_drv *crt, u16 address);
void xHal_cec_la_cfg(struct s_crt_drv *crt, u8 address);
#endif
#if 0
#define XIDX_CLIENT_CRT1 0
#define XIDX_CLIENT_CRT2 1
#else
#define XIDX_CLIENT_D 1
#define XIDX_CLIENT_A 0
#endif
#define XIDX_HOST_D 0 //Host loopback digital
#define XIDX_HOST_A 1 //Host loopback analog
#define NUM_XMITER 2

#endif

