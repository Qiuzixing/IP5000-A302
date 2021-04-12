/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#include <asm/arch/drivers/board_def.h>
#if defined(CONFIG_AST1500_SII9679)
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <asm/uaccess.h> // For copy_from_user()
#include <asm/arch/drivers/crt.h>
#include <asm/arch/drivers/video_hal.h>
#include <asm/arch/drivers/board_def.h>
#include "ast_main.h"
#include "ast_utilities.h"

#define MODULE_NAME     "sii9679"
#define REF_FW_VER	"1.02.02 SVN27071"
#define I2C_SPEED	100000 /* 100KHz bus speed */
#define CHIP_ID         0x9679
#define I2C_ADDR        0x70

MODULE_LICENSE ("GPL");

static struct platform_device *pdev;
static struct sii_drv_data *drv_data;

#define MAX_SII9679_DEVS 1
static struct cdev sii9679_devs[MAX_SII9679_DEVS];
static int sii9679_major = 0;

static unsigned char firmware_file[] = {
#include "ast_firmware.h"
};

#if (3 == CONFIG_AST1500_SOC_VER)
static unsigned char firmware_file_splitter[] = {
#include "ast_firmware_splitter.h"
};
#endif

//static u8 *firmware_info(u32 *size)
u8 *firmware_info(u32 *size)
{
	u8 *fw;

	fw = firmware_file;
	*size = ARRAY_SIZE(firmware_file);

#if (3 == CONFIG_AST1500_SOC_VER)
	if (ast_scu.board_info.board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
		fw = firmware_file_splitter;
		*size = ARRAY_SIZE(firmware_file_splitter);
	}
#endif

	return fw;
}

static void firmware_update(struct sii_drv_data *d, u32 force);
static unsigned char *fw_tmp = 0;
static unsigned int fw_tmp_max = 0;
static unsigned int fw_tmp_size = 0;
static int sii9679_fop_open(struct inode *inode, struct file *filp)
{
	/* FIXME. We use firmware_file directly as buffer and assume buffer size is enough. */
	/* FIXME. We assume there is only one user access this dev node at a time. */
	fw_tmp_size = 0;
	fw_tmp = firmware_info(&fw_tmp_max);
	return 0;
}

static int sii9679_fop_release(struct inode *inode, struct file *filp)
{
	if (fw_tmp_size) {
		uinfo("Start programming fw into SPI flash.\n");
		firmware_update(drv_data, 1);
	}

	return 0;
}

static ssize_t sii9679_fop_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	//uinfo("Copy to offset (%ld)\n", (long)*offp);

	if (*offp >= fw_tmp_max) {
		uerr("OOM 1!\n");
		count = -EFAULT;
		goto err_out;
	}
	if ((*offp + count) > fw_tmp_max) {
		uerr("OOM 2!\n");
		count = -EFAULT;
		goto err_out;
	}

	if ((fw_tmp_size == 0) && fw_tmp_max) {
		/* This is the very first file write. */
		memset(fw_tmp, 0, fw_tmp_max);
	}
	if (copy_from_user(fw_tmp + *offp, buff, count)) {
		count = -EFAULT;
		goto out;
	}
	*offp += count;
	fw_tmp_size += count;

out:
	return count;

err_out:
	fw_tmp_size = 0;
	fw_tmp_max = 0;
	return count;
}

static ssize_t sii9679_fop_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	unsigned int retval = 0;
	unsigned int fw_size;
	unsigned char *fw_base;

	fw_size= fw_tmp_max;
	if (*offp >= fw_size)
		goto out;

	if ((*offp + count) > fw_size)
		count = fw_size - *offp;

	fw_base = fw_tmp;
	BUG_ON(!fw_base);
	if (copy_to_user(buff, fw_base + *offp, count)) {
		retval = -EFAULT;
		goto out;
	}

	*offp += count;
	retval = count;
out:
	return retval;
}

static struct file_operations sii9679_fops = {
//	.owner   = THIS_MODULE,
	.open    = sii9679_fop_open,
	.release = sii9679_fop_release,
	.read = sii9679_fop_read,
	.write = sii9679_fop_write,
};



