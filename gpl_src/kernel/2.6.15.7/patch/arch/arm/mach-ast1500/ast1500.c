/*
 *  linux/arch/arm/mach-ast2000/ast2000.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#ifdef CONFIG_SPI_ASTSMC
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#endif
#endif
#include <linux/config.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/io.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include <asm/arch/gpio.h>
#include <asm/arch/leds-gpio.h>

#include <asm/arch/drivers/board_def.h>

#ifdef CONFIG_SPI_ASTSMC
#include <asm/arch/astsmc_spi.h>
#endif

#include <asm/arch/ast-scu.h>

extern void aspeed_map_io(void);
extern void aspeed_init_irq(void);
extern struct sys_timer aspeed_timer;

static struct ast_led_platdata ast1500_pdata_ledb4 = {
	.gpio		= GPIO_BUTTON1,
	.flags		= AST_LEDF_BUTTON,
	.name		= "button_link",
	.def_trigger    = "pushbutton"
};

static struct ast_led_platdata ast1500_pdata_ledb5 = {
	.gpio		= LED_LINK_B,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_link_b",
};

static struct ast_led_platdata ast1500_pdata_ledb6 = {
	.gpio		= LED_LINK_G,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_link_g",
};

static struct ast_led_platdata ast1500_pdata_ledb7 = {
	.gpio		= LED_LINK_R,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_link_r",
};

static struct ast_led_platdata ast1500_pdata_ledg0 = {
	.gpio		= LED_STATUS_B,
	.flags		=AST_LEDF_DEFAULT_ON  ,
	.name		= "led_status_b",
};

static struct ast_led_platdata ast1500_pdata_ledg1 = {
	.gpio		= LED_STATUS_G,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_status_g",
};

static struct ast_led_platdata ast1500_pdata_ledg2 = {
	.gpio		= LED_STATUS_R,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_status_r",
};

static struct ast_led_platdata ast1500_pdata_ledg3 = {
	.gpio		= LED_ON_B,
	.flags		= AST_LEDF_DEFAULT_ON  ,
	.name		= "led_on_b",
};

static struct ast_led_platdata ast1500_pdata_ledg4 = {
	.gpio		= LED_ON_G,
	.flags		=  AST_LEDF_DEFAULT_ON,
	.name		= "led_on_g",
};

static struct ast_led_platdata ast1500_pdata_ledg5 = {
	.gpio		= LED_ON_R,
	.flags		=  AST_LEDF_DEFAULT_ON,
	.name		= "led_on_r",
};

static struct ast_led_platdata ast1500_pdata_poweron_1V8 = {
	.gpio		= POWERON_1V8,
	.flags		=  AST_LEDF_DEFAULT_ON,
	.name		= "poweron_1V8",
};

static struct ast_led_platdata ast1500_pdata_poweron_1V2 = {
	.gpio		= POWERON_1V2,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "poweron_1V2",
};

static struct ast_led_platdata ast1500_pdata_poweron_1V3 = {
	.gpio		= POWERON_1V3,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "poweron_1V3",
};

static struct ast_led_platdata ast1500_pdata_ledf0 = {
	.gpio		= LINEIN_MUTE,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "linein_mute",
};

static struct ast_led_platdata ast1500_pdata_ledf1 = {
	.gpio		= LINEOUT_MUTE,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "lineout_mute",
};

static struct ast_led_platdata ast1500_pdata_ledf2 = {
	.gpio		= AUDIO_SENSITIVE_HIGH,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "audio_sensitive_high",
};

static struct ast_led_platdata ast1500_pdata_ledf3 = {
	.gpio		= AUDIO_SENSITIVE_MIDDLE,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "audio_sensitive_middle",
};

static struct ast_led_platdata ast1500_pdata_ledh7 = {
	.gpio		= LCD_POWER,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "lcd_power",
};

static struct ast_led_platdata ast1500_pdata_ledi4 = {
	.gpio		= MCU_RESET,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "mcu_reset",
};

static struct ast_led_platdata ast1500_pdata_ledd3 = {
	.gpio		= AST1500_GPD3,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "EN_USB1",
};

static struct ast_led_platdata ast1500_pdata_ledd4 = {
	.gpio		= AST1500_GPD4,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "EN_USB2",
};

static struct ast_led_platdata ast1500_pdata_ledi5 = {
	.gpio		= RTL_RESET,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "rtl_reset",
};

static struct ast_led_platdata ast1500_pdata_ledh0 = {
	.gpio		= GPIO_CH0,
	.flags		= AST_LEDF_SWITCH | ((GPIO_CH0_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "ch0",
};

static struct ast_led_platdata ast1500_pdata_ledh1 = {
	.gpio		= GPIO_CH1,
	.flags		= AST_LEDF_SWITCH | ((GPIO_CH1_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "ch1",
};

static struct ast_led_platdata ast1500_pdata_ledh2 = {
	.gpio		= GPIO_CH2,
	.flags		= AST_LEDF_SWITCH | ((GPIO_CH2_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "ch2",
};
#ifdef CONFIG_ARCH_AST1500_HOST
#if (BOARD_DESIGN_VER_VIDEO >= 300)
static struct ast_led_platdata ast1500_pdata_v_input = {
	.gpio		= GPIO_V_TYPE,
	.flags		= AST_LEDF_SWITCH | ((GPIO_V_TYPE_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "v_input",
	.def_trigger    = "switch",
};
#endif

#if (BOARD_DESIGN_VER_I2S >= 204)
static struct ast_led_platdata ast1500_pdata_a_input = {
	.gpio		= GPIO_AUDIO_CODEC_IN_HOTPLUG, /* 0:analog 1:digital */
	.flags		= AST_LEDF_SWITCH | ((GPIO_AUDIO_CODEC_IN_HOTPLUG_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "a_input",
	.def_trigger    = "switch",
};
#endif
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT

static struct ast_led_platdata ast1500_pdata_led5v = {
	.gpio		= AST1500_GPH2,
	.flags		= AST_LEDF_DEFAULT_ON,
	.name		= "gsv_5v",
};

#if (BOARD_DESIGN_VER_MISC >= 104)
//#if defined(CONFIG_AST1500_BOARD_V4) || defined (CONFIG_AST1500_BOARD_V5) || defined(CONFIG_AST1510_BOARD_EVA_V4)
static struct ast_led_platdata ast1500_pdata_leda3 = {
	.gpio		= GPIO_CH_UPDATE,
	.flags		= AST_LEDF_BUTTON | ((GPIO_CH_UPDATE_ACTIVE)?(0):(AST_LEDF_ACTLOW)),
	.name		= "button_chg_ch",
	.def_trigger	= "pushbutton",
};
#endif

static struct ast_led_platdata ast1500_pdata_leda5 = {
	.gpio		= GPIO_POWER_CTRL_IN,
	.flags		= AST_LEDF_BUTTON,
	.name		= "button_remote_ctrl",
	/*.def_trigger    = "pushbutton",*/
};

#if (BOARD_DESIGN_VER_MISC >= 105)
/* Define for Remote Power Status */
static struct ast_led_platdata ast1500_pdata_ledc3 = {
	.gpio		= GPIO_POWER_STATUS_OUT,
	.flags		= AST_LEDF_TRISTATE_OFF,
	.name		= "gpio_pwr_status_out",
};
#endif

#else //#ifdef CONFIG_ARCH_AST1500_CLIENT
static struct ast_led_platdata ast1500_pdata_ledb2 = {
	.gpio		= GPIO_POWER_CTRL_OUT,
	.flags		= AST_LEDF_TRISTATE_OFF,
	.name		= "pwr_ctrl",
};

#if (BOARD_DESIGN_VER_MISC >= 105)
/* Define for Remote Power Status */
static struct ast_led_platdata ast1500_pdata_ledc6 = {
	.gpio		= GPIO_POWER_STATUS_IN,
	.flags		= AST_LEDF_SWITCH,
	.name		= "gpio_pwr_status_in",
};
#endif //#if (BOARD_DESIGN_VER_MISC >= 105)
#endif //#ifdef CONFIG_ARCH_AST1500_CLIENT

static struct platform_device ast_ledb4 = {
	.name		= "ast1500_led",
	.id		= 2,
	.dev		= {
		.platform_data = &ast1500_pdata_ledb4,
	},
};

static struct platform_device ast_ledb5 = {
	.name		= "ast1500_led",
	.id		= 3,
	.dev		= {
		.platform_data = &ast1500_pdata_ledb5,
	},
};

static struct platform_device ast_ledb6 = {
	.name		= "ast1500_led",
	.id		= 0,
	.dev		= {
		.platform_data = &ast1500_pdata_ledb6,
	},
};

static struct platform_device ast_ledb7 = {
	.name		= "ast1500_led",
	.id		= 1,
	.dev		= {
		.platform_data = &ast1500_pdata_ledb7,
	},
};

static struct platform_device ast_ledh0 = {
	.name		= "ast1500_led",
	.id		= 4,
	.dev		= {
		.platform_data = &ast1500_pdata_ledh0,
	},
};

static struct platform_device ast_ledh1 = {
	.name		= "ast1500_led",
	.id		= 5,
	.dev		= {
		.platform_data = &ast1500_pdata_ledh1,
	},
};

static struct platform_device ast_ledh2 = {
	.name		= "ast1500_led",
	.id		= 6,
	.dev		= {
		.platform_data = &ast1500_pdata_ledh2,
	},
};

static struct platform_device ast_poweron_1V8 = {
	.name		= "ast1500_led",
	.id		= 7,
	.dev		= {
		.platform_data = &ast1500_pdata_poweron_1V8,
	},
};

static struct platform_device ast_poweron_1V2 = {
	.name		= "ast1500_led",
	.id		= 19,
	.dev		= {
		.platform_data = &ast1500_pdata_poweron_1V2,
	},
};

static struct platform_device ast_poweron_1V3 = {
	.name		= "ast1500_led",
	.id		= 20,
	.dev		= {
		.platform_data = &ast1500_pdata_poweron_1V3,
	},
};

static struct platform_device ast_ledg0 = {
	.name		= "ast1500_led",
	.id		= 22,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg0,
	},
};

