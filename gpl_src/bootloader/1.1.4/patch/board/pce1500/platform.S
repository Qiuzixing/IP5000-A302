/*
 * Board specific setup info
 *
 * (C) Copyright 2003, ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
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

#include <config.h>
#include <version.h>

.globl lowlevel_init
lowlevel_init:

init_dram:
    mov r4, lr		/* save lr */

    /*Set Scratch register Bit 7 before initialize*/
    /* *(scu1+ 0) = 0xa8 */
    ldr r0, =0x1e6e2000
    ldr r1, =0x1688a8a8
    str r1, [r0]

    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x80
    str r1, [r0]

    /*Check Stratch Register Bit 6*/
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    bic r1, r1, #0xFFFFFFBF
    mov r2, r1, lsr #6
    cmp r2, #0x01
    beq platform_exit

    /* MCLK Setting for AST1500*/
    ldr r0, =0x1e6e2004
    ldr r1, [r0]
#ifdef CONFIG_AST1500_CLIENT
    orr r1, r1, #0x01A0
#else
    /* Host need to enable internal VGA. */
    orr r1, r1, #0x0020
#endif
    str r1, [r0]

    ldr r0, =0x1e6e2008
    ldr r1, [r0]
    orr r1, r1, #0x0D
    str r1, [r0]

    ldr r0, =0x1e6e200C
    ldr r1, [r0]
#ifdef CONFIG_AST1500_CLIENT
    orr r1, r1, #0x0130
#else
    /* Host need to enable internal VGA. */
    orr r1, r1, #0x0100
#endif
    str r1, [r0]

    /* Enable 12MHz clock output for audio, client side only */
    ldr r0, =0x1e6e2010
    ldr r1, =0x00000010
    str r1, [r0]

    ldr r0, =0x1e6e2020
#ifdef CONFIG_AST1500_CLIENT
    ldr r1, =0x00034120     //12MHz   //Host: 0x00014120
#else
    ldr r1, =0x00014120     //12MHz
#endif
    str r1, [r0]

#if CONFIG_AST1500_CLIENT //overclock
    //Set CPU clock
    ldr r0, =0x1e6e2024
    //ldr r1, =0x00044120    //266MHZ
    //ldr r1, =0x000442A1    //275MHZ
    //ldr r1, =0x00044140    //288MHZ
    //ldr r1, =0x000442E1    //300MHZ
    ldr r1, =0x00044160    //312MHZ
    //ldr r1, =0x00044321    //324MHZ
    //ldr r1, =0x00044180    //336MHZ
    //ldr r1, =0x00044361    //350MHZ
    str r1, [r0]
#else
    //AHB/3
    //ldr r0, =0x1e6e2070
    //ldr r1, =0x00013002
    //str r1, [r0]

    //Set CPU clock
    ldr r0, =0x1e6e2024
    //ldr r1, =0x00044120    //266MHZ
    //ldr r1, =0x000442A1    //275MHZ
    //ldr r1, =0x00044140    //288MHZ
    //ldr r1, =0x000442E1    //300MHZ
    ldr r1, =0x00044160    //312MHZ
    //ldr r1, =0x00044321    //324MHZ
    //ldr r1, =0x00044180    //336MHZ
    //ldr r1, =0x00044361    //350MHZ
    str r1, [r0]
#endif

    ldr r0, =0x1e6e202C
#ifdef CONFIG_AST1500_CLIENT
    ldr r1, =0x00000012 //For C-Media OSCCLK
#else
    ldr r1, =0x00000010 //For GPIOD7
#endif
    str r1, [r0]

    //This defualt value should be fine for PTV platform
    ldr r0, =0x1e6e2074
#ifdef CONFIG_AST1500_CLIENT
    ldr r1, =0x0
#else
    ldr r1, =0x00048000 //Enable Internal VGA path
#endif
    str r1, [r0]

    /* DRAM Setting */
