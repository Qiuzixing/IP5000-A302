/* 
 * xmiter hal driver for CH7301 (chrontel)
 */
#ifndef _XMITER_CH7301_C_
#define _XMITER_CH7301_C_

#include <linux/delay.h>

/* CH7301 Register Definition */
#define     CH7301_CD_REG				0x20
	#define     CD_DACT					0x0E
	#define		CD_DVIT					1 << 5
#define     CH7301_DC_REG				0x21
#define     CH7301_PM_REG				0x49


#define I2C_DEV_ADDR 0xEC
#define I2C_BUS_NUM 3

#if 1
#define _set_reg(idx, value) SetI2CReg(I2C_BUS_NUM, I2C_DEV_ADDR, idx, value)
#define _get_reg(idx, pValue) GetI2CReg(I2C_BUS_NUM, I2C_DEV_ADDR, idx, pValue)
#else
static inline int _set_reg(unsigned int idx, unsigned char value)
{
	return SetI2CReg(I2C_BUS_NUM, I2C_DEV_ADDR, idx, value);
}

static inline int _get_reg(unsigned int idx, unsigned char *outValue)
{
	return GetI2CReg(I2C_BUS_NUM, I2C_DEV_ADDR, idx, outValue);
}
#endif

/*
** Return:
**  0: success
**  <0: fail
*/
static unsigned int xmiter_init_ch7301(struct s_crt_drv *crt)
{
	return 0;
}


/*
** Return:
**  0: success
**  <0: fail
*/
static unsigned int xmiter_disable_ch7301(struct s_crt_drv *crt)
{
	return 0;
}


/*
** return:
** 1: is analog
** 0: is digital or fail
*/
static unsigned int _CheckDAC(void)
{
	unsigned int isAnalog = 0;
	unsigned char btValue;
    int     i;

	//Enable all DAC's and set register 21h[0] = '0'
	//DVIP and DVIL disable for DAC
	_set_reg(CH7301_PM_REG, 0x00);

	//btValue = GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_DC_REG);
	i = 0;
	while (_get_reg(CH7301_CD_REG, &btValue))
    {
        if (i>200)
			goto out;

        i++;
        msleep(5);
    }
	
	btValue = btValue & 0xFE;
	_set_reg(CH7301_DC_REG, btValue);

	//Set SENSE bit to 1
	//btValue = GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG);
	i = 0;
	while (_get_reg(CH7301_CD_REG, &btValue))
    {
        if (i>200)
			goto out;        

        i++;
        msleep(5);
    }
	
	btValue = btValue | 0x01;
	_set_reg(CH7301_CD_REG, btValue);

	//Reset SENSE bit to 0
	//btValue = GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG);
	i = 0;
	while (_get_reg(CH7301_CD_REG, &btValue))
    {
        if (i>200)
			goto out;
        
        i++;
        msleep(5);
    }
	
	btValue = btValue & 0xFE;
	_set_reg(CH7301_CD_REG, btValue);

	//bValue = (GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, CH7301_CD_REG) & CD_DACT) ? TRUE : FALSE;
	i = 0;
	while (_get_reg(CH7301_CD_REG, &btValue))
    {
        if (i>200)
			goto out;
		
        i++;
        msleep(5);
    }
	
    isAnalog = (btValue & CD_DACT)?1 : 0;

out:
	return isAnalog;
}



/*
** Return:
**  0: success
**  <0: fail
*/
static unsigned int xmiter_setup_ch7301(struct s_crt_drv *crt, struct s_crt_info *info)
{
	MODE_ITEM *mt = crt->mode_table;
	int i;
	unsigned int isAnalog;

	isAnalog = _CheckDAC();
	
	i = lookupModeTable(crt, info);

    // pixel clock is less than VCLK65 (*10000) 65MHz. Datasheet P.20 table 10.
	if (mt[i].DCLK10000 <= 650000) 
	{
		_set_reg(0x33, 0x08);
		_set_reg(0x34, 0x16);
		_set_reg(0x36, 0x60);
	}
	else
	{
		_set_reg(0x33, 0x06);
		_set_reg(0x34, 0x26);
		_set_reg(0x36, 0xA0);
	}

	switch (isAnalog)
	{
	case 0:
		//DVI is normal function
		 _set_reg(0x49, 0xC0);
         _set_reg(0x1D, 0x47);
		break;
	case 1:
		//RGB
		_set_reg(0x48, 0x18);
		_set_reg(0x49, 0x0);
		_set_reg(0x56, 0x0);
		_set_reg(0x21, 0x9);
		//SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x1D, 0x4F);
		_set_reg(0x1D, 0x48); //default
		//SetI2CRegClient(MMIOBase, 0x3, btDeviceSelect, 0x1C, 0x40);
		_set_reg(0x1C, 0x00); //default
		break;
	default:
		break;
	};

    //uinfo("7301 33[%x]\n", GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, 0x33));
    //uinfo("7301 34[%x]\n", GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, 0x34));
    //uinfo("7301 36[%x]\n", GetI2CReg(DEVICE_SELECT_CH7301, btDeviceSelect, 0x36));
    
	return 0;
}

/*
** return:
** 1: is plugged
** 0: is not plugged
*/
static unsigned int xmiter_check_hotplug_ch7301(struct s_crt_drv *crt)
{
	unsigned int isPlugged = 0;
    unsigned char bTemp;
    
	if (_get_reg(CH7301_CD_REG, &bTemp))
		goto out;

	isPlugged = (bTemp & CD_DVIT)? 1 : 0;

out:
	return isPlugged;

}

#undef _set_reg
#undef _get_reg
#undef I2C_DEV_ADDR
#undef I2C_BUS_NUM

#endif //#ifndef _XMITER_CH7301_C_

