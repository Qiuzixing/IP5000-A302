/*
** ahu stands for Ast_Hw_Udp.
*/
#define __AHU__ (1)
#if __AHU__

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <asm/arch/drivers/ast_hw_udp.h>
#include <linux/wait.h>

#define AHU_FPGA_TEST 0
int ftgmac100_wait_to_send_packet( struct sk_buff * skb, struct net_device * dev );
void mac_ahu_start_rx_filter(struct net_device *dev, u32 dest_ip_addr, u16 dest_udp_port);
void mac_ahu_stop_rx_filter(struct net_device *dev);
void mac_ahu_vsync_filter(struct net_device *dev, unsigned int enable, unsigned int sender_id);


#define skb_to_skbe(skb) (*(struct skb_entry **)(skb->cb))

#if AHU_FPGA_TEST
struct eth_hdr {
	__u8 dest[6];
	__u8 source[6];
	__u8 ethtype[2];
} __attribute__ ((packed));

//From linux/include/linux/ip.h
struct iphdr {
	__u8   version:4,
		   ihl:4; //Internet header length
	__u8   tos; //Type of service (0x00)
	__u16 tot_len; //Total length (hdr+data)
	__u16 id; //id 0~65535
	__u16 frag_off; // fragment offset 0
	__u8   ttl; //Time to live (linux 64)
	__u8   protocol; // UDP: 0x11
	__u16 check; //header checksum
	__u32 saddr; //source IP address
	__u32 daddr; //dest IP address
};

//From linux/include/linux/udp.h
struct udphdr {
	__u16 source; //source port (optional 0)
	__u16 dest;  //destination port
	__u16 len; //length (hdr+data)
	__u16 check; //checksum (optional. Fixed 0)
};
#endif

#if 0 //Moved to .h
struct skb_entry {
	struct list_head list;
	struct sk_buff *skb;
	unsigned char *data_ptr; //Point to skb buffer's start of data (exclude hdrs).
	u32 skb_data_phy; //virt_to_phys(skb->data);
	unsigned int data_size; //valid data length
	unsigned int buf_size; //skb buffer length
};
#endif

struct ahu_context {
	struct net_device *dev;
	spinlock_t lock; //generic driver lock
	wait_queue_head_t ahu_wq;

	struct list_head skb_tx_pool; //A list of struct skb_entry
	struct skb_entry *skb_entry_tx_buf; //BruceToDo. To be released.
	struct skb_entry *skb_entry_rx_buf; //BruceToDo. To be released.
	unsigned int skb_tx_pool_qlen;

	struct list_head skb_rx_pool; //A list of struct skb_entry list
	struct list_head skb_rx_rdy_list; //A list of struct skb_entry with ready skb
	unsigned int skb_rx_rdy_qlen;
#define AHU_ENABLED    (0x1UL << 0)
#define AHU_SUSPEND    (0x1UL << 1)
	unsigned int rx_enabled;

#if AHU_FPGA_TEST
	struct workqueue_struct *wq; //upstream work queue
	struct work_struct test_work; //upstream work
	struct list_head test_skb_queue; //A list of skb_entry queue for test

	struct eth_hdr eth_tmp;
	struct iphdr ip_tmp;
	struct udphdr udp_tmp;
#endif

};

struct ahu_context *ahu;

#define is_ahu_rx_enabled(ahu)     ((ahu)->rx_enabled & AHU_ENABLED)
#define is_ahu_rx_suspeneded(ahu)  ((ahu)->rx_enabled & AHU_SUSPEND)
#define ahu_rx_state_disable(ahu)   do { (ahu)->rx_enabled = 0; } while (0)
#define ahu_rx_state_enable(ahu)    do { (ahu)->rx_enabled |= AHU_ENABLED; } while (0)
#define ahu_rx_state_suspend(ahu)   do { if (is_ahu_rx_enabled(ahu)) (ahu)->rx_enabled |= AHU_SUSPEND; } while (0)
#define ahu_rx_state_resume(ahu)    do { if (is_ahu_rx_enabled(ahu)) (ahu)->rx_enabled &= ~AHU_SUSPEND; } while (0)

/**
 * uerr - print error messages
 * @fmt:
 * @args:
 */
