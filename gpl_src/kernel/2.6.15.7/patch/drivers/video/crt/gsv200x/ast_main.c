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
//#if defined(CONFIG_AST1500_gsv200x)
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <linux/cdev.h>
#include <asm/uaccess.h> // For copy_from_user()
#include <asm/arch/drivers/crt.h>
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/video_hal.h>
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/crt.h>
#include <aspeed/hdcp1.h>
#include "ast_main.h"
#include "ast_utilities.h"
//#include "gsv200x_adapter.h"



#define MODULE_NAME     "gsv200x"
MODULE_LICENSE ("GPL");

static struct platform_device *pdev;
static struct gsv200x_drv_data *drv_data;

#define MAX_gsv200x_DEVS 1
static struct cdev gsv200x_devs[MAX_gsv200x_DEVS];
static int gsv200x_major = 0;
static unsigned char ast_hdcp_status = 0;
extern struct s_xmiter_info xmiter_info[];
#if SUPPORT_HDCP_REPEATER
void CRT_HDCP1_Downstream_Port_Auth_Status_Callback(unsigned char authenticated, unsigned char *Bksv, unsigned char Bcaps, unsigned short Bstatus, unsigned char *KSV_FIFO);
#endif

#define A_MAX_PAYLOAD 1024

static int gsv200x_fop_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int gsv200x_fop_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t gsv200x_fop_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

static ssize_t gsv200x_fop_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
	return 0;
}

static struct file_operations gsv200x_fops = {
//	.owner   = THIS_MODULE,
	.open    = gsv200x_fop_open,
	.release = gsv200x_fop_release,
	.read = gsv200x_fop_read,
	.write = gsv200x_fop_write,
};
#if SUPPORT_HDCP_REPEATER
#define HDCP_RETRY_MAX 20
static void gsv_hdcp1_downstream_port_auth(int enable)
{
	char msg[A_MAX_PAYLOAD];
	uinfo("HDCP %s\n", enable?"ON":"OFF");
	uinfo("enable = 0x%x\n", enable);
	uinfo("ast_hdcp_status = 0x%x\n", ast_hdcp_status);
    int authenticated = 0;

	if(enable)
	{
		switch(ast_hdcp_status)
		{
			case HDCP_V_1X:
				if(HDCP_V_1X == enable )
				{
					authenticated = 1;
				}
				break;
			case HDCP_V_22:
				if(HDCP_V_22 == enable)
				{
					authenticated = 1;
				}
				break;
			case HDCP_V_11_AND_22:
				authenticated = 1;
				break;
			case HDCP_DISABLE:
				authenticated = 0;
				break;
			default:
				break;
		}

		if( authenticated == 1 )
		{
			if(HDCP_V_1X == enable)
			{
				snprintf(msg, A_MAX_PAYLOAD, "e_%s", "hdcp_1.x");
			}
			else if(HDCP_V_22 == enable || HDCP_V_11_AND_22 == enable)
			{
				snprintf(msg, A_MAX_PAYLOAD, "e_%s", "hdcp_2.x");
			}
			else
			{
				snprintf(msg, A_MAX_PAYLOAD, "e_%s", "hdcp_no_hdcp");
			} 
			ast_notify_user(msg);
		}

		//authenticated=0:executes OSD,echo hdcp fail    authenticated=1:Normal plotting    authenticated=2:Black screen
    	CRT_HDCP1_Downstream_Port_Auth_Status_Callback(authenticated, NULL, 0, 0, NULL);
	}
	else
	{
		snprintf(msg, A_MAX_PAYLOAD, "e_%s", "hdcp_no_hdcp");
		ast_notify_user(msg);
	}

}
#endif

static void xmiter_info_init(void)
{
	struct s_xmiter_info *pinfo;
	pinfo = xmiter_info + XIDX_CLIENT_D;

#if SUPPORT_HDCP_REPEATER
	if (pinfo->Hdcp1_set_mode) {
		pinfo->Hdcp1_set_mode(0);
		pinfo->Hdcp1_set_mode = NULL;
	}

	if (pinfo->Hdcp1_auth) {
		pinfo->Hdcp1_auth(0);
		pinfo->Hdcp1_auth = gsv_hdcp1_downstream_port_auth;
	}

	if (pinfo->Hdcp1_encrypt) {
		pinfo->Hdcp1_encrypt(0);
		pinfo->Hdcp1_encrypt = NULL;
	}
	pinfo->cap |= xCap_EXTERNAL_HDCP;
#endif

}

