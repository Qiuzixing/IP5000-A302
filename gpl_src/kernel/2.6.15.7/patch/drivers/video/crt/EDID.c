#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>    /* size_t */
#include <linux/string.h>    /* memset */
#include <linux/delay.h>    /* msleep */
#include "astdebug.h"
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/drivers/I2C.h>
#include <asm/arch/drivers/edid.h>
#include <asm/arch/drivers/crt.h>

/* 0: the same, 1:different */
u32 CmpEdid(unsigned char *pEdid1, unsigned char *pEdid2)
{
	int i;

	for (i = 0; i < EDID_ROM_SIZE; i++) {
		if (pEdid1[i] != pEdid2[i])
			return 1;
	}

	return 0;
}
EXPORT_SYMBOL(CmpEdid);

/* check if edid has CEA
 * 1 : have
 * 0 : no
 */
unsigned int EdidHasCEA(unsigned char *pEdid)
{
	unsigned char btExtTag, btRevisonNum;
	unsigned int bCEA = 0;

	btExtTag        = pEdid[EXT_TAG];
	btRevisonNum    = pEdid[CEA_VERSION];

	if ((btExtTag == 2)
#if 0
		&& (btRevisonNum == 3)
#endif
		) {
		bCEA = 1;
	}

	return bCEA;
}

void removeDupBlock(u8 *pbuf)
{
	int i;
	int dup = 1;
	u8 edidheader[8] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};
	
	for (i = 0; i < 8; i++){
		if (*(pbuf + i) != edidheader[i]){
			dup = 0;
			break;
		}
	}

	if (dup){
		uinfo("zero out duplicate block\n");
		memset(pbuf, 0, 128);
	}
}

/*
** return 0 if OK, otherwise non-zero
** EDID is checksum is per block basis.
** Each block(bank) is 128 bytes long and
** the checksum value is the last byte.
*/
/*inline */u32 verify_edid_block_checksum(u8 *pbuf)
{
	u8 btTemp = 0;
	u32 i;
	u32 zero_cnt = 0;
	
	for (i = 0; i < EDID_BLOCK_SIZE; i++) {
		btTemp += pbuf[i];
		if (pbuf[i] == 0)
			zero_cnt++;
	}

	if (zero_cnt == EDID_BLOCK_SIZE)
		return 0xFF;
	
	return btTemp;
}
EXPORT_SYMBOL(verify_edid_block_checksum);

unsigned int read_i2c_byte(unsigned int ulDeviceSel, unsigned int addr, int offset, unsigned char *pBuffer)
{
	int iCount = 0;

	while (GetI2CReg(ulDeviceSel, addr, offset, pBuffer)) {
		if (iCount > 3) {
			uerr("read EEPROM fail\n");
			return -ENODEV;
		}
	
		iCount++;
		msleep(10);
	}
	return 0;
}

int hostReadEepromEdid(unsigned char *pEdid, unsigned int is_port_b)
{
	int i;
	unsigned char bTemp;
	unsigned int ulDeviceSel = I2C_HOST_VIDEO_DDC_EEPROM;
	unsigned int addr = I2C_DDC_EDID_ADDR;

#if (BOARD_DESIGN_VER_VIDEO >= 300)
	if (is_port_b)
		ulDeviceSel = I2C_HOST_VIDEO_DDC_EEPROM_B;
#endif

	/* The EEPROM is on board and should be already ready. */
	if (WaitI2CRdy(ulDeviceSel, addr, 500)) {
		uerr("I2C is not ready\n");
		return -ENODEV;
	}
    
	for (i = 0; i < EDID_ROM_SIZE; i++) {
		//pEdid[i] = GetI2CReg(ulDeviceSel, 0xA0, i);
		if (read_i2c_byte(ulDeviceSel, addr, i, &bTemp)) {
			uerr("GetI2CReg Fail\n");
			return -ENODEV;
		}
			
		pEdid[i] = bTemp;
		//uinfo("read [%x]/[%x]\n", i, pEdid[i]);
		/*
		** Bruce141202 EEPROM write is slow and we should yield CPU here to
		** avoid video engine sw_watchdog time up.
		*/
		if (!(i & 0xF))
			yield();

	}
	return 0;
}
EXPORT_SYMBOL(hostReadEepromEdid);

