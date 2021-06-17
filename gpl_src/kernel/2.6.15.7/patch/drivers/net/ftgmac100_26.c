//-----------------------------------------------------------------------------
// Original version by Faraday handled Faraday GMAC and Marvell PHY:
//  "Faraday FTGMAC Driver, (Linux Kernel 2.6.14) 06/16/06 - by Faraday\n"
// Merged improvements from ASPEED to handle Broadcom PHY, and 1 or 2 ports.
//  "ASPEED MAC Driver, (Linux Kernel 2.6.15.7) 10/02/07 - by ASPEED\n"
// Further improvements:
//
//   -- Assume MAC1 has a PHY chip.  Read the chip type and handle Marvell
//      or Broadcom, else don't touch PHY chip (if present).
//
//   -- If MAC2 is on, check if U-Boot enabled the MII2DC+MII2DIO pins.
//      If yes, handle Marvell or Broadcom PHY.  If no, assume sideband RMII
//      interface with no PHY chip.
// 1.12/27/07 - by river@aspeed
//   Workaround for the gigabit hash function
// 2.12/27/07 - by river@aspeed
//   Synchronize the EDORR bit with document, D[30], D[15] both are EDORR
// 3.12/31/07 - by river@aspeed
//   Add aspeed_i2c_init and aspeed_i2c_read function for DHCP
// 4.04/10/2008 - by river@aspeed
//   Synchronize the EDOTR bit with document, D[30] is EDOTR
// 5.04/10/2008 - by river@aspeed
//   Remove the workaround for multicast hash function in A2 chip
// SDK 0.19
// 6.05/15/2008 - by river@aspeed
//   Fix bug of free sk_buff in wrong routine
// 7.05/16/2008 - by river@aspeed
//   Fix bug of skb_over_panic()
// 8.05/22/2008 - by river@aspeed
//   Support NCSI Feature
// SDK 0.20
// 9.07/02/2008 - by river@aspeed
//   Fix TX will drop packet bug
// SDK 0.21
//10.08/06/2008 - by river@aspeed
//   Add the netif_carrier_on() and netif_carrier_off()
//11.08/06/2008 - by river@aspeed
//   Fix the timer did not work after device closed
// SDK0.22
//12.08/12/2008 - by river@aspeed
//   Support different PHY configuration
// SDK0.23
//13.10/14/2008 - by river@aspeed
//   Support Realtek RTL8211BN Gigabit PHY
//14.11/17/2008 - by river@aspeed
//   Modify the allocate buffer to alignment to IP header
//15.11/27/2008 - by river@aspeed
//   Disable the TPKT2E(tx complete) interrupt and use the tasklet to handle the skb release
//16.11/27/2008 - by river@aspeed
//   Implement NAPI
//17.11/27/2008 - by river@aspeed
//   Add the change_mtu function (max (2016 - 32), hardware FIFO limitation)
//18.11/28/2008 - by river@aspeed
//   Remove the change_mtu function
//19.01/07/2009 - by river@aspeed
//   Set the LED control of Marvell's PHY chip
//-----------------------------------------------------------------------------

static const char version[] =
	"ASPEED FTGMAC Driver, (Linux Kernel 2.6.15.7) 01/07/2009 - by ASPEED\n";


#ifdef FREE_TX_IN_WQ
#include <<linux/workqueue.h>
#endif
#include <asm/arch/drivers/board_def.h>
//#include <asm/arch/spec.h>
#include "ftgmac100_26.h"
#include <asm/arch/platform.h>
#include <asm/arch/ast-scu.h>
#include "rtl8364/rtl8364_i2c_driver.h"
/*------------------------------------------------------------------------
 .
 . Configuration options, for the experienced user to change.
 .
 -------------------------------------------------------------------------*/
#define DUMMY_PHY (0x18) //(3|4|8)//(4|2|8) //0:disable dummy phy mode. 1:enable. No MDIO access. 2: Allow MDIO read but return dummy. 3:Allow MDIO read and return read result. 4:Allow MDIO write. 8:disable PHY interrupt. 0x10:no PHY reset
#define HW_RANDOM 0 //We don't see the necessary to add random noise in our application. So, set 0.
#define NEW_RX_PROC 1
#define NEW_AHU 1 //ast hw utp
#define PHY_RESET_INTERVAL 10000//10 seconds
NCSI_Capability NCSI_Cap;
#if (CONFIG_AST1500_SOC_VER == 2)
/* Bruce150302.Enable auto Tx poll to avoid AST1510 chip bug. */
#define AUTO_TX_POLL
#endif

/*
 . DEBUGGING LEVELS
 .
 . 0 for normal operation
 . 1 for slightly more details
 . >2 for various levels of increasingly useless information
 .    2 for interrupt tracking, status flags
 .    3 for packet info
 .    4 for complete packet dumps
*/

#define DO_PRINT(args...) printk(CARDNAME ": " args)

#define FTMAC100_DEBUG  1

#if (FTMAC100_DEBUG > 2 )
#define PRINTK3(args...) DO_PRINT(args)
#else
#define PRINTK3(args...)
#endif

#if FTMAC100_DEBUG > 1
#define PRINTK2(args...) DO_PRINT(args)
#else
#define PRINTK2(args...)
#endif

#ifdef FTMAC100_DEBUG
#define PRINTK(args...) DO_PRINT(args)
#else
#define PRINTK(args...)
#endif

#ifdef CONFIG_FTGMAC100_NAPI
#define ftgmac100_rx_skb			netif_receive_skb
#else
#define ftgmac100_rx_skb			netif_rx
#endif

/*------------------------------------------------------------------------
 .
 . The internal workings of the driver.  If you are changing anything
 . here with the SMC stuff, you should have the datasheet and know
 . what you are doing.
 .
 -------------------------------------------------------------------------*/
#define CARDNAME "FTGMAC100"
// static const char    mac_string[] = "Faraday GMAC";
// #define IPMODULE GMAC
// #define IPNAME   FTGMAC1000
//struct tq_struct rcv_tq;

#define PHYSRC_SPEED_MASK	0xC000
#define PHYSRC_SPEED_OFFSET	14

/*-----------------------------------------------------------------
 .
 .  The driver can be entered at any of the following entry points.
 .
 .------------------------------------------------------------------  */

/*
 . This is called by  register_netdev().  It is responsible for
 . checking the portlist for the FTMAC100 series chipset.  If it finds
 . one, then it will initialize the device, find the hardware information,
 . and sets up the appropriate device parameters.
 . NOTE: Interrupts are *OFF* when this procedure is called.
 .
 . NB:This shouldn't be static since it is referred to externally.
*/

/*
 . This is called by  unregister_netdev().  It is responsible for
 . cleaning up before the driver is finally unregistered and discarded.
*/
void ftgmac100_destructor(struct net_device *dev);

/*
 . The kernel calls this function when someone wants to use the net_device,
 . typically 'ifconfig ethX up'.
*/
static int ftgmac100_open(struct net_device *dev);

/*
 . This is called by the kernel to send a packet out into the net.  it's
 . responsible for doing a best-effort send, but if it's simply not possible
 . to send it, the packet gets dropped.
*/
static void ftgmac100_timeout (struct net_device *dev);
/*
 . This is called by the kernel in response to 'ifconfig ethX down'.  It
 . is responsible for cleaning up everything that the open routine
 . does, and maybe putting the card into a powerdown state.
*/
static int ftgmac100_close(struct net_device *dev);

/*
 . This routine allows the proc file system to query the driver's
 . statistics.
*/
static struct net_device_stats * ftgmac100_query_statistics( struct net_device *dev);

/*
 . Finally, a call to set promiscuous mode ( for TCPDUMP and related
 . programs ) and multicast modes.
*/
static void ftgmac100_set_multicast_list(struct net_device *dev);

/*
 . Configures the PHY through the MII Management interface
*/
static void ftgmac100_phy_configure(struct net_device* dev);

/*
 . TX routine
*/
int ftgmac100_wait_to_send_packet(struct sk_buff *skb, struct net_device *dev);
/*---------------------------------------------------------------
 .
 . Interrupt level calls..
 .
 ----------------------------------------------------------------*/

/*
 . Handles the actual interrupt
*/
static irqreturn_t ftgmac100_interrupt(int irq, void *, struct pt_regs *regs);
/*
 . This is a separate procedure to handle the receipt of a packet, to
 . leave the interrupt code looking slightly cleaner
*/
#if NEW_RX_PROC
inline static int ftgmac100_rcv( struct net_device *dev, unsigned int process_limit);
#else
inline static int ftgmac100_rcv( struct net_device *dev );
#endif

#ifdef CONFIG_FTGMAC100_NAPI
static int ftgmac100_poll(struct net_device *dev, int *budget);
#endif
#ifdef NEW_JUMBO_FRAME
/*
   Chnage MTU routine
*/
static int ftgmac100_change_mtu(struct net_device *dev, int new_mtu);

#endif

#if NEW_AHU
int ahu_init(struct net_device *dev);
void ahu_release_tx_skb(struct sk_buff *skb);
int ahu_netif_rx(struct sk_buff *skb);
void ahu_alert_cfg(u32 en);
#endif

/*
 ------------------------------------------------------------
 .
 . Internal routines
 .
 ------------------------------------------------------------
*/

/*
 . Test if a given location contains a chip, trying to cause as
 . little damage as possible if it's not a SMC chip.
*/
static int ftgmac100_probe(struct net_device *dev);

/*
 . A rather simple routine to print out a packet for debugging purposes.
*/
#if FTMAC100_DEBUG > 2
static void print_packet( byte *, int );
#endif


/* this does a soft reset on the device */
static void ftgmac100_reset( struct net_device* dev );

/* Enable Interrupts, Receive, and Transmit */
static void ftgmac100_enable( struct net_device *dev );


/* Routines to Read and Write the PHY Registers across the
   MII Management Interface
*/
static inline void ftgmac100_phy_rw_waiting(unsigned int ioaddr, unsigned int is_atomic);
static word ftgmac100_read_phy_register_ex(unsigned int ioaddr, byte phyaddr, byte phyreg, unsigned int is_atomic);
static void ftgmac100_write_phy_register_ex(unsigned int ioaddr, byte phyaddr, byte phyreg, word phydata, unsigned int is_atomic);
#define ftgmac100_read_phy_register(ioaddr, phyaddr, phyreg) ftgmac100_read_phy_register_ex(ioaddr, phyaddr, phyreg, 1)
#define ftgmac100_read_phy_register_sleep(ioaddr, phyaddr, phyreg) ftgmac100_read_phy_register_ex(ioaddr, phyaddr, phyreg, 0)
#define ftgmac100_write_phy_register(ioaddr, phyaddr, phyreg, phydata) ftgmac100_write_phy_register_ex(ioaddr, phyaddr, phyreg, phydata, 1)
#define ftgmac100_write_phy_register_sleep(ioaddr, phyaddr, phyreg, phydata) ftgmac100_write_phy_register_ex(ioaddr, phyaddr, phyreg, phydata, 0)
#define gb_ftgmac100_read_phy_register(ioaddr, phyaddr, phyreg) gb_ftgmac100_read_phy_register_ex(ioaddr, phyaddr, phyreg, 1)
#define gb_ftgmac100_write_phy_register(ioaddr, phyaddr, phyreg, phydata) gb_ftgmac100_write_phy_register_ex(ioaddr, phyaddr, phyreg, phydata, 1)
u8  aspeedi2c_read (u8, u8, u16);
void  i2c_init(u8);
static void ftgmac100_force_free_rx (struct net_device *dev);
static void ftgmac100_force_free_tx (struct net_device *dev);
static void ftgmac100_force_free_skbs (struct net_device *dev);
static void ftgmac100_free_tx (unsigned long);
static void ftgmac100_link_chg (unsigned long data);

static volatile int trans_busy = 0;
// static char mac1[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x1};
// static char mac2[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x2};
static unsigned long IP_va_base[2] = {IO_ADDRESS(0x1e660000), IO_ADDRESS(0x1e680000)};
static unsigned long IP_va_limit[2] =
    {IO_ADDRESS(0x1e660000) + 0x20000, IO_ADDRESS(0x1e680000) + 0x20000};
static unsigned long IP_irq[2] = {IRQ_MAC1, IRQ_MAC2};

// Values 0..31 are the PHY chip address on the MDC+MDIO bus driven by this
// MAC.  Values 32..63 mean this MAC's PHY is on the _MAC1_ MDC+MDIO bus.
// Value 255 prevents this MAC from accessing PHY chip; assume 100 Mbit/s FDX.
static unsigned char IP_phy_addr[2] = { 0x0, 0x0 };

static struct net_device *ftgmac100_netdev[IP_COUNT];
//static struct resource ftgmac100_resource[IP_COUNT];
static unsigned int DF_support = 0;

static inline void ast_dev_kfree_skb(struct sk_buff *skb)
{
#if NEW_AHU
	if (skb->priority == 0x16881688) {
		ahu_release_tx_skb(skb);
	} else
#endif
	{
		dev_kfree_skb_any(skb);
	}
}

void i2c_init(u8 channel)
{
    unsigned long    status;
//Open SCU key and Reset I2C
    *(u32 *)(IO_ADDRESS(0x1e6e2000)) = 0x1688a8a8;
    status = *(u32 *)(IO_ADDRESS(0x1e6e2004));
    *(u32 *)(IO_ADDRESS(0x1e6e2004)) = status | 0x04;
    *(u32 *)(IO_ADDRESS(0x1e6e2004)) = status & ~(0x04);
//Device init
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_FUNCTION_CONTROL_REGISTER + 0x40 * channel)) = MASTER_ENABLE;
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_AC_TIMING_REGISTER_1 + 0x40 * channel)) = AC_TIMING;
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_AC_TIMING_REGISTER_2 + 0x40 * channel)) = 0;
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = ALL_CLEAR;
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_CONTROL_REGISTER + 0x40 * channel)) = 0;
}

u8  aspeedi2c_read (u8 channel, u8 device_address, u16 register_index)
{
    unsigned long    status, loop = 0;

// Start and Send device address
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_BYTE_BUFFER_REGISTER + 0x40 * channel)) = device_address;
    barrier();
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_COMMAND_REGISTER + 0x40 * channel)) = MASTER_START_COMMAND | MASTER_TX_COMMAND;
    do {
	loop++;
    	status = *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel));
    } while (!(status & TX_ACK) && (loop < LOOP_COUNT));
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = TX_ACK;
    loop = 0;

// Send HIGH BYTE (16bits EEPROM, if 8 bits EEPROM skip Send HIGH BYTE)
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_BYTE_BUFFER_REGISTER + 0x40 * channel)) = (register_index & 0xFF00) >> 8;
    barrier();
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_COMMAND_REGISTER + 0x40 * channel)) = MASTER_TX_COMMAND;
    do {
	loop++;
    	status = *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel));
    } while (!(status & TX_ACK) && (loop < LOOP_COUNT));
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = TX_ACK;
    loop = 0;

// Send register index LOW BYTE
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_BYTE_BUFFER_REGISTER + 0x40 * channel)) = register_index;
    barrier();
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_COMMAND_REGISTER + 0x40 * channel)) = MASTER_TX_COMMAND;
    do {
	loop++;
    	status = *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel));
    } while (!(status & TX_ACK) && (loop < LOOP_COUNT));
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = TX_ACK;
    loop = 0;

// Start, Send device address + 1 (read mode)
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_BYTE_BUFFER_REGISTER + 0x40 * channel)) = device_address + 1;
    barrier();
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_COMMAND_REGISTER + 0x40 * channel)) = MASTER_START_COMMAND | MASTER_TX_COMMAND | MASTER_RX_COMMAND | RX_COMMAND_LIST;
    do {
    	loop++;
    	status = *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel));
    } while (!(status & RX_DONE) && (loop < LOOP_COUNT));
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = RX_DONE;
    loop = 0;

// Stop
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_COMMAND_REGISTER + 0x40 * channel)) = MASTER_STOP_COMMAND;
    do {
    	loop++;
    	status = *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel));
    } while (!(status & STOP_DONE) && (loop < LOOP_COUNT));
    *(u32 *)(IO_ADDRESS(I2C_BASE + I2C_INTERRUPT_STATUS_REGISTER + 0x40 * channel)) = ALL_CLEAR;

    status = (*(u32 *)(IO_ADDRESS(I2C_BASE + I2C_BYTE_BUFFER_REGISTER + 0x40 * channel)) & 0xFF00) >> 8;

    return status;
}

static void auto_get_mac(int id, char *mac_addr)
{
#ifndef CONFIG_EEPROM_SUPPRESS
	unsigned char i, eeprom_empty = 1;

//Read EEPROM to get MAC address, software partner should use their own mechanism to get MAC address
//	i2c_init(3);
	for (i = 0; i < 6; i++) {
#if (CONFIG_AST1500_SOC_VER >= 2)
		mac_addr[i] = aspeedi2c_read(4, 0xA0, i + id * 6);
#else
		mac_addr[i] = aspeedi2c_read(3, 0xA0, i + id * 6);
#endif
	}
	for (i = 0; i < 6; i++) {
		if (mac_addr[i] != 0xFF) {
			eeprom_empty = 0;
		}
	}

	if (eeprom_empty == 1)
#endif
	{
//FIX MAC ADDRESS
	        mac_addr[0] = 0;
        	mac_addr[1] = 0x84;
        	mac_addr[2] = 0x14;
        	mac_addr[3] = 0xA0;
        	mac_addr[4] = 0xB0;
        	mac_addr[5] = 0x22 + id;
        }

	return;
}

void put_mac(int base, char *mac_addr)
{
	int val;

	val = ((u32)mac_addr[0])<<8 | (u32)mac_addr[1];
	outl(val, base);
	val = ((((u32)mac_addr[2])<<24)&0xff000000) |
		  ((((u32)mac_addr[3])<<16)&0xff0000) |
		  ((((u32)mac_addr[4])<<8)&0xff00)  |
		  ((((u32)mac_addr[5])<<0)&0xff);
	outl(val, base+4);
}

void get_mac(int base, char *mac_addr)
{
	int val;
	val = inl(base);
	mac_addr[0] = (val>>8)&0xff;
	mac_addr[1] = val&0xff;
	val = inl(base+4);
	mac_addr[2] = (val>>24)&0xff;
	mac_addr[3] = (val>>16)&0xff;
	mac_addr[4] = (val>>8)&0xff;
	mac_addr[5] = val&0xff;
}

// --------------------------------------------------------------------
// 	Print the Ethernet address
// --------------------------------------------------------------------
/*
void print_mac(char *mac_addr)
{
	int i;

	DO_PRINT("MAC ADDR: ");
	for (i = 0; i < 5; i++)
	{
		DO_PRINT("%2.2x:", mac_addr[i] );
	}
	DO_PRINT("%2.2x \n", mac_addr[5] );
}
*/

#if NCSI_SUPPORT
// --------------------------------------------------------------------
// 	NCSI function
// --------------------------------------------------------------------
void NCSI_Struct_Initialize(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long i;

	for (i = 0; i < 6; i++) {
		lp->NCSI_Request.DA[i] = 0xFF;
		lp->NCSI_Respond.DA[i] = 0xFF;
		lp->NCSI_Respond.SA[i] = 0xFF;
		lp->NCSI_Request.SA[i] = dev->dev_addr[i];
	}
	lp->NCSI_Request.EtherType = 0xF888;
	lp->NCSI_Request.MC_ID = 0;
	lp->NCSI_Request.Header_Revision = 0x01;
	lp->NCSI_Request.Reserved_1 = 0;
	lp->NCSI_Request.Reserved_2 = 0;
	lp->NCSI_Request.Reserved_3 = 0;
	lp->NCSI_Respond.EtherType = 0xF888;
	lp->NCSI_Respond.MC_ID = 0;
	lp->NCSI_Respond.Header_Revision = 0x01;
	lp->NCSI_Respond.Reserved_1 = 0;
	lp->NCSI_Respond.Reserved_2 = 0;
	lp->NCSI_Respond.Reserved_3 = 0;

	lp->InstanceID = 0;
	lp->Payload_Checksum = 0;
	for (i = 0; i < 4; i++) {
		lp->Payload_Pad[i] = 0;
	}
}

void Calculate_Checksum(struct net_device * dev, unsigned char *buffer_base, int Length)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned int i, CheckSum = 0;
	unsigned int Data, Data1;

	for (i = 0; i < ((Length - 14) / 2); i++) {
		Data = buffer_base[i * 2];
		Data1 = buffer_base[i * 2 + 1];
		CheckSum += ((Data << 8) + Data1);
	}
	lp->Payload_Checksum = (~(CheckSum) + 1); //2's complement