#define uerr(fmt, args...)						\
	do {								\
		printk(KERN_ERR "%-10s: ***ERROR*** %s(%d): " fmt,	\
			(in_interrupt() ? "interrupt" : (current)->comm),\
			__FUNCTION__, __LINE__,  ##args);	\
	} while(0)

/**
 * uinfo - print information messages
 * @fmt:
 * @args:
 */
#define uinfo(fmt, args...)					\
	do {							\
		printk(KERN_INFO "AHU: " fmt , ## args);	\
	} while(0)


static inline void udump(void *buf, int bufflen)
{
	int i;
	unsigned char *buff= buf;

	printk("dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

	if (bufflen > 768) {
		for (i = 0; i< 768; i++) {
			if (i%16 == 0)
				printk("   ");
			printk("%02x ", (unsigned char ) buff[i]);
			if (i%4 == 3) printk("| ");
			if (i%16 == 15) printk("\n");
		}
		printk("... (%d byte)\n", bufflen);
		return;
	}

	for (i = 0; i< bufflen; i++) {
		if (i%16 == 0)
			printk("   ");
		printk("%02x ", (unsigned char ) buff[i]);
		if (i%4 == 3)
			printk("| ");
		if (i%16 == 15)
			printk("\n");
	}
	printk("\n");

}

static inline int __is_fifo_empty(struct list_head *fifo_head)
{
	return list_empty(fifo_head);
}

static inline void __enqueue_fifo(struct list_head *fifo_head, struct list_head *in_head)
{
	list_add_tail(in_head, fifo_head);
}

static inline struct list_head *__dequeue_fifo(struct list_head *fifo_head)
{
	struct list_head *out;

	if (list_empty(fifo_head)) {
		return NULL;
	}
	out = fifo_head->next;
	list_del_init(out);

	return out;
}

void enqueue_skb_rx_pool(struct ahu_context *ahu, struct skb_entry *skb_entry, int lock)
{
	unsigned long flags = 0;
	if (lock) spin_lock_irqsave(&ahu->lock, flags);

	//clean up
	__enqueue_fifo(&ahu->skb_rx_pool, &skb_entry->list);

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);
}

struct skb_entry *dequeue_skb_rx_pool(struct ahu_context *ahu, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags = 0;

	if (lock) spin_lock_irqsave(&ahu->lock, flags);
	list_head = __dequeue_fifo(&ahu->skb_rx_pool);
	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);

	if (!list_head)
		return NULL;

	return list_entry(list_head, struct skb_entry, list);
}

void enqueue_skb_rx_ready(struct ahu_context *ahu, struct skb_entry *skb_entry, int lock)
{
	unsigned long flags = 0;
	if (lock) spin_lock_irqsave(&ahu->lock, flags);

	//clean up
	__enqueue_fifo(&ahu->skb_rx_rdy_list, &skb_entry->list);
	ahu->skb_rx_rdy_qlen++;

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);
}

struct skb_entry *dequeue_skb_rx_ready(struct ahu_context *ahu, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags = 0;

	if (lock) spin_lock_irqsave(&ahu->lock, flags);

	list_head = __dequeue_fifo(&ahu->skb_rx_rdy_list);
	if (list_head)
			ahu->skb_rx_rdy_qlen--;

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);

	if (!list_head)
		return NULL;

	return list_entry(list_head, struct skb_entry, list);
}

void enqueue_skb_tx_pool(struct ahu_context *ahu, struct skb_entry *skb_entry, int lock)
{
	unsigned long flags = 0;
	if (lock) spin_lock_irqsave(&ahu->lock, flags);

	//clean up
	__enqueue_fifo(&ahu->skb_tx_pool, &skb_entry->list);
	ahu->skb_tx_pool_qlen++;

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);

	/*wakeup waitq.*/
	if (ahu->skb_tx_pool_qlen == 1)
		wake_up_interruptible(&ahu->ahu_wq); //Used to wake up thread waiting for ahu_dequeue_skbe_tx_pool().

}