int hostWriteEepromEdid(unsigned char *pEdid, unsigned int is_port_b)
{
	int i;
	unsigned int ulDeviceSel = I2C_HOST_VIDEO_DDC_EEPROM;
	unsigned int addr = I2C_DDC_EDID_ADDR;
	unsigned char arEdid[EDID_ROM_SIZE];

#if (BOARD_DESIGN_VER_VIDEO >= 300)
	if (is_port_b)
		ulDeviceSel = I2C_HOST_VIDEO_DDC_EEPROM_B;
#endif

	/* The EEPROM is on board and should be already ready. */
	if (WaitI2CRdy(ulDeviceSel, addr, 500)) {
		uerr("I2C is not ready\n");
		goto fail;
	}

	for (i = 0; i < EDID_ROM_SIZE; i++) {
		/* Wait I2C ready before going further */
		if (SetI2CReg(ulDeviceSel, addr, i, pEdid[i]))
			goto fail;
		/*
		** Bruce141202 EEPROM write is slow and we should yield CPU here to
		** avoid video engine sw_watchdog time up.
		*/
		if (!(i & 0xF))
			yield();
	}
	/* Wait I2C ready before going further. */

	/* 
	** Somtimes we will met physical problems causes write eeprom incorrectly.
	** To resolve this issue, we double check again.
	*/	
	if (hostReadEepromEdid(arEdid, is_port_b)) {
		uerr("Fail to read eeprom again\n");
		goto fail;
	}

	if (CmpEdid(arEdid, pEdid)) {
		uerr("Failed to write EDID?!\n");
		udump(arEdid, EDID_ROM_SIZE);
		uerr("Should be:\n");
		udump(pEdid, EDID_ROM_SIZE);
		/* Write again, and hopefully we will success one day */
		if (hostWriteEepromEdid(pEdid, is_port_b))
			goto fail;
	}
	return 0;

fail:
	return -ENODEV;
}
EXPORT_SYMBOL(hostWriteEepromEdid);

