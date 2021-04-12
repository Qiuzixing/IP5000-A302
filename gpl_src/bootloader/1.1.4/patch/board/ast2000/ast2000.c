/*
 * Board initialize code for SOCLE Evaluation board LeopardII_A.
 *
 * (C) Peter Chen <peterc@socle-tech.com.tw> 2004
 *
 */

#include <common.h>
#include <command.h>

int board_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;
    unsigned char data;
    unsigned long reg;
           
    /* memory remap*/
    *((volatile ulong*)0x1E8E000C) = 0xdeadbeef;
    /* Setup Vendor ID*/
    *((volatile ulong*)0x1E6E0100) = 0xa8;
    reg = *((volatile ulong*)0x1E6E017C);
    if (!(reg & 0x01))
    {
        *((volatile ulong*)0x1E6E0130) = 0x20001A03;
        *((volatile ulong*)0x1E6E0134) = 0x20001A03;
    }
                                                                                                                             
    /* detect video capture enable , then set to scratch register*/
    /*
    Seeprom_read (OFFSET(OBJ_DEFINE, kvm_enable), &data, 1);
    if (data & 0x01)
        *((volatile ulong*)0x1E6E0140) |= 0x08;
    else
        *((volatile ulong*)0x1E6E0140) &= ~0x08;
    */                                                                                                                         
                                                                                                                             
    /* arch number of Leopard2A Board */
    gd->bd->bi_arch_number = MACH_TYPE_AST2000;
                                                                                                                             
    /* adress of boot parameters */
    gd->bd->bi_boot_params = 0x40000100;
                                                                                                                             
    return 0;
}

int dram_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;
                                                                                                                             
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
    /*
    gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
    gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
    */
                                                                                                                             
    return 0;
}
