#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "audio_switch_cmd.h"

extern audio_inout_info_struct audio_inout_info;
extern uint8_t last_hdmi_in_index;
extern uint8_t board_type_flag;
extern uint8_t mute_flag;
void mDelay(unsigned int msecond)
{
    struct timeval time;
    time.tv_sec = 0; //seconds
    time.tv_usec = msecond * 1000;     //microsecond
    select(0, NULL, NULL, NULL, &time);
}

void set_io_select(io_select_value value)
{
    int fd = open(IO_SELECT, O_RDWR);
    char io_value[32] = {0};
    if (fd < 0)
    {
        printf("open %s error:%s", IO_SELECT,strerror(errno));
        return;
    }

    ssize_t ret = 0;
    switch(value)
    {
        case HDMI:
            memcpy(io_value,"hdmi",strlen("hdmi"));
            break;
        case ANALOG_IN:
            memcpy(io_value,"analog",strlen("analog"));
            break;
        case ANALOG_OUT:
            memcpy(io_value,"out_analog",strlen("out_analog"));
            break;
        case IO_ANALOG_OUTMUTE:
            memcpy(io_value,"mute_out_analog",strlen("mute_out_analog"));
            break;
        default:
            break;
    }
    ret = write(fd,io_value,strlen(io_value));
    
    if (strlen(io_value) != ret)
    {
        printf("write %s error:%s\n", IO_SELECT,strerror(errno));
        close(fd);
        return;
    }
    close(fd);
}

void mute_control(const char *file_name,mute_value value)
{
    int fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error:%s", file_name,strerror(errno));
        return;
    }

    ssize_t ret = 0;
    if(value == MUTE)
    {
        if(0 == strcmp(file_name,DANTE_MUTE))
        {
            ret = write(fd, "1", 1);
        }
        else
        {
            ret = write(fd, "0", 1);
        }  
    }
    else
    {
        if(0 == strcmp(file_name,DANTE_MUTE))
        {
            ret = write(fd, "0", 1);
        }
        else
        {
            ret = write(fd, "1", 1);
        }  
    }
        
    if (1 != ret)
    {
        printf("write %s error:%s\n", file_name,strerror(errno));
        close(fd);
        return;
    }

    close(fd);

}

static void set_hdmi_mute(mute_value value)
{
    uint16_t cmd = CMD_HDMI_AUDIO_CONTROL;
    struct CmdDataAudioControl ado_mode;
    ado_mode.port = 16;
    ado_mode.enable = 1;
    if(value == MUTE)
    {
        ado_mode.mute = 1;
    }
    else
    {
        ado_mode.mute = 0;
    }
    APP_Comm_Send(cmd, (U8*)&ado_mode, sizeof(ado_mode));
}

static void set_gsv_insert_audio(insert_value value)
{
    uint16_t cmd = CMD_HDMI_SET_AUDIO_INSERT_EXTRACT;
    char insert_param[] = "33:16";
    char no_insert_param[5] = "0:16";
    if(value == INSERT)
    {
        if(board_type_flag == IPE5000 )
        {
            strncpy(insert_param,"32:17",sizeof(no_insert_param));
        }
        do_handle_set_audio_insert_extract(cmd,insert_param);
    }
    else
    {
        switch(last_hdmi_in_index)
        {
            case HDMIRX1:
                strncpy(no_insert_param,"0:16",sizeof(no_insert_param));
                break;
            case HDMIRX2:
                strncpy(no_insert_param,"1:16",sizeof(no_insert_param));
                break;
            case HDMIRX3:
                strncpy(no_insert_param,"2:16",sizeof(no_insert_param));
                break;
            default:
                break;
        }
        if(board_type_flag == IPE5000)
        {
            strncpy(no_insert_param,"0:17",sizeof(no_insert_param));
        }
        do_handle_set_audio_insert_extract(cmd,no_insert_param);
    }
}

static void stop_i2s_driver(void)
{
    int fd = open(I2S_STOP_DRIVER, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error:%s", I2S_STOP_DRIVER,strerror(errno));
        return;
    }
    int ret = write(fd, "1", 1);
    if (1 != ret)
    {
        printf("write %s error:%s\n", I2S_STOP_DRIVER,strerror(errno));
        close(fd);
        return;
    }

    close(fd);
}

