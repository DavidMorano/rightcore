# MAKEFILE

T= testssh

SRCROOT= $(LOCAL)


CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


#CFLAGS= -I.


LIBSDIRS= -L$(LIBDIR)/lib/S5

LIBS= -ldam -lb -luc -lu -lsocket -lnsl

ARS= libdam.a libb.a libuc.a libu.a


OBJ= main.o

OBJS= $(OBJ) levomods.o


.SUFFIXES:		.ls .i .cx .cs


default:	$(T) Makefile


$(T):		$(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBDIRS) $(LIBS)

install:	$(T)
	bsdinstall $(T) $(SRCROOT)/bin

clean:
	rm -f $(T).x *.o


levomods.o:	levomods.obj
	cp -p levomods.obj levomods.o


main.o:		main.c ssh.h
	$(CC) -c $(CCOPTS) $<



