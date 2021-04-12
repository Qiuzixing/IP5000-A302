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

#define PHY_C
static const char ThisFile[] = "PHY.c";

#include "SWFUNC.H"
#include "COMMINF.H"

#if defined(SLT_UBOOT)
  #include <common.h>
  #include <command.h>
  #include "STDUBOOT.H"
#endif
#if defined(DOS_ALONE)
  #include <stdio.h>
  #include <stdlib.h>
  #include <conio.h>
  #include <string.h>
#endif

#include "PHY.H"
#include "TYPEDEF.H"
#include "IO.H"

//------------------------------------------------------------
// PHY R/W basic
//------------------------------------------------------------
void phy_write (MAC_ENGINE *eng, int adr, ULONG data) {
	int        timeout = 0;

	if ( eng->inf.NewMDIO ) {
		Write_Reg_MAC_DD( eng, 0x60, ( data << 16 ) | MAC_PHYWr_New | (eng->phy.Adr<<5) | (adr & 0x1f) );

		while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
#if defined(ENABLE_LOG_FILE)
					fprintf( eng->fp_log, "[PHY-Write] Time out: %08lx\n", Read_Reg_MAC_DD( eng, 0x60 ) );
#endif
				FindErr( eng, Err_PHY_TimeOut_RW );
				break;
			}
		}
	}
	else {
		Write_Reg_MAC_DD( eng, 0x64, data );

		Write_Reg_MAC_DD( eng, 0x60, MDC_Thres | MAC_PHYWr | (eng->phy.Adr<<16) | ((adr & 0x1f) << 21) );

		while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYWr ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
#if defined(ENABLE_LOG_FILE)
				if ( !eng->run.TM_Burst )
					fprintf( eng->fp_log, "[PHY-Write] Time out: %08lx\n", Read_Reg_MAC_DD( eng, 0x60 ) );
#endif
				FindErr( eng, Err_PHY_TimeOut_RW );
				break;
			}
		}
	} // End if ( eng->inf.NewMDIO )

	if (DbgPrn_PHYRW)
		printf("[Wr ]%02d: %04lx\n", adr, data);

} // End void phy_write (int adr, ULONG data)

//------------------------------------------------------------
ULONG phy_read (MAC_ENGINE *eng, int adr) {
	int        timeout = 0;
	ULONG      read_value;

	if ( eng->inf.NewMDIO ) {
		Write_Reg_MAC_DD( eng, 0x60, MAC_PHYRd_New | (eng->phy.Adr << 5) | ( adr & 0x1f ) );

		while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYBusy_New ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
				if ( !eng->run.TM_Burst )
#if defined(ENABLE_LOG_FILE)
					fprintf( eng->fp_log, "[PHY-Read] Time out: %08lx\n", Read_Reg_MAC_DD( eng, 0x60 ) );
#endif
				FindErr( eng, Err_PHY_TimeOut_RW );
				break;
			}
		}

		DELAY( Delay_PHYRd );
		read_value = Read_Reg_MAC_DD( eng, 0x64 ) & 0xffff;
	}
	else {
		Write_Reg_MAC_DD( eng, 0x60, MDC_Thres | MAC_PHYRd | (eng->phy.Adr << 16) | ((adr & 0x1f) << 21) );

		while ( Read_Reg_MAC_DD( eng, 0x60 ) & MAC_PHYRd ) {
			if ( ++timeout > TIME_OUT_PHY_RW ) {
#if defined(ENABLE_LOG_FILE)
				if ( !eng->run.TM_Burst )
					fprintf( eng->fp_log, "[PHY-Read] Time out: %08lx\n", Read_Reg_MAC_DD( eng, 0x60 ) );
#endif
				FindErr( eng, Err_PHY_TimeOut_RW );
				break;
			}
		}

		DELAY( Delay_PHYRd );
		read_value = Read_Reg_MAC_DD( eng, 0x64 ) >> 16;
	}

	if ( DbgPrn_PHYRW )
		printf("[Rd ]%02d: %04lx\n", adr, read_value );

	return( read_value );
} // End ULONG phy_read (MAC_ENGINE *eng, int adr)

//------------------------------------------------------------
void phy_Read_Write (MAC_ENGINE *eng, int adr, ULONG clr_mask, ULONG set_mask) {
	if ( DbgPrn_PHYRW )
		printf("[RW ]%02d: clr:%04lx: set:%04lx\n", adr, clr_mask, set_mask);
	phy_write( eng, adr, ((phy_read( eng, adr ) & (~clr_mask)) | set_mask) );
}

//------------------------------------------------------------
void phy_out (MAC_ENGINE *eng, int adr) {
	printf("%02d: %04lx\n", adr, phy_read( eng, adr ));
}

//------------------------------------------------------------
//void phy_outchg (MAC_ENGINE *eng,  int adr) {
//	ULONG	PHY_valold = 0;
//	ULONG	PHY_val;
//
//	while (1) {
//		PHY_val = phy_read( eng, adr );
//		if (PHY_valold != PHY_val) {
//			printf("%02d: %04lx\n", adr, PHY_val);
//			PHY_valold = PHY_val;
//		}
//	}
//}

//------------------------------------------------------------
void phy_dump (MAC_ENGINE *eng, char *name) {
	int        index;

	printf("[%s][%d]----------------\n", name, eng->phy.Adr);
	for (index = 0; index < 32; index++) {
		printf("%02d: %04lx ", index, phy_read( eng, index ));

		if ((index % 8) == 7)
			printf("\n");
	}
}

//------------------------------------------------------------
void phy_id (MAC_ENGINE *eng, BYTE option) {

	ULONG      reg_adr;
	CHAR       PHY_ADR_org;

	FILE_VAR
	GET_OBJ( option )

	PHY_ADR_org = eng->phy.Adr;
	for ( eng->phy.Adr = 0; eng->phy.Adr < 32; eng->phy.Adr++ ) {

		PRINT(OUT_OBJ "[%02d] ", eng->phy.Adr);

		for ( reg_adr = 2; reg_adr <= 3; reg_adr++ )
			PRINT(OUT_OBJ "%ld:%04lx ", reg_adr, phy_read( eng, reg_adr ));

		if ( ( eng->phy.Adr % 4 ) == 3 )
			PRINT(OUT_OBJ "\n");
	}
	eng->phy.Adr = PHY_ADR_org;
}


//------------------------------------------------------------
void phy_delay (int dt) {
	DELAY( dt );
}

//------------------------------------------------------------
// PHY IC
//------------------------------------------------------------
void phy_basic_setting (MAC_ENGINE *eng) {
	phy_Read_Write( eng,  0, 0x7140, eng->phy.PHY_00h ); //clr set
	if ( DbgPrn_PHYRW )
		printf("[Set]00: %04lx\n", phy_read( eng, PHY_REG_BMCR ));
}

//------------------------------------------------------------
void phy_Wait_Reset_Done (MAC_ENGINE *eng) {
	int        timeout = 0;

	while (  phy_read( eng, PHY_REG_BMCR ) & 0x8000 ) {
		if (++timeout > TIME_OUT_PHY_Rst) {
#if defined(ENABLE_LOG_FILE)
			if ( !eng->run.TM_Burst ) fprintf( eng->fp_log, "[PHY-Reset] Time out: %08lx\n", Read_Reg_MAC_DD( eng, 0x60 ) );
#endif
			FindErr( eng, Err_PHY_TimeOut_Rst );
			break;
		}
	}//wait Rst Done

	if (DbgPrn_PHYRW) printf("[Clr]00: %04lx\n", phy_read( eng, PHY_REG_BMCR ));
	DELAY( Delay_PHYRst );
}