//Inverse for insert into buffer
	Data = (lp->Payload_Checksum & 0xFF000000) >> 24;
	Data1 = (lp->Payload_Checksum & 0x000000FF) << 24;
	lp->Payload_Checksum = (lp->Payload_Checksum & 0x00FFFF00) + Data + Data1;
	Data = (lp->Payload_Checksum & 0x00FF0000) >> 8;
	Data1 = (lp->Payload_Checksum & 0x0000FF00) << 8;
	lp->Payload_Checksum = (lp->Payload_Checksum & 0xFF0000FF) + Data + Data1;
}

void copy_data (struct net_device * dev, struct sk_buff * skb, int Length)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;

	memcpy ((unsigned char *)(skb->data + 30), &lp->Payload_Data, Length);
	Calculate_Checksum(dev, skb->data + 14, 30 + Length);
	memcpy ((unsigned char *)(skb->data + 30 + Length), &lp->Payload_Checksum, 4);
}

void NCSI_Rx (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long status, length, i = 0;
	volatile RX_DESC *cur_desc;


	cur_desc = &lp->rx_descs[lp->rx_idx];
 	do {
		status = cur_desc->RXPKT_RDY;
		i++;
	} while (!(((status & RX_OWNBY_SOFTWARE) != 0) || (i >= NCSI_LOOP)));

	if (i < NCSI_LOOP) {
		if (cur_desc->LRS) {
			length = cur_desc->VDBC;
			memcpy (&lp->NCSI_Respond, (unsigned char *)phys_to_virt(cur_desc->RXBUF_BADR), length);
		}
	        lp->rx_descs[lp->rx_idx].RXPKT_RDY = RX_OWNBY_FTGMAC100;
		lp->rx_idx = (lp->rx_idx+1) & (RXDES_NUM -1);
	}
}

