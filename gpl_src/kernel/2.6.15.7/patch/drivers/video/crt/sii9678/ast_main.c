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

#if defined(CONFIG_AST1500_SII9678)
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <linux/cdev.h>
#include <asm/uaccess.h> // For copy_from_user()
#if defined(SUPPORT_HDCP_REPEATER)
#include <aspeed/hdcp1.h>
#endif
#include <asm/arch/drivers/crt.h>
#include "ast_main.h"
#include "ast_utilities.h"

#define MODULE_NAME	"sii9678"
#define REF_FW_VER      "1.00.07 SVN25378"
#define I2C_SPEED	100000 /* 100KHz bus speed */
#define CHIP_ID		0x9678
#define I2C_ADDR	0x60
#define I2C_ADDR_DEBUG	0x60

#define GPIO_HDCP22_TX_INT GPIO_HDCP22_TX_DEBUG_MODE
#define ENABLE_INT() gpio_cfg(GPIO_HDCP22_TX_INT, AST1500_GPIO_INT_LEVEL_LOW)
#define DISABLE_INT() \
do { \
	gpio_cfg(GPIO_HDCP22_TX_INT, AST1500_GPIO_INT_DISABLE); \
	gpio_ack_int_stat(GPIO_HDCP22_TX_INT); \
} while (0)

MODULE_LICENSE ("GPL");

static struct platform_device *pdev;
static struct sii_drv_data *drv_data;

#define MAX_SII9678_DEVS 1
static struct cdev sii9678_devs[MAX_SII9678_DEVS];
static int sii9678_major = 0;

#if SUPPORT_HDCP_REPEATER
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif

static void firmware_update(struct sii_drv_data *d, u32 force);
static unsigned char firmware_file[] = {
#include "ast_firmware.h"
};

#ifdef FW_TEST_CODE /* It is useless so far. */
static unsigned char firmware_file_cts_hdcp22[] = {
#include "ast_firmware_cts_hdcp22.h"
};

static unsigned char firmware_file_cts_hdcp14[] = {
#include "ast_firmware_cts_hdcp14.h"
};
#endif

static unsigned char *fw_tmp = 0;
static unsigned int fw_tmp_max = 0;
static unsigned int fw_tmp_size = 0;
static int sii9678_fop_open(struct inode *inode, struct file *filp)
{
	/* FIXME. We use firmware_file directly as buffer and assume buffer size is enough. */
	/* FIXME. We assume there is only one user access this dev node at a time. */
	fw_tmp_size = 0;
	fw_tmp_max = ARRAY_SIZE(firmware_file);
	fw_tmp = firmware_file;
	return 0;
}

static int sii9678_fop_release(struct inode *inode, struct file *filp)
{
	if (fw_tmp_size) {
		uinfo("Start programming fw into SPI flash.\n");
		firmware_update(drv_data, 1);
	}

	return 0;
}

static ssize_t sii9678_fop_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
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
		memset(firmware_file, 0, fw_tmp_max);
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

static ssize_t sii9678_fop_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
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

