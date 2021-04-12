#include <asm/arch/sizes.h>

#define  I2C_BASE       0x1E78A000
/* Cause U-boot i2c command limitation, it can't assign channel number. Our EEPROM is at channel 3 now*/
/* AST2200's EEPROM is at channel 4 */
#if (CONFIG_AST1500_SOC_VER >= 2)
#define  I2C_CHANNEL    4
#else
#define  I2C_CHANNEL    3
#endif
/* Fix timing for EEPROM 100Khz*/
#define  AC_TIMING      0x77743335
#define  ALL_CLEAR      0xFFFFFFFF
#define  MASTER_ENABLE  0x01
#define  SLAVE_ENABLE   0x02
#define  LOOP_COUNT     0x100000
#define  SCU_BASE       0x1E6E2000
#define  SCU_RESET_CONTROL    0x04
#if (CONFIG_AST1500_SOC_VER >= 2)
#define  SCU_MULTIFUNCTION_PIN_CTL5_REG    0x90
#endif

/* I2C Register */
#define  I2C_FUNCTION_CONTROL_REGISTER    (I2C_BASE + I2C_CHANNEL * 0x40 + 0x00)
#define  I2C_AC_TIMING_REGISTER_1         (I2C_BASE + I2C_CHANNEL * 0x40 + 0x04)
#define  I2C_AC_TIMING_REGISTER_2         (I2C_BASE + I2C_CHANNEL * 0x40 + 0x08)
#define  I2C_INTERRUPT_CONTROL_REGISTER   (I2C_BASE + I2C_CHANNEL * 0x40 + 0x0c)
#define  I2C_INTERRUPT_STATUS_REGISTER    (I2C_BASE + I2C_CHANNEL * 0x40 + 0x10)
#define  I2C_COMMAND_REGISTER             (I2C_BASE + I2C_CHANNEL * 0x40 + 0x14)
#define  I2C_DEVICE_ADDRESS_REGISTER      (I2C_BASE + I2C_CHANNEL * 0x40 + 0x18)
#define  I2C_BUFFER_CONTROL_REGISTER      (I2C_BASE + I2C_CHANNEL * 0x40 + 0x1c)
#define  I2C_BYTE_BUFFER_REGISTER         (I2C_BASE + I2C_CHANNEL * 0x40 + 0x20)
#define  I2C_DMA_CONTROL_REGISTER         (I2C_BASE + I2C_CHANNEL * 0x40 + 0x24)
#define  I2C_DMA_STATUS_REGISTER          (I2C_BASE + I2C_CHANNEL * 0x40 + 0x28)

/* Command Bit */
#define  MASTER_START_COMMAND    (1 << 0)
#define  MASTER_TX_COMMAND       (1 << 1)
#define  MASTER_RX_COMMAND       (1 << 3)
#define  RX_COMMAND_LIST         (1 << 4)
#define  MASTER_STOP_COMMAND     (1 << 5)

/* Interrupt Status Bit */
#define  TX_ACK        (1 << 0)
#define  TX_NACK       (1 << 1)
#define  RX_DONE       (1 << 2)
#define  STOP_DONE     (1 << 4)

/* Macros to access registers */
#define outb(v,p) *(volatile u8 *) (p) = v
#define outw(v,p) *(volatile u16 *) (p) = v
#define outl(v,p) *(volatile u32 *) (p) = v

#define inb(p)	*(volatile u8 *) (p)
#define inw(p)	*(volatile u16 *) (p)
#define inl(p)	*(volatile u32 *) (p)
