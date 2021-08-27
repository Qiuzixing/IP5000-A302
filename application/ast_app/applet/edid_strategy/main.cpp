#include <iostream>
#include <string>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/igmp.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "jsonparse.h"
using namespace std;
#define EEPROM_CONTENT          "/sys/devices/platform/videoip/eeprom_content"
#define EDID_LIST               "edid_list.json"
#define DEFAULT_EDID            "default"
#define EDID_FILE_LENGTH        30
#define EDID_MAXLEN             256
#define EDID_FILE_MAXNUM        4
#define EDID_PATH_LENGTH        120
enum
{
    GB_VALID = 0,
    GB_INVALID,
    GB_ERROR
};
enum
{
    READ_EDID_FILE = 0,
    ADD_EDID_FILE,
    DELETE_EDID_FILE,
    UNKNOW
};
typedef enum
{
    IPE5000 = 0,
    IPE5000P,
    IPD5000,
    IPD5000W,
    UNKNOW_BOARD
}a30_board_type;

typedef enum
{
    WAIT_RX_EDID_REPORT = 0,
    GET_RX_EDID,
    UNKNOW_CMD
}send_socket_cmd;

typedef enum
{
    LOOP = 0,
    QUERY,
    UNKNOW_CMD_TYPE
}send_socket_cmd_type;

typedef struct
{
    a30_board_type board_type;
    char *edid_path;
} a30_board_name;

typedef struct
{
    send_socket_cmd socket_cmd;
    char *cmd_str;
    unsigned char cmd_len;
    send_socket_cmd_type socket_cmd_type;
} socket_cmd_struct;

const a30_board_name board_name_list[] =
{
    {IPE5000,   "/data/configs/kds-7/edid/"},
    {IPE5000P,   "/data/configs/kds-7/edid/"},
    {IPD5000,   "/data/configs/kds-7/edid/"},
    {IPD5000W,   "/data/configs/kds-7/edid/"}
};

static void HexToAscii(char *pHex, char *pAscii, int nLen)
{
    char Nibble[2];
    unsigned int i,j;
    for (i = 0; i < nLen; i++){
        Nibble[0] = (pHex[i] & 0xF0) >> 4;
        Nibble[1] = pHex[i] & 0x0F;
        for (j = 0; j < 2; j++){
            if (Nibble[j] < 10){
                Nibble[j] += 0x30;
            }
            else{
                if (Nibble[j] < 16)
                    Nibble[j] = Nibble[j] - 10 + 'A';
            }
            *pAscii++ = Nibble[j];
        }
        if(i != nLen - 1)
        {
            *pAscii++ = 0x20;   //0x20 Represents a space,You need to add spaces when writing EEPROM
        }        
    }       
}

static void write_eeprom(char *edid_data)
{
    char eeprom_edid_buf[1024] = {0};
    HexToAscii(edid_data,eeprom_edid_buf,EDID_MAXLEN);//Data conversion
    int fd = open(EEPROM_CONTENT, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error:%s", EEPROM_CONTENT,strerror(errno));
        return;
    }
    ssize_t ret = 0;
    ret = write(fd,eeprom_edid_buf,strlen(eeprom_edid_buf));
    if(strlen(eeprom_edid_buf) != ret)
    {
        printf("write %s error:%s\n", EEPROM_CONTENT,strerror(errno));
        close(fd);
        return;
    }
    close(fd);
}

static void read_file_and_write_eeprom(char *path)
{
    FILE *fp = fopen(path, "r");
    char edid_buf[EDID_MAXLEN] = {0};

    if (fp == NULL)
    {
        printf( "\nCan not open the path: %s \n", path);
        return;
    }

    fread(edid_buf, sizeof(unsigned char), EDID_MAXLEN, fp);
    fclose(fp);

    write_eeprom(edid_buf);
}

