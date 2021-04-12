
CROSS_COMPILE=
cc=$(CROSS_COMPILE)gcc

cflags=-O3 -g
ldflags=

cflags+=-W -Wstrict-prototypes -std=gnu99 -I.


OBJ = soip.o


default: build
	@echo "Done"

build: $(OBJ)
	$(cc) $(ldflags) -o soip $(OBJ)

clean:
	rm -f *.o
	#rm -f *.o *.c

%.o : %.c Makefile
	${cc} ${cflags} -c $<
