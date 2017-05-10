# MAKEFILE
# make the library

T= libbsd.a

SRCROOT= $(LOCAL)


INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib/$(OFD)


CC= gcc
GPP= g++

CCOPTS= -O -fpic -mcpu=ultrasparc -mno-app-regs

AS= as
AFLAGS=

LD= $(CC)

LDFLAGS=


.s.o:
	$(AS) -o $@ $<
	ar -r $(T) $@

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar -r $(T) $@

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ= bcopy.o bcmp.o bzero.o scandir.o


# specific rules

$(T):		$(OBJ) Makefile

clean:
	rm -f *.o $(T)

install:	$(T)
	cp $(T) $(LIBDIR)

control:
	uname -n > Control
	date >> Control


scandir.o:	scandir.c
	$(CC) -c $(CFLAGS) -DSOLARIS=1 scandir.c
	ar -r $(T) $@


