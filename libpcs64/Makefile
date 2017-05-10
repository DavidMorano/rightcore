# MAKEFILE

T= libpcs

ALL= $(T).a

SRCROOT= $(PCS)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib


# stuff

#CC= gcc

#CCOPTS= -O3 -fpic -mcpu=ultrasparc -mno-app-regs

# HyperSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -Kpic -xtarget=ultra -xsafe=mem -xdepend

# UltraSPARC-v9 (64 bits)
CCOPTS= -xO5 -xtarget=ultra -xarch=v9 -xsafe=mem -xdepend


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
CFLAGS= $(CCOPTS) $(LF_CFLAGS)

#LD= $(CC)

LDFLAGS= $(LF_LDFLAGS)

LIBDIRS=

LIBS= $(LF_LIBS) -L$(GNU)/lib -lgcc

LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /bin/touch


INCS= pcsconf.h mimetypes.h


OBJ00=
OBJ01= pcsconf.o 
OBJ02= pcsmsgid.o pcsuserfile.o pcspoll.o
OBJ03= pcsngdir.o pcsgetprog.o pcsgetprogpath.o pcsdialuucp.o
OBJ04= pcstrustuser.o pcsmailhost.o pcsfullname.o
OBJ05= 
OBJ06= 
OBJ07= 
OBJ08= attach.o mimetypes.o
OBJ09= ns_mailname.o ns_fixgecos.o getgecos.o
OBJ10= mheader.o errfile.o
OBJ11= datestr_envelope.o datestr_header.o
OBJ12= securefile.o
OBJ13= bopenroot.o 
OBJ14= ids.o
OBJ15=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07)
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)

OBJ= $(OBJA) $(OBJB)


.SUFFIXES:		.ls .i .cx .cs


all:			$(ALL)

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


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJ) Makefile $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

install:		$(ALL)
	makenewer $(ALL) $(LIBDIR)/$(OFD)
	makenewer $(ALL) $(LIBDIR)
	makenewer -z $(INCS) $(INCDIR)

install.$(OFF):		$(ALL)
	makenewer $(ALL) $(LIBDIR)/$(OFD)


again:
	rm -f $(ALL) $(T).so $(T).a

clean:			again
	rm -f *.o $(ALL)

control:
	uname -n > Control
	date >> Control


pcsconf.o:		pcsconf.c pcsconf.h

pcsdialuucp.ln:		pcsdialuucp.c
	$(LINT) -c $(LINTFLAGS) $(CPPFLAGS) pcsdialuucp.c


pcsdialuucp.o:		pcsdialuucp.c pcsconf.h

pcsgetdate.o:		pcsgetdate.c pcsconf.h

pcsgetprog.o:		pcsgetprog.c pcsconf.h

pcsgetprogpath.o:	pcsgetprogpath.c pcsconf.h

pcsmailhost.o:		pcsmailhost.c pcsconf.h

pcsmsgid.o:		pcsmsgid.c pcsconf.h

pcsngdir.o:		pcsngdir.c pcsconf.h

pcspoll.o:		pcspoll.c pcsconf.h

pcsuserfile.o:		pcsuserfile.c pcsconf.h

pcstrustuser.o:		pcstrustuser.c pcsconf.h

pcsfullname.o:		pcsfullname.c pcsconf.h


securefile.o:		securefile.c


mimetypes.o:		mimetypes.c mimetypes.h

attach.o:		attach.c attach.h

ids.o:			ids.c ids.h



