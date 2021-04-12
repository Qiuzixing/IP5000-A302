
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#include <asm/arch/ast-scu.h>
#include "astdebug.h"
#include <asm/arch/drivers/crt.h>
#include "cursor.h"


#define DRIVER_AUTHOR "Bruce"
#define DRIVER_DESC "ASpeed 1500 display driver"

static const char driver_name[] = "display";


struct s_crt_drv *crt = NULL;


static ssize_t show_dbg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	return sprintf(buf, "0x%x\n", (crt->debug_flag & ~(CDBG_CASTING_MODE_3)));
}

static ssize_t store_dbg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 flag;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sscanf(buf, "%x", &flag);

	if (flag & CDBG_VERIFY_MT) {
		verify_timing_table(crt);
		return count;
	}
#if (CONFIG_AST1500_SOC_VER >= 2)
	if ((flag & CDBG_DESK_OFF) && !((crt->debug_flag) & CDBG_DESK_OFF)) {
		crt_disable_desktop(crt);
	} else if (!(flag & CDBG_DESK_OFF) && ((crt->debug_flag) & CDBG_DESK_OFF)) {
		crt_enable_desktop(crt);
	}
#endif
	crt->debug_flag = flag;

	return count;
}
static DEVICE_ATTR(debug, (S_IRUGO | S_IWUSR), show_dbg, store_dbg);


static ssize_t show_OSD(struct device *dev, struct device_attribute *attr, char *buf)
{
    int nEn, nAlpha, nNum;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

    nEn     = (crt->OSD_VideoOn_Setting & OSD_DISPLAY_MASK);
    nAlpha  = (crt->OSD_VideoOn_Setting & OSD_SET_ALPHA_MASK)>>OSD_SET_ALPHA_BIT;
    nNum    = (crt->OSD_VideoOn_Setting & OSD_BITMAP_MASK)>>OSD_BITMAP_BIT;
    sprintf(buf, "Enable[%d], Alpha[%d], Picture Num[%d]\n", nEn, nAlpha, nNum);
    return strlen(buf);
}

static ssize_t store_OSD(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int nEn, nAlpha, nNum;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d %d %d", &nEn, &nAlpha, &nNum);
	crt->OSD_VideoOn_Setting = nEn|(nAlpha<<OSD_SET_ALPHA_BIT)|(nNum<<OSD_BITMAP_BIT);
	crt->OSD_force_2_color = 0;
	flush_workqueue(crt->wq);
	queue_work(crt->wq, &crt->OsdVideoOnWork);
	return strlen(buf);
}
static DEVICE_ATTR(osd, (S_IRUGO | S_IWUSR), show_OSD, store_OSD);

#if OSD_FROM_GUI
static ssize_t show_OSD_from_gui(struct device *dev, struct device_attribute *attr, char *buf)
{
    int nEn, nAlpha, nNum;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

    nEn     = (crt->OSD_VideoOn_Setting & OSD_DISPLAY_MASK);
    nAlpha  = (crt->OSD_VideoOn_Setting & OSD_SET_ALPHA_MASK)>>OSD_SET_ALPHA_BIT;
    nNum    = (crt->OSD_VideoOn_Setting & OSD_BITMAP_MASK)>>OSD_BITMAP_BIT;
    sprintf(buf, "Enable[%d], Alpha[%d], Picture Num[%d]\n", nEn, nAlpha, nNum);
    return strlen(buf);
}

static ssize_t store_OSD_from_gui(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int nEn, nAlpha;
	u32 force_2_color, y_offset, y_size;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	/* may have delayed work if OSD is enabled before */
	cancel_delayed_work(&crt->OsdVideoOnWork);
	flush_workqueue(crt->wq);

	sscanf(buf, "%d %d %x %d %d", &nEn, &nAlpha, &force_2_color, &y_offset, &y_size);

	if (nEn) {
		crt->OSD_force_2_color = force_2_color;
		crt->OSD_y_offset = y_offset;
		crt->OSD_y_size = y_size;
	}

	crt->OSD_VideoOn_Setting = nEn|(nAlpha<<OSD_SET_ALPHA_BIT)|(OSD_FROM_GUI_NUM<<OSD_BITMAP_BIT);

	queue_work(crt->wq, &crt->OsdVideoOnWork);
	flush_workqueue(crt->wq);

	return strlen(buf);
}
static DEVICE_ATTR(osd_from_gui, (S_IRUGO | S_IWUSR), show_OSD_from_gui, store_OSD_from_gui);
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT) && (CONFIG_AST1500_SOC_VER >= 3) && (OSD_FROM_GUI >= 3)
static ssize_t show_OSD_position(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	char *align_str[] = {
		"Default",
		"Top-Left",
		"Top",
		"Top-Right",
		"Left",
		"Center",
		"Right",
		"Bottom-Left",
		"Bottom",
		"Bottom-Right"
	};

	BUG_ON(crt == NULL);

    sprintf(buf, "Alignment[%s], x offset[%i], y offset[%i]\n",
	        align_str[crt->OSD_pos_align],
	        crt->OSD_pos_x_offset,
	        crt->OSD_pos_y_offset);

    return strlen(buf);
}

