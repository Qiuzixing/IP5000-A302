/*
 * Global Data
 */
// Y,C,RGB offset
// for register 73~75
static  BYTE bCSCOffset_16_235[] =
{
    0x00,0x80,0x00
};

static  BYTE bCSCOffset_0_255[] =
{
    0x10,0x80,0x10
};

#ifdef SUPPORT_INPUTRGB
    static  BYTE bCSCMtx_RGB2YUV_ITU601_16_235[] =
    {
        0xB2,0x04,0x64,0x02,0xE9,0x00,
        0x93,0x3C,0x16,0x04,0x56,0x3F,
        0x49,0x3D,0x9F,0x3E,0x16,0x04
    } ;

    static  BYTE bCSCMtx_RGB2YUV_ITU601_0_255[] =
    {
        0x09,0x04,0x0E,0x02,0xC8,0x00,
        0x0E,0x3D,0x83,0x03,0x6E,0x3F,
        0xAC,0x3D,0xD0,0x3E,0x83,0x03
    } ;

    static  BYTE bCSCMtx_RGB2YUV_ITU709_16_235[] =
    {
        0xB8,0x05,0xB4,0x01,0x93,0x00,
        0x49,0x3C,0x16,0x04,0x9F,0x3F,
        0xD9,0x3C,0x10,0x3F,0x16,0x04
    } ;

    static  BYTE bCSCMtx_RGB2YUV_ITU709_0_255[] =
    {
        0xE5,0x04,0x78,0x01,0x81,0x00,
        0xCE,0x3C,0x83,0x03,0xAE,0x3F,
        0x49,0x3D,0x33,0x3F,0x83,0x03
    } ;
#endif

#ifdef SUPPORT_INPUTYUV

    static  BYTE bCSCMtx_YUV2RGB_ITU601_16_235[] =
    {
        0x00,0x08,0x6A,0x3A,0x4F,0x3D,
        0x00,0x08,0xF7,0x0A,0x00,0x00,
        0x00,0x08,0x00,0x00,0xDB,0x0D
    } ;

    static  BYTE bCSCMtx_YUV2RGB_ITU601_0_255[] =
    {
        0x4F,0x09,0x81,0x39,0xDF,0x3C,
        0x4F,0x09,0xC2,0x0C,0x00,0x00,
        0x4F,0x09,0x00,0x00,0x1E,0x10
    } ;

    static  BYTE bCSCMtx_YUV2RGB_ITU709_16_235[] =
    {
        0x00,0x08,0x53,0x3C,0x89,0x3E,
        0x00,0x08,0x51,0x0C,0x00,0x00,
        0x00,0x08,0x00,0x00,0x87,0x0E
    } ;

    static  BYTE bCSCMtx_YUV2RGB_ITU709_0_255[] =
    {
        0x4F,0x09,0xBA,0x3B,0x4B,0x3E,
        0x4F,0x09,0x56,0x0E,0x00,0x00,
        0x4F,0x09,0x00,0x00,0xE7,0x10
    } ;
#endif

/* 
 * Macro
 */
#define Switch_HDMITX_Bank(x)   HDMITX_WriteI2C_Byte(0x0f,(x)&1)

#define HDMITX_OrREG_Byte(reg,ormask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) | (ormask)))
#define HDMITX_AndREG_Byte(reg,andmask) HDMITX_WriteI2C_Byte(reg,(HDMITX_ReadI2C_Byte(reg) & (andmask)))
#define HDMITX_SetREG_Byte(reg,andmask,ormask) HDMITX_WriteI2C_Byte(reg,((HDMITX_ReadI2C_Byte(reg) & (andmask))|(ormask)))

/*
 * DEBUG
 */
static void
DumpCatHDMITXReg(void)
{
    int i,j ;
    //BYTE reg ;
    //BYTE bank ;
    BYTE ucData ;

    ErrorF("       ") ;
    for(j = 0 ; j < 16 ; j++)
    {
        ErrorF(" %02X",j) ;
        if((j == 3)||(j==7)||(j==11))
        {
            ErrorF("  ") ;
        }
    }
    ErrorF("\n        -----------------------------------------------------\n") ;

    Switch_HDMITX_Bank(0) ;

    for(i = 0 ; i < 0x100 ; i+=16)
    {
        ErrorF("[%3X]  ",i) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF)) ;
            ErrorF(" %02X",ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                ErrorF(" -") ;
            }
        }
        ErrorF("\n") ;
        if((i % 0x40) == 0x30)
        {
            ErrorF("        -----------------------------------------------------\n") ;
        }
    }

    Switch_HDMITX_Bank(1) ;
    for(i = 0x130; i < 0x1B0 ; i+=16)
    {
        ErrorF("[%3X]  ",i) ;
        for(j = 0 ; j < 16 ; j++)
        {
            ucData = HDMITX_ReadI2C_Byte((BYTE)((i+j)&0xFF)) ;
            ErrorF(" %02X",ucData) ;
            if((j == 3)||(j==7)||(j==11))
            {
                ErrorF(" -") ;
            }
        }
        ErrorF("\n") ;
        if(i == 0x160)
        {
            ErrorF("        -----------------------------------------------------\n") ;
        }

    }
    Switch_HDMITX_Bank(0) ;
} /* DumpCatHDMITXReg */

/*
 * Basic Control Functions
 */
static void
ENABLE_NULL_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ACP_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ACP_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC1_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC1_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_ISRC2_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC2_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AVI_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AVI_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_AUD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AUD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_SPD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_SPD_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}


static void
ENABLE_MPG_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_MPG_INFOFRM_CTRL,B_ENABLE_PKT|B_REPEAT_PKT);
}

static void
DISABLE_NULL_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_NULL_CTRL,0);
}


static void
DISABLE_ACP_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ACP_CTRL,0);
}


static void
DISABLE_ISRC1_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC1_CTRL,0);
}


static void
DISABLE_ISRC2_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_ISRC2_CTRL,0);
}


static void
DISABLE_AVI_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AVI_INFOFRM_CTRL,0);
}


static void
DISABLE_AUD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_AUD_INFOFRM_CTRL,0);
}


static void
DISABLE_SPD_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_SPD_INFOFRM_CTRL,0);
}


static void
DISABLE_MPG_INFOFRM_PKT(void)
{

    HDMITX_WriteI2C_Byte(REG_TX_MPG_INFOFRM_CTRL,0);
}


/*
 * Audio
 */
//////////////////////////////////////////////////////////////////////
// Function: SetNCTS
// Parameter: PCLK - video clock in Hz.
//            Fs - Encoded audio sample rate
//                          AUDFS_22p05KHz  4
//                          AUDFS_44p1KHz 0
//                          AUDFS_88p2KHz 8
//                          AUDFS_176p4KHz    12
//
//                          AUDFS_24KHz  6
//                          AUDFS_48KHz  2
//                          AUDFS_96KHz  10
//                          AUDFS_192KHz 14
//
//                          AUDFS_768KHz 9
//
//                          AUDFS_32KHz  3
//                          AUDFS_OTHER    1

// Return: ER_SUCCESS if success
// Remark: set N value,the CTS will be auto generated by HW.
// Side-Effect: register bank will reset to bank 0.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetNCTS(ULONG PCLK,BYTE Fs)
{
    ULONG n,MCLK ;

    MCLK = Fs * 256 ; // MCLK = fs * 256 ;

    ErrorF("SetNCTS(%ld,%ld): MCLK = %ld\n",PCLK,Fs,MCLK) ;

    if( PCLK )
    {
    	switch (Fs) {
    		case AUDFS_32KHz:
    			switch (PCLK) {
    				case 74175000: n = 11648; break;
    				case 14835000: n = 11648; break;
    				default: n = 4096;
    			}
    			break;
    		case AUDFS_44p1KHz:
    			switch (PCLK) {
    				case 74175000: n = 17836; break;
    				case 14835000: n = 8918; break;
    				default: n = 6272;
    			}
    			break;
    		case AUDFS_48KHz:
    			switch (PCLK) {
    				case 74175000: n = 11648; break;
    				case 14835000: n = 5824; break;
    				default: n = 6144;
    			}
    			break;
    		case AUDFS_88p2KHz:
    			switch (PCLK) {
    				case 74175000: n = 35672; break;
    				case 14835000: n = 17836; break;
    				default: n = 12544;
    			}
    			break;
    		case AUDFS_96KHz:
    			switch (PCLK) {
    				case 74175000: n = 23296; break;
    				case 14835000: n = 11648; break;
    				default: n = 12288;
    			}
    			break;
    		case AUDFS_176p4KHz:
    			switch (PCLK) {
    				case 74175000: n = 71344; break;
    				case 14835000: n = 35672; break;
    				default: n = 25088;
    			}
    			break;
    		case AUDFS_192KHz:
    			switch (PCLK) {
    				case 74175000: n = 46592; break;
    				case 14835000: n = 23296; break;
    				default: n = 24576;
    			}
    			break;
    		default: n = MCLK / 2000;
    	}
    }
    else
    {
        switch(Fs)
        {
		case AUDFS_32KHz: n = 4096; break;
		case AUDFS_44p1KHz: n = 6272; break;
		case AUDFS_48KHz: n = 6144; break;
		case AUDFS_88p2KHz: n = 12544; break;
		case AUDFS_96KHz: n = 12288; break;
		case AUDFS_176p4KHz: n = 25088; break;
		case AUDFS_192KHz: n = 24576; break;
		default: n = 6144;
        }

    }


    //ErrorF("N = %ld\n",n) ;
    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((n)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((n>>8)&0xFF)) ;
    HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((n>>16)&0xF)) ;
    Switch_HDMITX_Bank(0) ;

    HDMITX_WriteI2C_Byte(REG_TX_PKT_SINGLE_CTRL,0) ; // D[1] = 0,HW auto count CTS

    HDMITX_SetREG_Byte(REG_TX_CLK_CTRL0,~M_EXT_MCLK_SEL,B_EXT_256FS) ;
    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetAudioFormat
// Parameter:
//    NumChannel - number of channel,from 1 to 8
//    AudioEnable - Audio source and type bit field,value of bit field are
//        ENABLE_SPDIF    (1<<4)
//        ENABLE_I2S_SRC3  (1<<3)
//        ENABLE_I2S_SRC2  (1<<2)
//        ENABLE_I2S_SRC1  (1<<1)
//        ENABLE_I2S_SRC0  (1<<0)
//    SampleFreq - the audio sample frequence in Hz
//    AudSWL - Audio sample width,only support 16,18,20,or 24.
//    AudioCatCode - The audio channel catalogy code defined in IEC 60958-3
// Return: ER_SUCCESS if done,ER_FAIL for otherwise.
// Remark: program audio channel control register and audio channel registers
//         to enable audio by input.
// Side-Effect: register bank will keep in bank zero.
//////////////////////////////////////////////////////////////////////


static SYS_STATUS
SetAudioFormat(BYTE NumChannel,BYTE AudioEnable,BYTE bSampleFreq,BYTE AudSWL,BYTE AudioCatCode)
{
    BYTE fs = bSampleFreq ;
    BYTE SWL ;

    BYTE SourceValid ;
    BYTE SoruceNum ;


    ErrorF("SetAudioFormat(%d channel,%02X,SampleFreq %d,AudSWL %d,%02X)\n",NumChannel,AudioEnable,bSampleFreq,AudSWL,AudioCatCode) ;


    InitInstanceData.bOutputAudioMode |= 0x41 ;
    if(NumChannel > 6)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S3VALID | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 4 ;
    }
    else if (NumChannel > 4)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S2VALID | B_AUD_S1VALID ;
        SoruceNum = 3 ;
    }
    else if (NumChannel > 2)
    {
        SourceValid = B_AUD_ERR2FLAT | B_AUD_S1VALID ;
        SoruceNum = 2 ;
    }
    else
    {
        SourceValid = B_AUD_ERR2FLAT ; // only two channel.
        SoruceNum = 1 ;
        InitInstanceData.bOutputAudioMode &= ~0x40 ;
    }

    AudioEnable &= ~ (M_AUD_SWL|B_SPDIFTC) ;

    switch(AudSWL)
    {
    case 16:
        SWL = AUD_SWL_16 ;
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        SWL = AUD_SWL_18 ;
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        SWL = AUD_SWL_20 ;
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
        SWL = AUD_SWL_24 ;
        AudioEnable |= M_AUD_24BIT ;
        break ;
    default:
        return ER_FAIL ;
    }


    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0) ;

    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,InitInstanceData.bOutputAudioMode) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4) ; // default mapping.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,(InitInstanceData.bAudioChannelSwap&0xF)|(AudioEnable&B_AUD_SPDIF)) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,SourceValid) ;

    // suggested to be 0x41