static int sii967x_exists(struct sii_drv_data *d)
{
	SiiDrvAdaptVersionInfo_t info;

	SiiDrvAdaptVersionGet(&d->adapter, &info);

	if ((0x9678 == info.chipId) ||  (0x9679 == info.chipId)) {
		uinfo("Chip ID: %.4x (Rev %.2x) Firmware Version: %d.%.2d.%.2d\n",
			info.chipId, info.chipRev,
			info.fwVersion >> 13, (info.fwVersion & 0x1FFF) >> 5, info.fwVersion & 0x001F);
		return 1;
	}

	return 0;
}

#if (CONFIG_AST1500_SOC_VER >= 3)
static int hdcp_status(void)
{
	int ret = HDCP_DISABLE;

	switch (SiiDrvVirtualPageRead(&drv_data->adapter, HDCP_STATE) & 0x3) {
	case 0: /* no HDCP */
		ret = HDCP_DISABLE;
		break;
	case 1: /* HDCP 1.X */
		ret = HDCP_V_1X;
		break;
	case 2: /* HDCP 2.0 */
		ret = HDCP_V_20;
		break;
	case 3: /* HDCP 2.2 */
		ret = HDCP_V_22;
		break;
	default:
		BUG();
	}

	if (ast_v_hdcp_param() & PARAM_DISABLE_HDCP)
		ret = HDCP_DISABLE;

	return ret;
}

#if defined(SII_9679_UPDATE_EEPROM)
/* copy from si_drv_adapter_rx.c */
static void SiiDrvAdaptEepromEdidWrite(SiiInst_t inst, uint8_t offset, uint8_t *poData, uint16_t length)
{
#define EEPROM_FIFO_SIZE 4
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);
	uint8_t i = 0;
	uint8_t LocalLen = (uint8_t)(length/EEPROM_FIFO_SIZE);
	uint8_t LastLen = (uint8_t)(length%EEPROM_FIFO_SIZE);

	SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_ENABLE);

	for (i = 0; i < LocalLen; i++)
		SiiDrvRegWriteBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], EEPROM_FIFO_SIZE);

	if (0 != LastLen)
		SiiDrvRegWriteBlock(pDrvAdapt, 0xA0, (offset+i*EEPROM_FIFO_SIZE), &poData[i*EEPROM_FIFO_SIZE], LastLen);

	SiiDrvVirtualRegModify(pDrvAdapt, EDID_CTRL, EDID_LOCAL_I2C_MASK, EDID_LOCAL_I2C_DISABLE);
}

static void edid_update(u8 *edid, u32 size)
{
	SiiDrvAdaptEepromEdidWrite(&drv_data->adapter, 0,  edid, size);
}
#endif

