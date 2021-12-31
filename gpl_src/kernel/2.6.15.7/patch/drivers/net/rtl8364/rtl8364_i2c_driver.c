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
#include "vlan.h"
#include "trunk.h"
#include "port.h"
#include <asm/arch/ast-scu.h>
#include <asm/arch/drivers/board_def.h>
#define ENET_PHY_CONFIG_25V 0
#define ENET_PHY_CONFIG_18V 1
#define VLAN_ID_10  10
#define VLAN_ID_20  20
#define VLAN_ID_30  30
#define PVLAN_ID_10  10
#define PVLAN_ID_20  20
#define PVLAN_ID_30  30

#define DEFAULT_VID     1
#define DEFAULT_PVID    1
#define PRIORITY_SETTING    7
#define HIDE_VLAN_VID   4094
#define HIDE_VLAN_PVID   4094

/*  
rtl8367:
    ast1520:EXT_PORT0
    dante:UTP_PORT2
    eth0:UTP_PORT0
    eth1:UTP_PORT3
rtl8364:
    ast1520:EXT_PORT0
    eth0:UTP_PORT1
    eth1:UTP_PORT3
*/
#define AST1520_PORT           EXT_PORT0
#define DANTE_PORT             UTP_PORT2
#define RTL8367_ETH0_PORT      UTP_PORT0
#define RTL8364_ETH0_PORT      UTP_PORT1
#define ETH1_PORT              UTP_PORT3


static void handle_multicast_settings(void)
{
    rtl8367c_setAsicReg(0x0a30,0x021e);
    //rtl8367c_setAsicReg(0x08c9,0x0);
    //rtl8367c_setAsicReg(0x0891,0x0);
    rtl8367c_setAsicReg(0x1c00,0x0629);
}

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

    /* only rtl8367 need to do it,rtl8364 not need it for now */
    if(ast_scu.astparam.model_number == A30_IPE5000P)
    {
        handle_multicast_settings();
    }
    else
    {

    }

    return STATUS_OK;
}

