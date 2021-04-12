
#if (CONFIG_AST1500_SOC_VER >= 3)

#ifndef _DESC_MODE_H_
#define _DESC_MODE_H_

#include <linux/list.h>
#include <asm/arch/drivers/ast_hw_udp.h>

#define DESC_NET_MODE 1 //0:standard socket. 1:HW UDP direct to MAC
#define DESC_SYNC_WITH_VSYNC 1

/*
** #define DESC_RX_DESC_NUM 512 ==> Should be ok
** #define DESC_RX_DESC_NUM 1024 ==> Tested at 873Mbps (not bottleneck)
** #define DESC_RX_DESC_NUM 2048 ==> OOM. Bruce150626. No OOM from my test and we need 2048 for huge frame.
** #define DESC_RX_DESC_NUM 2048 ==> Bruce151027. OOM on H pattern + 4K + Video mode change. Huge frame is about 5MB which we can only queue one frame using 1024 desc (x8K).
*/
#define DESC_RX_DESC_NUM	1024 /* 2048 // 512 // 1024//128// 64   //Must be 2^N */

/*
** DESC_TX_DESC_NUM 16 ==> Tx performance up to 120Mbps
** DESC_TX_DESC_NUM 32 ==> Tx performance up to ??Mbps
** DESC_TX_DESC_NUM 64 ==> Tx performance up to 447Mbps
** DESC_TX_DESC_NUM 128 ==> Tx performance over 873Mbps. Currently bound to Rx.
*/
#define DESC_TX_DESC_NUM 128                              //Must be 2^N

#define DESC_FLG_VSYNC_PKT_MODE 2 //0:merge 1:separate desc queue 2:Pure HW mode
#define DESC_MTU_JUMBO	8128	/* (62+1)x128+64. Must be 128x(N+1)+64 bytes */
#define DESC_MTU_NORMAL	1472	/* (10+1)x128+64. Must be 128x(N+1)+64 bytes */
#define DESC_MTU DESC_MTU_JUMBO

#if (DESC_NET_MODE == 0)
#define DESC_DUMMY_HDR 1
#define DESC_FLG_VSYNC_PKT 0
#define DESC_VSYNC_SEQ_CHECK 0
#else
#define DESC_DUMMY_HDR 0
#define DESC_FLG_VSYNC_PKT 1
#define DESC_VSYNC_SEQ_CHECK 0
#endif

#define OWN_SW (0)
#define OWN_HW (1)

struct parse_result_t {
	u32 parse_result;
	u32 frame_size; // only valid on frame start
	u32 time_stamp; // only valid on frame start. time stamp of frame start
} __attribute__ ((packed));

//For little-endian only.
struct ve_desc_t {
	//DW0
	u32 buf_size:14,
	    :2,
#if 0 //A0
	    compress_complete_line:8,
	    :2,
#else
	    compress_complete_line:10,
#endif
		stream_decode_error:1, // D[26]
		frame_hdr_decode_error:1, //D[27]
	    is_frame_end:1, //D[28]
	    is_frame_start:1, //D[29]
	    last:1,
	    own:1; // 1:HW 0:SW
	//DW1
	u32 :31,
	    interrupt:1; //Insert interrupt when finished
	//DW2
	u32 :32;
	//DW3
	u32 buf_badr;

	/* Start of SW private data. MUST be 4xDW size. */
	struct skb_entry *skbe;
	struct parse_result_t pr;
} __attribute__ ((packed));
#define VE_DESC_SIZE (sizeof(struct ve_desc_t)) //32bytes

/*
** VE frame header contains 16 bytes HW header and N bytes of SW header.
** The base address must be 16 bytes aligned.
*/
#define AHU_SW_HEADER_SIZE (256)
struct ve_sw_frame_hdr_t {
	PacketInfo frame_info;
} __attribute__ ((packed));

struct ve_frame_hdr_t {
	__u32 hdr_id; //0x16881A03
	__u16 frame_cnt_h;
	__u16 frame_cnt_l;
	__u16 time_stamp_m;
	__u16 time_stamp_l;
	/* ---------------------------------------------------------- */
	__u16 profile_num:3;
	__u16 profile_en:1;
	__u16 is_full_frame:1;
	__u16 aes_on:1;
	__u16 is_jpeg_frame:1;
	__u16 is_420:1;
	__u16 user:8;
	/* ---------------------------------------------------------- */
	__u16 time_stamp_h;