static void hook_init(void)
{
	ast_v_hdcp_status = hdcp_status;
#if defined(SII_9679_UPDATE_EEPROM)
#if (3 == CONFIG_AST1500_SOC_VER)
	if (ast_scu.board_info.board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
		external_edid_update = edid_update;
#endif
#endif
}

static void hook_fini(void)
{
	ast_v_hdcp_status = NULL;
#if defined(SII_9679_UPDATE_EEPROM)
#if (3 == CONFIG_AST1500_SOC_VER)
	if (ast_scu.board_info.board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
		external_edid_update = NULL;
#endif
#endif
}
#endif

static void sii_drv_init(struct sii_drv_data *d)
{
	DrvAdaptInst_t *adapter;
	SiiDrvAdaptConfig_t config;

	adapter = &d->adapter;

	adapter->instCra = I2C_HDCP22_RX;
	adapter->pNtfCallback = NULL;
	adapter->accessStatus = SII_DRV_ADAPTER_ACCESS__IN_PROGRESS;
	adapter->upgradeMode = false;
	adapter->isError = false;

	config.platformId = SII_DRV_ADAPT_PLATFORM__SII9678;
	config.interruptPinType = SII_DRV_ADAPTER_INT__PUSHPULL;
	config.interruptPolarity = SII_DRV_ADAPTER_INT__POL_LOW;
	config.i2cSlaveAddr = I2C_ADDR;
	config.i2cSlaveAddrBase = I2C_ADDR_DEBUG;
	config.i2cSlaveAddrF0 = SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_F0;
	config.i2cSlaveAddrAA = SII_DRV_ADAPT_DEFAULT_I2C_DEVICE_ID_AA;

	/* following SPI commands should sync with SPI flash datasheet */
	config.spiConfig.eraseCmd = 0xC7; /* chip erase */
	config.spiConfig.writeEnCmd = 0x06;
	config.spiConfig.pageWrCmd = 0x02;

	SiiDrvAdaptConfigure(adapter, &config);

	SiiDrvAdaptStart(adapter);
}

static int firmware_check(struct sii_drv_data *d, u8 *new_fw)
{
	u16 chip_id, fw_ver, fw_ver_new;
	u8 customer_ver;

	chip_id = (SiiDrvVirtualPageRead(&d->adapter, CHIP_ID_HIGH) << 8) | SiiDrvVirtualPageRead(&d->adapter, CHIP_ID_LOW);

	if (CHIP_ID != chip_id) {
		uerr("Chip ID mismatch. Driver is %.4x, Chip is %.4x\n", CHIP_ID, chip_id);
		return 0;
	}

	fw_ver = (SiiDrvVirtualPageRead(&d->adapter, FW_VER_H) << 8) | SiiDrvVirtualPageRead(&d->adapter, FW_VER_L);

	fw_ver_new = (new_fw[0x19] << 13) | (new_fw[0x18] << 5) | new_fw[0x17];

	/* TODO
	 *
	 * firmware header information check
	 */

	if (fw_ver_new != fw_ver) {
		uinfo("New firmware (%d.%.2d.%.2d, 0x%.4x) != current firmware (%d.%.2d.%.2d, 0x%.4x)\n",
			fw_ver_new >> 13, (fw_ver_new >> 5) & 0x1F, fw_ver_new & 0x1F, fw_ver_new,
			fw_ver >> 13, (fw_ver >> 5) & 0x1F, fw_ver & 0x1F, fw_ver);
		return 1;
	}

	/* customer version check */
	customer_ver = SiiDrvVirtualPageRead(&d->adapter, FW_CUSTOMER_SUB_VER);

	if (new_fw[0x41] != customer_ver) {
		uinfo("New customer version (0x%.2x) != current customer version (0x%.2x)\n", new_fw[0x41], customer_ver);
		return 1;
	}

	return 0;
}

static inline void switch_to_debug(void)
{
	/* set INT to LOW and reset =>  debug mode */
	gpio_direction_output(GPIO_HDCP22_RX_DEBUG_MODE, 0); /* FIXME */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_RX);
}

static inline void switch_to_normal(void)
{
	/* set INT to HIGH and reset => normal mode */
	gpio_direction_output(GPIO_HDCP22_RX_DEBUG_MODE, 1); /* FIXME */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_RX);
}

u8 data[SPI_MAX_PAGE_SIZE];

static int firmware_compare(struct sii_drv_data *d)
{
	u8 *firmware;
	u32 size, bytes_to_work, done;
	u32 result = 0, i;

	firmware = firmware_info(&size);

	SiiDrvAdaptFirmwareReadInit(&d->adapter);
	uinfo("SPI flash reading may take some time, please wait...");
	done = 0;

	while (size) {
		bytes_to_work = (size > SPI_MAX_PAGE_SIZE) ? SPI_MAX_PAGE_SIZE : size;
		SiiSpiFlashBlockRead(&d->adapter, done, data, bytes_to_work);
		for (i = 0; i < bytes_to_work; i++) {
			if (data[i] != *(firmware + done + i)) {
				printk("\n!!! index %d (0x%.8x) not match, flash:0x%.2x firmware 0x%.2x\n",
					i + done, i + done, data[i], *(firmware + done + i));
				result = 1;
				goto check_end;
			}
		}

		size -= bytes_to_work;
		done += bytes_to_work;
		kick_wdt();
	}
	printk("\n");

check_end:
	return result;
}


