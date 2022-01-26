/*
 * Copyright (c) 2004-2012
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */
#include <linux/platform_device.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/random.h>
#include <asm/semaphore.h>
#include <aspeed/features.h>
#include <asm/arch/gpio.h>
#include "ioaccess.h"
#include "i2s_types.h"
#include "i2s.h"
#include <asm/arch/drivers/crt.h>
#include "../../hdmirx/hdmirx_export.h"
#include "i2s_task.h"
#include "i2s_cmd.h"
#include "i2s_network.h"
#include "client_mgnt.h"
#include "i2s_event.h"
#if (CONFIG_AST1500_SOC_VER == 1)
#include "ast1500i2s.h"
#endif
#if (CONFIG_AST1500_SOC_VER >= 2)
#include "ast1510i2s.h"
#include "i2s_clk.h"
#endif
#include "codec_hal.h"
#if HDCP2_Encryption
#include <asm/arch/drivers/aes.h>
#include "../../videoip/driver/hdcp2_sm.h"
#endif

#if defined(I2S_CLK_INFO_FROM_REMOTE)
#include <asm/arch/drivers/video_hal.h>
#endif

#include <asm/arch/ast-scu.h>

MODULE_LICENSE ("GPL");

static int useI2S = 0;//whether audio is available
static unsigned int HDMI_available = 0;//whether HDMI is available.
static unsigned int Codec_available = 0;//whether audio CODEC is available.
static unsigned int upstream_available = 0;//whether upstream audio is available
static u32 i2s_tx = 0;
static u32 i2s_rx = 0;
int IO_in_Use = IO_IN_USE_NONE;
int i2sInit = 0;

static Audio_Info rx_settings;
static Audio_Info tx_settings;
#if defined(CONFIG_ARCH_AST1500_HOST)
static Audio_Info hdmi_settings;
static Audio_Info codec_settings;
static unsigned int hdmi_settings_valid = 0;
#endif

static int ioaccess_major = 249;

unsigned int rx_data_available = 0;
wait_queue_head_t	rx_wait_queue;
static unsigned int net_tx_idx = 0;

int is_client = 0;
struct socket *rx_data_socket = NULL;

typedef struct _s_i2s_hdr {
	unsigned int magic; //0x49325344
	unsigned int from; //from which host
	unsigned int cfg_sn; //config serial number
	unsigned int pkt_size;
	int sample_frequency_in_ppm;
#if HDCP2_Encryption
	unsigned char HDCP2_encrypted;
	unsigned char inputCtr[HDCP2_AES_INPUTCTR_LENGTH];
#endif
	union {
		unsigned char raw[24];
		Audio_Info info; //audio info
	} a;
} i2s_hdr, *pi2s_hdr;

#define I2S_HDR_SIZE sizeof(i2s_hdr)

i2s_hdr i2s_rx_hdr_buf;
i2s_hdr i2s_tx_hdr_buf;
unsigned int current_sender = 0;
unsigned int current_cfg_sn = 0;
unsigned long the_src_stable_timeout = 192; //in msecs
unsigned long the_drop_period = 5000; //in msecs. After audio change, ignore drop packets messages for a period of time.

unsigned int mic_input_pin = CODEC_INPUT_MIC1;

static struct i2s_task rx_data_t;
static struct i2s_task tx_data_t;
#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
static unsigned int clock_adj_interval = 3000; /* in msecs */
static struct i2s_task clock_adj_t;
wait_queue_head_t	clock_adj_wait_queue;
#endif /* #if defined(I2S_CLOCK_ADJUST_IN_THREAD) */

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
static u32 i2s_dual_output_en = 1;
#endif
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
struct list_head ctrl_event_list;
spinlock_t ctrl_event_list_lock;
unsigned int ctrl_event_list_size = 0;
#endif

unsigned int rx_ready = 0;
unsigned int tx_ready = 0;
unsigned int rx_enabled = 0;

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static struct s_crt_drv *crt;
#endif

static unsigned int rx_buf_size_shift;
static struct dma_pool *rx_buf_pool = NULL;
static char rx_buf_pool_name[] = "Rx_pool";
static void *rx_buf_va[NUM_RX_DESCRIPTOR];
static dma_addr_t rx_buf_pa[NUM_RX_DESCRIPTOR];
static unsigned int tx_buf_size_shift;
static struct dma_pool *tx_buf_pool = NULL;
static char tx_buf_pool_name[] = "Tx_pool";
static void *tx_buf_va[NUM_TX_DESCRIPTOR];
static dma_addr_t tx_buf_pa[NUM_TX_DESCRIPTOR];
static unsigned int max_net_num_rx;
static unsigned int max_net_num_tx;
static unsigned int max_udp_xfer_size = 0x8000;
static Audio_Buf Rx_Buffer, Tx_Buffer;
#if (BOARD_DESIGN_VER_I2S >= 204)
static struct workqueue_struct *wq = NULL;
static struct work_struct hotplug_work;
#define CODEC_IN_STATUS_UNKNOWN		0
#define CODEC_IN_STATUS_PLUGGED		1
#define CODEC_IN_STATUS_UNPLUGGED	2
static unsigned int codec_in_status = CODEC_IN_STATUS_UNKNOWN;
#define IO_SELECT_AUTO		0
#define IO_SELECT_CODEC	1
#define IO_SELECT_HDMI		2
static unsigned int IO_Select = IO_SELECT_AUTO;
#endif /* #if (BOARD_DESIGN_VER_I2S >= 204) */
#if (CONFIG_AST1500_SOC_VER >= 2)
static unsigned int reset_dead_audio_engine = 1;
#endif
#if HDCP2_Encryption
#ifdef CONFIG_ARCH_AST1500_HOST
static unsigned int enable_aes_encryption = 1;
#endif
static unsigned int debug_aes_encryption = 0;
#endif

static void tx_data(struct i2s_task *i2st);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
static int client_handle_audio_chg(Audio_Info *pAudio_info);
#endif
static int analog_in_vol = -1, analog_out_vol = -1;

typedef struct {
	unsigned int sample_rate_index; /* used by CAT6023 & CAT6613 */
	unsigned int sample_rate;
} sample_rate_entry;

static sample_rate_entry sample_rate_table[] = {
	{0, 44100},	/* 44.1KHz */
	{2, 48000},	/* 48KHz */
	{3, 32000},	/* 32KHz */
	{8, 88200},	/* 88.2KHz */
	{10, 96000},	/* 96KHz */
	{12, 176400},	/* 176.4KHz */
	{14, 192000}	/* 192KHz */
};

static unsigned int sample_rate_num = sizeof(sample_rate_table) / sizeof(sample_rate_entry);
struct semaphore i2s_user_lock;

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	.name = (char *) "1500_i2s", /* should be the same name as driver_name */
	.id = -1,
	.dev = {
		.release = the_pdev_release,
	},
};

#if defined(I2S_CLOCK_DETECT)
/*
 * just show how much time I2S engine take to generate audio data buffer
 *
 * for example: 48K sample frequency, there are 64 sample in single-one buffer
 * ideally, I2S engine need (64 / 48K) seconds,
 * In 24MHz (free run counter) unit, it is (24M * 64 / 48K) = 32000
 * if sample number is 100 => 32000x100 = 3200000
 * so the deviation is abs(result - 3200000)/3200000
 */
#define SCU_FREE_RUN_COUNTER (ASPEED_SCU_BASE + 0xE0)
#define I2S_DETECT_SAMPLE_CNT 100
static u32 start_time, end_time, i2s_detect_cnt = I2S_DETECT_SAMPLE_CNT + 1;
#endif /* #if defined(I2S_CLOCK_DETECT) */

irqreturn_t i2s_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int rx_int;
	int handled = audio_int_handler(&rx_int);

	if ((rx_int) && (rx_enabled)) {

#ifndef I2S_CLOCK_DETECT
		rx_data_available = 1;
		wake_up_interruptible(&rx_wait_queue);
		disable_rx_int();
#else
		if (i2s_detect_cnt < (I2S_DETECT_SAMPLE_CNT + 1)) {
			if (i2s_detect_cnt == 0)
				start_time = read_register(SCU_FREE_RUN_COUNTER);

			i2s_detect_cnt++;
			if (i2s_detect_cnt == (I2S_DETECT_SAMPLE_CNT + 1)) {
				end_time = read_register(SCU_FREE_RUN_COUNTER);
				if (end_time > start_time)
					printk("\t%d\n", end_time - start_time);
				else
					printk("\t%d\n", 0xffffffff - start_time + end_time);
			}
		} else {
			rx_data_available = 1;
			wake_up_interruptible(&rx_wait_queue);
			disable_rx_int();
		}
#endif
	}

	if (handled)
		return IRQ_HANDLED;
	else
		return IRQ_NONE;
}

#ifdef CONFIG_ARCH_AST1500_HOST
static void _dump_audio_info(Audio_Info *ai)
{
	int i;
	u32 freq = ai->SampleFrequency;

	I2S_info("Audio_On = %d\n", ai->Audio_On);
	if (!ai->Audio_On)
		return;

	/* Sink should don't care bit[4:5] first */
	switch (freq & 0xF) {
	case AST_FS_32000: //	(3)	/*   32 kHz, b--0011 */
		I2S_info("Sample Freq %d = 0x%02x = 32 kHz, b--0011\n", freq, freq);
		break;

	case AST_FS_44100: //	(0)	/*   44.1 kHz, b--0000 */
		I2S_info("Sample Freq %d = 0x%02x = 44.1 kHz, b--0000\n", freq, freq);
		break;
	case AST_FS_88200: //	(8)	/*   88.2 kHz, b--1000 */
		I2S_info("Sample Freq %d = 0x%02x = 88.2 kHz, b--1000\n", freq, freq);
		break;
	case AST_FS_176400: //	(12)	/*  176.4 kHz, b--1100 */
		I2S_info("Sample Freq %d = 0x%02x = 176.4 kHz, b--1100\n", freq, freq);
		break;

	case AST_FS_48000: //	(2)	/*   48 kHz, b--0010 */
		I2S_info("Sample Freq %d = 0x%02x = 48 kHz, b--0010\n", freq, freq);
		break;
	case AST_FS_96000: //	(10)	/*   96 kHz, b--1010 */
		I2S_info("Sample Freq %d = 0x%02x = 96 kHz, b--1010\n", freq, freq);
		break;
	case AST_FS_192000: //	(14)	/*  192 kHz, b--1110 */
		I2S_info("Sample Freq %d = 0x%02x = 192 kHz, b--1110\n", freq, freq);
		break;

	case AST_FS_768000: //	(9)	/*  768 kHz, b--1001 */
		I2S_info("Sample Freq %d = 0x%02x = 768 kHz, b--1001\n", freq, freq);
		break;
	/* Check new defined sample freq. */
	default:
		switch (freq) {
			case AST_FS_64000: //	(11)	/*   64 kHz, b001011 */
				I2S_info("Sample Freq %d = 0x%02x = 64 kHz, b001011\n", freq, freq);
				break;
			case AST_FS_128000: //	(43)	/*  128 kHz, b101011 */
				I2S_info("Sample Freq %d = 0x%02x = 128 kHz, b101011\n", freq, freq);
				break;
			case AST_FS_256000: //	(27)	/*  256 kHz, b011011 */
				I2S_info("Sample Freq %d = 0x%02x = 256 kHz, b011011\n", freq, freq);
				break;
			case AST_FS_512000: //	(59)	/*  512 kHz, b111011 */
				I2S_info("Sample Freq %d = 0x%02x = 512 kHz, b111011\n", freq, freq);
				break;
			case AST_FS_1024000: //	(53)	/* 1024 kHz, b110101 */
				I2S_info("Sample Freq %d = 0x%02x = 1024 kHz, b110101\n", freq, freq);
				break;

			case AST_FS_352800: //	(13)	/*  352.8 kHz, b001101 */
				I2S_info("Sample Freq %d = 0x%02x = 352.8 kHz, b001101\n", freq, freq);
				break;
			case AST_FS_705600: //	(45)	/*  705.6 kHz, b101101 */
				I2S_info("Sample Freq %d = 0x%02x = 705.6 kHz, b101101\n", freq, freq);
				break;
			case AST_FS_1411200: //	(29)	/* 1411.2 kHz, b011101 */
				I2S_info("Sample Freq %d = 0x%02x = 1411.2 kHz, b011101\n", freq, freq);
				break;

			case AST_FS_384000: //	(5)	/*  384 kHz, b000101 */
				I2S_info("Sample Freq %d = 0x%02x = 384 kHz, b000101\n", freq, freq);
				break;
			case AST_FS_1536000: //	(21)	/* 1536 kHz, b010101 */
				I2S_info("Sample Freq %d = 0x%02x = 1536 kHz, b010101\n", freq, freq);
				break;
			default:
				I2S_info("Sample Freq = Unknown!?\n");
				break;
		}
		break;
	}

	I2S_info("ValidCh(layout) = %d\n", ai->ValidCh);
	I2S_info("SampleSize = %d\n", ai->SampleSize);
	I2S_info("Audio_Type = %X\n", ai->Audio_Type);

	if (ai->Audio_Type & AST_AUDIO_HBR)
		I2S_info("Audio_Type = HBR\n");
	if (ai->Audio_Type & AST_AUDIO_DSD)
		I2S_info("Audio_Type = DSD\n");
	if (ai->Audio_Type & AST_AUDIO_NLPCM)
		I2S_info("Audio_Type = NLPCM\n");

	for (i = 0; i < 5; i++)
		I2S_info("AUD_DB[%d] = %02X\n", i, ai->AUD_DB[i]);

	for (i = 0; i < 5; i++)
		I2S_info("IEC60958ChStat[%d] = %02X\n", i, ai->ucIEC60958ChStat[i]);
}

