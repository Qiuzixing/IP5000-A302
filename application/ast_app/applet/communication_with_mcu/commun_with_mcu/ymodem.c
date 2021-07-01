#include "ymodem.h"
#include <unistd.h>
//#include "Taskfile2mcu.h"
//#include "GBIHSandexWebSocketTask.h"

//CTaskfile2mcu*  g_pTaskUP4mcu;
//extern CGBIHSandexWSTask*  g_SandexWSTask;

/* filesize 999999999999999 should be enough... */
#define YM_FILE_SIZE_LENGTH (16)

/* packet constants */
#define YM_PACKET_SEQNO_INDEX (1)
#define YM_PACKET_SEQNO_COMP_INDEX (2)
#define YM_PACKET_HEADER (3)  /* start, block, block-complement */
#define YM_PACKET_TRAILER (2) /* CRC bytes */
#define YM_PACKET_OVERHEAD (YM_PACKET_HEADER + YM_PACKET_TRAILER)
#define YM_PACKET_SIZE (128)
#define YM_PACKET_1K_SIZE (1024)
#define YM_PACKET_RX_TIMEOUT_MS (1000)
#define YM_PACKET_ERROR_MAX_NBR (5)

/* contants defined by YModem protocol */
#define YM_SOH (0x01)  /* start of 128-byte data packet */
#define YM_STX (0x02)  /* start of 1024-byte data packet */
#define YM_EOT (0x04)  /* End Of Transmission */
#define YM_ACK (0x06)  /* ACKnowledge, receive OK */
#define YM_NAK (0x15)  /* Negative ACKnowledge, receiver ERROR, retry */
#define YM_CAN (0x18)  /* two CAN in succession will abort transfer */
#define YM_CRC (0x43)  /* 'C' == 0x43, request 16-bit CRC, use in place of first NAK for CRC mode */
#define YM_ABT1 (0x41) /* 'A' == 0x41, assume try abort by user typing */
#define YM_ABT2 (0x61) /* 'a' == 0x61, assume try abort by user typing */

/* ------------------------------------------------ */

/* user callbacks, implement these for your target */

#define __ym_sleep_ms(ms) usleep(ms * 1000)
#define __ym_flush() fflush(NULL)
/* example functions for POSIX/Unix */
#define __ym_getchar_posix(tmo_ms) read(tmo_ms / 1000)
#define __ym_putchar_posix(c) write(c)
#define __ym_sleep_ms_posix(ms) sleep(ms / 1000)
#define __ym_flush_posix() flush()
extern int uart_fd;
/* error logging function */
#define YM_ERR(fmt, ...)      \
  do                          \
  {                           \
    printf(fmt, __VA_ARGS__); \
  } while (0)

static int StUartRead1Byte(unsigned int timout)
{
  int nfds;
  fd_set readfds;
  struct timeval tv;
  char buf[1] = {0};

  tv.tv_sec = timout * 4;
  tv.tv_usec = 0;

  FD_ZERO(&readfds);
  FD_SET(uart_fd, &readfds);

  // 暂时没用传入的等待时间，采用阻塞式读取

  nfds = select(uart_fd + 1, &readfds, NULL, NULL, &tv);
  if (nfds <= 0)
  {
    if (nfds == 0)
      // errno = ETIME;
      return (-1);
  }

  if (read(uart_fd, buf, 1) < 0)
    return -1;
  if (*(char *)buf == 0x06 || *(char *)buf == 0x15 || *(char *)buf == 0x18)
  {
    //printf("StUartRead1Byte(), *buf = %c, = %#x\n", *buf, *buf);
    return *(char *)buf;
  }
}

static int StUartWrite(const char *s, int n)
{
  int ret = -1;
  ret = write(uart_fd, s, n);
  //printf("StUartWrite, ret = %d\n", ret);
  return ret;
}
/* ------------------------------------------------ */
/* calculate crc16-ccitt very fast
   Idea from: http://www.ccsinfo.com/forum/viewtopic.php?t=24977
*/
uint16_t ym_crc16(const uint8_t *buf, uint16_t len)
{
  uint16_t x;
  uint16_t crc = 0;
  while (len--)
  {
    x = (crc >> 8) ^ *buf++;
    x ^= x >> 4;
    crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
  }
  return crc;
}

/* ------------------------------------------------- */
/* write 32bit value as asc to buffer, return chars written. */
static uint32_t ym_writeU32(uint32_t val, uint8_t *buf)
{
  uint32_t ci = 0;
  if (val == 0)
  {
    /* If already zero then just return zero */
    buf[ci++] = '0';
  }
  else
  {
    /* Maximum number of decimal digits in uint32_t is 10, add one for z-term */
    uint8_t s[11];
    int32_t i = sizeof(s) - 1;
    /* z-terminate string */
    s[i] = 0;
    while ((val > 0) && (i > 0))
    {
      /* write decimal char */
      s[--i] = (val % 10) + '0';
      val /= 10;
    }
    uint8_t *sp = &s[i];
    /* copy results to out buffer */
    while (*sp)
    {
      buf[ci++] = *sp++;
    }
  }
  /* z-term */
  buf[ci] = 0;
  /* return chars written */
  return ci;
}

