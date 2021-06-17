// --------------------------------------------------------------------

#ifndef FTMAC100_H
#define FTMAC100_H
#include <linux/types.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/bitops.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#define CACHE_ABLE    1

#define IP_COUNT                1
#define BUFPOOL_SIZE		0x20000
#define BUFPOOL_OFFSET_RXDESC	0
#define BUFPOOL_OFFSET_RXBUF	(BUFPOOL_OFFSET_RXDESC+0x1000)
#define BUFPOOL_OFFSET_TXDESC	(BUFPOOL_OFFSET_RXBUF+0x10000)
#define BUFPOOL_OFFSET_TXBUF	(BUFPOOL_OFFSET_TXDESC+0x1000)


typedef unsigned char			byte;
typedef unsigned short			word;
typedef unsigned long int 		dword;


#if (CONFIG_AST1500_SOC_VER >= 2)
	#define NEW_JUMBO_FRAME
#endif

#define NCSI_SUPPORT 0
#define CONFIG_FTGMAC100_NAPI
#define FTGMAC100_NAPI_WEIGHT	1
#define FREE_TX_IN_WQ 1
#define TEST_USE_HPTX 0
#define HPTX_SUPPORT 1

#define PHY_FIBER_STATUS_IN_WQ

#if defined(PHY_FIBER_STATUS_IN_WQ)
#define PHY_FIBER_STATUS_PERIOD_MS     2000
#endif


// --------------------------------------------------------------------
//		FTMAC100 hardware related defenition
// --------------------------------------------------------------------
#define HAVE_MULTICAST

//base address and interrupt information
#define NC_GMAC1_BASE		0xE0400000
#define IRQ_GMAC1		6
#define NC_GMAC2_BASE		0xE0500000
#define IRQ_GMAC2		5


#define ISR_REG				0x00				// interrups status register
#define IER_REG				0x04				// interrupt maks register
#define MAC_MADR_REG		0x08				// MAC address (Most significant)
#define MAC_LADR_REG		0x0c				// MAC address (Least significant)

#define MAHT0_REG			0x10				// Multicast Address Hash Table 0 register
#define MAHT1_REG			0x14				// Multicast Address Hash Table 1 register
#define TXPD_REG			0x18				// Transmit Poll Demand register
#define RXPD_REG			0x1c				// Receive Poll Demand register
#define TXR_BADR_REG		0x20				// Transmit Ring Base Address register
#define RXR_BADR_REG		0x24				// Receive Ring Base Address register

#define HPTXPD_REG			0x28	//
#define HPTXR_BADR_REG		0x2c	//

#define ITC_REG				0x30				// interrupt timer control register
#define APTC_REG			0x34				// Automatic Polling Timer control register
#define DBLAC_REG			0x38				// DMA Burst Length and Arbitration control register

#define DMAFIFOS_REG		0x3c	//
#define FEAR_REG			0x44	//
#define TPAFCR_REG			0x48	//
#define RBSR_REG			0x4c	//for NC Body
#define MACCR_REG			0x50				// MAC control register
#define MACSR_REG			0x54				// MAC status register
#define PHYCR_REG			0x60				// PHY control register
#define PHYDATA_REG			0x64				// PHY Write Data register
#define FCR_REG				0x68				// Flow Control register
#define BPR_REG				0x6c				// back pressure register
#define WOLCR_REG			0x70				// Wake-On-Lan control register
#define WOLSR_REG			0x74				// Wake-On-Lan status register
#define WFCRC_REG			0x78				// Wake-up Frame CRC register
#define WFBM1_REG			0x80				// wake-up frame byte mask 1st double word register
#define WFBM2_REG			0x84				// wake-up frame byte mask 2nd double word register
#define WFBM3_REG			0x88				// wake-up frame byte mask 3rd double word register
#define WFBM4_REG			0x8c				// wake-up frame byte mask 4th double word register