static ssize_t store_OSD_position(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int alignment, x_offset, y_offset;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%i %i %i", &alignment, &x_offset, &y_offset);

	crt->OSD_pos_align = alignment;
	crt->OSD_pos_x_offset = x_offset;
	crt->OSD_pos_y_offset = y_offset;

	if (crt->OSD_VideoOn_Setting & OSD_DISPLAY_MASK) {
		/* re-draw to update OSD position. */
		flush_workqueue(crt->wq);
		queue_work(crt->wq, &crt->OsdVideoOnWork);
	}
	return strlen(buf);
}
static DEVICE_ATTR(osd_position, (S_IRUGO | S_IWUSR), show_OSD_position, store_OSD_position);
#endif

#if (CONFIG_AST1500_SOC_VER >= 3)
static ssize_t show_logo(struct device *dev, struct device_attribute *attr, char *buf)
{
	int nEn, nAlpha;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	nEn     = (crt->Logo_Setting & OSD_DISPLAY_MASK);
	nAlpha  = (crt->Logo_Setting & OSD_SET_ALPHA_MASK)>>OSD_SET_ALPHA_BIT;
	sprintf(buf, "Enable[%d], Alpha[%d]\n", nEn, nAlpha);
	return strlen(buf);
}

static ssize_t store_logo(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	int nEn, nAlpha;
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	int rv;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	rv = sscanf(buf, "%d %d", &nEn, &nAlpha);
	if (1 <= rv) {
		if (nEn) {
			if (rv < 2)
				nAlpha = OSD_ALPHA_VALUE;
			crt->Logo_Setting = OSD_DISPLAY_ENABLE | (nAlpha << OSD_SET_ALPHA_BIT);
		} else
			crt->Logo_Setting &= ~OSD_DISPLAY_ENABLE;

#if defined(LOGO_ON_OSD_BUFFER)
		schedule_work(&crt->LogoWork);
#endif
	}

	return strlen(buf);
}
static DEVICE_ATTR(logo, (S_IRUGO | S_IWUSR), show_logo, store_logo);
#endif

static ssize_t show_screen(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	switch (crt->current_owner) {
	case OWNER_CON:
		sprintf(buf, "[%d] message screen\n", crt->current_owner);
		break;
	case OWNER_VE1:
		sprintf(buf, "[%d] decode screen\n", crt->current_owner);
		break;
	case OWNER_HOST:
		sprintf(buf, "[%d] host loopback screen\n", crt->current_owner);
		break;
	case OWNER_NONE:
		sprintf(buf, "[%d] no screen\n", crt->current_owner);
		break;
	default:
		uerr("unsupported screen number(%d)\n", crt->current_owner);
		break;
	}

    return strlen(buf);
}

static ssize_t store_screen(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int con_num;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", &con_num);

	if ((crt->current_owner == con_num) && !(crt->op_state & COPS_PWR_SAVE)) {
		/*
		 * just skip the same screen setup at decode screen
		 *
		 * this change is to fix a case which crt_enable_desktop doesn't get executed:
		 * 1. client has connected to host and wait for video data (at message screen, desktop is disabled)
		 * 2. run do_CLIENT_EVENT_STOP_STREAM before power save timer expired
		 * 3. LM run into to_s_idle(), switch_to_GUI_screen will switch to message screen
		 *	it got filtered in the same screen check => do nothing
		 * 4. users get black screen because no-one do crt_enable_desktop
		 *
		 */
		if (con_num == OWNER_VE1) {
			uinfo("change to the same screen\n");
			return strlen(buf);
		}
	}

	if (!(con_num & crt->available_screen)) {
		uerr("screen(%d) not setup yet\n", con_num);
		return strlen(buf);
	}


	switch (con_num) {
	case OWNER_CON:
		crt_to_console_screen(crt);
		break;
	case OWNER_VE1:
		crt_to_decode_screen(crt);
		break;
	default:
		uerr("unsupported screen number(%d)\n", con_num);
		break;
	}

	return strlen(buf);
}

static DEVICE_ATTR(screen, (S_IRUGO | S_IWUSR), show_screen, store_screen);

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static ssize_t show_ignore_cts7_33(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	return sprintf(buf, "%d\n", (crt->debug_flag & CDBG_IGNORE_CTS7_33)?(1):(0));
}

static ssize_t store_ignore_cts7_33(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 on;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sscanf(buf, "%x", &on);

	if (on) {
		crt->debug_flag |= CDBG_IGNORE_CTS7_33;
	} else {
		crt->debug_flag &= ~CDBG_IGNORE_CTS7_33;
	}

	return strlen(buf);
}
static DEVICE_ATTR(ignore_cts7_33, (S_IRUGO | S_IWUSR), show_ignore_cts7_33, store_ignore_cts7_33);
#endif

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static ssize_t show_screen_off(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (crt->op_state & COPS_FORCE_SYNC_OFF)
		sprintf(buf, "%s\n", "enable");
	else
		sprintf(buf, "%s\n", "disable");

	return strlen(buf);
}

static ssize_t store_screen_off(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int enable;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%x", &enable);

	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_force_sync_off, (void *)enable, 0);

	return strlen(buf);
}
static DEVICE_ATTR(screen_off, (S_IRUGO | S_IWUSR), show_screen_off, store_screen_off);
#endif


#if defined(CONFIG_ARCH_AST1500_CLIENT)
static ssize_t show_pwr_save(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (crt->op_state & COPS_PWR_SAVE)
		sprintf(buf, "%s\n", "enable");
	else
		sprintf(buf, "%s\n", "disable");

	return strlen(buf);
}

static ssize_t store_pwr_save(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int enable;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%x", &enable);

	crt_pwr_save(crt, enable);

	return strlen(buf);
}
static DEVICE_ATTR(power_save, (S_IRUGO | S_IWUSR), show_pwr_save, store_pwr_save);
#endif

