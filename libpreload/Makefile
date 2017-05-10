# MAKEFILE

T= libpreload

ALL= $(T).so

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


#
# Load-time initialization is needed for those few programs which change
# their own inherited environment (from the prior |exec(2)|) before
# they call on any subroutine which would have otherwise triggered
# initialization of this module.  The known program that does this
# (rather stupidly if one asks me) is the Korn Shell (KSH) program!
#

LDSOARRAY=
LDSOARRAY += -z initarray=preload_init -z finiarray=preload_fini


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

LDFLAGS= $(LF_LDFLAGS) $(LDSOARRAY)


LDRPATH= $(EXTRA}/lib


SLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

SLIB0=
SLIB1= -ldam
SLIB2=
SLIB3= -luc -lu
SLIB4= $(CGS_LIBS)
SLIB5= $(LF_LIBS) 
SLIB6= -ldl
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR) -L$(CGS_LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -lut -luc -lu
XLIB4= $(CGS_LIBS)
XLIB5=  $(LF_LIBS)
XLIB6= -lsecdb -lproject -lpthread -lrt -lxnet -lsocket -lnsl
XLIB7= -ldl

XLIBS= $(XLIB0) $(XLIB1) $(XLIB2) $(XLIB3) $(XLIB4) $(XLIB5) $(XLIB6) $(XLIB7)

XLIBINFO= $(XLIBDIRS) $(XLIBS)


LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn -Dlint

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= ${CCS}/bin/tsort

RM= rm -f
TOUCH= /bin/touch


INCS= vsystem.h localmisc.h


OBJ00= preload.o
OBJ01= confstr.o
OBJ02= sysinfo.o
OBJ03= gethostid.o
OBJ04= getpagesize.o
OBJ05=
OBJ06=
OBJ07=

OBJ08= 
OBJ09=
OBJ10=
OBJ11=
OBJ12= vecstr.o vecstr_envfile.o
OBJ13= buffer.o
OBJ14=
OBJ15=

# these are needed for their LIBMALLOC features
OBJ16= varray.o lookaside.o
OBJ17=
OBJ18= 
OBJ19=
OBJ20=
OBJ21=
OBJ22=
OBJ23=

OBJ24= strdcpy.o strdcpyw.o
OBJ25=
OBJ26= sfsub.o
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

OBJ56=
OBJ57= 
OBJ58=
OBJ59=
OBJ60=
OBJ61=
OBJ62=
OBJ63=

OBJ64=
OBJ65=
OBJ66=
OBJ67=
OBJ68=
OBJ69=
OBJ70=
OBJ71=

OBJ72=
OBJ73=
OBJ74=
OBJ75=
OBJ76=
OBJ77=
OBJ78=
OBJ79=

OBJ80=
OBJ81=
OBJ82=
OBJ83=
OBJ84=
OBJ85=
OBJ86=
OBJ87=

OBJ88=
OBJ89=
OBJ90=
OBJ91=
OBJ92=
OBJ93=
OBJ94=
OBJ95=

OBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
OBJB= $(OBJ08) $(OBJ09) $(OBJ10) $(OBJ11) $(OBJ12) $(OBJ13) $(OBJ14) $(OBJ15)
OBJC= $(OBJ16) $(OBJ17) $(OBJ18) $(OBJ19) $(OBJ20) $(OBJ21) $(OBJ22) $(OBJ23)
OBJD= $(OBJ24) $(OBJ25) $(OBJ26) $(OBJ27) $(OBJ28) $(OBJ29) $(OBJ30) $(OBJ31)
OBJE= $(OBJ32) $(OBJ33) $(OBJ34) $(OBJ35) $(OBJ36) $(OBJ37) $(OBJ38) $(OBJ39)
OBJF= $(OBJ40) $(OBJ41) $(OBJ42) $(OBJ43) $(OBJ44) $(OBJ45) $(OBJ46) $(OBJ47)
OBJG= $(OBJ48) $(OBJ49) $(OBJ50) $(OBJ51) $(OBJ52) $(OBJ53) $(OBJ54) $(OBJ55)
OBJH= $(OBJ56) $(OBJ57) $(OBJ58) $(OBJ59) $(OBJ60) $(OBJ61) $(OBJ62) $(OBJ63)
OBJI= $(OBJ64) $(OBJ65) $(OBJ66) $(OBJ67) $(OBJ68) $(OBJ69) $(OBJ70) $(OBJ71)
OBJJ= $(OBJ72) $(OBJ73) $(OBJ74) $(OBJ75) $(OBJ76) $(OBJ77) $(OBJ78) $(OBJ79)
OBJK= $(OBJ80) $(OBJ81) $(OBJ82) $(OBJ83) $(OBJ84) $(OBJ85) $(OBJ86) $(OBJ87)
OBJL= $(OBJ88) $(OBJ89) $(OBJ90) $(OBJ91) $(OBJ92) $(OBJ93) $(OBJ94) $(OBJ95)

OBJg0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
OBJg1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL)

