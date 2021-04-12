#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#include <linux/moduleparam.h>
#endif
#include <linux/kernel.h>
#include <linux/smp_lock.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/fb.h>
#include <linux/selection.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/vmalloc.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <linux/vt_kern.h>
#endif
#include <linux/capability.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#ifdef CONFIG_MTRR
#include <asm/mtrr.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <video/fbcon.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>
#include <video/fbcon-cfb24.h>
#include <video/fbcon-cfb32.h>
#endif

#include <linux/platform_device.h>
#include <asm/arch/platform.h>
#include <asm/arch/hardware.h>
#include <asm/arch/drivers/board_def.h>

#include "aspeed_fb.h"
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/vbuf.h>

/* external reference fucntion */
extern int astfb_getchipinfo(struct device *dev, struct astfb_info *dinfo);
extern int astfb_initchip(struct device *dev, struct astfb_info *dinfo);
extern int astfb_pan_display_var(struct fb_var_screeninfo *var, int con, struct astfb_info *dinfo);
extern int astfb_blank_var(int blank, struct astfb_info *dinfo);
extern int astfb_crtc_to_var(struct fb_var_screeninfo *var, struct astfb_info *dinfo);

extern int astfb_setmode(struct fb_var_screeninfo *var, struct astfb_info *dinfo, USHORT usWhichOne);
extern int astfb_getmodeinfo(struct fb_var_screeninfo *var, struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo);
extern int astfb_SearchOPTModeInfo(char *mode, struct astfb_info *dinfo);

/* FBD interface */
/* init & setup */
#ifndef	MODULE
int
astfb_setup(char *options);
#endif
int
astfb_init(void);

/* register & unregister */
static int astfb_drv_probe(struct device *dev);
static int astfb_drv_remove(struct device *dev);
int astfb_drv_suspend(struct device *dev, u32 state, u32 level);
int astfb_drv_resume(struct device *dev, u32 level);

static void cleanup(struct astfb_info *dinfo);
static int __devinit astfb_set_fbinfo(struct astfb_info *dinfo);

/* fops */
static int
astfb_get_fix(struct fb_fix_screeninfo *fix, int con, struct fb_info *info);
static int
astfb_pan_display(struct fb_var_screeninfo *var, struct fb_info* info);
static int
astfb_blank(int blank, struct fb_info *info);
static int
astfb_get_cmap_len(struct fb_var_screeninfo *var);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
static int
astfb_open(struct fb_info *info, int user);
static int
astfb_release(struct fb_info *info, int user);
static int
astfb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
		unsigned transp, struct fb_info *info);
static int
astfb_set_par(struct fb_info *info);
static int
astfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
static int
astfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg);
#else
static int
astfb_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
            unsigned long arg, struct fb_info *info);
#endif
#else /* 2.4.x */
static int
astfb_switch(int con, struct fb_info *info);
static int
astfb_updatevar(int con, struct fb_info *info);

static int
astfb_get_var(struct fb_var_screeninfo *var, int con, struct fb_info *info);
static int
astfb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info);
static void
astfb_set_disp(struct fb_var_screeninfo *var, int con, struct fb_info *info);
static int
astfb_setcolreg(unsigned regno, unsigned red, unsigned green,
		unsigned blue, unsigned transp, struct fb_info *info);
static int
astfb_getcolreg(unsigned regno, unsigned *red, unsigned *green,
		unsigned *blue, unsigned *transp, struct fb_info *info);
static void
astfb_do_install_cmap(int con, struct fb_info *info);
static int
astfb_get_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info);
static int
astfb_set_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info);
static int
astfb_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
            unsigned long arg, int con, struct fb_info *info);
#endif /* 2.4.x */

/* Global data */
static int num_registered = 0;
static int ypan = 0;
static unsigned char *mode=NULL;
static int rate=60;