//     Switch_HDMITX_Bank(1) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)) ; // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,AudioCatCode) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,SoruceNum) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,0x21) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUD1CHST_CHTNUM,0x43) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUD2CHST_CHTNUM,0x65) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUD3CHST_CHTNUM,0x87) ;
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs) ; // choose clock
//     fs = ~fs ; // OFS is the one's complement of FS
//     HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL) ;
//     Switch_HDMITX_Bank(0) ;

    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,0 |((NumChannel == 1)?1:0)) ; // 2 audio channel without pre-emphasis,if NumChannel set it as 1.
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,AudioCatCode) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,SoruceNum) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs) ; // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,(fs<<4)|SWL) ;
    Switch_HDMITX_Bank(0) ;
    
    if(!(AudioEnable | B_AUD_SPDIF))
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable) ;
    }

    InitInstanceData.bAudioChannelEnable = AudioEnable ;
    
    // HDMITX_AndREG_Byte(REG_TX_SW_RST,B_AUD_RST) ;    // enable Audio
    return ER_SUCCESS;
}



static void
AutoAdjustAudio(void)
{
    unsigned long SampleFreq,TMDSClock ;
    unsigned long N ;
    unsigned long CTS ;
    BYTE fs, uc ;

//    bPendingAdjustAudioFreq = TRUE ;

//     if( CAT6611_AudioChannelEnable & B_AUD_SPDIF )
//     {
//         if(!(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK))
//         {
//             return ;
//         }
//     }
    
    Switch_HDMITX_Bank(1) ;
    N = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN2)&0xF) << 16 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN1)) <<8 ;
    N |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudN0)) ;
    
    CTS = ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt2)) << 12 ;
    CTS |= ((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt1)) <<4 ;
    CTS |= (((unsigned long)HDMITX_ReadI2C_Byte(REGPktAudCTSCnt0))>>4)&0xF ;
    Switch_HDMITX_Bank(0) ;
    
    // CTS = TMDSCLK * N / ( 128 * SampleFreq ) 
    // SampleFreq = TMDSCLK * N / (128*CTS) 
    
    if( CTS == 0 ) 
    {
        return  ;
    }

    uc = HDMITX_ReadI2C_Byte(0xc1) ;
    
	TMDSClock = InitInstanceData.TMDSClock ;

	switch(uc & 0x70)
	{
	case 0x50: 
		TMDSClock *= 5 ;
		TMDSClock /= 4 ;
		break ;
	case 0x60:
		TMDSClock *= 3 ;
		TMDSClock /= 2 ;
	}

    SampleFreq = TMDSClock/CTS ;
    SampleFreq *= N ;
    SampleFreq /= 128 ;
            
    if( SampleFreq>31000 && SampleFreq<=38050 )
    {
        InitInstanceData.bAudFs = AUDFS_32KHz ;
        fs = AUDFS_32KHz ;;
    }
    else if (SampleFreq < 46050 ) // 44.1KHz
    {
        InitInstanceData.bAudFs = AUDFS_44p1KHz ;
        fs = AUDFS_44p1KHz ;;
    }
    else if (SampleFreq < 68100 ) // 48KHz
    {
        InitInstanceData.bAudFs = AUDFS_48KHz ;
        fs = AUDFS_48KHz ;;
    }
    else if (SampleFreq < 92100 ) // 88.2 KHz
    {
        InitInstanceData.bAudFs = AUDFS_88p2KHz ;
        fs = AUDFS_88p2KHz ;;
    }
    else if (SampleFreq < 136200 ) // 96KHz
    {
        InitInstanceData.bAudFs = AUDFS_96KHz ;
        fs = AUDFS_96KHz ;;
    }
    else if (SampleFreq < 184200 ) // 176.4KHz
    {
        InitInstanceData.bAudFs = AUDFS_176p4KHz ;
        fs = AUDFS_176p4KHz ;;
    }
    else if (SampleFreq < 240200 ) // 192KHz
    {
        InitInstanceData.bAudFs = AUDFS_192KHz ;
        fs = AUDFS_192KHz ;;
    }
    else 
    {
        InitInstanceData.bAudFs = AUDFS_OTHER;
        fs = AUDFS_OTHER;;
    }

//    bPendingAdjustAudioFreq = FALSE ;
    
    SetNCTS(InitInstanceData.TMDSClock, InitInstanceData.bAudFs) ; // set N, CTS by new generated clock.
    
    Switch_HDMITX_Bank(1) ; // adjust the new fs in channel status registers
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,0x00|fs) ; // choose clock
    fs = ~fs ; // OFS is the one's complement of FS
    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_OFS_WL) ;
    uc &= 0xF ;
    uc |= fs << 4 ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,uc) ;
    
    Switch_HDMITX_Bank(0) ;

}

static void
SetupAudioChannel(void)
{
    static BYTE bEnableAudioChannel=FALSE ;
	BYTE uc ;
    if( (HDMITX_ReadI2C_Byte(REG_TX_SW_RST) & (B_AUD_RST|B_AREF_RST)) == 0) // audio enabled
    {
        Switch_HDMITX_Bank(0) ;
		uc =HDMITX_ReadI2C_Byte(REG_TX_AUDIO_CTRL0); 
        if((uc & 0x1f) == 0x10)
        {
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                SetNCTS(InitInstanceData.TMDSClock, InitInstanceData.bAudFs) ; // to enable automatic progress setting for N/CTS
                DelayMS(5);
                AutoAdjustAudio() ;
                Switch_HDMITX_Bank(0) ;
                HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, InitInstanceData.bAudioChannelEnable) ;
                bEnableAudioChannel=TRUE ;
            }
        }
		else if((uc & 0xF) == 0x00 )
		{
            SetNCTS(InitInstanceData.TMDSClock, InitInstanceData.bAudFs) ; // to enable automatic progress setting for N/CTS
            DelayMS(5);
            AutoAdjustAudio() ;
            Switch_HDMITX_Bank(0) ;
            HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, InitInstanceData.bAudioChannelEnable) ;
            bEnableAudioChannel=TRUE ;
		}
        else
        {
			/*
            if((HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)==0)
            {
                // AutoAdjustAudio() ;
                // ForceSetNCTS(CurrentPCLK, CurrentSampleFreq) ;
                if( bEnableAudioChannel == TRUE )
                {
                    Switch_HDMITX_Bank(0) ;
                    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, InitInstanceData.bAudioChannelEnable&0xF0) ;
                }
                bEnableAudioChannel=FALSE ;
            }
			*/
        }
    }
}
 
void
setCAT6613_ChStat(BYTE ucIEC60958ChStat[])
{
    BYTE uc ;
    
    Switch_HDMITX_Bank(1) ;
    uc = (ucIEC60958ChStat[0] <<1)& 0x7C ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_MODE,uc) ; 
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CAT,ucIEC60958ChStat[1]) ; // 192, audio CATEGORY
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_SRCNUM,ucIEC60958ChStat[2]&0xF) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD0CHST_CHTNUM,(ucIEC60958ChStat[2]>>4)&0xF) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,ucIEC60958ChStat[3]) ; // choose clock
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,ucIEC60958ChStat[4]) ;
    
    Switch_HDMITX_Bank(0) ;
}

void
setCAT6613_UpdateChStatFs(ULONG Fs) 
{
    BYTE uc ;
    
    /////////////////////////////////////
    // Fs should be the following value.
    // #define AUDFS_22p05KHz  4
    // #define AUDFS_44p1KHz 0
    // #define AUDFS_88p2KHz 8
    // #define AUDFS_176p4KHz    12
    // 
    // #define AUDFS_24KHz  6
    // #define AUDFS_48KHz  2
    // #define AUDFS_96KHz  10
    // #define AUDFS_192KHz 14
    // 
    // #define AUDFS_768KHz 9
    // 
    // #define AUDFS_32KHz  3
    // #define AUDFS_OTHER    1
    /////////////////////////////////////

    Switch_HDMITX_Bank(1) ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_CA_FS) ; // choose clock
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_CA_FS,uc) ; // choose clock
    uc &= 0xF0 ;
    uc |= (Fs&0xF) ;
    
    uc = HDMITX_ReadI2C_Byte(REG_TX_AUDCHST_OFS_WL) ;
    uc &= 0xF ;
    uc |= ((~Fs) << 4)&0xF0 ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDCHST_OFS_WL,uc) ;
    
    Switch_HDMITX_Bank(0) ;
    

}

void
setCAT6613_LPCMAudio(BYTE AudioSrcNum, BYTE AudSWL, BOOL bSPDIF)
{
    
    BYTE AudioEnable, AudioFormat ;
    
    AudioEnable = 0 ;
    AudioFormat = InitInstanceData.bOutputAudioMode ;
    
    
    switch(AudSWL)
    {
    case 16:
        AudioEnable |= M_AUD_16BIT ;
        break ;
    case 18:
        AudioEnable |= M_AUD_18BIT ;
        break ;
    case 20:
        AudioEnable |= M_AUD_20BIT ;
        break ;
    case 24:
    default:
        AudioEnable |= M_AUD_24BIT ;
        break ;
    }
    
    if( bSPDIF )
    {
        AudioFormat &= ~0x40 ;
        AudioEnable |= B_SPDIFTC|B_AUD_EN_I2S0 ;
    }
    else
    {
        AudioFormat |= 0x40 ;
        switch(AudioSrcNum)
        {
        case 4:
            AudioEnable |= B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;
            
        case 3:
            AudioEnable |= B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;
            
        case 2:
            AudioEnable |= B_AUD_EN_I2S1|B_AUD_EN_I2S0 ;
            break ;
            
        case 1:
        default:
            AudioFormat &= ~0x40 ;
            AudioEnable |= B_AUD_EN_I2S0 ;
            break ;
            
        }
    }
    
    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0) ;
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,AudioFormat) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4) ; // default mapping.
#ifdef USE_SPDIF_CHSTAT
    if( bSPDIF )
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0) ;
    }
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0) ;
#endif // USE_SPDIF_CHSTAT     

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,0x00) ; // regE5 = 0 ;
    
    if( bSPDIF )
    {
        BYTE i ;
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
    }    
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0,AudioEnable&0xF0) ;

    
}

void
setCAT6613_NLPCMAudio(void) // no Source Num, no I2S.
{
    BYTE AudioEnable, AudioFormat ;
    BYTE i ;
    
    AudioFormat = 0x01 ; // NLPCM must use standard I2S mode.
    AudioEnable = M_AUD_24BIT|B_SPDIFTC|B_AUD_EN_I2S0 ;
    
    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_SPDIFTC) ;
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x01) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4) ; // default mapping.
    
#ifdef USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL) ;
#else // not USE_SPDIF_CHSTAT
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0) ;
#endif // USE_SPDIF_CHSTAT     

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,0x00) ; // regE5 = 0 ;
    
    for( i = 0 ; i < 100 ; i++ )
    {
        if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
        {
            break ; // stable clock.
        }
    }
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_SPDIFTC|B_AUD_EN_I2S0) ;
}

