/*
 * Board initialize for ASPEED AST1500
 *
 *
 */

#include <common.h>
#include <command.h>
#include <pci.h>

#define GPIO_MUX_SCU90 			0x1E6E2090
#define GPIO_MUX_SCU84 			0x1E6E2084
#define GPIO_MUX_SCU94 			0x1E6E2094
#define GPIO_MUX_STRAP 			0x1e6e2070

#define GPIOE_H_DIR_REG			0x1e780024
#define GPIOE_H_DATA_REG		0x1e780020

#define GPIOI_L_DIR_REG			0x1e780074
#define GPIOI_L_DATA_REG		0x1e780070

typedef enum
{
	BIT_CLEAR = 0,
	BIT_SET,
}bit_flag;

static void register_opeartion(unsigned long gpio_mux_reg,unsigned char bit,bit_flag  flag)
{
	unsigned long reg;
	reg = *((volatile ulong*) gpio_mux_reg);
	if(flag == BIT_CLEAR)
	{
		reg &= ~(1UL << bit);
	}
	else if(flag == BIT_SET)
	{
		reg |= (1UL << bit);
	}
	else
	{
		
	}
	*((volatile ulong*) gpio_mux_reg) = reg;
}

int board_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	unsigned char data;
	unsigned long reg;

#if (CONFIG_AST1500_SOC_VER >= 3)
	*((volatile ulong*) 0x1e620000) |= 0x00030000;	/* enable Flash Write */
#else
	*((volatile ulong*) 0x16000000) |= 0x00001c00;	/* enable Flash Write */
#endif    

	/* SCU */
	*((volatile ulong*) 0x1e6e2000) = 0x1688A8A8;	/* unlock SCU */ 
	reg = *((volatile ulong*) 0x1e6e2008);
	reg &= ~(0x7UL << 23);
	reg |= (0x7UL << 23); /* PCLK  = HPLL/32 */
	*((volatile ulong*) 0x1e6e2008) = reg;

	/* arch number */
	gd->bd->bi_arch_number = MACH_TYPE_AST1500;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x80000100;

#if (CONFIG_AST1500_SOC_VER >= 3)
	/* pull up rtl8364_reset (GPIOI5)*/
	register_opeartion(GPIO_MUX_SCU90,6,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,13,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,12,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,5,BIT_CLEAR);

	register_opeartion(GPIOI_L_DIR_REG,5,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,5,BIT_SET);
	/* pull up POWERON_1V8 (GPIOH3)*/
	register_opeartion(GPIO_MUX_SCU90,6,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU90,7,BIT_CLEAR);

	register_opeartion(GPIOE_H_DIR_REG,27,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,27,BIT_SET);
	/* pull up POWERON_1V2 (GPIOH4)*/
	register_opeartion(GPIOE_H_DIR_REG,28,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,28,BIT_SET);
	/* pull up POWERON_1V3 (GPIOH6)*/
	register_opeartion(GPIOE_H_DIR_REG,30,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,30,BIT_SET);

	/* turn on all led */
	register_opeartion(GPIO_MUX_SCU84,9,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,9,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,9,BIT_CLEAR);/* led_link_b -- GPIOJ1 */

	register_opeartion(GPIO_MUX_SCU84,10,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,10,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,10,BIT_CLEAR);/* led_link_g -- GPIOJ2 */

	register_opeartion(GPIO_MUX_SCU84,11,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,11,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,11,BIT_CLEAR);/* led_link_r -- GPIOJ3 */

	register_opeartion(GPIO_MUX_SCU90,6,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU84,0,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,16,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,16,BIT_CLEAR);/* led_status_b -- GPIOG0 */

	register_opeartion(GPIO_MUX_SCU84,1,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,17,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,17,BIT_CLEAR);/* led_status_g -- GPIOG1 */

	register_opeartion(GPIO_MUX_SCU84,2,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,18,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,18,BIT_CLEAR);/* led_status_r -- GPIOG2 */

	register_opeartion(GPIO_MUX_SCU84,3,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,19,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,19,BIT_CLEAR);/* led_on_g -- GPIOG3 */

	register_opeartion(GPIO_MUX_SCU94,12,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU84,4,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,20,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,20,BIT_CLEAR);/* led_on_r -- GPIOG4 */

	register_opeartion(GPIO_MUX_SCU84,5,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,21,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,21,BIT_CLEAR);/* led_on_b -- GPIOG5 */