static struct file_operations sii9678_fops = {
//	.owner   = THIS_MODULE,
	.open    = sii9678_fop_open,
	.release = sii9678_fop_release,
	.read = sii9678_fop_read,
	.write = sii9678_fop_write,
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

static void sii_drv_init(struct sii_drv_data *d)
{
	DrvAdaptInst_t *adapter;
	SiiDrvAdaptConfig_t config;

	adapter = &d->adapter;

	adapter->instCra = I2C_HDCP22_TX;
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

	/* FIXME following commands should sync with SPI flash datasheet */
	config.spiConfig.eraseCmd = 0xC7; /* chip erase or sector erase ?*/
	config.spiConfig.writeEnCmd = 0x06;
	config.spiConfig.pageWrCmd = 0x02;

	SiiDrvAdaptConfigure(adapter, &config);

	SiiDrvAdaptStart(adapter);
}

#if 0
static u32 sii_dev_init(struct sii_drv_data *d)
{
	SiiDrvAdaptBootStatus_t status;
	u32 ret = 1, retry = 0;

check_status:
	SiiDrvAdaptBootStatusGet(&d->adapter, &status);

	switch (status) {
	case SII_DRV_ADAPTER_BOOTING__SUCCESS:
		ret = 0;
		break;
	case SII_DRV_ADAPTER_BOOTING__IN_PROGRESS:
		retry++;
		if (64 > retry)
			goto check_status;
		break;
	case SII_DRV_ADAPTER_BOOTING__FAILURE:
		break;
	default:
		break;
	}

	return ret;
}
#endif


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

#if defined(CONFIG_ARCH_AST1500_HOST)
static u32 sii9678_on_host(u32 board_revision)
{
	int ret = 0;

#if (CONFIG_AST1500_SOC_VER == 3)
	if (board_revision & BOARD_REV_PATCH_VIDEO_SPLITTER) {
		switch (BOARD_REV_PATCH_MINOR_VERSION(board_revision)) {
		case 0:
			ret = 1;
			break;
		default:
			break;
		}
	}
#endif
	return ret;
}
#endif

static inline void hw_init(void)
{
#if defined(CONFIG_ARCH_AST1500_HOST)
	if (sii9678_on_host(ast_scu.board_info.board_revision)) {
		/* HW reset */
		ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_TX);
		msleep(1000);
	}
#else
	/* Let HDCP transmitter handle reset operation instead */
#endif
	/* I2C init */
	I2CInit(I2C_HDCP22_TX, I2C_SPEED);
}

static inline void switch_to_debug(void)
{
	/* set INT to LOW and reset =>  debug mode */
	gpio_direction_output(GPIO_HDCP22_TX_DEBUG_MODE, 0); /* FIXME */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_TX);
}

static inline void switch_to_normal(void)
{
	/* set INT to HIGH and reset => normal mode */
	gpio_direction_output(GPIO_HDCP22_TX_DEBUG_MODE, 1); /* FIXME */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_TX);

#if defined(CONFIG_ARCH_AST1500_HOST)
	if (sii9678_on_host(ast_scu.board_info.board_revision)) {
		/*
		 * with splitter, we need interrupt to notify downstream link status
		 * change gpio direction to input
		 */
		gpio_direction_input(GPIO_HDCP22_TX_DEBUG_MODE);
	}
#endif
}

u8 data[SPI_MAX_PAGE_SIZE];

static int firmware_compare(struct sii_drv_data *d)
{
	u8 *firmware;
	u32 size, bytes_to_work, done;
	u32 result = 0, i;

	firmware = firmware_file;
	size = ARRAY_SIZE(firmware_file);

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
	static u8 *firmware;

#ifdef FW_TEST_CODE /* It is useless so far. */
	if (ast_scu.board_info.hdcp_cts_option & HDCP_CTS_HDCP22_ONLY) {
		uinfo("Use HDCP 2.2 Only FW\n");
		firmware = firmware_file_cts_hdcp22;
	} else if (ast_scu.board_info.hdcp_cts_option & HDCP_CTS_HDCP14_ONLY) {
		uinfo("Use HDCP 1.x Only FW\n");
		firmware = firmware_file_cts_hdcp14;
	} else {
		firmware = firmware_file;
	}
#else
	firmware = firmware_file;
#endif

	if (firmware_check(d, firmware) || force) {
		SiiDrvAdaptFwBuffer_t fw;

		uinfo("Start updating firmware.");
		fw.pBuffer = firmware;
		fw.ucBufferSize = (ARRAY_SIZE(firmware_file) + 0xFF) & 0xFFFFFF00;
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
	end = 0xff;

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
	num += snprintf(buf + num, PAGE_SIZE - num, "Customer Version: 0x%.2x\n",
		SiiDrvVirtualPageRead(&d->adapter, FW_CUSTOMER_SUB_VER));

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

	num += snprintf(buf + num, PAGE_SIZE - num, "Resolution: %d x %d\n",
		(SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_H_ACTIVE_HIGH) << 8)
			| SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_H_ACTIVE_LOW),
		(SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_V_ACTIVE_HIGH) << 8)
			| SiiDrvVirtualPageRead(&d->adapter, RX_RESOLUTION_V_ACTIVE_LOW));

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}
static DEVICE_ATTR(info, S_IRUGO, show_info, NULL);

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


