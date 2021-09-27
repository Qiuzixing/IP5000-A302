#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include "util.h"
#include "AST1500_I2C.h"
#include <asm/arch/drivers/board_def.h>
#include <asm/arch/ast-scu.h>
#if defined(AST_I2C_BLOCK_ACCESS)
#if defined(I2C_BLOCK_ACCESS_DMA)
#include <linux/dma-mapping.h>
#else
#include <asm/io.h>
#endif
#endif

#define DRIVER_AUTHOR "Steven"
#define DRIVER_DESC "AST I2C driver"
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

static const char driver_name[] = "i2c";

static int I2C_major = 256;
static spinlock_t I2CSpinLock[NUM_I2C_BUS];
static unsigned int I2CBusSpeed[NUM_I2C_BUS];

#if defined(AST_I2C_BLOCK_ACCESS)
struct i2c_buff_info {
	u8 *dma_buf;
#if defined(I2C_BLOCK_ACCESS_DMA)
	u32 dma_addr;
#endif
	u32 dma_size;
};
struct i2c_buff_info buff_info[NUM_I2C_BUS];
#endif

static unsigned int THE_PCLK = 0;

void I2CInit(u32 DeviceSelect, u32 speed);
module_param(I2C_major, int, 0);

#define I2C_TIMEOUT 5000 //us
#define I2C_TIMEOUT_DELAY() do { udelay(1); } while (0)



#if 0//from river
static unsigned int i2c_log2table[21] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576};
static u32 select_i2c_clock(unsigned int speed)
{
	unsigned int pclk, divisor, base_clock, tCK_H, tCK_L, i, j, k;
	u32 data;

#if (CONFIG_AST1500_SOC_VER == 1)
	pclk = 39000000;
#elif (CONFIG_AST1500_SOC_VER >= 2)
	pclk = 27000000;
#else
	#error "PCLK undetermined?!"
#endif

	divisor = pclk / speed;
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 8; j++) {
			for (k = 0; k < 8; k++) {
				if (i2c_log2table[i] * (j + k + 2) > divisor) {
					base_clock = i;
					tCK_H = j;
					tCK_L = k;
					data = 0x77700300 | (tCK_H << 16) | (tCK_L << 12) | base_clock;
					printk("I2CD04 for %d = %08X\n", speed, data);
					return data;
				}
			}
		}
	}
	printk("cannot select a working I2CD04 for %d\n", speed);
	BUG();
	return 0;
}
#else//from datasheet
#define REG_SCU08 (0x08)
#define REG_SCU24 (0x24)
#define REG_SCU70 (0x70)
static unsigned int get_HPLL_Hz(void)
{
	unsigned int scu24, hpll_select;

	scu24 = ReadMemoryLong(SCU_BASE, REG_SCU24);
	if (scu24 & (1<<18)) {
		unsigned int OD, num, denum;
		/* H-PLL from SCU24[17:0] */
		if (scu24 & (1<<17)) {
			//bypass from 24MHz directly.
			return 24000000;
		}
		if (scu24 & (1<<16)) {
			printk("H-PLL is off?!\n");
			//Ignore.
		}
		//OD == SCU24[4]
		OD = (scu24 >> 4) & 1;
		//Numerator == SCU24[10:5]
		num = (scu24 >> 5) & 0x3F;
		//Denumerator == SCU24[3:0]
		denum = scu24 & 0xF;

#if (CONFIG_AST1500_SOC_VER == 1)
		{
			unsigned int postDiv;
			//Post Div == SCU24[14:12]
			if (scu24 & (1<<14)) {
				postDiv = 1 << (((scu24 >> 12) & 0x3) + 1);
			} else {
				postDiv = 1;
			}
			//hpll = (24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1)))/postDiv
			return (24000 * (2-OD) * ((num+2)*1000/(denum+1)))/postDiv;
		}
#endif
		//hpll = 24MHz * (2-OD) * ((Numerator+2)/(Denumerator+1))
		return (24000 * (2-OD) * ((num+2)*1000/(denum+1)));
	}

	// H-PLL from trap pin reg.
#if (CONFIG_AST1500_SOC_VER == 1)
	hpll_select = (ReadMemoryLong(SCU_BASE, REG_SCU70) >> 9) & 0x7;
	switch (hpll_select) {
	case 0:
		return 266000000;
	case 1:
		return 233000000;
	case 2:
		return 200000000;
	case 3:
		return 166000000;
	case 4:
		return 133000000;
	case 5:
		return 100000000;
	case 6:
		return 300000000;
	case 7:
		return 24000000;
	default:
		BUG();
	};
#elif (CONFIG_AST1500_SOC_VER >= 2)
	hpll_select = (ReadMemoryLong(SCU_BASE, REG_SCU70) >> 8) & 0x3;
	switch (hpll_select) {
	case 0:
		return 384000000;
	case 1:
		return 360000000;
	case 2:
		return 336000000;
	case 3:
		return 408000000;
	default:
		BUG();
	};
#else
#error "H-PLL undetermined?!"
#endif
}
//static unsigned int get_PCLK_Hz(void)
//{
//	unsigned int div, hpll;
//
//	//PCLK == HPLL/div, div is from SCU08[25:23]
//	hpll = get_HPLL_Hz();
//	div = (((ReadMemoryLong(SCU_BASE, REG_SCU08) >> 23) & 0x7) + 1) << 1;
//	printk("HPLL=%d, Div=%d, PCLK=%d\n", hpll, div, hpll/div);
//	return (hpll/div);
//}
static u32 select_i2c_clock(unsigned int speed)
{
	unsigned int pclk, inc = 0, div, divider_ratio;
	u32 SCL_Low, SCL_High, data;

	pclk = THE_PCLK;

	divider_ratio = pclk / speed;
	for (div = 0; divider_ratio >= 16; div++)
	{
		inc |= (divider_ratio & 1);
		divider_ratio >>= 1;
	}
	divider_ratio += inc;
	SCL_Low = (divider_ratio >> 1) - 1;
	SCL_High = divider_ratio - SCL_Low - 2;
	data = 0x77700300 | (SCL_High << 16) | (SCL_Low << 12) | div;
	printk("I2CD04 for %d = %08X\n", speed, data);
	return data;
}
#endif

//DeviceSelect is from 1.
#if 0
void I2CInit(u32   DeviceSelect)
#else
void _I2CInit_(u32   DeviceSelect, unsigned int speed)
#endif
{
	ast_scu.scu_op(SCUOP_I2C_INIT, (void *) DeviceSelect);
	/* For AST1520, we move multiple function pin configuration to SCU driver */
#if (CONFIG_AST1500_SOC_VER == 2)
	//must explicitly enable multi-function pins for I2C3 - I2C14
	if (DeviceSelect >= 3)
		WriteMemoryLong(SCU_BASE, MULTIFUCTION_PIN_CONTROL_5,
		ReadMemoryLong(SCU_BASE, MULTIFUCTION_PIN_CONTROL_5) | (1 << (DeviceSelect + 13)));
#endif
	//	Reset
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 0);
	barrier();
	//	Set Speed