void hdmi_aduio_rx_callback(Audio_Info a_info)
{
#if (API_VER >= 2)
	int i;
#endif

	I2S_info("audio Rx callback:\n");
	_dump_audio_info(&a_info);

	if (a_info.Audio_On) {
		if (ast_scu.ability.soc_op_mode == 1) {
			if (a_info.ValidCh > 2) {
				/*
				** Bruce120517. RctBug#2011092200.
				** We still pass the audio change to the client. But The client will
				** disable audio under HBR case.
				** If we don't pass this audio change to client, we can't avoid
				** LPCM --> HBR NLPCM noise issue.
				*/
				if ((a_info.Audio_Type & (AST_AUDIO_HBR | AST_AUDIO_NLPCM)) == (AST_AUDIO_HBR | AST_AUDIO_NLPCM))
					I2S_err("AST1500 does not support NLPCM audio with more than 2 channels!!!");

				a_info.ValidCh = 2;
			}
		}
#if (API_VER >= 2)
		hdmi_settings.Audio_Type = a_info.Audio_Type;
#endif
		hdmi_settings.SampleFrequency = a_info.SampleFrequency;
		hdmi_settings.ValidCh = a_info.ValidCh;
		hdmi_settings.SampleSize = a_info.SampleSize;
#if (API_VER >= 2)
		for (i = 0; i < 5; i++)
			hdmi_settings.AUD_DB[i] = a_info.AUD_DB[i];

		for (i = 0; i < 5; i++)
			hdmi_settings.ucIEC60958ChStat[i] = a_info.ucIEC60958ChStat[i];
#endif
	}
	hdmi_settings.Audio_On = a_info.Audio_On;
	hdmi_settings_valid = 1;
	if (IO_in_Use == IO_IN_USE_HDMI)//just record HDMI audio setting if audio is through CODEC
	{
		add_event(EVENT_HDMI_SETTINGS_CHANGED, NULL, NULL);
	}
}
#endif

static int ioaccess_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 1, err = 0;
	IO_ACCESS_DATA  Kernal_IO_Data;

	memset(&Kernal_IO_Data, 0, sizeof(IO_ACCESS_DATA));

	Kernal_IO_Data = *(IO_ACCESS_DATA *)arg;

	down(&i2s_user_lock);

	switch (cmd) {
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	case IOCTL_I2S_TX: /* client */
		if (useI2S) {
			I2S_info("IOCTL_I2S_TX (%d,%d,%d)\n", Kernal_IO_Data.transport, Kernal_IO_Data.I2sSocket, Kernal_IO_Data.CtrlSocket);
			tx_data_t.arg = sockfd_lookup(Kernal_IO_Data.I2sSocket, &err);
			start_i2s_thread(&tx_data_t);
#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
			start_i2s_thread(&clock_adj_t);
#endif
			/* Kernal_IO_Data.CtrlSocket == -1 means no upstream */
			if (Kernal_IO_Data.CtrlSocket != -1) {
				/*
				 * Although rx_data_socket is used for upstream_available only,
				 * but should always be released when calling i2s_cleanup
				 */
				rx_data_socket = sockfd_lookup(Kernal_IO_Data.CtrlSocket, &err);
				if (upstream_available) {
					start_i2s_thread(&rx_data_t);
					wake_up_interruptible(&rx_wait_queue);
				}
			}
		} else
			I2S_err("Audio hardware is unavailable!!!");
		ret = 0;
		break;
#endif

#if defined(CONFIG_ARCH_AST1500_HOST)
	case IOCTL_I2S_RX: /* host */
		if (useI2S) {
			I2S_info("IOCTL_I2S_RX (%d,%d)\n", Kernal_IO_Data.transport, Kernal_IO_Data.I2sSocket);
			start_i2s_thread(&rx_data_t);
			rx_data_socket = sockfd_lookup(Kernal_IO_Data.I2sSocket, &err);
			if (!Kernal_IO_Data.Data) { /* not multicast */
				I2S_info("unicast\n");
#if (BOARD_DESIGN_VER_I2S >= 204)
				if (upstream_available)
					codec_settings.enable_upstream = 1;

				if (IO_in_Use == IO_IN_USE_CODEC)
					add_event(EVENT_CODEC_SETTINGS_CHANGED, NULL, NULL);
#endif
			} else
				I2S_info("multicast\n");

			if (rx_ready) {
				enable_rx();
				wake_up_interruptible(&rx_wait_queue);
			}
		} else
			I2S_err("Audio hardware is unavailable!!!");
		ret = 0;
		break;
	case IOCTL_ADD_CLIENT: /* host */
		if (useI2S) {
			I2S_info("IOCTL_ADD_CLIENT (%d)\n", Kernal_IO_Data.CtrlSocket);

			if (upstream_available) {
				tx_data_t.arg  = sockfd_lookup(Kernal_IO_Data.CtrlSocket, &err);
				start_i2s_thread(&tx_data_t);
			}
		} else
			I2S_err("Audio hardware is unavailable!!!");
		ret = 0;
		break;
#endif
	default:
		ret = 3;
	}
	up(&i2s_user_lock);
	return ret;
}

static void i2s_stop_tx(void)
{
	unsigned long status;

	if (tx_data_t.thread != NULL) {
		stop_i2s_thread(&tx_data_t);
		I2S_info("i2s tx data thread stopped\n");
	}

	if (tx_data_t.arg != NULL) {
		status = ((struct socket *)(tx_data_t.arg))->ops->shutdown(tx_data_t.arg, 2);
		I2S_info("data socket shutdown (%lu)\n", status);
	}

	if (tx_data_t.arg != NULL) {
		sockfd_put(((struct socket *)(tx_data_t.arg)));
		I2S_info("data socket released\n");
		tx_data_t.arg = NULL;
	}
}

static void i2s_stop_rx(void)
{
	unsigned long status;

	if (rx_data_t.thread != NULL) {
		stop_i2s_thread(&rx_data_t);
		I2S_info("i2s rx data thread stopped\n");
	}

	if (rx_data_socket != NULL) {
		status = rx_data_socket->ops->shutdown(rx_data_socket, 2);
		I2S_info("data socket shutdown (%lu)\n", status);
	}

	if (rx_data_socket != NULL) {
		sockfd_put(rx_data_socket);
		I2S_info("data socket released\n");
		rx_data_socket = NULL;
	}
}

#ifdef CONFIG_ARCH_AST1500_CLIENT
static void i2s_stop_client(void)
{
	if (!is_client)
		return;

#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
	if (clock_adj_t.thread != NULL) {
		stop_i2s_thread(&clock_adj_t);
		I2S_info("i2s clock adj thread stopped\n");
	}
#endif
	i2s_stop_tx();
	i2s_stop_rx();
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

#ifdef CONFIG_ARCH_AST1500_HOST
static void i2s_stop_host(void)
{
	if (is_client)
		return;

	i2s_stop_tx();
	i2s_stop_rx();
}
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

static ssize_t memdump(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long md_size;
	unsigned long md_start;

	sscanf(buf, "%08lx %08lx", &md_start, &md_size);

	udump((void *)md_start, md_size);

	return count;
}
static DEVICE_ATTR(mem_dump, (S_IRUGO | S_IWUSR), NULL, memdump);

static ssize_t show_unlink(struct device *dev, struct device_attribute *attr, char *buf)
{
	return 0;
}

static ssize_t store_unlink(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{

	return count;
}
static DEVICE_ATTR(unlink, (S_IRUGO | S_IWUSR), show_unlink, store_unlink);

#if (BOARD_DESIGN_VER_I2S >= 204)
static ssize_t show_io_select(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (IO_Select == IO_SELECT_AUTO)
		sprintf(buf, "auto\n");
	else if (IO_Select == IO_SELECT_HDMI)
		sprintf(buf, "hdmi\n");
	else if (IO_Select == IO_SELECT_CODEC)
		sprintf(buf, "analog\n");
	return strlen(buf);
}
#if defined(CONFIG_ARCH_AST1500_HOST)
static ssize_t store_io_select(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	if (!strncmp(buf, "auto", 4)) {
		if (IO_Select != IO_SELECT_AUTO) {
			printk("set io_select to \"auto\"\n");
			IO_Select = IO_SELECT_AUTO;
			if (codec_in_status == CODEC_IN_STATUS_PLUGGED) {
				printk("use CODEC\n");
				if (IO_in_Use != IO_IN_USE_CODEC) {
					IO_in_Use = IO_IN_USE_CODEC;
					ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, NULL);
					add_event(EVENT_CODEC_SETTINGS_CHANGED, NULL, NULL);
				}
			} else if (codec_in_status == CODEC_IN_STATUS_UNPLUGGED) {
				printk("use HDMI\n");
				if (IO_in_Use != IO_IN_USE_HDMI) {
					if (IO_in_Use == IO_IN_USE_CODEC)
						PowerDownCodec();
					IO_in_Use = IO_IN_USE_HDMI;
					ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_H, NULL);
					if (hdmi_settings_valid)
						add_event(EVENT_HDMI_SETTINGS_CHANGED, NULL, NULL);
				}
			}
		}
	} else if (!strncmp(buf, "hdmi", 4)) {
		if ((HDMI_available) && (IO_Select != IO_SELECT_HDMI)) {
			IO_Select = IO_SELECT_HDMI;
			printk("set io_select to \"hdmi\"\n");
			if (IO_in_Use != IO_IN_USE_HDMI) {
				if (IO_in_Use == IO_IN_USE_CODEC)
					PowerDownCodec();
				IO_in_Use = IO_IN_USE_HDMI;
				ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_H, NULL);
				if (hdmi_settings_valid)
					add_event(EVENT_HDMI_SETTINGS_CHANGED, NULL, NULL);
			}
		}
	} else if (!strncmp(buf, "analog", 6)) {
		if ((Codec_available) && (IO_Select != IO_SELECT_CODEC)) {
			printk("set io_select to \"analog\"\n");
			IO_Select = IO_SELECT_CODEC;
			if (IO_in_Use != IO_IN_USE_CODEC) {
				IO_in_Use = IO_IN_USE_CODEC;
				ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, NULL);
				add_event(EVENT_CODEC_SETTINGS_CHANGED, NULL, NULL);
			}
		}
	} else if(!strncmp(buf, "out_analog", 10)){
		SetupCodec(1, 1, mic_input_pin, CODEC_OUTPUT_HP);
		analog_in_volume_cfg(analog_in_vol); analog_out_volume_cfg(analog_out_vol);
	}else if(!strncmp(buf, "mute_out_analog", 15)){
		SetupCodec(0, 0, mic_input_pin, CODEC_OUTPUT_HP);
	}else {
		printk("usage: auto | hdmi | analog | out_analog\n");
	}
	return strlen(buf);
}
static DEVICE_ATTR(io_select, (S_IRUGO | S_IWUSR), show_io_select, store_io_select);

static ssize_t show_analog_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	if (codec_in_status == CODEC_IN_STATUS_UNKNOWN)
		sprintf(buf, "analog IN status unknown\n");
	else if (codec_in_status == CODEC_IN_STATUS_PLUGGED)
		sprintf(buf, "analog IN plugged\n");
	else if (codec_in_status == CODEC_IN_STATUS_UNPLUGGED)
		sprintf(buf, "analog IN unplugged\n");
	return strlen(buf);
}
static DEVICE_ATTR(analog_status, S_IRUGO, show_analog_status, NULL);
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static ssize_t store_io_select(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 io_select_changed = 0;
	if (!strncmp(buf, "auto", 4)) {
		if (IO_Select != IO_SELECT_AUTO) {
			printk("set io_select to \"auto\"\n");
			IO_Select = IO_SELECT_AUTO;
			io_select_changed = 1;
		}
	} else if (!strncmp(buf, "hdmi", 4)) {
		if (IO_Select != IO_SELECT_HDMI) {
			IO_Select = IO_SELECT_HDMI;
			printk("set io_select to \"hdmi\"\n");
			io_select_changed = 1;
		}
	} else if (!strncmp(buf, "analog", 6)) {
		if (IO_Select != IO_SELECT_CODEC) {
			printk("set io_select to \"analog\"\n");
			IO_Select = IO_SELECT_CODEC;
			io_select_changed = 1;
		}
	} else {
		printk("usage: auto | hdmi | analog\n");
	}

	if (io_select_changed) {
		tx_settings.io_in_use = IO_IN_USE_NONE;
		client_handle_audio_chg(&i2s_tx_hdr_buf.a.info);
	}

	return strlen(buf);
}
static DEVICE_ATTR(io_select, (S_IRUGO | S_IWUSR), show_io_select, store_io_select);
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
#endif /* #if (BOARD_DESIGN_VER_I2S >= 204) */

#if (CONFIG_AST1500_SOC_VER >= 2)
#if NEW_DOWN_SAMPLE
static ssize_t show_down_sampling(struct device *dev, struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_ARCH_AST1500_HOST)
	if (rx_settings.DownSampling)
#else
	if (tx_settings.DownSampling)
#endif
		printk("Downsampling is enabled.\n");
	else
		printk("Downsampling is disable.\n");

	return 0;
}
#if defined(CONFIG_ARCH_AST1500_HOST)
static ssize_t store_down_sampling(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int change = 0;

	if (!strncmp(buf, "on", 2)) {
		if (rx_settings.DownSampling != 1) {
			printk("Enable downsampling.\n");
			rx_settings.DownSampling = 1;
			change = 1;
		} else
			printk("Downsampling is already enabled.\n");
	} else if (!strncmp(buf, "off", 3)) {
		if (rx_settings.DownSampling != 0) {
			printk("Disable downsampling.\n");
			rx_settings.DownSampling = 0;
			change = 1;
		} else
			printk("Downsampling is already disabled.\n");
	} else {
		printk("usage: on | off\n");
	}

	if (change)
		add_event(EVENT_AUDIO_SETTINGS_CHANGED, NULL, NULL);

	return strlen(buf);
}
static DEVICE_ATTR(down_sampling, (S_IRUGO | S_IWUSR), show_down_sampling, store_down_sampling);
#else /* #if defined(CONFIG_ARCH_AST1500_HOST) */
static DEVICE_ATTR(down_sampling, S_IRUGO, show_down_sampling, NULL);
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */
#endif /* #if NEW_DOWN_SAMPLE */

#if NEW_24_TO_16
static ssize_t show_16bits_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_ARCH_AST1500_HOST)
	if (rx_settings.Mode16bits)
#else
	if (tx_settings.Mode16bits)
#endif
		printk("16bits-mode is enabled.\n");
	else
		printk("16bits-mode is disable.\n");

	return 0;
}
#if defined(CONFIG_ARCH_AST1500_HOST)
static ssize_t store_16bits_mode(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int change = 0;
	if (!strncmp(buf, "on", 2)){
		if (rx_settings.Mode16bits != 1) {
			printk("Enable 16bits-mode.\n");
			rx_settings.Mode16bits = 1;
			change = 1;
		} else
			printk("16bits-mode is already enabled.\n");
	} else if (!strncmp(buf, "off", 3)) {
		if (rx_settings.Mode16bits != 0) {
			printk("Disable 16bits-mode.\n");
			rx_settings.Mode16bits = 0;
			change = 1;
		} else
			printk("16bits-mode is already disabled.\n");
	} else {
		printk("usage: on | off\n");
	}

	if (change)
		add_event(EVENT_AUDIO_SETTINGS_CHANGED, NULL, NULL);

	return strlen(buf);
}
static DEVICE_ATTR(16bits_mode, (S_IRUGO | S_IWUSR), show_16bits_mode, store_16bits_mode);
#else /* #if defined(CONFIG_ARCH_AST1500_HOST) */
static DEVICE_ATTR(16bits_mode, S_IRUGO, show_16bits_mode, NULL);
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */
#endif /* #if NEW_24_TO_16 */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static ssize_t show_src_stable_timeout(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%lu\n", the_src_stable_timeout);
	return strlen(buf);
}

