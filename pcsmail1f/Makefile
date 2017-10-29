# MAKEFILE

T= pcsmail

ALL= $(T) $(T).x

#PCS= /home/gwbb/add-on/pcs


BINDIR= $(PCS)/bin
INCDIR= $(PCS)/include
LIBDIR= $(PCS)/lib/$(OFD)


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

# HyperSPARC
#CCOPTS= -xO5 -xtarget=ss20/hs22 -dalign -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -dalign -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR) -I$(PCS)/include

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

# regular
LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(PCS)/lib
# for profiling
LDFLAGS_P= $(LF_LDFLAGS) -L/usr/lib/libp # -R$(LIBDIR):$(PCS)/lib


LIBDIRS= -L$(LIBDIR) -L$(PCS)/lib

LIB0= -lparser lpcs
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

CRTC= makedate.o

LINT= lint
LINTFLAGS= -uxn -Dlint

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


INCS= b_$(T).h defs.h

HDRS=   config.h header.h prompt.h defs.h

SRC=	defs.c bld_links.c\
	checkit.c checkname.c checklist.c cmpnames.c cookname.c \
	deliv_mbag.c deliverem.c doinfo.c editit.c fappend.c fetchfrom.c \
	findname.c  genname.c \
	getheader.c getpchdr.c getoptions.c gettable.c getvars.c \
	header.c help.c mailit.c  nmalloc.c\
	pipeit.c prompt.c putheader.c copymessage.c redomsg.c writeheader.c \
	reviewit.c rmtemp.c main.c \
	date_header.c \
	getnames.c getname.c \
	mm_getfield.c getfield.c debugging.c isheader.c skipheaders.c \
	bopen_wall.o


OBJ= $(SRC:.c=.o)

# for regular (no profiling)
OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)
# for 'prof(1)'
MOBJS= $(MCRT0) $(OBJ) $(CRTC)
# for 'gprof(1)'
GOBJS= $(GCRT0) $(OBJ) $(CRTC)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i

.c.cx:
	$(CXREF) -C $(CXREFFLAGS) $(CPPFLAGS) $<

.c.cs:
	$(CXREF) $(CXREFFLAGS) $(CPPFLAGS) -o $(*).cs $<


$(T):			$(T).sh
	cp $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).$(OFF) $(OFF):	$(T).x
	strip $(T).x
	-cp $(T).x $(T).$(OFF)

install:		$(ALL)
	rm -f n$(T) n$(T).x n$(T).elf
	-ln $(T).x $(T).$(OFF)
	-ln $(T) n$(T) 2> /dev/null
	-ln $(T).$(OFF) n$(T).$(OFF) 2> /dev/null
	bsdinstall $(T) n$(T) $(BINDIR)
	bsdinstall $(T).$(OFF) n$(T).$(OFF) $(BINDIR)

again:
	rm -f $(ALL)

clean:
	rm -f *.o $(ALL)

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