static struct fb_var_screeninfo default_var = {
	.xres            = 0,
	.yres            = 0,
	.xres_virtual    = 0,
	.yres_virtual    = 0,
	.xoffset         = 0,
	.yoffset         = 0,
	.bits_per_pixel  = 0,
	.grayscale       = 0,
	.red             = {0, 8, 0},
	.green           = {0, 8, 0},
	.blue            = {0, 8, 0},
	.transp          = {0, 0, 0},
	.nonstd          = 0,
	.activate        = FB_ACTIVATE_NOW,
	.height          = -1,
	.width           = -1,
	.accel_flags     = 0,
	.pixclock        = 0,
	.left_margin     = 0,
	.right_margin    = 0,
	.upper_margin    = 0,
	.lower_margin    = 0,
	.hsync_len       = 0,
	.vsync_len       = 0,
	.sync            = 0,
	.vmode           = FB_VMODE_NONINTERLACED,
};

/*
static struct fb_fix_screeninfo default_fix = {
	.id	= "AST1000/2000",
	.type	= FB_TYPE_PACKED_PIXELS,
	.accel	= FB_ACCEL_NONE,
};
*/

/* driver ops */
static struct device_driver astfb_driver = {
	.name		= "astfb",
	.bus		= &platform_bus_type,

	.probe		= astfb_drv_probe,
        .remove		= astfb_drv_remove,
	.suspend	= astfb_drv_suspend,
        .resume		= astfb_drv_resume,
};

static struct platform_device astfb_device = {
	.name = "astfb",
};

/* fb ops */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
static struct fb_ops ast_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_open        = astfb_open,
	.fb_release     = astfb_release,
	.fb_check_var   = astfb_check_var,
	.fb_set_par     = astfb_set_par,
	.fb_setcolreg   = astfb_setcolreg,
        .fb_pan_display = astfb_pan_display,
        .fb_blank       = astfb_blank,
	.fb_fillrect    = cfb_fillrect,
	.fb_copyarea    = cfb_copyarea,
	.fb_imageblit   = cfb_imageblit,
        .fb_ioctl       = astfb_ioctl
};
#else /* 2.4.x */
static struct fb_ops ast_fb_ops = {
	.owner          = THIS_MODULE,
	.fb_get_fix     = astfb_get_fix,
	.fb_get_var     = astfb_get_var,
	.fb_set_var     = astfb_set_var,
	.fb_get_cmap    = astfb_get_cmap,
	.fb_set_cmap    = astfb_set_cmap,
	.fb_pan_display	= astfb_pan_display,
	.fb_ioctl       = astfb_ioctl
};
#endif

/*
 * Device Driver: Register & Un-Register
 */
#ifndef MODULE
int __init astfb_setup(char *options)
{
	char *this_opt;

	printk(KERN_DEBUG "astfb: astfb_setup is called \n");

    	/* parse options */
	while ((this_opt = strsep(&options, ","))) {
	    if (!*this_opt)
	        continue;

	    if (!strnicmp(this_opt, "mode:", 5)) {
                mode = this_opt + 5;
	    }
	    else if (!strnicmp(this_opt, "rate:", 5)) {
		rate = simple_strtoul(this_opt + 5, NULL, 0);
	    }

	}

    	return 0;

} /* astfb_setup */
#endif /* ifndef MODULE */

int __init astfb_init(void)
{
	int ret;

    	printk(KERN_DEBUG "astfb: astfb_init is called \n");

#ifndef MODULE
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,8)
	char *options = NULL;

	if(fb_get_options("astfb", &options))
		return -ENODEV;

	astfb_setup(options);
#endif
#else
        /* options */
        /* undo */
#endif

	ret = driver_register(&astfb_driver);

	if (!ret) {
		ret = platform_device_register(&astfb_device);
		if (ret)
			driver_unregister(&astfb_driver);
	}

	return ret;

} /* astfb_init */

module_init(astfb_init);


#ifdef MODULE
/* Module description/parameters */
MODULE_DESCRIPTION("Framebuffer driver for ASPEED (R)");
MODULE_AUTHOR("Y.C. Chen <yc_chen@aspeedtech.com>");
MODULE_LICENSE("GPL");

