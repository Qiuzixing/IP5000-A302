#ifndef _AUDIO_SWITCH_CMD_H_
#define _AUDIO_SWITCH_CMD_H_

#include "../gb_commun_with_mcu.h"

#define ANALOG_IN_MUTE      "/sys/class/leds/linein_mute/brightness"
#define ANALOG_OUT_MUTE     "/sys/class/leds/lineout_mute/brightness"
#define DANTE_MUTE          "/sys/class/leds/dante_mute/brightness"
#define IO_SELECT           "/sys/devices/platform/1500_i2s/io_select"
#define I2S_STOP_DRIVER     "/sys/devices/platform/1500_i2s/stop"
#define TIME_OUT            500
typedef enum
{
    LOW_LEVEL = 0,
    HIGH_LEVEL = 1
}bool_switch_level;

typedef enum
{
    MUTE = 0,
    UNMUTE = 1
}mute_value;

typedef enum
{
    INSERT = 0,
    NO_INSERT = 1
}insert_value;

typedef enum
{
    HDMI = 0,
    ANALOG_IN = 1,
    ANALOG_OUT,
    IO_ANALOG_OUTMUTE
}io_select_value;

typedef enum
{
    SWITCH_1_OE_1 = 11,     //PA11
    SWITCH_1_OE_2 = 12,     //PA12

    SWITCH_2_OE_1 = 15,     //PA15
    SWITCH_2_OE_2 = 34,     //PB3

    SWITCH_3_OE_1 = 36,     //PB4
    SWITCH_3_OE_2 = 37,     //PB5

    SWITCH_4_OE_1 = 8,     //PA8

    SWITCH_5_OE_1 = 32,     //PB0
    SWITCH_5_OE_2 = 33,     //PB1

    SWITCH_ONLY_1 = 0xff
}audio_switch_list;

void mute_control(const char *file_name,mute_value value);
void all_switch_set_high(void);
void mDelay(unsigned int msecond);
void audio_switch(void);
void ipe5000_and_ipe5000w_autoaudio_control(void);
#endif
