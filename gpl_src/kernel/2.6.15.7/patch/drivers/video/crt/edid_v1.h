#ifndef _EDID_PARSER_V1_H_
#define _EDID_PARSER_V1_H_

#define REPEATER_EDID_PA 0

#define DISPLAY_PARA_INPUT		0x14
	#define	DIGITAL_INPUT		(1<<7)
	/* suggest value */
	#define	DIGITAL_VALUE		0x80
	#define	ANALOG_VALUE		0x6A	/* video level bit5-6 11(0.7, 0), bit3 separate sync, bit 1 sync on green */

#define STD_TIMING_1			0x26
#define	STD_TIMING_GRAP			2
#define	STD_TIMING_GROUP		8
	#define	NO_USED_VALUE		0x01

#define	TIMING_DES_1			0x36
#define	TIMING_DES_GRAP			18
#define	TIMING_DES_GROUP		4
	#define	RANGE_DES			0xFD	/* 00h, 00h, 00h <Tag#>, 00h */

#define	PIXEL_CLK_LSB			0x36
#define PIXEL_CLK_MSB			0x37
#define	HOR_ACT_LSB				0x38
#define	HOR_BLANK_LSB			0x39
#define	HOR_MIX_MSB				0x3A
	#define HOR_ACT_MSB_MASK	0xF0
	#define HOR_ACT_MSB_BIT		4
	#define HOR_BLANK_MSB_MASK	0x0F
	#define HOR_BLANK_MSB_BIT	0
#define	VER_ACT_LSB				0x3B
#define	VER_BLANK_LSB			0x3C
#define	VER_MIX_MSB				0x3D
	#define VER_ACT_MSB_MASK	0xF0
	#define VER_ACT_MSB_BIT		4
	#define VER_BLANK_MSB_MASK	0x0F
	#define VER_BLANK_MSB_BIT	0
#define	HOR_SYNC_OFFSET_LSB		0x3E
#define	HOR_SYNC_WIDTH_LSB		0x3F
#define	VER_SYNC_MIX_LSB		0x40
	#define VER_SYNC_OFFSET_LSB_MASK	0xF0
	#define VER_SYNC_OFFSET_LSB_BIT		4
	#define VER_SYNC_WIDTH_LSB_MASK		0x0F
	#define VER_SYNC_WIDTH_LSB_BIT		0
#define ALL_SYNC_MIX_MSB		0x41
	#define HOR_SYNC_OFFSET_MSB_MASK	(3<<6)
	#define HOR_SYNC_OFFSET_MSB_BIT		6
	#define HOR_SYNC_WIDTH_MSB			(3<<4)
	#define HOR_SYNC_WIDTH_MSB_BIT		4
	#define VER_SYNC_OFFSET_MSB_MASK	(3<<2)
	#define VER_SYNC_OFFSET_MSB_BIT		2
	#define VER_SYNC_WIDTH_MSB			(3<<0)
	#define VER_SYNC_WIDTH_MSB_BIT		0
#define HOR_BORDER				0x45
#define VER_BORDER				0x46
#define	SYNC_MISC				0x47
	#define	VER_POLARITY_MASK	(1<<2)
	#define	VER_POLARITY_BIT	2
	#define	HOR_POLARITY_MASK	(1<<1)
	#define	HOR_POLARITY_BIT	1
	#define	SYNC_MISC_VALUE		0x18	/* NO INterlaced bit7(0), no Stereo bit6-5(00), Digital Seperate bit4-3(11), Stereo bit0(0) */

/* DTD is offset */
#define	DTD_FLAG				17
	#define	INTERLACED_MASK		(1UL<<7)
	#define	NON_INTERLACED		0
	#define INTERLACED			(1UL<<7)

#define DVI_INPUT		0
#define ANALOG_INPUT	1

typedef union {
	unsigned char raw_byte[1];
	struct {
		/* From CEA-861B spec. The native_num should be:
		** Total number of native (preferred) formats described by
		** 18 byte detailed timing descriptors in entire E-EDID structure.
		*/
		unsigned char native_num:4;
		unsigned char YCbCr422:1;
		unsigned char YCbCr444:1;
		unsigned char basic_audio:1;
		unsigned char under_scan:1;
	} __attribute__ ((packed)) bits;
}  __attribute__ ((packed)) CEAHDR_B3, *PCEAHDR_B3;

#define DBTYPE_AUDIO 1  /* SAD */
#define DBTYPE_VIDEO 2  /* SVD */
#define DBTYPE_VENDOR 3  /* VSD */
#define DBTYPE_SPEAKER 4 /* SPKR */
#define DBTYPE_EXTENDED 7 /* extended tag */

/* extended tag codes */
#define EXTENDED_DBTYPE_COLORIMETRY 5 /* colorimetry */