static struct platform_device ast_ledg1 = {
	.name		= "ast1500_led",
	.id		= 23,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg1,
	},
};

static struct platform_device ast_ledg2 = {
	.name		= "ast1500_led",
	.id		= 24,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg2,
	},
};

static struct platform_device ast_ledg3= {
	.name		= "ast1500_led",
	.id		= 25,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg3,
	},
};

static struct platform_device ast_ledg4 = {
	.name		= "ast1500_led",
	.id		= 26,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg4,
	},
};

static struct platform_device ast_ledg5 = {
	.name		= "ast1500_led",
	.id		= 27,
	.dev		= {
		.platform_data = &ast1500_pdata_ledg5,
	},
};

static struct platform_device ast_ledf0 = {
	.name		= "ast1500_led",
	.id		= 28,
	.dev		= {
		.platform_data = &ast1500_pdata_ledf0,
	},
};

static struct platform_device ast_ledf1 = {
	.name		= "ast1500_led",
	.id		= 29,
	.dev		= {
		.platform_data = &ast1500_pdata_ledf1,
	},
};

static struct platform_device ast_ledf2 = {
	.name		= "ast1500_led",
	.id		= 30,
	.dev		= {
		.platform_data = &ast1500_pdata_ledf2,
	},
};

static struct platform_device ast_ledf3 = {
	.name		= "ast1500_led",
	.id		= 31,
	.dev		= {
		.platform_data = &ast1500_pdata_ledf3,
	},
};