#ifdef	CONFIG_AST1500_CLIENT	/* DDRII DDRII 8Mbx16IOx4bank*2 */
    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e006C
    ldr r1, =0x00909090
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x00070000
    str r1, [r0]

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000C8D
    str r1, [r0]
      
    ldr r0, =0x1e6e0008
    ldr r1, =0x00000000		/* CRT */
    str r1, [r0]

    ldr r0, =0x1e6e0010
    //ldr r1, =0x32302926 /* For 266MHz CPU */
    ldr r1, =0x23202A27 /* For 312MHz CPU */
    //ldr r1, =0x33302A27 /* For 324MHz CPU */
    //ldr r1, =0x33302B27 /* For 336MHz CPU */
    //ldr r1, =0x33302B27 /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0018
    //ldr r1, =0x274c0122 /* For 266MHz CPU */
    ldr r1, =0x2B6E0126 /* For 312MHz CPU */
    //ldr r1, =0x2D6E0128 /* For 324MHz CPU */
    //ldr r1, =0x2F6F0129 /* For 336MHz CPU */
    //ldr r1, =0x316F012B /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0020
    //ldr r1, =0x00ce2222 /* For 266MHz CPU */
    ldr r1, =0x00D22222 /* For 312MHz CPU */
    //ldr r1, =0x00D22222 /* For 324MHz CPU */
    //ldr r1, =0x00D22222 /* For 336MHz CPU */
    //ldr r1, =0x00D22222 /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x01001523
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x1024010d
    str r1, [r0]

    ldr r0, =0x1e6e0024
    ldr r1, =0x00cb2522
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0058
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e005c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0060
    ldr r1, =0x0f2AA02a
    str r1, [r0]
                 
    ldr r0, =0x1e6e0064
    ldr r1, =0x003F3005
    str r1, [r0]
 
    ldr r0, =0x1e6e0068
    ldr r1, =0x00000000
    str r1, [r0]
 
    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0074
    ldr r1, =0x00000000
    str r1, [r0]
  
    ldr r0, =0x1e6e0078
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e007c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]
   
    /* Delay about 400us */
    ldr r2, =0x00000400 
delay0:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0	 
	
    ldr r0, =0x1e6e002c
    ldr r1, =0x00000942
    str r1, [r0]
 
    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A08
    str r1, [r0]

    ldr r0, =0x1e6e002c
    //ldr r1, =0x00000842 /* For 266MHz CPU */
    ldr r1, =0x00000852 /* For 312MHz CPU */
    //ldr r1, =0x00000852 /* For 324MHz CPU */
    //ldr r1, =0x00000A52 /* For 336MHz CPU */
    //ldr r1, =0x00000A52 /* For 350MHz CPU */
    str r1, [r0]
  
    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x000003c0
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A21
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00007C03
    str r1, [r0]
