# MAKEFILE (telnetd)

FBSD= /usr/add-on/fbsd

SRCROOT= $(LOCAL)

BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


PROG=	telnetd
T= telnetd

ALL= $(T) $(T).$(OFF)


CC= gcc
CCOPTS= -g

DEFINES1= -D$(OSTYPE)=1 -DSOLARIS=1 -DUTMPX=1
D1= -DLINEMODE -DUSE_TERMIO=1
#D2= -DKLUDGELINEMODE
D3= -DOLD_ENVIRON -DENV_HACK
#D4= -DAUTHENTICATION -DENCRYPTION
D5= -DSTREAMSPTY

CFLAGS+= -I${.CURDIR}/../../lib
DEFINES= $(DEFINES1) $(D1) $(D2) $(D3) $(D4) $(D5)
#INCDIRS= -I$(FBSD)/src/include -I$(FBSD)/src/lib
INCDIRS= -I$(NCMP)/include
CFLAGS= $(CCOPTS) $(DEFINES) $(INCDIRS)


#LD= $(CC)
LD= ld
#LDFLAGS= -m -R$(LIBDIR)/$(OFD):$(LIBDIR)
LDFLAGS=
LIBDIRS= -L$(LIBDIR)/$(OFD) -L$(LIBDIR)

LIBS0= -Bstatic -ldam -Bdynamic
LIBS1= -Bstatic -lb -luc -Bdynamic
LIBS2= -lu
LIBS3= -L$(GNU)/lib -lgcc
LIBS4= -lgen -lrt -lsocket -lnsl -lc

LIBS= $(LIBS0) $(LIBS1) $(LIBS2) $(LIBS3) $(LIBS4)

CRTI= $(LDCRTDIR)/crti.o
CRT1= $(LDCRTDIR)/crt1.o
CRTN= $(LDCRTDIR)/crtn.o
CRT0= $(CRTI) $(CRT1)

CRTC= makedate.o
CRT= $(CRTI) $(CRT1) $(CRTN)

LINT= lint
LINTFLAGS= -uxn -Dlint $(DEFINES) $(INCDIRS)



SRC=	global.c slc.c state.c sys_term.c telnetd.c \
	termstat.c utility.c 

#authenc.c


DPADD=	$(LIBUTIL) $(LIBTERMCAP) $(LIBTELNET)

LDADD=	-lutil -ltermcap -ltelnet
#LDADD+=	-lkrb -ldes

#DPADD+=	$(LIBKRB) $(LIBDES)

MAN1M=	telnetd.1m



OBJ= $(SRC:.c=.o)

OBJS= $(CRT0) $(OBJ) $(CRTC) 




default:		$(T).x

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<


$(T):			$(T).sh
	cp -p $(T).sh $(T)

$(T).x:			$(OBJ) Makefile
	makedate -m $(T) > makedate.c
	$(CC) -c makedate.c
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS) $(CRTN) > $(T).lm

$(T).$(OFF) $(OFF):	$(T).x
	cp -p $(T).x $(T).$(OFF)

strip:			$(T).x
	strip $(T).x
	rm -f $(T) $(T).$(OFF)

install:		$(T) install.x
	bsdinstall $(T) $(BINDIR)

install.$(OFF):		$(T).$(OFF)
	bsdinstall $(T).$(OFF) $(BINDIR)
	chmod u+s $(BINDIR)/$(T).$(OFF)

install.x:		$(T).x
	bsdinstall $(T).x $(BINDIR)
	chmod u+s $(BINDIR)/$(T).x

again:
	rm -f $(ALL) $(T).x

clean:			again
	rm -f *.o

control:
	(uname -n ; date) > Control




