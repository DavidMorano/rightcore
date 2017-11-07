# MAKEFILE

T= libcsdb

ALL= $(T).a $(T).so

SRCROOT= $(LOCAL)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib


CC= gcc
GPP= g++

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB)
#CCOPTS= -g -Wstrict-aliasing $(CCOPTS_GCCALL) $(CCOPTS_GCCLIB) 


LDSOARRAY=

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


SLIBDIRS= -L$(LIBDIR)

SLIB0=
SLIB1= -ldam -lb
SLIB2=
SLIB3= -luc -lu
SLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
SLIB5= $(LF_LIBS) 
SLIB6=
SLIB7=

SLIBS= $(SLIB0) $(SLIB1) $(SLIB2) $(SLIB3) $(SLIB4) $(SLIB5) $(SLIB6) $(SLIB7)

SLIBINFO= $(SLIBDIRS) $(SLIBS)


XLIBDIRS= -L$(LIBDIR)

XLIB0=
XLIB1= -ldam -lb
XLIB2= -ldebug
XLIB3= -lut -luc -lu
XLIB4= -L$(GNU)/lib -lstdc++ -lgcc_eh -lgcc
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


OBJ00= commandment.o
OBJ01= biblebook.o 
OBJ02= bibleverse.o 
OBJ03= bvs.o bvsmk.o bvsbook.o bvshdr.o
OBJ04= bibleq.o
OBJ05= biblepara.o
OBJ06= txtindex.o txtindexmk.o txtindexfu.o
OBJ07= cyi.o cyimk.o cyihdr.o
OBJ08= manstr.o
OBJ09= outema.o
OBJ10= dayofmonth.o
OBJ11= calyear.o
OBJ12= holidays.o holidayer.o
OBJ13= searchkeys.o
OBJ14= dirseen.o
OBJ15= offindex.o
OBJ16= wordfill.o
OBJ17= strstore.o
OBJ18= xwords.o
OBJ19= eigendb.o
OBJ20= vecint.o
OBJ21= babycalc.o babiesfu.o
OBJ22= cvtdater.o
OBJ23= sysvar.o

OBJ24= var.o varmk.o varhdr.o
OBJ25= buffer.o buffer_stropaque.o buffer_strquote.o
OBJ26= modload.o
OBJ27= fifostr.o
OBJ28=
OBJ29=
OBJ30=
OBJ31=

OBJ32= dayspec.o bcspec.o
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

OBJ96=
OBJ97=
OBJ98=
OBJ99=
OBJ100=
OBJ101=
OBJ102=
OBJ103=

OBJ104=
OBJ105=
OBJ106=
OBJ107=
OBJ108=
OBJ109=
OBJ110=
OBJ111=

OBJ112=
OBJ113=
OBJ114=
OBJ115=
OBJ116=
OBJ117=
OBJ118=
OBJ119=

OBJ120=
OBJ121=
OBJ122=
OBJ123=
OBJ124=
OBJ125=
OBJ126=
OBJ127=

OBJ128=
OBJ129=
OBJ130=
OBJ131=
OBJ132=
OBJ133=
OBJ134=
OBJ135=

OBJ136=
OBJ137=
OBJ138=
OBJ139=
OBJ140=
OBJ141=
OBJ142=
OBJ143=

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

OBJMa= $(OBJ96) $(OBJ97) $(OBJ98) $(OBJ99)
OBJMb= $(OBJ100) $(OBJ101) $(OBJ102) $(OBJ103)
OBJM= $(OBJMa) $(OBJMb)

OBJNa= $(OBJ104) $(OBJ105) $(OBJ106) $(OBJ107)
OBJNb= $(OBJ108) $(OBJ109) $(OBJ110) $(OBJ111)
OBJN= $(OBJNa) $(OBJNb)

OBJOa= $(OBJ112) $(OBJ113) $(OBJ114) $(OBJ115)
OBJOb= $(OBJ116) $(OBJ117) $(OBJ118) $(OBJ119)
OBJO= $(OBJOa) $(OBJOb)

OBJPa= $(OBJ120) $(OBJ121) $(OBJ122) $(OBJ123)
OBJPb= $(OBJ124) $(OBJ125) $(OBJ126) $(OBJ127)
OBJP= $(OBJPa) $(OBJPb)

OBJQa= $(OBJ128) $(OBJ129) $(OBJ130) $(OBJ131)
OBJQb= $(OBJ132) $(OBJ133) $(OBJ134) $(OBJ135)
OBJQ= $(OBJQa) $(OBJQb)

OBJRa= $(OBJ136) $(OBJ137) $(OBJ138) $(OBJ139)
OBJRb= $(OBJ140) $(OBJ141) $(OBJ142) $(OBJ143)
OBJR= $(OBJRa) $(OBJRb)