OBJ= $(OBJg0) $(OBJg1)

#OBJS0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o objj.o
#OBJS1= objk.o objl.o
OBJS0= obja.o objb.o objc.o

OBJS= $(OBJS0) $(OBJS1)

TOBJ00=

#TOBJ01a=
TOBJ01b=

TOBJ01= $(TOBJ01a) $(TOBJ01b) $(TOBJ01c) $(TOBJ01d)

#TOBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) $(OBJ04) $(OBJ05) $(OBJ06) $(OBJ07) 
#TOBJA= $(OBJ00) $(OBJ01) $(OBJ02) $(OBJ03) 
# working
#TOBJA= $(OBJ02) $(OBJ03) 
# not-working
#TOBJA= $(OBJ00) $(OBJ01) 
# not-working
TOBJA= $(TOBJ01) 
# working
#TOBJA= $(TOBJ00) 

#TOBJG0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
#TOBJG0= $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
#TOBJG0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) 
#TOBJG0= $(OBJA) $(OBJB) 
#TOBJG0= $(OBJB) 
TOBJG0= $(TOBJA) 
#TOBJG0= $(OBJC) $(OBJD) 
#TOBJG1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) $(OBJM) $(OBJN) $(OBJO) $(OBJP)
#TOBJG1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) 
#TOBJG1= $(OBJM) $(OBJN) $(OBJO) $(OBJP)

TOBJ= $(TOBJG0) $(TOBJG1)


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

.cc.o:
	$(GPP) -c $(GPPFLAGS) $(CFLAGS) $(CPPFLAGS) $<

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c $(LINTFLAGS) $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(TT).a:		$(TOBJ)
	$(AR) -rc $@ $?

$(T).so:		$(OBJS) Makefile $(T).map
	$(LD) -G -o $@ $(LDFLAGS) -M $(T).map $(OBJS) $(SLIBDIRS) $(SLIBS)

$(T).a:			$(OBJ)
	$(AR) -rc $@ $?

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

intall-pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(ALL) Makefile install-incs
	ranlib $(T).a
	makenewer -r $(ALL) $(LIBDIR)
	makeinstall $(T)

install-incs:
	makenewer -im -z -m o=h $(OBJA) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJB) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJC) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJD) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJE) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJF) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJG) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJH) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJI) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJJ) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJK) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJL) -d $(INCDIR)

safe:
	makesafe -z -v=3 -I $(INCDIR) $(OBJA)
	makesafe -z -v=3 -I $(INCDIR) $(OBJB)
	makesafe -z -v=3 -I $(INCDIR) $(OBJC)
	makesafe -z -v=3 -I $(INCDIR) $(OBJD)
	makesafe -z -v=3 -I $(INCDIR) $(OBJE)
	makesafe -z -v=3 -I $(INCDIR) $(OBJF)
	makesafe -z -v=3 -I $(INCDIR) $(OBJG)
	makesafe -z -v=3 -I $(INCDIR) $(OBJH)
	makesafe -z -v=3 -I $(INCDIR) $(OBJI)
	makesafe -z -v=3 -I $(INCDIR) $(OBJJ)
	makesafe -z -v=3 -I $(INCDIR) $(OBJK)
	makesafe -z -v=3 -I $(INCDIR) $(OBJL)

again:
	rm -f $(ALL)

clean:
	rm -f $(ALL) *.o *.i *.ls *.ln *.lm

control:
	(uname -n ; date) > Control


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

objh.o:			$(OBJH)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJH)

obji.o:			$(OBJI)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJI)

objj.o:			$(OBJJ)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJJ)

objk.o:			$(OBJK)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJK)

objl.o:			$(OBJL)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJL)


# library private subroutine modules

preload.o:		preload.c preload.h

confstr.o:		confstr.c preload.h

sysinfo.o:		sysinfo.c preload.h

gethostid.o:		gethostid.c preload.h


# general subroutine modules

fsdir.o:		fsdir.c fsdir.h

# POSIX Thread Attribute
pta.o:			pta.c pta.h

# POSIX Thread Mutex Attribute
ptma.o:			ptma.c ptma.h

# POSIX Thread Condition Attribute
ptca.o:			ptca.c ptca.h

# POSIX Thread Read-Write Lock Attribute
ptrwa.o:		ptrwa.c ptrwa.h

# POSIX Thread Mutex
ptm.o:			ptm.c ptm.h

# POSIX Thread Condition Variable
ptc.o:			ptc.c ptc.h

# POSIX Semaphore
psem.o:			psem.c psem.h

# POSIX Read-Write Lock
ptrwlock.o:		ptrwlock.c ptrwlock.h

# POSIX Message Queue
pmq.o:			pmq.c pmq.h

sigman.o:		sigman.c sigman.h

sigblock.o:		sigblock.c sigblock.h

sigign.o:		sigign.c sigign.h

userattr.o:		userattr.c userattr.h

ucsem.o:		ucsem.c ucsem.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

pta.o:			pta.c pta.h

