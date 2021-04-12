#ifndef _EDID_PARSER_V2_H_
#define _EDID_PARSER_V2_H_
#include <linux/list.h>
#include <asm/arch/drivers/video_hal.h>

#define DRM_EDID_PT_HSYNC_POSITIVE (1 << 1)
#define DRM_EDID_PT_VSYNC_POSITIVE (1 << 2)
#define DRM_EDID_PT_SEPARATE_SYNC  (3 << 3)
#define DRM_EDID_PT_STEREO         (1 << 5)
#define DRM_EDID_PT_INTERLACED     (1 << 7)

#define DRM_EDID_INPUT_SERRATION_VSYNC (1 << 0)
#define DRM_EDID_INPUT_SYNC_ON_GREEN   (1 << 1)
#define DRM_EDID_INPUT_COMPOSITE_SYNC  (1 << 2)
#define DRM_EDID_INPUT_SEPARATE_SYNCS  (1 << 3)
#define DRM_EDID_INPUT_BLANK_TO_BLACK  (1 << 4)
#define DRM_EDID_INPUT_VIDEO_LEVEL     (3 << 5)
#define DRM_EDID_INPUT_DIGITAL         (1 << 7)

#define DRM_EDID_FEATURE_DEFAULT_GTF      (1 << 0)
#define DRM_EDID_FEATURE_PREFERRED_TIMING (1 << 1)
#define DRM_EDID_FEATURE_STANDARD_COLOR   (1 << 2)
/* If analog */
#define DRM_EDID_FEATURE_DISPLAY_TYPE     (3 << 3) /* 00=mono, 01=rgb, 10=non-rgb, 11=unknown */
/* If digital */
#define DRM_EDID_FEATURE_COLOR_MASK	  (3 << 3)
#define DRM_EDID_FEATURE_RGB		  (0 << 3)
#define DRM_EDID_FEATURE_RGB_YCRCB444	  (1 << 3)
#define DRM_EDID_FEATURE_RGB_YCRCB422	  (2 << 3)
#define DRM_EDID_FEATURE_RGB_YCRCB	  (3 << 3) /* both 4:4:4 and 4:2:2 */

/* 3-byte Short Audio Descriptors (SADs) */
typedef union {
	unsigned char raw_byte[3];
	struct {
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
		/*
		 * Byte 3 (bitrate)
		 * For LPCM, bits 7:3 are reserved and the remaining bits define bit depth
		 * For all other sound formats, bits 7..0 designate the maximium
		 * supported bitrate divided by 8kHz.
		 */
		unsigned char bit16:1;
		unsigned char bit20:1;
		unsigned char bit24:1;
		unsigned char reserved2:5;
	} __attribute__ ((packed)) bits;

} __attribute__ ((packed)) SAD, *PSAD;

#define AST_SAD(type, chs, freq, byte2) \
	.raw_byte[0] = (type << 3) | (chs - 1), .raw_byte[1] = freq & 0x7F, .raw_byte[2] = byte2,

#endif /* #ifndef _EDID_PARSER_V2_H_ */
