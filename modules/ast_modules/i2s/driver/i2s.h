/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#ifndef _I2S_H_
#define _I2S_H_

//I2S Buffer/Descriptor base address and Paramemters
//#define    TX_SOURCE_ADDRESS                0x00000000
//#define    TX_DESCRIPTOR_ADDRESS            0x1000000
//#define    RX_SOURCE_ADDRESS                0x00000000
//#define    RX_DESCRIPTOR_ADDRESS            0x2000000
//#define	   OUTL(unsigned long addr, unsigned long data)		*(phys_to_virt(addr)) = data;
//#define	   INL(unsigned long addr)		*(phys_to_virt(addr));
//#include "../../cat6023/cat6023.h"

#define    RX_SIZE_TO_PACKET_NUM(s) 		((s) >> rx_buf_size_shift)
#define    TX_SIZE_TO_PACKET_NUM(s) 		((s) >> tx_buf_size_shift)
#define    MAX_RX_SIZE		               (max_udp_xfer_size) // MUST < 64KB, limited by UDP
//#define TX_DESC_GUARD_BAND 1
//#define MASK_TX_DESC_NUM (NUM_TX_DESCRIPTOR - 1)
#define MASK_TX_DESC_NUM (tx_buf_num - 1)


//I2S Descriptor
#define    OWNER                                  (1 << 31)
#define    SW_OWN                                 0
#define    ENGINE_OWN                             1
#define    END_OF_RING                            (1 << 30)

static inline void udump(void *buf, int bufflen)
{
    int i;
    //unsigned char *buff= buf;
    unsigned int *buff= buf;

    printk("dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

    /*if (bufflen > 768) {
        for (i = 0; i< 768; i++) {
            if (i%16 == 0)
                printk("   ");
            printk("%02x ", (unsigned char ) buff[i]);
            if (i%4 == 3) printk("| ");
            if (i%16 == 15) printk("\n");
        }
        printk("... (%d byte)\n", bufflen);
        return;
    }*/
	bufflen = (bufflen + 3) >>2;

    for (i = 0; i< bufflen; i++) {
//        if (i%4 == 0)
//            printk("   ");
//        if (i%4 == 3)
//            printk("| ");
        if (i%4 == 3)
			printk("%08x\n", (unsigned int) buff[i]); 

		else
			printk("%08x ", (unsigned int) buff[i]);    
	}
    //printk("\n");

}


#define read_register(address)	*(volatile u32 *)(IO_ADDRESS((u32)(address)))
#define write_register(address, data)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)(data)
#define write_register_or(address, data)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)(read_register(address) | (data))
#define write_register_and(address, data, anddata)	*(volatile u32 *)(IO_ADDRESS((u32)(address))) = (u32)((read_register(address) & (~(anddata))) |(data))

typedef struct
{
	void *desc_va;
	u32 desc_pa;
	unsigned int buf_size;
	unsigned int buf_num;
	void **buf_va;
	u32 *buf_pa;
} Audio_Buf;


#if 0
typedef struct _TRANSFER_HEADER {
    ULONG     Data_Length;
    ULONG     Blocks_Changed;
    USHORT    User_Width;
    USHORT    User_Height;
    USHORT    Source_Width;
    USHORT    Source_Height;
    BYTE      RC4_Enable;
    BYTE      RC4_Reset;
    BYTE      Y_Table;
    BYTE      UV_Table;
    BYTE      Mode_420;
    BYTE      Direct_Mode;
    BYTE      VQ_Mode;
    BYTE      Disable_VGA;
    BYTE      Differential_Enable;
    BYTE      Auto_Mode;
    BYTE      VGA_Status;
    BYTE      RC4State;
} TRANSFER_HEADER, *PTRANSFER_HEADER;

//AUDIO Engine Info
typedef struct _AUDIO_ENGINE_INFO {
    USHORT             iEngVersion;
    BYTE               InputSignal;
    BYTE               ChipVersion;
    BYTE               NoSignal;
    USHORT             MemoryBandwidth;
    ULONG              ChipBounding;
    ULONG              TotalMemory;
    ULONG              VGAMemory;
} AUDIO_ENGINE_INFO, *PAUDIO_ENGINE_INFO;
#endif

/*
typedef struct
{
	u8 numerator:8; //0-7
	u8 denumerator:5; //8-12
	u8 output_divider:2; //13-14
	u8 post_divider:2; //15-16
	u8 pll_turn_off:1; //17
	u8 bypass_mode:1; //18
	u16 reserved:13; //19-31
} __attribute__ ((packed)) SCU1C_REG_PARAM;
*/
#endif
