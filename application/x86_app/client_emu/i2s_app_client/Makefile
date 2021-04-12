
#include $(AST_SDK_ROOT)/ast.cfg

#TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC=gcc

BUILD_TYPE=client

default: build
	@echo "Done"

build: client_i2s.o
ifeq ($(BUILD_TYPE),client)
	$(CC) -o client_i2s client_i2s.o -lm
	cp -f client_i2s ../
else
	@echo "Client only app"
endif

clean:
	rm -rf *.o client_i2s

