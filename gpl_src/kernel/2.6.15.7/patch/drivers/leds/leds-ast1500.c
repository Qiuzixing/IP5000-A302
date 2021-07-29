
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/delay.h>
#include <asm/hardware.h>
//#include <asm/arch/regs-gpio.h>
#include <asm/arch/leds-gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/ast-scu.h>

/* This macro convert true/false to 1/0. */
#define LOGIC_TO_01(l) (!!(l))

/* our context */
static const char driver_name[] = "leds-ast1500";
struct ast1500_gpio_led {
	struct led_classdev		 cdev;
	struct ast_led_platdata	*pdata;
};

static inline struct ast1500_gpio_led *pdev_to_gpio(struct platform_device *dev)
{
	return platform_get_drvdata(dev);
}

static inline struct ast1500_gpio_led *to_gpio(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct ast1500_gpio_led, cdev);
}

static void ast1500_switch_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct ast1500_gpio_led *led = to_gpio(led_cdev);
	struct ast_led_platdata *pd = led->pdata;

	switch (value) {
	case SW_RD:
	case SW_RD_INV:
	    /* Always update the brightness value */
	    if (gpio_get_value(pd->gpio))
	        led_cdev->brightness = (1 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW)) ^ LOGIC_TO_01(value & SW_RD_INV);
	    else
	        led_cdev->brightness = (0 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW)) ^ LOGIC_TO_01(value & SW_RD_INV);
		break;

	case SW_INT_STAT:
		if (gpio_get_int_stat(pd->gpio))
			led_cdev->brightness = INT_STAT_TRUE;
		else
			led_cdev->brightness = INT_STAT_FALSE;
		break;

	case SW_INT_ACK:
		gpio_ack_int_stat(pd->gpio);
		break;

	case SW_INT_OFF:
		gpio_cfg(pd->gpio, AST1500_GPIO_INT_DISABLE);
		break;

	case SW_INT_ON:
		/* clear the status bit. */
		gpio_ack_int_stat(pd->gpio);
		gpio_cfg(pd->gpio, AST1500_GPIO_DEBOUNCE_T1);
		gpio_cfg(pd->gpio, AST1500_GPIO_INT_DUAL_EDGE);
		break;

	default:
		BUG();
	}
}

static void ast1500_button_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct ast1500_gpio_led *led = to_gpio(led_cdev);
	struct ast_led_platdata *pd = led->pdata;

	switch (value) {
	case PB_INT_STAT:
		if (gpio_get_int_stat(pd->gpio))
			led_cdev->brightness = INT_STAT_TRUE;
		else
			led_cdev->brightness = INT_STAT_FALSE;
		break;

	case PB_INT_ACK:
		gpio_ack_int_stat(pd->gpio);
		break;

	case PB_INT_OFF:
		gpio_cfg(pd->gpio, AST1500_GPIO_INT_DISABLE);
		break;

	case PB_INT_ON:
		gpio_cfg(pd->gpio, AST1500_GPIO_DEBOUNCE_T1);
		gpio_cfg(pd->gpio,
			(pd->flags & AST_LEDF_INT_FALLING_EDGE)?
			(AST1500_GPIO_INT_FALLING_EDGE):(AST1500_GPIO_INT_RISING_EDGE));
		/* clear the status bit. */
		gpio_ack_int_stat(pd->gpio);
		break;

	case PB_VAL_GET:
		if (gpio_get_value(pd->gpio))
			led_cdev->brightness = 1;
		 else
			led_cdev->brightness = 0;
		break;
	default:
		BUG();
		break;
	}
}

static void ast1500_led_set(struct led_classdev *led_cdev,
			    enum led_brightness value)
{
	struct ast1500_gpio_led *led = to_gpio(led_cdev);
	struct ast_led_platdata *pd = led->pdata;

	/* there will be a short delay between setting the output and
	 * going from output to input when using tristate. */

	//printk("!!ast1500_led_set:cdev(%p) bright(%d)\n", led_cdev, value);
	if (value) {
		if (pd->flags & AST_LEDF_TRISTATE_ON)
			gpio_direction_input(pd->gpio);
		else
			gpio_direction_output(pd->gpio, 1 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW));
	} else {
		if (pd->flags & AST_LEDF_TRISTATE_OFF) {
			gpio_direction_input(pd->gpio);
		} else {
			gpio_direction_output(pd->gpio, 0 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW));
		}
	}

}