void
setCAT6613_HBRAudio(BOOL bSPDIF)
{
    BYTE rst,uc ;
    Switch_HDMITX_Bank(0) ;
    
    rst = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) ;
    
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst | (B_AUD_RST|B_AREF_RST) ) ;

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x41) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4) ; // default mapping.

    if( bSPDIF )
    {    
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_SPDIFTC) ;
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,B_CHSTSEL) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT) ;
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0) ;
    }

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,B_HBR) ; // regE5 = 0 ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst & ~(B_AUD_RST|B_AREF_RST) ) ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1) ;
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc) ;
    
    

    if( bSPDIF )
    {    
		BYTE i ;
        for( i = 0 ; i < 100 ; i++ )
        {
            if(HDMITX_ReadI2C_Byte(REG_TX_CLK_STATUS2) & B_OSF_LOCK)
            {
                break ; // stable clock.
            }
        }
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_SPDIFTC|B_AUD_EN_SPDIF) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0) ;
    }
}

void
setCAT6613_DSDAudio(void)
{
    // to be continue
    BYTE rst, uc ;
    rst = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) ;
    
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst | (B_AUD_RST|B_AREF_RST) ) ;

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL1,0x41) ; // regE1 bOutputAudioMode should be loaded from ROM image.
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_FIFOMAP,0xE4) ; // default mapping.

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL3,0) ;

    HDMITX_WriteI2C_Byte(REG_TX_AUD_SRCVALID_FLAT,0x00) ;
    HDMITX_WriteI2C_Byte(REG_TX_AUD_HDAUDIO,B_DSD) ; // regE5 = 0 ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST, rst & ~(B_AUD_RST|B_AREF_RST) ) ;

    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1) ;
    uc &= ~M_AUD_DIV ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc) ;
    
    

    HDMITX_WriteI2C_Byte(REG_TX_AUDIO_CTRL0, M_AUD_24BIT|B_AUD_EN_I2S3|B_AUD_EN_I2S2|B_AUD_EN_I2S1|B_AUD_EN_I2S0) ;
}

void
_SetAVMute(BYTE bEnable)
{
    BYTE uc ;

    Switch_HDMITX_Bank(0) ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_GCP) ;
    uc &= ~B_TX_SETAVMUTE ;
    uc |= bEnable?B_TX_SETAVMUTE:0 ;
    HDMITX_WriteI2C_Byte(REG_TX_GCP,uc) ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_GENERAL_CTRL,B_ENABLE_PKT|B_REPEAT_PKT) ;
}

void
SetAVMute(BYTE bEnable)
{
    InitInstanceData.bMuteRequested = bEnable;
	_SetAVMute(bEnable);
}
EXPORT_SYMBOL(SetAVMute);

BOOL EnableAudioOutput(BYTE bAudioSampleFreq,BYTE ChannelNumber,BYTE bAudSWL,BYTE bSPDIF)
{
    BYTE bAudioChannelEnable ;
//    unsigned long N ;

    //InitInstanceData.TMDSClock = VideoPixelClock ;
    InitInstanceData.bAudFs = bAudioSampleFreq ;
    
    //ErrorF("EnableAudioOutput(%d,%ld,%x,%d,%d,%d);\n",0,VideoPixelClock,bAudioSampleFreq,ChannelNumber,bAudSWL,bSPDIF) ;

    switch(ChannelNumber)
    {
    case 7:
    case 8:
        bAudioChannelEnable = 0xF ;
        break ;
    case 6:
    case 5:
        bAudioChannelEnable = 0x7 ;
        break ;
    case 4:
    case 3:
        bAudioChannelEnable = 0x3 ;
        break ;
    case 2:
    case 1:
    default:
        bAudioChannelEnable = 0x1 ;
        break ;
    }

    if(bSPDIF) bAudioChannelEnable |= B_AUD_SPDIF ;

    if( bSPDIF )
    {
        Switch_HDMITX_Bank(1) ;
        HDMITX_WriteI2C_Byte(REGPktAudCTS0,0x50) ;
        HDMITX_WriteI2C_Byte(REGPktAudCTS1,0x73) ;
        HDMITX_WriteI2C_Byte(REGPktAudCTS2,0x00) ;
        
        HDMITX_WriteI2C_Byte(REGPktAudN0,0) ;
        HDMITX_WriteI2C_Byte(REGPktAudN1,0x18) ;
        HDMITX_WriteI2C_Byte(REGPktAudN2,0) ;
        Switch_HDMITX_Bank(0) ;
    
        HDMITX_WriteI2C_Byte(0xC5, 2) ; // D[1] = 0, HW auto count CTS
    }
    else
    {
        SetNCTS(InitInstanceData.TMDSClock,bAudioSampleFreq) ;
    }

    /*
    if(InitInstanceData.TMDSClock != 0)
    {
        SetNCTS(InitInstanceData.TMDSClock,bAudioSampleFreq) ;
    }
    else
    {
        switch(bAudioSampleFreq)
        {
		case AUDFS_32KHz: N = 4096; break;
		case AUDFS_44p1KHz: N = 6272; break;
		case AUDFS_48KHz: N = 6144; break;
		case AUDFS_88p2KHz: N = 12544; break;
		case AUDFS_96KHz: N = 12288; break;
		case AUDFS_176p4KHz: N = 25088; break;
		case AUDFS_192KHz: N = 24576; break;
		default: N = 6144;
        }
        Switch_HDMITX_Bank(1) ;
        HDMITX_WriteI2C_Byte(REGPktAudN0,(BYTE)((N)&0xFF)) ;
        HDMITX_WriteI2C_Byte(REGPktAudN1,(BYTE)((N>>8)&0xFF)) ;
        HDMITX_WriteI2C_Byte(REGPktAudN2,(BYTE)((N>>16)&0xF)) ;
        Switch_HDMITX_Bank(0) ;
        HDMITX_WriteI2C_Byte(REG_TX_PKT_SINGLE_CTRL,0) ; // D[1] = 0,HW auto count CTS
    }
    */

	//HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_AUD_RST|B_AREF_RST)) ;
    SetAudioFormat(ChannelNumber,bAudioChannelEnable,bAudioSampleFreq,bAudSWL,bSPDIF) ;

    #if 0
    DumpCatHDMITXReg() ;
    #endif // DEBUG
    return TRUE ;
} /* EnableAudioOutput */

void
EnableHDMIAudio(BYTE AudioType, BOOL bSPDIF,  ULONG SampleFreq,  BYTE ChNum, BYTE *pIEC60958ChStat, ULONG TMDSClock)
{
    static BYTE ucIEC60958ChStat[5] ;
    BYTE Fs = 0;
    
    if( AudioType != T_AUDIO_DSD) 
    {
        // one bit audio have no channel status.
        switch(SampleFreq)
        {
        case  44100: Fs =  AUDFS_44p1KHz ; break ;
        case  88200: Fs =  AUDFS_88p2KHz ; break ;
        case 176400: Fs = AUDFS_176p4KHz ; break ;
        case  32000: Fs =    AUDFS_32KHz ; break ;
        case  48000: Fs =    AUDFS_48KHz ; break ;
        case  96000: Fs =    AUDFS_96KHz ; break ;
        case 192000: Fs =   AUDFS_192KHz ; break ;
        case 768000: Fs =   AUDFS_768KHz ; break ;
        default:     
            SampleFreq = 48000 ;
            Fs =    AUDFS_48KHz ; 
            break ; // default, set Fs = 48KHz.
        }
        
        if( pIEC60958ChStat == NULL )
        {
            ucIEC60958ChStat[0] = 0 ;
            ucIEC60958ChStat[1] = 0 ;
            ucIEC60958ChStat[2] = (ChNum+1)/2 ;
            
            if(ucIEC60958ChStat[2]<1)
            {
                ucIEC60958ChStat[2] = 1 ;
            }
            else if( ucIEC60958ChStat[2] >4 )
            {
                ucIEC60958ChStat[2] = 4 ;
            }
            
            ucIEC60958ChStat[3] = Fs ;
            ucIEC60958ChStat[4] = (((~Fs)<<4) & 0xF0) | 0xB ; // Fs | 24bit word length
            
            pIEC60958ChStat = ucIEC60958ChStat ;
        }
    }
    
    switch(AudioType)
    {
    case T_AUDIO_HBR:
        pIEC60958ChStat[0] |= 1<<1 ;
        pIEC60958ChStat[3] &= 0xF0 ;
        pIEC60958ChStat[3] |= AUDFS_768KHz ;
        pIEC60958ChStat[4] &= 0x0F ;
        pIEC60958ChStat[3] |= ((~AUDFS_768KHz)<<4) & 0xF0 ;
        
        setCAT6613_ChStat(pIEC60958ChStat) ;
        SetNCTS(TMDSClock, AUDFS_192KHz) ;
        setCAT6613_HBRAudio(bSPDIF) ;
 
        break ;        
    case T_AUDIO_DSD:

        SetNCTS(TMDSClock, AUDFS_44p1KHz) ;
        setCAT6613_DSDAudio() ;
        break ;
    case T_AUDIO_NLPCM:
        pIEC60958ChStat[0] |= 1<<1 ;
        setCAT6613_ChStat(pIEC60958ChStat) ;
        SetNCTS(TMDSClock, Fs) ;
        setCAT6613_NLPCMAudio() ;
        break ;
    case T_AUDIO_LPCM:
        pIEC60958ChStat[0] &= ~(1<<1) ;
        setCAT6613_ChStat(pIEC60958ChStat) ;
        SetNCTS(TMDSClock, Fs) ;
        setCAT6613_LPCMAudio((ChNum+1)/2, 24, bSPDIF) ;
        // can add auto adjust         
        break ;
    }    
}

/*
 * Video
 */
//////////////////////////////////////////////////////////////////////
// Function: SetInputMode
// Parameter: InputMode,bInputSignalType
//      InputMode - use [1:0] to identify the color space for reg70[7:6],
//                  definition:
//                     #define F_MODE_RGB444  0
//                     #define F_MODE_YUV422 1
//                     #define F_MODE_YUV444 2
//                     #define F_MODE_CLRMOD_MASK 3
//      bInputSignalType - defined the CCIR656 D[0],SYNC Embedded D[1],and
//                     DDR input in D[2].
// Return: N/A
// Remark: program Reg70 with the input value.
// Side-Effect: Reg70.
//////////////////////////////////////////////////////////////////////

static void
SetInputMode(BYTE InputMode,BYTE bInputSignalType)
{
    BYTE ucData ;

    ErrorF("SetInputMode(%02X,%02X)\n",InputMode,bInputSignalType) ;

    ucData = HDMITX_ReadI2C_Byte(REG_TX_INPUT_MODE) ;

    ucData &= ~(M_INCOLMOD|B_2X656CLK|B_SYNCEMB|B_INDDR|B_PCLKDIV2) ;

    switch(InputMode & F_MODE_CLRMOD_MASK)
    {
    case F_MODE_YUV422:
        ucData |= B_IN_YUV422 ;
        break ;
    case F_MODE_YUV444:
        ucData |= B_IN_YUV444 ;
        break ;
    case F_MODE_RGB444:
    default:
        ucData |= B_IN_RGB ;
        break ;
    }

    if(bInputSignalType & T_MODE_PCLKDIV2)
    {
        ucData |= B_PCLKDIV2 ; //ErrorF("PCLK Divided by 2 mode\n") ;
    }
    if(bInputSignalType & T_MODE_CCIR656)
    {
        ucData |= B_2X656CLK ; //ErrorF("CCIR656 mode\n") ;
    }

    if(bInputSignalType & T_MODE_SYNCEMB)
    {
        ucData |= B_SYNCEMB ; //ErrorF("Sync Embedded mode\n") ;
    }

    if(bInputSignalType & T_MODE_INDDR)
    {
        ucData |= B_INDDR ; //ErrorF("Input DDR mode\n") ;
    }

    HDMITX_WriteI2C_Byte(REG_TX_INPUT_MODE,ucData) ;
}

