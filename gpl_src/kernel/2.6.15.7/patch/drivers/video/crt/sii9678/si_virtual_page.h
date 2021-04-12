/******************************************************************************
 *
 * Copyright 2013, Silicon Image, Inc.  All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of: Silicon Image, Inc., 1140
 * East Arques Avenue, Sunnyvale, California 94085
 *
 *****************************************************************************/
/**
 * @file si_virtual_page.h
 *
 * @brief Definition of the Foo API.
 *
 *****************************************************************************/

#ifndef __SI_VIRTUAL_PAGE_H__
#define __SI_VIRTUAL_PAGE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define BIT0_MASK   0x01
#define BIT1_MASK   0x02
#define BIT2_MASK   0x04
#define BIT3_MASK   0x08
#define BIT4_MASK   0x10
#define BIT5_MASK   0x20
#define BIT6_MASK   0x40
#define BIT7_MASK   0x80

#define CBUS_LOCAL_DEVCAP_LEN               16
#define CBUS_RCV_MSG_FIFO_SIZE              50
#define CBUS_RCV_MSG_FIFO_DCAP_OFFSET       0
#define CBUS_RCV_MSG_FIFO_DCAP_LEN_MAX      32
#define CBUS_RCV_MSG_FIFO_WRBURST_OFFSET    32
#define CBUS_RCV_MSG_FIFO_WRBURST_LEN_MAX   16
#define CBUS_RCV_MSG_FIFO_SUB_CMD_OFFSET    48
#define CBUS_RCV_MSG_FIFO_SUB_CMD_LEN       1
#define CBUS_RCV_MSG_FIFO_RCPE_ERR_CODE_OFFSET 49
#define CBUS_RCV_MSG_FIFO_RCPE_ERR_CODE_LEN 1
#define HDCP_RX_ID_FIFO_LEN 0x05
#define HDCP_MAX_K 0x04

/*****************************************************************************
*
*           Virtual Page Registers Mapping
*
******************************************************************************
*   |0x00--0x0F| => General Group
*   |0x10--0x1F| => Backup For General Group expansion
*
*   |0x20--0x27| => Tx Group
*   |0x28--0x2F| => Backup for Tx Group expansion
*
*   |0x30--0x37| => Rx Group
*   |0x38--0x3F| => Backup for Rx Group expansion
*
*   |0x40--0x6F| => HDCP Group
*   |0x70--0x7F| => Backup for HDCP Group expansion
*
*   |0x80--0x9F| => CBUS Group
*   |0xA0--0xAF| => Backup for CBUS Group expansion
*
*   |0xB0--0xCF| => CEC Group
*   |0xD0--0xDF| => Backup for CEC Group expansion
*
*   |0xE0--0xE7| => EDID Group
*   |0xE8--0xEF| => Backup for EDID Group expansion
*
*   |0xF0--0xF7| => Backup for ISP Group expansion
*   |0xF8--0xFF| => ISP Group
******************************************************************************/



/********************************************************
*
*                   General Group
*
*********************************************************/
#define FW_VER_L        0x00                              /*R, Firmware version ID low*/
#define FW_VER_H        0x01                              /*R, Firmware version ID hight*/
#define CHIP_ID_LOW     0x02                              /*R, Chip ID low*/
#define CHIP_ID_HIGH    0x03                              /*R, Chip ID high*/
#define REV_ID          0x04                              /*R, Revision number*/

#define CHIP_GENERAL_INT_STATUS 0x05                   /*R, 0x06[7:6]:Connection state of upstream/downstream link
                                                                      Together with INT0.1, to implement Receiver
                                                                      Connected/Disconnected indication for Rx)
                                                                      00:No connection
                                                                      01:HDMI is connected
                                                                      10:MHL is connected
                                                         R, 0x06[5:4]: VBUS request status:
                                                                      00: No pending request
                                                                      01: Request to provide VBUS power
                                                                      10: Request to remove VBUS power
                                                                      11: reserved
                                                         R, 0x06[3:2]: Boot State Machine Status. (using together with INT0_0).
                                                                      00:Boot in progress
                                                                      01:Boot Done;
                                                                      10:Boot Error, in this case virtual page does not work
                                                                      11:RSVD
                                                         R, 0x06[1]:CKDT state of the chip. (Using together with INT0.2)
                                                         R, 0X06[0]:SCDT state of the chip. (Using together with INT0.3)*/

#define CONN_STATE_MASK (BIT7_MASK|BIT6_MASK)
#define CON_TYPE_NONE 0x00
#define CON_TYPE_HDMI BIT6_MASK
#define CON_TYPE_MHL  BIT7_MASK

#define VBUS_REQUEST_STATUS_MASK (BIT5_MASK|BIT4_MASK)
#define VBUS_NO_PENDING             0x00
#define VBUS_REQUEST_PROVIDE_POWER  BIT4_MASK
#define VBUS_REQUEST_REMOVE_POWER   BIT5_MASK
#define VBUS_REQUEST_RSVD           (BIT4_MASK|BIT5_MASK)

#define BSM_MASK        (BIT3_MASK|BIT2_MASK)
#define BOOT_INPROGRESS 0x00
#define BOOT_SUCCESS    BIT2_MASK
#define BOOT_FAILURE    BIT3_MASK

#define CKDT_STATE_MASK BIT1_MASK
#define SCDT_STATE_MASK BIT0_MASK
#define OLD_CHIP_GENERAL_INT_STATUS 0x06