#define NPTXR_PTR_REG		0x90	//
#define HPTXR_PTR_REG		0x94	//
#define RXR_PTR_REG			0x98	//

#define FRMID0_REG			0x150
#define FRMID1_REG			0x154
#define FRMID2_REG			0x158

// --------------------------------------------------------------------
//		ISR_REG IMR_REG
// --------------------------------------------------------------------
#define HPTXBUF_UNAVA_bit	(1UL<<10)
#define PHYSTS_CHG_bit		(1UL<<9)
#define AHB_ERR_bit			(1UL<<8)
#define TPKT_LOST_bit		(1UL<<7)
#define NPTXBUF_UNAVA_bit	(1UL<<6)
#define TPKT2F_bit			(1UL<<5)
#define TPKT2E_bit			(1UL<<4)
#define RPKT_LOST_bit		(1UL<<3)
#define RXBUF_UNAVA_bit		(1UL<<2)
#define RPKT2F_bit			(1UL<<1)
#define RPKT2B_bit			(1UL<<0)


// --------------------------------------------------------------------
//	APTC_REG
// --------------------------------------------------------------------

#ifndef CONFIG_CPU_BIG_ENDIAN

typedef struct
{
	u32 RXPOLL_CNT:4;
	u32 RXPOLL_TIME_SEL:1;
	u32 Reserved1:3;
	u32 TXPOLL_CNT:4;
	u32 TXPOLL_TIME_SEL:1;
	u32 Reserved2:19;
}FTGMAC100_APTCR_Status;

#else

typedef struct
{
	u32 Reserved2:19;
	u32 TXPOLL_TIME_SEL:1;
	u32 TXPOLL_CNT:4;
	u32 Reserved1:3;
	u32 RXPOLL_TIME_SEL:1;
	u32 RXPOLL_CNT:4;
}FTGMAC100_APTCR_Status;

#endif

// --------------------------------------------------------------------
//		PHYCR_REG
// --------------------------------------------------------------------
#define PHY_RE_AUTO_bit			(1UL<<9)
#define PHY_READ_bit			(1UL<<26)
#define PHY_WRITE_bit			(1UL<<27)
// --------------------------------------------------------------------
//		PHYCR_REG
// --------------------------------------------------------------------
#define PHY_AUTO_OK_bit			(1UL<<5)
// --------------------------------------------------------------------
//		PHY INT_STAT_REG
// --------------------------------------------------------------------
#define PHY_SPEED_CHG_bit		(1UL<<14)
#define PHY_DUPLEX_CHG_bit		(1UL<<13)
#define PHY_LINK_CHG_bit		(1UL<<10)
#define PHY_AUTO_COMP_bit		(1UL<<11)
// --------------------------------------------------------------------
//		PHY SPE_STAT_REG
// --------------------------------------------------------------------
#define PHY_RESOLVED_bit		(1UL<<11)
#define PHY_SPEED_mask			0xC000
#define PHY_SPEED_10M			0x0
#define PHY_SPEED_100M			0x1
#define PHY_SPEED_1G			0x2
#define PHY_DUPLEX_mask                 0x2000
//#define PHY_FULLDUPLEX                  0x1
#define PHY_SPEED_DUPLEX_MASK           0x01E0
#define PHY_100M_DUPLEX                 0x0100
#define PHY_100M_HALF                   0x0080
#define PHY_10M_DUPLEX                  0x0040
#define PHY_10M_HALF                    0x0020
#define LINK_STATUS                     0x04


// --------------------------------------------------------------------
//		MACCR_REG
// --------------------------------------------------------------------

