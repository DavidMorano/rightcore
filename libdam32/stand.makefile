# MAKEFILE

T= libdam

ALL= $(T).a

SRCROOT= $(HOME)


BINDIR= $(SRCROOT)/bin
INCDIR= $(SRCROOT)/include
LIBDIR= $(SRCROOT)/lib

#LDCRTDIR= /opt/SUNWspro/WS6/lib
#LDCRTDIR= /opt/SUNWspro/SC5.0/lib
#LDCRTDIR= /opt/SUNWspro/SC4.0/lib
#LDCRTDIR= /opt/SUNWspro/lib
LDCRTDIR= $(SRCROOT)/lib


CC= gcc

CCOPTS= $(CCOPTS_GCCOPT) $(CCOPTS_GCCLIB)
#CCOPTS= $(CCOPTS_GCCLIB)
#CCOPTS= -g $(CCOPTS_GCCLIB)


# HyperSPARC
#CCOPTS= -xO5 -K pic -xtarget=ss20/hs22 -xdepend

# UltraSPARC
#CCOPTS= -xO5 -K pic -xtarget=ultra -xsafe=mem -xdepend

# UltraSPARC-v9 (64 bits)
#CCOPTS= -xO5 -xtarget=ultra -xarch=v9 -xsafe=mem -xdepend


DEF0= -DPOSIX=1 -DPTHREAD=1
DEF1= -DOSNAME_$(SYSNAME) -DOSTYPE_$(OSTYPE) -DOSNUM=$(OSNUM) 
DEF2= -D__EXTENSIONS__=1 -D_REENTRANT=1 
DEF3= -D_POSIX_C_SOURCE=199506L 
DEF4= -D_POSIX_PTHREAD_SEMANTICS=1 -D_POSIX_PER_PROCESS_TIMER_SOURCE=1 
DEF5=
DEF6=
DEF7= $(LF_DEFS) 

DEFS= $(DEF0) $(DEF1) $(DEF2) $(DEF3) $(DEF4) $(DEF5) $(DEF6) $(DEF7)

INCDIRS= -I$(INCDIR)

CPPFLAGS= $(DEFS) $(INCDIRS)
CFLAGS= $(CCOPTS) $(LF_CFLAGS) $(CPPFLAGS)

#LD= $(CC)
LDFLAGS= $(LF_LDFLAGS)

LIBDIRS=

LIBS= $(LF_LIBS) -L$(GNU)/lib -lgcc

LINT= lint
LINTFLAGS= $(LF_LINTFLAGS) -uxn

NM= nm
NMFLAGS= -sx -v

CPP= cpp

LORDER= lorder
TSORT= tsort

RM= rm -f
TOUCH= /bin/touch


INSTALLINC0= install.inca install.incb install.incc install.incd
INSTALLINC1= install.ince install.incf

INSTALLINCS= $(INSTALLINC0) $(INSTALLINC1)


I00= vsystem.h exitcodes.h misc.h
I01= vechand.h vecstr.h vecitem.h vecobj.h vecint.h
I02= hdb.h shdb.h lookaside.h pq.h plainq.h
I03= field.h userinfo.h 
I04= randomvar.h entropy.h randmwc.o random.h
I05= date.h tmz.h
I06= termstr.h
I07= kinfo.h loadave.h fsdir.h fsdirtree.h
I08= logfile.h lfm.h lastlogfile.h tmpx.h filemap.h
I09= storeitem.h char.h 
I10= mallocstuff.h bitops.h
I11=
I12=
I13=
I14=
I15=

INCA= $(I00) $(I01) $(I02) $(I03)
INCB= $(I04) $(I05) $(I06) $(I07)
INCC= $(I08) $(I09) $(I10) $(I11)
INCD= $(I12) $(I13) $(I14) $(I15)
INCE= $(I16) $(I17) $(I18) $(I19) 
INCF= $(I20) $(I21) $(I22) $(I32)

INCS= $(INCA) $(INCB) $(INCC) $(INCD)