static void firmware_update(struct sii_drv_data *d, u32 force)
{
	u8 *firmware;
	u32 size;

	firmware = firmware_info(&size);

	/* align to flash block size */
	size = (size + 0xFF) & 0xFFFFFF00;

	if (firmware_check(d, firmware) || force)  {
		SiiDrvAdaptFwBuffer_t fw;

		uinfo("Start updating firmware.");
		fw.pBuffer = firmware;
		fw.ucBufferSize = size;
		fw.ucOffset = 0;
		kick_wdt();
		SiiDrvAdaptFirmwareUpdateInit(&d->adapter);
		SiiDrvAdaptFirmwareUpdate(&d->adapter, &fw);
		kick_wdt();
		printk("\n");

		if (0 == firmware_compare(d)) {
			uinfo("firmware update success.\n");
		} else
			uinfo("firmware update FAIL!!!\n");

		/* FIXME
		 * sometimes no dispay after firmware update,
		 * switch to debug mode and then switch back to workaround
		 */
		switch_to_debug();
		switch_to_normal();
		kick_wdt();
	}
}

#if defined(UPDATE_FW_IN_THREAD)
struct work_struct update_work;
#endif

static void firmware_erase(struct sii_drv_data *d)
{
	uinfo("Start erasing firmware.\n");
	SiiDrvAdaptFirmwareUpdateInit(&d->adapter);
	uinfo("firmware erased.\n");
}

static ssize_t show_reg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sii_drv_data *d = platform_get_drvdata(pdev);
	int i, num = 0;
	u32 start, end;

	start = 0x00;
	end = 0x7f;

	num += snprintf(buf + num, PAGE_SIZE - num, "%s register dump:", MODULE_NAME);

	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
			num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.2x: ", i);

			num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", SiiDrvVirtualPageRead(&d->adapter, i));
	}
	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	return num;
}

static ssize_t store_reg(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sii_drv_data *d = platform_get_drvdata(pdev);
	u32 offset, value, c;

	c = sscanf(buf, "%x %x", &offset, &value);

	if (2 != c) {
		printk("Usage:\nOffset [Value]\n");
	} else {
		printk("Register 0x%02X is 0x%02X.\n", offset, value);
		SiiDrvVirtualPageWrite(&d->adapter, offset, value);
	}

	return count;
}
static DEVICE_ATTR(reg, (S_IRUGO | S_IWUSR), show_reg, store_reg);

