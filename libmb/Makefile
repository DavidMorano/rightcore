# MAKEFILE

T= libmb

SRCROOT= $(LOCAL)


LIBDIR= $(PCS)/lib
INCDIR= $(PCS)/include


#CC= cc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

INCDIRS= -I${INCDIR)

CFLAGS= -O -I$(INCDIR)


LD= $(CC)

LDFLAGS=

LIBDIRS=

LIBS=




.s.o:
	$(AS) -o $@ $<
	ar -sr $(T).a $@

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar -r $(T).a $@

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ0= mb_open.o 
#OBJ1= mb_close.o
OBJ1=
OBJ2=
OBJ3=
OBJ4=
OBJ5=
OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)


$(T).a:		$(OBJ) Makefile

install:	$(T).a
	install $(T).a $(LIBDIR)
	ranlib $(LIBDIR)/$(T).a

clean:
	rm -f *.o $(T).a


