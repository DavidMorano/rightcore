# $Id: Makefile.SH,v 1.6 1992/08/03 04:51:45 sob Exp sob $
#
# This software is Copyright 1992 by Stan Barber. 
#
# Permission is hereby granted to copy, reproduce, redistribute or otherwise
# use this software as long as: there is no monetary profit gained
# specifically from the use or reproduction of this software, it is not
# sold, rented, traded or otherwise marketed, and this copyright notice is
# included prominently in any copy made. 
#
# The author make no claims as to the fitness or correctness of this software
# for any use whatsoever, and it is provided as is. Any use of this software
# is at the user's own risk. 
#
# $Log: Makefile.SH,v $
# Revision 1.6  1992/08/03  04:51:45  sob
# Release version 1.6
#
#
# 
CC	=	cc 
MANSRC	=	/usr/add-on/netnews/man/man1
MANEXT	=	1
CFLAGS	=	  -DUSG -O  -DNNTPSRC -DSYSV=1
LDFLAGS	=	 
LIBS	=	    -lsocket -lnsl
NEWSUSR	=	news
DESTDIR	=	/usr/add-on/netnews/lib/news
SHELL	=	/bin/ksh
SRCS	=	inews.c uname.c postauth.c nntpclnt.c version.c
OBJS	=	inews.o uname.o postauth.o nntpclnt.o version.o 
HFILES	=	nntp.h nntpclnt.h
SHFILES = 	Makefile.SH config.h.SH
README	= 	README README.DECNET README.PCNFS README.PCTCP
MAN	=	nntpclnt.3

inews:  $(OBJS) nntp.h nntpclnt.h
	$(CC) $(CFLAGS) $(OBJS) -o inews $(LIBS)

uname.o: uname.c config.h 

inews.o: inews.c config.h nntpclnt.h

nntpclnt.o: nntpclnt.c config.h nntp.h
	$(CC) -o inews $(CFLAGS) -I/home/dam/include $(OBJS) $(LIBS)

postauth.o: postauth.c config.h nntp.h

Makefile: Makefile.SH
	$(SHELL) Makefile.SH

config.h: config.h.SH
	$(SHELL) config.h.SH

install: inews
	cp inews ${DESTDIR}/inews
	chown ${NEWSUSR} ${DESTDIR}/inews
	chmod 775 ${DESTDIR}/inews

clean:
	-rm -f ${OBJS} inews 

distrib: clean
	rm -f make.out
	rm -rf RCS

compat:
	ln nntpclnt.h clientlib.h
	ln nntpclnt.c clientlib.c
	ln nntpclnt.o clientlib.o

check:
	ci -l *.[ch] Makefile

lint:
	lint ${SRCS} ${HFILES}

tags:
	ctags $(SRCS) $(HFILES)

shar:
	shar MANIFEST $(SHFILES) $(SRCS) $(HFILES) $(README) $(MAN) Configure > nntpclnt.shar