OBJg0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
OBJg1= $(OBJI) $(OBJJ) $(OBJK) $(OBJL) $(OBJM) $(OBJN) $(OBJO) $(OBJP)
OBJg2= $(OBJQ) $(OBJR)

OBJ= $(OBJg0) $(OBJg1) $(OBJg2)

OBJS0= obja.o objb.o objc.o objd.o obje.o
#OBJS0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o objj.o
#OBJS1= objk.o objl.o objm.o objn.o objo.o objp.o objq.o objr.o

OBJS= $(OBJS0) $(OBJS1)


.SUFFIXES:		.ls .i .cx .cs


default:		all

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

.cc.o:
	$(GPP) -c $(CFLAGS) $(CPPFLAGS) $<

.c.ln:
	$(LINT) -c $(LINTFLAGS) $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(T).so:		$(OBJS) Makefile $(T).a
	$(LD) -G -o $@ $(LDFLAGS) $(OBJS) $(SLIBINFO)

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
	makenewer -im -z -m o=h $(OBJM) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJN) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJO) -d $(INCDIR)
	makenewer -im -z -m o=h $(OBJP) -d $(INCDIR)

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
	makesafe -z -v=3 -I $(INCDIR) $(OBJM)
	makesafe -z -v=3 -I $(INCDIR) $(OBJN)
	makesafe -z -v=3 -I $(INCDIR) $(OBJO)
	makesafe -z -v=3 -I $(INCDIR) $(OBJP)
	makesafe -z -v=3 -I $(INCDIR) $(OBJQ)
	makesafe -z -v=3 -I $(INCDIR) $(OBJR)

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

objm.o:			$(OBJM)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJM)

objn.o:			$(OBJN)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJN)

objo.o:			$(OBJO)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJO)

objp.o:			$(OBJP)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJP)

objq.o:			$(OBJQ)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJQ)

objr.o:			$(OBJR)
	$(LD) -r -o $@ $(LDFLAGS) $(OBJR)


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

sigblock.o:		sigblock.c sigblock.h

sigign.o:		sigign.c sigign.h

userattr.o:		userattr.c userattr.h

ucsem.o:		ucsem.c ucsem.h

pta.o:			pta.c pta.h

ucsem.o:		ucsem.c ucsem.h

storeitem.o:		storeitem.c storeitem.h

storebuf.o:		storebuf.c storebuf.h

sbuf.o:			sbuf.c sbuf.h

sbuf_addquoted.o:	sbuf_addquoted.c sbuf.h

buffer.o:		buffer.c buffer.h

buffer_stropaque.o:	buffer_stropaque.c buffer.h

buffer_strquote.o:	buffer_strquote.c buffer.h

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

filebuf_writealign.o:	filebuf_writealign.c filebuf.h

filebuf_writezero.o:	filebuf_writezero.c filebuf.h

recarr.o:		recarr.c recarr.h

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_envfile.o:	vecstr_envfile.c vecstr.h

vecpstr.o:		vecpstr.c vecpstr.h

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

varray.o:		varray.c varray.h
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DVARRAY_LIBMEMALLOC=1 $<

commandment.o:		commandment.c commandment.h commandments.h

biblebook.o:		biblebook.c biblebook.h biblebooks.h

bibleverse.o:		bibleverse.c bibleverse.h bibleverses.h

bvs.o:			bvs.c bvs.h

bvsmk.o:		bvsmk.c bvsmk.h

bvsbook.o:		bvsbook.c bvsbook.h

bvshdr.o:		bvshdr.c bvshdr.h

bibleq.o:		bibleq.c bibleq.h

biblepara.o:		biblepara.c biblepara.h

txtindex.o:		txtindex.c txtindex.h txtindexes.h

txtindexmk.o:		txtindexmk.c txtindexmk.h txtindexmks.h

bibleqs.o:		bibleqs.c bibleqs.h

txtindexes.o:		txtindexes.c txtindexes.h txtindexfu.h

txtindexmks.o:		txtindexmks.c txtindexmks.h txtindexfu.h

txtindexfu.o:		txtindexfu.c txtindexfu.h

searchkeys.o:		searchkeys.c searchkeys.h

dirseen.o:		dirseen.c dirseen.h

offindex.o:		offindex.c offindex.h

wordfill.o:		wordfill.c wordfill.h

strstore.o:		strstore.c strstore.h

xwords.o:		xwords.c xwords.h

eigendb.o:		eigendb.c eigendb.h

vecint.o:		vecint.c vecint.h

namecache.o:		namecache.c namecache.h

gncache.o:		gncache.c gncache.h

address.o:		address.c address.h

babycalc.o:		babycalc.c babycalc.h

babiesfu.o:		babiesfu.c babiesfu.h

cvtdater.o:		cvtdater.c cvtdater.h

sysvar.o:		sysvar.c sysvar.h

var.o:			var.c var.h

varmk.o:		varmk.c varmk.h

varhdr.o:		varhdr.c varhdr.h