//////////////////////////////////////////////////////////////////////
// Function: SetCSCScale
// Parameter: bInputMode -
//             D[1:0] - Color Mode
//             D[4] - Colorimetry 0: ITU_BT601 1: ITU_BT709
//             D[5] - Quantization 0: 0_255 1: 16_235
//             D[6] - Up/Dn Filter 'Required'
//                    0: no up/down filter
//                    1: enable up/down filter when csc need.
//             D[7] - Dither Filter 'Required'
//                    0: no dither enabled.
//                    1: enable dither and dither free go "when required".
//            bOutputMode -
//             D[1:0] - Color mode.
// Return: N/A
// Remark: reg72~reg8D will be programmed depended the input with table.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
SetCSCScale(BYTE bInputMode,BYTE bOutputMode)
{
    BYTE ucData,csc = 0;
    BYTE filter = 0 ; // filter is for Video CTRL DN_FREE_GO,EN_DITHER,and ENUDFILT


	// (1) YUV422 in,RGB/YUV444 output (Output is 8-bit,input is 12-bit)
	// (2) YUV444/422  in,RGB output (CSC enable,and output is not YUV422)
	// (3) RGB in,YUV444 output   (CSC enable,and output is not YUV422)
    //
    // YUV444/RGB24 <-> YUV422 need set up/down filter.

    switch(bInputMode&F_MODE_CLRMOD_MASK)
    {
    #ifdef SUPPORT_INPUTYUV444
    case F_MODE_YUV444:
        ErrorF("Input mode is YUV444 ") ;
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            ErrorF("Output mode is YUV444\n") ;
            csc = B_CSC_BYPASS ;
            break ;

        case F_MODE_YUV422:
            ErrorF("Output mode is YUV422\n") ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV444 to YUV422 need up/down filter for processing.
            {
                filter |= B_TX_EN_UDFILTER ;
            }
            csc = B_CSC_BYPASS ;
            break ;
        case F_MODE_RGB444:
            ErrorF("Output mode is RGB24\n") ;
            csc = B_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_DITHER) // YUV444 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTYUV422
    case F_MODE_YUV422:
        ErrorF("Input mode is YUV422\n") ;
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            ErrorF("Output mode is YUV444\n") ;
            csc = B_CSC_BYPASS ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to YUV444 need up filter
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        case F_MODE_YUV422:
            ErrorF("Output mode is YUV422\n") ;
            csc = B_CSC_BYPASS ;

            break ;

        case F_MODE_RGB444:
            ErrorF("Output mode is RGB24\n") ;
            csc = B_CSC_YUV2RGB ;
            if(bInputMode & F_MODE_EN_UDFILT) // YUV422 to RGB24 need up/dn filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // YUV422 to RGB24 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }

            break ;
        }
        break ;
    #endif

    #ifdef SUPPORT_INPUTRGB
    case F_MODE_RGB444:
        ErrorF("Input mode is RGB24\n") ;
        switch(bOutputMode&F_MODE_CLRMOD_MASK)
        {
        case F_MODE_YUV444:
            ErrorF("Output mode is YUV444\n") ;
            csc = B_CSC_RGB2YUV ;

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV444 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            break ;

        case F_MODE_YUV422:
            ErrorF("Output mode is YUV422\n") ;
            if(bInputMode & F_MODE_EN_UDFILT) // RGB24 to YUV422 need down filter.
            {
                filter |= B_TX_EN_UDFILTER ;
            }

            if(bInputMode & F_MODE_EN_DITHER) // RGB24 to YUV422 need dither
            {
                filter |= B_TX_EN_DITHER | B_TX_DNFREE_GO ;
            }
            csc = B_CSC_RGB2YUV ;
            break ;

        case F_MODE_RGB444:
            ErrorF("Output mode is RGB24\n") ;
            csc = B_CSC_BYPASS ;
            break ;
        }
        break ;
    #endif
    }

    #ifdef SUPPORT_INPUTRGB
    // set the CSC metrix registers by colorimetry and quantization
    if(csc == B_CSC_RGB2YUV)
    {
        ErrorF("CSC = RGB2YUV %x ",csc) ;
        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            ErrorF("ITU709 16-235 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            ErrorF("ITU709 0-255 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU709_0_255,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            ErrorF("ITU601 16-235 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            ErrorF("ITU601 0-255 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_RGB2YUV_ITU601_0_255,SIZEOF_CSCMTX) ;
            break ;
        }

    }
    #endif

    #ifdef SUPPORT_INPUTYUV
    if (csc == B_CSC_YUV2RGB)
    {
        ErrorF("CSC = YUV2RGB %x ",csc) ;

        switch(bInputMode&(F_MODE_ITU709|F_MODE_16_235))
        {
        case F_MODE_ITU709|F_MODE_16_235:
            ErrorF("ITU709 16-235 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU709|F_MODE_0_255:
            ErrorF("ITU709 0-255 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU709_0_255,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU601|F_MODE_16_235:
            ErrorF("ITU601 16-235 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_16_235,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_16_235,SIZEOF_CSCMTX) ;
            break ;
        case F_MODE_ITU601|F_MODE_0_255:
        default:
            ErrorF("ITU601 0-255 ") ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_YOFF,bCSCOffset_0_255,SIZEOF_CSCOFFSET) ;
            HDMITX_WriteI2C_ByteN(REG_TX_CSC_MTX11_L,bCSCMtx_YUV2RGB_ITU601_0_255,SIZEOF_CSCMTX) ;
            break ;
        }
    }
    #endif

    ucData = HDMITX_ReadI2C_Byte(REG_TX_CSC_CTRL) & ~(M_CSC_SEL|B_TX_DNFREE_GO|B_TX_EN_DITHER|B_TX_EN_UDFILTER) ;
    ucData |= filter|csc ;

    HDMITX_WriteI2C_Byte(REG_TX_CSC_CTRL,ucData) ;

    // set output Up/Down Filter,Dither control

}


//////////////////////////////////////////////////////////////////////
// Function: SetupAFE
// Parameter: VIDEOPCLKLEVEL level
//            PCLK_LOW - for 13.5MHz (for mode less than 1080p)
//            PCLK MEDIUM - for 25MHz~74MHz
//            PCLK HIGH - PCLK > 80Hz (for 1080p mode or above)
// Return: N/A
// Remark: set reg62~reg65 depended on HighFreqMode
//         reg61 have to be programmed at last and after video stable input.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
// SetupAFE(BYTE ucFreqInMHz)
SetupAFE(VIDEOPCLKLEVEL level)
{
	BYTE uc ;
	// @emily turn off reg61 before SetupAFE parameters.
	HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST) ;/* 0x10 */
	// HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,0x3) ;
    //ErrorF("SetupAFE()\n") ;

    //TMDS Clock < 80MHz	TMDS Clock > 80MHz
    //Reg61	0x03	0x03

    //Reg62	0x18	0x88
    //Reg63	Default	Default
    //Reg64	0x08	0x80
    //Reg65	Default	Default
    //Reg66	Default	Default
    //Reg67	Default	Default
	uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1) ;
	uc |= B_VDO_LATCH_EDGE ;
	HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc) ;
    switch(level)
    {
    case PCLK_HIGH:
        HDMITX_WriteI2C_Byte(REG_TX_AFE_XP_CTRL,0x88) ; // reg62
        HDMITX_WriteI2C_Byte(REG_TX_AFE_ISW_CTRL, 0x10) ; // reg63
        HDMITX_WriteI2C_Byte(REG_TX_AFE_IP_CTRL,0x84) ; // reg64
        break ;
    default:
        HDMITX_WriteI2C_Byte(REG_TX_AFE_XP_CTRL,0x18) ; // reg62
        HDMITX_WriteI2C_Byte(REG_TX_AFE_ISW_CTRL, 0x10) ; // reg63
        HDMITX_WriteI2C_Byte(REG_TX_AFE_IP_CTRL,0x0C) ; // reg64
        break ;
    }
    //HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_REF_RST|B_VID_RST|B_AREF_RST|B_HDMI_RST)) ;
    DelayMS(1) ;
    HDMITX_AndREG_Byte(REG_TX_SW_RST,B_VID_RST|B_AREF_RST|B_AUD_RST|B_HDCP_RST) ;
    DelayMS(100) ;
    HDMITX_AndREG_Byte(REG_TX_SW_RST,          B_AREF_RST|B_AUD_RST|B_HDCP_RST) ;
    // REG_TX_AFE_DRV_CTRL have to be set at the last step of setup .
   
}


//////////////////////////////////////////////////////////////////////
// Function: FireAFE
// Parameter: N/A
// Return: N/A
// Remark: write reg61 with 0x04
//         When program reg61 with 0x04,then audio and video circuit work.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////
static void
FireAFE(void)
{
    BYTE reg;
    Switch_HDMITX_Bank(0) ;

    HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,0) ;

    for(reg = 0x61 ; reg <= 0x67 ; reg++)
    {
        //ErrorF("Reg[%02X] = %02X\n",reg,HDMITX_ReadI2C_Byte(reg)) ;
    }
}

BOOL EnableVideoOutput(ULONG VCLK,BYTE inputColorMode,BYTE outputColorMode,BYTE bHDMI,BYTE ColorDepth)
{
    // bInputVideoMode,bOutputVideoMode,InitInstanceData.bInputVideoSignalType,bAudioInputType,should be configured by upper F/W or loaded from EEPROM.
    // should be configured by initsys.c
    WORD i ;
//    BYTE uc ;
    VIDEOPCLKLEVEL level ;

    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_VID_RST|B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;

    InitInstanceData.bHDMIMode = (BYTE)bHDMI ;
    // 2009/12/09 added by jau-chih.tseng@ite.com.tw
    Switch_HDMITX_Bank(1);
    HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1,0x00);
    Switch_HDMITX_Bank(0);
    //~jau-chih.tseng@ite.com.tw

    if(InitInstanceData.bHDMIMode)
    {
        _SetAVMute(TRUE) ;
    }

    SetInputMode(inputColorMode,InitInstanceData.bInputVideoSignalType) ;

    SetCSCScale(inputColorMode,outputColorMode) ;

    if(InitInstanceData.bHDMIMode)
    {
		ErrorF("color depth = %d\n", ColorDepth) ;
		BYTE temp = HDMITX_ReadI2C_Byte(REG_TX_GCP);
		temp = ((temp & (~B_COLOR_DEPTH_MASK)) | ((ColorDepth & M_COLOR_DEPTH) << O_COLOR_DEPTH));
		HDMITX_WriteI2C_Byte(REG_TX_GCP, temp);
        HDMITX_WriteI2C_Byte(REG_TX_HDMI_MODE,B_TX_HDMI_MODE) ;
    }
    else
    {
        HDMITX_WriteI2C_Byte(REG_TX_HDMI_MODE,B_TX_DVI_MODE) ;
    }

#ifdef INVERT_VID_LATCHEDGE
    uc = HDMITX_ReadI2C_Byte(REG_TX_CLK_CTRL1) ;
    uc |= B_VDO_LATCH_EDGE ;
    HDMITX_WriteI2C_Byte(REG_TX_CLK_CTRL1, uc) ;
#endif    

    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,          B_AUD_RST|B_AREF_RST|B_HDCP_RST) ;

    // if (pVTiming->VideoPixelClock>80000000)
    // {
    //     level = PCLK_HIGH ;
    // }
    // else if (pVTiming->VideoPixelClock>20000000)
    // {
    //     level = PCLK_MEDIUM ;
    // }
    // else
    // {
    //     level = PCLK_LOW ;
    // }

    if( VCLK>80000000 )
    {
        level = PCLK_HIGH ;
    }
    else if(VCLK>20000000)
    {
        level = PCLK_MEDIUM ;
    }
    else
    {
        level = PCLK_LOW ;
    }

    SetupAFE(level) ; // pass if High Freq request

    for(i = 0 ; i < 100 ; i++)
    {
        if(HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) & B_TXVIDSTABLE)
        {
            break ;

        }
        DelayMS(1) ;
    }
    // Clive suggestion.
    // clear int3 video stable interrupt.
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,B_CLR_VIDSTABLE) ;
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE) ;
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0) ;

    FireAFE() ;
    return TRUE ;
} /* EnableVideoOutput */

