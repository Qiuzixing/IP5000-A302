
#include <stdio.h>
#include <sys/stat.h>
#include <pthread.h>

#include "msg_queue.h"
#include "udp_socket.h"
#include "recv_button_status.h"

pthread_mutex_t g_lock;
static int udp_socket = -1;


int recv_button_init()
{
    int err = 0;
    pthread_mutex_init(&g_lock, NULL);

	udp_socket = udp_init();
	if (udp_socket < 0)
	{
		return -1;
	}

	err = udp_bind(udp_socket, "127.0.0.1", 10200);
	if (err == -1)
    {
        printf("udp_bind fail \n");
        return -1;
    }
	
	err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
	
    err = msg_queue_destroy();
    if (err != 0)
    {
        printf("msg_queue_destroy fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }
   
    err = msg_queue_create();
    if (err == -1)
    {
        printf("msg_queue_create fail - [%s:%d]\n", __func__, __LINE__);
        return -1;
    }

    chmod("/var/run/send_key_pressed_info", S_IXUSR|S_IXGRP|S_IXOTH); //

    return 0;
}


int recv_button_status()
{
    int err = 0;
    struct msg_buf msg;
	char recv_buf[100] = {0};
	
	err = recv_event(udp_socket, recv_buf);
	if (err == 0)
	{
	    pthread_mutex_lock(&g_lock);
	    err = msg_recv_state(&msg);
	    if (err == -1)
	    {
	        printf("msg_recv_state fail");
	        return -1;
	    }
	    pthread_mutex_unlock(&g_lock);
	    
	    return msg.mtext[0];
	}
	else if(err == -1) //error: err = -1
	{
		return FAULT;
	}
	else if (err == -2) // timeout: err = -2
	{
		return TIMEOUT;
	}
	
}



