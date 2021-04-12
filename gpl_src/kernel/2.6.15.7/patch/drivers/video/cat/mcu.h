#ifndef __MCU_H__
#define __MCU_H__



#ifndef HDMI_RX_I2C_SLAVE_ADDR
#define HDMI_RX_I2C_SLAVE_ADDR 0x90
#endif // HDMI_RX_I2C_SLAVE_ADDR

#ifndef HDMI_TX_I2C_SLAVE_ADDR
#define HDMI_TX_I2C_SLAVE_ADDR 0x9A
#endif // HDMI_TX_I2C_SLAVE_ADDR

#if	defined(Linux_UserMode)
void
DelayMS(unsigned short ms) 
{
	/* TODO */
}

SYS_STATUS
I2C_Read_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    I2C_ACCESS_DATA I2C_DATA;
    BYTE *pI2CData;
    unsigned long   i;

    pI2CData = pData;
    
    I2C_DATA.channel = 2;	
    for (i = 0; i < N; i++)
    {
        I2C_DATA.address = Addr;
        I2C_DATA.index = RegAddr + i;
        if (ioctl (fd, IOCTL_I2C_READ, &I2C_DATA) < 0)
        {
            printf("[CAT] Read from Address:%x Index:%x Failed!! \n", Addr, (RegAddr+i));
            return ER_FAIL;        	
        }

        *(BYTE *) (pI2CData++) = I2C_DATA.data;
    }

    return ER_SUCCESS ;
}

SYS_STATUS
I2C_Write_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    I2C_ACCESS_DATA I2C_DATA;
    BYTE *pI2CData;
    unsigned long   i;

    pI2CData = pData;
    
    I2C_DATA.channel = 2;	
    for (i = 0; i < N; i++)
    {
        I2C_DATA.address = Addr;
        I2C_DATA.index = RegAddr + i;
        I2C_DATA.data = *(BYTE *) (pI2CData++);
        if (ioctl (fd, IOCTL_I2C_WRITE, &I2C_DATA) < 0)
        {
            printf("[CAT] Write to Address:%x Index:%x Failed!! \n", Addr, (RegAddr+i));
            return ER_FAIL;        	
        }
    }

    return ER_SUCCESS ;
}
#elif	defined(Linux_KernelMode)

void
DelayMS(unsigned short ms) 
{
#if (HANDLE_INT_IN_THREAD && USE_THREAD)
    msleep(ms);
#else //#if (HANDLE_INT_IN_THREAD && USE_THREAD)
    mdelay(ms);
#endif //#if (HANDLE_INT_IN_THREAD && USE_THREAD)
}

#ifndef CAN_NOT_FIND_DEVICE
#define CAN_NOT_FIND_DEVICE     1
#endif

#ifndef SET_I2C_DONE
#define SET_I2C_DONE            0
#endif

#ifdef	TestMode
#define	APB_BRIDGE_2_BASE       0x1E780000
#define	I2C_BASE		0xA000
#define AC_TIMING               0x77743335
#define LOOP_COUNT              0x100000

#define ReadMemoryLong(baseaddress,offset)        (*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))
#define WriteMemoryLong(baseaddress,offset,data)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset)))=(u32)(data)
#define WriteMemoryBYTE(baseaddress,offset,data)  *(volatile u8 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (u8)(data)    
#define WriteMemoryLongWithMASK(baseaddress, offset, data, mask)  *(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))) = (((*(volatile u32 *)(IO_ADDRESS((u32)(baseaddress)+(u32)(offset))))&(~(mask))) | (u32)((data)&(mask)))

void I2CInit(BYTE   DeviceSelect)
{
//  Reset
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 0);
//  Set Speed
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, AC_TIMING);
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0x7333f823);
//  Lower Speed
//    WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0, 0x33317805);
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x08, 0);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable Master Mode
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 1);
//  Enable Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0xAF);
}

int  SetI2CReg(u32   DeviceSelect, 
               u32   DeviceAddress, 
               u32   RegisterIndex, 
               BYTE   RegisterValue)
{
    ULONG   Status; 
    ULONG   Count = 0;
    ULONG flags;

    spin_lock_irqsave(&cat_s_lock_i2c, flags);
    	
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

		if (2 == Status)
		{
			//Clear Interrupt
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
			//Re-Send Start and Send Device Address while NACK return
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		}
		else
		{
			if (Count > LOOP_COUNT) {
                            goto SetI2CReg_Fail;
			}
		}
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Value and Stop
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterValue);
    barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
    do {
        Count++;
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
    Count = 0;
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Enable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
	barrier();
//  Wait STOP
    do {
        Count++;
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
        if (Count > LOOP_COUNT) {
            goto SetI2CReg_Fail;
        }
    } while (Status != 1);
//  Disable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0, 0x10);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    return SET_I2C_DONE;
    
SetI2CReg_Fail:
    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    return CAN_NOT_FIND_DEVICE;

}