//------------------------------------------------------------
void phy_Reset (MAC_ENGINE *eng) {
	phy_basic_setting( eng );

	phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
	phy_Wait_Reset_Done( eng );
    
	phy_basic_setting( eng );
	DELAY( Delay_PHYRst );
}

//------------------------------------------------------------
void recov_phy_marvell (MAC_ENGINE *eng) {//88E1111
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng,  9, eng->phy.PHY_09h );

			phy_Reset( eng );

			phy_write( eng, 29, 0x0007 );
			phy_Read_Write( eng, 30, 0x0008, 0x0000 );//clr set
			phy_write( eng, 29, 0x0010 );
			phy_Read_Write( eng, 30, 0x0002, 0x0000 );//clr set
			phy_write( eng, 29, 0x0012 );
			phy_Read_Write( eng, 30, 0x0001, 0x0000 );//clr set

			phy_write( eng, 18, eng->phy.PHY_12h );
		}
	}
}

//------------------------------------------------------------
void phy_marvell (MAC_ENGINE *eng) {//88E1111
	int        Retry;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			eng->phy.PHY_09h = phy_read( eng, PHY_GBCR );
			eng->phy.PHY_12h = phy_read( eng, PHY_INER );
			phy_write( eng, 18, 0x0000 );
			phy_Read_Write( eng,  9, 0x0000, 0x1800 );//clr set
		}

		phy_Reset( eng );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 29, 0x0007 );
			phy_Read_Write( eng, 30, 0x0000, 0x0008 );//clr set
			phy_write( eng, 29, 0x0010 );
			phy_Read_Write( eng, 30, 0x0000, 0x0002 );//clr set
			phy_write( eng, 29, 0x0012 );
			phy_Read_Write( eng, 30, 0x0000, 0x0001 );//clr set
		}
	}

	Retry = 0;
	do {
		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void recov_phy_marvell0 (MAC_ENGINE *eng) {//88E1310
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 16, 0x0020, 0x0000 );//clr set
			phy_write( eng, 22, 0x0000 );
		}
	}
}

//------------------------------------------------------------
void phy_marvell0 (MAC_ENGINE *eng) {//88E1310
	int        Retry;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_write( eng, 22, 0x0002 );

	eng->phy.PHY_15h = phy_read( eng, 21 );
	if ( eng->phy.PHY_15h & 0x0030 ) {
		printf("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h);
#if defined(ENABLE_LOG_FILE)
	if ( eng->run.TM_IOTiming )
		fprintf( eng->fp_io, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h );
	if ( !eng->run.TM_Burst )
		fprintf( eng->fp_log, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15_2:%04lx]\n\n", eng->phy.PHY_15h );
#endif
//		phy_Read_Write( eng, 21, 0x0030, 0x0000 );//clr set//[5]Rx Dly, [4]Tx Dly
	phy_write( eng, 21, eng->phy.PHY_15h & 0xffcf ); // Set [5]Rx Dly, [4]Tx Dly to 0
	}
	phy_write( eng, 22, 0x0000 );

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng, 22, 0x0002 );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Read_Write( eng, 21, 0x6040, 0x0040 );//clr set
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			phy_Read_Write( eng, 21, 0x6040, 0x2000 );//clr set
		}
		else {
			phy_Read_Write( eng, 21, 0x6040, 0x0000 );//clr set
		}
		phy_write( eng, 22, 0x0000 );
		phy_Reset(  eng  );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 16, 0x0000, 0x0020 );//clr set
			phy_write( eng, 22, 0x0000 );
		}

		phy_Reset( eng );
	}

	Retry = 0;
	do {
		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void recov_phy_marvell1 (MAC_ENGINE *eng) {//88E6176
	CHAR       PHY_ADR_org;

	PHY_ADR_org = eng->phy.Adr;
	for ( eng->phy.Adr = 16; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
		if ( eng->run.TM_Burst ) {
		}
		else {
			phy_write( eng,  6, eng->phy.PHY_06hA[eng->phy.Adr-16] );//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	for ( eng->phy.Adr = 21; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
		phy_write( eng,  1, 0x0003 ); //01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
	}
	eng->phy.Adr = PHY_ADR_org;
}

//------------------------------------------------------------
void phy_marvell1 (MAC_ENGINE *eng) {//88E6176
//	ULONG      PHY_01h;
	CHAR       PHY_ADR_org;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	//The 88E6176 is switch with 7 Port(P0~P6) and the PHYAdr will be fixed at 0x10~0x16, and only P5/P6 can be connected to the MAC.
	//Therefor, the 88E6176 only can run the internal loopback.
	PHY_ADR_org = eng->phy.Adr;
	for ( eng->phy.Adr = 16; eng->phy.Adr <= 20; eng->phy.Adr++ ) {
		if ( eng->run.TM_Burst ) {
		}
		else {
			eng->phy.PHY_06hA[eng->phy.Adr-16] = phy_read( eng, PHY_ANER );
			phy_write( eng,  6, 0x0000 );//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}

	for ( eng->phy.Adr = 21; eng->phy.Adr <= 22; eng->phy.Adr++ ) {
//		PHY_01h = phy_read( eng, PHY_REG_BMSR );
//		if      ( eng->run.Speed_sel[ 0 ] ) phy_write( eng,  1, (PHY_01h & 0xfffc) | 0x0002 );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//		else if ( eng->run.Speed_sel[ 1 ] ) phy_write( eng,  1, (PHY_01h & 0xfffc) | 0x0001 );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
//		else                              phy_write( eng,  1, (PHY_01h & 0xfffc)          );//[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		if      ( eng->run.Speed_sel[ 0 ] ) phy_write( eng,  1, 0x0002 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		else if ( eng->run.Speed_sel[ 1 ] ) phy_write( eng,  1, 0x0001 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.
		else                              phy_write( eng,  1, 0x0000 );//01h[1:0]00 = 10 Mbps, 01 = 100 Mbps, 10 = 1000 Mbps, 11 = Speed is not forced.

		if ( eng->run.TM_Burst ) {
		}
		else {
			eng->phy.PHY_06hA[eng->phy.Adr-16] = phy_read( eng, PHY_ANER );
			if (eng->phy.Adr == 21) phy_write( eng,  6, 0x0020 );//06h[5]P5 loopback, 06h[6]P6 loopback
			else                    phy_write( eng,  6, 0x0040 );//06h[5]P5 loopback, 06h[6]P6 loopback
		}
	}
	eng->phy.Adr = PHY_ADR_org;
}

//------------------------------------------------------------
void recov_phy_marvell2 (MAC_ENGINE *eng) {//88E1512
	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 18, 0x0008, 0x0000 );//clr set
			phy_write( eng, 22, 0x0000 );
		}
	}
}

//------------------------------------------------------------
void phy_marvell2 (MAC_ENGINE *eng) {//88E1512
	int        Retry = 0;
	ULONG      temp_reg;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Marvell] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_write( eng, 22, 0x0002 );
	eng->phy.PHY_15h = phy_read( eng, 21 );

	if ( eng->phy.PHY_15h & 0x0030 ) {
		printf("\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h);
#if defined(ENABLE_LOG_FILE)
		if ( eng->run.TM_IOTiming ) fprintf( eng->fp_io, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h );
		if ( !eng->run.TM_Burst ) fprintf( eng->fp_log, "\n\n[Warning] Page2, Register 21, bit 4~5 must be 0 [Reg15h_2:%04lx]\n\n", eng->phy.PHY_15h );
#endif
//		phy_Read_Write( eng, 21, 0x0030, 0x0000 );//clr set//[5]Rx Dly, [4]Tx Dly
//		phy_write( eng, 21, eng->phy.PHY_15h & 0xffcf );
	}
	phy_write( eng, 22, 0x0000 );

	if ( eng->run.TM_Burst ) {
		phy_Reset( eng );
	}
	else if ( eng->phy.loop_phy ) {
		// Internal loopback funciton only support in copper mode
		// switch page 18
		phy_write( eng, 22, 0x0012 );
		// Change mode to Copper mode
		eng->phy.PHY_14h = phy_read( eng, 20 );
		if ( eng->phy.PHY_14h & 0x0020 ) {
			printf("\n\n[Warning] Internal loopback funciton only support in copper mode\n\n");
#if defined(ENABLE_LOG_FILE)
			if ( eng->run.TM_IOTiming )
				fprintf( eng->fp_io, "\n\n[Warning] Internal loopback funciton only support in copper mode\n\n" );
			if ( !eng->run.TM_Burst )
				fprintf( eng->fp_log, "\n\n[Warning] Internal loopback funciton only support in copper mode\n\n" );
#endif
		}
		phy_write( eng, 20, ( eng->phy.PHY_14h & 0xffc0 ) | 0x8010 );
		// do software reset
		do {
			temp_reg = phy_read( eng, 20 );
		} while ( ( (temp_reg & 0x8000) == 0x8000 ) & (Retry++ < 20) );

		// switch page 2
		phy_write( eng, 22, 0x0002 );
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Read_Write( eng, 21, 0x2040, 0x0040 );//clr set
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			phy_Read_Write( eng, 21, 0x2040, 0x2000 );//clr set
		}
		else {
			phy_Read_Write( eng, 21, 0x2040, 0x0000 );//clr set
		}
		phy_write( eng, 22, 0x0000 );

		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 22, 0x0006 );
			phy_Read_Write( eng, 18, 0x0000, 0x0008 );//clr set
			phy_write( eng, 22, 0x0000 );
		}

		phy_Reset( eng );
	}

	Retry = 0;
	do {
		eng->phy.PHY_11h = phy_read( eng, PHY_SR );
	} while ( !( ( eng->phy.PHY_11h & 0x0400 ) | eng->phy.loop_phy | ( Retry++ > 20 ) ) );
}

