# MAKEFILE
#ident	"@(#)Makefile	1.7	92/12/15 SMI"
#
# Copyright (c) 1990 by Sun Microsystems, Inc.
#

ROOT= /
ROOTLIB= /usr/lib

PROG= listen nlps_server

LDCRTDIR= $(EXTRA)/lib

DBGOBJ	= doprnt.o
DBGSRC	= doprnt.c
 
LSOBJ  =	listen.o lslog.o lsdbf.o lsdata.o nstoa.o nlsaddr.o
 
NLPSOBJ =	nlps_serv.o lsdbf.o lssmb.o nstoa.o lslog.o lsdata.o

OBJS= $(LSOBJS) $(NLPSOBJS)
LSSRC= $(LSOBJS:%.o=%.c)
NLPSSRC= $(NLPSOBJS:%.o=%.c)
SRCS= $(LSSRC) $(NLPSSRC)


CC= gcc

LINK.c= ld

CCOPTS= -O -pthreads -mcpu=v8 -fno-builtin
#CCOPTS= -g -pthreads -mcpu=v8 -fno-builtin

CFLAGS= $(CCOPTS)


# include Makefile.cmd

LIBSAFD = $(ROOTLIB)/saf
NETD = $(ROOT)/usr/net
NETNLSD = $(NETD)/nls

# Don"t re-install /usr/lib/saf/ and /usr/net, Targetdirs installs them
#DIRS= $(LIBSAFD) $(NETD) $(NETNLSD)
DIRS=  $(NETNLSD)
ROOTSAFPROG= $(PROG:%=$(LIBSAFD)/%) 

# If machine name and /etc/issue file need to be printed 
# before the service prompt is printed, then:  CPPFLAGS += -DSYS_NAME
# If debug is needed, then:  CPPFLAGS += -DDEBUG

CPPFLAGS= -I$(EXTRA)/include


LIBDIRS= -L$(EXTRA)/lib

LIB0=
LIB1= -ldam -lb
LIB2=
LIB3= -luc -lu
LIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
LIB5= 
LIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
LIB7= -ldl -lc

LIBS= $(LIB0) $(LIB1) $(LIB2) $(LIB3) $(LIB4) $(LIB5) $(LIB6) $(LIB7)


LDLIBS= $(LIBDIRS) $(LIBS)

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

LDFLAGS=

LINTFLAGS = -b -x


LSOBJS= $(CRTFRONT) $(LSOBJ) $(CRTC) $(CRTBACK)

NLPSOBJS= $(CRTFRONT) $(NLPSOBJ) $(CRTC) $(CRTBACK)


OWNER= root
GROUP= sys
FILEMODE= 0755

$(LIBSAFD)/% : %
	$(INS.file)

.KEEP_STATE:

all:		$(PROG) 

listen:		$(LSOBJS)
	$(LD) -o $@ $(LDFLAGS) $(LSOBJS) $(LDLIBS)
	$(POST_PROCESS)

nlps_server:	$(NLPSOBJS)
	$(LD) -o $@ $(LDFLAGS) $(NLPSOBJS) $(LDLIBS)
	$(POST_PROCESS)

strip:		$(PROG)
	strip $(PROG)

install:	all $(DIRS) $(ROOTSAFPROG)
	$(RM) $(ROOT)/usr/net/nls/listen
	-$(SYMLINK) ../../lib/saf/listen $(ROOT)/usr/net/nls/listen
	$(RM) $(ROOT)/usr/net/nls/nlps_server
	-$(SYMLINK) ../../lib/saf/nlps_server $(ROOT)/usr/net/nls/nlps_server

$(DIRS):
	$(INS.dir)

clean:
	makeclean

lint:
	$(LINT.c) $(LSSRC)
	$(LINT.c) $(NLPSSRC)

# include Makefile.targ


