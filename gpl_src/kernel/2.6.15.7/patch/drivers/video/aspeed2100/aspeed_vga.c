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

#include <asm/arch/platform.h>
#include <asm/arch/memory.h>

#include "aspeed_fb.h"

OPT_MODE_STRUCT OptModeTable[] = {
    {"640x480x8" ,    640,  480,  8},
    {"640x480x16",    640,  480, 16},
    {"640x480x32",    640,  480, 32},
    {"800x600x8" ,    800,  600,  8},
    {"800x600x16",    800,  600, 16},
    {"800x600x32",    800,  600, 32},
    {"1024x768x8",   1024,  768,  8},
    {"1024x768x16",  1024,  768, 16},
    {"1024x768x32",  1024,  768, 32},
    {"1280x1024x8",  1280, 1024,  8},
    {"1280x1024x16", 1280, 1024, 16},
    {"1280x1024x32", 1280, 1024, 32},
    {"1600x1200x8",  1600, 1200,  8},
    {"1600x1200x16", 1600, 1200, 16},
    {"1600x1200x32", 1600, 1200, 32},
    {"end",1600,1200,  32},
    	
};	


VBIOS_ENHTABLE_STRUCT  Res640x480Table[] = {
    { 800, 640, 8, 96, 525, 480, 2, 2, VCLK25_175,	/* 60Hz */
      (SyncNN | HBorder | VBorder | Charx8Dot), 60, 1, 0x2E },
    { 832, 640, 16, 40, 520, 480, 1, 3, VCLK31_5,	/* 72Hz */
      (SyncNN | HBorder | VBorder | Charx8Dot), 72, 2, 0x2E  },
    { 840, 640, 16, 64, 500, 480, 1, 3, VCLK31_5,	/* 75Hz */
      (SyncNN | Charx8Dot) , 75, 3, 0x2E },      
    { 832, 640, 56, 56, 509, 480, 1, 3, VCLK36,		/* 85Hz */
      (SyncNN | Charx8Dot) , 85, 4, 0x2E },
    { 832, 640, 56, 56, 509, 480, 1, 3, VCLK36,		/* end */
      (SyncNN | Charx8Dot) , 0xFF, 4, 0x2E },              	      			
};


VBIOS_ENHTABLE_STRUCT  Res800x600Table[] = {
    {1024, 800, 24, 72, 625, 600, 1, 2, VCLK36,		/* 56Hz */
      (SyncPP | Charx8Dot), 56, 1, 0x30 },	      
    {1056, 800, 40, 128, 628, 600, 1, 4, VCLK40,	/* 60Hz */ 
      (SyncPP | Charx8Dot), 60, 2, 0x30 },
    {1040, 800, 56, 120, 666, 600, 37, 6, VCLK50,	/* 72Hz */ 
      (SyncPP | Charx8Dot), 72, 3, 0x30 },            
    {1056, 800, 16, 80, 625, 600, 1, 3, VCLK49_5,	/* 75Hz */ 
      (SyncPP | Charx8Dot), 75, 4, 0x30 },  	  			
    {1048, 800, 32, 64, 631, 600, 1, 3, VCLK56_25,	/* 85Hz */ 
      (SyncPP | Charx8Dot), 85, 5, 0x30 },  
    {1048, 800, 32, 64, 631, 600, 1, 3, VCLK56_25,	/* end */ 
      (SyncPP | Charx8Dot), 0xFF, 5, 0x30 },           
};


VBIOS_ENHTABLE_STRUCT  Res1024x768Table[] = {	
    {1344, 1024, 24, 136, 806, 768, 3, 6, VCLK65,	/* 60Hz */ 
      (SyncNN | Charx8Dot), 60, 1, 0x31 },	
    {1328, 1024, 24, 136, 806, 768, 3, 6, VCLK75,	/* 70Hz */ 
      (SyncNN | Charx8Dot), 70, 2, 0x31 },
    {1312, 1024, 16, 96, 800, 768, 1, 3, VCLK78_75,	/* 75Hz */ 
      (SyncPP | Charx8Dot), 75, 3, 0x31 },      
    {1376, 1024, 48, 96, 808, 768, 1, 3, VCLK94_5,	/* 85Hz */ 
      (SyncPP | Charx8Dot), 85, 4, 0x31 },  
    {1376, 1024, 48, 96, 808, 768, 1, 3, VCLK94_5,	/* end */ 
      (SyncPP | Charx8Dot), 0xFF, 4, 0x31 },             
};

