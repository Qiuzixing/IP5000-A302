
#include <asm/arch/drivers/edid.h>

#if (AST_EDID_PARSER_VER == 2)
#include <linux/slab.h>
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/crt.h>

#undef AST_EDID_DEBUG
#undef AST_EDID_PREFER_4K_SUPPORT
#undef AST_EDID_REMOVE_OVER_SPEC_DTD
#ifndef AST_EDID_REMOVE_OVER_SPEC_DTD
#define AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP
#endif

typedef _Bool	bool;

enum {
	false	= 0,
	true	= 1
};

#define EDID_EST_TIMINGS 16
#define EDID_STD_TIMINGS 8
#define EDID_DETAILED_TIMINGS 4

#define DETAILED_TIMING_DESCRIPTION_SIZE	18

#define EDID_LENGTH EDID_BLOCK_SIZE

#define CEA_EXT     0x02
#define VTB_EXT     0x10
#define DI_EXT      0x40
#define LS_EXT      0x50
#define MI_EXT      0x60
#define DISPLAYID_EXT 0x70


#define CEA_EXT_DATA_OFFSET_START	4
#define CEA_EXT_DATA_OFFSET_END		126 /* offset 127 is checksum */

#define AUDIO_BLOCK             0x01
#define VIDEO_BLOCK             0x02
#define VENDOR_BLOCK            0x03
#define SPEAKER_BLOCK           0x04
#define VESA_DTC_BLOCK	        0x05 /* VESA Display Transfer Characteristic Data block*/
#define VIDEO_CAPABILITY_BLOCK  0x07

#define VC_EXT_VCDB             0       /* Video Capability Data Block */
#define VC_EXT_VSVDB            1       /* Vendor-Specific Video Data Block */
#define VC_EXT_VESA_DDDB        2       /* VESA Display Device Data Block */
#define VC_EXT_VESA_VTB_EXT     3       /* VESA Video Timing Block Extension */
#define VC_EXT_COLORIMETRYDB    5       /* Colorimetry Data Block */
#define VC_EXT_HDRDB            6       /* HDR Static Metadata Data Block */
#define VC_EXT_VFPDB            13      /* Video Format Preference Data Block */
#define VC_EXT_Y420VDB          14      /* YCbCr 4:2:0 Video Data Block */
#define VC_EXT_Y420CMDB         15      /* YCbCr 4:2:0 Capability Map Data Block */
#define VC_EXT_VSADB            17      /* Vendor-Specific Audio Data Block */
#define VC_EXT_IFDB             32      /* InfoFrame Data Block */

#define EDID_EXT_NUM_MAX	1

#define HDMI_IEEE_OUI 0x000c03
#define HDMI_IEEE_OUI_FORUM 0xC45DD8
#define HDMI_IEEE_OUI_DOLBY_VISION 0x00D046

#define CEA_DB_TYPE(tag, size) ((((tag) & 0x7) << 5) | ((size) & 0x1F))

#define for_each_cea_db(cea, i, start, end) \
	for ((i) = (start); (i) < (end) && (i) + cea_db_payload_len(&(cea)[(i)]) < (end); (i) += cea_db_payload_len(&(cea)[(i)]) + 1)

#define SPEC_PIXEL_CLOCK_MHZ	165 /* 165MHz for AST1525, AST1510 and AST1500 */
#define SPEC_TIMING_PIXEL_CLOCK ((SPEC_PIXEL_CLOCK_MHZ*1000*1000)/(10 * 1000)) /* unit is 10KHz*/

#define SPEC_4K_PIXEL_CLOCK_MHZ	300
#define SPEC_4K_TIMING_PIXEL_CLOCK ((SPEC_4K_PIXEL_CLOCK_MHZ*1000*1000)/(10 * 1000)) /* unit is 10KHz*/

#define AST_SPEC_PIXEL_CLOCK() ((ast_scu.ability.v_support_4k >= 3) ? SPEC_4K_TIMING_PIXEL_CLOCK : SPEC_TIMING_PIXEL_CLOCK)
#define AST_SPEC_PIXEL_CLOCK_MHZ() ((ast_scu.ability.v_support_4k >= 3) ? SPEC_4K_PIXEL_CLOCK_MHZ : SPEC_PIXEL_CLOCK_MHZ)

extern struct s_crt_drv *crt;

struct est_timings {
	u8 t1;
	u8 t2;
	u8 mfg_rsvd;
} __attribute__((packed));

struct std_timing {
	u8 hsize; /* need to multiply by 8 then add 248 */
	u8 vfreq_aspect;
} __attribute__((packed));

/* If detailed data is pixel timing */
struct detailed_pixel_timing {
	u8 hactive_lo;
	u8 hblank_lo;
	u8 hactive_hblank_hi;
	u8 vactive_lo;
	u8 vblank_lo;
	u8 vactive_vblank_hi;
	u8 hsync_offset_lo;
	u8 hsync_pulse_width_lo;
	u8 vsync_offset_pulse_width_lo;
	u8 hsync_vsync_offset_pulse_width_hi;
	u8 width_mm_lo;
	u8 height_mm_lo;
	u8 width_height_mm_hi;
	u8 hborder;
	u8 vborder;
	u8 misc;
} __attribute__((packed));

struct detailed_data_monitor_range {
	u8 min_vfreq;
	u8 max_vfreq;
	u8 min_hfreq_khz;
	u8 max_hfreq_khz;
	u8 pixel_clock_mhz; /* need to multiply by 10 */
	u8 flags;
	union {
		struct {
			u8 reserved;
			u8 hfreq_start_khz; /* need to multiply by 2 */
			u8 c; /* need to divide by 2 */
			__le16 m;
			u8 k;
			u8 j; /* need to divide by 2 */
		} __attribute__((packed)) gtf2;
		struct {
			u8 version;
			u8 data1; /* high 6 bits: extra clock resolution */
			u8 data2; /* plus low 2 of above: max hactive */
			u8 supported_aspects;
			u8 flags; /* preferred aspect and blanking support */
			u8 supported_scalings;
			u8 preferred_refresh;
		} __attribute__((packed)) cvt;
	} formula;
} __attribute__((packed));

struct detailed_non_pixel {
	u8 pad1;
	u8 type; /* ff=serial, fe=string, fd=monitor range, fc=monitor name
		    fb=color point data, fa=standard timing data,
		    f9=undefined, f8=mfg. reserved */
	u8 pad2;
	union {
		struct detailed_data_monitor_range range;
		u8 raw[13];
	} data;
} __attribute__((packed));

#define T_H_ACTIVE(t) (((t)->hactive_hblank_hi & 0xf0) << 4 | (t)->hactive_lo)
#define T_V_ACTIVE(t) (((t)->vactive_vblank_hi & 0xf0) << 4 | (t)->vactive_lo)
#define T_H_BLANK(t) (((t)->hactive_hblank_hi & 0xf) << 8 | (t)->hblank_lo)
#define T_V_BLANK(t) (((t)->vactive_vblank_hi & 0xf) << 8 | (t)->vblank_lo)
#define T_H_SYNC_OFFSET(t) (((t)->hsync_vsync_offset_pulse_width_hi & 0xc0) << 2 | (t)->hsync_offset_lo)
#define T_H_SYNC_W(t) (((t)->hsync_vsync_offset_pulse_width_hi & 0x30) << 4 | (t)->hsync_pulse_width_lo)
#define T_V_SYNC_OFFSET(t) (((t)->hsync_vsync_offset_pulse_width_hi & 0xc) << 2 | (t)->vsync_offset_pulse_width_lo >> 4)
#define T_V_SYNC_W(t) (((t)->hsync_vsync_offset_pulse_width_hi & 0x3) << 4 | ((t)->vsync_offset_pulse_width_lo & 0xf))

struct detailed_timing {
	__le16 pixel_clock; /* need to multiply by 10 KHz */
	struct detailed_pixel_timing pixel_data;
} __attribute__((packed));

/**/
struct ast_detailed_mode {
	struct list_head head;
	struct detailed_timing timing;
} __attribute__((packed));

struct ast_audio_db {
	struct list_head head;
	u8 sad[3]; /* short audio descriptor, 3 bytes */
} __attribute__((packed));

struct ast_video_db {
	struct list_head head;
	u8 svd; /* short video descriptor, 1 byte */
} __attribute__((packed));

struct ast_vs_db {
	struct list_head head;
	u8 *data;
} __attribute__((packed));

struct ast_sa_db {
	struct list_head head;
	u8 data[4]; /* speaker allocation, 3 bytes, (tag, length), 1 byte */
} __attribute__((packed));

struct ast_vc_db {
	struct list_head head;
	u8 *data;
} __attribute__((packed));

struct ast_others_db {
	struct list_head head;
	u8 *data;
} __attribute__((packed));

struct ast_cea_ext_header {
	u8 tag;
	u8 revision;
	u8 dtd_offset;
	u8 native_num:4,
		ycbcr422:1,
		ycbcr444:1,
		audio:1,
		underscan:1;
} __attribute__((packed));

struct edid {
	u8 header[8];
	/* Vendor & product info */
	u8 mfg_id[2];
	u8 prod_code[2];
	u32 serial; /* FIXME: byte order */
	u8 mfg_week;
	u8 mfg_year;
	/* EDID version */
	u8 version;
	u8 revision;
	/* Display info: */
	u8 input;
	u8 width_cm;
	u8 height_cm;
	u8 gamma;
	u8 features;
	/* Color characteristics */
	u8 red_green_lo;
	u8 black_white_lo;
	u8 red_x;
	u8 red_y;
	u8 green_x;
	u8 green_y;
	u8 blue_x;
	u8 blue_y;
	u8 white_x;
	u8 white_y;
	/* Est. timings and mfg rsvd timings*/
	struct est_timings established_timings;
	/* Standard timings 1-8*/
	struct std_timing standard_timings[8];
	/* Detailing timings 1-4 */
	struct detailed_timing detailed_timings[4];
	/* Number of 128 byte ext. blocks */
	u8 extensions;
	/* Checksum */
	u8 checksum;
} __attribute__((packed));

struct edid_adapter {
	struct edid edid;
	struct ast_cea_ext_header ext_header;
	struct list_head detailed_modes;
	struct list_head audio_db;
	struct list_head video_db;
	struct list_head vs_db;
	struct list_head sa_db;
	struct list_head vc_db;
	struct list_head others_db;
	struct list_head ext_detailed_modes;
	struct list_head ext_display_descriptors;
	u8 *ext_data; /* buffer pointer to store extension block when checksum is error */

	u32 detailed_timing_num;
	u32 orig_detailed_timing_num;

	u32 audio_db_size; /* not include the length of tag */
	u32 video_db_size; /* not include the length of tag */
	u32 vs_db_size;
	u32 sa_db_size;
	u32 vc_db_size;
	u32 others_db_size;
	u32 ext_detailed_timing_num;
	u32 ext_display_descriptor_num;

	edid_status *status;
};

static spinlock_t edid_lock;
static struct edid_adapter adapter_entity, *_adapter = &adapter_entity;


static struct detailed_timing dtd_default_4k = {
	.pixel_clock = 29700, /* need to multiply by 10 KHz */
	.pixel_data = {
		.hactive_lo = 0x00,
		.hblank_lo = 0x30,
		.hactive_hblank_hi = 0xf2,
		.vactive_lo = 0x70,
		.vblank_lo = 0x5A,
		.vactive_vblank_hi = 0x80,
		.hsync_offset_lo = 0xB0,
		.hsync_pulse_width_lo = 0x58,
		.vsync_offset_pulse_width_lo = 0x8A,
		.hsync_vsync_offset_pulse_width_hi = 0x00,
		.width_mm_lo = 0x50,
		.height_mm_lo = 0x1D,
		.width_height_mm_hi = 0x74,
		.hborder = 0x00,
		.vborder = 0x00,
		.misc = 0x1E,
	},
};

static struct detailed_timing dtd_default_1080p = {
	.pixel_clock = 14850, /* need to multiply by 10 KHz */
	.pixel_data = {
		.hactive_lo = 0x80,
		.hblank_lo = 0x18,
		.hactive_hblank_hi = 0x71,
		.vactive_lo = 0x38,
		.vblank_lo = 0x2D,
		.vactive_vblank_hi = 0x40,
		.hsync_offset_lo = 0x58,
		.hsync_pulse_width_lo = 0x2C,
		.vsync_offset_pulse_width_lo = 0x45,
		.hsync_vsync_offset_pulse_width_hi = 0x00,
		.width_mm_lo = 0x50,
		.height_mm_lo = 0x1D,
		.width_height_mm_hi = 0x74,
		.hborder = 0x00,
		.vborder = 0x00,
		.misc = 0x1E,
	},
};

static int drm_edid_block_checksum(const u8 *raw_edid)
{
	int i;
	u8 csum = 0;

	for (i = 0; i < EDID_LENGTH; i++)
		csum += raw_edid[i];

	return csum;
}

static u8 *drm_find_edid_extension(struct edid *edid, int ext_id)
{
	u8 *edid_ext = NULL;
	int i;

	/* No EDID or EDID extensions */
	if (edid == NULL || edid->extensions == 0)
		return NULL;

	/* Find CEA extension */
	/* we only support 1 extension block */
	for (i = 0; i < EDID_EXT_NUM_MAX; i++) {
		edid_ext = (u8 *)edid + EDID_LENGTH * (i + 1);
		if (edid_ext[0] == ext_id)
			break;
	}

	if (i == EDID_EXT_NUM_MAX)
		return NULL;

	return edid_ext;
}

static u8 *drm_find_cea_extension(struct edid *edid)
{
	return drm_find_edid_extension(edid, CEA_EXT);
}

static inline int cea_db_payload_len(const u8 *db)
{
	return db[0] & 0x1f;
}

static inline int cea_db_tag(const u8 *db)
{
	return db[0] >> 5;
}

#if 0
static int cea_revision(const u8 *cea)
{
	return cea[1];
}
#endif

static int cea_db_offsets(const u8 *cea, int *start, int *end)
{
	/* Data block offset in CEA extension block */
	*start = CEA_EXT_DATA_OFFSET_START;
	*end = cea[2];

	/*
	 * From ANSI-CTA-861-F, Table 41 CEA Extension Version 3
	 * d = offset for the byte following the reserved data block.
	 * If no data is provided in the reserved data block, then d=4.
	 * If d=0, then no detailed timing descriptors are provided
	 * and no data is provided in the reserved data block collection.
	 *
	 * so we return -ENOENT to skip CEA extension parsing
	 */
	if (*end == 0)
		return -ENOENT;

	if (*end < CEA_EXT_DATA_OFFSET_START || *end > CEA_EXT_DATA_OFFSET_END)
		return -ERANGE;

	return 0;
}

