# MAKEFILE (LIBCX)

T= libcx

ALL= $(T).a

SRCROOT= $(EXTRA)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

LDCRTDIR= $(CGS_CRTDIR)
VALDIR= $(CGS_VALDIR)


CC= $(CGS_CC)
GPP= $(CGS_GPP)

GPPFLAGS= $(CGS_CCFLAGS)
CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


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


LDRPATH= /usr/lib/lwp

LDFLAGS= $(LF_LDFLAGS) -R$(CGS_RPATH):$(LDRPATH)


SLIBDIRS= -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -ldam
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= $(LF_LIBS) 
SLIB6=
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5= 
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)


VALUES= $(VALDIR)/$(CGS_VALUES)

CRT1= $(LDCRTDIR)/crt1.o
CRTI= $(LDCRTDIR)/crti.o
CRTBEGIN= $(LDCRTDIR)/crtbegin.o
MCRT1= $(LDCRTDIR)/mcrt1.o
GCRT1= $(LDCRTDIR)/gcrt1.o
CRTEND= $(LDCRTDIR)/crtend.o
CRTN= $(LDCRTDIR)/crtn.o

CRTFRONT= $(CRT1) $(CRTI) $(VALUES) $(CRTBEGIN)
CRTBACK= $(CRTEND) $(CRTN)

# for regular (no profiling)
CRT0= $(CRT1) $(CRTI) $(VALUES)
# for 'prof(1)'
MCRT0= $(CRTI) $(MCRT1) $(VALUES)
# for 'gprof(1)'
GCRT0= $(CRTI) $(GCRT1) $(VALUES)

LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /usr/bin/touch


INSTALLINCS= vsystem.h localmisc.h syshas.h

INCS= localmisc.h syshas.h vsystem.h


OBJ00= strtoimax.o strtoumax.o
OBJ01= eaccess.o
OBJ02= pipe2.o
OBJ03=
OBJ04=
OBJ05=
OBJ06=
OBJ07=
OBJ08=
OBJ09= 
OBJ10=
OBJ11=
OBJ12=
OBJ13=
OBJ14=
OBJ15=
OBJ16=
OBJ17=
OBJ18=
OBJ19=
OBJ20=
OBJ21=
OBJ22=
OBJ23=
OBJ24=
OBJ25=
OBJ26=
OBJ27=
OBJ28=
OBJ29=
OBJ30=
OBJ31=
OBJ32=
OBJ33=
OBJ34=
OBJ35=
OBJ36=
OBJ37=
OBJ38=
OBJ39= 
OBJ40=
OBJ41=
OBJ42=
OBJ43= 
OBJ44=
OBJ45=
OBJ46=
OBJ47=

OBJ48=
OBJ49=
OBJ50=
OBJ51=
OBJ52=
OBJ53=
OBJ54=
OBJ55=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)

#OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG)
OBJ= $(OBJA)

#OBJS= obja.o objb.o objc.o objd.o obje.o objf.o objg.o
OBJS= obja.o


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


obja.o:			$(OBJA)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJC)

objd.o:			$(OBJD)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJD)

obje.o:			$(OBJE)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJE)

objf.o:			$(OBJF)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJF)

objg.o:			$(OBJG)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJG)


$(T).a:			$(OBJ)
	$(AR) -rc $(T).a $?

$(T).so:		$(OBJS) Makefile localmisc.h
	$(LD) -G -m -o $@ $(LDFLAGS) $(OBJS) $(SLIBINFO) > $(T).lm

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

install-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(ALL) Makefile install-incs
	ranlib $(T).a
	makenewer -r $(ALL) $(LIBDIR)

install-incs:		$(INSTALLINCS)
	makenewer $(INSTALLINCS) $(INCDIR)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJ)

again:
	rm -f $(ALL)

clean:
	rm -f *.x *.o $(T).a

control:
	(uname -n ; date) > Control


mtime.o:		mtime.c mtime.h

strtoimax.o:		strtoimax.c
strtoumax.o:		strtoumax.c
strtointmax.o:		strtointmax.c
strtouintmax.o:		strtouintmax.c



# testing

X01= testsigpending

X01OBJ= $(X01).o u_sigpending.o

X01OBJS= $(CRTFRONT) $(X01OBJ) $(CRTC) $(CRTBACK)

$(X01).o:		$(X01).c

$(X01).x:		$(X01OBJ) Makefile
	$(LD) -o $@ $(X01OBJS) $(XLIBDIRS) $(XLIBS) -lc