static ssize_t show_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sii_drv_data *d = platform_get_drvdata(pdev);
	int num = 0;
	u8 val;

	SiiDrvAdaptHdcpVer_t version;
	SiiDrvAdaptHdcpTxStatus_t status;
	SiiDrvAdaptVersionInfo_t info;

	SiiDrvAdaptVersionGet(&d->adapter, &info);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "Chip ID: %.4x (Rev %.2x)\n",
			info.chipId, info.chipRev);
	num += snprintf(buf + num, PAGE_SIZE - num, "Firmware Version: %d.%.2d.%.2d\n",
			info.fwVersion >> 13, (info.fwVersion & 0x1FFF) >> 5, info.fwVersion & 0x001F);

	num += snprintf(buf + num, PAGE_SIZE - num, "Customer Version: 0x%.2x\n", SiiDrvVirtualPageRead(&d->adapter, FW_CUSTOMER_SUB_VER));

	val = SiiDrvVirtualPageRead(&d->adapter, HDCP_CTRL);
	num += snprintf(buf + num, PAGE_SIZE - num, "DownStream HDCP: %s\n", (val & BIT5_MASK)?"Enable":"Disable");

	SiiDrvAdaptTxHdcpDsVerGet(&d->adapter, &version);
	num += snprintf(buf + num, PAGE_SIZE - num, "DownStream HDCP Version: ");
	switch (version) {
	case SII_DRV_ADAPTER_HDCP_VER__NOT_SUPPORTED:
		num += snprintf(buf + num, PAGE_SIZE - num, "NOT SUPPORTED\n");
		break;
	case SII_DRV_ADAPTER_HDCP_VER__1x:
		num += snprintf(buf + num, PAGE_SIZE - num, "1.x\n");
		break;
	case SII_DRV_ADAPTER_HDCP_VER__20:
		num += snprintf(buf + num, PAGE_SIZE - num, "2.0\n");
		break;
	case SII_DRV_ADAPTER_HDCP_VER__21:
		num += snprintf(buf + num, PAGE_SIZE - num, "2.1\n");
		break;
	case SII_DRV_ADAPTER_HDCP_VER__22:
		num += snprintf(buf + num, PAGE_SIZE - num, "2.2\n");
		break;
	default:
		num += snprintf(buf + num, PAGE_SIZE - num, "Unknown\n");
	}

	SiiDrvAdaptTxHdcpDsStatusGet(&d->adapter, &status);
	num += snprintf(buf + num, PAGE_SIZE - num, "DownStream HDCP Status: ");
	switch (status) {
	case SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF:
		num += snprintf(buf + num, PAGE_SIZE - num, "Off\n");
		break;
	case SII_DRV_ADAPTER_HDCP_TX_STATUS__SUCCESS:
		num += snprintf(buf + num, PAGE_SIZE - num, "Success\n");
		break;
	case SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTHENTICATING:
		num += snprintf(buf + num, PAGE_SIZE - num, "Authenticating\n");
		break;
	case SII_DRV_ADAPTER_HDCP_TX_STATUS__FAILED:
		num += snprintf(buf + num, PAGE_SIZE - num, "Done\n");
		break;
	default:
		break;
	}

	val = SiiDrvVirtualPageRead(&d->adapter, HDCP_STATE);
	num += snprintf(buf + num, PAGE_SIZE - num, "UpStream HDCP: ");
	switch (val & 0x3) {
	case 0:
		num += snprintf(buf + num, PAGE_SIZE - num, "NO\n");
		break;
	case 1:
		num += snprintf(buf + num, PAGE_SIZE - num, "1.x\n");
		break;
	case 2:
		num += snprintf(buf + num, PAGE_SIZE - num, "2.0\n");
		break;
	case 3:
		num += snprintf(buf + num, PAGE_SIZE - num, "2.2\n");
		break;
	default:
		break;
	}

#if 0
	num += snprintf(buf + num, PAGE_SIZE - num, "Resolution: %d x %d\n",
		(SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_H_ACTIVE_HIGH) << 8)
			| SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_H_ACTIVE_LOW),
		(SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_V_ACTIVE_HIGH) << 8)
			| SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_V_ACTIVE_LOW));
#endif
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}
static DEVICE_ATTR(info, S_IRUGO, show_info, NULL);

/* modify from SiiDrvAdaptRxEdidSramRead in  si_drv_adapter_rx.c */
static void SiiDrvAdaptRxEdidSramRead(SiiInst_t inst, uint16_t offset, uint8_t *poData, uint16_t length)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

	SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_H, (offset>>8));
	SiiDrvVirtualPageWrite(pDrvAdapt, EDID_OFFSET_L, (uint8_t)(offset&0x00FF));
	SiiDrvVirtualPageReadBlock(pDrvAdapt, EDID_FIFO, poData, length);
}

static ssize_t show_sram(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
#if (3 == CONFIG_AST1500_SOC_VER)
	if (ast_scu.board_info.board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
		int i;
		u8 data;

		for (i = 0; i < 256; i++) {
			SiiDrvAdaptRxEdidSramRead(&drv_data->adapter, i, &data, 1);
			num += snprintf(buf + num, PAGE_SIZE - num, " %.2x", data);
			if (3 == (i & 3))
				num += snprintf(buf + num, PAGE_SIZE - num, " |");
			if (0xF == (i & 0xF))
				num += snprintf(buf + num, PAGE_SIZE - num, "\n");
		}
	} else
#endif
		num += snprintf(buf + num, PAGE_SIZE - num, "Not supported!!\n");

	return num;
}
static DEVICE_ATTR(sram, S_IRUGO, show_sram, NULL);