static bool cea_db_is_hdmi_vsdb(const u8 *db)
{
	int hdmi_id;

	if (cea_db_tag(db) != VENDOR_BLOCK)
		return false;

	if (cea_db_payload_len(db) < 5)
		return false;

	hdmi_id = db[1] | (db[2] << 8) | (db[3] << 16);

	return hdmi_id == HDMI_IEEE_OUI;
}

static bool cea_db_is_hdmi_forum_vsdb(const u8 *db)
{
	int hdmi_id;

	if (cea_db_tag(db) != VENDOR_BLOCK)
		return false;

	if (cea_db_payload_len(db) < 5)
		return false;

	hdmi_id = db[1] | (db[2] << 8) | (db[3] << 16);

	return hdmi_id == HDMI_IEEE_OUI_FORUM;
}

static bool cea_db_is_vcdb_hdr(const u8 *db)
{

	u8 extended_tag_code;

	if (cea_db_tag(db) != VIDEO_CAPABILITY_BLOCK)
		return false;

	extended_tag_code = db[1];

	return extended_tag_code == VC_EXT_HDRDB;
}

static int detailed_mode_add(struct edid_adapter *adapter, struct detailed_timing *timing, struct list_head *list)
{
	struct ast_detailed_mode *mode;

	if (!timing->pixel_clock)
		return -1;

	if (!T_H_ACTIVE(&timing->pixel_data) || !T_V_ACTIVE(&timing->pixel_data)) /* H active or V active is 0 */
		return -1;

	mode = kzalloc(sizeof(struct ast_detailed_mode), GFP_KERNEL);

	if (!mode)
		return -1;

	memcpy(&mode->timing, timing, sizeof(struct detailed_timing));

	list_add_tail(&mode->head, list);

	return 0;
}

static void _detailed_mode_cleanup(struct edid_adapter *adapter, struct list_head *list)
{
	struct ast_detailed_mode *entry, *next;

	list_for_each_entry_safe(entry, next, list, head) {
		list_del(&entry->head);
		kfree(entry);
	}
}

static void detailed_mode_cleanup(struct edid_adapter *adapter)
{
	_detailed_mode_cleanup(adapter, &adapter->detailed_modes);

	adapter->detailed_timing_num = 0;
	adapter->orig_detailed_timing_num = 0;
}

static void ext_detailed_mode_cleanup(struct edid_adapter *adapter)
{
	_detailed_mode_cleanup(adapter, &adapter->ext_detailed_modes);
	adapter->ext_detailed_timing_num = 0;
}

static int ext_display_descriptor_add(struct edid_adapter *adapter, struct detailed_timing *timing)
{
	struct ast_detailed_mode *mode;
	struct detailed_non_pixel *p;

	if (timing->pixel_clock)
		return -1;

	p = (struct detailed_non_pixel *) &timing->pixel_data;

	if ((p->pad1 == 0) && (p->type != 0) && (p->pad2 == 0)) {

		mode = kzalloc(sizeof(struct ast_detailed_mode), GFP_KERNEL);

		if (!mode)
			return -1;

		memcpy(&mode->timing, timing, sizeof(struct detailed_timing));

		list_add_tail(&mode->head, &adapter->ext_display_descriptors);
		adapter->ext_display_descriptor_num++;

		return 0;
	}

	return -1;
}

static void ext_monitor_descriptor_cleanup(struct edid_adapter *adapter)
{
	struct ast_detailed_mode *entry, *next;
	struct list_head *lhead;

	lhead = &adapter->ext_display_descriptors;
	list_for_each_entry_safe(entry, next, lhead, head) {
		list_del(&entry->head);
		kfree(entry);
	}

	adapter->ext_display_descriptor_num = 0;
}

static int audio_db_add(struct edid_adapter *adapter, const u8 *db)
{
	u32 size = cea_db_payload_len(db), unit = 3;
	const u8 *loc;
	int i, num;
	struct ast_audio_db *new_db;

	loc = db + 1;
	num = size / unit;

	for (i = 0; i < num; i++) {
		new_db = kzalloc(sizeof(struct ast_audio_db), GFP_KERNEL);

		if (new_db) {
			memcpy(new_db->sad, loc, unit);

			list_add_tail(&new_db->head, &adapter->audio_db);
			adapter->audio_db_size += unit;
		}
		loc += unit;
	}

	return 0;
}

static inline void ast_audio_db_free(struct ast_audio_db *entry)
{
	kfree(entry);
}

static void audio_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_audio_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->audio_db, head) {
		list_del(&entry->head);
		ast_audio_db_free(entry);
	}

	adapter->audio_db_size = 0;
}

static int video_db_add(struct edid_adapter *adapter, const u8 *db)
{
	u32 size = cea_db_payload_len(db), unit = 1;
	const u8 *loc;
	int i, num;
	struct ast_video_db *new_db;

	loc = db + 1;
	num = size / unit;

	for (i = 0; i < num; i++) {
		new_db = kzalloc(sizeof(struct ast_video_db), GFP_KERNEL);

		if (new_db) {
			memcpy(&new_db->svd, loc, unit);
			list_add_tail(&new_db->head, &adapter->video_db);
			adapter->video_db_size += unit;
		}
		loc += unit;
	}

	return 0;
}

static inline void ast_video_db_free(struct ast_video_db *entry)
{
	kfree(entry);
}

static void video_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_video_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->video_db, head) {
		list_del(&entry->head);
		ast_video_db_free(entry);
	}

	adapter->video_db_size = 0;
}

static int vendor_specific_db_add(struct edid_adapter *adapter, const u8 *db)
{
	u32 size = cea_db_payload_len(db) + 1;
	struct ast_vs_db *new_db;

	new_db = kzalloc(sizeof(struct ast_vs_db), GFP_KERNEL);

	if (new_db) {
		new_db->data = kzalloc(size, GFP_KERNEL);
		if (new_db->data) {
			memcpy(new_db->data, db, size);
			list_add_tail(&new_db->head, &adapter->vs_db);
			adapter->vs_db_size += size;

			if (cea_db_is_hdmi_forum_vsdb(db)) {
				edid_status *status = adapter->status;

				status->hdmi_20 = 1;
			}

			if (cea_db_is_hdmi_vsdb(db)) {
				edid_status *status = adapter->status;

				status->hdmi_1x = 1;
				status->hdmi_physical_address = (db[4] << 8) | db[5];
			}

			return 0;
		}
		kfree(new_db);
	}

	return -1;
}

static inline void ast_vs_db_free(struct ast_vs_db *entry)
{
	kfree(entry->data);
	kfree(entry);
}

static void vendor_specific_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_vs_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->vs_db, head) {
		list_del(&entry->head);
		ast_vs_db_free(entry);
	}

	adapter->vs_db_size = 0;
}

static int video_capability_db_add(struct edid_adapter *adapter, const u8 *db)
{
	u32 size = cea_db_payload_len(db) + 1;
	struct ast_vc_db *new_vc;
	edid_status *status = adapter->status;

	new_vc = kzalloc(sizeof(struct ast_vc_db), GFP_KERNEL);

	if (new_vc) {
		new_vc->data = kzalloc(size, GFP_KERNEL);
		if (new_vc->data) {
			memcpy(new_vc->data, db, size);
			list_add_tail(&new_vc->head, &adapter->vc_db);

			if (cea_db_is_vcdb_hdr(db))
				status->hdr = 1;

			adapter->vc_db_size += size;
			return 0;
		}
		kfree(new_vc);
	}

	return -1;
}

static inline void ast_vc_db_free(struct ast_vc_db *entry)
{
	kfree(entry->data);
	kfree(entry);
}

static void video_capability_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_vc_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->vc_db, head) {
		list_del(&entry->head);
		ast_vc_db_free(entry);
	}

	adapter->vc_db_size = 0;
}

static int speaker_allocation_db_add(struct edid_adapter *adapter, const u8 *db)
{
	struct ast_sa_db *new_db;
	u32 unit = 4; /* size of speaker allocation data block is 3 and  (tag, length) is 1 */

	new_db = kzalloc(sizeof(struct ast_sa_db), GFP_KERNEL);

	if (new_db) {
		memcpy(new_db->data, db, unit);
		list_add_tail(&new_db->head, &adapter->sa_db);
		adapter->sa_db_size += unit;
	}

	return 0;
}

static inline void ast_sa_db_free(struct ast_sa_db *entry)
{
	kfree(entry);
}

static void speaker_allocation_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_sa_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->sa_db, head) {
		list_del(&entry->head);
		ast_sa_db_free(entry);
	}

	adapter->sa_db_size = 0;
}

static int others_db_add(struct edid_adapter *adapter, const u8 *db)
{
	u32 size = cea_db_payload_len(db) + 1;
	struct ast_others_db *new_db;

	new_db = kzalloc(sizeof(struct ast_others_db), GFP_KERNEL);

	if (new_db) {
		new_db->data = kzalloc(size, GFP_KERNEL);
		if (new_db->data) {
			memcpy(new_db->data, db, size);
			list_add_tail(&new_db->head, &adapter->others_db);
			adapter->others_db_size += size;
			return 0;
		}
		kfree(new_db);
	}

	return -1;
}
static inline void ast_others_db_free(struct ast_others_db *entry)
{
	kfree(entry->data);
	kfree(entry);
}

static void others_db_cleanup(struct edid_adapter *adapter)
{
	struct ast_others_db *entry, *next;

	list_for_each_entry_safe(entry, next, &adapter->others_db, head) {
		list_del(&entry->head);
		ast_others_db_free(entry);
	}

	adapter->others_db_size = 0;
}

static void ext_db_cleanup(struct edid_adapter *adapter)
{
	others_db_cleanup(adapter);
	audio_db_cleanup(adapter);
	video_db_cleanup(adapter);
	vendor_specific_db_cleanup(adapter);
	speaker_allocation_db_cleanup(adapter);
	video_capability_db_cleanup(adapter);
}

static void misc_cleanup(struct edid_adapter *adapter)
{
	if ((adapter->status->cea_ext_checksum_error) && (adapter->ext_data))
		kfree(adapter->ext_data);
}

static void ext_detailed_mode_add(struct edid_adapter *adapter, struct detailed_timing *timing)
{
	if (detailed_mode_add(adapter, timing, &adapter->ext_detailed_modes) == 0)
		adapter->ext_detailed_timing_num++;
}

static void edid_cea_parse_detailed_block(struct edid_adapter *adapter, const u8 *ext)
{
	int i, n = 0;
	u8 d = ext[0x02];
	const u8 *det_base = ext + d;
	struct detailed_timing *timing;

	if (d == 0)
		return;

	n = (127 - d) / DETAILED_TIMING_DESCRIPTION_SIZE; /* n = (126 - d + 1)/18 */

	for (i = 0; i < n; i++) {
		timing = (struct detailed_timing *)(det_base + DETAILED_TIMING_DESCRIPTION_SIZE * i);

		if (timing->pixel_clock)
			ext_detailed_mode_add(adapter, timing);
		else
			ext_display_descriptor_add(adapter, timing);
	}
}

static int edid_cea_parse(struct edid_adapter *adapter, const u8 *cea)
{
	const u8 *db;
	int i, start, end, ret = -1;;

	if (cea_db_offsets(cea, &start, &end)) {
		goto cea_exit;
	}

	for_each_cea_db(cea, i, start, end) {
		db = &cea[i];

		if (unlikely(cea_db_payload_len(db) == 0)) {
			ext_db_cleanup(adapter);
			goto cea_exit;
		}

		switch (cea_db_tag(db)) {
		case AUDIO_BLOCK:
			audio_db_add(adapter, db);
			break;
		case VIDEO_BLOCK:
			video_db_add(adapter, db);
			break;
		case VENDOR_BLOCK:
			vendor_specific_db_add(adapter, db);
			break;
		case SPEAKER_BLOCK:
			speaker_allocation_db_add(adapter, db);
			break;
		case VIDEO_CAPABILITY_BLOCK:
			video_capability_db_add(adapter, db);
			break;
		default:
			/* VESA_DTC_BLOCK */
			/* 0 and 6 are reserved */
			others_db_add(adapter, db);
			break;
		}
	}

	edid_cea_parse_detailed_block(adapter, cea);
	ret = 0;

cea_exit:
	return ret;
}

static u8 svd_to_vic(u8 svd)
{
	/*
	 * From 861-F, 7.5.1:
	 * 0: reserved
	 * 1 ~ 64: 7-bit VIC
	 * 65~127: 8-bit VIC
	 * 128: reserved
	 * 129~192: 7-bit VIC and is a native code
	 * 193~253: 8-bit VIC
	 * 254: reserved
	 * 255: reserved
	 */
	if (svd <= 127)
		return svd;
	else if (svd <= 192)
		return svd & 0x7F;
	else
		return 0;
}

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

static bool video_db_interlaced(struct ast_video_db *db)
{
	bool ret = false;
	u8 vic;
	int i;

	vic = svd_to_vic(db->svd);

	for (i = 0; i < ARRAY_SIZE(interlace_idx); i++) {
		if (vic == interlace_idx[i]) {
			ret = true;
			break;
		}
	}
	return ret;
}

static void edid_patch_interlaced_dtd(struct edid_adapter *adapter, u32 patch)
{
	struct ast_detailed_mode *entry, *next;
	struct detailed_pixel_timing *pt;
	struct list_head *lhead;

	if (!(patch & EDID_PATCH_VIDEO_INTERLACE_MASK))
		return;

	lhead = &adapter->ext_detailed_modes;
	list_for_each_entry_safe(entry, next, lhead, head) {
		pt = &entry->timing.pixel_data;
		if (pt->misc & DRM_EDID_PT_INTERLACED) {
			list_del(&entry->head);
			adapter->ext_detailed_timing_num--;
			kfree(entry);
		}
	}

	lhead = &adapter->detailed_modes;
	list_for_each_entry_safe(entry, next, lhead, head) {
		pt = &entry->timing.pixel_data;
		if (pt->misc & DRM_EDID_PT_INTERLACED) {
			adapter->detailed_timing_num--;
			list_del(&entry->head);
			kfree(entry);
		}
	}
}

static void edid_patch_interlace_video_db(struct edid_adapter *adapter, u32 patch)
{
	struct ast_video_db *entry, *next;

	if (!(patch & EDID_PATCH_VIDEO_INTERLACE_MASK))
		return;

	list_for_each_entry_safe(entry, next, &adapter->video_db, head) {
		if (video_db_interlaced(entry)) {
			list_del(&entry->head);
			adapter->video_db_size--; /* SVD is one byte */
			ast_video_db_free(entry);
		}
	}

	if (list_empty(&adapter->video_db))
		adapter->video_db_size = 0;
}

static void edid_patch_interlaced(struct edid_adapter *adapter, u32 patch)
{
	if (!(patch & EDID_PATCH_VIDEO_INTERLACE_MASK))
		return;

	/* detailed timing */
	edid_patch_interlaced_dtd(adapter, patch);

	/* Video data block */
	edid_patch_interlace_video_db(adapter, patch);
}

