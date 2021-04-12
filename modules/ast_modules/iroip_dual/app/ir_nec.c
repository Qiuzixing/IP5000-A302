/*
 * Copyright (c) 2017
 * ASPEED Technology Inc. All Rights Reserved
 * Proprietary and Confidential
 *
 * By using this code you acknowledge that you have signed and accepted
 * the terms of the ASPEED SDK license agreement.
 */

#include <stdlib.h>
#include <stdio.h>

#define NEC_CARRIER_FREQ 38000 /* 38KHz */
#define NEC_HEADER_ON  9000 /* 9ms, 9000 micro sec */
#define NEC_HEADER_OFF 4500 /* 4.5ms, 4500 micro sec */
#define NEC_HEADER_REPEAT_ON  9000 /* 9ms, 9000 micro sec */
#define NEC_HEADER_REPEAT_OFF 2250 /* 2.25ms, 2250 micro sec */
#define NEC_DATA_1_ON 560
#define NEC_DATA_1_OFF 1690
#define NEC_DATA_0_ON 560
#define NEC_DATA_0_OFF 560
#define TOLERANCE 10 /* 10 % */

struct _range {
	unsigned short min;
	unsigned short max;
};

struct ir_protocol {
	struct _range freq;
	struct _range header_pulse;
	struct _range header_space;
	struct _range header_repeat_pulse;
	struct _range header_repeat_space;
	struct _range data_1_pulse;
	struct _range data_1_space;
	struct _range data_0_pulse;
	struct _range data_0_space;
	unsigned int data_size; /* the size of one single IR decoded command, unit is bit */
};

#define IR_SET(range, cfg) { (range).min = (cfg) * (100 - TOLERANCE) / 100; (range).max = (cfg) * (100 + TOLERANCE) / 100; }

static void ir_protocol_data_init(struct ir_protocol *p)
{
	IR_SET(p->freq, NEC_CARRIER_FREQ);
	IR_SET(p->header_pulse, NEC_HEADER_ON); IR_SET(p->header_space, NEC_HEADER_OFF);
	IR_SET(p->header_repeat_pulse, NEC_HEADER_REPEAT_ON); IR_SET(p->header_repeat_space, NEC_HEADER_REPEAT_OFF);
	IR_SET(p->data_1_pulse, NEC_DATA_1_ON); IR_SET(p->data_1_space, NEC_DATA_1_OFF);
	IR_SET(p->data_0_pulse, NEC_DATA_0_ON); IR_SET(p->data_0_space, NEC_DATA_1_OFF);
	p->data_size = 32; /* device:16 + button code:16 */
}


#define VALUE_IN(a, range) (((a) >= (range).min) && ((a) <= (range).max))

static int nec_value(struct ir_protocol *protocol, unsigned int pulse, unsigned int space)
{
	if (VALUE_IN(pulse, protocol->data_1_pulse) && VALUE_IN(space, protocol->data_1_space))
		return 1;
	if (VALUE_IN(pulse, protocol->data_0_pulse) && VALUE_IN(space, protocol->data_0_space))
		return 0;

	if (pulse == 0)
		return -1;

	if (space == 0)
		return -1;

	/*
	 * 0 =>  pulse:space ~=  560: 560 ~= 1:1
	 * 1 =>  pulse:space ~= 560: 1690 ~= 1:3
	 */
	if (space > (pulse << 1))
		if (space > (pulse << 2))
			return -1;
		else
			return 1;
	else
		return 0;
}

static void nec_low_pass(unsigned int *data, unsigned int num, struct ir_protocol *protocol)
{
	unsigned int i, volume;

	i = 0;

	volume = (protocol->data_1_pulse.min > protocol->data_0_pulse.min) ? protocol->data_0_pulse.min : protocol->data_1_pulse.min;
	volume = volume >> 1;

	while (i < num) {
		/*
		 * just filter 'pulse'
		 * for example:
		 * working on 'space', (index 2n + 1)
		 * if consequent 'pulse' < volume,  (index 2n+2)
		 * we merge 'pulse' and next 'space' (pulse: index 2n+2, space: index 2n+3)
		 */
		if ((i & 0x1) == 1) {
			if (data[i + 1] < volume) {
				data[i] += (data[i + 1] + data[i + 2]);
				data[i + 1] = 0;
				data[i + 2] = 0;
				i += 2;
			}
		}

		i++;
	}
}

static void ir_event_nec(unsigned short addr, unsigned short code)
{
	char cmd_str[128] = "echo TODO";

	sprintf(cmd_str, "/usr/local/bin/ast_send_event -1 e_ir_decoded::nec::%d::%d", addr, code);
	system(cmd_str);
}

#define IR_NEC_DEV_ADDR_DEFAULT 0xFFFF

/*
 * ir_decode_nec - do IR NEC software decoder
 * @freq: IR carrier frequency in Hz
 * @data: pointer to IR data in pulse-space pair format,
 *	each one is half of a pair: pulse, space, pulse, space...
 *	unit is micro second
 * @num: number of input data
 * @dev_addr: device address
 *
 *
 * Return Value:
 *	 0 :NEC code AND match device address
 *	-1: NOT NEC
 *
 *
 * after ir_parse() in irrd.c
 * users can directly use the member of struct ast_ir: freq, buf_index and buf
 * for input parameter of ir_decode_nec()
 * freq => freq;  buf => data;  buf_index => num
 *
 */
int ir_decode_nec(unsigned int freq, unsigned int *data, unsigned int num, unsigned short dev_addr)
{
	int ret = -1;
	struct ir_protocol nec;
	short addr, code;

	ir_protocol_data_init(&nec);

	if ((VALUE_IN(data[0], nec.header_pulse) && VALUE_IN(data[1], nec.header_space))
		&& (VALUE_IN(freq, nec.freq))) {
		unsigned int done = 0, max = nec.data_size;
		unsigned int decode = 0;
		int value;
		unsigned char a, b;

		if (num < max)
			goto exit;

		nec_low_pass(data, num, &nec);

		while (done < max) {
			value = nec_value(&nec, data[(1 + done) << 1], data[1 + ((1 + done) << 1)]);
			if (value == -1)
				goto exit;

			/* value is 0 or 1, done is 0 ~ 31 (nec.data_size is 32 bit) */
			decode = decode | (value << done);
			done++;
		}

		a = (decode >> 16) & 0xFF;
		b = (decode >> 24) & 0xFF;

		if (a == (~b & 0xFF)) { /* command check ok */
			code = a;

			a = decode & 0xFF;
			b = (decode >> 8) & 0xFF;

			if (a == (~b & 0xFF)) {
				if ((dev_addr <= 0xFF) || (dev_addr == IR_NEC_DEV_ADDR_DEFAULT))
					addr = a;
				else
					addr = decode & 0xffff;

				if (addr == dev_addr)
					ret = 0;
			} else { /* extended */
				addr = decode & 0xffff;
				if (addr == dev_addr)
					ret = 0;
			}

			if ((ret == 0) || (dev_addr == IR_NEC_DEV_ADDR_DEFAULT))
				ir_event_nec(addr, code);
		}
	}
exit:
	return ret;
}