//------------------------------------------------------------
void phy_broadcom (MAC_ENGINE *eng) {//BCM5221
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Broadcom] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

	if ( eng->run.TM_IEEE ) {
		if ( eng->arg.GIEEE_sel == 0 ) {
			phy_write( eng, 25, 0x1f01 );//Force MDI  //Measuring from channel A
		}
		else {
			phy_Read_Write( eng, 24, 0x0000, 0x4000 );//clr set//Force Link
//			phy_write( eng,  0, eng->phy.PHY_00h );
//			phy_write( eng, 30, 0x1000 );
		}
	}
}

//------------------------------------------------------------
void recov_phy_broadcom0 (MAC_ENGINE *eng) {//BCM54612
	phy_write( eng,  0, eng->phy.PHY_00h );
	phy_write( eng,  9, eng->phy.PHY_09h );
//	phy_write( eng, 24, eng->phy.PHY_18h | 0xf007 );//write reg 18h, shadow value 111
//	phy_write( eng, 28, eng->phy.PHY_1ch | 0x8c00 );//write reg 1Ch, shadow value 00011

	if ( eng->run.TM_Burst ) {
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng,  0, eng->phy.PHY_00h );
	}
	else {
	}
}

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: Don't support(?)
//internal loop 10M : Don't support(?)
void phy_broadcom0 (MAC_ENGINE *eng) {//BCM54612
	ULONG      PHY_new;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Broadcom] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	eng->phy.PHY_00h = phy_read( eng, PHY_REG_BMCR );
	eng->phy.PHY_09h = phy_read( eng, PHY_GBCR );
	phy_write( eng, 24, 0x7007 );//read reg 18h, shadow value 111
	eng->phy.PHY_18h = phy_read( eng, 24 );
	phy_write( eng, 28, 0x0c00 );//read reg 1Ch, shadow value 00011
	eng->phy.PHY_1ch = phy_read( eng, 28 );

	if ( eng->phy.PHY_18h & 0x0100 ) {
		PHY_new = ( eng->phy.PHY_18h & 0x0af0 ) | 0xf007;
		printf("\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new);
#if defined(ENABLE_LOG_FILE)
		if ( eng->run.TM_IOTiming ) fprintf( eng->fp_io, "\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new );
		if ( !eng->run.TM_Burst ) fprintf( eng->fp_log, "\n\n[Warning] Shadow value 111, Register 24, bit 8 must be 0 [Reg18h_7:%04lx->%04lx]\n\n", eng->phy.PHY_18h, PHY_new );
#endif
		phy_write( eng, 24, PHY_new ); // Disable RGMII RXD to RXC Skew
	}
	if ( eng->phy.PHY_1ch & 0x0200 ) {
		PHY_new = ( eng->phy.PHY_1ch & 0x0000 ) | 0x8c00;
		printf("\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new);
#if defined(ENABLE_LOG_FILE)
		if ( eng->run.TM_IOTiming ) fprintf( eng->fp_io, "\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new );
		if ( !eng->run.TM_Burst ) fprintf( eng->fp_log, "\n\n[Warning] Shadow value 00011, Register 28, bit 9 must be 0 [Reg1ch_3:%04lx->%04lx]\n\n", eng->phy.PHY_1ch, PHY_new );
#endif
		phy_write( eng, 28, PHY_new );// Disable GTXCLK Clock Delay Enable
	}

	if ( eng->run.TM_Burst ) {
		phy_basic_setting( eng );
	}
	else if ( eng->phy.loop_phy ) {
		phy_basic_setting( eng );

		// Enable Internal Loopback mode
		// Page 58, BCM54612EB1KMLG_Spec.pdf
		phy_write( eng,  0, 0x5140 );
		DELAY( Delay_PHYRst );
		/* Only 1G Test is PASS, 100M and 10M is false @20130619 */

// Waiting for BCM FAE's response
//		if ( eng->run.Speed_sel[ 0 ] ) {
//			// Speed 1G
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x5140 );
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {
//			// Speed 100M
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x7100 );
//			phy_write( eng, 30, 0x1000 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {
//			// Speed 10M
//			// Enable Internal Loopback mode
//			// Page 58, BCM54612EB1KMLG_Spec.pdf
//			phy_write( eng,  0, 0x5100 );
//			phy_write( eng, 30, 0x1000 );
//		}
//
//		DELAY( Delay_PHYRst );
	}
	else {

		if ( eng->run.Speed_sel[ 0 ] ) {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  9, 0x1800 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
		else if ( eng->run.Speed_sel[ 1 ] ) {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  0, 0x2100 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
		else {
			// Page 60, BCM54612EB1KMLG_Spec.pdf
			// need to insert loopback plug
			phy_write( eng,  0, 0x0100 );
			phy_write( eng, 24, 0x8400 ); // Enable Transmit test mode
		}
	}
}

//------------------------------------------------------------
void phy_realtek (MAC_ENGINE *eng) {//RTL8201N
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
//internal loop 100M: Don't support
//internal loop 10M : no  loopback stub
void phy_realtek0 (MAC_ENGINE *eng) {//RTL8201E
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

//	phy_Read_Write( eng, 25, 0x2800, 0x0000 );//clr set
//	printf("Enable phy output RMII clock\n");
	if ( eng->run.TM_IEEE ) {
		phy_write( eng, 31, 0x0001 );
		if ( eng->arg.GIEEE_sel == 0 ) {
			phy_write( eng, 25, 0x1f01 );//Force MDI  //Measuring from channel A
		}
		else {
			phy_write( eng, 25, 0x1f00 );//Force MDIX //Measuring from channel B
		}
		phy_write( eng, 31, 0x0000 );
	}
}

//------------------------------------------------------------
void recov_phy_realtek1 (MAC_ENGINE *eng) {//RTL8211D
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {
					//Test Mode 1
					phy_write( eng, 31, 0x0002 );
					phy_write( eng,  2, 0xc203 );
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x0000 );
				}
				else {
					//Test Mode 4
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x0000 );
				}

				phy_write( eng, 31, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 23, 0x2100 );
				phy_write( eng, 16, 0x016e );
			}
			else {
//				phy_write( eng, 31, 0x0006 );
//				phy_write( eng,  0, 0x5a00 );
//				phy_write( eng, 31, 0x0000 );
			}
		} // End if ( eng->run.TM_IEEE )
	}
	else if ( eng->phy.loop_phy ) {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0000 ); // new in Rev. 1.6
			phy_write( eng,  0, 0x1140 ); // new in Rev. 1.6
			phy_write( eng, 20, 0x8040 ); // new in Rev. 1.6
		}
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  3, 0xdf41 );
			phy_write( eng,  2, 0xdf20 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 26, 0x0020 ); // Rev. 1.2
			phy_write( eng, 26, 0x0040 ); // new in Rev. 1.6
			phy_write( eng,  0, 0x1140 );
