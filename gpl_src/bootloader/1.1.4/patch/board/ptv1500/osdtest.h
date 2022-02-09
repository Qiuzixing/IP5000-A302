/* Data Define */
#ifndef UCHAR
#define UCHAR  unsigned char
#endif

#ifndef ULONG
#define ULONG unsigned long int
#endif

#ifndef BOOL
#define BOOL  int
#endif

#ifndef	USHORT
#define USHORT unsigned short
#endif

#ifndef TRUE
#define	TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

/* Options */
#define DRAM_BASE			0x40000000
#define VRAM_OFFSET			0x0F800000
#define VRAM_BASE			(DRAM_BASE + VRAM_OFFSET)
#define OSD_OFFSET			0x0F700000
#define OSD_BASE			(DRAM_BASE + 0x0F700000)

typedef struct {
    UCHAR R;
    UCHAR G;
    UCHAR B;    	
} _OSDPalette;

typedef struct {

    UCHAR  ModeInfoStr[20];
    USHORT usWhichOne;
    ULONG  ulPattern[4];	
    ULONG  X;
    ULONG  Y;
    ULONG  CRC;

} _OSDInfo;