static ssize_t store_src_stable_timeout(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long ms;
	sscanf(buf, "%lu", &ms);

	the_src_stable_timeout = ms;

	return strlen(buf);
}
static DEVICE_ATTR(src_stable_timeout, (S_IRUGO | S_IWUSR), show_src_stable_timeout, store_src_stable_timeout);


static ssize_t show_drop_period(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%lu\n", the_drop_period);
	return strlen(buf);
}

static ssize_t store_drop_period(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long ms;
	sscanf(buf, "%lu", &ms);

	the_drop_period = ms;

	return strlen(buf);
}
static DEVICE_ATTR(drop_period, (S_IRUGO | S_IWUSR), show_drop_period, store_drop_period);


#if (CONFIG_AST1500_SOC_VER >= 2)
#if defined(I2S_CLOCK_ADJUST)
#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
static ssize_t show_clock_adj_interval(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%i\n", clock_adj_interval);
	return strlen(buf);
}
static ssize_t store_clock_adj_interval(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	sscanf(buf, "%i", &clock_adj_interval);
	printk("set I2S clock adj. interval to %ims\n", clock_adj_interval);
	return strlen(buf);
}
static DEVICE_ATTR(clock_adj_interval, (S_IRUGO | S_IWUSR), show_clock_adj_interval, store_clock_adj_interval);
#endif /* #if defined(I2S_CLOCK_ADJUST_IN_THREAD) */

static ssize_t show_clock_adj_test_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%i\n", clock_adj_test_mode());
	return strlen(buf);
}

static ssize_t store_clock_adj_test_mode(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);

	if (1 <= c)
		clock_adj_test_mode_cfg(cfg);

	return strlen(buf);
}
static DEVICE_ATTR(clock_adj_test_mode, (S_IRUGO | S_IWUSR), show_clock_adj_test_mode, store_clock_adj_test_mode);

static ssize_t show_clock_lock(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "0x%.8x\n", clock_lock_mode());
	num += snprintf(buf + num, PAGE_SIZE - num, "stage %d (0:L, 1:N, 2:H)\n", i2s_stage());
	num += snprintf(buf + num, PAGE_SIZE - num, "ppm: %d (%d ~ %d ~ %d)\n", clock_gen_lock(), clock_ppm() - clock_limit(), clock_ppm(), clock_ppm() + clock_limit());

	return num;
}

static ssize_t store_clock_lock(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 cfg;

	if (strlen(buf) >= 1) {
		cfg = simple_strtoul(buf, NULL, 0);

		/*
		** Bruce121112. This is a special patch used for patch wrong MIC input pin.
		** Note that the patch assems that HW bias is correcct, only MIC1 change to MIC2.
		*/
		if (cfg & CLK_LOCK_MODE_MIC_PATCH) {
			mic_input_pin = CODEC_INPUT_MIC2;
			cfg &= ~CLK_LOCK_MODE_MIC_PATCH;
		} else {
			mic_input_pin = CODEC_INPUT_MIC1;
		}

		clock_lock_mode_cfg(cfg);
	}

	return strlen(buf);
}
static DEVICE_ATTR(clock_lock, (S_IRUGO | S_IWUSR), show_clock_lock, store_clock_lock);
#endif /* #if defined(I2S_CLOCK_ADJUST) */

static ssize_t show_reset_dead_engine(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%i\n", reset_dead_audio_engine);
	return strlen(buf);
}

static ssize_t store_reset_dead_engine(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%i", &reset_dead_audio_engine);
	return strlen(buf);
}
static DEVICE_ATTR(reset_dead_engine, (S_IRUGO | S_IWUSR), show_reset_dead_engine, store_reset_dead_engine);
#endif

static ssize_t show_test_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "echo sample_freq ch_num > test_mode\n");
	sprintf(buf + strlen(buf), "sample_freq:\n\t3:32KHz\n\t0:44.1KHz\n\t2:48KHz\n\t8:88.2KHz\n\t10:96KHz\n\t12:176.4KHz\n\t14:192KHz\n");
	sprintf(buf + strlen(buf), "ch_num:\n\t2,4,6,8\n");
	return strlen(buf);
}

static ssize_t store_test_mode(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	Audio_Info Audio_info;
	unsigned int f = I2S_SAMPLE_RATE_44d1K, ch = 8;

	sscanf(buf, "%d %d", &f, &ch);

	Audio_info.Audio_On = 1;
	Audio_info.SampleFrequency = f;
	Audio_info.ValidCh = ch;
	Audio_info.SampleSize = 24;
	Audio_info.io_in_use = IO_IN_USE_HDMI;
	Audio_info.enable_upstream = 0;
	Audio_info.DownSampling = 0;
	Audio_info.Mode16bits = 0;
	Audio_info.Audio_Type = AST_AUDIO_ON;
	Audio_info.bSPDIF = 0;

	client_handle_audio_chg(&Audio_info);

	return strlen(buf);
}
static DEVICE_ATTR(test_mode, (S_IRUGO | S_IWUSR), show_test_mode, store_test_mode);

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
static void sys_i2s_dual_output_cfg(u32 en)
{
	i2s_dual_output_cfg(en, IO_in_Use);

	if (en) {
		if (IO_IN_USE_HDMI == IO_in_Use) {
			if (Codec_available) {
				SetupCodec(1, 1, mic_input_pin, CODEC_OUTPUT_HP);
				analog_in_volume_cfg(analog_in_vol); analog_out_volume_cfg(analog_out_vol);
			}
		}
	} else {
		/*
		 * the reason for changing tx_settings.io_in_use:
		 * client_handle_audio_chg() just return if tx_settings and i2s_tx_hdr_buf.a.info are identical
		 * So change tx_settings.io_in_use to IO_IN_USE_NONE to let client_handle_audio_chg() work
		 */
		tx_settings.io_in_use = IO_IN_USE_NONE;
		client_handle_audio_chg(&i2s_tx_hdr_buf.a.info);
	}
}

static ssize_t show_dual_output(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	num += snprintf(buf + num, PAGE_SIZE - num,
		"I2S dual output: %d\n", i2s_dual_output());
	num += snprintf(buf + num, PAGE_SIZE - num,
		"\n");
	num += snprintf(buf + num, PAGE_SIZE - num,
		"Configure I2S00[3:0] (0x1E6E5000) to enable data channel transmition\n");
	return num;
}

static ssize_t store_dual_output(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 en, c;

	if (3 <= ast_scu.ability.soc_op_mode) {
		c = sscanf(buf, "%d", &en);

		if (1 <= c) {
			if (en)
				i2s_dual_output_en = 1;
			else
				i2s_dual_output_en = 0;

			sys_i2s_dual_output_cfg(i2s_dual_output_en);
		}
	}

	return count;
}
static DEVICE_ATTR(dual_output, (S_IRUGO | S_IWUSR), show_dual_output, store_dual_output);
#endif /* #if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT) */

#if defined(I2S_DATA_DELAY)
static u32 _accumulate_buffer_time_us = 0;
static u32 accumulate_buffer_time_us(Audio_Info *audio_info, u32 buf_size)
{
	u32 sample_data_size; /* in byte */
	u32 sample_num_per_buf;

	u32 rate_idx = audio_info->SampleFrequency;
	u32 channel = audio_info->ValidCh;
	unsigned int rate = 0;
	unsigned int i;

	for (i = 0; i < sample_rate_num; i++) {
		if (sample_rate_table[i].sample_rate_index == rate_idx) {
			rate = sample_rate_table[i].sample_rate;
			break;
		}
	}

	if (rate == 0) {
		I2S_err("unsupported sample rate idx = %d\n", rate_idx);
		BUG();
	}

	if (1 == ast_scu.ability.soc_op_mode)
		channel = 2; /* AST1500 force I2S channel to 2 */

	/*
	 * sample data size = channel number * sample size (32 bit)
	 * the channel number must be even because one data pin includes L + R
	 */
	sample_data_size = ((channel + 1) & 0xFFFFFFFE) * (SRC_WS_SIZE >> 3);
	sample_num_per_buf = buf_size / sample_data_size;

	/* To accumulate a data buffer will spend sample_num_per_buf/sample frequency */
	_accumulate_buffer_time_us = (sample_num_per_buf * 1000) * 10 / (rate / 100);

	return _accumulate_buffer_time_us;
}

static u32 _tx_delay_ms = 0;

void tx_delay_ms_cfg(u32 cfg)
{
	_tx_delay_ms = cfg;
}

u32 tx_delay_ms(void)
{
	return _tx_delay_ms;
}

static void update_tx_delay_queue(Audio_Info *audio_info, u32 buf_size, unsigned int reserved_buf_num)
{
	u32 len;

	accumulate_buffer_time_us(audio_info, buf_size);

	len = tx_delay_ms() * 1000 / _accumulate_buffer_time_us;

	I2S_info("Update Tx Delay Queue: buf_size=%d, q_len=%d, us_per_buf=%d, tx_buf_num=%d\n", buf_size, len, _accumulate_buffer_time_us, Tx_Buffer.buf_num);

	tx_queue_len_cfg(len, Tx_Buffer.buf_num - reserved_buf_num, I2S_TX_DELAY_TIME_MIN_MS * 1000 / _accumulate_buffer_time_us);
}

static ssize_t show_tx_queue_len(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", tx_queue_len());
	return num;
}

static ssize_t store_tx_queue_len(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
		tx_queue_len_cfg(cfg, Tx_Buffer.buf_num, 0);

	return count;
}
static DEVICE_ATTR(tx_queue_len, (S_IRUGO | S_IWUSR), show_tx_queue_len, store_tx_queue_len);

static ssize_t show_tx_delay_ms(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", tx_delay_ms());
	return num;
}

static ssize_t store_tx_delay_ms(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 cfg, c;

	c = sscanf(buf, "%d", &cfg);

	if (1 <= c) {
		tx_delay_ms_cfg(cfg);
		/* FIXME */
		current_sender = 0;
	}

	return count;
}
static DEVICE_ATTR(tx_delay_ms, (S_IRUGO | S_IWUSR), show_tx_delay_ms, store_tx_delay_ms);
#endif /* #if defined(I2S_DATA_DELAY) */

#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */

#if HDCP2_Encryption

#ifdef CONFIG_ARCH_AST1500_HOST
static ssize_t show_enable_aes_encryption(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%u\n", enable_aes_encryption);
	return strlen(buf);
}
static ssize_t store_enable_aes_encryption(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%u", &enable_aes_encryption);
	return strlen(buf);
}
static DEVICE_ATTR(enable_aes_encryption, (S_IRUGO | S_IWUSR), show_enable_aes_encryption, store_enable_aes_encryption);
#endif

static ssize_t show_debug_aes_encryption(struct device *dev, struct device_attribute *attr, char *buf)
{
	sprintf(buf, "%u\n", debug_aes_encryption);
	return strlen(buf);
}
static ssize_t store_debug_aes_encryption(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%u", &debug_aes_encryption);
	return strlen(buf);
}
static DEVICE_ATTR(debug_aes_encryption, (S_IRUGO | S_IWUSR), show_debug_aes_encryption, store_debug_aes_encryption);

#endif /* #if HDCP2_Encryption */

static ssize_t show_buffer_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

#if defined(CONFIG_ARCH_AST1500_HOST)
	num += snprintf(buf + num, PAGE_SIZE - num, "Rx Buffer num:  %d\n", Rx_Buffer.buf_num);
#else /* #if defined(CONFIG_ARCH_AST1500_HOST) */
	num += snprintf(buf + num, PAGE_SIZE - num, "Tx Buffer num:  %d\n", Tx_Buffer.buf_num);
#if (CONFIG_AST1500_SOC_VER >= 2)
	num += snprintf(buf + num, PAGE_SIZE - num, "Tx Buffer used: %d\n", tx_buffer_filled());
#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3)
		num += snprintf(buf + num, PAGE_SIZE - num, "benchmark 0x%.8x, avg: 0x%.8x, last_num %d\n", i2s_gen_lock_benchmark(), i2s_gen_lock_avg(), i2s_gen_lock_last_num());
#endif /* #if defined(I2S_CLOCK_ADJUST_NEW) */
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */
	return num;
}
static DEVICE_ATTR(buffer_status, S_IRUGO, show_buffer_status, NULL);

static ssize_t show_analog_in_vol(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	if (!Codec_available)
		goto sysfs_end;

	if (analog_in_vol == -1)
		num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", analog_in_volume());
	else
		num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", analog_in_vol);

sysfs_end:
	return num;
}

static ssize_t store_analog_in_vol(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg, c;

	if (!Codec_available)
		goto sysfs_end;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1) {

		if ((cfg > 100) || (cfg < 0))
			cfg = -1;

		if (cfg != analog_in_vol) {
			analog_in_vol = cfg;
			analog_in_volume_cfg(analog_in_vol);
		}
	}

sysfs_end:
	return count;
}
static DEVICE_ATTR(analog_in_vol, (S_IRUGO | S_IWUSR), show_analog_in_vol, store_analog_in_vol);

static ssize_t show_analog_out_vol(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	if (!Codec_available)
		goto sysfs_end;

	if (analog_out_vol == -1)
		num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", analog_out_volume());
	else
		num += snprintf(buf + num, PAGE_SIZE - num, "%d\n", analog_out_vol);

sysfs_end:
	return num;
}

static ssize_t store_analog_out_vol(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int cfg, c;

	if (!Codec_available)
		goto sysfs_end;

	c = sscanf(buf, "%d", &cfg);
	if (c >= 1) {
		if ((cfg > 100) || (cfg < 0))
			cfg = -1;

		if (cfg != analog_out_vol) {
			analog_out_vol =  cfg;
			analog_out_volume_cfg(analog_out_vol);
		}
	}

sysfs_end:
	return count;
}
static DEVICE_ATTR(analog_out_vol, (S_IRUGO | S_IWUSR), show_analog_out_vol, store_analog_out_vol);