//			phy_write( eng, 21, 0x0006 ); // Rev. 1.2
			phy_write( eng, 21, 0x1006 ); // new in Rev. 1.6
			phy_write( eng, 23, 0x2100 );
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  0, 0x1200 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
		}
		phy_Reset( eng );
		phy_delay(2000);
	} // End if ( eng->run.TM_Burst )
} // End void recov_phy_realtek1 (MAC_ENGINE *eng)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek1 (MAC_ENGINE *eng) {//RTL8211D
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {
					//Test Mode 1
					phy_write( eng, 31, 0x0002 );
					phy_write( eng,  2, 0xc22b );
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x2000 );
				}
				else {
					//Test Mode 4
					phy_write( eng, 31, 0x0000 );
					phy_write( eng,  9, 0x8000 );
				}
				phy_write( eng, 31, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {
					//From Channel A
					phy_write( eng, 23, 0xa102 );
					phy_write( eng, 16, 0x01ae );//MDI
				}
				else {
					//From Channel B
					phy_Read_Write( eng, 17, 0x0008, 0x0000 ); // clr set
					phy_write( eng, 23, 0xa102 );         // MDI
					phy_write( eng, 16, 0x010e );
				}
			}
			else {
//				if ( eng->arg.GIEEE_sel == 0 ) {//Pseudo-random pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  0, 0x5a21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else if ( eng->arg.GIEEE_sel == 1 ) {//��FF�� pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0xff21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else {//��00�� pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0x0021 );
//					phy_write( eng, 31, 0x0000 );
//				}
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 20, 0x0042 );//new in Rev. 1.6
		}
	}
	else {
        // refer to RTL8211D Register for Manufacture Test_V1.6.pdf
        // MDI loop back
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xd720 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0040 );
			phy_write( eng, 24, 0x0008 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  9, 0x0300 );
			phy_write( eng, 26, 0x0020 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 23, 0xa101 );
			phy_write( eng, 21, 0x0200 );
			phy_write( eng, 23, 0xa121 );
			phy_write( eng, 23, 0xa161 );
			phy_write( eng,  0, 0x8000 );
			phy_Wait_Reset_Done( eng );

//			phy_delay(200); // new in Rev. 1.6
			phy_delay(5000); // 20150504
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x0061 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
		}
		else {
			phy_Reset( eng );
		}
	}
} // End void phy_realtek1 (MAC_ENGINE *eng)

//------------------------------------------------------------
void recov_phy_realtek2 (MAC_ENGINE *eng) {//RTL8211E
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_write( eng, 31, 0x0005 );
			phy_write( eng,  5, 0x8b86 );
			phy_write( eng,  6, 0xe201 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0020 );
			phy_write( eng, 21, 0x1108 );
			phy_write( eng, 31, 0x0000 );

			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 31, 0x0007 );
				phy_write( eng, 30, 0x002f );
				phy_write( eng, 23, 0xd88f );
				phy_write( eng, 30, 0x002d );
				phy_write( eng, 24, 0xf050 );
				phy_write( eng, 31, 0x0000 );
				phy_write( eng, 16, 0x006e );
			}
			else {
			}
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//Rev 1.5  //not stable
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x8000 );
//			phy_Wait_Reset_Done( eng );
//			phy_delay(30);
//			phy_write( eng, 23, 0x2160 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0004 );
//			phy_write( eng, 24, 0x1a24 );
//			phy_write( eng, 25, 0xfd00 );
//			phy_write( eng, 24, 0x0000 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x1140 );
//			phy_write( eng, 26, 0x0040 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd88f );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0300 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x1006 );
//			phy_write( eng, 23, 0x2100 );
/**/
			//Rev 1.6
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x8000 );
			phy_Wait_Reset_Done( eng );
			phy_delay(30);
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0042 );
			phy_write( eng, 21, 0x0500 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x1140 );
			phy_write( eng, 26, 0x0040 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x002f );
			phy_write( eng, 23, 0xd88f );
			phy_write( eng, 30, 0x0023 );
			phy_write( eng, 22, 0x0300 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 21, 0x1006 );
			phy_write( eng, 23, 0x2100 );
/**/
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  0, 0x1200 );
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0200 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
		}
	}
} // End void recov_phy_realtek2 (MAC_ENGINE *eng)

//------------------------------------------------------------
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek2 (MAC_ENGINE *eng) {//RTL8211E

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h ); // clr set // Rst PHY
	phy_Wait_Reset_Done( eng );
	phy_delay(30);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_write( eng, 31, 0x0005 );
			phy_write( eng,  5, 0x8b86 );
			phy_write( eng,  6, 0xe200 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0020 );
			phy_write( eng, 21, 0x0108 );
			phy_write( eng, 31, 0x0000 );

			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng, 31, 0x0000 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng,  9, 0x2000 );//Test Mode 1
				}
				else {
					phy_write( eng,  9, 0x8000 );//Test Mode 4
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 31, 0x0007 );
				phy_write( eng, 30, 0x002f );
				phy_write( eng, 23, 0xd818 );
				phy_write( eng, 30, 0x002d );
				phy_write( eng, 24, 0xf060 );
				phy_write( eng, 31, 0x0000 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng, 16, 0x00ae );//From Channel A
				}
				else {
					phy_write( eng, 16, 0x008e );//From Channel B
				}
			}
			else {
			}
		}
		else {
			phy_basic_setting( eng );
			phy_delay(30);
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_basic_setting( eng );

		phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done( eng );
		phy_delay(30);

		phy_basic_setting( eng );
		phy_delay(30);
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//Rev 1.5  //not stable
//			phy_write( eng, 23, 0x2160 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0004 );
//			phy_write( eng, 24, 0x1a24 );
//			phy_write( eng, 25, 0x7d00 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 23, 0x2100 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0040 );
//			phy_write( eng, 24, 0x0000 );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0006 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x0140 );
//			phy_write( eng, 26, 0x0060 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd820 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x0206 );
//			phy_write( eng, 23, 0x2120 );
//			phy_write( eng, 23, 0x2160 );
/**/
			//Rev 1.6
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x0042 );
			phy_write( eng, 21, 0x2500 );
			phy_write( eng, 30, 0x0023 );
			phy_write( eng, 22, 0x0006 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 26, 0x0060 );
			phy_write( eng, 31, 0x0007 );
			phy_write( eng, 30, 0x002f );
			phy_write( eng, 23, 0xd820 );
			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 21, 0x0206 );
			phy_write( eng, 23, 0x2120 );
			phy_write( eng, 23, 0x2160 );
			phy_delay(300);