#define SW_RST_bit			(1UL<<31)				// software reset/
#define DIRPATH_bit			(1UL<<21)
#define RX_IPCS_FAIL_bit	(1UL<<20)	//
#define SPEED_100_bit   	        (1UL<<19)	//
#define RX_UDPCS_FAIL_bit	(1UL<<18)	//
#define RX_BROADPKT_bit		(1UL<<17)				// Receiving broadcast packet
#define RX_MULTIPKT_bit		(1UL<<16)				// receiving multicast packet
#define RX_HT_EN_bit		(1UL<<15)
#define RX_ALLADR_bit		(1UL<<14)				// not check incoming packet's destination address
#define JUMBO_LF_bit		(1UL<<13)	//
#define RX_RUNT_bit			(1UL<<12)				// Store incoming packet even its length is les than 64 byte
#define PHY_INT_LOW_ACTIVE_bit			(1UL<<11)	// SoC > AST1510 only
#define CRC_APD_bit			(1UL<<10)				// append crc to transmit packet
#define GMAC_MODE_bit		(1UL<<9)	//
#define FULLDUP_bit			(1UL<<8)				// full duplex
#define ENRX_IN_HALFTX_bit	(1UL<<7)	//
#define LOOP_EN_bit			(1UL<<6)				// Internal loop-back
#define HPTXR_EN_bit		(1UL<<5)	//
#define REMOVE_VLAN_bit		(1UL<<4)	//
//#define MDC_SEL_bit		(1UL<<13)				// set MDC as TX_CK/10
//#define RX_FTL_bit		(1UL<<11)				// Store incoming packet even its length is great than 1518 byte
#define RXMAC_EN_bit		(1UL<<3)				// receiver enable
#define TXMAC_EN_bit		(1UL<<2)				// transmitter enable
#define RXDMA_EN_bit		(1UL<<1)				// enable DMA receiving channel
#define TXDMA_EN_bit		(1UL<<0)				// enable DMA transmitting channel


// --------------------------------------------------------------------
//		FRMID1_REG
// --------------------------------------------------------------------
#define IP_PRTCL_mask		(0xFFUL<<24)			// IP protocol check
#define PRT_mask			(0xFFFFUL<<8)			// UDP/TCP Port number check
#define CHK_VSYNC_SNDR_bit	(0x1UL<<4)				// check sender id on vysnc packets
#define CHK_MGC_NUM_bit		(0x1UL<<3)				// check magic number
#define CHK_DST_PRT_bit		(0x1UL<<2)				// check destination port number
#define CHK_PRTCL_bit		(0x1UL<<1)				// check IP protocol
#define CHK_DST_IP_bit		(0x1UL<<0)				// check destination ip address


// --------------------------------------------------------------------
//		SCU_REG
// --------------------------------------------------------------------
#define  SCU_PROTECT_KEY_REG                0x0
#define  SCU_PROT_KEY_MAGIC                 0x1688a8a8
#define  SCU_RESET_CONTROL_REG              0x04
#define  SCU_RESET_MAC1                     (1u << 11)
#define  SCU_RESET_MAC2                     (1u << 12)

#define  SCU_HARDWARE_TRAPPING_REG          0x70
#define  SCU_HT_MAC_INTF_LSBIT              6
#define  SCU_HT_MAC_INTERFACE               (0x7u << SCU_HT_MAC_INTF_LSBIT)
#define  MAC_INTF_SINGLE_PORT_MODES         (1u<<0/*GMII*/ | 1u<<3/*MII_ONLY*/ | 1u<<4/*RMII_ONLY*/)
#define  SCU_HT_MAC_GMII                    0x0u
// MII and MII mode
#define  SCU_HT_MAC_MII_MII                 0x1u
#define  SCU_HT_MAC_MII_ONLY                0x3u
#define  SCU_HT_MAC_RMII_ONLY               0x4u