void DeSelect_Package (struct net_device * dev, int Package_ID)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = DESELECT_PACKAGE;
	Combined_Channel_ID = (Package_ID << 5) + 0x1F; //Internal Channel ID = 0x1F, 0x1F means all channel
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (DESELECT_PACKAGE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

int Select_Package (struct net_device * dev, int Package_ID)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID, Found = 0;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = SELECT_PACKAGE;
	Combined_Channel_ID = (Package_ID << 5) + 0x1F; //Internal Channel ID = 0x1F
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (4 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 4;
        memset ((void *)lp->Payload_Data, 0, 4);
	lp->Payload_Data[3] = 1; //Arbitration Disable
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len = 30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (SELECT_PACKAGE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		Found = 0;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
		Found = 1;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;

    return Found;
}

void DeSelect_Active_Package (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = DESELECT_PACKAGE;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + 0x1F; //Internal Channel ID = 0x1F, 0x1F means all channel
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (DESELECT_PACKAGE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

int Select_Active_Package (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID, Found = 0;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = SELECT_PACKAGE;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + 0x1F; //Internal Channel ID = 0x1F
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (4 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 4;
        memset ((void *)lp->Payload_Data, 0, 4);
	lp->Payload_Data[3] = 1; //Arbitration Disable
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len = 30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (SELECT_PACKAGE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
		Found = 0;
	}
	else {
		lp->Retry = 0;
		Found = 1;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;

    return Found;
}

int Clear_Initial_State (struct net_device * dev, int Channel_ID)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID, Found = 0;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = CLEAR_INITIAL_STATE;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + Channel_ID; //Internal Channel ID = 0
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (CLEAR_INITIAL_STATE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
		Found = 0;
	}
	else {
		lp->Retry = 0;
		Found = 1;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;

    return Found;
}

void Get_Version_ID (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = GET_VERSION_ID;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (GET_VERSION_ID | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Get_Capabilities (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = GET_CAPABILITIES;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (GET_CAPABILITIES | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
		lp->NCSI_Cap.Capabilities_Flags = lp->NCSI_Respond.Payload_Data[0];
		lp->NCSI_Cap.Broadcast_Packet_Filter_Capabilities = lp->NCSI_Respond.Payload_Data[1];
		lp->NCSI_Cap.Multicast_Packet_Filter_Capabilities = lp->NCSI_Respond.Payload_Data[2];
		lp->NCSI_Cap.Buffering_Capabilities = lp->NCSI_Respond.Payload_Data[3];
		lp->NCSI_Cap.AEN_Control_Support = lp->NCSI_Respond.Payload_Data[4];
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Enable_AEN (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = AEN_ENABLE;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (8 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 8;
	lp->Payload_Data[3] = 0; //MC ID
	lp->Payload_Data[7] = 1; //Link Status Change AEN
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (AEN_ENABLE | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Enable_Set_MAC_Address (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID, i;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = SET_MAC_ADDRESS;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (8 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 8;
	for (i = 0; i < 6; i++) {
		lp->Payload_Data[i] = lp->NCSI_Request.SA[i];
	}
	lp->Payload_Data[6] = 1; //MAC Address Num = 1 --> address filter 1, fixed in sample code
	lp->Payload_Data[7] = UNICAST + 0 + ENABLE_MAC_ADDRESS_FILTER; //AT + Reserved + E
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (SET_MAC_ADDRESS | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Enable_Broadcast_Filter (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = ENABLE_BROADCAST_FILTERING;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (4 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 4;
        memset ((void *)lp->Payload_Data, 0, 4);
        lp->Payload_Data[3] = 0xF; //ARP, DHCP, NetBIOS
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (ENABLE_BROADCAST_FILTERING | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Disable_VLAN (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = DISABLE_VLAN;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (DISABLE_VLAN | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Get_Parameters (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = GET_PARAMETERS;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (GET_PARAMETERS | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
//		printk ("Retry: Command = %x, Response_Code = %x, Resonpd.Command = %x, IID = %x, lp->InstanceID = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code, lp->NCSI_Respond.Command, lp->NCSI_Respond.IID, lp->InstanceID);
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
		lp->NCSI_Cap.Capabilities_Flags = lp->NCSI_Respond.Payload_Data[0];
		lp->NCSI_Cap.Broadcast_Packet_Filter_Capabilities = lp->NCSI_Respond.Payload_Data[1];
		lp->NCSI_Cap.Multicast_Packet_Filter_Capabilities = lp->NCSI_Respond.Payload_Data[2];
		lp->NCSI_Cap.Buffering_Capabilities = lp->NCSI_Respond.Payload_Data[3];
		lp->NCSI_Cap.AEN_Control_Support = lp->NCSI_Respond.Payload_Data[4];
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Enable_Network_TX (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = ENABLE_CHANNEL_NETWORK_TX;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (ENABLE_CHANNEL_NETWORK_TX | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Disable_Network_TX (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = DISABLE_CHANNEL_NETWORK_TX;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + (lp->NCSI_Request.Payload_Length % 4) + 8;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (DISABLE_CHANNEL_NETWORK_TX | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Enable_Channel (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = ENABLE_CHANNEL;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (ENABLE_CHANNEL | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

void Disable_Channel (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = DISABLE_CHANNEL;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (4 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 4;
        memset ((void *)lp->Payload_Data, 0, 4);
        lp->Payload_Data[3] = 0x1; //ALD
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (DISABLE_CHANNEL | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}

int Get_Link_Status (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = GET_LINK_STATUS;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = 0;
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (GET_LINK_STATUS | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
    if (lp->NCSI_Respond.Payload_Data[3] & 0x40) {
	    return (lp->NCSI_Respond.Payload_Data[3] & 0x01); //Link Up or Not
    }
    else {
	    return 0; //Auto Negotiate did not finish
    }
}

void Set_Link (struct net_device * dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long Combined_Channel_ID;
	struct sk_buff * skb;

    do {
	skb = dev_alloc_skb (TX_BUF_SIZE + 16);
//TX
	lp->InstanceID++;
	lp->NCSI_Request.IID = lp->InstanceID;
	lp->NCSI_Request.Command = SET_LINK;
	Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
	lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
	lp->NCSI_Request.Payload_Length = (8 << 8);
	memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
	lp->NCSI_Request.Payload_Length = 8;
        memset ((void *)lp->Payload_Data, 0, 8);
        lp->Payload_Data[2] = 0x02; //full duplex
        lp->Payload_Data[3] = 0x04; //100M, auto-disable
	copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
	skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
	ftgmac100_wait_to_send_packet (skb, dev);
//RX
	NCSI_Rx(dev);
	if (((lp->NCSI_Respond.IID != lp->InstanceID) || (lp->NCSI_Respond.Command != (SET_LINK | 0x80)) || (lp->NCSI_Respond.Response_Code != COMMAND_COMPLETED)) && (lp->Retry == RETRY_COUNT)) {
		printk ("Retry: Command = %x, Response_Code = %x\n", lp->NCSI_Request.Command, lp->NCSI_Respond.Response_Code);
		lp->Retry++;
		lp->InstanceID--;
	}
	else {
		lp->Retry = 0;
	}
    } while ((lp->Retry != 0) && (lp->Retry <= RETRY_COUNT));
    lp->Retry = 0;
}
#endif

// --------------------------------------------------------------------
// 	Finds the CRC32 of a set of bytes.
//	Again, from Peter Cammaert's code.
// --------------------------------------------------------------------
static int crc32( char * s, int length )
{
	/* indices */
	int perByte;
	int perBit;
	/* crc polynomial for Ethernet */
	const unsigned long poly = 0xedb88320;
	/* crc value - preinitialized to all 1's */
	unsigned long crc_value = 0xffffffff;

	for ( perByte = 0; perByte < length; perByte ++ ) {
		unsigned char	c;

		c = *(s++);
		for ( perBit = 0; perBit < 8; perBit++ ) {
			crc_value = (crc_value>>1)^
				(((crc_value^c)&0x01)?poly:0);
			c >>= 1;
		}
	}
	return	crc_value;
}

static void stop_mac(unsigned long ioaddr, struct ftgmac100_local *lp)
{
	/* Bruce130130. Chip bug >= AST1510. Blame Jazoe for this shxt.
	** SW_RST_bit must be set together with (SPEED_100_bit and GMAC_MODE_bit).
	** It is harmless to do this on AST1500.
	*/
	u32 maccr = (lp->maccr_val & (SPEED_100_bit|GMAC_MODE_bit)) | SW_RST_bit;

#ifdef AUTO_TX_POLL
	/* Disable Tx Poll before SW_RST to avoid MAC goes crazy. */
	outl( (0x0<<TXPOLL_CNT)|(0x0<<RXPOLL_CNT), ioaddr + APTC_REG);
	udelay(10);
#endif

	///interrupt mask register
	outl(0, ioaddr + IER_REG );

	//outl(inl(ioaddr + MACCR_REG) & 0xFFF0, ioaddr + MACCR_REG);
	outl(maccr, ioaddr + MACCR_REG );

	/*
	** Bruce130130. Chip bug >= AST1510. Blame Jazoe for this shxt.
	** SW_RST needs longer delay.
	** Bruce141218. Per Jazoe. This is not a bug. and has to apply to ALL chips.
	*/
	udelay(10);
	outl(maccr, ioaddr + MACCR_REG);

	/* this should pause enough for the chip to be happy */
	for (; (inl( ioaddr + MACCR_REG ) & SW_RST_bit) != 0; )
	{
		mdelay(10);
		PRINTK3("RESET: reset not complete yet\n" );
	}
#if 0 //Bruce150302. Jazoe don't like this line. It is meaningless.
	while (inl( ioaddr + DMAFIFOS_REG ) & 0xF) {
		printk("Wait for MAC idle\n");
	}
#endif
}


static void phy_speed_duplex(unsigned long ioaddr, struct ftgmac100_local *lp, u32 *speed, u32 *duplex, u32 atomic)
{
	u32 id, tmp;

	id = lp->ids.miiPhyId;

	if ((id & PHYID_VENDOR_MASK) == PHYID_VENDOR_BROADCOM) {
		tmp    = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x18, atomic);
		*duplex = (tmp & 0x0001);
		*speed  = (tmp & 0x0002) ? PHY_SPEED_100M : PHY_SPEED_10M;
	} else if ((id & PHYID_VENDOR_MASK) == PHYID_VENDOR_MARVELL) {
		tmp    = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x11, atomic);
		*duplex = (tmp & PHY_DUPLEX_mask)>>13;
		*speed  = (tmp & PHY_SPEED_mask)>>14;
	} else if ((id & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8201EL) {
		tmp    = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x00, atomic);
		*duplex = (tmp & 0x0100) ? 1 : 0;
		*speed  = (tmp & 0x2000) ? PHY_SPEED_100M : PHY_SPEED_10M;
	} else if ((id & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211) {
		if (lp->ids.is_rtl8211_fiber) {
			*duplex = 1;
			*speed = PHY_SPEED_1G;
		} else {
			if (PHYID_RTL8211F_VER <= (id & PHYID_VENDOR_MODEL_VER_MASK)) {
				ftgmac100_write_phy_register_ex(ioaddr, lp->ids.phyAddr, 31, 0xa43, atomic); /* change to page 0xa43*/
				tmp = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x1A, atomic);
				*duplex = (tmp & 0x0008) >> 3;
				*speed = (tmp & 0x0030) >> 4;
				ftgmac100_write_phy_register_ex(ioaddr, lp->ids.phyAddr, 31, 0xa42, atomic); /* change back to default page*/
			} else {
				tmp = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x11, atomic);
				*duplex = (tmp & PHY_DUPLEX_mask) >> 13;
				*speed = (tmp & PHY_SPEED_mask) >> 14;
			}
		}
	} else {
		*duplex = 1;
		*speed = PHY_SPEED_100M;
	}

}

static int ast_phy_has_link(struct net_device *dev, u32 atomic)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u32 val;

	/*
	 * Some PHYs require the MII_BMSR register to be read twice due to the link bit being sticky.
	 * No harm doing it across the board.
	 */
	val = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x1, atomic);
	val = ftgmac100_read_phy_register_ex(ioaddr, lp->ids.phyAddr, 0x1, atomic);

	/* Bit[2], Link Status, link is up */
	if (val & LINK_STATUS)
		return 1;

	return 0;
}

static unsigned long phy_reset_timeout_jiffies = 0;

static void ast_phy_reset_init(void)
{
	phy_reset_timeout_jiffies = 0;
}

static void ast_phy_reset(struct net_device *dev)
{
	unsigned long flags;

#if (DUMMY_PHY & 0x10)
	return;
#endif

	if (phy_reset_timeout_jiffies == 0)
		phy_reset_timeout_jiffies = jiffies + msecs_to_jiffies(PHY_RESET_INTERVAL);

	if (time_after(jiffies, phy_reset_timeout_jiffies)) {
		printk("reset Ethernet PHY\n");
		local_irq_save(flags);
		ftgmac100_phy_configure(dev);
		local_irq_restore(flags);
		phy_reset_timeout_jiffies = jiffies + msecs_to_jiffies(PHY_RESET_INTERVAL);
	}
}

#if defined(PHY_FIBER_STATUS_IN_WQ)
static void ast_phy_change(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	unsigned int do_link_chg = 0;
	u32 has_link;

	/* polling when link down or fiber mode */
	if ((!lp->ids.is_rtl8211_fiber) && netif_carrier_ok(dev))
		goto phy_change_exit;

	has_link = ast_phy_has_link(dev, 0);

	if (has_link) {
		if (!netif_carrier_ok(dev))
			do_link_chg = 1;
	} else {
		if (netif_carrier_ok(dev))
			do_link_chg = 1;

		/* reset PHY when link is down, rtl8211 only */
		if ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211) {
			if (do_link_chg) {
				/*
				 * link up => down
				 * ast_phy_reset_init() set phy_reset_timeout_jiffies = 0
				 * to trigger phy reset after PHY_RESET_INTERVAL
				 */
				ast_phy_reset_init();
			}

			ast_phy_reset(dev);
		}
	}

	if (do_link_chg) {
		if (!has_link) {
			PRINTK("Ethernet Link OFF\n");
			netif_stop_queue(dev);
			netif_carrier_off(dev);
			ast_notify_user("e_eth_link_off");
		}
		stop_mac(ioaddr, lp);
		queue_work(lp->tx_task, &lp->link_chg_work);
	}

phy_change_exit:
	schedule_delayed_work(&lp->phy_change_work, msecs_to_jiffies(PHY_FIBER_STATUS_PERIOD_MS));
}
#endif

static void aspeed_mac_timer(unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ftgmac100_local *lp 	= (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	unsigned int speed, duplex, macSpeed;
	unsigned int do_link_chg = 0;
	u32 has_link;


	has_link = ast_phy_has_link(dev, 1);

	if (has_link) {
		phy_speed_duplex(ioaddr, lp, &speed, &duplex, 1);

		macSpeed = ((lp->maccr_val & GMAC_MODE_bit)>>8   // Move bit[9] to bit[1]
			    | (lp->maccr_val & SPEED_100_bit)>>19);  // bit[19] to bit[0]
		// The MAC hardware ignores SPEED_100_bit if GMAC_MODE_bit is set.
		if (macSpeed > PHY_SPEED_1G) macSpeed = PHY_SPEED_1G;	// 0x3 --> 0x2

		if ( ((lp->maccr_val & FULLDUP_bit)!=0) != duplex
		     || macSpeed != speed )
		{
			do_link_chg = 1;
		}
		if (!netif_carrier_ok(dev)) {
			do_link_chg = 1;
		}
	} else if (netif_carrier_ok(dev)) {
		do_link_chg = 1;
	}

	if (do_link_chg) {
		if (!has_link) {
			PRINTK("Ethernet Link OFF\n");
			netif_stop_queue(dev);
			netif_carrier_off(dev);
			ast_notify_user("e_eth_link_off");
		}
		stop_mac(ioaddr, lp);
		queue_work(lp->tx_task, &lp->link_chg_work);
	}

	lp->timer.expires = jiffies + 1 * HZ;
	add_timer(&lp->timer);
}

static void getMacAndPhy( struct net_device* dev, struct AstMacHwConfig* out )
{
    unsigned int macId = 1;

    while (macId > 0 && dev->base_addr != IP_va_base[macId])
	macId --;

    out->macId   = macId;
    out->phyAddr = IP_phy_addr[macId];
}

/*
 * MAC1 always has MII MDC+MDIO pins to access PHY registers.  We assume MAC1
 * always has a PHY chip, if MAC1 is enabled.
 * U-Boot can enable MAC2 MDC+MDIO pins for a 2nd PHY, or MAC2 might be
 * disabled (only one port), or it's sideband-RMII which has no PHY chip.
 *
 * Return miiPhyId==0 if the MAC cannot be accessed.
 * Return miiPhyId==1 if the MAC registers are OK but it cannot carry traffic.
 * Return miiPhyId==2 if the MAC can send/receive but it has no PHY chip.
 * Else return the PHY 22-bit vendor ID, 6-bit model and 4-bit revision.
 */
static void getMacHwConfig( struct net_device* dev, struct AstMacHwConfig* out )
{
    struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
    unsigned int resetControl, multiFuncPin;
#if (CONFIG_AST1500_SOC_VER == 1)
    unsigned int hwStrap_macMode;
#endif
    unsigned long ioaddr = dev->base_addr;

    getMacAndPhy(dev, out);
    out->miiPhyId = 0;
#if 0
    /* Bruce120903. Fix for set_multicast_list
    ** The following patch is for AST2050 A0. For SoC versions other than AST2050 A0, we
    ** should use A2 code path. Since AST2050 A0 doesn't exists anymore, we simply #if0
    ** this code.
    */
    unsigned int regVal;
    regVal = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_SILICON_REVISION_REG)) & 0xFF;
    if (regVal >= 0x08 && regVal <= 0x0f) {
	// AST2100 FPGA board: up to 10 means rev.A0, 11 means rev.A1
	out->isRevA0 = (regVal < 11);
    } else {
	// Real silicon: rev.A0 has 0x00 in bits[7:0]. rev A2 = 0x02 in bits[7:0]
	out->isRevA0 = ((regVal & 0x00ff) == 0x00);
	out->isRevA2 = ((regVal & 0x00ff) == 0x02);
    }
#else
	out->isRevA0 = 0;
	out->isRevA2 = 1;
#endif
    DF_support = !out->isRevA0;

#if NCSI_SUPPORT
/*
* D[15:11] in 0x1E6E2040 is NCSI scratch from U-Boot. D[15:14] = MAC1, D[13:12] = MAC2
* The meanings of the 2 bits are:
* 00(0): Dedicated PHY
* 01(1): ASPEED's EVA + INTEL's NC-SI PHY chip EVA
* 10(2): ASPEED's MAC is connected to NC-SI PHY chip directly
* 11: Reserved
*/
    if (dev->irq == 2) { //We don't have MAC_ID in private data, we use IRQ number to check MAC1/MAC2 in current version
	    regVal = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_SCRATCH_REG)) & 0xC000;
	    if (regVal == 0x8000) {
		lp->NCSI_support = 1;
		printk ("MAC1 Support NCSI\n");
	    }
	    else if (regVal == 0x4000) {
		lp->INTEL_NCSI_EVA_support = 1;
		printk ("MAC1 Support INTEL_NCSI_EVA\n");
	    }
	    else {
		lp->INTEL_NCSI_EVA_support = 0;
		lp->NCSI_support = 0;
		printk ("MAC1 Support neither NCSI nor INTEL_NCSI_EVA\n");
	    }
    }
    else if (dev->irq == 3) { //MAC2
	    regVal = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_SCRATCH_REG)) & 0x3000;
	    if (regVal == 0x2000) {
		lp->NCSI_support = 1;
		printk ("MAC2 Support NCSI\n");
	    }
	    else if (regVal == 0x1000) {
		lp->INTEL_NCSI_EVA_support = 1;
		printk ("MAC2 Support INTEL_NCSI_EVA\n");
	    }
	    else {
		lp->INTEL_NCSI_EVA_support = 0;
		lp->NCSI_support = 0;
		printk ("MAC2 Support neither NCSI nor INTEL_NCSI_EVA\n");
	    }
    }
#else
	lp->INTEL_NCSI_EVA_support = 0;
	lp->NCSI_support = 0;
#endif

    // We assume the Clock Stop register does not disable the MAC1 or MAC2 clock
    // unless Reset Control also holds the MAC in reset.
#if (CONFIG_AST1500_SOC_VER >= 2)
    resetControl    = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_RESET_CONTROL_REG));

    switch (out->macId) {
    case 0:
	if (resetControl & SCU_RESET_MAC1)
	    goto no_phy_access;
	out->miiPhyId = 1;		// MAC registers are accessible
        multiFuncPin = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL3_REG));
        if (lp->NCSI_support == 0) {
            if (0 == (multiFuncPin & (SCU_MFP_MAC1_MDIO | SCU_MFP_MAC1_MDC))
                && out->phyAddr <= 0x1f)
                goto no_phy_access;
	    out->miiPhyId = 2;
        }
        else {
	    out->miiPhyId = 2;
        }
	break;

    case 1:
	if (resetControl & SCU_RESET_MAC2)
	    goto no_phy_access;
	out->miiPhyId = 1;		// The MAC itself is usable
        multiFuncPin = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_MULTIFUNCTION_PIN_CTL5_REG));
	// If there are no MII2DC+MII2DIO pins for a MAC2 PHY bus...
        if (lp->NCSI_support == 0) {
	    if (0==(multiFuncPin & SCU_MFP_MAC2_MDC_MDIO)
	        && out->phyAddr <= 0x1f)
	        goto no_phy_access;
	    out->miiPhyId = 2;
        }
        else {
	    out->miiPhyId = 2;
        }
	break;

    default:
	goto no_phy_access;			// AST2100 has only two MACs
    }
#else
    resetControl    = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_RESET_CONTROL_REG));
    hwStrap_macMode = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_HARDWARE_TRAPPING_REG));
    hwStrap_macMode =
	(hwStrap_macMode & SCU_HT_MAC_INTERFACE) >> SCU_HT_MAC_INTF_LSBIT;
    multiFuncPin   = inl(IO_ADDRESS(ASPEED_SCU_BASE + SCU_MULTIFUNCTION_PIN_REG));

    switch (out->macId) {
    case 0:
	if (resetControl & SCU_RESET_MAC1)
	    goto no_phy_access;
	out->miiPhyId = 1;		// MAC registers are accessible
	if (hwStrap_macMode == 0x7)
	    goto no_phy_access;		// No MAC interfaces enabled
	out->miiPhyId = 2;		// The MAC itself is usable
	break;

    case 1:
	if (resetControl & SCU_RESET_MAC2)
	    goto no_phy_access;
	out->miiPhyId = 1;		// MAC registers are accessible
	if (hwStrap_macMode == 0x7)
	    goto no_phy_access;		// No MAC interfaces enabled
	if (MAC_INTF_SINGLE_PORT_MODES & (1u << hwStrap_macMode)) {
	    goto no_phy_access;		// Second MAC interface not enabled
        }
	if (hwStrap_macMode == SCU_HT_MAC_MII_MII) {
	    if (0==(multiFuncPin & SCU_MFP_MAC2_MII_INTF))
		goto no_phy_access;	// MII pins not enabled for MAC2 in MII mode
	}
	out->miiPhyId = 2;		// The MAC itself is usable
	// If there are no MII2DC+MII2DIO pins for a MAC2 PHY bus...
	if (0==(multiFuncPin & SCU_MFP_MAC2_MDC_MDIO)
	    && out->phyAddr <= 0x1f)	// ...and the PHY chip is on the MAC2 bus
	    goto no_phy_access;		// then here is no MAC2 PHY chip.
	break;

    default:
	goto no_phy_access;			// AST2100 has only two MACs
    }
#endif
    // For now, we only support a PHY chip on the MAC's own MDC+MDIO bus.
    if (out->phyAddr > 0x1f) {
no_phy_access:
	out->phyAddr = 0xff;
	return;
    }

	if (lp->NCSI_support == 0) {
		out->miiPhyId = ftgmac100_read_phy_register(ioaddr, out->phyAddr, 0x02);
		if (out->miiPhyId == 0xFFFF) { //Realtek PHY at address 1
			out->phyAddr = 1;
		}
		out->miiPhyId = ftgmac100_read_phy_register(ioaddr, out->phyAddr, 0x02);
		out->miiPhyId = (out->miiPhyId & 0xffff) << 16;
		out->miiPhyId |= ftgmac100_read_phy_register(ioaddr, out->phyAddr, 0x03) & 0xffff;

		switch (out->miiPhyId >> 16) {
		case 0x0040:	// Broadcom
		case 0x0141:	// Marvell
		case 0x001c:	// Realtek
			if (((out->miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211)
				&& ((out->miiPhyId & PHYID_VENDOR_MODEL_VER_MASK) >= PHYID_RTL8211D_VER))
			{
				word r1, r15;
				r1 = ftgmac100_read_phy_register(ioaddr, out->phyAddr, 0x01);
				r15 = ftgmac100_read_phy_register(ioaddr, out->phyAddr, 0x0F);
				/*
				** Bruce130830. Check PHY capability to identify fiber mode (1000Base-X full).
				*/
				/*
				 * reg 0x01  [8]: 1 => support extend status register
				 * reg 0x0F [15]: 1 => support 1000Base-X (fiber)
				 */
				if ((r1 & 0x0100) && (r15 == 0x8000)) {
					out->is_rtl8211_fiber = 1;
					PRINTK("RTL8211DN Fiber mode detected!\n");
				} else {
					out->is_rtl8211_fiber = 0;
				}

			}
			break;

		default:
			// Leave miiPhyId for DO_PRINT(), but reset phyAddr.
			// out->miiPhyId = 2;
			goto no_phy_access;
			break;
		}
	}
	return;
}

/*
 . Function: ftgmac100_reset( struct device* dev )
 . Purpose:
 .  	This sets the SMC91111 chip to its normal state, hopefully from whatever
 . 	mess that any other DOS driver has put it in.
 .
 . Maybe I should reset more registers to defaults in here?  SOFTRST  should
 . do that for me.
 .
 . Method:
 .	1.  send a SOFT RESET
 .	2.  wait for it to finish
 .	3.  enable autorelease mode
 .	4.  reset the memory management unit
 .	5.  clear all interrupts
 .
*/
static void ftgmac100_reset( struct net_device* dev )
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	struct AstMacHwConfig* ids = &lp->ids;
	unsigned long ioaddr = dev->base_addr;
	unsigned int tmp, speed, duplex;

	getMacHwConfig(dev, ids);
	PRINTK("%s:ftgmac100_reset, phyAddr=0x%x, miiPhyId=0x%04x_%04x\n",
	       dev->name, ids->phyAddr, (ids->miiPhyId >> 16), (ids->miiPhyId & 0xffff));
	if (ids->miiPhyId < 1)
	    return;	// Cannot access MAC registers

	// Check the link speed and duplex.
	// They are not valid until auto-neg is resolved, which is reg.1 bit[5],
	// or the link is up, which is reg.1 bit[2].
#if 0
	printk("0(0x%08x), 1(0x%08x), 4(0x%08x), 5(0x%08x), 9(0x%08x), 17(0x%08x)\n",
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x0),
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x1),
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x4),
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x5),
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x9),
	       ftgmac100_read_phy_register (ioaddr, ids->phyAddr, 0x11));
#endif


	if (ids->phyAddr < 0xff)
	    tmp = ftgmac100_read_phy_register(ioaddr, ids->phyAddr, 0x1);
	else tmp = 0;

	if (0==(tmp & (1u<<5 | 1u<<2)) || ids->phyAddr >= 0xff) {
	    // No PHY chip, or link has not negotiated.
	    speed  = PHY_SPEED_100M;
	    duplex = 1;
	    netif_carrier_off(dev);
	} else
		phy_speed_duplex(ioaddr, lp, &speed, &duplex, 1);

	if (speed == PHY_SPEED_1G) {
	    // Set SPEED_100_bit too, for consistency.
	    lp->maccr_val |= GMAC_MODE_bit | SPEED_100_bit;
	    tmp = inl( ioaddr + MACCR_REG );
	    tmp |= GMAC_MODE_bit | SPEED_100_bit;
	    outl(tmp, ioaddr + MACCR_REG );
	} else {
	    lp->maccr_val &= ~(GMAC_MODE_bit | SPEED_100_bit);
	    tmp = inl( ioaddr + MACCR_REG );
	    tmp &= ~(GMAC_MODE_bit | SPEED_100_bit);
	    if (speed == PHY_SPEED_100M) {
		lp->maccr_val |= SPEED_100_bit;
		tmp |= SPEED_100_bit;
	    }
	    outl(tmp, ioaddr + MACCR_REG );
	}
	if (duplex)
		lp->maccr_val |= FULLDUP_bit;
	else
		lp->maccr_val &= ~FULLDUP_bit;

	stop_mac(ioaddr, lp);
}

/*
 . Function: ftgmac100_enable
 . Purpose: let the chip talk to the outside work
 . Method:
 .	1.  Enable the transmitter
 .	2.  Enable the receiver
 .	3.  Enable interrupts
*/
static void ftgmac100_enable( struct net_device *dev )
{
	unsigned long ioaddr 	= dev->base_addr;
	int i;
	struct ftgmac100_local *lp 	= (struct ftgmac100_local *)dev->priv;
	unsigned int tmp_rsize;		//Richard
	unsigned int rfifo_rsize;	//Richard
	unsigned int tfifo_rsize;	//Richard
#if NCSI_SUPPORT
	unsigned long Package_Found = 0, Channel_Found = 0, Re_Send = 0, Link_Status;
#endif

	PRINTK2("%s:ftgmac100_enable\n", dev->name);

	outl(lp->rx_buff_size, ioaddr + RBSR_REG); //for NC Body

	memset((void*)lp->rx_descs, 0, sizeof(RX_DESC) * lp->rx_buff_num);

	printk("MAC buffer size = %d\n", lp->rx_buff_size);
	for (i = 0; i < lp->rx_buff_num; ++i)
	{
		if (lp->rx_skbuff[i] == NULL)
		{
			/* Allocated fixed size of skbuff */
			// Books say dev_alloc_skb() adds 16 bytes to the given
			// length, for an Ethernet MAC header.  RX_BUF_SIZE==1536
			// already covers the header, and is 48*32 so 48 cache lines.
			// alloc_skb() adds sizeof skb_shinfo and rounds up to cache line.
			struct sk_buff *skb = dev_alloc_skb(lp->rx_buff_size);
			lp->rx_skbuff[i] = skb;
			if (skb == NULL) {
				printk(KERN_ERR "failed to allocate MAC Rx buffer (%d)\n", i);
				BUG();
				break;
			}
			skb->dev = dev;	/* Mark as being used by this device. */
			// skb_reserve(skb, 2);	/* 16 byte align the IP header. */
			// ASPEED: Align IP header to 32-byte cache line.
			skb_reserve (skb, 2);
			dmac_inv_range ((unsigned long)skb->data, (unsigned long)skb->data + lp->rx_buff_size);
			/* Rubicon now supports 40 bits of addressing space. */
			lp->rx_descs[i].RXBUF_BADR = virt_to_phys(skb->tail);
			lp->rx_descs[i].VIR_RXBUF_BADR = (unsigned long)skb->tail;
		}
		lp->rx_descs[i].RXPKT_RDY = RX_OWNBY_FTGMAC100;				// owned by FTMAC100
		lp->rx_descs[i].EDORR = 0;		// not last descriptor
	}
	lp->rx_descs[lp->rx_buff_num - 1].EDORR = 1;					// is last descriptor
	lp->rx_idx = 0;

	for (i=0; i<TXDES_NUM; ++i)
	{
		lp->tx_descs[i].TXDMA_OWN = TX_OWNBY_SOFTWARE;			// owned by software
		lp->tx_skbuff[i] = NULL;
	}
	lp->tx_idx = 0;lp->old_tx = 0;lp->tx_free=TXDES_NUM;

#if HPTX_SUPPORT
	for (i=0; i<TXDES_NUM; ++i)
	{
		lp->hptx_descs[i].TXDMA_OWN = TX_OWNBY_SOFTWARE;			// owned by software
		lp->hptx_skbuff[i] = NULL;
	}
	lp->hptx_idx = 0;lp->old_hptx = 0;lp->hptx_free=TXDES_NUM;
#endif

	/* set the MAC address */
	put_mac(ioaddr + MAC_MADR_REG, dev->dev_addr);

	outl( lp->rx_descs_dma, ioaddr + RXR_BADR_REG);
#if TEST_USE_HPTX
	outl( lp->tx_descs_dma, ioaddr + HPTXR_BADR_REG);
#else
	outl( lp->tx_descs_dma, ioaddr + TXR_BADR_REG);
#endif

#if HPTX_SUPPORT
	outl( lp->hptx_descs_dma, ioaddr + HPTXR_BADR_REG);
#endif

	outl( 0x00001010, ioaddr + ITC_REG);


#ifdef AUTO_TX_POLL //Bruce130123. For Resolving chip bug.
	outl( (0x1<<TXPOLL_CNT)|(0x1<<RXPOLL_CNT), ioaddr + APTC_REG);
#else
	outl( (0UL<<TXPOLL_CNT)|(0x1<<RXPOLL_CNT), ioaddr + APTC_REG);
#endif
	outl( 0x44f97, ioaddr + DBLAC_REG );

	///	outl( inl(FCR_REG)|0x1, ioaddr + FCR_REG );				// enable flow control
	///	outl( inl(BPR_REG)|0x1, ioaddr + BPR_REG );				// enable back pressure register

	// +++++ Richard +++++ //
	tmp_rsize = inl( ioaddr + FEAR_REG );
	rfifo_rsize = tmp_rsize & 0x00000007;
	tfifo_rsize = (tmp_rsize >> 3)& 0x00000007;

	tmp_rsize = inl( ioaddr + TPAFCR_REG );
#if (CONFIG_AST1500_SOC_VER == 1)
	tmp_rsize &= ~0x3f000000;
	tmp_rsize |= (tfifo_rsize << 27);
	tmp_rsize |= (rfifo_rsize << 24);
#else
	//set up early receive threshold
	tmp_rsize &= ~0x0000FF00;
	if (lp->maccr_val & GMAC_MODE_bit) {
		tmp_rsize |= (2 << 8);
	} else {
		//Bug#2011101100. Set to 4 to avoid MAC hang. (E_F)
		/*Bruce111114. I can still hit similar issue when client is running at 100Mbps and system
		**is connected to host but doing nothing (idle). Disable this feature.
		*/
		//tmp_rsize |= (4 << 8);
	}
#endif
	outl(tmp_rsize, ioaddr + TPAFCR_REG);
	// ----- Richard ----- //

//river set MAHT0, MAHT1
	if (lp->maccr_val & GMAC_MODE_bit) {
		outl (lp->GigaBit_MAHT0, ioaddr + MAHT0_REG);
		outl (lp->GigaBit_MAHT1, ioaddr + MAHT1_REG);
	}
	else {
		outl (lp->Not_GigaBit_MAHT0, ioaddr + MAHT0_REG);
		outl (lp->Not_GigaBit_MAHT1, ioaddr + MAHT1_REG);
	}

	/// enable trans/recv,...
	outl(lp->maccr_val, ioaddr + MACCR_REG );

#if NCSI_SUPPORT
//NCSI Start
//DeSelect Package/ Select Package
	if ((lp->NCSI_support == 1) || (lp->INTEL_NCSI_EVA_support == 1)) {
		NCSI_Struct_Initialize(dev);
		for (i = 0; i < 4; i++) {
			DeSelect_Package (dev, i);
			Package_Found = Select_Package (dev, i);
			if (Package_Found == 1) {
//AST2100/AST2050/AST1100 supports 1 slave only
				lp->NCSI_Cap.Package_ID = i;
				break;
			}
		}
		if (Package_Found != 0) {
//Initiali State
			for (i = 0; i < 2; i++) { //Suppose 2 channels in current version, You could modify it to 0x1F to support 31 channels
				Channel_Found = Clear_Initial_State(dev, i);
				if (Channel_Found == 1) {
					lp->NCSI_Cap.Channel_ID = i;
					printk ("Found NCSI Network Controller at (%d, %d)\n", lp->NCSI_Cap.Package_ID, lp->NCSI_Cap.Channel_ID);
//Get Version and Capabilities
					Get_Version_ID(dev);
					Get_Capabilities(dev);
//Configuration
					Select_Active_Package(dev);
//Set MAC Address
					Enable_Set_MAC_Address(dev);
//Enable Broadcast Filter
					Enable_Broadcast_Filter(dev);
//Disable VLAN
					Disable_VLAN(dev);
//Enable AEN
					Enable_AEN(dev);
//Get Parameters
					Get_Parameters(dev);
//Enable TX
					Enable_Network_TX(dev);
//Enable Channel
					Enable_Channel(dev);
//Get Link Status
Re_Get_Link_Status:
					Link_Status = Get_Link_Status(dev);
					if (Link_Status == LINK_UP) {
						printk ("Using NCSI Network Controller (%d, %d)\n", lp->NCSI_Cap.Package_ID, lp->NCSI_Cap.Channel_ID);
						netif_carrier_on(dev);
						break;
					}
					else if ((Link_Status == LINK_DOWN) && (Re_Send < 2)) {
						Re_Send++;
						netif_carrier_off(dev);
						goto Re_Get_Link_Status;
					}
//Disable TX
					Disable_Network_TX(dev);
//Disable Channel
//					Disable_Channel(dev);
					Re_Send = 0;
					Channel_Found = 0;
				}
			}
		}
	}
#endif

	/* Initial Link Status. This code should be compatible with any PHY.  */
	{
		u32 has_link;

		has_link = ast_phy_has_link(dev, 1);
		if ((has_link) && !netif_carrier_ok(dev)) {
			unsigned short val = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x11);
			unsigned short speed = ((val & PHYSRC_SPEED_MASK) >> PHYSRC_SPEED_OFFSET);
			PRINTK("Init Ethernet Link ON (%d)\n", (lp->ids.is_rtl8211_fiber)?(0x10):(speed));
			netif_carrier_on(dev);
			netif_wake_queue(dev);
			ast_notify_user("e_eth_link_on");
		} else if (!(has_link) && netif_carrier_ok(dev)) {
			PRINTK("Init Ethernet Link OFF\n");
			netif_stop_queue(dev);
			netif_carrier_off(dev);
			ast_notify_user("e_eth_link_off");
		}
	}

	/* now, enable interrupts */

	if (((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_MARVELL) ||
	    ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211)) {
		unsigned int r;
		r =
#if !(DUMMY_PHY & 0x8)
			PHYSTS_CHG_bit		|
#endif
			AHB_ERR_bit			|
			TPKT_LOST_bit		|
//			TPKT2E_bit			|
			RPKT_LOST_bit 	|
			RXBUF_UNAVA_bit		|
			RPKT2B_bit;

		if (lp->ids.is_rtl8211_fiber)
			r &= ~PHYSTS_CHG_bit;

		outl(r, ioaddr + IER_REG);
	}
	else if (((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_BROADCOM) ||
		 ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8201EL)) {
		outl(
			AHB_ERR_bit			|
			TPKT_LOST_bit		|
//			TPKT2E_bit			|
			RXBUF_UNAVA_bit		|
			RPKT2B_bit
        	,ioaddr + IER_REG
       	 	);
	}
}

/*
 . Function: ftgmac100_shutdown
 . Purpose:  closes down the SMC91xxx chip.
 . Method:
 .	1. zero the interrupt mask
 .	2. clear the enable receive flag
 .	3. clear the enable xmit flags
 .
 . TODO:
 .   (1) maybe utilize power down mode.
 .	Why not yet?  Because while the chip will go into power down mode,
 .	the manual says that it will wake up in response to any I/O requests
 .	in the register space.   Empirical results do not show this working.
*/
static void ftgmac100_shutdown( unsigned int ioaddr )
{
	///interrupt mask register
	outl( 0, ioaddr + IER_REG );

	/// enable trans/recv,...
	outl( 0, ioaddr + MACCR_REG );
}


/*
 . Function: ftgmac100_wait_to_send_packet( struct sk_buff * skb, struct device * )
 . Purpose:
 .    Attempt to allocate memory for a packet, if chip-memory is not
 .    available, then tell the card to generate an interrupt when it
 .    is available.
 .
 . Algorithm:
 .
 . o	if the saved_skb is not currently null, then drop this packet
 .		on the floor.  This should never happen, because of TBUSY.
 . o	if the saved_skb is null, then replace it with the current packet,
 . o	See if I can sending it now.
 . o 	(NO): Enable interrupts and let the interrupt handler deal with it.
 . o	(YES):Send it now.
*/
static int _ftgmac100_wait_to_send_packet( struct sk_buff * skb, struct net_device * dev, int hp )
{
	struct ftgmac100_local *lp 	= (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr 	= dev->base_addr;
	volatile TX_DESC *cur_desc;
	unsigned int		length;
	unsigned long flags;

	int _tx_idx;
	struct sk_buff **_tx_skbuff;

	spin_lock_irqsave(&lp->lock,flags);

	if (skb==NULL)
	{
		DO_PRINT("%s(%d): NULL skb???\n", __FILE__,__LINE__);
		spin_unlock_irqrestore(&lp->lock, flags);
		return 0;
	}
#if HPTX_SUPPORT
	if (hp) {
		_tx_idx = lp->hptx_idx;
		cur_desc = &lp->hptx_descs[_tx_idx];
		_tx_skbuff = lp->hptx_skbuff;
	} else
#endif
	{
		_tx_idx = lp->tx_idx;
		cur_desc = &lp->tx_descs[_tx_idx];
		_tx_skbuff = lp->tx_skbuff;
	}

	PRINTK3("%s:ftgmac100_wait_to_send_packet, skb=%x\n", dev->name, skb);

#ifdef not_complete_yet
	if (cur_desc->TXDMA_OWN != TX_OWNBY_SOFTWARE)		/// no empty transmit descriptor
	{
		DO_PRINT("no empty transmit descriptor\n");
		DO_PRINT("jiffies = %d\n", jiffies);
		lp->stats.tx_dropped++;
		netif_stop_queue(dev);		/// waiting to do:
		spin_unlock_irqrestore(&lp->lock, flags);

		return 1;
   	}
#endif /* end_of_not */

	//if (cur_desc->TXDMA_OWN != TX_OWNBY_SOFTWARE)		/// no empty transmit descriptor
	if ((hp && !lp->hptx_free) || (!hp && !lp->tx_free))
	{
		DO_PRINT("no empty TX descriptor:0x%x:0x%x\n",
				(unsigned int)cur_desc,((unsigned int *)cur_desc)[0]);
		lp->stats.tx_dropped++;
		netif_stop_queue(dev);		/// waiting to do:
		spin_unlock_irqrestore(&lp->lock, flags);

		return 1;
   	}
	_tx_skbuff[_tx_idx] = skb;
	length = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
	length = min(length, lp->tx_buff_size);

#if FTMAC100_DEBUG > 2
	DO_PRINT("Transmitting Packet at 0x%x, skb->data = %x, len = %x\n",
		 (unsigned int)cur_desc->VIR_TXBUF_BADR, skb->data, length);
	print_packet( skb->data, length );
#endif

	cur_desc->VIR_TXBUF_BADR = (unsigned long)skb->data;
	cur_desc->TXBUF_BADR = virt_to_phys(skb->data);
	dmac_clean_range((unsigned long)skb->data, (unsigned long)(skb->data + length));

	//clean_dcache_range(skb->data, (char*)(skb->data + length));

	cur_desc->TXBUF_Size = length;
	cur_desc->LTS = 1;
	cur_desc->FTS = 1;

	cur_desc->TX2FIC = 0;
	cur_desc->TXIC = 0;

	cur_desc->TXDMA_OWN = TX_OWNBY_FTGMAC100;

#if HPTX_SUPPORT
	if (hp) {
		_tx_idx = (_tx_idx + 1) & (TXDES_NUM - 1);
		lp->hptx_idx = _tx_idx;
		lp->hptx_free--;
		outl( 0xffffffff, ioaddr + HPTXPD_REG);
	} else
#endif
	{
		_tx_idx = (_tx_idx + 1) & (TXDES_NUM - 1);
		lp->tx_idx = _tx_idx;
		lp->tx_free--;
#if TEST_USE_HPTX
		outl( 0xffffffff, ioaddr + HPTXPD_REG);
#else
		outl( 0xffffffff, ioaddr + TXPD_REG);
#endif
	}
	lp->stats.tx_packets++;

#ifdef FREE_TX_IN_WQ
	queue_work(lp->tx_task, &lp->tx_free_work);
	//queue_delayed_work(lp->tx_task, &lp->tx_free_work, msecs_to_jiffies(5));
#else
	tasklet_schedule(&lp->tx_task);
#endif

	dev->trans_start = jiffies;

	spin_unlock_irqrestore(&lp->lock, flags);

	if (lp->tx_free <= 0) {
		DO_PRINT("no free TX descriptor:0x%x:0x%x\n",
				(unsigned int)cur_desc,((unsigned int *)cur_desc)[0]);

		ftgmac100_free_tx((unsigned long)dev);
		if (lp->tx_free <= 0) {
			DO_PRINT("REALLY ran out of tx desc!!\n");
			netif_stop_queue(dev);
			queue_work(lp->tx_task, &lp->tx_free_work);
		}
	}

	return 0;
}

int ftgmac100_wait_to_send_packet( struct sk_buff * skb, struct net_device * dev )
{
	int hp = 1;

	if (skb->priority == 0x16881688)
		hp = 0;

#if (PATCH_CODEV3 & PATCH_AST1520A0_BUG14030300_MAC_HQ)
	hp = 0;
#endif

	return _ftgmac100_wait_to_send_packet(skb, dev, hp);
}

/*-------------------------------------------------------------------------
 |
 | ftgmac100_destructor( struct device * dev )
 |   Input parameters:
 |	dev, pointer to the device structure
 |
 |   Output:
 |	None.
 |
 ------------------------------------------------------------------ftg---------
*/
void ftgmac100_destructor(struct net_device *dev)
{
	PRINTK3("%s:ftgmac100_destructor\n", dev->name);
}

#define dma_allocate(x,y,z,w) dma_alloc_coherent((x),(y),(dma_addr_t*)(z),(w))
void ftgmac100_ringbuf_alloc(struct net_device *dev)
{
	int i;
	struct ftgmac100_local *lp;

	lp = (struct ftgmac100_local *)dev->priv;

	lp->rx_descs = dma_allocate( NULL, sizeof(RX_DESC) * lp->rx_buff_num, &(lp->rx_descs_dma),GFP_DMA|GFP_KERNEL );

	if (lp->rx_descs == NULL || (( (u32)lp->rx_descs & 0xf)!=0))
	{
		DO_PRINT("Receive Ring Buffer(desc)  allocation error, lp->rx_desc = %p\n",
			 lp->rx_descs);
		BUG();
	}

	memset((void*)lp->rx_descs, 0, sizeof(RX_DESC) * lp->rx_buff_num);

	for (i = 0; i < lp->rx_buff_num; ++i)
	{
		lp->rx_descs[i].EDORR = 0;		// not last descriptor
		{
			/* Allocated fixed size of skbuff */
			// Books say dev_alloc_skb() adds 16 bytes to the given
			// length, for an Ethernet MAC header.  RX_BUF_SIZE==1536
			// already covers the header, and is 48*32 so 48 cache lines.
			// alloc_skb() adds sizeof skb_shinfo and rounds up to cache line.
			struct sk_buff *skb = dev_alloc_skb(lp->rx_buff_size);
			lp->rx_skbuff[i] = skb;
			if (skb == NULL) {
				printk (KERN_ERR
					"%s: alloc_list: allocate Rx buffer error! ",
					dev->name);
				break;
			}
			skb->dev = dev;	/* Mark as being used by this device. */
			// skb_reserve(skb, 2);	/* 16 byte align the IP header. */
			// ASPEED: Align IP header to 32-byte cache line.
			skb_reserve (skb, 2);
			dmac_inv_range ((unsigned long)skb->data, (unsigned long)skb->data + lp->rx_buff_size);
			/* Rubicon now supports 40 bits of addressing space. */
			lp->rx_descs[i].RXBUF_BADR = virt_to_phys(skb->tail);
			lp->rx_descs[i].VIR_RXBUF_BADR = (unsigned long)skb->tail;
		}
	}
	lp->rx_descs[lp->rx_buff_num - 1].EDORR = 1;					// is last descriptor

	lp->tx_descs = dma_allocate(NULL, sizeof(TX_DESC)*TXDES_NUM, &(lp->tx_descs_dma) ,GFP_DMA|GFP_KERNEL);

	if (lp->tx_descs == NULL || (( (u32)lp->tx_descs & 0xf)!=0))
	{
		DO_PRINT("Transmit Ring Buffer (desc) allocation error, lp->tx_desc = %p\n",
			 lp->tx_descs);
		BUG();
	}

	memset((void*)lp->tx_descs, 0, sizeof(TX_DESC)*TXDES_NUM);

	for (i=0; i<TXDES_NUM; ++i)
	{
		lp->tx_descs[i].EDOTR = 0;			// not last descriptor
	}
	lp->tx_descs[TXDES_NUM-1].EDOTR = 1;			// is last descriptor

#if HPTX_SUPPORT
	lp->hptx_descs = dma_allocate(NULL, sizeof(TX_DESC)*TXDES_NUM, &(lp->hptx_descs_dma) ,GFP_DMA|GFP_KERNEL);

	if (lp->hptx_descs == NULL || (( (u32)lp->hptx_descs & 0xf)!=0))
	{
		DO_PRINT("High Priority Transmit Ring Buffer (desc) allocation error, lp->hptx_desc = %p\n",
			 lp->hptx_descs);
		BUG();
	}

	memset((void*)lp->hptx_descs, 0, sizeof(TX_DESC)*TXDES_NUM);

	for (i=0; i<TXDES_NUM; ++i)
	{
		lp->hptx_descs[i].EDOTR = 0;			// not last descriptor
	}
	lp->hptx_descs[TXDES_NUM-1].EDOTR = 1;			// is last descriptor

	PRINTK("lp->hptx_descs = %p, lp->hptx_rx_descs_dma = %x\n",
			lp->hptx_descs, lp->hptx_descs_dma);
#endif

	PRINTK("lp->rx_descs = %p, lp->rx_rx_descs_dma = %x\n",
			lp->rx_descs, lp->rx_descs_dma);
	PRINTK("lp->tx_descs = %p, lp->tx_rx_descs_dma = %x\n",
			lp->tx_descs, lp->tx_descs_dma);
}

void ftgmac100_ringbuf_free(struct net_device *dev)
{
	struct ftgmac100_local *lp;

	lp = (struct ftgmac100_local *)dev->priv;

	if (!lp)
		return;

	if (lp->rx_descs)
		dma_free_coherent( NULL, sizeof(RX_DESC) * lp->rx_buff_num,
		                                 (void*)lp->rx_descs, (dma_addr_t)lp->rx_descs_dma );
	if (lp->tx_descs)
		dma_free_coherent( NULL, sizeof(TX_DESC)*TXDES_NUM,
		                                 (void*)lp->tx_descs, (dma_addr_t)lp->tx_descs_dma );
#if HPTX_SUPPORT
	if (lp->hptx_descs)
		dma_free_coherent( NULL, sizeof(TX_DESC)*TXDES_NUM,
		                                 (void*)lp->hptx_descs, (dma_addr_t)lp->hptx_descs_dma );

	lp->hptx_descs = NULL; lp->hptx_descs_dma = 0;
#endif

	lp->rx_descs = NULL; lp->rx_descs_dma = 0;
	lp->tx_descs = NULL; lp->tx_descs_dma = 0;
}

static struct proc_dir_entry *proc_ftgmac100;

static int ftgmac100_read_proc(char *page, char **start,  off_t off, int count, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ftgmac100_local *lp 	= (struct ftgmac100_local *)dev->priv;
	int num;
	int i;

	num = sprintf(page, "lp->rx_idx = %d\n", lp->rx_idx);
	for (i=0; i< lp->rx_buff_num; ++i)
	{
		num += sprintf(page + num, "[%d].RXDMA_OWN = %d\n", i, lp->rx_descs[i].RXPKT_RDY);
	}
	return num;
}


#if FTMAC100_DEBUG
#include <linux/platform_device.h>

#define AST_DBG_BITRATE    (1 << 0)

unsigned long net_debug_flag = 0;
unsigned long gJiffiesStart = 0;
unsigned long gRxTotalLen = 0;
unsigned long gJiffiesStartEx = 0;
unsigned long gRxTotalLenEx = 0; //in KBytes
unsigned int gBitrateInterval = 4; // 10 seconds
unsigned int gRcvCnt = 0;

static ssize_t show_flag(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%lx\n", net_debug_flag);
}

static ssize_t store_flag(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long flag;

	sscanf(buf, "%lx", &flag);

	if (flag & AST_DBG_BITRATE) {
		gJiffiesStart = 0;
		gRxTotalLen = 0;
		gJiffiesStartEx = 0;
		gRxTotalLenEx = 0;
		//gRcvCnt = 0;
	}

	net_debug_flag = flag;

	return count;
}
DEVICE_ATTR(net_debug, (S_IRUGO | S_IWUSR), show_flag, store_flag);

static ssize_t show_link_mode(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct net_device *dev = (struct net_device *)pdev->driver_data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u32 speed, duplex;

	phy_speed_duplex(ioaddr, lp, &speed, &duplex, 0);

	/*
	** Bruce151126. Link manager use this sysfs string.
	** DO NOT change the string value to keep compatibility.
	*/
	switch (speed)
	{
	case PHY_SPEED_10M:
		sprintf(buf, "10M\n");
		break;
	case PHY_SPEED_100M:
		sprintf(buf, "100M\n");
		break;
	case PHY_SPEED_1G:
		sprintf(buf, "1G\n");
		break;
	default:
		sprintf(buf, "unknown\n");
		break;
	}
	return strlen(buf);
}

static ssize_t store_link_mode(struct device *pdev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct net_device *dev = (struct net_device *)pdev->driver_data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	struct AstMacHwConfig* ids = &lp->ids;
	unsigned long ioaddr = dev->base_addr;
	unsigned int tmp;
	unsigned int mode;

	sscanf(buf, "%x", &mode);
	PRINTK("Set MAC link mode to %d\n", mode);

	switch (mode)
	{
	case PHY_SPEED_10M:
		PRINTK("Unsupport yet\n");
		break;
	case PHY_SPEED_100M:
		if (ids->is_rtl8211_fiber) {
			PRINTK("Unsupported\n");
			break;
		}

		// Reset and force 100M and enable auto negotiation.
		ftgmac100_write_phy_register_sleep(ioaddr, ids->phyAddr, 0x4, 0x1E0);
		ftgmac100_write_phy_register_sleep(ioaddr, ids->phyAddr, 0x9, 0);
		ftgmac100_write_phy_register_sleep(ioaddr, ids->phyAddr, 0x0, 0x3100);
		//Reset to take effect
		tmp = ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x0);
		ftgmac100_write_phy_register_sleep(ioaddr, ids->phyAddr, 0x0, (tmp | 0x8000));
		do {
			msleep(20);
			tmp = (ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x0) & 0x8000);
		} while (tmp);
#if 0
		//Wait for auto negotiation complete
		for (i = 0; i < 10; i++) {
			msleep(10);
			tmp = ftgmac100_read_phy_register(ioaddr, ids->phyAddr, 0x1);
			if (tmp & (1u<<5))
				break;
		}
#endif
		break;
	case PHY_SPEED_1G:
		PRINTK("Unsupport yet\n");
		break;


	}

	printk("FORCE PHY rate\n");
	printk("0(0x%08x), 1(0x%08x), 4(0x%08x), 5(0x%08x), 9(0x%08x), 17(0x%08x)\n",
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x0),
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x1),
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x4),
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x5),
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x9),
	       ftgmac100_read_phy_register_sleep(ioaddr, ids->phyAddr, 0x11));

	return count;
}
DEVICE_ATTR(link_mode, (S_IRUGO | S_IWUSR), show_link_mode, store_link_mode);


