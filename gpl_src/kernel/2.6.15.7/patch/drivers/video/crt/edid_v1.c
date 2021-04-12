#include <asm/arch/drivers/edid.h>

#if (AST_EDID_PARSER_VER == 1)
#include <linux/string.h>    /* memset */
#include <asm/arch/drivers/crt.h>
#include "astdebug.h"

static void _EDID_PA_Patch(unsigned char *vsd_blk, int block_size)
{
#if !REPEATER_EDID_PA
	return;
#endif
	/* Validate this block. Valid VSD block must >= 5 bytes */
	if (block_size < 5)
		return;
	/*
	** PA A.B.C.D is at vsd_blk[4].MSB, vsd_blk[4].LSB, vsd_blk[5].MSB, vsd_blk[5].LSB
	** 2 bytes long. 4 bits each.
	*/
	if (!(vsd_blk[4] & 0xF0)) {
		uerr("Invalid EDID PA?! %02X %02X\n", vsd_blk[4], vsd_blk[5]);
		vsd_blk[4] |= 0x10;
	} else if (!(vsd_blk[4] & 0x0F)) {
		vsd_blk[4] |= 0x01;
	} else if (!(vsd_blk[5] & 0xF0)) {
		vsd_blk[5] |= 0x10;
	} else if (!(vsd_blk[5] & 0x0F)) {
		vsd_blk[5] |= 0x01;
	} else {
		uerr("EDID PA is full?! %02X %02X\n", vsd_blk[4], vsd_blk[5]);
		vsd_blk[4] = 0xFF;
		vsd_blk[5] = 0xFF;
	}
}

/*
EDID Timing Priority Order
1 "Preferred Timing Mode" as defined in Base EDID (The first 18 Byte Descriptor Block)
2 Other "Detailed Timing Modes" in the order listed in BASE EDID
3 Any additional "Detailed Timing Modes" (priority is in the order listed)
  in optional EXTENSION Blocks to the BASE EDID
4 Any optional 3-Byte CVT Codes (defined in optional Display
  Descriptors) listed in BASE EDID or an optional Extension Block. (No interlace mode)
5 "Standard Timings" listed in BASE EDID and in optional EXTENSION Blocks. (No interlace mode)
6 Additional Timing Mode Information: Established Timings I, II & III,
  Default GTF, GTF Secondary Curve & CVT  (No interlace mode)
7 BASE VIDEO MODE (Default is often VGA) (Not interlace mode)

Which means to patch interlace mode, we only need to patch "Detailed Timing Desc Block".
*/

/* From VESA EDID spec v1.4
Preferred Timing Mode: The display's preferred timing mode shall be listed in the first 18 byte
data block (starting at address 36h. Refer to section 3.10). This is a requirement for EDID data
structure version 1, revision 3 and newer. For EDID version 1, revision 3, bit 1 (at address 18h)
shall be set to 1 (0 is invalid). For EDID version 1, revision 4, setting bit 1 (at address 18h) to 1
indicates that the preferred timing mode includes the native pixel format and the preferred
refresh rate of the display device (for example, an LCD module). A 0 at bit 1 (address 18h)
indicates the native pixel format and preferred refresh rate of the display device are not
included in the preferred timing mode.
*/

/*
7.2.2  Full 861B Implementation
If a Version 3 CEA Timing Extension has been included in EDID, all CEA video formats
shall be advertised using Short Video Descriptors, even if they are also required to be
advertised using the 18-byte Detailed Timing Descriptors (see below).
Even though short descriptors are now available in the Version 3 CEA Timing
Extension, there are still requirements to use 18-byte Detailed Timing Descriptors in
some cases to maintain backward compatibility with the previous versions of 861.  The
required 861/861A formats shall continue to be advertised using the EDID 18-byte
Detailed Timing Descriptors (for backward compatibility).  The optional 861A formats
(i.e., 480i and 576i) should be advertised in an EDID 18-byte Detailed Timing Descriptor
(for backward compatibility), but this is not required.  The optional formats from 861A
(720X480i, 720X576i) can be advertised using only Short Video Descriptors at the
option of the DTV Monitor designer.

7.4  CEA EDID Timing Extension Version 2
Version 2 of the CEA Timing Extension  (Table 28) contains a field for designating the
number of native formats supported by the DTV Monitor.  As with EIA/CEA-861 [22],
the first format listed in the base EDID data structure is the preferred format.  Other
native formats should be listed consecutively after that format.  The new field consists
of the count of the number of timing descriptors, starting with any listed in the base 128
EDID data structure and continuing with those listed in the extension blocks, that are to
be considered "native" timings (see Section 7.2.5).  If more than one timing extension is
needed, then the value shall be the same in all extensions.  Value zero means that this
information is not provided (for backward compatibility with EIA/CEA-861).  Format
Timings should not be repeated within a timing extension.  Timing extensions shall be
included to provide timing descriptors for all supported formats.  The last timing
extension may contain less than six 18-byte descriptors.  All other timing extensions
that contain detailed timing descriptors shall contain six 18-byte descriptors.

7.5  CEA EDID Timing Extension Version 3
Version 3 of the CEA EDID Timing Extension includes all of the indications contained in
CEA EDID Timing Extension Version 2 including the field that designates the number of
native formats described using 18-byte detailed descriptors.  The use of this field is the
same as in Version 2 of the CEA EDID Timing Extension.

When a Version 3 CEA Timing Extension is provided in the DTV Monitor's EDID data
structure, a short video descriptor shall be provided for each CEA video format
supported by the DTV Monitor.  The format of the short video descriptor shall conform
*/