#define TMDS_SWING_LEVEL 0x06                           /*R, 0x07[7:5]:Control the TMDS swing level of the chip.*/
#define SWING_LEVEL_MASK (BIT7_MASK|BIT6_MASK|BIT5_MASK|BIT4_MASK)





#define INT_STATE 0x07                                  /*R, 0x70[0]: interrupt pin state of the chip*/
#define INT_STATE_MASK      BIT0_MASK
#define INT_POLARITY_MASK   BIT1_MASK
#define INT_OD_MASK         BIT2_MASK

#define CHIP_GENERAL_INT 0x08            /*R/C, 0x71[0]: hardware boot done interrupt
                                                0x71[1]: upstream/downstream connection change interrupt
                                                0x71[2]: CKDT change interrupt
                                                0x71[3]: SCDT change interrupt
                                                0x71[4]: CBUS event interrupt
                                                0x71[5]: CBUS status chg interrupt
                                                0x71[6]: VBUS power Request interrupt
                                                0x71[7]: cable detection interrupt*/

#define HW_BOOT_DONE_INT    BIT0_MASK
#define RX_CONN_CHG_INT     BIT1_MASK
#define CKDT_CHG_INT        BIT2_MASK
#define SCDT_CHG_INT        BIT3_MASK
#define CBUS_EVENT_INT      BIT4_MASK
#define CBUS_STATUS_CHG_INT BIT5_MASK
#define VBUS_REQ_INT        BIT6_MASK
#define CD_SENSE_INT        BIT7_MASK

#define CHIP_GENERAL_INT_MASK 0x09       /*R/W, CHIP_GENERAL_INT register MASK*/
#define HW_BOOT_DONE_INT_MASK    BIT0_MASK
#define CONN_CHG_INT_MASK        BIT1_MASK
#define CKDT_CHG_INT_MASK        BIT2_MASK
#define SCDT_CHG_INT_MASK        BIT3_MASK
#define CBUS_EVENT_INT_MASK      BIT4_MASK
#define LINK_STATUS_CHG_INT_MASK BIT5_MASK
#define VBUS_REQ_INT_MASK        BIT6_MASK
#define CD_SENSE_INT_MASK        BIT7_MASK

#define AUDIO_INT           0x0A                           /*R/C bit[0]:  Audio Set Mode received*/
#define AUDIO_OUTPUT_MODE_INT_MASK BIT0_MASK

#define AUDIO_INT_MASK      0x0B                           /*R/W bit[0]:  Audio Set Mode received mask*/

#define AUDIO_OUTPUT_MODE   0x0C                           /*R/W bit[1:0]:  00: I2S
                                                                            01: TDM
                                                                            10: S/PDIF
                                                                            11: Reserved*/

#define GPIO_CTRL 0x0D
#define GPIO_NUM_MASK (BIT0_MASK|BIT1_MASK|BIT2_MASK)
#define GPIO_R_W_CTRL_MASK (BIT3_MASK)
#define GPIO_READ_CTRL BIT3_MASK
#define GPIO_WRITE_CTRL 0
#define GPIO_POW_LEVEL_MASK (BIT7_MASK)
#define GPIO_POW_HIGH BIT7_MASK
#define GPIO_POW_LOW 0

#define FW_CUSTOMER_SUB_VER 0x0E

#define MODE_CONFIG 0x0F                               /* R/W, 0x0F bit[7]: Rogue/Sydney mode congiguration.
                                                                        0:normal mode;
                                                                        1:debug mode
                                                                    bit[6]: Chip Host mode
                                                                        0: stand alone mode
                                                                        1: Host mode*/
#define CHIP_MODE_CONFIG_MASK BIT7_MASK
#define CHIP_HOST_MODE_MASK   BIT6_MASK
#define CHIP_TEST_MODE_MASK   BIT5_MASK
#define CHIP_AVR_MODE_MASK    BIT4_MASK
#define CHIP_FW_START_MODE    BIT3_MASK

#define GPIO_READ 0x10

/********************************************************
*
*                   Tx Group
*
*********************************************************/
#define AV_CTRL 0x20
#define AV_CTRL_MUTE_MASK (BIT0_MASK|BIT1_MASK|BIT2_MASK)
#define AV_CTRL_MUTE_OFF         0x00
#define AV_CTRL_AV_MUTE_OFF      BIT0_MASK
#define AV_CTRL_AV_MUTE_ON       BIT1_MASK
#define AV_CTRL_MUTE_VIDEO       BIT2_MASK
#define AV_CTRL_MUTE_AUDIO       (BIT0_MASK|BIT1_MASK)
#define AV_CTRL_MUTE_AUDIO_VIDEO (BIT0_MASK|BIT2_MASK)
//#define AV_CTRL_MUTE_BYPASS      (BIT1_MASK|BIT2_MASK)

#define TMDS_OUTPUT_CTRL_MASK   (BIT4_MASK | BIT5_MASK)
#define TMDS_OUTPUT_ENABLE      BIT4_MASK
#define TMDS_OUTPUT_DISABLE     BIT5_MASK
#define TMDS_OUTPUT_RSVD        (BIT4_MASK | BIT5_MASK)

#define MISC_STATUS 0x21
#define TX_AV_MUTE_STATUS_MASK (BIT0_MASK)
#define TX_AV_MUTE_STATUS_ON 0
#define TX_AV_MUTE_STATUS_OFF (BIT0_MASK)

