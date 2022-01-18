
#include <stdio.h>
#include "msg_queue.h"
#include "udp_socket.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("send_key_info param error\n");
		return -1;
	}
	
    int err = 0;
	int fd = -1;
    int key_value;
    key_value = argv[1][0] -'0';

	fd = udp_init();
	if (fd < 0)
	{
		return -1;
	}
	
    err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail");
        return -1;
    }
    
    err = msg_send_state(key_value);
    if (err == -1)
    {
        printf("msg_send_state fail");
        return -1;
    }

	send_event(fd, "127.0.0.1", 10200);
	
	close(fd);
	
    return 0;
}


