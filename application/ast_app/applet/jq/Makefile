include $(AST_SDK_ROOT)/ast.cfg

TARGET_FOLDER_NAME=$(TARGET)-$(CONFIG)

CC  = $(CROSS_COMPILE)gcc 
CXX = $(CROSS_COMPILE)g++ 
LD  = $(CROSS_COMPILE)ld  
AR  = $(CROSS_COMPILE)ar

export INCLUDE=$(AST_SDK_ROOT)/usr/include
export C_INCLUDE_PATH=$(AST_SDK_ROOT)/usr/include
export CPLUS_INCLUDE_PATH=$(AST_SDK_ROOT)/usr/include

LIBJQ_SRC = src/builtin.c src/bytecode.c src/compile.c src/execute.c    \
        src/jq_test.c src/jv.c src/jv_alloc.c src/jv_aux.c              \
        src/jv_dtoa.c src/jv_file.c src/jv_parse.c src/jv_print.c       \
        src/jv_unicode.c src/linker.c src/locfile.c src/util.c          \
        src/decNumber/decContext.c src/decNumber/decNumber.c            \
        src/jv_dtoa_tsd.c src/lexer.c src/parser.c src/main.c

CFLAGS = -I./ -Wextra -Wall -std=c99           \
        -Wno-unused-parameter -Wno-unused-function                      \
        -DHAVE_STRPTIME -DIEEE_8087

bin_PROGRAMS = src/jq

ver="`{ ./scripts/version || echo '$(VERSION)' ; } | sed 's/.*/\#define JQ_VERSION \"&\"/'`"

default: clean
	echo "$(ver)" > src/version.h
	sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/\\n"/' ./src/builtin.jq > src/builtin.inc
	make build

build: jq
	mkdir -p $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/
	cp -f $(bin_PROGRAMS) $(AST_SDK_ROOT)/build/$(TARGET_FOLDER_NAME)/rootfs_addon/usr/local/bin/

jq: $(LIBJQ_SRC:.c=.o)
	$(CC) -o $(bin_PROGRAMS) $(LIBJQ_SRC:.c=.o) -lm -lpthread

clean:
	rm -f $(bin_PROGRAMS) $(LIBJQ_SRC:.c=.o)