#define TX_GENERAL_INT  0x22
#define TX_CONN_ON_INT BIT0_MASK
#define TX_RXSENSE_CHG_INT BIT1_MASK
#define TX_CONN_OFF_INT BIT2_MASK

#define TX_GENERAL_STAT 0x23
#define TX_LINK_TYPE_MASK (BIT0_MASK|BIT1_MASK)
#define TX_LINK_TYPE_DISCONN 0x00
#define TX_LINK_TYPE_HDMI  BIT0_MASK
#define TX_LINK_TYPE_MHL12 BIT1_MASK
#define TX_LINK_TYPE_MHL3  (BIT0_MASK|BIT1_MASK)
#define TX_RXSENSE_STATUS_MASK BIT2_MASK
#define TX_NO_RXSENSED 0
#define TX_RXSENSED BIT2_MASK

#define TX_GENERAL_INT_MASK 0x24
#define TX_CONN_ON_INT_MASK BIT0_MASK
#define TX_RXSENSE_CHG_MASK BIT1_MASK
#define TX_CONN_OFF_INT_MASK BIT2_MASK

/********************************************************
*
*                   Rx Group
*
*********************************************************/
#define RX_CTRL 0x30                                   /*R/W, 0x0A[1:0] Termination Control */
#define TERM_CTRL_MASK (BIT0_MASK|BIT1_MASK)
#define TERM_CTRL_BYPASS                   0x00
#define TERM_CTRL_DISABLE                  BIT0_MASK
#define TERM_CTRL_ENABLE                   BIT1_MASK
#define TERM_CTRL_RSVD                     (BIT1_MASK | BIT0_MASK)

#define RX_EDID_DDC_CTRL_MASK        (BIT2_MASK | BIT3_MASK)
#define RX_EDID_DDC_CTRL_BYPASS      0x00
#define RX_EDID_DDC_CTRL_DISABLE     BIT2_MASK
#define RX_EDID_DDC_CTRL_ENABLE      BIT3_MASK
#define RX_EDID_DDC_CTRL_RSVD        (BIT2_MASK | BIT3_MASK)

#define RX_VBUS_CTRL_MASK (BIT4_MASK | BIT5_MASK)
#define RX_VBUS_CTRL_BYPASS     0x00
#define RX_VBUS_CTRL_DISABLE    BIT4_MASK
#define RX_VBUS_CTRL_ENABLE     BIT5_MASK
#define RX_VBUS_CTRL_RSVD       (BIT4_MASK | BIT5_MASK)

#define RX_HPD_CTRL_MASK (BIT7_MASK|BIT6_MASK)
#define RX_HPD_CTRL_LOW 0x00
#define RX_HPD_CTRL_HIGH  BIT6_MASK
#define RX_HPD_CTRL_TRISTATE BIT7_MASK
#define RX_HPD_CTRL_RSVD (BIT7_MASK|BIT6_MASK)

#define RX_GENERAL_STATE 0x31                                   /*R, connection status*/
#define RX_LINK_STATUS_MASK (BIT0_MASK|BIT1_MASK)
#define NO_CONNECTED    0x00
#define HDMI_CONNECTED  0x01
#define MHL12_CONNECTED 0x02
#define MHL3_CONNECTED  0x03

#define CD_SENSE_STATUS     BIT2_MASK
#define NO_CABLE_DETECTED 0x00
#define CABLE_DETECTED    BIT2_MASK
#define PWR5V_STATE_MASK  BIT3_MASK                         /* R, 0x0B[3]: State of the Rx port's 5V power detect pin.*/

#define RX_GENERAL_INT    0x32
#define RX_AV_MUTE_STATUS_CHG_INT BIT0_MASK

#define RX_GENERAL_INT_MASK 0x33

#define RX_RESOLUTION_H_ACTIVE_LOW  0x34
#define RX_RESOLUTION_H_ACTIVE_HIGH 0x35
#define RX_RESOLUTION_V_ACTIVE_LOW  0x36
#define RX_RESOLUTION_V_ACTIVE_HIGH 0x37


/********************************************************
*
*                   HDCP Group
*
*********************************************************/
#define HDCP_CTRL_1                 0x40                  /*R/W: bit[0]: Rx receiver id list valid set*/
#define HDCP_RX_RECEIVER_ID_LIST_VALID_MASK BIT0_MASK
#define HDCP_RX_SET_TOPOLOGY_START          BIT1_MASK
#define HDCP_RX_SET_SMNG_START              BIT2_MASK
#define HDCP_RX_SET_RCVID_LIST_START        BIT3_MASK
#define RX_DDC_CTRL_MASK               (BIT4_MASK|BIT5_MASK)
#define RX_DDC_CTRL_BYPASS             0x00
#define RX_DDC_CTRL_DISABLE            BIT4_MASK
#define RX_DDC_CTRL_ENABLE             BIT5_MASK
#define HDCP_RX_RESET_RI_ENABLE             BIT6_MASK
#define HDCP_RX_HDCP2X_CAP_ENABLE_MASK      BIT7_MASK    //1: enable HDCP2X capability, 0: disable HDCP2X capability
#define HDCP_RX_HDCP2X_CAP_DISABLE          0x00
#define HDCP_RX_HDCP2X_CAP_ENABLE           BIT7_MASK