/* module parameters */
/* undo */

/* module init * exit */
/* integrate to astfb_init
int __init astfb_init_module(void)
{
    	printk(KERN_DEBUG "astfb: astfb_init_module is called \n");

    	return(astfb_init());
}
*/

static void __exit astfb_remove_module(void)
{
    	printk(KERN_DEBUG "astfb: astfb_remove_module is called \n");

	platform_device_unregister(&astfb_device);
	driver_unregister(&astfb_driver);
}

module_exit(astfb_remove_module);
#endif /* end of MODULE */

/*-------------------------------------------------------------------------*/

/*
 * Device Driver ops
 */
static int
astfb_drv_probe(struct device *dev)
{

	struct astfb_info *dinfo;
	struct fb_info *info;

    	printk(KERN_DEBUG "astfb: astfb_drv_probe is called \n");

	num_registered++;
	if (num_registered != 1) {
		printk(KERN_ERR "Attempted to register %d devices "
	               "(should be only 1).\n", num_registered);
		return -ENODEV;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,3)
	if (!(info = framebuffer_alloc(sizeof(struct astfb_info), dev))) {
		printk(KERN_ERR "Could not allocate memory for astfb_info.\n");
		return -ENODEV;
	}

	if (fb_alloc_cmap(&info->cmap, 256, 0) < 0) {
		printk(KERN_ERR "Could not allocate cmap for astfb_info.\n");
		framebuffer_release(info);
		return -ENODEV;
	}
#else
	if (!(info = kmalloc(sizeof(struct astfb_info), GFP_KERNEL))) {
		printk(KERN_ERR "Could not allocate memory for astfb_info.\n");
		return -ENODEV;
	}
#endif

	dinfo = (struct astfb_info *) info->par;
	memset(dinfo, 0, sizeof(*dinfo));
	dinfo->info  = info;
	dinfo->dev  = dev;
	strcpy(dinfo->name, "AST3000");

	dev_set_drvdata(dev, (void*)dinfo);

	/* Get the chip info */
	if (astfb_getchipinfo(dev, dinfo)) {
		cleanup(dinfo);
		return -ENODEV;
	}

	dinfo->crt = crt_connect(OWNER_CON);

	/* resource allocation */
#if 0
	dinfo->ulFBSize = DEFAULT_VIDEOMEM_SIZE;
	dinfo->ulFBPhys = DEFAULT_VIDEOMEM_BASE;

	dinfo->pjFBVirtual = (UCHAR *) ioremap(dinfo->ulFBPhys, dinfo->ulFBSize);
	if (!dinfo->pjFBVirtual) {
		printk(KERN_ERR "Cannot map FB.\n");
		cleanup(dinfo);
		return -ENODEV;
	}
#else
	dinfo->ulFBSize = CRT_CONSOLE_SIZE;
	dinfo->ulFBPhys = CRT_CONSOLE_PHY_BASE;

	dinfo->pjFBVirtual = (UCHAR *)TO_CRT_CONSOLE_BASE(dinfo->crt->vbuf_va);
#endif
	printk(KERN_INFO "FB Phys:%lx, Virtual:%lx \n", dinfo->ulFBPhys, (ULONG) dinfo->pjFBVirtual);

	dinfo->ulMMIOSize = DEFAULT_MMIO_SIZE;
	dinfo->ulMMIOPhys = ASPEED_2D_BASE;
	dinfo->pjMMIOVirtual = (UCHAR *) IO_ADDRESS(ASPEED_2D_BASE);

	printk(KERN_INFO "MMIO Phys:%lx, Virtual:%lx \n", dinfo->ulMMIOPhys, (ULONG) dinfo->pjMMIOVirtual);

#if EnableCMDQ
	dinfo->ulCMDQSize = DEFAULT_CMDQ_SIZE;
	dinfo->ulCMDQOffset = dinfo->ulFBSize - dinfo->ulCMDQSize;
	dinfo->ul2DMode = CMDQ_MODE;

	printk(KERN_INFO "CMDQ Base:%x, Size:%x \n", dinfo->ulCMDQOffset, dinfo->ulCMDQSize);
#else
	dinfo->ulCMDQSize = 0;
	dinfo->ulCMDQOffset = 0;
	dinfo->ul2DMode = MMIO_MODE;
#endif

        /* Init H/W setting for Drv */
	if (astfb_initchip(dev, dinfo)) {
		cleanup(dinfo);
		return -ENODEV;
	}

	/* currcon will be set by the first switch. */
	dinfo->currcon = -1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
        /* set default palette */
	for (i = 0; i < 16; i++) {
		j = color_table[i];
		dinfo->palette[i].red = default_red[j];
		dinfo->palette[i].green = default_grn[j];
		dinfo->palette[i].blue = default_blu[j];
	}
#endif

        /* set fb interface */
	memcpy(&dinfo->var, &default_var, sizeof(default_var));
	/* set default var mode */
	if (mode == NULL)
	{
		dinfo->var.xres = dinfo->var.xres_virtual = dinfo->crt->CON_mode.width;
		dinfo->var.yres = dinfo->var.yres_virtual = dinfo->crt->CON_mode.height;
		rate = dinfo->crt->CON_mode.refresh_rate;
		if (dinfo->crt->CON_mode.crt_color == XRGB8888_FORMAT)
			dinfo->var.bits_per_pixel  = 32;
		else
			dinfo->var.bits_per_pixel  = 16;
	}
	else /* opt */
	{
		if (astfb_SearchOPTModeInfo(mode, dinfo))
		{
			dinfo->var.xres = dinfo->var.xres_virtual = dinfo->crt->CON_mode.width;
			dinfo->var.yres = dinfo->var.yres_virtual = dinfo->crt->CON_mode.height;
			rate = dinfo->crt->CON_mode.refresh_rate;
			if (dinfo->crt->CON_mode.crt_color == XRGB8888_FORMAT)
				dinfo->var.bits_per_pixel  = 32;
			else
				dinfo->var.bits_per_pixel  = 16;
		}
	}

	if (!rate)
		rate = 60;
	else
		dinfo->refreshrate = rate;

	if (astfb_set_fbinfo(dinfo)) {
		cleanup(dinfo);
		return -ENODEV;
	}

	/* save init state */
	/* TODO */

	/* Register Framebuffer */
	if (register_framebuffer(info) < 0) {
		printk(KERN_ERR "Cannot register framebuffer.\n");
		cleanup(dinfo);
		return -ENODEV;
	}

	dinfo->registered = 1;

	return 0;

}