#if (CONFIG_AST1500_SOC_VER >= 2)
	#define  SCU_MULTIFUNCTION_PIN_CTL1_REG     0x80
	#define  SCU_MULTIFUNCTION_PIN_CTL3_REG     0x88
	#define  SCU_MULTIFUNCTION_PIN_CTL5_REG     0x90
	/*
	SCU88 D[31]: MAC1 MDIO
	SCU88 D[30]: MAC1 MDC
	SCU90 D[2]:  MAC2 MDC/MDIO
	SCU80 D[0]:  MAC1 Link
	SCU80 D[1]:  MAC2 Link
	*/
	#define  SCU_MFP_MAC2_PHYLINK               (1u << 1)
	#define  SCU_MFP_MAC1_PHYLINK               (1u << 0)
	#define  SCU_MFP_MAC2_MII_INTF              (1u << 21)
	#define  SCU_MFP_MAC2_MDC_MDIO              (1u << 2)
	#define  SCU_MFP_MAC1_MDIO                  (1u << 31)
	#define  SCU_MFP_MAC1_MDC                   (1u << 30)
#else
	#define  SCU_MULTIFUNCTION_PIN_REG			0x74
	#define  SCU_MFP_MAC2_PHYLINK               (1u << 26)
	#define  SCU_MFP_MAC1_PHYLINK               (1u << 25)
	#define  SCU_MFP_MAC2_MII_INTF              (1u << 21)
	#define  SCU_MFP_MAC2_MDC_MDIO              (1u << 20)
#endif

#define  SCU_SILICON_REVISION_REG			0x7C
#define  SCU_SCRATCH_REG				0x40


// --------------------------------------------------------------------
//		NCSI
// --------------------------------------------------------------------

//NCSI define & structure
//NC-SI Command Packet
typedef struct {
//Ethernet Header
    unsigned char  DA[6];
    unsigned char  SA[6];
    unsigned short EtherType;						//DMTF NC-SI
//NC-SI Control Packet
    unsigned char  MC_ID;						//Management Controller should set this field to 0x00
    unsigned char  Header_Revision;					//For NC-SI 1.0 spec, this field has to set 0x01
    unsigned char  Reserved_1;						//Reserved has to set to 0x00
    unsigned char  IID;							//Instance ID
    unsigned char  Command;
    unsigned char  Channel_ID;
    unsigned short Payload_Length;					//Payload Length = 12 bits, 4 bits are reserved
    unsigned long  Reserved_2;
    unsigned long  Reserved_3;
}  NCSI_Command_Packet;

//Command and Response Type
#define	CLEAR_INITIAL_STATE			0x00			//M
#define	SELECT_PACKAGE				0x01			//M
#define	DESELECT_PACKAGE			0x02			//M
#define	ENABLE_CHANNEL				0x03			//M
#define	DISABLE_CHANNEL				0x04			//M
#define	RESET_CHANNEL				0x05			//M
#define	ENABLE_CHANNEL_NETWORK_TX		0x06			//M
#define	DISABLE_CHANNEL_NETWORK_TX		0x07			//M
#define	AEN_ENABLE				0x08
#define	SET_LINK				0x09			//M
#define	GET_LINK_STATUS				0x0A			//M
#define	SET_VLAN_FILTER				0x0B			//M
#define	ENABLE_VLAN				0x0C			//M
#define	DISABLE_VLAN				0x0D			//M
#define	SET_MAC_ADDRESS				0x0E			//M
#define	ENABLE_BROADCAST_FILTERING		0x10			//M
#define	DISABLE_BROADCAST_FILTERING		0x11			//M
#define	ENABLE_GLOBAL_MULTICAST_FILTERING	0x12
#define	DISABLE_GLOBAL_MULTICAST_FILTERING	0x13
#define	SET_NCSI_FLOW_CONTROL			0x14
#define	GET_VERSION_ID				0x15			//M
#define	GET_CAPABILITIES			0x16			//M
#define	GET_PARAMETERS				0x17			//M
#define	GET_CONTROLLER_PACKET_STATISTICS	0x18
#define	GET_NCSI_STATISTICS			0x19
#define	GET_NCSI_PASS_THROUGH_STATISTICS	0x1A

