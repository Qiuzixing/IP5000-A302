#ifndef _AST1500_BOARD_DEF_1520_V3_H
#define	_AST1500_BOARD_DEF_1520_V3_H

#undef PATCH_CODEV3
#define PATCH_CODEV3 (\
                    PATCH_AST1520A0_BUG14071002_VE_SLOW \
                  | PATCH_AST1520A0_I2S_DUAL_OUTPUT \
                  | PATCH_AST1520A1_BUG15051500_JPEG_VE_HANG \
                   )

/*
** Different board design version
*/
// BOARD_DESIGN_VER_TRAP 204:
//	- The first complete trapping design for AST1510
#define BOARD_DESIGN_VER_TRAP		204 //Basically based on AST1510 board V4
// BOARD_DESIGN_VER_I2S 204:
//	-Add codec/HDMI hotplug detect and auto change
// BOARD_DESIGN_VER_I2S 205:
//	- Fixed UART1 pin conflict with audio codec
//	- GPIOJ6,J7 program multi-pin.
#define BOARD_DESIGN_VER_I2S		300 /* Basically based on AST1520 board V1 */
// BOARD_DESIGN_VER_VIDEO 205:
//	- Host DAC output controlled by GPIO.
// BOARD_DESIGN_VER_VIDEO 300:
//	- SoC V3 demo board design
//  - Dual video input (HDMI and VGA)
//  - Both video input has its own EDID EEPROM and bus.
//  - Loopback DAC has "HSync as DE" hack.
#define BOARD_DESIGN_VER_VIDEO	300 //Basically based on AST1520 demo board
// BOARD_DESIGN_VER_IR 204:
//	- Add Tx/Rx dual mode switch. Also changes IR polarity for dual mode
#define BOARD_DESIGN_VER_IR		300 //Basically based on AST1520 board V1
// BOARD_DESIGN_VER_MISC 104:
//	- Add GPIO_CH_UPDATE feature
// BOARD_DESIGN_VER_MISC 105:
//	- Add GPIO_POWER_STATUS feature
#define BOARD_DESIGN_VER_MISC		105 //Basically based on AST1510 board V5


/*
** components
*/
#ifdef CONFIG_ARCH_AST1500_HOST
	#define CONFIG_AST1500_CAT6023
	#define CONFIG_AST1500_ADC9883
	#define CONFIG_AST1500_CAT9883
	#define CONFIG_AST1500_SII9679
	#define CONFIG_AST1500_SII9678
#endif
#ifdef CONFIG_ARCH_AST1500_CLIENT
	#define CONFIG_AST1500_SII9678
#endif
#define CONFIG_AST1500_CAT6613
#define CONFIG_AST1500_SII9136
//#define FPGA_TEST_ONLY

/*
** I2C
*/
//host EDID EEPROM DDC
/*
 * For AST1520 EVA board, the EEPROM is on HDMI receiver daughter board
 * It is connect with I2C address 6
 */
#define I2C_HOST_VIDEO_DDC_EEPROM   6
/* SOC V3 new design. Video port B has its own EEPROM. */
#define I2C_HOST_VIDEO_DDC_EEPROM_B 7

//host loopback VGA DDC
#define I2C_HOST_VIDEO_LOOPBACK_DDC 3 //TBD
//client VGA DDC
#define I2C_CLIENT_VIDEO_DDC_EEPROM 3
//host analog video receiver
#define I2C_VIDEO2_9883             2
//host/client analog audio codec
#define I2C_AUDIO_CODEC             4
//host HDMI receiver
#define I2C_HDMI_RX                 1
//host/client HDMI transmitter
#ifdef CONFIG_ARCH_AST1500_HOST
	#define I2C_HDMI_TX             3
#else
	#define I2C_HDMI_TX             1
#endif

/* Client HDCP2.2*/
#define I2C_HDCP22_TX	            2

/* Host HDCP2.2*/
#define I2C_HDCP22_RX	            5

/*
** GPIO
*/
#define GPIO_BUTTON1				AST1500_GPJ0
#define LED_LINK_B					AST1500_GPJ1
#define LED_LINK_G					AST1500_GPJ2
#define LED_LINK_R					AST1500_GPJ3
#define LED_STATUS_B				AST1500_GPG0
#define LED_STATUS_G				AST1500_GPG1
#define LED_STATUS_R				AST1500_GPG2
#define LED_ON_B					AST1500_GPG3
#define LED_ON_G					AST1500_GPG4
#define LED_ON_R					AST1500_GPG5
#define POWERON_1V8					AST1500_GPH3
#define POWERON_1V2					AST1500_GPH4
#define POWERON_1V3					AST1500_GPH6
#define LCD_POWER					AST1500_GPH7
#define LINEIN_MUTE					AST1500_GPF0
#define LINEOUT_MUTE				AST1500_GPF1
#define AUDIO_SENSITIVE_HIGH		AST1500_GPF2
#define AUDIO_SENSITIVE_MIDDLE		AST1500_GPF3
#define MCU_RESET					AST1500_GPI4
#define RTL_RESET					AST1500_GPI5
#define KEY_UP						AST1500_GPE0
#define KEY_DOWN					AST1500_GPE1
#define KEY_LEFT					AST1500_GPE2
#define KEY_RIGHT					AST1500_GPE3
#define KEY_ENTER					AST1500_GPE4

