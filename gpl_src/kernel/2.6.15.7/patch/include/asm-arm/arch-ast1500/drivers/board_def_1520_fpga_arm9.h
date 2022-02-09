#ifndef _AST1500_BOARD_DEF_1520_FPGA_ARM9_H
#define	_AST1500_BOARD_DEF_1520_FPGA_ARM9_H

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
#define BOARD_DESIGN_VER_I2S		205 //Basically based on AST1510 board V5
// BOARD_DESIGN_VER_VIDEO 205:
//	- Host DAC output controlled by GPIO.
#define BOARD_DESIGN_VER_VIDEO	205 //Basically based on AST1510 board V5
// BOARD_DESIGN_VER_IR 204:
//	- Add Tx/Rx dual mode switch. Also changes IR polarity for dual mode
#define BOARD_DESIGN_VER_IR		204 //Basically based on AST1510 board V4
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
#endif
#define CONFIG_AST1500_CAT6613
#define CONFIG_AST1500_SII9136
#define FPGA_TEST_ONLY

/*
** I2C
*/
//host EDID EEPROM DDC
#define I2C_HOST_VIDEO_DDC_EEPROM	2
//host loopback VGA DDC
#define I2C_HOST_VIDEO_LOOPBACK_DDC	3 //TBD
//client VGA DDC
#define I2C_CLIENT_VIDEO_DDC_EEPROM 3 //TBD
//host analog video receiver
#define I2C_VIDEO2_9883 		    1 //TBD
//host/client analog audio codec
#define I2C_AUDIO_CODEC 1 //TBD
//host HDMI receiver
#define I2C_HDMI_RX 1

#ifdef FPGA_TEST_ONLY
	//host/client HDMI transmitter
	#define I2C_HDMI_TX 1
#else
	//host/client HDMI transmitter
	#define I2C_HDMI_TX 3
#endif

/*
** GPIO
*/
#define GPIO_BUTTON1	AST1500_GPK4 //TBD
#define GPIO_BUTTON2	AST1500_GPK4 //TBD
#define GPIO_LED_LINK	AST1500_GPP4 //TBD
#define GPIO_LED_PWR	AST1500_GPP5 //TBD
#define GPIO_CH0	AST1500_GPK0 //TBD
	#define GPIO_CH0_ACTIVE 1
#define GPIO_CH1	AST1500_GPK1 //TBD
	#define GPIO_CH1_ACTIVE 1
#define GPIO_CH2	AST1500_GPK2 //TBD
	#define GPIO_CH2_ACTIVE 1
#define GPIO_CH3	AST1500_GPK3 //TBD
	#define GPIO_CH3_ACTIVE 1
#define GPIO_CH_UPDATE	AST1500_GPC0 //TBD
	#define GPIO_CH_UPDATE_ACTIVE	0
#define GPIO_POWER_CTRL_IN		AST1500_GPC1//client //TBD
#define GPIO_POWER_CTRL_OUT		AST1500_GPC1//host //TBD
#define GPIO_POWER_STATUS_OUT	AST1500_GPK4 //For remote PC pwr status (Client) //TBD
#define GPIO_POWER_STATUS_IN		AST1500_GPK4 //For remote PC pwr status (Host) //TBD

#define	GPIO_HOST_VIDEO_MONITOR_DETECT	AST1500_GPC4 //TBD
#define	GPIO_HOST_VIDEO_ATTACH_CONTROL	AST1500_GPE4
	#define GPIO_HOST_VIDEO_DETACH_LEVEL	1
	#define GPIO_HOST_VIDEO_ATTACH_LEVEL	0
#define	GPIO_HOST_VIDEO_DDC_PATH_CONTROL	AST1500_GPE5
	#define	GPIO_HOST_VIDEO_DDC_PATH_DETACH_LEVEL 1
	#define	GPIO_HOST_VIDEO_DDC_PATH_ATTACH_LEVEL 0
#define	GPIO_CLIENT_VIDEO_MONITOR_DETECT	AST1500_GPC4 //TBD
#define GPIO_CAT6023_RST AST1500_GPE7
#define GPIO_CAT6023_INT AST1500_GPE3
#define GPIO_CAT6613_RST AST1500_GPE6
#ifdef FPGA_TEST_ONLY
	#define GPIO_CAT6613_INT AST1500_GPD4
#else
	#define GPIO_CAT6613_INT AST1500_GPC6
#endif
#define GPIO_AUDIO_CODEC_IN_HOTPLUG AST1500_GPC3 //TBD
#define GPIO_AUDIO_CODEC_OUT_HOTPLUG AST1500_GPC2 //TBD
#define GPIO_AUDIO_IO_SELECT AST1500_GPJ6 //TBD
#define GPIO_AUDIO_CODEC_RESET AST1500_GPJ7 //TBD
#define GPIO_IR_DISABLE AST1500_GPP6 //TBD
#define GPIO_IR_IO_SWITCH AST1500_GPP6 //TBD
#define GPIO_HOST_DAC_SWITCH AST1500_GPJ5 //TBD. Maybe won't need it in AST1520.
	#define GPIO_HOST_DAC_SWITCH_ON 1
	#define GPIO_HOST_DAC_SWITCH_OFF 0
	

#endif //#ifndef _AST1500_BOARD_DEF_1520_FPGA_ARM9_H