OBJ00= clow.o cup.o movc.o cmpc.o
OBJ01= substring.o shrink.o strwhite.o 
OBJ02= strbasename.o strdirname.o strdomain.o strshrink.o 
OBJ03= strkeycmp.o strkeydictcmp.o strnkeycmp.o strpcmp.o strdictcmp.o
OBJ04= vstrkeycmp.o vstrkeydictcmp.o vstrcmp.o vstrcmpr.o
OBJ05= strsub.o strleadcmp.o strnleadcmp.o strtoken.o strrpbrk.o
OBJ06= strnlen.o strnchr.o strnrchr.o strnpbrk.o strnrpbrk.o
OBJ07= strcpylow.o strcpyup.o strncpylow.o strncpyup.o 
OBJ08= strwcpy.o strwcpylow.o strwcpyup.o
OBJ09=
OBJ10= 
OBJ11= 
OBJ12= mallocstuff.o 
OBJ13= logfile.o lfm.o lastlogfile.o filemap.o tmpx.o
OBJ14= getenv2.o getenv3.o delenv.o env.o
OBJ15= putheap.o cpystr.o cpywstr.o
OBJ16= optmatch.o optmatch2.o optmatch3.o starmat.o varsub.o
OBJ17= field.o field_word.o field_srvarg.o nextfield.o bits.o
OBJ18= vechand.o vecstr.o vecitem.o vecobj.o vecint.o
OBJ19= hdb.o shdb.o lookaside.o hdbstr.o mapstrint.o paramopt.o keyopt.o
OBJ20= q.o plainq.o pq.o cpq.o fifostr.o fifoitem.o charq.o 
OBJ21= getpwd.o getnodedomain.o nisdomainname.o
OBJ22= rexecl.o rcmdu.o qualdisplay.o quoteshellarg.o 
OBJ23= rex.o rfile.o dupup.o lockfile.o
OBJ24= msgenv.o msg.o msgheaders.o ema.o matmsg.o matenv.o
OBJ25= timestr_gmtlog.o timestr_log.o timestr_logz.o 
OBJ26= timestr_date.o timestr_nist.o timestr_elapsed.o timevalstr_ulog.o 
OBJ27= mktmpfile.o mktmplock.o mkjobfile.o mkdatefile.o 
OBJ28= mkpath1.o mkpath2.o mkpath3.o mkpath4.o mkpath5.o 
OBJ29= mkfname2.o mkfname3.o mkfnamesuf.o mkfnamesuf2.o
OBJ30= isproc.o termdevice.o makedirs.o mkdirs.o upwdb.o
OBJ31= inetping.o cleanpath.o
OBJ32= isasocket.o isinteractive.o
OBJ33= isinetaddr.o isindomain.o issamehostname.o 
OBJ34= getehostname.o getchostname.o getcanonical.o gethe.o
OBJ35= gethename.o getheaddr.o getcname.o
OBJ36= sfsub.o sfskipwhite.o sfshrink.o sfdirname.o sfbasename.o sfkey.o
OBJ37= sisub.o sibreak.o sibasename.o
OBJ38= mailenvelope_parse.o pwfile.o
OBJ39= buffer.o bufstr.o storebuf.o sbuf.o storeitem.o dstr.o
OBJ40= serialbuf.o srvreg.o srvrege.o
OBJ41= sperm.o perm.o permsched.o fperm.o fperm64.o expandcookie.o schedvar.o
OBJ42= dialprog.o progspawn.o openport.o
OBJ43= dialtcp.o dialtcpmux.o dialtcpnls.o dialudp.o dialuss.o dialusd.o
OBJ44= dialticotsord.o dialticotsordnls.o dialussmux.o dialussnls.o
OBJ45= listentcp.o listenudp.o listenuss.o listenfifo.o acceptpass.o
OBJ46= base64.o netorder.o stdorder.o getserial.o 
OBJ47= sockaddress.o hostent.o hostaddr.o inetaddr.o 
OBJ48= unlinkd.o wdt.o wdt64.o getfiledirs.o findfilepath.o getprogpath.o
OBJ49= bopensched.o bopenshell.o bopenrcmde.o bopenprog.o
OBJ50= readn.o writen.o
OBJ51= daytime.o dater.o tmz.o comparse.o
OBJ52= opentmpfile.o opentmpusd.o
OBJ53= randomvar.o entropy.o randmwc.o randlc.o random.o
OBJ54= matstr.o matpstr.o matcasestr.o matkeystr.o
OBJ55= findbit.o qsort.o 
OBJ56= isprintlatin.o isalnumlatin.o
OBJ57= mesg.o getloginterm.o
OBJ58= procnoise.o sysnoise.o rijndael.o
OBJ59= cfhex.o cfdec.o cfoct.o cfbin.o cfdouble.o cfdecf.o cfhexs.o
OBJ60= cfnum.o cfdecmf.o cfdect.o 
OBJ61= cthex.o ctdec.o ctdecpi.o ctbin.o
OBJ62= 
OBJ63= nextpowtwo.o cksum.o sha.o elfhash.o
OBJ64= kvstab.o svctab.o paramfile.o
OBJ65= freadline.o fbread.o fbwrite.o
OBJ66= loadave.o 
OBJ67= pmq.o psem.o fmq.o dw.o 
OBJ68= fmeanvaral.o fmeanvarai.o inetpton.o cpuspeed.o msleep.o
OBJ69= char.o strnncpy.o mkutmpid.o
OBJ70= strdcpy1.o strdcpy2.o strdcpy3.o
OBJ71= sncpy1.o sncpy2.o sncpy3.o sncpy4.o sncpy5.o
OBJ72= snddd.o snsds.o snscs.o snses.o snsd.o
OBJ73=
OBJ74= filebuf.o netfile.o ids.o
OBJ75= fsdir.o fsdirtree.o
OBJ76= mklogid.o
OBJ77= msfile.o msfilee.o nodedb.o clusterdb.o kinfo.o
OBJ78= density.o densitystati.o densitystatll.o denpercentsi.o
OBJ79= uceil.o ufloor.o