#define HDCP_RX_TOPOLOGY_MISC       0x41                /*R: bit[0]: HDCP 1.X compliant device in the topology if set to true, HDCP 2.X use only
                                                             bit[1]: HDCP 2.0 compliant device in the topology if set to true, HDCP 2.X use only
                                                             bit[2]: More than seven level for HDCP 1.X or four levels for HDCP 2.X of
                                                                     repeaters cascaded together if set to true
                                                             bit[3]: More than 31 devices (for HDCP1.X) or 127 devices (for HDCP 2.X) as attached if set to true*/
#define HDCP_RX_TOPOLOGY_MISC_MASK  (BIT0_MASK|BIT1_MASK|BIT2_MASK|BIT3_MASK)
#define HDCP1_TOPOLOGY_DEV_DS_MASK       BIT0_MASK
#define HDCP20_TOPOLOGY_DEV_DS_MASK      BIT1_MASK
#define TOPOLOGY_MAX_CASCADE_EXCEEDED    BIT2_MASK
#define TOPOLOGY_MAX_DEVS_EXCEEDED       BIT3_MASK

#define HDCP_RX_TOPOLOGY_DEV_COUNT      0x42            /*R: Total number of attached downstream devices*/
#define HDCP_RX_TOPOLOGY_DEPTH          0x43            /*R: Repeater cascade depth*/
#define HDCP_RX_TOPOLOGY_SEQ_NUM_V_FIFO 0x44            /*R: seq_num_V value, HDCP 2.X use only*/

#define HDCP_RX_SMNG_SEQ_NUM_M_FIFO     0x45
#define HDCP_RX_SMNG_VALUE_K_L          0x46
#define HDCP_RX_SMNG_VALUE_K_H          0x47
#define HDCP_RX_MAX_K 0x04
#define HDCP_RX_SMNG_STREAMID_TYPE_FIFO 0x48

#define HDCP_RX_ID_LIST_OFFSET_L 0x49
#define HDCP_RX_ID_LIST_OFFSET_H 0x4A

#define HDCP_RX_ID_LIST_BKSV_NUM    0x4B
#define HDCP_RX_ID_LIST_FIFO_SIZE_L 0x4C
#define HDCP_RX_ID_LIST_FIFO_SIZE_H 0x4D
#define HDCP_RX_ID_LIST_FIFO        0x4E

#define HDCP_RX_INT                 0x4F
#define HDCP_RX_INT_AVAILABLE_MASK    (BIT0_MASK|BIT1_MASK|BIT2_MASK|BIT3_MASK|BIT4_MASK|BIT5_MASK)
#define HDCP_RX_AUTH_DONE_INT           BIT0_MASK
#define HDCP_RX_RCVID_DONE_INT          BIT1_MASK
#define HDCP_RX_SMNG_CHG_INT            BIT2_MASK
#define HDCP_RX_AUTH_FAIL               BIT3_MASK
#define HDCP_RX_AUTH_START              BIT4_MASK
//#define HDCP_RX_STATUS_CHG_INT          BIT5_MASK

#define HDCP_RX_INT_MASK            0x50
#define HDCP_RX_AUTH_DONE_INT_MASK  (BIT0_MASK)
#define HDCP_RX_RCVID_DONE_INT_MASK (BIT1_MASK)
#define HDCP_RX_SMNG_CHG_INT_MASK   (BIT2_MASK)
#define HDCP_RX_AUTH_FAIL_MASK      (BIT3_MASK)
#define HDCP_RX_AUTH_START_MASK     (BIT4_MASK)


#define HDCP_STATE  0x51                  /*R, 0x31[7:6]: indicate Tx Downstream HDCP authentication state
                                                 00: HDCP STATUS OFF
                                                 01: AUTHENTICATION done
                                                 10: AUTHENTICATION in progress
                                                 11: AUTHENTICATION FAIL*/
                                          /*R, 0x31[5]: 1: receiver ID changed*/
                                          /*R, 0x31[4]: 1: stream manage msg done*/
                                          /*R, 0x31[3]: 1: authentication done*/
                                          /*R, 0x31[2]: 1: Rx Hdcp receiver id list valid
                                                        0: Rx Hdcp receiver id list invalid*/
                                          /*R/W, 0x31[1:0]: HDCP link version of the Upstream.
                                                     00: no HDCP
                                                     01: HDCP 1.X
                                                     10: HDCP 2.0
                                                     11: HDCP2.2*/
#define HDCP_TX_AUTH_STATE_MASK (BIT7_MASK|BIT6_MASK)
#define HDCP_TX_STATUS_OFF              0x00
#define HDCP_TX_STATUS_AUTHENTICATED    BIT6_MASK
#define HDCP_TX_STATUS_AUTHENTICATING   BIT7_MASK
#define HDCP_TX_STATUS_FAIL             (BIT6_MASK|BIT7_MASK)

#define HDCP_RCVID_CHG_MASK (BIT5_MASK)
#define HDCP_SMNG_DONE_MASK (BIT4_MASK)
#define HDCP_AUTH_DONE_MASK (BIT3_MASK)

#define HDCP_RX_RECEIVER_ID_LIST_VALID_STATUS_MASK BIT2_MASK

#define HDCP_RX_US_LINK_MASK (BIT1_MASK|BIT0_MASK)
#define LINK_NOHDCP     0x00
#define LINK_HDCP_1x    0x01
#define LINK_HDCP_2     0x02
#define LINK_HDCP_2x    0x03