static struct platform_device ast_ledh7 = {
	.name		= "ast1500_led",
	.id		= 32,
	.dev		= {
		.platform_data = &ast1500_pdata_ledh7,
	},
};

static struct platform_device ast_ledi4 = {
	.name		= "ast1500_led",
	.id		= 33,
	.dev		= {
		.platform_data = &ast1500_pdata_ledi4,
	},
};

static struct platform_device ast_ledi5 = {
	.name		= "ast1500_led",
	.id		= 34,
	.dev		= {
		.platform_data = &ast1500_pdata_ledi5,
	},
};



#ifdef CONFIG_ARCH_AST1500_HOST
#if (BOARD_DESIGN_VER_VIDEO >= 300)
static struct platform_device ast_led_v_input = {
	.name		= "ast1500_led",
	.id		= 8,
	.dev		= {
		.platform_data = &ast1500_pdata_v_input,
	},
};
#endif
#if (BOARD_DESIGN_VER_I2S >= 204)
static struct platform_device ast_led_a_input = {
	.name		= "ast1500_led",
	.id		= 9,
	.dev		= {
		.platform_data = &ast1500_pdata_a_input,
	},
};
#endif
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT

static struct platform_device ast_led5v = {
	.name		= "ast1500_led",
	.id		= 35,
	.dev		= {
		.platform_data = &ast1500_pdata_led5v,
	},
};

static struct platform_device ast_ledd4 = {
	.name		= "ast1500_led",
	.id		= 36,
	.dev		= {
		.platform_data = &ast1500_pdata_ledd4,
	},
};

