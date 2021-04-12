/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include "i2s_types.h"
#include "i2s.h"
#include "i2s_task.h"
#include "i2s_network.h"
#include "i2s_event.h"

#ifdef CONFIG_ARCH_AST1500_HOST
extern int i2sInit;
extern struct list_head	ctrl_event_list;
extern spinlock_t	ctrl_event_list_lock;
extern unsigned int ctrl_event_list_size;
#if 0
extern wait_queue_head_t	ctrl_wait_queue;
#else
extern wait_queue_head_t	rx_wait_queue;
#endif

//The caller should check the return value to to free the memory pointed to by pdata if fail.
//If success, the memory pointed to by pdata will be freed later by event_handler.
int add_event(u32 event, void *pdata, struct socket *pfrom)
{
	pevent_struct pevent;
	ULONG	flags;
	
	if (!i2sInit)
		return 0;
	
	pevent = kmalloc(sizeof(event_struct), GFP_KERNEL);
	if (pevent == NULL){
		I2S_err("error kmalloc for event_struct (event = %X)\n", event);
		return 0;
	}

	pevent->event = event;
	pevent->pdata = pdata;
	pevent->pfrom = pfrom;

	/* Bruce190416. List must be inited before use. */
	INIT_LIST_HEAD(&pevent->list);
	
	spin_lock_irqsave(&ctrl_event_list_lock, flags);
	/*
	** Bruce190417. Bug fix.
	** Scenario:
	**     1. host config to auto select dual video input (hdmi or vga)
	**     2. multicast mode. one pair of host and client.
	**     3. disconnect (e_stop_link) client from host. (i2s driver rx thread will stop)
	**     4. disconnect video input from host.
	**     5. wait for over 60 minutes. (let host do auto video switching for thousand times)
	**     6. connect client to host by e_reconnect.
	**     7. i2s driver will loop in handle_events() for a long time. ==> bug
	** Root cause:
	**     auto video input switching will trigger audio io select.
	**     ==> a new audio event will be added each time video input switch.
	**     ==> however under disconnected state, there is no code handling this event.
	**     ==> so, a lot of events are queued until client connected back to host.
	** Solution:
	**     The procedure makes sense. The only problem is that under this condition
	**     old audio change events is meaningless.And since add_event() function is
	**     so far only used for queuing audio change events.
	**     So, just drop old events here is the most quick way to plug the hole without
	**     re-consider the whole picture and remodeling the driver.
	** Note:
	**     - You CAN NOT use add_event() for events other than audio changes any more.
	**     - Bruce think keep queue size as 1 is enough to resolve this issue without
	**       side effects. However Jerry is afraid of something goes wrong. So, we pick
	**       4 as queue size.
	*/
	if (ctrl_event_list_size > 4) {
		pevent_struct old_pevent;
		/* pop and free queue head entry. */
		old_pevent = list_entry(ctrl_event_list.next, event_struct, list);
		list_del(ctrl_event_list.next);
		ctrl_event_list_size--;
		I2S_info("drop expired event (0x%08X)\n", old_pevent->event);
		kfree(old_pevent);
	}
	list_add_tail(&pevent->list, &ctrl_event_list);
	ctrl_event_list_size++;

	spin_unlock_irqrestore(&ctrl_event_list_lock, flags);
#if 0
	wake_up_interruptible(&ctrl_wait_queue);
#else
	wake_up_interruptible(&rx_wait_queue);
#endif
	return 1;
}
#endif
