
cc=gcc

cflags=-O3 -g
ldflags=-g

OBJ_QUERY = node_query.o astparam.o astnetwork.o

default: build
	@echo "Done"

build: clean node_query

node_query: $(OBJ_QUERY)
	$(cc) $(ldflags) -o node_query $(OBJ_QUERY)

%.o: %.c Makefile
	${cc} ${cflags} -c $<

clean:
	rm -f *.o
	rm -f node_query.exe