static unsigned char interlace_idx[] = {
	5, /*1080i    16:9              1920x1080i @ 59.94/60Hz */
	6, /*480i      4:3          720(1440)x480i @ 59.94/60Hz */
	7, /* 480iH    16:9          720(1440)x480i @ 59.94/60Hz */
	10, /* 480i4x    4:3             (2880)x480i @ 59.94/60Hz */
	11, /* 480i4xH  16:9             (2880)x480i @ 59.94/60Hz */
	20, /* 1080i25  16:9              1920x1080i @ 50Hz* */
	21, /* 576i      4:3          720(1440)x576i @ 50Hz */
	22, /* 576iH    16:9          720(1440)x576i @ 50Hz */
	25, /* 576i4x    4:3             (2880)x576i @ 50Hz */
	26, /* 576i4xH  16:9             (2880)x576i @ 50Hz */
	39, /* 108Oi25  16:9  1920x1080i(1250 Total) @ 50Hz* */
	40, /* 1080i50  16:9              1920x1080i @ 100Hz */
	44, /* 576i50    4:3          720(1440)x576i @ 100Hz */
	45, /* 576i50H  16:9          720(1440)x576i @ 100Hz */
	46, /* 1080i60  16:9              1920x1080i @ 119.88/120Hz */
	50, /* 480i59    4:3          720(1440)x480i @ 119.88/120Hz */
	51, /* 480i59H  16:9          720(1440)x480i @ 119.88/120Hz */
	54, /* 576i100   4:3          720(1440)x576i @ 200Hz */
	55, /* 576i100H 16:9          720(1440)x576i @ 200Hz */
	58, /* 480i119   4:3          720(1440)x480i @ 239.76/240Hz */
	59 /* 480i119H 16:9          720(1440)x480i @ 239.76/240Hz */
};

#define IIDX_ARRAY_SIZE (sizeof(interlace_idx)/sizeof(interlace_idx[0]))
/* calculate the offset value from pt0 to pt1. (pt1-pt0) */
#define PT_OFFSET(pt1, pt0) ((unsigned char)(((unsigned char *)(pt1)) - ((unsigned char *)(pt0))))

#define DTD_640x480 {0xd8, 0x09, 0x80, 0xa0, 0x20, 0xe0, 0x2d, 0x10, 0x10, 0x60, 0xa2, 0x00, 0x13, 0x8e, 0x21, 0x0, 0x0, 0x18}

