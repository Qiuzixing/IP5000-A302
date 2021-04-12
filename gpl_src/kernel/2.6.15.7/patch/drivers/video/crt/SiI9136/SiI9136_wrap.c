/*
 * Copyright (c) 2004-2014
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include "defs.h"
#include "SiI9136_main.h"
#include "TPI.h"
#include "Externals.h"
#include "TPI_Access.h"
#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
#include "ast_utilities.h" /* msleep() */
#include "si_apiCEC.h"
#include "si_apiCpi.h"
#endif

int is_dev_exists(void)
{
	return sii9136_identify_chip();
}

int sii9136_dev_init(struct sii9136_drv_data *d)
{
	down(&d->reg_lock);

	if (TPI_Init()) {

		SetUSRX_OutputChange; //BruceToCheck

		//Success
		up(&d->reg_lock);
		return 0;
	}
	up(&d->reg_lock);

	return -1;
}

void sii9136_fsm_timer_handler(struct sii9136_drv_data *d)
{
	down(&d->reg_lock);
	TPI_Poll();
	up(&d->reg_lock);
}

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1)
void sii9136_cec_tx_handler(struct sii9136_drv_data *d)
{
	si_cec_do_tx();
}

void sii9136_cec_topology_handler(struct sii9136_drv_data *d)
{
	if (CableConnected)
		d->cec_topology = si_cec_topology_status();
	else
		d->cec_topology = 0;
}

void sii9136_cec_request_pa_handler(struct sii9136_drv_data *d)
{
	int i;
	u8 buf[2];

	buf[1] = 0x83; /* Give Physical Address */

	for (i = 0; i < 12; i++) {
		if (d->cec_topology & (0x1 << i)) {
			buf[0] = (SI_CpiGetLogicalAddr() << 4 | i);
			sii9136_cec_send(buf, 2);

			msleep(200);
		}
	}
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) && (IS_CEC == 1) */