static void edid_patch_deepcolor(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vs_db *db;

	if (!(patch & EDID_PATCH_VIDEO_DEEP_COLOR_MASK))
		return;

	list_for_each_entry(db, &adapter->vs_db, head) {
		if (cea_db_is_hdmi_vsdb(db->data)) {
			if (cea_db_payload_len(db->data) >= 6) {
				/*
				 * byte 6[6]: 48 bits/pixel
				 * byte 6[5]: 36 bits/pixel
				 * byte 6[4]: 30 bits/pixel
				 */
				db->data[6] &= 0x8F;
			}
		} else if (cea_db_is_hdmi_forum_vsdb(db->data)) {
				/*
				 * byte 7
				 * [7:3]: 0
				 * [2]: 48 bits/pixel
				 * [1]: 36 bits/pixel
				 * [0]: 30 bits/pixel
				 */
			db->data[7] = 0;
		}
	}
}

static void do_patch_3d(u8 *db, u32 patch)
{
	u32 offset = 0, len = cea_db_payload_len(db);
	u32 hdmi_3d_present, multi_present, hdmi_3d_len;

	/* no HDMI_Video_Present */
	if (!(db[8] & (1 << 5)))
		goto out;

	/* Latency_Fields_Present */
	if (db[8] & (1 << 7))
		offset += 2;

	/* I_Latency_Fields_Present */
	if (db[8] & (1 << 6))
		offset += 2;

	/* the declared length is not long enough for the 2 first bytes
	 * of additional video format capabilities
	 */
	if (len < (8 + offset + 2))
		goto out;

	/* 3D_Present */
	offset++;

	hdmi_3d_present = db[8 + offset] >> 7;

	if (hdmi_3d_present) {
		hdmi_3d_present = offset; /* hdmi_3d_present is index after this */

		/* 3D_Multi_present */
		multi_present = (db[8 + offset] & 0x60) >> 5;

		if (multi_present) {
			multi_present = offset; /* multi_present is idnex after this */

			offset++;
			hdmi_3d_len = db[8 + offset] & 0x1f;

			/* clear 3D_Multi_present */
			db[8 + multi_present] &= ~0x60;

			/* adjust Data Block length */
			db[0] = CEA_DB_TYPE(VENDOR_BLOCK, (len - hdmi_3d_len));
		}

		if (patch == EDID_PATCH_VIDEO_3D_ALL) {
			/* clear 3D_present */
			db[8 + hdmi_3d_present] &= ~0x80;
		}
	}

out:
	return;
}

static void edid_patch_3d(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vs_db *db;
	u32 patch_cfg;

	patch_cfg = ((patch & EDID_PATCH_VIDEO_3D_MASK) >> EDID_PATCH_VIDEO_3D_SHIFT);

	if (!patch_cfg)
		return;

	list_for_each_entry(db, &adapter->vs_db, head) {
		if (cea_db_is_hdmi_vsdb(db->data))
			do_patch_3d(db->data, patch_cfg);
		else if (cea_db_is_hdmi_forum_vsdb(db->data)) {
			/*
			 * byte 6
			 * [2]: 3D Independent View
			 * [1]: 3D Dual View
			 * [0]: 3D OSD Disparity
			 */
			db->data[6] &= 0xF8;
		}
	}
}

static bool ast_dtd_over_spec(struct ast_detailed_mode *mode)
{
	struct detailed_timing *t = &mode->timing;
	u32 spec_pixel_clock;

	spec_pixel_clock = AST_SPEC_PIXEL_CLOCK();

	/* just check pixel clock, pixel clock unit in DTD is 10KHz, AST_SPEC_PIXEL_CLOCK is in 10KHz */
	if (spec_pixel_clock < t->pixel_clock)
		return true;

	return false;
}

/*
 * From 861-F Table 1,
 * Low
 *	93, 103 (3840x2160, 297MHz)
 *	94, 104 (3840x2160, 297MHz)
 *	95, 105 (3840x2160, 297MHz)
 *	98 (4096x2160, 297MHz)
 *	99 (4096x2160, 297MHz)
 *	100 (4096x2160, 297MHz)
 * 50Hz
 *	89 (2560x1080, 185.625MHz)
 *	96, 106 (3840x2160, 594MHz)
 *	101 (4096x2160, 594MHz)
 * 60Hz
 *	90 (2560x1080, 198MHz)
 *	97, 107 (3840x2160, 594MHz)
 *	102 (4096x2160, 594MHz)
 * 100Hz
 *	84 (1680x720, 165MHz)
 *	64, 77 (1920x1080, 297MHz)
 *	91 (2560x1080, 371.25MHz)
 * 120Hz:
 *	85 (1680x720, 198MHz)
 *	63, 78 (1920x1080, 297MHz)
 *	92 (2560x1080, 495MHz)
 *
 * 1. clock > 300
 *	91, 92, 96, 97, 101, 102, 106, 107
 * 2. clock > 160
 *	63, 64, 77, 78, 84, 85
 *	89, 90, 91, 92,
 *	93, 94, 95,
 *	96, 97, 98, 99, 100
 *	101, 102
 *	103, 104, 105, 106, 107
 */
static u8 vic_clock_over_160[] = {
	63, 64, 77, 78, 84, 85,
	89, 90, 91, 92,
	93, 94, 95,
	96, 97, 98, 99, 100,
	101, 102,
	103, 104, 105, 106, 107
};

static u8 vic_clock_over_300[] = {
	91, 92, 96, 97, 101, 102, 106, 107
};

#define VIC_OVER_SPEC_MAX  107

static bool vic_over_spec(u8 vic)
{
	u8 *check;
	bool ret = false;
	u32 i, max;

	if (vic > VIC_OVER_SPEC_MAX) {
		ret = true;
		goto out;
	}

	if (ast_scu.ability.v_support_4k >= 3) {
		check = vic_clock_over_300;
		max = ARRAY_SIZE(vic_clock_over_300);
	} else {
		check = vic_clock_over_160;
		max = ARRAY_SIZE(vic_clock_over_160);
	}

	for (i = 0; i < max; i++) {
		if (vic == check[i]) {
			ret = true;
			break;
		}
	}
out:
	return ret;
}

static bool video_db_over_spec(struct ast_video_db *db)
{
	u8 vic;

	vic = svd_to_vic(db->svd);

	return vic_over_spec(vic);
}

static u8 vic_4k_24[] = {
	VIC_2160p24, VIC_2160p24E, VIC_2160p24W1
};

static bool vic_4k_24hz(u8 vic)
{
	u8 *check;
	bool ret = false;
	u32 i, max;

	check = vic_4k_24;
	max = ARRAY_SIZE(vic_4k_24);

	for (i = 0; i < max; i++) {
		if (vic == check[i]) {
			ret = true;
			break;
		}
	}

	return ret;
}

static bool video_db_4k_24hz(struct ast_video_db *db)
{
	u8 vic;

	vic = svd_to_vic(db->svd);

	return vic_4k_24hz(vic);

}

static u32 edid_video_db_4K_24hz(struct edid_adapter *adapter)
{
	struct ast_video_db *db;

	list_for_each_entry(db, &adapter->video_db, head) {
		if (video_db_4k_24hz(db))
			return 1;
	}

	return 0;
}

static bool vs_db_4k_24hz(struct ast_vs_db *db)
{
	bool ret = false;

	if (cea_db_is_hdmi_vsdb(db->data)) {
		u32 size = cea_db_payload_len(db->data);
		u32 offset = 8;

		if (size >= 8) {
			u32 latency, latency_i, hdmi, i, len;

			latency = (db->data[8] >> 7) & 0x1;
			latency_i = (db->data[8] >> 6) & 0x1;
			hdmi = (db->data[8] >> 5) & 0x1;

			if (!hdmi)
				goto exit;

			offset++;

			if (latency)
				offset += 2;

			if (latency_i)
				offset += 2;

			offset += 1;

			len = (db->data[offset]	>> 5) & 0x7;

			offset += 1;

			if (offset > size)
				goto exit;

			if ((offset + len - 1) > size)
				len = size - offset + 1;

			for (i = 0; i < len; i++) {
				/*
				 * HDMI 1.4b Table 8-14
				 * 3: 4K 24Hz
				 * 4: SMPTE 4K 24Hz
				 *
				 */
				if ((db->data[offset + i] == 3)
					|| (db->data[offset + i] == 4)) {
					ret = true;
					goto exit;
				}
			}
		}
	}

exit:
	return ret;

}

static u32 edid_vs_db_4K_24hz(struct edid_adapter *adapter)
{
	struct ast_vs_db *db;

	list_for_each_entry(db, &adapter->vs_db, head) {
		if (vs_db_4k_24hz(db))
			return 1;
	}

	return 0;
}


/*
 * edid_4k_24Hz - return 4K 24Hz timing status
 * @adapter:
 *
 * return 1 if has 4K 24Hz
 */
static u32 edid_4k_24hz(struct edid_adapter *adapter)
{
	if (edid_video_db_4K_24hz(adapter))
		return 1;

	if (edid_vs_db_4K_24hz(adapter))
		return 1;

	return 0;
}

static void edid_patch_over_spec_ext_video_db(struct edid_adapter *adapter, u32 patch)
{
	struct ast_video_db *entry, *next;

	if (!(patch & EDID_PATCH_VIDEO_OVER_SPEC_MASK))
		return;

	list_for_each_entry_safe(entry, next, &adapter->video_db, head) {
		if (video_db_over_spec(entry)) {
			list_del(&entry->head);
			adapter->video_db_size--; /* SVD is one byte */
			ast_video_db_free(entry);
		}
	}

	if (list_empty(&adapter->video_db))
		adapter->video_db_size = 0;
}

static void remove_hdmi_14b_vsdb_vic(u8 *db)
{
	u32 payload_len;

	payload_len = cea_db_payload_len(db);

	/* remove HDMI1.4b 4K vic in VSDB if exist */
	if ((payload_len >= 8) && (db[8] & (0x1 << 5))) {
		/*
		 * db[8]
		 * bit 5: HDMI video present
		 * bit 6: interlaced latency present
		 * bit 7: latency present
		 */
		u32 len_vic, len_3d;
		u32 offset = 8 + 1;

		if (db[8] & (0x1 << 7))
			offset += 2;
		if (db[8] & (0x1 << 6))
			offset += 2;
		offset += 1;

		len_vic = db[offset] >> 5;
		len_3d = db[offset] & 0x1F;

		if (len_vic > 0) {
			if (len_3d > 0)
				memcpy(db + offset + 1,  db + offset + 1 + len_vic, len_3d);

			payload_len -= len_vic;

			if (len_3d == 0) {
				db[8] = db[8] & 0xDF; /* clear [5]: HDMI video present */
				payload_len -= len_vic;
			}
			/* update payload length */
			db[0] = db[0] & 0xE0;
			db[0] |= (payload_len & 0x1F);
		}
	}
}

static void edid_patch_over_spec_ext_vs_db(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vs_db *entry, *next;
	u32 spec_pixel_clock_mhz;
	u8 *db;

	if (!(patch & EDID_PATCH_VIDEO_OVER_SPEC_MASK))
		return;

	spec_pixel_clock_mhz = AST_SPEC_PIXEL_CLOCK_MHZ();

	list_for_each_entry_safe(entry, next, &adapter->vs_db, head) {
		db = entry->data;
		if ((cea_db_is_hdmi_vsdb(db)) && (cea_db_payload_len(db) >= 7)) {
			if (spec_pixel_clock_mhz < (db[7] * 5))
				db[7] = spec_pixel_clock_mhz / 5;

			if (!(ast_scu.ability.v_support_4k >= 3)) {
				/* remove HDMI1.4b 4K vic in VSDB if exist */
				remove_hdmi_14b_vsdb_vic(db);
			}
		}

		/*
		 * From HDMI Spec.2.0, 10.3.2
		 * if the Sink does not support TMDS Character Rates > 340 Mcsc,
		 * then the sink shall set this field to 0
		 * length: minimum is 7, maximum is 31
		 */
		if (cea_db_is_hdmi_forum_vsdb(db)) {
			if (spec_pixel_clock_mhz < (db[5] * 5))
				db[5] = 0;
			/* patch all except 3D */
			db[6] &= 0x7;
			db[7] &= 0x0;
		}
	}
}

static inline u8 vc_db_ext_tag(u8 *db)
{
	return db[1];
}

static bool vc_db_over_spec(struct ast_vc_db *db)
{
	/*
	 * 0xE: YCbCr 4:2:0 Video Data Block (Y420VDB)
	 * 0xF: YCbCr 4:2:0 Capability Map Data Block (Y420CMDB)
	 * Y420VDB => only support 420
	 * Y420CMDB => also support 420
	 *
	 * HDMI Spec. 2.0, Table 7-1, Video Timing that may be used with 420
	 *	3840x2160P (50Hz:96, 106; 60Hz:97, 107)
	 *	4096X2160P (50Hz:102; 60Hz:102)
	 *
	 * we already filter the timings listed above in video_db_over_spec(),
	 * just check VC_EXT_Y420CMDB in this function.
	 */
	if (vc_db_ext_tag(db->data) == VC_EXT_Y420CMDB)
		return true;

	if (vc_db_ext_tag(db->data) == VC_EXT_VSVDB) {
		u32 oui;

		oui = db->data[2] | (db->data[3] << 8) | (db->data[4] << 16);
		/*
		 * cannot support dolby vision HDR
		 * dolby vision Vendor-Specific Video Data Block => over spec
		 */
		if (oui == HDMI_IEEE_OUI_DOLBY_VISION)
			return true;

	}

	return false;
}

static inline u32 u8_array_bit(u8 *array, u32 array_size_in_bit, u32 index)
{
	/*
	** This function returns the value of a specified bit by index of u8 array.
	**
	** index is bit index.
	** Step 1.array[index >> 3] will map 'index' to 'array[x]'.
	** Step 2.array[index >> 3] >> (index & 0x7) ==> bit 0 will the the answer.
	** Step 3.return bit 0.
	*/
	if (index < array_size_in_bit)
		return (array[index >> 3] >> (index & 0x7)) & 0x1UL;

	return 0;
}

static inline void u8_array_bit_set(u8 *array, u32 index)
{
	array[index >> 3] |= (0x1 << (index & 0x7));
}

