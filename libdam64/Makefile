# MAKEFILE

T= libdam

ALL= $(T).a

SRCROOT= $(LOCAL)


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
CCOPTS= -g $(CCOPTS_GCCLIB)


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

CFLAGS= $(CCOPTS) $(LF_CFLAGS) $(CPPFLAGS)

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
TOUCH= /usr/bin/touch


INSTALLINC0= install.inca install.incb install.incc install.incd
INSTALLINC1= install.ince install.incf

INSTALLINCS= $(INSTALLINC0) $(INSTALLINC1)


I00= vsystem.h exitcodes.h localmisc.h
I01= vechand.h vecstr.h vecitem.h vecobj.h vecint.h
I02= hdb.h lookaside.h pq.h plainq.h
I03= field.h 
I04= randomvar.h egs.h randmwc.o random.h
I05= zos.h tmz.h date.h zoffparts.o
I06= termstr.h
I07= kinfo.h loadave.h fsdir.h fsdirtree.h
I08= logfile.h lfm.h lastlogfile.h tmpx.h filemap.h
I09= storeitem.h 
I10= mallocstuff.h userinfo.h 
I11= char.h bitops.h 
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

INCS= $(INCA) $(INCB) $(INCC) $(INCD) $(INCE) $(INCF) 


OBJ00= substring.o strwhite.o strdomain.o strpcmp.o strwcmp.o strnncmp.o
OBJ01= strbasename.o strdirname.o strshrink.o 
OBJ02= strdictcmp.o strnndictcmp.o
OBJ03= strkeycmp.o strkeydictcmp.o strnkeycmp.o 
OBJ04= vstrkeycmp.o vstrkeydictcmp.o vstrcmp.o vstrcmpr.o
OBJ05= strsub.o strleadcmp.o strnleadcmp.o strtoken.o strrpbrk.o
OBJ06= strnchr.o strnrchr.o strnpbrk.o strnrpbrk.o
OBJ07= strcpylc.o strcpyuc.o strncpylc.o strncpyuc.o 
OBJ08= strwcpy.o strwcpylc.o strwcpyuc.o
OBJ09= cpystr.o cpywstr.o 
OBJ10= starmat.o nleadstr.o nleadcasestr.o nleadkeystr.o 
OBJ11= matstr.o matcasestr.o matostr.o matocasestr.o matpstr.o matpcasestr.o
OBJ12= matkeystr.o matnstr.o
OBJ13= strcasestr.o strnstr.o strncasestr.o strnsub.o strncasesub.o
OBJ14= filemap.o tmpx.o varsub.o mailalias.o bits.o
OBJ15= mallocstuff.o strnlen.o strnnlen.o strlinelen.o
OBJ16= getenv2.o getenv3.o delenv.o env.o
OBJ17= field.o field_word.o field_wordphrase.o field_srvarg.o field_svcargs.o
OBJ18= vechand.o vecitem.o vecint.o
OBJ19= vecstr.o vecstr_adduniq.o vecstr_env.o vecstr_loadfile.o vecstr_foilcmp.o
OBJ20= vecobj.o vecpstr.o
OBJ21= hdb.o lookaside.o hdbstr.o mapstrint.o paramopt.o keyopt.o keyvals.o
OBJ23= q.o plainq.o pq.o cpq.o cq.o ciq.o fifostr.o fifoitem.o charq.o 
OBJ22= logfile.o logfile_userinfo.o lfm.o lastlogfile.o 
OBJ24= rexecl.o rcmdu.o qualdisplay.o mkquoted.o
OBJ25= rex.o rfile.o dupup.o lockfile.o lockend.o
OBJ26= timestr_date.o timestr_nist.o timestr_elapsed.o timevalstr_ulog.o 
OBJ27= mktmpfile.o mktmplock.o mkjobfile.o mkdatefile.o
OBJ28= getnodename.o getclustername.o getnodedomain.o nisdomainname.o getpwd.o
OBJ29= isasocket.o isinteractive.o isfsremote.o
OBJ30= isproc.o termdevice.o makedirs.o mkdirs.o chmods.o
OBJ31= inetping.o pathclean.o
OBJ32= isinetaddr.o isindomain.o issamehostname.o 
OBJ33= getehostname.o getchostname.o geteaddrinfo.o getcanonical.o gethe.o
OBJ34= gethename.o getheaddr.o getcname.o getportnum.o getfstype.o
OBJ35= sfsub.o sfcasesub.o sfkey.o sfbreak.o sfskipwhite.o sfshrink.o sfword.o
OBJ36= sfdirname.o sfbasename.o sfthing.o sfwhitedot.o
OBJ37= sisub.o sicasesub.o sibreak.o siskipwhite.o sibasename.o sispan.o
OBJ38= pwfile.o pwi.o ipasswd.o mailaddrquote.o
OBJ39= buffer.o bufstr.o storebuf.o sbuf.o sbuf_blanks.o sbuf_termconseq.o
OBJ40= serialbuf.o srvreg.o srvrege.o wordfill.o linefold.o storeitem.o dstr.o 
OBJ41= permsched.o fperm.o fperm64.o expcook.o schedvar.o
OBJ42= dialprog.o spawnproc.o openport.o
OBJ43= dialtcp.o dialtcpmux.o dialtcpnls.o dialudp.o dialuss.o dialusd.o
OBJ44= dialticotsord.o dialticotsordnls.o dialussmux.o dialussnls.o
OBJ45= dialhttp.o
OBJ46= listentcp.o listenudp.o listenuss.o listenfifo.o listenpass.o 
OBJ47= hostaddr.o hostinfo.o hostent.o inetaddr.o sockaddress.o 
OBJ48= wdt.o base64.o netorder.o stdorder.o getserial.o 
OBJ49= bopensched.o bopenshell.o bopenrcmde.o bopenprog.o
OBJ50= readn.o writen.o acceptpass.o
OBJ51= daytime.o comparse.o mhcom.o zos.o tmz.o dater.o zoffparts.o
OBJ52= opentmpfile.o
OBJ53= randomvar.o egs.o randmwc.o randlc.o random.o
OBJ54= char.o findbit.o toxc.o
OBJ55= iswhite.o isdict.o isprintterm.o isprintbad.o
OBJ56= isprintlatin.o isalnumlatin.o isalphalatin.o isdigitlatin.o
OBJ57= haswhite.o hasuc.o haslc.o hasprintbad.o 
OBJ58= hasalllc.o hasalluc.o hasalldig.o hasallalnum.o
OBJ59= ctbin.o cfhex.o cfdec.o cfoct.o cfdouble.o cfdecf.o cfhexs.o cfb26.o
OBJ60= cfnum.o cfdecmf.o cfdect.o 
OBJ61= ctbin.o cthex.o ctdec.o ctdecpi.o ctb26.o
OBJ62= 
OBJ63= cksum.o sha1.o gecos.o 
OBJ64= kvsfile.o paramfile.o strtab.o strpack.o strstore.o svcfile.o
OBJ65= freadline.o 
OBJ66= fbreadline.o fbread.o fbwrite.o fbprint.o fbprintf.o
OBJ67= nextpowtwo.o hashelf.o hashagain.o urotate.o
OBJ68= fmeanvaral.o fmeanvarai.o inetpton.o inetntop.o cpuspeed.o msleep.o
OBJ69= mkutmpid.o optbool.o optvalue.o
OBJ70= snddd.o snsds.o snscs.o snses.o snsd.o snsdd.o snfsflags.o
OBJ71= sncpy.o sncpylc.o sncpyuc.o sncpyw.o
OBJ72= snwcpy.o snwcpylc.o snwcpyuc.o snwcpyhyphen.o snwcpylatin.o snwcpyclean.o
OBJ73= sncat1.o sncat2.o
OBJ74= strnwcpy.o strnncpy.o sncpyw.o
OBJ75= strdcpy.o strdfill.o
OBJ76=
OBJ77= strwset.o strwcpyblanks.o strnset.o strncpyblanks.o
OBJ78= 
OBJ79=
OBJ80= getlogname.o getloghost.o
OBJ81= getpwentry.o getxusername.o getpwlogname.o getgroupname.o 
OBJ82= density.o densitystati.o densitystatll.o denpercents.o
OBJ83= userinfo.o tmstrs.o
OBJ84= udomain.o getutmpent.o getutmpterm.o 
OBJ85= mklogid.o mkuiname.o mkbestaddr.o mkuibang.o 
OBJ86= mkgecosname.o mkmailname.o mkrealname.o
OBJ87= mktmpuserdir.o statvfsdir64.o

