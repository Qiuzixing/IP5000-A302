/*
 * drivers/base/power/sysfs.c - sysfs entries for device PM
 */

#include <linux/device.h>
#include <linux/string.h>
#include "power.h"


/**
 *	state - Control current power state of device
 *
 *	show() returns the current power state of the device. '0' indicates
 *	the device is on. Other values (1-3) indicate the device is in a low
 *	power state.
 *
 *	store() sets the current power state, which is an integer value
 *	between 0-3. If the device is on ('0'), and the value written is
 *	greater than 0, then the device is placed directly into the low-power
 *	state (via its driver's ->suspend() method).
 *	If the device is currently in a low-power state, and the value is 0,
 *	the device is powered back on (via the ->resume() method).
 *	If the device is in a low-power state, and a different low-power state
 *	is requested, the device is first resumed, then suspended into the new
 *	low-power state.
 */

static ssize_t state_show(struct device * dev, struct device_attribute *attr, char * buf)
{
	return sprintf(buf, "%u\n", dev->power.power_state.event);
}

static ssize_t state_store(struct device * dev, struct device_attribute *attr, const char * buf, size_t n)
{
	pm_message_t state;
	char * rest;
	int error = 0;

	state.event = simple_strtoul(buf, &rest, 10);
	if (*rest)
		return -EINVAL;
	if (state.event)
		error = dpm_runtime_suspend(dev, state);
	else
		dpm_runtime_resume(dev);
	return error ? error : n;
}

static DEVICE_ATTR(state, 0644, state_show, state_store);

/*
 *	wakeup - Report/change current wakeup option for device
 *
 *	Some devices support "wakeup" events, which are hardware signals
 *	used to activate devices from suspended or low power states.  Such
 *	devices have one of three values for the sysfs power/wakeup file:
 *
 *	 + "enabled\n" to issue the events;
 *	 + "disabled\n" not to do so; or
 *	 + "\n" for temporary or permanent inability to issue wakeup.
 *
 *	(For example, unconfigured USB devices can't issue wakeups.)
 *
 *	Familiar examples of devices that can issue wakeup events include
 *	keyboards and mice (both PS2 and USB styles), power buttons, modems,
 *	"Wake-On-LAN" Ethernet links, GPIO lines, and more.  Some events
 *	will wake the entire system from a suspend state; others may just
 *	wake up the device (if the system as a whole is already active).
 *	Some wakeup events use normal IRQ lines; other use special out
 *	of band signaling.
 *
 *	It is the responsibility of device drivers to enable (or disable)
 *	wakeup signaling as part of changing device power states, respecting
 *	the policy choices provided through the driver model.
 *
 *	Devices may not be able to generate wakeup events from all power
 *	states.  Also, the events may be ignored in some configurations;
 *	for example, they might need help from other devices that aren't
 *	active, or which may have wakeup disabled.  Some drivers rely on
 *	wakeup events internally (unless they are disabled), keeping
 *	their hardware in low power modes whenever they're unused.  This
 *	saves runtime power, without requiring system-wide sleep states.
 */

static const char enabled[] = "enabled";
static const char disabled[] = "disabled";

static ssize_t
wake_show(struct device * dev, struct device_attribute *attr, char * buf)
{
	return sprintf(buf, "%s\n", device_can_wakeup(dev)
		? (device_may_wakeup(dev) ? enabled : disabled)
		: "");
}

static ssize_t
wake_store(struct device * dev, struct device_attribute *attr,
	const char * buf, size_t n)
{
	char *cp;
	int len = n;

	if (!device_can_wakeup(dev))
		return -EINVAL;

	cp = memchr(buf, '\n', n);
	if (cp)
		len = cp - buf;
	if (len == sizeof enabled - 1
			&& strncmp(buf, enabled, sizeof enabled - 1) == 0)
		device_set_wakeup_enable(dev, 1);
	else if (len == sizeof disabled - 1
			&& strncmp(buf, disabled, sizeof disabled - 1) == 0)
		device_set_wakeup_enable(dev, 0);
	else
		return -EINVAL;
	return n;
}

static DEVICE_ATTR(wakeup, 0644, wake_show, wake_store);

extern struct device_attribute dev_attr_constraints;

static struct attribute * power_attrs[] = {
	&dev_attr_state.attr,
	&dev_attr_wakeup.attr,
#ifdef CONFIG_DPM
	&dev_attr_constraints.attr,
#endif
	NULL,
};
static struct attribute_group pm_attr_group = {
	.name	= "power",
	.attrs	= power_attrs,
};

int dpm_sysfs_add(struct device * dev)
{
	return sysfs_create_group(&dev->kobj, &pm_attr_group);
}

void dpm_sysfs_remove(struct device * dev)
{
	sysfs_remove_group(&dev->kobj, &pm_attr_group);
}