	struct ve_sw_frame_hdr_t sw_frame_hdr[0];
} __attribute__ ((packed)); //16 bytes aligned

struct ve_desc_context_t {
	struct ve_desc_t *ve_desc;
	dma_addr_t ve_desc_dma_addr;
	struct ve_frame_hdr_t *ve_frame_hdr;
	dma_addr_t ve_frame_hdr_dma_addr;

#if (DESC_NET_MODE == 0)
	spinlock_t lock; //generic driver lock
	struct list_head skbe_pool;
	struct skb_entry *skbe_list_buf;
	char *skbe_buf;
	dma_addr_t skbe_buf_dma_addr;
#endif

	unsigned int sw_wrtr_idx; //software own index. The index to kick data to VE. writer ptr
	unsigned int desc_num; // MUST be 2^n
	unsigned int mtu;

	unsigned int ss_copy_offset; //snap shot offset
	unsigned int ss_size; //snap shot completed file size
#if DESC_FLG_VSYNC_PKT
	unsigned int genlck_log_cnt; //Can't over CRT_SNAPSHOT_SIZE/log_entry_size
	unsigned int genlck_last_fc;
	unsigned int genlck_last_location;
	int genlck_diff_sum;
	int genlck_diff_entry_cnt;
	unsigned int genlck_max_diff;
#endif

#if (DESC_SYNC_WITH_VSYNC == 1)
	// sw_rdr_idx <= sw_wrtr_idx <= sw_tmp_h_idx <= sw_tmp_t_idx
	unsigned int sw_rdr_idx; //The index to free kicked data. sw_rd_idx <= sw_idx. read ptr
	unsigned int sw_tmp_h_idx; //The head index of new coming partial frame skbe.
	unsigned int sw_tmp_t_idx; //The tail index of new coming partital frame skbe. New coming skbe enqueued here.
#endif

};


#endif //#ifndef _DESC_MODE_H_

#ifndef NO_FUNC_DECLAR
/*******************************************************************************
*	Start of exported function calls
*******************************************************************************/
void desc_debug(VIDEOIP_DEVICE *v, unsigned int flag);
void desc_dbg_dump(struct ve_desc_context_t *c);
unsigned int jpgss_snapshot_size(VIDEOIP_DEVICE *v);
unsigned char *jpgss_snapshot_base_addr(void);
void jpgss_take_snapshot(VIDEOIP_DEVICE *v);

int desc_pkt_ready(VIDEOIP_DEVICE *v);
int desc_pkt_ready_to_recycle(VIDEOIP_DEVICE *v);

#if defined(CONFIG_ARCH_AST1500_HOST)
void reset_compress_desc_dma_mode(VIDEOIP_DEVICE *v);
void init_compress_desc_dma_mode(VIDEOIP_DEVICE *v);
void start_compress_desc_dma_mode(VIDEOIP_DEVICE *v);
void ve_set_ahu_sw_hdr_frame_rate(VIDEOIP_DEVICE *v);
void ve_set_ahu_hdr(VIDEOIP_DEVICE *v);
int mtu_changed(VIDEOIP_DEVICE *v);
void V1TxHost_loop_desc(struct videoip_task *ut);
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT)
void free_decompress_desc(VIDEOIP_DEVICE *v);
int alloc_decompress_desc(VIDEOIP_DEVICE *v);
int realloc_decompress_desc_if_mtu_changed(VIDEOIP_DEVICE *v);
int video_stream_net_rx_work(VIDEOIP_DEVICE *v);
int video_complete_decompress_packets(VIDEOIP_DEVICE *v);
void init_decompress_desc_dma_mode(VIDEOIP_DEVICE *v);
void halt_decompress_desc_dma_mode(VIDEOIP_DEVICE *v);
unsigned int genlck_log_size(VIDEOIP_DEVICE *v);
unsigned char *genlck_log_base_addr(void);
void genlck_log_restart(VIDEOIP_DEVICE *v, unsigned int log_cnt);
void sw_flip_reset(void);
u32 desc_frame_drop(void);
void _handle_decode_ve_hang_desc(VIDEOIP_DEVICE *v, struct ve_desc_context_t *c);

#if SYNC_WITH_VSYNC
void client_dequeue_frame_desc(void);
#endif /* #if SYNC_WITH_VSYNC */

#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */

#endif /* #ifndef NO_FUNC_DECLAR */

#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