VBIOS_ENHTABLE_STRUCT  Res1280x1024Table[] = {	
    {1688, 1280, 48, 112, 1066, 1024, 1, 3, VCLK108,	/* 60Hz */ 
      (SyncPP | Charx8Dot), 60, 1, 0x32 }, 
    {1688, 1280, 16, 144, 1066, 1024, 1, 3, VCLK135,	/* 75Hz */ 
      (SyncPP | Charx8Dot), 75, 2, 0x32 },  
    {1728, 1280, 64, 160, 1072, 1024, 1, 3, VCLK157_5,	/* 85Hz */ 
      (SyncPP | Charx8Dot), 85, 3, 0x32 },    
    {1728, 1280, 64, 160, 1072, 1024, 1, 3, VCLK157_5,	/* end */ 
      (SyncPP | Charx8Dot), 0xFF, 3, 0x32 },                        
};

VBIOS_ENHTABLE_STRUCT  Res1600x1200Table[] = {	
    {2160, 1600, 64, 192, 1250, 1200, 1, 3, VCLK162,	/* 60Hz */ 
      (SyncPP | Charx8Dot), 60, 1, 0x33 }, 
    {2160, 1600, 64, 192, 1250, 1200, 1, 3, VCLK162,	/* end */ 
      (SyncPP | Charx8Dot), 60, 1, 0x33 },         
};

ULONG AST2100DCLKTable [] = {
    0x00046515,					/* 00: VCLK25_175	*/		
    0x00047255,				        /* 01: VCLK28_322	*/
    0x0004682a,				        /* 02: VCLK31_5         */
    0x0004672a,				        /* 03: VCLK36         	*/
    0x00046c50,				        /* 04: VCLK40          	*/
    0x00046842, 		        	/* 05: VCLK49_5        	*/
    0x00006c32,                        	        /* 06: VCLK50          	*/
    0x00006a2f,                        	        /* 07: VCLK56_25       	*/
    0x00006c41,                        	        /* 08: VCLK65		*/
    0x00006832,                        	        /* 09: VCLK75	        */
    0x0000672e,				        /* 0A: VCLK78_75       	*/
    0x0000683f,                        	        /* 0B: VCLK94_5        	*/
    0x00004824,                        	        /* 0C: VCLK108         	*/
    0x0000482d,                        	        /* 0D: VCLK135         	*/
    0x0000472e,                        	        /* 0E: VCLK157_5       	*/
    0x00004836,				        /* 0F: VCLK162         	*/
};

/* external reference fucntion */
extern int astfb_bEnable2D(struct astfb_info *dinfo);
extern void astfb_vDisable2D(struct astfb_info *dinfo);

/* internal declaration */
int astfb_SearchOPTModeInfo(char *mode, struct astfb_info *dinfo);	
int astfb_setmode(struct fb_var_screeninfo *var, struct astfb_info *dinfo, USHORT usWhichOne);
int astfb_getmodeinfo(struct fb_var_screeninfo *var, struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo);
void vSetCRTCReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne);
void vSetOffsetReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne);
void vSetDCLKReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne);
void vSetExtReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne);

int
astfb_SearchOPTModeInfo(char *mode, struct astfb_info *dinfo)
{
    OPT_MODE_STRUCT *OptMode;
  
    OptMode = OptModeTable;
    while (strcmp(OptMode++->ModeString, "end"))
    {
    	
        if (!strcmp(OptMode->ModeString, mode))
        {
            dinfo->var.xres = dinfo->var.xres_virtual = OptMode->xres;
            dinfo->var.yres = dinfo->var.yres_virtual = OptMode->yres;
            dinfo->var.bits_per_pixel  = OptMode->bits_per_pixel;           	
            return 0;	
        }	
        
    }	
	
    return 1;
}	