static ssize_t show_codec_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, num = 0;
	u32 start, end;

	if (codec_reg_rd == NULL)
		goto show_codec_reg_exit;

	start = 0x00;
	end = 0xFB;

	num += snprintf(buf + num, PAGE_SIZE - num, "\n       ");

	for (i = 0; i <= 0x0F; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.4x", i);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
		num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.2x: ", i);

		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.4x", codec_reg_rd(i));
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

show_codec_reg_exit:
	return num;
}

static ssize_t store_codec_reg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int c, offset, val;

	if (codec_reg_wr == NULL)
		goto store_codec_reg_exit;

	c = sscanf(buf, "%x %x", &offset, &val);

	if (c >= 2)
		codec_reg_wr(offset, val);

store_codec_reg_exit:
	return count;
}
static DEVICE_ATTR(codec_reg, (S_IRUGO | S_IWUSR), show_codec_reg, store_codec_reg);

static ssize_t _show_audio_info(char *buf, Audio_Info *a_info)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "State: %s\n", a_info->Audio_On ? "On" : "Off");
	if (!a_info->Audio_On)
		goto out;

	num += snprintf(buf + num, PAGE_SIZE - num, "Source: %s\n",
		(a_info->io_in_use == IO_IN_USE_CODEC) ? ("Analog Codec") :
		(a_info->io_in_use == IO_IN_USE_HDMI) ? ("HDMI") : ("Auto"));

	num += snprintf(buf + num, PAGE_SIZE - num, "Type: %s (0x%X)\n",
		((a_info->Audio_Type & (AST_AUDIO_HBR)) == (AST_AUDIO_HBR)) ? "HBR Audio [7.1 Ch]" :
		(a_info->Audio_Type & AST_AUDIO_NLPCM) ? "Non-LPCM [5.1 Ch]" :
		(a_info->Audio_Type & AST_AUDIO_DSD) ? "DSD[Not Supported]" : "LPCM",
		a_info->Audio_Type);

	/*
	#define B_FS_44100		  (0)  44.1kHz
	#define B_FS_NOTID		  (1)  non indicated
	#define B_FS_48000		  (2)  48kHz
	#define B_FS_32000		  (3)  32kHz
	#define B_FS_22050		  (4)  22.05kHz
	#define B_FS_24000		  (6)  24kHz
	#define B_FS_88200		  (8)  88.2kHz
	#define B_FS_768000 	  (9)  768kHz, HBR
	#define B_FS_96000		  (10) 96kHz
	#define B_FS_176400 	  (12) 176.4kHz
	#define B_FS_192000 	  (14) 192kHz
	*/
	switch (a_info->SampleFrequency) {
		case 0:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "44.1 KHz");
			break;
		case 2:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "48 KHz");
			break;
		case 3:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "32 KHz");
			break;
		case 4:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "22.05 KHz");
			break;
		case 6:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "24 KHz");
			break;
		case 8:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "88.2 KHz");
			break;
		case 9:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "768 KHz (HBR)");
			break;
		case 10:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "96 KHz");
			break;
		case 12:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "176.4 KHz");
			break;
		case 14:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "192 KHz");
			break;
		default:
			num += snprintf(buf + num, PAGE_SIZE - num, "Sample Freq: %s\n", "Unknown");
			break;
	}
	num += snprintf(buf + num, PAGE_SIZE - num, "Sample Size: %d bits\n", a_info->SampleSize);

	if (!(a_info->Audio_Type & AST_AUDIO_ENCODE_MASK)) {
		num += snprintf(buf + num, PAGE_SIZE - num, "Valid Ch: %d\n", a_info->ValidCh);
	} else {
		num += snprintf(buf + num, PAGE_SIZE - num, "Valid Ch: %d (Encoded)\n", a_info->ValidCh);
	}

	/*num += snprintf(buf + num, PAGE_SIZE - num, "Upstream: %s\n", a_info->enable_upstream ? "Enabled" : "Disabled");*/
	/*num += snprintf(buf + num, PAGE_SIZE - num, "SPDIF: %s\n", a_info->bSPDIF ? "On" : "Off");*/

out:
	return num;
}

static ssize_t show_output_audio_info(struct device *dev, struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	return _show_audio_info(buf, &tx_settings);
#else
	int num = 0;

	if ((rx_settings.io_in_use == IO_IN_USE_CODEC) && (rx_settings.enable_upstream)) {
		return _show_audio_info(buf, &rx_settings);
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "State: Off\n");
	return num;
#endif
}
static DEVICE_ATTR(output_audio_info, (S_IRUGO), show_output_audio_info, NULL);

static ssize_t show_input_audio_info(struct device *dev, struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	int num = 0;

	if ((tx_settings.enable_upstream) && (upstream_available))
		return _show_audio_info(buf, &tx_settings);
	
	num += snprintf(buf + num, PAGE_SIZE - num, "State: Off\n");
	return num;
#else
	return _show_audio_info(buf, &rx_settings);
#endif
}
static DEVICE_ATTR(input_audio_info, (S_IRUGO), show_input_audio_info, NULL);


#if defined(AST_I2S_RX_DEBUG)
static u32 rx_buf_log_cnt = 0;
static ssize_t show_rx_buf(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	return num;
}

static ssize_t store_rx_buf(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 log_cnt;

	sscanf(buf, "%u", &log_cnt);
	rx_buf_log_cnt = log_cnt;

	return count;
}
static DEVICE_ATTR(rx_buf, (S_IRUGO | S_IWUSR), show_rx_buf, store_rx_buf);
#endif

#if defined(I2S_CLOCK_DETECT)
static ssize_t show_detect(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	return num;
}

static ssize_t store_detect(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	i2s_detect_cnt = 0;

	return count;
}
static DEVICE_ATTR(detect, (S_IRUGO | S_IWUSR), show_detect, store_detect);
#endif

#ifdef CONFIG_ARCH_AST1500_CLIENT
static ssize_t show_gen_lock(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	return num;
}

static ssize_t store_gen_lock(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int c, ppm;

	c = sscanf(buf, "%d", &ppm);

	if (c >= 1)
		clock_gen_lock_cfg(ppm);

	return count;
}
static DEVICE_ATTR(gen_lock, (S_IRUGO | S_IWUSR), show_gen_lock, store_gen_lock);
#endif
static int stop_flag = 0;

static ssize_t show_stop(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	if(stop_flag == 1)
	{
		num += snprintf(buf + num, PAGE_SIZE - num, "1");
	}
	else
	{
		num += snprintf(buf + num, PAGE_SIZE - num, "0");
	}

	return num;
}

static ssize_t store_stop(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int c;
	c = sscanf(buf, "%d", &stop_flag);
	if(stop_flag == 1)
	{
	down(&i2s_user_lock);
#ifdef CONFIG_ARCH_AST1500_CLIENT
	i2s_stop_client();
#endif
#ifdef CONFIG_ARCH_AST1500_HOST
	i2s_stop_host();
#endif
	up(&i2s_user_lock);
	//A7 removed
	//ast_notify_user("e_i2s_stopped");
	}


	return count;
}
static DEVICE_ATTR(stop, (S_IRUGO | S_IWUSR), show_stop, store_stop);

static struct attribute *dev_attrs[] = {
	&dev_attr_unlink.attr,
#if (BOARD_DESIGN_VER_I2S >= 204)
	&dev_attr_io_select.attr,
#if defined(CONFIG_ARCH_AST1500_HOST)
	&dev_attr_analog_status.attr,
#endif
#endif /* #if (BOARD_DESIGN_VER_I2S >= 204) */
#if (CONFIG_AST1500_SOC_VER >= 2)
#if NEW_DOWN_SAMPLE
	&dev_attr_down_sampling.attr,
#endif
#if NEW_24_TO_16
	&dev_attr_16bits_mode.attr,
#endif
#endif
	&dev_attr_mem_dump.attr,

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	&dev_attr_src_stable_timeout.attr,
	&dev_attr_drop_period.attr,
	&dev_attr_test_mode.attr,
#if (CONFIG_AST1500_SOC_VER >= 2)
#if defined(I2S_CLOCK_ADJUST)
#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
	&dev_attr_clock_adj_interval.attr,
#endif
	&dev_attr_clock_adj_test_mode.attr,
	&dev_attr_clock_lock.attr,
#endif
#if defined(I2S_DATA_DELAY)
	&dev_attr_tx_queue_len.attr,
	&dev_attr_tx_delay_ms.attr,
#endif
	&dev_attr_reset_dead_engine.attr,
#endif
#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
	&dev_attr_dual_output.attr,
#endif
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
#if HDCP2_Encryption
#if defined(CONFIG_ARCH_AST1500_HOST)
	&dev_attr_enable_aes_encryption.attr,
#endif
	&dev_attr_debug_aes_encryption.attr,
#endif
	&dev_attr_buffer_status.attr,
	&dev_attr_analog_in_vol.attr,
	&dev_attr_analog_out_vol.attr,
	&dev_attr_codec_reg.attr,
	&dev_attr_output_audio_info.attr,
	&dev_attr_input_audio_info.attr,
#if defined(AST_I2S_RX_DEBUG)
	&dev_attr_rx_buf.attr,
#endif
#if defined(I2S_CLOCK_DETECT)
	&dev_attr_detect.attr,
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	&dev_attr_gen_lock.attr,
#endif
	&dev_attr_stop.attr,
	NULL,
};

struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static void select_buf_size(Audio_Info *audio_info, unsigned int *size, unsigned int *size_shift)
{
	unsigned int rate_idx = audio_info->SampleFrequency;
	unsigned int channel = audio_info->ValidCh;
	unsigned int rate = 0;
	unsigned int i;

	I2S_info("select_buf_size(%d, %d)\n", rate_idx, channel);
	for (i = 0; i < sample_rate_num; i++) {
		if (sample_rate_table[i].sample_rate_index == rate_idx) {
			rate = sample_rate_table[i].sample_rate;
			I2S_info("sample rate = %d\n", rate);
			break;
		}
	}

	if (rate == 0) {
		I2S_err("unsupported sample rate idx = %d\n", rate_idx);
		BUG();
	}

	if (1 == ast_scu.ability.soc_op_mode) {
		/* This is the case for SoC V1 */
		channel = 2; /* AST1500 force I2S channel to 2 */
		switch (rate) {
		case 44100:	//44.1KHz
			*size_shift = 11;//12;
			break;
		case 48000:	//48KHz
			*size_shift = 11;//12;
			break;
		case 32000:	//32KHz
			*size_shift = 10;//11;
			break;
		case 88200:	//88.2KHz
			*size_shift = 12;//13;
			break;
		case 96000:	//96KHz
			*size_shift = 12;//13;
			break;
		case 176400:	//176.4KHz
			*size_shift = 13;
			break;
		case 192000:	//192KHz
			*size_shift = 13;
			break;
		default:
			I2S_info("invalid sample frequency?! (%d)\n", rate);
		};
	} else {
		/* This is the case for SoC V2 and V3 later */
		//(*size < MAX_RX_SIZE)
		//trade-off: descriptor # impact freq adj
		switch (rate) {
		case 44100:	//44.1KHz
			*size_shift = 9;
			break;
		case 48000:	//48KHz
			*size_shift = 9;
			break;
		case 32000:	//32KHz
			*size_shift = 8;
			break;
		case 88200:	//88.2KHz
			*size_shift = 10;
			break;
		case 96000:	//96KHz
			*size_shift = 10;
			break;
		case 176400:	//176.4KHz
			*size_shift = 11;
			break;
		case 192000:	//192KHz
			*size_shift = 11;
			break;
		default:
			I2S_info("invalid sample frequency?! (%d)\n", rate);
		};
	}

#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3) {
		u32 size_pre_sample_in_shift, sample_per_buffer;

		if (SRC_WS_SIZE == 16)
			size_pre_sample_in_shift = 2;
		else
			size_pre_sample_in_shift = 3;

		sample_per_buffer = 0x1 << (*size_shift - size_pre_sample_in_shift);

		i2s_gen_lock_param_init(sample_per_buffer, rate);
	}
#endif

	/* channel should be # of I2S channel */
	switch (channel) {
	case 1:
	case 2:
		break;
	case 3:
	case 4:
		*size_shift += 1;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
		*size_shift += 2;
		break;
	default:
		I2S_info("invalid channel?! (%d)\n", channel);
	};

#if (CONFIG_AST1500_SOC_VER >= 2)
	if (audio_info->DownSampling)
		*size_shift -= 1;

	if (audio_info->Mode16bits)
		*size_shift -= 1;
#endif
	*size = (1 << (*size_shift));

	max_udp_xfer_size = *size << 2;

	I2S_info("selected buffer size = %d\n", *size);
	BUG_ON(max_udp_xfer_size>=65536); //Actually, the size should also include i2s header.
}

static u32 _rx_desc_num(void)
{
#if (CONFIG_AST1500_SOC_VER >= 3)
	if (ast_scu.ability.soc_op_mode == 1)
		return NUM_RX_DESCRIPTOR_OP1;
#endif
	return NUM_RX_DESCRIPTOR;
}

static u32 _tx_desc_num(void)
{
#if (CONFIG_AST1500_SOC_VER >= 3)
	if (ast_scu.ability.soc_op_mode == 1)
		return NUM_TX_DESCRIPTOR_OP1;
#endif
	return NUM_TX_DESCRIPTOR;
}

//This routine might sleep.
static int i2s_rx_buff_alloc(unsigned int size)
{
	I2S_info("Rx buffer size = %d\n", size);

	if (!rx_buf_pool) {
		dma_addr_t handle;
		unsigned int i, desc_num;

		desc_num = _rx_desc_num();
		rx_buf_pool = dma_pool_create(rx_buf_pool_name, NULL, size, 8, 0);
		if (!rx_buf_pool) {
			I2S_err("dma_pool_create failed (%d)\n", size);
			BUG();
			return -1;
		}

		for (i = 0; i < desc_num; i++) {
			rx_buf_va[i] = dma_pool_alloc(rx_buf_pool, GFP_KERNEL, &handle);
			if (rx_buf_va[i] == NULL)
				break;
			rx_buf_pa[i] = handle;
		}

		if (i < (desc_num >> 1)) {
			I2S_err("too few Rx buffers (%d/%d)\n", i, desc_num);
			BUG();
			return -1;
		}
		Rx_Buffer.buf_num = i;
		Rx_Buffer.buf_va = rx_buf_va;
		Rx_Buffer.buf_pa = rx_buf_pa;
	}
	I2S_info("# of Rx buffers = %d\n", Rx_Buffer.buf_num);
	return 0;
}