#define HDCP_RX_STATE_MACHINE 0x52                        /*R, indicate current HDCP state include every step of
                                                               HDCP authentication*/
#define RX_HDCP_AUTH_STATE_MASK (BIT0_MASK|BIT1_MASK)
#define HDCP_RX_STATUS_OFF              0x00
#define HDCP_RX_STATUS_AUTHENTICATED    BIT0_MASK
#define HDCP_RX_STATUS_AUTHENTICATING   BIT1_MASK
#define HDCP_RX_STATUS_FAIL             (BIT0_MASK|BIT1_MASK)

#define HDCP_ERR_REASON 0x53                              /*R, HDCP failure reason code, could be read when
                                                               HDCP_AUTH is unsuccessful*/

#define HDCP_TX_FAIL_REASON_NONE      0x00               /*No failure detected so far*/
#define HDCP_TX_FAIL_REASON_INTERNAL (BIT0_MASK)
#define HDCP_TX_FAIL_REASON_CAS_EXC  (BIT1_MASK)         /*Cascade exceeded error; firmware does not try until HPD Low to High transition is done*/
#define HDCP_TX_FAIL_REASON_DEV_EXC  (BIT2_MASK)         /*Too many devices; firmware does not try until HPD Low to High transition*/
//not support now
#define HDCP_TX_FAIL_REASON_NACK     (BIT3_MASK)                    /*Downstream device does not acknowledge HDCP registers; firmware continues trying*/
#define HDCP_TX_FAIL_REASON_V        (BIT4_MASK)                       /*V verification failed; firmware continues trying*/
#define HDCP_TX_FAIL_REASON_TIMEOUT  (BIT5_MASK)                 /*Authentication timed out; firmware continues trying*/
#define HDCP_TX_FAIL_REASON_OTHER    (BIT6_MASK)


#define HDCP_CTRL 0x54                                    /*R/W, 0x32[0]: Content stream type transmitter to downstream.
                                                                          0: Type 0 Content Streams
                                                                          1: Type 1 Content Streams
                                                                          This register is only valid when downstream is
                                                                          HDCP2.2 repeater
                                                            R,   0x32[2]: Indicate the downstream device is
                                                                          repeater/converter or not
                                                            R,   0x32[4:3]: Indicate if there is an HDCP1.X/HDCP2.0 device
                                                                            in downstream
                                                                            00: Downstream have no HDCP1.X and HDCP2.0 devices.
                                                                            01: Downstream have HDCP1.X devices
                                                                            10: Downstream have HDCP2.0 devices
                                                                            11: Downstream have both HDCP1.x and
                                                                            HDCP2.0 devices
                                                                            This register maybe changed at HDCPINT.1 coming.
                                                                            This register is only effect when downstream is repeater
                                                                            and HDCP link version is HDCP2.2.
                                                              R/W 0x32[5]: 0:TX HDCP disable
                                                                           1:TX HDCP enable
                                                              R/W 0x32[6]: 0: set stream manage completed
                                                                           1: start to set stream manage
                                                              R/W:   0x32[7]: 0: indicate set completed or default value
                                                                            1: start to set repeater bit*/
#define HDCP_CONTENT_TYPE_MASK BIT0_MASK
#define HDCP_CONTENT_TYPE0 0x00
#define HDCP_CONTENT_TYPE1 0x01

#define HDCP_REPEATER_MASK BIT2_MASK
#define HDCP_DEVICE_DOWNSTREAM (BIT3_MASK|BIT4_MASK)

#define HDCP_TX_EN_MASK BIT5_MASK
#define HDCP_DIS    0x00
#define HDCP_EN     BIT5_MASK

#define HDCP_SMNG_SET_START BIT6_MASK
#define HDCP_SET_REPEATER_BIT_MASK BIT7_MASK

#define HDCP_RCV_ID_FIFO_LEN 0x55                           /*R/W: ksv list of this len will be read*/
#define HDCP_RCV_ID_FIFO_OFFSET_L 0x56                        /*R/W: offset of ksv list. 5 byte as a cell*/
#define HDCP_RCV_ID_FIFO_OFFSET_H 0x57

//#define HDCP_RCV_ID_FIFO_COUNT_MASK (BIT0_MASK|BIT1_MASK|BIT2_MASK|BIT3_MASK|BIT4_MASK)
//#define HDCP_RCV_ID_FIFO_LAST_MASK BIT7_MASK

#define HDCP_RCV_ID_FIFO 0x58                             /*R, DeviceID list of the downstream
                                                               (This register is implement as a read-only FIFO)*/
#define HDCP_READ   0x59
#define READ_BKSV_MASK BIT0_MASK
#define READ_RCVID_MASK BIT1_MASK
#define READ_RX_ID_MASK BIT2_MASK
#define READ_AKSV_MASK BIT3_MASK
#define READ_RX_SMNG_MASK BIT4_MASK
#define READ_RX_BKSV_MASK BIT5_MASK

#define HDCP_RX_ID_FIFO  0x5A
#define HDCP_AKSV_FIFO 0x5B

#define HDCP_RX_INFO_MISC 0x5C               /*R: bit[0]: HDCP 1.X compliant device in the topology if set to true, HDCP 2.X use only
                                                  bit[1]: HDCP 2.0 compliant device in the topology if set to true, HDCP 2.X use only
                                                  bit[2]: More than seven level for HDCP 1.X or four levels for HDCP 2.X of repeaters cascaded together if set to true
                                                  bit[3]: More than 31 devices (for HDCP1.X) or 127 devices (for HDCP 2.X) as attached if set to true*/