ucsem.o:		ucsem.c ucsem.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

storeitem.o:		storeitem.c storeitem.h

storebuf.o:		storebuf.c storebuf.h

sbuf.o:			sbuf.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

buffer.o:		buffer.c buffer.h

ptca.o:			ptca.c ptca.h

pta.o:			pta.c pta.h

ucsem.o:		ucsem.c ucsem.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

field.o:		field.c field.h

char.o:			char.c char.h

filebuf.o:		filebuf.c filebuf.h

filebuf_writeblanks.o:	filebuf_writeblanks.c filebuf.h

filebuf_writefill.o:	filebuf_writefill.c filebuf.h

recarr.o:		recarr.c recarr.h

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_envfile.o:	vecstr_envfile.c vecstr.h

vecpstr.o:		vecpstr.c vecpstr.h

vecpstr_envfile.o:	vecpstr_envfile.c vecpstr.h

vecobj.o:		vecobj.c vecobj.h

vecelem.o:		vecelem.c vecelem.h

hdb.o:			hdb.c hdb.h

envlist.o:		envlist.c envlist.h

lookaside.o:		lookaside.c lookaside.h

strpack.o:		strpack.c strpack.h

pq.o:			pq.c pq.h

ids.o:			ids.c ids.h

linefold.o:		linefold.c linefold.h

envhelp.o:		envhelp.c envhelp.h

mkprogenv.o:		mkprogenv.c mkprogenv.h

spawner.o:		spawner.c spawner.h

nulstr.o:		nulstr.c nulstr.h

lockrw.o:		lockrw.c lockrw.h

upwcache.o:		upwcache.c upwcache.h recarr.h

filemap.o:		filemap.c filemap.h

uname.o:		uname.c uname.h

dirlist.o:		dirlist.c dirlist.h

dirseen.o:		dirseen.c dirseen.h

dirseen_not.o:		dirseen_not.c dirseen.h

tmz.o:			tmz.c tmz.h

varray.o:		varray.c varray.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DVARRAY_LIBMEMALLOC=1 $<

lookaside.o:		lookaside.c lookaside.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DLOOKASIDE_LIBMEMALLOC=1 $<

randomvar.o:		randomvar.c randomvar.h


upt.o:			upt.c upt.h

base64.o:		base64.c base64.h

spawnproc.o:		spawnproc.c spawnproc.h

getxusername.o:		getxusername.c getxusername.h

getutmpent.o:		getutmpent.c getutmpent.h

getax.o:		getax.c getax.h

passwdent.o:		passwdent.c passwdent.h

shadowent.o:		shadowent.c shadowent.h

groupent.o:		groupent.c groupent.h

projectent.o:		projectent.c projectent.h

userattrent.o:		userattrent.c userattrent.h

utmpaccent.o:		utmpaccent.c utmpaccent.h

findbit.o:		findbit.c findbit.h

snflags.o:		snflags.c snflags.h


dialudp.o:		dialudp.c

dialtcp.o:		dialtcp.c

dialtcpmux.o:		dialtcpmux.c

dialtcpnls.o:		dialtcpnls.c nlsdialassist.h

nlsdialassist.o:	nlsdialassist.c nlsdialassist.h

opensysfs.o:		opensysfs.c opensysfs.h

quoteshellarg.o:	quoteshellarg.c

mkquoted.o:		mkquoted.c

snsd.o:			snsd.c

snsds.o:		snsds.c

sncpy.o:		sncpy.c

snfsflags.o:		snfsflags.c snflags.h

snopenflags.o:		snopenflags.c snflags.h

snpollflags.o:		snpollflags.c snflags.h

sntmtime.o:		sntmtime.c tmtime.h

mkpath.o:		mkpath.c

intsat.o:		intsat.c intsat.h

tmtime.o:		tmtime.c tmtime.h

ctdecp.o:		ctdecp.c ctdecp.h

gethe.o:		gethe.c

getnodedomain.o:	getnodedomain.c

inetpton.o:		inetpton.c

matstr.o:		matstr.c

strshrink.o:		strshrink.c

stwhitedot.o:		sfwhitedot.c

strnchr.o:		strnchr.c

mkvarpath.o:		mkvarpath.c

mallocstuff.o:		mallocstuff.c

ndigits.o:		ndigits.c


# special built-ins

uinfo.o:		uinfo.c uinfo.h

umask.o:		umask.c umask.h

ugetpw.o:		ugetpw.c ugetpw.h upwcache.h recarr.h

utmpacc.o:		utmpacc.c utmpacc.h recarr.h

uproguser.o:		uproguser.c uproguser.h

uprogdata.o:		uprogdata.c uprogdata.h

uclustername.o:		uclustername.c uclustername.h


# testing

X01= testgetrandom

X01OBJ= $(X01).o getrandom.o

$(X01).o:		$(X01).c

$(X01).x:		$(X01OBJ)
	$(CC) -o $@ $(X01OBJ) $(XLIBDIRS) $(XLIBS)