static void i2s_rx_buff_free(void)
{
	unsigned int i;

	if (rx_buf_pool) {
		for (i = 0; i < Rx_Buffer.buf_num; i++)
			 dma_pool_free(rx_buf_pool, rx_buf_va[i], rx_buf_pa[i]);

		dma_pool_destroy(rx_buf_pool);
		rx_buf_pool= NULL;
	}
}

static int i2s_rx_desc_alloc(void)
{
	u32 desc_num = _rx_desc_num();

	if (!Rx_Buffer.desc_va) {
		Rx_Buffer.desc_va = dma_alloc_coherent(NULL, SIZE_RX_DESCRIPTOR * desc_num, (dma_addr_t *)&Rx_Buffer.desc_pa, GFP_KERNEL);
		if (Rx_Buffer.desc_va == NULL) {
			I2S_err("Can't allocate rx descriptors\n");
			return -1;
		}
	}

	memset(Rx_Buffer.desc_va, 0, SIZE_RX_DESCRIPTOR * desc_num);
	I2S_info("rx_desc = %08x\n", (u32)Rx_Buffer.desc_va);
	I2S_info("rx_desc_dma = %08x\n", Rx_Buffer.desc_pa);
	I2S_info("NUM_RX_DESCRIPTOR = %d\n", desc_num);
	return 0;
}

static void i2s_rx_desc_free(void)
{
	if (Rx_Buffer.desc_va) {
		dma_free_coherent(NULL, SIZE_RX_DESCRIPTOR * _rx_desc_num(), Rx_Buffer.desc_va, (dma_addr_t)Rx_Buffer.desc_pa);
		Rx_Buffer.desc_va = NULL;
	}
}

//This routine might sleep.
static int i2s_tx_buff_alloc(unsigned int size)
{
	I2S_info("Tx buffer size = %d\n", size);

	if (!tx_buf_pool) {
		dma_addr_t handle;
		unsigned int i, desc_num;

		desc_num = _tx_desc_num();
		tx_buf_pool = dma_pool_create(tx_buf_pool_name, NULL, size, 8, 0);
		for (i = 0; i < desc_num; i++) {
			tx_buf_va[i] = dma_pool_alloc(tx_buf_pool, GFP_KERNEL, &handle);
			if (tx_buf_va[i] == NULL)
				break;
			tx_buf_pa[i] = handle;
		}

		if (i < (desc_num >> 1)) {
			I2S_err("too few Tx buffers (%d/%d)\n", i, desc_num);
			BUG();
			return -1;
		}
		Tx_Buffer.buf_num = i;
		Tx_Buffer.buf_va = tx_buf_va;
		Tx_Buffer.buf_pa = tx_buf_pa;
	}
	I2S_info("# of Tx buffers = %d\n", Tx_Buffer.buf_num);
	return 0;
}

static void i2s_tx_buff_free(void)
{
	unsigned int i;
	if (tx_buf_pool) {
		for (i = 0; i < Tx_Buffer.buf_num; i++)
			 dma_pool_free(tx_buf_pool, tx_buf_va[i], tx_buf_pa[i]);

		dma_pool_destroy(tx_buf_pool);
		tx_buf_pool= NULL;
	}
}

static int i2s_tx_desc_alloc(void)
{
	u32 desc_num = _tx_desc_num();

	if (!Tx_Buffer.desc_va) {
		Tx_Buffer.desc_va = dma_alloc_coherent(NULL, SIZE_TX_DESCRIPTOR * desc_num, (dma_addr_t *)&Tx_Buffer.desc_pa, GFP_KERNEL);
		if (Tx_Buffer.desc_va == NULL) {
			I2S_err("Can't allocate tx descriptors\n");
			return -1;
		}
	}

	memset(Tx_Buffer.desc_va, 0, SIZE_TX_DESCRIPTOR * desc_num);
	I2S_info("tx_desc = %08x\n", (u32)Tx_Buffer.desc_va);
	I2S_info("tx_desc_dma = %08x\n", Tx_Buffer.desc_pa);
	I2S_info("NUM_TX_DESCRIPTOR = %d\n", desc_num);

	return 0;
}

static void i2s_tx_desc_free(void)
{
	if (Tx_Buffer.desc_va) {
		dma_free_coherent(NULL, SIZE_TX_DESCRIPTOR * _tx_desc_num(), Tx_Buffer.desc_va, (dma_addr_t)Tx_Buffer.desc_pa);
		Tx_Buffer.desc_va = NULL;
	}
}

#if defined(CONFIG_ARCH_AST1500_CLIENT)
#if 1 /*(PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)*/
#define I2S_DUAL_OUTPUT_EN_DEFAULT 1

static void gen_audio_info(Audio_Info *dst, Audio_Info *src)
{
	hdmi_audio_infoframe audio_infoframe;
	iec60958_channel_status_mode_0 channel_status;

	memset(&audio_infoframe, 0, sizeof(hdmi_audio_infoframe));

	audio_infoframe.info.ChannelCount = 0x1; /* 2 ch, FIXME */
	audio_infoframe.info.CodingType = 0x0; /* refer to stream header */
	audio_infoframe.info.SampleSize = 0x0; /* refer to stream header */
	audio_infoframe.info.SampleFrequency = 0x0; /* refer to stream header */
	audio_infoframe.info.ChannelAllocation = 0; /* FR FL */
	audio_infoframe.info.LFEPlaybackLevelInformation = 0;
	audio_infoframe.info.LevelShiftValue = 0;
	audio_infoframe.info.DownmixInhibit = 0;

	memcpy(dst->AUD_DB, audio_infoframe.pktbyte.AUDIO_PB + 1, 5);

	channel_status.info.Use = 0; /* consumer */
	channel_status.info.LinearPCM = 0; /* LPCM */
	channel_status.info.CopyrightInformation = 0; /* copyright is asserted */
	channel_status.info.AdditionalFormatInformation = 0; /* 2 channel without pre-emphasis FIXME */
	channel_status.info.Mode = 0; /* mode 0 */
	channel_status.info.CategoryCode = 0; /* general */
	channel_status.info.SourceNumber = 0;
	channel_status.info.ChannelNumber = 2; /* FIXME */
	channel_status.info.R0 = 0;
	channel_status.info.SamplingFrequency = 2; /* 48K FIXME */
	channel_status.info.ClockAccuracy = 0;
	channel_status.info.WordLength = 0xB; /* 24 bit FIXME */
	channel_status.info.OriginalSamplingFrequency = 0; /* not indicated */
	memcpy(dst->ucIEC60958ChStat, channel_status.pktbyte.data, 5);

	/* HDMI transmitter needs following infomations for further processing */
	dst->SampleFrequency = 2;
	dst->ValidCh = 2;
	dst->SampleSize = 24;
#if (API_VER >= 2)
	/* Bruce161212. codec hybrid to it6613 need Audio_Type to be 0x80. */
	dst->Audio_Type = AST_AUDIO_ON;
	dst->DownSampling = 0;
	dst->Mode16bits = 0;
#endif
}
#endif

static void i2s_tx_sys_init(void)
{
#if defined(I2S_CLOCK_ADJUST_NEW)
	if (ast_scu.ability.soc_op_mode >= 3) {
		int ppm_init = 0;

#if defined(I2S_CLK_INFO_FROM_REMOTE)
		ppm_init = i2s_tx_hdr_buf.sample_frequency_in_ppm;

		if (abs(ppm_init) < 32) /* precision for 32KHz is 1/32000 ~= 31.25 ppm */
			ppm_init = 0;
#endif /* #if defined(I2S_CLK_INFO_FROM_REMOTE) */
		i2s_gen_lock_start(ppm_init);
	}
#endif /* #if defined(I2S_CLOCK_ADJUST_NEW) */
#if defined(I2S_SYNC_CHECK_NET_RX)
	net_rx_check_start(1);
#endif
}

// Return 0 if no change. Return 1 if changed.
static int client_handle_audio_chg(Audio_Info *pAudio_info)
{
	unsigned int force_dual_output_support = ast_scu.astparam.a_force_dual_output;

	if (!memcmp(pAudio_info, &tx_settings, I2S_HDR_SIZE)) {
		I2S_info("Same audio setting, don't change.\n");
		return 0;
	}

	/*
	** Bruce161104. Take dual output into consideration and to avoid pop noise.
	** We always disable codec before stopping audio engine.
	*/
	if (Codec_available)
		PowerDownCodec();
	/*
	** Bruce111031. A correct procedure of for clock change:
	** 1. SCU reset and hold
	** 2. Setup I2S clock and wait for clock stable.
	** 3. Release SCU reset
	** 3. Program I2S engine
	** 4. Start I2S engine
	*/
	stop_audio_engine();
	I2S_info("tx_data idle\n");

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
	if ((force_dual_output_support) || (3 <= ast_scu.ability.soc_op_mode)) {
		switch (IO_Select) {
		case IO_SELECT_AUTO:
			break;
		case IO_SELECT_CODEC:
			pAudio_info->io_in_use = IO_IN_USE_CODEC;
			break;
		case IO_SELECT_HDMI:
			pAudio_info->io_in_use = IO_IN_USE_HDMI;
			break;
		default:
			/* inproper IO_Select, force to auto */
			IO_Select = IO_SELECT_AUTO;
		}
	}
#endif

	if (IO_in_Use != pAudio_info->io_in_use) {
		if (pAudio_info->io_in_use == IO_IN_USE_CODEC) {
			I2S_info("use CODEC\n");
#if (BOARD_DESIGN_VER_I2S >= 204)
			ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_C, NULL);
#endif
		} else {
			I2S_info("use HDMI\n");
#if (BOARD_DESIGN_VER_I2S >= 204)
			ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_C, NULL);
#endif
		}
	}

	memcpy(&tx_settings, pAudio_info, sizeof(Audio_Info));

	if (((tx_settings.io_in_use == IO_IN_USE_CODEC) && !Codec_available)
		|| ((tx_settings.io_in_use == IO_IN_USE_HDMI) && !HDMI_available))
	{
		if (!Codec_available)
			I2S_err("CODEC unavailable\n");
		else
			I2S_err("HDMI unavailable\n");

#if 1 /*(PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)*/
		/*
		** For AST1500 platform HW analog codec using the same I2S channel as
		** HDMI's I2S. User expect analog codec source can output to HDMI audio.
		** Solution:
		** Enable 'force_dual_output_support'.
		*/
		if ((force_dual_output_support) || (3 <= ast_scu.ability.soc_op_mode)) {
			if (HDMI_available) {
				tx_settings.io_in_use = IO_IN_USE_HDMI;
				gen_audio_info(&tx_settings, NULL);
			} else if (Codec_available) {
				tx_settings.io_in_use = IO_IN_USE_CODEC;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
#endif
	}
	IO_in_Use = tx_settings.io_in_use;

	i2s_tx_buff_free();
	select_buf_size(&tx_settings, &Tx_Buffer.buf_size, &tx_buf_size_shift);
	i2s_tx_buff_alloc(Tx_Buffer.buf_size);
	max_net_num_tx = TX_SIZE_TO_PACKET_NUM(MAX_RX_SIZE);
	I2S_info("max_net_num_tx = %d\n", max_net_num_tx);

#if defined(I2S_DATA_DELAY)
	/* update_tx_delay_queue need Tx_Buffer.buf_num and max_net_num_tx initialized. */
	update_tx_delay_queue(&tx_settings, Tx_Buffer.buf_size, max_net_num_tx << 2);
#endif

	net_tx_idx = 0;

	if (IO_in_Use == IO_IN_USE_CODEC) {
		unsigned int enable_upstream = 0;
		u32 client_codec_rx = 0;

		crt_disable_audio(crt, OWNER_VE1);
		if ((tx_settings.enable_upstream) && (upstream_available)) {
			I2S_info("upstream enabled\n");
			enable_upstream = 1;
			client_codec_rx = 1;
		} else {
			I2S_info("upstream disabled\n");
		}

		if (enable_upstream) {
			setup_audio_engine(&tx_settings, &Rx_Buffer, &Tx_Buffer, 1, 1);
			enable_rx();
			wake_up_interruptible(&rx_wait_queue);
		} else {
			setup_audio_engine(&tx_settings, NULL, &Tx_Buffer, 1, 1);
		}
		SetupCodec(enable_upstream, 1, mic_input_pin, CODEC_OUTPUT_HP);
		analog_in_volume_cfg(analog_in_vol); analog_out_volume_cfg(analog_out_vol);
	} else {
		setup_audio_engine(&tx_settings, NULL, &Tx_Buffer, 1, 0);
		memcpy(&crt->audio_info, &tx_settings, sizeof(Audio_Info));

		crt_setup_audio(crt, OWNER_VE1);

#if (CONFIG_AST1500_SOC_VER == 1)
		/* Bruce120517. RctBug#2011092200. */
		if ((tx_settings.Audio_Type & AST_AUDIO_HBR | AST_AUDIO_NLPCM) == (AST_AUDIO_HBR | AST_AUDIO_NLPCM)) {
			I2S_err("AST1500 does not support NLPCM audio with more than 2 channels!!!");
			crt_disable_audio(crt, OWNER_VE1);
		}
#endif
	}

#if (PATCH_CODEV3 & PATCH_AST1520A0_I2S_DUAL_OUTPUT)
	if (!((force_dual_output_support) || (3 <= ast_scu.ability.soc_op_mode)))
		return 1;

	if (IO_SELECT_AUTO != IO_Select) {
		if (i2s_dual_output_en)
			i2s_dual_output_cfg(0, 0);
		return 1;
	}

	if (i2s_dual_output_en) {
		if (IO_IN_USE_CODEC == IO_in_Use) {
			/* TODO create crt->audio_info and call crt_setup_audio() */
			sys_i2s_dual_output_cfg(I2S_DUAL_OUTPUT_EN_DEFAULT);
			gen_audio_info(&crt->audio_info, &tx_settings);
			crt_setup_audio(crt, OWNER_VE1);
		} else {
			/*
			 * SampleFrequency is smaller than 96K and non-NLPCM
			 *
			 * 1. we have checked SampleFrequency in select_buf_size() before, just use pAudio_info->SampleFrequency for check
			 *	(SampleFrequency: 10 => 96K, please refer to sample_rate_table[] for more details)
			 * 2. Audio_Type is 0x10 means NLPCM
			 */
			/* FIXME SampleFrequency >= 10 is not correct anymore in HDMI spec 2.0 (p.84) */
			if ((10 >= pAudio_info->SampleFrequency)
				&& !(pAudio_info->Audio_Type & AST_AUDIO_NLPCM)) {
				/* including CODEC configuration */
				sys_i2s_dual_output_cfg(I2S_DUAL_OUTPUT_EN_DEFAULT);
			} else {
				i2s_dual_output_cfg(0, 0);
			}
		}
	}
#endif

	i2s_tx_sys_init();

	return 1;
}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */

void reset_audio_engine(void)
{
	stop_audio_engine();
	net_tx_idx = 0;
#if (CONFIG_AST1500_SOC_VER >= 2)
	if (IO_in_Use == IO_IN_USE_CODEC) {
		unsigned int enable_upstream = 0;
		if ((tx_settings.enable_upstream) && (upstream_available)) {
			I2S_info("upstream enabled\n");
			enable_upstream = 1;
		} else {
			I2S_info("upstream disabled\n");
		}

		if (enable_upstream) {
			setup_audio_engine(&tx_settings, &Rx_Buffer, &Tx_Buffer, 1, 1);
			enable_rx();
			wake_up_interruptible(&rx_wait_queue);
		} else {
			setup_audio_engine(&tx_settings, NULL, &Tx_Buffer, 1, 1);
		}
	} else
#endif
	{
		setup_audio_engine(&tx_settings, NULL, &Tx_Buffer, 1, 0);
	}
}

#if (CONFIG_AST1500_SOC_VER >= 2)
static int drop_packets_udp(struct socket *tx_data_socket, u32 threshold)
{
	int ret = 0;

	while (tx_buffer_filled() > threshold) {
		ret = socket_xmit(0, tx_data_socket, (char *)(&i2s_tx_hdr_buf), I2S_HDR_SIZE, 0);
		if (ret < I2S_HDR_SIZE) {
			I2S_info("wrong total size!(%d)\n", ret);
			ret = -4;
			goto out;
		}
		yield();
	}
out:
	return ret;
}

/* to do: detect audio change and set up audio engine with new setting in _drop_packets */
int _drop_packets(struct socket *tx_data_socket)
{
	return drop_packets_udp(tx_data_socket, Tx_Buffer.buf_num >> 1);
}
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_DATA_DELAY)
static u32 tx_data_drop_tolerance = I2S_TX_DELAY_QUEUE_TOLERANCE;
#endif

