/*
 * LED Push Button Trigger
 *
 * Based on ledtrig-heartbeat.c.
 * Use this trigger to monitor push button.
 *
 * Interrupt status will be latched and stored in "brightness".
 * If auto_ack turns off, the user should write 1 to "pb_ack_indicated_store()"
 * to clear the "brightness". Otherwise "brightness" will be cleared atomatically
 * and user can use netlink to receive the notification.
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
#include <asm/arch/leds-gpio.h>
static const char driver_name[] = "ledtrig-pushbutton";

#define LONG_PRESS_INTERVAL 2000 //in ms. indicate once per LONG_PRESS_INTERVAL


struct pushbutton_trig_data {
	struct timer_list timer;
	int auto_clear;
	int use_netlink;
	int indicated;
	int delay; // in ms
	int long_press_cnt;
	int long_press_cnt_interval; // == INTERVAL/delay
	int long_press_val; // how many counting interval pressed
	int state_on_active; // Used to record the status on active(boot)
};


/*-------------------------------------------------------------------------*/
#include <net/sock.h>
#include <linux/netlink.h>
//#include <linux/skbuff.h>
struct sock *nl_sk = NULL;

#define MAX_PAYLOAD 1024


void ast_notify_user(const char *name)
{
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	int err;
	static int seq = 0;

	skb = alloc_skb(NLMSG_SPACE(MAX_PAYLOAD), GFP_ATOMIC);
	if (!skb) {
		printk("failed to allocate skb\n");
		goto nlmsg_failure;
	}

	nlh = NLMSG_NEW(skb,
			0, /* pid. from kernel. */
			seq++,
			0, /* message content. */
			strlen(name) + 1, /* length */
			0 /* flags */
			);

	strncpy(NLMSG_DATA(nlh), name, strlen(name));
	*(((char *)NLMSG_DATA(nlh)) + strlen(name)) = 0; //NULL end;

	/* sender is in group 1<<0 */
	NETLINK_CB(skb).pid = 0;  /* from kernel */
	NETLINK_CB(skb).dst_pid = 0;  /* multicast */
	/* to mcast group 1<<1 */
	NETLINK_CB(skb).dst_group = 1;

	/*multicast the message to all listening processes*/
	err = netlink_broadcast(nl_sk, skb, 0, 1/*dst_groups*/, GFP_ATOMIC);
	if (err)
		printk("netlink_broadcast failed?! (%d)\n", err);

nlmsg_failure:
	return;
}
EXPORT_SYMBOL(ast_notify_user);

static ssize_t pb_delay_show(struct class_device *dev, char *buf)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;

	sprintf(buf, "%i\n", tdata->delay);

	return strlen(buf) + 1;
}

static ssize_t pb_delay_store(struct class_device *dev, const char *buf,
				size_t size)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long ms = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (*after && isspace(*after))
		count++;

	if (count == size) {
		tdata->delay = ms;
		tdata->long_press_cnt_interval = (LONG_PRESS_INTERVAL/tdata->delay);
		//mod_timer(&tdata->timer, jiffies + 1);
		ret = count;
	}

	return ret;
}

static CLASS_DEVICE_ATTR(delay, 0644, pb_delay_show,
			pb_delay_store);


static ssize_t pb_auto_clear_show(struct class_device *dev, char *buf)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;

	sprintf(buf, "%i\n", tdata->auto_clear);

	return strlen(buf) + 1;
}

static ssize_t pb_auto_clear_store(struct class_device *dev, const char *buf,
				size_t size)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long ms = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (*after && isspace(*after))
		count++;

	if (count == size) {
		tdata->auto_clear = ms;
		//mod_timer(&tdata->timer, jiffies + 1);
		ret = count;
	}

	return ret;
}

static CLASS_DEVICE_ATTR(auto_clear, 0644, pb_auto_clear_show,
			pb_auto_clear_store);


