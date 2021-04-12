#ifndef _AST1500_BOARD_DEF_1500_V3_H
#define	_AST1500_BOARD_DEF_1500_V3_H

//trapping
#define 	CLIENT_MODE_EN_MASK 			0x00040000
#define 	BE_SOURCE_CHIP				(0UL<<18)
#define 	BE_CLIENT_CHIP				(1UL<<18)
//GPIO
//#define	GPIO_HOST_VIDEO_MONITOR_DETECT	/* Loopback monitor is only available under internal VGA mode. And is analog RGB only. So, there is no HP detect on V3 board. */
#define	GPIO_HOST_VIDEO_INPUT_SELECT	AST1500_GPC6 /* V3 board doesn't support Analog RGB in. But has internal VGA. */
#define	GPIO_HOST_VIDEO_DETACH_LEVEL	1
#define	GPIO_HOST_VIDEO_ATTACH_CONTROL	AST1500_GPA6
#define	GPIO_HOST_VIDEO_DDC_PATH_0	AST1500_GPB2
#define	GPIO_HOST_VIDEO_DDC_PATH_1	AST1500_GPB3
#define	GPIO_CLIENT_VIDEO_MONITOR_DETECT	AST1500_GPA6
//I2C
#define	I2C_HOST_VIDEO_DDC_EEPROM	2
#define	I2C_CLIENT_VIDEO_DDC_EEPROM	1

#endif //#ifndef _AST1500_BOARD_DEF_1500_V3_H