int  GetI2CReg(u32 DeviceSelect, 
	       u32 DeviceAddress, 
	       u32 RegisterIndex,
	       BYTE *outValue)
{
    BYTE    Data;
    ULONG   ulError = 0;
    ULONG   Status;
    int		i;
    ULONG flags;

    spin_lock_irqsave(&cat_s_lock_i2c, flags);

	//uinfo("GetI2CReg\n");
//  BYTE I2C Mode
//  Start and Send Device Address
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
	barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
//  Wait TX ACK
	i = 0;
    //uinfo("Start and Send Device Address\n");
    do {
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;
    } while (Status != 1);
    
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Send Device Register Index
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
//  Wait Tx ACK
	i = 0;
    //uinfo("Send Device Register Index\n");
    do {
        Status = ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x03;

        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;        
    } while (Status != 1);
    
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
//  Start, Send Device Address + 1(Read Mode), Receive Data
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress + 1);
	barrier();
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x1B);
//  Wait Rx Done
	i = 0;
    //uinfo("Start, Send Device Address + 1(Read Mode), Receive Data\n");
    do {
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x04) >> 2;
        
        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;           
    } while (Status != 1);

STOPI2C:   
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

//  Enable STOP Interrupt
    WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
//  Issue STOP Command
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
//  Wait STOP
	i = 0;
    //uinfo("Enable STOP Interrupt\n");
    do {
        Status = (ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10) & 0x10) >> 4;
        
        if (i>1000)
        {
            //return FALSE;
            ulError = 1;
            goto STOPI2C;
        }    
        i++;                 
    } while (Status != 1);
    
//  Disable STOP Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10);
//  Clear Interrupt
    WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

    spin_unlock_irqrestore(&cat_s_lock_i2c, flags);
    if (1 == ulError)
    {
        return 1;
    }
    else
    {
        //  Read Received Data
        Data = (BYTE)((ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00) >> 8);
        *outValue = Data;
        return    0;
    }
}
#endif

SYS_STATUS
I2C_Read_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    BYTE *pI2CData, jData;
    unsigned long   i;

    pI2CData = pData;
    
    for (i = 0; i < N; i++)
    {
        if (GetI2CReg(HDMI_I2C_CHANNEL, Addr, (RegAddr + i), &jData) != 0)
        {
            return ER_FAIL;	
        }
	
        *(BYTE *) (pI2CData++) = jData;
    }

    return ER_SUCCESS ;
	
}

SYS_STATUS
I2C_Write_ByteN(BYTE Addr,BYTE RegAddr,BYTE *pData,int N)
{
    unsigned long   i;
    
    for (i = 0; i < N; i++)
    {
        if (SetI2CReg(HDMI_I2C_CHANNEL, Addr, (RegAddr + i), *(BYTE *)(pData + i)) == CAN_NOT_FIND_DEVICE)
        {
            return ER_FAIL;	
        }	
    }

    return ER_SUCCESS ;
}
#endif

BYTE
I2C_Read_Byte(BYTE Addr,BYTE RegAddr)
{
    BYTE data ;
    I2C_Read_ByteN(Addr,RegAddr,&data,1)  ;
    return data ;
}

SYS_STATUS
I2C_Write_Byte(BYTE Addr,BYTE RegAddr,BYTE Data)
{
    return I2C_Write_ByteN(Addr,RegAddr,&Data,1) ;
}

BYTE
HDMITX_ReadI2C_Byte(BYTE RegAddr)
{
	return I2C_Read_Byte(HDMI_TX_I2C_SLAVE_ADDR,RegAddr) ;
}

SYS_STATUS
HDMITX_WriteI2C_Byte(BYTE RegAddr,BYTE Data)
{
	return I2C_Write_Byte(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,Data) ;
}

SYS_STATUS
HDMITX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Read_ByteN(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}

SYS_STATUS
HDMITX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Write_ByteN(HDMI_TX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}


BYTE
HDMIRX_ReadI2C_Byte(BYTE RegAddr)
{
	return I2C_Read_Byte(HDMI_RX_I2C_SLAVE_ADDR,RegAddr) ;
}

SYS_STATUS
HDMIRX_WriteI2C_Byte(BYTE RegAddr,BYTE Data)
{
	return I2C_Write_Byte(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,Data) ;
}

SYS_STATUS
HDMIRX_ReadI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Read_ByteN(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}

SYS_STATUS
HDMIRX_WriteI2C_ByteN(BYTE RegAddr,BYTE *pData,int N)
{
	return I2C_Write_ByteN(HDMI_RX_I2C_SLAVE_ADDR,RegAddr,pData,N) ;
}

#endif //#ifndef __MCU_H__

