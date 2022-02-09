/*
 * xmiter code for internal dac
 */
#ifndef _XMITER_DAC_C_
#define _XMITER_DAC_C_

#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/util.h>
#include <asm/arch/regs-scu.h>
#include <asm/arch/drivers/crt.h>

#include <asm/arch/ast-scu.h>

#if (CONFIG_AST1500_SOC_VER == 1)
unsigned int xmiter_dac_exist(struct s_crt_drv *crt)
{
	//SoC built-in, Always exists, but _init_all_xmiter_host() need the return to be 0.
	return 0;
}

unsigned int xmiter_init_dac(struct s_crt_drv *crt)
{
	//TBD
	return 0;
}

unsigned int xmiter_disable_dac(struct s_crt_drv *crt)
{
	//TBD
	return 0;
}

unsigned int xmiter_setup_dac(struct s_crt_drv *crt, struct s_crt_info *info)
{
	//TBD
	return 0;
}

#else //#if (CONFIG_AST1500_SOC_VER == 1)

#if defined(CONFIG_ARCH_AST1500_CLIENT)
unsigned int xmiter_dac_exist(struct s_crt_drv *crt)
{
#if 0 //TBD. Maybe check trapping. If video is disable, then return 0......
	unsigned int iTemp;

	if (crt->dual_port) {
		return 1;
	}
	iTemp = (GetSCUReg(SCU_HW_TRAPPING_REG) & VIDEO_TYPE_MASK) >> VIDEO_TYPE_SHIFT;
	if (iTemp == VIDEO_TYPE_ANALOG) {
		return 1;
	}
	return 0;
#endif
	return 1;
}

unsigned int xmiter_disable_dac(struct s_crt_drv *crt)
{
	//Disable Sync
	ast_scu.scu_op(SCUOP_CRT_DAC_CTRL_DISABLE, NULL);
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DAC, 0);
//	ModSCUReg(SCU_PIN_CTRL2_REG, 0, BIT12|BIT13);
	//CRT64[0]: 0:disalbe, 1:enable
	ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60, PWR_OFF_DAC, PWR_ON_DAC_MASK);
	return 0;
}


unsigned int xmiter_init_dac(struct s_crt_drv *crt)
{
	return xmiter_disable_dac(crt);
}

unsigned int xmiter_setup_dac(struct s_crt_drv *crt, struct s_crt_info *info)
{
	if ((info->EnHDCP != HDCP_DISABLE) && (info->owner != OWNER_CON)) {
		/* disable dac to bloack HDCP content */
		uinfo("DAC disabled due to HDCP\n");
		return xmiter_disable_dac(crt);
	}

	ModSCUReg(CRT1_CONTROL2_REG + CRT_2*0x60, PWR_ON_DAC, PWR_ON_DAC_MASK);
	//Enable Sync
	ast_scu.scu_op(SCUOP_CRT_DAC_CTRL_FROM_CRT, NULL);
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DAC, 1);
//	ModSCUReg(SCU_PIN_CTRL2_REG, (BIT12|BIT13), BIT12|BIT13);
	return 0;
}

#else //#if defined(CONFIG_ARCH_AST1500_CLIENT)

#if (BOARD_DESIGN_VER_VIDEO >= 205)
unsigned int xmiter_dac_exist(struct s_crt_drv *crt)
{
#if 0 //TBD. Maybe check trapping. If video is disable, then return 0......
	unsigned int iTemp;

	if (crt->dual_port) {
		return 1;
	}
	iTemp = (GetSCUReg(SCU_HW_TRAPPING_REG) & VIDEO_TYPE_MASK) >> VIDEO_TYPE_SHIFT;
	if (iTemp == VIDEO_TYPE_ANALOG) {
		return 1;
	}
	return 0;
#endif
	return 1;
}