/*
** static void new_n_add_vc_ext_y420vdb(struct edid_adapter *adapter, u8 *y420cmdb)
**
** This function converts y420cmdb to y420vdb.
** This function MUST be called after adapter->video_db is valid.
**
** 0xE: YCbCr 4:2:0 Video Data Block (Y420VDB)
** 0xF: YCbCr 4:2:0 Capability Map Data Block (Y420CMDB)
** y420cmdb is a bitmap mapping to SVD. CEA-861-F 7.5.11.
** y420db is a list of SVD supporting YUV420. CEA-861-F 7.5.10.
**
** - Reference to y420cmdb to know which YUV420 timing supported.
** - Create a new y420vdb based on y420cmdb.
** - Add y420vdb into adapter->vc_db, video capability db list.
*/
static void new_n_add_vc_ext_y420vdb(struct edid_adapter *adapter, u8 *y420cmdb)
{
	int i, j, size;
	u8 *y420vdb;
	u32 max = (cea_db_payload_len(y420cmdb) - 1) * 8;

	size = 0;

	/* Walk through y420cmdb bitmap bit by bit. 'size' will be the number of supported YUV420 timing. */
	for (i = 0; i < max; i++) {
		if (u8_array_bit(y420cmdb + 2, max, i))
			size++;
	}

	if (size == 0)
		return;

	y420vdb = kzalloc(size + 2, GFP_KERNEL);

	i = 0; /* iterate through video DB list  */
	j = 2; /* Y420VDB data index */

	if (y420vdb) {
		struct ast_video_db *entry;

		/* Walk through SVD list. If a SVD support YUV420, Copy it to the new y420vdb. */
		list_for_each_entry(entry, &adapter->video_db, head) {
			if (u8_array_bit(y420cmdb + 2, max, i)) {
				u8 vic;

				vic = svd_to_vic(entry->svd);
				if (!vic_over_spec(vic)) { /* must check VIC whether over spec */
					y420vdb[j] = vic;
					j++;
				}
			}
			i++;
		}

		if (j > 2) {
			y420vdb[0] = CEA_DB_TYPE(VIDEO_CAPABILITY_BLOCK, (j - 2) + 1);
			y420vdb[1] = VC_EXT_Y420VDB;
			video_capability_db_add(adapter, y420vdb);
		}

		kfree(y420vdb);
	}
}

/*
 * do_patch_vc_vfpdb - return a new pointer to a memory contain VFPDB without any overspec VICs
 *
 * 7.5.12 Video Format Preference Data Block
 * When present, the VFPDB shall take precedence over preferred indications defined elsewhere in CEA-861-F
 * We have to eliminate VIC of VFPDB if it is over spec
 */
static u8 *do_patch_vc_vfpdb(u8 *data)
{
	u32 i, j, max, over = 0;
	u8 *pvic, *new_db;

	pvic = data + 2;

	max = cea_db_payload_len(data) - 1;

	for (i = 0; i < max; i++) {
		if (vic_over_spec(*pvic))
			over++;
		pvic++;
	}

	if (over == max)
		return NULL;

	new_db = kzalloc(max - over, GFP_KERNEL);

	if (new_db) {
		j = 2;
		for (i = 0; i < max; i++) {
			if (!vic_over_spec(*pvic)) {
				new_db[j] = *pvic;
				j++;
			}
			pvic++;
		}

		new_db[0] = CEA_DB_TYPE(VIDEO_CAPABILITY_BLOCK, (max - over + 1));
		new_db[1] = VC_EXT_VFPDB;
	}

	/* do not release data, it should be done outside */
#if 0
	kfree(data);
#endif

	return new_db;
}

static void edid_patch_over_spec_ext_vc_db(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vc_db *entry, *next;
	struct list_head *lhead = &adapter->vc_db;
	u32 patch_cfg;

	if (!(patch & EDID_PATCH_VIDEO_OVER_SPEC_MASK))
		return;

	patch_cfg = patch & EDID_PATCH_VIDEO_EXT_VC_YUV420_MASK;

	list_for_each_entry_safe(entry, next, lhead, head) {
		if (vc_db_over_spec(entry)) {
			list_del(&entry->head);
			adapter->vc_db_size -= cea_db_payload_len(entry->data);
			/*
			 * 0xE: YCbCr 4:2:0 Video Data Block (Y420VDB)
			 * 0xF: YCbCr 4:2:0 Capability Map Data Block (Y420CMDB)
			 * Y420VDB => only support 420
			 * Y420CMDB => also support 420
			 *
			 * HDMI Spec. 2.0, Table 7-1, Video Timing that may be used with 420
			 *	3840x2160P (50Hz:96, 106; 60Hz:97, 107)
			 *	4096X2160P (50Hz:102; 60Hz:102)
			 *
			 * we already filter the timings listed above in video_db_over_spec(),
			 * just check VC_EXT_Y420CMDB in this function.
			 */
			/*
			 * Remove Y420CMDB
			 * (and create Y420VDB if EDID_PATCH_VIDEO_EXT_VC_YUV420_MASK is not defined)
			 */
			if ((vc_db_ext_tag(entry->data) == VC_EXT_Y420CMDB) && (!patch_cfg)) {
				new_n_add_vc_ext_y420vdb(adapter, entry->data);
			}

			ast_vc_db_free(entry);
		} else if (vc_db_ext_tag(entry->data) == VC_EXT_VFPDB) {
			u8 *data;

			list_del(&entry->head);
			adapter->vc_db_size -= cea_db_payload_len(entry->data);

			/*
			 * 7.5.12 Video Format Preference Data Block
			 * When present, the VFPDB shall take precedence over preferred indications defined elsewhere in CEA-861-F
			 * We have to eliminate VIC of VFPDB if it is over spec
			 */
			data = do_patch_vc_vfpdb(entry->data);
			if (data) {
				video_capability_db_add(adapter, data);
				kfree(data);
			}

			/*
			 * need entry->data for further process in do_patch_vc_vfpdb()
			 * ast_vc_db_free() must be placed after do_patch_vc_vfpdb()
			 */
			ast_vc_db_free(entry);
		}
	}

	if (list_empty(&adapter->vc_db))
		adapter->vc_db_size = 0;
}

static int ntsc(u32 rate)
{
	if ((rate % 6) == 0)
		return 1;

	return 0;
}

static int valid_mode_for_dtd(MODE_ITEM *mode)
{
	/*
	 * DTD
	 * active  12 bit (0xFFF)
	 *  blank  12 bit (0xFFF)
	 *    hbp  10 bit (0x3FF)
	 *  hsync  10 bit (0x3FF)
	 *    vbp   6 bit (0x03F)
	 *  vsync   6 bit (0x03F)
	 */
	if (mode->HActive > 0xFFF)
		return 0;

	if (mode->HBPorch > 0x3FF)
		return 0;

	if (mode->HSyncTime > 0x3FF)
		return 0;

	if ((mode->HBPorch + mode->HSyncTime + mode->HFPorch) > 0xFFF)
		return 0;

	if (mode->VActive > 0xFFF)
		return 0;

	if (mode->VBPorch > 0x03F)
		return 0;

	if (mode->VSyncTime > 0x03F)
		return 0;

	if ((mode->VBPorch + mode->VSyncTime + mode->VFPorch) > 0xFFF)
		return 0;

	return 1;
}

static int mode_to_dtd(MODE_ITEM *mode, struct detailed_timing *timing)
{
	struct detailed_pixel_timing *pt;
	u32 hblank, vblank;

	hblank = mode->HTotal - mode->HActive;
	vblank = mode->VTotal - mode->VActive;
	pt = &timing->pixel_data;

	pt->hactive_lo = mode->HActive & 0xFF;
	pt->hblank_lo = hblank & 0xFF;
	pt->hactive_hblank_hi = (((mode->HActive >> 4) & 0xF0) | (hblank >> 8 & 0x0F)); /* (ha >> 8) << 4 => ha >> 4 */
	pt->vactive_lo =  mode->VActive & 0xFF;
	pt->vblank_lo = vblank & 0xFF;
	pt->vactive_vblank_hi = (((mode->VActive >> 4) & 0xF0) | (vblank >> 8 & 0x0F));

	pt->hsync_offset_lo = mode->HFPorch & 0xFF;
	pt->hsync_pulse_width_lo = mode->HSyncTime & 0xFF;
	pt->vsync_offset_pulse_width_lo = ((mode->VFPorch & 0xF) << 4) | (mode->VSyncTime & 0xF);
	pt->hsync_vsync_offset_pulse_width_hi = ((mode->HFPorch & 0x300) << 6)
						| ((mode->HSyncTime & 0x300) << 4)
						| ((mode->VFPorch & 0x030) << 2)
						| (mode->VSyncTime & 0x030);
	timing->pixel_clock = mode->RefreshRate * mode->HTotal * mode->VTotal  / 10000;

	pt->misc &= 0x7F; /* clear [7]:interlaced */

	if ((pt->misc & 0x18) == 0x18) {
		/* [4:3] = 11 => digital separate */
		pt->misc &= 0xF9; /* clear [2:1], [2]: v polarity, [1]: h polarity */
		if (mode->VerPolarity == P_POS)
			pt->misc |= (0x1 << 2);
		if (mode->HorPolarity == P_POS)
			pt->misc |= (0x1 << 1);
	}
}

static int _vrefresh(u32 clock, u32 ht, u32 vt)
{
	int refresh = 0;
	unsigned int calc_val;

	if (ht > 0 && vt > 0) {

		/* work out vrefresh the value will be x1000 */
		calc_val = clock;
		calc_val /= ht;
		refresh = (calc_val + vt / 2) / vt;
#if 0
		/* Bruce160720. Since mode->VActive from DTD is half only, there is no need to double the refresh rate. */
		if (mode->ScanMode == Interl)
			refresh *= 2;
#endif
	}

	/*
	 * there is no 59.94 case in AstModeTable, fix refresh in case we got this kind of timing
	 */
	switch (refresh) {
	case 49: /* 49.xxx*/
	case 59: /* 59.xxx */
	case 119: /* 119.88 */
	case 239: /* 239.76 */
		refresh++;
		break;
	default:
		break;
	}
	return refresh;
}

static int mode_is_valid(MODE_ITEM *mode, u32 clock, u32 ha, u32 va, u32 hp, u32 vp, u32 ntsc, u32 dtd_check)
{
	u32 prefer_rate;

	if (mode->ScanMode == Interl)
		return 0;

	if (mode->HActive != ha)
		return 0;

	if (mode->VActive != va)
		return 0;

	if (mode->VerPolarity != vp)
		return 0;

	if (mode->HorPolarity != hp)
		return 0;

	if (mode->DCLK10000 > clock)
		return 0;

	if (ntsc) {
		/* NTSC prefer 60Hz */
		prefer_rate = 60;
	} else {
		/* PAL prefer 50Hz */
		prefer_rate = 50;
	}

	if ((ha >= 3840) && (va >= 2160))
		prefer_rate = prefer_rate / 2;

	if (mode->RefreshRate != prefer_rate)
		return 0;

	if (dtd_check)
		if (!valid_mode_for_dtd(mode))
			return 0;

	return 1;
}


static MODE_ITEM *mode_meet_spec(u32 spec_pixel_clock, u32 ha, u32 va, u32 hp, u32 vp, u32 ntsc, u32 dtd_check)
{
	MODE_ITEM *table = crt->mode_table, *mode, *candidate = NULL;
	u32 mt_size_hdmi = crt->mode_table_vesa_start;
	u32 mt_size_all = crt->mode_table_size;
	u32 i;

	/* search HDMI timing */
	for (i = 0; i < mt_size_hdmi; i++) {
		mode = table + i;
		if (mode_is_valid(mode, spec_pixel_clock, ha, va, hp, vp, ntsc, dtd_check) == 0)
			continue;

		candidate = mode;
		goto search_done; /* got, no need to search VESA timing */
	}

	/* search VESA timing */
	for (i = mt_size_hdmi; i < mt_size_all; i++) {
		mode = table + i;
		if (mode_is_valid(mode, spec_pixel_clock, ha, va, hp, vp, ntsc, dtd_check) == 0)
			continue;

		candidate = mode;
		break;
	}

search_done:
	return candidate;
}

#if defined(AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP)
static int dtd_patch_by_table_lookup(struct edid_adapter *adapter, struct detailed_timing *timing)
{
	struct detailed_pixel_timing *pt;
	MODE_ITEM *candidate = NULL;
	u32 ha, va, rate, hp, vp, is_ntsc;
	u32 spec_pixel_clock;
	struct detailed_timing *src, *dst, new;
	u32 digital, size;

	spec_pixel_clock = AST_SPEC_PIXEL_CLOCK();

	/*
	 * DTD pixel clock field = pixel clock / 10000
	 * our vesa table  is pixel clock / 100
	 */
	spec_pixel_clock = spec_pixel_clock * 100;

	pt = &timing->pixel_data;
	ha = T_H_ACTIVE(pt);
	va = T_V_ACTIVE(pt);
	vp = (pt->misc & DRM_EDID_PT_VSYNC_POSITIVE) ? PosPolarity : NegPolarity;
	hp = (pt->misc & DRM_EDID_PT_HSYNC_POSITIVE) ? PosPolarity : NegPolarity;
	rate = _vrefresh(timing->pixel_clock * 10000, ha + T_H_BLANK(pt), va + T_V_BLANK(pt));

	is_ntsc = ntsc(rate);

	if (pt->misc & DRM_EDID_PT_INTERLACED)
		goto search_done;

	candidate = mode_meet_spec(spec_pixel_clock, ha, va, hp, vp, is_ntsc, 1);

search_done:
	dst = timing;
	digital = (adapter->edid.input & DRM_EDID_INPUT_DIGITAL) ? 1 : 0;
	size = sizeof(struct detailed_timing) - 6; /* exclude last 6 bytes of DTD:image size, border and flag */

	if (candidate == NULL) {
		src = &dtd_default_1080p;
		if ((ast_scu.ability.v_support_4k >= 3) && digital) {
			if ((ha >= 3840) && (va >= 2160))
				src = &dtd_default_4k;
		}
	} else {
		memcpy(&new, timing, sizeof(struct detailed_timing));
		mode_to_dtd(candidate, &new);

		src = &new;
	}

	memcpy((u8 *)dst, (u8 *)src, size);

	/*
	 * last byte of DTD: flag
	 * [7]: 0: non-interlaced; 1:interlaced
	 * [4:1]: refer to EDID Standard, Table 3.18 - Sync Signal Description
	 */
	dst->pixel_data.misc &= 0x61;
	dst->pixel_data.misc |= (src->pixel_data.misc & 0x9E);
}
#endif /* #if defined(AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP) */

static void edid_patch_dtd_detailed_timing(struct edid_adapter *adapter, struct ast_detailed_mode *entry)
{
#if defined(AST_EDID_REMOVE_OVER_SPEC_DTD)
	/* just delete */
	list_del(&entry->head);
	adapter->ext_detailed_timing_num--;
	kfree(entry);
#else
#if defined(AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP)
	dtd_patch_by_table_lookup(adapter, &entry->timing);
#else /* #if defined(AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP) */
	struct detailed_timing *src, *dst;
	u32 digital, size;

	digital = (adapter->edid.input & DRM_EDID_INPUT_DIGITAL) ? 1 : 0;

	size = sizeof(struct detailed_timing) - 6; /* exclude last 6 bytes of DTD:image size, border and flag */

	if ((ast_scu.ability.v_support_4k >= 3) && digital)
		src = &dtd_default_4k;
	else
		src = &dtd_default_1080p;

	dst = &entry->timing;

	memcpy((u8 *)dst, (u8 *)src, size);

	/*
	 * last byte of DTD: flag
	 * [7]: 0: non-interlaced; 1:interlaced
	 * [4:1]: refer to EDID Standard, Table 3.18 - Sync Signal Description
	 */
	dst->pixel_data.misc &= 0x61;
	dst->pixel_data.misc |= (src->pixel_data.misc & 0x9E);
#endif /* #if defined(AST_EDID_PATCH_OVER_SPEC_DTD_BY_TABLE_LOOKUP) */
#endif
}