static ssize_t show_csr_init(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "csr_info: %p\n", crt->csr_info);

	csr_dump_cache(crt);

    return strlen(buf);
}

static ssize_t store_csr_init(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct socket *sock = NULL;
	int user_socket;
	int *err = 0;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", &user_socket);
	uinfo("coip socket=%d\n", user_socket);

	if (user_socket != -1) {
		sock = sockfd_lookup(user_socket, err);
		if (!sock) {
			uerr("create kernel soocket\n");
			return -EINVAL;
		}
	}

	csr_initial(crt, sock);

	return strlen(buf);
}

static DEVICE_ATTR(csr_init, (S_IRUGO | S_IWUSR), show_csr_init, store_csr_init);


static ssize_t show_csr_en(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d\n", csr_is_enable(crt));

    return strlen(buf);
}

static ssize_t store_csr_en(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int enable;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", &enable);

	csr_enable(crt, enable);

	return strlen(buf);
}

static DEVICE_ATTR(csr_en, (S_IRUGO | S_IWUSR), show_csr_en, store_csr_en);


static ssize_t show_csr_chg(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d\n", crt->csr_info->cur_idx);

    return strlen(buf);
}

static ssize_t store_csr_chg(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int x, y, index;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d %d %d", &x, &y, &index);

	csr_chg_shape(crt, x, y, index);

	return strlen(buf);
}

static DEVICE_ATTR(csr_chg, (S_IRUGO | S_IWUSR), show_csr_chg, store_csr_chg);


static ssize_t show_csr_mov(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "idx:%d, hw_offset:0x%08x, hw_pos:0x%08x\n",
		crt->csr_info->cur_idx,
		csr_get_hw_offset(crt),
		csr_get_hw_pos(crt));

    return strlen(buf);
}

static ssize_t store_csr_mov(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int x, y;
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d %d", &x, &y);

	csr_move(crt, x, y);

	return strlen(buf);
}
static DEVICE_ATTR(csr_mov, (S_IRUGO | S_IWUSR), show_csr_mov, store_csr_mov);

static ssize_t show_Tx(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);


	if (xHal_get_xmiter_cap(crt) == xCap_RGB)
		return sprintf(buf, "analog\n");
	else
		return sprintf(buf, "digital\n");
}
static DEVICE_ATTR(tx, S_IRUGO, show_Tx, NULL);

#if 0
static void _append_buf_to_str(void *buf, int bufflen, char *str_buf)
{
	int i;
	unsigned char *buff= buf;

	//sprintf(str_buf + strlen(str_buf), "dump 0x%08x: %d bytes\n", (u32)(buff), bufflen);

	if (bufflen > 768) {
		for (i = 0; i< 768; i++) {
			if (i%16 == 0)
				sprintf(str_buf + strlen(str_buf), "   ");
			sprintf(str_buf + strlen(str_buf), "%02x ", (unsigned char ) buff[i]);
			if (i%4 == 3) sprintf(str_buf + strlen(str_buf), "| ");
			if (i%16 == 15) sprintf(str_buf + strlen(str_buf), "\n");
		}
		sprintf(str_buf + strlen(str_buf), "... (%d byte)\n", bufflen);
		return;
	}

	for (i = 0; i< bufflen; i++) {
		if (i%16 == 0)
			sprintf(str_buf + strlen(str_buf), "   ");
		sprintf(str_buf + strlen(str_buf), "%02x ", (unsigned char ) buff[i]);
		if (i%4 == 3)
			sprintf(str_buf + strlen(str_buf), "| ");
		if (i%16 == 15)
			sprintf(str_buf + strlen(str_buf), "\n");
	}
	sprintf(str_buf + strlen(str_buf), "\n");

}
#endif