static void eth1_p3k_eth1_dante_vlan_settings(unsigned int eth1_p3k_tag,unsigned int eth1_dante_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2,vlan3,vlan4;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan3, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan4, 0x00, sizeof(rtk_vlan_cfg_t));
    //e e_p3k_net_vlan_set_rtl_chip::0:0:1:20
    //e e_p3k_net_vlan_set_rtl_chip::0:0:20:1

    //e e_p3k_net_vlan_set_rtl_chip::0:0:1:1
    if(eth1_p3k_tag == DEFAULT_VID && eth1_dante_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
        return;
    }

    //e e_p3k_net_vlan_set_rtl_chip::0:0:20:20
    if(eth1_p3k_tag == eth1_dante_tag)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        rtk_vlan_set(eth1_p3k_tag,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,eth1_p3k_tag,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);

        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan2);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
        return;
    }

    if(eth1_p3k_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan2);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
    else
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        rtk_vlan_set(eth1_p3k_tag,&vlan2);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    if(eth1_dante_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
    else
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        rtk_vlan_set(eth1_dante_tag,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,eth1_dante_tag,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

static void eth1_p3k_eth0_dante_vlan_settings(unsigned int eth1_p3k_tag,unsigned int eth0_dante_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2,vlan3,vlan4;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan3, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan4, 0x00, sizeof(rtk_vlan_cfg_t));
    //e e_p3k_net_vlan_set_rtl_chip::0:1:20:0
    //e e_p3k_net_vlan_set_rtl_chip::0:20:1:0
    if(eth0_dante_tag == DEFAULT_VID)
    {
        rtk_vlan_portPvid_set(DANTE_PORT,DEFAULT_PVID,PRIORITY_SETTING);
    }

    if(eth1_p3k_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    //e e_p3k_net_vlan_set_rtl_chip::0:1:1:0
    if(eth1_p3k_tag == DEFAULT_VID && eth0_dante_tag == DEFAULT_VID)
    {
        return;
    }

    //e e_p3k_net_vlan_set_rtl_chip::0:20:20:0
    if(eth1_p3k_tag == eth0_dante_tag)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8367_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,DANTE_PORT);
        rtk_vlan_set(eth1_p3k_tag,&vlan2);
        rtk_vlan_portPvid_set(DANTE_PORT,eth1_p3k_tag,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);

        RTK_PORTMASK_PORT_SET(vlan3.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan3.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan3);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
        return;
    }

    //e e_p3k_net_vlan_set_rtl_chip::0:20:30:0
    if(eth0_dante_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8367_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,DANTE_PORT);
        rtk_vlan_set(eth0_dante_tag,&vlan2);
        rtk_vlan_portPvid_set(DANTE_PORT,eth0_dante_tag,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    if(eth1_p3k_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan3.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan3.mbr,AST1520_PORT);
        rtk_vlan_set(eth1_p3k_tag,&vlan3);

        RTK_PORTMASK_PORT_SET(vlan4.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan4.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan4);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

static void eth0_p3k_eth1_dante_vlan_settings(unsigned int eth0_p3k_tag,unsigned int eth1_dante_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2,vlan3,vlan4;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan3, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan4, 0x00, sizeof(rtk_vlan_cfg_t));
    
    //e e_p3k_net_vlan_set_rtl_chip::1:0:0:20
    //e e_p3k_net_vlan_set_rtl_chip::20:0:0:1
    //e e_p3k_net_vlan_set_rtl_chip::20:0:0:20
    //e e_p3k_net_vlan_set_rtl_chip::20:0:0:30
    if(eth0_p3k_tag == DEFAULT_VID)
    {

    }

    if(eth1_dante_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    //e e_p3k_net_vlan_set_rtl_chip::1:0:0:1
    if(eth1_dante_tag == DEFAULT_VID && eth0_p3k_tag == DEFAULT_VID)
    {
        return;
    }

    if(eth0_p3k_tag == eth1_dante_tag)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,RTL8367_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        rtk_vlan_set(eth0_p3k_tag,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,eth0_p3k_tag,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);

        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan2);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
        return;
    }

    if(eth0_p3k_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8367_ETH0_PORT);
        rtk_vlan_set(eth0_p3k_tag,&vlan2);

        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    if(eth1_dante_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
        rtk_vlan_set(eth1_dante_tag,&vlan1);
        rtk_vlan_portPvid_set(DANTE_PORT,eth1_dante_tag,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);

        RTK_PORTMASK_PORT_SET(vlan3.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan3.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan3);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

static void eth0_p3k_eth0_dante_vlan_settings(unsigned int eth0_p3k_tag,unsigned int eth0_dante_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2,vlan3,vlan4;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan3, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan4, 0x00, sizeof(rtk_vlan_cfg_t));
    //eth1 will never work
    RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
    rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
    rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
    //e e_p3k_net_vlan_set_rtl_chip::1:20:0:0
    //e e_p3k_net_vlan_set_rtl_chip::20:1:0:0
    if(eth0_p3k_tag == DEFAULT_VID)
    {

    }

    if(eth0_dante_tag == DEFAULT_VID)
    {
        rtk_vlan_portPvid_set(DANTE_PORT,DEFAULT_PVID,PRIORITY_SETTING);
    }
    //e e_p3k_net_vlan_set_rtl_chip::1:1:0:0
    if(eth0_p3k_tag == DEFAULT_VID && eth0_dante_tag == DEFAULT_VID)
    {
        return;
    }

    //e e_p3k_net_vlan_set_rtl_chip::20:20:0:0
    if(eth0_p3k_tag == eth0_dante_tag)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8367_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,DANTE_PORT);
        rtk_vlan_set(eth0_p3k_tag,&vlan2);
        rtk_vlan_portPvid_set(DANTE_PORT,eth0_dante_tag,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        return;
    }

    //e e_p3k_net_vlan_set_rtl_chip::20:30:0:0
    if(eth0_p3k_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan3.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan3.mbr,RTL8367_ETH0_PORT);
        rtk_vlan_set(eth0_p3k_tag,&vlan3);

        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }

    if(eth0_dante_tag != DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,DANTE_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8367_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.untag,DANTE_PORT);
        rtk_vlan_set(eth0_dante_tag,&vlan2);
        rtk_vlan_portPvid_set(DANTE_PORT,eth0_dante_tag,PRIORITY_SETTING);

        rtk_vlan_tagMode_set(RTL8367_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(DANTE_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

void a30_rtl8367_vlan_setting(vlan_setting_param *vlan_settings)
{
    rtk_vlan_init();
    rtk_vlan_cfg_t vlan1;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.mbr,RTL8367_ETH0_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.mbr,DANTE_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.untag,RTL8367_ETH0_PORT); 
    RTK_PORTMASK_PORT_SET(vlan1.untag,AST1520_PORT);
    if(vlan_settings->eth0.dante_port_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.untag,DANTE_PORT);
    }
    rtk_vlan_set(DEFAULT_VID,&vlan1);
    rtk_vlan_portPvid_set(AST1520_PORT,DEFAULT_PVID,PRIORITY_SETTING);
    rtk_vlan_portPvid_set(RTL8367_ETH0_PORT,DEFAULT_PVID,PRIORITY_SETTING); 

    //eth1--p3k;    eth1--dante
    if(vlan_settings->eth0.p3k_port_tag == 0 && vlan_settings->eth0.dante_port_tag == 0)
    {
        eth1_p3k_eth1_dante_vlan_settings(vlan_settings->eth1.p3k_port_tag,vlan_settings->eth1.dante_port_tag);
    }
    //eth1--p3k;    eth0--dante
    else if(vlan_settings->eth0.p3k_port_tag == 0 && vlan_settings->eth1.dante_port_tag == 0)
    {
        eth1_p3k_eth0_dante_vlan_settings(vlan_settings->eth1.p3k_port_tag,vlan_settings->eth0.dante_port_tag);
    }
    //eth0--p3k;    eth1--dante
    else if(vlan_settings->eth1.p3k_port_tag == 0 && vlan_settings->eth0.dante_port_tag == 0)
    {
        eth0_p3k_eth1_dante_vlan_settings(vlan_settings->eth0.p3k_port_tag,vlan_settings->eth1.dante_port_tag);
    }
    //eth0--p3k;    eth0--dante
    else if(vlan_settings->eth1.p3k_port_tag == 0 && vlan_settings->eth1.dante_port_tag == 0)
    {
        eth0_p3k_eth0_dante_vlan_settings(vlan_settings->eth0.p3k_port_tag,vlan_settings->eth0.dante_port_tag);
    }
    else
    {

    }
}

