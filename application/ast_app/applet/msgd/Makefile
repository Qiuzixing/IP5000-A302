
include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=-g

OBJ = msgd.o astnetwork.o

default: build
	@echo "Done"

build: MSGD
	if [ "$(BUILD_TYPE)" = 'client' ]; then \
		mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
		cp -f msgd $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/; \
	fi

MSGD: $(OBJ)
	$(cc) $(ldflags) -o msgd $(OBJ)

%.o: %.c Makefile
	${cc} -MMD -MP ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f msgd
	rm -f *.d

# The dependency file names == .c file names
DEPS := $(OBJ:.o=.d)

# Let make read the dependency files and handle them.
-include $(DEPS)