static int tx_data_udp(struct socket *tx_data_socket)
{
	struct kvec iov[max_net_num_tx + 1];
	size_t iov_len;
	unsigned int i, j, total_buf_size;
	unsigned int max_net_num;
	unsigned int pkt_num = 0;
	int rx_size = 0;
	static unsigned long src_stable_timeout = 0;
	static unsigned long drop_period = 0;
	u32 size;

	/* DO NOT DROP packet during tx, it will cause pop noise */
#if 0
#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_DATA_DELAY)
#if defined(I2S_CLOCK_ADJUST)
	/* with audio gen-lock, set drop threshold to clk adjust maximum number */
	drop_packets_udp(tx_data_socket, clk_adjust_length_max());
#else
	drop_packets_udp(tx_data_socket, tx_queue_len() + tx_data_drop_tolerance);
#endif
#endif
#endif

	iov[0].iov_base = (char *)&i2s_tx_hdr_buf;
	iov[0].iov_len = I2S_HDR_SIZE;
	iov_len = 1;
	total_buf_size = I2S_HDR_SIZE;

	/* Only receive header when tx_ready is false. (Tx_Buffer may not be valid) */
	if (tx_ready) {
		max_net_num = max_net_num_tx;
#if (CONFIG_AST1500_SOC_VER >= 2)
		{
			unsigned int free_buf_num;
			//Bruce120419. We first check whether the buffer queue is full.
			free_buf_num = Tx_Buffer.buf_num - (tx_buffer_filled() + 1);
			if (free_buf_num < max_net_num_tx) {
				if (free_buf_num <= 1) {
					printk("F\n");
					//Check and do the patch if buffer queue is full.
					if (do_tx_buf_overflow_patch(reset_dead_audio_engine)) {
						//If we did the patch, the audio engine will be reset and buffer queue will be reset.
						//So the free_buf_num will be Tx_Buffer.buf_num.
						free_buf_num = Tx_Buffer.buf_num;
					} else
						max_net_num = 0;
				} else
					max_net_num = 0;
			}
		}
#endif

		if (max_net_num != 0) {
			for (i = 0, j = net_tx_idx; i < max_net_num; i++, j++) {
				if (j == Tx_Buffer.buf_num)
					j = 0;

#if (CONFIG_AST1500_SOC_VER == 1)
				if (is_desc_own_by_eng(j)) {
					//I2S_info("!!!!!!Buffer overflow!! %d\n", j);
					break;
				}
#endif
				iov[i + 1].iov_base = (char *)(tx_buf_va[j]);
				iov[i + 1].iov_len	= Tx_Buffer.buf_size;
				iov_len++;
				total_buf_size += Tx_Buffer.buf_size;
			}
		}
	}

	/* recv from host */
	rx_size = socket_xmit_iov_udp(0, tx_data_socket, iov, iov_len, total_buf_size, 0);

	if (rx_size <= 0) {
		I2S_info("socket_xmit_iov_udp rx failed\n");
		return -1;
	}

	if (rx_size < I2S_HDR_SIZE) {
		I2S_info("wrong total size!(%d)\n", rx_size);
		return 0;
	}

	if (i2s_tx_hdr_buf.magic != I2S_MAGIC) {
		I2S_err("Wrong Magic number?! (0x%08X vs 0x%08X)\n", i2s_tx_hdr_buf.magic, I2S_MAGIC);
		return 0;
	}

	if (i2s_tx_hdr_buf.from != current_sender || i2s_tx_hdr_buf.cfg_sn != current_cfg_sn
#if defined(CONFIG_ARCH_AST1500_CLIENT)
			    /*Bruce120808.
				** Bug scenario: Host video hotplug->audio ready->video mode change (crt reset audio_ready)->video ready.
				** If we don't check "crt->audio_ready" here, there is no chance for crt to set
				** xmiter's audio.
				*/
			    || (IO_in_Use == IO_IN_USE_HDMI && !crt->audio_ready)
#endif
				)
	{
		if (i2s_tx_hdr_buf.from != current_sender) {
			if (!src_stable_timeout) {
				src_stable_timeout = jiffies + msecs_to_jiffies(the_src_stable_timeout);
				return 0;
			} else if (time_before(jiffies, src_stable_timeout)) {
				return 0;
			}
		}
#if defined(CONFIG_ARCH_AST1500_CLIENT)
		//qzx 2021.11.09:When an audio change is detected, mute audio_out first,Prevent pop sound
		gpio_direction_output(AST1500_GPF1,0);
#endif
		I2S_info("audio chg!\n");
		src_stable_timeout = 0;
		current_sender = i2s_tx_hdr_buf.from;
		current_cfg_sn = i2s_tx_hdr_buf.cfg_sn;
		drop_period = jiffies + msecs_to_jiffies(the_drop_period);
#if defined(CONFIG_ARCH_AST1500_CLIENT)
		if (client_handle_audio_chg(&i2s_tx_hdr_buf.a.info)) {
#if defined(I2S_SYNC_CHECK_NET_RX)
			net_rx_check_start(1);
#endif
			return 0;
		}
#endif
	}
	src_stable_timeout = 0;

	rx_size -= I2S_HDR_SIZE;
	if (rx_size != i2s_tx_hdr_buf.pkt_size) {
		if (tx_ready && time_after(jiffies, drop_period)) {
			I2S_info("D r(%d), n(%d)\n", rx_size, i2s_tx_hdr_buf.pkt_size);
			drop_period = jiffies + msecs_to_jiffies(the_drop_period);
#if defined(CONFIG_ARCH_AST1500_CLIENT) && defined(I2S_CLOCK_ADJUST)
			clock_fixup_tx_full();
#endif
		}
		return 0;
	}

	size = rx_size;

	if (size) {
		pkt_num = TX_SIZE_TO_PACKET_NUM(size);
#if defined(I2S_SYNC_CHECK_NET_RX)
		if (net_rx_check(pkt_num))
			return 0;
#endif
#if HDCP2_Encryption
		if (i2s_tx_hdr_buf.HDCP2_encrypted) {
			unsigned int status;
			while (((status = AST_AES_Acquire(AST_AES_USER_HDCP, AST_AES_FLAG_SET, i2s_tx_hdr_buf.inputCtr, HDCP2_AES_INPUTCTR_LENGTH)) == AST_AES_STATUS_BUSY) && i2sInit)
				yield();

			if (!i2sInit)
				return -1;

			if (status == AST_AES_STATUS_SUCCESSS) {
				if (debug_aes_encryption) {
					printk("inputCtr:\n");
					for (i = 0; i < HDCP2_AES_INPUTCTR_LENGTH; i++) {
						if (i == (HDCP2_AES_INPUTCTR_LENGTH - 1))
							printk("%02X\n", i2s_tx_hdr_buf.inputCtr[i]);
						else
							printk("%02X ", i2s_tx_hdr_buf.inputCtr[i]);
					}
				}

				for (i = 0, j = net_tx_idx; i < pkt_num; i++, j++) {
					if (j == Tx_Buffer.buf_num)
						j = 0;
					if (i == 0) {
						AST_AES_Decrypt((void *)tx_buf_pa[j], Tx_Buffer.buf_size, (void *)tx_buf_pa[j], AST_AES_FLAG_LOAD);
						if (debug_aes_encryption) {
							unsigned int k;
							unsigned char *buf = tx_buf_va[j];
							for (k = 0; k < 16; k++) {
								if (k == 15)
									printk("%02X\n", buf[k]);
								else
									printk("%02X ", buf[k]);
							}

							for (k = 0; k < 16; k++) {
								if (k == 15)
									printk("%02X\n", buf[Tx_Buffer.buf_size - 1 - k]);
								else
									printk("%02X ", buf[Tx_Buffer.buf_size - 1 - k]);
							}
						}
					} else
						AST_AES_Decrypt((void *)tx_buf_pa[j], Tx_Buffer.buf_size, (void *)tx_buf_pa[j], 0);
				}
				AST_AES_Release();
			} else if (status == AST_AES_STATUS_NO_CONTEXT) {
				/* skip these data */
				return 0;
			} else {
				printk("AST_AES_Acquire return %d\n", status);
				BUG();
			}
		}
#endif
		net_tx_idx += pkt_num;

		if (net_tx_idx >= Tx_Buffer.buf_num)
			net_tx_idx -= Tx_Buffer.buf_num;

		kick_tx_desc(net_tx_idx);
	}

	return 0;
}

static int (*tx_data_handler)(struct socket *tx_data_socket);

/* This thread receives data from network and pass them to audio engine */
static void tx_data(struct i2s_task *i2st)
{
	struct socket *tx_data_socket = i2st->arg;

	I2S_info("enter tx_data\n");

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	i2s_tx_sys_init();
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */

	tx_data_handler = tx_data_udp;

	while (1) {
		if (signal_pending (current) || !i2sInit)
			break;

		if ((*tx_data_handler)(tx_data_socket) < 0)
			break;
	}

	I2S_info("exit tx_data\n");
}

#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
static void clock_adj_loop(struct i2s_task *i2st)
{
	while (1) {
		if (signal_pending(current))
			break;

		/*
		 * the condition of previous implementation is tx_ready && !rx_ready
		 * change to tx_ready to include the case with MIC
		 */
		if (wait_event_interruptible(clock_adj_wait_queue, tx_ready) != 0)
			break;

		clk_adj_proc(tx_buffer_filled());

		if (clock_adj_interval)
			//Bruce120824. interruptible so that we can speed up i2s_cleanup() time.
			msleep_interruptible(clock_adj_interval);
		else
			yield();
	}
}
#endif

#ifdef CONFIG_ARCH_AST1500_HOST

#if defined(I2S_CLK_INFO_FROM_REMOTE)
static u32 i2s_sample_frequency_in_hz(void)
{
	u64 dividend, divisor;
	u32 ret, n, cts, pixel_clk;

	vrxhal_get_video_hdmi_n_cts(&n, &cts);
	pixel_clk = vrxhal_pixel_clk();

	if ((n == 0) || (cts == 0) || (pixel_clk == 0)) {
		ret = 0;
		goto exit;
	}

	I2S_info("pixel_clk %d, n %d, cts %d\n", pixel_clk, n, cts);

	/* 128 * fs = TMDS * N / CTS, fs = (TMDS * N) / (CTS *128)*/
	dividend = (u64) pixel_clk * (u64) n;
	divisor = (u64) cts * 128;
	do_div(dividend, divisor);
	ret = (u32) dividend;
exit:
	return ret;
}

static int i2s_sample_frequency_in_ppm(Audio_Info *pinfo, u32 fs)
{
	u32 rate_idx = pinfo->SampleFrequency;
	u32 rate = 0;
	u32 i;
	int ret;

	if (fs == 0) {
		ret = 0;
		goto exit;
	}

	for (i = 0; i < sample_rate_num; i++) {
		if (sample_rate_table[i].sample_rate_index == rate_idx) {
			rate = sample_rate_table[i].sample_rate;
			break;
		}
	}

	if (rate == 0) {
		I2S_err("unsupported sample rate idx = %d\n", rate_idx);
		BUG();
	}

	ret = abs(rate - fs) * 1000000 / rate;

	if (fs < rate)
		ret = ret * (-1);
exit:
	return ret;
}
#endif