OBJ88= mkpath.o mknpath.o mkpathw.o
OBJ89= mkfnamesuf.o mkfname.o
OBJ90= unlinkd.o openshmtmp.o mapshmtmp.o
OBJ91= getax.o userattr.o mailmsgmatenv.o mailmsgmathdr.o ema.o
OBJ92= mailmsgenv.o mailmsg.o mailmsg_loadfile.o mailmsghdrs.o mailmsgstage.o
OBJ93= mailbox.o mailmsghdrval.o mailmsghdrfold.o
OBJ94= msfile.o msfilee.o ebuf.o nodedb.o clusterdb.o kinfo.o nulstr.o
OBJ95= fsdir.o fsdirtree.o filebuf.o netfile.o ids.o realname.o termstr.o

OBJ96= tabcols.o ncols.o nextfield.o removes.o fsdirtreestat.o
OBJ97= nusers.o mesg.o getprotofamily.o mknoise.o sysnoise.o rijndael.o
OBJ98= getfiledirs.o findfilepath.o findfile.o findxfile.o getprogpath.o xfile.o
OBJ99= cvttemperature.o dictdiff.o bufprintf.o format.o
OBJ100= getmjd.o ncpu.o getsysmisc.o mapex.o hdrextnum.o getev.o termconseq.o
OBJ101=
OBJ102= loadave.o modload.o dirseen.o sysmiscem.o sysmiscfh.o
OBJ103= dw.o fmq.o sigblock.o

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

