#ifndef _ASTSMC_SPI_H_
#define _ASTSMC_SPI_H_

struct astsmc_spi_device {
	u32 phy_dev_base;
	u32 vir_dev_base;
	u32 dev_len;
	u32 tCK_Read;
	u32 tCK_Erase;
	u32 tCK_Write;
};

#endif