struct skb_entry *dequeue_skb_tx_pool(struct ahu_context *ahu, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags = 0;

	if (lock) spin_lock_irqsave(&ahu->lock, flags);
	list_head = __dequeue_fifo(&ahu->skb_tx_pool);

	if (!list_head) {
		if (lock) spin_unlock_irqrestore(&ahu->lock, flags);
		printk("OOM?!\n");
		return NULL;
	}
	ahu->skb_tx_pool_qlen--;

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);
	return list_entry(list_head, struct skb_entry, list);
}

#if AHU_FPGA_TEST
static struct eth_hdr eth_tmp = {
	//.dest = { 0x01, 0x00, 0x5E, 0x00, 0x64, 0x00 },
	.dest = { 0x82, 0xDE, 0x7E, 0x03, 0x13, 0x33 },
	.source = { 0x02, 0xD9, 0x65, 0xD4, 0x2C, 0x86 },
	.ethtype = { 0x08, 0x00 }
};

struct iphdr ip_tmp = {
	.version = 0x4,
	.ihl = 0x5, //Internet header length
	.tos = 0, //Type of service (0x00)
	.tot_len = 0, //Total length (hdr+data)
	.id = 0, //id 0~65535
	.frag_off = 0x2, // fragment offset 0. bit1: don't fragment (DF) set to 1.
	.ttl = 64, //Time to live (linux 64)
	.protocol = 0x11, // UDP: 0x11
	.check = 0, //header checksum
	.saddr = 0xEF07FEA9, //source IP address. 169.254.7.239 -> 0xEF07FEA9
	.daddr = 0x006400E1 //dest IP address. 225.0.100.0 -> 0x006400E1
};

struct udphdr udp_tmp = {
	.source = 0x0104, //source port (optional 0). 1025 -> 0x0104 (big-endian)
	.dest = 0xAD0C,  //destination port
	.len = 0, //length (hdr+data)
	.check = 0 //checksum (optional. Fixed 0)
};


void enqueue_test_skb(struct ahu_context *ahu, struct skb_entry *skb_entry, int lock)
{
	unsigned long flags;
	if (lock) spin_lock_irqsave(&ahu->lock, flags);

	//clean up
	__enqueue_fifo(&ahu->test_skb_queue, &skb_entry->list);

	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);
}

struct skb_entry *dequeue_test_skb(struct ahu_context *ahu, int lock)
{
	struct list_head *list_head = NULL;
	unsigned long flags;

	if (lock) spin_lock_irqsave(&ahu->lock, flags);
	list_head = __dequeue_fifo(&ahu->test_skb_queue);
	if (lock) spin_unlock_irqrestore(&ahu->lock, flags);

	if (!list_head)
		return NULL;

	return list_entry(list_head, struct skb_entry, list);
}

static void test_construct_skb(struct ahu_context *ahu)
{
	struct skb_entry *skbe;
	struct sk_buff *skb;

	for (;;) {
		skbe = dequeue_skb_tx_pool(ahu, 1);
		if (!skbe)
			break;
		skb = skbe->skb;
		skb->priority = 0x16881688;
		skb_reserve(skb, 2);

		memcpy(skb->tail, &eth_tmp, sizeof(eth_tmp));
		skb->mac.raw = skb_put(skb, sizeof(eth_tmp));
		memcpy(skb->tail, &ip_tmp, sizeof(ip_tmp));
		skb->nh.raw = skb_put(skb, sizeof(ip_tmp));
		memcpy(skb->tail, &udp_tmp, sizeof(udp_tmp));
		skb->h.raw = skb_put(skb, sizeof(udp_tmp));
		skbe->data_ptr = skb->tail;
		//Test code.
#if 0
		skb->tail[0] = 0x01;
		skb->tail[1] = 0x02;
		skb->tail[2] = 0x03;
		skb->tail[3] = 0x04;
		skb_put(skb, 4);
#endif
		enqueue_test_skb(ahu, skbe, 1);
	}
}