/* obsoleted */
#if 0
void hostChangeEepromEdid(u8 *pEdid, int btHostSource)
{
	int	i;
	u8	btTemp;

	//don't need to change EDID now
	return;
	
	//Change Display input
	btTemp = pEdid[DISPLAY_PARA_INPUT] & DIGITAL_VALUE;

	if (btTemp)
	{
		//Station is Digital. but Host is Analog, need change to analog
		if (DVI_INPUT != btHostSource)
		{
			pEdid[DISPLAY_PARA_INPUT] = ANALOG_VALUE;
			btTemp = pEdid[EXT_EDID_BLOCK];
			if (btTemp)
			{
				 pEdid[EXT_EDID_BLOCK] = 0;
			
				for (i=128; i<256; i++)
				{
				   pEdid[i] = 0xFF;
				}
			}
		}
	}
	else
	{
		//Station is analog. but Host is digital, need change to digital
		if (DVI_INPUT == btHostSource)
		{
			pEdid[DISPLAY_PARA_INPUT] = DIGITAL_VALUE;
		}
	}

	//check if interlace	
	for (i=0;i<TIMING_DES_GROUP;i++)
	{
		btTemp = TIMING_DES_1+i*TIMING_DES_GRAP;

		if ((0 != pEdid[btTemp]) || (0 != pEdid[btTemp+1])) 
		{
			//find out interlace
			if (pEdid[btTemp+DTD_FLAG]&INTERLACED_MASK)
			{
				//uinfo("find interlace");
				pEdid[btTemp] = 0x00;
				pEdid[btTemp+1] = 0x00;
				pEdid[btTemp+2] = 0x00;
				pEdid[btTemp+3] = 0x10;
				pEdid[btTemp+3] = 0x00;
			}
		}
	}
	
#ifdef DISABLE_CEA_BLOCK
    //ss debug always 128 bytes
    pEdid[EXT_EDID_BLOCK] = 0;

	for (i=128; i<256; i++)
	{
       pEdid[i] = 0xFF;
	}
    
#endif  

	//Final, update checksum
	btTemp = 0;

	for (i=0; i<127; i++)
	{
       btTemp = btTemp + pEdid[i];
	}

	pEdid[CHECKSUM_FLAG] = ~btTemp + 1;	

	//if it has Extension EDID Blocks
	if (!pEdid[EXT_EDID_BLOCK])
		return;

#ifndef DISABLE_CEA_BLOCK
	uinfo("Has Extend Edid\n");

{
	int	j, nDTDPos; //nDTDPos can't use byte unit,because the value is only to 256
	u8 btDateBlkColPos, btDTDBegin;
	u8 btNumBytesBlock;
	u8 InterValue[10] = {5,6,7,10,11,20,21,22,25,26}; 	
	u8 btFirstDTD1, btFirstDTD2;
	u8 btHasFindProgree = 0;
	u8 ProgressDTD[18];

	btDateBlkColPos = 0;
	nDTDPos		    = 0;
	btDTDBegin = pEdid[DTD_BEGIN];

	//erase video Data Block Collection
	btDateBlkColPos = DATA_BLOCK_COL_START;

	do
	{
		btNumBytesBlock = pEdid[btDateBlkColPos]&NUM_BYTES_MASK;
		btTemp	= pEdid[btDateBlkColPos]&BLK_TYPE_TAG;

		if (VIDEO_BLK_TAG == btTemp)
		{
			for (i=(btDateBlkColPos+1); i<(btDateBlkColPos+btNumBytesBlock+1); i++)
			{
				btTemp = pEdid[i]&0x7F; //mask "native"

				for (j=0; j<10; j++)
				{
					if (btTemp == InterValue[j])
					{
						pEdid[i] = 0;
						break;
	}
				}
			}
		}

		btDateBlkColPos = btDateBlkColPos + btNumBytesBlock + 1; 	
	} while (btDateBlkColPos < (btDTDBegin+EXT_EDID_OFFSET));


	//erase cea DTD
	//1. Find out one process mode
	nDTDPos	= EXT_EDID_OFFSET + btDTDBegin;
	btFirstDTD1 = pEdid[nDTDPos];
	btFirstDTD2 = pEdid[nDTDPos+1];
	while ((0 !=btFirstDTD1) || (0 !=btFirstDTD2))
	{
	    if (nDTDPos >= 255)
            break;	    
		btFirstDTD1 = pEdid[nDTDPos];
		btFirstDTD2 = pEdid[nDTDPos+1];

		//first: find one of progress mode
		if ((!btHasFindProgree) && (!(pEdid[nDTDPos+DTD_FLAG]&INTERLACED_MASK)))
		{
			//save to temp
			for (i=0; i<18; i++)
			{
				ProgressDTD[i] = pEdid[nDTDPos+i];
			}
			btHasFindProgree = 1;
			break;
		}
		nDTDPos = nDTDPos + 18;
		btFirstDTD1 = pEdid[nDTDPos];
		btFirstDTD2 = pEdid[nDTDPos+1];
	}

	//2. fill process mode into interlace mode
	nDTDPos	= EXT_EDID_OFFSET + btDTDBegin;
	btFirstDTD1 = pEdid[nDTDPos];
	btFirstDTD2 = pEdid[nDTDPos+1];
	while ((0 !=btFirstDTD1) || (0 !=btFirstDTD2))
	{
	    if (nDTDPos >= 255)
            break;
		//first: find one of progress mode
		if (pEdid[nDTDPos+DTD_FLAG]&INTERLACED_MASK)
		{
			//save to temp
			for (i=0; i<18; i++)
		{
				pEdid[nDTDPos+i] = ProgressDTD[i];
		}
	}
		nDTDPos = nDTDPos + 18;
		btFirstDTD1 = pEdid[nDTDPos];
		btFirstDTD2 = pEdid[nDTDPos+1];
	}
	

	//Final, update checksum
	btTemp = 0;

	for (i=128; i<255; i++)
	{
       btTemp = btTemp + pEdid[i];
	}

	pEdid[255] = ~btTemp + 1;	

} //extented DTD section
#endif
}
EXPORT_SYMBOL(hostChangeEepromEdid);