unsigned int xmiter_init_dac(struct s_crt_drv *crt)
{
	/* disable dac, turn off sync, disable bypass mode */
	ast_scu.scu_op(SCUOP_CRT_DAC_CTRL_DISABLE, NULL);

//	//Set multi-pin. AST1510's sync failed to bypass, so we disable it and use it as GPIO control.
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DAC, 0);
////	ModSCUReg(SCU_PIN_CTRL2_REG, 0, BIT12|BIT13);
//	//Enable GPIOJ5, turn off Sync
//	gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_OFF);
//	//Disable DVO bypass mode
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DVO_BYPASS_MODE, 0);
////	ModSCUReg(SCU_MISC_CTRL_REG,
////	                   (0 << DAC_SOURCE_SELECT_SHIFT),
////	                   DAC_SOURCE_SELECT_MASK);

	if (crt->tx_select == XIDX_HOST_A)
		I2CInit(I2C_HOST_VIDEO_LOOPBACK_DDC, I2C_MONITOR_DDC_SPEED);

	return 0;
}

unsigned int xmiter_disable_dac(struct s_crt_drv *crt)
{
	/* turn off sync , disable bypass mode */
	ast_scu.scu_op(SCUOP_CRT_DAC_CTRL_DISABLE, NULL);

//	//Enable GPIOJ5, turn off Sync
//	gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_OFF);
//	//Disable DVO bypass mode
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DVO_BYPASS_MODE, 0);
////	ModSCUReg(SCU_MISC_CTRL_REG,
////	                   (0 << DAC_SOURCE_SELECT_SHIFT),
////	                   DAC_SOURCE_SELECT_MASK);

	return 0;
}

unsigned int xmiter_setup_dac(struct s_crt_drv *crt, struct s_crt_info *info)
{
	if ((info->EnHDCP != HDCP_DISABLE) && (info->owner != OWNER_CON)) {
		uinfo("DAC disabled due to HDCP\n");
		/* disable dac to block HDCP content */
		return xmiter_disable_dac(crt);
	}

	/* no video output on host DAC if color format is not RGB */
	if (info->crt_output_format != XRGB8888_FORMAT) {
		uinfo("DAC disabled due to non-RGB color mode\n");
		return xmiter_disable_dac(crt);
	}

#if (BOARD_DESIGN_VER_VIDEO >= 300)
	/*
	** Loopback HSync used as DE signal. To resolve blank zone noise issue.
	** Solution: (Need v1.4 or above HW)
	** - When input is digital, GPIOV6 set to low.
	** - When input is analog, GPIOV6 set to high.
	*/
	if (info->HDMI_Mode == s_RGB)
		gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_ON);
	else
		gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_OFF);
#endif

	/* turn on sync , enable bypass mode */
	ast_scu.scu_op(SCUOP_CRT_DAC_CTRL_FROM_DVO, NULL);

//	//Enable GPIOJ5, turn on Sync
//	gpio_direction_output(GPIO_HOST_DAC_SWITCH, GPIO_HOST_DAC_SWITCH_ON);
//	//Enable DVO bypass mode
//	ast_scu.scu_op(SCUOP_CRT_ENABLE_DVO_BYPASS_MODE, 1);
////	ModSCUReg(SCU_MISC_CTRL_REG,
////	                   (0x2 << DAC_SOURCE_SELECT_SHIFT),
////	                   DAC_SOURCE_SELECT_MASK);

	return 0;
}
#else //#if (BOARD_DESIGN_VER_VIDEO >= 205)
unsigned int xmiter_dac_exist(struct s_crt_drv *crt)
{
	//TBD
	return 0;
}
unsigned int xmiter_init_dac(struct s_crt_drv *crt)
{
	//TBD
	return 0;
}

unsigned int xmiter_disable_dac(struct s_crt_drv *crt)
{
	//TBD
	return 0;
}

unsigned int xmiter_setup_dac(struct s_crt_drv *crt, struct s_crt_info *info)
{
	//TBD
	return 0;
}
#endif //#if (BOARD_DESIGN_VER_VIDEO >= 205)

#endif //#if defined(CONFIG_ARCH_AST1500_CLIENT)

#endif //#if (CONFIG_AST1500_SOC_VER == 1)

#endif //#ifndef _XMITER_DAC_C_

