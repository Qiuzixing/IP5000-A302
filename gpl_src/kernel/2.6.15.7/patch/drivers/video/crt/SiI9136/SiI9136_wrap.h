/*
 * Copyright (c) 2004-2014
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _SII9136_WRAP_H_
#define _SII9136_WRAP_H_

struct sii9136_drv_data;

int is_dev_exists(void);
int sii9136_dev_init(struct sii9136_drv_data *d);
void sii9136_fsm_timer_handler(struct sii9136_drv_data *d);
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
void sii9136_cec_topology_handler(struct sii9136_drv_data *d);
void sii9136_cec_request_pa_handler(struct sii9136_drv_data *d);
void sii9136_cec_tx_handler(struct sii9136_drv_data *d);
u16 si_cec_topology_status(void);
#endif

#endif //#ifndef _SII9136_WRAP_H_