//Return 0 if analog, else return non-zero
unsigned int EdidIsDigital(unsigned char *pEdid)
{
	//Change Display input
	return pEdid[DISPLAY_PARA_INPUT] & DIGITAL_VALUE;
}
EXPORT_SYMBOL(EdidIsDigital);

//Get max resolution
void EdidGetMaxRes(unsigned char *pEdid, u16 *outWidth, u16 *outHeight)
{
	int i;
	unsigned char	btTemp;
	u16	usWidth, usHeight;
	u16	tmpWidth, tmpHeight;

	u8	btDTDBegin, btDTDPos;
	u8	btFirstDTD1, btFirstDTD2;

	usWidth		= 0;
	usHeight	= 0;

	for (i=0;i<TIMING_DES_GROUP;i++)
	{
		btTemp = TIMING_DES_1+i*TIMING_DES_GRAP;

		if ((0 != pEdid[btTemp]) || (0 != pEdid[btTemp+1])) 
		{
			tmpWidth	= (((pEdid[btTemp+4]&0xF0)>>4)<<8) | pEdid[btTemp+2];
			tmpHeight	= (((pEdid[btTemp+7]&0xF0)>>4)<<8) | pEdid[btTemp+5];

			if ((tmpWidth > usWidth) && (tmpHeight > usHeight))
			{
				usWidth		= tmpWidth;
				usHeight	= tmpHeight;
			}
		}
	}

	//if it has Extension EDID Blocks
	if (!pEdid[EXT_EDID_BLOCK])
    {
        uinfo("EDID: no block 2\n");
		goto GetMaxWindow;
    }

	btDTDBegin	= pEdid[DTD_BEGIN];
	btDTDPos	= EXT_EDID_OFFSET + btDTDBegin;
	btFirstDTD1 = pEdid[btDTDPos];
	btFirstDTD2 = pEdid[btDTDPos+1];
	while ((0 !=btFirstDTD1) || (0 !=btFirstDTD2))
	{
	    if (btDTDPos >= 255)
			break;
		btFirstDTD1 = pEdid[btDTDPos];
		btFirstDTD2 = pEdid[btDTDPos+1];

		tmpWidth	= (((pEdid[btDTDPos+4]&0xF0)>>4)<<8) | pEdid[btDTDPos+2];
		tmpHeight	= (((pEdid[btDTDPos+7]&0xF0)>>4)<<8) | pEdid[btDTDPos+5];

		if ((tmpWidth > usWidth) && (tmpHeight > usHeight))
		{
			usWidth		= tmpWidth;
			usHeight	= tmpHeight;
		}

		btDTDPos = btDTDPos + 18;
		btFirstDTD1 = pEdid[btDTDPos];
		btFirstDTD2 = pEdid[btDTDPos+1];
	}

GetMaxWindow:
    //uinfo("usWidth [%d] usHeight [%d]\n", usWidth, usHeight);
    *outWidth   = usWidth;
    *outHeight  = usHeight;
}
EXPORT_SYMBOL(EdidGetMaxRes);
#endif
