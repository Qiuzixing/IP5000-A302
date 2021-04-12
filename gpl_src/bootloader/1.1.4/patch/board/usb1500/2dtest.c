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

#if ((CFG_CMD_SLT & CFG_CMD_2DTEST) && defined(CONFIG_SLT))
#include "modelib.h"
#include "2dtest.h"

/* Test Options */
_BLTTest BltTest = {
	"800x600x32bpp@60",
	CRT1,
	{0xFFFFFFFF, 0x00000000, 0x55555555, 0xAAAAAAAA},
	32,
	32,
	0xFFFFFFFF,
	0x00000000,
	0x062d3000
};

void vInit2D(void)
{
    /* MMIO Mode */	
    *(ULONG *) (ENG_BASE + 0x40) = 0x80000000;
    *(ULONG *) (ENG_BASE + 0x44) = 0xF2000000;
    	
}

void vWaitEngIdle(void)
{
    ULONG ulEngState, ulEngState2;	

    do {
        ulEngState  = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
        ulEngState2 = *(volatile ULONG *) (ENG_BASE + 0x4C);
    } while ((ulEngState & 0x80000000) || (ulEngState != ulEngState2));
	
}

int do_blt(_BLTTest *BltInfo, _DSTSurface *DstSurface)
{
	
    ULONG i, j, cmdreg, step, SrcPitch, DstPitch;
    ULONG dst_x=0, dst_y=0;
    ULONG CRC;
    
    /* Prepare Parameters */
    cmdreg = CMD_BITBLT;
    switch (DstSurface->ulBitPerPixel)
    {
    case 8:
        cmdreg |= CMD_COLOR_08;
        step = 1;
        break;
    case 15:
    case 16:
        cmdreg |= CMD_COLOR_16;
        step = 2;
        break;    
    case 24:
    case 32:
        cmdreg |= CMD_COLOR_32;
        step = 4;
        break;    	
    }
    SrcPitch = DstPitch = DstSurface->ulWidth * ((DstSurface->ulBitPerPixel + 1) / 8);
    
    /* Prepare Src */
    for (i=0; i< BltInfo->SrcHeight; i++)
    {
        for (j=0; j<BltInfo->SrcWidth; j++)
        {
            switch (step)
            {
            case 1:
                *(unsigned char *) (DstSurface->ulVirtualAddr + i*DstPitch + j) = (unsigned char) (BltInfo->ulPattern[i%4]);
                break;
            case 2:
                *(unsigned short *) (DstSurface->ulVirtualAddr + i*DstPitch + j*2) = (unsigned short) (BltInfo->ulPattern[i%4]);
                break;            
            case 4:
                *(unsigned long *) (DstSurface->ulVirtualAddr + i*DstPitch + j*4) = (unsigned long) (BltInfo->ulPattern[i%4]);
                break;
            	
            }
        }
    }

    /* Set up */
    ASTSetupSRCBase_MMIO(DstSurface->ulBaseAddr);
    ASTSetupSRCPitch_MMIO(SrcPitch);
    
    ASTSetupDSTBase_MMIO(DstSurface->ulBaseAddr);           
    ASTSetupDSTPitchHeight_MMIO(DstPitch, DstSurface->ulHeight);    

    ASTSetupSRCXY_MMIO(0, 0);    
    ASTSetupRECTXY_MMIO(BltInfo->SrcWidth, BltInfo->SrcHeight);    

    ASTSetupFG_MMIO(BltInfo->FG);
    ASTSetupBG_MMIO(BltInfo->BG);
    
    /* Subsequent */
    for (i=0; i<256; i++)			/* ROP */
    {
        dst_x += BltInfo->SrcWidth;
        if (dst_x > DstSurface->ulWidth)
        {
            dst_x = 0;
            dst_y += BltInfo->SrcHeight;
        }	
    	
        ASTSetupDSTXY_MMIO(dst_x, dst_y);    
        ASTSetupCMDReg_MMIO(cmdreg | (i << 8));       
            	
        vWaitEngIdle();
        
    } /* ROP */  

    /* Check CRC */
    CRC = bGetCRC(BltInfo->usWhichOne);
    if ( CRC != BltInfo->CRC)
    {
        printf("[FAIL] 2D Blt Test Failed!! \n");
        printf("[INFO] Correct CRC: %x, Actual CRC: %x \n", BltInfo->CRC, CRC);
        return 1;
    }	
    
    return 0;
    
} /* do_blt */

int do_2dtest (void)
{
	unsigned long Flags = 0;
	_ModeInfo ModeInfo;
        _DSTSurface DstSurface;
        	
	vInitASTSCURegForCRT();

        vInit2D();
        
        /* Do Blt Test */
        if (!bSetSOCMode(BltTest.ModeInfoStr, &ModeInfo, NULL, BltTest.usWhichOne, VRAM_OFFSET))
        {
            printf("Set to %s Failed \n", BltTest.ModeInfoStr);
            return 1;
        } 
        
        DstSurface.ulWidth = (ULONG) ModeInfo.usWidth;
        DstSurface.ulHeight = (ULONG) ModeInfo.usHeight;
        DstSurface.ulBitPerPixel = (ULONG) ModeInfo.usBitPerPixel;
        DstSurface.ulBaseAddr = (ULONG) VRAM_OFFSET;
        DstSurface.ulVirtualAddr = (ULONG) VRAM_BASE;        
        ClearScreen(VRAM_BASE);        
        if (do_blt(&BltTest, &DstSurface))
        {
            Flags = 1;
        } 
        
	return Flags;
	
} /* do_2dtest */

#endif /* CONFIG_SLT */
