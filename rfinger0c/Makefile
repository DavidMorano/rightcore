# MAKEFILE

T= rfinger

ALL= $(T)

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib
MANDIR= $(SRCROOT)/man/man1
HELPDIR= $(SRCROOT)/share/help

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS_GCCALL= -pthreads -fexceptions
CCOPTS_CCALL= -MT

CCOPTS_GCCOPTSGPP= $(CGS_CCFLAGS)
CCOPTS_GCCOPTSPARC= -mcpu=ultrasparc
CCOPTS_GCCLIBSPARC= -mno-app-regs

CCOPTS= $(CCOPTS_GCCALL) -O $(CCOPTS_GCCOPTSPARC)
#CCOPTS= $(CCOPTS_GCCALL) -g -O
#CCOPTS= $(CCOPTS_GCCALL) -g -pg
#CCOPTS= $(CCOPTS_GCCALL) -g -Wstrict-aliasing
#CCOPTS= $(CCOPTS_GCCALL) -g -Wall

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
CCLAGS= $(GPPFLAGS) $(CFLAGS)

#LD= $(CC)
#LD= cc
LD= ld


LDRPATH=

LDFLAGS= $(LF_LDFLAGS) -R$(LIBDIR):$(CGS_RPATH)


LIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

LIB0=
LIB1= -lpcs -ldam -lb -lut
LIB2=
LIB3= -luc -lu
LIB4= $(CGS_LIBS)
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)

LIBINFO= $(LIBDIRS) $(LIBS)


VALUES= $(VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
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
OBJ01= proguserlist.o proglog.o
OBJ02= b_$(T).o
OBJ03= systems.o sysdialer.o cm.o cmbuf.o
OBJ04= 
OBJ05= 
OBJ06=
OBJ07= printhelp.o shio.o kshlib.o maininfo.o sesmsg.o msgdata.o
OBJ08=
OBJ09= vecstr_avmkstr.o
OBJ10= termout.o
OBJ11=
OBJ12= opendial.o getopendial.o
OBJ13= 
OBJ14=
OBJ15= field_srvarg.o

OBJ16= dialuss.o dialussmux.o dialussnls.o
OBJ17= dialtcp.o dialtcpmux.o dialtcpnls.o
OBJ18= dialticotsord.o dialticotsordnls.o 
OBJ19= dialuux.o
OBJ20= dialusd.o
OBJ21= dialudp.o
OBJ22= mkcleanline.o mkquoted.o mkfingerquery.o
OBJ23= inetaddrparse.o hasleadcolon.o getaf.o 

#OBJ24= uss.o ussmux.o ussinfo.o
#OBJ25= tcp.o tcpmux.o tcpnls.o 
#OBJ26= uux.o udp.o prog.o
#OBJ27= defproc.o 
#OBJ28= sysvar.o
OBJ29= uuname.o 
#OBJ30= envs.o envs_procxe.o envs_subs.o
OBJ31= getprogroot.o

OBJ32= snwcpyclean.o
OBJ33=
OBJ34=
OBJ35=
OBJ36=
OBJ37=
OBJ38=
#OBJ39= debug.o debugprintf.o nprintf.o

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE)

OBJS= $(CRTFRONT) $(OBJ) $(CRTC) $(CRTBACK)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		$(T).x

all:			$(ALL)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

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
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBINFO) > $(T).lm

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


main.o:			main.c $(INCS) sysdialer.h systems.h cm.h cmbuf.h \
				maininfo.h

whatinfo.o:		whatinfo.c config.h

proginfo.o:		proginfo.c defs.h

proginfo_setpiv.o:	proginfo_setpiv.c defs.h

proginfo_rootname.o:	proginfo_rootname.c defs.h

b_$(T).o:		b_$(T).c $(INCS)

maininfo.o:		maininfo.c maininfo.h kshlib.h

kshlib.o:		kshlib.c kshlib.h sesmsg.h msgdata.h

sesmsg.o:		sesmsg.c sesmsg.h

msgdata.o:		msgdata.c msgdata.h


systems.o:		systems.c systems.h

sysdialer.o:		sysdialer.c sysdialer.h

cm.o:			cm.c cm.h sysdialer.h systems.h

cmbuf.o:		cmbuf.c cmbuf.h

opendial.o:		opendial.c opendial.h


shio.o:			shio.c shio.h

userinfo.o:		userinfo.c userinfo.h

vecstr_loadfile.o:	vecstr_loadfile.c

vecstr_env.o:		vecstr_env.c

vecstr_avmkstr.o:	vecstr_avmkstr.c

filebuf.o:		filebuf.c filebuf.h

streamsync.o:		streamsync.c streamsync.h

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

hostinfo.o:		hostinfo.c hostinfo.h

termout.o:		termout.cc termout.h ansigr.h
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) termout.cc

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

uux.o:			uux.c uux.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcp.o:			tcp.c tcp.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpmux.o:		tcpmux.c tcpmux.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

tcpnls.o:		tcpnls.c tcpnls.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

udp.o:			udp.c udp.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

inetaddrparse.o:	inetaddrparse.c inetaddrparse.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envs.o:			envs.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envs_procxe.o:		envs_procxe.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

envs_subs.o:		envs_subs.c envs.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

uuname.o:		uuname.c uuname.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

useraccdb.o:		useraccdb.c useraccdb.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


getopendial.o:		getopendial.c opendial.h
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialuss.o:		dialuss.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialussmux.o:		dialussmux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialussnls.o:		dialussnls.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcp.o:		dialtcp.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcpmux.o:		dialtcpmux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialtcpnls.o:		dialtcpnls.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialticotsord.o:	dialticotsord.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialticotsordnls.o:	dialticotsordnls.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

dialuux.o:		dialuux.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

getaf.o:		getaf.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

defproc.o:		defproc.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

hasleadcolon.o:		hasleadcolon.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<

getprogroot.o:		getprogroot.c
	$(CC) -c $(CFLAGS) $(CCOPTS_GCCLIB) $(CPPFLAGS) $<