#define HDCP_RX_INFO_MISC_MASK (BIT0_MASK|BIT1_MASK|BIT2_MASK|BIT3_MASK)
#define HDCP1_DEV_DS_MASK BIT0_MASK
#define HDCP20_DEV_DS_MASK BIT1_MASK
#define MAX_CASCADE_EXCEEDED BIT2_MASK
#define MAX_DEVS_EXCEEDED BIT3_MASK

#define HDCP_RX_INFO_DEV_COUNT 0x5D          /*R: Total number of attached downstream devices*/
#define HDCP_RX_INFO_DEPTH 0x5E              /*R: Repeater cascade depth*/
#define HDCP_RX_INFO_SEQ_NUM_V_FIFO 0x5F          /*R: seq_num_V value, HDCP 2.X use only*/

#define HDCP_TX_STATE            0X60
#define HDCP_TX_LINK_TYPE_MASK  (BIT0_MASK|BIT1_MASK)
#define HDCP_TX_LINK_NOHDCP     0x00
#define HDCP_TX_LINK_HDCP_1x    BIT0_MASK
#define HDCP_TX_LINK_HDCP_2     BIT1_MASK
#define HDCP_TX_LINK_HDCP_2x    (BIT0_MASK|BIT1_MASK)

#define HDCP_SMNG_SEQ_NUM_M_FIFO 0x61
#define HDCP_SMNG_VALUE_K_L 0x62
#define HDCP_SMNG_VALUE_K_H 0x63
#define HDCP_SMNG_STREAMID_TYPE_FIFO 0x64

#define HDCP_TX_INT 0x65
#define HDCP_TX_INT_AVAILABLE_MASK    (BIT2_MASK|BIT3_MASK|BIT4_MASK|BIT5_MASK|BIT6_MASK)
#define HDCP_TX_AUTH_FAIL_INT    (BIT6_MASK)
#define HDCP_TX_RCVID_CHG_INT    (BIT5_MASK)
#define HDCP_TX_SMNG_DONE_INT    (BIT4_MASK)
#define HDCP_TX_AUTH_DONE_INT    (BIT3_MASK)
#define HDCP_TX_STATUS_CHG_INT   (BIT2_MASK)

#define HDCP_TX_INT_MASK 0x66                                /*Mask HDCP interrupt*/
#define HDCP_TX_AUTH_FAIL_MASK   (BIT6_MASK)
#define HDCP_TX_RCVID_CHG_MASK   (BIT5_MASK)
#define HDCP_TX_SMNG_DONE_MASK   (BIT4_MASK)
#define HDCP_TX_AUTH_DONE_MASK   (BIT3_MASK)
#define HDCP_TX_STATUS_CHG_MASK  (BIT2_MASK)

#define HDCP_RX_BKSV_FIFO 0x67


/********************************************************
*
*                   CBUS Group
*
*********************************************************/
#define CBUS_INT 0x80                                      /*R/C, 0x73[0]: CBUS MSC Message received (RCP, RAP, UCP received)
                                                                  0x73[1]: CBUS Remote DEVCAP changed/received (DEVCAP received)
                                                                  0x73[2]: CBUS Feedback Received (RCPK/E,UCPK/E,RAPK received)
                                                                  0x73[3]: CBUS MSC sending failed (RCP,UCP,RAP send error in link level-cbus busy)
                                                                  0x73[4]: CBUS Message send successful
                                                                  0x73[5]: CBUS Notify DEVCAP changed failed
                                                                  0x73[6]: CBUS Notify DEVCAP changed successful
                                                                  0x73[7]: CBUS Write burst received*/


#define CBUS_MSC_MSG_RCVD_MASK              BIT0_MASK
#define CBUS_DEVCAP_RCVD_MASK               BIT1_MASK
#define CBUS_FEEDBACK_RCVD_MASK             BIT2_MASK
#define CBUS_MSC_SENDING_FAILED_MASK        BIT3_MASK
#define CBUS_MSG_SENDING_SUCCESS_MASK       BIT4_MASK
#define CBUS_NOTIFY_DEVCAP_FAILED_MASK      BIT5_MASK
#define CBUS_NOTIFY_DEVCAP_SUCCESS_MASK     BIT6_MASK
#define CBUS_WRITE_BURST_RCVD_MASK          BIT7_MASK


#define CBUS_INT_MASK 0x81                                 /*R/W, mask CBUS_INT*/

#define CBUS_LOCAL_DEVCAP_OFFSET 0x82
#define CBUS_LOCAL_DEVCAP_FIFO 0x83

#define CBUS_STATUS   0x84                      /*R, bit[0:1] CBUS status*/
#define CBUS_STATUS_MASK (BIT0_MASK|BIT1_MASK)
#define CBUS_STATUS_RSVD 0x00
#define CBUS_BUSY BIT0_MASK
#define CBUS_IDLE BIT1_MASK
#define CBUS_ABORT (BIT0_MASK|BIT1_MASK)

#define CBUS_MSG_CMD_CTRL 0x85                            /*W, 0xB8[0]: Start to send out CBUS message*/
#define CBUS_MSG_CMD_START_MASK BIT0_MASK
#define CBUS_MSG_RCVD_RCPE_AVAILABLE BIT1_MASK            /*R/W, 1: received RCPE/UCPE/RBPE status code(error code).
                                                                 0: not received error code refer to CBUS_RCV_MSG_FIFO mapping*/