static ssize_t show_link_state(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct net_device *dev = (struct net_device *)pdev->driver_data;

	return sprintf(buf, "%s", netif_carrier_ok(dev)?("on"):("off"));
}
DEVICE_ATTR(link_state, (S_IRUGO), show_link_state, NULL);


/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
static word _ftgmac100_read_phy_register(unsigned int ioaddr, byte phyaddr, byte phyreg)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return 0xffff;

	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_READ_bit;

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, 0);

	tmp = (inl(ioaddr + PHYDATA_REG)>>16);

	return tmp;
}


/*------------------------------------------------------------
 . Writes a register to the MII Management serial interface
 .-------------------------------------------------------------*/
static void _ftgmac100_write_phy_register(unsigned int ioaddr,
	byte phyaddr, byte phyreg, word phydata)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return;

	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_WRITE_bit;

	outl( phydata, ioaddr + PHYDATA_REG );

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, 0);
}


static ssize_t show_phy_reg(struct device *pdev, struct device_attribute *attr, char *buf)
{
	struct net_device *dev = (struct net_device *)pdev->driver_data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	struct AstMacHwConfig* ids = &lp->ids;
	unsigned long ioaddr = dev->base_addr;
	unsigned int i;

	if (PHYID_RTL8211F_VER <= (lp->ids.miiPhyId & PHYID_VENDOR_MODEL_VER_MASK)) {
		for (i = 0; i <= 15; i++) {
			printk("Reg0x%02X(%d): %04X\n",
				i, i, _ftgmac100_read_phy_register(ioaddr, ids->phyAddr, i));
		}
		if (!lp->ids.is_rtl8211_fiber) {
			i = 18;
			printk("Reg0x%02X(%d): %04X\n",
				i, i, _ftgmac100_read_phy_register(ioaddr, ids->phyAddr, i));

			printk("page: 0xa43\n");
			ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa43); /* change to page 0xa43*/
			for (i = 24; i <= 26; i++) {
				printk("Reg0x%02X(%d): %04X\n",
					i, i, _ftgmac100_read_phy_register(ioaddr, ids->phyAddr, i));
			}
			i = 29;
			printk("Reg0x%02X(%d): %04X\n",
				i, i, _ftgmac100_read_phy_register(ioaddr, ids->phyAddr, i));
			ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa42); /* change back to default page */

		}
	} else {
		for (i = 0; i < 0x20; i++)
			printk("Reg%02X: %04X\n", i, _ftgmac100_read_phy_register(ioaddr, ids->phyAddr, i));
	}

	return 0;
}

static ssize_t store_phy_reg(struct device *pdev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c, index, value;
	struct net_device *dev = (struct net_device *)pdev->driver_data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	c = sscanf(buf, "%x %x", &index, &value);
	if (c != 0)
	{
		if (c == 2)
		{
			printk("Write %04X to Reg%02X\n", value, index);
			_ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, index, value);
		}
		else if (c == 1) {
			printk("Reg%02X = %04X\n", index, _ftgmac100_read_phy_register (ioaddr, lp->ids.phyAddr, index));
		}
	}
	else
	{
		printk("Usage:\nIndex (Value)\n");
	}
	return count;
}
DEVICE_ATTR(phy_reg, (S_IRUGO | S_IWUSR), show_phy_reg, store_phy_reg);

