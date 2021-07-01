#ifndef _UART_MCU_CMD_H_
#define _UART_MCU_CMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <strings.h>
#include <time.h>
#include <dirent.h>

#include <getopt.h>
#include <netdb.h>       //resolve hostname
#include <netinet/tcp.h> //For TCP
#include <linux/netlink.h>
#include <asm/types.h>
#include <string.h>
#include <errno.h>
#include "command.h"


#include "ymodem.h"
#define UART_RX_BUFFER_LEN (64)
#define CMD_NULL_DATA    0

int uart_Read(unsigned int timout, uint8_t *buf, int len);
void ymodem_send_recv_file(void);
int update(CmdProtocolParam * param);
void arm_send_cmd(uint16_t cmd);

#endif