#define GPIO_CH0	AST1500_GPE0
	#define GPIO_CH0_ACTIVE 1
#define GPIO_CH1	AST1500_GPE1
	#define GPIO_CH1_ACTIVE 1
#define GPIO_CH2	AST1500_GPE2
	#define GPIO_CH2_ACTIVE 1
#define GPIO_CH3	AST1500_GPE3
	#define GPIO_CH3_ACTIVE 1
#define GPIO_CH_UPDATE	AST1500_GPE4
	#define GPIO_CH_UPDATE_ACTIVE	0
#define GPIO_V_TYPE	AST1500_GPQ4
	#define GPIO_V_TYPE_ACTIVE 0 /* should active when analog port selected. */
#define GPIO_POWER_CTRL_IN		AST1500_GPQ5//client
#define GPIO_POWER_CTRL_OUT		AST1500_GPQ5//host
#define GPIO_POWER_STATUS_OUT	AST1500_GPE5 //For remote PC pwr status (Client)
#define GPIO_POWER_STATUS_IN		AST1500_GPE5 //For remote PC pwr status (Host)

#define	GPIO_HOST_VIDEO_MONITOR_DETECT	AST1500_GPC4 //TBD. For loopback
#define	GPIO_HOST_VIDEO_ATTACH_CONTROL	AST1500_GPD4
	#define GPIO_HOST_VIDEO_DETACH_LEVEL	1
	#define GPIO_HOST_VIDEO_ATTACH_LEVEL	0
#define	GPIO_HOST_VIDEO_DDC_PATH_CONTROL	AST1500_GPD0
	#define	GPIO_HOST_VIDEO_DDC_PATH_DETACH_LEVEL 1
	#define	GPIO_HOST_VIDEO_DDC_PATH_ATTACH_LEVEL 0
#define	GPIO_CLIENT_VIDEO_MONITOR_DETECT	AST1500_GPD0
#define GPIO_CAT6023_RST AST1500_GPD3 //is low active
#define GPIO_CAT6023_INT AST1500_GPD6 //is low active

#ifdef CONFIG_ARCH_AST1500_HOST
#define GPIO_CAT6613_RST AST1500_GPS1 //is low active
#define GPIO_CAT6613_INT AST1500_GPS2 //is low active
#else
#define GPIO_CAT6613_RST AST1500_GPD1 //is low active
#define GPIO_CAT6613_INT AST1500_GPD2 //is low active
#endif

#ifdef CONFIG_ARCH_AST1500_HOST
#define GPIO_HDCP22_TX_RST AST1500_GPK6 /* low active */
#define GPIO_HDCP22_TX_DEBUG_MODE AST1500_GPK7
#else
#define GPIO_HDCP22_TX_RST AST1500_GPA6 /* low active */
#define GPIO_HDCP22_TX_DEBUG_MODE AST1500_GPA7
#endif

#define GPIO_HDCP22_RX_RST AST1500_GPV5 /* low active */
#define GPIO_HDCP22_RX_DEBUG_MODE AST1500_GPV4

#define GPIO_AUDIO_CODEC_IN_HOTPLUG AST1500_GPE7 /* low active. 0:analog 1:digital */
#define GPIO_AUDIO_CODEC_IN_HOTPLUG_ACTIVE 0 /* 0:low active, 1:high active */
#define GPIO_AUDIO_CODEC_OUT_HOTPLUG AST1500_GPE6 /* low active */
#define GPIO_AUDIO_IO_SELECT AST1500_GPJ6 //Useless AST1520 built-in
#define GPIO_AUDIO_CODEC_RESET AST1500_GPD7 //is low active
#define GPIO_IR_DISABLE AST1500_GPP6 //Useless on AST1520
#define GPIO_IR_IO_SWITCH AST1500_GPP6 //Useless. AST1520 built-in
#define GPIO_HOST_DAC_SWITCH AST1500_GPV6 //Used to resolve loopback ripple noise issue.
	#define GPIO_HOST_DAC_SWITCH_ON 1
	#define GPIO_HOST_DAC_SWITCH_OFF 0


#endif //#ifndef _AST1500_BOARD_DEF_1520_V3_H

