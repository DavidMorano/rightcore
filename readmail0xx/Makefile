# MAKEFILE ('readmail')

PCS= /usr/add-on/pcs

#SRCROOT= $(LOCAL)


PCSBIN= $(PCS)/bin
PCSINC= $(PCS)/include
PCSLIB= $(PCS)/lib/$(OFD)


#CC = cc

DEFINES= -D$(OS)=1 -DSUN=1

CFLAGS = -O $(DEFINES) -I$(PCSINC)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(PCSLIB)

LIBS= -lparser -lmisc -lb


READMAIL= readmail
AREADMAIL= rdmail


SRC=    defs.c booleval.c boxname.c delete.c display.c \
   	forward.c getfield.c gsearch.c help.c inter.c \
	leparse.c list.c logexpr.c main.c messcheck.c \
	profile.c putfile.c range.c reply.c rewrite.c \
	save.c scan.c search.c setup.c undelete.c output.c \
	mm_getfield.c usystem.c ssystem.c

OBJ = $(SRC:.c=.o)

all:	readmail
	rm -f nreadmail nrdmail
	-ln readmail nreadmail
	-ln readmail nrdmail

$(OBJ): defs.h

profile.o:	defs.h profile.h

readmail:	$(OBJ)
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

clean:
	-rm -f *.o

install:	readmail
	install readmail ${PCSBIN}/nreadmail
	install readmail $(PCSBIN)/nrdmail


