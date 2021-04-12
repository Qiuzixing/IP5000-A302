/*
** Copyright (c) 2004-2014
** ASPEED Technology Inc. All Rights Reserved
** Proprietary and Confidential
**
** By using this code you acknowledge that you have signed and accepted
** the terms of the ASPEED SDK license agreement.
**
*/
#include "ast_main.h"

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#if (INTR_MODE == 1)
#include <linux/interrupt.h>
#endif
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/video_hal.h>

#include "ast_utilities.h"
#include "ast_wrap.h"

#define MODULE_NAME "it6802"

MODULE_LICENSE ("GPL");

int drv_param = 0;
module_param(drv_param, int, S_IRUGO);

#define I2C_IT680X_SPEED 100000 //100KHz bus speed
#define LOOP_MSEC 150
#define LOOP_MSEC 150

struct platform_device *pdev;

static void it680x_fsm_timer(struct it680x_drv_data *d)
{
	ast_fsm_timer_handler(d);

	if (d->wq)
		queue_delayed_work(d->wq, &d->timerwork, msecs_to_jiffies(LOOP_MSEC));
}

#if (INTR_MODE == 1)
static void it680x_intr_handler(struct it680x_drv_data *d)
{
	ast_intr_handler(d);

	gpio_cfg(GPIO_IT6802_INT, AST1500_GPIO_INT_LEVEL_LOW); /* enable interrupt */
}
#endif

#ifdef Enable_IT6802_CEC
static int _cec_poll = 1;
#define CEC_PA_MSEC (10 * 1000)
static void it680x_cec_pa_timer(struct it680x_drv_data *d)
{
	if (_cec_poll)
		switch_req_pa_state(s_req_pa_init);

	if (d->wq)
		queue_delayed_work(d->wq, &d->cec_pa_work, msecs_to_jiffies(CEC_PA_MSEC));
}
#endif

static int create_works_and_thread(struct it680x_drv_data *d)
{
	struct workqueue_struct *wq;

	/* timer handler init. */
	wq = create_singlethread_workqueue("it680x_wq");
	if (!wq) {
		uerr("Failed to allocate wq?!\n");
		goto err;
	}

	d->wq = wq;

	INIT_WORK(&d->timerwork, (void (*)(void *))it680x_fsm_timer, d);
	INIT_WORK(&d->audio_update_work, (void (*)(void *))NULL, d);
#ifdef Enable_IT6802_CEC
	INIT_WORK(&d->cec_pa_work, (void (*)(void *))it680x_cec_pa_timer, d);
#endif
	INIT_WORK(&d->cec_work, (void (*)(void *))it680x_intr_handler, d);

	return 0;
err:
	return -ENOMEM;
}

static int destroy_works_and_thread(struct it680x_drv_data *d)
{
	struct workqueue_struct *wq = d->wq;

	if (wq) {
		d->wq = NULL;
		cancel_delayed_work(&d->timerwork);
#ifdef Enable_IT6802_CEC
		cancel_delayed_work(&d->cec_pa_work);
#endif
		flush_workqueue(wq);
		destroy_workqueue(wq);
	}

	return 0;
}

static void _default_audio_event_callback(Audio_Info audio_info)
{
	uinfo("No one interests in audio events.\n");
}

static void _default_infoframe_callback(e_vrxhal_infoframe_type type)
{
	uinfo("No one interests in audio events.\n");
}

void ast_drv_init(struct it680x_drv_data *d)
{
	init_MUTEX(&d->reg_lock);
#if 0
	d->video_event_callback = NULL;
	d->infoframe_callback = _default_infoframe_callback;
	d->audio_event_callback = _default_audio_event_callback;
#endif
	/* force sample size as same as CAT6023 */
	d->audio_info.SampleSize = 24; /* */
	d->dev_exist = 0;
}

#if 0
void vrxhal_reg_video_event_callback(PFN_UPDATE_VIDEO_INFO cb)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	BUG_ON(d == NULL);
	d->video_event_callback = cb;

	ast_notify_video_state_change(d);
}
EXPORT_SYMBOL(vrxhal_reg_video_event_callback);

void vrxhal_dereg_video_event_callback(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	d->video_event_callback = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_video_event_callback);


