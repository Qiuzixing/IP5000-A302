#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ADDR_LEN 6

#define IP_TO_UINT(A, B, C, D) (((D) << 24) + ((C) << 16) + ((B) << 8) + (A))
// --------------------------------------------------------------------
// 	Finds the CRC32 of a set of bytes.
//	Again, from Peter Cammaert's code.
// --------------------------------------------------------------------
static int crc32( char * s, int length ) 
{
	/* indices */
	int perByte;
	int perBit;
	/* crc polynomial for Ethernet */
	const unsigned long poly = 0xedb88320;
	/* crc value - preinitialized to all 1's */
	unsigned long crc_value = 0xffffffff;

	for ( perByte = 0; perByte < length; perByte ++ ) {
		unsigned char	c;

		c = *(s++);
		for ( perBit = 0; perBit < 8; perBit++ ) {
			crc_value = (crc_value>>1)^
				(((crc_value^c)&0x01)?poly:0);
			c >>= 1;
		}
	}
	return	crc_value;
}

int multicast_ip_to_mac(const unsigned int ip, unsigned char *mac)
{
	unsigned char *ip_digit;
	
	ip_digit = (unsigned char *)&ip;
	
	if ((ip_digit[0] & 0xF0) != 0xE0) {
		printf("Invalid multicast IP address!!\n");
		return -1;
	}
	/*
	** Multicast MAC is constructed by fixed higher order 25bits 01:00:5E:0
	** and lower 23bits copied from IP's lower 23bits.
	*/
	mac[0] = 0x01;
	mac[1] = 0x00;
	mac[2] = 0x5E;
	mac[3] = ip_digit[1] & 0x7F;
	mac[4] = ip_digit[2];
	mac[5] = ip_digit[3];
#if 1
	printf("Multicast IP (%d.%d.%d.%d) to MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
			((unsigned char *)&ip)[0], ((unsigned char *)&ip)[1], ((unsigned char *)&ip)[2], ((unsigned char *)&ip)[3],
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
	return 0;
}

int main(void)
{
	unsigned int mip1 = IP_TO_UINT(225, 0, 20, 011);
	unsigned int mip2 = IP_TO_UINT(225, 0, 80, 011);
	unsigned char mmac1[MAX_ADDR_LEN] = { 0x01, 0x00, 0x5e, 0x00, 0x64, 0x0b };
	unsigned char mmac2[MAX_ADDR_LEN] = { 0x01, 0x00, 0x5e, 0x00, 0xa0, 0x0b };
	unsigned int crc_val_1, crc_val_2;

	multicast_ip_to_mac(mip1, mmac1);
	multicast_ip_to_mac(mip2, mmac2);
	crc_val_1 = crc32(mmac1, MAX_ADDR_LEN);
	crc_val_1 = (~(crc_val_1 >> 2)) & 0x3f;
	printf("mac1 hash index = %d\n", crc_val_1);
	
	crc_val_2 = crc32(mmac2, MAX_ADDR_LEN);
	crc_val_2 = (~(crc_val_2 >> 2)) & 0x3f;
	printf("mac2 hash index = %d\n", crc_val_2);

	return 0;
}

