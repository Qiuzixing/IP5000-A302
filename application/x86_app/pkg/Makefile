cc=gcc
cflags=-O3 -g
ldflags=-g

all: pkg

pkg : pkg.o
	$(cc) $(ldflags) -o pkg pkg.o
	cp -f pkg ../../../x86_bin

%.o : %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o pkg