void vrxhal_reg_infoframe_callback(PFN_UPDATE_INFOFRAME cb)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	d->infoframe_callback = cb;
}
EXPORT_SYMBOL(vrxhal_reg_infoframe_callback);

void vrxhal_dereg_infoframe_callback(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	d->infoframe_callback = NULL;
}
EXPORT_SYMBOL(vrxhal_dereg_infoframe_callback);

void vrxhal_reg_audio_event_callback(PFN_UPDATE_AUDIO_INFO cb)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	uinfo("Register audio event callback\n");
	BUG_ON(d == NULL);
	d->audio_event_callback = cb;

	if (d->it6802data)
		ast_schedule_audio_info_update(d, 1);
}
EXPORT_SYMBOL(vrxhal_reg_audio_event_callback);

void vrxhal_dereg_audio_event_callback(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	uinfo("Deregister audio event callback\n");

	d->audio_event_callback = _default_audio_event_callback;
}
EXPORT_SYMBOL(vrxhal_dereg_audio_event_callback);
#endif

static void sync_audio(int force_update)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	ast_schedule_audio_info_update(d, force_update);
}

static int video_timing_info(void *mode_info)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	return ast_get_video_timing_info(d, mode_info);
}

#if defined(EXTERNAL_HDCP_CHIP)
int (*ast_v_hdcp_status)(void);
EXPORT_SYMBOL(ast_v_hdcp_status);
int (*ast_v_hdcp_version)(void);
EXPORT_SYMBOL(ast_v_hdcp_version);

int ast_v_hdcp_param(void)
{
	return drv_param;
}
EXPORT_SYMBOL(ast_v_hdcp_param);
#endif

static int video_avi_info(unsigned char *pData)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	return ast_get_avi_info(d, pData);
}


static int video_vsd_info(unsigned char *pData)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	return ast_get_vsd_info(d, pData);
}

static int video_hdr_info(unsigned char *pData)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	return ast_get_hdr_info(d, pData);
}

static void video_update(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	BUG_ON(d == NULL);

	ast_notify_video_state_change(d);
}

static void audio_update(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	BUG_ON(d == NULL);

	if (d->it6802data)
		ast_schedule_audio_info_update(d, 1);
}

#ifdef Enable_IT6802_CEC
extern void it680x_cec_tx_en_q(unsigned char *buf, unsigned int size);
extern CECDevice CECList[15];

static int cec_send(unsigned char *buf, unsigned int size)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	it680x_cec_tx_en_q(buf, size);
	queue_work(d->wq, &d->cec_work);

	return 0;
}

static int cec_topology(unsigned char *buf)
{
	u16 *pdata = (u16 *) buf;
	u32 i;

	memset(pdata, 0, 16 * 2);

	for (i = 1; i < 15; i++) {
		if (CECList[i].Active) {
			pdata[0] |= (0x1 << i);
			pdata[i] = (CECList[i].PhyicalAddr1 << 8 | CECList[i].PhyicalAddr2);
		}
	}

#if 0
	{

		pdata[0] |= ((0x1 << 1)
				| (0x1 << 3)
				| (0x1 << 4)
				| (0x1 << 5)
				| (0x1 << 6)
				| (0x1 << 8)
			);
		pdata[1] = 0x1000;
		pdata[3] = 0x2000;
		pdata[4] = 0x1100;
		pdata[5] = 0x2100;
		pdata[6] = 0x1110;
		pdata[8] = 0x1120;

	}
#endif

	return 0;
}
#endif /* #ifdef Enable_IT6802_CEC */

static void hdcp_mode(unsigned char repeater)
{
	//BruceToDo
}

static void hdcp_status(unsigned int state, unsigned short Bstatus, void *KSV_list)
{
	//BruceToDo
}


static int hdmi_n_cts(unsigned int *pn, unsigned int *pcts)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	if (d)
		ast_get_hdmi_n_cts(d, pn, pcts);
	else
		*pn = *pcts = 0;

	return 0;
}

void vrxhal_HDCP1_reg_upstream_port_auth_request_callback(pfn_HDCP1_Upstream_Port_Auth_Request_Callback cb)
{
	//BruceToDo
	//pHDCP1_Upstream_Port_Auth_Request_Callback = pcallback;
}
EXPORT_SYMBOL(vrxhal_HDCP1_reg_upstream_port_auth_request_callback);