static unsigned int random_size = 512;
#define SCU_SET_REG_32(addr, value)    ((*(volatile u32 *)(IO_ADDRESS(addr))) = cpu_to_le32(value))
#define SCU_GET_REG_32(addr)               (le32_to_cpu(*(volatile u32 *)(IO_ADDRESS(addr))))

#if (CONFIG_AST1500_SOC_VER == 1)
static inline unsigned int _rng0(void)
{
	volatile unsigned int tmp;

	// Generate type 0000
	SCU_SET_REG_32(0x1e6e2010, 0x1);
	wmb();
	SCU_SET_REG_32(0x1e6e2010, 0x3);
	wmb();
	do
	{	//Wait for BIT6 as 1. (stable)
		tmp = SCU_GET_REG_32(0x1e6e2010);
		mb();
	} while (!(tmp & (1<<6)));
	tmp =  SCU_GET_REG_32(0x1e6e2014);
	return (tmp);
}

static inline unsigned int _rng1(void)
{
	volatile unsigned int tmp;

	// Generate type 0001
	SCU_SET_REG_32(0x1e6e2010, 0x5);
	wmb();
	SCU_SET_REG_32(0x1e6e2010, 0x7);
	wmb();
	do
	{	//Wait for BIT6 as 1. (stable)
		tmp = SCU_GET_REG_32(0x1e6e2010);
		mb();
	} while (!(tmp & (1<<6)));
	tmp =  SCU_GET_REG_32(0x1e6e2014);
/*
	{
		static unsigned int odd = 0, even = 0;
		if (tmp & 1)
			odd++;
		else
			even++;
		printk("odd=%d, even=%d\n", odd, even);
	}
*/
	return (tmp);
}

static inline unsigned int _rng(void)
{
#if 0
	unsigned int n = (_rng0() ^ _rng1());
	{
		static unsigned int odd = 0, even = 0;
		if (n & 1)
			odd++;
		else
			even++;
		printk("odd=%d, even=%d\n", odd, even);
	}
	return n;
#else
	return (_rng0() ^ _rng1());
#endif
}


#if HW_RANDOM

#if (CONFIG_AST1500_SOC_VER == 1)
static unsigned int gen_32bits_random(void)
{
	unsigned int i = 0, cnt;
	volatile unsigned int tmp;
	unsigned char buf[sizeof(unsigned int)];

	SCU_SET_REG_32(0x1e6e2010, 0);
	wmb();
	SCU_SET_REG_32(0x1e6e2010, 0x16);
	wmb();
	do
	{	//Wait for SCU14 == 0
		tmp = SCU_GET_REG_32(0x1e6e2014);
		mb();
	} while (tmp);

	for (cnt = 0; cnt < sizeof(unsigned int); cnt++) {
		unsigned char d0, d1, d2, d3, d4, d5, d6, d7;

		d0 = ((unsigned char)(_rng() & 0x00000001)) << 0;
		d1 = ((unsigned char)(_rng() & 0x00000001)) << 1;
		d2 = ((unsigned char)(_rng() & 0x00000001)) << 2;
		d3 = ((unsigned char)(_rng() & 0x00000001)) << 3;
		d4 = ((unsigned char)(_rng() & 0x00000001)) << 4;
		d5 = ((unsigned char)(_rng() & 0x00000001)) << 5;
		d6 = ((unsigned char)(_rng() & 0x00000001)) << 6;
		d7 = ((unsigned char)(_rng() & 0x00000001)) << 7;

		buf[i] = d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7;
		//printk("%d,\n", buf[i]);
		i++;
		yield();
	}

	return (*(unsigned int *)(buf));
}
#else
static unsigned int gen_32bits_random(void)
{
	return SCU_GET_REG_32(0x1e6e2078);
}
#endif

void add_randomness(int irq, unsigned int cnt)
{
	while (cnt--) {
		add_32bits_randomness(irq, gen_32bits_random());
	}
}

#endif //#if HW_RANDOM



static ssize_t show_random(struct device *pdev, struct device_attribute *attr, char *buf)
{
	unsigned int i = 0, cnt;
	volatile unsigned int tmp;

	SCU_SET_REG_32(0x1e6e2010, 0);
	wmb();
	SCU_SET_REG_32(0x1e6e2010, 0x16);
	wmb();
	do
	{	//Wait for SCU14 == 0
		tmp = SCU_GET_REG_32(0x1e6e2014);
		mb();
	} while (tmp);

	for (cnt = 0; cnt < random_size; cnt++) {
		unsigned char d0, d1, d2, d3, d4, d5, d6, d7;

		d0 = ((unsigned char)(_rng() & 0x00000001)) << 0;
		d1 = ((unsigned char)(_rng() & 0x00000001)) << 1;
		d2 = ((unsigned char)(_rng() & 0x00000001)) << 2;
		d3 = ((unsigned char)(_rng() & 0x00000001)) << 3;
		d4 = ((unsigned char)(_rng() & 0x00000001)) << 4;
		d5 = ((unsigned char)(_rng() & 0x00000001)) << 5;
		d6 = ((unsigned char)(_rng() & 0x00000001)) << 6;
		d7 = ((unsigned char)(_rng() & 0x00000001)) << 7;

		buf[i] = d0 | d1 | d2 | d3 | d4 | d5 | d6 | d7;
		//printk("%d,\n", buf[i]);
		i++;
		yield();
	}

	return i;
}
#else //#if (CONFIG_AST1500_SOC_VER == 1)
static ssize_t show_random(struct device *pdev, struct device_attribute *attr, char *buf)
{
	unsigned int i, m;

	for (i = 0; i < (random_size >> 2); i++) {
		*((unsigned int *)buf) = SCU_GET_REG_32(0x1e6e2078);
		buf += 4;
		udelay(2);
	}
	m = random_size & 0x3;
	while (m) {
		*buf = (char)(SCU_GET_REG_32(0x1e6e2078) & 0x000000FF);
		udelay(2);
		buf++;
	}
	return random_size;
}
#endif //#if (CONFIG_AST1500_SOC_VER == 1)

static ssize_t store_random(struct device *pdev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	sscanf(buf, "%d", &random_size);
	return count;
}
DEVICE_ATTR(random, (S_IRUGO | S_IWUSR), show_random, store_random);

#define REALTEK_SWITCH_EEPROM 1
#if REALTEK_SWITCH_EEPROM
#include <asm/arch/drivers/I2C.h>

#define MAX_EEPROM_SIZE 512
#define I2C_RTK_EEPROM_BUS_NUM 4
#define I2C_RTK_EEPROM_BUS_SPD 40000
#define I2C_RTK_EEPROM_ADDR 0xa0

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

int ReadEeprom(unsigned char *tRom)
{
	int i;
	unsigned char bTemp;
	unsigned int ulDeviceSel = I2C_RTK_EEPROM_BUS_NUM;
	unsigned int addr = I2C_RTK_EEPROM_ADDR;

	I2CInit(I2C_RTK_EEPROM_BUS_NUM, I2C_RTK_EEPROM_BUS_SPD);
	/* The EEPROM is on board and should be already ready. */
	if (WaitI2CRdy(ulDeviceSel, addr, 500))
	{
		printk("I2C is not ready\n");
		return -ENODEV;
	}

	for (i = 0; i < MAX_EEPROM_SIZE; i++)
	{
		if (GetI2CReg(ulDeviceSel, ((i>0xFF)?(addr|0x2):(addr)), i, &bTemp))
		{
			printk("GetI2CReg Fail\n");
			return -ENODEV;
		}

		tRom[i] = bTemp;
	}
	return 0;
}

int WriteEeprom(unsigned char *pRom)
{
	int i;
	unsigned int ulDeviceSel = I2C_RTK_EEPROM_BUS_NUM;
	unsigned int addr = I2C_RTK_EEPROM_ADDR;
	unsigned char tRom[MAX_EEPROM_SIZE];

	I2CInit(I2C_RTK_EEPROM_BUS_NUM, I2C_RTK_EEPROM_BUS_SPD);
	/* The EEPROM is on board and should be already ready. */
	if (WaitI2CRdy(ulDeviceSel, addr, 500))
	{
		printk("I2C is not ready\n");
		goto fail;
	}

	for (i = 0; i < MAX_EEPROM_SIZE; i++) {
		/* Wait I2C ready before going further */
		if (SetI2CReg(ulDeviceSel, ((i>0xFF)?(addr|0x2):(addr)), i, pRom[i]))
			goto fail;
		//printk("echo %X %X > io_value\n", i, pRom[i]);
	}
	/* Wait I2C ready before going further. */

	/*
	** Somtimes we will met physical problems causes write eeprom incorrectly.
	** To resolve this issue, we double check again.
	*/
	if (ReadEeprom(tRom)) {
		printk("Fail to read eeprom again\n");
		goto fail;
	}

	if (memcmp(pRom, tRom, MAX_EEPROM_SIZE)) {
		printk("Failed to write EEPROM?!\n");
		udump(tRom, MAX_EEPROM_SIZE);
		printk("Should be:\n");
		udump(pRom, MAX_EEPROM_SIZE);
		goto fail;
	}
	return 0;

fail:
	return -ENODEV;
}


static ssize_t show_eeprom_content(struct device *pdev, struct device_attribute *attr, char *buf)
{
	unsigned char tRom[MAX_EEPROM_SIZE];

	if (ReadEeprom(tRom)) {
		printk("Fail to read eeprom again\n");
		goto fail;
	}
	udump(tRom, MAX_EEPROM_SIZE);
fail:
	return 0;
}
static ssize_t store_eeprom_content(struct device *pdev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned char rom[MAX_EEPROM_SIZE];
	unsigned int i = 0;
	unsigned char *word;
	unsigned char *str;
	char *sep=" |,\n\r";

	/*
	** Bruce110922. This is the code to write Realtek Ethernet Switch's EEPROM.
	** The EEPROM is 512Bytes which contains 2 blocks of 256 bytes data. So, the addressing
	** is a little bit different.
	** ToDo. Should avoid two I2C master by reset Realtek switch.
	*/
	memset(rom, 0, MAX_EEPROM_SIZE);
	str = (unsigned char*)buf;
	while (i < MAX_EEPROM_SIZE) {
		word = strsep((char**)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		rom[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}

	WriteEeprom(rom);

	return count;
}
static DEVICE_ATTR(eeprom_content, (S_IRUGO | S_IWUSR), show_eeprom_content, store_eeprom_content);
#endif //#if REALTEK_SWITCH_EEPROM

static struct attribute *dev_attrs[] = {
	&dev_attr_net_debug.attr,
	&dev_attr_link_mode.attr,
	&dev_attr_link_state.attr,
	&dev_attr_phy_reg.attr,
	&dev_attr_random.attr,
#if REALTEK_SWITCH_EEPROM
	&dev_attr_eeprom_content.attr,
#endif
	NULL,
};

struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) "ftgmac",
	.id = -1,
	.dev = {
		.release = the_pdev_release,
	},
};

#endif

/*----------------------------------------------------------------------
 . Function: ftgmac100_probe( struct net_device *dev )
 .
 . Purpose:
 .	Tests to see if a given ioaddr points to an ftgmac100 chip.
 .	Returns a 0 on success
 .
 .
 .---------------------------------------------------------------------
 */
/*---------------------------------------------------------------
 . Here I do typical initialization tasks.
 .
 . o  Initialize the structure if needed
 . o  print out my vanity message if not done so already
 . o  print out what type of hardware is detected
 . o  print out the ethernet address
 . o  find the IRQ
 . o  set up my private data
 . o  configure the dev structure with my subroutines
 . o  actually GRAB the irq.
 . o  GRAB the region
 .-----------------------------------------------------------------*/

static int __init ftgmac100_probe(struct net_device *dev )
{
	int retval;
	static unsigned version_printed = 0;
	struct ftgmac100_local *lp = 0;

	PRINTK2("%s:ftgmac100_probe\n", dev->name);

	/// waiting to do: probe
	if (version_printed++ == 0)
	{
		DO_PRINT("%s", version);
	}

	/* now, print out the card info, in a short format.. */
	DO_PRINT("%s: at %#3lx IRQ:%d noWait:%d, MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		 dev->name, dev->base_addr, dev->irq, dev->dma,
		 dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		 dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	if (IRQ_MAC1 == dev->irq)
		ast_scu.scu_op(SCUOP_MAC_INIT, (void *)1);
	else
		ast_scu.scu_op(SCUOP_MAC_INIT, (void *)2);

	/* Grab the IRQ next.  Beyond this, we will free the IRQ. */
	retval = request_irq(dev->irq, &ftgmac100_interrupt,
			     SA_INTERRUPT, dev->name, dev);
	if (retval)
	{
		DO_PRINT("%s: unable to get IRQ %d (retval=%d).\n",
			 dev->name, dev->irq, retval);
		return retval;
	}
	// IRQ properties are handled by kernel driver in ASPEED SDK.
	IRQ_SET_HIGH_LEVEL (dev->irq);
	IRQ_SET_LEVEL_TRIGGER (dev->irq);
#if HW_RANDOM
	rand_initialize_irq(dev->irq);
#endif
	/// --------------------------------------------------------------------
	///		initialize ftgmac100_local
	/// --------------------------------------------------------------------
	memset(dev->priv, 0, sizeof(struct ftgmac100_local));
	lp = (struct ftgmac100_local *)dev->priv;
	spin_lock_init(&lp->lock);

#if 0
	lp->maccr_val = (CRC_APD_bit | RXMAC_EN_bit | TXMAC_EN_bit  | RXDMA_EN_bit
			 | TXDMA_EN_bit | CRC_CHK_bit | RX_BROADPKT_bit | SPEED_100_bit | FULLDUP_bit | JUMBO_LF_bit);
#else
	lp->maccr_val = (CRC_APD_bit | RXMAC_EN_bit | TXMAC_EN_bit  | RXDMA_EN_bit
			 | TXDMA_EN_bit | PHY_INT_LOW_ACTIVE_bit | RX_BROADPKT_bit | SPEED_100_bit | FULLDUP_bit);
#endif
#if TEST_USE_HPTX || HPTX_SUPPORT
	lp->maccr_val |= HPTXR_EN_bit;
#endif

	lp->tx_buff_size = lp->rx_buff_size = 1536;
	lp->rx_buff_num = MAX_RX_BUF_NUM;

	/* now, reset the chip, and put it into a known state */
	ftgmac100_reset( dev );

	// Quit with an error if this MAC cannot carry traffic.
	if (lp->ids.miiPhyId < 2) {
	    retval = -EBADSLT;	// Or -ENODEV...?
	    goto err_out;
	}

	ftgmac100_ringbuf_alloc(dev);//(lp); //disable for OTG //fred

	/* Fill in the fields of the device structure with ethernet values. */
	ether_setup(dev);

	dev->open	        	= ftgmac100_open;
	dev->stop	        	= ftgmac100_close;
	dev->hard_start_xmit    = ftgmac100_wait_to_send_packet;
	dev->tx_timeout			= ftgmac100_timeout;
	dev->get_stats			= ftgmac100_query_statistics;
#ifdef NEW_JUMBO_FRAME
	dev->change_mtu			= ftgmac100_change_mtu;
#endif
#ifdef	HAVE_MULTICAST
	dev->set_multicast_list = &ftgmac100_set_multicast_list;
#endif
#ifdef CONFIG_FTGMAC100_NAPI
	dev->poll = ftgmac100_poll;
	dev->weight = FTGMAC100_NAPI_WEIGHT;
#endif

#ifdef FREE_TX_IN_WQ
	lp->tx_task = create_singlethread_workqueue("gmac_tx_free");
	BUG_ON(lp->tx_task == NULL);
	INIT_WORK(&lp->tx_free_work, (void (*)(void *))(ftgmac100_free_tx), (void *)dev);
	INIT_WORK(&lp->link_chg_work, (void (*)(void *))(ftgmac100_link_chg), (void *)dev);
#if defined(PHY_FIBER_STATUS_IN_WQ)
	INIT_WORK(&lp->phy_change_work, (void (*)(void *))(ast_phy_change), (void *)dev);
#endif
#else
	tasklet_init(&lp->tx_task, ftgmac100_free_tx, (unsigned long)dev);
#endif
	//Fred add for hw checksum offload
	//dev->features|=NETIF_F_HW_CSUM;

	if ((proc_ftgmac100 = create_proc_entry( dev->name, 0, 0 )))
	{
		proc_ftgmac100->read_proc = ftgmac100_read_proc;
		proc_ftgmac100->data = dev;
		proc_ftgmac100->owner = THIS_MODULE;
	}
#if NEW_AHU
	ahu_init(dev);
#endif
	return 0;

err_out:
	ftgmac100_ringbuf_free(dev);
	free_irq( dev->irq, dev );
	return retval;
}


#if FTMAC100_DEBUG > 2
static void print_packet( byte * buf, int length )
{
#if 1
#if FTMAC100_DEBUG > 3
	int i;
	int remainder;
	int lines;
#endif


#if FTMAC100_DEBUG > 3
	lines = length / 16;
	remainder = length % 16;

	for ( i = 0; i < lines ; i ++ ) {
		int cur;

		for ( cur = 0; cur < 8; cur ++ ) {
			byte a, b;

			a = *(buf ++ );
			b = *(buf ++ );
			DO_PRINT("%02x%02x ", a, b );
		}
		DO_PRINT("\n");
	}
	for ( i = 0; i < remainder/2 ; i++ ) {
		byte a, b;

		a = *(buf ++ );
		b = *(buf ++ );
		DO_PRINT("%02x%02x ", a, b );
	}
	DO_PRINT("\n");
#endif
#endif
}
#endif


/*
 * Open and Initialize the board
 *
 * Set up everything, reset the card, etc ..
 *
 */
static int ftgmac100_open(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;

	DO_PRINT("%s:ftgmac100_open\n", dev->name);

	/* reset the hardware */
	ftgmac100_reset( dev );
	/*
	//Bruce111117. Under AST1500 initial case, skbs are already allocated in
	//ftgmac100_probe()::ftgmac100_ringbuf_alloc(). We should free all skbs
	//before calling ftgmac100_enable() because ftgmac100_enable() assumes
	//all Rx descriptors should be reset.
	*/
	ftgmac100_force_free_skbs(dev);

	if (((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_BROADCOM) ||
	    ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8201EL)) {
	    init_timer(&lp->timer);
	    lp->timer.data = (unsigned long)dev;
	    lp->timer.function = aspeed_mac_timer;
	    lp->timer.expires = jiffies + 1 * HZ;
	    add_timer (&lp->timer);
	}

	/* Configure the PHY */
	ftgmac100_phy_configure(dev);

	ftgmac100_enable( dev );

	if ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211) {
		unsigned long flags;
		u32 has_link;

		has_link = ast_phy_has_link(dev, 1);
		if (has_link) {
			printk("Ethernet link up\n");
		} else {
			printk("reset Ethernet PHY\n");
			local_irq_save(flags);
			ftgmac100_phy_configure(dev);
			local_irq_restore(flags);
		}

#if defined(PHY_FIBER_STATUS_IN_WQ)
		schedule_delayed_work(&lp->phy_change_work, msecs_to_jiffies(PHY_FIBER_STATUS_PERIOD_MS));
#else
		if (lp->ids.is_rtl8211_fiber) {
			init_timer(&lp->timer);
			lp->timer.data = (unsigned long)dev;
			lp->timer.function = aspeed_mac_timer;
			lp->timer.expires = jiffies + 1 * HZ;
			add_timer (&lp->timer);
		}
#endif
	}

	netif_start_queue(dev);

	return 0;
}


/*--------------------------------------------------------
 . Called by the kernel to send a packet out into the void
 . of the net.  This routine is largely based on
 . skeleton.c, from Becker.
 .--------------------------------------------------------
*/
static void ftgmac100_timeout (struct net_device *dev)
{
	/* If we get here, some higher level has decided we are broken.
	There should really be a "kick me" function call instead. */
	DO_PRINT(KERN_WARNING "%s: transmit timed out? (jiffies=%ld)\n",
		 dev->name, jiffies);
	/* "kick" the adaptor */
	ftgmac100_reset( dev );
	ftgmac100_force_free_skbs(dev);

	/* Reconfigure the PHY */
	ftgmac100_phy_configure(dev);

	ftgmac100_enable( dev );
	//netif_wake_queue(dev); //done in ftgmac100_enable()
	dev->trans_start = jiffies;
}

static void ftgmac100_link_chg (unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	ftgmac100_reset(dev);
	ftgmac100_force_free_skbs(dev);
	ftgmac100_enable(dev);
}