void edid_patcher(unsigned char *pEdid, unsigned int edid_patch)
{
	unsigned char tEDID[EDID_ROM_SIZE];
	unsigned int PatchInterlace = (edid_patch & EDID_PATCH_VIDEO_INTERLACE_MASK)?1:0;
	unsigned int PatchDeepColor = (edid_patch & EDID_PATCH_VIDEO_DEEP_COLOR_MASK)?1:0;
	unsigned int Patch3D = ((edid_patch & EDID_PATCH_VIDEO_3D_MASK) >> EDID_PATCH_VIDEO_3D_SHIFT);
	unsigned int PatchDTDOverSpec = (edid_patch & EDID_PATCH_VIDEO_OVER_SPEC_MASK)?1:0;
	unsigned int PatchAudioMultiChan = (edid_patch & EDID_PATCH_AUDIO_LPCM_CHAN_MASK)?1:0;
	unsigned int PatchAudioHighSampleRate = (edid_patch & EDID_PATCH_AUDIO_LPCM_FREQ_MASK)?1:0;
	unsigned int PatchAudioNLPCM = ((edid_patch & EDID_PATCH_AUDIO_NLPCM_MASK) >> EDID_PATCH_AUDIO_NLPCM_SHIFT);
	unsigned int PatchColorimetry = 1; /* always patch Colorimetry Data Block since extended colorimetry is not supported */
	PBASE_EDID b = (PBASE_EDID)pEdid;
	PCEAExtHdr e;
	unsigned char *pt = tEDID;
	PDTD pDtd;
	PData_Block pDB;
	unsigned int blk_size = 0;
	PCEAExtHdr pt_CEAExtHdr;
	unsigned char chksum = 0;
	unsigned int i;
	/* From CEA-861B spec.
	** TotalNativeNum should be the total numbers of DTDs for backward compatible
	** with CEA revision 2.
	*/
	unsigned char TotalNativeNum = 0;
	PDTD pt_pPtm; /* pt's pointer to preferred timing mode block. (block 0 of DTD of base EDID). */
	unsigned int CopyDTDFromCEA = 0;

	/* Assume the original EDID is valid. */
#if 0
	uinfo("Original EDID:\n");
	udump(pEdid, EDID_ROM_SIZE);
#endif

	if (edid_patch == 0) {
		/* Bruce120409. The patch code will always patch "Colorimetry" and "Dual DVI". It is
		** easier to just skip the whole patch code when we set "edid_patch == 0".
		*/
		printk("EDID pass through. No patches.\n");
		goto dont_patch;
	}

	printk("PatchInterlace = %d\n", PatchInterlace);
	printk("PatchDeepColor = %d\n", PatchDeepColor);
	printk("Patch3D = %d\n", Patch3D);
	printk("PatchDTDOverSpec = %d\n", PatchDTDOverSpec);
	printk("PatchAudioMultiChan = %d\n", PatchAudioMultiChan);
	printk("PatchAudioHighSampleRate = %d\n", PatchAudioHighSampleRate);
	printk("PatchAudioNLPCM = %d\n", PatchAudioNLPCM);
patch_restart:
	/* Initialization */
	b = (PBASE_EDID)pEdid;
	pt = tEDID;
	blk_size = 0;
	chksum = 0;
	TotalNativeNum = 0;
	CopyDTDFromCEA = 0;
	memset(pt, 0, EDID_ROM_SIZE);
	/* Copy the base EDID */
	memcpy(pt, b, EDID_BLOCK_SIZE);
	/* Patch Base EDID block.
	** Try to remove the interlace mode in DTDx4 blocks.
	** 1. First, zero all 4 blocks of the tEDID.
	** 2. Go through all pEdid's 4 DTD blocks and copy non-interlace mode to tEDID.
	** 3. If there is no non-interlace mode available, reserves block0 (PTM) and
	**   set CopyDTDFromCEA to true. The DTD in CEA extension block will be used later.
	** 4. Go through all pEdid's 4 DTD blocks again and copy non-Detailed timing blocks
	**   to tEDID's DTD blocks.
	** 5. If tEDID is not fully used, mark the unused blocks as "unused".
	** 6. Done. adjust pt.
	*/
	pt_pPtm = ((PBASE_EDID)pt)->desc_block; /* Saved here for global usage. */
	if (PatchInterlace || PatchDTDOverSpec) {
		PDTD pt_pDtd = pt_pPtm;
		pDtd = b->desc_block;
		/* Clear all desc_blocks first because we may move it. */
		memset(pt_pDtd, 0, sizeof(DTD) * 4);
		/* Copy all detailed timing mode first */
		for (i = 0; i < 4; i++, pDtd++) {
			if (pDtd->bits.PixelClk == 0)
				continue;
			/* Check for interlace */
			if (PatchInterlace && pDtd->bits.Interlace) {
				//uinfo("base EDID's DTD block %d is interlace mode\n", i);
				/* Don't copy. */
				continue;
			}
			/* Check for HA and VA spec and pixel clock < 160MHz */
			if (PatchDTDOverSpec) {
				unsigned int HA = (pDtd->bits.H_highbits & 0xF0) >> 4;
				unsigned int VA = (pDtd->bits.V_highbits & 0xF0) >> 4;
				HA <<= 8;
				HA |= pDtd->bits.HA;
				VA <<= 8;
				VA |= pDtd->bits.VA;
				if (HA > 1920 || VA > 1200 || pDtd->bits.PixelClk > 16000) {
					uinfo("DTD over spec. ignore it\n");
					/* Don't copy. */
					continue;
				}
			}
			/* Copy the non-interlaced detailed timing block. */
			memcpy(pt_pDtd, pDtd, sizeof(DTD));
			pt_pDtd++;
			TotalNativeNum++;
		}
		/* Check if there is at least one detailed timing saved in preferred timing block */
		if (TotalNativeNum == 0) {
			/* No preferred timing mode!! */
			//uinfo("No preferred timing mode after patch!\n");
			CopyDTDFromCEA = 1;
			pt_pDtd = &(((PBASE_EDID)pt)->desc_block[1]);
		}
		/* Copy the other non-"detailed timing" blocks. */
		pDtd = b->desc_block;
		for (i = 0; i < 4; i++, pDtd++) {
			if (pDtd->bits.PixelClk != 0)
				continue;
			memcpy(pt_pDtd, pDtd, sizeof(DTD));
			pt_pDtd++;
		}
		/* Mark the "unused" */
		while (pt_pDtd < (pt_pPtm + 4)) {
			pt_pDtd->raw_byte[3] = 0x10; /* Unused code */
			pt_pDtd++;
		}
	}
	/* Patch extension_numbers over 1. */
	if (b->extension_num > 1)
		((PBASE_EDID)pt)->extension_num = 1;

	pt += EDID_BLOCK_SIZE;

	/* Start of patch extension block:
	** Basically, go through each "data block" in pEdid, and copy to tEDID with necessary patch.
	** 1. Validate if it is CEA extension block.
	** 2. Go through each "data block", ignore/copy/patch to tEDID.
	** 3. Until DTD offset is reached.
	*/
	if (b->extension_num == 0)
		goto update_B0_chksum;

	e = (PCEAExtHdr)(b->ext_block);
	/* Verify checksum */
	if (verify_edid_block_checksum(b->ext_block)) {
		uerr("Extension block checksum error!? blind copy\n");
		memcpy(pt, e, EDID_BLOCK_SIZE);
		goto update_B0_chksum;
	}
	/* Check CEA header */
	if (e->tag != 0x02) {
		//uinfo("Has extension block, but is not CEA extension?!(%X)\n", e->tag);
		/* Just blind copy. */
		memcpy(pt, e, EDID_BLOCK_SIZE);
		goto update_B0_chksum;
	}
	memcpy(pt, e, sizeof(CEAExtHdr));
	pt_CEAExtHdr = (PCEAExtHdr)pt;
	pt += sizeof(CEAExtHdr);

#if 0
	uinfo("CEA revision: %d\n", e->revision);
	uinfo("DTD offset: 0x%02X (%d)\n", e->dtd_offset, e->dtd_offset);
	uinfo("Native num: %d\n", e->monitor_support.bits.native_num);
#endif

	/* dtd_offset:
	** If no non-DTD data is present in this extension block,
	** the value should be set to 04h (the byte after next).
	** If set to 00h, there are no DTDs present in this block and no non-DTD data.
	*/
	if (e->dtd_offset == 0) {
		//uinfo("Nothing to do in CEA extension block\n");
		goto update_B0_chksum;
	}
	if (e->dtd_offset == 4) {
		//uinfo("No non-DTD data in CEA extension block\n");
		goto DTD_patch;
	}
	/* Start of parsing non-DTD blocks */
	pDB = (PData_Block)(e->data_block_start);

next_db:
	if ((PT_OFFSET(pDB, pEdid) - EDID_BLOCK_SIZE) > e->dtd_offset) {
		uerr("Parsing error. Over dtd_offset?!\n");
		goto dont_patch;
	}
	if ((PT_OFFSET(pDB, pEdid) - EDID_BLOCK_SIZE) == e->dtd_offset) {
		//uinfo("Reach DTD block\n");
		goto DTD_patch;
	}
	blk_size = 1 + pDB->hdr.bits.size;

	/* valid current pDB */
	if ((PT_OFFSET(pDB, pEdid) + blk_size) >= EDID_ROM_SIZE) {
		uerr("Data Block parsing error?!\n");
		goto dont_patch;
	}

	if ((PT_OFFSET(pDB, pEdid) + blk_size - EDID_BLOCK_SIZE) > e->dtd_offset) {
		uerr("Parse error. data block size over DTD offset?!\n");
		goto DTD_patch;
	}

	if (pDB->hdr.bits.type == DBTYPE_AUDIO) {
		unsigned int blks = (pDB->hdr.bits.size)/sizeof(SAD);
		//uinfo("SAD with %d blocks\n", blks);
#if 0
		if (PatchAudio)
#else
		if (PatchAudioMultiChan || PatchAudioHighSampleRate || PatchAudioNLPCM)
#endif
		{
			PSAD pSAD = (PSAD)(&(pDB->data));
			PData_Block pt_pDB = (PData_Block)pt;
			uinfo("patch audio data block (%02X)\n", pt - tEDID);
			/* re-construct my own SADs */
			pt_pDB->hdr.bits.type = DBTYPE_AUDIO;
			pt_pDB->hdr.bits.size = 0;
			pt += sizeof(DBHDR);
			/* go through each SAD */
			while (blks) {
				//uinfo("SAD format=%d\n", pSAD->bits.format);
#if 0
				/* Remove (channel number > 2 || format != LPCM) */
				if (!(pSAD->bits.ch_num > 1 || pSAD->bits.format != 1)) {
					/* This SAD is 2Ch LPCM format. save it. */
					memcpy(pt, pSAD->raw_byte, sizeof(SAD));
					if (PatchAudioHighSampleRate) {
						/* Remove the support of sample frequency which > 48KHz */
						pt[1] &= (unsigned char)0x07;
					}
					pt += sizeof(SAD);
					/* update the size in pDB's header */
					pt_pDB->hdr.bits.size += sizeof(SAD);
				} else if (PatchAudio == 2) {
					/* Remove HD audio. Copy the others */
					if ((pSAD->bits.format != 1) /* LPCM and > 2ch */
						&& (pSAD->bits.format != 9) /* SACD */
						&& (pSAD->bits.format != 11) /* DTS-HD */
						&& (pSAD->bits.format != 12) /* MLP/Dolby TrueHD */
					) {
						memcpy(pt, pSAD->raw_byte, sizeof(SAD));
						pt += sizeof(SAD);
						/* update the size in pDB's header */
						pt_pDB->hdr.bits.size += sizeof(SAD);
					}
				}
#else
				PSAD pt_SAD = (PSAD)pt;
				if (pSAD->bits.format == 1) {
					/* LPCM */
					memcpy(pt, pSAD->raw_byte, sizeof(SAD));

					if ((pt_SAD->bits.ch_num > 1) && (PatchAudioMultiChan)) {
						pt_SAD->bits.ch_num = 1;
					}
					if ((pt[1] & (unsigned char)0xF8) && (PatchAudioHighSampleRate)) {
						/* Remove the support of sample frequency which > 48KHz. */
						pt[1] &= (unsigned char)0x07;
					}

					pt += sizeof(SAD);
					/* update the size in pDB's header */
					pt_pDB->hdr.bits.size += sizeof(SAD);
				} else {
					if (PatchAudioNLPCM == EDID_PATCH_AUDIO_NLPCM_NONE) {
						memcpy(pt, pSAD->raw_byte, sizeof(SAD));
						pt += sizeof(SAD);
						/* update the size in pDB's header */
						pt_pDB->hdr.bits.size += sizeof(SAD);
					} else if (PatchAudioNLPCM == EDID_PATCH_AUDIO_NLPCM_HD) {
						if ((pSAD->bits.format != 9) /* SACD */
							&& (pSAD->bits.format != 11) /* DTS-HD */
							&& (pSAD->bits.format != 12) /* MLP/Dolby TrueHD */
						) {
							memcpy(pt, pSAD->raw_byte, sizeof(SAD));
							pt += sizeof(SAD);
							/* update the size in pDB's header */
							pt_pDB->hdr.bits.size += sizeof(SAD);
						}
					}
				}
#endif
				/* to next SAD */
				pSAD++;
				blks--;
			}
			if (pt_pDB->hdr.bits.size == 0) {
#if 0
				unsigned char my_sad[] = { 0x09, 0x07, 0x07 };
				uerr("No 2Ch SAD?! Create myself\n");
				memcpy(pt, my_sad, sizeof(SAD));
				pt += sizeof(SAD);
				/* update the size in pDB's header */
				pt_pDB->hdr.bits.size += sizeof(SAD);
#else
				uinfo("remove audio data block\n");
				pt -= sizeof(DBHDR);
#endif
			}

			/* When every SAD is parsed, we constructed our own SADs.
			 * And now jump to next Data Block.
			 * to next data block
			 */
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
	} else if (pDB->hdr.bits.type == DBTYPE_VIDEO) {
		unsigned int blks = (pDB->hdr.bits.size)/sizeof(SVD);
		//uinfo("SVD with %d blocks\n", blks);
		if (PatchInterlace) {
			PSVD pSVD = (PSVD)(&(pDB->data));
			PData_Block pt_pDB = (PData_Block)pt;
			/* re-construct my own SVDs */
			pt_pDB->hdr.bits.type = DBTYPE_VIDEO;
			pt_pDB->hdr.bits.size = 0;
			pt += sizeof(DBHDR);
			/* go through each SVD */
			while (blks) {
				//uinfo("SVD idx=%d native=%d\n", pSVD->bits.index, pSVD->bits.native);
				for (i = 0; i < IIDX_ARRAY_SIZE; i++) {
					if (pSVD->bits.index == interlace_idx[i]) {
						//uinfo("Remove interlace SVD: 0x%02X\n", pSVD->raw_byte[0]);
						/* From CEA-861B. Following code should be wrong.
						** But I saw this kind of EDID on BenQ's 1080P monitor.!?
						// Check Native
						if (pSVD->bits.native && pt_CEAExtHdr->monitor_support.bits.native_num) {
							pt_CEAExtHdr->monitor_support.bits.native_num--;
							uinfo("Native Num=%d\n", pt_CEAExtHdr->monitor_support.bits.native_num);
						}
						*/
						break;
					}
				}
				if (i == IIDX_ARRAY_SIZE) {
					/* This SVD is not interlace. save it. */
					memcpy(pt, pSVD->raw_byte, sizeof(SVD));
					pt += sizeof(SVD);
					/* update the size in pDB's header */
					pt_pDB->hdr.bits.size += sizeof(SVD);
				}
				/* to next SVD */
				pSVD++;
				blks--;
			}
			/* When every SVD is parsed, we constructed our own SVDs.
			 * And now jump to next Data Block.
			 */
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
	} else if (pDB->hdr.bits.type == DBTYPE_VENDOR) {
		//uinfo("VSD\n");
#if 0
		if (Patch3D) {
			/* From HDMI spec 1.4a bytes >= offset 13 will be 3D and reserved.
			** So, just copy the first 13 bytes and clear "HDMI_Video_present".
			*/
			if (blk_size > 13) {
				PData_Block pt_pDB = (PData_Block)pt;
				PVSD pt_pVSD = &(pt_pDB->data.vsd);
				uinfo("Patch VSD\n");
				memcpy(pt, pDB, 13);
				if (PatchDeepColorAndDviDual) {
					uinfo("Patch deep color\n");
					pt[6] &= (unsigned char)0x80;
				}
				_EDID_PA_Patch(pt, blk_size);
				/* Patch size */
				pt_pDB->hdr.bits.size = 12;
				/* clear "HDMI_Video_present" */
				pt_pVSD->HDMI_video_present = 0;
				pt += 13;
				/* to next data block */
				pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
				goto next_db;
			} else {
				/* Otherwise, just copy all VSD. */
				memcpy(pt, pDB, blk_size);
				if (PatchDeepColorAndDviDual && (blk_size >= 7)) {
					uinfo("Patch deep color\n");
					pt[6] &= (unsigned char)0x80;
				}
				_EDID_PA_Patch(pt, blk_size);
				pt += blk_size;
				/* to next data block */
				pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
				goto next_db;
			}
		} else {
			/* Otherwise, just copy all VSD. */
			memcpy(pt, pDB, blk_size);
			if (PatchDeepColorAndDviDual && (blk_size >= 7)) {
				uinfo("Patch deep color\n");
				pt[6] &= (unsigned char)0x80;
			}
			_EDID_PA_Patch(pt, blk_size);
			pt += blk_size;
			/* to next data block */
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
#else
		PVSD pVSD = &(pDB->data.vsd);
		unsigned int delay_fields_length;
		PHDMI_VSDB_BYTE13 pt_b13, pb13;
		PHDMI_VSDB_BYTE14 pt_b14, pb14;
		PData_Block pt_DB = (PData_Block)pt;
//		PVSD pt_pVSD = &(pt_DB->data.vsd);
		if ((pVSD->id0 == 0x03) && (pVSD->id1 == 0x0c) && (pVSD->id2 == 0x00)) {
			uinfo("patch HDMI VSD (%02X)\n", pt - tEDID);
			if (pDB->hdr.bits.size >= 8 && pVSD->HDMI_video_present && Patch3D) {
				if (pVSD->Latency_Fields_present) {
					if (pVSD->I_Latency_Fields_present)
						delay_fields_length = 4;
					else
						delay_fields_length = 2;
				} else
					delay_fields_length = 0;

				pb13 = (PHDMI_VSDB_BYTE13)(((unsigned char *)pDB) + 9 + delay_fields_length);
				pb14 = (PHDMI_VSDB_BYTE14)(((unsigned char *)pDB) + 9 + delay_fields_length + 1);
				if (pb13->_3d_present) {
					/* skip 3D */
					memcpy(pt, pDB, 9 + delay_fields_length + 2 + pb14->hdmi_vic_len);

					pt_b13 = (PHDMI_VSDB_BYTE13)(pt + 9 + delay_fields_length);
					pt_b14 = (PHDMI_VSDB_BYTE14)(pt + 9 + delay_fields_length + 1);
					/* patch Length */
					pt_DB->hdr.bits.size = 8 + delay_fields_length + 2 + pb14->hdmi_vic_len;
					/* patch byte#13 */
					pt_b13->_3d_multi_present = 0;
					if (Patch3D == EDID_PATCH_VIDEO_3D_ALL)
						pt_b13->_3d_present = 0;

					/* patch byte#14 */
					pt_b14->hdmi_3d_len = 0;
				} else {
					/* Otherwise, just copy all VSD. */
					memcpy(pt, pDB, blk_size);
				}
			} else {
				/* Otherwise, just copy all VSD. */
				memcpy(pt, pDB, blk_size);
			}
			/* patch byte#4 & byte#5 */
			_EDID_PA_Patch(pt, pt_DB->hdr.bits.size);
			/* patch byte#6 */
			if (pt_DB->hdr.bits.size >= 6) {
				if (PatchDeepColor)
					pt[6] &= (unsigned char)0x80;
				else
					/* always patch DVI_Dual */
					pt[6] &= (unsigned char)0xF8;
			}

			pt += (pt_DB->hdr.bits.size + 1);
		} else
			uinfo("unknown VSD OUI (%02X %02X %02X)\n", pVSD->id0, pVSD->id1, pVSD->id2);
		/* to next data block */
		pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
		goto next_db;
#endif
	} else if (pDB->hdr.bits.type == DBTYPE_SPEAKER) {
		//uinfo("SPKR\n");
		uinfo("patch speaker allocation data block (%02X)\n", pt - tEDID);
		/* This case is found using "bad_speaker_data_block.txt. Ignore it. */
		if (blk_size != 4) {
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
#if 0
		if ((PatchAudio == 1) && (pDB->data.speaker.raw_byte[0] != 0x01)) {
			unsigned char my_spkr[] = { 0x83, 0x01, 0x00, 0x00 };
			//uinfo("Overwrite SPKR: 0x%02X\n", pDB->data.speaker.raw_byte[0]);
			memcpy(pt, my_spkr, blk_size);
			pt += blk_size;
			/* to next data block */
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
#else
		if ((PatchAudioMultiChan) && (PatchAudioNLPCM == EDID_PATCH_AUDIO_NLPCM_ALL)) {
			unsigned char my_spkr[] = { 0x83, 0x01, 0x00, 0x00 };
			//uinfo("Overwrite SPKR: 0x%02X\n", pDB->data.speaker.raw_byte[0]);
			memcpy(pt, my_spkr, blk_size);
			pt += blk_size;
			/* to next data block */
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
#endif
	} else if (pDB->hdr.bits.type == DBTYPE_EXTENDED) {
		unsigned char ext_tag_code = *(unsigned char *)(&pDB->data);
		uinfo("Extended Tag (%d)\n", ext_tag_code);
		if ((ext_tag_code == EXTENDED_DBTYPE_COLORIMETRY) && PatchColorimetry) {
			uinfo("patch Colorimetry Data Block\n");
			pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
			goto next_db;
		}
	} else {
		/* Unknown type */
		uerr("Unknown Data Block Type?! 0x%02X\n",
		     pDB->hdr.raw_byte[0]);
		/* I have seen this kind of data in "samsumgln40a550p1rxz.bin".
		 * It uses the reserved 0b111 as data block type.
		 * Just blind copy this kind of data would be a better solution.
		 * But watch out for the corruptted format.
		 */
		//goto DTD_patch;
	}
	/* Make sure we won't write over the size of EDID_BLOCK_SIZE */
	if ((PT_OFFSET(pt, tEDID) + blk_size) >= EDID_ROM_SIZE) {
		uerr("Invalid EDID?! Write over the size of EDID_ROM_SIZE!\n");
		goto dont_patch;
	}
	memcpy(pt, pDB, blk_size);
	pt += blk_size;
	/* to next data block */
	pDB = (PData_Block)(((unsigned char *)pDB) + blk_size);
	goto next_db;

DTD_patch:
	/*
	** Patch the DTDs in CEA extension block.
	** Just copy non-interlaced mode to tEDID.
	*/
	/* Update the dtd_offset. dtd_offset may changes due to the remove of SADs and SVDs. */
	pt_CEAExtHdr->dtd_offset = PT_OFFSET(pt, pt_CEAExtHdr);

	/*
	** From Wiki. Detailed Timings Description
	** "d": byte (designated in byte 02) where DTDs begin.  18-byte DTD strings continue for an unspecified
	** length (modulo 18) until a "00 00" is as the first bytes of a prospective DTD.  At this point,
	** the DTDs are known to be complete, and the start address of the "00 00" can be considered to be "XX"
	** (see below)
	** "XX"-126: Post-DTD padding.  Should be populated with 00h
	*/
	pDtd = (PDTD)(&(pEdid[EDID_BLOCK_SIZE + e->dtd_offset]));
	if (PT_OFFSET(pDtd, pEdid) + sizeof(DTD) >= EDID_ROM_SIZE) {
		uerr("Invalid DTD start?!\n");
		goto dont_patch;
	}
	/* Parse the DTDs */
	while (pDtd->bits.PixelClk != 0) {
		//uinfo("DTD\n");
		if (PatchInterlace && pDtd->bits.Interlace) {
			//uinfo("Remove interlaced DTD\n");
			// to next DTD
			pDtd++;
			continue;
		}
		/* Check for HA and VA spec and pixel clock < 160MHz */
		if (PatchDTDOverSpec) {
			unsigned int HA = (pDtd->bits.H_highbits & 0xF0) >> 4;
			unsigned int VA = (pDtd->bits.V_highbits & 0xF0) >> 4;
			HA <<= 8;
			HA |= pDtd->bits.HA;
			VA <<= 8;
			VA |= pDtd->bits.VA;
			if (HA > 1920 || VA > 1200 || pDtd->bits.PixelClk > 16000)
			{
				uinfo("CEA DTD over spec. ignore it\n");
				/* to next DTD */
				pDtd++;
				continue;
			}
		}

		/* if CopyDTDFromCEA is true, we need to copy to PTM */
		if (CopyDTDFromCEA) {
			//uinfo("Copy DTD from CEA to Base EDID\n");
			memcpy(pt_pPtm, pDtd, sizeof(DTD));
			CopyDTDFromCEA = 0;
			/*
			 * To avoid "no DTD in CEA extension", we will also have this copy
			 * in CEA extension block. It is harmless.
			 */
		}
		memcpy(pt, pDtd, sizeof(DTD));
		pt += sizeof(DTD);
		TotalNativeNum++;
		/* to next DTD */
		pDtd++;
		if (PT_OFFSET(pDtd, pEdid) + sizeof(DTD) >= EDID_ROM_SIZE) {
			//uinfo("no more rooms for DTD\n");
			break;
		}
	}
	// BruceToDo. Should handle the case where there is no DTD acceptted.
	// Now we finally has "Total number of native formats (DTDs). Update it.
	if (PatchInterlace || PatchDTDOverSpec)
		pt_CEAExtHdr->monitor_support.bits.native_num = TotalNativeNum;
	//uinfo("Patched Native Num=%d\n", pt_CEAExtHdr->monitor_support.bits.native_num);

	/*
	** We are done of parsing and patching.
	** The new EDID is now in tEDID and pEdid is never touched.
	** Update the checksum in tEDID and then copy to pEdid for return.
	*/

//update_B1_chksum:
	chksum = 0;

	for (i = 128; i < 255; i++)
		chksum = chksum + tEDID[i];

	tEDID[255] = ~chksum + 1;

update_B0_chksum:
	if (CopyDTDFromCEA) {
		unsigned char dtd[] = DTD_640x480;
#if 0
		/* If base EDID's PTM is still invalid, run the whole patch again without patching interlace */
		PatchInterlace = 0;
		goto patch_restart;
#endif
		uinfo("Copy DTD 640x480 to Base EDID\n");
		memcpy(pt_pPtm, dtd, sizeof(DTD));
		CopyDTDFromCEA = 0;
	}

	chksum = 0;
	for (i = 0; i < 127; i++)
		chksum = chksum + tEDID[i];

	tEDID[127] = ~chksum + 1;

	/* update to pEdid */
	memcpy(pEdid, tEDID, EDID_ROM_SIZE);
#if 0
	uinfo("Patched EDID:\n");
	udump(pEdid, EDID_ROM_SIZE);
#endif
	return;
dont_patch:
	return;
}

static void edid_parse(const u8 *raw_edid, edid_status *status)
{
	/* empty */
}

static void edid_destroy(void)
{
	/* empty */
}

static u32 edid_patch(u8 *edid, u32 patch)
{
	if (patch)
		edid_patcher(edid, patch);

	return 0;
}

/* */
static struct ast_edid_ops ast_edid = {
	.parse = edid_parse,
	.patch = edid_patch,
};

/**
 * ast_edid_init() -
 */
void ast_edid_init(struct s_crt_drv *crt)
{
	crt->ast_edid = &ast_edid;
}
EXPORT_SYMBOL(ast_edid_init);
#endif /* #if (AST_EDID_PARSER_VER == 1) */