static void ahu_test_work(struct ahu_context *ahu)
{
	struct skb_entry *skbe;
	struct sk_buff *skb;
	static int i = 0;

	if (netif_queue_stopped(ahu->dev))
		goto again;

	skbe = dequeue_test_skb(ahu, 1);
	BUG_ON(!skbe);

	skb = skbe->skb;
	skb->tail[0] = i++;
	skb_put(skb, 1);

	//printk("(%d)\n", skbe->data_ptr[0]);

	ftgmac100_wait_to_send_packet(skbe->skb, ahu->dev);

again:
	queue_delayed_work(ahu->wq, &ahu->test_work, msecs_to_jiffies(1000));
}

static void ahu_test_init(struct ahu_context *ahu)
{
	INIT_LIST_HEAD(&ahu->test_skb_queue);
	ahu->wq = create_singlethread_workqueue("AST_HW_UDP");
	INIT_WORK(&ahu->test_work, (void (*)(void *))ahu_test_work, ahu);

	test_construct_skb(ahu);

	queue_work(ahu->wq, &ahu->test_work);
}
#endif

void ahu_release_tx_skb(struct sk_buff *skb)
{
	struct skb_entry *skbe;

#if AHU_FPGA_TEST
	BUG_ON(skb->priority != 0x16881688);
	skbe = skb_to_skbe(skb);
	//printk("[%d]\n", skbe->data_ptr[0]);

	skb_trim(skb, skb->len - 1);

	enqueue_test_skb(ahu, skbe, 1);
#else
	//BruceToDo. re-init skb
	//BruceToCheck. Is there a kernel skb API to re-init skb? --> can't find.
	skbe = skb_to_skbe(skb);
	skbe->data_size = 0;
	skb->len = 0;
	skb->data = skbe->data_ptr;
	skb->tail = skb->data;
	enqueue_skb_tx_pool(ahu, skbe, 1);
#endif
}

void ahu_force_release_tx_skbe(struct skb_entry *skbe)
{
	ahu_release_tx_skb(skbe->skb);
}

struct skb_entry *ahu_dequeue_skbe_tx_pool(void)
{
	wait_event_interruptible(ahu->ahu_wq, ahu->skb_tx_pool_qlen);
	if (signal_pending(current)) {
		uinfo("ahu_dequeue_skbe_tx_pool signal catched!\n");
		goto out;
	}

out:
	return dequeue_skb_tx_pool(ahu, 1);
}

int ahu_send(struct skb_entry *skbe)
{
	struct sk_buff *skb = skbe->skb;
	int ret = 0;

	skb->priority = 0x16881688;

	/* For profiling. Skip MAC send. */
	if (skbe->data_size == 0)
		goto drop_1;

	if (netif_queue_stopped(ahu->dev)) {
		ret = -1;
		goto drop;
	}

	skb_put(skb, skbe->data_size);

	if (ftgmac100_wait_to_send_packet(skbe->skb, ahu->dev))
		goto drop;

	return ret;

drop:
	//uerr("ahu_send failed?!\n");
	printk(".");
drop_1:
	ahu_release_tx_skb(skb);
	return ret;
}

void ahu_release_rx_skb(struct skb_entry *skbe)
{
	/*free skb*/
	dev_kfree_skb(skbe->skb);
	skbe->skb = NULL;
	skbe->data_ptr = NULL;
	skbe->skb_data_phy = 0;
	skbe->data_size = 0;
	/*free skbe*/
	enqueue_skb_rx_pool(ahu, skbe, 1);
}

struct skb_entry *ahu_dequeue_skb_rx_ready(void)
{
	struct skb_entry *skbe = NULL;

	wait_event_interruptible(ahu->ahu_wq, ahu->skb_rx_rdy_qlen);
	if (signal_pending(current)) {
		uinfo("ahu_dequeue_skb_rx_ready signal catched!\n");
		goto out;
	}
	skbe = dequeue_skb_rx_ready(ahu, 1);
	BUG_ON(!skbe);

out:
	return skbe;
}


static u32 ahu_alert = 0;
/*
 * Without deleting fw file,
 * we got failed to allocate sk_buff in MAC driver if ahu_drop_th = 512
 * in the meanwhile, skb_rx_rdy_qlen is 374
 * So set ahu_drop_th to 256
 */
static u32 ahu_drop_th = 256;
static u32 ahu_drop_release = 32;

void ahu_alert_cfg(u32 en)
{
	ahu_alert = en;
}