#if 0
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, AC_TIMING);
#else
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, select_i2c_clock(speed));
#endif
	barrier();
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0x7333f823);
	//	Lower Speed
	//	  WriteMemoryLongWithANDData (VideoEngineInfo->VGAPCIInfo.ulMMIOBaseAddress, I2C_BASE + DeviceSelect * 0x40 + 0x04, 0, 0x33317805);
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x08, 0);
	barrier();
	//	Clear Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	barrier();
	//	Enable Master Mode
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x00, 1);
	barrier();
	//	Enable Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0xAF);
	barrier();
}

#define S_ACK          (1 << 0)
#define S_NAK          (1 << 1)
#define S_RX_DONE  (1 << 2)
#define S_STOP        (1 << 4)
#define S_RECOVERED (1 << 13)
#define S_MASK (S_NAK | S_ACK | S_RX_DONE | S_STOP | S_RECOVERED)
#define NOP_MASK (0xC2F) //BIT 11,10,5,3,2,1,0

static int _wait_status(u32 DeviceSelect, u32 to_wait, u32 timeout)
{
	u32 cnt = 0;
	u32 Status = 1; /* assign value to make the compiler happy */

	if (0 == timeout) {
		printk("Improper timeout value %d\n", timeout);
		return -1;
	}

	//Wait command sent
	for (cnt = 0; cnt < timeout; cnt++) {
		Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
		                        I2C_BASE + DeviceSelect * 0x40 + 0x14) & NOP_MASK;
		if (Status == 0)
			break;

		I2C_TIMEOUT_DELAY();
	}
	if (Status) {
		printk("I2C controller goes crazy (0x%08x)\n", Status);
		return -1;
	}

	for (cnt = 0; cnt < timeout; cnt++) {
		//Read interrupt status
		Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
		                        I2C_BASE + DeviceSelect * 0x40 + 0x10);
		//Clear interrupt status
		WriteMemoryLong(APB_BRIDGE_2_BASE,
		                        I2C_BASE + DeviceSelect * 0x40 + 0x10, Status);
		barrier();
		Status &= S_MASK;
		if (Status & to_wait)
			return Status;

		I2C_TIMEOUT_DELAY();
	}
	printk("wait_status timeouted (%d) (0x%08x) (0x%08x)\n", DeviceSelect, to_wait, ReadMemoryLong(APB_BRIDGE_2_BASE,
		                        I2C_BASE + DeviceSelect * 0x40 + 0x10));
	return -1;
}

#define wait_status(dev, wait) _wait_status(dev, wait, I2C_TIMEOUT)

/* runs under device lock */
int _i2c_stop(u32 DeviceSelect)
{
	u32 Status;

	//  Clear Interrupt
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	//  Enable STOP Interrupt
	WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0x10, 0x10);
	barrier();
	//  Issue STOP Command
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x20);
	barrier();
	//  Wait STOP
	Status = wait_status(DeviceSelect, S_STOP);

	//  Disable STOP Interrupt
	WriteMemoryLongWithMASK(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x0C, 0, 0x10);
	barrier();
	//  Clear Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

	if (Status == -1)
		return -1;

	return 0;
}

#define SDA  (1<<17)
#define SCL  (1<<18)

static int bus_hang_recover(u32 DeviceSelect)
{
	u32 Status;
	u32 i = 0;

	//Check 0x14's SDA and SCL status
	Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
	                        I2C_BASE + DeviceSelect * 0x40 + 0x14);

	if ((Status & SDA) && (Status & SCL)) {
		//Means bus is idle.
		printk("I2C bus (%d) is idle. I2C slave doesn't exist?!\n", DeviceSelect);
		return -1;
	}

	printk("ERROR!! I2C(%d) bus hanged, try to recovery it!\n", DeviceSelect);

	//if SDA == 1 and SCL == 0, it means the master is locking the bus.
	//Send a stop command to unlock the bus.
	if ((Status & SDA) && !(Status & SCL)) {
		printk("I2C's master is locking the bus, try to stop it.\n");
		Status = _i2c_stop(DeviceSelect);
		if (Status) {
			printk("ERROR!! Failed to stop I2C(%d)\n", DeviceSelect);
			return -1;
		}
	} else if (!(Status & SDA)) {
		//else if SDA == 0, the device is dead. We need to reset the bus
		//And do the recovery command.
		printk("I2C's slave is dead, try to recover it\n");
		//Let's retry 10 times
		for (i = 0; i < 10; i++) {
#if 0
			I2CInit(DeviceSelect);
#else
			_I2CInit_(DeviceSelect, I2CBusSpeed[DeviceSelect - 1]);
#endif
			//Do the recovery command BIT11
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x800);
			Status = wait_status(DeviceSelect, S_RECOVERED);
			if (Status == -1) {
				printk("ERROR!! Failed to do recovery command(0x%08x)\n", Status);
				return -1;
			}
			//Check 0x14's SDA and SCL status
			Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
			                        I2C_BASE + DeviceSelect * 0x40 + 0x14);
			if (Status & SDA) //Recover OK
				break;
		}
		if (i == 10) {
			printk("ERROR!! recover failed\n");
			return -1;
		}
	} else {
		printk("Don't know how to handle this case?!\n");
		return -1;
	}
	printk("Recovery successfully\n");
	return 0;
}

/* copy from bus_hang_recover and add dbg to determine whether print message */
static int bus_hang_recover_dbg(u32 DeviceSelect, u32 dbg)
{
#define DPRINT(fmt, args...) { if (dbg) printk(fmt, ##args); }
	u32 Status;
	u32 i = 0;

	/* Check 0x14's SDA and SCL status */
	Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
				I2C_BASE + DeviceSelect * 0x40 + 0x14);

	if ((Status & SDA) && (Status & SCL)) {
		/* Means bus is idle. */
		DPRINT("I2C bus (%d) is idle. I2C slave doesn't exist?!\n", DeviceSelect);
		return -1;
	}

	DPRINT("ERROR!! I2C(%d) bus hanged, try to recovery it!\n", DeviceSelect);

	/* if SDA == 1 and SCL == 0, it means the master is locking the bus. */
	/* Send a stop command to unlock the bus. */
	if ((Status & SDA) && !(Status & SCL)) {
		DPRINT("I2C's master is locking the bus, try to stop it.\n");
		Status = _i2c_stop(DeviceSelect);
		if (Status) {
			DPRINT("ERROR!! Failed to stop I2C(%d)\n", DeviceSelect);
			return -1;
		}
	} else if (!(Status & SDA)) {
		/* else if SDA == 0, the device is dead. We need to reset the bus */
		/* And do the recovery command. */
		DPRINT("I2C's slave is dead, try to recover it\n");
		/* Let's retry 10 times */
		for (i = 0; i < 10; i++) {
			_I2CInit_(DeviceSelect, I2CBusSpeed[DeviceSelect - 1]);
			/* Do the recovery command BIT11 */
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x800);
			Status = wait_status(DeviceSelect, S_RECOVERED);
			if (Status == -1) {
				DPRINT("ERROR!! Failed to do recovery command(0x%08x)\n", Status);
				return -1;
			}
			/* Check 0x14's SDA and SCL status */
			Status = ReadMemoryLong(APB_BRIDGE_2_BASE,
						I2C_BASE + DeviceSelect * 0x40 + 0x14);
			if (Status & SDA) /* Recover OK */
				break;
		}
		if (i == 10) {
			DPRINT("ERROR!! recover failed\n");
			return -1;
		}
	} else {
		DPRINT("Don't know how to handle this case?!\n");
		return -1;
	}
	DPRINT("Recovery successfully\n");
	return 0;
}