static void handle_events(void)
{
	ULONG flags;
	struct list_head	*plist, *ptemp;
	pevent_struct	pevent;
	u32	event;
	void	*pdata;
	struct socket *pfrom;

	spin_lock_irqsave(&ctrl_event_list_lock, flags);
	list_for_each_safe(plist, ptemp, &ctrl_event_list){
		pevent = list_entry(plist, event_struct, list);
		list_del(plist);
		ctrl_event_list_size--;
		spin_unlock_irqrestore(&ctrl_event_list_lock, flags);

		event = pevent->event;
		pdata = pevent->pdata;
		pfrom = pevent->pfrom;
		kfree(pevent);

		//process events
		switch (event) {
		case EVENT_AUDIO_SETTINGS_CHANGED:
		case EVENT_CODEC_SETTINGS_CHANGED:
		case EVENT_HDMI_SETTINGS_CHANGED:
			if (rx_settings.Audio_On) {
				stop_audio_engine();
				//We should not go on until rx_data thread has sent out all remaining data..
				//We must guarantee no more data will be sent after rx_idle is set.
				I2S_info("rx_data idle\n");
				i2s_rx_buff_free();
			}

			if (event == EVENT_CODEC_SETTINGS_CHANGED) {
				I2S_info("EVENT_CODEC_SETTINGS_CHANGED\n");
				memcpy(&rx_settings, &codec_settings, sizeof(Audio_Info));
			} else if (event == EVENT_HDMI_SETTINGS_CHANGED) {
				I2S_info("EVENT_HDMI_SETTINGS_CHANGED\n");
				memcpy(&rx_settings, &hdmi_settings, sizeof(Audio_Info));
			} else
				I2S_info("EVENT_AUDIO_SETTINGS_CHANGED\n");

			if (rx_settings.Audio_On) {
				i2s_rx_hdr_buf.cfg_sn++;
#if defined(I2S_CLK_INFO_FROM_REMOTE)
				if (event == EVENT_HDMI_SETTINGS_CHANGED)
					i2s_rx_hdr_buf.sample_frequency_in_ppm = i2s_sample_frequency_in_ppm(&rx_settings, i2s_sample_frequency_in_hz());
				else
					i2s_rx_hdr_buf.sample_frequency_in_ppm = 0;

				I2S_info("clock %d ppm\n", i2s_rx_hdr_buf.sample_frequency_in_ppm);
#endif

				memcpy(&i2s_rx_hdr_buf.a.info, &rx_settings, sizeof(Audio_Info));

				select_buf_size(&rx_settings, &Rx_Buffer.buf_size, &rx_buf_size_shift);
				i2s_rx_buff_alloc(Rx_Buffer.buf_size);
				max_net_num_rx = RX_SIZE_TO_PACKET_NUM(MAX_RX_SIZE);
				if (rx_settings.io_in_use == IO_IN_USE_CODEC) {
#if (CONFIG_AST1500_SOC_VER >= 2)
					if (rx_settings.enable_upstream) {
						I2S_info("enable upstream\n");
						net_tx_idx = 0;
						setup_audio_engine(&rx_settings, &Rx_Buffer, &Tx_Buffer, 1, 1);
					} else
#endif
					{
						I2S_info("disable upstream\n");
						setup_audio_engine(&rx_settings, &Rx_Buffer, NULL, 1, 1);
					}

					SetupCodec(1, rx_settings.enable_upstream, CODEC_INPUT_LINE, CODEC_OUTPUT_HP);
					analog_in_volume_cfg(analog_in_vol); analog_out_volume_cfg(analog_out_vol);
				}
				else
					setup_audio_engine(&rx_settings, &Rx_Buffer, NULL, 0, 0);
			}
			break;

		default:
			I2S_err("unknown event (%08X)\n", event);
		}

		if (pdata != NULL)
			kfree(pdata);

		spin_lock_irqsave(&ctrl_event_list_lock, flags);
	}
	spin_unlock_irqrestore(&ctrl_event_list_lock, flags);
}
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#if defined(AST_I2S_RX_DEBUG)
static void buffer_dump(u32 index, u32 len)
{
	int i, num, *ptr, val;
	u8 *data;

	data = rx_buf_va[index];

	num = len >> 2;
	if (rx_buf_log_cnt) {
		ptr = (int *) data;

		for (i = 0; i < num; i++) {
			val = (*ptr) & 0xFFFFFF;
			if (val & 0x800000)
				val = ((*ptr) & 0x7FFFFF) - 0x800000;

			if (i & 1) {
				printk("%d\n", val);
				rx_buf_log_cnt--;
			} else {
				printk("%d\t", val);
			}
			ptr++;

			if (!rx_buf_log_cnt)
				break;
		}
	}
}
#endif

static int i2s_rx_net_tx_udp(struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size)
{
	int tx_size, ret = 0;

#if defined(CONFIG_ARCH_AST1500_HOST) && (BOARD_DESIGN_VER_I2S >= 204)
	/* Bruce160712. Host only send packets when codec plugged. */
	/* qzx210714. In IPE5000-A30,The pin is reused as other functions,Therefore, the pin is not judged */
/* 	if ((IO_in_Use == IO_IN_USE_CODEC) && (codec_in_status != CODEC_IN_STATUS_PLUGGED))
		return 0; */
#endif
	tx_size = socket_xmit_iov_udp(1, sock, rq_iov, rq_iovlen, size, 0);

	if (tx_size < 0) {
		if (tx_size != -ECONNREFUSED) {
			/* Bruce110617. For bug#2011061700. Just ignore this packet should be OK.
			** From google.
			** A previous UDP packet to the same machine and port got an ICMP message,
			** probably port not available.  Linux treats some ICMP messages as
			** ECONNREFUSED for UDP.  Not all *nix do this, Alan Cox says that Linux
			** is conforming to the RFC (search linux-net and linux-kernel archives for
			** ECONNREFUSED).
			**
			** Simple workaround.  Treat ECONNREFUSED as "try again", up to an
			** arbitrary number of attempts.
			**
			** for (i = 0; i < 5; ++i) {
			**     if (sendto(...) >= 0 || errno != ECONNREFUSED)
			**         break;
			** }
			**
			** Works on all systems.
			*/
			I2S_info("i2s host transmit data error (%d, %d)\n", I2S_HDR_SIZE + size, tx_size);
			ret = -1;
		}
	}
	return ret;

}

static int (*i2s_rx_net_tx_handler)(struct socket *sock, struct kvec *rq_iov, size_t rq_iovlen, int size);

static void rx_thread(struct i2s_task *i2st)
{
	unsigned int packet_num = 0, size, start_index = 0;
	unsigned int max_pkt_num;
	i2s_rx_net_tx_handler = i2s_rx_net_tx_udp;

	I2S_info("enter rx_thread\n");

	while (1) {

		if (signal_pending(current))
			break;

#ifdef CONFIG_ARCH_AST1500_HOST
#if (CONFIG_AST1500_SOC_VER == 1)
		if (IO_in_Use == IO_IN_USE_HDMI) {
			static unsigned long t = 0;
			//driven by CPLD, indicates that there is something wrong
			if (!gpio_get_value(AST1500_GPC4)) {
				if ((t == 0) || time_after(jiffies, t)) {
					printk("RA\n");
					ResetAudio();
					/* Bruce140526. bug fix. GW Audio DSP issue.
					** Frequently reset IT6605 may not be able to recover
					** CPLD lock issue and may get worse.
					** So, we do the reset in at least 500ms interval.
					*/
					t = jiffies + msecs_to_jiffies(500);
				}
			} else {
				/*
				** Bruce170106. t is not initialized as a jiffies value. ==> random bug.
				** Above time_after(jiffies, t) check may unexpected fail due
				** to 0 initialized t value.
				** Solution:
				** - We should double check t == 0 case.
				** - Reset t to 0 when audio recoverred. ==> Ensure 'RA' be
				**   triggered everytime GPIOC4 goes to low.
				*/
				t = 0;
			}
		}
#endif
		if (wait_event_interruptible(rx_wait_queue, ((rx_data_socket && rx_enabled && rx_data_available) || (!list_empty(&ctrl_event_list)))) != 0) {
			if (!list_empty(&ctrl_event_list))
				I2S_info("ctrl event queue not empty!!!\n");
			break;
		}

		if (!list_empty(&ctrl_event_list)) {
			handle_events();
			if (rx_data_socket && rx_ready)
				enable_rx();
			continue;
		}
#else//#ifdef CONFIG_ARCH_AST1500_HOST
		if (wait_event_interruptible(rx_wait_queue, (rx_data_socket && rx_enabled && rx_data_available)) != 0)
			break;
#endif

		max_pkt_num = max_net_num_rx;
		collect_rx_data(max_pkt_num, &packet_num, &start_index);

		if (packet_num != 0) { /* have got some data to send */
			struct kvec iov[packet_num + 1];
			unsigned int i, j;
#if HDCP2_Encryption
			unsigned int status = AST_AES_STATUS_NO_CONTEXT;
#endif

			size = Rx_Buffer.buf_size * packet_num;

			/* header */
			i2s_rx_hdr_buf.pkt_size = size;
#if HDCP2_Encryption
#ifdef CONFIG_ARCH_AST1500_HOST
			if (enable_aes_encryption) {
				while ((status = AST_AES_Acquire(AST_AES_USER_HDCP, AST_AES_FLAG_GET, i2s_rx_hdr_buf.inputCtr, HDCP2_AES_INPUTCTR_LENGTH)) == AST_AES_STATUS_BUSY)
					yield();
				if (status == AST_AES_STATUS_SUCCESSS) {
					if (debug_aes_encryption) {
						unsigned int i;
						printk("inputCtr:\n");
						for (i = 0; i < HDCP2_AES_INPUTCTR_LENGTH; i++) {
							printk("%02X ", i2s_rx_hdr_buf.inputCtr[i]);
							if (i == (HDCP2_AES_INPUTCTR_LENGTH - 1))
								printk("\n");
						}
					}
					i2s_rx_hdr_buf.HDCP2_encrypted = 1;
				} else /* for AST_AES_STATUS_NO_CONTEXT */
					i2s_rx_hdr_buf.HDCP2_encrypted = 0;
			} else
#endif
				i2s_rx_hdr_buf.HDCP2_encrypted = 0;
#endif
			iov[0].iov_base = (char *)&i2s_rx_hdr_buf;
			iov[0].iov_len	= I2S_HDR_SIZE;

			for (i = 0, j = start_index; i < packet_num; i++, j++) {
				if (j == Rx_Buffer.buf_num)
					j = 0;
				iov[i + 1].iov_base = (char *)(rx_buf_va[j]);
				iov[i + 1].iov_len	= Rx_Buffer.buf_size;

#if defined(AST_I2S_RX_DEBUG)
				/* dump rx_buffer */
				buffer_dump(j, Rx_Buffer.buf_size);
#endif
#if HDCP2_Encryption
				if (i2s_rx_hdr_buf.HDCP2_encrypted) {
					if ((debug_aes_encryption) && (i == 0)) {
						unsigned int k;
						unsigned char *buf = rx_buf_va[j];

						for (k = 0; k < 16; k++) {
							printk("%02X ", buf[k]);
							if (k == 15)
								printk("\n");
						}

						for (k = 0; k < 16; k++) {
							printk("%02X ", buf[Rx_Buffer.buf_size - 1 - k]);
							if (k == 15)
								printk("\n");
						}
					}

					if (i == (packet_num - 1))
						AST_AES_Encrypt((void *)rx_buf_pa[j], Rx_Buffer.buf_size, (void *)rx_buf_pa[j], AST_AES_FLAG_SAVE);
					else
						AST_AES_Encrypt((void *)rx_buf_pa[j], Rx_Buffer.buf_size, (void *)rx_buf_pa[j], 0);
				}
#endif
			}
#if HDCP2_Encryption
			if (status == AST_AES_STATUS_SUCCESSS)
				AST_AES_Release();
#endif

			if ((*i2s_rx_net_tx_handler)(rx_data_socket, iov, packet_num + 1, I2S_HDR_SIZE + size))
				break;

			reclaim_rx_desc(start_index);
			packet_num = 0;
		}

		if (rx_enabled) {
			rx_data_available = poll_rx_data();
			if (!rx_data_available)
				enable_rx_int();
		} else {
			/* Audio settings have been changed */
			rx_data_available = 0;
		}
	}

	I2S_info("exit rx_thread\n");
}

#if (BOARD_DESIGN_VER_I2S >= 204)
#define CHECK_HOTPLUG_INTERVAL 3000
static int is_plugged(void)
{
	int is_plugged;

#if (BOARD_DESIGN_VER_I2S >= 300)
#ifdef CONFIG_ARCH_AST1500_HOST
	/* low active */
	is_plugged = !gpio_get_value(GPIO_AUDIO_CODEC_IN_HOTPLUG);
#else
	/* low active */
	is_plugged = !gpio_get_value(GPIO_AUDIO_CODEC_IN_HOTPLUG);
#endif
#else /* #if (BOARD_DESIGN_VER_I2S >= 300) */
#ifdef CONFIG_ARCH_AST1500_HOST
	is_plugged = gpio_get_value(GPIO_AUDIO_CODEC_IN_HOTPLUG);
#else //#ifdef CONFIG_ARCH_AST1500_HOST
	is_plugged = !(gpio_get_value(GPIO_AUDIO_CODEC_IN_HOTPLUG));
#endif
#endif /* #if (BOARD_DESIGN_VER_I2S >= 300) */

	/* check bit9 a_input. */
	if (ast_scu.astparam.gpio_inv & (1 << 9))
		is_plugged = !is_plugged;

	return is_plugged;
}

static void hotplug_proc(unsigned long ptr)
{
	/* check input */
	if (is_plugged()) { /* CODEC is plugged */
		if (codec_in_status != CODEC_IN_STATUS_PLUGGED) {
			I2S_info("CODEC is plugged!\n");
			codec_in_status = CODEC_IN_STATUS_PLUGGED;
#ifdef CONFIG_ARCH_AST1500_HOST
#ifdef I2S_LEGACY_AUTO_MODE
			if (IO_Select == IO_SELECT_AUTO) {
				if (IO_in_Use != IO_IN_USE_CODEC) {
					I2S_info("switch to CODEC\n");
					IO_in_Use = IO_IN_USE_CODEC;
					ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, NULL);
					add_event(EVENT_CODEC_SETTINGS_CHANGED, NULL, NULL);
				}
			}
#endif
#endif
		}
	} else { /* CODEC is unplugged */
		if (codec_in_status != CODEC_IN_STATUS_UNPLUGGED) {
			I2S_info("CODEC is unplugged!\n");
			codec_in_status = CODEC_IN_STATUS_UNPLUGGED;
#ifdef CONFIG_ARCH_AST1500_HOST
#ifdef I2S_LEGACY_AUTO_MODE
			if (IO_Select == IO_SELECT_AUTO) {
				if (IO_in_Use != IO_IN_USE_HDMI) {
					I2S_info("switch to HDMI\n");

					if (IO_in_Use == IO_IN_USE_CODEC)
						PowerDownCodec();

					IO_in_Use = IO_IN_USE_HDMI;
					ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_H, NULL);

					if (hdmi_settings_valid)
						add_event(EVENT_HDMI_SETTINGS_CHANGED, NULL, NULL);
				}
			}
#endif
#endif
		}
	}

	queue_delayed_work(wq, &hotplug_work, msecs_to_jiffies(CHECK_HOTPLUG_INTERVAL));
}
#endif /* #if (BOARD_DESIGN_VER_I2S >= 204) */