static void ast1500_led_trigger_change(struct led_classdev *led_cdev, struct led_trigger *trigger)
{
	struct ast1500_gpio_led *led = to_gpio(led_cdev);
	struct ast_led_platdata *pd = led->pdata;

	if (!trigger)
		goto set_default;

	if (!strcmp("pushbutton", trigger->name)) {
		led_cdev->brightness_set = ast1500_button_set;
	} else if (!strcmp("switch", trigger->name)) {
		led_cdev->brightness_set = ast1500_switch_set;
		gpio_direction_input(pd->gpio);
	} else {
		goto set_default;
	}

	return;

set_default:
	/* Non-button LED initialization. */
	if (pd->flags & AST_LEDF_DEFAULT_ON) {
		if (pd->flags & AST_LEDF_TRISTATE_ON) {
			gpio_direction_input(pd->gpio);
		} else {
			gpio_direction_output(pd->gpio, 1 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW));
		}
	} else {
		/* no point in having a pull-up if we are always driving */
		if (pd->flags & AST_LEDF_TRISTATE_OFF) {
			gpio_direction_input(pd->gpio);
		} else {
			gpio_direction_output(pd->gpio, 0 ^ LOGIC_TO_01(pd->flags & AST_LEDF_ACTLOW));
		}
	}
	led_cdev->brightness_set = ast1500_led_set;
}

static int ast1500_led_remove(struct platform_device *dev)
{
	struct ast1500_gpio_led *led = pdev_to_gpio(dev);

	led_classdev_unregister(&led->cdev);
	kfree(led);

	return 0;
}

static void gpio_inv_sw_patch(unsigned int dev_id, struct ast_led_platdata *pd)
{
	u32 gpio_inv = ast_scu.astparam.gpio_inv;

	/*
	** Bruce161005. Read astparam, gpio_inv, and invert the GPIO for buggy HW patch.
	** We use unique dev_id as u32 bitmap for all defined GPIOs. Here is the list from ast1500.c
	**
	** case 0: // ast_ledb6, led_link
	** case 1: // ast_ledb7, led_pwr
	** case 2: // ast_ledb4, button_link
	** case 3: // ast_ledb5, button_pairing
	** case 4: // ast_ledh0, ch0
	** case 5: // ast_ledh1, ch1
	** case 6: // ast_ledh2, ch2
	** case 7: // ast_ledh3, ch3
	** case 8: // ast_led_v_input, v_input
	** case 9: // ast_led_a_input, a_input
	** case 14: // ast_leda3, button_chg_ch
	** case 15: // ast_leda5, button_remote_ctrl
	** case 16: // ast_ledc3, gpio_pwr_status_out
	** case 17: // ast_ledb2, pwr_ctrl
	** case 18: // ast_ledc6, gpio_pwr_status_in
	*/
	if (gpio_inv & (1 << dev_id)) {
		/* invert AST_LEDF_ACTLOW flag. */
		pd->flags ^= AST_LEDF_ACTLOW;
	}
}