int App_EDID_IsValid(uint8_t *pEdid)
{
    uint8_t u1CheckSum = 0;
    int i = 0;

    if (!((pEdid[0] == 00) && (pEdid[1] == 0xFF) && (pEdid[2] == 0xFF) && (pEdid[3] == 0xFF) &&
          (pEdid[4] == 0xFF) && (pEdid[5] == 0xFF) && (pEdid[6] == 0xFF) && (pEdid[7] == 00)))
    {
        return 0;
    }

    // calu block 0
    for (i = 0; i < 128; i++)
    {
        u1CheckSum += pEdid[i];
    }

    if (u1CheckSum != 0)
    {
        return 0;
    }

    // calu block 1
    u1CheckSum = 0;

    if (pEdid[126])
    {
        for (i = 128; i < 256; i++)
        {
            u1CheckSum += pEdid[i];
        }
    }

    if (u1CheckSum != 0)
    {
        return 0;
    }

    return 1;
}

int connect_rx_get_edid(char *rx_ip_addr,int port_num,int index)
{
    static const socket_cmd_struct socket_cmd_list[] =
    {
        {WAIT_RX_EDID_REPORT,   "wait_rx_edid",     sizeof("wait_rx_edid"),     LOOP},
        {GET_RX_EDID,           "get_rx_edid",      sizeof("get_rx_edid"),      QUERY}
    };
    int sockfd = 0;
    int n = 0;
    int i = 0;
    char edid_info[EDID_MAXLEN] = {0};
    struct sockaddr_in servaddr;
    int ret = 0;
    int reuse = 1;
    do
    {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
            break;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port_num);
        if (inet_pton(AF_INET, rx_ip_addr, &servaddr.sin_addr) <= 0)
        {
            printf("inet_pton error for %s\n", rx_ip_addr);
            break;
        }

        ret = setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&reuse, sizeof(reuse) );//close Nagle,because the data is very small
        if(ret < 0)
        {
            perror("cannot setsockopt server socket");
            break;
        }

        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
            break;
        }

        switch(socket_cmd_list[index].socket_cmd)
        {
            case WAIT_RX_EDID_REPORT:
                ret = write(sockfd,socket_cmd_list[index].cmd_str,socket_cmd_list[index].cmd_len);
                if(ret != socket_cmd_list[index].cmd_len)
                {
                    printf("write socket error\n");
                    break;
                }
                while(1)
                {
                    ret = read(sockfd,edid_info,sizeof(edid_info));
                    if(ret == 0)
                    {
                        printf("server close\n");
                        break;
                    }
                    else
                    {
                        if(App_EDID_IsValid((unsigned char*)edid_info))
                        {
                            printf("edid verify success,write eeprom\n");
                            write_eeprom(edid_info);
                            
                        }
                        else
                        {
                            printf("edid verify fail,discard\n");
                        } 
                    }
                }
                break;
            case GET_RX_EDID:
                ret = write(sockfd,socket_cmd_list[index].cmd_str,socket_cmd_list[index].cmd_len);
                if(ret != socket_cmd_list[index].cmd_len)
                {
                    printf("write socket error\n");
                    break;
                }

                ret = read(sockfd,edid_info,sizeof(edid_info));
                if(ret == 0)
                {
                    printf("server close\n");
                    break;
                }
                else
                {
                    if(App_EDID_IsValid((unsigned char*)edid_info))
                    {
                        write_eeprom(edid_info);
                    }
                    else
                    {
                        printf("no edid\n");
                    }
                }
                break;
            default:
                break;
        }
    } while (0);
    
    if(sockfd > 0)
    {
        close(sockfd);
    }
    
    return 0;
}

static void edid_mode_is_passthru(void)
{
    printf("edid_mode_is_passthru\n");
}

static void edid_mode_is_default(unsigned char board_index)
{
    char edid_file_path[EDID_PATH_LENGTH] = {0};
    sprintf(edid_file_path,"%s%s",board_name_list[board_index].edid_path,DEFAULT_EDID);
    read_file_and_write_eeprom(edid_file_path);
}

