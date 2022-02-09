/*
 * include/asm-arm/arch-omap/dpm.h
 * DPM for TI OMAP
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
 * Copyright (C) 2002, MontaVista Software <source@mvista.com>
 *
 * Based on arch/ppc/platforms/ibm405lp_dpm.h by Bishop Brock.
 */

#ifndef __ASM_OMAP_DPM_H__
#define __ASM_OMAP_DPM_H__

/*
 * machine dependent operating state
 *
 * An operating state is a cpu execution state that has implications for power
 * management. The DPM will select operating points based largely on the
 * current operating state.
 *
 * DPM_STATES is the number of supported operating states. Valid operating
 * states are from 0 to DPM_STATES-1 but when setting an operating state the
 * kernel should only specify a state from the set of "base states" and should
 * do so by name.  During the context switch the new operating state is simply
 * extracted from current->dpm_state.
 *
 * task states:
 *
 * APIs that reference task states use the range -(DPM_TASK_STATE_LIMIT + 1)
 * through +DPM_TASK_STATE_LIMIT.  This value is added to DPM_TASK_STATE to
 * obtain the downward or upward adjusted task state value. The
 * -(DPM_TASK_STATE_LIMIT + 1) value is interpreted specially, and equates to
 * DPM_NO_STATE.
 *
 * Tasks inherit their task operating states across calls to
 * fork(). DPM_TASK_STATE is the default operating state for all tasks, and is
 * inherited from init.  Tasks can change (or have changed) their tasks states
 * using the DPM_SET_TASK_STATE variant of the sys_dpm() system call.  */

#define DPM_IDLE_TASK_STATE  0
#define DPM_IDLE_STATE       1
#define DPM_SLEEP_STATE      2
#define DPM_BASE_STATES      3

#define DPM_TASK_STATE_LIMIT 4
#define DPM_TASK_STATE       (DPM_BASE_STATES + DPM_TASK_STATE_LIMIT)
#define DPM_STATES           (DPM_TASK_STATE + DPM_TASK_STATE_LIMIT + 1)
#define DPM_TASK_STATES      (DPM_STATES - DPM_BASE_STATES)

#define DPM_STATE_NAMES                  \
{ "idle-task", "idle", "sleep",          \
  "task-4", "task-3", "task-2", "task-1",\
  "task",                                \
  "task+1", "task+2", "task+3", "task+4" \
}

#define DPM_PARAM_NAMES				\
{ "hpll-mode", "dpll-mode", "video-mode" }

/* MD operating point parameters */
#define DPM_MD_HPLL_MODE	0  /* ARM PLL Mode */
#define DPM_MD_DPLL_MODE	1  /* DRAM PLL Mode */
#define DPM_MD_VIDEO_MODE	2  /* VIDEO Mode */

#define DPM_PP_NBR 3

#ifndef __ASSEMBLER__

#include <linux/types.h>

/* Instances of this structure define valid Innovator operating points for DPM.
   Voltages are represented in mV, and frequencies are represented in KHz. */

struct dpm_md_opt {
	unsigned int hpll_mode;		/* CPU frequency in MHz */
	unsigned int dpll_mode;		/* 0/1: normal/low */
	unsigned int video_mode;	/* 0/1: enable/disable */	
};

#endif /* __ASSEMBLER__ */
#endif /* __ASM_OMAP_DPM_H__ */