static void eth1_p3k_vlan_setting(unsigned int eth1_p3k_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));

    //e e_p3k_net_vlan_set_rtl_chip::0:0:1:0
    if(eth1_p3k_tag == DEFAULT_VID)
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
    //e e_p3k_net_vlan_set_rtl_chip::0:0:20:0
    else
    {
        RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
        rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
        rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);

        RTK_PORTMASK_PORT_SET(vlan2.mbr,ETH1_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        rtk_vlan_set(eth1_p3k_tag,&vlan2);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(ETH1_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

static void eth0_p3k_vlan_setting(unsigned int eth0_p3k_tag)
{
    rtk_vlan_cfg_t vlan1,vlan2;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));

    RTK_PORTMASK_PORT_SET(vlan1.mbr,ETH1_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.untag,ETH1_PORT);
    rtk_vlan_set(HIDE_VLAN_VID,&vlan1);
    rtk_vlan_portPvid_set(ETH1_PORT,HIDE_VLAN_PVID,PRIORITY_SETTING);

    //e e_p3k_net_vlan_set_rtl_chip::1:0:0:0
    if(eth0_p3k_tag == DEFAULT_VID)
    {

    }
    //e e_p3k_net_vlan_set_rtl_chip::20:0:0:0
    else
    {
        RTK_PORTMASK_PORT_SET(vlan2.mbr,RTL8364_ETH0_PORT);
        RTK_PORTMASK_PORT_SET(vlan2.mbr,AST1520_PORT);
        rtk_vlan_set(eth0_p3k_tag,&vlan2);
        rtk_vlan_tagMode_set(RTL8364_ETH0_PORT,VLAN_TAG_MODE_ORIGINAL);
        rtk_vlan_tagMode_set(AST1520_PORT,VLAN_TAG_MODE_ORIGINAL);
    }
}

void a30_rtl8364_vlan_setting(vlan_setting_param *vlan_settings)
{
    rtk_vlan_init();
    rtk_vlan_cfg_t vlan1,vlan2,vlan3,vlan4;
    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan3, 0x00, sizeof(rtk_vlan_cfg_t));
    memset(&vlan4, 0x00, sizeof(rtk_vlan_cfg_t));

    RTK_PORTMASK_PORT_SET(vlan1.mbr,AST1520_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.mbr,RTL8364_ETH0_PORT);
    RTK_PORTMASK_PORT_SET(vlan1.untag,RTL8364_ETH0_PORT); 
    RTK_PORTMASK_PORT_SET(vlan1.untag,AST1520_PORT);
    rtk_vlan_set(DEFAULT_VID,&vlan1);
    rtk_vlan_portPvid_set(AST1520_PORT,DEFAULT_PVID,PRIORITY_SETTING);
    rtk_vlan_portPvid_set(RTL8364_ETH0_PORT,DEFAULT_PVID,PRIORITY_SETTING); 

    if(vlan_settings->eth0.p3k_port_tag == 0)
    {
        eth1_p3k_vlan_setting(vlan_settings->eth1.p3k_port_tag);
    }
    else
    {
        eth0_p3k_vlan_setting(vlan_settings->eth0.p3k_port_tag);
    }
}