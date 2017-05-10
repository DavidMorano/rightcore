# MAKEFILE

T= libpcs.a

PCS= /usr/add-on/pcs

SRCROOT=${PCS}


LIBDIR= $(PCS)/lib/$(OFD)
INCDIR= $(PCS)/include
BINDIR= $(PCS)/bin


#CC= cc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

INCDIRS= -I$(INCDIR)

CFLAGS= $(CCOPTS) $(INCDIRS) -D$(OS)=1

LD= $(CC)

LDFLAGS=

LIBDIRS=

LIBS=


# default rules

.s.o:
	$(AS) -o $@ $<
	ar -cr $(T) $@

.c.o:
	$(CC) -c $(CFLAGS) $<
	ar -cr $(T) $@

.c~.c:
	null "want to get $@"

.s~.s:
	null "want to get $@"

.h~.h:
	null "want to get $@"


# files

OBJ0= gecosname.o getgecos.o
OBJ1= logfile.o errfile.o 
OBJ2= mkjobfile.o mkdatefile.o
OBJ3= ns_mailname.o mailname.o
OBJ4= ns_fixgecos.o fixgecos.o
OBJ5= mheader.o base64.o
OBJ= $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(OBJ5)


$(T):		$(OBJ) Makefile

install:	$(T)
	bsdinstall $(T) $(LIBDIR)
	ranlib $(LIBDIR)/$(T)

clean:
	rm -f *.o $(T)



