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

#include "aspeed_fb.h"

int astfb_bEnable2D(struct astfb_info *dinfo);
void astfb_vDisable2D(struct astfb_info *dinfo);

int astfb_bEnableCMDQ(struct astfb_info *dinfo);
void astfb_vWaitEngIdle(struct astfb_info *dinfo);

int astfb_bEnable2D(struct astfb_info *dinfo)
{

        IO_WRITE(dinfo->pjMMIOVirtual + 0x40, 0x80000000);                         		       
	
        astfb_bEnableCMDQ(dinfo);

	return 0;	
}

void astfb_vDisable2D(struct astfb_info *dinfo)
{

        IO_WRITE(dinfo->pjMMIOVirtual + 0x40, 0x00000000);                         		       
	
	/* wait idle */ 
        astfb_vWaitEngIdle(dinfo);
        	
}


int astfb_bEnableCMDQ(struct astfb_info *dinfo)
{
    ULONG ulVMCmdQBasePort = 0;

    astfb_vWaitEngIdle(dinfo);

    /* set DBG Select Info */
    /* none */
    
    /* set CMDQ base */
    if (dinfo->ul2DMode == CMDQ_MODE)
    {	/* CMDQ */
        ulVMCmdQBasePort  = (dinfo->ulFBPhys + dinfo->ulCMDQOffset) >> 3;
 
        /* set CMDQ Threshold */
        ulVMCmdQBasePort |= 0xF0000000;			   

        /* set CMDQ Size */
        switch (dinfo->ulCMDQSize)
        {
        case CMD_QUEUE_SIZE_256K:
            ulVMCmdQBasePort |= 0x00000000;   
            break;
        	
        case CMD_QUEUE_SIZE_512K:
            ulVMCmdQBasePort |= 0x04000000;   
            break;
      
        case CMD_QUEUE_SIZE_1M:
            ulVMCmdQBasePort |= 0x08000000;       
            break;
            
        case CMD_QUEUE_SIZE_2M:
            ulVMCmdQBasePort |= 0x0C000000;       
            break;        
            
        default:
            return 0;
            break;
        }     

        IO_WRITE(dinfo->pjMMIOVirtual + 0x44, ulVMCmdQBasePort);
    }
    else
    {	/* MMIO */
        /* set CMDQ Threshold */
        ulVMCmdQBasePort |= 0xF0000000;			       
        ulVMCmdQBasePort |= 0x02000000;			/* MMIO mode */

        IO_WRITE(dinfo->pjMMIOVirtual + 0x44, ulVMCmdQBasePort);
    }
   	
}
	
void astfb_vWaitEngIdle(struct astfb_info *dinfo)
{
    ULONG ulData, ulEngState, ulEngState2;
    ULONG ulEngCheckSetting; 
    
    if (dinfo->ul2DMode == CMDQ_MODE)
        ulEngCheckSetting = 0x80000000;
    else
        ulEngCheckSetting = 0x10000000;
    
    /* Chk if disable 2D */
    ulData  = IO_READ(dinfo->pjMMIOVirtual + 0x40);
    if (!(ulData & 0x80000000))
        goto  Exit_vWaitEngIdle;

    do  
    {
        ulEngState  = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
        ulEngState2 = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
        ulEngState2 = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
        ulEngState2 = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
        ulEngState2 = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
        ulEngState2 = IO_READ(dinfo->pjMMIOVirtual + 0x4C) & 0xFFFC0000;
                      
    } while ((ulEngState & ulEngCheckSetting) || (ulEngState != ulEngState2));
    
Exit_vWaitEngIdle:
    ;   	
}