static void edid_mode_is_custom(unsigned char index,char *edid_list_path,unsigned char board_index)
{
    char *edid_file_name = NULL;
    char edid_file_path[EDID_PATH_LENGTH] = {0};
    if (false == ParseJsonFile(edid_list_path))
    {
        return;
    }
    edid_file_name = GetStructBufValue((E_Buf_Name)index);
    if(edid_file_name == NULL)
    {
        printf("File does not exist in %d index\n",index);
    }
    else if(strlen(edid_file_name) >= EDID_FILE_LENGTH)
    {
        printf("file name is too long,please Modify file name\n");
    }
    else
    {
        sprintf(edid_file_path,"%s%s",board_name_list[board_index].edid_path,edid_file_name);
        read_file_and_write_eeprom(edid_file_path);
    }
}

static void do_handle_edid_file(char op_edid_file, char *edid_file, unsigned char index,char *edid_list_path)
{
    switch (op_edid_file)
    {
    case READ_EDID_FILE:
        break;
    case ADD_EDID_FILE:
        SetStructBufValue((E_Buf_Name)index, edid_file);
        SaveStruct2File(edid_list_path);
        break;
    case DELETE_EDID_FILE:
        SetStructBufValue((E_Buf_Name)index, "");
        SaveStruct2File(edid_list_path);
        break;
    default:
        break;
    }
}

static uint8_t ascii_to_hex(uint8_t b_hex)
{
    if ((b_hex >= 0x30) && (b_hex <= 0x39)) //num:0-9
    {
        b_hex = b_hex - 0x30;
    }
    else if ((b_hex >= 0x41) && (b_hex <= 0x46)) //A,B,C,D,E,F....
    {
        b_hex = b_hex - 0x37;
    }
    else if ((b_hex >= 0x61) && (b_hex <= 0x66)) //a,b,c,d,e,f....
    {
        b_hex = b_hex - 0x57;
    }
    else
    {
        b_hex = 0xff;
    }

    return b_hex;
}

static uint8_t *do_handle_read_edid_info(int edid_file_size, uint8_t *edid_file_point, uint8_t *edid_info)
{
    int i = 0;

    uint8_t k = 0;
    uint8_t tmp = 0;
    for (i = 0; i < edid_file_size; i++)
    {
        tmp = ascii_to_hex(edid_file_point[i]);
        if (0xff == tmp)
        {
            continue;
        }
        edid_info[k++] = tmp * 16 + ascii_to_hex(edid_file_point[++i]);
    }

    return edid_info;
}

static int gb_detect_eeprom_edid_whether_valid()
{
    int ret = 0;
    uint8_t edid_num[EDID_MAXLEN];
    uint8_t tmp_point[1024] = {0};
    int fd = open(EEPROM_CONTENT, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error\n", EEPROM_CONTENT);
        return GB_ERROR;
    }

    ret = read(fd, tmp_point, sizeof(tmp_point));
    close(fd);
    if (ret < 0)
    {
        printf("read %s error\n", EEPROM_CONTENT);

        return GB_ERROR;
    }

    do_handle_read_edid_info(ret, tmp_point, edid_num);

    if (App_EDID_IsValid(edid_num))
    {
        return GB_VALID;
    }

    return GB_ERROR;
}

static void Usage(void)
{
    printf("[Usage]:\n");
    printf("delete file: ./handle_edid -d -i 2\n");
    printf("add file: ./handle_edid -a filename -i 2\n");
    printf("set_mode: ./handle_edid -s custom -i 2\n");
    printf("set_mode: ./handle_edid -s default\n");
    printf("set_mode: ./handle_edid -s passthru\n");
    printf("verify eeprom validity: ./handle_edid -v\n");
    printf("connect RX and wait for EDID Report: ./handle_edid -c rx_ip -t rx_tcp_port\n");
    printf("connect RX and query EDID data: ./handle_edid -c rx_ip -t rx_tcp_port -m 1\n");
}

