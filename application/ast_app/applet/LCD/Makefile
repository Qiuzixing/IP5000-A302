
CC=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
AR=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

MAIN=lcd_display

OBJS += ./common/driver_i2c.o ./common/msg_queue.o  ./common/oled.o
OBJS +=  main.o  ./src/IPE5000_info_setting.o main.o ./src/IPD5000_info_setting.o ./src/IPE5000P_info_setting.o
#./src/IPD5000_info_setting.o  ./src/IPE5000_info_setting.o main.o  ./src/IPE5000P_info_setting.o

CFLAGS= -I./src -I./common  
LDFLAGS = -lpthread

deault:clean build

build:$(OBJS)
	$(CC) $^ -o $(MAIN) $(LDFLAGS)
	
%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)
	
clean:
	rm -f $(MAIN) $(OBJS)
