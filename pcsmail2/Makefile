# MAKEFILE
#
# Thu May 31 16:45:42 EDT 1984
# Makefile for PCS mail sending program which serves the functions of
# sending mail, info, pc and mail receiving deamon


#PCS= /home/gwbb/add-on/pcs

BINDIR= $(PCS)/bin
INCDIR= $(PCS)/include
LIBDIR= $(PCS)/lib/$(OFD)

T= pcsmail

ALL= $(T)


CC= gcc
GPP= g++

CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing

CCOPTS_GCCLIB= -fpic $(CCOPTS_GCCLIBSPARC)

#CCOPTS= $(CCOPTS_CCALL) -g -xs


DEFINES= -D$(OS)=1 -DPCS=\"$(PCS)\"
INCDIRS= -I$(INCDIR)
CFLAGS= $(DEFINES) $(INCDIRS)

LD= $(CC)

LDFLAGS=

LIBDIRS= -L$(LIBDIR)

LIBS= -lparser -lpcs -lmisc -lb -lu -lnsl


HDRS=   config.h header.h prompt.h defs.h

SRC=	defs.c bld_links.c \
	checkit.c checkname.c checklist.c cmpnames.c cookname.c \
	deliv_mbag.c deliverem.c doinfo.c editit.c fappend.c fetchfrom.c \
	findname.c  genname.c \
	getheader.c getpchdr.c gettable.c getvars.c \
	header.c help.c mailit.c nmalloc.c\
	pipeit.c prompt.c putheader.c copymessage.c redomsg.c writeheader.c \
	reviewit.c main.c \
	date_envelope.c date_header.c \
	getnames.c getname.c \
	mm_getfield.c getfield.c debugging.c isheader.c skipheaders.c \
	bopen_wall.o process.c inter.c


OBJ= $(SRC:.c=.o)


.c.o:			config.h defs.h
	$(CC) -c $(CFLAGS) $*.c


all:			$(ALL)

$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

elf $(T).elf:		$(T).x
	strip $(T).x
	cp $(T).x $(T).elf

aout $(T).aout:		$(T).x
	strip $(T).x
	cp $(T).x $(T).aout

clean:
	-rm -f *.o $(ALL)

install:		$(ALL)
	rm -f n$(T) n$(T).x n$(T).elf
	-ln $(T).x $(T).$(OFF)
	-ln $(T) n$(T) 2> /dev/null
	-ln $(T).$(OFF) n$(T).$(OFF) 2> /dev/null
	bsdinstall $(T) n$(T) $(BINDIR)
	bsdinstall $(T).$(OFF) n$(T).$(OFF) $(BINDIR)

control:
		uname -n > Control
		date >> Control


defs.o:		defs.c defs.h

main.o:		main.c $(HDRS)
	$(CC) -c $(CFLAGS) -DMASTER=1 main.c

bld_links.o:	defs.h config.h

checkit.o:	defs.h config.h header.h

checkname.o:	defs.h config.h

checklist.o:	defs.h config.h

cmpnames.o:	defs.h config.h

getname.o:	defs.h config.h

cookname.o:	defs.h config.h

deliv_mbag.o:	defs.h config.h 

deliverem.o:	defs.h config.h

doinfo.o:	defs.h config.h

editit.o:	defs.h config.h header.h

fappend.o:	defs.h config.h

fetchfrom.o:	defs.h config.h

findname.o:	defs.h config.h

genname.o:	defs.h config.h

getheader.o:	defs.h config.h header.h

getnames.o:	defs.h config.h

getoptions.o:	defs.h config.h

getpchdr.o:	defs.h config.h header.h

gettable.o:	defs.h config.h

getvars.o:	defs.h config.h

header.o:	defs.h config.h header.h

mailit.o:	defs.h config.h  header.h

netpost.o:	defs.h config.h header.h

nmalloc.o:	defs.h config.h

pipeit.o:	defs.h config.h header.h

prompt.o:	defs.h config.h prompt.h header.h

putheader.o:	defs.h config.h

redomsg.o:	defs.h config.h header.h

reviewit.o:	defs.h config.h header.h


