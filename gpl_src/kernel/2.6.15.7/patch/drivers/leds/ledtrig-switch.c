/*
 * LED Switch Trigger
 *
 * Based on ledtrig-pushbutton.c.
 * Use this trigger to monitor GPIO switch changes.
 *
 * User can use netlink to receive the notification.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include "leds.h"

static const char driver_name[] = "ledtrig-switch";

struct switch_trig_data {
	int use_netlink;
};


/*-------------------------------------------------------------------------*/
#include <net/sock.h>
#include <linux/netlink.h>
//#include <linux/skbuff.h>

#define MAX_PAYLOAD 1024

static irqreturn_t sw_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
	struct led_classdev *led_cdev = (struct led_classdev *)dev_id;

	/* Get interrupt status. */
	led_set_brightness(led_cdev, SW_INT_STAT);

	if (led_cdev->brightness != INT_STAT_TRUE)
		return IRQ_NONE;

	/* Ack interrupt */
	led_set_brightness(led_cdev, SW_INT_ACK);

	/* Read the value. */
	led_set_brightness(led_cdev, SW_RD);

	{ //Add event to indicate pressed/released
		char msg[MAX_PAYLOAD];
		snprintf(msg, MAX_PAYLOAD, "e_%s_%s", led_cdev->name, led_cdev->brightness?"1":"0");
		ast_notify_user(msg);
	}
	return IRQ_HANDLED;
}

static void switch_trig_activate(struct led_classdev *led_cdev)
{
	struct switch_trig_data *switch_data;
	int err;

	switch_data = kzalloc(sizeof(*switch_data), GFP_KERNEL);
	if (!switch_data)
		return;

	led_cdev->trigger_data = switch_data;
	switch_data->use_netlink = 1;

	err = request_irq(20, sw_interrupt, SA_SHIRQ, driver_name, led_cdev);
	if (err) {
		printk("Unable to get IRQ\n");
		BUG();
	}

	/* Turn on interrupt. */
	led_set_brightness(led_cdev, SW_INT_ON);
}

static void switch_trig_deactivate(struct led_classdev *led_cdev)
{
	struct switch_trig_data *switch_data = led_cdev->trigger_data;

	if (switch_data) {
		/* Turn off interrupt. */
		led_set_brightness(led_cdev, SW_INT_OFF);

		free_irq(20, led_cdev);

		kfree(switch_data);
	}
}

static struct led_trigger switch_trigger = {
	.name     = "switch",
	.activate = switch_trig_activate,
	.deactivate = switch_trig_deactivate,
};

static int __init switch_trig_init(void)
{
	return led_trigger_register(&switch_trigger);
}

static void __exit switch_trig_exit(void)
{
	led_trigger_unregister(&switch_trigger);
}

subsys_initcall(switch_trig_init);
module_exit(switch_trig_exit);