static ssize_t show_fw(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	num += snprintf(buf + num, PAGE_SIZE - num, "Usage:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, " echo [CMD] > fw\n\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "CMD:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "    0 => firmware compare\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "    1 => firmware update\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "    2 => firmware erase\n");

	return num;
}

static ssize_t store_fw(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cmd;
	c = sscanf(buf, "%d", &cmd);

	if (1 > c) {
		printk("Usage:\n");
		printk(" echo [CMD] > fw\n\n");
		printk("CMD:\n");
		printk("    0 => firmware compare\n");
		printk("    1 => firmware update\n");
		printk("    2 => firmware erase\n");
	} else {
		switch (cmd) {
		case 0:
			if (0 == firmware_compare(drv_data))
				printk("Firmware compare pass\n");
			else
				printk("Firmware compare fail\n");
			break;
		case 1:
			printk("Start to update firmware, please wait...\n");
			firmware_update(drv_data, 1);
			printk("firmware update done\n");
			break;
		case 2:
			firmware_erase(drv_data);
			break;
		default:
			break;
		}
	}

	return count;
}
static DEVICE_ATTR(fw, (S_IRUGO | S_IWUSR), show_fw, store_fw);

static struct attribute *dev_attrs[] = {
	&dev_attr_reg.attr,
	&dev_attr_info.attr,
	&dev_attr_sram.attr,
	&dev_attr_fw.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

/* copy from si_drv_adapter_rx.h*/
typedef enum {
	SII_DRV_ADAPT_INP_CTRL__BYPASS,
	SII_DRV_ADAPT_INP_CTRL__DISABLE,
	SII_DRV_ADAPT_INP_CTRL__ENABLE,
} SiiDrvAdaptInpCtrl_t;

/* modify from SiiDrvAdaptHdcpDdcSet() in si_drv_adapter_rx.c */
/*
 * @brief RX DDC control, both HDCP and EDID DDC can be controlled by this function.
 *
 * Control HDCP&EDID DDC registers accessibility from upstream.
 *
 * @param[in]  inst  Adapter driver instance returned by @ref SiiDrvAdaptCreate()
 * @param[in]  ddc   RX DDC state
 *                     - @ref SII_DRV_ADAPT_INP_CTRL__DISABLE RX HDCP&EDID DDC is disabled
 *                     - @ref SII_DRV_ADAPT_INP_CTRL__ENABLE  RX HDCP&EDID DDC is enabled
 *                     - @ref SII_DRV_ADAPT_INP_CTRL__BYPASS  Currently not supported
 *
 */
static void SiiDrvAdaptHdcpDdcSet(SiiInst_t inst, SiiDrvAdaptInpCtrl_t ddc)
{
	DrvAdaptInst_t *pDrvAdapt = DRV_ADAPT_INST2PTR(inst);

	switch (ddc) {
	case SII_DRV_ADAPT_INP_CTRL__ENABLE:
		SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, RX_DDC_CTRL_MASK, RX_DDC_CTRL_ENABLE);
		break;
	case SII_DRV_ADAPT_INP_CTRL__DISABLE:
		SiiDrvVirtualRegModify(pDrvAdapt, HDCP_CTRL_1, RX_DDC_CTRL_MASK, RX_DDC_CTRL_DISABLE);
		break;
	default:
		uerr("unsupported option (%d)\n", ddc);
		break;
	}
}

static int drv_probe(struct platform_device *pdev)
{
	int ret = 0, fw_check = 1;
	struct sii_drv_data *d;
	dev_t	dev;

	uinfo("Hello probe!\n");

	/* HW reset */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_RX);
	msleep(1000);

	/* I2C init */
	I2CInit(I2C_HDCP22_RX, I2C_SPEED);

	d = kzalloc(sizeof(struct sii_drv_data), GFP_KERNEL);

	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}


	drv_data = d;

	sii_drv_init(d);

	if (!sii967x_exists(d)) {
		uinfo("%s device not exists?! Switch to debug mode\n", MODULE_NAME);
		d->adapter.config.i2cSlaveAddr = I2C_ADDR_DEBUG;

		switch_to_debug();

		if (!sii967x_exists(d)) {
			uerr("%s DONOT exist!!!!\n", MODULE_NAME);
			goto err2;
		}

		if (!(ast_scu.astparam.hdcp_cts_option & HDCP_CTS_DISABLE_AUTO_FW_UPDATE)) {
			firmware_update(d, 0);
		}
		d->adapter.config.i2cSlaveAddr = I2C_ADDR;
		fw_check = 0;
	}

