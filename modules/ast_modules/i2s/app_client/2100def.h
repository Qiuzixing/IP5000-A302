/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#define Watcom 1
//#define Linux 1
//#define Windows 1

#define    VERSIONSTRING "AST2050/AST2100 video sample program version 0.61.01\n"
#ifdef Windows
#include <wchar.h>
#include <ntddk.h>
#include <ntdef.h>
#endif

#define    FLONG                    unsigned long
#define    BYTE                     unsigned char
#define    INT                      int
#define    VOID                     void
#define    BOOLEAN                  unsigned short

#ifndef Windows
#define    ULONG                    unsigned long
#define    USHORT                   unsigned short
#define    UCHAR                    unsigned char
#define    CHAR                     char
#define    LONG                     long
#define    PUCHAR                   UCHAR *
#define    PULONG                   ULONG *
#endif

#define    PCI                      1
#define    PCIE                     2
#define    AGP                      3
#define    ACTIVE                   4
#define    PASS                     0
#define    TRUE                     0
#define    FALSE                    1
#define    FAIL                     1
#define    FIX                      0
#define    STEP                     1
#define    SEQ_ADDRESS_REGISTER     0x3C4
#define    SEQ_DATA_REGISTER        0x3C5
#define    CRTC_ADDRESS_REGISTER    0x3D4
#define    CRTC_DATA_REGISTER       0x3D5
#define    DAC_INDEX_REGISTER       0x3C8
#define    DAC_DATA_REGISTER        0x3C9
//PCI information define
#define    HOST_BRIDGE_MIN                             0x80000000
#define    HOST_BRIDGE_MAX                             0x80060000
#define    PCI_SLOT_STEP                               0x800
#define    AGP_SLOT_MIN                                0x80010000
#define    AGP_SLOT_MAX                                0x8001F800
#define    PCI_ADDR_PORT                               0x0CF8
#define    PCI_DATA_PORT                               0x0CFC
#define    REG_VENENDER_ID_DEVICE_ID                   0
#define    REG_PCI_STATUS_COMMAND                      0x04
#define    REG_PCI_CLASS_REVISION_ID                   0x08
#define    REG_CAPABILITY_POINTER                      0x34
#define    PhysicalBase_Offset                         0x10
#define    MMIO_Offset                                 0x14
#define    RelocateIO_Offset                           0x18

//  Registers
//#define    VIDEOBASE_OFFSET                            0x10000
#define    SCU_CLOCK_SELECTION_REGISTER                0x1E6E2008
#define    SCU_CLOCK_CONTROL_REGISTER                  0x1E6E200C
#define    VIDEOBASE_OFFSET                            0x1E700000
#define    KEY_CONTROL                                 0x00 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_SEQUENCE_CONTROL                0x04 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_PASS1_CONTROL                   0x08 + VIDEOBASE_OFFSET
#define    VIDEOENGINE_MODEDETECTIONSETTING_H          0x0C + VIDEOBASE_OFFSET
#define    VIDEOENGINE_MODEDETECTIONSETTING_V          0x10 + VIDEOBASE_OFFSET
#define    SCALE_FACTOR_REGISTER                       0x14 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_1                 0x18 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_2                 0x1C + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_3                 0x20 + VIDEOBASE_OFFSET
#define    SCALING_FILTER_PARAMETERS_4                 0x24 + VIDEOBASE_OFFSET
#define    MODEDETECTION_STATUS_READBACK               0x98 + VIDEOBASE_OFFSET
#define    VIDEOPROCESSING_CONTROL                     0x2C + VIDEOBASE_OFFSET
#define    VIDEO_CAPTURE_WINDOW_SETTING                0x30 + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_WINDOW_SETTING               0x34 + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_READ                         0x3C + VIDEOBASE_OFFSET
#define    VIDEO_IN_BUFFER_BASEADDRESS                 0x44 + VIDEOBASE_OFFSET
#define    VIDEO_IN_BUFFER_OFFSET                      0x48 + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_BUFFER_BASEADDRESS             0x4C + VIDEOBASE_OFFSET 
#define    VIDEOCOMPRESS_SOURCE_BUFFER_BASEADDRESS     0x44 + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_OFFSET                         0x48 + VIDEOBASE_OFFSET
#define    VIDEOPROCESS_REFERENCE_BUFFER_BASEADDRESS   0x4C + VIDEOBASE_OFFSET
#define    FLAG_BUFFER_BASEADDRESS                     0x50 + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_DESTINATION_BASEADDRESS      0x54 + VIDEOBASE_OFFSET
#define    STREAM_BUFFER_SIZE_REGISTER                 0x58 + VIDEOBASE_OFFSET
#define    VIDEO_CAPTURE_BOUND_REGISTER                0x5C + VIDEOBASE_OFFSET
#define    VIDEO_COMPRESS_CONTROL                      0x60 + VIDEOBASE_OFFSET
#define    VIDEO_QUANTIZATION_TABLE_REGISTER           0x64 + VIDEOBASE_OFFSET
#define    BLOCK_SHARPNESS_DETECTION_CONTROL           0x6C + VIDEOBASE_OFFSET
#define    POST_WRITE_BUFFER_DRAM_THRESHOLD            0x68 + VIDEOBASE_OFFSET
#define    DETECTION_STATUS_REGISTER                   0x98 + VIDEOBASE_OFFSET
#define    H_DETECTION_STATUS                          0x90 + VIDEOBASE_OFFSET
#define    V_DETECTION_STATUS                          0x94 + VIDEOBASE_OFFSET
#define    VIDEO_CONTROL_REGISTER                      0x300 + VIDEOBASE_OFFSET
#define    VIDEO_INTERRUPT_CONTROL                     0x304 + VIDEOBASE_OFFSET
#define    VIDEO_INTERRUPT_STATUS                      0x308 + VIDEOBASE_OFFSET
#define    MODE_DETECTION_REGISTER                     0x30C + VIDEOBASE_OFFSET

