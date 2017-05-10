# MAKEFILE

T= mc68pas

ALL= $(T) $(T).$(OFF)

#M68TOOLS= /home/gwbb/add-on/m68tools


BINDIR= $(M68TOOLS)/bin
INCDIR= $(M68TOOLS)/include
LIBDIR= $(M68TOOLS)/lib


#CC= cc
CCOPTS= -g


DEFINES0= -D$(OSTYPE)=1 -DSOLARIS=1 
DEFINES1= -DPOSIX=1 -D_POSIX_PTHREAD_SEMANTICS=1 -D_REENTRANT=1
DEFINES2= -D_POSIX_C_SOURCE=199506L -D_POSIX_PER_PROCESS_TIMER_SOURCE=1
DEFINES3= -D__EXTENSIONS__=1
DEFINES= $(DEFINES0) $(DEFINES1) $(DEFINES2) $(DEFINES3)
INCDIRS= -I$(INCDIR) -I/home/dam/include
CFLAGS= $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)
LIBS= -ldam -lb -lgen -lnsl

#CRT0= /lib/crt0.o
CRTN=
CRTC=
CRT0=



.SUFFIXES:	.t



INCS=

OBJ0=
OBJ1= main.o whatinfo.o
OBJ2=

OBJ= $(OBJ1) $(OBJ2)

OBJS= $(CRT0) $(OBJ) $(CRTC)




default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.t:
	lint -uxn $(DEFINES) $(INCDIRS) $<


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN)

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF)

install:		$(ALL)
	bsdinstall $(ALL) $(BINDIR)
 
again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	uname -n >> Control
	date > Control


tp.o:		tp.c


tp:		tp.o
	$(LD) -o $@ $(LDFLAGS) $(CRT0) tp.o -L$(LIBDIR) $(LIBS)



main.o:		main.c config.h defs.h

whatinfo.o:	whatinfo.c config.h


main.t:		main.c

watch.t:	watch.c

srvfile.t:	srvfile.c

configfile.t:	configfile.c

quoted.t:	quoted.c

exapnd.t:	expand.c