static void i2s_rx_hdr_init(void)
{
	memset(&i2s_rx_hdr_buf, 0, I2S_HDR_SIZE);
	i2s_rx_hdr_buf.magic = I2S_MAGIC;
	get_random_bytes(&i2s_rx_hdr_buf.from, sizeof(i2s_rx_hdr_buf.from));
	I2S_info("i2s_rx_hdr_buf.from=0x%08X\n", i2s_rx_hdr_buf.from);
	i2s_rx_hdr_buf.cfg_sn = i2s_rx_hdr_buf.from;
}

static void audio_info_codec_default(Audio_Info *a_info)
{
	memset(a_info, 0, sizeof(Audio_Info));

	a_info->SampleFrequency = 2;
	a_info->ValidCh = 2;
	a_info->SampleSize = 24;
#if (API_VER >= 2)
	/* Bruce161212. codec hybrid to it6613 need Audio_Type to be 0x80. */
	a_info->Audio_Type = AST_AUDIO_ON;
	a_info->DownSampling = 0;
	a_info->Mode16bits = 0;
#endif
}

#ifdef CONFIG_ARCH_AST1500_HOST
/*
 * i2s_host_init -
 *
 * return 0 on success and a different value on error (failure)
 */
static int i2s_host_init(void)
{
	i2s_rx = 1;

	if (CodecExist()) {
		I2S_info("CODEC available\n");
		Codec_available = 1;
		audio_info_codec_default(&codec_settings);

		codec_settings.Audio_On = 1;
#if (API_VER >= 2)
		codec_settings.io_in_use = IO_IN_USE_CODEC;

		if (2 <= ast_scu.ability.soc_op_mode)
			upstream_available = 1;
		else
			upstream_available = 0;
#endif
	}

	if (vrxhal_is_dev_exist()) {
		I2S_info("HDMI available\n");
		HDMI_available = 1;
#if (API_VER >= 2)
		hdmi_settings.io_in_use = IO_IN_USE_HDMI;
		hdmi_settings.DownSampling = 0;
		hdmi_settings.Mode16bits = 0;
#endif
	}

	if (upstream_available)
		i2s_tx = 1;

	if (Codec_available || HDMI_available) {
		useI2S = 1;

		if (Codec_available && HDMI_available) {
#if (CONFIG_AST1500_SOC_VER == 1)
			gpio_direction_input(GPIO_AUDIO_IO_SELECT);
			if (gpio_get_value(GPIO_AUDIO_IO_SELECT)) {
				I2S_info("use CODEC\n");
				IO_in_Use = IO_IN_USE_CODEC;
			} else {
				I2S_info("use HDMI\n");
				IO_in_Use = IO_IN_USE_HDMI;
			}
#else
#if (BOARD_DESIGN_VER_I2S >= 204)
			I2S_info("choose audio IO via hotplug\n");
#endif
#endif
		} else if (Codec_available) {
			I2S_info("use CODEC\n");
			IO_in_Use = IO_IN_USE_CODEC;
#if (CONFIG_AST1500_SOC_VER >= 2) && (BOARD_DESIGN_VER_I2S >= 204)
			ast_scu.scu_op(SCUOP_I2S_PIN_CODEC_H, NULL);
#endif
		} else {
			I2S_info("use HDMI\n");
			IO_in_Use = IO_IN_USE_HDMI;
#if (CONFIG_AST1500_SOC_VER >= 2) && (BOARD_DESIGN_VER_I2S >= 204)
			/*
			** For the case which HW is originally HDMI+codec, but codec was removed.
			** I2S route switches are still available in this case, so we have to
			** initial the GPIO_AUDIO_IO_SELECT value.
			*/
			ast_scu.scu_op(SCUOP_I2S_PIN_HDMI_H, NULL);
#endif
		}

		if (i2s_rx) {
			if (i2s_rx_desc_alloc()) {
				I2S_err("i2s_rx_desc_alloc failed\n");
				BUG();
				return 1;
			}

			i2s_task_init(&rx_data_t, "AUD_RX_DATA", rx_thread);

			init_waitqueue_head(&rx_wait_queue);
			INIT_LIST_HEAD(&ctrl_event_list);
			spin_lock_init(&ctrl_event_list_lock);

			i2s_rx_hdr_init();

			IRQ_SET_HIGH_LEVEL(I2S_INT);
			IRQ_SET_LEVEL_TRIGGER(I2S_INT);

#if (CONFIG_AST1500_SOC_VER == 1)
			if (request_irq(I2S_INT, &i2s_interrupt, SA_INTERRUPT, "1500_i2s", NULL))
#else
			if (request_irq(I2S_INT, &i2s_interrupt, SA_SHIRQ, "1500_i2s", &rx_data_available))
#endif
			{
				I2S_info ("Unable to get I2S IRQ");
			}
		}

		if (i2s_tx) {
			if (i2s_tx_desc_alloc()) {
				I2S_err("i2s_tx_desc_alloc failed\n");
				BUG();
				return 2;
			}

			i2s_task_init(&tx_data_t, "AUD_TX_DATA", tx_data);

			audio_info_codec_default(&tx_settings);
			select_buf_size(&tx_settings, &Tx_Buffer.buf_size, &tx_buf_size_shift);
			i2s_tx_buff_alloc(Tx_Buffer.buf_size);
			max_net_num_tx = TX_SIZE_TO_PACKET_NUM(MAX_RX_SIZE);
		}

		i2sInit = 1;

		if (HDMI_available)
			vrxhal_reg_audio_event_callback(&hdmi_aduio_rx_callback);

		if (IO_in_Use == IO_IN_USE_CODEC)
			add_event(EVENT_CODEC_SETTINGS_CHANGED, NULL, NULL);

#if (BOARD_DESIGN_VER_I2S >= 204)
		/*
		** Bruce160712. To stop audio stream when codec is not plugged,
		** we always check GPIO hotplug when codec is available.
		*/
		if (Codec_available) {
			wq = create_singlethread_workqueue("audio_hotplug");
			if (!wq) {
				I2S_err("Failed to allocate hotplug workqueue?!\n");
				BUG();
				return 3;
			}
			INIT_WORK(&hotplug_work, (void (*)(void *))hotplug_proc, NULL);
			//BruceToDo. Move GPIO config to SCU driver?
			gpio_direction_input(GPIO_AUDIO_CODEC_IN_HOTPLUG);
			/*
			**NOTE: disable GPIO_AUDIO_CODEC_IN_HOTPLUG GPIO pin's internal pull
			**      down resister. This is done in SCU driver's SCUOP_I2S_INIT
			**
			**write_register(SCU_MULTIPIN_CONTROL4_REGISTER,
			**	read_register(SCU_MULTIPIN_CONTROL4_REGISTER) | 0x00800000);
			*/
			queue_work(wq, &hotplug_work);
		}
#endif
	} else
		I2S_info("audio unavailable\n");

	return 0;
}
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

#ifdef CONFIG_ARCH_AST1500_CLIENT
/*
 * i2s_client_init -
 *
 * return 0 on success and a different value on error (failure)
 */
static int i2s_client_init(void)
{
	i2s_tx = 1;

	if (CodecExist()) {
		I2S_info("CODEC available\n");
		Codec_available = 1;

		if (ast_scu.ability.soc_op_mode >= 2) {
			upstream_available = 1;
			i2s_rx = 1;
		}
	}

	if (crt_get_tx_cap(crt) & xCap_HDMI) {
		I2S_info("HDMI available\n");
		HDMI_available = 1;
	}

	if (Codec_available || HDMI_available) {
		useI2S = 1;

		if (i2s_tx) {
			if (i2s_tx_desc_alloc()) {
				I2S_err("i2s_tx_desc_alloc failed\n");
				BUG();
				return 1;
			}
			i2s_task_init(&tx_data_t, "AUD_TX_DATA", tx_data);
#if defined(I2S_CLOCK_ADJUST_IN_THREAD)
			i2s_task_init(&clock_adj_t, "CLK_ADJ", clock_adj_loop);
			init_waitqueue_head(&clock_adj_wait_queue);
#endif
		}

		if (i2s_rx) {
			if (i2s_rx_desc_alloc()) {
				I2S_err("i2s_rx_desc_alloc failed\n");
				BUG();
				return 2;
			}

			i2s_task_init(&rx_data_t, "AUD_RX_DATA", rx_thread);
			init_waitqueue_head(&rx_wait_queue);

			audio_info_codec_default(&rx_settings);

			select_buf_size(&rx_settings, &Rx_Buffer.buf_size, &rx_buf_size_shift);
			i2s_rx_buff_alloc(Rx_Buffer.buf_size);
			max_net_num_rx = RX_SIZE_TO_PACKET_NUM(MAX_RX_SIZE);

			i2s_rx_hdr_init();

			IRQ_SET_HIGH_LEVEL(I2S_INT);
			IRQ_SET_LEVEL_TRIGGER(I2S_INT);
			if (request_irq(I2S_INT, &i2s_interrupt, SA_SHIRQ, "1500_i2s", &rx_data_available)) {
				I2S_err("request_irq failed");
				BUG();
			}
		}
		i2sInit = 1;

#if (BOARD_DESIGN_VER_I2S >= 204)
		if (Codec_available && HDMI_available) {
			wq = create_singlethread_workqueue("audio_hotplug");
			if (!wq) {
				I2S_err("Failed to allocate hotplug workqueue?!\n");
				BUG();
				return 3;
			}
			INIT_WORK(&hotplug_work, (void (*)(void *))hotplug_proc, NULL);
			gpio_direction_input(GPIO_AUDIO_CODEC_IN_HOTPLUG);
			queue_work(wq, &hotplug_work);
		}
#endif
	} else
		I2S_info("audio unavailable\n");

	return 0;
}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */

struct file_operations ioaccess_fops = {
	.ioctl = ioaccess_ioctl,
};

int i2s_init(void)
{
	int    result;
	unsigned long  status = 0;

	I2S_info("--->i2s_init\n");
	result = register_chrdev(ioaccess_major, "1500_i2s", &ioaccess_fops);
	if (result < 0) {
		I2S_err("register_chrdev error (%d)\n", ioaccess_major);
		BUG();
		return result;
	}

	if (ioaccess_major == 0) {
		ioaccess_major = result;
		I2S_info("ioaccess_major = %d\n", ioaccess_major);
	}

	init_MUTEX(&i2s_user_lock);

	if (init_audio_engine()) {
		I2S_err("init_audio_engine failed\n");
		BUG();
		return 3;
	}

	is_client = ast_scu.board_info.is_client;
#ifdef CONFIG_ARCH_AST1500_CLIENT
#if defined(I2S_DATA_DELAY)
	if (1 == ast_scu.ability.soc_op_mode)
		tx_data_drop_tolerance = I2S_TX_DELAY_QUEUE_TOLERANCE_1500;
	else
		tx_data_drop_tolerance = I2S_TX_DELAY_QUEUE_TOLERANCE;
#endif
	if (is_client) {
		crt = crt_connect(OWNER_NONE);
		status = i2s_client_init();
		if (status) {
			I2S_err("i2s_client_init failed\n");
			BUG();
			return status;
		}
	} else {
		I2S_err("not client trapping!!!\n");
		BUG();
		return -1;
	}
#endif
#ifdef CONFIG_ARCH_AST1500_HOST
	if (!is_client) {
		status = i2s_host_init();
		if (status) {
			I2S_err("i2s_host_init failed\n");
			BUG();
			return status;
		}
	} else {
		I2S_err("not host trapping!!!\n");
		BUG();
		return -1;
	}
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
	/* qzx 2021.11.25:The original factory will start the thread only when Rx is connected to TX, 
					which will lead to audio_out has no sound when Rx is not connected, so you can force the process to start
	*/
	start_i2s_thread(&rx_data_t);
#endif
	if (platform_device_register(&pdev) < 0) {
		I2S_err("platform_driver_register err\n");
		BUG();
		return 1;
	}

	if (sysfs_create_group(&pdev.dev.kobj, &dev_attr_group)) {
		I2S_err("can't create sysfs files\n");
		BUG();
		return 1;
	}

	I2S_info("i2s_init<---\n");
	return 0;
}

void i2s_cleanup(void)
{
	I2S_info("--->i2s_cleanup\n");
	i2sInit = 0;

	sysfs_remove_group(&pdev.dev.kobj, &dev_attr_group);
	platform_device_unregister(&pdev);

	unregister_chrdev(ioaccess_major, "1500_i2s");

#if (BOARD_DESIGN_VER_I2S >= 204)
	if (wq) {
		cancel_delayed_work(&hotplug_work);
		flush_workqueue(wq);
		destroy_workqueue(wq);
		wq = NULL;
	}
#endif
	/*
	** Bruce161104. Take dual output into consideration and to avoid pop noise.
	** We always disable codec before stopping audio engine.
	*/
	if (Codec_available)
		PowerDownCodec();

	stop_audio_engine();

	is_client = ast_scu.board_info.is_client;
#ifdef CONFIG_ARCH_AST1500_CLIENT
	if (is_client) { /* CLIENT */
		if (upstream_available)
			free_irq(I2S_INT, &rx_data_available);

		i2s_stop_client();

		i2s_rx_buff_free();
		i2s_rx_desc_free();
		i2s_tx_buff_free();
		i2s_tx_desc_free();
	}
#endif /* #ifdef CONFIG_ARCH_AST1500_CLIENT */
#ifdef CONFIG_ARCH_AST1500_HOST
	if (!is_client) { /* HOST */
		if (useI2S) {
#if (CONFIG_AST1500_SOC_VER == 1)
			free_irq(I2S_INT, NULL);
#else
			free_irq(I2S_INT, &rx_data_available);
#endif
			if (HDMI_available)
				vrxhal_dereg_audio_event_callback();

			i2s_stop_host();

			i2s_tx_buff_free();
			i2s_tx_desc_free();
			i2s_rx_buff_free();
			i2s_rx_desc_free();
		}
	}
#endif /* #ifdef CONFIG_ARCH_AST1500_HOST */

	//A7 removed
	//ast_notify_user("e_i2s_stopped");
	I2S_info("i2s_cleanup<---\n");
	return;
}

module_init(i2s_init);
module_exit(i2s_cleanup);