mxalias.o:		mxalias.c mxalias.h

issue.o:		issue.c issue.h

motd.o:			motd.c motd.h

holidays.o:		holidays.c holidays.h

holidayer.o:		holidayer.c holidayer.h holidays.h

calyear.o:		calyear.c calyear.h

calyears.o:		calyears.c calyears.h

dayofmonth.o:		dayofmonth.c dayofmonth.h

outema.o:		outema.c outema.h

manstr.o:		manstr.c manstr.h

cyi.o:			cyi.c cyi.h cyihdr.h

cyimk.o:		cyimk.c cyimk.h cyihdr.h

cyihdr.o:		cyihdr.c cyihdr.h

dayspec.o:		dayspec.c dayspec.h

bcspec.o:		bcspec.c bcspec.h

modload.o:		modload.c modload.h

fifostr.o:		fifostr.c fifostr.h


upt.o:			upt.c upt.h

base64.o:		base64.c base64.h

spawnproc.o:		spawnproc.c spawnproc.h

getxusername.o:		getxusername.c getxusername.h

getutmpent.o:		getutmpent.c getutmpent.h

getax.o:		getax.c getax.h

gethe.o:		gethe.c gethe.h

passwdent.o:		passwdent.c passwdent.h

shadowent.o:		shadowent.c shadowent.h

groupent.o:		groupent.c groupent.h

projectent.o:		projectent.c projectent.h

userattrent.o:		userattrent.c userattrent.h

utmpaccent.o:		utmpaccent.c utmpaccent.h

findbit.o:		findbit.c findbit.h

dialudp.o:		dialudp.c

dialtcp.o:		dialtcp.c

dialtcpmux.o:		dialtcpmux.c

dialtcpnls.o:		dialtcpnls.c nlsdialassist.h

nlsdialassist.o:	nlsdialassist.c nlsdialassist.h

opensysfs.o:		opensysfs.c opensysfs.h

quoteshellarg.o:	quoteshellarg.c

mkquoted.o:		mkquoted.c

snflags.o:		snflags.c snflags.h

sncpy.o:		sncpy.c

snsd.o:			snsd.c

snsds.o:		snsds.c

snfsflags.o:		snfsflags.c snflags.h

snopenflags.o:		snopenflags.c snflags.h

snpollflags.o:		snpollflags.c snflags.h

sntmtime.o:		sntmtime.c tmtime.h

mkpath.o:		mkpath.c

intsat.o:		intsat.c intsat.h

tmtime.o:		tmtime.c tmtime.h

ctdecp.o:		ctdecp.c ctdecp.h

getnodedomain.o:	getnodedomain.c

inetpton.o:		inetpton.c

matstr.o:		matstr.c

strshrink.o:		strshrink.c

stwhitedot.o:		sfwhitedot.c

strnchr.o:		strnchr.c

mkvarpath.o:		mkvarpath.c

mallocstuff.o:		mallocstuff.c

ndigits.o:		ndigits.c

calstrs.o:		calstrs.c calstrs.h


# testing

X01= testugetpw

X01OBJ= $(X01).o ugetpw.o upwcache.o recarr.o

$(X01).o:		$(X01).c ugetpw.h upwcache.h recarr.h

$(X01).x:		$(X01OBJ)
	$(CC) -o $@ $(X01OBJ) $(XLIBINFO)


X02= testutmpacc

X02OBJ= $(X02).o utmpacc.o utmpaccent.o filemap.o recarr.o

$(X02).o:		$(X02).c utmpacc.h utmpaccent.h filemap.h recarr.h

$(X02).x:		$(X02OBJ)
	$(CC) -o $@ $(X02OBJ) $(XLIBINFO)


XDIAL0= dialticotsordnls.o dialticotsordmux.o dialticotsord.o 
XDIAL1= filebuf.o opensysfs.o
XDIAL2= nlsdialassist.o strnwcpy.o
XDIAL3= snopenflags.o
XDIAL4= uc_reade.o uc_openuser.o uc_opensys.o uc_opendev.o

XDIALERS= $(XDIAL0) $(XDIAL1) $(XDIAL2) $(XDIAL3) $(XDIAL4)


X03= testucopen

X03OBJ00= $(X03).o 
X03OBJ01= uc_openinfo.o uc_openproto.o uc_openprog.o 
X03OBJ02= uc_opendialer.o uc_openfsvc.o uc_openusvc.o
X03OBJ03= $(XDIALERS)
X03OBJ04= attachso.o opensvc_hotd.o
X03OBJ05=

X03OBJ= $(X03OBJ00) $(X03OBJ01) $(X03OBJ02) $(X03OBJ03) $(X03OBJ04) $(X03OBJ05)

$(X03).o:		$(X03).c vsystem.h nlsdialassist.h

$(X03).x:		$(X03OBJ)
	$(CC) -o $@ $(X03OBJ) $(XLIBINFO)