#if 0
	if (sii_dev_init(d)) {
		uerr("Device init failed?!\n");
		ret = -ENOMEM;
		goto err2;
	}
#endif

	if (sii9679_major){
		dev = MKDEV(sii9679_major, 0);
		ret = register_chrdev_region(dev, MAX_SII9679_DEVS, MODULE_NAME);
	} else {
		ret = alloc_chrdev_region(&dev, 0, MAX_SII9679_DEVS, MODULE_NAME);
	}
	if (ret < 0) {
		uerr("Failed to allocate char dev for SiI9679?! (%i)\n", ret);
		goto err2;
	}
	sii9679_major = MAJOR(dev);
	uinfo("sii9679_major is %d\n", sii9679_major);

	cdev_init(sii9679_devs, &sii9679_fops);
	sii9679_devs->owner = THIS_MODULE;
	ret = cdev_add(sii9679_devs, dev, 1);
	if (ret < 0){
		uerr("Error %d adding sii9679 char dev\n", ret);
		return ret;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		goto out;
	}

	platform_set_drvdata(pdev, (void *)d);

	if (fw_check
		&& !(ast_scu.astparam.hdcp_cts_option & HDCP_CTS_DISABLE_AUTO_FW_UPDATE))
	{
#if defined(UPDATE_FW_IN_THREAD)
		INIT_WORK(&update_work, (void (*)(void *))(firmware_update), d);
		schedule_delayed_work(&update_work, 0);
#else
		firmware_update(d, 0);
#endif
	}

	drv_data = d;

#if (CONFIG_AST1500_SOC_VER >= 3)
	hook_init();
#endif

	if (ast_v_hdcp_param() & PARAM_DISABLE_HDCP)
		SiiDrvAdaptHdcpDdcSet(&d->adapter, SII_DRV_ADAPT_INP_CTRL__DISABLE);
	else
		SiiDrvAdaptHdcpDdcSet(&d->adapter, SII_DRV_ADAPT_INP_CTRL__ENABLE);

out:
	return ret;
err2:
	kfree(d);
err:
	return ret;
}

static int drv_remove(struct platform_device *pdev)
{
	struct sii_drv_data *d = platform_get_drvdata(pdev);

	if (!d)
		return 0;

#if (CONFIG_AST1500_SOC_VER >= 3)
	hook_fini();
#endif

	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);

	drv_data = NULL;

	kfree(d);

	return 0;
}

static struct platform_driver sii_driver = {
	.probe		= drv_probe,
	.remove		= __devexit_p(drv_remove),
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
		.name	= MODULE_NAME,
	},
};

static int __init drv_init(void)
{
	int ret;

	uinfo("%s driver based on Silicon Image SiI9679 Host SW %s\n",
		MODULE_NAME, REF_FW_VER);
	pdev = platform_device_register_simple(
					MODULE_NAME, /* driver name string */
					-1, /* id */
					NULL, /* struct resource to alloc */
					0);
	/* resource number */
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out;
	}

	ret = platform_driver_register(&sii_driver);

	if (ret == 0)
		goto out;

	platform_device_unregister(pdev);

out:
	uinfo("init done\n");
	return ret;
}

static void __exit drv_exit(void)
{
	platform_driver_unregister(&sii_driver);
	unregister_chrdev_region(MKDEV(sii9679_major, 0), 1);
	platform_device_unregister(pdev);
}

#ifndef MODULE
arch_initcall(drv_init);
#else
module_init(drv_init);
#endif
module_exit(drv_exit);
#endif