//NC-SI Response Packet
typedef struct {
    unsigned char  DA[6];
    unsigned char  SA[6];
    unsigned short EtherType;						//DMTF NC-SI
//NC-SI Control Packet
    unsigned char  MC_ID;						//Management Controller should set this field to 0x00
    unsigned char  Header_Revision;					//For NC-SI 1.0 spec, this field has to set 0x01
    unsigned char  Reserved_1;						//Reserved has to set to 0x00
    unsigned char  IID;							//Instance ID
    unsigned char  Command;
    unsigned char  Channel_ID;
    unsigned short Payload_Length;					//Payload Length = 12 bits, 4 bits are reserved
    unsigned short  Reserved_2;
    unsigned short  Reserved_3;
    unsigned short  Reserved_4;
    unsigned short  Reserved_5;
    unsigned short  Response_Code;
    unsigned short  Reason_Code;
    unsigned char   Payload_Data[64];
}  NCSI_Response_Packet;

//Standard Response Code
#define	COMMAND_COMPLETED			0x00
#define	COMMAND_FAILED				0x01
#define	COMMAND_UNAVAILABLE			0x02
#define	COMMAND_UNSUPPORTED			0x03

//Standard Reason Code
#define	NO_ERROR				0x0000
#define	INTERFACE_INITIALIZATION_REQUIRED	0x0001
#define	PARAMETER_IS_INVALID			0x0002
#define	CHANNEL_NOT_READY			0x0003
#define	PACKAGE_NOT_READY			0x0004
#define	INVALID_PAYLOAD_LENGTH			0x0005
#define	UNKNOWN_COMMAND_TYPE			0x7FFF


struct AEN_Packet {
//Ethernet Header
    unsigned char  DA[6];
    unsigned char  SA[6];						//Network Controller SA = FF:FF:FF:FF:FF:FF
    unsigned short EtherType;						//DMTF NC-SI
//AEN Packet Format
    unsigned char  MC_ID;						//Network Controller should set this field to 0x00
    unsigned char  Header_Revision;					//For NC-SI 1.0 spec, this field has to set 0x01
    unsigned char  Reserved_1;						//Reserved has to set to 0x00
//    unsigned char  IID = 0x00;						//Instance ID = 0 in Network Controller
//    unsigned char  Command = 0xFF;					//AEN = 0xFF
    unsigned char  Channel_ID;
//    unsigned short Payload_Length = 0x04;				//Payload Length = 4 in Network Controller AEN Packet
    unsigned long  Reserved_2;
    unsigned long  Reserved_3;
    unsigned char  AEN_Type;
//    unsigned char  Reserved_4[3] = {0x00, 0x00, 0x00};
    unsigned long  Optional_AEN_Data;
    unsigned long  Payload_Checksum;
};

//AEN Type
#define	LINK_STATUS_CHANGE			0x0
#define	CONFIGURATION_REQUIRED			0x1
#define	HOST_NC_DRIVER_STATUS_CHANGE		0x2

typedef struct {
	unsigned char Package_ID;
	unsigned char Channel_ID;
	unsigned long Capabilities_Flags;
	unsigned long Broadcast_Packet_Filter_Capabilities;
	unsigned long Multicast_Packet_Filter_Capabilities;
	unsigned long Buffering_Capabilities;
	unsigned long AEN_Control_Support;
} NCSI_Capability;


//SET_MAC_ADDRESS
#define UNICAST		(0x00 << 5)
#define MULTICAST_ADDRESS	(0x01 << 5)
#define DISABLE_MAC_ADDRESS_FILTER	0x00
#define ENABLE_MAC_ADDRESS_FILTER	0x01

//GET_LINK_STATUS
#define LINK_DOWN	0
#define LINK_UP		1

#define NCSI_LOOP   1500000
#define RETRY_COUNT     1

#define NCSI_HEADER	0xF888		//Reversed because of 0x88 is low byte, 0xF8 is high byte in memory

// --------------------------------------------------------------------
//		Receive Ring descriptor structure
// --------------------------------------------------------------------


