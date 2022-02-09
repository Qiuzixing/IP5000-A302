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

#if ((CFG_CMD_SLT & CFG_CMD_HWCTEST) && defined(CONFIG_SLT))
#include "modelib.h"
#include "hwctest.h"

_HWCInfo HWCInfo = {
	"800x600x32bpp@60",
	CRT1,
	{0xFFFF, 0xF000, 0xF555, 0xFAAA},
	32,
	32,
	0xda6e1e00
};

BOOL bDrawHWC(_HWCInfo *HWCInfo, USHORT usWhichOne, ULONG ulHWC_BASE)
{

    ULONG i,j;
    ULONG ulHWCBase, ulLast;
    
    ulHWCBase = ulHWC_BASE + (HWC_HEIGHT - HWCInfo->Height) * (HWC_WIDTH * 2);
    ulLast = (HWC_WIDTH - HWCInfo->Width) * 2;
    ulHWCBase += ulLast;
    for (i=0; i<HWCInfo->Height; i++)
    {
        for (j=0; j< HWCInfo->Width; j++)
        {
            *(unsigned short *) (ulHWCBase + i * (HWC_WIDTH * 2) + j*2) =  HWCInfo->usPattern[i % 4];
        }		
    }
    
} /* bDrawHWC */

BOOL bShowHWC(_HWCInfo *HWCInfo, USHORT usWhichOne, ULONG ulHWC_OFFSET)
{
    ULONG ulCtlRegIndex, ulCtlReg = 0x03;		/* enable display */
    ULONG ulHWCOffsetReg, ulHWCXYReg, ulPatternReg;
    ULONG ulOffset, ulXY;
    
    /* Reg. Index Select */
    if (usWhichOne == CRT2 )
    {
        ulCtlRegIndex  = AST3000_VGA2_CTLREG;
        ulHWCOffsetReg = AST3000_HWC2_OFFSET;
        ulHWCXYReg     = AST3000_HWC2_XY;
        ulPatternReg   = AST3000_HWC2_PBase; 	
    }
    else
    {
        ulCtlRegIndex  = AST3000_VGA1_CTLREG;
        ulHWCOffsetReg = AST3000_HWC1_OFFSET;
        ulHWCXYReg     = AST3000_HWC1_XY;
        ulPatternReg   = AST3000_HWC1_PBase;          	
    }    
    
    /* Set default */
    ulOffset = ((HWC_HEIGHT - HWCInfo->Height) << 8) | (HWC_WIDTH -  HWCInfo->Width);
    ulXY = 0;
    
    /* Enable HWC */
    *(unsigned long *) (AST3000_VGAREG_BASE + ulHWCOffsetReg) = ulOffset;
    *(unsigned long *) (AST3000_VGAREG_BASE + ulHWCXYReg) = ulXY;
    *(unsigned long *) (AST3000_VGAREG_BASE + ulPatternReg) = ulHWC_OFFSET;
    *(unsigned long *) (AST3000_VGAREG_BASE + ulCtlRegIndex) = ulCtlReg;
    
    return (TRUE);
    	
} /* bShowHWC */  

int do_hwctest (void)
{
	unsigned long CRC, Flags = 0;
	_ModeInfo ModeInfo;
	
	vInitAST3000SCURegForCRT();

        /* Set Mode */
        if (!bSetSOCMode(HWCInfo.ModeInfoStr, &ModeInfo, NULL, HWCInfo.usWhichOne, VRAM_OFFSET))
        {
            printf("Set to %s Failed \n", HWCInfo.ModeInfoStr);
            return 1;
        } 
        ClearScreen(VRAM_BASE);
    
        /* Draw HWC */
	bDrawHWC(&HWCInfo, HWCInfo.usWhichOne & 0x01, HWC_BASE);
        
        /* Show HWC */
	bShowHWC(&HWCInfo, HWCInfo.usWhichOne & 0x01, HWC_OFFSET);
	
	/* Check HWC */
	CRC = bGetCRC(HWCInfo.usWhichOne);
	if ( CRC != HWCInfo.CRC)
	{
	    Flags = 1;
            printf("[FAIL] HWC Test Failed!! \n");
            printf("[INFO] Correct CRC: %x, Actual CRC: %x \n", HWCInfo.CRC, CRC);
	}	
	
	return Flags;
	
} /* do_hwctest */

#endif /* CONFIG_SLT */