OBJ80= getlogname.o getusername.o
OBJ81= getpwusername.o getpwlogname.o 
OBJ82= getloghost.o getutmpname.o 
OBJ83= userinfo.o udomain.o
OBJ84= realname.o gecos.o gecosname.o mailname.o 
OBJ85=
OBJ86=
OBJ87=

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

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) \
	$(OBJE) $(OBJF) $(OBJG) $(OBJH) \
	$(OBJI) $(OBJJ) $(OBJK)

OBJS0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o objj.o
OBJS1= objk.o

OBJS= $(OBJS0) $(OBJS1)



.SUFFIXES:		.ls .i .cx .cs


default:		install.incs all

all:			$(ALL)

.c.o:
	$(CC) -c $(CFLAGS) $<

.c.ln:
	$(LINT) -c -u $(CPPFLAGS) $<

.c.ls:
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) $<

.c.i:
	$(CPP) $(CPPFLAGS) $< > $(*).i


$(T).a:			$(OBJ)
	$(AR) -cr $(T).a $?

$(T).so:		$(OBJS) Makefile misc.h $(T).a
	$(LD) -o $@ -G $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS)

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

safe:
	makesafe -v -I $(INCDIR) $(OBJ)

install.pre:
	find *.h -print | cpio -pdmu $(INCDIR)

install:		install.incs Makefile $(ALL)
	makenewer -r $(ALL) -d $(SRCROOT)/lib/$(OFD)
	makenewer -r $(ALL) -d $(SRCROOT)/lib

install.incs:		install.ih install.io
	touch install.incs

install.ih:		$(INSTALLINCS)
	touch install.ih

install.inca:		$(INCA)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.incb:		$(INCB)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.incc:		$(INCC)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.incd:		$(INCD)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.ince:		$(INCE)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.incf:		$(INCF)
	find $? -print | cpio -pdmu $(INCDIR)
	touch $@

install.io:		$(OBJ)
	find *.h -print | cpio -pdmu $(INCDIR)
	touch $@

other:			here
	find *.h -print | cpio -pdmu $(INCDIR)
	touch $@

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

control:
	(uname -n ; date) > Control


obja.o:			$(OBJA)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJA)

objb.o:			$(OBJB)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJB)

objc.o:			$(OBJC)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJC)

objd.o:			$(OBJD)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJD)

obje.o:			$(OBJE)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJE)

objf.o:			$(OBJF)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJF)

objg.o:			$(OBJG)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJG)

objh.o:			$(OBJH)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJH)

obji.o:			$(OBJI)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJI)

objj.o:			$(OBJJ)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJJ)

objk.o:			$(OBJK)
	$(LD) -o $@ -r $(LDFLAGS) $(OBJK)


cpuspeed.o:	cpuspeed.c


field.o:	field.c field.h

field_word.o:	field_word.c field.h

field_srvarg.o:	field_srvarg.c field.h

