#ifndef __SOCLE_MAC_Driver_Value__
#define __SOCLE_MAC_Driver_Value__


//#define	SOCLE_MAC_DEBUG

#define	FINAL_RELEASE_VERSION

/* SOCLE_MAC:
	First define your CPU & Platform here.
*/

#define	CONFIG_SOCLEMAC_BASE_ADDR1 SOCLEMAC_ETH0_ADDR
#define	CONFIG_SOCLEMAC_INTV1 7
#define CONFIG_SOCLEMAC_BASE_ADDR2 SOCLEMAC_ETH1_ADDR
#define   CONFIG_SOCLEMAC_INTV2 10


/* SOCLE_MAC:
	Second define your CPU & Platform only emulation on FPGA or not.
*/

/* SOCLE_MAC:
How many SCOLE MAC device on board. */
#define MAC_MAX_UNITS	1

/* SOCLE_MAC:
	For the SOCLE MAC core with AMBA interface, it is necessary to define
	where in the memory map the device resides and what IRQ the device is
	connected to.  The driver has no mechanism to probe to find out this
	information.  YOU MUST UNCOMMENT THE FOLLOWING #defines and put in
	the correct address and IRQ */
#define MAC_AMBA_BASE_ADDR(x) 	(mac_amba_base_address[x])
#define MAC_AMBA_IRQ(x)			(mac_amba_irq[x])


/* SOCLE_MAC:
	For ARM and MIPS processors, endianess can be either big or little
	depending on how the hardware is wired and configured.  The SOCLE MAC
	core can be configured by the driver for either case, so it is necessary
	to uncomment one of the following lines which will cause the appropriate
	bits to be set or cleared below in csr0 (bits 7 and 20).
*/
#define MAC_LITTLE_ENDIAN
#define CPU_LITTLE_ENDIAN

// Mark by alan_tsai 2006/02/20 work around for hw DMA burst mode bug
//#define DEFAULT_CSR0_SETTING	(CSR0_BAR_RXH | CSR0_DSL_VAL(0) | CSR0_PBL_VAL(16) | CSR0_TAP_VAL(1))
#define DEFAULT_CSR0_SETTING	(CSR0_BAR_RXH | CSR0_DSL_VAL(0) | CSR0_PBL_VAL(32) | CSR0_TAP_VAL(3))


#if defined (MAC_LITTLE_ENDIAN)
/* turn off the endian bits */
#elif defined (MAC_BIG_ENDIAN)
/* turn on the endian bits */
#else
/* display an error message */
#error "!!!! SOCLE MAC core Endianess undefined - YOU MUST FIX THIS IN SOCLE_MAC_Driver_Value.h!"
#endif


#define DEFAULT_CSR6_SETTING	(CSR6_SF)
#define DEFAULT_CSR11_SETTING	(CSR11_TT_VAL(0)|CSR11_NTP_VAL(0)|CSR11_RT_VAL(0)|CSR11_NRP_VAL(0))

/* Operational parameters that are set at compile time. */
/* Keep the descriptor ring sizes a power of two for efficiency.
   The Tx queue length limits transmit packets to a portion of the available
   ring entries.  It should be at least one element less to allow multicast
   filter setup frames to be queued.  It must be at least four for hysteresis.
   Making the Tx queue too long decreases the effectiveness of channel
   bonding and packet priority.
   Large receive rings waste memory and confound network buffer limits.
   These values have been carefully studied: changing these might mask a
   problem, it won't fix it.
*/


//+grant
#define TX_RING_SIZE	32
#define TX_QUEUE_LEN	28
#define RX_RING_SIZE	64

//#define TX_RING_SIZE	16
//#define TX_QUEUE_LEN	10
//#define RX_RING_SIZE	32

/* Operational parameters that usually are not changed. */
#define LINK_CHECK_FAST	(1*HZ)
//#define LINK_CHECK_SLOW	(30*HZ) +grant
#define LINK_CHECK_SLOW	(30*HZ)

/* Time in jiffies before concluding the transmitter is hung. */
//#define TX_TIMEOUT  	(6*HZ) +grant
#define TX_TIMEOUT  	(6*HZ)

/* Preferred skbuff allocation size. */
#define PKT_BUF_SZ		1536


/* SOCLE_MAC:
	CSR7 interrupt enable settings,
	we define an explict interrupt enable bitmap for the SOCLE MAC Modules.
	It enables all interrupts except for ETE. */
#define SOCLE_MAC_INTR	(					\
				CSR7_NIS		\
                         |	CSR7_AIS		\
/*                         |	CSR7_ERI	*/	\
/*                         |	CSR7_GPTE	*/	\
/*                         |	CSR7_ETI	*/	\
                         |	CSR7_RPS		\
                         |	CSR7_RU			\
                         |	CSR7_RI			\
                         |	CSR7_UNF		\
                         |	CSR7_TU			\
                         |	CSR7_TPS		\
                         |	CSR7_TI			\
                        )

/* SOCLE_MAC:
	Define should we include the SROM(EEPROM) on MAC driver
	EEPROM_SIZE will need to be set based on what EEPROM/SROM is used
*/
//+grant


#define DEFAULT_IFPORTE		(options_AutoNeg)

#define SOCLE_MAC_HAVE_PRIVATE_IOCTL

#endif	/* __SOCLE_MAC_Driver_Value__ */
