# MAKEFILE (readmail)

T= readmail

# PCS= /home/dam/add-on/pcs


PCSBIN= $(PCS)/bin
PCSINC= $(PCS)/include
PCSLIB= $(PCS)/lib/$(OFD)


#CC= cc
DEFINES= -D$(OS)=1 -DPCS=\"$(PCS)\"
INCDIRS= -I$(PCSINC)
CFLAGS= -O $(DEFINES) $(INCDIRS)

LD= $(CC)
LDFLAGS=
LIBDIRS= -L$(PCSLIB)
LIBS= -lparser -lmisc -lb -lnsl



SRC=    defs.c booleval.c boxname.c delete.c display.c \
   	forward.c getfield.c hmatch.c gsearch.c help.c inter.c \
	leparse.c list.c logexpr.c main.c messcheck.c \
	profile.c putfile.c range.c reply.c rewrite.c \
	save.c scan.c search.c setup.c undelete.c output.c \
	usystem.c ssystem.c

OBJ= $(SRC:.c=.o)


ALL= $(T) $(T).elf


all:		$(ALL)

$(T):		$(T).sh
	cp $(T).sh $(T)

$(T).x:		$(OBJ)
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).elf elf:	$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

$(T).aout aout:	$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

clean:
	rm -f *.o

install:	$(ALL)
	rm -f n$(T) n$(T).*
	-ln $(T) n$(T) 2> /dev/null
	-ln $(T).elf n$(T).elf 2> /dev/null
	bsdinstall $(T) n$(T) ${PCSBIN}
	bsdinstall $(T).elf n$(T).elf $(PCSBIN)

control:
	uname -n > Control
	date >> Control



main.o:		main.c defs.h config.h

profile.o:	defs.h profile.h

$(OBJ):		defs.h



