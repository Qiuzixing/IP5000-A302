///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   >mcu.h<
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2009/12/09
//   @fileversion: CAT6613_SAMPLEINTERFACE_1.09
//******************************************/

#ifndef _MCU_H_
#define _MCU_H_

#ifndef CONFIG_AST1500_CAT6613
#include "515xram.h"
#include <stdio.h>
#include <stdlib.h>

//#define MySon
//#define Unixtar

/***************************************/
/* DEBUG INFO define                   */
/**************************************/
//#define Build_LIB
//#define MODE_RS232



/*************************************/
/*Port Using Define                  */
/*************************************/

#define _1PORT_

#define _Demo_2RX_TX_

#define _Hold_sys_

#ifdef _Demo_2RX_TX_
    #ifdef _Hold_sys_
    #define HoldPin P1_5
    #endif
#else
    #ifdef _Hold_sys_
    #define HoldPin P4_3
    #endif
#endif


/************************************/
/* Function DEfine                  */
/***********************************/

//#define RX_KEY_SWITCH
#define RX_SWITCH_KEY P4_2

#define    _FIXED_TXAUDIO_
//ksh#define      _HW_RESET_TX_

//#define    _enable_DownSample_
//#define     _ENABLE_AVR_Audio_
#define       _HBR_I2S_
//#define     _AUD_COMPRESS_VIA_I2S_

//max7088 20080702
#define     _SUPPORT_RX_REPEATER_
#define     _RX_HDCP_STATE_MACHINE_
#define     _TX_HDCP_STATE_MACHINE_
#define     _ENABLE_HDCP_REPEATER_
//end


#define _ENMULTICH_
#ifdef _ENMULTICH_
// #define CAP_AUD_SAMPLE_FRAME
#endif
//#define _ENLED_
//#define _HPDMOS_
#define _ENPARSE_
//#define _RXPOLLING_

#ifdef _ENPARSE_
    // #define _EDIDI2C_
    // #define _DEFAULT_1080P_
    // #define _COPYEDID_
    // #define _EDIDPARSE_


    #ifdef _COPYEDID_
    //    #define COPY_MASTER_PORT TX0DEV
    #endif


    #ifdef _EDIDPARSE_
        //#define _MAXEDID_
        //#define _PARSEONCE_
    #endif

#endif




///////////////////////////////////////////////////////////////////////////////
// Include file
///////////////////////////////////////////////////////////////////////////////
//#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "Reg_c51.h"
///////////////////////////////////////////////////////////////////////////////
// Type Definition
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#ifdef MySon
extern BYTE xdata  PADMOD0;//        _at_  0x0f50;
extern BYTE xdata  PORT4OE;//        _at_  0x0f57;
extern BYTE xdata  PORT40;//             _at_  0x0f58;
extern BYTE xdata  PORT41;//             _at_  0x0f59;
extern BYTE xdata  PORT42;//             _at_  0x0f5A;
extern BYTE xdata  PORT43;//             _at_  0x0f5B;
extern BYTE xdata  PORT44;//             _at_  0x0f5C;
#else
#endif
////////////////////////////////////////////////////////////////////////////////
#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define FAIL -1

#define ON 1
#define OFF 0

#define LO_ACTIVE TRUE
#define HI_ACTIVE FALSE

#ifndef NULL
#define NULL 0
#endif
///////////////////////////////////////////////////////////////////////////////
// Constant Definition
///////////////////////////////////////////////////////////////////////////////
#define TX0DEV            0
#define TX1DEV            1
#define TX2DEV            2
#define TX3DEV            3
#define TX4DEV            4
#define TX5DEV            5
#define TX6DEV            6
#define TX7DEV            7
#define RXDEV            8
#ifdef _1PORT_
    #define TX0ADR    0x98
#endif

#ifdef _2PORT_
        #define TX0ADR    0x98
        #define TX1ADR    0x9A
#endif

#ifdef _3PORT_
        #define TX0ADR    0x98
        #define TX1ADR    0x9A
        #define TX2ADR    0x98
