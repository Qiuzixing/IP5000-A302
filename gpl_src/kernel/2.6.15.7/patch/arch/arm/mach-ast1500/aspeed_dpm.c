/*
 * arch/arm/mach-ast3000/aspeed_dpm.c  DPM support for ASPEED
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Copyright (C) 2002, 2004 MontaVista Software <source@mvista.com>.
 *
 * Based on code by Matthew Locke, Dmitry Chigirev, and Bishop Brock.
 */

#include <linux/config.h>
#include <linux/dpm.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/pm.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <asm/hardirq.h>
#include <asm/page.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/hardware/clock.h>
#include <asm/arch/pm.h>
#include <asm/mach-types.h>

#include <asm/arch/ast-scu.h>

#define ASPEED_SCU_VA	(IO_ADDRESS(ASPEED_SCU_BASE))
#define ASPEED_MMC_VA	(IO_ADDRESS(ASPEED_MMC_BASE))

///* PLL Settings */
//unsigned int get_hpll_mode(void)
//{
//    unsigned int data;
//    	 
//    data = IO_READ(ASPEED_SCU_VA + 0x70);
//    data >>= 9;
//    data &= 0x07;
//    
//    return data;	
//}

//void set_hpll_mode(unsigned int mode)
//{
//    unsigned int data;
//    	 
//    data = IO_READ(ASPEED_SCU_VA + 0x70);
//    data &= 0xfffff1ff;
//    mode <<= 9;
//    IO_WRITE((ASPEED_SCU_VA + 0x70), (data | mode));
//
//}

/* DPLL Mode Settings */
unsigned int get_dpll_mode(void)
{ 
    /* TODO */	 
    return 0;	
}

void set_dpll_mode(unsigned int mode)
{
    /* TODO */	
}

/* Video Mode Settings */
unsigned int get_video_mode(void)
{ 
    /* TODO */	 
    return 0;	
}

void set_video_mode(unsigned int mode)
{
    /* TODO */	
}
				
int
dpm_aspeed_set_opt(struct dpm_opt *cur, struct dpm_opt *new)
{

	unsigned long flags;

	if (new->md_opt.hpll_mode == 8) {
#ifdef CONFIG_PM
		pm_suspend(PM_SUSPEND_STANDBY);

	 	/* Here when we wake up.  Recursive call to switch back to
		 * to task state.
		 */

		dpm_set_os(DPM_TASK_STATE);
#endif
		return 0;
	}

	local_irq_save(flags);

		/* set hpll */
		ast_scu.scu_op(SCUOP_HPLL_SET, new->md_opt.hpll_mode);
//      set_hpll_mode(new->md_opt.hpll_mode);
        
		/* set dpll mode */
		set_dpll_mode(new->md_opt.dpll_mode);
        
		/* set video mode */
		set_video_mode(new->md_opt.video_mode);
           
	local_irq_restore(flags);
	
	return 0;
}

int
dpm_aspeed_get_opt(struct dpm_opt *opt)
{
	struct dpm_md_opt *md_opt = &opt->md_opt;

	md_opt->hpll_mode  = ast_scu.HPLL_Mode;
//	md_opt->hpll_mode  = get_hpll_mode();
	md_opt->dpll_mode  = get_dpll_mode();
	md_opt->video_mode = get_video_mode();

	return 0;
}

int
dpm_aspeed_init_opt(struct dpm_opt *opt)
{
	unsigned int hpll_mode = opt->pp[DPM_MD_HPLL_MODE];
	unsigned int dpll_mode = opt->pp[DPM_MD_DPLL_MODE];
	unsigned int video_mode = opt->pp[DPM_MD_VIDEO_MODE];
	struct dpm_md_opt *md_opt = &opt->md_opt;

	md_opt->hpll_mode  = hpll_mode;
	md_opt->dpll_mode  = dpll_mode;
	md_opt->video_mode = video_mode;

	return 0;
}

/****************************************************************************
 *  DPM Idle Handler
 ****************************************************************************/

static void (*orig_idle)(void);

void dpm_aspeed_idle(void)
{
	extern void default_idle(void);

	if (orig_idle)
		orig_idle();
	else
		default_idle();
}

/****************************************************************************
 * Initialization/Exit
 ****************************************************************************/

extern void (*pm_idle)(void);

static void
dpm_aspeed_startup(void)
{
	if (pm_idle != dpm_idle) {
		orig_idle = pm_idle;
		pm_idle = dpm_idle;
	}
}

static void
dpm_aspeed_cleanup(void)
{
	pm_idle = orig_idle;
}

int __init
dpm_aspeed_init(void)
{
	printk("ASPEED Dynamic Power Management.\n");
	dpm_md.init_opt		= dpm_aspeed_init_opt;
	dpm_md.set_opt		= dpm_aspeed_set_opt;
	dpm_md.get_opt		= dpm_aspeed_get_opt;
	dpm_md.check_constraint	= dpm_default_check_constraint;
	dpm_md.idle		= dpm_aspeed_idle;
	dpm_md.startup		= dpm_aspeed_startup;
	dpm_md.cleanup		= dpm_aspeed_cleanup;

	return 0;
}

__initcall(dpm_aspeed_init);

/*
 * Local variables:
 * c-basic-offset: 8
 * End:
 */