#ifdef CONFIG_CPU_BIG_ENDIAN
typedef struct
{
	// RXDES0
	u32 RXPKT_RDY:1;		// 1 ==> owned by FTMAC100, 0 ==> owned by software
	u32 EDORR:1;
	u32 FRS:1;
	u32 LRS:1;
	u32 Reserved2:2;
	u32 PAUSE_FRAME:1;
	u32 PAUSE_OPCODE:1;
	u32 FIFO_FULL:1;
	u32 RX_ODD_NB:1;
	u32 RUNT:1;
	u32 FTL:1;
	u32 CRC_ERR:1;
	u32 RX_ERR:1;
	u32 BROADCAST:1;
	u32 MULTICAST:1;
	u32 Reserved3:1;
	u32 Reserved1:1;
	u32 VDBC:14;


	// RXDES1
	u32 Reserved5:4;
	u32 IPCS_FAIL:1;
	u32 UDPCS_FAIL:1;
	u32 TCPCS_FAIL:1;
	u32 VLAN_AVA:1;
	u32 DF:1;
	u32 LLC_PKT:1;
	u32 PROTL_TYPE:2;
	u32 Reserved4:4;
	u32 VLAN_TAGC:16;


	// RXDES2
	u32 Reserved6:32;

	// RXDES3
	u32 RXBUF_BADR;

	u32 VIR_RXBUF_BADR;			// not defined, the virtual address of receive buffer is placed here

	u32 RESERVED;
	u32 RESERVED1;
	u32 RESERVED2;
}RX_DESC;

#else // little endian

typedef struct
{
	// RXDES0
	u32 VDBC:14;//0~10
	u32 Reserved1:1;          //11~15
	u32 Reserved3:1;
	u32 MULTICAST:1;          //16
	u32 BROADCAST:1;          //17
	u32 RX_ERR:1;             //18
	u32 CRC_ERR:1;            //19
	u32 FTL:1;
	u32 RUNT:1;
	u32 RX_ODD_NB:1;
	u32 FIFO_FULL:1;
	u32 PAUSE_OPCODE:1;
	u32 PAUSE_FRAME:1;
	u32 FRAMEID_HIT:1; //bit 26
	u32 FRAMEID_GENLOCK_HIT:1; //bit 27
	u32 LRS:1;
	u32 FRS:1;
	u32 EDORR:1;
	u32 RXPKT_RDY:1;		// 1 ==> owned by FTMAC100, 0 ==> owned by software

	// RXDES1
	u32 VLAN_TAGC:16;
	u32 Reserved4:4;
	u32 PROTL_TYPE:2;
	u32 LLC_PKT:1;
	u32 DF:1;
	u32 VLAN_AVA:1;
	u32 TCPCS_FAIL:1;
	u32 UDPCS_FAIL:1;
	u32 IPCS_FAIL:1;
	u32 Reserved5:1; //bit 28
	u32 FRAMEID_PAYLOAD_OFFSET:3; //D[29::31]

	// RXDES2
	u32 Reserved6:32;

	// RXDES3
	u32 RXBUF_BADR;

	u32 VIR_RXBUF_BADR;			// not defined, the virtual address of receive buffer is placed here

	u32 RESERVED;
	u32 RESERVED1;
	u32 RESERVED2;
}RX_DESC;

#endif


#ifdef CONFIG_CPU_BIG_ENDIAN

typedef struct
{
	// TXDES0
	u32 TXDMA_OWN:1;
	u32 EDOTR:1;
	u32 FTS:1;
	u32 LTS:1;
	u32 Reserved4:8;
	u32 CRC_ERR:1;
	u32 Reserved3:3;
	u32 Reserved2:1;
	u32 Reserved1:1;
	u32 TXBUF_Size:14;

	// TXDES1
	u32 TXIC:1;
	u32 TX2FIC:1;
	u32 Reserved6:7;
	u32 LLC_PKT:1;
	u32 Reserved5:2;
	u32 IPCS_EN:1;
	u32 UDPCS_EN:1;
	u32 TCPCS_EN:1;
	u32 INS_VLAN:1;
	u32 VLAN_TAGC:16;

	// TXDES2
	u32 Reserved7:32;

	// TXDES3
	u32 TXBUF_BADR;

	u32 VIR_TXBUF_BADR;			// Reserve, the virtual address of transmit buffer is placed here

	u32 RESERVED;
	u32 RESERVED1;
	u32 RESERVED2;
}TX_DESC;

