#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved. The Berkeley Software License Agreement
# specifies the terms and conditions for redistribution.
#
# $Header:Makefile 12.1$
# $ACIS:Makefile 12.1$
# $Source: /ibm/acis/usr/src/ucb/msgs/RCS/Makefile,v $
#
#
#	@(#)Makefile	4.1 5/19/82
#
CFLAGS=-O
IFLAGS=-c -s
SRCS = msgs.c msgs.h Makefile
msgs: msgs.c msgs.h
	${CC} $(CFLAGS) -o msgs msgs.c -ltermlib

install: msgs ${DESTDIR}/usr/msgs
	install ${IFLAGS} msgs ${DESTDIR}/usr/ucb

${DESTDIR}/usr/msgs:
	mkdir $@
	chmod 777 $@

clean:
	-rm -f msgs
sources: $(SRCS)
$(SRCS):
	sccs get $@