/**/
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x0061 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  9, 0x0000 );
//			phy_write( eng,  4, 0x05e1 );
//			phy_write( eng,  0, 0x1200 );
//			phy_delay(5000);
		}
		else {
			phy_basic_setting( eng );
			phy_delay(150);
		}
	}
} // End void phy_realtek2 (MAC_ENGINE *eng)

//------------------------------------------------------------
void recov_phy_realtek3 (MAC_ENGINE *eng) {//RTL8211C
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 17, eng->phy.PHY_11h );
				phy_write( eng, 14, 0x0000 );
				phy_write( eng, 16, 0x00a0 );
			}
			else {
//				phy_write( eng, 31, 0x0006 );
//				phy_write( eng,  0, 0x5a00 );
//				phy_write( eng, 31, 0x0000 );
			}
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 11, 0x0000 );
		}
		phy_write( eng, 12, 0x1006 );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  4, 0xb000 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xdf20 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 25, 0x8c00 );
			phy_write( eng, 26, 0x0040 );
			phy_write( eng,  0, 0x1140 );
			phy_write( eng, 14, 0x0000 );
			phy_write( eng, 12, 0x1006 );
			phy_write( eng, 23, 0x2109 );
		}
	}
}

//------------------------------------------------------------
void phy_realtek3 (MAC_ENGINE *eng) {//RTL8211C
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				if ( eng->arg.GIEEE_sel == 0 ) {   //Test Mode 1
					phy_write( eng,  9, 0x2000 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Test Mode 2
					phy_write( eng,  9, 0x4000 );
				}
				else if ( eng->arg.GIEEE_sel == 2 ) {//Test Mode 3
					phy_write( eng,  9, 0x6000 );
				}
				else {                           //Test Mode 4
					phy_write( eng,  9, 0x8000 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				eng->phy.PHY_11h = phy_read( eng, PHY_SR );
				phy_write( eng, 17, eng->phy.PHY_11h & 0xfff7 );
				phy_write( eng, 14, 0x0660 );

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng, 16, 0x00a0 );//MDI  //From Channel A
				}
				else {
					phy_write( eng, 16, 0x0080 );//MDIX //From Channel B
				}
			}
			else {
//				if ( eng->arg.GIEEE_sel == 0 ) {//Pseudo-random pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  0, 0x5a21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else if ( eng->arg.GIEEE_sel == 1 ) {//��FF�� pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0xff21 );
//					phy_write( eng, 31, 0x0000 );
//				}
//				else {//��00�� pattern
//					phy_write( eng, 31, 0x0006 );
//					phy_write( eng,  2, 0x05ee );
//					phy_write( eng,  0, 0x0021 );
//					phy_write( eng, 31, 0x0000 );
//				}
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_write( eng,  0, 0x9200 );
		phy_Wait_Reset_Done( eng );
		phy_delay(30);

		phy_write( eng, 17, 0x401c );
		phy_write( eng, 12, 0x0006 );

		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 11, 0x0002 );
		}
		else {
			phy_basic_setting( eng );
		}
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 31, 0x0001 );
			phy_write( eng,  4, 0xb000 );
			phy_write( eng,  3, 0xff41 );
			phy_write( eng,  2, 0xd720 );
			phy_write( eng,  1, 0x0140 );
			phy_write( eng,  0, 0x00bb );
			phy_write( eng,  4, 0xb800 );
			phy_write( eng,  4, 0xb000 );

			phy_write( eng, 31, 0x0000 );
			phy_write( eng, 25, 0x8400 );
			phy_write( eng, 26, 0x0020 );
			phy_write( eng,  0, 0x0140 );
			phy_write( eng, 14, 0x0210 );
			phy_write( eng, 12, 0x0200 );
			phy_write( eng, 23, 0x2109 );
			phy_write( eng, 23, 0x2139 );
		}
		else {
			phy_Reset( eng );
		}
	}
} // End void phy_realtek3 (MAC_ENGINE *eng)

//------------------------------------------------------------
//external loop 100M: OK
//external loop 10M : OK
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_realtek4 (MAC_ENGINE *eng) {//RTL8201F
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_write( eng, 31, 0x0007 );
	eng->phy.PHY_10h = phy_read( eng, 16 );
	if ( ( eng->phy.PHY_10h & 0x0008 ) == 0x0 ) {
		phy_write( eng, 16, eng->phy.PHY_10h | 0x0008 );
		printf("\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h);
#if defined(ENABLE_LOG_FILE)
		if ( eng->run.TM_IOTiming ) fprintf( eng->fp_io, "\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h );
		if ( !eng->run.TM_Burst ) fprintf( eng->fp_log, "\n\n[Warning] Page 7 Register 16, bit 3 must be 1 [Reg10h_7:%04lx]\n\n", eng->phy.PHY_10h );
#endif
	}
	phy_write( eng, 31, 0x0000 );

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_write( eng, 31, 0x0004 );
			phy_write( eng, 16, 0x4077 );
			phy_write( eng, 21, 0xc5a0 );
			phy_write( eng, 31, 0x0000 );

			if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng,  0, 0x8000 ); // Reset PHY
				phy_write( eng, 24, 0x0310 ); // Disable ALDPS

				if ( eng->arg.GIEEE_sel == 0 ) {
					phy_write( eng, 28, 0x40c2 ); //Force MDI //From Channel A (RJ45 pair 1, 2)
				}
				else {
					phy_write( eng, 28, 0x40c0 ); //Force MDIX//From Channel B (RJ45 pair 3, 6)
				}
				phy_write( eng,  0, 0x2100 );       //Force 100M/Full Duplex)
			}
		}
		else {
			phy_Reset( eng );
		}
	}
    else if ( eng->phy.loop_phy ) {
        // Internal loopback
        if ( eng->run.Speed_sel[ 1 ] ) {
            // Enable 100M PCS loop back; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
            phy_write( eng, 31, 0x0 );
            phy_write( eng, 0, 0x6100 );
            phy_write( eng, 31, 0x7 );
            phy_write( eng, 16, 0x1FF8 );
            phy_write( eng, 16, 0x0FF8 );
            phy_write( eng, 31, 0x0 );
            phy_delay(20);
        }else if ( eng->run.Speed_sel[ 2 ] ) {
            // Enable 10M PCS loop back; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
            phy_write( eng, 31, 0x0 );
            phy_write( eng, 0, 0x4100 );
            phy_write( eng, 31, 0x7 );
            phy_write( eng, 16, 0x1FF8 );
            phy_write( eng, 16, 0x0FF8 );
            phy_write( eng, 31, 0x0 );
            phy_delay(20);
        }        
    }	
	else {
        // External loopback
        if ( eng->run.Speed_sel[ 1 ] ) {
            // Enable 100M MDI loop back Nway option; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
            phy_write( eng, 31, 0x0 );
            phy_write( eng, 4, 0x01E1 );
            phy_write( eng, 0, 0x1200 );
            phy_delay(4000);
        }else if ( eng->run.Speed_sel[ 2 ] ) {
            // Enable 10M MDI loop back Nway option; RTL8201(F_FL_FN)-VB-CG_DataSheet_1.6.pdf
            phy_write( eng, 31, 0x0 );
            phy_write( eng, 4, 0x0061 );
            phy_write( eng, 0, 0x1200 );
            phy_delay(4000);
        }
	}
}

