/* Mode Limitation */
#define MAX_HResolution		1600
#define MAX_VResolution		1200
 
/* Std. Table Index Definition */
#define TextModeIndex 		0
#define EGAModeIndex 		1
#define VGAModeIndex 		2
#define HiCModeIndex 		3
#define TrueCModeIndex 		4

/* DCLK Index */
#define VCLK25_175     		0x00
#define VCLK28_322     		0x01
#define VCLK31_5       		0x02
#define VCLK36         		0x03
#define VCLK40         		0x04
#define VCLK49_5       		0x05
#define VCLK50         		0x06
#define VCLK56_25      		0x07
#define VCLK65		 	0x08
#define VCLK75	        	0x09
#define VCLK78_75      		0x0A
#define VCLK94_5       		0x0B
#define VCLK108        		0x0C
#define VCLK135        		0x0D
#define VCLK157_5      		0x0E
#define VCLK162        		0x0F

/* Flags Definition */
#define Charx8Dot               0x00000001
#define HalfDCLK                0x00000002
#define DoubleScanMode          0x00000004
#define LineCompareOff          0x00000008
#define SyncPP                  0x00000000
#define SyncPN                  0x00000040
#define SyncNP                  0x00000080
#define SyncNN                  0x000000C0
#define HBorder                 0x00000020
#define VBorder                 0x00000010

/* DAC Definition */
#define DAC_NUM_TEXT		64
#define DAC_NUM_EGA		64
#define DAC_NUM_VGA		256

/* Data Structure decalration for internal use */
typedef struct {
	
    UCHAR ModeString[12];
    int xres;
    int yres;
    int bits_per_pixel;	
    
} OPT_MODE_STRUCT, *POPT_MODE_STRUCT;

typedef struct {
	
    ULONG HT;
    ULONG HDE;
    ULONG HFP;
    ULONG HSYNC;
    ULONG VT;
    ULONG VDE;
    ULONG VFP;
    ULONG VSYNC;
    ULONG DCLKIndex;        
    ULONG Flags;

    ULONG ulRefreshRate;
    ULONG ulRefreshRateIndex;
    ULONG ulModeID;
        
} VBIOS_ENHTABLE_STRUCT, *PVBIOS_ENHTABLE_STRUCT;

typedef struct {
    PVBIOS_ENHTABLE_STRUCT pEnhTableEntry;
    	
} VBIOS_MODE_INFO, *PVBIOS_MODE_INFO;
