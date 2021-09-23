

include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)
PREFIX=$(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/prefix

CC=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.

NAME=ldap_daemon

OBJ=ldap_main.o ldap_auth_methods.o ldap_funcs.o notify.o
		
LDFLAGS=-L $(PREFIX)/lib -lssl -lcrypto -lldap -llber -lrt -lpthread
CFLAGS=-I $(PREFIX)/include

default: build
	@echo "Done"
	
build:$(OBJ)
	$(CC) $^ -o $(NAME) $(LDFLAGS)
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(NAME) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/var/run/certs/
	cp -f ./certs/* $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/var/run/certs/
	
%.o:%.c
	$(CC) -c $<   $(CFLAGS) -o $@

clean:
	rm -rf $(OBJ) $(NAME)