/* 
 * Info Frame
 */
//////////////////////////////////////////////////////////////////////
// Function: SetAVIInfoFrame()
// Parameter: pAVIInfoFrame - the pointer to HDMI AVI Infoframe ucData
// Return: N/A
// Remark: Fill the AVI InfoFrame ucData,and count checksum,then fill into
//         AVI InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAVIInfoFrame(AVI_InfoFrame *pAVIInfoFrame)
{
    int i ;
    byte ucData ;

    if(!pAVIInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1) ;
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB1,pAVIInfoFrame->pktbyte.AVI_DB[0]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB2,pAVIInfoFrame->pktbyte.AVI_DB[1]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB3,pAVIInfoFrame->pktbyte.AVI_DB[2]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB4,pAVIInfoFrame->pktbyte.AVI_DB[3]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB5,pAVIInfoFrame->pktbyte.AVI_DB[4]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB6,pAVIInfoFrame->pktbyte.AVI_DB[5]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB7,pAVIInfoFrame->pktbyte.AVI_DB[6]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB8,pAVIInfoFrame->pktbyte.AVI_DB[7]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB9,pAVIInfoFrame->pktbyte.AVI_DB[8]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB10,pAVIInfoFrame->pktbyte.AVI_DB[9]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB11,pAVIInfoFrame->pktbyte.AVI_DB[10]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB12,pAVIInfoFrame->pktbyte.AVI_DB[11]);
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_DB13,pAVIInfoFrame->pktbyte.AVI_DB[12]);
    for(i = 0,ucData = 0; i < 13 ; i++)
    {
        ucData -= pAVIInfoFrame->pktbyte.AVI_DB[i] ;
    }
	//ErrorF("SetAVIInfo(): ") ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB1)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB2)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB3)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB4)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB5)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB6)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB7)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB8)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB9)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB10)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB11)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB12)) ;
    ////ErrorF("%02X ",HDMITX_ReadI2C_Byte(REG_TX_AVIINFO_DB13)) ;
	//ErrorF("\n") ;
    ucData -= 0x80+AVI_INFOFRAME_VER+AVI_INFOFRAME_TYPE+AVI_INFOFRAME_LEN ;
	HDMITX_WriteI2C_Byte(REG_TX_AVIINFO_SUM,ucData);


    Switch_HDMITX_Bank(0) ;
    ENABLE_AVI_INFOFRM_PKT();
    return ER_SUCCESS ;
} /* SetAVIInfoFrame */

//////////////////////////////////////////////////////////////////////
// Function: SetAudioInfoFrame()
// Parameter: pAudioInfoFrame - the pointer to HDMI Audio Infoframe ucData
// Return: N/A
// Remark: Fill the Audio InfoFrame ucData,and count checksum,then fill into
//         Audio InfoFrame registers.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
SetAudioInfoFrame(Audio_InfoFrame *pAudioInfoFrame)
{
    int i ;
    BYTE ucData ;

    if(!pAudioInfoFrame)
    {
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(1) ;
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_CC,pAudioInfoFrame->pktbyte.AUD_DB[0]);
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_SF,pAudioInfoFrame->pktbyte.AUD_DB[1]);
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_CA,pAudioInfoFrame->pktbyte.AUD_DB[3]);
    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_DM_LSV,pAudioInfoFrame->pktbyte.AUD_DB[4]) ;

    for(i = 0,ucData = 0 ; i< 5 ; i++)
    {
        ucData -= pAudioInfoFrame->pktbyte.AUD_DB[i] ;
    }
    ucData -= 0x80+AUDIO_INFOFRAME_VER+AUDIO_INFOFRAME_TYPE+AUDIO_INFOFRAME_LEN ;

    HDMITX_WriteI2C_Byte(REG_TX_PKT_AUDINFO_SUM,ucData) ;


    Switch_HDMITX_Bank(0) ;
    ENABLE_AUD_INFOFRM_PKT();
    return ER_SUCCESS ;
} /* SetAudioInfoFrame */
 
BOOL
EnableAVIInfoFrame(BYTE bEnable,BYTE *pAVIInfoFrame)
{
    if(!bEnable)
    {
        DISABLE_AVI_INFOFRM_PKT() ;
        return TRUE ;
    }

    if(SetAVIInfoFrame((AVI_InfoFrame *)pAVIInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}

BOOL
EnableAudioInfoFrame(BYTE bEnable,BYTE *pAudioInfoFrame)
{
    if(!bEnable)
    {
        DISABLE_AVI_INFOFRM_PKT() ;
        return TRUE ;
    }


    if(SetAudioInfoFrame((Audio_InfoFrame *)pAudioInfoFrame) == ER_SUCCESS)
    {
        return TRUE ;
    }

    return FALSE ;
}
 
void
ConfigAVIInfoFrame(BYTE bOutputColorMode, BYTE VIC, BYTE pixelrep)
{
    AVI_InfoFrame AviInfo;

    ErrorF("ConfigAVIInfoFrame\n") ;

    AviInfo.pktbyte.AVI_HB[0] = AVI_INFOFRAME_TYPE|0x80 ; 
    AviInfo.pktbyte.AVI_HB[1] = AVI_INFOFRAME_VER ; 
    AviInfo.pktbyte.AVI_HB[2] = AVI_INFOFRAME_LEN ; 
    
    switch(bOutputColorMode)
    {
    case F_MODE_YUV444:
        // AviInfo.info.ColorMode = 2 ;
        AviInfo.pktbyte.AVI_DB[0] = (2<<5)|(1<<4) ;
        break ;
    case F_MODE_YUV422:
        // AviInfo.info.ColorMode = 1 ;
        AviInfo.pktbyte.AVI_DB[0] = (1<<5)|(1<<4) ;
        break ;
    case F_MODE_RGB444:
    default:
        // AviInfo.info.ColorMode = 0 ;
        AviInfo.pktbyte.AVI_DB[0] = (0<<5)|(1<<4) ;
        break ;
    }
    AviInfo.pktbyte.AVI_DB[1] = 8 ;
    AviInfo.pktbyte.AVI_DB[1] |= (aspec != HDMI_16x9)?(1<<4):(2<<4) ; // 4:3 or 16:9
    AviInfo.pktbyte.AVI_DB[1] |= (Colorimetry != HDMI_ITU709)?(1<<6):(2<<6) ; // 4:3 or 16:9
    AviInfo.pktbyte.AVI_DB[2] = 0 ;
    AviInfo.pktbyte.AVI_DB[3] = VIC ;
    AviInfo.pktbyte.AVI_DB[4] =  pixelrep & 3 ;
    AviInfo.pktbyte.AVI_DB[5] = 0 ;
    AviInfo.pktbyte.AVI_DB[6] = 0 ;
    AviInfo.pktbyte.AVI_DB[7] = 0 ;
    AviInfo.pktbyte.AVI_DB[8] = 0 ;
    AviInfo.pktbyte.AVI_DB[9] = 0 ;
    AviInfo.pktbyte.AVI_DB[10] = 0 ;
    AviInfo.pktbyte.AVI_DB[11] = 0 ;
    AviInfo.pktbyte.AVI_DB[12] = 0 ;

    EnableAVIInfoFrame(TRUE, (unsigned char *)&AviInfo) ;
} /* ConfigAVIInfoFrame */



////////////////////////////////////////////////////////////////////////////////
// Function: ConfigAudioInfoFrm
// Parameter: NumChannel, number from 1 to 8
// Return: ER_SUCCESS for successfull.
// Remark: Evaluate. The speakerplacement is only for reference.
//         For production, the caller of SetAudioInfoFrame should program
//         Speaker placement by actual status.
// Side-Effect:
////////////////////////////////////////////////////////////////////////////////

void
ConfigAudioInfoFrm(void)
{
    Audio_InfoFrame AudioInfo;	
    int i ;
    ErrorF("ConfigAudioInfoFrm(%d)\n",2) ;

    AudioInfo.pktbyte.AUD_HB[0] = AUDIO_INFOFRAME_TYPE ;
    AudioInfo.pktbyte.AUD_HB[1] = 1 ;
    AudioInfo.pktbyte.AUD_HB[2] = AUDIO_INFOFRAME_LEN ;
    AudioInfo.pktbyte.AUD_DB[0] = 1 ;
    for( i = 1 ;i < AUDIO_INFOFRAME_LEN ; i++ )
    {
        AudioInfo.pktbyte.AUD_DB[i] = 0 ;
    }
    EnableAudioInfoFrame(TRUE, (unsigned char *)&AudioInfo) ;
} /* ConfigAudioInfoFrm */

/*
 * DDC
 */
//////////////////////////////////////////////////////////////////////
// Function: ClearDDCFIFO
// Parameter: N/A
// Return: N/A
// Remark: clear the DDC FIFO.
// Side-Effect: DDC master will set to be HOST.
//////////////////////////////////////////////////////////////////////

static void
ClearDDCFIFO(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_FIFO_CLR) ;
}
 
//////////////////////////////////////////////////////////////////////
// Function: AbortDDC
// Parameter: N/A
// Return: N/A
// Remark: Force abort DDC and reset DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
AbortDDC(void)
{
    BYTE CPDesire,SWReset,DDCMaster ;
    BYTE uc, timeout ;
    // save the SW reset,DDC master,and CP Desire setting.
    SWReset = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) ;
    CPDesire = HDMITX_ReadI2C_Byte(REG_TX_HDCP_DESIRE) ;
    DDCMaster = HDMITX_ReadI2C_Byte(REG_TX_DDC_MASTER_CTRL) ;


    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,CPDesire&(~B_CPDESIRE)) ; // @emily change order
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,SWReset|B_HDCP_RST) ;		 // @emily change order
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_ABORT) ;

    for( timeout = 0 ; timeout < 200 ; timeout++ )
    {
        uc = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
        if (uc&B_DDC_DONE)
        {
            break ; // success
        }
        
        if( uc & (B_DDC_NOACK|B_DDC_WAITBUS|B_DDC_ARBILOSE) )
        {
            //ErrorF("AbortDDC Fail by reg16=%02X\n",uc) ;
            break ;
        }
        DelayMS(1) ; // delay 1 ms to stable.
    }

    // restore the SW reset,DDC master,and CP Desire setting.
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,SWReset) ;
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,CPDesire) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,DDCMaster) ;
}

static void
GenerateDDCSCLK(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_GEN_SCLCLK) ;
}
 
