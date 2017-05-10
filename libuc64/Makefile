# MAKEFILE

T= libuc

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

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCLIB)
#CCOPTS= -g $(CCOPTS_GCCLIB)


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
TSORT= ${CCS}/bin/tsort

RM= rm -f
TOUCH= /bin/touch


INCS=


OBJ00= uc_sysconf.o uc_gethostid.o uc_confstr.o uc_gethostname.o
OBJ01= uc_memalloc.o uc_mallocstrw.o uc_mallocbuf.o
OBJ02= uc_opensocket.o uc_openproto.o uc_openprog.o uc_openpass.o uc_openuser.o
OBJ03= uc_accepte.o uc_write.o
OBJ04= uc_readn.o uc_reade.o uc_readline.o uc_writen.o uc_copy.o 
OBJ05= uc_recve.o uc_recvfrome.o uc_recvmsge.o
OBJ06= uc_stat.o uc_lstat.o uc_stat64.o uc_lstat64.o 
OBJ07= uc_statvfs.o uc_statvfs64.o
OBJ08= uc_lockf.o uc_remove.o uc_connecte.o
OBJ09= uc_fsync.o uc_fdatasync.o uc_fchmodsuid.o uc_chmodsuid.o
OBJ10= uc_moveup.o uc_closeonexec.o uc_nonblock.o uc_append.o uc_keepalive.o
OBJ11= uc_msgdiscard.o uc_reuseaddr.o
OBJ12= uc_aioread.o uc_aiowrite.o uc_aioreturn.o uc_aiolist.o uc_aiosuspend.o
OBJ13= uc_fattach.o uc_fdetach.o uc_mkfifo.o uc_msync.o uc_aiofsync.o 
OBJ14= uc_close.o
OBJ15= 
OBJ16= uc_getprotoby.o uc_getnetby.o 
OBJ17= uc_gethostby.o uc_getipnodeby.o uc_getservby.o 
OBJ18= uc_getaddrinfo.o uc_getnameinfo.o
OBJ19= 
OBJ20= uc_getcwd.o uc_initgroups.o uc_getpuid.o uc_getauid.o
OBJ21= uc_system.o uc_localtime.o uc_gmtime.o
OBJ22= uc_ftime.o uc_mktime.o
OBJ23= uc_getlogin.o uc_getrusage.o uc_getloadavg.o
#OBJ24= uc_mkdirp.o 
OBJ25= uc_strtod.o uc_inetpton.o uc_inetntop.o
OBJ26= uc_realpath.o uc_resolvepath.o
OBJ27= uc_sigemptyset.o uc_sigfillset.o uc_sigaddset.o uc_sigdelset.o
OBJ28= uc_sigismember.o
OBJ29= uc_sigignore.o
OBJ30= uc_settimeofday.o uc_gettimeofday.o
OBJ31= uc_getpw.o uc_getsp.o uc_getgr.o 
OBJ32= uc_getproj.o uc_getdefaultproj.o uc_inproj.o 
OBJ33= uc_getuserattr.o uc_kvamatch.o
OBJ34= uc_tcsetattr.o uc_tcgetattr.o uc_tcdrain.o uc_tcflush.o uc_tcflow.o
OBJ35= uc_cfsetispeed.o uc_cfsetospeed.o
OBJ36= uc_openpt.o uc_grantpt.o uc_unlockpt.o uc_ptsname.o uc_ttyname.o
OBJ37= uc_truncate.o uc_open.o uc_openinfo.o uc_ftruncate.o
OBJ38= uc_truncate64.o uc_open64.o uc_ftruncate64.o
OBJ39=
OBJ40= uc_safesleep.o uc_exit.o uc_getpriority.o uc_setpriority.o uc_isaexec.o
OBJ41= uc_openshm.o uc_unlinkshm.o 
OBJ42= uc_openpsem.o uc_unlinkpsem.o
OBJ43= uc_isatty.o uc_fminmod.o uc_minmod.o uc_link.o uc_unlink.o uc_mkdir.o
OBJ44=
OBJ45= 
OBJ46= mkvarpath.o
OBJ47= mkquoted.o bufprintf.o format.o toxc.o
OBJ48= matstr.o matcasestr.o matkeystr.o 
OBJ49= nextfield.o 
OBJ50= sfshrink.o sfdirname.o sfbasename.o sfwhitedot.o
OBJ51= strkeycmp.o strnkeycmp.o strnchr.o strnlen.o
OBJ52= 
OBJ53= strwcpy.o strwcpylc.o strwcpyuc.o
OBJ54= snwcpy.o sncpy.o sncpyw.o sncpylc.o sncpyuc.o sncat1.o sncat2.o
OBJ55= snsds.o snsd.o 
OBJ56= mkpath.o mknpath.o mkpathw.o 
OBJ57= strdcpy.o strdcpyw.o
OBJ58= 
OBJ59= dialudp.o dialusd.o dialpass.o
OBJ60= dialtcp.o dialtcpmux.o dialtcpnls.o 
OBJ61= dialuss.o dialussmux.o dialussnls.o
OBJ62= dialprog.o dialhttp.o
OBJ63= getnodedomain.o getxusername.o getutmpent.o getax.o
OBJ64= getprotofamily.o geteaddrinfo.o
OBJ65= 
OBJ66= opentmpfile.o mallocstuff.o
OBJ67= isasocket.o isinetaddr.o isindomain.o
OBJ68= inetpton.o perm.o sperm.o findxfile.o getprogpath.o xfile.o
OBJ69= nextpowtwo.o findbit.o base64.o
OBJ70= getpwd.o pathclean.o getenv2.o
OBJ71= 
OBJ72= filebuf.o vechand.o vecobj.o 
OBJ73= vecstr.o vecstr_adduniq.o
OBJ74= storeitem.o storebuf.o sbuf.o buffer.o field.o char.o
OBJ75= hostinfo.o hostaddr.o hostent.o inetaddr.o sockaddress.o 
OBJ76= envlist.o hdb.o lookaside.o pq.o ids.o linefold.o
OBJ77= fsdir.o ucsem.o pmq.o sigblock.o
OBJ78= pta.o ptma.o ptca.o ptrwa.o ptm.o ptc.o psem.o
OBJ79= 
OBJ80= ctbin.o cthex.o ctdec.o
OBJ81= cfbin.o cfhex.o cfdec.o cfoct.o cfnum.o
OBJ82=
OBJ83= nleadstr.o nleadcasestr.o nleadkeystr.o
OBJ84= isdigitlatin.o isprintlatin.o
OBJ85= hasuc.o haslc.o haswhite.o hasalldig.o hasvarpathprefix.o
OBJ86= upt.o mkcleanline.o ncols.o tabcols.o
OBJ87= vstrcmp.o vstrcmpr.o vstrkeycmp.o vstrkeydictcmp.o vstrsort.o

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