/*--------------------------------------------------------------------
 .
 . This is the main routine of the driver, to handle the net_device when
 . it needs some attention.
 .
 . So:
 .   first, save state of the chipset
 .   branch off into routines to handle each case, and acknowledge
 .	    each to the interrupt register
 .   and finally restore state.
 .
 ---------------------------------------------------------------------*/
static irqreturn_t ftgmac100_interrupt(int irq, void * dev_id,  struct pt_regs * regs)
{
	struct net_device *dev 	= dev_id;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr 	= dev->base_addr;
	int		timeout;
	unsigned int tmp;
	unsigned int	mask;			// interrupt mask
	unsigned int	status;			// interrupt status

//	PRINTK3("%s: ftgmac100 interrupt started \n", dev->name);

	if (dev == NULL) {
		DO_PRINT(KERN_WARNING "%s: irq %d for unknown device.\n",	dev->name, irq);
		return IRQ_HANDLED;
	}

	/* read the interrupt status register */
	mask = inl( ioaddr + IER_REG );

	/* set a timeout value, so I don't stay here forever */

	for (timeout=1; timeout>0; --timeout)
	{
		/* read the status flag, and mask it */
		status = inl( ioaddr + ISR_REG ) & mask;

		outl(status, ioaddr + ISR_REG ); //Richard, write to clear

		if (!status )
		{
			break;
		}

		if (status & PHYSTS_CHG_bit) {
		    DO_PRINT("PHYSTS_CHG \n");
		    // Is this interrupt for changes of the PHYLINK pin?
		    // Note: PHYLINK is optional; not all boards connect it.
		    if (((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_MARVELL)
			|| ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211))
		    {
				if (PHYID_RTL8211F_VER <= (lp->ids.miiPhyId & PHYID_VENDOR_MODEL_VER_MASK)) {
					ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa43); /* change to page 0xa43*/
					tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x1D); /* read to clear interrupt status */
					if (tmp & (0x1 << 4))
						tmp = PHY_LINK_CHG_bit;
					ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa42); /* change back to default page */
				} else {
					tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x13);
					PRINTK("%s: PHY interrupt status, read_phy_reg(0x13) = 0x%04x\n",
						dev->name, tmp);
					tmp &= (PHY_SPEED_CHG_bit | PHY_DUPLEX_CHG_bit | PHY_LINK_CHG_bit);
				}

				if (tmp & PHY_LINK_CHG_bit) {
					u32 has_link;

					has_link = ast_phy_has_link(dev, 1);

					if (has_link && !netif_carrier_ok(dev)) {
						/* ftgmac100_enable() will do it.
						PRINTK("Ethernet Link ON\n");
						netif_carrier_on(dev);
						netif_wake_queue(dev);
						ast_notify_user("e_eth_link_on");
						*/
					} else if (!(has_link) && netif_carrier_ok(dev)) {
						PRINTK("Ethernet Link OFF\n");
						netif_stop_queue(dev);
						netif_carrier_off(dev);
						ast_notify_user("e_eth_link_off");
						/*
						 * link up => down
						 * ast_phy_reset_init() set phy_reset_timeout_jiffies = 0
						 * to trigger phy reset after PHY_RESET_INTERVAL
						 */
						ast_phy_reset_init();
					}
				}

		    }
		    else if ((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_BROADCOM)
		    {
				tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x1a);
				PRINTK("%s: PHY interrupt status, read_phy_reg(0x1a) = 0x%04x\n",
				       dev->name, tmp);
				// Bits [3:1] are {duplex, speed, link} change interrupts.
				tmp &= 0x000e;
				//steven:to do
		    }
		    else tmp = 0;

		    if (tmp) {
#ifdef FREE_TX_IN_WQ
				stop_mac(ioaddr, lp);
				queue_work(lp->tx_task, &lp->link_chg_work);
				return IRQ_HANDLED;
#else
				BUG(); //ToDo
#endif
		    }
		}

		if (status & TPKT_LOST_bit)
		{
			printk("XPKT_LOST\n");
		}

//		PRINTK3(KERN_WARNING "%s: Handling interrupt status %x \n",	dev->name, status);

		if ( status & RPKT2B_bit )
		{
#ifdef CONFIG_FTGMAC100_NAPI
			if (netif_carrier_ok(dev)) {
				tmp = inl (ioaddr + IER_REG);
				outl (tmp & ~(RPKT2B_bit), ioaddr + IER_REG);
				if (likely(netif_rx_schedule_prep(dev)))
					__netif_rx_schedule(dev);
			} else {
				PRINTK("Calling NAPI during carrier off!!\n");
			}
#else
			tmp = ftgmac100_rcv(dev);
#endif
		}
#if 0
		else if (status & RXBUF_UNAVA_bit)
		{
			outl( mask & ~RXBUF_UNAVA_bit, ioaddr + IER_REG);
			trans_busy = 1;
		/*
			rcv_tq.sync = 0;
			rcv_tq.routine = ftgmac100_rcv;
			rcv_tq.data = dev;
			queue_task(&rcv_tq, &tq_timer);
		*/

		} else if (status & AHB_ERR_bit)
		{
			DO_PRINT("AHB ERR \n");
		}
#else
		else
		{
			if (status & RXBUF_UNAVA_bit)
			{
				/* limit this message to avoid worse situation when CPU is busy */
				if (printk_ratelimit())
					printk("E_B\n");
				outl( mask & ~RXBUF_UNAVA_bit, ioaddr + IER_REG);
				trans_busy = 1;
			/*
				rcv_tq.sync = 0;
				rcv_tq.routine = ftgmac100_rcv;
				rcv_tq.data = dev;
				queue_task(&rcv_tq, &tq_timer);
			*/

			}
			if (status & RPKT_LOST_bit)
			{
				/* limit this message to avoid worse situation when CPU is busy */
				if (printk_ratelimit())
					DO_PRINT("E_F\n");
			}
			if (status & AHB_ERR_bit)
			{
				DO_PRINT("AHB ERR \n");
			}
		}
#endif
	}

#if HW_RANDOM
	add_interrupt_randomness(dev->irq);
#endif
//	PRINTK3("%s: Interrupt done\n", dev->name);
	return IRQ_HANDLED;
}

#ifdef CONFIG_FTGMAC100_NAPI
static int ftgmac100_poll(struct net_device *dev, int *budget)
{
	unsigned int work_done, work_to_do = min(*budget, dev->quota);
	unsigned long ioaddr = dev->base_addr;
	volatile unsigned long tmp;

	if (!netif_carrier_ok(dev))
		PRINTK("!!!!!WTH!!!!!!\n");

	if (netif_queue_stopped(dev)) {
		PRINTK("net dev Stopped!\n");
		netif_rx_complete(dev);
		return 0; //done
	}

#if NEW_RX_PROC
	work_done = ftgmac100_rcv(dev, work_to_do);
#else
	work_done = ftgmac100_rcv(dev);
#endif

	*budget -= work_done;
	dev->quota -= work_done;

#if NEW_RX_PROC
	if (work_done == work_to_do)
		return 1;
	else
	{
		netif_rx_complete(dev);
		//to do: make sure MAC is enabled
		tmp = inl( ioaddr + IER_REG );
		outl((tmp | RPKT2B_bit), ioaddr + IER_REG);
		return 0;
	}
#else
	if (work_done == 0) {
		netif_rx_complete(dev);
		tmp = inl( ioaddr + IER_REG );
		outl((tmp | RPKT2B_bit), ioaddr + IER_REG);
	}

	return (work_done >= work_to_do);
#endif
}
#endif
#ifdef NEW_JUMBO_FRAME
static int ftgmac100_change_mtu (struct net_device *dev, int new_mtu)
{
	if (new_mtu > (TX_BUF_SIZE - 16)) {
		DO_PRINT("err: unsupported MTU size %d\n", new_mtu);
		return 1;
	}
	else {
		if (dev->mtu != new_mtu)
		{
			struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
			DO_PRINT("set MTU size to %d\n", new_mtu);
			//netif_stop_queue(dev);
			netif_tx_disable(dev);
			stop_mac(dev->base_addr, lp);
			netif_carrier_off(dev);
			cancel_delayed_work(&lp->tx_free_work);
			flush_workqueue(lp->tx_task);

			//netif_poll_disable(dev);
			ftgmac100_reset(dev);
			ftgmac100_force_free_skbs(dev);
			dev->mtu = new_mtu;
			if (new_mtu == 8000)
			{
				lp->tx_buff_size = lp->rx_buff_size = 8192;
				lp->rx_buff_num = (MAX_RX_BUF_NUM >> 1);
				//lp->rx_buff_num = MAX_RX_BUF_NUM;
#if 1
				lp->maccr_val |= JUMBO_LF_bit;
#endif
			}
			else
			{
				lp->tx_buff_size = lp->rx_buff_size = 1536;
				lp->rx_buff_num = MAX_RX_BUF_NUM;
#if 1
				lp->maccr_val &= ~JUMBO_LF_bit;
#endif
			}
			ftgmac100_enable(dev);
			//netif_poll_enable(dev);
		}
	}
	return 0;
}
#endif
/* This function assumes that HW and NAPI are both stopped. */
static void ftgmac100_force_free_rx (struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	volatile unsigned long flags = 0;
	volatile int entry;
	struct sk_buff *skb;

	spin_lock_irqsave(&lp->lock,flags);

	/* Force free skbs. */
	for (entry = 0; entry < lp->rx_buff_num; entry++)
	{
		skb = lp->rx_skbuff[entry];
		if (skb)
			dev_kfree_skb_any (skb);
		lp->rx_skbuff[entry] = NULL;
	}

	spin_unlock_irqrestore(&lp->lock, flags);
}


static void ftgmac100_force_free_tx (struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	volatile unsigned long flags = 0;
	volatile int entry;
	struct sk_buff *skb;

	spin_lock_irqsave(&lp->lock,flags);

	entry = (lp->old_tx) & (TXDES_NUM - 1);
	while ((lp->tx_descs[entry].TXDMA_OWN == TX_OWNBY_SOFTWARE) && (lp->tx_skbuff[entry] != NULL)) {
		skb = lp->tx_skbuff[entry];
			ast_dev_kfree_skb (skb);
		lp->tx_skbuff[entry] = NULL;
		entry = (entry + 1) & (TXDES_NUM - 1);
		lp->tx_free++;
	}
	/* Force free skbs which are not owned by sw. */
	while ((lp->tx_free < TXDES_NUM) && (lp->tx_skbuff[entry] != NULL)) {
		skb = lp->tx_skbuff[entry];
			ast_dev_kfree_skb (skb);
		lp->tx_skbuff[entry] = NULL;
		entry = (entry + 1) & (TXDES_NUM - 1);
		lp->tx_free++;
	}

	lp->old_tx = entry;

#if HPTX_SUPPORT
	/* Free High Priority Tx Q. */
	entry = (lp->old_hptx) & (TXDES_NUM - 1);
	while ((lp->hptx_descs[entry].TXDMA_OWN == TX_OWNBY_SOFTWARE) && (lp->hptx_skbuff[entry] != NULL)) {
		skb = lp->hptx_skbuff[entry];
		ast_dev_kfree_skb (skb);
		lp->hptx_skbuff[entry] = NULL;
		entry = (entry + 1) & (TXDES_NUM - 1);
		lp->hptx_free++;
	}
	/* Force free skbs which are not owned by sw. */
	while ((lp->hptx_free < TXDES_NUM) && (lp->hptx_skbuff[entry] != NULL)) {
		skb = lp->hptx_skbuff[entry];
		ast_dev_kfree_skb (skb);
		lp->hptx_skbuff[entry] = NULL;
		entry = (entry + 1) & (TXDES_NUM - 1);
		lp->hptx_free++;
	}

	lp->old_hptx = entry;
#endif
	spin_unlock_irqrestore(&lp->lock, flags);
}


static void ftgmac100_force_free_skbs (struct net_device *dev)
{
	ftgmac100_force_free_tx(dev);
	ftgmac100_force_free_rx(dev);
}

static void ftgmac100_free_tx (unsigned long data)
{
	struct net_device *dev = (struct net_device *)data;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	volatile unsigned long flags = 0;
	int rdr, wrtr;

	spin_lock_irqsave(&lp->lock,flags);

	//printk("c(%i,%i)", lp->old_tx, lp->tx_idx);

	rdr = lp->old_tx;
	wrtr = lp->tx_idx;
	while (rdr != wrtr) {
		struct sk_buff *skb;

		if (lp->tx_descs[rdr].TXDMA_OWN != TX_OWNBY_SOFTWARE)
			break;

		if (lp->tx_descs[rdr].CRC_ERR)
			printk("T_CRC\n");

		skb = lp->tx_skbuff[rdr];
		ast_dev_kfree_skb (skb);
		lp->tx_skbuff[rdr] = NULL;
		rdr = (rdr + 1) & (TXDES_NUM - 1);
		lp->tx_free++;
	}
	lp->old_tx = rdr;

#if HPTX_SUPPORT
	rdr = lp->old_hptx;
	wrtr = lp->hptx_idx;
	while (rdr != wrtr) {
		struct sk_buff *skb;

		if (lp->hptx_descs[rdr].TXDMA_OWN != TX_OWNBY_SOFTWARE)
			break;

		if (lp->hptx_descs[rdr].CRC_ERR)
			printk("T_CRC\n");

		skb = lp->hptx_skbuff[rdr];
		ast_dev_kfree_skb (skb);
		lp->hptx_skbuff[rdr] = NULL;
		rdr = (rdr + 1) & (TXDES_NUM - 1);
		lp->hptx_free++;
	}
	lp->old_hptx = rdr;
#endif

	if ((netif_queue_stopped(dev)) && (lp->tx_free > 0) && netif_carrier_ok(dev)) {
		netif_wake_queue (dev);
	}

	if ((lp->tx_free != TXDES_NUM)
#if HPTX_SUPPORT
		|| (lp->hptx_free != TXDES_NUM)
#endif
	    ) {
		//PRINTK("q(%lu)(%lu)\n", TXDES_NUM - lp->tx_free, TXDES_NUM - lp->hptx_free);
		queue_delayed_work(lp->tx_task, &lp->tx_free_work, msecs_to_jiffies(5));
	}

	spin_unlock_irqrestore(&lp->lock, flags);

#if HW_RANDOM
	add_randomness(dev->irq, 1);
#endif

}

/*-------------------------------------------------------------
 .
 . ftgmac100_rcv -  receive a packet from the card
 .
 . There is ( at least ) a packet waiting to be read from
 . chip-memory.
 .
 . o Read the status
 . o If an error, record it
 . o otherwise, read in the packet
 --------------------------------------------------------------
*/
// extern dce_dcache_invalidate_range(unsigned int start, unsigned int end);
#if NEW_RX_PROC
static int ftgmac100_rcv(struct net_device *dev, unsigned int process_limit)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr 	= dev->base_addr;
	unsigned int 	packet_length;
	volatile RX_DESC *cur_desc;
	unsigned int		cur_idx;
	unsigned int		have_frs;
	unsigned int             count = 0;
	unsigned int 	rcv_cnt = 0;
	unsigned int work_done = 0;

	while ((cur_desc = &lp->rx_descs[lp->rx_idx])->RXPKT_RDY==RX_OWNBY_SOFTWARE)
	{
		cur_idx = lp->rx_idx;
		if (lp->rx_skbuff[cur_idx] == NULL)
		{
			printk("lp->rx_skbuff[%d] == NULL\n", cur_idx);
			BUG();
		}
		lp->rx_idx = (lp->rx_idx+1) & (lp->rx_buff_num - 1);
		count++;

		have_frs = 0;
		packet_length = 0;

		do
		{
			if (!cur_desc->FRS)
			{
				printk("not FRS (%d)\n", cur_idx);
				break;
			}
			if (!cur_desc->LRS)
			{
				printk("not LRS (%d)\n", cur_idx);
				break;
			}
			have_frs = 1;
			/* Per Jazoe140416. Other errors are meaningless when FIFO_FULL. */
			if (cur_desc->FIFO_FULL)
			{
				/* limit this message to avoid worse situation when CPU is busy */
				if (printk_ratelimit())
					printk("err: FIFO_FULL\n");
				lp->stats.rx_errors++;			// error frame....
#if NEW_AHU
				ahu_alert_cfg(1);
#endif
				break;
			}
			if (cur_desc->RX_ERR)
			{
				DO_PRINT("err: RX_ERR\n");
				lp->stats.rx_errors++;			// error frame....
				break;
			}
			if (cur_desc->CRC_ERR)
			{
//				DO_PRINT("err: CRC_ERR\n");
//				printk("R_CRC\n");
				lp->stats.rx_errors++;			// error frame....
				break;
			}
			if (cur_desc->FTL)
			{
#if !(PATCH_CODEV3 & PATCH_AST1520A0_BUG14080600_MAC_FTL)
				DO_PRINT("err: FTL\n");
				lp->stats.rx_errors++;			// error frame....
				break;
#endif
			}
			if (cur_desc->RUNT)
			{
				printk("RUNT\n");
				lp->stats.rx_errors++;			// error frame....
				break;
			}
			if (cur_desc->RX_ODD_NB)
			{
				printk("ODD_NB\n");
				lp->stats.rx_errors++;			// error frame....
				break;
			}
			if (DF_support)
			{
				if (cur_desc->DF)
				{
					if (cur_desc->IPCS_FAIL || cur_desc->UDPCS_FAIL || cur_desc->TCPCS_FAIL)
					{
						DO_PRINT("err: CS FAIL (%d) 0x%08X,0x%08X,0x%08X,0x%08X,\n",
							cur_idx,
							((u32*)(cur_desc))[0],
							((u32*)(cur_desc))[1],
							((u32*)(cur_desc))[2],
							((u32*)(cur_desc))[3]);
						break;
					}
				}
			}
			if (cur_desc->MULTICAST)
			{
				lp->stats.multicast++;
			}
#if NCSI_SUPPORT
			if ((lp->NCSI_support == 1) || (lp->INTEL_NCSI_EVA_support == 1)) {
				if (cur_desc->BROADCAST) {
					if (*(unsigned short *)(cur_desc->VIR_RXBUF_BADR + 12) == NCSI_HEADER) {
						printk ("AEN PACKET ARRIVED\n");
						ftgmac100_reset(dev);
						ftgmac100_enable(dev);
						return count;
					}
				}
			}
#endif
			if (cur_desc->VDBC > lp->rx_buff_size)
			{
				printk("invalid VDBC (%d)\n", cur_desc->VDBC);
				BUG();
			}
			packet_length += cur_desc->VDBC;
		}
		while (0);

		if (have_frs == 0)
		{
		//	DO_PRINT("error, loss first\n");
			lp->stats.rx_over_errors++;
		}

		if (packet_length>0)
		{
			struct sk_buff  * skb;

#if FTMAC100_DEBUG
			if (net_debug_flag & AST_DBG_BITRATE) {
				unsigned long duration;

				if (gRxTotalLenEx == 0 && gJiffiesStartEx == 0) {
					gJiffiesStartEx = jiffies;
					printk("Wait %d secs...", gBitrateInterval);
				}
				if (gRxTotalLen == 0) {
					gJiffiesStart = jiffies;
				}
				gRxTotalLen += packet_length;
				duration = jiffies_to_msecs(jiffies - gJiffiesStart);
				if (duration > gBitrateInterval * 1000) {
					unsigned long bitrate;
					char str[64];

					bitrate = (gRxTotalLen /duration)*8;

					duration = jiffies_to_msecs(jiffies - gJiffiesStartEx);
					gRxTotalLenEx += gRxTotalLen/1000;

					printk("Net Rx [%lu]Kbps, avg[%lu]Mbps, peak rx desc=%d\n", bitrate, (gRxTotalLenEx /duration)*8, gRcvCnt);
					sprintf(str, "e_net_rate::%luKbps", bitrate);
					ast_notify_user(str);
					gRxTotalLen = 0;
					gJiffiesStart = 0;
				}
			}
#endif

			if (packet_length < 4)
			{
				printk("invalid packet_length (%d)\n", packet_length);
				BUG();
			}
			packet_length -= 4;

			skb_put (skb = lp->rx_skbuff[cur_idx], packet_length);

// Rx Offload
			if (DF_support) {
				if (cur_desc->DF)
					skb->ip_summed = CHECKSUM_UNNECESSARY;
			}

#if NEW_AHU
			if (cur_desc->FRAMEID_HIT) {
				if (ahu_netif_rx(skb) > 32) {
					//printk("\nf\n");
				}
			} else {
#endif
				//NOTE: eth_type_trans() will remove eth header from skb->data.
				skb->protocol = eth_type_trans(skb, dev );
				ftgmac100_rx_skb(skb);
#if NEW_AHU
			}
#endif
			rcv_cnt++;
			lp->stats.rx_packets++;

			skb = dev_alloc_skb (lp->rx_buff_size);
			if (skb == NULL) {
				printk ("%s: receive_packet: "
					"Unable to re-allocate Rx skbuff.#%d\n",
					dev->name, cur_idx);
				BUG();
			}
			lp->rx_skbuff[cur_idx] = skb;
			skb->dev = dev;
			/* 16 byte align the IP header */
			skb_reserve (skb, 2);
			dmac_inv_range ((unsigned long)skb->data,
					(unsigned long)skb->data + lp->rx_buff_size);
			lp->rx_descs[cur_idx].RXBUF_BADR = virt_to_phys(skb->tail); //Richard
			lp->rx_descs[cur_idx].VIR_RXBUF_BADR = (unsigned long)skb->tail;
		}
		//return to HW
		lp->rx_descs[cur_idx].RXPKT_RDY = RX_OWNBY_FTGMAC100;
		work_done++;

		/*
		** Bruce171013. Concerning this while loop may loop forever under very high bitrate with error FIFO_FULL situation.
		** process_limit take 'error' descriptors into account.
		** So, this polling loop will return not only rcv_cnt meet, but also desc process cnt meet process_limit.
		*/
		if (work_done == process_limit)
			break;
	}