void vrxhal_HDCP1_dereg_upstream_port_auth_request_callback(void)
{
	//BruceToDo
	//pHDCP1_Upstream_Port_Auth_Request_Callback = NULL;
}
EXPORT_SYMBOL(vrxhal_HDCP1_dereg_upstream_port_auth_request_callback);


static int dev_exist(void)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	return d->dev_exist;
}

static u32 register_bank = 0;

static ssize_t show_registers(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct it680x_drv_data *d = dev_get_drvdata(dev);
	u32 start, end;
	int i, num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%s register dump:\n", MODULE_NAME);

	/* FIXME: Move register access to ast_wrap.c */

	switch (register_bank) {
	case 0:
		/* REG_RX_000 ~ REG_RX_0FF */
		start = 0x00; end = 0xFF;
		break;
	case 1:
		/* REG_RX_100 ~ REG_RX_1F6 */
		start = 0x00; end = 0x1F6;
		break;
	case 2:
		/* REG_RX_200 ~ REG_RX_2FF */
		start = 0x00; end = 0xFF;
		break;
	default:
		num += snprintf(buf + num, PAGE_SIZE - num, "Unsupported bank: %d\n", register_bank);
		return num;
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "bank %d\n", register_bank);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n        ");
	for (i = 0; i <= 0xF; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", i);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	down(&d->reg_lock);
	chgbank(register_bank);
	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
			num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.3x: ", i);
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", hdmirxrd(i));
	}

	chgbank(0);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	up(&d->reg_lock);
	return num;
}

#define REG_BANK_MAX 2
static ssize_t store_registers(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct it680x_drv_data *d = dev_get_drvdata(dev);
	u32 offset, value, c;

	c = sscanf(buf, "%x %x", &offset, &value);

	/* FIXME: Move register access to ast_wrap.c */
	down(&d->reg_lock);

	if (2 == c) {
		chgbank(register_bank);
		hdmirxwr(offset, value);
		chgbank(0);
		printk("Set HDMI Rx bank%d register 0x%2x to 0x%02x.\n", register_bank, offset, value);
	} else {
		printk("Usage:\nOffset [Value]\n");
	}

	up(&d->reg_lock);

	return count;
}
static DEVICE_ATTR(registers, (S_IRUGO | S_IWUSR), show_registers, store_registers);

static ssize_t show_registers_bank(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "%s register bank: %d\n", MODULE_NAME, register_bank);

	return num;
}

static ssize_t store_registers_bank(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 bank, c;

	c = sscanf(buf, "%d", &bank);

	/* FIXME: Move register access to ast_wrap.c */

	if (1 == c) {
		if (REG_BANK_MAX >= bank)
			register_bank = bank;
		else
			printk("Usage:\nBank\nrange:0~2\n");
	} else
		printk("Usage:\nBank\nrange:0~2\n");

	return count;
}
static DEVICE_ATTR(registers_bank, (S_IRUGO | S_IWUSR), show_registers_bank, store_registers_bank);

static ssize_t show_cec(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct it680x_drv_data *d = dev_get_drvdata(dev);
	u32 start, end;
	int i, num = 0;

	start = 0x00; end = 0x4c;
	num += snprintf(buf + num, PAGE_SIZE - num, "%s CEC register dump:\n", MODULE_NAME);

	num += snprintf(buf + num, PAGE_SIZE - num, "\n        ");
	for (i = 0; i <= 0xF; i++)
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", i);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	down(&d->reg_lock);
	for (i = start; i <= end; i++) {
		if (0 == (i & 0xF))
			num += snprintf(buf + num, PAGE_SIZE - num, "\n 0x%.3x: ", i);
		num += snprintf(buf + num, PAGE_SIZE - num, " 0x%.2x", IT6802_CEC_ReadI2C_Byte(i));
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "\n\n");

	up(&d->reg_lock);
	return num;
}

