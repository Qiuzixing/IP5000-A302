#define	SCU_BASE	0x1E6E2000
#define	MULTIFUCTION_PIN_CONTROL_5	0x90
#define	APB_BRIDGE_2_BASE	0x1E780000
#define	I2C_BASE			0xA000
#define	NUM_I2C_BUS			7 //bus num count from 1 to 7
#if (CONFIG_AST1500_SOC_VER == 1)
//#define	AC_TIMING			0x77743335 //about 100KHz
#define	AC_TIMING			0x77777736 //lower about half of speed
#elif (CONFIG_AST1500_SOC_VER >= 2)
//#define	AC_TIMING			0x77754735 //27MHz(PCLK) / ((2^5) * ((5 + 1) + (4 + 1))) ~= 77KHz
#define	AC_TIMING			0x77754736 //27MHz(PCLK) / ((2^6) * ((5 + 1) + (4 + 1))) ~= 38KHz
#else
#error "I2C AC_TIMING not defined!"
#endif

#if (3 <= CONFIG_AST1500_SOC_VER)
#define AST_I2C_BLOCK_ACCESS

#define I2C_TIMEOUT_LONG	50000 /* FIXME we need ~50ms to read 128 byte @100KHz */
#define wait_status_long(dev, wait)	_wait_status(dev, wait, I2C_TIMEOUT_LONG)

#define I2C_FUN_CTRL_REG	0x00
#define I2C_AC_TIMING_REG1	0x04
#define I2C_AC_TIMING_REG2	0x08
#define I2C_INTR_CTRL_REG	0x0C
#define I2C_INTR_STS_REG	0x10
#define I2C_CMD_REG		0x14
#define I2C_DEV_ADDR_REG	0x18
#define I2C_BUF_CTRL_REG	0x1C
#define I2C_BYTE_BUF_REG	0x20
#define I2C_DMA_BASE_REG	0x24
#define I2C_DMA_LEN_REG		0x28

#define AST_I2CD_SCL_LINE_STS		(0x1 << 18)
#define AST_I2CD_SDA_LINE_STS		(0x1 << 17)
#define AST_I2CD_BUS_BUSY_STS		(0x1 << 16)
#define AST_I2CD_SDA_OE_OUT_DIR		(0x1 << 15)
#define AST_I2CD_SDA_O_OUT_DIR		(0x1 << 14)
#define AST_I2CD_SCL_OE_OUT_DIR		(0x1 << 13)
#define AST_I2CD_SCL_O_OUT_DIR		(0x1 << 12)
#define AST_I2CD_BUS_RECOVER_CMD_EN	(0x1 << 11)
#define AST_I2CD_S_ALT_EN		(0x1 << 10)
#define AST_I2CD_RX_DMA_ENABLE		(0x1 << 9)
#define AST_I2CD_TX_DMA_ENABLE		(0x1 << 8)
#define AST_I2CD_RX_BUFF_ENABLE		(0x1 << 7)
#define AST_I2CD_TX_BUFF_ENABLE		(0x1 << 6)
#define AST_I2CD_M_STOP_CMD		(0x1 << 5)
#define AST_I2CD_M_S_RX_CMD_LAST	(0x1 << 4)
#define AST_I2CD_M_RX_CMD		(0x1 << 3)
#define AST_I2CD_S_TX_CMD		(0x1 << 2)
#define AST_I2CD_M_TX_CMD		(0x1 << 1)
#define AST_I2CD_M_START_CMD		(0x1)

#define AST_I2C_BUFFER_BASE    (APB_BRIDGE_2_BASE + I2C_BASE + 0x200)
#define i2c_wr(bus_id, value, offset)	WriteMemoryLong(APB_BRIDGE_2_BASE + I2C_BASE + (bus_id) * 0x40, offset, value);
#define i2c_rd(bus_id, offset)		ReadMemoryLong(APB_BRIDGE_2_BASE + I2C_BASE + (bus_id) * 0x40, offset)
#endif /* #if (3 <= CONFIG_AST1500_SOC_VER) */