static ssize_t pb_clear_status_store(struct class_device *dev, const char *buf,
				size_t size)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long ms = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (*after && isspace(*after))
		count++;

	if (count == size) {
		if (ms) {
			tdata->indicated = 0;
			led_set_brightness(led_cdev, PB_VAL_GET);
		}
		ret = count;
	}

	return ret;
}
static CLASS_DEVICE_ATTR(clear_status, 0222, NULL, pb_clear_status_store);


static ssize_t pb_default_state_show(struct class_device *dev, char *buf)
{
	struct led_classdev *led_cdev = class_get_devdata(dev);
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;

	sprintf(buf, "%s", ((tdata->state_on_active)?("On"):("Off")));

	return strlen(buf) + 1;
}

static CLASS_DEVICE_ATTR(state_on_active, 0444, pb_default_state_show,
			NULL);


static void pb_pressed(unsigned long data)
{
    struct led_classdev *led_cdev = (struct led_classdev *) data;
    struct pushbutton_trig_data *tdata = led_cdev->trigger_data;

    /*
    ** We can't check "led_cdev->brightness" here because this value will be cleared
    ** if there are other GPIO interrupts happen in the mean time.
    ** ==> pb_interrupt happens again before timer fired.
    ** 090821. above bug should be resolved by using the gpio_ack_int_stat() function.
    */
    if (tdata->use_netlink && !tdata->indicated) {
		led_set_brightness(led_cdev, PB_VAL_GET);
		if(led_cdev->flags & AST_LEDF_DEFAULT_ON)	
		{
			if(led_cdev->brightness == 1)	//release key
			{
				led_cdev->brightness = 0;
			}
			else							//still pressing the button
			{
				led_cdev->brightness = 1;	
			}
		}
		if (!led_cdev->brightness) {
			/*Button un-pressed*/
			if (tdata->long_press_val == 0) {
				//This is a short press. Just notify the short message.
				char msg[MAX_PAYLOAD];
				if(0 != strcmp(led_cdev->name,"audio_detect"))
				{
					snprintf(msg, MAX_PAYLOAD, "e_%s", led_cdev->name);
					ast_notify_user(msg);
					tdata->indicated = 1;
				}
				led_set_brightness(led_cdev, PB_INT_ON);
			} else {
				//This is the end of long press.
				if(0 != strcmp(led_cdev->name,"audio_detect"))
				{
					tdata->indicated = 1;
				}
				led_set_brightness(led_cdev, PB_INT_ON);
			}
			{ //Add event to indicate pressed/released
				char msg[MAX_PAYLOAD];
				snprintf(msg, MAX_PAYLOAD, "e_%s_released", led_cdev->name);
				ast_notify_user(msg);
			}

		} else {
			tdata->long_press_cnt++;
			if (tdata->long_press_cnt == tdata->long_press_cnt_interval && 0 != strcmp(led_cdev->name,"audio_detect")) {
				char msg[MAX_PAYLOAD];

				tdata->long_press_val++;
				snprintf(msg, MAX_PAYLOAD, "e_%s_%d", led_cdev->name, tdata->long_press_val);
				ast_notify_user(msg);
				tdata->long_press_cnt = 0;
			}
			mod_timer(&tdata->timer, jiffies + msecs_to_jiffies(tdata->delay));
		}

    }
    if (tdata->auto_clear) {
        tdata->indicated = 0;
        led_set_brightness(led_cdev, PB_VAL_GET);
        if (led_cdev->brightness) //Ack interrupt status again to ignore this event.
            led_set_brightness(led_cdev, PB_INT_ACK);
    }
}