#define CBUS_SEND_MSG_TYPE 0x86                            /*W, Choose the cbus message type to send out(RAP/RCP/UCP/MDT/DCAP)*/
#define CBUS_SEND_MSG_FIFO 0x87                            /*W, FIFO for send out cbus messages. The maximum length is 17 bytes,
                                                                used in MDT message, first byte for burst length and other 16 bytes
                                                                burst data.*/

#define CBUS_RCV_MSG_TYPE       0x88            /*R, Recived cbus message type (RAP/RCP/UCP/MDT)
                                                     CBUS_RCP   0x10
                                                     CBUS_RCPK  0x11
                                                     CBUS_RCPE  0x12
                                                     CBUS_RAP   0x20
                                                     CBUS_RAPK  0x21
                                                     CBUS_RBP   0x22
                                                     CBUS_RBPK  0x23
                                                     CBUS_RBPE  0x24
                                                     CBUS_UCP   0x30
                                                     CBUS_UCPK  0x31
                                                     CBUS_UCPE  0x32*/

/*****************FIFO Mapping:*****************************
*   | DEV_CAP   | WRITE_BURST | SUB_CMD | R*PE ERROR CODE |
*   | 0......31 | 32......47  |   48    |       49        |
************************************************************/
#define CBUS_RCV_MSG_FIFO 0x89                             /*R, FIFO for recieve cbus messages*/
#define CBUS_RCV_MSG_FIFO_OFFSET 0x8A           /*R/W, offset of Received message fifo*/
#define CBUS_SEND_ERR_REASON 0x8B                          /*R, CBUS message error reason for requester*/
#define CBUS_RCV_ERR_REASON 0x8C                           /*R, CBUS message error reason for responder*/

#define CBUS_RX_CTRL            0x8D            /*R/W, CBUS RX ctrol register*/
#define CBUS_POW_CLEAR_MASK     0x00
#define CBUS_POW_SET_MASK       BIT0_MASK

#define RX_CBUS_IMPEDANCE_MASK  (BIT3_MASK|BIT4_MASK)
#define RX_CBUS_IMPEDANCE_BYPASS 0x00
#define RX_CBUS_IMPEDANCE_1K    (BIT3_MASK)
#define RX_CBUS_IMPEDANCE_HI_Z  (BIT4_MASK)

#define RX_CBUS_LOCAL_DEVCAP_CHG_MASK BIT5_MASK
#define RX_CBUS_LOCAL_DEVCAP_CHG BIT5_MASK

// BIST APIs
#define MHL_CBUS_MODE  0x8E
#define MHL_CBUS_MODE_MASK      (BIT0_MASK|BIT1_MASK)
#define MHL_CBUS_MODE_NONE      0x00                    /*R, CBUS is not connected */
#define MHL_CBUS_MODE_OCBUS     BIT0_MASK               /*R, CBUS is in oCBus mode */
#define MHL_CBUS_MODE_ECBUS_S   BIT1_MASK               /*R, CBUS is in eCBus_s mode */
#define MHL_CBUS_MODE_OCBUS_D   (BIT0_MASK|BIT1_MASK)   /*R, CBUS is in eCBus_d mode */

#define MHL_VER        0x8F                       /*R, MHL version register
                                                       bit[1:0]:   0 - MHL link is not established
                                                                   1 - MHL1,2 is set up
                                                                   2 - MHL3 is set up
                                                                   3 - Reserverd
                                                       bit[7:2]:   Reserved for future use*/
#define MHL_VER_MASK (BIT0_MASK|BIT1_MASK)
#define NO_MHL_LINK 0x00
#define MHL_VER_12 BIT0_MASK
#define MHL_VER_3  BIT1_MASK


#define MHL3_BIST_STATUS  0x90                    /*R, MHL3 BIST status register
                                                       bit[1:0]:   0 - BIST is not started
                                                                   1 - BIST is in progress
                                                                   2 - BIST is done */
#define MHL3_BIST_STATUS_MASK (BIT0_MASK|BIT1_MASK)
#define MHL3_BIST_STATUS_NOTSTARTED 0x00
#define MHL3_BIST_STATUS_INPROGRESS BIT0_MASK
#define MHL3_BIST_STATUS_SUCCESS    BIT1_MASK
#define MHL3_BIST_STATUS_FAIL       (BIT0_MASK|BIT1_MASK)

#define MHL3_BIST_ERROR_COUNT_FIFO 0x91
#define MHL3_BIST_CONFIG_FIFO 0x92

#define MHL3_BIST_CTRL     0x93
#define MHL3_BIST_START_MASK BIT0_MASK
#define MHL3_BIST_START BIT0_MASK
#define MHL3_BIST_STOP  0x00

#define MHL3_INT 0x94
#define MHL3_BIST_STATUS_CHG BIT0_MASK
#define MHL_VER_CHG_INT BIT1_MASK
#define MHL_CBUS_MODE_CHG_INT BIT2_MASK

#define MHL3_INT_MASK 0x95
#define MHL3_BIST_STATUS_CHG_MASK BIT0_MASK
#define MHL_VER_CHG_INT_MASK BIT1_MASK
#define MHL_CBUS_MODE_CHG_INT_MASK BIT2_MASK