static ssize_t show_edid_sram(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0, i;
	char data[256];

	num += snprintf(buf + num, PAGE_SIZE - num, "SRAM (EDID):");

	SiiDrvAdaptTxDsEdidReRead(&drv_data->adapter);
	SiiDrvAdaptRxEdidSramRead(&drv_data->adapter, 0, data, 256);

	for (i = 0; i < 256; i++) {
		if ((i & 0x0F) == 0)
			num += snprintf(buf + num, PAGE_SIZE - num, "\n");

		num += snprintf(buf + num, PAGE_SIZE - num, " %.2x", data[i]);
	}
	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	return num;
}
static DEVICE_ATTR(edid_sram, S_IRUGO, show_edid_sram, NULL);

static ssize_t store_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c;

	if (c > 1)
		ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDCP22_TX);

	return count;
}
static DEVICE_ATTR(reset, S_IWUSR, NULL, store_reset);

static struct attribute *dev_attrs[] = {
	&dev_attr_reg.attr,
	&dev_attr_info.attr,
	&dev_attr_fw.attr,
	&dev_attr_edid_sram.attr,
	&dev_attr_reset.attr,

	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

#if SUPPORT_HDCP_REPEATER

#define HDCP_RETRY_MAX 20
static void sii_hdcp1_downstream_port_auth(int enable)
{
	uinfo("HDCP %s\n", enable?"ON":"OFF");

	if (enable) {
		SiiDrvAdaptHdcpTxStatus_t status;
		int authenticated = 0, retry = 0;

		if (SII_DRV_ADAPTER_HDCP_PROTECT__ON != SiiDrvAdaptTxHdcpProtectionGet(&drv_data->adapter))
			SiiDrvAdaptTxHdcpProtectionSet(&drv_data->adapter, SII_DRV_ADAPTER_HDCP_PROTECT__ON);

		do {
			msleep(50);
			SiiDrvAdaptTxHdcpDsStatusGet(&drv_data->adapter, &status);
			retry++;
			if (retry > HDCP_RETRY_MAX)
				break;
		 } while ((SII_DRV_ADAPTER_HDCP_TX_STATUS__OFF == status)
			|| (SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTHENTICATING == status));

		uinfo("HDCP Status %d\n", status);

		if (SII_DRV_ADAPTER_HDCP_TX_STATUS__SUCCESS == status) {
			SiiDrvAdaptHdcpVer_t version;

			SiiDrvAdaptTxHdcpDsVerGet(&drv_data->adapter, &version);

			switch (version) { /* 1.x and 2.2 only */
			case SII_DRV_ADAPTER_HDCP_VER__1x:
				if (HDCP_V_1X == enable) {
					authenticated = 1;
					uinfo("H1\n");
				}
				break;
			case SII_DRV_ADAPTER_HDCP_VER__22:
				if (HDCP_V_22 == enable) {
					authenticated = 1;
					uinfo("H2\n");
				} else if (HDCP_V_1X == enable) {
					/* special case:
					 * video source is HDCP 1.x
					 * the HDCP connection betweenient and video sink is HDCP 2.2
					 */
					authenticated = 1;
					uinfo("H2\n");
				}
				break;
			default:
				break;
			}
			if (!authenticated)
				uinfo("H0\n");

#if defined(CONFIG_ARCH_AST1500_HOST)
			/* host loopback */
			if (!authenticated)
				SiiDrvVirtualRegModify(&drv_data->adapter, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_AV_MUTE_ON);
			else
				SiiDrvVirtualRegModify(&drv_data->adapter, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_AV_MUTE_OFF);
#endif
		} else if (SII_DRV_ADAPTER_HDCP_TX_STATUS__AUTHENTICATING == status) {
			if (!(TX_RXSENSED & SiiDrvVirtualPageRead(&drv_data->adapter, TX_GENERAL_STAT)))
				return;

			/* retry next time */
			uinfo("H?\n");
			authenticated = 2;
#if 0
			/*
			** Bruce151211. From my test, SiI9678 can recover by itself.
			** Turn off/on HDCP may cause HDCP 2.2 CTS fail.(??)
			*/
			if (current_protection != SII_DRV_ADAPTER_HDCP_PROTECT__OFF) {
				SiiDrvAdaptTxHdcpProtectionSet(&drv_data->adapter, SII_DRV_ADAPTER_HDCP_PROTECT__OFF);
				current_protection = SII_DRV_ADAPTER_HDCP_PROTECT__OFF;
			}
#endif
		}
		/* TODO non-legacy mode */
		CRT_HDCP1_Downstream_Port_Auth_Status_Callback(authenticated, NULL, 0, 0, NULL);
	} else {
		if (SII_DRV_ADAPTER_HDCP_PROTECT__OFF != SiiDrvAdaptTxHdcpProtectionGet(&drv_data->adapter))
			SiiDrvAdaptTxHdcpProtectionSet(&drv_data->adapter, SII_DRV_ADAPTER_HDCP_PROTECT__OFF);
#if defined(CONFIG_ARCH_AST1500_HOST)
		/* host loopback */
		SiiDrvVirtualRegModify(&drv_data->adapter, AV_CTRL, AV_CTRL_MUTE_MASK, AV_CTRL_AV_MUTE_OFF);
#endif
	}
}
#endif

extern struct s_xmiter_info xmiter_info[];

#if defined(CONFIG_ARCH_AST1500_HOST)
static unsigned int sii_exist(struct s_crt_drv *crt)
{
	return 1;
}

static unsigned int sii_setup_video(struct s_crt_drv *crt, struct s_crt_info *info)
{
	CRT_HDCP1_Downstream_Port_Auth_Status_Callback(2, NULL, 0, 0, NULL);
	return 0;
}

static unsigned int sii_check_hotplug(struct s_crt_drv *crt)
{
	if (TX_LINK_TYPE_DISCONN == SiiDrvVirtualPageRead(&drv_data->adapter, TX_GENERAL_STAT)) {
		printk("sink not plugged yet\n");
		return 0;
	}
	return 1;
}

static u32 edid_valid = 0;
static unsigned char edid[EDID_ROM_SIZE];

static unsigned int sii_rd_edid_chksum(struct s_crt_drv *crt, unsigned char *pChksum)
{
	DrvAdaptInst_t *padapter;
	padapter = &drv_data->adapter;

	if (TX_LINK_TYPE_DISCONN == SiiDrvVirtualPageRead(padapter, TX_GENERAL_STAT)) {
		printk("sink not plugged yet\n");
		return 2;
	}

	if (edid_valid)
		*pChksum = edid[EDID_CHECKSUM_OFFSET];
	else {
		SiiDrvVirtualPageWrite(padapter, EDID_OFFSET_H, EDID_CHECKSUM_OFFSET >> 8);
		SiiDrvVirtualPageWrite(padapter, EDID_OFFSET_L, EDID_CHECKSUM_OFFSET & 0x00FF);
		SiiDrvVirtualPageReadBlock(padapter, EDID_FIFO, pChksum, 1);
	}

	return 0;
}

static unsigned int sii_rd_edid(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	DrvAdaptInst_t *padapter;
	int i, j;
	padapter = &drv_data->adapter;

	if (1 < blk_num) {
		printk("blk_num(%d) not supported\n", blk_num);
		return 1;
	}

	if (TX_LINK_TYPE_DISCONN == SiiDrvVirtualPageRead(padapter, TX_GENERAL_STAT)) {
		printk("sink not plugged yet\n");
		return 2;
	}

	i = blk_num * 128;
	j = i + 128;

	for (; i < j; i++) {
		SiiDrvVirtualPageWrite(padapter, EDID_OFFSET_H, i >> 8);
		SiiDrvVirtualPageWrite(padapter, EDID_OFFSET_L, i & 0x00FF);
		SiiDrvVirtualPageReadBlock(padapter, EDID_FIFO, edid + i, 1);
	}

	memcpy(pEdid, edid + (blk_num * 128), 128);
	edid_valid = 1;

	return 0;
}
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */

static void xmiter_info_init(void)
{
	struct s_xmiter_info *pinfo;

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#if SUPPORT_HDCP_REPEATER
	if (pinfo->Hdcp1_set_mode) {
		pinfo->Hdcp1_set_mode(0);
		pinfo->Hdcp1_set_mode = NULL;
	}

	if (pinfo->Hdcp1_auth) {
		pinfo->Hdcp1_auth(0);
		pinfo->Hdcp1_auth = sii_hdcp1_downstream_port_auth;
	}

	if (pinfo->Hdcp1_encrypt) {
		pinfo->Hdcp1_encrypt(0);
		pinfo->Hdcp1_encrypt = NULL;
	}
	pinfo->cap |= xCap_EXTERNAL_HDCP;
#endif
#else /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
	/* HOST */
	if (sii9678_on_host(ast_scu.board_info.board_revision)) {

		if (VT_DISABLE == ast_scu.board_info.video_loopback_type) {
			SiiDrvVirtualRegModify(&drv_data->adapter, AV_CTRL, TMDS_OUTPUT_CTRL_MASK, TMDS_OUTPUT_DISABLE);
			return;
		}
		pinfo = xmiter_info + XIDX_HOST_D;

		pinfo->chip             = xmiter_dummy;
		pinfo->cap              = (xCap_DVI | xCap_HDMI);
		pinfo->exist            = sii_exist;
		pinfo->setup            = sii_setup_video;
		pinfo->setup_audio      = NULL;
		pinfo->disable          = NULL;
		pinfo->disable_audio    = NULL;
		pinfo->init             = NULL;
		pinfo->check_hotplug    = sii_check_hotplug;
		pinfo->rd_edid_chksum   = sii_rd_edid_chksum;
		pinfo->rd_edid          = sii_rd_edid;
		pinfo->poll_hotplug     = NULL;
#if SUPPORT_HDCP_REPEATER
		pinfo->Hdcp1_set_mode   = NULL;
		pinfo->Hdcp1_auth       = sii_hdcp1_downstream_port_auth;
		pinfo->Hdcp1_encrypt    = NULL;
#endif
	}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
}

static void xmiter_info_fini(void)
{
	struct s_xmiter_info *pinfo;

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	pinfo = xmiter_info + XIDX_CLIENT_D;
#if SUPPORT_HDCP_REPEATER
	pinfo->Hdcp1_set_mode = NULL;
	pinfo->Hdcp1_auth = NULL;
	pinfo->Hdcp1_encrypt = NULL;
	pinfo->cap &= ~xCap_EXTERNAL_HDCP;
#endif
#else /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
	/* Host */
	if (sii9678_on_host(ast_scu.board_info.board_revision)) {
		pinfo = xmiter_info + XIDX_HOST_D;
		pinfo->chip             = xmiter_none;
		pinfo->cap              = xCap_NONE;
		pinfo->exist            = NULL;
		pinfo->setup            = NULL;
		pinfo->check_hotplug    = NULL;
		pinfo->rd_edid_chksum   = NULL;
		pinfo->rd_edid          = NULL;
#if SUPPORT_HDCP_REPEATER
		pinfo->Hdcp1_auth       = NULL;
#endif
	}
#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
}


#if defined(CONFIG_ARCH_AST1500_HOST)
static irqreturn_t sii9678_intr(int irq, void *dev_id, struct pt_regs *regs)
{
	/* check share interrupt */
	if (!gpio_get_int_stat(GPIO_HDCP22_TX_INT))
		return IRQ_NONE;

	DISABLE_INT();

	/* ack interrupt */
	gpio_ack_int_stat(GPIO_HDCP22_TX_INT);

	/* IRQ Handle */
	schedule_work(&drv_data->intr_work);

	return IRQ_HANDLED;
}

static void sii_intr(struct sii_drv_data *d)
{
	u8 val;

	val = SiiDrvVirtualPageRead(&d->adapter, TX_GENERAL_INT);

	SiiDrvVirtualPageWrite(&d->adapter, TX_GENERAL_INT, val);

	if (TX_RXSENSE_CHG_MASK & val) {
		if (TX_RXSENSED & SiiDrvVirtualPageRead(&d->adapter, TX_GENERAL_STAT))
			crt_hotplug_tx(1);
		else
			crt_hotplug_tx(0);
	}

	ENABLE_INT();
}

static int intr_init(struct sii_drv_data *d)
{
	/* mask all interrupts except TX_RXSENSE_CHG */
	SiiDrvVirtualPageWrite(&drv_data->adapter, CHIP_GENERAL_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, AUDIO_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, TX_GENERAL_INT_MASK, TX_RXSENSE_CHG_MASK);
	SiiDrvVirtualPageWrite(&drv_data->adapter, RX_GENERAL_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, HDCP_RX_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, HDCP_TX_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, CBUS_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, MHL3_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, CEC_INT_MASK, 0);
	SiiDrvVirtualPageWrite(&drv_data->adapter, EDID_INT_MASK, 0);

	INIT_WORK(&d->intr_work, (void (*)(void *))(sii_intr), d);

	/* request IRQ */
	if (request_irq(INT_GPIO, &sii9678_intr, SA_SHIRQ, MODULE_NAME, MODULE_NAME))
		return -1;

	/* enable interrupt after registering ISR */
	ENABLE_INT();

	return 0;
}

static void intr_fini(void)
{
	DISABLE_INT();

	flush_scheduled_work();

	free_irq(INT_GPIO, MODULE_NAME);
}
#endif /* #if defined(CONFIG_ARCH_AST1500_HOST) */

#if defined(UPDATE_FW_IN_THREAD)
struct work_struct update_work;
#endif

static int drv_probe(struct platform_device *pdev)
{
	int ret = 0, fw_check = 1;
	struct sii_drv_data *d;
	dev_t	dev;

	uinfo("Hello probe!\n");

	/* HW init */
	hw_init();

	d = kzalloc(sizeof(struct sii_drv_data), GFP_KERNEL);

	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}

