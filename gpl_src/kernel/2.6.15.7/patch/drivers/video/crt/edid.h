#ifndef _EDID_H_
#define _EDID_H_

#include <linux/types.h>    /* size_t */
#include <linux/module.h>

#define AST_EDID_PARSER_VER	2

#if (AST_EDID_PARSER_VER == 1)
#include "edid_v1.h"
#else
#include "edid_v2.h"
#endif

/* */
#define I2C_DDC_EDID_ADDR	0xA0

/* */
#define EDID_ROM_SIZE	256
#define EDID_BLOCK_SIZE	128

#define	EXT_EDID_BLOCK		0x7E
#define EDID_CHECKSUM_OFFSET	0x7F

/* extend edid */
#define EXT_EDID_OFFSET		128
#define EXT_TAG			(0 + EXT_EDID_OFFSET)
#define CEA_VERSION		(1 + EXT_EDID_OFFSET)
#define DTD_BEGIN		(2 + EXT_EDID_OFFSET)
#define DATA_BLOCK_COL_START	(4 + EXT_EDID_OFFSET)
	#define VIDEO_BLK_TAG	(2UL << 5)
	#define NUM_BYTES_MASK	0x1F
	#define BLK_TYPE_TAG	0xE0

/* */
unsigned int read_i2c_byte(unsigned int ulDeviceSel, unsigned int addr, int offset,  unsigned char *pBuffer);
unsigned int EdidHasCEA(unsigned char *pEdid);
u32 verify_edid_block_checksum(u8 *pbuf);
void removeDupBlock(u8 *pbuf);

/* Exported Functions */
#define EDID_PATCH_VIDEO_OVER_SPEC_MASK			0x00000001
#define EDID_PATCH_VIDEO_INTERLACE_MASK		0x00000002
#define EDID_PATCH_VIDEO_DEEP_COLOR_MASK	0x00000004
#define EDID_PATCH_VIDEO_3D_NONE			0
#define EDID_PATCH_VIDEO_3D_ALL				1
#define EDID_PATCH_VIDEO_3D_NONMANDATORY	2
#define EDID_PATCH_VIDEO_3D_MASK			0x00000030
#define EDID_PATCH_VIDEO_3D_SHIFT			4
#define EDID_PATCH_VIDEO_COLORIMETRY_SHIFT		6
#define EDID_PATCH_VIDEO_COLORIMETRY_MASK		(0x1 << EDID_PATCH_VIDEO_COLORIMETRY_SHIFT) /* Remove extended colorimetry. */
#define EDID_PATCH_VIDEO_EXT_VC_YUV420_SHIFT		8
#define EDID_PATCH_VIDEO_EXT_VC_YUV420_MASK		(0x1 << EDID_PATCH_VIDEO_EXT_VC_YUV420_SHIFT) /* Remove YUV420 support when EDID_PATCH_VIDEO_OVER_SPEC_MASK is set. */
#define EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_SHIFT		9
#define EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK		(0x1 << EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_SHIFT) /* add HDR static metadata data block */
#define EDID_PATCH_VIDEO_ADD_EXT_VC_420_ONLY_SHIFT	10
#define EDID_PATCH_VIDEO_ADD_EXT_VC_420_ONLY_MASK	(0x1 << EDID_PATCH_VIDEO_ADD_EXT_VC_420_ONLY_SHIFT) /* add 420 only VCDB */
#define EDID_PATCH_VIDEO_ADD_4K_24HZ_SHIFT		11
#define EDID_PATCH_VIDEO_ADD_4K_24HZ_MASK		(0x1 << EDID_PATCH_VIDEO_ADD_4K_24HZ_SHIFT) /* add 4K@24Hz VIC, HDMI1.4: in VSDB, HDMI2.0: in SVD */
#define EDID_PATCH_VIDEO_ADD_HF_VSDB_Y420_DC_SHIFT	12
#define EDID_PATCH_VIDEO_ADD_HF_VSDB_Y420_DC_MASK	(0x1 << EDID_PATCH_VIDEO_ADD_HF_VSDB_Y420_DC_SHIFT) /* add HDMI forum VSDB YUV420 Deep color */
#define EDID_PATCH_VIDEO_ADD_Y420CMDB_SHIFT		13
#define EDID_PATCH_VIDEO_ADD_Y420CMDB_MASK		(0x1 << EDID_PATCH_VIDEO_ADD_Y420CMDB_SHIFT) /* add YUV420 also support */
#define EDID_PATCH_VIDEO_DEL_EXT_VC_HDR_SHIFT		14
#define EDID_PATCH_VIDEO_DEL_EXT_VC_HDR_MASK		(0x1 << EDID_PATCH_VIDEO_DEL_EXT_VC_HDR_SHIFT) /* delete HDR static metadata data block */
#define EDID_PATCH_AUDIO_LPCM_CHAN_MASK		0x00010000
#define EDID_PATCH_AUDIO_LPCM_FREQ_MASK		0x00020000
#define EDID_PATCH_AUDIO_NLPCM_NONE			0
#define EDID_PATCH_AUDIO_NLPCM_ALL			1
#define EDID_PATCH_AUDIO_NLPCM_HD			2
#define EDID_PATCH_AUDIO_NLPCM_MASK			0x00300000
#define EDID_PATCH_AUDIO_NLPCM_SHIFT		20
#define EDID_PATCH_AUDIO_OVER_SPEC_SHIFT	23
#define EDID_PATCH_AUDIO_OVER_SPEC_MASK		(0x1 << EDID_PATCH_AUDIO_OVER_SPEC_SHIFT) /* Remove unsupported audio format. Ex: one bit audio. */
#define EDID_PATCH_AUDIO_ADD_ALL_SHIFT		24
#define EDID_PATCH_AUDIO_ADD_ALL_MASK		(0x1 << EDID_PATCH_AUDIO_ADD_ALL_SHIFT) /* Add all supported audio format. */

extern int hostReadEepromEdid(unsigned char *pEdid, unsigned int is_port_b);
extern int hostWriteEepromEdid(unsigned char *pEdid, unsigned int is_port_b);
extern unsigned int CmpEdid(unsigned char *pEdid1, unsigned char *pEdid2);
extern void edid_patcher(unsigned char *pEdid, unsigned int edid_patch);

/* obsoleted */
#if 0
extern void hostChangeEepromEdid(unsigned char *, int);
extern void EdidGetMaxRes(unsigned char *pEdid, unsigned short *outWidth, unsigned short *outHeight);
extern unsigned int EdidIsDigital(unsigned char *pEdid);
#endif
struct s_crt_drv;
void ast_edid_init(struct s_crt_drv *);

typedef struct {
	u32 valid;
	u32 checksum_error;
	u32 cea_ext_valid;
	u32 cea_ext_checksum_error;

	u32 digital;
	u32 yuv;
	u32 hdr;
	u32 preferred_timing_index; /* EDID parser should always return an valid index unless EDID is NOT valid. what we can display */
	u32 preferred_timing_index_raw; /* original preferred timing on EDID */
	u32 hdmi_20; /* HDMI 2.0 */
	u32 hdmi_1x; /* HDMI 1.x */
	u32 hdmi_physical_address; /* physical address */
} edid_status;

struct ast_edid_ops {
	void (*parse)(const u8 *raw_edid, edid_status *status);
	u32 (*patch)(u8 *edid, const u32 patch);
};

#endif /* #ifndef _EDID_H */