static void edid_patch_over_spec_dtd(struct edid_adapter *adapter, u32 patch)
{
	struct ast_detailed_mode *entry, *next;
	struct list_head *lhead;

	if (!(patch & EDID_PATCH_VIDEO_OVER_SPEC_MASK))
		return;

	lhead = &adapter->ext_detailed_modes;
	list_for_each_entry_safe(entry, next, lhead, head) {
		if (ast_dtd_over_spec(entry))
			edid_patch_dtd_detailed_timing(adapter, entry);
	}

	lhead = &adapter->detailed_modes;
	list_for_each_entry_safe(entry, next, lhead, head) {
		if (ast_dtd_over_spec(entry))
			edid_patch_dtd_detailed_timing(adapter, entry);
	}
}

static void do_patch_over_spec_dd(struct detailed_timing *t)
{
	struct detailed_non_pixel *p;

	if (t->pixel_clock)
		return;

	p = (struct detailed_non_pixel *) &t->pixel_data;

	if ((p->pad1 == 0) && (p->type == 0xFD) && (p->pad2 == 0)) {
		u32 spec_pixel_clock_mhz = AST_SPEC_PIXEL_CLOCK_MHZ();

		/* Monitor range limit Max. support pixel clock is MHz / 10 */
		if ((p->data.range.pixel_clock_mhz * 10) > spec_pixel_clock_mhz)
			p->data.range.pixel_clock_mhz = spec_pixel_clock_mhz / 10;
	}
}

static void edid_patch_over_spec_dd(struct edid_adapter *adapter, u32 patch)
{
	struct ast_detailed_mode *entry;
	int i;

	for (i = 0; i < EDID_DETAILED_TIMINGS; i++)
		do_patch_over_spec_dd(&adapter->edid.detailed_timings[i]);

	list_for_each_entry(entry, &adapter->ext_display_descriptors, head) {
		do_patch_over_spec_dd(&entry->timing);
	}
}

static void edid_patch_video_over_spec(struct edid_adapter *adapter, u32 patch)
{
	edid_patch_over_spec_dtd(adapter, patch); /* detailed timing descriptors */
	edid_patch_over_spec_dd(adapter, patch); /* display descriptors*/
	edid_patch_over_spec_ext_vc_db(adapter, patch);
	edid_patch_over_spec_ext_vs_db(adapter, patch);
	/*
	 * Keep edid_patch_over_spec_ext_video_db() in the end of this function
	 * because edid_patch_over_spec_ext_vc_db() needs original video DB information
	 */
	edid_patch_over_spec_ext_video_db(adapter, patch);
}

enum hdmi_audio_coding_type {
	HDMI_AUDIO_CODING_TYPE_STREAM,
	HDMI_AUDIO_CODING_TYPE_PCM,	/* LPCM */
	HDMI_AUDIO_CODING_TYPE_AC3,	/* AC-3 */
	HDMI_AUDIO_CODING_TYPE_MPEG1,	/* MPEG-1 */
	HDMI_AUDIO_CODING_TYPE_MP3,	/* MP3*/
	HDMI_AUDIO_CODING_TYPE_MPEG2,	/* MPEG2 */
	HDMI_AUDIO_CODING_TYPE_AAC_LC,	/* AAC LC */
	HDMI_AUDIO_CODING_TYPE_DTS,	/* DTS */
	HDMI_AUDIO_CODING_TYPE_ATRAC,	/* ATARC */
	HDMI_AUDIO_CODING_TYPE_DSD,	/* One Bit Audio, SACD */
	HDMI_AUDIO_CODING_TYPE_EAC3,	/* Enhanced AC-3, Dolby Digital Plus (DD+) */
	HDMI_AUDIO_CODING_TYPE_DTS_HD,	/* DTS-HD */
	HDMI_AUDIO_CODING_TYPE_MLP,	/* MAT, MLP/Dolby TrueHD */
	HDMI_AUDIO_CODING_TYPE_DST,	/* DST */
	HDMI_AUDIO_CODING_TYPE_WMA_PRO, /* WMA Pro */
	HDMI_AUDIO_CODING_TYPE_CXT,
};

/*
 * following SADs are from yamaha amp.
 *
 * HDFury's HDMI_AUDIO_CODING_TYPE_DTS_HD and HDMI_AUDIO_CODING_TYPE_MLP support all sample frequency
 * so add 32/44/88/176 kHz support for them
 *
 * LPCM 2-channel, 16/20/24 bit depths at 32/44/48/88/96/176/192 kHz
 * LPCM 8-channel, 16/20/24 bit depths at 32/44/48/88/96/176/192 kHz
 * AC-3 6-channel, 640k max. bit rate at 32/44/48 kHz
 * DTS 7-channel, 1536k max. bit rate at 32/44/48/88/96 kHz
 * DD+ 8-channel at 44/48 kHz
 * DTS-HD 8-channel, 16-bit at 32/44/48/88/96/176/192 kHz
 * MAT 8-channel at 32/44/48/88/96/176/192 kHz
 */
static SAD all_sound_sad_list[] = {
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_PCM, 2, 0x7F, 0x07) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_PCM, 8, 0x7F, 0x07) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_AC3, 6, 0x07, 640/8) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_DTS, 7, 0x1F, 1536/8) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_EAC3, 8, 0x06, 0x00) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_DTS_HD, 8, 0x7F, 0x01) },
	{ AST_SAD(HDMI_AUDIO_CODING_TYPE_MLP, 8, 0x7F, 0x00) },
};

static void edid_patch_audio_add_all_features(struct edid_adapter *adapter, u32 patch)
{
	int max, unit;
	u8 *data;

	if (!(patch & EDID_PATCH_AUDIO_ADD_ALL_MASK))
		return;

	/* clear original SADs*/
	audio_db_cleanup(adapter);

	/* add items in all_sound_sad_list */
	max = ARRAY_SIZE(all_sound_sad_list);
	unit = 3; /* SAD is 3-byte */

	data = kzalloc(max * unit + 1, GFP_KERNEL);

	if (data) {
		memcpy(data + 1, all_sound_sad_list, max * unit);
		data[0] = ((AUDIO_BLOCK << 5) | (max * unit));

		audio_db_add(adapter, data);
		kfree(data);
	}

	if (list_empty(&adapter->sa_db)) {
		u8 *sa;

		sa = kzalloc(4, GFP_KERNEL);
		if (sa) {
			sa[0] = CEA_DB_TYPE(SPEAKER_BLOCK, 3);
			sa[1] = 0x7F;
			sa[2] = 0x07;
			speaker_allocation_db_add(adapter, sa);
			kfree(sa);
		}
	} else {
		struct ast_sa_db *db;
		list_for_each_entry(db, &adapter->sa_db, head) {
			db->data[1] = 0x7F;
			db->data[2] = 0x07;
		}
	}
}

static bool ast_audio_over_spec(struct ast_audio_db *db)
{
	bool ret = false;
	u32 hbr = 1;
	SAD *psad;

	psad = (SAD *) db->sad;

	if (ast_scu.ability.soc_op_mode	== 1)
		hbr = 0;

	switch (psad->bits.format) {
	case HDMI_AUDIO_CODING_TYPE_PCM:	/* LPCM */
		/* TODO LPCM 16bit, 20bit */
		break;
	case HDMI_AUDIO_CODING_TYPE_AC3:	/* AC-3 */
	case HDMI_AUDIO_CODING_TYPE_MPEG1:	/* MPEG-1 */
	case HDMI_AUDIO_CODING_TYPE_MP3:	/* MP3*/
	case HDMI_AUDIO_CODING_TYPE_MPEG2:	/* MPEG2 */
	case HDMI_AUDIO_CODING_TYPE_AAC_LC:	/* AAC LC */
	case HDMI_AUDIO_CODING_TYPE_DTS:	/* DTS */
	case HDMI_AUDIO_CODING_TYPE_ATRAC:	/* ATARC */
		break;
	case HDMI_AUDIO_CODING_TYPE_DSD:	/* One Bit Audio, SACD */
		ret = true;
		break;
	case HDMI_AUDIO_CODING_TYPE_EAC3:	/* Enhanced AC-3, Dolby Digital Plus (DD+) */
		break;
	case HDMI_AUDIO_CODING_TYPE_DTS_HD:	/* DTS-HD */
		if (!hbr)
			ret = true;
		break;
	case HDMI_AUDIO_CODING_TYPE_MLP:	/* MAT, MLP/Dolby TrueHD */
		if (!hbr)
			ret = true;
		break;
	case HDMI_AUDIO_CODING_TYPE_DST:	/* DST */
	case HDMI_AUDIO_CODING_TYPE_WMA_PRO: /* WMA Pro */
		break;
	default: /* 0 and 15 */
		/* keep all except the format we cannot handle */
		ret = false;
		break;
	}

	return ret;
}

static void edid_patch_audio_over_spec(struct edid_adapter *adapter, u32 patch)
{
	struct ast_audio_db *entry, *next;

	if (!(patch & EDID_PATCH_AUDIO_OVER_SPEC_MASK))
		return;

	list_for_each_entry_safe(entry, next, &adapter->audio_db, head) {

		if (ast_audio_over_spec(entry)) {
			list_del(&entry->head);
			adapter->audio_db_size -= 3; /* SAD is 3-byte length */
			ast_audio_db_free(entry);
		}
	}

	if (list_empty(&adapter->audio_db))
		adapter->audio_db_size = 0;
}

static void edid_patch_audio_multi_ch(struct edid_adapter *adapter, u32 patch)
{
	struct ast_audio_db *db;
	SAD *psad;

	if (!(patch & EDID_PATCH_AUDIO_LPCM_CHAN_MASK))
		return;

	list_for_each_entry(db, &adapter->audio_db, head) {
		psad = (SAD *) db->sad;
		/* ch_num is max number of channel - 1 */
		if ((psad->bits.format == HDMI_AUDIO_CODING_TYPE_PCM) && (psad->bits.ch_num > 1))
			psad->bits.ch_num = 1;
	}
}

static void edid_patch_audio_high_sample_rate(struct edid_adapter *adapter, u32 patch)
{
	struct ast_audio_db *db;
	SAD *psad;

	if (!(patch & EDID_PATCH_AUDIO_LPCM_FREQ_MASK))
		return;

	list_for_each_entry(db, &adapter->audio_db, head) {
		psad = (SAD *) db->sad;
		if (psad->bits.format == HDMI_AUDIO_CODING_TYPE_PCM) {
			/* remove sample rate > 48KHz */
			psad->bits.KHz88 = 0;
			psad->bits.KHz96 = 0;
			psad->bits.KHz176 = 0;
			psad->bits.KHz192 = 0;
		}
	}
}

static bool audio_format_nlpcm(u8 format_code, u32 patch)
{
	u32 patch_cfg;

	patch_cfg = ((patch & EDID_PATCH_AUDIO_NLPCM_MASK) >> EDID_PATCH_AUDIO_NLPCM_SHIFT);

	if (patch_cfg == EDID_PATCH_AUDIO_NLPCM_ALL) {
		if (format_code != HDMI_AUDIO_CODING_TYPE_PCM)
			return true;
	} else if (patch_cfg == EDID_PATCH_AUDIO_NLPCM_HD) {
		switch (format_code) {
		case HDMI_AUDIO_CODING_TYPE_DSD:
		case HDMI_AUDIO_CODING_TYPE_DTS_HD:
		case HDMI_AUDIO_CODING_TYPE_MLP:
			return true;
		default:
			break;
		}
	}

	return false;
}

static void edid_patch_audio_nlpcm(struct edid_adapter *adapter, u32 patch)
{
	struct ast_audio_db *entry, *next;
	SAD *psad;

	if (!(patch & EDID_PATCH_AUDIO_NLPCM_MASK))
		return;

	list_for_each_entry_safe(entry, next, &adapter->audio_db, head) {
		psad = (SAD *) entry->sad;
		if (audio_format_nlpcm(psad->bits.format, patch)) {
			list_del(&entry->head);
			adapter->audio_db_size -= 3; /* SAD is 3-byte length */
			ast_audio_db_free(entry);
		}
	}

	if (list_empty(&adapter->audio_db))
		adapter->audio_db_size = 0;
}

static void edid_patch_audio(struct edid_adapter *adapter, u32 patch)
{
	edid_patch_audio_add_all_features(adapter, patch);

	/*
	 * all patchs related to elimination should get executed
	 * after edid_patch_audio_add_all_features()
	 */
	edid_patch_audio_over_spec(adapter, patch);
	edid_patch_audio_multi_ch(adapter, patch);
	edid_patch_audio_high_sample_rate(adapter, patch);
	edid_patch_audio_nlpcm(adapter, patch);
}

/*
 * edid_patch_add_vc_hdr -
 *
 * just in case we need to add HDR static metadata DB in future
 */
static void edid_patch_add_vc_hdr(struct edid_adapter *adapter, u32 patch)
{
	if (!(patch & EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK))
		return;

	if (patch & EDID_PATCH_VIDEO_DEL_EXT_VC_HDR_MASK) {
		/*
		 * EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK
		 * and EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK => exclusive
		 */
		return;
	}

	/*
	 * byte 2:
	 * [0]: SDR
	 * [1]: HDR
	 * [2]: SMPTE ST 2084
	 * [3]: future EOTF
	 * [5:4]: reserved for future use
	 * [7:6]: 0
	 */
	if (adapter->status->hdr) {
		struct ast_vc_db *entry;

		list_for_each_entry(entry, &adapter->vc_db, head) {
			if (cea_db_is_vcdb_hdr(entry->data)) {
				entry->data[2] = 0x5; /* ST 2084 and SDR */
				entry->data[3] = 0x1; /* staic metadata type 1 */
			}
		}
	} else {
		/* create VC DB with HDR extended tag code */
		u8 *new_db, len;

		len = 3; /* no optional data */

		new_db = kzalloc(len + 1, GFP_KERNEL);

		if (new_db) {
			new_db[0] = CEA_DB_TYPE(VIDEO_CAPABILITY_BLOCK, len);
			new_db[1] = VC_EXT_HDRDB;
			new_db[2] = 0x5; /* ST 2084 and SDR */
			new_db[3] = 0x1; /* staic metadata type 1 */
			video_capability_db_add(adapter, new_db);
			kfree(new_db);
		}
	}
}