	sii_drv_init(d);

	if (!sii967x_exists(d)) {
		uinfo("%s device not exists?! Switch to debug mode\n", MODULE_NAME);

		switch_to_debug();

		if (!sii967x_exists(d)) {
			uerr("%s DONOT exist!!!!\n", MODULE_NAME);
			goto err2;
		}

		if (!(ast_scu.astparam.hdcp_cts_option & HDCP_CTS_DISABLE_AUTO_FW_UPDATE)) {
			firmware_update(d, 0);
		}
		fw_check = 0;
	}

#if 0
	if (sii_dev_init(d)) {
		uerr("Device init failed?!\n");
		ret = -ENOMEM;
		goto err2;
	}
#endif

	if (sii9678_major){
		dev = MKDEV(sii9678_major, 0);
		ret = register_chrdev_region(dev, MAX_SII9678_DEVS, MODULE_NAME);
	} else {
		ret = alloc_chrdev_region(&dev, 0, MAX_SII9678_DEVS, MODULE_NAME);
	}
	if (ret < 0) {
		uerr("Failed to allocate char dev for SiI9678?! (%i)\n", ret);
		goto err2;
	}
	sii9678_major = MAJOR(dev);
	uinfo("sii9678_major is %d\n", sii9678_major);

	cdev_init(sii9678_devs, &sii9678_fops);
	sii9678_devs->owner = THIS_MODULE;
	ret = cdev_add(sii9678_devs, dev, 1);
	if (ret < 0){
		uerr("Error %d adding sii9678 char dev\n", ret);
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

	xmiter_info_init();

#if defined(CONFIG_ARCH_AST1500_HOST)
	if (sii9678_on_host(ast_scu.board_info.board_revision))
		intr_init(d);
#endif
out:
	return ret;
err2:
	kfree(d);
err:
	return ret;
}

static int __devexit drv_remove(struct platform_device *pdev)
{
	struct sii_drv_data *d = platform_get_drvdata(pdev);

	if (!d)
		return 0;

	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);

#if defined(CONFIG_ARCH_AST1500_HOST)
	if (sii9678_on_host(ast_scu.board_info.board_revision))
		intr_fini();
#endif

	drv_data = NULL;

	xmiter_info_fini();

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

	uinfo("%s driver based on Silicon Image SiI9678 Host SW %s\n",
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
	unregister_chrdev_region(MKDEV(sii9678_major, 0), 1);
	platform_device_unregister(pdev);
}

#ifndef MODULE
arch_initcall(drv_init);
#else
module_init(drv_init);
#endif
module_exit(drv_exit);
#endif
