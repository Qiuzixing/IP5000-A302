

#include $(AST_SDK_ROOT)/ast.cfg

#TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=gcc

cflags=-O3 -g
ldflags=-g

OBJ = msg_channel.o astnetwork.o

default: build
	@echo "Done"

build: msg_channel
	cp -f msg_channel ../ast_heartbeat
	@#cp -f msg_channel /tftpboot/ast_heartbeat

msg_channel: $(OBJ)
	$(cc) $(ldflags) -o msg_channel $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f msg_channel
