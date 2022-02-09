/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <linux/module.h>
#include "hid.h"
#include "kmoip.h"


static inline int _match_hotkey_down(struct kmoip_drv *kmdrv, unsigned char mod, unsigned char key, int hk_idx)
{
	unsigned char match_mod, match_key;

	match_mod = kmdrv->hotkey[hk_idx][0];
	match_key = kmdrv->hotkey[hk_idx][1];

	//Don't check NULL hotkey slot.
	if (match_mod == 0 && match_key == 0)
		return -1;

	if (match_mod == mod && match_key == key)
		return hk_idx;

	return -1;
}
static inline int match_hotkey_down(struct kmoip_drv *kmdrv, unsigned char mod, unsigned char key, int hk_idx)
{
	int i;

	if (hk_idx >= 0)
		return _match_hotkey_down(kmdrv, mod, key, hk_idx);

	for (i = 0; i < KMOIP_MAX_HOTKEY_SLOT; i++) {
		if (_match_hotkey_down(kmdrv, mod, key, i) == i) {
			return i;
		}
	}
	return -1;
}

static inline int match_hotkey_up(struct kmoip_drv *kmdrv, unsigned char mod, unsigned char key, int hk_idx)
{
	unsigned char match_mod, match_key;

	match_mod = kmdrv->hotkey[hk_idx][0];
	match_key = kmdrv->hotkey[hk_idx][1];

	/*
	** Rules:
	** 1. key must be 0. Means not pressed. no other keys pressed, either.
	** 2. mod key can be 0 or the same.
	*/
	if (key != 0)
		return -1;

	if (mod != 0 && mod != match_mod)
		return -1;

	return hk_idx;
}

/*
** hotkey_filter() will be called by kmoip.c each time KBD0 key event happens.
** report->report[0] will be the keycode of 'key modifier'. Ex: ctrl, alt, shift.
** report->report[2] will be the keycode of normal key.
** kmdrv->hotkey[KMOIP_MAX_HOTKEY_SLOT][2] saves the defined hotkeys. 
**     They are KMOIP_MAX_HOTKEY_SLOT hotkey defines in total, where
**     hotkey[i][0] saves 'key modifier', hotkey[i][1] saves 'normal key'.
** Will trigger e_hotkey_X event when hotkey triggerred. The 'X' maps to 0 ~ (KMOIP_MAX_HOTKEY_SLOT-1).
*/
void hotkey_filter(struct kmoip_drv *kmdrv, struct kmoip_report *report)
{
	static int hk_idx;
	static unsigned long timeout;
	static int hit_cnt; //3,2,1,0
	int t;
	unsigned char mod, key;
	static enum {
		HK_IDLE = 0,
		HK_DOWN,
		HK_UP,
	} hk_state = HK_IDLE;

	/*
	** Following state machine triggers hotkey event when hotkey is pressed for 3 times in 750ms.
	*/
	mod = report->report[0];
	key = report->report[2];

	if ((hk_state != HK_IDLE) && time_after(jiffies, timeout))
		goto restart;

check_again:
	switch (hk_state) {
		case HK_IDLE:
			t = match_hotkey_down(kmdrv, mod, key, -1);
			if (t < 0)
				break;
			hk_idx = t;
			hk_state = HK_DOWN;
			timeout = jiffies + msecs_to_jiffies(750);
			hit_cnt = 2;	// Sid 2022-01-01, Just follow customer's requirement, with Double Click instead of Triple Click.
			break;
		case HK_DOWN:
			t = match_hotkey_up(kmdrv, mod, key, hk_idx);
			if (t < 0)
				goto restart;
			hk_state = HK_UP;

			if (!(--hit_cnt)) {
				/* hotkey triggered */
				//uinfo("Hotkey %d triggerred in %dms!!\n", hk_idx, 750 - jiffies_to_msecs(timeout-jiffies));
				/* notify user */
				char event[16];
				sprintf(event, "e_hotkey_%d", hk_idx);
				ast_notify_user(event);
				goto reset;
			}
			break;
		case HK_UP:
			t = match_hotkey_down(kmdrv, mod, key, hk_idx);
			if (t < 0)
				goto restart;
			hk_state = HK_DOWN;
			break;
		default:
			goto restart;
	}
	return;

restart:
	hk_idx = -1;
	hk_state = HK_IDLE;
	goto check_again;

reset:
	hk_idx = -1;
	hk_state = HK_IDLE;
}

EXPORT_SYMBOL(hotkey_filter);
