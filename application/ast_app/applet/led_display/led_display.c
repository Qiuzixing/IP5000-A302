#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <strings.h>
#include <time.h>
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

#define LED_POWER1      "/sys/class/leds/led_display_power1/brightness"
#define LED_POWER2      "/sys/class/leds/led_display_power2/brightness"
#define LED_POWER3      "/sys/class/leds/led_display_power3/brightness"
#define SEG_LEDA        "/sys/class/leds/seg_leda/brightness"
#define SEG_LEDB        "/sys/class/leds/seg_ledb/brightness"
#define SEG_LEDC        "/sys/class/leds/seg_ledc/brightness"
#define SEG_LEDD        "/sys/class/leds/seg_ledd/brightness"
#define SEG_LEDE        "/sys/class/leds/seg_lede/brightness"
#define SEG_LEDF        "/sys/class/leds/seg_ledf/brightness"
#define SEG_LEDG        "/sys/class/leds/seg_ledg/brightness"
#define SEG_LEDDP       "/sys/class/leds/seg_leddp/brightness"

typedef enum
{
    LED_LIGHT_UP = 0,
    LED_LIGHT_DOWN = 1
}led_value;

typedef enum
{
    POWER1 = 0,
    POWER2  ,
    POWER3
}led_power_select;

enum
{
    NUM_0 = 0,
    NUM_1,
    NUM_2,
    NUM_3,
    NUM_4,
    NUM_5,
    NUM_6,
    NUM_7,
    NUM_8,
    NUM_9
};

static void led_file_control(const char *file_name,led_value value)
{
    int fd = open(file_name, O_RDWR);
    if (fd < 0)
    {
        printf("open %s error:%s", file_name,strerror(errno));
        return;
    }

    ssize_t ret = 0;
    if(value == LED_LIGHT_UP)
    {
        ret = write(fd, "0", 1);
    }
    else
    {
        ret = write(fd, "1", 1);
    }

    if (1 != ret)
    {
        printf("write %s error:%s\n", file_name,strerror(errno));
        close(fd);
        return;
    }

    close(fd);
}

static void led_display_num0(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_UP);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_DOWN);
}

static void led_display_num1(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDG,LED_LIGHT_DOWN);
}

static void led_display_num2(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_UP);
    led_file_control(SEG_LEDF,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num3(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num4(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num5(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num6(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_UP);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num7(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDG,LED_LIGHT_DOWN);
}

static void led_display_num8(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_UP);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_display_num9(void)
{
    led_file_control(SEG_LEDA,LED_LIGHT_UP);
    led_file_control(SEG_LEDB,LED_LIGHT_UP);
    led_file_control(SEG_LEDC,LED_LIGHT_UP);
    led_file_control(SEG_LEDD,LED_LIGHT_UP);
    led_file_control(SEG_LEDE,LED_LIGHT_DOWN);
    led_file_control(SEG_LEDF,LED_LIGHT_UP);
    led_file_control(SEG_LEDG,LED_LIGHT_UP);
}

static void led_power_control(led_power_select power_value)
{
    switch(power_value)
    {
        case POWER1:
            led_file_control(LED_POWER1,LED_LIGHT_UP);
            led_file_control(LED_POWER2,LED_LIGHT_DOWN);
            led_file_control(LED_POWER3,LED_LIGHT_DOWN);
            break;
        case POWER2:
            led_file_control(LED_POWER1,LED_LIGHT_DOWN);
            led_file_control(LED_POWER2,LED_LIGHT_UP);
            led_file_control(LED_POWER3,LED_LIGHT_DOWN);
            break;
        case POWER3:
            led_file_control(LED_POWER1,LED_LIGHT_DOWN);
            led_file_control(LED_POWER2,LED_LIGHT_DOWN);
            led_file_control(LED_POWER3,LED_LIGHT_UP);       
            break;
        default:
            break;            
    }
}

static void led_display_num(unsigned char num)
{
    switch(num)
    {
        case NUM_0:
            led_display_num0();
            break;
        case NUM_1:
            led_display_num1();
            break;
        case NUM_2:
            led_display_num2();
            break;
        case NUM_3:
            led_display_num3();
            break;
        case NUM_4:
            led_display_num4();
            break;
        case NUM_5:
            led_display_num5();
            break;
        case NUM_6:
            led_display_num6();
            break;
        case NUM_7:
            led_display_num7();
            break;
        case NUM_8:
            led_display_num8();
            break;
        case NUM_9:
            led_display_num9();
            break;
        default:
            break;                                                                                    
    }
}

static void Delay(int us)
{
    struct timeval  timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = us; 
    select(1, NULL, NULL, NULL, &timeout);
    return;
}

static void print_usage(void)
{
    printf("[Usage]:\n");
    printf(": ./led_display -n xxx\n");
}

int main(int argc, char *argv[])
{
    int opt = 0;
    int long_index =0;
    int display_num = 0;
    unsigned char led_power_num1 = 0;
    unsigned char led_power_num2 = 0;
    unsigned char led_power_num3 = 0;
    static struct option long_options[] = {
        {"num_display",           required_argument,       NULL,  'n' },
		{NULL,                             0,              NULL,  0   }
	};
    while ((opt = getopt_long_only(argc, argv, "n:", long_options, &long_index )) != -1) {
		switch (opt) {
		case 'n':
            display_num = strtoul(optarg, NULL, 0);
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

    if(display_num < 0 || display_num > 999)
    {
        printf("Warning:The range of numbers is 0~999\n");
        return 0;
    }
    printf("display_num = 0x%x\n",display_num);
    led_power_num1 = display_num/100 % 10;
    led_power_num2 = display_num/10 % 10;
    led_power_num3 = display_num % 10;
    while(1)
    {
        led_display_num(led_power_num1);
        led_power_control(POWER1);
        Delay(1);//The select function also needs to run a tick time at the shortest time about 10ms,Therefore, the minimum delay is 10ms
        led_display_num(led_power_num2);
        led_power_control(POWER2);
        Delay(1); 
        led_display_num(led_power_num3);
        led_power_control(POWER3);
        Delay(1);
    }
}
