#ifndef _asm_arm_arch_umvp_vic_h_
#define _asm_arm_arch_umvp_vic_h_

/* implemented in arch/arm/mach-umvp/irq.c */
enum vic_intr_trig_mode_E
{
    vicc_edge_activeFalling = 0,
    vicc_edge_activeRaising = 1,
    vicc_edge_activeBoth    = 2,
    vicc_level_activeHigh   = 3,
    vicc_level_activeLow    = 4
};
typedef enum vic_intr_trig_mode_E vic_intr_trig_mode_E;

void vic_set_intr_trigger(int irqnr, vic_intr_trig_mode_E);
void vic_clear_intr(int irqnr);

#endif

