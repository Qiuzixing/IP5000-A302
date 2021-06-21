#GPULS = arm-linux-gnueabihf-gcc
include ../conf.mak


SRCS                +=  list.c
SRCS                +=  lldpcontrol.c
OUTINCLUDE := ../out/include
INCLUDEDIR += ../libdir

LIB_PATH += ../libdir/liblldpctl.a


OBJS                :=  $(SRCS:%.c=%.o)

COMPILE         = $(CC) -std=c99 -I$(INCLUDEDIR) -I./ -c "$<" -o "$@" 


LLDPOBJ += lldpctl/*.o

LIBTARGET = libusrctl.a


.PHONY:all

$(shell echo "#define DAEMON_SOCKET_PATH" \"$(PREFIX)"/var/run/lldpd.socket\"" > ctlconfig.h)
all:$(LIBTARGET)
$(LIBTARGET):$(OBJS)
	$(AR) -x ../libdir/liblldpctl.a
	mv *.o ./lldpctl
	$(AR) rcs $(LIBTARGET) $(LLDPOBJ)
	cp libusrctl.a ../out
	cp list.h $(OUTINCLUDE)
	cp lldpcontrol.h $(OUTINCLUDE)
	cp ctlconfig.h $(OUTINCLUDE)
$(OBJS) :%.o : %.c
	@echo complie $@
	$(COMPILE)

clean:
	$(RM) -rf $(LLDPOBJ) $(LIBTARGET)  ctlconfig.h
	