//------------------------------------------------------------
void recov_phy_realtek5 (MAC_ENGINE *eng) {//RTL8211F
	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {
				phy_write( eng, 31, 0x0000 );
				phy_write( eng, 24, 0x2118 );//RGMII
				phy_write( eng,  9, 0x0200 );
				phy_write( eng,  0, 0x9200 );
				phy_Wait_Reset_Done( eng );
			}
			else {
			}
		}
		else {
		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//V1.0
			phy_write( eng, 31, 0x0a43 );
			phy_write( eng, 24, 0x2118 );
			phy_write( eng, 31, 0x0000 );
//			//V1.6
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x8000 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0042 );
//			phy_write( eng, 21, 0x0500 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x1140 );
//			phy_write( eng, 26, 0x0040 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd88f );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0300 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x1006 );
//			phy_write( eng, 23, 0x2100 );
//			phy_write( eng, 31, 0x0000 );
		}
	}
}

//------------------------------------------------------------
void phy_realtek5 (MAC_ENGINE *eng) {//RTL8211F
	USHORT     wait_phy_ready = 0;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if ( eng->run.Speed_sel[ 0 ] ) {
				phy_write( eng, 31, 0x0000 );
				if ( eng->arg.GIEEE_sel == 0 ) {//Test Mode 1
					phy_write( eng,  9, 0x0200 );
				}
				else if ( eng->arg.GIEEE_sel == 1 ) {//Test Mode 2
					phy_write( eng,  9, 0x0400 );
				}
				else {//Test Mode 4
					phy_write( eng,  9, 0x0800 );
				}
			}
			else if ( eng->run.Speed_sel[ 1 ] ) {//option
				phy_write( eng, 31, 0x0000 );
				if ( eng->arg.GIEEE_sel == 0 ) {//Output MLT-3 from Channel A
					phy_write( eng, 24, 0x2318 );
				}
				else {//Output MLT-3 from Channel B
					phy_write( eng, 24, 0x2218 );
				}
				phy_write( eng,  9, 0x0000 );
				phy_write( eng,  0, 0x2100 );
			}
			else {
				//0: For Diff. Voltage/TP-IDL/Jitter with EEE
				//1: For Diff. Voltage/TP-IDL/Jitter without EEE
				//2: For Harmonic (all "1" patten) with EEE
				//3: For Harmonic (all "1" patten) without EEE
				//4: For Harmonic (all "0" patten) with EEE
				//5: For Harmonic (all "0" patten) without EEE
				phy_write( eng, 31, 0x0000 );
				phy_write( eng,  9, 0x0000 );
				phy_write( eng,  4, 0x0061 );
				if ( (eng->arg.GIEEE_sel & 0x1) == 0 ) {//with EEE
					phy_write( eng, 25, 0x0853 );
				}
				else {//without EEE
					phy_write( eng, 25, 0x0843 );
				}
				phy_write( eng,  0, 0x9200 );
				phy_Wait_Reset_Done( eng );

				if ( (eng->arg.GIEEE_sel & 0x6) == 0 ) {//For Diff. Voltage/TP-IDL/Jitter
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0115 );
					phy_write( eng, 16, 0x5a21 );
				}
				else if ( (eng->arg.GIEEE_sel & 0x6) == 0x2 ) {//For Harmonic (all "1" patten)
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0015 );
					phy_write( eng, 16, 0xff21 );
				}
				else {//For Harmonic (all "0" patten)
					phy_write( eng, 31, 0x0c80 );
					phy_write( eng, 18, 0x0015 );
					phy_write( eng, 16, 0x0021 );
				}
			}
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			//V1.0
			phy_write( eng, 31, 0x0a43 );
			phy_write( eng,  0, 0x8000 );
			phy_delay(20);
			phy_Wait_Reset_Done( eng );

			phy_write( eng, 24, 0x2d18 );
			phy_write( eng, 31, 0x0000 );
//			phy_delay(1200);
			phy_delay(200);

			//V1.6
//			phy_write( eng, 31, 0x0a43 );
//			phy_write( eng,  0, 0x8000 );
//			phy_delay(20);
//			phy_Wait_Reset_Done( eng );
//
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x0042 );
//			phy_write( eng, 21, 0x2500 );
//			phy_write( eng, 30, 0x0023 );
//			phy_write( eng, 22, 0x0006 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng,  0, 0x0140 );
//			phy_write( eng, 26, 0x0060 );
//			phy_write( eng, 31, 0x0007 );
//			phy_write( eng, 30, 0x002f );
//			phy_write( eng, 23, 0xd820 );
//			phy_write( eng, 31, 0x0000 );
//			phy_write( eng, 21, 0x0206 );
//			phy_write( eng, 23, 0x2120 );
//			phy_write( eng, 23, 0x2160 );
//			phy_write( eng, 31, 0x0000 );
//			phy_delay(200);
//		}
//		else if ( eng->run.Speed_sel[ 1 ] ) {//option
//		}
//		else if ( eng->run.Speed_sel[ 2 ] ) {//option
		}
		else {
			phy_Reset( eng );
		}

		// Check register 0x1A bit2 Link OK or not OK
		while ( ( (phy_read( eng, 26 ) & 0x0004) != 0x0004 ) &&
			( wait_phy_ready++ < 30 ) )
			phy_delay(100);
		if ( wait_phy_ready >= 30 )
			printf("Timeout: init RTL8211F\n");
	}
}

//------------------------------------------------------------
void phy_smsc (MAC_ENGINE *eng) {//LAN8700
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[SMSC] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
void phy_micrel (MAC_ENGINE *eng) {//KSZ8041
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );

//	phy_write( eng, 24, 0x0600 );
}

//------------------------------------------------------------
void phy_micrel0 (MAC_ENGINE *eng) {//KSZ8031/KSZ8051
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Micrel] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	//For KSZ8051RNL only
	//Reg1Fh[7] = 0(default): 25MHz Mode, XI, XO(pin 9, 8) is 25MHz(crystal/oscilator).
	//Reg1Fh[7] = 1         : 50MHz Mode, XI(pin 9) is 50MHz(oscilator).
	eng->phy.PHY_1fh = phy_read( eng, 31 );
	if ( eng->phy.PHY_1fh & 0x0080 ) sprintf(eng->phy.PHYName, "%s-50MHz Mode", eng->phy.PHYName);
	else                             sprintf(eng->phy.PHYName, "%s-25MHz Mode", eng->phy.PHYName);

	if ( eng->run.TM_IEEE ) {
		phy_Read_Write( eng,  0, 0x0000, 0x8000 | eng->phy.PHY_00h );//clr set//Rst PHY
		phy_Wait_Reset_Done( eng );

		phy_Read_Write( eng, 31, 0x0000, 0x2000 );//clr set//1Fh[13] = 1: Disable auto MDI/MDI-X
		phy_basic_setting( eng );
		phy_Read_Write( eng, 31, 0x0000, 0x0800 );//clr set//1Fh[11] = 1: Force link pass

//		phy_delay(2500);//2.5 sec
	}
	else {
		phy_Reset( eng );

		//Reg16h[6] = 1         : RMII B-to-B override
		//Reg16h[1] = 1(default): RMII override
		phy_Read_Write( eng, 22, 0x0000, 0x0042 );//clr set
	}

	if ( eng->phy.PHY_1fh & 0x0080 )
		phy_Read_Write( eng, 31, 0x0000, 0x0080 );//clr set//Reset PHY will clear Reg1Fh[7]
}