OBJM0= $(OBJ96) $(OBJ97) $(OBJ98) $(OBJ99) 
OBJM1= $(OBJ100) $(OBJ101) $(OBJ102) $(OBJ103)

OBJM= $(OBJM0) $(OBJM1)

OBJ= $(OBJA) $(OBJB) $(OBJC) $(OBJD) \
	$(OBJE) $(OBJF) $(OBJG) $(OBJH) \
	$(OBJI) $(OBJJ) $(OBJK) $(OBJL) $(OBJM)

OBJS0= obja.o objb.o objc.o objd.o obje.o objf.o objg.o objh.o obji.o objj.o
OBJS1= objk.o objl.o objm.o

OBJS= $(OBJS0) $(OBJS1)


.SUFFIXES:		.ls .i .cx .cs


default:		belowincs install.incs all

all:			below $(ALL) 

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

$(T).so:		$(OBJS) Makefile localmisc.h $(T).a
	$(LD) -G -o $@ $(LDFLAGS) $(OBJS) $(LIBDIRS) $(LIBS)

$(T).nm:		$(T).so
	$(NM) $(NMFLAGS) $(T).so > $(T).nm

$(T).order order:	$(OBJ) $(T).a
	$(LORDER) $(T).a | $(TSORT) > $(T).order
	$(RM) $(T).a
	while read O ; do $(AR) -cr $(T).a $${O} ; done < $(T).order

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
	makesafe -v=3 -I $(INCDIR) $(OBJL)
	makesafe -v=3 -I $(INCDIR) $(OBJM)

install.pre:
	filefind . -s h | makenewer -af - -d $(INCDIR)

install:		install.incs Makefile $(ALL)
	makenewer -r $(ALL) -d $(SRCROOT)/lib

install.incs:		install.ih install.io
	touch install.incs

install.ih:		$(INSTALLINCS)
	touch install.ih

install.inca:		$(INCA)
	makenewer -z $? -d $(INCDIR) -t $@

install.incb:		$(INCB)
	makenewer -z $? -d $(INCDIR) -t $@

install.incc:		$(INCC)
	makenewer -z $? -d $(INCDIR) -t $@

install.incd:		$(INCD)
	makenewer -z $? -d $(INCDIR) -t $@

install.ince:		$(INCE)
	makenewer -z $? -d $(INCDIR) -t $@

install.incf:		$(INCF)
	makenewer -z $? -d $(INCDIR) -t $@

install.io:		$(OBJ)
	makenewer -z -m o=h $? -d $(INCDIR) -t $@

other:			here
	makenewer -z -m o=h $(OBJA) -d $(INCDIR)
	makenewer -z -m o=h $(OBJB) -d $(INCDIR)
	makenewer -z -m o=h $(OBJC) -d $(INCDIR)
	makenewer -z -m o=h $(OBJD) -d $(INCDIR)
	makenewer -z -m o=h $(OBJE) -d $(INCDIR)
	makenewer -z -m o=h $(OBJF) -d $(INCDIR)
	makenewer -z -m o=h $(OBJG) -d $(INCDIR)
	makenewer -z -m o=h $(OBJH) -d $(INCDIR)
	makenewer -z -m o=h $(OBJI) -d $(INCDIR)
	makenewer -z -m o=h $(OBJJ) -d $(INCDIR) -t $@
	makenewer -z -m o=h $(OBJK) -d $(INCDIR)
	makenewer -z -m o=h $(OBJL) -d $(INCDIR)
	makenewer -z -m o=h $(OBJM) -d $(INCDIR)