#undef SDA
#undef SCL

/* runs under device lock */
int _i2c_start(u32 DeviceSelect, u32 DeviceAddress)
{
	u32   Status;
	u32   Count = 0;

	//uinfo("SetI2CReg: Start and Send Device Address\n");
	for (Count = 0; Count < I2C_TIMEOUT; Count++) {
		//  Start and Send Device Address
		//ToDo. EEPROM > 256 bytes will have problem.
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
		barrier();
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		barrier();

		//  Wait Tx ACK
		Status = wait_status(DeviceSelect, (S_ACK | S_NAK));
		if (Status == -1)
		{
			printk("wait_status failed\n");
			goto fail;
		}

		if (Status & S_ACK)
			goto ok;

		if (Status & S_NAK) {
			//Calling stop to release I2C Master.
#if 1
			printk("S_NAK\n");
			_i2c_stop(DeviceSelect);
			// S_NAK means I2C slave is not available. go to fail right away.
			goto fail;
#else
			if (_i2c_stop(DeviceSelect))
				goto fail;
#endif
		}
		I2C_TIMEOUT_DELAY();
	}

fail:
	//  Clear Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	bus_hang_recover(DeviceSelect);
	//We try to recover the bus, but we let the caller to retry the fail case.
	return -1;
ok:
	//  Clear Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	return 0;
}

int IsI2CReady(u32   DeviceSelect,
                 u32   DeviceAddress)
{
	u32   Status;
	u32   ulError = 0;
	unsigned long	flags;

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
	//  BYTE I2C Mode
	Status = _i2c_start(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto out;
	}

	//If i2c started, we need to stop it.
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1)
		ulError = 1;

out:
	if (1 == ulError)
	{
		printk("I2C (%d, 0x%02x) is not ready\n", DeviceSelect, DeviceAddress);
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;
}

#define I2C_TIMEOUT_LITE (I2C_TIMEOUT >> 1)
/* copy from _i2c_start and change I2C_TIMEOUT_LITE to I2C_TIMEOUT_LITE */
static int _i2c_start_lite(u32 DeviceSelect, u32 DeviceAddress)
{
	u32   Status;
	u32   Count = 0;

	for (Count = 0; Count < I2C_TIMEOUT_LITE; Count++) {
		/* Start and Send Device Address */
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
		barrier();
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		barrier();

		/* Wait Tx ACK */
		Status = _wait_status(DeviceSelect, (S_ACK | S_NAK), I2C_TIMEOUT_LITE);
		if (Status == -1)
			goto fail;

		if (Status & S_ACK)
			goto ok;

		if (Status & S_NAK) {
			/* Calling stop to release I2C Master. */
			_i2c_stop(DeviceSelect);
			/* S_NAK means I2C slave is not available. go to fail right away. */
			goto fail;
		}
		I2C_TIMEOUT_DELAY();
	}

fail:
	/* Clear Interrupt */
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	bus_hang_recover_dbg(DeviceSelect, 0);
	/* We try to recover the bus, but we let the caller to retry the fail case. */
	return -1;
ok:
	/* Clear Interrupt */
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	return 0;
}

int IsI2CReadyLite(u32 DeviceSelect, u32 DeviceAddress)
{
	u32   Status;
	u32   ulError = 0;
	unsigned long	flags;

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect - 1], flags);
	/* BYTE I2C Mode */
	Status = _i2c_start_lite(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto out;
	}

	/* If i2c started, we need to stop it. */
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1)
		ulError = 1;

out:
	if (1 == ulError) {
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect - 1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect - 1], flags);
	return 0;
}

/* To avoid the noisy err msg, I re-write the start code here. */
int WaitI2CRdy(u32 DeviceSelect, u32 DeviceAddress, u32 ms)
{
	u32   Status;
	u32   ulError = 0;
	unsigned long	flags;
#if 0//since interrupt is disabled during spin_lock_irqsave, jiffies will not increment, so it cannot be used as the timeout mehanism.
	unsigned int timeout_jiffies;
#else//just use a retry limit
	unsigned int retry;
#endif

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
#if 0
	timeout_jiffies = jiffies + msecs_to_jiffies(ms);
#endif

#if 0
	while (1)
#else
	for (retry = 0; retry < ms; retry++)
#endif
	{
		//  Start and Send Device Address
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress);
		barrier();
		WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x3);
		barrier();

		//  Wait Tx ACK
		Status = wait_status(DeviceSelect, (S_ACK | S_NAK));
		if (Status == -1) {
			ulError = 1;
			break;
		}

		if (Status & S_ACK) {
			//If i2c started, we need to stop it.
			Status = _i2c_stop(DeviceSelect);
			if (Status == -1)
				ulError = 1;
			break;
		}

		if (Status & S_NAK) {
			//Calling stop to release I2C Master.
			Status = _i2c_stop(DeviceSelect);
			if (Status == -1) {
				ulError = 1;
				break;
			}
#if 0
			if ((ms > 0) && (jiffies > timeout_jiffies)) {
				ulError = 2;
				break;
			}
#endif
			//  Clear Interrupt
			WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
#if 0//useless
			continue;
#endif
		}
	}
#if 1
	if (retry == ms)
		ulError = 2;
#endif
//out:
	//  Clear Interrupt
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);

	if (1 == ulError)
	{
		bus_hang_recover(DeviceSelect);
		//We try to recover the bus, but we let the caller to retry the fail case.
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -ENODEV;
	} else if (2 == ulError) {
		printk("Wait for I2C ready timeout! DeviceSelect %d, DeviceAddress %d\n", DeviceSelect, DeviceAddress);
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;

}

int SetI2CBuf(u32 DeviceSelect,
                u32 DeviceAddress,
                u8 *buf,
                u32 len)
{
    u32   Status;
    u32   ulError = 0;
    u32   count = 0;
    unsigned long	flags;

    if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
        return -ENODEV;
    }

    spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
    Status = _i2c_start(DeviceSelect, DeviceAddress);
    if (Status == -1) {
        ulError = 1;
        goto out;
    }

    for (count = 0; count < len; count++) {
        WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, buf[count]);
        barrier();
        WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
        barrier();
        Status = wait_status(DeviceSelect, S_ACK);
        if (Status == -1) {
            printk("wait status timeout 1 (%d,0x%x,0x%x)\n", DeviceSelect, DeviceAddress,  buf[count]);
            ulError = 1;
            goto STOPWriteI2C;
        }
    }

