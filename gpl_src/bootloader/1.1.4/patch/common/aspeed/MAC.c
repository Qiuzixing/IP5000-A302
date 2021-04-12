/*
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define MAC_C
static const char ThisFile[] = "MAC.c";

#include "SWFUNC.H"

#if defined(SLT_UBOOT)
  #include <common.h>
  #include <command.h>
  #include "STDUBOOT.H"
  #include "COMMINF.H"
#endif
#if defined(DOS_ALONE)
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
  #include "COMMINF.H"
#endif
#if defined(LinuxAP)
  #include <stdio.h>
  #include <string.h>
  #include "COMMINF.H"
#endif
#if defined(SLT_NEW_ARCH)
  #include <stdlib.h>
  #include "COMMINF.H"
#endif

// -------------------------------------------------------------
const  ULONG   ARP_org_data[16] = {
        0xffffffff,
        0x0000ffff, // SA:00 00
        0x12345678, // SA:12 34 56 78
        0x01000608, // ARP(0x0806)
        0x04060008,
        0x00000100, // sender MAC Address: 00 00
        0x12345678, // sender MAC Address: 12 34 56 78
        0xeb00a8c0, // sender IP Address:  192.168.0.235
        0x00000000, // target MAC Address: 00 00 00 00
        0xa8c00000, // target MAC Address: 00 00, sender IP Address:192.168
        0x00000100, // sender IP Address:  0.1
//      0x0000de00, // sender IP Address:  0.222
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0xc68e2bd5
};



//------------------------------------------------------------
// Read
//------------------------------------------------------------
ULONG Read_Mem_Dat_NCSI_DD (ULONG addr) {
#if defined(SLT_UBOOT) && defined(AST1010_CHIP)
	return ( DWSwap_SLT(ReadSOC_DD(addr)) );
#else
	return ( ReadSOC_DD(addr) );
#endif
}
ULONG Read_Mem_Des_NCSI_DD (ULONG addr) {
	return ( ReadSOC_DD(addr) );
}
ULONG Read_Mem_Dat_DD (ULONG addr) {
	return ( ReadSOC_DD(addr) );
}
ULONG Read_Mem_Des_DD (ULONG addr) {
	return ( ReadSOC_DD(addr) );
}
ULONG Read_Reg_MAC_DD (MAC_ENGINE *eng, ULONG addr) {
	return ( ReadSOC_DD( eng->run.MAC_BASE + addr ) );
}
ULONG Read_Reg_SCU_DD (ULONG addr) {
	return ( ReadSOC_DD( SCU_BASE + addr ) );
}
ULONG Read_Reg_SDR_DD (ULONG addr) {
	return ( ReadSOC_DD( SDR_BASE + addr ) );
}
ULONG Read_Reg_SMB_DD (ULONG addr) {
	return ( ReadSOC_DD( SMB_BASE + addr ) );
}

//------------------------------------------------------------
// Write
//------------------------------------------------------------
void Write_Mem_Dat_NCSI_DD (ULONG addr, ULONG data) {
#if defined(SLT_UBOOT) && defined(AST1010_CHIP)
	WriteSOC_DD( addr, DWSwap_SLT(data) );
#else
	WriteSOC_DD( addr, data );
#endif
}
void Write_Mem_Des_NCSI_DD (ULONG addr, ULONG data) {
	WriteSOC_DD( addr, data );
}
void Write_Mem_Dat_DD (ULONG addr, ULONG data) {
	WriteSOC_DD( addr, data );
}
void Write_Mem_Des_DD (ULONG addr, ULONG data) {
	WriteSOC_DD( addr, data );
}
void Write_Reg_MAC_DD (MAC_ENGINE *eng, ULONG addr, ULONG data) {
	WriteSOC_DD( eng->run.MAC_BASE + addr, data );
}
void Write_Reg_SCU_DD (ULONG addr, ULONG data) {
	WriteSOC_DD( SCU_BASE + addr, data );
}

//------------------------------------------------------------
// Others
//------------------------------------------------------------
void Debug_delay (void) {
#ifdef DbgPrn_Enable_Debug_delay
	GET_CAHR();
#endif
}

//------------------------------------------------------------
void dump_mac_ROreg (MAC_ENGINE *eng) {
	DELAY( Delay_MACDump );
	printf("\n");
	printf("[MAC-H] ROReg A0h~ACh: %08lx %08lx %08lx %08lx\n", Read_Reg_MAC_DD( eng, 0xA0 ), Read_Reg_MAC_DD( eng, 0xA4 ), Read_Reg_MAC_DD( eng, 0xA8 ), Read_Reg_MAC_DD( eng, 0xAC ));
	printf("[MAC-H] ROReg B0h~BCh: %08lx %08lx %08lx %08lx\n", Read_Reg_MAC_DD( eng, 0xB0 ), Read_Reg_MAC_DD( eng, 0xB4 ), Read_Reg_MAC_DD( eng, 0xB8 ), Read_Reg_MAC_DD( eng, 0xBC ));
	printf("[MAC-H] ROReg C0h~C8h: %08lx %08lx %08lx      \n", Read_Reg_MAC_DD( eng, 0xC0 ), Read_Reg_MAC_DD( eng, 0xC4 ), Read_Reg_MAC_DD( eng, 0xC8 ));
}

//------------------------------------------------------------
// IO delay
//------------------------------------------------------------
void init_iodelay (MAC_ENGINE *eng) {
	int        index;

#ifdef AST2500_IOMAP
	if ( eng->env.AST2500A1 && ( !eng->env.MAC_RMII ) )
		eng->io.Dly_3Regiser = 1;
	else
		eng->io.Dly_3Regiser = 0;
#else
	eng->io.Dly_3Regiser = 0;
#endif

	//------------------------------
	// IO Strength Max.
	//------------------------------
	//------------------------------
	// [IO]setup Str_reg_idx
	// [IO]setup Str_reg_value
	// [IO]setup Str_reg_mask
	// [IO]setup Str_max
	// [IO]setup Str_shf
	//------------------------------
	// Get bit (shift) of IO driving strength register
#ifdef AST1010_CHIP
	//GPIOL4 ~ GPIOL6
	eng->io.Str_reg_idx   = 0xac;
	eng->io.Str_reg_Lbit  = 14;
	eng->io.Str_reg_Hbit  = 15;
	eng->io.Str_reg_value = ( eng->reg.SCU_0ac >> eng->io.Str_reg_Lbit ) & 0x3;
	eng->io.Str_reg_mask  = ( eng->reg.SCU_0ac & 0xffff3fff );
	eng->io.Str_max       = 1;//0~1
	eng->io.Str_shf       = 14;
#elif defined(AST2500_IOMAP)
	eng->io.Str_reg_idx   = 0x90;
	eng->io.Str_reg_Lbit  =  8;
	eng->io.Str_reg_Hbit  = 11;
	eng->io.Str_reg_value = ( eng->reg.SCU_090 >> eng->io.Str_reg_Lbit ) & 0xf;
	eng->io.Str_reg_mask  = ( eng->reg.SCU_090 & 0xfffff0ff );
	eng->io.Str_max       = 1;//0~1
	if ( eng->env.MAC_RMII ) {
		switch ( eng->run.MAC_idx ) {
			case 0  : eng->io.Str_shf =  9; break;
			case 1  : eng->io.Str_shf = 11; break;
		}
	}
	else {//AST2500 RGMII only support high drive RGMIITXCK
		if ( eng->run.TM_IOStrength )
			printf("The RGMII driving strength testing v1.0\n");
		switch ( eng->run.MAC_idx ) {
			case 0  : eng->io.Str_shf =  8; break;
			case 1  : eng->io.Str_shf = 10; break;
		}
	}
#else
	if ( eng->env.AST2400 ) {
		eng->io.Str_reg_idx   = 0x90;
		eng->io.Str_reg_Lbit  =  8;
		eng->io.Str_reg_Hbit  = 11;
		eng->io.Str_reg_value = ( eng->reg.SCU_090 >> eng->io.Str_reg_Lbit ) & 0xf;
		eng->io.Str_reg_mask  = ( eng->reg.SCU_090 & 0xfffff0ff );
		eng->io.Str_max       = 1;//0~1
		switch ( eng->run.MAC_idx ) {
			case 0  : eng->io.Str_shf =  9; break;
			case 1  : eng->io.Str_shf = 11; break;
		}
	}
	else {
		eng->io.Str_reg_idx   = 0x90;
		eng->io.Str_reg_Lbit  =  8;
		eng->io.Str_reg_Hbit  = 15;
		eng->io.Str_reg_value = ( eng->reg.SCU_090 >> eng->io.Str_reg_Lbit ) & 0xff;
		eng->io.Str_reg_mask  = ( eng->reg.SCU_090 & 0xffff00ff );
		eng->io.Str_max       = 3;//0~3
		switch ( eng->run.MAC_idx ) {
			case 0  : eng->io.Str_shf =  8; break;
			case 1  : eng->io.Str_shf = 10; break;
			case 2  : eng->io.Str_shf = 12; break;
			case 3  : eng->io.Str_shf = 14; break;
		}
		}
#endif
	if ( !eng->run.TM_IOStrength )
		eng->io.Str_max = 0;

	//------------------------------
	// IO Delay Stage/Step
	//------------------------------
	//------------------------------
	// [IO]setup Dly_stagebit
	// [IO]setup Dly_stage_in
	// [IO]setup Dly_stage_out
	// [IO]setup Dly_step
	// [IO]setup Dly_mask
	//------------------------------
#ifdef AST2500_IOMAP
	eng->io.Dly_stagebit  = 6;
	eng->io.Dly_stage     =   ( 1 << eng->io.Dly_stagebit );
	eng->io.Dly_stage_in  = ( eng->io.Dly_stage >> AST2500_IOStageShiftBit_In );
	eng->io.Dly_stage_out = ( eng->io.Dly_stage >> AST2500_IOStageShiftBit_Out );
	eng->io.Dly_step      = AST2500_IOStageStep;
#else
	eng->io.Dly_stagebit  = 4;
	eng->io.Dly_stage     = ( 1 << eng->io.Dly_stagebit );
	eng->io.Dly_stage_in  = eng->io.Dly_stage;
	eng->io.Dly_stage_out = eng->io.Dly_stage;
	eng->io.Dly_step      = 1;
#endif
	eng->io.Dly_mask  = (1 << eng->io.Dly_stagebit)-1;

	//------------------------------
	// IO-Delay Register Bit Position
	//------------------------------
	//------------------------------
	// [IO]setup Dly_out_shf
	// [IO]setup Dly_in_shf
	// [IO]setup Dly_in_shf_regH
	// [IO]setup Dly_out_shf_regH
	//------------------------------
#ifdef AST2500_IOMAP
	if ( eng->env.MAC_RMII ) {
		switch ( eng->arg.GRun_Mode ) {
			case 0  : eng->io.Dly_out_shf = 24; eng->io.Dly_in_shf = 12; break;
			case 1  : eng->io.Dly_out_shf = 25; eng->io.Dly_in_shf = 18; break;
		}
	}
	else {
		switch ( eng->arg.GRun_Mode ) {
			case 0  : eng->io.Dly_out_shf =  0; eng->io.Dly_in_shf  = 12; break;
			case 1  : eng->io.Dly_out_shf =  6; eng->io.Dly_in_shf  = 18; break;
		}
	} // End if ( eng->env.MAC_RMII )
#else
	if ( eng->env.MAC_RMII ) {
		switch ( eng->arg.GRun_Mode ) {
			case 0  : eng->io.Dly_out_shf = 24; eng->io.Dly_in_shf =  8; break;
			case 1  : eng->io.Dly_out_shf = 25; eng->io.Dly_in_shf = 12; break;
			case 2  : eng->io.Dly_out_shf = 26; eng->io.Dly_in_shf = 16; break;
			case 3  : eng->io.Dly_out_shf = 27; eng->io.Dly_in_shf = 20; break;
		}
	}
	else {
		switch ( eng->arg.GRun_Mode ) {
			case 0  : eng->io.Dly_out_shf =  0; eng->io.Dly_in_shf  =  8; break;
			case 1  : eng->io.Dly_out_shf =  4; eng->io.Dly_in_shf  = 12; break;
		}
	} // End if ( eng->env.MAC_RMII )
#endif
	eng->io.Dly_in_shf_regH  = eng->io.Dly_in_shf  + eng->io.Dly_stagebit - 1;
	eng->io.Dly_out_shf_regH = eng->io.Dly_out_shf + eng->io.Dly_stagebit - 1;

	//------------------------------
	// [IO]setup value_ary
	//------------------------------
	if ( eng->env.AST2300 && (eng->reg.SCU_07c == 0x01000003) ) {
		//AST2300-A0
		for (index = 0; index < 16; index++) {
			eng->io.value_ary[ index ] = IOValue_Array_A0[ index ];
		}
	}
	else {
		for (index = 0; index < eng->io.Dly_stage; index++)
			eng->io.value_ary[ index ] = index;
	}
}

//------------------------------------------------------------
int get_iodelay (MAC_ENGINE *eng) {
	int        index;

	//------------------------------
	// IO Delay Register Setting
	//------------------------------
	//------------------------------
	// [IO]setup Dly_reg_idx
	// [IO]setup Dly_reg_value
	//------------------------------
#ifdef AST2500_IOMAP
	switch ( eng->run.Speed_idx ) {
		case 0        : eng->io.Dly_reg_idx = 0x48; eng->io.Dly_reg_value = eng->reg.SCU_048; break;
		case 1        : eng->io.Dly_reg_idx = 0xb8; eng->io.Dly_reg_value = eng->reg.SCU_0b8; break;
		case 2        : eng->io.Dly_reg_idx = 0xbc; eng->io.Dly_reg_value = eng->reg.SCU_0bc; break;
	}
#else
	 eng->io.Dly_reg_idx   = 0x48;
	 eng->io.Dly_reg_value = eng->reg.SCU_048;
#endif

	//------------------------------
	// [IO]setup Dly_reg_name_tx
	// [IO]setup Dly_reg_name_rx
	// [IO]setup Dly_reg_name_tx_new
	// [IO]setup Dly_reg_name_rx_new
	//------------------------------
	if ( eng->env.MAC_RMII )
		sprintf( eng->io.Dly_reg_name_tx, "Tx:SCU%2X[   %2d]=",  eng->io.Dly_reg_idx,                           eng->io.Dly_out_shf );
	else
		sprintf( eng->io.Dly_reg_name_tx, "Tx:SCU%2X[%2d:%2d]=", eng->io.Dly_reg_idx, eng->io.Dly_out_shf_regH, eng->io.Dly_out_shf );
	sprintf( eng->io.Dly_reg_name_rx, "Rx:SCU%2X[%2d:%2d]=", eng->io.Dly_reg_idx, eng->io.Dly_in_shf_regH,  eng->io.Dly_in_shf );
#ifdef AST2500_IOMAP
    if ( eng->env.MAC_RMII )
        sprintf( eng->io.Dly_reg_name_tx_new, "Tx[   %2d]=",                            eng->io.Dly_out_shf );
    else
        sprintf( eng->io.Dly_reg_name_tx_new, "Tx[%2d:%2d]=", eng->io.Dly_out_shf_regH, eng->io.Dly_out_shf );
        sprintf( eng->io.Dly_reg_name_rx_new, "Rx[%2d:%2d]=", eng->io.Dly_in_shf_regH,  eng->io.Dly_in_shf );
#endif

	//------------------------------
	// [IO]setup Dly_in_reg
	// [IO]setup Dly_out_reg
	//------------------------------
	// Get current clock delay value of TX(out) and RX(in) in the SCU48 register
	// and setting test range
	if ( eng->env.MAC_RMII ) {
		eng->io.Dly_in_reg  = ( eng->io.Dly_reg_value >> eng->io.Dly_in_shf  ) & eng->io.Dly_mask;
		eng->io.Dly_out_reg = ( eng->io.Dly_reg_value >> eng->io.Dly_out_shf ) & 0x1;
	}
	else {
		eng->io.Dly_in_reg  = ( eng->io.Dly_reg_value >> eng->io.Dly_in_shf  ) & eng->io.Dly_mask;
		eng->io.Dly_out_reg = ( eng->io.Dly_reg_value >> eng->io.Dly_out_shf ) & eng->io.Dly_mask;
	} // End if ( eng->env.MAC_RMII )

	//------------------------------
	// [IO]setup Dly_in_reg_idx
	// [IO]setup Dly_in_min
	// [IO]setup Dly_in_max
	// [IO]setup Dly_out_reg_idx
	// [IO]setup Dly_out_min
	// [IO]setup Dly_out_max
	//------------------------------
	// Find the coordinate in X-Y axis
	for ( index = 0; index < eng->io.Dly_stage_in; index++ )
		if ( eng->io.Dly_in_reg == eng->io.value_ary[ index ] ) {
			eng->io.Dly_in_reg_idx = index;
			eng->io.Dly_in_min     = index - ( eng->run.IO_Bund >> 1 );
			eng->io.Dly_in_max     = index + ( eng->run.IO_Bund >> 1 );
			break;
		}
	for ( index = 0; index < eng->io.Dly_stage_out; index++ )
		if ( eng->io.Dly_out_reg == eng->io.value_ary[ index ] ) {
			eng->io.Dly_out_reg_idx = index;
			if ( eng->env.MAC_RMII ) {
				eng->io.Dly_out_min = index;
				eng->io.Dly_out_max = index;
			}
			else {
				eng->io.Dly_out_min = index - ( eng->run.IO_Bund >> 1 );
				eng->io.Dly_out_max = index + ( eng->run.IO_Bund >> 1 );
			}
			break;
		}

#ifdef Enable_No_IOBoundary
	if ( eng->io.Dly_in_min  <  0                     ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_IOMarginOUF; eng->io.Dly_in_min  = 0                     ;}
	if ( eng->io.Dly_in_max  >= eng->io.Dly_stage_in  ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_IOMarginOUF; eng->io.Dly_in_max  = eng->io.Dly_stage_in-1;}

	if ( eng->io.Dly_out_min <  0                     ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_IOMarginOUF; eng->io.Dly_out_min = 0                      ;}
	if ( eng->io.Dly_out_max >= eng->io.Dly_stage_out ) { eng->flg.Wrn_Flag = eng->flg.Wrn_Flag | Wrn_IOMarginOUF; eng->io.Dly_out_max = eng->io.Dly_stage_out-1;}
#else
	if ( ( eng->io.Dly_in_min < 0 ) || ( eng->io.Dly_in_max >= eng->io.Dly_stage_in ) )
		return( Finish_Check( eng, Err_IOMarginOUF ) );

	if ( ( eng->io.Dly_out_min < 0 ) || ( eng->io.Dly_out_max >= eng->io.Dly_stage_out ) )
		return( Finish_Check( eng, Err_IOMarginOUF ) );
#endif

	//------------------------------
	// IO Delay Testing Boundary
	//------------------------------
	//------------------------------
	// [IO]setup Dly_in_cval
	// [IO]setup Dly_out_cval
	// [IO]setup Dly_in_str
	// [IO]setup Dly_in_end
	// [IO]setup Dly_out_str
	// [IO]setup Dly_out_end
	//------------------------------
	// Get the range for testmargin block
	if ( eng->run.TM_IOTiming ) {
		eng->io.Dly_in_cval  = eng->io.Dly_step;
		eng->io.Dly_out_cval = eng->io.Dly_step;
		eng->io.Dly_in_str   = 0;
		eng->io.Dly_in_end   = eng->io.Dly_stage_in-1;
		eng->io.Dly_out_str  = 0;
		if ( eng->env.MAC_RMII )
			eng->io.Dly_out_end  = 1;
		else
			eng->io.Dly_out_end  = eng->io.Dly_stage_out-1;
	}
	else if ( eng->run.IO_Bund ) {
		eng->io.Dly_in_cval  = eng->io.Dly_step;
		eng->io.Dly_out_cval = eng->io.Dly_step;
		eng->io.Dly_in_str   = eng->io.Dly_in_min;
		eng->io.Dly_in_end   = eng->io.Dly_in_max;
		eng->io.Dly_out_str  = eng->io.Dly_out_min;
		eng->io.Dly_out_end  = eng->io.Dly_out_max;
	}
	else {
		eng->io.Dly_in_cval  = 1;
		eng->io.Dly_out_cval = 1;
		eng->io.Dly_in_str   = 0;
		eng->io.Dly_in_end   = 0;
		eng->io.Dly_out_str  = 0;
		eng->io.Dly_out_end  = 0;
	} // End if ( eng->run.TM_IOTiming )

	return(0);
}

//------------------------------------------------------------
// SCU
//------------------------------------------------------------
void recov_scu (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("recov_scu\n");
	Debug_delay();
#endif

	//MAC
	Write_Reg_MAC_DD( eng, 0x08, eng->reg.MAC_008 );
	Write_Reg_MAC_DD( eng, 0x0c, eng->reg.MAC_00c );
	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040 );

	//SCU
	Write_Reg_SCU_DD( 0x004, eng->reg.SCU_004 );
	Write_Reg_SCU_DD( 0x008, eng->reg.SCU_008 );
	Write_Reg_SCU_DD( 0x00c, eng->reg.SCU_00c );
	Write_Reg_SCU_DD( 0x048, eng->reg.SCU_048 );
#ifdef AST2500_IOMAP
  #ifdef SLT_UBOOT
	Write_Reg_SCU_DD( 0x070, ( eng->reg.SCU_070 & 0xFFFFFFFE) );
  #else
	Write_Reg_SCU_DD( 0x070,   eng->reg.SCU_070  );
  #endif
	Write_Reg_SCU_DD( 0x07c, (~eng->reg.SCU_070) );
#else
  #ifdef SLT_UBOOT
	Write_Reg_SCU_DD( 0x070, ( eng->reg.SCU_070 & 0xFFFFFFFE) );
  #else
	Write_Reg_SCU_DD( 0x070,   eng->reg.SCU_070  );
  #endif
#endif
	Write_Reg_SCU_DD( 0x074, eng->reg.SCU_074 );
	Write_Reg_SCU_DD( 0x080, eng->reg.SCU_080 );
	Write_Reg_SCU_DD( 0x088, eng->reg.SCU_088 );
	Write_Reg_SCU_DD( 0x090, eng->reg.SCU_090 );
#ifdef AST1010_CHIP
	Write_Reg_SCU_DD( 0x0ac, eng->reg.SCU_0ac );
#endif
#ifdef AST2500_IOMAP
	Write_Reg_SCU_DD( 0x0b8, eng->reg.SCU_0b8 );
	Write_Reg_SCU_DD( 0x0bc, eng->reg.SCU_0bc );
#endif

	//WDT
#ifdef AST1010_IOMAP
#else
//	WriteSOC_DD( 0x1e78500c, eng->reg.WDT_00c );
//	WriteSOC_DD( 0x1e78502c, eng->reg.WDT_02c );
#endif

} // End void recov_scu (MAC_ENGINE *eng)

//------------------------------------------------------------
void read_scu (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("read_scu\n");
	Debug_delay();
#endif

	if ( !eng->reg.SCU_oldvld ) {
		//SCU
		eng->reg.SCU_004 = Read_Reg_SCU_DD( 0x004 );
		eng->reg.SCU_008 = Read_Reg_SCU_DD( 0x008 );
		eng->reg.SCU_00c = Read_Reg_SCU_DD( 0x00c );
		eng->reg.SCU_048 = Read_Reg_SCU_DD( 0x048 );
		eng->reg.SCU_070 = Read_Reg_SCU_DD( 0x070 );
		eng->reg.SCU_074 = Read_Reg_SCU_DD( 0x074 );
		eng->reg.SCU_07c = Read_Reg_SCU_DD( 0x07c );
		eng->reg.SCU_080 = Read_Reg_SCU_DD( 0x080 );
		eng->reg.SCU_088 = Read_Reg_SCU_DD( 0x088 );
		eng->reg.SCU_090 = Read_Reg_SCU_DD( 0x090 );
#ifdef AST1010_CHIP
		eng->reg.SCU_0ac = Read_Reg_SCU_DD( 0x0ac );
#endif
#ifdef AST2500_IOMAP
		eng->reg.SCU_0b8 = Read_Reg_SCU_DD( 0x0b8 );
		eng->reg.SCU_0bc = Read_Reg_SCU_DD( 0x0bc );
#endif
		eng->reg.SCU_0f0 = Read_Reg_SCU_DD( 0x0f0 );

#ifdef AST1010_CHIP
		eng->reg.SCU_048_mix   = ( eng->reg.SCU_048 & 0xfefff0ff );
		eng->reg.SCU_048_check = ( eng->reg.SCU_048 & 0x01000f00 );
#else
  #ifdef AST2500_IOMAP
		eng->reg.SCU_048_mix   = ( eng->reg.SCU_048 & 0xfc000000 );
		eng->reg.SCU_048_check = ( eng->reg.SCU_048 & 0x03ffffff );
  #else
		eng->reg.SCU_048_mix   = ( eng->reg.SCU_048 & 0xf0000000 );
		eng->reg.SCU_048_check = ( eng->reg.SCU_048 & 0x0fffffff );
  #endif
#endif

		//WDT
#ifndef AST1010_IOMAP
		eng->reg.WDT_00c = ReadSOC_DD( 0x1e78500c );
		eng->reg.WDT_02c = ReadSOC_DD( 0x1e78502c );
#endif

		eng->reg.SCU_oldvld = 1;
	} // End if ( !eng->reg.SCU_oldvld )
} // End read_scu(MAC_ENGINE *eng)

//------------------------------------------------------------
void Setting_scu (MAC_ENGINE *eng) {
	//SCU
#if defined(SLT_UBOOT) || defined(Enable_MAC_ExtLoop)
#else
  #ifdef AST1010_CHIP
	do {
		Write_Reg_SCU_DD( 0x00 , 0x1688a8a8 );
		Write_Reg_SCU_DD( 0x70 , eng->reg.SCU_070 & 0xfffffffe );  // Disable CPU
	} while ( Read_Reg_SCU_DD( 0x00 ) != 0x1 );
  #elif defined(AST2500_IOMAP)
	do {
		Write_Reg_SCU_DD( 0x00, 0x1688a8a8 );
		Write_Reg_SCU_DD( 0x70, 0x3 ); // Disable CPU
	} while ( Read_Reg_SCU_DD( 0x00 ) != 0x1 );
  #else
	do {
		Write_Reg_SCU_DD( 0x00, 0x1688a8a8 );
		Write_Reg_SCU_DD( 0x70, eng->reg.SCU_070 | 0x3 ); // Disable CPU
	} while ( Read_Reg_SCU_DD( 0x00 ) != 0x1 );
  #endif
#endif

#ifdef AST1010_CHIP
  #if( AST1010_IOMAP == 1 )
	Write_Reg_SCU_DD( 0x11C, 0x00000000 ); // Disable Cache functionn
  #endif
#endif
	//WDT
#ifndef AST1010_IOMAP
	WriteSOC_DD( 0x1e78500c, eng->reg.WDT_00c & 0xfffffffc );
	WriteSOC_DD( 0x1e78502c, eng->reg.WDT_02c & 0xfffffffc );
#endif
}

//------------------------------------------------------------
void init_scu1 (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("init_scu1\n");
	Debug_delay();
#endif

#ifdef AST1010_CHIP
	Write_Reg_SCU_DD( 0x0c, ( eng->reg.SCU_00c & 0xffffffbf ) );//Clock Stop Control
	Write_Reg_SCU_DD( 0x88, ((eng->reg.SCU_088 & 0x003fffff ) | 0xffc00000) );//Multi-function Pin Control
#else
	if ( eng->env.AST2300 ) {
  #ifdef Enable_BufMerge
		Write_Reg_SCU_DD( 0xf0, 0x66559959 );//MAC buffer merge
  #endif
  #ifdef Enable_Int125MHz
  #endif
		switch ( eng->run.MAC_idx ) {
			case 0  :
				Write_Reg_SCU_DD( 0x88, (eng->reg.SCU_088 & 0x3fffffff) | 0xc0000000 );//[31]MAC1 MDIO, [30]MAC1 MDC
				break;
			case 1  :
				Write_Reg_SCU_DD( 0x90, (eng->reg.SCU_090 & 0xfffffffb) | 0x00000004 );//[2 ]MAC2 MDC/MDIO
				break;
			case 2  :
			case 3  :
			default : break;
		}

		if ( eng->env.AST2400 ) {
			Write_Reg_SCU_DD( 0x0c, (eng->reg.SCU_00c & 0xffcfffff) );//Clock Stop Control
		}
		else {
			Write_Reg_SCU_DD( 0x0c, (eng->reg.SCU_00c & 0xff0fffff) );//Clock Stop Control
		}
//		Write_Reg_SCU_DD( 0x80, (eng->reg.SCU_080 & 0xfffffff0) | 0x0000000f );//MAC1LINK/MAC2LINK
	}
	else {
		switch ( eng->run.MAC_idx ) {
			case 0  :
//				Write_Reg_SCU_DD( 0x74, (eng->reg.SCU_074 & 0xfdffffff) | 0x02000000 );//[25]MAC1 PHYLINK
				break;
			case 1  :
				if ( eng->env.MAC2_RMII ) {
//					Write_Reg_SCU_DD( 0x74, (eng->reg.SCU_074 & 0xfbefffff) | 0x04100000 );//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
					Write_Reg_SCU_DD( 0x74, (eng->reg.SCU_074 & 0xffefffff) | 0x00100000 );//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
				}
				else {
//					Write_Reg_SCU_DD( 0x74, (eng->reg.SCU_074 & 0xfbcfffff) | 0x04300000 );//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
					Write_Reg_SCU_DD( 0x74, (eng->reg.SCU_074 & 0xffcfffff) | 0x00300000 );//[26]MAC2 PHYLINK, [21]MAC2 MII, [20]MAC2 MDC/MDIO
				}
				break;
			default : break;
		} // End switch ( eng->run.MAC_idx )
	} // End if ( eng->env.AST2300 )
#endif

	//------------------------------
	// [Reg]setup SCU_004_mix
	// [Reg]setup SCU_004_dis
	// [Reg]setup SCU_004_en
	//------------------------------
	eng->reg.SCU_004_mix = eng->reg.SCU_004;
	eng->reg.SCU_004_en  = eng->reg.SCU_004_mix & (~eng->reg.SCU_004_rstbit);
	eng->reg.SCU_004_dis = eng->reg.SCU_004_mix |   eng->reg.SCU_004_rstbit;
} // End void init_scu1 (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu_macrst (MAC_ENGINE *eng) {

	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_dis );//Rst
	DELAY( Delay_SCU );
	Write_Reg_SCU_DD( 0x04, eng->reg.SCU_004_en );//Enable Engine
} // End void init_scu_macrst (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu2 (MAC_ENGINE *eng) {
#ifdef SCU_74h
  #ifdef  DbgPrn_FuncHeader
	printf("init_scu2\n");
	Debug_delay();
  #endif

	Write_Reg_SCU_DD( 0x74, eng->reg.SCU_074 | SCU_74h );//PinMux
	DELAY( Delay_SCU );
#endif
} // End void init_scu2 (MAC_ENGINE *eng)

//------------------------------------------------------------
void init_scu3 (MAC_ENGINE *eng) {
#ifdef SCU_74h
  #ifdef  DbgPrn_FuncHeader
	printf("init_scu3\n");
	Debug_delay();
  #endif

	Write_Reg_SCU_DD( 0x74, eng->reg.SCU_074 | (SCU_74h & 0xffefffff) );//PinMux
	DELAY( Delay_SCU );
#endif
} // End void init_scu3 (MAC_ENGINE *eng)

//------------------------------------------------------------
// MAC
//------------------------------------------------------------
void init_mac (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("init_mac\n");
	Debug_delay();
#endif

#ifdef Enable_MAC_SWRst
	Write_Reg_MAC_DD( eng, 0x50, 0x80000000 | eng->reg.MAC_050_Speed );

	while (0x80000000 & Read_Reg_MAC_DD( eng, 0x50 )) {
//printf(".");
		DELAY( Delay_MACRst );
	}
	DELAY( Delay_MACRst );
#endif

	Write_Reg_MAC_DD( eng, 0x20, ( eng->run.TDES_BASE + CPU_BUS_ADDR_SDRAM_OFFSET ) ); // 20130730
	Write_Reg_MAC_DD( eng, 0x24, ( eng->run.RDES_BASE + CPU_BUS_ADDR_SDRAM_OFFSET ) ); // 20130730

#ifdef MAC_030_def
	Write_Reg_MAC_DD( eng, 0x30, MAC_030_def );//Int Thr/Cnt
#endif
#ifdef MAC_034_def
	Write_Reg_MAC_DD( eng, 0x34, MAC_034_def );//Poll Cnt
#endif
#ifdef MAC_038_def
	Write_Reg_MAC_DD( eng, 0x38, MAC_038_def );
#endif
#ifdef MAC_040_def
	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040_new | MAC_040_def );
#else
	Write_Reg_MAC_DD( eng, 0x40, eng->reg.MAC_040_new );
#endif
#ifdef MAC_048_def
	Write_Reg_MAC_DD( eng, 0x48, MAC_048_def );
#endif
#ifdef MAC_058_def
  #ifdef AST2500_IOMAP
	Write_Reg_MAC_DD( eng, 0x58, MAC_058_def );
  #endif
#endif

	if ( eng->ModeSwitch == MODE_NSCI )
		Write_Reg_MAC_DD( eng, 0x4c, NCSI_RxDMA_PakSize );
	else
		Write_Reg_MAC_DD( eng, 0x4c, DMA_PakSize );

	Write_Reg_MAC_DD( eng, 0x50, eng->reg.MAC_050_Speed );
	DELAY( Delay_MACRst );
} // End void init_mac (MAC_ENGINE *eng)

//------------------------------------------------------------
// Basic
//------------------------------------------------------------
void FPri_RegValue (MAC_ENGINE *eng, BYTE option) {
#ifdef SLT_UBOOT
#else
	time_t     timecur;
#endif

	FILE_VAR
	GET_OBJ( option )

#ifdef AST2500_IOMAP
	PRINT( OUT_OBJ "[SDR] Date:%08lx\n", Read_Reg_SDR_DD( 0x88 ) );
	PRINT( OUT_OBJ "[SDR]  80:%08lx %08lx %08lx %08lx\n", Read_Reg_SDR_DD( 0x80 ), Read_Reg_SDR_DD( 0x84 ), Read_Reg_SDR_DD( 0x88 ), Read_Reg_SDR_DD( 0x8c ) );
#else
	PRINT( OUT_OBJ "[SMB] Date:%08lx\n", Read_Reg_SMB_DD( 0xa8 ) );
	PRINT( OUT_OBJ "[SMB]  00:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x00 ), Read_Reg_SMB_DD( 0x04 ), Read_Reg_SMB_DD( 0x08 ), Read_Reg_SMB_DD( 0x0c ) );
	PRINT( OUT_OBJ "[SMB]  10:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x10 ), Read_Reg_SMB_DD( 0x14 ), Read_Reg_SMB_DD( 0x18 ), Read_Reg_SMB_DD( 0x1c ) );
	PRINT( OUT_OBJ "[SMB]  20:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x20 ), Read_Reg_SMB_DD( 0x24 ), Read_Reg_SMB_DD( 0x28 ), Read_Reg_SMB_DD( 0x2c ) );
	PRINT( OUT_OBJ "[SMB]  30:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x30 ), Read_Reg_SMB_DD( 0x34 ), Read_Reg_SMB_DD( 0x38 ), Read_Reg_SMB_DD( 0x3c ) );
	PRINT( OUT_OBJ "[SMB]  40:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x40 ), Read_Reg_SMB_DD( 0x44 ), Read_Reg_SMB_DD( 0x48 ), Read_Reg_SMB_DD( 0x4c ) );
	PRINT( OUT_OBJ "[SMB]  50:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x50 ), Read_Reg_SMB_DD( 0x54 ), Read_Reg_SMB_DD( 0x58 ), Read_Reg_SMB_DD( 0x5c ) );
	PRINT( OUT_OBJ "[SMB]  60:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x60 ), Read_Reg_SMB_DD( 0x64 ), Read_Reg_SMB_DD( 0x68 ), Read_Reg_SMB_DD( 0x6c ) );
	PRINT( OUT_OBJ "[SMB]  70:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x70 ), Read_Reg_SMB_DD( 0x74 ), Read_Reg_SMB_DD( 0x78 ), Read_Reg_SMB_DD( 0x7c ) );
	PRINT( OUT_OBJ "[SMB]  80:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x80 ), Read_Reg_SMB_DD( 0x84 ), Read_Reg_SMB_DD( 0x88 ), Read_Reg_SMB_DD( 0x8c ) );
	PRINT( OUT_OBJ "[SMB]  90:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0x90 ), Read_Reg_SMB_DD( 0x94 ), Read_Reg_SMB_DD( 0x98 ), Read_Reg_SMB_DD( 0x9c ) );
	PRINT( OUT_OBJ "[SMB]  A0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xa0 ), Read_Reg_SMB_DD( 0xa4 ), Read_Reg_SMB_DD( 0xa8 ), Read_Reg_SMB_DD( 0xac ) );
	PRINT( OUT_OBJ "[SMB]  B0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xb0 ), Read_Reg_SMB_DD( 0xb4 ), Read_Reg_SMB_DD( 0xb8 ), Read_Reg_SMB_DD( 0xbc ) );
	PRINT( OUT_OBJ "[SMB]  C0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xc0 ), Read_Reg_SMB_DD( 0xc4 ), Read_Reg_SMB_DD( 0xc8 ), Read_Reg_SMB_DD( 0xcc ) );
	PRINT( OUT_OBJ "[SMB]  D0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xd0 ), Read_Reg_SMB_DD( 0xd4 ), Read_Reg_SMB_DD( 0xd8 ), Read_Reg_SMB_DD( 0xdc ) );
	PRINT( OUT_OBJ "[SMB]  E0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xe0 ), Read_Reg_SMB_DD( 0xe4 ), Read_Reg_SMB_DD( 0xe8 ), Read_Reg_SMB_DD( 0xec ) );
	PRINT( OUT_OBJ "[SMB]  F0:%08lx %08lx %08lx %08lx\n", Read_Reg_SMB_DD( 0xf0 ), Read_Reg_SMB_DD( 0xf4 ), Read_Reg_SMB_DD( 0xf8 ), Read_Reg_SMB_DD( 0xfc ) );
#endif

	PRINT( OUT_OBJ "[SCU]  04:%08lx  08:%08lx  0c:%08lx\n",           eng->reg.SCU_004, eng->reg.SCU_008, eng->reg.SCU_00c );
	PRINT( OUT_OBJ "[SCU]  1c:%08lx  2c:%08lx  48:%08lx  4c:%08lx\n", Read_Reg_SCU_DD( 0x01c ), Read_Reg_SCU_DD( 0x02c ), eng->reg.SCU_048, Read_Reg_SCU_DD( 0x04c ) );
	PRINT( OUT_OBJ "[SCU]  70:%08lx  74:%08lx  7c:%08lx\n",           eng->reg.SCU_070, eng->reg.SCU_074, eng->reg.SCU_07c );
	PRINT( OUT_OBJ "[SCU]  80:%08lx  88:%08lx  90:%08lx  f0:%08lx\n", eng->reg.SCU_080, eng->reg.SCU_088, eng->reg.SCU_090, eng->reg.SCU_0f0 );
#ifdef AST1010_CHIP
	PRINT( OUT_OBJ "[SCU]  a4:%08lx  ac:%08lx\n",                     Read_Reg_SCU_DD( 0x0a4 ), eng->reg.SCU_0ac );
#elif defined(AST2500_IOMAP)
	PRINT( OUT_OBJ "[SCU]  a0:%08lx  a4:%08lx  b8:%08lx  bc:%08lx\n", Read_Reg_SCU_DD( 0x0a0 ), Read_Reg_SCU_DD( 0x0a4 ), eng->reg.SCU_0b8, eng->reg.SCU_0bc );
#else
	PRINT( OUT_OBJ "[SCU]  a4:%08lx\n",                               Read_Reg_SCU_DD( 0x0a4 ) );
#endif
#ifdef AST2500_IOMAP
	PRINT( OUT_OBJ "[SCU] 13c:%08lx 140:%08lx 144:%08lx 1dc:%08lx\n", Read_Reg_SCU_DD( 0x13c ), Read_Reg_SCU_DD( 0x140 ), Read_Reg_SCU_DD( 0x144 ), Read_Reg_SCU_DD( 0x1dc ) );
#endif
	PRINT( OUT_OBJ "[WDT]  0c:%08lx  2c:%08lx\n", eng->reg.WDT_00c, eng->reg.WDT_02c );
	PRINT( OUT_OBJ "[MAC]  08:%08lx  0c:%08lx\n", eng->reg.MAC_008, eng->reg.MAC_00c );
	PRINT( OUT_OBJ "[MAC]  A0|%08lx %08lx %08lx %08lx\n", Read_Reg_MAC_DD( eng, 0xa0 ), Read_Reg_MAC_DD( eng, 0xa4 ), Read_Reg_MAC_DD( eng, 0xa8 ), Read_Reg_MAC_DD( eng, 0xac ) );
	PRINT( OUT_OBJ "[MAC]  B0|%08lx %08lx %08lx %08lx\n", Read_Reg_MAC_DD( eng, 0xb0 ), Read_Reg_MAC_DD( eng, 0xb4 ), Read_Reg_MAC_DD( eng, 0xb8 ), Read_Reg_MAC_DD( eng, 0xbc ) );
	PRINT( OUT_OBJ "[MAC]  C0|%08lx %08lx %08lx\n",       Read_Reg_MAC_DD( eng, 0xc0 ), Read_Reg_MAC_DD( eng, 0xc4 ), Read_Reg_MAC_DD( eng, 0xc8 ) );

#ifdef SLT_UBOOT
#elif defined(LinuxAP)
#else
	PRINT( OUT_OBJ "Time: %s", ctime( &(eng->timestart) ));
	time( &timecur );
	PRINT( OUT_OBJ "----> %s", ctime( &timecur ) );
#endif
} // End void FPri_RegValue (MAC_ENGINE *eng, BYTE *fp)

//------------------------------------------------------------
void FPri_End (MAC_ENGINE *eng, BYTE option) {
	FILE_VAR
	GET_OBJ( option )

	if ( !eng->run.TM_RxDataEn ) {
	}
	else if ( eng->flg.Err_Flag )
		PRINT( OUT_OBJ "                    \n----> fail !!!\n" );
	else
		PRINT( OUT_OBJ "                    \n----> All Pass !!!\n" );

	if ( eng->ModeSwitch == MODE_DEDICATED ) {
		if ( eng->arg.GPHYADR != eng->phy.Adr )
			PRINT( OUT_OBJ "\n[Warning] PHY Address change from %d to %d !!!\n", eng->arg.GPHYADR, eng->phy.Adr );
	}

#ifdef AST1010_CHIP
	if ( eng->io.Str_reg_value ) {
		PRINT( OUT_OBJ "\n[Warning] SCU%02lX[%2d:%2d] == 0x%02lx is not the suggestion value 0.\n", eng->io.Str_reg_idx, eng->io.Str_reg_Hbit, eng->io.Str_reg_Lbit, eng->io.Str_reg_value );
		PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
	}

	eng->reg.SCU_048_default = SCU_48h_AST1010 & 0x01000f00;
	if ( ( eng->reg.SCU_048_check != eng->reg.SCU_048_default ) ) {
		PRINT( OUT_OBJ "\n[Warning] SCU48 == 0x%08lx is not the suggestion value 0x%08lx.\n", eng->reg.SCU_048, eng->reg.SCU_048_default );
		PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
	}
#else
	if ( eng->env.AST2300 ) {
		if ( eng->io.Str_reg_value ) {
			PRINT( OUT_OBJ "\n[Warning] SCU%02lX[%2d:%2d] == 0x%02lx is not the suggestion value 0.\n", eng->io.Str_reg_idx, eng->io.Str_reg_Hbit, eng->io.Str_reg_Lbit, eng->io.Str_reg_value );
			PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
		}

  #ifdef AST2500_IOMAP
		eng->reg.SCU_048_default = SCU_48h_AST2500 & 0x03ffffff;
  #else
		if ( eng->env.MAC34_vld )
			eng->reg.SCU_048_default = SCU_48h_AST2300 & 0x0fffffff;
		else
			eng->reg.SCU_048_default = SCU_48h_AST2300 & 0x0300ffff;
  #endif

		if ( ( eng->reg.SCU_048_check != eng->reg.SCU_048_default ) ) {
			PRINT( OUT_OBJ "\n[Warning] SCU48 == 0x%08lx is not the suggestion value 0x%08lx.\n", eng->reg.SCU_048, eng->reg.SCU_048_default );
			PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
		}
  #ifdef AST2500_IOMAP
		if ( eng->env.AST2500A1 ) {
			if ( ( eng->reg.SCU_0b8 != SCU_B8h_AST2500 ) ) {
				PRINT( OUT_OBJ "\n[Warning] SCUB8 == 0x%08lx is not the suggestion value 0x%08lx.\n", eng->reg.SCU_0b8, SCU_B8h_AST2500 );
				PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
			}
			if ( ( eng->reg.SCU_0bc != SCU_BCh_AST2500 ) ) {
				PRINT( OUT_OBJ "\n[Warning] SCUBC == 0x%08lx is not the suggestion value 0x%08lx.\n", eng->reg.SCU_0bc, SCU_BCh_AST2500 );
				PRINT( OUT_OBJ "          This change at this platform must been proven again by the ASPEED.\n" );
			}
		}
  #endif
	} // End if ( eng->env.AST2300 )
#endif

	if ( eng->ModeSwitch == MODE_NSCI ) {
		PRINT( OUT_OBJ "\n[Arg] %d %d %d %d %d %ld (%s)\n", eng->arg.GRun_Mode, eng->arg.GPackageTolNum, eng->arg.GChannelTolNum, eng->arg.GTestMode, eng->arg.GChk_TimingBund, ( eng->arg.GARPNumCnt | (ULONG)eng->arg.GEn_PrintNCSI ), eng->env.ASTChipName );

		switch ( eng->ncsi_cap.PCI_DID_VID ) {
			case PCI_DID_VID_Intel_82574L        : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82574L       \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10d6    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10a7    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82575_10a9    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82575        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10c9    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e6    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e7    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_10e8    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_1518    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_1526    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_150a    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82576_150d    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82576        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82599_10fb    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82599        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_82599_1557    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel 82599        \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I210          : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I210         \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I350_1521     : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I350         \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_I350_1523     : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel I350         \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Intel_X540          : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel X540         \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5718    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5718   \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5719    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5719   \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5720    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5720   \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM5725    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM5725   \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Broadcom_BCM57810S  : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom BCM57810S \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			case PCI_DID_VID_Mellanox_ConnectX_3 : PRINT( OUT_OBJ "[NC]%08lx %08lx: Mellanox ConnectX-3\n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			default:
			switch ( eng->ncsi_cap.ManufacturerID ) {
				case ManufacturerID_Intel    : PRINT( OUT_OBJ "[NC]%08lx %08lx: Intel              \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
				case ManufacturerID_Broadcom : PRINT( OUT_OBJ "[NC]%08lx %08lx: Broadcom           \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
				case ManufacturerID_Mellanox : PRINT( OUT_OBJ "[NC]%08lx %08lx: Mellanox           \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
				default                      : PRINT( OUT_OBJ "[NC]%08lx %08lx                     \n", eng->ncsi_cap.ManufacturerID, eng->ncsi_cap.PCI_DID_VID ); break;
			} // End switch ( eng->ncsi_cap.ManufacturerID )
		} // End switch ( eng->ncsi_cap.PCI_DID_VID )
	}
	else {
		if (eng->arg.GLOOP_INFINI) PRINT( OUT_OBJ "\n[Arg] %d %d %d # %d %d %d %lx (%s)[%d %d %d]\n"  , eng->arg.GRun_Mode, eng->arg.GSpeed, eng->arg.GCtrl,                     eng->arg.GTestMode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund, eng->arg.GUserDVal, eng->env.ASTChipName, eng->run.Loop_rl[0], eng->run.Loop_rl[1], eng->run.Loop_rl[2] );
		else                       PRINT( OUT_OBJ "\n[Arg] %d %d %d %ld %d %d %d %lx (%s)[%d %d %d]\n", eng->arg.GRun_Mode, eng->arg.GSpeed, eng->arg.GCtrl, eng->arg.GLOOP_MAX, eng->arg.GTestMode, eng->arg.GPHYADR, eng->arg.GChk_TimingBund, eng->arg.GUserDVal, eng->env.ASTChipName, eng->run.Loop_rl[0], eng->run.Loop_rl[1], eng->run.Loop_rl[2] );

		PRINT( OUT_OBJ "[PHY] Adr:%d ID2:%04lx ID3:%04lx (%s)\n", eng->phy.Adr, eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName );
	} // End if ( eng->ModeSwitch == MODE_NSCI )

#ifdef SUPPORT_PHY_LAN9303
	PRINT( OUT_OBJ "[Ver II] %s (for LAN9303 with I2C%d)\n", version_name, LAN9303_I2C_BUSNUM );
#else
	PRINT( OUT_OBJ "[Ver II] %s\n", version_name );
#endif
} // End void FPri_End (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void FPri_ErrFlag (MAC_ENGINE *eng, BYTE option) {

	FILE_VAR
	GET_OBJ( option )

	if ( eng->flg.Wrn_Flag && eng->flg.Err_Flag_PrintEn ) {
		if ( eng->flg.Wrn_Flag &Wrn_IOMarginOUF ) {
			PRINT( OUT_OBJ "[Warning] IO timing testing range out of boundary\n" );
			if ( eng->env.MAC_RMII )
				PRINT( OUT_OBJ "      (%d,%d): %dx1 [%d:%d]x[%d]\n",     eng->io.Dly_in_reg_idx, eng->io.Dly_out_reg_idx, eng->run.IO_Bund,                   eng->io.Dly_in_min, eng->io.Dly_in_max, eng->io.Dly_out_min );
			else
				PRINT( OUT_OBJ "      (%d,%d): %dx%d [%d:%d]x[%d:%d]\n", eng->io.Dly_in_reg_idx, eng->io.Dly_out_reg_idx, eng->run.IO_Bund, eng->run.IO_Bund, eng->io.Dly_in_min, eng->io.Dly_in_max, eng->io.Dly_out_min, eng->io.Dly_out_max );
		} // End if ( eng->flg.Wrn_Flag & Wrn_IOMarginOUF )
	}

	if ( eng->flg.Err_Flag && eng->flg.Err_Flag_PrintEn ) {
		PRINT( OUT_OBJ "\n\n" );
//PRINT( OUT_OBJ "Err_Flag: %x\n\n", eng->flg.Err_Flag );

		if ( eng->flg.Err_Flag & Err_PHY_Type                ) PRINT( OUT_OBJ "[Err] Unidentifiable PHY                                     \n" );
		if ( eng->flg.Err_Flag & Err_MALLOC_FrmSize          ) PRINT( OUT_OBJ "[Err] Malloc fail at frame size buffer                       \n" );
		if ( eng->flg.Err_Flag & Err_MALLOC_LastWP           ) PRINT( OUT_OBJ "[Err] Malloc fail at last WP buffer                          \n" );
		if ( eng->flg.Err_Flag & Err_Check_Buf_Data          ) PRINT( OUT_OBJ "[Err] Received data mismatch                                 \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_Check_TxOwnTimeOut ) PRINT( OUT_OBJ "[Err] Time out of checking Tx owner bit in NCSI packet       \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_Check_RxOwnTimeOut ) PRINT( OUT_OBJ "[Err] Time out of checking Rx owner bit in NCSI packet       \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_Check_ARPOwnTimeOut) PRINT( OUT_OBJ "[Err] Time out of checking ARP owner bit in NCSI packet      \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_No_PHY             ) PRINT( OUT_OBJ "[Err] Can not find NCSI PHY                                  \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_Channel_Num        ) PRINT( OUT_OBJ "[Err] NCSI Channel Number Mismatch                           \n" );
		if ( eng->flg.Err_Flag & Err_NCSI_Package_Num        ) PRINT( OUT_OBJ "[Err] NCSI Package Number Mismatch                           \n" );
		if ( eng->flg.Err_Flag & Err_PHY_TimeOut_RW          ) PRINT( OUT_OBJ "[Err] Time out of read/write PHY register                    \n" );
		if ( eng->flg.Err_Flag & Err_PHY_TimeOut_Rst         ) PRINT( OUT_OBJ "[Err] Time out of reset PHY register                         \n" );
		if ( eng->flg.Err_Flag & Err_RXBUF_UNAVA             ) PRINT( OUT_OBJ "[Err] MAC00h[2]:Receiving buffer unavailable                 \n" );
		if ( eng->flg.Err_Flag & Err_RPKT_LOST               ) PRINT( OUT_OBJ "[Err] MAC00h[3]:Received packet lost due to RX FIFO full     \n" );
		if ( eng->flg.Err_Flag & Err_NPTXBUF_UNAVA           ) PRINT( OUT_OBJ "[Err] MAC00h[6]:Normal priority transmit buffer unavailable  \n" );
		if ( eng->flg.Err_Flag & Err_TPKT_LOST               ) PRINT( OUT_OBJ "[Err] MAC00h[7]:Packets transmitted to Ethernet lost         \n" );
		if ( eng->flg.Err_Flag & Err_DMABufNum               ) PRINT( OUT_OBJ "[Err] DMA Buffer is not enough                               \n" );
		if ( eng->flg.Err_Flag & Err_IOMargin                ) PRINT( OUT_OBJ "[Err] IO timing margin is not enough                         \n" );

		if ( eng->flg.Err_Flag & Err_MHCLK_Ratio             ) {
#ifdef AST1010_CHIP
			PRINT( OUT_OBJ "[Err] Error setting of MAC AHB bus clock (SCU08[13:12])      \n" );
			PRINT( OUT_OBJ "      SCU08[13:12] == 0x%01lx is not the suggestion value 0.\n", eng->env.MHCLK_Ratio );
#elif defined(AST2500_IOMAP)
			PRINT( OUT_OBJ "[Err] Error setting of MAC AHB bus clock (SCU08[18:16])      \n" );
			PRINT( OUT_OBJ "      SCU08[18:16] == 0x%01lx is not the suggestion value 4.\n", eng->env.MHCLK_Ratio );
#else
			PRINT( OUT_OBJ "[Err] Error setting of MAC AHB bus clock (SCU08[18:16])      \n" );
			if ( eng->env.MAC_atlast_1Gvld )
				PRINT( OUT_OBJ "      SCU08[18:16] == 0x%01lx is not the suggestion value 2.\n", eng->env.MHCLK_Ratio );
			else
				PRINT( OUT_OBJ "      SCU08[18:16] == 0x%01lx is not the suggestion value 4.\n", eng->env.MHCLK_Ratio );
#endif
		} // End if ( eng->flg.Err_Flag & Err_MHCLK_Ratio             )

		if ( eng->flg.Err_Flag & Err_IOMarginOUF ) {
			PRINT( OUT_OBJ "[Err] IO timing testing range out of boundary\n");
			if ( eng->env.MAC_RMII )
				PRINT( OUT_OBJ "      (%d,%d): %dx1 [%d:%d]x[%d]\n",     eng->io.Dly_in_reg_idx, eng->io.Dly_out_reg_idx, eng->run.IO_Bund,                   eng->io.Dly_in_min, eng->io.Dly_in_max, eng->io.Dly_out_min );
			else
				PRINT( OUT_OBJ "      (%d,%d): %dx%d [%d:%d]x[%d:%d]\n", eng->io.Dly_in_reg_idx, eng->io.Dly_out_reg_idx, eng->run.IO_Bund, eng->run.IO_Bund, eng->io.Dly_in_min, eng->io.Dly_in_max, eng->io.Dly_out_min, eng->io.Dly_out_max );
		} // End if ( eng->flg.Err_Flag & Err_IOMarginOUF )

		if ( eng->flg.Err_Flag & Err_Check_Des ) {
			PRINT( OUT_OBJ "[Err] Descriptor error\n");
			if ( eng->flg.Check_Des_Val & Check_Des_TxOwnTimeOut ) PRINT( OUT_OBJ "[Des] Time out of checking Tx owner bit\n" );
			if ( eng->flg.Check_Des_Val & Check_Des_RxOwnTimeOut ) PRINT( OUT_OBJ "[Des] Time out of checking Rx owner bit\n" );
			if ( eng->flg.Check_Des_Val & Check_Des_FrameLen     ) PRINT( OUT_OBJ "[Des] Frame length mismatch            \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_RxErr        ) PRINT( OUT_OBJ "[Des] Input signal RxErr               \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_CRC          ) PRINT( OUT_OBJ "[Des] CRC error of frame               \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_FTL          ) PRINT( OUT_OBJ "[Des] Frame too long                   \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_Runt         ) PRINT( OUT_OBJ "[Des] Runt packet                      \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_OddNibble    ) PRINT( OUT_OBJ "[Des] Nibble bit happen                \n" );
			if ( eng->flg.Check_Des_Val & Check_Des_RxFIFOFull   ) PRINT( OUT_OBJ "[Des] Rx FIFO full                     \n" );
		} // End if ( eng->flg.Err_Flag & Err_Check_Des )

		if ( eng->flg.Err_Flag & Err_MACMode ) {
			PRINT( OUT_OBJ "[Err] MAC interface mode mismatch\n" );
#ifndef AST1010_CHIP
			if ( eng->env.AST2300 ) {
				switch ( eng->env.MAC_Mode ) {
					case 0 : PRINT( OUT_OBJ "      SCU70h[7:6] == 0: [MAC#1] RMII   [MAC#2] RMII \n" ); break;
					case 1 : PRINT( OUT_OBJ "      SCU70h[7:6] == 1: [MAC#1] RGMII  [MAC#2] RMII \n" ); break;
					case 2 : PRINT( OUT_OBJ "      SCU70h[7:6] == 2: [MAC#1] RMII   [MAC#2] RGMII\n" ); break;
					case 3 : PRINT( OUT_OBJ "      SCU70h[7:6] == 3: [MAC#1] RGMII  [MAC#2] RGMII\n" ); break;
				}
			}
			else {
				switch ( eng->env.MAC_Mode ) {
					case 0 : PRINT( OUT_OBJ "      SCU70h[8:6] == 000: [MAC#1] GMII               \n" ); break;
					case 1 : PRINT( OUT_OBJ "      SCU70h[8:6] == 001: [MAC#1] MII    [MAC#2] MII \n" ); break;
					case 2 : PRINT( OUT_OBJ "      SCU70h[8:6] == 010: [MAC#1] RMII   [MAC#2] MII \n" ); break;
					case 3 : PRINT( OUT_OBJ "      SCU70h[8:6] == 011: [MAC#1] MII                \n" ); break;
					case 4 : PRINT( OUT_OBJ "      SCU70h[8:6] == 100: [MAC#1] RMII               \n" ); break;
					case 5 : PRINT( OUT_OBJ "      SCU70h[8:6] == 101: Reserved                   \n" ); break;
					case 6 : PRINT( OUT_OBJ "      SCU70h[8:6] == 110: [MAC#1] RMII   [MAC#2] RMII\n" ); break;
					case 7 : PRINT( OUT_OBJ "      SCU70h[8:6] == 111: Disable MAC                \n" ); break;
				}
			} // End if ( eng->env.AST2300 )
#endif
		} // End if ( eng->flg.Err_Flag & Err_MACMode )

		if ( eng->ModeSwitch == MODE_NSCI ) {
			if ( eng->flg.Err_Flag & Err_NCSI_LinkFail ) {
				PRINT( OUT_OBJ "[Err] NCSI packet retry number over flows when find channel\n" );

				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Get_Version_ID         ) PRINT( OUT_OBJ "[NCSI] Time out when Get Version ID         \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Get_Capabilities       ) PRINT( OUT_OBJ "[NCSI] Time out when Get Capabilities       \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Select_Active_Package  ) PRINT( OUT_OBJ "[NCSI] Time out when Select Active Package  \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Set_MAC_Address ) PRINT( OUT_OBJ "[NCSI] Time out when Enable Set MAC Address \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Broadcast_Filter) PRINT( OUT_OBJ "[NCSI] Time out when Enable Broadcast Filter\n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Network_TX      ) PRINT( OUT_OBJ "[NCSI] Time out when Enable Network TX      \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Enable_Channel         ) PRINT( OUT_OBJ "[NCSI] Time out when Enable Channel         \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Disable_Network_TX     ) PRINT( OUT_OBJ "[NCSI] Time out when Disable Network TX     \n" );
				if ( eng->flg.NCSI_LinkFail_Val & NCSI_LinkFail_Disable_Channel        ) PRINT( OUT_OBJ "[NCSI] Time out when Disable Channel        \n" );
			}

			if ( eng->flg.Err_Flag & Err_NCSI_Channel_Num ) PRINT( OUT_OBJ "[NCSI] Channel number expected: %d, real: %d\n", eng->arg.GChannelTolNum, eng->dat.number_chl );
			if ( eng->flg.Err_Flag & Err_NCSI_Package_Num ) PRINT( OUT_OBJ "[NCSI] Peckage number expected: %d, real: %d\n", eng->arg.GPackageTolNum, eng->dat.number_pak );
		} // End if ( eng->ModeSwitch == MODE_NSCI )
	} // End if ( eng->flg.Err_Flag && eng->flg.Err_Flag_PrintEn )
} // End void FPri_ErrFlag (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void Finish_Close (MAC_ENGINE *eng) {

#ifdef Enable_RecovSCU
	if ( eng->reg.SCU_oldvld )
		recov_scu( eng );
#endif

#if defined(ENABLE_LOG_FILE)
	if ( eng->fp_io && eng->run.TM_IOTiming )
		fclose( eng->fp_io );

	if ( eng->fp_log )
		fclose( eng->fp_log );
#endif
} // End void Finish_Close (MAC_ENGINE *eng)

//------------------------------------------------------------
char Finish_Check (MAC_ENGINE *eng, int value) {

#ifdef Disable_VGA
	if ( eng->env.VGAModeVld ) {
		outp(0x3d4, 0x17);
		outp(0x3d5, eng->env.VGAMode);
	}
#endif
#ifdef  DbgPrn_FuncHeader
	printf("Finish_Check\n");
	Debug_delay();
#endif

	if ( eng->dat.FRAME_LEN )
		free( eng->dat.FRAME_LEN );

	if ( eng->dat.wp_lst )
		free(eng->dat.wp_lst );

	eng->flg.Err_Flag = eng->flg.Err_Flag | value;

	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08lx]\n", eng->flg.Err_Flag);

	if ( !eng->run.TM_Burst )
		FPri_ErrFlag( eng, FP_LOG );

	if ( eng->run.TM_IOTiming )
		FPri_ErrFlag( eng, FP_IO );

	FPri_ErrFlag( eng, STD_OUT );

	if ( !eng->run.TM_Burst )
		FPri_End( eng, FP_LOG );

	if ( eng->run.TM_IOTiming )
		FPri_End( eng, FP_IO );

	FPri_End( eng, STD_OUT );


	if ( !eng->run.TM_Burst )
		FPri_RegValue( eng, FP_LOG );
	if ( eng->run.TM_IOTiming )
		FPri_RegValue( eng, FP_IO  );

	Finish_Close( eng );

	if ( eng->flg.Err_Flag )
		return( 1 );
	else
		return( 0 );
} // End char Finish_Check (MAC_ENGINE *eng, int value)

//------------------------------------------------------------
int FindErr (MAC_ENGINE *eng, int value) {
	eng->flg.Err_Flag = eng->flg.Err_Flag | value;

	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08lx]\n", eng->flg.Err_Flag);

	return(1);
}

//------------------------------------------------------------
int FindErr_Des (MAC_ENGINE *eng, int value) {
	eng->flg.Check_Des_Val = eng->flg.Check_Des_Val | value;
	eng->flg.Err_Flag      = eng->flg.Err_Flag | Err_Check_Des;
	if ( DbgPrn_ErrFlg )
		printf("\nErr_Flag: [%08lx] Check_Des_Val: [%08lx]\n", eng->flg.Err_Flag, eng->flg.Check_Des_Val);

	return(1);
}

//------------------------------------------------------------
// Get and Check status of Interrupt
//------------------------------------------------------------
int check_int (MAC_ENGINE *eng, char *type ) {
#ifdef  DbgPrn_FuncHeader
	printf("check_int  : %d\n", eng->run.Loop);
	Debug_delay();
#endif

	eng->reg.MAC_000 = Read_Reg_MAC_DD( eng, 0x00 );//Interrupt Status
#ifdef CheckRxbufUNAVA
	if ( eng->reg.MAC_000 & 0x00000004 ) {
  #if defined(ENABLE_LOG_FILE)
		fprintf( eng->fp_log, "[%sIntStatus] Receiving buffer unavailable               : %08lx [loop:%d]\n", type, eng->reg.MAC_000, eng->run.Loop );
  #endif
		FindErr( eng, Err_RXBUF_UNAVA );
	}
#endif

#ifdef CheckRPktLost
	if ( eng->reg.MAC_000 & 0x00000008 ) {
  #if defined(ENABLE_LOG_FILE)
		fprintf( eng->fp_log, "[%sIntStatus] Received packet lost due to RX FIFO full   : %08lx [loop:%d]\n", type, eng->reg.MAC_000, eng->run.Loop );
  #endif
		FindErr( eng, Err_RPKT_LOST );
	}
#endif

#ifdef CheckNPTxbufUNAVA
	if ( eng->reg.MAC_000 & 0x00000040 ) {
  #if defined(ENABLE_LOG_FILE)
		fprintf( eng->fp_log, "[%sIntStatus] Normal priority transmit buffer unavailable: %08lx [loop:%d]\n", type, eng->reg.MAC_000, eng->run.Loop );
  #endif
		FindErr( eng, Err_NPTXBUF_UNAVA );
	}
#endif

#ifdef CheckTPktLost
	if ( eng->reg.MAC_000 & 0x00000080 ) {
  #if defined(ENABLE_LOG_FILE)
		fprintf( eng->fp_log, "[%sIntStatus] Packets transmitted to Ethernet lost       : %08lx [loop:%d]\n", type, eng->reg.MAC_000, eng->run.Loop );
  #endif
		FindErr( eng, Err_TPKT_LOST );
	}
#endif

	if ( eng->flg.Err_Flag )
		return(1);
	else
		return(0);
} // End int check_int (MAC_ENGINE *eng, char *type)


//------------------------------------------------------------
// Buffer
//------------------------------------------------------------
void setup_framesize (MAC_ENGINE *eng) {
	int        des_num;

#ifdef  DbgPrn_FuncHeader
	printf("setup_framesize\n");
	Debug_delay();
#endif

	//------------------------------
	// Fill Frame Size out descriptor area
	//------------------------------
#ifdef SLT_UBOOT
	if (0)
#else
	if ( ENABLE_RAND_SIZE )
#endif
	{
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			if ( RAND_SIZE_SIMPLE )
				switch( rand() % 5 ) {
					case 0 : eng->dat.FRAME_LEN[ des_num ] = 0x4e ; break;
					case 1 : eng->dat.FRAME_LEN[ des_num ] = 0x4ba; break;
					default: eng->dat.FRAME_LEN[ des_num ] = 0x5ea; break;
				}
			else
				eng->dat.FRAME_LEN[ des_num ] = ( rand() + RAND_SIZE_MIN ) % ( RAND_SIZE_MAX + 1 );
#if defined(ENABLE_LOG_FILE)
			if ( DbgPrn_FRAME_LEN )
				fprintf( eng->fp_log, "[setup_framesize] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.Loop );
#endif
		}
	}
	else {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
#ifdef SelectSimpleLength
			if ( des_num % FRAME_SELH_PERD )
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
			else
				eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
#else
			if ( eng->run.TM_Burst ) {
				if ( eng->run.TM_IEEE )
					eng->dat.FRAME_LEN[ des_num ] = 1514;
				else
  #ifdef ENABLE_ARP_2_WOL
					eng->dat.FRAME_LEN[ des_num ] = 164;
  #else
					eng->dat.FRAME_LEN[ des_num ] = 60;
  #endif
			}
			else {
  #ifdef SelectLengthInc
				eng->dat.FRAME_LEN[ des_num ] = 1514 - ( des_num % 1455 );
  #else
				if ( des_num % FRAME_SELH_PERD )
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENH;
				else
					eng->dat.FRAME_LEN[ des_num ] = FRAME_LENL;
  #endif
			} // End if ( eng->run.TM_Burst )
#endif
#if defined(ENABLE_LOG_FILE)
			if ( DbgPrn_FRAME_LEN )
				fprintf( eng->fp_log, "[setup_framesize] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", eng->dat.FRAME_LEN[ des_num ], des_num, eng->run.Loop );
#endif
		} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
	} // End if ( ENABLE_RAND_SIZE )

	// Calculate average of frame size
#ifdef Enable_ShowBW
	eng->dat.Total_frame_len = 0;

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		eng->dat.Total_frame_len += eng->dat.FRAME_LEN[ des_num ];
#endif

	//------------------------------
	// Write Plane
	//------------------------------
	switch( ZeroCopy_OFFSET & 0x3 ) {
		case 0: eng->dat.wp_fir = 0xffffffff; break;
		case 1: eng->dat.wp_fir = 0xffffff00; break;
		case 2: eng->dat.wp_fir = 0xffff0000; break;
		case 3: eng->dat.wp_fir = 0xff000000; break;
	}

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ )
		switch( ( ZeroCopy_OFFSET + eng->dat.FRAME_LEN[ des_num ] - 1 ) & 0x3 ) {
			case 0: eng->dat.wp_lst[ des_num ] = 0x000000ff; break;
			case 1: eng->dat.wp_lst[ des_num ] = 0x0000ffff; break;
			case 2: eng->dat.wp_lst[ des_num ] = 0x00ffffff; break;
			case 3: eng->dat.wp_lst[ des_num ] = 0xffffffff; break;
		}
} // End void setup_framesize (void)

//------------------------------------------------------------
void setup_arp (MAC_ENGINE *eng) {
	int        i;

	for (i = 0; i < 16; i++ )
		eng->dat.ARP_data[ i ] = ARP_org_data[ i ];

	eng->dat.ARP_data[ 1 ] = 0x0000ffff
	                       | ( eng->inf.SA[ 0 ] << 16 )
	                       | ( eng->inf.SA[ 1 ] << 24 );

	eng->dat.ARP_data[ 2 ] = ( eng->inf.SA[ 2 ]       )
	                       | ( eng->inf.SA[ 3 ] <<  8 )
	                       | ( eng->inf.SA[ 4 ] << 16 )
	                       | ( eng->inf.SA[ 5 ] << 24 );

	eng->dat.ARP_data[ 5 ] = 0x00000100
	                       | ( eng->inf.SA[ 0 ] << 16 )
	                       | ( eng->inf.SA[ 1 ] << 24 );

	eng->dat.ARP_data[ 6 ] = ( eng->inf.SA[ 2 ]       )
	                       | ( eng->inf.SA[ 3 ] <<  8 )
	                       | ( eng->inf.SA[ 4 ] << 16 )
	                       | ( eng->inf.SA[ 5 ] << 24 );
} // End void setup_arp (MAC_ENGINE *eng)

//------------------------------------------------------------
void setup_buf (MAC_ENGINE *eng) {
	int        des_num;
	int        i;
	ULONG      adr;
	ULONG      adr_srt;
	ULONG      adr_end;
	ULONG      len;
	int        cnt;
	ULONG      Current_framelen;
	ULONG      gdata;

#ifdef ENABLE_ARP_2_WOL
	int        DA[3];

	DA[ 0 ] =  ( ( SelectWOLDA_DatH >>  8 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatH <<  8 ) & 0xff00 );

	DA[ 1 ] =  ( ( SelectWOLDA_DatL >> 24 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatL >>  8 ) & 0xff00 );

	DA[ 2 ] =  ( ( SelectWOLDA_DatL >>  8 ) & 0x00ff ) |
	           ( ( SelectWOLDA_DatL <<  8 ) & 0xff00 );
#endif

#ifdef  DbgPrn_FuncHeader
	printf("setup_buf  : %d\n", eng->run.Loop);
	Debug_delay();
#endif

	// It need be multiple of 4
	eng->dat.DMA_Base_Setup = GET_DMA_BASE_SETUP & 0xfffffffc;
	adr_srt = eng->dat.DMA_Base_Setup;

	for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
		if ( DbgPrn_BufAdr )
			printf("[loop:%4d][des:%4d][setup_buf  ] %08lx\n", eng->run.Loop, des_num, adr_srt);

		if ( eng->run.TM_Burst ) {
			if ( eng->run.TM_IEEE ) {
#ifdef ENABLE_DASA
				Write_Mem_Dat_DD( adr_srt    , 0xffffffff           );
				Write_Mem_Dat_DD( adr_srt + 4, eng->dat.ARP_data[1] );
				Write_Mem_Dat_DD( adr_srt + 8, eng->dat.ARP_data[2] );

				for ( adr = (adr_srt + 12); adr < (adr_srt + DMA_PakSize); adr += 4 )
#else
				for ( adr =  adr_srt;       adr < (adr_srt + DMA_PakSize); adr += 4 )
#endif
				{
					switch( eng->arg.GTestMode ) {
						case 1: gdata = 0xffffffff;              break;
						case 2: gdata = 0x55555555;              break;
						case 3: gdata = rand() | (rand() << 16); break;
						case 5: gdata = eng->arg.GUserDVal;      break;
					}
					Write_Mem_Dat_DD( adr, gdata );
				} // End for()
			}
			else {
				for (i = 0; i < 16; i++)
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ), eng->dat.ARP_data[i] );

#ifdef ENABLE_ARP_2_WOL
				for (i = 16; i < 40; i += 3) {
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ),     ( DA[ 1 ] << 16 ) |  DA[ 0 ] );
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ) + 4, ( DA[ 0 ] << 16 ) |  DA[ 2 ] );
					Write_Mem_Dat_DD( adr_srt + ( i << 2 ) + 8, ( DA[ 2 ] << 16 ) |  DA[ 1 ] );
				}
#endif
			} // End if ( eng->run.TM_IEEE )
		}
		else {
			// --------------------------------------------
#ifdef SelectSimpleData
  #ifdef SimpleData_Fix
			switch( des_num % SimpleData_FixNum ) {
				case  0 : gdata = SimpleData_FixVal00; break;
				case  1 : gdata = SimpleData_FixVal01; break;
				case  2 : gdata = SimpleData_FixVal02; break;
				case  3 : gdata = SimpleData_FixVal03; break;
				case  4 : gdata = SimpleData_FixVal04; break;
				case  5 : gdata = SimpleData_FixVal05; break;
				case  6 : gdata = SimpleData_FixVal06; break;
				case  7 : gdata = SimpleData_FixVal07; break;
				case  8 : gdata = SimpleData_FixVal08; break;
				case  9 : gdata = SimpleData_FixVal09; break;
				case 10 : gdata = SimpleData_FixVal10; break;
				default : gdata = SimpleData_FixVal11; break;
			}
  #else
			gdata   = 0x11111111 * ((des_num + SEED_START) % 256);
  #endif
#else
			gdata   = DATA_SEED( des_num + SEED_START );
#endif
			Current_framelen = eng->dat.FRAME_LEN[ des_num ];

#if defined(ENABLE_LOG_FILE)
			if ( DbgPrn_FRAME_LEN )
				fprintf( eng->fp_log, "[setup_buf      ] Current_framelen:%08lx[Des:%d][loop:%d]\n", Current_framelen, des_num, eng->run.Loop );
#endif

			cnt     = 0;
 			len     = ( ( ( Current_framelen - 14 ) & 0xff ) << 8) |
 			            ( ( Current_framelen - 14 ) >> 8 );
 			adr_end = adr_srt + DMA_PakSize;
			for ( adr = adr_srt; adr < adr_end; adr += 4 ) {
#ifdef SelectSimpleDA
				cnt++;
				if      ( cnt == 1 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat0 );
				else if ( cnt == 2 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat1 );
				else if ( cnt == 3 ) Write_Mem_Dat_DD( adr, SelectSimpleDA_Dat2 );
				else if ( cnt == 4 ) Write_Mem_Dat_DD( adr, len | (len << 16)   );
				else
#endif
				                     Write_Mem_Dat_DD( adr, gdata );
#ifdef SelectSimpleData
				gdata = gdata ^ SimpleData_XORVal;
#else
				gdata += DATA_IncVal;
#endif
			}
		} // End if ( eng->run.TM_Burst )

		adr_srt += DMA_PakSize;
	} // End for (des_num = 0; des_num < eng->dat.Des_Num; des_num++)
} // End void setup_buf (MAC_ENGINE *eng)

//------------------------------------------------------------
// Check data of one packet
//------------------------------------------------------------
char check_Data (MAC_ENGINE *eng, ULONG datbase, LONG number) {
	int        index;
	int        cnt;
	ULONG      rdata;
	ULONG      wp_lst_cur;
	ULONG      adr_las;
	ULONG      adr;
	ULONG      adr_srt;
	ULONG      adr_end;
	ULONG      len;
#ifdef SelectSimpleDA
	ULONG      gdata_bak;
#endif
	ULONG      gdata;
	ULONG      wp;

#ifdef  DbgPrn_FuncHeader
	printf("check_Data : %d\n", eng->run.Loop);
	Debug_delay();
#endif

	wp_lst_cur             = eng->dat.wp_lst[number];
	eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[number];
#if defined(ENABLE_LOG_FILE)
	if ( DbgPrn_FRAME_LEN )
		fprintf( eng->fp_log, "[check_Data     ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", eng->dat.FRAME_LEN_Cur, number, eng->run.Loop );
#endif
	adr_srt = datbase;
	adr_end = adr_srt + PktByteSize;

#ifdef SelectSimpleData
  #ifdef SimpleData_Fix
	switch( number % SimpleData_FixNum ) {
		case  0 : gdata = SimpleData_FixVal00; break;
		case  1 : gdata = SimpleData_FixVal01; break;
		case  2 : gdata = SimpleData_FixVal02; break;
		case  3 : gdata = SimpleData_FixVal03; break;
		case  4 : gdata = SimpleData_FixVal04; break;
		case  5 : gdata = SimpleData_FixVal05; break;
		case  6 : gdata = SimpleData_FixVal06; break;
		case  7 : gdata = SimpleData_FixVal07; break;
		case  8 : gdata = SimpleData_FixVal08; break;
		case  9 : gdata = SimpleData_FixVal09; break;
		case 10 : gdata = SimpleData_FixVal10; break;
		default : gdata = SimpleData_FixVal11; break;
	}
  #else
	gdata   = 0x11111111 * (( number + SEED_START ) % 256 );
  #endif
#else
	gdata   = DATA_SEED( number + SEED_START );
#endif

//printf("check_buf: %08lx - %08lx \n", adr_srt, adr_end);
	wp      = eng->dat.wp_fir;
	adr_las = adr_end - 4;
	cnt     = 0;
	len     = ((( eng->dat.FRAME_LEN_Cur-14 ) & 0xff ) << 8 ) |
	          ( ( eng->dat.FRAME_LEN_Cur-14 )          >> 8 );
#if defined(ENABLE_LOG_FILE)
	if ( DbgPrn_Bufdat )
		fprintf( eng->fp_log, " Inf:%08lx ~ %08lx(%08lx) %08lx [Des:%d][loop:%d]\n", adr_srt, adr_end, adr_las, gdata, number, eng->run.Loop );
#endif
	for ( adr = adr_srt; adr < adr_end; adr+=4 ) {

#ifdef SelectSimpleDA
		cnt++;
		if      ( cnt == 1 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat0; }
		else if ( cnt == 2 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat1; }
		else if ( cnt == 3 ) { gdata_bak = gdata; gdata = SelectSimpleDA_Dat2; }
		else if ( cnt == 4 ) { gdata_bak = gdata; gdata = len | (len << 16);   }
#endif
		rdata = Read_Mem_Dat_DD( adr );
		if ( adr == adr_las )
			wp = wp & wp_lst_cur;

		if ( ( rdata & wp ) != ( gdata & wp ) ) {
#if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "\nError: Adr:%08lx[%3d] (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.Loop );
			for ( index = 0; index < 6; index++ )
				fprintf( eng->fp_log, "Rep  : Adr:%08lx      (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, Read_Mem_Dat_DD( adr ), gdata, wp, number, eng->run.Loop );
#endif

			if ( DbgPrn_DumpMACCnt )
				dump_mac_ROreg( eng );

			return( FindErr( eng, Err_Check_Buf_Data ) );
		} // End if ( (rdata & wp) != (gdata & wp) )
#if defined(ENABLE_LOG_FILE)
		if ( DbgPrn_BufdatDetail )
			fprintf( eng->fp_log, " Adr:%08lx[%3d] (%08lx) (%08lx:%08lx) [Des:%d][loop:%d]\n", adr, ( adr - adr_srt ) / 4, rdata, gdata, wp, number, eng->run.Loop );
#endif
#ifdef SelectSimpleDA
		if ( cnt <= 4 )
			gdata = gdata_bak;
#endif

#ifdef SelectSimpleData
		gdata = gdata ^ SimpleData_XORVal;
#else
		gdata += DATA_IncVal;
#endif

		wp     = 0xffffffff;
	}
	return(0);
} // End char check_Data (MAC_ENGINE *eng, ULONG datbase, LONG number)

//------------------------------------------------------------
char check_buf (MAC_ENGINE *eng, int loopcnt) {
	int        des_num;
	ULONG      desadr;
	ULONG      datbase;

#ifdef  DbgPrn_FuncHeader
	printf("check_buf  : %d\n", eng->run.Loop);
	Debug_delay();
#endif

	desadr = eng->run.RDES_BASE + ( 16 * eng->dat.Des_Num ) - 4;
	for ( des_num = eng->dat.Des_Num - 1; des_num >= 0; des_num-- ) {
		datbase = Read_Mem_Des_DD( desadr ) & 0xfffffffc;
//printf("%d:%08lx\n", des_num, desadr);
		if ( check_Data( eng, datbase, des_num ) ) {
			check_int ( eng, "" );

			return(1);
		}
		desadr-= 16;
	}
	if ( check_int ( eng, "" ) )
		return(1);

#ifdef Delay_CheckData
	if ( ( loopcnt % Delay_CheckData_LoopNum ) == 0 ) DELAY( Delay_CheckData );
#endif
	return(0);
} // End char check_buf (MAC_ENGINE *eng, int loopcnt)

//------------------------------------------------------------
// Descriptor
//------------------------------------------------------------
void setup_txdes (MAC_ENGINE *eng, ULONG desadr, ULONG bufbase) {
	ULONG      bufadr;
	ULONG      desval;
	int        des_num;

#ifdef  DbgPrn_FuncHeader
	printf("setup_txdes: %d\n", eng->run.Loop);
	Debug_delay();
#endif

	bufadr = bufbase;

	if ( eng->run.TM_TxDataEn ) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
			desval                 = TDES_IniVal;
			Write_Mem_Des_DD( desadr + 0x04, 0      );
			Write_Mem_Des_DD( desadr + 0x08, 0      );
			Write_Mem_Des_DD( desadr + 0x0C, bufadr );
			Write_Mem_Des_DD( desadr       , desval );

#if defined(ENABLE_LOG_FILE)
			if ( DbgPrn_FRAME_LEN )
				fprintf( eng->fp_log, "[setup_txdes    ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", eng->dat.FRAME_LEN_Cur, des_num, eng->run.Loop );
#endif
			if ( DbgPrn_BufAdr )
				printf("[loop:%4d][des:%4d][setup_txdes] %08lx\n", eng->run.Loop, des_num, bufadr);

			desadr += 16;
			bufadr += DMA_PakSize;
		}
		Write_Mem_Des_DD( desadr - 0x10, desval | EOR_IniVal );
	}
	else {
		Write_Mem_Des_DD( desadr, 0);
	} // End if ( eng->run.TM_TxDataEn )
} // End void setup_txdes (ULONG desadr, ULONG bufbase)

//------------------------------------------------------------
void setup_rxdes (MAC_ENGINE *eng, ULONG desadr, ULONG bufbase) {
	ULONG      bufadr;
	ULONG      desval;
	int        des_num;

#ifdef  DbgPrn_FuncHeader
	printf("setup_rxdes: %ld\n", eng->run.Loop);
	Debug_delay();
#endif

	bufadr = bufbase;
	desval = RDES_IniVal;
	if ( eng->run.TM_RxDataEn ) {
		for ( des_num = 0; des_num < eng->dat.Des_Num; des_num++ ) {
			Write_Mem_Des_DD( desadr + 0x04, 0      );
			Write_Mem_Des_DD( desadr + 0x08, 0      );
			Write_Mem_Des_DD( desadr + 0x0C, bufadr );
			Write_Mem_Des_DD( desadr       , desval );

			if ( DbgPrn_BufAdr )
				printf("[loop:%4d][des:%4d][setup_rxdes] %08lx\n", eng->run.Loop, des_num, bufadr);

			desadr += 16;
			bufadr += DMA_PakSize;
		}
		Write_Mem_Des_DD( desadr - 0x10, desval | EOR_IniVal );
	}
	else {
		Write_Mem_Des_DD( desadr, 0x80000000 );
	} // End if ( eng->run.TM_RxDataEn )
} // End void setup_rxdes (ULONG desadr, ULONG bufbase)

//------------------------------------------------------------
// First setting TX and RX information
//------------------------------------------------------------
void setup_des (MAC_ENGINE *eng, ULONG bufnum) {

#ifdef  DbgPrn_FuncHeader
	printf("setup_rxdes: %ld\n", bufnum);
	Debug_delay();
#else
	if ( DbgPrn_BufAdr ) {
		printf("setup_des: %ld\n", bufnum);
		Debug_delay();
	}
#endif

	eng->dat.DMA_Base_Tx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + eng->dat.DMA_Base_Setup; // 20130730
	eng->dat.DMA_Base_Rx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + GET_DMA_BASE(0); // 20130730
#ifndef Enable_MAC_ExtLoop
	setup_txdes( eng, eng->run.TDES_BASE, eng->dat.DMA_Base_Tx );
#endif
	setup_rxdes( eng, eng->run.RDES_BASE, eng->dat.DMA_Base_Rx );
} // End void setup_des (ULONG bufnum)

//------------------------------------------------------------
// Move buffer point of TX and RX descriptor to next DMA buffer
//------------------------------------------------------------
void setup_des_loop (MAC_ENGINE *eng, ULONG bufnum) {
	int        des_num;
	ULONG      H_rx_desadr;
	ULONG      H_tx_desadr;
	ULONG      H_tx_bufadr;
	ULONG      H_rx_bufadr;

#ifdef  DbgPrn_FuncHeader
	printf("setup_rxdes_loop: %ld\n", bufnum);
	Debug_delay();
#else
	if ( DbgPrn_BufAdr ) {
		printf("setup_des_loop: %ld\n", bufnum);
		Debug_delay();
	}
#endif

	if ( eng->run.TM_RxDataEn ) {
		H_rx_bufadr = eng->dat.DMA_Base_Rx;
		H_rx_desadr = eng->run.RDES_BASE;
		for ( des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++ ) {
			Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
			Write_Mem_Des_DD( H_rx_desadr       , RDES_IniVal );

			if ( DbgPrn_BufAdr )
				printf("[loop:%4d][des:%4d][setup_rxdes] %08lx\n", eng->run.Loop, des_num, H_rx_desadr);

			H_rx_bufadr += DMA_PakSize;
			H_rx_desadr += 16;
		}
		if ( DbgPrn_BufAdr )
			printf("[loop:%4d][des:%4d][setup_rxdes] %08lx\n", eng->run.Loop, des_num, H_rx_desadr);
		Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
		Write_Mem_Des_DD( H_rx_desadr       , RDES_IniVal | EOR_IniVal );
	}

	if ( eng->run.TM_TxDataEn ) {
		H_tx_bufadr = eng->dat.DMA_Base_Tx;
		H_tx_desadr = eng->run.TDES_BASE;
		for ( des_num = 0; des_num < eng->dat.Des_Num - 1; des_num++ ) {
			eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
			Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
			Write_Mem_Des_DD( H_tx_desadr       , TDES_IniVal );

			if ( DbgPrn_BufAdr )
				printf("[loop:%4d][des:%4d][setup_txdes] %08lx\n", eng->run.Loop, des_num, H_tx_desadr);

			H_tx_bufadr += DMA_PakSize;
			H_tx_desadr += 16;
		}
		if ( DbgPrn_BufAdr )
			printf("[loop:%4d][des:%4d][setup_txdes] %08lx\n", eng->run.Loop, des_num, H_tx_desadr);
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[ des_num ];
		Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
		Write_Mem_Des_DD( H_tx_desadr       , TDES_IniVal | EOR_IniVal );
	}

	Write_Reg_MAC_DD( eng, 0x18, 0x00000000 ); // Tx Poll
	Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 ); // Rx Poll
} // End void setup_des_loop (ULONG bufnum)

//------------------------------------------------------------
char check_des_header_Tx (MAC_ENGINE *eng, char *type, ULONG adr, LONG desnum) {
	int        timeout = 0;

	eng->dat.TxDes0DW = Read_Mem_Des_DD( adr );

	while ( HWOwnTx( eng->dat.TxDes0DW ) ) {
		// we will run again, if transfer has not been completed.
		if ( ( eng->run.TM_Burst || eng->run.TM_RxDataEn ) && ( ++timeout > eng->run.TIME_OUT_Des ) ) {
#if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sTxDesOwn] Address %08lx = %08lx [Des:%d][loop:%d]\n", type, adr, eng->dat.TxDes0DW, desnum, eng->run.Loop );
#endif
			return( FindErr_Des( eng, Check_Des_TxOwnTimeOut ) );
		}
		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll
		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll

#ifdef Delay_ChkTxOwn
		DELAY( Delay_ChkTxOwn );
#endif
		eng->dat.TxDes0DW = Read_Mem_Des_DD( adr );
	}

	return(0);
} // End char check_des_header_Tx (MAC_ENGINE *eng, char *type, ULONG adr, LONG desnum)

//------------------------------------------------------------
char check_des_header_Rx (MAC_ENGINE *eng, char *type, ULONG adr, LONG desnum) {
#ifdef CheckRxOwn
	int        timeout = 0;

	eng->dat.RxDes0DW = Read_Mem_Des_DD( adr );

	while ( HWOwnRx( eng->dat.RxDes0DW ) ) {
		// we will run again, if transfer has not been completed.
		if ( eng->run.TM_TxDataEn && ( ++timeout > eng->run.TIME_OUT_Des ) ) {
  #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDesOwn] Address %08lx = %08lx [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, desnum, eng->run.Loop );
  #endif
			return( FindErr_Des( eng, Check_Des_RxOwnTimeOut ) );
		}
		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll
		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll

  #ifdef Delay_ChkRxOwn
		DELAY( Delay_ChkRxOwn );
  #endif
		eng->dat.RxDes0DW = Read_Mem_Des_DD( adr );
	};


  #ifdef CheckRxLen
    #if defined(ENABLE_LOG_FILE)
	if ( DbgPrn_FRAME_LEN )
		fprintf( eng->fp_log, "[%sRxDes          ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]\n", type, ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.Loop );
    #endif

	if ( ( eng->dat.RxDes0DW & 0x3fff ) != ( eng->dat.FRAME_LEN_Cur + 4 ) ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
    #if defined(ENABLE_LOG_FILE)
		fprintf( eng->fp_log, "[%sRxDes] Error Frame Length %08lx:%08lx %08lx(%4d/%4d) [Des:%d][loop:%d]\n",   type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, ( eng->dat.RxDes0DW & 0x3fff ), ( eng->dat.FRAME_LEN_Cur + 4 ), desnum, eng->run.Loop );
    #endif
		FindErr_Des( eng, Check_Des_FrameLen );
	}
  #endif // End CheckRxLen

	if ( eng->dat.RxDes0DW & Check_ErrMask_ALL ) {
		eng->dat.RxDes3DW = Read_Mem_Des_DD( adr + 12 );
  #ifdef CheckRxErr
		if ( eng->dat.RxDes0DW & Check_ErrMask_RxErr ) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Error RxErr        %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_RxErr );
		}
  #endif // End CheckRxErr

  #ifdef CheckCRC
		if ( eng->dat.RxDes0DW & Check_ErrMask_CRC ) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Error CRC          %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_CRC );
		}
  #endif // End CheckCRC

  #ifdef CheckFTL
		if ( eng->dat.RxDes0DW & Check_ErrMask_FTL ) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Error FTL          %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_FTL );
		}
  #endif // End CheckFTL

  #ifdef CheckRunt
		if ( eng->dat.RxDes0DW & Check_ErrMask_Runt) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Error Runt         %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_Runt );
		}
  #endif // End CheckRunt

  #ifdef CheckOddNibble
		if ( eng->dat.RxDes0DW & Check_ErrMask_OddNibble ) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Odd Nibble         %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_OddNibble );
		}
  #endif // End CheckOddNibble

  #ifdef CheckRxFIFOFull
		if ( eng->dat.RxDes0DW & Check_ErrMask_RxFIFOFull ) {
    #if defined(ENABLE_LOG_FILE)
			fprintf( eng->fp_log, "[%sRxDes] Error Rx FIFO Full %08lx:%08lx %08lx            [Des:%d][loop:%d]\n", type, adr, eng->dat.RxDes0DW, eng->dat.RxDes3DW, desnum, eng->run.Loop );
    #endif
			FindErr_Des( eng, Check_Des_RxFIFOFull );
		}
  #endif // End CheckRxFIFOFull
	}

#endif // End CheckRxOwn

	if ( eng->flg.Err_Flag )
		return(1);
	else
		return(0);
} // End char check_des_header_Rx (MAC_ENGINE *eng, char *type, ULONG adr, LONG desnum)

//------------------------------------------------------------
char check_des (MAC_ENGINE *eng, ULONG bufnum, int checkpoint) {
	int        desnum;
	int        desnum_last;
	ULONG      H_rx_desadr;
	ULONG      H_tx_desadr;
	ULONG      H_tx_bufadr;
	ULONG      H_rx_bufadr;
#ifdef Delay_DesGap
	ULONG      dly_cnt = 0;
	ULONG      dly_max = Delay_CntMaxIncVal;
#endif

#ifdef  DbgPrn_FuncHeader
	printf("check_des  : %d(%d)\n", eng->run.Loop, checkpoint);
	Debug_delay();
#endif

	// Fire the engine to send and recvice
	Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll
	Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll

#ifndef SelectSimpleDes
	H_tx_bufadr = eng->dat.DMA_Base_Tx;
	H_rx_bufadr = eng->dat.DMA_Base_Rx;
#endif
	H_rx_desadr = eng->run.RDES_BASE;
	H_tx_desadr = eng->run.TDES_BASE;

#ifdef Delay_DES
	DELAY( Delay_DES );
#endif

	for ( desnum = 0; desnum < eng->dat.Des_Num; desnum++ ) {
		desnum_last = ( desnum == ( eng->dat.Des_Num - 1 ) ) ? 1 : 0;
		if ( DbgPrn_BufAdr )
			printf("[loop:%4d][des:%4d][check_des  ] %08lx %08lx [%08lx] [%08lx]\n", eng->run.Loop, desnum, ( H_tx_desadr ), ( H_rx_desadr ), Read_Mem_Des_DD( H_tx_desadr + 12 ), Read_Mem_Des_DD( H_rx_desadr + 12 ) );

		//[Delay]--------------------
#ifdef Delay_DesGap
		if ( dly_cnt++ > 3 ) {
			switch ( rand() % 12 ) {
				case 1 : dly_max = 00000; break;
				case 3 : dly_max = 20000; break;
				case 5 : dly_max = 40000; break;
				case 7 : dly_max = 60000; break;
				defaule: dly_max = 70000; break;
			}

			dly_max += ( rand() % 4 ) * 14321;

			while (dly_cnt < dly_max) {
				dly_cnt++;
			}

			dly_cnt = 0;
		}
		else {
//			timeout = 0;
//			while (timeout < 50000) {timeout++;};
		}
#endif // End Delay_DesGap

		//[Check Owner Bit]--------------------
		eng->dat.FRAME_LEN_Cur = eng->dat.FRAME_LEN[desnum];
#if defined(ENABLE_LOG_FILE)
		if ( DbgPrn_FRAME_LEN )
			fprintf( eng->fp_log, "[check_des      ] FRAME_LEN_Cur:%08lx[Des:%d][loop:%d]%d\n", eng->dat.FRAME_LEN_Cur, desnum, eng->run.Loop, checkpoint );
#endif
		// Check the description of Tx and Rx
		if ( eng->run.TM_TxDataEn && check_des_header_Tx( eng, "", H_tx_desadr, desnum ) ) {
			eng->flg.CheckDesFail_DesNum = desnum;

			return(1);
		}
		if ( eng->run.TM_RxDataEn && check_des_header_Rx( eng, "", H_rx_desadr, desnum ) ) {
			eng->flg.CheckDesFail_DesNum = desnum;

			return(1);
		}

#ifndef SelectSimpleDes
		if ( !checkpoint ) {
			// Setting buffer address to description of Tx and Rx on next stage
			if ( eng->run.TM_RxDataEn ) {
				Write_Mem_Des_DD( H_rx_desadr + 0x0C, H_rx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_rx_desadr, RDES_IniVal );
				Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 ); //Rx Poll
				H_rx_bufadr += DMA_PakSize;
			}
			if ( eng->run.TM_TxDataEn ) {
				Write_Mem_Des_DD( H_tx_desadr + 0x0C, H_tx_bufadr );
				if ( desnum_last )
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal | EOR_IniVal );
				else
					Write_Mem_Des_DD( H_tx_desadr, TDES_IniVal );
				Write_Reg_MAC_DD( eng, 0x18, 0x00000000 ); //Tx Poll
				H_tx_bufadr += DMA_PakSize;
			}
		}
#endif // End SelectSimpleDes

		H_rx_desadr += 16;
		H_tx_desadr += 16;
	} // End for (desnum = 0; desnum < eng->dat.Des_Num; desnum++)

	return(0);
} // End char check_des (MAC_ENGINE *eng, ULONG bufnum, int checkpoint)
//#endif

//------------------------------------------------------------
// Print
//------------------------------------------------------------
void PrintMode (MAC_ENGINE *eng) {
	if (eng->env.MAC34_vld) printf("run_mode[dec]    | 0->MAC1 1->MAC2 2->MAC3 3->MAC4\n");
	else                    printf("run_mode[dec]    | 0->MAC1 1->MAC2\n");
}

//------------------------------------------------------------
void PrintSpeed (MAC_ENGINE *eng) {
#ifdef Enable_MAC_ExtLoop
	printf("speed[dec]       | 0->1G   1->100M 2->10M (default:%3d)\n", DEF_GSPEED);
#else
	printf("speed[dec]       | 0->1G   1->100M 2->10M  3->all speed (default:%3d)\n", DEF_GSPEED);
#endif
}

//------------------------------------------------------------
void PrintCtrl (MAC_ENGINE *eng) {
	printf("ctrl[dec]        | bit0~2: Reserved\n");
	printf("(default:%3d)    | bit3  : 1->Enable PHY init     0->Disable PHY init\n", DEF_GCTRL);
	printf("                 | bit4  : 1->Enable PHY int-loop 0->Disable PHY int-loop\n");
	printf("                 | bit5  : 1->Ignore PHY ID       0->Check PHY ID\n");
	if ( eng->env.AST2400 ) {
		printf("                 | bit6  : 1->Enable MAC int-loop 0->Disable MAC int-loop\n");
	}
}

//------------------------------------------------------------
void PrintLoop (MAC_ENGINE *eng) {
	printf("loop_max[dec]    | 1G  :  (default:%3d)\n", DEF_GLOOP_MAX * 20);
	printf("                 | 100M:  (default:%3d)\n", DEF_GLOOP_MAX * 2);
	printf("                 | 10M :  (default:%3d)\n", DEF_GLOOP_MAX);
}

//------------------------------------------------------------
void PrintTest (MAC_ENGINE *eng) {
	if ( eng->ModeSwitch == MODE_NSCI ) {
		printf("test_mode[dec]   | 0: NCSI configuration with    Disable_Channel request\n");
		printf("(default:%3d)    | 1: NCSI configuration without Disable_Channel request\n", DEF_GTESTMODE);
	}
	else {
		printf("test_mode[dec]   | 0: Tx/Rx frame checking\n");
		printf("(default:%3d)    | 1: Tx output 0xff frame\n", DEF_GTESTMODE);
		printf("                 | 2: Tx output 0x55 frame\n");
		printf("                 | 3: Tx output random frame\n");
		printf("                 | 4: Tx output ARP frame\n");
		printf("                 | 5: Tx output user defined value frame (default:0x%8x)\n", DEF_GUSER_DEF_PACKET_VAL);
	} // End if ( eng->ModeSwitch == MODE_NSCI )

	if ( eng->env.AST2300 ) {
		printf("                 | 6: IO timing testing\n");
		printf("                 | 7: IO timing/strength testing\n");
	}
}

//------------------------------------------------------------
void PrintPHYAdr (MAC_ENGINE *eng) {
	printf("phy_adr[dec]     | 0~31: PHY Address (default:%d)\n", DEF_GPHY_ADR);
}

//------------------------------------------------------------
void PrintIOTimingBund (MAC_ENGINE *eng) {
	if ( eng->env.AST2300 )
		printf("IO margin[dec]   | 0/1/3/5 (default:%d)\n", DEF_GIOTIMINGBUND);
}

//------------------------------------------------------------
void PrintPakNUm (MAC_ENGINE *eng) {
	printf("package_num[dec] | 1~ 8: Total Number of NCSI Package (default:%d)\n", DEF_GPACKAGE2NUM);
}

//------------------------------------------------------------
void PrintChlNUm (MAC_ENGINE *eng) {
	printf("channel_num[dec] | 1~32: Total Number of NCSI Channel (default:%d)\n", DEF_GCHANNEL2NUM);
}

//------------------------------------------------------------
void Print_Header (MAC_ENGINE *eng, BYTE option) {

	FILE_VAR
	GET_OBJ( option )

	if      ( eng->run.Speed_sel[ 0 ] ) PRINT( OUT_OBJ " 1G   " );
	else if ( eng->run.Speed_sel[ 1 ] ) PRINT( OUT_OBJ " 100M " );
	else                                PRINT( OUT_OBJ " 10M  " );

#ifdef Enable_MAC_ExtLoop
	PRINT( OUT_OBJ "Tx/Rx loop\n" );
#else
	switch ( eng->arg.GTestMode ) {
		case 0 : PRINT( OUT_OBJ "Tx/Rx frame checking       \n" ); break;
		case 1 : PRINT( OUT_OBJ "Tx output 0xff frame       \n" ); break;
		case 2 : PRINT( OUT_OBJ "Tx output 0x55 frame       \n" ); break;
		case 3 : PRINT( OUT_OBJ "Tx output random frame     \n" ); break;
		case 4 : PRINT( OUT_OBJ "Tx output ARP frame        \n" ); break;
		case 5 : PRINT( OUT_OBJ "Tx output 0x%08lx frame    \n", eng->arg.GUserDVal ); break;
		case 6 : PRINT( OUT_OBJ "IO delay testing           \n" ); break;
		case 7 : PRINT( OUT_OBJ "IO delay testing(Strength) \n" ); break;
		case 8 : PRINT( OUT_OBJ "Tx frame                   \n" ); break;
		case 9 : PRINT( OUT_OBJ "Rx frame checking          \n" ); break;
	}
#endif
}

//------------------------------------------------------------
void PrintIO_Header (MAC_ENGINE *eng, BYTE option) {

	FILE_VAR
	GET_OBJ( option )

	if ( eng->run.TM_IOStrength )
		if ( eng->io.Str_max > 1 )
			PRINT( OUT_OBJ "<IO Strength: SCU%02lx[%2d:%2d]=%2d>", eng->io.Str_reg_idx, eng->io.Str_shf + 1, eng->io.Str_shf, eng->io.Str_i );
		else
			PRINT( OUT_OBJ "<IO Strength: SCU%02lx[%2d]=%2d>", eng->io.Str_reg_idx, eng->io.Str_shf, eng->io.Str_i );

	if      ( eng->run.Speed_sel[ 0 ] ) PRINT( OUT_OBJ "[1G  ]========================================>\n" );
	else if ( eng->run.Speed_sel[ 1 ] ) PRINT( OUT_OBJ "[100M]========================================>\n" );
	else                                PRINT( OUT_OBJ "[10M ]========================================>\n" );


	if ( !(option == FP_LOG) ) {
#ifdef AST2500_IOMAP
		PRINT( OUT_OBJ "   SCU%2X      ", eng->io.Dly_reg_idx );

		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			eng->io.Dly_in_selval = eng->io.value_ary[ eng->io.Dly_in ];
			PRINT( OUT_OBJ "%1x", ( eng->io.Dly_in_selval >> 4 ) );
		}

		PRINT( OUT_OBJ "\n%s    ", eng->io.Dly_reg_name_rx_new );
		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			eng->io.Dly_in_selval = eng->io.value_ary[ eng->io.Dly_in ];
			PRINT( OUT_OBJ "%1x", eng->io.Dly_in_selval & 0xf );
		}

		PRINT( OUT_OBJ "\n              " );
		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			if ( eng->io.Dly_in_reg_idx == eng->io.Dly_in ) PRINT( OUT_OBJ "|" );
			else                                            PRINT( OUT_OBJ " " );
		}
#else
		PRINT( OUT_OBJ "%s   ", eng->io.Dly_reg_name_rx );

		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			eng->io.Dly_in_selval = eng->io.value_ary[ eng->io.Dly_in ];
			PRINT( OUT_OBJ "%2x", eng->io.Dly_in_selval & 0xf );
		}

		PRINT( OUT_OBJ "\n                   " );
		for ( eng->io.Dly_in = eng->io.Dly_in_str; eng->io.Dly_in <= eng->io.Dly_in_end; eng->io.Dly_in+=eng->io.Dly_in_cval ) {
			if ( eng->io.Dly_in_reg_idx == eng->io.Dly_in ) PRINT( OUT_OBJ " |" );
			else                                            PRINT( OUT_OBJ "  " );
		}
#endif
		PRINT( OUT_OBJ "\n");
	} // End if ( !(option == FP_LOG) )
} // End void PrintIO_Header (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_LineS (MAC_ENGINE *eng, BYTE option) {

	FILE_VAR
	GET_OBJ( option )

#ifdef AST2500_IOMAP
	if ( eng->io.Dly_out_reg_hit )
		PRINT( OUT_OBJ "%s%02x:-", eng->io.Dly_reg_name_tx_new, eng->io.Dly_out_selval );
	else
		PRINT( OUT_OBJ "%s%02x: ", eng->io.Dly_reg_name_tx_new, eng->io.Dly_out_selval );
#else
	if ( eng->io.Dly_out_reg_hit )
		PRINT( OUT_OBJ "%s%01x:-", eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
	else
		PRINT( OUT_OBJ "%s%01x: ", eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
#endif
} // End void PrintIO_LineS (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_Line (MAC_ENGINE *eng, BYTE option) {

	FILE_VAR
	GET_OBJ( option )

#ifdef AST2500_IOMAP
	if ( ( eng->io.Dly_in_reg == eng->io.Dly_in_selval ) && eng->io.Dly_out_reg_hit ) {
		if ( eng->io.Dly_result ) PRINT( OUT_OBJ "X" );
		else                      PRINT( OUT_OBJ "O" );
	}
	else {
		if ( eng->io.Dly_result ) PRINT( OUT_OBJ "x" );
		else                      PRINT( OUT_OBJ "o" );
	}
#else
	if ( ( eng->io.Dly_in_reg == eng->io.Dly_in_selval ) && eng->io.Dly_out_reg_hit ) {
		if ( eng->io.Dly_result ) PRINT( OUT_OBJ " X" );
		else                      PRINT( OUT_OBJ " O" );
	}
	else {
		if ( eng->io.Dly_result ) PRINT( OUT_OBJ " x" );
		else                      PRINT( OUT_OBJ " o" );
	}
#endif
} // End void PrintIO_Line (MAC_ENGINE *eng, BYTE option)

//------------------------------------------------------------
void PrintIO_Line_LOG (MAC_ENGINE *eng) {
#if defined(ENABLE_LOG_FILE)
	if ( eng->io.Dly_result )
		fprintf( eng->fp_log, "\n=====>[Check]%s%2x, %s%2x:  X\n", eng->io.Dly_reg_name_rx, eng->io.Dly_in_selval, eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
	else
		fprintf( eng->fp_log, "\n=====>[Check]%s%2x, %s%2x:  O\n", eng->io.Dly_reg_name_rx, eng->io.Dly_in_selval, eng->io.Dly_reg_name_tx, eng->io.Dly_out_selval );
#endif
}

//------------------------------------------------------------
// main
//------------------------------------------------------------
void Calculate_LOOP_CheckNum (MAC_ENGINE *eng) {

#define ONE_MBYTE    1048576

#ifdef CheckDataEveryTime
	eng->run.LOOP_CheckNum = 1;
#else
	if ( eng->run.IO_MrgChk || ( eng->arg.GSpeed == SET_1G_100M_10MBPS ) ) {
		eng->run.LOOP_CheckNum = eng->run.LOOP_MAX;
	}
	else {
		switch ( eng->arg.GSpeed ) {
			case SET_1GBPS    : eng->run.CheckBuf_MBSize =  MOVE_DATA_MB_SEC      ; break; // 1G
			case SET_100MBPS  : eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 3); break; // 100M ~ 1G / 8
			case SET_10MBPS   : eng->run.CheckBuf_MBSize = (MOVE_DATA_MB_SEC >> 6); break; // 10M  ~ 1G / 64
		}
		eng->run.LOOP_CheckNum = ( eng->run.CheckBuf_MBSize / ( ((eng->dat.Des_Num * DMA_PakSize) / ONE_MBYTE ) + 1) );
	}
#endif
}

//------------------------------------------------------------
void TestingSetup (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("TestingSetup\n");
	Debug_delay();
#endif

#ifdef SLT_UBOOT
#elif defined(LinuxAP)
#else
  #ifdef RAND_SIZE_SED
	srand((unsigned) RAND_SIZE_SED );
  #else
	srand((unsigned) eng->timestart );
  #endif
#endif

	//[Disable VGA]--------------------
#ifdef Disable_VGA
//	if ( eng->arg.GLOOP_INFINI & ~( eng->run.TM_Burst || eng->run.TM_IOTiming ) ) {
		eng->env.VGAModeVld = 1;
		outp(0x3d4, 0x17);
		eng->env.VGAMode = inp(0x3d5);
		outp(0x3d4, 0x17);
		outp(0x3d5, 0);
//	}
#endif

	//[Setup]--------------------
	setup_framesize( eng );
	setup_buf( eng );
}

//------------------------------------------------------------
// Return 1 ==> fail
// Return 0 ==> PASS
//------------------------------------------------------------
char TestingLoop (MAC_ENGINE *eng, ULONG loop_checknum) {
	char       checkprd;
	char       looplast;
	char       checken;
	ULONG      desadr;

#ifdef Enable_ShowBW
  #ifdef SLT_UBOOT
  #else
	clock_t    timeold;
  #endif
#endif

#ifdef  DbgPrn_FuncHeader
	printf("TestingLoop: %d\n", eng->run.Loop);
	Debug_delay();
#endif

	if ( DbgPrn_DumpMACCnt )
		dump_mac_ROreg( eng );

	//[Setup]--------------------
	eng->run.Loop = 0;
	checkprd = 0;
	checken  = 0;
	looplast = 0;


	setup_des( eng, 0 );

#ifdef Enable_ShowBW
  #ifdef SLT_UBOOT
  #else
	timeold = clock();
  #endif
#endif
	if ( eng->run.TM_WaitStart ) {
		printf("Press any key to start...\n");
		GET_CAHR();
	}

#ifdef Enable_MAC_ExtLoop
	while ( 0 ) {
#else
	while ( ( eng->run.Loop < eng->run.LOOP_MAX ) || eng->arg.GLOOP_INFINI ) {
#endif
		looplast = !eng->arg.GLOOP_INFINI && ( eng->run.Loop == eng->run.LOOP_MAX - 1 );

#ifdef CheckRxBuf
		if ( !eng->run.TM_Burst )
			checkprd = ( ( eng->run.Loop % loop_checknum ) == ( loop_checknum - 1 ) );
		checken = looplast | checkprd;
#endif



#ifdef  DbgPrn_FuncHeader
		printf("for start ======> %d/%d(%d) looplast:%d checkprd:%d checken:%d\n", eng->run.Loop, eng->run.LOOP_MAX, eng->arg.GLOOP_INFINI, looplast, checkprd, checken);
		Debug_delay();
#else
		if ( DbgPrn_BufAdr ) {
			printf("for start ======> %d/%d(%d) looplast:%d checkprd:%d checken:%d\n", eng->run.Loop, eng->run.LOOP_MAX, eng->arg.GLOOP_INFINI, looplast, checkprd, checken);
			Debug_delay();
		}
#endif

		if ( eng->run.TM_RxDataEn )
			eng->dat.DMA_Base_Tx = eng->dat.DMA_Base_Rx;
		eng->dat.DMA_Base_Rx = CPU_BUS_ADDR_SDRAM_OFFSET + ZeroCopy_OFFSET + GET_DMA_BASE( eng->run.Loop + 1 ); // 20130730
		//[Check DES]--------------------
		if ( check_des( eng, eng->run.Loop, checken ) ) {
			//descriptor error
#ifdef CheckRxBuf
			eng->dat.Des_Num = eng->flg.CheckDesFail_DesNum + 1;
			if ( checkprd )
				check_buf( eng, loop_checknum );
			else
				check_buf( eng, ( eng->run.LOOP_MAX % loop_checknum ) );
			eng->dat.Des_Num = eng->dat.Des_Num_Org;
#endif

			if (DbgPrn_DumpMACCnt)
				dump_mac_ROreg ( eng );

			return(1);
		}

		//[Check Buf]--------------------
		if ( eng->run.TM_RxDataEn && checken ) {
#ifdef Enable_ShowBW
  #ifdef SLT_UBOOT
  #elif defined(LinuxAP)
  #else
			eng->timeused = ( clock() - timeold ) / (double) CLK_TCK;
  #endif
#endif

			if ( checkprd ) {
#ifdef Enable_ShowBW
  #if defined(LinuxAP)
  #else
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				fprintf( eng->fp_log, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", loop_checknum, ((double)loop_checknum * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );
  #endif
#endif

#ifdef CheckRxBuf
				if ( check_buf( eng, loop_checknum ) )
					return(1);
#endif
			}
			else {
#ifdef Enable_ShowBW
  #if defined(LinuxAP)
  #else
				printf("[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.LOOP_MAX % loop_checknum), ((double)(eng->run.LOOP_MAX % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused);
				fprintf( eng->fp_log, "[run loop:%3d] BandWidth: %7.2f Mbps, %6.2f sec\n", (eng->run.LOOP_MAX % loop_checknum), ((double)(eng->run.LOOP_MAX % loop_checknum) * (double)eng->dat.Total_frame_len * 8.0) / ((double)eng->timeused * 1000000.0), eng->timeused );
  #endif
#endif

#ifdef CheckRxBuf
				if ( check_buf( eng, ( eng->run.LOOP_MAX % loop_checknum ) ) )
					return(1);
#endif
			} // End if ( checkprd )

#ifndef SelectSimpleDes
			if ( !looplast )
				setup_des_loop( eng, eng->run.Loop );
#endif

#ifdef Enable_ShowBW
  #if defined(LinuxAP)
  #else
			timeold = clock();
  #endif
#endif
		} // End if ( eng->run.TM_RxDataEn && checken )

#ifdef SelectSimpleDes
		if ( !looplast )
			setup_des_loop( eng, eng->run.Loop );
#endif

		if ( eng->arg.GLOOP_INFINI )
				printf("===============> Loop: %d  \r", eng->run.Loop);
		else if ( eng->arg.GTestMode == 0 ) {
			if ( !( DbgPrn_BufAdr || eng->run.IO_Bund ) )
				printf(" %d                        \r", eng->run.Loop);
//			switch ( eng->run.Loop % 4 ) {
//				case 0x00: printf("| %d                        \r", eng->run.Loop); break;
//				case 0x01: printf("/ %d                        \r", eng->run.Loop); break;
//				case 0x02: printf("- %d                        \r", eng->run.Loop); break;
//				default  : printf("\ %d                        \r", eng->run.Loop); break;
//			}
		}

#ifdef  DbgPrn_FuncHeader
		printf("for end   ======> %d/%d(%d)\n", eng->run.Loop, eng->run.LOOP_MAX, eng->arg.GLOOP_INFINI);
		Debug_delay();
#else
		if ( DbgPrn_BufAdr ) {
			printf("for end   ======> %d/%d(%d)\n", eng->run.Loop, eng->run.LOOP_MAX, eng->arg.GLOOP_INFINI);
			Debug_delay();
		}
#endif

		if ( eng->run.Loop >= Loop_OverFlow ) {
			printf("Over-flow\n");
			eng->run.Loop = 0;
		}
		else
			eng->run.Loop++;
	} // End while ((eng->run.Loop < eng->run.LOOP_MAX) || eng->arg.GLOOP_INFINI)
	eng->run.Loop_rl[ eng->run.Speed_idx ] = eng->run.Loop;


#ifdef Enable_MAC_ExtLoop
  #ifdef Enable_MAC_ExtLoop_PakcegMode
	desadr = eng->run.RDES_BASE + ( eng->dat.Des_Num - 1 ) * 16;
	Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
	while ( 1 ) {
		while ( !HWOwnTx( Read_Mem_Des_DD( desadr ) ) ) {
		}
		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll

		while ( !HWOwnRx( Read_Mem_Des_DD( eng->run.RDES_BASE ) ) ) {
		}
		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
		switch ( eng->run.Loop % 4 ) {
			case 0x00: printf("| %d                        \r", eng->run.Loop); break;
			case 0x01: printf("/ %d                        \r", eng->run.Loop); break;
			case 0x02: printf("- %d                        \r", eng->run.Loop); break;
			default  : printf("\ %d                        \r", eng->run.Loop); break;
		}
//		printf("===============> Loop: %d  \r", eng->run.Loop);
		eng->run.Loop++;
		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
	}
  #else
	while ( !kbhit() ) {
		Write_Reg_MAC_DD( eng, 0x1c, 0x00000000 );//Rx Poll
		Write_Reg_MAC_DD( eng, 0x18, 0x00000000 );//Tx Poll

		printf("Tx:%08lx, Rx:%08lx %08lx\r", Read_Reg_MAC_DD( eng, 0xa0 ), Read_Reg_MAC_DD( eng, 0xb0 ), Read_Reg_MAC_DD( eng, 0xb4 ));
	}
  #endif
#endif

	return(0);
} // End char TestingLoop (MAC_ENGINE *eng, ULONG loop_checknum)