static irqreturn_t pb_interrupt (int irq, void *dev_id, struct pt_regs *regs)
{
	struct led_classdev *led_cdev = (struct led_classdev *)dev_id;
	struct pushbutton_trig_data *tdata = led_cdev->trigger_data;
	unsigned long delay = msecs_to_jiffies(tdata->delay);

	//Get interrupt status.
	led_set_brightness(led_cdev, PB_INT_STAT);

	if (led_cdev->brightness != INT_STAT_TRUE)
		return IRQ_NONE;

	// Disable interrupt. Will enable again in polling timer.
	led_set_brightness(led_cdev, PB_INT_OFF);

	//Ack interrupt
	led_set_brightness(led_cdev, PB_INT_ACK);

	{ // Add event to indicate pressed/released
		char msg[MAX_PAYLOAD];
		snprintf(msg, MAX_PAYLOAD, "e_%s_pressed", led_cdev->name);
		ast_notify_user(msg);
	}

	tdata->long_press_cnt = 0;
	tdata->long_press_val = 0;
	mod_timer(&tdata->timer, jiffies + delay);

	return IRQ_HANDLED;
}

static void pushbutton_trig_activate(struct led_classdev *led_cdev)
{
	struct pushbutton_trig_data *pushbutton_data;
	int err;

	pushbutton_data = kzalloc(sizeof(*pushbutton_data), GFP_KERNEL);
	if (!pushbutton_data)
		return;

	led_cdev->trigger_data = pushbutton_data;
	pushbutton_data->delay = 200;
	pushbutton_data->use_netlink = 1;
	pushbutton_data->auto_clear = 1;
	pushbutton_data->long_press_cnt_interval = (LONG_PRESS_INTERVAL/pushbutton_data->delay);
	/* Get the default state here. */
	led_set_brightness(led_cdev, PB_VAL_GET);
	pushbutton_data->state_on_active = led_cdev->brightness;

	err = request_irq(20, pb_interrupt, SA_SHIRQ, driver_name, led_cdev);
	if (err) {
		printk("Unable to get IRQ\n");
		BUG();
	}

	setup_timer(&pushbutton_data->timer,
		    pb_pressed, (unsigned long) led_cdev);
	//pb_pressed(pushbutton_data->timer.data);

	/* Turn on interrupt. */
	led_set_brightness(led_cdev, PB_INT_ON);

	class_device_create_file(led_cdev->class_dev,
				&class_device_attr_delay);
	class_device_create_file(led_cdev->class_dev,
				&class_device_attr_auto_clear);
	class_device_create_file(led_cdev->class_dev,
				&class_device_attr_clear_status);
	class_device_create_file(led_cdev->class_dev,
				&class_device_attr_state_on_active);

}

static void pushbutton_trig_deactivate(struct led_classdev *led_cdev)
{
	struct pushbutton_trig_data *pushbutton_data = led_cdev->trigger_data;

	if (pushbutton_data) {
		/* Turn off interrupt. */
		led_set_brightness(led_cdev, PB_INT_OFF);

		free_irq(20, led_cdev);

		class_device_remove_file(led_cdev->class_dev,
					&class_device_attr_delay);
		class_device_remove_file(led_cdev->class_dev,
					&class_device_attr_auto_clear);
		class_device_remove_file(led_cdev->class_dev,
					&class_device_attr_clear_status);

		del_timer_sync(&pushbutton_data->timer);
		kfree(pushbutton_data);
	}
}

static struct led_trigger pushbutton_trigger = {
	.name     = "pushbutton",
	.activate = pushbutton_trig_activate,
	.deactivate = pushbutton_trig_deactivate,
};

static int __init pushbutton_trig_init(void)
{
	nl_sk = netlink_kernel_create(NETLINK_USERSOCK, 1, NULL, THIS_MODULE);
	if (!nl_sk) {
		printk("can't open netlink sk!?(%p)\n", nl_sk);
		return -ENODEV;
	}

	return led_trigger_register(&pushbutton_trigger);
}

static void __exit pushbutton_trig_exit(void)
{
	if (nl_sk)
		sock_release(nl_sk->sk_socket);

	led_trigger_unregister(&pushbutton_trigger);
}

subsys_initcall(pushbutton_trig_init);
//module_init(heartbeat_trig_init);
module_exit(pushbutton_trig_exit);





