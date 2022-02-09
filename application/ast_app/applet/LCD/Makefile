
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc

#CC=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
#AR=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

LCD=lcd_display

OBJS += ./common/driver_i2c.o ./common/msg_queue.o  ./common/oled.o ./common/recv_button_status.o ./common/udp_socket.o  ./p3k_client/tcp_client.o ./p3k_client/send_p3k_cmd.o
OBJS +=  main.o  ./src/IPE5000_info_setting.o  ./src/IPD5000_info_setting.o ./src/IPE5000P_info_setting.o
 
LDFLAGS = -lpthread
CFLAGS = -I ./src -I./common -I./p3k_client

default:clean build_get_button build_lcd
	chmod +x $(LCD)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(LCD)  $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

build_lcd:$(OBJS)
	$(CC) $^ -o $(LCD) $(LDFLAGS)

build_get_button:
	make -C recv_butoon_event

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)
	
clean:
	make clean -C recv_butoon_event
	rm -f $(LCD)  $(OBJS)