#endif

#ifdef _4PORT_
        #define TX0ADR    0x98
        #define TX1ADR    0x9A
        #define TX2ADR    0x98
        #define TX3ADR    0x9A
#endif

#ifdef _8PORT_
    #define TX0ADR        0x98
    #define TX1ADR        0x9A
    #define TX2ADR        0x98
    #define TX3ADR        0x9A
    #define TX4ADR        0x98
    #define TX5ADR        0x9A
    #define TX6ADR        0x98
    #define TX7ADR        0x9A
#endif

#define RXADR            0x90

#define EDID_ADR        0xA0    // alex 070321

#define DELAY_TIME        1        // unit=1 us;
#define IDLE_TIME        100        // unit=1 ms;

#define HIGH            1
#define LOW                0

#ifdef _HPDMOS_
    #define HPDON        0
    #define HPDOFF        1
#else
    #define HPDON        1
    #define HPDOFF        0
#endif


///////////////////////////////////////////////////////////////////////////////
// 8051 Definition
///////////////////////////////////////////////////////////////////////////////

#ifdef _ENCEC_
    #define CECDEV    0
#endif

#ifdef _1PORT_
    #define DevNum    1
    #define LOOPMS    20
    #ifdef _Demo_2RX_TX_
        sbit TX0_SDA_PORT=P1^1;//P1^1;//P1^4;
        sbit RX_SDA_PORT=P1^1;//P1^1;//P1^4;
        sbit SCL_PORT=P1^0;//P1^0;//P0^2;
    #else
        #ifdef _RIGHT_SIDE_
            sbit TX0_SDA_PORT=P0^6;  //right port
        #else
            sbit TX0_SDA_PORT=P1^4;//P1^1;//P1^4;
        #endif
        sbit RX_SDA_PORT=P1^4;//P1^1;//P1^4;
        sbit SCL_PORT=P0^2;//P1^0;//P0^2;
    #endif
#endif

#ifdef _2PORT_
    #define DevNum    2
    #define LOOPMS    20
    #ifdef _RIGHT_SIDE_
        sbit TX0_SDA_PORT=P0^6;
        sbit TX1_SDA_PORT=P0^6;
    #else
        sbit TX0_SDA_PORT=P1^4;
        sbit TX1_SDA_PORT=P1^4;
    #endif
        sbit RX_SDA_PORT=P1^4;
        sbit SCL_PORT=P0^2;
#endif

#ifdef _3PORT_
    #define DevNum    3
    #define LOOPMS    30
        sbit TX0_SDA_PORT=P1^4;
        sbit TX1_SDA_PORT=P1^4;
        sbit TX2_SDA_PORT=P0^5;
        sbit RX_SDA_PORT=P1^4;
        sbit SCL_PORT=P0^2;
#endif

#ifdef _4PORT_
    #define DevNum    4
    #define LOOPMS    40
    #ifdef Unixtar
        sbit TX0_SDA_PORT=P0^6;
        sbit TX1_SDA_PORT=P0^6;
        sbit TX2_SDA_PORT=P1^7;
        sbit TX3_SDA_PORT=P1^7;
        sbit RX_SDA_PORT=P0^6;
    #else
    #ifndef _RIGHT_SIDE_
        sbit TX0_SDA_PORT=P1^4;
        sbit TX1_SDA_PORT=P1^4;
        sbit TX2_SDA_PORT=P0^5;
        sbit TX3_SDA_PORT=P0^5;
    #else
        sbit TX0_SDA_PORT=P0^6;
        sbit TX1_SDA_PORT=P0^6;
        sbit TX2_SDA_PORT=P1^7;
        sbit TX3_SDA_PORT=P1^7;
    #endif
        sbit RX_SDA_PORT=P1^4;
    #endif
        sbit SCL_PORT=P0^2;
#endif

