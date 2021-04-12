

#ifndef __VHUB_CONFIG_H__
#define __VHUB_CONFIG_H__

void cfg_copy_patched_cfg_desc(struct usb_device *udev, struct usb_config_descriptor *cfg, unsigned int cfg_len);
int cfg_update_configuration(struct usb_device *udev, struct usb_config_descriptor *cfg, unsigned int cfg_len);
int cfg_initialize_configuration(struct usb_device *udev);
void cfg_destroy_configuration(struct usb_device *udev);
void cfg_release_interface_cache(struct kref *ref);


#endif /* __VHUB_CONFIG_H__ */