#else //little endian

typedef struct
{
	// TXDES0
	u32 TXBUF_Size:14;
	u32 Reserved1:1;
	u32 Reserved2:1;
	u32 Reserved3:3;
	u32 CRC_ERR:1;
	u32 Reserved4:8;
	u32 LTS:1;
	u32 FTS:1;
	u32 EDOTR:1;
	u32 TXDMA_OWN:1;

	// TXDES1
	u32 VLAN_TAGC:16;
	u32 INS_VLAN:1;
	u32 TCPCS_EN:1;
	u32 UDPCS_EN:1;
	u32 IPCS_EN:1;
	u32 Reserved5:2;
	u32 LLC_PKT:1;
	u32 Reserved6:7;
	u32 TX2FIC:1;
	u32 TXIC:1;

	// TXDES2
	u32 Reserved7:32;

	// TXDES3
	u32 TXBUF_BADR;

	u32 VIR_TXBUF_BADR;			// Reserve, the virtual address of transmit buffer is placed here

	u32 RESERVED;
	u32 RESERVED1;
	u32 RESERVED2;

}TX_DESC;
#endif



// waiting to do:
#define	TXPOLL_CNT			8
#define RXPOLL_CNT			0

#define TX_OWNBY_SOFTWARE		0
#define TX_OWNBY_FTGMAC100		1


#define RX_OWNBY_SOFTWARE		1
#define RX_OWNBY_FTGMAC100		0

// --------------------------------------------------------------------
//		driver related definition
// --------------------------------------------------------------------

#if (CONFIG_AST1500_SOC_VER >= 3)
#define MAX_RX_BUF_NUM			(1 << 9) //Must be power of 2
#else
#define MAX_RX_BUF_NUM			(1 << 9) //Must be power of 2 //Bruce120521.Change from 8 to 9 for RctBug#2012051500.
#endif

#if 0
//#define RX_BUF_SIZE			512					//  receive buffer  512 byte
#ifdef NEW_JUMBO_FRAME
#define RX_BUF_SIZE			8192
#else
#define RX_BUF_SIZE			1536
#endif
#endif

//#define TXDES_NUM			64//64 //define 32 for OTG working
//#define TX_BUF_SIZE			2048

#define TXDES_NUM			(1 << 12) //Must be power of 2
#ifdef NEW_JUMBO_FRAME
#define TX_BUF_SIZE			8192
#else
#define TX_BUF_SIZE			1536
#endif
#define PHYID_VENDOR_MASK		0xfffffc00
#define PHYID_VENDOR_MODEL_MASK		0xfffffff0
#define PHYID_VENDOR_MARVELL		0x01410c00
#define PHYID_VENDOR_MODEL_VER_MASK	0x0000000f
#define PHYID_VENDOR_BROADCOM		0x00406000
#define PHYID_VENDOR_REALTEK		0x001cc800
#define PHYID_RTL8201EL			0x001cc810
#define PHYID_RTL8211			0x001cc910
#define PHYID_RTL8211D_VER		0x00000004
#define PHYID_RTL8211E_VER		0x00000005
#define PHYID_RTL8211F_VER		0x00000006


/* store this information for the driver.. */

struct AstMacHwConfig {
    unsigned char phyAddr;	// See IP_phy_addr[] encoding
    unsigned char macId;
    unsigned char isRevA0;
    unsigned char isRevA2;
    unsigned char pad[1];
    unsigned int  miiPhyId;
    unsigned int is_rtl8211_fiber;
};

struct ftgmac100_local {

