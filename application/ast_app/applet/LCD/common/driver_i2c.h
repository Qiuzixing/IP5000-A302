#ifndef __DRIVER_I2C_H__
#define __DRIVER_I2C_H__


#define u8 unsigned char 
#define u16 unsigned short
#define u32 unsigned int

#ifdef __cplusplus
extern "C"{
#endif

int i2c_init(unsigned int  i2c_bus_num, unsigned int i2c_freq, u8 i2c_addr, int enable);
int i2c_write_one_byte(u8 reg, u8 data);
int i2c_write_multi_byte(u8 *data, u8 size);


#ifdef __cplusplus
}
#endif

#endif