STOPWriteI2C:
    Status = _i2c_stop(DeviceSelect);
    if (Status == -1) {
        printk("wait status timeout 3 (%d,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, buf[count]);
        ulError = 1;
    }

out:
    if (1 == ulError)
    {
        printk("WriteI2C reg error \n");
        spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
        return -1;
    }
    spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
    return 0;
}

int  SetI2CReg(u32   DeviceSelect,
                 u32   DeviceAddress,
                 u32   RegisterIndex,
                 u8   RegisterValue)
{
	u32   Status;
	u32   ulError = 0;
	unsigned long	flags;

	if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
		return -ENODEV;
	}

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
	//  BYTE I2C Mode
	//  Start and Send Device Address
	Status = _i2c_start(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto out;
	}

	//  Send Device Register Index
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//uinfo("Send Device Register Index\n");
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		printk("wait status timeout 1 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
		goto STOPWriteI2C;
	}

	//  Clear Interrupt
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	//  Send Device Register Value
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterValue);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//uinfo("Send Device Register Value\n");
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		printk("wait status timeout 2 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
		goto STOPWriteI2C;
	}

STOPWriteI2C:
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1) {
		printk("wait status timeout 3 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
	}

out:
	if (1 == ulError)
	{
		printk("WriteI2C reg error \n");
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;
}


int  GetI2CReg(u32 DeviceSelect,
				 u32 DeviceAddress,
				 u32 RegisterIndex,
				 u8 *outValue)
{
	u8    Data;
	u32   ulError = 0;
	u32   Status;
	unsigned long	flags;

	if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
		return -ENODEV;
	}

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
	//uinfo("GetI2CReg\n");
	//  BYTE I2C Mode
	Status = _i2c_start(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto OUT;
	}

	//  Send Device Register Index
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		ulError = 1;
		goto STOPI2C;
	}
	_i2c_stop(DeviceSelect);
	//  Start, Send Device Address + 1(Read Mode), Receive Data
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress + 1);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x1B);
	barrier();
	//  Wait Rx Done
	Status = wait_status(DeviceSelect, S_RX_DONE);
	if (Status == -1) {
		ulError = 1;
		goto STOPI2C;
	}

STOPI2C:
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1) {
		ulError = 1;
	}

OUT:
	if (1 == ulError)
	{
		printk("GetI2C reg error \n");
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	//  Read Received Data
	Data = (u8)((ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00) >> 8);
	*outValue = Data;
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;
}

int SetI2CRegRetry(u32   DeviceSelect,
                 u32   DeviceAddress,
                 u32   RegisterIndex,
                 u8   RegisterValue)
{
	u8 chk = 0;
	unsigned int cnt;

	/* Wait I2C ready before going further */
	if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
		return -ENODEV;
	}

	for (cnt = 0; cnt < I2C_TIMEOUT; cnt++) {
		if (SetI2CReg(DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue))
			return -1;

		//A delay is a MUST. Or sometimes EEPROM will write fail.
		if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
			return -ENODEV;
		}

		GetI2CReg(DeviceSelect, DeviceAddress, RegisterIndex, &chk);
		if (chk == RegisterValue) {
			//Write successfully. return;
			return 0;
		}
		printk("SetI2CReg Compare fail at index %d, value=0x%02x, should be=0x%02x\n",
		           RegisterIndex, chk, RegisterValue);
		//BruceTestOnly.
		//return -1;
		I2C_TIMEOUT_DELAY();
	}

	return -1;

}

int  SetI2CWord(u32   DeviceSelect,
                 u32   DeviceAddress,
                 u32   RegisterIndex,
                 u16   RegisterValue)
{
	u32   Status;
	u32   ulError = 0;
	unsigned long	flags;

#if 0
	if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
		return -ENODEV;
	}
#endif

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
	//  BYTE I2C Mode
	//  Start and Send Device Address
	Status = _i2c_start(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto out;
	}

	//  Send Device Register Index
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//uinfo("Send Device Register Index\n");
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		printk("wait status timeout 1 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
		goto STOPWriteI2C;
	}

	//  Clear Interrupt
	//WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x10, 0xFFFFFFFF);
	//  Send Device Register Value
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, (RegisterValue & 0xFF00) >> 8);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//uinfo("Send Device Register Value\n");
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		printk("wait status timeout 2 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
		goto STOPWriteI2C;
	}
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterValue & 0xFF);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//uinfo("Send Device Register Value\n");
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		printk("wait status timeout 2 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
		goto STOPWriteI2C;
	}

STOPWriteI2C:
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1) {
		printk("wait status timeout 3 (%d,0x%x,0x%x,0x%x)\n", DeviceSelect, DeviceAddress, RegisterIndex, RegisterValue);
		ulError = 1;
	}

out:
	if (1 == ulError)
	{
		printk("WriteI2C reg error \n");
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;
}

int  GetI2CWord(u32 DeviceSelect,
				 u32 DeviceAddress,
				 u32 RegisterIndex,
				 u16 *outValue)
{
	u16    Data;
	u32   ulError = 0;
	u32   Status;
	unsigned long	flags;

#if 0
	if (WaitI2CRdy(DeviceSelect, DeviceAddress, 500)) {
		return -ENODEV;
	}
#endif

	spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
	//uinfo("GetI2CReg\n");
	//  BYTE I2C Mode
	Status = _i2c_start(DeviceSelect, DeviceAddress);
	if (Status == -1) {
		ulError = 1;
		goto OUT;
	}

	//  Send Device Register Index
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, RegisterIndex);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x2);
	barrier();
	//  Wait Tx ACK
	Status = wait_status(DeviceSelect, S_ACK);
	if (Status == -1) {
		ulError = 1;
		goto STOPI2C;
	}
	_i2c_stop(DeviceSelect);
	//  Start, Send Device Address + 1(Read Mode), Receive Data
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20, DeviceAddress + 1);
	barrier();
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0xB);
	barrier();
	//  Wait Rx Done
	Status = wait_status(DeviceSelect, S_RX_DONE);
	if (Status == -1) {
		ulError = 1;
		goto STOPI2C;
	}
	//  Read Received Data
	Data = (u16)(ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00);
	WriteMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x14, 0x18);
	barrier();
	//  Wait Rx Done
	Status = wait_status(DeviceSelect, S_RX_DONE);
	if (Status == -1) {
		ulError = 1;
		goto STOPI2C;
	}
	//  Read Received Data
	Data |= (u16)((ReadMemoryLong(APB_BRIDGE_2_BASE, I2C_BASE + DeviceSelect * 0x40 + 0x20) & 0xFF00) >> 8);
	*outValue = Data;