static int edid_info_display(char *buf, edid_status *status, char *edid_raw)
{
	int num = 0, i;
	MODE_ITEM *mode;


	num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "Checksum: %s\n", (status->checksum_error) ? "fail":"ok");
	num += snprintf(buf + num, PAGE_SIZE - num, "CEA Ext: %s\n", (status->cea_ext_valid) ? "y":"n");

	if (status->cea_ext_valid)
		num += snprintf(buf + num, PAGE_SIZE - num, "CEA Ext Checksum: %s\n", (status->cea_ext_checksum_error) ? "fail":"ok");

	num += snprintf(buf + num, PAGE_SIZE - num, "HDMI 2.0: %s\n", (status->hdmi_20) ? "y":"n");
	num += snprintf(buf + num, PAGE_SIZE - num, "HDMI 1.x: %s\n", (status->hdmi_1x) ? "y":"n");
	if (status->hdmi_1x) {
		u16 pa = status->hdmi_physical_address;
		num += snprintf(buf + num, PAGE_SIZE - num, "PA: %2d.%2d.%2d.%2d\n",
				(pa >> 12) & 0xf, (pa >> 8) & 0xf, (pa >> 4) & 0xf, (pa >> 0) & 0xf);

	} else {
		num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "Input Signal: %s\n", (status->digital) ? "digital":"analog");

	num += snprintf(buf + num, PAGE_SIZE - num, "Support Features:\n");
	if (status->digital)
		num += snprintf(buf + num, PAGE_SIZE - num, "    yuv: %s\n", (status->yuv) ? "y" : "n");

	num += snprintf(buf + num, PAGE_SIZE - num, "    hdr: %s\n", (status->hdr) ? "y" : "n");


	mode = crt_timing_by_index(status->preferred_timing_index);

	num += snprintf(buf + num, PAGE_SIZE - num, "Preferred Timing:\n");

	num += snprintf(buf + num, PAGE_SIZE - num, "    %dx%d@%dHz,%s (idx %d, sn %d)\n",
			mode->HActive,
			(mode->ScanMode == Prog) ? (mode->VActive) : (mode->VActive << 1),
			mode->RefreshRate,
			(mode->ScanMode == Prog) ? "Prog" : "Interl",
			status->preferred_timing_index,
			mode->sn);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	mode = crt_timing_by_index(status->preferred_timing_index_raw);

	num += snprintf(buf + num, PAGE_SIZE - num, "Monitor Preferred Timing:\n");

	num += snprintf(buf + num, PAGE_SIZE - num, "    %dx%d@%dHz,%s (idx %d, sn %d)\n",
			mode->HActive,
			(mode->ScanMode == Prog) ? (mode->VActive) : (mode->VActive << 1),
			mode->RefreshRate,
			(mode->ScanMode == Prog) ? "Prog" : "Interl",
			status->preferred_timing_index_raw,
			mode->sn);
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	num += snprintf(buf + num, PAGE_SIZE - num, "EDID:\n");

	for (i = 0; i < EDID_ROM_SIZE; i++) {
		if (i%16 == 0)
			num += snprintf(buf + num, PAGE_SIZE - num, "   ");
		num += snprintf(buf + num, PAGE_SIZE - num, "%02x ", edid_raw[i]);
		if (i%4 == 3)
			num += snprintf(buf + num, PAGE_SIZE - num, "| ");
		if (i%16 == 15)
			num += snprintf(buf + num, PAGE_SIZE - num, "\n");
	}
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}

static ssize_t show_monitor_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	int num = 0;

	BUG_ON(crt == NULL);

	num += snprintf(buf + num, PAGE_SIZE - num, "attached=%c\n", (crt->display_attached & 0x1)?'y':'n');

	if (!(crt->display_attached & 0x1))
		goto out;

	num += edid_info_display(buf + num, &crt->edid_parse_result, crt->edid);
out:
	return num;
}
static DEVICE_ATTR(monitor_info, (S_IRUGO), show_monitor_info, NULL);

static ssize_t show_timing_info(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *crt_info = NULL;
	int i;

	BUG_ON(crt == NULL);

	if (crt->current_owner == OWNER_NONE)
		goto out;

	switch (crt->current_owner) {
	case OWNER_CON:
		crt_info = &crt->CON_mode;
		break;
	case OWNER_VE1:
		crt_info = &crt->VE1_mode;
		break;
	case OWNER_HOST:
		crt_info = &crt->HOST_mode;
		break;
	default:
		uerr("un-supported screen?!(%d)\n", crt->current_owner);
		goto out;
	}
	i = lookupModeTable(crt, crt_info);
	sprintf(buf, "timing=[%d] %dx%d%c@%dHz H%c V%c, Serial Number[0x%04X]\n",
		i,
		(crt_info->crt_output_format == YUV420_FORMAT) ? (crt->mode_table[i].HActive << 1) : (crt->mode_table[i].HActive),
		(crt->mode_table[i].ScanMode == Prog) ? (crt->mode_table[i].VActive) : (crt->mode_table[i].VActive << 1),
		(crt->mode_table[i].ScanMode == Prog) ? 'p' : 'i',
		crt->mode_table[i].RefreshRate,
		(crt->mode_table[i].HorPolarity == NegPolarity) ? '-' : '+',
		(crt->mode_table[i].VerPolarity == NegPolarity) ? '-' : '+',
		crt_timing_index_to_sn(i)
		);
	sprintf(buf + strlen(buf), "type=%s\n", SIGNAL_TYPE_STR(crt_info->HDMI_Mode));
	sprintf(buf + strlen(buf), "HDCP=%c (%s)\n", crt_info->EnHDCP ? 'y' : 'n', HDCP_VER_STR(crt_info->EnHDCP));
	sprintf(buf + strlen(buf), "color depth=%d\n", crt_info->color_depth);

out:
	return strlen(buf);
}
static DEVICE_ATTR(timing_info, (S_IRUGO), show_timing_info, NULL);

static u8 edid[EDID_ROM_SIZE];
static edid_status sysfs_edid_parse_result;

