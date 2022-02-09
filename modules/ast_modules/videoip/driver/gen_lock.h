
#if (CONFIG_AST1500_SOC_VER >= 3)

#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if DESC_FLG_VSYNC_PKT

#ifndef _GEN_LOCK_H_
#define _GEN_LOCK_H_


/*******************************************************************************
*	Start of exported function calls
*******************************************************************************/
u32 gen_lock_ppm_current(void);
u32 gen_lock_ppm_positive(void);
u32 gen_lock_ppm_negative(void);

u32 gen_lock_last_location(void);
u32 gen_lock_location_h(void);
u32 gen_lock_location_l(void);

void gen_lock_jump_wait_cfg(u32 cnt);
u32 gen_lock_jump_wait(void);
void gen_lock_approach_wait_cfg(u32 cnt);
u32 gen_lock_approach_wait(void);
void gen_lock_ppm_budget_cfg(u32 budget);
u32 gen_lock_ppm_budget(void);
void gen_lock_cool_down_cfg(u32 cnt);
u32 gen_lock_cool_down(void);
u32 gen_lock_monitor_ppm_limit(void);
u32 gen_lock_monitor_ppm_limit_4k_patch(void);
int gen_lock_ppm_max(void);
int gen_lock_ppm_min(void);
u32 gen_lock_location(void);
u32 gen_lock_location_max(void);
u32 gen_lock_en(void);
u32 gen_lock_state(void);
u32 gen_lock_factor(void);
void gen_lock_cfg(u32 cfg);
void gen_lock_fsm_restart(void);
void gen_lock_fsm(void);

#endif /* #ifndef _GEN_LOCK_H_ */
#endif /* #if DESC_FLG_VSYNC_PKT */
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */
