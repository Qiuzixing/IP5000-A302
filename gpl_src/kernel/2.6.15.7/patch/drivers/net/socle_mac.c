/* socle_mac.c: ethernet driver for the SOCLE MAC core with AMBA interface.

	SOCLE Technology Corp.
	14F-2, No.287, Section 2, Kwan-Fu Road,
	Hsin-Chu City, Taiwan
	http://www.socle-tech.com.tw
*/

/* The include file section.  We start by doing checks and fix-ups for
   missing compile flags. */
#ifndef __KERNEL__
#define __KERNEL__
#endif
#if !defined(__OPTIMIZE__)
#warning  You must compile this file with the correct options!
#warning  See the last lines of the source file.
#error You must compile this driver with "-O".
#endif



#include <asm/io.h>
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/unistd.h>


#include <linux/delay.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <asm/unaligned.h>
#include <asm/uaccess.h>
#include <asm/cpu-single.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <asm/arch/platform.h>
#include <asm/arch/hardware.h>
#include "socle.h"
#include "if_socle.h"
#include "socle_mac.h"



#ifndef MODULE
#define MODULE 1
#endif

#ifndef TRUE
#define TRUE -1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*  The possible media types that can be set in options[] are: */
static const char * const medianame[] = {
	"None",
	"AutoNeg",
	"100baseTxF",
	"100baseTxH",
	"10baseTxF",
	"10baseTxH",
	"FullDuplex",
	"HalfDuplex",
	"100Base",
	"10Base",
};

enum {
	options_None		=0,
	options_AutoNeg		=1,
	options_100baseTxF	=2,
	options_100baseTxH	=3,
	options_10baseTxF	=4,
	options_10baseTxH	=5,
	options_FullDuplex	=6,
	options_HalfDuplex	=7,
	options_100Base		=8,
	options_10Base		=9,
};



#define DRV_NAME    "MAC"
#define DRV_VERSION "v1.0"
#define DRV_RELDATE "DEC 30, 2005"
#define DRV_BUSMODE "Embedded AMBA"

/* Version strings */
static const char version[] =
DRV_NAME " : " DRV_VERSION " " DRV_RELDATE " "  DRV_BUSMODE " " "\n";

/* SOCLE_MAC:
	Before we use this driver there have some Pre-define value should be include
	in  SOCLE_MAC_Driver_Value.h"
	All Board Specification Configure should be included*/

#ifdef SOCLE_MAC_DEBUG
	#define MAC_KERN_INFO		KERN_INFO
	#define MAC_KERN_DEBUG		KERN_INFO
	#define MAC_KERN_ERR    	KERN_INFO
	#define MAC_KERN_WARNING	KERN_INFO
	#define DEBUG_LEVEL			31
	static void socle_mac_hwcsrshow(struct net_device *dev);
	static void socle_mac_desshow(struct net_device *dev, int txrx, int *despt);
#else
	#define MAC_KERN_INFO		KERN_INFO
	#define MAC_KERN_DEBUG		KERN_INFO //KERN_DEBUG
	#define MAC_KERN_ERR    	KERN_ERR
	#define MAC_KERN_WARNING	KERN_INFO //KERN_WARNING
	#define DEBUG_LEVEL			1
	#define socle_mac_hwcsrshow(x)
	#define socle_mac_desshow(x,y,z)
#endif
/* Condensed operations for readability. */
//We didn't enable DMA in AST2000
//#define virt_to_le32desc(addr)  cpu_to_le32(virt_to_bus(addr))
//#define le32desc_to_virt(addr)  le32_to_cpu(bus_to_virt(addr))
#define virt_to_le32desc(addr)  cpu_to_le32(virt_to_phys(addr))
#define le32desc_to_virt(addr)  le32_to_cpu(phys_to_virt(addr))

#if (LINUX_VERSION_CODE >= 0x20100)  &&  defined(MODULE)
char kernel_version[] = UTS_RELEASE;
#endif

struct net_device *DEV=NULL;
//#define UP_IRQ_BASE 0
/**************************************************************************************
   Modules parameter
   The user-configurable values.
   These may be modified when a driver module is loaded.
**************************************************************************************/
static int	mac_max_dev = MAC_MAX_UNITS;	/* Maximum device for modules install */
static int	mac_active_dev = 0;     	/* The first one intsall device */

static unsigned int mac_amba_base_address[MAC_MAX_UNITS] =
	{
		  CONFIG_SOCLEMAC_BASE_ADDR1
		#if (MAC_MAX_UNITS > 1)
		, CONFIG_SOCLEMAC_BASE_ADDR2
		#endif
	};

static unsigned int mac_amba_irq[MAC_MAX_UNITS] =
	{
		  (CONFIG_SOCLEMAC_INTV1+UP_IRQ_BASE)
		#if (MAC_MAX_UNITS > 1)
		, (CONFIG_SOCLEMAC_INTV2+UP_IRQ_BASE)
		#endif
	};

static int	mac_debug_level = DEBUG_LEVEL;	/* Message enable: 0..31 = no..all messages. */
static int	max_interrupt_work = 25;		/* Maximum events (Rx packets, etc.) to handle at each interrupt. */
static int	rx_copythreshold = 1518;		/* Set the copy breakpoint for the copy-only-tiny-buffer Rx structure. */
static int	csr0 = 0;						/* csr0 setting value */
static int	multicast_filter_limit = 16;	/* Maximum number of multicast addresses to filter (vs. rx-all-multicast). */
											/* This value does not apply to the 512 bit hash table. */
static int	options[MAC_MAX_UNITS] = {0, };	/* force transceiver type or fixed speed+duplex */
static int	mtu[MAC_MAX_UNITS] = {0, };		/* Jumbo MTU for interfaces. */


MODULE_AUTHOR("SOCLE_MAC.c: Sherlock Wang at Socle-Tech http://www.socle-tech.com.tw");
MODULE_DESCRIPTION("SOCLE MAC AMBA ethernet driver");
MODULE_LICENSE("GPL");

MODULE_PARM(mac_max_dev, "i");
MODULE_PARM(mac_active_dev, "i");
MODULE_PARM(mac_amba_base_address, "1-" __MODULE_STRING(MAC_MAX_UNITS) "i");
MODULE_PARM(mac_amba_irq, "1-" __MODULE_STRING(MAC_MAX_UNITS) "i");

MODULE_PARM(mac_debug_level, "i");
MODULE_PARM(max_interrupt_work, "i");
MODULE_PARM(rx_copythreshold, "i");
MODULE_PARM(csr0, "i");
MODULE_PARM(multicast_filter_limit, "i");
MODULE_PARM(options, "1-" __MODULE_STRING(MAC_MAX_UNITS) "i");
MODULE_PARM(mtu, "1-" __MODULE_STRING(MAC_MAX_UNITS) "i");

#ifdef MODULE_PARM_DESC

MODULE_PARM_DESC(mac_max_dev,
				 "SOCLE MAC driver maximum devices to install");
MODULE_PARM_DESC(mac_active_dev,
				 "SOCLE MAC driver install from which one");
MODULE_PARM_DESC(mac_amba_base_address,
				 "SOCLE MAC driver AMBA base address");
MODULE_PARM_DESC(mac_amba_irq,
				 "SOCLE MAC driver IRQ number");

MODULE_PARM_DESC(mac_debug_level,
				 "SOCLE MAC driver message level (0-31)");
MODULE_PARM_DESC(max_interrupt_work,
				 "SOCLE MAC driver maximum events handled per interrupt");
MODULE_PARM_DESC(rx_copythreshold,
				 "SOCLE MAC threshold in bytes for copy-only-tiny-frames");
MODULE_PARM_DESC(csr0,
				 "SOCLE MAC driver CSR0 setting");
MODULE_PARM_DESC(multicast_filter_limit,
				 "SOCLE MAC threshold for switching to Rx-all-multicast");
MODULE_PARM_DESC(options,
				 "SOCLE MAC force transceiver type or fixed speed+duplex");
MODULE_PARM_DESC(mtu,
				 "SOCLE MAC Jumbo MTU for interfaces");
#endif

/* SOCLE_MAC:
	For the SOCLE_MAC core with AMBA, it may be necessary to change this
	selection based on the CPU used and the hardware design. */
#undef inb
#undef inw
#undef inl
#undef outb
#undef outw
#undef outl
#define inb readb
#define inw readw
#define inl readl
#define outb writeb
#define outw writew
#define outl writel


/*
				Theory of Operation

I. Board Compatibility

This device driver is designed for the Socle MAC IP or SOC Modules.
For SOC Modules, fully test on Socle uDK and LeopartII Platform.

II. Board-specific settings

Please check the menuconfig and the include file SOLCE_MAC_Driver_Value.c.
We remove all Board-specific settings & Complier time setting to menuconfig and this file.
The most important setting is
	- Platform type
	- System Endian
	- Address Map
	- IRQ Vector
	- Performace Parameter
	- EEPROM
Some boards have EEPROMs mount on MAC modules.  The factory default
is usually "autoselect".  This should only be overridden when using
transceiver connections without link beat e.g. 10base2 or AUI, or (rarely!)
for forcing full-duplex when used with old link partners that do not do
autonegotiation.

III. Driver operation

IIIa. Ring buffers

The Socle MAC can use either ring buffers or lists of Tx and Rx descriptors.
This driver uses statically allocated rings of Rx and Tx descriptors, set at
compile time by RX/TX_RING_SIZE.  This version of the driver allocates skbuffs
for the Rx ring buffers at open() time and passes the skb->data field  as
receive data buffers.  When an incoming frame is less than rx_copythreshold bytes
long, a fresh skbuff is allocated and the frame is copied to the new skbuff.
When the incoming frame is larger, the skbuff is passed directly up the protocol
stack and replaced by a newly allocated skbuff.

The rx_copythreshold value is chosen to trade-off the memory wasted by
using a full-sized skbuff for small frames vs. the copying costs of larger
frames.  For small frames the copying cost is negligible (esp. considering
that we are pre-loading the cache with immediately useful header
information).  For large frames the copying cost is non-trivial, and the
larger copy might flush the cache of useful data.

IIIC. Synchronization
The driver runs as two independent, single-threaded flows of control.  One
is the send-packet routine, which enforces single-threaded use by the
dev->tbusy flag.  The other thread is the interrupt handler, which is single
threaded by the hardware and other software.

The send packet thread has partial control over the Tx ring and 'dev->tbusy'
flag.  It sets the tbusy flag whenever it is queuing a Tx packet. If the next
queue slot is empty, it clears the tbusy flag when finished otherwise it sets
the 'tp->tx_full' flag.

The interrupt handler has exclusive control over the Rx ring and records stats
from the Tx ring.  (The Tx-done interrupt can not be selectively turned off, so
we cannot avoid the interrupt overhead by having the Tx routine reap the Tx
stats.)	 After reaping the stats, it marks the queue entry as empty by setting
the 'base' to zero.	 If the 'tp->tx_full' flag is set, it clears both the
tx_full and tbusy flags.

IV. Notes
The driver parameter have 3 way to setting
	1.	Load time paramter set
	2.	SROM (EEPROM) Conternt
	3.  Complier time default
the setting priority is 1 > 2 > 3
When load time not issue parameter value, we will try to use EEPROM content. If both
not issue then will use the default setting.

IVb. References

http://www.socle-tech.com.tw

IVc. Errata

*/

/* The SOCLE MAC Rx and Tx buffer descriptors. */
struct socle_mac_rx_desc {
	s32 status;
	s32 length;
	u32 buffer1, buffer2;			/* We use only buffer 1.  */
};

struct socle_mac_tx_desc {
	s32 status;
	s32 length;
	u32 buffer1, buffer2;			/* We use only buffer 1.  */
};

#define PRIV_ALIGN	15	/* Required alignment mask */


/* SOCLE_MAC:
	The SOCLE MAC device driver private struct.
*/
struct socle_mac_private {
	/* Please carefully design the memory. Make sure the DMA access is longword aligned */
	struct	socle_mac_rx_desc 	rx_ring_a[RX_RING_SIZE];
	struct	socle_mac_tx_desc 	tx_ring_a[TX_RING_SIZE];
	u16 						setup_frame_a[96];	/* Pseudo-Tx frame to init address table. */

	struct	socle_mac_rx_desc	*rx_ring;
	struct	socle_mac_tx_desc	*tx_ring;
	u16 						*setup_frame;		/* Pseudo-Tx frame to init address table. */
	/* The saved addresses of Rx/Tx-in-place packet buffers. */
	struct	sk_buff* 			tx_skbuff[TX_RING_SIZE];
	struct	sk_buff* 			rx_skbuff[RX_RING_SIZE];
	struct	net_device 			*next_module;

	/* Multicast filter control. */
	int 						multicast_filter_limit;

	int 						chip_id;
	int 						max_interrupt_work;
	int 						msg_level;
	unsigned int 				csr0;				/* Current CSR0 settings. */
	unsigned int 				csr6;				/* Current CSR6 settings. */
	unsigned int 				csr11;				/* Current CSR11 settings. */

	spinlock_t 					lock;

	struct 	net_device_stats 	stats;

	/* Note: cache line pairing and isolation of Rx vs. Tx indicies. */
	unsigned int 				cur_rx;				/* Producer ring indices */
	unsigned int 				dirty_rx;			/* consumer ring indices */
	unsigned int 				rx_buf_sz;			/* Based on MTU+slack. */
	int 						rx_copythreshold;

	unsigned int 				cur_tx;
	unsigned int 				dirty_tx;

	unsigned int 				tx_full:1;			/* The Tx queue is full. */
	unsigned int 				rx_dead:1;			/* We have no Rx buffers. */

	/* Media selection state. */
	unsigned int 				full_duplex:1;		/* Full-duplex operation requested. */
	unsigned int 				full_duplex_lock:1;
	unsigned int 				speed_100M:1;		/* Speed_100M operation requested. */
	unsigned int 				speed_100M_lock:1;

	unsigned int 				medialock:1;		/* Do not sense media type. */
	unsigned int 				mediasense:1;		/* Media sensing in progress. */
	unsigned int 				medialink:1;		/* Media link success. */
	unsigned int 				medianeq:1;			/* Media Auto Neq success. */
	unsigned int				rmii:1;			/*Fatmouse Add for check hardware trap */

	struct 	timer_list 			timer;				/* Media selection timer. */

	spinlock_t 					mii_lock;
	u16 						mii_lpar;			/* MII Link partner ability. */
	u16 						mii_advertise;		/* MII advertise, from SROM table. */
	signed char 				phys;				/* MII device addresses. */


};


/* SOCLE_MAC:
	The function pre-claim.
*/
/********************************** Module Part 			***************************/
#ifdef MODULE
static int __init	init_mac_module(void);
static void __exit	cleanup_mac_module(void);
static void 		cleanup_mac(void);
#endif

/********************************** Function Part 			***************************/
       int 						socle_mac_probe(void);
static void 					*socle_mac_probe_one( void *init_dev, long ioaddr, int irq, int chip_idx);
static int						socle_mac_open(struct net_device *dev);
static int						socle_mac_start_xmit(struct sk_buff *skb, struct net_device *dev);
static void 					socle_mac_clean_tx_ring(struct net_device *dev);
static void						socle_mac_down(struct net_device *dev);
static int 						socle_mac_close(struct net_device *dev);
static struct net_device_stats 	*socle_mac_get_stats(struct net_device *dev);

#ifdef SOCLE_MAC_HAVE_PRIVATE_IOCTL
static int 						netdev_ethtool_ioctl(struct net_device *dev, void *useraddr);
static int 						private_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
#endif
static void 					set_rx_mode(struct net_device *dev);
static void 					socle_mac_timer(unsigned long data);

/********************************** Call Function Part 		***************************/
static int						socle_mac_start_link(struct net_device *dev);
static void 					check_media_link(struct net_device *dev, int init);
static void 					wait_txrxstop(struct net_device *dev);
static void 					start_rxtx(struct net_device *dev);
static void 					restart_rxtx(struct net_device *dev);
static void						update_csr6_speed_duplex(struct net_device *dev, int init);
static int  					check_speed_duplex(struct net_device *dev, int init, int mii_sr);
static inline u32 				ether_crc(int length, unsigned char *data);
static inline u32 				ether_crc_le(int length, unsigned char *data);
static void 					socle_mac_init_ring(struct net_device *dev);
static void 					empty_rings(struct net_device *dev);

/********************************** Interrupt Function Part ***************************/
static irqreturn_t 					socle_mac_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
static void 					socle_mac_tx_timeout(struct net_device *dev);
static int 						socle_mac_refill_rx(struct net_device *dev);
static int 						socle_mac_rx(struct net_device *dev);

/***** Added for fixing Dribble bit error *****/
static void                     socle_mac_reset_ring(struct net_device *dev);



/* A list of all installed SOCLE MAC devices. */
static struct net_device *root_socle_mac_dev = NULL;

#include "socle_mii.c"

#ifdef MODULE
/* init_mac_module() */
static int __init
init_mac_module(void)
{
	if (mac_debug_level >= NETIF_MSG_DRV)	/* Emit version even if no cards detected. */
	{
		printk(MAC_KERN_INFO "%s" , version);
	}
	return(socle_mac_probe());
} /* init_mac_module() */

/* cleanup_mac() */
static void cleanup_mac(void)
{
	struct net_device *next_dev;

	/* No need to check MOD_IN_USE, as sys_delete_module() checks. */
	while (root_socle_mac_dev)
	{
		struct socle_mac_private *cap =
			(struct socle_mac_private*)root_socle_mac_dev->priv;

		next_dev = cap->next_module;

		unregister_netdev(root_socle_mac_dev);

		kfree(root_socle_mac_dev);

		root_socle_mac_dev = next_dev;
	}
} /* cleanup_mac() */

/* cleanup_mac_module() */
static void __exit
cleanup_mac_module(void)
{
	cleanup_mac();
} /* cleanup_mac_module() */

module_init(init_mac_module);
module_exit(cleanup_mac_module);
#endif


/* socle_mac_probe()
	probe socle mac devices
*/
int __init socle_mac_probe(void)
{
	int	i;
	int module_installed = 0;

	for(i=mac_active_dev;i<mac_max_dev;i++)
	{
		printk(MAC_KERN_INFO "Probe MAC module at <%8.8x> IRQ %d\n",
							MAC_AMBA_BASE_ADDR(i), MAC_AMBA_IRQ(i));
		if (socle_mac_probe_one(NULL, MAC_AMBA_BASE_ADDR(i), MAC_AMBA_IRQ(i), i) == NULL)
		{
			//Any wrong cleanup all install module before
			printk(MAC_KERN_INFO "Probe MAC module Fail !!!!!!!!!!! at <%8.8x> IRQ %d\n", MAC_AMBA_BASE_ADDR(i), MAC_AMBA_IRQ(i));
		}
		else
		{
			module_installed = TRUE;
			printk(MAC_KERN_INFO "Probe MAC module at <%8.8x> IRQ %d : Pass---------------------\n", MAC_AMBA_BASE_ADDR(i), MAC_AMBA_IRQ(i));
		}
	}
	if(module_installed == TRUE)
		return 0;
	else
		return -ENODEV;

} /* socle_mac_probe() */

/* socle_mac_probe_one()
	probe socle mac device by one
*/
static void *socle_mac_probe_one( void *init_dev, long ioaddr, int irq, int chip_idx)
{
	struct net_device 			*dev;
	struct socle_mac_private 	*cap;
	static unsigned char 		last_phys_addr[2][6];
	int i;

	if (mac_debug_level >= NETIF_MSG_DRV)	/* Emit version even if no cards detected. */
	{
		printk(MAC_KERN_INFO "Probe ... MAC  : Device chip index<%d>\n", chip_idx);
	}

	if ( (inl(ioaddr + CSR_OFFSET(CSR5)) == 0xffffffff)
	   ||(inl(ioaddr + CSR_OFFSET(CSR5)) == 0x00000000) )
	{
		printk(MAC_KERN_ERR "The SOCLE MAC moudle (%d) at %#lx is not functioning.\n", chip_idx, ioaddr);
		return NULL;
	}

	/* alloc_etherdev ensures aligned and zeroed private structures */
	dev = alloc_etherdev (sizeof (*cap));
	if (!dev)
	{
		printk(MAC_KERN_ERR "!!!! MAC ERROR : alloc_etherdev Fail.\n");
		return NULL;
	}
	cap = dev->priv;

	cap->rx_ring =
		(struct  socle_mac_rx_desc *)
		ioremap_nocache (virt_to_phys (cap->rx_ring_a),sizeof(struct  socle_mac_rx_desc) *RX_RING_SIZE);
	cap->tx_ring =
		(struct  socle_mac_tx_desc *)
		ioremap_nocache (virt_to_phys (cap->tx_ring_a),sizeof (struct socle_mac_tx_desc) * TX_RING_SIZE);
	cap->setup_frame =
		(u16*) ioremap_nocache (virt_to_phys (cap->setup_frame_a), (sizeof (u16) * 96));

	/* Template give a device name for debug/information */
	strcpy(dev->name, "Probe ... MAC ");

	/* Recode every install socle mac modules
	   First Install, Last Remove */
	cap->next_module = root_socle_mac_dev;
	root_socle_mac_dev = dev;

	printk(MAC_KERN_INFO "%s : %s at %#3lx IRQ %d\n",
		   dev->name, "MAC core w/AMBA", ioaddr, irq);

	/* initial spinlock flag */
	spin_lock_init(&cap->lock);
	spin_lock_init(&cap->mii_lock);
	init_timer(&cap->timer);
	cap->timer.data = (unsigned long)dev;
	cap->timer.function = socle_mac_timer;

	/* Reset the MAC chip */
	outl(CSR0_SWR, ioaddr + CSR_OFFSET(CSR0));

	memcpy(dev->dev_addr,&last_phys_addr[chip_idx],6);
	last_phys_addr[chip_idx][0] += 2;
	dev->if_port = DEFAULT_IFPORTE;
	cap->csr0 = DEFAULT_CSR0_SETTING;
	cap->csr6 = DEFAULT_CSR6_SETTING;
	cap->csr11 = DEFAULT_CSR11_SETTING;

	//Second check driver parameter override the setting
	if(options[chip_idx])
	{
		dev->if_port = options[chip_idx];
		printk(MAC_KERN_DEBUG "%s : Module load force Transceiver selection to %s.\n", dev->name, medianame[options[chip_idx]]);
	}

	switch(dev->if_port)
	{
		case options_AutoNeg :	//Auto Neg
			cap->medialock = 0;
			cap->full_duplex_lock = 0;
			cap->speed_100M_lock = 0;
			cap->mii_advertise = 0;
		break;
		case options_100baseTxF :	//100TxF
			cap->medialock = 1;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 1;
			cap->full_duplex = 1;
			cap->speed_100M = 1;
			cap->mii_advertise = MII_PHY_ANA_100F;
		break;
		case options_100baseTxH :	//100TxH
			cap->medialock = 1;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 1;
			cap->full_duplex = 0;
			cap->speed_100M = 1;
			cap->mii_advertise = MII_PHY_ANA_100H;
		break;
		case options_10baseTxF :	//10TxF
			cap->medialock = 1;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 1;
			cap->full_duplex = 1;
			cap->speed_100M = 0;
			cap->mii_advertise = MII_PHY_ANA_10F;
		break;
		case options_10baseTxH :	//10TxH
			cap->medialock = 1;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 1;
			cap->full_duplex = 0;
			cap->speed_100M = 0;
			cap->mii_advertise = MII_PHY_ANA_10H;
		break;
		case options_FullDuplex :	//FullDuplex
			cap->medialock = 0;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 0;
			cap->full_duplex = 1;
			cap->mii_advertise = MII_PHY_ANA_FDAM;
		break;
		case options_HalfDuplex :	//HalfDuplex
			cap->medialock = 0;
			cap->full_duplex_lock = 1;
			cap->speed_100M_lock = 0;
			cap->full_duplex = 0;
			cap->mii_advertise = MII_PHY_ANA_HDAM;
		break;
		case options_100Base :	//100base
			cap->medialock = 0;
			cap->full_duplex_lock = 0;
			cap->speed_100M_lock = 1;
			cap->speed_100M = 1;
			cap->mii_advertise = MII_PHY_ANA_100M;
		break;
		case options_10Base :	//10base
			cap->medialock = 0;
			cap->full_duplex_lock = 0;
			cap->speed_100M_lock = 1;
			cap->speed_100M = 0;
			cap->mii_advertise = MII_PHY_ANA_10M;
		break;
		default :
		break;
	}
	printk(MAC_KERN_DEBUG "%s : Module load if_port be %s.\n",
			dev->name, medianame[dev->if_port]);

	if(mtu[chip_idx] > 0)
	{
		dev->mtu = mtu[chip_idx];
		printk(MAC_KERN_DEBUG "%s : Module load force MTU %d.\n", dev->name, dev->mtu);
	}
	if(csr0)
	{
		cap->csr0 = csr0;
		printk(MAC_KERN_DEBUG "%s : Module load force csr0<%8.8x>.\n", dev->name, cap->csr0);
	}

	printk(MAC_KERN_INFO "%s : MAC address <", dev->name);
	for (i = 0; i < 6; i++)
	{
		printk(MAC_KERN_INFO "%c%2.2X", i ? ':' : ' ', dev->dev_addr[i]);
	}
	printk(MAC_KERN_INFO " >\n");

	dev->base_addr = ioaddr;
	dev->irq = irq;

	cap->msg_level = (1 << mac_debug_level) - 1;
	cap->chip_id = chip_idx;

	cap->rx_copythreshold = rx_copythreshold;
	cap->max_interrupt_work = max_interrupt_work;
	cap->multicast_filter_limit = multicast_filter_limit;

	/* The entries in the device structure. */
	dev->open = socle_mac_open;
	dev->stop = socle_mac_close;
	dev->hard_start_xmit = socle_mac_start_xmit;
	dev->get_stats = socle_mac_get_stats;
	dev->do_ioctl = private_ioctl;
	dev->set_multicast_list = set_rx_mode;

	dev->tx_timeout = socle_mac_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;


	if(socle_mac_start_link(dev) == FALSE)
	{
		printk(MAC_KERN_ERR "%s : start link Fail!!!!!!!!!!!!\n", dev->name);
		// Release the un-success intall device
		root_socle_mac_dev = cap->next_module;
		kfree (dev);
		return NULL;
	}

	dev->name[0] = 0;	//EOF
	if (register_netdev(dev))
	{
		printk(MAC_KERN_ERR "Probe ... MAC %d  : register_netdev Fail!!!!!!!!!!!!\n", chip_idx);
		// Release the un-success intall device
		root_socle_mac_dev = cap->next_module;
		kfree (dev);
		return NULL;
	}

	printk(MAC_KERN_INFO "Probe ... MAC : Success register in netdev : %s.\n", dev->name);

	ether_setup(dev);
	return dev;

} /* socle_mac_probe() */


/* socle_mac_start_link()
   Start the link, typically called at socle_mac_probe() time but sometimes later with
   multiport cards. */
static int socle_mac_start_link(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int phy;

	/* Find the connected MII xcvrs.
	   Doing this in open() would allow detecting external xcvrs later,
	   but takes much time. */
	for (phy = 0; phy < MAC_MAX_PHY ; phy++)
	{
		u16	miiData_ID0;
		u16	miiData_ID1;

		miiData_ID0 = sMacMiiPhyRead(dev, phy, MII_PHY_ID0);
		miiData_ID1 = sMacMiiPhyRead(dev, phy, MII_PHY_ID1);

		if (miiData_ID0 != 0xFFFF) /* Found PHY */
	    {
			void *scu1;
		    /* here we detect hardware trapping ROMA16*/
    		scu1 = ioremap( 0x1E6E0100, PAGE_SIZE );
    		cap->rmii = (*((unsigned long *)scu1+0x1c) & 0x10000) >> 16;
    		iounmap (scu1);

	    	cap->phys = phy;

			printk (MAC_KERN_INFO "%s:  MII PHY #%d : ID0<%4.4x> ID1<%4.4x> in %s Mode\n", dev->name, phy, miiData_ID0, miiData_ID1, cap->rmii ? "RMII": "MII");
			break;
		}
	}

	if (phy == MAC_MAX_PHY)
	{
		printk (MAC_KERN_INFO "%s: ***WARNING***: No MII PHY found!\n", dev->name);
		cap->phys = MAC_MAX_PHY;
		return FALSE;
	}
	else
	{
		u16 mii_cr = sMacMiiPhyRead(dev, phy, MII_PHY_CR);
		u16 mii_advert = sMacMiiPhyRead(dev, phy, MII_PHY_ANA);
		u16 mii_status = sMacMiiPhyRead(dev, phy, MII_PHY_SR);
		u16 to_advert;

		if (cap->mii_advertise)	/* only CSMA support by phy now */
			to_advert = cap->mii_advertise | MII_PHY_ANA_CSMA;
		else					/* Leave unchanged. */
			cap->mii_advertise = to_advert = mii_advert;

		printk(MAC_KERN_DEBUG "%s:  MII PHY #%d :" "config<%4.4x> status<%4.4x> advertising<%4.4x>.\n", dev->name, phy, mii_cr, mii_status, mii_advert);

		if (mii_advert != to_advert)
		{
			printk(MAC_KERN_DEBUG "%s:  Advertising %4.4x on PHY %d," " previously advertising %4.4x.\n",  dev->name, to_advert, phy, mii_advert);
			sMacMiiPhyWrite(dev, phy, MII_PHY_ANA, to_advert);
		}
		if (cap->medialock)
		{
			//Even force mode we still need our partner know so enable the restart
			//auto negotiation
			u16 mii_mode = 0;

			if(cap->full_duplex)
				mii_mode |= MII_PHY_CR_FDX;
			if (cap->speed_100M)
			{
				//For RMII
				mii_mode |= MII_PHY_CR_100M;
				outl (inl(dev->base_addr+CSR_OFFSET(CSR6))|CSR6_SPEED, dev->base_addr + CSR_OFFSET(CSR6));
			}
			else
				outl(inl(dev->base_addr+CSR_OFFSET(CSR6))&(~CSR6_SPEED), dev->base_addr + CSR_OFFSET(CSR6));

			printk (MAC_KERN_DEBUG "%s:  MediaLock MII Mode %4.4x on PHY %d\n", dev->name, mii_mode, phy);
			/* force MII mode setting */
			sMacMiiPhyWrite(dev, phy, MII_PHY_CR, mii_mode|MII_PHY_CR_RAN);
		}
		else
		{
			/* Enable autonegotiation */
			sMacMiiPhyWrite(dev, phy, MII_PHY_CR, MII_PHY_CR_AUTO|MII_PHY_CR_RAN);
		}
		// Because we Re-enable autonegotiation
		// Need a dumy Readback status
		mii_status = sMacMiiPhyRead(dev, phy, MII_PHY_SR);
		//For MII
	}

	return TRUE;
} /* socle_mac_start_link() */


/* socle_mac_open()
	Open Driver. The real working function. */
static int
socle_mac_open(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;

	int retval;

	if (cap->msg_level & NETIF_MSG_IFUP) {
		printk("%s: START OPEN --- socle_mac_open().\n", dev->name);
        }
	outl(CSR0_SWR, ioaddr + CSR_OFFSET(CSR0));

	/* This would be done after interrupts are initialized, but we do not want
	   to frob the transceiver only to fail later. */

	//First Set the triger level of interrupt controller
	if ( (retval = request_irq(dev->irq, &socle_mac_interrupt, SA_SHIRQ, dev->name, dev) ) )
	{
		return retval;
	}

	outl(0x00000001, ioaddr + CSR_OFFSET(CSR0)); //+grant
	udelay(100);
	/* Deassert reset.
	   Wait the specified 50 PCI cycles after a reset by initializing
	   Tx and Rx queues and the address filter list. */
	outl(cap->csr0, ioaddr + CSR_OFFSET(CSR0));
	udelay(100);

	if (cap->msg_level & NETIF_MSG_IFUP) {
		printk("%s: socle_mac_open() irq %d.\n", dev->name, dev->irq);
        }
	socle_mac_init_ring(dev);

	/* CSR 3 contains the address ofthe first descriptor in a receive
		descriptor list.  This address should be longword aligned
		(RLA(1..0)=00+.  */
	/* CSR 4 contains the address of the first descriptor in a transmit
		descriptor list.  This address should be longword aligned
		(TLA(1..0)=00).  */
	outl(virt_to_phys(cap->rx_ring_a), ioaddr + CSR_OFFSET(CSR3));
	outl(virt_to_phys(cap->tx_ring_a), ioaddr + CSR_OFFSET(CSR4));

	/* Check media link ok */
	check_media_link(dev,1);

	set_rx_mode(dev);

	/* Start the Tx to process setup frame. */
	outl(cap->csr6, ioaddr + CSR_OFFSET(CSR6));
	outl(cap->csr6 | CSR6_ST, ioaddr + CSR_OFFSET(CSR6));

	printk("%s: Enable Interrupt ... Now!!!!!\n",
			dev->name);

	/* Clear Interrupt. */
	outl(SOCLE_MAC_INTR, ioaddr + CSR_OFFSET(CSR5));
	/* Enable interrupts by setting the interrupt mask. */
	outl(SOCLE_MAC_INTR, ioaddr + CSR_OFFSET(CSR7));


	outl(cap->csr6 | CSR6_ST | CSR6_SR , ioaddr + CSR_OFFSET(CSR6));
	outl(0, ioaddr + CSR_OFFSET(CSR2));		/* Rx poll demand */

	if (cap->msg_level & NETIF_MSG_IFUP)
		printk(MAC_KERN_DEBUG "%s: Done socle_mac_open(), CSR0<%8.8x>, CSR5<%8.8x> CSR6<%8.8x> CSR7<%8.8x>.\n", dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR0)), (int)inl(ioaddr + CSR_OFFSET(CSR5)), (int)inl(ioaddr + CSR_OFFSET(CSR6)), (int)inl(ioaddr + CSR_OFFSET(CSR7)));

	/* Set the timer to switch to check for link beat and perhaps switch
	   to an alternate media type. */
	if(cap->medialink && cap->medianeq)
	{
		//Link OK use Slow Timer
		cap->timer.expires = jiffies + LINK_CHECK_SLOW;
	}
	else
	{
		cap->timer.expires = jiffies + LINK_CHECK_FAST;
	}
	add_timer(&cap->timer);

	netif_start_queue(dev);

	/* Show MAC CSR content */
	socle_mac_hwcsrshow(dev);

	return 0;
} /* socle_mac_open() */


/* socle_mac_init_ring() */
/*	Initialize the Rx and Tx rings, along with various 'dev' bits. */
static void socle_mac_init_ring(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int i;
	struct	socle_mac_rx_desc	*phypt_rx_ring;
	struct	socle_mac_tx_desc	*phypt_tx_ring;

	cap->rx_dead = cap->tx_full = 0;
	cap->cur_rx = cap->cur_tx = 0;
	cap->dirty_rx = cap->dirty_tx = 0;

	cap->rx_buf_sz = dev->mtu + 18;
	if (cap->rx_buf_sz < PKT_BUF_SZ)
		cap->rx_buf_sz = PKT_BUF_SZ;

	for (i = 0; i < RX_RING_SIZE; i++)
	{
		phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));
		phypt_rx_ring->status = 0x00000000;
		phypt_rx_ring->length = cpu_to_le32(cap->rx_buf_sz);
		phypt_rx_ring->buffer2 = virt_to_le32desc(&cap->rx_ring_a[i+1]);
		cap->rx_skbuff[i] = NULL;
	}
	/* Mark the last entry as wrapping the ring. */
	phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[RX_RING_SIZE-1]));
	phypt_rx_ring->length |= cpu_to_le32(RDESC1_RER);
	phypt_rx_ring->buffer2 = virt_to_le32desc(&cap->rx_ring_a[0]);

	for (i = 0; i < RX_RING_SIZE; i++)
	{
		/* Note the receive buffer must be longword aligned.
		   dev_alloc_skb() provides 16 byte alignment.  But do *not*
		   use skb_reserve() to align the IP header! */
		struct sk_buff *skb = dev_alloc_skb(cap->rx_buf_sz);
		cap->rx_skbuff[i] = skb;
		if (skb == NULL)
			break;
		skb->dev = dev;			/* Mark as being used by this device. */
		phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));
		phypt_rx_ring->status = cpu_to_le32(RDESC0_OWN);
		phypt_rx_ring->buffer1 = virt_to_le32desc(skb->tail);
	}
	cap->dirty_rx = (unsigned int)(i - RX_RING_SIZE);

	/* The Tx buffer descriptor is filled in as needed, but we
	   do need to clear the ownership bit. */
	for (i = 0; i < TX_RING_SIZE; i++)
	{
		phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[i]));
		cap->tx_skbuff[i] = NULL;
		phypt_tx_ring->status = 0x00000000;
		phypt_tx_ring->buffer2 = virt_to_le32desc(&cap->tx_ring_a[i+1]);
	}
	/* Mark the last entry as wrapping the ring. */
	phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[TX_RING_SIZE-1]));
	phypt_tx_ring->length |= cpu_to_le32(TDESC1_TER);  /* 0516_Gary */
	phypt_tx_ring->buffer2 = virt_to_le32desc(&cap->tx_ring_a[0]);
} /* socle_mac_init_ring() */

/* 0517_Gary Added for fixing Dribble bit error */
static void socle_mac_reset_ring(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int i;
	struct	socle_mac_rx_desc	*phypt_rx_ring;
	struct	socle_mac_tx_desc	*phypt_tx_ring;
        struct  sk_buff *skb;
	long ioaddr = dev->base_addr;
	unsigned int cr0, cr6, cr11;

	spin_lock(&cap->lock);
    /* Flush transmit descriptor */
        cap->tx_full = 1;
	netif_stop_queue(dev);

	cr0 = inl(ioaddr + CSR_OFFSET(CSR0));
	cr6 = inl(ioaddr + CSR_OFFSET(CSR6));
	cr11 = inl(ioaddr + CSR_OFFSET(CSR11));

	outl(cr6 & ~CSR6_SR, ioaddr + CSR_OFFSET(CSR6));

	while (cap->cur_tx != cap->dirty_tx){
	  int entry = cap->dirty_tx % TX_RING_SIZE;
	  int status;

	  phypt_tx_ring = (struct socle_mac_tx_desc *)(&(cap->tx_ring[entry]));
	  status = le32_to_cpu(phypt_tx_ring->status);

	  if (status & TDESC0_OWN)
			continue;			/* It still has not been Txed */

          cap->dirty_tx++;
	  /* Free the original skb. */
	  dev_kfree_skb_irq(cap->tx_skbuff[entry]);
	  cap->tx_skbuff[entry] = NULL;
    }

	outl(CSR0_SWR, ioaddr + CSR_OFFSET(CSR0));
	outl(CSR0_SWR, ioaddr + CSR_OFFSET(CSR0));
	outl(CSR0_SWR, ioaddr + CSR_OFFSET(CSR0));

	cap->rx_dead = cap->tx_full = 0;
	cap->cur_rx = cap->cur_tx = 0;
	cap->dirty_rx = cap->dirty_tx = 0;

	for (i = 0; i < RX_RING_SIZE; i++)
	{
		phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));
		phypt_rx_ring->status = cpu_to_le32(RDESC0_OWN);
		if (cap->rx_skbuff[i] == NULL)
		{
			skb = cap->rx_skbuff[i] = dev_alloc_skb(cap->rx_buf_sz);
			skb->dev = dev;			/* Mark as being used by this device. */
			phypt_rx_ring->buffer1 = virt_to_le32desc(skb->tail);
		}
	}

	for (i = 0; i < TX_RING_SIZE; i++)
	{
		phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[i]));

		if (cap->tx_skbuff[i]) {
			dev_kfree_skb_irq(cap->tx_skbuff[i]);
		}
		cap->tx_skbuff[i] = NULL;
		phypt_tx_ring->status = 0x00000000;
	}
	outl(virt_to_phys(cap->rx_ring_a), ioaddr + CSR_OFFSET(CSR3));
	outl(virt_to_phys(cap->tx_ring_a), ioaddr + CSR_OFFSET(CSR4));
	/* Clear Interrupt. */
	outl(0xffffffff, ioaddr + CSR_OFFSET(CSR5));
	/* Enable interrupts by setting the interrupt mask. */
	outl(SOCLE_MAC_INTR, ioaddr + CSR_OFFSET(CSR7));

	outl(cr11, ioaddr + CSR_OFFSET(CSR11));
	outl(cr0, ioaddr + CSR_OFFSET(CSR0));
	outl(cr6, ioaddr + CSR_OFFSET(CSR6));
	outl(1, ioaddr + CSR_OFFSET(CSR2));		/* Rx poll demand */

	spin_unlock(&cap->lock);

} /* socle_mac_reset_ring() */


/* check_media_link() */
static void check_media_link(struct net_device *dev, int init)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int mii_sr, newlink = 0;

	mii_sr = sMacMiiPhyRead(dev, cap->phys, MII_PHY_SR);
	if (cap->msg_level & NETIF_MSG_LINK)
		printk(MAC_KERN_DEBUG "%s: Check MII PHY %d, status %4.4x.\n",
			   dev->name, cap->phys, mii_sr);

	if(!(cap->medialink))
	{
		if(mii_sr & MII_PHY_SR_LNK)
		{
			printk(MAC_KERN_INFO "%s: MII PHY %d : Get New Media Link.\n",
			   dev->name, cap->phys);
			cap->medialink = TRUE;
			newlink = 1;
		}
	}
	else
	{
		if(!(mii_sr & MII_PHY_SR_LNK))
		{
			printk(MAC_KERN_INFO "%s: MII PHY %d : Media Link Lost.\n",
			   dev->name, cap->phys);
			cap->medialink = FALSE;
		}
	}

	//Check speed & deuplex for csr6
	/* 0525_Gary */
	//if(cap->medialink || init)
	if(newlink || init)
		check_speed_duplex(dev, init, mii_sr);

} /* check_media_link() */

/* wait_txrxstop() */
static void wait_txrxstop(struct net_device *dev)
{
//	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;
	int csr6 = inl(ioaddr + CSR_OFFSET(CSR6));
	int csr5;

	if(csr6 & (CSR6_ST|CSR6_SR))
	{
		csr6 &= (~(CSR6_ST|CSR6_SR));

		outl(csr6, ioaddr + CSR_OFFSET(CSR6));
		printk (MAC_KERN_DEBUG "\nwait_txrxstop, csr6 = %x", inl(ioaddr + CSR_OFFSET(CSR6)));

		do{
			csr5= inl(ioaddr + CSR_OFFSET(CSR5));
		}
		while((csr5 & (CSR5_TS_MSK|CSR5_RS_MSK)) != (CSR5_TS_ST|CSR5_RS_ST));
	}
}

/* start_rxtx() */
static void start_rxtx(struct net_device *dev)
{
	long ioaddr = dev->base_addr;

	outl( (CSR6_ST | CSR6_SR | inl(ioaddr+CSR_OFFSET(CSR6))), ioaddr + CSR_OFFSET(CSR6));
	printk (MAC_KERN_DEBUG "\nstart_rxtx, csr6 = %x", inl(ioaddr + CSR_OFFSET(CSR6)));
}

/* restart_rxtx() */
static void restart_rxtx(struct net_device *dev)
{
	wait_txrxstop(dev);
	start_rxtx(dev);
}

/* update_csr6_speed_duplex() */
static void update_csr6_speed_duplex(struct net_device *dev, int init)
{
	long ioaddr = dev->base_addr;
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int csr6;

	csr6 = cap->csr6;
	if(cap->full_duplex){
		cap->csr6 |= CSR6_FD;
	}else{
		cap->csr6 &= ~CSR6_FD;  /* 0524_Gary */
	}
	if(cap->speed_100M)
	{
//	    cap->csr6 &= (~CSR6_TTM);  /* 0524_Gary CSR6_TTM never used */

//	    if( dev->base_addr == 0xf0e00000 )  /* 0524_Gary Marked for RMII Compliant */
        cap->csr6 |= CSR6_SPEED;
	}
	else
	{
//	    if( dev->base_addr == 0xf0e00000 )  /* 0524_Gary Marked for RMII Compliant */
		cap->csr6 &= (~CSR6_SPEED);
	}

	if( (csr6 != cap->csr6) || (init) )
	{
		printk(MAC_KERN_INFO "%s: Set Media Mode <%s><%s>\n"
			   , dev->name
			   , cap->speed_100M ? "100M" : "10M"
			   , cap->full_duplex ? "Full" : "Half");

		csr6 = inl(ioaddr + CSR_OFFSET(CSR6));

		wait_txrxstop(dev);

		csr6 = (csr6 & (CSR6_ST | CSR6_SR )) | cap->csr6 ;
		outl(csr6, ioaddr + CSR_OFFSET(CSR6));
		printk (MAC_KERN_DEBUG "Update_CSR6 as <%8.8x>\n", (unsigned int)inl(ioaddr + CSR_OFFSET(CSR6)));
	}
}	/* update_csr6_speed_duplex() */


/* check_speed_duplex() */
/*
  Check the MII negotiated duplex, and change the CSR6 setting if
  required.
  Return 0 if everything is OK.
  Return < 0 if the transceiver auot neg is fail.
  */
static int check_speed_duplex(struct net_device *dev, int init, int mii_sr)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;

	if(cap->medialock)
	{
		printk(MAC_KERN_DEBUG "Media Lock Mode\n");
		update_csr6_speed_duplex(dev, init);
		return 0;
	}

//	mii_sr = sMacMiiPhyRead(dev, cap->phys, MII_PHY_SR); /* 0524_Gary Marked for saving time */

	if(mii_sr & MII_PHY_SR_AN)
	{
		int mii_anlpa;
		int negotiated;

		cap->medianeq = TRUE;

		mii_anlpa = sMacMiiPhyRead(dev, cap->phys, MII_PHY_ANLPA);
		negotiated = mii_anlpa & cap->mii_advertise;

		printk(MAC_KERN_DEBUG "%s: MII link partner %4.4x, negotiated %4.4x.\n",
			   dev->name, mii_anlpa, negotiated);

		if (!(cap->speed_100M_lock))
		{
			if(negotiated & MII_PHY_ANA_100M)
				cap->speed_100M = TRUE;
			else
				cap->speed_100M = FALSE;
		}

		if (!(cap->full_duplex_lock))
		{
			/* 0527_Gary Duplex must based on speed mode */
			if(cap->speed_100M){
				if(negotiated & MII_PHY_ANA_100F){
					cap->full_duplex = TRUE;
				}else{
					cap->full_duplex = FALSE;
				}
			}else{
				if(negotiated & MII_PHY_ANA_10F){
					cap->full_duplex = TRUE;
				}else{
					cap->full_duplex = FALSE;
				}
			}
		}

		update_csr6_speed_duplex(dev, init);

		return 0;
	}

	return -1;
} /* check_speed_duplex() */



/* The little-endian AUTODIN32 ethernet CRC calculation.
   N.B. Do not use for bulk data, use a table-based routine instead.
   This is common code and should be moved to net/core/crc.c */
static unsigned const ethernet_polynomial_le = 0xedb88320U;
static inline u32 ether_crc_le(int length, unsigned char *data)
{
	u32 crc = 0xffffffff;	/* Initial value. */
	while(--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 8; --bit >= 0; current_octet >>= 1) {
			if ((crc ^ current_octet) & 1) {
				crc >>= 1;
				crc ^= ethernet_polynomial_le;
			} else
				crc >>= 1;
		}
	}
	return crc;
}
static unsigned const ethernet_polynomial = 0x04c11db7U;
static inline u32 ether_crc(int length, unsigned char *data)
{
	int crc = -1;

	while(--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 0; bit < 8; bit++, current_octet >>= 1)
			crc = (crc << 1) ^
				((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
	}
	return crc;
}

/* set_rx_mode() */
static void set_rx_mode(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;
	int csr6;

/* 0524_Gary
	if (cap->rmii)
	{
        csr6 = inl(ioaddr + CSR_OFFSET(CSR6))
                & (~(CSR6_RA|CSR6_PM|CSR6_IF|CSR6_PB|CSR6_HO|CSR6_HP));

        cap->csr6 = cap->csr6
                &(~(CSR6_RA|CSR6_PM|CSR6_IF|CSR6_PB|CSR6_HO|CSR6_HP));

	}
	else
	{
*/
		csr6 = inl(ioaddr + CSR_OFFSET(CSR6))
				& (~(CSR6_RA|CSR6_PM|CSR6_PR|CSR6_IF|CSR6_PB|CSR6_HO|CSR6_HP));

		cap->csr6 = cap->csr6
				& (~(CSR6_RA|CSR6_PM|CSR6_PR|CSR6_IF|CSR6_PB|CSR6_HO|CSR6_HP));
//	}

	if (dev->flags & IFF_PROMISC)
	{
		printk(MAC_KERN_DEBUG "%s: Rx Promiscuous mode enabled.\n", dev->name);
		/* Set promiscuous. */
		cap->csr6 |= CSR6_PM | CSR6_PR;
		csr6 |= CSR6_PM | CSR6_PR;
		/* Unconditionally log net taps. */
	}
	else if ( (dev->mc_count > cap->multicast_filter_limit)
			||(dev->flags & IFF_ALLMULTI) )
	{
		printk(MAC_KERN_DEBUG "%s:Multicast<%d> enable Rx All Multicast mode.\n"
			, dev->name, dev->mc_count);
		/* Too many to filter well -- accept all multicasts. */
		cap->csr6 |= CSR6_PM;
		csr6 |= CSR6_PM;
	}
	else
	{
		u16 *eaddrs;
		u16 *setup_frm = (u16 *)(cap->setup_frame);

		struct dev_mc_list *mclist;
		u32 tx_flags = 0x08000000 | 192;
		int i;


		/* Fill the all unused entries with the broadcast address. */
		//memset(setup_frm, 0xff, sizeof(cap->setup_frame)); +peter
		memset(setup_frm, 0xff, sizeof(cap->setup_frame_a));

		/* Note that only the low-address shortword of setup_frame is valid!
		   The values are doubled for big-endian architectures. */
		if (dev->mc_count > 14)
		{
			u16 hash_table[32];
			printk(MAC_KERN_DEBUG "%s:Multicast<%d> enable Rx Hash Table.\n"
				, dev->name, dev->mc_count);
			/* Must use a multicast hash table. */

			cap->csr6 |= CSR6_HP;
			csr6 |= CSR6_HP;

			tx_flags = 0x08400000 | 192;		/* Use hash filter. */
			memset(hash_table, 0, sizeof(hash_table));
			set_bit(255, (void *)hash_table);			/* Broadcast entry */
			/* This should work on big-endian machines as well. */
			for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
				 i++, mclist = mclist->next)
			{
				set_bit(ether_crc_le(ETH_ALEN, mclist->dmi_addr) & 0x1ff, (void *)hash_table);
			}
			for (i = 0; i < 32; i++)
			{
				*setup_frm++ = hash_table[i];
				*setup_frm++ = hash_table[i];
			}
			setup_frm = &cap->setup_frame[13*6];
		}
		else
		{
			/* We have <= 14 addresses so we can use the wonderful
			   16 address perfect filtering. */
			printk(MAC_KERN_DEBUG "%s:Multicast<%d> use Rx Perfect Filter mode.\n"
				, dev->name, dev->mc_count);
			for (i = 0, mclist = dev->mc_list; i < dev->mc_count;
				 i++, mclist = mclist->next)
			{
				printk(MAC_KERN_DEBUG "%s:MC_List<%d>:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n"
					, dev->name, i
					,mclist->dmi_addr[0]
					,mclist->dmi_addr[1]
					,mclist->dmi_addr[2]
					,mclist->dmi_addr[3]
					,mclist->dmi_addr[4]
					,mclist->dmi_addr[5]);
			}

			for (i = 0, mclist = dev->mc_list; i < dev->mc_count;
				 i++, mclist = mclist->next)
			{
				eaddrs = (u16 *)mclist->dmi_addr;
				*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
				*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
				*setup_frm++ = *eaddrs; *setup_frm++ = *eaddrs++;
			}
			setup_frm = &cap->setup_frame[15*6];
		}
		/* Fill the final entry with our physical address. */
		eaddrs = (u16 *)dev->dev_addr;
		*setup_frm++ = eaddrs[0]; *setup_frm++ = eaddrs[0];
		*setup_frm++ = eaddrs[1]; *setup_frm++ = eaddrs[1];
		*setup_frm++ = eaddrs[2]; *setup_frm++ = eaddrs[2];

		/* Now add this frame to the Tx list. */
		if (cap->cur_tx - cap->dirty_tx > TX_RING_SIZE - 2)
		{
			/* Same setup recently queued, we need not add it. */
		}
		else
		{
			unsigned long flags;
			unsigned int entry;
			struct	socle_mac_tx_desc	*phypt_tx_ring;

			spin_lock_irqsave(&cap->lock, flags);
			entry = cap->cur_tx++ % TX_RING_SIZE;
			phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[entry]));

			if (entry != 0)
			{
				/* Avoid a chip errata by prefixing a dummy entry. */
				cap->tx_skbuff[entry] = NULL;
				phypt_tx_ring->length =
					(entry == TX_RING_SIZE-1) ? cpu_to_le32(TDESC1_TER):0;
				phypt_tx_ring->buffer1 = 0;
				phypt_tx_ring->status = cpu_to_le32(TDESC0_OWN);
				entry = cap->cur_tx++ % TX_RING_SIZE;
				phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[entry]));
			}

			cap->tx_skbuff[entry] = NULL;
			/* Put the setup frame on the Tx list. */
			if (entry == TX_RING_SIZE-1)
				tx_flags |= TDESC1_TER;		/* Wrap ring. */
			phypt_tx_ring->length = cpu_to_le32(tx_flags);
			phypt_tx_ring->buffer1 = virt_to_le32desc(cap->setup_frame_a);
			cpu_dcache_clean_area ((void *) cap->setup_frame_a, 2048);
/*
			cpu_dcache_clean_range ((unsigned long) cap->setup_frame_a,
			       (unsigned long) (cap->setup_frame_a + 2048));
*/
			phypt_tx_ring->status = cpu_to_le32(TDESC0_OWN);
			if (cap->cur_tx - cap->dirty_tx >= TX_RING_SIZE - 2)
			{
				netif_stop_queue(dev);
				cap->tx_full = 1;
			}
			spin_unlock_irqrestore(&cap->lock, flags);

			/* Trigger an immediate transmit demand. */
			/* Tx poll demand */
			outl(1, ioaddr + CSR_OFFSET(CSR1));
		}
	}

	outl(csr6, ioaddr + CSR_OFFSET(CSR6));
} /* set_rx_mode() */

/* socle_mac_timer() */
/* 2 Function control by this mac timer handle
	1. Check Mii status for Link & Auot neq
	2. Tx timeout check
*/
static void socle_mac_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;

	/* Check Mii status for Link & Auot neq */
	if (cap->msg_level & NETIF_MSG_TIMER)
		printk(MAC_KERN_DEBUG "%s: Media selection tick\n",
			   dev->name);

	check_media_link(dev, 0);

	if(cap->medialink && cap->medianeq)
	{
		//Link OK use Slow Timer
		cap->timer.expires = jiffies + LINK_CHECK_SLOW;
	}
	else
	{
		//Link Down use Fast Timer
		cap->timer.expires = jiffies + LINK_CHECK_FAST;
	}
	add_timer(&cap->timer);
} /* socle_mac_timer() */


/* socle_mac_start_xmit() */
static int
socle_mac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int entry, q_used_cnt;
	unsigned long eflags_irqsave;
	u32 flag;
	struct	socle_mac_tx_desc	*phypt_tx_ring;

	if (cap->msg_level & NETIF_MSG_TX_QUEUED)
		printk(MAC_KERN_DEBUG "%s: socle_mac_start_xmit ... go\n",
			   dev->name);

	/* Caution: the write order is important here, set the field
	   with the ownership bits last. */

	spin_lock_irqsave(&cap->lock, eflags_irqsave);

	/* Calculate the next Tx descriptor entry. */
	entry = cap->cur_tx % TX_RING_SIZE;
	q_used_cnt = cap->cur_tx - cap->dirty_tx;
	phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[entry]));

	cap->tx_skbuff[entry] = skb;
	phypt_tx_ring->buffer1 = virt_to_le32desc(skb->data);
	cpu_dcache_clean_area((void *)skb->data, 2048);
/*
	cpu_dcache_clean_range((unsigned long)skb->data,(unsigned long)(skb->data+2048));
*/
	if (q_used_cnt < TX_QUEUE_LEN/2)
	{/* Typical path */
		flag = TDESC1_LS|TDESC1_FS; /* No interrupt */
	}
	else if (q_used_cnt == TX_QUEUE_LEN/2)
	{
		flag = TDESC1_IC|TDESC1_LS|TDESC1_FS; /* Tx-done intr. */
	}
	else if (q_used_cnt < TX_QUEUE_LEN)
	{
		flag = TDESC1_LS|TDESC1_FS; /* No Tx-done intr. */
	}
	else
	{		/* Leave room for set_rx_mode() to fill entries. */
		netif_stop_queue(dev); /* 0516_Gary */
		cap->tx_full = 1;
		flag = TDESC1_IC|TDESC1_LS|TDESC1_FS; /* Tx-done intr. */
	}

	if (entry == TX_RING_SIZE-1)
		flag = TDESC1_IC|TDESC1_LS|TDESC1_FS|TDESC1_TER;

	phypt_tx_ring->length = cpu_to_le32(skb->len | flag);
	phypt_tx_ring->status = cpu_to_le32(TDESC0_OWN);
	cap->cur_tx++;

	dev->trans_start = jiffies;

	/* Show MAC DES content */
	socle_mac_desshow(dev, 1, (int *)(phypt_tx_ring));

	/* Trigger an immediate transmit demand. */
	outl(1, dev->base_addr + CSR_OFFSET(CSR1));

	spin_unlock_irqrestore(&cap->lock, eflags_irqsave);

	return 0;
} /* socle_mac_start_xmit() */


/* empty_rings() */
static void empty_rings(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int i;
	struct	socle_mac_rx_desc	*phypt_rx_ring;

	/* Free all the skbuffs in the Rx queue. */
	for (i = 0; i < RX_RING_SIZE; i++)
	{
		phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));
		phypt_rx_ring->status = 0;		/* Not owned. */
		phypt_rx_ring->length = 0;
		phypt_rx_ring->buffer1 = cpu_to_le32(0xBADF00D0); /* An invalid address. */

		if (cap->rx_skbuff[i]) {
			dev_kfree_skb(cap->rx_skbuff[i]);
		}
		cap->rx_skbuff[i] = NULL;
	}
	for (i = 0; i < TX_RING_SIZE; i++)
	{
		if (cap->tx_skbuff[i]) {
			dev_kfree_skb(cap->tx_skbuff[i]);
		}
		cap->tx_skbuff[i] = NULL;
	}
} /* empty_rings() */

/* 0517_Gary Modify some errors*/
static void socle_mac_clean_tx_ring(struct net_device *dev)
{
	struct socle_mac_private	*cap = (struct socle_mac_private *)dev->priv;
	struct socle_mac_tx_desc	*phypt_tx_ring;
	unsigned int status;

	for ( ; cap->cur_tx - cap->dirty_tx > 0; cap->dirty_tx++)
	{
		int entry = cap->dirty_tx % TX_RING_SIZE;
		phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[entry]));
		status = le32_to_cpu(phypt_tx_ring->status);

		if (status & TDESC0_OWN)
		{
			cap->stats.tx_errors++;	/* It wasn't Txed */
			phypt_tx_ring->status = 0;
		}

		/* Check for Tx filter setup frames. */
		if (cap->tx_skbuff[entry])
		{
			/* Free the original skb. */
			dev_kfree_skb_any(cap->tx_skbuff[entry]);
			cap->tx_skbuff[entry] = NULL;
		}
	}
}

static void socle_mac_down (struct net_device *dev)
{
	long ioaddr = dev->base_addr;
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	unsigned long flags;

	del_timer_sync (&cap->timer);

	spin_lock_irqsave (&cap->lock, flags);

	/* Disable interrupts by clearing the interrupt mask. */
	outl (0x00000000, ioaddr + CSR_OFFSET(CSR7));

	/* Stop the Tx and Rx processes. */
	wait_txrxstop(dev);

	/* prepare receive buffers */
	socle_mac_refill_rx(dev);

	/* release any unconsumed transmit buffers */
	socle_mac_clean_tx_ring(dev);

	if (inl (ioaddr + CSR_OFFSET(CSR6)) != 0xffffffff)
		cap->stats.rx_missed_errors += inl (ioaddr + CSR_OFFSET(CSR8)) & 0xffff;

	spin_unlock_irqrestore (&cap->lock, flags);

	init_timer(&cap->timer);
	cap->timer.data = (unsigned long)dev;
	cap->timer.function = socle_mac_timer;
}

/* socle_mac_close() */
static int socle_mac_close(struct net_device *dev)
{
	long ioaddr = dev->base_addr;
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;

	if (cap->msg_level & NETIF_MSG_IFUP)
		printk(MAC_KERN_DEBUG "%s: START CLOSE --- socle_mac_close().\n",
								dev->name);

	netif_stop_queue(dev);

	socle_mac_down (dev);

	free_irq(dev->irq, dev);

	empty_rings(dev);

	if (cap->msg_level & NETIF_MSG_IFDOWN)
		printk(MAC_KERN_DEBUG "%s: SHUTTING DOWN ethercard, status was %8.8x.\n",
			   dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR5)));

	return 0;
} /* socle_mac_close() */

static struct net_device_stats *socle_mac_get_stats(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;

	if (netif_running(dev))
	{
		unsigned long flags;
		int csr8;

		spin_lock_irqsave (&cap->lock, flags);

		csr8 = inl(ioaddr + CSR_OFFSET(CSR8));
		cap->stats.rx_missed_errors += (csr8 & CSR8_MFC_MSK);

		spin_unlock_irqrestore(&cap->lock, flags);
	}
	return &cap->stats;
}

#ifdef HAVE_PRIVATE_IOCTL
static int netdev_ethtool_ioctl(struct net_device *dev, void *useraddr)
{
	u32 ethcmd;

	if (copy_from_user(&ethcmd, useraddr, sizeof(ethcmd)))
		return -EFAULT;

    switch (ethcmd)
    {
        case ETHTOOL_GDRVINFO:
        {
			struct ethtool_drvinfo info = {ETHTOOL_GDRVINFO};
			strcpy(info.driver, DRV_NAME);
			strcpy(info.version, DRV_VERSION);
			strcpy(info.bus_info, DRV_BUSMODE);
			if (copy_to_user(useraddr, &info, sizeof(info)))
				return -EFAULT;
			return 0;
		}
    }

	return -EOPNOTSUPP;
}

/* private_ioctl() */
/* Provide ioctl() calls to examine the MII xcvr state.
   The two numeric constants are because some clueless person
   changed value for the symbolic name.
 */
static int private_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	u16 *data = (u16 *)&rq->ifr_data;
	u32 *data32 = (void *)&rq->ifr_data;
	unsigned int phy = cap->phys;
	unsigned int regnum = data[1];

	switch(cmd) {
	case SIOCETHTOOL:			/* Ethtool interface		*/
		return netdev_ethtool_ioctl(dev, (void *) rq->ifr_data);

	case SIOCGMIIPHY:			/* Get address of MII PHY in use. */
	case SIOCDEVPRIVATE:		/* for binary compat */
		/* SIOCGMIIPHY: Get the address of the PHY in use. */
		data[0] = phy;
	case SIOCGMIIREG:			/* Read MII PHY register. */
	case SIOCDEVPRIVATE+1:		/* for binary compat */
		/* SIOCGMIIREG: Read the specified MII register. */
		if(phy > 31)
			return -ENODEV;
		if (regnum & ~0x1f)
			return -EINVAL;
		return 0;

	case SIOCSMIIREG:			/* Write MII PHY register. */
	case SIOCDEVPRIVATE+2:		/* for binary compat */
		/* SIOCSMIIREG: Write the specified MII register */
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (regnum & ~0x1f)
			return -EINVAL;
		if(phy > 31)
			return -ENODEV;

		if (data[0] == phy)
		{
			u16 value = data[2];
			switch (regnum)
			{
				case MII_PHY_CR: /* Check for autonegotiation on or reset. */
					if( !(value & (MII_PHY_CR_RESET|MII_PHY_CR_AUTO)) )
					//No Auto Neg & No Reset is force the speed and duplex
					{
						cap->full_duplex_lock = 1;
						cap->speed_100M_lock = 1;
					}
					if (cap->full_duplex_lock)
						cap->full_duplex = (value & MII_PHY_CR_FDX) ? 1 : 0;
					if (cap->speed_100M_lock)
						cap->speed_100M = (value & MII_PHY_CR_100M) ? 1 : 0;
				break;
				case MII_PHY_ANA:
					cap->mii_advertise = data[2];
				break;
			}
		}
		sMacMiiPhyWrite(dev, data[0], regnum, data[2]);
		return 0;

	case SIOCGIFMAP:
		data32[0] = cap->msg_level;
		data32[1] = cap->multicast_filter_limit;
		data32[2] = cap->max_interrupt_work;
		data32[3] = cap->rx_copythreshold;
		return 0;
	case SIOCSIFMAP:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		cap->msg_level = data32[0];
		cap->multicast_filter_limit = data32[1];
		cap->max_interrupt_work = data32[2];
		cap->rx_copythreshold = data32[3];
		return 0;
	default:
		return -EOPNOTSUPP;
	}

	return -EOPNOTSUPP;
} /* private_ioctl() */
#endif  /* HAVE_PRIVATE_IOCTL */

/* socle_mac_tx_timeout() */
static void socle_mac_tx_timeout(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;
	unsigned long eflags_irqsave;

	spin_lock_irqsave(&cap->lock, eflags_irqsave);

#ifndef	FINAL_RELEASE_VERSION
	if (cap->msg_level & NETIF_MSG_TX_ERR)
	{
		int i;
		for (i = 0; i < RX_RING_SIZE; i++)
		{
			u8 *buf;
			int j;
			struct	socle_mac_rx_desc	*phypt_rx_ring;

			phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));
			buf = (u8 *)(phypt_rx_ring->buffer1);

			printk(MAC_KERN_DEBUG "%2d: %8.8x %8.8x %8.8x %8.8x  "
				   "%2.2x %2.2x %2.2x.\n",
				   i, (unsigned int)le32_to_cpu(phypt_rx_ring->status),
				   (unsigned int)le32_to_cpu(phypt_rx_ring->length),
				   (unsigned int)le32_to_cpu(phypt_rx_ring->buffer1),
				   (unsigned int)le32_to_cpu(phypt_rx_ring->buffer2),
				   buf[0], buf[1], buf[2]);
			for (j = 0; buf[j] != 0xee && j < 1600; j++)
				if (j < 100) printk(KERN_DEBUG " %2.2x", buf[j]);

			printk(KERN_DEBUG " j=%d.\n", j);
		}

		printk(MAC_KERN_DEBUG "  Rx ring %8.8x: ", (int)cap->rx_ring);
		for (i = 0; i < RX_RING_SIZE; i++)
		{
			struct	socle_mac_rx_desc	*phypt_rx_ring;
			phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[i]));

			printk(KERN_DEBUG " %8.8x", (unsigned int)le32_to_cpu(phypt_rx_ring->status));
		}

		printk("\n" MAC_KERN_DEBUG "  Tx ring %8.8x: ", (int)cap->tx_ring);
		for (i = 0; i < TX_RING_SIZE; i++)
		{
			struct	socle_mac_tx_desc	*phypt_tx_ring;
			phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[i]));

			printk(KERN_DEBUG " %8.8x", (unsigned int)le32_to_cpu(phypt_tx_ring->status));
		}
		printk("\n");
	}
#endif
	check_media_link(dev, 0);

	cap->stats.tx_errors++;

	restart_rxtx(dev);
	/* Trigger an immediate transmit demand. */
	outl(1, ioaddr + CSR_OFFSET(CSR1));
	dev->trans_start = jiffies;

	netif_wake_queue(dev);

	spin_unlock_irqrestore(&cap->lock, eflags_irqsave);

	return;
} /* socle_mac_tx_timeout() */



/* socle_mac_interrupt() */
/* The interrupt handler does all of the Rx thread work and cleans up
   after the Tx thread. */
static irqreturn_t socle_mac_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
/*
static void socle_mac_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
*/
{
	struct net_device *dev = (struct net_device *)dev_instance;
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;
	int csr5 = inl(ioaddr + CSR_OFFSET(CSR5));
	int work_budget = cap->max_interrupt_work;
	DEV=dev;
	if ((csr5 & CSR5_ALLINT_MASK) == 0) {
		return 0;
        }
	do
	{
		//csr5 = inl(ioaddr + CSR_OFFSET(CSR5));
		if ((csr5 & (CSR5_NIS|CSR5_AIS)) == 0)
		{
			/* Acknowledge all of the current interrupt sources ASAP. */
			outl(csr5 , ioaddr + CSR_OFFSET(CSR5));
			printk("%s: WARNING !!!!!!!!!!!!!!!!!!! NO NIS AIS interrupt  csr5=%#8.8x new csr5=%#8.8x!\n",
				   dev->name, csr5, (int)inl(dev->base_addr + CSR_OFFSET(CSR5)));
			break;
		}

		/* Acknowledge all of the current interrupt sources ASAP. */
		outl(csr5 , ioaddr + CSR_OFFSET(CSR5));

		if (cap->msg_level & NETIF_MSG_INTR)
			printk("%s: interrupt  csr5=%#8.8x new csr5=%#8.8x.\n",
				   dev->name, csr5, (int)inl(dev->base_addr + CSR_OFFSET(CSR5)));

		if (csr5 & (CSR5_RI | CSR5_RU))
		{
			work_budget -= socle_mac_rx(dev);
			socle_mac_refill_rx(dev);
		}

		if (csr5 & (CSR5_TU | CSR5_TPS | CSR5_TI))
		{
			unsigned int dirty_tx;

			spin_lock(&cap->lock);

			for (dirty_tx = cap->dirty_tx; cap->cur_tx - dirty_tx > 0; dirty_tx++)
			{
				int entry = dirty_tx % TX_RING_SIZE;
				int status;
				struct	socle_mac_tx_desc	*phypt_tx_ring;

				phypt_tx_ring = (struct	socle_mac_tx_desc *)(&(cap->tx_ring[entry]));
				status = le32_to_cpu(phypt_tx_ring->status);

				if (status & TDESC0_OWN)
					break;			/* It still has not been Txed */

				if (cap->full_duplex)
				{
					/* Full Duplex don't check the error cause by collision */
					status = status & (~(TDESC0_LO | TDESC0_NC | TDESC0_ES | TDESC0_LC | TDESC0_EC | TDESC0_DE | TDESC0_CC_MSK));
					if(status)
						status = status | TDESC0_ES;
				}

				/* Skip the Rx filter setup frames. */
				if (cap->tx_skbuff[entry] == NULL)
				  continue;

				if (status & TDESC0_ES)
				{
					/* There was an major error, log it. */
					if (cap->msg_level & NETIF_MSG_TX_ERR)
						printk("%s: Transmit error, Tx status<%8.8x>.\n",
							   dev->name, status);
					cap->stats.tx_errors++;
					if (status & (TDESC0_EC)) 			cap->stats.tx_aborted_errors++;
					if (status & (TDESC0_LO|TDESC0_NC)) cap->stats.tx_carrier_errors++;
					if (status & (TDESC0_LC)) 			cap->stats.tx_window_errors++;
					if (status & (TDESC0_UF)) 			cap->stats.tx_fifo_errors++;
#ifdef ETHER_STATS
					if (status & (TDESC0_EC)) 			cap->stats.collisions16++;
#endif
				}
				else
				{
					if (cap->msg_level & NETIF_MSG_TX_DONE)
						printk("%s: Transmit complete, status<%8.8x>.\n", dev->name, status);
#ifdef ETHER_STATS
					if (status & TDESC0_DE) 			cap->stats.tx_deferred++;
#endif
					cap->stats.tx_bytes += cap->tx_skbuff[entry]->len;
					cap->stats.collisions += TDESC0_CC_GET(status);
					cap->stats.tx_packets++;
				}

				/* Free the original skb. */
				dev_kfree_skb_irq(cap->tx_skbuff[entry]);
				cap->tx_skbuff[entry] = NULL;
			}

#ifndef FINAL_RELEASE_VERSION
			if (cap->cur_tx - dirty_tx > TX_RING_SIZE)
			{
				printk("%s: Out-of-sync dirty pointer, %d vs. %d, full=%d.\n",
					   dev->name, dirty_tx, cap->cur_tx, cap->tx_full);
				dirty_tx += TX_RING_SIZE;
			}
#endif

			if (cap->tx_full && cap->cur_tx - dirty_tx  < TX_QUEUE_LEN - 4)
			{
				/* The ring is no longer full, clear tbusy. */
				cap->tx_full = 0;
				netif_wake_queue(dev);
			}

			cap->dirty_tx = dirty_tx;

			spin_unlock(&cap->lock);
		}


		if (cap->rx_dead)
		{
			socle_mac_rx(dev);
			socle_mac_refill_rx(dev);
			if (cap->cur_rx - cap->dirty_rx < RX_RING_SIZE - 3)
			{
			/* +grant
			 * 	printk(MAC_KERN_ERR "%s: Restarted Rx at %d / %d.\n",
					   dev->name, cap->cur_rx, cap->dirty_rx);
			 */
				outl(0, ioaddr + CSR_OFFSET(CSR2));		/* Rx poll demand   */
				start_rxtx(dev); //+grant
				cap->rx_dead = 0;
			}
		}

		/* Log errors. */
		if (csr5 & CSR5_AIS)
		{	/* Abnormal error summary bit. */
			if (csr5 == 0xffffffff)
				break;

			if (csr5 & CSR5_UNF)
			{
				if ((cap->csr6 & CSR6_TR_MSK) != CSR6_TR_MSK)
					cap->csr6 += CSR6_TR_STEP;	/* Bump up the Tx threshold */
				else
					cap->csr6 |= CSR6_SF;		/* Store-n-forward. */

				if (cap->msg_level & NETIF_MSG_TX_ERR)
					printk("%s: Tx threshold increased, "
						   "new CSR6<%8.8x>.\n", dev->name, cap->csr6);
				//+grant
				restart_rxtx(dev);
				outl(0, ioaddr + CSR_OFFSET(CSR1));
			}

			if (csr5 & CSR5_TPS)
			{
				/* This is normal when changing Tx modes. */
				if (cap->msg_level & NETIF_MSG_LINK)
					printk("%s: The transmitter stopped."
						   "  CSR5 is<%8.8x>, CSR6<%8.8x>, new CSR6<%8.8x>.\n",
						   dev->name, csr5, (int)inl(ioaddr + CSR_OFFSET(CSR6)), cap->csr6);
			}

		//	if (csr5 & CSR5_UNF)
		//	{
				/* Restart the transmit process. */
		//		restart_rxtx(dev);
		//		outl(0, ioaddr + CSR_OFFSET(CSR1));
		//	}

			if (csr5 & (CSR5_RPS | CSR5_RU))
			{
				/* Missed a Rx frame or mode change. */
				cap->stats.rx_missed_errors += inl(ioaddr + CSR_OFFSET(CSR8)) & CSR8_MFC_MSK;
				socle_mac_rx(dev);
				socle_mac_refill_rx(dev);
				outl(0, ioaddr + CSR_OFFSET(CSR2)); //+grant

				if (csr5 & CSR5_RU)
				{
					cap->rx_dead = 1;

				}
			}

			if (csr5 & CSR5_GPTE)
			{
				if (cap->msg_level & NETIF_MSG_INTR)
					printk("%s: MAC Embedded Timer Interrupt.\n",
						   dev->name);
			}
		}

		cap->csr11 = (0x8b240000|( cap->csr11 & (~(CSR11_TT_MSK|CSR11_NTP_MSK|CSR11_RT_MSK|CSR11_NRP_MSK))));
		outl(cap->csr11, ioaddr + CSR_OFFSET(CSR11));
		csr5 = inl(ioaddr + CSR5);
	} while ((csr5 & CSR5_ALLINT_MASK) != 0);
	socle_mac_refill_rx(dev); //+grant

    /* 0516_Gary Prevent counter ring back */
	if(cap->cur_tx > TX_RING_SIZE && cap->dirty_tx > TX_RING_SIZE){
	    cap->cur_tx -= TX_RING_SIZE;
	    cap->dirty_tx -= TX_RING_SIZE;
	}
	if(cap->cur_rx > RX_RING_SIZE && cap->dirty_rx > RX_RING_SIZE){
	    cap->cur_rx -= RX_RING_SIZE;
	    cap->dirty_rx -= RX_RING_SIZE;
	}
	/**/

	if (cap->msg_level & NETIF_MSG_INTR) {
		printk("%s: exiting interrupt, csr5=%#4.4x.\n",
			   dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR5)) );
        }


	return IRQ_HANDLED;
} /* socle_mac_amba_interrupt() */


/* Refill the Rx ring buffers. */
static int socle_mac_refill_rx(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	int entry;
	int work_done = 0;
	struct	socle_mac_rx_desc	*phypt_rx_ring;

	for (; cap->cur_rx - cap->dirty_rx > 0; cap->dirty_rx++)
	{
		entry = cap->dirty_rx % RX_RING_SIZE;
		phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[entry]));

		if (cap->rx_skbuff[entry] == NULL)
		{
			struct sk_buff *skb;
			skb = cap->rx_skbuff[entry] = dev_alloc_skb(cap->rx_buf_sz);
			if (skb == NULL)
			{
				if (cap->cur_rx - cap->dirty_rx == RX_RING_SIZE)
					printk(MAC_KERN_ERR "%s: No kernel memory to allocate " "receive buffers.\n", dev->name);
				break;
			}
			skb->dev = dev;			/* Mark as being used by this device. */
			phypt_rx_ring->buffer1 = virt_to_le32desc(skb->tail);
			work_done++;
		}
		phypt_rx_ring->status = cpu_to_le32(RDESC0_OWN);
	}

	return work_done;
}

/* socle_mac_rx() */
static int socle_mac_rx(struct net_device *dev)
{
    struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
    int entry = cap->cur_rx % RX_RING_SIZE;
    int work_done = 0;
    s32 status;
    struct	socle_mac_rx_desc	*phypt_rx_ring;
    long ioaddr = dev->base_addr;

    /* If we own the next entry, it is a new packet. Send it up. */
    phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[entry]));
    status = le32_to_cpu(phypt_rx_ring->status);

    while ( ! (status & RDESC0_OWN) )
    {
 	if (cap->msg_level & NETIF_MSG_RX_STATUS)
    	printk(MAC_KERN_DEBUG "%s: In socle_mac_rx(), entry %d status<%8.8x>.\n", dev->name, entry, status);

	socle_mac_desshow(dev, 0, (int *)(phypt_rx_ring));

/* 0517_Gary */
	if ((cap->cur_rx - cap->dirty_rx) == RX_RING_SIZE){
	    phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[entry]));
	    phypt_rx_ring->status = 0x80000000;

	    outl(CSR2_RPD, ioaddr + CSR_OFFSET(CSR2));	/* Rx poll demand   */
	    outl((cap->csr6&(~CSR6_SR)), ioaddr + CSR_OFFSET(CSR6));
	    break;
	}
	//HW Bug of ES not include CS TL
	if (cap->full_duplex)
	{
	    if( status & (RDESC0_TL | RDESC0_CS) )
		status |= RDESC0_ES;
	}
	else
	{
	    if( status & (RDESC0_TL) )
		status |= RDESC0_ES;
	}

	if ( (status & (RDESC0_ES|RDESC0_FD|RDESC0_LS)) != (RDESC0_FD|RDESC0_LS) )
	{
	    if ( (status & (RDESC0_FD|RDESC0_LS)) != (RDESC0_FD|RDESC0_LS) )
	    {
	        /* Ingore earlier buffers. */
			if ((status & 0xffff) != 0x7fff)
			{
			    if (cap->msg_level & NETIF_MSG_RX_ERR)
				printk(MAC_KERN_WARNING "%s: Oversized Ethernet frame "
				   "spanned multiple buffers, status<%8.8x>!\n",
				   dev->name, status);
			          cap->stats.rx_length_errors++;
			}
	    }
	    else if(status & RDESC0_ES)
	    {
		    /* Ther was a fatal error. */
		    if (status & (RDESC0_RF|RDESC0_TL)) 	cap->stats.rx_length_errors++;
		    /* 0516_Gary Modify */
		    if ((status & (RDESC0_DB | 0x00030000)) == RDESC0_DB) {
		 	    cap->stats.rx_frame_errors++;

		 	    socle_mac_reset_ring(dev);
		 	    netif_start_queue(dev);
		 	    return 0;
		    }
		    if (status & (RDESC0_CE)) 		cap->stats.rx_crc_errors++;
		    if (status & (RDESC0_FIFOERR))
		    {
		        long ioaddr = dev->base_addr;

		        cap->stats.rx_fifo_errors++;

		        outl(1, ioaddr + CSR_OFFSET(CSR2));		/* Rx poll demand */
		        printk(MAC_KERN_INFO "%s: Warnning:Receive FIFO error!!!!!!!!!\n",dev->name);
		    }
	    }
	}
	else
	{
	    /* Omit the four octet CRC from the length. */
	    short pkt_len = RDESC0_FL_GET(status) - 4;
	    struct sk_buff *skb;
	    long ioaddr = dev->base_addr;

#ifndef FINAL_RELEASE_VERSION
        if (pkt_len > 1518)
	    {
			printk(MAC_KERN_WARNING "%s: Bogus packet size of %d (%#x).\n", dev->name, pkt_len, pkt_len);
			pkt_len = 1518;
			cap->stats.rx_length_errors++;
	    }
#endif
	    /* Check if the packet is long enough to accept without copying
			   to a minimally-sized skbuff. */
	    if (pkt_len < cap->rx_copythreshold
			&& (skb = dev_alloc_skb(pkt_len + 2)) != NULL)
	    {
			skb->dev = dev;
			skb_reserve(skb, 2);	/* 16 byte align the IP header */
			cpu_dcache_clean_area ((void *)cap->rx_skbuff[entry]->tail, 2048);
/*
			cpu_dcache_invalidate_range ((unsigned long)cap->rx_skbuff[entry]->tail,
			(unsigned long)(cap->rx_skbuff[entry]->tail + 2048));
*/
			eth_copy_and_sum(skb, cap->rx_skbuff[entry]->tail, pkt_len, 0);
			skb_put(skb, pkt_len);
			work_done++;
	    }
	    else
	    {	/* Pass up the skb already on the Rx ring. */
			cpu_dcache_clean_area ((void *)cap->rx_skbuff[entry]->tail, 2048);
/*
			cpu_dcache_invalidate_range ((unsigned long)cap->rx_skbuff[entry]->tail, (unsigned long)(cap->rx_skbuff[entry]->tail + 2048));
*/
			skb_put(skb = cap->rx_skbuff[entry], pkt_len);
			cap->rx_skbuff[entry] = NULL;
        }
	    skb->protocol = eth_type_trans(skb, dev);
	    netif_rx(skb);
	    dev->last_rx = jiffies;
	    cap->stats.rx_packets++;
	    cap->stats.rx_bytes += pkt_len;
	    outl(1, ioaddr + CSR_OFFSET(CSR2));		/* Rx poll demand */
	}
	cap->cur_rx++;
	entry = cap->cur_rx % RX_RING_SIZE;
	phypt_rx_ring = (struct	socle_mac_rx_desc *)(&(cap->rx_ring[entry]));
	status = le32_to_cpu(phypt_rx_ring->status);
    } /* end while loop */

	return work_done;
} /* socle_mac_rx() */

#ifdef SOCLE_MAC_DEBUG
static void
socle_mac_hwcsrshow(struct net_device *dev)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	long ioaddr = dev->base_addr;

	if (cap->msg_level & NETIF_MSG_HW)
	{
		printk(MAC_KERN_DEBUG
		"%s: SHOW HW CSR : CSR0<%8.8x>, CSR1<%8.8x> CSR2<%8.8x> CSR3<%8.8x>.\n",
			dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR0))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR1))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR2))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR3)));

		printk(MAC_KERN_DEBUG
		"%s: SHOW HW CSR : CSR4<%8.8x>, CSR5<%8.8x> CSR6<%8.8x> CSR7<%8.8x>.\n",
			dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR4))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR5))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR6))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR7)));
		printk(MAC_KERN_DEBUG
		"%s: SHOW HW CSR : CSR8<%8.8x>, CSR9<%8.8x> CSR11<%8.8x>.\n",
			dev->name, (int)inl(ioaddr + CSR_OFFSET(CSR8))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR9))
					 , (int)inl(ioaddr + CSR_OFFSET(CSR11)));
	}
}

static void
socle_mac_desshow(struct net_device *dev, int txrx, int *despt)
{
	struct socle_mac_private *cap = (struct socle_mac_private *)dev->priv;
	if (cap->msg_level & NETIF_MSG_HW)
	{
		printk(MAC_KERN_DEBUG
		"%s: SHOW %s : DES0<%8.8x>, DES1<%8.8x> DES2<%8.8x> DES3<%8.8x> at <%8.8x>.\n", dev->name, txrx ? "TX Des." : "RXDes."
					 , (int)despt[0]
					 , (int)despt[1]
					 , (int)despt[2]
					 , (int)despt[3]
					 , (int)despt);
	}
}
#endif