int ahu_netif_rx(struct sk_buff *skb)
{
	struct skb_entry *skbe;

	if (!is_ahu_rx_enabled(ahu)) {
		printk("X");
		goto drop_skb;
	}

	if (is_ahu_rx_suspeneded(ahu))
		goto drop_skb;

	if (skb->len < 60) {
		//This is Invalid or VSync packets. Should ignore it.
		/*
		 * VSync packet is 60 bytes
		 * In order to check the Vsync packet sequence, we should keep packets which length is 60 byte
		 */
		goto drop_skb;
	}

	if (ahu_alert) {
		if (ahu_drop_th < ahu->skb_rx_rdy_qlen)
			goto drop_skb;

		if (ahu_drop_release > ahu->skb_rx_rdy_qlen)
			ahu_alert = 0;
	}

	/*dequeue from skbe_rx_pool.*/
	while (!(skbe = dequeue_skb_rx_pool(ahu, 1))) {
		//printk("O\n");
		//TOGGLE_GPIO1();
		/* Drop skbe in rx ready queue. */
		skbe = dequeue_skb_rx_ready(ahu, 1);
		BUG_ON(!skbe);
		ahu_release_rx_skb(skbe);
		if (!is_ahu_rx_enabled(ahu)) {
			goto drop_skb;
		}
	}
	/*attach received skb to skbe. */
	skbe->skb = skb;
	skbe->data_ptr = skb->data;
	skbe->skb_data_phy = virt_to_phys(skb->data);
	skbe->data_size = skb->len;
	/*enqueue to skbe_rx_rdy_list.*/
	enqueue_skb_rx_ready(ahu, skbe, 1);
	/*wakeup waitq.*/
	wake_up_interruptible(&ahu->ahu_wq); //Used to wake up thread waiting for ahu_dequeue_skb_rx_ready().

#if 0
	struct ethhdr *eth;

	//Must be called after eth_type_trans().
	eth = eth_hdr(skb);

	if (compare_ether_addr(eth->h_source, eth_tmp.source))
		goto out;

	/* Matched frame. */
	udump(skb->data, 32);

out:
#endif
	return(ahu->skb_rx_rdy_qlen);

drop_skb:
	dev_kfree_skb(skb);
	return(ahu->skb_rx_rdy_qlen);
}

int ahu_init_tx(unsigned int skb_pool_size, unsigned int skb_size)
{
	struct skb_entry *skb_entry_buf;
	int i;

	INIT_LIST_HEAD(&ahu->skb_tx_pool);
	ahu->skb_tx_pool_qlen = 0;

	ahu->skb_entry_tx_buf = kzalloc(sizeof(struct skb_entry) * skb_pool_size, GFP_KERNEL);
	skb_entry_buf = ahu->skb_entry_tx_buf;
	BUG_ON(!skb_entry_buf);
	//BruceToDo. who is going to release skb_entry_buf??
	for (i = 0; i < skb_pool_size; i++) {
		INIT_LIST_HEAD(&skb_entry_buf->list);
		skb_entry_buf->skb = dev_alloc_skb(skb_size);
		/* We save the skbe ptr into skb->cb so that we can do skb_to_skbe(). */
		*(struct skb_entry **)(skb_entry_buf->skb->cb) = skb_entry_buf;
		skb_entry_buf->data_ptr = skb_entry_buf->skb->data;
		skb_entry_buf->skb_data_phy = virt_to_phys(skb_entry_buf->data_ptr);
		skb_entry_buf->buf_size = skb_size;
		skb_entry_buf->data_size = 0;
		enqueue_skb_tx_pool(ahu, skb_entry_buf, 1);
		skb_entry_buf++;
	}

#if AHU_FPGA_TEST
	ahu_test_init(ahu);
#endif

	return 0;
}

int ahu_clenup_tx(void)
{
	//BruceToDo. dev_kfree_skb() allocated skb in skb_tx_pool
	kfree(ahu->skb_entry_tx_buf);
	return 0;
}

int ahu_start_tx(void)
{
	/* allocate resource based on socket, MAC mtu....*/
	return 0;
}

int ahu_stop_tx(void)
{
	/* revert what is done in ahu_start_tx() */
	return 0;
}

