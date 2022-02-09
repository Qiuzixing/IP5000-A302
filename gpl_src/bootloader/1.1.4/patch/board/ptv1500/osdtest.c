/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Diagnostics support
 */
#include <common.h>
#include <command.h>
#include <post.h>
#include "slt.h"

#if ((CFG_CMD_SLT & CFG_CMD_OSDTEST) && defined(CONFIG_SLT))
#include "modelib.h"
#include "osdtest.h"

/* Test Options */
_OSDPalette OSDPallette[] = {
    { 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x1f},
    { 0x00, 0x1f, 0x00},
    { 0x00, 0x1f, 0x1f},
    { 0x1f, 0x00, 0x00},
    { 0x1f, 0x00, 0x1f},
    { 0x1f, 0x1f, 0x00},
    { 0x1f, 0x1f, 0x1f},

    { 0x00, 0x00, 0x00},
    { 0x00, 0x00, 0x1f},
    { 0x00, 0x1f, 0x00},
    { 0x00, 0x1f, 0x1f},
    { 0x1f, 0x00, 0x00},
    { 0x1f, 0x00, 0x1f},
    { 0x1f, 0x1f, 0x00},
    { 0x1f, 0x1f, 0x1f},
};

_OSDInfo OSDInfo = {
	"800x600x32bpp@60",
	CRT1,
	{0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA},
	64,
	64,
	0x1aab3200
};


BOOL bDrawOSD(_OSDInfo *OSDInfo, USHORT usWhichOne, ULONG ulOSD_BASE)
{

    ULONG i,j, Pitch;
    
    Pitch = OSDInfo->X / 2;
    for (i=0; i<OSDInfo->Y; i++)
    {
        for (j=0; j< (OSDInfo->X / 2); j++)
        {
            *(unsigned char *) (ulOSD_BASE + i * Pitch + j) =  (unsigned char) OSDInfo->ulPattern[i % 4];
        }		
    }
    
} /* bDrawOSD */


BOOL bShowOSD(_OSDInfo *OSDInfo, USHORT usWhichOne, ULONG ulOSD_OFFSET)
{
    ULONG ulOSDHReg, ulOSDVReg, ulOSDPBaseReg, ulOSDOffsetReg, ulOSDThresholdReg;
    ULONG ulOSDHStart, ulOSDHEnd, ulOSDVStart, ulOSDVEnd;
    ULONG ulOSDPBase, ulOSDOffset, ulOSDCount; 
    ULONG ulOSDThreshold_High,ulOSDThreshold_Low, ulOSDAlpha;
    ULONG ulCtlRegIndex, ulCtlReg = 0x05;		/* enable display */
    ULONG i;
    ULONG PaletteData, PaletteIndex=0;
    
    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulCtlRegIndex =  AST1500_VGA2_CTLREG;    	
        ulOSDHReg =  AST1500_OSD2_H;
        ulOSDVReg =  AST1500_OSD2_V;     	
        ulOSDPBaseReg =  AST1500_OSD2_PBase;     	
        ulOSDOffsetReg =  AST1500_OSD2_Offset;     	
        ulOSDThresholdReg =  AST1500_OSD2_THRESHOLD;     	             	
    }
    else
    {
        ulCtlRegIndex =  AST1500_VGA1_CTLREG;    	
        ulOSDHReg =  AST1500_OSD1_H;
        ulOSDVReg =  AST1500_OSD1_V;     	
        ulOSDPBaseReg =  AST1500_OSD1_PBase;     	
        ulOSDOffsetReg =  AST1500_OSD1_Offset;     	
        ulOSDThresholdReg =  AST1500_OSD1_THRESHOLD;     	             	      	
    }    
    
    /* Set default */
    ulOSDHStart = 0;
    ulOSDHEnd   = OSDInfo->X;
    ulOSDVStart = 0;
    ulOSDVEnd   = OSDInfo->Y;
    ulOSDPBase = ulOSD_OFFSET;
    ulOSDThreshold_High = 8;
    ulOSDThreshold_Low = 3;
    ulOSDAlpha = 0x08;

    ulOSDOffset = (ulOSDHEnd - ulOSDHStart) / 2 ;
    ulOSDCount = (ulOSDHEnd - ulOSDHStart + 15 ) / 16;
              
    /* Set OSD Color */
    for (i=0 ; i<32; i+=4)
    {
        PaletteData = 0x8000 | (OSDPallette[PaletteIndex].R << 10) | (OSDPallette[PaletteIndex].G << 5) | (OSDPallette[PaletteIndex].B);
        PaletteData |= 0x80000000 | (OSDPallette[PaletteIndex+1].R << 26) | (OSDPallette[PaletteIndex+1].G << 21) | (OSDPallette[PaletteIndex+1].B << 16);
        *(unsigned long *) (AST1500_VGAREG_BASE + 0xE0 + i) = PaletteData;        
        PaletteIndex+=2;      
    }
    
    /* Enable OSD */
    *(unsigned long *) (AST1500_VGAREG_BASE + ulOSDHReg) = (ulOSDHStart & 0xFFF) | ((ulOSDHEnd & 0xFFF) << 16);
    *(unsigned long *) (AST1500_VGAREG_BASE + ulOSDVReg) = (ulOSDVStart & 0x7FF) | ((ulOSDVEnd & 0x7FF) << 16);
    *(unsigned long *) (AST1500_VGAREG_BASE + ulOSDPBaseReg) = ulOSDPBase;
    *(unsigned long *) (AST1500_VGAREG_BASE + ulOSDOffsetReg) = (ulOSDOffset & 0x7FF) | ((ulOSDCount & 0xFF) << 16);
    *(unsigned long *) (AST1500_VGAREG_BASE + ulOSDThresholdReg) = ulOSDThreshold_High | (ulOSDThreshold_Low << 8)  | (ulOSDAlpha << 16);
    *(unsigned long *) (AST1500_VGAREG_BASE + ulCtlRegIndex) = ulCtlReg;
    
    return (TRUE);
    	
} /* bShowOSD */  

int do_osdtest (void)
{
	unsigned long CRC, Flags = 0;
	_ModeInfo ModeInfo;
	
	vInitASTSCURegForCRT();

        /* Set Mode */
        if (!bSetSOCMode(OSDInfo.ModeInfoStr, &ModeInfo, NULL, OSDInfo.usWhichOne, VRAM_OFFSET))
        {
            printf("Set to %s Failed \n", OSDInfo.ModeInfoStr);
            return 1;
        } 
        ClearScreen(VRAM_BASE);
    
        /* Draw OSD */
	bDrawOSD(&OSDInfo, OSDInfo.usWhichOne & 0x01, OSD_BASE);
        
        /* Show OSD */
	bShowOSD(&OSDInfo, OSDInfo.usWhichOne & 0x01, OSD_OFFSET);
	
	/* Check OSD */
	CRC = bGetCRC(OSDInfo.usWhichOne);
	if ( CRC != OSDInfo.CRC)
	{
	    Flags = 1;
            printf("[FAIL] OSD Test Failed!! \n");
            printf("[INFO] Correct OSD: %x, Actual OSD: %x \n", OSDInfo.CRC, CRC);
	}	
	
	return Flags;
	
} /* do_osdtest */

#endif /* CONFIG_SLT */