#define CBUS_INT_1 0x96
#define CBUS_WRITE_BURST_SUCCESS_INT BIT0_MASK
#define CBUS_WRITE_BURST_FAIL_INT BIT1_MASK

#define CBUS_INT_1_MASK 0x97

/********************************************************
*
*                   CEC Group
*
*********************************************************/
#define CEC_INT 0xB0                                       /*R/C, bit[0]:CEC command received*/
#define CEC_INT_MASK 0xB1                                  /*R/W, Mask of CEC interrupt.*/
#define CEC_CMD_RCVD_MASK BIT0_MASK

#define CEC_START_SEND_CMD 0xB2                            /*W, bit[0]: Start to send CEC command*/
#define CEC_START_SEND_CMD_MASK BIT0_MASK

#define CEC_RCV_OPCODE 0xB3                                /*R, Received CEC operation code*/
#define CEC_SEND_OPCODE 0xB4                               /*W, Sending CEC operation code*/
#define CEC_RCV_ARGS_FIFO 0xB5                             /*R, FIFO for received CEC args, max length is 16*/
#define CEC_SEND_ARGS_FIFO 0xB6                            /*W, FIFO for send out CEC args, max length is 16*/
#define CEC_RCV_SRC_DES_ADDR 0xB7                          /*R, Bit[0:3] source address
                                                                Bit[4:7] destination address*/
#define CEC_SEND_SRC_DES_ADDR 0xB8                         /*W, Bit[0:3] source address
                                                                Bit[4:7] destination address*/
#define CEC_SEND_RCV_ARGS_LEN 0xB9                         /*Bit[0:3](R) length of received CEC args
                                                             Bit[4:7] (W)length of sending CEC args*/
#define CEC_VENDOR_SET_FIFO 0xBA
#define CEC_OSD_SET_FIFO    0xBB
#define CEC_RX_COUNT        0xBC
#define CEC_RX_CMD_HEADER   0xBD
#define CEC_RX_CMD_OPCODE   0xBE
#define CEC_RCVD_CMD_FIFO   0xBF
#define CEC_DEST_ID         0xC0
#define CEC_TX_OPCODE       0xC1
#define CEC_SEND_CMD_FIFO   0xC2
#define CEC_SEND_CMD_BYTE_CNT       0xC3
#define CEC_SEND_ARGS_LEN_MASK (BIT4_MASK|BIT5_MASK|BIT6_MASK|BIT7_MASK)
#define CEC_RCV_ARGS_LEN_MASK (BIT0_MASK|BIT1_MASK|BIT2_MASK|BIT3_MASK)

/********************************************************
*
*                   EDID Group
*
*********************************************************/
#define EDID_INT            0xE0                            /*R/C Bit[0]: EDID changed interrupt*/
#define EDID_CHG_INT_MASK   BIT0_MASK

#define EDID_INT_MASK       0xE1                            /*R/W Bit[0]: EDID changed interrupt mask*/
#define EDID_OFFSET_L       0xE2                            /*R/W the length of EDID*/
#define EDID_OFFSET_H       0xE3                            /*R/W the length of EDID*/
#define EDID_FIFO           0xE4                            /*R/W the fifo of EDID*/

#define EDID_CTRL           0xE5                            /*R/W Bit[0]: 1-enable local I2C for EDID read/write from EEPROM on SK board
                                                                          0-disable local I2C for EDID read/write from EEPROM on SK board*/
#define EDID_LOCAL_I2C_MASK (BIT0_MASK)
#define EDID_LOCAL_I2C_DISABLE 0x00
#define EDID_LOCAL_I2C_ENABLE (BIT0_MASK)

#define EDID_REREAD_MASK (BIT1_MASK)
#define EDID_REREAD BIT1_MASK                   /*trigger SiI9678 to re-read DS EDID*/

#define EDID_STATE          0xE6                /*R, EDID read ok, data available*/
#define EDID_AVAILABLE_MASK BIT0_MASK

/********************************************************
*
*                   ISP Group
*
*********************************************************/
#define ISP_CONFIGURE_SLAVE_ADDR    0xF6  //HW page 0xF0 offset 40
#define ISP_CONFIGURE_ERASE_CMD     0xF7  //HW page 0xF0 offset 34
#define ISP_CONFIGUER_WRITE_EN_CMD  0xF8  //HW page 0xF0 offset 31
#define ISP_CONFIGURE_ENABLE_PAGE   0xF9  //HW page 0xF0 offset 30

#define LEGCY_VIRTUAL_REQ_WRITE     0xFC
#define LEGACY_VIRTUAL_REQ_RDWR     0xFD
#define LEGACY_VIRTUAL_REQ_OFFSET   0xFE
#define LEGACY_VIRTUAL_REQ_ADDR     0xFF


/********************************************************
*
*                   Debug Group
*
*********************************************************/
#define DEBUG_REGISTER     0xF0
#define DEBUG_TX_TMDS_CTRL               0xF2
#define TX_TMDS_ENABLE_ANALOG_POWER_MASK (BIT4_MASK|BIT5_MASK|BIT6_MASK|BIT7_MASK)
#define DEBUG_DIAGNOSTICS_POWER_DOWN     0xF3
#define BIT_POWER_DOWN_IDCK              BIT2_MASK

#ifdef __cplusplus
}
#endif
#endif /*__SI_VIRTUAL_PAGE_H__*/


