/*
** This file defines standard JPEG header format and related data used by VE.
*/

#ifndef __JPEG_H__
#define __JPEG_H__


#define HEADER_DW 160
#define JPG_HDR_SIZE (HEADER_DW << 2) //in bytes

struct jpeg_header_t {
	u16 mkr_soi; //start of image. 0xD8FF
	struct jfif_header_t {
		u16 mkr_app0; //jfif app0. 0xE0FF
		u16 jfif_hdr_len; //0x1000 //big endien
		u8  jfif_id[5]; //JFIF. 0x4A,0x46,0x49,0x46,0x00
		u16 jfif_ver; //v1.01 0x0101
		u8  jfif_units; // 0x01: dots per inch
		u16 jfif_x_density; //0x6000?
		u16 jfif_y_density; //0x6000?
		u8  jfif_x_thumbnail; //0x00?
		u8  jfif_y_thumbnail; //0x00?
	} __attribute__ ((packed)) jfif_hdr;
	struct comment_t {
		u16 mkr_com; //comment. 0xFEFF
		u16 len; //0x0F00 //big endien
		u8  string[13]; //null end.
	} __attribute__ ((packed)) comment;
	struct dqt_1_t {
		u16 mkr_dqt; //DQT. 0xDBFF
		u16 len; //0x4300. //big endien. 67bytes
		u8 qt[65];
	} __attribute__ ((packed)) qdt_1; // DQT. Define quantization table
	struct dqt_2_t {
		u16 mkr_dqt; //DQT. 0xDBFF
		u16 len; //0x4300. //big endien. 67bytes
		u8 qt[65];
	} __attribute__ ((packed)) qdt_2; // DQT. Define quantization table
	struct sof0_t {
		u16 mkr_sof0; //SOF0. 0xC0FF
		u16 hdr_len; //header length. 0x1100. big endien
		u8  P; //0x08
		u16 Y; //0x0000 //To be updated.
		u16 X; //0x0000 //To be updated.
		u8  Nf; //0x03
		struct sof_cp_t {
			u8  Ci;
			u8  Hi:4;
			u8  Vi:4;
			u8  Tqi;
		} __attribute__ ((packed)) cp[3]; // 3x3 bytes
	} __attribute__ ((packed)) sof0;
	struct dht_1_t {
		u16 mkr_dht; //DHT. 0xC4FF
		u16 len; // 0x1F00 big endien
		u8  ht[29];
	} __attribute__ ((packed)) dht1; //DHT. Define huffman table
	struct dht_2_t {
		u16 mkr_dht; //DHT. 0xC4FF
		u16 len; // 0xB500 big endien. 181 bytes
		u8  ht[179];
	} __attribute__ ((packed)) dht2; //DHT. Define huffman table

	struct dht_3_t {
		u16 mkr_dht; //DHT. 0xC4FF
		u16 len; // 0x1F00 big endien
		u8  ht[29];
	} __attribute__ ((packed)) dht3; //DHT. Define huffman table

	struct dht_4_t {
		u16 mkr_dht; //DHT. 0xC4FF
		u16 len; // 0xB500 big endien 181 bytes
		u8  ht[179];
	} __attribute__ ((packed)) dht4; //DHT. Define huffman table

	struct sos_t {
		u16 mkr_sos; //SOS. Start of scan. 0xDAFF
		u16 len; //0x0C00 big endien. 12 bytes.
		u8  Ns; //0x03
		struct sos_cp_t {
			u8 Cs;
			u8 Td:4;
			u8 Ta:4;
		} __attribute__ ((packed)) cp[3]; // 2x3=6 bytes
		u8 Ss; //0x00
		u8 Se; //0x3F
		u8 Ah:4; //0x0
		u8 Al:4; //0x0
	} __attribute__ ((packed)) sos; //SOS. Start of scan
} __attribute__ ((packed));

struct jpeg_header_t jpeg_hdr_tmp;

