#include "ipc.h"

#define IPC_CH "@m_lm_get"
#define IPC_RELAY_CH "@m_lm_reply"
#define IPC_QUERY_CH "@m_lm_query"
#define EEPROM_EDID_MAX_LEN     1024
#define EDID_MAXLEN             256
#define EEPROM_CONTENT          "/sys/devices/platform/videoip/eeprom_content"
#define EEPROM_CONTENT_EX          "/sys/devices/platform/videoip/edid_cache"
enum cmd_type {
	CMD_QUERY = 0,
	CMD_GET,
	CMD_SET
};

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

static void HexToAscii(unsigned char *pHex, char *pAscii, int nLen)
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

static void write_eeprom(unsigned char *edid_data)
{
    char eeprom_edid_buf[EEPROM_EDID_MAX_LEN] = {0};
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

static void do_handle_relay_msg(ipc_relay_msg *msg)
{
    int i = 0;
    if (1 == App_EDID_IsValid(msg->rawEdid))
    {
        write_eeprom(msg->rawEdid);
    }
    else
    {
        printf("read edid failed\n");
    }

}

int main(int argc, char *argv[])
{
    enum cmd_type cmd_type = CMD_QUERY;
	unsigned char *ipc_ch = IPC_QUERY_CH;
	FILE *lock_file = NULL;
    ipc_relay_msg buffer;

	set_signal();
    memset(&buffer, 0, sizeof(buffer));

	if (cmd_type == CMD_QUERY) {
		lock_file = lock_ipc_query(ipc_ch);

		if (lock_file == NULL)
			goto err;
	}

	/* open ipc channel */
	int ipc_fd = create_uds_client(ipc_ch);
	if (ipc_fd < 0) {
		goto err;
	}
	/* do the command */
	switch (cmd_type) {
	case CMD_QUERY:
        ipc_set(ipc_fd, argv[1], strlen(argv[1]));
        ipc_get(ipc_fd,&buffer,sizeof(buffer));
        close(ipc_fd);
		break;
	default:
		goto err;
	}

	if (lock_file)
		unlock_ipc_query(lock_file);

    do_handle_relay_msg(&buffer);

	return 0;
err:
	if (ipc_fd >= 0)
		close(ipc_fd);

	if (lock_file)
		unlock_ipc_query(lock_file);

	return -1;
    return 0;
}
