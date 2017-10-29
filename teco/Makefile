# SCCSID:  @(#) makefile:  6.1 8/25/83
#
#	TECO MAKEFILE
#

#CC=		cc
CFLAGS=		-O -DSUN=1 -DSVR4=1

LD= $(CC)
LDFLAGS=


DEFLIST=

# termcap/curses library.  If none, leave blank and turn off
# CRTRUB in mdconfig.h .

CURSES=	-lcurses

AOUTNAME=	teco

OFILES=		cmarith.o cmcond.o cmdel.o cmdio.o cmecmd.o \
		cmenv.o cmfcmd.o cmfile.o cmflags.o cmgoto.o \
		cmins.o cmmacro.o cmmode.o cmpos.o cmqreg.o \
		cmsearch.o cmspec.o cmtecio.o cmtype.o cmutil.o \
		crt.o ctype.o errmsg.o errors.o fileio.o find.o \
		getcmd.o interr.o memory.o sm.o tb.o \
		tecexit.o tecinit.o tecmain.o teco.tec.o tty.o \
		values.o wild.o xec.o



#-----------------

all:	teco

.c.o:
	$(CC) -c $(CFLAGS) $(DEFLIST) $<


teco:	$(OFILES) Makefile
	$(LD) -o $@ $(LDFLAGS) $(OFILES) -i $(CURSES)

debug t:
	$(MAKE) -$(MAKEFLAGS) teco CFLAGS=-gp DEFLIST=-DDEBUG \
	CC="$(CC)" AOUTNAME=t


#-----------------------

cmarith.o:	bool.h errors.h errmsg.h exittype.h qname.h \
		skiptype.h tb.h values.h xec.h

cmcond.o:	bool.h ctype.h errors.h errmsg.h exittype.h \
		skiptype.h values.h xec.h

cmdel.o:	bool.h cmutil.h errors.h errmsg.h exittype.h \
		qname.h skiptype.h tb.h values.h xec.h

cmdio.o:	bool.h chars.h crt.h mdconfig.h qname.h \
		sources.h tb.h tflags.h tty.h values.h

cmecmd.o:	bool.h errors.h errmsg.h exittype.h mdconfig.h \
		skiptype.h tty.h values.h xec.h qname.h

cmenv.o:	bool.h chars.h cmdio.h cmutil.h ctype.h \
		errors.h errmsg.h exittype.h find.h mdconfig.h \
		memory.h qname.h skiptype.h tb.h tty.h values.h xec.h

cmfcmd.o:	bool.h errors.h errmsg.h xec.h

cmfile.o:	bool.h chars.h cmutil.h errors.h errmsg.h \
		exittype.h mdtypes.h fileio.h find.h mdconfig.h \
		memory.h qname.h skiptype.h tb.h tflags.h tty.h \
		values.h xec.h

cmflags.o:	bool.h exittype.h skiptype.h values.h xec.h

cmgoto.o:	bool.h chars.h cmutil.h errors.h errmsg.h \
		exittype.h find.h qname.h skiptype.h tb.h \
		values.h xec.h xectype.h

cmins.o:	bool.h chars.h errors.h errmsg.h exittype.h \
		find.h qname.h skiptype.h tb.h values.h xec.h

cmmacro.o:	bool.h chars.h errors.h errmsg.h exittype.h \
		qname.h skiptype.h tflags.h values.h xec.h xectype.h

cmmode.o:	bool.h crt.h exittype.h skiptype.h tflags.h tty.h values.h xec.h

cmpos.o:	bool.h errors.h errmsg.h exittype.h find.h \
		qname.h skiptype.h tb.h values.h xec.h

cmqreg.o:	bool.h chars.h ctype.h cmutil.h errors.h \
		errmsg.h exittype.h find.h qname.h skiptype.h \
		tb.h tty.h values.h xec.h

cmsearch.o:	bool.h chars.h ctype.h cmutil.h errors.h \
		errmsg.h exittype.h find.h mdconfig.h memory.h \
		qname.h skiptype.h tb.h tflags.h values.h xec.h xectype.h

cmspec.o:	bool.h errors.h errmsg.h exittype.h qname.h \
		skiptype.h tb.h values.h xec.h

cmtecio.o:	bool.h errors.h errmsg.h exittype.h qname.h \
		skiptype.h tb.h tflags.h tty.h values.h xec.h

cmtype.o:	cmtype.c \
		bool.h chars.h cmdio.h cmutil.h ctype.h \
		errors.h errmsg.h exittype.h find.h qname.h \
		skiptype.h tflags.h tty.h values.h xec.h

cmutil.o:	bool.h chars.h ctype.h errors.h errmsg.h \
		qname.h tb.h tflags.h values.h xec.h

crt.o:		bool.h chars.h mdconfig.h tflags.h tty.h values.h

ctype.o:	ctype.h mdconfig.h qname.h

errmsg.o:	errors.h

errors.o:	bool.h chars.h cmdio.h crt.h errors.h exittype.h \
		tb.h tflags.h tty.h xec.h

fileio.o:	bool.h chars.h mdconfig.h mdtypes.h memory.h

find.o:		bool.h chars.h ctype.h tb.h xec.h

getcmd.o:	bool.h chars.h cmdio.h cmutil.h errors.h \
		errmsg.h find.h qname.h sources.h tb.h \
		tflags.h tty.h values.h xec.h

memory.o:	mdconfig.h

sm.o:		bool.h memory.h

smtest.o:	bool.h

tb.o:		qname.h bool.h memory.h tb.h

tbtest.o:	bool.h qname.h tb.h

tecexit.o:	bool.h tty.h

tecinit.o:	bool.h chars.h cmutil.h mdconfig.h qname.h \
		tb.h tflags.h tty.h values.h xec.h

tecmain.o:	bool.h exittype.h values.h

tty.o:		bool.h chars.h ctype.h errors.h errmsg.h tflags.h values.h

values.o:	bool.h errors.h errmsg.h find.h mdconfig.h qname.h xec.h

wild.o:		bool.h chars.h cmutil.h errors.h errmsg.h fileio.h qname.h tb.h

xec.o:		bool.h ctype.h errors.h errmsg.h exittype.h \
		mdconfig.h qname.h skiptype.h tb.h tflags.h \
		tty.h values.h xectype.h

#----------------

clean:
		rm -f $(OFILES)

#----------------

clobber:	clean
		rm -f teco t

#----------------

lint:		$(OFILES:.o=.c)
		lint $(OFILES:.o=.c) >lint.out