again:
	$(RM) $(ALL)

clean:			again
	$(RM) *.o $(ALL)

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


field.o:		field.c field.h

field_word.o:		field_word.c field.h

field_srvarg.o:		field_srvarg.c field.h

dstr.o:			dstr.c dstr.h

fifostr.o:		fifostr.c fifostr.h

fifoitem.o:		fifoitem.c fifoitem.h

vechand.o:		vechand.c vechand.h

vecstr.o:		vecstr.c vecstr.h

vecstr_adduniq.o:	vecstr_adduniq.c vecstr.h

vecstr_env.o:		vecstr_env.c vecstr.h

vecstr_loadfile.o:	vecstr_loadfile.c vecstr.h

vecstr_foilcmp.o:	vecstr_foilcmp.c vecstr.h

vecpstr.o:		vecpstr.c vecpstr.h

vecitem.o:		vecitem.c vecitem.h

vecobj.o:		vecobj.c vecobj.h

vecint.o:		vecint.c vecint.h

vecelem.o:		vecelem.c vecelem.h

hdb.o:			hdb.c hdb.h

hdbstr.o:		hdbstr.c hdbstr.h

keyvals.o:		keyvals.c keyvals.h

paramopt.o:		paramopt.c paramopt.h

logfile.o:		logfile.c logfile.h

logfile_userinfo.o:	logfile_userinfo.c logfile.h userinfo.h

lastlogfile.o:		lastlogfile.c lastlogfile.h

loadave.o:		loadave.c loadave.h

egs.o:			egs.c egs.h

sha1.o:			sha1.c sha1.h

buffer.o:		buffer.c buffer.h

bufstr.o:		bufstr.c bufstr.h

sbuf.o:			sbuf.c sbuf.h

lookaside.o:		lookaside.c lookaside.h

filemap.o:		filemap.c filemap.h

q.o:			q.c q.h

plainq.o:		plainq.c plainq.h

pq.o:			pq.c pq.h

cpq.o:			cpq.c cpq.h

charq.o:		charq.c charq.h

aiq.o:			aiq.c aiq.h

cq.o:			cq.c cq.h

ciq.o:			ciq.c ciq.h

serialbuf.o:		serialbuf.c serialbuf.h

netorder.o:		netorder.c netorder.h

storeitem.o:		storeitem.c storeitem.h

outbuf.o:		outbuf.c outbuf.h

matenv.o:		matenv.c matenv.h

mailbox.o:		mailbox.c mailbox.h

mailmsgmatenv.o:	mailmsgmatenv.c mailmsgmatenv.h

mailmsg.o:		mailmsg.c mailmsg.h mailmsgmatenv.h

mailmsg_loadfile.o:	mailmsg_loadfile.c mailmsg.h

mailmsghdrs.o:		mailmsghdrs.c mailmsghdrs.h mailmsg.h

mailmsgstage.o:		mailmsgstage.c mailmsgstage.h 

mailmsghdrval.o:	mailmsghdrval.c mailmsghdrval.h

mailmsghdrfold.o:	mailmsghdrfold.c mailmsghdrfold.h

mailaddrquote.o:	mailaddrquote.c mailaddrquote.h

char.o:			char.c char.h

date.o:			date.c date.h

comparse.o:		comparse.c comparse.h

mhcom.o:		mhcom.c mhcom.h

zos.o:			zos.c zos.h

tmz.o:			tmz.c tmz.h

dater.o:		dater.c dater.h

zoffparts.o:		zoffparts.c zoffparts.h

realname.o:		realname.c realname.h

gecos.o:		gecos.c gecos.h

kinfo.o:		kinfo.c kinfo.h

nulstr.o:		nulstr.c nulstr.h

msfile.o:		msfile.c msfile.h msfilee.h

msfilee.o:		msfilee.c msfilee.h

ebuf.o:			ebuf.c ebuf.h

mapstrint.o:		mapstrint.c mapstrint.h

srvreg.o:		srvreg.c srvreg.h srvrege.h

srvrege.o:		srvrege.c srvrege.h

spawnproc.o:		spawnproc.c spawnproc.h

openport.o:		openport.c openport.h

pmq.o:			pmq.c pmq.h

ucsem.o:		ucsem.c ucsem.h

fmq.o:			fmq.c fmq.h

sigblock.o:		sigblock.c sigblock.h

