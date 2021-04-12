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
#include <linux/delay.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#include <video/fbcon.h>
#include <video/fbcon-cfb8.h>
#include <video/fbcon-cfb16.h>
#include <video/fbcon-cfb24.h>
#include <video/fbcon-cfb32.h>
#endif

#include <asm/arch/platform.h>

#include "aspeed_fb.h"

/* Prototype Declaration */
int astfb_getchipinfo(struct device *dev, struct astfb_info *dinfo);
int astfb_initchip(struct device *dev, struct astfb_info *dinfo);
int astfb_pan_display_var(struct fb_var_screeninfo *var, int con, struct astfb_info *dinfo);
int astfb_blank_var(int blank, struct astfb_info *dinfo);
int astfb_crtc_to_var(struct fb_var_screeninfo *var, struct astfb_info *dinfo);
                        
int
astfb_getchipinfo(struct device *dev, struct astfb_info *dinfo)
{

    strcpy(dinfo->name, "AST(R) 3000");

    return 0;
    	
} /* astfb_getchipinfo */

	
int     
astfb_initchip(struct device *dev, struct astfb_info *dinfo)
{        
    u32 ulData;

    /* Set Multi-Func. Pin */
    ulData = IO_READ(ASPEED_SCU_VA_BASE + 0x74);                                                                                             
    ulData &= 0xFFFF7FFF;
    ulData |= 0x00c30000;
    IO_WRITE(ASPEED_SCU_VA_BASE + 0x74, ulData);    

    /* Set Delay Compensation */
    ulData = IO_READ(ASPEED_SCU_VA_BASE + 0x08);                                                                                             
    ulData &= 0xFFFF00FF;
    ulData |= 0x0000D500;
    IO_WRITE(ASPEED_SCU_VA_BASE + 0x08, ulData);        

    /* Enable 2D Trapping after FPGA v9, ycchen@030807 */
    ulData = IO_READ(ASPEED_SCU_VA_BASE + 0x2C);                                                                                             
    IO_WRITE(ASPEED_SCU_VA_BASE + 0x2C, (ulData | 0x80));                                                                                             

    /* Reset CRT */     
    ulData = IO_READ(ASPEED_SCU_VA_BASE + 0x0C);                                                                                             
    IO_WRITE(ASPEED_SCU_VA_BASE + 0x0C, (ulData & 0xfffff3fd));                                                                                             
    udelay(10);
    ulData = IO_READ(ASPEED_SCU_VA_BASE + 0x04);                                                                                             
    IO_WRITE(ASPEED_SCU_VA_BASE + 0x04, (ulData & 0xffffdf7b));                                                                                             
        	
    return 0;
    	
} /* astfb_initchip */

int
astfb_pan_display_var(struct fb_var_screeninfo *var, int con, struct astfb_info *dinfo)
{
    u32 base, ulStartAddressIndex;
    u16 usWhichOne = CRT1;
        	
    base = var->yoffset * var->xres_virtual + var->xoffset;
		
    /* calculate base bpp dep. */
    switch(var->bits_per_pixel) {
    case 32:
        break;
    case 16:
       	base >>= 1;
       	break;
    case 8:
    default:
       	base >>= 2;
        break;
    }

    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulStartAddressIndex =  AST2100_VGA2_STARTADDR;
    }
    else
    {
        ulStartAddressIndex =  AST2100_VGA1_STARTADDR;
    }

    /* set reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulStartAddressIndex, dinfo->ulFBPhys + base);
    
    return 0;    	
}

int
astfb_blank_var(int blank, struct astfb_info *dinfo)
{
   u32 reg, ulCrtlRegIndex;
   u16 usWhichOne = CRT1;
      	
   switch(blank) {
   case 0:	/* on */
      reg = 0x00000000;
      break;
   case 1:	/* blank */
      reg = 0x00100000;
      break;
   case 2:	/* no vsync */
      reg = 0x00180000;
      break;
   case 3:	/* no hsync */
      reg = 0x00140000;
      break;
   case 4:	/* off */
      reg = 0x001c0000;
      break;
   default:
      return 1;
   }	

    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulCrtlRegIndex =  AST2100_VGA2_CTLREG;
    }
    else
    {
        ulCrtlRegIndex =  AST2100_VGA1_CTLREG;
    }
   
    /* set reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulCrtlRegIndex, reg);
 
    return 0;
   
} /* astfb_blank_var */

       
int
astfb_crtc_to_var(struct fb_var_screeninfo *var, struct astfb_info *dinfo)
{
    
    /* crtc */
    var->xoffset = var->yoffset = 0;
    
    /* palette */
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
    
    var->red.msb_right = 
    var->green.msb_right =
    var->blue.msb_right =
    var->transp.offset = 
    var->transp.length =  
    var->transp.msb_right = 0;
	    
    return 0;
}