//////////////////////////////////////////////////////////////////////
// Function: ReadEDID
// Parameter: pData - the pointer of buffer to receive EDID ucdata.
//            bSegment - the segment of EDID readback.
//            offset - the offset of EDID ucdata in the segment. in byte.
//            count - the read back bytes count,cannot exceed 32
// Return: ER_SUCCESS if successfully getting EDID. ER_FAIL otherwise.
// Remark: function for read EDID ucdata from reciever.
// Side-Effect: DDC master will set to be HOST. DDC FIFO will be used and dirty.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
ReadEDID(BYTE *pData,BYTE bSegment,BYTE offset,SHORT Count)
{
    SHORT RemainedCount,ReqCount ;
    BYTE bCurrOffset ;
    SHORT TimeOut ;
    BYTE *pBuff = pData ;
    BYTE ucdata ;

    ErrorF("ReadEDID(%08lX,%d,%d,%d)\n",(ULONG)pData,bSegment,offset,Count) ;
    if(!pData)
    {
        ErrorF("ReadEDID(): Invallid pData pointer %08lX\n",(ULONG)pData) ;
        return ER_FAIL ;
    }

    if(HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG)
    {
    	ErrorF("Called AboutDDC()\n") ;
        AbortDDC() ;

    }

    ClearDDCFIFO() ;

    RemainedCount = Count ;
    bCurrOffset = offset ;

    Switch_HDMITX_Bank(0) ;

    while(RemainedCount > 0)
    {

        ReqCount = (RemainedCount > DDC_FIFO_MAXREQ)?DDC_FIFO_MAXREQ:RemainedCount ;
        ErrorF("ReadEDID(): ReqCount = %d,bCurrOffset = %d\n",ReqCount,bCurrOffset) ;

        HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
        HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_FIFO_CLR) ;

        for(TimeOut = 0 ; TimeOut < 200 ; TimeOut++)
    	{
		    ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
		    
		    if(ucdata&B_DDC_DONE)
		    {
		        break ;  
		    }
		    
		    if((ucdata & B_DDC_ERROR)||(HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) & B_INT_DDC_BUS_HANG))
		    {
		    	ErrorF("Called AboutDDC()\n") ;
		        AbortDDC() ;
				return ER_FAIL ;
		    }
    	}

        HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
        HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_EDID_ADDRESS) ; // for EDID ucdata get
        HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,bCurrOffset) ;
        HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,(BYTE)ReqCount) ;
        HDMITX_WriteI2C_Byte(REG_TX_DDC_EDIDSEG,bSegment) ;
        HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_EDID_READ) ;

        bCurrOffset += ReqCount ;
        RemainedCount -= ReqCount ;

        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            DelayMS(1) ;
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
            if(ucdata & B_DDC_DONE)
            {
                break ;
            }

            if(ucdata & B_DDC_ERROR)
            {
                ErrorF("ReadEDID(): DDC_STATUS = %02X,fail.\n",ucdata) ;
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
            ErrorF("ReadEDID(): DDC TimeOut. \n") ;
            return ER_FAIL ;
        }

        do
        {
            *(pBuff++) = HDMITX_ReadI2C_Byte(REG_TX_DDC_READFIFO) ;
            ReqCount -- ;
        }while(ReqCount > 0) ;

    }

    return ER_SUCCESS ;
}

#ifdef SUPPORT_HDCP
/*
 * HDCP
 */
static BYTE InitCAT6613_HDCPROM(void)
{
    BYTE uc[5]  ;
    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(0xF8,0xC3) ;
    HDMITX_WriteI2C_Byte(0xF8,0xA5) ;
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0x60) ;
    I2C_Read_ByteN(0xE0,0x00,uc,5) ;
    
    if(uc[0] == 1 &&
        uc[1] == 1 &&
        uc[2] == 1 &&
        uc[3] == 1 &&
        uc[4] == 1)
    {
        // with internal eMem
        HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xE0) ;
        HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0x48) ;
    }
    else
    {
        // with external ROM
        HDMITX_WriteI2C_Byte(REG_TX_ROM_HEADER,0xA0) ;
        HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0x00) ;
    }
    HDMITX_WriteI2C_Byte(0xF8,0xFF) ;
}

static void
HDCP_Reset(void)
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_SW_RST) | B_HDCP_RST ;
    HDMITX_WriteI2C_Byte(REG_TX_SW_RST,uc) ;
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST) ;
    ClearDDCFIFO() ;
    AbortDDC() ;
}

static void
HDCP_ClearAuthInterrupt(void)
{
    BYTE uc ;
    uc = HDMITX_ReadI2C_Byte(REG_TX_INT_MASK2) & (~(B_KSVLISTCHK_MASK|B_T_AUTH_DONE_MASK|B_AUTH_FAIL_MASK));
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL|B_CLR_AUTH_DONE|B_CLR_KSVLISTCHK) ;
    HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE) ;
}

static void
HDCP_ResetAuth(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0) ;
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,0) ;
    HDMITX_OrREG_Byte(REG_TX_SW_RST,B_HDCP_RST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDCP_ClearAuthInterrupt() ;
    AbortDDC() ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Authenticate_Repeater
// Parameter: BCaps and BStatus
// Return: ER_SUCCESS if success,if AUTH_FAIL interrupt status,return fail.
// Remark:
// Side-Effect: as Authentication
//////////////////////////////////////////////////////////////////////
static BYTE KSVList[32] ;
static BYTE Vr[20] ;
static BYTE M0[8] ;

static void
HDCP_CancelRepeaterAuthenticate(void)
{
    ErrorF("HDCP_CancelRepeaterAuthenticate") ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    AbortDDC() ;
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,B_LISTFAIL|B_LISTDONE) ;
    HDCP_ClearAuthInterrupt() ;
}

static void
HDCP_ResumeRepeaterAuthenticate(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,B_LISTDONE) ;
	HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHDCP) ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBCaps
// Parameter: pBCaps - pointer of byte to get BCaps.
//            pBStatus - pointer of two bytes to get BStatus
// Return: ER_SUCCESS if successfully got BCaps and BStatus.
// Remark: get B status and capability from HDCP reciever via DDC bus.
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBCaps(PBYTE pBCaps ,PUSHORT pBStatus)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x40) ; // BCaps offset
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,3) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1) ;

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
            ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            ErrorF("HDCP_GetBCaps(): DDC fail by reg16=%02X.\n",ucdata) ;
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN(REG_TX_BSTAT,(PBYTE)pBStatus,2) ;
    *pBCaps = HDMITX_ReadI2C_Byte(REG_TX_BCAP) ;
    return ER_SUCCESS ;

}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GetBKSV
// Parameter: pBKSV - pointer of 5 bytes buffer for getting BKSV
// Return: ER_SUCCESS if successfuly got BKSV from Rx.
// Remark: Get BKSV from HDCP reciever.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_GetBKSV(BYTE *pBKSV)
{
    BYTE ucdata ;
    BYTE TimeOut ;

    Switch_HDMITX_Bank(0) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,DDC_HDCP_ADDRESS) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x00) ; // BKSV offset
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,5) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;

    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        DelayMS(1) ;

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
            ErrorF("HDCP_GetBCaps(): DDC Done.\n") ;
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            ErrorF("HDCP_GetBCaps(): DDC No ack or arbilose,%x,maybe cable did not connected. Fail.\n",ucdata) ;
            return ER_FAIL ;
        }
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    HDMITX_ReadI2C_ByteN(REG_TX_BKSV,(PBYTE)pBKSV,5) ;

    return ER_SUCCESS ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_Auth_Fire()
// Parameter: N/A
// Return: N/A
// Remark: write anything to reg21 to enable HDCP authentication by HW
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_Auth_Fire(void)
{
    // //ErrorF("HDCP_Auth_Fire():\n") ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERDDC|B_MASTERHDCP) ; // MASTERHDCP,no need command but fire.
	HDMITX_WriteI2C_Byte(REG_TX_AUTHFIRE,1);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StartAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Start the Cipher to free run for random number. When stop,An is
//         ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StartAnCipher(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_START_CIPHER_GEN) ;
    DelayMS(1) ; // delay 1 ms
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_StopAnCipher
// Parameter: N/A
// Return: N/A
// Remark: Stop the Cipher,and An is ready in Reg30.
// Side-Effect: N/A
//////////////////////////////////////////////////////////////////////

static void
HDCP_StopAnCipher(void)
{
    HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN) ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_GenerateAn
// Parameter: N/A
// Return: N/A
// Remark: start An ciper random run at first,then stop it. Software can get
//         an in reg30~reg38,the write to reg28~2F
// Side-Effect:
//////////////////////////////////////////////////////////////////////

static void
HDCP_GenerateAn(void)
{
    BYTE Data[8] ;
#if 1
    HDCP_StartAnCipher() ;
    // HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_START_CIPHER_GEN) ;
    // DelayMS(1) ; // delay 1 ms
    // HDMITX_WriteI2C_Byte(REG_TX_AN_GENERATE,B_STOP_CIPHER_GEN) ;

    HDCP_StopAnCipher() ;

    Switch_HDMITX_Bank(0) ;
    // new An is ready in reg30
    HDMITX_ReadI2C_ByteN(REG_TX_AN_GEN,Data,8) ;
#else
	Data[0] = 0 ;Data[1] = 0 ;Data[2] = 0 ;Data[3] = 0 ;
	Data[4] = 0 ;Data[5] = 0 ;Data[6] = 0 ;Data[7] = 0 ;
#endif
    HDMITX_WriteI2C_ByteN(REG_TX_AN,Data,8) ;

}

static SYS_STATUS
HDCP_GetKSVList(BYTE *pKSVList,BYTE cDownStream)
{
    BYTE TimeOut = 100 ;
	BYTE ucdata ;
	
	if(cDownStream == 0 || pKSVList == NULL)
	{
	    return ER_FAIL ;
	}
	
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,0x74) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x43) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,cDownStream * 5) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {

        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
            ErrorF("HDCP_GetKSVList(): DDC Done.\n") ;
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            ErrorF("HDCP_GetKSVList(): DDC Fail by REG_TX_DDC_STATUS = %x.\n",ucdata) ;
            return ER_FAIL ;
        }
        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
        return ER_FAIL ;
    }

    ErrorF("HDCP_GetKSVList(): KSV") ;
    for(TimeOut = 0 ; TimeOut < cDownStream * 5 ; TimeOut++)
    {
        pKSVList[TimeOut] = HDMITX_ReadI2C_Byte(REG_TX_DDC_READFIFO) ;
        ErrorF(" %02X",pKSVList[TimeOut]) ;
    }
    ErrorF("\n") ;
	return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetVr(BYTE *pVr)
{
    BYTE TimeOut  ;
	BYTE ucdata ;
	
	if(pVr == NULL)
	{
	    return ER_FAIL ;
	}
	
    HDMITX_WriteI2C_Byte(REG_TX_DDC_MASTER_CTRL,B_MASTERHOST) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_HEADER,0x74) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQOFF,0x20) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_REQCOUNT,20) ;
    HDMITX_WriteI2C_Byte(REG_TX_DDC_CMD,CMD_DDC_SEQ_BURSTREAD) ;


    for(TimeOut = 200 ; TimeOut > 0 ; TimeOut --)
    {
        ucdata = HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS) ;
        if(ucdata & B_DDC_DONE)
        {
            //ErrorF("HDCP_GetVr(): DDC Done.\n") ;
            break ;
        }

        if(ucdata & B_DDC_ERROR)
        {
            //ErrorF("HDCP_GetVr(): DDC fail by REG_TX_DDC_STATUS = %x.\n",ucdata) ;
            return ER_FAIL ;
        }
        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
        //ErrorF("HDCP_GetVr(): DDC fail by timeout.\n",ucdata) ;
        return ER_FAIL ;
    }

    Switch_HDMITX_Bank(0) ;

    for(TimeOut = 0 ; TimeOut < 5 ; TimeOut++)
    {
        HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL ,TimeOut) ;
        pVr[TimeOut*4+3]  = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE1) ;
        pVr[TimeOut*4+2] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE2) ;
        pVr[TimeOut*4+1] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE3) ;
        pVr[TimeOut*4] = (ULONG)HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE4) ;
		//ErrorF("V' = %02X %02X %02X %02X\n",pVr[TimeOut*4],pVr[TimeOut*4+1],pVr[TimeOut*4+2],pVr[TimeOut*4+3]) ; 
    }
    
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_GetM0(BYTE *pM0)
{
	int i ;

    if(!pM0)
    {
        return ER_FAIL ;
    }
    
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,5) ; // read m0[31:0] from reg51~reg54
    pM0[0] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE1) ;
    pM0[1] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE2) ;
    pM0[2] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE3) ;
    pM0[3] = HDMITX_ReadI2C_Byte(REG_TX_SHA_RD_BYTE4) ;
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,0) ; // read m0[39:32] from reg55
    pM0[4] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,1) ; // read m0[47:40] from reg55
    pM0[5] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,2) ; // read m0[55:48] from reg55
    pM0[6] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5) ;
    HDMITX_WriteI2C_Byte(REG_TX_SHA_SEL,3) ; // read m0[63:56] from reg55
    pM0[7] = HDMITX_ReadI2C_Byte(REG_TX_AKSV_RD_BYTE5) ;

    //ErrorF("M[] =") ;
	for(i = 0 ; i < 8 ; i++){
		//ErrorF("0x%02x,",pM0[i]) ;
	}
	//ErrorF("\n") ;
    return ER_SUCCESS ;
}