static int
astfb_drv_remove(struct device *dev)
{
	struct astfb_info *dinfo = dev_get_drvdata(dev);

    	printk(KERN_DEBUG "astfb: astfb_drv_remove is called \n");

	if (!dinfo)
            return 0;

        /* release resource */
	cleanup(dinfo);

	return 0;
}

int astfb_drv_suspend(struct device *dev, u32 state, u32 level)
{
	/* TODO */
	return 0;
}

int astfb_drv_resume(struct device *dev, u32 level)
{
	/* TODO */
	return 0;
}

static void
cleanup(struct astfb_info *dinfo)
{
	printk(KERN_DEBUG "astfb: cleanupis called \n");

	if (!dinfo)
		return;

        /* release resource */
#if	Enable2D
        astfb_vDisable2D(dinfo);
#endif

#if 0
        /* unmap resource */
	if (dinfo->pjFBVirtual)
		iounmap((UCHAR *)dinfo->pjFBVirtual);
#endif
	crt_disconnect(dinfo->crt, OWNER_CON);

	/* Unregister the framebuffer */
	if(dinfo->registered) {
		unregister_framebuffer(dinfo->info);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,3)
		framebuffer_release(dinfo->info);
#else
		kfree(dinfo);
#endif
        }

	dev_set_drvdata(dinfo->dev, NULL);

} /* cleanup */

