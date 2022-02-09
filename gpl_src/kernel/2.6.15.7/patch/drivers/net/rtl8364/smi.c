/*
* Copyright c                  Realtek Semiconductor Corporation, 2006
* All rights reserved.
*
* Program : Control smi connected RTL8366
* Abstract :
* Author : Yu-Mei Pan (ympan@realtek.com.cn)
*  $Id: smi.c,v 1.2 2008-04-10 03:04:19 shiehyy Exp $
*/
#include "rtk_types.h"
#include "smi.h"
#include "rtk_error.h"
#include "rtl8364_i2c_driver.h"
#include "../ftgmac100_26.h"
extern void gb_rtl8367_phy_write_register(unsigned short int Register_addr,unsigned short int register_value);
extern unsigned short int  gb_rtl8367_phy_read_register(unsigned short int Register_addr);

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
    //*rData = rtl8364_i2c_read_reg(mAddrs);
    * rData = gb_rtl8367_phy_read_register(mAddrs);
    return 0;
}

rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
    //rtl8364_i2c_write_reg(mAddrs,rData);
    gb_rtl8367_phy_write_register(mAddrs,rData);
    return 0;
}