static ssize_t store_cec(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct it680x_drv_data *d = dev_get_drvdata(dev);
	u32 offset, value, c;

	c = sscanf(buf, "%x %x", &offset, &value);

	/* FIXME: Move register access to ast_wrap.c */
	down(&d->reg_lock);

	if (2 == c) {
		IT6802_CEC_WriteI2C_Byte(offset, value);
		printk("Set HDMI Rx CEC register 0x%2x to 0x%02x.\n", offset, value);
	} else {
		printk("Usage:\nOffset [Value]\n");
	}

	up(&d->reg_lock);

	return count;
}
static DEVICE_ATTR(cec, (S_IRUGO | S_IWUSR), show_cec, store_cec);


static ssize_t show_LoopbackEnable(struct device *dev, struct device_attribute *attr, char *buf)
{
	//BruceToDo
	return 0;
}

static ssize_t store_LoopbackEnable(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	//BruceToDo
	return count;
}
static DEVICE_ATTR(LoopbackEnable, (S_IRUGO | S_IWUSR), show_LoopbackEnable, store_LoopbackEnable);

static ssize_t show_csc_bypass(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "csc_bypass %d\n", csc_bypass_status());

	return num;
}

static ssize_t store_csc_bypass(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (1 <= c)
		csc_bypass_cfg(cfg);

	return count;
}
static DEVICE_ATTR(csc_bypass, (S_IRUGO | S_IWUSR), show_csc_bypass, store_csc_bypass);

static ssize_t show_n_cts(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;
	u32 n, cts;

	hdmi_n_cts(&n, &cts);

	num += snprintf(buf + num, PAGE_SIZE - num,  "%d %d\n", n, cts);

	return num;
}
static DEVICE_ATTR(n_cts, (S_IRUGO), show_n_cts, NULL);

#ifdef Enable_IT6802_CEC
static ssize_t show_cec_poll(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num,  "%d\n\n", _cec_poll);

	return num;
}

static ssize_t store_cec_poll(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	u32 c, cfg;

	c = sscanf(buf, "%d", &cfg);

	if (c >= 1)
		_cec_poll = cfg;

	return count;
}
static DEVICE_ATTR(cec_poll, (S_IRUGO | S_IWUSR), show_cec_poll, store_cec_poll);
#endif

static struct attribute *dev_attrs[] = {
	&dev_attr_registers.attr,
	&dev_attr_registers_bank.attr,
	&dev_attr_cec.attr,
	&dev_attr_LoopbackEnable.attr,
	&dev_attr_csc_bypass.attr,
	&dev_attr_n_cts.attr,
#ifdef Enable_IT6802_CEC
	&dev_attr_cec_poll.attr,
#endif
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static void recver_info_init(void)
{
	struct recver_info recver;

	memset(&recver, 0, sizeof(struct recver_info));

	recver.chip = recver_chip_it6802;
	recver.cap = recver_cap_cec;
	recver.dev_exist = dev_exist;
	recver.sync_audio = sync_audio;
	recver.video_timing_info = video_timing_info;
	recver.video_avi_info = video_avi_info;
	recver.video_vsd_info = video_vsd_info;
	recver.video_hdr_info = video_hdr_info;
	recver.video_update = video_update;
	recver.audio_update =  audio_update;
#ifdef Enable_IT6802_CEC
	recver.cec_send = cec_send;
	recver.cec_topology = cec_topology;
#endif
#if SUPPORT_HDCP_REPEATER
	recver.hdcp_mode = hdcp_mode;
	recver.hdcp_status = hdcp_status;
#endif
	recver.hdmi_n_cts = hdmi_n_cts;

	vrx_register_info(&recver);
}

static void recver_info_fini(void)
{
	struct recver_info recver;

	memset(&recver, 0, sizeof(struct recver_info));
	vrx_register_info(&recver);
}

#if (INTR_MODE == 1)
static irqreturn_t isr(int irq, void *dev_id, struct pt_regs *regs)
{
	struct it680x_drv_data *d;

	/* check share interrupt */
	if (!gpio_get_int_stat(GPIO_IT6802_INT))
		return IRQ_NONE;

	/* disable interrupt */
	gpio_cfg(GPIO_IT6802_INT, AST1500_GPIO_INT_DISABLE);

	/* ack interrupt. Disabling GPIO int won't clear its status, so we need to do it by ourself. */
	gpio_ack_int_stat(GPIO_IT6802_INT);

	d = dev_id;

	/* IRQ Handle */
	queue_work(d->wq, &d->cec_work);

	return IRQ_HANDLED;
}

static int intr_init(struct it680x_drv_data *d)
{
	/* request IRQ */
	if (request_irq (INT_GPIO, &isr, SA_SHIRQ, MODULE_NAME, d))
		return -1;

	/* enable interrupt after registering ISR */
	gpio_cfg(GPIO_IT6802_INT, AST1500_GPIO_INT_LEVEL_LOW);

	return 0;
}
#endif

static int __devinit it680x_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct it680x_drv_data *d;