/*
 * edid_patch_del_vc_hdr - delete VC DB HDR if exist
 */
static void edid_patch_del_vc_hdr(struct edid_adapter *adapter, u32 patch)
{
	if (!(patch & EDID_PATCH_VIDEO_DEL_EXT_VC_HDR_MASK))
		return;

	if (patch & EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK) {
		/*
		 * EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK
		 * and EDID_PATCH_VIDEO_ADD_EXT_VC_HDR_MASK => exclusive
		 */
		return;
	}

	if (adapter->status->hdr) {
		struct ast_vc_db *entry, *next;
		struct list_head *lhead;

		lhead = &adapter->vc_db;

		list_for_each_entry_safe(entry, next, lhead, head) {
			if (cea_db_is_vcdb_hdr(entry->data)) {
				list_del(&entry->head);
				adapter->vc_db_size -= cea_db_payload_len(entry->data);
				ast_vc_db_free(entry);
			}
		}
	}

}

static int vsdb_add_4k_24hz(struct ast_vs_db *db)
{
	u32 len = cea_db_payload_len(db->data);
	u8 *new_data, new_len, vic_len_offset, vic_offset, size_after_vic;

	if (len < 8) {
		new_len = 8 + 3;
		vic_len_offset = 8 + 2;
		vic_offset = vic_len_offset + 1;
		size_after_vic = 0;
	} else {
		if (db->data[8] & 0x20) {
			/*
			 * HDMI video present, 'HDMI_VIC_LEN + HDMI_3D_LEN' exist
			 * +1 to store VIC
			 */
			new_len = len + 1;
			vic_len_offset = 8 + 2;
			vic_offset = vic_len_offset + 1 + ((db->data[vic_len_offset] >> 5) & 0x7);
		} else {
			u8 offset = 8 + 1;

			if (db->data[8] & 0x80)
				offset += 2;

			if (db->data[8] & 0x40)
				offset += 2;

			vic_len_offset = offset + 1;
			vic_offset = vic_len_offset + 1;

			if (db->data[offset] & 0x80) {
				/*
				 * 3D present , 'HDMI_VIC_LEN + HDMI_3D_LEN' exist
				 * +1 to store VIC
				 */
				new_len = len + 1;
			} else {
				/*
				 * +2 to store 'HDMI_VIC_LEN + HDMI_3D_LEN' and VIC
				 */
				new_len = len + 2;
			}
		}
		size_after_vic = new_len - vic_offset;
	}

	new_data = kzalloc(new_len + 1, GFP_KERNEL);

	if (new_data) {
		u8 new_vic_len;

		memcpy(new_data, db->data, vic_len_offset - 1);

		new_data[8] |= (0x1 << 5);

		new_vic_len = ((new_data[vic_len_offset] >> 5) & 0x7) + 1;
		new_data[vic_len_offset] &= ~(0x7 << 5);
		new_data[vic_len_offset] |= (new_vic_len << 5);
		new_data[vic_offset] = 0x3;

		memcpy(new_data + vic_offset + 1, db->data + (len - vic_len_offset + 1), size_after_vic);


		/* pixel clock fixup */
		if ((new_data[7] * 5) < SPEC_4K_PIXEL_CLOCK_MHZ)
			new_data[7] = SPEC_4K_PIXEL_CLOCK_MHZ / 5;

		new_data[0] = CEA_DB_TYPE(VENDOR_BLOCK, new_len);

		kfree(db->data);
		db->data = new_data;
	}
}

/*
 * edid_patch_add_4k_24hz -
 *
 */
static void edid_patch_add_4k_24hz(struct edid_adapter *adapter, u32 patch)
{
	if (!(patch & EDID_PATCH_VIDEO_ADD_4K_24HZ_MASK))
		return;

	if (ast_scu.ability.v_support_4k < 3)
		return;

	if (edid_4k_24hz(adapter) == 0) {
		edid_status *status = adapter->status;

		if (status->hdmi_20) {
			u8 video_db_4k_24[2] = {CEA_DB_TYPE(VIDEO_BLOCK, 1), VIC_2160p24};

			video_db_add(adapter, video_db_4k_24);
		} else {
			struct ast_vs_db *db;

			list_for_each_entry(db, &adapter->vs_db, head) {
				if (cea_db_is_hdmi_vsdb(db->data)) {
					vsdb_add_4k_24hz(db);
					return;
				}
			}
		}
	}

}

/*
 * edid_patch_add_hf_vsdb_dc_420 - add YUV420 deep color feature in HDMI forum VSDB
 *
 */
static void edid_patch_add_hf_vsdb_dc_420(struct edid_adapter *adapter, u32 patch)
{
	edid_status *status;

	if (!(patch & EDID_PATCH_VIDEO_ADD_HF_VSDB_Y420_DC_MASK))
		return;

	status = adapter->status;

	if (status->hdmi_20) {
		struct ast_vs_db *db;

		list_for_each_entry(db, &adapter->vs_db, head) {
			if (cea_db_is_hdmi_forum_vsdb(db->data)) {
				/*
				 * byte 7
				 * [7:3]: 0
				 * [2]:DC_48bit_420, [1]:DC_36bit_420, [0]:DC_30bit_420
				 */
				db->data[7] |= 1;
			}
		}
	} else {
		u8 *db;

		db = kzalloc(1 + 7, GFP_KERNEL);

		if (db) {
			db[0] = CEA_DB_TYPE(VENDOR_BLOCK, 7);
			db[1] = 0xD8; db[2] = 0x5D; db[3] = 0xC4; /* IEEE OUI */
			db[4] = 1; /* version */
			db[5] = SPEC_4K_PIXEL_CLOCK_MHZ / 5; /* Max TMDS */
			db[6] = 0; /*  something we don't care now */
			db[7] = 0x1; /* [2]:DC_48bit_420, [1]:DC_36bit_420, [0]:DC_30bit_420 */

			vendor_specific_db_add(adapter, db);
			kfree(db);
		}
	}
}

static u8 vic_420_also[] = {
	VIC_2160p24, VIC_2160p25, VIC_2160p30
};


static void edid_add_svd(struct edid_adapter *adapter, u8 *vic_array, u32 vic_array_size)
{
	struct ast_video_db *entry;
	u32 i, vic, to_do;

	u8 *to_do_array, to_do_size;

	to_do_array = kzalloc(vic_array_size, GFP_KERNEL);

	if (to_do_array == NULL)
		return;

	to_do_size = 0;

	for (i = 0; i < vic_array_size; i++) {
		vic = vic_array[i];
		to_do = 1;

		list_for_each_entry(entry, &adapter->video_db, head) {
			if (entry->svd == vic) {
				to_do = 0;
				break;
			}
		}

		if (to_do) {

			to_do_array[to_do_size] = vic;
			to_do_size++;

		}
	}

	if (to_do_size) {
		u8 *video_db;

		video_db = kzalloc(1 + to_do_size, GFP_KERNEL);
		if (video_db == NULL)
			goto exit_free_to_do_array;

		video_db[0] = CEA_DB_TYPE(VIDEO_BLOCK, to_do_size);
		memcpy(video_db + 1, to_do_array, to_do_size);
		video_db_add(adapter, video_db);
		kfree(video_db);
	}

exit_free_to_do_array:
	kfree(to_do_array);
}

static void edid_add_y420_cmdb(struct edid_adapter *adapter, u8 *vic_array, u32 vic_array_size)
{
	struct ast_video_db *video_entry;
	struct ast_vc_db *entry, *next, *vc_entry;
	u32 i, vic;

	u8 *to_do_array, index;
	u8 *new_data, new_size;

	to_do_array = kzalloc(vic_array_size, GFP_KERNEL);

	if (to_do_array == NULL)
		return;

	memset(to_do_array, 0xFF, vic_array_size);

	for (i = 0; i < vic_array_size; i++) {
		vic = vic_array[i];
		index = 0;

		list_for_each_entry(video_entry, &adapter->video_db, head) {
			if (video_entry->svd == vic) {
				to_do_array[i] = index;
				break;
			}
			index++;
		}
	}

	vc_entry = NULL;

	list_for_each_entry_safe(entry, next, &adapter->vc_db, head) {
		if (vc_db_ext_tag(entry->data) == VC_EXT_Y420CMDB) {
			list_del(&entry->head);
			vc_entry = entry;
			break;
		}
	}

	new_size = ((adapter->video_db_size + 7) >> 3);
	new_data = kzalloc(new_size, GFP_KERNEL);

	new_data[0] = CEA_DB_TYPE(VIDEO_CAPABILITY_BLOCK, (new_size + 2));
	new_data[1] = VC_EXT_Y420CMDB;

	if (vc_entry) {
		memcpy(new_data + 2, vc_entry->data + 2, cea_db_payload_len(vc_entry->data) - 1 - 2);
		ast_vc_db_free(vc_entry);
	}

	for (i = 0; i < vic_array_size; i++) {
		if (to_do_array[i] != 0xFF)
			u8_array_bit_set(new_data + 2, to_do_array[i]);
	}

	video_capability_db_add(adapter, new_data);

	kfree(to_do_array);
}

/*
 * edid_patch_add_vc_420_also - add Y420 also-support block
 *
 * this patch add:
 * 1. VICs in vic_420_also[]
 * 2. Y420CMDB
 */
static void edid_patch_add_vc_420_also(struct edid_adapter *adapter, u32 patch)
{
	if (!(patch & EDID_PATCH_VIDEO_ADD_Y420CMDB_MASK))
		return;

	edid_add_svd(adapter, vic_420_also, ARRAY_SIZE(vic_420_also));
	edid_add_y420_cmdb(adapter, vic_420_also, ARRAY_SIZE(vic_420_also));
}

/* TBD, is it necessary to list all VICs which support 420 */
static u8 vic_420_only[] = {
	VIC_2160p50, VIC_2160p, VIC_2160p50E, VIC_2160pE, VIC_2160p50W1, VIC_2160pW1
};

/*
 * edid_patch_add_vc_420_only -
 *
 */
static void edid_patch_add_vc_420_only(struct edid_adapter *adapter, u32 patch)
{
	u32 size;
	u8 *db;

	if (!(patch & EDID_PATCH_VIDEO_ADD_EXT_VC_420_ONLY_MASK))
		return;

	size = ARRAY_SIZE(vic_420_only);

	db = kzalloc(size + 2, GFP_KERNEL);

	if (db) {
		struct ast_vc_db *entry, *next;
		struct list_head *lhead;

		db[0] = CEA_DB_TYPE(VIDEO_CAPABILITY_BLOCK, (size + 1));
		db[1] = VC_EXT_Y420VDB;
		memcpy(db + 2, vic_420_only, size);

		lhead = &adapter->vc_db;

		list_for_each_entry_safe(entry, next, lhead, head) {
			if (vc_db_ext_tag(entry->data) == VC_EXT_Y420VDB) {
				list_del(&entry->head);
				adapter->vc_db_size -= cea_db_payload_len(entry->data);
				ast_vc_db_free(entry);
			}
		}

		video_capability_db_add(adapter, db);
		kfree(db);
	}
}

static void edid_patch_colorimetry(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vc_db *entry, *next;
	struct list_head *lhead;

	if (!(patch & EDID_PATCH_VIDEO_COLORIMETRY_MASK))
		return;

	lhead = &adapter->vc_db;

	list_for_each_entry_safe(entry, next, lhead, head) {

		if (vc_db_ext_tag(entry->data) == VC_EXT_COLORIMETRYDB) {
			list_del(&entry->head);
			adapter->vc_db_size -= cea_db_payload_len(entry->data);
			ast_vc_db_free(entry);
		}
	}

	if (list_empty(&adapter->vc_db))
		adapter->vc_db_size = 0;
}

static void edid_patch_misc(struct edid_adapter *adapter, u32 patch)
{
	struct ast_vs_db *db;

	if (!patch)
		return;

	/* always patch DVI_Dual */
	list_for_each_entry(db, &adapter->vs_db, head) {
		if (cea_db_is_hdmi_vsdb(db->data)) {
			u32 len = cea_db_payload_len(db->data);

			if (len >= 6) {
				/* byte 6[0] is DVI_Dual */
				db->data[6] &= 0xFE;
			}
		}
	}

	/*
	 * patch manufacture ID to AST
	 *
	 * this is a special situation we got a problem in LAB: samsung 4K player + samsung 4K TV.
	 *
	 * samsung manufacture ID + specified serial number always causes samsung 4K player display 4K60Hz YUV420
	 * even if the data block related to timing of EDID get patched already
	 *
	 * we guess that there is a special list includes 'manufacture ID + serial number' in samsung player
	 * For samsung player, this list precedence over what timing defined in EDID
	 */
	adapter->edid.mfg_id[0] = 0x06;
	adapter->edid.mfg_id[1] = 0x74;
}

static int edid_checksum(struct edid_adapter *adapter)
{
	u32 i;
	u8 sum = 0, *dst;

	dst = (u8 *)&adapter->edid;

	adapter->edid.checksum = 0;

	for (i = 0; i < EDID_BLOCK_SIZE; i++)
		sum += dst[i];

	adapter->edid.checksum = 0x100 - sum;

	return 0;
}