static ssize_t show_edid_parse(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += edid_info_display(buf, &sysfs_edid_parse_result, edid);

	return num;
}
static ssize_t store_edid_parse(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 i = 0, len = strlen(buf);
	u8 *word, *str;
	char *sep = " |,\n\r";

	str = (unsigned char *) buf;

	while (i < EDID_ROM_SIZE) {
		word = strsep((char **)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		edid[i] = (unsigned char) simple_strtoul(word, NULL, 16);
		i++;
	}

	crt->ast_edid->parse(edid, &sysfs_edid_parse_result);

	return len;
}
static DEVICE_ATTR(edid_parse, (S_IRUGO | S_IWUSR), show_edid_parse, store_edid_parse);

static ssize_t show_default_edid_hdmi(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	if (ast_scu.ability.v_support_4k >= 3)
		udump_s(AST_EDID_HDMI_4K, EDID_ROM_SIZE, buf);
	else
		udump_s(AST_EDID_HDMI, EDID_ROM_SIZE, buf);

	return strlen(buf);
}

static ssize_t store_default_edid_hdmi(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int i = 0;
	unsigned int len = strlen(buf);
	unsigned char *word;
	unsigned char *str;
	char *sep=" |,\n\r";
	unsigned char *edid;

	if (ast_scu.ability.v_support_4k >= 3)
		edid = AST_EDID_HDMI_4K;
	else
		edid = AST_EDID_HDMI;

	str = (unsigned char*)buf;
	while (i < EDID_ROM_SIZE) {
		word = strsep((char**)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		edid[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}
	queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, 0);
	return len;
}
static DEVICE_ATTR(default_edid_hdmi, (S_IRUGO | S_IWUSR), show_default_edid_hdmi, store_default_edid_hdmi);

static ssize_t show_default_edid_dvi(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	udump_s(AST_EDID_DVI, EDID_ROM_SIZE, buf);

	return strlen(buf);
}

static ssize_t store_default_edid_dvi(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int i = 0;
	unsigned int len = strlen(buf);
	unsigned char *word;
	unsigned char *str;
	char *sep=" |,\n\r";

	str = (unsigned char*)buf;
	while (i < EDID_ROM_SIZE) {
		word = strsep((char**)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		AST_EDID_DVI[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}
	queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, 0);
	return len;
}
static DEVICE_ATTR(default_edid_dvi, (S_IRUGO | S_IWUSR), show_default_edid_dvi, store_default_edid_dvi);

static ssize_t show_default_edid_vga(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	udump_s(AST_EDID_VGA, EDID_ROM_SIZE, buf);

	return strlen(buf);
}

static ssize_t store_default_edid_vga(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int i = 0;
	unsigned int len = strlen(buf);
	unsigned char *word;
	unsigned char *str;
	char *sep=" |,\n\r";

	str = (unsigned char*)buf;
	while (i < EDID_ROM_SIZE) {
		word = strsep((char**)&str, sep);
		if (!word)
			break;
		if (strlen(word) == 0)
			continue;
		AST_EDID_VGA[i] = (unsigned char)simple_strtoul(word, NULL, 16);
		i++;
	}
	queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, 0);
	return len;
}
static DEVICE_ATTR(default_edid_vga, (S_IRUGO | S_IWUSR), show_default_edid_vga, store_default_edid_vga);


#if (CONFIG_AST1500_SOC_VER != 2)
static ssize_t show_vw_layout(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d %d %d %d\n",
		crt->vw_max_row,
		crt->vw_max_col,
		crt->vw_row_num,
		crt->vw_col_num);

	return strlen(buf);
}

static ssize_t store_vw_layout(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int max_row, max_col, row_num, col_num;
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;
	int scanned;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	scanned = sscanf(buf, "%d %d %d %d", &max_row, &max_col, &row_num, &col_num);

	if (scanned != 4)
		return -EINVAL;

	crt->vw_max_row = max_row;
	crt->vw_max_col = max_col;
	crt->vw_row_num = row_num;
	crt->vw_col_num = col_num;

	if (max_row || max_col)
		crt->vw_enabled = 1;
	else
		crt->vw_enabled = 0;

#if defined(VW_API_V2_SUPPORT)
	crt->vw_ver = 1;
#endif
	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_layout, (S_IRUGO | S_IWUSR), show_vw_layout, store_vw_layout);

#if defined(VW_API_V2_SUPPORT)
static ssize_t show_vw_layout_v2(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d %d %d %d\n",
		crt->vw_top_left_x,
		crt->vw_top_left_y,
		crt->vw_bottom_right_x,
		crt->vw_bottom_right_y);

	return strlen(buf);
}

static void row_col_to_coordiante(u32 max_row, u32 max_col, u32 row_num, u32 col_num, u32 scale, u32 *px1, u32 *py1, u32 *px2, u32 *py2)
{
	u32 ht, ha, vt, va;
	u32 x1, y1, x2, y2;
	u32 x_total, y_total;

	ha = crt->vw_fc_h_active;
	ht = crt->vw_fc_h_total;
	va = crt->vw_fc_v_active;
	vt = crt->vw_fc_v_total;

	x_total = max_col * ht + ha;
	y_total = max_row * vt + va;

	x1 = col_num * ht;
	y1 = row_num * vt;
	x2 = x1 + ha;
	y2 = y1 + va;

	/* round off by adding denominator / 2, (x_total/2 , y_total/2) */
	x1 = (x1 * scale + (x_total >> 1)) / x_total;
	y1 = (y1 * scale + (y_total >> 1)) / y_total;
	x2 = (x2 * scale + (x_total >> 1)) / x_total;
	y2 = (y2 * scale + (y_total >> 1)) / y_total;

	*px1 = x1;
	*py1 = y1;
	*px2 = x2;
	*py2 = y2;
}