static BYTE SHABuff[64] ;
static BYTE V[20] ;

static ULONG w[80];
static ULONG sha[5] ;

#define rol(x,y) (((x) << (y)) | (((ULONG)x) >> (32-y)))

void SHATransform(ULONG * h)
{
	LONG t;

      
	for (t = 16; t < 80; t++) {
		ULONG tmp = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
		w[t] = rol(tmp,1);
		////printf("w[%2d] = %08lX\n",t,w[t]) ;
	}

	h[0] = 0x67452301 ;
	h[1] = 0xefcdab89;
	h[2] = 0x98badcfe;
	h[3] = 0x10325476;
	h[4] = 0xc3d2e1f0;

	for (t = 0; t < 20; t++) {
		ULONG tmp =
			rol(h[0],5) + ((h[1] & h[2]) | (h[3] & ~h[1])) + h[4] + w[t] + 0x5a827999;
		////printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;

		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
		
	}
	for (t = 20; t < 40; t++) {
		ULONG tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0x6ed9eba1;
		////printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	for (t = 40; t < 60; t++) {
		ULONG tmp = rol(h[0],
						 5) + ((h[1] & h[2]) | (h[1] & h[3]) | (h[2] & h[3])) + h[4] + w[t] +
			0x8f1bbcdc;
		//printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	for (t = 60; t < 80; t++) {
		ULONG tmp = rol(h[0],5) + (h[1] ^ h[2] ^ h[3]) + h[4] + w[t] + 0xca62c1d6;
		//printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
		h[4] = h[3];
		h[3] = h[2];
		h[2] = rol(h[1],30);
		h[1] = h[0];
		h[0] = tmp;
	}
	//printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;

	h[0] += 0x67452301 ;
	h[1] += 0xefcdab89;
	h[2] += 0x98badcfe;
	h[3] += 0x10325476;
	h[4] += 0xc3d2e1f0;
	//printf("%08lX %08lX %08lX %08lX %08lX\n",h[0],h[1],h[2],h[3],h[4]) ;
}

/* ----------------------------------------------------------------------
 * Outer SHA algorithm: take an arbitrary length byte string,
 * convert it into 16-word blocks with the prescribed padding at
 * the end,and pass those blocks to the core SHA algorithm.
 */


void SHA_Simple(void *p,LONG len,BYTE *output)
{
	// SHA_State s;
    int i, t ;
    ULONG c ;
    char *pBuff = p ;
    
    
    for( i = 0 ; i < len ; i++ )
    {
        t = i/4 ;
        if( i%4 == 0 )
        {
            w[t] = 0 ;
        }
        c = pBuff[i]&0xFF ;
        c <<= (3-(i%4))*8 ;
        w[t] |= c ;
        //printf("pBuff[%d] = %02x, c = %08lX, w[%d] = %08lX\n",i,pBuff[i],c,t,w[t]) ;
    }
    t = i/4 ;
    if( i%4 == 0 )
    {
        w[t] = 0 ;
    }
    c = 0x80 << ((3-i%4)*24) ;
    w[t]|= c ; t++ ;
    for( ; t < 15 ; t++ )
    {
        w[t] = 0 ;
    }
    w[15] = len*8  ;
    
    for( t = 0 ; t< 16 ; t++ )
    {
        //printf("w[%2d] = %08lX\n",t,w[t]) ;
    }

    SHATransform(sha) ;
    
    for( i = 0 ; i < 5 ; i++ )
    {
        output[i*4]   = (BYTE)((sha[i]>>24)&0xFF) ;
        output[i*4+1] = (BYTE)((sha[i]>>16)&0xFF) ;
        output[i*4+2] = (BYTE)((sha[i]>>8)&0xFF) ;
        output[i*4+3] = (BYTE)(sha[i]&0xFF) ;
    }
}

static SYS_STATUS
HDCP_CheckSHA(BYTE pM0[],USHORT BStatus,BYTE pKSVList[],int cDownStream,BYTE Vr[])
{
    int i,n ;
    
    for(i = 0 ; i < cDownStream*5 ; i++)
    {
        SHABuff[i] = pKSVList[i] ;
    }
    SHABuff[i++] = BStatus & 0xFF ;
    SHABuff[i++] = (BStatus>>8) & 0xFF ;
    for(n = 0 ; n < 8 ; n++,i++)
    {
        SHABuff[i] = pM0[n] ;
    }
    n = i ;
    // SHABuff[i++] = 0x80 ; // end mask
    for(; i < 64 ; i++)
    {
        SHABuff[i] = 0 ;
    }
    // n = cDownStream * 5 + 2 /* for BStatus */ + 8 /* for M0 */ ;
    // n *= 8 ;
    // SHABuff[62] = (n>>8) & 0xff ;
    // SHABuff[63] = (n>>8) & 0xff ;
    for(i = 0 ; i < 64 ; i++)
	{
		//if(i % 16 == 0) printf("SHA[]: ") ;
		//printf(" %02X",SHABuff[i]) ;
		//if((i%16)==15) printf("\n") ;
	}
    SHA_Simple(SHABuff,n,V) ;
    //printf("V[] =") ;
    for(i = 0 ; i < 20 ; i++)
    {
        //printf(" %02X",V[i]) ;
    }
    //printf("\nVr[] =") ;
    for(i = 0 ; i < 20 ; i++)
    {
        //printf(" %02X",Vr[i]) ;
    }
        
    for(i = 0 ; i < 20 ; i++)
    {
        if(V[i] != Vr[i])
        {
            return ER_FAIL ;
        }
    }
    return ER_SUCCESS ;
}

static SYS_STATUS
HDCP_Authenticate_Repeater(void)
{
    BYTE uc ;
    #ifdef SUPPORT_DSSSHA    
    BYTE revoked ;
    int i ;
    #endif // _DSS_SHA_
	// BYTE test;
	// BYTE test06;
	// BYTE test07;
	// BYTE test08;
    BYTE cDownStream ;

    BYTE BCaps;
    USHORT BStatus ;
    USHORT TimeOut ;

	//ErrorF("Authentication for repeater\n") ;
    // emily add for test,abort HDCP
    // 2007/10/01 marked by jj_tseng@chipadvanced.com
    // HDMITX_WriteI2C_Byte(0x20,0x00) ;
    // HDMITX_WriteI2C_Byte(0x04,0x01) ;
	// HDMITX_WriteI2C_Byte(0x10,0x01) ;
	// HDMITX_WriteI2C_Byte(0x15,0x0F) ;
	// DelayMS(100);
    // HDMITX_WriteI2C_Byte(0x04,0x00) ;
	// HDMITX_WriteI2C_Byte(0x10,0x00) ;
	// HDMITX_WriteI2C_Byte(0x20,0x01) ;
	// DelayMS(100);
	// test07 = HDMITX_ReadI2C_Byte(0x7)  ;
	// test06 = HDMITX_ReadI2C_Byte(0x6);
	// test08 = HDMITX_ReadI2C_Byte(0x8);
	//~jj_tseng@chipadvanced.com
	// end emily add for test
    //////////////////////////////////////
    // Authenticate Fired
    //////////////////////////////////////
  
    HDCP_GetBCaps(&BCaps,&BStatus) ;
	DelayMS(2);
    HDCP_Auth_Fire();
	DelayMS(550); // emily add for test

    for(TimeOut = 250*6 ; TimeOut > 0 ; TimeOut --)
    {

        uc = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) ;
        if(uc & B_INT_DDC_BUS_HANG)
        {
            ErrorF("DDC Bus hang\n") ;
            goto HDCP_Repeater_Fail ;
        }

        uc = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2) ;

        if(uc & B_INT_AUTH_FAIL)
        {
			/*
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL) ;
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0) ;
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE) ;
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0) ;
			*/
            ErrorF("HDCP_Authenticate_Repeater(): B_INT_AUTH_FAIL.\n") ;
            goto HDCP_Repeater_Fail ;
        }
        // emily add for test
		// test =(HDMITX_ReadI2C_Byte(0x7)&0x4)>>2 ;
        if(uc & B_INT_KSVLIST_CHK)
        {
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_KSVLISTCHK) ;
            HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0) ;
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE) ;
            HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0) ;
            //ErrorF("B_INT_KSVLIST_CHK\n") ;
            break ;
        }

        DelayMS(5) ;
    }

    if(TimeOut == 0)
    {
        ErrorF("Time out for wait KSV List checking interrupt\n") ;
        goto HDCP_Repeater_Fail ;
    }

    ///////////////////////////////////////
    // clear KSVList check interrupt.
    ///////////////////////////////////////

    for(TimeOut = 500 ; TimeOut > 0 ; TimeOut --)
    {
		if((TimeOut % 100) == 0)
		{
		    ErrorF("Wait KSV FIFO Ready %d\n",TimeOut) ;
		}
		
        if(HDCP_GetBCaps(&BCaps,&BStatus) == ER_FAIL)
        {
            //ErrorF("Get BCaps fail\n") ;
            goto HDCP_Repeater_Fail ;
        }

        if(BCaps & B_CAP_KSV_FIFO_RDY)
        {
			 //ErrorF("FIFO Ready\n") ;
			 break ;
        }
        DelayMS(5) ;

    }

    if(TimeOut == 0)
    {
        ErrorF("Get KSV FIFO ready TimeOut\n") ;
        goto HDCP_Repeater_Fail ;
    }

	//ErrorF("Wait timeout = %d\n",TimeOut) ;
	
    ClearDDCFIFO() ;
    GenerateDDCSCLK() ;
    cDownStream =  (BStatus & M_DOWNSTREAM_COUNT) ;
    
    if(cDownStream == 0 || cDownStream > 6 || BStatus & (B_MAX_CASCADE_EXCEEDED|B_DOWNSTREAM_OVER))
    {
        //ErrorF("Invalid Down stream count,fail\n") ;
        goto HDCP_Repeater_Fail ;
    }
    

    if(HDCP_GetKSVList(KSVList,cDownStream) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }

#if 0
    for(i = 0 ; i < cDownStream ; i++)
    {
		revoked=FALSE ; uc = 0 ;
		for( TimeOut = 0 ; TimeOut < 5 ; TimeOut++ )
		{
		    // check bit count
		    uc += countbit(KSVList[i*5+TimeOut]) ;
		}
		if( uc != 20 ) revoked = TRUE ;
    #ifdef SUPPORT_REVOKE_KSV    
        HDCP_VerifyRevocationList(SRM1,&KSVList[i*5],&revoked) ;
    #endif
        if(revoked)
        {
            //ErrorF("KSVFIFO[%d] = %02X %02X %02X %02X %02X is revoked\n",i,KSVList[i*5],KSVList[i*5+1],KSVList[i*5+2],KSVList[i*5+3],KSVList[i*5+4]) ; 
			 goto HDCP_Repeater_Fail ;
        }
    }
#endif

    
    if(HDCP_GetVr(Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ; 
    }

    if(HDCP_GetM0(M0) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }
    
    // do check SHA
    if(HDCP_CheckSHA(M0,BStatus,KSVList,cDownStream,Vr) == ER_FAIL)
    {
        goto HDCP_Repeater_Fail ;
    }
    
    
    HDCP_ResumeRepeaterAuthenticate() ;
	InitInstanceData.bAuthenticated = TRUE ;
    return ER_SUCCESS ;
    
HDCP_Repeater_Fail:
    HDCP_CancelRepeaterAuthenticate() ;
    return ER_FAIL ;
}

static BYTE
countbit(BYTE b)
{
    BYTE i,count ;
    for( i = 0, count = 0 ; i < 8 ; i++ )
    {
        if( b & (1<<i) )
        {
            count++ ;
        }
    }
    return count ;
}
 
