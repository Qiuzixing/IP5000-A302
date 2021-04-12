cc=gcc

#prefix=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/arm-aspeed-linux-gnu
#gcc_include_dir=/opt/crosstool/gcc-3.4.2-glibc-2.3.3/arm-aspeed-linux-gnu/lib/gcc/arm-aspeed-linux-gnu/3.4.2/include
#system_dir=$(prefix)/include
#linux_include_dir=$(prefix)/include/linux
#rootfs=../../../rootfs

cflags=-O3 -g
ldflags=-g

host : all

client : all

all : astparam

astparam : astparam.o
	$(cc) $(ldflags) -o astparam astparam.o

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f astparam