static ssize_t store_vw_layout_v2(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	u32 top_left_x, top_left_y, bottom_right_x, bottom_right_y, transfer = 0;
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;
	int scanned;

	BUG_ON(crt == NULL);

	if (ast_scu.ability.soc_op_mode < 3) {
		uinfo("This function is for (SOC OP MODE >= 3) only\n");
		return strlen(buf);
	}

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	scanned = sscanf(buf, "%d %d %d %d %d", &top_left_x, &top_left_y, &bottom_right_x, &bottom_right_y, &transfer);

	if (scanned < 4)
		goto v2_store_exit;


	if (transfer) {
		row_col_to_coordiante(top_left_x, top_left_y,
					bottom_right_x, bottom_right_y,
					VW_COORDINATE_MAX,
					&crt->vw_top_left_x,
					&crt->vw_top_left_y,
					&crt->vw_bottom_right_x,
					&crt->vw_bottom_right_y);
	} else {
		crt->vw_top_left_x = top_left_x;
		crt->vw_top_left_y = top_left_y;
		crt->vw_bottom_right_x = bottom_right_x;
		crt->vw_bottom_right_y = bottom_right_y;
	}

	if (crt->vw_top_left_x > VW_COORDINATE_MAX)
		crt->vw_top_left_x = VW_COORDINATE_MAX;

	if (crt->vw_top_left_y > VW_COORDINATE_MAX)
		crt->vw_top_left_y = VW_COORDINATE_MAX;

	if (crt->vw_bottom_right_x > VW_COORDINATE_MAX)
		crt->vw_bottom_right_x = VW_COORDINATE_MAX;

	if (crt->vw_bottom_right_y > VW_COORDINATE_MAX)
		crt->vw_bottom_right_y = VW_COORDINATE_MAX;

	if ((crt->vw_top_left_x > crt->vw_bottom_right_x)
		|| (crt->vw_top_left_y > crt->vw_bottom_right_y)) {
		goto v2_store_exit;
	}

	if ((crt->vw_top_left_x != crt->vw_bottom_right_x)
		|| (crt->vw_top_left_y != crt->vw_bottom_right_y)) {
		crt->vw_enabled = 1;
		crt->vw_ver = 2;
	} else {
		crt->vw_enabled = 0;
		crt->vw_ver = 1;
	}

	crt_vw_enable(crt, i);

v2_store_exit:
	return strlen(buf);
}
static DEVICE_ATTR(vw_layout_v2, (S_IRUGO | S_IWUSR), show_vw_layout_v2, store_vw_layout_v2);
#endif /* #if defined(VW_API_V2_SUPPORT) */

static ssize_t show_vw_frame_comp(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d %d %d %d\n", crt->vw_fc_h_active
	                            , crt->vw_fc_h_total
	                            , crt->vw_fc_v_active
	                            , crt->vw_fc_v_total);

	return strlen(buf);
}

static ssize_t store_vw_frame_comp(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d %d %d %d", &crt->vw_fc_h_active
	                         , &crt->vw_fc_h_total
	                         , &crt->vw_fc_v_active
	                         , &crt->vw_fc_v_total);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_frame_comp, (S_IRUGO | S_IWUSR), show_vw_frame_comp, store_vw_frame_comp);

static ssize_t show_vw_scale_up(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d %d\n", crt->vw_extra_h_scale_up
	                      , crt->vw_extra_v_scale_up);

	return strlen(buf);
}

static ssize_t store_vw_scale_up(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d %d", &crt->vw_extra_h_scale_up
	                   , &crt->vw_extra_v_scale_up);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_scale_up, (S_IRUGO | S_IWUSR), show_vw_scale_up, store_vw_scale_up);

static ssize_t show_vw_h_shift(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d\n", crt->vw_h_shift);

	return strlen(buf);
}