#define    FRONT_BOUND_REGISTER                        0x310 + VIDEOBASE_OFFSET
#define    END_BOUND_REGISTER                          0x314 + VIDEOBASE_OFFSET
#define    CRC_1_REGISTER                              0x320 + VIDEOBASE_OFFSET
#define    CRC_2_REGISTER                              0x324 + VIDEOBASE_OFFSET
#define    REDUCE_BIT_REGISTER                         0x328 + VIDEOBASE_OFFSET
#define    BIOS_SCRATCH_REGISTER                       0x34C + VIDEOBASE_OFFSET
#define    COMPRESS_DATA_COUNT_REGISTER                0x70 + VIDEOBASE_OFFSET
#define    COMPRESS_BLOCK_COUNT_REGISTER               0x74 + VIDEOBASE_OFFSET
#define    VIDEO_SCRATCH_REGISTER_34C                  0x34C + VIDEOBASE_OFFSET
#define    VIDEO_SCRATCH_REGISTER_35C                  0x35C + VIDEOBASE_OFFSET
#define    RC4KEYS_REGISTER                            0x400 + VIDEOBASE_OFFSET
#define    VQHUFFMAN_TABLE_REGISTER                    0x300 + VIDEOBASE_OFFSET



//  Parameters
#define    SAMPLE_RATE                                 24000000.0
#define    MODEDETECTION_VERTICAL_STABLE_MAXIMUM       0x6
#define    MODEDETECTION_HORIZONTAL_STABLE_MAXIMUM     0x6
#define    MODEDETECTION_VERTICAL_STABLE_THRESHOLD     0x2
#define    MODEDETECTION_HORIZONTAL_STABLE_THRESHOLD   0x2
#define    HORIZONTAL_SCALING_FILTER_PARAMETERS_LOW    0xFFFFFFFF
#define    HORIZONTAL_SCALING_FILTER_PARAMETERS_HIGH   0xFFFFFFFF
#define    VIDEO_WRITE_BACK_BUFFER_THRESHOLD_LOW       0x08
#define    VIDEO_WRITE_BACK_BUFFER_THRESHOLD_HIGH      0x04
#define    VQ_Y_LEVELS                                 0x10
#define    VQ_UV_LEVELS                                0x05
#define    EXTERNAL_VIDEO_HSYNC_POLARITY               0x01
#define    EXTERNAL_VIDEO_VSYNC_POLARITY               0x01
#define    VIDEO_SOURCE_FROM                           0x01
#define    EXTERNAL_ANALOG_SOURCE                      0x01
#define    USE_INTERNAL_TIMING_GENERATOR               0x01
#define    WRITE_DATA_FORMAT                           0x00
#define    SET_BCD_TO_WHOLE_FRAME                      0x01
#define    ENABLE_VERTICAL_DOWN_SCALING                0x01
#define    BCD_TOLERENCE                               0xFF
#define    BCD_START_BLOCK_XY                          0x0
#define    BCD_END_BLOCK_XY                            0x3FFF
#define    COLOR_DEPTH                                 16
#define    BLOCK_SHARPNESS_DETECTION_HIGH_THRESHOLD    0xFF
#define    BLOCK_SHARPNESS_DETECTION_LOE_THRESHOLD     0xFF
#define    BLOCK_SHARPNESS_DETECTION_HIGH_COUNTS_THRESHOLD   0x3F
#define    BLOCK_SHARPNESS_DETECTION_LOW_COUNTS_THRESHOLD    0x1F
#define    VQTABLE_AUTO_GENERATE_BY_HARDWARE           0x0
#define    VQTABLE_SELECTION                           0x0
#define    JPEG_COMPRESS_ONLY                          0x0
#define    DUAL_MODE_COMPRESS                          0x1
#define    BSD_H_AND_V                                 0x0
#define    ENABLE_RC4_ENCRYPTION                       0x1
#define    BSD_ENABLE_HIGH_THRESHOLD_CHECK             0x0
#define    VIDEO_PROCESS_AUTO_TRIGGER                  0x0
#define    VIDEO_COMPRESS_AUTO_TRIGGER                 0x0
#define    DIGITAL_SIGNAL			       0x0
#define    ANALOG_SIGNAL			       0x1