#ifdef _8PORT_
    #define DevNum    8
    #define LOOPMS    80
        sbit TX0_SDA_PORT=P1^4;
        sbit TX1_SDA_PORT=P1^4;
        sbit TX2_SDA_PORT=P0^5;
        sbit TX3_SDA_PORT=P0^5;
        sbit TX4_SDA_PORT=P0^6;
        sbit TX5_SDA_PORT=P0^6;
        sbit TX6_SDA_PORT=P1^7;
        sbit TX7_SDA_PORT=P1^7;
        sbit RX_SDA_PORT=P1^4;
        sbit SCL_PORT=P0^2;
#endif

//sbit RX_LED=P4^0;

#ifdef _1PORT_
    sbit TX0_LED=P2^0;
#endif

#ifdef _2PORT_
    sbit TX0_LED=P2^0;
    sbit TX1_LED=P2^1;
#endif

#ifdef _3PORT_
    sbit TX0_LED=P2^0;
    sbit TX1_LED=P2^1;
    sbit TX2_LED=P2^2;
#endif

#ifdef _4PORT_
#ifndef _RIGHT_SIDE_
    sbit TX0_LED=P2^0;
    sbit TX1_LED=P2^1;
    sbit TX2_LED=P2^2;
    sbit TX3_LED=P2^3;
#else
    sbit TX0_LED=P2^4;
    sbit TX1_LED=P2^5;
    sbit TX2_LED=P2^6;
    sbit TX3_LED=P2^7;
#endif
#endif

#ifdef _8PORT_
    sbit TX0_LED=P2^0;
    sbit TX1_LED=P2^1;
    sbit TX2_LED=P2^2;
    sbit TX3_LED=P2^3;
    sbit TX4_LED=P2^4;
    sbit TX5_LED=P2^5;
    sbit TX6_LED=P2^6;
    sbit TX7_LED=P2^7;
#endif

#ifdef _Demo_2RX_TX_
    sbit HW_RSTN=P3^5;
    sbit EDID_SCL=P1^3;
    sbit EDID_SDA=P1^2;
    sbit EDID1_SCL=P1^7;
    sbit EDID1_SDA=P1^4;

    sbit EDID_WP=P4^0;

//    sbit RX_HPD=P0^0;
	sbit RX_HPD=P4^2;

    sbit EDID1_WP=P4^1;

//    sbit RX_HPD1=P0^1;
	sbit RX_HPD1=P4^3;


#else

#ifndef RX_SINGAL_PORT_B
    sbit EDID_SCL=P1^0;
    sbit EDID_SDA=P1^1;
    sbit EDID_WP=P3^5;
    sbit RX_HPD=P3^3;

    sbit EDID1_SCL=P1^2;
    sbit EDID1_SDA=P1^3;
    sbit EDID1_WP=P0^1;
    sbit RX_HPD1=P0^0;
#else

    sbit EDID1_SCL=P1^0;
    sbit EDID1_SDA=P1^1;
    sbit EDID1_WP=P3^5;
    sbit RX_HPD1=P3^3;

    sbit EDID_SCL=P1^2;
    sbit EDID_SDA=P1^3;
    sbit EDID_WP=P0^1;
    sbit RX_HPD=P0^0;
#endif


    sbit HW_RSTN=P3^4;
    #ifdef MySon
    //#define RX0_LED            P4^0;
    //#define RX1_LED            P4^1;
    //#define Port_Secection    P4^2;
    #else
    sbit RX0_LED=P4^0;
    sbit RX1_LED=P4^1;
    #endif
#endif

#ifndef HDMI_RX_I2C_SLAVE_ADDR
#define HDMI_RX_I2C_SLAVE_ADDR 0x90
#endif // HDMI_RX_I2C_SLAVE_ADDR

#else

#include <asm/arch/drivers/board_def.h>

#define HDMI_I2C_CHANNEL	I2C_HDMI_TX
#ifndef HDMI_TX_I2C_SLAVE_ADDR
#define HDMI_TX_I2C_SLAVE_ADDR 0x9A
#endif // HDMI_TX_I2C_SLAVE_ADDR

#endif

#endif    // _MCU_H_