static ssize_t store_vw_h_shift(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", &crt->vw_h_shift);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_h_shift, (S_IRUGO | S_IWUSR), show_vw_h_shift, store_vw_h_shift);

static ssize_t show_vw_v_shift(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d\n", crt->vw_v_shift);

	return strlen(buf);
}

static ssize_t store_vw_v_shift(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", &crt->vw_v_shift);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_v_shift, (S_IRUGO | S_IWUSR), show_vw_v_shift, store_vw_v_shift);
#endif //#if (CONFIG_AST1500_SOC_VER != 2)

#if (CONFIG_AST1500_SOC_VER >= 3)
static unsigned char *output_rotate[] = {
	"Normal",
	"Vertical flip",
	"Horiziontal flip",
	"Rotate 180 degree",
	"Backslash",
	"Rotate 90 degree",
	"Rotate 270 degree",
	"Slash"
};

static ssize_t show_vw_rotate(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	int num = 0;
	u32 val;

	BUG_ON(crt == NULL);

	num += snprintf(buf + num, PAGE_SIZE - num, "\nCurrent Setting: %s\n\n", output_rotate[crt->vw_rotate]);
	num += snprintf(buf + num, PAGE_SIZE - num, "Help:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t #echo [mode] > vw_rotate\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t mode:\n");

	for (val = 0; val < 8; val++)
		num += snprintf(buf + num, PAGE_SIZE - num, "\t     %d => %s\n", val, output_rotate[val]);

	return num;
}

static ssize_t store_vw_rotate(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", (int *)&crt->vw_rotate);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_rotate, (S_IRUGO | S_IWUSR), show_vw_rotate, store_vw_rotate);


static ssize_t show_vw_stretch_type(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	int num = 0;
	u32 val;
	unsigned char *s[] = {
		"Auto",
		"Stretch out",
		"Fit in"
	};

	BUG_ON(crt == NULL);

	num += snprintf(buf + num, PAGE_SIZE - num, "\nCurrent Setting: %s\n\n", s[crt->vw_stretch_type]);
	num += snprintf(buf + num, PAGE_SIZE - num, "Help:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t #echo [type] > vw_stretch_type\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t type:\n");

	for (val = 0; val < 3; val++)
		num += snprintf(buf + num, PAGE_SIZE - num, "\t     %d => %s\n", val, s[val]);

	return num;
}

static ssize_t store_vw_stretch_type(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	struct s_crt_info *i = &crt->VE1_mode;

	BUG_ON(crt == NULL);

	if (!crt->is_client) {
		uinfo("This function is for client board only\n");
		return strlen(buf);
	}

	sscanf(buf, "%d", (int *)&crt->vw_stretch_type);

	crt_vw_enable(crt, i);

	return strlen(buf);
}
static DEVICE_ATTR(vw_stretch_type, (S_IRUGO | S_IWUSR), show_vw_stretch_type, store_vw_stretch_type);
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#if (CONFIG_AST1500_SOC_VER >= 2)
static ssize_t show_dual_port(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);

	BUG_ON(crt == NULL);

	sprintf(buf, "%d\n", crt->dual_port);

	return strlen(buf);
}

static ssize_t store_dual_port(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_drv *crt = dev_get_drvdata(dev);
	unsigned int on;

	BUG_ON(crt == NULL);

	sscanf(buf, "%d", &on);

	queue_crt_work_n_wait(crt, (crt_work_func_t)_crt_cfg_dual_port, (void *)on, 0);

	return strlen(buf);
}
static DEVICE_ATTR(dual_port, (S_IRUGO | S_IWUSR), show_dual_port, store_dual_port);
#endif //#if (CONFIG_AST1500_SOC_VER >= 2)

#if defined(CONFIG_ARCH_AST1500_CLIENT)
static u32 test_horizontal, test_vertical, test_refresh_rate = 60, test_type = 0;
static u32 test_color = BLACK_XRGB888, test_color2 = WHITE_XRGB888;
static u32 test_is_hdmi = 1, test_hdcp_mode = HDCP_DISABLE;
static unsigned int test_ftcfg = 0;

static ssize_t show_test_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
	int num = 0;

	num += snprintf(buf + num, PAGE_SIZE - num, "WIDTH: %d, HEIGHT: %d, REFRESH_RATE: %d\n",
		test_horizontal, test_vertical, test_refresh_rate);
	num += snprintf(buf + num, PAGE_SIZE - num, "IS_HDMI: %d, HDCP_MODE: %d, force_timing_cfg=0x%08X\n",
		test_is_hdmi, test_hdcp_mode, test_ftcfg);
	num += snprintf(buf + num, PAGE_SIZE - num, "PATTERN_TYPE: %d, COLOR: 0x%.8x COLOR2: 0x%.8x\n",
		test_type, test_color, test_color2);
	num += snprintf(buf + num, PAGE_SIZE - num, "Help:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t #echo [WIDTH] [HEIGHT] [REFRESH_RATE] [IS_HDMI] [HDCP_MODE] [PATTERN_TYPE] [COLOR] [COLOR2] > test_mode\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t IS_HDMI: 0 => DVI, 1 => HDMI (Send info-frame)\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t HDCP_MODE: 0 => disable, 1 => HDCP 1.4, 2 => HDCP 2.2\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t PATTERN_TYPE: 0 => black, 1 => blue, 2 => purple\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               3 => pattern #1 [two tone in half]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               4 => pattern #2 [two tone in X pixels. X is specified in XRGB's X.]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               5 => pattern #3 [backslash + x/y boundary]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               6 => pattern #4 [64pixels wide color bar]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               7 => pattern ramp [ramp/stair per pixel]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               8 => pattern ramp full [ramp/stair full screen]\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               9 => pattern dynamic random\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t               other else => white\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t COLOR/COLOR2: XRGB8888 format in hexadecimal.\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t       PURPLE:0xFF8000FF, BLACK: 0xFF000000\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t       BLUE: 0xFF0000FF, WHITE: 0xFFFFFFFF\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "Example:\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t # echo 4096 2160 24 0 0 4 > /sys/devices/platform/display/test_mode\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t # echo 4096 2160 24 1 1 5 0xFF000000 0xFFFFFFFF> /sys/devices/platform/display/test_mode\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\t # echo 3840 2160 30 1 2 4 0xFF000000 0xFFFFFFFF> /sys/devices/platform/display/test_mode\n");
	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}

static ssize_t store_test_mode(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	struct s_crt_info i;

	/* re-init test_color, in case next command want to use default test_color value. */
	/* We use white first in order to trigger all high case in the first left edge. */
	test_color = WHITE_XRGB888;
	test_color2 = BLACK_XRGB888;

	sscanf(buf, "%d %d %d %d %d %d 0x%x 0x%x",
		&test_horizontal, &test_vertical, &test_refresh_rate,
		&test_is_hdmi, &test_hdcp_mode,
		&test_type, &test_color, &test_color2);

	memset(&i, 0, sizeof(struct s_crt_info));

	i.crt_color = XRGB8888_FORMAT;
	i.crt_output_format = XRGB8888_FORMAT;
	i.edid = crt->edid;
	i.width = test_horizontal;
	i.height = test_vertical;
	i.refresh_rate = test_refresh_rate;
	i.owner = OWNER_CON;
	i.clr = 0;
	i.line_offset = 0;
	i.AVI_Info.number = 0;
	i.HDMI_Info.number = 0;
	i.HDR_Info.number = 0;
	i.HDMI_Mode = s_HDMI_16x9;
	switch(test_hdcp_mode) {
	case 0:
		i.EnHDCP = HDCP_DISABLE;
		break;
	case 1:
		i.EnHDCP = HDCP_V_1X;
		break;
	case 2:
		i.EnHDCP = HDCP_V_22;
		break;
	default:
		i.EnHDCP = HDCP_DISABLE;
		break;
	}

	/*
	** Construct force_timing_cfg:
	** We always use force_timing_cfg for test_mode so that HDMI info frame
	** can be generated by force_timing function call.
	*/
	test_ftcfg = 0;
	test_ftcfg = FTCFG_S_ENABLE(test_ftcfg, 1);
	test_ftcfg = FTCFG_S_DVI(test_ftcfg, test_is_hdmi?0:1);
	test_ftcfg = FTCFG_S_TYPE(test_ftcfg, FTCFG_TYPE_TIMING_SN);
	test_ftcfg = FTCFG_S_TIMING_CODE(test_ftcfg, crt_timing_index_to_sn(lookupModeTable(crt, &i)));
	i.force_timing.cfg = test_ftcfg;

	crt_setup_par(crt, &i);

	crt_to_console_screen(crt);

	vbufset_test_mode(crt, &i, test_type, test_color, test_color2);

	return count;
}
static DEVICE_ATTR(test_mode, (S_IRUGO | S_IWUSR), show_test_mode, store_test_mode);
#endif

static struct attribute *dev_attrs[] = {
	&dev_attr_debug.attr,
	&dev_attr_osd.attr,
	&dev_attr_screen.attr,
	&dev_attr_csr_init.attr,
	&dev_attr_csr_en.attr,
	&dev_attr_csr_chg.attr,
	&dev_attr_csr_mov.attr,
	&dev_attr_tx.attr,
	&dev_attr_monitor_info.attr,
	&dev_attr_timing_info.attr,
	&dev_attr_edid_parse.attr,
	&dev_attr_default_edid_hdmi.attr,
	&dev_attr_default_edid_dvi.attr,
	&dev_attr_default_edid_vga.attr,
#if (CONFIG_AST1500_SOC_VER >= 2)
	&dev_attr_dual_port.attr,
#endif /* #if (CONFIG_AST1500_SOC_VER >= 2) */
#if defined(CONFIG_ARCH_AST1500_CLIENT)
	&dev_attr_power_save.attr,
	&dev_attr_screen_off.attr,
	&dev_attr_ignore_cts7_33.attr,
	&dev_attr_test_mode.attr,

#if OSD_FROM_GUI
	&dev_attr_osd_from_gui.attr,
#if (OSD_FROM_GUI >= 3)
	&dev_attr_osd_position.attr,
#endif
#endif

#if (CONFIG_AST1500_SOC_VER != 2)
	&dev_attr_vw_layout.attr,
#if defined(VW_API_V2_SUPPORT)
	&dev_attr_vw_layout_v2.attr,
#endif
	&dev_attr_vw_frame_comp.attr,
	&dev_attr_vw_scale_up.attr,
	&dev_attr_vw_h_shift.attr,
	&dev_attr_vw_v_shift.attr,
#endif //#if (CONFIG_AST1500_SOC_VER != 2)

#if (CONFIG_AST1500_SOC_VER >= 3)
	&dev_attr_logo.attr,
	&dev_attr_vw_rotate.attr,
	&dev_attr_vw_stretch_type.attr,
#endif /* #if (CONFIG_AST1500_SOC_VER >= 3) */

#endif /* #if defined(CONFIG_ARCH_AST1500_CLIENT) */
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};


static int crt_probe(struct platform_device *pdev)
{
	int ret;

	crt = kzalloc(sizeof(struct s_crt_drv), GFP_KERNEL);

	if (!crt) {
		uerr("Failed to allocate crt driver context.\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, (void*)crt);

	ret = crt_init(crt);

	if (ret)
		goto err;

	ast_edid_init((void *)crt);

	return 0;

err:
	if (crt)
		kfree(crt);

	return ret;
}

static int crt_remove(struct platform_device *pdev)
{
	struct s_crt_drv *crt = platform_get_drvdata(pdev);

	if (crt) {
		crt_exit(crt);
		kfree(crt);
	}
	return 0;
}

/* Unnecessary?! */
#define crt_suspend	NULL
#define crt_resume	NULL
static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) driver_name,
	.id = -1,
	.dev = {
		.release = the_pdev_release,
		.driver_data = NULL,
	},
};

static struct platform_driver crt_driver = {
    .probe	       = crt_probe,
    .remove	= __devexit_p(crt_remove),
    .suspend    = crt_suspend,
    .resume	= crt_resume,
    .driver	= {
        .name = (char *) driver_name,
        .owner = THIS_MODULE,
    },
};


int __init crt_module_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&crt_driver);
	if (ret < 0) {
		uerr("platform_driver_register err\n");
		goto err_driver_register;
	}


    ret = platform_device_register(&pdev);
    if (ret < 0) {
        uerr("platform_device_register err\n");
        goto err_platform_device_register;
    }

    ret = sysfs_create_group(&pdev.dev.kobj, &dev_attr_group);
    if (ret) {
        uerr("can't create sysfs files\n");
        goto err_sysfs_create;
    }

	uinfo("display.ko loaded\n");
	return 0;

err_sysfs_create:
	platform_device_unregister(&pdev);
err_platform_device_register:
	platform_driver_unregister(&crt_driver);
err_driver_register:
	return ret;
}

void __exit crt_cleanup(void)
{
	platform_device_unregister(&pdev);
	platform_driver_unregister(&crt_driver);
	uinfo("display.ko unloaded\n");
}

#ifndef MODULE
subsys_initcall(crt_module_init);
#else
module_init (crt_module_init);
#endif
module_exit (crt_cleanup);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

