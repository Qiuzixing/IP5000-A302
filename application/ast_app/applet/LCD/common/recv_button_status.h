
#ifndef __RECV_BUTTON_STATUS_H__
#define __RECV_BUTTON_STATUS_H__

#define SHOW_TIMEOUT  (-2)
#define FAULT -1

//方向
enum 
{
    ENTER_KEY = 1,
    UP_KEY,
    DOWN_KEY,
    LEFT_KEY,
    RIGHT_KEY,
    UP_CONTINUE_PRESS,
    DOWN_CONTINUE_PRESS,
    TIMEOUT,
};


int recv_button_init();
int recv_button_status();
int run_recv_butoon_event();

#endif