static int ast1500_led_probe(struct platform_device *dev)
{
	struct ast_led_platdata *pdata = dev->dev.platform_data;
	struct ast1500_gpio_led *led;
	int ret;

	led = kzalloc(sizeof(struct ast1500_gpio_led), GFP_KERNEL);
	if (led == NULL) {
		dev_err(&dev->dev, "No memory for device\n");
		return -ENOMEM;
	}

	gpio_inv_sw_patch(dev->id, pdata);

	platform_set_drvdata(dev, led);

	if(ast_scu.board_info.ip5000_a30_board_type == IPD5000W_A30)
	{
		//GPIOE2
		if(0 == strcmp(pdata->name,"key_left"))
		{
			pdata->name = "led_hdmi";
			pdata->flags = AST_LEDF_DEFAULT_ON;
			pdata->def_trigger = NULL;
		}
		//GPIOE3
		if(0 == strcmp(pdata->name,"key_right"))
		{
			pdata->name = "on_red";
			pdata->flags = AST_LEDF_DEFAULT_ON;
			pdata->def_trigger = NULL;
		}
		//GPIOE4
		if(0 == strcmp(pdata->name,"key_enter"))
		{
			pdata->name = "spi_wp";
			pdata->flags = AST_LEDF_DEFAULT_ON;
			pdata->def_trigger = NULL;
		}
		//GPIOH6
		if(0 == strcmp(pdata->name,"poweron_1V3"))
		{
			pdata->name = "poweron_1V0";
			pdata->flags = AST_LEDF_DEFAULT_ON;
		}
	}

	led->cdev.brightness_set = ast1500_led_set;
	led->cdev.default_trigger = pdata->def_trigger;
	led->cdev.name = pdata->name;
	led->cdev.flags = pdata->flags;
	led->pdata = pdata;

	if (pdata->flags & AST_LEDF_BUTTON) {
	/* 
	** Moved to ast1500_button_set() with PB_INT_ON. So that button function can be
	** dynamically turned on/off in trigger active/deactive.
	*/
#if 0
		/* The initialization of GPIO interrupt mode is coded 
		** in arch/arm/ast1500.c::aspeed_init().
		** I am doing this way because I am lazy 
		** and it won't break the led class framework. */
		/* 091106. I add initialize code here. It should works. */
		gpio_cfg(pdata->gpio, AST1500_GPIO_DEBOUNCE_T1);
		gpio_cfg(pdata->gpio, 
			(pdata->flags & AST_LEDF_ACTLOW)?
			(AST1500_GPIO_INT_FALLING_EDGE):(AST1500_GPIO_INT_RISING_EDGE));
		/* clear the status bit. */
		gpio_ack_int_stat(pdata->gpio);
#endif
		led->cdev.brightness_set = ast1500_button_set;
	} else if (pdata->flags & AST_LEDF_SWITCH) {
		led->cdev.brightness_set = ast1500_switch_set;
		gpio_direction_input(pdata->gpio);
	} else {
		/* Non-button LED initialization. */
		if (pdata->flags & AST_LEDF_DEFAULT_ON) {
			if (pdata->flags & AST_LEDF_TRISTATE_ON) {
				gpio_direction_input(pdata->gpio);
			} else {
				gpio_direction_output(pdata->gpio, 1 ^ LOGIC_TO_01(pdata->flags & AST_LEDF_ACTLOW));
			}
		} else {
			/* no point in having a pull-up if we are always driving */
			if (pdata->flags & AST_LEDF_TRISTATE_OFF) {
				gpio_direction_input(pdata->gpio);
			} else {
				gpio_direction_output(pdata->gpio, 0 ^ LOGIC_TO_01(pdata->flags & AST_LEDF_ACTLOW));
			}
		}
	}
#ifdef CONFIG_LEDS_TRIGGERS
	led->cdev.trigger_chg = ast1500_led_trigger_change;
#endif

	/* register our new led device */
	ret = led_classdev_register(&dev->dev, &led->cdev);
	if (ret < 0) {
		dev_err(&dev->dev, "led_classdev_register failed\n");
		goto exit_err1;
	}
	if(0 == strcmp(pdata->name,"led_on_g"))
	{
		gpio_set_value(pdata->gpio,0);
	}
	if(0 == strcmp(pdata->name,"led_on_r"))
	{
		gpio_set_value(pdata->gpio,0);
	}
	if(0 == strcmp(pdata->name,"audio_sense_mid"))
	{
		gpio_set_value(pdata->gpio,0);
	}

	if(0 == strcmp(pdata->name,"mcu_reset"))
	{
		gpio_direction_output(pdata->gpio, 0);
		mdelay(10);
		gpio_direction_output(pdata->gpio, 1);
		mdelay(10);
		gpio_direction_input(pdata->gpio);
	}
	return 0;

 exit_err1:
	kfree(led);
	return ret;
}


#ifdef CONFIG_PM
static int ast1500_led_suspend(struct platform_device *dev, pm_message_t state)
{
	struct ast1500_gpio_led *led = pdev_to_gpio(dev);

	led_classdev_suspend(&led->cdev);
	return 0;
}

static int ast1500_led_resume(struct platform_device *dev)
{
	struct ast1500_gpio_led *led = pdev_to_gpio(dev);

	led_classdev_resume(&led->cdev);
	return 0;
}
#else
#define ast1500_led_suspend NULL
#define ast1500_led_resume NULL
#endif

static struct platform_driver ast1500_led_driver = {
	.probe		= ast1500_led_probe,
	.remove		= ast1500_led_remove,
	.suspend	= ast1500_led_suspend,
	.resume		= ast1500_led_resume,
	.driver		= {
		.name		= "ast1500_led",
		.owner		= THIS_MODULE,
	},
};

static int __init ast1500_led_init(void)
{
	return platform_driver_register(&ast1500_led_driver);
}

static void __exit ast1500_led_exit(void)
{
 	platform_driver_unregister(&ast1500_led_driver);
}

subsys_initcall(ast1500_led_init);
//module_init(ast1500_led_init);
module_exit(ast1500_led_exit);

