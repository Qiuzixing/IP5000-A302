/*
 * This file implements common functions used by xmiter_hal
 */
#ifndef _XMITER_COMMON_C_
#define _XMITER_COMMON_C_

static unsigned int _poll_hotplug(struct s_crt_drv *crt)
{
	//check analog hotplug every 5 seconds
	queue_delayed_work(crt->xmiter_wq, &crt->ReadDisplayEdidWork, msecs_to_jiffies(5000));
	return 0;
}


static unsigned int _check_hotplug_gpio(struct s_crt_drv *crt)
{
	unsigned int ret;
	unsigned int gpio_num;

#if defined(CONFIG_ARCH_AST1500_CLIENT)
	if (crt->is_client)
		gpio_num = GPIO_CLIENT_VIDEO_MONITOR_DETECT;
#endif
#if defined(CONFIG_ARCH_AST1500_HOST)
	if (!crt->is_client)
#if defined(CONFIG_AST1500_HOST_VIDEO_LOOPBACK) && !defined(CONFIG_AST1500_BOARD_V3)
		gpio_num = GPIO_HOST_VIDEO_MONITOR_DETECT;
#else
		return 0;
#endif
#endif

    //Monitor Detect Input
    gpio_direction_input(gpio_num);
    
    if (gpio_get_value(gpio_num))
        ret = 1;
    else
        ret = 0;
    
	return ret;
}


static unsigned int  _rd_edid_chksum_i2c(struct s_crt_drv *crt, unsigned char *pChksum)
{
	unsigned int DevSel;
	unsigned int DevAddr;

	if (crt->is_client) {
		DevSel = I2C_CLIENT_VIDEO_DDC_EEPROM;
		DevAddr = I2C_DDC_EDID_ADDR;
	} else {
#if (BOARD_DESIGN_VER_VIDEO >= 105)
		DevSel = I2C_HOST_VIDEO_LOOPBACK_DDC;
		DevAddr = I2C_DDC_EDID_ADDR;
#else
		//TBD
		BUG();
		DevSel = I2C_HOST_VIDEO_DDC_EEPROM;
		DevAddr = I2C_DDC_EDID_ADDR;
#endif
	}

	if (IsI2CReadyLite(DevSel, DevAddr))
	{
		/* do not print message to ease loading when analog not connected */
#if 0
		uinfo("I2C (%d,0x%0x) is not ready\n", DevSel, DevAddr);
#endif
		return -ENODEV;

	}

	if (read_i2c_byte(DevSel,
	                  DevAddr,
	                  EDID_CHECKSUM_OFFSET,
	                  pChksum))
	{
		uerr("Read EDID checksum failed\n");
		return -ENODEV;
	}
	//uinfo("EDID checksum read = %02X\n", *pChksum);
	return 0;
}


static unsigned int _rd_edid_i2c(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	unsigned int DevSel;
	unsigned int DevAddr;
	unsigned int i;
	unsigned char bTemp;

	if (crt->is_client) {
		DevSel = I2C_CLIENT_VIDEO_DDC_EEPROM;
		DevAddr = I2C_DDC_EDID_ADDR;
	} else {
#if (BOARD_DESIGN_VER_VIDEO >= 105)
		DevSel = I2C_HOST_VIDEO_LOOPBACK_DDC;
		DevAddr = I2C_DDC_EDID_ADDR;
#else
		//TBD. 
		BUG();
		DevSel = I2C_HOST_VIDEO_DDC_EEPROM;
		DevAddr = I2C_DDC_EDID_ADDR;
#endif
	}


	if (IsI2CReady(DevSel, DevAddr))
	{
		uinfo("I2C is not ready\n");
		goto fail;
	}

	for (i = 0; i < EDID_BLOCK_SIZE; i++)
	{
		if (read_i2c_byte(DevSel, 
			        DevAddr, 
			        i + (EDID_BLOCK_SIZE * blk_num), 
			        &bTemp))
		{
			uerr("Get EDID Fail\n");
			goto fail;
		}
		pEdid[i] = bTemp;
		/* jerry150511 add this sleep back to release CPU occupation when reading edid */
		msleep(1);
	}

	return 0;
fail:
	return -ENODEV;
}


/* check hotplug by reading edid. */
static unsigned int _check_hotplug_edid(struct s_crt_drv *crt)
{
	unsigned char t;

	if (_xHal_rd_edid_chksum(crt, &t) == 0) {
		/* is plugged */
		return 1;
	}
	/* Not plugged */
	return 0;
}


#if 0//useless now
static unsigned int _rd_edid_default(struct s_crt_drv *crt, unsigned int blk_num, unsigned char *pEdid)
{
	/* FPGA's I2C 2 is not available. Use default value instead. */
#if defined(CONFIG_AST1500_ANALOG_SOURCE) || defined(CONFIG_AST1500_ANALOG_SINK)
	memcpy(pEdid, AST_EDID_VGA + (EDID_BLOCK_SIZE * blk_num), EDID_BLOCK_SIZE);
#else
	memcpy(pEdid, AST_EDID_HDMI + (EDID_BLOCK_SIZE * blk_num), EDID_BLOCK_SIZE);
#endif
	return 0;
}

static unsigned int _rd_edid_chksum_default(struct s_crt_drv *crt, unsigned char *pChksum)
{
	/* FPGA's I2C 2 is not available. Use default value instead. */
#if defined(CONFIG_AST1500_ANALOG_SOURCE) || defined(CONFIG_AST1500_ANALOG_SINK)
	*pChksum = AST_EDID_VGA[EDID_CHECKSUM_OFFSET];
#else
	*pChksum = AST_EDID_HDMI[EDID_CHECKSUM_OFFSET];
#endif
	return 0;

}
#endif


#endif //#ifndef _XMITER_COMMON_C_