OBJG0= $(OBJA) $(OBJB) $(OBJC) $(OBJD) $(OBJE) $(OBJF) $(OBJG) $(OBJH) 
OBJG1= $(OBJI) $(OBJJ) $(OBJK)

OBJ= $(OBJG0) $(OBJG1)


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


$(T).so:		$(OBJ) Makefile localmisc.h $(T).a
	$(LD) -G -o $@ $(LDFLAGS) $(OBJ) $(LIBDIRS) $(LIBS)

$(T).a:			$(OBJ)
	$(AR) -rc $@ $?

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

install.pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		$(ALL) Makefile install-incs
	ranlib $(T).a
	makenewer $(T).a $(LIBDIR)/$(OFD)
	makenewer $(T).a $(LIBDIR)

install-incs:
	makenewer -z -m o=h $(OBJA) -d $(INCDIR)
	makenewer -z -m o=h $(OBJB) -d $(INCDIR)
	makenewer -z -m o=h $(OBJC) -d $(INCDIR)
	makenewer -z -m o=h $(OBJD) -d $(INCDIR)
	makenewer -z -m o=h $(OBJE) -d $(INCDIR)
	makenewer -z -m o=h $(OBJF) -d $(INCDIR)
	makenewer -z -m o=h $(OBJG) -d $(INCDIR)
	makenewer -z -m o=h $(OBJH) -d $(INCDIR)
	makenewer -z -m o=h $(OBJI) -d $(INCDIR)
	makenewer -z -m o=h $(OBJJ) -d $(INCDIR)
	makenewer -z -m o=h $(OBJK) -d $(INCDIR)

safe:
	makesafe -v=3 -I $(INCDIR) $(OBJA)
	makesafe -v=3 -I $(INCDIR) $(OBJB)
	makesafe -v=3 -I $(INCDIR) $(OBJC)
	makesafe -v=3 -I $(INCDIR) $(OBJD)
	makesafe -v=3 -I $(INCDIR) $(OBJE)
	makesafe -v=3 -I $(INCDIR) $(OBJF)
	makesafe -v=3 -I $(INCDIR) $(OBJG)
	makesafe -v=3 -I $(INCDIR) $(OBJH)
	makesafe -v=3 -I $(INCDIR) $(OBJI)
	makesafe -v=3 -I $(INCDIR) $(OBJJ)
	makesafe -v=3 -I $(INCDIR) $(OBJK)

again:
	rm -f $(ALL)

clean:
	rm -f $(ALL) *.o *.i *.ls *.ln

control:
	(uname -n ; date) > Control



uc_truncate64.o:	uc_truncate64.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DFUN64=1 $<

uc_open64.o:		uc_open64.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DFUN64=1 $<

uc_ftruncate64.o:	uc_ftruncate64.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -DFUN64=1 $<


uc_safesleep.o:		uc_safesleep.c

uc_getpuid.o:		uc_getpuid.c


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

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecobj.o:		vecobj.c vecobj.h

hdb.o:			hdb.c hdb.h

envlist.o:		envlist.c envlist.h

lookaside.o:		lookaside.c lookaside.h

pq.o:			pq.c pq.h

ids.o:			ids.c ids.h

linefold.o:		linefold.c linefold.h


upt.o:			upt.c upt.h

base64.o:		base64.c base64.h

getxusername.o:		getxusername.c getxusername.h

getutmpent.o:		getutmpent.c getutmpent.h

getax.o:		getax.c getax.h


dialudp.o:		dialudp.c

dialtcp.o:		dialtcp.c

dialtcpmux.o:		dialtcpmux.c

dialtcpnls.o:		dialtcpnls.c

quoteshellarg.o:	quoteshellarg.c

mkquoted.o:		mkquoted.c

snsd.o:			snsd.c

snsds.o:		snsds.c

gethe.o:		gethe.c

gethename.o:		gethename.c

getheaddr.o:	 	getheaddr.c

getnodedomain.o:	getnodedomain.c

inetpton.o:		inetpton.c

matstr.o:		matstr.c

strshrink.o:		strshrink.c

stwhitedot.o:		sfwhitedot.c

strnchr.o:		strnchr.c

mkpath.o:		mkpath.c

mkvarpath.o:		mkvarpath.c

sncpy.o:		sncpy.c

mallocstuff.o:		mallocstuff.c