#if FTMAC100_DEBUG
	if (count > gRcvCnt)
		gRcvCnt = count;
#endif

	if (trans_busy == 1)
	{
		trans_busy = 0;
		//printk("set MACCR_REG to %08X (ftgmac100_rcv)\n", lp->maccr_val);
		outl( lp->maccr_val, ioaddr + MACCR_REG );
		// Clear status bit again to avoid false alert.
		outl(RXBUF_UNAVA_bit, ioaddr + ISR_REG);
		outl( inl(ioaddr + IER_REG) | RXBUF_UNAVA_bit, ioaddr + IER_REG);
	}

	return work_done;
}
#else
static int ftgmac100_rcv(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr 	= dev->base_addr;
	int 	packet_length;
	int 	rcv_cnt;
	volatile RX_DESC *cur_desc;
	int		cur_idx;
	int		have_package;
	int		have_frs;
	int	 	start_idx;
        int             count = 0;
        int             packet_full = 0;
	int		data_not_fragment = 1;
	int cs_error;

//	DO_PRINT("%s:ftgmac100_rcv\n", dev->name);
	start_idx = lp->rx_idx;

	for (rcv_cnt=0; rcv_cnt < lp->rx_buff_num; ++rcv_cnt)
	{
		packet_length = 0;
		cur_idx = lp->rx_idx;

		have_package = 0;
		have_frs = 0;
		cs_error = 0;

		for (; (cur_desc = &lp->rx_descs[lp->rx_idx])->RXPKT_RDY==RX_OWNBY_SOFTWARE; )
		{
			have_package = 1;
			lp->rx_idx = (lp->rx_idx+1) & (lp->rx_buff_num - 1);
			count++;
			if (count == lp->rx_buff_num) {
				packet_full = 1;
			}
			if (DF_support) {
                            if (data_not_fragment == 1) {
                                if (!(cur_desc->DF)) {
                                    data_not_fragment = 0;
                                }
                            }
                        }
			if (cur_desc->FRS)
			{
				have_frs = 1;
				if (cur_desc->RX_ERR || cur_desc->CRC_ERR || cur_desc->FTL ||
				    cur_desc->RUNT || cur_desc->RX_ODD_NB
				    // cur_desc->IPCS_FAIL || cur_desc->UDPCS_FAIL || cur_desc->TCPCS_FAIL
				    )
				{
				//	#ifdef not_complete_yet
					if (cur_desc->RX_ERR)
					{
						DO_PRINT("err: RX_ERR\n");
					}
					if (cur_desc->CRC_ERR)
					{
				//		DO_PRINT("err: CRC_ERR\n");
						printk("R_CRC\n");
					}
					if (cur_desc->FTL)
					{
						DO_PRINT("err: FTL\n");
					}
#if 0
					if (cur_desc->RX_ODD_NB)
					{
				//		DO_PRINT("err: RX_ODD_NB\n");
				        }
#else
					if (cur_desc->RUNT)
					{
						printk("RUNT\n");
					}
					if (cur_desc->RX_ODD_NB)
					{
						printk("ODD_NB\n");
					}
#endif
//				        if (cur_desc->IPCS_FAIL || cur_desc->UDPCS_FAIL || cur_desc->TCPCS_FAIL)
//				        {
//				        	DO_PRINT("err: CS FAIL\n");
//				        }
				//	#endif /* end_of_not */
					lp->stats.rx_errors++;			// error frame....
					break;
				}
				if (DF_support) {
					if (cur_desc->DF) {
						if (cur_desc->IPCS_FAIL || cur_desc->UDPCS_FAIL || cur_desc->TCPCS_FAIL)
						{
							DO_PRINT("err: CS FAIL (%d)\n", lp->rx_idx);
#if 0
							lp->stats.rx_errors++;			// error frame....
								break;
#else
							cs_error = 1;
#endif
						}
					}
				}
				if (cur_desc->MULTICAST)
				{
					lp->stats.multicast++;
				}
#if NCSI_SUPPORT
				if ((lp->NCSI_support == 1) || (lp->INTEL_NCSI_EVA_support == 1)) {
					if (cur_desc->BROADCAST) {
						if (*(unsigned short *)(cur_desc->VIR_RXBUF_BADR + 12) == NCSI_HEADER) {
							printk ("AEN PACKET ARRIVED\n");
							ftgmac100_reset(dev);
							ftgmac100_enable(dev);
							return count;
						}
					}
				}
#endif
			}
			else
			{
				printk("not FRS (%d)\n", lp->rx_idx);
				//ignore this fragment
				break;
			}
#if 0
			packet_length += cur_desc->VDBC;

			if ( cur_desc->LRS )		// packet's last frame
			{
				break;
			}
			else
				printk("not LRS (%d)\n", lp->rx_idx);
#else
			if ( cur_desc->LRS )		// packet's last frame
			{
				packet_length += cur_desc->VDBC;
			}
			else
			{
				printk("not LRS (%d)\n", lp->rx_idx);
				//just ignore this fragment
			}
			break;
#endif
		}

		if (have_package==0)
		{
			goto done;
		}
		if (have_frs == 0)
		{
		//	DO_PRINT("error, loss first\n");
			lp->stats.rx_over_errors++;
		}

		if (packet_length>0)
		{
			struct sk_buff  * skb;
			byte		* data = 0;  if (data) { }

#if FTMAC100_DEBUG
			if (net_debug_flag & AST_DBG_BITRATE) {
				unsigned long duration;

				if (gRxTotalLenEx == 0 && gJiffiesStartEx == 0) {
					gJiffiesStartEx = jiffies;
					printk("Wait %d secs...", gBitrateInterval);
				}
				if (gRxTotalLen == 0) {
					gJiffiesStart = jiffies;
				}
				gRxTotalLen += packet_length;
				duration = jiffies_to_msecs(jiffies - gJiffiesStart);
				if (duration > gBitrateInterval * 1000) {
					unsigned long bitrate;
					char str[64];

					bitrate = (gRxTotalLen /duration)*8;

					duration = jiffies_to_msecs(jiffies - gJiffiesStartEx);
					gRxTotalLenEx += gRxTotalLen/1000;

					printk("Net Rx [%lu]Kbps, avg[%lu]Mbps, peak rx desc=%d\n", bitrate, (gRxTotalLenEx /duration)*8, gRcvCnt);
					sprintf(str, "e_net_rate::%luKbps", bitrate);
					ast_notify_user(str);
					gRxTotalLen = 0;
					gJiffiesStart = 0;
				}
			}
#endif

			packet_length -= 4;

			skb_put (skb = lp->rx_skbuff[cur_idx], packet_length);

// Rx Offload
			if (DF_support) {
				if ((data_not_fragment) && (!cs_error))
				{
	//				printk ("Offload\n");
					skb->ip_summed = CHECKSUM_UNNECESSARY;
                            }
				data_not_fragment = 1;
			}

#if FTMAC100_DEBUG > 2
			DO_PRINT("Receiving Packet at 0x%x, packet len = %x\n",(unsigned int)data, packet_length);
			print_packet( data, packet_length );
#endif

			skb->protocol = eth_type_trans(skb, dev );
			ftgmac100_rx_skb(skb);
			lp->stats.rx_packets++;
			lp->rx_skbuff[cur_idx] = NULL;
		}
        if (packet_full) {
//                  DO_PRINT ("RX Buffer full before driver entered ISR\n");
            goto done;
        }
	}

done:

#if FTMAC100_DEBUG
	if (rcv_cnt > gRcvCnt)
		gRcvCnt = rcv_cnt;
#endif

	if (packet_full) {
		struct sk_buff *skb;

		for (cur_idx = 0; cur_idx < lp->rx_buff_num; cur_idx++)
		{
			if (lp->rx_skbuff[cur_idx] == NULL) {
				skb = dev_alloc_skb (lp->rx_buff_size);
				if (skb == NULL) {
					printk ("%s: receive_packet: "
						"Unable to re-allocate Rx skbuff.#%d\n",
						dev->name, cur_idx);
					BUG();
				}
				lp->rx_skbuff[cur_idx] = skb;
				skb->dev = dev;
				// ASPEED: See earlier skb_reserve() cache alignment
				skb_reserve (skb, 2);
				dmac_inv_range ((unsigned long)skb->data,
						(unsigned long)skb->data + lp->rx_buff_size);
				lp->rx_descs[cur_idx].RXBUF_BADR = virt_to_phys(skb->tail);
				lp->rx_descs[cur_idx].VIR_RXBUF_BADR = (unsigned long)skb->tail;
			}
	        lp->rx_descs[cur_idx].RXPKT_RDY = RX_OWNBY_FTGMAC100;
        }
        packet_full = 0;
	}
	else {
		if (start_idx != lp->rx_idx) {
			struct sk_buff *skb;

			for (cur_idx = ((start_idx+1) & (lp->rx_buff_num - 1)); cur_idx != lp->rx_idx; cur_idx = ((cur_idx+1) & (lp->rx_buff_num - 1)))
			{
				//struct sk_buff *skb;
				/* Dropped packets don't need to re-allocate */
				if (lp->rx_skbuff[cur_idx] == NULL) {

					skb = dev_alloc_skb (lp->rx_buff_size);
					if (skb == NULL) {
						printk ("%s: receive_packet: "
							"Unable to re-allocate Rx skbuff.#%d\n",
							dev->name, cur_idx);
						break;
					}
					lp->rx_skbuff[cur_idx] = skb;
					skb->dev = dev;
					/* 16 byte align the IP header */
					skb_reserve (skb, 2);

					dmac_inv_range ((unsigned long)skb->data,
							(unsigned long)skb->data + lp->rx_buff_size);
					lp->rx_descs[cur_idx].RXBUF_BADR = virt_to_phys(skb->tail);
					lp->rx_descs[cur_idx].VIR_RXBUF_BADR = (unsigned long)skb->tail;
				}
				lp->rx_descs[cur_idx].RXPKT_RDY = RX_OWNBY_FTGMAC100;
			}

				//struct sk_buff *skb;
			/* Dropped packets don't need to re-allocate */
			if (lp->rx_skbuff[start_idx] == NULL) {

				skb = dev_alloc_skb (lp->rx_buff_size);
				if (skb == NULL) {
					printk ("%s: receive_packet: "
						"Unable to re-allocate Rx skbuff.#%d\n",
						dev->name, start_idx);
					BUG();
				}
				lp->rx_skbuff[start_idx] = skb;
				skb->dev = dev;
				/* 16 byte align the IP header */
				skb_reserve (skb, 2);
				dmac_inv_range ((unsigned long)skb->data,
						(unsigned long)skb->data + lp->rx_buff_size);
				lp->rx_descs[start_idx].RXBUF_BADR = virt_to_phys(skb->tail); //Richard
				lp->rx_descs[start_idx].VIR_RXBUF_BADR = (unsigned long)skb->tail;
			}
			lp->rx_descs[start_idx].RXPKT_RDY = RX_OWNBY_FTGMAC100;
	    }
	}
	if (trans_busy == 1)
	{
		/// lp->maccr_val |= RXMAC_EN_bit;
		outl( lp->maccr_val, ioaddr + MACCR_REG );
		outl( inl(ioaddr + IER_REG) | RXBUF_UNAVA_bit, ioaddr + IER_REG);
	}
	return count;
}
#endif

/*----------------------------------------------------
 . ftgmac100_close
 .
 . this makes the board clean up everything that it can
 . and not talk to the outside world.   Caused by
 . an 'ifconfig ethX down'
 .
 -----------------------------------------------------*/
static int ftgmac100_close(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;

	//netif_stop_queue(dev);
	netif_tx_disable(dev);
	//dev->start = 0;

	PRINTK2("%s:ftgmac100_close\n", dev->name);

	/* clear everything */
	ftgmac100_shutdown( dev->base_addr );

	if (lp->timer.function != NULL) {
	    del_timer_sync(&lp->timer);
	}

	/* Update the statistics here. */

	return 0;
}

/*------------------------------------------------------------
 . Get the current statistics.
 . This may be called with the card open or closed.
 .-------------------------------------------------------------*/
static struct net_device_stats* ftgmac100_query_statistics(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;

	return &lp->stats;
}



#ifdef HAVE_MULTICAST

/*
 . Function: ftgmac100_setmulticast( struct net_device *dev, int count, struct dev_mc_list * addrs )
 . Purpose:
 .    This sets the internal hardware table to filter out unwanted multicast
 .    packets before they take up memory.
*/

static void ftgmac100_setmulticast( struct net_device *dev, int count, struct dev_mc_list * addrs )
{
	struct dev_mc_list	* cur_addr;
	int crc_val;
	unsigned int	ioaddr = dev->base_addr;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	struct AstMacHwConfig* ids = &lp->ids;
#if NCSI_SUPPORT
	unsigned long Combined_Channel_ID, i;
	struct sk_buff * skb;
#endif
	u32 maht1 = 0, maht0 = 0;

	cur_addr = addrs;
//TX
#if NCSI_SUPPORT
	if (lp->NCSI_support == 1) {
		skb = dev_alloc_skb (TX_BUF_SIZE);
		lp->InstanceID++;
		lp->NCSI_Request.IID = lp->InstanceID;
		lp->NCSI_Request.Command = SET_MAC_ADDRESS;
		Combined_Channel_ID = (lp->NCSI_Cap.Package_ID << 5) + lp->NCSI_Cap.Channel_ID;
		lp->NCSI_Request.Channel_ID = Combined_Channel_ID;
		lp->NCSI_Request.Payload_Length = (8 << 8);
		memcpy ((unsigned char *)skb->data, &lp->NCSI_Request, 30);
		lp->NCSI_Request.Payload_Length = 8;
		for (i = 0; i < 6; i++) {
			lp->Payload_Data[i] = cur_addr->dmi_addr[i];
		}
		lp->Payload_Data[6] = 2; //MAC Address Num = 1 --> address filter 1, fixed in sample code
		lp->Payload_Data[7] = MULTICAST_ADDRESS + 0 + ENABLE_MAC_ADDRESS_FILTER; //AT + Reserved + E
		copy_data (dev, skb, lp->NCSI_Request.Payload_Length);
		skb->len =  30 + lp->NCSI_Request.Payload_Length + 4;
		ftgmac100_wait_to_send_packet (skb, dev);
	}
#endif

	for (cur_addr = addrs ; cur_addr!=NULL ; cur_addr = cur_addr->next )
	{
		/* make sure this is a multicast address - shouldn't this be a given if we have it here ? */
		if ( !( *cur_addr->dmi_addr & 1 ) )
		{
			continue;
		}
//GigaBit
		if (!(ids->isRevA2)) {//A0, A1
			crc_val = crc32( cur_addr->dmi_addr, 5 );
			crc_val = (~(crc_val>>2)) & 0x3f;
			if (crc_val >= 32)
			{
				outl(inl(ioaddr+MAHT1_REG) | (1UL<<(crc_val-32)), ioaddr+MAHT1_REG);
				lp->GigaBit_MAHT1 = inl (ioaddr + MAHT1_REG);
			}
			else
			{
				outl(inl(ioaddr+MAHT0_REG) | (1UL<<crc_val), ioaddr+MAHT0_REG);
				lp->GigaBit_MAHT0 = inl (ioaddr + MAHT0_REG);
			}
//10/100M
			crc_val = crc32( cur_addr->dmi_addr, 6 );
			crc_val = (~(crc_val>>2)) & 0x3f;
			if (crc_val >= 32)
			{
				outl(inl(ioaddr+MAHT1_REG) | (1UL<<(crc_val-32)), ioaddr+MAHT1_REG);
				lp->Not_GigaBit_MAHT1 = inl (ioaddr + MAHT1_REG);
			}
			else
			{
				outl(inl(ioaddr+MAHT0_REG) | (1UL<<crc_val), ioaddr+MAHT0_REG);
				lp->Not_GigaBit_MAHT0 = inl (ioaddr + MAHT0_REG);
			}
		}
		else {//A2
			crc_val = crc32( cur_addr->dmi_addr, 6 );
			crc_val = (~(crc_val>>2)) & 0x3f;
			if (crc_val >= 32)
				maht1 |= (1UL << (crc_val - 32));
			else
				maht0 |= (1UL << crc_val);
		}
	}

	/*
	** Bruce151013. Fix.
	** Driver should re-init MAHT1_REG and MAHT0_REG in case of leaving multicast group.
	** Bruce is lazy, only support isRevA2 code path.
	*/
	if (ids->isRevA2) {
		outl(maht1, ioaddr + MAHT1_REG);
		outl(maht0, ioaddr + MAHT0_REG);
		lp->Not_GigaBit_MAHT1 = maht1;
		lp->GigaBit_MAHT1 = maht1;
		lp->Not_GigaBit_MAHT0 = maht0;
		lp->GigaBit_MAHT0 = maht0;
	}

}


/*-----------------------------------------------------------
 . ftgmac100_set_multicast_list
 .
 . This routine will, depending on the values passed to it,
 . either make it accept multicast packets, go into
 . promiscuous mode ( for TCPDUMP and cousins ) or accept
 . a select set of multicast packets
*/
static void ftgmac100_set_multicast_list(struct net_device *dev)
{
	unsigned int ioaddr = dev->base_addr;
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;

	PRINTK2("%s:ftgmac100_set_multicast_list\n", dev->name);

	if (dev->flags & IFF_PROMISC)
	{
		lp->maccr_val |= RX_ALLADR_bit;
	}
	else
	{
		lp->maccr_val &= ~RX_ALLADR_bit;
	}
	if ( (dev->flags & IFF_ALLMULTI))
	{
		lp->maccr_val |= RX_MULTIPKT_bit;
	}
	else
	{
		lp->maccr_val &= ~RX_MULTIPKT_bit;
	}
	if (dev->mc_count)
	{
//		PRINTK("set multicast\n");
		lp->maccr_val |= RX_HT_EN_bit;
		ftgmac100_setmulticast( dev, dev->mc_count, dev->mc_list );
	}
	else
	{
		lp->maccr_val &= ~RX_HT_EN_bit;
	}
	outl( lp->maccr_val, ioaddr + MACCR_REG );

}
#endif


/*
 * Module initialization function
 */

static char *strdup(const char *s)
{
	char *dups = kmalloc(strlen(s)+1, GFP_KERNEL);
	if (dups)
		strcpy(dups, s);
	return dups;
}

static inline void set_io_resource(int id, struct resource *mac_res)
{
        char buf[32];
        sprintf( buf, "FTGMAC1000 MAC controller %d", id+1 );
        mac_res->name  = strdup( buf );
        mac_res->start = IP_va_base[id];
        mac_res->end   = IP_va_limit[id];
}


static int devnum = IP_COUNT;
module_param(devnum, int, S_IRUGO);


