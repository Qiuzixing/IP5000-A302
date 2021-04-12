///*****************************************
//  Copyright (C) 2009-2019
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <Main.c>
//   @author Kuro.Chung@ite.com.tw
//   @date   2019/03/05
//   @fileversion: iTE6805_MCUSRC_1.31
//******************************************/
#include "iTE6805_Global.h"
#include "Main.h"
#include "version.h"

_iTE6805_DATA			iTE6805_DATA;
_iTE6805_VTiming		iTE6805_CurVTiming;
_iTE6805_PARSE3D_STR	iTE6805_EDID_Parse3D;



#ifdef _MCU_8051_
extern AVI_Color_Format TEMP_Output_ColorFormat;
void main( void ){
#else
void iTE6805_Main( void ){
#endif

	#ifdef _MCU_8051_
	iTE_u16 INT_TimeOutCnt = 0;
	P2_3 = 0;	// for DAC enable !!!
	initialTimer1();
	InitMessage();
	HoldSystem();

	NEAREST_P1 = 1;
	MIDDLE_P1 = 1;
	FAREST_P1 = 1;
	#endif

	while( 1 ) {
		#ifdef _MCU_8051_
		if( INT_TimeOutCnt > 50 ) {
		#endif

            iTE6805_FSM();  // put it to your system thread/MCU loop, thread loop perfer setting to 50ms/per round.

		#ifdef _MCU_8051_
			INT_TimeOutCnt = 0;

		}
		else
		{
        	INT_TimeOutCnt += getloopTicCount();
		}
		#endif

		#ifdef _MCU_8051_
		HoldSystem();
		#endif
	}
}

#ifdef _MCU_8051_
void InitMessage(void)
{
#if Debug_message
	init_printf();
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	printf("                   %s\n",VERSION_STRING);
	printf("                   %s\n",DATE_STRING);
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
#endif
}


void HoldSystem(void)
{
    Hold_Pin=1;
    while(!Hold_Pin)
    {
    #if Debug_message
            printf("Hold\\\r");
            printf("Hold-\r");
            printf("Hold/\r");
            printf("Hold|\r");
    #endif
    }
}
#endif
