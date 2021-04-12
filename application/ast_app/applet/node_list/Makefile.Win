
cc=gcc

cflags=-O3 -g
ldflags=-g

OBJ = node_list.o astnetwork.o

default: build
	@echo "Done"

build: node_list

node_list: $(OBJ)
	$(cc) $(ldflags) -o node_list $(OBJ)

%.o: %.c Makefile
	${cc} ${cflags} -c $<
	
clean:
	rm -f *.o
	rm -f node_list
