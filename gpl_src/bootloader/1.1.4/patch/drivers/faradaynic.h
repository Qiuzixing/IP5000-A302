#ifndef _FARADAYNIC_H_
#define _FARADAYNIC_H_
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <pci.h>
extern void gb_rtl8367_phy_write_register(u16 Register_addr,u16 register_value);
extern u16 gb_rtl8367_phy_read_register(u16 Register_addr);

#endif