int ahu_init_rx(unsigned int skb_pool_size)
{
	struct skb_entry *skb_entry_buf;
	int i;

	/* init rx only static context data */
	INIT_LIST_HEAD(&ahu->skb_rx_pool);
	INIT_LIST_HEAD(&ahu->skb_rx_rdy_list);
	ahu->skb_rx_rdy_qlen = 0;

	ahu->skb_entry_rx_buf = kzalloc(sizeof(struct skb_entry) * skb_pool_size, GFP_KERNEL);
	skb_entry_buf = ahu->skb_entry_rx_buf;
	BUG_ON(!skb_entry_buf);

	for (i = 0; i < skb_pool_size; i++) {
		INIT_LIST_HEAD(&skb_entry_buf->list);
		skb_entry_buf->skb = NULL;
		enqueue_skb_rx_pool(ahu, skb_entry_buf, 1);
		skb_entry_buf++;
	}

	return 0;
}

int ahu_cleanup_rx(void)
{
	/* revert what is done in ahu_init_rx() */
	/* Make sure ahu_stop_rx() is called before calling this function. */
	kfree(ahu->skb_entry_rx_buf);
	return 0;
}

int ahu_start_rx(u32 dest_ip_addr, u16 dest_udp_port)
{
	if (is_ahu_rx_enabled(ahu)) {
		uerr("Wrong sequence?!\n");
		BUG();
	}
	if (ahu->skb_rx_rdy_qlen) {
		uerr("memory leak?!\n");
		BUG();
	}

	ahu_rx_state_enable(ahu);

	/* start accepting specific udp packet. Set MAC filter based on socket info. */
	mac_ahu_start_rx_filter(ahu->dev, dest_ip_addr, dest_udp_port);
	return 0;
}

int ahu_stop_rx(void)
{
	struct skb_entry *skbe;

	/* stop accepting specific udp packet. Clear MAC filter */
	mac_ahu_stop_rx_filter(ahu->dev);
	ahu_rx_state_disable(ahu);
	/* cleanup skb_rx_rdy_list */
	while ((skbe = dequeue_skb_rx_ready(ahu, 1))) {
		ahu_release_rx_skb(skbe);
		printk(".");
	}
	return 0;
}

void ahu_suspend_rx(void)
{
	ahu_rx_state_suspend(ahu);
}

void ahu_resume_rx(void)
{
	ahu_rx_state_resume(ahu);
}

void ahu_vsync_filter(unsigned int enable, unsigned int sender_id)
{
	// Always enable VSync filter and change the sender_id to toggle the actual ON/OFF behavior.
	if (enable)
		mac_ahu_vsync_filter(ahu->dev, 1, sender_id);
	else
		mac_ahu_vsync_filter(ahu->dev, 1, 0);
}

int ahu_init(struct net_device *dev)
{
	ahu = kzalloc(sizeof(struct ahu_context), GFP_KERNEL);
	BUG_ON(!ahu);

	ahu->dev = dev;
	spin_lock_init(&ahu->lock);
	init_waitqueue_head(&ahu->ahu_wq);

	return 0;
}

int ahu_cleanup(void)
{
	kfree(ahu);
	return 0;
}

EXPORT_SYMBOL(ahu_init);

EXPORT_SYMBOL(ahu_init_tx);
EXPORT_SYMBOL(ahu_release_tx_skb);
EXPORT_SYMBOL(ahu_force_release_tx_skbe);
EXPORT_SYMBOL(ahu_dequeue_skbe_tx_pool);
EXPORT_SYMBOL(ahu_send);

EXPORT_SYMBOL(ahu_start_rx);
EXPORT_SYMBOL(ahu_stop_rx);
EXPORT_SYMBOL(ahu_suspend_rx);
EXPORT_SYMBOL(ahu_resume_rx);
EXPORT_SYMBOL(ahu_init_rx);
EXPORT_SYMBOL(ahu_cleanup_rx);
EXPORT_SYMBOL(ahu_dequeue_skb_rx_ready);
EXPORT_SYMBOL(ahu_release_rx_skb);
EXPORT_SYMBOL(ahu_vsync_filter);

#endif //#if __AHU__