STOPI2C:
	Status = _i2c_stop(DeviceSelect);
	if (Status == -1) {
		ulError = 1;
	}

OUT:
	if (1 == ulError)
	{
		printk("GetI2C reg error \n");
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
		return -1;
	}
	spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	return 0;
}

#if defined(AST_I2C_BLOCK_ACCESS)
#if defined(I2C_BLOCK_ACCESS_DMA)
#define AST_I2C_DMA_SIZE_MAX 4095
#define AST_I2C_DMA_SIZE 256
static inline void i2c_dma_free(struct i2c_buff_info *p)
{
	if (p->dma_buf) {
		dma_free_coherent(NULL, p->dma_size, p->dma_buf, p->dma_addr);
		p->dma_size = 0;
		p->dma_buf = NULL;
	}
}

static int i2c_dma_alloc(u32 bus, u32 dma_size)
{
	struct i2c_buff_info *p = &buff_info[bus - 1];

	if (AST_I2C_DMA_SIZE_MAX < dma_size)
		return -1;

	if (0 == dma_size)
		return -1;

	p->dma_size = dma_size;
	p->dma_buf = dma_alloc_coherent(NULL, p->dma_size,
					&p->dma_addr, GFP_DMA | GFP_KERNEL);

	if (!p->dma_buf)
		return -1;

	return 0;
}

/*
** Only allocate dma buff when needed.
*/
static void lazy_alloc_dma_buf(u32 bus_num, u32 bytes_needed)
{
	struct i2c_buff_info *p = &buff_info[bus_num - 1];

	if (NULL == p->dma_buf) {
		if (i2c_dma_alloc(bus_num, bytes_needed))
			BUG();
	} else if (bytes_needed > p->dma_size) {
		i2c_dma_free(p);
		if (i2c_dma_alloc(bus_num, bytes_needed))
			BUG();
	}
}

/*
 * i2c_write_block_dma - write NBytes to I2C in DMA mode
 * @id: I2C bus id
 * @addr: I2C device address
 * @index: register offset
 * @data: pointer to the data buffer
 * @size: number of byte to write
 */
static int i2c_write_block_dma(u32 id, u32 addr, u32 index, const u8 *data, u32 size)
{
	unsigned long flags;
	u32 status, cmd, error = 0;
	struct i2c_buff_info *p;

	if (NUM_I2C_BUS < id)
		return -1;

	p = &buff_info[id - 1];

	lazy_alloc_dma_buf(id, size);

	if (WaitI2CRdy(id, addr, 500))
		return -ENODEV;

	spin_lock_irqsave(&I2CSpinLock[id - 1], flags);

	memcpy((void *)p->dma_buf, data, size);

	status = _i2c_start(id, addr);

	if (-1 == status) {
		error = 1;
		goto OUT;
	}

	/* Wr 'offset' */
	cmd = AST_I2CD_M_TX_CMD;
	i2c_wr(id, index, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	/* Enable DMA */
	cmd = AST_I2CD_M_TX_CMD | AST_I2CD_TX_DMA_ENABLE | AST_I2CD_M_S_RX_CMD_LAST;
	i2c_wr(id, p->dma_addr, I2C_DMA_BASE_REG);
	i2c_wr(id, size, I2C_DMA_LEN_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait TX Done */
	status = wait_status_long(id, S_ACK);
	if (status == -1) {
		error = 1;
		goto STOPI2C;
	}

STOPI2C:
	status = _i2c_stop(id);
	if (-1 == status)
		error = 1;

OUT:
	if (1 == error) {
		printk("GetI2C reg error\n");
		spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);
		return -1;
	}

	barrier();
	i2c_wr(id, i2c_rd(id, I2C_INTR_STS_REG), I2C_INTR_STS_REG);
	barrier();

	spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);

	return 0;
}

/*
 * i2c_read_block_dma - read NBytes from I2C in DMA mode
 * @id: I2C bus id
 * @addr: I2C device address
 * @index: register offset
 * @data: pointer to the data buffer
 * @size: number of byte to read
 */
static int i2c_read_block_dma(u32 id, u32 addr, u32 index, u8 *data, u32 size)
{
	unsigned long flags;
	u32 status, cmd, error = 0;
	struct i2c_buff_info *p;

	if (NUM_I2C_BUS < id)
		return -1;

	p = &buff_info[id - 1];

	lazy_alloc_dma_buf(id, size);

	if (WaitI2CRdy(id, addr, 500))
		return -ENODEV;

	spin_lock_irqsave(&I2CSpinLock[id - 1], flags);

	status = _i2c_start(id, addr);
	if (-1 == status) {
		error = 1;
		goto OUT;
	}

	/* */
	cmd = AST_I2CD_M_TX_CMD;
	i2c_wr(id, index, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	_i2c_stop(id);

	/* Rd 'address', Rd is '1' */
	cmd = AST_I2CD_M_TX_CMD | AST_I2CD_M_START_CMD;
	i2c_wr(id, addr + 1, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	/* Enable DMA */
	cmd = AST_I2CD_M_RX_CMD | AST_I2CD_RX_DMA_ENABLE | AST_I2CD_M_S_RX_CMD_LAST;
	i2c_wr(id, p->dma_addr, I2C_DMA_BASE_REG);
	i2c_wr(id, size, I2C_DMA_LEN_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Rx Done */
	status = wait_status_long(id, S_RX_DONE);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

STOPI2C:
	status = _i2c_stop(id);
	if (-1 == status)
		error = 1;

OUT:
	if (1 == error) {
		printk("GetI2C reg error\n");
		spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);
		return -1;
	}

	barrier();
	i2c_wr(id, i2c_rd(id, I2C_INTR_STS_REG), I2C_INTR_STS_REG);
	barrier();

	memcpy(data, (void *)p->dma_buf, size);

	spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);

	return 0;
}
#else /*  #if defined(I2C_BLOCK_ACCESS_DMA) */
#define AST_I2C_POOL_SIZE_MAX 16	/* AST1520 pool buffer is 16 bytes */
static inline void i2c_pool_free(struct i2c_buff_info *p)
{
	if (p->dma_buf) {
		iounmap(p->dma_buf);
		p->dma_size = 0;
		p->dma_buf = NULL;
	}
}

static int i2c_pool_alloc(u32 bus)
{
	struct i2c_buff_info *p = &buff_info[bus - 1];

	p->dma_size = AST_I2C_POOL_SIZE_MAX;
	p->dma_buf = ioremap_nocache(AST_I2C_BUFFER_BASE + ((bus - 1) * AST_I2C_POOL_SIZE_MAX), AST_I2C_POOL_SIZE_MAX);

	if (!p->dma_buf)
		return -1;

	return 0;
}

/*
 * i2c_read_block_pool - read NBytes from I2C in pool buffer mode
 * @id: I2C bus id
 * @addr: I2C device address
 * @index: register offset
 * @data: pointer to the data buffer
 * @size: number of byte to read
 */
static int i2c_read_block_pool(u32 id, u32 addr, u32 index, u8 *data, u32 size)
{
	unsigned long flags;
	u32 status, cmd, error = 0;
	struct i2c_buff_info *p;
	u32 bytes_to_work, num;
	u8 *pdata;

	if (NUM_I2C_BUS < id)
		return -1;

	p = &buff_info[id - 1];

	if (!p->dma_buf)
		i2c_pool_alloc(id);

	if (WaitI2CRdy(id, addr, 500))
		return -ENODEV;

	spin_lock_irqsave(&I2CSpinLock[id - 1], flags);

	status = _i2c_start(id, addr);
	if (-1 == status) {
		error = 1;
		goto OUT;
	}

	/* */
	cmd = AST_I2CD_M_TX_CMD;
	i2c_wr(id, index, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	/* Rd 'address', Rd is '1' */
	cmd = AST_I2CD_M_TX_CMD | AST_I2CD_M_START_CMD;
	i2c_wr(id, addr + 1, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/* Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	pdata = data;
	num = size;

	while (num) {
		bytes_to_work = (u32) ((num > AST_I2C_POOL_SIZE_MAX) ? AST_I2C_POOL_SIZE_MAX : num);
		/* Enable receive data buffer */
		i2c_wr(id, (bytes_to_work - 1) << 16, I2C_BUF_CTRL_REG);

		cmd = AST_I2CD_M_RX_CMD | AST_I2CD_RX_BUFF_ENABLE;
		if (0 == (num - bytes_to_work))
			cmd |= AST_I2CD_M_S_RX_CMD_LAST;

		barrier();
		i2c_wr(id, cmd, I2C_CMD_REG);

		/* Wait Rx Done */
		status = wait_status_long(id, S_RX_DONE);
		if (-1 == status) {
			error = 1;
			goto STOPI2C;
		}

		barrier();
		memcpy(pdata, (void *)p->dma_buf, bytes_to_work);
		num -= bytes_to_work;
		pdata += bytes_to_work;
	}

STOPI2C:
	status = _i2c_stop(id);
	if (-1 == status)
		error = 1;

OUT:
	if (1 == error) {
		printk("GetI2C reg error\n");
		spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);
		return -1;
	}

	barrier();
	i2c_wr(id, i2c_rd(id, I2C_INTR_STS_REG), I2C_INTR_STS_REG);
	barrier();

	spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);

	return 0;
}