int __init ftgmac100_init(void)
{
        int result, id, thisresult;
        struct net_device *dev;

        result = -ENODEV;

        for (id=0; id < devnum; id++) {

                dev = alloc_etherdev(sizeof(struct ftgmac100_local));
                if (!dev) {
                        printk(KERN_ERR "Fail allocating ethernet device");
                        return -ENODEV;
                }
                ftgmac100_netdev[id] = NULL;
                SET_MODULE_OWNER (dev);
                /* Copy the parameters from the platform specification */
                dev->base_addr = IP_va_base[id];
                dev->irq = IP_irq[id];

                /* Setup initial mac address */
                auto_get_mac(id, dev->dev_addr);

                dev->init = ftgmac100_probe;
                if ((thisresult = register_netdev(dev)) != 0) {
			if (thisresult != 0) {
			    // register_netdev() filled in dev->name.
			    DO_PRINT("%s:register_netdev() returned %d\n",
				     dev->name, thisresult);
			}
                        free_netdev(dev);
                } else {
                        ftgmac100_netdev[id] = dev;

#if FTMAC100_DEBUG
                        pdev.dev.driver_data = dev;
                        result = platform_device_register(&pdev);
                        if (result < 0) {
                                DO_PRINT("platform_driver_register err\n");
                                goto err_out;
                        }

                        result = sysfs_create_group(&pdev.dev.kobj, &dev_attr_group);
                        if (result) {
                                DO_PRINT("can't create sysfs files\n");
                                goto err_out;
                        }
#endif

                }
                if (thisresult == 0) // any of the devices initialized, run
                        result = 0;
        }
err_out:
        return result;
}


/*------------------------------------------------------------
 . Cleanup when module is removed with rmmod
 .-------------------------------------------------------------*/
void ftgmac100_module_exit(void)
{
	int id;
        struct net_device *dev;
        struct ftgmac100_local *priv;
	PRINTK("+cleanup_module\n");

 for (id=0; id < devnum; id++) {
                dev = ftgmac100_netdev[id];

                if (dev==NULL)
                        continue;

#if FTMAC100_DEBUG
                sysfs_remove_group(&pdev.dev.kobj, &dev_attr_group);
                platform_device_unregister(&pdev);
#endif
                priv = (struct ftgmac100_local *)netdev_priv(dev);

#ifdef FREE_TX_IN_WQ
				flush_workqueue(priv->tx_task);
				destroy_workqueue(priv->tx_task);
#endif

                ftgmac100_ringbuf_free(dev);

                /* No need to check MOD_IN_USE, as sys_delete_module() checks. */
                unregister_netdev(dev);

                free_irq(dev->irq, dev);
                free_netdev(dev);
                ftgmac100_netdev[id] = NULL;
                // free resource, free allocated memory
        }
}


module_init(ftgmac100_init);
module_exit(ftgmac100_module_exit);


//---PHY CONTROL AND CONFIGURATION-----------------------------------------
#ifdef not_complete_yet

void ftgmac100_phy_restart_auto(unsigned int ioaddr)
{
    unsigned int tmp;

    if (((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_MARVELL) ||
    	((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211)) {
	tmp = ftgmac100_read_phy_register(ioaddr, PHY_MARVELL_ADDR, 0x00 );
	tmp |= PHY_RE_AUTO_bit;
	ftgmac100_write_phy_register(ioaddr, PHY_MARVELL_ADDR, 0x00, tmp);

	// Waiting for complete Auto-N
	do {
		mdelay(2);
		tmp = ftgmac100_read_phy_register(ioaddr, PHY_MARVELL_ADDR, 0x01 );
	} while ((tmp&PHY_AUTO_OK_bit)==0);
    }
    else if ((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_BROADCOM) {
    }
}
#endif // not_complete_yet

static inline void ftgmac100_phy_rw_waiting(unsigned int ioaddr, unsigned int is_atomic)
{
	unsigned int tmp;

	do {
		if (is_atomic)
			;//mdelay(10);
		else
			msleep(10);

		tmp =inl(ioaddr + PHYCR_REG);
	} while ((tmp&(PHY_READ_bit|PHY_WRITE_bit)) > 0);
}

#if DUMMY_PHY
static word dummy_phy(byte phyreg)
{
	switch(phyreg)
	{
	case 0x00:
		//Reset done. (0x8000 == 0)
		return 0;
	case 0x01:
		//Status= Link_on
		return LINK_STATUS | (PHY_AUTO_OK_bit);
	case 0x02:
		//phy mii id (high)= marvell phy
		return (0x0141);
	case 0x03:
		//phy mii id (low)= marvell phy
		return (0x0c00);
	case 0x11:
		//mode= full duplex, 1G
		return (1 << 13) | (PHY_SPEED_1G << 14);
	case 0x13:
		//int status. no change
		return 0;
	case 0x18:
		return 0;
	default:
		//0x4,0x5,0x9
		return 0;
	}
}
#endif
/*------------------------------------------------------------
 . Reads a register from the MII Management serial interface
 .-------------------------------------------------------------*/
static word gb_ftgmac100_read_phy_register_ex(unsigned int ioaddr, byte phyaddr, byte phyreg, unsigned int is_atomic)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return 0xffff;
	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_READ_bit;

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, is_atomic);

	tmp = (inl(ioaddr + PHYDATA_REG)>>16);
	return tmp;
}

static word ftgmac100_read_phy_register_ex(unsigned int ioaddr, byte phyaddr, byte phyreg, unsigned int is_atomic)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return 0xffff;

#if (DUMMY_PHY) && !(DUMMY_PHY & 0x2) //This is the DUMMY_PHY mode which avoid PHY register read.
	return dummy_phy(phyreg);
#endif

	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_READ_bit;

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, is_atomic);

	tmp = (inl(ioaddr + PHYDATA_REG)>>16);

#if (DUMMY_PHY & 0x2) && !(DUMMY_PHY & 0x1) //Allow access PHY. read PHY status to clear interrupt events.
	return dummy_phy(phyreg);
#else
	return tmp;
#endif
}

/*------------------------------------------------------------
 . Writes a register to the MII Management serial interface
 .-------------------------------------------------------------*/
static void gb_ftgmac100_write_phy_register_ex(unsigned int ioaddr,
	byte phyaddr, byte phyreg, word phydata, unsigned int is_atomic)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return;

	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_WRITE_bit;

	outl( phydata, ioaddr + PHYDATA_REG );

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, is_atomic);
}

static void ftgmac100_write_phy_register_ex(unsigned int ioaddr,
	byte phyaddr, byte phyreg, word phydata, unsigned int is_atomic)
{
	unsigned int tmp;

	if (phyaddr > 0x1f)	// MII chip IDs are 5 bits long
	    return;

#if (DUMMY_PHY) && !(DUMMY_PHY & 0x4)
	return;
#endif
	tmp = inl(ioaddr + PHYCR_REG);

	tmp &= 0x3000003F;
	tmp |=(phyaddr<<16);
	tmp |=(phyreg<<(16+5));
	tmp |=PHY_WRITE_bit;

	outl( phydata, ioaddr + PHYDATA_REG );

	outl( tmp, ioaddr + PHYCR_REG );

	ftgmac100_phy_rw_waiting(ioaddr, is_atomic);
}

static void rtl8211d_phy_reset(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u16 tmp;

	DO_PRINT("Ethernet PHY reset");
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0x0000);

	/*
	 * [15]: reset
	 * [13]: speed[0]
	 * [12]: AN enable, default:1
	 *  [9]: restart AN
	 *  [8]: duplex mode, 1: full. default:1
	 *  [6]: speed[1], default:1
	 *
	 * Not 1000 full-duplex, 82111DN fiber mode doesn't work even if AN is enabled
	 *
	 * change the valude of BMCR from 0x9200 to 0x9340
	 * (For A5, no reset action if fiber mode)
	 */
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0, 0x9340);

	do {
		printk(".");
		msleep(20);
		tmp = (ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x0) & 0x8000);
	} while (tmp);

	printk("\n");
}

static void rtl8211f_phy_reset(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u16 tmp;

	DO_PRINT("Ethernet PHY reset");
	if (ast_scu.astparam.net_drv_option & 0x1) {
		/*
		 * RTL8211FS always keep fiber mode with specified fiber module (RJ45-to-fiber, FSFP-CJ-T10-X1, Ficer Techology)
		 * even if cable is unplugged.
		 * If RJ45 and fiber coexist at the same board with single one PHY, this will cause system cannot link-up when only RJ45 is available.
		 *
		 * use software-forced mode to switch between fiber and copper alternately in reset procedures to fix it.
		 */
		static int mode_sel = -1;
		/*
		 * Page 0xd40, reg16 bit[3:0]=4’b0000  (UTP<->RGMII) one mode
		 * Page 0xd40, reg16 bit[3:0]=4’b0001  (Fiber<->RGMII) one mode
		 * Page 0xd40, reg16 bit[3:0]=4’b0010  (UTP/Fiber<->RGMII) auto mode
		 */
		if (mode_sel == -1)
			mode_sel = lp->ids.is_rtl8211_fiber;
		mode_sel = 1 - (mode_sel & 0x1);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xd40); /* select page 0xd40 */
		tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 16);
		tmp &= 0xFFF0;
		tmp |= mode_sel;
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 16, tmp);
	}

	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0); /* select page 0 */
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0, 0x8000); /* software reset */
	/*
	 * From jerry's test, it is necessary to wait 20ms after software reset
	 * if not, PHY will enter into LPI and we will always get link off
	 */
	do {
		printk(".");
		msleep(20);
		tmp = (ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0) & 0x8000);
	} while (tmp);
	printk("\n");

	/*
	** Bruce151124. 8211FS PHY fiber/UTP auto detection patch.
	** 8211FS will enter fiber mode under high temperature + reset.
	** From realtek, it is caused by PHY internal noise --> wrong detection result.
	** Higher detection threshold can resolve this issue.
	** Change Page 0xdcc Reg20[4:2] from 4 (default 100mVpp) to 7 (160mVpp).
	**
	** FIXME. Can we apply this to 8211FD? (UTP Only PHY)
	*/
	// set page to 0xdcc
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xdcc);
	// read modify write reg 0x14 [4:2]
	tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 20);
	tmp &= ~0x1C;
	tmp |= 0x1C;
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 20, tmp);
	// set page back to default 0xa42
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa42);
}

static void rtk_phy_reset(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;
	switch (lp->ids.miiPhyId & PHYID_VENDOR_MODEL_VER_MASK) {
	case PHYID_RTL8211D_VER:
		rtl8211d_phy_reset(dev);
		break;
	case PHYID_RTL8211F_VER:
		rtl8211f_phy_reset(dev);
		break;
	default:
		break;
	}
}


/*
** Bruce151105.
** 1.8211FS reset PHY will kill the PHY under high temperature.
** 2.Current code path doesn't reset 8211DN as expected. FIXME.
** So, I ignore "disable EEE" for now.
**
** Bruce151124. Root cause and workaround found. Re-enable RTK_PHY_EEE_CONFIG.
** Searh 'Bruce151124' comment for details.
*/
#define RTK_PHY_EEE_CONFIG
#if defined(RTK_PHY_EEE_CONFIG)
static void rtl8211d_eee_disable(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u16 tmp;
	/*
	** Bruce130830. From my test. Disable EEE kills 8211DN fiber mode.
	*/
	if (!lp->ids.is_rtl8211_fiber) {
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0x0005);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 5, 0x8b84);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 6, 0x00c2);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0x0007);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 30, 0x0020);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 21, 0x0000);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 27, 0xa03a);
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0x0000);
#if 0
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0, 0x9200);
		do {
			tmp = (ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x0) & 0x8000);
		} while (tmp);
#else
		rtk_phy_reset(dev);
#endif
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 18, 0x4400);

		tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 19);
	}
}

static void rtl8211f_eee_cfg(struct net_device *dev, u32 enable)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;
	unsigned long ioaddr = dev->base_addr;
	u16 data;
	/*
	 * from RTL8211F_Series_MII(R)GMII_Behavior_in_EEE_App_Note_1 2
	 * Enable (default)
	 *	Reg31 = 0x0000 (select page 0)
	 *	Reg00 = 0x8000
	 *	wait for 20ms
	 *	Reg31 = 0x0A4b (select page 0xa4b)
	 *	Reg17 = 0x1114
	 *	Reg31 = 0x0000 (select page 0)
	 *	Reg13 = 0x0007 (set address mode, MMD device = 7)
	 *	Reg14 = 0x003c (set address value)
	 *	Reg13 = 0x4007 (set data mode, MMD device = 7)
	 *	Reg14 = 0x0006 (set data)
	 * Disable
	 *	Reg31 = 0x0000 (select page 0)
	 *	Reg00 = 0x8000
	 *	wait for 20ms
	 *	Reg31 = 0x0A4b (select page 0xa4b)
	 *	Reg17 = 0x1110
	 *	Reg31 = 0x0000 (select page 0)
	 *	Reg13 = 0x0007 (set address mode, MMD device = 7)
	 *	Reg14 = 0x003c (set address value)
	 *	Reg13 = 0x4007 (set data mode, MMD device = 7)
	 *	Reg14 = 0x0000 (set data)
	 */

#if 0
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0); /* select page 0 */
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0, 0x8000); /* software reset */

	/*
	 * From jerry's test, it is necessary to wait 20ms after software reset
	 * if not, PHY will enter into LPI and we will always get link off
	 */
	mdelay(20);
#else
	rtk_phy_reset(dev);
#endif

	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa4b); /* select page 0xa4b */

	if (enable)
		data = 0x1114;
	else
		data = 0x1110;
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 17, data);
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa42); /* select default page 0xa42 */

	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 13, 0x0007);
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 14, 0x003c);
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 13, 0x4007);

	if (enable)
		data = 0x0006; /* [2]: advertise 1000Base-T EEE capability, [1]: advertise 100Base-T EEE capability */
	else
		data = 0x0000;
	ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 14, data);
}

static void rtk_eee_disable(struct net_device *dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *) dev->priv;

	//DO_PRINT("Disable EEE\n");
	switch (lp->ids.miiPhyId & PHYID_VENDOR_MODEL_VER_MASK) {
	case PHYID_RTL8211D_VER:
		rtl8211d_eee_disable(dev);
		break;
	case PHYID_RTL8211F_VER:
		rtl8211f_eee_cfg(dev, 0);
		break;
	default:
		break;
	}
}
#endif /* #if defined(RTK_PHY_EEE_CONFIG) */


#define MDC_MDIO_PHY_ID 0
#define MDC_MDIO_CTRL0_REG          31
#define MDC_MDIO_START_REG          29
#define MDC_MDIO_CTRL1_REG          21
#define MDC_MDIO_ADDRESS_REG        23
#define MDC_MDIO_DATA_WRITE_REG     24
#define MDC_MDIO_DATA_READ_REG      25
#define MDC_MDIO_PREAMBLE_LEN       32

#define MDC_MDIO_START_OP          0xFFFF
#define MDC_MDIO_ADDR_OP           0x000E
#define MDC_MDIO_READ_OP           0x0001
#define MDC_MDIO_WRITE_OP          0x0003

static unsigned long tmp_io = 0;

u16 gb_rtl8367_phy_read_register(u16 Register_addr)
{
	/* Write address control code to register 31 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_CTRL0_REG,MDC_MDIO_ADDR_OP);

	/* Write address to register 23 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_ADDRESS_REG,Register_addr);

	/* Write read control code to register 21 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_CTRL1_REG,MDC_MDIO_READ_OP);
	
	/* Read data from register 25 */
	return gb_ftgmac100_read_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_DATA_READ_REG);
}

void gb_rtl8367_phy_write_register(u16 Register_addr,u16 register_value)
{
	/* Write address control code to register 31 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_CTRL0_REG,MDC_MDIO_ADDR_OP);

	/* Write address to register 23 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_ADDRESS_REG,Register_addr);

	/* Write data to register 24 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_DATA_WRITE_REG,register_value);

	/* Write data control code to register 21 */
	gb_ftgmac100_write_phy_register(tmp_io,MDC_MDIO_PHY_ID,MDC_MDIO_CTRL1_REG,MDC_MDIO_WRITE_OP);
}

/*------------------------------------------------------------
 . Configures the specified PHY using Autonegotiation.
 .-------------------------------------------------------------*/
static void ftgmac100_phy_configure(struct net_device* dev)
{
	struct ftgmac100_local *lp = (struct ftgmac100_local *)dev->priv;
	unsigned long ioaddr = dev->base_addr;
	tmp_io = ioaddr;
	word tmp;

	if ( STATUS_ERROR == enet_phy_init())
	{
		printk("enet_phy_init fail\n");
	}
	else
	{
		printk("enet_phy_init success\n");
	}

	if ((lp->ids.miiPhyId & PHYID_VENDOR_MODEL_MASK) == PHYID_RTL8211)
	{
		/* Bruce151124. Fiber doesn't support EEE. Don't touch EEE when under fiber mode. */
		if (lp->ids.is_rtl8211_fiber) {
			rtk_phy_reset(dev);
		} else {
#if defined(RTK_PHY_EEE_CONFIG)
			/* disable EEE function */
			/* Assume disable EEE always includes PHY reset. */
			rtk_eee_disable(dev);
#else
			rtk_phy_reset(dev);
#endif
		}

		if (PHYID_RTL8211F_VER == (lp->ids.miiPhyId & PHYID_VENDOR_MODEL_VER_MASK)) {
			/* Fix interrupt configuration */
			//ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 31, 0xa42);
			ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x12, 0x10);

			/* Config LED. See realtek datasheet for details.
			** Scenario 1:
			**	LED1 (Green): 1000 Link
			**	LED2 (Yellow): 10/100/1000 Link + Active
			**	=> LED Control Register =  0x6D00
			** Scenario 2:
			**	LED1 (Green): 10/100/1000 Link
			**	LED2 (Yellow): 10/100/1000 Link + Active
			**	=> LED Control Register = 0x6D60
			** Works for UTP mode only
			**
			** currently, we DO NOT customize LED behavior, use default setting
			*/
#if 0
			ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x1F, 0xD04);
			tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x10);
			tmp = (tmp & 0x9084) | 0x6D60;
			ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x10, tmp);
#endif
			//ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x11, 0x0000);

			/* we have set Reg0x12 before, so disable following procedures */
#if 0
			if (1) {
				/*
				** For RTL8211FS only. FD doesn't have Reg0x12.
				*/
				ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x1F, 0xA42);
				tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x12);
				tmp = (tmp & 0x3FF) | 0x0;
				ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x12, tmp);
			}
#endif
			return;
		}
	}
	if ((lp->ids.miiPhyId & PHYID_VENDOR_MASK) == PHYID_VENDOR_MARVELL) {
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x12, 0x4400);
		tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x13 );

		tmp = ftgmac100_read_phy_register(ioaddr, lp->ids.phyAddr, 0x18 );
		ftgmac100_write_phy_register(ioaddr, lp->ids.phyAddr, 0x18, ((tmp & ~(0x41)) | 0x01));
	}
}


EXPORT_SYMBOL(ftgmac100_wait_to_send_packet);

/*******************************************************************************
** Start of AST HW UDP specific code
*******************************************************************************/
#if NEW_AHU
void mac_ahu_start_rx_filter(struct net_device *dev, u32 dest_ip_addr, u16 dest_udp_port)
{
	unsigned long ioaddr = dev->base_addr;
	u32 r = 0;

	outl(dest_ip_addr, ioaddr + FRMID0_REG);

	r = 0x11 << 24; //UDP protocol
	r |= dest_udp_port << 8;
	outl(r, ioaddr + FRMID1_REG);

	r = inl(ioaddr + FRMID1_REG); //make sure filter is programmed.
	//Enable filter
	r |= (CHK_DST_IP_bit | CHK_PRTCL_bit | CHK_DST_PRT_bit | CHK_MGC_NUM_bit | CHK_VSYNC_SNDR_bit);
	mb();
	outl(r, ioaddr + FRMID1_REG);
}

void mac_ahu_stop_rx_filter(struct net_device *dev)
{
	unsigned long ioaddr = dev->base_addr;

	outl(0, ioaddr + FRMID1_REG);
	outl(0, ioaddr + FRMID0_REG);
	mb();
}

void mac_ahu_vsync_filter(struct net_device *dev, unsigned int enable, unsigned int sender_id)
{
	unsigned long ioaddr = dev->base_addr;

	outl(sender_id, ioaddr + FRMID2_REG);
	mb();
	{
		u32 r;

		r = inl(ioaddr + FRMID1_REG);
		if (enable)
			r |= CHK_VSYNC_SNDR_bit;
		else
			r &= ~CHK_VSYNC_SNDR_bit;

		outl(r, ioaddr + FRMID1_REG);
	}
}

EXPORT_SYMBOL(mac_ahu_start_rx_filter);
EXPORT_SYMBOL(mac_ahu_stop_rx_filter);
EXPORT_SYMBOL(mac_ahu_vsync_filter);
#endif