static void xmiter_info_fini(void)
{
	struct s_xmiter_info *pinfo;
	pinfo = xmiter_info + XIDX_CLIENT_D;
#if SUPPORT_HDCP_REPEATER
	pinfo->Hdcp1_set_mode = NULL;
	pinfo->Hdcp1_auth = NULL;
	pinfo->Hdcp1_encrypt = NULL;
	pinfo->cap &= ~xCap_EXTERNAL_HDCP;
#endif
}

static ssize_t show_ast_hdcp_status(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num,  "%d\n\n", ast_hdcp_status);

	return num;
}

static ssize_t store_ast_hdcp_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	char c, cfg;
	c = sscanf(buf, "%c", &cfg);

 	if (c >= 1)
	{
		switch(cfg)
		{
			case '0':
				ast_hdcp_status = HDCP_DISABLE;
				break;
			case '1':
				ast_hdcp_status = HDCP_V_1X;
				break;
			case '2':
				ast_hdcp_status = HDCP_V_20;
				break;
			case '3':
				ast_hdcp_status = HDCP_V_21;
				break;
			case '4':
				ast_hdcp_status = HDCP_V_22;
				break;
			case '5':
				ast_hdcp_status = HDCP_V_11_AND_22;
				break;
			default:
				printk("warning:error param\n");
				break;
		}
	}

	return count;
}
static DEVICE_ATTR(ast_hdcp_status, (S_IRUGO | S_IWUSR), show_ast_hdcp_status, store_ast_hdcp_status);
static struct attribute *dev_attrs[] = {
	&dev_attr_ast_hdcp_status.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

static int drv_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct gsv200x_drv_data *d;
	dev_t	dev;

	uinfo("Hello probe!\n");

	d = kzalloc(sizeof(struct gsv200x_drv_data), GFP_KERNEL);

	if (!d) {
		uerr("Failed to allocate driver data?!\n");
		ret = -ENOMEM;
		goto err;
	}
	drv_data = d;

	if (gsv200x_major){
		dev = MKDEV(gsv200x_major, 0);
		ret = register_chrdev_region(dev, MAX_gsv200x_DEVS, MODULE_NAME);
	} else {
		ret = alloc_chrdev_region(&dev, 0, MAX_gsv200x_DEVS, MODULE_NAME);
	}
	if (ret < 0) {
		uerr("Failed to allocate char dev for gsv200x?! (%i)\n", ret);
		goto err2;
	}
	gsv200x_major = MAJOR(dev);
	uinfo("gsv200x_major is %d\n", gsv200x_major);

	cdev_init(gsv200x_devs, &gsv200x_fops);
	gsv200x_devs->owner = THIS_MODULE;
	ret = cdev_add(gsv200x_devs, dev, 1);
	if (ret < 0){
		uerr("Error %d adding gsv200x char dev\n", ret);
		return ret;
	}

	if (sysfs_create_group(&pdev->dev.kobj, &dev_attr_group)) {
		uerr("can't create sysfs files\n");
		goto out;
	}

	platform_set_drvdata(pdev, (void *)d);

	drv_data = d;

    xmiter_info_init();
out:
	return ret;
err2:
	kfree(d);
err:
	return ret;
}

static int drv_remove(struct platform_device *pdev)
{
	struct gsv200x_drv_data *d = platform_get_drvdata(pdev);

	if (!d)
		return 0;


	sysfs_remove_group(&pdev->dev.kobj, &dev_attr_group);

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
	if(ast_scu.astparam.model_number == A30_IPD5000W)
	{
		return 0;
	}
	printk(KERN_ERR "hello gsv200x\n");

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
	if(ast_scu.astparam.model_number == A30_IPD5000W)
	{
		return;
	}
	printk(KERN_ERR "goodbay gsv200x\n");
	platform_driver_unregister(&sii_driver);
	unregister_chrdev_region(MKDEV(gsv200x_major, 0), 1);
	platform_device_unregister(pdev);
}

#ifndef MODULE
//arch_initcall(drv_init);
module_init(drv_init);
#else
module_init(drv_init);
#endif
module_exit(drv_exit);
//#endif