#elif (CONFIG_AST1500_SOC_VER == 2) //This is actually board dependent.
    /* Initialize LED. Turn off Power LED (GPIOP5). */
	reg = *((volatile ulong*) 0x1e78007C);
	reg |= (1UL << 29);
	*((volatile ulong*) 0x1e78007C) = reg;

	reg = *((volatile ulong*) 0x1e780078);
	reg &= ~(1UL << 29);
	*((volatile ulong*) 0x1e780078) = reg;

	/*hold host CAT 6613 in reset state (GPIOJ2) to disable Tx*/
	reg = *((volatile ulong*) 0x1e780074);
	reg |= (1UL << 10);
	*((volatile ulong*) 0x1e780074) = reg;

	reg = *((volatile ulong*) 0x1e780070);
	reg &= ~(1UL << 10);
	*((volatile ulong*) 0x1e780070) = reg;
#elif (CONFIG_AST1500_SOC_VER == 1)
	/* Initialize LED. Turn off Power LED (GPIOB7). */
	/*hold host CAT 6613 in reset state (GPIOD5) to disable Tx*/
	reg = *((volatile ulong*) 0x1e780004);
	reg |= 0x20008000;
	*((volatile ulong*) 0x1e780004) = reg;

	reg = *((volatile ulong*) 0x1e780000);
	reg &= ~0x20008000;
	*((volatile ulong*) 0x1e780000) = reg;
#endif

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
	unsigned int reg1;

	/* Show H/W Version */
	reg1 = (unsigned int) (*((ulong*) 0x1e6e207c));
	puts ("H/W:   ");
	printf("Rev. %08X \n", reg1);    

#ifdef	CONFIG_PCI
	// Bruce100512. Some HW configuration don't need PCI and running
	// aspeed_init_pci() will halt uboot if HW grounded all PCI pins together.
	// We have to check the trpping pin to avoid this condition.
#if (CONFIG_AST1500_SOC_VER >= 3)
	reg1 = *((volatile ulong*) 0x1e6e2070) & (1UL<<4);
#elif (CONFIG_AST1500_SOC_VER == 2)
	#if 0//AST1510 A0
	reg1 = *((volatile ulong*) 0x1e6e2070) & (1UL<<26);
	#else
	reg1 = *((volatile ulong*) 0x1e6e2070) & (1UL<<23);
	#endif
#elif (CONFIG_AST1500_SOC_VER == 1)
	reg1 = *((volatile ulong*) 0x1e6e2070) & 0x00000010;
#endif

	if (reg1) {
		puts("Enable PCI Host.\n");
		pci_init ();    
	}
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
	/*
	** TODO:Reset RTL8211FS PHY.
	** udelay() is not working in board_init(). So, I put PHY reset code in misc_init_r()
	*/
	if (0) {
		unsigned int reg;
		//Output GPIOE1 to 0
		reg = *((volatile ulong*) 0x1e780020);
		reg &= ~(1UL << 1);
		*((volatile ulong*) 0x1e780020) = reg;
		reg = *((volatile ulong*) 0x1e780024);
		reg |= (1UL << 1);
		*((volatile ulong*) 0x1e780024) = reg;
		//delay at least 10ms
		udelay(20*1000);
		//Output GPIOE1 to 1
		reg = *((volatile ulong*) 0x1e780020);
		reg |= (1UL << 1);
		*((volatile ulong*) 0x1e780020) = reg;
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