static SYS_STATUS
HDCP_Authenticate(void)
{
    BYTE ucdata ;
    BYTE BCaps ;
    USHORT BStatus ;
    USHORT TimeOut ;
    #ifdef SUPPORT_REVOKE_KSV
    BYTE revoked = FALSE ;
    #endif
    BYTE BKSV[5] ;

    InitInstanceData.bAuthenticated = FALSE ;

    // Authenticate should be called after AFE setup up.

    ErrorF("HDCP_Authenticate():\n") ;
	HDCP_Reset() ;
    // ClearDDCFIFO() ;
    // AbortDDC() ;

    if(HDCP_GetBCaps(&BCaps,&BStatus) != ER_SUCCESS)
    {
        ErrorF("HDCP_GetBCaps fail.\n") ;
        return ER_FAIL ;
    }


	if(InitInstanceData.bHDMIMode)
	{
		if((BStatus & B_CAP_HDMI_MODE)==0)
		{
			ErrorF("Not a HDMI mode,do not authenticate and encryption. BCaps = %x BStatus = %x\n",BCaps,BStatus) ;
			return ER_FAIL ;
		}
	}

	//ErrorF("BCaps = %02X BStatus = %04X\n",BCaps,BStatus) ;
    /*
    if((BStatus & M_DOWNSTREAM_COUNT)> 6)
    {
        //ErrorF("Down Stream Count %d is over maximum supported number 6,fail.\n",(BStatus & M_DOWNSTREAM_COUNT)) ;
        return ER_FAIL ;
    }
    */

    HDCP_GetBKSV(BKSV) ;
	//ErrorF("BKSV %02X %02X %02X %02X %02X\n",BKSV[0],BKSV[1],BKSV[2],BKSV[3],BKSV[4]) ;
	
	for(TimeOut = 0, ucdata = 0 ; TimeOut < 5 ; TimeOut ++)
	{
	    ucdata += countbit(BKSV[TimeOut]) ;
	}
	if( ucdata != 20 ) return ER_FAIL ;
	
	
    #ifdef SUPPORT_REVOKE_KSV
    HDCP_VerifyRevocationList(SRM1,BKSV,&revoked) ;
    if(revoked)
    {
        ErrorF("BKSV is revoked\n") ; return ER_FAIL ;
    }
    //ErrorF("BKSV %02X %02X %02X %02X %02X is NOT %srevoked\n",BKSV[0],BKSV[1],BKSV[2],BKSV[3],BKSV[4],revoked?"not ":"") ;
    #endif // SUPPORT_DSSSHA

    Switch_HDMITX_Bank(0) ; // switch bank action should start on direct register writting of each function.

    // 2006/08/11 added by jjtseng
    // enable HDCP on CPDired enabled.
    HDMITX_AndREG_Byte(REG_TX_SW_RST,~(B_HDCP_RST)) ;
    //~jjtseng 2006/08/11

//    if(BCaps & B_CAP_HDCP_1p1)
//    {
//        //ErrorF("RX support HDCP 1.1\n") ;
//        HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,B_ENABLE_HDPC11|B_CPDESIRE) ;
//    }
//    else
//    {
//        //ErrorF("RX not support HDCP 1.1\n") ;
    HDMITX_WriteI2C_Byte(REG_TX_HDCP_DESIRE,B_CPDESIRE) ;
//    }


    // HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_DONE|B_CLR_AUTH_FAIL|B_CLR_KSVLISTCHK) ;
    // HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0) ; // don't clear other settings.
    // ucdata = HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) ;
    // ucdata = (ucdata & M_CTSINTSTEP) | B_INTACTDONE ;
    // HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,ucdata) ; // clear action.

    // HDMITX_AndREG_Byte(REG_TX_INT_MASK2,~(B_AUTH_FAIL_MASK|B_T_AUTH_DONE_MASK)) ;    // enable GetBCaps Interrupt
    HDCP_ClearAuthInterrupt() ;
    //ErrorF("int2 = %02X DDC_Status = %02X\n",HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2),HDMITX_ReadI2C_Byte(REG_TX_DDC_STATUS)) ;


    HDCP_GenerateAn() ;
    HDMITX_WriteI2C_Byte(REG_TX_LISTCTRL,0) ;
    InitInstanceData.bAuthenticated = FALSE ;

    if((BCaps & B_CAP_HDMI_REPEATER) == 0)
    {
        HDCP_Auth_Fire();
        // wait for status ;
        
        for(TimeOut = 250 ; TimeOut > 0 ; TimeOut --)
        {
            DelayMS(5) ; // delay 1ms
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_AUTH_STAT) ;
            //ErrorF("reg46 = %02x reg16 = %02x\n",ucdata,HDMITX_ReadI2C_Byte(0x16)) ;

            if(ucdata & B_T_AUTH_DONE)
            {
                InitInstanceData.bAuthenticated = TRUE ;
                break ;
            }
            
            ucdata = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2) ;
            if(ucdata & B_INT_AUTH_FAIL)
            {
                /*
                HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,B_CLR_AUTH_FAIL) ;
                HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0) ;
                HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,B_INTACTDONE) ;
                HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,0) ;
                */
                ErrorF("HDCP_Authenticate(): Authenticate fail\n") ;
                InitInstanceData.bAuthenticated = FALSE ;
                return ER_FAIL ;
            }
        }

        if(TimeOut == 0)
        {
             ErrorF("HDCP_Authenticate(): Time out. return fail\n") ;
             InitInstanceData.bAuthenticated = FALSE ;
             return ER_FAIL ;
        }
        return ER_SUCCESS ;
    }

    return HDCP_Authenticate_Repeater() ;
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_EnableEncryption
// Parameter: N/A
// Return: ER_SUCCESS if done.
// Remark: Set regC1 as zero to enable continue authentication.
// Side-Effect: register bank will reset to zero.
//////////////////////////////////////////////////////////////////////

static SYS_STATUS
HDCP_EnableEncryption(void)
{
    Switch_HDMITX_Bank(0) ;
	return HDMITX_WriteI2C_Byte(REG_TX_ENCRYPTION,B_ENABLE_ENCRYPTION);
}

//////////////////////////////////////////////////////////////////////
// Function: HDCP_ResumeAuthentication
// Parameter: N/A
// Return: N/A
// Remark: called by interrupt handler to restart Authentication and Encryption.
// Side-Effect: as Authentication and Encryption.
//////////////////////////////////////////////////////////////////////

static void
HDCP_ResumeAuthentication(void)
{
    _SetAVMute(TRUE) ;
    if(HDCP_Authenticate() == ER_SUCCESS)
	{
		HDCP_EnableEncryption() ;
	}
	_SetAVMute(FALSE) ;
}

BOOL
EnableHDCP(BYTE bEnable)
{
    if(bEnable)
    {
        if(ER_FAIL == HDCP_Authenticate())
        {
			
            HDCP_ResetAuth() ;    
			return FALSE ;
        }

    }
    else
    {
        HDCP_ResetAuth() ;    
    }
    return TRUE ;
}
#endif /* SUPPORT_HDCP */
  
/*
 * Interrupt
 */
BOOL
CheckHDMITX(void)
{
    BYTE intdata1,intdata2,intdata3,sysstat;
    BYTE  intclr3 = 0 ;
    //BOOL PrevHPD = InitInstanceData.bHPD ;
    BOOL HPD ;

    sysstat = HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) ;

    //HPD = ((sysstat & (B_HPDETECT|B_RXSENDETECT)) == (B_HPDETECT|B_RXSENDETECT))?TRUE:FALSE ;
    HPD = (sysstat & B_HPDETECT) ? TRUE:FALSE ;

#if 0
    // 2007/06/20 added by jj_tseng@chipadvanced.com
    if(pHPDChange)
    {
    	*pHPDChange = FALSE ;

    }
    //~jj_tseng@chipadvanced.com 2007/06/20
#endif

    if(!HPD)
    {
        InitInstanceData.bAuthenticated = FALSE ;
    }

    if(sysstat & B_INT_ACTIVE)
    {

        intdata1 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT1) ;
        ErrorF("INT_Handler: reg%02x = %02x\n",REG_TX_INT_STAT1,intdata1) ;

	if(intdata1 & B_INT_DDCFIFO_ERR)
	{
	    ErrorF("DDC FIFO Error.\n") ;
	    ClearDDCFIFO() ;
	}


	if(intdata1 & B_INT_DDC_BUS_HANG)
	{
	    ErrorF("DDC BUS HANG.\n") ;
            AbortDDC() ;

            if(InitInstanceData.bAuthenticated)
            {
                ErrorF("when DDC hang,and aborted DDC,the HDCP authentication need to restart.\n") ;
                HDCP_ResumeAuthentication() ;
            }
	}


	if(intdata1 & (B_INT_HPD_PLUG|B_INT_RX_SENSE))
	{

            //if(pHPDChange) *pHPDChange = TRUE ;

            if(HPD)
            {
                ErrorF("plug \n");
                //HDMITX_Reconfig();
                crt_hotplug_tx(1);
            }	
            else
            {
                HDMITX_WriteI2C_Byte(REG_TX_SW_RST,B_AREF_RST|B_VID_RST|B_AUD_RST|B_HDCP_RST) ;
                DelayMS(1) ;
                HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;
                ErrorF("Unplug,%x %x\n",HDMITX_ReadI2C_Byte(REG_TX_SW_RST),HDMITX_ReadI2C_Byte(REG_TX_AFE_DRV_CTRL)) ;
                crt_hotplug_tx(0);
                // VState = TXVSTATE_Unplug ;
            }
	}


        intdata2 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT2) ;
        ErrorF("INT_Handler: reg%02x = %02x\n",REG_TX_INT_STAT2,intdata2) ;



#ifdef SUPPORT_HDCP
	if(intdata2 & B_INT_AUTH_DONE)
	{
            ErrorF("interrupt Authenticate Done.\n") ;
            HDMITX_OrREG_Byte(REG_TX_INT_MASK2,(BYTE)B_T_AUTH_DONE_MASK) ;
            InitInstanceData.bAuthenticated = TRUE ;
            _SetAVMute(FALSE) ;
	}

	if(intdata2 & B_INT_AUTH_FAIL)
	{
            ErrorF("interrupt Authenticate Fail.\n") ;
	    AbortDDC();   // @emily add
            HDCP_ResumeAuthentication() ;
	}
#endif // SUPPORT_HDCP

	intdata3 = HDMITX_ReadI2C_Byte(REG_TX_INT_STAT3) ;
        ErrorF("INT_Handler: reg%02x = %02x\n",REG_TX_INT_STAT3,intdata3) ;
	
	if(intdata3 & B_INT_VIDSTABLE)
	{
		sysstat = HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS) ;
		if(sysstat & B_TXVIDSTABLE)
		{
			FireAFE() ;
		}
	}
        HDMITX_WriteI2C_Byte(REG_TX_INT_CLR0,0xFF) ;
        HDMITX_WriteI2C_Byte(REG_TX_INT_CLR1,0xFF) ;
        intclr3 = (HDMITX_ReadI2C_Byte(REG_TX_SYS_STATUS))|B_CLR_AUD_CTS | B_INTACTDONE ;
        HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr3) ; // clear interrupt.
        intclr3 &= ~(B_INTACTDONE) ;
        HDMITX_WriteI2C_Byte(REG_TX_SYS_STATUS,intclr3) ; // INTACTDONE reset to zero.
    }
#if 0    
    else
    {
        if(pHPDChange)
        {
            *pHPDChange = (HPD != PrevHPD)?TRUE:FALSE ;

            if(*pHPDChange &&(!HPD))
            {
                HDMITX_WriteI2C_Byte(REG_TX_AFE_DRV_CTRL,B_AFE_DRV_RST|B_AFE_DRV_PWD) ;
            }
        }
    }
#endif

    SetupAudioChannel() ; // 2007/12/12 added by jj_tseng

#if 0    
    if(pHPD)
    {
        *pHPD = HPD ? TRUE:FALSE ;
    }
#endif

    InitInstanceData.bHPD  = (BYTE)HPD ;
    return TRUE ;
} /* CheckHDMITX */
 