static int __devinit
astfb_set_fbinfo(struct astfb_info *dinfo)
{
	struct fb_info *info;

	printk(KERN_INFO "astfb: astfb_set_fbinfo is called \n");

	info = dinfo->info;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,3)
	info->flags = FBINFO_FLAG_DEFAULT;
	info->fbops = &ast_fb_ops;
	info->var = dinfo->var;
	info->fix = dinfo->fix;
	info->pseudo_palette = dinfo->pseudo_palette;
        info->screen_base = dinfo->pjFBVirtual;

        /* update crtc to var */
        if (astfb_check_var(&info->var, info))
        {
	    printk(KERN_ERR "astfb: check default mode failed \n");
            return 1;
        }

        if (astfb_setmode(&info->var, dinfo, CRT1))
        {
	    printk(KERN_ERR "astfb: set default mode failed \n");
            return 1;
        }

        /* update fix */
	astfb_get_fix(&info->fix, -1, info);

#else   /* 2.4.x */
        /* set fb info */
	strcpy(info->modename, dinfo->name);
	info->node = -1;
	info->flags = FBINFO_FLAG_DEFAULT;
	info->fbops = &ast_fb_ops;
	info->disp = &dinfo->disp;
	info->changevar = NULL;
	info->switch_con = &astfb_switch;
	info->updatevar = &astfb_updatevar;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	info->blank = &astfb_blank;
#endif

        /* set default mode */
        /*
        astfb_setmode(&var, dinfo, CRT1);
        astfb_crtc_to_var(&var, dinfo);
        */

        /* set disp */
        astfb_set_disp(&var, -1, info);

#endif /* 2.4.x */

	return 0;

} /* astfb_set_fbinfo */


/*-------------------------------------------------------------------------*/


/*
 * fb_ops
 */
static int
astfb_get_fix(struct fb_fix_screeninfo *fix, int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk(KERN_DEBUG "astfb: astfb_get_fix is called \n");

	memset(fix, 0, sizeof(*fix));
	strcpy(fix->id, dinfo->name);
	fix->smem_start = dinfo->ulFBPhys;
	fix->smem_len = dinfo->ulFBSize;
	fix->type = FB_TYPE_PACKED_PIXELS;
	fix->type_aux = 0;
	fix->visual   = (dinfo->bpp == 8) ? FB_VISUAL_PSEUDOCOLOR : FB_VISUAL_TRUECOLOR;
	fix->xpanstep = 0;
	fix->ypanstep = ypan ? 1 : 0;;
	fix->ywrapstep = 0;
	fix->line_length = dinfo->pitch;
	fix->mmio_start = dinfo->ulMMIOPhys;
	fix->mmio_len = dinfo->ulMMIOSize;
	fix->accel = FB_ACCEL_NONE;

	return 0;

} /* astfb_get_fix */


/*
 * DPMS
 */
static int
astfb_blank(int blank, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk(KERN_DEBUG "astfb: astfb_blank is called \n");

	/* Bruce. I'm not going to support this feature. */
	return 0;

	astfb_blank_var(blank, dinfo);

	return 0;

} /* astfb_blank */

static int
astfb_pan_display(struct fb_var_screeninfo *var, struct fb_info* info)
{
	struct astfb_info *dinfo = GET_DINFO(info);
	u32 xoffset, yoffset;

	/*
	** Bruce. This is used when virtual desktop is bigger then real one.
	** I'm not going to support this feature.
	*/
	return 0;


    	xoffset = (var->xoffset+3) & ~3;		/* DW alignment */
    	yoffset = var->yoffset;

    	if((xoffset + var->xres) > var->xres_virtual) {
		return -EINVAL;
    	}
    	if((yoffset + var->yres) > var->yres_virtual) {
		return -EINVAL;
    	}

    	info->var.xoffset = xoffset;
    	info->var.yoffset = yoffset;

    	if (astfb_pan_display_var(var, -1, dinfo))
    	    return -EINVAL;

	return 0;
} /* astfb_pan_display */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
static int
astfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
#else
static int
astfb_ioctl(struct inode *inode, struct file *file,
            unsigned int cmd, unsigned long arg,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
	    int con,
#endif
	    struct fb_info *info)
