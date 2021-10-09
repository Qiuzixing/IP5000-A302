/*
 * Board initialize for ASPEED AST1500
 *
 *
 */

#include <common.h>
#include <command.h>
#include <pci.h>

#define GPIO_MUX_SCU80 			0x1E6E2080
#define GPIO_MUX_SCU8C 			0x1E6E208c
#define GPIO_MUX_SCU90 			0x1E6E2090
#define GPIO_MUX_SCU84 			0x1E6E2084
#define GPIO_MUX_SCU94 			0x1E6E2094
#define GPIO_MUX_STRAP 			0x1e6e2070
#define GPIO_MUX_SCUA4 			0x1E6E20a4

#define GPIOE_H_DIR_REG			0x1e780024
#define GPIOE_H_DATA_REG		0x1e780020

#define GPIOI_L_DIR_REG			0x1e780074
#define GPIOI_L_DATA_REG		0x1e780070

#define GPIOA_D_DIR_REG			0x1e780004
#define GPIOA_D_DATA_REG		0x1e780000

#define IPE5000_BOARD_NAME		"KDS-EN7"
#define IPE5000P_BOARD_NAME		"KDS-SW3-EN7"
#define IPE5000W_BOARD_NAME		"WP-SW2-EN7"
#define IPD5000_BOARD_NAME		"KDS-DEC7"
#define IPD5000W_BOARD_NAME		"WP-DEC7"

typedef enum
{
	BIT_CLEAR = 0,
	BIT_SET,
}bit_flag;

typedef enum
{
	IPE5000 = 0,
	IPE5000P,
	IPE5000W,
	IPD5000,
	IPD5000W,
}a30_board_type;

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

static void a30_led_control(void)
{
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
}

static void a30_tx_led_control(void)
{
	a30_led_control();
	/* pull up GPIOD4 */
	/* 2021.10.09 qzx: pull down hpd pin（connect to gsv）,let it plot in the kernel */
	register_opeartion(GPIO_MUX_SCU90,1,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU8C,10,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,21,BIT_CLEAR);
	register_opeartion(GPIOA_D_DIR_REG,28,BIT_SET);
	register_opeartion(GPIOA_D_DATA_REG,28,BIT_CLEAR);
}

static void IPE5000W_GPIO_SET(void)
{
	/* i2c2_scl(GPIOY6) mux gpio */
	/* 2021.10.09 qzx: Configure the default I2C to GPIO mode,let the LED driver framework of the kernel operate it  */
	register_opeartion(GPIO_MUX_SCUA4,14,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCUA4,15,BIT_CLEAR);

	/* pull up LED_ON (GPIOG4) */
	register_opeartion(GPIO_MUX_SCU94,12,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU84,4,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,20,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,20,BIT_CLEAR);

	/* pull up LED_ON (GPIOG5) */
	register_opeartion(GPIO_MUX_SCU84,5,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,21,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,21,BIT_CLEAR);
}

static void IPD5000W_GPIO_SET(void)
{
	/* pull up LED_HDMI (GPIOE2)*/
	register_opeartion(GPIO_MUX_SCU80,18,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU8C,13,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,22,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,2,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,2,BIT_CLEAR);

	/* pull up LED_ON_R (GPIOE3) */
	register_opeartion(GPIO_MUX_SCU80,19,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,3,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,3,BIT_CLEAR);

	/* pull up LED_ON_G (GPIOI7) */
	register_opeartion(GPIO_MUX_SCU90,6,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,12,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,13,BIT_CLEAR);
	register_opeartion(GPIO_MUX_STRAP,5,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,7,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,7,BIT_CLEAR);

	/* Light up the nixie tube*/
	register_opeartion(GPIO_MUX_SCU84,9,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,9,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,9,BIT_CLEAR);/* led_power1 -- GPIOJ1 */

	register_opeartion(GPIO_MUX_SCU84,10,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,10,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,10,BIT_CLEAR);/* led_power2 -- GPIOJ2 */

	register_opeartion(GPIO_MUX_SCU84,11,BIT_CLEAR);
	register_opeartion(GPIOI_L_DIR_REG,11,BIT_SET);
	register_opeartion(GPIOI_L_DATA_REG,11,BIT_CLEAR);/* led_power3 -- GPIOJ3 */

	register_opeartion(GPIO_MUX_SCU90,6,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU84,1,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,17,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,17,BIT_CLEAR);/* SEG_LEDE -- GPIOG1 */

	register_opeartion(GPIO_MUX_SCU84,2,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,18,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,18,BIT_CLEAR);/* SEG_LEDF -- GPIOG2 */

	register_opeartion(GPIO_MUX_SCU84,3,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,19,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,19,BIT_CLEAR);/* SEG_LEDG -- GPIOG3 */

	register_opeartion(GPIO_MUX_SCU94,12,BIT_CLEAR);
	register_opeartion(GPIO_MUX_SCU84,4,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,20,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,20,BIT_CLEAR);/* SEG_LEDA -- GPIOG4 */

	register_opeartion(GPIO_MUX_SCU84,5,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,21,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,21,BIT_CLEAR);/* SEG_LEDB -- GPIOG5 */

	register_opeartion(GPIO_MUX_SCU84,6,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,22,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,22,BIT_CLEAR);/* SEG_LEDC -- GPIOG6 */

	register_opeartion(GPIO_MUX_SCU84,7,BIT_CLEAR);
	register_opeartion(GPIOE_H_DIR_REG,23,BIT_SET);
	register_opeartion(GPIOE_H_DATA_REG,23,BIT_CLEAR);/* SEG_LEDD -- GPIOG7 */
}

static void a30_ip5000_gpio_set(a30_board_type board_type)
{
	switch(board_type)
	{
		case IPE5000:
		case IPE5000P:
			a30_tx_led_control();
			break;
		case IPD5000:
			a30_led_control();
			break;
		case IPE5000W:
			IPE5000W_GPIO_SET();
			break;
		case IPD5000W:
			IPD5000W_GPIO_SET();
			break;
		default:
			break;
	}
}

int a30_led_init (void)
{	
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

	char *board_type = getenv("model_number");
	//The configuration information of GPIO is the same in IPE5000P、IPE5000、IPD5000
	if(0 == strcmp(board_type,IPE5000_BOARD_NAME) || 0 == strcmp(board_type,IPE5000P_BOARD_NAME))
	{
		a30_ip5000_gpio_set(IPE5000);
	}
	else if(0 == strcmp(board_type,IPD5000_BOARD_NAME))
	{
		a30_ip5000_gpio_set(IPD5000);
	}
	else if(0 == strcmp(board_type,IPE5000W_BOARD_NAME))
	{
		a30_ip5000_gpio_set(IPE5000W);
	}
	else if(0 == strcmp(board_type,IPD5000W_BOARD_NAME))
	{
		a30_ip5000_gpio_set(IPD5000W);
	}
	else
	{
		printf("Warning:please set right model_number >_<\n");
		return 0;
	}

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