//------------------------------------------------------------
//external loop 1G  : NOT Support
//external loop 100M: OK
//external loop 10M : OK
//internal loop 1G  : no  loopback stub
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_micrel1 (MAC_ENGINE *eng) {//KSZ9031
//	int        temp;

	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

/*
	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0004 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.4[ 7: 4]: RXDV Pad Skew
	phy_write( eng, 14, temp & 0xff0f | 0x0000 );
//	phy_write( eng, 14, temp & 0xff0f | 0x00f0 );
printf("Reg2.4 = %04x -> %04x\n", temp, phy_read( eng, 14 ));

	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0005 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.5[15:12]: RXD3 Pad Skew
	//Reg2.5[11: 8]: RXD2 Pad Skew
	//Reg2.5[ 7: 4]: RXD1 Pad Skew
	//Reg2.5[ 3: 0]: RXD0 Pad Skew
	phy_write( eng, 14, 0x0000 );
//	phy_write( eng, 14, 0xffff );
printf("Reg2.5 = %04x -> %04x\n", temp, phy_read( eng, 14 ));

	phy_write( eng, 13, 0x0002 );
	phy_write( eng, 14, 0x0008 );
	phy_write( eng, 13, 0x4002 );
	temp = phy_read( eng, 14 );
	//Reg2.8[9:5]: GTX_CLK Pad Skew
	//Reg2.8[4:0]: RX_CLK Pad Skew
//	phy_write( eng, 14, temp & 0xffe0 | 0x0000 );
	phy_write( eng, 14, temp & 0xffe0 | 0x001f );
printf("Reg2.8 = %04x -> %04x\n", temp, phy_read( eng, 14 ));
*/

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_Reset( eng );//DON'T support for 1G external loopback testing
		}
		else {
			phy_Reset( eng );
		}
	}
}

//------------------------------------------------------------
//external loop 100M: OK
//external loop 10M : OK
//internal loop 100M: no  loopback stub
//internal loop 10M : no  loopback stub
void phy_micrel2 (MAC_ENGINE *eng) {//KSZ8081
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[Realtek] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 1 ] )
			phy_Reset( eng );
		else
			phy_Reset( eng );
	}
}

//------------------------------------------------------------
void recov_phy_vitesse (MAC_ENGINE *eng) {//VSC8601
	if ( eng->run.TM_Burst ) {
//		if ( eng->run.TM_IEEE ) {
//		}
//		else {
//		}
	}
	else if ( eng->phy.loop_phy ) {
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			phy_write( eng, 24, eng->phy.PHY_18h );
			phy_write( eng, 18, eng->phy.PHY_12h );
		}
	}
}

//------------------------------------------------------------
void phy_vitesse (MAC_ENGINE *eng) {//VSC8601
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)[VITESSE] %s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			phy_Reset( eng );
		}
		else {
			phy_Reset( eng );
		}
	}
	else if ( eng->phy.loop_phy ) {
		phy_Reset( eng );
	}
	else {
		if ( eng->run.Speed_sel[ 0 ] ) {
			eng->phy.PHY_18h = phy_read( eng, 24 );
			eng->phy.PHY_12h = phy_read( eng, PHY_INER );

			phy_Reset( eng );

			phy_write( eng, 24, eng->phy.PHY_18h | 0x0001 );
			phy_write( eng, 18, eng->phy.PHY_12h | 0x0020 );
		}
		else {
			phy_Reset( eng );
		}
	}
}

//------------------------------------------------------------
void phy_default (MAC_ENGINE *eng) {
	if ( DbgPrn_PHYName )
		printf("--->(%04lx %04lx)%s\n", eng->phy.PHY_ID2, eng->phy.PHY_ID3, eng->phy.PHYName);

	phy_Reset( eng );
}

//------------------------------------------------------------
// PHY Init
//------------------------------------------------------------
BOOLEAN find_phyadr (MAC_ENGINE *eng) {
	ULONG      PHY_val;
	BOOLEAN    ret = FALSE;
	CHAR       PHY_ADR_org;

#ifdef  DbgPrn_FuncHeader
	printf("find_phyadr\n");
	Debug_delay();
#endif

	PHY_ADR_org = eng->phy.Adr;
	// Check current PHY address by user setting
	PHY_val = phy_read( eng, PHY_REG_ID_1 );
	if ( PHY_IS_VALID( PHY_val ) ) {
		ret = TRUE;
	}
	else if ( eng->arg.GEn_SkipChkPHY ) {
		PHY_val = phy_read( eng, PHY_REG_BMCR );

		if ( ( PHY_val & 0x8000 ) & eng->arg.GEn_InitPHY ) {
		}
		else {
			ret = TRUE;
		}
	}

#ifdef Enable_SearchPHYID
	if ( ret == FALSE ) {
		// Scan PHY address from 0 to 31
		printf("Search PHY address\n");
		for ( eng->phy.Adr = 0; eng->phy.Adr < 32; eng->phy.Adr++ ) {
			PHY_val = phy_read( eng, PHY_REG_ID_1 );
			if ( PHY_IS_VALID( PHY_val ) ) {
				ret = TRUE;
				break;
			}
		}
		// Don't find PHY address
	}
	if ( ret == FALSE )
		eng->phy.Adr = eng->arg.GPHYADR;
#endif

	if ( ret == TRUE ) {
		if ( PHY_ADR_org != eng->phy.Adr ) {
			if ( !eng->run.TM_Burst )
				phy_id( eng, FP_LOG );

			phy_id( eng, STD_OUT );
		}
	}
	else {
		if ( !eng->run.TM_Burst )
			phy_id( eng, FP_LOG );

		phy_id( eng, STD_OUT );
		FindErr( eng, Err_PHY_Type );
	}

	return ret;
} // End BOOLEAN find_phyadr (MAC_ENGINE *eng)

//------------------------------------------------------------
char phy_chk (MAC_ENGINE *eng, ULONG id2, ULONG id3, ULONG id3_mask) {
	if ( ( eng->phy.PHY_ID2 == id2 ) && ( ( eng->phy.PHY_ID3 & id3_mask ) == ( id3 & id3_mask ) ) )
		return(1);
	else
		return(0);
}

//------------------------------------------------------------
void phy_set00h (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("phy_set00h\n");
	Debug_delay();
#endif

	if ( eng->run.TM_Burst ) {
		if ( eng->run.TM_IEEE ) {
			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
			else                              eng->phy.PHY_00h = 0x0100;
		}
		else {
			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
			else                              eng->phy.PHY_00h = 0x0100;
//			if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x1140;
//			else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x3100;
//			else                              eng->phy.PHY_00h = 0x1100;
		}
	}
	else if ( eng->phy.loop_phy ) {
		if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x4140;
		else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x6100;
		else                              eng->phy.PHY_00h = 0x4100;
	}
	else {
		if      ( eng->run.Speed_sel[ 0 ] ) eng->phy.PHY_00h = 0x0140;
		else if ( eng->run.Speed_sel[ 1 ] ) eng->phy.PHY_00h = 0x2100;
		else                              eng->phy.PHY_00h = 0x0100;
	}
}