/*
 * i2c_write_block_pool - write NBytes from I2C in pool buffer mode
 * @id: I2C bus id
 * @addr: I2C device address
 * @index: register offset
 * @data: pointer to the data buffer
 * @size: number of byte to write
 */
static int i2c_write_block_pool(u32 id, u32 addr, u32 index, const u8 *data, u32 size)
{
	unsigned long flags;
	u32 status, cmd, error = 0;
	struct i2c_buff_info *p;
	u32 bytes_to_work;
	u32 num = size;
	const u8 *pdata;

	if (NUM_I2C_BUS < id)
		return -1;

	p = &buff_info[id - 1];

	if (!p->dma_buf)
		i2c_pool_alloc(id);

	if (WaitI2CRdy(id, addr, 500))
		return -ENODEV;

	spin_lock_irqsave(&I2CSpinLock[id - 1], flags);

	status = _i2c_start(id, addr);

	if (-1 == status) {
		error = 1;
		goto OUT;
	}

	pdata = (u8 *)data;

	/* Wr 'offset' */
	cmd = AST_I2CD_M_TX_CMD;
	i2c_wr(id, index, I2C_BYTE_BUF_REG);

	barrier();
	i2c_wr(id, cmd, I2C_CMD_REG);

	/*  Wait Tx ACK */
	status = wait_status(id, S_ACK);
	if (-1 == status) {
		error = 1;
		goto STOPI2C;
	}

	while (num) {
		bytes_to_work = (u32) ((num > AST_I2C_POOL_SIZE_MAX) ? AST_I2C_POOL_SIZE_MAX : num);
		memcpy((void *)p->dma_buf, pdata, bytes_to_work);

		barrier();
		num -= bytes_to_work;
		pdata += bytes_to_work;

		/* Enable transmit data buffer */
		i2c_wr(id, (bytes_to_work - 1)  << 8, I2C_BUF_CTRL_REG);
		cmd = AST_I2CD_M_TX_CMD | AST_I2CD_TX_BUFF_ENABLE;

		barrier();
		i2c_wr(id, cmd, I2C_CMD_REG);

		/*  Wait TX Done */
		status = wait_status_long(id, S_ACK);
		if (status == -1) {
			error = 1;
			goto STOPI2C;
		}
	}

STOPI2C:
	status = _i2c_stop(id);
	if (-1 == status)
		error = 1;

OUT:
	if (1 == error) {
		printk("GetI2C reg error\n");
		spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);
		return -1;
	}

	barrier();
	i2c_wr(id, i2c_rd(id, I2C_INTR_STS_REG), I2C_INTR_STS_REG);
	barrier();

	spin_unlock_irqrestore(&I2CSpinLock[id - 1], flags);

	return 0;
}
#endif /* #if defined(I2C_BLOCK_ACCESS_DMA) */
#endif /* #if defined(AST_I2C_BLOCK_ACCESS) */

int GetI2CBlock(u32 DeviceSelect, u32 DeviceAddress, u32 RegisterIndex, u8 *Value, u32 count)
{
#if defined(AST_I2C_BLOCK_ACCESS)
	u32 bytes_to_work;
	int ret;

	while (count) {
#if defined(I2C_BLOCK_ACCESS_DMA)
		bytes_to_work = ((count > AST_I2C_DMA_SIZE_MAX) ? AST_I2C_DMA_SIZE_MAX : count);
		ret = i2c_read_block_dma(DeviceSelect, DeviceAddress, RegisterIndex, Value, bytes_to_work);
#else
		/* the limitation of access size is handled by i2c_read_block_pool() */
		bytes_to_work = count;
		ret = i2c_read_block_pool(DeviceSelect, DeviceAddress, RegisterIndex, Value, bytes_to_work);
#endif
		if (ret)
			return ret;

		count -= bytes_to_work;
		Value += bytes_to_work;
		RegisterIndex += bytes_to_work;
	}
#else
	int i;

	for (i = 0; i < count; i++)
		GetI2CReg(DeviceSelect, DeviceAddress, RegisterIndex + i, Value + i);
#endif
	return 0;
}
EXPORT_SYMBOL(GetI2CBlock);