paramfile.o:		paramfile.c paramfile.h

kvsfile.o:		kvsfile.c kvsfile.h

svcfile.o:		svcfile.c svcfile.h

acctab.o:		acctab.c acctab.h

srvtab.o:		srvtab.c srvtab.h

schedvar.o:		schedvar.c schedvar.h

varsub.o:		varsub.c varsub.h

mailalias.o:		mailalias.c mailalias.h

filebuf.o:		filebuf.c filebuf.h

bits.o:			bits.c bits.h

msgenv.o:		msgenv.c msgenv.h

rijndael.o:		rijndael.c rijndael.h

ema.o:			ema.c ema.h

mimetypes.o:		mimetypes.c mimetypes.h

ids.o:			ids.c ids.h

randomvar.o:		randomvar.c randomvar.h

fsdir.o:		fsdir.c fsdir.h

fsdirtree.o:		fsdirtree.c fsdirtree.h

density.o:		density.c density.h

dialtab.o:		dialtab.c dialtab.h

nodedb.o:		nodedb.c nodedb.h

clusterdb.o:		clusterdb.c clusterdb.h

hostinfo.o:		hostinfo.c hostinfo.h

hostaddr.o:		hostaddr.c hostaddr.h

hostent.o:		hostent.c hostent.h

inetaddr.o:		inetaddr.c inetaddr.h

sockaddress.o:		sockaddress.c sockaddress.h

lfm.o:			lfm.c lfm.h

keyopt.o:		keyopt.c keyopt.h

strtab.o:		strtab.c strtab.h

strpack.o:		strpack.c strpack.h

strstore.o:		strstore.c strstore.h

userattr.o:		userattr.c userattr.h

pwi.o:			pwi.c pwi.h ipasswd.h

ipasswd.o:		ipasswd.c ipasswd.h

wordfill.o:		wordfill.c wordfill.h

linefold.o:		linefold.c linefold.h

getax.o:		getax.c getax.h

tmtime.o:		tmtime.c tmtime.h

tmstrs.o:		tmstrs.c tmstrs.h

rmermsg.o:		rmermsg.c rmermsg.h

modload.o:		modload.c modload.h

dirseen.o:		dirseen.c dirseen.h

sysmiscem.o:		sysmiscem.c sysmiscem.h

sysmiscfh.o:		sysmiscfh.c sysmiscfh.h


wdt64.o:	wdt.c
	rm -f wdt64.c
	ln -f wdt.c wdt64.c
	$(CC) -c $(CFLAGS) -DWDT64 wdt64.c

fsdirtreestat.o:	fsdirtreestat.c fsdirtree.h

userinfo.o:		userinfo.c userinfo.h storeitem.h char.h gecos.h

getutmpent.o:		getutmpent.c getutmpent.h

getutmpterm.o:		getutmpterm.c getutmpent.h

udomain.o:		udomain.c

cpuspeed.o:		cpuspeed.c

ncpu.o:			ncpu.c 

rfile.o:		rfile.c incfile_rfilewrite.h

mkuiname.o:		mkuiname.c userinfo.h

mkuibang.o:		mkuibang.c userinfo.h

incfile_rfilewrite.h:	rfilewrite
	mkincfile rfilewrite

getdate.o:		getdate.mod
	cp -p getdate.mod getdate.o

getpwentry.o:		getpwentry.c getpwentry.h

getxusername.o:		getxusername.c getxusername.h

getpwlogname.o:		getpwlogname.c

getlogname.o:		getlogname.c

getloghost.o:		getloghost.c

pwilookup.o:		pwilookup.c pwi.h

dialtcp.o:		dialtcp.c

dialtcpnls.o:		dialtcpnls.c

dialtcpmux.o:		dialtcpmux.c

dialudp.o:		dialudp.c

dialticotsord.o:	dialticotsord.c

dialticotsordnls.o:	dialticotsordnls.c

dialuss.o:		dialuss.c

dialussnls.o:		dialussnls.c

dialussmux.o:		dialussmux.c

dialusd.o:		dialusd.c


unlinkd.o:		$(*)/$(@)
	cd $(*) ; make up

belowincs:
	makebelow upincs

below:
	makebelow


sfreadline.o:	sfreadline.c
	$(CC) -c $(CFLAGS) -I$(AST)/include/ast sfreadline.c

sfreadline.ls:	sfreadline.c
	$(LINT) $(LINTFLAGS) $(CPPFLAGS) -I$(AST)/include/ast sfreadline.c



