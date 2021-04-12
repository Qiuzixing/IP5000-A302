/*
 * Copyright (c) 2019 ASPEED Technology Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef _AST_AUDIO_DEF_H_
#define _AST_AUDIO_DEF_H_
/*
 * Audio Sample Frequency
 * from channel status bit [31 30 27 26 25 24], HDMI Spec. 2.0
 */
#define AST_FS_32000	(3)	/*   32 kHz, b--0011 */
#define AST_FS_64000	(11)	/*   64 kHz, b001011 */
#define AST_FS_128000	(43)	/*  128 kHz, b101011 */
#define AST_FS_256000	(27)	/*  256 kHz, b011011 */
#define AST_FS_512000	(59)	/*  512 kHz, b111011 */
#define AST_FS_1024000	(53)	/* 1024 kHz, b110101 */

#define AST_FS_44100	(0)	/*   44.1 kHz, b--0000 */
#define AST_FS_88200	(8)	/*   88.2 kHz, b--1000 */
#define AST_FS_176400	(12)	/*  176.4 kHz, b--1100 */
#define AST_FS_352800	(13)	/*  352.8 kHz, b001101 */
#define AST_FS_705600	(45)	/*  705.6 kHz, b101101 */
#define AST_FS_1411200	(29)	/* 1411.2 kHz, b011101 */

#define AST_FS_48000	(2)	/*   48 kHz, b--0010 */
#define AST_FS_96000	(10)	/*   96 kHz, b--1010 */
#define AST_FS_192000	(14)	/*  192 kHz, b--1110 */
#define AST_FS_384000	(5)	/*  384 kHz, b000101 */
#define AST_FS_768000	(9)	/*  768 kHz, b--1001 */
#define AST_FS_1536000	(21)	/* 1536 kHz, b010101 */

/*
 * Audio Type
 * for HDMI Rx, HDMI Tx and I2S
 */
#define AST_AUDIO_ON		(1 << 7)
#define AST_AUDIO_HBR		(1 << 6)
#define AST_AUDIO_DSD		(1 << 5)
#define AST_AUDIO_NLPCM		(1 << 4)
#define AST_AUDIO_HBR_BY_SPDIF	(1 << 3) /* ! unused */
#define AST_AUDIO_SPDIF		(1 << 2) /* ! unused */
#define AST_AUDIO_LPCM		(1 << 0) /* ! unused */

#define AST_AUDIO_ENCODE_MASK (AST_AUDIO_HBR | AST_AUDIO_DSD | AST_AUDIO_NLPCM)



#endif /* #ifndef _AST_AUDIO_DEF_H_ */