/* ------------------------------------ */
static void ym_send_packet(uint8_t *txdata,
                           int32_t block_nbr)
{
  int32_t tx_packet_size;
  char packet[1200] = {0};
  int idx = 0;

  /* We use a short packet for block 0, all others are 1K */
  if (block_nbr == 0)
  {
    tx_packet_size = YM_PACKET_SIZE;
  }
  else
  {
    tx_packet_size = YM_PACKET_1K_SIZE;
  }

  uint16_t crc16_val = ym_crc16(txdata, tx_packet_size);

  /* For 128 byte packets use SOH, for 1K use STX */
  packet[idx++] = ((block_nbr == 0) ? YM_SOH : YM_STX);
  /* write seq numbers */
  packet[idx++] = (block_nbr & 0xFF);
  packet[idx++] = (~block_nbr & 0xFF);

  /* write txdata */
  int32_t i;
  for (i = 0; i < tx_packet_size; i++)
  {
    packet[idx++] = (txdata[i]);
  }

  /* write crc16 */
  packet[idx++] = ((crc16_val >> 8) & 0xFF);
  packet[idx++] = (crc16_val & 0xFF);
  StUartWrite(packet, idx);
}

/* ----------------------------------------------- */
/* Send block 0 (the filename block), filename might be truncated to fit. */
static void ym_send_packet0(const char *filename,
                            int32_t filesize)
{
  int32_t pos = 0;
  /* put 256byte on stack, ok? reuse other stack mem? */
  uint8_t block[YM_PACKET_SIZE];
  if (filename)
  {
    /* write filename */
    while (*filename && (pos < YM_PACKET_SIZE - YM_FILE_SIZE_LENGTH - 2))
    {
      block[pos++] = *filename++;
    }
    /* z-term filename */
    block[pos++] = 0;

    /* write size, TODO: check if buffer can overwritten here. */
    pos += ym_writeU32(filesize, &block[pos]);
  }

  /* z-terminate string, pad with zeros */
  while (pos < YM_PACKET_SIZE)
  {
    block[pos++] = 0;
  }
  /* send header block */
  ym_send_packet(block, 0);
}

/* ------------------------------------------------- */
static int ym_send_data_packets(uint8_t *txdata,
                                uint32_t txlen,
                                uint32_t timeout_ms)
{
  int32_t block_nbr = 1;
  int totalBlock = txlen / 1024 + 1;
  float fPercent = 0.0;
  char cSendPercent[512] = {0};

  while (txlen > 0)
  {
    /* check if send full 1k packet */
    uint32_t send_size;
    if (txlen > YM_PACKET_1K_SIZE)
    {
      send_size = YM_PACKET_1K_SIZE;
    }
    else
    {
      send_size = txlen;
    }
    /* send packet */
    ym_send_packet(txdata, block_nbr);
    int32_t c = StUartRead1Byte(timeout_ms);
    switch (c)
    {
    case YM_ACK:
    {
      txdata += send_size;
      txlen -= send_size;
      block_nbr++;
      break;
    }
    case -1:
    {
      fprintf(stderr, "wait ymodem's respond timeout\n");
      return -1;
    }
    case YM_CAN:
    {
      c = StUartRead1Byte(timeout_ms);
      if (YM_CAN == c)
      {
        return -1;
      }
      else
      {
        break;
      }
    }
    default:
    {
      return -1;
    }
    }
  }
  if(txlen == 0)
  {
    int32_t ch;
    char Packet_eot[1] = {0};
    Packet_eot[0] = YM_EOT;
    int32_t a = 0;
    int32_t b = 0;
    do
    {
      StUartWrite(Packet_eot, 1);
      ch = StUartRead1Byte(timeout_ms);
      a++;
      if(6 == a)
        break;
    } while ((ch != YM_ACK) && (ch != -1));
    //ch = StUartRead1Byte(timeout_ms);
    /* send last data packet */
    if (ch == YM_ACK)
    {
      do
      {
        ym_send_packet0(0, 0);
        ch = StUartRead1Byte(timeout_ms);
        b++;
        if(6 == b)
          break;
      } while ((ch != YM_ACK) && (ch != -1));
      if(b != 6)
        return 1;
      else
        return -1;
    }
    return -1;
  }
  else{
    return -1;
  }
  
}

/* ------------------------------------------------------- */
int32_t fymodem_send(uint8_t *txdata, size_t txsize, const char *filename)
{
  int32_t ch;
  bool crc_nak = true;
  bool file_done = false;
  int32_t ret = 0;
  int16_t num = 0;
  /* flush the RX FIFO, after a cool off delay */
  __ym_sleep_ms(1000);
  __ym_flush();
 
  /* not in the specs, send CRC here just for balance */
  /*do {
    // __ym_putchar(YM_CRC);
    ch = StUartRead1Byte(1000);
  } while (ch < 0);
  
  
  if (ch != YM_CRC) {
    goto tx_err_handler;
  }*/
  do
  {
    ym_send_packet0(filename, txsize);
    /* When the receiving program receives this block and successfully
       opened the output file, it shall acknowledge this block with an ACK
       character and then proceed with a normal XMODEM file transfer
       beginning with a "C" or NAK tranmsitted by the receiver. */
    ch = StUartRead1Byte(2);
    num++;
    if( 2 == num)
        goto tx_err_handler;
    if (ch == YM_ACK)
    {
      //    ch = StUartRead1Byte(YM_PACKET_RX_TIMEOUT_MS);
      //      if (ch == YM_CRC) {
      ret = ym_send_data_packets(txdata, txsize, 1);
      if (ret != 1)
      { 
        goto tx_err_handler;
      }
      else
      {
        file_done = true;
      }

    }
    else if (ch == YM_CAN)
    {
        goto tx_err_handler;
    }
  } while (!file_done);

  return txsize;
tx_err_handler:
  //StUartWrite((const char*)YM_CAN,1);
  __ym_sleep_ms(1000);
  return 0;
}

