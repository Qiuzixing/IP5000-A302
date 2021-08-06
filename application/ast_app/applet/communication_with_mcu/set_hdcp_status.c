#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "set_hdcp_status.h"

#define HDCP_DISABLE	    '0'
#define HDCP_V_1X	        '1'
#define HDCP_V_20	        '2'
#define HDCP_V_21	        '3'
#define HDCP_V_22	        '4'
#define HDCP_V_11_AND_22	'5'

void set_hdcp_status(const char *file_name,unsigned char value)
{
    int fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error:%s", file_name,strerror(errno));
        return;
    }
    int is_client = 0;
    if(0 == strcmp(file_name,RX_HDCP_STATUS_DIR))
    {
        is_client = 1;
    }
    else
    {
        is_client = 0;
    }
    
    ssize_t ret = 0;
    switch(value)
    {
        case 0:
            value = HDCP_DISABLE;
            break;
        case 1:

            value = HDCP_V_1X;
            break;
        case 2:
            if(is_client == 1)
            {
                value = HDCP_V_22;
            }
            else
            {
                value = '2';
            }
            break;
        case 3:
            value = HDCP_V_11_AND_22;
            break;
        default:
            break;
    }
    ret = write(fd,&value,1);
    
    if (1 != ret)
    {
        printf("write %s error:%s\n", file_name,strerror(errno));
        close(fd);
        return;
    }
    close(fd);
}