static struct platform_device ast_ledd3 = {
	.name		= "ast1500_led",
	.id		= 37,
	.dev		= {
		.platform_data = &ast1500_pdata_ledd3,
	},
};

#if (BOARD_DESIGN_VER_MISC >= 104)
//#if defined(CONFIG_AST1500_BOARD_V4) || defined (CONFIG_AST1500_BOARD_V5)
static struct platform_device ast_leda3 = {
	.name		= "ast1500_led",
	.id		= 14,
	.dev		= {
		.platform_data = &ast1500_pdata_leda3,
	},
};
#endif

static struct platform_device ast_leda5 = {
	.name		= "ast1500_led",
	.id		= 15,
	.dev		= {
		.platform_data = &ast1500_pdata_leda5,
	},
};

#if (BOARD_DESIGN_VER_MISC >= 105)
static struct platform_device ast_ledc3 = {
	.name		= "ast1500_led",
	.id		= 16,
	.dev		= {
		.platform_data = &ast1500_pdata_ledc3,
	},
};
#endif //#if (BOARD_DESIGN_VER_MISC >= 105)

#else //#ifdef CONFIG_ARCH_AST1500_CLIENT
static struct platform_device ast_ledb2 = {
	.name		= "ast1500_led",
	.id		= 17,
	.dev		= {
		.platform_data = &ast1500_pdata_ledb2,
	},
};

#if (BOARD_DESIGN_VER_MISC >= 105)
static struct platform_device ast_ledc6 = {
	.name		= "ast1500_led",
	.id		= 18,
	.dev		= {
		.platform_data = &ast1500_pdata_ledc6,
	},
};
#endif //#if (BOARD_DESIGN_VER_MISC >= 105)
#endif