/* Data Block Header */
typedef union {
	unsigned char raw_byte[1];
	struct {
		unsigned char size:5;
		unsigned char type:3;
	}  __attribute__ ((packed)) bits;
}  __attribute__ ((packed)) DBHDR;

/* 3-byte Short Audio Descriptors (SADs) */
typedef union {
	unsigned char raw_byte[3];
	struct 	{
		/* Byte 1 (format and number of channels) */
		unsigned char ch_num:3;
		unsigned char format:4;
		unsigned char reserved0: 1;
		/* Byte 2 (sampling frequencies supported) */
		unsigned char KHz32:1;
		unsigned char KHz44:1;
		unsigned char KHz48:1;
		unsigned char KHz88:1;
		unsigned char KHz96:1;
		unsigned char KHz176:1;
		unsigned char KHz192:1;
		unsigned char reserved1:1;
		/* Byte 3 (bitrate) */
		/* For LPCM, bits 7:3 are reserved and the remaining bits define bit depth
		 * For all other sound formats, bits 7..0 designate the maximium
		 * supported bitrate divided by 8kHz.
		 */
		unsigned char bit16:1;
		unsigned char bit20:1;
		unsigned char bit24:1;
		unsigned char reserved2:5;
	} __attribute__ ((packed)) bits;
} __attribute__ ((packed)) SAD, *PSAD;

/* 1-byte Short Video Descriptors (SVDs) */
typedef union {
	unsigned char raw_byte[1];
	/* 1-byte Short Video Descriptors (SVDs) */
	struct {
		unsigned char index:7;
		unsigned char native:1;
	} __attribute__ ((packed)) bits;
}  __attribute__ ((packed)) SVD, *PSVD;

/* Vendor Specific Data Block */
/* This byte is not necessary byte#13. */
typedef struct {
	unsigned char rsvd:3;
	unsigned char image_size:2;
	unsigned char _3d_multi_present:2;
	unsigned char _3d_present:1;
} __attribute__ ((packed)) HDMI_VSDB_BYTE13, *PHDMI_VSDB_BYTE13;

/* This byte is not necessary byte#14. */
typedef struct {
	unsigned char hdmi_3d_len:5;
	unsigned char hdmi_vic_len:3;
} __attribute__ ((packed)) HDMI_VSDB_BYTE14, *PHDMI_VSDB_BYTE14;

typedef struct {
	unsigned char id0; /* 0x03 */
	unsigned char id1; /* 0x0C */
	unsigned char id2; /* 0x00 */
	unsigned short int src_phy_addr; /* LSB first for CEC */
	unsigned char byte_block_6;
	unsigned char byte_block_7; /* Max_TMDS_Clock */
	/* byte_block_8 */
	unsigned char cnc0:1; /* HDMI 1.4a CNC0 */
	unsigned char cnc1:1; /* HDMI 1.4a CNC0 */
	unsigned char cnc2:1; /* HDMI 1.4a CNC0 */
	unsigned char cnc3:1; /* HDMI 1.4a CNC0 */
	unsigned char reserved0:1; /* HDMI 1.4a */
	unsigned char HDMI_video_present:1; /* HDMI 1.4a */
	unsigned char I_Latency_Fields_present:1; /* HDMI 1.3a */
	unsigned char Latency_Fields_present:1; /* HDMI 1.3a */
	/* byte block 9~12 are optional */
#if 0
	unsigned char Video_Latency;
	unsigned char Audio_Latency;
	unsigned char Interlaced_Video_Latency;
	unsigned char Interlaced_Audio_Latency;
#endif
	/* Additional bytes may be present, but the HDMI spec 1.3a says they shall be zero.
	 * Additional bytes may contain 3D information per HDMI spec 1.4a
	 */
} __attribute__ ((packed)) VSD, *PVSD;

/* 3-bytes Speaker Allocation Data Block */
typedef union {
	unsigned char raw_byte[3];
	struct {
		/* byte 1 */
		unsigned char FL_FR:1; /* bit 0: Front Left / Front Right present for 1, absent for 0 */
		unsigned char LFE:1; /* bit 1: LFE present for 1, absent for 0 */
		unsigned char FC:1; /* bit 2: Front Center present for 1, absent for 0 */
		unsigned char RL_RR:1; /* bit 3: Rear Left / Rear Right present for 1, absent for 0 */
		unsigned char RC:1; /*  bit 4: Rear Center present for 1, absent for 0 */
		unsigned char FLC_FRC:1; /* bit 5: Front Left Center / Front Right Center present for 1, absent for 0 */
		unsigned char RLC_RRC:1; /* bit 6: Rear Left Center / Rear Right Center present for 1, absent for 0 */
		unsigned char Reserved0:1;
		/*  byte 2 */
		unsigned char Reserved1;
		/*  byte 3 */
		unsigned char Reserved2;
	} __attribute__ ((packed)) bits;
} __attribute__ ((packed)) SPKR;

