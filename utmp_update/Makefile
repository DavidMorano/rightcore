#
#ident  "@(#)Makefile 1.2     93/02/03 SMI"
#
# Copyright (c) 1989 by Sun Microsystems, Inc.
#

PROG= utmp_update


FILEMODE = 4555
OWNER = root
GROUP = bin


INCDIR= $(EXTRA)/include
LIBDIR= $(EXTRA)/lib



CCOPTS= -O -mcpu=ultrasparc
#CCOPTS= -g


INCDIRS= -I$(INCDIR)


CFLAGS= $(CCOPTS) $(INCDIRS)


OBJ= main.o


LIBDIRS= -L$(LIBDIR)

LIBS= -ldam -lb -luc -lu -lrt -lsocket -lnsl


LDFLAGS= -R$(LIBDIR)


.KEEP_STATE:


all:		$(PROG) 

$(PROG):	$(OBJ)
	rm -f $@
	$(CC) -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

strip:		$(PROG)
	strip $(PROG)

install:	$(PROG)
	cp -p $(PROG) prog
	-chown root prog
	-chgrp sys prog
	-chmod o-w,u+xs prog
	mv prog /usr/lib/utmp_update