struct option longopts[] = {
    {"set_mode", required_argument, NULL, 's'},
    {"lock_edid", required_argument, NULL, 'l'},
    {"add_edid_file", required_argument, NULL, 'a'},
    {"delete_edid_file", no_argument, NULL, 'd'},
    {"read_edid_file", required_argument, NULL, 'r'},
    {"index", required_argument, NULL, 'i'},
    {"eeprom_edid_whether_valid", no_argument, NULL, 'v'},
    {"connect_rx_server", required_argument, NULL, 'c'},
    {"board_type", required_argument, NULL, 'b'},
    {"tcp_port", required_argument, NULL, 't'},
    {"command", required_argument, NULL, 'm'},
    {"udp_commun", required_argument, NULL, 'u'},
    {NULL, 0, NULL, 0}};

int main(int argc, char *argv[])
{
    char *edid_file = NULL;
    char op_edid_file = UNKNOW;
    char *edid_mode = NULL;
    char *lock_edid = NULL;
    char *rx_ip_addr = NULL;
    char edid_list_path[EDID_PATH_LENGTH] = {0};
    unsigned char board_type = IPE5000P;
    unsigned char edid_file_map_index = EDID_FILE_MAXNUM - 1;
    char detect_eeprom_edid_whether_valid = 0;
    int port_num = 0;
    int socket_cmd_index = WAIT_RX_EDID_REPORT;
    int udp_flag = 0;
    for (;;)
    {
        int c;
        int index = 0;

        c = getopt_long(argc, argv, "m:t:b:c:s:l:a:r:i:dvu", longopts, &index);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
        case 's':
            edid_mode = optarg;
            break;
        case 'l':
            lock_edid = optarg;
            break;
        case 'a':
            edid_file = optarg;
            op_edid_file = ADD_EDID_FILE;
            break;
        case 'r':
            edid_file = optarg;
            op_edid_file = READ_EDID_FILE;
            break;
        case 'd':
            op_edid_file = DELETE_EDID_FILE;
            break;
        case 'i':
            edid_file_map_index = (unsigned char)strtoul(optarg, NULL, 0);
            break;
        case 'v':
            detect_eeprom_edid_whether_valid = 1;
            break;
        case 'c':
            rx_ip_addr = optarg;
            break;
        case 'b':
            board_type = (unsigned char)strtoul(optarg, NULL, 0);
            break;
        case 't':
            port_num = strtoul(optarg, NULL, 0);
            break;
         case 'm':
            socket_cmd_index = strtoul(optarg, NULL, 0);
            break;
        case 'u':
            udp_flag = 1;
            break;
        default:
            Usage();
            exit(EXIT_FAILURE);
        }
    }

    if (edid_file_map_index > EDID_FILE_MAXNUM )
    {
        printf("index is too large\n");
        return 0;
    }

    if(edid_file_map_index == 0 && 0 != strcmp(edid_mode, "custom"))
    {
        printf("Index 0 indicates that the increase cannot be deleted by default\n");
        return 0;
    }

    sprintf(edid_list_path,"%s%s",board_name_list[board_type].edid_path,EDID_LIST);

    if (op_edid_file != UNKNOW)
    {
        if (false == ParseJsonFile(edid_list_path))
        {
            return 0;
        }
        do_handle_edid_file(op_edid_file, edid_file, edid_file_map_index,edid_list_path);
    }

    if (1 == detect_eeprom_edid_whether_valid)
    {
        if (GB_VALID == gb_detect_eeprom_edid_whether_valid())
        {
            printf("edid verify success\n");
        }
        else
        {
            printf("edid verify error\n");
        }
    }

    if (NULL != edid_mode)
    {
        if (0 == strcmp(edid_mode, "passthru"))
        {
            edid_mode_is_passthru();
        }
        else if (0 == strcmp(edid_mode, "default"))
        {
            edid_mode_is_default(board_type);
        }
        else if (0 == strcmp(edid_mode, "custom"))
        {
            edid_mode_is_custom(edid_file_map_index,edid_list_path,board_type);
        }
        else
        {
        }
    }
    if(NULL != rx_ip_addr && port_num != 0 && udp_flag == 0)
    {
        connect_rx_get_edid(rx_ip_addr,port_num,socket_cmd_index);
    }
    return 0;
}