static void edid_patch_fixup_detailed_timing(struct edid_adapter *adapter)
{
	struct ast_detailed_mode *entry;
#if defined(AST_EDID_REMOVE_OVER_SPEC_DTD)
	struct ast_detailed_mode *next;
#endif
	struct list_head *lhead;
	u32 to_do, index;
#if defined(AST_EDID_REMOVE_OVER_SPEC_DTD)
	u32 digital;
#endif
	u8 *src, *dst;

	/*
	 * From E-EDID Standard, 3.10
	 *	The first descriptor block shall be used to indicate the display's preferred timing mode
	 *	A Monitor Range Limits Descriptor must be provided
	 *	A Monitor Name Descriptor must be provided
	 *
	 * Example A: Preferred Detailed Timing, Detailed Timing 2, Monitor Name, Monitor Range Limits.
	 * Example B: Preferred Detailed Timing, Monitor Serial Number, Monitor Range Limits, Monitor Name
	 */

	to_do = adapter->orig_detailed_timing_num;
	index = 0;

	/* Hosts may encounter displays using EDID version 1 revision 0-2 which do not meet all of these requirements. */
#if 0
	if (to_do > 2)
		to_do = 2;
#endif

#if defined(AST_EDID_REMOVE_OVER_SPEC_DTD)
	/* only check this case if our patch do remove DTD */
	if (to_do < 1)
		to_do = 1;
#endif

	lhead = &adapter->detailed_modes;
	list_for_each_entry(entry, lhead, head) {
		if (to_do) {
			src = (u8 *)&entry->timing;
			dst = (u8 *) (adapter->edid.detailed_timings + index);
			memcpy(dst, src, sizeof(struct detailed_timing));
			index++;
			to_do--;
		} else
			return;
	}

#if defined(AST_EDID_REMOVE_OVER_SPEC_DTD)
	lhead = &adapter->ext_detailed_modes;
	list_for_each_entry_safe(entry, next, lhead, head) {
		if (to_do) {
			src = (u8 *)&entry->timing;
			dst = (u8 *) (adapter->edid.detailed_timings + index);
			memcpy(dst, src, sizeof(struct detailed_timing));
			index++;
			to_do--;
			list_del(&entry->head);
			adapter->ext_detailed_timing_num--;
			list_add_tail(&entry->head, &adapter->detailed_modes);
			adapter->detailed_timing_num++;
		} else
			return;
	}

	digital = (adapter->edid.input & DRM_EDID_INPUT_DIGITAL) ? 1 : 0;

	/*
	 * From ANSI-CTA-861-F, A2.11.1
	 * In Block 0, all four descriptor blocks are required by VESA EDID standard [9] to be filled with valid data,
	 * even if it means repeating a timing descriptor.
	 */
	while (to_do) {
#ifdef AST_EDID_PREFER_4K_SUPPORT
		if ((ast_scu.ability.v_support_4k >= 3) && digital)
			src = (u8 *) &dtd_default_4k;
		else
			src = (u8 *) &dtd_default_1080p;
#else
		src = (u8 *) &dtd_default_1080p;
#endif

		dst = (u8 *) (adapter->edid.detailed_timings + index);
		memcpy(dst, src, sizeof(struct detailed_timing));
		index++;
		to_do--;
	}
#endif /* #if defined(AST_EDID_REMOVE_OVER_SPEC_DTD) */
}

static u32 video_db_size_with_tag(u32 size)
{
	u32 ret = 0;

	if (size) {
		ret = size + 1; /* data + tag */
		/* if need more than 1 db store SVD */
		ret += (size >> 5); /* 1 video_db maximum to 31 SVD */
	}

	return 0;
}

static u32 audio_db_size_with_tag(u32 size)
{
	u32 ret = 0;

	if (size) {
		ret = size + 1; /* data + tag */
		/* if need more than 1 db store SAD */
		/* 1 audioo_db maximum to 10 SAD (10*3) */
		while (size > 30) {
			ret++;
			size -= 30;
		}
	}

	return ret;
}

static void edid_patch_fixup_extension(struct edid_adapter *adapter)
{
	u32 ext_size;

	/*
	 * From ANSI-CTA-861-F, 7.5 CEA Extension Version 3
	 * The lower 4 bits of byte 3 indicates the total number of DTDs
	 * defining Native Video Formats in the whole EDID (see Section 2.2 for definition of "Native Video Format").
	 *
	 * The placement of native DTDs shall be contiguous,
	 * starting with the first DTD in the DTD list (which starts in the base EDID block).
	 */
	if (adapter->ext_header.native_num
		> (adapter->detailed_timing_num + adapter->ext_detailed_timing_num))
		adapter->ext_header.native_num = adapter->detailed_timing_num + adapter->ext_detailed_timing_num;

	ext_size = audio_db_size_with_tag(adapter->audio_db_size)
		+ video_db_size_with_tag(adapter->video_db_size)
		+ adapter->vs_db_size
		+ adapter->sa_db_size
		+ adapter->vc_db_size
		+ adapter->others_db_size
		+ adapter->ext_detailed_timing_num * DETAILED_TIMING_DESCRIPTION_SIZE
		+ adapter->ext_display_descriptor_num * DETAILED_TIMING_DESCRIPTION_SIZE
		+ 4 /* tag, revision, DTD offset, #of DTD */
		+ 1 /* checksum */
		;

	if (ext_size > EDID_BLOCK_SIZE) {
#if defined(AST_EDID_DEBUG)
		printk("audio_db_size %d, video_db_size %d, vs_db_size %d, sa_db_size %d, vc_db_size %d, others_db_size %d, ext_detailed_timing_num %d\n",
			audio_db_size_with_tag(adapter->audio_db_size),
			video_db_size_with_tag(adapter->video_db_size),
			adapter->vs_db_size,
			adapter->sa_db_size,
			adapter->vc_db_size,
			adapter->others_db_size,
			adapter->ext_detailed_timing_num
		);
#endif
		if (adapter->others_db_size) {
			ext_size -= adapter->others_db_size;
			others_db_cleanup(adapter);
			if (ext_size <= EDID_BLOCK_SIZE)
				return;
		}

		if (adapter->ext_display_descriptor_num) {
			ext_size -= (adapter->ext_display_descriptor_num * DETAILED_TIMING_DESCRIPTION_SIZE);
			ext_monitor_descriptor_cleanup(adapter);
			if (ext_size <= EDID_BLOCK_SIZE)
				return;
		}

		if (adapter->ext_detailed_timing_num) {
			ext_size -= (adapter->ext_detailed_timing_num * DETAILED_TIMING_DESCRIPTION_SIZE);
			ext_detailed_mode_cleanup(adapter);
			if (ext_size <= EDID_BLOCK_SIZE)
				return;
		}

		/*
		 * TODO
		 * the extension block size still > 128 byte
		 * this case will get fixed in cea_db_to_edid()
		 */
	}
}

static void edid_patch_hdmi_physical_address(struct edid_adapter *adapter)
{
	struct ast_vs_db *db;

	list_for_each_entry(db, &adapter->vs_db, head) {
		if (cea_db_is_hdmi_vsdb(db->data)) {
			/* force physical address to 1.0.0.0 */
			db->data[4] = 0x10;
			db->data[5] = 0x00;
		}
	}
}

static void edid_patch_fixup(struct edid_adapter *adapter)
{
	edid_status *status = adapter->status;

	if (status->checksum_error)
		return;

	/* mandatory */
	edid_patch_fixup_detailed_timing(adapter);

	/* extensions */
	edid_patch_fixup_extension(adapter);

	if (status->cea_ext_valid)
		adapter->edid.extensions = 1;
	else
		adapter->edid.extensions = 0;

	/* physical address */
	edid_patch_hdmi_physical_address(adapter);

	edid_checksum(adapter);
}

static u32 do_edid_patch(struct edid_adapter *adapter, const u32 patch)
{
	u32 patch_misc = 1;
	edid_status *status = adapter->status;

	if (status->checksum_error)
		return 0;

	if (status->cea_ext_valid && status->cea_ext_checksum_error)
		return 0;

	edid_patch_interlaced(adapter, patch);
	edid_patch_deepcolor(adapter, patch);
	edid_patch_3d(adapter, patch);

	edid_patch_video_over_spec(adapter, patch);

	edid_patch_audio(adapter, patch);

	/*
	 * YUV420 deep color is defined in HDMI forum VSDB
	 * for 4K 24Hz, we also need to add Y420CMDB to support YUV420
	 *
	 * with following patch, samsung player didn't output 4K24Hz YUV420
	 */
	edid_patch_add_hf_vsdb_dc_420(adapter, patch);
	edid_patch_add_vc_420_also(adapter, patch);

	/*
	 * Samsung player HDR behavior (with HDR patch and 4K 24Hz patch)
	 *   Player  |     EDID      |   EDID    |
	 *    24Fs   | 4k 60 420only | PCM 8 chs |  HDR
	 * ----------+---------------+-----------+---------------
	 *     Y           N              N         4K 24Hz
	 *     Y           N              Y         1080P 24Hz
	 *     Y           Y              N         4K 24Hz
	 *     Y           Y              Y         4K 24Hz
	 *     N           N              N         1080P 60Hz
	 *     N           N              Y         1080P 60Hz
	 *     N           Y              N         4K 60Hz
	 *     N           Y              Y         4K 60Hz
	 *
	 * Note:
	 * For two tested discs(MAD MAX/ THE REVENANT),
	 * we always get complain (the dialog) when output is not 4K HDR even if 1080P 60Hz HDR
	 * MAD MAX: dialog disappear after user press some key
	 * THE REVENANT: disappear itself in short time (may miss this information)
	 *
	 * still no idea why get 1080P24Hz HDR when (PCM 8 chs && NO 420 only DB)
	 *
	 * For HDR support of samsung player, we need:
	 * 1. VC DB: HBR
	 * 2. VC DB: 420 only
	 * 3. 4K 24Hz timing (1.4b:VSDB, 2.0: SVD) for quality
	 */
	edid_patch_add_vc_hdr(adapter, patch);
	edid_patch_add_vc_420_only(adapter, patch);
	edid_patch_add_4k_24hz(adapter, patch);

	/* delete VC HDR*/
	edid_patch_del_vc_hdr(adapter, patch);
	edid_patch_colorimetry(adapter, patch);

	edid_patch_misc(adapter, patch_misc);

	edid_patch_fixup(adapter);

	return 0;
}

static int video_mode_vrefresh(VIDEO_MODE_INFO *mode)
{
	return _vrefresh(mode->PCLK * 100, mode->HTotal, mode->VTotal);
}


static u32 timing_index_from_detailed_timing(struct detailed_timing *timing, u32 digital, int *preset_match)
{
	VIDEO_MODE_INFO mode_info, *mode = &mode_info;
	struct detailed_pixel_timing *pt;
	u32 index;

	pt = &timing->pixel_data;

	/*
	 * timing->pixel_clock: unit of 10KHz
	 * mode->PCLK: unit of 100Hz
	 */

	mode->PCLK = le16_to_cpu(timing->pixel_clock) * 100;

	mode->HActive = T_H_ACTIVE(pt);
	mode->HTotal = mode->HActive + T_H_BLANK(pt);
	mode->HBackPorch = T_H_SYNC_OFFSET(pt);
	mode->HSyncWidth = T_H_SYNC_W(pt);
	mode->HFrontPorch = T_H_BLANK(pt) - mode->HBackPorch - mode->HSyncWidth;

	mode->VActive = T_V_ACTIVE(pt);
	mode->VTotal = mode->VActive + T_V_BLANK(pt);
	mode->VBackPorch = T_V_SYNC_OFFSET(pt);
	mode->VSyncWidth = T_V_SYNC_W(pt);
	mode->VFrontPorch = T_V_BLANK(pt) - mode->VBackPorch - mode->VSyncWidth;

	mode->ScanMode = (pt->misc & DRM_EDID_PT_INTERLACED) ? Interl : Prog;
	mode->VPolarity = (pt->misc & DRM_EDID_PT_VSYNC_POSITIVE) ? PosPolarity : NegPolarity;
	mode->HPolarity = (pt->misc & DRM_EDID_PT_HSYNC_POSITIVE) ? PosPolarity : NegPolarity;
	mode->RefreshRate = video_mode_vrefresh(mode);
	mode->SignalType = digital ? s_HDMI_16x9 : s_RGB;

	index = crt_match_timing_table(mode, preset_match);

	return index;
}

/* from hdmi_vic_to_idx() */
static u32 timing_index_from_vic(u32 vic, u32 *match)
{
	unsigned int sn;

	if (match)
		*match = 0;

	sn = crt_timing_hdmi_vic_to_sn(vic);

	if (sn == TIMING_SN_INVALID)
		goto not_found;

	if (match)
		*match = 1;

	return crt_timing_sn_to_index(sn);

not_found:
	return crt_timing_sn_to_index(PREFERRED_DEFAULT_TIMING_SN);
}

static u32 timing_index_from_vc_vfpdb(struct edid_adapter *adapter, u32 *match)
{
	struct ast_vc_db *entry;
	u32 index = 0;

	list_for_each_entry(entry, &adapter->vc_db, head) {
		if (vc_db_ext_tag(entry->data) == VC_EXT_VFPDB) {
			u8 *pvic;
			u32 index, i, max;

			max = cea_db_payload_len(entry->data) - 1;
			pvic = entry->data + 2;
			for (i = 0; i < max; i++) {
				index = timing_index_from_vic(*pvic, match);
				if (*match)
					return index;
				pvic++;
			}
		}
	}
	return index;
}

static u32 timing_index_from_video_db(struct edid_adapter *adapter, u32 *match)
{
	struct ast_video_db *entry;
	u32 index = 0;

	list_for_each_entry(entry, &adapter->video_db, head) {
		index = timing_index_from_vic(svd_to_vic(entry->svd), match);
		if (*match)
			return index;
	}
	return index;
}

/*
 *
 * ast_preferred_timing_index - return timing index of preferred timing not over spec
 * @timing_index:
 *
 * this function checks video timing by timing_index.
 * if timing is over spec, return a replaced items in our vesa table instead
 *
 */
static u32 ast_preferred_timing_index(u32 timing_index)
{
	MODE_ITEM *mode, *candidate = NULL;
	u32 spec_pixel_clock, is_ntsc, index;

	index = timing_index;

	spec_pixel_clock = AST_SPEC_PIXEL_CLOCK();

	spec_pixel_clock = spec_pixel_clock * 100; /* our vesa table is in 100Hz */

	mode = crt_timing_by_index(index);

	if (mode->DCLK10000 > spec_pixel_clock) {
		is_ntsc = ntsc(mode->RefreshRate);

		candidate = mode_meet_spec(spec_pixel_clock, mode->HActive, mode->VActive, mode->HorPolarity, mode->VerPolarity, is_ntsc, 0);

		if (candidate)
			index = crt_timing_sn_to_index(mode->sn);
		else
			index = crt_timing_sn_to_index(crt_timing_hdmi_vic_to_sn(VIC_1080p));
	}

	return index;
}

/*
 * edid_preferred_timing_index - return timing index of preferred timing
 * @adapter:
 *
 * we just check detailed timings now
 */