#endif
{
	struct astfb_info *dinfo = GET_DINFO(info);
	struct astfb_dfbinfo dfbinfo;

	printk(KERN_DEBUG "astfb: astfb_ioctl is called \n");

	switch(cmd) {
	   case ASTFB_GET_DFBINFO:
	        dfbinfo.ulFBSize = dinfo->ulFBSize;
	        dfbinfo.ulFBPhys = dinfo->ulFBPhys;
	        dfbinfo.ulCMDQSize = dinfo->ulCMDQSize;
	        dfbinfo.ulCMDQOffset = dinfo->ulCMDQOffset;
	        dfbinfo.ul2DMode = dinfo->ul2DMode;

		if(copy_to_user((void __user *)arg, &dfbinfo, sizeof(struct astfb_dfbinfo)))
			return -EFAULT;
	        return 0;

           default:
	        return -EINVAL;
        }

	return 0;

} /* astfb_ioctl */

static int
astfb_get_cmap_len(struct fb_var_screeninfo *var)
{
	return (var->bits_per_pixel == 8) ? 256 : 16;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
static int
astfb_open(struct fb_info *info, int user)
{
	return 0;
}

static int
astfb_release(struct fb_info *info, int user)
{
	return 0;
}


static int
astfb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue,
		unsigned transp, struct fb_info *info)
{
	if(regno >= astfb_get_cmap_len(&info->var))
		return 1;

	switch(info->var.bits_per_pixel) {
	case 8:
		return 1;
		break;
	case 16:
		((u32 *)(info->pseudo_palette))[regno] =
				(red & 0xf800)          |
				((green & 0xfc00) >> 5) |
				((blue & 0xf800) >> 11);
		break;
	case 32:
		red >>= 8;
		green >>= 8;
		blue >>= 8;
		((u32 *)(info->pseudo_palette))[regno] =
				(red << 16) | (green << 8) | (blue);
		break;
	}
	return 0;
}


static int
astfb_set_par(struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk("astfb_set_par is called!!\n");

	if (astfb_setmode(&info->var, dinfo, CRT1))
	    return 1;

	astfb_get_fix(&info->fix, -1, info);

	return 0;
}

static int
astfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);
        VBIOS_MODE_INFO vgamodeinfo;

        if (astfb_getmodeinfo(var, dinfo, &vgamodeinfo))
        {
	    printk(KERN_ERR "astfb: check mode failed \n");
            return 1;
        }

	/* Sanity check for offsets */
	if(var->xoffset < 0) var->xoffset = 0;
	if(var->yoffset < 0) var->yoffset = 0;

	if(var->xres > var->xres_virtual)
		var->xres_virtual = var->xres;

	/* Truncate offsets to maximum if too high */
	if(var->xoffset > var->xres_virtual - var->xres) {
		var->xoffset = var->xres_virtual - var->xres - 1;
	}

	if(var->yoffset > var->yres_virtual - var->yres) {
		var->yoffset = var->yres_virtual - var->yres - 1;
	}

	switch(var->bits_per_pixel) {
	case 8:
		var->red.offset = var->green.offset = var->blue.offset = 0;
		var->red.length = var->green.length = var->blue.length = 6;
		break;
	case 16:
		var->red.offset = 11;
		var->red.length = 5;
		var->green.offset = 5;
		var->green.length = 6;
		var->blue.offset = 0;
		var->blue.length = 5;
		var->transp.offset = 0;
		var->transp.length = 0;
		break;
	case 32:
		var->red.offset = 16;
		var->red.length = 8;
		var->green.offset = 8;
		var->green.length = 8;
		var->blue.offset = 0;
		var->blue.length = 8;
		var->transp.offset = 24;
		var->transp.length = 8;
		break;
	}
	/* Set everything else to 0 */
	var->red.msb_right =
		var->green.msb_right =
		var->blue.msb_right =
		var->transp.offset =
		var->transp.length =
		var->transp.msb_right = 0;

	return 0;
}

