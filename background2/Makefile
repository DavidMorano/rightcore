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
MANDIR= $(PROGROOT)/man/man1
RUNDIR= $(PROGROOT)/var/run

MAN1= 1
MAN8= 1m
SPOOLDIR= $(PROGROOT)/var/spool/jobsub

# Some may prefer:
# PIDFILE = $(RUNDIR)/batchd.pid
# PIDFILE = $(SPOOLDIR)/.batchd.pid
PIDFILE = $(RUNDIR)/batchd


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


# Compile-time configuration.
# The defaults are suitable for a BSD-based system, like SunOS 4.x.
#
# If you're using the old-style directory scheme
# i.e. "struct direct" and <sys/dir.h>, use -DOLD_DIR.
# If you don't have initgroups(), use -DNO_INITGROUPS.
# Define LSGOPT to be "g" if 'ls -l' displays the group.

# OSF/1 v3.0
# CONFIG=-DBSD4_2 -D_BSD
# LDFLAGS=

# Solaris 2.x, under gcc 2.3.3
CC= gcc

CONFIG= -DSYSVR4=1 -Dindex=strchr -DSUNOS5=1 -DSOLARIS=8


LIBDIRS= -L$(PROGROOT)/lib

LIB0=
LIB1= -Bstatic -ldam -Bdynamic
LIB2=
LIB3= -Bstatic -lb -luc -Bdynamic
LIB4= -Bstatic -lu -Bdynamic
LIB5= -L$(GNU)/lib -lgcc
LIB6= -lsecdb -lproject -lpthread -lrt -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


#LDFLAGS= -lgen -lkvm -lelf
#LDFLAGS= -lgen -lelf
LDFLAGS= 

# For IRIX 3.x, 4.x
# For IRIX 5.0, omit the -DSIGRET=int
#CONFIG=-D_BSD_COMPAT -DSIGRET=int
#LDFLAGS=-lsun

# Apollo SR10.x.  We must use the generic (but slow) load-average code.
#CONFIG=-DNOKMEM -DOLD_DIR
#EXTRAOBJ=ugetloads.o

# MIPS RISCos 4.5X
#  Be sure that /bsd43/bin is before /bin in your path.
#  You must use the bsd43 compiler and install.
#CONFIG=-DRISCos -DOLD_DIR
#CC=/bsd43/bin/cc
#LDFLAGS=-lmld

# Stardent 3030.
#CONFIG=-43 -DOLD_DIR -DNO_INITGROUPS

# HP HP-UX 8.07.
#  Use BSD signal and process group system calls, not SYSV.
#  Remove the group from the 'ls -l' display.
#CONFIG=-DSYSVR4
#EXTRAOBJ=hp.o
#LDFLAGS=-lBSD
#LSGOPT=o

# You might want to set this to -g.
# Add -DMODIFYPRIORITY to have batch lower the priority automatically
# if the user already has job(s) submitted.
#
#COPT= -xO5 -xtarget=ss20/hs22 -dalign -xdepend
COPT= -g

CFLAGS0= $(INCDIRS)
CFLAGS1= -DSPOOLDIR=\"$(SPOOLDIR)\" -DPIDFILE=\"$(PIDFILE)\" $(CONFIG) $(COPT)
CFLAGS= $(CFLAGS0) $(CFLAGS1)


O1= main.o proginfo.o whatinfo.o printhelp.o progconfig.o batch.o
O2= shio.o
O3=
O4=
O5=
O6=
O7=

O_JOBSUB= $(O1) $(O2) $(O3) $(O4) $(O5) $(O6) $(O7)

O_JOBSUBS= batchd.o lex.o env.o $(EXTRAOBJ)



INCS= config.h defs.h



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



all:		$(ALL)

jobsub:		$(O_JOBSUB)
	$(CC) -o $@ $(LDFLAGS) $(O_JOBSUB) $(LIBDIRS) $(LIBS)

jobsubs:	$(O_JOBSUBS)
	$(CC) -o $@ $(LDFLAGS) $(O_JOBSUBS) $(LIBDIRS) $(LIBS)

jobsub-cancel:	batchcancel.o
	$(CC) -o $@ $(LDFLAGS) batchcancel.o $(LIBDIRS) $(LIBS)

jobsub-queue:	batchqueue.sh
	rm -f $@
	sed -e "s:SPOOLDIR:$(SPOOLDIR):" \
	    -e "s:^LSOPT=-l:&$(LSGOPT):" batchqueue.sh > $@
	chmod +x $@

jobsub-signal:	batchsignal.o
	$(CC) -o $@ $(LDFLAGS) batchsignal.o $(LIBDIRS) $(LIBS)

jobsub-examine:	batchexamine.sh
	rm -f $@
	sed "s:SPOOLDIR:$(SPOOLDIR):" batchexamine.sh > $@
	chmod +x $@


# batch must be setuid root so it can signal batchd.
# Stardent note: use -u instead of -o
install:	all
	if [ -f /bin/hp-pa ]; then \
	   make install-hp ; \
	else \
	   install -m 755 batchd ${LIBDIR}/ ; \
	   install -o root -m 4711 batch ${BINDIR}/ ; \
	   install -m 755 batchcancel ${BINDIR}/ ; \
	   install -m 555 batchqueue ${BINDIR}/batchqueue ; \
	   install -m 755 batchsignal ${BINDIR}/ ; \
	   install -m 555 batchexamine ${BINDIR}/batchexamine ; \
	fi

# batch must be setuid root so it can signal batchd.
# HP-UX note: get rid of the previous batchd (rm or mv it to /tmp).
install-hp:	all
	chmod 755 batchd
	-/bin/rm -f ${LIBDIR}/batchd
	if [ -f ${LIBDIR}/batchd ]; then \
	   mv ${LIBDIR}/batchd /tmp; \
	fi
	cp -p batchd ${LIBDIR}/
	chown root batch
	chmod 4711 batch
	cp -p batch ${BINDIR}/
	chmod 755 batchcancel
	cp -p batchcancel ${BINDIR}/
	chmod 555 batchqueue
	cp -p batchqueue ${BINDIR}/batchqueue
	chmod 755 batchsignal
	cp -p batchsignal ${BINDIR}/
	chmod 555 batchexamine
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


msfile.o:	msfile.c msfile.h msfilee.h

msfilee.o:	msfilee.c msfilee.h