#ifdef CONFIG_SPI_ASTSMC
static struct resource astsmc_spi_master_resources[] = {
	[0] = {
		.start = ASPEED_SMC_CONTROLLER_BASE,
		.end   = ASPEED_SMC_CONTROLLER_BASE + 0x2000000,
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device astsmc_spi_master = {
	.name = "astsmc-spi-master",//must match astsmc_spi_driver.driver.name
	.id = 0,
	.num_resources = ARRAY_SIZE(astsmc_spi_master_resources),
	.resource = astsmc_spi_master_resources,
};
#endif

/* devices we initialise */

static struct platform_device __initdata *ast_devs[] = {
	&ast_ledb4,
	&ast_ledb5,
	&ast_ledb6,
	&ast_ledb7,
	&ast_ledh0,
	&ast_ledh1,
	&ast_ledh2,
	&ast_poweron_1V8,
	&ast_poweron_1V2,
	&ast_poweron_1V3,
	&ast_ledg0,
	&ast_ledg1,
	&ast_ledg2,
	&ast_ledg3,
	&ast_ledg4,
	&ast_ledg5,
	&ast_ledf0,
	&ast_ledf1,
	&ast_ledf2,
	&ast_ledf3,
	&ast_ledh7,
	&ast_ledi4,
	&ast_ledi5,
#ifdef CONFIG_ARCH_AST1500_CLIENT
	&ast_led5v,
	&ast_ledd4,
	&ast_ledd3,
#if (BOARD_DESIGN_VER_MISC >= 104)
//#if defined(CONFIG_AST1500_BOARD_V4) || defined (CONFIG_AST1500_BOARD_V5)
	&ast_leda3,
#endif
	&ast_leda5,
#if (BOARD_DESIGN_VER_MISC >= 105)
	&ast_ledc3,
#endif //#if (BOARD_DESIGN_VER_MISC >= 105)

#else //#ifdef CONFIG_ARCH_AST1500_CLIENT
	&ast_ledb2,
#if (BOARD_DESIGN_VER_MISC >= 105)
	&ast_ledc6,
#endif //#if (BOARD_DESIGN_VER_MISC >= 105)
#endif //#ifdef CONFIG_ARCH_AST1500_CLIENT

#if defined(CONFIG_ARCH_AST1500_HOST) && (BOARD_DESIGN_VER_VIDEO >= 300)
	&ast_led_v_input,
#endif
#if defined(CONFIG_ARCH_AST1500_HOST) && (BOARD_DESIGN_VER_I2S >= 204)
	&ast_led_a_input,
#endif

#ifdef CONFIG_SPI_ASTSMC
	&astsmc_spi_master,
#endif
};

#ifdef CONFIG_SPI_ASTSMC
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
static struct mtd_partition ast1500_flash_partitions[] = {
	{
		.name = "bootloader",
		.size = 0x80000,
		.offset = 0,
		.mask_flags = MTD_CAP_ROM
	}, {
		.name = "firmware",
		.size = 0xF60000,
		.offset = 0x80000
	}, {
		.name = "param",
		.size = 0x10000,
		.offset = 0xFE0000,
	}, {
		.name = "ROparam",
		.size = 0x10000,
		.offset = 0xFF0000,
	}
};

static struct flash_platform_data ast1500_flash_data = {
	.name = "ast1500Flash",
	.parts = ast1500_flash_partitions,
	.nr_parts = ARRAY_SIZE(ast1500_flash_partitions),
};

static struct astsmc_spi_device astsmc_spi_flash = {
	.phy_dev_base = ASPEED_SMC_FLASH_BASE,
	.dev_len = 0x2000000,
};
#endif

/*
 * INTERFACE between board init code and SPI infrastructure.
 *
 * SPI devices can't be probed.  Instead, board init code
 * provides a table listing the devices which are present, with enough
 * information to bind and set up the device's driver.
 */
static struct spi_board_info astsmc_spi_board_info[] __initdata = {
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	{
		.modalias = "m25p80",       /*"modalias" is normally the driver name.*/
		.max_speed_hz = 104000000,     /* max spi clock (SCK) speed in HZ */
		.bus_num = 0,               /* bus_num matches the bus_num of some spi_master.*/
		.chip_select = 2,
		.platform_data = &ast1500_flash_data,/*platform_data goes to spi_device.dev.platform_data.*/
		.controller_data = &astsmc_spi_flash,/* controller_data goes to spi_device.controller_data.*/
	},
#endif
};
#endif

#if (CONFIG_AST1500_SOC_VER >= 3) && defined(CONFIG_ARCH_AST1500_CLIENT)
static u64 ehci_dmamask = 0xffffffff;

static struct resource aspeed_ehci1_resources[] = {
	{
		.start	= ASPEED_USB20HC1_BASE,
		.end	= ASPEED_USB20HC1_BASE + SZ_4K,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_USB20HC1,
		.end	= INT_USB20HC1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device aspeed_ehci1 = {
	.name		= "aspeed-ehci",
	.id		= 0,
	.dev		= {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= aspeed_ehci1_resources,
	.num_resources	= ARRAY_SIZE(aspeed_ehci1_resources),
};

static struct resource aspeed_ehci2_resources[] = {
	{
		.start	= ASPEED_USB20HC2_BASE,
		.end	= ASPEED_USB20HC2_BASE + SZ_4K,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_USB20HC2,
		.end	= INT_USB20HC2,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device aspeed_ehci2 = {
	.name		= "aspeed-ehci",
	.id		= 1,
	.dev		= {
		.dma_mask		= &ehci_dmamask,
		.coherent_dma_mask	= 0xffffffff,
	},
	.resource	= aspeed_ehci2_resources,
	.num_resources	= ARRAY_SIZE(aspeed_ehci2_resources),
};

#endif

static void __init aspeed_init(void)
{
	u32 r;
	struct ast_led_platdata *p;
	platform_add_devices(ast_devs, ARRAY_SIZE(ast_devs));
#ifdef CONFIG_SPI_ASTSMC
	spi_register_board_info(astsmc_spi_board_info, ARRAY_SIZE(astsmc_spi_board_info));
#endif
#if (CONFIG_AST1500_SOC_VER >= 3) && defined(CONFIG_ARCH_AST1500_CLIENT)
	ast_scu.scu_op(SCUOP_USB20HC_INIT, NULL);
	platform_device_register(&aspeed_ehci1);
	platform_device_register(&aspeed_ehci2);
#endif
}

MACHINE_START(AST1500, "ASPEED-AST1500")
	/* MAINTAINER("ASPEED Technology Inc.") */
	.phys_io		= IO_START,
	.phys_ram		= ASPEED_SDRAM_BASE,
	.io_pg_offst		= (IO_ADDRESS(IO_START)>>18) & 0xfffc,
	.map_io			= aspeed_map_io,
	.timer			= &aspeed_timer,
	.init_irq		= aspeed_init_irq,
	.init_machine		= aspeed_init,
MACHINE_END