int
astfb_setmode(struct fb_var_screeninfo *var, struct astfb_info *dinfo, USHORT usWhichOne)
{
    VBIOS_MODE_INFO vgamodeinfo; 
		    
    if (!astfb_getmodeinfo(var, dinfo, &vgamodeinfo))
    {
        /* pre set mode */    		
#if	Enable2D
        astfb_vDisable2D(dinfo);
#endif
      
        /* set mode */
        vSetCRTCReg(dinfo,  &vgamodeinfo, usWhichOne);
        vSetOffsetReg(dinfo,  &vgamodeinfo, usWhichOne);    
        vSetDCLKReg(dinfo,  &vgamodeinfo, usWhichOne);
        vSetExtReg(dinfo, &vgamodeinfo, usWhichOne);
        
        /* post set mode */
#if	Enable2D
        astfb_bEnable2D(dinfo);
#endif        
        
        return 0;
    }
    	
    return 1;
	
} /* astfb_setmode */


int astfb_getmodeinfo(struct fb_var_screeninfo *var, struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo)
{
    ULONG ulColorIndex, ulRefreshRate, ulRefreshRateIndex = 0;
    ULONG ulHT, ulVT,ulHBorder, ulVBorder;
            
    ulHT = var->xres + var->left_margin + var->right_margin + var->hsync_len;
    ulVT = var->yres + var->upper_margin + var->lower_margin + var->vsync_len;
    ulRefreshRate = (var->pixclock * 1000) / (ulHT * ulVT);
    
    ulRefreshRate = dinfo->refreshrate;
    
    /* check limitation */
    /*[undo] */
         
    /* Get mode entry */     
    switch (var->bits_per_pixel)
    {
    case 8:
	 ulColorIndex = VGAModeIndex-1;       
         break;   
    case 16:
	 ulColorIndex = HiCModeIndex-1;                
         break;
    case 24:         
    case 32:
	 ulColorIndex = TrueCModeIndex-1;                
	 break;    
    default:
         return 1;
    }

    switch (var->xres)
    {
    case 640:
         pVGAModeInfo->pEnhTableEntry = (PVBIOS_ENHTABLE_STRUCT) &Res640x480Table[ulRefreshRateIndex];
         break;
    case 800:
         pVGAModeInfo->pEnhTableEntry = (PVBIOS_ENHTABLE_STRUCT) &Res800x600Table[ulRefreshRateIndex];
         break;         
    case 1024:
         pVGAModeInfo->pEnhTableEntry = (PVBIOS_ENHTABLE_STRUCT) &Res1024x768Table[ulRefreshRateIndex];
         break;
    case 1280:
         pVGAModeInfo->pEnhTableEntry = (PVBIOS_ENHTABLE_STRUCT) &Res1280x1024Table[ulRefreshRateIndex];
         break;
    case 1600:
         pVGAModeInfo->pEnhTableEntry = (PVBIOS_ENHTABLE_STRUCT) &Res1600x1200Table[ulRefreshRateIndex];
         break;                  
    default:
         return 1;     
    }

    /* Get Proper Mode Index */
    while (pVGAModeInfo->pEnhTableEntry->ulRefreshRate < ulRefreshRate)
    {
        pVGAModeInfo->pEnhTableEntry++;
        if ((pVGAModeInfo->pEnhTableEntry->ulRefreshRate > ulRefreshRate) || 
            (pVGAModeInfo->pEnhTableEntry->ulRefreshRate == 0xFF)) 
        {
            pVGAModeInfo->pEnhTableEntry--;
            break; 	
        }    
    }

    /* Update var info */
    ulHBorder = (pVGAModeInfo->pEnhTableEntry->Flags & HBorder) ? 1:0;    
    ulVBorder = (pVGAModeInfo->pEnhTableEntry->Flags & VBorder) ? 1:0;    
    
    var->left_margin = ulHBorder ? (u32) (pVGAModeInfo->pEnhTableEntry->HFP + 8) : (u32) (pVGAModeInfo->pEnhTableEntry->HFP);
    var->hsync_len = (u32) pVGAModeInfo->pEnhTableEntry->HSYNC;    
    var->right_margin = (u32) (pVGAModeInfo->pEnhTableEntry->HT - var->xres - var->left_margin - var->hsync_len);
 
    var->upper_margin = ulVBorder ? (u32) (pVGAModeInfo->pEnhTableEntry->VFP + 1) : (u32) (pVGAModeInfo->pEnhTableEntry->VFP);
    var->vsync_len = (u32) pVGAModeInfo->pEnhTableEntry->VSYNC;    
    var->lower_margin = (u32) (pVGAModeInfo->pEnhTableEntry->VT - var->yres - var->upper_margin - var->vsync_len);           

    /* update dinfo */
    dinfo->xres = var->xres;
    dinfo->yres = var->yres;
    dinfo->xres_virtual = var->xres_virtual;
    dinfo->yres_virtual = var->yres_virtual;    
    dinfo->bpp = var->bits_per_pixel;
    dinfo->pitch = var->xres_virtual * ((dinfo->bpp + 1) / 8) ;           
    dinfo->pixclock = var->pixclock;
                          
    return 0;
    
} /* astfb_getmodeinfo */


