#ifndef _AST_HW_UDP_H_
#define _AST_HW_UDP_H_

#include <linux/skbuff.h>
#include <linux/list.h>

struct skb_entry {
	struct list_head list;
	struct sk_buff *skb;
	unsigned char *data_ptr; //Point to skb buffer's start of data (include eth hdrs).
	u32 skb_data_phy; //PHY addr of skb->data;
	unsigned int data_size; //valid data length
	unsigned int buf_size; //skb buffer length
};

int ahu_init_tx(unsigned int skb_pool_size, unsigned int skb_size);
struct skb_entry *ahu_dequeue_skbe_tx_pool(void);
int ahu_send(struct skb_entry *skbe);
void ahu_force_release_tx_skbe(struct skb_entry *skbe);

int ahu_start_rx(u32 dest_ip_addr, u16 dest_udp_port);
int ahu_stop_rx(void);
void ahu_suspend_rx(void);
void ahu_resume_rx(void);
void ahu_release_rx_skb(struct skb_entry *skbe);
void ahu_vsync_filter(unsigned int enable, unsigned int sender_id);
struct skb_entry *ahu_dequeue_skb_rx_ready(void);
int ahu_init_rx(unsigned int skb_pool_size);
int ahu_cleanup_rx(void);


#endif //#ifndef _AST_HW_UDP_H_

