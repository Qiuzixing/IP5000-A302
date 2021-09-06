/*
 * LED Kernel Timer Trigger
 *
 * Copyright 2005-2006 Openedhand Ltd.
 *
 * Author: Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/sysdev.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/leds.h>
#include "leds.h"

struct timer_trig_data {
	unsigned long delay_on;		/* milliseconds on */
	unsigned long delay_off;	/* milliseconds off */
	struct timer_list timer;
};

extern unsigned long a30_delay_on;		/* milliseconds on */
extern unsigned long a30_delay_off;	/* milliseconds off */

static void led_timer_function(unsigned long data)
{
	struct led_classdev *led_cdev = (struct led_classdev *) data;
	struct timer_trig_data *timer_data = led_cdev->trigger_data;
	unsigned long brightness = LED_OFF;
	unsigned long delay = timer_data->delay_off;

	if (!timer_data->delay_on || !timer_data->delay_off) {
		//printk("!!t:cdev(%p) bright(%d)\n", led_cdev, LED_OFF);
		led_set_brightness(led_cdev, LED_OFF);
		return;
	}

	if (!led_cdev->brightness) {
		brightness = LED_FULL;
		delay = timer_data->delay_on;
	}

	//printk("!!t:cdev(%p) bright(%lu)\n", led_cdev, brightness);
	led_set_brightness(led_cdev, brightness);

	mod_timer(&timer_data->timer, jiffies + msecs_to_jiffies(delay));
}



static void timer_trig_activate(struct led_classdev *led_cdev)
{
	struct timer_trig_data *timer_data;

	timer_data = kzalloc(sizeof(struct timer_trig_data), GFP_KERNEL);
	if (!timer_data)
		return;

	led_cdev->trigger_data = timer_data;

	init_timer(&timer_data->timer);
	timer_data->timer.function = led_timer_function;
	timer_data->timer.data = (unsigned long) led_cdev;
	timer_data->delay_on = a30_delay_on;
	timer_data->delay_off = a30_delay_off;
	mod_timer(&timer_data->timer, jiffies + 1);


}

static void timer_trig_deactivate(struct led_classdev *led_cdev)
{
	struct timer_trig_data *timer_data = led_cdev->trigger_data;

	if (timer_data) {
		del_timer_sync(&timer_data->timer);
		kfree(timer_data);
	}
}

static struct led_trigger timer_led_trigger = {
	.name     = "timer",
	.activate = timer_trig_activate,
	.deactivate = timer_trig_deactivate,
};

static int __init timer_trig_init(void)
{
	return led_trigger_register(&timer_led_trigger);
}

static void __exit timer_trig_exit(void)
{
	led_trigger_unregister(&timer_led_trigger);
}

//module_init(timer_trig_init);
subsys_initcall(timer_trig_init);
module_exit(timer_trig_exit);

MODULE_AUTHOR("Richard Purdie <rpurdie@openedhand.com>");
MODULE_DESCRIPTION("Timer LED trigger");
MODULE_LICENSE("GPL");
