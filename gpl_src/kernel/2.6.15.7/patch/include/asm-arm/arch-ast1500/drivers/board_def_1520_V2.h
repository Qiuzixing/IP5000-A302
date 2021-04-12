#ifndef _AST1500_BOARD_DEF_1520_V2_H
#define	_AST1500_BOARD_DEF_1520_V2_H

#undef PATCH_CODEV3
#define PATCH_CODEV3 (\
                    PATCH_AST1520A0_BUG14071001_VR238 \
                  | PATCH_AST1520A0_BUG14071002_VE_SLOW \
                  | PATCH_AST1520A0_BUG14030300_MAC_HQ \
                  | PATCH_AST1520A0_BUG14080600_MAC_FTL \
                  | PATCH_AST1520A0_BUG14071000_HPLL_RESET \
                  | PATCH_AST1520A0_BUG14081201_SCU_RESET \
                  | PATCH_AST1520A0_BUG14090302_VE_4K30HZ_HDOWN \
                  | PATCH_AST1520A0_BUG14090203_VE_ANTI_FLICKER \
                  | PATCH_AST1520A0_I2S_DUAL_OUTPUT \
                  | PATCH_AST1520A0_BUG14082801_CRT_4K \
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

#ifdef FPGA_TEST_ONLY
	//host/client HDMI transmitter
	#define I2C_HDMI_TX             1 //TBD
#else
	//host/client HDMI transmitter
	#define I2C_HDMI_TX             5
#endif

/* Client HDCP2.2*/
#define I2C_HDCP22_TX	            2

/* Host HDCP2.2*/
#define I2C_HDCP22_RX	            5

/*
** GPIO
*/
#define GPIO_BUTTON1	AST1500_GPJ0
#define GPIO_BUTTON2	AST1500_GPJ1
#define GPIO_LED_LINK	AST1500_GPJ3
#define GPIO_LED_PWR	AST1500_GPJ2
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
#define GPIO_CAT6613_RST AST1500_GPD1 //is low active
#define GPIO_HDCP22_TX_RST AST1500_GPA6 /* low active */
#define GPIO_HDCP22_RX_RST AST1500_GPV5 /* low active */
#ifdef FPGA_TEST_ONLY
	#define GPIO_CAT6613_INT AST1500_GPD4 //TBD
#else
	#define GPIO_CAT6613_INT AST1500_GPD2 //is low active
#endif
#define GPIO_AUDIO_CODEC_IN_HOTPLUG AST1500_GPE7 /* low active */
#define GPIO_AUDIO_CODEC_OUT_HOTPLUG AST1500_GPE6 /* low active */
#define GPIO_AUDIO_IO_SELECT AST1500_GPJ6 //Useless AST1520 built-in
#define GPIO_AUDIO_CODEC_RESET AST1500_GPD7 //is low active
#define GPIO_IR_DISABLE AST1500_GPP6 //Useless on AST1520
#define GPIO_IR_IO_SWITCH AST1500_GPP6 //Useless. AST1520 built-in
#define GPIO_HOST_DAC_SWITCH AST1500_GPJ5 //Useless AST1520 built-in
	#define GPIO_HOST_DAC_SWITCH_ON 1
	#define GPIO_HOST_DAC_SWITCH_OFF 0


#endif //#ifndef _AST1500_BOARD_DEF_1520_V2_H