static void analog_in_xxx_out(void)
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char dante_out[] = "2:11:1:12:0";
    char hdmi_out[] = "2:32:1:33:0";
    char ast1520_out[] = "3:36:1:37:0:8:0";
    int i = 0;
    int flag = 0;
    int out_lan_flag = 0;
    do_handle_set_gpio_val(cmd,ast1520_out);
    for(i = 0;i <= AUDIO_OUT_TYPE_NUM;i++)
    {
        switch(audio_inout_info.audio_out[i])
        {
            case AUDIO_OUT_DANTE:
                do_handle_set_gpio_val(cmd,dante_out);
                mute_control(DANTE_MUTE,UNMUTE);       
                break;
            case AUDIO_OUT_HDMI:
                set_gsv_insert_audio(INSERT);
                do_handle_set_gpio_val(cmd,hdmi_out);
                break;
            case AUDIO_OUT_LAN:
                out_lan_flag = 1;
                set_gsv_insert_audio(INSERT);
                //do_handle_set_gpio_val(cmd,ast1520_out);
                break;
            case AUDIO_OUT_NULL:
                flag = 1;
                break;
            default:
                break;
        }
        if(flag == 1 )
        {
            break;
        }
    }
    if(out_lan_flag == 0)
    {
        stop_i2s_driver();
    }
    if(i != 0)
    {
        mute_control(ANALOG_IN_MUTE,UNMUTE); 
        mDelay(TIME_OUT);
        set_io_select(ANALOG_IN);
    }
}

static void hdmi_in_xxx_out()
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char dante_out[] = "2:11:0:12:1";
    char analog_out[] = "1:8:1";
    char ast1520_out[] = "2:15:0:35:1";
    set_gsv_insert_audio(NO_INSERT);
    set_hdmi_mute(MUTE);
    int flag = 0;
    int i = 0;
    for(i = 0;i <= AUDIO_OUT_TYPE_NUM;i++)
    {
        switch(audio_inout_info.audio_out[i])
        {
            case AUDIO_OUT_HDMI:
                set_hdmi_mute(UNMUTE);
                break;
            case AUDIO_OUT_DANTE:
                do_handle_set_gpio_val(cmd,dante_out);
                mute_control(DANTE_MUTE,UNMUTE);
                break;
            case AUDIO_OUT_ANALOG:
                do_handle_set_gpio_val(cmd,analog_out);
                mute_control(ANALOG_OUT_MUTE,UNMUTE);
                set_io_select(ANALOG_OUT);
                break;
            case AUDIO_OUT_LAN:
                do_handle_set_gpio_val(cmd,ast1520_out);
                break;
            case AUDIO_OUT_NULL:
                flag = 1;
                break;
            default:
                break;
        }
        if(flag == 1 )
        {
            break;
        }
    }
}

static void dante_in_xxx_out()
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char hdmi_out[] = "2:32:0:33:1";
    char analog_out[] = "3:36:0:37:1:8:0";
    char ast1520_out[] = "2:15:1:35:0";
    int flag = 0;
    int i = 0;
    for(i = 0;i <= AUDIO_OUT_TYPE_NUM;i++)
    {
        switch(audio_inout_info.audio_out[i])
        {
            case AUDIO_OUT_HDMI:
                set_gsv_insert_audio(INSERT);
                do_handle_set_gpio_val(cmd,hdmi_out);
                break;
            case AUDIO_OUT_ANALOG:
                do_handle_set_gpio_val(cmd,analog_out);
                mute_control(ANALOG_OUT_MUTE,UNMUTE);
                set_io_select(ANALOG_OUT);
                break;
            case AUDIO_OUT_LAN:
                set_gsv_insert_audio(INSERT);
                do_handle_set_gpio_val(cmd,ast1520_out);
                break;
            case AUDIO_OUT_NULL:
                flag = 1;
                break;
            default:
                break;
        }
        if(flag == 1 )
        {
            break;
        }
    }
    
}

