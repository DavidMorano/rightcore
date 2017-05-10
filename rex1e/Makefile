# MAKEFILE

T= rex

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


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
#CCOPTS= -xO5 -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -xtarget=ultra -xsafe=mem -xdepend


DEF0=
DEF1=
DEF2=
DEF3=
DEF4=
DEF5=
DEF6=
DEF7= $(LF_DEFS)

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)


INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)

CFLAGS= $(CCOPTS)

#LD= $(CC)
#LD= cc
LD= ld

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR)


LIBDIRS= -L$(LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb -lut
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

CRT0= $(CRT1) $(CRTI) $(VALUES)
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


OBJ00= main.o whatinfo.o proginfo.o proginfo_setpiv.o proginfo_rootname.o
OBJ01= proglog.o proguserlist.o 
OBJ02=
OBJ03=
OBJ04= 
OBJ05= 
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o
OBJ08= 
OBJ09= vecstr_avmkstr.o
OBJ10=
OBJ11=
OBJ12= opendial.o getopendial.o
OBJ13= sbuf.o sbuf_addquoted.o
OBJ14=
#OBJ15= uss.o ussmux.o ussinfo.o prog.o

OBJ16= dialuss.o dialussmux.o dialussnls.o
OBJ17= dialtcp.o dialtcpmux.o dialtcpnls.o
OBJ18= dialticotsord.o dialticotsordnls.o 
OBJ19= 
OBJ20=
OBJ21=
OBJ22= 
#OBJ23= inetaddrparse.o hasleadcolon.o 

OBJ24= mkquoted.o
OBJ25= 
OBJ26=
OBJ27=
OBJ28=
OBJ29= getaf.o 
OBJ30=
OBJ31=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


SRC= $(OBJ:.c=.o)


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


$(T):			$(T).ee
	cp -p $(T).ee $(T)

$(T).x:			$(OBJ) Makefile
	makedate > makedate.c
	$(CC) -c $(CFLAGS) makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:			$(T).x
	strip $(T).x
	rm -f $(T).$(OFF) $(T)

install:		install-raw install-help

install-ee:		$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install-raw:		$(T).x
	rm -f $(BINDIR)/$(T).$(OFF)
	makenewer -r -o rmsuf $(T).x $(BINDIR)

install-help:		$(T).help
	-mkdir -p $(HELPDIR) 2> /dev/null
	makenewer -o rmsuf $(T).help $(HELPDIR)

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.ls *.ln *.i *.o *.so *.x *.lm

control:
	uname -n > Control
	date >> Control


main.o:			main.c $(INCS)

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

proglog.o:		proglog.c defs.h

proguserlist.o:		proguserlist.c defs.h


opendial.o:		opendial.c opendial.h


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr_avmkstr.o:	vecstr_avmkstr.c

filebuf.o:		filebuf.c filebuf.h

streamsync.o:		streamsync.c streamsync.h

nodesfile.o:		nodesfile.c nodesfile.h

envlist.o:		envlist.c envlist.h

hdb.o:			hdb.c hdb.h

lookaside.o:		lookaside.c lookaside.h

pq.o:			pq.c pq.h

hdbstr.o:		hdbstr.c hdbstr.h

hdbstr_loadfile.o:	hdbstr_loadfile.c

msfile.o:		msfile.c msfile.h msfilee.h msflag.h

msfilee.o:		msfilee.c msfilee.h

mapstrint.o:		mapstrint.c mapstrint.h

mkcexsync.o:		mkcexsync.c mkcexsync.h

nodesearch.o:		nodesearch.c nodesearch.h

nodedb.o:		nodedb.c nodedb.h

clusterdb.o:		clusterdb.c clusterdb.h kvsfile.h

svctab.o:		svctab.c svctab.h

strstore.o:		strstore.c strstore.h

serialbuf.o:		serialbuf.c serialbuf.h

kvsfile.o:		kvsfile.c kvsfile.h

sbuf.o:			sbuf.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

buffer.o:		buffer.c buffer.h

expcook.o:		expcook.c expcook.h

inetaddr.o:		inetaddr.c inetaddr.h

linefold.o:		linefold.c linefold.h

termout.o:		termout.cc termout.h ansigr.h
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) termout.cc

sysvar.o:		sysvar.c sysvar.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

prog.o:			prog.c prog.h sysdialer.h envs.h sysvar.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

uss.o:			uss.c uss.h inetaddrparse.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ussmux.o:		ussmux.c ussmux.h inetaddrparse.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ussnls.o:		ussnls.c ussnls.h inetaddrparse.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

ussinfo.o:		ussinfo.c ussinfo.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

inetaddrparse.o:	inetaddrparse.c inetaddrparse.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


getopendial.o:		getopendial.c opendial.h

getaf.o:		getaf.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hasleadcolon.o:		hasleadcolon.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


