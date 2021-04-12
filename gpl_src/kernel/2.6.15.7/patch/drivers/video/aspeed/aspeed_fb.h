/* Chip Info */
#ifndef PCI_VENDOR_ID_AST
#define	PCI_VENDOR_ID_AST		0x1A03
#endif

#ifndef PCI_DEVICE_ID_AST2000
#define	PCI_DEVICE_ID_AST2000		0x2000
#endif

#ifndef PCI_DEVICE_ID_AST2100
#define	PCI_DEVICE_ID_AST2100		0x3000
#endif

#ifndef FB_ACCEL_ASPEED
#define FB_ACCEL_ASPEED			0xc0		/* AST2000/3000 */
#endif

/* Version Info */
#define ASTFB_VERSION			"0.3.0"
#define ASTFB_MODULE_NAME		"astfb"
#define SUPPORTED_CHIPSETS		"AST3000"

/* Cmopiler Options */
#ifndef DEBUG
#define DEBUG				0
#endif
//#define FPGA				0		/* FPGA EVB */
#define InitVGA				1		/* Init VGA in FB */
#define Enable2D			0		/* Enable 2D Acceleration */
#define EnableCMDQ			0		/* 0/1 : MMIO/CMDQ */
#define EnableBigEndian			0		/* AST2100 */

/* H/W Feature Definition */
#define DEFAULT_VIDEOMEM_SIZE		0x00800000
#if	defined(CONFIG_ARCH_AST3000FPGA_CLIENT)
#define DEFAULT_VIDEOMEM_BASE		0x43800000	/* 56MB */
#else
#define DEFAULT_VIDEOMEM_BASE		0x4F800000	/* 120MB */
#endif
#define DEFAULT_MMIO_SIZE		0x00020000
#define DEFAULT_CMDQ_SIZE		0x00100000
#define MIN_CMDQ_SIZE			0x00040000
#define CMD_QUEUE_GUARD_BAND		0x00000020
#define DEFAULT_HWC_NUM			0x00000002

/* Data Type Definition */
typedef u8	UCHAR;
typedef u16  	USHORT;
typedef u32  	ULONG;

/* get commonly used pointers */
#define GET_DINFO(info)		(struct astfb_info *)(info->par)
#define GET_DISP(info, con)	((con) < 0) ? (info)->disp : &fb_display[con]

/* data structure */
struct astfb_info {
	
	/* fb info */
	struct fb_info *info;
	struct device *dev;	
		
	struct fb_var_screeninfo var;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	struct fb_fix_screeninfo fix;
	u32 pseudo_palette[17];
#endif
	
	/* driver registered */
	int registered;	
		
        /* console control */	
	int currcon;
	
	/* chip info */
	char name[40];

	/* resource stuff */	
	unsigned long ulFBPhys;
	unsigned long ulFBSize;
	UCHAR *pjFBVirtual;
	UCHAR *pjFBVirtualReal;

	unsigned long ulMMIOPhys;
	unsigned long ulMMIOSize;	
	UCHAR *pjMMIOVirtual;
		
	/* Options */

        /* CMDQ */
	unsigned long ulCMDQSize;
	unsigned long ulCMDQOffset;
	unsigned long ul2DMode;
	 
	/* mode stuff */	
	int xres;
	int yres;
	int xres_virtual;
	int yres_virtual;	
	int bpp;
	int pixclock;
	int pitch;
	int refreshrate;	
	
	/* disp stuff */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)	
	struct display 		disp;
	struct display_switch 	dispsw;		
	struct { u8 red, green, blue, pad; } palette[256];
		
#if defined(FBCON_HAS_CFB16) || defined(FBCON_HAS_CFB32)
	union {
#if defined(FBCON_HAS_CFB16)
		u_int16_t cfb16[16];
#endif
#if defined(FBCON_HAS_CFB32)
		u_int32_t cfb32[16];
#endif
	} fbcon_cmap;
#endif
#endif

	struct s_crt_drv *crt;

};

/* include files */
#include "aspeed_hw.h"
#include "aspeed_vga.h"
#include "aspeed_2dtool.h"
#include "aspeed_ioctl.h"