#else	/* DDRII 8Mbx16IOx4bank*2 */

    ldr r0, =0x1e6e0000
    ldr r1, =0xfc600309
    str r1, [r0]

    ldr r0, =0x1e6e006C
    ldr r1, =0x00909090
    str r1, [r0]

    ldr r0, =0x1e6e0064
    ldr r1, =0x00070000
    str r1, [r0]

    ldr r0, =0x1e6e0004
    ldr r1, =0x00000C8D
    str r1, [r0]
      
    ldr r0, =0x1e6e0008
    //ldr r1, =0x00000000		/* CRT */
    ldr r1, =0x0011030f		/* VGA */        
    str r1, [r0]

    ldr r0, =0x1e6e0010
    //ldr r1, =0x32302926 /* For 266MHz CPU */
    ldr r1, =0x23202A27 /* For 312MHz CPU */
    //ldr r1, =0x33302A27 /* For 324MHz CPU */
    //ldr r1, =0x33302B27 /* For 336MHz CPU */
    //ldr r1, =0x33302B27 /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0018
    //ldr r1, =0x274c0122 /* For 266MHz CPU */
    ldr r1, =0x2B6E0126 /* For 312MHz CPU */
    //ldr r1, =0x2D6E0128 /* For 324MHz CPU */
    //ldr r1, =0x2F6F0129 /* For 336MHz CPU */
    //ldr r1, =0x316F012B /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0020
    //ldr r1, =0x00ce2222 /* For 266MHz CPU */
    ldr r1, =0x00D22222 /* For 312MHz CPU */
    //ldr r1, =0x00D22222 /* For 324MHz CPU */
    //ldr r1, =0x00D22222 /* For 336MHz CPU */
    //ldr r1, =0x00D22222 /* For 350MHz CPU */
    str r1, [r0]

    ldr r0, =0x1e6e0014
    ldr r1, =0x01001523
    str r1, [r0]

    ldr r0, =0x1e6e001c
    ldr r1, =0x1024010d
    str r1, [r0]

    ldr r0, =0x1e6e0024
    ldr r1, =0x00cb2522
    str r1, [r0]

    ldr r0, =0x1e6e0038
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e003c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0040
    ldr r1, =0x00000000
    //ldr r1, =0x44444444 /* Optimize grant length */
    str r1, [r0]

    ldr r0, =0x1e6e0044
    ldr r1, =0x00000000
    //ldr r1, =0x44444444 /* Optimize grant length */
    str r1, [r0]

    ldr r0, =0x1e6e0048
    ldr r1, =0x00000000
    //ldr r1, =0x44444444 /* Optimize grant length */
    str r1, [r0]

    ldr r0, =0x1e6e004c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0050
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0054
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0058
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e005c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0060
    ldr r1, =0x0f2AA02a
    str r1, [r0]
                 
    ldr r0, =0x1e6e0064
    ldr r1, =0x003F3005
    str r1, [r0]
 
    ldr r0, =0x1e6e0068
    ldr r1, =0x00000000
    str r1, [r0]
 
    ldr r0, =0x1e6e0070
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0074
    ldr r1, =0x00000000
    str r1, [r0]
  
    ldr r0, =0x1e6e0078
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e007c
    ldr r1, =0x00000000
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00000001
    str r1, [r0]
   
    /* Delay about 400us */
    ldr r2, =0x00000400 
delay0:	
    nop
    nop
    subs r2, r2, #1          /* until source end addreee [r2]    */
    bne delay0	 
	
    ldr r0, =0x1e6e002c
    ldr r1, =0x00000942
    str r1, [r0]
 
    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000005
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000007
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e000c
    ldr r1, =0x00005A08
    str r1, [r0]

    ldr r0, =0x1e6e002c
    //ldr r1, =0x00000842 /* For 266MHz CPU */
    ldr r1, =0x00000852 /* For 312MHz CPU */
    //ldr r1, =0x00000852 /* For 324MHz CPU */
    //ldr r1, =0x00000A52 /* For 336MHz CPU */
    //ldr r1, =0x00000A52 /* For 350MHz CPU */
    str r1, [r0]
  
    ldr r0, =0x1e6e0028
    ldr r1, =0x00000001
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x000003c0
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e0030
    ldr r1, =0x00000040
    str r1, [r0]

    ldr r0, =0x1e6e0028
    ldr r1, =0x00000003
    str r1, [r0]

    ldr r0, =0x1e6e000c
    //ldr r1, =0x00005A21
    ldr r1, =0x00005C22 /* Refresh Optimize */
    str r1, [r0]

    ldr r0, =0x1e6e0034
    ldr r1, =0x00007c03
    str r1, [r0]

    ldr r0, =0x1e6e0120			/* for AST2000 compatible */
    ldr r1, =0x00005061			/* 266MHz */
    str r1, [r0]

#endif /* CONFIG_AST1500_CLIENT */


    /*Set Scratch register Bit 6 after ddr initial finished */
    ldr r0, =0x1e6e2040
    ldr r1, [r0]
    orr r1, r1, #0x40
    str r1, [r0]

platform_exit:
    mov lr, r4		/* restore lr */
 
    /* back to arch calling code */    
    mov pc, lr