dstr.o:		dstr.c dstr.h

fifostr.o:	fifostr.c fifostr.h

fifoelem.o:	fifoelem.c fifoelem.h

vechand.o:	vechand.c vechand.h

vecstr.o:	vecstr.c vecstr.h

vecitem.o:	vecitem.c vecitem.h

vecobj.o:	vecobj.c vecobj.h

vecint.o:	vecint.c vecint.h

vecelem.o:	vecelem.c vecelem.h

userinfo.o:	userinfo.c userinfo.h

rfile.o:	rfile.c incfile_rfilewrite.h

incfile_rfilewrite.h:	rfilewrite
	mkincfile rfilewrite

getdate.o:	getdate.mod
	cp -p getdate.mod getdate.o

hdb.o:		hdb.c hdb.h

shdb.o:		shdb.c shdb.h

hdbstr.o:	hdbstr.c hdbstr.h

paramopt.o:	paramopt.c paramopt.h

lastlogfile.o:	lastlogfile.c lastlogfile.h

loadave.o:	loadave.c loadave.h

entropy.o:	entropy.c entropy.h

sha.o:		sha.c sha.h

buffer.o:	buffer.c buffer.h

bufstr.o:	bufstr.c bufstr.h

sbuf.o:		sbuf.c sbuf.h

lookaside.o:	lookaside.c lookaside.h

filemap.o:	filemap.c filemap.h

q.o:		q.c q.h

plainq.o:	plainq.c plainq.h

pq.o:		pq.c pq.h

cpq.o:		cpq.c cpq.h

charq.o:	charq.c charq.h

aiq.o:		aiq.c aiq.h

serialbuf.o:	serialbuf.c serialbuf.h

netorder.o:	netorder.c netorder.h

storeitem.o:	storeitem.c storeitem.h

outbuf.o:	outbuf.c outbuf.h

matenv.o:	matenv.c matenv.h

msg.o:		msg.c msg.h matenv.h

char.o:		char.c char.h

date.o:		date.c date.h

dater.o:	dater.c dater.h

comparse.o:	comparse.c comparse.h

realname.o:	realname.c realname.h

inetaddr.o:	inetaddr.c inetaddr.h

gecos.o:	gecos.c gecos.h

kinfo.o:	kinfo.c kinfo.h

msfile.o:	msfile.c msfile.h msfilee.h

msfilee.o:	msfilee.c msfilee.h

mapstrint.o:	mapstrint.c mapstrint.h

srvreg.o:	srvreg.c srvreg.h srvrege.h

srvrege.o:	srvrege.c srvrege.h

sockaddress.o:	sockaddress.c sockaddress.h

progspawn.o:	progspawn.c progspawn.h

openport.o:	openport.c openport.h

pmq.o:		pmq.c pmq.h

psem.o:		psem.c psem.h

fmq.o:		fmq.c fmq.h

paramfile.o:	paramfile.c paramfile.h

kvstab.o:	kvstab.c kvstab.h

svctab.o:	svctab.c svctab.h

acctab.o:	acctab.c acctab.h

srvtab.o:	srvtab.c srvtab.h

schedvar.o:	schedvar.c schedvar.h

varsub.o:	varsub.c varsub.h

filebuf.o:	filebuf.c filebuf.h

bits.o:		bits.c bits.h

msgenv.o:	msgenv.c msgenv.h

rijndael.o:	rijndael.c rijndael.h

ema.o:		ema.c ema.h

mimetypes.o:	mimetypes.c mimetypes.h

ids.o:		ids.c ids.h

randomvar.o:	randomvar.c randomvar.h

fsdir.o:	fsdir.c fsdir.h

fsdirtree.o:	fsdirtree.c fsdirtree.h

density.o:	density.c density.h

dialtab.o:	dialtab.c dialtab.h

nodedb.o:	nodedb.c nodedb.h

clusterdb.o:	clusterdb.c clusterdb.h

hostent.o:	hostent.c hostent.h

hostaddr.o:	hostaddr.c hostaddr.h

lfm.o:		lfm.c lfm.h

keyopt.o:	keyopt.c keyopt.h



wdt64.o:	wdt.c
	rm -f wdt64.c
	cp -p wdt.c wdt64.c
	$(CC) -c $(CFLAGS) -DCF_LF64=1 wdt64.c




