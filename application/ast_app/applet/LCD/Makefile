
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=$(CROSS_COMPILE)gcc

#CC=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-gcc
#AR=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/bin/arm-aspeed-linux-gnu-ar

LCD=lcd_display
SEND=send_key_info

OBJS1 += ./common/driver_i2c.o ./common/msg_queue.o  ./common/oled.o ./p3k_client/tcp_client.o ./p3k_client/send_p3k_cmd.o
OBJS1 +=  main.o  ./src/IPE5000_info_setting.o  ./src/IPD5000_info_setting.o ./src/IPE5000P_info_setting.o

OBJS2 +=  send_key_info.o ./common/msg_queue.o

CFLAGS= -I./p3k_client -I./src -I./common 
LDFLAGS = 

default:clean build_lcd build_send 
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(LCD)  $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(SEND)  $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	
build_lcd:$(OBJS1)
	$(CC) $^ -o $(LCD) $(LDFLAGS)
	
build_send:$(OBJS2)
	$(CC) $^ -o $(SEND) $(LDFLAGS)
	
%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS)
	
clean:
	rm -f $(LCD) $(SEND)  $(OBJS1) $(OBJS2)

