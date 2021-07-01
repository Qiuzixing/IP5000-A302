#include "uart_mcu_cmd.h"
#include "ipx5000_a30_cmd.h"
#include "../gb_commun_with_mcu.h"
#define RESET_MCU_FILE "/sys/class/leds/mcu_reset/brightness"

extern uint8_t arm_version[20];
extern uint8_t version_len;
extern uint8_t file_path[120];
extern uint8_t file_path_len;
extern int uart_fd;
extern int errno;

extern uint8_t Send_File_Flag;

struct CmdDataCRC Cal_Crc;
extern uint8_t Cmd_Send_Flag;
extern uint8_t Up_Succ_Flag ;
extern uint8_t Up_Fail_Flag;
extern uint8_t up_or_commun_flag;
int err_count = 0;


uint16_t crc16(const uint8_t *buf, uint32_t len)
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

void Msleep(unsigned int microsecond)
{
    struct timeval time;
    time.tv_sec = 0; //seconds
    time.tv_usec = microsecond *1000;     //microsecond
    select(0, NULL, NULL, NULL, &time);
}

int uart_Read(unsigned int timout, uint8_t *buf, int len)
{
    int nfds;
    int i;
    int ret = 0;
    fd_set readfds;
    struct timeval tv;

    tv.tv_sec =  0;
    tv.tv_usec = timout*1000;
    if(up_or_commun_flag == UPGRADE)
    {
        FD_ZERO(&readfds);
        FD_SET(uart_fd, &readfds);

        nfds = select(uart_fd + 1, &readfds, NULL, NULL, &tv);
        if (0 == nfds)
        {
            return 0;
        }
        else if (nfds < 0)
        {
            fprintf(stderr, "read uart error:%s", strerror(errno));
            return -1;
        } 
    }
    else
    {

    }
    //read_uart_buffer.read_buffer_len = read(uart_fd, read_uart_buffer.read_buffer + read_uart_buffer.offset, UART_RX_BUFFER_LEN - read_uart_buffer.offset);
    
    ret = read(uart_fd, buf, len);
   return ret;
}


int uart_Write(const char *s, int n)
{
    int ret = -1;
    ret = write(uart_fd, s, n);
    if (ret <= 0)
    {
        fprintf(stderr, "write uart error:%s", strerror(errno));
        return (-1);
    }
    // printf("write data len->0x%x\n", ret);

    return ret;
}

static void reset_mcu()
{
    int fd = open(RESET_MCU_FILE, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "open reset_stm32 error:%s", strerror(errno));
        exit(1);
    }
    ssize_t ret = 0;
    ret = write(fd, "0", 1);
    if (1 != ret)
    {
        fprintf(stderr, "write reset_stm32 error:%s", strerror(errno));
        exit(1);
    }
    Msleep(1000);
    ret = write(fd, "1", 1);
    if (1 != ret)
    {
        fprintf(stderr, "write reset_stm32 error:%s", strerror(errno));
        exit(1);
    }
    close(fd);
}

void arm_send_cmd(uint16_t cmd)
{
    uint32_t uctemp = CMD_NULL_DATA;
    APP_Comm_Send(cmd, &uctemp, 4);
}

uint8_t *get_file_content(size_t *file_size)
{
    FILE *rf = NULL;
    //文件大小
    rf = fopen(file_path, "rb+");
    if (NULL == rf)
    {
        fprintf(stderr, "fopen error:%s", strerror(errno));
        return NULL;
    }
    if (fseek(rf, 0L, SEEK_END) != 0)
    {
        fprintf(stderr, "fseek error:%s", strerror(errno));
        fclose(rf);
        return NULL;
    }
    *file_size = ftell(rf);
    rewind(rf);
    uint8_t *p = (uint8_t *)calloc(*file_size + 1024, sizeof(uint8_t));
    if (NULL == p)
    {
        fprintf(stderr, "Failed to allocate memory\n");
        fclose(rf);
        return NULL;
    }
    size_t fread_ret = fread(p, 1, *file_size, rf);
    if (fread_ret != *file_size)
    {
        fprintf(stderr, "fread error:%s", strerror(errno));
        fclose(rf);
        free(p);
        return NULL;
    }
    fclose(rf);
    return p;
}

void ymodem_send_recv_file(void)
{
    uint8_t *file_pointer = NULL;
    size_t file_size = 0;
    int ymodem_ret;
    uint32_t crc;

    file_pointer = get_file_content(&file_size);
    if(NULL == file_pointer)
    {
        return ;
    }

    if(err_count == 3)
    {
        printf("reset mcu,continue to try\n");
        reset_mcu();
    }
    else if(err_count == 6)
    {
        Up_Fail_Flag = 1;
        return ;
    }

    printf("updating...\n");
    ymodem_ret = fymodem_send(file_pointer, file_size, &(file_path[file_path_len + 1]));
    if(ymodem_ret > 0)
    {
        crc = crc16(file_pointer, file_size); // crc
        Cal_Crc.crc = crc;
        arm_send_cmd(CMD_UPDATE_GET_CRC);
    }
    else
    {
        printf("Ymodem error,try load again\n");
        err_count++;
        Send_File_Flag = 1;
        arm_send_cmd(CMD_UPDATE_SEND_FILE);
    }

}

int update(CmdProtocolParam * param)
{
    struct CmdDataMCUStatus mcu_sta;
    struct CmdDataCRC check_crc;
    struct CmdDataMCUVersion mcu_ver;


    if(err_count >= 3)
    {
        Up_Fail_Flag = 1;
        return 0;
    }

    switch (param->CMD)
    {
        case CMD_UPDATE_MCU_STATUS:
            mcu_sta.status = param->Data;
            if(mcu_sta.status) //app
                arm_send_cmd(CMD_UPDATE_GET_MCU_VERSION);
            else // boot
            {
                Send_File_Flag = 1;
                arm_send_cmd(CMD_UPDATE_SEND_FILE);
                return 0;
            }
            break;
        case CMD_UPDATE_MCU_VERSION:
            memcpy(mcu_ver.version, (const char *)param->Data, sizeof(mcu_ver));
            printf("mcu ver:%s\n", mcu_ver.version);
            if(strncmp((const char*)mcu_ver.version, (const char*)arm_version, version_len) == 0)
            {
                Up_Succ_Flag = 1;
                printf("The version number is consistent\n");
            }
            else
            {
                printf("The version number isn't consistent!,Ready to update\n");
                arm_send_cmd(CMD_UPDATE_JUMP_TO_BOOT);
            }
            break;
        case CMD_UPDATE_READY_TO_BOOT:
            arm_send_cmd(CMD_UPDATE_SEND_FILE);
            Send_File_Flag = 1;
            return 0;
        case CMD_UPDATE_READY_REC_FILE:
            break;
        case CMD_UPDATE_CRC:
            check_crc.crc= param->Data;
            if(check_crc.crc == Cal_Crc.crc)
            {
                printf("CRC verification success\n");
                Up_Succ_Flag = 1;
                arm_send_cmd(CMD_UPDATE_JUMP_TO_APP);
            }
            else
            {
                err_count++;
                printf("CRC verification failed,try load again\n");
                arm_send_cmd(CMD_UPDATE_SEND_FILE);
            }
            break;
        default:
            break;
    }

    return 0;
}

