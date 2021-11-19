
#include <stdio.h>
#include "msg_queue.h"

int main(int argc, char* argv[])
{
    int err = 0;
    int key_value;
    key_value = argv[1][0] -'0';
    
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
    
}