void all_switch_set_high(void)
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char all_switch[] = "9:15:1:35:1:8:1:36:1:37:1:32:1:33:1:11:1:12:1";
    do_handle_set_gpio_val(cmd,all_switch);
}

void audio_switch(void)
{
    switch(audio_inout_info.audio_in)
    {
        case AUDIO_IN_ANALOG:
            analog_in_xxx_out();
            break;
        case AUDIO_IN_DANTE:
            dante_in_xxx_out();
            break;
        case AUDIO_IN_HDMI:
            hdmi_in_xxx_out();
            break;
        case AUDIO_IN_NONE:     //Set all switches to 1 to turn off all audio sources
            break;
        default:
            break;
    }
    return;
}

static void ipe5000_and_ipe5000w_analog_in_xxx_out(void)
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char ast1520_out[] = "1:72:0";
    int flag = 0;
    int i = 0;

    set_gsv_insert_audio(INSERT);
    do_handle_set_gpio_val(cmd,ast1520_out);
    set_io_select(ANALOG_IN);
    if(mute_flag == UNMUTE)
    {
        set_hdmi_mute(UNMUTE);
        mute_control(ANALOG_IN_MUTE,UNMUTE);
    }

#if 0
    for(i = 0;i <= AUDIO_OUT_TYPE_NUM;i++)
    {
        switch(audio_inout_info.audio_out[i])
        {
            case AUDIO_OUT_HDMI:
            case AUDIO_OUT_LAN:
                set_gsv_insert_audio(INSERT);
                do_handle_set_gpio_val(cmd,ast1520_out);
                set_io_select(ANALOG_IN);
                break;
            case AUDIO_OUT_NULL:
                flag = 1;
                break;
            default:
                break;
        }
        if(flag == 1 )
        {
            break;
        }
    }
#endif
}

static void ipe5000_and_ipe5000w_hdmi_in_xxx_out(void)
{
    uint16_t cmd = CMD_GPIO_SET_VAL;
    char it6802_out[] = "1:72:1";
    int flag = 0;
    int i = 0;
    set_hdmi_mute(MUTE);
    mute_control(ANALOG_OUT_MUTE,MUTE);
    do_handle_set_gpio_val(cmd,it6802_out);
    //set_io_select(IO_ANALOG_OUTMUTE);
    set_gsv_insert_audio(NO_INSERT);
    set_io_select(HDMI);
    set_io_select(ANALOG_OUT);
    mDelay(100);
    if(mute_flag == UNMUTE)
    {
        set_hdmi_mute(UNMUTE);
        mute_control(ANALOG_OUT_MUTE,UNMUTE);
    }


#if 0
    for(i = 0;i <= AUDIO_OUT_TYPE_NUM;i++)
    {
        switch(audio_inout_info.audio_out[i])
        {
            case AUDIO_OUT_ANALOG:
                set_hdmi_mute(UNMUTE);
                set_io_select(HDMI);
                set_io_select(ANALOG_OUT);
                break;
            //Web HDMI_and_LAN merged into one API
            case AUDIO_OUT_LAN:
            case AUDIO_OUT_HDMI:
                set_io_select(HDMI);
                set_gsv_insert_audio(NO_INSERT);
                set_hdmi_mute(UNMUTE);
                break;
            case AUDIO_OUT_NULL:
                flag = 1;
                break;
            default:
                break;
        }
        if(flag == 1)
        {
            break;
        }
    }
#endif

}

static ipe5000_and_ipe5000w_none_in_xxx_out(void)
{
    mute_control(ANALOG_OUT_MUTE,UNMUTE);
    set_hdmi_mute(MUTE);
    mute_control(ANALOG_IN_MUTE,MUTE);
}

void ipe5000_and_ipe5000w_autoaudio_control(void)
{
    switch(audio_inout_info.audio_in)
    {
        case AUDIO_IN_ANALOG:
            ipe5000_and_ipe5000w_analog_in_xxx_out();
            break;
        case AUDIO_IN_HDMI:
            ipe5000_and_ipe5000w_hdmi_in_xxx_out();
            break;
        case AUDIO_IN_NONE:
            ipe5000_and_ipe5000w_none_in_xxx_out();
            break;
        default:
            break;
    }
}
