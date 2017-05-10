# MAKEFILE


ALL= jobsub jobsubs jobsub-cancel jobsub-queue jobsub-signal jobsub-examine


# User commands go in BINDIR
# batchd goes in LIBDIR
# Manual pages go in MANDIR/man$(MAN1) and MANDIR/man$(MAN8)
# Batch queues are under SPOOLDIR
# The batch daemon writes its process id to PIDFILE

#PROGROOT= $(LOCAL)
PROGROOT= $(HOME)

BINDIR= $(PROGROOT)/bin
LIBDIR= $(PROGROOT)/lib
INCDIR= $(PROGROOT)/include
MANDIR= $(PROGROOT)/man
RUNDIR= $(PROGROOT)/var/run
LDCRTDIR= $(PROGROOT)/lib


MAN1= 1
MAN8= 1m
SPOOLDIR= $(PROGROOT)/var/spool/jobsub

PIDFILE = $(RUNDIR)/jobsub


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

LD= ld

NM= nm
NMFLAGS= -xs -v

CXREF= cxref
CXREFFLAGS= -R -s

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f


# Compile-time configuration.
# The defaults are suitable for a BSD-based system, like SunOS 4.x.
#
# If you're using the old-style directory scheme
# i.e. "struct direct" and <sys/dir.h>, use -DOLD_DIR.
# If you don't have initgroups(), use -DNO_INITGROUPS.
# Define LSGOPT to be "g" if 'ls -l' displays the group.

CC= gcc

#CONFIG= -DSYSVR4=1 -DSUNOS5=1 -DSOLARIS=8
CONFIG= -DSYSVR4=1 -DSUNOS5=1 # -DSOLARIS=8

CCOPTS= -O -mcpu=ultrasparc
#CCOPTS= -g 

CFLAGS0= $(INCDIRS)
CFLAGS1= -DSPOOLDIR=\"$(SPOOLDIR)\" -DPIDFILE=\"$(PIDFILE)\" $(CONFIG)
CFLAGS2= $(CCOPTS)

CFLAGS= $(CFLAGS0) $(CFLAGS1) $(CFLAGS2)


LIBDIRS= -L$(PROGROOT)/lib

LIB0=
LIB1= -ldam -lb -luc
LIB2=
LIB3= -Bstatic -lu -Bdynamic
LIB4= -L$(GNU)/lib -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
VALUES= $(LDCRTDIR)/values-xa.o
CRTN= $(LDCRTDIR)/crtn.o

# for regular (no profiling)
CRT0= $(CRTI) $(CRT1) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

CRTC=

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


INCS= config.h defs.h


O1= main.o proginfo.o whatinfo.o printhelp.o progconfig.o batch.o
O2= shio.o
O3=
O4= 
O5= paramfile.o spawnproc.o
O6=
O7= printhelp.o

O_JOBSUB= $(CRT0) $(O1) $(O2) $(O3) $(O4) $(O5) $(O6) $(O7) $(CRTC)

O_JOBSUBS= $(CRT0) batchd.o lex.o env.o $(EXTRAOBJ) $(CRTC)


SRC= $(OBJ:.c=.o)


.SUFFIXES:		.ls .i .cx .cs


default:		all

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


all:			$(ALL)

jobsub:			Makefile $(O_JOBSUB)
	$(LD) -o $@ $(LDFLAGS) $(O_JOBSUB) $(LIBDIRS) $(LIBS) $(CRTN)

jobsubs:		Makefile $(O_JOBSUBS)
	$(LD) -o $@ $(LDFLAGS) $(O_JOBSUBS) $(LIBDIRS) $(LIBS) $(CRTN)

jobsub-cancel:		Makefile batchcancel.o
	$(LD) -o $@ $(LDFLAGS) batchcancel.o $(LIBDIRS) $(LIBS) $(CRTN)

jobsub-queue:		Makefile batchqueue.sh
	rm -f $@
	sed -e "s:SPOOLDIR:$(SPOOLDIR):" \
	    -e "s:^LSOPT=-l:&$(LSGOPT):" batchqueue.sh > $@
	chmod +x $@

jobsub-signal:		Makefile batchsignal.o
	$(LD) -o $@ $(LDFLAGS) batchsignal.o $(LIBDIRS) $(LIBS) $(CRTN)

jobsub-examine:		Makefile batchexamine.sh
	rm -f $@
	sed "s:SPOOLDIR:$(SPOOLDIR):" batchexamine.sh > $@
	chmod +x $@

strip:
	strip jobsub
	strip jobsubs

# batch must be setuid root so it can signal batchd.
# Stardent note: use -u instead of -o
install:	$(ALL)
	makenewer -r $(ALL) $(BINDIR)

install.jobsubs:	jobsub jobsubs
	strip jobsub jobsubs
	makenewer -r jobsub jobsubs $(BINDIR)

# batch must be setuid root so it can signal batchd.
# HP-UX note: get rid of the previous batchd (rm or mv it to /tmp).
install-hp:	all
	chmod 775 batchd
	-/bin/rm -f ${LIBDIR}/batchd
	if [ -f ${LIBDIR}/batchd ]; then \
	   mv ${LIBDIR}/batchd /tmp; \
	fi
	cp -p batchd ${LIBDIR}/
	chown root batch
	chmod 4711 batch
	cp -p batch ${BINDIR}/
	chmod 775 batchcancel
	cp -p batchcancel ${BINDIR}/
	chmod 777 batchqueue
	cp -p batchqueue ${BINDIR}/batchqueue
	chmod 775 batchsignal
	cp -p batchsignal ${BINDIR}/
	chmod 777 batchexamine
	cp -p batchexamine ${BINDIR}/batchexamine

install-man:
	for i in *.1; do \
	  sed "s:SPOOLDIR:$(SPOOLDIR):" $$i > $(MANDIR)/man$(MAN1)/$$i; done
	for i in *.8; do \
	  sed -e "s:SPOOLDIR:$(SPOOLDIR):" \
	      -e "s:BINDIR:$(BINDIR):" \
	      -e "s:LIBDIR:$(LIBDIR):" \
	      -e "s:PIDFILE:$(PIDFILE):" $$i > $(MANDIR)/man$(MAN8)/$$i; done

clean:
	rm -f *.o batch batchcancel batchd lex.c batchqueue batchsignal \
	  batchexamine

lint:
	lint -DSPOOLDIR=\"x\" -DPIDFILE=\"y\" batchd.c


main.o:		main.c $(INCS)

proginfo.o:	proginfo.c $(INCS)

progconfig.o:	progconfig.c $(INCS)

batch.o:	batch.c $(INCS)

batchd.o:	batchd.c $(INCS)


msfile.o:	msfile.c msfile.h msfilee.h

msfilee.o:	msfilee.c msfilee.h

paramfile.o:	paramfile.c paramfile.h

spawnproc.o:	spawnproc.c spawnproc.h

varsub.o:	varsub.c varsub.h



