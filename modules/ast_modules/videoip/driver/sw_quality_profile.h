/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _SW_QUALITY_PROFILE_H_
#define _SW_QUALITY_PROFILE_H_

#ifdef CONFIG_ARCH_AST1500_HOST
int ve_set_encode_quality_mode(int quality_mode);
void chg_q_state_machine_host(void);
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#ifdef CONFIG_ARCH_AST1500_CLIENT
int ve_set_decode_quality_mode(int quality_mode);
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

#endif /* #ifndef _SW_QUALITY_PROFILE_H_ */