static inline void init_default_jpeg_table(struct jpeg_header_t *jpg_buf, int is_420)
{
	unsigned int *jpg_hdr = (unsigned int *)jpg_buf;

	//JPEG header default value:
	jpg_hdr[ 0] = 0xE0FFD8FF;
	jpg_hdr[ 1] = 0x464A1000;
	jpg_hdr[ 2] = 0x01004649;
	jpg_hdr[ 3] = 0x60000101;
	jpg_hdr[ 4] = 0x00006000;
	jpg_hdr[ 5] = 0x0F00FEFF;
	jpg_hdr[ 6] = 0x00002D05;
	jpg_hdr[ 7] = 0x00000000;
	jpg_hdr[ 8] = 0x00000000;
	jpg_hdr[ 9] = 0x00DBFF00;
	jpg_hdr[44] = 0x081100C0;
	jpg_hdr[45] = 0x00000000;
	if (is_420) {
		jpg_hdr[46] = 0x00220103; //for YUV420 mode
	} else {
		jpg_hdr[46] = 0x00110103; //for YUV444 mode
	}
	jpg_hdr[47] = 0x03011102;
	jpg_hdr[48] = 0xC4FF0111;
	jpg_hdr[49] = 0x00001F00;
	jpg_hdr[50] = 0x01010501;
	jpg_hdr[51] = 0x01010101;
	jpg_hdr[52] = 0x00000000;
	jpg_hdr[53] = 0x00000000;
	jpg_hdr[54] = 0x04030201;
	jpg_hdr[55] = 0x08070605;
	jpg_hdr[56] = 0xFF0B0A09;
	jpg_hdr[57] = 0x10B500C4;
	jpg_hdr[58] = 0x03010200;
	jpg_hdr[59] = 0x03040203;
	jpg_hdr[60] = 0x04040505;
	jpg_hdr[61] = 0x7D010000;
	jpg_hdr[62] = 0x00030201;
	jpg_hdr[63] = 0x12051104;
	jpg_hdr[64] = 0x06413121;
	jpg_hdr[65] = 0x07615113;
	jpg_hdr[66] = 0x32147122;
	jpg_hdr[67] = 0x08A19181;
	jpg_hdr[68] = 0xC1B14223;
	jpg_hdr[69] = 0xF0D15215;
	jpg_hdr[70] = 0x72623324;
	jpg_hdr[71] = 0x160A0982;
	jpg_hdr[72] = 0x1A191817;
	jpg_hdr[73] = 0x28272625;
	jpg_hdr[74] = 0x35342A29;
	jpg_hdr[75] = 0x39383736;
	jpg_hdr[76] = 0x4544433A;
	jpg_hdr[77] = 0x49484746;
	jpg_hdr[78] = 0x5554534A;
	jpg_hdr[79] = 0x59585756;
	jpg_hdr[80] = 0x6564635A;
	jpg_hdr[81] = 0x69686766;
	jpg_hdr[82] = 0x7574736A;
	jpg_hdr[83] = 0x79787776;
	jpg_hdr[84] = 0x8584837A;
	jpg_hdr[85] = 0x89888786;
	jpg_hdr[86] = 0x9493928A;
	jpg_hdr[87] = 0x98979695;
	jpg_hdr[88] = 0xA3A29A99;
	jpg_hdr[89] = 0xA7A6A5A4;
	jpg_hdr[90] = 0xB2AAA9A8;
	jpg_hdr[91] = 0xB6B5B4B3;
	jpg_hdr[92] = 0xBAB9B8B7;
	jpg_hdr[93] = 0xC5C4C3C2;
	jpg_hdr[94] = 0xC9C8C7C6;
	jpg_hdr[95] = 0xD4D3D2CA;
	jpg_hdr[96] = 0xD8D7D6D5;
	jpg_hdr[97] = 0xE2E1DAD9;
	jpg_hdr[98] = 0xE6E5E4E3;
	jpg_hdr[99] = 0xEAE9E8E7;
	jpg_hdr[100] = 0xF4F3F2F1;
	jpg_hdr[101] = 0xF8F7F6F5;
	jpg_hdr[102] = 0xC4FFFAF9;
	jpg_hdr[103] = 0x00011F00;
	jpg_hdr[104] = 0x01010103;
	jpg_hdr[105] = 0x01010101;
	jpg_hdr[106] = 0x00000101;
	jpg_hdr[107] = 0x00000000;
	jpg_hdr[108] = 0x04030201;
	jpg_hdr[109] = 0x08070605;
	jpg_hdr[110] = 0xFF0B0A09;
	jpg_hdr[111] = 0x11B500C4;
	jpg_hdr[112] = 0x02010200;
	jpg_hdr[113] = 0x04030404;
	jpg_hdr[114] = 0x04040507;
	jpg_hdr[115] = 0x77020100;
	jpg_hdr[116] = 0x03020100;
	jpg_hdr[117] = 0x21050411;
	jpg_hdr[118] = 0x41120631;
	jpg_hdr[119] = 0x71610751;
	jpg_hdr[120] = 0x81322213;
	jpg_hdr[121] = 0x91421408;
	jpg_hdr[122] = 0x09C1B1A1;
	jpg_hdr[123] = 0xF0523323;
	jpg_hdr[124] = 0xD1726215;
	jpg_hdr[125] = 0x3424160A;
	jpg_hdr[126] = 0x17F125E1;
	jpg_hdr[127] = 0x261A1918;
	jpg_hdr[128] = 0x2A292827;
	jpg_hdr[129] = 0x38373635;
	jpg_hdr[130] = 0x44433A39;
	jpg_hdr[131] = 0x48474645;
	jpg_hdr[132] = 0x54534A49;
	jpg_hdr[133] = 0x58575655;
	jpg_hdr[134] = 0x64635A59;
	jpg_hdr[135] = 0x68676665;
	jpg_hdr[136] = 0x74736A69;
	jpg_hdr[137] = 0x78777675;
	jpg_hdr[138] = 0x83827A79;
	jpg_hdr[139] = 0x87868584;
	jpg_hdr[140] = 0x928A8988;
	jpg_hdr[141] = 0x96959493;
	jpg_hdr[142] = 0x9A999897;
	jpg_hdr[143] = 0xA5A4A3A2;
	jpg_hdr[144] = 0xA9A8A7A6;
	jpg_hdr[145] = 0xB4B3B2AA;
	jpg_hdr[146] = 0xB8B7B6B5;
	jpg_hdr[147] = 0xC3C2BAB9;
	jpg_hdr[148] = 0xC7C6C5C4;
	jpg_hdr[149] = 0xD2CAC9C8;
	jpg_hdr[150] = 0xD6D5D4D3;
	jpg_hdr[151] = 0xDAD9D8D7;
	jpg_hdr[152] = 0xE5E4E3E2;
	jpg_hdr[153] = 0xE9E8E7E6;
	jpg_hdr[154] = 0xF4F3F2EA;
	jpg_hdr[155] = 0xF8F7F6F5;
	jpg_hdr[156] = 0xDAFFFAF9;
	jpg_hdr[157] = 0x01030C00;
	jpg_hdr[158] = 0x03110200;
	jpg_hdr[159] = 0x003F0011;
}