static u32 edid_preferred_timing_index(struct edid_adapter *adapter)
{
	u32 index = DEFAULT_TIMING_IDX, match = 0;
	u32 digital;
	struct detailed_timing *timing;
	edid_status *status = adapter->status;

	digital = (adapter->edid.input & DRM_EDID_INPUT_DIGITAL) ? 1 : 0;

	/*
	 * From ANSI-CTA-861-F, 7.5 CEA Extension Version 3
	 * As with the Version 1 CEA Extension,
	 * the first detailed timing (DTD) listed in the base EDID data structure of the Version 3 CEA Extension is preferred.
	 * The first Short Video Descriptor (SVD), listed in the first CEA Extension, is also preferred
	 *
	 * Note: base EDID is block 0
	 *
	 * 7.5.12 Video Format Preference Data Block
	 * When present, the VFPDB shall take precedence over preferred indications defined elsewhere in CEA-861-F
	 */

	/* Ext. VC VFPDB is 1st priority*/
	index = timing_index_from_vc_vfpdb(adapter, &match);
	if (match)
		goto prefer_done;

	/*
	 * 7.5 CEA Extension Version 3
	 * NOTE: Because DTDs are not able to represent some Video Formats,
	 * which can be represented as SVDs and might be preferred by Sinks,
	 * the first DTD in the base EDID data structure and the first SVD in the first CEA Extension can differ.
	 * When the first DTD and SVD do not match
	 * and the total number of DTDs defining Native Video Formats in the whole EDID is zero
	 * (see Table 41, byte 3, lower 4 bits), the first SVD shall take precedence.
	 *
	 * Note: native number includes DTD in block 0
	 * The lower 4 bits of byte 3 indicates the total number of DTDs defining Native Video Formats in the whole EDID
	 * (see Section 2.2 for definition of "Native Video Format").
	 * The placement of native DTDs shall be contiguous, starting with the first DTD in the DTD list (which starts in the base EDID block).
	 */

	/* native != 0, choose detailed mode */
	if (adapter->ext_header.native_num /* cea_ext_valid:1, cea_ext_checksum_error: 0 */
		|| status->cea_ext_checksum_error /* cea_ext_valid:1 */
		|| !status->cea_ext_valid /* no CEA ext. */
		) {
		struct ast_detailed_mode *entry;
		struct list_head *lhead;

		lhead = &adapter->detailed_modes;
		list_for_each_entry(entry, lhead, head) {
			timing = &entry->timing;
			index = timing_index_from_detailed_timing(timing, digital, &match);

			if (match)
				goto prefer_done;
		}

		lhead = &adapter->ext_detailed_modes;
		list_for_each_entry(entry, lhead, head) {
			timing = &entry->timing;
			index = timing_index_from_detailed_timing(timing, digital, &match);

			if (match)
				goto prefer_done;
		}
	}

	/* native is '0' or DTD not found => SVD */
	index = timing_index_from_video_db(adapter, &match);
	if (match)
		goto prefer_done;

	/* no one matched, just use default timing */
#ifdef AST_EDID_PREFER_4K_SUPPORT
	if ((ast_scu.ability.v_support_4k >= 3) && digital)
		index = crt_timing_sn_to_index(PREFERRED_DEFAULT_4K_TIMING_SN);
	else
		index = crt_timing_sn_to_index(PREFERRED_DEFAULT_TIMING_SN);
#else
	index = crt_timing_sn_to_index(PREFERRED_DEFAULT_TIMING_SN);
#endif
prefer_done:
	status->preferred_timing_index_raw = index;
	status->preferred_timing_index = ast_preferred_timing_index(index);

	return 0;
}

static void data_init(struct edid_adapter *adapter)
{
	memset(adapter, 0, sizeof(struct edid_adapter));

	INIT_LIST_HEAD(&adapter->detailed_modes);
	INIT_LIST_HEAD(&adapter->audio_db);
	INIT_LIST_HEAD(&adapter->video_db);
	INIT_LIST_HEAD(&adapter->vs_db);
	INIT_LIST_HEAD(&adapter->sa_db);
	INIT_LIST_HEAD(&adapter->vc_db);
	INIT_LIST_HEAD(&adapter->others_db);
	INIT_LIST_HEAD(&adapter->ext_detailed_modes);
	INIT_LIST_HEAD(&adapter->ext_display_descriptors);
}

static void edid_destroy(struct edid_adapter *adapter)
{
	misc_cleanup(adapter);
	detailed_mode_cleanup(adapter);
	ext_db_cleanup(adapter);
	ext_detailed_mode_cleanup(adapter);
	ext_monitor_descriptor_cleanup(adapter);
}

static void edid_status_update(struct edid_adapter *adapter)
{
	edid_status *p = adapter->status;

	p->digital = (adapter->edid.input & DRM_EDID_INPUT_DIGITAL) ? 1 : 0;

	p->yuv = 0;

	/*
	 * color format in mandatory block (offset 0x18) is only available in EDID 1.4
	 * we use the definition in CEA extension block (after rev 1.2) instead
	 */
	if ((adapter->status->cea_ext_valid) && (!adapter->status->cea_ext_checksum_error)) {
		if ((adapter->ext_header.revision >= 2) && (adapter->ext_header.ycbcr444)) {
			/*
			 * set yuv when sink support YUV444
			 * (do not set yuv if sink support YUV422 only)
			 */
			p->yuv = 1;
		}
	}

	edid_preferred_timing_index(adapter);
}

static void edid_parse(struct edid_adapter *adapter, const u8 *raw_edid)
{
	const u8 *cea;
	struct edid *edid = (struct edid *)raw_edid;
	int i;
	edid_status *status = adapter->status;

	memset(status, 0, sizeof(edid_status));

	memcpy(&adapter->edid, raw_edid, sizeof(struct edid));
	status->valid = 1;

	if (drm_edid_block_checksum(raw_edid))
		status->checksum_error = 1;

	if (status->checksum_error)
		return;

	for (i = 0; i < EDID_DETAILED_TIMINGS; i++) {
		if (detailed_mode_add(adapter, &edid->detailed_timings[i], &adapter->detailed_modes) == 0) {
			adapter->detailed_timing_num++;
			adapter->orig_detailed_timing_num++;
		}
	}

	/* extension block */
	cea = drm_find_cea_extension(edid);

	if (cea) {
		status->cea_ext_valid = 1;
		if ((drm_edid_block_checksum(cea) == 0) && (edid_cea_parse(adapter, cea) == 0)) {
			memcpy(&adapter->ext_header, cea, sizeof(struct ast_cea_ext_header));
		} else {
			adapter->ext_data = kzalloc(EDID_LENGTH, GFP_KERNEL);

			if (adapter->ext_data)
				memcpy(adapter->ext_data, cea, EDID_LENGTH);
			else
				status->cea_ext_valid = 0;

			status->cea_ext_checksum_error = 1;
		}
	}

	edid_status_update(adapter);
#if defined(AST_EDID_DEBUG)
	printk("<%s> valid %d checksum_error %d cea_ext_valid %d cea_ext_checksum_error %d digital %d yuv %d hdr %d preferred_timing_index %d\n",
		__func__,
		status->valid,
		status->checksum_error,
		status->cea_ext_valid,
		status->cea_ext_checksum_error,
		status->digital,
		status->yuv,
		status->hdr,
		status->preferred_timing_index);
#endif
}

static int cea_audio_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_audio_db *db;
	u32 done = 0, size = 3, tag_index;
	u32 ret_fix = 0;
	u8 *loc;

	tag_index = index;
	loc = dst + index + 1;

	list_for_each_entry(db, &adapter->audio_db, head) {
		/*
		 * Audio Data Block: (tag, length) + SAD1 + SAD2 + ... + SADn
		 * 1: for (tag, length)
		 */
		if ((tag_index + 1 + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;

		memcpy(loc, db->sad, size);
		loc += size;
		done += size;

		if (done == 30) {
			/*
			 * only 5 bits for length field,
			 * one audio data block maximum to include 10 SAD (10 * 3)
			 * complete this db (and prepare for consequent one)
			 */
			dst[tag_index] = CEA_DB_TYPE(AUDIO_BLOCK, done);
			ret_fix += (done + 1); /* data + tag */
			tag_index = index + ret_fix;
			done = 0;
		}
	}

	if ((ret_fix) || (done)) {
		if (done) {
			dst[index] = CEA_DB_TYPE(AUDIO_BLOCK, done);
			ret_fix += 1; /* tag */
		}
		return ret_fix + done;
	}

	return 0;
}

static int cea_video_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_video_db *db;
	u32 done = 0, size = 1, tag_index;
	u32 ret_fix = 0;
	u8 *loc;

	tag_index = index;
	loc = dst + index + 1;

	list_for_each_entry(db, &adapter->video_db, head) {
		/*
		 * Video Data Block: (tag, length) + SVD1 + SVD2 + ... + SVDn
		 * 1: for (tag, length)
		 */
		if ((tag_index + 1 + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;

		memcpy(loc, &db->svd, size);
		loc += size;
		done += size;

		if (done == 31) {
			/*
			 * only 5 bits for length field, one video data block maximum to include 31 SVD
			 * complete this db (and prepare for consequent one)
			 */
			dst[tag_index] = CEA_DB_TYPE(VIDEO_BLOCK, done);
			ret_fix += (done + 1); /* data + tag */
			tag_index = index + ret_fix;
			done = 0;
		}
	}

	if ((ret_fix) || (done)) {

		if (done) {
			dst[tag_index] = CEA_DB_TYPE(VIDEO_BLOCK, done);
			ret_fix += 1; /* tag */
		}

		return ret_fix + done;
	}

	return 0;
}

static int cea_vs_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_vs_db *db;
	u32 done = 0, size;
	u8 *loc = dst + index;

	list_for_each_entry(db, &adapter->vs_db, head) {
		size = cea_db_payload_len(db->data) + 1;
		if ((index + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;

		memcpy(loc, db->data, size);

		loc += size;
		done += size;
	}

	return done;
}

static int cea_sa_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_sa_db *db;
	u32 done = 0, size;
	u8 *loc = dst + index;

	list_for_each_entry(db, &adapter->sa_db, head) {
		size = cea_db_payload_len(db->data) + 1;

		if ((index + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;
		memcpy(loc + done, db->data, size);
		loc += size;
		done += size;
	}

	return done;
}

static int cea_vc_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_vc_db *db;
	u32 done = 0, size;
	u8 *loc = dst + index;

	list_for_each_entry(db, &adapter->vc_db, head) {
		size = cea_db_payload_len(db->data) + 1;
		if ((index + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;
		memcpy(loc, db->data, size);
		loc += size;
		done += size;
	}

	return done;
}

static int cea_others_db_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_others_db *db;
	u32 done = 0, size;
	u8 *loc = dst + index;

	list_for_each_entry(db, &adapter->others_db, head) {
		size = cea_db_payload_len(db->data) + 1;
		if ((index + done + size) > CEA_EXT_DATA_OFFSET_END)
			break;
		memcpy(loc, db->data, size);
		loc += size;
		done += size;
	}

	return done;
}

static int cea_detailed_to_edid(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	struct ast_detailed_mode *mode;
	u32 done = 0, unit;
	u8 *loc = dst + index;
	struct ast_cea_ext_header *header;

	unit = sizeof(struct detailed_timing);
	list_for_each_entry(mode, &adapter->ext_detailed_modes, head) {
		if ((index + done + unit) > CEA_EXT_DATA_OFFSET_END)
			break;

		memcpy(loc, &mode->timing, unit);
		loc += unit;
		done += unit;
	}

	header = (struct ast_cea_ext_header *) dst;

	header->dtd_offset = index;

	/*
	 * From ANSI-CTA-861-F, Table 41 CEA Extension Version 3
	 * d = offset for the byte following the reserved data block.
	 * If no data is provided in the reserved data block, then d=4.
	 * If d=0, then no detailed timing descriptors are provided
	 * and no data is provided in the reserved data block collection.
	 */
	if ((done == 0) && (index == CEA_EXT_DATA_OFFSET_START))
		header->dtd_offset = 0;

	return done;
}

static int cea_padding_and_checksum(struct edid_adapter *adapter, u8 *dst, u32 index)
{
	u32 padding_size, i;
	u8 sum = 0;

	if (index >= EDID_BLOCK_SIZE) {
		printk(" !!!!!!! <%s> index %d\n", __func__, index);
		return -1;
	}

	padding_size = EDID_BLOCK_SIZE - index;

	memset(dst + index, 0, padding_size);

	for (i = 0; i < EDID_BLOCK_SIZE; i++)
		sum += dst[i];

	dst[EDID_BLOCK_SIZE - 1] = 0x100 - sum;

	return 0;
}

static void cea_db_to_edid(struct edid_adapter *adapter, u8 *dst)
{
	u32 index;

	memcpy(dst, &adapter->ext_header, sizeof(struct ast_cea_ext_header));

	index = CEA_EXT_DATA_OFFSET_START;

	/*
	 * From ANSI-CTA-861-F, 7.5 CEA Extension Version 3
	 * The format of the “CEA Data Block Collection” shall conform to that shown in Table 42.
	 * => Video, Audio, SA, VS, VC
	 */
	index += cea_video_db_to_edid(adapter, dst, index);
	index += cea_audio_db_to_edid(adapter, dst, index);
	index += cea_sa_db_to_edid(adapter, dst, index);
	index += cea_vs_db_to_edid(adapter, dst, index);
	index += cea_vc_db_to_edid(adapter, dst, index);
	index += cea_others_db_to_edid(adapter, dst, index);

	/* detailed timing descriptors are in the end of ext. block */
	index += cea_detailed_to_edid(adapter, dst, index);
	cea_padding_and_checksum(adapter, dst, index);
}

static void raw_dump(u8 *dst)
{
#if defined(AST_EDID_DEBUG)
	int i;

	for (i = 0; i < 128; i++) {
		printk(" %.2x", dst[i]);
		if (0 == ((i + 1) & 0xf))
			printk("\n");
	}
#endif
}

static u32 edid_create(struct edid_adapter *adapter, u8 *dst)
{
	u8 *cea;
	edid_status *status = adapter->status;

	if (status->valid == 0)
		return 0;

	memcpy(dst, &adapter->edid, sizeof(struct edid));

	raw_dump(dst);

	cea = dst + EDID_LENGTH;

	if (status->cea_ext_valid == 0) {
		memset(cea, 0, EDID_LENGTH);
		return 0;
	}

	if (status->cea_ext_checksum_error)
		memcpy(cea, &adapter->ext_data, EDID_LENGTH);
	else
		cea_db_to_edid(adapter, cea);

	raw_dump(cea);

	return 0;
}

static u32 edid_patch(struct edid_adapter *adapter, u8 *edid, const u32 patch)
{
	if (patch) {
		edid_parse(adapter, edid);
		do_edid_patch(adapter, patch);
		edid_create(adapter, edid);
	}

	return 0;
}

static void sys_edid_parse(const u8 *raw_edid, edid_status *status)
{
	unsigned long flags;

	spin_lock_irqsave(&edid_lock, flags);
	_adapter->status = status;
	edid_parse(_adapter, raw_edid);
	edid_destroy(_adapter);
	spin_unlock_irqrestore(&edid_lock, flags);
}

static u32 sys_edid_patch(u8 *edid, const u32 patch)
{
	unsigned long flags;
	edid_status tmp_edid_status;

	spin_lock_irqsave(&edid_lock, flags);
	_adapter->status = &tmp_edid_status;
	edid_patch(_adapter, edid, patch);
	edid_destroy(_adapter);
	spin_unlock_irqrestore(&edid_lock, flags);
	return 0;
}

/* */
static struct ast_edid_ops ast_edid = {
	.parse = sys_edid_parse,
	.patch = sys_edid_patch,
};

/**
 * ast_edid_init() -
 */
void ast_edid_init(struct s_crt_drv *crt)
{
	data_init(_adapter);
	crt->ast_edid = &ast_edid;
	spin_lock_init(&edid_lock);
}
EXPORT_SYMBOL(ast_edid_init);
#endif /* #if (AST_EDID_PARSER_VER == 2) */