int SetI2CBlock(u32 DeviceSelect, u32 DeviceAddress, u32 RegisterIndex, const u8 *Value, u32 count)
{
#if defined(AST_I2C_BLOCK_ACCESS)
	u32 bytes_to_work;
	int ret;

	while (count) {
#if defined(I2C_BLOCK_ACCESS_DMA)
		bytes_to_work = ((count > AST_I2C_DMA_SIZE_MAX) ? AST_I2C_DMA_SIZE_MAX : count);
		ret = i2c_write_block_dma(DeviceSelect, DeviceAddress, RegisterIndex, Value, bytes_to_work);
#else
		/* the limitation of access size is handled by i2c_write_block_pool() */
		bytes_to_work = count;
		ret = i2c_write_block_pool(DeviceSelect, DeviceAddress, RegisterIndex, Value, bytes_to_work);
#endif

		if (ret)
			return ret;

		count -= bytes_to_work;
		Value += bytes_to_work;
		RegisterIndex += bytes_to_work;
	}
#else
	int i;

	for (i = 0; i < count; i++)
		SetI2CReg(DeviceSelect, DeviceAddress, RegisterIndex + i, *(Value + i));
#endif
	return 0;
}
EXPORT_SYMBOL(SetI2CBlock);

#if 1
static unsigned int BusNum, DevAddr, Offset = 0, Length = 1;
#else
static unsigned int BusNum, DevAddr, Offset, Value;
#endif

static ssize_t show_io_select(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "Selecting: BusNum:%d DevAddr:0x%02x\n", BusNum, DevAddr);
}

static ssize_t store_io_select(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	/* BusNum counts from 1 to NUM_I2C_BUS */
	sscanf(buf, "%x %x", &BusNum, &DevAddr);
	printk("Select: BusNum:%d DevAddr:0x%02x\n", BusNum, DevAddr);
	return count;
}
DEVICE_ATTR(io_select, (S_IRUGO | S_IWUSR), show_io_select, store_io_select);

static ssize_t show_i_range(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "Selecting: Offset:0x%x Length:0x%x\n", Offset, Length);
}

static ssize_t store_i_range(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int offset, length;
	unsigned int c;

	c = sscanf(buf, "%x %x", &offset, &length);
	if (c == 1) {
		Offset = offset;
		printk("Selecting: Offset:0x%x\n", Offset);
	} else if (c == 2) {
		Offset = offset;
		Length = length;
		printk("Selecting: Offset:0x%x Length:0x%x\n", Offset, Length);
	} else {
		printk("Usage:\nOffset [Length]\n");
	}
	return count;
}
DEVICE_ATTR(i_range, (S_IRUGO | S_IWUSR), show_i_range, store_i_range);

#if 1
static ssize_t show_io_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	int i, num = 0;
	unsigned char byte;

	num += snprintf(buf + num, PAGE_SIZE - num, "Rd Offset:0x%x, Length:0x%x", Offset, Length);

	for (i = 0; i < Length; i++) {
		if ((i % 16) == 0)
			num += snprintf(buf + num, PAGE_SIZE - num, "\n%02X: ", Offset + i);
		else
			num += snprintf(buf + num, PAGE_SIZE - num, " ");

		if (GetI2CReg(BusNum, DevAddr, (Offset + i), &byte)) {
			printk("Rd Offset:0x%X FAILED!\n", Offset + i);
			break;
		}

		num += snprintf(buf + num, PAGE_SIZE - num, "%02X", byte);
	}

	num += snprintf(buf + num, PAGE_SIZE - num, "\n");

	return num;
}

static ssize_t store_io_value(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c, offset, value;

	c = sscanf(buf, "%x %x", &offset, &value);
	if (c == 2) {
		if (SetI2CReg(BusNum, DevAddr, offset, value)) {
			printk("Wr Offset:0x%x FAILED!\n", offset);
			return -1;
		}
		//printk("Wr Offset:0x%x, Value:0x%02x\n", offset, value);
	} else {
		printk("Usage:\nOffset Value\n");
	}
out:
	return count;
}
DEVICE_ATTR(io_value, (S_IRUGO | S_IWUSR), show_io_value, store_io_value);

static ssize_t store_io_buf(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
    int err;

    err = SetI2CBuf(BusNum, DevAddr, buf, count);
    if (err != 0) {
        printk("SetI2CBuf error! count = %d\n", count);
        return -1;
    }
    
out:
    return count;
}
DEVICE_ATTR(io_buf, (S_IRUGO|S_IWUSR), NULL, store_io_buf);
#else
static ssize_t show_io_value(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned char byte;
	if (GetI2CReg(BusNum, DevAddr, Offset, &byte)) {
		printk("Rd Offset:0x%x FAILED!\n", Offset);
		return 0;
	}
	printk("Rd Offset:0x%x, Value:0x%02x\n", Offset, byte);

	return 0;
}

static ssize_t store_io_value(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c;

	c = sscanf(buf, "%x %x", &Offset, &Value);
	if (c == 1) {
		//It is read
		unsigned char byte;
		if (GetI2CReg(BusNum, DevAddr, Offset, &byte)) {
			printk("Rd Offset:0x%x FAILED!\n", Offset);
			goto out;
		}
		printk("Rd Offset:0x%x, Value:0x%02x\n", Offset, byte);
	} else if (c == 2) {
		//It is write
		if (SetI2CReg(BusNum, DevAddr, Offset, Value)) {
			printk("Wr Offset:0x%x FAILED!\n", Offset);
			goto out;
		}
		printk("Wr Offset:0x%x, Value:0x%02x\n", Offset, Value);
	} else {
		printk("Usage:\nWrite: Offset Value\nRead: Offset\n");
	}
out:
	return count;
}
DEVICE_ATTR(io_value, (S_IRUGO | S_IWUSR), show_io_value, store_io_value);

static ssize_t store_io_dump(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c;
	unsigned int i, start, length;
	unsigned char byte;

	c = sscanf(buf, "%x %x", &start, &length);
	if (c == 2) {
		printk("dump 0x%X bytes from 0x%X", length, start);
		for (i = 0; i <= length; i++)
		{
			if ((i % 16) == 0)
				printk("\n%02X	", start + i);
			else
				printk(" ");
			if (GetI2CReg(BusNum, DevAddr, (start + i), &byte)) {
				printk("Rd Offset:0x%X FAILED!\n", start + i);
				break;
			}
			printk("%02X", byte);
		}
		printk("\n");
	} else {
		printk("Usage:\nstart length\n");
	}
	return count;
}
DEVICE_ATTR(io_dump, S_IWUSR, NULL, store_io_dump);
#endif

static ssize_t store_io_word(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c, offset, value;

	c = sscanf(buf, "%x %x", &offset, &value);
	if (c != 0)
	{
#if 0 /* WM8903 has odd offset. */
		if (offset & 1)
		{
			printk("even offset only please\n");
			goto out;
		}
#endif
		if (c == 2)
		{
			printk("Write word at Offset:0x%x to Value:0x%04x\n", offset, value);
			if (SetI2CWord(BusNum, DevAddr, offset, (u16)value)) {
				printk("Write word at Offset:0x%x FAILED!\n", offset);
				goto out;
			}
		}
		else if (c == 1) {
			printk("Read word at Offset:0x%x\n", offset);
			if (GetI2CWord(BusNum, DevAddr, offset, (u16 *)&value)) {
				printk("Read word at Offset:0x%x FAILED!\n", offset);
				goto out;
			}
			printk("Value:0x%04x\n", value);
		}
	}
	else
	{
		printk("Usage:\nOffset (Value)\n");
	}
out:
	return count;
}
DEVICE_ATTR(io_word, S_IWUSR, NULL, store_io_word);

