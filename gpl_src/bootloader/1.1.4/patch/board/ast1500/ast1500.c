/*
 * Board initialize for ASPEED AST1500
 *
 *
 */

#include <common.h>
#include <command.h>
#include <pci.h>

int board_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;
    unsigned char data;
    unsigned long reg;

    /* AHB Controller */
    *((volatile ulong*) 0x1E600000)  = 0xAEED1A03;	/* unlock AHB controller */ 
    *((volatile ulong*) 0x1E60008C) |= 0x01;		/* map DRAM to 0x00000000 */
#ifdef CONFIG_PCI
    *((volatile ulong*) 0x1E60008C) |= 0x30;		/* map PCI */
#endif

    /* Flash Controller */
    *((volatile ulong*) 0x16000000) |= 0x00001c00;	/* enable Flash Write */ 

    /* SCU */
    *((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;	/* unlock SCU */ 
    reg = *((volatile ulong*) 0x1e6e2008);		/* LHCLK = HPLL/8 */
    reg &= 0x1c0fffff;                                  /* PCLK  = HPLL/8 */
    reg |= 0x61b00000;					/* BHCLK = HPLL/8 */
    //reg |= 0x61c00000;					/* BHCLK = HPLL/10 */
    *((volatile ulong*) 0x1e6e2008) = reg;

#if 0 //#ifdef	CONFIG_AST1500_CLIENT. This is used for 32MB flash.
    *((volatile ulong*) 0x1e6e2074) |= 0x00000001;	/* enable Flash ROMA24 */ 
#endif

    /* arch number */
    gd->bd->bi_arch_number = MACH_TYPE_AST1500;
                                                                                                                             
    /* adress of boot parameters */
    gd->bd->bi_boot_params = 0x40000100;
                                                                                                                             
    /* Initialize LED. Turn off Power LED (GPIOB7). */
	/*hold host CAT 6613 in reset state (GPIOD5) to disable Tx*/
	reg = *((volatile ulong*) 0x1e780004);
	reg |= 0x20008000;
	*((volatile ulong*) 0x1e780004) = reg;

	reg = *((volatile ulong*) 0x1e780000);
	reg &= ~0x20008000;
	*((volatile ulong*) 0x1e780000) = reg;
	return 0;
}

int dram_init (void)
{
    DECLARE_GLOBAL_DATA_PTR;
                                                                                                                             
    gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
    gd->bd->bi_dram[0].size  = PHYS_SDRAM_1_SIZE;
                                                                                                                 
    return 0;
}


int misc_init_r(void)
{
    unsigned int reg1, reg2;

    /* Show H/W Version */
    reg1 = (unsigned int) (*((ulong*) 0x1e6e207c));
    reg2 = (unsigned int) (*((ulong*) 0x1e6e2070) >> 24);
    puts ("H/W:   ");
    printf("Rev. %02x \n", reg1);    
    		
#ifdef	CONFIG_PCI
    // Bruce100512. Some HW configuration don't need PCI and running
    // aspeed_init_pci() will halt uboot if HW grounded all PCI pins together.
    // We have to check the trpping pin to avoid this condition.
    reg1 = *((volatile ulong*) 0x1e6e2070) & 0x00000010;
    if (reg1)
    {
        puts("Enable PCI Host.\n");
        pci_init ();    
    }
#endif

    if (getenv ("eeprom") == NULL) {
	setenv ("eeprom", "n");
    }
}


#ifdef	CONFIG_PCI
static struct pci_controller hose;

extern void aspeed_init_pci (struct pci_controller *hose);

void pci_init_board(void)
{
    aspeed_init_pci(&hose);
}
#endif