//2100 with 128MB
#define    VIDEO_IN_BUFFER_ADDRESS                     0x5E00000
#define    VIDEO_REFERENCE_BUFFER_ADDRESS              0x6800000
#define    VIDEO_COMPRESS_DESTINATION_BUFFER_ADDRESS   0x5A00000
#define    VIDEO_FLAG_BUFFER_ADDRESS                   0x7200000
//2050 with 64MB
/*
#define    VIDEO_IN_BUFFER_ADDRESS                     0x2700000
#define    VIDEO_REFERENCE_BUFFER_ADDRESS              0x2F00000
#define    VIDEO_COMPRESS_DESTINATION_BUFFER_ADDRESS   0x2400000
#define    VIDEO_FLAG_BUFFER_ADDRESS                   0x3700000
*/

/*
#define    ReadMemoryBYTE(baseaddress,offset)        *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    ReadMemoryLong(baseaddress,offset)        *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    ReadMemoryShort(baseaddress,offset)       *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))
#define    WriteMemoryBYTE(baseaddress,offset,data)  *(BYTE *)((ULONG)(baseaddress)+(ULONG)(offset)) = (BYTE)(data)
#define    WriteMemoryLong(baseaddress,offset,data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset))=(ULONG)(data)
#define    WriteMemoryShort(baseaddress,offset,data) *(USHORT *)((ULONG)(baseaddress)+(ULONG)(offset))=(USHORT)(data)
#define    WriteMemoryLongWithANDData(baseaddress, offset, anddata, data)  *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) = *(ULONG *)((ULONG)(baseaddress)+(ULONG)(offset)) & (ULONG)(anddata) | (ULONG)(data)
*/
#define    intfunc      int386

#ifndef  Linux
#define    outdwport         outpd
#define    indwport          inpd
#define    outport           outp
#define    inport            inp
#endif

#ifdef   Linux
#include <asm/io.h>
#include <linux/pci_ids.h>
#define    u8                unsigned char
#define    u16               unsigned short
#define    u32               unsigned int

#define    outdwport(p,v)    outl((u32)(v),(u16)(p))
#define    indwport(p)       inl((u16)(p))
#define    outport(p,v)      outb((u8)(v),(u16)(p))
#define    inport(p)         inb((u16)(p))
#endif

#ifndef  _STRUCTURE_INFO
#define  _STRUCTURE_INFO

//Buffer structure
typedef struct _XMS_BUFFER
{
    ULONG     handle;
    ULONG     size;
    ULONG     physical_address;
    ULONG     virtual_address;
} XMS_BUFFER, *PXMS_BUFFER;

//PCI info structure
typedef struct
{
    USHORT    usVendorID;
    USHORT    usDeviceID;
    ULONG     ulPCIConfigurationBaseAddress;
    BYTE      jAGPStatusPort;
    BYTE      jAGPCommandPort;
    BYTE      jAGPVersion;
} PCI_INFO;

typedef struct _DEVICE_PCI_INFO
{
    USHORT    usVendorID;
    USHORT    usDeviceID;
    ULONG     ulPCIConfigurationBaseAddress;
    ULONG     ulPhysicalBaseAddress;
    ULONG     ulMMIOBaseAddress;
    USHORT    usRelocateIO;
} DEVICE_PCI_INFO, *PDEVICE_PCI_INFO;

typedef struct _VIDEO_MODE_INFO
{
    USHORT    X;
    USHORT    Y;
    USHORT    ColorDepth;
    USHORT    RefreshRate;
    BYTE      ModeIndex;
} VIDEO_MODE_INFO, *PVIDEO_MODE_INFO;

typedef struct _VQ_INFO {
    BYTE    Y[16];
    BYTE    U[32];
    BYTE    V[32];
    BYTE    NumberOfY;
    BYTE    NumberOfUV;
    BYTE    NumberOfInner;
    BYTE    NumberOfOuter;
} VQ_INFO, *PVQ_INFO;

typedef struct _HUFFMAN_TABLE {
    ULONG  HuffmanCode[32];
} HUFFMAN_TABLE, *PHUFFMAN_TABLE;

