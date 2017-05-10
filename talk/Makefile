# TALK
#ident	"@(#)Makefile	1.12	97/06/06 SMI"
#
# Copyright (c) 1989 by Sun Microsystems, Inc.
#

#SRCROOT= $(HOME)
SRCROOT= $(LOCAL)

PROG=	talk


OBJS=	main.o get_names.o display.o io.o ctl.o init_disp.o \
	msgs.o get_addrs.o ctl_transact.o invite.o look_up.o

SRCS= $(OBJS:%.o=%.c)


#
# Message catalog
#
POFILE= talk.po
#

CC= gcc

CCOPTS= -O -mcpu=ultrasparc
#CCOPTS= -g

CFLAGS=		$(CCOPTS)

CPPFLAGS +=	-DSYSV -DSTRNET -DBSD_COMP -I$(SRCROOT)/include


LDLIBS=		-L$(HOME)/lib -ldam -luc libu.a
LDLIBS +=	-lcurses -lsocket -lnsl


LINK.c= $(CC)



.KEEP_STATE:

.PARALLEL:	$(OBJS)



all:		$(PROG)

$(PROG):	$(OBJS)
	$(LINK.c) -o $@ $(OBJS) $(LDLIBS)


#
# message catalog
#
_msg: $(POFILE)

$(POFILE):	$(SRCS)
	$(RM) talk.po
	$(COMPILE.cpp) 	$(SRCS)	> $(POFILE).i
	$(XGETTEXT) $(XGETFLAGS)	$(POFILE).i	
	sed "/^domain/d" messages.po > $@
	$(RM) messages.po $(POFILE).i


strip:		$(PROG)
	strip $(PROG)

install:	all
	makenewer -r $(PROG) $(SRCROOT)/bin

install-raw:	install

clean:
	$(RM) $(OBJS)

lint:	lint_SRCS

clobber: clean
	-$(RM) $(PROG) $(CLOBBERFILES)

lint_PROG:
	$(LINT.c) $(PROG).c $(LDLIBS)

lint_SRCS:
	$(LINT.c) $(SRCS) $(LDLIBS)

#
# For message catalogue files
#
_msg: $(MSGDOMAIN) $(POFILE)
	$(RM) $(MSGDOMAIN)/$(POFILE)
	$(CP) $(POFILE) $(MSGDOMAIN)

# the build of the $(DCFILE) should be defined locally
# its .dc extension gets renamed to .po upon installation
#
_dc: $(DCMSGDOMAIN) $(DCFILE)
	$(RM) $(DCMSGDOMAIN)/$(DCFILE)
	$(CP) $(DCFILE) $(DCMSGDOMAIN)/$(DCFILE:.dc=.po)

$(MSGDOMAIN) $(DCMSGDOMAIN):
	$(INS.dir)