#else /* 2.4.x */
static int
astfb_switch(int con, struct fb_info *info)
{

	struct astfb_info *dinfo = GET_DINFO(info);
	struct display *disp;

	printk(KERN_DEBUG "astfb: astfb_switch is called \n");

	disp = GET_DISP(info, con);

    	if (astfb_setmode(&fb_display[con].var, dinfo, CRT1))
    	    return 1;

	dinfo->currcon = con;
	disp->var.activate = FB_ACTIVATE_NOW;

} /* astfb_switch */

/*
 * panning
 */
static int
astfb_updatevar(int con, struct fb_info *info)
{
	struct display *disp = GET_DISP(info, con);

	printk(KERN_DEBUG "astfb: astfb_updatevar is called \n");

	if (con < 0)
		return -EINVAL;
	else
		return astfb_pan_display(&disp->var, -1, info);

} /* astfb_updatevar */

static void
astfb_set_disp(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{

	struct astfb_info *dinfo = GET_DINFO(info);
	struct display *disp = GET_DISP(info, con);
	struct fb_fix_screeninfo fix;
	struct display_switch *sw;

	astfb_get_fix(&fix, con, info);

	disp->var = *var;
	disp->screen_base = (UCHAR *)dinfo->pjFBVirtual;
	disp->visual = fix.visual;
	disp->type = fix.type;
	disp->type_aux = fix.type_aux;
	disp->ypanstep = fix.ypanstep;
	disp->ywrapstep = fix.ywrapstep;
	disp->line_length = fix.line_length;
	disp->next_line = fix.line_length;
	disp->can_soft_blank = 1;
	disp->inverse = 0;

	switch(disp->var.bits_per_pixel) {
#ifdef FBCON_HAS_CFB8
	case 8:
		sw = &fbcon_cfb8;
		break;
#endif
#ifdef FBCON_HAS_CFB16
	case 16:
		sw = &fbcon_cfb16;
		disp->dispsw_data = &dinfo->fbcon_cmap.cfb16;
		break;
#endif
#ifdef FBCON_HAS_CFB32
	case 32:
		sw = &fbcon_cfb32;
		disp->dispsw_data = &dinfo->fbcon_cmap.cfb32;
		break;
#endif
	default:
		sw = &fbcon_dummy;
		break;
	}
	memcpy(&dinfo->dispsw, sw, sizeof(*sw));
	disp->dispsw = &dinfo->dispsw;

	if (!ypan) {
		disp->scrollmode = SCROLL_YREDRAW;
		dinfo->dispsw.bmove = fbcon_redraw_bmove;
	}

} /* astfb_set_disp */

static int
astfb_get_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk(KERN_DEBUG "astfb: astfb_get_var is called \n");

	if (con == -1)
		*var = dinfo->disp.var;
	else
		*var = fb_display[con].var;

	return 0;
} /* astfb_get_var */

static int
astfb_set_var(struct fb_var_screeninfo *var, int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);
	int err;

	printk(KERN_DEBUG "astfb: astfb_set_var is called \n");

    	if (astfb_setmode(var, dinfo, CRT1))
    	    return 1;

        astfb_crtc_to_var(var, dinfo);

        astfb_set_disp(var, con, info);

	if((err = fb_alloc_cmap(&fb_display[con].cmap, 0, 0))) {
		return err;
	}

	astfb_do_install_cmap(con, info);

	return 0;

} /* astfb_set_var */


static void
astfb_do_install_cmap(int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

        if(con != dinfo->currcon) return;
        if(fb_display[con].cmap.len) {
		fb_set_cmap(&fb_display[con].cmap, 1, astfb_setcolreg, info);
        } else {
		int size = (dinfo->bpp == 8) ? 256 : 16;
		fb_set_cmap(fb_default_cmap(size), 1, astfb_setcolreg, info);
	}

} /* astfb_do_install_cmap */