void
vSetCRTCReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne)
{
    PVBIOS_ENHTABLE_STRUCT pEnhModePtr;
    ULONG HTIndex, HRIndex;
    ULONG VTIndex, VRIndex;     
    USHORT HT, HDE, HBS, HBE, HRS, HRE;
    USHORT VT, VDE, VBS, VBE, VRS, VRE;    	
    USHORT HT2, HDE2, HRS2, HRE2;
    USHORT VT2, VDE2, VRS2, VRE2;    	

    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        HTIndex =  AST2100_VGA2_HTREG;
        HRIndex =  AST2100_VGA2_HRREG;  
        VTIndex =  AST2100_VGA2_VTREG;
        VRIndex =  AST2100_VGA2_VRREG;               
    }
    else
    {
        HTIndex =  AST2100_VGA1_HTREG;
        HRIndex =  AST2100_VGA1_HRREG;
        VTIndex =  AST2100_VGA1_VTREG;
        VRIndex =  AST2100_VGA1_VRREG;          
    }    
    
    /* Get CRTC Info */
    pEnhModePtr = pVGAModeInfo->pEnhTableEntry; 
    HT = pEnhModePtr->HT;
    HDE= pEnhModePtr->HDE;
    HBS= (pEnhModePtr->Flags & HBorder) ? HDE+1 : HDE;
    HBE= (pEnhModePtr->Flags & HBorder) ? HT-1 : HT;    
    HRS= HBS + pEnhModePtr->HFP;
    HRE= HBS + pEnhModePtr->HFP + pEnhModePtr->HSYNC;
    VT = pEnhModePtr->VT;
    VDE= pEnhModePtr->VDE;
    VBS= (pEnhModePtr->Flags & VBorder) ? VDE+1 : VDE;
    VBE= (pEnhModePtr->Flags & VBorder) ? VT-1 : VT;    
    VRS= VBS + pEnhModePtr->VFP;
    VRE= VBS + pEnhModePtr->VFP + pEnhModePtr->VSYNC;           
    
    /* Calculate CRTC Reg Setting */
    HT2  = HT - 1;
    HDE2 = HDE - 1;
    HRS2 = HRS - 1;
    HRE2 = HRE - 1;
    VT2  = VT  - 1;
    VDE2 = VDE - 1;
    VRS2 = VRS - 1;    
    VRE2 = VRE - 1;
    
    /* Write Reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + HTIndex, (ULONG)(HDE2 << 16) | (ULONG) (HT2));                                                                                             
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + HRIndex, (ULONG)(HRE2 << 16) | (ULONG) (HRS2));                                                                                             
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + VTIndex, (ULONG)(VDE2 << 16) | (ULONG) (VT2));                                                                                             
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + VRIndex, (ULONG)(VRE2 << 16) | (ULONG) (VRS2));                                                                                             
                          		
}

void vSetOffsetReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne)
{
    ULONG ulOffsetIndex;
    ULONG usOffset, usTermalCount;	
    
    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulOffsetIndex =  AST2100_VGA2_OFFSETREG;
    }
    else
    {
        ulOffsetIndex =  AST2100_VGA1_OFFSETREG;
    }
    
    usOffset = 	((ULONG) dinfo->xres *  dinfo->bpp) >> 3; 		/* Unit: char */
    usTermalCount = ((ULONG) dinfo->xres *  dinfo->bpp + 63 ) >> 6; 		/* Unit: quad char */ 
    
    /* Write Reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulOffsetIndex, (ULONG) (usTermalCount << 16) | (ULONG) (usOffset));

}

void vSetDCLKReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne)
{
    PVBIOS_ENHTABLE_STRUCT pEnhModePtr;
    ULONG *pDCLKPtr, ulPLLIndex;
         
    pEnhModePtr = pVGAModeInfo->pEnhTableEntry;
    pDCLKPtr = &AST2100DCLKTable[pEnhModePtr->DCLKIndex];

    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulPLLIndex =  AST2100_VGA2_PLL;
    }
    else
    {
        ulPLLIndex =  AST2100_VGA1_PLL;
    }
        
    /* Write Reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulPLLIndex, *(ULONG *) pDCLKPtr);
}

void vSetExtReg(struct astfb_info *dinfo, PVBIOS_MODE_INFO pVGAModeInfo, USHORT usWhichOne)
{
    PVBIOS_ENHTABLE_STRUCT pEnhModePtr;	
    ULONG ulCtlRegIndex, ulCtlReg = 0x01;
    ULONG ulCtlReg2Index, ulCtlReg2 = 0xC1;    
    ULONG ulThresholdRegIndex;
    ULONG ulStartAddressIndex, ulStartAddress;

    pEnhModePtr = pVGAModeInfo->pEnhTableEntry;
    	
    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulCtlRegIndex =  AST2100_VGA2_CTLREG;
        ulCtlReg2Index =  AST2100_VGA2_CTLREG2;        
        ulThresholdRegIndex = AST2100_VGA2_THRESHOLD;
        ulStartAddressIndex =  AST2100_VGA2_STARTADDR;
    }
    else
    {
        ulCtlRegIndex =  AST2100_VGA1_CTLREG;
        ulCtlReg2Index =  AST2100_VGA1_CTLREG2;        
        ulThresholdRegIndex = AST2100_VGA1_THRESHOLD;
        ulStartAddressIndex =  AST2100_VGA1_STARTADDR;
    }

    /* Mode Type Setting */	
    switch (dinfo->bpp) {
    case 16:   
        ulCtlReg |= 0x0;            	//RGB565
        break;    
    case 32:
        ulCtlReg |= 0x100;             	//RGB888         
        break;
    }
    
    /* Start Address */
    ulStartAddress = dinfo->ulFBPhys;

    /* Polarity */
    ulCtlReg |= ((pEnhModePtr->Flags & SyncNN) << 10);
        
    /* Write Reg */
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulStartAddressIndex, ulStartAddress);
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulThresholdRegIndex, ((ULONG) CRT_HIGH_THRESHOLD_VALUE << 8) | (ULONG) (CRT_LOW_THRESHOLD_VALUE) );
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulCtlReg2Index, ulCtlReg2);                                                                                             
    IO_WRITE(ASPEED_GRAPHIC_VA_BASE + ulCtlRegIndex, ulCtlReg);                                                                                             
	
}