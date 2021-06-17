#include "rtk_switch.h"
#include "rtk_error.h"
#include "rtl8364_i2c_driver.h"
#include "rtk_types.h"
#include "rate.h"
#include "eee.h"
#include "rtl8367c_asicdrv.h"
#include "rtl8367c_asicdrv_misc.h"
#include "rtl8367c_asicdrv_green.h"
#include "rtl8367c_asicdrv_lut.h"
#include "rtl8367c_asicdrv_rma.h"
#include "rtl8367c_asicdrv_mirror.h"
#include "rtl8367c_asicdrv_scheduling.h"
#include "rtl8367c_asicdrv_inbwctrl.h"
#include "rtl8367c_asicdrv_port.h"
#include "port.h"

#define ENET_PHY_CONFIG_25V 0
#define ENET_PHY_CONFIG_18V 1



return_t enet_phy_init(void)
{
    rtk_port_t asic_port;
    rtk_port_t mcu_port;
	rtk_data_t tx_delay = 0;
	rtk_data_t rx_delay = 0;
	rtk_port_mac_ability_t portability;
    //hi2c_phy = get_i2c_handle_pointer(CHIP_PHY_INDEX);

    if (rtk_switch_init() != RT_ERR_OK)
    {
        return STATUS_ERROR;
    }

    if (rtk_eee_init() != RT_ERR_OK)
    {
        return STATUS_ERROR;
    }

    asic_port = EXT_PORT0; //lint to ast1520
    //mcu_port = EXT_PORT1;

    if (rtk_port_rgmiiDelayExt_set(EXT_PORT0, tx_delay, rx_delay) != RT_ERR_OK)
    {
        printk("rgmii error\n");
        return STATUS_ERROR;
    }


    /* Disable Port UTP 3 */
    
    portability.forcemode = MAC_FORCE;
    portability.speed = SPD_1000M;
    portability.duplex = FULL_DUPLEX;
    portability.link = PORT_LINKUP;
    portability.nway = DISABLED;
    portability.txpause = ENABLED;
    portability.rxpause = ENABLED;
    
    //rtk_port_macForceLink_set(UTP_PORT0, &portability);
    /* Set Ext port 1 to 100Mbit */
/*     portability.forcemode = MAC_FORCE;
    portability.speed = SPD_10M;
    portability.duplex = FULL_DUPLEX;
    portability.link = PORT_LINKUP;
    portability.nway = DISABLED;
    portability.txpause = DISABLED;
    portability.rxpause = DISABLED; */

    rtk_port_macForceLinkExt_set(asic_port, MODE_EXT_RGMII, &portability);

    /* Set Ext port 1 to 1Gbit */
    //portability.speed = SPD_10M;
    //rtk_port_macForceLinkExt_set(asic_port, MODE_EXT_RGMII, &portability);

    //enet_phy_delay_set(config);

    return STATUS_OK;
}