	/*
	** Note! Probe fail won't trigger dev remove! WTF Linux driver framework.
	*/
	uinfo("it680x driver probe\n");
	//BruceToDo. Do the probe thing. Ask SCU driver for information?

	/* Reset device. Do HW reset */
	ast_scu.scu_op(SCUOP_DEVICE_RST, (void *) SCU_RST_HDMI_RX);

	/* I2C init */
	I2CInit(I2C_HDMI_RX, I2C_IT680X_SPEED);

	d = kzalloc(sizeof(struct it680x_drv_data), GFP_KERNEL);
	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}

	ast_drv_init(d);

	/* BruceToDo. VideoIP need vrxhal_xx() interfaces. Current link mananger
	** doesn't handle 'device not exists' case. And causes problem.
	** As a temp solution, we always allocate it680x driver context even device
	** doesn't exists. We should have a better design (maybe dummy vrxhal) in
	** the production stage.
	*/
	if (!is_dev_exists()) {
		//Just ignore it when device not exists.
		uerr("IT6802 device not exists?!\n");
		ret = -ENODEV;
		goto dummy_dev;
	}

	if (ast_dev_init(d)) {
		uerr("Device init failed?!\n");
		ret = -ENOMEM;
		goto err2;
	}

	if (create_works_and_thread(d)) {
		goto err2;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		BUG();
		goto err1;
	}

	recver_info_init();

	d->dev_exist = 1;

	platform_set_drvdata(pdev, (void*)d);

	/* Start it680x state machine */
	queue_delayed_work(d->wq, &d->timerwork, msecs_to_jiffies(LOOP_MSEC));

#if (INTR_MODE == 1)
	if (intr_init(d)) {
		recver_info_fini();
		platform_set_drvdata(pdev, NULL);
		goto err1;
	}
#endif

#ifdef Enable_IT6802_CEC
	queue_delayed_work(d->wq, &d->cec_pa_work, msecs_to_jiffies(CEC_PA_MSEC));
#endif

	return ret;

err1:
	destroy_works_and_thread(d);
err2:
	if (d)
		kfree(d);
err:
	return ret;

dummy_dev:
	if (create_works_and_thread(d)) {
		goto err2;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		BUG();
		goto err1;
	}

	platform_set_drvdata(pdev, (void*)d);

	return 0;
}

static int __devexit it680x_remove(struct platform_device *pdev)
{
	struct it680x_drv_data *d = platform_get_drvdata(pdev);

	uinfo("Hello remove!\n");
	if (!d)
		return 0;

	recver_info_fini();
	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);
	//BruceToDo. move to _remove_workqueue()
	destroy_works_and_thread(d);

	kfree(d);

	return 0;
}

static struct platform_driver it680x_driver = {
	.probe		= it680x_probe,
	.remove		= __devexit_p(it680x_remove),
	.suspend	= NULL,
	.resume		= NULL,
	.driver		= {
		.name	= MODULE_NAME,
	},
};


static int __init it680x_init(void)
{
	int ret;

	uinfo("IT6801/IT6802 driver based on ITE sample driver V1.13.20141121\n");

	pdev = platform_device_register_simple(
	                           MODULE_NAME, /* driver name string */
	                           -1,          /* id */
	                           NULL,        /* struct resource to alloc */
	                           0);          /* resource number */
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		goto out;
	}

	ret = platform_driver_register(&it680x_driver);
	if (ret == 0)
		goto out;

	platform_device_unregister(pdev);

out:
	uinfo("init done\n");
	return ret;
}

static void __exit it680x_exit(void)
{
	struct platform_device *it680x_dev = pdev;

	platform_driver_unregister(&it680x_driver);
	platform_device_unregister(it680x_dev);
}

module_init(it680x_init);
module_exit(it680x_exit);
