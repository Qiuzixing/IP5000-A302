/******************************************************************************
 * Mode Stuff
 ******************************************************************************/
/* Default Settings */
#define CRT_LOW_THRESHOLD_VALUE         0x12
#define CRT_HIGH_THRESHOLD_VALUE        0x1E

/* Output Selection */
#define CRT1				0x00
#define CRT2				0x01
#define DVI1				0x10
#define DVI2				0x11
#define LVDS1				0x20
#define LVDS2				0x21

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
#define VCLK119        		0x10

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
#define COLORINDEX		0x00000000
#define MONOINDEX		0x00000100

/* DAC Definition */
#define DAC_NUM_TEXT		64
#define DAC_NUM_EGA		64
#define DAC_NUM_VGA		256

/* AST1500 Reg. Definition */
#define	AST1500_VGAREG_BASE	0x1e6e6000
#define AST1500_VGA1_CTLREG	0x00
#define AST1500_VGA1_CTLREG2	0x04
#define AST1500_VGA1_STATUSREG	0x08 
#define AST1500_VGA1_PLL	0x0C
#define AST1500_VGA1_HTREG	0x10
#define AST1500_VGA1_HRREG	0x14
#define AST1500_VGA1_VTREG	0x18
#define AST1500_VGA1_VRREG	0x1C
#define AST1500_VGA1_STARTADDR	0x20
#define AST1500_VGA1_OFFSETREG	0x24
#define AST1500_VGA1_THRESHOLD	0x28
#define AST1500_HWC1_OFFSET     0x30
#define AST1500_HWC1_XY		0x34
#define AST1500_HWC1_PBase      0x38
#define AST1500_OSD1_H          0x40
#define AST1500_OSD1_V          0x44
#define AST1500_OSD1_PBase      0x48
#define AST1500_OSD1_Offset     0x4C
#define AST1500_OSD1_THRESHOLD  0x50

#define AST1500_VGA2_CTLREG	0x60
#define AST1500_VGA2_CTLREG2	0x64
#define AST1500_VGA2_STATUSREG	0x68 
#define AST1500_VGA2_PLL	0x6C
#define AST1500_VGA2_HTREG	0x70
#define AST1500_VGA2_HRREG	0x74
#define AST1500_VGA2_VTREG	0x78
#define AST1500_VGA2_VRREG	0x7C
#define AST1500_VGA2_STARTADDR	0x80
#define AST1500_VGA2_OFFSETREG	0x84
#define AST1500_VGA2_THRESHOLD	0x88
#define AST1500_HWC2_OFFSET     0x90
#define AST1500_HWC2_XY		0x94
#define AST1500_HWC2_PBase      0x98
#define AST1500_OSD2_H          0xA0
#define AST1500_OSD2_V          0xA4
#define AST1500_OSD2_PBase      0xA8
#define AST1500_OSD2_Offset     0xAC
#define AST1500_OSD2_THRESHOLD  0xB0

/* Data Structure */
typedef struct {
    UCHAR ModeName[20];
    USHORT usModeIndex;
    USHORT usModeID;
    USHORT usColorIndex;
    USHORT usRefreshRateIndex;
    USHORT usWidth;
    USHORT usHeight;
    USHORT usBitsPerPlane;
    USHORT usRefreshRate;    
} ModeInfoStruct;

typedef struct {
	
    UCHAR MISC;	
    UCHAR SEQ[4];
    UCHAR CRTC[25];
    UCHAR AR[20];	    
    UCHAR GR[9];
    
} VBIOS_STDTABLE_STRUCT, *PVBIOS_STDTABLE_STRUCT;

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
    UCHAR Param1;
    UCHAR Param2;
    UCHAR Param3;	
} VBIOS_DCLK_INFO, *PVBIOS_DCLK_INFO;

typedef struct {
    UCHAR DACR;
    UCHAR DACG;
    UCHAR DACB;	
} VBIOS_DAC_INFO, *PVBIOS_DAC_INFO;

typedef struct {
    PVBIOS_STDTABLE_STRUCT pStdTableEntry;
    PVBIOS_ENHTABLE_STRUCT pEnhTableEntry;
    	
} VBIOS_MODE_INFO, *PVBIOS_MODE_INFO;