//------------------------------------------------------------
void phy_sel (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("phy_sel\n");
	Debug_delay();
#endif

	eng->phy.PHY_ID2 = phy_read( eng, PHY_REG_ID_1 );
	eng->phy.PHY_ID3 = phy_read( eng, PHY_REG_ID_2 );
	phy_set00h( eng );

	if      ((eng->phy.PHY_ID2 == 0xffff) && ( eng->phy.PHY_ID3 == 0xffff ) && !eng->arg.GEn_SkipChkPHY) {
		sprintf( eng->phy.PHYName, "--" );
		FindErr( eng, Err_PHY_Type );
	}
#ifdef Enable_CheckZeroPHYID
	else if ((eng->phy.PHY_ID2 == 0x0000) && ( eng->phy.PHY_ID3 == 0x0000 ) && !eng->arg.GEn_SkipChkPHY) {
		sprintf( eng->phy.PHYName, "--" );
		FindErr( eng, Err_PHY_Type );
	}
#endif

	if      ( phy_chk( eng, 0x0362, 0x5e6a, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM54612"         ); if ( eng->arg.GEn_InitPHY ) phy_broadcom0( eng );}//BCM54612         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0362, 0x5d10, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM54616S"        ); if ( eng->arg.GEn_InitPHY ) phy_broadcom0( eng );}//BCM54616S        1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0143, 0xBCA0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5481"          ); if ( eng->arg.GEn_InitPHY ) phy_broadcom0( eng );}//BCM5481          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0020, 0x60B0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "BCM5464SR"        ); if ( eng->arg.GEn_InitPHY ) phy_broadcom0( eng );}//BCM5464SR        1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0040, 0x61e0, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "BCM5221"          ); if ( eng->arg.GEn_InitPHY ) phy_broadcom ( eng );}//BCM5221             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0141, 0x0dd0, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "88E1512"          ); if ( eng->arg.GEn_InitPHY ) phy_marvell2 ( eng );}//88E1512          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0xff00, 0x1761, 0xffff      ) ) { sprintf( eng->phy.PHYName, "88E6176(IntLoop)" ); if ( eng->arg.GEn_InitPHY ) phy_marvell1 ( eng );}//88E6176          1G/100/10M  2 RGMII Switch
	else if ( phy_chk( eng, 0xff00, 0x1152, 0xffff      ) ) { sprintf( eng->phy.PHYName, "88E6320(IntLoop)" ); if ( eng->arg.GEn_InitPHY ) phy_marvell1 ( eng );}//88E6320          1G/100/10M  2 RGMII Switch
	else if ( phy_chk( eng, 0x0141, 0x0e90, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "88E1310"          ); if ( eng->arg.GEn_InitPHY ) phy_marvell0 ( eng );}//88E1310          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0141, 0x0cc0, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "88E1111"          ); if ( eng->arg.GEn_InitPHY ) phy_marvell  ( eng );}//88E1111          1G/100/10M  GMII, MII, RGMII
	else if ( phy_chk( eng, 0x001c, 0xc816, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8201F"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek4 ( eng );}//RTL8201F            100/10M  MII, RMII
	else if ( phy_chk( eng, 0x001c, 0xc815, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "RTL8201E"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek0 ( eng );}//RTL8201E            100/10M  MII, RMII(RTL8201E(L)-VC only)
	else if ( phy_chk( eng, 0x001c, 0xc912, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211C"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek3 ( eng );}//RTL8211C         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x001c, 0xc914, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211D"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek1 ( eng );}//RTL8211D         1G/100/10M  GMII(RTL8211DN/RTL8211DG only), MII(RTL8211DN/RTL8211DG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc915, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211E"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek2 ( eng );}//RTL8211E         1G/100/10M  GMII(RTL8211EG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc916, 0xffff      ) ) { sprintf( eng->phy.PHYName, "RTL8211F"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek5 ( eng );}//RTL8211F         1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0000, 0x8201, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "RTL8201N"         ); if ( eng->arg.GEn_InitPHY ) phy_realtek  ( eng );}//RTL8201N            100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0007, 0xc0c4, PHYID3_Mask ) ) { sprintf( eng->phy.PHYName, "LAN8700"          ); if ( eng->arg.GEn_InitPHY ) phy_smsc     ( eng );}//LAN8700             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0022, 0x1555, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8031/KSZ8051"  ); if ( eng->arg.GEn_InitPHY ) phy_micrel0  ( eng );}//KSZ8051/KSZ8031     100/10M  RMII
	else if ( phy_chk( eng, 0x0022, 0x1622, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ9031"          ); if ( eng->arg.GEn_InitPHY ) phy_micrel1  ( eng );}//KSZ9031          1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0022, 0x1562, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8081"          ); if ( eng->arg.GEn_InitPHY ) phy_micrel2  ( eng );}//KSZ8081             100/10M  MII, RMII
	else if ( phy_chk( eng, 0x0022, 0x1512, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "KSZ8041"          ); if ( eng->arg.GEn_InitPHY ) phy_micrel   ( eng );}//KSZ8041             100/10M  RMII
	else if ( phy_chk( eng, 0x0007, 0x0421, 0xfff0      ) ) { sprintf( eng->phy.PHYName, "VSC8601"          ); if ( eng->arg.GEn_InitPHY ) phy_vitesse  ( eng );}//VSC8601          1G/100/10M  RGMII
	else                                                    { sprintf( eng->phy.PHYName, "default"          ); if ( eng->arg.GEn_InitPHY ) phy_default  ( eng );}//
}

//------------------------------------------------------------
void recov_phy (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("recov_phy\n");
	Debug_delay();
#endif

	if      ( phy_chk( eng, 0x0362, 0x5e6a, 0xfff0      ) ) recov_phy_broadcom0( eng );//BCM54612  1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0362, 0x5d10, 0xfff0      ) ) recov_phy_broadcom0( eng );//BCM54616S 1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0143, 0xBCA0, 0xfff0      ) ) recov_phy_broadcom0( eng );//BCM5481   1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0020, 0x60B0, 0xfff0      ) ) recov_phy_broadcom0( eng );//BCM5464SR 1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0141, 0x0dd0, 0xfff0      ) ) recov_phy_marvell2 ( eng );//88E1512   1G/100/10M  RGMII
	else if ( phy_chk( eng, 0xff00, 0x1761, 0xffff      ) ) recov_phy_marvell1 ( eng );//88E6176   1G/100/10M  2 RGMII Switch
	else if ( phy_chk( eng, 0x0141, 0x0e90, 0xfff0      ) ) recov_phy_marvell0 ( eng );//88E1310   1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0141, 0x0cc0, PHYID3_Mask ) ) recov_phy_marvell  ( eng );//88E1111   1G/100/10M  GMII, MII, RGMII
	else if ( phy_chk( eng, 0x001c, 0xc914, 0xffff      ) ) recov_phy_realtek1 ( eng );//RTL8211D  1G/100/10M  GMII(RTL8211DN/RTL8211DG only), MII(RTL8211DN/RTL8211DG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc915, 0xffff      ) ) recov_phy_realtek2 ( eng );//RTL8211E  1G/100/10M  GMII(RTL8211EG only), RGMII
	else if ( phy_chk( eng, 0x001c, 0xc912, 0xffff      ) ) recov_phy_realtek3 ( eng );//RTL8211C  1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x001c, 0xc916, 0xffff      ) ) recov_phy_realtek5 ( eng );//RTL8211F  1G/100/10M  RGMII
	else if ( phy_chk( eng, 0x0007, 0x0421, 0xfff0      ) ) recov_phy_vitesse  ( eng );//VSC8601   1G/100/10M  RGMII
}

//------------------------------------------------------------
void init_phy (MAC_ENGINE *eng) {
#ifdef  DbgPrn_FuncHeader
	printf("init_phy\n");
	Debug_delay();
#endif

	sprintf( eng->phy.PHYID, "PHY%d", eng->run.MAC_idx + 1 );

	if ( DbgPrn_PHYInit )
		phy_dump( eng, eng->phy.PHYID );

	if ( find_phyadr( eng ) == TRUE )
		phy_sel( eng );

	if ( DbgPrn_PHYInit )
		phy_dump( eng, eng->phy.PHYID );
}


