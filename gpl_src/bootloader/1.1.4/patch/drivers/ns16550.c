/*
 * COM1 NS16550 support
 * originally from linux source (arch/ppc/boot/ns16550.c)
 * modified to use CFG_ISA_MEM and new defines
 */

#include <config.h>

#ifdef CFG_NS16550

#include <ns16550.h>

#define LCRVAL LCR_8N1					    /* 8 data, 1 stop, no parity */
#define MCRVAL (MCR_DTR | MCR_RTS)			/* RTS/DTR */
#define FCRVAL (FCR_FIFO_EN | FCR_RXSR | FCR_TXSR )	/* Clear & enable FIFOs */

void NS16550_init (NS16550_t com_port, int baud_divisor)
{
//	volatile int* p=(char*)0x1e840000;   // COM1

    // +C.C.Chen change console from COM1 to COM2
//    volatile int* p=(char*)0x1e860000; // COM2a
    volatile int* p=(char*)com_port; // COM2a
    
	*(p+1) = 0x0;
	*(p+3) = LCR_BKSE | LCRVAL;
	/* MSB first */
	*(p+1) = (baud_divisor >> 8) & 0xff;
	*p = baud_divisor & 0xff;
	*(p+3) = LCRVAL;
	*(p+4) = MCRVAL;
	*(p+2) = FCRVAL;

}

void NS16550_reinit (NS16550_t com_port, int baud_divisor)
{
//	volatile int* p=(char*)0x1e840000;  // COM1

    // +C.C.Chen change console from COM1 to COM2
//    volatile int* p=(char*)0x1e860000; // COM2
    volatile int* p=(char*)com_port; // COM2
    
	*(p+1) = 0x0;
	*(p+3) = LCR_BKSE;
	/* MSB first */
	*(p+1) = (baud_divisor >> 8) & 0xff;
	*p = baud_divisor & 0xff;
	*(p+3) = LCRVAL;
	*(p+4) = MCRVAL;
	*(p+2) = FCRVAL;
}

void NS16550_putc (NS16550_t com_port, char c)
{
//	volatile char* p=(char*)0x1e840000; // COM1
    // +C.C.Chen change console from COM1 to COM2
//    volatile char* p=(char*)0x1e860000; // COM2
    volatile char* p=(char*)com_port; // COM2

	while ((*(p+0x14) & LSR_THRE) == 0)
		;

	*p = c;
}

char NS16550_getc (NS16550_t com_port)
{
//	volatile char* p=(char*)0x1e840000; // COM1
    // +C.C.Chen change console from COM1 to COM2
//    volatile char* p=(char*)0x1e860000; // COM2
    volatile char* p=(char*)com_port; // COM2

	while ((*(p+0x14) & LSR_DR) == 0) 
		;
	return (*p);
}

int NS16550_tstc (NS16550_t com_port)
{
	//	char* p=(char*)0x1e840000;  // COM1
    // +C.C.Chen change console from COM1 to COM2
//    char* p=(char*)0x1e860000; // COM2
    char* p=(char*)com_port; // COM2
    
	return ((*(p+0x14) & LSR_DR) != 0);
}

#endif