static int
astfb_get_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);
	struct display *disp = GET_DISP(info, con);
	printk(KERN_DEBUG "astfb: astfb_get_cmap is called \n");

	if (con == dinfo->currcon)
		return fb_get_cmap(cmap, kspc, astfb_getcolreg, info);
	else if (disp->cmap.len)
		fb_copy_cmap(&disp->cmap, cmap, kspc ? 0 : 2);
	else {
		int cmap_len = (disp->var.bits_per_pixel > 8) ? 16 : 256;
		fb_copy_cmap(fb_default_cmap(cmap_len), cmap, kspc ? 0 : 2);
	}

	return 0;
} /* astfb_get_cmap */

static int
astfb_set_cmap(struct fb_cmap *cmap, int kspc, int con, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);
	struct display *disp = GET_DISP(info, con);
	unsigned int cmap_len;
	int err;
	printk(KERN_DEBUG "astfb: astfb_set_cmap is called \n");

	cmap_len = (disp->var.bits_per_pixel > 8) ? 16 : 256;
	if (!disp->cmap.len) {
		err = fb_alloc_cmap(&disp->cmap, cmap_len, 0);
		if (err)
			return err;
	}
	if (con == dinfo->currcon)
		return fb_set_cmap(cmap, kspc, astfb_setcolreg, info);
	else
		fb_copy_cmap(cmap, &disp->cmap, kspc ? 0 : 1);

	return 0;
} /* astfb_set_cmap */

static int
astfb_getcolreg(unsigned regno, unsigned *red, unsigned *green,
		unsigned *blue, unsigned *transp, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk(KERN_DEBUG "astfb: astfb_getcolreg is called \n");

	if (regno > 255)
		return 1;

	*red = (dinfo->palette[regno].red<<8) | dinfo->palette[regno].red;
	*green = (dinfo->palette[regno].green<<8) | dinfo->palette[regno].green;
	*blue = (dinfo->palette[regno].blue<<8) | dinfo->palette[regno].blue;
	*transp = 0;

	return 0;
} /* astfb_getcolreg */

static int
astfb_setcolreg(unsigned regno, unsigned red, unsigned green,
		unsigned blue, unsigned transp, struct fb_info *info)
{
	struct astfb_info *dinfo = GET_DINFO(info);

	printk(KERN_DEBUG "astfb: astfb_setcolreg is called \n");

	if(regno >= astfb_get_cmap_len(&info->var))
		return 1;

	switch(info->var.bits_per_pixel) {
	case 8:
                VGA_LOAD_PALETTE_INDEX (regno, (red >> 10), \
                                        (green >> 10), (blue >> 10));


	switch (info->var.bits_per_pixel) {
#ifdef FBCON_HAS_CFB8
	case 8:
		{
			red >>= 10;
			green >>= 10;
			blue >>= 10;

			dinfo->palette[regno].red = red;
			dinfo->palette[regno].green = green;
			dinfo->palette[regno].blue = blue;

			VGA_LOAD_PALETTE_INDEX(regno, red, green, blue);
		}
		break;
#endif
#ifdef FBCON_HAS_CFB16
	case 15:
		dinfo->fbcon_cmap.cfb16[regno] = ((red & 0xf800) >>  1) |
					       ((green & 0xf800) >>  6) |
					       ((blue & 0xf800) >> 11);
		break;
	case 16:
		dinfo->fbcon_cmap.cfb16[regno] = (red & 0xf800) |
					       ((green & 0xfc00) >>  5) |
					       ((blue  & 0xf800) >> 11);
		break;
#endif
#ifdef FBCON_HAS_CFB32
	case 24:
	case 32:
		dinfo->fbcon_cmap.cfb32[regno] = ((red & 0xff00) << 8) |
					       (green & 0xff00) |
					       ((blue  & 0xff00) >> 8);
		break;
#endif
	}
	return 0;
} /* astfb_setcolreg */

#endif /* Linux 2.4.x */