static inline void init_jpeg_dct_table(struct jpeg_header_t *jpg_buf, int y, int uv)
{
	unsigned int *jpg_hdr = (unsigned int *)jpg_buf;

	switch (y) {
	case 0:
        jpg_hdr[10] = 0x0D140043;
        jpg_hdr[11] = 0x0C0F110F;
        jpg_hdr[12] = 0x11101114;
        jpg_hdr[13] = 0x17141516;
        jpg_hdr[14] = 0x1E20321E;
        jpg_hdr[15] = 0x3D1E1B1B;
        jpg_hdr[16] = 0x32242E2B;
        jpg_hdr[17] = 0x4B4C3F48;
        jpg_hdr[18] = 0x44463F47;
        jpg_hdr[19] = 0x61735A50;
        jpg_hdr[20] = 0x566C5550;
        jpg_hdr[21] = 0x88644644;
        jpg_hdr[22] = 0x7A766C65;
        jpg_hdr[23] = 0x4D808280;
        jpg_hdr[24] = 0x8C978D60;
        jpg_hdr[25] = 0x7E73967D;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00007B80;
		break;
	case 1:
        jpg_hdr[10] = 0x0C110043;
        jpg_hdr[11] = 0x0A0D0F0D;
        jpg_hdr[12] = 0x0F0E0F11;
        jpg_hdr[13] = 0x14111213;
        jpg_hdr[14] = 0x1A1C2B1A;
        jpg_hdr[15] = 0x351A1818;
        jpg_hdr[16] = 0x2B1F2826;
        jpg_hdr[17] = 0x4142373F;
        jpg_hdr[18] = 0x3C3D373E;
        jpg_hdr[19] = 0x55644E46;
        jpg_hdr[20] = 0x4B5F4A46;
        jpg_hdr[21] = 0x77573D3C;
        jpg_hdr[22] = 0x6B675F58;
        jpg_hdr[23] = 0x43707170;
        jpg_hdr[24] = 0x7A847B54;
        jpg_hdr[25] = 0x6E64836D;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00006C70;
		break;
	case 2:
        jpg_hdr[10] = 0x090E0043;
        jpg_hdr[11] = 0x090A0C0A;
        jpg_hdr[12] = 0x0C0B0C0E;
        jpg_hdr[13] = 0x110E0F10;
        jpg_hdr[14] = 0x15172415;
        jpg_hdr[15] = 0x2C151313;
        jpg_hdr[16] = 0x241A211F;
        jpg_hdr[17] = 0x36372E34;
        jpg_hdr[18] = 0x31322E33;
        jpg_hdr[19] = 0x4653413A;
        jpg_hdr[20] = 0x3E4E3D3A;
        jpg_hdr[21] = 0x62483231;
        jpg_hdr[22] = 0x58564E49;
        jpg_hdr[23] = 0x385D5E5D;
        jpg_hdr[24] = 0x656D6645;
        jpg_hdr[25] = 0x5B536C5A;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x0000595D;
		break;
	case 3:
        jpg_hdr[10] = 0x070B0043;
        jpg_hdr[11] = 0x07080A08;
        jpg_hdr[12] = 0x0A090A0B;
        jpg_hdr[13] = 0x0D0B0C0C;
        jpg_hdr[14] = 0x11121C11;
        jpg_hdr[15] = 0x23110F0F;
        jpg_hdr[16] = 0x1C141A19;
        jpg_hdr[17] = 0x2B2B2429;
        jpg_hdr[18] = 0x27282428;
        jpg_hdr[19] = 0x3842332E;
        jpg_hdr[20] = 0x313E302E;
        jpg_hdr[21] = 0x4E392827;
        jpg_hdr[22] = 0x46443E3A;
        jpg_hdr[23] = 0x2C4A4A4A;
        jpg_hdr[24] = 0x50565137;
        jpg_hdr[25] = 0x48425647;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x0000474A;
		break;
	case 4:
        jpg_hdr[10] = 0x06090043;
        jpg_hdr[11] = 0x05060706;
        jpg_hdr[12] = 0x07070709;
        jpg_hdr[13] = 0x0A09090A;
        jpg_hdr[14] = 0x0D0E160D;
        jpg_hdr[15] = 0x1B0D0C0C;
        jpg_hdr[16] = 0x16101413;
        jpg_hdr[17] = 0x21221C20;
        jpg_hdr[18] = 0x1E1F1C20;
        jpg_hdr[19] = 0x2B332824;
        jpg_hdr[20] = 0x26302624;
        jpg_hdr[21] = 0x3D2D1F1E;
        jpg_hdr[22] = 0x3735302D;
        jpg_hdr[23] = 0x22393A39;
        jpg_hdr[24] = 0x3F443F2B;
        jpg_hdr[25] = 0x38334338;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00003739;
		break;
	case 5:
        jpg_hdr[10] = 0x04060043;
        jpg_hdr[11] = 0x03040504;
        jpg_hdr[12] = 0x05040506;
        jpg_hdr[13] = 0x07060606;
        jpg_hdr[14] = 0x09090F09;
        jpg_hdr[15] = 0x12090808;
        jpg_hdr[16] = 0x0F0A0D0D;
        jpg_hdr[17] = 0x16161315;
        jpg_hdr[18] = 0x14151315;
        jpg_hdr[19] = 0x1D221B18;
        jpg_hdr[20] = 0x19201918;
        jpg_hdr[21] = 0x281E1514;
        jpg_hdr[22] = 0x2423201E;
        jpg_hdr[23] = 0x17262726;
        jpg_hdr[24] = 0x2A2D2A1C;
        jpg_hdr[25] = 0x25222D25;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00002526;
		break;
	case 6:
        jpg_hdr[10] = 0x02030043;
        jpg_hdr[11] = 0x01020202;
        jpg_hdr[12] = 0x02020203;
        jpg_hdr[13] = 0x03030303;
        jpg_hdr[14] = 0x04040704;
        jpg_hdr[15] = 0x09040404;
        jpg_hdr[16] = 0x07050606;
        jpg_hdr[17] = 0x0B0B090A;
        jpg_hdr[18] = 0x0A0A090A;
        jpg_hdr[19] = 0x0E110D0C;
        jpg_hdr[20] = 0x0C100C0C;
        jpg_hdr[21] = 0x140F0A0A;
        jpg_hdr[22] = 0x1211100F;
        jpg_hdr[23] = 0x0B131313;
        jpg_hdr[24] = 0x1516150E;
        jpg_hdr[25] = 0x12111612;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00001213;
		break;
	case 7:
        jpg_hdr[10] = 0x01020043;
        jpg_hdr[11] = 0x01010101;
        jpg_hdr[12] = 0x01010102;
        jpg_hdr[13] = 0x02020202;
        jpg_hdr[14] = 0x03030503;
        jpg_hdr[15] = 0x06030202;
        jpg_hdr[16] = 0x05030404;
        jpg_hdr[17] = 0x07070607;
        jpg_hdr[18] = 0x06070607;
        jpg_hdr[19] = 0x090B0908;
        jpg_hdr[20] = 0x080A0808;
        jpg_hdr[21] = 0x0D0A0706;
        jpg_hdr[22] = 0x0C0B0A0A;
        jpg_hdr[23] = 0x070C0D0C;
        jpg_hdr[24] = 0x0E0F0E09;
        jpg_hdr[25] = 0x0C0B0F0C;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00000C0C;
		break;
	case 8:
        jpg_hdr[10] = 0x01020043;
        jpg_hdr[11] = 0x01010101;
        jpg_hdr[12] = 0x01010102;
        jpg_hdr[13] = 0x02020202;
        jpg_hdr[14] = 0x03030503;
        jpg_hdr[15] = 0x06030202;
        jpg_hdr[16] = 0x05030404;
        jpg_hdr[17] = 0x07070607;
        jpg_hdr[18] = 0x06070607;
        jpg_hdr[19] = 0x090B0908;
        jpg_hdr[20] = 0x080A0808;
        jpg_hdr[21] = 0x0D0A0706;
        jpg_hdr[22] = 0x0C0B0A0A;
        jpg_hdr[23] = 0x070C0D0C;
        jpg_hdr[24] = 0x0E0F0E09;
        jpg_hdr[25] = 0x0C0B0F0C;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00000C0C;
		break;
	case 9:
        jpg_hdr[10] = 0x01010043;
        jpg_hdr[11] = 0x01010101;
        jpg_hdr[12] = 0x01010101;
        jpg_hdr[13] = 0x01010101;
        jpg_hdr[14] = 0x02020302;
        jpg_hdr[15] = 0x04020202;
        jpg_hdr[16] = 0x03020303;
        jpg_hdr[17] = 0x05050405;
        jpg_hdr[18] = 0x05050405;
        jpg_hdr[19] = 0x07080606;
        jpg_hdr[20] = 0x06080606;
        jpg_hdr[21] = 0x0A070505;
        jpg_hdr[22] = 0x09080807;
        jpg_hdr[23] = 0x05090909;
        jpg_hdr[24] = 0x0A0B0A07;
        jpg_hdr[25] = 0x09080B09;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00000909;
		break;
	case 10:
        jpg_hdr[10] = 0x01010043;
        jpg_hdr[11] = 0x01010101;
        jpg_hdr[12] = 0x01010101;
        jpg_hdr[13] = 0x01010101;
        jpg_hdr[14] = 0x01010201;
        jpg_hdr[15] = 0x03010101;
        jpg_hdr[16] = 0x02010202;
        jpg_hdr[17] = 0x03030303;
        jpg_hdr[18] = 0x03030303;
        jpg_hdr[19] = 0x04050404;
        jpg_hdr[20] = 0x04050404;
        jpg_hdr[21] = 0x06050303;
        jpg_hdr[22] = 0x06050505;
        jpg_hdr[23] = 0x03060606;
        jpg_hdr[24] = 0x07070704;
        jpg_hdr[25] = 0x06050706;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00000606;
		break;
	case 11:
        jpg_hdr[10] = 0x01010043;
        jpg_hdr[11] = 0x01010101;
        jpg_hdr[12] = 0x01010101;
        jpg_hdr[13] = 0x01010101;
        jpg_hdr[14] = 0x01010101;
        jpg_hdr[15] = 0x01010101;
        jpg_hdr[16] = 0x01010101;
        jpg_hdr[17] = 0x01010101;
        jpg_hdr[18] = 0x01010101;
        jpg_hdr[19] = 0x02020202;
        jpg_hdr[20] = 0x02020202;
        jpg_hdr[21] = 0x03020101;
        jpg_hdr[22] = 0x03020202;
        jpg_hdr[23] = 0x01030303;
        jpg_hdr[24] = 0x03030302;
        jpg_hdr[25] = 0x03020303;
        jpg_hdr[26] &= 0xFFFF0000;
        jpg_hdr[26] |= 0x00000403;
		break;
	};


	switch (uv) {
	case 0:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x1F014300;
        jpg_hdr[28] = 0x272D2121;
        jpg_hdr[29] = 0x3030582D;
        jpg_hdr[30] = 0x697BB958;
        jpg_hdr[31] = 0xB8B9B97B;
        jpg_hdr[32] = 0xB9B8A6A6;
        jpg_hdr[33] = 0xB9B9B9B9;
        jpg_hdr[34] = 0xB9B9B9B9;
        jpg_hdr[35] = 0xB9B9B9B9;
        jpg_hdr[36] = 0xB9B9B9B9;
        jpg_hdr[37] = 0xB9B9B9B9;
        jpg_hdr[38] = 0xB9B9B9B9;
        jpg_hdr[39] = 0xB9B9B9B9;
        jpg_hdr[40] = 0xB9B9B9B9;
        jpg_hdr[41] = 0xB9B9B9B9;
        jpg_hdr[42] = 0xB9B9B9B9;
        jpg_hdr[43] = 0xFFB9B9B9;
		break;
	case 1:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x1B014300;
        jpg_hdr[28] = 0x22271D1D;
        jpg_hdr[29] = 0x2A2A4C27;
        jpg_hdr[30] = 0x5B6BA04C;
        jpg_hdr[31] = 0xA0A0A06B;
        jpg_hdr[32] = 0xA0A0A0A0;
        jpg_hdr[33] = 0xA0A0A0A0;
        jpg_hdr[34] = 0xA0A0A0A0;
        jpg_hdr[35] = 0xA0A0A0A0;
        jpg_hdr[36] = 0xA0A0A0A0;
        jpg_hdr[37] = 0xA0A0A0A0;
        jpg_hdr[38] = 0xA0A0A0A0;
        jpg_hdr[39] = 0xA0A0A0A0;
        jpg_hdr[40] = 0xA0A0A0A0;
        jpg_hdr[41] = 0xA0A0A0A0;
        jpg_hdr[42] = 0xA0A0A0A0;
        jpg_hdr[43] = 0xFFA0A0A0;
		break;
	case 2:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x16014300;
        jpg_hdr[28] = 0x1C201818;
        jpg_hdr[29] = 0x22223F20;
        jpg_hdr[30] = 0x4B58853F;
        jpg_hdr[31] = 0x85858558;
        jpg_hdr[32] = 0x85858585;
        jpg_hdr[33] = 0x85858585;
        jpg_hdr[34] = 0x85858585;
        jpg_hdr[35] = 0x85858585;
        jpg_hdr[36] = 0x85858585;
        jpg_hdr[37] = 0x85858585;
        jpg_hdr[38] = 0x85858585;
        jpg_hdr[39] = 0x85858585;
        jpg_hdr[40] = 0x85858585;
        jpg_hdr[41] = 0x85858585;
        jpg_hdr[42] = 0x85858585;
        jpg_hdr[43] = 0xFF858585;
		break;
	case 3:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x12014300;
        jpg_hdr[28] = 0x161A1313;
        jpg_hdr[29] = 0x1C1C331A;
        jpg_hdr[30] = 0x3D486C33;
        jpg_hdr[31] = 0x6C6C6C48;
        jpg_hdr[32] = 0x6C6C6C6C;
        jpg_hdr[33] = 0x6C6C6C6C;
        jpg_hdr[34] = 0x6C6C6C6C;
        jpg_hdr[35] = 0x6C6C6C6C;
        jpg_hdr[36] = 0x6C6C6C6C;
        jpg_hdr[37] = 0x6C6C6C6C;
        jpg_hdr[38] = 0x6C6C6C6C;
        jpg_hdr[39] = 0x6C6C6C6C;
        jpg_hdr[40] = 0x6C6C6C6C;
        jpg_hdr[41] = 0x6C6C6C6C;
        jpg_hdr[42] = 0x6C6C6C6C;
        jpg_hdr[43] = 0xFF6C6C6C;
		break;
	case 4:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x0D014300;
        jpg_hdr[28] = 0x11130E0E;
        jpg_hdr[29] = 0x15152613;
        jpg_hdr[30] = 0x2D355026;
        jpg_hdr[31] = 0x50505035;
        jpg_hdr[32] = 0x50505050;
        jpg_hdr[33] = 0x50505050;
        jpg_hdr[34] = 0x50505050;
        jpg_hdr[35] = 0x50505050;
        jpg_hdr[36] = 0x50505050;
        jpg_hdr[37] = 0x50505050;
        jpg_hdr[38] = 0x50505050;
        jpg_hdr[39] = 0x50505050;
        jpg_hdr[40] = 0x50505050;
        jpg_hdr[41] = 0x50505050;
        jpg_hdr[42] = 0x50505050;
        jpg_hdr[43] = 0xFF505050;
		break;
	case 5:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x09014300;
        jpg_hdr[28] = 0x0B0D0A0A;
        jpg_hdr[29] = 0x0E0E1A0D;
        jpg_hdr[30] = 0x1F25371A;
        jpg_hdr[31] = 0x37373725;
        jpg_hdr[32] = 0x37373737;
        jpg_hdr[33] = 0x37373737;
        jpg_hdr[34] = 0x37373737;
        jpg_hdr[35] = 0x37373737;
        jpg_hdr[36] = 0x37373737;
        jpg_hdr[37] = 0x37373737;
        jpg_hdr[38] = 0x37373737;
        jpg_hdr[39] = 0x37373737;
        jpg_hdr[40] = 0x37373737;
        jpg_hdr[41] = 0x37373737;
        jpg_hdr[42] = 0x37373737;
        jpg_hdr[43] = 0xFF373737;
		break;
	case 6:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x04014300;
        jpg_hdr[28] = 0x05060505;
        jpg_hdr[29] = 0x07070D06;
        jpg_hdr[30] = 0x0F121B0D;
        jpg_hdr[31] = 0x1B1B1B12;
        jpg_hdr[32] = 0x1B1B1B1B;
        jpg_hdr[33] = 0x1B1B1B1B;
        jpg_hdr[34] = 0x1B1B1B1B;
        jpg_hdr[35] = 0x1B1B1B1B;
        jpg_hdr[36] = 0x1B1B1B1B;
        jpg_hdr[37] = 0x1B1B1B1B;
        jpg_hdr[38] = 0x1B1B1B1B;
        jpg_hdr[39] = 0x1B1B1B1B;
        jpg_hdr[40] = 0x1B1B1B1B;
        jpg_hdr[41] = 0x1B1B1B1B;
        jpg_hdr[42] = 0x1B1B1B1B;
        jpg_hdr[43] = 0xFF1B1B1B;
		break;
	case 7:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x03014300;
        jpg_hdr[28] = 0x03040303;
        jpg_hdr[29] = 0x04040804;
        jpg_hdr[30] = 0x0A0C1208;
        jpg_hdr[31] = 0x1212120C;
        jpg_hdr[32] = 0x12121212;
        jpg_hdr[33] = 0x12121212;
        jpg_hdr[34] = 0x12121212;
        jpg_hdr[35] = 0x12121212;
        jpg_hdr[36] = 0x12121212;
        jpg_hdr[37] = 0x12121212;
        jpg_hdr[38] = 0x12121212;
        jpg_hdr[39] = 0x12121212;
        jpg_hdr[40] = 0x12121212;
        jpg_hdr[41] = 0x12121212;
        jpg_hdr[42] = 0x12121212;
        jpg_hdr[43] = 0xFF121212;
		break;
	case 8:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x02014300;
        jpg_hdr[28] = 0x03030202;
        jpg_hdr[29] = 0x04040703;
        jpg_hdr[30] = 0x080A0F07;
        jpg_hdr[31] = 0x0F0F0F0A;
        jpg_hdr[32] = 0x0F0F0F0F;
        jpg_hdr[33] = 0x0F0F0F0F;
        jpg_hdr[34] = 0x0F0F0F0F;
        jpg_hdr[35] = 0x0F0F0F0F;
        jpg_hdr[36] = 0x0F0F0F0F;
        jpg_hdr[37] = 0x0F0F0F0F;
        jpg_hdr[38] = 0x0F0F0F0F;
        jpg_hdr[39] = 0x0F0F0F0F;
        jpg_hdr[40] = 0x0F0F0F0F;
        jpg_hdr[41] = 0x0F0F0F0F;
        jpg_hdr[42] = 0x0F0F0F0F;
        jpg_hdr[43] = 0xFF0F0F0F;
		break;
	case 9:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x02014300;
        jpg_hdr[28] = 0x02030202;
        jpg_hdr[29] = 0x03030503;
        jpg_hdr[30] = 0x07080C05;
        jpg_hdr[31] = 0x0C0C0C08;
        jpg_hdr[32] = 0x0C0C0C0C;
        jpg_hdr[33] = 0x0C0C0C0C;
        jpg_hdr[34] = 0x0C0C0C0C;
        jpg_hdr[35] = 0x0C0C0C0C;
        jpg_hdr[36] = 0x0C0C0C0C;
        jpg_hdr[37] = 0x0C0C0C0C;
        jpg_hdr[38] = 0x0C0C0C0C;
        jpg_hdr[39] = 0x0C0C0C0C;
        jpg_hdr[40] = 0x0C0C0C0C;
        jpg_hdr[41] = 0x0C0C0C0C;
        jpg_hdr[42] = 0x0C0C0C0C;
        jpg_hdr[43] = 0xFF0C0C0C;
		break;
	case 10:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x01014300;
        jpg_hdr[28] = 0x01020101;
        jpg_hdr[29] = 0x02020402;
        jpg_hdr[30] = 0x05060904;
        jpg_hdr[31] = 0x09090906;
        jpg_hdr[32] = 0x09090909;
        jpg_hdr[33] = 0x09090909;
        jpg_hdr[34] = 0x09090909;
        jpg_hdr[35] = 0x09090909;
        jpg_hdr[36] = 0x09090909;
        jpg_hdr[37] = 0x09090909;
        jpg_hdr[38] = 0x09090909;
        jpg_hdr[39] = 0x09090909;
        jpg_hdr[40] = 0x09090909;
        jpg_hdr[41] = 0x09090909;
        jpg_hdr[42] = 0x09090909;
        jpg_hdr[43] = 0xFF090909;
		break;
	case 11:
        jpg_hdr[26] &= 0x0000FFFF;
        jpg_hdr[26] |= 0xDBFF0000;
        jpg_hdr[27] = 0x01014300;
        jpg_hdr[28] = 0x01010101;
        jpg_hdr[29] = 0x01010201;
        jpg_hdr[30] = 0x03040602;
        jpg_hdr[31] = 0x06060604;
        jpg_hdr[32] = 0x06060606;
        jpg_hdr[33] = 0x06060606;
        jpg_hdr[34] = 0x06060606;
        jpg_hdr[35] = 0x06060606;
        jpg_hdr[36] = 0x06060606;
        jpg_hdr[37] = 0x06060606;
        jpg_hdr[38] = 0x06060606;
        jpg_hdr[39] = 0x06060606;
        jpg_hdr[40] = 0x06060606;
        jpg_hdr[41] = 0x06060606;
        jpg_hdr[42] = 0x06060606;
        jpg_hdr[43] = 0xFF060606;
		break;
	};

}


static inline int init_jpeg_hdr(
	struct jpeg_header_t *jpg_buf,
	u16 x,
	u16 y,
	int is_420,
	int y_dct,
	int uv_dct)
{
	init_default_jpeg_table(jpg_buf, is_420);
	init_jpeg_dct_table(jpg_buf, y_dct, uv_dct);

	jpg_buf->sof0.X = cpu_to_be16(x);
	jpg_buf->sof0.Y = cpu_to_be16(y);

	//printk("offset:%d, X:%d, Y:%d\n", ((u32)(&jpg_buf->sof0.Y) - (u32)jpg_buf),be16_to_cpu(jpg_buf->sof0.X), be16_to_cpu(jpg_buf->sof0.Y));

	return JPG_HDR_SIZE;
}


#endif