static ssize_t store_d_word(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int c, offset, length, i;
	unsigned short value;

	c = sscanf(buf, "%x %x", &offset, &length);
	if (c == 2)
	{
#if 0 /* WM8903 has odd address. */
		if (offset & 1)
		{
			printk("even offset only please\n");
			goto out;
		}
		if (length & 1)
		{
			printk("even length only please\n");
			goto out;
		}
		for (i = 0; i < (length >> 1); i++, offset += 2)
#endif
		for (i = 0; i < length; i++, offset += 1)
		{
			if (GetI2CWord(BusNum, DevAddr, offset, &value)) {
				printk("Read word at Offset:0x%x FAILED!\n", offset);
				goto out;
			}
			printk("0x%08x:0x%04x\n", offset, value);
		}
	}
	else
	{
		printk("Usage:\nOffset Length\n");
	}
out:
	return count;
}
DEVICE_ATTR(d_word, S_IWUSR, NULL, store_d_word);

static ssize_t show_bus_init(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int i;

	buf[0] = '\0';
	for (i = 0; i < NUM_I2C_BUS; i++)
	{
		sprintf(buf+strlen(buf), "BusNum:%d Speed: %d Hz\n", i + 1, I2CBusSpeed[i]);
	}
	return strlen(buf);
}

static ssize_t store_bus_init(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int bus_num, bus_spd;
	/* BusNum counts from 1 to NUM_I2C_BUS */
	sscanf(buf, "%d %d", &bus_num, &bus_spd);
	printk("Set: I2C Bus %d to speed: %d Hz\n", bus_num, bus_spd);
	I2CInit(bus_num, bus_spd);
	return count;
}
DEVICE_ATTR(bus_init, (S_IRUGO | S_IWUSR), show_bus_init, store_bus_init);


static struct attribute *dev_attrs[] = {
	&dev_attr_io_select.attr,
	&dev_attr_io_value.attr,
	&dev_attr_io_buf.attr,
#if 0
	&dev_attr_io_dump.attr,
#else
	&dev_attr_i_range.attr,
#endif
	&dev_attr_io_word.attr,
	&dev_attr_d_word.attr,
	&dev_attr_bus_init.attr,
	NULL,
};

static struct attribute_group dev_attr_group = {
	.attrs = dev_attrs,
};

#if defined(AST_I2C_BLOCK_ACCESS)
static void i2c_buff_init(void)
{
	memset(buff_info, 0, sizeof(struct i2c_buff_info) * NUM_I2C_BUS);
}

static void i2c_buff_fini(void)
{
	struct i2c_buff_info *p = buff_info;
	int i;

	for (i = 0; i < NUM_I2C_BUS; i++) {
#if defined(I2C_BLOCK_ACCESS_DMA)
		i2c_dma_free(p);
#else
		i2c_pool_free(p);
#endif
		p++;
	}
}
#endif

void I2CInit(u32 DeviceSelect, u32 speed)
{
	if (I2CBusSpeed[DeviceSelect - 1] == 0)//This bus has not been initialized yet.
	{
		printk("Init I2C #%d for speed %d\n", DeviceSelect, speed);
		spin_lock_init(&I2CSpinLock[DeviceSelect - 1]);
		I2CBusSpeed[DeviceSelect - 1] = speed;
		_I2CInit_(DeviceSelect, speed);
	}
	else if (I2CBusSpeed[DeviceSelect - 1] > speed)//Current bus speed is higher than the speed requested. Reinitialize this bus with the request speed..
	{
		unsigned long	flags;
		printk("Reinit I2C #%d for speed %d\n", DeviceSelect, speed);
		spin_lock_irqsave(&I2CSpinLock[DeviceSelect -1], flags);
		I2CBusSpeed[DeviceSelect - 1] = speed;
		_I2CInit_(DeviceSelect, speed);
		spin_unlock_irqrestore(&I2CSpinLock[DeviceSelect -1], flags);
	}
}

EXPORT_SYMBOL(I2CInit);
EXPORT_SYMBOL(IsI2CReady);
EXPORT_SYMBOL(IsI2CReadyLite);
EXPORT_SYMBOL(SetI2CReg);
EXPORT_SYMBOL(SetI2CRegRetry);
EXPORT_SYMBOL(GetI2CReg);
EXPORT_SYMBOL(WaitI2CRdy);
EXPORT_SYMBOL(SetI2CWord);
EXPORT_SYMBOL(GetI2CWord);

static void the_pdev_release(struct device *dev)
{
	return;
}

static struct platform_device pdev = {
	/* should be the same name as driver_name */
	.name = (char *) driver_name,
	.id = -1,
	.dev = {
		.release = the_pdev_release,
		.driver_data = NULL,
	},
};


static int __init I2C_init(void)
{
	int i;
	int ret = 0;

	printk("I2C_init\n");
	//SCUOP_I2C_INIT, 0 means global reset.
	ast_scu.scu_op(SCUOP_I2C_INIT, 0);
	THE_PCLK = ast_scu.PCLK_Hz;
//	THE_PCLK = ast_scu.scu_op(SCUOP_SCU_PCLK_GET, NULL);
//	THE_PCLK = get_PCLK_Hz();
	for (i = 0; i< NUM_I2C_BUS; i++){
#if 0//The driver of each device is responsible to initialize the relevant bus.
		spin_lock_init(&I2CSpinLock[i]);
		I2CInit(i + 1);
#else
		I2CBusSpeed[i] = 0;
#endif
	}

    ret = platform_device_register(&pdev);
    if (ret < 0) {
        printk("platform_device_register err\n");
        goto err_platform_device_register;
    }

    ret = sysfs_create_group(&pdev.dev.kobj, &dev_attr_group);
    if (ret) {
        printk("can't create sysfs files\n");
        goto err_sysfs_create;
    }

#if defined(AST_I2C_BLOCK_ACCESS)
	i2c_buff_init();

	/* SRAM buffer enable. It must be enabled for DMA mode!! */
	i2c_wr(0, 1, 0xC);
#endif
	return 0;

err_sysfs_create:
	platform_device_unregister(&pdev);
err_platform_device_register:
	return ret;
}

static void __exit I2C_exit(void)
{
#if defined(AST_I2C_BLOCK_ACCESS)
	i2c_buff_fini();
#endif
	platform_device_unregister(&pdev);
	printk("I2C_exit\n");
}

#ifndef MODULE
postcore_initcall(I2C_init);
#else
module_init (I2C_init);
#endif
module_exit (I2C_exit);
