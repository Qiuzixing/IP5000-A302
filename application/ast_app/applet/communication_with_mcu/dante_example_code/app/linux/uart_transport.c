/*
* File     : uart_transport.c
* Created  : October 2014
* Updated  : 2014/10/27
* Author   : Chamira Perera
* Synopsis : HostCPU Transport (physical) Interface PC UART implementation
*
* This is an implementation of the SPI / UART peripheral interface on the Host CPU
*
* Copyright 2005-2017, Audinate Pty Ltd and/or its licensors
* Confidential
* Audinate Copyright Header Version 1
*/
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "hostcpu_transport.h"

#include "uhip_structures.h"
#include "../../gb_commun_with_mcu.h"
extern unsigned char dante_cmd_buff[DANTE_UART_BUFFER];
extern int dante_cmd_len;
#define RX_BUF_SIZE 1024
#define PC_COM_PORT 10

static uint8_t rx_in_buffer[RX_BUF_SIZE];
static size_t rx_read_idx = 0;
static size_t total_rx = 0;
static int h_serial_port = -1;
static const char* g_com_port = "/dev/ttyS2";
static const char* g_com_param = "115200-8n1";
static uint32_t g_baud = 115200;

static int setserial(int s, struct termios *cfg, int speed, int data, unsigned char parity, int stopb)
{
	speed_t speed_cfg;

	cfmakeraw(cfg);

	switch (speed) {
	case 50:
		speed_cfg = B50; break;
	case 75:
		speed_cfg = B75; break;
	case 110:
		speed_cfg = B110; break;
	case 134:
		speed_cfg = B134; break;
	case 150:
		speed_cfg = B150; break;
	case 200:
		speed_cfg = B200; break;
	case 300:
		speed_cfg = B300; break;
	case 600:
		speed_cfg = B600; break;
	case 1200:
		speed_cfg = B1200; break;
	case 1800:
		speed_cfg = B1800; break;
	case 2400:
		speed_cfg = B2400; break;
	case 4800:
		speed_cfg = B4800; break;
	case 9600:
		speed_cfg = B9600; break;
	case 19200:
		speed_cfg = B19200; break;
	case 38400:
		speed_cfg = B38400; break;
	case 57600:
		speed_cfg = B57600; break;
	case 115200:
		speed_cfg = B115200; break;
	case 230400:
		speed_cfg = B230400; break;
	default:
		speed_cfg = B115200; break;
	}

	cfsetispeed(cfg, speed_cfg);
	cfsetospeed(cfg, speed_cfg);

	switch (parity|32) {
	case 'e':
		cfg->c_cflag |= PARENB; cfg->c_cflag &= ~PARODD; break;
	case 'o':
		cfg->c_cflag |= PARENB; cfg->c_cflag |= PARODD ; break;
	case 'n':
	default:
		cfg->c_cflag &= ~PARENB; break;
	}

	cfg->c_cflag &= ~CSIZE;

	switch (data) {
	case 5:
		cfg->c_cflag |= CS5; break;
	case 6:
		cfg->c_cflag |= CS6; break;
	case 7:
		cfg->c_cflag |= CS7; break;
	case 8:
	default:
		cfg->c_cflag |= CS8; break;
	}

	if (stopb == 1)
		cfg->c_cflag &= ~CSTOPB;
	else
		cfg->c_cflag |= CSTOPB;

	return tcsetattr(s, TCSANOW, cfg);
}

static int open_uart(const char *uart_port, uint32_t uart_baud)
{
	int fd;
	int speed, data, stopb;
	unsigned char parity;
	unsigned int n;
	struct termios cfg;
    bzero(&cfg, sizeof(cfg));

	fd = open(uart_port, O_RDWR|O_NDELAY);
	if (fd < 0) {
		AUD_PRINTF("could not open device %s", uart_port);
		return -1;
	}

	if (setserial(fd, &cfg, uart_baud, 8, 'n', 1) < 0) {
		AUD_PRINTF("could not initialize device %s", uart_port);
		return -3;
	}

	return fd;
}

static void cleanup(void);
static size_t read_data(void);

static size_t read_data()
{	
	ssize_t num_data = read(h_serial_port, rx_in_buffer, sizeof(rx_in_buffer));

	if (num_data < 0)
	{
		if (errno != EAGAIN)
		{
			AUD_PRINTF("Unable to read from serial port\n");
		}
		num_data = 0;
	}

	return num_data;
}

static void cleanup()
{
	if (h_serial_port < 0)
	{
		close(h_serial_port);
	}
}

aud_bool_t hostcpu_transport_init(void)
{

	return AUD_TRUE;
}

/**
* Write bytes to the transport interface
* @param buffer [in] Pointer to the buffer to write
* @param num_bytes [in] The number of bytes to write
* @return the number of bytes written
*/
size_t hostcpu_transport_write(uint8_t const * buffer, size_t num_bytes)
{
	uint32_t i;
	if(num_bytes >= DANTE_UART_BUFFER)
	{
		return;
	}
	for (i = 0; i < num_bytes; i++)
	{
		dante_cmd_buff[i] = buffer[i];
	}
	dante_cmd_len = num_bytes;
#if 0
	uint32_t i;
	uint32_t chunks = num_bytes / UHIP_CHUNK_SIZE;
	ssize_t num_bytes_written;

	if (!num_bytes) {
		return 0;
	}

	if (chunks)
	{
		for (i = 0; i < chunks; ++i)
		{
			num_bytes_written = write(h_serial_port, &buffer[i * UHIP_CHUNK_SIZE], UHIP_CHUNK_SIZE);
			if (num_bytes_written < 0)
			{
				AUD_PRINTF("Unable to write data\n");
				return 0;
			}
			if (num_bytes_written == 0)
			{
				AUD_PRINTF("Did not write any data to the serial port\n");
				return 0;
			}
			if (num_bytes_written != UHIP_CHUNK_SIZE)
			{
				AUD_PRINTF("Write but not all data to the serial port\n");
			}
		}
	}
	else
	{
		num_bytes_written = write(h_serial_port, buffer, num_bytes);
		if (num_bytes_written < 0)
		{
			AUD_PRINTF("Unable to write data\n");
			return 0;
		}
		if (num_bytes_written == 0)
		{
			AUD_PRINTF("Did not write any data to the serial port\n");
			return 0;
		}
		if (num_bytes_written != num_bytes)
		{
			AUD_PRINTF("Write but not all data to the serial port\n");
		}
	}
#endif

	return num_bytes;
}

/**
* Read bytes from the transport interface
* @param buffer [in] Pointer to the buffer to read into
* @param max_bytes_to_read [in] The maximum number of bytes to read
* @return the number of bytes read
*/
size_t hostcpu_transport_read(uint8_t* buffer, size_t max_bytes_to_read)
{
	if (total_rx == 0)
	{
		total_rx = read_data();
		if (total_rx == 0) {
			return 0;
		}
	}

	size_t data_size;

	if (total_rx >= max_bytes_to_read) {
		data_size = max_bytes_to_read;
	}
	else {
		data_size = total_rx;
	}

	memcpy(buffer, &rx_in_buffer[rx_read_idx], data_size);

	rx_read_idx = (rx_read_idx + data_size);
	total_rx -= data_size;

	if (total_rx == 0) {
		rx_read_idx = 0;
	}

	return data_size;
}