/*
** Detailed Timings Description
** "d": byte (designated in byte 02) where DTDs begin.  18-byte DTD strings continue for an unspecified
** length (modulo 18) until a "00 00" is as the first bytes of a prospective DTD.  At this point,
** the DTDs are known to be complete, and the start address of the "00 00" can be considered to be "XX"
** (see below)
** "XX"-126: Post-DTD padding.  Should be populated with 00h
*/
typedef union {
	unsigned char raw_byte[18]; /* could be multiple of raw_byte[18]. { 0x00, 0x00, 0x00, 0x10 } for Unused */
	struct { /* For the case that pixel_clk != 0 */
		/*  54-55  */
		unsigned short PixelClk;
		/*  56 */
		unsigned char HA;
		/*  57 */
		unsigned char HBlank;
		/*  58 */
		unsigned char H_highbits;
		/*  59 */
		unsigned char VA;
		/*  60 */
		unsigned char VBlank;
		/*  61 */
		unsigned char V_highbits;
		/*  62 */
		unsigned char HSyncOffset;
		/*  63 */
		unsigned char HSyncWidth;
		/*  64 */
		unsigned char VSyncOffsetWidth;
		/*  65 */
		unsigned char Sync_highbits;
		/*  66 */
		unsigned char HImgSize;
		/*  67 */
		unsigned char VImgSize;
		/*  68 */
		unsigned char ImgSize_highbits;
		/*  69 */
		unsigned char H_Border;
		/*  70 */
		unsigned char V_Border;
		/*  71 */
		unsigned char StereoMode:1;
		unsigned char HSyncPolarity:1;
		unsigned char VSyncPolarity:1;
		unsigned char SepSync:2;
		unsigned char Stereo:2;
		unsigned char Interlace:1;
	} __attribute__ ((packed)) bits;
} __attribute__ ((packed)) DTD, *PDTD;

/*  Base EDID Format */
typedef struct {
	/* 00-07 */
	unsigned char header[8]; /* MUST be "00h FFh FFh FFh FFh FFh FFh 00h" */
	/* 08-09 */
	unsigned short ManufacturerId;
	/*  10-11 */
	unsigned short ProductId;
	/*  12-15 */
	unsigned int Serial;
	/*  16 */
	unsigned char ManuWeek;
	/*  17 */
	unsigned char ManuYear; /* Add 1990 to the value for actual year */
	/*  18 */
	unsigned char version; /*  1 for EDID 1.3 */
	/*  19 */
	unsigned char revision; /*  3 for EDID 1.3 */
	/*  20-24 */
	unsigned char basic_display_parms[5];
	/*  25-34 */
	unsigned char chrom_coordinate[10]; /* Chromaticity coordinates */
	/*  35 */
	unsigned char establish_timing1;
	/*  36 */
	unsigned char establish_timing2;
	/*  37 */
	unsigned char reserved_timing; /* Manufacturer's reserved timing */
	/*  38-53 */
	unsigned char standard_timing_id[16]; /* Standard timing identification */
	/*  54-71, 72-89, 90-107, 108-125 */
	DTD desc_block[4]; /* Descriptor Block 1,2,3,4 */
	/*  126 */
	unsigned char extension_num; /* Extension Flag */
	/*  127 */
	unsigned char checksum;
	/*  128 */
	unsigned char ext_block[];
} __attribute__ ((packed)) *PBASE_EDID;

/*  CEA Timing Extension Format */
typedef struct {
	unsigned char tag; /* MUST be 0x02 */
	unsigned char revision; /*  Should be 0x03 for revision 3 (E-EDID 1.3) */
	/* dtd_offset:
	** If no non-DTD data is present in this extension block,
	** the value should be set to 04h (the byte after next).
	** If set to 00h, there are no DTDs present in this block and no non-DTD data.
	*/
	unsigned char dtd_offset;
	CEAHDR_B3 monitor_support;
	unsigned char data_block_start[];
} __attribute__ ((packed)) CEAExtHdr, *PCEAExtHdr;

/*  Data Block Collection Format */
typedef struct {
	/*  Data Block Header */
	DBHDR hdr;
	/*  Different types of Data Blocks */
	union {
		/* 3-byte Short Audio Descriptors (SADs) */
		SAD sad;
		/*  1-byte Short Video Descriptors (SVDs) */
		SVD svd;
		/*  Vendor Specific Data Block */
		VSD vsd;
		/*  Speaker Allocation Data Block */
		SPKR speaker;
	}  __attribute__ ((packed)) data;
} __attribute__ ((packed)) *PData_Block;
#endif /* #ifndef _EDID_PARSER_V1_H_ */