typedef struct _FRAME_HEADER {
    ULONG     StartCode;
    ULONG     FrameNumber;
    USHORT    HSize;
    USHORT    VSize;
    ULONG     Reserved[2];
    BYTE      CompressionMode;
    BYTE      JPEGScaleFactor;
    BYTE      Y_JPEGTableSelector;
    BYTE      UV_JPEGTableSelector;
    BYTE      JPEGYUVTableMapping;
    BYTE      SharpModeSelection;
    BYTE      AdvanceTableSelector;
    BYTE      AdvanceScaleFactor;
    ULONG     NumberOfMB;
    BYTE      VQ_YLevel;
    BYTE      VQ_UVLevel;
    VQ_INFO   VQVectors;
    BYTE      RC4Enable;
    BYTE      RC4Reset;
    BYTE      Mode420;
} FRAME_HEADER, *PFRAME_HEADER;

typedef struct _INF_DATA {
    BYTE    DownScalingEnable;
    BYTE    DifferentialSetting;
    USHORT  AnalogDifferentialThreshold;
    USHORT  DigitalDifferentialThreshold;
    BYTE    ExternalSignalEnable;
    BYTE    AutoMode;
    BYTE    DirectMode;
    BYTE    AutoFlip;
    USHORT  DelayControl;
    BYTE    VQMode;
} INF_DATA, *PINF_DATA;

typedef struct _TIME_DATA {
    ULONG    UnitTimeLow;
    ULONG    UnitTimeHigh;
    ULONG    StartTimeLow;
    ULONG    StartTimeHigh;
    ULONG    TimeUsed;
} TIME_DATA, *PTIME_DATA;

typedef struct _COMPRESS_DATA {
    ULONG   SourceFrameSize;
    ULONG   CompressSize;
    ULONG   HDebug;
    ULONG   VDebug;
} COMPRESS_DATA, *PCOMPRESS_DATA;

//VIDEO Engine Info
typedef struct _VIDEO_ENGINE_INFO {
    USHORT             iEngVersion;
    DEVICE_PCI_INFO    VGAPCIInfo;
    XMS_BUFFER         VideoINBuffer;
    XMS_BUFFER         VideoProcessBuffer;
    XMS_BUFFER         VideoProcessReferenceBuffer;
    XMS_BUFFER         FlagBuffer;
    XMS_BUFFER         VideoCompressBuffer;
    XMS_BUFFER         VideoOutputBuffer;
    VIDEO_MODE_INFO    SourceModeInfo;
    VIDEO_MODE_INFO    DestinationModeInfo;
    VQ_INFO            VQInfo;
    FRAME_HEADER       FrameHeader;
    INF_DATA           INFData;
    COMPRESS_DATA      CompressData;
    TIME_DATA          TimeData;
    BYTE               InputSignal;
    BYTE               ChipVersion;
    BYTE               NoSignal;
    USHORT             MemoryBandwidth;
    ULONG              ChipBounding;
    ULONG              TotalMemory;
    ULONG              VGAMemory;
} VIDEO_ENGINE_INFO, *PVIDEO_ENGINE_INFO;

typedef struct {
    USHORT    HorizontalTotal;
    USHORT    VerticalTotal;
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    BYTE      RefreshRate;
    double    HorizontalFrequency;
    USHORT    HSyncTime;
    USHORT    HBackPorch;
    USHORT    VSyncTime;
    USHORT    VBackPorch;
    USHORT    HLeftBorder;
    USHORT    HRightBorder;
    USHORT    VBottomBorder;
    USHORT    VTopBorder;
} VESA_MODE;

typedef struct {
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    USHORT    RefreshRate;
    BYTE      ADCIndex1;
    BYTE      ADCIndex2;
    BYTE      ADCIndex3;
    BYTE      ADCIndex5;
    BYTE      ADCIndex6;
    BYTE      ADCIndex7;
    BYTE      ADCIndex8;
    BYTE      ADCIndex9;
    BYTE      ADCIndexA;
    BYTE      ADCIndexF;
    BYTE      ADCIndex15;
    int       HorizontalShift;
    int       VerticalShift;
} ADC_MODE;


typedef struct {
    USHORT    HorizontalActive;
    USHORT    VerticalActive;
    USHORT    RefreshRateIndex;
    double    PixelClock;
} INTERNAL_MODE;

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

struct RGB {
	unsigned char B;
	unsigned char G;
	unsigned char R;
	unsigned char Reserved;
};

struct YUV444 {
	unsigned char U;
	unsigned char Y;
	unsigned char V;
};

struct YUV422 {
	unsigned char Y0;
	unsigned char U;
	unsigned char Y1;
	unsigned char V;
};

//  RC4 structure
struct rc4_state
{
    int x;
    int y;
    int m[256];
};

//  RC4 keys. Current keys are fedcba98765432210
unsigned char EncodeKeys[256];
unsigned char DecodeKeys[256] = "fedcba9876543210";

#endif