 	// these are things that the kernel wants me to keep, so users
	// can find out semi-useless statistics of how well the card is
	// performing
	struct net_device_stats stats;

	struct AstMacHwConfig ids;

	// Set to true during the auto-negotiation sequence
	int	autoneg_active;

	// Last contents of PHY Register 18
	u32		lastPhy18;

	spinlock_t lock;

	struct sk_buff *rx_skbuff[MAX_RX_BUF_NUM];

	volatile RX_DESC *rx_descs;			// receive ring base address
	u32		rx_descs_dma;				// receive ring physical base address
	int		rx_idx;						// receive descriptor

	volatile TX_DESC *tx_descs;
	u32		tx_descs_dma;
	int		tx_idx;
	int		old_tx;
	struct sk_buff *tx_skbuff[TXDES_NUM];
	unsigned long	tx_free;
#if HPTX_SUPPORT
	volatile TX_DESC *hptx_descs;
	u32		hptx_descs_dma;
	int		hptx_idx;
	int		old_hptx;
	struct sk_buff *hptx_skbuff[TXDES_NUM];
	unsigned long	hptx_free;
#endif
	int     maccr_val;
        struct timer_list    timer;
	u32		GigaBit_MAHT0;
	u32		GigaBit_MAHT1;
	u32		Not_GigaBit_MAHT0;
	u32		Not_GigaBit_MAHT1;
	NCSI_Command_Packet NCSI_Request;
	NCSI_Response_Packet NCSI_Respond;
	NCSI_Capability NCSI_Cap;
	unsigned int 	InstanceID;
	unsigned int 	Retry;
	unsigned char	Payload_Data[16];
	unsigned char	Payload_Pad[4];
	unsigned long	Payload_Checksum;
	unsigned long	NCSI_support;
	unsigned long	INTEL_NCSI_EVA_support;
	unsigned int 	tx_buff_size;
	unsigned int 	rx_buff_size;
	unsigned int 	rx_buff_num;
#ifdef FREE_TX_IN_WQ
	struct workqueue_struct  *tx_task;
	struct work_struct        tx_free_work;
	struct work_struct        link_chg_work;
#else
	struct tasklet_struct	tx_task;
#endif
#if defined(PHY_FIBER_STATUS_IN_WQ)
	struct work_struct        phy_change_work;
#endif
};


#define FTGMAC100_STROBE_TIME			(10*HZ)
///#define FTMAC100_STROBE_TIME			1

//I2C define for EEPROM
#define    AC_TIMING        0x77743335
#define    ALL_CLEAR        0xFFFFFFFF
#define    MASTER_ENABLE    0x01
#define    SLAVE_ENABLE     0x02
#define    LOOP_COUNT       0x100000


#define    I2C_BASE                         0x1e78A000
#define    I2C_FUNCTION_CONTROL_REGISTER    0x00
#define    I2C_AC_TIMING_REGISTER_1         0x04
#define    I2C_AC_TIMING_REGISTER_2         0x08
#define    I2C_INTERRUPT_CONTROL_REGISTER   0x0C
#define    I2C_INTERRUPT_STATUS_REGISTER    0x10
#define    I2C_COMMAND_REGISTER             0x14
#define    I2C_BYTE_BUFFER_REGISTER         0x20


#define    MASTER_START_COMMAND    (1 << 0)
#define    MASTER_TX_COMMAND       (1 << 1)
#define    MASTER_RX_COMMAND       (1 << 3)
#define    RX_COMMAND_LIST         (1 << 4)
#define    MASTER_STOP_COMMAND     (1 << 5)

#define    TX_ACK       (1 << 0)
#define    TX_NACK      (1 << 1)
#define    RX_DONE      (1 << 2)
#define    STOP_DONE    (1 << 4)

u16 gb_rtl8367_phy_read_register(u16 Register_addr);
void gb_rtl8367_phy_write_register(u16 Register_addr,u16 register_value);

#endif  /* _SMC_91111_H_ */


